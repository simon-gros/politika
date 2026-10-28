#if WITH_DEV_AUTOMATION_TESTS

#include "PolitikaPrototypeContent.h"
#include "PolitikaPrototypeGameplay.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Misc/AutomationTest.h"

namespace
{
bool HasExactDiagnostic(const TArray<FPolitikaContentDiagnostic>& Diagnostics, FName Code, FName AssetId, const FString& PropertyPath)
{
    return Diagnostics.ContainsByPredicate([&](const FPolitikaContentDiagnostic& Diagnostic)
    {
        return Diagnostic.Code == Code && Diagnostic.AssetId == AssetId && Diagnostic.PropertyPath == PropertyPath;
    });
}

UPolitikaPrototypeContentCatalog* MakeValidTestCatalog(UPolitikaWeaponDefinition*& OutWeapon, UPolitikaUnitArchetypeDefinition*& OutUnit)
{
    UStaticMesh* Mesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
    OutWeapon = NewObject<UPolitikaWeaponDefinition>();
    OutWeapon->StableId = TEXT("weapon.test_rifle");
    OutWeapon->DisplayName = FText::FromString(TEXT("Test Rifle"));
    OutWeapon->RangeCm = 500.0f;
    OutWeapon->Damage = 10.0f;
    OutWeapon->CooldownSeconds = 1.0f;
    OutUnit = NewObject<UPolitikaUnitArchetypeDefinition>();
    OutUnit->StableId = TEXT("unit.test_infantry");
    OutUnit->FactionId = TEXT("faction.test");
    OutUnit->DisplayName = FText::FromString(TEXT("Test Infantry"));
    OutUnit->MaxHealth = 100;
    OutUnit->MovementSpeedCmPerSecond = 200.0f;
    OutUnit->WeaponId = OutWeapon->StableId;
    OutUnit->VisualMesh = Mesh;
    UPolitikaPrototypeContentCatalog* Catalog = NewObject<UPolitikaPrototypeContentCatalog>();
    Catalog->StableId = UPolitikaPrototypeContentCatalog::CatalogAssetId;
    Catalog->Weapons.Add(OutWeapon);
    Catalog->Units.Add(OutUnit);
    return Catalog;
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPolitikaPrototypeContentResolutionTest,
    "Politika.Content.ValidTypedDefinitionsResolve",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPolitikaPrototypeContentResolutionTest::RunTest(const FString& Parameters)
{
    UStaticMesh* Mesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
    UPolitikaWeaponDefinition* Weapon = NewObject<UPolitikaWeaponDefinition>();
    Weapon->StableId = TEXT("weapon.test_rifle"); Weapon->DisplayName = FText::FromString(TEXT("Test Rifle"));
    Weapon->RangeCm = 500; Weapon->Damage = 10; Weapon->CooldownSeconds = 1;
    UPolitikaUnitArchetypeDefinition* Unit = NewObject<UPolitikaUnitArchetypeDefinition>();
    Unit->StableId = TEXT("unit.test_infantry"); Unit->FactionId = TEXT("faction.test");
    Unit->DisplayName = FText::FromString(TEXT("Test Infantry")); Unit->MaxHealth = 100;
    Unit->MovementSpeedCmPerSecond = 200; Unit->WeaponId = Weapon->StableId; Unit->VisualMesh = Mesh;
    UPolitikaPrototypeContentCatalog* Catalog = NewObject<UPolitikaPrototypeContentCatalog>();
    Catalog->StableId = UPolitikaPrototypeContentCatalog::CatalogAssetId;
    Catalog->Weapons.Add(Weapon); Catalog->Units.Add(Unit);
    TestTrue(TEXT("Typed weapon reference resolves"), Catalog->FindWeapon(Unit->WeaponId) == Weapon);
    TestEqual(TEXT("Primary asset ID uses explicit content ID"), Unit->GetPrimaryAssetId().PrimaryAssetName, Unit->StableId);
    TArray<FPolitikaContentDiagnostic> Diagnostics;
    UPolitikaPrototypeContentValidator::ValidateCatalog(Catalog, Diagnostics);
    TestFalse(TEXT("Valid unit/weapon catalog passes structural validation"), UPolitikaPrototypeContentValidator::HasErrors(Diagnostics));
    TestEqual(TEXT("Original health is retained"), Unit->MaxHealth, 100);
    TestEqual(TEXT("Original movement speed is retained"), Unit->MovementSpeedCmPerSecond, 200.0f);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPolitikaPrototypeMapPositiveTest,
    "Politika.Content.PrototypeMapDefinitionPasses",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPolitikaPrototypeMapPositiveTest::RunTest(const FString& Parameters)
{
    UPolitikaPrototypeContentCatalog* Catalog = LoadObject<UPolitikaPrototypeContentCatalog>(nullptr,
        TEXT("/Game/Politika/Definitions/DA_PrototypeContentCatalog.DA_PrototypeContentCatalog"));
    TestNotNull(TEXT("Authored catalog asset loads"), Catalog);
    if (!Catalog) return false;
    const UPolitikaPrototypeMapDefinition* Map = Catalog->FindMap(UPolitikaPrototypeContentValidator::PrototypeMapAssetId);
    TestNotNull(TEXT("Stable map ID resolves from catalog"), Map);
    if (!Map) return false;
    TArray<FPolitikaContentDiagnostic> Diagnostics;
    UPolitikaPrototypeContentValidator::ValidateMapStructure(Map, Catalog, Diagnostics);
    TestFalse(TEXT("Authored versioned prototype map passes structural validation"), UPolitikaPrototypeContentValidator::HasErrors(Diagnostics));
    TestEqual(TEXT("Map schema version is explicit and supported"), Map->SchemaVersion, UPolitikaPrototypeMapDefinition::CurrentSchemaVersion);
    const UPolitikaUnitArchetypeDefinition* Recon = Catalog->FindUnit(TEXT("unit.recon_vehicle"));
    const UPolitikaUnitArchetypeDefinition* Fireteam = Catalog->FindUnit(TEXT("unit.mechanized_fireteam"));
    const UPolitikaUnitArchetypeDefinition* Patrol = Catalog->FindUnit(TEXT("unit.opposing_patrol"));
    TestNotNull(TEXT("Recon definition resolves"), Recon);
    TestNotNull(TEXT("Fireteam definition resolves"), Fireteam);
    TestNotNull(TEXT("Patrol definition resolves"), Patrol);
    if (Recon && Fireteam && Patrol)
    {
        TestEqual(TEXT("Recon health preserves prototype value"), Recon->MaxHealth, 120);
        TestEqual(TEXT("Recon speed preserves prototype value"), Recon->MovementSpeedCmPerSecond, 340.0f);
        TestEqual(TEXT("Fireteam health preserves prototype value"), Fireteam->MaxHealth, 180);
        TestEqual(TEXT("Fireteam speed preserves prototype value"), Fireteam->MovementSpeedCmPerSecond, 210.0f);
        TestEqual(TEXT("Patrol health preserves prototype value"), Patrol->MaxHealth, 150);
        TestEqual(TEXT("Patrol speed preserves prototype value"), Patrol->MovementSpeedCmPerSecond, 180.0f);
    }
    const UPolitikaWeaponDefinition* ReconWeapon = Catalog->FindWeapon(TEXT("weapon.recon_cannon"));
    const UPolitikaWeaponDefinition* FireteamWeapon = Catalog->FindWeapon(TEXT("weapon.fireteam_rifle"));
    const UPolitikaWeaponDefinition* PatrolWeapon = Catalog->FindWeapon(TEXT("weapon.patrol_rifle"));
    TestNotNull(TEXT("Recon weapon resolves"), ReconWeapon);
    TestNotNull(TEXT("Fireteam weapon resolves"), FireteamWeapon);
    TestNotNull(TEXT("Patrol weapon resolves"), PatrolWeapon);
    if (ReconWeapon && FireteamWeapon && PatrolWeapon)
    {
        TestEqual(TEXT("Recon range preserves prototype value"), ReconWeapon->RangeCm, 650.0f);
        TestEqual(TEXT("Recon damage preserves prototype value"), ReconWeapon->Damage, 12.0f);
        TestEqual(TEXT("Recon cooldown preserves prototype value"), ReconWeapon->CooldownSeconds, 0.8f);
        TestEqual(TEXT("Fireteam range preserves prototype value"), FireteamWeapon->RangeCm, 520.0f);
        TestEqual(TEXT("Fireteam damage preserves prototype value"), FireteamWeapon->Damage, 18.0f);
        TestEqual(TEXT("Fireteam cooldown preserves prototype value"), FireteamWeapon->CooldownSeconds, 1.1f);
        TestEqual(TEXT("Patrol range preserves prototype value"), PatrolWeapon->RangeCm, 500.0f);
        TestEqual(TEXT("Patrol damage preserves prototype value"), PatrolWeapon->Damage, 10.0f);
        TestEqual(TEXT("Patrol cooldown preserves prototype value"), PatrolWeapon->CooldownSeconds, 1.3f);
    }
    TestEqual(TEXT("Objective capture radius preserves prototype value"), Map->Objective.CaptureRadiusCm, 300.0f);
    TestEqual(TEXT("Objective capture duration preserves prototype value"), Map->Objective.CaptureDurationSeconds, 20.0f);
    TestEqual(TEXT("Map retains the original two player slots"), Map->PlayerSlots.Num(), 2);
    TestEqual(TEXT("Map retains the original three scenario unit placements"), Map->UnitPlacements.Num(), 3);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPolitikaPrototypeMapRejectionTest,
    "Politika.Content.InvalidMapVersionBoundsAndMarkerAreRejected",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPolitikaPrototypeMapRejectionTest::RunTest(const FString& Parameters)
{
    UPolitikaPrototypeContentCatalog* Catalog = LoadObject<UPolitikaPrototypeContentCatalog>(nullptr,
        TEXT("/Game/Politika/Definitions/DA_PrototypeContentCatalog.DA_PrototypeContentCatalog"));
    if (!Catalog) return false;
    const UPolitikaPrototypeMapDefinition* AuthoredMap = Catalog->FindMap(UPolitikaPrototypeContentValidator::PrototypeMapAssetId);
    if (!AuthoredMap) return false;
    UPolitikaPrototypeMapDefinition* InvalidMap = DuplicateObject<UPolitikaPrototypeMapDefinition>(AuthoredMap, GetTransientPackage());
    InvalidMap->SchemaVersion = UPolitikaPrototypeMapDefinition::CurrentSchemaVersion + 1;
    InvalidMap->PlayableMax = InvalidMap->PlayableMin;
    InvalidMap->Objective.Location = FVector(5000.0, 0.0, 0.0);
    TArray<FPolitikaContentDiagnostic> Diagnostics;
    UPolitikaPrototypeContentValidator::ValidateMapStructure(InvalidMap, Catalog, Diagnostics);
    TestTrue(TEXT("Unsupported map schema is rejected"), Diagnostics.ContainsByPredicate([](const FPolitikaContentDiagnostic& D) { return D.Code == TEXT("CONTENT.SCHEMA.UNSUPPORTED"); }));
    TestTrue(TEXT("Degenerate map bounds are rejected"), Diagnostics.ContainsByPredicate([](const FPolitikaContentDiagnostic& D) { return D.Code == TEXT("MAP.BOUNDS.INVALID"); }));
    TestTrue(TEXT("Starts outside invalid playable bounds are rejected"), Diagnostics.ContainsByPredicate([](const FPolitikaContentDiagnostic& D) { return D.Code == TEXT("MAP.START.OUT_OF_BOUNDS"); }));
    TestTrue(TEXT("Out-of-bounds objective marker is rejected"), Diagnostics.ContainsByPredicate([](const FPolitikaContentDiagnostic& D) { return D.Code == TEXT("MAP.OBJECTIVE.OUT_OF_BOUNDS"); }));

    UPolitikaPrototypeMapDefinition* DuplicateSlotMap = DuplicateObject<UPolitikaPrototypeMapDefinition>(AuthoredMap, GetTransientPackage());
    DuplicateSlotMap->PlayerSlots[1].SlotId = DuplicateSlotMap->PlayerSlots[0].SlotId;
    DuplicateSlotMap->PlayerSlots[1].RuntimePlayerId = DuplicateSlotMap->PlayerSlots[0].RuntimePlayerId;
    DuplicateSlotMap->PlayerSlots[1].FactionId = DuplicateSlotMap->PlayerSlots[0].FactionId;
    Diagnostics.Reset();
    UPolitikaPrototypeContentValidator::ValidateMapStructure(DuplicateSlotMap, Catalog, Diagnostics);
    TestTrue(TEXT("Duplicate player slots are rejected"), Diagnostics.ContainsByPredicate([](const FPolitikaContentDiagnostic& D) { return D.Code == TEXT("MAP.SLOT.DUPLICATE"); }));
    TestTrue(TEXT("Duplicate runtime players are rejected"), Diagnostics.ContainsByPredicate([](const FPolitikaContentDiagnostic& D) { return D.Code == TEXT("MAP.PLAYER.DUPLICATE"); }));
    TestTrue(TEXT("Duplicate faction-to-slot assignments are rejected"), Diagnostics.ContainsByPredicate([](const FPolitikaContentDiagnostic& D) { return D.Code == TEXT("MAP.FACTION.DUPLICATE"); }));

    UPolitikaPrototypeMapDefinition* MissingSlotsMap = DuplicateObject<UPolitikaPrototypeMapDefinition>(AuthoredMap, GetTransientPackage());
    MissingSlotsMap->PlayerSlots.Reset();
    Diagnostics.Reset();
    UPolitikaPrototypeContentValidator::ValidateMapStructure(MissingSlotsMap, Catalog, Diagnostics);
    TestTrue(TEXT("Missing required player starts are rejected"), Diagnostics.ContainsByPredicate([](const FPolitikaContentDiagnostic& D) { return D.Code == TEXT("MAP.SLOTS.INSUFFICIENT"); }));

    UPolitikaPrototypeMapDefinition* MutableAuthoredMap = const_cast<UPolitikaPrototypeMapDefinition*>(AuthoredMap);
    const int32 OriginalSchemaVersion = MutableAuthoredMap->SchemaVersion;
    MutableAuthoredMap->SchemaVersion = UPolitikaPrototypeMapDefinition::CurrentSchemaVersion + 1;
    FPolitikaPrototypeContentSnapshot Snapshot;
    TArray<FPolitikaContentDiagnostic> StartupDiagnostics;
    const bool bInvalidSnapshotLoaded = FPolitikaPrototypeContentLoader::LoadAndValidate(
        nullptr, UPolitikaPrototypeContentValidator::PrototypeMapAssetId, Snapshot, StartupDiagnostics);
    MutableAuthoredMap->SchemaVersion = OriginalSchemaVersion;
    TestFalse(TEXT("Invalid authored content cannot produce a validated runtime snapshot"), bInvalidSnapshotLoaded);
    TestNull(TEXT("Failed content load leaves no partially initialized catalog snapshot"), Snapshot.Catalog.Get());
    TestNull(TEXT("Failed content load leaves no partially initialized map snapshot"), Snapshot.MapDefinition.Get());
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPolitikaPrototypeContentRejectionTest,
    "Politika.Content.InvalidReferencesAndDuplicateIdsAreRejected",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPolitikaPrototypeContentRejectionTest::RunTest(const FString& Parameters)
{
    UPolitikaWeaponDefinition* First = NewObject<UPolitikaWeaponDefinition>();
    First->StableId = TEXT("weapon.duplicate"); First->DisplayName = FText::FromString(TEXT("First"));
    First->RangeCm = 500; First->Damage = 10; First->CooldownSeconds = 1;
    UPolitikaWeaponDefinition* Second = DuplicateObject<UPolitikaWeaponDefinition>(First, GetTransientPackage());
    UPolitikaUnitArchetypeDefinition* Unit = NewObject<UPolitikaUnitArchetypeDefinition>();
    Unit->StableId = TEXT("unit.bad_reference"); Unit->FactionId = TEXT("faction.test");
    Unit->DisplayName = FText::FromString(TEXT("Broken")); Unit->MaxHealth = 10; Unit->MovementSpeedCmPerSecond = 100;
    Unit->WeaponId = TEXT("weapon.missing"); Unit->VisualMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
    UPolitikaPrototypeContentCatalog* Catalog = NewObject<UPolitikaPrototypeContentCatalog>();
    Catalog->StableId = UPolitikaPrototypeContentCatalog::CatalogAssetId; Catalog->Weapons = {First, Second}; Catalog->Units.Add(Unit);
    TArray<FPolitikaContentDiagnostic> Diagnostics;
    UPolitikaPrototypeContentValidator::ValidateCatalog(Catalog, Diagnostics);
    TestTrue(TEXT("Duplicate IDs and unresolved references fail validation"), UPolitikaPrototypeContentValidator::HasErrors(Diagnostics));
    TestTrue(TEXT("Duplicate ID has stable diagnostic code"), Diagnostics.ContainsByPredicate([](const FPolitikaContentDiagnostic& D) { return D.Code == TEXT("CONTENT.ID.DUPLICATE"); }));
    TestTrue(TEXT("Missing weapon has stable diagnostic code"), Diagnostics.ContainsByPredicate([](const FPolitikaContentDiagnostic& D) { return D.Code == TEXT("CONTENT.REFERENCE.UNRESOLVED"); }));

    Diagnostics.Reset();
    UPolitikaPrototypeContentValidator::ValidateMapStructure(nullptr, Catalog, Diagnostics);
    TestTrue(TEXT("Missing map definition is a structural error"), UPolitikaPrototypeContentValidator::HasErrors(Diagnostics));
    TestTrue(TEXT("Missing map definition has a stable diagnostic"), Diagnostics.ContainsByPredicate([](const FPolitikaContentDiagnostic& D) { return D.Code == TEXT("MAP.DEFINITION.MISSING"); }));

    Diagnostics.Reset();
    TestFalse(TEXT("Missing level world cannot pass spatial validation"), UPolitikaPrototypeContentValidator::ValidateMapSpatial(nullptr, nullptr, Diagnostics));
    TestTrue(TEXT("Missing level world has a stable diagnostic"), Diagnostics.ContainsByPredicate([](const FPolitikaContentDiagnostic& D) { return D.Code == TEXT("MAP.LEVEL.WORLD_MISSING"); }));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPolitikaPrototypeStructuralNegativeCoverageTest,
    "Politika.Content.StructuralNegativeDiagnosticsAreSpecific",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPolitikaPrototypeStructuralNegativeCoverageTest::RunTest(const FString& Parameters)
{
    UPolitikaWeaponDefinition* Weapon = nullptr;
    UPolitikaUnitArchetypeDefinition* Unit = nullptr;
    UPolitikaPrototypeContentCatalog* Catalog = MakeValidTestCatalog(Weapon, Unit);
    TArray<FPolitikaContentDiagnostic> Diagnostics;

    Unit->StableId = TEXT("unit.Bad-ID");
    UPolitikaPrototypeContentValidator::ValidateCatalog(Catalog, Diagnostics);
    TestTrue(TEXT("Malformed stable ID reports CONTENT.ID.INVALID on the unit StableId"),
        HasExactDiagnostic(Diagnostics, TEXT("CONTENT.ID.INVALID"), TEXT("unit.Bad-ID"), TEXT("StableId")));
    TestEqual(TEXT("Malformed-ID case emits only its intended diagnostic"), Diagnostics.Num(), 1);

    Unit->StableId = TEXT("unit.test_infantry");
    Unit->MaxHealth = 0;
    Diagnostics.Reset();
    UPolitikaPrototypeContentValidator::ValidateCatalog(Catalog, Diagnostics);
    TestTrue(TEXT("Invalid required health reports CONTENT.VALUE.RANGE on MaxHealth"),
        HasExactDiagnostic(Diagnostics, TEXT("CONTENT.VALUE.RANGE"), Unit->StableId, TEXT("MaxHealth")));
    TestEqual(TEXT("Invalid-health case emits only its intended diagnostic"), Diagnostics.Num(), 1);

    Unit->MaxHealth = 100;
    Unit->SchemaVersion = UPolitikaUnitArchetypeDefinition::CurrentSchemaVersion + 1;
    Diagnostics.Reset();
    UPolitikaPrototypeContentValidator::ValidateCatalog(Catalog, Diagnostics);
    TestTrue(TEXT("Unsupported unit schema reports CONTENT.SCHEMA.UNSUPPORTED on SchemaVersion"),
        HasExactDiagnostic(Diagnostics, TEXT("CONTENT.SCHEMA.UNSUPPORTED"), Unit->StableId, TEXT("SchemaVersion")));
    TestEqual(TEXT("Unsupported-unit-schema case emits only its intended diagnostic"), Diagnostics.Num(), 1);

    Unit->SchemaVersion = UPolitikaUnitArchetypeDefinition::CurrentSchemaVersion;
    Weapon->SchemaVersion = UPolitikaWeaponDefinition::CurrentSchemaVersion + 1;
    Diagnostics.Reset();
    UPolitikaPrototypeContentValidator::ValidateCatalog(Catalog, Diagnostics);
    TestTrue(TEXT("Unsupported weapon schema reports CONTENT.SCHEMA.UNSUPPORTED on SchemaVersion"),
        HasExactDiagnostic(Diagnostics, TEXT("CONTENT.SCHEMA.UNSUPPORTED"), Weapon->StableId, TEXT("SchemaVersion")));
    TestEqual(TEXT("Unsupported-weapon-schema case emits only its intended diagnostic"), Diagnostics.Num(), 1);

    Weapon->SchemaVersion = UPolitikaWeaponDefinition::CurrentSchemaVersion;
    const UPolitikaPrototypeContentCatalog* AuthoredCatalog = LoadObject<UPolitikaPrototypeContentCatalog>(nullptr,
        TEXT("/Game/Politika/Definitions/DA_PrototypeContentCatalog.DA_PrototypeContentCatalog"));
    const UPolitikaPrototypeMapDefinition* AuthoredMap = AuthoredCatalog
        ? AuthoredCatalog->FindMap(UPolitikaPrototypeContentValidator::PrototypeMapAssetId) : nullptr;
    TestNotNull(TEXT("Canonical map is available for focused capability diagnostics"), AuthoredMap);
    if (!AuthoredMap) return false;
    UPolitikaPrototypeMapDefinition* MismatchedMap = DuplicateObject<UPolitikaPrototypeMapDefinition>(AuthoredMap, GetTransientPackage());
    MismatchedMap->UnitPlacements[0].UnitArchetypeId = TEXT("unit.opposing_patrol");
    Diagnostics.Reset();
    UPolitikaPrototypeContentValidator::ValidateMapStructure(MismatchedMap, AuthoredCatalog, Diagnostics);
    TestTrue(TEXT("Slot/archetype faction mismatch reports CONTENT.FACTION.MISMATCH at the placement reference"),
        HasExactDiagnostic(Diagnostics, TEXT("CONTENT.FACTION.MISMATCH"), MismatchedMap->StableId, TEXT("UnitPlacements[0].UnitArchetypeId")));
    TestEqual(TEXT("Faction-mismatch case emits only its intended diagnostic"), Diagnostics.Num(), 1);

    UPolitikaPrototypeMapDefinition* MissingMarkerMap = DuplicateObject<UPolitikaPrototypeMapDefinition>(AuthoredMap, GetTransientPackage());
    MissingMarkerMap->Objective.MarkerMesh = nullptr;
    Diagnostics.Reset();
    UPolitikaPrototypeContentValidator::ValidateMapStructure(MissingMarkerMap, AuthoredCatalog, Diagnostics);
    TestTrue(TEXT("Missing required objective marker reports CONTENT.REFERENCE.MISSING_MESH at Objective.MarkerMesh"),
        HasExactDiagnostic(Diagnostics, TEXT("CONTENT.REFERENCE.MISSING_MESH"), MissingMarkerMap->StableId, TEXT("Objective.MarkerMesh")));
    TestEqual(TEXT("Missing-marker case emits only its intended diagnostic"), Diagnostics.Num(), 1);

    UPolitikaPrototypeMapDefinition* OutOfBoundsObjectiveMap = DuplicateObject<UPolitikaPrototypeMapDefinition>(AuthoredMap, GetTransientPackage());
    OutOfBoundsObjectiveMap->Objective.Location = FVector(5000.0, 0.0, 0.0);
    Diagnostics.Reset();
    UPolitikaPrototypeContentValidator::ValidateMapStructure(OutOfBoundsObjectiveMap, AuthoredCatalog, Diagnostics);
    TestTrue(TEXT("Out-of-bounds required objective marker reports MAP.OBJECTIVE.OUT_OF_BOUNDS at Objective.Location"),
        HasExactDiagnostic(Diagnostics, TEXT("MAP.OBJECTIVE.OUT_OF_BOUNDS"), OutOfBoundsObjectiveMap->StableId, TEXT("Objective.Location")));
    TestEqual(TEXT("Out-of-bounds objective case emits only its intended diagnostic"), Diagnostics.Num(), 1);

    UPolitikaPrototypeMapDefinition* InvalidObjectiveScaleMap = DuplicateObject<UPolitikaPrototypeMapDefinition>(AuthoredMap, GetTransientPackage());
    InvalidObjectiveScaleMap->Objective.MarkerScale.X = 0.0;
    Diagnostics.Reset();
    UPolitikaPrototypeContentValidator::ValidateMapStructure(InvalidObjectiveScaleMap, AuthoredCatalog, Diagnostics);
    TestTrue(TEXT("Invalid required objective marker scale reports CONTENT.VALUE.RANGE at Objective.MarkerScale"),
        HasExactDiagnostic(Diagnostics, TEXT("CONTENT.VALUE.RANGE"), InvalidObjectiveScaleMap->StableId, TEXT("Objective.MarkerScale")));
    TestEqual(TEXT("Invalid objective-scale case emits only its intended diagnostic"), Diagnostics.Num(), 1);

    UPolitikaPrototypeMapDefinition* InvalidObjectiveIdMap = DuplicateObject<UPolitikaPrototypeMapDefinition>(AuthoredMap, GetTransientPackage());
    InvalidObjectiveIdMap->Objective.StableId = TEXT("objective.Bad-ID");
    Diagnostics.Reset();
    UPolitikaPrototypeContentValidator::ValidateMapStructure(InvalidObjectiveIdMap, AuthoredCatalog, Diagnostics);
    TestTrue(TEXT("Malformed required objective marker ID reports CONTENT.ID.INVALID at Objective.StableId"),
        HasExactDiagnostic(Diagnostics, TEXT("CONTENT.ID.INVALID"), InvalidObjectiveIdMap->StableId, TEXT("Objective.StableId")));
    TestEqual(TEXT("Invalid objective-ID case emits only its intended diagnostic"), Diagnostics.Num(), 1);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPolitikaPrototypeAuthoredAuthorityTest,
    "Politika.Content.AuthoredValueChangesAuthoritativeCombatThroughLoader",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPolitikaPrototypeAuthoredAuthorityTest::RunTest(const FString& Parameters)
{
    FPolitikaPrototypeContentSnapshot Baseline;
    TArray<FPolitikaContentDiagnostic> Diagnostics;
    if (!FPolitikaPrototypeContentLoader::LoadAndValidate(nullptr,
        UPolitikaPrototypeContentValidator::PrototypeMapAssetId, Baseline, Diagnostics))
    {
        AddError(TEXT("Canonical runtime content loader failed before controlled authored-value mutation"));
        return false;
    }
    UPolitikaUnitArchetypeDefinition* AuthoredUnit = const_cast<UPolitikaUnitArchetypeDefinition*>(
        Baseline.Catalog->FindUnit(TEXT("unit.recon_vehicle")));
    UPolitikaWeaponDefinition* AuthoredWeapon = const_cast<UPolitikaWeaponDefinition*>(
        Baseline.Catalog->FindWeapon(TEXT("weapon.recon_cannon")));
    if (!AuthoredUnit || !AuthoredWeapon) return false;

    const float OriginalDamage = AuthoredWeapon->Damage;
    const int32 WeaponIndex = Baseline.Catalog->Weapons.IndexOfByPredicate(
        [](const TObjectPtr<UPolitikaWeaponDefinition>& Candidate)
        {
            return Candidate && Candidate->StableId == TEXT("weapon.recon_cannon");
        });
    TestTrue(TEXT("Authored weapon has a catalog slot for an isolated test copy"), WeaponIndex != INDEX_NONE);
    if (WeaponIndex == INDEX_NONE) return false;
    UPolitikaWeaponDefinition* TestWeaponCopy = DuplicateObject<UPolitikaWeaponDefinition>(AuthoredWeapon, GetTransientPackage());
    TestNotNull(TEXT("Transient weapon test copy created"), TestWeaponCopy);
    if (!TestWeaponCopy) return false;
    TestWeaponCopy->Damage = OriginalDamage + 37.0f;
    Baseline.Catalog->Weapons[WeaponIndex] = TestWeaponCopy;
    FPolitikaPrototypeContentSnapshot Mutated;
    Diagnostics.Reset();
    const bool bLoaded = FPolitikaPrototypeContentLoader::LoadAndValidate(nullptr,
        UPolitikaPrototypeContentValidator::PrototypeMapAssetId, Mutated, Diagnostics);
    TestTrue(TEXT("Runtime content loader validates and resolves the isolated in-memory authored copy"), bLoaded);
    TestTrue(TEXT("Loader returns the edited weapon definition through its stable ID"),
        bLoaded && Mutated.Catalog->FindWeapon(TEXT("weapon.recon_cannon"))->Damage == OriginalDamage + 37.0f);
    if (bLoaded)
    {
        const UWorld::InitializationValues InitValues = UWorld::InitializationValues()
            .AllowAudioPlayback(false).RequiresHitProxies(false).CreatePhysicsScene(false)
            .CreateNavigation(false).CreateAISystem(false).ShouldSimulatePhysics(false)
            .SetTransactional(false).CreateFXSystem(false);
        UWorld* World = UWorld::CreateWorld(EWorldType::Game, false, TEXT("PolitikaAuthoredAuthorityWorld"),
            GetTransientPackage(), false, ERHIFeatureLevel::Num, &InitValues);
        TestNotNull(TEXT("Transient authoritative simulation world created"), World);
        if (World)
        {
            const UPolitikaWeaponDefinition* ResolvedWeapon = Mutated.Catalog->FindWeapon(AuthoredUnit->WeaponId);
            APolitikaPrototypeUnit* Attacker = World->SpawnActor<APolitikaPrototypeUnit>(FVector::ZeroVector, FRotator::ZeroRotator);
            APolitikaPrototypeUnit* Target = World->SpawnActor<APolitikaPrototypeUnit>(FVector(100.0f, 0.0f, 0.0f), FRotator::ZeroRotator);
            TestNotNull(TEXT("Authoritative simulation attacker spawned"), Attacker);
            TestNotNull(TEXT("Authoritative simulation target spawned"), Target);
            if (ResolvedWeapon && Attacker && Target)
            {
                Attacker->Configure(1, 1, AuthoredUnit->DisplayName, AuthoredUnit->MaxHealth,
                    AuthoredUnit->MovementSpeedCmPerSecond, ResolvedWeapon->RangeCm, ResolvedWeapon->Damage,
                    ResolvedWeapon->CooldownSeconds, AuthoredUnit->bAttackable, AuthoredUnit->VisualMesh,
                    AuthoredUnit->VisualScale, AuthoredUnit->FactionTint);
                Target->Configure(2, 2, FText::FromString(TEXT("Target")), 500, 0, 0, 0, 1,
                    true, AuthoredUnit->VisualMesh, FVector::OneVector, FLinearColor::Red);
                Attacker->SetAttackOrder(Target);
                Attacker->ResolveCombat(0.01f);
                const int32 AppliedDamage = Target->ApplyPendingDamage();
                TestEqual(TEXT("Mutated authored damage is applied by authoritative combat"), AppliedDamage,
                    FMath::RoundToInt(OriginalDamage + 37.0f));
            }
            World->DestroyWorld(false);
        }
    }
    Baseline.Catalog->Weapons[WeaponIndex] = AuthoredWeapon;
    TestEqual(TEXT("Canonical authored definition was never mutated"), AuthoredWeapon->Damage, OriginalDamage);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPolitikaPrototypeDisplayLabelIdentityTest,
    "Politika.Content.DisplayLabelDoesNotChangeStableIdentityOrResolution",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPolitikaPrototypeDisplayLabelIdentityTest::RunTest(const FString& Parameters)
{
    FPolitikaPrototypeContentSnapshot Snapshot;
    TArray<FPolitikaContentDiagnostic> Diagnostics;
    if (!FPolitikaPrototypeContentLoader::LoadAndValidate(nullptr,
        UPolitikaPrototypeContentValidator::PrototypeMapAssetId, Snapshot, Diagnostics)) return false;
    UPolitikaUnitArchetypeDefinition* Unit = const_cast<UPolitikaUnitArchetypeDefinition*>(Snapshot.Catalog->FindUnit(TEXT("unit.recon_vehicle")));
    if (!Unit) return false;
    const FPrimaryAssetId OriginalAssetId = Unit->GetPrimaryAssetId();
    const FName StableId = Unit->StableId;
    const int32 UnitIndex = Snapshot.Catalog->Units.IndexOfByPredicate(
        [](const TObjectPtr<UPolitikaUnitArchetypeDefinition>& Candidate)
        {
            return Candidate && Candidate->StableId == TEXT("unit.recon_vehicle");
        });
    TestTrue(TEXT("Authored unit has a catalog slot for an isolated label test copy"), UnitIndex != INDEX_NONE);
    if (UnitIndex == INDEX_NONE) return false;
    UPolitikaUnitArchetypeDefinition* TestUnitCopy = DuplicateObject<UPolitikaUnitArchetypeDefinition>(Unit, GetTransientPackage());
    TestNotNull(TEXT("Transient unit test copy created"), TestUnitCopy);
    if (!TestUnitCopy) return false;
    TestUnitCopy->DisplayName = FText::FromString(TEXT("Controlled Label Change"));
    Snapshot.Catalog->Units[UnitIndex] = TestUnitCopy;
    const UPolitikaUnitArchetypeDefinition* ResolvedCopy = Snapshot.Catalog->FindUnit(StableId);
    TestTrue(TEXT("Display-label edit leaves stable ID unchanged"), TestUnitCopy->StableId == StableId);
    TestTrue(TEXT("Display-label edit leaves primary asset identity unchanged"), TestUnitCopy->GetPrimaryAssetId() == OriginalAssetId);
    TestTrue(TEXT("Catalog stable-ID resolution resolves the controlled label copy"), ResolvedCopy == TestUnitCopy);
    Snapshot.Catalog->Units[UnitIndex] = Unit;
    TestTrue(TEXT("Canonical display label and definition are restored"), Snapshot.Catalog->FindUnit(StableId) == Unit);
    return true;
}

#endif
