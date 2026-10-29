#if WITH_DEV_AUTOMATION_TESTS

#include "PolitikaPrototypeGameplay.h"
#include "PolitikaPrototypeContent.h"
#include "PolitikaNavigationFailureFixture.h"
#include "FileHelpers.h"
#include "Engine/World.h"
#include "Engine/StaticMeshActor.h"
#include "EngineUtils.h"
#include "NavigationSystem.h"
#include "NavigationData.h"
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPolitikaNavigationFailureAutomationTest,
    "Politika.RTS.Navigation.SavedMapDisconnectedDestinationFailsClosed",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPolitikaNavigationFailureAutomationTest::RunTest(const FString& Parameters)
{
    using namespace politika::rules;
    UWorld* World = UEditorLoadingAndSavingUtils::LoadMap(PolitikaNavigationFailureFixture::Level);
    if (!TestNotNull(TEXT("Saved disconnected fixture loads"), World)) return false;
    UNavigationSystemV1* Nav = FNavigationSystem::GetCurrent<UNavigationSystemV1>(World);
    ANavigationData* Data = Nav ? Nav->GetDefaultNavDataInstance(FNavigationSystem::DontCreate) : nullptr;
    TestTrue(TEXT("Cookable saved Recast data is present and ready"), Data && !Nav->IsNavigationBuildInProgress());
    FNavLocation Start, Goal;
    TestTrue(TEXT("Unit origin independently projects to saved NavMesh"), Nav && Data
        && Nav->ProjectPointToNavigation(PolitikaNavigationFailureFixture::Start, Start, FVector(80, 80, 180), Data)
        && FVector::Dist(Start.Location, PolitikaNavigationFailureFixture::Start) <= 80.0f);
    TestTrue(TEXT("Requested goal independently projects to saved NavMesh"), Nav && Data
        && Nav->ProjectPointToNavigation(PolitikaNavigationFailureFixture::Goal, Goal, FVector(80, 80, 180), Data)
        && FVector::Dist(Goal.Location, PolitikaNavigationFailureFixture::Goal) <= 80.0f);
    if (!Nav || !Data || Nav->IsNavigationBuildInProgress()) return false;
    FPathFindingQuery Query(nullptr, *Data, Start.Location, Goal.Location);
    Query.SetAllowPartialPaths(false);
    Query.SetRequireNavigableEndLocation(true);
    const FPathFindingResult QueryResult = Nav->FindPathSync(Query);
    TestTrue(FString::Printf(TEXT("Real complete-only query fails, not Success (actual=%u)"),
        static_cast<uint32>(QueryResult.Result)), QueryResult.Result != ENavigationQueryResult::Success);

    auto* Match = World->SpawnActor<APolitikaPrototypeMatch>();
    auto* Unit = World->SpawnActor<APolitikaPrototypeUnit>(PolitikaNavigationFailureFixture::Start, FRotator::ZeroRotator);
    if (!TestNotNull(TEXT("Coordinator spawned"), Match) || !TestNotNull(TEXT("Unit spawned"), Unit)) return false;
    Match->SetActorTickEnabled(false);
    Unit->Configure(9914, 1, FText::FromString(TEXT("Disconnected-route fixture")), 100, 210, 250, 10, 0.5f,
        true, nullptr, FVector(0.85f), FLinearColor::White);
    Unit->SetActorLocation(PolitikaNavigationFailureFixture::Start);
    Match->Units = {Unit};
    Match->MapDefinition = LoadObject<UPolitikaPrototypeMapDefinition>(nullptr,
        TEXT("/Game/Politika/TestFixtures/Navigation/DA_DisconnectedNavigation.DA_DisconnectedNavigation"));
    Match->MatchGeneration = 991;
    Match->LocalRuntimePlayerId = 1;
    Match->PlayableMin = FVector2D(-2000, -2000);
    Match->PlayableMax = FVector2D(2000, 2000);
    Match->bContentReady = true;
    const bool bActive = Match->TransitionLifecycle(MatchPhase::SetupValidated)
        && Match->TransitionLifecycle(MatchPhase::Loading)
        && Match->TransitionLifecycle(MatchPhase::Starting)
        && Match->TransitionLifecycle(MatchPhase::Active);
    TestTrue(TEXT("Coordinator reaches Active through legal lifecycle transitions"), bActive);
    FPathFindingQuery PartialQuery(Unit, *Data, Start.Location, Goal.Location);
    PartialQuery.SetAllowPartialPaths(true);
    PartialQuery.SetRequireNavigableEndLocation(true);
    const FPathFindingResult PartialResult = Nav->FindPathSync(PartialQuery);
    FString PartialRejection;
    const bool bPartialAdmitted = Match->IsCompleteNavigationResult(PartialResult, PartialRejection);
    if (PartialResult.IsPartial() && PartialResult.Path.IsValid() && !PartialResult.Path->GetPathPoints().IsEmpty())
    {
        const FVector End = PartialResult.Path->GetPathPoints().Last().Location;
        TestTrue(TEXT("Diagnostic query produced a genuine partial endpoint distinct from requested goal"),
            FVector::Distance(End, Goal.Location) > 50.0f);
        TestFalse(TEXT("Production complete-route boundary rejects genuine partial result"), bPartialAdmitted);
        TestTrue(TEXT("Partial-route rejection has a failure classification"), !PartialRejection.IsEmpty());
    }
    else
    {
        AddInfo(TEXT("NQ-02 INCONCLUSIVE: diagnostic Unreal query did not return a genuine partial path."));
    }
    int32 AcceptedMoves = 0, NavigationFailures = 0, Arrivals = 0;
    Match->GameplayEventPublished.AddLambda([&](const GameplayEvent& Event)
    {
        if (const auto* Command = std::get_if<CommandOutcomeEvent>(&Event))
        {
            if (Command->order == OrderKind::Move && Command->accepted && Command->sequence == 1) ++AcceptedMoves;
        }
        if (const auto* Changed = std::get_if<OrderChangedEvent>(&Event))
        {
            NavigationFailures += Changed->previous_end == OrderEndReason::NavigationFailed ? 1 : 0;
            Arrivals += Changed->previous_end == OrderEndReason::Arrived ? 1 : 0;
        }
    });
    Match->SubmitMove(1, Unit, PolitikaNavigationFailureFixture::Goal);
    Match->Tick(0.05f);
    TestEqual(TEXT("Typed Move intent admitted once"), AcceptedMoves, 1);
    TestEqual(TEXT("NavigationFailed published once"), NavigationFailures, 1);
    TestEqual(TEXT("No arrival published"), Arrivals, 0);
    TestTrue(TEXT("Failed route leaves the unit alive and stationary"), Unit->IsAlive()
        && Unit->GetActorLocation().Equals(PolitikaNavigationFailureFixture::Start, 0.001));
    TestTrue(TEXT("Failed route is not executable and order is stopped"), Unit->GetCurrentOrder() == EPolitikaPrototypeOrder::Stop
        && Unit->NavigationWaypoints.IsEmpty() && Unit->NavigationWaypointIndex == 0
        && Unit->NavigationOrderSequence == 0 && Unit->NavigationMatchGeneration == 0);
    for (int32 Step = 0; Step < 20; ++Step) Match->Tick(0.05f);
    TestTrue(TEXT("Match remains Active through 20 post-failure steps"), Match->Lifecycle.IsActive()
        && Match->SimulationStepsExecuted >= 21 && Unit->GetActorLocation().Equals(PolitikaNavigationFailureFixture::Start, 0.001));
    TestEqual(TEXT("No repeated navigation failure"), NavigationFailures, 1);
    TestEqual(TEXT("No delayed arrival"), Arrivals, 0);
    Match->GameplayEventPublished.Clear();
    UEditorLoadingAndSavingUtils::LoadMap(TEXT("/Game/Maps/MainMenu"));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPolitikaUnprojectableDestinationAutomationTest,
    "Politika.RTS.Navigation.SavedMapUnprojectableDestinationFailsClosed",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPolitikaUnprojectableDestinationAutomationTest::RunTest(const FString& Parameters)
{
    using namespace politika::rules;
    using namespace PolitikaUnprojectableDestinationFixture;
    UWorld* World = UEditorLoadingAndSavingUtils::LoadMap(Level);
    if (!TestNotNull(TEXT("Saved unprojectable-destination fixture loads"), World)) return false;
    UNavigationSystemV1* Nav = FNavigationSystem::GetCurrent<UNavigationSystemV1>(World);
    ANavigationData* Data = Nav ? Nav->GetDefaultNavDataInstance(FNavigationSystem::DontCreate) : nullptr;
    TestTrue(TEXT("Ready saved Recast data is present"), Nav && Data && !Nav->IsNavigationBuildInProgress());
    FNavLocation ProjectedStart, ProjectedGoal;
    TestTrue(TEXT("Unit origin projects successfully"), Nav && Data
        && Nav->ProjectPointToNavigation(Start, ProjectedStart, FVector(80, 80, 180), Data));
    TestFalse(TEXT("In-bounds authored ground goal does not project"), Nav && Data
        && Nav->ProjectPointToNavigation(Goal, ProjectedGoal, FVector(80, 80, 180), Data));
    AActor* Exclusion = nullptr;
    for (TActorIterator<AActor> It(World); It; ++It)
        if (It->ActorHasTag(ExclusionTag)) Exclusion = *It;
    FHitResult GroundHit;
    FCollisionQueryParams GroundQuery(SCENE_QUERY_STAT(NavigationFailureGroundAutomation), true);
    if (Exclusion) GroundQuery.AddIgnoredActor(Exclusion);
    const bool bGroundTrace = World && World->LineTraceSingleByChannel(GroundHit,
        Goal + FVector(0, 0, 150), Goal - FVector(0, 0, 150), ECC_Visibility, GroundQuery);
    AddInfo(FString::Printf(TEXT("Ground trace=%d hit_actor=%s hit_location=%s exclusion=%s"), bGroundTrace ? 1 : 0,
        GroundHit.GetActor() ? *GroundHit.GetActor()->GetPathName() : TEXT("none"),
        *GroundHit.ImpactPoint.ToString(), Exclusion ? *Exclusion->GetPathName() : TEXT("none")));
    TestTrue(FString::Printf(TEXT("Unnavigable destination lies on authored ground (hit=%s point=%s)"),
        GroundHit.GetActor() ? *GroundHit.GetActor()->GetClass()->GetName() : TEXT("none"), *GroundHit.ImpactPoint.ToString()),
        bGroundTrace && GroundHit.GetActor()
        && (GroundHit.GetActor()->ActorHasTag(TEXT("PolitikaPrototypeGround"))
            || GroundHit.GetActor()->GetClass()->GetName().StartsWith(TEXT("Landscape")))
        && FVector::Dist(GroundHit.ImpactPoint, Goal) <= 5.0f);
    auto* Match = World->SpawnActor<APolitikaPrototypeMatch>();
    auto* Unit = World->SpawnActor<APolitikaPrototypeUnit>(Start, FRotator::ZeroRotator);
    if (!TestNotNull(TEXT("Coordinator spawned"), Match) || !TestNotNull(TEXT("Unit spawned"), Unit)) return false;
    Match->SetActorTickEnabled(false);
    Unit->Configure(9915, 1, FText::FromString(TEXT("Unprojectable destination fixture")), 100, 210, 250, 10, 0.5f,
        true, nullptr, FVector(0.85f), FLinearColor::White);
    Unit->SetActorLocation(Start);
    Match->Units = {Unit};
    Match->MapDefinition = LoadObject<UPolitikaPrototypeMapDefinition>(nullptr,
        TEXT("/Game/Politika/TestFixtures/Navigation/DA_UnprojectableDestination.DA_UnprojectableDestination"));
    Match->MatchGeneration = 992;
    Match->LocalRuntimePlayerId = 1;
    Match->PlayableMin = FVector2D(-2000, -2000);
    Match->PlayableMax = FVector2D(2000, 2000);
    Match->bContentReady = true;
    const bool bActive = Match->TransitionLifecycle(MatchPhase::SetupValidated)
        && Match->TransitionLifecycle(MatchPhase::Loading)
        && Match->TransitionLifecycle(MatchPhase::Starting)
        && Match->TransitionLifecycle(MatchPhase::Active);
    TestTrue(TEXT("Coordinator reaches Active through legal lifecycle transitions"), bActive);
    int32 AcceptedMoves = 0, NavigationFailures = 0, Arrivals = 0;
    Match->GameplayEventPublished.AddLambda([&](const GameplayEvent& Event)
    {
        if (const auto* Command = std::get_if<CommandOutcomeEvent>(&Event))
            AcceptedMoves += Command->order == OrderKind::Move && Command->accepted ? 1 : 0;
        if (const auto* Changed = std::get_if<OrderChangedEvent>(&Event))
        {
            NavigationFailures += Changed->previous_end == OrderEndReason::NavigationFailed ? 1 : 0;
            Arrivals += Changed->previous_end == OrderEndReason::Arrived ? 1 : 0;
        }
    });
    Match->SubmitMove(1, Unit, Goal);
    Match->Tick(0.05f);
    TestEqual(TEXT("Typed in-bounds Move is admitted once"), AcceptedMoves, 1);
    TestEqual(TEXT("DestinationNotNavigable is published once"), NavigationFailures, 1);
    TestEqual(TEXT("Failed Move does not arrive"), Arrivals, 0);
    TestTrue(TEXT("Failure retires executable route and leaves unit stationary"), Unit->IsAlive()
        && Unit->GetActorLocation().Equals(Start, 0.001) && Unit->NavigationWaypoints.IsEmpty()
        && Unit->NavigationWaypointIndex == 0 && Unit->NavigationOrderSequence == 0
        && Unit->NavigationMatchGeneration == 0);
    for (int32 Step = 0; Step < 20; ++Step) Match->Tick(0.05f);
    TestTrue(TEXT("Twenty subsequent fixed steps remain inert"), Unit->GetActorLocation().Equals(Start, 0.001));
    TestEqual(TEXT("No repeated navigation failure"), NavigationFailures, 1);
    TestEqual(TEXT("No delayed arrival"), Arrivals, 0);
    Match->GameplayEventPublished.Clear();
    UEditorLoadingAndSavingUtils::LoadMap(TEXT("/Game/Maps/MainMenu"));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPolitikaProjectionDisplacementAutomationTest,
    "Politika.RTS.Navigation.SavedMapExcessiveProjectionDisplacementFailsClosed",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPolitikaProjectionDisplacementAutomationTest::RunTest(const FString& Parameters)
{
    using namespace politika::rules;
    using namespace PolitikaProjectionDisplacementFixture;
    UWorld* World = UEditorLoadingAndSavingUtils::LoadMap(Level);
    if (!TestNotNull(TEXT("Saved projection-displacement fixture loads"), World)) return false;
    UNavigationSystemV1* Nav = FNavigationSystem::GetCurrent<UNavigationSystemV1>(World);
    ANavigationData* Data = Nav ? Nav->GetDefaultNavDataInstance(FNavigationSystem::DontCreate) : nullptr;
    TestTrue(TEXT("Ready saved Recast data is present"), Nav && Data && !Nav->IsNavigationBuildInProgress());
    FNavLocation ProjectedStart, ProjectedGoal;
    const bool bOrigin = Nav && Data && Nav->ProjectPointToNavigation(Start, ProjectedStart, FVector(80, 80, 180), Data);
    const bool bGoal = Nav && Data && Nav->ProjectPointToNavigation(Goal, ProjectedGoal, FVector(80, 80, 180), Data);
    const double Displacement = bGoal ? FVector::Dist(ProjectedGoal.Location, Goal) : 0.0;
    TestTrue(TEXT("Unit origin projects successfully"), bOrigin);
    TestTrue(FString::Printf(TEXT("In-bounds ground goal projects >150 cm away (actual=%.9f)"), Displacement),
        bGoal && Displacement > 150.0);
    if (!Nav || !Data || !bOrigin || !bGoal || Displacement <= 150.0) return false;

    AActor* Platform = nullptr;
    for (TActorIterator<AActor> It(World); It; ++It)
        if (It->ActorHasTag(PlatformTag)) Platform = *It;
    FHitResult GroundHit;
    const bool bGround = World->LineTraceSingleByChannel(GroundHit, Goal + FVector(0, 0, 300),
        Goal - FVector(0, 0, 300), ECC_Visibility);
    TestTrue(TEXT("Requested XY is on the authored elevated ground surface"), bGround && GroundHit.GetActor() == Platform
        && FMath::Abs(GroundHit.ImpactPoint.Z - 200.0) <= 5.0);
    AddInfo(FString::Printf(TEXT("requested=%s projected=%s ground=%s horizontal=%.9f vertical=%.9f distance=%.9f"),
        *Goal.ToString(), *ProjectedGoal.Location.ToString(), *GroundHit.ImpactPoint.ToString(),
        FVector::Dist2D(ProjectedGoal.Location, Goal), FMath::Abs(ProjectedGoal.Location.Z - Goal.Z), Displacement));
    FPathFindingQuery ConnectivityQuery(nullptr, *Data, ProjectedStart.Location, ProjectedGoal.Location);
    ConnectivityQuery.SetAllowPartialPaths(false);
    ConnectivityQuery.SetRequireNavigableEndLocation(true);
    const FPathFindingResult Connectivity = Nav->FindPathSync(ConnectivityQuery);
    AddInfo(FString::Printf(TEXT("Ramp connectivity: result=%u partial=%d path_valid=%d points=%d start=%s goal=%s"),
        static_cast<uint32>(Connectivity.Result), Connectivity.IsPartial() ? 1 : 0,
        Connectivity.Path.IsValid() ? 1 : 0,
        Connectivity.Path.IsValid() ? Connectivity.Path->GetPathPoints().Num() : 0,
        *ProjectedStart.Location.ToString(), *ProjectedGoal.Location.ToString()));
    TestTrue(FString::Printf(TEXT("Raised target is connected by the authored ramp (result=%u partial=%d points=%d)"),
        static_cast<uint32>(Connectivity.Result), Connectivity.IsPartial() ? 1 : 0,
        Connectivity.Path.IsValid() ? Connectivity.Path->GetPathPoints().Num() : 0),
        Connectivity.Result == ENavigationQueryResult::Success && Connectivity.Path.IsValid()
        && !Connectivity.Path->IsPartial() && !Connectivity.Path->GetPathPoints().IsEmpty());

    auto* Match = World->SpawnActor<APolitikaPrototypeMatch>();
    auto* Unit = World->SpawnActor<APolitikaPrototypeUnit>(Start, FRotator::ZeroRotator);
    if (!TestNotNull(TEXT("Coordinator spawned"), Match) || !TestNotNull(TEXT("Unit spawned"), Unit)) return false;
    Match->SetActorTickEnabled(false);
    Unit->Configure(9916, 1, FText::FromString(TEXT("Projection displacement fixture")), 100, 210, 250, 10, 0.5f,
        true, nullptr, FVector(0.85f), FLinearColor::White);
    Unit->SetActorLocation(Start);
    Match->Units = {Unit};
    Match->MapDefinition = LoadObject<UPolitikaPrototypeMapDefinition>(nullptr,
        TEXT("/Game/Politika/TestFixtures/Navigation/DA_ProjectionDisplacement.DA_ProjectionDisplacement"));
    Match->MatchGeneration = 993;
    Match->LocalRuntimePlayerId = 1;
    Match->PlayableMin = FVector2D(-2000, -2000);
    Match->PlayableMax = FVector2D(2000, 2000);
    Match->bContentReady = true;
    const bool bActive = Match->TransitionLifecycle(MatchPhase::SetupValidated)
        && Match->TransitionLifecycle(MatchPhase::Loading)
        && Match->TransitionLifecycle(MatchPhase::Starting)
        && Match->TransitionLifecycle(MatchPhase::Active);
    TestTrue(TEXT("Coordinator reaches Active through legal lifecycle transitions"), bActive);
    auto& Trace = Match->NavigationFailureAcceptance;
    Trace.bRunning = true;
    Trace.bValid = true;
    Trace.bDestinationProjectionCase = true;
    Trace.bExcessiveProjectionCase = true;
    Trace.Unit = Unit;
    Trace.Start = Start;
    Trace.Goal = Goal;
    Trace.Sequence = Match->NextCommandSequence;
    int32 AcceptedMoves = 0, NavigationFailures = 0, Arrivals = 0;
    Match->GameplayEventPublished.AddLambda([&](const GameplayEvent& Event)
    {
        if (const auto* Command = std::get_if<CommandOutcomeEvent>(&Event))
            AcceptedMoves += Command->order == OrderKind::Move && Command->accepted ? 1 : 0;
        if (const auto* Changed = std::get_if<OrderChangedEvent>(&Event))
        {
            NavigationFailures += Changed->previous_end == OrderEndReason::NavigationFailed ? 1 : 0;
            Arrivals += Changed->previous_end == OrderEndReason::Arrived ? 1 : 0;
        }
    });
    Match->SubmitMove(1, Unit, Goal);
    Match->Tick(0.05f);
    TestEqual(TEXT("Typed Move is accepted once before navigation failure"), AcceptedMoves, 1);
    TestEqual(TEXT("DestinationNotNavigable publishes once"), NavigationFailures, 1);
    TestEqual(TEXT("Rejected route never arrives"), Arrivals, 0);
    TestTrue(TEXT("Production guard observed both projections and skipped pathfinding"),
        Match->NavigationFailureAcceptance.ProjectionChecks == 2
        && Match->NavigationFailureAcceptance.bOriginProjected
        && Match->NavigationFailureAcceptance.bGoalProjected
        && Match->NavigationFailureAcceptance.ProjectionDisplacement > 150.0
        && Match->NavigationFailureAcceptance.Queries == 0
        && Match->NavigationFailureAcceptance.LastFailure == TEXT("DestinationNotNavigable"));
    TestTrue(TEXT("Failure retires route and leaves unit stationary"), Unit->IsAlive()
        && Unit->GetActorLocation().Equals(Start, 0.001) && Unit->GetCurrentOrder() == EPolitikaPrototypeOrder::Stop
        && Unit->NavigationWaypoints.IsEmpty() && Unit->NavigationWaypointIndex == 0
        && Unit->NavigationOrderSequence == 0 && Unit->NavigationMatchGeneration == 0);
    for (int32 Step = 0; Step < 20; ++Step) Match->Tick(0.05f);
    TestTrue(TEXT("Twenty following fixed steps remain inert and Active"), Match->Lifecycle.IsActive()
        && Unit->GetActorLocation().Equals(Start, 0.001));
    TestEqual(TEXT("No duplicate failure event"), NavigationFailures, 1);
    TestEqual(TEXT("No delayed arrival"), Arrivals, 0);
    TestTrue(TEXT("Production acceptance observer records actual successful excessive projection and no route query"),
        !Trace.bRunning && Trace.bValid && Trace.ProjectionChecks == 2
        && Trace.bOriginProjected && Trace.bGoalProjected && Trace.ProjectionDisplacement > 150.0
        && Trace.Queries == 0 && Trace.FailureEvents == 1 && Trace.Arrivals == 0 && Trace.Samples == 21);
    Match->GameplayEventPublished.Clear();
    UEditorLoadingAndSavingUtils::LoadMap(TEXT("/Game/Maps/MainMenu"));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPolitikaRecastStraightPathResultProbe,
    "Politika.RTS.Navigation.StraightPathResultProbe",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPolitikaRecastStraightPathResultProbe::RunTest(const FString& Parameters)
{
    // Bounded ER-01 feasibility observer: issue genuine synchronous queries against the
    // existing saved static Recast fixture. No result/path state is modified and the
    // production route resolver is not replaced or bypassed by an injected result.
    UWorld* World = UEditorLoadingAndSavingUtils::LoadMap(PolitikaNavigationFailureFixture::Level);
    if (!TestNotNull(TEXT("Existing static-navigation fixture loads"), World)) return false;
    UNavigationSystemV1* Nav = FNavigationSystem::GetCurrent<UNavigationSystemV1>(World);
    ANavigationData* Data = Nav ? Nav->GetDefaultNavDataInstance(FNavigationSystem::DontCreate) : nullptr;
    if (!TestTrue(TEXT("Static Recast NavData is ready"), Nav && Data && !Nav->IsNavigationBuildInProgress())) return false;

    FNavLocation ProjectedOrigin;
    if (!TestTrue(TEXT("Probe origin projects to the saved NavMesh"),
        Nav->ProjectPointToNavigation(PolitikaNavigationFailureFixture::Start, ProjectedOrigin,
            FVector(80, 80, 180), Data))) return false;
    APolitikaPrototypeUnit* QueryOwner = World->SpawnActor<APolitikaPrototypeUnit>(
        PolitikaNavigationFailureFixture::Start, FRotator::ZeroRotator);
    if (!TestNotNull(TEXT("Production query-owner unit is available"), QueryOwner)) return false;
    QueryOwner->Configure(9915, 1, FText::FromString(TEXT("Straight-path probe")), 100, 210,
        250, 10, 0.5f, true, nullptr, FVector(0.85f), FLinearColor::White);

    struct FProbeCase
    {
        const TCHAR* Name;
        FVector RequestedEnd;
    };
    const FProbeCase Cases[] = {
        { TEXT("SP-01-coincident"), ProjectedOrigin.Location },
        { TEXT("SP-02-one-centimeter"), ProjectedOrigin.Location + FVector(1, 0, 0) },
        { TEXT("SP-03-one-meter"), ProjectedOrigin.Location + FVector(100, 0, 0) },
        { TEXT("SP-04-two-point-five-meters"), ProjectedOrigin.Location + FVector(0, 250, 0) },
    };

    for (const FProbeCase& Probe : Cases)
    {
        FNavLocation ProjectedEnd;
        const bool bProjected = Nav->ProjectPointToNavigation(Probe.RequestedEnd, ProjectedEnd,
            FVector(80, 80, 180), Data);
        if (!TestTrue(FString::Printf(TEXT("%s endpoint projects on the existing NavMesh"), Probe.Name), bProjected))
            continue;

        FPathFindingQuery Query(QueryOwner, *Data, ProjectedOrigin.Location, ProjectedEnd.Location);
        Query.SetAllowPartialPaths(false);
        Query.SetRequireNavigableEndLocation(true);
        const FPathFindingResult Result = Nav->FindPathSync(Query);
        const bool bSharedPath = Result.Path.IsValid();
        const bool bPathObjectValid = bSharedPath && Result.Path->IsValid();
        const bool bPartial = Result.IsPartial();
        const int32 PointCount = bSharedPath ? Result.Path->GetPathPoints().Num() : 0;
        const FString Record = FString::Printf(
            TEXT("ER01_STRAIGHT_PATH_PROBE case=%s nav=%s origin=%.6f,%.6f,%.6f requested_end=%.6f,%.6f,%.6f projected_end=%.6f,%.6f,%.6f projected_delta=%.6f result=%u shared_path_valid=%d path_object_valid=%d partial=%d points=%d"),
            Probe.Name, *Data->GetPathName(), ProjectedOrigin.Location.X, ProjectedOrigin.Location.Y,
            ProjectedOrigin.Location.Z, Probe.RequestedEnd.X, Probe.RequestedEnd.Y, Probe.RequestedEnd.Z,
            ProjectedEnd.Location.X, ProjectedEnd.Location.Y, ProjectedEnd.Location.Z,
            FVector::Distance(ProjectedOrigin.Location, ProjectedEnd.Location),
            static_cast<uint32>(Result.Result), bSharedPath ? 1 : 0, bPathObjectValid ? 1 : 0,
            bPartial ? 1 : 0, PointCount);
        AddInfo(Record);
    }
    return true;
}

#endif
