#include "PolitikaBootstrap.h"
#include "PolitikaVersion.generated.h"

#include "Dom/JsonObject.h"
#include "HAL/FileManager.h"
#include "HAL/PlatformProperties.h"
#include "Misc/App.h"
#include "Misc/CommandLine.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/DefaultValueHelper.h"
#include "Misc/EngineVersion.h"
#include "Misc/FileHelper.h"
#include "Misc/OutputDevice.h"
#include "Misc/Paths.h"
#include "Modules/ModuleManager.h"
#include "Policies/CondensedJsonPrintPolicy.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

#define LOCTEXT_NAMESPACE "PolitikaBootstrap"

DEFINE_LOG_CATEGORY(LogPolitikaStartup);
DEFINE_LOG_CATEGORY(LogPolitikaSettings);
DEFINE_LOG_CATEGORY(LogPolitikaPlatform);

#if !UE_BUILD_SHIPPING
namespace
{
    // Development-only switches owned by gameplay modules, which resolve and validate the
    // values. Bootstrap only recognizes them so they do not surface as launch errors.
    const TCHAR* const DevelopmentValuedSwitches[] = {
        TEXT("-PolitikaTestFixture="),
        TEXT("-PolitikaMPRematch="),
        TEXT("-PolitikaMPRematchDelay="),
        TEXT("-PolitikaMPRematchAttempts="),
        TEXT("-PolitikaMPGroundProbe="),
    };
    const TCHAR* const DevelopmentFlagSwitches[] = {
        TEXT("-PolitikaMPProbe"),
    };

    bool IsDevelopmentSwitch(const FString& Token)
    {
        for (const TCHAR* Prefix : DevelopmentValuedSwitches)
        {
            // A valued switch needs a value; an empty one would be a silent no-op.
            if (Token.StartsWith(Prefix, ESearchCase::IgnoreCase) && Token.Len() > FCString::Strlen(Prefix))
            {
                return true;
            }
        }
        for (const TCHAR* Flag : DevelopmentFlagSwitches)
        {
            if (Token.Equals(Flag, ESearchCase::IgnoreCase))
            {
                return true;
            }
        }
        return false;
    }
}
#endif

FPolitikaLaunchOptions FPolitikaLaunchOptions::Parse(const TCHAR* CommandLine)
{
    return Parse(CommandLine, UE_BUILD_SHIPPING ? EPolitikaLaunchSwitchPolicy::Strict : EPolitikaLaunchSwitchPolicy::Development);
}

FPolitikaLaunchOptions FPolitikaLaunchOptions::Parse(const TCHAR* CommandLine, EPolitikaLaunchSwitchPolicy Policy)
{
    FPolitikaLaunchOptions Result;
    FString Token;
    while (FParse::Token(CommandLine, Token, false))
    {
        if (!Token.StartsWith(TEXT("-Politika"), ESearchCase::IgnoreCase) &&
            !Token.StartsWith(TEXT("--Politika"), ESearchCase::IgnoreCase))
        {
            continue;
        }
        if (Token.Equals(TEXT("-PolitikaSafeMode"), ESearchCase::IgnoreCase))
        {
            Result.bSafeMode = true;
        }
        else if (Token.Equals(TEXT("-PolitikaResetUserConfig"), ESearchCase::IgnoreCase))
        {
            Result.bResetUserConfig = true;
        }
#if !UE_BUILD_SHIPPING
        else if (Policy == EPolitikaLaunchSwitchPolicy::Development && IsDevelopmentSwitch(Token))
        {
            // Recognized only; the owning gameplay module consumes it.
        }
#endif
        else if (!Token.Equals(TEXT("-PolitikaOffline"), ESearchCase::IgnoreCase))
        {
            // Do not echo unknown arguments: they may contain sensitive user-supplied values.
            Result.Errors.AddUnique(TEXT("Unknown or valued POLITIKA switch. Use -PolitikaSafeMode, -PolitikaResetUserConfig or -PolitikaOffline without '=value'."));
        }
    }
    if (Result.bSafeMode && Result.bResetUserConfig)
    {
        Result.Errors.Add(TEXT("Safe mode and config reset are mutually exclusive. Safe mode was selected; no settings were reset."));
        Result.bResetUserConfig = false;
    }
    if (!Result.Errors.IsEmpty())
    {
        // A typo must never partially execute a destructive request.
        Result.bResetUserConfig = false;
    }
    return Result;
}

FPolitikaBootstrap& FPolitikaBootstrap::Get()
{
    static FPolitikaBootstrap Instance;
    return Instance;
}

int64 FPolitikaBootstrap::NetworkProtocol() const
{
    return politika::version::NETWORK_PROTOCOL;
}

void FPolitikaBootstrap::Initialize()
{
    if (bInitialized)
    {
        return;
    }
    bInitialized = true;
    StartSeconds = FPlatformTime::Seconds();
    CorrelationId = FGuid::NewGuid().ToString(EGuidFormats::Digits);
    Version = UTF8_TO_TCHAR(politika::version::GAME_VERSION);
    LaunchOptions = FPolitikaLaunchOptions::Parse(FCommandLine::Get());

    // Never reset an editor user's settings while compiling/cooking/generating maps.
    bApplicationRun = !IsRunningCommandlet() && !IsRunningDedicatedServer();
#if WITH_EDITOR
    bApplicationRun &= FParse::Param(FCommandLine::Get(), TEXT("game"));
#endif
    LogEvent(TEXT("bootstrap"), TEXT("PostConfigInit"));
    for (const FString& Error : LaunchOptions.Errors)
    {
        AddIssue(TEXT("InvalidLaunchOptions"), FText::FromString(Error));
    }
    if (!bApplicationRun)
    {
        return;
    }

    // Restrict reset to the engine-selected, generated user settings file, not arbitrary paths.
    UserSettingsFile = FPaths::Combine(FPaths::GeneratedConfigDir(),
        ANSI_TO_TCHAR(FPlatformProperties::PlatformName()), TEXT("GameUserSettings.ini"));
    FPaths::MakeStandardFilename(UserSettingsFile);
    FString Override;
    if (FParse::Value(FCommandLine::Get(), TEXT("GameUserSettingsINI="), Override))
    {
        if (LaunchOptions.bSafeMode || LaunchOptions.bResetUserConfig)
        {
            // A cooked build may dirty the loaded override branch even before GameInstance.
            // Preserve it during orderly fatal shutdown, not just the ordinary settings file.
            if (FConfigFile* LoadedSettings = GConfig->FindConfigFile(GGameUserSettingsIni))
            {
                LoadedSettings->NoSave = true;
            }
            ReportFatal(TEXT("ConflictingSettingsOverride"), LOCTEXT("IniConflict",
                "Remove -GameUserSettingsINI when using POLITIKA safe mode or config reset. No user settings were changed."));
            return;
        }
        // UE supports this development override. It is not an authorized reset destination.
        AddIssue(TEXT("ExternalSettingsOverride"), LOCTEXT("ExternalIni",
            "An engine settings-file override is active. Shell safe mode/reset is disabled for this run; restart without that override to recover settings."));
        UserSettingsFile.Reset();
        return;
    }

    if (LaunchOptions.bResetUserConfig && !ResetUserConfig())
    {
        AddIssue(TEXT("SettingsBackupFailed"), LOCTEXT("BackupFailed",
            "Settings could not be backed up. The original was preserved. Continue with temporary defaults, fix storage permissions, then retry reset."));
        if (!UseTransientSettings())
        {
            return;
        }
    }
    if (LaunchOptions.bSafeMode || SettingsNeedRecovery())
    {
        if (!LaunchOptions.bSafeMode)
        {
            AddIssue(TEXT("InvalidUserSettings"), LOCTEXT("InvalidSettings",
                "User display settings are invalid or from an unsupported version. Temporary defaults are active; the original settings are preserved. You can back up and reset them."));
        }
        UseTransientSettings();
    }
}

bool FPolitikaBootstrap::SettingsNeedRecovery() const
{
    IFileManager& Files = IFileManager::Get();
    if (UserSettingsFile.IsEmpty() || !Files.FileExists(*UserSettingsFile))
    {
        return false;
    }
    // UE owns INI parsing. Check the shell's known fields; preserve unknown future keys.
    FConfigFile Config;
    Config.Read(UserSettingsFile);
    const TCHAR* Section = TEXT("/Script/Politika.PolitikaUserSettings");
    // A diff-only generated INI may legitimately omit our section when all device
    // settings equal shipped defaults. Only an entirely unparseable file is invalid here.
    if (Config.IsEmpty())
    {
        return true;
    }
    struct FIntRange { const TCHAR* Key; int32 Min; int32 Max; };
    const FIntRange Ranges[] = {
        {TEXT("Version"), 5, 5}, {TEXT("ResolutionSizeX"), 0, 16384},
        {TEXT("ResolutionSizeY"), 0, 16384}, {TEXT("FullscreenMode"), 0, 2},
        {TEXT("PreferredFullscreenMode"), 0, 2}
    };
    for (const FIntRange& Range : Ranges)
    {
        FString Raw;
        if (Config.GetString(Section, Range.Key, Raw))
        {
            int32 Value = 0;
            if (!FDefaultValueHelper::ParseInt(Raw, Value) || Value < Range.Min || Value > Range.Max)
            {
                return true;
            }
        }
    }
    FString Rate;
    double Value = 0;
    if (Config.GetString(Section, TEXT("FrameRateLimit"), Rate) &&
        (!FDefaultValueHelper::ParseDouble(Rate, Value) || !FMath::IsFinite(Value) || Value < 0 || Value > 1000))
    {
        return true;
    }
    for (const TCHAR* Key : {TEXT("bUseVSync"), TEXT("bUseDynamicResolution"), TEXT("bUseHDRDisplayOutput")})
    {
        FString Raw;
        if (Config.GetString(Section, Key, Raw))
        {
            bool ValueBool = false;
            if (!FDefaultValueHelper::ParseBool(Raw, ValueBool)) { return true; }
        }
    }
    return false;
}

bool FPolitikaBootstrap::ResetUserConfig()
{
    if (!bApplicationRun || UserSettingsFile.IsEmpty())
    {
        return false;
    }
    IFileManager& Files = IFileManager::Get();
    if (!Files.FileExists(*UserSettingsFile))
    {
        LogEvent(TEXT("settings_reset"), TEXT("LoadLocalSettings"), TEXT("no_file"));
        return true;
    }
    const FString Backup = UserSettingsFile + TEXT(".backup-") + FDateTime::UtcNow().ToString(TEXT("%Y%m%dT%H%M%S"))
        + TEXT("-") + FGuid::NewGuid().ToString(EGuidFormats::Digits);
    // Copy, verify bytes, then remove only this one source. Failed backup never deletes source.
    if (Files.Copy(*Backup, *UserSettingsFile, false) != COPY_OK)
    {
        return false;
    }
    TArray<uint8> OriginalBytes, BackupBytes;
    if (!FFileHelper::LoadFileToArray(OriginalBytes, *UserSettingsFile) ||
        !FFileHelper::LoadFileToArray(BackupBytes, *Backup) || OriginalBytes != BackupBytes ||
        !Files.Delete(*UserSettingsFile, true, false))
    {
        return false;
    }
    if (!bTransientSettings)
    {
        // UE 5.8 keeps known INIs in named branches; removing a filename would leave the
        // old cached settings alive. Reload the hierarchy after removing the user layer.
        FConfigCacheIni::LoadGlobalIniFile(GGameUserSettingsIni, TEXT("GameUserSettings"), nullptr, true);
    }
    UE_LOG(LogPolitikaSettings, Display, TEXT("User settings backed up and reset; profile and game saves are outside the reset allowlist."));
    LogEvent(TEXT("settings_reset"), TEXT("LoadLocalSettings"), TEXT("backed_up"));
    return true;
}

bool FPolitikaBootstrap::UseTransientSettings()
{
    if (bTransientSettings)
    {
        return true;
    }
    if (!bApplicationRun || UserSettingsFile.IsEmpty())
    {
        return false;
    }
    const FString TempIni = FPaths::ConvertRelativePathToFull(FPaths::Combine(
        FPaths::ProjectSavedDir(), TEXT("ShellSessions"), CorrelationId, TEXT("GameUserSettings.ini")));
    if (!IFileManager::Get().MakeDirectory(*FPaths::GetPath(TempIni), true))
    {
        ReportFatal(TEXT("TemporarySettingsUnavailable"), LOCTEXT("TempFailed",
            "Safe defaults could not be prepared. Check free space and write access to the game's Saved directory."));
        return false;
    }
    // A menu-triggered recovery can happen after UE has dirtied the original cached branch.
    // Freeze that branch as well: changing the destination alone would not prevent a later flush.
    if (FConfigFile* Original = GConfig->FindConfigFile(GGameUserSettingsIni))
    {
        Original->NoSave = true;
    }
    bTransientSettings = true;
    // Redirect before PreloadResolutionSettings and scalability loading. UE's own INI hierarchy
    // provides shipped defaults; no copy of the suspect preferences goes into this directory.
    FCommandLine::Append(*FString::Printf(TEXT(" -GameUserSettingsINI=\"%s\""), *TempIni));
    // UE 5.8 initializes this global to the known-branch name "GameUserSettings", not a
    // filename. Change it explicitly, otherwise a force reload can keep resolving the original.
    GGameUserSettingsIni = TempIni;
    if (!FConfigCacheIni::LoadGlobalIniFile(GGameUserSettingsIni, TEXT("GameUserSettings"), nullptr, true))
    {
        ReportFatal(TEXT("TemporarySettingsLoadFailed"), LOCTEXT("TempLoadFailed",
            "The temporary settings hierarchy could not be loaded. Restore the shipped config and check storage access."));
        return false;
    }
    LogEvent(TEXT("settings_overlay"), TEXT("LoadLocalSettings"), TEXT("temporary_defaults"));
    return true;
}

void FPolitikaBootstrap::AddIssue(const TCHAR* Code, const FText& Message)
{
    StartupIssues.Add({Code, Message});
    LogEvent(TEXT("recovery_required"), TEXT("PostConfigInit"), TEXT("recoverable"), Code);
}

void FPolitikaBootstrap::LogEvent(const TCHAR* Event, const TCHAR* Phase, const TCHAR* Outcome,
    const TCHAR* Code, const TCHAR* Route) const
{
    TSharedRef<FJsonObject> Record = MakeShared<FJsonObject>();
    Record->SetStringField(TEXT("event"), Event);
    Record->SetStringField(TEXT("run"), CorrelationId);
    Record->SetStringField(TEXT("phase"), Phase);
    Record->SetNumberField(TEXT("elapsed_ms"), (FPlatformTime::Seconds() - StartSeconds) * 1000.0);
    Record->SetStringField(TEXT("outcome"), Outcome);
    Record->SetStringField(TEXT("code"), Code);
    Record->SetStringField(TEXT("route"), Route);
    Record->SetStringField(TEXT("game"), Version);
    Record->SetStringField(TEXT("engine"), FEngineVersion::Current().ToString());
    Record->SetStringField(TEXT("build"), UTF8_TO_TCHAR(politika::version::BUILD_NUMBER));
    Record->SetStringField(TEXT("git_revision"), UTF8_TO_TCHAR(politika::version::GIT_REVISION));
    Record->SetNumberField(TEXT("save_schema"), politika::version::SAVE_SCHEMA);
    Record->SetBoolField(TEXT("safe_mode"), LaunchOptions.bSafeMode);
    Record->SetBoolField(TEXT("reset_requested"), LaunchOptions.bResetUserConfig);
    Record->SetBoolField(TEXT("temporary_settings"), bTransientSettings);
    FString Json;
    const auto Writer = TJsonWriterFactory<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>::Create(&Json);
    FJsonSerializer::Serialize(Record, Writer);
    UE_LOG(LogPolitikaStartup, Display, TEXT("%s"), *Json);
}

void FPolitikaBootstrap::ReportFatal(const TCHAR* Code, const FText& Message, const TCHAR* Phase) const
{
    LogEvent(TEXT("startup_fatal"), Phase, TEXT("fatal"), Code);
    UE_LOG(LogPolitikaStartup, Error, TEXT("%s"), *Message.ToString());
    if (GLog)
    {
        GLog->Flush();
    }
    if (!FApp::IsUnattended() && !IsRunningCommandlet() && !IsRunningDedicatedServer())
    {
        FPlatformMisc::MessageBoxExt(EAppMsgType::Ok, *Message.ToString(), TEXT("POLITIKA startup failure"));
    }
    FPlatformMisc::RequestExitWithStatus(false, 70);
}

class FPolitikaBootstrapModule final : public IModuleInterface
{
    virtual void StartupModule() override { FPolitikaBootstrap::Get().Initialize(); }
    virtual void ShutdownModule() override
    {
        FPolitikaBootstrap::Get().LogEvent(TEXT("shutdown"), TEXT("Shutdown"));
        if (GLog) { GLog->Flush(); }
    }
};

IMPLEMENT_MODULE(FPolitikaBootstrapModule, PolitikaBootstrap)

#undef LOCTEXT_NAMESPACE
