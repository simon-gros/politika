#if WITH_DEV_AUTOMATION_TESTS

#include "PolitikaPrototypeGameplay.h"
#include "Engine/World.h"
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPolitikaNavigationUnavailableAutomationTest,
    "Politika.RTS.Navigation.MissingNavigationFailsClosed",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPolitikaNavigationUnavailableAutomationTest::RunTest(const FString& Parameters)
{
    using namespace politika::rules;
    const UWorld::InitializationValues InitValues = UWorld::InitializationValues()
        .AllowAudioPlayback(false).RequiresHitProxies(false).CreatePhysicsScene(false)
        .CreateNavigation(false).CreateAISystem(false).ShouldSimulatePhysics(false)
        .SetTransactional(false).CreateFXSystem(false);
    UWorld* World = UWorld::CreateWorld(EWorldType::Game, false,
        FName(TEXT("PolitikaNavigationUnavailableAutomationWorld")), GetTransientPackage(), false,
        ERHIFeatureLevel::Num, &InitValues);
    TestNotNull(TEXT("Transient navigation-negative world created"), World);
    if (!World) return false;

    APolitikaPrototypeMatch* Match = World->SpawnActor<APolitikaPrototypeMatch>();
    APolitikaPrototypeUnit* Unit = World->SpawnActor<APolitikaPrototypeUnit>(FVector(0, 0, 42), FRotator::ZeroRotator);
    TestNotNull(TEXT("Production match coordinator spawned"), Match);
    TestNotNull(TEXT("Production unit spawned"), Unit);
    if (!Match || !Unit)
    {
        World->DestroyWorld(false);
        return false;
    }
    Unit->Configure(8101, 1, FText::FromString(TEXT("Navigation fixture")), 100, 100,
        250, 10, 0.5f, true, nullptr, FVector::OneVector, FLinearColor::White);
    Match->Units = {Unit};
    Match->MatchGeneration = 88;
    Match->LocalRuntimePlayerId = 1;
    Match->PlayableMin = FVector2D(-1000, -1000);
    Match->PlayableMax = FVector2D(1000, 1000);
    Match->bContentReady = true;
    Match->TransitionLifecycle(MatchPhase::SetupValidated);
    Match->TransitionLifecycle(MatchPhase::Loading);
    Match->TransitionLifecycle(MatchPhase::Starting);
    const bool bActive = Match->TransitionLifecycle(MatchPhase::Active);
    TestTrue(TEXT("Coordinator is active before command"), bActive);

    const FVector Start = Unit->GetActorLocation();
    Match->SubmitMove(1, Unit, FVector(500, 0, 42));
    Match->Tick(0.05f);

    TestEqual(TEXT("Command admission still reports the typed request as accepted"),
        Match->PublishedEvents.IsEmpty() ? OrderRejection::None
            : std::get<CommandOutcomeEvent>(Match->PublishedEvents[0]).rejection,
        OrderRejection::None);
    TestEqual(TEXT("No NavMesh means no executable Move order"), Unit->GetCurrentOrder(), EPolitikaPrototypeOrder::Stop);
    TestTrue(TEXT("Unit does not fall back to direct-line movement"), Unit->GetActorLocation().Equals(Start));
    bool bExplicitNavigationFailure = false;
    for (const GameplayEvent& Event : Match->PublishedEvents)
    {
        if (const OrderChangedEvent* Changed = std::get_if<OrderChangedEvent>(&Event))
            bExplicitNavigationFailure |= Changed->previous_end == OrderEndReason::NavigationFailed;
    }
    TestTrue(TEXT("Unavailable route publishes an explicit navigation termination"), bExplicitNavigationFailure);

    World->DestroyWorld(false);
    return true;
}

#endif
