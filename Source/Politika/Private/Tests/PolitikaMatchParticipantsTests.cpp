#if WITH_DEV_AUTOMATION_TESTS

#include "PolitikaPrototypeGameplay.h"
#include "PolitikaPrototypeContent.h"
#include "Engine/World.h"
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPolitikaMatchParticipantsAutomationTest,
    "Politika.RTS.MatchParticipants.CanonicalMappingAndAuthorityBoundaries",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPolitikaMatchParticipantsAutomationTest::RunTest(const FString& Parameters)
{
    using namespace politika::rules;
    UPolitikaPrototypeContentCatalog* Catalog = LoadObject<UPolitikaPrototypeContentCatalog>(nullptr,
        TEXT("/Game/Politika/Definitions/DA_PrototypeContentCatalog.DA_PrototypeContentCatalog"));
    TestNotNull(TEXT("Production content catalog is available"), Catalog);
    if (!Catalog) return false;
    const UPolitikaPrototypeMapDefinition* AuthoredMap =
        Catalog->FindMap(UPolitikaPrototypeContentValidator::PrototypeMapAssetId);
    TestNotNull(TEXT("Canonical prototype map resolves from production catalog"), AuthoredMap);
    if (!AuthoredMap) return false;

    TArray<FPolitikaPrototypeRuntimeParticipant> Participants;
    uint32 LocalPlayerId = 0;
    FName FailureCode;
    TestTrue(TEXT("Validated authored slots produce a candidate runtime roster"),
        APolitikaPrototypeMatch::BuildRuntimeParticipantsFromSlots(*AuthoredMap, Participants, LocalPlayerId, FailureCode));
    TestEqual(TEXT("Roster preserves the authored playable slot count"), Participants.Num(), AuthoredMap->PlayerSlots.Num());
    const FPolitikaPrototypePlayerSlot* AuthoredHuman = AuthoredMap->PlayerSlots.FindByPredicate(
        [](const FPolitikaPrototypePlayerSlot& Slot) { return Slot.bHumanControlled; });
    TestNotNull(TEXT("Canonical prototype declares one local human slot"), AuthoredHuman);
    if (AuthoredHuman)
    {
        TestEqual(TEXT("Local authority resolves from authored runtime participant ID"), LocalPlayerId,
            static_cast<uint32>(AuthoredHuman->RuntimePlayerId));
        const FPolitikaPrototypeRuntimeParticipant* Local = Participants.FindByPredicate(
            [LocalPlayerId](const FPolitikaPrototypeRuntimeParticipant& P) { return P.RuntimePlayerId == LocalPlayerId; });
        TestTrue(TEXT("Local controller kind is distinct and matches authored faction"), Local
            && Local->ControllerKind == EPolitikaPrototypeControllerKind::LocalHuman
            && Local->FactionId == AuthoredHuman->FactionId);
    }
    const FPolitikaPrototypeRuntimeParticipant* Opponent = Participants.FindByPredicate(
        [LocalPlayerId](const FPolitikaPrototypeRuntimeParticipant& P) { return P.RuntimePlayerId != LocalPlayerId; });
    TestTrue(TEXT("Opposing test participant is assigned but not locally controlled"), Opponent
        && Opponent->ControllerKind == EPolitikaPrototypeControllerKind::AssignedOpponent);
    const uint32 AuthoredOpponentId = Opponent ? Opponent->RuntimePlayerId : 0;
    const FName AuthoredOpponentFaction = Opponent ? Opponent->FactionId : NAME_None;

    auto AssertRejectedCandidate = [this, AuthoredMap](const TCHAR* Label,
        TFunction<void(UPolitikaPrototypeMapDefinition&)> Mutate)
    {
        UPolitikaPrototypeMapDefinition* Invalid = DuplicateObject<UPolitikaPrototypeMapDefinition>(AuthoredMap, GetTransientPackage());
        Mutate(*Invalid);
        TArray<FPolitikaPrototypeRuntimeParticipant> Candidate;
        uint32 CandidateLocalId = 999;
        FName CandidateFailure;
        const bool bAccepted = APolitikaPrototypeMatch::BuildRuntimeParticipantsFromSlots(
            *Invalid, Candidate, CandidateLocalId, CandidateFailure);
        TestFalse(FString::Printf(TEXT("%s fails closed"), Label), bAccepted);
        TestEqual(FString::Printf(TEXT("%s publishes no candidate participants"), Label), Candidate.Num(), 0);
        TestEqual(FString::Printf(TEXT("%s leaves no local authority binding"), Label), CandidateLocalId, uint32(0));
    };
    AssertRejectedCandidate(TEXT("Duplicate runtime participant ID"), [](UPolitikaPrototypeMapDefinition& M)
        { M.PlayerSlots[1].RuntimePlayerId = M.PlayerSlots[0].RuntimePlayerId; });
    AssertRejectedCandidate(TEXT("Duplicate playable slot ID"), [](UPolitikaPrototypeMapDefinition& M)
        { M.PlayerSlots[1].SlotId = M.PlayerSlots[0].SlotId; });
    AssertRejectedCandidate(TEXT("Duplicate playable faction assignment"), [](UPolitikaPrototypeMapDefinition& M)
        { M.PlayerSlots[1].FactionId = M.PlayerSlots[0].FactionId; });
    AssertRejectedCandidate(TEXT("Missing prototype local human"), [](UPolitikaPrototypeMapDefinition& M)
        { for (FPolitikaPrototypePlayerSlot& Slot : M.PlayerSlots) Slot.bHumanControlled = false; });
    AssertRejectedCandidate(TEXT("Multiple prototype local humans"), [](UPolitikaPrototypeMapDefinition& M)
        { for (FPolitikaPrototypePlayerSlot& Slot : M.PlayerSlots) Slot.bHumanControlled = true; });

    UPolitikaPrototypeMapDefinition* InvalidMap = DuplicateObject<UPolitikaPrototypeMapDefinition>(AuthoredMap, GetTransientPackage());
    InvalidMap->PlayerSlots[0].FactionId = TEXT("faction.unaligned");
    TArray<FPolitikaContentDiagnostic> Diagnostics;
    UPolitikaPrototypeContentValidator::ValidateMapStructure(InvalidMap, Catalog, Diagnostics);
    TestTrue(TEXT("Unaligned is rejected from ordinary playable slots by shared validation"),
        Diagnostics.ContainsByPredicate([](const FPolitikaContentDiagnostic& D)
            { return D.Code == TEXT("MAP.FACTION.UNALIGNED_PLAYABLE"); }));
    TestFalse(TEXT("Runtime conversion independently fails closed for Unaligned playable mapping"),
        APolitikaPrototypeMatch::BuildRuntimeParticipantsFromSlots(*InvalidMap, Participants, LocalPlayerId, FailureCode));
    TestEqual(TEXT("Rejected candidate roster remains empty"), Participants.Num(), 0);
    TestEqual(TEXT("Rejected candidate has no local authority binding"), LocalPlayerId, uint32(0));
    TestTrue(TEXT("Canonical authored mapping can be rebuilt after an isolated rejected candidate"),
        APolitikaPrototypeMatch::BuildRuntimeParticipantsFromSlots(*AuthoredMap, Participants, LocalPlayerId, FailureCode));

    const UWorld::InitializationValues InitValues = UWorld::InitializationValues()
        .AllowAudioPlayback(false).RequiresHitProxies(false).CreatePhysicsScene(false)
        .CreateNavigation(false).CreateAISystem(false).ShouldSimulatePhysics(false)
        .SetTransactional(false).CreateFXSystem(false);
    UWorld* World = UWorld::CreateWorld(EWorldType::Game, false,
        FName(TEXT("PolitikaMatchParticipantsAutomationWorld")), GetTransientPackage(), false,
        ERHIFeatureLevel::Num, &InitValues);
    TestNotNull(TEXT("Isolated coordinator world created"), World);
    if (!World) return false;
    APolitikaPrototypeMatch* Match = World->SpawnActor<APolitikaPrototypeMatch>();
    APolitikaPrototypeUnit* LocalUnit = World->SpawnActor<APolitikaPrototypeUnit>(FVector(0, 0, 42), FRotator::ZeroRotator);
    APolitikaPrototypeUnit* OpponentUnit = World->SpawnActor<APolitikaPrototypeUnit>(FVector(100, 0, 42), FRotator::ZeroRotator);
    APolitikaPrototypeUnit* SystemUnit = World->SpawnActor<APolitikaPrototypeUnit>(FVector(200, 0, 42), FRotator::ZeroRotator);
    if (!Match || !LocalUnit || !OpponentUnit || !SystemUnit) { World->DestroyWorld(false); return false; }

    const uint32 HumanId = LocalPlayerId;
    const FName HumanFaction = AuthoredHuman ? AuthoredHuman->FactionId : NAME_None;
    const uint32 OpponentId = AuthoredOpponentId;
    const FName OpponentFaction = AuthoredOpponentFaction;
    LocalUnit->Configure(101, HumanId, FText::FromString(TEXT("Local human")), 100, 100, 200, 10, 0.5f,
        true, nullptr, FVector::OneVector, FLinearColor::White, HumanFaction);
    OpponentUnit->Configure(202, OpponentId, FText::FromString(TEXT("Opponent")), 100, 100, 200, 10, 0.5f,
        true, nullptr, FVector::OneVector, FLinearColor::White, OpponentFaction);
    SystemUnit->Configure(303, 0, FText::FromString(TEXT("Unaligned system fixture")), 100, 100, 200, 10, 0.5f,
        true, nullptr, FVector::OneVector, FLinearColor::White, TEXT("faction.unaligned"));
    Match->RuntimeParticipants = Participants;
    Match->LocalRuntimePlayerId = HumanId;
    Match->UnalignedSystemAuthority = {TEXT("faction.unaligned"), true};
    Match->Units = {LocalUnit, OpponentUnit, SystemUnit};
    Match->PlayableMin = FVector2D(-1000, -1000);
    Match->PlayableMax = FVector2D(1000, 1000);
    Match->bContentReady = true;
    Match->MatchGeneration = 17;
    TestTrue(TEXT("Production coordinator enters Active through legal lifecycle transitions"),
        Match->TransitionLifecycle(MatchPhase::SetupValidated)
        && Match->TransitionLifecycle(MatchPhase::Loading)
        && Match->TransitionLifecycle(MatchPhase::Starting)
        && Match->TransitionLifecycle(MatchPhase::Active));
    TestTrue(TEXT("Unaligned authority is a separate system record, not a participant"),
        Match->UnalignedSystemAuthority.bNpcSystemControlled
        && Match->UnalignedSystemAuthority.FactionId == TEXT("faction.unaligned")
        && Match->RuntimeParticipants.Num() == AuthoredMap->PlayerSlots.Num());
    TestTrue(TEXT("Runtime unit keeps faction distinct from owner"),
        OpponentUnit->GetOwnerPlayer() == OpponentId && OpponentUnit->GetFactionId() == OpponentFaction);
    TestFalse(TEXT("No transient-world controller is treated as a bound human input source"),
        Match->IsLocalControllerBound(nullptr));
    Match->SetSelection(HumanId, LocalUnit);
    TestEqual(TEXT("Local human selects its admitted unit"), Match->GetSelectedUnitForPlayer(HumanId), LocalUnit);
    // The raw-ID entry is private to trusted coordinator/test code. The packaged
    // PlayerController interaction harness separately exercises the real input route.
    Match->SubmitCommand(HumanId, LocalUnit, nullptr, MoveOrder{{100, 100}});
    Match->Tick(0.05f);
    TestEqual(TEXT("Trusted local Move still requires a complete NavMesh route"), LocalUnit->GetCurrentOrder(), EPolitikaPrototypeOrder::Stop);

    const int32 InitialOpponentHealth = OpponentUnit->GetHealth();
    const EPolitikaPrototypeOrder InitialOpponentOrder = OpponentUnit->GetCurrentOrder();
    TestFalse(TEXT("Unbound input source cannot use the local-human binding"),
        Match->SubmitLocalControllerCommand(nullptr, OpponentUnit, nullptr, MoveOrder{{100, 100}}));
    TestFalse(TEXT("Unbound input source cannot claim opposing authority"),
        Match->SubmitLocalControllerCommand(nullptr, OpponentUnit, nullptr, MoveOrder{{100, 100}}));
    TestFalse(TEXT("Unbound input source cannot claim system authority"),
        Match->SubmitLocalControllerCommand(nullptr, SystemUnit, nullptr, MoveOrder{{100, 100}}));
    Match->SubmitCommand(0, SystemUnit, nullptr, MoveOrder{{100, 100}});
    TestEqual(TEXT("System entity cannot be selected as a playable participant"),
        Match->GetSelectedUnitForPlayer(0), static_cast<APolitikaPrototypeUnit*>(nullptr));
    Match->SetSelection(OpponentId, OpponentUnit);
    TestEqual(TEXT("Opponent unit is not selected by the local-player selection store"),
        Match->GetSelectedUnitForPlayer(OpponentId), static_cast<APolitikaPrototypeUnit*>(nullptr));
    TestEqual(TEXT("Rejected authority requests do not change opponent health"), OpponentUnit->GetHealth(), InitialOpponentHealth);
    TestEqual(TEXT("Rejected authority requests do not change the opposing unit order"), OpponentUnit->GetCurrentOrder(), InitialOpponentOrder);
    TestEqual(TEXT("Valid order was consumed and rejected authority requests leave no runnable commands"), Match->PendingOrders.Num(), 0);
    TestEqual(TEXT("Rejected spoof attempts do not resume the unavailable local route"), LocalUnit->GetCurrentOrder(), EPolitikaPrototypeOrder::Stop);

    Match->BeginReturnToMenu();
    TestEqual(TEXT("Return retires runtime participants"), Match->RuntimeParticipants.Num(), 0);
    TestEqual(TEXT("Return invalidates local controller identity"), Match->LocalRuntimePlayerId, uint32(0));
    TestFalse(TEXT("Return retires Unaligned system authority"), Match->UnalignedSystemAuthority.bNpcSystemControlled);
    TestEqual(TEXT("Return clears selected units"), Match->SelectionByPlayer.Num(), 0);

    APolitikaPrototypeMatch* RejectedMatch = World->SpawnActor<APolitikaPrototypeMatch>();
    APolitikaPrototypeUnit* PartialUnit = World->SpawnActor<APolitikaPrototypeUnit>(FVector(300, 0, 42), FRotator::ZeroRotator);
    APolitikaPrototypeObjective* PartialObjective = World->SpawnActor<APolitikaPrototypeObjective>();
    if (!RejectedMatch || !PartialUnit || !PartialObjective) { World->DestroyWorld(false); return false; }
    RejectedMatch->RuntimeParticipants = Participants;
    RejectedMatch->LocalRuntimePlayerId = HumanId;
    RejectedMatch->UnalignedSystemAuthority = {TEXT("faction.unaligned"), true};
    RejectedMatch->Units = {PartialUnit};
    RejectedMatch->Objective = PartialObjective;
    RejectedMatch->bContentReady = true;
    RejectedMatch->PendingOrders.AddDefaulted();
    RejectedMatch->SelectionByPlayer.Add(HumanId, PartialUnit);
    RejectedMatch->RecoverToShellAfterAdmissionFailure(AuthoredMap->StableId);
    TestTrue(TEXT("Rejected participant setup enters the existing AdmissionRejected lifecycle"),
        RejectedMatch->GetMatchPhase() == MatchPhase::AdmissionRejected);
    TestEqual(TEXT("Rejected setup retires its roster atomically"), RejectedMatch->RuntimeParticipants.Num(), 0);
    TestEqual(TEXT("Rejected setup invalidates its local binding"), RejectedMatch->GetLocalRuntimePlayerId(), uint32(0));
    TestFalse(TEXT("Rejected setup retires Unaligned authority"), RejectedMatch->UnalignedSystemAuthority.bNpcSystemControlled);
    TestEqual(TEXT("Rejected setup removes partial units"), RejectedMatch->GetUnits().Num(), 0);
    TestNull(TEXT("Rejected setup removes the partial objective"), RejectedMatch->GetObjective());
    TestEqual(TEXT("Rejected setup clears selection and pending commands"), RejectedMatch->SelectionByPlayer.Num(), 0);
    TestEqual(TEXT("Rejected setup leaves no accepted pending commands"), RejectedMatch->PendingOrders.Num(), 0);
    RejectedMatch->Tick(0.5f);
    TestEqual(TEXT("Rejected setup cannot run simulation"), RejectedMatch->SimulationStepsExecuted, uint64(0));
    World->DestroyWorld(false);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPolitikaMatchParticipantsAuthorityAutomationTest,
    "Politika.RTS.MatchParticipants.AuthorityAndStaleUnitLifecycle",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPolitikaMatchParticipantsAuthorityAutomationTest::RunTest(const FString& Parameters)
{
    using namespace politika::rules;
    UPolitikaPrototypeContentCatalog* Catalog = LoadObject<UPolitikaPrototypeContentCatalog>(nullptr,
        TEXT("/Game/Politika/Definitions/DA_PrototypeContentCatalog.DA_PrototypeContentCatalog"));
    TestNotNull(TEXT("Production content catalog is available"), Catalog);
    if (!Catalog) return false;
    const UPolitikaPrototypeMapDefinition* AuthoredMap =
        Catalog->FindMap(UPolitikaPrototypeContentValidator::PrototypeMapAssetId);
    if (!AuthoredMap) { TestNotNull(TEXT("Canonical map definition resolves"), AuthoredMap); return false; }

    TArray<FPolitikaPrototypeRuntimeParticipant> Participants;
    uint32 LocalPlayerId = 0;
    FName FailureCode;
    if (!APolitikaPrototypeMatch::BuildRuntimeParticipantsFromSlots(*AuthoredMap, Participants, LocalPlayerId, FailureCode))
    {
        AddError(FString::Printf(TEXT("Canonical participant roster failed: %s"), *FailureCode.ToString()));
        return false;
    }
    const FPolitikaPrototypeRuntimeParticipant* OpponentParticipant = Participants.FindByPredicate(
        [LocalPlayerId](const FPolitikaPrototypeRuntimeParticipant& Candidate)
        { return Candidate.RuntimePlayerId != LocalPlayerId; });
    const FPolitikaPrototypeRuntimeParticipant* LocalParticipant = Participants.FindByPredicate(
        [LocalPlayerId](const FPolitikaPrototypeRuntimeParticipant& Candidate)
        { return Candidate.RuntimePlayerId == LocalPlayerId; });
    if (!OpponentParticipant || !LocalParticipant) { AddError(TEXT("Local/opponent fixture identities are incomplete")); return false; }

    const UWorld::InitializationValues InitValues = UWorld::InitializationValues()
        .AllowAudioPlayback(false).RequiresHitProxies(false).CreatePhysicsScene(false)
        .CreateNavigation(false).CreateAISystem(false).ShouldSimulatePhysics(false)
        .SetTransactional(false).CreateFXSystem(false);
    UWorld* World = UWorld::CreateWorld(EWorldType::Game, false,
        FName(TEXT("PolitikaParticipantAuthorityAutomationWorld")), GetTransientPackage(), false,
        ERHIFeatureLevel::Num, &InitValues);
    TestNotNull(TEXT("Isolated authority-test world created"), World);
    if (!World) return false;

    APolitikaPrototypePlayerController* Controller = World->SpawnActor<APolitikaPrototypePlayerController>();
    APolitikaPrototypeMatch* Match = World->SpawnActor<APolitikaPrototypeMatch>();
    APolitikaPrototypeUnit* LocalUnit = World->SpawnActor<APolitikaPrototypeUnit>(FVector(0, 0, 42), FRotator::ZeroRotator);
    APolitikaPrototypeUnit* OpponentUnit = World->SpawnActor<APolitikaPrototypeUnit>(FVector(100, 0, 42), FRotator::ZeroRotator);
    APolitikaPrototypeUnit* SystemUnit = World->SpawnActor<APolitikaPrototypeUnit>(FVector(200, 0, 42), FRotator::ZeroRotator);
    APolitikaPrototypeUnit* DeadUnit = World->SpawnActor<APolitikaPrototypeUnit>(FVector(300, 0, 42), FRotator::ZeroRotator);
    APolitikaPrototypeUnit* QueuedUnit = World->SpawnActor<APolitikaPrototypeUnit>(FVector(400, 0, 42), FRotator::ZeroRotator);
    APolitikaPrototypeObjective* Objective = World->SpawnActor<APolitikaPrototypeObjective>(FVector(900, 900, 42), FRotator::ZeroRotator);
    if (!Controller || !Match || !LocalUnit || !OpponentUnit || !SystemUnit || !DeadUnit || !QueuedUnit || !Objective)
    {
        TestTrue(TEXT("All authority fixture actors spawn"), false);
        World->DestroyWorld(false);
        return false;
    }
    if (World->GetFirstPlayerController() != Controller) World->AddController(Controller);

    const auto Configure = [](APolitikaPrototypeUnit* Unit, uint64 StableId, uint32 Owner, FName Faction)
    {
        Unit->Configure(StableId, Owner, FText::FromString(TEXT("Authority fixture")), 100, 100, 200, 10, 0.5f,
            true, nullptr, FVector::OneVector, FLinearColor::White, Faction);
    };
    Configure(LocalUnit, 501, LocalPlayerId, LocalParticipant->FactionId);
    Configure(OpponentUnit, 502, OpponentParticipant->RuntimePlayerId, OpponentParticipant->FactionId);
    Configure(SystemUnit, 503, 0, TEXT("faction.unaligned"));
    Configure(DeadUnit, 504, LocalPlayerId, LocalParticipant->FactionId);
    Configure(QueuedUnit, 505, LocalPlayerId, LocalParticipant->FactionId);

    Match->RuntimeParticipants = Participants;
    Match->LocalRuntimePlayerId = LocalPlayerId;
    Match->UnalignedSystemAuthority = {TEXT("faction.unaligned"), true};
    Match->Units = {LocalUnit, OpponentUnit, SystemUnit, DeadUnit, QueuedUnit};
    Match->Objective = Objective;
    Match->PlayableMin = FVector2D(-1000, -1000);
    Match->PlayableMax = FVector2D(1000, 1000);
    Match->MatchGeneration = 51;
    Match->bContentReady = true;
    TestTrue(TEXT("Fixture enters Active through legal lifecycle transitions"),
        Match->TransitionLifecycle(MatchPhase::SetupValidated)
        && Match->TransitionLifecycle(MatchPhase::Loading)
        && Match->TransitionLifecycle(MatchPhase::Starting)
        && Match->TransitionLifecycle(MatchPhase::Active));

    TestTrue(TEXT("Real world first PlayerController is bound as local human"), Match->IsLocalControllerBound(Controller));
    Controller->SelectUnitForInput(LocalUnit);
    TestEqual(TEXT("Bound controller selects its living owned unit"), Match->GetSelectedUnitForPlayer(LocalPlayerId), LocalUnit);
    TestTrue(TEXT("Selected local unit visual is active"), LocalUnit->SelectionIndicator->IsVisible());
    TestTrue(TEXT("Bound local command reaches typed admission"),
        Match->SubmitLocalControllerCommand(Controller, LocalUnit, nullptr, GuardOrder{}));
    Match->Tick(0.05f);
    TestEqual(TEXT("Valid local Guard command executes through the coordinator"), LocalUnit->GetCurrentOrder(), EPolitikaPrototypeOrder::Guard);

    Match->SubmitCommand(OpponentParticipant->RuntimePlayerId, OpponentUnit, nullptr, GuardOrder{});
    Match->Tick(0.05f);
    TestEqual(TEXT("Opponent fixture has a legitimate order admitted through the coordinator"),
        OpponentUnit->GetCurrentOrder(), EPolitikaPrototypeOrder::Guard);
    const TArray<FVector> OriginalRoute = OpponentUnit->NavigationWaypoints;
    const int32 OriginalCursor = OpponentUnit->NavigationWaypointIndex;
    const uint64 OriginalRouteSequence = OpponentUnit->NavigationOrderSequence;
    const FVector OriginalOpponentPosition = OpponentUnit->GetActorLocation();
    Controller->SelectUnitForInput(OpponentUnit);
    TestNull(TEXT("Bound controller cannot place opposing unit in local selection"), Match->GetSelectedUnitForPlayer(LocalPlayerId));
    TestFalse(TEXT("Foreign unit selection visual remains off"), OpponentUnit->SelectionIndicator->IsVisible());
    TestTrue(TEXT("Bound-controller foreign typed request reaches authoritative rejection boundary"),
        Match->SubmitLocalControllerCommand(Controller, OpponentUnit, nullptr, MoveOrder{{200, 200}}));
    const CommandOutcomeEvent* ForeignOutcome = nullptr;
    for (const GameplayEvent& Event : Match->PublishedEvents)
        if (const auto* Outcome = std::get_if<CommandOutcomeEvent>(&Event); Outcome && Outcome->issuer_id == OpponentUnit->GetStableId())
            ForeignOutcome = Outcome;
    TestNotNull(TEXT("Foreign typed request publishes a rejection outcome"), ForeignOutcome);
    if (ForeignOutcome)
    {
        TestFalse(TEXT("Foreign typed request is rejected"), ForeignOutcome->accepted);
        TestEqual(TEXT("Foreign typed request reports NotOwned"), ForeignOutcome->rejection, OrderRejection::NotOwned);
    }
    TestEqual(TEXT("Foreign rejection preserves existing opposing order"), OpponentUnit->GetCurrentOrder(), EPolitikaPrototypeOrder::Guard);
    TestEqual(TEXT("Foreign rejection preserves route identity"), OpponentUnit->NavigationOrderSequence, OriginalRouteSequence);
    TestEqual(TEXT("Foreign rejection preserves route cursor"), OpponentUnit->NavigationWaypointIndex, OriginalCursor);
    TestTrue(TEXT("Foreign rejection preserves route points"), OpponentUnit->NavigationWaypoints == OriginalRoute);
    TestTrue(TEXT("Foreign rejection does not alter the opponent's legitimate fixed-step trajectory"),
        OpponentUnit->GetActorLocation().Equals(OriginalOpponentPosition, 0.01f));

    Controller->SelectUnitForInput(SystemUnit);
    TestNull(TEXT("Bound local controller cannot select the system-owned Unaligned fixture"),
        Match->GetSelectedUnitForPlayer(LocalPlayerId));
    TestFalse(TEXT("System-owned unit does not receive local selection visual"), SystemUnit->SelectionIndicator->IsVisible());
    const int32 BeforeSystemEvents = Match->PublishedEvents.Num();
    const FVector OriginalSystemPosition = SystemUnit->GetActorLocation();
    const int32 OriginalSystemHealth = SystemUnit->GetHealth();
    TestTrue(TEXT("System-unit attempt from bound local controller is forwarded for authoritative rejection"),
        Match->SubmitLocalControllerCommand(Controller, SystemUnit, nullptr, GuardOrder{}));
    const CommandOutcomeEvent* SystemOutcome = nullptr;
    for (int32 Index = BeforeSystemEvents; Index < Match->PublishedEvents.Num(); ++Index)
        if (const auto* Outcome = std::get_if<CommandOutcomeEvent>(&Match->PublishedEvents[Index]); Outcome && Outcome->issuer_id == SystemUnit->GetStableId())
            SystemOutcome = Outcome;
    TestNotNull(TEXT("System-unit request publishes typed rejection"), SystemOutcome);
    if (SystemOutcome) TestEqual(TEXT("System-owned unit is rejected as not owned"), SystemOutcome->rejection, OrderRejection::NotOwned);
    const int32 BeforeUnassignedEvents = Match->PublishedEvents.Num();
    Match->SubmitCommand(0, SystemUnit, nullptr, GuardOrder{});
    TestEqual(TEXT("Unassigned participant command is rejected immediately"), Match->PublishedEvents.Num(), BeforeUnassignedEvents + 1);
    if (Match->PublishedEvents.Num() > BeforeUnassignedEvents)
        if (const auto* Outcome = std::get_if<CommandOutcomeEvent>(&Match->PublishedEvents.Last()))
            TestEqual(TEXT("Unassigned participant reports Unauthorized"), Outcome->rejection, OrderRejection::Unauthorized);
    const int32 BeforeUnboundEvents = Match->PublishedEvents.Num();
    TestFalse(TEXT("Null/unbound controller cannot submit a command"),
        Match->SubmitLocalControllerCommand(nullptr, LocalUnit, nullptr, StopOrder{}));
    TestEqual(TEXT("Pre-typed unbound rejection publishes no command outcome"), Match->PublishedEvents.Num(), BeforeUnboundEvents);

    Controller->SelectUnitForInput(DeadUnit);
    TestEqual(TEXT("Living local unit can be selected before death"), Match->GetSelectedUnitForPlayer(LocalPlayerId), DeadUnit);
    DeadUnit->SetGuardOrder();
    DeadUnit->PendingDamage = static_cast<float>(DeadUnit->GetHealth());
    TestEqual(TEXT("Production pending-damage application reaches dead-but-existing state"), DeadUnit->ApplyPendingDamage(), 100);
    TestTrue(TEXT("Dead actor still exists for deferred command validation"), IsValid(DeadUnit));
    TestFalse(TEXT("Dead actor is unavailable"), DeadUnit->IsAlive());
    const int32 BeforeDeadInputEvents = Match->PublishedEvents.Num();
    Controller->GuardSelected();
    TestNull(TEXT("Dead selected unit is lazily removed from match selection"), Match->GetSelectedUnitForPlayer(LocalPlayerId));
    TestFalse(TEXT("Dead selected unit is no longer cached by PlayerController"), Controller->SelectedUnit.IsValid());
    TestEqual(TEXT("Ordinary input cannot submit against dead cached selection"), Match->PublishedEvents.Num(), BeforeDeadInputEvents);
    const int32 BeforeDeadAbilityInput = Match->PublishedEvents.Num();
    Controller->SelectedUnit = DeadUnit;
    Controller->GrenadeSelected();
    TestTrue(TEXT("Grenade input retires dead cached selection before submission"), !Controller->SelectedUnit.IsValid());
    TestTrue(TEXT("Grenade input cannot queue work for a dead cached selection"), Match->PendingOrders.IsEmpty());
    Controller->SelectedUnit = DeadUnit;
    Controller->ReloadSelected();
    TestTrue(TEXT("Reload input retires dead cached selection before submission"), !Controller->SelectedUnit.IsValid());
    TestTrue(TEXT("Reload input cannot queue work for a dead cached selection"), Match->PendingOrders.IsEmpty());
    Controller->SelectedUnit = DeadUnit;
    Controller->SprintSelected();
    TestTrue(TEXT("Sprint input retires dead cached selection before submission"), !Controller->SelectedUnit.IsValid());
    TestTrue(TEXT("Sprint input cannot queue work for a dead cached selection"), Match->PendingOrders.IsEmpty());
    TestEqual(TEXT("Stale ability inputs publish no command outcomes"), Match->PublishedEvents.Num(), BeforeDeadAbilityInput);
    TestTrue(TEXT("Explicit queued dead-issuer request enters typed admission"),
        Match->SubmitLocalControllerCommand(Controller, DeadUnit, nullptr, GuardOrder{}));
    Match->Tick(0.05f);
    int32 DeadIssuerOutcomes = 0;
    for (const GameplayEvent& Event : Match->PublishedEvents)
        if (const auto* Outcome = std::get_if<CommandOutcomeEvent>(&Event); Outcome && Outcome->issuer_id == 504)
        {
            ++DeadIssuerOutcomes;
            TestEqual(TEXT("Dead-but-existing issuer reports IssuerUnavailable"), Outcome->rejection, OrderRejection::IssuerUnavailable);
        }
    TestEqual(TEXT("Dead issuer produces exactly one command outcome"), DeadIssuerOutcomes, 1);

    Controller->SelectUnitForInput(QueuedUnit);
    TestTrue(TEXT("Local unit is selected before invalidating its queued issuer"), Controller->SelectedUnit.Get() == QueuedUnit);
    QueuedUnit->SetGuardOrder();
    const int32 BeforePending = Match->PublishedEvents.Num();
    TestTrue(TEXT("Command queues before issuer destruction"),
        Match->SubmitLocalControllerCommand(Controller, QueuedUnit, nullptr, GuardOrder{}));
    TestEqual(TEXT("Typed request is pending before destruction"), Match->PendingOrders.Num(), 1);
    QueuedUnit->Destroy();
    Match->Tick(0.05f);
    int32 MissingIssuerOutcomes = 0;
    for (const GameplayEvent& Event : Match->PublishedEvents)
        if (const auto* Outcome = std::get_if<CommandOutcomeEvent>(&Event); Outcome && Outcome->issuer_id == 505)
        {
            ++MissingIssuerOutcomes;
            TestFalse(TEXT("Destroyed queued issuer is not accepted"), Outcome->accepted);
            TestEqual(TEXT("Destroyed queued issuer reports MissingIssuer"), Outcome->rejection, OrderRejection::MissingIssuer);
        }
    TestEqual(TEXT("Destroyed pending issuer produces exactly one command outcome"), MissingIssuerOutcomes, 1);
    TestEqual(TEXT("Destroyed pending request is retired"), Match->PendingOrders.Num(), 0);
    TestFalse(TEXT("Destroyed issuer leaves no controller selection"), Controller->SelectedUnit.IsValid());
    TestNull(TEXT("Destroyed issuer leaves no match selection"), Match->GetSelectedUnitForPlayer(LocalPlayerId));

    APolitikaPrototypeUnit* OrphanUnit = World->SpawnActor<APolitikaPrototypeUnit>(FVector(500, 0, 42), FRotator::ZeroRotator);
    Configure(OrphanUnit, 506, LocalPlayerId, LocalParticipant->FactionId);
    Controller->SelectUnitForInput(OrphanUnit);
    TestNull(TEXT("Alive but non-member actor cannot be selected"), Match->GetSelectedUnitForPlayer(LocalPlayerId));
    const int32 BeforeOrphan = Match->PublishedEvents.Num();
    TestTrue(TEXT("Bound attempt against non-member unit is handled as typed rejection"),
        Match->SubmitLocalControllerCommand(Controller, OrphanUnit, nullptr, GuardOrder{}));
    TestEqual(TEXT("Non-member issuer is rejected as missing"), Match->PublishedEvents.Num(), BeforeOrphan + 1);
    if (Match->PublishedEvents.Num() > BeforeOrphan)
        if (const auto* Outcome = std::get_if<CommandOutcomeEvent>(&Match->PublishedEvents.Last()))
            TestEqual(TEXT("Non-member unit reports MissingIssuer"), Outcome->rejection, OrderRejection::MissingIssuer);

    for (int32 Step = 0; Step < 20; ++Step) Match->Tick(0.05f);
    TestEqual(TEXT("No invalid queued authority request remains after 20 steps"), Match->PendingOrders.Num(), 0);
    TestEqual(TEXT("Opponent order remains unchanged through observation window"), OpponentUnit->GetCurrentOrder(), EPolitikaPrototypeOrder::Guard);
    TestTrue(TEXT("System unit never receives executable order"), SystemUnit->GetCurrentOrder() == EPolitikaPrototypeOrder::Idle);
    TestEqual(TEXT("System unit owner remains unassigned"), SystemUnit->GetOwnerPlayer(), uint32(0));
    TestEqual(TEXT("System unit faction remains Unaligned"), SystemUnit->GetFactionId(), FName(TEXT("faction.unaligned")));
    TestEqual(TEXT("Rejected system command preserves health"), SystemUnit->GetHealth(), OriginalSystemHealth);
    TestTrue(TEXT("Rejected system command causes no movement"), SystemUnit->GetActorLocation().Equals(OriginalSystemPosition, 0.01f));
    TestTrue(TEXT("Unaligned system-authority record is active and separate from player roster"),
        Match->GetUnalignedSystemAuthority().bNpcSystemControlled
        && Match->GetUnalignedSystemAuthority().FactionId == FName(TEXT("faction.unaligned"))
        && !Match->RuntimeParticipants.ContainsByPredicate([](const FPolitikaPrototypeRuntimeParticipant& Participant)
            { return Participant.RuntimePlayerId == 0 || Participant.FactionId == FName(TEXT("faction.unaligned")); }));
    World->DestroyWorld(false);
    return true;
}

#endif
