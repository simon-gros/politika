// MP-2 unit-scale fixture (Development builds only). `-PolitikaMPUnitScale=N` gives every
// seat of a networked multiplayer-skirmish session N units of its authored archetype, so
// bandwidth and authoritative step cost can be measured at RTS unit counts (U-12). After
// activation the units hold for an idle window; then the server submits one attack-move per
// seated unit through the ordinary trusted command path. Step cost is reported per phase.
// Nothing here runs in Shipping builds or changes replicated state or the network protocol.
#include "PolitikaPrototypeGameplay.h"

#include "PolitikaBootstrap.h"
#include "PolitikaPrototypeContent.h"
#include "Metrics.h"
#include "Rules.h"
#include "UnitScale.h"

#include "HAL/PlatformTime.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "NavigationSystem.h"

#include <vector>

#if !UE_BUILD_SHIPPING
namespace
{
// Ten seconds at the 20 Hz simulation step: long enough for the idle replication state to
// settle on every client before the engagement starts.
constexpr uint64 UnitScaleIdleSteps = 200;
// Playable-area margin for scaled placements and engagement destinations.
constexpr double UnitScaleBoundsInsetCm = 100.0;
// A candidate must project onto the navmesh this close to where it was planned.
constexpr double UnitScaleProjectionToleranceCm = 25.0;

const TCHAR* PhaseName(uint8 Phase)
{
    switch (Phase)
    {
        case 1: return TEXT("idle");
        case 2: return TEXT("engage");
        default: return TEXT("off");
    }
}

struct FStepStats
{
    int32 Steps = 0;
    double Mean = 0.0;
    float P50 = 0.0f;
    float P95 = 0.0f;
    float P99 = 0.0f;
    float Max = 0.0f;
};

FStepStats Summarize(const TArray<float>& Samples, int32 Begin, int32 End)
{
    FStepStats Stats;
    Begin = FMath::Clamp(Begin, 0, Samples.Num());
    End = FMath::Clamp(End, Begin, Samples.Num());
    if (End == Begin) return Stats;
    TArray<float> Sorted(Samples.GetData() + Begin, End - Begin);
    Sorted.Sort();
    double Total = 0.0;
    for (const float Sample : Sorted) Total += Sample;
    using politika::metrics::NearestRank;
    const std::size_t Count = static_cast<std::size_t>(Sorted.Num());
    Stats.Steps = Sorted.Num();
    Stats.Mean = Total / Sorted.Num();
    Stats.P50 = NearestRank(Sorted.GetData(), Count, 50);
    Stats.P95 = NearestRank(Sorted.GetData(), Count, 95);
    Stats.P99 = NearestRank(Sorted.GetData(), Count, 99);
    Stats.Max = NearestRank(Sorted.GetData(), Count, 100);
    return Stats;
}
}

bool APolitikaPrototypeMatch::ResolveUnitScale(TArray<FPolitikaPrototypeUnitPlacement>& InOutPlacements, FName& OutFailure)
{
    using namespace politika::unitscale;
    UnitScalePerSeat = 0;
    UnitScalePhase = EUnitScalePhase::Off;
    bUnitScaleHold = false;
    FString Value;
    if (!FParse::Value(FCommandLine::Get(), TEXT("PolitikaMPUnitScale="), Value)) return true;
    // Measurement fixture only: a networked session on the multiplayer skirmish map.
    if (!IsNetworkSession() || !MapDefinition || MapDefinition->StableId != FName(TEXT("map.test.multiplayer_skirmish")))
    {
        OutFailure = TEXT("MP.UNIT_SCALE.UNSUPPORTED");
        return false;
    }
    bool bDigits = !Value.IsEmpty() && Value.Len() <= 4;
    for (const TCHAR Character : Value) bDigits &= FChar::IsDigit(Character);
    const long long PerSeat = bDigits ? FCString::Atoi64(*Value) : 0;
    if (!ValidUnitsPerSeat(PerSeat))
    {
        UE_LOG(LogPolitikaStartup, Error, TEXT("MP_UNIT_SCALE_INVALID value=%s min=%lld max=%lld"),
            *Value.Left(16), kMinUnitsPerSeat, kMaxUnitsPerSeat);
        OutFailure = TEXT("MP.UNIT_SCALE.INVALID");
        return false;
    }
    UNavigationSystemV1* NavSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
    const ANavigationData* NavData = NavSystem ? NavSystem->GetDefaultNavDataInstance(FNavigationSystem::DontCreate) : nullptr;
    if (!NavData)
    {
        OutFailure = TEXT("MP.UNIT_SCALE.NAVIGATION_UNAVAILABLE");
        return false;
    }

    struct FSeat { const FPolitikaPrototypeRuntimeParticipant* Participant; FVector Start; TArray<int32> Authored; };
    TArray<FSeat> Seats;
    for (const FPolitikaPrototypeRuntimeParticipant& Participant : RuntimeParticipants)
    {
        const FPolitikaPrototypePlayerSlot* Slot = MapDefinition->PlayerSlots.FindByPredicate(
            [&Participant](const FPolitikaPrototypePlayerSlot& Candidate) { return Candidate.SlotId == Participant.SlotId; });
        if (!Slot) continue;
        FSeat& Seat = Seats.Add_GetRef({&Participant, Slot->StartLocation, {}});
        for (int32 Index = 0; Index < InOutPlacements.Num(); ++Index)
            if (InOutPlacements[Index].SlotId == Participant.SlotId) Seat.Authored.Add(Index);
    }
    const Bounds Area{{PlayableMin.X + UnitScaleBoundsInsetCm, PlayableMin.Y + UnitScaleBoundsInsetCm},
        {PlayableMax.X - UnitScaleBoundsInsetCm, PlayableMax.Y - UnitScaleBoundsInsetCm}};
    const FVector QueryExtent(50.0f, 50.0f, 500.0f);
    TArray<FPolitikaPrototypeUnitPlacement> Added;
    // Per seat: team and spawned XY, for the fixture's own geometry summary.
    TArray<TPair<uint8, FVector2D>> Spawned;
    double MaxRadius = 0.0;
    for (const FSeat& Seat : Seats)
    {
        if (Seat.Authored.IsEmpty() || Seat.Authored.Num() > PerSeat)
        {
            OutFailure = TEXT("MP.UNIT_SCALE.PLACEMENT");
            return false;
        }
        const FPolitikaPrototypeUnitPlacement Template = InOutPlacements[Seat.Authored[0]];
        FNavLocation TemplateProjection;
        if (!NavSystem->ProjectPointToNavigation(Template.Location, TemplateProjection, QueryExtent, NavData))
        {
            OutFailure = TEXT("MP.UNIT_SCALE.PLACEMENT");
            return false;
        }
        // Scaled units stand at the authored height above the ground.
        const double HeightAboveNav = Template.Location.Z - TemplateProjection.Location.Z;
        std::vector<Point> Others;
        for (const FSeat& Other : Seats)
            if (&Other != &Seat) Others.push_back({Other.Start.X, Other.Start.Y});
        std::vector<Point> Occupied;
        for (const int32 Index : Seat.Authored)
        {
            const FVector& Location = InOutPlacements[Index].Location;
            Occupied.push_back({Location.X, Location.Y});
            Spawned.Add({Seat.Participant->TeamId, FVector2D(Location)});
        }
        const int32 Needed = static_cast<int32>(PerSeat) - Seat.Authored.Num();
        // Some cells may miss the navmesh; plan generously and take them nearest first.
        const std::vector<Point> Candidates = PlanSeatCandidates({Seat.Start.X, Seat.Start.Y}, Others, Occupied, Area,
            kSpacingCm, kClearanceCm, static_cast<std::size_t>(Needed) * 3 + 16);
        int32 Placed = 0;
        for (const Point& Candidate : Candidates)
        {
            if (Placed == Needed) break;
            FNavLocation Projected;
            const FVector Planned(Candidate.x, Candidate.y, Template.Location.Z);
            if (!NavSystem->ProjectPointToNavigation(Planned, Projected, QueryExtent, NavData)
                || FVector::Dist2D(Projected.Location, Planned) > UnitScaleProjectionToleranceCm)
                continue;
            Added.Add({Template.UnitArchetypeId, Template.SlotId,
                FVector(Candidate.x, Candidate.y, Projected.Location.Z + HeightAboveNav), Template.Rotation});
            Spawned.Add({Seat.Participant->TeamId, FVector2D(Candidate.x, Candidate.y)});
            MaxRadius = FMath::Max(MaxRadius, FVector::Dist2D(Planned, Seat.Start));
            ++Placed;
        }
        UE_LOG(LogPolitikaStartup, Display, TEXT("MP_UNIT_SCALE_SEAT participant=%u slot=%d team=%u authored=%d added=%d needed=%d candidates=%d"),
            Seat.Participant->RuntimePlayerId, Seat.Participant->SlotId, Seat.Participant->TeamId, Seat.Authored.Num(),
            Placed, Needed, static_cast<int32>(Candidates.size()));
        if (Placed != Needed)
        {
            OutFailure = TEXT("MP.UNIT_SCALE.PLACEMENT");
            return false;
        }
    }
    // Closest pair of units on opposing teams at spawn: below sight radius, the idle phase
    // already replicates enemies; below the idle acquisition range, it already fights.
    double MinOpposingGap = TNumericLimits<double>::Max();
    for (int32 A = 0; A < Spawned.Num(); ++A)
        for (int32 B = A + 1; B < Spawned.Num(); ++B)
            if (Spawned[A].Key != Spawned[B].Key)
                MinOpposingGap = FMath::Min(MinOpposingGap, FVector2D::Distance(Spawned[A].Value, Spawned[B].Value));
    UnitScalePerSeat = static_cast<int32>(PerSeat);
    bUnitScaleHold = FParse::Param(FCommandLine::Get(), TEXT("PolitikaMPUnitScaleHold"));
    UE_LOG(LogPolitikaStartup, Display,
        TEXT("MP_UNIT_SCALE_FIXTURE per_seat=%d hold=%d seats=%d seat_units=%d added=%d spacing_cm=%.0f max_radius_cm=%.1f min_opposing_gap_cm=%.1f"),
        UnitScalePerSeat, bUnitScaleHold ? 1 : 0, Seats.Num(), UnitScalePerSeat * Seats.Num(), Added.Num(), kSpacingCm, MaxRadius,
        MinOpposingGap == TNumericLimits<double>::Max() ? -1.0 : MinOpposingGap);
    // Fail closed rather than measure a fight that started during the idle window (F46).
    if (MinOpposingGap < kMinOpposingGapCm)
    {
        UE_LOG(LogPolitikaStartup, Error, TEXT("MP_UNIT_SCALE_OPPOSING_GAP min_opposing_gap_cm=%.1f required_cm=%.0f"),
            MinOpposingGap, kMinOpposingGapCm);
        UnitScalePerSeat = 0;
        OutFailure = TEXT("MP.UNIT_SCALE.OPPOSING_GAP");
        return false;
    }
    InOutPlacements.Append(Added);
    return true;
}

void APolitikaPrototypeMatch::BeginUnitScaleMeasurement()
{
    if (UnitScalePerSeat <= 0) return;
    UnitScalePhase = EUnitScalePhase::Idle;
    UnitScaleActivationStep = SimulationStepsExecuted;
    UnitScaleEngageIndex = INDEX_NONE;
    UnitScaleWindowIndex = 0;
    LastUnitScaleWindowSeconds = FPlatformTime::Seconds();
    UnitScaleFirstOrderSequence = UnitScaleLastOrderSequence = 0;
    UnitScaleOrdersIssued = UnitScaleOrdersAccepted = UnitScaleOrdersRejected = UnitScaleDestroyed = 0;
    bUnitScaleOrdersReported = false;
    UE_LOG(LogPolitikaStartup, Display, TEXT("MP_UNIT_SCALE_PHASE phase=idle generation=%llu step=%llu units=%d idle_steps=%llu"),
        MatchGeneration, SimulationStepsExecuted, Units.Num(), UnitScaleIdleSteps);
}

void APolitikaPrototypeMatch::TickUnitScaleDriver()
{
    if (UnitScalePhase == EUnitScalePhase::Off || !Lifecycle.IsActive() || bUnitScaleHold) return;
    if (UnitScalePhase == EUnitScalePhase::Idle && SimulationStepsExecuted >= UnitScaleActivationStep + UnitScaleIdleSteps)
    {
        // Each unit keeps its offset from its seat start around the objective, so seats meet
        // there from their own directions. Submitted now, admitted in the next step.
        UnitScaleEngageIndex = StepCostMicros.Num();
        const FVector Goal = Objective ? Objective->GetActorLocation() : FVector::ZeroVector;
        TMap<uint32, FVector> Starts;
        for (const FPolitikaPrototypeRuntimeParticipant& Participant : RuntimeParticipants)
            if (const FPolitikaPrototypePlayerSlot* Slot = MapDefinition->PlayerSlots.FindByPredicate(
                [&Participant](const FPolitikaPrototypePlayerSlot& Candidate) { return Candidate.SlotId == Participant.SlotId; }))
                Starts.Add(Participant.RuntimePlayerId, Slot->StartLocation);
        UnitScaleFirstOrderSequence = NextCommandSequence;
        for (APolitikaPrototypeUnit* Unit : Units)
        {
            const FVector* Start = IsValid(Unit) && Unit->IsAlive() ? Starts.Find(Unit->GetOwnerPlayer()) : nullptr;
            if (!Start) continue;
            FVector Destination = Goal + (Unit->GetActorLocation() - *Start);
            Destination.X = FMath::Clamp(Destination.X, PlayableMin.X + UnitScaleBoundsInsetCm, PlayableMax.X - UnitScaleBoundsInsetCm);
            Destination.Y = FMath::Clamp(Destination.Y, PlayableMin.Y + UnitScaleBoundsInsetCm, PlayableMax.Y - UnitScaleBoundsInsetCm);
            Destination.Z = Goal.Z;
            SubmitMove(Unit->GetOwnerPlayer(), Unit, Destination, true);
            ++UnitScaleOrdersIssued;
        }
        UnitScaleLastOrderSequence = NextCommandSequence - 1;
        UnitScalePhase = EUnitScalePhase::Engage;
        UE_LOG(LogPolitikaStartup, Display,
            TEXT("MP_UNIT_SCALE_PHASE phase=engage generation=%llu step=%llu units=%d issued=%d first_sequence=%llu last_sequence=%llu"),
            MatchGeneration, SimulationStepsExecuted, Units.Num(), UnitScaleOrdersIssued, UnitScaleFirstOrderSequence,
            UnitScaleLastOrderSequence);
    }
    if (UnitScaleOrdersIssued > 0 && !bUnitScaleOrdersReported
        && UnitScaleOrdersAccepted + UnitScaleOrdersRejected >= UnitScaleOrdersIssued)
    {
        bUnitScaleOrdersReported = true;
        UE_LOG(LogPolitikaStartup, Display, TEXT("MP_UNIT_SCALE_ORDERS generation=%llu step=%llu issued=%d accepted=%d rejected=%d"),
            MatchGeneration, SimulationStepsExecuted, UnitScaleOrdersIssued, UnitScaleOrdersAccepted, UnitScaleOrdersRejected);
    }
    const double Now = FPlatformTime::Seconds();
    if (Now < LastUnitScaleWindowSeconds + 1.0) return;
    LastUnitScaleWindowSeconds = Now;
    const FStepStats Window = Summarize(StepCostMicros, UnitScaleWindowIndex, StepCostMicros.Num());
    UnitScaleWindowIndex = StepCostMicros.Num();
    int32 Alive = 0;
    for (const APolitikaPrototypeUnit* Unit : Units) Alive += IsValid(Unit) && Unit->IsAlive() ? 1 : 0;
    UE_LOG(LogPolitikaStartup, Display,
        TEXT("MP_UNIT_SCALE_WINDOW phase=%s generation=%llu step=%llu elapsed_s=%.3f steps=%d mean_us=%.1f max_us=%.1f alive=%d destroyed=%d"),
        PhaseName(static_cast<uint8>(UnitScalePhase)), MatchGeneration, SimulationStepsExecuted, Now - MeasurementStartSeconds,
        Window.Steps, Window.Mean, Window.Max, Alive, UnitScaleDestroyed);
}

void APolitikaPrototypeMatch::ObserveUnitScaleEvent(const politika::rules::GameplayEvent& Event)
{
    using namespace politika::rules;
    if (UnitScalePhase == EUnitScalePhase::Off) return;
    if (const auto* Outcome = std::get_if<CommandOutcomeEvent>(&Event))
    {
        if (UnitScaleLastOrderSequence != 0 && Outcome->sequence >= UnitScaleFirstOrderSequence
            && Outcome->sequence <= UnitScaleLastOrderSequence)
            ++(Outcome->accepted ? UnitScaleOrdersAccepted : UnitScaleOrdersRejected);
    }
    else if (std::holds_alternative<UnitDestroyedEvent>(Event))
        ++UnitScaleDestroyed;
}

void APolitikaPrototypeMatch::ReportUnitScaleMeasurements(const TCHAR* Reason)
{
    if (UnitScalePerSeat <= 0 || UnitScalePhase == EUnitScalePhase::Off) return;
    const int32 Engage = UnitScaleEngageIndex == INDEX_NONE ? StepCostMicros.Num() : UnitScaleEngageIndex;
    int32 Alive = 0;
    for (const APolitikaPrototypeUnit* Unit : Units) Alive += IsValid(Unit) && Unit->IsAlive() ? 1 : 0;
    for (const uint8 Phase : {uint8{1}, uint8{2}})
    {
        const FStepStats Stats = Phase == 1 ? Summarize(StepCostMicros, 0, Engage)
            : Summarize(StepCostMicros, Engage, StepCostMicros.Num());
        UE_LOG(LogPolitikaStartup, Display,
            TEXT("MP_UNIT_SCALE_STEP_SUMMARY reason=%s phase=%s generation=%llu per_seat=%d units=%d alive=%d destroyed=%d steps=%d mean_us=%.1f p50_us=%.1f p95_us=%.1f p99_us=%.1f max_us=%.1f budget_us=%.0f"),
            Reason, PhaseName(Phase), MatchGeneration, UnitScalePerSeat, Units.Num(), Alive, UnitScaleDestroyed, Stats.Steps,
            Stats.Mean, Stats.P50, Stats.P95, Stats.P99, Stats.Max, SimulationStepSeconds * 1.0e6);
    }
    // The step that admitted every engagement order (route queries for all units at once).
    UE_LOG(LogPolitikaStartup, Display,
        TEXT("MP_UNIT_SCALE_ADMISSION generation=%llu issued=%d accepted=%d rejected=%d admission_step_us=%.1f"),
        MatchGeneration, UnitScaleOrdersIssued, UnitScaleOrdersAccepted, UnitScaleOrdersRejected,
        StepCostMicros.IsValidIndex(UnitScaleEngageIndex) ? StepCostMicros[UnitScaleEngageIndex] : -1.0f);
    UnitScalePhase = EUnitScalePhase::Off;
}
#endif
