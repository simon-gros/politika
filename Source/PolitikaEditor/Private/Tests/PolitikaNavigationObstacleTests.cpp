#if WITH_DEV_AUTOMATION_TESTS

#include "PolitikaPrototypeGameplay.h"
#include "PolitikaPrototypeContent.h"
#include "PolitikaNavigationObstacleFixture.h"
#include "FileHelpers.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "EngineUtils.h"
#include "NavigationSystem.h"
#include "Misc/AutomationTest.h"
#include "Misc/PackageName.h"
#include "HAL/PlatformProcess.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPolitikaNavigationObstacleAutomationTest,
    "Politika.RTS.Navigation.SavedMapStaticObstacleDetour",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPolitikaNavigationObstacleAutomationTest::RunTest(const FString& Parameters)
{
    using namespace politika::rules;
    const FString LevelFilename = FPackageName::LongPackageNameToFilename(
        TEXT("/Game/Maps/RTSVerticalSlice"), FPackageName::GetMapPackageExtension());
    UWorld* World = UEditorLoadingAndSavingUtils::LoadMap(LevelFilename);
    TestNotNull(TEXT("Canonical saved RTS map loads for an isolated in-memory navigation fixture"), World);
    if (!World) return false;

    UNavigationSystemV1* NavSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(World);
    UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
    AStaticMeshActor* Wall = Cube
        ? World->SpawnActor<AStaticMeshActor>(PolitikaNavigationObstacleFixture::WallLocation, FRotator::ZeroRotator) : nullptr;
    TestNotNull(TEXT("Transient static obstacle spawned"), Wall);
    if (!NavSystem || !Wall)
    {
        UEditorLoadingAndSavingUtils::LoadMap(TEXT("/Game/Maps/MainMenu"));
        return false;
    }
    Wall->GetStaticMeshComponent()->SetStaticMesh(Cube);
    Wall->SetActorScale3D(PolitikaNavigationObstacleFixture::WallScale);
    Wall->GetStaticMeshComponent()->SetMobility(EComponentMobility::Static);
    Wall->GetStaticMeshComponent()->SetCollisionProfileName(TEXT("BlockAll"));
    Wall->GetStaticMeshComponent()->SetCanEverAffectNavigation(true);
    Wall->GetStaticMeshComponent()->ReregisterComponent();
    PolitikaNavigationObstacleFixture::ExcludeSolidInterior(Wall);
    UNavigationSystemV1::UpdateActorAndComponentsInNavOctree(*Wall);
    NavSystem->RemoveNavigationBuildLock(ENavigationBuildLock::AsyncLoadLock,
        UNavigationSystemV1::ELockRemovalRebuildAction::NoRebuild);
    NavSystem->ReleaseInitialBuildingLock();
    NavSystem->Build();
    int32 WaitIterations = 0;
    while (NavSystem->IsNavigationBuildInProgress() && WaitIterations++ < 600)
        FPlatformProcess::Sleep(0.1f);
    TestFalse(TEXT("Transient obstacle NavMesh build completes within 60 seconds"),
        NavSystem->IsNavigationBuildInProgress());

    const FVector Start = PolitikaNavigationObstacleFixture::Start;
    const FVector Goal = PolitikaNavigationObstacleFixture::Goal;
    FHitResult DirectHit;
    const bool bDirectBlocked = World->LineTraceSingleByChannel(DirectHit, Start, Goal,
        ECC_WorldStatic, FCollisionQueryParams(SCENE_QUERY_STAT(PolitikaNavigationDirectSegment), true));
    TestTrue(TEXT("The requested direct segment physically intersects the static blocker"),
        bDirectBlocked && DirectHit.GetActor() == Wall);

    APolitikaPrototypeMatch* Match = World->SpawnActor<APolitikaPrototypeMatch>();
    APolitikaPrototypeUnit* Unit = World->SpawnActor<APolitikaPrototypeUnit>(Start, FRotator::ZeroRotator);
    APolitikaPrototypeObjective* Objective = World->SpawnActor<APolitikaPrototypeObjective>();
    TestNotNull(TEXT("Production coordinator spawned in the saved map"), Match);
    TestNotNull(TEXT("Production unit spawned in the saved map"), Unit);
    if (!Match || !Unit || !Objective)
    {
        UEditorLoadingAndSavingUtils::LoadMap(TEXT("/Game/Maps/MainMenu"));
        return false;
    }
    Unit->Configure(9901, 1, FText::FromString(TEXT("Navigation detour fixture")), 100,
        220.0f, 250.0f, 10.0f, 0.5f, true, nullptr, FVector(0.5f), FLinearColor::White);
    Unit->SetActorLocation(Start, false, nullptr, ETeleportType::TeleportPhysics);
    Match->Units = {Unit};
    Match->Objective = nullptr;
    Match->MatchGeneration = 990;
    Match->LocalRuntimePlayerId = 1;
    Match->PlayableMin = FVector2D(-2000, -2000);
    Match->PlayableMax = FVector2D(2000, 2000);
    Match->bContentReady = true;
    const bool bActive = Match->TransitionLifecycle(MatchPhase::SetupValidated)
        && Match->TransitionLifecycle(MatchPhase::Loading)
        && Match->TransitionLifecycle(MatchPhase::Starting)
        && Match->TransitionLifecycle(MatchPhase::Active);
    TestTrue(TEXT("Coordinator becomes Active through its production lifecycle"), bActive);

    Match->SubmitMove(1, Unit, Goal);
    Match->AdmitPendingOrders();
    TestEqual(TEXT("Legitimate typed Move installs an executable path"), Unit->GetCurrentOrder(), EPolitikaPrototypeOrder::Move);
    TestTrue(TEXT("Returned complete route has multiple usable points"), Unit->NavigationWaypoints.Num() >= 2);
    bool bRouteAvoidsWall = Unit->NavigationWaypoints.Num() >= 2;
    for (int32 Index = 1; Index < Unit->NavigationWaypoints.Num(); ++Index)
    {
        FHitResult SegmentHit;
        FCollisionQueryParams Params(SCENE_QUERY_STAT(PolitikaNavigationRouteSegment), true);
        Params.AddIgnoredActor(Unit);
        Params.AddIgnoredActor(Match);
        Params.AddIgnoredActor(Objective);
        bRouteAvoidsWall &= !World->LineTraceSingleByChannel(SegmentHit,
            Unit->NavigationWaypoints[Index - 1], Unit->NavigationWaypoints[Index], ECC_WorldStatic, Params)
            || SegmentHit.GetActor() != Wall;
    }
    TestTrue(TEXT("Every route segment goes around the blocker rather than through it"), bRouteAvoidsWall);

    const FBox WallBounds = Wall->GetComponentsBoundingBox(true);
    bool bNeverPenetrated = true;
    FVector FirstOverlap = FVector::ZeroVector;
    float MaxStepDistance = 0.0f;
    FVector Previous = Unit->GetActorLocation();
    for (int32 Step = 0; Step < 1000 && Unit->GetCurrentOrder() == EPolitikaPrototypeOrder::Move; ++Step)
    {
        Match->RunSimulationStep(0.05f);
        const FVector Current = Unit->GetActorLocation();
        MaxStepDistance = FMath::Max(MaxStepDistance, FVector::Distance(Previous, Current));
        const FBox UnitBounds = Unit->Body->Bounds.GetBox();
        const FBox Intersection(FVector::Max(UnitBounds.Min, WallBounds.Min),
            FVector::Min(UnitBounds.Max, WallBounds.Max));
        if (Intersection.IsValid && Intersection.GetSize().GetMin() > 0.5f)
        {
            if (bNeverPenetrated) FirstOverlap = Current;
            bNeverPenetrated = false;
        }
        Previous = Current;
    }
    TestTrue(FString::Printf(TEXT("Unit collision body remains outside the blocking obstacle throughout authoritative steps (first overlap %s, wall %s, unit %s)"),
        *FirstOverlap.ToString(), *WallBounds.ToString(), *Unit->Body->Bounds.GetBox().ToString()), bNeverPenetrated);
    TestTrue(TEXT("Per-step distance remains within authored speed times 0.05 s"),
        MaxStepDistance <= 220.0f * 0.05f + 1.0f);
    TestEqual(TEXT("Route completes without overshoot or oscillation"), Unit->GetCurrentOrder(), EPolitikaPrototypeOrder::Idle);
    TestTrue(TEXT("Authoritative final position reaches the resolved goal tolerance"),
        FVector::Dist2D(Unit->GetActorLocation(), Unit->ResolvedNavigationDestination) <= 18.0f);
    int32 ArrivalEvents = 0;
    for (const GameplayEvent& Event : Match->PublishedEvents)
        if (const OrderChangedEvent* Changed = std::get_if<OrderChangedEvent>(&Event))
            ArrivalEvents += Changed->previous_end == OrderEndReason::Arrived ? 1 : 0;
    TestEqual(TEXT("Move arrival is published exactly once"), ArrivalEvents, 1);

    UEditorLoadingAndSavingUtils::LoadMap(TEXT("/Game/Maps/MainMenu"));
    return true;
}

#endif
