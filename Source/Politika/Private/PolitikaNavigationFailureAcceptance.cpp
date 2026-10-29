#include "PolitikaPrototypeGameplay.h"
#include "PolitikaBootstrap.h"

#if !UE_BUILD_SHIPPING
#include "PolitikaNavigationFailureFixture.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "NavigationSystem.h"
#include "NavigationData.h"
#include "HAL/PlatformMisc.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"

void APolitikaPrototypeMatch::BeginNavigationFailureAcceptance()
{
    auto& Trace = NavigationFailureAcceptance;
    FString Mode;
    if (!FParse::Value(FCommandLine::Get(), TEXT("PolitikaNavigationFailure="), Mode)
        || (Mode != TEXT("Disconnected") && Mode != TEXT("UnprojectableDestination"))
        || !MapDefinition || !Lifecycle.IsActive())
    {
        FinishNavigationFailureAcceptance(TEXT("invalid_selector_or_match"));
        return;
    }
    const bool bProjectionCase = Mode == TEXT("UnprojectableDestination");
    const TCHAR* BarrierTag = bProjectionCase ? PolitikaUnprojectableDestinationFixture::ExclusionTag
        : PolitikaNavigationFailureFixture::BarrierTag;
    Trace.bDestinationProjectionCase = bProjectionCase;
    Trace.Unit = Units.IsEmpty() ? nullptr : Units[0];
    Trace.Start = bProjectionCase ? PolitikaUnprojectableDestinationFixture::Start : PolitikaNavigationFailureFixture::Start;
    Trace.Goal = bProjectionCase ? PolitikaUnprojectableDestinationFixture::Goal : PolitikaNavigationFailureFixture::Goal;
    const FVector Start = Trace.Start;
    const FVector Goal = Trace.Goal;
    APolitikaPrototypeUnit* Unit = Trace.Unit.Get();
    UNavigationSystemV1* Nav = GetWorld() ? FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld()) : nullptr;
    ANavigationData* Data = Nav ? Nav->GetDefaultNavDataInstance(FNavigationSystem::DontCreate) : nullptr;
    FNavLocation ProjectedStart, ProjectedGoal;
    const bool bStartProjects = Nav && Data && !Nav->IsNavigationBuildInProgress()
        && Nav->ProjectPointToNavigation(Start, ProjectedStart, FVector(80, 80, 180), Data)
        && FVector::Dist(ProjectedStart.Location, Start) <= 80.0f;
    const bool bGoalProjectionSucceeded = Nav && Data && !Nav->IsNavigationBuildInProgress()
        && Nav->ProjectPointToNavigation(Goal, ProjectedGoal, FVector(80, 80, 180), Data);
    const bool bGoalProjects = bGoalProjectionSucceeded
        && FVector::Dist(ProjectedGoal.Location, Goal) <= 80.0f;
    AActor* Barrier = nullptr;
    int32 Barriers = 0;
    for (TActorIterator<AActor> It(GetWorld()); It; ++It)
        if (It->ActorHasTag(BarrierTag)) { Barrier = *It; ++Barriers; }
    const FBox BarrierBounds = Barrier ? Barrier->GetComponentsBoundingBox(true) : FBox(ForceInit);
    FHitResult GroundHit;
    FCollisionQueryParams GroundQuery(SCENE_QUERY_STAT(NavigationFailureGround), true);
    if (Barrier) GroundQuery.AddIgnoredActor(Barrier);
    const bool bGroundHit = GetWorld()->LineTraceSingleByChannel(GroundHit,
        Goal + FVector(0, 0, 150.0f), Goal - FVector(0, 0, 150.0f), ECC_Visibility, GroundQuery)
        && GroundHit.GetActor()
        && (GroundHit.GetActor()->ActorHasTag(TEXT("PolitikaPrototypeGround"))
            || GroundHit.GetActor()->GetClass()->GetName().StartsWith(TEXT("Landscape")))
        && FVector::Dist(GroundHit.ImpactPoint, Goal) <= 5.0f;
    const bool bInBounds = Start.X >= PlayableMin.X && Start.X <= PlayableMax.X
        && Start.Y >= PlayableMin.Y && Start.Y <= PlayableMax.Y
        && Goal.X >= PlayableMin.X && Goal.X <= PlayableMax.X
        && Goal.Y >= PlayableMin.Y && Goal.Y <= PlayableMax.Y;
    const bool bProfile = MapDefinition->MovementProfileId == TEXT("movement.prototype_ground");
    const bool bCleanUnit = Unit && Unit->IsAlive() && Unit->GetOwnerPlayer() == LocalRuntimePlayerId
        && Unit->GetActorLocation().Equals(Start, 0.01)
        && Unit->NavigationWaypoints.IsEmpty() && Unit->NavigationWaypointIndex == 0
        && Unit->NavigationOrderSequence == 0 && Unit->NavigationMatchGeneration == 0;
    const bool bGeometryValid = bProjectionCase
        ? Barriers == 1 && Barrier && bGroundHit
            && Goal.X >= BarrierBounds.Min.X && Goal.X <= BarrierBounds.Max.X
            && Goal.Y >= BarrierBounds.Min.Y && Goal.Y <= BarrierBounds.Max.Y
        : Barrier && Barriers == 1 && Barrier->GetActorEnableCollision()
            && Cast<AStaticMeshActor>(Barrier)
            && Cast<AStaticMeshActor>(Barrier)->GetStaticMeshComponent()->CanEverAffectNavigation()
            && BarrierBounds.Min.Y < PlayableMin.Y && BarrierBounds.Max.Y > PlayableMax.Y;
    Trace.bValid = bStartProjects && (bProjectionCase ? !bGoalProjectionSucceeded : bGoalProjects)
        && bInBounds && bProfile && bCleanUnit && bGeometryValid && Lifecycle.IsActive() && Data;
    if (!Trace.bValid)
    {
        FinishNavigationFailureAcceptance(TEXT("fixture_projection_authority_or_bounds_invalid"));
        return;
    }
    Trace.bRunning = true;
    Trace.Sequence = NextCommandSequence;
    if (bProjectionCase)
    {
        UE_LOG(LogPolitikaStartup, Display, TEXT("NAV_FAILURE_FIXTURE scenario=UnprojectableDestination map=%s level=%s generation=%llu unit=%llu owner=%u profile=%s nav=%s start=%.6f,%.6f,%.6f goal=%.6f,%.6f,%.6f origin_projected=1 goal_projected=0 extent=80.000000,80.000000,180.000000 bounds_min=%.6f,%.6f bounds_max=%.6f,%.6f exclusion=%s exclusion_min=%.6f,%.6f,%.6f exclusion_max=%.6f,%.6f,%.6f runtime_generation=%d"),
            *MapDefinition->StableId.ToString(), *GetWorld()->GetPackage()->GetName(), MatchGeneration,
            Unit->GetStableId(), Unit->GetOwnerPlayer(), *MapDefinition->MovementProfileId.ToString(), *Data->GetPathName(),
            Start.X, Start.Y, Start.Z, Goal.X, Goal.Y, Goal.Z, PlayableMin.X, PlayableMin.Y,
            PlayableMax.X, PlayableMax.Y, *Barrier->GetPathName(), BarrierBounds.Min.X,
            BarrierBounds.Min.Y, BarrierBounds.Min.Z, BarrierBounds.Max.X, BarrierBounds.Max.Y,
            BarrierBounds.Max.Z, static_cast<int32>(Data->GetRuntimeGenerationMode()));
        UE_LOG(LogPolitikaStartup, Display, TEXT("NAV_FAILURE_GROUND goal=%.6f,%.6f,%.6f ground_hit=%d ground_actor=%s bounds_valid=1 nav_excluded=1 exclusion=%s"), Goal.X, Goal.Y, Goal.Z, bGroundHit ? 1 : 0, GroundHit.GetActor() ? *GroundHit.GetActor()->GetPathName() : TEXT("none"), *Barrier->GetPathName());
        SubmitMove(LocalRuntimePlayerId, Unit, Goal);
        UE_LOG(LogPolitikaStartup, Display, TEXT("NAV_FAILURE_SUBMITTED generation=%llu unit=%llu sequence=%llu participant=%u boundary_step=%llu"), MatchGeneration, Unit->GetStableId(), Trace.Sequence, LocalRuntimePlayerId, SimulationStepsExecuted);
        return;
    }
    UE_LOG(LogPolitikaStartup, Display,
        TEXT("NAV_FAILURE_FIXTURE scenario=Disconnected map=%s level=%s generation=%llu unit=%llu owner=%u profile=%s nav=%s start=%.6f,%.6f,%.6f goal=%.6f,%.6f,%.6f projected_start=%.6f,%.6f,%.6f projected_goal=%.6f,%.6f,%.6f start_offset=%.6f goal_offset=%.6f bounds_min=%.6f,%.6f bounds_max=%.6f,%.6f barrier=%s barrier_min=%.6f,%.6f,%.6f barrier_max=%.6f,%.6f,%.6f runtime_generation=%d"),
        *MapDefinition->StableId.ToString(), *GetWorld()->GetPackage()->GetName(), MatchGeneration,
        Unit->GetStableId(), Unit->GetOwnerPlayer(), *MapDefinition->MovementProfileId.ToString(), *Data->GetPathName(),
        Start.X, Start.Y, Start.Z, Goal.X, Goal.Y, Goal.Z,
        ProjectedStart.Location.X, ProjectedStart.Location.Y, ProjectedStart.Location.Z,
        ProjectedGoal.Location.X, ProjectedGoal.Location.Y, ProjectedGoal.Location.Z,
        FVector::Distance(ProjectedStart.Location, Start), FVector::Distance(ProjectedGoal.Location, Goal),
        PlayableMin.X, PlayableMin.Y, PlayableMax.X, PlayableMax.Y, *Barrier->GetPathName(),
        BarrierBounds.Min.X, BarrierBounds.Min.Y, BarrierBounds.Min.Z,
        BarrierBounds.Max.X, BarrierBounds.Max.Y, BarrierBounds.Max.Z,
        static_cast<int32>(Data->GetRuntimeGenerationMode()));
    FPathFindingQuery PartialQuery(Unit, *Data, ProjectedStart.Location, ProjectedGoal.Location);
    PartialQuery.SetAllowPartialPaths(true);
    PartialQuery.SetRequireNavigableEndLocation(true);
    const FPathFindingResult PartialResult = Nav->FindPathSync(PartialQuery);
    FString PartialRejection;
    const bool bPartialResultAdmitted = IsCompleteNavigationResult(PartialResult, PartialRejection);
    const FVector PartialEndpoint = PartialResult.Path.IsValid() && !PartialResult.Path->GetPathPoints().IsEmpty()
        ? PartialResult.Path->GetPathPoints().Last().Location : FVector::ZeroVector;
    Trace.DiagnosticResult = static_cast<uint32>(PartialResult.Result);
    Trace.bDiagnosticPartial = PartialResult.IsPartial();
    Trace.bDiagnosticPathValid = PartialResult.Path.IsValid();
    Trace.DiagnosticPathPoints = PartialResult.Path.IsValid() ? PartialResult.Path->GetPathPoints().Num() : 0;
    Trace.DiagnosticEndpointError = PartialResult.Path.IsValid()
        ? FVector::Distance(PartialEndpoint, ProjectedGoal.Location) : 0.0f;
    Trace.DiagnosticRejection = PartialRejection;
    UE_LOG(LogPolitikaStartup, Display,
        TEXT("NAV_FAILURE_PARTIAL_PROBE generation=%llu unit=%llu profile=%s nav=%s allow_partial=1 require_navigable_end=1 result=%u partial=%d path_valid=%d points=%d endpoint=%.6f,%.6f,%.6f endpoint_error=%.6f route_admission=%s rejection=%s installed=0"),
        MatchGeneration, Unit->GetStableId(), *MapDefinition->MovementProfileId.ToString(), *Data->GetPathName(),
        Trace.DiagnosticResult, Trace.bDiagnosticPartial ? 1 : 0, Trace.bDiagnosticPathValid ? 1 : 0,
        Trace.DiagnosticPathPoints, PartialEndpoint.X, PartialEndpoint.Y, PartialEndpoint.Z,
        Trace.DiagnosticEndpointError, bPartialResultAdmitted ? TEXT("ACCEPTED") : TEXT("REJECTED"),
        *PartialRejection);
    SubmitMove(LocalRuntimePlayerId, Unit, Goal);
    UE_LOG(LogPolitikaStartup, Display,
        TEXT("NAV_FAILURE_SUBMITTED generation=%llu unit=%llu sequence=%llu participant=%u boundary_step=%llu"),
        MatchGeneration, Unit->GetStableId(), Trace.Sequence, LocalRuntimePlayerId, SimulationStepsExecuted);
}

void APolitikaPrototypeMatch::ObserveNavigationFailureEvent(const politika::rules::GameplayEvent& Event)
{
    auto& Trace = NavigationFailureAcceptance;
    APolitikaPrototypeUnit* Unit = Trace.Unit.Get();
    if (!Trace.bRunning || !Unit) return;
    using namespace politika::rules;
    if (const auto* Command = std::get_if<CommandOutcomeEvent>(&Event);
        Command && Command->issuer_id == Unit->GetStableId())
    {
        const bool bExpected = Command->sequence == Trace.Sequence && Command->correlation_id == Trace.Sequence
            && Command->order == OrderKind::Move && Command->accepted
            && Command->rejection == OrderRejection::None;
        Trace.bValid &= bExpected;
        if (bExpected) ++Trace.AcceptedCommands;
        UE_LOG(LogPolitikaStartup, Display,
            TEXT("NAV_FAILURE_EVENT kind=CommandOutcome generation=%llu unit=%llu sequence=%llu correlation=%llu step=%llu order=%u accepted=%d rejection=%u valid=%d"),
            MatchGeneration, Unit->GetStableId(), Command->sequence, Command->correlation_id, Command->step,
            static_cast<uint32>(Command->order), Command->accepted ? 1 : 0,
            static_cast<uint32>(Command->rejection), bExpected ? 1 : 0);
    }
    if (const auto* Changed = std::get_if<OrderChangedEvent>(&Event);
        Changed && Changed->entity_id == Unit->GetStableId())
    {
        const bool bFailure = Changed->previous_end == OrderEndReason::NavigationFailed;
        const bool bArrival = Changed->previous_end == OrderEndReason::Arrived;
        Trace.bValid &= bFailure || !bArrival;
        if (bFailure) { ++Trace.FailureEvents; Trace.FailureStep = Changed->step; }
        if (bArrival) ++Trace.Arrivals;
        UE_LOG(LogPolitikaStartup, Display,
            TEXT("NAV_FAILURE_EVENT kind=OrderChanged generation=%llu unit=%llu step=%llu previous=%u current=%u reason=%u failure=%d arrival=%d"),
            MatchGeneration, Unit->GetStableId(), Changed->step, static_cast<uint32>(Changed->previous),
            static_cast<uint32>(Changed->current), static_cast<uint32>(Changed->previous_end),
            bFailure ? 1 : 0, bArrival ? 1 : 0);
    }
}

void APolitikaPrototypeMatch::AfterNavigationFailureStep()
{
    auto& Trace = NavigationFailureAcceptance;
    if (!Trace.bRunning || !Trace.FailureStep || SimulationStepsExecuted < Trace.FailureStep) return;
    APolitikaPrototypeUnit* Unit = Trace.Unit.Get();
    if (!Unit) { FinishNavigationFailureAcceptance(TEXT("unit_lost")); return; }
    const FVector Position = Unit->GetActorLocation();
    const bool bInert = Position.Equals(Trace.Start, 0.001)
        && Unit->IsAlive() && Unit->GetOwnerPlayer() == LocalRuntimePlayerId
        && Unit->GetCurrentOrder() == EPolitikaPrototypeOrder::Stop
        && Unit->NavigationWaypoints.IsEmpty() && Unit->NavigationWaypointIndex == 0
        && Unit->NavigationOrderSequence == 0 && Unit->NavigationMatchGeneration == 0
        && Lifecycle.IsActive() && PendingOrders.IsEmpty() && Trace.Arrivals == 0;
    Trace.bValid &= bInert;
    ++Trace.Samples;
    UE_LOG(LogPolitikaStartup, Display,
        TEXT("NAV_FAILURE_STEP step=%llu generation=%llu unit=%llu position=%.6f,%.6f,%.6f order=%u points=%d cursor=%d route_sequence=%llu route_generation=%llu alive=%d owner=%u active=%d pending=%d arrivals=%d valid=%d"),
        SimulationStepsExecuted, MatchGeneration, Unit->GetStableId(), Position.X, Position.Y, Position.Z,
        static_cast<uint32>(Unit->GetCurrentOrder()), Unit->NavigationWaypoints.Num(), Unit->NavigationWaypointIndex,
        Unit->NavigationOrderSequence, Unit->NavigationMatchGeneration, Unit->IsAlive() ? 1 : 0,
        Unit->GetOwnerPlayer(), Lifecycle.IsActive() ? 1 : 0, PendingOrders.Num(), Trace.Arrivals, bInert ? 1 : 0);
    if (!Trace.bValid) { FinishNavigationFailureAcceptance(TEXT("post_failure_state_changed")); return; }
    if (Trace.Samples >= 21) FinishNavigationFailureAcceptance(nullptr);
}

void APolitikaPrototypeMatch::FinishNavigationFailureAcceptance(const TCHAR* Failure)
{
    auto& Trace = NavigationFailureAcceptance;
    APolitikaPrototypeUnit* Unit = Trace.Unit.Get();
    const bool bQueryFailedClosed = Trace.Queries == 1 && Trace.LastQueryResult != 3
        && (!Trace.bLastQueryPartial || Trace.LastPathPoints > 0 || !Trace.bLastPathValid);
    const bool bProjectionFailedClosed = Trace.ProjectionChecks == 2 && Trace.bOriginProjected
        && !Trace.bGoalProjected && Trace.Queries == 0 && Trace.LastFailure == TEXT("DestinationNotNavigable");
    const bool bPass = !Failure && Trace.bValid && Unit
        && (Trace.bDestinationProjectionCase ? bProjectionFailedClosed : bQueryFailedClosed)
        && Trace.AcceptedCommands == 1 && Trace.FailureEvents == 1 && Trace.Arrivals == 0
        && Trace.Samples == 21 && Trace.FailureStep && SimulationStepsExecuted >= Trace.FailureStep + 20
        && Unit->GetCurrentOrder() == EPolitikaPrototypeOrder::Stop
        && Unit->NavigationWaypoints.IsEmpty() && Unit->NavigationWaypointIndex == 0
        && Unit->NavigationOrderSequence == 0 && Unit->NavigationMatchGeneration == 0
        && Unit->GetActorLocation().Equals(Trace.Start, 0.001) && Lifecycle.IsActive();
    UE_LOG(LogPolitikaStartup, Display,
        TEXT("NAV_FAILURE_RESULT scenario=%s result=%s reason=%s generation=%llu unit=%llu sequence=%llu projection_checks=%d origin_projected=%d goal_projected=%d queries=%d query_result=%s partial=%s path_valid=%s points=%s failure_classification=%s accepted_commands=%d failure_events=%d arrivals=%d failure_step=%llu final_step=%llu inert_steps=%d retired=%d partial_probe_result=%s partial_probe_partial=%s partial_probe_path_valid=%s partial_probe_points=%s partial_probe_endpoint_error=%s partial_probe_rejection=%s partial_probe_installed=0"),
        Trace.bDestinationProjectionCase ? TEXT("UnprojectableDestination") : TEXT("Disconnected"),
        bPass ? TEXT("PASS") : TEXT("FAIL"), Failure ? Failure : (bPass ? TEXT("none") : TEXT("final_invariant")),
        MatchGeneration, Unit ? Unit->GetStableId() : 0, Trace.Sequence, Trace.ProjectionChecks,
        Trace.bOriginProjected ? 1 : 0, Trace.bGoalProjected ? 1 : 0, Trace.Queries,
        Trace.bDestinationProjectionCase ? TEXT("NA") : *LexToString(Trace.LastQueryResult),
        Trace.bDestinationProjectionCase ? TEXT("NA") : (Trace.bLastQueryPartial ? TEXT("1") : TEXT("0")),
        Trace.bDestinationProjectionCase ? TEXT("NA") : (Trace.bLastPathValid ? TEXT("1") : TEXT("0")),
        Trace.bDestinationProjectionCase ? TEXT("NA") : *LexToString(Trace.LastPathPoints),
        Trace.bDestinationProjectionCase ? *Trace.LastFailure : TEXT("NoCompletePath"),
        Trace.AcceptedCommands, Trace.FailureEvents, Trace.Arrivals,
        Trace.FailureStep, SimulationStepsExecuted, Trace.Samples,
        Unit && Unit->NavigationWaypoints.IsEmpty() && Unit->NavigationOrderSequence == 0 ? 1 : 0,
        Trace.bDestinationProjectionCase ? TEXT("NA") : *LexToString(Trace.DiagnosticResult),
        Trace.bDestinationProjectionCase ? TEXT("NA") : (Trace.bDiagnosticPartial ? TEXT("1") : TEXT("0")),
        Trace.bDestinationProjectionCase ? TEXT("NA") : (Trace.bDiagnosticPathValid ? TEXT("1") : TEXT("0")),
        Trace.bDestinationProjectionCase ? TEXT("NA") : *LexToString(Trace.DiagnosticPathPoints),
        Trace.bDestinationProjectionCase ? TEXT("NA") : *LexToString(Trace.DiagnosticEndpointError),
        Trace.bDestinationProjectionCase ? TEXT("NA") : *Trace.DiagnosticRejection);
    Trace.bRunning = false;
    FPlatformMisc::RequestExit(false);
}
#endif
