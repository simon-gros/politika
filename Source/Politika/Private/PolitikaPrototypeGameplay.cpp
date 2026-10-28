#include "PolitikaPrototypeGameplay.h"

#include "Camera/CameraComponent.h"
#include "Components/SceneComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/DirectionalLight.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerState.h"
#include "TimerManager.h"
#include "InputCoreTypes.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "UObject/ConstructorHelpers.h"
#include "PolitikaGameInstance.h"
#include "PolitikaPrototypeContent.h"
#include "Rules.h"
#include "NavigationSystem.h"
#include "NavigationData.h"
#include "HAL/PlatformTime.h"
#if !UE_BUILD_SHIPPING
#include "PolitikaNavigationObstacleFixture.h"
#endif

namespace
{
constexpr float UnitGroundZ = 42.0f;
uint64 NextPrototypeMatchGeneration = 1;

UStaticMesh* LoadPrototypeMesh(const TCHAR* Path)
{
    return LoadObject<UStaticMesh>(nullptr, Path);
}

void ApplyPrototypeColor(UStaticMeshComponent* Component, const FLinearColor& Color)
{
    if (!Component) return;
    UMaterialInterface* Base = LoadObject<UMaterialInterface>(nullptr,
        TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
    if (Base)
    {
        UMaterialInstanceDynamic* Material = UMaterialInstanceDynamic::Create(Base, Component);
        if (Material)
        {
            Material->SetVectorParameterValue(TEXT("Color"), Color);
            Component->SetMaterial(0, Material);
        }
    }
}

float FlatDistance(const FVector& A, const FVector& B)
{
    return FVector::Dist2D(A, B);
}

politika::rules::ExecutionOrder ToExecutionOrder(EPolitikaPrototypeOrder Order)
{
    switch (Order)
    {
        case EPolitikaPrototypeOrder::Move: return politika::rules::ExecutionOrder::Move;
        case EPolitikaPrototypeOrder::Attack: return politika::rules::ExecutionOrder::Attack;
        case EPolitikaPrototypeOrder::AttackMove: return politika::rules::ExecutionOrder::AttackMove;
        case EPolitikaPrototypeOrder::Guard: return politika::rules::ExecutionOrder::Guard;
        case EPolitikaPrototypeOrder::Idle: return politika::rules::ExecutionOrder::Idle;
        case EPolitikaPrototypeOrder::Stop: return politika::rules::ExecutionOrder::Stop;
    }
    return politika::rules::ExecutionOrder::Idle;
}
}

APolitikaPrototypeUnit::APolitikaPrototypeUnit()
{
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderMesh(TEXT("/Engine/BasicShapes/Cylinder"));
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> ShapeMaterial(TEXT("/Engine/BasicShapes/BasicShapeMaterial"));
    PrimaryActorTick.bCanEverTick = false;
    Body = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Body"));
    SetRootComponent(Body);
    Body->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    Body->SetCollisionObjectType(ECC_WorldDynamic);
    Body->SetCollisionResponseToAllChannels(ECR_Ignore);
    Body->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
    Body->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
    Body->SetGenerateOverlapEvents(false);
    if (CubeMesh.Succeeded()) Body->SetStaticMesh(CubeMesh.Object);
    if (ShapeMaterial.Succeeded()) Body->SetMaterial(0, ShapeMaterial.Object);
    SelectionIndicator = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SelectionIndicator"));
    SelectionIndicator->SetupAttachment(Body);
    SelectionIndicator->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    SelectionIndicator->SetVisibility(false, true);
    if (CylinderMesh.Succeeded()) SelectionIndicator->SetStaticMesh(CylinderMesh.Object);
    if (ShapeMaterial.Succeeded()) SelectionIndicator->SetMaterial(0, ShapeMaterial.Object);
}

void APolitikaPrototypeUnit::Configure(uint64 InStableId, uint32 InOwner, const FText& InName,
    int32 InHealth, float InMoveSpeed, float InWeaponRange, float InDamage, float InCooldown,
    bool bInAttackable, UStaticMesh* InMesh, const FVector& InMeshScale, const FLinearColor& InColor,
    FName InFactionId, bool bInUseAuthoredMeshMaterials)
{
    StableId = InStableId;
    OwnerPlayer = InOwner;
    FactionId = InFactionId;
    UnitName = InName;
    MaxHealth = InHealth;
    Health = MaxHealth;
    MoveSpeed = InMoveSpeed;
    WeaponRange = InWeaponRange;
    WeaponDamage = InDamage;
    WeaponCooldown = InCooldown;
    bAttackable = bInAttackable;
    Body->SetStaticMesh(InMesh);
    SetActorScale3D(InMeshScale);
    if (bInUseAuthoredMeshMaterials)
    {
        // The constructor sets an Engine BasicShapes material override. Clear it so
        // Unreal uses the imported static mesh material/texture in cooked playtests.
        Body->SetMaterial(0, nullptr);
    }
    else
    {
        ApplyPrototypeColor(Body, InColor);
    }
    SelectionIndicator->SetStaticMesh(LoadPrototypeMesh(TEXT("/Engine/BasicShapes/Cylinder.Cylinder")));
    SelectionIndicator->SetRelativeLocation(FVector(0, 0, -34.0f / InMeshScale.Z));
    SelectionIndicator->SetRelativeScale3D(FVector(2.1f / InMeshScale.X, 2.1f / InMeshScale.Y, 0.025f / InMeshScale.Z));
    ApplyPrototypeColor(SelectionIndicator, FLinearColor(0.96f, 0.83f, 0.30f));
    SetActorEnableCollision(true);
    NavigationWaypoints.Reset();
    NavigationWaypointIndex = 0;
    NavigationMatchGeneration = 0;
    NavigationOrderSequence = 0;
    NavigationReplanCooldownSeconds = 0.0f;
    NavigationReplanAttempts = 0;
}

void APolitikaPrototypeUnit::SetMoveOrder(EPolitikaPrototypeOrder Kind, const FVector& Destination,
    const FVector& ResolvedDestination, TArray<FVector>&& Route)
{
    CurrentOrder = Kind;
    OrderDestination = Destination;
    ResolvedNavigationDestination = ResolvedDestination;
    NavigationWaypoints = MoveTemp(Route);
    NavigationWaypointIndex = NavigationWaypoints.Num() > 1 ? 1 : 0;
    NavigationReplanCooldownSeconds = 0.0f;
    NavigationReplanAttempts = 0;
    OrderTarget.Reset();
#if !UE_BUILD_SHIPPING
    bAcceptanceMovementLogged = false;
#endif
}

void APolitikaPrototypeUnit::SetAttackOrder(APolitikaPrototypeUnit* Target)
{
    NavigationWaypoints.Reset();
    NavigationWaypointIndex = 0;
    NavigationMatchGeneration = 0;
    NavigationOrderSequence = 0;
    NavigationReplanCooldownSeconds = 0.0f;
    NavigationReplanAttempts = 0;
    CurrentOrder = EPolitikaPrototypeOrder::Attack;
    OrderTarget = Target;
#if !UE_BUILD_SHIPPING
    bAcceptanceMovementLogged = false;
#endif
}

void APolitikaPrototypeUnit::SetGuardOrder()
{
    NavigationWaypoints.Reset();
    NavigationWaypointIndex = 0;
    NavigationMatchGeneration = 0;
    NavigationOrderSequence = 0;
    NavigationReplanCooldownSeconds = 0.0f;
    NavigationReplanAttempts = 0;
    CurrentOrder = EPolitikaPrototypeOrder::Guard;
    OrderTarget.Reset();
#if !UE_BUILD_SHIPPING
    bAcceptanceMovementLogged = false;
#endif
}

void APolitikaPrototypeUnit::StopOrder()
{
    CurrentOrder = EPolitikaPrototypeOrder::Stop;
    NavigationWaypoints.Reset();
    NavigationWaypointIndex = 0;
    NavigationMatchGeneration = 0;
    NavigationOrderSequence = 0;
    NavigationReplanCooldownSeconds = 0.0f;
    NavigationReplanAttempts = 0;
    OrderTarget.Reset();
    OrderDestination = GetActorLocation();
}

void APolitikaPrototypeUnit::SetSelectedVisual(bool bSelected)
{
    SelectionIndicator->SetVisibility(bSelected, true);
}

politika::rules::OrderEndReason APolitikaPrototypeUnit::SimulateMovement(float DeltaSeconds)
{
    if (!IsAlive() || MoveSpeed <= 0.0f) return politika::rules::OrderEndReason::None;
    const bool bCanAdvance = CurrentOrder == EPolitikaPrototypeOrder::Move
        || CurrentOrder == EPolitikaPrototypeOrder::AttackMove
        || ((CurrentOrder == EPolitikaPrototypeOrder::Attack || CurrentOrder == EPolitikaPrototypeOrder::Guard)
            && OrderTarget.IsValid());
    if (!bCanAdvance) return politika::rules::OrderEndReason::None;

    FVector Destination = OrderDestination;
    bool bTargetAlreadyInRange = false;
    if ((CurrentOrder == EPolitikaPrototypeOrder::Attack || CurrentOrder == EPolitikaPrototypeOrder::Guard)
        && OrderTarget.IsValid())
    {
        Destination = OrderTarget->GetActorLocation();
        const FVector ToTarget = Destination - GetActorLocation();
        if (ToTarget.Size2D() <= WeaponRange)
        {
            bTargetAlreadyInRange = true;
        }
        else
        {
            // Navigation projection may shift a goal by up to 80 cm and route
            // consumption tolerates 2 cm. Aim inside the firing radius rather
            // than repeatedly replanning to an endpoint just outside range.
            Destination -= ToTarget.GetSafeNormal2D() * FMath::Max(0.0f, WeaponRange - 100.0f);
        }
    }
    if (bTargetAlreadyInRange) return politika::rules::OrderEndReason::None;
    // Target-following orders resolve a route only when the current route is
    // exhausted or the target has materially moved. Never turn them back into
    // direct-line movement or query every 20 Hz step.
    if (CurrentOrder == EPolitikaPrototypeOrder::Attack || CurrentOrder == EPolitikaPrototypeOrder::Guard)
    {
        NavigationReplanCooldownSeconds = FMath::Max(0.0f, NavigationReplanCooldownSeconds - DeltaSeconds);
        const bool bTargetMoved = FlatDistance(ResolvedNavigationDestination, Destination) > 100.0f;
        if (bTargetMoved)
        {
            NavigationWaypoints.Reset();
            NavigationWaypointIndex = 0;
            NavigationReplanAttempts = 0;
            NavigationReplanCooldownSeconds = 0.0f;
        }
        if (NavigationWaypoints.IsEmpty() || NavigationWaypointIndex >= NavigationWaypoints.Num())
        {
            if (NavigationReplanCooldownSeconds > 0.0f) return politika::rules::OrderEndReason::None;
            if (NavigationReplanAttempts >= 3)
            {
                StopOrder();
                return politika::rules::OrderEndReason::NavigationFailed;
            }
            ++NavigationReplanAttempts;
            NavigationReplanCooldownSeconds = 0.5f;
            UNavigationSystemV1* NavSystem = GetWorld()
                ? FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld()) : nullptr;
            ANavigationData* NavData = NavSystem
                ? NavSystem->GetDefaultNavDataInstance(FNavigationSystem::DontCreate) : nullptr;
            FNavLocation ProjectedStart, ProjectedGoal;
            const bool bProjected = NavSystem && NavData && !NavSystem->IsNavigationBuildInProgress()
                && NavSystem->ProjectPointToNavigation(GetActorLocation(), ProjectedStart,
                    FVector(80.0f, 80.0f, 180.0f), NavData)
                && NavSystem->ProjectPointToNavigation(Destination, ProjectedGoal,
                    FVector(80.0f, 80.0f, 180.0f), NavData)
                && FlatDistance(ProjectedGoal.Location, Destination) <= 80.0f;
            bool bRouteReady = false;
            if (bProjected)
            {
                FPathFindingQuery Query(this, *NavData, ProjectedStart.Location, ProjectedGoal.Location);
                Query.SetAllowPartialPaths(false);
                const double QueryStartSeconds = FPlatformTime::Seconds();
                const FPathFindingResult Result = NavSystem->FindPathSync(Query);
#if !UE_BUILD_SHIPPING
                UE_LOG(LogPolitikaStartup, Display,
                    TEXT("RTS_NAV_TARGET_QUERY generation=%llu unit=%llu sequence=%llu result=%u partial=%d points=%d duration_us=%.1f attempt=%u"),
                    NavigationMatchGeneration, StableId, NavigationOrderSequence,
                    static_cast<uint32>(Result.Result), Result.IsPartial() ? 1 : 0,
                    Result.Path.IsValid() ? Result.Path->GetPathPoints().Num() : 0,
                    (FPlatformTime::Seconds() - QueryStartSeconds) * 1000000.0, NavigationReplanAttempts);
#endif
                if (Result.Result == ENavigationQueryResult::Success && Result.Path.IsValid()
                    && !Result.Path->IsPartial() && Result.Path->GetPathPoints().Num() > 0)
                {
                    NavigationWaypoints.Reset();
                    for (const FNavPathPoint& Point : Result.Path->GetPathPoints())
                    {
                        if (!FMath::IsFinite(Point.Location.X) || !FMath::IsFinite(Point.Location.Y)
                            || !FMath::IsFinite(Point.Location.Z)) { NavigationWaypoints.Reset(); break; }
                        NavigationWaypoints.Add(Point.Location
                            + FVector(0.0, 0.0, GetActorLocation().Z - ProjectedStart.Location.Z));
                    }
                    NavigationWaypointIndex = NavigationWaypoints.Num() > 1 ? 1 : 0;
                    const FVector VerticalOffset(0.0, 0.0, GetActorLocation().Z - ProjectedStart.Location.Z);
                    ResolvedNavigationDestination = ProjectedGoal.Location + VerticalOffset;
                    bRouteReady = !NavigationWaypoints.IsEmpty()
                        && FVector::Dist(NavigationWaypoints.Last(), ResolvedNavigationDestination) <= 150.0f;
                }
            }
            if (!bRouteReady)
            {
                StopOrder();
                return politika::rules::OrderEndReason::NavigationFailed;
            }
        }
    }

    if (NavigationWaypoints.IsEmpty() || NavigationWaypointIndex >= NavigationWaypoints.Num())
    {
        StopOrder();
        return politika::rules::OrderEndReason::NavigationFailed;
    }

    float RemainingDistance = MoveSpeed * DeltaSeconds;
    int32 WaypointsConsumed = 0;
    const FVector PreviousLocation = GetActorLocation();
    while (RemainingDistance > KINDA_SMALL_NUMBER && NavigationWaypointIndex < NavigationWaypoints.Num()
        && WaypointsConsumed < 16)
    {
        const FVector Waypoint = NavigationWaypoints[NavigationWaypointIndex];
        const FVector Current = GetActorLocation();
        FVector Offset = Waypoint - Current;
        const float SegmentDistance = Offset.Size();
        if (!FMath::IsFinite(SegmentDistance))
        {
            StopOrder();
            return politika::rules::OrderEndReason::NavigationFailed;
        }
        if (SegmentDistance <= 2.0f)
        {
            ++NavigationWaypointIndex;
            ++WaypointsConsumed;
            continue;
        }
        const float Travel = FMath::Min(RemainingDistance, SegmentDistance);
        const FVector Next = Current + Offset.GetSafeNormal() * Travel;
        FHitResult Hit;
        SetActorLocation(Next, true, &Hit, ETeleportType::None);
#if !UE_BUILD_SHIPPING
        if (bTraceNavigationTraversal) NavigationStepTraversal.Add(GetActorLocation());
#endif
        if (Hit.bBlockingHit)
        {
            StopOrder();
            return politika::rules::OrderEndReason::Obstructed;
        }
        RemainingDistance -= Travel;
        ++WaypointsConsumed;
        if (Travel + 2.0f >= SegmentDistance) ++NavigationWaypointIndex;
    }
#if !UE_BUILD_SHIPPING
    if (!bAcceptanceMovementLogged && FlatDistance(PreviousLocation, GetActorLocation()) > 1.0f)
    {
        const TCHAR* OrderName = CurrentOrder == EPolitikaPrototypeOrder::AttackMove ? TEXT("AttackMove")
            : CurrentOrder == EPolitikaPrototypeOrder::Attack ? TEXT("Attack")
            : CurrentOrder == EPolitikaPrototypeOrder::Guard ? TEXT("Guard") : TEXT("Move");
        UE_LOG(LogPolitikaStartup, Display, TEXT("RTS_POSITION_UPDATED order=%s unit=%llu x=%.1f y=%.1f"),
            OrderName, StableId, GetActorLocation().X, GetActorLocation().Y);
        bAcceptanceMovementLogged = true;
    }
#endif
    if ((CurrentOrder == EPolitikaPrototypeOrder::Move || CurrentOrder == EPolitikaPrototypeOrder::AttackMove)
        && NavigationWaypointIndex >= NavigationWaypoints.Num()
        && FlatDistance(GetActorLocation(), ResolvedNavigationDestination) <= 18.0f)
    {
        CurrentOrder = EPolitikaPrototypeOrder::Idle;
        NavigationWaypoints.Reset();
        NavigationWaypointIndex = 0;
        return politika::rules::OrderEndReason::Arrived;
    }
    return politika::rules::OrderEndReason::None;
}

void APolitikaPrototypeUnit::AutoAcquireForAttackMove()
{
    APolitikaPrototypeUnit* Nearest = nullptr;
    float NearestDistance = 800.0f;
    for (TActorIterator<APolitikaPrototypeUnit> It(GetWorld()); It; ++It)
    {
        APolitikaPrototypeUnit* Candidate = *It;
        if (!Candidate || Candidate == this || !Candidate->IsAlive()
            || Candidate->GetOwnerPlayer() == OwnerPlayer || !Candidate->IsAttackable()) continue;
        const float Distance = FlatDistance(GetActorLocation(), Candidate->GetActorLocation());
        if (Distance < NearestDistance)
        {
            NearestDistance = Distance;
            Nearest = Candidate;
        }
    }
    OrderTarget = Nearest;
}

void APolitikaPrototypeUnit::ResolveCombat(float DeltaSeconds)
{
    if (!IsAlive()) return;
    CooldownRemaining = FMath::Max(0.0f, CooldownRemaining - DeltaSeconds);
    if (CurrentOrder == EPolitikaPrototypeOrder::AttackMove && !OrderTarget.IsValid())
    {
        AutoAcquireForAttackMove();
    }
    else if (CurrentOrder == EPolitikaPrototypeOrder::Guard && !OrderTarget.IsValid())
    {
        AutoAcquireForAttackMove();
    }

    APolitikaPrototypeUnit* Target = OrderTarget.Get();
    if (!Target || !Target->IsAlive() || Target->GetOwnerPlayer() == OwnerPlayer)
    {
        if (CurrentOrder == EPolitikaPrototypeOrder::Attack) CurrentOrder = EPolitikaPrototypeOrder::Idle;
        OrderTarget.Reset();
        return;
    }
    if (FlatDistance(GetActorLocation(), Target->GetActorLocation()) <= WeaponRange && CooldownRemaining <= 0.0f)
    {
        Target->PendingDamage += WeaponDamage;
        CooldownRemaining = WeaponCooldown;
    }
}

int32 APolitikaPrototypeUnit::ApplyPendingDamage()
{
    if (PendingDamage <= 0.0f || !IsAlive()) return 0;
    const int32 HealthBefore = Health;
    Health = FMath::Max(0, Health - FMath::CeilToInt(PendingDamage));
    PendingDamage = 0.0f;
    const int32 AppliedDamage = HealthBefore - Health;
#if !UE_BUILD_SHIPPING
    if (Health != HealthBefore)
        UE_LOG(LogPolitikaStartup, Display, TEXT("RTS_HEALTH_CHANGED unit=%llu before=%d after=%d"),
            StableId, HealthBefore, Health);
#endif
    if (Health == 0) StopOrder();
    return AppliedDamage;
}

uint64 APolitikaPrototypeUnit::CleanupDeadTarget()
{
    if (APolitikaPrototypeUnit* Target = OrderTarget.Get(); Target && !Target->IsAlive())
    {
        const uint64 LostTargetId = Target->GetStableId();
        OrderTarget.Reset();
        if (CurrentOrder == EPolitikaPrototypeOrder::Attack) CurrentOrder = EPolitikaPrototypeOrder::Idle;
        return LostTargetId;
    }
    return 0;
}

APolitikaPrototypeObjective::APolitikaPrototypeObjective()
{
    PrimaryActorTick.bCanEverTick = false;
    Marker = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ControlPointMarker"));
    SetRootComponent(Marker);
    Marker->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void APolitikaPrototypeObjective::ConfigureFromDefinition(const FPolitikaPrototypeObjectiveDefinition& Definition, uint32 InLocalPlayerId)
{
    CaptureRadius = Definition.CaptureRadiusCm;
    CaptureDuration = Definition.CaptureDurationSeconds;
    LocalRuntimePlayerId = InLocalPlayerId;
    SetActorLocation(Definition.Location);
    Marker->SetStaticMesh(Definition.MarkerMesh);
    Marker->SetWorldScale3D(Definition.MarkerScale);
    ApplyPrototypeColor(Marker, Definition.MarkerTint);
}

void APolitikaPrototypeObjective::UpdateCapture(const TArray<APolitikaPrototypeUnit*>& Units, float DeltaSeconds)
{
    if (bComplete) return;
#if !UE_BUILD_SHIPPING
    const bool bWasContested = bContested;
    const bool bWasComplete = bComplete;
    const bool bWasCapturing = Progress > 0.0f;
#endif
    bool bPlayerPresent = false;
    bool bEnemyPresent = false;
    for (const APolitikaPrototypeUnit* Unit : Units)
    {
        if (!Unit || !Unit->IsAlive() || FlatDistance(Unit->GetActorLocation(), GetActorLocation()) > CaptureRadius) continue;
        bPlayerPresent |= Unit->GetOwnerPlayer() == LocalRuntimePlayerId;
        bEnemyPresent |= Unit->GetOwnerPlayer() != LocalRuntimePlayerId;
    }
    bContested = bPlayerPresent && bEnemyPresent;
    if (bPlayerPresent && !bEnemyPresent)
    {
        Progress = FMath::Min(1.0f, Progress + DeltaSeconds / CaptureDuration);
        bComplete = Progress >= 1.0f;
    }
#if !UE_BUILD_SHIPPING
    if (bContested != bWasContested || bComplete != bWasComplete || (!bWasCapturing && Progress > 0.0f))
        UE_LOG(LogPolitikaStartup, Display, TEXT("RTS_OBJECTIVE_STATE contested=%d progress_milli=%d complete=%d"),
            bContested ? 1 : 0, FMath::RoundToInt(Progress * 1000.0f), bComplete ? 1 : 0);
#endif
}

FText APolitikaPrototypeObjective::GetStatusText() const
{
    if (bComplete) return FText::FromString(TEXT("SECURED"));
    if (bContested) return FText::FromString(TEXT("CONTESTED"));
    if (Progress > 0.0f) return FText::FromString(FString::Printf(TEXT("CAPTURING %d%%"), FMath::RoundToInt(Progress * 100.0f)));
    return FText::FromString(TEXT("NEUTRAL"));
}

APolitikaPrototypeMatch::APolitikaPrototypeMatch()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickGroup = TG_PrePhysics;
}

void APolitikaPrototypeMatch::BeginPlay()
{
    Super::BeginPlay();
    MatchGeneration = NextPrototypeMatchGeneration++;
    BuildScenario();
#if !UE_BUILD_SHIPPING
    if (bContentReady) BeginAcceptanceHarness();
#endif
}

bool APolitikaPrototypeMatch::BuildRuntimeParticipantsFromSlots(
    const UPolitikaPrototypeMapDefinition& Definition,
    TArray<FPolitikaPrototypeRuntimeParticipant>& OutParticipants,
    uint32& OutLocalRuntimePlayerId, FName& OutFailureCode)
{
    OutParticipants.Reset();
    OutLocalRuntimePlayerId = 0;
    OutFailureCode = NAME_None;
    if (Definition.PlayerSlots.Num() < 2)
    {
        OutFailureCode = TEXT("MAP.SLOTS.INSUFFICIENT");
        return false;
    }
    int32 LocalHumanCount = 0;
    TSet<uint32> ParticipantIds;
    TSet<int32> SlotIds;
    TSet<FName> FactionIds;
    for (const FPolitikaPrototypePlayerSlot& Slot : Definition.PlayerSlots)
    {
        if (Slot.SlotId <= 0 || Slot.RuntimePlayerId <= 0 || Slot.FactionId.IsNone()
            || Slot.FactionId == FName(TEXT("faction.unaligned"))
            || SlotIds.Contains(Slot.SlotId) || ParticipantIds.Contains(static_cast<uint32>(Slot.RuntimePlayerId))
            || FactionIds.Contains(Slot.FactionId))
        {
            OutParticipants.Reset();
            OutLocalRuntimePlayerId = 0;
            OutFailureCode = Slot.FactionId == FName(TEXT("faction.unaligned"))
                ? FName(TEXT("MAP.FACTION.UNALIGNED_PLAYABLE")) : FName(TEXT("MAP.PARTICIPANT.MAPPING_INVALID"));
            return false;
        }
        SlotIds.Add(Slot.SlotId);
        ParticipantIds.Add(static_cast<uint32>(Slot.RuntimePlayerId));
        FactionIds.Add(Slot.FactionId);
        FPolitikaPrototypeRuntimeParticipant& Participant = OutParticipants.AddDefaulted_GetRef();
        Participant.SlotId = Slot.SlotId;
        Participant.RuntimePlayerId = static_cast<uint32>(Slot.RuntimePlayerId);
        Participant.FactionId = Slot.FactionId;
        Participant.ControllerKind = Slot.bHumanControlled
            ? EPolitikaPrototypeControllerKind::LocalHuman : EPolitikaPrototypeControllerKind::AssignedOpponent;
        if (Slot.bHumanControlled)
        {
            ++LocalHumanCount;
            OutLocalRuntimePlayerId = Participant.RuntimePlayerId;
        }
    }
    if (LocalHumanCount != 1)
    {
        OutParticipants.Reset();
        OutLocalRuntimePlayerId = 0;
        OutFailureCode = TEXT("MAP.SLOTS.HUMAN_COUNT");
        return false;
    }
    return true;
}

bool APolitikaPrototypeMatch::TransitionLifecycle(politika::rules::MatchPhase NextPhase)
{
    const politika::rules::MatchPhase Previous = Lifecycle.phase;
    if (!Lifecycle.TransitionTo(NextPhase))
    {
        UE_LOG(LogPolitikaStartup, Error, TEXT("MATCH_LIFECYCLE_TRANSITION_REJECTED from=%u to=%u"),
            static_cast<uint32>(Previous), static_cast<uint32>(NextPhase));
        return false;
    }
    if (NextPhase != politika::rules::MatchPhase::Active)
    {
        // A queued request belongs to the phase in which it was submitted. Never
        // let terminal handling, teardown, or recovery carry it into later work.
        PendingOrders.Reset();
        StepAccumulator = 0.0f;
        if (Previous == politika::rules::MatchPhase::Active)
            for (APolitikaPrototypeUnit* Unit : Units)
                if (Unit && Unit->CanMove()) Unit->StopOrder();
    }
    UE_LOG(LogPolitikaStartup, Display, TEXT("MATCH_LIFECYCLE phase=%u"), static_cast<uint32>(NextPhase));
    return true;
}

void APolitikaPrototypeMatch::BuildScenario()
{
    LastAdmissionDiagnostics.Reset();
    FName RequestedMapId = UPolitikaPrototypeContentValidator::PrototypeMapAssetId;
#if !UE_BUILD_SHIPPING
    FString FixtureId;
    UPolitikaGameInstance* GameInstance = GetWorld() ? GetWorld()->GetGameInstance<UPolitikaGameInstance>() : nullptr;
    if (GameInstance && GameInstance->ConsumeDevelopmentTestFixture(FixtureId))
    {
        if (FixtureId == TEXT("InvalidMapSchema")) RequestedMapId = TEXT("map.test.invalid_schema");
        else if (FixtureId == TEXT("InvalidParticipantFaction")) RequestedMapId = TEXT("map.test.invalid_participant_faction");
        else if (FixtureId == TEXT("StaticObstacleDetour")) RequestedMapId = TEXT("map.test.static_obstacle_detour");
        else
        {
            UE_LOG(LogPolitikaStartup, Error, TEXT("MATCH_ADMISSION_SELECTION_FAILED fixture=%s reason=unknown_fixture"), *FixtureId);
            LastCommandFeedback = FString::Printf(TEXT("Test fixture '%s' is unknown; no match initialized."), *FixtureId);
            RecoverToShellAfterAdmissionFailure(FName(*FixtureId));
            return;
        }
    }
#endif
    FPolitikaPrototypeContentSnapshot Snapshot;
    TArray<FPolitikaContentDiagnostic> Diagnostics;
    if (!FPolitikaPrototypeContentLoader::LoadAndValidate(GetWorld(), RequestedMapId, Snapshot, Diagnostics))
    {
        LastAdmissionDiagnostics = Diagnostics;
        for (const FPolitikaContentDiagnostic& Diagnostic : Diagnostics)
            UE_LOG(LogPolitikaStartup, Error, TEXT("CONTENT_VALIDATION %s"), *UPolitikaPrototypeContentValidator::FormatDiagnostic(Diagnostic));
        UE_LOG(LogPolitikaStartup, Error, TEXT("MATCH_ADMISSION_REJECTED fixture=%s units=0 objective=0 simulation_started=0"), *RequestedMapId.ToString());
        LastCommandFeedback = TEXT("Prototype setup failed: gameplay content validation failed. See CONTENT_VALIDATION diagnostics.");
        RecoverToShellAfterAdmissionFailure(RequestedMapId);
        return;
    }
    ContentCatalog = Snapshot.Catalog;
    MapDefinition = Snapshot.MapDefinition;
    PlayableMin = MapDefinition->PlayableMin;
    PlayableMax = MapDefinition->PlayableMax;
    TArray<FPolitikaPrototypeRuntimeParticipant> CandidateParticipants;
    uint32 CandidateLocalRuntimePlayerId = 0;
    FName ParticipantFailureCode;
    if (!BuildRuntimeParticipantsFromSlots(*MapDefinition, CandidateParticipants,
        CandidateLocalRuntimePlayerId, ParticipantFailureCode))
    {
        LastCommandFeedback = TEXT("Prototype setup failed: participant, controller, or playable-faction assignments are invalid.");
        UE_LOG(LogPolitikaStartup, Error, TEXT("MATCH_ADMISSION_REJECTED map=%s reason=%s participants=%d"),
            *RequestedMapId.ToString(), *ParticipantFailureCode.ToString(), CandidateParticipants.Num());
        RecoverToShellAfterAdmissionFailure(RequestedMapId);
        return;
    }
    RuntimeParticipants = MoveTemp(CandidateParticipants);
    LocalRuntimePlayerId = CandidateLocalRuntimePlayerId;
    UnalignedSystemAuthority = {FName(TEXT("faction.unaligned")), true};
    const FPolitikaPrototypeRuntimeParticipant* LocalParticipant = RuntimeParticipants.FindByPredicate(
        [this](const FPolitikaPrototypeRuntimeParticipant& Participant)
        { return Participant.RuntimePlayerId == LocalRuntimePlayerId; });
    UE_LOG(LogPolitikaStartup, Display,
        TEXT("MATCH_PARTICIPANTS_ADMITTED map=%s playable=%d local_player=%u local_faction=%s unaligned_system=1"),
        *RequestedMapId.ToString(), RuntimeParticipants.Num(), LocalRuntimePlayerId,
        LocalParticipant ? *LocalParticipant->FactionId.ToString() : TEXT("invalid"));

    Diagnostics.Reset();
    if (!UPolitikaPrototypeContentValidator::ValidateMapSpatial(GetWorld(), MapDefinition, Diagnostics))
    {
        LastAdmissionDiagnostics = Diagnostics;
        for (const FPolitikaContentDiagnostic& Diagnostic : Diagnostics)
            UE_LOG(LogPolitikaStartup, Error, TEXT("CONTENT_VALIDATION %s"), *UPolitikaPrototypeContentValidator::FormatDiagnostic(Diagnostic));
        LastCommandFeedback = TEXT("Prototype setup failed: map traversal validation failed. See CONTENT_VALIDATION diagnostics.");
        RecoverToShellAfterAdmissionFailure(RequestedMapId);
        return;
    }
    if (!TransitionLifecycle(politika::rules::MatchPhase::SetupValidated)
        || !TransitionLifecycle(politika::rules::MatchPhase::Loading))
    {
        LastCommandFeedback = TEXT("Prototype setup failed: invalid match lifecycle transition.");
        RecoverToShellAfterAdmissionFailure(RequestedMapId);
        return;
    }
    UE_LOG(LogPolitikaStartup, Display, TEXT("CONTENT_VALIDATION_PASSED map=%s movement=%s"),
        *MapDefinition->StableId.ToString(), *MapDefinition->MovementProfileId.ToString());

    for (TActorIterator<AStaticMeshActor> It(GetWorld()); It; ++It)
    {
        if (It->ActorHasTag(MapDefinition->GroundActorTag))
        {
            ApplyPrototypeColor(It->GetStaticMeshComponent(), MapDefinition->Lighting.GroundTint);
            break;
        }
    }
    if (ADirectionalLight* Sun = GetWorld()->SpawnActor<ADirectionalLight>(MapDefinition->Lighting.SunLocation, MapDefinition->Lighting.SunRotation))
    {
        Sun->GetLightComponent()->SetMobility(EComponentMobility::Movable);
        Sun->GetLightComponent()->SetIntensity(MapDefinition->Lighting.SunIntensity);
        Sun->GetLightComponent()->SetLightColor(MapDefinition->Lighting.SunColor);
    }

    auto SpawnUnit = [this](const FPolitikaPrototypeUnitPlacement& Placement)
    {
        const UPolitikaUnitArchetypeDefinition* Definition = ContentCatalog->FindUnit(Placement.UnitArchetypeId);
        const UPolitikaWeaponDefinition* Weapon = Definition ? ContentCatalog->FindWeapon(Definition->WeaponId) : nullptr;
        const FPolitikaPrototypeRuntimeParticipant* Slot = RuntimeParticipants.FindByPredicate([&](const FPolitikaPrototypeRuntimeParticipant& Candidate) { return Candidate.SlotId == Placement.SlotId; });
        if (!Definition || !Weapon || !Slot) return;
        APolitikaPrototypeUnit* Unit = GetWorld()->SpawnActor<APolitikaPrototypeUnit>(Placement.Location, Placement.Rotation);
        if (!Unit) return;
        Unit->Configure(NextStableId++, Slot->RuntimePlayerId, Definition->DisplayName, Definition->MaxHealth,
            Definition->MovementSpeedCmPerSecond, Weapon->RangeCm, Weapon->Damage, Weapon->CooldownSeconds,
            Definition->bAttackable, Definition->VisualMesh, Definition->VisualScale, Definition->FactionTint,
            Slot->FactionId, Definition->bUseAuthoredMeshMaterials);
        Units.Add(Unit);
    };

    for (const FPolitikaPrototypeUnitPlacement& Placement : MapDefinition->UnitPlacements) SpawnUnit(Placement);

    Objective = GetWorld()->SpawnActor<APolitikaPrototypeObjective>(MapDefinition->Objective.Location, FRotator::ZeroRotator);
    if (Objective) Objective->ConfigureFromDefinition(MapDefinition->Objective, LocalRuntimePlayerId);
    bContentReady = Units.Num() == MapDefinition->UnitPlacements.Num() && Objective != nullptr;
    if (!bContentReady)
    {
        for (APolitikaPrototypeUnit* Unit : Units) if (Unit) Unit->Destroy();
        Units.Reset();
        if (Objective) Objective->Destroy();
        Objective = nullptr;
        LastCommandFeedback = TEXT("Prototype setup failed: validated content could not be instantiated.");
        UE_LOG(LogPolitikaStartup, Error, TEXT("CONTENT_INITIALIZATION_FAILED map=%s expected_units=%d actual_units=%d"), *MapDefinition->StableId.ToString(), MapDefinition->UnitPlacements.Num(), Units.Num());
        RecoverToShellAfterAdmissionFailure(RequestedMapId);
        return;
    }
    if (!TransitionLifecycle(politika::rules::MatchPhase::Starting)
        || !TransitionLifecycle(politika::rules::MatchPhase::Active))
    {
        LastCommandFeedback = TEXT("Prototype setup failed: match could not enter active play.");
        RecoverToShellAfterAdmissionFailure(RequestedMapId);
        return;
    }
#if !UE_BUILD_SHIPPING
    for (int32 Index = 0; Index < MapDefinition->UnitPlacements.Num(); ++Index)
    {
        const FPolitikaPrototypeUnitPlacement& Placement = MapDefinition->UnitPlacements[Index];
        const UPolitikaUnitArchetypeDefinition* Definition = ContentCatalog->FindUnit(Placement.UnitArchetypeId);
        const UPolitikaWeaponDefinition* Weapon = Definition ? ContentCatalog->FindWeapon(Definition->WeaponId) : nullptr;
        const FPolitikaPrototypeRuntimeParticipant* Slot = RuntimeParticipants.FindByPredicate(
            [&Placement](const FPolitikaPrototypeRuntimeParticipant& Candidate) { return Candidate.SlotId == Placement.SlotId; });
        const APolitikaPrototypeUnit* Unit = Units.IsValidIndex(Index) ? Units[Index] : nullptr;
        if (Unit && Definition && Weapon && Slot)
        {
            const FVector Location = Unit->GetActorLocation();
            UE_LOG(LogPolitikaStartup, Display,
                TEXT("MATCH_AUTHORITATIVE_UNIT index=%d entity=%llu archetype=%s owner=%u faction=%s slot=%d location=%.1f,%.1f,%.1f health=%d speed=%.1f weapon_ref=%s weapon_resolved=%s weapon_range=%.1f weapon_damage=%.1f weapon_cooldown=%.2f attackable=%d"),
                Index, Unit->GetStableId(), *Definition->StableId.ToString(), Unit->GetOwnerPlayer(),
                *Unit->GetFactionId().ToString(), Slot->SlotId,
                Location.X, Location.Y, Location.Z, Definition->MaxHealth, Definition->MovementSpeedCmPerSecond,
                *Definition->WeaponId.ToString(), *Weapon->StableId.ToString(), Weapon->RangeCm, Weapon->Damage,
                Weapon->CooldownSeconds, Definition->bAttackable ? 1 : 0);
        }
    }
    const FVector ObjectiveLocation = Objective->GetActorLocation();
    UE_LOG(LogPolitikaStartup, Display,
        TEXT("MATCH_AUTHORITATIVE_OBJECTIVE id=%s type=%d location=%.1f,%.1f,%.1f radius=%.1f duration=%.2f"),
        *MapDefinition->Objective.StableId.ToString(), static_cast<int32>(MapDefinition->Objective.Type),
        ObjectiveLocation.X, ObjectiveLocation.Y, ObjectiveLocation.Z, MapDefinition->Objective.CaptureRadiusCm,
        MapDefinition->Objective.CaptureDurationSeconds);
    UE_LOG(LogPolitikaStartup, Display, TEXT("MATCH_AUTHORITATIVE_SNAPSHOT_COMPLETE units=%d objectives=%d"), Units.Num(), Objective ? 1 : 0);
#endif
    UE_LOG(LogPolitikaStartup, Display, TEXT("MATCH_CONTENT_READY map=%s units=%d objective=%s"),
        *MapDefinition->StableId.ToString(), Units.Num(), *MapDefinition->Objective.StableId.ToString());
}

void APolitikaPrototypeMatch::RecoverToShellAfterAdmissionFailure(FName MapId)
{
    for (APolitikaPrototypeUnit* Unit : Units) if (Unit) Unit->Destroy();
    Units.Reset();
    if (Objective) Objective->Destroy();
    Objective = nullptr;
    RuntimeParticipants.Reset();
    LocalRuntimePlayerId = 0;
    UnalignedSystemAuthority = {};
    SelectionByPlayer.Reset();
    bContentReady = false;
    PendingOrders.Reset();
    if (Lifecycle.phase != politika::rules::MatchPhase::AdmissionRejected)
        TransitionLifecycle(politika::rules::MatchPhase::AdmissionRejected);
    UWorld* World = GetWorld();
    if (!World) return;
    if (UPolitikaGameInstance* Game = World->GetGameInstance<UPolitikaGameInstance>())
        Game->ReportMatchAdmissionRejected(MapId);

    int32 WorldUnitCount = 0;
    int32 WorldObjectiveCount = 0;
    for (TActorIterator<APolitikaPrototypeUnit> It(World); It; ++It) ++WorldUnitCount;
    for (TActorIterator<APolitikaPrototypeObjective> It(World); It; ++It) ++WorldObjectiveCount;
#if !UE_BUILD_SHIPPING
    UE_LOG(LogPolitikaStartup, Display,
        TEXT("MATCH_ADMISSION_STATE map=%s participants=%d local_player=%u unaligned_system=%d units=%d objectives=%d selection=%d pending_commands=%d accepted_sequences=%llu simulation_steps=%llu content_ready=%d terminal_result=%d"),
        *MapId.ToString(), RuntimeParticipants.Num(), LocalRuntimePlayerId,
        UnalignedSystemAuthority.bNpcSystemControlled ? 1 : 0, WorldUnitCount, WorldObjectiveCount,
        SelectionByPlayer.Num(), PendingOrders.Num(), NextCommandSequence - 1, SimulationStepsExecuted,
        bContentReady ? 1 : 0, CommittedResult.IsSet() ? 1 : 0);
#else
    UE_LOG(LogPolitikaStartup, Display,
        TEXT("MATCH_ADMISSION_STATE map=%s units=%d objectives=%d commands=%llu simulation_steps=%llu content_ready=%d"),
        *MapId.ToString(), WorldUnitCount, WorldObjectiveCount, NextCommandSequence - 1,
        SimulationStepsExecuted, bContentReady ? 1 : 0);
#endif

    World->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateWeakLambda(this, [this]
    {
        UWorld* RecoveryWorld = GetWorld();
        APlayerController* Player = RecoveryWorld ? RecoveryWorld->GetFirstPlayerController() : nullptr;
        if (UPolitikaGameInstance* Game = RecoveryWorld ? RecoveryWorld->GetGameInstance<UPolitikaGameInstance>() : nullptr)
            Game->ReturnToMenu(Player);
    }));
}

void APolitikaPrototypeMatch::BeginReturnToMenu()
{
    PendingOrders.Reset();
    if (Lifecycle.phase == politika::rules::MatchPhase::Active
        || Lifecycle.phase == politika::rules::MatchPhase::Results)
        TransitionLifecycle(politika::rules::MatchPhase::Returning);
    if (Lifecycle.phase == politika::rules::MatchPhase::Returning)
    {
        if (APlayerController* Player = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr)
            if (APolitikaPrototypePlayerController* PrototypePlayer = Cast<APolitikaPrototypePlayerController>(Player))
            {
                PrototypePlayer->SelectedUnit.Reset();
                PrototypePlayer->bMapCameraApplied = false;
            }
        SelectionByPlayer.Reset();
        RuntimeParticipants.Reset();
        LocalRuntimePlayerId = 0;
        UnalignedSystemAuthority = {};
        bContentReady = false;
        for (APolitikaPrototypeUnit* Unit : Units) if (Unit) Unit->Destroy();
        Units.Reset();
        if (Objective) Objective->Destroy();
        Objective = nullptr;
    }
}

void APolitikaPrototypeMatch::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    if (!bContentReady || !Lifecycle.IsActive()) return;
#if !UE_BUILD_SHIPPING
    const bool bScheduled = NavigationAcceptance.bRunning && NavigationAcceptance.Schedule != 0;
    const uint64 FirstStep = SimulationStepsExecuted;
    if (bScheduled)
        DeltaSeconds = PolitikaNavigationObstacleFixture::ScheduleDelta(
            NavigationAcceptance.Schedule, NavigationAcceptance.OuterUpdates);
#endif
    StepAccumulator += FMath::Min(DeltaSeconds, 0.25f);
    int32 Steps = 0;
    if (StepAccumulator >= SimulationStepSeconds) PublishedEvents.Reset();
    while (Lifecycle.IsActive() && StepAccumulator >= SimulationStepSeconds && Steps < 5)
    {
        PendingGameplayEvents.Reset();
        AdmitPendingOrders();
        RunSimulationStep(SimulationStepSeconds);
        StepAccumulator -= SimulationStepSeconds;
        ++Steps;
#if !UE_BUILD_SHIPPING
        // Stop capture exactly at arrival + 20, including multi-step updates.
        // No additional scheduler: the ordinary Tick loop is the only driver.
        if (bScheduled && !NavigationAcceptance.bRunning) break;
#endif
    }
    if (!Lifecycle.IsActive()) StepAccumulator = 0.0f;
    if (Steps == 5 && StepAccumulator >= SimulationStepSeconds) StepAccumulator = 0.0f;
#if !UE_BUILD_SHIPPING
    if (bScheduled)
    {
        UE_LOG(LogPolitikaStartup, Display,
            TEXT("NAV_REPEATABILITY_UPDATE ordinal=%llu delta=%.9f clamped=%.9f steps=%d first=%llu last=%llu accumulator=%.9f cap_reached=%d"),
            ++NavigationAcceptance.OuterUpdates, DeltaSeconds, FMath::Min(DeltaSeconds, 0.25f), Steps,
            FirstStep, SimulationStepsExecuted, StepAccumulator, Steps == 5 ? 1 : 0);
    }
    TickAcceptanceHarness(DeltaSeconds);
#endif
}

#if !UE_BUILD_SHIPPING
void APolitikaPrototypeMatch::SetAcceptanceMove(APolitikaPrototypeUnit* Unit, const FVector& Destination)
{
    TArray<FVector> Route;
    FVector Resolved = Destination;
    FString Failure;
    if (ResolveNavigationRoute(Unit, Destination, Route, Resolved, Failure, 0))
    {
        Unit->SetMoveOrder(EPolitikaPrototypeOrder::Move, Destination, Resolved, MoveTemp(Route));
        Unit->NavigationMatchGeneration = MatchGeneration;
    }
    else
    {
        Unit->StopOrder();
        UE_LOG(LogPolitikaStartup, Error, TEXT("RTS_ACCEPTANCE_NAVIGATION_FAILED reason=%s"), *Failure);
    }
}

void APolitikaPrototypeMatch::BeginAcceptanceHarness()
{
    if (MapDefinition && MapDefinition->StableId == TEXT("map.test.static_obstacle_detour"))
    {
        BeginNavigationAcceptance();
        return;
    }
    FParse::Value(FCommandLine::Get(), TEXT("RTSAcceptance="), AcceptanceMode);
    AcceptanceMode = AcceptanceMode.ToLower();
    if (AcceptanceMode != TEXT("capturepause") && AcceptanceMode != TEXT("attackerdeath")
        && AcceptanceMode != TEXT("combined") && AcceptanceMode != TEXT("objectivecompletion")) return;

    UE_LOG(LogTemp, Display, TEXT("RTS_ACCEPTANCE_BEGIN mode=%s"), *AcceptanceMode);
    if (AcceptanceMode == TEXT("objectivecompletion"))
    {
        APolitikaPrototypeUnit* Capturer = Units.IsEmpty() ? nullptr : Units[0];
        APolitikaPrototypeUnit* Opponent = Units.Num() >= 3 ? Units[2] : nullptr;
        if (!Capturer || !Opponent || !Objective)
        {
            UE_LOG(LogTemp, Error, TEXT("RTS_ACCEPTANCE_OBJECTIVE_COMPLETION result=FAIL reason=fixture_missing"));
            AcceptancePhase = EAcceptancePhase::Done;
            return;
        }
        // Development acceptance only: arrange a legitimate uncontested control-point
        // condition and shorten its configured capture duration. Completion still runs
        // through UpdateCapture, the authoritative coordinator and normal event publish.
        Capturer->SetActorLocation(Objective->GetActorLocation());
        Capturer->StopOrder();
        Opponent->SetActorLocation(FVector(1500.0f, 1500.0f, UnitGroundZ));
        Objective->SetAcceptanceCaptureDuration(0.1f);
        UE_LOG(LogTemp, Display, TEXT("RTS_ACCEPTANCE_OBJECTIVE_COMPLETION armed eligible_player=%u uncontested=1 duration=0.1"), LocalRuntimePlayerId);
        return;
    }
    if (AcceptanceMode == TEXT("capturepause") || AcceptanceMode == TEXT("combined"))
    {
        APolitikaPrototypeUnit* Capturer = Units.IsEmpty() ? nullptr : Units[0];
        if (APolitikaPrototypeUnit* OpposingPatrol = Units.Num() >= 3 ? Units[2] : nullptr)
            OpposingPatrol->SetActorLocation(FVector(1500.0f, 1500.0f, UnitGroundZ));
        if (!Capturer || !Objective)
        {
            UE_LOG(LogTemp, Error, TEXT("RTS_ACCEPTANCE_CAPTURE_PAUSE result=FAIL reason=fixture_missing"));
            AcceptancePhase = EAcceptancePhase::Done;
            return;
        }
        SetAcceptanceMove(Capturer, Objective->GetActorLocation());
        AcceptancePhase = EAcceptancePhase::CaptureApproach;
        LastCommandFeedback = TEXT("Acceptance fixture: moving into the control point.");
    }
    else
    {
        AcceptancePhase = EAcceptancePhase::CombatSetup;
        AcceptanceElapsed = 0.0f;
    }
}

void APolitikaPrototypeMatch::StartAttackerDeathFixture()
{
    if (Units.Num() < 3)
    {
        UE_LOG(LogTemp, Error, TEXT("RTS_ACCEPTANCE_ATTACKER_DEATH result=FAIL reason=fixture_missing"));
        AcceptancePhase = EAcceptancePhase::Done;
        return;
    }

    APolitikaPrototypeUnit* Attacker = Units[0];
    APolitikaPrototypeUnit* Defender = Units[2];
    if (!Attacker || !Defender)
    {
        UE_LOG(LogTemp, Error, TEXT("RTS_ACCEPTANCE_ATTACKER_DEATH result=FAIL reason=fixture_invalid"));
        AcceptancePhase = EAcceptancePhase::Done;
        return;
    }

    // Test fixture only: put both entities in normal simulation range and give
    // the friendly attacker a deliberately low starting health. Damage, death,
    // order cleanup and entity removal still run through the production phases.
    Attacker->SetActorLocation(FVector(-220.0f, 150.0f, UnitGroundZ));
    Defender->SetActorLocation(FVector(220.0f, 150.0f, UnitGroundZ));
    Attacker->SetAcceptanceHealth(10);
    Attacker->SetAttackOrder(Defender);
    Defender->SetGuardOrder();
    AcceptanceAttacker = Attacker;
    AcceptanceDefender = Defender;
    bAcceptanceAttackerOrderCleared = false;
    AcceptancePhase = EAcceptancePhase::CombatResolve;
    AcceptanceElapsed = 0.0f;
    LastCommandFeedback = TEXT("Acceptance fixture: friendly attacker engaged; opposing patrol returning fire.");
    UE_LOG(LogTemp, Display, TEXT("RTS_ACCEPTANCE_ATTACKER_DEATH started attacker_order=Attack attacker_health=10 defender_order=Guard distance=440"));
}

void APolitikaPrototypeMatch::TickAcceptanceHarness(float DeltaSeconds)
{
    if (AcceptancePhase == EAcceptancePhase::None || AcceptancePhase == EAcceptancePhase::Done) return;
    AcceptanceElapsed += DeltaSeconds;

    if (AcceptancePhase == EAcceptancePhase::CombatSetup)
    {
        if (AcceptanceElapsed >= 1.0f) StartAttackerDeathFixture();
        return;
    }
    if (AcceptancePhase == EAcceptancePhase::CombatResolve)
    {
        APolitikaPrototypeUnit* Defender = AcceptanceDefender.Get();
        if (!AcceptanceAttacker.IsValid())
        {
            const bool bDefenderSurvived = Defender && Defender->IsAlive();
            const bool bDefenderTargetCleared = !Defender || Defender->GetOrderTarget() == nullptr;
            const bool bPassed = bDefenderSurvived && bDefenderTargetCleared && bAcceptanceAttackerOrderCleared;
            UE_LOG(LogTemp, Display, TEXT("RTS_ACCEPTANCE_ATTACKER_DEATH result=%s defender_alive=%d attacker_removed=1 attacker_order_cleared=%d defender_target_cleared=%d"),
                bPassed ? TEXT("PASS") : TEXT("FAIL"), bDefenderSurvived ? 1 : 0,
                bAcceptanceAttackerOrderCleared ? 1 : 0, bDefenderTargetCleared ? 1 : 0);
            LastCommandFeedback = bPassed
                ? TEXT("Acceptance fixture: attacker destroyed; attack ended cleanly.")
                : TEXT("Acceptance fixture: attacker death cleanup failed.");
            AcceptancePhase = EAcceptancePhase::Done;
            return;
        }
        if (AcceptanceElapsed > 6.0f)
        {
            UE_LOG(LogTemp, Error, TEXT("RTS_ACCEPTANCE_ATTACKER_DEATH result=FAIL reason=attacker_survived"));
            AcceptancePhase = EAcceptancePhase::Done;
        }
        return;
    }

    APolitikaPrototypeUnit* Capturer = Units.IsEmpty() ? nullptr : Units[0];
    if (!Capturer || !Objective)
    {
        UE_LOG(LogTemp, Error, TEXT("RTS_ACCEPTANCE_CAPTURE_PAUSE result=FAIL reason=fixture_lost"));
        AcceptancePhase = EAcceptancePhase::Done;
        return;
    }

    switch (AcceptancePhase)
    {
    case EAcceptancePhase::CaptureApproach:
        if (FlatDistance(Capturer->GetActorLocation(), Objective->GetActorLocation()) <= 280.0f)
        {
            AcceptanceElapsed = 0.0f;
            AcceptancePhase = EAcceptancePhase::CaptureHold;
            LastCommandFeedback = TEXT("Acceptance fixture: uncontested capture progressing.");
        }
        else if (AcceptanceElapsed > 12.0f)
        {
            UE_LOG(LogTemp, Error, TEXT("RTS_ACCEPTANCE_CAPTURE_PAUSE result=FAIL reason=unit_never_entered"));
            AcceptancePhase = EAcceptancePhase::Done;
        }
        break;
    case EAcceptancePhase::CaptureHold:
        if (AcceptanceElapsed >= 1.5f)
        {
            AcceptanceProgressAtPause = Objective->GetProgress();
            SetAcceptanceMove(Capturer, FVector(-500.0f, -400.0f, UnitGroundZ));
            AcceptanceElapsed = 0.0f;
            AcceptancePhase = EAcceptancePhase::CaptureExit;
            LastCommandFeedback = TEXT("Acceptance fixture: moving the capturer out of the area.");
        }
        break;
    case EAcceptancePhase::CaptureExit:
        if (FlatDistance(Capturer->GetActorLocation(), Objective->GetActorLocation()) > 320.0f)
        {
            AcceptanceProgressAtPause = Objective->GetProgress();
            AcceptanceElapsed = 0.0f;
            AcceptancePhase = EAcceptancePhase::CapturePause;
            LastCommandFeedback = TEXT("Acceptance fixture: no eligible unit present; capture should pause.");
        }
        else if (AcceptanceElapsed > 8.0f)
        {
            UE_LOG(LogTemp, Error, TEXT("RTS_ACCEPTANCE_CAPTURE_PAUSE result=FAIL reason=unit_never_left"));
            AcceptancePhase = EAcceptancePhase::Done;
        }
        break;
    case EAcceptancePhase::CapturePause:
        if (AcceptanceElapsed >= 1.5f)
        {
            const float ProgressAfterPause = Objective->GetProgress();
            const bool bPassed = AcceptanceProgressAtPause > 0.0f
                && FMath::IsNearlyEqual(AcceptanceProgressAtPause, ProgressAfterPause, 0.0001f);
            UE_LOG(LogTemp, Display, TEXT("RTS_ACCEPTANCE_CAPTURE_PAUSE result=%s progress_before=%.4f progress_after=%.4f"),
                bPassed ? TEXT("PASS") : TEXT("FAIL"), AcceptanceProgressAtPause, ProgressAfterPause);
            LastCommandFeedback = bPassed
                ? TEXT("Acceptance fixture: capture paused with no eligible unit present.")
                : TEXT("Acceptance fixture: capture pause assertion failed.");
            if (AcceptanceMode == TEXT("combined"))
            {
                AcceptancePhase = EAcceptancePhase::CombatSetup;
                AcceptanceElapsed = 0.0f;
            }
            else
            {
                AcceptancePhase = EAcceptancePhase::Done;
            }
        }
        break;
    default:
        break;
    }
}
#endif

APolitikaPrototypeUnit* APolitikaPrototypeMatch::GetSelectedUnitForPlayer(uint32 PlayerId) const
{
    const TWeakObjectPtr<APolitikaPrototypeUnit>* Found = SelectionByPlayer.Find(PlayerId);
    return Found ? Found->Get() : nullptr;
}

FVector2D APolitikaPrototypeMatch::GetCameraPanMin() const
{
    return MapDefinition ? MapDefinition->CameraPanMin : PlayableMin;
}

FVector2D APolitikaPrototypeMatch::GetCameraPanMax() const
{
    return MapDefinition ? MapDefinition->CameraPanMax : PlayableMax;
}

void APolitikaPrototypeMatch::SetSelection(uint32 PlayerId, APolitikaPrototypeUnit* Unit)
{
    if (!RuntimeParticipants.IsEmpty())
    {
        const FPolitikaPrototypeRuntimeParticipant* Participant = RuntimeParticipants.FindByPredicate(
            [PlayerId](const FPolitikaPrototypeRuntimeParticipant& Candidate) { return Candidate.RuntimePlayerId == PlayerId; });
        if (!Participant || Participant->ControllerKind != EPolitikaPrototypeControllerKind::LocalHuman)
        {
            if (APolitikaPrototypeUnit* Previous = GetSelectedUnitForPlayer(PlayerId)) Previous->SetSelectedVisual(false);
            SelectionByPlayer.Remove(PlayerId);
            return;
        }
    }
    if (APolitikaPrototypeUnit* Previous = GetSelectedUnitForPlayer(PlayerId))
        Previous->SetSelectedVisual(false);
    if (!Unit || Unit->GetOwnerPlayer() != PlayerId || !Unit->IsAlive())
    {
        SelectionByPlayer.Remove(PlayerId);
        return;
    }
    SelectionByPlayer.Add(PlayerId, Unit);
    Unit->SetSelectedVisual(true);
#if !UE_BUILD_SHIPPING
    UE_LOG(LogPolitikaStartup, Display, TEXT("RTS_SELECTION player=%u unit=%llu"), PlayerId, Unit->GetStableId());
#endif
}

void APolitikaPrototypeMatch::SubmitMove(uint32 PlayerId, APolitikaPrototypeUnit* Unit, const FVector& Destination, bool bAttackMove)
{
    const FVector2D MapPosition(Destination.X - PlayableMin.X, Destination.Y - PlayableMin.Y);
    if (bAttackMove) SubmitCommand(PlayerId, Unit, nullptr, politika::rules::AttackMoveOrder{{MapPosition.X, MapPosition.Y}});
    else SubmitCommand(PlayerId, Unit, nullptr, politika::rules::MoveOrder{{MapPosition.X, MapPosition.Y}});
}

void APolitikaPrototypeMatch::SubmitAttack(uint32 PlayerId, APolitikaPrototypeUnit* Unit, APolitikaPrototypeUnit* Target)
{
    SubmitCommand(PlayerId, Unit, Target,
        politika::rules::AttackOrder{Target ? Target->GetStableId() : 0});
}

void APolitikaPrototypeMatch::SubmitStop(uint32 PlayerId, APolitikaPrototypeUnit* Unit)
{
    SubmitCommand(PlayerId, Unit, nullptr, politika::rules::StopOrder{});
}

void APolitikaPrototypeMatch::SubmitGuard(uint32 PlayerId, APolitikaPrototypeUnit* Unit)
{
    SubmitCommand(PlayerId, Unit, nullptr, politika::rules::GuardOrder{});
}

bool APolitikaPrototypeMatch::IsLocalControllerBound(const APlayerController* Source) const
{
    if (!Source || !GetWorld() || Source != GetWorld()->GetFirstPlayerController() || LocalRuntimePlayerId == 0)
        return false;
    const FPolitikaPrototypeRuntimeParticipant* Participant = RuntimeParticipants.FindByPredicate(
        [this](const FPolitikaPrototypeRuntimeParticipant& Candidate)
        {
            return Candidate.RuntimePlayerId == LocalRuntimePlayerId
                && Candidate.ControllerKind == EPolitikaPrototypeControllerKind::LocalHuman;
        });
    return Participant != nullptr;
}

bool APolitikaPrototypeMatch::SubmitLocalControllerCommand(APlayerController* Source,
    APolitikaPrototypeUnit* Unit, APolitikaPrototypeUnit* Target, const politika::rules::Order& Intent)
{
    if (!IsLocalControllerBound(Source))
    {
        LastCommandFeedback = TEXT("This controller is not bound to a playable participant.");
        return false;
    }
    SubmitCommand(LocalRuntimePlayerId, Unit, Target, Intent);
    return true;
}

void APolitikaPrototypeMatch::SubmitCommand(uint32 PlayerId, APolitikaPrototypeUnit* Unit,
    APolitikaPrototypeUnit* Target, const politika::rules::Order& Intent)
{
    FPendingOrder Pending;
    Pending.Request.match_generation = MatchGeneration;
    Pending.Request.sequence = NextCommandSequence++;
    Pending.Request.participant_id = PlayerId;
    Pending.Request.issuer_entity_id = Unit ? Unit->GetStableId() : 0;
    Pending.Request.correlation_id = Pending.Request.sequence;
    Pending.Request.intent = Intent;
    if (!Lifecycle.IsActive())
    {
        LastCommandFeedback = TEXT("The match is not accepting commands.");
        PublishGameplayEvent(politika::rules::CommandOutcomeEvent{
            SimulationStepsExecuted, Pending.Request.sequence, Pending.Request.correlation_id,
            Pending.Request.issuer_entity_id, politika::rules::KindOf(Pending.Request.intent), false,
            politika::rules::OrderRejection::MatchInactive});
        return;
    }
    if (!RuntimeParticipants.IsEmpty())
    {
        const FPolitikaPrototypeRuntimeParticipant* Participant = RuntimeParticipants.FindByPredicate(
            [PlayerId](const FPolitikaPrototypeRuntimeParticipant& Candidate) { return Candidate.RuntimePlayerId == PlayerId; });
        if (!Participant)
        {
            LastCommandFeedback = TEXT("That command authority is not assigned to a playable participant.");
            PublishGameplayEvent(politika::rules::CommandOutcomeEvent{
                SimulationStepsExecuted, Pending.Request.sequence, Pending.Request.correlation_id,
                Pending.Request.issuer_entity_id, politika::rules::KindOf(Pending.Request.intent), false,
                politika::rules::OrderRejection::Unauthorized});
            return;
        }
        if (Unit && (Unit->GetOwnerPlayer() != PlayerId || Unit->GetFactionId() != Participant->FactionId))
        {
            LastCommandFeedback = TEXT("The unit owner or faction does not match the admitted participant.");
            PublishGameplayEvent(politika::rules::CommandOutcomeEvent{
                SimulationStepsExecuted, Pending.Request.sequence, Pending.Request.correlation_id,
                Pending.Request.issuer_entity_id, politika::rules::KindOf(Pending.Request.intent), false,
                politika::rules::OrderRejection::NotOwned});
            return;
        }
    }
    Pending.Unit = Unit;
    Pending.Target = Target;
    if (const auto* Move = std::get_if<politika::rules::MoveOrder>(&Intent))
    {
        Pending.Kind = EPolitikaPrototypeOrder::Move;
        Pending.Destination = FVector(Move->destination.x + PlayableMin.X, Move->destination.y + PlayableMin.Y, UnitGroundZ);
    }
    else if (const auto* AttackMove = std::get_if<politika::rules::AttackMoveOrder>(&Intent))
    {
        Pending.Kind = EPolitikaPrototypeOrder::AttackMove;
        Pending.Destination = FVector(AttackMove->destination.x + PlayableMin.X, AttackMove->destination.y + PlayableMin.Y, UnitGroundZ);
    }
    else if (std::holds_alternative<politika::rules::AttackOrder>(Intent)) Pending.Kind = EPolitikaPrototypeOrder::Attack;
    else if (std::holds_alternative<politika::rules::GuardOrder>(Intent)) Pending.Kind = EPolitikaPrototypeOrder::Guard;
    else Pending.Kind = EPolitikaPrototypeOrder::Stop;
    PendingOrders.Add(MoveTemp(Pending));
}

politika::rules::OrderResult APolitikaPrototypeMatch::ValidateOrder(const FPendingOrder& Pending) const
{
    const APolitikaPrototypeUnit* Issuer = Pending.Unit.Get();
    politika::rules::Entity IssuerRules;
    if (Issuer) { IssuerRules.id = Issuer->GetStableId(); IssuerRules.owner_player = Issuer->GetOwnerPlayer();
        IssuerRules.movable = Issuer->CanMove(); IssuerRules.can_attack = Issuer->CanAttack();
        IssuerRules.alive = Issuer->IsAlive(); IssuerRules.operational = Issuer->IsAlive(); }

    politika::rules::Entity TargetRules;
    const APolitikaPrototypeUnit* Target = Pending.Target.Get();
    if (Target)
    {
        TargetRules.id = Target->GetStableId();
        TargetRules.owner_player = Target->GetOwnerPlayer();
        TargetRules.attackable = Target->IsAttackable();
        TargetRules.alive = Target->IsAlive();
    }
    return politika::rules::AdmitCommand(Pending.Request, MatchGeneration,
        bContentReady && Lifecycle.IsActive(),
        Issuer ? &IssuerRules : nullptr,
        {PlayableMax.X - PlayableMin.X, PlayableMax.Y - PlayableMin.Y}, Target ? &TargetRules : nullptr);
}

static FString DescribeRejection(politika::rules::OrderRejection Rejection)
{
    switch (Rejection)
    {
        case politika::rules::OrderRejection::NotOwned: return TEXT("You cannot command that unit.");
        case politika::rules::OrderRejection::CannotMove: return TEXT("That unit cannot move.");
        case politika::rules::OrderRejection::CannotAttack: return TEXT("That unit cannot attack.");
        case politika::rules::OrderRejection::InvalidBounds: return TEXT("The map bounds are invalid.");
        case politika::rules::OrderRejection::InvalidTarget: return TEXT("That destination is outside the map.");
        case politika::rules::OrderRejection::MissingTarget: return TEXT("That target is no longer available.");
        case politika::rules::OrderRejection::TargetNotAttackable: return TEXT("That target cannot be attacked.");
        case politika::rules::OrderRejection::FriendlyTarget: return TEXT("Orders cannot target friendly units.");
        case politika::rules::OrderRejection::MissingIssuer: return TEXT("The selected unit no longer exists.");
        case politika::rules::OrderRejection::IssuerUnavailable: return TEXT("That unit is unavailable.");
        case politika::rules::OrderRejection::Unauthorized: return TEXT("You cannot command that unit.");
        case politika::rules::OrderRejection::MatchInactive: return TEXT("The match is not accepting commands.");
        case politika::rules::OrderRejection::StaleMatch: return TEXT("That command belongs to an earlier match.");
        case politika::rules::OrderRejection::None: return TEXT("Order rejected.");
    }
    return TEXT("Order rejected.");
}

void APolitikaPrototypeMatch::PublishGameplayEvent(politika::rules::GameplayEvent&& Event)
{
#if !UE_BUILD_SHIPPING
    ObserveNavigationAcceptanceEvent(Event);
#endif
#if !UE_BUILD_SHIPPING
    using namespace politika::rules;
    if (const auto* Outcome = std::get_if<CommandOutcomeEvent>(&Event))
    {
        UE_LOG(LogPolitikaStartup, Display, TEXT("RTS_GAMEPLAY_EVENT kind=CommandOutcome step=%llu sequence=%llu correlation=%llu entity=%llu order=%u accepted=%d rejection=%u"),
            Outcome->step, Outcome->sequence, Outcome->correlation_id, Outcome->issuer_id,
            static_cast<uint32>(Outcome->order), Outcome->accepted ? 1 : 0, static_cast<uint32>(Outcome->rejection));
    }
    else if (const auto* Changed = std::get_if<OrderChangedEvent>(&Event))
    {
        UE_LOG(LogPolitikaStartup, Display, TEXT("RTS_GAMEPLAY_EVENT kind=OrderChanged step=%llu entity=%llu previous=%u current=%u reason=%u"),
            Changed->step, Changed->entity_id, static_cast<uint32>(Changed->previous),
            static_cast<uint32>(Changed->current), static_cast<uint32>(Changed->previous_end));
    }
    else if (const auto* Damaged = std::get_if<UnitDamagedEvent>(&Event))
    {
        UE_LOG(LogPolitikaStartup, Display, TEXT("RTS_GAMEPLAY_EVENT kind=UnitDamaged step=%llu entity=%llu applied=%d"),
            Damaged->step, Damaged->entity_id, Damaged->applied);
    }
    else if (const auto* Destroyed = std::get_if<UnitDestroyedEvent>(&Event))
    {
        UE_LOG(LogPolitikaStartup, Display, TEXT("RTS_GAMEPLAY_EVENT kind=UnitDestroyed step=%llu entity=%llu"),
            Destroyed->step, Destroyed->entity_id);
    }
    else if (const auto* Lost = std::get_if<TargetLostEvent>(&Event))
    {
        UE_LOG(LogPolitikaStartup, Display, TEXT("RTS_GAMEPLAY_EVENT kind=TargetLost step=%llu entity=%llu target=%llu"),
            Lost->step, Lost->entity_id, Lost->target_id);
    }
    else if (const auto* ObjectiveEvent = std::get_if<ObjectiveChangedEvent>(&Event))
    {
        UE_LOG(LogPolitikaStartup, Display, TEXT("RTS_GAMEPLAY_EVENT kind=ObjectiveChanged step=%llu contested=%d complete=%d progress_milli=%u"),
            ObjectiveEvent->step, ObjectiveEvent->contested ? 1 : 0, ObjectiveEvent->complete ? 1 : 0, ObjectiveEvent->progress_milli);
    }
#endif
    PublishedEvents.Add(MoveTemp(Event));
    GameplayEventPublished.Broadcast(PublishedEvents.Last());
}

void APolitikaPrototypeMatch::QueueGameplayEvent(politika::rules::GameplayEvent&& Event)
{
    PendingGameplayEvents.Add(MoveTemp(Event));
}

void APolitikaPrototypeMatch::PublishGameplayEvents()
{
    for (politika::rules::GameplayEvent& Event : PendingGameplayEvents)
        PublishGameplayEvent(MoveTemp(Event));
    PendingGameplayEvents.Reset();
}

bool APolitikaPrototypeMatch::ResolveNavigationRoute(APolitikaPrototypeUnit* Unit, const FVector& Destination,
    TArray<FVector>& OutWaypoints, FVector& OutResolvedDestination, FString& OutFailure,
    uint64 OrderSequence) const
{
    OutWaypoints.Reset();
    OutFailure = TEXT("NavigationUnavailable");
    if (!Unit || !GetWorld()) return false;
    UNavigationSystemV1* NavSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
    if (!NavSystem) return false;
    if (NavSystem->IsNavigationBuildInProgress())
    {
        OutFailure = TEXT("NavigationBuilding");
        return false;
    }
    const ANavigationData* NavData = NavSystem->GetDefaultNavDataInstance(FNavigationSystem::DontCreate);
    if (!NavData) return false;
    FNavLocation ProjectedStart, ProjectedGoal;
    const FVector QueryExtent(80.0f, 80.0f, 180.0f);
    if (!NavSystem->ProjectPointToNavigation(Unit->GetActorLocation(), ProjectedStart, QueryExtent, NavData))
    {
        OutFailure = TEXT("OriginNotNavigable");
        return false;
    }
    if (!NavSystem->ProjectPointToNavigation(Destination, ProjectedGoal, QueryExtent, NavData)
        || FVector::Dist(ProjectedGoal.Location, Destination) > 150.0f)
    {
        OutFailure = TEXT("DestinationNotNavigable");
        return false;
    }
    FPathFindingQuery Query(Unit, *NavData, ProjectedStart.Location, ProjectedGoal.Location);
    Query.SetAllowPartialPaths(false);
    Query.SetRequireNavigableEndLocation(true);
    const double QueryStartSeconds = FPlatformTime::Seconds();
    const FPathFindingResult Result = NavSystem->FindPathSync(Query);
#if !UE_BUILD_SHIPPING
    UE_LOG(LogPolitikaStartup, Display,
        TEXT("RTS_NAV_QUERY generation=%llu unit=%llu sequence=%llu result=%u partial=%d points=%d duration_us=%.1f"),
        MatchGeneration, Unit->GetStableId(), OrderSequence, static_cast<uint32>(Result.Result),
        Result.IsPartial() ? 1 : 0, Result.Path.IsValid() ? Result.Path->GetPathPoints().Num() : 0,
        (FPlatformTime::Seconds() - QueryStartSeconds) * 1000000.0);
#endif
    if (Result.Result != ENavigationQueryResult::Success || !Result.Path.IsValid())
    {
        OutFailure = TEXT("NoCompletePath");
        return false;
    }
    if (Result.Path->IsPartial())
    {
        OutFailure = TEXT("PartialPath");
        return false;
    }
    const TArray<FNavPathPoint>& Points = Result.Path->GetPathPoints();
    if (Points.IsEmpty())
    {
        OutFailure = TEXT("EmptyPath");
        return false;
    }
    OutWaypoints.Reserve(Points.Num());
    for (const FNavPathPoint& Point : Points)
    {
        if (!FMath::IsFinite(Point.Location.X) || !FMath::IsFinite(Point.Location.Y)
            || !FMath::IsFinite(Point.Location.Z))
        {
            OutWaypoints.Reset();
            OutFailure = TEXT("InvalidWaypoint");
            return false;
        }
        OutWaypoints.Add(Point.Location + FVector(0.0, 0.0,
            Unit->GetActorLocation().Z - ProjectedStart.Location.Z));
    }
    if (FVector::Dist(OutWaypoints.Last(), ProjectedGoal.Location) > 150.0f)
    {
        OutWaypoints.Reset();
        OutFailure = TEXT("PathEndMismatch");
        return false;
    }
    OutResolvedDestination = ProjectedGoal.Location
        + FVector(0.0, 0.0, Unit->GetActorLocation().Z - ProjectedStart.Location.Z);
    OutFailure.Reset();
#if !UE_BUILD_SHIPPING
    if (NavigationAcceptance.bRunning)
    {
        UE_LOG(LogPolitikaStartup, Display,
            TEXT("NAV_ACCEPTANCE_QUERY generation=%llu unit=%llu sequence=%llu profile=%s nav=%s result=%u partial=%d projected_start=%.6f,%.6f,%.6f projected_goal=%.6f,%.6f,%.6f resolved=%.6f,%.6f,%.6f points=%d"),
            MatchGeneration, Unit->GetStableId(), OrderSequence, *MapDefinition->MovementProfileId.ToString(),
            *NavData->GetPathName(), static_cast<uint32>(Result.Result), Result.IsPartial() ? 1 : 0,
            ProjectedStart.Location.X, ProjectedStart.Location.Y, ProjectedStart.Location.Z,
            ProjectedGoal.Location.X, ProjectedGoal.Location.Y, ProjectedGoal.Location.Z,
            OutResolvedDestination.X, OutResolvedDestination.Y, OutResolvedDestination.Z, OutWaypoints.Num());
        for (int32 Index = 0; Index < OutWaypoints.Num(); ++Index)
            UE_LOG(LogPolitikaStartup, Display, TEXT("NAV_ACCEPTANCE_POINT index=%d position=%.6f,%.6f,%.6f sequence=%llu"),
                Index, OutWaypoints[Index].X, OutWaypoints[Index].Y, OutWaypoints[Index].Z, OrderSequence);
    }
    UE_LOG(LogPolitikaStartup, Display,
        TEXT("RTS_NAVIGATION_READY generation=%llu unit=%llu points=%d requested=(%.1f,%.1f,%.1f) resolved=(%.1f,%.1f,%.1f)"),
        MatchGeneration, Unit->GetStableId(), OutWaypoints.Num(), Destination.X, Destination.Y, Destination.Z,
        OutResolvedDestination.X, OutResolvedDestination.Y, OutResolvedDestination.Z);
#endif
    return true;
}

void APolitikaPrototypeMatch::AdmitPendingOrders()
{
    for (const FPendingOrder& Pending : PendingOrders)
    {
        APolitikaPrototypeUnit* Unit = Pending.Unit.Get();
        const politika::rules::OrderResult Result = ValidateOrder(Pending);
        if (!Result)
        {
            const FString Reason = DescribeRejection(Result.rejection);
            LastCommandFeedback = Reason;
#if !UE_BUILD_SHIPPING
            const APolitikaPrototypeUnit* RejectedUnit = Pending.Unit.Get();
            UE_LOG(LogPolitikaStartup, Display, TEXT("RTS_ORDER_REJECTED unit=%llu sequence=%llu outcome=%u reason=%s"),
                RejectedUnit ? RejectedUnit->GetStableId() : 0, Pending.Request.sequence,
                static_cast<uint32>(Result.rejection), *Reason.Replace(TEXT(" "), TEXT("_")));
#endif
            QueueGameplayEvent(politika::rules::CommandOutcomeEvent{
                SimulationStepsExecuted + 1, Pending.Request.sequence, Pending.Request.correlation_id,
                Pending.Request.issuer_entity_id, politika::rules::KindOf(Pending.Request.intent), false, Result.rejection});
            continue;
        }
        const EPolitikaPrototypeOrder PreviousOrder = Unit->GetCurrentOrder();
        bool bNavigationReady = true;
        FString NavigationFailure;
        TArray<FVector> Route;
        FVector ResolvedDestination = Pending.Destination;
        if (Pending.Kind == EPolitikaPrototypeOrder::Move || Pending.Kind == EPolitikaPrototypeOrder::AttackMove)
        {
            bNavigationReady = ResolveNavigationRoute(Unit, Pending.Destination, Route,
                ResolvedDestination, NavigationFailure, Pending.Request.sequence);
        }
        switch (Pending.Kind)
        {
            case EPolitikaPrototypeOrder::Move:
            case EPolitikaPrototypeOrder::AttackMove:
                if (bNavigationReady)
                {
                    Unit->SetMoveOrder(Pending.Kind, Pending.Destination, ResolvedDestination, MoveTemp(Route));
                    Unit->NavigationMatchGeneration = MatchGeneration;
                    Unit->NavigationOrderSequence = Pending.Request.sequence;
                    LastCommandFeedback = Pending.Kind == EPolitikaPrototypeOrder::Move
                        ? TEXT("Move order accepted.") : TEXT("Attack-move order accepted.");
                }
                else
                {
                    Unit->StopOrder();
                    LastCommandFeedback = FString::Printf(TEXT("Movement route unavailable: %s."), *NavigationFailure);
                }
                break;
            case EPolitikaPrototypeOrder::Attack:
                Unit->SetAttackOrder(Pending.Target.Get());
                Unit->NavigationMatchGeneration = MatchGeneration;
                Unit->NavigationOrderSequence = Pending.Request.sequence;
                LastCommandFeedback = TEXT("Attack order accepted.");
                break;
            case EPolitikaPrototypeOrder::Guard:
                Unit->SetGuardOrder();
                Unit->NavigationMatchGeneration = MatchGeneration;
                Unit->NavigationOrderSequence = Pending.Request.sequence;
                LastCommandFeedback = TEXT("Guard order accepted.");
                break;
            case EPolitikaPrototypeOrder::Stop: Unit->StopOrder(); LastCommandFeedback = TEXT("Unit stopped."); break;
            case EPolitikaPrototypeOrder::Idle: break;
        }
        QueueGameplayEvent(politika::rules::CommandOutcomeEvent{
            SimulationStepsExecuted + 1, Pending.Request.sequence, Pending.Request.correlation_id,
            Pending.Request.issuer_entity_id, politika::rules::KindOf(Pending.Request.intent), true,
            politika::rules::OrderRejection::None});
        if (!bNavigationReady)
        {
            UE_LOG(LogPolitikaStartup, Warning,
                TEXT("RTS_NAVIGATION_FAILED generation=%llu unit=%llu sequence=%llu reason=%s x=%.1f y=%.1f"),
                MatchGeneration, Unit->GetStableId(), Pending.Request.sequence, *NavigationFailure,
                Pending.Destination.X, Pending.Destination.Y);
            QueueGameplayEvent(politika::rules::OrderChangedEvent{
                SimulationStepsExecuted + 1, Unit->GetStableId(), ToExecutionOrder(PreviousOrder),
                ToExecutionOrder(Unit->GetCurrentOrder()), politika::rules::OrderEndReason::NavigationFailed});
            continue;
        }
        const bool bOrderTransitioned = PreviousOrder != Unit->GetCurrentOrder()
            || (PreviousOrder != EPolitikaPrototypeOrder::Idle && Pending.Kind != EPolitikaPrototypeOrder::Stop);
        if (bOrderTransitioned)
        {
            const auto EndReason = PreviousOrder == EPolitikaPrototypeOrder::Idle
                ? politika::rules::OrderEndReason::None
                : Pending.Kind == EPolitikaPrototypeOrder::Stop
                    ? politika::rules::OrderEndReason::Stopped : politika::rules::OrderEndReason::Replaced;
            QueueGameplayEvent(politika::rules::OrderChangedEvent{
                SimulationStepsExecuted + 1, Unit->GetStableId(), ToExecutionOrder(PreviousOrder),
                ToExecutionOrder(Unit->GetCurrentOrder()), EndReason});
        }
#if !UE_BUILD_SHIPPING
        const APolitikaPrototypeUnit* AdmittedUnit = Pending.Unit.Get();
        const APolitikaPrototypeUnit* AdmittedTarget = Pending.Target.Get();
        static const TCHAR* OrderNames[] = { TEXT("Idle"), TEXT("Move"), TEXT("Attack"), TEXT("AttackMove"), TEXT("Stop"), TEXT("Guard") };
        const int32 OrderIndex = static_cast<int32>(Pending.Kind);
        UE_LOG(LogPolitikaStartup, Display, TEXT("RTS_ORDER_ADMITTED order=%s unit=%llu target=%llu sequence=%llu"),
            OrderNames[OrderIndex], AdmittedUnit ? AdmittedUnit->GetStableId() : 0,
            AdmittedTarget ? AdmittedTarget->GetStableId() : 0, Pending.Request.sequence);
#endif
    }
    PendingOrders.Reset();
}

void APolitikaPrototypeMatch::RunSimulationStep(float DeltaSeconds)
{
    ++SimulationStepsExecuted;
#if !UE_BUILD_SHIPPING
    BeforeNavigationAcceptanceStep();
    if (SimulationStepsExecuted == 1)
        UE_LOG(LogPolitikaStartup, Display, TEXT("MATCH_SIMULATION_STEP first=1"));
#endif
    // Explicit simulation phase order: movement → target/combat intent → damage
    // application → dead-entity cleanup → objective evaluation. UI only reads.
    for (APolitikaPrototypeUnit* Unit : Units)
    {
        if (!Unit) continue;
        if ((Unit->GetCurrentOrder() == EPolitikaPrototypeOrder::Move
            || Unit->GetCurrentOrder() == EPolitikaPrototypeOrder::AttackMove)
            && Unit->NavigationMatchGeneration != MatchGeneration)
        {
            const EPolitikaPrototypeOrder PreviousOrder = Unit->GetCurrentOrder();
            Unit->StopOrder();
            QueueGameplayEvent(politika::rules::OrderChangedEvent{SimulationStepsExecuted + 1,
                Unit->GetStableId(), ToExecutionOrder(PreviousOrder), ToExecutionOrder(Unit->GetCurrentOrder()),
                politika::rules::OrderEndReason::NavigationFailed});
            continue;
        }
        const EPolitikaPrototypeOrder PreviousOrder = Unit->GetCurrentOrder();
        const politika::rules::OrderEndReason EndReason = Unit->SimulateMovement(DeltaSeconds);
#if !UE_BUILD_SHIPPING
        if (NavigationAcceptance.bRunning && Unit == NavigationAcceptance.Unit.Get()
            && EndReason != politika::rules::OrderEndReason::None)
        {
            if (EndReason == politika::rules::OrderEndReason::Arrived) ++NavigationAcceptance.Terminations;
            else ++NavigationAcceptance.Failures;
            UE_LOG(LogPolitikaStartup, Display, TEXT("NAV_ACCEPTANCE_TERMINATION step=%llu sequence=%llu reason=%u"),
                SimulationStepsExecuted, Unit->NavigationOrderSequence, static_cast<uint32>(EndReason));
        }
#endif
        if (EndReason != politika::rules::OrderEndReason::None)
            QueueGameplayEvent(politika::rules::OrderChangedEvent{SimulationStepsExecuted,
                Unit->GetStableId(), ToExecutionOrder(PreviousOrder), ToExecutionOrder(Unit->GetCurrentOrder()), EndReason});
    }
    for (APolitikaPrototypeUnit* Unit : Units) if (Unit) Unit->ResolveCombat(DeltaSeconds);
    for (APolitikaPrototypeUnit* Unit : Units)
    {
        if (!Unit) continue;
        const EPolitikaPrototypeOrder PreviousOrder = Unit->GetCurrentOrder();
        const int32 AppliedDamage = Unit->ApplyPendingDamage();
        if (AppliedDamage > 0)
            QueueGameplayEvent(politika::rules::UnitDamagedEvent{SimulationStepsExecuted, Unit->GetStableId(), AppliedDamage});
        if (PreviousOrder != Unit->GetCurrentOrder())
            QueueGameplayEvent(politika::rules::OrderChangedEvent{SimulationStepsExecuted, Unit->GetStableId(),
                ToExecutionOrder(PreviousOrder), ToExecutionOrder(Unit->GetCurrentOrder()),
                Unit->IsAlive() ? politika::rules::OrderEndReason::Stopped : politika::rules::OrderEndReason::UnitDestroyed});
    }
    for (APolitikaPrototypeUnit* Unit : Units)
    {
        if (!Unit) continue;
        const EPolitikaPrototypeOrder PreviousOrder = Unit->GetCurrentOrder();
        if (const uint64 LostTargetId = Unit->CleanupDeadTarget(); LostTargetId != 0)
        {
            QueueGameplayEvent(politika::rules::TargetLostEvent{SimulationStepsExecuted, Unit->GetStableId(), LostTargetId});
            if (PreviousOrder != Unit->GetCurrentOrder())
                QueueGameplayEvent(politika::rules::OrderChangedEvent{SimulationStepsExecuted, Unit->GetStableId(),
                    ToExecutionOrder(PreviousOrder), ToExecutionOrder(Unit->GetCurrentOrder()), politika::rules::OrderEndReason::TargetLost});
        }
    }
    for (APolitikaPrototypeUnit* Unit : Units)
    {
        if (Unit && !Unit->IsAlive())
        {
            QueueGameplayEvent(politika::rules::UnitDestroyedEvent{SimulationStepsExecuted, Unit->GetStableId()});
#if !UE_BUILD_SHIPPING
            if (AcceptancePhase == EAcceptancePhase::CombatResolve && Unit == AcceptanceAttacker.Get())
                bAcceptanceAttackerOrderCleared = Unit->GetCurrentOrder() == EPolitikaPrototypeOrder::Stop;
#endif
            LastCommandFeedback = FString::Printf(TEXT("%s destroyed."), *Unit->GetUnitName().ToString());
#if !UE_BUILD_SHIPPING
            UE_LOG(LogPolitikaStartup, Display, TEXT("RTS_UNIT_DESTROYED unit=%llu"), Unit->GetStableId());
#endif
            Unit->Destroy();
        }
    }
    Units.RemoveAll([](const TObjectPtr<APolitikaPrototypeUnit>& Unit) { return !Unit || !Unit->IsAlive(); });
    if (Objective)
    {
        const bool bWasContested = Objective->IsContested();
        const bool bWasComplete = Objective->IsComplete();
        const int32 PreviousProgressMilli = FMath::RoundToInt(Objective->GetProgress() * 1000.0f);
        TArray<APolitikaPrototypeUnit*> RawUnits;
        RawUnits.Reserve(Units.Num());
        for (APolitikaPrototypeUnit* Unit : Units) RawUnits.Add(Unit);
        Objective->UpdateCapture(RawUnits, DeltaSeconds);
        const int32 CurrentProgressMilli = FMath::RoundToInt(Objective->GetProgress() * 1000.0f);
        if (bWasContested != Objective->IsContested() || bWasComplete != Objective->IsComplete()
            || PreviousProgressMilli != CurrentProgressMilli)
            QueueGameplayEvent(politika::rules::ObjectiveChangedEvent{SimulationStepsExecuted,
                Objective->IsContested(), Objective->IsComplete(),
                static_cast<uint16>(FMath::Clamp(CurrentProgressMilli, 0, 1000))});
        if (!bWasComplete && Objective->IsComplete())
        {
            PendingOrders.Reset();
            if (TransitionLifecycle(politika::rules::MatchPhase::Terminal)
                && TransitionLifecycle(politika::rules::MatchPhase::Results)
                && !CommittedResult.IsSet())
            {
                CommittedResult.Emplace(FPolitikaPrototypeMatchResult{
                    LocalRuntimePlayerId, TEXT("objective_secured")});
                LastCommandFeedback = TEXT("Objective secured. Match results are ready; press Escape to return to menu.");
                UE_LOG(LogPolitikaStartup, Display, TEXT("MATCH_RESULTS outcome=%s player=%u committed=1"),
                    *CommittedResult->Reason.ToString(), CommittedResult->WinningPlayerId);
            }
        }
    }
    // Publication is the final simulation phase; observers get a const snapshot
    // until the next authoritative step, with step IDs to avoid duplicate handling.
    PublishGameplayEvents();
#if !UE_BUILD_SHIPPING
    AfterNavigationAcceptanceStep(DeltaSeconds);
#endif
#if WITH_DEV_AUTOMATION_TESTS
    if (NavigationStepObserver) NavigationStepObserver();
#endif
}

APolitikaPrototypeCameraPawn::APolitikaPrototypeCameraPawn()
{
    PrimaryActorTick.bCanEverTick = false;
    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    SetRootComponent(SceneRoot);
    Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
    Camera->SetupAttachment(SceneRoot);
}

void APolitikaPrototypePlayerController::BeginPlay()
{
    Super::BeginPlay();
    ResolveMatch();
    bShowMouseCursor = true;
    SetInputMode(FInputModeGameAndUI());
}

void APolitikaPrototypePlayerController::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    if (!bMapCameraApplied) ResolveMatch();
}

APolitikaPrototypeMatch* APolitikaPrototypePlayerController::ResolveMatch()
{
    if (APolitikaPrototypeMatch* Existing = Match.Get())
    {
        if (APolitikaPrototypeHUD* PrototypeHUD = Cast<APolitikaPrototypeHUD>(GetHUD()))
            PrototypeHUD->AttachMatch(Existing);
        ApplyMapCameraIfReady();
        return Existing;
    }
    if (!GetWorld()) return nullptr;
    TActorIterator<APolitikaPrototypeMatch> MatchIt(GetWorld());
    if (!MatchIt) return nullptr;
    Match = *MatchIt;
    if (APolitikaPrototypeHUD* PrototypeHUD = Cast<APolitikaPrototypeHUD>(GetHUD()))
        PrototypeHUD->AttachMatch(Match.Get());
    ApplyMapCameraIfReady();
    return Match.Get();
}

void APolitikaPrototypePlayerController::ApplyMapCameraIfReady()
{
    if (bMapCameraApplied || !Match.IsValid() || !GetPawn()) return;
    const UPolitikaPrototypeMapDefinition* Definition = Match->GetMapDefinition();
    if (!Definition || !Match->GetObjective()) return;
    APolitikaPrototypeCameraPawn* CameraPawn = Cast<APolitikaPrototypeCameraPawn>(GetPawn());
    if (!CameraPawn || !CameraPawn->GetCamera()) return;
    CameraPawn->GetCamera()->SetRelativeLocation(Definition->CameraOffset);
    CameraPawn->GetCamera()->SetFieldOfView(Definition->CameraFieldOfView);
    const FVector Focus = Match->GetObjective()->GetActorLocation();
    CameraPawn->GetCamera()->SetWorldRotation((Focus - CameraPawn->GetCamera()->GetComponentLocation()).Rotation());
    bMapCameraApplied = true;
}

void APolitikaPrototypePlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();
    InputComponent->BindKey(EKeys::LeftMouseButton, IE_Pressed, this, &APolitikaPrototypePlayerController::SelectUnderCursor);
    InputComponent->BindKey(EKeys::RightMouseButton, IE_Pressed, this, &APolitikaPrototypePlayerController::OrderUnderCursor);
    InputComponent->BindKey(EKeys::S, IE_Pressed, this, &APolitikaPrototypePlayerController::StopSelected);
    InputComponent->BindKey(EKeys::G, IE_Pressed, this, &APolitikaPrototypePlayerController::GuardSelected);
    InputComponent->BindKey(EKeys::A, IE_Pressed, this, &APolitikaPrototypePlayerController::AttackMoveSelected);
    InputComponent->BindKey(EKeys::Escape, IE_Pressed, this, &APolitikaPrototypePlayerController::ReturnToMenu);
    InputComponent->BindAxis(TEXT("PrototypePanForward"), this, &APolitikaPrototypePlayerController::PanForward);
    InputComponent->BindAxis(TEXT("PrototypePanRight"), this, &APolitikaPrototypePlayerController::PanRight);
    InputComponent->BindAxis(TEXT("PrototypeZoom"), this, &APolitikaPrototypePlayerController::Zoom);
}

void APolitikaPrototypePlayerController::SelectUnderCursor()
{
    APolitikaPrototypeMatch* ActiveMatch = ResolveMatch();
    if (!ActiveMatch) return;
    FHitResult Hit;
    if (GetHitResultUnderCursorByChannel(UEngineTypes::ConvertToTraceType(ECC_Visibility), true, Hit))
    {
        APolitikaPrototypeUnit* Unit = Cast<APolitikaPrototypeUnit>(Hit.GetActor());
        if (ActiveMatch->IsLocalControllerBound(this) && Unit
            && Unit->GetOwnerPlayer() == GetCommandPlayerId() && Unit->IsAlive())
        {
            SelectedUnit = Unit;
            ActiveMatch->SetSelection(GetCommandPlayerId(), Unit);
        return;
    }
}
    SelectedUnit.Reset();
    ActiveMatch->SetSelection(GetCommandPlayerId(), nullptr);
}

void APolitikaPrototypePlayerController::OrderUnderCursor()
{
    APolitikaPrototypeMatch* ActiveMatch = ResolveMatch();
    if (!ActiveMatch || !SelectedUnit.IsValid()) return;
    FHitResult Hit;
    if (!GetHitResultUnderCursorByChannel(UEngineTypes::ConvertToTraceType(ECC_Visibility), true, Hit))
    {
        ActiveMatch->ReportRejectedCommand(TEXT("Order rejected: click valid map ground or a hostile unit."));
        return;
    }
    if (APolitikaPrototypeUnit* Target = Cast<APolitikaPrototypeUnit>(Hit.GetActor()))
    {
        ActiveMatch->SubmitLocalControllerCommand(this, SelectedUnit.Get(), Target,
            politika::rules::AttackOrder{Target->GetStableId()});
    }
    else
    {
        const FVector2D MapPosition(Hit.ImpactPoint.X - ActiveMatch->GetPlayableMin().X,
            Hit.ImpactPoint.Y - ActiveMatch->GetPlayableMin().Y);
        ActiveMatch->SubmitLocalControllerCommand(this, SelectedUnit.Get(), nullptr,
            politika::rules::MoveOrder{{MapPosition.X, MapPosition.Y}});
}
}

void APolitikaPrototypePlayerController::StopSelected()
{
    if (APolitikaPrototypeMatch* ActiveMatch = ResolveMatch(); ActiveMatch && SelectedUnit.IsValid())
        ActiveMatch->SubmitLocalControllerCommand(this, SelectedUnit.Get(), nullptr, politika::rules::StopOrder{});
}

void APolitikaPrototypePlayerController::GuardSelected()
{
    if (APolitikaPrototypeMatch* ActiveMatch = ResolveMatch(); ActiveMatch && SelectedUnit.IsValid())
        ActiveMatch->SubmitLocalControllerCommand(this, SelectedUnit.Get(), nullptr, politika::rules::GuardOrder{});
}

void APolitikaPrototypePlayerController::AttackMoveSelected()
{
    APolitikaPrototypeMatch* ActiveMatch = ResolveMatch();
    if (!ActiveMatch || !SelectedUnit.IsValid()) return;
    FHitResult Hit;
    if (GetHitResultUnderCursorByChannel(UEngineTypes::ConvertToTraceType(ECC_Visibility), true, Hit))
    {
        const FVector2D MapPosition(Hit.ImpactPoint.X - ActiveMatch->GetPlayableMin().X,
            Hit.ImpactPoint.Y - ActiveMatch->GetPlayableMin().Y);
        ActiveMatch->SubmitLocalControllerCommand(this, SelectedUnit.Get(), nullptr,
            politika::rules::AttackMoveOrder{{MapPosition.X, MapPosition.Y}});
    }
    else
        ActiveMatch->ReportRejectedCommand(TEXT("Attack-move rejected: click valid map ground."));
}

void APolitikaPrototypePlayerController::ReturnToMenu()
{
    if (APolitikaPrototypeMatch* ActiveMatch = ResolveMatch()) ActiveMatch->BeginReturnToMenu();
    if (UPolitikaGameInstance* Game = GetGameInstance<UPolitikaGameInstance>()) Game->ReturnToMenu(this);
}

void APolitikaPrototypePlayerController::PanForward(float Value)
{
    if (FMath::IsNearlyZero(Value) || !GetPawn()) return;
#if !UE_BUILD_SHIPPING
    const bool bLogAcceptancePan = Value > 0.0f && !bAcceptancePanForwardLogged;
#endif
    GetPawn()->AddActorWorldOffset(FVector(0, Value * 1400.0f * GetWorld()->GetDeltaSeconds(), 0), true);
    FVector Location = GetPawn()->GetActorLocation();
    if (APolitikaPrototypeMatch* ActiveMatch = ResolveMatch())
    {
        const FVector2D Min = ActiveMatch->GetCameraPanMin(); const FVector2D Max = ActiveMatch->GetCameraPanMax();
        Location.X = FMath::Clamp(Location.X, Min.X, Max.X);
        Location.Y = FMath::Clamp(Location.Y, Min.Y, Max.Y);
    }
    GetPawn()->SetActorLocation(Location);
#if !UE_BUILD_SHIPPING
    if (bLogAcceptancePan)
    {
        UE_LOG(LogPolitikaStartup, Display, TEXT("RTS_CAMERA_INPUT axis=forward value=%.1f x=%.1f y=%.1f"),
            Value, Location.X, Location.Y);
        bAcceptancePanForwardLogged = true;
    }
#endif
}

void APolitikaPrototypePlayerController::PanRight(float Value)
{
    if (FMath::IsNearlyZero(Value) || !GetPawn()) return;
    const APolitikaPrototypeCameraPawn* CameraPawn = Cast<APolitikaPrototypeCameraPawn>(GetPawn());
    if (!CameraPawn || !CameraPawn->GetCamera()) return;
    // Screen-right is not world +X for this map's camera orientation.
    FVector Right = CameraPawn->GetCamera()->GetRightVector();
    Right.Z = 0.0f;
    Right.Normalize();
#if !UE_BUILD_SHIPPING
    const bool bLogAcceptancePan = Value > 0.0f && !bAcceptancePanRightLogged;
#endif
    GetPawn()->AddActorWorldOffset(Right * Value * 1400.0f * GetWorld()->GetDeltaSeconds(), true);
    FVector Location = GetPawn()->GetActorLocation();
    if (APolitikaPrototypeMatch* ActiveMatch = ResolveMatch())
    {
        const FVector2D Min = ActiveMatch->GetCameraPanMin(); const FVector2D Max = ActiveMatch->GetCameraPanMax();
        Location.X = FMath::Clamp(Location.X, Min.X, Max.X);
        Location.Y = FMath::Clamp(Location.Y, Min.Y, Max.Y);
    }
    GetPawn()->SetActorLocation(Location);
#if !UE_BUILD_SHIPPING
    if (bLogAcceptancePan)
    {
        UE_LOG(LogPolitikaStartup, Display, TEXT("RTS_CAMERA_INPUT axis=right value=%.1f x=%.1f y=%.1f"),
            Value, Location.X, Location.Y);
        bAcceptancePanRightLogged = true;
    }
#endif
}

void APolitikaPrototypePlayerController::Zoom(float Value)
{
    if (FMath::IsNearlyZero(Value)) return;
    if (APolitikaPrototypeCameraPawn* CameraPawn = Cast<APolitikaPrototypeCameraPawn>(GetPawn()))
    {
        UCameraComponent* Camera = CameraPawn->GetCamera();
        if (!Camera) return;
        const FVector Offset = Camera->GetRelativeLocation();
        const FVector Forward = Camera->GetRelativeRotation().Vector();
        if (Forward.Z >= -UE_SMALL_NUMBER) return;
        // Dolly along the view ray, keeping the ground point at screen centre fixed.
        // Wheel-up moves closer; wheel-down moves away. Keep perspective unchanged.
        const float Distance = -Offset.Z / Forward.Z;
        if (Distance <= UE_SMALL_NUMBER) return;
        const FVector Focus = Offset + Forward * Distance;
        const float NewDistance = FMath::Clamp(Distance * FMath::Pow(0.85f, Value), 900.0f, 7500.0f);
        Camera->SetRelativeLocation(Focus - Forward * NewDistance);
#if !UE_BUILD_SHIPPING
        UE_LOG(LogPolitikaStartup, Display, TEXT("RTS_CAMERA_ZOOM wheel=%.2f distance_before=%.1f distance_after=%.1f fov=%.1f"),
            Value, Distance, NewDistance, Camera->FieldOfView);
#endif
    }
}

void APolitikaPrototypeHUD::AttachMatch(APolitikaPrototypeMatch* InMatch)
{
    if (Match.Get() == InMatch) return;
    if (APolitikaPrototypeMatch* PreviousMatch = Match.Get(); PreviousMatch && GameplayEventHandle.IsValid())
        PreviousMatch->OnGameplayEventPublished().Remove(GameplayEventHandle);
    Match = InMatch;
    GameplayEventHandle.Reset();
    CommandFeedback.Reset();
    if (InMatch)
        GameplayEventHandle = InMatch->OnGameplayEventPublished().AddUObject(
            this, &APolitikaPrototypeHUD::HandleGameplayEvent);
}

void APolitikaPrototypeHUD::HandleGameplayEvent(const politika::rules::GameplayEvent& Event)
{
    using namespace politika::rules;
    const CommandOutcomeEvent* Outcome = std::get_if<CommandOutcomeEvent>(&Event);
    if (!Outcome) return;

    static const TCHAR* OrderNames[] = {
        TEXT("Move"), TEXT("Attack"), TEXT("Attack-move"), TEXT("Stop"), TEXT("Guard")
    };
    const uint8 OrderIndex = static_cast<uint8>(Outcome->order);
    const TCHAR* OrderName = OrderIndex < UE_ARRAY_COUNT(OrderNames) ? OrderNames[OrderIndex] : TEXT("Command");
    if (Outcome->accepted)
    {
        CommandFeedback = FString::Printf(TEXT("%s order accepted  ·  #%llu"), OrderName, Outcome->correlation_id);
        return;
    }

    static const TCHAR* RejectionNames[] = {
        TEXT("No rejection"), TEXT("You cannot command that unit."), TEXT("That unit cannot move."),
        TEXT("That unit cannot attack."), TEXT("The map bounds are invalid."),
        TEXT("That destination is outside the map."), TEXT("That target is no longer available."),
        TEXT("That target cannot be attacked."), TEXT("Orders cannot target friendly units."),
        TEXT("The selected unit no longer exists."), TEXT("That unit is unavailable."),
        TEXT("You cannot command that unit."), TEXT("The match is not accepting commands."),
        TEXT("That command belongs to an earlier match.")
    };
    const uint8 RejectionIndex = static_cast<uint8>(Outcome->rejection);
    const TCHAR* Reason = RejectionIndex < UE_ARRAY_COUNT(RejectionNames)
        ? RejectionNames[RejectionIndex] : TEXT("Order rejected.");
    CommandFeedback = FString::Printf(TEXT("%s rejected: %s  ·  #%llu"),
        OrderName, Reason, Outcome->correlation_id);
}

void APolitikaPrototypeHUD::DrawHUD()
{
    Super::DrawHUD();
    if (!Canvas || !GetWorld()) return;
    APolitikaPrototypeMatch* ActiveMatch = Match.Get();
    if (!ActiveMatch) return;

    // UCanvas text is drawn in physical pixels, unlike the Slate shell which
    // receives the project's DPI scale from the viewport layer manager.
    const float UiScale = FMath::Clamp(Canvas->ClipX / 1920.0f, 1.0f, 2.0f);
    float Y = 38.0f * UiScale;
    const float Left = 40.0f * UiScale;
    const FLinearColor Ink(0.91f, 0.93f, 0.84f);
    DrawText(TEXT("POLITIKA  /  RTS DEVELOPMENT SLICE"), Ink, Left, Y, nullptr, 1.5f * UiScale); Y += 34.0f * UiScale;
    DrawText(TEXT("LMB select  ·  RMB move / attack  ·  A attack-move  ·  G guard  ·  S stop  ·  WASD pan  ·  wheel zoom  ·  Esc menu"), Ink, Left, Y, nullptr, UiScale); Y += 28.0f * UiScale;
    {
        APolitikaPrototypeUnit* Selected = ActiveMatch->GetSelectedUnitForPlayer(ActiveMatch->GetLocalRuntimePlayerId());
        if (Selected)
        {
            const TCHAR* Order = TEXT("Idle");
            switch (Selected->GetCurrentOrder())
            {
                case EPolitikaPrototypeOrder::Move: Order = TEXT("Moving"); break;
                case EPolitikaPrototypeOrder::Attack: Order = TEXT("Attacking"); break;
                case EPolitikaPrototypeOrder::AttackMove: Order = TEXT("Attack-move"); break;
                case EPolitikaPrototypeOrder::Stop: Order = TEXT("Stopped"); break;
                case EPolitikaPrototypeOrder::Guard: Order = TEXT("Guarding"); break;
                case EPolitikaPrototypeOrder::Idle: break;
            }
            DrawText(FString::Printf(TEXT("%s    HP %d / %d    %s"), *Selected->GetUnitName().ToString(),
                Selected->GetHealth(), Selected->GetMaxHealth(), Order), Ink, Left, Y, nullptr, 1.25f * UiScale);
            Y += 30.0f * UiScale;
        }
        else
        {
            DrawText(TEXT("No unit selected"), Ink, Left, Y, nullptr, 1.15f * UiScale); Y += 28.0f * UiScale;
        }
    }
    if (APolitikaPrototypeObjective* Objective = ActiveMatch->GetObjective())
    {
        DrawText(FString::Printf(TEXT("CONTROL POINT  /  %s"), *Objective->GetStatusText().ToString()),
            FLinearColor(0.94f, 0.78f, 0.31f), Left, Y, nullptr, 1.25f * UiScale);
        Y += 30.0f * UiScale;
    }
    const FString& Feedback = CommandFeedback.IsEmpty()
        ? ActiveMatch->GetLastCommandFeedback() : CommandFeedback;
    DrawText(Feedback, Ink, Left, Y, nullptr, UiScale);

    // Original text/glyph placeholders keep this prototype independent of reference-game art.
    const float PanelLeft = Left;
    const float RowHeight = 34.0f * UiScale;
    const float PanelWidth = 390.0f * UiScale;
    const float PanelTop = Canvas->ClipY - (RowHeight * 5.0f) - (36.0f * UiScale);
    DrawText(TEXT("COMMANDS"), Ink, PanelLeft, PanelTop, nullptr, 0.9f * UiScale);
    const TCHAR* Labels[] = { TEXT("MOVE"), TEXT("ATTACK"), TEXT("ATTACK-MOVE"), TEXT("STOP"), TEXT("GUARD") };
    const TCHAR* Hints[] = { TEXT("RMB ground"), TEXT("RMB hostile"), TEXT("A + ground"), TEXT("S"), TEXT("G · provisional") };
    const TCHAR* Marks[] = { TEXT("M"), TEXT("!"), TEXT("A+"), TEXT("X"), TEXT("G") };
    const EPolitikaPrototypeOrder Orders[] = {
        EPolitikaPrototypeOrder::Move, EPolitikaPrototypeOrder::Attack,
        EPolitikaPrototypeOrder::AttackMove, EPolitikaPrototypeOrder::Stop,
        EPolitikaPrototypeOrder::Guard
    };
    const APolitikaPrototypeUnit* Selected = ActiveMatch->GetSelectedUnitForPlayer(ActiveMatch->GetLocalRuntimePlayerId());
    for (int32 Index = 0; Index < UE_ARRAY_COUNT(Labels); ++Index)
    {
        const float RowY = PanelTop + (18.0f * UiScale) + (Index * RowHeight);
        const bool bEnabled = Selected && Selected->IsAlive()
            && ((Index == 0 || Index == 2) ? Selected->CanMove()
                : Index == 1 ? Selected->CanAttack() : true);
        const FLinearColor RowColor = bEnabled
            ? FLinearColor(0.10f, 0.14f, 0.15f, 0.88f)
            : FLinearColor(0.06f, 0.08f, 0.09f, 0.78f);
        const FLinearColor TextColor = bEnabled ? Ink : FLinearColor(0.46f, 0.50f, 0.48f);
        DrawRect(RowColor, PanelLeft, RowY, PanelWidth, RowHeight - (2.0f * UiScale));
        DrawText(FString::Printf(TEXT("[%s]  %s"), Marks[Index], Labels[Index]),
            TextColor, PanelLeft + (10.0f * UiScale), RowY + (6.0f * UiScale), nullptr, 0.9f * UiScale);
        const bool bActive = Selected && Selected->GetCurrentOrder() == Orders[Index];
        const FString Hint = bActive
            ? FString::Printf(TEXT("ACTIVE  ·  %s"), Hints[Index])
            : FString(Hints[Index]);
        DrawText(Hint, bActive ? FLinearColor(0.94f, 0.78f, 0.31f) : TextColor,
            PanelLeft + (230.0f * UiScale), RowY + (6.0f * UiScale), nullptr, 0.82f * UiScale);
    }
}

APolitikaPrototypeGameMode::APolitikaPrototypeGameMode()
{
    PlayerControllerClass = APolitikaPrototypePlayerController::StaticClass();
    DefaultPawnClass = APolitikaPrototypeCameraPawn::StaticClass();
    HUDClass = APolitikaPrototypeHUD::StaticClass();
}

void APolitikaPrototypeGameMode::StartPlay()
{
    Super::StartPlay();
    if (GetWorld() && !GetWorld()->GetAuthGameMode<APolitikaPrototypeGameMode>()) return;
    GetWorld()->SpawnActor<APolitikaPrototypeMatch>(FVector::ZeroVector, FRotator::ZeroRotator);
}
