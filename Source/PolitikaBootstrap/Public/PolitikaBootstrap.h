#pragma once

#include "CoreMinimal.h"

POLITIKABOOTSTRAP_API DECLARE_LOG_CATEGORY_EXTERN(LogPolitikaStartup, Log, All);
POLITIKABOOTSTRAP_API DECLARE_LOG_CATEGORY_EXTERN(LogPolitikaSettings, Log, All);
POLITIKABOOTSTRAP_API DECLARE_LOG_CATEGORY_EXTERN(LogPolitikaPlatform, Log, All);

/** Only project-owned options are parsed here. UE retains ownership of its flags and map URL. */
struct POLITIKABOOTSTRAP_API FPolitikaLaunchOptions
{
    bool bSafeMode = false;
    bool bResetUserConfig = false;
    bool bOffline = true;
    TArray<FString> Errors;

    static FPolitikaLaunchOptions Parse(const TCHAR* CommandLine);
};

struct POLITIKABOOTSTRAP_API FPolitikaStartupIssue
{
    FString Code;
    FText Message;
};

/** Non-UObject startup policy, loaded at PostConfigInit before resolution/scalability preload. */
class POLITIKABOOTSTRAP_API FPolitikaBootstrap
{
public:
    static FPolitikaBootstrap& Get();
    void Initialize();
    const FPolitikaLaunchOptions& Options() const { return LaunchOptions; }
    const TArray<FPolitikaStartupIssue>& Issues() const { return StartupIssues; }
    const FString& RunId() const { return CorrelationId; }
    const FString& GameVersion() const { return Version; }
    bool UsesTransientSettings() const { return bTransientSettings; }
    bool IsApplicationRun() const { return bApplicationRun; }
    void AddIssue(const TCHAR* Code, const FText& Message);
    void LogEvent(const TCHAR* Event, const TCHAR* Phase, const TCHAR* Outcome = TEXT("ok"),
        const TCHAR* Code = TEXT("None"), const TCHAR* Route = TEXT("offline")) const;
    void ReportFatal(const TCHAR* Code, const FText& Message, const TCHAR* Phase = TEXT("PostConfigInit")) const;
    /** Backs up exactly the generated GameUserSettings.ini. Never traverses Saved/SaveGames. */
    bool ResetUserConfig();
    /** Redirects writes to a run-specific INI, preserving the original preferences byte-for-byte. */
    bool UseTransientSettings();

private:
    bool SettingsNeedRecovery() const;
    FString UserSettingsFile;
    FString CorrelationId;
    FString Version;
    double StartSeconds = 0;
    bool bInitialized = false;
    bool bApplicationRun = false;
    bool bTransientSettings = false;
    FPolitikaLaunchOptions LaunchOptions;
    TArray<FPolitikaStartupIssue> StartupIssues;
};
