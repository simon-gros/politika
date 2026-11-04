// Prototype garrison, crush, concealment and building selection (Tactical.h holds the portable rules).
#include "PolitikaPrototypeGameplay.h"

#include "PolitikaBootstrap.h"

#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

namespace pt = politika::tactical;

namespace
{
constexpr int32 kMaxUnitsPerRequest = 64;
const FLinearColor kBuildingRingOwn(0.15f, 0.9f, 0.3f);
const FLinearColor kBuildingRingOther(0.95f, 0.75f, 0.2f);
constexpr double kBuildingRingMarginCm = 80.0;

// These events have no authored sound yet, so they are silent. The log line lets a session see that each
// one reached the player who should hear it, and an authored sound would play from here.
void TacticalCue(FName Role, const TCHAR* Event)
{
#if !UE_BUILD_SHIPPING
    UE_LOG(LogPolitikaStartup, Display, TEXT("RTS_TACTICAL_CUE role=%s event=%s"), *Role.ToString(), Event);
#endif
}
}

// ----------------------------------------------------------------------------
// Unit
// ----------------------------------------------------------------------------
bool APolitikaPrototypeUnit::IsVehicle() const
{
    static const TSet<FName> Vehicles = {
        APolitikaPrototypeMatch::HarvesterArchetypeId(),
        FName(TEXT("unit.recon_vehicle")),
        FName(TEXT("unit.mechanized_fireteam")),
        FName(TEXT("unit.opposing_patrol")),
    };
    return Vehicles.Contains(ArchetypeId);
}

float APolitikaPrototypeUnit::GetFireRange() const
{
    return bGarrisoned ? WeaponRange * static_cast<float>(pt::kGarrisonRangePermille) / 1000.0f : WeaponRange;
}

void APolitikaPrototypeUnit::HoldForGarrison(APolitikaPrototypeStructure* Structure)
{
    GarrisonTarget = Structure;
}

void APolitikaPrototypeUnit::EnterGarrison(APolitikaPrototypeStructure& Structure)
{
    GarrisonEntryLocation = GetActorLocation();
    GarrisonedIn = &Structure;
    GarrisonTarget = nullptr;
    bGarrisoned = true;
    // It keeps no order and no post: inside, it only fires on what comes into range.
    StopOrder();
    CurrentOrder = EPolitikaPrototypeOrder::Idle;
    bHasPost = false;
    bConcealPosture = false;
    SetActorLocation(Structure.GetActorLocation(), false, nullptr, ETeleportType::TeleportPhysics);
    ApplyGarrisonPresentation();
}

void APolitikaPrototypeUnit::LeaveGarrison()
{
    GarrisonedIn.Reset();
    bGarrisoned = false;
    OrderTarget.Reset();
    SetActorLocation(GarrisonEntryLocation, false, nullptr, ETeleportType::TeleportPhysics);
    bHasPost = false;
    ApplyGarrisonPresentation();
}

void APolitikaPrototypeUnit::ApplyGarrisonPresentation()
{
    Body->SetCollisionEnabled(bGarrisoned ? ECollisionEnabled::NoCollision : ECollisionEnabled::QueryOnly);
    Body->SetHiddenInGame(bGarrisoned || bLocalFogHidden, true);
}

void APolitikaPrototypeUnit::OnRep_Garrisoned()
{
    ApplyGarrisonPresentation();
#if !UE_BUILD_SHIPPING
    UE_LOG(LogPolitikaStartup, Display, TEXT("RTS_CLIENT_GARRISON unit=%llu garrisoned=%d"), StableId, bGarrisoned ? 1 : 0);
#endif
}

void APolitikaPrototypeUnit::SetConcealPosture(bool bInPosture)
{
    if (bInPosture && (!CanConceal() || bGarrisoned)) return;
    bConcealPosture = bInPosture;
    Concealment.posture = bInPosture;
    // An idle soldier holding concealment stops firing, so what it was firing at is dropped.
    if (bInPosture && CurrentOrder == EPolitikaPrototypeOrder::Idle) OrderTarget.Reset();
#if !UE_BUILD_SHIPPING
    UE_LOG(LogPolitikaStartup, Display, TEXT("RTS_CONCEAL phase=posture unit=%llu posture=%d"), StableId, bInPosture ? 1 : 0);
#endif
}

void APolitikaPrototypeUnit::AdvanceConcealment(int32 AppliedDamage, float DeltaSeconds)
{
    const bool bFired = bFiredThisStep;
    bFiredThisStep = false;
    if (!Concealment.posture && !Concealment.concealed) return;
    Concealment.posture = bConcealPosture && IsAlive() && !bGarrisoned;
    const double Moved = FVector::Dist2D(StepStartPosition, GetActorLocation());
    const bool bMoving = Moved > 0.5 * MoveSpeed * DeltaSeconds;
    const pt::ConcealChange Change = pt::AdvanceConcealment(Concealment, bMoving, bFired, AppliedDamage > 0);
    if (Change == pt::ConcealChange::None) return;
    bConcealed = Concealment.concealed;
#if !UE_BUILD_SHIPPING
    UE_LOG(LogPolitikaStartup, Display, TEXT("RTS_CONCEAL phase=%s unit=%llu moving=%d fired=%d damaged=%d step=%llu"),
        bConcealed ? TEXT("on") : TEXT("off"), StableId, bMoving ? 1 : 0, bFired ? 1 : 0, AppliedDamage > 0 ? 1 : 0, AuthorityStep());
#endif
    PresentConcealment();
}

void APolitikaPrototypeUnit::OnRep_Concealed()
{
#if !UE_BUILD_SHIPPING
    UE_LOG(LogPolitikaStartup, Display, TEXT("RTS_CLIENT_CONCEAL unit=%llu concealed=%d begun=%d"), StableId, bConcealed ? 1 : 0, HasActorBegunPlay() ? 1 : 0);
#endif
    // A unit first seen already concealed is a sighting, not the moment it went to ground.
    if (HasActorBegunPlay()) PresentConcealment();
}

void APolitikaPrototypeUnit::PresentConcealment()
{
    TacticalCue(ArchetypeId, bConcealed ? TEXT("StealthOn") : TEXT("StealthOff"));
}

void APolitikaPrototypeUnit::PresentCrush()
{
    TacticalCue(ArchetypeId, TEXT("Crush"));
}

// ----------------------------------------------------------------------------
// Match
// ----------------------------------------------------------------------------
bool APolitikaPrototypeMatch::CanGarrisonIn(const APolitikaPrototypeUnit* Unit, const APolitikaPrototypeStructure* Structure) const
{
    if (!Unit || !Unit->IsAlive() || Unit->IsRouted() || !Unit->CanGarrison() || !Structure || !Structure->IsAlive() || Structure->IsWithdrawn()) return false;
    if (Structure->GetOwnerPlayer() != Unit->GetOwnerPlayer()) return false;
    if (Structure->IsBuilding()) return !pt::EjectsOccupants(Structure->GetHealth(), Structure->GetMaxBuildingHealth());
    return Structure->IsWorks();
}

bool APolitikaPrototypeMatch::BeginGarrison(APolitikaPrototypeUnit* Unit, APolitikaPrototypeStructure* Structure)
{
    if (!HasAuthority() || GetMatchPhase() != politika::rules::MatchPhase::Active) return false;
    if (!CanGarrisonIn(Unit, Structure) || Unit->IsGarrisoned() || Structure->GetOccupants() >= pt::kGarrisonCapacity) return false;
    // A formation holds its members' places, so a soldier leaves it before going inside.
    if (Formations.FormationOf(Unit->GetStableId())) return false;
    Unit->StopOrder();
    Unit->HoldForGarrison(Structure);
#if !UE_BUILD_SHIPPING
    UE_LOG(LogPolitikaStartup, Display, TEXT("RTS_GARRISON phase=approach unit=%llu structure=%llu distance=%.1f"),
        Unit->GetStableId(), Structure->GetStableId(), FVector::Dist2D(Unit->GetActorLocation(), Structure->GetActorLocation()));
#endif
    return true;
}

void APolitikaPrototypeMatch::EjectOccupant(APolitikaPrototypeUnit& Unit, const TCHAR* Reason)
{
    APolitikaPrototypeStructure* Structure = Unit.GetGarrisonStructure();
    Unit.LeaveGarrison();
    if (Structure) Structure->SetOccupants(Structure->GetOccupants() - 1);
#if !UE_BUILD_SHIPPING
    UE_LOG(LogPolitikaStartup, Display, TEXT("RTS_GARRISON phase=exit unit=%llu structure=%llu reason=%s occupants=%d x=%.1f y=%.1f"),
        Unit.GetStableId(), Structure ? Structure->GetStableId() : 0ull, Reason, Structure ? Structure->GetOccupants() : 0,
        Unit.GetActorLocation().X, Unit.GetActorLocation().Y);
#endif
}

int32 APolitikaPrototypeMatch::UnloadGarrison(APolitikaPrototypeStructure* Structure)
{
    if (!HasAuthority() || !Structure) return 0;
    int32 Left = 0;
    for (APolitikaPrototypeUnit* Unit : Units)
    {
        if (!IsValid(Unit) || Unit->GetGarrisonStructure() != Structure) continue;
        EjectOccupant(*Unit, TEXT("unload"));
        ++Left;
    }
    return Left;
}

void APolitikaPrototypeMatch::AdvanceGarrisons()
{
    if (!HasAuthority()) return;
    for (APolitikaPrototypeUnit* Unit : Units)
    {
        if (!IsValid(Unit)) continue;
        if (APolitikaPrototypeStructure* Structure = Unit->GetGarrisonStructure())
        {
            // Section 6.9, plus what ends the building as this soldier's: it was lost, or captured.
            const bool bLost = !IsValid(Structure) || !Structure->IsAlive() || Structure->IsWithdrawn() || Structure->GetOwnerPlayer() != Unit->GetOwnerPlayer();
            if (bLost) EjectOccupant(*Unit, TEXT("lost"));
            else if (Structure->IsBuilding() && pt::EjectsOccupants(Structure->GetHealth(), Structure->GetMaxBuildingHealth()))
                EjectOccupant(*Unit, TEXT("damage"));
            continue;
        }
        APolitikaPrototypeStructure* Target = Unit->GetGarrisonTarget();
        if (!Target || FVector::Dist2D(Unit->GetActorLocation(), Target->GetActorLocation()) > pt::kGarrisonEntryRangeCm) continue;
        if (!CanGarrisonIn(Unit, Target) || Target->GetOccupants() >= pt::kGarrisonCapacity)
        {
            Unit->HoldForGarrison(nullptr);
            continue;
        }
        Unit->EnterGarrison(*Target);
        Target->SetOccupants(Target->GetOccupants() + 1);
#if !UE_BUILD_SHIPPING
        UE_LOG(LogPolitikaStartup, Display, TEXT("RTS_GARRISON phase=enter unit=%llu structure=%llu occupants=%d"),
            Unit->GetStableId(), Target->GetStableId(), Target->GetOccupants());
#endif
    }
}

void APolitikaPrototypeMatch::AdvanceCrushes()
{
    if (!HasAuthority()) return;
    for (APolitikaPrototypeUnit* Vehicle : Units)
    {
        if (!IsValid(Vehicle) || !Vehicle->IsAlive() || !Vehicle->IsVehicle()) continue;
        // A harvester's routine drives through a base without aiming at anyone, so only a player's order crushes.
        const politika::economy::Harvester* Routine = Economy.FindHarvester(Vehicle->GetOwnerPlayer(), Vehicle->GetStableId());
        if (Routine && Routine->phase != politika::economy::HarvesterPhase::Waiting) continue;
        const FVector From = Vehicle->StepStartPosition;
        const FVector To = Vehicle->GetActorLocation();
        int32 Crushed = 0;
        for (APolitikaPrototypeUnit* Victim : Units)
        {
            // The same enemies a unit would open fire on: never allies, and never the neutral Unaligned.
            if (!IsValid(Victim) || !Victim->IsAlive() || Victim->IsVehicle() || !Victim->IsAttackable()
                || !politika::rules::CanAutoEngage(Vehicle->GetOwnerPlayer(), Vehicle->GetTeamId(), Victim->GetOwnerPlayer(), Victim->GetTeamId()))
                continue;
            const FVector Where = Victim->GetActorLocation();
            if (!pt::Crushes(From.X, From.Y, To.X, To.Y, Where.X, Where.Y)) continue;
            Victim->PendingDamage = static_cast<float>(Victim->GetMaxHealth());
            ++Crushed;
#if !UE_BUILD_SHIPPING
            UE_LOG(LogPolitikaStartup, Display, TEXT("RTS_CRUSH vehicle=%llu victim=%llu x=%.1f y=%.1f step=%llu"),
                Vehicle->GetStableId(), Victim->GetStableId(), Where.X, Where.Y, SimulationStepsExecuted);
#endif
        }
        if (Crushed == 0 || (Vehicle->LastCrushCueStep != 0 && SimulationStepsExecuted - Vehicle->LastCrushCueStep < pt::kCrushCueSteps)) continue;
        Vehicle->LastCrushCueStep = SimulationStepsExecuted;
        Vehicle->PresentCrush();
        Vehicle->MulticastAbilityPresentation(2, 0.0f);
    }
}

// ----------------------------------------------------------------------------
// Structure
// ----------------------------------------------------------------------------
FName APolitikaPrototypeStructure::GetSoundRole() const
{
    return IsProducer() ? FName(TEXT("Producer")) : IsGenerator() ? FName(TEXT("Generator")) : IsWorks() ? FName(TEXT("Works"))
        : IsGoldSource() ? FName(TEXT("GoldSource")) : IsFoodSource() ? FName(TEXT("FoodSource")) : FName(TEXT("OilSource"));
}

void APolitikaPrototypeStructure::SetOccupants(int32 InOccupants)
{
    Occupants = FMath::Max(0, InOccupants);
    PresentOccupants();
}

void APolitikaPrototypeStructure::OnRep_Occupants()
{
#if !UE_BUILD_SHIPPING
    UE_LOG(LogPolitikaStartup, Display, TEXT("RTS_CLIENT_GARRISON structure=%llu occupants=%d"), StableId, Occupants);
#endif
    PresentOccupants();
}

void APolitikaPrototypeStructure::PresentOccupants()
{
    const int32 Before = PresentedOccupants;
    PresentedOccupants = Occupants;
    // A building first seen with soldiers inside is a sighting, not someone walking in.
    if (Before == Occupants || !HasActorBegunPlay() || IsBodyHidden()) return;
    TacticalCue(GetSoundRole(), Occupants > Before ? TEXT("Enter") : TEXT("Exit"));
}

void APolitikaPrototypeStructure::SetSelectedVisual(bool bSelected, bool bOwnedByViewer)
{
    if (!SelectionRing) return;
    if (bSelected)
    {
        const FVector Footprint = Body->GetComponentScale() * 100.0;
        const double Across = (FMath::Max(Footprint.X, Footprint.Y) + kBuildingRingMarginCm) / 100.0;
        SelectionRing->SetRelativeScale3D(FVector(Across, Across, 0.02));
        if (UMaterialInstanceDynamic* Material = SelectionRing->CreateAndSetMaterialInstanceDynamic(0))
            Material->SetVectorParameterValue(TEXT("Color"), bOwnedByViewer ? kBuildingRingOwn : kBuildingRingOther);
    }
    SelectionRing->SetVisibility(bSelected, true);
}

// ----------------------------------------------------------------------------
// Player controller
// ----------------------------------------------------------------------------
void APolitikaPrototypePlayerController::SelectStructure(APolitikaPrototypeStructure* Structure)
{
    APolitikaPrototypeStructure* Previous = SelectedStructure.Get();
    if (Previous == Structure) return;
    if (Previous) Previous->SetSelectedVisual(false);
    SelectedStructure = Structure;
    if (!Structure) return;
    Structure->SetSelectedVisual(true, Structure->GetOwnerPlayer() == GetCommandPlayerId());
    const FName Role = Structure->GetSoundRole();
    TacticalCue(Role, TEXT("Select"));
#if !UE_BUILD_SHIPPING
    UE_LOG(LogPolitikaStartup, Display, TEXT("RTS_STRUCTURE_SELECTED participant=%u structure=%llu owner=%u role=%s occupants=%d"),
        GetCommandPlayerId(), Structure->GetStableId(), Structure->GetOwnerPlayer(), *Role.ToString(), Structure->GetOccupants());
#endif
}

void APolitikaPrototypePlayerController::RefreshStructureSelection()
{
    const APolitikaPrototypeStructure* Structure = SelectedStructure.Get();
    if (!Structure) return;
    APolitikaPrototypeMatch* ActiveMatch = ResolveMatch();
    const bool bUnitsSelected = ActiveMatch && !GetCommandableSelectedUnits(ActiveMatch).IsEmpty();
    if (bUnitsSelected || Structure->IsWithdrawn() || Structure->IsBodyHidden()) SelectStructure(nullptr);
}

void APolitikaPrototypePlayerController::GarrisonOrUnload()
{
    APolitikaPrototypeMatch* ActiveMatch = ResolveMatch();
    if (!ActiveMatch) return;
    const TArray<APolitikaPrototypeUnit*> Selected = GetCommandableSelectedUnits(ActiveMatch);
    if (Selected.IsEmpty())
    {
        APolitikaPrototypeStructure* Structure = SelectedStructure.Get();
        if (!Structure || Structure->GetOwnerPlayer() != GetCommandPlayerId() || Structure->GetOccupants() == 0)
        {
            ShowLocalFeedback(TEXT("Select soldiers and point at your building, or select a garrisoned building."));
            return;
        }
        ShowLocalFeedback(TEXT("Unloading."));
        if (HasAuthority()) ActiveMatch->UnloadGarrison(Structure);
        else ServerUnloadGarrison(Structure->GetStableId());
        return;
    }
    APolitikaPrototypeStructure* Structure = StructureUnderCursor(*ActiveMatch);
#if !UE_BUILD_SHIPPING
    UE_LOG(LogPolitikaStartup, Display, TEXT("RTS_BUILDING_PRESS action=garrison structure=%llu"), Structure ? Structure->GetStableId() : 0ull);
#endif
    TArray<uint64> Ids;
    APolitikaPrototypeUnit* Speaker = nullptr;
    if (Structure && Structure->GetOccupants() < pt::kGarrisonCapacity)
        for (APolitikaPrototypeUnit* Unit : Selected)
        {
            if (!ActiveMatch->CanGarrisonIn(Unit, Structure)) continue;
            Ids.Add(Unit->GetStableId());
            if (!Speaker) Speaker = Unit;
        }
    if (Ids.IsEmpty())
    {
        ShowLocalFeedback(TEXT("Soldiers can garrison your own standing building, if it has room."));
        return;
    }
    TacticalCue(Speaker->GetArchetypeId(), TEXT("Garrison"));
    ShowLocalFeedback(TEXT("Moving to garrison."));
    if (!HasAuthority())
    {
        ServerBeginGarrison(Ids, Structure->GetStableId());
        return;
    }
    for (APolitikaPrototypeUnit* Unit : Selected)
        if (Ids.Contains(Unit->GetStableId())) ActiveMatch->BeginGarrison(Unit, Structure);
}

void APolitikaPrototypePlayerController::ServerBeginGarrison_Implementation(const TArray<uint64>& UnitIds, uint64 StructureId)
{
    APolitikaPrototypeMatch* ActiveMatch = ResolveMatch();
    if (!ActiveMatch || UnitIds.Num() > kMaxUnitsPerRequest) return;
    APolitikaPrototypeStructure* Structure = ActiveMatch->FindStructureByStableId(StructureId);
    const uint32 Participant = ActiveMatch->GetBoundParticipantId(this);
    for (const uint64 Id : UnitIds)
    {
        APolitikaPrototypeUnit* Unit = ActiveMatch->FindUnitByStableId(Id);
        if (Unit && Unit->GetOwnerPlayer() == Participant) ActiveMatch->BeginGarrison(Unit, Structure);
    }
}

void APolitikaPrototypePlayerController::ServerUnloadGarrison_Implementation(uint64 StructureId)
{
    APolitikaPrototypeMatch* ActiveMatch = ResolveMatch();
    if (!ActiveMatch) return;
    APolitikaPrototypeStructure* Structure = ActiveMatch->FindStructureByStableId(StructureId);
    if (Structure && Structure->GetOwnerPlayer() == ActiveMatch->GetBoundParticipantId(this)) ActiveMatch->UnloadGarrison(Structure);
}

void APolitikaPrototypePlayerController::ToggleConcealment()
{
    APolitikaPrototypeMatch* ActiveMatch = ResolveMatch();
    if (!ActiveMatch) return;
    TArray<uint64> Ids;
    bool bAnyOut = false;
    TArray<APolitikaPrototypeUnit*> Able;
    for (APolitikaPrototypeUnit* Unit : GetCommandableSelectedUnits(ActiveMatch))
    {
        if (!Unit->CanConceal()) continue;
        Able.Add(Unit);
        Ids.Add(Unit->GetStableId());
        bAnyOut |= !Unit->IsConcealPosture();
    }
    if (Able.IsEmpty())
    {
        ShowLocalFeedback(TEXT("Select soldiers to conceal."));
        return;
    }
    ShowLocalFeedback(bAnyOut ? TEXT("Concealing: hold still for 3 s. Soldiers will not fire unless ordered.") : TEXT("No longer concealing."));
    if (!HasAuthority())
    {
        ServerSetConcealment(Ids, bAnyOut);
        return;
    }
    for (APolitikaPrototypeUnit* Unit : Able) Unit->SetConcealPosture(bAnyOut);
}

void APolitikaPrototypePlayerController::ServerSetConcealment_Implementation(const TArray<uint64>& UnitIds, bool bConceal)
{
    APolitikaPrototypeMatch* ActiveMatch = ResolveMatch();
    if (!ActiveMatch || UnitIds.Num() > kMaxUnitsPerRequest) return;
    const uint32 Participant = ActiveMatch->GetBoundParticipantId(this);
    for (const uint64 Id : UnitIds)
    {
        APolitikaPrototypeUnit* Unit = ActiveMatch->FindUnitByStableId(Id);
        if (Unit && Unit->IsAlive() && Unit->GetOwnerPlayer() == Participant) Unit->SetConcealPosture(bConceal);
    }
}
