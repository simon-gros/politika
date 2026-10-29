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

#endif
