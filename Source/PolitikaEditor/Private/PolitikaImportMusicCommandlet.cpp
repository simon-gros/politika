#include "PolitikaImportMusicCommandlet.h"

#include "AssetImportTask.h"
#include "AssetToolsModule.h"
#include "Factories/SoundFactory.h"
#include "HAL/FileManager.h"
#include "Misc/PackageName.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"
#include "PolitikaBootstrap.h"
#include "Sound/SoundWave.h"
#include "UObject/SavePackage.h"

UPolitikaImportMusicCommandlet::UPolitikaImportMusicCommandlet()
{
    IsClient = false;
    IsEditor = true;
    LogToConsole = true;
}

int32 UPolitikaImportMusicCommandlet::Main(const FString& Params)
{
    const TCHAR* AssetPath = TEXT("/Game/Politika/Audio/Music/SW_Dusk.SW_Dusk");
    USoundWave* Music = LoadObject<USoundWave>(nullptr, AssetPath, nullptr, LOAD_NoWarn);
    if (!Music)
    {
        FString Source;
        if (!FParse::Value(*Params, TEXT("Source="), Source) || !FPaths::FileExists(Source)) return 1;
        auto* Task = NewObject<UAssetImportTask>();
        Task->Filename = Source;
        Task->DestinationPath = TEXT("/Game/Politika/Audio/Music");
        Task->DestinationName = TEXT("SW_Dusk");
        Task->Factory = NewObject<USoundFactory>();
        Task->bAutomated = true;
        Task->bReplaceExisting = false;
        FModuleManager::LoadModuleChecked<FAssetToolsModule>(TEXT("AssetTools")).Get().ImportAssetTasks({Task});
        Music = LoadObject<USoundWave>(nullptr, AssetPath);
    }
    if (!Music) return 2;
    Music->bLooping = true;
    Music->Volume = 1.0f; // Gain is applied once, by the map's audio component.
    Music->VirtualizationMode = EVirtualizationMode::PlayWhenSilent;
    UPackage* Package = Music->GetOutermost();
    const FString File = FPackageName::LongPackageNameToFilename(Package->GetName(), FPackageName::GetAssetPackageExtension());
    IFileManager::Get().MakeDirectory(*FPaths::GetPath(File), true);
    FSavePackageArgs Args;
    Args.TopLevelFlags = RF_Public | RF_Standalone;
    if (!UPackage::SavePackage(Package, Music, *File, Args)) return 3;
    UE_LOG(LogPolitikaStartup, Display, TEXT("TEST_MAP_MUSIC_IMPORTED track=Dusk duration=%.2f looping=%d volume=%.2f"),
        Music->Duration, Music->bLooping, Music->Volume);
    return 0;
}
