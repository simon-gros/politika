// Match-owned formations (FORM-F3, RTS_UNIT_FORMATIONS_AND_DISCIPLINE.md §3.2 and §7).
//
// Authority boundary: the server's APolitikaPrototypeMatch owns every formation through the
// portable politika::formation::Registry. A formation changes only through an admitted
// Form/Release request, the death of a member, an admitted member order (which may make that
// member independent) or match lifecycle cleanup. A formation never moves a unit itself: its
// Move/AttackMove/Stop and its bounded regroup are planned here, on the server's own navmesh,
// and submitted as ordinary single-issuer commands that the coordinator admits one by one.
// Clients receive only an owner-only summary of their own formations.
//
// Combat stance and adherence (FORM-F4): a stance or adherence change is a formation request
// that submits no order. A unit's effective stance is its own override, else its formation's
// stance, else the default; Form, Release and member loss never change it (StanceSnapshot and
// PreserveStances re-derive each override around every registry change).

#include "PolitikaPrototypeGameplay.h"

#include "PolitikaBootstrap.h"
#include "Formation.h"
#include "GroupSelection.h"
#include "Rules.h"
#include "Stance.h"

#include "Engine/World.h"
#include "EngineUtils.h"
#include "HAL/PlatformTime.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "NavigationData.h"
#include "NavigationSystem.h"

#include <limits>
#include <vector>

namespace pf = politika::formation;
namespace ps = politika::stance;

namespace
{
// The prototype unit's standing height above the ground plane (PolitikaPrototypeGameplay.cpp).
constexpr float FormationFallbackGroundZ = 42.0f;
// The development casualty fixture strikes this long after the first formation move.
constexpr uint64 FormationCasualtyDelaySteps = 30;

APolitikaPrototypeMatch* FindFormationMatch(UWorld* World)
{
    if (!World) return nullptr;
    TActorIterator<APolitikaPrototypeMatch> It(World);
    return It ? *It : nullptr;
}

FString FormationRejectionText(pf::FormationRejection Rejection)
{
    switch (Rejection)
    {
        case pf::FormationRejection::None: return TEXT("No rejection");
        case pf::FormationRejection::MalformedCommand: return TEXT("The formation request was malformed.");
        case pf::FormationRejection::DuplicateCommand: return TEXT("That request was already processed.");
        case pf::FormationRejection::RateLimited: return TEXT("Too many pending commands; try again.");
        case pf::FormationRejection::MatchInactive: return TEXT("The match is not accepting commands.");
        case pf::FormationRejection::StaleMatch: return TEXT("That request belongs to an earlier match.");
        case pf::FormationRejection::Unauthorized: return TEXT("You cannot command those units.");
        case pf::FormationRejection::TooFewMembers: return TEXT("A formation needs at least two units.");
        case pf::FormationRejection::TooManyMembers: return TEXT("A formation holds at most 64 units.");
        case pf::FormationRejection::DuplicateMember: return TEXT("A unit was listed twice.");
        case pf::FormationRejection::MemberUnavailable: return TEXT("A selected unit cannot join a formation.");
        case pf::FormationRejection::UnknownFormation: return TEXT("That formation no longer exists.");
        case pf::FormationRejection::FormationLimit: return TEXT("You already lead ten formations.");
        case pf::FormationRejection::NotMember: return TEXT("None of those units is in a formation.");
        case pf::FormationRejection::InvalidDestination: return TEXT("That destination is not open ground.");
        case pf::FormationRejection::NotMilitary: return TEXT("Civilians cannot form formations.");
    }
    return TEXT("Formation request rejected.");
}

template <class Range>
FString JoinIds(const Range& Ids)
{
    FString Text;
    for (const auto Id : Ids) Text += FString::Printf(TEXT("%s%llu"), Text.IsEmpty() ? TEXT("") : TEXT(","), static_cast<uint64>(Id));
    return Text;
}

const TCHAR* ShapeText(pf::Shape Shape)
{
    return Shape == pf::Shape::Column ? TEXT("column") : TEXT("line");
}

const TCHAR* StanceText(ps::CombatStance Stance)
{
    switch (Stance)
    {
        case ps::CombatStance::HoldGround: return TEXT("HOLD GROUND");
        case ps::CombatStance::Guard: return TEXT("GUARD");
        case ps::CombatStance::Aggressive: return TEXT("AGGRESSIVE");
        case ps::CombatStance::HoldFire: return TEXT("HOLD FIRE");
    }
    return TEXT("?");
}

const TCHAR* AdherenceText(ps::Adherence Adherence)
{
    switch (Adherence)
    {
        case ps::Adherence::Flexible: return TEXT("FLEXIBLE");
        case ps::Adherence::Maintain: return TEXT("MAINTAIN");
        case ps::Adherence::Free: return TEXT("FREE");
    }
    return TEXT("?");
}
}

FString PolitikaStanceLabel(ps::CombatStance Stance) { return StanceText(Stance); }
FString PolitikaAdherenceLabel(ps::Adherence Adherence) { return AdherenceText(Adherence); }

// ----------------------------------------------------------------------------
// Shared advisory slot check
// ----------------------------------------------------------------------------

bool APolitikaPrototypeMatch::IsGroundSlotOpen(const UWorld* World, const FVector2D& InPlayableMin,
    const FVector2D& InPlayableMax, const FVector2D& MapPosition, float GroundZ)
{
    // Keep slots off the very edge of the playable area: the server rejects outside it.
    constexpr double EdgeMarginCm = 50.0;
    const FVector2D Size = InPlayableMax - InPlayableMin;
    if (MapPosition.X < EdgeMarginCm || MapPosition.Y < EdgeMarginCm || MapPosition.X > Size.X - EdgeMarginCm
        || MapPosition.Y > Size.Y - EdgeMarginCm)
        return false;
    if (!World) return false;
    // Ground under the slot: static world geometry only, so units standing there do not count.
    // A hit far above or below the clicked ground is a rooftop, wall or cliff, not open ground.
    constexpr float MaxStepCm = 120.0f;
    constexpr float MinGroundNormalZ = 0.75f;
    const FVector SlotWorld(InPlayableMin.X + MapPosition.X, InPlayableMin.Y + MapPosition.Y, GroundZ);
    FHitResult Ground;
    FCollisionQueryParams Params(SCENE_QUERY_STAT(PolitikaGroupMoveSlot), false);
    if (!World->LineTraceSingleByObjectType(Ground, SlotWorld + FVector(0, 0, 2000), SlotWorld - FVector(0, 0, 2000),
            FCollisionObjectQueryParams(ECC_WorldStatic), Params))
        return false;
    if (FMath::Abs(Ground.ImpactPoint.Z - GroundZ) > MaxStepCm || Ground.ImpactNormal.Z < MinGroundNormalZ) return false;
    // Where this process has navigation (standalone, listen host, server), mirror the
    // coordinator's projection threshold so a blocked slot is replaced before it is sent.
    if (const UNavigationSystemV1* NavSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(World))
        if (const ANavigationData* NavData = NavSystem->GetDefaultNavDataInstance())
        {
            FNavLocation Projected;
            if (!NavSystem->ProjectPointToNavigation(Ground.ImpactPoint, Projected, FVector(80.0f, 80.0f, 180.0f), NavData)
                || FVector::Dist(Projected.Location, Ground.ImpactPoint) > 150.0f)
                return false;
        }
    return true;
}

bool APolitikaPrototypeMatch::FormationAnchorGround(const pf::Point& Anchor, float& OutGroundZ) const
{
#if WITH_DEV_AUTOMATION_TESTS
    if (FormationSlotProbeForTest)
    {
        OutGroundZ = FormationFallbackGroundZ;
        return FormationSlotProbeForTest({Anchor.x, Anchor.y});
    }
#endif
    const UWorld* World = GetWorld();
    if (!World || !pf::IsFinite(Anchor)) return false;
    const FVector Point(PlayableMin.X + Anchor.x, PlayableMin.Y + Anchor.y, FormationFallbackGroundZ);
    FHitResult Ground;
    FCollisionQueryParams Params(SCENE_QUERY_STAT(PolitikaFormationAnchor), false);
    if (!World->LineTraceSingleByObjectType(Ground, Point + FVector(0, 0, 5000), Point - FVector(0, 0, 5000),
            FCollisionObjectQueryParams(ECC_WorldStatic), Params)
        || Ground.ImpactNormal.Z < 0.75f)
        return false;
    OutGroundZ = static_cast<float>(Ground.ImpactPoint.Z);
    return true;
}

// ----------------------------------------------------------------------------
// Formation intake (server)
// ----------------------------------------------------------------------------

void APolitikaPrototypeMatch::SendFormationOutcome(APlayerController* Source, const FPolitikaNetFormationIntent& Intent,
    pf::FormationRejection Rejection, uint32 FormationId, int32 Members, int32 Orders)
{
    APolitikaPrototypePlayerController* Controller = Cast<APolitikaPrototypePlayerController>(Source);
    if (!Controller) return;
    FPolitikaNetFormationOutcome Outcome;
    Outcome.CorrelationId = Intent.CorrelationId;
    Outcome.ClientSequence = Intent.ClientSequence;
    Outcome.Op = Intent.Op;
    Outcome.FormationId = FormationId;
    Outcome.Rejection = static_cast<uint8>(Rejection);
    Outcome.bAccepted = Rejection == pf::FormationRejection::None;
    Outcome.Members = Members;
    Outcome.Orders = Orders;
    Outcome.MatchGeneration = MatchGeneration;
#if WITH_DEV_AUTOMATION_TESTS
    TestFormationOutcomes.Emplace(Source, Outcome);
#endif
    if (Controller->IsLocalSeatController()) Controller->ClientReceiveFormationOutcome_Implementation(Outcome);
    else Controller->ClientReceiveFormationOutcome(Outcome);
}

void APolitikaPrototypeMatch::RejectFormationIntent(APlayerController* Source, const FPolitikaNetFormationIntent& Intent,
    uint32 ParticipantId, pf::FormationRejection Rejection)
{
#if !UE_BUILD_SHIPPING
    UE_LOG(LogPolitikaStartup, Display,
        TEXT("RTS_FORMATION_INTENT participant=%u op=%u correlation=%u client_sequence=%u formation=%u members=%d accepted=0 rejection=%u generation=%llu step=%llu"),
        ParticipantId, Intent.Op, Intent.CorrelationId, Intent.ClientSequence, Intent.FormationId, Intent.Members.Num(),
        static_cast<uint32>(Rejection), MatchGeneration, SimulationStepsExecuted);
#endif
    SendFormationOutcome(Source, Intent, Rejection, Intent.FormationId, 0, 0);
}

bool APolitikaPrototypeMatch::SubmitFormationIntent(APlayerController* Source, const FPolitikaNetFormationIntent& Intent)
{
    using pf::FormationRejection;
    if (!HasAuthority()) return false;
    FParticipantBinding* Binding = ResolveCommandBinding(Source);
    if (!Binding)
    {
        UE_LOG(LogPolitikaStartup, Display, TEXT("MP_FORMATION_UNBOUND controller=%s correlation=%u client_sequence=%u"),
            Source ? *Source->GetName() : TEXT("none"), Intent.CorrelationId, Intent.ClientSequence);
        if (const APolitikaPrototypePlayerController* Prototype = Cast<APolitikaPrototypePlayerController>(Source);
            Prototype && !Prototype->IsLocalSeatController())
            SendFormationOutcome(Source, Intent, FormationRejection::Unauthorized, 0, 0, 0);
        return false;
    }
    const uint32 ParticipantId = Binding->RuntimePlayerId;
    // Protocol, single-use sequence (shared with command intents) and payload shape first.
    if (Intent.ProtocolVersion != static_cast<int32>(FPolitikaBootstrap::Get().NetworkProtocol()))
    {
        RejectFormationIntent(Source, Intent, ParticipantId, FormationRejection::MalformedCommand);
        return true;
    }
    if (const politika::rules::OrderResult Fresh = Binding->Sequence.Consume(Intent.ClientSequence); !Fresh)
    {
        RejectFormationIntent(Source, Intent, ParticipantId,
            Fresh.rejection == politika::rules::OrderRejection::DuplicateCommand
                ? FormationRejection::DuplicateCommand : FormationRejection::MalformedCommand);
        return true;
    }
    std::vector<pf::EntityId> Ids;
    Ids.reserve(static_cast<std::size_t>(FMath::Min(Intent.Members.Num(), static_cast<int32>(pf::kMaxReleaseIds) + 1)));
    for (int32 Index = 0; Index < Intent.Members.Num() && Index <= static_cast<int32>(pf::kMaxReleaseIds); ++Index)
        Ids.push_back(static_cast<pf::EntityId>(Intent.Members[Index]));
    if (Intent.Members.Num() > static_cast<int32>(pf::kMaxReleaseIds)) Ids.push_back(0); // fails the shape check
    if (const FormationRejection Shape = pf::CheckIntentShape({Intent.Op, Intent.FormationId, Intent.Shape, Ids,
            Intent.DestinationX, Intent.DestinationY, Intent.Value}); Shape != FormationRejection::None)
    {
        RejectFormationIntent(Source, Intent, ParticipantId, Shape);
        return true;
    }
    if (!bContentReady || !Lifecycle.IsActive())
    {
        RejectFormationIntent(Source, Intent, ParticipantId, FormationRejection::MatchInactive);
        return true;
    }
    if (Intent.MatchGeneration != MatchGeneration)
    {
        RejectFormationIntent(Source, Intent, ParticipantId, FormationRejection::StaleMatch);
        return true;
    }
    PruneFormationMembers();
    const uint64 Step = SimulationStepsExecuted;
    const pf::FormationOp Op = static_cast<pf::FormationOp>(Intent.Op);
    const auto LogAccepted = [&](uint32 FormationId, int32 Members, int32 Orders)
    {
#if !UE_BUILD_SHIPPING
        UE_LOG(LogPolitikaStartup, Display,
            TEXT("RTS_FORMATION_INTENT participant=%u op=%u correlation=%u client_sequence=%u formation=%u members=%d accepted=1 rejection=0 orders=%d generation=%llu step=%llu remote=%d"),
            ParticipantId, Intent.Op, Intent.CorrelationId, Intent.ClientSequence, FormationId, Members, Orders,
            MatchGeneration, Step, Binding->bRemote ? 1 : 0);
#endif
    };

    if (Op == pf::FormationOp::SetStance || Op == pf::FormationOp::SetAdherence)
        return SubmitStanceIntent(Source, Intent, ParticipantId);

    if (Op == pf::FormationOp::Form || Op == pf::FormationOp::Release)
    {
        // Form and Release never change any unit's effective stance.
        TArray<uint64> NamedIds;
        for (const pf::EntityId Id : Ids) NamedIds.Add(Id);
        const TMap<uint64, ps::CombatStance> StancesBefore = StanceSnapshot(NamedIds);
        pf::Change Change;
        if (Op == pf::FormationOp::Form)
        {
            // Authoritative snapshots: an unknown, hidden, dead or foreign ID is unavailable.
            std::vector<politika::rules::Entity> Members;
            Members.reserve(Ids.size());
            for (const pf::EntityId Id : Ids)
            {
                politika::rules::Entity Entity;
                Entity.id = Id;
                const APolitikaPrototypeUnit* Unit = FindUnitByStableId(Id);
                if (Unit && IsCommandableUnit(Unit, ParticipantId))
                {
                    Entity.owner_player = Unit->GetOwnerPlayer();
                    Entity.alive = Unit->IsAlive();
                    Entity.operational = Unit->IsAlive();
                    Entity.movable = Unit->CanMove();
                    Entity.military = Unit->IsMilitary();
                }
                else
                {
                    Entity.owner_player = 0;
                    Entity.alive = false;
                }
                Members.push_back(Entity);
            }
            // A formation of units that all fight the same way takes that stance, so none of
            // them needs an override; otherwise it starts at the default and they keep theirs.
            std::optional<ps::CombatStance> Common;
            bool bCommon = true;
            for (const pf::EntityId Id : Ids)
            {
                const ps::CombatStance* Stance = StancesBefore.Find(Id);
                if (!Stance) { bCommon = false; break; }
                if (Common && *Common != *Stance) { bCommon = false; break; }
                Common = *Stance;
            }
            Change = Formations.Form(ParticipantId, Members, static_cast<pf::Shape>(Intent.Shape), Step,
                bCommon && Common ? *Common : ps::kDefaultStance);
        }
        else
            Change = Formations.Release(ParticipantId, Ids, Step);
        if (!Change.Accepted())
        {
            RejectFormationIntent(Source, Intent, ParticipantId, Change.rejection);
            return true;
        }
        const pf::Formation* Formed = Change.formed ? Formations.Find(Change.formed) : nullptr;
        const int32 Members = Formed ? static_cast<int32>(Formed->members.size()) : static_cast<int32>(Change.left.size());
        LogAccepted(Change.formed.value, Members, 0);
        SendFormationOutcome(Source, Intent, FormationRejection::None, Change.formed.value, Members, 0);
        HandleFormationChange(Change, Op == pf::FormationOp::Form ? TEXT("form") : TEXT("release"));
        PreserveStances(StancesBefore);
        SyncFormationReplication();
        return true;
    }

    const pf::FormationId Addressed{Intent.FormationId};
    const pf::Formation* Formation = Formations.FindOwned(Addressed, ParticipantId);
    if (!Formation)
    {
        RejectFormationIntent(Source, Intent, ParticipantId, FormationRejection::UnknownFormation);
        return true;
    }
    // A formation order fans out into one command per member: it must fit the same bounded
    // per-participant queue as a group order.
    int32 PendingForParticipant = 0;
    for (const FPendingOrder& Pending : PendingOrders)
        PendingForParticipant += Pending.Request.participant_id == ParticipantId ? 1 : 0;
    int32 LivingUnits = 0;
    for (const APolitikaPrototypeUnit* Unit : Units)
        LivingUnits += IsValid(Unit) && Unit->IsAlive() && Unit->GetOwnerPlayer() == ParticipantId ? 1 : 0;
    if (PendingForParticipant + static_cast<int32>(Formation->members.size())
        > politika::selection::PendingCommandLimit(LivingUnits))
    {
        RejectFormationIntent(Source, Intent, ParticipantId, FormationRejection::RateLimited);
        return true;
    }

    if (Op == pf::FormationOp::Stop)
    {
        const FormationRejection Stopped = Formations.Stop(ParticipantId, Addressed);
        if (Stopped != FormationRejection::None)
        {
            RejectFormationIntent(Source, Intent, ParticipantId, Stopped);
            return true;
        }
        Formation = Formations.Find(Addressed);
        const int32 Orders = static_cast<int32>(Formation->members.size());
        LogAccepted(Addressed.value, Orders, Orders);
        SendFormationOutcome(Source, Intent, FormationRejection::None, Addressed.value, Orders, Orders);
        for (const pf::MemberSlot& Member : Formation->members)
        {
            const uint64 Expected = NextCommandSequence;
            OutcomeRoutes.Add(Expected, {Source, Intent.ClientSequence, Intent.CorrelationId, MatchGeneration,
                static_cast<uint8>(politika::rules::OrderKind::Stop)});
            SubmitCommand(ParticipantId, FindUnitByStableId(Member.id), nullptr, politika::rules::StopOrder{}, Member.id,
                Intent.CorrelationId, MatchGeneration, Addressed.value);
        }
        bFormationsDirty = true;
        SyncFormationReplication();
        return true;
    }

    // Move / AttackMove: plan on the server, where the navmesh exists in every hosting mode.
    const pf::Point Anchor{Intent.DestinationX, Intent.DestinationY};
    float GroundZ = FormationFallbackGroundZ;
    if (!politika::rules::IsValidPosition({PlayableMax.X - PlayableMin.X, PlayableMax.Y - PlayableMin.Y},
            {Anchor.x, Anchor.y})
        || !FormationAnchorGround(Anchor, GroundZ))
    {
        RejectFormationIntent(Source, Intent, ParticipantId, FormationRejection::InvalidDestination);
        return true;
    }
    const bool bAttackMove = Op == pf::FormationOp::AttackMove;
    const auto PositionOf = [this](pf::EntityId Id)
    {
        const APolitikaPrototypeUnit* Unit = FindUnitByStableId(Id);
        if (!Unit) return pf::Point{std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::quiet_NaN()};
        return pf::Point{Unit->GetActorLocation().X - PlayableMin.X, Unit->GetActorLocation().Y - PlayableMin.Y};
    };
    const auto IsLegal = [this, GroundZ](pf::Point Candidate)
    {
#if WITH_DEV_AUTOMATION_TESTS
        if (FormationSlotProbeForTest) return FormationSlotProbeForTest({Candidate.x, Candidate.y});
#endif
        return IsGroundSlotOpen(GetWorld(), PlayableMin, PlayableMax, {Candidate.x, Candidate.y}, GroundZ);
    };
    const pf::MovePlan Plan = Formations.PlanMove(ParticipantId, Addressed, Anchor, bAttackMove, PositionOf, IsLegal);
    if (Plan.rejection != FormationRejection::None)
    {
        RejectFormationIntent(Source, Intent, ParticipantId, Plan.rejection);
        return true;
    }
    Formation = Formations.Find(Addressed);
    const int32 Orders = static_cast<int32>(Plan.orders.size());
    LogAccepted(Addressed.value, static_cast<int32>(Formation->members.size()), Orders);
    // The outcome precedes the member commands, so the client can tally their outcomes.
    SendFormationOutcome(Source, Intent, FormationRejection::None, Addressed.value,
        static_cast<int32>(Formation->members.size()), Orders);
#if !UE_BUILD_SHIPPING
    const double Separation = pf::MinimumSeparation(Plan.plan);
    UE_LOG(LogPolitikaStartup, Display,
        TEXT("RTS_FORMATION_MOVE formation=%u participant=%u op=%s shape=%s members=%d anchor=%.1f,%.1f ground_z=%.1f facing=%.4f,%.4f checked=%d rejected=%d unplaced=%d min_separation=%.1f orders=%d step=%llu"),
        Addressed.value, ParticipantId, UTF8_TO_TCHAR(pf::OpName(Op).data()), ShapeText(Formation->shape),
        static_cast<int32>(Formation->members.size()), Anchor.x, Anchor.y, GroundZ, Plan.plan.facing.x, Plan.plan.facing.y,
        static_cast<int32>(Plan.plan.candidatesChecked), static_cast<int32>(Plan.plan.candidatesRejected),
        static_cast<int32>(Plan.plan.unplaced), FMath::IsFinite(Separation) ? Separation : -1.0, Orders, Step);
    if (FormationCasualtyArmed < 0)
        FormationCasualtyArmed = FParse::Param(FCommandLine::Get(), TEXT("PolitikaMPFormationCasualty")) ? 1 : 0;
    if (FormationCasualtyArmed == 1 && FormationCasualtyFormation == 0 && Formation->members.size() >= 3)
    {
        FormationCasualtyFormation = Addressed.value;
        FormationCasualtyStep = Step + FormationCasualtyDelaySteps;
        UE_LOG(LogPolitikaStartup, Display, TEXT("MP_FORMATION_CASUALTY_ARMED formation=%u due_step=%llu"),
            Addressed.value, FormationCasualtyStep);
    }
#endif
    SubmitFormationOrders(*Formation, Plan, Source, &Intent, TEXT("move"));
    bFormationsDirty = true;
    SyncFormationReplication();
    return true;
}

int32 APolitikaPrototypeMatch::SubmitFormationOrders(const pf::Formation& Formation, const pf::MovePlan& Plan,
    APlayerController* Source, const FPolitikaNetFormationIntent* Intent, const TCHAR* Cause)
{
    int32 Submitted = 0;
    const politika::rules::OrderKind Kind = Formation.attackMove ? politika::rules::OrderKind::AttackMove
        : politika::rules::OrderKind::Move;
    for (const pf::MemberOrder& Order : Plan.orders)
    {
        APolitikaPrototypeUnit* Unit = FindUnitByStableId(Order.id);
        if (!Unit) continue;
        const politika::rules::Position Destination{Order.destination.x, Order.destination.y};
        const uint64 Expected = NextCommandSequence;
        if (Source && Intent)
            OutcomeRoutes.Add(Expected, {Source, Intent->ClientSequence, Intent->CorrelationId, MatchGeneration,
                static_cast<uint8>(Kind)});
        const politika::rules::Order Command = Formation.attackMove
            ? politika::rules::Order{politika::rules::AttackMoveOrder{Destination}}
            : politika::rules::Order{politika::rules::MoveOrder{Destination}};
        const uint64 Sequence = SubmitCommand(Formation.owner, Unit, nullptr, Command, Order.id,
            Intent ? Intent->CorrelationId : 0, MatchGeneration, Formation.id.value);
#if !UE_BUILD_SHIPPING
        UE_LOG(LogPolitikaStartup, Display,
            TEXT("RTS_FORMATION_SLOT formation=%u cause=%s unit=%llu slot=%d x=%.1f y=%.1f world_x=%.1f world_y=%.1f sequence=%llu"),
            Formation.id.value, Cause, Order.id, Order.slot != pf::kNoSlot ? static_cast<int32>(Order.slot) : -1,
            Order.destination.x, Order.destination.y, PlayableMin.X + Order.destination.x,
            PlayableMin.Y + Order.destination.y, Sequence);
#else
        (void)Sequence;
        (void)Cause;
#endif
        ++Submitted;
    }
    return Submitted;
}

// ----------------------------------------------------------------------------
// Membership transitions
// ----------------------------------------------------------------------------

void APolitikaPrototypeMatch::HandleFormationChange(const pf::Change& Change, const TCHAR* Cause)
{
    if (!Change.formed && Change.left.empty() && Change.dissolved.empty() && Change.regroups.empty()) return;
    bFormationsDirty = true;
#if !UE_BUILD_SHIPPING
    for (const pf::Change::Left& Left : Change.left)
    {
        const pf::Formation* Remaining = Formations.Find(Left.formation);
        UE_LOG(LogPolitikaStartup, Display,
            TEXT("RTS_FORMATION_MEMBER_LEFT formation=%u unit=%llu reason=%s remaining=%d cause=%s step=%llu"),
            Left.formation.value, Left.member, UTF8_TO_TCHAR(pf::LeaveReasonName(Left.reason).data()),
            Remaining ? static_cast<int32>(Remaining->members.size()) : 0, Cause, SimulationStepsExecuted);
    }
    for (const pf::Change::Dissolved& Dissolved : Change.dissolved)
        UE_LOG(LogPolitikaStartup, Display, TEXT("RTS_FORMATION_DISSOLVED formation=%u participant=%u reason=%s cause=%s step=%llu"),
            Dissolved.formation.value, Dissolved.owner, UTF8_TO_TCHAR(pf::DissolveReasonName(Dissolved.reason).data()), Cause,
            SimulationStepsExecuted);
    if (const pf::Formation* Formed = Change.formed ? Formations.Find(Change.formed) : nullptr)
    {
        std::vector<pf::EntityId> Ids;
        for (const pf::MemberSlot& Member : Formed->members) Ids.push_back(Member.id);
        UE_LOG(LogPolitikaStartup, Display,
            TEXT("RTS_FORMATION_FORMED formation=%u participant=%u shape=%s members=%d ids=%s generation=%llu step=%llu"),
            Formed->id.value, Formed->owner, ShapeText(Formed->shape), static_cast<int32>(Formed->members.size()), *JoinIds(Ids),
            MatchGeneration, SimulationStepsExecuted);
    }
    for (const pf::Change::Scheduled& Scheduled : Change.regroups)
        UE_LOG(LogPolitikaStartup, Display, TEXT("RTS_FORMATION_REGROUP_SCHEDULED formation=%u due_step=%llu step=%llu cause=%s"),
            Scheduled.formation.value, Scheduled.dueStep, SimulationStepsExecuted, Cause);
#else
    (void)Cause;
#endif
}

void APolitikaPrototypeMatch::PruneFormationMembers()
{
    if (Formations.Empty()) return;
    std::vector<std::pair<pf::EntityId, pf::LeaveReason>> Lost;
    for (const pf::Formation& Formation : Formations.All())
        for (const pf::MemberSlot& Member : Formation.members)
        {
            const APolitikaPrototypeUnit* Unit = FindUnitByStableId(Member.id);
            if (!Unit || !Unit->IsAlive()) Lost.emplace_back(Member.id, pf::LeaveReason::Destroyed);
            else if (Unit->GetOwnerPlayer() != Formation.owner) Lost.emplace_back(Member.id, pf::LeaveReason::Unavailable);
        }
    for (const auto& [Id, Reason] : Lost) LoseFormationMember(Id, Reason, TEXT("revalidation"));
}

void APolitikaPrototypeMatch::LoseFormationMember(uint64 UnitId, pf::LeaveReason Reason, const TCHAR* Cause)
{
    const TMap<uint64, ps::CombatStance> StancesBefore = StanceSnapshot();
    // DISC-D3: the survivors' average Discipline (after this step's casualty shock) sets how soon
    // the formation closes its ranks.
    int64 SurvivorSum = 0;
    int32 Survivors = 0;
    if (const pf::Formation* Formation = Formations.Find(Formations.FormationOf(UnitId)))
        for (const pf::MemberSlot& Member : Formation->members)
            if (const APolitikaPrototypeUnit* Unit = FindUnitByStableId(Member.id);
                Member.id != UnitId && Unit && Unit->IsAlive())
            {
                SurvivorSum += Unit->GetResolveMilli();
                ++Survivors;
            }
    const int32 SurvivorAverage = Survivors > 0 ? static_cast<int32>(SurvivorSum / Survivors) : politika::discipline::kBaseline;
    const uint64 RegroupDelay = politika::discipline::RegroupDelaySteps(SurvivorAverage, pf::kRegroupDelaySteps);
    const pf::Change Change = Formations.MemberLost(UnitId, SimulationStepsExecuted, Reason, RegroupDelay);
#if !UE_BUILD_SHIPPING
    for (const pf::Change::Scheduled& Regroup : Change.regroups)
        if (RegroupDelay != pf::kRegroupDelaySteps)
            UE_LOG(LogPolitikaStartup, Display,
                TEXT("RTS_DISCIPLINE_EFFECT effect=regroup_delay formation=%u lost=%llu survivors=%d value=%d permille=%d base_steps=%llu steps=%llu due=%llu step=%llu"),
                Regroup.formation.value, UnitId, Survivors, SurvivorAverage,
                politika::discipline::EffectPermille(politika::modifiers::Attribute::RegroupDelay, SurvivorAverage),
                pf::kRegroupDelaySteps, RegroupDelay, Regroup.dueStep, SimulationStepsExecuted);
#endif
    HandleFormationChange(Change, Cause);
    PreserveStances(StancesBefore);
}

// ----------------------------------------------------------------------------
// Combat stance and adherence (FORM-F4)
// ----------------------------------------------------------------------------

void APolitikaPrototypeMatch::RefreshUnitStance(APolitikaPrototypeUnit* Unit)
{
    if (!Unit || !HasAuthority()) return;
    const pf::FormationId Id = Formations.FormationOf(Unit->GetStableId());
    const pf::Formation* Formation = Id ? Formations.Find(Id) : nullptr;
    Unit->ApplyStance(ps::Effective(Unit->GetStanceOverride(), Formation ? std::optional(Formation->stance) : std::nullopt),
        Formation ? std::optional(Formation->adherence) : std::nullopt);
}

void APolitikaPrototypeMatch::RefreshFormationStances(pf::FormationId Id)
{
    if (const pf::Formation* Formation = Formations.Find(Id))
        for (const pf::MemberSlot& Member : Formation->members) RefreshUnitStance(FindUnitByStableId(Member.id));
}

TMap<uint64, ps::CombatStance> APolitikaPrototypeMatch::StanceSnapshot(TConstArrayView<uint64> ExtraIds) const
{
    TMap<uint64, ps::CombatStance> Snapshot;
    const auto Record = [this, &Snapshot](uint64 Id)
    {
        if (const APolitikaPrototypeUnit* Unit = FindUnitByStableId(Id)) Snapshot.Add(Id, Unit->GetEffectiveStance());
    };
    for (const pf::Formation& Formation : Formations.All())
        for (const pf::MemberSlot& Member : Formation.members) Record(Member.id);
    for (const uint64 Id : ExtraIds) Record(Id);
    return Snapshot;
}

void APolitikaPrototypeMatch::PreserveStances(const TMap<uint64, ps::CombatStance>& Before)
{
    for (const TPair<uint64, ps::CombatStance>& Entry : Before)
    {
        APolitikaPrototypeUnit* Unit = FindUnitByStableId(Entry.Key);
        if (!Unit || !Unit->IsAlive()) continue;
        const pf::FormationId Id = Formations.FormationOf(Entry.Key);
        const pf::Formation* Formation = Id ? Formations.Find(Id) : nullptr;
        Unit->SetStanceOverride(ps::OverrideFor(Entry.Value, Formation ? Formation->stance : ps::kDefaultStance));
        RefreshUnitStance(Unit);
    }
}

bool APolitikaPrototypeMatch::SubmitStanceIntent(APlayerController* Source, const FPolitikaNetFormationIntent& Intent,
    uint32 ParticipantId)
{
    using pf::FormationRejection;
    const pf::FormationOp Op = static_cast<pf::FormationOp>(Intent.Op);
    const uint64 Step = SimulationStepsExecuted;
    int32 Affected = 0;
    if (Intent.FormationId != 0)
    {
        const pf::FormationId Addressed{Intent.FormationId};
        const FormationRejection Rejection = Op == pf::FormationOp::SetStance
            ? Formations.SetStance(ParticipantId, Addressed, static_cast<ps::CombatStance>(Intent.Value))
            : Formations.SetAdherence(ParticipantId, Addressed, static_cast<ps::Adherence>(Intent.Value));
        if (Rejection != FormationRejection::None)
        {
            RejectFormationIntent(Source, Intent, ParticipantId, Rejection);
            return true;
        }
        const pf::Formation* Formation = Formations.Find(Addressed);
#if !UE_BUILD_SHIPPING
        if (Op == pf::FormationOp::SetStance) TriggerStanceProbe(ParticipantId, *Formation);
        else TriggerDisciplineCasualties(*Formation);
#endif
        // Setting the whole formation's stance applies it to every member: overrides go.
        if (Op == pf::FormationOp::SetStance)
            for (const pf::MemberSlot& Member : Formation->members)
                if (APolitikaPrototypeUnit* Unit = FindUnitByStableId(Member.id)) Unit->SetStanceOverride(std::nullopt);
        RefreshFormationStances(Addressed);
        Affected = static_cast<int32>(Formation->members.size());
#if !UE_BUILD_SHIPPING
        UE_LOG(LogPolitikaStartup, Display,
            TEXT("RTS_STANCE_SET participant=%u op=%s formation=%u stance=%s adherence=%s members=%d correlation=%u step=%llu"),
            ParticipantId, UTF8_TO_TCHAR(pf::OpName(Op).data()), Addressed.value,
            UTF8_TO_TCHAR(ps::StanceName(Formation->stance).data()), UTF8_TO_TCHAR(ps::AdherenceName(Formation->adherence).data()),
            Affected, Intent.CorrelationId, Step);
#endif
    }
    else
    {
        // Per-unit stance: every listed unit must be the participant's own living unit.
        TArray<APolitikaPrototypeUnit*> Targets;
        for (const uint64 Id : Intent.Members)
        {
            APolitikaPrototypeUnit* Unit = FindUnitByStableId(Id);
            if (!Unit || !IsCommandableUnit(Unit, ParticipantId) || !Unit->IsAlive())
            {
                RejectFormationIntent(Source, Intent, ParticipantId, FormationRejection::MemberUnavailable);
                return true;
            }
            Targets.Add(Unit);
        }
        const ps::CombatStance Value = static_cast<ps::CombatStance>(Intent.Value);
        for (APolitikaPrototypeUnit* Unit : Targets)
        {
            const pf::FormationId Id = Formations.FormationOf(Unit->GetStableId());
            const pf::Formation* Formation = Id ? Formations.Find(Id) : nullptr;
            Unit->SetStanceOverride(ps::OverrideFor(Value, Formation ? Formation->stance : ps::kDefaultStance));
            RefreshUnitStance(Unit);
        }
        Affected = Targets.Num();
#if !UE_BUILD_SHIPPING
        TArray<uint64> SortedIds;
        for (const APolitikaPrototypeUnit* Unit : Targets) SortedIds.Add(Unit->GetStableId());
        SortedIds.Sort();
        UE_LOG(LogPolitikaStartup, Display,
            TEXT("RTS_STANCE_SET participant=%u op=set_stance formation=0 stance=%s units=%d ids=%s correlation=%u step=%llu"),
            ParticipantId, UTF8_TO_TCHAR(ps::StanceName(Value).data()), Affected, *JoinIds(SortedIds), Intent.CorrelationId, Step);
#endif
    }
#if !UE_BUILD_SHIPPING
    UE_LOG(LogPolitikaStartup, Display,
        TEXT("RTS_FORMATION_INTENT participant=%u op=%u correlation=%u client_sequence=%u formation=%u members=%d accepted=1 rejection=0 orders=0 generation=%llu step=%llu remote=%d"),
        ParticipantId, Intent.Op, Intent.CorrelationId, Intent.ClientSequence, Intent.FormationId, Affected, MatchGeneration, Step,
        Source && Cast<APolitikaPrototypePlayerController>(Source) && !Cast<APolitikaPrototypePlayerController>(Source)->IsLocalSeatController() ? 1 : 0);
#endif
    SendFormationOutcome(Source, Intent, FormationRejection::None, Intent.FormationId, Affected, 0);
    bFormationsDirty = true;
    SyncFormationReplication();
    return true;
}

void APolitikaPrototypeMatch::NoteFormationOrderAdmitted(const APolitikaPrototypeUnit* Unit, const FPendingOrder& Pending)
{
    if (!Unit || Formations.Empty()) return;
    switch (Pending.Kind)
    {
        case EPolitikaPrototypeOrder::Move:
        case EPolitikaPrototypeOrder::AttackMove:
        case EPolitikaPrototypeOrder::Attack:
        case EPolitikaPrototypeOrder::Guard:
        case EPolitikaPrototypeOrder::Stop:
            break;
        default:
            return; // Abilities do not replace the member's movement order.
    }
    const pf::FormationId Formation = Formations.FormationOf(Unit->GetStableId());
    if (!Formation) return;
    const bool bBecameIndependent = Formations.NoteOrderAdmitted(Unit->GetStableId(), pf::FormationId{Pending.FormationId});
    bFormationsDirty = true;
#if !UE_BUILD_SHIPPING
    if (bBecameIndependent)
        UE_LOG(LogPolitikaStartup, Display,
            TEXT("RTS_FORMATION_MEMBER_INDEPENDENT formation=%u unit=%llu sequence=%llu order=%u following=%d step=%llu"),
            Formation.value, Unit->GetStableId(), Pending.Request.sequence, static_cast<uint32>(Pending.Kind),
            static_cast<int32>(Formations.Find(Formation)->FollowingCount()), SimulationStepsExecuted + 1);
#else
    (void)bBecameIndependent;
#endif
}

void APolitikaPrototypeMatch::ProcessFormationRegroups()
{
#if !UE_BUILD_SHIPPING
    TickFormationCasualtyFixture();
    TickDisciplineCasualtyFixture();
#endif
    const std::vector<pf::FormationId> Due = Formations.DueRegroups(SimulationStepsExecuted);
    if (!Due.empty())
    {
        PruneFormationMembers();
        const auto PositionOf = [this](pf::EntityId Id)
        {
            const APolitikaPrototypeUnit* Unit = FindUnitByStableId(Id);
            if (!Unit) return pf::Point{std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::quiet_NaN()};
            return pf::Point{Unit->GetActorLocation().X - PlayableMin.X, Unit->GetActorLocation().Y - PlayableMin.Y};
        };
        for (const pf::FormationId Id : Due)
        {
            const pf::Formation* Before = Formations.Find(Id);
            if (!Before || Before->regroupDueStep == 0) continue;
            float GroundZ = FormationFallbackGroundZ;
            FormationAnchorGround(Before->anchor, GroundZ);
            const auto IsLegal = [this, GroundZ](pf::Point Candidate)
            {
#if WITH_DEV_AUTOMATION_TESTS
                if (FormationSlotProbeForTest) return FormationSlotProbeForTest({Candidate.x, Candidate.y});
#endif
                return IsGroundSlotOpen(GetWorld(), PlayableMin, PlayableMax, {Candidate.x, Candidate.y}, GroundZ);
            };
            const pf::MovePlan Plan = Formations.PlanRegroup(Id, SimulationStepsExecuted, PositionOf, IsLegal);
            const pf::Formation* Formation = Formations.Find(Id);
            if (!Formation) continue;
#if !UE_BUILD_SHIPPING
            const double Separation = pf::MinimumSeparation(Plan.plan);
            UE_LOG(LogPolitikaStartup, Display,
                TEXT("RTS_FORMATION_REGROUP formation=%u participant=%u step=%llu members=%d following=%d planned=%d orders=%d unplaced=%d min_separation=%.1f regroups=%u facing=%.4f,%.4f"),
                Id.value, Formation->owner, SimulationStepsExecuted, static_cast<int32>(Formation->members.size()),
                static_cast<int32>(Formation->FollowingCount()), static_cast<int32>(Plan.plan.assignments.size()),
                static_cast<int32>(Plan.orders.size()), static_cast<int32>(Plan.plan.unplaced),
                FMath::IsFinite(Separation) ? Separation : -1.0, Formation->regroups, Formation->facing.x, Formation->facing.y);
#endif
            SubmitFormationOrders(*Formation, Plan, nullptr, nullptr, TEXT("regroup"));
            bFormationsDirty = true;
        }
    }
    if (bFormationsDirty) SyncFormationReplication();
}

void APolitikaPrototypeMatch::ClearFormations(const TCHAR* Reason)
{
#if !UE_BUILD_SHIPPING
    FormationCasualtyFormation = 0;
    FormationCasualtyStep = 0;
    if (FormationCasualtyArmed == 2) FormationCasualtyArmed = 1;
    DisciplineCasualtiesFormation = 0;
    DisciplineCasualtyVictims.Reset();
    if (DisciplineCasualtiesArmed == 2) DisciplineCasualtiesArmed = 1;
#endif
    const TMap<uint64, ps::CombatStance> StancesBefore = StanceSnapshot();
    const pf::Change Cleared = Formations.Clear();
    PreserveStances(StancesBefore);
    if (Cleared.dissolved.empty()) return;
#if !UE_BUILD_SHIPPING
    std::vector<uint32> Ids;
    for (const pf::Change::Dissolved& Dissolved : Cleared.dissolved) Ids.push_back(Dissolved.formation.value);
    UE_LOG(LogPolitikaStartup, Display, TEXT("RTS_FORMATIONS_CLEARED reason=%s count=%d ids=%s generation=%llu step=%llu"),
        Reason, static_cast<int32>(Cleared.dissolved.size()), *JoinIds(Ids), MatchGeneration, SimulationStepsExecuted);
#else
    (void)Reason;
#endif
    bFormationsDirty = true;
    SyncFormationReplication();
}

void APolitikaPrototypeMatch::SyncFormationReplication()
{
    bFormationsDirty = false;
    if (!HasAuthority()) return;
    for (const FParticipantBinding& Binding : ParticipantBindings)
    {
        APolitikaPrototypePlayerController* Controller = Cast<APolitikaPrototypePlayerController>(Binding.Controller.Get());
        if (!Controller) continue;
        TArray<FPolitikaNetFormationInfo> Infos;
        for (const pf::Formation& Formation : Formations.All())
        {
            if (Formation.owner != Binding.RuntimePlayerId) continue;
            FPolitikaNetFormationInfo& Info = Infos.AddDefaulted_GetRef();
            Info.FormationId = Formation.id.value;
            Info.Shape = static_cast<uint8>(Formation.shape);
            Info.State = static_cast<uint8>(Formation.state);
            Info.bHasDestination = Formation.hasDestination;
            Info.Following = static_cast<uint8>(FMath::Min<std::size_t>(Formation.FollowingCount(), 255));
            Info.Regroups = static_cast<uint16>(FMath::Min<uint32>(Formation.regroups, 65535));
            Info.FormedMembers = static_cast<uint16>(Formation.formedMembers);
            Info.Stance = static_cast<uint8>(Formation.stance);
            Info.Adherence = static_cast<uint8>(Formation.adherence);
            for (const pf::MemberSlot& Member : Formation.members) Info.Members.Add(Member.id);
        }
        Controller->SetOwnFormations(MoveTemp(Infos));
    }
}

#if !UE_BUILD_SHIPPING
void APolitikaPrototypeMatch::TriggerDisciplineCasualties(const pf::Formation& Formation)
{
    constexpr std::size_t MinimumMembers = 8;
    constexpr int32 Victims = 4;
    constexpr uint64 DelaySteps = 10;
    if (DisciplineCasualtiesArmed < 0)
        DisciplineCasualtiesArmed = FParse::Param(FCommandLine::Get(), TEXT("PolitikaMPDisciplineCasualties")) ? 1 : 0;
    if (DisciplineCasualtiesArmed != 1 || Formation.members.size() < MinimumMembers) return;
    TArray<const APolitikaPrototypeUnit*> Living;
    FVector2D Centre = FVector2D::ZeroVector;
    for (const pf::MemberSlot& Member : Formation.members)
        if (const APolitikaPrototypeUnit* Unit = FindUnitByStableId(Member.id); Unit && Unit->IsAlive())
        {
            Living.Add(Unit);
            Centre += FVector2D(Unit->GetActorLocation());
        }
    if (Living.Num() < static_cast<int32>(MinimumMembers)) return;
    Centre /= Living.Num();
    // Farthest from the centre (lowest ID on a tie), then its nearest fellow members.
    const APolitikaPrototypeUnit* End = nullptr;
    double EndDistance = -1.0;
    for (const APolitikaPrototypeUnit* Unit : Living)
    {
        const double Distance = FVector2D::Distance(FVector2D(Unit->GetActorLocation()), Centre);
        if (Distance > EndDistance + 0.001 || (FMath::IsNearlyEqual(Distance, EndDistance, 0.001) && Unit->GetStableId() < End->GetStableId()))
        {
            End = Unit;
            EndDistance = Distance;
        }
    }
    const FVector2D EndAt(End->GetActorLocation());
    Living.Sort([EndAt](const APolitikaPrototypeUnit& A, const APolitikaPrototypeUnit& B)
    {
        const double DA = FVector2D::DistSquared(FVector2D(A.GetActorLocation()), EndAt);
        const double DB = FVector2D::DistSquared(FVector2D(B.GetActorLocation()), EndAt);
        return DA != DB ? DA < DB : A.GetStableId() < B.GetStableId();
    });
    DisciplineCasualtyVictims.Reset();
    FString Names;
    for (int32 Index = 0; Index < Victims; ++Index)
    {
        DisciplineCasualtyVictims.Add(Living[Index]->GetStableId());
        Names += FString::Printf(TEXT("%s%llu"), Index ? TEXT(",") : TEXT(""), Living[Index]->GetStableId());
    }
    DisciplineCasualtiesArmed = 2; // once per match
    DisciplineCasualtiesFormation = Formation.id.value;
    DisciplineCasualtiesFirstStep = SimulationStepsExecuted + DelaySteps;
    UE_LOG(LogPolitikaStartup, Display,
        TEXT("MP_DISCIPLINE_CASUALTIES_ARMED formation=%u participant=%u members=%d victims=%s first_step=%llu centre=%.1f,%.1f"),
        Formation.id.value, Formation.owner, Living.Num(), *Names, DisciplineCasualtiesFirstStep, Centre.X, Centre.Y);
}

void APolitikaPrototypeMatch::TickDisciplineCasualtyFixture()
{
    if (DisciplineCasualtiesArmed != 2 || DisciplineCasualtyVictims.IsEmpty() || SimulationStepsExecuted < DisciplineCasualtiesFirstStep) return;
    // One per step: the next step's damage and cleanup phases destroy it.
    const uint64 Victim = DisciplineCasualtyVictims[0];
    DisciplineCasualtyVictims.RemoveAt(0);
    APolitikaPrototypeUnit* Unit = FindUnitByStableId(Victim);
    if (!Unit || !Unit->IsAlive()) return;
    Unit->PendingDamage += static_cast<float>(Unit->GetHealth() + 1);
    const FVector Location = Unit->GetActorLocation();
    UE_LOG(LogPolitikaStartup, Display, TEXT("MP_DISCIPLINE_CASUALTY formation=%u unit=%llu x=%.1f y=%.1f remaining=%d step=%llu"),
        DisciplineCasualtiesFormation, Victim, Location.X, Location.Y, DisciplineCasualtyVictims.Num(), SimulationStepsExecuted);
}

void APolitikaPrototypeMatch::TriggerStanceProbe(uint32 ParticipantId, const pf::Formation& Formation)
{
    if (StanceProbeArmed < 0) StanceProbeArmed = FParse::Param(FCommandLine::Get(), TEXT("PolitikaMPStanceProbe")) ? 1 : 0;
    if (StanceProbeArmed != 1) return;
    double NearestCm = 0.0;
    const TCHAR* Phase = nullptr;
    if (Formation.stance == ps::CombatStance::Guard && !bStanceProbeGuardSent)
    {
        bStanceProbeGuardSent = true;
        NearestCm = 740.0;
        Phase = TEXT("guard");
    }
    else if (Formation.stance == ps::CombatStance::HoldFire && !bStanceProbeHoldFireSent)
    {
        bStanceProbeHoldFireSent = true;
        NearestCm = 450.0;
        Phase = TEXT("hold_fire");
    }
    else return;
    TArray<FVector2D> Members;
    FVector2D Centre = FVector2D::ZeroVector;
    for (const pf::MemberSlot& Member : Formation.members)
        if (const APolitikaPrototypeUnit* Unit = FindUnitByStableId(Member.id); Unit && Unit->IsAlive())
        {
            Members.Add(FVector2D(Unit->GetActorLocation()));
            Centre += Members.Last();
        }
    APolitikaPrototypeUnit* Probe = nullptr;
    for (APolitikaPrototypeUnit* Unit : Units)
        if (IsValid(Unit) && Unit->IsAlive() && Unit->GetOwnerPlayer() != 0 && Unit->GetOwnerPlayer() != ParticipantId
            && !StanceProbeUnits.Contains(Unit->GetStableId()) && (!Probe || Unit->GetStableId() < Probe->GetStableId()))
            Probe = Unit;
    if (Members.IsEmpty() || !Probe)
    {
        UE_LOG(LogPolitikaStartup, Display, TEXT("MP_STANCE_PROBE phase=%s placed=0 reason=no_units"), Phase);
        return;
    }
    StanceProbeUnits.Add(Probe->GetStableId());
    Centre /= Members.Num();
    const auto Nearest = [&Members](const FVector2D& Point)
    {
        double Best = TNumericLimits<double>::Max();
        for (const FVector2D& Member : Members) Best = FMath::Min(Best, FVector2D::Distance(Member, Point));
        return Best;
    };
    // Head-on from the probe unit's side, turning in 15 degree steps until the point is open
    // ground, inside the map and well clear of the control point (a capture would end the match).
    const FVector2D Base = (FVector2D(Probe->GetActorLocation()) - Centre).GetSafeNormal();
    const FVector2D ObjectivePoint = Objective ? FVector2D(Objective->GetActorLocation()) : FVector2D(TNumericLimits<double>::Max());
    for (int32 Turn = 0; Turn <= 12; ++Turn)
    {
        const double Degrees = 15.0 * ((Turn + 1) / 2) * (Turn % 2 == 0 ? 1.0 : -1.0);
        const FVector2D Direction = Base.GetRotated(Degrees);
        double Radius = 0.0;
        while (Radius < 6000.0 && Nearest(Centre + Direction * Radius) < NearestCm) Radius += 2.0;
        const FVector2D Point = Centre + Direction * Radius;
        const FVector2D MapPoint = Point - PlayableMin;
        float GroundZ = Probe->GetActorLocation().Z;
        if (FVector2D::Distance(Point, ObjectivePoint) < 900.0 || MapPoint.X < 200.0 || MapPoint.Y < 200.0
            || MapPoint.X > PlayableMax.X - PlayableMin.X - 200.0 || MapPoint.Y > PlayableMax.Y - PlayableMin.Y - 200.0
            || !FormationAnchorGround({MapPoint.X, MapPoint.Y}, GroundZ)
            || !IsGroundSlotOpen(GetWorld(), PlayableMin, PlayableMax, MapPoint, GroundZ))
            continue;
        SubmitMove(Probe->GetOwnerPlayer(), Probe, FVector(Point.X, Point.Y, GroundZ));
        UE_LOG(LogPolitikaStartup, Display,
            TEXT("MP_STANCE_PROBE phase=%s placed=1 formation=%u unit=%llu owner=%u x=%.1f y=%.1f nearest_member_cm=%.1f objective_cm=%.1f turn_deg=%.0f step=%llu"),
            Phase, Formation.id.value, Probe->GetStableId(), Probe->GetOwnerPlayer(), Point.X, Point.Y, Nearest(Point),
            FVector2D::Distance(Point, ObjectivePoint), Degrees, SimulationStepsExecuted);
        return;
    }
    UE_LOG(LogPolitikaStartup, Display, TEXT("MP_STANCE_PROBE phase=%s placed=0 reason=no_open_point"), Phase);
}

void APolitikaPrototypeMatch::TickFormationCasualtyFixture()
{
    if (FormationCasualtyArmed != 1 || FormationCasualtyFormation == 0 || SimulationStepsExecuted < FormationCasualtyStep) return;
    FormationCasualtyArmed = 2; // one casualty per match
    const pf::Formation* Formation = Formations.Find(pf::FormationId{FormationCasualtyFormation});
    if (!Formation) return;
    for (auto It = Formation->members.rbegin(); It != Formation->members.rend(); ++It)
    {
        if (!It->following) continue;
        APolitikaPrototypeUnit* Unit = FindUnitByStableId(It->id);
        if (!Unit || !Unit->IsAlive()) continue;
        // Ordinary damage: the next step's damage and cleanup phases destroy it.
        Unit->PendingDamage += static_cast<float>(Unit->GetHealth() + 1);
        UE_LOG(LogPolitikaStartup, Display, TEXT("MP_FORMATION_CASUALTY formation=%u unit=%llu step=%llu order=%u"),
            FormationCasualtyFormation, It->id, SimulationStepsExecuted, static_cast<uint32>(Unit->GetCurrentOrder()));
        return;
    }
}
#endif

// ----------------------------------------------------------------------------
// Controller: formation input, ingress and owner-only view
// ----------------------------------------------------------------------------

const FPolitikaNetFormationInfo* APolitikaPrototypePlayerController::FindOwnFormationOf(uint64 UnitId) const
{
    return OwnFormations.FindByPredicate([UnitId](const FPolitikaNetFormationInfo& Info) { return Info.Members.Contains(UnitId); });
}

void APolitikaPrototypePlayerController::SetOwnFormations(TArray<FPolitikaNetFormationInfo>&& Formations)
{
    if (!HasAuthority() || Formations == OwnFormations) return;
    OwnFormations = MoveTemp(Formations);
    if (IsLocalSeatController()) LogOwnFormations(TEXT("authority"));
}

void APolitikaPrototypePlayerController::OnRep_OwnFormations()
{
    LogOwnFormations(TEXT("replicated"));
}

void APolitikaPrototypePlayerController::LogOwnFormations(const TCHAR* Source) const
{
#if !UE_BUILD_SHIPPING
    FString Detail;
    for (const FPolitikaNetFormationInfo& Info : OwnFormations)
        Detail += FString::Printf(TEXT("%s%u:%s:%s:%d/%u:f%u:r%u:dest%d:ids=%s:stance=%s:adherence=%s"), Detail.IsEmpty() ? TEXT("") : TEXT(";"),
            Info.FormationId, ShapeText(static_cast<pf::Shape>(Info.Shape)),
            UTF8_TO_TCHAR(pf::StateName(static_cast<pf::FormationState>(Info.State)).data()), Info.Members.Num(),
            Info.FormedMembers, Info.Following, Info.Regroups, Info.bHasDestination ? 1 : 0, *JoinIds(Info.Members).Replace(TEXT(","), TEXT("/")),
            UTF8_TO_TCHAR(ps::StanceName(static_cast<ps::CombatStance>(Info.Stance)).data()),
            UTF8_TO_TCHAR(ps::AdherenceName(static_cast<ps::Adherence>(Info.Adherence)).data()));
    UE_LOG(LogPolitikaStartup, Display, TEXT("RTS_FORMATION_VIEW participant=%u source=%s formations=%d detail=%s"),
        AssignedRuntimePlayerId != 0 ? AssignedRuntimePlayerId : GetCommandPlayerId(), Source, OwnFormations.Num(),
        Detail.IsEmpty() ? TEXT("none") : *Detail);
#else
    (void)Source;
#endif
}

const FPolitikaNetFormationInfo* APolitikaPrototypePlayerController::SelectedWholeFormation(
    TConstArrayView<APolitikaPrototypeUnit*> Selected) const
{
    if (Selected.Num() < 2 || !Selected[0]) return nullptr;
    const FPolitikaNetFormationInfo* Formation = FindOwnFormationOf(Selected[0]->GetStableId());
    if (!Formation || Formation->Members.Num() != Selected.Num()) return nullptr;
    for (const APolitikaPrototypeUnit* Unit : Selected)
        if (!Unit || !Formation->Members.Contains(Unit->GetStableId())) return nullptr;
    return Formation;
}

bool APolitikaPrototypePlayerController::SubmitFormationRequest(APolitikaPrototypeMatch* ActiveMatch, pf::FormationOp Op,
    uint32 FormationId, TConstArrayView<APolitikaPrototypeUnit*> Members, const politika::rules::Position& Destination,
    const FVector* GroundHit, uint8 Value)
{
    if (!ActiveMatch) return false;
    FPolitikaNetFormationIntent Intent;
    Intent.ProtocolVersion = static_cast<int32>(FPolitikaBootstrap::Get().NetworkProtocol());
    Intent.Op = static_cast<uint8>(Op);
    Intent.FormationId = FormationId;
    Intent.Shape = Op == pf::FormationOp::Form ? static_cast<uint8>(GroupMoveShape) : 0;
    Intent.Value = Op == pf::FormationOp::SetStance || Op == pf::FormationOp::SetAdherence ? Value : 0;
    for (const APolitikaPrototypeUnit* Unit : Members)
        if (Unit) Intent.Members.Add(Unit->GetStableId());
    if (Op == pf::FormationOp::Move || Op == pf::FormationOp::AttackMove)
    {
        Intent.DestinationX = Destination.x;
        Intent.DestinationY = Destination.y;
    }
    Intent.ClientSequence = NextClientSequence++;
    Intent.CorrelationId = NextCorrelationId++;
    Intent.MatchGeneration = ActiveMatch->GetMatchGeneration();
    if (Op == pf::FormationOp::Move || Op == pf::FormationOp::AttackMove || Op == pf::FormationOp::Stop)
    {
        if (PendingFormationOrderKinds.Num() >= 16) PendingFormationOrderKinds.Reset();
        PendingFormationOrderKinds.Add(Intent.CorrelationId, static_cast<uint8>(Op == pf::FormationOp::Move
            ? politika::rules::OrderKind::Move : Op == pf::FormationOp::AttackMove ? politika::rules::OrderKind::AttackMove
            : politika::rules::OrderKind::Stop));
    }
#if !UE_BUILD_SHIPPING
    UE_LOG(LogPolitikaStartup, Display,
        TEXT("RTS_FORMATION_SENT participant=%u op=%s correlation=%u client_sequence=%u formation=%u shape=%s members=%d ids=%s x=%.1f y=%.1f value=%u authority=%d"),
        GetCommandPlayerId(), UTF8_TO_TCHAR(pf::OpName(Op).data()), Intent.CorrelationId, Intent.ClientSequence, FormationId,
        ShapeText(static_cast<pf::Shape>(Intent.Shape)), Intent.Members.Num(), *JoinIds(Intent.Members), Intent.DestinationX,
        Intent.DestinationY, Intent.Value, HasAuthority() ? 1 : 0);
#endif
    if (GroundHit)
    {
        // The server plans the slots; mark where the formation was sent.
        GroupMoveMarkers.Reset();
        GroupMoveMarkers.Add(*GroundHit);
        GroupMoveMarkersSeconds = FPlatformTime::Seconds();
    }
    if (HasAuthority()) return ActiveMatch->SubmitFormationIntent(this, Intent);
    ServerSubmitFormationIntent(Intent);
    return true;
}

void APolitikaPrototypePlayerController::ServerSubmitFormationIntent_Implementation(const FPolitikaNetFormationIntent& Intent)
{
    if (APolitikaPrototypeMatch* ActiveMatch = FindFormationMatch(GetWorld()))
        ActiveMatch->SubmitFormationIntent(this, Intent);
}

void APolitikaPrototypePlayerController::ClientReceiveFormationOutcome_Implementation(const FPolitikaNetFormationOutcome& Outcome)
{
#if !UE_BUILD_SHIPPING
    UE_LOG(LogPolitikaStartup, Display,
        TEXT("RTS_FORMATION_OUTCOME participant=%u op=%u correlation=%u client_sequence=%u formation=%u accepted=%d rejection=%u members=%d orders=%d generation=%llu authority=%d"),
        GetCommandPlayerId(), Outcome.Op, Outcome.CorrelationId, Outcome.ClientSequence, Outcome.FormationId,
        Outcome.bAccepted ? 1 : 0, Outcome.Rejection, Outcome.Members, Outcome.Orders, Outcome.MatchGeneration,
        HasAuthority() ? 1 : 0);
#endif
    uint8 OrderKind = 0;
    const bool bOrders = PendingFormationOrderKinds.RemoveAndCopyValue(Outcome.CorrelationId, OrderKind);
    if (!Outcome.bAccepted)
    {
        ShowLocalFeedback(FString::Printf(TEXT("Formation request rejected: %s"),
            *FormationRejectionText(static_cast<pf::FormationRejection>(Outcome.Rejection))));
        return;
    }
    const pf::FormationOp Op = static_cast<pf::FormationOp>(Outcome.Op);
    if (bOrders && Outcome.Orders >= 2)
    {
        // The member commands report under this correlation; tally them like a group order.
        FCommandBatch Batch;
        Batch.Id = NextBatchId++;
        Batch.OrderKind = OrderKind;
        Batch.FirstCorrelation = Outcome.CorrelationId;
        Batch.LastCorrelation = Outcome.CorrelationId;
        Batch.Tally.expected = static_cast<uint32>(Outcome.Orders);
        if (CommandBatches.Num() >= 8) CommandBatches.RemoveAt(0);
        CommandBatches.Add(Batch);
#if !UE_BUILD_SHIPPING
        UE_LOG(LogPolitikaStartup, Display,
            TEXT("RTS_GROUP_ORDER_SENT participant=%u batch=%u order=%u issuers=%d first_correlation=%u last_correlation=%u formation=%u"),
            GetCommandPlayerId(), Batch.Id, Batch.OrderKind, Outcome.Orders, Batch.FirstCorrelation, Batch.LastCorrelation,
            Outcome.FormationId);
#endif
    }
    switch (Op)
    {
        case pf::FormationOp::Form:
            ShowLocalFeedback(FString::Printf(TEXT("Formation %u formed: %d units in %s  ·  RMB moves the whole formation  ·  Y releases"),
                Outcome.FormationId, Outcome.Members, GroupMoveShape == pf::Shape::Column ? TEXT("COLUMN") : TEXT("LINE")));
            break;
        case pf::FormationOp::Release:
            ShowLocalFeedback(FString::Printf(TEXT("%d units released from their formation"), Outcome.Members));
            break;
        case pf::FormationOp::Move:
        case pf::FormationOp::AttackMove:
            ShowLocalFeedback(FString::Printf(TEXT("Formation %u %s: %d members"), Outcome.FormationId,
                Op == pf::FormationOp::Move ? TEXT("moving") : TEXT("attack-moving"), Outcome.Orders));
            break;
        case pf::FormationOp::Stop:
            ShowLocalFeedback(FString::Printf(TEXT("Formation %u stopped"), Outcome.FormationId));
            break;
        case pf::FormationOp::SetStance:
        case pf::FormationOp::SetAdherence:
        {
            uint8 Requested = 0;
            const bool bKnown = PendingStanceValues.RemoveAndCopyValue(Outcome.CorrelationId, Requested);
            const FString Value = !bKnown ? FString(TEXT("updated"))
                : Op == pf::FormationOp::SetStance ? FString(StanceText(static_cast<ps::CombatStance>(Requested)))
                : FString(AdherenceText(static_cast<ps::Adherence>(Requested)));
            ShowLocalFeedback(Outcome.FormationId != 0
                ? FString::Printf(TEXT("Formation %u %s: %s  ·  no order sent"), Outcome.FormationId,
                    Op == pf::FormationOp::SetStance ? TEXT("stance") : TEXT("adherence"), *Value)
                : FString::Printf(TEXT("%d unit%s set to %s  ·  no order sent"), Outcome.Members,
                    Outcome.Members == 1 ? TEXT("") : TEXT("s"), *Value));
            break;
        }
    }
}

void APolitikaPrototypePlayerController::CycleStanceSelected()
{
    APolitikaPrototypeMatch* ActiveMatch = ResolveMatch();
    const TArray<APolitikaPrototypeUnit*> Selected = GetCommandableSelectedUnits(ActiveMatch);
    if (Selected.IsEmpty())
    {
        ShowLocalFeedback(TEXT("Select your units to change their stance."));
        return;
    }
    // The next stance after the one the formation (or the first selected unit) holds now.
    const FPolitikaNetFormationInfo* Whole = SelectedWholeFormation(Selected);
    const ps::CombatStance Current = Whole ? static_cast<ps::CombatStance>(Whole->Stance) : Selected[0]->GetEffectiveStance();
    const uint8 Next = static_cast<uint8>(ps::NextStance(Current));
    // Recorded first: a host-local request is answered before SubmitFormationRequest returns.
    if (PendingStanceValues.Num() >= 16) PendingStanceValues.Reset();
    PendingStanceValues.Add(NextCorrelationId, Next);
    if (Whole)
        SubmitFormationRequest(ActiveMatch, pf::FormationOp::SetStance, Whole->FormationId, {}, {}, nullptr, Next);
    else
        SubmitFormationRequest(ActiveMatch, pf::FormationOp::SetStance, 0, Selected, {}, nullptr, Next);
}

void APolitikaPrototypePlayerController::CycleAdherenceSelected()
{
    APolitikaPrototypeMatch* ActiveMatch = ResolveMatch();
    const TArray<APolitikaPrototypeUnit*> Selected = GetCommandableSelectedUnits(ActiveMatch);
    const FPolitikaNetFormationInfo* Whole = SelectedWholeFormation(Selected);
    if (!Whole)
    {
        ShowLocalFeedback(TEXT("Adherence belongs to a formation: select one whole formation."));
        return;
    }
    const uint8 Next = static_cast<uint8>(ps::NextAdherence(static_cast<ps::Adherence>(Whole->Adherence)));
    if (PendingStanceValues.Num() >= 16) PendingStanceValues.Reset();
    PendingStanceValues.Add(NextCorrelationId, Next);
    SubmitFormationRequest(ActiveMatch, pf::FormationOp::SetAdherence, Whole->FormationId, {}, {}, nullptr, Next);
}

void APolitikaPrototypePlayerController::FormSelected()
{
    APolitikaPrototypeMatch* ActiveMatch = ResolveMatch();
    const TArray<APolitikaPrototypeUnit*> Selected = GetCommandableSelectedUnits(ActiveMatch);
    if (Selected.Num() < static_cast<int32>(pf::kMinFormationMembers))
    {
        ShowLocalFeedback(TEXT("Select at least two of your units to form a formation."));
        return;
    }
    if (Selected.Num() > static_cast<int32>(pf::kMaxFormationMembers))
    {
        ShowLocalFeedback(TEXT("A formation holds at most 64 units."));
        return;
    }
    SubmitFormationRequest(ActiveMatch, pf::FormationOp::Form, 0, Selected, {});
}

void APolitikaPrototypePlayerController::ReleaseSelected()
{
    APolitikaPrototypeMatch* ActiveMatch = ResolveMatch();
    const TArray<APolitikaPrototypeUnit*> Selected = GetCommandableSelectedUnits(ActiveMatch);
    if (Selected.IsEmpty()) return;
    TArray<APolitikaPrototypeUnit*> Members;
    for (APolitikaPrototypeUnit* Unit : Selected)
        if (FindOwnFormationOf(Unit->GetStableId())) Members.Add(Unit);
    if (Members.IsEmpty())
    {
        ShowLocalFeedback(TEXT("None of the selected units is in a formation."));
        return;
    }
    SubmitFormationRequest(ActiveMatch, pf::FormationOp::Release, 0, Members, {});
}
