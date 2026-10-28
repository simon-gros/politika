#include "PolitikaPrototypeGameplay.h"

#if !UE_BUILD_SHIPPING
#include "PolitikaNavigationObstacleFixture.h"
#include "PolitikaBootstrap.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "NavigationSystem.h"
#include "NavigationData.h"
#include "PhysicsEngine/BodySetup.h"
#include "HAL/PlatformMisc.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"

namespace
{
bool BodySweepHitsWall(UWorld* World, UStaticMeshComponent* Body, AActor* Wall, const FVector& Start, const FVector& End)
{
    FComponentQueryParams Params(SCENE_QUERY_STAT(NavigationAcceptanceBodySweep));
    for (TActorIterator<AActor> It(World); It; ++It)
        if (*It != Wall) Params.AddIgnoredActor(*It);
    TArray<FHitResult> Hits;
    World->ComponentSweepMulti(Hits, Body, Start, End, Body->GetComponentQuat(), Params);
    return Hits.ContainsByPredicate([Wall](const FHitResult& Hit) { return Hit.bBlockingHit && Hit.GetActor() == Wall; });
}
}

void APolitikaPrototypeMatch::BeginNavigationAcceptance()
{
    using namespace PolitikaNavigationObstacleFixture;
    auto& Trace = NavigationAcceptance;
    Trace.bRunning = true;
    FParse::Value(FCommandLine::Get(), TEXT("PolitikaNavigationInterruption="), Trace.Interruption);
    if (!Trace.Interruption.IsEmpty() && Trace.Interruption != TEXT("OI-01")
        && Trace.Interruption != TEXT("OI-02") && Trace.Interruption != TEXT("OI-03"))
    {
        FinishNavigationAcceptance(TEXT("invalid_interruption"));
        return;
    }
    FString ScheduleName;
    if (FParse::Value(FCommandLine::Get(), TEXT("PolitikaNavigationSchedule="), ScheduleName))
    {
        for (int32 Index = 1; Index <= 5; ++Index)
            if (ScheduleName == FString::Printf(TEXT("RS-%02d"), Index)) Trace.Schedule = Index;
        if (!Trace.Schedule || SimulationStepsExecuted != 0 || StepAccumulator != 0.0f)
        {
            FinishNavigationAcceptance(TEXT("invalid_schedule_or_initial_step"));
            return;
        }
        UE_LOG(LogPolitikaStartup, Display, TEXT("NAV_REPEATABILITY_BEGIN schedule=%s step=0 accumulator=0 clamp=0.25 cap=5 dt=0.05"), *ScheduleName);
    }
    Trace.Unit = Units.IsEmpty() ? nullptr : Units[0];
    APolitikaPrototypeUnit* Unit = Trace.Unit.Get();
    AStaticMeshActor* Wall = nullptr;
    int32 WallCount = 0;
    for (TActorIterator<AStaticMeshActor> It(GetWorld()); It; ++It)
        if (It->ActorHasTag(WallTag)) { Wall = *It; ++WallCount; }
    if (!Unit || !Wall || WallCount != 1 || !Lifecycle.IsActive())
    {
        FinishNavigationAcceptance(TEXT("fixture_or_authority_missing"));
        return;
    }
    Trace.Wall = Wall;
    UStaticMeshComponent* Component = Wall->GetStaticMeshComponent();
    Trace.Blocker = Component->Bounds.GetBox();
    Trace.Extent = Unit->Body->Bounds.BoxExtent;
    const UBodySetup* BodySetup = Unit->Body->GetBodySetup();
    if (!BodySetup || BodySetup->AggGeom.SphereElems.Num() != 1 || BodySetup->AggGeom.GetElementCount() != 1
        || !BodySetup->AggGeom.SphereElems[0].Center.IsNearlyZero())
    {
        FinishNavigationAcceptance(TEXT("unsupported_fixture_collision_shape"));
        return;
    }
    Trace.CollisionRadius = BodySetup->AggGeom.SphereElems[0].Radius * Unit->GetActorScale3D().GetAbsMin();
    Trace.Previous = Unit->GetActorLocation();
    Unit->bTraceNavigationTraversal = true;
    FHitResult Hit;
    const bool bHit = Component->LineTraceComponent(Hit, Start, Goal,
        FCollisionQueryParams(SCENE_QUERY_STAT(NavigationAcceptanceDirect), false));
    UNavigationSystemV1* Nav = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
    ANavigationData* Data = Nav ? Nav->GetDefaultNavDataInstance(FNavigationSystem::DontCreate) : nullptr;
    FNavLocation BlockedProjection;
    const bool bBlockedFootprintExcluded = Nav && Data
        && !Nav->ProjectPointToNavigation(FVector(0, -600, 42), BlockedProjection, FVector(5, 5, 80), Data);
    const bool bCollision = Component->GetCollisionEnabled() != ECollisionEnabled::NoCollision
        && Component->GetCollisionResponseToChannel(ECC_WorldDynamic) == ECR_Block
        && Component->CanEverAffectNavigation() && Wall->GetActorEnableCollision()
        && Unit->Body->GetCollisionEnabled() != ECollisionEnabled::NoCollision
        && Unit->Body->GetCollisionResponseToChannel(ECC_WorldStatic) == ECR_Block
        && Unit->GetActorEnableCollision();
    UE_LOG(LogPolitikaStartup, Display,
        TEXT("NAV_ACCEPTANCE_FIXTURE scenario=StaticObstacleDetour map=%s level=%s generation=%llu unit=%llu owner=%u speed=%.6f dt=0.050000 start=%.6f,%.6f,%.6f goal=%.6f,%.6f,%.6f extent=%.6f,%.6f,%.6f radius=%.6f wall=%s min=%.6f,%.6f,%.6f max=%.6f,%.6f,%.6f nav=%s runtime_generation=%d agent_radius=%.3f"),
        *MapDefinition->StableId.ToString(), *GetWorld()->GetPackage()->GetName(), MatchGeneration,
        Unit->GetStableId(), Unit->GetOwnerPlayer(), Unit->MoveSpeed,
        Start.X, Start.Y, Start.Z, Goal.X, Goal.Y, Goal.Z, Trace.Extent.X, Trace.Extent.Y, Trace.Extent.Z, Trace.CollisionRadius,
        *Wall->GetPathName(), Trace.Blocker.Min.X, Trace.Blocker.Min.Y, Trace.Blocker.Min.Z,
        Trace.Blocker.Max.X, Trace.Blocker.Max.Y, Trace.Blocker.Max.Z,
        Data ? *Data->GetPathName() : TEXT("missing"), Data ? static_cast<int32>(Data->GetRuntimeGenerationMode()) : -1,
        Data ? Data->GetConfig().AgentRadius : 0.0f);
    UE_LOG(LogPolitikaStartup, Display,
        TEXT("NAV_ACCEPTANCE_DIRECT_BLOCKED hit=%d actor=%s collision=%d nav_excluded=%d distance=%.6f"),
        bHit ? 1 : 0, Hit.GetActor() ? *Hit.GetActor()->GetPathName() : TEXT("none"),
        bCollision ? 1 : 0, bBlockedFootprintExcluded ? 1 : 0, FVector::Distance(Start, Goal));
    // The real authored collision body must also hit the intended cooked wall.
    Trace.bValid = bHit && Hit.GetActor() == Wall && bCollision && bBlockedFootprintExcluded
        && Unit->GetActorLocation().Equals(Start, 0.01) && Unit->GetOwnerPlayer() == LocalRuntimePlayerId
        && Wall->GetActorRotation().IsNearlyZero() && Wall->GetActorLocation().Equals(WallLocation, 0.01)
        && Wall->GetActorScale3D().Equals(WallScale, 0.01) && Data && Nav && !Nav->IsNavigationBuildInProgress()
        && BodySweepHitsWall(GetWorld(), Unit->Body, Wall, Start, Goal);
    if (!Trace.bValid) { FinishNavigationAcceptance(TEXT("direct_blocker_or_fixture_invalid")); return; }
    Trace.Sequence = NextCommandSequence;
    SubmitMove(LocalRuntimePlayerId, Unit, Goal); // Normal typed admission; no fabricated waypoints.
    UE_LOG(LogPolitikaStartup, Display, TEXT("NAV_ACCEPTANCE_SUBMITTED generation=%llu unit=%llu sequence=%llu participant=%u boundary_step=%llu"),
        MatchGeneration, Unit->GetStableId(), Trace.Sequence, LocalRuntimePlayerId, SimulationStepsExecuted);
}

void APolitikaPrototypeMatch::BeforeNavigationAcceptanceStep()
{
    auto& Trace = NavigationAcceptance;
    if (!Trace.bRunning) return;
    APolitikaPrototypeUnit* Unit = Trace.Unit.Get();
    if (!Unit) { FinishNavigationAcceptance(TEXT("unit_lost")); return; }
    Trace.bValid &= Unit->GetActorLocation().Equals(Trace.Previous, 0.001);
    Unit->NavigationStepTraversal.Reset();
    if (!Trace.Interruption.IsEmpty() && (SimulationStepsExecuted == 41 || SimulationStepsExecuted == 61))
        UE_LOG(LogPolitikaStartup, Display, TEXT("NAV_INTERRUPTION_ADMITTED step=%llu sequence=%llu nav_generation=%llu points=%d cursor=%d order=%d position=%.6f,%.6f,%.6f"),
            SimulationStepsExecuted, Unit->NavigationOrderSequence, Unit->NavigationMatchGeneration,
            Unit->NavigationWaypoints.Num(), Unit->NavigationWaypointIndex, static_cast<int32>(Unit->CurrentOrder),
            Trace.Previous.X, Trace.Previous.Y, Trace.Previous.Z);
    if (Trace.bRouteRecorded && !(Trace.Interruption == TEXT("OI-02") && SimulationStepsExecuted == 41)) return;
    Trace.Route = Unit->NavigationWaypoints;
    Trace.Resolved = Unit->ResolvedNavigationDestination;
    Trace.bRouteRecorded = true;
    double Length = 0;
    bool bClear = Trace.Route.Num() >= 2;
    for (int32 Index = 1; Index < Trace.Route.Num(); ++Index)
    {
        Length += FVector::Distance(Trace.Route[Index - 1], Trace.Route[Index]);
        bClear &= !BodySweepHitsWall(GetWorld(), Unit->Body, Trace.Wall.Get(), Trace.Route[Index - 1], Trace.Route[Index]);
    }
    const bool bReplacement = Trace.Interruption == TEXT("OI-02") && SimulationStepsExecuted == 41;
    const FVector Goal = bReplacement ? PolitikaNavigationObstacleFixture::Start : PolitikaNavigationObstacleFixture::Goal;
    const uint64 Sequence = bReplacement ? Trace.FollowupSequence : Trace.Sequence;
    const double Direct = FVector::Distance(Trace.Previous, Goal);
    Trace.bValid &= bClear && (bReplacement || Length > Direct + 0.1) && Unit->CurrentOrder == EPolitikaPrototypeOrder::Move
        && Unit->NavigationOrderSequence == Sequence && Unit->NavigationMatchGeneration == MatchGeneration
        && FVector::Distance(Trace.Resolved, Goal) <= 150.0;
    UE_LOG(LogPolitikaStartup, Display,
        TEXT("NAV_ACCEPTANCE_PATH_COMPLETE sequence=%llu points=%d length=%.6f direct=%.6f body_clear=%d valid=%d"),
        Sequence, Trace.Route.Num(), Length, Direct, bClear ? 1 : 0, Trace.bValid ? 1 : 0);
    if (!Trace.bValid) FinishNavigationAcceptance(TEXT("complete_detour_invalid"));
}

void APolitikaPrototypeMatch::ObserveNavigationAcceptanceEvent(const politika::rules::GameplayEvent& Event)
{
    auto& Trace = NavigationAcceptance;
    APolitikaPrototypeUnit* Unit = Trace.Unit.Get();
    if (!Trace.bRunning || !Unit) return;
    using namespace politika::rules;
    if (const auto* Command = std::get_if<CommandOutcomeEvent>(&Event);
        Command && Command->issuer_id == Unit->GetStableId())
    {
        const bool bInitial = Command->sequence == Trace.Sequence;
        const bool bRejected = Trace.Interruption == TEXT("OI-03") && !bInitial;
        const bool bStop = Trace.Interruption == TEXT("OI-01") && !bInitial;
        const uint64 Expected = bInitial ? Trace.Sequence : Command->step == 61 ? Trace.DuplicateSequence : Trace.FollowupSequence;
        Trace.bValid &= Command->sequence == Expected && Command->correlation_id == Expected
            && Command->accepted == !bRejected && Command->order == (bStop ? OrderKind::Stop : OrderKind::Move)
            && Command->step == (bInitial ? 1 : Command->sequence == Trace.DuplicateSequence ? 61 : 41)
            && Command->rejection == (bRejected ? OrderRejection::InvalidTarget : OrderRejection::None);
        if (bRejected) ++Trace.Rejected;
        ++Trace.Commands;
    }
    if (const auto* Changed = std::get_if<OrderChangedEvent>(&Event);
        Changed && Changed->entity_id == Unit->GetStableId())
    {
        if (Changed->previous_end == OrderEndReason::Stopped) ++Trace.Stopped;
        if (Changed->previous_end == OrderEndReason::Replaced) ++Trace.Replaced;
        if (Changed->previous_end == OrderEndReason::Arrived)
        {
            ++Trace.Arrivals;
            Trace.ArrivalStep = SimulationStepsExecuted;
            UE_LOG(LogPolitikaStartup, Display,
                TEXT("NAV_ACCEPTANCE_ARRIVED step=%llu unit=%llu sequence=%llu publication=%d"),
                Changed->step, Unit->GetStableId(), Trace.Interruption == TEXT("OI-02") ? Trace.FollowupSequence : Trace.Sequence, Trace.Arrivals);
        }
        else if (Changed->previous_end == OrderEndReason::NavigationFailed || Changed->previous_end == OrderEndReason::Obstructed)
            ++Trace.Failures;
    }
}

void APolitikaPrototypeMatch::AfterNavigationAcceptanceStep(float DeltaSeconds)
{
    auto& Trace = NavigationAcceptance;
    if (!Trace.bRunning) return;
    APolitikaPrototypeUnit* Unit = Trace.Unit.Get();
    AActor* Wall = Trace.Wall.Get();
    if (!Unit || !Wall) { FinishNavigationAcceptance(TEXT("fixture_lost")); return; }
    const auto* WallMesh = Cast<AStaticMeshActor>(Wall);
    const FVector Current = Unit->GetActorLocation();
    FVector PreviousSubstep = Trace.Previous;
    double Travel = 0;
    bool bClear = true;
    for (int32 Index = 0; Index < Unit->NavigationStepTraversal.Num(); ++Index)
    {
        const FVector Point = Unit->NavigationStepTraversal[Index];
        Travel += FVector::Distance(PreviousSubstep, Point);
        bClear &= !BodySweepHitsWall(GetWorld(), Unit->Body, Wall, PreviousSubstep, Point);
        UE_LOG(LogPolitikaStartup, Display, TEXT("NAV_ACCEPTANCE_TRAVERSE step=%llu index=%d position=%.6f,%.6f,%.6f"),
            SimulationStepsExecuted, Index, Point.X, Point.Y, Point.Z);
        PreviousSubstep = Point;
    }
    bClear &= !BodySweepHitsWall(GetWorld(), Unit->Body, Wall, PreviousSubstep, Current);
    const bool bSpeed = Travel <= Unit->MoveSpeed * DeltaSeconds + 0.01;
    Trace.bValid &= bClear && bSpeed && PreviousSubstep.Equals(Current, 0.001)
        && FMath::IsNearlyEqual(DeltaSeconds, 0.05f, 0.000001f) && Lifecycle.IsActive()
        && Unit->GetOwnerPlayer() == LocalRuntimePlayerId
        && (Trace.Interruption == TEXT("OI-01") && SimulationStepsExecuted >= 41
            ? Unit->NavigationOrderSequence == 0 && Unit->NavigationMatchGeneration == 0
                && Unit->NavigationWaypoints.IsEmpty() && Unit->NavigationWaypointIndex == 0
                && Unit->CurrentOrder == EPolitikaPrototypeOrder::Stop && Current.Equals(Trace.BoundaryPosition, 0.001)
            : Unit->NavigationOrderSequence == (Trace.Interruption == TEXT("OI-02") && SimulationStepsExecuted >= 41
                ? Trace.FollowupSequence : Trace.Sequence) && Unit->NavigationMatchGeneration == MatchGeneration)
        && Unit->GetActorEnableCollision()
        && Unit->Body->GetCollisionEnabled() != ECollisionEnabled::NoCollision
        && Unit->Body->GetCollisionResponseToChannel(ECC_WorldStatic) == ECR_Block
        && Unit->Body->Bounds.BoxExtent.Equals(Trace.Extent, 0.001)
        && Wall->GetActorEnableCollision() && WallMesh
        && WallMesh->GetStaticMeshComponent()->GetCollisionEnabled() != ECollisionEnabled::NoCollision
        && WallMesh->GetStaticMeshComponent()->GetCollisionResponseToChannel(ECC_WorldDynamic) == ECR_Block
        && WallMesh->GetStaticMeshComponent()->Bounds.GetBox().Equals(Trace.Blocker, 0.001);
    Trace.MaxTravel = FMath::Max(Trace.MaxTravel, Travel);
    ++Trace.Samples;
    UE_LOG(LogPolitikaStartup, Display,
        TEXT("NAV_ACCEPTANCE_MOVEMENT_SAMPLE step=%llu generation=%llu unit=%llu sequence=%llu owner=%u active=%d position=%.6f,%.6f,%.6f waypoint=%d order=%d travel=%.6f budget=%.6f clear=%d valid=%d path_points=%d subsegments=%d terminations=%d publications=%d failures=%d nav_generation=%llu"),
        SimulationStepsExecuted, MatchGeneration, Unit->GetStableId(), Unit->NavigationOrderSequence,
        Unit->GetOwnerPlayer(), Lifecycle.IsActive() ? 1 : 0, Current.X, Current.Y, Current.Z,
        Unit->NavigationWaypointIndex, static_cast<int32>(Unit->CurrentOrder), Travel,
        Unit->MoveSpeed * DeltaSeconds, bClear ? 1 : 0, Trace.bValid ? 1 : 0,
        Unit->NavigationWaypoints.Num(), Unit->NavigationStepTraversal.Num(), Trace.Terminations, Trace.Arrivals, Trace.Failures,
        Unit->NavigationMatchGeneration);
    Trace.Previous = Current;
    if (!Trace.bValid || Trace.Failures) { FinishNavigationAcceptance(TEXT("movement_invariant")); return; }
    // After movement and publication of this step, before the next admission,
    // including when both steps occur in one catch-up Tick. Enqueue only.
    if (!Trace.Interruption.IsEmpty() && SimulationStepsExecuted == 40)
    {
        Trace.bValid &= Unit->CurrentOrder == EPolitikaPrototypeOrder::Move
            && Unit->NavigationWaypoints.Num() >= 2 && Unit->NavigationOrderSequence == Trace.Sequence
            && !Current.Equals(PolitikaNavigationObstacleFixture::Start, 0.1)
            && FVector::Dist2D(Current, Trace.Resolved) > 18.0 && Trace.Arrivals == 0 && Trace.Commands == 1;
        if (!Trace.bValid) { FinishNavigationAcceptance(TEXT("invalid_step40_baseline")); return; }
        Trace.BoundaryPosition = Current;
        Trace.FollowupSequence = NextCommandSequence;
        const FVector Destination = Trace.Interruption == TEXT("OI-03")
            ? FVector(PlayableMax.X + 1000, PlayableMax.Y + 1000, Current.Z) : PolitikaNavigationObstacleFixture::Start;
        UE_LOG(LogPolitikaStartup, Display, TEXT("NAV_INTERRUPTION_SUBMITTED scenario=%s boundary=40 admission=41 sequence=%llu destination=%.6f,%.6f,%.6f bounds_min=%.6f,%.6f bounds_max=%.6f,%.6f"),
            *Trace.Interruption, Trace.FollowupSequence, Destination.X, Destination.Y, Destination.Z,
            PlayableMin.X, PlayableMin.Y, PlayableMax.X, PlayableMax.Y);
        if (Trace.Interruption == TEXT("OI-01")) SubmitStop(LocalRuntimePlayerId, Unit);
        else SubmitMove(LocalRuntimePlayerId, Unit, Destination);
    }
    if (Trace.Interruption == TEXT("OI-01") && SimulationStepsExecuted == 60)
    {
        Trace.DuplicateSequence = NextCommandSequence;
        SubmitStop(LocalRuntimePlayerId, Unit);
        UE_LOG(LogPolitikaStartup, Display, TEXT("NAV_INTERRUPTION_DUPLICATE boundary=60 admission=61 sequence=%llu"), Trace.DuplicateSequence);
    }
    if (Trace.Interruption == TEXT("OI-01") && SimulationStepsExecuted == 81)
        FinishNavigationAcceptance(nullptr);
    if (Trace.ArrivalStep && SimulationStepsExecuted >= Trace.ArrivalStep + 20)
        FinishNavigationAcceptance(nullptr);
    else if (Trace.Samples >= 800) FinishNavigationAcceptance(TEXT("movement_timeout"));
}

void APolitikaPrototypeMatch::FinishNavigationAcceptance(const TCHAR* Failure)
{
    auto& Trace = NavigationAcceptance;
    APolitikaPrototypeUnit* Unit = Trace.Unit.Get();
    const double Error = Unit ? FVector::Dist2D(Unit->GetActorLocation(), Trace.Resolved) : -1;
    const bool bStop = Trace.Interruption == TEXT("OI-01");
    const bool bPass = !Failure && Trace.bValid && Trace.bRouteRecorded && Unit
        && Trace.Failures == 0 && Trace.Commands == (bStop ? 3 : Trace.Interruption.IsEmpty() ? 1 : 2)
        && Trace.Stopped == (bStop ? 1 : 0) && Trace.Replaced == (Trace.Interruption == TEXT("OI-02") ? 1 : 0)
        && Trace.Rejected == (Trace.Interruption == TEXT("OI-03") ? 1 : 0)
        && (bStop ? Trace.Terminations == 0 && Trace.Arrivals == 0 && SimulationStepsExecuted == 81
                && Unit->CurrentOrder == EPolitikaPrototypeOrder::Stop
            : Trace.Terminations == 1 && Trace.Arrivals == 1 && Trace.ArrivalStep
                && SimulationStepsExecuted >= Trace.ArrivalStep + 20 && Error <= 18.0
                && Unit->CurrentOrder == EPolitikaPrototypeOrder::Idle)
        && Unit->NavigationWaypoints.IsEmpty() && Unit->NavigationWaypointIndex == 0;
    UE_LOG(LogPolitikaStartup, Display,
        TEXT("NAV_ACCEPTANCE_RESULT scenario=StaticObstacleDetour result=%s reason=%s generation=%llu unit=%llu sequence=%llu samples=%d commands=%d terminations=%d arrivals=%d failures=%d arrival_step=%llu final_step=%llu error=%.6f max_travel=%.6f retired=%d"),
        bPass ? TEXT("PASS") : TEXT("FAIL"), Failure ? Failure : (bPass ? TEXT("none") : TEXT("final_invariant")),
        MatchGeneration, Unit ? Unit->GetStableId() : 0, Trace.Sequence, Trace.Samples, Trace.Commands,
        Trace.Terminations, Trace.Arrivals, Trace.Failures, Trace.ArrivalStep, SimulationStepsExecuted, Error,
        Trace.MaxTravel, Unit && Unit->NavigationWaypoints.IsEmpty() ? 1 : 0);
    Trace.bRunning = false;
    if (!Trace.Interruption.IsEmpty())
        UE_LOG(LogPolitikaStartup, Display, TEXT("NAV_INTERRUPTION_RESULT scenario=%s result=%s stopped=%d replaced=%d rejected=%d arrivals=%d original=%llu followup=%llu duplicate=%llu final_step=%llu"),
            *Trace.Interruption, bPass ? TEXT("PASS") : TEXT("FAIL"), Trace.Stopped, Trace.Replaced, Trace.Rejected,
            Trace.Arrivals, Trace.Sequence, Trace.FollowupSequence, Trace.DuplicateSequence, SimulationStepsExecuted);
    if (Unit) Unit->bTraceNavigationTraversal = false;
    FPlatformMisc::RequestExit(false);
}
#endif
