#pragma once

#include "CoreMinimal.h"
#include "Engine/TimerHandle.h"
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
	// Team of the winning participant (each standalone participant is its own team).
	uint8 WinningTeamId = 0;
};

// Default horizontal sight radius for units configured without an archetype value.
inline constexpr float PolitikaDefaultSightRadiusCm = 1200.0f;

// LocalHuman is the authored human slot on this process (standalone player or
// listen-server host). RemoteHuman is a seat bound by the server to one admitted
// remote connection; it is never inferred from client-provided data.
enum class EPolitikaPrototypeControllerKind : uint8 { LocalHuman, AssignedOpponent, RemoteHuman };

struct FPolitikaPrototypeRuntimeParticipant
{
    int32 SlotId = 0;
    uint32 RuntimePlayerId = 0;
    FName FactionId;
    EPolitikaPrototypeControllerKind ControllerKind = EPolitikaPrototypeControllerKind::AssignedOpponent;
    // Alliance team (politika::rules::kTeamA/kTeamB in networked layouts; one team per
    // participant offline). Assigned by the server from the session layout.
    uint8 TeamId = 0;
};

struct FPolitikaPrototypeSystemFactionAuthority
{
    FName FactionId;
    bool bNpcSystemControlled = false;
};

UENUM(BlueprintType)
enum class EPolitikaPrototypeOrder : uint8 { Idle, Move, Attack, AttackMove, Stop, Guard, Grenade, Reload, Sprint };

// Versioned remote command intent (network_protocol in version.json). It carries
// no participant, owner, faction or object pointer: the server derives authority
// from the sending connection and resolves entity IDs in the current match.
USTRUCT()
struct POLITIKA_API FPolitikaNetCommandIntent
{
    GENERATED_BODY()
    UPROPERTY() int32 ProtocolVersion = 0;
    UPROPERTY() uint8 OrderKind = 0;
    UPROPERTY() uint64 IssuerEntityId = 0;
    UPROPERTY() uint64 TargetEntityId = 0;
    // Rules-domain map coordinates, relative to the playable minimum (cm).
    UPROPERTY() double DestinationX = 0.0;
    UPROPERTY() double DestinationY = 0.0;
    UPROPERTY() uint32 ClientSequence = 0;
    UPROPERTY() uint32 CorrelationId = 0;
    // Client-observed match generation; checked by the shared AdmitCommand guard.
    UPROPERTY() uint64 MatchGeneration = 0;
};

// Sanitized, correlation-linked result returned only to the issuing connection.
USTRUCT()
struct POLITIKA_API FPolitikaNetCommandOutcome
{
    GENERATED_BODY()
    UPROPERTY() uint32 CorrelationId = 0;
    UPROPERTY() uint32 ClientSequence = 0;
    UPROPERTY() uint64 IssuerEntityId = 0;
    UPROPERTY() uint64 MatchGeneration = 0;
    UPROPERTY() uint64 Step = 0;
    UPROPERTY() uint8 OrderKind = 0;
    UPROPERTY() uint8 Rejection = 0;
    UPROPERTY() bool bAccepted = false;
};

// Public roster entry (slot, participant, faction, controller kind) for observers.
USTRUCT()
struct POLITIKA_API FPolitikaNetParticipantInfo
{
    GENERATED_BODY()
    UPROPERTY() int32 SlotId = 0;
    UPROPERTY() uint32 RuntimePlayerId = 0;
    UPROPERTY() FName FactionId;
    UPROPERTY() uint8 ControllerKind = 0;
    UPROPERTY() uint8 TeamId = 0;
};

UCLASS()
class POLITIKA_API APolitikaPrototypeUnit final : public AActor
{
    GENERATED_BODY()
public:
    APolitikaPrototypeUnit();
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    // Fog of war: when the match enforces it, a connection receives this unit only while
    // its participant's team observes it (own and allied units are always observed).
    virtual bool IsNetRelevantFor(const AActor* RealViewer, const AActor* ViewTarget, const FVector& SrcLocation) const override;
    // Authority-assigned alliance and vision. Hostility is derived from owner and team.
    void SetTeamId(uint8 InTeamId) { TeamId = InTeamId; }
    uint8 GetTeamId() const { return TeamId; }
    void SetSightRadius(float InSightRadiusCm) { SightRadius = FMath::Max(0.0f, InSightRadiusCm); }
    float GetSightRadius() const { return SightRadius; }
    bool IsHostileTo(const APolitikaPrototypeUnit* Other) const;
    // Server-computed bitmask of teams currently observing this unit (bit = 1 << team).
    bool IsObservedByTeam(uint8 Team) const { return Team < 8 && (ObservedByTeams & (1u << Team)) != 0; }
    // Local presentation only (listen-server host view of fog); never replicated.
    void SetLocalFogHidden(bool bHidden);
    bool IsLocalFogHidden() const { return bLocalFogHidden; }
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
    // Shared content identity used by remote observers to resolve the same authored
    // presentation locally. Authority sets it at spawn; it never grants control.
    void SetArchetypeId(FName InArchetypeId) { ArchetypeId = InArchetypeId; }
    FName GetArchetypeId() const { return ArchetypeId; }
    // Client-side presentation only: mesh, scale, tint and infantry visuals.
    void ApplyObservedArchetypePresentation(const class UPolitikaUnitArchetypeDefinition* Definition);
    bool HasObservedPresentation() const { return bObservedPresentationApplied; }
    void PlayLocalAcknowledgement(FName Event);
    void BeginDeathPresentation();
    void ConfigureInfantryBehavior(UPolitikaInfantryBehaviorDefinition* Definition);
    int32 GetAmmo() const { return Ammo; }
    bool IsReloading() const { return ReloadRemaining > 0; }
    bool IsSprinting() const { return SprintRemaining > 0; }
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
    friend class FPolitikaMultiplayerParticipantBindingTest;
    friend class FPolitikaMultiplayerCommandIntakeTest;
    friend class FPolitikaMultiplayerAuthorityOnlySimulationTest;
    friend class FPolitikaMultiplayerResultAndCleanupTest;
    friend class FPolitikaMultiplayerFogOfWarTest;
    friend struct FPolitikaMultiplayerFixture;
#endif
    UFUNCTION() void OnRep_ArchetypeId();
    UFUNCTION() void OnRep_Health();
    // Coordinator that spawned this unit (authority only); supplies fog/relevancy policy.
    TWeakObjectPtr<APolitikaPrototypeMatch> AuthorityMatch;
    float SightRadius = PolitikaDefaultSightRadiusCm;
    uint8 ObservedByTeams = 0;
    bool bLocalFogHidden = false;
    // Public alliance (friend/foe presentation); authority is still server-side.
    UPROPERTY(Replicated) uint8 TeamId = 0;
    // Cosmetic fan-out of authoritative events. Authority already presented them
    // locally; remote observers only animate/play audio and never change rules state.
    UFUNCTION(NetMulticast, Unreliable) void MulticastShotPresentation(FVector_NetQuantize Target, float Cooldown);
    UFUNCTION(NetMulticast, Unreliable) void MulticastAbilityPresentation(uint8 Cue, float Seconds);
    void TryApplyObservedPresentation();
    UPROPERTY(Transient) TObjectPtr<UPolitikaInfantryBehaviorDefinition> InfantryBehavior;
    // Owner-only HUD state (ammunition/reload); other connections never receive it.
    UPROPERTY(Replicated) int32 Ammo = 0;
    UPROPERTY(Replicated) float ReloadRemaining = 0;
    float GrenadeCooldown = 0;
    float GrenadeWindup = 0;
    UPROPERTY(Replicated) float SprintRemaining = 0;
    float SprintCooldown = 0;
    UPROPERTY(Replicated) bool bBlastDeath = false;
    UPROPERTY(ReplicatedUsing=OnRep_ArchetypeId) FName ArchetypeId;
    bool bObservedPresentationApplied = false;
    bool bDeathPresented = false;
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
    // Public observation: identity, allegiance, health and capability flags.
    UPROPERTY(Replicated) FText UnitName;
    UPROPERTY(ReplicatedUsing=OnRep_Health) int32 Health = 0;
    UPROPERTY(Replicated) int32 MaxHealth = 0;
    UPROPERTY(Replicated) uint64 StableId = 0;
    UPROPERTY(Replicated) uint32 OwnerPlayer = 0;
    UPROPERTY(Replicated) FName FactionId;
    UPROPERTY(Replicated) float MoveSpeed = 0.0f;
    UPROPERTY(Replicated) float WeaponRange = 0.0f;
    UPROPERTY(Replicated) float WeaponDamage = 0.0f;
    UPROPERTY(Replicated) float WeaponCooldown = 0.0f;
    // Authority-only simulation internals below are never replicated.
    UPROPERTY() float CooldownRemaining = 0.0f;
    UPROPERTY() float PendingDamage = 0.0f;
    UPROPERTY(Replicated) bool bAttackable = false;
    // Owner-only: an opponent sees movement and fire, not the issued order.
    UPROPERTY(Replicated) EPolitikaPrototypeOrder CurrentOrder = EPolitikaPrototypeOrder::Idle;
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
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    void ConfigureFromDefinition(const struct FPolitikaPrototypeObjectiveDefinition& Definition, uint32 LocalPlayerId);
    // Presentation only (marker mesh, scale and tint); used by authority and observers.
    void ConfigurePresentation(const struct FPolitikaPrototypeObjectiveDefinition& Definition);
    // Teams allowed to capture. Offline this is only the local human's team (opponents
    // contest); networked sessions allow every human seat's team.
    void SetEligibleTeams(const TArray<uint8>& InTeams) { EligibleTeams = InTeams; }
    void UpdateCapture(const TArray<APolitikaPrototypeUnit*>& Units, float DeltaSeconds);
    float GetProgress() const { return Progress; }
    bool IsContested() const { return bContested; }
    bool IsComplete() const { return bComplete; }
    uint32 GetCapturingPlayerId() const { return CapturingPlayerId; }
    uint8 GetCapturingTeamId() const { return CapturingTeamId; }
    bool HasMarkerPresentation() const;
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
    friend class FPolitikaMultiplayerResultAndCleanupTest;
    friend class FPolitikaMultiplayerFogOfWarTest;
#endif
    UPROPERTY(VisibleAnywhere) TObjectPtr<UStaticMeshComponent> Marker;
    UPROPERTY() float CaptureRadius = 0.0f;
    UPROPERTY() float CaptureDuration = 0.0f;
    UPROPERTY() uint32 LocalRuntimePlayerId = 0;
    TArray<uint8> EligibleTeams;
    UPROPERTY(Replicated) float Progress = 0.0f;
    UPROPERTY(Replicated) bool bContested = false;
    UPROPERTY(Replicated) bool bComplete = false;
    // First participant of the capturing team present when the capture began (attribution).
    UPROPERTY(Replicated) uint32 CapturingPlayerId = 0;
    UPROPERTY(Replicated) uint8 CapturingTeamId = 0;
};

UCLASS()
class POLITIKA_API APolitikaPrototypeMatch final : public AActor
{
    GENERATED_BODY()
public:
    APolitikaPrototypeMatch();
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void Tick(float DeltaSeconds) override;
    const TArray<TObjectPtr<APolitikaPrototypeUnit>>& GetUnits() const { return Units; }
    APolitikaPrototypeObjective* GetObjective() const { return Objective; }
    APolitikaPrototypeUnit* GetSelectedUnitForPlayer(uint32 PlayerId);
    void SetSelection(uint32 PlayerId, APolitikaPrototypeUnit* Unit);
    // Pointer adapter for the local controller and trusted test fixtures; it builds
    // the same versioned intent and calls SubmitControllerIntent.
    bool SubmitLocalControllerCommand(APlayerController* Source, APolitikaPrototypeUnit* Unit,
        APolitikaPrototypeUnit* Target, const politika::rules::Order& Intent);
    // The single server-side command intake for host-local and remote input. Authority
    // is derived from the connection binding; payload, freshness and entity IDs are
    // validated before the existing typed admission pipeline. Returns false only when
    // the source has no participant binding (no typed command is created).
    bool SubmitControllerIntent(APlayerController* Source, const FPolitikaNetCommandIntent& Intent);
    bool IsLocalControllerBound(const APlayerController* Source) const;
    // Server connection lifecycle. A binding is the only source of command authority.
    bool BindController(APlayerController* Controller, FName& OutFailure);
    void HandleControllerLogout(AController* Exiting);
    bool CanAdmitRemoteParticipant(FName& OutReason) const;
    uint32 GetBoundParticipantId(const APlayerController* Controller) const;
    bool IsNetworkSession() const;
    uint64 GetMatchGeneration() const { return MatchGeneration; }
    APolitikaPrototypeUnit* FindUnitByStableId(uint64 StableId) const;
    const class UPolitikaPrototypeContentCatalog* GetContentCatalog() const { return ContentCatalog; }
    const TArray<FPolitikaNetParticipantInfo>& GetReplicatedParticipants() const { return ReplicatedParticipants; }
    // Remote observer only: the participant assigned to this process by its server.
    void SetLocalViewerParticipant(uint32 PlayerId);
    // Session layout and alliances. Standalone matches give every participant its own team.
    politika::rules::TeamLayout GetSessionLayout() const { return SessionLayout; }
    int32 GetSessionSeatCount() const { return SessionSeatCount; }
    uint8 GetTeamForParticipant(uint32 PlayerId) const;
    uint8 GetLocalViewerTeam() const { return GetTeamForParticipant(LocalRuntimePlayerId); }
    // Fog of war is enforced in networked sessions: server-computed team vision filters
    // replication, target admission and acquisition. Offline play keeps full vision.
    bool IsFogOfWarEnabled() const { return bFogOfWar; }
    bool IsUnitObservedByParticipant(const APolitikaPrototypeUnit* Unit, uint32 PlayerId) const;
    // Relevancy query used by APolitikaPrototypeUnit::IsNetRelevantFor (server only).
    bool IsUnitRelevantToViewer(const APolitikaPrototypeUnit* Unit, const AActor* RealViewer) const;
    // Observer-side unit registry: clients only hold units their team currently observes.
    void RegisterObservedUnit(APolitikaPrototypeUnit* Unit);
    void UnregisterObservedUnit(APolitikaPrototypeUnit* Unit);
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
        uint8 Team = 0;
        float Elapsed = 0, Duration = 0, Height = 0, Damage = 0, Radius = 0;
        uint64 Generation = 0;
    };
    TArray<FInfantryProjectile> InfantryProjectiles;
    void LaunchGrenade(APolitikaPrototypeUnit* Unit, const FVector& Destination);
    void AdvanceGrenades(float DeltaSeconds);
    // Server-only connection bindings. Never replicated; clients receive only their
    // own owner-only assignment and the public roster.
    struct FParticipantBinding
    {
        TWeakObjectPtr<APlayerController> Controller;
        uint32 RuntimePlayerId = 0;
        bool bRemote = false;
        politika::rules::ClientSequenceGuard Sequence;
    };
    TArray<FParticipantBinding> ParticipantBindings;
    struct FOutcomeRoute
    {
        TWeakObjectPtr<APlayerController> Controller;
        uint32 ClientSequence = 0;
        uint32 CorrelationId = 0;
        uint64 Generation = 0;
        uint8 RawOrderKind = 0;
    };
    TMap<uint64, FOutcomeRoute> OutcomeRoutes;
    FParticipantBinding* FindBinding(const APlayerController* Controller);
    const FParticipantBinding* FindBinding(const APlayerController* Controller) const;
    FParticipantBinding* ResolveCommandBinding(APlayerController* Source);
    void BindExistingControllers();
    void RefreshUnitNetOwnership(uint32 PlayerId, APlayerController* Controller);
    void SyncReplicatedParticipants();
    void SyncReplicatedLifecycle();
    void TryActivateNetworkMatch();
    void TerminateForParticipantDisconnect(uint32 PlayerId);
    void RouteCommandOutcome(const politika::rules::CommandOutcomeEvent& Outcome);
    void RejectIntentAtIntake(APlayerController* Source, const FPolitikaNetCommandIntent& Intent,
        uint32 ParticipantId, politika::rules::OrderRejection Rejection);
    void ApplyMapPresentation();
    bool ResolveSessionLayout(FName& OutFailure);
    void UpdateTeamVisibility();
    void ApplyLocalFogPresentation();
    uint32 GetMaxRemoteHumans() const;
    void InitializeObservedMatch();
    void ApplyObservedPresentationToUnits();
    UFUNCTION() void OnRep_MapDefinitionId();
    UFUNCTION() void OnRep_Objective();
    UFUNCTION() void OnRep_Lifecycle();
    UFUNCTION() void OnRep_Participants();
    UPROPERTY(ReplicatedUsing=OnRep_MapDefinitionId) FName ReplicatedMapId;
    UPROPERTY(ReplicatedUsing=OnRep_Lifecycle) uint8 ReplicatedPhase = 0;
    UPROPERTY(ReplicatedUsing=OnRep_Lifecycle) bool bReplicatedResultCommitted = false;
    UPROPERTY(ReplicatedUsing=OnRep_Lifecycle) uint32 ReplicatedResultWinner = 0;
    UPROPERTY(ReplicatedUsing=OnRep_Lifecycle) FName ReplicatedResultReason;
    UPROPERTY(ReplicatedUsing=OnRep_Lifecycle) uint8 ReplicatedResultTeam = 0;
    UPROPERTY(ReplicatedUsing=OnRep_Participants) TArray<FPolitikaNetParticipantInfo> ReplicatedParticipants;
    // Session policy mirrored to observers for presentation (HUD layout/fog labels).
    UPROPERTY(ReplicatedUsing=OnRep_Participants) uint8 ReplicatedLayout = 0;
    UPROPERTY(ReplicatedUsing=OnRep_Participants) bool bReplicatedFogOfWar = false;
    politika::rules::TeamLayout SessionLayout = politika::rules::TeamLayout::OneVsOne;
    int32 SessionSeatCount = 0;
    bool bFogOfWar = false;
    bool bAwaitingNetworkParticipants = false;
    bool bObservedContentReady = false;
    bool bMapPresentationApplied = false;
    FTimerHandle NetworkReturnTimer;
#if !UE_BUILD_SHIPPING
    // Development multiplayer acceptance: server-side fixture actors and sampling.
    bool bMultiplayerAcceptance = false;
    bool bMultiplayerFixtureSpawned = false;
    uint64 LastMultiplayerSampleStep = 0;
    double LastObserverSampleSeconds = 0.0;
    void BeginMultiplayerAcceptance();
    void SampleMultiplayerState(const TCHAR* Reason);
    void SampleObservedState(const TCHAR* Reason);
#endif
#if WITH_DEV_AUTOMATION_TESTS
    friend class FPolitikaMultiplayerParticipantBindingTest;
    friend class FPolitikaMultiplayerCommandIntakeTest;
    friend class FPolitikaMultiplayerAuthorityOnlySimulationTest;
    friend class FPolitikaMultiplayerResultAndCleanupTest;
    friend class FPolitikaMultiplayerFogOfWarTest;
    friend struct FPolitikaMultiplayerFixture;
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
    // Authority: every live match unit. Observer: only units its connection currently
    // receives (fog-filtered); membership follows unit BeginPlay/EndPlay. Not replicated,
    // so the array size cannot reveal hidden enemy counts.
    UPROPERTY() TArray<TObjectPtr<APolitikaPrototypeUnit>> Units;
    UPROPERTY(ReplicatedUsing=OnRep_Objective) TObjectPtr<APolitikaPrototypeObjective> Objective;
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
    UPROPERTY(Replicated) uint64 MatchGeneration = 1;
    void BuildScenario();
    // Trusted coordinator/test entry points; ordinary input must use the controller-bound method above.
    void SubmitMove(uint32 PlayerId, APolitikaPrototypeUnit* Unit, const FVector& Destination, bool bAttackMove = false);
    void SubmitAttack(uint32 PlayerId, APolitikaPrototypeUnit* Unit, APolitikaPrototypeUnit* Target);
    void SubmitStop(uint32 PlayerId, APolitikaPrototypeUnit* Unit);
    void SubmitGuard(uint32 PlayerId, APolitikaPrototypeUnit* Unit);
    // Returns the assigned server sequence. RequestedIssuerId/CorrelationId and the
    // generation hint are supplied by the controller intake; trusted callers omit them.
    uint64 SubmitCommand(uint32 PlayerId, APolitikaPrototypeUnit* Unit, APolitikaPrototypeUnit* Target,
        const politika::rules::Order& Intent, uint64 RequestedIssuerId = 0, uint64 CorrelationId = 0,
        TOptional<uint64> GenerationHint = {});
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
    friend class FPolitikaMultiplayerParticipantBindingTest;
    friend class FPolitikaMultiplayerCommandIntakeTest;
    friend class FPolitikaMultiplayerAuthorityOnlySimulationTest;
    friend class FPolitikaMultiplayerResultAndCleanupTest;
    friend class FPolitikaMultiplayerFogOfWarTest;
    friend struct FPolitikaMultiplayerFixture;
#endif
public:
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    // Owner-only replicated assignment, written by the server when it binds this
    // connection. It labels the local view; the server never trusts it for authority.
    uint32 GetAssignedRuntimePlayerId() const { return AssignedRuntimePlayerId; }
    int32 GetAssignedSlotId() const { return AssignedSlotId; }
    FName GetAssignedFactionId() const { return AssignedFactionId; }
    void SetServerAssignment(uint32 RuntimePlayerId, int32 SlotId, FName FactionId, uint64 Generation);
    void ClearServerAssignment();
    // The only network command ingress. Host-local calls execute directly on authority.
    UFUNCTION(Server, Reliable) void ServerSubmitCommandIntent(const FPolitikaNetCommandIntent& Intent);
    UFUNCTION(Client, Reliable) void ClientReceiveCommandOutcome(const FPolitikaNetCommandOutcome& Outcome);
    virtual void ClientWasKicked_Implementation(const FText& KickReason) override;
    // The host/standalone seat controller (has a ULocalPlayer on this process).
    bool IsLocalSeatController() const;
#if WITH_DEV_AUTOMATION_TESTS
    // Automation-only: a spawned controller in a listen world is remote unless marked as
    // the host's local seat (a real host gets this from its ULocalPlayer).
    void MarkLocalControllerForTest() { bTestLocalSeat = true; }
    bool bTestLocalSeat = false;
#endif
protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    virtual void SetupInputComponent() override;
private:
    UPROPERTY() TWeakObjectPtr<APolitikaPrototypeMatch> Match;
    UPROPERTY() TWeakObjectPtr<APolitikaPrototypeUnit> SelectedUnit;
    UPROPERTY(ReplicatedUsing=OnRep_Assignment) uint32 AssignedRuntimePlayerId = 0;
    UPROPERTY(Replicated) int32 AssignedSlotId = 0;
    UPROPERTY(Replicated) FName AssignedFactionId;
    UPROPERTY(Replicated) uint64 AssignedMatchGeneration = 0;
    UFUNCTION() void OnRep_Assignment();
    uint32 NextClientSequence = 1;
    uint32 NextCorrelationId = 1;
    bool SubmitIntent(APolitikaPrototypeMatch* ActiveMatch, APolitikaPrototypeUnit* Unit,
        APolitikaPrototypeUnit* Target, const politika::rules::Order& Order);
    void ShowLocalFeedback(const FString& Feedback);
#if !UE_BUILD_SHIPPING
    // Development-only hostile-input probe: sends forged intents through the real RPC.
    bool bHostileProbeEnabled = false;
    bool bHostileProbeSent = false;
    bool bPostTerminalProbeSent = false;
    double HostileProbeArmedSeconds = 0.0;
    void TickHostileProbe();
    void SendProbeIntent(const TCHAR* Name, FPolitikaNetCommandIntent Intent, bool bAdvanceSequence = true);
#endif
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
    uint32 GetCommandPlayerId() const;
};

UCLASS()
class POLITIKA_API APolitikaPrototypeHUD final : public AHUD
{
    GENERATED_BODY()
public:
    APolitikaPrototypeHUD();
    void AttachMatch(APolitikaPrototypeMatch* InMatch);
    virtual void DrawHUD() override;
    // Only the owning controller's own correlated outcomes reach this HUD.
    void ShowCommandOutcome(const FPolitikaNetCommandOutcome& Outcome);
    void ShowLocalFeedback(const FString& Feedback) { CommandFeedback = Feedback; }
private:
    UPROPERTY() TWeakObjectPtr<APolitikaPrototypeMatch> Match;
    FString CommandFeedback;
};

UCLASS()
class POLITIKA_API APolitikaPrototypeGameMode final : public AGameModeBase
{
    GENERATED_BODY()
public:
    APolitikaPrototypeGameMode();
    virtual void StartPlay() override;
    // Listen-server admission: version and capacity are checked before a
    // PlayerController exists; PostLogin binds, Logout revokes.
    virtual void PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId,
        FString& ErrorMessage) override;
    virtual void PostLogin(APlayerController* NewPlayer) override;
    virtual void Logout(AController* Exiting) override;
private:
    UPROPERTY() TWeakObjectPtr<APolitikaPrototypeMatch> ActiveMatch;
};
