#if WITH_DEV_AUTOMATION_TESTS

#include "PolitikaPrototypeGameplay.h"
#include "PolitikaPrototypeContent.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Misc/AutomationTest.h"

namespace
{
UWorld* CreateAdmissionTestWorld(const TCHAR* Name)
{
    const UWorld::InitializationValues InitValues = UWorld::InitializationValues()
        .AllowAudioPlayback(false).RequiresHitProxies(false).CreatePhysicsScene(false)
        .CreateNavigation(false).CreateAISystem(false).ShouldSimulatePhysics(false)
        .SetTransactional(false).CreateFXSystem(false);
    return UWorld::CreateWorld(EWorldType::Game, false, FName(Name), GetTransientPackage(), false,
        ERHIFeatureLevel::Num, &InitValues);
}

bool HasDiagnostic(const TArray<FPolitikaContentDiagnostic>& Diagnostics, FName Code,
    const FString& PropertyPath, const FString& Actual)
{
    return Diagnostics.ContainsByPredicate([&](const FPolitikaContentDiagnostic& D)
    {
        return D.Code == Code && D.AssetId == UPolitikaPrototypeContentValidator::PrototypeMapAssetId
            && D.PropertyPath == PropertyPath && (Actual.IsEmpty() || D.Actual.Contains(Actual))
            && D.Severity == EPolitikaContentDiagnosticSeverity::Error;
    });
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPolitikaAuthoredStartupAdmissionAutomationTest,
    "Politika.RTS.MatchParticipants.AuthoredStartupAdmission",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPolitikaAuthoredStartupAdmissionAutomationTest::RunTest(const FString& Parameters)
{
    UPolitikaPrototypeContentCatalog* Catalog = LoadObject<UPolitikaPrototypeContentCatalog>(nullptr,
        TEXT("/Game/Politika/Definitions/DA_PrototypeContentCatalog.DA_PrototypeContentCatalog"));
    TestNotNull(TEXT("Production catalog resolves before startup injection"), Catalog);
    if (!Catalog) return false;
    const UPolitikaPrototypeMapDefinition* Canonical = Catalog->FindMap(UPolitikaPrototypeContentValidator::PrototypeMapAssetId);
    TestNotNull(TEXT("Canonical production map resolves before startup injection"), Canonical);
    if (!Canonical) return false;

    UPolitikaPrototypeMapDefinition* CookedFixtureSource = LoadObject<UPolitikaPrototypeMapDefinition>(nullptr,
        TEXT("/Game/Politika/TestFixtures/DA_InvalidParticipantFaction.DA_InvalidParticipantFaction"));
    TestNotNull(TEXT("Authored participant fixture exists as a real Unreal asset"), CookedFixtureSource);
    if (!CookedFixtureSource) return false;
    TestEqual(TEXT("Fixture has its isolated stable map ID"), CookedFixtureSource->StableId, FName(TEXT("map.test.invalid_participant_faction")));
    TestEqual(TEXT("Fixture retains the supported schema"), CookedFixtureSource->SchemaVersion, Canonical->SchemaVersion);
    TestEqual(TEXT("Fixture retains the canonical level reference"), CookedFixtureSource->LevelAsset, Canonical->LevelAsset);
    TestEqual(TEXT("Fixture retains canonical playable slots"), CookedFixtureSource->PlayerSlots.Num(), Canonical->PlayerSlots.Num());
    TestEqual(TEXT("Fixture retains canonical placement count"), CookedFixtureSource->UnitPlacements.Num(), Canonical->UnitPlacements.Num());
    bool bOnlyPlacementArchetypeDiffers = CookedFixtureSource->UnitPlacements.Num() == Canonical->UnitPlacements.Num();
    for (int32 Index = 0; bOnlyPlacementArchetypeDiffers && Index < Canonical->UnitPlacements.Num(); ++Index)
    {
        const FPolitikaPrototypeUnitPlacement& Expected = Canonical->UnitPlacements[Index];
        const FPolitikaPrototypeUnitPlacement& Actual = CookedFixtureSource->UnitPlacements[Index];
        bOnlyPlacementArchetypeDiffers = Expected.SlotId == Actual.SlotId
            && Expected.Location.Equals(Actual.Location) && Expected.Rotation.Equals(Actual.Rotation)
            && (Index == 0 ? Actual.UnitArchetypeId == FName(TEXT("unit.opposing_patrol"))
                : Expected.UnitArchetypeId == Actual.UnitArchetypeId);
    }
    TestTrue(TEXT("Fixture differs in placement archetype only, leaving placement slots/transforms intact"), bOnlyPlacementArchetypeDiffers);
    FPolitikaPrototypeContentSnapshot FixtureSnapshot;
    TArray<FPolitikaContentDiagnostic> FixtureDiagnostics;
    TestFalse(TEXT("Real loader rejects the authored fixture through shared validation"),
        FPolitikaPrototypeContentLoader::LoadAndValidate(nullptr, FName(TEXT("map.test.invalid_participant_faction")), FixtureSnapshot, FixtureDiagnostics));
    TestEqual(TEXT("Authored fixture has exactly the intended single diagnostic"), FixtureDiagnostics.Num(), 1);
    TestTrue(TEXT("Authored fixture diagnostic names faction mismatch and intended property/value"),
        FixtureDiagnostics.ContainsByPredicate([](const FPolitikaContentDiagnostic& D)
        {
            return D.Code == TEXT("CONTENT.FACTION.MISMATCH")
                && D.AssetId == TEXT("map.test.invalid_participant_faction")
                && D.PropertyPath == TEXT("UnitPlacements[0].UnitArchetypeId")
                && D.Actual.Contains(TEXT("faction.opposition"))
                && D.Severity == EPolitikaContentDiagnosticSeverity::Error;
        }));

    struct FCase
    {
        const TCHAR* Name;
        FName Code;
        FString Path;
        FString Actual;
        TFunction<void(UPolitikaPrototypeMapDefinition&)> Mutate;
    };
    const TArray<FCase> Cases = {
        {TEXT("duplicate slot ID"), TEXT("MAP.SLOT.DUPLICATE"), TEXT("PlayerSlots.SlotId"), TEXT("occurs"),
            [](UPolitikaPrototypeMapDefinition& M) { M.PlayerSlots[1].SlotId = M.PlayerSlots[0].SlotId; }},
        {TEXT("duplicate runtime participant ID"), TEXT("MAP.PLAYER.DUPLICATE"), TEXT("PlayerSlots.RuntimePlayerId"), TEXT("occurs"),
            [](UPolitikaPrototypeMapDefinition& M) { M.PlayerSlots[1].RuntimePlayerId = M.PlayerSlots[0].RuntimePlayerId; }},
        {TEXT("duplicate playable faction"), TEXT("MAP.FACTION.DUPLICATE"), TEXT("PlayerSlots.FactionId"), TEXT("occurs"),
            [](UPolitikaPrototypeMapDefinition& M) { M.PlayerSlots[1].FactionId = M.PlayerSlots[0].FactionId; }},
        {TEXT("no local-human slot"), TEXT("MAP.SLOTS.HUMAN_COUNT"), TEXT("PlayerSlots.bHumanControlled"), TEXT("0"),
            [](UPolitikaPrototypeMapDefinition& M) { for (auto& S : M.PlayerSlots) S.bHumanControlled = false; }},
        {TEXT("multiple local-human slots"), TEXT("MAP.SLOTS.HUMAN_COUNT"), TEXT("PlayerSlots.bHumanControlled"), TEXT("2"),
            [](UPolitikaPrototypeMapDefinition& M) { for (auto& S : M.PlayerSlots) S.bHumanControlled = true; }},
        {TEXT("Unaligned playable slot"), TEXT("MAP.FACTION.UNALIGNED_PLAYABLE"), TEXT("PlayerSlots[0].FactionId"), TEXT("faction.unaligned"),
            [](UPolitikaPrototypeMapDefinition& M) { M.PlayerSlots[0].FactionId = TEXT("faction.unaligned"); }},
        {TEXT("authored unit faction does not match its participant slot"), TEXT("CONTENT.FACTION.MISMATCH"),
            TEXT("UnitPlacements[0].UnitArchetypeId"), TEXT("faction.opposition"),
            [](UPolitikaPrototypeMapDefinition& M) { M.UnitPlacements[0].UnitArchetypeId = TEXT("unit.opposing_patrol"); }},
        {TEXT("unknown unit-placement slot"), TEXT("MAP.SLOT.REFERENCE_UNRESOLVED"), TEXT("UnitPlacements[0].SlotId"), TEXT("9999"),
            [](UPolitikaPrototypeMapDefinition& M) { M.UnitPlacements[0].SlotId = 9999; }}
    };

    // Rejection diagnostics are intentionally logged at Error severity by the
    // production startup path; tell Automation they are expected test output.
    AddExpectedError(TEXT("CONTENT_VALIDATION "), EAutomationExpectedErrorFlags::Contains, 13);
    AddExpectedError(TEXT("MATCH_ADMISSION_REJECTED"), EAutomationExpectedErrorFlags::Contains, Cases.Num());

    int32 CaseIndex = 0;
    for (const FCase& Case : Cases)
    {
        UPolitikaPrototypeMapDefinition* Candidate = DuplicateObject<UPolitikaPrototypeMapDefinition>(Canonical, GetTransientPackage());
        Case.Mutate(*Candidate);
        UWorld* World = CreateAdmissionTestWorld(*FString::Printf(TEXT("PolitikaAuthoredAdmission_%d"), CaseIndex++));
        TestNotNull(FString::Printf(TEXT("%s test world exists"), Case.Name), World);
        if (!World) continue;
        APolitikaPrototypeMatch* Match = World->SpawnActor<APolitikaPrototypeMatch>();
        TestNotNull(FString::Printf(TEXT("%s coordinator exists"), Case.Name), Match);
        if (Match)
        {
            FPolitikaPrototypeContentLoader::SetAutomationMapCandidate(Candidate);
            Match->BuildScenario(); // Actual coordinator startup, with the normal loader and shared validator.
            FPolitikaPrototypeContentLoader::ClearAutomationMapCandidate();

            TestTrue(FString::Printf(TEXT("%s selected canonical map identity"), Case.Name),
                HasDiagnostic(Match->LastAdmissionDiagnostics, Case.Code, Case.Path, Case.Actual));
            TestTrue(FString::Printf(TEXT("%s rejects before Active"), Case.Name),
                Match->GetMatchPhase() == politika::rules::MatchPhase::AdmissionRejected);
            TestEqual(FString::Printf(TEXT("%s publishes no participants"), Case.Name), Match->RuntimeParticipants.Num(), 0);
            TestEqual(FString::Printf(TEXT("%s has no local authority"), Case.Name), Match->LocalRuntimePlayerId, uint32(0));
            TestFalse(FString::Printf(TEXT("%s admits no Unaligned system authority"), Case.Name), Match->UnalignedSystemAuthority.bNpcSystemControlled);
            TestEqual(FString::Printf(TEXT("%s has no authoritative units"), Case.Name), Match->Units.Num(), 0);
            TestNull(FString::Printf(TEXT("%s has no objective"), Case.Name), Match->Objective.Get());
            TestEqual(FString::Printf(TEXT("%s has no selection"), Case.Name), Match->SelectionByPlayer.Num(), 0);
            TestEqual(FString::Printf(TEXT("%s has no pending commands"), Case.Name), Match->PendingOrders.Num(), 0);
            TestFalse(FString::Printf(TEXT("%s is not content-ready"), Case.Name), Match->bContentReady);
            TestEqual(FString::Printf(TEXT("%s simulation accumulator is clear"), Case.Name), Match->StepAccumulator, 0.0f);
            TestEqual(FString::Printf(TEXT("%s executed no simulation steps"), Case.Name), Match->SimulationStepsExecuted, uint64(0));
            TestFalse(FString::Printf(TEXT("%s has no terminal result"), Case.Name), Match->CommittedResult.IsSet());
            TestEqual(FString::Printf(TEXT("%s admitted no command generation"), Case.Name), Match->NextCommandSequence, uint64(1));
            TestEqual(FString::Printf(TEXT("%s published no gameplay events"), Case.Name), Match->PublishedEvents.Num(), 0);
            TestEqual(FString::Printf(TEXT("%s queued no gameplay events"), Case.Name), Match->PendingGameplayEvents.Num(), 0);
            int32 WorldUnits = 0, WorldObjectives = 0;
            for (TActorIterator<APolitikaPrototypeUnit> It(World); It; ++It) if (!It->IsActorBeingDestroyed()) ++WorldUnits;
            for (TActorIterator<APolitikaPrototypeObjective> It(World); It; ++It) if (!It->IsActorBeingDestroyed()) ++WorldObjectives;
            TestEqual(FString::Printf(TEXT("%s leaves no live world units"), Case.Name), WorldUnits, 0);
            TestEqual(FString::Printf(TEXT("%s leaves no live world objectives"), Case.Name), WorldObjectives, 0);
        }
        World->DestroyWorld(false);
    }

    // Exercise the reachable post-roster boundary: valid authored structure is
    // admitted into the candidate roster, then spatial validation fails in
    // this deliberately unlevelled test world. Recovery must retire that
    // committed authority before a subsequent startup resolves canonical data.
    UWorld* RetryWorld = CreateAdmissionTestWorld(TEXT("PolitikaAuthoredAdmissionRetry"));
    TestNotNull(TEXT("Retry world exists"), RetryWorld);
    if (RetryWorld)
    {
        TArray<FPolitikaContentDiagnostic> SpatialDiagnostics;
        UPolitikaPrototypeContentValidator::ValidateMapSpatial(RetryWorld, Canonical, SpatialDiagnostics);
        AddExpectedError(TEXT("CONTENT_VALIDATION "), EAutomationExpectedErrorFlags::Contains, SpatialDiagnostics.Num() + 1);
        AddExpectedError(TEXT("MATCH_ADMISSION_REJECTED"), EAutomationExpectedErrorFlags::Contains, 1);

        UPolitikaPrototypeMapDefinition* Invalid = DuplicateObject<UPolitikaPrototypeMapDefinition>(Canonical, GetTransientPackage());
        Invalid->PlayerSlots[1].RuntimePlayerId = Invalid->PlayerSlots[0].RuntimePlayerId;
        APolitikaPrototypeMatch* Rejected = RetryWorld->SpawnActor<APolitikaPrototypeMatch>();
        FPolitikaPrototypeContentLoader::SetAutomationMapCandidate(Invalid);
        Rejected->BuildScenario();
        FPolitikaPrototypeContentLoader::ClearAutomationMapCandidate();
        TestEqual(TEXT("Invalid first startup is rejected"), Rejected->GetMatchPhase(), politika::rules::MatchPhase::AdmissionRejected);

        APolitikaPrototypeMatch* CanonicalRetry = RetryWorld->SpawnActor<APolitikaPrototypeMatch>();
        CanonicalRetry->BuildScenario();
        TestTrue(TEXT("One-shot candidate is consumed; retry resolves the canonical map through production loading"),
            CanonicalRetry->MapDefinition == Canonical && HasDiagnostic(CanonicalRetry->LastAdmissionDiagnostics,
                TEXT("MAP.LEVEL.MISMATCH"), TEXT("LevelAsset"), FString()));
        TestEqual(TEXT("Later spatial rejection retires the roster committed by startup"), CanonicalRetry->RuntimeParticipants.Num(), 0);
        TestEqual(TEXT("Later spatial rejection clears local authority"), CanonicalRetry->LocalRuntimePlayerId, uint32(0));
        TestFalse(TEXT("Later spatial rejection clears Unaligned authority"), CanonicalRetry->UnalignedSystemAuthority.bNpcSystemControlled);
        TestFalse(TEXT("Later spatial rejection leaves content unready"), CanonicalRetry->bContentReady);
        TestEqual(TEXT("Later spatial rejection leaves no authoritative units"), CanonicalRetry->Units.Num(), 0);
        TestNull(TEXT("Later spatial rejection leaves no authoritative objective"), CanonicalRetry->Objective.Get());
        RetryWorld->DestroyWorld(false);
    }
    FPolitikaPrototypeContentLoader::ClearAutomationMapCandidate();
    return true;
}

#endif
