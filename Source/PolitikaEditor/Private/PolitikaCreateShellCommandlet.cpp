#include "PolitikaCreateShellCommandlet.h"

#include "Editor.h"
#include "Engine/World.h"
#include "FileHelpers.h"
#include "GameFramework/PlayerStart.h"
#include "GameFramework/WorldSettings.h"
#include "HAL/FileManager.h"
#include "Misc/PackageName.h"
#include "Misc/Paths.h"
#include "PolitikaBootstrap.h"
#include "PolitikaPrototypeGameplay.h"
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
    return 0;
}
