#if WITH_DEV_AUTOMATION_TESTS

#include "PolitikaPrototypeContent.h"
#include "Engine/StaticMesh.h"
#include "Misc/AutomationTest.h"

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

#endif
