#if WITH_DEV_AUTOMATION_TESTS

#include "Rules.h"
#include "PolitikaPrototypeGameplay.h"
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPolitikaCommandContractAutomationTest,
    "Politika.RTS.CommandContract.TypedAdmissionAndEvents",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPolitikaCommandContractAutomationTest::RunTest(const FString& Parameters)
{
    using namespace politika::rules;
    const Entity Issuer{17, 2, true, true, true, true, true, true};
    const Entity Hostile{23, 3, true, true, true, true, true, true};
    const MapBounds Bounds{1000, 1000};
    const CommandRequest Move{41, 1, 2, Issuer.id, 501, MoveOrder{{400, 600}}};
    const OrderResult Accepted = AdmitCommand(Move, 41, true, &Issuer, Bounds);
    TestTrue(TEXT("Current participant and movable issuer admit typed Move"), static_cast<bool>(Accepted));

    const CommandRequest FriendlyAttack{41, 2, 2, Issuer.id, 502, AttackOrder{Issuer.id}};
    const OrderResult Rejected = AdmitCommand(FriendlyAttack, 41, true, &Issuer, Bounds, &Issuer);
    TestEqual(TEXT("Friendly target is explicitly rejected"), Rejected.rejection, OrderRejection::FriendlyTarget);

    const CommandRequest Attack{41, 3, 2, Issuer.id, 503, AttackOrder{Hostile.id}};
    TestTrue(TEXT("Hostile target is admitted from authoritative target data"),
        static_cast<bool>(AdmitCommand(Attack, 41, true, &Issuer, Bounds, &Hostile)));
    const OrderResult Stale = AdmitCommand(Attack, 42, true, &Issuer, Bounds, &Hostile);
    TestEqual(TEXT("Cross-match request is rejected"), Stale.rejection, OrderRejection::StaleMatch);

    const GameplayEvent Outcome = CommandOutcomeEvent{9, Attack.sequence, Attack.correlation_id,
        Attack.issuer_entity_id, KindOf(Attack.intent), true, OrderRejection::None};
    const CommandOutcomeEvent* TypedOutcome = std::get_if<CommandOutcomeEvent>(&Outcome);
    TestNotNull(TEXT("Published command outcome retains its typed payload"), TypedOutcome);
    if (TypedOutcome)
    {
        TestEqual(TEXT("Outcome correlates to its request"), TypedOutcome->correlation_id, uint64(503));
        TestTrue(TEXT("Outcome distinguishes accepted command from world damage"), TypedOutcome->accepted);
        TestEqual(TEXT("Outcome identifies Attack intent"), TypedOutcome->order, OrderKind::Attack);
    }
    FPolitikaPrototypeGameplayEventPublished Observer;
    bool bObservedTypedOutcome = false;
    Observer.AddLambda([&bObservedTypedOutcome](const GameplayEvent& Published)
    {
        const auto* Payload = std::get_if<CommandOutcomeEvent>(&Published);
        bObservedTypedOutcome = Payload && Payload->accepted && Payload->correlation_id == 503;
    });
    Observer.Broadcast(Outcome);
    TestTrue(TEXT("Native observer receives the immutable typed event payload"), bObservedTypedOutcome);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPolitikaGameplayEventPublicationAutomationTest,
    "Politika.RTS.CommandContract.GameplayEventPublicationBroadcasts",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPolitikaGameplayEventPublicationAutomationTest::RunTest(const FString& Parameters)
{
    using namespace politika::rules;
    APolitikaPrototypeMatch* Match = NewObject<APolitikaPrototypeMatch>(GetTransientPackage());
    TestNotNull(TEXT("Coordinator can be constructed for publication test"), Match);
    if (!Match) return false;

    bool bReceived = false;
    uint64 ReceivedCorrelation = 0;
    OrderRejection ReceivedRejection = OrderRejection::None;
    Match->OnGameplayEventPublished().AddLambda([&](const GameplayEvent& Event)
    {
        if (const CommandOutcomeEvent* Outcome = std::get_if<CommandOutcomeEvent>(&Event))
        {
            bReceived = true;
            ReceivedCorrelation = Outcome->correlation_id;
            ReceivedRejection = Outcome->rejection;
        }
    });

    Match->PublishGameplayEvent(GameplayEvent{CommandOutcomeEvent{
        7, 13, 902, 44, OrderKind::Attack, false, OrderRejection::FriendlyTarget}});

    TestTrue(TEXT("Coordinator broadcasts a typed command outcome"), bReceived);
    TestEqual(TEXT("Broadcast preserves correlation identity"), ReceivedCorrelation, uint64(902));
    TestEqual(TEXT("Broadcast preserves rejection reason"), ReceivedRejection, OrderRejection::FriendlyTarget);
    TestEqual(TEXT("Published snapshot retains the broadcast event"), Match->GetPublishedEvents().Num(), 1);
    return true;
}

#endif
