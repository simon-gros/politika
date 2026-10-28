#if WITH_DEV_AUTOMATION_TESTS
#include "PolitikaPrototypeGameplay.h"
#include "PolitikaNavigationObstacleFixture.h"
#include "FileHelpers.h"
#include "Engine/World.h"
#include "NavigationSystem.h"
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPolitikaNavigationScheduleAutomationTest,
    "Politika.RTS.Navigation.SavedMapDetourRenderScheduleRepeatability",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPolitikaNavigationScheduleAutomationTest::RunTest(const FString& Parameters)
{
    using namespace politika::rules;
    struct FSample { FVector Position; int32 Cursor; EPolitikaPrototypeOrder Order; };
    TArray<FSample> Reference;
    TArray<FVector> ReferenceRoute;
    TArray<FString> ReferenceEvents;
    uint64 ReferenceArrival = 0;
    for (int32 Schedule = 1; Schedule <= 5; ++Schedule)
    {
        UWorld* World = UEditorLoadingAndSavingUtils::LoadMap(PolitikaNavigationObstacleFixture::Level);
        if (!TestNotNull(TEXT("Accepted saved fixture loads"), World)) return false;
        auto* Nav = FNavigationSystem::GetCurrent<UNavigationSystemV1>(World);
        if (!TestTrue(TEXT("Saved navigation is ready"), Nav && Nav->GetDefaultNavDataInstance(FNavigationSystem::DontCreate)
            && !Nav->IsNavigationBuildInProgress())) return false;
        auto* Match = World->SpawnActor<APolitikaPrototypeMatch>();
        Match->SetActorTickEnabled(false); // Explicit harness owns outer updates, never world-ticked too.
        auto* Unit = World->SpawnActor<APolitikaPrototypeUnit>(PolitikaNavigationObstacleFixture::Start, FRotator::ZeroRotator);
        Unit->Configure(9901, 1, FText::FromString(TEXT("Schedule fixture")), 100, 210, 250, 10, 0.5f,
            true, nullptr, FVector(0.85f), FLinearColor::White);
        Unit->SetActorLocation(PolitikaNavigationObstacleFixture::Start);
        Match->Units = {Unit};
        Match->MatchGeneration = Schedule;
        Match->LocalRuntimePlayerId = 1;
        Match->PlayableMin = FVector2D(-2000, -2000);
        Match->PlayableMax = FVector2D(2000, 2000);
        Match->bContentReady = true;
        TestTrue(TEXT("Legal lifecycle reaches Active"), Match->TransitionLifecycle(MatchPhase::SetupValidated)
            && Match->TransitionLifecycle(MatchPhase::Loading) && Match->TransitionLifecycle(MatchPhase::Starting)
            && Match->TransitionLifecycle(MatchPhase::Active));
        TArray<FSample> Samples;
        TArray<FString> Events;
        TArray<FVector> Route;
        uint64 Arrival = 0;
        int32 Arrivals = 0;
        Match->GameplayEventPublished.AddLambda([&](const GameplayEvent& Event)
        {
            if (const auto* Changed = std::get_if<OrderChangedEvent>(&Event))
            {
                Events.Add(FString::Printf(TEXT("%llu:%u:%u:%u"), Changed->step,
                    static_cast<uint32>(Changed->previous), static_cast<uint32>(Changed->current), static_cast<uint32>(Changed->previous_end)));
                if (Changed->previous_end == OrderEndReason::Arrived) { Arrival = Changed->step; ++Arrivals; }
            }
        });
        Match->NavigationStepObserver = [&]
        {
            if (Route.IsEmpty()) Route = Unit->NavigationWaypoints;
            Samples.Add({Unit->GetActorLocation(), Unit->NavigationWaypointIndex, Unit->GetCurrentOrder()});
        };
        Match->SubmitMove(1, Unit, PolitikaNavigationObstacleFixture::Goal);
        for (uint64 Update = 0; Update < 6000 && (!Arrival || Match->SimulationStepsExecuted < Arrival + 20); ++Update)
            Match->Tick(PolitikaNavigationObstacleFixture::ScheduleDelta(Schedule, Update));
        Match->NavigationStepObserver = nullptr;
        Match->GameplayEventPublished.Clear();
        TestTrue(TEXT("Real Recast detour installed"), Route.Num() >= 3);
        TestTrue(TEXT("Arrival followed by 20 steps"), Arrival > 0 && Match->SimulationStepsExecuted >= Arrival + 20);
        TestEqual(TEXT("Single arrival publication"), Arrivals, 1);
        TestTrue(TEXT("Path retired"), Unit->NavigationWaypoints.IsEmpty() && Unit->NavigationWaypointIndex == 0);
        if (Schedule == 1)
        {
            Reference = Samples; ReferenceRoute = Route; ReferenceArrival = Arrival; ReferenceEvents = Events;
        }
        else
        {
            TestEqual(TEXT("Same arrival step"), Arrival, ReferenceArrival);
            TestTrue(TEXT("Same ordered events"), Events == ReferenceEvents);
            TestEqual(TEXT("Same route point count"), Route.Num(), ReferenceRoute.Num());
            for (int32 I = 0; I < FMath::Min(Route.Num(), ReferenceRoute.Num()); ++I)
                TestTrue(TEXT("Corresponding Recast points within 0.1 cm"), FVector::Distance(Route[I], ReferenceRoute[I]) <= 0.1);
            // A final multi-step outer update may overshoot the observation horizon.
            TestTrue(TEXT("Complete comparable fixed-step horizon"), Samples.Num() >= Reference.Num());
            for (int32 I = 0; I < FMath::Min(Reference.Num(), Samples.Num()); ++I)
                TestTrue(FString::Printf(TEXT("Equivalent state at authoritative step %d"), I + 1),
                    FVector::Distance(Samples[I].Position, Reference[I].Position) <= 0.1
                    && Samples[I].Cursor == Reference[I].Cursor && Samples[I].Order == Reference[I].Order);
        }
    }
    UEditorLoadingAndSavingUtils::LoadMap(TEXT("/Game/Maps/MainMenu"));
    return true;
}
#endif
