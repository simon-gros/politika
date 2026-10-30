// Unit resolve: Discipline earned in formation (DISC-D1) and its stress and faction rules
// (DISC-D2, RTS_UNIT_DISCIPLINE.md).
//
// Authority boundary: only the server's fixed-step simulation changes a unit's resolve: once
// every politika::discipline::kEvaluationIntervalSteps, and at a formation member's death for
// casualty shock. Support is measured from the step's actual positions among living members of
// the unit's own formation, so selection, control groups, a neighbouring formation or a dead
// member never count. Isolation is measured among living friendly soldiers (same owner, or same
// alliance team). Clients receive their own units' values for the HUD and nothing else.
// DISC-D3: the value drives damage taken, fire interval, reload time, return-to-post speed and
// the formation regroup delay through the unit's modifier registry.
// DISC-D4: a soldier at the broken threshold routs: it leaves its formation, stops fighting,
// refuses orders (OrderRejection::Routed) and flees to its side's start, recovering at the fade
// rate until it rallies at politika::discipline::kRallyThreshold.

#include "PolitikaPrototypeGameplay.h"

#include "PolitikaBootstrap.h"
#include "PolitikaInfantryPresentation.h"
#include "Discipline.h"
#include "Formation.h"

#include <limits>
#include <vector>

namespace pd = politika::discipline;
namespace pf = politika::formation;

namespace
{
// Content faction IDs of the playable factions and the Unaligned (UNIT_SPECIFICATION_FRAMEWORK.md).
// Prototype test factions have no faction rules and use the Standard doctrine.
pd::Doctrine DoctrineOfFactionId(FName FactionId)
{
    using politika::rules::Faction;
    static const TMap<FName, Faction> Known = {
        {FName(TEXT("faction.atlantic_concord")), Faction::AtlanticConcord},
        {FName(TEXT("faction.red_directorate")), Faction::RedDirectorate},
        {FName(TEXT("faction.iron_ascendancy")), Faction::IronAscendancy},
        {FName(TEXT("faction.meridian_pact")), Faction::MeridianPact},
        {FName(TEXT("faction.unaligned")), Faction::Unaligned},
    };
    const Faction* Found = Known.Find(FactionId);
    return Found ? pd::DoctrineOf(*Found) : pd::Doctrine::Standard;
}

bool AreComrades(const APolitikaPrototypeUnit& A, const APolitikaPrototypeUnit& B)
{
    return A.GetOwnerPlayer() == B.GetOwnerPlayer()
        || (A.GetTeamId() != politika::rules::kNoTeam && A.GetTeamId() == B.GetTeamId());
}

#if !UE_BUILD_SHIPPING
const TCHAR* DoctrineName(pd::Doctrine Doctrine)
{
    switch (Doctrine)
    {
        case pd::Doctrine::Standard: return TEXT("standard");
        case pd::Doctrine::MassFormation: return TEXT("mass");
        case pd::Doctrine::SoloTolerant: return TEXT("solo");
    }
    return TEXT("invalid");
}

// Samples for the development log every 5 s.
constexpr uint64 ResolveSampleIntervalSteps = 100;

const TCHAR* ServiceName(pd::Service Service)
{
    switch (Service)
    {
        case pd::Service::Outside: return TEXT("outside");
        case pd::Service::MemberHolding: return TEXT("holding");
        case pd::Service::MemberEarning: return TEXT("earning");
        case pd::Service::Isolated: return TEXT("isolated");
        case pd::Service::Routed: return TEXT("routed");
    }
    return TEXT("invalid");
}

struct FResolveTally
{
    int32 Count = 0;
    int32 Earning = 0;
    int32 Holding = 0;
    int32 Min = std::numeric_limits<int32>::max();
    int32 Max = 0;
    int64 Sum = 0;
    void Add(int32 Value, pd::Service Service)
    {
        ++Count;
        Earning += Service == pd::Service::MemberEarning ? 1 : 0;
        Holding += Service == pd::Service::MemberHolding ? 1 : 0;
        Min = FMath::Min(Min, Value);
        Max = FMath::Max(Max, Value);
        Sum += Value;
    }
    int32 Average() const { return Count > 0 ? static_cast<int32>(Sum / Count) : 0; }
};
#endif
} // namespace

pd::Doctrine APolitikaPrototypeMatch::DisciplineDoctrineOf(const APolitikaPrototypeUnit& Unit)
{
#if !UE_BUILD_SHIPPING
    if (!bDisciplineDoctrineOverridesParsed)
    {
        bDisciplineDoctrineOverridesParsed = true;
        FString Spec;
        if (FParse::Value(FCommandLine::Get(), TEXT("PolitikaMPDisciplineDoctrine="), Spec))
        {
            TArray<FString> Entries;
            Spec.ParseIntoArray(Entries, TEXT(","));
            for (const FString& Entry : Entries)
            {
                FString Participant, Name;
                if (!Entry.Split(TEXT(":"), &Participant, &Name)) continue;
                const uint32 Id = static_cast<uint32>(FCString::Atoi(*Participant));
                TOptional<pd::Doctrine> Parsed;
                if (Name == TEXT("standard")) Parsed = pd::Doctrine::Standard;
                else if (Name == TEXT("mass")) Parsed = pd::Doctrine::MassFormation;
                else if (Name == TEXT("solo")) Parsed = pd::Doctrine::SoloTolerant;
                // Automation entries set before the first lookup win.
                if (!Parsed || DisciplineDoctrineOverrides.Contains(Id)) continue;
                DisciplineDoctrineOverrides.Add(Id, *Parsed);
                UE_LOG(LogPolitikaStartup, Display, TEXT("RTS_DISCIPLINE_DOCTRINE_OVERRIDE participant=%u doctrine=%s"),
                    Id, DoctrineName(*Parsed));
            }
        }
    }
#endif
    if (const pd::Doctrine* Override = DisciplineDoctrineOverrides.Find(Unit.GetOwnerPlayer())) return *Override;
    return DoctrineOfFactionId(Unit.GetFactionId());
}

void APolitikaPrototypeMatch::ApplyCasualtyShock(const APolitikaPrototypeUnit& Fallen)
{
    const pf::FormationId Id = Formations.FormationOf(Fallen.GetStableId());
    const pf::Formation* Formation = Formations.Find(Id);
    if (!Formation) return;
    const FVector Where = Fallen.GetActorLocation();
    for (const pf::MemberSlot& Member : Formation->members)
    {
        if (Member.id == Fallen.GetStableId()) continue;
        APolitikaPrototypeUnit* Unit = FindUnitByStableId(Member.id);
        // Members falling in the same step are not shocked by each other.
        if (!Unit || !Unit->IsAlive()) continue;
        if (FVector::DistSquared2D(Unit->GetActorLocation(), Where) > FMath::Square(pd::kCasualtyShockRadiusCm))
        {
#if !UE_BUILD_SHIPPING
            // Out of range: listed so a packaged run can check the radius from both sides.
            UE_LOG(LogPolitikaStartup, Display,
                TEXT("RTS_DISCIPLINE_SHOCK casualty=%llu unit=%llu participant=%u formation=%u in_range=0 distance=%.1f before=%d after=%d applied=0 step=%llu"),
                Fallen.GetStableId(), Unit->GetStableId(), Unit->GetOwnerPlayer(), Id.value,
                FVector::Dist2D(Unit->GetActorLocation(), Where), Unit->GetResolveMilli(), Unit->GetResolveMilli(), SimulationStepsExecuted);
#endif
            continue;
        }
        const int32 Before = Unit->GetResolveMilli();
        const pd::ShockResult Shock = pd::ApplyCasualtyShock(Before, Unit->ResolveShock, SimulationStepsExecuted);
        Unit->ResolveShock = Shock.window;
        if (Shock.applied > 0) Unit->ApplyResolve({Shock.value, pd::Trend::Fading});
#if !UE_BUILD_SHIPPING
        UE_LOG(LogPolitikaStartup, Display,
            TEXT("RTS_DISCIPLINE_SHOCK casualty=%llu unit=%llu participant=%u formation=%u in_range=1 distance=%.1f before=%d after=%d applied=%d window_start=%llu window_taken=%d step=%llu"),
            Fallen.GetStableId(), Unit->GetStableId(), Unit->GetOwnerPlayer(), Id.value,
            FVector::Dist2D(Unit->GetActorLocation(), Where), Before, Shock.value, Shock.applied, Shock.window.start,
            Shock.window.taken, SimulationStepsExecuted);
        if (const std::optional<std::int32_t> Band = pd::CrossedBand(Before, Shock.value))
            UE_LOG(LogPolitikaStartup, Display,
                TEXT("RTS_DISCIPLINE_BAND unit=%llu participant=%u band=%d direction=fading value=%d service=shock formation=%u step=%llu"),
                Unit->GetStableId(), Unit->GetOwnerPlayer(), *Band, Shock.value, Id.value, SimulationStepsExecuted);
#endif
    }
}

void APolitikaPrototypeMatch::UpdateUnitResolve()
{
    if (SimulationStepsExecuted == 0 || SimulationStepsExecuted % pd::kEvaluationIntervalSteps != 0) return;

    // Service and ceiling of every formation member this evaluation; units absent here are outside.
    struct FMemberEvaluation
    {
        pd::Service Service = pd::Service::MemberHolding;
        int32 Ceiling = pd::kNormalCeiling;
    };
    TMap<uint64, FMemberEvaluation> MemberService;
    std::vector<pf::Point> Positions;
    std::vector<uint64> Earning;
    for (const pf::Formation& Formation : Formations.All())
    {
        Positions.clear();
        Positions.reserve(Formation.members.size());
        for (const pf::MemberSlot& Member : Formation.members)
        {
            const APolitikaPrototypeUnit* Unit = FindUnitByStableId(Member.id);
            if (Unit && Unit->IsAlive())
                Positions.push_back({Unit->GetActorLocation().X, Unit->GetActorLocation().Y});
            else
                Positions.push_back({std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::quiet_NaN()});
        }
        const std::size_t Required = pd::RequiredSupport(Formation.members.size());
        Earning.clear();
        for (std::size_t Index = 0; Index < Formation.members.size(); ++Index)
        {
            const pf::MemberSlot& Member = Formation.members[Index];
            // Independent: an individual order took the member off the formation's current move.
            const bool bIndependent = Formation.hasDestination && !Member.following;
            const bool bSupported = pd::SupportCount(Positions, Index) >= Required;
            const bool bEarning = !bIndependent && bSupported;
            MemberService.Add(Member.id, {bEarning ? pd::Service::MemberEarning : pd::Service::MemberHolding});
            if (bEarning) Earning.push_back(Member.id);
        }
        // The mass-formation ceiling counts this evaluation's earning members only. Members of one
        // formation share an owner and therefore a doctrine.
        const APolitikaPrototypeUnit* Representative = Formation.members.empty() ? nullptr : FindUnitByStableId(Formation.members.front().id);
        const pd::Doctrine Doctrine = Representative ? DisciplineDoctrineOf(*Representative) : pd::Doctrine::Standard;
        const int32 Ceiling = pd::FormationCeiling(Doctrine, Earning.size());
        for (const uint64 MemberId : Earning) MemberService.FindChecked(MemberId).Ceiling = Ceiling;
#if !UE_BUILD_SHIPPING
        if (Ceiling != pd::kNormalCeiling && SimulationStepsExecuted % 100 == 0)
            UE_LOG(LogPolitikaStartup, Display,
                TEXT("RTS_DISCIPLINE_MASS formation=%u participant=%u doctrine=%s earning=%d ceiling=%d step=%llu"),
                Formation.id.value, Formation.owner, DoctrineName(Doctrine), static_cast<int32>(Earning.size()), Ceiling,
                SimulationStepsExecuted);
#endif
    }

    // Soldiers outside any formation with no living friendly soldier within the support radius.
    // Each lone unit stops at its first comrade, so the scan is cheap when units stand together.
    auto IsAlone = [this](const APolitikaPrototypeUnit& Unit)
    {
        const FVector Location = Unit.GetActorLocation();
        const double RadiusSquared = FMath::Square(pd::kSupportRadiusCm);
        for (const APolitikaPrototypeUnit* Other : Units)
        {
            if (Other == &Unit || !IsValid(Other) || !Other->IsAlive() || !Other->IsMilitary() || !AreComrades(Unit, *Other)) continue;
            if (FVector::DistSquared2D(Other->GetActorLocation(), Location) <= RadiusSquared) return false;
        }
        return true;
    };

#if !UE_BUILD_SHIPPING
    const bool bSample = SimulationStepsExecuted % ResolveSampleIntervalSteps == 0;
    TMap<uint32, FResolveTally> ByFormation;
    TMap<uint32, FResolveTally> ByParticipant;
    TMap<uint32, FResolveTally> OutsideByParticipant;
#endif
    for (APolitikaPrototypeUnit* Unit : Units)
    {
        if (!IsValid(Unit) || !Unit->IsAlive()) continue;
        const FMemberEvaluation* Found = MemberService.Find(Unit->GetStableId());
        // Only military units serve; Form refuses civilians, so this is a safeguard. A routed
        // unit (never a member) recovers wherever it is (DISC-D4).
        pd::Service Service = Unit->IsRouted() ? pd::Service::Routed
            : Found && Unit->IsMilitary() ? Found->Service : pd::Service::Outside;
        bool bAlone = false;
        pd::Doctrine Doctrine = pd::Doctrine::Standard;
        if (Service == pd::Service::Outside && Unit->IsMilitary())
        {
            bAlone = IsAlone(*Unit);
            if (bAlone)
            {
                Doctrine = DisciplineDoctrineOf(*Unit);
                if (pd::IsolationApplies(Doctrine, pd::UnitCategory::Military)) Service = pd::Service::Isolated;
            }
        }
        const int32 Before = Unit->GetResolveMilli();
        Unit->ApplyResolve(pd::Evaluate(Before, Service, Found ? Found->Ceiling : pd::kNormalCeiling));
        if (Service == pd::Service::Routed && pd::Rallies(Unit->GetResolveMilli()))
        {
            const EPolitikaPrototypeOrder PreviousOrder = Unit->GetCurrentOrder();
#if !UE_BUILD_SHIPPING
            UE_LOG(LogPolitikaStartup, Display,
                TEXT("RTS_DISCIPLINE_ROUT phase=rally unit=%llu participant=%u value=%d routed_steps=%llu x=%.1f y=%.1f rally_distance=%.1f order=%u step=%llu"),
                Unit->GetStableId(), Unit->GetOwnerPlayer(), Unit->GetResolveMilli(), SimulationStepsExecuted - Unit->RoutStartStep,
                Unit->GetActorLocation().X, Unit->GetActorLocation().Y, FVector::Dist2D(Unit->GetActorLocation(), Unit->RoutRallyPoint),
                static_cast<uint32>(PreviousOrder), SimulationStepsExecuted);
#endif
            Unit->EndRout();
            if (PreviousOrder != Unit->GetCurrentOrder())
                QueueGameplayEvent(politika::rules::OrderChangedEvent{SimulationStepsExecuted, Unit->GetStableId(),
                    PolitikaExecutionOrderOf(PreviousOrder), PolitikaExecutionOrderOf(Unit->GetCurrentOrder()), politika::rules::OrderEndReason::Stopped});
        }
#if !UE_BUILD_SHIPPING
        const int32 After = Unit->GetResolveMilli();
        // Lone soldiers are few; list them, exempt or not, so isolation can be checked per unit.
        if (bAlone && SimulationStepsExecuted % ResolveSampleIntervalSteps == 0)
            UE_LOG(LogPolitikaStartup, Display,
                TEXT("RTS_DISCIPLINE_ISOLATED unit=%llu participant=%u doctrine=%s exempt=%d value=%d step=%llu"),
                Unit->GetStableId(), Unit->GetOwnerPlayer(), DoctrineName(Doctrine), Service == pd::Service::Isolated ? 0 : 1,
                After, SimulationStepsExecuted);
        if (const std::optional<std::int32_t> Band = pd::CrossedBand(Before, After))
            UE_LOG(LogPolitikaStartup, Display,
                TEXT("RTS_DISCIPLINE_BAND unit=%llu participant=%u band=%d direction=%s value=%d service=%s formation=%u step=%llu"),
                Unit->GetStableId(), Unit->GetOwnerPlayer(), *Band, After > Before ? TEXT("rising") : TEXT("fading"), After,
                ServiceName(Service), Formations.FormationOf(Unit->GetStableId()).value, SimulationStepsExecuted);
        if (bSample && Unit->GetOwnerPlayer() != 0)
        {
            const pf::FormationId Formation = Formations.FormationOf(Unit->GetStableId());
            if (Formation) ByFormation.FindOrAdd(Formation.value).Add(After, Service);
            else OutsideByParticipant.FindOrAdd(Unit->GetOwnerPlayer()).Add(After, Service);
            ByParticipant.FindOrAdd(Unit->GetOwnerPlayer()).Add(After, Service);
            // Members that are not earning are few; list them so a hold can be checked per unit.
            if (Service == pd::Service::MemberHolding)
                UE_LOG(LogPolitikaStartup, Display,
                    TEXT("RTS_DISCIPLINE_HOLDING unit=%llu formation=%u value=%d step=%llu"),
                    Unit->GetStableId(), Formation.value, After, SimulationStepsExecuted);
        }
#endif
    }
#if !UE_BUILD_SHIPPING
    if (!bSample) return;
    ByFormation.KeySort(TLess<uint32>());
    for (const TPair<uint32, FResolveTally>& Entry : ByFormation)
    {
        const pf::Formation* Formation = Formations.Find(pf::FormationId{Entry.Key});
        UE_LOG(LogPolitikaStartup, Display,
            TEXT("RTS_DISCIPLINE_SAMPLE formation=%u participant=%u members=%d earning=%d holding=%d min=%d avg=%d max=%d step=%llu"),
            Entry.Key, Formation ? Formation->owner : 0u, Entry.Value.Count, Entry.Value.Earning, Entry.Value.Holding,
            Entry.Value.Min, Entry.Value.Average(), Entry.Value.Max, SimulationStepsExecuted);
    }
    ByParticipant.KeySort(TLess<uint32>());
    for (const TPair<uint32, FResolveTally>& Entry : ByParticipant)
    {
        const FResolveTally* Outside = OutsideByParticipant.Find(Entry.Key);
        UE_LOG(LogPolitikaStartup, Display,
            TEXT("RTS_DISCIPLINE_PARTICIPANT participant=%u units=%d members=%d earning=%d outside=%d outside_min=%d outside_avg=%d outside_max=%d min=%d avg=%d max=%d step=%llu"),
            Entry.Key, Entry.Value.Count, Entry.Value.Count - (Outside ? Outside->Count : 0), Entry.Value.Earning,
            Outside ? Outside->Count : 0, Outside ? Outside->Min : 0, Outside ? Outside->Average() : 0, Outside ? Outside->Max : 0,
            Entry.Value.Min, Entry.Value.Average(), Entry.Value.Max, SimulationStepsExecuted);
    }
#endif
}

// ----------------------------------------------------------------------------
// Rout (DISC-D4)
// ----------------------------------------------------------------------------
// Casualty shock is what takes a soldier to the broken threshold, so breaks are checked once per
// step after the casualties and before the evaluation. A routed soldier is no formation member,
// so no later shock reaches it; it recovers at the fade rate (UpdateUnitResolve) and rallies at
// kRallyThreshold.

FVector APolitikaPrototypeMatch::RallyPointFor(const APolitikaPrototypeUnit& Unit) const
{
    if (MapDefinition)
    {
        const FPolitikaPrototypeRuntimeParticipant* Participant = RuntimeParticipants.FindByPredicate(
            [&Unit](const FPolitikaPrototypeRuntimeParticipant& Candidate) { return Candidate.RuntimePlayerId == Unit.GetOwnerPlayer(); });
        const FPolitikaPrototypePlayerSlot* Slot = Participant ? MapDefinition->PlayerSlots.FindByPredicate(
            [Participant](const FPolitikaPrototypePlayerSlot& Candidate) { return Candidate.SlotId == Participant->SlotId; }) : nullptr;
        if (Slot && !Slot->StartLocation.IsNearlyZero())
            return FVector(Slot->StartLocation.X, Slot->StartLocation.Y, Unit.GetActorLocation().Z);
    }
    return FVector(Unit.HomeLocation.X, Unit.HomeLocation.Y, Unit.GetActorLocation().Z);
}

void APolitikaPrototypeMatch::ProcessDisciplineBreaks()
{
    TArray<APolitikaPrototypeUnit*> Breaking;
    for (APolitikaPrototypeUnit* Unit : Units)
        if (IsValid(Unit) && Unit->IsAlive() && !Unit->IsRouted()
            && pd::Breaks(Unit->GetResolveMilli(), Unit->IsMilitary() ? pd::UnitCategory::Military : pd::UnitCategory::NonCombat))
            Breaking.Add(Unit);
    Breaking.Sort([](const APolitikaPrototypeUnit& A, const APolitikaPrototypeUnit& B) { return A.GetStableId() < B.GetStableId(); });
    for (APolitikaPrototypeUnit* Unit : Breaking) BreakUnit(*Unit);
}

void APolitikaPrototypeMatch::BreakUnit(APolitikaPrototypeUnit& Unit)
{
    const EPolitikaPrototypeOrder PreviousOrder = Unit.GetCurrentOrder();
    const pf::FormationId Formation = Formations.FormationOf(Unit.GetStableId());
    // Routed first: the formation loss and any later Form see a unit that is no longer operational.
    Unit.bRouted = true;
    if (Formation) LoseFormationMember(Unit.GetStableId(), pf::LeaveReason::Routed, TEXT("routed"));
    const FVector Rally = RallyPointFor(Unit);
    TArray<FVector> Route;
    FVector Resolved = Rally;
    FString Failure = TEXT("none");
    // Already at the rally point: nothing to flee to.
    bool bRoute = false;
    if (FVector::Dist2D(Unit.GetActorLocation(), Rally) > 100.0)
    {
#if WITH_DEV_AUTOMATION_TESTS
        if (RoutRouteForTest)
        {
            bRoute = RoutRouteForTest(Unit, Rally, Route);
            Failure = bRoute ? TEXT("none") : TEXT("test_route");
        }
        else
#endif
        bRoute = ResolveNavigationRoute(&Unit, Rally, Route, Resolved, Failure, 0);
    }
    else Failure = TEXT("at_rally_point");
    Unit.BeginRout(Rally, Resolved, bRoute ? MoveTemp(Route) : TArray<FVector>(), MatchGeneration, SimulationStepsExecuted);
#if !UE_BUILD_SHIPPING
    UE_LOG(LogPolitikaStartup, Display,
        TEXT("RTS_DISCIPLINE_ROUT phase=break unit=%llu participant=%u value=%d formation=%u previous_order=%u order=%u route=%d failure=%s x=%.1f y=%.1f rally=%.1f,%.1f rally_distance=%.1f step=%llu"),
        Unit.GetStableId(), Unit.GetOwnerPlayer(), Unit.GetResolveMilli(), Formation.value, static_cast<uint32>(PreviousOrder),
        static_cast<uint32>(Unit.GetCurrentOrder()), bRoute ? 1 : 0, bRoute ? TEXT("none") : *Failure, Unit.GetActorLocation().X, Unit.GetActorLocation().Y,
        Rally.X, Rally.Y, FVector::Dist2D(Unit.GetActorLocation(), Rally), SimulationStepsExecuted);
#endif
    if (PreviousOrder != Unit.GetCurrentOrder() || PreviousOrder != EPolitikaPrototypeOrder::Idle)
        QueueGameplayEvent(politika::rules::OrderChangedEvent{SimulationStepsExecuted, Unit.GetStableId(),
            PolitikaExecutionOrderOf(PreviousOrder), PolitikaExecutionOrderOf(Unit.GetCurrentOrder()), politika::rules::OrderEndReason::Routed});
    LastCommandFeedback = FString::Printf(TEXT("%s broke and is routing."), *Unit.GetUnitName().ToString());
#if !UE_BUILD_SHIPPING
    TriggerRoutWitness(Unit);
#endif
}

#if !UE_BUILD_SHIPPING
void APolitikaPrototypeMatch::TriggerRoutWitness(const APolitikaPrototypeUnit& Routed)
{
    if (RoutWitnessArmed < 0) RoutWitnessArmed = FParse::Param(FCommandLine::Get(), TEXT("PolitikaMPDisciplineRoutWitness")) ? 1 : 0;
    if (RoutWitnessArmed != 1) return;
    RoutWitnessArmed = 2; // once per match
    APolitikaPrototypeUnit* Witness = nullptr;
    for (APolitikaPrototypeUnit* Unit : Units)
        if (IsValid(Unit) && Unit->IsAlive() && Unit->GetOwnerPlayer() != 0 && Unit->GetOwnerPlayer() != Routed.GetOwnerPlayer()
            && Routed.IsHostileTo(Unit) && (!Witness || Unit->GetStableId() < Witness->GetStableId()))
            Witness = Unit;
    if (!Witness)
    {
        UE_LOG(LogPolitikaStartup, Display, TEXT("MP_DISCIPLINE_ROUT_WITNESS placed=0 reason=no_unit"));
        return;
    }
    // 1000 cm from the rally point, first towards the witness's own side, turning in 15 degree
    // steps until the point is open ground inside the map, 900 cm clear of the control point (the
    // witness must not stand on it), and its straight walk keeps 900 cm from every soldier of the
    // routed side that can still fire, so the witness does not fight.
    const FVector2D Rally(Routed.RoutRallyPoint);
    const FVector2D From(Witness->GetActorLocation());
    const FVector2D Base = (From - Rally).GetSafeNormal();
    const FVector2D ObjectivePoint = Objective ? FVector2D(Objective->GetActorLocation()) : FVector2D(TNumericLimits<double>::Max());
    TArray<FVector2D> Avoid;
    for (const APolitikaPrototypeUnit* Unit : Units)
        if (IsValid(Unit) && Unit->IsAlive() && !Unit->IsRouted() && Unit->GetOwnerPlayer() == Routed.GetOwnerPlayer())
            Avoid.Add(FVector2D(Unit->GetActorLocation()));
    const auto Clearance = [&Avoid, &From](const FVector2D& To)
    {
        double Best = TNumericLimits<double>::Max();
        const FVector2D Segment = To - From;
        const double Length = Segment.SizeSquared();
        for (const FVector2D& Point : Avoid)
        {
            const double T = Length > 0.0 ? FMath::Clamp(FVector2D::DotProduct(Point - From, Segment) / Length, 0.0, 1.0) : 0.0;
            Best = FMath::Min(Best, FVector2D::Distance(Point, From + Segment * T));
        }
        return Best;
    };
    constexpr double DistanceCm = 1000.0;
    for (int32 Turn = 0; Turn <= 24; ++Turn)
    {
        const double Degrees = 15.0 * ((Turn + 1) / 2) * (Turn % 2 == 0 ? 1.0 : -1.0);
        const FVector2D Point = Rally + Base.GetRotated(Degrees) * DistanceCm;
        const FVector2D MapPoint = Point - PlayableMin;
        float GroundZ = Witness->GetActorLocation().Z;
        if (Clearance(Point) < 900.0 || FVector2D::Distance(Point, ObjectivePoint) < 900.0 || MapPoint.X < 200.0 || MapPoint.Y < 200.0
            || MapPoint.X > PlayableMax.X - PlayableMin.X - 200.0 || MapPoint.Y > PlayableMax.Y - PlayableMin.Y - 200.0
            || !FormationAnchorGround({MapPoint.X, MapPoint.Y}, GroundZ)
            || !IsGroundSlotOpen(GetWorld(), PlayableMin, PlayableMax, MapPoint, GroundZ))
            continue;
        SubmitMove(Witness->GetOwnerPlayer(), Witness, FVector(Point.X, Point.Y, GroundZ));
        UE_LOG(LogPolitikaStartup, Display,
            TEXT("MP_DISCIPLINE_ROUT_WITNESS placed=1 unit=%llu owner=%u x=%.1f y=%.1f rally=%.1f,%.1f rally_distance=%.1f clearance=%.1f turn_deg=%.0f step=%llu"),
            Witness->GetStableId(), Witness->GetOwnerPlayer(), Point.X, Point.Y, Rally.X, Rally.Y, DistanceCm, Clearance(Point),
            Degrees, SimulationStepsExecuted);
        return;
    }
    UE_LOG(LogPolitikaStartup, Display, TEXT("MP_DISCIPLINE_ROUT_WITNESS placed=0 reason=no_open_point"));
}
#endif

void APolitikaPrototypeUnit::BeginRout(const FVector& RallyPoint, const FVector& ResolvedDestination, TArray<FVector>&& Route,
    uint64 Generation, uint64 Step)
{
    bRouted = true;
    RoutRallyPoint = RallyPoint;
    RoutStartStep = Step;
    GrenadeWindup = 0;
    GrenadeTarget.Reset();
    InfantryPresentation->CancelGrenade();
    ResetAutonomy();
    OrderTarget.Reset();
    NavigationReplanCooldownSeconds = 0.0f;
    NavigationReplanAttempts = 0;
    NavigationOrderSequence = 0;
    if (Route.IsEmpty())
    {
        // No way back: the unit cowers where it is.
        CurrentOrder = EPolitikaPrototypeOrder::Idle;
        NavigationWaypoints.Reset();
        NavigationWaypointIndex = 0;
        NavigationMatchGeneration = 0;
        OrderDestination = GetActorLocation();
        return;
    }
    // A plain Move never acquires a target, so the unit runs without firing.
    CurrentOrder = EPolitikaPrototypeOrder::Move;
    OrderDestination = RallyPoint;
    ResolvedNavigationDestination = ResolvedDestination;
    NavigationWaypoints = MoveTemp(Route);
    NavigationWaypointIndex = NavigationWaypoints.Num() > 1 ? 1 : 0;
    NavigationMatchGeneration = Generation;
#if !UE_BUILD_SHIPPING
    bAcceptanceMovementLogged = false;
#endif
}

void APolitikaPrototypeUnit::EndRout()
{
    bRouted = false;
    CurrentOrder = EPolitikaPrototypeOrder::Idle;
    NavigationWaypoints.Reset();
    NavigationWaypointIndex = 0;
    NavigationMatchGeneration = 0;
    NavigationReplanCooldownSeconds = 0.0f;
    NavigationReplanAttempts = 0;
    OrderTarget.Reset();
    // Idle autonomy takes its new post where it rallied.
    ResetAutonomy();
}

// ----------------------------------------------------------------------------
// Discipline effects (DISC-D3)
// ----------------------------------------------------------------------------
// Each consumer reads the unit's combined modifier once per use. RTS_DISCIPLINE_EFFECT lists every
// use whose multiplier is not neutral, with the unit's value, so a run can check the formula per
// use; at the baseline the multipliers are exactly 1000 and nothing is logged.

namespace
{
#if !UE_BUILD_SHIPPING
const TCHAR* AttributeLogName(politika::modifiers::Attribute Value)
{
    // The same names as politika::modifiers::AttributeName.
    using politika::modifiers::Attribute;
    switch (Value)
    {
        case Attribute::DamageTaken: return TEXT("damage_taken");
        case Attribute::Spread: return TEXT("spread");
        case Attribute::FireInterval: return TEXT("fire_interval");
        case Attribute::ReloadTime: return TEXT("reload_time");
        case Attribute::ReturnSpeed: return TEXT("return_speed");
        case Attribute::RegroupDelay: return TEXT("regroup_delay");
    }
    return TEXT("invalid");
}
#endif
} // namespace

void APolitikaPrototypeUnit::ReceiveWeaponHit(float Damage, const APolitikaPrototypeUnit* Shooter, bool bBlast)
{
    if (!HasAuthority() || !IsAlive() || !(Damage > 0.0f)) return;
    namespace pm = politika::modifiers;
    const int32 Permille = GetModifierPermille(pm::Attribute::DamageTaken);
    const int64 BaseMilli = FMath::RoundToInt64(static_cast<double>(Damage) * 1000.0);
    const int64 AppliedMilli = BaseMilli * Permille / pm::kNeutral;
    PendingWeaponDamageMilli += AppliedMilli;
    if (bBlast) bPendingBlastDamage = true;
#if !UE_BUILD_SHIPPING
    if (Permille != pm::kNeutral)
        UE_LOG(LogPolitikaStartup, Display,
            TEXT("RTS_DISCIPLINE_EFFECT effect=%s unit=%llu participant=%u shooter=%llu value=%d permille=%d base_milli=%lld applied_milli=%lld blast=%d step=%llu"),
            AttributeLogName(pm::Attribute::DamageTaken), StableId, OwnerPlayer, Shooter ? Shooter->GetStableId() : 0ull,
            ResolveMilli, Permille, BaseMilli, AppliedMilli, bBlast ? 1 : 0, AuthorityStep());
#else
    (void)Shooter;
#endif
}

float APolitikaPrototypeUnit::NextFireInterval(float DeltaSeconds)
{
    namespace pm = politika::modifiers;
    const int32 Permille = GetModifierPermille(pm::Attribute::FireInterval);
    // At the neutral multiplier the weapon's own interval runs unchanged and nothing is owed.
    if (Permille == pm::kNeutral)
    {
        FireIntervalCarryMilliSteps = 0;
        return WeaponCooldown;
    }
    if (!(DeltaSeconds > 0.0f)) return WeaponCooldown * static_cast<float>(Permille) / static_cast<float>(pm::kNeutral);
    // The countdown fires on the step it reaches zero, so the weapon's interval is this many
    // whole steps; the modifier scales that, and the fraction of a step is owed to the next shot.
    const int32 BaseSteps = FMath::Max(1, FMath::CeilToInt((WeaponCooldown - KINDA_SMALL_NUMBER) / DeltaSeconds));
    const int64 MilliSteps = static_cast<int64>(BaseSteps) * Permille + FireIntervalCarryMilliSteps;
    const int32 Steps = FMath::Max<int32>(1, static_cast<int32>(MilliSteps / 1000));
    FireIntervalCarryMilliSteps = static_cast<int32>(FMath::Max<int64>(0, MilliSteps - static_cast<int64>(Steps) * 1000));
#if !UE_BUILD_SHIPPING
    UE_LOG(LogPolitikaStartup, Display,
        TEXT("RTS_DISCIPLINE_EFFECT effect=%s unit=%llu participant=%u value=%d permille=%d base_steps=%d steps=%d carry=%d step=%llu"),
        AttributeLogName(pm::Attribute::FireInterval), StableId, OwnerPlayer, ResolveMilli, Permille, BaseSteps, Steps,
        FireIntervalCarryMilliSteps, AuthorityStep());
#endif
    return static_cast<float>(Steps) * DeltaSeconds;
}
