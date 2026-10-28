#if WITH_DEV_AUTOMATION_TESTS

#include "PolitikaPrototypeGameplay.h"
#include "PolitikaPrototypeContent.h"
#include "Engine/World.h"
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPolitikaMatchLifecycleAutomationTest,
    "Politika.RTS.MatchLifecycle.CoordinatorActiveOnlySimulationAndPendingCleanup",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPolitikaMatchLifecycleAutomationTest::RunTest(const FString& Parameters)
{
    using namespace politika::rules;
    const UWorld::InitializationValues InitValues = UWorld::InitializationValues()
        .AllowAudioPlayback(false).RequiresHitProxies(false).CreatePhysicsScene(false)
        .CreateNavigation(false).CreateAISystem(false).ShouldSimulatePhysics(false)
        .SetTransactional(false).CreateFXSystem(false);
    UWorld* World = UWorld::CreateWorld(EWorldType::Game, false,
        FName(TEXT("PolitikaMatchLifecycleAutomationWorld")), GetTransientPackage(), false,
        ERHIFeatureLevel::Num, &InitValues);
    TestNotNull(TEXT("Transient test world created"), World);
    if (!World) return false;
    APolitikaPrototypeMatch* Match = World->SpawnActor<APolitikaPrototypeMatch>();
    APolitikaPrototypeUnit* Unit = World->SpawnActor<APolitikaPrototypeUnit>(FVector(0, 0, 42), FRotator::ZeroRotator);
    TestNotNull(TEXT("Production coordinator spawned"), Match);
    TestNotNull(TEXT("Production unit spawned"), Unit);
    if (!Match || !Unit) { World->DestroyWorld(false); return false; }
    Unit->Configure(101, 1, FText::FromString(TEXT("Lifecycle fixture")), 100, 100,
        250, 10, 0.5f, true, nullptr, FVector::OneVector, FLinearColor::White);
    Match->Units = {Unit};
    Match->bContentReady = true;
    Match->MatchGeneration = 10;
    Match->LocalRuntimePlayerId = 1;
    Match->PlayableMin = FVector2D(-1000, -1000);
    Match->PlayableMax = FVector2D(1000, 1000);
    const bool bActivated = Match->TransitionLifecycle(MatchPhase::SetupValidated)
        && Match->TransitionLifecycle(MatchPhase::Loading)
        && Match->TransitionLifecycle(MatchPhase::Starting)
        && Match->TransitionLifecycle(MatchPhase::Active);
    TestTrue(TEXT("Coordinator activates only through admitted startup phases"), bActivated);
    if (bActivated)
    {
        Match->SubmitMove(1, Unit, FVector(500, 0, 42));
        Match->Tick(0.05f);
        TestEqual(TEXT("Active coordinator executes one fixed 20 Hz step"), Match->SimulationStepsExecuted, uint64(1));
        TestEqual(TEXT("Active positive control applies Move order"), Unit->GetCurrentOrder(), EPolitikaPrototypeOrder::Move);
    }
    Match->SubmitMove(1, Unit, FVector(-500, 0, 42));
    TestEqual(TEXT("Active request awaits authoritative admission"), Match->PendingOrders.Num(), 1);
    TestTrue(TEXT("Coordinator accepts legal terminal transition"), Match->TransitionLifecycle(MatchPhase::Terminal));
    TestEqual(TEXT("Leaving Active invalidates pending requests"), Match->PendingOrders.Num(), 0);
    const FVector PositionAtTerminal = Unit->GetActorLocation();
    const uint64 StepsAtTerminal = Match->SimulationStepsExecuted;
    Match->Tick(1.0f);
    TestEqual(TEXT("Terminal tick does not advance authoritative steps"), Match->SimulationStepsExecuted, StepsAtTerminal);
    TestTrue(TEXT("Terminal tick does not move units"), Unit->GetActorLocation().Equals(PositionAtTerminal));
    Match->SubmitMove(1, Unit, FVector(900, 900, 42));
    TestEqual(TEXT("Inactive command request is not retained"), Match->PendingOrders.Num(), 0);
    TestTrue(TEXT("Inactive command reports the MatchInactive rejection contract"),
        !Match->PublishedEvents.IsEmpty()
        && std::holds_alternative<CommandOutcomeEvent>(Match->PublishedEvents.Last())
        && std::get<CommandOutcomeEvent>(Match->PublishedEvents.Last()).rejection == OrderRejection::MatchInactive);
    Match->Tick(1.0f);
    TestEqual(TEXT("Non-active request is never executed"), Match->SimulationStepsExecuted, StepsAtTerminal);
    TestTrue(TEXT("Non-active request does not alter unit order"), Unit->GetCurrentOrder() == EPolitikaPrototypeOrder::Move);
    World->DestroyWorld(false);

    UWorld* ObjectiveWorld = UWorld::CreateWorld(EWorldType::Game, false,
        FName(TEXT("PolitikaObjectiveLifecycleAutomationWorld")), GetTransientPackage(), false,
        ERHIFeatureLevel::Num, &InitValues);
    TestNotNull(TEXT("Objective test world created"), ObjectiveWorld);
    if (!ObjectiveWorld) return false;
    APolitikaPrototypeMatch* ObjectiveMatch = ObjectiveWorld->SpawnActor<APolitikaPrototypeMatch>();
    APolitikaPrototypeUnit* Capturer = ObjectiveWorld->SpawnActor<APolitikaPrototypeUnit>(FVector(0, 0, 42), FRotator::ZeroRotator);
    APolitikaPrototypeObjective* Objective = ObjectiveWorld->SpawnActor<APolitikaPrototypeObjective>(FVector(0, 0, 42), FRotator::ZeroRotator);
    TestNotNull(TEXT("Production coordinator for objective scenario"), ObjectiveMatch);
    TestNotNull(TEXT("Eligible local player unit"), Capturer);
    TestNotNull(TEXT("Production control-point objective"), Objective);
    if (!ObjectiveMatch || !Capturer || !Objective) { ObjectiveWorld->DestroyWorld(false); return false; }
    Capturer->Configure(201, 7, FText::FromString(TEXT("Objective capturer")), 100, 100,
        250, 10, 0.5f, true, nullptr, FVector::OneVector, FLinearColor::White);
    FPolitikaPrototypeObjectiveDefinition ObjectiveDefinition;
    ObjectiveDefinition.StableId = TEXT("objective.lifecycle.test_control");
    ObjectiveDefinition.Type = EPolitikaPrototypeObjectiveType::ControlPoint;
    ObjectiveDefinition.Location = FVector(0, 0, 42);
    ObjectiveDefinition.CaptureRadiusCm = 100;
    ObjectiveDefinition.CaptureDurationSeconds = 0.1f;
    Objective->ConfigureFromDefinition(ObjectiveDefinition, 7);
    ObjectiveMatch->Units = {Capturer};
    ObjectiveMatch->Objective = Objective;
    ObjectiveMatch->bContentReady = true;
    ObjectiveMatch->LocalRuntimePlayerId = 7;
    ObjectiveMatch->PlayableMin = FVector2D(-1000, -1000);
    ObjectiveMatch->PlayableMax = FVector2D(1000, 1000);
    const bool bObjectiveActivated = ObjectiveMatch->TransitionLifecycle(MatchPhase::SetupValidated)
        && ObjectiveMatch->TransitionLifecycle(MatchPhase::Loading)
        && ObjectiveMatch->TransitionLifecycle(MatchPhase::Starting)
        && ObjectiveMatch->TransitionLifecycle(MatchPhase::Active);
    TestTrue(TEXT("Objective match reaches Active through legal setup transitions"), bObjectiveActivated);
    TestTrue(TEXT("Control-point objective starts neutral and incomplete"), !Objective->IsComplete() && FMath::IsNearlyZero(Objective->GetProgress()));
    int32 CompleteObjectivePublications = 0;
    ObjectiveMatch->OnGameplayEventPublished().AddLambda([&](const GameplayEvent& Event)
    {
        if (const ObjectiveChangedEvent* Changed = std::get_if<ObjectiveChangedEvent>(&Event);
            Changed && Changed->complete && Changed->progress_milli == 1000)
            ++CompleteObjectivePublications;
    });
    if (bObjectiveActivated)
    {
        ObjectiveMatch->Tick(0.05f);
        TestTrue(TEXT("One authoritative step advances capture by the configured rate"),
            FMath::IsNearlyEqual(Objective->GetProgress(), 0.5f) && ObjectiveMatch->GetMatchPhase() == MatchPhase::Active);
        ObjectiveMatch->Tick(0.05f);
        TestTrue(TEXT("Authoritative objective evaluator completes the control point"), Objective->IsComplete());
        TestEqual(TEXT("Objective progress reaches completion by its capture rule"), Objective->GetProgress(), 1.0f);
        TestTrue(TEXT("Objective completion transitions through Results"), ObjectiveMatch->GetMatchPhase() == MatchPhase::Results);
        TestTrue(TEXT("Result is committed with expected player and objective reason"),
            ObjectiveMatch->GetCommittedResult().IsSet()
            && ObjectiveMatch->GetCommittedResult()->WinningPlayerId == 7
            && ObjectiveMatch->GetCommittedResult()->Reason == FName(TEXT("objective_secured")));
        TestEqual(TEXT("Completion event is published exactly once to subscriber"), CompleteObjectivePublications, 1);
        int32 RetainedCompletionEvents = 0;
        for (const GameplayEvent& Published : ObjectiveMatch->GetPublishedEvents())
            if (const ObjectiveChangedEvent* Changed = std::get_if<ObjectiveChangedEvent>(&Published);
                Changed && Changed->complete && Changed->progress_milli == 1000)
                ++RetainedCompletionEvents;
        TestEqual(TEXT("Published event snapshot retains one completion payload"), RetainedCompletionEvents, 1);
        const uint64 TerminalStepCount = ObjectiveMatch->SimulationStepsExecuted;
        ObjectiveMatch->Tick(1.0f);
        TestEqual(TEXT("Subsequent rendered ticks do not advance authoritative steps"), ObjectiveMatch->SimulationStepsExecuted, TerminalStepCount);
        TestEqual(TEXT("Subsequent ticks do not republish completion"), CompleteObjectivePublications, 1);
        TestTrue(TEXT("Retained result remains singular and unchanged"), ObjectiveMatch->GetCommittedResult().IsSet()
            && ObjectiveMatch->GetCommittedResult()->WinningPlayerId == 7);
        ObjectiveMatch->BeginReturnToMenu();
        TestTrue(TEXT("Results return request enters Returning"), ObjectiveMatch->GetMatchPhase() == MatchPhase::Returning);
        ObjectiveMatch->BeginReturnToMenu();
        TestTrue(TEXT("Repeated return request does not create another transition"), ObjectiveMatch->GetMatchPhase() == MatchPhase::Returning);
    }
    ObjectiveWorld->DestroyWorld(false);
    return true;
}

#endif
