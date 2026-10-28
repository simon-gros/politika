#include "PolitikaImportTerrainCommandlet.h"

#include "AssetImportTask.h"
#include "AssetToolsModule.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkyLightComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/SkyLight.h"
#include "Engine/Texture2D.h"
#include "EngineUtils.h"
#include "Factories/TextureFactory.h"
#include "FileHelpers.h"
#include "HAL/FileManager.h"
#include "Landscape.h"
#include "LandscapeComponent.h"
#include "LandscapeHeightfieldCollisionComponent.h"
#include "Materials/Material.h"
#include "Materials/MaterialExpressionAdd.h"
#include "Materials/MaterialExpressionComponentMask.h"
#include "Materials/MaterialExpressionConstant3Vector.h"
#include "Materials/MaterialExpressionMultiply.h"
#include "Materials/MaterialExpressionTextureSample.h"
#include "Materials/MaterialExpressionWorldPosition.h"
#include "Misc/FileHelper.h"
#include "Misc/PackageName.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"
#include "PolitikaBootstrap.h"
#include "UObject/SavePackage.h"

namespace
{
constexpr int32 Side = 505; // 8 x 8 components, 63 quads each.
constexpr float Extent = 6000.0f;
const FName TerrainTag(TEXT("PolitikaCC0Terrain002"));
const FString Root(TEXT("/Game/Politika/Terrain/Terrain002"));

bool SaveAsset(UObject* Asset)
{
    UPackage* Package = Asset->GetOutermost();
    const FString Filename = FPackageName::LongPackageNameToFilename(Package->GetName(), FPackageName::GetAssetPackageExtension());
    IFileManager::Get().MakeDirectory(*FPaths::GetPath(Filename), true);
    FSavePackageArgs Args;
    Args.TopLevelFlags = RF_Public | RF_Standalone;
    return UPackage::SavePackage(Package, Asset, *Filename, Args);
}

template<typename T> T* MakeAsset(const TCHAR* Name, const FString& AssetRoot = Root)
{
    const FString PackageName = AssetRoot / Name;
    if (T* Existing = LoadObject<T>(nullptr, *(PackageName + TEXT(".") + Name), nullptr, LOAD_NoWarn)) return Existing;
    T* Asset = NewObject<T>(CreatePackage(*PackageName), Name, RF_Public | RF_Standalone);
    FAssetRegistryModule::AssetCreated(Asset);
    return Asset;
}

template<typename T> T* Expression(UMaterial* Material)
{
    T* Node = NewObject<T>(Material);
    Material->GetExpressionCollection().AddExpression(Node);
    return Node;
}

UMaterial* MakeTerrainMaterial(UTexture2D* Color)
{
    UMaterial* Material = MakeAsset<UMaterial>(TEXT("M_Terrain002"));
    if (Material->GetExpressions().IsEmpty())
    {
        auto* Position = Expression<UMaterialExpressionWorldPosition>(Material);
        auto* Mask = Expression<UMaterialExpressionComponentMask>(Material);
        Mask->Input.Expression = Position;
        Mask->R = Mask->G = true;
        auto* Offset = Expression<UMaterialExpressionAdd>(Material);
        Offset->A.Expression = Mask;
        Offset->ConstB = Extent;
        auto* Scale = Expression<UMaterialExpressionMultiply>(Material);
        Scale->A.Expression = Offset;
        Scale->ConstB = 1.0f / (2.0f * Extent);
        auto* Sample = Expression<UMaterialExpressionTextureSample>(Material);
        Sample->Texture = Color;
        Sample->SamplerType = SAMPLERTYPE_Color;
        Sample->Coordinates.Expression = Scale;
        Material->GetEditorOnlyData()->BaseColor.Expression = Sample;
        Material->GetEditorOnlyData()->Roughness.Constant = 0.95f;
        Material->PostEditChange();
    }
    return Material;
}

// Explicit material-only upgrade: never regenerate terrain heights, collision or navigation.
FString TerrainGeometrySnapshot(UWorld* World)
{
    TArray<FString> Rows;
    for (TActorIterator<AActor> It(World); It; ++It)
    {
        if (It->HasAnyFlags(RF_Transient)) continue;
        Rows.Add(FString::Printf(TEXT("actor %s %s %s hidden=%d collision=%d"),
            *It->GetName(), *It->GetClass()->GetPathName(), *It->GetActorTransform().ToString(),
            It->IsHidden(), It->GetActorEnableCollision()));
        TInlineComponentArray<UPrimitiveComponent*> Components;
        It->GetComponents(Components);
        for (UPrimitiveComponent* Component : Components)
        {
            Rows.Add(FString::Printf(TEXT("component %s.%s %s collision=%d nav=%d"),
                *It->GetName(), *Component->GetName(), *Component->GetRelativeTransform().ToString(),
                static_cast<int32>(Component->GetCollisionEnabled()), Component->CanEverAffectNavigation()));
            if (ULandscapeComponent* Landscape = Cast<ULandscapeComponent>(Component))
            {
                UTexture2D* Heightmap = Landscape->GetHeightmap();
                Rows.Add(FString::Printf(TEXT("height %s %s"), *Component->GetName(),
                    Heightmap ? *Heightmap->Source.GetId().ToString() : TEXT("missing")));
            }
            if (UStaticMeshComponent* Mesh = Cast<UStaticMeshComponent>(Component))
                Rows.Add(FString::Printf(TEXT("mesh %s.%s %s"), *It->GetName(), *Component->GetName(),
                    *GetPathNameSafe(Mesh->GetStaticMesh())));
        }
    }
    Rows.Sort();
    return FString::Join(Rows, TEXT("\n")) + TEXT("\n");
}

int32 ApplyGrass(UWorld* World, const FString& MapPath, const FString& Source)
{
    ALandscape* Terrain = nullptr;
    for (TActorIterator<ALandscape> It(World); It; ++It)
    {
        if (!It->ActorHasTag(TerrainTag)) continue;
        if (Terrain) return 20;
        Terrain = *It;
    }
    if (!Terrain || Terrain->LandscapeComponents.IsEmpty()) return 21;
    const FString GrassRoot(TEXT("/Game/Politika/Terrain/MarburgumGrass"));
    const TCHAR* Names[] = {TEXT("T_Grass_BaseColor"), TEXT("T_Grass_Normal"), TEXT("T_Grass_ORM")};
    const TCHAR* Files[] = {TEXT("grass_albedo.png"), TEXT("grass_normal.png"), TEXT("grass_orm.png")};
    UTexture2D* Textures[3] = {};
    for (int32 Index = 0; Index < 3; ++Index)
    {
        const FString PackagePath = GrassRoot / Names[Index];
        Textures[Index] = LoadObject<UTexture2D>(nullptr,
            *(PackagePath + TEXT(".") + Names[Index]), nullptr, LOAD_NoWarn);
        if (Textures[Index]) continue;
        if (!IFileManager::Get().FileExists(*(Source / Files[Index]))) return 22;
        UAssetImportTask* Task = NewObject<UAssetImportTask>();
        Task->Filename = Source / Files[Index];
        Task->DestinationPath = GrassRoot;
        Task->DestinationName = Names[Index];
        UTextureFactory* Factory = NewObject<UTextureFactory>();
        Factory->bFlipNormalMapGreenChannel = Index == 1; // Source is OpenGL +Y.
        Task->Factory = Factory;
        Task->bAutomated = true;
        FModuleManager::LoadModuleChecked<FAssetToolsModule>(TEXT("AssetTools")).Get().ImportAssetTasks({Task});
        Textures[Index] = LoadObject<UTexture2D>(nullptr, *(PackagePath + TEXT(".") + Names[Index]));
        if (!Textures[Index]) return 23;
        Textures[Index]->SRGB = Index == 0;
        Textures[Index]->CompressionSettings = Index == 1 ? TC_Normalmap : Index == 2 ? TC_Masks : TC_Default;
        Textures[Index]->AddressX = TA_Wrap;
        Textures[Index]->AddressY = TA_Wrap;
        Textures[Index]->LODGroup = Index == 1 ? TEXTUREGROUP_WorldNormalMap : TEXTUREGROUP_World;
        Textures[Index]->PostEditChange();
        if (!SaveAsset(Textures[Index])) return 24;
    }
    UMaterial* Material = MakeAsset<UMaterial>(TEXT("M_TestMapGrass"), GrassRoot);
    if (Material->GetExpressions().IsEmpty())
    {
        auto* Position = Expression<UMaterialExpressionWorldPosition>(Material);
        auto* Axes = Expression<UMaterialExpressionComponentMask>(Material);
        Axes->Input.Connect(0, Position);
        Axes->R = Axes->G = true;
        auto* Scale = Expression<UMaterialExpressionMultiply>(Material);
        Scale->A.Connect(0, Axes);
        Scale->ConstB = 1.0f / 400.0f; // One source tile per four metres, not per entire map.
        UMaterialExpressionTextureSample* Samples[3] = {};
        for (int32 Index = 0; Index < 3; ++Index)
        {
            Samples[Index] = Expression<UMaterialExpressionTextureSample>(Material);
            Samples[Index]->Texture = Textures[Index];
            Samples[Index]->SamplerType = Index == 1 ? SAMPLERTYPE_Normal : Index == 2 ? SAMPLERTYPE_Masks : SAMPLERTYPE_Color;
            Samples[Index]->Coordinates.Connect(0, Scale);
        }
        auto* Data = Material->GetEditorOnlyData();
        Data->BaseColor.Connect(0, Samples[0]);
        Data->Normal.Connect(0, Samples[1]);
        Data->AmbientOcclusion.Connect(1, Samples[2]);
        Data->Roughness.Connect(2, Samples[2]);
        Data->Metallic.Connect(3, Samples[2]);
        Material->PostEditChange();
        if (!SaveAsset(Material)) return 25;
    }
    if (Terrain->LandscapeMaterial == Material)
    {
        UE_LOG(LogPolitikaStartup, Display, TEXT("GRASS_MATERIAL_PRESERVED map=%s components=%d tile_cm=400"),
            *MapPath, Terrain->LandscapeComponents.Num());
        return 0;
    }
    Terrain->Modify();
    Terrain->LandscapeMaterial = Material;
    Terrain->UpdateAllComponentMaterialInstances();
    Terrain->PostEditChange();
    const FString Filename = FPackageName::LongPackageNameToFilename(MapPath, FPackageName::GetMapPackageExtension());
    if (!FEditorFileUtils::SaveMap(World, Filename)) return 26;
    UE_LOG(LogPolitikaStartup, Display, TEXT("GRASS_MATERIAL_APPLIED map=%s components=%d tile_cm=400 material=%s geometry_unchanged=1"),
        *MapPath, Terrain->LandscapeComponents.Num(), *Material->GetPathName());
    return 0;
}
}

UPolitikaImportTerrainCommandlet::UPolitikaImportTerrainCommandlet()
{
    // Landscape render components are not server-loadable. A server-only commandlet
    // can silently discard them when resaving an existing map, even with NullRHI.
    IsClient = true;
    IsServer = false;
    IsEditor = true;
    LogToConsole = true;
}

int32 UPolitikaImportTerrainCommandlet::Main(const FString& Params)
{
    FString Source;
    FString GrassSource;
    FString AuditPath;
    const bool bAudit = FParse::Value(*Params, TEXT("TerrainAudit="), AuditPath);
    const bool bGrass = FParse::Value(*Params, TEXT("GrassSource="), GrassSource);
    if (!bAudit && !bGrass && !FParse::Value(*Params, TEXT("Source="), Source)) return 1;
    FString MapPath(TEXT("/Game/Maps/RTSVerticalSlice"));
    FParse::Value(*Params, TEXT("Map="), MapPath);
    UWorld* World = UEditorLoadingAndSavingUtils::LoadMap(MapPath);
    if (!World) return 2;
    if (bAudit) return FFileHelper::SaveStringToFile(TerrainGeometrySnapshot(World), *AuditPath) ? 0 : 27;
    if (bGrass) return ApplyGrass(World, MapPath, GrassSource);
    for (TActorIterator<ALandscape> It(World); It; ++It)
    {
        if (It->ActorHasTag(TerrainTag))
        {
            UE_LOG(LogPolitikaStartup, Display, TEXT("TERRAIN_IMPORT_PRESERVED map=%s asset=Terrain002"), *MapPath);
            return 0;
        }
    }
    // Read explicit little-endian unsigned R16; do not reinterpret the EXR as raw heights.
    TArray<uint8> Bytes;
    if (!FFileHelper::LoadFileToArray(Bytes, *(Source / TEXT("Terrain002_505.r16"))) || Bytes.Num() != Side * Side * 2) return 3;
    TArray<uint16> Heights;
    Heights.SetNumUninitialized(Side * Side);
    uint16 MaxHeight = 32768;
    for (int32 Y = 0; Y < Side; ++Y)
    {
        for (int32 X = 0; X < Side; ++X)
        {
            const int32 Index = Y * Side + X;
            const uint16 Input = static_cast<uint16>(Bytes[2 * Index]) | (static_cast<uint16>(Bytes[2 * Index + 1]) << 8);
            const float WorldX = -Extent + (2.0f * Extent * X / (Side - 1));
            const float WorldY = -Extent + (2.0f * Extent * Y / (Side - 1));
            // Preserve all existing collision/navigation and the 40m square command fixture.
            // Imported elevations form surrounding relief; these are not new traversable slopes.
            const float Edge = FMath::Max(FMath::Abs(WorldX), FMath::Abs(WorldY));
            const float Alpha = FMath::Clamp((Edge - 2050.0f) / 900.0f, 0.0f, 1.0f);
            const float Smooth = Alpha * Alpha * (3.0f - 2.0f * Alpha);
            const float HeightCm = (static_cast<float>(Input) / 65535.0f) * 1500.0f * Smooth;
            Heights[Index] = static_cast<uint16>(32768 + FMath::RoundToInt(HeightCm * 8.0f));
            MaxHeight = FMath::Max(MaxHeight, Heights[Index]);
        }
    }
    UTexture2D* Color = LoadObject<UTexture2D>(nullptr, *(Root / TEXT("T_Terrain002_Color.T_Terrain002_Color")), nullptr, LOAD_NoWarn);
    if (!Color)
    {
        UAssetImportTask* Task = NewObject<UAssetImportTask>();
        Task->Filename = Source / TEXT("Terrain002_2K_Color.png");
        Task->DestinationPath = Root;
        Task->DestinationName = TEXT("T_Terrain002_Color");
        Task->Factory = NewObject<UTextureFactory>();
        Task->bAutomated = true;
        Task->bSave = true;
        FModuleManager::LoadModuleChecked<FAssetToolsModule>(TEXT("AssetTools")).Get().ImportAssetTasks({Task});
        Color = LoadObject<UTexture2D>(nullptr, *(Root / TEXT("T_Terrain002_Color.T_Terrain002_Color")));
    }
    if (!Color) return 4;
    UMaterial* TerrainMaterial = MakeTerrainMaterial(Color);
    if (!SaveAsset(TerrainMaterial)) return 5;
    ALandscape* Terrain = World->SpawnActor<ALandscape>(FVector(-Extent, -Extent, -14.0f), FRotator::ZeroRotator);
    if (!Terrain) return 6;
    Terrain->SetActorLabel(TEXT("CC0 Terrain002 - adapted test valley [scenery]"));
    Terrain->Tags.Add(TerrainTag);
    Terrain->SetActorScale3D(FVector(2.0f * Extent / (Side - 1), 2.0f * Extent / (Side - 1), 16.0f));
    Terrain->LandscapeMaterial = TerrainMaterial;
    Terrain->BodyInstance.SetCollisionEnabled(ECollisionEnabled::NoCollision);
    TMap<FGuid, TArray<uint16>> HeightData;
    HeightData.Add(FGuid(), MoveTemp(Heights));
    TMap<FGuid, TArray<FLandscapeImportLayerInfo>> LayerData;
    LayerData.Add(FGuid(), {});
    Terrain->Import(FGuid::NewGuid(), 0, 0, Side - 1, Side - 1, 1, 63, HeightData,
        *(Source / TEXT("Terrain002_505.r16")), LayerData, ELandscapeImportAlphamapType::Additive, {});
    for (ULandscapeComponent* Component : Terrain->LandscapeComponents)
    {
        Component->SetCanEverAffectNavigation(false);
        Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }
    for (ULandscapeHeightfieldCollisionComponent* Component : Terrain->CollisionComponents)
    {
        Component->SetCanEverAffectNavigation(false);
        Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }
    Terrain->UpdateAllComponentMaterialInstances();
    Terrain->PostEditChange();
    // Retain the authored command/collision ground; only its empty-box rendering is hidden.
    int32 GroundCount = 0;
    for (TActorIterator<AStaticMeshActor> It(World); It; ++It)
    {
        if (It->ActorHasTag(TEXT("PolitikaPrototypeGround")))
        {
            It->SetActorHiddenInGame(true);
            ++GroundCount;
        }
    }
    if (GroundCount != 1) return 7;
    const FString Filename = FPackageName::LongPackageNameToFilename(MapPath, FPackageName::GetMapPackageExtension());
    if (!FEditorFileUtils::SaveMap(World, Filename)) return 8;
    UE_LOG(LogPolitikaStartup, Display, TEXT("TERRAIN_IMPORTED map=%s source=ambientCG/Terrain002 license=CC0 vertices=%d components=%d max_relief_cm=%.1f ground_preserved=%d"),
        *MapPath, Side * Side, Terrain->LandscapeComponents.Num(), (MaxHeight - 32768) / 8.0f, GroundCount);
    return 0;
}
