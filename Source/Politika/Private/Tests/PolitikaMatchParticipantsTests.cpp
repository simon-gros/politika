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

#endif
