#include "PolitikaGameInstance.h"

#include "Engine/Engine.h"
#include "GameFramework/PlayerController.h"
#include "HAL/PlatformOutputDevices.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/EngineVersion.h"
#include "Misc/OutputDevice.h"
#include "Misc/PackageName.h"
#include "Misc/Paths.h"
#include "PolitikaLocalProfile.h"
#include "PolitikaUserSettings.h"

#define LOCTEXT_NAMESPACE "PolitikaGameInstance"

void UPolitikaGameInstance::Init()
{
    if (bBootStarted)
    {
        return;
    }
    bBootStarted = true;
    Super::Init();
    FPolitikaBootstrap& Bootstrap = FPolitikaBootstrap::Get();
    RecoveryIssues = Bootstrap.Issues();
    SetPhase(EPolitikaBootPhase::EngineReady);
    SetPhase(EPolitikaBootPhase::ParseLaunchOptions);
    SetPhase(EPolitikaBootPhase::LoadLocalSettings);
    // The engine loads/applies UPolitikaUserSettings. The early module owns reset/overlay timing.
    if (!IsDedicatedServerInstance() && GEngine && !Cast<UPolitikaUserSettings>(GEngine->GetGameUserSettings()))
    {
        AddIssue(TEXT("SettingsClassMismatch"), LOCTEXT("SettingsClass",
            "The configured user-settings class is unavailable. Restore DefaultEngine.ini before using settings recovery."));
    }
    SetPhase(EPolitikaBootPhase::ResolveLocalProfile);
    if (!IsDedicatedServerInstance())
    {
        ResolveProfile();
    }
    SetPhase(EPolitikaBootPhase::SelectStartupRoute);
    UE_LOG(LogPolitikaPlatform, Display, TEXT("Offline provider selected; no online SDK, login or network request is required."));
    if (GEngine)
    {
        TravelFailureHandle = GEngine->OnTravelFailure().AddUObject(this, &ThisClass::HandleTravelFailure);
    }
    // Ready means local services are ready. PlayerController emits ui_ready after a world exists.
    SetPhase(EPolitikaBootPhase::Ready);
}

void UPolitikaGameInstance::Shutdown()
{
    if (GEngine)
    {
        GEngine->OnTravelFailure().Remove(TravelFailureHandle);
    }
    SetPhase(EPolitikaBootPhase::Shutdown);
    if (GLog) { GLog->Flush(); }
    Super::Shutdown();
}

ULocalPlayer* UPolitikaGameInstance::CreateInitialPlayer(FString& OutError)
{
    return IsDedicatedServerInstance() ? nullptr : Super::CreateInitialPlayer(OutError);
}

void UPolitikaGameInstance::SetPhase(EPolitikaBootPhase Next)
{
    if (bPhaseEntered)
    {
        LastCompletedPhase = PhaseName();
        FPolitikaBootstrap::Get().LogEvent(TEXT("phase_complete"), *LastCompletedPhase,
            RecoveryIssues.IsEmpty() ? TEXT("ok") : TEXT("recovery_available"));
    }
    Phase = Next;
    bPhaseEntered = true;
    FPolitikaBootstrap::Get().LogEvent(TEXT("phase_started"), *PhaseName(),
        TEXT("started"), TEXT("None"),
        IsDedicatedServerInstance() ? TEXT("server") : TEXT("offline"));
}

FString UPolitikaGameInstance::PhaseName() const
{
    return StaticEnum<EPolitikaBootPhase>()->GetNameStringByValue(static_cast<int64>(Phase));
}

void UPolitikaGameInstance::AddIssue(const TCHAR* Code, const FText& Message)
{
    RecoveryIssues.RemoveAll([Code](const FPolitikaStartupIssue& Issue) { return Issue.Code == Code; });
    RecoveryIssues.Add({Code, Message});
    FPolitikaBootstrap::Get().LogEvent(TEXT("recovery_required"), *PhaseName(), TEXT("recoverable"), Code);
}

void UPolitikaGameInstance::ResolveProfile()
{
    bPersistentProfile = false;
    if (UGameplayStatics::DoesSaveGameExist(UPolitikaLocalProfile::Slot, 0))
    {
        Profile = UPolitikaLocalProfile::LoadVerified();
        if (Profile && Profile->IsValidProfile())
        {
            bPersistentProfile = true;
        }
        else
        {
            Profile = nullptr;
            AddIssue(TEXT("ProfileInvalid"), LOCTEXT("ProfileInvalid",
                "The local profile cannot be read or has an unsupported version. It has not been overwritten. Retry after repairing it, or continue offline with a temporary profile."));
        }
    }
    else
    {
        // Never persist PIE/debug session identity into a player's standalone save slot.
        Profile = NewObject<UPolitikaLocalProfile>(this);
        Profile->LocalId = FGuid::NewGuid();
        Profile->CreatedUtc = FDateTime::UtcNow();
        if (!GIsEditor)
        {
            bPersistentProfile = Profile->SaveVerified();
            if (!bPersistentProfile)
            {
                AddIssue(TEXT("ProfileWriteFailed"), LOCTEXT("ProfileWrite",
                    "The local profile could not be saved. Check storage space and permissions, then retry. Offline entry remains available with a temporary profile."));
            }
        }
    }
    FPolitikaBootstrap::Get().LogEvent(TEXT("profile"), *PhaseName(),
        bPersistentProfile ? TEXT("persistent") : TEXT("temporary"));
}

void UPolitikaGameInstance::RetryProfile()
{
    RecoveryIssues.RemoveAll([](const FPolitikaStartupIssue& Issue) { return Issue.Code.StartsWith(TEXT("Profile")); });
    SetPhase(EPolitikaBootPhase::ResolveLocalProfile);
    ResolveProfile();
    SetPhase(EPolitikaBootPhase::Ready);
}

void UPolitikaGameInstance::ContinueOffline()
{
    if (!Profile)
    {
        Profile = NewObject<UPolitikaLocalProfile>(this);
        Profile->LocalId = FGuid::NewGuid();
        Profile->CreatedUtc = FDateTime::UtcNow();
    }
    FPolitikaBootstrap::Get().LogEvent(TEXT("continue_offline"), *PhaseName(),
        bPersistentProfile ? TEXT("persistent_profile") : TEXT("temporary_profile"));
}

void UPolitikaGameInstance::EnableSessionSafeMode()
{
    if (UPolitikaUserSettings* Settings = GEngine ? Cast<UPolitikaUserSettings>(GEngine->GetGameUserSettings()) : nullptr)
    {
        if (FPolitikaBootstrap::Get().UseTransientSettings())
        {
            Settings->ApplySessionDefaults();
            return;
        }
    }
    AddIssue(TEXT("SafeModeUnavailable"), LOCTEXT("SafeUnavailable",
        "Session safe mode is unavailable in this editor or overridden-config run. Launch the game with -PolitikaSafeMode and no settings-file override."));
}

void UPolitikaGameInstance::ResetSettings()
{
    FPolitikaBootstrap& Bootstrap = FPolitikaBootstrap::Get();
    // Redirect first, so engine shutdown cannot write the old in-memory settings back afterwards.
    if (!Bootstrap.UseTransientSettings() || !Bootstrap.ResetUserConfig())
    {
        AddIssue(TEXT("SettingsBackupFailed"), LOCTEXT("ResetFailed",
            "Settings reset could not complete. Existing data has been preserved. Check storage access, or restart without an engine settings-file override."));
        return;
    }
    RecoveryIssues.RemoveAll([](const FPolitikaStartupIssue& Issue)
    {
        return Issue.Code == TEXT("InvalidUserSettings") || Issue.Code == TEXT("SettingsBackupFailed");
    });
    EnableSessionSafeMode();
    AddIssue(TEXT("SettingsResetComplete"), LOCTEXT("ResetComplete",
        "User settings are reset; any existing file was backed up alongside it. Temporary defaults apply now. Restart normally to save new preferences. Profile and game saves were not changed."));
}

void UPolitikaGameInstance::Travel(APlayerController* Player, const TCHAR* Map, const TCHAR* Route)
{
    if (!Player || !Player->IsLocalController() || IsDedicatedServerInstance())
    {
        return;
    }
    if (!FPackageName::DoesPackageExist(Map))
    {
        AddIssue(TEXT("MissingShellMap"), LOCTEXT("MissingMap",
            "A required shell map is missing. Run the PolitikaCreateShell commandlet and rebuild the package. You can remain in the current menu."));
        return;
    }
    FPolitikaBootstrap::Get().LogEvent(TEXT("travel_requested"), *PhaseName(), TEXT("ok"), TEXT("None"), Route);
    UGameplayStatics::OpenLevel(Player, FName(Map), true);
}

void UPolitikaGameInstance::OpenOfflineSandbox(APlayerController* Player)
{
    ContinueOffline();
    Travel(Player, TEXT("/Game/Maps/OfflineSandbox"), TEXT("offline_sandbox"));
}

void UPolitikaGameInstance::OpenPrototypeSlice(APlayerController* Player)
{
    ContinueOffline();
    if (!Player || !Player->IsLocalController() || IsDedicatedServerInstance())
    {
        return;
    }
    constexpr TCHAR PrototypeMap[] = TEXT("/Game/Maps/RTSVerticalSlice");
    if (!FPackageName::DoesPackageExist(PrototypeMap))
    {
        AddIssue(TEXT("MissingPrototypeMap"), LOCTEXT("MissingPrototypeMap",
            "The prototype RTS map is missing. Rebuild the development package."));
        return;
    }
    FPolitikaBootstrap::Get().LogEvent(TEXT("travel_requested"), *PhaseName(), TEXT("ok"), TEXT("None"), TEXT("rts_vertical_slice"));
    UGameplayStatics::OpenLevel(Player, FName(PrototypeMap), true);
}

void UPolitikaGameInstance::ReturnToMenu(APlayerController* Player)
{
    Travel(Player, TEXT("/Game/Maps/MainMenu"), TEXT("main_menu"));
}

void UPolitikaGameInstance::HandleTravelFailure(UWorld* World, ETravelFailure::Type FailureType, const FString& Error)
{
    AddIssue(TEXT("TravelFailed"), LOCTEXT("TravelFailed",
        "The requested map could not be opened. Check the packaged maps and engine log. The current shell remains available where possible."));
    if (!World || !World->HasBegunPlay())
    {
        FPolitikaBootstrap::Get().ReportFatal(TEXT("StartupMapUnavailable"), LOCTEXT("NoStartupMap",
            "No startup world is available. Check GameDefaultMap, generate the shell maps, and rebuild the package. See the engine log for details."), *PhaseName());
    }
}

FText UPolitikaGameInstance::Diagnostics() const
{
    const FPolitikaBootstrap& Bootstrap = FPolitikaBootstrap::Get();
    return FText::Format(LOCTEXT("Diagnostics", "Phase: {0} · Game: {1} · UE: {2}\nRun: {3}\nLog: {4}\nLast completed phase: {5}"),
        FText::FromString(PhaseName()), FText::FromString(Bootstrap.GameVersion()),
        FText::FromString(FEngineVersion::Current().ToString()), FText::FromString(Bootstrap.RunId()),
        FText::FromString(FPlatformOutputDevices::GetAbsoluteLogFilename()), FText::FromString(LastCompletedPhase));
}

#undef LOCTEXT_NAMESPACE
