#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Engine/TimerHandle.h"
#include "PolitikaBootstrap.h"
#include "PolitikaGameInstance.generated.h"

class UPolitikaLocalProfile;
class UNetDriver;

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
    void ReportMatchAdmissionRejected(FName MapId);
    bool ConsumeDevelopmentTestFixture(FString& OutFixtureId);
    // Network session recovery. Observed state is recorded so a later connection loss
    // can be explained truthfully (a killed host cannot send a terminal event).
    void NoteNetworkMatchObserved(uint64 Generation, uint8 Phase, bool bResultCommitted, uint32 Winner,
        FName Reason, uint32 LocalParticipant);
    void NoteNetworkKick(const FText& Reason);
    void ReportNetworkSessionEnded(const TCHAR* Code, const FText& Message);
    // Called when the shell menu is ready; drives the development-only rematch hook.
    void OnShellReady(APlayerController* Player);
    const TArray<FPolitikaStartupIssue>& Issues() const { return RecoveryIssues; }
    bool HasPersistentProfile() const { return bPersistentProfile; }
    FString PhaseName() const;
    FText Diagnostics() const;

    // Fog of war is enforced through generic-replication actor relevancy
    // (APolitikaPrototypeUnit::IsNetRelevantFor); Iris would bypass that filter.
    virtual EReplicationSystem GetDesiredReplicationSystem(FName InNetDriverDefinition) const override;

protected:
    virtual void OnStart() override;

private:
    void HandleNetworkFailure(UWorld* World, UNetDriver* NetDriver, ENetworkFailure::Type FailureType,
        const FString& ErrorString);
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
    bool bDevelopmentTestFixtureConsumed = false;
    TArray<FPolitikaStartupIssue> RecoveryIssues;
    FDelegateHandle TravelFailureHandle;
    FDelegateHandle NetworkFailureHandle;
    struct FObservedNetworkMatch
    {
        uint64 Generation = 0;
        uint8 Phase = 0;
        bool bResultCommitted = false;
        uint32 Winner = 0;
        FName Reason;
        uint32 LocalParticipant = 0;
    } ObservedNetworkMatch;
    FText PendingKickReason;
    bool bNetworkSessionEnded = false;
#if !UE_BUILD_SHIPPING
    // Development-only same-process rematch: travel back to a session URL after a
    // networked match ends, so packaged tests can verify fresh match identities.
    FString DevelopmentRematchUrl;
    float DevelopmentRematchDelaySeconds = 6.0f;
    int32 DevelopmentRematchTravels = 0;
    FTimerHandle DevelopmentRematchTimer;
#endif
};
