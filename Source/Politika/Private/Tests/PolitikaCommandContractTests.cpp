#if WITH_DEV_AUTOMATION_TESTS

#include "Rules.h"
#include "PolitikaPrototypeGameplay.h"
#include "Engine/World.h"
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPolitikaCommandCoordinatorAcceptanceAutomationTest,
    "Politika.RTS.CommandContract.CoordinatorRejectionsPreserveGameplayState",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPolitikaCommandCoordinatorAcceptanceAutomationTest::RunTest(const FString& Parameters)
{
    using namespace politika::rules;
    const UWorld::InitializationValues InitValues = UWorld::InitializationValues()
        .AllowAudioPlayback(false).RequiresHitProxies(false).CreatePhysicsScene(false)
        .CreateNavigation(false).CreateAISystem(false).ShouldSimulatePhysics(false)
        .SetTransactional(false).CreateFXSystem(false);
    UWorld* World = UWorld::CreateWorld(EWorldType::Game, false,
        FName(TEXT("PolitikaCommandRejectionAutomationWorld")), GetTransientPackage(), false,
        ERHIFeatureLevel::Num, &InitValues);
    TestNotNull(TEXT("Transient test world created"), World);
    if (!World) return false;

    APolitikaPrototypeMatch* Match = World->SpawnActor<APolitikaPrototypeMatch>();
    APolitikaPrototypeUnit* Issuer = World->SpawnActor<APolitikaPrototypeUnit>(FVector(0, 0, 42), FRotator::ZeroRotator);
    APolitikaPrototypeUnit* Hostile = World->SpawnActor<APolitikaPrototypeUnit>(FVector(120, 0, 42), FRotator::ZeroRotator);
    APolitikaPrototypeUnit* Friendly = World->SpawnActor<APolitikaPrototypeUnit>(FVector(60, 60, 42), FRotator::ZeroRotator);
    APolitikaPrototypeObjective* Objective = World->SpawnActor<APolitikaPrototypeObjective>(FVector(900, 900, 42), FRotator::ZeroRotator);
    TestNotNull(TEXT("Real match coordinator spawned"), Match);
    TestNotNull(TEXT("Real issuing unit spawned"), Issuer);
    TestNotNull(TEXT("Real hostile target spawned"), Hostile);
    TestNotNull(TEXT("Real friendly target spawned"), Friendly);
    TestNotNull(TEXT("Real objective spawned"), Objective);
    if (!Match || !Issuer || !Hostile || !Friendly || !Objective)
    {
        World->DestroyWorld(false);
        return false;
    }

    Match->MatchGeneration = 77;
    Match->LocalRuntimePlayerId = 1;
    Match->PlayableMin = FVector2D(-1000, -1000);
    Match->PlayableMax = FVector2D(1000, 1000);
    Match->bContentReady = true;
    Match->Lifecycle = {};
    TestTrue(TEXT("Fixture enters Active through legal startup phases"),
        Match->TransitionLifecycle(MatchPhase::SetupValidated)
        && Match->TransitionLifecycle(MatchPhase::Loading)
        && Match->TransitionLifecycle(MatchPhase::Starting)
        && Match->TransitionLifecycle(MatchPhase::Active));
    Match->Units = {Issuer, Hostile, Friendly};
    Match->Objective = Objective;
    Issuer->Configure(101, 1, FText::FromString(TEXT("Issuer")), 100, 100, 250, 10, 0.5f,
        true, nullptr, FVector::OneVector, FLinearColor::White);
    Hostile->Configure(202, 2, FText::FromString(TEXT("Hostile")), 100, 0, 250, 10, 0.5f,
        true, nullptr, FVector::OneVector, FLinearColor::Red);
    Friendly->Configure(303, 1, FText::FromString(TEXT("Friendly")), 100, 0, 250, 10, 0.5f,
        true, nullptr, FVector::OneVector, FLinearColor::Blue);

    struct FGameplaySnapshot
    {
        FVector IssuerPosition;
        FVector HostilePosition;
        FVector FriendlyPosition;
        int32 IssuerHealth = 0, HostileHealth = 0, FriendlyHealth = 0;
        float IssuerCooldown = 0, HostileCooldown = 0, IssuerPendingDamage = 0, HostilePendingDamage = 0;
        EPolitikaPrototypeOrder IssuerOrder = EPolitikaPrototypeOrder::Idle;
        APolitikaPrototypeUnit* IssuerTarget = nullptr;
        FVector IssuerDestination;
        float ObjectiveProgress = 0;
        bool bObjectiveContested = false, bObjectiveComplete = false;
        int32 EntityCount = 0, PendingCount = 0, PublishedCount = 0;
        uint64 SimulationSteps = 0;
    };

    const auto ResetState = [&]()
    {
        Match->MatchGeneration = 77;
        Match->LocalRuntimePlayerId = 1;
        Match->PlayableMin = FVector2D(-1000, -1000);
        Match->PlayableMax = FVector2D(1000, 1000);
        Match->bContentReady = true;
        Match->Lifecycle = {};
        Match->TransitionLifecycle(MatchPhase::SetupValidated);
        Match->TransitionLifecycle(MatchPhase::Loading);
        Match->TransitionLifecycle(MatchPhase::Starting);
        Match->TransitionLifecycle(MatchPhase::Active);
        Match->Units = {Issuer, Hostile, Friendly};
        Match->PendingOrders.Reset();
        Match->PendingGameplayEvents.Reset();
        Match->PublishedEvents.Reset();
        Match->SimulationStepsExecuted = 0;
        Issuer->SetActorLocation(FVector(0, 0, 42), false, nullptr, ETeleportType::TeleportPhysics);
        Hostile->SetActorLocation(FVector(120, 0, 42), false, nullptr, ETeleportType::TeleportPhysics);
        Friendly->SetActorLocation(FVector(60, 60, 42), false, nullptr, ETeleportType::TeleportPhysics);
        Issuer->Health = 100;
        Issuer->MoveSpeed = 100;
        Issuer->WeaponRange = 250;
        Issuer->WeaponDamage = 10;
        Issuer->WeaponCooldown = 0.5f;
        Issuer->CooldownRemaining = 0.2f;
        Issuer->PendingDamage = 4.0f;
        Issuer->SetAttackOrder(Hostile);
        Hostile->Health = 80;
        Hostile->CooldownRemaining = 0.3f;
        Hostile->PendingDamage = 6.0f;
        Hostile->SetGuardOrder();
        Friendly->Health = 90;
        Friendly->SetGuardOrder();
        Objective->Progress = 0.25f;
        Objective->bContested = true;
        Objective->bComplete = false;
    };

    const auto Capture = [&]()
    {
        FGameplaySnapshot State;
        State.IssuerPosition = Issuer->GetActorLocation();
        State.HostilePosition = Hostile->GetActorLocation();
        State.FriendlyPosition = Friendly->GetActorLocation();
        State.IssuerHealth = Issuer->Health;
        State.HostileHealth = Hostile->Health;
        State.FriendlyHealth = Friendly->Health;
        State.IssuerCooldown = Issuer->CooldownRemaining;
        State.HostileCooldown = Hostile->CooldownRemaining;
        State.IssuerPendingDamage = Issuer->PendingDamage;
        State.HostilePendingDamage = Hostile->PendingDamage;
        State.IssuerOrder = Issuer->CurrentOrder;
        State.IssuerTarget = Issuer->OrderTarget.Get();
        State.IssuerDestination = Issuer->OrderDestination;
        State.ObjectiveProgress = Objective->Progress;
        State.bObjectiveContested = Objective->bContested;
        State.bObjectiveComplete = Objective->bComplete;
        State.EntityCount = Match->Units.Num();
        State.PendingCount = Match->PendingOrders.Num();
        State.PublishedCount = Match->PublishedEvents.Num();
        State.SimulationSteps = Match->SimulationStepsExecuted;
        return State;
    };

    OrderRejection ExpectedRejection = OrderRejection::None;
    const auto AssertSnapshotUnchanged = [&](const TCHAR* Label, const FGameplaySnapshot& Before)
    {
        const FGameplaySnapshot After = Capture();
        const bool bUnchanged = Before.IssuerPosition.Equals(After.IssuerPosition, 0.001f)
            && Before.HostilePosition.Equals(After.HostilePosition, 0.001f)
            && Before.FriendlyPosition.Equals(After.FriendlyPosition, 0.001f)
            && Before.IssuerHealth == After.IssuerHealth && Before.HostileHealth == After.HostileHealth
            && Before.FriendlyHealth == After.FriendlyHealth
            && FMath::IsNearlyEqual(Before.IssuerCooldown, After.IssuerCooldown)
            && FMath::IsNearlyEqual(Before.HostileCooldown, After.HostileCooldown)
            && FMath::IsNearlyEqual(Before.IssuerPendingDamage, After.IssuerPendingDamage)
            && FMath::IsNearlyEqual(Before.HostilePendingDamage, After.HostilePendingDamage)
            && Before.IssuerOrder == After.IssuerOrder && Before.IssuerTarget == After.IssuerTarget
            && Before.IssuerDestination.Equals(After.IssuerDestination, 0.001f)
            && FMath::IsNearlyEqual(Before.ObjectiveProgress, After.ObjectiveProgress)
            && Before.bObjectiveContested == After.bObjectiveContested
            && Before.bObjectiveComplete == After.bObjectiveComplete
            && Before.EntityCount == After.EntityCount && After.PendingCount == 0
            && Before.PublishedCount == After.PublishedCount
            && Before.SimulationSteps == After.SimulationSteps;
        TestTrue(FString::Printf(TEXT("%s: gameplay snapshot unchanged and no pending execution"), Label), bUnchanged);
        TestEqual(FString::Printf(TEXT("%s: one diagnostic rejection event queued"), Label),
            Match->PendingGameplayEvents.Num(), 1);
        if (Match->PendingGameplayEvents.Num() == 1)
        {
            const CommandOutcomeEvent* Outcome = std::get_if<CommandOutcomeEvent>(&Match->PendingGameplayEvents[0]);
            TestNotNull(FString::Printf(TEXT("%s: queued result is typed"), Label), Outcome);
            if (Outcome)
            {
                TestFalse(FString::Printf(TEXT("%s: rejection is not recorded as accepted"), Label), Outcome->accepted);
                TestEqual(FString::Printf(TEXT("%s: typed rejection reason preserved"), Label), Outcome->rejection, ExpectedRejection);
            }
        }
    };

    const auto RunRejected = [&](const TCHAR* Label, uint32 PlayerId, APolitikaPrototypeUnit* CommandIssuer,
        APolitikaPrototypeUnit* Target, const Order& Intent, OrderRejection Expected,
        const TFunction<void()>& Mutate = TFunction<void()>(), bool bStaleGeneration = false)
    {
        ResetState();
        if (Mutate) Mutate();
        ExpectedRejection = Expected;
        const FGameplaySnapshot Before = Capture();
        Match->SubmitCommand(PlayerId, CommandIssuer, Target, Intent);
        if (bStaleGeneration) Match->PendingOrders.Last().Request.match_generation = 76;
        Match->AdmitPendingOrders();
        TestEqual(FString::Printf(TEXT("%s: request consumed by authoritative admission"), Label),
            Match->PendingOrders.Num(), 0);
        AssertSnapshotUnchanged(Label, Before);
        Match->PendingGameplayEvents.Reset();
    };

    RunRejected(TEXT("Missing issuer"), 1, nullptr, nullptr, MoveOrder{{100, 100}}, OrderRejection::MissingIssuer);
    RunRejected(TEXT("Unauthorized participant"), 2, Issuer, nullptr, MoveOrder{{100, 100}}, OrderRejection::Unauthorized);
    RunRejected(TEXT("Stale match generation"), 1, Issuer, nullptr, MoveOrder{{100, 100}},
        OrderRejection::StaleMatch, TFunction<void()>(), true);

    ResetState();
    Match->bContentReady = false;
    ExpectedRejection = OrderRejection::MatchInactive;
    {
        const FGameplaySnapshot Before = Capture();
        Match->SubmitCommand(1, Issuer, nullptr, MoveOrder{{100, 100}});
        Match->AdmitPendingOrders();
        AssertSnapshotUnchanged(TEXT("Inactive match"), Before);
    }

    ResetState();
    Issuer->Health = 0;
    ExpectedRejection = OrderRejection::IssuerUnavailable;
    {
        const FGameplaySnapshot Before = Capture();
        Match->SubmitCommand(1, Issuer, nullptr, MoveOrder{{100, 100}});
        Match->AdmitPendingOrders();
        AssertSnapshotUnchanged(TEXT("Destroyed issuer"), Before);
    }

    RunRejected(TEXT("Non-movable issuer"), 1, Issuer, nullptr, MoveOrder{{100, 100}},
        OrderRejection::CannotMove, [&]() { Issuer->MoveSpeed = 0; });
    RunRejected(TEXT("Non-attacking issuer"), 1, Issuer, Hostile, AttackOrder{202},
        OrderRejection::CannotAttack, [&]() { Issuer->WeaponDamage = 0; });
    RunRejected(TEXT("Invalid destination"), 1, Issuer, nullptr, MoveOrder{{2001, 100}}, OrderRejection::InvalidTarget);
    RunRejected(TEXT("Invalid map bounds"), 1, Issuer, nullptr, MoveOrder{{100, 100}},
        OrderRejection::InvalidBounds, [&]() { Match->PlayableMax.X = Match->PlayableMin.X; });
    RunRejected(TEXT("Missing target"), 1, Issuer, nullptr, AttackOrder{0}, OrderRejection::MissingTarget);
    RunRejected(TEXT("Friendly target"), 1, Issuer, Friendly, AttackOrder{303}, OrderRejection::FriendlyTarget);
    RunRejected(TEXT("Destroyed target"), 1, Issuer, Hostile, AttackOrder{202}, OrderRejection::MissingTarget,
        [&]() { Hostile->Health = 0; });
    RunRejected(TEXT("Non-attackable target"), 1, Issuer, Hostile, AttackOrder{202}, OrderRejection::TargetNotAttackable,
        [&]() { Hostile->bAttackable = false; });

    ResetState();
    Match->SubmitCommand(1, Issuer, nullptr, MoveOrder{{300, 250}});
    Match->SubmitCommand(1, Issuer, Friendly, AttackOrder{303});
    Match->AdmitPendingOrders();
    TestEqual(TEXT("Mixed request queue is fully consumed"), Match->PendingOrders.Num(), 0);
    TestEqual(TEXT("Later rejected request does not cancel earlier accepted Move"),
        Issuer->GetCurrentOrder(), EPolitikaPrototypeOrder::Move);
    TestTrue(TEXT("Earlier Move destination is the active execution state"),
        Issuer->OrderDestination.Equals(FVector(-700, -750, 42), 0.001f));
    TestEqual(TEXT("Mixed queue publishes command outcomes and one order transition"), Match->PendingGameplayEvents.Num(), 3);
    if (Match->PendingGameplayEvents.Num() == 3)
    {
        const CommandOutcomeEvent* First = std::get_if<CommandOutcomeEvent>(&Match->PendingGameplayEvents[0]);
        const OrderChangedEvent* Transition = std::get_if<OrderChangedEvent>(&Match->PendingGameplayEvents[1]);
        const CommandOutcomeEvent* Second = std::get_if<CommandOutcomeEvent>(&Match->PendingGameplayEvents[2]);
        TestNotNull(TEXT("First queued result is typed"), First);
        TestNotNull(TEXT("Accepted Move emits an order transition"), Transition);
        TestNotNull(TEXT("Second queued result is typed"), Second);
        if (First && Second)
        {
            TestTrue(TEXT("First Move is admitted"), First->accepted);
            TestFalse(TEXT("Second friendly attack is rejected"), Second->accepted);
            TestTrue(TEXT("Processing preserves submission sequence"), First->sequence < Second->sequence);
            TestEqual(TEXT("Second request reports friendly target"), Second->rejection, OrderRejection::FriendlyTarget);
        }
    }

    ResetState();
    Match->SubmitCommand(1, Issuer, nullptr, MoveOrder{{300, 250}});
    Match->SubmitCommand(1, Issuer, nullptr, GuardOrder{});
    Match->SubmitCommand(1, Issuer, Friendly, AttackOrder{303});
    Match->AdmitPendingOrders();
    TestEqual(TEXT("Three-request queue is consumed"), Match->PendingOrders.Num(), 0);
    TestEqual(TEXT("Last successfully admitted command becomes active in submission order"),
        Issuer->GetCurrentOrder(), EPolitikaPrototypeOrder::Guard);
    TestEqual(TEXT("Guard clears the replaced attack target"), Issuer->GetOrderTarget(), static_cast<APolitikaPrototypeUnit*>(nullptr));
    TestEqual(TEXT("Three-request queue publishes outcome and transitions"), Match->PendingGameplayEvents.Num(), 5);
    if (Match->PendingGameplayEvents.Num() == 5)
    {
        const CommandOutcomeEvent* First = std::get_if<CommandOutcomeEvent>(&Match->PendingGameplayEvents[0]);
        const CommandOutcomeEvent* Second = std::get_if<CommandOutcomeEvent>(&Match->PendingGameplayEvents[2]);
        const CommandOutcomeEvent* Third = std::get_if<CommandOutcomeEvent>(&Match->PendingGameplayEvents[4]);
        TestNotNull(TEXT("First request outcome exists"), First);
        TestNotNull(TEXT("Second request outcome exists"), Second);
        TestNotNull(TEXT("Third request outcome exists"), Third);
        if (First && Second && Third)
        {
            TestTrue(TEXT("Move is admitted before Guard"), First->accepted && First->sequence < Second->sequence);
            TestTrue(TEXT("Guard is admitted before the final rejection"), Second->accepted && Second->sequence < Third->sequence);
            TestFalse(TEXT("Final friendly Attack is rejected"), Third->accepted);
        }
    }

    ResetState();
    Match->SubmitCommand(1, Issuer, nullptr, MoveOrder{{100, 100}});
    Issuer->Destroy();
    Match->AdmitPendingOrders();
    TestEqual(TEXT("Destroyed issuer pending request is consumed"), Match->PendingOrders.Num(), 0);
    TestEqual(TEXT("Destroyed issuer is rejected as missing"), Match->PendingGameplayEvents.Num(), 1);
    if (Match->PendingGameplayEvents.Num() == 1)
    {
        const CommandOutcomeEvent* Outcome = std::get_if<CommandOutcomeEvent>(&Match->PendingGameplayEvents[0]);
        TestNotNull(TEXT("Destroyed-issuer result is typed"), Outcome);
        if (Outcome) TestEqual(TEXT("Destroyed issuer rejection reason"), Outcome->rejection, OrderRejection::MissingIssuer);
    }

    World->DestroyWorld(false);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPolitikaCommandRepeatabilityAutomationTest,
    "Politika.RTS.CommandContract.FixedStepScriptRepeatability",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPolitikaCommandRepeatabilityAutomationTest::RunTest(const FString& Parameters)
{
    using namespace politika::rules;
    struct FRunResult
    {
        FVector IssuerLocation;
        int32 TargetHealth = 0;
        EPolitikaPrototypeOrder FinalOrder = EPolitikaPrototypeOrder::Idle;
        uint64 Steps = 0;
        float CooldownRemaining = 0;
        float ObjectiveProgress = 0;
        bool bObjectiveComplete = false;
        int32 EntityCount = 0;
        TArray<FString> EventSignatures;
    };

    const auto RunScript = [&](const TArray<float>& FrameDeltas, const TCHAR* WorldName)
    {
        FRunResult Result;
        const UWorld::InitializationValues InitValues = UWorld::InitializationValues()
            .AllowAudioPlayback(false).RequiresHitProxies(false).CreatePhysicsScene(false)
            .CreateNavigation(false).CreateAISystem(false).ShouldSimulatePhysics(false)
            .SetTransactional(false).CreateFXSystem(false);
        UWorld* World = UWorld::CreateWorld(EWorldType::Game, false, FName(WorldName),
            GetTransientPackage(), false, ERHIFeatureLevel::Num, &InitValues);
        if (!World) return Result;
        APolitikaPrototypeMatch* Match = World->SpawnActor<APolitikaPrototypeMatch>();
        APolitikaPrototypeUnit* Issuer = World->SpawnActor<APolitikaPrototypeUnit>(FVector(100, 100, 42), FRotator::ZeroRotator);
        APolitikaPrototypeUnit* Target = World->SpawnActor<APolitikaPrototypeUnit>(FVector(400, 100, 42), FRotator::ZeroRotator);
        APolitikaPrototypeObjective* Objective = World->SpawnActor<APolitikaPrototypeObjective>(FVector(900, 900, 42), FRotator::ZeroRotator);
        if (!Match || !Issuer || !Target || !Objective)
        {
            World->DestroyWorld(false);
            return Result;
        }
        Match->MatchGeneration = 88;
        Match->LocalRuntimePlayerId = 1;
        Match->PlayableMin = FVector2D(0, 0);
        Match->PlayableMax = FVector2D(1000, 1000);
        Match->bContentReady = true;
        Match->Lifecycle = {};
        const bool bActivated = Match->TransitionLifecycle(MatchPhase::SetupValidated)
            && Match->TransitionLifecycle(MatchPhase::Loading)
            && Match->TransitionLifecycle(MatchPhase::Starting)
            && Match->TransitionLifecycle(MatchPhase::Active);
        if (!bActivated) { World->DestroyWorld(false); return Result; }
        Match->Units = {Issuer, Target};
        Match->Objective = Objective;
        Issuer->Configure(401, 1, FText::FromString(TEXT("Issuer")), 100, 100, 200, 5, 0.25f,
            true, nullptr, FVector::OneVector, FLinearColor::White);
        Target->Configure(402, 2, FText::FromString(TEXT("Target")), 100, 0, 200, 0, 0.25f,
            true, nullptr, FVector::OneVector, FLinearColor::Red);
        Objective->CaptureRadius = 75;
        Objective->CaptureDuration = 1;
        Objective->LocalRuntimePlayerId = 1;
        Objective->SetActorLocation(FVector(100, 100, 42));
        Match->OnGameplayEventPublished().AddLambda([&Result](const GameplayEvent& Event)
        {
            Result.EventSignatures.Add(std::visit([](const auto& Payload)
            {
                using T = std::decay_t<decltype(Payload)>;
                if constexpr (std::is_same_v<T, CommandOutcomeEvent>)
                    return FString::Printf(TEXT("C:%llu:%llu:%d:%u"), Payload.step, Payload.sequence,
                        Payload.accepted ? 1 : 0, static_cast<uint32>(Payload.rejection));
                else if constexpr (std::is_same_v<T, OrderChangedEvent>)
                    return FString::Printf(TEXT("O:%llu:%llu:%u:%u:%u"), Payload.step, Payload.entity_id,
                        static_cast<uint32>(Payload.previous), static_cast<uint32>(Payload.current),
                        static_cast<uint32>(Payload.previous_end));
                else if constexpr (std::is_same_v<T, UnitDamagedEvent>)
                    return FString::Printf(TEXT("D:%llu:%llu:%d"), Payload.step, Payload.entity_id, Payload.applied);
                else if constexpr (std::is_same_v<T, UnitDestroyedEvent>)
                    return FString::Printf(TEXT("X:%llu:%llu"), Payload.step, Payload.entity_id);
                else if constexpr (std::is_same_v<T, TargetLostEvent>)
                    return FString::Printf(TEXT("L:%llu:%llu:%llu"), Payload.step, Payload.entity_id, Payload.target_id);
                else
                    return FString::Printf(TEXT("B:%llu:%d:%d:%u"), Payload.step, Payload.contested ? 1 : 0,
                        Payload.complete ? 1 : 0, Payload.progress_milli);
            }, Event));
        });

        Match->SubmitMove(1, Issuer, FVector(800, 100, 42));
        int32 StepsAtAttack = 0;
        int32 StepsAtStop = 0;
        for (const float DeltaSeconds : FrameDeltas)
        {
            Match->Tick(DeltaSeconds);
            if (Match->SimulationStepsExecuted >= 10 && StepsAtAttack == 0)
            {
                StepsAtAttack = static_cast<int32>(Match->SimulationStepsExecuted);
                Match->SubmitAttack(1, Issuer, Target);
            }
            if (Match->SimulationStepsExecuted >= 20 && StepsAtStop == 0)
            {
                StepsAtStop = static_cast<int32>(Match->SimulationStepsExecuted);
                Match->SubmitStop(1, Issuer);
            }
        }
        Result.IssuerLocation = Issuer->GetActorLocation();
        Result.TargetHealth = Target->GetHealth();
        Result.FinalOrder = Issuer->GetCurrentOrder();
        Result.Steps = Match->SimulationStepsExecuted;
        Result.CooldownRemaining = Issuer->CooldownRemaining;
        Result.ObjectiveProgress = Objective->GetProgress();
        Result.bObjectiveComplete = Objective->IsComplete();
        Result.EntityCount = Match->Units.Num();
        World->DestroyWorld(false);
        return Result;
    };

    TArray<float> FineDeltas;
    for (int32 Index = 0; Index < 30; ++Index) FineDeltas.Add(0.05f);
    const FRunResult Fine = RunScript(FineDeltas, TEXT("PolitikaCommandRepeatabilityFine"));
    TArray<float> MediumDeltas;
    for (int32 Index = 0; Index < 15; ++Index) MediumDeltas.Add(0.1f);
    TArray<float> CoarseDeltas;
    for (int32 Index = 0; Index < 6; ++Index) CoarseDeltas.Add(0.25f);
    TArray<float> OverCapDeltas;
    for (int32 Index = 0; Index < 6; ++Index) OverCapDeltas.Add(0.5f);
    const FRunResult Medium = RunScript(MediumDeltas, TEXT("PolitikaCommandRepeatabilityMedium"));
    const FRunResult Coarse = RunScript(CoarseDeltas, TEXT("PolitikaCommandRepeatabilityCoarse"));
    const FRunResult OverCap = RunScript(OverCapDeltas, TEXT("PolitikaCommandRepeatabilityOverCap"));

    TestEqual(TEXT("Fine and medium frame schedules run same fixed-step count"), Fine.Steps, Medium.Steps);
    TestEqual(TEXT("Fine and capped-catchup schedules run same fixed-step count"), Fine.Steps, Coarse.Steps);
    TestEqual(TEXT("Input deltas over the accumulator cap preserve bounded catch-up"), Fine.Steps, OverCap.Steps);
    TestEqual(TEXT("Scripted steps reach expected count"), Fine.Steps, uint64(30));
    TestTrue(TEXT("Final movement positions agree across render schedules"), Fine.IssuerLocation.Equals(Medium.IssuerLocation, 0.1f)
        && Fine.IssuerLocation.Equals(Coarse.IssuerLocation, 0.1f)
        && Fine.IssuerLocation.Equals(OverCap.IssuerLocation, 0.1f));
    TestEqual(TEXT("Final target health agrees across render schedules"), Fine.TargetHealth, Medium.TargetHealth);
    TestEqual(TEXT("Coarse schedule target health agrees"), Fine.TargetHealth, Coarse.TargetHealth);
    TestEqual(TEXT("Over-cap schedule target health agrees"), Fine.TargetHealth, OverCap.TargetHealth);
    TestTrue(TEXT("Cooldown agrees across render schedules"), FMath::IsNearlyEqual(Fine.CooldownRemaining, Medium.CooldownRemaining)
        && FMath::IsNearlyEqual(Fine.CooldownRemaining, Coarse.CooldownRemaining)
        && FMath::IsNearlyEqual(Fine.CooldownRemaining, OverCap.CooldownRemaining));
    TestEqual(TEXT("Final execution order agrees across render schedules"), Fine.FinalOrder, Medium.FinalOrder);
    TestEqual(TEXT("Coarse schedule final execution order agrees"), Fine.FinalOrder, Coarse.FinalOrder);
    TestEqual(TEXT("Over-cap final execution order agrees"), Fine.FinalOrder, OverCap.FinalOrder);
    TestTrue(TEXT("Objective progress agrees across render schedules"), FMath::IsNearlyEqual(Fine.ObjectiveProgress, Medium.ObjectiveProgress)
        && FMath::IsNearlyEqual(Fine.ObjectiveProgress, Coarse.ObjectiveProgress)
        && FMath::IsNearlyEqual(Fine.ObjectiveProgress, OverCap.ObjectiveProgress));
    TestEqual(TEXT("Objective completion agrees"), Fine.bObjectiveComplete, Medium.bObjectiveComplete);
    TestEqual(TEXT("Coarse schedule objective completion agrees"), Fine.bObjectiveComplete, Coarse.bObjectiveComplete);
    TestEqual(TEXT("Over-cap schedule objective completion agrees"), Fine.bObjectiveComplete, OverCap.bObjectiveComplete);
    TestEqual(TEXT("Entity survival count agrees across render schedules"), Fine.EntityCount, Medium.EntityCount);
    TestEqual(TEXT("Coarse schedule entity survival count agrees"), Fine.EntityCount, Coarse.EntityCount);
    TestEqual(TEXT("Over-cap schedule entity survival count agrees"), Fine.EntityCount, OverCap.EntityCount);
    TestTrue(TEXT("Published event signatures agree across render schedules"), Fine.EventSignatures == Medium.EventSignatures
        && Fine.EventSignatures == Coarse.EventSignatures && Fine.EventSignatures == OverCap.EventSignatures);
    TestTrue(TEXT("Script applied damage and Stop through the real coordinator"), Fine.TargetHealth < 100
        && Fine.FinalOrder == EPolitikaPrototypeOrder::Stop);
    return true;
}

#endif
