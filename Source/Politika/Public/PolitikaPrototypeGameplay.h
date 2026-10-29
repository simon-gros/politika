#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/HUD.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Rules.h"
#include "PolitikaPrototypeContent.h"
#include "PolitikaPrototypeGameplay.generated.h"

struct FPolitikaPrototypeContentSnapshot;
struct FPathFindingResult;

class UCameraComponent;
class UStaticMeshComponent;
class APolitikaPrototypeUnit;
class APolitikaPrototypeObjective;
class UPolitikaInfantryPresentationComponent;

// Native observers receive immutable, authoritative simulation events after
// the complete fixed-step update. Subscribers must copy the payload if they
// need to retain it beyond the callback.
DECLARE_MULTICAST_DELEGATE_OneParam(FPolitikaPrototypeGameplayEventPublished,
    const politika::rules::GameplayEvent& /* Event */);

struct FPolitikaPrototypeMatchResult
{
	uint32 WinningPlayerId = 0;
	FName Reason;
};

enum class EPolitikaPrototypeControllerKind : uint8 { LocalHuman, AssignedOpponent };

struct FPolitikaPrototypeRuntimeParticipant
{
    int32 SlotId = 0;
    uint32 RuntimePlayerId = 0;
    FName FactionId;
    EPolitikaPrototypeControllerKind ControllerKind = EPolitikaPrototypeControllerKind::AssignedOpponent;
};

struct FPolitikaPrototypeSystemFactionAuthority
{
    FName FactionId;
    bool bNpcSystemControlled = false;
};

UENUM(BlueprintType)
enum class EPolitikaPrototypeOrder : uint8 { Idle, Move, Attack, AttackMove, Stop, Guard, Grenade, Reload, Sprint };

UCLASS()
class POLITIKA_API APolitikaPrototypeUnit final : public AActor
{
    GENERATED_BODY()
public:
    APolitikaPrototypeUnit();
    void Configure(uint64 InStableId, uint32 InOwner, const FText& InName, int32 InHealth,
        float InMoveSpeed, float InWeaponRange, float InDamage, float InCooldown,
        bool bInAttackable, UStaticMesh* InMesh, const FVector& InMeshScale, const FLinearColor& InColor,
        FName InFactionId = NAME_None, bool bInUseAuthoredMeshMaterials = false);
    void SetMoveOrder(EPolitikaPrototypeOrder Kind, const FVector& Destination,
        const FVector& ResolvedDestination, TArray<FVector>&& Route);
    void SetAttackOrder(APolitikaPrototypeUnit* Target);
    void SetGuardOrder();
    void StopOrder();
    void SetSelectedVisual(bool bSelected);
    void ConfigureInfantryPresentation(UPolitikaInfantryPresentationDefinition* Definition);
    void BeginDeathPresentation();
    void ConfigureInfantryBehavior(UPolitikaInfantryBehaviorDefinition* Definition);
    int32 GetAmmo() const { return Ammo; }
    bool IsReloading() const { return ReloadRemaining > 0; }
    bool HasInfantryBehavior() const { return InfantryBehavior != nullptr; }
    bool CanUseGrenade(const APolitikaPrototypeUnit* Target) const;
    bool CanReload() const;
    bool CanSprint() const;
    // Development-only acceptance fixtures; never used by the normal scenario.
#if !UE_BUILD_SHIPPING
    void SetAcceptanceHealth(int32 InHealth) { Health = FMath::Clamp(InHealth, 1, MaxHealth); }
#endif
    politika::rules::OrderEndReason SimulateMovement(float DeltaSeconds);
    void ResolveCombat(float DeltaSeconds);
    int32 ApplyPendingDamage();
    uint64 CleanupDeadTarget();
    uint64 GetStableId() const { return StableId; }
    uint32 GetOwnerPlayer() const { return OwnerPlayer; }
    FName GetFactionId() const { return FactionId; }
    int32 GetHealth() const { return Health; }
    int32 GetMaxHealth() const { return MaxHealth; }
    bool IsAlive() const { return Health > 0; }
    bool IsAttackable() const { return bAttackable; }
    bool CanMove() const { return MoveSpeed > 0.0f; }
    bool CanAttack() const { return WeaponDamage > 0.0f && WeaponCooldown > 0.0f && WeaponRange > 0.0f; }
    EPolitikaPrototypeOrder GetCurrentOrder() const { return CurrentOrder; }
    FText GetUnitName() const { return UnitName; }
    APolitikaPrototypeUnit* GetOrderTarget() const { return OrderTarget.Get(); }
private:
    friend class APolitikaPrototypeMatch;
#if WITH_DEV_AUTOMATION_TESTS
    friend class FPolitikaInfantryMechanicsTest;
#endif
    UPROPERTY(Transient) TObjectPtr<UPolitikaInfantryBehaviorDefinition> InfantryBehavior;
    int32 Ammo = 0;
    float ReloadRemaining = 0;
    float GrenadeCooldown = 0;
    float GrenadeWindup = 0;
    float SprintRemaining = 0;
    float SprintCooldown = 0;
    bool bBlastDeath = false;
    bool bPendingBlastDamage = false;
    bool bMovedThisStep = false;
    TWeakObjectPtr<APolitikaPrototypeUnit> GrenadeTarget;
    FVector StepStartPosition = FVector::ZeroVector;
    void BeginReload();
    void BeginGrenade(APolitikaPrototypeUnit* Target);
    bool HasLineOfFire(const APolitikaPrototypeUnit* Target) const;
#if !UE_BUILD_SHIPPING
    bool bTraceNavigationTraversal = false;
    TArray<FVector> NavigationStepTraversal;
#endif
#if WITH_DEV_AUTOMATION_TESTS
    friend class FPolitikaCommandCoordinatorAcceptanceAutomationTest;
    friend class FPolitikaCommandRepeatabilityAutomationTest;
    friend class FPolitikaMatchLifecycleAutomationTest;
    friend class FPolitikaMatchParticipantsAutomationTest;
    friend class FPolitikaMatchParticipantsAuthorityAutomationTest;
    friend class FPolitikaNavigationUnavailableAutomationTest;
    friend class FPolitikaNavigationObstacleAutomationTest;
    friend class FPolitikaNavigationFailureAutomationTest;
    friend class FPolitikaUnprojectableDestinationAutomationTest;
    friend class FPolitikaProjectionDisplacementAutomationTest;
    friend class FPolitikaNavigationScheduleAutomationTest;
    friend class FPolitikaNavigationInterruptionAutomationTest;
    friend class FPolitikaDefaultInfantryAutomationTest;
#endif
    UPROPERTY(VisibleAnywhere) TObjectPtr<UStaticMeshComponent> Body;
    UPROPERTY(VisibleAnywhere) TObjectPtr<UPolitikaInfantryPresentationComponent> InfantryPresentation;
    UPROPERTY(VisibleAnywhere) TObjectPtr<UStaticMeshComponent> SelectionIndicator;
    UPROPERTY() FText UnitName;
    UPROPERTY() int32 Health = 0;
    UPROPERTY() int32 MaxHealth = 0;
    UPROPERTY() uint64 StableId = 0;
    UPROPERTY() uint32 OwnerPlayer = 0;
    UPROPERTY() FName FactionId;
    UPROPERTY() float MoveSpeed = 0.0f;
    UPROPERTY() float WeaponRange = 0.0f;
    UPROPERTY() float WeaponDamage = 0.0f;
    UPROPERTY() float WeaponCooldown = 0.0f;
    UPROPERTY() float CooldownRemaining = 0.0f;
    UPROPERTY() float PendingDamage = 0.0f;
    UPROPERTY() bool bAttackable = false;
    UPROPERTY() EPolitikaPrototypeOrder CurrentOrder = EPolitikaPrototypeOrder::Idle;
    UPROPERTY() FVector OrderDestination = FVector::ZeroVector;
    // Route state belongs to this unit's current order and is advanced only by
    // APolitikaPrototypeMatch's authoritative fixed-step movement phase.
    UPROPERTY(Transient) TArray<FVector> NavigationWaypoints;
    UPROPERTY(Transient) int32 NavigationWaypointIndex = 0;
    UPROPERTY(Transient) FVector ResolvedNavigationDestination = FVector::ZeroVector;
    UPROPERTY(Transient) uint64 NavigationMatchGeneration = 0;
    UPROPERTY(Transient) uint64 NavigationOrderSequence = 0;
    UPROPERTY(Transient) float NavigationReplanCooldownSeconds = 0.0f;
    UPROPERTY(Transient) uint8 NavigationReplanAttempts = 0;
    UPROPERTY() TWeakObjectPtr<APolitikaPrototypeUnit> OrderTarget;
#if !UE_BUILD_SHIPPING
    bool bAcceptanceMovementLogged = false;
#endif
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
#if !UE_BUILD_SHIPPING
    void SetAcceptanceCaptureDuration(float Seconds) { CaptureDuration = FMath::Max(Seconds, KINDA_SMALL_NUMBER); }
#endif
private:
#if WITH_DEV_AUTOMATION_TESTS
    friend class FPolitikaCommandCoordinatorAcceptanceAutomationTest;
    friend class FPolitikaCommandRepeatabilityAutomationTest;
    friend class FPolitikaMatchLifecycleAutomationTest;
    friend class FPolitikaMatchParticipantsAutomationTest;
    friend class FPolitikaNavigationUnavailableAutomationTest;
    friend class FPolitikaNavigationObstacleAutomationTest;
    friend class FPolitikaNavigationFailureAutomationTest;
    friend class FPolitikaUnprojectableDestinationAutomationTest;
    friend class FPolitikaProjectionDisplacementAutomationTest;
    friend class FPolitikaNavigationScheduleAutomationTest;
    friend class FPolitikaNavigationInterruptionAutomationTest;
#endif
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
    APolitikaPrototypeUnit* GetSelectedUnitForPlayer(uint32 PlayerId);
    void SetSelection(uint32 PlayerId, APolitikaPrototypeUnit* Unit);
    bool SubmitLocalControllerCommand(APlayerController* Source, APolitikaPrototypeUnit* Unit,
        APolitikaPrototypeUnit* Target, const politika::rules::Order& Intent);
    bool IsLocalControllerBound(const APlayerController* Source) const;
    void ReportRejectedCommand(const FString& Reason) { LastCommandFeedback = Reason; }
    FString GetLastCommandFeedback() const { return LastCommandFeedback; }
    const TArray<politika::rules::GameplayEvent>& GetPublishedEvents() const { return PublishedEvents; }
    FPolitikaPrototypeGameplayEventPublished& OnGameplayEventPublished() { return GameplayEventPublished; }
    FVector2D GetPlayableMin() const { return PlayableMin; }
    FVector2D GetPlayableMax() const { return PlayableMax; }
    FVector2D GetCameraPanMin() const;
    FVector2D GetCameraPanMax() const;
    uint32 GetLocalRuntimePlayerId() const { return LocalRuntimePlayerId; }
    const TArray<FPolitikaPrototypeRuntimeParticipant>& GetRuntimeParticipants() const { return RuntimeParticipants; }
    const FPolitikaPrototypeSystemFactionAuthority& GetUnalignedSystemAuthority() const { return UnalignedSystemAuthority; }
    const class UPolitikaPrototypeMapDefinition* GetMapDefinition() const { return MapDefinition; }
    politika::rules::MatchPhase GetMatchPhase() const { return Lifecycle.phase; }
    const TOptional<FPolitikaPrototypeMatchResult>& GetCommittedResult() const { return CommittedResult; }
    void BeginReturnToMenu();
private:
    friend class APolitikaPrototypeUnit;
#if WITH_DEV_AUTOMATION_TESTS
    friend class FPolitikaInfantryMechanicsTest;
#endif
    struct FInfantryProjectile
    {
        FVector Start, End, Previous;
        uint32 Owner = 0;
        float Elapsed = 0, Duration = 0, Height = 0, Damage = 0, Radius = 0;
        uint64 Generation = 0;
    };
    TArray<FInfantryProjectile> InfantryProjectiles;
    void LaunchGrenade(APolitikaPrototypeUnit* Unit, const FVector& Destination);
    void AdvanceGrenades(float DeltaSeconds);
#if WITH_DEV_AUTOMATION_TESTS
    friend class FPolitikaGameplayEventPublicationAutomationTest;
    friend class FPolitikaDefaultInfantryAutomationTest;
    friend class FPolitikaCommandCoordinatorAcceptanceAutomationTest;
    friend class FPolitikaCommandRepeatabilityAutomationTest;
    friend class FPolitikaMatchLifecycleAutomationTest;
    friend class FPolitikaMatchParticipantsAutomationTest;
    friend class FPolitikaAuthoredStartupAdmissionAutomationTest;
    friend class FPolitikaMatchParticipantsAuthorityAutomationTest;
    friend class FPolitikaNavigationUnavailableAutomationTest;
    friend class FPolitikaNavigationObstacleAutomationTest;
    friend class FPolitikaNavigationScheduleAutomationTest;
    friend class FPolitikaNavigationInterruptionAutomationTest;
    friend class FPolitikaAuthoredStartupAdmissionAutomationTest;
    friend class FPolitikaNavigationFailureAutomationTest;
    friend class FPolitikaUnprojectableDestinationAutomationTest;
    friend class FPolitikaProjectionDisplacementAutomationTest;
    TFunction<void()> NavigationStepObserver;
#endif
    struct FPendingOrder
    {
        politika::rules::CommandRequest Request;
        TWeakObjectPtr<APolitikaPrototypeUnit> Unit;
        TWeakObjectPtr<APolitikaPrototypeUnit> Target;
        FVector Destination = FVector::ZeroVector;
        EPolitikaPrototypeOrder Kind = EPolitikaPrototypeOrder::Idle;
    };
    UPROPERTY() TArray<TObjectPtr<APolitikaPrototypeUnit>> Units;
    UPROPERTY() TObjectPtr<APolitikaPrototypeObjective> Objective;
    UPROPERTY() TMap<uint32, TWeakObjectPtr<APolitikaPrototypeUnit>> SelectionByPlayer;
    UPROPERTY() FString LastCommandFeedback = TEXT("Select a friendly unit; right-click ground or an enemy to issue an order.");
    TArray<FPolitikaContentDiagnostic> LastAdmissionDiagnostics;
    UPROPERTY() float SimulationStepSeconds = 0.05f;
    UPROPERTY() FVector2D PlayableMin = FVector2D::ZeroVector;
    UPROPERTY() FVector2D PlayableMax = FVector2D::ZeroVector;
    UPROPERTY() uint32 LocalRuntimePlayerId = 0;
    TArray<FPolitikaPrototypeRuntimeParticipant> RuntimeParticipants;
    FPolitikaPrototypeSystemFactionAuthority UnalignedSystemAuthority;
    UPROPERTY() bool bContentReady = false;
    politika::rules::MatchLifecycle Lifecycle;
    TOptional<FPolitikaPrototypeMatchResult> CommittedResult;
    UPROPERTY() TObjectPtr<class UPolitikaPrototypeContentCatalog> ContentCatalog;
    UPROPERTY() TObjectPtr<class UPolitikaPrototypeMapDefinition> MapDefinition;
    UPROPERTY() float StepAccumulator = 0.0f;
    uint64 SimulationStepsExecuted = 0;
    TArray<FPendingOrder> PendingOrders;
    TArray<politika::rules::GameplayEvent> PublishedEvents;
    TArray<politika::rules::GameplayEvent> PendingGameplayEvents;
    FPolitikaPrototypeGameplayEventPublished GameplayEventPublished;
    uint64 NextStableId = 1;
    uint64 NextCommandSequence = 1;
    uint64 MatchGeneration = 1;
    void BuildScenario();
    // Trusted coordinator/test entry points; ordinary input must use the controller-bound method above.
    void SubmitMove(uint32 PlayerId, APolitikaPrototypeUnit* Unit, const FVector& Destination, bool bAttackMove = false);
    void SubmitAttack(uint32 PlayerId, APolitikaPrototypeUnit* Unit, APolitikaPrototypeUnit* Target);
    void SubmitStop(uint32 PlayerId, APolitikaPrototypeUnit* Unit);
    void SubmitGuard(uint32 PlayerId, APolitikaPrototypeUnit* Unit);
    void SubmitCommand(uint32 PlayerId, APolitikaPrototypeUnit* Unit, APolitikaPrototypeUnit* Target,
        const politika::rules::Order& Intent);
    bool IsCommandableUnit(const APolitikaPrototypeUnit* Unit, uint32 PlayerId) const;
    static bool BuildRuntimeParticipantsFromSlots(const UPolitikaPrototypeMapDefinition& Definition,
        TArray<FPolitikaPrototypeRuntimeParticipant>& OutParticipants, uint32& OutLocalRuntimePlayerId,
        FName& OutFailureCode);
    bool TransitionLifecycle(politika::rules::MatchPhase NextPhase);
    void RecoverToShellAfterAdmissionFailure(FName MapId);
    void AdmitPendingOrders();
    bool ResolveNavigationRoute(APolitikaPrototypeUnit* Unit, const FVector& Destination,
        TArray<FVector>& OutWaypoints, FVector& OutResolvedDestination, FString& OutFailure,
        uint64 OrderSequence = 0);
    static bool IsCompleteNavigationResult(const FPathFindingResult& Result, FString& OutFailure);
    void RunSimulationStep(float DeltaSeconds);
    void QueueGameplayEvent(politika::rules::GameplayEvent&& Event);
    void PublishGameplayEvent(politika::rules::GameplayEvent&& Event);
    void PublishGameplayEvents();
    politika::rules::OrderResult ValidateOrder(const FPendingOrder& Pending) const;
#if !UE_BUILD_SHIPPING
    struct FNavigationAcceptance
    {
        bool bRunning = false;
        int32 Schedule = 0;
        uint64 OuterUpdates = 0;
        bool bRouteRecorded = false;
        bool bValid = true;
        TWeakObjectPtr<APolitikaPrototypeUnit> Unit;
        TWeakObjectPtr<AActor> Wall;
        FVector Previous = FVector::ZeroVector;
        FVector Extent = FVector::ZeroVector;
        float CollisionRadius = 0.0f;
        FVector Resolved = FVector::ZeroVector;
        FBox Blocker;
        TArray<FVector> Route;
        uint64 Sequence = 0;
        FString Interruption;
        uint64 FollowupSequence = 0;
        uint64 DuplicateSequence = 0;
        FVector BoundaryPosition = FVector::ZeroVector;
        int32 Stopped = 0;
        int32 Replaced = 0;
        int32 Rejected = 0;
        uint64 ArrivalStep = 0;
        int32 Commands = 0;
        int32 Terminations = 0;
        int32 Arrivals = 0;
        int32 Failures = 0;
        int32 Samples = 0;
        double MaxTravel = 0.0;
    } NavigationAcceptance;
    struct FNavigationFailureAcceptance
    {
        bool bRunning = false;
        bool bValid = true;
        bool bDestinationProjectionCase = false;
        bool bExcessiveProjectionCase = false;
        TWeakObjectPtr<APolitikaPrototypeUnit> Unit;
        FVector Start = FVector::ZeroVector;
        FVector Goal = FVector::ZeroVector;
        uint64 Sequence = 0;
        uint64 FailureStep = 0;
        int32 Queries = 0;
        int32 ProjectionChecks = 0;
        bool bOriginProjected = false;
        bool bGoalProjected = false;
        FVector ProjectedOrigin = FVector::ZeroVector;
        FVector ProjectedGoal = FVector::ZeroVector;
        double ProjectionDisplacement = 0.0;
        int32 AcceptedCommands = 0;
        int32 FailureEvents = 0;
        int32 Arrivals = 0;
        int32 Samples = 0;
        uint32 LastQueryResult = 0;
        bool bLastQueryPartial = false;
        bool bLastPathValid = false;
        int32 LastPathPoints = 0;
        FString LastFailure;
        uint32 DiagnosticResult = 0;
        bool bDiagnosticPartial = false;
        bool bDiagnosticPathValid = false;
        int32 DiagnosticPathPoints = 0;
        float DiagnosticEndpointError = 0.0f;
        FString DiagnosticRejection;
    } NavigationFailureAcceptance;
    void BeginNavigationAcceptance();
    void BeginNavigationFailureAcceptance();
    void AfterNavigationFailureStep();
    void ObserveNavigationFailureEvent(const politika::rules::GameplayEvent& Event);
    void FinishNavigationFailureAcceptance(const TCHAR* Failure);
    void BeforeNavigationAcceptanceStep();
    void AfterNavigationAcceptanceStep(float DeltaSeconds);
    void ObserveNavigationAcceptanceEvent(const politika::rules::GameplayEvent& Event);
    void FinishNavigationAcceptance(const TCHAR* Failure);
    struct FParticipantAuthorityAcceptance
    {
        bool bRunning = false;
        uint64 StartStep = 0;
        uint64 ForeignEntityId = 0, SystemEntityId = 0, DeadEntityId = 0, DestroyedEntityId = 0;
        uint64 ForeignSeedSequence = 0;
        uint64 LocalSequence = 0, ForeignSequence = 0, SystemSequence = 0, UnassignedSequence = 0;
        uint64 DeadSequence = 0, DestroyedSequence = 0;
        int32 ForeignSeedOutcomes = 0, LocalOutcomes = 0, ForeignOutcomes = 0, SystemOutcomes = 0, UnassignedOutcomes = 0;
        int32 DeadOutcomes = 0, DestroyedOutcomes = 0, ForeignOrderChanges = 0;
        bool bForeignSeedAccepted = false, bForeignBaselineCaptured = false;
        bool bLocalAccepted = false, bSetupChecksPassed = false, bSystemRecordValid = false;
        politika::rules::OrderRejection ForeignRejection = politika::rules::OrderRejection::None;
        politika::rules::OrderRejection SystemRejection = politika::rules::OrderRejection::None;
        politika::rules::OrderRejection UnassignedRejection = politika::rules::OrderRejection::None;
        politika::rules::OrderRejection DeadRejection = politika::rules::OrderRejection::None;
        politika::rules::OrderRejection DestroyedRejection = politika::rules::OrderRejection::None;
        TWeakObjectPtr<APolitikaPrototypeUnit> LocalUnit, ForeignUnit, SystemUnit, DeadUnit, DestroyedUnit;
        uint64 ForeignRouteSequence = 0;
        int32 ForeignWaypointIndex = 0;
        FVector ForeignPosition = FVector::ZeroVector;
        TArray<FVector> ForeignRoutePoints;
    } ParticipantAuthorityAcceptance;
    enum class EAcceptancePhase : uint8 { None, CaptureApproach, CaptureHold, CaptureExit, CapturePause, CombatSetup, CombatResolve, ParticipantAuthoritySeed, ParticipantAuthority, Done };
    EAcceptancePhase AcceptancePhase = EAcceptancePhase::None;
    FString AcceptanceMode;
    float AcceptanceElapsed = 0.0f;
    float AcceptanceProgressAtPause = 0.0f;
    bool bAcceptanceAttackerOrderCleared = false;
    TWeakObjectPtr<APolitikaPrototypeUnit> AcceptanceAttacker;
    TWeakObjectPtr<APolitikaPrototypeUnit> AcceptanceDefender;
    void BeginAcceptanceHarness();
    void BeginParticipantAuthorityAcceptance();
    void ContinueParticipantAuthorityAcceptance();
    void TickAcceptanceHarness(float DeltaSeconds);
    void SetAcceptanceMove(APolitikaPrototypeUnit* Unit, const FVector& Destination);
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
    friend class APolitikaPrototypeMatch;
#if WITH_DEV_AUTOMATION_TESTS
    friend class FPolitikaMatchParticipantsAuthorityAutomationTest;
#endif
protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    virtual void SetupInputComponent() override;
private:
    UPROPERTY() TWeakObjectPtr<APolitikaPrototypeMatch> Match;
    UPROPERTY() TWeakObjectPtr<APolitikaPrototypeUnit> SelectedUnit;
    bool bMapCameraApplied = false;
#if !UE_BUILD_SHIPPING
    bool bAcceptancePanForwardLogged = false;
    bool bAcceptancePanRightLogged = false;
#endif
    void SelectUnderCursor();
    void SelectUnitForInput(APolitikaPrototypeUnit* Unit);
    APolitikaPrototypeUnit* GetCommandableSelectedUnit(APolitikaPrototypeMatch* ActiveMatch);
    void OrderUnderCursor();
    void StopSelected();
    void GuardSelected();
    void AttackMoveSelected();
    void GrenadeSelected();
    void ReloadSelected();
    void SprintSelected();
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
    void AttachMatch(APolitikaPrototypeMatch* InMatch);
    virtual void DrawHUD() override;
private:
    UPROPERTY() TWeakObjectPtr<APolitikaPrototypeMatch> Match;
    FDelegateHandle GameplayEventHandle;
    FString CommandFeedback;
    void HandleGameplayEvent(const politika::rules::GameplayEvent& Event);
};

UCLASS()
class POLITIKA_API APolitikaPrototypeGameMode final : public AGameModeBase
{
    GENERATED_BODY()
public:
    APolitikaPrototypeGameMode();
    virtual void StartPlay() override;
};
