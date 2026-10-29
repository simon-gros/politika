#include "PolitikaImportInfantryCommandlet.h"
#include "PolitikaInfantryPresentation.h"
#include "PolitikaInfantryBehavior.h"
#include "PolitikaPrototypeContent.h"
#include "PolitikaBootstrap.h"
#include "AssetToolsModule.h"
#include "IAssetTools.h"
#include "AssetCompilingManager.h"
#include "AssetImportTask.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Factories/FbxFactory.h"
#include "Factories/FbxImportUI.h"
#include "Factories/FbxAnimSequenceImportData.h"
#include "Factories/TextureFactory.h"
#include "Factories/SoundFactory.h"
#include "Animation/AnimSequence.h"
#include "Animation/Skeleton.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/Texture2D.h"
#include "Engine/StaticMesh.h"
#include "Engine/Engine.h"
#include "Materials/Material.h"
#include "Materials/MaterialExpressionConstant3Vector.h"
#include "Materials/MaterialExpressionTextureSample.h"
#include "Sound/SoundWave.h"
#include "Sound/SoundConcurrency.h"
#include "Misc/FileHelper.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"
#include "Rendering/SkeletalMeshRenderData.h"
#include "FileHelpers.h"
#include "HAL/FileManager.h"
#include "Misc/PackageName.h"
#include "Misc/Paths.h"
#include "Misc/Parse.h"
#include "UObject/SavePackage.h"

namespace
{
FString Root(TEXT("/Game/Politika/Prototypes/Infantry/Rifle"));

bool Save(UObject* Object)
{
    if (!Object) return false;
    UPackage* Package = Object->GetOutermost();
    const FString File = FPackageName::LongPackageNameToFilename(Package->GetName(), FPackageName::GetAssetPackageExtension());
    IFileManager::Get().MakeDirectory(*FPaths::GetPath(File), true);
    FSavePackageArgs Args;
    Args.TopLevelFlags = RF_Public | RF_Standalone;
    return UPackage::SavePackage(Package, Object, *File, Args);
}

template<typename T> T* Make(const FString& Name)
{
    const FString PackageName = Root / Name;
    if (UObject* Existing = StaticLoadObject(T::StaticClass(), nullptr, *(PackageName + TEXT(".") + Name)))
        return Cast<T>(Existing);
    T* Object = NewObject<T>(CreatePackage(*PackageName), *Name, RF_Public | RF_Standalone);
    FAssetRegistryModule::AssetCreated(Object);
    return Object;
}

template<typename T> T* Import(const FString& File, const FString& Name, UFactory* Factory, UObject* Options = nullptr)
{
    const FString Destination = Root / TEXT("Media");
    if (auto* Existing = LoadObject<T>(nullptr, *(Destination / Name + TEXT(".") + Name))) return Existing;
    if (!IFileManager::Get().FileExists(*File))
    {
        UE_LOG(LogPolitikaStartup, Error, TEXT("INFANTRY_IMPORT_MISSING file=%s"), *File);
        return nullptr;
    }
    auto* Task = NewObject<UAssetImportTask>();
    Task->Filename = File;
    Task->DestinationPath = Destination;
    Task->DestinationName = Name;
    Task->Factory = Factory;
    Task->Options = Options;
    Task->bAutomated = true;
    Task->bSave = true;
    Task->bReplaceExisting = false;
    FModuleManager::LoadModuleChecked<FAssetToolsModule>(TEXT("AssetTools")).Get().ImportAssetTasks({Task});
    for (UObject* Object : Task->GetObjects()) if (auto* Result = Cast<T>(Object)) return Result;
    return LoadObject<T>(nullptr, *(Destination / Name + TEXT(".") + Name));
}
}

UPolitikaImportInfantryCommandlet::UPolitikaImportInfantryCommandlet()
{
    IsClient = false; IsEditor = true; LogToConsole = true;
}

int32 UPolitikaImportInfantryCommandlet::Main(const FString& Params)
{
    const bool bCC0SquadKit = FParse::Param(*Params, TEXT("CC0SquadKit"));
    const bool bMiazmatic = FParse::Param(*Params, TEXT("MiazmaticLocal"));
    const bool bLocalRifleInfantry = FParse::Param(*Params, TEXT("LocalRifleInfantry"));
    if (bCC0SquadKit) Root = TEXT("/Game/Politika/Prototypes/Infantry/CC0SquadKit");
    if (bMiazmatic) Root = TEXT("/Game/Politika/Prototypes/Infantry/MiazmaticLocal");
    if (bLocalRifleInfantry) Root = TEXT("/Game/Politika/Prototypes/Infantry/RifleInfantry");
    FString Source;
    if (!FParse::Value(*Params, TEXT("Source="), Source))
    {
        UE_LOG(LogPolitikaStartup, Error, TEXT("Use -run=PolitikaImportInfantry -Source=<converted directory>"));
        return 1;
    }
    const FString Prefix = bCC0SquadKit ? TEXT("CC0_RifleInfantry") : bMiazmatic ? TEXT("Miazmatic_LightInfantry")
        : bLocalRifleInfantry ? TEXT("PT_RifleInfantry") : TEXT("PT_RifleInfantry");
    const FString MapPackage = Root / TEXT("RTSInfantryPlaytest");
    const FString MapFile = FPackageName::LongPackageNameToFilename(MapPackage, FPackageName::GetMapPackageExtension());
    // Existing imported media is reused. Never regenerate an authored play-test map silently.
    auto* MeshOptions = NewObject<UFbxImportUI>();
    MeshOptions->MeshTypeToImport = FBXIT_SkeletalMesh;
    MeshOptions->bAutomatedImportShouldDetectType = false;
    MeshOptions->bImportAsSkeletal = true;
    MeshOptions->bImportMesh = true;
    MeshOptions->bImportAnimations = false;
    MeshOptions->bImportMaterials = bCC0SquadKit;
    MeshOptions->bImportTextures = false;
    MeshOptions->bCreatePhysicsAsset = false;
    auto* Mesh = Import<USkeletalMesh>(Source / TEXT("RifleInfantry_Mesh.fbx"), TEXT("SK_") + Prefix, NewObject<UFbxFactory>(), MeshOptions);
    UE_LOG(LogPolitikaStartup, Display, TEXT("INFANTRY_IMPORT_MESH asset=%s skeleton=%s"),
        Mesh ? *Mesh->GetPathName() : TEXT("missing"), Mesh && Mesh->GetSkeleton() ? *Mesh->GetSkeleton()->GetPathName() : TEXT("missing"));
    if (Mesh && !Mesh->GetSkeleton())
    {
        auto* ExistingSkeleton = LoadObject<USkeleton>(nullptr, *(Root / TEXT("Media") / (TEXT("SKEL_") + Prefix) + TEXT(".SKEL_") + Prefix));
        if (ExistingSkeleton && ExistingSkeleton->GetReferenceSkeleton().GetNum() == Mesh->GetRefSkeleton().GetNum())
        {
            Mesh->SetSkeleton(ExistingSkeleton);
            Save(Mesh);
            UE_LOG(LogPolitikaStartup, Display, TEXT("INFANTRY_IMPORT_REBOUND_SKELETON mesh=%s skeleton=%s"),
                *Mesh->GetPathName(), *ExistingSkeleton->GetPathName());
        }
    }
    if (!Mesh || !Mesh->GetSkeleton()) return 2;
    const FString SkeletonName = TEXT("SKEL_") + Prefix;
    if (Mesh->GetSkeleton()->GetName() != SkeletonName)
    {
        const TArray<FAssetRenameData> Renames{FAssetRenameData(TWeakObjectPtr<UObject>(Mesh->GetSkeleton()), Root / TEXT("Media"), SkeletonName)};
        if (!FModuleManager::LoadModuleChecked<FAssetToolsModule>(TEXT("AssetTools")).Get().RenameAssets(Renames)) return 2;
        auto* RenamedSkeleton = LoadObject<USkeleton>(nullptr, *(Root / TEXT("Media") / (SkeletonName + TEXT(".") + SkeletonName)));
        if (!RenamedSkeleton) return 2;
        Mesh->SetSkeleton(RenamedSkeleton);
        if (!Save(Mesh)) return 2;
    }
    auto ImportAnimation = [&](const TCHAR* Role) -> UAnimSequence*
    {
        auto* Options = NewObject<UFbxImportUI>();
        Options->MeshTypeToImport = FBXIT_Animation;
        Options->bAutomatedImportShouldDetectType = false;
        Options->bImportAsSkeletal = true;
        Options->bImportMesh = false;
        Options->bImportAnimations = true;
        Options->Skeleton = Mesh->GetSkeleton();
        Options->AnimSequenceImportData->AnimationLength = FBXALIT_ExportedTime;
        Options->AnimSequenceImportData->bImportBoneTracks = true;
        Options->AnimSequenceImportData->bUseDefaultSampleRate = true;
        return Import<UAnimSequence>(Source / FString::Printf(TEXT("RifleInfantry_%s.fbx"), Role),
            FString::Printf(TEXT("AS_%s_%s"), *Prefix, Role), NewObject<UFbxFactory>(), Options);
    };
    auto* Profile = Make<UPolitikaInfantryPresentationDefinition>(bCC0SquadKit ? TEXT("DA_CC0RifleInfantryPresentation") : bMiazmatic ? TEXT("DA_MiazmaticLightInfantryPresentation")
        : TEXT("DA_RifleInfantryPresentation"));
    Profile->StableId = bCC0SquadKit ? TEXT("presentation.playtest.cc0_rifle_infantry") : bMiazmatic ? TEXT("presentation.playtest.miazmatic_light_infantry")
        : bLocalRifleInfantry ? TEXT("presentation.playtest.rifle_infantry") : TEXT("presentation.playtest.rifle_infantry");
    Profile->SourceDescription = bMiazmatic
        ? TEXT("Terran Light Infantry v1.1 by miazmatic, CurseForge SC2 Assets file 2678592, https://www.curseforge.com/sc2/assets/miazmatics-assets/files/2678592; SC2 M3 converted to FBX using M3Studio bef1b08136d73d6b530d07068dbb510aeba4c6db and Blender 4.5.15. Local prototype use only; retain attribution; not for redistribution.")
        : bCC0SquadKit
        ? TEXT("Low-poly Squad Kit 1.0 by Toolbench Dev; CC0 1.0; https://toolbenchdev.com/gamedev/cc0-game-assets/low-poly-squad-kit/; imported and retargeted for Unreal.")
        : bLocalRifleInfantry
        ? TEXT("Locally sourced external reference mesh and animation data; retained for POLITIKA playtest only. Source attribution and analysis are maintained in the project's separate provenance record. Not original POLITIKA production art.")
        : TEXT("External reference placeholder; exact source attribution retained outside this repository at /mnt/data/politika-reference-provenance/20261028. Not original POLITIKA art.");
    Profile->bPlaytestPlaceholder = true;
    Profile->bRequireAudio = !bCC0SquadKit && !bMiazmatic && !bLocalRifleInfantry;
    Profile->bRequireExtendedPlaytestMedia = !bCC0SquadKit && !bMiazmatic && !bLocalRifleInfantry;
    Profile->Mesh = Mesh;
    Profile->Idle = ImportAnimation(TEXT("Idle"));
    Profile->Move = ImportAnimation(TEXT("Move"));
    Profile->Fire = ImportAnimation(TEXT("Fire"));
    Profile->Death = ImportAnimation(TEXT("Death"));
    Profile->ReloadClip = bLocalRifleInfantry ? nullptr : ImportAnimation(TEXT("Reload"));
    Profile->GrenadeClip = bLocalRifleInfantry ? nullptr : ImportAnimation(TEXT("Grenade"));
    Profile->ReadyClip = ImportAnimation(TEXT("Ready"));
    Profile->SelectClip = ImportAnimation(TEXT("Select"));
    Profile->DeselectClip = ImportAnimation(TEXT("Deselect"));
    Profile->DeathAlternate = ImportAnimation(TEXT("DeathAlternate"));
    Profile->DeathBlast = ImportAnimation(TEXT("DeathBlast"));
    if ((!bLocalRifleInfantry && (!Profile->ReloadClip || !Profile->GrenadeClip)) || !Profile->ReadyClip || !Profile->SelectClip
        || !Profile->DeselectClip || !Profile->DeathAlternate || !Profile->DeathBlast) return 4;
    Profile->MuzzleSocket = bCC0SquadKit ? TEXT("Hand_R") : bMiazmatic ? TEXT("Unit_MarineMercenary_AutoShotgun")
        : bLocalRifleInfantry ? TEXT("B_WEAPONA_FX") : TEXT("fx_weapon_01");
    if (bCC0SquadKit)
    {
        auto* WeaponOptions = NewObject<UFbxImportUI>();
        WeaponOptions->MeshTypeToImport = FBXIT_StaticMesh;
        WeaponOptions->bAutomatedImportShouldDetectType = false;
        WeaponOptions->bImportMesh = true;
        WeaponOptions->bImportAsSkeletal = false;
        WeaponOptions->bImportMaterials = true;
        WeaponOptions->bImportTextures = false;
        Profile->WeaponMesh = Import<UStaticMesh>(Source / TEXT("RifleInfantry_Weapon.fbx"), TEXT("SM_CC0_RifleInfantry_Weapon"), NewObject<UFbxFactory>(), WeaponOptions);
        Profile->WeaponSocket = TEXT("Hand_R");
        if (!Profile->WeaponMesh) return 3;
    }
    // The kit's authoring coordinates are metres; UE gameplay coordinates are centimetres.
    Profile->MeshScale = bCC0SquadKit ? FVector(100.0f) : bMiazmatic ? FVector(1.0f) : bLocalRifleInfantry ? FVector(10.0f) : FVector(2.4f);
    Profile->LocomotionReferenceSpeed = 210;
    if (bLocalRifleInfantry)
    {
        Profile->CorpseSinkDelaySeconds = FMath::Max3(Profile->Death->GetPlayLength(),
            Profile->DeathAlternate->GetPlayLength(), Profile->DeathBlast->GetPlayLength()) + 0.1f;
        Profile->CorpseLifetimeSeconds = Profile->CorpseSinkDelaySeconds + 2.0f;
    }

    UTexture2D* Texture = nullptr;
    UTexture2D* Specular = nullptr;
    UMaterialInterface* Material = nullptr;
    TArray<UMaterial*> CC0Materials;
    if (bCC0SquadKit)
    {
        const FLinearColor Palette[] = {
            FLinearColor(0.5271f, 0.0409f, 0.0242f), FLinearColor(0.7991f, 0.5841f, 0.3712f),
            FLinearColor(0.0232f, 0.0423f, 0.0578f), FLinearColor(0.0467f, 0.0630f, 0.0802f),
            FLinearColor(0.0953f, 0.1274f, 0.1559f)};
        Mesh->GetMaterials().SetNum(FMath::Max(5, Mesh->GetMaterials().Num()));
        for (int32 Index = 0; Index < UE_ARRAY_COUNT(Palette); ++Index)
        {
            auto* PaletteMaterial = Make<UMaterial>(FString::Printf(TEXT("M_CC0_RifleInfantry_%02d"), Index));
            PaletteMaterial->GetEditorOnlyData()->BaseColor.Constant = Palette[Index];
            PaletteMaterial->GetEditorOnlyData()->EmissiveColor.Constant = Palette[Index];
            PaletteMaterial->GetEditorOnlyData()->Roughness.Constant = 0.78f;
            PaletteMaterial->SetShadingModel(MSM_Unlit);
            if (PaletteMaterial->GetExpressions().IsEmpty())
            {
                auto* ColorExpression = NewObject<UMaterialExpressionConstant3Vector>(PaletteMaterial);
                ColorExpression->Constant = Palette[Index];
                PaletteMaterial->GetExpressionCollection().AddExpression(ColorExpression);
                PaletteMaterial->GetEditorOnlyData()->BaseColor.Expression = ColorExpression;
                PaletteMaterial->GetEditorOnlyData()->EmissiveColor.Expression = ColorExpression;
            }
            PaletteMaterial->SetMaterialUsage(MATUSAGE_SkeletalMesh);
            PaletteMaterial->PostEditChange();
            Mesh->GetMaterials()[Index].MaterialInterface = PaletteMaterial;
            CC0Materials.Add(PaletteMaterial);
        }
        Material = Mesh->GetMaterials()[0].MaterialInterface;
        if (!Profile->WeaponMesh->GetStaticMaterials().IsEmpty())
            Profile->WeaponMesh->GetStaticMaterials()[0].MaterialInterface = CC0Materials[2];
    }
    else if (bMiazmatic)
    {
        auto* ModelMaterial = Make<UMaterial>(TEXT("M_Miazmatic_LightInfantry"));
        if (ModelMaterial->GetExpressions().IsEmpty())
        {
            auto* Color = NewObject<UMaterialExpressionConstant3Vector>(ModelMaterial);
            Color->Constant = FLinearColor(0.36f, 0.42f, 0.34f);
            ModelMaterial->GetExpressionCollection().AddExpression(Color);
            ModelMaterial->GetEditorOnlyData()->BaseColor.Expression = Color;
            ModelMaterial->GetEditorOnlyData()->Roughness.Constant = 0.82f;
            ModelMaterial->SetMaterialUsage(MATUSAGE_SkeletalMesh);
            ModelMaterial->PostEditChange();
        }
        Material = ModelMaterial;
        for (auto& Slot : Mesh->GetMaterials()) Slot.MaterialInterface = Material;
    }
    else if (bLocalRifleInfantry)
    {
        Texture = Import<UTexture2D>(Source / TEXT("RifleInfantry_Diffuse.png"), TEXT("T_PT_RifleInfantry_D"), NewObject<UTextureFactory>());
        if (!Texture) return 3;
        auto* PrototypeMaterial = Make<UMaterial>(TEXT("M_PT_RifleInfantry"));
        if (PrototypeMaterial->GetExpressions().IsEmpty())
        {
            auto* Sample = NewObject<UMaterialExpressionTextureSample>(PrototypeMaterial);
            Sample->Texture = Texture;
            Sample->SamplerType = SAMPLERTYPE_Color;
            PrototypeMaterial->GetExpressionCollection().AddExpression(Sample);
            PrototypeMaterial->GetEditorOnlyData()->BaseColor.Expression = Sample;
            PrototypeMaterial->GetEditorOnlyData()->Roughness.Constant = 0.78f;
            PrototypeMaterial->TwoSided = true;
            PrototypeMaterial->SetMaterialUsage(MATUSAGE_SkeletalMesh);
            PrototypeMaterial->PostEditChange();
        }
        Material = PrototypeMaterial;
        for (auto& Slot : Mesh->GetMaterials()) Slot.MaterialInterface = Material;
    }
    else
    {
        Texture = Import<UTexture2D>(Source / TEXT("RifleInfantry_Diffuse.png"), TEXT("T_PT_RifleInfantry_D"), NewObject<UTextureFactory>());
        Specular = Import<UTexture2D>(Source / TEXT("RifleInfantry_Specular.png"), TEXT("T_PT_RifleInfantry_S"), NewObject<UTextureFactory>());
        if (!Texture || !Specular) return 3;
        Specular->SRGB = false;
        Specular->PostEditChange();
        auto* PrototypeMaterial = Make<UMaterial>(TEXT("M_PT_RifleInfantry"));
        if (PrototypeMaterial->GetExpressions().IsEmpty())
        {
            auto* Sample = NewObject<UMaterialExpressionTextureSample>(PrototypeMaterial);
            Sample->Texture = Texture;
            Sample->SamplerType = SAMPLERTYPE_Color;
            PrototypeMaterial->GetExpressionCollection().AddExpression(Sample);
            PrototypeMaterial->GetEditorOnlyData()->BaseColor.Expression = Sample;
            PrototypeMaterial->GetEditorOnlyData()->Roughness.Constant = 0.7f;
            auto* SpecularSample = NewObject<UMaterialExpressionTextureSample>(PrototypeMaterial);
            SpecularSample->Texture = Specular;
            SpecularSample->SamplerType = SAMPLERTYPE_LinearColor;
            PrototypeMaterial->GetExpressionCollection().AddExpression(SpecularSample);
            PrototypeMaterial->GetEditorOnlyData()->Specular.Expression = SpecularSample;
            PrototypeMaterial->GetEditorOnlyData()->Specular.Mask = 1;
            PrototypeMaterial->GetEditorOnlyData()->Specular.MaskR = 1;
            PrototypeMaterial->TwoSided = true;
            PrototypeMaterial->SetMaterialUsage(MATUSAGE_SkeletalMesh);
            PrototypeMaterial->PostEditChange();
        }
        Material = PrototypeMaterial;
        for (auto& Slot : Mesh->GetMaterials()) Slot.MaterialInterface = Material;
    }
    USoundConcurrency* Concurrency = nullptr;
    if (!bCC0SquadKit && !bMiazmatic && !bLocalRifleInfantry)
    {
        Concurrency = Make<USoundConcurrency>(TEXT("SC_PT_Infantry"));
        Concurrency->Concurrency.MaxCount = 6;
        Concurrency->Concurrency.ResolutionRule = EMaxConcurrentResolutionRule::PreventNew;
    }
    auto ImportSound = [&](const TCHAR* Role) -> USoundWave*
    {
        const FString RoleName = FString(Role) == TEXT("Select") || FString(Role) == TEXT("Move") || FString(Role) == TEXT("Attack")
            ? TEXT("Voice_") + FString(Role) : FString(Role) == TEXT("Fire") ? TEXT("RifleShot")
            : FString(Role) == TEXT("Grenade") ? TEXT("GrenadeLaunch") : FString(Role) == TEXT("Death")
            ? TEXT("Voice_Death") : FString(Role) == TEXT("DeathBlast") ? TEXT("Voice_DeathBlast") : FString(Role);
        auto* Wave = Import<USoundWave>(Source / FString::Printf(TEXT("RifleInfantry_%s.wav"), Role),
            TEXT("SW_PT_RifleInfantry_") + RoleName, NewObject<USoundFactory>());
        if (Wave) { Wave->ConcurrencySet.Add(Concurrency); Wave->bLooping = false; Save(Wave); }
        return Wave;
    };
    if (!bCC0SquadKit && !bMiazmatic && !bLocalRifleInfantry)
    {
        Profile->SelectSound = ImportSound(TEXT("Select"));
        Profile->MoveSound = ImportSound(TEXT("Move"));
        Profile->AttackSound = ImportSound(TEXT("Attack"));
        Profile->FireSound = ImportSound(TEXT("Fire"));
        Profile->FootstepSound = ImportSound(TEXT("Footstep"));
        Profile->DeathSound = ImportSound(TEXT("Death"));
        Profile->ReloadSound = ImportSound(TEXT("Reload"));
        Profile->GrenadeSound = ImportSound(TEXT("Grenade"));
        Profile->DeathBlastSound = ImportSound(TEXT("DeathBlast"));
    }
    FString Error;
    if (!Profile->Validate(Error))
    {
        UE_LOG(LogPolitikaStartup, Error, TEXT("INFANTRY_IMPORT_INVALID %s"), *Error);
        return 4;
    }
    FAssetCompilingManager::Get().FinishAllCompilation();
    if ((!bCC0SquadKit && !bMiazmatic && !bLocalRifleInfantry && (!Save(Concurrency) || !Save(Specular) || !Save(Material)))
        || !Save(Mesh) || !Save(Mesh->GetSkeleton()) || !Save(Profile)) return 5;
    if (bLocalRifleInfantry && (!Save(Texture) || !Save(Material))) return 5;
    if (bMiazmatic && !Save(Cast<UMaterial>(Material))) return 5;
    for (UMaterial* PaletteMaterial : CC0Materials) if (!Save(PaletteMaterial)) return 5;

    auto* Canonical = LoadObject<UPolitikaPrototypeContentCatalog>(nullptr,
        TEXT("/Game/Politika/Definitions/DA_PrototypeContentCatalog.DA_PrototypeContentCatalog"));
    if (!Canonical) return 6;
    const auto* OriginalMap = Canonical->FindMap(UPolitikaPrototypeContentValidator::PrototypeMapAssetId);
    const auto* OriginalTarget = Canonical->FindUnit(TEXT("unit.opposing_patrol"));
    if (!OriginalMap || !OriginalTarget) return 6;
    auto* Weapon = Make<UPolitikaWeaponDefinition>(TEXT("DA_TestRifle"));
    Weapon->StableId = TEXT("weapon.playtest.rifle_infantry");
    Weapon->DisplayName = FText::FromString(TEXT("Placeholder rifle"));
    Weapon->RangeCm = 650; Weapon->Damage = 8; Weapon->CooldownSeconds = 0.2f;
    auto* Soldier = Make<UPolitikaUnitArchetypeDefinition>(TEXT("DA_TestRifleInfantry"));
    Soldier->StableId = TEXT("unit.playtest.rifle_infantry");
    Soldier->DisplayName = FText::FromString(TEXT("Rifle Infantry [Playtest Placeholder]"));
    Soldier->FactionId = OriginalMap->PlayerSlots[0].FactionId;
    Soldier->MaxHealth = 225; Soldier->MovementSpeedCmPerSecond = 210;
    Soldier->WeaponId = Weapon->StableId;
    Soldier->VisualMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
    Soldier->VisualScale = FVector(0.45f, 0.45f, 0.84f);
    Soldier->InfantryPresentation = Profile;
    auto* Behavior = Make<UPolitikaInfantryBehaviorDefinition>(TEXT("DA_RifleInfantryBehavior"));
    if (!Behavior->Validate() || !Save(Behavior)) return 7;
    Soldier->InfantryBehavior = Behavior;
    auto* Target = Make<UPolitikaUnitArchetypeDefinition>(TEXT("DA_TargetDummy"));
    Target->StableId = TEXT("unit.playtest.target_dummy");
    Target->DisplayName = FText::FromString(TEXT("Target dummy"));
    Target->FactionId = OriginalMap->PlayerSlots[1].FactionId;
    Target->MaxHealth = 600; Target->MovementSpeedCmPerSecond = 1;
    Target->WeaponId = OriginalTarget->WeaponId;
    Target->VisualMesh = Soldier->VisualMesh;
    Target->InfantryPresentation = Profile;
    Target->VisualScale = FVector(0.6f, 0.6f, 0.84f);
    Target->FactionTint = FLinearColor(0.8f, 0.1f, 0.07f);
    auto* Map = Make<UPolitikaPrototypeMapDefinition>(TEXT("DA_InfantryMap"));
    // Copy fields through UE serialization, not a second map schema.
    UEngine::CopyPropertiesForUnrelatedObjects(const_cast<UPolitikaPrototypeMapDefinition*>(OriginalMap), Map);
    Map->StableId = TEXT("map.test.rifle_infantry");
    Map->LevelAsset = FSoftObjectPath(MapPackage);
    Map->CameraOffset = FVector(0, -950, 1250);
    Map->UnitPlacements.Reset();
    Map->UnitPlacements.Add({Soldier->StableId, Map->PlayerSlots[0].SlotId, FVector(-350, 0, 42), FRotator::ZeroRotator});
    Map->UnitPlacements.Add({Target->StableId, Map->PlayerSlots[1].SlotId, FVector(400, 0, 42), FRotator::ZeroRotator});
    // Camera centers on the objective in the existing prototype. Keep it between the subjects.
    Map->Objective.Location = FVector(0, 0, 10);
    Map->Objective.CaptureRadiusCm = 50;
    Map->Objective.CaptureDurationSeconds = 3600;
    Map->Objective.MarkerScale = FVector(0.2f, 0.2f, 0.02f);
    auto* Catalog = Make<UPolitikaPrototypeContentCatalog>(TEXT("DA_InfantryCatalog"));
    Catalog->StableId = UPolitikaPrototypeContentCatalog::CatalogAssetId;
    Catalog->Weapons = {Weapon, const_cast<UPolitikaWeaponDefinition*>(Canonical->FindWeapon(Target->WeaponId))};
    Catalog->Units = {Soldier, Target}; Catalog->Maps = {Map};
    TArray<FPolitikaContentDiagnostic> Diagnostics;
    UPolitikaPrototypeContentValidator::ValidateCatalog(Catalog, Diagnostics);
    for (const auto& Diagnostic : Diagnostics)
        UE_LOG(LogPolitikaStartup, Display, TEXT("INFANTRY_IMPORT_DIAGNOSTIC %s"), *UPolitikaPrototypeContentValidator::FormatDiagnostic(Diagnostic));
    if (UPolitikaPrototypeContentValidator::HasErrors(Diagnostics)) return 7;
    if (!Save(Weapon) || !Save(Soldier) || !Save(Target) || !Save(Map) || !Save(Catalog)) return 8;
    if (!IFileManager::Get().FileExists(*MapFile))
    {
        UWorld* World = UEditorLoadingAndSavingUtils::LoadMap(TEXT("/Game/Maps/RTSVerticalSlice"));
        if (!World || !FEditorFileUtils::SaveMap(World, MapFile)) return 9;
    }
    if (FParse::Param(*Params, TEXT("PromoteDefault")))
    {
        auto* DefaultMap = const_cast<UPolitikaPrototypeMapDefinition*>(OriginalMap);
        // When upgrading a previously promoted prototype, collapse same-ID definitions
        // back to the preserved canonical play-test objects before appending them.
        Canonical->Weapons.RemoveAll([&](const auto& Existing)
        { return !Existing || (Existing != Weapon && Existing->StableId == Weapon->StableId); });
        Canonical->Units.RemoveAll([&](const auto& Existing)
        { return !Existing || ((Existing == Soldier || Existing->StableId == Soldier->StableId
            || Existing->StableId == Target->StableId) && Existing != Soldier && Existing != Target); });
        Canonical->Weapons.AddUnique(Weapon);
        Canonical->Units.AddUnique(Soldier);
        Canonical->Units.AddUnique(Target);
        for (auto& Placement : DefaultMap->UnitPlacements)
        {
            const auto* Slot = DefaultMap->PlayerSlots.FindByPredicate([&](const FPolitikaPrototypePlayerSlot& S) { return S.SlotId == Placement.SlotId; });
            if (Slot && Slot->bHumanControlled) Placement.UnitArchetypeId = Soldier->StableId;
            else if (Slot) Placement.UnitArchetypeId = Target->StableId;
        }
        auto* Patrol = const_cast<UPolitikaUnitArchetypeDefinition*>(OriginalTarget);
        Patrol->InfantryPresentation = Profile;
        if (!Save(DefaultMap) || !Save(Canonical) || !Save(Patrol)) return 10;
        // Keep the existing negative fixtures identical to the current authored map except for their explicit fault.
        for (const TCHAR* Name : {TEXT("DA_InvalidMapSchema"), TEXT("DA_InvalidParticipantFaction")})
        {
            const FString P = FString(TEXT("/Game/Politika/TestFixtures/")) + Name;
            auto* Negative = LoadObject<UPolitikaPrototypeMapDefinition>(nullptr, *(P + TEXT(".") + Name));
            if (!Negative) return 10;
            UEngine::CopyPropertiesForUnrelatedObjects(DefaultMap, Negative);
            if (FString(Name) == TEXT("DA_InvalidMapSchema"))
            { Negative->StableId = TEXT("map.test.invalid_schema"); Negative->SchemaVersion = 2; }
            else
            { Negative->StableId = TEXT("map.test.invalid_participant_faction"); Negative->UnitPlacements[0].UnitArchetypeId = OriginalTarget->StableId; }
            if (!Save(Negative)) return 10;
        }
        UE_LOG(LogPolitikaStartup, Display, TEXT("INFANTRY_DEFAULT_PROMOTED map=%s unit=%s"), *DefaultMap->StableId.ToString(), *Soldier->StableId.ToString());
    }
    TArray<TSharedPtr<FJsonValue>> Entries;
    auto Record = [&](const TCHAR* Role, UObject* Asset, double Size)
    {
        auto Item = MakeShared<FJsonObject>();
        Item->SetStringField(TEXT("role"), Role);
        Item->SetStringField(TEXT("asset"), Asset ? Asset->GetPathName() : TEXT("missing"));
        Item->SetStringField(TEXT("class"), Asset ? Asset->GetClass()->GetName() : TEXT("missing"));
        Item->SetNumberField(TEXT("size_or_duration"), Size);
        Entries.Add(MakeShared<FJsonValueObject>(Item));
        return Asset && FMath::IsFinite(Size) && Size > 0;
    };
    bool bComplete = true;
    const auto* RenderData = Mesh->GetResourceForRendering();
    bComplete &= Record(TEXT("skinned_mesh_vertices"), Mesh, RenderData && RenderData->LODRenderData.Num() ? RenderData->LODRenderData[0].GetNumVertices() : 0);
    bComplete &= Record(TEXT("skeleton_bones"), Mesh->GetSkeleton(), Mesh->GetRefSkeleton().GetNum());
    const UAnimSequence* Clips[] = {Profile->Idle,Profile->Move,Profile->Fire,Profile->Death,Profile->ReloadClip,Profile->GrenadeClip,
        Profile->ReadyClip,Profile->SelectClip,Profile->DeselectClip,Profile->DeathAlternate,Profile->DeathBlast};
    for (const auto* Clip : Clips)
    {
        if (bLocalRifleInfantry && !Clip) continue;
        bComplete &= Record(TEXT("animation_seconds"), const_cast<UAnimSequence*>(Clip), Clip && Clip->GetNumberOfSampledKeys() > 1 ? Clip->GetPlayLength() : 0);
    }
    const USoundBase* Cues[] = {Profile->SelectSound,Profile->MoveSound,Profile->AttackSound,Profile->FireSound,Profile->FootstepSound,
        Profile->DeathSound,Profile->ReloadSound,Profile->GrenadeSound,Profile->DeathBlastSound};
    for (const auto* Cue : Cues)
    {
        if (bCC0SquadKit || bMiazmatic || bLocalRifleInfantry) continue;
        auto* Wave = const_cast<USoundWave*>(Cast<USoundWave>(Cue));
        TArray<uint8> PCM;
        uint32 Rate = 0;
        uint16 Channels = 0;
        const bool bDecoded = Wave && Wave->GetImportedSoundWaveData(PCM, Rate, Channels)
            && PCM.Num() > 0 && Rate > 0 && Channels > 0;
        bComplete &= Record(TEXT("sound_seconds"), Wave, bDecoded ? Wave->GetDuration() : 0);
    }
    // Source dimensions remain available under -nullrhi; resource dimensions do not.
    if (!bCC0SquadKit && !bMiazmatic && !bLocalRifleInfantry)
    {
        bComplete &= Record(TEXT("diffuse_pixels"), Texture, Texture->Source.GetSizeX() * Texture->Source.GetSizeY());
        bComplete &= Record(TEXT("specular_pixels"), Specular, Specular->Source.GetSizeX() * Specular->Source.GetSizeY());
    }
    if (bLocalRifleInfantry)
        bComplete &= Record(TEXT("diffuse_pixels"), Texture, Texture ? Texture->Source.GetSizeX() * Texture->Source.GetSizeY() : 0);
    bComplete &= Record(TEXT("material_slots"), Material, Mesh->GetMaterials().Num());
    auto Report = MakeShared<FJsonObject>();
    Report->SetBoolField(TEXT("complete"), bComplete);
    Report->SetArrayField(TEXT("assets"), Entries);
    FString Json;
    FJsonSerializer::Serialize(Report, TJsonWriterFactory<>::Create(&Json));
    const FString ReportPath = FPaths::ProjectSavedDir() / TEXT("InfantryDefaultAudit20261028/import-inventory.json");
    if (!FFileHelper::SaveStringToFile(Json, *ReportPath) || !bComplete) return 11;
    UE_LOG(LogPolitikaStartup, Display, TEXT("INFANTRY_IMPORT_SUCCESS map=%s bones=%d idle=%.3f move=%.3f fire=%.3f death=%.3f"),
        *MapPackage, Mesh->GetRefSkeleton().GetNum(),
        Profile->Idle->GetPlayLength(), Profile->Move->GetPlayLength(), Profile->Fire->GetPlayLength(), Profile->Death->GetPlayLength());
    return 0;
}
