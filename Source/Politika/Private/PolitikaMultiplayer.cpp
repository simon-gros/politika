// MP-1 listen-server foundation and MP-3 dedicated hosting: connection-bound participant
// authority, the single command intake shared by host-local and remote input, outcome
// routing, observer initialization, the replicated state boundary and the dedicated
// server's session close/reset. Simulation remains exclusively
// in APolitikaPrototypeMatch's authoritative fixed-step update.
#include "PolitikaPrototypeGameplay.h"

#include "PolitikaBootstrap.h"
#include "PolitikaGameInstance.h"
#include "PolitikaInfantryPresentation.h"
#include "PolitikaPrototypeContent.h"
#include "Metrics.h"
#include "Rules.h"

#include "Engine/NetConnection.h"
#include "Engine/NetDriver.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/GameSession.h"
#include "HAL/PlatformTime.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"

#include <limits>

namespace
{
// Bounded per-participant queue; a flood cannot grow authoritative work without limit.
constexpr int32 MaxPendingCommandsPerParticipant = 16;
// After a remote participant disconnects, the host shows the reason, then returns.
constexpr float ParticipantDisconnectReturnSeconds = 3.0f;
// A dedicated server keeps a finished match's results available to its clients this long,
// then closes the session for clients that have not left on their own.
constexpr float DedicatedResultsLingerSeconds = 30.0f;
// Once every client has left a finished match, the dedicated server resets promptly.
constexpr float DedicatedEmptySessionResetSeconds = 1.0f;
// Kicked clients receive their reason before the map reload tears down their connection.
constexpr float DedicatedKickFlushSeconds = 1.0f;

APolitikaPrototypeMatch* FindMatch(UWorld* World)
{
    if (!World) return nullptr;
    TActorIterator<APolitikaPrototypeMatch> It(World);
    return It ? *It : nullptr;
}

const TCHAR* ControllerKindName(EPolitikaPrototypeControllerKind Kind)
{
    switch (Kind)
    {
        case EPolitikaPrototypeControllerKind::LocalHuman: return TEXT("LocalHuman");
        case EPolitikaPrototypeControllerKind::AssignedOpponent: return TEXT("AssignedOpponent");
        case EPolitikaPrototypeControllerKind::RemoteHuman: return TEXT("RemoteHuman");
    }
    return TEXT("Unknown");
}

bool IsHumanSeat(EPolitikaPrototypeControllerKind Kind)
{
    return Kind == EPolitikaPrototypeControllerKind::LocalHuman || Kind == EPolitikaPrototypeControllerKind::RemoteHuman;
}

void FillIntentParameters(FPolitikaNetCommandIntent& Intent, const politika::rules::Order& Order)
{
    using namespace politika::rules;
    Intent.OrderKind = static_cast<uint8>(Order.index());
    if (const auto* Move = std::get_if<MoveOrder>(&Order))
    { Intent.DestinationX = Move->destination.x; Intent.DestinationY = Move->destination.y; }
    else if (const auto* AttackMove = std::get_if<AttackMoveOrder>(&Order))
    { Intent.DestinationX = AttackMove->destination.x; Intent.DestinationY = AttackMove->destination.y; }
    else if (const auto* Attack = std::get_if<AttackOrder>(&Order)) Intent.TargetEntityId = Attack->target_id;
    else if (const auto* Grenade = std::get_if<GrenadeOrder>(&Order)) Intent.TargetEntityId = Grenade->target_id;
}

#if !UE_BUILD_SHIPPING
// The engine's cumulative counters for one connection. Totals include the handshake and
// map load, so the harness differences the activation sample against later ones.
void LogConnectionStats(const TCHAR* Role, const TCHAR* Reason, uint64 Generation, uint64 Step, uint32 Participant,
    uint32 Team, const UNetConnection& Connection, int32 VisibleUnits, double ElapsedSeconds)
{
    UE_LOG(LogPolitikaStartup, Display,
        TEXT("MP_NET_STATS role=%s reason=%s generation=%llu step=%llu participant=%u team=%u elapsed_s=%.3f in_bytes=%d out_bytes=%d in_packets=%d out_packets=%d in_lost=%d out_lost=%d ooo_recovered=%d ooo_lost=%d ooo_duplicate=%d in_bps=%d out_bps=%d avg_lag_ms=%.1f jitter_ms=%.1f visible_units=%d"),
        Role, Reason, Generation, Step, Participant, Team, ElapsedSeconds, Connection.InTotalBytes, Connection.OutTotalBytes,
        Connection.InTotalPackets, Connection.OutTotalPackets, Connection.InTotalPacketsLost, Connection.OutTotalPacketsLost,
        Connection.GetTotalOutOfOrderPacketsRecovered(), Connection.GetTotalOutOfOrderPacketsLost(),
        Connection.GetTotalOutOfOrderPacketsDuplicate(), Connection.InBytesPerSecond, Connection.OutBytesPerSecond,
        Connection.AvgLag * 1000.0f, Connection.GetAverageJitterInMS(), VisibleUnits);
}
#endif
}

// ----------------------------------------------------------------------------
// Replicated state boundary
// ----------------------------------------------------------------------------

void APolitikaPrototypeUnit::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    // Public observation for connections whose team observes the unit (IsNetRelevantFor
    // withholds the whole actor otherwise, so none of these reach unobserving teams).
    DOREPLIFETIME(APolitikaPrototypeUnit, StableId);
    DOREPLIFETIME(APolitikaPrototypeUnit, OwnerPlayer);
    DOREPLIFETIME(APolitikaPrototypeUnit, TeamId);
    DOREPLIFETIME(APolitikaPrototypeUnit, FactionId);
    DOREPLIFETIME(APolitikaPrototypeUnit, ArchetypeId);
    DOREPLIFETIME(APolitikaPrototypeUnit, UnitName);
    DOREPLIFETIME(APolitikaPrototypeUnit, Health);
    DOREPLIFETIME(APolitikaPrototypeUnit, MaxHealth);
    DOREPLIFETIME(APolitikaPrototypeUnit, MoveSpeed);
    DOREPLIFETIME(APolitikaPrototypeUnit, WeaponRange);
    DOREPLIFETIME(APolitikaPrototypeUnit, WeaponDamage);
    DOREPLIFETIME(APolitikaPrototypeUnit, WeaponCooldown);
    DOREPLIFETIME(APolitikaPrototypeUnit, bAttackable);
    DOREPLIFETIME(APolitikaPrototypeUnit, bBlastDeath);
    // Owner-only: the issued order and ability timers belong to the controlling participant.
    DOREPLIFETIME_CONDITION(APolitikaPrototypeUnit, CurrentOrder, COND_OwnerOnly);
    DOREPLIFETIME_CONDITION(APolitikaPrototypeUnit, Ammo, COND_OwnerOnly);
    DOREPLIFETIME_CONDITION(APolitikaPrototypeUnit, ReloadRemaining, COND_OwnerOnly);
    DOREPLIFETIME_CONDITION(APolitikaPrototypeUnit, SprintRemaining, COND_OwnerOnly);
}

void APolitikaPrototypeObjective::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(APolitikaPrototypeObjective, Progress);
    DOREPLIFETIME(APolitikaPrototypeObjective, bContested);
    DOREPLIFETIME(APolitikaPrototypeObjective, bComplete);
    DOREPLIFETIME(APolitikaPrototypeObjective, CapturingPlayerId);
    DOREPLIFETIME(APolitikaPrototypeObjective, CapturingTeamId);
}

void APolitikaPrototypeMatch::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    // The unit list is deliberately not replicated: each client discovers only the
    // fog-relevant unit actors it receives (see RegisterObservedUnit).
    DOREPLIFETIME(APolitikaPrototypeMatch, Objective);
    DOREPLIFETIME(APolitikaPrototypeMatch, MatchGeneration);
    DOREPLIFETIME(APolitikaPrototypeMatch, ReplicatedMapId);
    DOREPLIFETIME(APolitikaPrototypeMatch, ReplicatedPhase);
    DOREPLIFETIME(APolitikaPrototypeMatch, bReplicatedResultCommitted);
    DOREPLIFETIME(APolitikaPrototypeMatch, ReplicatedResultWinner);
    DOREPLIFETIME(APolitikaPrototypeMatch, ReplicatedResultReason);
    DOREPLIFETIME(APolitikaPrototypeMatch, ReplicatedResultTeam);
    DOREPLIFETIME(APolitikaPrototypeMatch, ReplicatedParticipants);
    DOREPLIFETIME(APolitikaPrototypeMatch, ReplicatedLayout);
    DOREPLIFETIME(APolitikaPrototypeMatch, bReplicatedFogOfWar);
}

void APolitikaPrototypePlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME_CONDITION(APolitikaPrototypePlayerController, AssignedRuntimePlayerId, COND_OwnerOnly);
    DOREPLIFETIME_CONDITION(APolitikaPrototypePlayerController, AssignedSlotId, COND_OwnerOnly);
    DOREPLIFETIME_CONDITION(APolitikaPrototypePlayerController, AssignedFactionId, COND_OwnerOnly);
    DOREPLIFETIME_CONDITION(APolitikaPrototypePlayerController, AssignedMatchGeneration, COND_OwnerOnly);
}

void APolitikaPrototypeMatch::SyncReplicatedParticipants()
{
    ReplicatedParticipants.Reset();
    for (const FPolitikaPrototypeRuntimeParticipant& Participant : RuntimeParticipants)
    {
        FPolitikaNetParticipantInfo& Info = ReplicatedParticipants.AddDefaulted_GetRef();
        Info.SlotId = Participant.SlotId;
        Info.RuntimePlayerId = Participant.RuntimePlayerId;
        Info.FactionId = Participant.FactionId;
        Info.ControllerKind = static_cast<uint8>(Participant.ControllerKind);
        Info.TeamId = Participant.TeamId;
    }
    ReplicatedLayout = static_cast<uint8>(SessionLayout);
    bReplicatedFogOfWar = bFogOfWar;
}

void APolitikaPrototypeMatch::SyncReplicatedLifecycle()
{
    if (!HasAuthority()) return;
    ReplicatedPhase = static_cast<uint8>(Lifecycle.phase);
    bReplicatedResultCommitted = CommittedResult.IsSet();
    ReplicatedResultWinner = CommittedResult.IsSet() ? CommittedResult->WinningPlayerId : 0;
    ReplicatedResultReason = CommittedResult.IsSet() ? CommittedResult->Reason : NAME_None;
    ReplicatedResultTeam = CommittedResult.IsSet() ? CommittedResult->WinningTeamId : 0;
}

politika::rules::HostingMode APolitikaPrototypeMatch::GetHostingMode() const
{
#if WITH_DEV_AUTOMATION_TESTS
    if (bTestDedicatedHosting) return politika::rules::HostingMode::Dedicated;
#endif
    switch (GetNetMode())
    {
        case NM_DedicatedServer: return politika::rules::HostingMode::Dedicated;
        case NM_ListenServer: return politika::rules::HostingMode::Listen;
        default: return politika::rules::HostingMode::Standalone;
    }
}

bool APolitikaPrototypeMatch::IsNetworkSession() const
{
    return GetHostingMode() != politika::rules::HostingMode::Standalone;
}

// ----------------------------------------------------------------------------
// Session layout, alliances and fog of war (server-computed team vision)
// ----------------------------------------------------------------------------

bool APolitikaPrototypeMatch::ResolveSessionLayout(FName& OutFailure)
{
    using namespace politika::rules;
    OutFailure = NAME_None;
    bFogOfWar = IsNetworkSession();
    if (!IsNetworkSession())
    {
        // Offline: every authored participant plays as its own team, so alliances and
        // capture rules reduce exactly to the original owner-based behavior.
        SessionLayout = TeamLayout::OneVsOne;
        SessionSeatCount = RuntimeParticipants.Num();
        for (int32 Index = 0; Index < RuntimeParticipants.Num(); ++Index)
            RuntimeParticipants[Index].TeamId = static_cast<uint8>(Index + 1);
        return true;
    }
    FString LayoutText = TEXT("1v1");
    if (const UWorld* World = GetWorld(); World && World->URL.HasOption(TEXT("PolitikaLayout")))
        LayoutText = World->URL.GetOption(TEXT("PolitikaLayout="), TEXT(""));
    const FTCHARToUTF8 LayoutUtf8(*LayoutText);
    TeamLayout Layout = TeamLayout::OneVsOne;
    if (!ParseTeamLayout(std::string_view(LayoutUtf8.Get(), LayoutUtf8.Length()), Layout))
    {
        OutFailure = TEXT("MP.LAYOUT.INVALID");
        return false;
    }
    const int32 Seats = SeatCount(Layout);
    if (RuntimeParticipants.Num() < Seats)
    {
        OutFailure = TEXT("MAP.SLOTS.LAYOUT_UNSUPPORTED");
        return false;
    }
    // Seat order: the authored local-human slot hosts seat 0, then the remaining authored
    // slots by slot ID. Authored slots beyond the layout do not play this session.
    RuntimeParticipants.StableSort([](const FPolitikaPrototypeRuntimeParticipant& A, const FPolitikaPrototypeRuntimeParticipant& B)
    {
        const bool bHostA = A.ControllerKind == EPolitikaPrototypeControllerKind::LocalHuman;
        const bool bHostB = B.ControllerKind == EPolitikaPrototypeControllerKind::LocalHuman;
        return bHostA != bHostB ? bHostA : A.SlotId < B.SlotId;
    });
    RuntimeParticipants.SetNum(Seats);
    for (int32 Seat = 0; Seat < Seats; ++Seat)
        RuntimeParticipants[Seat].TeamId = TeamForSeat(Layout, static_cast<uint8>(Seat));
    const HostingMode Hosting = GetHostingMode();
    if (Hosting == HostingMode::Dedicated)
    {
        // No player lives on a dedicated server: every seat, including the authored
        // human slot (seat 0), is open to a remote connection in join order. Seat order,
        // teams and participant IDs are identical to the listen layout.
        for (FPolitikaPrototypeRuntimeParticipant& Participant : RuntimeParticipants)
            Participant.ControllerKind = EPolitikaPrototypeControllerKind::AssignedOpponent;
        LocalRuntimePlayerId = 0;
    }
    SessionLayout = Layout;
    SessionSeatCount = Seats;
    FString Roster;
    for (const FPolitikaPrototypeRuntimeParticipant& Participant : RuntimeParticipants)
        Roster += FString::Printf(TEXT("%s%u:%d:%c"), Roster.IsEmpty() ? TEXT("") : TEXT(","),
            Participant.RuntimePlayerId, Participant.SlotId, TCHAR('A' + Participant.TeamId - 1));
    UE_LOG(LogPolitikaStartup, Display, TEXT("MP_SESSION_LAYOUT layout=%s seats=%d fog=%d roster=%s hosting=%s remote_seats=%u"),
        UTF8_TO_TCHAR(LayoutName(Layout).data()), Seats, bFogOfWar ? 1 : 0, *Roster,
        UTF8_TO_TCHAR(HostingModeName(Hosting).data()), static_cast<uint32>(RemoteSeatCount(Layout, Hosting)));
    return true;
}

uint32 APolitikaPrototypeMatch::GetMaxRemoteHumans() const
{
    return IsNetworkSession() && SessionSeatCount > 1
        ? static_cast<uint32>(politika::rules::RemoteSeatCount(SessionLayout, GetHostingMode())) : 0;
}

uint8 APolitikaPrototypeMatch::GetTeamForParticipant(uint32 PlayerId) const
{
    if (PlayerId == 0) return politika::rules::kNoTeam;
    const FPolitikaPrototypeRuntimeParticipant* Participant = RuntimeParticipants.FindByPredicate(
        [PlayerId](const FPolitikaPrototypeRuntimeParticipant& Candidate) { return Candidate.RuntimePlayerId == PlayerId; });
    return Participant ? Participant->TeamId : politika::rules::kNoTeam;
}

void APolitikaPrototypeMatch::UpdateTeamVisibility()
{
    if (!HasAuthority()) return;
    if (!bFogOfWar)
    {
        for (APolitikaPrototypeUnit* Unit : Units) if (IsValid(Unit)) Unit->ObservedByTeams = 0xFF;
        return;
    }
    TArray<politika::rules::VisionSource, TInlineAllocator<16>> Sources;
    for (const APolitikaPrototypeUnit* Unit : Units)
    {
        if (!IsValid(Unit) || !Unit->IsAlive() || Unit->GetTeamId() == politika::rules::kNoTeam) continue;
        const FVector Location = Unit->GetActorLocation();
        Sources.Add({Unit->GetTeamId(), {Location.X, Location.Y}, Unit->GetSightRadius()});
    }
    // Player teams plus the Unaligned NPC side, which keeps its own (server-only) vision.
    TArray<uint8, TInlineAllocator<4>> Teams;
    for (const FPolitikaPrototypeRuntimeParticipant& Participant : RuntimeParticipants)
        if (Participant.TeamId != politika::rules::kNoTeam) Teams.AddUnique(Participant.TeamId);
    Teams.AddUnique(politika::rules::kUnalignedTeam);
    for (APolitikaPrototypeUnit* Unit : Units)
    {
        if (!IsValid(Unit)) continue;
        const FVector Location = Unit->GetActorLocation();
        uint8 Mask = 0;
        for (const uint8 Team : Teams)
            if (Team == Unit->GetTeamId() || politika::rules::IsObservedBy(Team, {Location.X, Location.Y},
                    std::span<const politika::rules::VisionSource>(Sources.GetData(), Sources.Num())))
                Mask |= static_cast<uint8>(1u << Team);
        Unit->ObservedByTeams = Mask;
    }
    ApplyLocalFogPresentation();
}

void APolitikaPrototypeMatch::ApplyLocalFogPresentation()
{
    // The listen host has every actor in memory; it renders only what its team sees. This
    // is cosmetic. Replication filtering protects remote clients; host-side input still
    // passes the same server visibility admission as everyone else.
    if (!bFogOfWar || GetNetMode() != NM_ListenServer) return;
    const uint8 HostTeam = GetTeamForParticipant(LocalRuntimePlayerId);
    for (APolitikaPrototypeUnit* Unit : Units)
        if (IsValid(Unit)) Unit->SetLocalFogHidden(!Unit->IsObservedByTeam(HostTeam));
}

bool APolitikaPrototypeMatch::IsUnitObservedByParticipant(const APolitikaPrototypeUnit* Unit, uint32 PlayerId) const
{
    if (!Unit) return false;
    if (!bFogOfWar || Unit->GetOwnerPlayer() == PlayerId) return true;
    return Unit->IsObservedByTeam(GetTeamForParticipant(PlayerId));
}

bool APolitikaPrototypeMatch::IsUnitRelevantToViewer(const APolitikaPrototypeUnit* Unit, const AActor* RealViewer) const
{
    if (!bFogOfWar) return true;
    // Unbound connections (joining, rejected or departed) receive no unit at all.
    const uint32 ParticipantId = GetBoundParticipantId(Cast<APlayerController>(RealViewer));
    return ParticipantId != 0 && IsUnitObservedByParticipant(Unit, ParticipantId);
}

void APolitikaPrototypeMatch::RegisterObservedUnit(APolitikaPrototypeUnit* Unit)
{
    if (HasAuthority() || !IsValid(Unit)) return;
    Units.AddUnique(Unit);
    if (bObservedContentReady) Unit->TryApplyObservedPresentation();
}

void APolitikaPrototypeMatch::UnregisterObservedUnit(APolitikaPrototypeUnit* Unit)
{
    if (HasAuthority()) return;
    Units.Remove(Unit);
    for (auto It = SelectionByPlayer.CreateIterator(); It; ++It)
        if (It->Value.Get() == Unit) It.RemoveCurrent();
}

bool APolitikaPrototypeUnit::IsHostileTo(const APolitikaPrototypeUnit* Other) const
{
    return Other && Other != this
        && !politika::rules::AreAllied(OwnerPlayer, TeamId, Other->OwnerPlayer, Other->TeamId);
}

bool APolitikaPrototypeUnit::IsNetRelevantFor(const AActor* RealViewer, const AActor* ViewTarget, const FVector& SrcLocation) const
{
    const APolitikaPrototypeMatch* Match = AuthorityMatch.Get();
    if (!Match) return Super::IsNetRelevantFor(RealViewer, ViewTarget, SrcLocation);
    return Match->IsUnitRelevantToViewer(this, RealViewer);
}

void APolitikaPrototypeUnit::SetLocalFogHidden(bool bHidden)
{
    if (bLocalFogHidden == bHidden) return;
    bLocalFogHidden = bHidden;
    // bHiddenInGame on non-replicated components: independent of selection visibility
    // and never sent to clients (unlike AActor::bHidden).
    if (USceneComponent* Root = GetRootComponent()) Root->SetHiddenInGame(bHidden, true);
}

void APolitikaPrototypeUnit::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (!HasAuthority())
    {
#if !UE_BUILD_SHIPPING
        UE_LOG(LogPolitikaStartup, Display, TEXT("MP_CLIENT_UNIT_WITHDRAWN entity=%llu owner=%u team=%u alive=%d reason=%d"),
            StableId, OwnerPlayer, TeamId, IsAlive() ? 1 : 0, static_cast<int32>(EndPlayReason));
#endif
        if (APolitikaPrototypeMatch* Match = FindMatch(GetWorld())) Match->UnregisterObservedUnit(this);
    }
    Super::EndPlay(EndPlayReason);
}

APolitikaPrototypeUnit* APolitikaPrototypeMatch::FindUnitByStableId(uint64 InStableId) const
{
    if (InStableId == 0) return nullptr;
    for (APolitikaPrototypeUnit* Unit : Units)
        if (IsValid(Unit) && Unit->GetStableId() == InStableId) return Unit;
    return nullptr;
}

void APolitikaPrototypeMatch::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(NetworkReturnTimer);
        World->GetTimerManager().ClearTimer(DedicatedSessionTimer);
    }
#if !UE_BUILD_SHIPPING
    ReportNetworkMeasurements(TEXT("end_play"));
#endif
    ParticipantBindings.Reset();
    OutcomeRoutes.Reset();
    Super::EndPlay(EndPlayReason);
}

// ----------------------------------------------------------------------------
// Connection-bound participant authority (server)
// ----------------------------------------------------------------------------

APolitikaPrototypeMatch::FParticipantBinding* APolitikaPrototypeMatch::FindBinding(const APlayerController* Controller)
{
    return Controller ? ParticipantBindings.FindByPredicate([Controller](const FParticipantBinding& Binding)
        { return Binding.Controller.Get() == Controller; }) : nullptr;
}

const APolitikaPrototypeMatch::FParticipantBinding* APolitikaPrototypeMatch::FindBinding(const APlayerController* Controller) const
{
    return Controller ? ParticipantBindings.FindByPredicate([Controller](const FParticipantBinding& Binding)
        { return Binding.Controller.Get() == Controller; }) : nullptr;
}

uint32 APolitikaPrototypeMatch::GetBoundParticipantId(const APlayerController* Controller) const
{
    if (!Controller || !HasAuthority()) return 0;
    if (const FParticipantBinding* Binding = FindBinding(Controller)) return Binding->RuntimePlayerId;
    // Offline compatibility: the sole local controller of a standalone world holds
    // the authored local-human seat, exactly as before networking existed.
    if (GetNetMode() == NM_Standalone && GetWorld() && Controller == GetWorld()->GetFirstPlayerController()
        && LocalRuntimePlayerId != 0 && RuntimeParticipants.ContainsByPredicate(
            [this](const FPolitikaPrototypeRuntimeParticipant& Candidate)
            {
                return Candidate.RuntimePlayerId == LocalRuntimePlayerId
                    && Candidate.ControllerKind == EPolitikaPrototypeControllerKind::LocalHuman;
            }))
        return LocalRuntimePlayerId;
    return 0;
}

bool APolitikaPrototypeMatch::IsLocalControllerBound(const APlayerController* Source) const
{
    if (!Source) return false;
    if (!HasAuthority())
    {
        // Observer copy: input gating only. The server re-derives authority per command.
        const APolitikaPrototypePlayerController* Prototype = Cast<APolitikaPrototypePlayerController>(Source);
        return Prototype && Prototype->IsLocalController() && LocalRuntimePlayerId != 0
            && Prototype->GetAssignedRuntimePlayerId() == LocalRuntimePlayerId;
    }
    const uint32 ParticipantId = GetBoundParticipantId(Source);
    return ParticipantId != 0 && RuntimeParticipants.ContainsByPredicate(
        [ParticipantId](const FPolitikaPrototypeRuntimeParticipant& Candidate)
        { return Candidate.RuntimePlayerId == ParticipantId && IsHumanSeat(Candidate.ControllerKind); });
}

APolitikaPrototypeMatch::FParticipantBinding* APolitikaPrototypeMatch::ResolveCommandBinding(APlayerController* Source)
{
    if (FParticipantBinding* Binding = FindBinding(Source)) return Binding;
    const uint32 ImplicitParticipant = GetBoundParticipantId(Source);
    if (ImplicitParticipant == 0) return nullptr;
    // Materialize the standalone implicit binding so it receives a sequence guard.
    FParticipantBinding& Binding = ParticipantBindings.AddDefaulted_GetRef();
    Binding.Controller = Source;
    Binding.RuntimePlayerId = ImplicitParticipant;
    return &Binding;
}

void APolitikaPrototypeMatch::RefreshUnitNetOwnership(uint32 PlayerId, APlayerController* Controller)
{
    // Network ownership only selects which connection receives owner-only fields
    // (orders, ammunition, ability timers). It is never used as command authority.
    for (APolitikaPrototypeUnit* Unit : Units)
        if (IsValid(Unit) && Unit->GetOwnerPlayer() == PlayerId) Unit->SetOwner(Controller);
}

bool APolitikaPrototypeMatch::CanAdmitRemoteParticipant(FName& OutReason) const
{
    if (!HasAuthority() || !IsNetworkSession()) { OutReason = TEXT("MP.SESSION.NOT_NETWORKED"); return false; }
    if (!bContentReady || RuntimeParticipants.IsEmpty()) { OutReason = TEXT("MP.SESSION.NOT_READY"); return false; }
    if (!bAwaitingNetworkParticipants || Lifecycle.phase != politika::rules::MatchPhase::Starting)
    { OutReason = TEXT("MP.SESSION.IN_PROGRESS"); return false; }
    uint32 RemoteBindings = 0;
    for (const FParticipantBinding& Binding : ParticipantBindings) RemoteBindings += Binding.bRemote ? 1 : 0;
    const bool bOpenSeat = RuntimeParticipants.ContainsByPredicate([](const FPolitikaPrototypeRuntimeParticipant& Candidate)
        { return Candidate.ControllerKind == EPolitikaPrototypeControllerKind::AssignedOpponent; });
    if (RemoteBindings >= GetMaxRemoteHumans() || !bOpenSeat) { OutReason = TEXT("MP.SESSION.FULL"); return false; }
    OutReason = NAME_None;
    return true;
}

bool APolitikaPrototypeMatch::BindController(APlayerController* Controller, FName& OutFailure)
{
    OutFailure = NAME_None;
    if (!HasAuthority() || !IsValid(Controller)) { OutFailure = TEXT("MP.BIND.INVALID"); return false; }
    if (FindBinding(Controller)) return true;
    if (!bContentReady || RuntimeParticipants.IsEmpty()) { OutFailure = TEXT("MP.SESSION.NOT_READY"); return false; }
    const APolitikaPrototypePlayerController* PrototypeController = Cast<APolitikaPrototypePlayerController>(Controller);
    const bool bRemote = PrototypeController ? !PrototypeController->IsLocalSeatController() : !Controller->IsLocalController();
    FPolitikaPrototypeRuntimeParticipant* Seat = nullptr;
    if (!bRemote && IsDedicatedHosting())
    {
        // A dedicated server has no local seat; only remote connections play.
        OutFailure = TEXT("MP.BIND.NO_LOCAL_SEAT");
        return false;
    }
    if (!bRemote)
    {
        // The local controller occupies the authored local-human slot (host or offline player).
        if (ParticipantBindings.ContainsByPredicate([](const FParticipantBinding& Binding) { return !Binding.bRemote; }))
        { OutFailure = TEXT("MP.BIND.LOCAL_SEAT_TAKEN"); return false; }
        Seat = RuntimeParticipants.FindByPredicate([this](const FPolitikaPrototypeRuntimeParticipant& Candidate)
            { return Candidate.RuntimePlayerId == LocalRuntimePlayerId
                && Candidate.ControllerKind == EPolitikaPrototypeControllerKind::LocalHuman; });
    }
    else
    {
        if (!CanAdmitRemoteParticipant(OutFailure)) return false;
        // First open seat in session seat order (RuntimeParticipants is seat-ordered).
        Seat = RuntimeParticipants.FindByPredicate([](const FPolitikaPrototypeRuntimeParticipant& Candidate)
            { return Candidate.ControllerKind == EPolitikaPrototypeControllerKind::AssignedOpponent; });
        if (Seat) Seat->ControllerKind = EPolitikaPrototypeControllerKind::RemoteHuman;
    }
    if (!Seat) { OutFailure = TEXT("MP.SESSION.FULL"); return false; }

    FParticipantBinding& Binding = ParticipantBindings.AddDefaulted_GetRef();
    Binding.Controller = Controller;
    Binding.RuntimePlayerId = Seat->RuntimePlayerId;
    Binding.bRemote = bRemote;
    RefreshUnitNetOwnership(Seat->RuntimePlayerId, Controller);
    if (APolitikaPrototypePlayerController* Prototype = Cast<APolitikaPrototypePlayerController>(Controller))
        Prototype->SetServerAssignment(Seat->RuntimePlayerId, Seat->SlotId, Seat->FactionId, MatchGeneration);
    SyncReplicatedParticipants();
    UE_LOG(LogPolitikaStartup, Display,
        TEXT("MP_PARTICIPANT_BOUND participant=%u slot=%d faction=%s team=%u kind=%s remote=%d generation=%llu bindings=%d net_mode=%d"),
        Seat->RuntimePlayerId, Seat->SlotId, *Seat->FactionId.ToString(), Seat->TeamId, ControllerKindName(Seat->ControllerKind),
        bRemote ? 1 : 0, MatchGeneration, ParticipantBindings.Num(), static_cast<int32>(GetNetMode()));
    TryActivateNetworkMatch();
    return true;
}

void APolitikaPrototypeMatch::BindExistingControllers()
{
    UWorld* World = GetWorld();
    if (!World || !HasAuthority()) return;
    for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
    {
        APlayerController* Controller = It->Get();
        if (!Controller || FindBinding(Controller)) continue;
        FName Failure;
        if (!BindController(Controller, Failure))
            UE_LOG(LogPolitikaStartup, Warning, TEXT("MP_PARTICIPANT_BIND_DEFERRED controller=%s reason=%s"),
                *Controller->GetName(), *Failure.ToString());
    }
}

void APolitikaPrototypeMatch::TryActivateNetworkMatch()
{
    if (!bAwaitingNetworkParticipants || Lifecycle.phase != politika::rules::MatchPhase::Starting) return;
    uint32 LocalBindings = 0;
    uint32 RemoteBindings = 0;
    for (const FParticipantBinding& Binding : ParticipantBindings)
        (Binding.bRemote ? RemoteBindings : LocalBindings) += Binding.Controller.IsValid() ? 1 : 0;
    const politika::rules::HostingMode Hosting = GetHostingMode();
    if (!politika::rules::SessionSeatsComplete(SessionLayout, Hosting, LocalBindings, RemoteBindings)) return;
    bAwaitingNetworkParticipants = false;
    if (!TransitionLifecycle(politika::rules::MatchPhase::Active)) return;
    LastCommandFeedback = TEXT("All participants connected; the match is active.");
    UE_LOG(LogPolitikaStartup, Display,
        TEXT("MP_MATCH_ACTIVE generation=%llu participants=%d units=%d layout=%s fog=%d hosting=%s local=%u remote=%u"),
        MatchGeneration, ParticipantBindings.Num(), Units.Num(),
        UTF8_TO_TCHAR(politika::rules::LayoutName(SessionLayout).data()), bFogOfWar ? 1 : 0,
        UTF8_TO_TCHAR(politika::rules::HostingModeName(Hosting).data()), LocalBindings, RemoteBindings);
#if !UE_BUILD_SHIPPING
    SampleMultiplayerState(TEXT("activated"));
    // Network measurements cover the active match only; the activation sample is the
    // baseline the harness subtracts from the final engine counters.
    StepCostMicros.Reset();
    ServerFrameMillis.Reset();
    CatchUpCapFrames = 0;
    bNetMeasurementsReported = false;
    MeasurementStartSeconds = FPlatformTime::Seconds();
    LogNetworkEmulation();
    SampleNetworkStats(TEXT("activated"));
#endif
}

void APolitikaPrototypeMatch::HandleControllerLogout(AController* Exiting)
{
    APlayerController* Controller = Cast<APlayerController>(Exiting);
    if (!HasAuthority() || !Controller) return;
    const int32 Index = ParticipantBindings.IndexOfByPredicate([Controller](const FParticipantBinding& Binding)
        { return Binding.Controller.Get() == Controller; });
    if (Index == INDEX_NONE) return;
    const FParticipantBinding Binding = ParticipantBindings[Index];
    ParticipantBindings.RemoveAt(Index);
    for (auto It = OutcomeRoutes.CreateIterator(); It; ++It)
        if (It->Value.Controller.Get() == Controller) It.RemoveCurrent();
    RefreshUnitNetOwnership(Binding.RuntimePlayerId, nullptr);
    if (Binding.bRemote)
        if (FPolitikaPrototypeRuntimeParticipant* Seat = RuntimeParticipants.FindByPredicate(
            [&Binding](const FPolitikaPrototypeRuntimeParticipant& Candidate)
            { return Candidate.RuntimePlayerId == Binding.RuntimePlayerId; }))
            Seat->ControllerKind = EPolitikaPrototypeControllerKind::AssignedOpponent;
    if (auto* Prototype = Cast<APolitikaPrototypePlayerController>(Controller)) Prototype->ClearServerAssignment();
    SyncReplicatedParticipants();
    UE_LOG(LogPolitikaStartup, Display, TEXT("MP_PARTICIPANT_UNBOUND participant=%u remote=%d phase=%u generation=%llu"),
        Binding.RuntimePlayerId, Binding.bRemote ? 1 : 0, static_cast<uint32>(Lifecycle.phase), MatchGeneration);
    if (!GetWorld() || GetWorld()->bIsTearingDown) return;
    if (Binding.bRemote && Lifecycle.IsActive())
        TerminateForParticipantDisconnect(Binding.RuntimePlayerId);
    else if (IsDedicatedHosting() && Lifecycle.phase == politika::rules::MatchPhase::Results
        && ParticipantBindings.IsEmpty())
        ScheduleDedicatedSessionClose(TEXT("results_complete"), DedicatedEmptySessionResetSeconds);
}

void APolitikaPrototypeMatch::TerminateForParticipantDisconnect(uint32 PlayerId)
{
    if (!Lifecycle.IsActive()) return;
    if (!TransitionLifecycle(politika::rules::MatchPhase::Terminal)
        || !TransitionLifecycle(politika::rules::MatchPhase::Results))
        return;
    if (!CommittedResult.IsSet())
        CommittedResult.Emplace(FPolitikaPrototypeMatchResult{0, TEXT("participant_disconnected")});
    SyncReplicatedLifecycle();
    LastCommandFeedback = TEXT("The remote participant disconnected; the match has ended.");
    UE_LOG(LogPolitikaStartup, Display,
        TEXT("MATCH_RESULTS outcome=%s player=%u committed=1 generation=%llu step=%llu departed=%u"),
        *CommittedResult->Reason.ToString(), CommittedResult->WinningPlayerId, MatchGeneration,
        SimulationStepsExecuted, PlayerId);
#if !UE_BUILD_SHIPPING
    ReportNetworkMeasurements(TEXT("participant_disconnected"));
#endif
    if (IsDedicatedHosting())
    {
        // No host to return: remaining clients see the result, then the server closes the
        // session and resets for a new match.
        ScheduleDedicatedSessionClose(TEXT("participant_disconnected"), ParticipantDisconnectReturnSeconds);
        return;
    }
    if (UPolitikaGameInstance* Game = GetWorld()->GetGameInstance<UPolitikaGameInstance>())
        Game->ReportNetworkSessionEnded(TEXT("NetworkParticipantDisconnected"), NSLOCTEXT("PolitikaMultiplayer",
            "ParticipantDisconnected", "The remote participant disconnected, so the network match was ended. Start a new session to play again."));
    GetWorld()->GetTimerManager().SetTimer(NetworkReturnTimer, FTimerDelegate::CreateWeakLambda(this, [this]
    {
        APlayerController* Host = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
        UE_LOG(LogPolitikaStartup, Display, TEXT("MP_HOST_RETURNING reason=participant_disconnected generation=%llu"),
            MatchGeneration);
        BeginReturnToMenu();
        if (UPolitikaGameInstance* Game = GetWorld() ? GetWorld()->GetGameInstance<UPolitikaGameInstance>() : nullptr)
            Game->ReturnToMenu(Host);
    }), ParticipantDisconnectReturnSeconds, false);
}

// ----------------------------------------------------------------------------
// Dedicated server session close and reset
// ----------------------------------------------------------------------------

void APolitikaPrototypeMatch::NoteNetworkResultCommitted()
{
    if (HasAuthority() && IsDedicatedHosting() && CommittedResult.IsSet())
        ScheduleDedicatedSessionClose(TEXT("results_linger_elapsed"), DedicatedResultsLingerSeconds);
}

void APolitikaPrototypeMatch::ScheduleDedicatedSessionClose(FName Reason, float DelaySeconds)
{
    UWorld* World = GetWorld();
    if (!HasAuthority() || !IsDedicatedHosting() || !World || bDedicatedSessionClosing) return;
    FTimerManager& Timers = World->GetTimerManager();
    // Keep an earlier pending close; a later request never postpones it.
    if (Timers.IsTimerActive(DedicatedSessionTimer) && Timers.GetTimerRemaining(DedicatedSessionTimer) <= DelaySeconds)
        return;
    DedicatedCloseReason = Reason;
    Timers.SetTimer(DedicatedSessionTimer, FTimerDelegate::CreateWeakLambda(this, [this] { CloseDedicatedSession(); }),
        DelaySeconds, false);
    UE_LOG(LogPolitikaStartup, Display, TEXT("MP_DEDICATED_SESSION_CLOSE_SCHEDULED reason=%s delay=%.1f generation=%llu bindings=%d"),
        *Reason.ToString(), DelaySeconds, MatchGeneration, ParticipantBindings.Num());
}

void APolitikaPrototypeMatch::CloseDedicatedSession()
{
    UWorld* World = GetWorld();
    if (!HasAuthority() || !World || World->bIsTearingDown || bDedicatedSessionClosing) return;
    bDedicatedSessionClosing = true;
    const uint64 ClosedGeneration = MatchGeneration;
    TArray<TWeakObjectPtr<APlayerController>, TInlineAllocator<4>> Remaining;
    for (const FParticipantBinding& Binding : ParticipantBindings) Remaining.Add(Binding.Controller);
    UE_LOG(LogPolitikaStartup, Display, TEXT("MP_DEDICATED_SESSION_CLOSING reason=%s generation=%llu remaining=%d phase=%u"),
        *DedicatedCloseReason.ToString(), ClosedGeneration, Remaining.Num(), static_cast<uint32>(Lifecycle.phase));
    // Revoke every binding and outcome route first, so nothing from this generation can
    // reach the next match, then remove the clients that are still connected.
    BeginReturnToMenu();
    AGameModeBase* GameMode = World->GetAuthGameMode();
    for (const TWeakObjectPtr<APlayerController>& Controller : Remaining)
        if (Controller.IsValid() && GameMode && GameMode->GameSession)
            GameMode->GameSession->KickPlayer(Controller.Get(), NSLOCTEXT("PolitikaMultiplayer", "DedicatedSessionClosed",
                "The network match has ended and the dedicated server closed the session."));
#if !UE_BUILD_SHIPPING
    if (UPolitikaGameInstance* Game = World->GetGameInstance<UPolitikaGameInstance>()) Game->RearmDevelopmentTestFixture();
#endif
    // Reload the same map and options (port and layout are unchanged) for a fresh match
    // generation. The world, its controllers and this coordinator are discarded.
    FString ResetUrl = World->URL.Map;
    for (const FString& Option : World->URL.Op) ResetUrl += TEXT("?") + Option;
    World->GetTimerManager().SetTimer(DedicatedSessionTimer, FTimerDelegate::CreateWeakLambda(this, [this, ResetUrl, ClosedGeneration]
    {
        UWorld* ResetWorld = GetWorld();
        if (!ResetWorld || ResetWorld->bIsTearingDown) return;
        const bool bTravel = ResetWorld->ServerTravel(ResetUrl, true);
        UE_LOG(LogPolitikaStartup, Display, TEXT("MP_DEDICATED_SESSION_RESET url=%s closed_generation=%llu travel=%d"),
            *ResetUrl, ClosedGeneration, bTravel ? 1 : 0);
    }), DedicatedKickFlushSeconds, false);
}

// ----------------------------------------------------------------------------
// Single command intake (server)
// ----------------------------------------------------------------------------

bool APolitikaPrototypeMatch::SubmitLocalControllerCommand(APlayerController* Source,
    APolitikaPrototypeUnit* Unit, APolitikaPrototypeUnit* Target, const politika::rules::Order& Intent)
{
    FParticipantBinding* Binding = HasAuthority() ? ResolveCommandBinding(Source) : nullptr;
    if (!Binding)
    {
        LastCommandFeedback = TEXT("This controller is not bound to a playable participant.");
        return false;
    }
    FPolitikaNetCommandIntent Request;
    Request.ProtocolVersion = static_cast<int32>(FPolitikaBootstrap::Get().NetworkProtocol());
    FillIntentParameters(Request, Intent);
    // A destroyed actor still carries the identity the caller meant; resolution fails closed.
    Request.IssuerEntityId = Unit ? Unit->GetStableId() : 0;
    if (Target && Request.TargetEntityId == 0) Request.TargetEntityId = Target->GetStableId();
    APolitikaPrototypePlayerController* Prototype = Cast<APolitikaPrototypePlayerController>(Source);
    Request.ClientSequence = Prototype ? Prototype->NextClientSequence++ : Binding->Sequence.last_seen + 1;
    Request.CorrelationId = 0; // Server sequence doubles as correlation for adapter callers.
    Request.MatchGeneration = MatchGeneration;
    return SubmitControllerIntent(Source, Request);
}

void APolitikaPrototypeMatch::RejectIntentAtIntake(APlayerController* Source, const FPolitikaNetCommandIntent& Intent,
    uint32 ParticipantId, politika::rules::OrderRejection Rejection)
{
    using namespace politika::rules;
    const uint64 Sequence = NextCommandSequence++;
    OutcomeRoutes.Add(Sequence, {Source, Intent.ClientSequence, Intent.CorrelationId, MatchGeneration, Intent.OrderKind});
    UE_LOG(LogPolitikaStartup, Display,
        TEXT("MP_COMMAND_REJECTED_AT_INTAKE participant=%u correlation=%u client_sequence=%u order=%u issuer=%llu rejection=%u server_sequence=%llu"),
        ParticipantId, Intent.CorrelationId, Intent.ClientSequence, Intent.OrderKind, Intent.IssuerEntityId,
        static_cast<uint32>(Rejection), Sequence);
    const OrderKind SafeKind = Intent.OrderKind < kOrderKindCount ? static_cast<OrderKind>(Intent.OrderKind) : OrderKind::Stop;
    PublishGameplayEvent(CommandOutcomeEvent{SimulationStepsExecuted, Sequence,
        Intent.CorrelationId != 0 ? Intent.CorrelationId : Sequence, Intent.IssuerEntityId, SafeKind, false, Rejection});
}

bool APolitikaPrototypeMatch::SubmitControllerIntent(APlayerController* Source, const FPolitikaNetCommandIntent& Intent)
{
    using namespace politika::rules;
    if (!HasAuthority()) return false;
    // 1-3. Identify the sending connection and its committed participant binding. The
    // payload has no participant field to trust or spoof.
    FParticipantBinding* Binding = ResolveCommandBinding(Source);
    if (!Binding)
    {
        LastCommandFeedback = TEXT("This controller is not bound to a playable participant.");
        UE_LOG(LogPolitikaStartup, Display, TEXT("MP_COMMAND_UNBOUND controller=%s correlation=%u client_sequence=%u"),
            Source ? *Source->GetName() : TEXT("none"), Intent.CorrelationId, Intent.ClientSequence);
        if (APolitikaPrototypePlayerController* Prototype = Cast<APolitikaPrototypePlayerController>(Source);
            Prototype && !Prototype->IsLocalSeatController())
        {
            FPolitikaNetCommandOutcome Outcome;
            Outcome.CorrelationId = Intent.CorrelationId;
            Outcome.ClientSequence = Intent.ClientSequence;
            Outcome.IssuerEntityId = Intent.IssuerEntityId;
            Outcome.MatchGeneration = MatchGeneration;
            Outcome.OrderKind = Intent.OrderKind;
            Outcome.Rejection = static_cast<uint8>(OrderRejection::Unauthorized);
            Prototype->ClientReceiveCommandOutcome(Outcome);
        }
        return false;
    }
    const uint32 ParticipantId = Binding->RuntimePlayerId;
    // 4-6. Protocol version, freshness (single-use client sequence) and payload shape.
    if (Intent.ProtocolVersion != static_cast<int32>(FPolitikaBootstrap::Get().NetworkProtocol()))
    {
        RejectIntentAtIntake(Source, Intent, ParticipantId, OrderRejection::MalformedCommand);
        return true;
    }
    if (const OrderResult Fresh = Binding->Sequence.Consume(Intent.ClientSequence); !Fresh)
    {
        RejectIntentAtIntake(Source, Intent, ParticipantId, Fresh.rejection);
        return true;
    }
    Order Decoded = StopOrder{};
    if (const OrderResult Shape = DecodeIntent({Intent.OrderKind, Intent.TargetEntityId, Intent.DestinationX,
        Intent.DestinationY}, Decoded); !Shape)
    {
        RejectIntentAtIntake(Source, Intent, ParticipantId, Shape.rejection);
        return true;
    }
    int32 PendingForParticipant = 0;
    for (const FPendingOrder& Pending : PendingOrders)
        PendingForParticipant += Pending.Request.participant_id == ParticipantId ? 1 : 0;
    if (PendingForParticipant >= MaxPendingCommandsPerParticipant)
    {
        RejectIntentAtIntake(Source, Intent, ParticipantId, OrderRejection::RateLimited);
        return true;
    }
    // 7-9. Resolve entity IDs in the current match only, then submit through the existing
    // typed pipeline. Ownership, phase, generation, liveness, bounds and ability checks
    // remain the shared kernel's responsibility.
    APolitikaPrototypeUnit* Issuer = FindUnitByStableId(Intent.IssuerEntityId);
    APolitikaPrototypeUnit* Target = FindUnitByStableId(Intent.TargetEntityId);
    const uint64 ExpectedSequence = NextCommandSequence;
    OutcomeRoutes.Add(ExpectedSequence, {Source, Intent.ClientSequence, Intent.CorrelationId, MatchGeneration, Intent.OrderKind});
#if !UE_BUILD_SHIPPING
    UE_LOG(LogPolitikaStartup, Display,
        TEXT("MP_COMMAND_RECEIVED participant=%u remote=%d correlation=%u client_sequence=%u order=%u issuer=%llu target=%llu generation_hint=%llu server_sequence=%llu"),
        ParticipantId, Binding->bRemote ? 1 : 0, Intent.CorrelationId, Intent.ClientSequence, Intent.OrderKind,
        Intent.IssuerEntityId, Intent.TargetEntityId, Intent.MatchGeneration, ExpectedSequence);
#endif
    const uint64 AssignedSequence = SubmitCommand(ParticipantId, Issuer, Target, Decoded, Intent.IssuerEntityId,
        Intent.CorrelationId, Intent.MatchGeneration);
    ensureMsgf(AssignedSequence == ExpectedSequence, TEXT("Command sequence routing drifted"));
    return true;
}

void APolitikaPrototypeMatch::RouteCommandOutcome(const politika::rules::CommandOutcomeEvent& Outcome)
{
    FOutcomeRoute Route;
    if (!OutcomeRoutes.RemoveAndCopyValue(Outcome.sequence, Route)) return;
    APolitikaPrototypePlayerController* Controller = Cast<APolitikaPrototypePlayerController>(Route.Controller.Get());
    if (!Controller) return;
    FPolitikaNetCommandOutcome Message;
    Message.CorrelationId = Route.CorrelationId != 0 ? Route.CorrelationId : static_cast<uint32>(Outcome.sequence);
    Message.ClientSequence = Route.ClientSequence;
    Message.IssuerEntityId = Outcome.issuer_id;
    Message.MatchGeneration = Route.Generation;
    Message.Step = Outcome.step;
    Message.OrderKind = Route.RawOrderKind;
    Message.Rejection = static_cast<uint8>(Outcome.rejection);
    Message.bAccepted = Outcome.accepted;
    // Only the issuing connection receives the result; a local controller is
    // handled in-process, a remote one through its owning reliable client RPC.
    if (Controller->IsLocalSeatController()) Controller->ClientReceiveCommandOutcome_Implementation(Message);
    else Controller->ClientReceiveCommandOutcome(Message);
}

bool APolitikaPrototypePlayerController::IsLocalSeatController() const
{
#if WITH_DEV_AUTOMATION_TESTS
    if (bTestLocalSeat) return true;
#endif
    return IsLocalController();
}

// ----------------------------------------------------------------------------
// Remote observer initialization
// ----------------------------------------------------------------------------

void APolitikaPrototypeMatch::SetLocalViewerParticipant(uint32 PlayerId)
{
    if (HasAuthority() || LocalRuntimePlayerId == PlayerId) return;
    LocalRuntimePlayerId = PlayerId;
    for (auto& Selection : SelectionByPlayer)
        if (APolitikaPrototypeUnit* Unit = Selection.Value.Get()) Unit->SetSelectedVisual(false);
    SelectionByPlayer.Reset();
}

void APolitikaPrototypeMatch::InitializeObservedMatch()
{
    if (HasAuthority() || bObservedContentReady || ReplicatedMapId.IsNone() || !GetWorld()) return;
    FPolitikaPrototypeContentSnapshot Snapshot;
    TArray<FPolitikaContentDiagnostic> Diagnostics;
    if (!FPolitikaPrototypeContentLoader::LoadAndValidate(GetWorld(), ReplicatedMapId, Snapshot, Diagnostics))
    {
        for (const FPolitikaContentDiagnostic& Diagnostic : Diagnostics)
            UE_LOG(LogPolitikaStartup, Error, TEXT("CONTENT_VALIDATION %s"), *UPolitikaPrototypeContentValidator::FormatDiagnostic(Diagnostic));
        UE_LOG(LogPolitikaStartup, Error, TEXT("MP_CLIENT_CONTENT_MISMATCH map=%s generation=%llu"),
            *ReplicatedMapId.ToString(), MatchGeneration);
        if (UPolitikaGameInstance* Game = GetWorld()->GetGameInstance<UPolitikaGameInstance>())
        {
            Game->ReportNetworkSessionEnded(TEXT("NetworkContentMismatch"), NSLOCTEXT("PolitikaMultiplayer",
                "ContentMismatch", "This build cannot load the host's match content. Both players must run the same POLITIKA build."));
            Game->ReturnToMenu(GetWorld()->GetFirstPlayerController());
        }
        return;
    }
    ContentCatalog = Snapshot.Catalog;
    MapDefinition = Snapshot.MapDefinition;
    PlayableMin = MapDefinition->PlayableMin;
    PlayableMax = MapDefinition->PlayableMax;
    bObservedContentReady = true;
    ApplyMapPresentation();
    OnRep_Objective();
    OnRep_Participants();
    OnRep_Lifecycle();
    ApplyObservedPresentationToUnits();
    UE_LOG(LogPolitikaStartup, Display, TEXT("MP_CLIENT_MATCH_OBSERVED map=%s generation=%llu units=%d objective=%d phase=%u"),
        *ReplicatedMapId.ToString(), MatchGeneration, Units.Num(), Objective ? 1 : 0, ReplicatedPhase);
#if !UE_BUILD_SHIPPING
    MeasurementStartSeconds = FPlatformTime::Seconds();
    LogNetworkEmulation();
#endif
}

void APolitikaPrototypeMatch::ApplyObservedPresentationToUnits()
{
    if (HasAuthority() || !bObservedContentReady) return;
    for (TActorIterator<APolitikaPrototypeUnit> It(GetWorld()); It; ++It) It->TryApplyObservedPresentation();
}

void APolitikaPrototypeMatch::OnRep_MapDefinitionId()
{
    InitializeObservedMatch();
}

void APolitikaPrototypeMatch::OnRep_Objective()
{
    if (HasAuthority() || !Objective || !MapDefinition) return;
    if (!Objective->HasMarkerPresentation()) Objective->ConfigurePresentation(MapDefinition->Objective);
}

void APolitikaPrototypeMatch::OnRep_Participants()
{
    if (HasAuthority()) return;
    RuntimeParticipants.Reset();
    for (const FPolitikaNetParticipantInfo& Info : ReplicatedParticipants)
    {
        FPolitikaPrototypeRuntimeParticipant& Participant = RuntimeParticipants.AddDefaulted_GetRef();
        Participant.SlotId = Info.SlotId;
        Participant.RuntimePlayerId = Info.RuntimePlayerId;
        Participant.FactionId = Info.FactionId;
        Participant.ControllerKind = Info.ControllerKind <= static_cast<uint8>(EPolitikaPrototypeControllerKind::RemoteHuman)
            ? static_cast<EPolitikaPrototypeControllerKind>(Info.ControllerKind) : EPolitikaPrototypeControllerKind::AssignedOpponent;
        Participant.TeamId = Info.TeamId;
    }
    // Presentation mirror of the server's session policy; never consulted for authority.
    SessionLayout = ReplicatedLayout <= static_cast<uint8>(politika::rules::TeamLayout::TwoVsTwo)
        ? static_cast<politika::rules::TeamLayout>(ReplicatedLayout) : politika::rules::TeamLayout::OneVsOne;
    SessionSeatCount = RuntimeParticipants.Num();
    bFogOfWar = bReplicatedFogOfWar;
}

void APolitikaPrototypeMatch::OnRep_Lifecycle()
{
    if (HasAuthority()) return;
    // Observer mirror of the server's lifecycle; validated transitions happen only on authority.
    Lifecycle.phase = static_cast<politika::rules::MatchPhase>(FMath::Min<uint8>(ReplicatedPhase,
        static_cast<uint8>(politika::rules::MatchPhase::AdmissionRejected)));
    if (bReplicatedResultCommitted)
        CommittedResult = FPolitikaPrototypeMatchResult{ReplicatedResultWinner, ReplicatedResultReason, ReplicatedResultTeam};
    else
        CommittedResult.Reset();
    if (UPolitikaGameInstance* Game = GetWorld() ? GetWorld()->GetGameInstance<UPolitikaGameInstance>() : nullptr)
        Game->NoteNetworkMatchObserved(MatchGeneration, ReplicatedPhase, bReplicatedResultCommitted,
            ReplicatedResultWinner, ReplicatedResultReason, LocalRuntimePlayerId);
#if !UE_BUILD_SHIPPING
    UE_LOG(LogPolitikaStartup, Display, TEXT("MP_CLIENT_LIFECYCLE phase=%u result=%d winner=%u team=%u reason=%s generation=%llu viewer=%u viewer_team=%u"),
        ReplicatedPhase, bReplicatedResultCommitted ? 1 : 0, ReplicatedResultWinner, ReplicatedResultTeam,
        *ReplicatedResultReason.ToString(), MatchGeneration, LocalRuntimePlayerId, GetLocalViewerTeam());
    SampleObservedState(TEXT("lifecycle"));
#endif
}

void APolitikaPrototypeUnit::BeginPlay()
{
    Super::BeginPlay();
    if (HasAuthority()) return;
    if (APolitikaPrototypeMatch* Match = FindMatch(GetWorld())) Match->RegisterObservedUnit(this);
    TryApplyObservedPresentation();
#if !UE_BUILD_SHIPPING
    UE_LOG(LogPolitikaStartup, Display, TEXT("MP_CLIENT_UNIT_RELEVANT entity=%llu owner=%u team=%u"), StableId, OwnerPlayer, TeamId);
#endif
}

void APolitikaPrototypeUnit::TryApplyObservedPresentation()
{
    if (HasAuthority() || bObservedPresentationApplied || ArchetypeId.IsNone()) return;
    APolitikaPrototypeMatch* Match = FindMatch(GetWorld());
    const UPolitikaPrototypeContentCatalog* Catalog = Match ? Match->GetContentCatalog() : nullptr;
    if (!Catalog) return;
    ApplyObservedArchetypePresentation(Catalog->FindUnit(ArchetypeId));
}

void APolitikaPrototypeUnit::OnRep_ArchetypeId()
{
    TryApplyObservedPresentation();
}

void APolitikaPrototypeUnit::OnRep_Health()
{
#if !UE_BUILD_SHIPPING
    UE_LOG(LogPolitikaStartup, Display, TEXT("MP_CLIENT_UNIT_HEALTH entity=%llu owner=%u health=%d max=%d"),
        StableId, OwnerPlayer, Health, MaxHealth);
#endif
    if (!HasAuthority() && Health <= 0 && MaxHealth > 0 && !bDeathPresented)
    {
#if !UE_BUILD_SHIPPING
        UE_LOG(LogPolitikaStartup, Display, TEXT("MP_CLIENT_UNIT_DESTROYED entity=%llu owner=%u blast=%d"),
            StableId, OwnerPlayer, bBlastDeath ? 1 : 0);
#endif
        BeginDeathPresentation();
    }
}

politika::rules::ShotCue APolitikaPrototypeMatch::ResolveShotCue(const APolitikaPrototypeUnit* Shooter,
    const APolitikaPrototypeUnit* Target, uint32 ViewerParticipant, FVector& OutPoint) const
{
    using politika::rules::ShotCue;
    OutPoint = FVector::ZeroVector;
    if (!Shooter || !Target) return ShotCue::None;
    // Owner-0 (Unaligned) units would otherwise count as "owned" by participant 0.
    const bool bViewer = ViewerParticipant != 0;
    const ShotCue Cue = politika::rules::ShotCueFor(bFogOfWar,
        bViewer && IsUnitObservedByParticipant(Shooter, ViewerParticipant),
        bViewer && IsUnitObservedByParticipant(Target, ViewerParticipant));
    if (Cue == ShotCue::Exact) OutPoint = Target->GetActorLocation();
    // Facing is already public through the shooter's replicated rotation; the distance
    // is fixed so the cue says nothing about where (or whether) the target is.
    else if (Cue == ShotCue::FacingOnly)
        OutPoint = Shooter->GetActorLocation() + Shooter->GetActorForwardVector().GetSafeNormal2D() * ShotFacingCueDistanceCm;
    return Cue;
}

void APolitikaPrototypeMatch::DistributeShotPresentation(APolitikaPrototypeUnit* Shooter,
    const APolitikaPrototypeUnit* Target, float Cooldown)
{
    using politika::rules::ShotCue;
    if (!HasAuthority() || !Shooter || !Target) return;
    const uint32 Sequence = ++ShotCueSequence;
    // This process's own view: the listen host sees through its seat's team; standalone
    // play has full vision; a dedicated server has no viewer and keeps the exact point.
    FVector LocalPoint;
    const bool bListen = GetHostingMode() == politika::rules::HostingMode::Listen;
    const ShotCue LocalCue = bListen ? ResolveShotCue(Shooter, Target, LocalRuntimePlayerId, LocalPoint)
        : (LocalPoint = Target->GetActorLocation(), ShotCue::Exact);
    if (LocalCue != ShotCue::None) Shooter->InfantryPresentation->Shot(LocalPoint, Cooldown);
#if WITH_DEV_AUTOMATION_TESTS
    if (bTestCaptureShotCues) TestShotCues.Add({0, LocalCue, LocalPoint});
#endif
    if (!IsNetworkSession()) return;
    for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
    {
        APolitikaPrototypePlayerController* Controller = Cast<APolitikaPrototypePlayerController>(It->Get());
        if (!Controller || Controller->IsLocalSeatController()) continue;
        const uint32 Participant = GetBoundParticipantId(Controller);
        if (Participant == 0) continue;
        FVector Point;
        const ShotCue Cue = ResolveShotCue(Shooter, Target, Participant, Point);
#if !UE_BUILD_SHIPPING
        const FVector TargetLocation = Target->GetActorLocation();
        UE_LOG(LogPolitikaStartup, Display,
            TEXT("MP_SHOT_CUE_SENT sequence=%u participant=%u shooter=%llu target=%llu cue=%s target_x=%.1f target_y=%.1f point_x=%.1f point_y=%.1f generation=%llu"),
            Sequence, Participant, Shooter->GetStableId(), Target->GetStableId(),
            *FString(politika::rules::ShotCueName(Cue).data()), TargetLocation.X, TargetLocation.Y, Point.X, Point.Y,
            MatchGeneration);
#endif
#if WITH_DEV_AUTOMATION_TESTS
        if (bTestCaptureShotCues) { TestShotCues.Add({Participant, Cue, Point}); continue; }
#endif
        if (Cue == ShotCue::None) continue;
        Controller->ClientShotPresentation(Shooter, Point, Cue == ShotCue::Exact, Cooldown, Sequence);
    }
}

void APolitikaPrototypePlayerController::ClientShotPresentation_Implementation(APolitikaPrototypeUnit* Shooter,
    FVector_NetQuantize Point, bool bTargetObserved, float Cooldown, uint32 Sequence)
{
    // The shooter may have left this connection's relevancy since the server sent the cue.
    if (!IsValid(Shooter) || Shooter->HasAuthority()) return;
#if !UE_BUILD_SHIPPING
    UE_LOG(LogPolitikaStartup, Display,
        TEXT("MP_CLIENT_SHOT_CUE sequence=%u participant=%u shooter=%llu exact=%d point_x=%.1f point_y=%.1f"),
        Sequence, GetCommandPlayerId(), Shooter->GetStableId(), bTargetObserved ? 1 : 0, Point.X, Point.Y);
#endif
    Shooter->PresentObservedShot(Point, Cooldown);
}

void APolitikaPrototypeUnit::PresentObservedShot(const FVector& Point, float Cooldown)
{
    if (!HasAuthority()) InfantryPresentation->Shot(Point, Cooldown);
}

void APolitikaPrototypeUnit::MulticastAbilityPresentation_Implementation(uint8 Cue, float Seconds)
{
    if (HasAuthority()) return;
    if (Cue == 0) InfantryPresentation->Reload(Seconds);
    else if (Cue == 1) InfantryPresentation->Grenade(Seconds);
}

void APolitikaPrototypeUnit::PlayLocalAcknowledgement(FName Event)
{
    // Order acknowledgements are heard only by the participant who issued the order.
    const APlayerController* Controller = Cast<APlayerController>(GetOwner());
    const bool bLocalViewer = Controller ? Controller->IsLocalController() : GetNetMode() == NM_Standalone;
    if (bLocalViewer) InfantryPresentation->Acknowledge(Event);
}

// ----------------------------------------------------------------------------
// PlayerController ingress and owner-only assignment
// ----------------------------------------------------------------------------

void APolitikaPrototypePlayerController::SetServerAssignment(uint32 RuntimePlayerId, int32 SlotId, FName FactionId,
    uint64 Generation)
{
    if (!HasAuthority()) return;
    AssignedRuntimePlayerId = RuntimePlayerId;
    AssignedSlotId = SlotId;
    AssignedFactionId = FactionId;
    AssignedMatchGeneration = Generation;
}

void APolitikaPrototypePlayerController::ClearServerAssignment()
{
    if (!HasAuthority()) return;
    AssignedRuntimePlayerId = 0;
    AssignedSlotId = 0;
    AssignedFactionId = NAME_None;
    AssignedMatchGeneration = 0;
    SelectedUnit.Reset();
}

void APolitikaPrototypePlayerController::OnRep_Assignment()
{
    UE_LOG(LogPolitikaStartup, Display, TEXT("MP_CLIENT_ASSIGNMENT participant=%u slot=%d faction=%s generation=%llu"),
        AssignedRuntimePlayerId, AssignedSlotId, *AssignedFactionId.ToString(), AssignedMatchGeneration);
    if (APolitikaPrototypeMatch* ActiveMatch = FindMatch(GetWorld()))
        ActiveMatch->SetLocalViewerParticipant(AssignedRuntimePlayerId);
    if (AssignedRuntimePlayerId == 0) SelectedUnit.Reset();
}

uint32 APolitikaPrototypePlayerController::GetCommandPlayerId() const
{
    if (!Match.IsValid()) return 0;
    return Match->HasAuthority() ? Match->GetBoundParticipantId(this) : AssignedRuntimePlayerId;
}

bool APolitikaPrototypePlayerController::SubmitIntent(APolitikaPrototypeMatch* ActiveMatch, APolitikaPrototypeUnit* Unit,
    APolitikaPrototypeUnit* Target, const politika::rules::Order& Order)
{
    if (!ActiveMatch) return false;
    FPolitikaNetCommandIntent Intent;
    Intent.ProtocolVersion = static_cast<int32>(FPolitikaBootstrap::Get().NetworkProtocol());
    FillIntentParameters(Intent, Order);
    Intent.IssuerEntityId = Unit ? Unit->GetStableId() : 0;
    if (Target && Intent.TargetEntityId == 0 && Intent.OrderKind != static_cast<uint8>(politika::rules::OrderKind::Move)
        && Intent.OrderKind != static_cast<uint8>(politika::rules::OrderKind::AttackMove))
        Intent.TargetEntityId = Target->GetStableId();
    Intent.ClientSequence = NextClientSequence++;
    Intent.CorrelationId = NextCorrelationId++;
    Intent.MatchGeneration = ActiveMatch->GetMatchGeneration();
#if !UE_BUILD_SHIPPING
    UE_LOG(LogPolitikaStartup, Display,
        TEXT("RTS_COMMAND_INTENT_SENT participant=%u correlation=%u client_sequence=%u order=%u issuer=%llu target=%llu authority=%d"),
        GetCommandPlayerId(), Intent.CorrelationId, Intent.ClientSequence, Intent.OrderKind, Intent.IssuerEntityId,
        Intent.TargetEntityId, HasAuthority() ? 1 : 0);
#endif
    // Host and standalone input call the server intake in-process; a remote client
    // sends the same payload through its owning connection. Both reach one method.
    if (HasAuthority()) return ActiveMatch->SubmitControllerIntent(this, Intent);
    ServerSubmitCommandIntent(Intent);
    return true;
}

void APolitikaPrototypePlayerController::ServerSubmitCommandIntent_Implementation(const FPolitikaNetCommandIntent& Intent)
{
    if (APolitikaPrototypeMatch* ActiveMatch = FindMatch(GetWorld()))
        ActiveMatch->SubmitControllerIntent(this, Intent);
}

void APolitikaPrototypePlayerController::ClientReceiveCommandOutcome_Implementation(const FPolitikaNetCommandOutcome& Outcome)
{
#if !UE_BUILD_SHIPPING
    UE_LOG(LogPolitikaStartup, Display,
        TEXT("MP_COMMAND_OUTCOME participant=%u correlation=%u client_sequence=%u order=%u issuer=%llu accepted=%d rejection=%u generation=%llu step=%llu authority=%d"),
        GetCommandPlayerId(), Outcome.CorrelationId, Outcome.ClientSequence, Outcome.OrderKind, Outcome.IssuerEntityId,
        Outcome.bAccepted ? 1 : 0, Outcome.Rejection, Outcome.MatchGeneration, Outcome.Step, HasAuthority() ? 1 : 0);
#endif
    if (APolitikaPrototypeHUD* PrototypeHUD = Cast<APolitikaPrototypeHUD>(GetHUD()))
        PrototypeHUD->ShowCommandOutcome(Outcome);
    // A remote participant hears its own unit acknowledge; authority already played it.
    if (!HasAuthority() && Outcome.bAccepted)
        if (APolitikaPrototypeMatch* ActiveMatch = FindMatch(GetWorld()))
            for (APolitikaPrototypeUnit* Unit : ActiveMatch->GetUnits())
                if (Unit && Unit->GetStableId() == Outcome.IssuerEntityId
                    && Unit->GetOwnerPlayer() == AssignedRuntimePlayerId)
                {
                    const auto Kind = static_cast<politika::rules::OrderKind>(Outcome.OrderKind);
                    if (Kind == politika::rules::OrderKind::Move || Kind == politika::rules::OrderKind::AttackMove)
                        Unit->PlayLocalAcknowledgement(TEXT("move"));
                    else if (Kind == politika::rules::OrderKind::Attack)
                        Unit->PlayLocalAcknowledgement(TEXT("attack"));
                    break;
                }
}

void APolitikaPrototypePlayerController::ClientWasKicked_Implementation(const FText& KickReason)
{
    UE_LOG(LogPolitikaStartup, Warning, TEXT("MP_CLIENT_KICKED reason=\"%s\""), *KickReason.ToString());
    if (UPolitikaGameInstance* Game = GetGameInstance<UPolitikaGameInstance>()) Game->NoteNetworkKick(KickReason);
}

void APolitikaPrototypePlayerController::ShowLocalFeedback(const FString& Feedback)
{
    if (APolitikaPrototypeHUD* PrototypeHUD = Cast<APolitikaPrototypeHUD>(GetHUD())) PrototypeHUD->ShowLocalFeedback(Feedback);
}

// ----------------------------------------------------------------------------
// GameMode admission (listen server)
// ----------------------------------------------------------------------------

void APolitikaPrototypeGameMode::PreLogin(const FString& Options, const FString& Address,
    const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage)
{
    Super::PreLogin(Options, Address, UniqueId, ErrorMessage);
    if (!ErrorMessage.IsEmpty() || GetNetMode() == NM_Standalone) return;
    // Earliest reliable rejection point: no PlayerController or gameplay state exists yet.
    const FString ClientProtocol = UGameplayStatics::ParseOption(Options, TEXT("PolitikaProtocol"));
    const FString HostProtocol = FString::Printf(TEXT("%lld"), FPolitikaBootstrap::Get().NetworkProtocol());
    FName Reason;
    if (ClientProtocol != HostProtocol)
    {
        Reason = TEXT("MP.PROTOCOL.MISMATCH");
        ErrorMessage = FString::Printf(TEXT("POLITIKA network protocol mismatch (host %s, client %s)."),
            *HostProtocol, ClientProtocol.IsEmpty() ? TEXT("missing") : *ClientProtocol);
    }
    else if (!ActiveMatch.IsValid() || !ActiveMatch->CanAdmitRemoteParticipant(Reason))
    {
        if (Reason.IsNone()) Reason = TEXT("MP.SESSION.NOT_READY");
        ErrorMessage = Reason == FName(TEXT("MP.SESSION.FULL"))
            ? TEXT("The POLITIKA session is full.")
            : Reason == FName(TEXT("MP.SESSION.IN_PROGRESS"))
                ? TEXT("The POLITIKA match is already in progress; late joining is not supported.")
                : TEXT("The POLITIKA host is not ready to admit participants.");
    }
    UE_LOG(LogPolitikaStartup, Display, TEXT("MP_PRELOGIN %s address=%s protocol=%s game=%s reason=%s"),
        ErrorMessage.IsEmpty() ? TEXT("ACCEPTED") : TEXT("REJECTED"), *Address,
        ClientProtocol.IsEmpty() ? TEXT("missing") : *ClientProtocol,
        *UGameplayStatics::ParseOption(Options, TEXT("PolitikaGame")), *Reason.ToString());
}

void APolitikaPrototypeGameMode::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);
    APolitikaPrototypeMatch* Match = ActiveMatch.Get();
    if (!Match || !NewPlayer) return; // The match binds early (local) controllers when it starts.
    FName Failure;
    if (Match->BindController(NewPlayer, Failure) || NewPlayer->IsLocalController()) return;
    // Fallback for a simultaneous join that passed PreLogin: remove the controller and
    // its pawn; no binding, seat or gameplay state was created for it.
    UE_LOG(LogPolitikaStartup, Warning, TEXT("MP_PARTICIPANT_REJECTED_AFTER_LOGIN controller=%s reason=%s"),
        *NewPlayer->GetName(), *Failure.ToString());
    if (GameSession) GameSession->KickPlayer(NewPlayer, FText::FromString(Failure.ToString()));
}

void APolitikaPrototypeGameMode::Logout(AController* Exiting)
{
    if (APolitikaPrototypeMatch* Match = ActiveMatch.Get()) Match->HandleControllerLogout(Exiting);
    Super::Logout(Exiting);
}

// ----------------------------------------------------------------------------
// Development-only multiplayer acceptance instrumentation
// ----------------------------------------------------------------------------

#if !UE_BUILD_SHIPPING
void APolitikaPrototypeMatch::BeginMultiplayerAcceptance()
{
    FString Mode;
    FParse::Value(FCommandLine::Get(), TEXT("RTSAcceptance="), Mode);
    bMultiplayerAcceptance = Mode.Equals(TEXT("multiplayer"), ESearchCase::IgnoreCase);
    if (!bMultiplayerAcceptance) return;
    // The Unaligned side comes from authored UnalignedPlacements; record what exists so
    // hostile-input probes can be correlated with real system-owned entities.
    int32 SystemUnits = 0;
    for (const APolitikaPrototypeUnit* Unit : Units)
        if (IsValid(Unit) && Unit->GetOwnerPlayer() == 0)
        {
            ++SystemUnits;
            UE_LOG(LogPolitikaStartup, Display, TEXT("MP_ACCEPTANCE_UNALIGNED entity=%llu team=%u x=%.1f y=%.1f"),
                Unit->GetStableId(), Unit->GetTeamId(), Unit->GetActorLocation().X, Unit->GetActorLocation().Y);
        }
    bMultiplayerFixtureSpawned = SystemUnits > 0;
    UE_LOG(LogPolitikaStartup, Display, TEXT("MP_ACCEPTANCE_FIXTURE unaligned_units=%d layout=%s fog=%d"),
        SystemUnits, UTF8_TO_TCHAR(politika::rules::LayoutName(SessionLayout).data()), bFogOfWar ? 1 : 0);
}

void APolitikaPrototypeMatch::SampleMultiplayerState(const TCHAR* Reason)
{
    const bool bPeriodic = FCString::Strcmp(Reason, TEXT("periodic")) == 0;
    if (bPeriodic && SimulationStepsExecuted < LastMultiplayerSampleStep + 20) return;
    LastMultiplayerSampleStep = SimulationStepsExecuted;
    // The listen host's own view projection lets the packaged harness click real units.
    APlayerController* HostView = GetNetMode() == NM_ListenServer && GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
    for (const APolitikaPrototypeUnit* Unit : Units)
    {
        if (!IsValid(Unit)) continue;
        const FVector Location = Unit->GetActorLocation();
        FVector2D Screen(-1.0, -1.0);
        if (HostView) HostView->ProjectWorldLocationToScreen(Location, Screen, true);
        UE_LOG(LogPolitikaStartup, Display,
            TEXT("MP_SERVER_SAMPLE reason=%s generation=%llu step=%llu entity=%llu owner=%u team=%u health=%d order=%u x=%.1f y=%.1f observed=%u host_hidden=%d screen_x=%.0f screen_y=%.0f"),
            Reason, MatchGeneration, SimulationStepsExecuted, Unit->GetStableId(), Unit->GetOwnerPlayer(), Unit->GetTeamId(),
            Unit->GetHealth(), static_cast<uint32>(Unit->GetCurrentOrder()), Location.X, Location.Y,
            static_cast<uint32>(Unit->ObservedByTeams), Unit->IsLocalFogHidden() ? 1 : 0, Screen.X, Screen.Y);
    }
    if (Objective)
    {
        FVector2D Screen(-1.0, -1.0);
        if (HostView) HostView->ProjectWorldLocationToScreen(Objective->GetActorLocation(), Screen, true);
        UE_LOG(LogPolitikaStartup, Display,
            TEXT("MP_SERVER_OBJECTIVE generation=%llu step=%llu progress_milli=%d contested=%d complete=%d capturer=%u team=%u screen_x=%.0f screen_y=%.0f"),
            MatchGeneration, SimulationStepsExecuted, FMath::RoundToInt(Objective->GetProgress() * 1000.0f),
            Objective->IsContested() ? 1 : 0, Objective->IsComplete() ? 1 : 0, Objective->GetCapturingPlayerId(),
            Objective->GetCapturingTeamId(), Screen.X, Screen.Y);
    }
}

void APolitikaPrototypeMatch::SampleObservedState(const TCHAR* Reason)
{
    if (!bObservedContentReady || !GetWorld()) return;
    const double Now = FPlatformTime::Seconds();
    const bool bPeriodic = FCString::Strcmp(Reason, TEXT("periodic")) == 0;
    if (bPeriodic && Now < LastObserverSampleSeconds + 1.0) return;
    LastObserverSampleSeconds = Now;
    APlayerController* Viewer = GetWorld()->GetFirstPlayerController();
    for (const APolitikaPrototypeUnit* Unit : Units)
    {
        if (!IsValid(Unit)) continue;
        const FVector Location = Unit->GetActorLocation();
        FVector2D Screen(-1.0, -1.0);
        if (Viewer) Viewer->ProjectWorldLocationToScreen(Location, Screen, true);
        UE_LOG(LogPolitikaStartup, Display,
            TEXT("MP_CLIENT_SAMPLE reason=%s generation=%llu entity=%llu owner=%u team=%u health=%d x=%.1f y=%.1f screen_x=%.0f screen_y=%.0f viewer=%u units=%d"),
            Reason, MatchGeneration, Unit->GetStableId(), Unit->GetOwnerPlayer(), Unit->GetTeamId(), Unit->GetHealth(),
            Location.X, Location.Y, Screen.X, Screen.Y, LocalRuntimePlayerId, Units.Num());
    }
    if (Objective)
    {
        FVector2D Screen(-1.0, -1.0);
        if (Viewer) Viewer->ProjectWorldLocationToScreen(Objective->GetActorLocation(), Screen, true);
        UE_LOG(LogPolitikaStartup, Display,
            TEXT("MP_CLIENT_OBJECTIVE generation=%llu progress_milli=%d contested=%d complete=%d capturer=%u team=%u screen_x=%.0f screen_y=%.0f"),
            MatchGeneration, FMath::RoundToInt(Objective->GetProgress() * 1000.0f), Objective->IsContested() ? 1 : 0,
            Objective->IsComplete() ? 1 : 0, Objective->GetCapturingPlayerId(), Objective->GetCapturingTeamId(), Screen.X, Screen.Y);
    }
}

void APolitikaPrototypeMatch::LogNetworkEmulation()
{
    const UNetDriver* Driver = GetWorld() ? GetWorld()->GetNetDriver() : nullptr;
    if (bNetEmulationLogged || !Driver) return;
    bNetEmulationLogged = true;
    const TCHAR* Role = HasAuthority() ? TEXT("server") : TEXT("client");
#if DO_ENABLE_NET_TEST
    // The engine applies -Pkt* switches and emulation profiles to each sending process;
    // logging the effective settings proves which conditions a run was under.
    const FPacketSimulationSettings& Settings = Driver->PacketSimulationSettings;
    UE_LOG(LogPolitikaStartup, Display,
        TEXT("MP_NET_EMULATION role=%s available=1 loss=%d order=%d dup=%d lag=%d lag_variance=%d lag_min=%d lag_max=%d incoming_loss=%d incoming_lag_min=%d incoming_lag_max=%d jitter=%d"),
        Role, Settings.PktLoss, Settings.PktOrder, Settings.PktDup, Settings.PktLag, Settings.PktLagVariance,
        Settings.PktLagMin, Settings.PktLagMax, Settings.PktIncomingLoss, Settings.PktIncomingLagMin,
        Settings.PktIncomingLagMax, Settings.PktJitter);
#else
    UE_LOG(LogPolitikaStartup, Display, TEXT("MP_NET_EMULATION role=%s available=0"), Role);
#endif
}

void APolitikaPrototypeMatch::SampleNetworkStats(const TCHAR* Reason)
{
    if (!HasAuthority() || !IsNetworkSession()) return;
    const double Now = FPlatformTime::Seconds();
    if (FCString::Strcmp(Reason, TEXT("periodic")) == 0 && Now < LastNetStatsSeconds + 1.0) return;
    LastNetStatsSeconds = Now;
    for (const FParticipantBinding& Binding : ParticipantBindings)
    {
        const APlayerController* Controller = Binding.Controller.Get();
        const UNetConnection* Connection = Binding.bRemote && Controller ? Controller->GetNetConnection() : nullptr;
        if (!Connection) continue;
        int32 Visible = 0;
        for (const APolitikaPrototypeUnit* Unit : Units)
            Visible += IsValid(Unit) && IsUnitObservedByParticipant(Unit, Binding.RuntimePlayerId) ? 1 : 0;
        LogConnectionStats(TEXT("server"), Reason, MatchGeneration, SimulationStepsExecuted, Binding.RuntimePlayerId,
            GetTeamForParticipant(Binding.RuntimePlayerId), *Connection, Visible, Now - MeasurementStartSeconds);
    }
}

void APolitikaPrototypeMatch::SampleClientNetworkStats()
{
    const UNetDriver* Driver = GetWorld() ? GetWorld()->GetNetDriver() : nullptr;
    const UNetConnection* Connection = Driver ? Driver->ServerConnection.Get() : nullptr;
    if (!bObservedContentReady || !Connection) return;
    const double Now = FPlatformTime::Seconds();
    if (Now < LastClientNetStatsSeconds + 1.0) return;
    LastClientNetStatsSeconds = Now;
    int32 Present = 0;
    for (const APolitikaPrototypeUnit* Unit : Units) Present += IsValid(Unit) ? 1 : 0;
    LogConnectionStats(TEXT("client"), TEXT("periodic"), MatchGeneration, 0, LocalRuntimePlayerId,
        GetLocalViewerTeam(), *Connection, Present, Now - MeasurementStartSeconds);
}

void APolitikaPrototypeMatch::ReportNetworkMeasurements(const TCHAR* Reason)
{
    if (bNetMeasurementsReported || !HasAuthority() || !IsNetworkSession() || StepCostMicros.IsEmpty()) return;
    bNetMeasurementsReported = true;
    SampleNetworkStats(Reason);
    using politika::metrics::NearestRank;
    TArray<float> Steps = StepCostMicros;
    Steps.Sort();
    TArray<float> Frames = ServerFrameMillis;
    Frames.Sort();
    double Total = 0.0;
    for (const float Cost : Steps) Total += Cost;
    int32 Remote = 0;
    for (const FParticipantBinding& Binding : ParticipantBindings) Remote += Binding.bRemote ? 1 : 0;
    const auto Step = [&Steps](unsigned Percent) { return NearestRank(Steps.GetData(), static_cast<std::size_t>(Steps.Num()), Percent); };
    const auto Frame = [&Frames](unsigned Percent) { return NearestRank(Frames.GetData(), static_cast<std::size_t>(Frames.Num()), Percent); };
    // The budget is one fixed simulation step: a server that needs longer cannot keep
    // real time without the catch-up cap dropping steps.
    UE_LOG(LogPolitikaStartup, Display,
        TEXT("MP_SERVER_STEP_SUMMARY reason=%s generation=%llu layout=%s hosting=%s seats=%u remote=%d units=%d steps=%d mean_us=%.1f p50_us=%.1f p95_us=%.1f p99_us=%.1f max_us=%.1f budget_us=%.0f frames=%d frame_p50_ms=%.2f frame_p95_ms=%.2f frame_max_ms=%.2f catchup_cap_frames=%d duration_s=%.3f"),
        Reason, MatchGeneration, UTF8_TO_TCHAR(politika::rules::LayoutName(SessionLayout).data()),
        UTF8_TO_TCHAR(politika::rules::HostingModeName(GetHostingMode()).data()),
        static_cast<uint32>(politika::rules::SeatCount(SessionLayout)), Remote, Units.Num(), Steps.Num(),
        Total / Steps.Num(), Step(50), Step(95), Step(99), Step(100), SimulationStepSeconds * 1.0e6, Frames.Num(),
        Frame(50), Frame(95), Frame(100), CatchUpCapFrames, FPlatformTime::Seconds() - MeasurementStartSeconds);
}

void APolitikaPrototypePlayerController::SendProbeIntent(const TCHAR* Name, FPolitikaNetCommandIntent Intent,
    bool bAdvanceSequence)
{
    if (Intent.ClientSequence == 0 && bAdvanceSequence) Intent.ClientSequence = NextClientSequence++;
    if (Intent.CorrelationId == 0) Intent.CorrelationId = NextCorrelationId++;
    UE_LOG(LogPolitikaStartup, Display,
        TEXT("MP_PROBE_SENT name=%s participant=%u correlation=%u client_sequence=%u order=%u issuer=%llu target=%llu generation=%llu protocol=%d"),
        Name, GetCommandPlayerId(), Intent.CorrelationId, Intent.ClientSequence, Intent.OrderKind, Intent.IssuerEntityId,
        Intent.TargetEntityId, Intent.MatchGeneration, Intent.ProtocolVersion);
    if (HasAuthority())
    {
        if (APolitikaPrototypeMatch* ActiveMatch = FindMatch(GetWorld())) ActiveMatch->SubmitControllerIntent(this, Intent);
    }
    else ServerSubmitCommandIntent(Intent);
}

void APolitikaPrototypePlayerController::TickHostileProbe()
{
    using namespace politika::rules;
    APolitikaPrototypeMatch* ActiveMatch = FindMatch(GetWorld());
    const uint32 Self = GetCommandPlayerId();
    if (!ActiveMatch || Self == 0) return;
    const double Now = FPlatformTime::Seconds();
    if (!bHostileProbeSent && ActiveMatch->GetMatchPhase() == MatchPhase::Active)
    {
        if (HostileProbeArmedSeconds <= 0.0) { HostileProbeArmedSeconds = Now; return; }
        if (Now < HostileProbeArmedSeconds + 3.0) return;
        const APolitikaPrototypeUnit* Own = nullptr;
        const APolitikaPrototypeUnit* Foreign = nullptr;
        const APolitikaPrototypeUnit* Ally = nullptr;
        const APolitikaPrototypeUnit* System = nullptr;
        for (const APolitikaPrototypeUnit* Unit : ActiveMatch->GetUnits())
        {
            if (!Unit || !Unit->IsAlive()) continue;
            if (Unit->GetOwnerPlayer() == Self && !Own) Own = Unit;
            else if (Unit->GetOwnerPlayer() == 0 && !System) System = Unit;
        }
        if (!Own) return;
        // Another participant's unit is either a hostile one or, in team layouts, an ally's;
        // an ally's unit is visible through shared vision but never commandable.
        for (const APolitikaPrototypeUnit* Unit : ActiveMatch->GetUnits())
        {
            if (!Unit || !Unit->IsAlive() || Unit->GetOwnerPlayer() == Self || Unit->GetOwnerPlayer() == 0) continue;
            if (Own->IsHostileTo(Unit)) { if (!Foreign) Foreign = Unit; }
            else if (!Ally) Ally = Unit;
        }
        // Under fog an enemy may not be replicated to this client at all. A hostile client
        // can still guess IDs (they are small integers), so probe with the lowest ID that
        // is neither its own nor currently visible: a hidden unit, an ally or nothing.
        uint64 HiddenGuess = 0;
        for (uint64 Candidate = 1; Candidate < 64 && HiddenGuess == 0; ++Candidate)
            if (!ActiveMatch->FindUnitByStableId(Candidate)) HiddenGuess = Candidate;
        const uint64 ForeignId = Foreign ? Foreign->GetStableId() : HiddenGuess;
        if (ForeignId == 0) return;
        bHostileProbeSent = true;
        const int32 Protocol = static_cast<int32>(FPolitikaBootstrap::Get().NetworkProtocol());
        const uint64 Generation = ActiveMatch->GetMatchGeneration();
        auto Make = [Protocol, Generation](OrderKind Kind, uint64 Issuer)
        {
            FPolitikaNetCommandIntent Intent;
            Intent.ProtocolVersion = Protocol;
            Intent.OrderKind = static_cast<uint8>(Kind);
            Intent.IssuerEntityId = Issuer;
            Intent.MatchGeneration = Generation;
            return Intent;
        };
        UE_LOG(LogPolitikaStartup, Display,
            TEXT("MP_PROBE_BEGIN participant=%u own=%llu foreign=%llu foreign_visible=%d ally=%llu hidden_guess=%llu system=%llu generation=%llu"),
            Self, Own->GetStableId(), ForeignId, Foreign ? 1 : 0, Ally ? Ally->GetStableId() : 0, HiddenGuess,
            System ? System->GetStableId() : 0, Generation);
        SendProbeIntent(TEXT("foreign_issuer"), Make(OrderKind::Guard, ForeignId));
        if (Ally)
        {
            SendProbeIntent(TEXT("allied_issuer"), Make(OrderKind::Guard, Ally->GetStableId()));
            FPolitikaNetCommandIntent AlliedTarget = Make(OrderKind::Attack, Own->GetStableId());
            AlliedTarget.TargetEntityId = Ally->GetStableId();
            SendProbeIntent(TEXT("allied_target"), AlliedTarget);
        }
        // Sweep every small ID this client cannot see (hidden enemies, allies, the Unaligned
        // side or nothing): none of them may accept this connection's command.
        for (uint64 Candidate = 1; Candidate <= 12; ++Candidate)
            if (!ActiveMatch->FindUnitByStableId(Candidate))
                SendProbeIntent(*FString::Printf(TEXT("hidden_issuer_%llu"), Candidate), Make(OrderKind::Guard, Candidate));
        if (HiddenGuess != 0)
        {
            // Attack an ID this client cannot see: must fail as not visible, never reveal it.
            FPolitikaNetCommandIntent Hidden = Make(OrderKind::Attack, Own->GetStableId());
            Hidden.TargetEntityId = HiddenGuess;
            SendProbeIntent(TEXT("hidden_target"), Hidden);
        }
        if (System) SendProbeIntent(TEXT("system_issuer"), Make(OrderKind::Guard, System->GetStableId()));
        SendProbeIntent(TEXT("unknown_issuer"), Make(OrderKind::Guard, 999999));
        FPolitikaNetCommandIntent InvalidKind = Make(OrderKind::Guard, Own->GetStableId());
        InvalidKind.OrderKind = 200;
        SendProbeIntent(TEXT("invalid_kind"), InvalidKind);
        FPolitikaNetCommandIntent NonFinite = Make(OrderKind::Move, Own->GetStableId());
        NonFinite.DestinationX = std::numeric_limits<double>::quiet_NaN();
        NonFinite.DestinationY = 100.0;
        SendProbeIntent(TEXT("nonfinite_destination"), NonFinite);
        FPolitikaNetCommandIntent OutOfBounds = Make(OrderKind::Move, Own->GetStableId());
        OutOfBounds.DestinationX = 1.0e6;
        OutOfBounds.DestinationY = 1.0e6;
        SendProbeIntent(TEXT("out_of_bounds"), OutOfBounds);
        FPolitikaNetCommandIntent Shape = Make(OrderKind::Stop, Own->GetStableId());
        Shape.TargetEntityId = ForeignId;
        SendProbeIntent(TEXT("shape_mismatch"), Shape);
        FPolitikaNetCommandIntent Stale = Make(OrderKind::Guard, Own->GetStableId());
        Stale.MatchGeneration = Generation - 1;
        SendProbeIntent(TEXT("stale_generation"), Stale);
        FPolitikaNetCommandIntent Protocolless = Make(OrderKind::Guard, Own->GetStableId());
        Protocolless.ProtocolVersion = Protocol + 99;
        SendProbeIntent(TEXT("protocol_mismatch"), Protocolless);
        FPolitikaNetCommandIntent Friendly = Make(OrderKind::Attack, Own->GetStableId());
        Friendly.TargetEntityId = Own->GetStableId();
        SendProbeIntent(TEXT("friendly_target"), Friendly);
        // One legitimate command followed by its exact replay and an older sequence.
        FPolitikaNetCommandIntent Valid = Make(OrderKind::Guard, Own->GetStableId());
        Valid.ClientSequence = NextClientSequence++;
        Valid.CorrelationId = NextCorrelationId++;
        SendProbeIntent(TEXT("valid_once"), Valid, false);
        FPolitikaNetCommandIntent Replay = Valid;
        Replay.CorrelationId = NextCorrelationId++;
        SendProbeIntent(TEXT("duplicate_replay"), Replay, false);
        FPolitikaNetCommandIntent Older = Valid;
        Older.ClientSequence = Valid.ClientSequence - 1;
        Older.CorrelationId = NextCorrelationId++;
        SendProbeIntent(TEXT("reordered_older"), Older, false);
        UE_LOG(LogPolitikaStartup, Display, TEXT("MP_PROBE_COMPLETE participant=%u"), Self);
        return;
    }
    if (bHostileProbeSent && !bPostTerminalProbeSent && ActiveMatch->GetMatchPhase() == MatchPhase::Results)
    {
        bPostTerminalProbeSent = true;
        for (const APolitikaPrototypeUnit* Unit : ActiveMatch->GetUnits())
            if (Unit && Unit->GetOwnerPlayer() == Self)
            {
                FPolitikaNetCommandIntent Late;
                Late.ProtocolVersion = static_cast<int32>(FPolitikaBootstrap::Get().NetworkProtocol());
                Late.OrderKind = static_cast<uint8>(OrderKind::Stop);
                Late.IssuerEntityId = Unit->GetStableId();
                Late.MatchGeneration = ActiveMatch->GetMatchGeneration();
                SendProbeIntent(TEXT("after_termination"), Late);
                break;
            }
    }
}
#endif
