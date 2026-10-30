#if WITH_DEV_AUTOMATION_TESTS

// Politika.RTS.Multiplayer.*: the production coordinator in an isolated world that owns a
// real listen-mode IpNetDriver (UWorld::Listen), so net mode, controller locality and
// relevancy follow the engine's listen-server rules. Connections are not opened here:
// remote controllers are server-side PlayerControllers without a ULocalPlayer, exactly
// as a listen server sees an admitted client. Two-process packaged runs (Tools/Shell
// rts_multiplayer) cover real sockets, RPC transport and replication to clients.

#include "PolitikaPrototypeGameplay.h"
#include "PolitikaPrototypeContent.h"
#include "PolitikaBootstrap.h"
#include "PolitikaInfantryBehavior.h"

#include "Camera/CameraComponent.h"
#include "Engine/Engine.h"
#include "Engine/NetDriver.h"
#include "Engine/World.h"
#include "HAL/PlatformTime.h"
#include "Misc/AutomationTest.h"

#include <limits>

using namespace politika::rules;

struct FPolitikaMultiplayerFixture
{
    explicit FPolitikaMultiplayerFixture(FAutomationTestBase& InTest) : Test(InTest) {}
    ~FPolitikaMultiplayerFixture() { Teardown(); }

    FAutomationTestBase& Test;
    UWorld* World = nullptr;
    bool bHasContext = false;
    APolitikaPrototypeMatch* Match = nullptr;
    APolitikaPrototypePlayerController* Host = nullptr;
    TArray<APolitikaPrototypePlayerController*> Remotes;
    TArray<CommandOutcomeEvent> Outcomes;
    TArray<TargetLostEvent> LostTargets;
    FName LayoutFailure;
    uint64 NextEntityId = 100;
    uint32 NextCorrelation = 1000;
    TMap<APolitikaPrototypePlayerController*, uint32> NextSequence;

    // Builds the coordinator from the authored multiplayer fixture (or another map),
    // optionally listening, and resolves the session layout through production code.
    bool Create(const TCHAR* Layout, bool bListen, FName MapId = TEXT("map.test.multiplayer_skirmish"),
        bool bDedicated = false)
    {
        const UWorld::InitializationValues InitValues = UWorld::InitializationValues()
            .AllowAudioPlayback(false).RequiresHitProxies(false).CreatePhysicsScene(false)
            .CreateNavigation(false).CreateAISystem(false).ShouldSimulatePhysics(false)
            .SetTransactional(false).CreateFXSystem(false);
        // Several fixtures run per session; destroyed worlds linger until garbage collection.
        World = UWorld::CreateWorld(EWorldType::Game, false, MakeUniqueObjectName(GetTransientPackage(),
            UWorld::StaticClass(), FName(TEXT("PolitikaMultiplayerAutomationWorld"))),
            GetTransientPackage(), false, ERHIFeatureLevel::Num, &InitValues);
        if (!Test.TestNotNull(TEXT("Isolated multiplayer world created"), World)) return false;
        FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
        Context.SetCurrentWorld(World);
        bHasContext = true;
        FURL Url;
        Url.AddOption(*FString::Printf(TEXT("PolitikaLayout=%s"), Layout));
        if (bListen)
        {
            Url.Port = 17800 + static_cast<int32>(FPlatformTime::Cycles() % 2000);
            World->URL = Url;
            if (!Test.TestTrue(TEXT("World listens through the engine IP net driver"), World->Listen(Url))) return false;
            Test.TestEqual(TEXT("Listening world reports listen-server net mode"),
                static_cast<int32>(World->GetNetMode()), static_cast<int32>(NM_ListenServer));
            Test.TestFalse(TEXT("Listen net driver uses generic replication (fog relevancy applies)"),
                World->GetNetDriver()->IsUsingIrisReplication());
        }
        else World->URL = Url;

        const UPolitikaPrototypeContentCatalog* Catalog = MapId == FName(TEXT("map.test.multiplayer_skirmish"))
            ? LoadObject<UPolitikaPrototypeContentCatalog>(nullptr,
                TEXT("/Game/Politika/TestFixtures/Multiplayer/DA_MultiplayerCatalog.DA_MultiplayerCatalog"))
            : LoadObject<UPolitikaPrototypeContentCatalog>(nullptr,
                TEXT("/Game/Politika/Definitions/DA_PrototypeContentCatalog.DA_PrototypeContentCatalog"));
        const UPolitikaPrototypeMapDefinition* Map = Catalog ? Catalog->FindMap(MapId) : nullptr;
        if (!Test.TestNotNull(TEXT("Authored map definition resolves"), Map)) return false;
        Match = World->SpawnActor<APolitikaPrototypeMatch>();
        if (!Test.TestNotNull(TEXT("Coordinator spawned"), Match)) return false;
        TArray<FPolitikaPrototypeRuntimeParticipant> Participants;
        uint32 LocalId = 0;
        FName Failure;
        if (!Test.TestTrue(TEXT("Authored slots build a runtime roster"),
            APolitikaPrototypeMatch::BuildRuntimeParticipantsFromSlots(*Map, Participants, LocalId, Failure))) return false;
        Match->RuntimeParticipants = Participants;
        Match->LocalRuntimePlayerId = LocalId;
        Match->UnalignedSystemAuthority = {TEXT("faction.unaligned"), true};
        // The real listen IpNetDriver stays in place; only the hosting policy differs.
        Match->bTestDedicatedHosting = bDedicated;
        if (!Match->ResolveSessionLayout(LayoutFailure)) return false;
        Match->PlayableMin = FVector2D(-2000, -2000);
        Match->PlayableMax = FVector2D(2000, 2000);
        Match->bContentReady = true;
        Match->MatchGeneration = 41;
        Match->OnGameplayEventPublished().AddLambda([this](const GameplayEvent& Event)
        {
            if (const auto* Outcome = std::get_if<CommandOutcomeEvent>(&Event)) Outcomes.Add(*Outcome);
            if (const auto* Lost = std::get_if<TargetLostEvent>(&Event)) LostTargets.Add(*Lost);
        });
        if (!Test.TestTrue(TEXT("Coordinator reaches Starting through legal lifecycle transitions"),
            Match->TransitionLifecycle(MatchPhase::SetupValidated) && Match->TransitionLifecycle(MatchPhase::Loading)
            && Match->TransitionLifecycle(MatchPhase::Starting))) return false;
        if (bListen) Match->bAwaitingNetworkParticipants = true;
        else Test.TestTrue(TEXT("Standalone coordinator enters Active"), Match->TransitionLifecycle(MatchPhase::Active));
        Match->SyncReplicatedParticipants();
        Host = World->SpawnActor<APolitikaPrototypePlayerController>();
        if (!Test.TestNotNull(TEXT("Host controller spawned"), Host)) return false;
        Host->MarkLocalControllerForTest();
        // Actors are not initialized for play in this world, so register the host as the
        // world's first player controller the way a launched game does.
        World->AddController(Host);
        return true;
    }

    APolitikaPrototypePlayerController* AddRemote()
    {
        APolitikaPrototypePlayerController* Remote = World->SpawnActor<APolitikaPrototypePlayerController>();
        if (Remote) Remotes.Add(Remote);
        return Remote;
    }

    bool BindAll()
    {
        FName Failure;
        if (!Match->BindController(Host, Failure)) return false;
        for (int32 Seat = 1; Seat < Match->GetSessionSeatCount(); ++Seat)
        {
            APolitikaPrototypePlayerController* Remote = AddRemote();
            if (!Remote || !Match->BindController(Remote, Failure)) return false;
        }
        return Match->GetMatchPhase() == MatchPhase::Active;
    }

    // Owner 0 spawns an Unaligned unit on its own NPC side.
    APolitikaPrototypeUnit* AddUnit(uint32 Owner, const FVector2D& Location, float Range = 600.0f)
    {
        APolitikaPrototypeUnit* Unit = World->SpawnActor<APolitikaPrototypeUnit>(FVector(Location.X, Location.Y, 42), FRotator::ZeroRotator);
        if (!Unit) return nullptr;
        const FPolitikaPrototypeRuntimeParticipant* Participant = Match->RuntimeParticipants.FindByPredicate(
            [Owner](const FPolitikaPrototypeRuntimeParticipant& Candidate) { return Candidate.RuntimePlayerId == Owner; });
        Unit->Configure(NextEntityId++, Owner, FText::FromString(TEXT("Multiplayer test unit")), 100, 300, Range, 10, 0.5f,
            true, nullptr, FVector::OneVector, FLinearColor::White,
            Participant ? Participant->FactionId : FName(TEXT("faction.unaligned")));
        Unit->SetTeamId(Participant ? Participant->TeamId : kUnalignedTeam);
        Unit->SetSightRadius(1200.0f);
        Unit->AuthorityMatch = Match;
        Match->Units.Add(Unit);
        return Unit;
    }

    FPolitikaNetCommandIntent Intent(APolitikaPrototypePlayerController* Source, OrderKind Kind, uint64 Issuer,
        uint64 Target = 0, double X = 0.0, double Y = 0.0)
    {
        FPolitikaNetCommandIntent Result;
        Result.ProtocolVersion = static_cast<int32>(FPolitikaBootstrap::Get().NetworkProtocol());
        Result.OrderKind = static_cast<uint8>(Kind);
        Result.IssuerEntityId = Issuer;
        Result.TargetEntityId = Target;
        Result.DestinationX = X;
        Result.DestinationY = Y;
        uint32& Sequence = NextSequence.FindOrAdd(Source, 1);
        Result.ClientSequence = Sequence++;
        Result.CorrelationId = NextCorrelation++;
        Result.MatchGeneration = Match->GetMatchGeneration();
        return Result;
    }

    const CommandOutcomeEvent* Outcome(uint32 Correlation) const
    {
        return Outcomes.FindByPredicate([Correlation](const CommandOutcomeEvent& Event) { return Event.correlation_id == Correlation; });
    }

    // Submits through the single production intake and runs one authoritative step.
    const CommandOutcomeEvent* Submit(APolitikaPrototypePlayerController* Source, const FPolitikaNetCommandIntent& Payload)
    {
        Match->SubmitControllerIntent(Source, Payload);
        Step();
        return Outcome(Payload.CorrelationId);
    }

    void Step(int32 Steps = 1) { for (int32 Index = 0; Index < Steps; ++Index) Match->Tick(0.05f); }
    void RefreshVision() { Match->UpdateTeamVisibility(); }

    bool Relevant(const APolitikaPrototypeUnit* Unit, const APlayerController* Viewer) const
    {
        return Unit->IsNetRelevantFor(Viewer, Viewer, FVector::ZeroVector);
    }

    void Teardown()
    {
        if (!World) return;
        if (World->GetNetDriver()) GEngine->DestroyNamedNetDriver(World, NAME_GameNetDriver);
        World->DestroyWorld(false);
        if (bHasContext) GEngine->DestroyWorldContext(World);
        World = nullptr;
        Match = nullptr;
    }
};

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPolitikaMultiplayerParticipantBindingTest,
    "Politika.RTS.Multiplayer.ParticipantBindingAndLayouts",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPolitikaMultiplayerParticipantBindingTest::RunTest(const FString& Parameters)
{
    // MP1-01/02/03 for every supported layout: the host binds the authored local-human
    // seat, remotes fill seats in seat order with alternating teams, the match activates
    // only when every seat is bound, and a further connection is refused.
    const TCHAR* Layouts[] = {TEXT("1v1"), TEXT("2v1"), TEXT("3v1"), TEXT("2v2")};
    for (const TCHAR* LayoutText : Layouts)
    {
        FPolitikaMultiplayerFixture F(*this);
        if (!F.Create(LayoutText, true)) return false;
        TeamLayout Expected = TeamLayout::OneVsOne;
        TestTrue(TEXT("Layout text parses"), ParseTeamLayout(TCHAR_TO_UTF8(LayoutText), Expected));
        const int32 Seats = SeatCount(Expected);
        TestEqual(FString::Printf(TEXT("%s: seat count"), LayoutText), F.Match->GetSessionSeatCount(), Seats);
        TestEqual(FString::Printf(TEXT("%s: only layout seats play"), LayoutText), F.Match->RuntimeParticipants.Num(), Seats);
        TestTrue(FString::Printf(TEXT("%s: fog of war is enforced in networked sessions"), LayoutText), F.Match->IsFogOfWarEnabled());
        for (int32 Seat = 0; Seat < Seats; ++Seat)
            TestEqual(FString::Printf(TEXT("%s: seat %d team"), LayoutText, Seat),
                F.Match->RuntimeParticipants[Seat].TeamId, TeamForSeat(Expected, static_cast<uint8>(Seat)));
        TestFalse(FString::Printf(TEXT("%s: no seat is the Unaligned"), LayoutText),
            F.Match->RuntimeParticipants.ContainsByPredicate([](const FPolitikaPrototypeRuntimeParticipant& P)
            { return P.FactionId == TEXT("faction.unaligned") || P.TeamId == kUnalignedTeam; }));

        FName Failure;
        TestTrue(FString::Printf(TEXT("%s: host binds"), LayoutText), F.Match->BindController(F.Host, Failure));
        TestEqual(FString::Printf(TEXT("%s: MP1-01 host holds participant 1"), LayoutText), F.Match->GetBoundParticipantId(F.Host), 1u);
        TestEqual(FString::Printf(TEXT("%s: host on team A"), LayoutText), F.Match->GetTeamForParticipant(1), kTeamA);
        TestEqual(FString::Printf(TEXT("%s: host assignment replicated to owner"), LayoutText), F.Host->GetAssignedRuntimePlayerId(), 1u);
        for (int32 Seat = 1; Seat < Seats; ++Seat)
        {
            TestEqual(FString::Printf(TEXT("%s: Starting until every seat is bound"), LayoutText),
                F.Match->GetMatchPhase(), MatchPhase::Starting);
            APolitikaPrototypePlayerController* Remote = F.AddRemote();
            TestFalse(TEXT("Spawned server-side controller is remote"), Remote->IsLocalSeatController());
            TestTrue(FString::Printf(TEXT("%s: open seat admits"), LayoutText), F.Match->CanAdmitRemoteParticipant(Failure));
            TestTrue(FString::Printf(TEXT("%s: remote %d binds"), LayoutText, Seat), F.Match->BindController(Remote, Failure));
            const uint32 ExpectedId = F.Match->RuntimeParticipants[Seat].RuntimePlayerId;
            TestEqual(FString::Printf(TEXT("%s: MP1-02 remote %d takes seat-order participant"), LayoutText, Seat),
                F.Match->GetBoundParticipantId(Remote), ExpectedId);
            TestEqual(TEXT("Owner-only assignment"), Remote->GetAssignedRuntimePlayerId(), ExpectedId);
            TestEqual(TEXT("Seat controller kind becomes RemoteHuman"),
                F.Match->RuntimeParticipants[Seat].ControllerKind, EPolitikaPrototypeControllerKind::RemoteHuman);
        }
        TestEqual(FString::Printf(TEXT("%s: match activates when all seats are bound"), LayoutText),
            F.Match->GetMatchPhase(), MatchPhase::Active);
        APolitikaPrototypePlayerController* Extra = F.AddRemote();
        TestFalse(FString::Printf(TEXT("%s: MP1-03 extra connection cannot be admitted"), LayoutText),
            F.Match->CanAdmitRemoteParticipant(Failure));
        TestFalse(FString::Printf(TEXT("%s: extra connection is not bound"), LayoutText), F.Match->BindController(Extra, Failure));
        TestEqual(TEXT("No binding or authority for the refused connection"), F.Match->GetBoundParticipantId(Extra), 0u);
        TestEqual(TEXT("Replicated roster carries every seat with its team"), F.Match->ReplicatedParticipants.Num(), Seats);
    }

    {
        // A departure before Active reopens exactly that seat for the next connection.
        FPolitikaMultiplayerFixture F(*this);
        if (!F.Create(TEXT("2v2"), true)) return false;
        FName Failure;
        F.Match->BindController(F.Host, Failure);
        APolitikaPrototypePlayerController* First = F.AddRemote();
        F.Match->BindController(First, Failure);
        const uint32 Seat2 = F.Match->GetBoundParticipantId(First);
        F.Match->HandleControllerLogout(First);
        TestEqual(TEXT("Logout revokes the binding"), F.Match->GetBoundParticipantId(First), 0u);
        TestEqual(TEXT("Logout clears the owner-only assignment"), First->GetAssignedRuntimePlayerId(), 0u);
        TestEqual(TEXT("Pre-active logout does not end the match"), F.Match->GetMatchPhase(), MatchPhase::Starting);
        APolitikaPrototypePlayerController* Replacement = F.AddRemote();
        TestTrue(TEXT("Reopened seat admits a new connection"), F.Match->BindController(Replacement, Failure));
        TestEqual(TEXT("New connection receives the reopened seat"), F.Match->GetBoundParticipantId(Replacement), Seat2);
    }

    {
        // Layout failures fail closed before anything spawns.
        FPolitikaMultiplayerFixture Invalid(*this);
        TestFalse(TEXT("Unknown layout is rejected"), Invalid.Create(TEXT("5v5"), true));
        TestEqual(TEXT("Unknown layout reason"), Invalid.LayoutFailure, FName(TEXT("MP.LAYOUT.INVALID")));
        FPolitikaMultiplayerFixture TooLarge(*this);
        TestFalse(TEXT("2v2 on the two-slot canonical map is rejected"),
            TooLarge.Create(TEXT("2v2"), true, UPolitikaPrototypeContentValidator::PrototypeMapAssetId));
        TestEqual(TEXT("Unsupported layout reason"), TooLarge.LayoutFailure, FName(TEXT("MAP.SLOTS.LAYOUT_UNSUPPORTED")));
    }

    {
        // MP1-04: standalone keeps the implicit local seat, every authored participant
        // plays as its own team, fog is off, and no other controller gains authority.
        FPolitikaMultiplayerFixture F(*this);
        if (!F.Create(TEXT("1v1"), false)) return false;
        TestEqual(TEXT("Standalone net mode"), static_cast<int32>(F.World->GetNetMode()), static_cast<int32>(NM_Standalone));
        TestFalse(TEXT("Standalone has no fog of war"), F.Match->IsFogOfWarEnabled());
        TestEqual(TEXT("Standalone plays every authored slot"), F.Match->RuntimeParticipants.Num(), 4);
        TSet<uint8> Teams;
        for (const FPolitikaPrototypeRuntimeParticipant& P : F.Match->RuntimeParticipants) Teams.Add(P.TeamId);
        TestEqual(TEXT("Standalone participants are each their own team"), Teams.Num(), 4);
        TestEqual(TEXT("Standalone local controller holds the authored human seat"), F.Match->GetBoundParticipantId(F.Host), 1u);
        APolitikaPrototypePlayerController* Second = F.World->SpawnActor<APolitikaPrototypePlayerController>();
        TestEqual(TEXT("Standalone second controller has no authority"), F.Match->GetBoundParticipantId(Second), 0u);
        FName Failure;
        TestFalse(TEXT("Standalone never admits remote participants"), F.Match->CanAdmitRemoteParticipant(Failure));
    }
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPolitikaMultiplayerCommandIntakeTest,
    "Politika.RTS.Multiplayer.CommandIntakeAndHostileInput",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPolitikaMultiplayerCommandIntakeTest::RunTest(const FString& Parameters)
{
    FPolitikaMultiplayerFixture F(*this);
    if (!F.Create(TEXT("2v2"), true) || !TestTrue(TEXT("All 2v2 seats bound"), F.BindAll())) return false;
    APolitikaPrototypePlayerController* P2 = F.Remotes[0];
    APolitikaPrototypePlayerController* P3 = F.Remotes[1];
    APolitikaPrototypePlayerController* Unbound = F.AddRemote();
    APolitikaPrototypeUnit* H = F.AddUnit(1, {-500, 0});
    APolitikaPrototypeUnit* Ally = F.AddUnit(3, {-450, 120});
    APolitikaPrototypeUnit* Enemy = F.AddUnit(2, {-150, 0});
    APolitikaPrototypeUnit* Unaligned = F.AddUnit(0, {-420, -160});
    if (!H || !Ally || !Enemy || !Unaligned) return false;
    F.RefreshVision();
    const FVector EnemyStart = Enemy->GetActorLocation();

    // MP1-05: a valid remote intent reaches the shared admission with server-derived authority.
    const CommandOutcomeEvent* Valid = F.Submit(P2, F.Intent(P2, OrderKind::Guard, Enemy->GetStableId()));
    TestTrue(TEXT("MP1-05 valid remote Guard is admitted"), Valid && Valid->accepted);
    TestEqual(TEXT("Admitted order applied by authority"), Enemy->GetCurrentOrder(), EPolitikaPrototypeOrder::Guard);
    TestEqual(TEXT("Outcome route consumed after delivery"), F.Match->OutcomeRoutes.Num(), 0);

    // MP1-06/07: issuers the connection does not own, regardless of payload content.
    auto ExpectRejected = [&](const TCHAR* Label, const CommandOutcomeEvent* Event, OrderRejection Expected)
    {
        TestTrue(FString::Printf(TEXT("%s produces a correlated outcome"), Label), Event != nullptr);
        if (!Event) return;
        TestFalse(FString::Printf(TEXT("%s is rejected"), Label), Event->accepted);
        TestEqual(FString::Printf(TEXT("%s reason"), Label), static_cast<int32>(Event->rejection), static_cast<int32>(Expected));
    };
    const EPolitikaPrototypeOrder HostOrder = H->GetCurrentOrder();
    ExpectRejected(TEXT("MP1-06 remote ordering the host's unit"), F.Submit(P2, F.Intent(P2, OrderKind::Stop, H->GetStableId())),
        OrderRejection::NotOwned);
    ExpectRejected(TEXT("Ally ordering a teammate's unit"), F.Submit(P3, F.Intent(P3, OrderKind::Stop, H->GetStableId())),
        OrderRejection::NotOwned);
    ExpectRejected(TEXT("Host ordering a remote's unit"), F.Submit(F.Host, F.Intent(F.Host, OrderKind::Stop, Enemy->GetStableId())),
        OrderRejection::NotOwned);
    ExpectRejected(TEXT("Remote ordering an Unaligned unit"), F.Submit(P2, F.Intent(P2, OrderKind::Guard, Unaligned->GetStableId())),
        OrderRejection::NotOwned);
    ExpectRejected(TEXT("Unknown issuer ID"), F.Submit(P2, F.Intent(P2, OrderKind::Guard, 999999)), OrderRejection::MissingIssuer);
    TestEqual(TEXT("MP1-07 rejected foreign commands leave the host unit unchanged"), H->GetCurrentOrder(), HostOrder);
    // The production payload has no participant field; the lower-level trusted boundary
    // still rejects a forged participant for an owned unit.
    F.Match->SubmitCommand(3, Enemy, nullptr, GuardOrder{}, 0, 4242);
    F.Step();
    ExpectRejected(TEXT("MP1-07 forged participant at the trusted boundary"), F.Outcome(4242), OrderRejection::NotOwned);
    TestFalse(TEXT("Unbound connection creates no command"), F.Match->SubmitControllerIntent(Unbound,
        F.Intent(Unbound, OrderKind::Guard, Enemy->GetStableId())));

    // MP1-08: malformed payloads never reach typed admission.
    FPolitikaNetCommandIntent BadKind = F.Intent(P2, OrderKind::Guard, Enemy->GetStableId());
    BadKind.OrderKind = 200;
    ExpectRejected(TEXT("MP1-08 invalid order kind"), F.Submit(P2, BadKind), OrderRejection::MalformedCommand);
    ExpectRejected(TEXT("MP1-08 non-finite destination"), F.Submit(P2, F.Intent(P2, OrderKind::Move, Enemy->GetStableId(), 0,
        std::numeric_limits<double>::quiet_NaN(), 1.0)), OrderRejection::MalformedCommand);
    ExpectRejected(TEXT("MP1-08 target on a Stop order"), F.Submit(P2, F.Intent(P2, OrderKind::Stop, Enemy->GetStableId(),
        H->GetStableId())), OrderRejection::MalformedCommand);
    ExpectRejected(TEXT("MP1-08 out-of-bounds destination"), F.Submit(P2, F.Intent(P2, OrderKind::Move, Enemy->GetStableId(), 0,
        1.0e6, 1.0e6)), OrderRejection::InvalidTarget);
    FPolitikaNetCommandIntent WrongProtocol = F.Intent(P2, OrderKind::Guard, Enemy->GetStableId());
    WrongProtocol.ProtocolVersion += 99;
    ExpectRejected(TEXT("MP1-08 protocol mismatch"), F.Submit(P2, WrongProtocol), OrderRejection::MalformedCommand);

    // MP1-09: replayed, reordered, stale or flooded commands cannot repeat effects.
    FPolitikaNetCommandIntent Once = F.Intent(P2, OrderKind::Stop, Enemy->GetStableId());
    const CommandOutcomeEvent* First = F.Submit(P2, Once);
    TestTrue(TEXT("Original command admitted"), First && First->accepted);
    FPolitikaNetCommandIntent Replay = Once;
    Replay.CorrelationId = F.NextCorrelation++;
    ExpectRejected(TEXT("MP1-09 exact replay"), F.Submit(P2, Replay), OrderRejection::DuplicateCommand);
    FPolitikaNetCommandIntent Older = Once;
    Older.ClientSequence -= 1;
    Older.CorrelationId = F.NextCorrelation++;
    ExpectRejected(TEXT("MP1-09 reordered older sequence"), F.Submit(P2, Older), OrderRejection::DuplicateCommand);
    FPolitikaNetCommandIntent Stale = F.Intent(P2, OrderKind::Guard, Enemy->GetStableId());
    Stale.MatchGeneration -= 1;
    ExpectRejected(TEXT("MP1-09 earlier match generation"), F.Submit(P2, Stale), OrderRejection::StaleMatch);
    // The bound is one pending command per living unit plus a fixed allowance (group orders).
    int32 P2LivingUnits = 0;
    for (const APolitikaPrototypeUnit* Unit : F.Match->GetUnits())
        P2LivingUnits += IsValid(Unit) && Unit->IsAlive() && Unit->GetOwnerPlayer() == Enemy->GetOwnerPlayer() ? 1 : 0;
    const int32 FloodLimit = politika::selection::PendingCommandLimit(P2LivingUnits);
    TestEqual(TEXT("Queue bound covers every living unit"), FloodLimit, 16 + P2LivingUnits);
    uint32 LastAdmittedFloodCorrelation = 0;
    uint32 LastFloodCorrelation = 0;
    for (int32 Index = 0; Index <= FloodLimit; ++Index)
    {
        const FPolitikaNetCommandIntent Flood = F.Intent(P2, OrderKind::Guard, Enemy->GetStableId());
        if (Index < FloodLimit) LastAdmittedFloodCorrelation = Flood.CorrelationId;
        LastFloodCorrelation = Flood.CorrelationId;
        F.Match->SubmitControllerIntent(P2, Flood);
    }
    TestNull(TEXT("MP1-09 commands within the bound are queued"), F.Outcome(LastAdmittedFloodCorrelation));
    ExpectRejected(TEXT("MP1-09 per-participant queue bound"), F.Outcome(LastFloodCorrelation), OrderRejection::RateLimited);
    F.Step();

    // Alliances and the Unaligned side through the same admission.
    ExpectRejected(TEXT("Attacking an allied unit"), F.Submit(F.Host, F.Intent(F.Host, OrderKind::Attack, H->GetStableId(),
        Ally->GetStableId())), OrderRejection::FriendlyTarget);
    ExpectRejected(TEXT("Seat 3 attacking its ally's unit"), F.Submit(P3, F.Intent(P3, OrderKind::Attack, Ally->GetStableId(),
        H->GetStableId())), OrderRejection::FriendlyTarget);
    ExpectRejected(TEXT("Seat 3 grenading its ally's unit"), F.Submit(P3, F.Intent(P3, OrderKind::Grenade, Ally->GetStableId(),
        H->GetStableId())), OrderRejection::FriendlyTarget);
    const CommandOutcomeEvent* Explicit = F.Submit(F.Host, F.Intent(F.Host, OrderKind::Attack, H->GetStableId(), Unaligned->GetStableId()));
    TestTrue(TEXT("An explicit attack on a visible Unaligned unit is legal"), Explicit && Explicit->accepted);
    const CommandOutcomeEvent* HostLocal = nullptr;
    // The adapter uses the controller's own client sequence; continue after the probes above.
    F.Host->NextClientSequence = F.NextSequence.FindOrAdd(F.Host, 1);
    F.Match->SubmitLocalControllerCommand(F.Host, H, nullptr, StopOrder{});
    F.Step();
    HostLocal = F.Outcomes.Num() ? &F.Outcomes.Last() : nullptr;
    TestTrue(TEXT("Host-local input uses the same intake and is admitted once"), HostLocal && HostLocal->accepted
        && HostLocal->issuer_id == H->GetStableId());

    // Commands after termination do not apply.
    TestTrue(TEXT("Match terminates"), F.Match->TransitionLifecycle(MatchPhase::Terminal)
        && F.Match->TransitionLifecycle(MatchPhase::Results));
    ExpectRejected(TEXT("Command after termination"), F.Submit(P2, F.Intent(P2, OrderKind::Guard, Enemy->GetStableId())),
        OrderRejection::MatchInactive);
    TestEqual(TEXT("Rejected commands never moved the remote unit"), Enemy->GetActorLocation(), EnemyStart);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPolitikaMultiplayerFogOfWarTest,
    "Politika.RTS.Multiplayer.FogOfWarVisibilityAndRelevancy",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPolitikaMultiplayerFogOfWarTest::RunTest(const FString& Parameters)
{
    FPolitikaMultiplayerFixture F(*this);
    if (!F.Create(TEXT("2v2"), true) || !TestTrue(TEXT("All 2v2 seats bound"), F.BindAll())) return false;
    APolitikaPrototypePlayerController* P2 = F.Remotes[0];
    APolitikaPrototypePlayerController* P3 = F.Remotes[1];
    APolitikaPrototypePlayerController* P4 = F.Remotes[2];
    APolitikaPrototypePlayerController* Unbound = F.AddRemote();
    TestEqual(TEXT("Seat 3 shares team A with the host"), F.Match->GetTeamForParticipant(3), kTeamA);
    TestEqual(TEXT("Seat 4 is on team B"), F.Match->GetTeamForParticipant(4), kTeamB);
    // Team A: host (west) and ally (north). Team B: E2 (east, unseen) and E4 near the ally.
    APolitikaPrototypeUnit* H = F.AddUnit(1, {-1500, 0});
    APolitikaPrototypeUnit* A3 = F.AddUnit(3, {0, 1500});
    APolitikaPrototypeUnit* E2 = F.AddUnit(2, {1500, 0});
    APolitikaPrototypeUnit* E4 = F.AddUnit(4, {800, 1500});
    APolitikaPrototypeUnit* U = F.AddUnit(0, {0, -1500});
    if (!H || !A3 || !E2 || !E4 || !U) return false;
    F.RefreshVision();

    struct FExpect { APolitikaPrototypeUnit* Unit; bool bTeamA; bool bTeamB; };
    const FExpect Expectations[] = {{H, true, false}, {A3, true, true}, {E2, false, true}, {E4, true, true}, {U, false, false}};
    for (const FExpect& E : Expectations)
    {
        const FString Name = FString::Printf(TEXT("entity %llu"), E.Unit->GetStableId());
        TestEqual(Name + TEXT(" observed by team A"), E.Unit->IsObservedByTeam(kTeamA), E.bTeamA);
        TestEqual(Name + TEXT(" observed by team B"), E.Unit->IsObservedByTeam(kTeamB), E.bTeamB);
        TestEqual(Name + TEXT(" relevant to host"), F.Relevant(E.Unit, F.Host), E.bTeamA);
        TestEqual(Name + TEXT(" relevant to allied seat 3 (shared vision)"), F.Relevant(E.Unit, P3), E.bTeamA);
        TestEqual(Name + TEXT(" relevant to seat 2"), F.Relevant(E.Unit, P2), E.bTeamB);
        TestEqual(Name + TEXT(" relevant to seat 4"), F.Relevant(E.Unit, P4), E.bTeamB);
        TestFalse(Name + TEXT(" never relevant to an unbound connection"), F.Relevant(E.Unit, Unbound));
    }
    TestTrue(TEXT("Unaligned keeps its own server-side vision of itself"), U->IsObservedByTeam(kUnalignedTeam));
    TestTrue(TEXT("Listen host renders hidden enemies hidden (local only)"), E2->IsLocalFogHidden() && U->IsLocalFogHidden());
    TestFalse(TEXT("Listen host renders its own and ally-observed units"), H->IsLocalFogHidden() || E4->IsLocalFogHidden());

    // Target knowledge: hidden or unknown targets are indistinguishable to the requester.
    auto Rejection = [&](APolitikaPrototypePlayerController* Source, uint64 Issuer, uint64 Target)
    {
        const CommandOutcomeEvent* Event = F.Submit(Source, F.Intent(Source, OrderKind::Attack, Issuer, Target));
        return Event ? Event->rejection : OrderRejection::None;
    };
    TestEqual(TEXT("Host cannot target an unseen enemy"), Rejection(F.Host, H->GetStableId(), E2->GetStableId()),
        OrderRejection::TargetNotVisible);
    TestEqual(TEXT("Unknown target ID looks the same as a hidden one"), Rejection(F.Host, H->GetStableId(), 777777),
        OrderRejection::TargetNotVisible);
    TestEqual(TEXT("Seat 2 cannot target the unseen host"), Rejection(P2, E2->GetStableId(), H->GetStableId()),
        OrderRejection::TargetNotVisible);
    const CommandOutcomeEvent* SharedTarget = F.Submit(F.Host, F.Intent(F.Host, OrderKind::Attack, H->GetStableId(), E4->GetStableId()));
    TestTrue(TEXT("Host may target an enemy its ally sees"), SharedTarget && SharedTarget->accepted);
    F.Match->SubmitControllerIntent(F.Host, F.Intent(F.Host, OrderKind::Stop, H->GetStableId()));
    F.Step();

    // Automatic acquisition sees only observed enemies and never engages the Unaligned.
    E2->SetActorLocation(FVector(-1150, 0, 42));
    U->SetActorLocation(FVector(-1400, -150, 42));
    F.RefreshVision();
    TestTrue(TEXT("Approaching enemy becomes relevant to the host"), F.Relevant(E2, F.Host));
    TestTrue(TEXT("Approaching Unaligned unit becomes relevant to the host"), F.Relevant(U, F.Host));
    TestTrue(TEXT("Seat 4 sees the Unaligned unit through teammate E2 (shared team vision)"), F.Relevant(U, P4));
    TestEqual(TEXT("Seat 4 relevancy for the host follows team B vision"), F.Relevant(H, P4), H->IsObservedByTeam(kTeamB));
    H->SetGuardOrder();
    H->OrderTarget.Reset();
    H->AutoAcquireForAttackMove();
    TestTrue(TEXT("Guard acquires the observed enemy, not the closer neutral Unaligned unit"), H->GetOrderTarget() == E2);

    // Losing sight ends the engagement: no fire at an unobserved unit, target reported lost.
    // A short-sighted attacker keeps the target inside weapon range but outside team vision,
    // so the result does not depend on navigation (this world has none).
    U->SetActorLocation(FVector(0, -1500, 42));
    H->SetSightRadius(300.0f);
    H->SetAttackOrder(E2);
    E2->SetActorLocation(FVector(-1000, 0, 42));
    const int32 HealthBeforeLoss = E2->GetHealth();
    F.LostTargets.Reset();
    F.Step(3);
    TestEqual(TEXT("No damage applied to a target that left vision"), E2->GetHealth(), HealthBeforeLoss);
    TestTrue(TEXT("Attacker reports the target lost"), F.LostTargets.ContainsByPredicate([&](const TargetLostEvent& Event)
        { return Event.entity_id == H->GetStableId() && Event.target_id == E2->GetStableId(); }));
    TestNull(TEXT("Attacker no longer holds the unseen target"), H->GetOrderTarget());

    // Offline play keeps full vision and the original targeting rules.
    FPolitikaMultiplayerFixture Offline(*this);
    if (!Offline.Create(TEXT("1v1"), false)) return false;
    APolitikaPrototypeUnit* Near = Offline.AddUnit(1, {-1500, 0});
    APolitikaPrototypeUnit* Far = Offline.AddUnit(2, {1500, 0});
    Offline.RefreshVision();
    TestTrue(TEXT("Standalone units are observed by every team"), Far->IsObservedByTeam(Near->GetTeamId()));
    TestTrue(TEXT("Standalone units are always relevant"), Offline.Relevant(Far, Offline.Host));
    TestFalse(TEXT("Standalone host hides nothing"), Far->IsLocalFogHidden());
    const CommandOutcomeEvent* FarAttack = Offline.Submit(Offline.Host,
        Offline.Intent(Offline.Host, OrderKind::Attack, Near->GetStableId(), Far->GetStableId()));
    TestTrue(TEXT("Standalone attack on a distant enemy is admitted as before"), FarAttack && FarAttack->accepted);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPolitikaMultiplayerAuthorityOnlySimulationTest,
    "Politika.RTS.Multiplayer.AuthorityOnlySimulation",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPolitikaMultiplayerAuthorityOnlySimulationTest::RunTest(const FString& Parameters)
{
    // MP1-10: a non-authoritative copy never advances gameplay state, even when its
    // simulation entry points are called directly.
    FPolitikaMultiplayerFixture F(*this);
    if (!F.Create(TEXT("1v1"), true) || !TestTrue(TEXT("1v1 seats bound"), F.BindAll())) return false;
    APolitikaPrototypeUnit* Attacker = F.AddUnit(1, {0, 0});
    APolitikaPrototypeUnit* Victim = F.AddUnit(2, {200, 0});
    if (!Attacker || !Victim) return false;
    F.RefreshVision();
    Attacker->SetAttackOrder(Victim);
    Attacker->SetRole(ROLE_SimulatedProxy);
    Victim->SetRole(ROLE_SimulatedProxy);
    Victim->PendingDamage = 50.0f;
    const FVector Start = Attacker->GetActorLocation();
    Attacker->SimulateMovement(0.05f);
    Attacker->ResolveCombat(0.05f);
    TestEqual(TEXT("Observer copy does not move"), Attacker->GetActorLocation(), Start);
    TestEqual(TEXT("Observer copy does not fire"), Victim->PendingDamage, 50.0f);
    TestEqual(TEXT("Observer copy does not apply damage"), Victim->ApplyPendingDamage(), 0);
    TestEqual(TEXT("Health unchanged on the observer"), Victim->GetHealth(), 100);
    Victim->Health = 0;
    TestEqual(TEXT("Observer copy does not clean up targets"), Attacker->CleanupDeadTarget(), uint64(0));

    APolitikaPrototypeObjective* Objective = F.World->SpawnActor<APolitikaPrototypeObjective>(FVector::ZeroVector, FRotator::ZeroRotator);
    FPolitikaPrototypeObjectiveDefinition Definition;
    Definition.CaptureRadiusCm = 500.0f;
    Definition.CaptureDurationSeconds = 0.1f;
    Objective->ConfigureFromDefinition(Definition, 1);
    Objective->SetRole(ROLE_SimulatedProxy);
    Objective->UpdateCapture({Attacker}, 1.0f);
    TestEqual(TEXT("Observer objective does not capture"), Objective->GetProgress(), 0.0f);

    APolitikaPrototypeMatch* Observer = F.World->SpawnActor<APolitikaPrototypeMatch>();
    Observer->SetRole(ROLE_SimulatedProxy);
    TestFalse(TEXT("Observer coordinator has no command intake"), Observer->SubmitControllerIntent(F.Host,
        F.Intent(F.Host, OrderKind::Stop, Attacker->GetStableId())));
    Observer->Tick(0.25f);
    TestEqual(TEXT("Observer coordinator runs no simulation step"), Observer->SimulationStepsExecuted, uint64(0));
    FName Failure;
    TestFalse(TEXT("Observer coordinator binds nobody"), Observer->BindController(F.Remotes[0], Failure));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPolitikaMultiplayerResultAndCleanupTest,
    "Politika.RTS.Multiplayer.ResultCommitAndCleanup",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPolitikaMultiplayerResultAndCleanupTest::RunTest(const FString& Parameters)
{
    {
        // MP1-11: a team capture commits one result, credited to the capturing team.
        FPolitikaMultiplayerFixture F(*this);
        if (!F.Create(TEXT("2v1"), true) || !TestTrue(TEXT("2v1 seats bound"), F.BindAll())) return false;
        APolitikaPrototypeUnit* Seat3 = F.AddUnit(3, {50, 0});
        APolitikaPrototypeUnit* Host = F.AddUnit(1, {-50, 0});
        F.AddUnit(2, {1800, 1800});
        F.AddUnit(0, {-1800, 1800});
        F.Match->Objective = F.World->SpawnActor<APolitikaPrototypeObjective>(FVector::ZeroVector, FRotator::ZeroRotator);
        FPolitikaPrototypeObjectiveDefinition Definition;
        Definition.CaptureRadiusCm = 300.0f;
        Definition.CaptureDurationSeconds = 0.2f;
        F.Match->Objective->ConfigureFromDefinition(Definition, 1);
        F.Match->Objective->SetEligibleTeams({kTeamA, kTeamB});
        F.RefreshVision();
        F.Step(4);
        TestTrue(TEXT("Allied units capture together without contesting"), F.Match->Objective->IsComplete()
            && !F.Match->Objective->IsContested());
        TestEqual(TEXT("Match enters Results"), F.Match->GetMatchPhase(), MatchPhase::Results);
        TestTrue(TEXT("Exactly one result committed"), F.Match->CommittedResult.IsSet());
        if (!F.Match->CommittedResult.IsSet()) return false;
        const FPolitikaPrototypeMatchResult First = *F.Match->CommittedResult;
        TestEqual(TEXT("Winning team is team A"), First.WinningTeamId, kTeamA);
        TestTrue(TEXT("Winning participant is on team A"), First.WinningPlayerId == 1 || First.WinningPlayerId == 3);
        TestEqual(TEXT("Result team replicated to observers"), F.Match->ReplicatedResultTeam, kTeamA);
#if !UE_BUILD_SHIPPING
        // F36: every authoritative step and server frame of the active match is timed, and
        // the measurements are reported once, when the result commits.
        const int32 ActiveSteps = static_cast<int32>(F.Match->SimulationStepsExecuted);
        TestTrue(TEXT("The active match executed steps"), ActiveSteps > 0);
        TestEqual(TEXT("Every authoritative step was timed"), F.Match->StepCostMicros.Num(), ActiveSteps);
        TestEqual(TEXT("Every active server frame was timed"), F.Match->ServerFrameMillis.Num(), ActiveSteps);
        TestTrue(TEXT("Measurements reported at the result"), F.Match->bNetMeasurementsReported);
#endif
        F.Step(5);
#if !UE_BUILD_SHIPPING
        TestEqual(TEXT("Finished matches are not measured"), F.Match->StepCostMicros.Num(), ActiveSteps);
#endif
        F.Match->HandleControllerLogout(F.Remotes[0]);
        TestTrue(TEXT("Later steps or departures never re-commit"), F.Match->CommittedResult->Reason == First.Reason
            && F.Match->CommittedResult->WinningTeamId == First.WinningTeamId);
        (void)Seat3; (void)Host;

        // MP1-12: returning clears every binding, route and assignment.
        F.Match->BeginReturnToMenu();
        TestEqual(TEXT("No bindings survive the match"), F.Match->ParticipantBindings.Num(), 0);
        TestEqual(TEXT("No outcome routes survive the match"), F.Match->OutcomeRoutes.Num(), 0);
        TestEqual(TEXT("Host assignment cleared"), F.Host->GetAssignedRuntimePlayerId(), 0u);
        TestEqual(TEXT("Remote assignment cleared"), F.Remotes[1]->GetAssignedRuntimePlayerId(), 0u);
        TestEqual(TEXT("Replicated roster cleared"), F.Match->ReplicatedParticipants.Num(), 0);
        TestFalse(TEXT("A former participant has no command authority"), F.Match->SubmitControllerIntent(F.Remotes[1],
            F.Intent(F.Remotes[1], OrderKind::Stop, Seat3->GetStableId())));
    }
    {
        // A remote departure during play ends the match exactly once with a defined reason.
        FPolitikaMultiplayerFixture F(*this);
        if (!F.Create(TEXT("1v1"), true) || !TestTrue(TEXT("1v1 seats bound"), F.BindAll())) return false;
        F.Match->HandleControllerLogout(F.Remotes[0]);
        TestEqual(TEXT("Disconnect moves the match to Results"), F.Match->GetMatchPhase(), MatchPhase::Results);
        TestTrue(TEXT("Disconnect result committed"), F.Match->CommittedResult.IsSet()
            && F.Match->CommittedResult->Reason == FName(TEXT("participant_disconnected"))
            && F.Match->CommittedResult->WinningTeamId == 0);
        TestEqual(TEXT("Departed participant unbound"), F.Match->GetBoundParticipantId(F.Remotes[0]), 0u);
        F.Match->HandleControllerLogout(F.Remotes[0]);
        TestEqual(TEXT("Repeated logout does not change the committed reason"),
            F.Match->CommittedResult->Reason, FName(TEXT("participant_disconnected")));
        if (UWorld* World = F.World) World->GetTimerManager().ClearAllTimersForObject(F.Match);
    }
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPolitikaMultiplayerDedicatedHostingTest,
    "Politika.RTS.Multiplayer.DedicatedHosting",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPolitikaMultiplayerDedicatedHostingTest::RunTest(const FString& Parameters)
{
    // MP3-01/02: a dedicated server has no local seat. Every seat, including seat 0, is
    // bound remotely in join order with the same participant IDs and teams as a listen
    // session of the same layout; the match activates only when all seats are bound.
    const TCHAR* Layouts[] = {TEXT("1v1"), TEXT("2v1"), TEXT("3v1"), TEXT("2v2")};
    for (const TCHAR* LayoutText : Layouts)
    {
        FPolitikaMultiplayerFixture Listen(*this);
        FPolitikaMultiplayerFixture F(*this);
        if (!Listen.Create(LayoutText, true) || !F.Create(LayoutText, true, TEXT("map.test.multiplayer_skirmish"), true))
            return false;
        const int32 Seats = F.Match->GetSessionSeatCount();
        TestEqual(FString::Printf(TEXT("%s: dedicated hosting mode"), LayoutText),
            static_cast<int32>(F.Match->GetHostingMode()), static_cast<int32>(HostingMode::Dedicated));
        TestEqual(FString::Printf(TEXT("%s: parity seat count"), LayoutText), Seats, Listen.Match->GetSessionSeatCount());
        for (int32 Seat = 0; Seat < Seats; ++Seat)
        {
            const FPolitikaPrototypeRuntimeParticipant& D = F.Match->RuntimeParticipants[Seat];
            const FPolitikaPrototypeRuntimeParticipant& L = Listen.Match->RuntimeParticipants[Seat];
            TestTrue(FString::Printf(TEXT("%s: seat %d parity (participant, slot, faction, team)"), LayoutText, Seat),
                D.RuntimePlayerId == L.RuntimePlayerId && D.SlotId == L.SlotId && D.FactionId == L.FactionId
                && D.TeamId == L.TeamId);
            TestEqual(FString::Printf(TEXT("%s: seat %d is open to a remote connection"), LayoutText, Seat),
                D.ControllerKind, EPolitikaPrototypeControllerKind::AssignedOpponent);
        }
        TestTrue(FString::Printf(TEXT("%s: fog of war enforced"), LayoutText), F.Match->IsFogOfWarEnabled());
        TestEqual(FString::Printf(TEXT("%s: no local participant on the server"), LayoutText), F.Match->LocalRuntimePlayerId, 0u);
        TestEqual(FString::Printf(TEXT("%s: every seat is remote"), LayoutText), F.Match->GetMaxRemoteHumans(),
            static_cast<uint32>(Seats));
        TestEqual(FString::Printf(TEXT("%s: listen keeps one local seat"), LayoutText), Listen.Match->GetMaxRemoteHumans(),
            static_cast<uint32>(Seats - 1));

        FName Failure;
        TestFalse(FString::Printf(TEXT("%s: a local controller cannot take a seat"), LayoutText),
            F.Match->BindController(F.Host, Failure));
        TestEqual(TEXT("Local bind reason"), Failure, FName(TEXT("MP.BIND.NO_LOCAL_SEAT")));
        TestEqual(TEXT("Local controller holds no authority"), F.Match->GetBoundParticipantId(F.Host), 0u);
        for (int32 Seat = 0; Seat < Seats; ++Seat)
        {
            TestEqual(FString::Printf(TEXT("%s: Starting until every seat is bound"), LayoutText),
                F.Match->GetMatchPhase(), MatchPhase::Starting);
            APolitikaPrototypePlayerController* Remote = F.AddRemote();
            TestTrue(FString::Printf(TEXT("%s: open seat %d admits"), LayoutText, Seat), F.Match->CanAdmitRemoteParticipant(Failure));
            TestTrue(FString::Printf(TEXT("%s: remote %d binds"), LayoutText, Seat), F.Match->BindController(Remote, Failure));
            const uint32 ExpectedId = F.Match->RuntimeParticipants[Seat].RuntimePlayerId;
            TestEqual(FString::Printf(TEXT("%s: remote %d takes the seat-order participant"), LayoutText, Seat),
                F.Match->GetBoundParticipantId(Remote), ExpectedId);
            TestEqual(TEXT("Owner-only assignment"), Remote->GetAssignedRuntimePlayerId(), ExpectedId);
            TestEqual(TEXT("Seat becomes RemoteHuman"), F.Match->RuntimeParticipants[Seat].ControllerKind,
                EPolitikaPrototypeControllerKind::RemoteHuman);
        }
        TestEqual(TEXT("First joiner holds participant 1 on team A"), F.Match->GetTeamForParticipant(
            F.Match->GetBoundParticipantId(F.Remotes[0])), kTeamA);
        TestEqual(FString::Printf(TEXT("%s: match activates when all seats are bound remotely"), LayoutText),
            F.Match->GetMatchPhase(), MatchPhase::Active);
        APolitikaPrototypePlayerController* Extra = F.AddRemote();
        TestFalse(FString::Printf(TEXT("%s: extra connection is refused"), LayoutText), F.Match->CanAdmitRemoteParticipant(Failure));
        TestEqual(TEXT("Extra connection reason"), Failure, FName(TEXT("MP.SESSION.IN_PROGRESS")));
        TestFalse(TEXT("Extra connection is not bound"), F.Match->BindController(Extra, Failure));
    }

    {
        // The same single intake and kernel admission serve seat 0 when it is remote.
        FPolitikaMultiplayerFixture F(*this);
        if (!F.Create(TEXT("1v1"), true, TEXT("map.test.multiplayer_skirmish"), true)) return false;
        FName Failure;
        APolitikaPrototypePlayerController* First = F.AddRemote();
        APolitikaPrototypePlayerController* Second = F.AddRemote();
        TestTrue(TEXT("Both remotes bind"), F.Match->BindController(First, Failure) && F.Match->BindController(Second, Failure));
        APolitikaPrototypeUnit* Own = F.AddUnit(1, FVector2D(0, 0));
        APolitikaPrototypeUnit* Foreign = F.AddUnit(2, FVector2D(300, 0));
        F.RefreshVision();
        const CommandOutcomeEvent* Accepted = F.Submit(First, F.Intent(First, OrderKind::Stop, Own->GetStableId()));
        TestTrue(TEXT("Seat-0 remote command on its own unit is accepted"), Accepted && Accepted->accepted);
        const CommandOutcomeEvent* Denied = F.Submit(First, F.Intent(First, OrderKind::Stop, Foreign->GetStableId()));
        TestTrue(TEXT("Seat-0 remote command on a foreign unit is NotOwned"), Denied && !Denied->accepted
            && Denied->rejection == OrderRejection::NotOwned);
        TestFalse(TEXT("The unbound local controller has no command authority"), F.Match->SubmitControllerIntent(F.Host,
            F.Intent(F.Host, OrderKind::Stop, Own->GetStableId())));
    }

    {
        // A departure before Active reopens exactly that seat, including seat 0.
        FPolitikaMultiplayerFixture F(*this);
        if (!F.Create(TEXT("1v1"), true, TEXT("map.test.multiplayer_skirmish"), true)) return false;
        FName Failure;
        APolitikaPrototypePlayerController* First = F.AddRemote();
        F.Match->BindController(First, Failure);
        TestEqual(TEXT("First joiner takes participant 1"), F.Match->GetBoundParticipantId(First), 1u);
        F.Match->HandleControllerLogout(First);
        TestEqual(TEXT("Pre-active departure keeps Starting"), F.Match->GetMatchPhase(), MatchPhase::Starting);
        TestFalse(TEXT("Pre-active departure schedules no session close"),
            F.World->GetTimerManager().IsTimerActive(F.Match->DedicatedSessionTimer));
        APolitikaPrototypePlayerController* Replacement = F.AddRemote();
        TestTrue(TEXT("Reopened seat 0 admits"), F.Match->BindController(Replacement, Failure));
        TestEqual(TEXT("Replacement receives participant 1"), F.Match->GetBoundParticipantId(Replacement), 1u);
    }

    {
        // MP3-12 disconnect: the match ends exactly once; the server schedules a session
        // close and reset rather than a host return to the menu.
        FPolitikaMultiplayerFixture F(*this);
        if (!F.Create(TEXT("2v2"), true, TEXT("map.test.multiplayer_skirmish"), true)) return false;
        FName Failure;
        for (int32 Seat = 0; Seat < 4; ++Seat) F.Match->BindController(F.AddRemote(), Failure);
        TestEqual(TEXT("2v2 dedicated is Active"), F.Match->GetMatchPhase(), MatchPhase::Active);
        F.Match->HandleControllerLogout(F.Remotes[2]);
        TestEqual(TEXT("Disconnect commits Results"), F.Match->GetMatchPhase(), MatchPhase::Results);
        TestTrue(TEXT("Disconnect reason committed"), F.Match->CommittedResult.IsSet()
            && F.Match->CommittedResult->Reason == FName(TEXT("participant_disconnected")));
        FTimerManager& Timers = F.World->GetTimerManager();
        TestTrue(TEXT("Session close scheduled"), Timers.IsTimerActive(F.Match->DedicatedSessionTimer));
        TestEqual(TEXT("Close reason"), F.Match->DedicatedCloseReason, FName(TEXT("participant_disconnected")));
        TestFalse(TEXT("No listen-host menu return is scheduled"), Timers.IsTimerActive(F.Match->NetworkReturnTimer));
        TestEqual(TEXT("Remaining clients stay bound until the close"), F.Match->ParticipantBindings.Num(), 3);
        Timers.ClearAllTimersForObject(F.Match);
    }

    {
        // Results: clients may linger on the result; once all have left, reset promptly.
        FPolitikaMultiplayerFixture F(*this);
        if (!F.Create(TEXT("1v1"), true, TEXT("map.test.multiplayer_skirmish"), true)) return false;
        FName Failure;
        for (int32 Seat = 0; Seat < 2; ++Seat) F.Match->BindController(F.AddRemote(), Failure);
        TestTrue(TEXT("Objective result transitions"), F.Match->TransitionLifecycle(MatchPhase::Terminal)
            && F.Match->TransitionLifecycle(MatchPhase::Results));
        F.Match->CommittedResult.Emplace(FPolitikaPrototypeMatchResult{1, TEXT("objective_secured"), kTeamA});
        F.Match->NoteNetworkResultCommitted();
        FTimerManager& Timers = F.World->GetTimerManager();
        TestEqual(TEXT("Linger close scheduled"), F.Match->DedicatedCloseReason, FName(TEXT("results_linger_elapsed")));
        TestTrue(TEXT("Linger is the long timeout"), Timers.GetTimerRemaining(F.Match->DedicatedSessionTimer) > 10.0f);
        F.Match->HandleControllerLogout(F.Remotes[0]);
        TestEqual(TEXT("One departure keeps the linger"), F.Match->DedicatedCloseReason, FName(TEXT("results_linger_elapsed")));
        TestTrue(TEXT("Departure after results does not change the result"),
            F.Match->CommittedResult->Reason == FName(TEXT("objective_secured")));
        F.Match->HandleControllerLogout(F.Remotes[1]);
        TestEqual(TEXT("Empty session resets promptly"), F.Match->DedicatedCloseReason, FName(TEXT("results_complete")));
        TestTrue(TEXT("Prompt reset is short"), Timers.GetTimerRemaining(F.Match->DedicatedSessionTimer) <= 1.5f);
        F.Match->ScheduleDedicatedSessionClose(TEXT("results_linger_elapsed"), 30.0f);
        TestEqual(TEXT("A later request never postpones a pending close"), F.Match->DedicatedCloseReason,
            FName(TEXT("results_complete")));
        Timers.ClearAllTimersForObject(F.Match);
    }

    {
        // A listen host never uses the dedicated session policy.
        FPolitikaMultiplayerFixture F(*this);
        if (!F.Create(TEXT("1v1"), true) || !TestTrue(TEXT("Listen seats bound"), F.BindAll())) return false;
        F.Match->HandleControllerLogout(F.Remotes[0]);
        TestFalse(TEXT("Listen disconnect schedules no dedicated close"),
            F.World->GetTimerManager().IsTimerActive(F.Match->DedicatedSessionTimer));
        F.World->GetTimerManager().ClearAllTimersForObject(F.Match);
    }
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPolitikaMultiplayerShotCueRedactionTest,
    "Politika.RTS.Multiplayer.ShotCueRedaction",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPolitikaMultiplayerShotCueRedactionTest::RunTest(const FString& Parameters)
{
    // MP-0 audit F27: a shot cue carries a point. Each connection must receive the target
    // point only when its team observes the target, a facing-only point when it sees just
    // the shooter, and nothing when it cannot see the shooter at all.
    using FCue = APolitikaPrototypeMatch::FTestShotCue;
    auto Fire = [](FPolitikaMultiplayerFixture& F, APolitikaPrototypeUnit* Shooter, APolitikaPrototypeUnit* Target)
    {
        F.Match->TestShotCues.Reset();
        F.Match->DistributeShotPresentation(Shooter, Target, 0.5f);
        TMap<uint32, FCue> ByViewer;
        for (const FCue& Cue : F.Match->TestShotCues) ByViewer.Add(Cue.Participant, Cue);
        return ByViewer;
    };
    auto Expect = [this](const TMap<uint32, FCue>& Cues, uint32 Viewer, ShotCue Expected, const FString& What)
    {
        const FCue* Cue = Cues.Find(Viewer);
        return TestNotNull(What + TEXT(" has a decision"), Cue)
            && TestEqual(What, static_cast<int32>(Cue->Cue), static_cast<int32>(Expected));
    };

    {
        FPolitikaMultiplayerFixture F(*this);
        if (!F.Create(TEXT("2v2"), true) || !TestTrue(TEXT("All 2v2 seats bound"), F.BindAll())) return false;
        for (APolitikaPrototypePlayerController* Remote : F.Remotes) F.World->AddController(Remote);
        APolitikaPrototypePlayerController* Unbound = F.AddRemote();
        F.World->AddController(Unbound);
        F.Match->bTestCaptureShotCues = true;
        // Host = participant 1 (team A, local view = 0); remotes 2 (B), 3 (A), 4 (B).
        APolitikaPrototypeUnit* H = F.AddUnit(1, {-1500, 0});
        APolitikaPrototypeUnit* A3 = F.AddUnit(3, {0, 1500});
        APolitikaPrototypeUnit* E2 = F.AddUnit(2, {1500, 0});
        APolitikaPrototypeUnit* E4 = F.AddUnit(4, {800, 1500});
        APolitikaPrototypeUnit* U = F.AddUnit(0, {1700, 1500});
        if (!H || !A3 || !E2 || !E4 || !U) return false;
        F.RefreshVision();
        TestTrue(TEXT("Team B sees the Unaligned unit through E4"), U->IsObservedByTeam(kTeamB));
        TestFalse(TEXT("Team A does not see the Unaligned unit"), U->IsObservedByTeam(kTeamA));
        TestTrue(TEXT("Team A sees E4 through its ally"), E4->IsObservedByTeam(kTeamA));
        TestFalse(TEXT("Team A does not see E2"), E2->IsObservedByTeam(kTeamA));
        TestFalse(TEXT("Team B does not see the host"), H->IsObservedByTeam(kTeamB));

        // Everyone sees both units: every viewer receives the exact target point.
        const TMap<uint32, FCue> Open = Fire(F, A3, E4);
        TestEqual(TEXT("One decision for the host view and each bound remote; none for the unbound one"), Open.Num(), 4);
        for (const uint32 Viewer : {0u, 2u, 3u, 4u})
            if (Expect(Open, Viewer, ShotCue::Exact, FString::Printf(TEXT("Mutually observed shot, viewer %u"), Viewer)))
                TestTrue(TEXT("Exact cue carries the target location"), Open[Viewer].Point.Equals(E4->GetActorLocation()));

        // Team A sees the shooter but not its Unaligned target: facing only, never the target.
        const TMap<uint32, FCue> Hidden = Fire(F, E4, U);
        const FVector Facing = E4->GetActorLocation()
            + E4->GetActorForwardVector().GetSafeNormal2D() * APolitikaPrototypeMatch::ShotFacingCueDistanceCm;
        for (const uint32 Viewer : {0u, 3u})
            if (Expect(Hidden, Viewer, ShotCue::FacingOnly, FString::Printf(TEXT("Unseen target, team A viewer %u"), Viewer)))
            {
                const FVector Point = Hidden[Viewer].Point;
                TestTrue(TEXT("Facing cue lies along the shooter's replicated facing"), Point.Equals(Facing, 0.1f));
                TestTrue(TEXT("Facing cue does not reveal the target position"),
                    FVector::Dist2D(Point, U->GetActorLocation()) > 250.0f);
            }
        for (const uint32 Viewer : {2u, 4u})
            if (Expect(Hidden, Viewer, ShotCue::Exact, FString::Printf(TEXT("Team B viewer %u sees the target"), Viewer)))
                TestTrue(TEXT("Team B receives the exact target"), Hidden[Viewer].Point.Equals(U->GetActorLocation()));
        // The facing distance is fixed: moving the hidden target changes nothing team A receives.
        U->SetActorLocation(FVector(1900, 1500, 42));
        F.RefreshVision();
        TestFalse(TEXT("Moved Unaligned unit is still unseen by team A"), U->IsObservedByTeam(kTeamA));
        const TMap<uint32, FCue> Moved = Fire(F, E4, U);
        TestTrue(TEXT("Team A cue is independent of the hidden target's distance"),
            Moved.Contains(3) && Moved[3].Point.Equals(Hidden[3].Point, 0.1f));

        // A shooter the viewer cannot see produces no cue at all for that viewer.
        const TMap<uint32, FCue> Unseen = Fire(F, E2, E4);
        Expect(Unseen, 0, ShotCue::None, TEXT("Host view of an unseen shooter"));
        Expect(Unseen, 3, ShotCue::None, TEXT("Allied viewer of an unseen shooter"));
        Expect(Unseen, 2, ShotCue::Exact, TEXT("Shooter's own team"));

        // The Unaligned side as shooter: team B sees it but not its team-A target.
        const TMap<uint32, FCue> Neutral = Fire(F, U, A3);
        A3->SetActorLocation(FVector(-600, 1500, 42));
        Expect(Neutral, 2, ShotCue::Exact, TEXT("Team B sees the Unaligned shooter and A3 through E4"));
        F.RefreshVision();
        TestFalse(TEXT("A3 moved out of team B vision"), A3->IsObservedByTeam(kTeamB));
        const TMap<uint32, FCue> NeutralHidden = Fire(F, U, A3);
        Expect(NeutralHidden, 4, ShotCue::FacingOnly, TEXT("Team B sees the Unaligned shooter but not its target"));
        Expect(NeutralHidden, 3, ShotCue::None, TEXT("Team A cannot see the Unaligned shooter"));
    }

    {
        // A dedicated server has no viewer of its own: its log keeps the exact point while
        // every remote seat still gets its own fog-filtered cue.
        FPolitikaMultiplayerFixture F(*this);
        if (!F.Create(TEXT("1v1"), true, TEXT("map.test.multiplayer_skirmish"), true)) return false;
        FName Failure;
        for (int32 Seat = 0; Seat < 2; ++Seat)
        {
            APolitikaPrototypePlayerController* Remote = F.AddRemote();
            F.World->AddController(Remote);
            F.Match->BindController(Remote, Failure);
        }
        F.Match->bTestCaptureShotCues = true;
        APolitikaPrototypeUnit* A = F.AddUnit(1, {-1500, 0});
        APolitikaPrototypeUnit* B = F.AddUnit(2, {-700, 0});
        APolitikaPrototypeUnit* U = F.AddUnit(0, {-1500, 1100});
        F.RefreshVision();
        TestFalse(TEXT("Team B does not see the Unaligned unit near A"), U->IsObservedByTeam(kTeamB));
        const TMap<uint32, FCue> Cues = Fire(F, A, U);
        Expect(Cues, 0, ShotCue::Exact, TEXT("Dedicated server's own record"));
        Expect(Cues, 1, ShotCue::Exact, TEXT("Seat 0 (remote) sees its target"));
        Expect(Cues, 2, ShotCue::FacingOnly, TEXT("Seat 1 sees the shooter, not the target"));
        TestNotNull(TEXT("B exists"), B);
    }

    {
        // Offline play keeps the original full presentation and sends nothing.
        FPolitikaMultiplayerFixture F(*this);
        if (!F.Create(TEXT("1v1"), false)) return false;
        F.Match->bTestCaptureShotCues = true;
        APolitikaPrototypeUnit* Near = F.AddUnit(1, {-1500, 0});
        APolitikaPrototypeUnit* Far = F.AddUnit(2, {1500, 0});
        F.RefreshVision();
        const TMap<uint32, FCue> Cues = Fire(F, Near, Far);
        TestEqual(TEXT("Offline: only the local presentation"), Cues.Num(), 1);
        if (Expect(Cues, 0, ShotCue::Exact, TEXT("Offline local presentation")))
            TestTrue(TEXT("Offline cue is the exact target"), Cues[0].Point.Equals(Far->GetActorLocation()));
    }
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPolitikaMultiplayerFogMemoryTest,
    "Politika.RTS.Multiplayer.FogMemoryAndShading",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// Listen-host fog presentation (MP-2, audit F33): the fogged start is not a sighting, a
// contact is remembered at its last *visible* position, a remembered spot the host clearly
// sees again is dropped, a death outside sight is not revealed, explored terrain persists,
// and results / a new generation clear the memory. Remote clients take the same rules
// through unit EndPlay; packaged rts_multiplayer --fog-memory covers that path.
bool FPolitikaMultiplayerFogMemoryTest::RunTest(const FString& Parameters)
{
    FPolitikaMultiplayerFixture F(*this);
    if (!F.Create(TEXT("1v1"), true) || !TestTrue(TEXT("Both 1v1 seats bound"), F.BindAll())) return false;
    APolitikaPrototypeMatch* M = F.Match;
    APolitikaPrototypeUnit* Own = F.AddUnit(1, {-1000, 0});
    APolitikaPrototypeUnit* Enemy = F.AddUnit(2, {1000, 0});
    APolitikaPrototypeUnit* Militia = F.AddUnit(0, {-1000, 1600});
    if (!Own || !Enemy || !Militia) return false;
    const auto Refresh = [&]()
    {
        F.RefreshVision();
        M->LastLocalFogSeconds = 0.0;
        M->UpdateLocalFogKnowledge();
    };
    const auto CellAt = [&](double X, double Y)
    {
        const FogMap& Fog = M->GetLocalFog();
        const FogGridSpec& Spec = Fog.Spec();
        return Fog.At(static_cast<uint32>((X - Spec.min.x) / Spec.cell_size), static_cast<uint32>((Y - Spec.min.y) / Spec.cell_size));
    };
    const auto Contacts = [&]() { return static_cast<int32>(M->GetLocalContacts().Contacts().size()); };

    Refresh();
    TestTrue(TEXT("Listen host presents fog in a networked session"), M->IsLocalFogPresentationActive());
    TestTrue(TEXT("Enemy and Unaligned start hidden"), Enemy->IsLocalFogHidden() && Militia->IsLocalFogHidden());
    TestEqual(TEXT("The fogged start is not a sighting: no remembered contacts"), Contacts(), 0);
    TestEqual(TEXT("Own position is visible"), CellAt(-1000, 0), FogCell::Visible);
    TestEqual(TEXT("Unseen enemy ground is unexplored"), CellAt(1000, 0), FogCell::Unexplored);

    // Seen at the edge of sight, then lost: remembered where it was last seen.
    Enemy->SetActorLocation(FVector(150, 0, 42));
    Refresh();
    TestFalse(TEXT("Enemy within sight is shown"), Enemy->IsLocalFogHidden());
    TestEqual(TEXT("A visible contact is not a memory"), Contacts(), 0);
    Enemy->SetActorLocation(FVector(1000, 0, 42));
    Refresh();
    const LastKnownContact* Remembered = M->GetLocalContacts().Find(Enemy->GetStableId());
    TestTrue(TEXT("Leaving sight records a last-known contact"), Enemy->IsLocalFogHidden() && Remembered != nullptr);
    if (Remembered)
    {
        TestEqual(TEXT("Remembered at the last visible x, not the hidden truth"), Remembered->position.x, 150.0);
        TestEqual(TEXT("Contact keeps its owner"), Remembered->owner, 2u);
    }

    // Explored terrain persists after vision leaves; the edge memory is inside the
    // reveal margin, so it is not cleared by sight that barely reaches it.
    Own->SetActorLocation(FVector(-1000, -1600, 42));
    Refresh();
    TestEqual(TEXT("Former sight becomes explored"), CellAt(-1000, 0), FogCell::Explored);
    TestEqual(TEXT("New sight is visible"), CellAt(-1000, -1600), FogCell::Visible);
    TestEqual(TEXT("Contact survives the move"), Contacts(), 1);

    // Seen again: the live unit replaces the memory.
    Enemy->SetActorLocation(FVector(-900, -1500, 42));
    Refresh();
    TestFalse(TEXT("Returning enemy is shown"), Enemy->IsLocalFogHidden());
    TestEqual(TEXT("Reobserved contact is no longer a memory"), Contacts(), 0);

    // Lost while the spot is still clearly in sight: recorded, then dropped as revealed.
    Enemy->SetActorLocation(FVector(1500, 1500, 42));
    F.RefreshVision();
    TestEqual(TEXT("Teleporting out of sight is recorded at the last visible spot"), Contacts(), 1);
    M->LastLocalFogSeconds = 0.0;
    M->UpdateLocalFogKnowledge();
    TestEqual(TEXT("A remembered spot the host clearly sees, empty, is cleared"), Contacts(), 0);

    // A death outside sight is not revealed.
    Own->SetActorLocation(FVector(-1000, 0, 42));
    Enemy->SetActorLocation(FVector(150, 0, 42));
    Refresh();
    Enemy->SetActorLocation(FVector(1000, 0, 42));
    Refresh();
    TestEqual(TEXT("Contact recorded before the hidden death"), Contacts(), 1);
    Enemy->Health = 0;
    F.Step();
    Refresh();
    TestFalse(TEXT("The dead unit left the authoritative list"), M->Units.Contains(Enemy));
    TestEqual(TEXT("A death the host did not see leaves the contact remembered"), Contacts(), 1);
    TestNull(TEXT("Never-seen Unaligned is not remembered"), M->GetLocalContacts().Find(Militia->GetStableId()));

    // Results clear contacts but keep the explored map; a new generation resets it all.
    TestTrue(TEXT("Match leaves Active"), M->TransitionLifecycle(MatchPhase::Terminal));
    Refresh();
    TestEqual(TEXT("Contacts are cleared when the match is no longer active"), Contacts(), 0);
    TestTrue(TEXT("Explored terrain is kept through results"), M->GetLocalFog().Count(FogCell::Explored) > 0);
    M->MatchGeneration += 1;
    Refresh();
    TestEqual(TEXT("A new generation forgets explored terrain"), static_cast<int32>(M->GetLocalFog().Count(FogCell::Explored)), 0);

    FPolitikaMultiplayerFixture Offline(*this);
    if (!Offline.Create(TEXT("1v1"), false)) return false;
    TestFalse(TEXT("Standalone play has no fog presentation"), Offline.Match->IsLocalFogPresentationActive());
    return true;
}

// GRP-02..09 (RTS_SELECTION_AND_GROUP_ORDERS.md): a local selection holds only the host's own
// living units, normalized once; a group order is a batch of ordinary single-issuer intents
// that are admitted independently and tallied without group atomicity.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPolitikaMultiplayerGroupOrdersTest,
    "Politika.RTS.Multiplayer.GroupSelectionAndOrders",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPolitikaMultiplayerGroupOrdersTest::RunTest(const FString& Parameters)
{
    using politika::selection::SelectionOp;
    FPolitikaMultiplayerFixture F(*this);
    if (!F.Create(TEXT("2v2"), true) || !TestTrue(TEXT("Every 2v2 seat binds"), F.BindAll())) return false;
    APolitikaPrototypePlayerController* Host = F.Host;
    const uint32 HostId = F.Match->GetBoundParticipantId(Host);
    uint32 AllyId = 0;
    uint32 EnemyId = 0;
    for (APolitikaPrototypePlayerController* Remote : F.Remotes)
    {
        const uint32 Id = F.Match->GetBoundParticipantId(Remote);
        if (F.Match->GetTeamForParticipant(Id) == F.Match->GetTeamForParticipant(HostId)) AllyId = Id;
        else if (EnemyId == 0) EnemyId = Id;
    }
    if (!TestTrue(TEXT("2v2 has an ally and an enemy of the host"), HostId != 0 && AllyId != 0 && EnemyId != 0)) return false;

    // The skirmish map cap: 20 units for the host, plus one unit of every other kind.
    TArray<APolitikaPrototypeUnit*> Own;
    for (int32 Index = 0; Index < 20; ++Index)
        Own.Add(F.AddUnit(HostId, {-1500.0 + (Index % 5) * 120.0, -1500.0 + (Index / 5) * 120.0}));
    APolitikaPrototypeUnit* AllyUnit = F.AddUnit(AllyId, {-900, -1500});
    APolitikaPrototypeUnit* EnemyUnit = F.AddUnit(EnemyId, {1500, 1500});
    APolitikaPrototypeUnit* UnalignedUnit = F.AddUnit(0, {0, 1500});
    APolitikaPrototypeUnit* DeadOwn = F.AddUnit(HostId, {-1500, -800});
    if (!TestFalse(TEXT("Fixture units spawned"), Own.Contains(nullptr) || !AllyUnit || !EnemyUnit || !UnalignedUnit || !DeadOwn))
        return false;
    DeadOwn->PendingDamage = static_cast<float>(DeadOwn->GetHealth());
    DeadOwn->ApplyPendingDamage();
    auto SelectedIds = [&]()
    {
        TArray<uint64> Ids;
        for (const APolitikaPrototypeUnit* Unit : F.Match->GetSelectedUnitsForPlayer(HostId)) Ids.Add(Unit->GetStableId());
        return Ids;
    };
    TArray<uint64> OwnIds;
    for (const APolitikaPrototypeUnit* Unit : Own) OwnIds.Add(Unit->GetStableId());
    OwnIds.Sort();

    // GRP-02/03: every valid own unit, once, in stable-ID order; ally, enemy, Unaligned and dead excluded.
    TArray<APolitikaPrototypeUnit*> Candidates = {EnemyUnit, Own[3], AllyUnit, UnalignedUnit, DeadOwn, Own[3]};
    for (int32 Index = Own.Num() - 1; Index >= 0; --Index) Candidates.Add(Own[Index]);
    Host->ApplySelection(Candidates, SelectionOp::Replace, TEXT("automation"));
    TestTrue(TEXT("GRP-02/03 selection is exactly the 20 living own units, sorted"), SelectedIds() == OwnIds);
    TestEqual(TEXT("Primary selected unit is the lowest stable ID"), F.Match->GetSelectedUnitForPlayer(HostId), Own[0]);
    Host->ApplySelection({Own[4]}, SelectionOp::Toggle, TEXT("automation"));
    TestEqual(TEXT("Toggle removes a selected unit"), SelectedIds().Num(), 19);
    Host->ApplySelection({Own[4]}, SelectionOp::Toggle, TEXT("automation"));
    TestTrue(TEXT("Toggle restores it"), SelectedIds() == OwnIds);
    Host->ApplySelection({Own[0], Own[1], Own[2], EnemyUnit}, SelectionOp::Subtract, TEXT("automation"));
    TestEqual(TEXT("Subtract removes only the named own units"), SelectedIds().Num(), 17);
    Host->ApplySelection({Own[0], Own[1], Own[2], AllyUnit}, SelectionOp::Union, TEXT("automation"));
    TestTrue(TEXT("Union adds own units and never an ally's"), SelectedIds() == OwnIds);
    Host->ApplySelection({EnemyUnit, AllyUnit}, SelectionOp::Replace, TEXT("automation"));
    TestEqual(TEXT("Replacing with only foreign units clears the selection"), SelectedIds().Num(), 0);
    Host->ApplySelection(Own, SelectionOp::Replace, TEXT("automation"));

    // GRP-04 and the pending bound: a 20-unit group order is 20 distinct correlated intents,
    // all queued (16 + 20 living units) and admitted on the next step.
    F.Outcomes.Reset();
    Host->SubmitGroupIntent(F.Match, Host->GetCommandableSelectedUnits(F.Match), nullptr, GuardOrder{});
    TestEqual(TEXT("Group order queues one request per selected unit"), F.Match->PendingOrders.Num(), 20);
    F.Step();
    const APolitikaPrototypePlayerController::FCommandBatch* Batch = Host->CommandBatches.IsEmpty() ? nullptr : &Host->CommandBatches.Last();
    if (!TestNotNull(TEXT("Group order recorded a batch"), Batch)) return false;
    TSet<uint32> Correlations;
    TSet<uint64> Issuers;
    for (const CommandOutcomeEvent& Outcome : F.Outcomes)
    {
        TestTrue(TEXT("GRP-04 every member accepted"), Outcome.accepted);
        Correlations.Add(static_cast<uint32>(Outcome.correlation_id));
        Issuers.Add(Outcome.issuer_id);
    }
    TestEqual(TEXT("GRP-04 twenty outcomes"), F.Outcomes.Num(), 20);
    TestEqual(TEXT("GRP-04 distinct correlations"), Correlations.Num(), 20);
    TestEqual(TEXT("GRP-04 distinct issuers"), Issuers.Num(), 20);
    TestTrue(TEXT("Batch tally complete: 20 of 20 accepted"),
        Batch->Tally.Complete() && Batch->Tally.accepted == 20 && Batch->Tally.rejected == 0);
    for (const APolitikaPrototypeUnit* Unit : Own)
        TestTrue(TEXT("Each member carries its own Guard order"), Unit->GetCurrentOrder() == EPolitikaPrototypeOrder::Guard);

    // GRP-05: a foreign issuer in the batch is rejected alone; the valid members still apply.
    F.Outcomes.Reset();
    TArray<APolitikaPrototypeUnit*> Mixed = {Own[0], Own[1], AllyUnit, Own[2]};
    Host->SubmitGroupIntent(F.Match, Mixed, nullptr, StopOrder{});
    F.Step();
    Batch = &Host->CommandBatches.Last();
    TestTrue(TEXT("GRP-05 three accepted, one rejected"),
        Batch->Tally.Complete() && Batch->Tally.accepted == 3 && Batch->Tally.rejected == 1);
    for (const CommandOutcomeEvent& Outcome : F.Outcomes)
        if (Outcome.issuer_id == AllyUnit->GetStableId())
            TestFalse(TEXT("GRP-05 the ally's unit is not commandable by the host"), Outcome.accepted);
    TestTrue(TEXT("GRP-05 ally unit keeps its own order"), AllyUnit->GetCurrentOrder() == EPolitikaPrototypeOrder::Idle);
    TestTrue(TEXT("GRP-08 an unlisted member keeps its Guard order"), Own[3]->GetCurrentOrder() == EPolitikaPrototypeOrder::Guard);
    TestTrue(TEXT("GRP-08 a listed member was stopped"), Own[0]->GetCurrentOrder() == EPolitikaPrototypeOrder::Stop);

    // GRP-07: a selected unit that dies is retired once and never returns.
    Own[5]->PendingDamage = static_cast<float>(Own[5]->GetHealth());
    Own[5]->ApplyPendingDamage();
    TestEqual(TEXT("GRP-07 dead member leaves the selection"), SelectedIds().Num(), 19);
    TestFalse(TEXT("GRP-07 dead member is not in the stored set"),
        F.Match->SelectionByPlayer.FindChecked(HostId).Units.Contains(Own[5]));
    Host->ApplySelection({Own[5]}, SelectionOp::Union, TEXT("automation"));
    TestEqual(TEXT("GRP-07 a dead unit cannot be added back"), SelectedIds().Num(), 19);

    // GRP-06: an unbound controller selects nothing and submits nothing.
    APolitikaPrototypePlayerController* Unbound = F.World->SpawnActor<APolitikaPrototypePlayerController>();
    if (!TestNotNull(TEXT("Unbound controller spawned"), Unbound)) return false;
    Unbound->MarkLocalControllerForTest();
    const int32 PendingBefore = F.Match->PendingOrders.Num();
    TestEqual(TEXT("GRP-06 unbound controller has no commandable selection"),
        Unbound->GetCommandableSelectedUnits(F.Match).Num(), 0);
    Unbound->StopSelected();
    TestEqual(TEXT("GRP-06 unbound controller queues nothing"), F.Match->PendingOrders.Num(), PendingBefore);

    // GRP-09: return/rematch cleanup clears the selection and every pending batch.
    Host->ClearSelectionState();
    TestEqual(TEXT("GRP-09 selection cleared"), SelectedIds().Num(), 0);
    TestEqual(TEXT("GRP-09 batches cleared"), Host->CommandBatches.Num(), 0);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPolitikaMultiplayerControlGroupsTest,
    "Politika.RTS.Multiplayer.ControlGroups",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPolitikaMultiplayerControlGroupsTest::RunTest(const FString& Parameters)
{
    using politika::selection::SelectionOp;
    FPolitikaMultiplayerFixture F(*this);
    if (!F.Create(TEXT("1v1"), true) || !TestTrue(TEXT("Every 1v1 seat binds"), F.BindAll())) return false;
    APolitikaPrototypePlayerController* Host = F.Host;
    const uint32 HostId = F.Match->GetBoundParticipantId(Host);
    const uint32 EnemyId = F.Match->GetBoundParticipantId(F.Remotes[0]);
    TArray<APolitikaPrototypeUnit*> Own;
    for (int32 Index = 0; Index < 20; ++Index)
        Own.Add(F.AddUnit(HostId, {-1500.0 + (Index % 5) * 120.0, -1500.0 + (Index / 5) * 120.0}));
    APolitikaPrototypeUnit* EnemyUnit = F.AddUnit(EnemyId, {1500, 1500});
    if (!TestFalse(TEXT("Fixture units spawned"), Own.Contains(nullptr) || !EnemyUnit)) return false;
    auto Ids = [](TConstArrayView<APolitikaPrototypeUnit*> Units)
    {
        std::vector<politika::selection::EntityId> Out;
        for (const APolitikaPrototypeUnit* Unit : Units) Out.push_back(Unit->GetStableId());
        return politika::selection::Normalize(std::move(Out));
    };
    auto Slice = [&Own](int32 First, int32 Count) { return TArray<APolitikaPrototypeUnit*>(Own.GetData() + First, Count); };
    auto Selected = [&]() { return Ids(F.Match->GetSelectedUnitsForPlayer(HostId)); };
    const auto& Groups = Host->GetControlGroups();
    auto Key = [&](int32 Slot, bool bCtrl, bool bShift, bool bAlt)
    {
        TestTrue(TEXT("Control groups are available in the active match"), Host->SyncControlGroups(F.Match));
        Host->ApplyControlGroupInput(F.Match, Slot, bCtrl, bShift, bAlt, TEXT("automation"));
    };
    const auto OrdersOf = [&Own]()
    {
        TArray<EPolitikaPrototypeOrder> Orders;
        for (const APolitikaPrototypeUnit* Unit : Own) Orders.Add(Unit->GetCurrentOrder());
        return Orders;
    };

    // GRP-13: Ctrl replaces, Shift unions without stealing, Alt transfers only the selected IDs.
    // None of them changes the current selection.
    Host->ApplySelection(Slice(0, 10), SelectionOp::Replace, TEXT("automation"));
    Key(1, true, false, false);
    TestTrue(TEXT("GRP-13 Ctrl+1 stores the selection"), Groups.Members(1) == Ids(Slice(0, 10)));
    TestEqual(TEXT("Groups bind to the host participant"), Groups.Participant(), HostId);
    Host->ApplySelection(Slice(5, 10), SelectionOp::Replace, TEXT("automation"));
    Key(1, false, true, false);
    TestTrue(TEXT("GRP-13 Shift+1 unions"), Groups.Members(1) == Ids(Slice(0, 15)));
    TestTrue(TEXT("Assigning never changes the selection"), Selected() == Ids(Slice(5, 10)));
    Key(3, true, false, false);
    Key(2, false, false, true);
    TestTrue(TEXT("GRP-13 Alt+2 holds the moved units"), Groups.Members(2) == Ids(Slice(5, 10)));
    TestTrue(TEXT("GRP-13 Alt+2 takes them from group 1"), Groups.Members(1) == Ids(Slice(0, 5)));
    TestTrue(TEXT("GRP-13 Alt+2 takes them from group 3"), Groups.Members(3).empty());
    Key(2, false, false, true);
    TestTrue(TEXT("GRP-13 repeated Alt+2 is idempotent"), Groups.Members(2) == Ids(Slice(5, 10)) && Groups.Members(1) == Ids(Slice(0, 5)));
    Key(4, true, true, false);
    TestTrue(TEXT("Two modifiers change no slot"), Groups.Members(4).empty());
    Host->ApplySelection({}, SelectionOp::Replace, TEXT("automation"));
    Key(1, true, false, false);
    TestTrue(TEXT("Ctrl with an empty selection keeps the slot"), Groups.Members(1) == Ids(Slice(0, 5)));

    // GRP-14/15: overlapping slots, recall and double-tap focus issue no orders and change no unit.
    Host->ApplySelection(Slice(3, 4), SelectionOp::Replace, TEXT("automation"));
    Key(0, true, false, false);
    TestTrue(TEXT("GRP-14 slot 0 exists and overlaps group 1"), Groups.Members(0) == Ids(Slice(3, 4)));
    APolitikaPrototypeCameraPawn* CameraPawn = F.World->SpawnActor<APolitikaPrototypeCameraPawn>(FVector(0, 0, 0), FRotator::ZeroRotator);
    if (!TestNotNull(TEXT("Camera pawn spawned"), CameraPawn)) return false;
    Host->Possess(CameraPawn);
    CameraPawn->GetCamera()->SetRelativeLocation(FVector(-1400, 0, 1800));
    CameraPawn->GetCamera()->SetRelativeRotation(FRotator(-52, 0, 0));
    const TArray<EPolitikaPrototypeOrder> OrdersBefore = OrdersOf();
    const TArray<FVector> LocationsBefore = [&Own]() { TArray<FVector> L; for (const auto* U : Own) L.Add(U->GetActorLocation()); return L; }();
    const int32 PendingBefore = F.Match->PendingOrders.Num();
    const FVector PawnBefore = CameraPawn->GetActorLocation();
    Key(1, false, false, false);
    TestTrue(TEXT("GRP-15 recall selects group 1"), Selected() == Ids(Slice(0, 5)));
    TestTrue(TEXT("A single recall leaves the camera"), CameraPawn->GetActorLocation().Equals(PawnBefore));
    Key(1, false, false, false);
    TestFalse(TEXT("GRP-15 double tap moves the camera"), CameraPawn->GetActorLocation().Equals(PawnBefore));
    {
        FVector Centre = FVector::ZeroVector;
        for (const APolitikaPrototypeUnit* Unit : Slice(0, 5)) Centre += Unit->GetActorLocation();
        Centre /= 5.0;
        const FVector Origin = CameraPawn->GetCamera()->GetComponentLocation();
        const FVector Forward = CameraPawn->GetCamera()->GetForwardVector();
        const FVector Ground = Origin + Forward * ((Centre.Z - Origin.Z) / Forward.Z);
        TestTrue(TEXT("GRP-15 the group centre is at screen centre"), FVector2D::Distance(FVector2D(Ground), FVector2D(Centre)) < 1.0);
    }
    Key(0, false, false, false);
    TestTrue(TEXT("GRP-14 recalling the overlapping slot selects only its members"), Selected() == Ids(Slice(3, 4)));
    TestEqual(TEXT("GRP-15 recall and focus queue no command"), F.Match->PendingOrders.Num(), PendingBefore);
    F.Step();
    TestTrue(TEXT("GRP-15 no unit order changed"), OrdersOf() == OrdersBefore);
    for (int32 Index = 0; Index < Own.Num(); ++Index)
        TestTrue(TEXT("GRP-15 no unit moved"), Own[Index]->GetActorLocation().Equals(LocationsBefore[Index]));
    // GRP-14: a group order from an overlapping recall has one request per unit, no duplicates.
    Host->ApplySelection(Slice(0, 7), SelectionOp::Replace, TEXT("automation"));
    Host->StopSelected();
    TestEqual(TEXT("GRP-14 one request per selected unit"), F.Match->PendingOrders.Num(), PendingBefore + 7);
    F.Step();

    // Empty slots keep the selection.
    Key(7, false, false, false);
    TestTrue(TEXT("Empty slot recall keeps the selection"), Selected() == Ids(Slice(0, 7)));

    // GRP-16: dead and foreign references leave every slot; they cannot come back by recall.
    Own[3]->PendingDamage = static_cast<float>(Own[3]->GetHealth());
    Own[3]->ApplyPendingDamage();
    Host->ControlGroups.Assign(5, politika::selection::GroupOp::Replace, {EnemyUnit->GetStableId(), Own[10]->GetStableId(), 999999});
    TestTrue(TEXT("Slots sync"), Host->SyncControlGroups(F.Match));
    TestTrue(TEXT("GRP-16 dead member leaves group 1"), Groups.Members(1) == Ids({Own[0], Own[1], Own[2], Own[4]}));
    TestTrue(TEXT("GRP-16 dead member leaves group 0"), Groups.Members(0) == Ids({Own[4], Own[5], Own[6]}));
    TestTrue(TEXT("GRP-16 forged enemy and missing IDs are dropped"), Groups.Members(5) == Ids({Own[10]}));
    Key(1, false, false, false);
    TestTrue(TEXT("GRP-16 recall never returns the dead unit"), Selected() == Ids({Own[0], Own[1], Own[2], Own[4]}));

    // GRP-17: Tab cycles archetype subgroups; reload/sprint go to the active subgroup only.
    for (int32 Index = 15; Index < 20; ++Index) Own[Index]->SetArchetypeId(TEXT("unit.test.second_archetype"));
    const FName FirstArchetype = Own[0]->GetArchetypeId();
    Host->ApplySelection(Own, SelectionOp::Replace, TEXT("automation"));
    TestEqual(TEXT("Selection holds the 19 living units"), static_cast<int32>(Selected().size()), 19);
    TestEqual(TEXT("A new selection starts on the first subgroup"), Host->GetActiveSubgroup(), 0);
    TestTrue(TEXT("The first subgroup is the lowest ID's archetype"),
        Host->GetActiveSubgroupUnits(F.Match).Num() == 14 && Host->GetActiveSubgroupUnits(F.Match)[0]->GetArchetypeId() == FirstArchetype);
    Host->CycleSubgroup();
    TestEqual(TEXT("Tab moves to the second subgroup"), Host->GetActiveSubgroupUnits(F.Match).Num(), 5);
    const int32 PendingBeforeReload = F.Match->PendingOrders.Num();
    Host->ReloadSelected();
    TestEqual(TEXT("Reload goes to the active subgroup only"), F.Match->PendingOrders.Num(), PendingBeforeReload + 5);
    F.Step();
    Host->CycleSubgroup();
    TestEqual(TEXT("Tab wraps to the first subgroup"), Host->GetActiveSubgroup(), 0);
    TestTrue(TEXT("Tab never edits saved groups"), Groups.Members(5) == Ids({Own[10]}));

    // A new match generation, an inactive match and a selection reset each clear every slot.
    F.Match->MatchGeneration += 1;
    TestTrue(TEXT("Slots rebind"), Host->SyncControlGroups(F.Match));
    TestTrue(TEXT("A new generation clears every slot"), Groups.Empty() && Groups.Generation() == F.Match->GetMatchGeneration());
    Host->ApplySelection(Slice(0, 2), SelectionOp::Replace, TEXT("automation"));
    Key(9, true, false, false);
    TestEqual(TEXT("Slot 9 holds two units"), static_cast<int32>(Groups.Members(9).size()), 2);
    Host->ClearSelectionState();
    TestTrue(TEXT("GRP-09 selection reset clears every slot"), Groups.Empty() && Groups.Participant() == 0);
    APolitikaPrototypePlayerController* Unbound = F.World->SpawnActor<APolitikaPrototypePlayerController>();
    if (!TestNotNull(TEXT("Unbound controller spawned"), Unbound)) return false;
    Unbound->MarkLocalControllerForTest();
    TestFalse(TEXT("An unbound controller has no control groups"), Unbound->SyncControlGroups(F.Match));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPolitikaMultiplayerGroupMoveShapeTest,
    "Politika.RTS.Multiplayer.GroupMoveShape",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPolitikaMultiplayerGroupMoveShapeTest::RunTest(const FString& Parameters)
{
    using politika::selection::SelectionOp;
    using politika::formation::Shape;
    FPolitikaMultiplayerFixture F(*this);
    if (!F.Create(TEXT("1v1"), true) || !TestTrue(TEXT("Both 1v1 seats bind"), F.BindAll())) return false;
    APolitikaPrototypePlayerController* Host = F.Host;
    const uint32 HostId = F.Match->GetBoundParticipantId(Host);
    uint32 EnemyId = 0;
    for (APolitikaPrototypePlayerController* Remote : F.Remotes) EnemyId = F.Match->GetBoundParticipantId(Remote);
    if (!TestTrue(TEXT("Host and enemy are bound"), HostId != 0 && EnemyId != 0)) return false;

    // FORM-F2 first fixture: eight own infantry in a loose cluster, ordered east.
    TArray<APolitikaPrototypeUnit*> Own;
    for (int32 Index = 0; Index < 8; ++Index)
        Own.Add(F.AddUnit(HostId, {-1500.0 + (Index % 4) * 40.0, -1200.0 + (Index / 4) * 40.0}));
    APolitikaPrototypeUnit* EnemyUnit = F.AddUnit(EnemyId, {1500, 1500});
    if (!TestFalse(TEXT("Fixture units spawned"), Own.Contains(nullptr) || !EnemyUnit)) return false;
    Host->ApplySelection(Own, SelectionOp::Replace, TEXT("automation"));
    if (!TestEqual(TEXT("Eight own units selected"), Host->GetCommandableSelectedUnits(F.Match).Num(), 8)) return false;

    // Every candidate is open unless a test probe says otherwise (no physics in this world).
    Host->GroupMoveSlotProbeForTest = [](const FVector2D&) { return true; };
    const FVector Ground(-300.0, -1180.0, 42.0);
    const FVector2D Map(Ground.X - F.Match->GetPlayableMin().X, Ground.Y - F.Match->GetPlayableMin().Y);
    auto Destinations = [&F]()
    {
        TMap<uint64, FVector> Out;
        for (const APolitikaPrototypeMatch::FPendingOrder& Pending : F.Match->PendingOrders)
            if (const APolitikaPrototypeUnit* Unit = Pending.Unit.Get()) Out.Add(Unit->GetStableId(), Pending.Destination);
        return Out;
    };
    auto MinSeparation = [](const TMap<uint64, FVector>& Points)
    {
        double Best = TNumericLimits<double>::Max();
        for (const auto& A : Points)
            for (const auto& B : Points)
                if (A.Key < B.Key) Best = FMath::Min(Best, FVector::Dist2D(A.Value, B.Value));
        return Best;
    };

    TestTrue(TEXT("Line is the default group-move shape"), Host->GetGroupMoveShape() == Shape::Line);
    Host->SubmitGroupIntent(F.Match, Host->GetCommandableSelectedUnits(F.Match), nullptr,
        MoveOrder{{Map.X, Map.Y}}, &Ground);
    TMap<uint64, FVector> Line = Destinations();
    TestEqual(TEXT("FORM-04 one move request per member"), Line.Num(), 8);
    TestTrue(TEXT("FORM-04 line destinations are distinct and a full spacing apart"), FMath::Abs(MinSeparation(Line) - 150.0) < 0.01);
    double SumY = 0.0;
    bool bOneRank = true;
    for (const auto& Entry : Line)
    {
        SumY += Entry.Value.Y;
        bOneRank &= FMath::IsNearlyEqual(Entry.Value.X, Ground.X, 0.01);
    }
    TestTrue(TEXT("FORM-04 an eastward line of eight is one rank across the travel direction"), bOneRank);
    TestTrue(TEXT("FORM-04 the line is centred on the clicked point"), FMath::IsNearlyEqual(SumY / 8.0, Ground.Y, 0.01));
    TArray<FVector> Markers;
    float Alpha = 0.0f;
    TestTrue(TEXT("Slot markers are shown for the planned destinations"), Host->GetGroupMoveMarkers(Markers, Alpha) && Markers.Num() == 8);
    F.Step();
    TestEqual(TEXT("FORM-06 every member's request is admitted on its own"),
        Host->CommandBatches.Last().Tally.accepted, 8u);

    // V switches to Column: two files deep along the travel direction.
    Host->CycleGroupMoveShape();
    TestTrue(TEXT("V cycles to Column"), Host->GetGroupMoveShape() == Shape::Column);
    Host->SubmitGroupIntent(F.Match, Host->GetCommandableSelectedUnits(F.Match), nullptr,
        MoveOrder{{Map.X, Map.Y}}, &Ground);
    TMap<uint64, FVector> Column = Destinations();
    FBox2D Extent(ForceInit);
    for (const auto& Entry : Column) Extent += FVector2D(Entry.Value.X, Entry.Value.Y);
    TestEqual(TEXT("FORM-04 column has one destination per member"), Column.Num(), 8);
    TestTrue(TEXT("FORM-04 column is four ranks deep and two files wide"),
        FMath::IsNearlyEqual(Extent.GetSize().X, 450.0, 0.01) && FMath::IsNearlyEqual(Extent.GetSize().Y, 150.0, 0.01));
    F.Step();
    Host->CycleGroupMoveShape();

    // FORM-05: blocked candidates are replaced by spare slots, never sent.
    Host->GroupMoveSlotProbeForTest = [&Map](const FVector2D& Candidate) { return Candidate.Y >= Map.Y - 200.0; };
    Host->SubmitGroupIntent(F.Match, Host->GetCommandableSelectedUnits(F.Match), nullptr,
        MoveOrder{{Map.X, Map.Y}}, &Ground);
    TMap<uint64, FVector> Blocked = Destinations();
    bool bAllOpen = true;
    for (const auto& Entry : Blocked) bAllOpen &= Entry.Value.Y - F.Match->GetPlayableMin().Y >= Map.Y - 200.0 - 0.01;
    TestTrue(TEXT("FORM-05 no member is sent to a blocked slot"), Blocked.Num() == 8 && bAllOpen);
    TestTrue(TEXT("FORM-05 replacements keep full spacing"), FMath::Abs(MinSeparation(Blocked) - 150.0) < 0.01);
    F.Step();

    // A member with no open slot keeps the plain shared-point order instead of a bad slot.
    Host->GroupMoveSlotProbeForTest = [](const FVector2D&) { return false; };
    Host->SubmitGroupIntent(F.Match, Host->GetCommandableSelectedUnits(F.Match), nullptr,
        MoveOrder{{Map.X, Map.Y}}, &Ground);
    bool bAllShared = true;
    for (const auto& Entry : Destinations()) bAllShared &= FVector::Dist2D(Entry.Value, Ground) < 0.01;
    TestTrue(TEXT("No open slot at all: every member is sent to the clicked point"), bAllShared);
    F.Step();
    Host->GroupMoveSlotProbeForTest = [](const FVector2D&) { return true; };

    // Unchanged paths: one unit goes to the exact point; Attack shares its target; no ground hit
    // (scripted callers) keeps the shared point.
    Host->SubmitGroupIntent(F.Match, {Own[0]}, nullptr, MoveOrder{{Map.X, Map.Y}}, &Ground);
    TestTrue(TEXT("A single unit moves to the exact clicked point"),
        F.Match->PendingOrders.Num() == 1 && FVector::Dist2D(F.Match->PendingOrders[0].Destination, Ground) < 0.01);
    F.Step();
    Host->SubmitGroupIntent(F.Match, Host->GetCommandableSelectedUnits(F.Match), nullptr, MoveOrder{{Map.X, Map.Y}});
    bAllShared = true;
    for (const auto& Entry : Destinations()) bAllShared &= FVector::Dist2D(Entry.Value, Ground) < 0.01;
    TestTrue(TEXT("Without a ground hit the group shares one point"), bAllShared);
    F.Step();

    // FORM-02: the shape is local presentation; it never creates organization or bonus state,
    // and Return/rematch cleanup drops the markers.
    Host->ClearSelectionState();
    TestFalse(TEXT("Markers cleared with the selection state"), Host->GetGroupMoveMarkers(Markers, Alpha));
    Host->GroupMoveSlotProbeForTest = nullptr;
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPolitikaMultiplayerFormationLifecycleTest,
    "Politika.RTS.Multiplayer.FormationLifecycle",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPolitikaMultiplayerFormationLifecycleTest::RunTest(const FString& Parameters)
{
    using politika::selection::SelectionOp;
    namespace pf = politika::formation;
    FPolitikaMultiplayerFixture F(*this);
    if (!F.Create(TEXT("1v1"), true) || !TestTrue(TEXT("Both 1v1 seats bind"), F.BindAll())) return false;
    APolitikaPrototypePlayerController* Host = F.Host;
    APolitikaPrototypePlayerController* Enemy = F.Remotes[0];
    const uint32 HostId = F.Match->GetBoundParticipantId(Host);
    const uint32 EnemyId = F.Match->GetBoundParticipantId(Enemy);
    if (!TestTrue(TEXT("Host and enemy are bound"), HostId != 0 && EnemyId != 0)) return false;

    // The first FORM fixture: eight own infantry, plus one enemy unit.
    TArray<APolitikaPrototypeUnit*> Own;
    for (int32 Index = 0; Index < 8; ++Index)
        Own.Add(F.AddUnit(HostId, {-1500.0 + (Index % 4) * 40.0, -1200.0 + (Index / 4) * 40.0}));
    APolitikaPrototypeUnit* EnemyUnit = F.AddUnit(EnemyId, {1500, 1500});
    if (!TestFalse(TEXT("Fixture units spawned"), Own.Contains(nullptr) || !EnemyUnit)) return false;
    // No physics or navmesh here: every slot and anchor is open unless a probe says otherwise.
    F.Match->FormationSlotProbeForTest = [](const FVector2D&) { return true; };
    const pf::Registry& Formations = F.Match->GetFormations();
    auto LastOutcome = [&F](const APlayerController* Source) -> const FPolitikaNetFormationOutcome*
    {
        for (int32 Index = F.Match->TestFormationOutcomes.Num() - 1; Index >= 0; --Index)
            if (F.Match->TestFormationOutcomes[Index].Key.Get() == Source) return &F.Match->TestFormationOutcomes[Index].Value;
        return nullptr;
    };
    auto Request = [&F, Host](APolitikaPrototypePlayerController* Source, pf::FormationOp Op, uint32 FormationId,
        TArray<uint64> Members = {}, double X = 0.0, double Y = 0.0)
    {
        FPolitikaNetFormationIntent Intent;
        Intent.ProtocolVersion = static_cast<int32>(FPolitikaBootstrap::Get().NetworkProtocol());
        Intent.Op = static_cast<uint8>(Op);
        Intent.FormationId = FormationId;
        Intent.Members = MoveTemp(Members);
        Intent.DestinationX = X;
        Intent.DestinationY = Y;
        // The host's own input shares this sequence, exactly like one client connection.
        uint32& Sequence = Source == Host ? Host->NextClientSequence : F.NextSequence.FindOrAdd(Source, 1);
        Intent.ClientSequence = Sequence++;
        Intent.CorrelationId = F.NextCorrelation++;
        Intent.MatchGeneration = F.Match->GetMatchGeneration();
        return Intent;
    };
    const auto Code = [](pf::FormationRejection Value) { return static_cast<int32>(Value); };
    auto Rejection = [&](APolitikaPrototypePlayerController* Source, const FPolitikaNetFormationIntent& Intent) -> int32
    {
        F.Match->SubmitFormationIntent(Source, Intent);
        const FPolitikaNetFormationOutcome* Outcome = LastOutcome(Source);
        return Outcome && Outcome->CorrelationId == Intent.CorrelationId && !Outcome->bAccepted
            ? static_cast<int32>(Outcome->Rejection) : 0;
    };
    TArray<uint64> OwnIds;
    for (const APolitikaPrototypeUnit* Unit : Own) OwnIds.Add(Unit->GetStableId());

    // FORM-01: foreign, Unaligned-free and malformed requests fail closed and change nothing.
    TArray<uint64> WithEnemy = {OwnIds[0], EnemyUnit->GetStableId()};
    TestEqual(TEXT("FORM-01 a foreign unit cannot be formed"), Rejection(Host, Request(Host, pf::FormationOp::Form, 0, WithEnemy)),
        Code(pf::FormationRejection::MemberUnavailable));
    TestEqual(TEXT("FORM-01 the enemy cannot form the host's units"),
        Rejection(Enemy, Request(Enemy, pf::FormationOp::Form, 0, {OwnIds[0], OwnIds[1]})), Code(pf::FormationRejection::MemberUnavailable));
    TestEqual(TEXT("FORM-03 an unknown ID is unavailable"), Rejection(Host, Request(Host, pf::FormationOp::Form, 0, {OwnIds[0], 999999})),
        Code(pf::FormationRejection::MemberUnavailable));
    TestEqual(TEXT("FORM-03 a duplicated member is malformed"), Rejection(Host, Request(Host, pf::FormationOp::Form, 0, {OwnIds[0], OwnIds[0]})),
        Code(pf::FormationRejection::DuplicateMember));
    FPolitikaNetFormationIntent BadShape = Request(Host, pf::FormationOp::Form, 0, {OwnIds[0], OwnIds[1]});
    BadShape.Shape = 7;
    TestEqual(TEXT("FORM-03 an unknown shape is malformed"), Rejection(Host, BadShape), Code(pf::FormationRejection::MalformedCommand));
    FPolitikaNetFormationIntent OldProtocol = Request(Host, pf::FormationOp::Form, 0, {OwnIds[0], OwnIds[1]});
    OldProtocol.ProtocolVersion -= 1;
    TestEqual(TEXT("An older protocol is malformed"), Rejection(Host, OldProtocol), Code(pf::FormationRejection::MalformedCommand));
    FPolitikaNetFormationIntent Stale = Request(Host, pf::FormationOp::Form, 0, {OwnIds[0], OwnIds[1]});
    Stale.MatchGeneration -= 1;
    TestEqual(TEXT("FORM-15 an earlier generation is stale"), Rejection(Host, Stale), Code(pf::FormationRejection::StaleMatch));
    TestTrue(TEXT("FORM-01 rejected requests create no formation"), Formations.Empty() && Host->GetOwnFormations().IsEmpty());

    // FORM-02: selection and control groups alone never create membership.
    Host->ApplySelection(Own, SelectionOp::Replace, TEXT("automation"));
    TestTrue(TEXT("Control groups sync"), Host->SyncControlGroups(F.Match));
    Host->ApplyControlGroupInput(F.Match, 1, true, false, false, TEXT("automation"));
    TestTrue(TEXT("FORM-02 selecting and grouping create no formation"), Formations.Empty());
    TestTrue(TEXT("Not yet a whole formation"), Host->SelectedWholeFormation(Host->GetCommandableSelectedUnits(F.Match)) == nullptr);

    // T forms the selection with the current (Column) shape.
    Host->CycleGroupMoveShape();
    Host->FormSelected();
    if (!TestEqual(TEXT("T forms one formation"), static_cast<int32>(Formations.All().size()), 1)) return false;
    const pf::FormationId Id = Formations.All().front().id;
    TestTrue(TEXT("Formation holds the eight members in Column"), Formations.Find(Id)->members.size() == 8
        && Formations.Find(Id)->shape == pf::Shape::Column && Formations.Find(Id)->owner == HostId);
    TestTrue(TEXT("Form outcome accepted with eight members"), LastOutcome(Host) && LastOutcome(Host)->bAccepted
        && LastOutcome(Host)->Members == 8 && LastOutcome(Host)->FormationId == Id.value);
    TestEqual(TEXT("The host sees its formation"), Host->GetOwnFormations().Num(), 1);
    TestTrue(TEXT("The owner-only view lists the members"), Host->GetOwnFormations().Num() == 1
        && Host->GetOwnFormations()[0].Members == OwnIds);
    TestEqual(TEXT("The enemy controller is sent nothing"), Enemy->GetOwnFormations().Num(), 0);
    TestTrue(TEXT("The selection is now exactly this formation"),
        Host->SelectedWholeFormation(Host->GetCommandableSelectedUnits(F.Match)) != nullptr);
    TestEqual(TEXT("Re-sending the same sequence is a duplicate"),
        [&] { FPolitikaNetFormationIntent Replay = Request(Host, pf::FormationOp::Form, 0, {OwnIds[0], OwnIds[1]});
            Replay.ClientSequence = 1; return Rejection(Host, Replay); }(), Code(pf::FormationRejection::DuplicateCommand));

    // A formation move: the server plans one slot per member and submits ordinary member orders.
    const FVector Ground(-300.0, -1180.0, 42.0);
    const FVector2D Map(Ground.X - F.Match->GetPlayableMin().X, Ground.Y - F.Match->GetPlayableMin().Y);
    TestEqual(TEXT("FORM-01 the enemy cannot move the host's formation"),
        Rejection(Enemy, Request(Enemy, pf::FormationOp::Move, Id.value, {}, Map.X, Map.Y)), Code(pf::FormationRejection::UnknownFormation));
    TestEqual(TEXT("A destination outside the map is rejected"),
        Rejection(Host, Request(Host, pf::FormationOp::Move, Id.value, {}, -10.0, Map.Y)), Code(pf::FormationRejection::InvalidDestination));
    TestEqual(TEXT("Rejected moves queue nothing"), F.Match->PendingOrders.Num(), 0);
    Host->SubmitGroupIntent(F.Match, Host->GetCommandableSelectedUnits(F.Match), nullptr, MoveOrder{{Map.X, Map.Y}}, &Ground);
    TestEqual(TEXT("FORM-04 one member order per member"), F.Match->PendingOrders.Num(), 8);
    TSet<uint64> Issuers;
    TArray<FVector> Points;
    bool bTagged = true;
    for (const APolitikaPrototypeMatch::FPendingOrder& Pending : F.Match->PendingOrders)
    {
        Issuers.Add(Pending.Request.issuer_entity_id);
        Points.Add(Pending.Destination);
        bTagged &= Pending.FormationId == Id.value && Pending.Kind == EPolitikaPrototypeOrder::Move;
    }
    double Separation = TNumericLimits<double>::Max();
    for (int32 A = 0; A < Points.Num(); ++A)
        for (int32 B = A + 1; B < Points.Num(); ++B) Separation = FMath::Min(Separation, FVector::Dist2D(Points[A], Points[B]));
    TestTrue(TEXT("FORM-04 distinct members, distinct slots a full spacing apart"),
        Issuers.Num() == 8 && FMath::Abs(Separation - 150.0) < 0.01 && bTagged);
    FBox2D Extent(ForceInit);
    for (const FVector& Point : Points) Extent += FVector2D(Point.X, Point.Y);
    TestTrue(TEXT("FORM-04 the stored Column shape is used, not the local V choice"),
        FMath::IsNearlyEqual(Extent.GetSize().X, 450.0, 0.01) && FMath::IsNearlyEqual(Extent.GetSize().Y, 150.0, 0.01));
    TestTrue(TEXT("Formation holds the move"), Formations.Find(Id)->hasDestination && Formations.Find(Id)->FollowingCount() == 8);
    F.Step();
    const APolitikaPrototypePlayerController::FCommandBatch* Batch = Host->CommandBatches.IsEmpty() ? nullptr : &Host->CommandBatches.Last();
    TestTrue(TEXT("FORM-06 the member outcomes tally under the request: 8 of 8 accepted"),
        Batch && Batch->Tally.Complete() && Batch->Tally.accepted == 8 && Batch->Tally.rejected == 0);

    // FORM-07: one member's individual order makes only that member independent.
    Host->SubmitGroupIntent(F.Match, {Own[2]}, nullptr, StopOrder{});
    TestEqual(TEXT("An individual Stop is one ordinary order"), F.Match->PendingOrders.Num(), 1);
    TestEqual(TEXT("An individual order carries no formation"), F.Match->PendingOrders[0].FormationId, 0u);
    F.Step();
    TestTrue(TEXT("FORM-07 the stopped member stays a member but is independent"),
        Formations.Find(Id)->members.size() == 8 && !Formations.Find(Id)->Find(OwnIds[2])->following
        && Formations.Find(Id)->FollowingCount() == 7);

    // FORM-09/08: two losses close the ranks once, after the hysteresis window, for the members
    // still following; the independent member keeps its own order.
    Own[7]->PendingDamage = static_cast<float>(Own[7]->GetHealth() + 1);
    F.Step();
    const uint64 LossStep = F.Match->SimulationStepsExecuted;
    TestTrue(TEXT("FORM-09 the dead member leaves; the formation is degraded"),
        Formations.Find(Id)->members.size() == 7 && Formations.Find(Id)->state == pf::FormationState::Degraded);
    TestTrue(TEXT("A regroup is scheduled after the hysteresis window"), Formations.Find(Id)->regroupDueStep
        == LossStep + pf::kRegroupDelaySteps);
    F.Step(5);
    Own[6]->PendingDamage = static_cast<float>(Own[6]->GetHealth() + 1);
    F.Step();
    TestTrue(TEXT("A second loss inside the window keeps the same regroup"), Formations.Find(Id)->regroupDueStep
        == LossStep + pf::kRegroupDelaySteps);
    while (F.Match->SimulationStepsExecuted < LossStep + pf::kRegroupDelaySteps - 1)
    {
        F.Step();
        TestEqual(TEXT("Nothing is re-sent before the regroup is due"), F.Match->PendingOrders.Num(), 0);
    }
    F.Step();
    TestEqual(TEXT("FORM-08 exactly one regroup ran"), Formations.Find(Id)->regroups, 1u);
    TestTrue(TEXT("The regroup closed the ranks"), Formations.Find(Id)->state == pf::FormationState::Organized);
    bool bOnlyFollowers = !F.Match->PendingOrders.IsEmpty();
    for (const APolitikaPrototypeMatch::FPendingOrder& Pending : F.Match->PendingOrders)
        bOnlyFollowers &= Pending.FormationId == Id.value && Pending.Request.issuer_entity_id != OwnIds[2]
            && Pending.Request.issuer_entity_id != OwnIds[6] && Pending.Request.issuer_entity_id != OwnIds[7];
    TestTrue(TEXT("FORM-07/08 regroup orders go to following members only"), bOnlyFollowers && F.Match->PendingOrders.Num() <= 5);
    F.Step(100);
    TestEqual(TEXT("FORM-08 no oscillation: no further regroup without a new loss"), Formations.Find(Id)->regroups, 1u);

    // A mixed selection (part of the formation) is an ordinary batch, not a formation order.
    Host->ApplySelection({Own[0], Own[1]}, SelectionOp::Replace, TEXT("automation"));
    TestTrue(TEXT("Two members are not the whole formation"),
        Host->SelectedWholeFormation(Host->GetCommandableSelectedUnits(F.Match)) == nullptr);
    Host->GuardSelected();
    bool bOrdinary = F.Match->PendingOrders.Num() == 2;
    for (const APolitikaPrototypeMatch::FPendingOrder& Pending : F.Match->PendingOrders) bOrdinary &= Pending.FormationId == 0;
    TestTrue(TEXT("A partial selection sends ordinary member orders"), bOrdinary);
    F.Step();
    TestEqual(TEXT("Both guarded members stay members"), static_cast<int32>(Formations.Find(Id)->members.size()), 6);

    // Formation Stop: the whole formation selected; it forgets its destination.
    TArray<APolitikaPrototypeUnit*> Living = {Own[0], Own[1], Own[2], Own[3], Own[4], Own[5]};
    Host->ApplySelection(Living, SelectionOp::Replace, TEXT("automation"));
    TestTrue(TEXT("Six living members are the whole formation"),
        Host->SelectedWholeFormation(Host->GetCommandableSelectedUnits(F.Match)) != nullptr);
    Host->StopSelected();
    bool bStops = F.Match->PendingOrders.Num() == 6;
    for (const APolitikaPrototypeMatch::FPendingOrder& Pending : F.Match->PendingOrders)
        bStops &= Pending.FormationId == Id.value && Pending.Kind == EPolitikaPrototypeOrder::Stop;
    TestTrue(TEXT("Formation Stop sends one Stop per member"), bStops && !Formations.Find(Id)->hasDestination);
    F.Step();
    Own[5]->PendingDamage = static_cast<float>(Own[5]->GetHealth() + 1);
    F.Step();
    TestTrue(TEXT("Without a destination a loss schedules no regroup"), Formations.Find(Id)->regroupDueStep == 0);

    // Release: Y on two members removes them; releasing the rest dissolves the formation.
    Host->ApplySelection({Own[0], Own[1]}, SelectionOp::Replace, TEXT("automation"));
    Host->ReleaseSelected();
    TestEqual(TEXT("Release removes the two members"), static_cast<int32>(Formations.Find(Id)->members.size()), 3);
    TestTrue(TEXT("Released units have no formation"), !Formations.FormationOf(OwnIds[0]));
    const int32 OutcomesBefore = F.Match->TestFormationOutcomes.Num();
    Host->ReleaseSelected();
    TestEqual(TEXT("Y on units outside any formation sends nothing"), F.Match->TestFormationOutcomes.Num(), OutcomesBefore);
    Host->ApplySelection({Own[2], Own[3], Own[4]}, SelectionOp::Replace, TEXT("automation"));
    Host->ReleaseSelected();
    TestTrue(TEXT("Releasing every member dissolves the formation"), Formations.Empty() && Host->GetOwnFormations().IsEmpty());

    // Re-form, then end the match: FORM-15 retires every formation once and refuses new requests.
    Host->ApplySelection({Own[0], Own[1], Own[2]}, SelectionOp::Replace, TEXT("automation"));
    Host->FormSelected();
    TestEqual(TEXT("A new formation gets a new ID"), Formations.All().empty() ? 0u : Formations.All().front().id.value, Id.value + 1);
    TestEqual(TEXT("Formation limit counts only live formations"), static_cast<int32>(Formations.CountOwned(HostId)), 1);
    if (!TestTrue(TEXT("Match ends"), F.Match->TransitionLifecycle(MatchPhase::Terminal))) return false;
    TestTrue(TEXT("FORM-15 Terminal retires every formation and the owner view"),
        Formations.Empty() && Host->GetOwnFormations().IsEmpty());
    TestEqual(TEXT("FORM-15 an inactive match refuses formation requests"),
        Rejection(Host, Request(Host, pf::FormationOp::Form, 0, {OwnIds[0], OwnIds[1]})), Code(pf::FormationRejection::MatchInactive));
    F.Match->FormationSlotProbeForTest = nullptr;
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPolitikaMultiplayerFormationStanceTest,
    "Politika.RTS.Multiplayer.FormationStance",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// FORM-F4: combat stances and formation adherence. Stance requests are formation intents that
// submit no order; Hold Fire stops idle fire but not an explicit Attack; stance changes never
// replace the current order; Form, Release and member loss never change a unit's stance; a
// chase that finds no route backs off instead of querying every step.
bool FPolitikaMultiplayerFormationStanceTest::RunTest(const FString& Parameters)
{
    using politika::selection::SelectionOp;
    namespace pf = politika::formation;
    namespace ps = politika::stance;
    FPolitikaMultiplayerFixture F(*this);
    if (!F.Create(TEXT("1v1"), true) || !TestTrue(TEXT("Both 1v1 seats bind"), F.BindAll())) return false;
    APolitikaPrototypePlayerController* Host = F.Host;
    APolitikaPrototypePlayerController* Enemy = F.Remotes[0];
    const uint32 HostId = F.Match->GetBoundParticipantId(Host);
    const uint32 EnemyId = F.Match->GetBoundParticipantId(Enemy);
    if (!TestTrue(TEXT("Host and enemy are bound"), HostId != 0 && EnemyId != 0)) return false;

    // Four own infantry in a row and one enemy 400 cm in front of the first: inside every own
    // unit's 600 cm weapon range. The enemy has no infantry behaviour, so it never fires back.
    auto* Behavior = NewObject<UPolitikaInfantryBehaviorDefinition>();
    Behavior->bRequiresLineOfFire = false;
    TArray<APolitikaPrototypeUnit*> Own;
    for (int32 Index = 0; Index < 4; ++Index)
    {
        Own.Add(F.AddUnit(HostId, {-1500.0 + Index * 100.0, -1200.0}));
        if (Own.Last()) Own.Last()->ConfigureInfantryBehavior(Behavior);
    }
    APolitikaPrototypeUnit* Target = F.AddUnit(EnemyId, {-1350.0, -800.0});
    if (!TestFalse(TEXT("Fixture units spawned"), Own.Contains(nullptr) || !Target)) return false;
    Target->Health = Target->MaxHealth = 100000; // survives the whole test
    F.Match->FormationSlotProbeForTest = [](const FVector2D&) { return true; };
    const pf::Registry& Formations = F.Match->GetFormations();
    TArray<uint64> OwnIds;
    for (const APolitikaPrototypeUnit* Unit : Own) OwnIds.Add(Unit->GetStableId());
    auto LastOutcome = [&F](const APlayerController* Source) -> const FPolitikaNetFormationOutcome*
    {
        for (int32 Index = F.Match->TestFormationOutcomes.Num() - 1; Index >= 0; --Index)
            if (F.Match->TestFormationOutcomes[Index].Key.Get() == Source) return &F.Match->TestFormationOutcomes[Index].Value;
        return nullptr;
    };
    auto Request = [&F, Host](APolitikaPrototypePlayerController* Source, pf::FormationOp Op, uint32 FormationId,
        TArray<uint64> Members, uint8 Value)
    {
        FPolitikaNetFormationIntent Intent;
        Intent.ProtocolVersion = static_cast<int32>(FPolitikaBootstrap::Get().NetworkProtocol());
        Intent.Op = static_cast<uint8>(Op);
        Intent.FormationId = FormationId;
        Intent.Members = MoveTemp(Members);
        Intent.Value = Value;
        uint32& Sequence = Source == Host ? Host->NextClientSequence : F.NextSequence.FindOrAdd(Source, 1);
        Intent.ClientSequence = Sequence++;
        Intent.CorrelationId = F.NextCorrelation++;
        Intent.MatchGeneration = F.Match->GetMatchGeneration();
        return Intent;
    };
    const auto Code = [](pf::FormationRejection Value) { return static_cast<int32>(Value); };
    auto Rejection = [&](APolitikaPrototypePlayerController* Source, const FPolitikaNetFormationIntent& Intent) -> int32
    {
        F.Match->SubmitFormationIntent(Source, Intent);
        const FPolitikaNetFormationOutcome* Outcome = LastOutcome(Source);
        return Outcome && Outcome->CorrelationId == Intent.CorrelationId && !Outcome->bAccepted
            ? static_cast<int32>(Outcome->Rejection) : -1;
    };
    const auto AllStance = [&Own](ps::CombatStance Stance)
    {
        return !Own.ContainsByPredicate([Stance](const APolitikaPrototypeUnit* Unit) { return Unit->GetEffectiveStance() != Stance; });
    };
    const uint8 HoldFire = static_cast<uint8>(ps::CombatStance::HoldFire);

    // Default: Hold Ground, no override, holding.
    TestTrue(TEXT("Units start in Hold Ground without an override"), AllStance(ps::CombatStance::HoldGround)
        && !Own.ContainsByPredicate([](const APolitikaPrototypeUnit* Unit) { return Unit->GetStanceOverride().has_value(); }));

    // FORM-01/03: foreign, malformed and misaddressed stance requests fail closed.
    TestEqual(TEXT("FORM-01 the enemy cannot set the host's stance"),
        Rejection(Enemy, Request(Enemy, pf::FormationOp::SetStance, 0, {OwnIds[0]}, HoldFire)), Code(pf::FormationRejection::MemberUnavailable));
    TestEqual(TEXT("FORM-03 an unknown stance is malformed"),
        Rejection(Host, Request(Host, pf::FormationOp::SetStance, 0, {OwnIds[0]}, 9)), Code(pf::FormationRejection::MalformedCommand));
    TestEqual(TEXT("FORM-03 adherence needs a formation"),
        Rejection(Host, Request(Host, pf::FormationOp::SetAdherence, 0, {OwnIds[0]}, 1)), Code(pf::FormationRejection::MalformedCommand));
    TestEqual(TEXT("An unknown formation is refused"),
        Rejection(Host, Request(Host, pf::FormationOp::SetStance, 77, {}, HoldFire)), Code(pf::FormationRejection::UnknownFormation));
    TestTrue(TEXT("Rejected stance requests change nothing"), AllStance(ps::CombatStance::HoldGround));

    // A Guard order on unit 1, then Hold Fire for everyone: one request, no orders.
    Host->SubmitGroupIntent(F.Match, {Own[1]}, nullptr, GuardOrder{});
    F.Step();
    TestEqual(TEXT("Hold Fire accepted for four units"),
        Rejection(Host, Request(Host, pf::FormationOp::SetStance, 0, OwnIds, HoldFire)), -1);
    TestTrue(TEXT("Outcome reports four units and no order"), LastOutcome(Host) && LastOutcome(Host)->bAccepted
        && LastOutcome(Host)->Members == 4 && LastOutcome(Host)->Orders == 0);
    TestEqual(TEXT("A stance change queues no command"), F.Match->PendingOrders.Num(), 0);
    TestTrue(TEXT("FORM-10 all four hold fire by their own override"), AllStance(ps::CombatStance::HoldFire)
        && Own[0]->GetStanceOverride() == std::optional(ps::CombatStance::HoldFire));
    TestTrue(TEXT("FORM-10 a stance change never replaces the current order"), Own[1]->GetCurrentOrder() == EPolitikaPrototypeOrder::Guard);
    Host->SubmitGroupIntent(F.Match, {Own[1]}, nullptr, StopOrder{});
    F.Step();

    // Hold Fire: an enemy inside weapon range is never fired at by idle units.
    const int32 FullHealth = Target->GetHealth();
    F.Step(40);
    TestEqual(TEXT("Hold Fire: no idle unit fires"), Target->GetHealth(), FullHealth);
    TestTrue(TEXT("Hold Fire: no idle unit holds a target"),
        !Own[0]->GetOrderTarget() && !Own[2]->GetOrderTarget() && !Own[3]->GetOrderTarget());

    // Explicit Attack overrides Hold Fire for that order only (provisional decision).
    Host->SubmitGroupIntent(F.Match, {Own[0]}, Target, AttackOrder{Target->GetStableId()});
    F.Step(30);
    TestTrue(TEXT("An explicit Attack fires despite Hold Fire"), Target->GetHealth() < FullHealth);
    TestTrue(TEXT("The other Hold Fire units stay silent"), !Own[2]->GetOrderTarget() && !Own[3]->GetOrderTarget());
    TestTrue(TEXT("An explicit order does not clear the stance"), Own[0]->GetEffectiveStance() == ps::CombatStance::HoldFire);
    Host->SubmitGroupIntent(F.Match, {Own[0]}, nullptr, StopOrder{});
    F.Step();

    // H on one unit: Hold Fire -> Hold Ground (the default, so the override goes); it fires again.
    Host->ApplySelection({Own[2]}, SelectionOp::Replace, TEXT("automation"));
    Host->CycleStanceSelected();
    TestTrue(TEXT("H cycles Hold Fire to Hold Ground and clears the override"),
        Own[2]->GetEffectiveStance() == ps::CombatStance::HoldGround && !Own[2]->GetStanceOverride());
    const int32 HealthBefore = Target->GetHealth();
    F.Step(30);
    TestTrue(TEXT("Hold Ground: the idle unit fires at the enemy in range, as before stances"),
        Target->GetHealth() < HealthBefore && Own[2]->GetOrderTarget() == Target);
    TestTrue(TEXT("Hold Ground never moves"), FVector::Dist2D(Own[2]->GetActorLocation(), FVector(-1300.0, -1200.0, 0.0)) < 1.0);

    // FORM-10: Form keeps every member's stance. Mixed stances: the formation takes the default
    // and the Hold Fire members keep their overrides.
    Host->ApplySelection(Own, SelectionOp::Replace, TEXT("automation"));
    Host->FormSelected();
    if (!TestEqual(TEXT("T forms one formation"), static_cast<int32>(Formations.All().size()), 1)) return false;
    const pf::FormationId Id = Formations.All().front().id;
    TestTrue(TEXT("Mixed members: the formation starts at the default stance and adherence"),
        Formations.Find(Id)->stance == ps::CombatStance::HoldGround && Formations.Find(Id)->adherence == ps::Adherence::Flexible);
    TestTrue(TEXT("FORM-10 Form changes no member's stance"),
        Own[0]->GetEffectiveStance() == ps::CombatStance::HoldFire && Own[1]->GetEffectiveStance() == ps::CombatStance::HoldFire
        && Own[2]->GetEffectiveStance() == ps::CombatStance::HoldGround && Own[3]->GetEffectiveStance() == ps::CombatStance::HoldFire
        && !Own[2]->GetStanceOverride() && Own[3]->GetStanceOverride());

    // H with the whole formation selected sets the formation's stance and clears every override.
    Host->CycleStanceSelected();
    TestTrue(TEXT("Whole formation: H sets the formation to Guard"), Formations.Find(Id)->stance == ps::CombatStance::Guard);
    TestTrue(TEXT("Every member fights by the formation stance, no overrides"), AllStance(ps::CombatStance::Guard)
        && !Own.ContainsByPredicate([](const APolitikaPrototypeUnit* Unit) { return Unit->GetStanceOverride().has_value(); }));
    TestTrue(TEXT("The owner view carries the stance"), Host->GetOwnFormations().Num() == 1
        && Host->GetOwnFormations()[0].Stance == static_cast<uint8>(ps::CombatStance::Guard));
    TestEqual(TEXT("The enemy sees no formation or stance"), Enemy->GetOwnFormations().Num(), 0);
    TestTrue(TEXT("Guard members chase on a 600 cm leash and return"), Own[0]->GetIdleBehavior().chase
        && FMath::IsNearlyEqual(Own[0]->GetIdleBehavior().leashCm, ps::kGuardLeashCm) && Own[0]->bReturnToPost);

    // J cycles adherence: Maintain halves the leash; Free keeps the leash but does not return.
    Host->CycleAdherenceSelected();
    TestTrue(TEXT("J sets Maintain: half leash"), Formations.Find(Id)->adherence == ps::Adherence::Maintain
        && FMath::IsNearlyEqual(Own[1]->GetIdleBehavior().leashCm, ps::kGuardLeashCm * ps::kMaintainLeashScale));
    Host->CycleAdherenceSelected();
    TestTrue(TEXT("J sets Free: full leash, no return to post"), Formations.Find(Id)->adherence == ps::Adherence::Free
        && FMath::IsNearlyEqual(Own[1]->GetIdleBehavior().leashCm, ps::kGuardLeashCm) && !Own[1]->bReturnToPost);
    Host->ApplySelection({Own[0]}, SelectionOp::Replace, TEXT("automation"));
    Host->CycleAdherenceSelected();
    TestTrue(TEXT("J on part of a formation sends nothing"), Formations.Find(Id)->adherence == ps::Adherence::Free);
    Host->ApplySelection(Own, SelectionOp::Replace, TEXT("automation"));
    Host->CycleAdherenceSelected();
    TestTrue(TEXT("J wraps back to Flexible"), Formations.Find(Id)->adherence == ps::Adherence::Flexible);

    // FORM-08: with no navmesh a Guard chase finds no route; the unit backs off for 40 steps
    // instead of querying every step. Move the in-range enemy away and add one 700 cm out.
    Target->SetActorLocation(FVector(1500.0, 1500.0, 42.0));
    APolitikaPrototypeUnit* Far = F.AddUnit(EnemyId, {-1200.0, -500.0});
    if (!TestNotNull(TEXT("Far enemy spawned"), Far)) return false;
    Far->Health = Far->MaxHealth = 100000;
    for (APolitikaPrototypeUnit* Unit : Own) Unit->OrderTarget.Reset();
    F.Step(3);
    const APolitikaPrototypeUnit* Nearest = Own[3];
    const uint64 FirstBlock = Nearest->ChaseBlockedUntilStep;
    TestTrue(TEXT("A chase without a route backs off"), FirstBlock > F.Match->SimulationStepsExecuted);
    TestTrue(TEXT("A failed chase moves nobody"), FVector::Dist2D(Nearest->GetActorLocation(), FVector(-1200.0, -1200.0, 0.0)) < 1.0);
    F.Step(20);
    TestEqual(TEXT("No retry inside the back-off window"), Nearest->ChaseBlockedUntilStep, FirstBlock);
    F.Step(25);
    TestTrue(TEXT("One retry after the window, then another back-off"), Nearest->ChaseBlockedUntilStep > FirstBlock
        && Nearest->ChaseBlockedUntilStep <= FirstBlock + 2 * APolitikaPrototypeUnit::ChaseRetryDelaySteps);
    TestTrue(TEXT("Idle autonomy never ends or replaces an order"), Nearest->GetCurrentOrder() == EPolitikaPrototypeOrder::Idle);

    // FORM-10: Release keeps each member's stance (the formation's Guard becomes their own).
    Host->ApplySelection(Own, SelectionOp::Replace, TEXT("automation"));
    Host->ReleaseSelected();
    TestTrue(TEXT("Release dissolves the formation"), Formations.Empty());
    TestTrue(TEXT("FORM-10 Release changes no stance: Guard is now each unit's own"), AllStance(ps::CombatStance::Guard)
        && Own[0]->GetStanceOverride() == std::optional(ps::CombatStance::Guard));

    // FORM-09: a member's death dissolves a pair; the survivor keeps the formation's stance.
    Host->ApplySelection({Own[2], Own[3]}, SelectionOp::Replace, TEXT("automation"));
    Host->FormSelected();
    const pf::Formation* Pair = Formations.All().empty() ? nullptr : &Formations.All().front();
    TestTrue(TEXT("Same-stance members form a Guard formation without overrides"), Pair && Pair->stance == ps::CombatStance::Guard
        && !Own[2]->GetStanceOverride() && !Own[3]->GetStanceOverride());
    Own[3]->PendingDamage = static_cast<float>(Own[3]->GetHealth() + 1);
    F.Step();
    TestTrue(TEXT("The pair dissolves"), Formations.Empty());
    TestTrue(TEXT("FORM-09 the survivor keeps Guard as its own stance"), Own[2]->GetEffectiveStance() == ps::CombatStance::Guard
        && Own[2]->GetStanceOverride() == std::optional(ps::CombatStance::Guard));
    F.Match->FormationSlotProbeForTest = nullptr;
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPolitikaMultiplayerFormationDisciplineTest,
    "Politika.RTS.Multiplayer.FormationDiscipline",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// DISC-D1: one resolve attribute (Discipline / Morale). Every unit starts at the baseline; a
// military formation member earns +1 point per evaluation (20 steps) while it follows its
// formation with enough fellow members of the same formation within 400 cm, up to 80; other
// members hold; units outside a formation fade 0.5 per evaluation back to the baseline. Form
// refuses civilians. Owner-only replication is checked by the packaged --formation-discipline run.
bool FPolitikaMultiplayerFormationDisciplineTest::RunTest(const FString& Parameters)
{
    using politika::selection::SelectionOp;
    namespace pf = politika::formation;
    namespace pd = politika::discipline;
    FPolitikaMultiplayerFixture F(*this);
    if (!F.Create(TEXT("1v1"), true) || !TestTrue(TEXT("Both 1v1 seats bind"), F.BindAll())) return false;
    APolitikaPrototypePlayerController* Host = F.Host;
    const uint32 HostId = F.Match->GetBoundParticipantId(Host);
    if (!TestTrue(TEXT("Host is bound"), HostId != 0)) return false;

    // Four soldiers in a 150 cm row; a civilian beside them; two pairs P and Q laid out so each
    // P member's only neighbour is a Q member and the other way round.
    TArray<APolitikaPrototypeUnit*> Own;
    for (int32 Index = 0; Index < 4; ++Index) Own.Add(F.AddUnit(HostId, {-1500.0 + Index * 150.0, -1200.0}));
    APolitikaPrototypeUnit* Civilian = F.AddUnit(HostId, {-1500.0, -1050.0});
    APolitikaPrototypeUnit* P1 = F.AddUnit(HostId, {-1500.0, 0.0});
    APolitikaPrototypeUnit* P2 = F.AddUnit(HostId, {-500.0, 0.0});
    APolitikaPrototypeUnit* Q1 = F.AddUnit(HostId, {-1400.0, 0.0});
    APolitikaPrototypeUnit* Q2 = F.AddUnit(HostId, {-400.0, 0.0});
    if (!TestFalse(TEXT("Fixture units spawned"), Own.Contains(nullptr) || !Civilian || !P1 || !P2 || !Q1 || !Q2)) return false;
    Civilian->SetUnitCategory(EPolitikaUnitCategory::NonCombat);
    F.Match->FormationSlotProbeForTest = [](const FVector2D&) { return true; };
    const pf::Registry& Formations = F.Match->GetFormations();
    TArray<APolitikaPrototypeUnit*> All = Own;
    All.Append({Civilian, P1, P2, Q1, Q2});

    // Steps until the next evaluation has run (evaluations fall on multiples of 20 steps).
    auto NextEvaluation = [&F]()
    {
        const uint64 Target = (F.Match->SimulationStepsExecuted / pd::kEvaluationIntervalSteps + 1) * pd::kEvaluationIntervalSteps;
        for (int32 Guard = 0; F.Match->SimulationStepsExecuted < Target && Guard < 100; ++Guard) F.Step();
    };
    auto Evaluations = [&NextEvaluation](int32 Count) { for (int32 Index = 0; Index < Count; ++Index) NextEvaluation(); };
    auto Values = [](TConstArrayView<APolitikaPrototypeUnit*> Units)
    {
        TArray<int32> Result;
        for (const APolitikaPrototypeUnit* Unit : Units) Result.Add(Unit->GetResolveMilli());
        return Result;
    };

    // DISC-01: every unit starts at the baseline; the civilian is the only non-combat unit.
    TestTrue(TEXT("DISC-01 every unit starts at the baseline"),
        !All.ContainsByPredicate([](const APolitikaPrototypeUnit* Unit) { return Unit->GetResolveMilli() != pd::kBaseline; }));
    TestTrue(TEXT("DISC-01 soldiers are military, the civilian is not"),
        Own[0]->IsMilitary() && P1->IsMilitary() && !Civilian->IsMilitary());

    // DISC-01: Form refuses a civilian, through the controller's own request path, and changes nothing.
    Host->ApplySelection({Own[0], Own[1], Civilian}, SelectionOp::Replace, TEXT("automation"));
    Host->FormSelected();
    const FPolitikaNetFormationOutcome* Refused = F.Match->TestFormationOutcomes.Num() > 0 ? &F.Match->TestFormationOutcomes.Last().Value : nullptr;
    TestTrue(TEXT("DISC-01 Form with a civilian is refused as NotMilitary"), Refused && !Refused->bAccepted
        && Refused->Rejection == static_cast<uint8>(pf::FormationRejection::NotMilitary));
    TestTrue(TEXT("The refused Form created no formation"), Formations.Empty());

    // Outside any formation at the baseline: nothing changes at an evaluation.
    NextEvaluation();
    TestTrue(TEXT("DISC-06 units outside a formation stay at the baseline"),
        !All.ContainsByPredicate([](const APolitikaPrototypeUnit* Unit) { return Unit->GetResolveMilli() != pd::kBaseline; }));

    // DISC-02: Form changes no value; earning starts at the next evaluation.
    Host->ApplySelection(Own, SelectionOp::Replace, TEXT("automation"));
    Host->FormSelected();
    if (!TestEqual(TEXT("T forms one formation of four"), static_cast<int32>(Formations.All().size()), 1)) return false;
    const pf::FormationId Id = Formations.All().front().id;
    TestTrue(TEXT("DISC-02 Form changes no value"), Values(Own) == TArray<int32>({50000, 50000, 50000, 50000}));
    Host->ApplySelection({P1, P2}, SelectionOp::Replace, TEXT("automation"));
    Host->FormSelected();
    Host->ApplySelection({Q1, Q2}, SelectionOp::Replace, TEXT("automation"));
    Host->FormSelected();
    TestEqual(TEXT("P and Q are formed"), static_cast<int32>(Formations.All().size()), 3);

    // DISC-03: every supported member gains exactly one point; DISC-05: neighbouring formations
    // do not support each other; the civilian never earns.
    NextEvaluation();
    TestTrue(TEXT("DISC-03 supported members gain one point"), Values(Own) == TArray<int32>({51000, 51000, 51000, 51000}));
    TestTrue(TEXT("DISC-03 a gaining member shows Rising"), Own[0]->GetResolveTrend() == pd::Trend::Rising);
    TestTrue(TEXT("DISC-05 a neighbouring formation gives no support"),
        Values({P1, P2, Q1, Q2}) == TArray<int32>({50000, 50000, 50000, 50000}) && P1->GetResolveTrend() == pd::Trend::Steady);
    TestEqual(TEXT("DISC-01 the civilian never earns"), Civilian->GetResolveMilli(), pd::kBaseline);

    // DISC-04: a stranded member (1050 cm from the nearest member) holds; the others gain.
    Own[3]->SetActorLocation(FVector(-150.0, -1200.0, 42.0));
    NextEvaluation();
    TestTrue(TEXT("DISC-04 a stranded member holds, the others gain"), Values(Own) == TArray<int32>({52000, 52000, 52000, 51000}));
    TestTrue(TEXT("DISC-04 a holding member shows Steady"), Own[3]->GetResolveTrend() == pd::Trend::Steady);

    // DISC-04: an independent member holds. A formation move makes every member follow it (the
    // Automation world has no navmesh, so nobody moves); an individual Stop makes Own[0] independent.
    const FVector Ground(-300.0, -1180.0, 42.0);
    const FVector2D Map(Ground.X - F.Match->GetPlayableMin().X, Ground.Y - F.Match->GetPlayableMin().Y);
    Host->ApplySelection(Own, SelectionOp::Replace, TEXT("automation"));
    Host->SubmitGroupIntent(F.Match, Host->GetCommandableSelectedUnits(F.Match), nullptr, MoveOrder{{Map.X, Map.Y}}, &Ground);
    F.Step();
    Host->SubmitGroupIntent(F.Match, {Own[0]}, nullptr, StopOrder{});
    F.Step();
    const pf::Formation* Formation = Formations.Find(Id);
    if (!TestTrue(TEXT("Own[0] is now independent of the formation's move"), Formation && Formation->hasDestination
        && Formation->Find(Own[0]->GetStableId()) && !Formation->Find(Own[0]->GetStableId())->following)) return false;
    const TArray<int32> BeforeIndependent = Values(Own);
    NextEvaluation();
    TestEqual(TEXT("DISC-04 an independent member holds"), Own[0]->GetResolveMilli(), BeforeIndependent[0]);
    TestEqual(TEXT("A following, supported member still gains"), Own[1]->GetResolveMilli(), BeforeIndependent[1] + 1000);

    // DISC-05: a dead member gives no support. Own[2]'s only remaining neighbour is Own[0].
    Own[1]->Health = 0;
    F.Step();
    if (!TestNull(TEXT("The dead member left the formation"), Formations.Find(Id)->Find(Own[1]->GetStableId()))) return false;
    const int32 Own2Before = Own[2]->GetResolveMilli();
    NextEvaluation();
    TestEqual(TEXT("DISC-05 a dead member gives no support"), Own[2]->GetResolveMilli(), Own2Before);

    // DISC-03: back in the ranks, members climb to the ceiling and never past it.
    Own[3]->SetActorLocation(FVector(-1350.0, -1200.0, 42.0));
    NextEvaluation();
    TestEqual(TEXT("A member back in the ranks gains again"), Own[3]->GetResolveMilli(), 52000);
    Evaluations(40);
    TestEqual(TEXT("DISC-03 the gain stops at the ceiling of 80"), Own[3]->GetResolveMilli(), pd::kNormalCeiling);
    TestEqual(TEXT("DISC-03 every following member reaches the ceiling"), Own[2]->GetResolveMilli(), pd::kNormalCeiling);
    TestTrue(TEXT("At the ceiling the trend is Steady"), Own[3]->GetResolveTrend() == pd::Trend::Steady);
    TestEqual(TEXT("The independent member still holds"), Own[0]->GetResolveMilli(), BeforeIndependent[0]);

    // DISC-06: after Release values fade 0.5 per evaluation to the baseline and never below it.
    Host->ApplySelection({Own[0], Own[2], Own[3]}, SelectionOp::Replace, TEXT("automation"));
    Host->ReleaseSelected();
    TestTrue(TEXT("Release dissolves the formation"), Formations.Find(Id) == nullptr);
    TestEqual(TEXT("Release changes no value"), Own[3]->GetResolveMilli(), pd::kNormalCeiling);
    NextEvaluation();
    TestEqual(TEXT("DISC-06 a released unit fades half a point"), Own[3]->GetResolveMilli(), pd::kNormalCeiling - 500);
    TestTrue(TEXT("A fading unit shows Fading"), Own[3]->GetResolveTrend() == pd::Trend::Fading);
    Evaluations(58);
    TestEqual(TEXT("After 59 evaluations a released unit is half a point above the baseline"), Own[3]->GetResolveMilli(), pd::kBaseline + 500);
    Evaluations(5);
    TestTrue(TEXT("DISC-06 every released unit is back at the baseline and not below it"),
        Values({Own[0], Own[2], Own[3]}) == TArray<int32>({pd::kBaseline, pd::kBaseline, pd::kBaseline}));
    TestTrue(TEXT("At the baseline the trend is Steady"), Own[3]->GetResolveTrend() == pd::Trend::Steady);
    TestEqual(TEXT("The civilian stayed at the baseline throughout"), Civilian->GetResolveMilli(), pd::kBaseline);

    F.Match->FormationSlotProbeForTest = nullptr;
    return true;
}

#endif
