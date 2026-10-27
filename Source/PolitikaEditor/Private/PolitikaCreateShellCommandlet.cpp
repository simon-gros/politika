#include "PolitikaCreateShellCommandlet.h"

#include "Editor.h"
#include "Engine/World.h"
#include "FileHelpers.h"
#include "GameFramework/WorldSettings.h"
#include "HAL/FileManager.h"
#include "Misc/PackageName.h"
#include "Misc/Paths.h"
#include "PolitikaBootstrap.h"
#include "PolitikaShellGameMode.h"

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
        UE_LOG(LogPolitikaStartup, Error, TEXT("An editor build is required to create shell maps."));
        return 1;
    }
    struct FShellMap { const TCHAR* Package; UClass* GameMode; };
    const FShellMap Maps[] = {
        {TEXT("/Game/Maps/MainMenu"), APolitikaShellGameMode::StaticClass()},
        {TEXT("/Game/Maps/OfflineSandbox"), APolitikaOfflineGameMode::StaticClass()}
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
        if (!FEditorFileUtils::SaveMap(World, Filename)) { return 4; }
        UE_LOG(LogPolitikaStartup, Display, TEXT("Created shell map %s"), Map.Package);
    }
    return 0;
}
