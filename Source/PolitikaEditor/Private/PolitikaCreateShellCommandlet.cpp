#include "PolitikaCreateShellCommandlet.h"

#include "Editor.h"
#include "Engine/World.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "EngineUtils.h"
#include "Components/BrushComponent.h"
#include "Engine/Brush.h"
#include "Model.h"
#include "Engine/Polys.h"
#include "BSPOps.h"
#include "Engine/AssetManager.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "NavMesh/NavMeshBoundsVolume.h"
#include "Builders/CubeBuilder.h"
#include "UObject/SavePackage.h"
#include "UObject/Package.h"
#include "HAL/PlatformProcess.h"
#include "PolitikaPrototypeContent.h"
#include "FileHelpers.h"
#include "NavigationSystem.h"
#include "GameFramework/PlayerStart.h"
#include "GameFramework/WorldSettings.h"
#include "HAL/FileManager.h"
#include "Misc/PackageName.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "PolitikaBootstrap.h"
#include "PolitikaPrototypeGameplay.h"
#include "PolitikaShellGameMode.h"

namespace
{
template <typename T>
T* CreateDefinitionAsset(const FString& PackageName, const FString& AssetName)
{
    const FString Filename = FPackageName::LongPackageNameToFilename(PackageName, FPackageName::GetAssetPackageExtension());
    if (IFileManager::Get().FileExists(*Filename))
        return Cast<T>(StaticLoadObject(T::StaticClass(), nullptr, *(PackageName + TEXT(".") + AssetName)));
    IFileManager::Get().MakeDirectory(*FPaths::GetPath(Filename), true);
    UPackage* Package = CreatePackage(*PackageName);
    T* Asset = NewObject<T>(Package, *AssetName, RF_Public | RF_Standalone);
    if (!Asset) return nullptr;
    FAssetRegistryModule::AssetCreated(Asset);
    Package->MarkPackageDirty();
    FSavePackageArgs Args;
    Args.TopLevelFlags = RF_Public | RF_Standalone;
    Args.SaveFlags = SAVE_NoError;
    return UPackage::SavePackage(Package, Asset, *Filename, Args) ? Asset : nullptr;
}

bool SaveDefinitionAsset(UObject* Asset)
{
    if (!Asset) return false;
    UPackage* Package = Asset->GetOutermost();
    const FString Filename = FPackageName::LongPackageNameToFilename(Package->GetName(), FPackageName::GetAssetPackageExtension());
    FSavePackageArgs Args;
    Args.TopLevelFlags = RF_Public | RF_Standalone;
    Args.SaveFlags = SAVE_NoError;
    return UPackage::SavePackage(Package, Asset, *Filename, Args);
}

bool CreateInvalidMapSchemaFixture()
{
    const UPolitikaPrototypeMapDefinition* ValidMap = LoadObject<UPolitikaPrototypeMapDefinition>(nullptr,
        TEXT("/Game/Politika/Definitions/Maps/DA_RTSVerticalSlice.DA_RTSVerticalSlice"));
    if (!ValidMap) return false;
    UPolitikaPrototypeMapDefinition* Fixture = CreateDefinitionAsset<UPolitikaPrototypeMapDefinition>(
        TEXT("/Game/Politika/TestFixtures/DA_InvalidMapSchema"), TEXT("DA_InvalidMapSchema"));
    if (!Fixture) return false;
    Fixture->StableId = TEXT("map.test.invalid_schema");
    Fixture->SchemaVersion = UPolitikaPrototypeMapDefinition::CurrentSchemaVersion + 1;
    Fixture->Profile = ValidMap->Profile;
    Fixture->LevelAsset = ValidMap->LevelAsset;
    Fixture->CoordinateSpace = ValidMap->CoordinateSpace;
    Fixture->PlayableMin = ValidMap->PlayableMin;
    Fixture->PlayableMax = ValidMap->PlayableMax;
    Fixture->MovementProfileId = ValidMap->MovementProfileId;
    Fixture->GroundActorTag = ValidMap->GroundActorTag;
    Fixture->PlayerSlots = ValidMap->PlayerSlots;
    Fixture->UnitPlacements = ValidMap->UnitPlacements;
    Fixture->Objective = ValidMap->Objective;
    Fixture->CameraOffset = ValidMap->CameraOffset;
    Fixture->CameraFieldOfView = ValidMap->CameraFieldOfView;
    Fixture->CameraPanMin = ValidMap->CameraPanMin;
    Fixture->CameraPanMax = ValidMap->CameraPanMax;
    Fixture->Lighting = ValidMap->Lighting;
    Fixture->MarkPackageDirty();
    return SaveDefinitionAsset(Fixture);
}

bool CreateInvalidParticipantFactionFixture()
{
    const UPolitikaPrototypeMapDefinition* ValidMap = LoadObject<UPolitikaPrototypeMapDefinition>(nullptr,
        TEXT("/Game/Politika/Definitions/Maps/DA_RTSVerticalSlice.DA_RTSVerticalSlice"));
    if (!ValidMap || ValidMap->UnitPlacements.IsEmpty()) return false;
    UPolitikaPrototypeMapDefinition* Fixture = CreateDefinitionAsset<UPolitikaPrototypeMapDefinition>(
        TEXT("/Game/Politika/TestFixtures/DA_InvalidParticipantFaction"), TEXT("DA_InvalidParticipantFaction"));
    if (!Fixture) return false;
    Fixture->StableId = TEXT("map.test.invalid_participant_faction");
    Fixture->SchemaVersion = ValidMap->SchemaVersion;
    Fixture->Profile = ValidMap->Profile;
    Fixture->LevelAsset = ValidMap->LevelAsset;
    Fixture->CoordinateSpace = ValidMap->CoordinateSpace;
    Fixture->PlayableMin = ValidMap->PlayableMin;
    Fixture->PlayableMax = ValidMap->PlayableMax;
    Fixture->MovementProfileId = ValidMap->MovementProfileId;
    Fixture->GroundActorTag = ValidMap->GroundActorTag;
    Fixture->PlayerSlots = ValidMap->PlayerSlots;
    Fixture->UnitPlacements = ValidMap->UnitPlacements;
    Fixture->Objective = ValidMap->Objective;
    Fixture->CameraOffset = ValidMap->CameraOffset;
    Fixture->CameraFieldOfView = ValidMap->CameraFieldOfView;
    Fixture->CameraPanMin = ValidMap->CameraPanMin;
    Fixture->CameraPanMax = ValidMap->CameraPanMax;
    Fixture->Lighting = ValidMap->Lighting;
    // Keep all canonical references and placements except this single field:
    // the opposition archetype is assigned to the player's faction slot.
    Fixture->UnitPlacements[0].UnitArchetypeId = TEXT("unit.opposing_patrol");
    Fixture->MarkPackageDirty();
    UE_LOG(LogPolitikaStartup, Display,
        TEXT("TEST_FIXTURE_CREATED map=%s source=%s fault=UnitPlacements[0].UnitArchetypeId value=%s"),
        *Fixture->StableId.ToString(), *ValidMap->GetPathName(), *Fixture->UnitPlacements[0].UnitArchetypeId.ToString());
    return SaveDefinitionAsset(Fixture);
}

bool CreatePrototypeDefinitions()
{
    UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
    UStaticMesh* Sphere = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere.Sphere"));
    UStaticMesh* Cylinder = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
    if (!Cube || !Sphere || !Cylinder) return false;

    auto* ReconWeapon = CreateDefinitionAsset<UPolitikaWeaponDefinition>(TEXT("/Game/Politika/Definitions/Weapons/W_Recon"), TEXT("W_Recon"));
    auto* FireteamWeapon = CreateDefinitionAsset<UPolitikaWeaponDefinition>(TEXT("/Game/Politika/Definitions/Weapons/W_Fireteam"), TEXT("W_Fireteam"));
    auto* PatrolWeapon = CreateDefinitionAsset<UPolitikaWeaponDefinition>(TEXT("/Game/Politika/Definitions/Weapons/W_Patrol"), TEXT("W_Patrol"));
    auto* Recon = CreateDefinitionAsset<UPolitikaUnitArchetypeDefinition>(TEXT("/Game/Politika/Definitions/Units/DA_ReconVehicle"), TEXT("DA_ReconVehicle"));
    auto* Fireteam = CreateDefinitionAsset<UPolitikaUnitArchetypeDefinition>(TEXT("/Game/Politika/Definitions/Units/DA_MechanizedFireteam"), TEXT("DA_MechanizedFireteam"));
    auto* Patrol = CreateDefinitionAsset<UPolitikaUnitArchetypeDefinition>(TEXT("/Game/Politika/Definitions/Units/DA_OpposingPatrol"), TEXT("DA_OpposingPatrol"));
    auto* Map = CreateDefinitionAsset<UPolitikaPrototypeMapDefinition>(TEXT("/Game/Politika/Definitions/Maps/DA_RTSVerticalSlice"), TEXT("DA_RTSVerticalSlice"));
    auto* Catalog = CreateDefinitionAsset<UPolitikaPrototypeContentCatalog>(TEXT("/Game/Politika/Definitions/DA_PrototypeContentCatalog"), TEXT("DA_PrototypeContentCatalog"));
    if (!ReconWeapon || !FireteamWeapon || !PatrolWeapon || !Recon || !Fireteam || !Patrol || !Map || !Catalog) return false;

    auto SetWeapon = [](UPolitikaWeaponDefinition* D, FName Id, const TCHAR* Name, float Range, float Damage, float Cooldown)
    { D->StableId=Id; D->SchemaVersion=1; D->DisplayName=FText::FromString(Name); D->RangeCm=Range; D->Damage=Damage; D->CooldownSeconds=Cooldown; D->MarkPackageDirty(); };
    SetWeapon(ReconWeapon,TEXT("weapon.recon_cannon"),TEXT("Recon Cannon"),650,12,.8f);
    SetWeapon(FireteamWeapon,TEXT("weapon.fireteam_rifle"),TEXT("Fireteam Rifle"),520,18,1.1f);
    SetWeapon(PatrolWeapon,TEXT("weapon.patrol_rifle"),TEXT("Patrol Rifle"),500,10,1.3f);
    auto SetUnit = [](UPolitikaUnitArchetypeDefinition* D, FName Id, const TCHAR* Name, FName Faction, int32 Health, float Speed, FName Weapon, UStaticMesh* Mesh, FVector Scale, FLinearColor Tint)
    { D->StableId=Id; D->SchemaVersion=1; D->FactionId=Faction; D->DisplayName=FText::FromString(Name); D->MaxHealth=Health; D->MovementSpeedCmPerSecond=Speed; D->WeaponId=Weapon; D->VisualMesh=Mesh; D->VisualScale=Scale; D->FactionTint=Tint; D->bAttackable=true; D->MarkPackageDirty(); };
    SetUnit(Recon,TEXT("unit.recon_vehicle"),TEXT("Recon Vehicle"),TEXT("faction.player"),120,340,TEXT("weapon.recon_cannon"),Cube,FVector(1.8,1,.65),FLinearColor(.19,.52,.78));
    SetUnit(Fireteam,TEXT("unit.mechanized_fireteam"),TEXT("Mechanized Fireteam"),TEXT("faction.player"),180,210,TEXT("weapon.fireteam_rifle"),Sphere,FVector(.85,.85,1.35),FLinearColor(.26,.64,.37));
    SetUnit(Patrol,TEXT("unit.opposing_patrol"),TEXT("Opposing Patrol"),TEXT("faction.opposition"),150,180,TEXT("weapon.patrol_rifle"),Cube,FVector(1.2,1.2,.9),FLinearColor(.69,.24,.18));

    Map->StableId=UPolitikaPrototypeContentValidator::PrototypeMapAssetId; Map->SchemaVersion=1; Map->Profile=EPolitikaPrototypeMapProfile::Test;
    Map->LevelAsset=FSoftObjectPath(TEXT("/Game/Maps/RTSVerticalSlice")); Map->CoordinateSpace=TEXT("world_xy_centimeters");
    Map->PlayableMin=FVector2D(-2000,-2000); Map->PlayableMax=FVector2D(2000,2000); Map->MovementProfileId=UPolitikaPrototypeContentValidator::PrototypeMovementProfileId;
    Map->GroundActorTag=TEXT("PolitikaPrototypeGround"); Map->CameraOffset=FVector(0,-2450,2750); Map->CameraFieldOfView=65; Map->CameraPanMin=FVector2D(-2000,-1850); Map->CameraPanMax=FVector2D(2000,2000);
    FPolitikaPrototypePlayerSlot S1; S1.SlotId=1; S1.RuntimePlayerId=1; S1.FactionId=TEXT("faction.player"); S1.bHumanControlled=true; S1.StartLocation=FVector(-700,-600,42);
    FPolitikaPrototypePlayerSlot S2; S2.SlotId=2; S2.RuntimePlayerId=2; S2.FactionId=TEXT("faction.opposition"); S2.StartLocation=FVector(400,400,42); Map->PlayerSlots={S1,S2};
    FPolitikaPrototypeUnitPlacement P1; P1.UnitArchetypeId=Recon->StableId; P1.SlotId=1; P1.Location=FVector(-520,-520,42);
    FPolitikaPrototypeUnitPlacement P2; P2.UnitArchetypeId=Fireteam->StableId; P2.SlotId=1; P2.Location=FVector(-300,-720,42);
    FPolitikaPrototypeUnitPlacement P3; P3.UnitArchetypeId=Patrol->StableId; P3.SlotId=2; P3.Location=FVector(240,300,42); Map->UnitPlacements={P1,P2,P3};
    Map->Objective.StableId=TEXT("objective.central_control"); Map->Objective.Type=EPolitikaPrototypeObjectiveType::ControlPoint; Map->Objective.Location=FVector(0,150,4);
    Map->Objective.CaptureRadiusCm=300; Map->Objective.CaptureDurationSeconds=20; Map->Objective.MarkerMesh=Cylinder; Map->Objective.MarkerScale=FVector(6,6,.35); Map->Objective.MarkerTint=FLinearColor(.76,.66,.22);
    Map->Lighting.SunLocation=FVector(0,0,1600); Map->Lighting.SunRotation=FRotator(-52,-28,0); Map->Lighting.SunIntensity=2; Map->Lighting.SunColor=FLinearColor(1,.91,.75); Map->Lighting.GroundTint=FLinearColor(.20,.25,.19); Map->MarkPackageDirty();

    Catalog->StableId=UPolitikaPrototypeContentCatalog::CatalogAssetId; Catalog->SchemaVersion=1;
    Catalog->Weapons={ReconWeapon,FireteamWeapon,PatrolWeapon}; Catalog->Units={Recon,Fireteam,Patrol}; Catalog->Maps={Map}; Catalog->MarkPackageDirty();
    return SaveDefinitionAsset(ReconWeapon) && SaveDefinitionAsset(FireteamWeapon) && SaveDefinitionAsset(PatrolWeapon)
        && SaveDefinitionAsset(Recon) && SaveDefinitionAsset(Fireteam) && SaveDefinitionAsset(Patrol)
        && SaveDefinitionAsset(Map) && SaveDefinitionAsset(Catalog);
}

bool PreparePrototypeFixture()
{
    const FString Filename = FPackageName::LongPackageNameToFilename(TEXT("/Game/Maps/RTSVerticalSlice"), FPackageName::GetMapPackageExtension());
    UWorld* World = UEditorLoadingAndSavingUtils::LoadMap(Filename);
    if (!World) return false;
    AStaticMeshActor* Ground = nullptr;
    ANavMeshBoundsVolume* NavBounds = nullptr;
    for (TActorIterator<AActor> It(World); It; ++It)
    {
        if (It->ActorHasTag(TEXT("PolitikaPrototypeGround"))) Ground = Cast<AStaticMeshActor>(*It);
        if (It->IsA<ANavMeshBoundsVolume>() && It->ActorHasTag(TEXT("PolitikaPrototypeNavBounds"))) NavBounds = Cast<ANavMeshBoundsVolume>(*It);
    }
    if (!Ground)
    {
        UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
        Ground = World->SpawnActor<AStaticMeshActor>(FVector(0,0,-40), FRotator::ZeroRotator);
        if (!Cube || !Ground) return false;
        Ground->SetActorLabel(TEXT("Prototype Static Ground"));
        Ground->Tags.Add(TEXT("PolitikaPrototypeGround"));
        Ground->GetStaticMeshComponent()->SetStaticMesh(Cube);
    }
    Ground->SetActorLocation(FVector(0,0,-40));
    Ground->SetActorScale3D(FVector(40,40,.5));
    Ground->GetStaticMeshComponent()->SetMobility(EComponentMobility::Static);
    Ground->GetStaticMeshComponent()->SetCollisionProfileName(TEXT("BlockAll"));
    Ground->GetStaticMeshComponent()->SetCanEverAffectNavigation(true);
    Ground->GetStaticMeshComponent()->ReregisterComponent();
    if (!NavBounds)
    {
        NavBounds = World->SpawnActor<ANavMeshBoundsVolume>(FVector::ZeroVector, FRotator::ZeroRotator);
        if (!NavBounds) return false;
        NavBounds->SetActorLabel(TEXT("Prototype Ground Navigation Bounds"));
        NavBounds->Tags.Add(TEXT("PolitikaPrototypeNavBounds"));
    }
    ABrush* BuilderBrush = World->GetDefaultBrush();
    if (!BuilderBrush) return false;
    if (!BuilderBrush->Brush)
    {
        BuilderBrush->Brush = NewObject<UModel>(BuilderBrush, NAME_None, RF_Transactional);
        BuilderBrush->Brush->Initialize(nullptr, true);
        BuilderBrush->Brush->Polys = NewObject<UPolys>(BuilderBrush->Brush, NAME_None, RF_Transactional);
        BuilderBrush->GetBrushComponent()->Brush = BuilderBrush->Brush;
    }
    UCubeBuilder* Builder = NewObject<UCubeBuilder>(GetTransientPackage());
    Builder->X = 4400; Builder->Y = 4400; Builder->Z = 600;
    if (!Builder->Build(World, BuilderBrush)) return false;
    UE_LOG(LogPolitikaStartup, Display, TEXT("Builder geometry: model=%s polygons=%d"), BuilderBrush->Brush ? TEXT("valid") : TEXT("null"),
        BuilderBrush->Brush && BuilderBrush->Brush->Polys ? BuilderBrush->Brush->Polys->Element.Num() : 0);
    NavBounds->PreEditChange(nullptr);
    FBSPOps::csgCopyBrush(NavBounds, BuilderBrush, 0, RF_Transactional, true, true);
    NavBounds->PostEditChange();
    if (NavBounds->Brush) NavBounds->Brush->BuildBound();
    UE_LOG(LogPolitikaStartup, Display, TEXT("Nav volume geometry: model=%s polygons=%d brush_bounds=%s component_bounds=%s"),
        NavBounds->Brush ? TEXT("valid") : TEXT("null"), NavBounds->Brush && NavBounds->Brush->Polys ? NavBounds->Brush->Polys->Element.Num() : 0,
        NavBounds->Brush ? *NavBounds->Brush->Bounds.ToString() : TEXT("none"), *NavBounds->GetBrushComponent()->Bounds.ToString());
    NavBounds->GetBrushComponent()->SetMobility(EComponentMobility::Static);
    NavBounds->GetBrushComponent()->ReregisterComponent();
    UE_LOG(LogPolitikaStartup, Display, TEXT("Prototype nav bounds box: %s"), *NavBounds->GetComponentsBoundingBox().ToString());
    if (UNavigationSystemV1* NavSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(World))
    {
        UNavigationSystemV1::UpdateActorAndComponentsInNavOctree(*Ground);
        NavSystem->OnNavigationBoundsUpdated(NavBounds);
        NavSystem->RemoveNavigationBuildLock(ENavigationBuildLock::AsyncLoadLock, UNavigationSystemV1::ELockRemovalRebuildAction::NoRebuild);
        NavSystem->ReleaseInitialBuildingLock();
        NavSystem->InitializeLevelCollisions();
        NavSystem->Build();
        TArray<FBox> BuildBounds;
        if (const ANavigationData* NavData = NavSystem->GetDefaultNavDataInstance(FNavigationSystem::DontCreate))
            NavSystem->GetNavigationBoundsForNavData(*NavData, BuildBounds, World->PersistentLevel);
        UE_LOG(LogPolitikaStartup, Display, TEXT("Navigation build bounds count=%d first=%s"), BuildBounds.Num(), BuildBounds.IsEmpty() ? TEXT("none") : *BuildBounds[0].ToString());
        int32 WaitIterations = 0;
        while (NavSystem->IsNavigationBuildInProgress() && WaitIterations++ < 600) FPlatformProcess::Sleep(0.1f);
        if (NavSystem->IsNavigationBuildInProgress())
        {
            UE_LOG(LogPolitikaStartup, Error, TEXT("Navigation build did not complete within 60 seconds."));
            return false;
        }
    }
    else
    {
        UE_LOG(LogPolitikaStartup, Error, TEXT("No navigation system is available while preparing the RTS map."));
        return false;
    }
    return FEditorFileUtils::SaveMap(World, Filename);
}
}

UPolitikaCreateShellCommandlet::UPolitikaCreateShellCommandlet()
{
    IsClient = false;
    IsServer = false;
    IsEditor = true;
    LogToConsole = true;
}

int32 UPolitikaCreateShellCommandlet::Main(const FString& Params)
{
    if (!GEditor)
    {
        UE_LOG(LogPolitikaStartup, Error, TEXT("An editor build is required for shell authoring or content validation."));
        return 1;
    }
    // Isolated fixture authoring must not load/save or regenerate canonical maps,
    // navigation, catalog entries, or the existing schema fixture.
    if (FParse::Param(*Params, TEXT("CreateInvalidParticipantFactionFixture")))
        return CreateInvalidParticipantFactionFixture() ? 0 : 12;
    if (FParse::Param(*Params, TEXT("ValidateBlockedRoute")))
    {
        UWorld* World = UEditorLoadingAndSavingUtils::LoadMap(FPackageName::LongPackageNameToFilename(
            TEXT("/Game/Maps/RTSVerticalSlice"), FPackageName::GetMapPackageExtension()));
        UPolitikaPrototypeContentCatalog* Catalog = LoadObject<UPolitikaPrototypeContentCatalog>(nullptr,
            TEXT("/Game/Politika/Definitions/DA_PrototypeContentCatalog.DA_PrototypeContentCatalog"));
        UPolitikaPrototypeMapDefinition* Map = Catalog
            ? const_cast<UPolitikaPrototypeMapDefinition*>(Catalog->FindMap(UPolitikaPrototypeContentValidator::PrototypeMapAssetId)) : nullptr;
        if (!World || !Map)
        {
            UE_LOG(LogPolitikaStartup, Error, TEXT("Blocked-route validation requires the saved fixture and its authored map definition."));
            return 9;
        }
        UNavigationSystemV1* NavSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(World);
        UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
        AStaticMeshActor* Wall = Cube ? World->SpawnActor<AStaticMeshActor>(FVector(0.0, 0.0, 800.0), FRotator::ZeroRotator) : nullptr;
        if (!NavSystem || !Wall)
        {
            UE_LOG(LogPolitikaStartup, Error, TEXT("Blocked-route validation could not create its transient NavMesh obstacle."));
            return 9;
        }
        Wall->GetStaticMeshComponent()->SetStaticMesh(Cube);
        Wall->SetActorScale3D(FVector(2.0, 50.0, 20.0));
        Wall->GetStaticMeshComponent()->SetMobility(EComponentMobility::Static);
        Wall->GetStaticMeshComponent()->SetCollisionProfileName(TEXT("BlockAll"));
        Wall->GetStaticMeshComponent()->SetCanEverAffectNavigation(true);
        Wall->GetStaticMeshComponent()->ReregisterComponent();
        UNavigationSystemV1::UpdateActorAndComponentsInNavOctree(*Wall);
        NavSystem->RemoveNavigationBuildLock(ENavigationBuildLock::AsyncLoadLock, UNavigationSystemV1::ELockRemovalRebuildAction::NoRebuild);
        NavSystem->ReleaseInitialBuildingLock();
        NavSystem->Build();
        int32 WaitIterations = 0;
        while (NavSystem->IsNavigationBuildInProgress() && WaitIterations++ < 600) FPlatformProcess::Sleep(0.1f);
        if (NavSystem->IsNavigationBuildInProgress())
        {
            UE_LOG(LogPolitikaStartup, Error, TEXT("Blocked-route NavMesh build timed out."));
            return 9;
        }
        const FVector OriginalObjective = Map->Objective.Location;
        Map->Objective.Location = FVector(500.0, 0.0, 4.0);
        TArray<FPolitikaContentDiagnostic> Diagnostics;
        UPolitikaPrototypeContentValidator::ValidateMapSpatial(World, Map, Diagnostics);
        Map->Objective.Location = OriginalObjective;
        const bool bUnreachableRouteDetected = Diagnostics.ContainsByPredicate([](const FPolitikaContentDiagnostic& Diagnostic)
        {
            return Diagnostic.Code == TEXT("MAP.PATH.UNREACHABLE")
                && Diagnostic.PropertyPath.Contains(TEXT("StartLocation->Objective"));
        });
        const FString Report = UPolitikaPrototypeContentValidator::DiagnosticsToJson(Diagnostics);
        const FString ReportDir = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("Diagnostics"));
        IFileManager::Get().MakeDirectory(*ReportDir, true);
        if (!FFileHelper::SaveStringToFile(Report, *FPaths::Combine(ReportDir, TEXT("prototype-unreachable-route-validation.json")))) return 7;
        UE_LOG(LogPolitikaStartup, Display, TEXT("BLOCKED_ROUTE_VALIDATION_JSON %s"), *Report);
        return bUnreachableRouteDetected ? 0 : 10;
    }
    if (FParse::Param(*Params, TEXT("ValidateContent")))
    {
        UWorld* World = UEditorLoadingAndSavingUtils::LoadMap(FPackageName::LongPackageNameToFilename(
            TEXT("/Game/Maps/RTSVerticalSlice"), FPackageName::GetMapPackageExtension()));
        UPolitikaPrototypeContentCatalog* Catalog = LoadObject<UPolitikaPrototypeContentCatalog>(nullptr,
            TEXT("/Game/Politika/Definitions/DA_PrototypeContentCatalog.DA_PrototypeContentCatalog"));
        TArray<FPolitikaContentDiagnostic> Diagnostics;
        UPolitikaPrototypeContentValidator::ValidateCatalog(Catalog, Diagnostics);
        const UPolitikaPrototypeMapDefinition* Map = Catalog ? Catalog->FindMap(UPolitikaPrototypeContentValidator::PrototypeMapAssetId) : nullptr;
        UPolitikaPrototypeContentValidator::ValidateMapStructure(Map, Catalog, Diagnostics);
        const bool bSpatialValid = UPolitikaPrototypeContentValidator::ValidateMapSpatial(World, Map, Diagnostics);
        UE_LOG(LogPolitikaStartup, Display, TEXT("Content validation inputs: world=%s catalog=%s map=%s; spatial=%s; diagnostics=%d"),
            World ? *World->GetName() : TEXT("missing"), Catalog ? *Catalog->GetPathName() : TEXT("missing"),
            Map ? *Map->GetPathName() : TEXT("missing"), bSpatialValid ? TEXT("passed") : TEXT("failed"), Diagnostics.Num());
        const FString Report = UPolitikaPrototypeContentValidator::DiagnosticsToJson(Diagnostics);
        const FString ReportDir = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("Diagnostics"));
        IFileManager::Get().MakeDirectory(*ReportDir, true);
        if (!FFileHelper::SaveStringToFile(Report, *FPaths::Combine(ReportDir, TEXT("prototype-content-validation.json")))) return 7;
        UE_LOG(LogPolitikaStartup, Display, TEXT("CONTENT_VALIDATION_JSON %s"), *Report);
        return UPolitikaPrototypeContentValidator::HasErrors(Diagnostics) ? 8 : 0;
    }
    if (FParse::Param(*Params, TEXT("CreateContent")) && !CreatePrototypeDefinitions())
    {
        UE_LOG(LogPolitikaStartup, Error, TEXT("Could not create or load prototype gameplay definition assets."));
        return 5;
    }
    if (!CreateInvalidMapSchemaFixture())
    {
        UE_LOG(LogPolitikaStartup, Error, TEXT("Could not create Development invalid-map-schema fixture."));
        return 11;
    }
    struct FShellMap { const TCHAR* Package; UClass* GameMode; bool bNeedsPlayerStart; };
    const FShellMap Maps[] = {
        {TEXT("/Game/Maps/MainMenu"), APolitikaShellGameMode::StaticClass(), false},
        {TEXT("/Game/Maps/OfflineSandbox"), APolitikaOfflineGameMode::StaticClass(), false},
        {TEXT("/Game/Maps/RTSVerticalSlice"), APolitikaPrototypeGameMode::StaticClass(), true}
    };
    for (const FShellMap& Map : Maps)
    {
        const FString Filename = FPackageName::LongPackageNameToFilename(Map.Package, FPackageName::GetMapPackageExtension());
        if (IFileManager::Get().FileExists(*Filename))
        {
            UE_LOG(LogPolitikaStartup, Display, TEXT("Preserving existing map %s"), Map.Package);
            continue;
        }
        if (!IFileManager::Get().MakeDirectory(*FPaths::GetPath(Filename), true)) { return 2; }
        UWorld* World = GEditor->NewMap(false);
        if (!World) { return 3; }
        World->GetWorldSettings()->DefaultGameMode = Map.GameMode;
        World->GetWorldSettings()->bEnableWorldBoundsChecks = false;
        if (Map.bNeedsPlayerStart && !World->SpawnActor<APlayerStart>(FVector::ZeroVector, FRotator::ZeroRotator))
        {
            UE_LOG(LogPolitikaStartup, Error, TEXT("Could not create the prototype PlayerStart in %s"), Map.Package);
            return 3;
        }
        if (!FEditorFileUtils::SaveMap(World, Filename)) { return 4; }
        UE_LOG(LogPolitikaStartup, Display, TEXT("Created shell map %s"), Map.Package);
    }
    if (!PreparePrototypeFixture())
    {
        UE_LOG(LogPolitikaStartup, Error, TEXT("Could not prepare the RTS fixture ground and navigation data."));
        return 6;
    }
    return 0;
}
