#include "PolitikaImportRifleInfantryAudioCommandlet.h"

#include "AssetImportTask.h"
#include "AssetToolsModule.h"
#include "IAssetTools.h"
#include "PolitikaBootstrap.h"
#include "PolitikaInfantryPresentation.h"
#include "Sound/SoundCue.h"
#include "Sound/SoundWave.h"
#include "Sound/SoundNodeRandom.h"
#include "Sound/SoundNodeWavePlayer.h"
#include "Factories/SoundCueFactoryNew.h"
#include "HAL/FileManager.h"
#include "HAL/IConsoleManager.h"
#include "Misc/PackageName.h"
#include "Misc/Parse.h"
#include "UObject/SavePackage.h"

namespace
{
constexpr TCHAR ProfilePath[] = TEXT("/Game/Politika/Prototypes/Infantry/RifleInfantry/DA_RifleInfantryPresentation.DA_RifleInfantryPresentation");
constexpr TCHAR AssetPath[] = TEXT("/Game/Politika/Prototypes/Infantry/RifleInfantry/Media/Audio");

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

FString GroupFromStem(const FString& Stem)
{
    FString Group, Variant;
    if (!Stem.Split(TEXT("PT_RifleInfantry_"), nullptr, &Group)) return {};
    if (!Group.Split(TEXT("_"), &Group, &Variant, ESearchCase::CaseSensitive, ESearchDir::FromEnd)) return {};
    if (Variant.Len() != 1 || !FChar::IsAlpha(Variant[0])) return {};
    return Group;
}

}

UPolitikaImportRifleInfantryAudioCommandlet::UPolitikaImportRifleInfantryAudioCommandlet()
{
    IsClient = false;
    IsEditor = true;
    LogToConsole = true;
}

int32 UPolitikaImportRifleInfantryAudioCommandlet::Main(const FString& Params)
{
    FString Source;
    if (!FParse::Value(*Params, TEXT("Source="), Source))
    {
        UE_LOG(LogPolitikaStartup, Error, TEXT("Use -run=PolitikaImportRifleInfantryAudio -Source=<neutral MP3 directory>"));
        return 1;
    }
    IConsoleVariable* Mp3Import = IConsoleManager::Get().FindConsoleVariable(TEXT("Interchange.FeatureFlags.Import.Audio.MP3"));
    if (!Mp3Import)
    {
        UE_LOG(LogPolitikaStartup, Error, TEXT("UE Interchange MP3 import feature flag is unavailable."));
        return 2;
    }
    Mp3Import->Set(1, ECVF_SetByCode);

    const FString Destination = FPackageName::LongPackageNameToFilename(AssetPath);
    IFileManager::Get().MakeDirectory(*Destination, true);
    TArray<FString> Files;
    IFileManager::Get().FindFilesRecursive(Files, *Source, TEXT("*.mp3"), true, false);
    Files.Sort();
    if (Files.IsEmpty()) return 3;

    TMap<FString, TArray<USoundWave*>> WavesByGroup;
    auto& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>(TEXT("AssetTools")).Get();
    for (const FString& File : Files)
    {
        const FString Stem = FPaths::GetBaseFilename(File);
        const FString Group = GroupFromStem(Stem);
        if (Group.IsEmpty())
        {
            UE_LOG(LogPolitikaStartup, Error, TEXT("Unrecognized neutral MP3 filename: %s"), *File);
            return 4;
        }
        const FString WaveName = TEXT("SW_") + Stem;
        const FString WaveObjectPath = FString(AssetPath) + TEXT("/") + WaveName + TEXT(".") + WaveName;
        USoundWave* Wave = LoadObject<USoundWave>(nullptr, *WaveObjectPath);
        if (!Wave)
        {
            auto* Task = NewObject<UAssetImportTask>();
            Task->Filename = File;
            Task->DestinationPath = AssetPath;
            Task->bAutomated = true;
            Task->bSave = true;
            Task->bReplaceExisting = true;
            // A null factory selects UE 5.8's Interchange MP3 translator.
            Task->Factory = nullptr;
            AssetTools.ImportAssetTasks({Task});
            for (UObject* Object : Task->GetObjects())
                if ((Wave = Cast<USoundWave>(Object))) break;
            if (!Wave)
            {
                UE_LOG(LogPolitikaStartup, Error, TEXT("MP3 import produced no SoundWave: %s"), *File);
                return 5;
            }
            if (Wave->GetName() != WaveName)
            {
                const TArray<FAssetRenameData> Rename{FAssetRenameData(Wave, AssetPath, *WaveName)};
                if (!AssetTools.RenameAssets(Rename)) return 6;
                Wave = LoadObject<USoundWave>(nullptr, *WaveObjectPath);
                if (!Wave) return 6;
            }
        }
        Wave->bLooping = false;
        if (!Save(Wave)) return 7;
        WavesByGroup.FindOrAdd(Group).Add(Wave);
    }

    auto* Profile = LoadObject<UPolitikaInfantryPresentationDefinition>(nullptr, ProfilePath);
    if (!Profile) return 9;
    TMap<FString, USoundCue*> Cues;
    for (const auto& Group : WavesByGroup)
    {
        const FString Name = TEXT("SC_PT_RifleInfantry_") + Group.Key;
        const FString PackageName = FString(AssetPath) + TEXT("/") + Name;
        auto* Cue = LoadObject<USoundCue>(nullptr, *(PackageName + TEXT(".") + Name));
        if (!Cue)
        {
            UPackage* Package = CreatePackage(*PackageName);
            Package->FullyLoad();
            Cue = NewObject<USoundCue>(Package, *Name, RF_Public | RF_Standalone);
        }
        Cue->AllNodes.Reset();
        auto* Random = NewObject<USoundNodeRandom>(Cue);
        Random->bRandomizeWithoutReplacement = true;
        Cue->AllNodes.Add(Random);
        for (USoundWave* Wave : Group.Value)
        {
            auto* Player = NewObject<USoundNodeWavePlayer>(Cue);
            Player->SetSoundWave(Wave);
            Cue->AllNodes.Add(Player);
            Random->ChildNodes.Add(Player);
            Random->Weights.Add(1.0f);
            Random->HasBeenUsed.Add(false);
        }
        Cue->FirstNode = Random;
        Cue->CacheAggregateValues();
        if (!Save(Cue)) return 8;
        Cues.Add(Group.Key, Cue);
    }
    const auto FirstWave = [&Cues](const TCHAR* Name) -> USoundCue*
    {
        return Cues.FindRef(Name);
    };
    // English voices and shared Foley/fire clips are resolved by source role.
    // Roles not authored for this unit are deliberately left unset.
    Profile->SelectSound = FirstWave(TEXT("Voice_Select"));
    Profile->MoveSound = FirstWave(TEXT("Voice_Move"));
    Profile->AttackSound = FirstWave(TEXT("Voice_Attack"));
    Profile->FireSound = FirstWave(TEXT("Weapon_RifleShot"));
    Profile->FootstepSound = FirstWave(TEXT("Foley_Move"));
    Profile->DeathSound = FirstWave(TEXT("Voice_Death"));
    Profile->ReloadSound = nullptr;
    Profile->GrenadeSound = nullptr;
    Profile->DeathBlastSound = nullptr;
    Profile->bRequireAudio = true;
    Profile->bRequireExtendedPlaytestMedia = false;
    if (!Profile->SelectSound || !Profile->MoveSound || !Profile->FireSound || !Profile->FootstepSound) return 10;

    FString Error;
    if (!Profile->Validate(Error))
    {
        UE_LOG(LogPolitikaStartup, Error, TEXT("RIFLE_AUDIO_PROFILE_INVALID %s"), *Error);
        return 10;
    }
    if (!Save(Profile)) return 11;
    UE_LOG(LogPolitikaStartup, Display, TEXT("RIFLE_AUDIO_IMPORTED mp3_sources=%d soundwaves=%d profile=%s select=%s move=%s fire=%s"),
        Files.Num(), Files.Num(), *Profile->GetPathName(), *Profile->SelectSound->GetName(),
        *Profile->MoveSound->GetName(), *Profile->FireSound->GetName());
    return 0;
}
