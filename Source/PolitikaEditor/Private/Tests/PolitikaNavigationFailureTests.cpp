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

#endif
