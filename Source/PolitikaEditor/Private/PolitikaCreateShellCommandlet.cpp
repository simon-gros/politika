#include "PolitikaCreateShellCommandlet.h"

#include "Editor.h"
#include "Engine/World.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "EngineUtils.h"
#include "Components/BrushComponent.h"
#include "Components/BoxComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/Brush.h"
#include "Model.h"
#include "Engine/Polys.h"
#include "BSPOps.h"
#include "Engine/AssetManager.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "NavMesh/NavMeshBoundsVolume.h"
#include "NavModifierVolume.h"
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
#include "StaticMeshCompiler.h"
#include "NavigationData.h"
#include "PolitikaNavigationObstacleFixture.h"
#include "PolitikaNavigationFailureFixture.h"
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

bool CreateStaticObstacleDetourFixture()
{
    using namespace PolitikaNavigationObstacleFixture;
    const FString Filename = FPackageName::LongPackageNameToFilename(Level, FPackageName::GetMapPackageExtension());
    const FString DefinitionPackage = TEXT("/Game/Politika/TestFixtures/Navigation/DA_StaticObstacleDetour");
    const FString DefinitionFilename = FPackageName::LongPackageNameToFilename(DefinitionPackage, FPackageName::GetAssetPackageExtension());
    // Never overwrite an existing authored fixture (including a partial run).
    if (IFileManager::Get().FileExists(*Filename) || IFileManager::Get().FileExists(*DefinitionFilename))
    {
        UE_LOG(LogPolitikaStartup, Error, TEXT("Static obstacle fixture already exists; inspect/reuse it without regeneration."));
        return false;
    }
    const auto* Source = LoadObject<UPolitikaPrototypeMapDefinition>(nullptr,
        TEXT("/Game/Politika/Definitions/Maps/DA_RTSVerticalSlice.DA_RTSVerticalSlice"));
    UWorld* World = UEditorLoadingAndSavingUtils::LoadMap(TEXT("/Game/Maps/RTSVerticalSlice"));
    UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
    if (!Source || !World || !Cube) return false;
    // Pending static-mesh compilation makes IsNavigationRelevant() false. A
    // commandlet must finish it explicitly before exporting obstacle geometry.
    FStaticMeshCompilingManager::Get().FinishAllCompilation();
    IFileManager::Get().MakeDirectory(*FPaths::GetPath(Filename), true);
    // Like the Editor regression, modify the source world only in memory. The
    // sole save below names the isolated destination, never the canonical file.
    AStaticMeshActor* Wall = World->SpawnActor<AStaticMeshActor>(WallLocation, FRotator::ZeroRotator);
    if (!Wall) return false;
    Wall->Tags.Add(WallTag);
    Wall->SetActorLabel(TEXT("Navigation Static Obstacle"));
    Wall->GetStaticMeshComponent()->SetStaticMesh(Cube);
    Wall->SetActorScale3D(WallScale);
    Wall->GetStaticMeshComponent()->SetMobility(EComponentMobility::Static);
    Wall->GetStaticMeshComponent()->SetCollisionProfileName(TEXT("BlockAll"));
    Wall->GetStaticMeshComponent()->SetCanEverAffectNavigation(true);
    Wall->GetStaticMeshComponent()->ReregisterComponent();
    ExcludeSolidInterior(Wall);
    UNavigationSystemV1* Nav = FNavigationSystem::GetCurrent<UNavigationSystemV1>(World);
    if (!Nav) return false;
    // Match the existing PreparePrototypeFixture commandlet's collision setup;
    // unlike Editor Automation, commandlets do not initialize it through ticks.
    Nav->InitializeLevelCollisions();
    for (TActorIterator<AActor> It(World); It; ++It)
    {
        if (auto* Bounds = Cast<ANavMeshBoundsVolume>(*It)) Nav->OnNavigationBoundsUpdated(Bounds);
        UNavigationSystemV1::UpdateActorAndComponentsInNavOctree(**It);
    }
    UNavigationSystemV1::UpdateActorAndComponentsInNavOctree(*Wall);
    UE_LOG(LogPolitikaStartup, Display, TEXT("NAV_FIXTURE_GEOMETRY relevant=%d static_system=%d world=%s"),
        Wall->GetStaticMeshComponent()->IsNavigationRelevant() ? 1 : 0,
        UNavigationSystemV1::IsNavigationSystemStatic() ? 1 : 0, *World->GetPathName());
    Nav->RemoveNavigationBuildLock(ENavigationBuildLock::AsyncLoadLock, UNavigationSystemV1::ELockRemovalRebuildAction::NoRebuild);
    Nav->ReleaseInitialBuildingLock();
    // A commandlet has no Editor tick to drain newly registered geometry.
    // Build() synchronously builds tiles, but does not flush pending octree edits.
    Nav->FlushPendingOperations();
    if (!Wall->GetStaticMeshComponent()->IsNavigationRelevant()) return false;
    Nav->Build();
    int32 Wait = 0;
    while (Nav->IsNavigationBuildInProgress() && Wait++ < 600) FPlatformProcess::Sleep(0.1f);
    if (Nav->IsNavigationBuildInProgress()) return false;
    FNavLocation BlockedProjection;
    const bool bProjectsInsideWall = Nav->ProjectPointToNavigation(FVector(0, -600, 42),
        BlockedProjection, FVector(5, 5, 80));
    UE_LOG(LogPolitikaStartup, Display, TEXT("NAV_FIXTURE_BUILD blocked_projection=%d location=%s"),
        bProjectsInsideWall ? 1 : 0, *BlockedProjection.Location.ToString());
    const ANavigationData* DebugData = Nav->GetDefaultNavDataInstance(FNavigationSystem::DontCreate);
    if (DebugData)
    {
        FNavLocation DebugStart, DebugGoal;
        Nav->ProjectPointToNavigation(Start, DebugStart, FVector(80,80,180), DebugData);
        Nav->ProjectPointToNavigation(Goal, DebugGoal, FVector(80,80,180), DebugData);
        FPathFindingQuery DebugQuery(nullptr, *DebugData, DebugStart.Location, DebugGoal.Location);
        const FPathFindingResult DebugPath = Nav->FindPathSync(DebugQuery);
        UE_LOG(LogPolitikaStartup, Display, TEXT("NAV_FIXTURE_DEBUG data=%s result=%d"), *DebugData->GetPathName(), DebugPath.Result);
        if (DebugPath.Path.IsValid()) for (const FNavPathPoint& Point : DebugPath.Path->GetPathPoints())
            UE_LOG(LogPolitikaStartup, Display, TEXT("NAV_FIXTURE_DEBUG point=%s"), *Point.Location.ToString());
    }
    if (bProjectsInsideWall || !FEditorFileUtils::SaveMap(World, Filename)) return false;
    World = UEditorLoadingAndSavingUtils::LoadMap(Filename);
    if (!World || World->GetPackage()->GetName() != Level) return false;

    UPackage* Package = CreatePackage(*DefinitionPackage);
    auto* Fixture = DuplicateObject<UPolitikaPrototypeMapDefinition>(Source, Package, TEXT("DA_StaticObstacleDetour"));
    if (!Fixture) return false;
    Fixture->SetFlags(RF_Public | RF_Standalone);
    FAssetRegistryModule::AssetCreated(Fixture);
    Fixture->StableId = MapId;
    Fixture->LevelAsset = FSoftObjectPath(Level);
    Fixture->PlayerSlots[0].StartLocation = Start;
    // Use the unchanged authored fireteam (210 cm/s), not a tiny test collision body.
    FPolitikaPrototypeUnitPlacement Placement = Source->UnitPlacements[1];
    Placement.Location = Start;
    Fixture->UnitPlacements = {Placement};
    Fixture->Objective.Location = FVector(0, 1200, 4);
    Fixture->MarkPackageDirty();
    TArray<FPolitikaContentDiagnostic> Diagnostics;
    UPolitikaPrototypeContentCatalog* Catalog = LoadObject<UPolitikaPrototypeContentCatalog>(nullptr,
        TEXT("/Game/Politika/Definitions/DA_PrototypeContentCatalog.DA_PrototypeContentCatalog"));
    UPolitikaPrototypeContentValidator::ValidateMapStructure(Fixture, Catalog, Diagnostics);
    UPolitikaPrototypeContentValidator::ValidateMapSpatial(World, Fixture, Diagnostics);
    UE_LOG(LogPolitikaStartup, Display, TEXT("NAV_FIXTURE_AUTHORING diagnostics=%s"),
        *UPolitikaPrototypeContentValidator::DiagnosticsToJson(Diagnostics));
    return !UPolitikaPrototypeContentValidator::HasErrors(Diagnostics) && SaveDefinitionAsset(Fixture);
}

bool CreateDisconnectedNavigationFixture()
{
    using namespace PolitikaNavigationFailureFixture;
    const FString LevelFilename = FPackageName::LongPackageNameToFilename(Level, FPackageName::GetMapPackageExtension());
    const FString DefinitionFilename = FPackageName::LongPackageNameToFilename(Definition, FPackageName::GetAssetPackageExtension());
    if (IFileManager::Get().FileExists(*LevelFilename) || IFileManager::Get().FileExists(*DefinitionFilename))
    {
        UE_LOG(LogPolitikaStartup, Error, TEXT("Disconnected navigation fixture already exists; refusing to overwrite."));
        return false;
    }
    auto* Source = LoadObject<UPolitikaPrototypeMapDefinition>(nullptr,
        TEXT("/Game/Politika/Definitions/Maps/DA_RTSVerticalSlice.DA_RTSVerticalSlice"));
    UWorld* World = UEditorLoadingAndSavingUtils::LoadMap(TEXT("/Game/Maps/RTSVerticalSlice"));
    UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
    if (!Source || !World || !Cube) return false;
    FStaticMeshCompilingManager::Get().FinishAllCompilation();
    IFileManager::Get().MakeDirectory(*FPaths::GetPath(LevelFilename), true);
    AStaticMeshActor* Barrier = World->SpawnActor<AStaticMeshActor>(BarrierLocation, FRotator::ZeroRotator);
    if (!Barrier) return false;
    Barrier->Tags.Add(BarrierTag);
    Barrier->SetActorLabel(TEXT("Disconnected Navigation Barrier"));
    Barrier->GetStaticMeshComponent()->SetStaticMesh(Cube);
    Barrier->SetActorScale3D(BarrierScale);
    Barrier->GetStaticMeshComponent()->SetMobility(EComponentMobility::Static);
    Barrier->GetStaticMeshComponent()->SetCollisionProfileName(TEXT("BlockAll"));
    Barrier->GetStaticMeshComponent()->SetCanEverAffectNavigation(true);
    Barrier->GetStaticMeshComponent()->ReregisterComponent();
    PolitikaNavigationObstacleFixture::ExcludeSolidInterior(Barrier);
    UNavigationSystemV1* Nav = FNavigationSystem::GetCurrent<UNavigationSystemV1>(World);
    if (!Nav) return false;
    Nav->InitializeLevelCollisions();
    for (TActorIterator<AActor> It(World); It; ++It)
    {
        if (auto* Bounds = Cast<ANavMeshBoundsVolume>(*It)) Nav->OnNavigationBoundsUpdated(Bounds);
        UNavigationSystemV1::UpdateActorAndComponentsInNavOctree(**It);
    }
    Nav->RemoveNavigationBuildLock(ENavigationBuildLock::AsyncLoadLock, UNavigationSystemV1::ELockRemovalRebuildAction::NoRebuild);
    Nav->ReleaseInitialBuildingLock();
    Nav->FlushPendingOperations();
    Nav->Build();
    int32 Wait = 0;
    while (Nav->IsNavigationBuildInProgress() && Wait++ < 600) FPlatformProcess::Sleep(0.1f);
    if (Nav->IsNavigationBuildInProgress()) return false;
    const ANavigationData* Data = Nav->GetDefaultNavDataInstance(FNavigationSystem::DontCreate);
    FNavLocation ProjectedStart, ProjectedGoal;
    if (!Data || !Nav->ProjectPointToNavigation(Start, ProjectedStart, FVector(80, 80, 180), Data)
        || !Nav->ProjectPointToNavigation(Goal, ProjectedGoal, FVector(80, 80, 180), Data)
        || FVector::Dist(ProjectedStart.Location, Start) > 80.0f
        || FVector::Dist(ProjectedGoal.Location, Goal) > 80.0f) return false;
    FPathFindingQuery Query(nullptr, *Data, ProjectedStart.Location, ProjectedGoal.Location);
    Query.SetAllowPartialPaths(false);
    Query.SetRequireNavigableEndLocation(true);
    Query.SetRequireNavigableEndLocation(true);
    const FPathFindingResult Path = Nav->FindPathSync(Query);
    UE_LOG(LogPolitikaStartup, Display,
        TEXT("NAV_DISCONNECTED_FIXTURE_BUILD nav=%s start=%.3f,%.3f,%.3f goal=%.3f,%.3f,%.3f result=%u partial=%d points=%d barrier=%s"),
        *Data->GetPathName(), ProjectedStart.Location.X, ProjectedStart.Location.Y, ProjectedStart.Location.Z,
        ProjectedGoal.Location.X, ProjectedGoal.Location.Y, ProjectedGoal.Location.Z,
        static_cast<uint32>(Path.Result), Path.IsPartial() ? 1 : 0,
        Path.Path.IsValid() ? Path.Path->GetPathPoints().Num() : 0, *Barrier->GetPathName());
    if ((Path.Result == ENavigationQueryResult::Success && Path.Path.IsValid() && !Path.Path->IsPartial())
        || !FEditorFileUtils::SaveMap(World, LevelFilename)) return false;
    World = UEditorLoadingAndSavingUtils::LoadMap(LevelFilename);
    if (!World || World->GetPackage()->GetName() != Level) return false;
    UPackage* Package = CreatePackage(Definition);
    auto* Fixture = DuplicateObject<UPolitikaPrototypeMapDefinition>(Source, Package, TEXT("DA_DisconnectedNavigation"));
    if (!Fixture) return false;
    Fixture->SetFlags(RF_Public | RF_Standalone);
    FAssetRegistryModule::AssetCreated(Fixture);
    Fixture->StableId = MapId;
    Fixture->LevelAsset = FSoftObjectPath(Level);
    Fixture->PlayerSlots[0].StartLocation = Start;
    if (Fixture->PlayerSlots.Num() > 1) Fixture->PlayerSlots[1].StartLocation = FVector(-1100.0, 800.0, 42.0);
    FPolitikaPrototypeUnitPlacement Placement = Source->UnitPlacements[1];
    Placement.Location = Start;
    Fixture->UnitPlacements = {Placement};
    Fixture->Objective.Location = FVector(-1100.0, 1200.0, 4.0);
    Fixture->MarkPackageDirty();
    TArray<FPolitikaContentDiagnostic> Diagnostics;
    UPolitikaPrototypeContentCatalog* Catalog = LoadObject<UPolitikaPrototypeContentCatalog>(nullptr,
        TEXT("/Game/Politika/Definitions/DA_PrototypeContentCatalog.DA_PrototypeContentCatalog"));
    UPolitikaPrototypeContentValidator::ValidateMapStructure(Fixture, Catalog, Diagnostics);
    UPolitikaPrototypeContentValidator::ValidateMapSpatial(World, Fixture, Diagnostics);
    UE_LOG(LogPolitikaStartup, Display, TEXT("NAV_DISCONNECTED_FIXTURE_AUTHORING diagnostics=%s"),
        *UPolitikaPrototypeContentValidator::DiagnosticsToJson(Diagnostics));
    return !UPolitikaPrototypeContentValidator::HasErrors(Diagnostics) && SaveDefinitionAsset(Fixture);
}

bool CreateUnprojectableDestinationFixture()
{
    using namespace PolitikaUnprojectableDestinationFixture;
    const FString LevelFilename = FPackageName::LongPackageNameToFilename(Level, FPackageName::GetMapPackageExtension());
    const FString DefinitionFilename = FPackageName::LongPackageNameToFilename(Definition, FPackageName::GetAssetPackageExtension());
    if (IFileManager::Get().FileExists(*LevelFilename) || IFileManager::Get().FileExists(*DefinitionFilename)) return false;
    auto* Source = LoadObject<UPolitikaPrototypeMapDefinition>(nullptr, TEXT("/Game/Politika/Definitions/Maps/DA_RTSVerticalSlice.DA_RTSVerticalSlice"));
    UWorld* World = UEditorLoadingAndSavingUtils::LoadMap(TEXT("/Game/Maps/RTSVerticalSlice"));
    if (!Source || !World) return false;
    FStaticMeshCompilingManager::Get().FinishAllCompilation();
    IFileManager::Get().MakeDirectory(*FPaths::GetPath(LevelFilename), true);
    ANavModifierVolume* Exclusion = World->SpawnActor<ANavModifierVolume>(ExclusionLocation, FRotator::ZeroRotator);
    if (!Exclusion) return false;
    Exclusion->Tags.Add(ExclusionTag);
    Exclusion->SetActorLabel(TEXT("Unprojectable Destination Exclusion"));
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
    Builder->X = ExclusionExtent.X * 2.0;
    Builder->Y = ExclusionExtent.Y * 2.0;
    Builder->Z = ExclusionExtent.Z * 2.0;
    BuilderBrush->SetActorLocation(ExclusionLocation);
    if (!Builder->Build(World, BuilderBrush)) return false;
    FBSPOps::csgCopyBrush(Exclusion, BuilderBrush, 0, RF_Transactional, true, true);
    Exclusion->PostEditChange();
    if (Exclusion->Brush) Exclusion->Brush->BuildBound();
    Exclusion->GetBrushComponent()->SetMobility(EComponentMobility::Static);
    Exclusion->GetBrushComponent()->ReregisterComponent();
    BuilderBrush->SetActorLocation(FVector::ZeroVector);
    Exclusion->SetAreaClass(UNavArea_Null::StaticClass());
    UNavigationSystemV1* Nav = FNavigationSystem::GetCurrent<UNavigationSystemV1>(World);
    if (!Nav) return false;
    Nav->InitializeLevelCollisions();
    for (TActorIterator<AActor> It(World); It; ++It)
    {
        if (auto* Bounds = Cast<ANavMeshBoundsVolume>(*It)) Nav->OnNavigationBoundsUpdated(Bounds);
        UNavigationSystemV1::UpdateActorAndComponentsInNavOctree(**It);
    }
    UNavigationSystemV1::UpdateActorAndComponentsInNavOctree(*Exclusion);
    Nav->RemoveNavigationBuildLock(ENavigationBuildLock::AsyncLoadLock, UNavigationSystemV1::ELockRemovalRebuildAction::NoRebuild);
    Nav->ReleaseInitialBuildingLock();
    Nav->FlushPendingOperations();
    Nav->Build();
    int32 Wait = 0;
    while (Nav->IsNavigationBuildInProgress() && Wait++ < 600) FPlatformProcess::Sleep(0.1f);
    const ANavigationData* Data = Nav->GetDefaultNavDataInstance(FNavigationSystem::DontCreate);
    FNavLocation ProjectedStart, ProjectedGoal;
    const bool bStart = Data && Nav->ProjectPointToNavigation(Start, ProjectedStart, FVector(80, 80, 180), Data);
    const bool bGoal = Data && Nav->ProjectPointToNavigation(Goal, ProjectedGoal, FVector(80, 80, 180), Data);
    UE_LOG(LogPolitikaStartup, Display, TEXT("NAV_UNPROJECTABLE_FIXTURE_BUILD nav=%s start_projected=%d goal_projected=%d goal=%.3f,%.3f,%.3f exclusion=%s"),
        Data ? *Data->GetPathName() : TEXT("missing"), bStart ? 1 : 0, bGoal ? 1 : 0,
        Goal.X, Goal.Y, Goal.Z, *Exclusion->GetPathName());
    if (Nav->IsNavigationBuildInProgress() || !bStart || bGoal || !FEditorFileUtils::SaveMap(World, LevelFilename)) return false;
    World = UEditorLoadingAndSavingUtils::LoadMap(LevelFilename);
    Nav = World ? FNavigationSystem::GetCurrent<UNavigationSystemV1>(World) : nullptr;
    Data = Nav ? Nav->GetDefaultNavDataInstance(FNavigationSystem::DontCreate) : nullptr;
    if (!World || !Nav || !Data || Nav->IsNavigationBuildInProgress()
        || !Nav->ProjectPointToNavigation(Start, ProjectedStart, FVector(80, 80, 180), Data)
        || Nav->ProjectPointToNavigation(Goal, ProjectedGoal, FVector(80, 80, 180), Data)) return false;
    UPackage* Package = CreatePackage(Definition);
    auto* Fixture = DuplicateObject<UPolitikaPrototypeMapDefinition>(Source, Package, TEXT("DA_UnprojectableDestination"));
    if (!Fixture) return false;
    Fixture->SetFlags(RF_Public | RF_Standalone);
    FAssetRegistryModule::AssetCreated(Fixture);
    Fixture->StableId = MapId;
    Fixture->LevelAsset = FSoftObjectPath(Level);
    Fixture->PlayerSlots[0].StartLocation = Start;
    FPolitikaPrototypeUnitPlacement Placement = Source->UnitPlacements[1];
    Placement.Location = Start;
    Fixture->UnitPlacements = {Placement};
    Fixture->Objective.Location = FVector(-1100.0, 1200.0, 4.0);
    Fixture->MarkPackageDirty();
    UPolitikaPrototypeContentCatalog* Catalog = LoadObject<UPolitikaPrototypeContentCatalog>(nullptr,
        TEXT("/Game/Politika/Definitions/DA_PrototypeContentCatalog.DA_PrototypeContentCatalog"));
    TArray<FPolitikaContentDiagnostic> Diagnostics;
    UPolitikaPrototypeContentValidator::ValidateMapStructure(Fixture, Catalog, Diagnostics);
    UPolitikaPrototypeContentValidator::ValidateMapSpatial(World, Fixture, Diagnostics);
    UE_LOG(LogPolitikaStartup, Display, TEXT("NAV_UNPROJECTABLE_FIXTURE_AUTHORING diagnostics=%s"),
        *UPolitikaPrototypeContentValidator::DiagnosticsToJson(Diagnostics));
    return !UPolitikaPrototypeContentValidator::HasErrors(Diagnostics) && SaveDefinitionAsset(Fixture);
}

bool CreateProjectionDisplacementFixture()
{
    using namespace PolitikaProjectionDisplacementFixture;
    const FString LevelFilename = FPackageName::LongPackageNameToFilename(Level, FPackageName::GetMapPackageExtension());
    const FString DefinitionFilename = FPackageName::LongPackageNameToFilename(Definition, FPackageName::GetAssetPackageExtension());
    if (IFileManager::Get().FileExists(*LevelFilename) || IFileManager::Get().FileExists(*DefinitionFilename))
    {
        UE_LOG(LogPolitikaStartup, Error, TEXT("Projection-displacement fixture already exists; refusing to overwrite."));
        return false;
    }
    auto* Source = LoadObject<UPolitikaPrototypeMapDefinition>(nullptr,
        TEXT("/Game/Politika/Definitions/Maps/DA_RTSVerticalSlice.DA_RTSVerticalSlice"));
    UWorld* World = UEditorLoadingAndSavingUtils::LoadMap(TEXT("/Game/Maps/RTSVerticalSlice"));
    UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
    if (!Source || !World || !Cube) return false;
    FStaticMeshCompilingManager::Get().FinishAllCompilation();
    IFileManager::Get().MakeDirectory(*FPaths::GetPath(LevelFilename), true);

    AStaticMeshActor* Platform = World->SpawnActor<AStaticMeshActor>(PlatformLocation, FRotator::ZeroRotator);
    AStaticMeshActor* Ramp = World->SpawnActor<AStaticMeshActor>(RampLocation, FRotator(0.0, 0.0, RampAngleDegrees));
    if (!Platform || !Ramp) return false;
    Platform->Tags.Add(PlatformTag);
    Platform->SetActorLabel(TEXT("Projection Displacement Elevated Ground"));
    Platform->GetStaticMeshComponent()->SetStaticMesh(Cube);
    Platform->SetActorScale3D(PlatformScale);
    Platform->GetStaticMeshComponent()->SetMobility(EComponentMobility::Static);
    Platform->GetStaticMeshComponent()->SetCollisionProfileName(TEXT("BlockAll"));
    Platform->GetStaticMeshComponent()->SetCanEverAffectNavigation(true);
    Platform->GetStaticMeshComponent()->ReregisterComponent();
    Ramp->Tags.Add(RampTag);
    Ramp->SetActorLabel(TEXT("Projection Displacement Walkable Ramp"));
    Ramp->GetStaticMeshComponent()->SetStaticMesh(Cube);
    Ramp->SetActorScale3D(RampScale);
    Ramp->GetStaticMeshComponent()->SetMobility(EComponentMobility::Static);
    Ramp->GetStaticMeshComponent()->SetCollisionProfileName(TEXT("BlockAll"));
    Ramp->GetStaticMeshComponent()->SetCanEverAffectNavigation(true);
    Ramp->GetStaticMeshComponent()->ReregisterComponent();

    ANavModifierVolume* Exclusion = World->SpawnActor<ANavModifierVolume>(ExclusionLocation, FRotator::ZeroRotator);
    if (!Exclusion) return false;
    Exclusion->Tags.Add(ExclusionTag);
    Exclusion->SetActorLabel(TEXT("Projection Displacement Low Ground Exclusion"));
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
    Builder->X = ExclusionExtent.X * 2.0;
    Builder->Y = ExclusionExtent.Y * 2.0;
    Builder->Z = ExclusionExtent.Z * 2.0;
    BuilderBrush->SetActorLocation(ExclusionLocation);
    if (!Builder->Build(World, BuilderBrush)) return false;
    FBSPOps::csgCopyBrush(Exclusion, BuilderBrush, 0, RF_Transactional, true, true);
    Exclusion->PostEditChange();
    if (Exclusion->Brush) Exclusion->Brush->BuildBound();
    Exclusion->GetBrushComponent()->SetMobility(EComponentMobility::Static);
    Exclusion->GetBrushComponent()->ReregisterComponent();
    BuilderBrush->SetActorLocation(FVector::ZeroVector);
    Exclusion->SetAreaClass(UNavArea_Null::StaticClass());

    UNavigationSystemV1* Nav = FNavigationSystem::GetCurrent<UNavigationSystemV1>(World);
    if (!Nav) return false;
    Nav->InitializeLevelCollisions();
    for (TActorIterator<AActor> It(World); It; ++It)
    {
        if (auto* Bounds = Cast<ANavMeshBoundsVolume>(*It)) Nav->OnNavigationBoundsUpdated(Bounds);
        UNavigationSystemV1::UpdateActorAndComponentsInNavOctree(**It);
    }
    Nav->RemoveNavigationBuildLock(ENavigationBuildLock::AsyncLoadLock, UNavigationSystemV1::ELockRemovalRebuildAction::NoRebuild);
    Nav->ReleaseInitialBuildingLock();
    Nav->FlushPendingOperations();
    Nav->Build();
    int32 Wait = 0;
    while (Nav->IsNavigationBuildInProgress() && Wait++ < 600) FPlatformProcess::Sleep(0.1f);
    const ANavigationData* Data = Nav->GetDefaultNavDataInstance(FNavigationSystem::DontCreate);
    FNavLocation ProjectedStart, ProjectedGoal;
    const bool bStart = Data && Nav->ProjectPointToNavigation(Start, ProjectedStart, FVector(80, 80, 180), Data);
    const bool bGoal = Data && Nav->ProjectPointToNavigation(Goal, ProjectedGoal, FVector(80, 80, 180), Data);
    const double Displacement = bGoal ? FVector::Distance(ProjectedGoal.Location, Goal) : 0.0;
    FPathFindingResult Reachability;
    if (bStart && bGoal)
    {
        FPathFindingQuery ReachabilityQuery(nullptr, *Data, ProjectedStart.Location, ProjectedGoal.Location);
        ReachabilityQuery.SetAllowPartialPaths(false);
        ReachabilityQuery.SetRequireNavigableEndLocation(true);
        Reachability = Nav->FindPathSync(ReachabilityQuery);
    }
    const bool bConnected = Reachability.Result == ENavigationQueryResult::Success
        && Reachability.Path.IsValid() && !Reachability.Path->IsPartial()
        && !Reachability.Path->GetPathPoints().IsEmpty();
    const FVector ReachabilityEndpoint = Reachability.Path.IsValid() && !Reachability.Path->GetPathPoints().IsEmpty()
        ? Reachability.Path->GetPathPoints().Last().Location : FVector::ZeroVector;
    if (Data)
    {
        for (int32 SampleY = -800; SampleY <= 1300; SampleY += 200)
        {
            const double ExpectedZ = -14.0 + (static_cast<double>(SampleY) + 800.0) * 214.0 / 1900.0;
            FNavLocation SampleProjection;
            const bool bSampleProjected = Nav->ProjectPointToNavigation(FVector(1300.0, SampleY, ExpectedZ),
                SampleProjection, FVector(80, 80, 180), Data);
            FHitResult SampleGround;
            const bool bSampleGround = World->LineTraceSingleByChannel(SampleGround,
                FVector(1300, SampleY, ExpectedZ + 200), FVector(1300, SampleY, ExpectedZ - 200), ECC_Visibility);
            UE_LOG(LogPolitikaStartup, Display,
                TEXT("NAV_PROJECTION_DISPLACEMENT_RAMP_SAMPLE y=%.3f expected_z=%.3f projected=%d projected_location=%.3f,%.3f,%.3f ground_hit=%d ground=%.3f,%.3f,%.3f actor=%s"),
                static_cast<double>(SampleY), ExpectedZ, bSampleProjected ? 1 : 0,
                bSampleProjected ? SampleProjection.Location.X : 0.0,
                bSampleProjected ? SampleProjection.Location.Y : 0.0,
                bSampleProjected ? SampleProjection.Location.Z : 0.0, bSampleGround ? 1 : 0,
                SampleGround.ImpactPoint.X, SampleGround.ImpactPoint.Y, SampleGround.ImpactPoint.Z,
                SampleGround.GetActor() ? *SampleGround.GetActor()->GetPathName() : TEXT("none"));
        }
    }
    FHitResult GroundHit;
    const bool bGround = World->LineTraceSingleByChannel(GroundHit, Goal + FVector(0, 0, 300),
        Goal - FVector(0, 0, 300), ECC_Visibility);
    UE_LOG(LogPolitikaStartup, Display,
        TEXT("NAV_PROJECTION_DISPLACEMENT_FIXTURE_BUILD nav=%s start_projected=%d goal_projected=%d requested=%.6f,%.6f,%.6f projected_goal=%.6f,%.6f,%.6f displacement=%.6f ground_hit=%d ground=%.6f,%.6f,%.6f connected_path=%d path_result=%u partial=%d path_points=%d path_endpoint=%.6f,%.6f,%.6f platform=%s ramp=%s exclusion=%s"),
        Data ? *Data->GetPathName() : TEXT("missing"), bStart ? 1 : 0, bGoal ? 1 : 0,
        Goal.X, Goal.Y, Goal.Z, bGoal ? ProjectedGoal.Location.X : 0.0, bGoal ? ProjectedGoal.Location.Y : 0.0,
        bGoal ? ProjectedGoal.Location.Z : 0.0, Displacement, bGround ? 1 : 0,
        GroundHit.ImpactPoint.X, GroundHit.ImpactPoint.Y, GroundHit.ImpactPoint.Z, bConnected ? 1 : 0,
        static_cast<uint32>(Reachability.Result), Reachability.IsPartial() ? 1 : 0,
        Reachability.Path.IsValid() ? Reachability.Path->GetPathPoints().Num() : 0,
        ReachabilityEndpoint.X, ReachabilityEndpoint.Y, ReachabilityEndpoint.Z,
        *Platform->GetPathName(), *Ramp->GetPathName(), *Exclusion->GetPathName());
    if (Nav->IsNavigationBuildInProgress() || !bStart || !bGoal || Displacement <= 150.0
        || !bGround || !GroundHit.GetActor()->ActorHasTag(PlatformTag) || !bConnected
        || !FEditorFileUtils::SaveMap(World, LevelFilename)) return false;

    World = UEditorLoadingAndSavingUtils::LoadMap(LevelFilename);
    Nav = World ? FNavigationSystem::GetCurrent<UNavigationSystemV1>(World) : nullptr;
    Data = Nav ? Nav->GetDefaultNavDataInstance(FNavigationSystem::DontCreate) : nullptr;
    if (!World || !Nav || !Data || Nav->IsNavigationBuildInProgress()
        || !Nav->ProjectPointToNavigation(Start, ProjectedStart, FVector(80, 80, 180), Data)
        || !Nav->ProjectPointToNavigation(Goal, ProjectedGoal, FVector(80, 80, 180), Data)
        || FVector::Distance(ProjectedGoal.Location, Goal) <= 150.0) return false;

    UPackage* Package = CreatePackage(Definition);
    auto* Fixture = DuplicateObject<UPolitikaPrototypeMapDefinition>(Source, Package, TEXT("DA_ProjectionDisplacement"));
    if (!Fixture) return false;
    Fixture->SetFlags(RF_Public | RF_Standalone);
    FAssetRegistryModule::AssetCreated(Fixture);
    Fixture->StableId = MapId;
    Fixture->LevelAsset = FSoftObjectPath(Level);
    Fixture->PlayerSlots[0].StartLocation = Start;
    FPolitikaPrototypeUnitPlacement Placement = Source->UnitPlacements[1];
    Placement.Location = Start;
    Fixture->UnitPlacements = {Placement};
    Fixture->Objective.Location = FVector(-1100.0, 1200.0, 4.0);
    Fixture->MarkPackageDirty();
    UPolitikaPrototypeContentCatalog* Catalog = LoadObject<UPolitikaPrototypeContentCatalog>(nullptr,
        TEXT("/Game/Politika/Definitions/DA_PrototypeContentCatalog.DA_PrototypeContentCatalog"));
    TArray<FPolitikaContentDiagnostic> Diagnostics;
    UPolitikaPrototypeContentValidator::ValidateMapStructure(Fixture, Catalog, Diagnostics);
    UPolitikaPrototypeContentValidator::ValidateMapSpatial(World, Fixture, Diagnostics);
    UE_LOG(LogPolitikaStartup, Display, TEXT("NAV_PROJECTION_DISPLACEMENT_FIXTURE_AUTHORING diagnostics=%s"),
        *UPolitikaPrototypeContentValidator::DiagnosticsToJson(Diagnostics));
    return !UPolitikaPrototypeContentValidator::HasErrors(Diagnostics) && SaveDefinitionAsset(Fixture);
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
    if (FParse::Param(*Params, TEXT("CreateStaticObstacleDetourFixture")))
        return CreateStaticObstacleDetourFixture() ? 0 : 13;
    if (FParse::Param(*Params, TEXT("CreateDisconnectedNavigationFixture")))
        return CreateDisconnectedNavigationFixture() ? 0 : 14;
    if (FParse::Param(*Params, TEXT("CreateUnprojectableDestinationFixture")))
        return CreateUnprojectableDestinationFixture() ? 0 : 15;
    if (FParse::Param(*Params, TEXT("CreateProjectionDisplacementFixture")))
        return CreateProjectionDisplacementFixture() ? 0 : 16;
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
