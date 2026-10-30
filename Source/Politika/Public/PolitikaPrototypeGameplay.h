#pragma once

#include "CoreMinimal.h"
#include "Engine/TimerHandle.h"
#include "GameFramework/Actor.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/HUD.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Rules.h"
#include "Fog.h"
#include "GroupSelection.h"
#include "FormationGeometry.h"
#include "Formation.h"
#include "Stance.h"
#include "Discipline.h"
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

// What an idle unit is doing on its own under its combat stance (FORM-F4): standing at its
// post, chasing a target inside its leash, or walking back to its post. Never an order.
UENUM()
enum class EPolitikaUnitAutonomy : uint8 { Holding, Chasing, Returning };

// HUD labels for stance and adherence (PolitikaFormations.cpp).
FString PolitikaStanceLabel(politika::stance::CombatStance Stance);
FString PolitikaAdherenceLabel(politika::stance::Adherence Adherence);

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

// One participant's local selection (RTS_SELECTION_AND_GROUP_ORDERS.md §1): presentation
// state only, sorted by stable ID and revalidated on every read. Never command authority.
USTRUCT()
struct POLITIKA_API FPolitikaSelectionSet
{
    GENERATED_BODY()
    UPROPERTY() TArray<TWeakObjectPtr<APolitikaPrototypeUnit>> Units;
};

// Versioned formation request (FORM-F3, network_protocol 3; stance and adherence FORM-F4,
// network_protocol 4). Like the command intent it carries no participant or owner: the server
// derives authority from the sending connection, resolves member and formation IDs in the
// current match and fans a formation move out into ordinary, individually admitted member
// commands. A stance or adherence change is not an order and submits none.
USTRUCT()
struct POLITIKA_API FPolitikaNetFormationIntent
{
    GENERATED_BODY()
    UPROPERTY() int32 ProtocolVersion = 0;
    // politika::formation::FormationOp: Form, Release, Move, AttackMove, Stop, SetStance, SetAdherence.
    UPROPERTY() uint8 Op = 0;
    // Addressed formation for Move/AttackMove/Stop/SetAdherence and a whole-formation SetStance;
    // zero for Form, Release and a per-unit SetStance.
    UPROPERTY() uint32 FormationId = 0;
    // politika::formation::Shape for Form; zero otherwise.
    UPROPERTY() uint8 Shape = 0;
    // Unit IDs for Form (2-64), Release and a per-unit SetStance; empty otherwise.
    UPROPERTY() TArray<uint64> Members;
    // politika::stance::CombatStance for SetStance, politika::stance::Adherence for
    // SetAdherence; zero otherwise.
    UPROPERTY() uint8 Value = 0;
    // Rules-domain map coordinates for Move/AttackMove (cm from the playable minimum).
    UPROPERTY() double DestinationX = 0.0;
    UPROPERTY() double DestinationY = 0.0;
    // Shares the connection's single-use command sequence.
    UPROPERTY() uint32 ClientSequence = 0;
    UPROPERTY() uint32 CorrelationId = 0;
    UPROPERTY() uint64 MatchGeneration = 0;
};

// Result of one formation request, for the issuing connection only. A formation move's
// member commands still return their own command outcomes under the same correlation.
USTRUCT()
struct POLITIKA_API FPolitikaNetFormationOutcome
{
    GENERATED_BODY()
    UPROPERTY() uint32 CorrelationId = 0;
    UPROPERTY() uint32 ClientSequence = 0;
    UPROPERTY() uint8 Op = 0;
    UPROPERTY() uint32 FormationId = 0;
    UPROPERTY() uint8 Rejection = 0;
    UPROPERTY() bool bAccepted = false;
    // Members of the formed or addressed formation (released units for Release).
    UPROPERTY() int32 Members = 0;
    // Member commands submitted for a Move/AttackMove/Stop.
    UPROPERTY() int32 Orders = 0;
    UPROPERTY() uint64 MatchGeneration = 0;
};

// Owner-only view of one of the participant's own formations, for HUD and input routing.
USTRUCT()
struct POLITIKA_API FPolitikaNetFormationInfo
{
    GENERATED_BODY()
    UPROPERTY() uint32 FormationId = 0;
    UPROPERTY() uint8 Shape = 0;
    UPROPERTY() uint8 State = 0;
    UPROPERTY() bool bHasDestination = false;
    UPROPERTY() uint8 Following = 0;
    UPROPERTY() uint16 Regroups = 0;
    UPROPERTY() uint16 FormedMembers = 0;
    // politika::stance::CombatStance and politika::stance::Adherence (FORM-F4).
    UPROPERTY() uint8 Stance = 0;
    UPROPERTY() uint8 Adherence = 0;
    // Sorted unit IDs.
    UPROPERTY() TArray<uint64> Members;
    bool operator==(const FPolitikaNetFormationInfo& Other) const
    {
        return FormationId == Other.FormationId && Shape == Other.Shape && State == Other.State
            && bHasDestination == Other.bHasDestination && Following == Other.Following && Regroups == Other.Regroups
            && FormedMembers == Other.FormedMembers && Stance == Other.Stance && Adherence == Other.Adherence
            && Members == Other.Members;
    }
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
    // Listen-host fog view: applies the host team's visibility and returns true only for a
    // change after the first application (the fogged start is not a sighting or a loss).
    bool ApplyLocalFogVisibility(bool bHidden);
    // Where the listen host last saw this unit (never its position while hidden).
    FVector GetLastLocalFogVisibleLocation() const { return LastLocalFogVisibleLocation; }
    // Observer-side cosmetic shot (a fog-filtered cue from the server); no rules state.
    void PresentObservedShot(const FVector& Point, float Cooldown);
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
    // Combat stance (FORM-F4). Owner-only replicated: an opponent sees what the unit does,
    // never its stance. The override is the unit's own setting; without one the unit fights
    // by its formation's stance, else by the default.
    politika::stance::CombatStance GetEffectiveStance() const { return static_cast<politika::stance::CombatStance>(EffectiveStance); }
    std::optional<politika::stance::CombatStance> GetStanceOverride() const
    {
        return StanceOverride < politika::stance::kStanceCount
            ? std::optional(static_cast<politika::stance::CombatStance>(StanceOverride)) : std::nullopt;
    }
    EPolitikaUnitAutonomy GetAutonomy() const { return Autonomy; }
    // Authority: records the unit's own stance setting (nullopt clears it).
    void SetStanceOverride(std::optional<politika::stance::CombatStance> Override);
    // Authority: applies the effective stance and, for a formation member, its formation's
    // adherence. Never changes the current order; an idle chase that the new stance no longer
    // allows ends, and a Hold Fire unit drops any target it picked by itself.
    void ApplyStance(politika::stance::CombatStance Effective, std::optional<politika::stance::Adherence> FormationAdherence);
    const politika::stance::IdleBehavior& GetIdleBehavior() const { return IdleBehavior; }
    // Resolve (DISC-D1): one attribute, shown as Discipline on military units and Morale on
    // civilians. Owner-only replicated; only the authority's fixed-step evaluation changes it.
    EPolitikaUnitCategory GetUnitCategory() const { return UnitCategory; }
    bool IsMilitary() const { return UnitCategory == EPolitikaUnitCategory::Military; }
    void SetUnitCategory(EPolitikaUnitCategory InCategory) { UnitCategory = InCategory; }
    int32 GetResolveMilli() const { return ResolveMilli; }
    politika::discipline::Trend GetResolveTrend() const { return static_cast<politika::discipline::Trend>(ResolveTrend); }
    // Authority: stores one evaluation's result.
    void ApplyResolve(const politika::discipline::Evaluation& Evaluation)
    {
        ResolveMilli = Evaluation.value;
        ResolveTrend = static_cast<uint8>(Evaluation.trend);
    }
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
    friend class FPolitikaMultiplayerFogMemoryTest;
    friend class FPolitikaMultiplayerGroupOrdersTest;
    friend class FPolitikaMultiplayerControlGroupsTest;
    friend class FPolitikaMultiplayerFormationLifecycleTest;
    friend class FPolitikaMultiplayerFormationStanceTest;
    friend class FPolitikaMultiplayerFormationDisciplineTest;
#endif
    UFUNCTION() void OnRep_ArchetypeId();
    UFUNCTION() void OnRep_Health();
    // Coordinator that spawned this unit (authority only); supplies fog/relevancy policy.
    TWeakObjectPtr<APolitikaPrototypeMatch> AuthorityMatch;
    float SightRadius = PolitikaDefaultSightRadiusCm;
    uint8 ObservedByTeams = 0;
    bool bLocalFogHidden = false;
    bool bLocalFogApplied = false;
    FVector LastLocalFogVisibleLocation = FVector::ZeroVector;
    // Public alliance (friend/foe presentation); authority is still server-side.
    UPROPERTY(Replicated) uint8 TeamId = 0;
    // Cosmetic fan-out of authoritative events. Authority already presented them
    // locally; remote observers only animate/play audio and never change rules state.
    // Shots carry a target point and are sent per connection under fog
    // (APolitikaPrototypeMatch::DistributeShotPresentation); ability cues carry none.
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
    // Networked authority keeps a dead unit this long so every observer that saw it receives
    // its death before the actor channel closes (otherwise it reads as a fog withdrawal).
    static constexpr float NetworkDeathRetireSeconds = 1.5f;
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
    // Nearest hostile, attackable, team-observed unit within RangeCm (zero: the archetype's
    // acquisition range). With a post, only targets that can be fired at from inside the leash.
    void AutoAcquireForAttackMove(float RangeCm = 0.0f, const FVector* Post = nullptr, double LeashCm = 0.0);

    // Stance (FORM-F4). EffectiveStance/StanceOverride/Autonomy are owner-only replicated for
    // the HUD; the post, leash state and behaviour are authority-only simulation state.
    UPROPERTY(Replicated) uint8 EffectiveStance = 0;
    // 255: no override.
    UPROPERTY(Replicated) uint8 StanceOverride = 255;
    UPROPERTY(Replicated) EPolitikaUnitAutonomy Autonomy = EPolitikaUnitAutonomy::Holding;
    // Resolve (DISC-D1), owner-only replicated for the HUD.
    UPROPERTY(Replicated) EPolitikaUnitCategory UnitCategory = EPolitikaUnitCategory::Military;
    UPROPERTY(Replicated) int32 ResolveMilli = politika::discipline::kBaseline;
    // politika::discipline::Trend of the latest evaluation.
    UPROPERTY(Replicated) uint8 ResolveTrend = 0;
    politika::stance::IdleBehavior IdleBehavior = politika::stance::BehaviorFor(politika::stance::kDefaultStance, std::nullopt);
    // Where the unit stood when it last became idle; chases are leashed to it.
    FVector Post = FVector::ZeroVector;
    bool bHasPost = false;
    // False only for members of a Free formation: they stay where an engagement ends.
    bool bReturnToPost = true;
    // A chase that found no route (or was obstructed) is not retried for 2 s (40 steps), so an
    // unreachable enemy cannot cause a path query every step.
    static constexpr uint64 ChaseRetryDelaySteps = 40;
    uint64 ChaseBlockedUntilStep = 0;
    float MaxPostDisplacement = 0.0f;
    // An explicit order ends any autonomous engagement and forgets the post.
    void ResetAutonomy();
    // Idle movement under the stance: chase inside the leash, or walk back to the post.
    void SimulateIdleAutonomy(float DeltaSeconds);
    // Ends an autonomous chase: back to the post when the stance and adherence ask for it.
    void EndEngagement(const TCHAR* Reason);
    // Shared route following for target-following orders and idle autonomy: (re)plans towards
    // Destination when the route is exhausted or the goal moved, bounded by a 0.5 s cooldown
    // and three attempts. Returns false when no route can be found.
    enum class ERouteStatus : uint8 { Ready, Waiting, Failed };
    ERouteStatus EnsureRouteTo(const FVector& Destination, float DeltaSeconds);
    // Advances along the current waypoints for one step; Obstructed/NavigationFailed or None.
    politika::rules::OrderEndReason AdvanceAlongRoute(float DeltaSeconds);
    uint64 AuthorityStep() const;
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
    // Local selection (§1-2): revalidated on every read, so dead, foreign and unbound units
    // drop out once. The first member (lowest stable ID) is the primary unit the HUD details.
    TArray<APolitikaPrototypeUnit*> GetSelectedUnitsForPlayer(uint32 PlayerId);
    APolitikaPrototypeUnit* GetSelectedUnitForPlayer(uint32 PlayerId);
    // Replaces the selection with the commandable members of Candidates, normalized once.
    void SetSelectionSet(uint32 PlayerId, TConstArrayView<APolitikaPrototypeUnit*> Candidates);
    void SetSelection(uint32 PlayerId, APolitikaPrototypeUnit* Unit);
    // The same ownership/participant check selection revalidation uses (§1).
    bool IsCommandableUnitForPlayer(const APolitikaPrototypeUnit* Unit, uint32 PlayerId) const { return IsCommandableUnit(Unit, PlayerId); }
    // Pointer adapter for the local controller and trusted test fixtures; it builds
    // the same versioned intent and calls SubmitControllerIntent.
    bool SubmitLocalControllerCommand(APlayerController* Source, APolitikaPrototypeUnit* Unit,
        APolitikaPrototypeUnit* Target, const politika::rules::Order& Intent);
    // The single server-side command intake for host-local and remote input. Authority
    // is derived from the connection binding; payload, freshness and entity IDs are
    // validated before the existing typed admission pipeline. Returns false only when
    // the source has no participant binding (no typed command is created).
    bool SubmitControllerIntent(APlayerController* Source, const FPolitikaNetCommandIntent& Intent);
    // Formation intake (FORM-F3), with the same binding, protocol, freshness and phase guards.
    // Form/Release change match-owned membership; a formation Move/AttackMove/Stop is planned
    // here (server navmesh) and submitted as one ordinary command per member. Returns false
    // only when the source has no participant binding.
    bool SubmitFormationIntent(APlayerController* Source, const FPolitikaNetFormationIntent& Intent);
    const politika::formation::Registry& GetFormations() const { return Formations; }
    // Advisory slot check shared by the client planner and the server's formation planner:
    // inside the playable area, on static ground level with GroundZ and not too steep, and on
    // the navmesh where this process has one.
    static bool IsGroundSlotOpen(const UWorld* World, const FVector2D& PlayableMin, const FVector2D& PlayableMax,
        const FVector2D& MapPosition, float GroundZ);
    bool IsLocalControllerBound(const APlayerController* Source) const;
    // Server connection lifecycle. A binding is the only source of command authority.
    bool BindController(APlayerController* Controller, FName& OutFailure);
    void HandleControllerLogout(AController* Exiting);
    bool CanAdmitRemoteParticipant(FName& OutReason) const;
    uint32 GetBoundParticipantId(const APlayerController* Controller) const;
    bool IsNetworkSession() const;
    // Standalone, listen (host occupies seat 0) or dedicated (every seat is remote).
    politika::rules::HostingMode GetHostingMode() const;
    bool IsDedicatedHosting() const { return GetHostingMode() == politika::rules::HostingMode::Dedicated; }
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
    // Shot presentation for one viewer participant: the exact target point only when the
    // viewer observes the target, otherwise a point along the shooter's facing, or nothing
    // when the shooter is unseen. Under fog, participant 0 (unbound) sees nothing.
    politika::rules::ShotCue ResolveShotCue(const APolitikaPrototypeUnit* Shooter, const APolitikaPrototypeUnit* Target,
        uint32 ViewerParticipant, FVector& OutPoint) const;
    // Server only: presents a shot locally (if this process has a viewer) and sends each
    // bound remote connection its own fog-filtered cue.
    void DistributeShotPresentation(APolitikaPrototypeUnit* Shooter, const APolitikaPrototypeUnit* Target, float Cooldown);
    // A muzzle stub, not a range: Attack approaches stop 100 cm inside weapon range (400 cm
    // or more for every current weapon), so the endpoint never sits on a target at a normal
    // engagement distance (audit F43; owner decision 29 Oct 2026).
    static constexpr float ShotFacingCueDistanceCm = 150.0f;
    // Relevancy query used by APolitikaPrototypeUnit::IsNetRelevantFor (server only).
    bool IsUnitRelevantToViewer(const APolitikaPrototypeUnit* Unit, const AActor* RealViewer) const;
    // Observer-side unit registry: clients only hold units their team currently observes.
    void RegisterObservedUnit(APolitikaPrototypeUnit* Unit);
    void UnregisterObservedUnit(APolitikaPrototypeUnit* Unit);
    // Viewer-side fog presentation (MP-2, audit F33): explored/visible shading and last-known
    // contacts, derived only from this process's own-team units and the contacts it was
    // sent. Never replicated and never consulted for authority; contacts are not targets.
    // Inactive offline, on a dedicated server and for an unseated viewer.
    bool IsLocalFogPresentationActive() const;
    const politika::rules::FogMap& GetLocalFog() const { return LocalFog; }
    const politika::rules::ContactMemory& GetLocalContacts() const { return LocalContacts; }
    float GetLocalFogGroundZ() const { return LocalFogGroundZ; }
    void NoteLocalContactWithdrawn(const APolitikaPrototypeUnit* Unit, bool bLastKnownAlive, const FVector& LastSeenLocation);
    void NoteLocalContactObserved(const APolitikaPrototypeUnit* Unit);
    static constexpr double LocalFogCellSizeCm = 125.0;
    static constexpr double LocalFogUpdateSeconds = 0.1;
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
    // Dedicated server session lifecycle: after results or a disconnect the server closes
    // the session (removing any remaining clients) and reloads the map for a fresh match
    // generation. It never travels to the shell menu and never exits on its own.
    void NoteNetworkResultCommitted();
    void ScheduleDedicatedSessionClose(FName Reason, float DelaySeconds);
    void CloseDedicatedSession();
    void RouteCommandOutcome(const politika::rules::CommandOutcomeEvent& Outcome);
    void RejectIntentAtIntake(APlayerController* Source, const FPolitikaNetCommandIntent& Intent,
        uint32 ParticipantId, politika::rules::OrderRejection Rejection);
    void ApplyMapPresentation();
    bool ResolveSessionLayout(FName& OutFailure);
    void UpdateTeamVisibility();
    void ApplyLocalFogPresentation();
    void UpdateLocalFogKnowledge();
    void ResetLocalFogKnowledge(const TCHAR* Reason);
    void ClearLocalContacts(const TCHAR* Reason);
    bool IsLocalContact(const APolitikaPrototypeUnit* Unit) const;
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
    uint32 ShotCueSequence = 0;
    bool bAwaitingNetworkParticipants = false;
    bool bObservedContentReady = false;
    bool bMapPresentationApplied = false;
    politika::rules::FogMap LocalFog;
    politika::rules::ContactMemory LocalContacts;
    uint64 LocalFogGeneration = 0;
    bool bLocalFogInitialized = false;
    double LastLocalFogSeconds = 0.0;
    double LastLocalFogLogSeconds = 0.0;
    float LocalFogGroundZ = 0.0f;
    FTimerHandle NetworkReturnTimer;
    FTimerHandle DedicatedSessionTimer;
    FName DedicatedCloseReason;
    bool bDedicatedSessionClosing = false;
#if !UE_BUILD_SHIPPING
    // Development multiplayer acceptance: server-side fixture actors and sampling.
    bool bMultiplayerAcceptance = false;
    bool bMultiplayerFixtureSpawned = false;
    uint64 LastMultiplayerSampleStep = 0;
    double LastObserverSampleSeconds = 0.0;
    void BeginMultiplayerAcceptance();
    void SampleMultiplayerState(const TCHAR* Reason);
    void SampleObservedState(const TCHAR* Reason);
    // Harness ground probes (-PolitikaMPGroundProbe=X:Y/X:Y): fixture map points this
    // viewer reports on its screen, so packaged runs can click exact positions.
    TArray<FVector2D> GroundProbePoints;
    bool bGroundProbesParsed = false;
    void SampleGroundProbes(const APlayerController& Viewer);
    // Development network measurements (F36): authoritative step cost and server frame time
    // per active match, plus the engine's own per-connection traffic counters.
    TArray<float> StepCostMicros;
    TArray<float> ServerFrameMillis;
    int32 CatchUpCapFrames = 0;
    double MeasurementStartSeconds = 0.0;
    double LastNetStatsSeconds = 0.0;
    double LastClientNetStatsSeconds = 0.0;
    bool bNetMeasurementsReported = false;
    bool bNetEmulationLogged = false;
    void LogNetworkEmulation();
    void SampleNetworkStats(const TCHAR* Reason);
    void SampleClientNetworkStats();
    void ReportNetworkMeasurements(const TCHAR* Reason);
    // Client frame time since the last client network sample (MP_CLIENT_FRAME_WINDOW).
    int32 ClientWindowFrames = 0;
    double ClientWindowTotalMillis = 0.0;
    float ClientWindowMaxMillis = 0.0f;
    // Development unit-scale fixture (-PolitikaMPUnitScale=N, MP-2 unit-scale budgets): N
    // units per seat on the multiplayer skirmish map, an idle window, then one server-issued
    // attack-move for every seated unit, with step cost split by phase. See
    // PolitikaUnitScaleFixture.cpp.
    enum class EUnitScalePhase : uint8 { Off, Idle, Engage };
    int32 UnitScalePerSeat = 0;
    // -PolitikaMPUnitScaleHold: keep the fixture idle (no engagement order), so packaged
    // interaction checks can command the scaled armies themselves.
    bool bUnitScaleHold = false;
    EUnitScalePhase UnitScalePhase = EUnitScalePhase::Off;
    uint64 UnitScaleActivationStep = 0;
    int32 UnitScaleEngageIndex = INDEX_NONE;
    int32 UnitScaleWindowIndex = 0;
    double LastUnitScaleWindowSeconds = 0.0;
    uint64 UnitScaleFirstOrderSequence = 0;
    uint64 UnitScaleLastOrderSequence = 0;
    int32 UnitScaleOrdersIssued = 0;
    int32 UnitScaleOrdersAccepted = 0;
    int32 UnitScaleOrdersRejected = 0;
    bool bUnitScaleOrdersReported = false;
    int32 UnitScaleDestroyed = 0;
    // Adds the scaled placements after spatial validation; false fails admission.
    bool ResolveUnitScale(TArray<struct FPolitikaPrototypeUnitPlacement>& InOutPlacements, FName& OutFailure);
    void BeginUnitScaleMeasurement();
    void TickUnitScaleDriver();
    void ObserveUnitScaleEvent(const politika::rules::GameplayEvent& Event);
    void ReportUnitScaleMeasurements(const TCHAR* Reason);
#endif
#if WITH_DEV_AUTOMATION_TESTS
    friend class FPolitikaMultiplayerParticipantBindingTest;
    friend class FPolitikaMultiplayerCommandIntakeTest;
    friend class FPolitikaMultiplayerAuthorityOnlySimulationTest;
    friend class FPolitikaMultiplayerResultAndCleanupTest;
    friend class FPolitikaMultiplayerFogOfWarTest;
    friend class FPolitikaMultiplayerDedicatedHostingTest;
    friend class FPolitikaMultiplayerShotCueRedactionTest;
    friend struct FPolitikaMultiplayerFixture;
    friend class FPolitikaMultiplayerFogMemoryTest;
    friend class FPolitikaMultiplayerGroupOrdersTest;
    friend class FPolitikaMultiplayerGroupMoveShapeTest;
    friend class FPolitikaMultiplayerControlGroupsTest;
    // Automation-only: treat a listen-mode test world as a dedicated server (no local seat).
    bool bTestDedicatedHosting = false;
    // Automation-only: record each shot cue decision instead of sending the client RPC
    // (fixture controllers have no connection). Participant 0 marks this process's view.
    struct FTestShotCue { uint32 Participant = 0; politika::rules::ShotCue Cue = politika::rules::ShotCue::None; FVector Point; };
    bool bTestCaptureShotCues = false;
    TArray<FTestShotCue> TestShotCues;
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
        // The formation whose move or stop produced this member command; zero for every
        // individual order. Admission of a zero-formation order makes a member independent.
        uint32 FormationId = 0;
    };
    // Match-owned formations (FORM-F3). Only SubmitFormationIntent, member loss, admitted
    // member orders and lifecycle cleanup change it; see PolitikaFormations.cpp.
    politika::formation::Registry Formations;
    bool bFormationsDirty = false;
    void HandleFormationChange(const politika::formation::Change& Change, const TCHAR* Cause);
    void RejectFormationIntent(APlayerController* Source, const FPolitikaNetFormationIntent& Intent, uint32 ParticipantId,
        politika::formation::FormationRejection Rejection);
    void SendFormationOutcome(APlayerController* Source, const FPolitikaNetFormationIntent& Intent,
        politika::formation::FormationRejection Rejection, uint32 FormationId, int32 Members, int32 Orders);
    // Drops members that no longer exist or are no longer the owner's (before planning).
    void PruneFormationMembers();
    // Submits one ordinary Move/AttackMove per planned member; with a source, every outcome is
    // routed back to it under the request's correlation. Returns the commands submitted.
    int32 SubmitFormationOrders(const politika::formation::Formation& Formation, const politika::formation::MovePlan& Plan,
        APlayerController* Source, const FPolitikaNetFormationIntent* Intent, const TCHAR* Cause);
    bool FormationAnchorGround(const politika::formation::Point& Anchor, float& OutGroundZ) const;
    void ProcessFormationRegroups();
    void NoteFormationOrderAdmitted(const APolitikaPrototypeUnit* Unit, const FPendingOrder& Pending);
    void ClearFormations(const TCHAR* Reason);
    void SyncFormationReplication();
#if !UE_BUILD_SHIPPING
    // Development fixture (-PolitikaMPFormationCasualty): the first formation move loses its
    // highest-ID following member 1.5 s into transit, so packaged runs can check regrouping.
    int32 FormationCasualtyArmed = -1;
    uint32 FormationCasualtyFormation = 0;
    uint64 FormationCasualtyStep = 0;
    void TickFormationCasualtyFixture();
    // Development fixture (-PolitikaMPStanceProbe, FORM-F4): when a participant's formation is
    // first set to Guard, the opponent's lowest-ID unit is moved (an ordinary Move) to a point
    // 740 cm from the nearest member (inside acquisition, outside weapon range); when it is
    // first set to Hold Fire, the next one is moved to 450 cm (inside weapon range).
    int32 StanceProbeArmed = -1;
    bool bStanceProbeGuardSent = false;
    bool bStanceProbeHoldFireSent = false;
    TSet<uint64> StanceProbeUnits;
    void TriggerStanceProbe(uint32 ParticipantId, const politika::formation::Formation& Formation);
#endif
#if WITH_DEV_AUTOMATION_TESTS
    TFunction<bool(const FVector2D&)> FormationSlotProbeForTest;
    // Every formation outcome sent, with its destination controller (fixture controllers have
    // no connection for the client RPC).
    TArray<TPair<TWeakObjectPtr<APlayerController>, FPolitikaNetFormationOutcome>> TestFormationOutcomes;
    friend class FPolitikaMultiplayerFormationLifecycleTest;
    friend class FPolitikaMultiplayerFormationStanceTest;
    friend class FPolitikaMultiplayerFormationDisciplineTest;
#endif
    // Combat stance (FORM-F4). A unit's effective stance is its override, else its formation's
    // stance, else the default. Membership changes never change it: StanceSnapshot records it
    // before a registry change and PreserveStances re-derives each override afterwards.
    void RefreshUnitStance(APolitikaPrototypeUnit* Unit);
    void RefreshFormationStances(politika::formation::FormationId Formation);
    TMap<uint64, politika::stance::CombatStance> StanceSnapshot(TConstArrayView<uint64> ExtraIds = {}) const;
    void PreserveStances(const TMap<uint64, politika::stance::CombatStance>& Before);
    // Member loss with the stance of a dissolved formation's survivor preserved.
    void LoseFormationMember(uint64 UnitId, politika::formation::LeaveReason Reason, const TCHAR* Cause);
    bool SubmitStanceIntent(APlayerController* Source, const FPolitikaNetFormationIntent& Intent, uint32 ParticipantId);
    // Resolve (DISC-D1): every kEvaluationIntervalSteps, each formation member that follows its
    // formation with enough fellow members close by earns Discipline up to the ceiling; other
    // members hold; every unit outside a formation fades back to the baseline. Runs after the
    // step's movement and casualties, so support uses this step's positions and survivors.
    void UpdateUnitResolve();
    // Authority: every live match unit. Observer: only units its connection currently
    // receives (fog-filtered); membership follows unit BeginPlay/EndPlay. Not replicated,
    // so the array size cannot reveal hidden enemy counts.
    UPROPERTY() TArray<TObjectPtr<APolitikaPrototypeUnit>> Units;
    UPROPERTY(ReplicatedUsing=OnRep_Objective) TObjectPtr<APolitikaPrototypeObjective> Objective;
    UPROPERTY() TMap<uint32, FPolitikaSelectionSet> SelectionByPlayer;
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
        TOptional<uint64> GenerationHint = {}, uint32 FormationId = 0);
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
    friend class FPolitikaMultiplayerFogMemoryTest;
    friend class FPolitikaMultiplayerGroupOrdersTest;
    friend class FPolitikaMultiplayerControlGroupsTest;
    friend class FPolitikaMultiplayerGroupMoveShapeTest;
    friend class FPolitikaMultiplayerFormationLifecycleTest;
    friend class FPolitikaMultiplayerFormationStanceTest;
    friend class FPolitikaMultiplayerFormationDisciplineTest;
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
    // Formation ingress and result (FORM-F3); host-local calls execute directly on authority.
    UFUNCTION(Server, Reliable) void ServerSubmitFormationIntent(const FPolitikaNetFormationIntent& Intent);
    UFUNCTION(Client, Reliable) void ClientReceiveFormationOutcome(const FPolitikaNetFormationOutcome& Outcome);
    // Owner-only view of this participant's own formations (server-written).
    const TArray<FPolitikaNetFormationInfo>& GetOwnFormations() const { return OwnFormations; }
    const FPolitikaNetFormationInfo* FindOwnFormationOf(uint64 UnitId) const;
    // The formation whose whole membership is exactly this selection, or null.
    const FPolitikaNetFormationInfo* SelectedWholeFormation(TConstArrayView<APolitikaPrototypeUnit*> Selected) const;
    void SetOwnFormations(TArray<FPolitikaNetFormationInfo>&& Formations);
    // Fog-filtered cosmetic shot cue for this connection only. Point is the target when
    // this participant's team observes it, otherwise a point along the shooter's facing.
    UFUNCTION(Client, Unreliable) void ClientShotPresentation(APolitikaPrototypeUnit* Shooter, FVector_NetQuantize Point,
        bool bTargetObserved, float Cooldown, uint32 Sequence);
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
    UPROPERTY(ReplicatedUsing=OnRep_Assignment) uint32 AssignedRuntimePlayerId = 0;
    UPROPERTY(Replicated) int32 AssignedSlotId = 0;
    UPROPERTY(Replicated) FName AssignedFactionId;
    UPROPERTY(Replicated) uint64 AssignedMatchGeneration = 0;
    UFUNCTION() void OnRep_Assignment();
    UPROPERTY(ReplicatedUsing=OnRep_OwnFormations) TArray<FPolitikaNetFormationInfo> OwnFormations;
    UFUNCTION() void OnRep_OwnFormations();
    void LogOwnFormations(const TCHAR* Source) const;
    uint32 NextClientSequence = 1;
    uint32 NextCorrelationId = 1;
    bool SubmitIntent(APolitikaPrototypeMatch* ActiveMatch, APolitikaPrototypeUnit* Unit,
        APolitikaPrototypeUnit* Target, const politika::rules::Order& Order);
    void ShowLocalFeedback(const FString& Feedback);
public:
    // HUD read-only view of an in-progress box selection, in viewport pixels.
    bool GetSelectionBox(FVector2D& OutMin, FVector2D& OutMax) const;
    // HUD read-only view of the numbered control groups (4C-G) and the active Tab subgroup.
    const politika::selection::ControlGroups& GetControlGroups() const { return ControlGroups; }
    int32 GetActiveSubgroup() const { return ActiveSubgroup; }
    // HUD read-only view of the group-move shape (FORM-F2) and the latest planned slots.
    politika::formation::Shape GetGroupMoveShape() const { return GroupMoveShape; }
    // World positions of the latest group move's destinations while they are still fresh.
    bool GetGroupMoveMarkers(TArray<FVector>& OutMarkers, float& OutAlpha) const;
private:
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
    // Pointer selection (§2C): Idle -> PendingClick -> ClickRelease | BoxSelecting | Cancelled.
    // One stroke applies one normalized selection operation; nothing here issues an order.
    bool bSelectPressed = false;
    bool bBoxSelecting = false;
    FVector2D SelectPressPosition = FVector2D::ZeroVector;
    FVector2D SelectDragPosition = FVector2D::ZeroVector;
    // Real time at which the button was first seen up without its release binding; -1 when down.
    double SelectReleaseSeenSeconds = -1.0;
    bool IsViewportFocused() const;
    void BeginSelectPress();
    void EndSelectPress();
    void CancelSelectPress(const TCHAR* Reason);
    void UpdateSelectDrag();
    politika::selection::SelectionOp ModifierSelectionOp(bool bBox) const;
    void SelectUnderCursor(politika::selection::SelectionOp Op);
    void BoxSelect(const FVector2D& CornerA, const FVector2D& CornerB, politika::selection::SelectionOp Op);
    void ApplySelection(TConstArrayView<APolitikaPrototypeUnit*> Hits, politika::selection::SelectionOp Op, const TCHAR* Source);
    void SelectUnitForInput(APolitikaPrototypeUnit* Unit);
    // Ctrl+click or double-click: own units of the clicked archetype that are on screen now.
    void SelectSameArchetype(APolitikaPrototypeUnit* Unit, const TCHAR* Source);
    uint64 LastClickUnit = 0;
    double LastClickSeconds = -1.0;
    // Numbered control groups (4C-G, §2B/§2D): local membership snapshots of stable IDs for
    // this participant and match generation. They never issue an order or grant authority.
    politika::selection::ControlGroups ControlGroups;
    politika::selection::RecallTapTracker RecallTaps;
    // Binds the slots to the current participant and generation and drops dead or lost
    // members. Returns false (with the slots cleared) when no active bound match exists.
    bool SyncControlGroups(APolitikaPrototypeMatch* ActiveMatch);
    void ControlGroupKey(int32 Slot);
    void ControlGroupInput(int32 Slot, const TCHAR* Source);
    // Ctrl replaces, Shift adds, Alt moves; no modifier recalls. Callers sync the slots first.
    void ApplyControlGroupInput(APolitikaPrototypeMatch* ActiveMatch, int32 Slot, bool bCtrl, bool bShift, bool bAlt,
        const TCHAR* Source);
    void RecallControlGroup(APolitikaPrototypeMatch* ActiveMatch, int32 Slot, bool bFocus, const TCHAR* Source);
    void FocusCameraOn(APolitikaPrototypeMatch* ActiveMatch, TConstArrayView<APolitikaPrototypeUnit*> Units, int32 Slot);
    // The control-group bar slot under the cursor, or INDEX_NONE; the bar captures clicks.
    int32 GroupBarSlotUnderCursor() const;
    // Tab/Shift+Tab: the active archetype subgroup that receives grenade, reload and sprint.
    int32 ActiveSubgroup = 0;
    void CycleSubgroup();
    TArray<APolitikaPrototypeUnit*> GetActiveSubgroupUnits(APolitikaPrototypeMatch* ActiveMatch);
    // The revalidated selection; empty unless this controller is bound.
    TArray<APolitikaPrototypeUnit*> GetCommandableSelectedUnits(APolitikaPrototypeMatch* ActiveMatch);
    APolitikaPrototypeUnit* GetCommandableSelectedUnit(APolitikaPrototypeMatch* ActiveMatch);
    // Group order (§3): one ordinary single-issuer intent per unit, admitted independently;
    // the batch only tallies outcomes for feedback. No group atomicity.
    struct FCommandBatch
    {
        uint32 Id = 0;
        uint8 OrderKind = 0;
        uint32 FirstCorrelation = 0;
        uint32 LastCorrelation = 0;
        politika::selection::BatchTally<politika::rules::kOrderRejectionCount> Tally;
    };
    TArray<FCommandBatch> CommandBatches;
    uint32 NextBatchId = 1;
    // Move and attack-move groups get one distinct destination per member (FORM-F2): pass the
    // clicked ground point as GroundHit. Without it every member gets the same order.
    void SubmitGroupIntent(APolitikaPrototypeMatch* ActiveMatch, TConstArrayView<APolitikaPrototypeUnit*> Issuers,
        APolitikaPrototypeUnit* Target, const politika::rules::Order& Order, const FVector* GroundHit = nullptr);
    // Group-move geometry (FORM-F2): local presentation choice of Line or Column, cycled by V.
    // It is not a formation: no membership, stance or bonus exists, and the server admits and
    // routes every member's destination as an ordinary Move.
    politika::formation::Shape GroupMoveShape = politika::formation::Shape::Line;
    void CycleGroupMoveShape();
    // Advisory check of one candidate slot on this process: inside the playable area, on
    // ground level with the clicked point and not too steep, and on the local navmesh when
    // this process has one. The server's own projection and route stay authoritative.
    bool IsGroupMoveSlotOpen(const APolitikaPrototypeMatch& ActiveMatch, const FVector2D& MapPosition, float GroundZ) const;
#if WITH_DEV_AUTOMATION_TESTS
    TFunction<bool(const FVector2D&)> GroupMoveSlotProbeForTest;
#endif
    TArray<FVector> GroupMoveMarkers;
    double GroupMoveMarkersSeconds = -1.0;
    static constexpr double GroupMoveMarkerLifetimeSeconds = 2.5;
    bool RecordBatchOutcome(const FPolitikaNetCommandOutcome& Outcome);
    // Formations (FORM-F3): T forms the selection with the current V shape (re-forming moves
    // members out of their old formation); Y releases the selected members. When the selection
    // is exactly one whole formation, RMB/Q/X address that formation instead of the units.
    void FormSelected();
    void ReleaseSelected();
    // Stances (FORM-F4): H cycles the combat stance of the selection (a whole formation sets the
    // formation's stance, any other selection sets each unit's own); J cycles a whole
    // formation's adherence. Neither sends an order.
    void CycleStanceSelected();
    void CycleAdherenceSelected();
    bool SubmitFormationRequest(APolitikaPrototypeMatch* ActiveMatch, politika::formation::FormationOp Op,
        uint32 FormationId, TConstArrayView<APolitikaPrototypeUnit*> Members, const politika::rules::Position& Destination,
        const FVector* GroundHit = nullptr, uint8 Value = 0);
    // Pending formation move/stop correlations and their order kind, for the outcome batch.
    TMap<uint32, uint8> PendingFormationOrderKinds;
    // Requested stance/adherence per pending correlation, for the outcome feedback.
    TMap<uint32, uint8> PendingStanceValues;
    void ClearSelectionState();
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
    static constexpr float FogDarknessExplored = 0.45f;
    static constexpr float FogDarknessUnexplored = 0.78f;
    static constexpr double FogSkirtCm = 30000.0;
    // Only the owning controller's own correlated outcomes reach this HUD.
    void ShowCommandOutcome(const FPolitikaNetCommandOutcome& Outcome);
    void ShowLocalFeedback(const FString& Feedback) { CommandFeedback = Feedback; }
    // Player-facing text for an OrderRejection code.
    static FString RejectionText(uint8 Rejection);
    // Canvas text and panels scale with the viewport width (1920 px = 1).
    static float UiScaleFor(float ViewWidth) { return FMath::Clamp(ViewWidth / 1920.0f, 1.0f, 2.0f); }
private:
    UPROPERTY() TWeakObjectPtr<APolitikaPrototypeMatch> Match;
    FString CommandFeedback;
    void DrawLocalFog(APlayerController& Controller, const APolitikaPrototypeMatch& ActiveMatch);
    void DrawLastKnownContacts(APlayerController& Controller, const APolitikaPrototypeMatch& ActiveMatch, float UiScale);
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
