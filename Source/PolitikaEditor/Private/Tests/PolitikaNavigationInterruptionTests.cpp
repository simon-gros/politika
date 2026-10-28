#if WITH_DEV_AUTOMATION_TESTS
#include "PolitikaPrototypeGameplay.h"
#include "PolitikaNavigationObstacleFixture.h"
#include "FileHelpers.h"
#include "Engine/World.h"
#include "NavigationSystem.h"
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPolitikaNavigationInterruptionAutomationTest,
    "Politika.RTS.Navigation.SavedMapDetourOrderInterruption",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPolitikaNavigationInterruptionAutomationTest::RunTest(const FString& Parameters)
{
    using namespace politika::rules;
    struct FSample { FVector Position; TArray<FVector> Route; int32 Cursor; uint64 Sequence; EPolitikaPrototypeOrder Order; };
    for (int32 Schedule : {2, 5})
    {
        TArray<FSample> Reference;
        uint64 ReferenceArrival = 0;
        for (int32 Scenario = 0; Scenario <= 3; ++Scenario)
        {
            UWorld* World = UEditorLoadingAndSavingUtils::LoadMap(PolitikaNavigationObstacleFixture::Level);
            if (!TestNotNull(TEXT("Accepted saved map loads"), World)) return false;
            auto* Nav = FNavigationSystem::GetCurrent<UNavigationSystemV1>(World);
            if (!TestTrue(TEXT("Saved Recast ready"), Nav && Nav->GetDefaultNavDataInstance(FNavigationSystem::DontCreate)
                && !Nav->IsNavigationBuildInProgress())) return false;
            auto* Match = World->SpawnActor<APolitikaPrototypeMatch>();
            Match->SetActorTickEnabled(false);
            auto* Unit = World->SpawnActor<APolitikaPrototypeUnit>(PolitikaNavigationObstacleFixture::Start, FRotator::ZeroRotator);
            Unit->Configure(9901, 1, FText::FromString(TEXT("Interruption fixture")), 100, 210, 250, 10, 0.5f,
                true, nullptr, FVector(0.85f), FLinearColor::White);
            Unit->SetActorLocation(PolitikaNavigationObstacleFixture::Start);
            Match->Units = {Unit};
            Match->MatchGeneration = 1;
            Match->LocalRuntimePlayerId = 1;
            Match->PlayableMin = FVector2D(-2000, -2000);
            Match->PlayableMax = FVector2D(2000, 2000);
            Match->bContentReady = true;
            TestTrue(TEXT("Legal Active lifecycle"), Match->TransitionLifecycle(MatchPhase::SetupValidated)
                && Match->TransitionLifecycle(MatchPhase::Loading) && Match->TransitionLifecycle(MatchPhase::Starting)
                && Match->TransitionLifecycle(MatchPhase::Active));
            int32 Commands = 0, Stopped = 0, Replaced = 0, Arrivals = 0, Rejected = 0;
            uint64 Arrival = 0, Followup = 0;
            FVector Boundary;
            TArray<FSample> Samples;
            Match->GameplayEventPublished.AddLambda([&](const GameplayEvent& Event)
            {
                if (const auto* C = std::get_if<CommandOutcomeEvent>(&Event))
                {
                    ++Commands;
                    TestEqual(TEXT("Command processed at exact boundary"), C->step, Commands == 1 ? uint64(1) : Commands == 2 ? uint64(41) : uint64(61));
                    const bool Invalid = Scenario == 3 && Commands == 2;
                    TestEqual(TEXT("Typed admission result"), C->accepted, !Invalid);
                    TestTrue(TEXT("Exact rejection enum"), C->rejection == (Invalid ? OrderRejection::InvalidTarget : OrderRejection::None));
                    if (Invalid) ++Rejected;
                }
                if (const auto* E = std::get_if<OrderChangedEvent>(&Event))
                {
                    if (E->previous_end == OrderEndReason::Stopped) { ++Stopped; TestEqual(TEXT("Stop transition at 41"), E->step, uint64(41)); }
                    if (E->previous_end == OrderEndReason::Replaced) { ++Replaced; TestEqual(TEXT("Replacement at 41"), E->step, uint64(41)); }
                    if (E->previous_end == OrderEndReason::Arrived) { ++Arrivals; Arrival = E->step; }
                    TestTrue(TEXT("No unrelated termination"), E->previous_end == OrderEndReason::None || E->previous_end == OrderEndReason::Stopped
                        || E->previous_end == OrderEndReason::Replaced || E->previous_end == OrderEndReason::Arrived);
                }
            });
            Match->NavigationStepObserver = [&]
            {
                const uint64 Step = Match->SimulationStepsExecuted;
                Samples.Add({Unit->GetActorLocation(), Unit->NavigationWaypoints, Unit->NavigationWaypointIndex,
                    Unit->NavigationOrderSequence, Unit->CurrentOrder});
                if (Step == 40)
                {
                    TestTrue(TEXT("Original complete route still moving at 40"), Unit->CurrentOrder == EPolitikaPrototypeOrder::Move
                        && Unit->NavigationWaypoints.Num() >= 3 && Unit->NavigationOrderSequence == 1
                        && Unit->NavigationMatchGeneration == 1 && !Unit->GetActorLocation().Equals(PolitikaNavigationObstacleFixture::Start, 0.1)
                        && FVector::Dist2D(Unit->GetActorLocation(), Unit->ResolvedNavigationDestination) > 18 && Arrivals == 0);
                    Boundary = Unit->GetActorLocation();
                    Followup = Match->NextCommandSequence;
                    if (Scenario == 1) Match->SubmitStop(1, Unit);
                    if (Scenario == 2) Match->SubmitMove(1, Unit, PolitikaNavigationObstacleFixture::Start);
                    if (Scenario == 3) Match->SubmitMove(1, Unit, FVector(3000, 3000, 42));
                }
                if (Scenario == 1 && Step >= 41)
                {
                    TestTrue(TEXT("Stop stationary with retired identity"), Unit->GetActorLocation().Equals(Boundary, 0.001)
                        && Unit->NavigationWaypoints.IsEmpty() && Unit->NavigationWaypointIndex == 0
                        && Unit->NavigationOrderSequence == 0 && Unit->NavigationMatchGeneration == 0
                        && Unit->CurrentOrder == EPolitikaPrototypeOrder::Stop);
                    if (Step == 60) Match->SubmitStop(1, Unit);
                }
                if (Scenario == 2 && Step == 41)
                    TestTrue(TEXT("Fresh replacement route exclusively executable"), Unit->NavigationOrderSequence == Followup
                        && Unit->NavigationMatchGeneration == 1 && Unit->NavigationWaypoints.Num() >= 2
                        && Unit->ResolvedNavigationDestination.Equals(PolitikaNavigationObstacleFixture::Start, 0.1));
            };
            Match->SubmitMove(1, Unit, PolitikaNavigationObstacleFixture::Goal);
            for (uint64 Update = 0; Update < 6000 && (Scenario == 1 ? Match->SimulationStepsExecuted < 81 : !Arrival || Match->SimulationStepsExecuted < Arrival + 20); ++Update)
                Match->Tick(PolitikaNavigationObstacleFixture::ScheduleDelta(Schedule, Update));
            Match->NavigationStepObserver = nullptr;
            Match->GameplayEventPublished.Clear();
            TestEqual(TEXT("Exactly expected commands"), Commands, Scenario == 0 ? 1 : Scenario == 1 ? 3 : 2);
            TestEqual(TEXT("Exactly one Stop only for OI-01"), Stopped, Scenario == 1 ? 1 : 0);
            TestEqual(TEXT("Exactly one replacement only for OI-02"), Replaced, Scenario == 2 ? 1 : 0);
            TestEqual(TEXT("Exactly one rejection only for OI-03"), Rejected, Scenario == 3 ? 1 : 0);
            TestEqual(TEXT("Exactly one appropriate arrival, none after Stop"), Arrivals, Scenario == 1 ? 0 : 1);
            TestTrue(TEXT("Observation horizon reached"), Scenario == 1 ? Match->SimulationStepsExecuted >= 81 : Arrival && Match->SimulationStepsExecuted >= Arrival + 20);
            if (Scenario == 0) { Reference = Samples; ReferenceArrival = Arrival; }
            if (Scenario == 3)
            {
                TestEqual(TEXT("Rejected request preserves arrival step"), Arrival, ReferenceArrival);
                TestEqual(TEXT("Same full trace length"), Samples.Num(), Reference.Num());
                for (int32 I = 0; I < FMath::Min(Samples.Num(), Reference.Num()); ++I)
                    TestTrue(FString::Printf(TEXT("Original route/progress unchanged at step %d"), I + 1),
                        Samples[I].Position.Equals(Reference[I].Position, 0.1) && Samples[I].Route == Reference[I].Route
                        && Samples[I].Cursor == Reference[I].Cursor && Samples[I].Sequence == Reference[I].Sequence
                        && Samples[I].Order == Reference[I].Order);
            }
        }
    }
    UEditorLoadingAndSavingUtils::LoadMap(TEXT("/Game/Maps/MainMenu"));
    return true;
}
#endif
