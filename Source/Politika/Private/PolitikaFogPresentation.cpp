// Viewer-side fog-of-war presentation (MP-2, audit F33): explored/visible shading and
// last-known contacts on every rendering process of a networked session.
//
// Authority boundary: the server alone decides which units each connection receives
// (team vision + IsNetRelevantFor). This file only derives presentation from what this
// process already holds legitimately: its own team's units (always relevant) and contacts
// it was sent while they were observed. It never reads a hidden unit, never replicates,
// and a last-known contact is not a command target (orders still require a currently
// observed entity; see OrderRejection::TargetNotVisible).

#include "PolitikaPrototypeGameplay.h"

#include "PolitikaBootstrap.h"
#include "Fog.h"
#include "Rules.h"

#include "CanvasItem.h"
#include "Engine/Canvas.h"
#include "Engine/Texture2D.h"
#include "HAL/PlatformTime.h"

namespace
{
using politika::rules::ContactClearReason;
using politika::rules::FogCell;
using politika::rules::LastKnownContact;
using politika::rules::VisionSource;

using FVisionSources = TArray<VisionSource, TInlineAllocator<16>>;

FVisionSources CollectTeamSources(const TArray<TObjectPtr<APolitikaPrototypeUnit>>& Units, uint8 Team)
{
    FVisionSources Sources;
    for (const APolitikaPrototypeUnit* Unit : Units)
    {
        if (!IsValid(Unit) || !Unit->IsAlive() || Unit->GetTeamId() != Team) continue;
        const FVector Location = Unit->GetActorLocation();
        Sources.Add({Team, {Location.X, Location.Y}, Unit->GetSightRadius()});
    }
    return Sources;
}

std::span<const VisionSource> AsSpan(const FVisionSources& Sources)
{
    return std::span<const VisionSource>(Sources.GetData(), Sources.Num());
}

const TCHAR* ViewerRole(const AActor& Actor)
{
    return Actor.HasAuthority() ? TEXT("host") : TEXT("client");
}

float CellDarkness(FogCell Cell)
{
    switch (Cell)
    {
        case FogCell::Visible: return 0.0f;
        case FogCell::Explored: return APolitikaPrototypeHUD::FogDarknessExplored;
        case FogCell::Unexplored: return APolitikaPrototypeHUD::FogDarknessUnexplored;
    }
    return APolitikaPrototypeHUD::FogDarknessUnexplored;
}
} // namespace

// ----------------------------------------------------------------------------
// Match: local knowledge
// ----------------------------------------------------------------------------

bool APolitikaPrototypeMatch::IsLocalFogPresentationActive() const
{
    const ENetMode Mode = GetNetMode();
    if (!bFogOfWar || Mode == NM_Standalone || Mode == NM_DedicatedServer) return false;
    const bool bReady = HasAuthority() ? bContentReady : bObservedContentReady;
    return bReady && GetLocalViewerTeam() != politika::rules::kNoTeam;
}

bool APolitikaPrototypeMatch::IsLocalContact(const APolitikaPrototypeUnit* Unit) const
{
    // Only non-friendly contacts are remembered: own and allied units are always observed.
    return Unit && Unit->GetOwnerPlayer() != LocalRuntimePlayerId
        && !politika::rules::AreAllied(Unit->GetOwnerPlayer(), Unit->GetTeamId(), LocalRuntimePlayerId, GetLocalViewerTeam());
}

void APolitikaPrototypeMatch::NoteLocalContactWithdrawn(const APolitikaPrototypeUnit* Unit, bool bLastKnownAlive,
    const FVector& LastSeenLocation)
{
    if (!IsValid(Unit) || !bLocalFogInitialized || !IsLocalFogPresentationActive() || !IsLocalContact(Unit)) return;
    // Teardown at results/reset withdraws every unit; that is not a sighting.
    if (Lifecycle.phase != politika::rules::MatchPhase::Active) return;
    const FVector& Location = LastSeenLocation;
    const LastKnownContact Contact{Unit->GetStableId(), Unit->GetOwnerPlayer(), Unit->GetTeamId(),
        {Location.X, Location.Y}, static_cast<uint64>(FPlatformTime::Seconds() * 1000.0)};
    const auto Result = LocalContacts.Withdrawn(Contact, bLastKnownAlive);
#if !UE_BUILD_SHIPPING
    if (Result == politika::rules::ContactMemory::Withdrawal::Recorded)
    {
        UE_LOG(LogPolitikaStartup, Display,
            TEXT("MP_LOCAL_CONTACT_LAST_KNOWN role=%s generation=%llu viewer=%u viewer_team=%u entity=%llu owner=%u team=%u x=%.1f y=%.1f contacts=%d"),
            ViewerRole(*this), MatchGeneration, LocalRuntimePlayerId, GetLocalViewerTeam(), Contact.entity, Contact.owner,
            Contact.team, Location.X, Location.Y, static_cast<int32>(LocalContacts.Contacts().size()));
    }
    else if (Result == politika::rules::ContactMemory::Withdrawal::ForgottenDead)
    {
        UE_LOG(LogPolitikaStartup, Display,
            TEXT("MP_LOCAL_CONTACT_CLEARED role=%s generation=%llu viewer=%u entity=%llu reason=%s"),
            ViewerRole(*this), MatchGeneration, LocalRuntimePlayerId, Contact.entity,
            UTF8_TO_TCHAR(politika::rules::ContactClearReasonName(ContactClearReason::Died).data()));
    }
#else
    (void)Result;
#endif
}

void APolitikaPrototypeMatch::NoteLocalContactObserved(const APolitikaPrototypeUnit* Unit)
{
    if (!IsValid(Unit) || !LocalContacts.Reobserved(Unit->GetStableId())) return;
#if !UE_BUILD_SHIPPING
    UE_LOG(LogPolitikaStartup, Display, TEXT("MP_LOCAL_CONTACT_CLEARED role=%s generation=%llu viewer=%u entity=%llu reason=%s"),
        ViewerRole(*this), MatchGeneration, LocalRuntimePlayerId, Unit->GetStableId(),
        UTF8_TO_TCHAR(politika::rules::ContactClearReasonName(ContactClearReason::Reobserved).data()));
#endif
}

void APolitikaPrototypeMatch::ClearLocalContacts(const TCHAR* Reason)
{
    LocalContacts.Clear([this, Reason](const LastKnownContact& Contact)
    {
#if !UE_BUILD_SHIPPING
        UE_LOG(LogPolitikaStartup, Display, TEXT("MP_LOCAL_CONTACT_CLEARED role=%s generation=%llu viewer=%u entity=%llu reason=%s detail=%s"),
            ViewerRole(*this), LocalFogGeneration, LocalRuntimePlayerId, Contact.entity,
            UTF8_TO_TCHAR(politika::rules::ContactClearReasonName(ContactClearReason::Reset).data()), Reason);
#else
        (void)Contact; (void)Reason;
#endif
    });
}

void APolitikaPrototypeMatch::ResetLocalFogKnowledge(const TCHAR* Reason)
{
    ClearLocalContacts(Reason);
    LocalFog.Reset(politika::rules::FogGridSpec{});
    bLocalFogInitialized = false;
}

void APolitikaPrototypeMatch::UpdateLocalFogKnowledge()
{
    if (!IsLocalFogPresentationActive())
    {
        if (bLocalFogInitialized) ResetLocalFogKnowledge(TEXT("inactive"));
        return;
    }
    if (!bLocalFogInitialized || LocalFogGeneration != MatchGeneration)
    {
        if (bLocalFogInitialized) ResetLocalFogKnowledge(TEXT("generation"));
        // Shade the playable area plus a wide margin; the HUD extends the outer edge further
        // as unexplored so the surrounding terrain never reads as visible.
        constexpr double Margin = 3500.0;
        FVector2D Min = PlayableMin;
        FVector2D Max = PlayableMax;
        if (!(Max.X > Min.X) || !(Max.Y > Min.Y)) { Min = FVector2D(-3000.0); Max = FVector2D(3000.0); }
        LocalFog.Reset({{Min.X - Margin, Min.Y - Margin}, {Max.X + Margin, Max.Y + Margin}, LocalFogCellSizeCm});
        LocalFogGroundZ = Objective ? static_cast<float>(Objective->GetActorLocation().Z) : 0.0f;
        LocalFogGeneration = MatchGeneration;
        LastLocalFogSeconds = 0.0;
        bLocalFogInitialized = true;
    }
    if (Lifecycle.phase != politika::rules::MatchPhase::Active && !LocalContacts.Contacts().empty())
        ClearLocalContacts(TEXT("match_inactive"));
    const double Now = FPlatformTime::Seconds();
    if (Now < LastLocalFogSeconds + LocalFogUpdateSeconds) return;
    LastLocalFogSeconds = Now;

    const uint8 Team = GetLocalViewerTeam();
    const FVisionSources Sources = CollectTeamSources(Units, Team);
    LocalFog.Update(Team, AsSpan(Sources));
    LocalContacts.ClearRevealed(Team, AsSpan(Sources), [this](const LastKnownContact& Contact)
    {
#if !UE_BUILD_SHIPPING
        UE_LOG(LogPolitikaStartup, Display, TEXT("MP_LOCAL_CONTACT_CLEARED role=%s generation=%llu viewer=%u entity=%llu reason=%s x=%.1f y=%.1f"),
            ViewerRole(*this), MatchGeneration, LocalRuntimePlayerId, Contact.entity,
            UTF8_TO_TCHAR(politika::rules::ContactClearReasonName(ContactClearReason::Revealed).data()),
            Contact.position.x, Contact.position.y);
#else
        (void)Contact;
#endif
    });
#if !UE_BUILD_SHIPPING
    if (Now >= LastLocalFogLogSeconds + 1.0)
    {
        LastLocalFogLogSeconds = Now;
        UE_LOG(LogPolitikaStartup, Display,
            TEXT("MP_LOCAL_FOG role=%s generation=%llu viewer=%u viewer_team=%u sources=%d columns=%u rows=%u visible=%llu explored=%llu unexplored=%llu contacts=%d"),
            ViewerRole(*this), MatchGeneration, LocalRuntimePlayerId, Team, Sources.Num(), LocalFog.Columns(), LocalFog.Rows(),
            static_cast<uint64>(LocalFog.Count(FogCell::Visible)), static_cast<uint64>(LocalFog.Count(FogCell::Explored)),
            static_cast<uint64>(LocalFog.Count(FogCell::Unexplored)), static_cast<int32>(LocalContacts.Contacts().size()));
    }
#endif
}

// ----------------------------------------------------------------------------
// HUD: shading and remembered contacts
// ----------------------------------------------------------------------------

void APolitikaPrototypeHUD::DrawLocalFog(APlayerController& Controller, const APolitikaPrototypeMatch& ActiveMatch)
{
    if (!Canvas || !Canvas->DefaultTexture || !ActiveMatch.IsLocalFogPresentationActive()) return;
    const politika::rules::FogMap& Fog = ActiveMatch.GetLocalFog();
    const uint32 Columns = Fog.Columns();
    const uint32 Rows = Fog.Rows();
    if (Columns == 0 || Rows == 0) return;
    const politika::rules::FogGridSpec& Spec = Fog.Spec();
    const double Ground = ActiveMatch.GetLocalFogGroundZ();

    // Per-vertex darkness is the mean of the adjacent cells, so fog edges are soft.
    const uint32 VertexColumns = Columns + 1;
    TArray<float> Darkness;
    TArray<FVector2D> Screen;
    TBitArray<> OnScreen(false, VertexColumns * (Rows + 1));
    Darkness.SetNumZeroed(VertexColumns * (Rows + 1));
    Screen.SetNumZeroed(VertexColumns * (Rows + 1));
    for (uint32 Row = 0; Row <= Rows; ++Row)
        for (uint32 Column = 0; Column <= Columns; ++Column)
        {
            float Sum = 0.0f;
            int32 Count = 0;
            for (uint32 CellRow = Row > 0 ? Row - 1 : 0; CellRow <= FMath::Min(Row, Rows - 1); ++CellRow)
                for (uint32 CellColumn = Column > 0 ? Column - 1 : 0; CellColumn <= FMath::Min(Column, Columns - 1); ++CellColumn)
                {
                    Sum += CellDarkness(Fog.At(CellColumn, CellRow));
                    ++Count;
                }
            const int32 Index = Row * VertexColumns + Column;
            Darkness[Index] = Count > 0 ? Sum / Count : 0.0f;
            const FVector World(Spec.min.x + Column * Spec.cell_size, Spec.min.y + Row * Spec.cell_size, Ground);
            OnScreen[Index] = Controller.ProjectWorldLocationToScreen(World, Screen[Index], true);
        }

    FCanvasTriangleItem Item(FVector2D::ZeroVector, FVector2D::ZeroVector, FVector2D::ZeroVector,
        Canvas->DefaultTexture->GetResource());
    Item.BlendMode = SE_BLEND_Translucent;
    const auto Vertex = [&](int32 Index, FCanvasUVTri& Tri, int32 Corner)
    {
        const FLinearColor Color(0.012f, 0.016f, 0.02f, Darkness[Index]);
        switch (Corner)
        {
            case 0: Tri.V0_Pos = Screen[Index]; Tri.V0_Color = Color; break;
            case 1: Tri.V1_Pos = Screen[Index]; Tri.V1_Color = Color; break;
            default: Tri.V2_Pos = Screen[Index]; Tri.V2_Color = Color; break;
        }
    };
    for (uint32 Row = 0; Row < Rows; ++Row)
        for (uint32 Column = 0; Column < Columns; ++Column)
        {
            const int32 A = Row * VertexColumns + Column;
            const int32 B = A + 1;
            const int32 C = A + VertexColumns + 1;
            const int32 D = A + VertexColumns;
            if (!OnScreen[A] || !OnScreen[B] || !OnScreen[C] || !OnScreen[D]) continue;
            if (Darkness[A] + Darkness[B] + Darkness[C] + Darkness[D] < 0.01f) continue;
            FCanvasUVTri First, Second;
            Vertex(A, First, 0); Vertex(B, First, 1); Vertex(C, First, 2);
            Vertex(A, Second, 0); Vertex(C, Second, 1); Vertex(D, Second, 2);
            Item.TriangleList.Add(First);
            Item.TriangleList.Add(Second);
        }
    // Radial skirt from the grid perimeter outward, so terrain beyond the grid reads as
    // unexplored. Perimeter vertices in loop order: south, east, north, west edges.
    TArray<int32, TInlineAllocator<512>> Perimeter;
    for (uint32 Column = 0; Column < Columns; ++Column) Perimeter.Add(Column);
    for (uint32 Row = 0; Row < Rows; ++Row) Perimeter.Add(Row * VertexColumns + Columns);
    for (uint32 Column = Columns; Column > 0; --Column) Perimeter.Add(Rows * VertexColumns + Column);
    for (uint32 Row = Rows; Row > 0; --Row) Perimeter.Add(Row * VertexColumns);
    const FVector2D Centre((Spec.min.x + Spec.max.x) * 0.5, (Spec.min.y + Spec.max.y) * 0.5);
    TArray<FVector2D, TInlineAllocator<512>> Outer;
    TBitArray<> OuterOnScreen(false, Perimeter.Num());
    Outer.SetNumZeroed(Perimeter.Num());
    for (int32 Index = 0; Index < Perimeter.Num(); ++Index)
    {
        const int32 Corner = Perimeter[Index];
        const FVector2D Inner(Spec.min.x + (Corner % VertexColumns) * Spec.cell_size, Spec.min.y + (Corner / VertexColumns) * Spec.cell_size);
        const FVector2D Far = Inner + (Inner - Centre).GetSafeNormal() * FogSkirtCm;
        OuterOnScreen[Index] = Controller.ProjectWorldLocationToScreen(FVector(Far.X, Far.Y, Ground), Outer[Index], true);
    }
    const FLinearColor Shroud(0.012f, 0.016f, 0.02f, FogDarknessUnexplored);
    for (int32 Index = 0; Index < Perimeter.Num(); ++Index)
    {
        const int32 Next = (Index + 1) % Perimeter.Num();
        const int32 A = Perimeter[Index];
        const int32 B = Perimeter[Next];
        if (!OnScreen[A] || !OnScreen[B] || !OuterOnScreen[Index] || !OuterOnScreen[Next]) continue;
        FCanvasUVTri First, Second;
        Vertex(A, First, 0); Vertex(B, First, 1);
        First.V2_Pos = Outer[Next]; First.V2_Color = Shroud;
        Vertex(A, Second, 0);
        Second.V1_Pos = Outer[Next]; Second.V1_Color = Shroud;
        Second.V2_Pos = Outer[Index]; Second.V2_Color = Shroud;
        Item.TriangleList.Add(First);
        Item.TriangleList.Add(Second);
    }
    if (!Item.TriangleList.IsEmpty()) Canvas->DrawItem(Item);
}

void APolitikaPrototypeHUD::DrawLastKnownContacts(APlayerController& Controller, const APolitikaPrototypeMatch& ActiveMatch,
    float UiScale)
{
    if (!ActiveMatch.IsLocalFogPresentationActive()) return;
    const double NowMs = FPlatformTime::Seconds() * 1000.0;
    for (const LastKnownContact& Contact : ActiveMatch.GetLocalContacts().Contacts())
    {
        FVector2D Screen;
        const FVector Ground(Contact.position.x, Contact.position.y, ActiveMatch.GetLocalFogGroundZ());
        if (!Controller.ProjectWorldLocationToScreen(Ground, Screen, true)) continue;
        // A hollow diamond and dimmed label: a stale position, not a live (or targetable) unit.
        const bool bUnaligned = Contact.team == politika::rules::kUnalignedTeam;
        const FLinearColor Color = bUnaligned ? FLinearColor(0.78f, 0.75f, 0.58f, 0.8f) : FLinearColor(0.9f, 0.45f, 0.36f, 0.8f);
        const float R = 14.0f * UiScale;
        DrawLine(Screen.X, Screen.Y - R, Screen.X + R, Screen.Y, Color, 2.0f);
        DrawLine(Screen.X + R, Screen.Y, Screen.X, Screen.Y + R, Color, 2.0f);
        DrawLine(Screen.X, Screen.Y + R, Screen.X - R, Screen.Y, Color, 2.0f);
        DrawLine(Screen.X - R, Screen.Y, Screen.X, Screen.Y - R, Color, 2.0f);
        const int32 Seconds = FMath::Max(0, FMath::FloorToInt32((NowMs - static_cast<double>(Contact.last_seen_ms)) / 1000.0));
        DrawText(FString::Printf(TEXT("LAST SEEN #%llu  %ds"), Contact.entity, Seconds), Color,
            Screen.X - 60.0f * UiScale, Screen.Y - R - 20.0f * UiScale, nullptr, 0.8f * UiScale);
    }
}
