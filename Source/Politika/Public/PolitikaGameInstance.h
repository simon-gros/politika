#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "PolitikaBootstrap.h"
#include "PolitikaGameInstance.generated.h"

class UPolitikaLocalProfile;

UENUM()
enum class EPolitikaBootPhase : uint8
{
    EngineReady, ParseLaunchOptions, LoadLocalSettings, ResolveLocalProfile, SelectStartupRoute, Ready, Shutdown
};

/** Game-instance lifetime (one per PIE instance), not a global owner of match state. */
UCLASS()
class POLITIKA_API UPolitikaGameInstance final : public UGameInstance
{
    GENERATED_BODY()
public:
    virtual void Init() override;
    virtual void Shutdown() override;
    virtual ULocalPlayer* CreateInitialPlayer(FString& OutError) override;

    void RetryProfile();
    void ContinueOffline();
    void EnableSessionSafeMode();
    void ResetSettings();
    void OpenOfflineSandbox(APlayerController* Player);
    void OpenPrototypeSlice(APlayerController* Player);
    void ReturnToMenu(APlayerController* Player);
    const TArray<FPolitikaStartupIssue>& Issues() const { return RecoveryIssues; }
    bool HasPersistentProfile() const { return bPersistentProfile; }
    FString PhaseName() const;
    FText Diagnostics() const;

private:
    void SetPhase(EPolitikaBootPhase Next);
    void ResolveProfile();
    void AddIssue(const TCHAR* Code, const FText& Message);
    void HandleTravelFailure(UWorld* World, ETravelFailure::Type FailureType, const FString& Error);
    void Travel(APlayerController* Player, const TCHAR* Map, const TCHAR* Route);

    UPROPERTY(Transient)
    TObjectPtr<UPolitikaLocalProfile> Profile;

    EPolitikaBootPhase Phase = EPolitikaBootPhase::EngineReady;
    FString LastCompletedPhase = TEXT("None");
    bool bPhaseEntered = false;
    bool bBootStarted = false;
    bool bPersistentProfile = false;
    TArray<FPolitikaStartupIssue> RecoveryIssues;
    FDelegateHandle TravelFailureHandle;
};
