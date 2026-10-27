#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/HUD.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "PolitikaPrototypeGameplay.generated.h"

struct FPolitikaPrototypeContentSnapshot;

class UCameraComponent;
class UStaticMeshComponent;
class APolitikaPrototypeUnit;
class APolitikaPrototypeObjective;

UENUM(BlueprintType)
enum class EPolitikaPrototypeOrder : uint8 { Idle, Move, Attack, AttackMove, Stop, Guard };

UCLASS()
class POLITIKA_API APolitikaPrototypeUnit final : public AActor
{
    GENERATED_BODY()
public:
    APolitikaPrototypeUnit();
    void Configure(uint64 InStableId, uint32 InOwner, const FText& InName, int32 InHealth,
        float InMoveSpeed, float InWeaponRange, float InDamage, float InCooldown,
        bool bInAttackable, UStaticMesh* InMesh, const FVector& InMeshScale, const FLinearColor& InColor);
    void SetMoveOrder(EPolitikaPrototypeOrder Kind, const FVector& Destination);
    void SetAttackOrder(APolitikaPrototypeUnit* Target);
    void SetGuardOrder();
    void StopOrder();
    void SetSelectedVisual(bool bSelected);
    // Development-only acceptance fixtures; never used by the normal scenario.
#if !UE_BUILD_SHIPPING
    void SetAcceptanceHealth(int32 InHealth) { Health = FMath::Clamp(InHealth, 1, MaxHealth); }
#endif
    void SimulateMovement(float DeltaSeconds);
    void ResolveCombat(float DeltaSeconds);
    void ApplyPendingDamage();
    void CleanupDeadTarget();
    uint64 GetStableId() const { return StableId; }
    uint32 GetOwnerPlayer() const { return OwnerPlayer; }
    int32 GetHealth() const { return Health; }
    int32 GetMaxHealth() const { return MaxHealth; }
    bool IsAlive() const { return Health > 0; }
    bool IsAttackable() const { return bAttackable; }
    EPolitikaPrototypeOrder GetCurrentOrder() const { return CurrentOrder; }
    FText GetUnitName() const { return UnitName; }
    APolitikaPrototypeUnit* GetOrderTarget() const { return OrderTarget.Get(); }
private:
    UPROPERTY(VisibleAnywhere) TObjectPtr<UStaticMeshComponent> Body;
    UPROPERTY(VisibleAnywhere) TObjectPtr<UStaticMeshComponent> SelectionIndicator;
    UPROPERTY() FText UnitName;
    UPROPERTY() int32 Health = 0;
    UPROPERTY() int32 MaxHealth = 0;
    UPROPERTY() uint64 StableId = 0;
    UPROPERTY() uint32 OwnerPlayer = 0;
    UPROPERTY() float MoveSpeed = 0.0f;
    UPROPERTY() float WeaponRange = 0.0f;
    UPROPERTY() float WeaponDamage = 0.0f;
    UPROPERTY() float WeaponCooldown = 0.0f;
    UPROPERTY() float CooldownRemaining = 0.0f;
    UPROPERTY() float PendingDamage = 0.0f;
    UPROPERTY() bool bAttackable = false;
    UPROPERTY() EPolitikaPrototypeOrder CurrentOrder = EPolitikaPrototypeOrder::Idle;
    UPROPERTY() FVector OrderDestination = FVector::ZeroVector;
    UPROPERTY() TWeakObjectPtr<APolitikaPrototypeUnit> OrderTarget;
    void AutoAcquireForAttackMove();
};

UCLASS()
class POLITIKA_API APolitikaPrototypeObjective final : public AActor
{
    GENERATED_BODY()
public:
    APolitikaPrototypeObjective();
    void ConfigureFromDefinition(const struct FPolitikaPrototypeObjectiveDefinition& Definition, uint32 LocalPlayerId);
    void UpdateCapture(const TArray<APolitikaPrototypeUnit*>& Units, float DeltaSeconds);
    float GetProgress() const { return Progress; }
    bool IsContested() const { return bContested; }
    bool IsComplete() const { return bComplete; }
    FText GetStatusText() const;
private:
    UPROPERTY(VisibleAnywhere) TObjectPtr<UStaticMeshComponent> Marker;
    UPROPERTY() float CaptureRadius = 0.0f;
    UPROPERTY() float CaptureDuration = 0.0f;
    UPROPERTY() uint32 LocalRuntimePlayerId = 0;
    UPROPERTY() float Progress = 0.0f;
    UPROPERTY() bool bContested = false;
    UPROPERTY() bool bComplete = false;
};

UCLASS()
class POLITIKA_API APolitikaPrototypeMatch final : public AActor
{
    GENERATED_BODY()
public:
    APolitikaPrototypeMatch();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    const TArray<TObjectPtr<APolitikaPrototypeUnit>>& GetUnits() const { return Units; }
    APolitikaPrototypeObjective* GetObjective() const { return Objective; }
    APolitikaPrototypeUnit* GetSelectedUnitForPlayer(uint32 PlayerId) const;
    void SetSelection(uint32 PlayerId, APolitikaPrototypeUnit* Unit);
    void SubmitMove(uint32 PlayerId, APolitikaPrototypeUnit* Unit, const FVector& Destination, bool bAttackMove = false);
    void SubmitAttack(uint32 PlayerId, APolitikaPrototypeUnit* Unit, APolitikaPrototypeUnit* Target);
    void SubmitStop(uint32 PlayerId, APolitikaPrototypeUnit* Unit);
    void SubmitGuard(uint32 PlayerId, APolitikaPrototypeUnit* Unit);
    void ReportRejectedCommand(const FString& Reason) { LastCommandFeedback = Reason; }
    FString GetLastCommandFeedback() const { return LastCommandFeedback; }
    FVector2D GetPlayableMin() const { return PlayableMin; }
    FVector2D GetPlayableMax() const { return PlayableMax; }
    FVector2D GetCameraPanMin() const;
    FVector2D GetCameraPanMax() const;
    uint32 GetLocalRuntimePlayerId() const { return LocalRuntimePlayerId; }
    const class UPolitikaPrototypeMapDefinition* GetMapDefinition() const { return MapDefinition; }
private:
    struct FPendingOrder
    {
        uint32 PlayerId = 0;
        TWeakObjectPtr<APolitikaPrototypeUnit> Unit;
        TWeakObjectPtr<APolitikaPrototypeUnit> Target;
        FVector Destination = FVector::ZeroVector;
        EPolitikaPrototypeOrder Kind = EPolitikaPrototypeOrder::Idle;
    };
    UPROPERTY() TArray<TObjectPtr<APolitikaPrototypeUnit>> Units;
    UPROPERTY() TObjectPtr<APolitikaPrototypeObjective> Objective;
    UPROPERTY() TMap<uint32, TWeakObjectPtr<APolitikaPrototypeUnit>> SelectionByPlayer;
    UPROPERTY() FString LastCommandFeedback = TEXT("Select a friendly unit; right-click ground or an enemy to issue an order.");
    UPROPERTY() float SimulationStepSeconds = 0.05f;
    UPROPERTY() FVector2D PlayableMin = FVector2D::ZeroVector;
    UPROPERTY() FVector2D PlayableMax = FVector2D::ZeroVector;
    UPROPERTY() uint32 LocalRuntimePlayerId = 0;
    UPROPERTY() bool bContentReady = false;
    UPROPERTY() TObjectPtr<class UPolitikaPrototypeContentCatalog> ContentCatalog;
    UPROPERTY() TObjectPtr<class UPolitikaPrototypeMapDefinition> MapDefinition;
    UPROPERTY() float StepAccumulator = 0.0f;
    TArray<FPendingOrder> PendingOrders;
    uint64 NextStableId = 1;
    void BuildScenario();
    void AdmitPendingOrders();
    void RunSimulationStep(float DeltaSeconds);
    bool ValidateOrder(const FPendingOrder& Pending, FString& OutReason) const;
#if !UE_BUILD_SHIPPING
    enum class EAcceptancePhase : uint8 { None, CaptureApproach, CaptureHold, CaptureExit, CapturePause, CombatSetup, CombatResolve, Done };
    EAcceptancePhase AcceptancePhase = EAcceptancePhase::None;
    FString AcceptanceMode;
    float AcceptanceElapsed = 0.0f;
    float AcceptanceProgressAtPause = 0.0f;
    bool bAcceptanceAttackerOrderCleared = false;
    TWeakObjectPtr<APolitikaPrototypeUnit> AcceptanceAttacker;
    TWeakObjectPtr<APolitikaPrototypeUnit> AcceptanceDefender;
    void BeginAcceptanceHarness();
    void TickAcceptanceHarness(float DeltaSeconds);
    void StartAttackerDeathFixture();
#endif
};

UCLASS()
class POLITIKA_API APolitikaPrototypeCameraPawn final : public APawn
{
    GENERATED_BODY()
public:
    APolitikaPrototypeCameraPawn();
    UCameraComponent* GetCamera() const { return Camera; }
private:
    UPROPERTY(VisibleAnywhere) TObjectPtr<USceneComponent> SceneRoot;
    UPROPERTY(VisibleAnywhere) TObjectPtr<UCameraComponent> Camera;
};

UCLASS()
class POLITIKA_API APolitikaPrototypePlayerController final : public APlayerController
{
    GENERATED_BODY()
protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    virtual void SetupInputComponent() override;
private:
    UPROPERTY() TWeakObjectPtr<APolitikaPrototypeMatch> Match;
    UPROPERTY() TWeakObjectPtr<APolitikaPrototypeUnit> SelectedUnit;
    bool bMapCameraApplied = false;
    void SelectUnderCursor();
    void OrderUnderCursor();
    void StopSelected();
    void GuardSelected();
    void AttackMoveSelected();
    void ReturnToMenu();
    void PanForward(float Value);
    void PanRight(float Value);
    void Zoom(float Value);
    void ApplyMapCameraIfReady();
    APolitikaPrototypeMatch* ResolveMatch();
    uint32 GetCommandPlayerId() const { return Match.IsValid() ? Match->GetLocalRuntimePlayerId() : 0; }
};

UCLASS()
class POLITIKA_API APolitikaPrototypeHUD final : public AHUD
{
    GENERATED_BODY()
public:
    virtual void DrawHUD() override;
};

UCLASS()
class POLITIKA_API APolitikaPrototypeGameMode final : public AGameModeBase
{
    GENERATED_BODY()
public:
    APolitikaPrototypeGameMode();
    virtual void StartPlay() override;
};
