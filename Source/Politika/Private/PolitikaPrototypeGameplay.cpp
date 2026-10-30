#include "PolitikaPrototypeGameplay.h"
#include "PolitikaInfantryPresentation.h"
#include "PolitikaInfantryBehavior.h"
#include "PolitikaTestMapMusicComponent.h"

#include "Camera/CameraComponent.h"
#include "Components/SceneComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "Engine/LocalPlayer.h"
#include "UnrealClient.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/DirectionalLight.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "NavigationSystem.h"
#include "NavigationData.h"
#include "GameFramework/PlayerState.h"
#include "TimerManager.h"
#include "InputCoreTypes.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "Misc/App.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "HAL/PlatformTime.h"
#include "UObject/ConstructorHelpers.h"
#include "PolitikaGameInstance.h"
#include "PolitikaPrototypeContent.h"
#include "Rules.h"
#if !UE_BUILD_SHIPPING
#include "PolitikaNavigationObstacleFixture.h"
#include "PolitikaNavigationFailureFixture.h"
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
        case EPolitikaPrototypeOrder::Grenade:
        case EPolitikaPrototypeOrder::Reload:
        case EPolitikaPrototypeOrder::Sprint: return politika::rules::ExecutionOrder::Idle;
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
    // Server-authoritative transform and public state reach observers whose team
    // currently sees the unit; IsNetRelevantFor applies the match's fog-of-war policy.
    bReplicates = true;
    bAlwaysRelevant = false;
    SetReplicatingMovement(true);
    InfantryPresentation = CreateDefaultSubobject<UPolitikaInfantryPresentationComponent>(TEXT("InfantryPresentation"));
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

void APolitikaPrototypeUnit::ApplyObservedArchetypePresentation(const UPolitikaUnitArchetypeDefinition* Definition)
{
    // Observer-side mirror of Configure's presentation half. Gameplay values arrive
    // through replication from authority and are never derived here.
    if (HasAuthority() || bObservedPresentationApplied || !Definition) return;
    bObservedPresentationApplied = true;
    Body->SetStaticMesh(Definition->VisualMesh);
    SetActorScale3D(Definition->VisualScale);
    if (Definition->bUseAuthoredMeshMaterials) Body->SetMaterial(0, nullptr);
    else ApplyPrototypeColor(Body, Definition->FactionTint);
    const FVector Scale = Definition->VisualScale;
    SelectionIndicator->SetStaticMesh(LoadPrototypeMesh(TEXT("/Engine/BasicShapes/Cylinder.Cylinder")));
    SelectionIndicator->SetRelativeLocation(FVector(0, 0, -34.0f / Scale.Z));
    SelectionIndicator->SetRelativeScale3D(FVector(2.1f / Scale.X, 2.1f / Scale.Y, 0.025f / Scale.Z));
    ApplyPrototypeColor(SelectionIndicator, FLinearColor(0.96f, 0.83f, 0.30f));
    // Read-only capability data for the local HUD (ammunition row); never simulated here.
    InfantryBehavior = Definition->InfantryBehavior;
    // Own-team sight drives this viewer's fog shading; the server keeps the authoritative copy.
    SetSightRadius(Definition->SightRadiusCm);
    ConfigureInfantryPresentation(Definition->InfantryPresentation);
    if (!IsAlive() && MaxHealth > 0) BeginDeathPresentation();
}

bool APolitikaPrototypeObjective::HasMarkerPresentation() const
{
    return Marker && Marker->GetStaticMesh();
}

void APolitikaPrototypeMatch::ApplyMapPresentation()
{
    // Ground tint and the map sun are local, non-replicated presentation on every
    // rendering process; a dedicated server has nothing to light.
    if (bMapPresentationApplied || !MapDefinition || !GetWorld() || GetNetMode() == NM_DedicatedServer) return;
    bMapPresentationApplied = true;
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
}

void APolitikaPrototypeUnit::ConfigureInfantryPresentation(UPolitikaInfantryPresentationDefinition* Definition)
{
    if (!Definition) return;
    InfantryPresentation->Initialize(Definition);
    // Retain the simple body's query/blocking geometry, but do not render it.
    Body->SetVisibility(false, false);
    const FVector Scale = GetActorScale3D();
    SelectionIndicator->SetRelativeScale3D(FVector(1.1f / Scale.X, 1.1f / Scale.Y, 0.02f / Scale.Z));
}

void APolitikaPrototypeUnit::BeginDeathPresentation()
{
    if (bDeathPresented) return;
    bDeathPresented = true;
    if (!InfantryPresentation->GetVisual())
    {
        // Authority retires the actor; observers hide their copy until the
        // replicated destruction arrives. A networked authority retires it after a short
        // delay so observers that saw it receive Health 0 first (no false last-known contact).
        if (HasAuthority() && GetNetMode() == NM_Standalone) Destroy();
        else
        {
            SetActorEnableCollision(false);
            SetActorHiddenInGame(true);
            if (HasAuthority()) SetLifeSpan(NetworkDeathRetireSeconds);
        }
        return;
    }
    SetActorEnableCollision(false);
    SelectionIndicator->SetVisibility(false);
    InfantryPresentation->Die(bBlastDeath);
}

void APolitikaPrototypeUnit::SetMoveOrder(EPolitikaPrototypeOrder Kind, const FVector& Destination,
    const FVector& ResolvedDestination, TArray<FVector>&& Route)
{
    GrenadeWindup = 0;
    GrenadeTarget.Reset();
    InfantryPresentation->CancelGrenade();
    CurrentOrder = Kind;
    PlayLocalAcknowledgement(TEXT("move"));
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
    GrenadeWindup = 0;
    GrenadeTarget.Reset();
    InfantryPresentation->CancelGrenade();
    PlayLocalAcknowledgement(TEXT("attack"));
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
    GrenadeWindup = 0;
    GrenadeTarget.Reset();
    InfantryPresentation->CancelGrenade();
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
    GrenadeWindup = 0;
    GrenadeTarget.Reset();
    InfantryPresentation->CancelGrenade();
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
    InfantryPresentation->SetSelected(bSelected);
    if (bSelected) InfantryPresentation->Acknowledge(TEXT("select"));
    SelectionIndicator->SetVisibility(bSelected, true);
}

politika::rules::OrderEndReason APolitikaPrototypeUnit::SimulateMovement(float DeltaSeconds)
{
    // Replicated observers never advance navigation; only the match authority does.
    if (!HasAuthority()) return politika::rules::OrderEndReason::None;
    StepStartPosition = GetActorLocation();
    if (!IsAlive() || MoveSpeed <= 0.0f) return politika::rules::OrderEndReason::None;
    if (InfantryBehavior && SprintRemaining <= 0 && CurrentOrder == EPolitikaPrototypeOrder::AttackMove)
    {
        if (!OrderTarget.IsValid()) AutoAcquireForAttackMove();
        const auto* Candidate = OrderTarget.Get();
        if (Candidate && Candidate->IsAlive() && Candidate->IsAttackable()
            && IsHostileTo(Candidate)
            && FlatDistance(GetActorLocation(), Candidate->GetActorLocation()) <= WeaponRange
            && HasLineOfFire(Candidate))
            return politika::rules::OrderEndReason::None;
    }
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

    float RemainingDistance = MoveSpeed * (SprintRemaining > 0 && InfantryBehavior ? InfantryBehavior->SprintMultiplier : 1) * DeltaSeconds;
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
    if (!HasAuthority()) return;
    APolitikaPrototypeUnit* Nearest = nullptr;
    float NearestDistance = InfantryBehavior ? InfantryBehavior->AcquisitionRangeCm : 800.0f;
    const APolitikaPrototypeMatch* Match = AuthorityMatch.Get();
    const bool bFog = Match && Match->IsFogOfWarEnabled();
    for (TActorIterator<APolitikaPrototypeUnit> It(GetWorld()); It; ++It)
    {
        APolitikaPrototypeUnit* Candidate = *It;
        // Automatic acquisition never crosses the (normally neutral) Unaligned boundary.
        if (!Candidate || Candidate == this || !Candidate->IsAlive() || !Candidate->IsAttackable()
            || !politika::rules::CanAutoEngage(OwnerPlayer, TeamId, Candidate->OwnerPlayer, Candidate->TeamId)) continue;
        // Under fog, only enemies the owning team currently observes can be acquired.
        if (bFog && !Candidate->IsObservedByTeam(TeamId)) continue;
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
    if (!HasAuthority() || !IsAlive()) return;
    if (InfantryBehavior)
    {
        bMovedThisStep = FVector::DistSquared2D(StepStartPosition, GetActorLocation()) > 0.01;
        SprintRemaining = FMath::Max(0.0f, SprintRemaining - DeltaSeconds);
        SprintCooldown = FMath::Max(0.0f, SprintCooldown - DeltaSeconds);
        GrenadeCooldown = FMath::Max(0.0f, GrenadeCooldown - DeltaSeconds);
        if (ReloadRemaining > 0)
        {
            ReloadRemaining = FMath::Max(0.0f, ReloadRemaining - DeltaSeconds);
            if (ReloadRemaining <= KINDA_SMALL_NUMBER)
            {
                ReloadRemaining = 0;
                Ammo = InfantryBehavior->MagazineCapacity;
                UE_LOG(LogPolitikaStartup, Display, TEXT("INFANTRY_RELOAD_COMPLETE unit=%llu ammo=%d"), StableId, Ammo);
            }
            return;
        }
        if (GrenadeWindup > 0)
        {
            auto* Victim = GrenadeTarget.Get();
            const APolitikaPrototypeMatch* Match = AuthorityMatch.Get();
            if (!Victim || !Victim->IsAlive() || !Victim->IsAttackable() || !IsHostileTo(Victim)
                || (Match && Match->IsFogOfWarEnabled() && !Victim->IsObservedByTeam(TeamId))
                || FlatDistance(GetActorLocation(), Victim->GetActorLocation()) > InfantryBehavior->GrenadeRangeCm || bMovedThisStep)
            { GrenadeWindup = 0; GrenadeTarget.Reset(); return; }
            GrenadeWindup = FMath::Max(0.0f, GrenadeWindup - DeltaSeconds);
            if (GrenadeWindup <= KINDA_SMALL_NUMBER)
            {
                GrenadeWindup = 0;
                for (TActorIterator<APolitikaPrototypeMatch> It(GetWorld()); It; ++It)
                    if (It->GetMatchPhase() == politika::rules::MatchPhase::Active)
                    { It->LaunchGrenade(this, Victim->GetActorLocation()); break; }
                GrenadeCooldown = InfantryBehavior->GrenadeReloadSeconds;
                GrenadeTarget.Reset();
            }
            return;
        }
        if (SprintRemaining > 0) return;
        if (Ammo == 0) { BeginReload(); return; }
    }
    CooldownRemaining = FMath::Max(0.0f, CooldownRemaining - DeltaSeconds);
    // Decimal authored intervals such as 0.2 s must not acquire an extra 0.05 s
    // step from a positive floating-point residue after four simulation steps.
    if (CooldownRemaining <= KINDA_SMALL_NUMBER) CooldownRemaining = 0;
    if ((CurrentOrder == EPolitikaPrototypeOrder::AttackMove
        || (InfantryBehavior && InfantryBehavior->bAutoAcquireWhenIdle && CurrentOrder == EPolitikaPrototypeOrder::Idle))
        && !OrderTarget.IsValid())
    {
        AutoAcquireForAttackMove();
    }
    else if (CurrentOrder == EPolitikaPrototypeOrder::Guard && !OrderTarget.IsValid())
    {
        AutoAcquireForAttackMove();
    }

    APolitikaPrototypeUnit* Target = OrderTarget.Get();
    if (!Target || !Target->IsAlive() || !IsHostileTo(Target))
    {
        if (CurrentOrder == EPolitikaPrototypeOrder::Attack) CurrentOrder = EPolitikaPrototypeOrder::Idle;
        OrderTarget.Reset();
        return;
    }
    // Never fire at a target the owning team cannot see; cleanup reports it as lost.
    if (const APolitikaPrototypeMatch* Match = AuthorityMatch.Get();
        Match && Match->IsFogOfWarEnabled() && !Target->IsObservedByTeam(TeamId))
        return;
    if (InfantryBehavior)
    {
        const FRotator Aim = (Target->GetActorLocation() - GetActorLocation()).Rotation();
        SetActorRotation(FMath::RInterpConstantTo(GetActorRotation(), FRotator(0, Aim.Yaw, 0), DeltaSeconds,
            InfantryBehavior->TurnDegreesPerSecond));
        if (bMovedThisStep || FMath::Abs(FMath::FindDeltaAngleDegrees(GetActorRotation().Yaw, Aim.Yaw))
            > InfantryBehavior->AcceptableAimDegrees || !HasLineOfFire(Target)) return;
    }
    if (Target->IsAttackable() && FlatDistance(GetActorLocation(), Target->GetActorLocation()) <= WeaponRange && CooldownRemaining <= 0.0f)
    {
        Target->PendingDamage += WeaponDamage;
        CooldownRemaining = WeaponCooldown;
        if (InfantryBehavior) --Ammo;
        if (APolitikaPrototypeMatch* Match = AuthorityMatch.Get())
            Match->DistributeShotPresentation(this, Target, WeaponCooldown);
        else InfantryPresentation->Shot(Target->GetActorLocation(), WeaponCooldown);
    }
}

int32 APolitikaPrototypeUnit::ApplyPendingDamage()
{
    if (!HasAuthority() || PendingDamage <= 0.0f || !IsAlive()) return 0;
    const int32 HealthBefore = Health;
    Health = FMath::Max(0, Health - FMath::CeilToInt(PendingDamage));
    PendingDamage = 0.0f;
    const int32 AppliedDamage = HealthBefore - Health;
#if !UE_BUILD_SHIPPING
    if (Health != HealthBefore)
        UE_LOG(LogPolitikaStartup, Display, TEXT("RTS_HEALTH_CHANGED unit=%llu before=%d after=%d"),
            StableId, HealthBefore, Health);
#endif
    if (Health == 0) { bBlastDeath = bPendingBlastDamage; StopOrder(); }
    bPendingBlastDamage = false;
    return AppliedDamage;
}

uint64 APolitikaPrototypeUnit::CleanupDeadTarget()
{
    if (!HasAuthority()) return 0;
    const APolitikaPrototypeMatch* Match = AuthorityMatch.Get();
    if (APolitikaPrototypeUnit* Target = OrderTarget.Get(); Target && (!Target->IsAlive()
        || (Match && Match->IsFogOfWarEnabled() && !Target->IsObservedByTeam(TeamId))))
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
    bReplicates = true;
    bAlwaysRelevant = true;
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
    ConfigurePresentation(Definition);
}

void APolitikaPrototypeObjective::ConfigurePresentation(const FPolitikaPrototypeObjectiveDefinition& Definition)
{
    Marker->SetStaticMesh(Definition.MarkerMesh);
    Marker->SetWorldScale3D(Definition.MarkerScale);
    ApplyPrototypeColor(Marker, Definition.MarkerTint);
}

void APolitikaPrototypeObjective::UpdateCapture(const TArray<APolitikaPrototypeUnit*>& Units, float DeltaSeconds)
{
    if (!HasAuthority() || bComplete) return;
#if !UE_BUILD_SHIPPING
    const bool bWasContested = bContested;
    const bool bWasComplete = bComplete;
    const bool bWasCapturing = Progress > 0.0f;
#endif
    // Presence is evaluated per side: a team, or an owner without a team (the Unaligned
    // and team-less fixtures). One eligible side alone in the radius captures; any second
    // side contests; allies never contest each other. Offline every participant is its own
    // team and only the local human's team is eligible, which is the original rule.
    auto SideOf = [](const APolitikaPrototypeUnit* Unit)
    {
        return Unit->GetTeamId() != politika::rules::kNoTeam
            ? static_cast<uint64>(Unit->GetTeamId()) : (uint64(1) << 32) | Unit->GetOwnerPlayer();
    };
    auto IsEligible = [this](const APolitikaPrototypeUnit* Unit)
    {
        return EligibleTeams.IsEmpty()
            ? Unit->GetOwnerPlayer() != 0 && Unit->GetOwnerPlayer() == LocalRuntimePlayerId
            : Unit->GetTeamId() != politika::rules::kNoTeam && EligibleTeams.Contains(Unit->GetTeamId());
    };
    TArray<uint64, TInlineAllocator<4>> PresentSides;
    const APolitikaPrototypeUnit* FirstEligible = nullptr;
    for (const APolitikaPrototypeUnit* Unit : Units)
    {
        if (!Unit || !Unit->IsAlive() || FlatDistance(Unit->GetActorLocation(), GetActorLocation()) > CaptureRadius) continue;
        PresentSides.AddUnique(SideOf(Unit));
        if (!FirstEligible && IsEligible(Unit)) FirstEligible = Unit;
    }
    bContested = FirstEligible && PresentSides.Num() > 1;
    if (PresentSides.Num() == 1 && FirstEligible)
    {
        if (CapturingPlayerId == 0 || CapturingTeamId != FirstEligible->GetTeamId())
        {
            // A different side starts from zero; progress is not transferable.
            if (CapturingPlayerId != 0) Progress = 0.0f;
            CapturingPlayerId = FirstEligible->GetOwnerPlayer();
            CapturingTeamId = FirstEligible->GetTeamId();
        }
        Progress = FMath::Min(1.0f, Progress + DeltaSeconds / CaptureDuration);
        bComplete = Progress >= 1.0f;
    }
#if !UE_BUILD_SHIPPING
    if (bContested != bWasContested || bComplete != bWasComplete || (!bWasCapturing && Progress > 0.0f))
        UE_LOG(LogPolitikaStartup, Display, TEXT("RTS_OBJECTIVE_STATE contested=%d progress_milli=%d complete=%d capturer=%u team=%u"),
            bContested ? 1 : 0, FMath::RoundToInt(Progress * 1000.0f), bComplete ? 1 : 0, CapturingPlayerId, CapturingTeamId);
#endif
}

FText APolitikaPrototypeObjective::GetStatusText() const
{
    // Networked sessions name the capturing team; the offline text is unchanged.
    const bool bNamed = GetNetMode() != NM_Standalone && CapturingTeamId != 0;
    const FString By = bNamed ? FString::Printf(TEXT("  ·  TEAM %c"), TCHAR('A' + CapturingTeamId - 1)) : FString();
    if (bComplete) return FText::FromString(TEXT("SECURED") + By);
    if (bContested) return FText::FromString(TEXT("CONTESTED"));
    if (Progress > 0.0f) return FText::FromString(FString::Printf(TEXT("CAPTURING %d%%%s"), FMath::RoundToInt(Progress * 100.0f), *By));
    return FText::FromString(TEXT("NEUTRAL"));
}

APolitikaPrototypeMatch::APolitikaPrototypeMatch()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickGroup = TG_PrePhysics;
    // The coordinator replicates only its explicit public match state (map identity,
    // generation, lifecycle, result, roster, unit/objective references). Simulation
    // never runs on an observer copy; see Tick and BeginPlay.
    bReplicates = true;
    bAlwaysRelevant = true;
    SetReplicatingMovement(false);
}

void APolitikaPrototypeMatch::BeginPlay()
{
    Super::BeginPlay();
    if (!HasAuthority())
    {
        // Remote observer: adopt unit actors that arrived first, then initialize
        // presentation from replicated content identity.
        for (TActorIterator<APolitikaPrototypeUnit> It(GetWorld()); It; ++It) RegisterObservedUnit(*It);
        InitializeObservedMatch();
        return;
    }
    MatchGeneration = NextPrototypeMatchGeneration++;
    BuildScenario();
#if !UE_BUILD_SHIPPING
    if (bContentReady && !IsNetworkSession()) BeginAcceptanceHarness();
    if (bContentReady && IsNetworkSession()) BeginMultiplayerAcceptance();
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
        InfantryProjectiles.Reset();
        StepAccumulator = 0.0f;
        if (Previous == politika::rules::MatchPhase::Active)
            for (APolitikaPrototypeUnit* Unit : Units)
                if (Unit && Unit->CanMove()) Unit->StopOrder();
    }
    UE_LOG(LogPolitikaStartup, Display, TEXT("MATCH_LIFECYCLE phase=%u"), static_cast<uint32>(NextPhase));
    SyncReplicatedLifecycle();
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
        else if (FixtureId == TEXT("DisconnectedNavigation")) RequestedMapId = TEXT("map.test.disconnected_navigation");
        else if (FixtureId == TEXT("UnprojectableDestination")) RequestedMapId = TEXT("map.test.unprojectable_destination");
        else if (FixtureId == TEXT("ProjectionDisplacement")) RequestedMapId = TEXT("map.test.projection_displacement");
        else if (FixtureId == TEXT("RifleInfantry")) RequestedMapId = TEXT("map.test.rifle_infantry");
        else if (FixtureId == TEXT("MultiplayerSkirmish")) RequestedMapId = TEXT("map.test.multiplayer_skirmish");
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
    // Session layout selects the playing seats and their teams before anything spawns.
    if (FName LayoutFailure; !ResolveSessionLayout(LayoutFailure))
    {
        LastCommandFeedback = TEXT("Prototype setup failed: the requested session layout is not supported by this map.");
        UE_LOG(LogPolitikaStartup, Error, TEXT("MATCH_ADMISSION_REJECTED map=%s reason=%s participants=%d"),
            *RequestedMapId.ToString(), *LayoutFailure.ToString(), RuntimeParticipants.Num());
        RecoverToShellAfterAdmissionFailure(RequestedMapId);
        return;
    }

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
    ApplyMapPresentation();

    // Player-slot units take the slot's participant/faction/team; Unaligned units are
    // owned by the system authority (owner 0) on its own NPC side in every layout.
    auto SpawnUnit = [this](const FPolitikaPrototypeUnitPlacement& Placement, bool bUnaligned)
    {
        const UPolitikaUnitArchetypeDefinition* Definition = ContentCatalog->FindUnit(Placement.UnitArchetypeId);
        const UPolitikaWeaponDefinition* Weapon = Definition ? ContentCatalog->FindWeapon(Definition->WeaponId) : nullptr;
        const FPolitikaPrototypeRuntimeParticipant* Slot = bUnaligned ? nullptr : RuntimeParticipants.FindByPredicate(
            [&](const FPolitikaPrototypeRuntimeParticipant& Candidate) { return Candidate.SlotId == Placement.SlotId; });
        if (!Definition || !Weapon || (!Slot && !bUnaligned)) return false;
        if (bUnaligned && Definition->FactionId != UnalignedSystemAuthority.FactionId) return false;
        APolitikaPrototypeUnit* Unit = GetWorld()->SpawnActor<APolitikaPrototypeUnit>(Placement.Location, Placement.Rotation);
        if (!Unit) return false;
        Unit->Configure(NextStableId++, Slot ? Slot->RuntimePlayerId : 0u, Definition->DisplayName, Definition->MaxHealth,
            Definition->MovementSpeedCmPerSecond, Weapon->RangeCm, Weapon->Damage, Weapon->CooldownSeconds,
            Definition->bAttackable, Definition->VisualMesh, Definition->VisualScale, Definition->FactionTint,
            Slot ? Slot->FactionId : UnalignedSystemAuthority.FactionId, Definition->bUseAuthoredMeshMaterials);
        Unit->ConfigureInfantryPresentation(Definition->InfantryPresentation);
        Unit->ConfigureInfantryBehavior(Definition->InfantryBehavior);
        Unit->SetArchetypeId(Definition->StableId);
        Unit->SetTeamId(Slot ? Slot->TeamId : politika::rules::kUnalignedTeam);
        Unit->SetSightRadius(Definition->SightRadiusCm);
        Unit->AuthorityMatch = this;
        Units.Add(Unit);
        return true;
    };

    TArray<FPolitikaPrototypeUnitPlacement> UnitPlacements = MapDefinition->UnitPlacements;
#if !UE_BUILD_SHIPPING
    // Development unit-scale fixture: extra seat units, placed after spatial validation.
    if (FName ScaleFailure; !ResolveUnitScale(UnitPlacements, ScaleFailure))
    {
        LastCommandFeedback = TEXT("Prototype setup failed: the requested unit-scale fixture is invalid for this session.");
        UE_LOG(LogPolitikaStartup, Error, TEXT("MATCH_ADMISSION_REJECTED map=%s reason=%s participants=%d"),
            *RequestedMapId.ToString(), *ScaleFailure.ToString(), RuntimeParticipants.Num());
        RecoverToShellAfterAdmissionFailure(RequestedMapId);
        return;
    }
#endif
    // Placements for authored slots outside the session layout do not spawn.
    TArray<int32> SpawnedPlacementIndices;
    int32 ExpectedUnits = 0;
    for (int32 Index = 0; Index < UnitPlacements.Num(); ++Index)
    {
        const FPolitikaPrototypeUnitPlacement& Placement = UnitPlacements[Index];
        if (!RuntimeParticipants.ContainsByPredicate([&Placement](const FPolitikaPrototypeRuntimeParticipant& Candidate)
            { return Candidate.SlotId == Placement.SlotId; }))
            continue;
        ++ExpectedUnits;
        if (SpawnUnit(Placement, false)) SpawnedPlacementIndices.Add(Index);
    }
    // The Unaligned side is present regardless of the player layout.
    int32 UnalignedUnits = 0;
    for (const FPolitikaPrototypeUnitPlacement& Placement : MapDefinition->UnalignedPlacements)
    {
        ++ExpectedUnits;
        UnalignedUnits += SpawnUnit(Placement, true) ? 1 : 0;
    }
    if (!MapDefinition->UnalignedPlacements.IsEmpty())
        UE_LOG(LogPolitikaStartup, Display, TEXT("MATCH_UNALIGNED_SIDE units=%d faction=%s system_controlled=1 team=%u"),
            UnalignedUnits, *UnalignedSystemAuthority.FactionId.ToString(), politika::rules::kUnalignedTeam);

    Objective = GetWorld()->SpawnActor<APolitikaPrototypeObjective>(MapDefinition->Objective.Location, FRotator::ZeroRotator);
    if (Objective) Objective->ConfigureFromDefinition(MapDefinition->Objective, LocalRuntimePlayerId);
    if (Objective)
    {
        // Offline only the local human's team captures (opponents contest); networked
        // sessions let every human seat's team capture.
        TArray<uint8> CapturingTeams;
        for (const FPolitikaPrototypeRuntimeParticipant& Participant : RuntimeParticipants)
            if (IsNetworkSession() || Participant.RuntimePlayerId == LocalRuntimePlayerId)
                CapturingTeams.AddUnique(Participant.TeamId);
        Objective->SetEligibleTeams(CapturingTeams);
    }
    bContentReady = Units.Num() == ExpectedUnits && Objective != nullptr;
    if (!bContentReady)
    {
        for (APolitikaPrototypeUnit* Unit : Units) if (Unit) Unit->Destroy();
        Units.Reset();
        if (Objective) Objective->Destroy();
        Objective = nullptr;
        LastCommandFeedback = TEXT("Prototype setup failed: validated content could not be instantiated.");
        UE_LOG(LogPolitikaStartup, Error, TEXT("CONTENT_INITIALIZATION_FAILED map=%s expected_units=%d actual_units=%d"), *MapDefinition->StableId.ToString(), ExpectedUnits, Units.Num());
        RecoverToShellAfterAdmissionFailure(RequestedMapId);
        return;
    }
    ReplicatedMapId = MapDefinition->StableId;
    SyncReplicatedParticipants();
    // Initial team vision before any connection is considered for replication.
    UpdateTeamVisibility();
    if (!TransitionLifecycle(politika::rules::MatchPhase::Starting))
    {
        LastCommandFeedback = TEXT("Prototype setup failed: match could not enter active play.");
        RecoverToShellAfterAdmissionFailure(RequestedMapId);
        return;
    }
    if (IsNetworkSession())
    {
        // A networked match starts only when every human seat is bound to a live
        // connection; it does not admit late joiners once Active.
        bAwaitingNetworkParticipants = true;
        LastCommandFeedback = FString::Printf(TEXT("Hosting %s: waiting for %u remote participant(s) to join."),
            UTF8_TO_TCHAR(politika::rules::LayoutName(SessionLayout).data()), GetMaxRemoteHumans());
        UE_LOG(LogPolitikaStartup, Display, TEXT("MP_MATCH_AWAITING_PARTICIPANTS generation=%llu map=%s port=%d layout=%s remote_seats=%u"),
            MatchGeneration, *MapDefinition->StableId.ToString(), GetWorld()->URL.Port,
            UTF8_TO_TCHAR(politika::rules::LayoutName(SessionLayout).data()), GetMaxRemoteHumans());
    }
    else if (!TransitionLifecycle(politika::rules::MatchPhase::Active))
    {
        LastCommandFeedback = TEXT("Prototype setup failed: match could not enter active play.");
        RecoverToShellAfterAdmissionFailure(RequestedMapId);
        return;
    }
    BindExistingControllers();
#if !UE_BUILD_SHIPPING
    for (int32 SpawnIndex = 0; SpawnIndex < SpawnedPlacementIndices.Num(); ++SpawnIndex)
    {
        const int32 Index = SpawnedPlacementIndices[SpawnIndex];
        const FPolitikaPrototypeUnitPlacement& Placement = UnitPlacements[Index];
        const UPolitikaUnitArchetypeDefinition* Definition = ContentCatalog->FindUnit(Placement.UnitArchetypeId);
        const UPolitikaWeaponDefinition* Weapon = Definition ? ContentCatalog->FindWeapon(Definition->WeaponId) : nullptr;
        const FPolitikaPrototypeRuntimeParticipant* Slot = RuntimeParticipants.FindByPredicate(
            [&Placement](const FPolitikaPrototypeRuntimeParticipant& Candidate) { return Candidate.SlotId == Placement.SlotId; });
        const APolitikaPrototypeUnit* Unit = Units.IsValidIndex(SpawnIndex) ? Units[SpawnIndex] : nullptr;
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
    InfantryProjectiles.Reset();
    for (FParticipantBinding& Binding : ParticipantBindings)
        if (auto* Prototype = Cast<APolitikaPrototypePlayerController>(Binding.Controller.Get()))
            Prototype->ClearServerAssignment();
    ParticipantBindings.Reset();
    OutcomeRoutes.Reset();
    bAwaitingNetworkParticipants = false;
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

    if (GetNetMode() == NM_DedicatedServer)
    {
        // A headless server has no menu to recover to; refusing to host is the truthful
        // outcome. Exit non-zero so an operator or supervisor sees the failure.
        UE_LOG(LogPolitikaStartup, Error, TEXT("MP_DEDICATED_ADMISSION_FAILED map=%s exit_code=3"), *MapId.ToString());
        if (GLog) GLog->Flush();
        FPlatformMisc::RequestExitWithStatus(false, 3);
        return;
    }
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
    // Observers leave by travelling; only authority mutates the match it owns.
    if (!HasAuthority()) return;
    PendingOrders.Reset();
    if (Lifecycle.phase == politika::rules::MatchPhase::Active
        || Lifecycle.phase == politika::rules::MatchPhase::Results)
        TransitionLifecycle(politika::rules::MatchPhase::Returning);
    if (Lifecycle.phase == politika::rules::MatchPhase::Returning)
    {
        if (APlayerController* Player = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr)
            if (APolitikaPrototypePlayerController* PrototypePlayer = Cast<APolitikaPrototypePlayerController>(Player))
            {
                PrototypePlayer->ClearSelectionState();
                PrototypePlayer->bMapCameraApplied = false;
            }
        // Revoke every connection binding and outstanding outcome route so nothing
        // from this generation can reach a later match.
        for (FParticipantBinding& Binding : ParticipantBindings)
            if (auto* Prototype = Cast<APolitikaPrototypePlayerController>(Binding.Controller.Get()))
                Prototype->ClearServerAssignment();
        ParticipantBindings.Reset();
        OutcomeRoutes.Reset();
        bAwaitingNetworkParticipants = false;
        SelectionByPlayer.Reset();
        RuntimeParticipants.Reset();
        SyncReplicatedParticipants();
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
    UpdateLocalFogKnowledge();
    if (!HasAuthority())
    {
        // Observer copies interpolate replicated state; they never simulate.
#if !UE_BUILD_SHIPPING
        ++ClientWindowFrames;
        ClientWindowTotalMillis += DeltaSeconds * 1000.0;
        ClientWindowMaxMillis = FMath::Max(ClientWindowMaxMillis, DeltaSeconds * 1000.0f);
        SampleObservedState(TEXT("periodic"));
        SampleClientNetworkStats();
#endif
        return;
    }
    if (!bContentReady || !Lifecycle.IsActive()) return;
#if !UE_BUILD_SHIPPING
    const bool bScheduled = NavigationAcceptance.bRunning && NavigationAcceptance.Schedule != 0;
    const uint64 FirstStep = SimulationStepsExecuted;
    const bool bMeasureNetwork = IsNetworkSession();
    if (bMeasureNetwork) ServerFrameMillis.Add(DeltaSeconds * 1000.0f);
    if (bScheduled)
        DeltaSeconds = PolitikaNavigationObstacleFixture::ScheduleDelta(
            NavigationAcceptance.Schedule, NavigationAcceptance.OuterUpdates);
#endif
    StepAccumulator += FMath::Min(DeltaSeconds, 0.25f);
    int32 Steps = 0;
    if (StepAccumulator >= SimulationStepSeconds) PublishedEvents.Reset();
    while (Lifecycle.IsActive() && StepAccumulator >= SimulationStepSeconds && Steps < 5)
    {
#if !UE_BUILD_SHIPPING
        const double StepStartSeconds = FPlatformTime::Seconds();
#endif
        PendingGameplayEvents.Reset();
        AdmitPendingOrders();
        RunSimulationStep(SimulationStepSeconds);
        StepAccumulator -= SimulationStepSeconds;
        ++Steps;
#if !UE_BUILD_SHIPPING
        if (bMeasureNetwork) StepCostMicros.Add(static_cast<float>((FPlatformTime::Seconds() - StepStartSeconds) * 1.0e6));
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
    if (bMeasureNetwork)
    {
        CatchUpCapFrames += Steps == 5 ? 1 : 0;
        TickUnitScaleDriver();
        SampleMultiplayerState(TEXT("periodic"));
        // The result commits inside a step; report after the loop so that step is counted.
        if (CommittedResult.IsSet()) ReportNetworkMeasurements(TEXT("result"));
        else SampleNetworkStats(TEXT("periodic"));
    }
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
    if (MapDefinition && MapDefinition->StableId == TEXT("map.test.disconnected_navigation"))
    {
        BeginNavigationFailureAcceptance();
        return;
    }
    if (MapDefinition && MapDefinition->StableId == TEXT("map.test.unprojectable_destination"))
    {
        BeginNavigationFailureAcceptance();
        return;
    }
    if (MapDefinition && MapDefinition->StableId == TEXT("map.test.projection_displacement"))
    {
        BeginNavigationFailureAcceptance();
        return;
    }
    if (MapDefinition && MapDefinition->StableId == TEXT("map.test.static_obstacle_detour"))
    {
        BeginNavigationAcceptance();
        return;
    }
    FParse::Value(FCommandLine::Get(), TEXT("RTSAcceptance="), AcceptanceMode);
    AcceptanceMode = AcceptanceMode.ToLower();
    if (AcceptanceMode == TEXT("participantauthority"))
    {
        BeginParticipantAuthorityAcceptance();
        return;
    }
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

void APolitikaPrototypeMatch::BeginParticipantAuthorityAcceptance()
{
    APolitikaPrototypePlayerController* Controller = GetWorld()
        ? Cast<APolitikaPrototypePlayerController>(GetWorld()->GetFirstPlayerController()) : nullptr;
    APolitikaPrototypeUnit* LocalUnit = nullptr;
    APolitikaPrototypeUnit* ForeignUnit = nullptr;
    for (APolitikaPrototypeUnit* Unit : Units)
    {
        if (!IsValid(Unit)) continue;
        if (Unit->GetOwnerPlayer() == LocalRuntimePlayerId && !LocalUnit) LocalUnit = Unit;
        else if (Unit->GetOwnerPlayer() != LocalRuntimePlayerId && !ForeignUnit) ForeignUnit = Unit;
    }
    if (!Controller || !LocalUnit || !ForeignUnit || !IsLocalControllerBound(Controller))
    {
        UE_LOG(LogPolitikaStartup, Error,
            TEXT("RTS_ACCEPTANCE_PARTICIPANT_AUTHORITY result=FAIL reason=fixture_missing bound=%d local=%d foreign=%d"),
            IsLocalControllerBound(Controller) ? 1 : 0, LocalUnit ? 1 : 0, ForeignUnit ? 1 : 0);
        AcceptancePhase = EAcceptancePhase::Done;
        return;
    }

    ParticipantAuthorityAcceptance = {};
    FParticipantAuthorityAcceptance& Acceptance = ParticipantAuthorityAcceptance;
    Acceptance.LocalUnit = LocalUnit;
    Acceptance.ForeignUnit = ForeignUnit;
    Acceptance.ForeignEntityId = ForeignUnit->GetStableId();
    Acceptance.ForeignSeedSequence = NextCommandSequence;
    Acceptance.bRunning = true;
    SubmitCommand(ForeignUnit->GetOwnerPlayer(), ForeignUnit, nullptr, politika::rules::GuardOrder{});
    AcceptancePhase = EAcceptancePhase::ParticipantAuthoritySeed;
}

void APolitikaPrototypeMatch::ContinueParticipantAuthorityAcceptance()
{
    FParticipantAuthorityAcceptance& Acceptance = ParticipantAuthorityAcceptance;
    APolitikaPrototypePlayerController* Controller = GetWorld()
        ? Cast<APolitikaPrototypePlayerController>(GetWorld()->GetFirstPlayerController()) : nullptr;
    APolitikaPrototypeUnit* LocalUnit = Acceptance.LocalUnit.Get();
    APolitikaPrototypeUnit* ForeignUnit = Acceptance.ForeignUnit.Get();
    if (!Controller || !LocalUnit || !ForeignUnit || !IsLocalControllerBound(Controller))
    {
        UE_LOG(LogPolitikaStartup, Error,
            TEXT("RTS_ACCEPTANCE_PARTICIPANT_AUTHORITY result=FAIL reason=seeded_fixture_invalid bound=%d local=%d foreign=%d"),
            IsLocalControllerBound(Controller) ? 1 : 0, LocalUnit ? 1 : 0, ForeignUnit ? 1 : 0);
        Acceptance.bRunning = false;
        AcceptancePhase = EAcceptancePhase::Done;
        return;
    }

    Acceptance.ForeignRouteSequence = ForeignUnit->NavigationOrderSequence;
    Acceptance.ForeignWaypointIndex = ForeignUnit->NavigationWaypointIndex;
    Acceptance.ForeignRoutePoints = ForeignUnit->NavigationWaypoints;
    Acceptance.ForeignPosition = ForeignUnit->GetActorLocation();
    Acceptance.bForeignBaselineCaptured = true;
    Acceptance.ForeignOrderChanges = 0;
    Acceptance.StartStep = SimulationStepsExecuted;

    Controller->SelectUnitForInput(LocalUnit);
    const bool bLocalSelectionAccepted = GetSelectedUnitForPlayer(LocalRuntimePlayerId) == LocalUnit;
    Acceptance.LocalSequence = NextCommandSequence;
    const bool bLocalForwarded = SubmitLocalControllerCommand(
        Controller, LocalUnit, nullptr, politika::rules::GuardOrder{});
    Controller->SelectUnitForInput(ForeignUnit);
    const bool bForeignSelectionRejected = GetSelectedUnitForPlayer(LocalRuntimePlayerId) == nullptr;
    Controller->SelectUnitForInput(LocalUnit);
    Acceptance.ForeignSequence = NextCommandSequence;
    const bool bForeignForwarded = SubmitLocalControllerCommand(
        Controller, ForeignUnit, nullptr, politika::rules::MoveOrder{{100.0, 100.0}});

    const FPolitikaPrototypeRuntimeParticipant* LocalParticipant = RuntimeParticipants.FindByPredicate(
        [this](const FPolitikaPrototypeRuntimeParticipant& Candidate)
        { return Candidate.RuntimePlayerId == LocalRuntimePlayerId; });
    const FName LocalFaction = LocalParticipant ? LocalParticipant->FactionId : NAME_None;
    Acceptance.bSystemRecordValid = UnalignedSystemAuthority.bNpcSystemControlled
        && UnalignedSystemAuthority.FactionId == FName(TEXT("faction.unaligned"))
        && !RuntimeParticipants.ContainsByPredicate([](const FPolitikaPrototypeRuntimeParticipant& Candidate)
        { return Candidate.RuntimePlayerId == 0 || Candidate.FactionId == FName(TEXT("faction.unaligned")); });
    APolitikaPrototypeUnit* SystemUnit = GetWorld()->SpawnActor<APolitikaPrototypeUnit>(FVector(1500, 1200, UnitGroundZ), FRotator::ZeroRotator);
    APolitikaPrototypeUnit* DeadUnit = GetWorld()->SpawnActor<APolitikaPrototypeUnit>(FVector(1600, 1200, UnitGroundZ), FRotator::ZeroRotator);
    APolitikaPrototypeUnit* DestroyedUnit = GetWorld()->SpawnActor<APolitikaPrototypeUnit>(FVector(1700, 1200, UnitGroundZ), FRotator::ZeroRotator);
    if (!LocalParticipant || !SystemUnit || !DeadUnit || !DestroyedUnit)
    {
        UE_LOG(LogPolitikaStartup, Error, TEXT("RTS_ACCEPTANCE_PARTICIPANT_AUTHORITY result=FAIL reason=actor_spawn"));
        Acceptance.bRunning = false;
        AcceptancePhase = EAcceptancePhase::Done;
        return;
    }
    auto ConfigureAuthorityFixture = [this, &LocalFaction](APolitikaPrototypeUnit* Unit, uint32 Owner, FName Faction)
    {
        Unit->Configure(NextStableId++, Owner, FText::FromString(TEXT("Development authority fixture")),
            100, 100, 200, 10, 0.5f, true, nullptr, FVector::OneVector, FLinearColor::White, Faction);
        Units.Add(Unit);
    };
    ConfigureAuthorityFixture(SystemUnit, 0, TEXT("faction.unaligned"));
    ConfigureAuthorityFixture(DeadUnit, LocalRuntimePlayerId, LocalFaction);
    ConfigureAuthorityFixture(DestroyedUnit, LocalRuntimePlayerId, LocalFaction);
    Acceptance.SystemUnit = SystemUnit;
    Acceptance.DeadUnit = DeadUnit;
    Acceptance.DestroyedUnit = DestroyedUnit;
    Acceptance.SystemEntityId = SystemUnit->GetStableId();
    Acceptance.DeadEntityId = DeadUnit->GetStableId();
    Acceptance.DestroyedEntityId = DestroyedUnit->GetStableId();

    Acceptance.SystemSequence = NextCommandSequence;
    Controller->SelectUnitForInput(SystemUnit);
    const bool bSystemSelectionRejected = GetSelectedUnitForPlayer(LocalRuntimePlayerId) == nullptr;
    Controller->SelectUnitForInput(LocalUnit);
    const bool bSystemForwarded = SubmitLocalControllerCommand(
        Controller, SystemUnit, nullptr, politika::rules::GuardOrder{});
    Acceptance.UnassignedSequence = NextCommandSequence;
    SubmitCommand(0, SystemUnit, nullptr, politika::rules::GuardOrder{});
    const uint64 BeforeUnboundSequence = NextCommandSequence;
    const bool bUnboundRejected = !SubmitLocalControllerCommand(
        nullptr, LocalUnit, nullptr, politika::rules::StopOrder{}) && NextCommandSequence == BeforeUnboundSequence;

    Controller->SelectUnitForInput(DeadUnit);
    DeadUnit->SetGuardOrder();
    DeadUnit->PendingDamage = static_cast<float>(DeadUnit->GetHealth());
    const int32 DeadDamage = DeadUnit->ApplyPendingDamage();
    const uint64 BeforeStaleInputSequence = NextCommandSequence;
    Controller->GuardSelected();
    const bool bDeadSelectionRetired = DeadDamage == 100 && !DeadUnit->IsAlive()
        && GetSelectedUnitForPlayer(LocalRuntimePlayerId) == nullptr
        && NextCommandSequence == BeforeStaleInputSequence;
    Acceptance.DeadSequence = NextCommandSequence;
    const bool bDeadForwarded = SubmitLocalControllerCommand(
        Controller, DeadUnit, nullptr, politika::rules::GuardOrder{});

    Controller->SelectUnitForInput(DestroyedUnit);
    Acceptance.DestroyedSequence = NextCommandSequence;
    const bool bDestroyedForwarded = SubmitLocalControllerCommand(
        Controller, DestroyedUnit, nullptr, politika::rules::GuardOrder{});
    const bool bDestroyedWasQueued = !PendingOrders.IsEmpty() && PendingOrders.Last().Request.sequence == Acceptance.DestroyedSequence;
    DestroyedUnit->Destroy();
    Acceptance.bSetupChecksPassed = Acceptance.bSystemRecordValid
        && SystemUnit->GetOwnerPlayer() == 0 && SystemUnit->GetFactionId() == FName(TEXT("faction.unaligned"))
        && bLocalSelectionAccepted && bLocalForwarded && bForeignSelectionRejected
        && bForeignForwarded && bSystemSelectionRejected && bSystemForwarded && bUnboundRejected
        && bDeadSelectionRetired && bDeadForwarded && bDestroyedForwarded && bDestroyedWasQueued;

    UE_LOG(LogPolitikaStartup, Display,
        TEXT("RTS_ACCEPTANCE_PARTICIPANT_AUTHORITY_ARMED local=%llu local_owner=%u local_faction=%s foreign=%llu foreign_owner=%u foreign_faction=%s system=%llu system_owner=%u system_faction=%s unaligned_record=%d dead=%llu destroyed=%llu bound=1 local_forwarded=%d foreign_forwarded=%d foreign_selection_rejected=%d system_forwarded=%d unbound_rejected=%d dead_selection_retired=%d dead_forwarded=%d destroyed_forwarded=%d destroyed_queued=%d"),
        LocalUnit->GetStableId(), LocalUnit->GetOwnerPlayer(), *LocalUnit->GetFactionId().ToString(),
        ForeignUnit->GetStableId(), ForeignUnit->GetOwnerPlayer(), *ForeignUnit->GetFactionId().ToString(),
        Acceptance.SystemEntityId, SystemUnit->GetOwnerPlayer(), *SystemUnit->GetFactionId().ToString(),
        Acceptance.bSystemRecordValid ? 1 : 0,
        Acceptance.DeadEntityId, Acceptance.DestroyedEntityId,
        bLocalForwarded ? 1 : 0, bForeignForwarded ? 1 : 0, bForeignSelectionRejected ? 1 : 0,
        bSystemForwarded ? 1 : 0, bUnboundRejected ? 1 : 0, bDeadSelectionRetired ? 1 : 0,
        bDeadForwarded ? 1 : 0, bDestroyedForwarded ? 1 : 0, bDestroyedWasQueued ? 1 : 0);
    AcceptancePhase = EAcceptancePhase::ParticipantAuthority;
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

    if (AcceptancePhase == EAcceptancePhase::ParticipantAuthoritySeed)
    {
        if (ParticipantAuthorityAcceptance.ForeignSeedOutcomes > 0)
        {
            if (ParticipantAuthorityAcceptance.ForeignSeedOutcomes != 1
                || !ParticipantAuthorityAcceptance.bForeignSeedAccepted)
            {
                UE_LOG(LogPolitikaStartup, Error,
                    TEXT("RTS_ACCEPTANCE_PARTICIPANT_AUTHORITY result=FAIL reason=foreign_seed_rejected outcomes=%d accepted=%d"),
                    ParticipantAuthorityAcceptance.ForeignSeedOutcomes,
                    ParticipantAuthorityAcceptance.bForeignSeedAccepted ? 1 : 0);
                ParticipantAuthorityAcceptance.bRunning = false;
                AcceptancePhase = EAcceptancePhase::Done;
                return;
            }
            ContinueParticipantAuthorityAcceptance();
        }
        return;
    }

    if (AcceptancePhase == EAcceptancePhase::ParticipantAuthority)
    {
        const uint64 ObservedSteps = SimulationStepsExecuted - ParticipantAuthorityAcceptance.StartStep;
        if (ObservedSteps >= 20)
        {
            APolitikaPrototypePlayerController* Controller = GetWorld()
                ? Cast<APolitikaPrototypePlayerController>(GetWorld()->GetFirstPlayerController()) : nullptr;
            APolitikaPrototypeUnit* ForeignUnit = ParticipantAuthorityAcceptance.ForeignUnit.Get();
            APolitikaPrototypeUnit* SystemUnit = ParticipantAuthorityAcceptance.SystemUnit.Get();
            const bool bPassed = ParticipantAuthorityAcceptance.bSetupChecksPassed
                && ParticipantAuthorityAcceptance.bSystemRecordValid
                && ParticipantAuthorityAcceptance.LocalOutcomes == 1 && ParticipantAuthorityAcceptance.bLocalAccepted
                && ParticipantAuthorityAcceptance.ForeignOutcomes == 1
                && ParticipantAuthorityAcceptance.ForeignRejection == politika::rules::OrderRejection::NotOwned
                && ParticipantAuthorityAcceptance.SystemOutcomes == 1
                && ParticipantAuthorityAcceptance.SystemRejection == politika::rules::OrderRejection::NotOwned
                && ParticipantAuthorityAcceptance.UnassignedOutcomes == 1
                && ParticipantAuthorityAcceptance.UnassignedRejection == politika::rules::OrderRejection::Unauthorized
                && ParticipantAuthorityAcceptance.DeadOutcomes == 1
                && ParticipantAuthorityAcceptance.DeadRejection == politika::rules::OrderRejection::IssuerUnavailable
                && ParticipantAuthorityAcceptance.DestroyedOutcomes == 1
                && ParticipantAuthorityAcceptance.DestroyedRejection == politika::rules::OrderRejection::MissingIssuer
                && ParticipantAuthorityAcceptance.ForeignOrderChanges == 0
                && ForeignUnit && ForeignUnit->GetCurrentOrder() == EPolitikaPrototypeOrder::Guard
                && ForeignUnit->NavigationOrderSequence == ParticipantAuthorityAcceptance.ForeignRouteSequence
                && ForeignUnit->NavigationWaypointIndex == ParticipantAuthorityAcceptance.ForeignWaypointIndex
                && ForeignUnit->NavigationWaypoints == ParticipantAuthorityAcceptance.ForeignRoutePoints
                && ForeignUnit->GetActorLocation().Equals(ParticipantAuthorityAcceptance.ForeignPosition, 0.01f)
                && SystemUnit && SystemUnit->GetCurrentOrder() == EPolitikaPrototypeOrder::Idle
                && !ParticipantAuthorityAcceptance.DeadUnit.IsValid()
                && !ParticipantAuthorityAcceptance.DestroyedUnit.IsValid()
                && PendingOrders.IsEmpty() && Controller
                && GetSelectedUnitForPlayer(LocalRuntimePlayerId) == nullptr;
            UE_LOG(LogPolitikaStartup, Display,
                TEXT("RTS_ACCEPTANCE_PARTICIPANT_AUTHORITY result=%s steps=%llu local=%d foreign=%d system=%d unassigned=%d dead=%d destroyed=%d foreign_order_changes=%d pending=%d unaligned_record=%d system_owner=%u system_faction=%s"),
                bPassed ? TEXT("PASS") : TEXT("FAIL"), ObservedSteps,
                ParticipantAuthorityAcceptance.LocalOutcomes, ParticipantAuthorityAcceptance.ForeignOutcomes,
                ParticipantAuthorityAcceptance.SystemOutcomes, ParticipantAuthorityAcceptance.UnassignedOutcomes,
                ParticipantAuthorityAcceptance.DeadOutcomes, ParticipantAuthorityAcceptance.DestroyedOutcomes,
                ParticipantAuthorityAcceptance.ForeignOrderChanges, PendingOrders.Num(),
                ParticipantAuthorityAcceptance.bSystemRecordValid ? 1 : 0,
                SystemUnit ? SystemUnit->GetOwnerPlayer() : 999u,
                SystemUnit ? *SystemUnit->GetFactionId().ToString() : TEXT("invalid"));
            ParticipantAuthorityAcceptance.bRunning = false;
            AcceptancePhase = EAcceptancePhase::Done;
        }
        return;
    }

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

bool APolitikaPrototypeMatch::IsCommandableUnit(const APolitikaPrototypeUnit* Unit, uint32 PlayerId) const
{
    if (!IsValid(Unit) || !Unit->IsAlive() || PlayerId == 0 || !Units.Contains(const_cast<APolitikaPrototypeUnit*>(Unit)))
        return false;
    if (Unit->GetOwnerPlayer() != PlayerId) return false;
    const FPolitikaPrototypeRuntimeParticipant* Participant = RuntimeParticipants.FindByPredicate(
        [PlayerId](const FPolitikaPrototypeRuntimeParticipant& Candidate)
        { return Candidate.RuntimePlayerId == PlayerId; });
    // Human seats (local or connection-bound remote) are selectable; assigned
    // opponents and the Unaligned system authority never are.
    return Participant && (Participant->ControllerKind == EPolitikaPrototypeControllerKind::LocalHuman
            || Participant->ControllerKind == EPolitikaPrototypeControllerKind::RemoteHuman)
        && Participant->FactionId == Unit->GetFactionId();
}

TArray<APolitikaPrototypeUnit*> APolitikaPrototypeMatch::GetSelectedUnitsForPlayer(uint32 PlayerId)
{
    TArray<APolitikaPrototypeUnit*> Selected;
    FPolitikaSelectionSet* Found = SelectionByPlayer.Find(PlayerId);
    if (!Found) return Selected;
    const int32 Before = Found->Units.Num();
    for (int32 Index = Found->Units.Num() - 1; Index >= 0; --Index)
    {
        APolitikaPrototypeUnit* Unit = Found->Units[Index].Get();
        if (Unit && IsCommandableUnit(Unit, PlayerId)) continue;
        // Retired exactly once: the reference and its visual leave together.
        if (Unit) Unit->SetSelectedVisual(false);
        Found->Units.RemoveAt(Index);
    }
    for (const TWeakObjectPtr<APolitikaPrototypeUnit>& Unit : Found->Units) Selected.Add(Unit.Get());
#if !UE_BUILD_SHIPPING
    if (Found->Units.Num() != Before)
        UE_LOG(LogPolitikaStartup, Display, TEXT("RTS_SELECTION_RETIRED player=%u retired=%d count=%d"), PlayerId,
            Before - Found->Units.Num(), Found->Units.Num());
#endif
    if (Found->Units.IsEmpty()) SelectionByPlayer.Remove(PlayerId);
    return Selected;
}

APolitikaPrototypeUnit* APolitikaPrototypeMatch::GetSelectedUnitForPlayer(uint32 PlayerId)
{
    const TArray<APolitikaPrototypeUnit*> Selected = GetSelectedUnitsForPlayer(PlayerId);
    return Selected.IsEmpty() ? nullptr : Selected[0];
}

void APolitikaPrototypeMatch::SetSelectionSet(uint32 PlayerId, TConstArrayView<APolitikaPrototypeUnit*> Candidates)
{
    const TArray<APolitikaPrototypeUnit*> Previous = GetSelectedUnitsForPlayer(PlayerId);
    TArray<APolitikaPrototypeUnit*> Next;
    bool bHumanSeat = true;
    if (!RuntimeParticipants.IsEmpty())
    {
        const FPolitikaPrototypeRuntimeParticipant* Participant = RuntimeParticipants.FindByPredicate(
            [PlayerId](const FPolitikaPrototypeRuntimeParticipant& Candidate) { return Candidate.RuntimePlayerId == PlayerId; });
        bHumanSeat = Participant && (Participant->ControllerKind == EPolitikaPrototypeControllerKind::LocalHuman
            || Participant->ControllerKind == EPolitikaPrototypeControllerKind::RemoteHuman);
    }
    if (bHumanSeat)
        for (APolitikaPrototypeUnit* Unit : Candidates)
            if (IsCommandableUnit(Unit, PlayerId)) Next.AddUnique(Unit);
    Next.Sort([](const APolitikaPrototypeUnit& A, const APolitikaPrototypeUnit& B) { return A.GetStableId() < B.GetStableId(); });
    for (APolitikaPrototypeUnit* Unit : Previous)
        if (!Next.Contains(Unit)) Unit->SetSelectedVisual(false);
    for (APolitikaPrototypeUnit* Unit : Next)
        if (!Previous.Contains(Unit)) Unit->SetSelectedVisual(true);
    if (Next.IsEmpty())
    {
        SelectionByPlayer.Remove(PlayerId);
        return;
    }
    FPolitikaSelectionSet& Set = SelectionByPlayer.FindOrAdd(PlayerId);
    Set.Units.Reset(Next.Num());
    for (APolitikaPrototypeUnit* Unit : Next) Set.Units.Add(Unit);
}

void APolitikaPrototypeMatch::SetSelection(uint32 PlayerId, APolitikaPrototypeUnit* Unit)
{
    if (Unit) SetSelectionSet(PlayerId, MakeArrayView(&Unit, 1));
    else SetSelectionSet(PlayerId, {});
#if !UE_BUILD_SHIPPING
    if (Unit && GetSelectedUnitForPlayer(PlayerId) == Unit)
        UE_LOG(LogPolitikaStartup, Display, TEXT("RTS_SELECTION player=%u unit=%llu"), PlayerId, Unit->GetStableId());
#endif
}

FVector2D APolitikaPrototypeMatch::GetCameraPanMin() const
{
    return MapDefinition ? MapDefinition->CameraPanMin : PlayableMin;
}

FVector2D APolitikaPrototypeMatch::GetCameraPanMax() const
{
    return MapDefinition ? MapDefinition->CameraPanMax : PlayableMax;
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

uint64 APolitikaPrototypeMatch::SubmitCommand(uint32 PlayerId, APolitikaPrototypeUnit* Unit,
    APolitikaPrototypeUnit* Target, const politika::rules::Order& Intent, uint64 RequestedIssuerId,
    uint64 CorrelationId, TOptional<uint64> GenerationHint)
{
    APolitikaPrototypeUnit* ValidUnit = IsValid(Unit) ? Unit : nullptr;
    FPendingOrder Pending;
    Pending.Request.match_generation = GenerationHint.Get(MatchGeneration);
    Pending.Request.sequence = NextCommandSequence++;
    Pending.Request.participant_id = PlayerId;
    Pending.Request.issuer_entity_id = ValidUnit ? ValidUnit->GetStableId() : RequestedIssuerId;
    Pending.Request.correlation_id = CorrelationId != 0 ? CorrelationId : Pending.Request.sequence;
    Pending.Request.intent = Intent;
    if (!Lifecycle.IsActive())
    {
        LastCommandFeedback = TEXT("The match is not accepting commands.");
        PublishGameplayEvent(politika::rules::CommandOutcomeEvent{
            SimulationStepsExecuted, Pending.Request.sequence, Pending.Request.correlation_id,
            Pending.Request.issuer_entity_id, politika::rules::KindOf(Pending.Request.intent), false,
            politika::rules::OrderRejection::MatchInactive});
        return Pending.Request.sequence;
    }
    if ((Unit && !ValidUnit) || (!ValidUnit && RequestedIssuerId != 0))
    {
        LastCommandFeedback = TEXT("The selected unit no longer exists.");
        PublishGameplayEvent(politika::rules::CommandOutcomeEvent{
            SimulationStepsExecuted, Pending.Request.sequence, Pending.Request.correlation_id,
            Pending.Request.issuer_entity_id, politika::rules::KindOf(Pending.Request.intent), false,
            politika::rules::OrderRejection::MissingIssuer});
        return Pending.Request.sequence;
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
            return Pending.Request.sequence;
        }
        if (ValidUnit && (ValidUnit->GetOwnerPlayer() != PlayerId || ValidUnit->GetFactionId() != Participant->FactionId))
        {
            LastCommandFeedback = TEXT("The unit owner or faction does not match the admitted participant.");
            PublishGameplayEvent(politika::rules::CommandOutcomeEvent{
                SimulationStepsExecuted, Pending.Request.sequence, Pending.Request.correlation_id,
                Pending.Request.issuer_entity_id, politika::rules::KindOf(Pending.Request.intent), false,
                politika::rules::OrderRejection::NotOwned});
            return Pending.Request.sequence;
        }
    }
    if (ValidUnit && !Units.Contains(ValidUnit))
    {
        LastCommandFeedback = TEXT("That unit is no longer part of the active match.");
        PublishGameplayEvent(politika::rules::CommandOutcomeEvent{
            SimulationStepsExecuted, Pending.Request.sequence, Pending.Request.correlation_id,
            Pending.Request.issuer_entity_id, politika::rules::KindOf(Pending.Request.intent), false,
            politika::rules::OrderRejection::MissingIssuer});
        return Pending.Request.sequence;
    }
    Pending.Unit = ValidUnit;
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
    else if (std::holds_alternative<politika::rules::GrenadeOrder>(Intent)) Pending.Kind = EPolitikaPrototypeOrder::Grenade;
    else if (std::holds_alternative<politika::rules::ReloadOrder>(Intent)) Pending.Kind = EPolitikaPrototypeOrder::Reload;
    else if (std::holds_alternative<politika::rules::SprintOrder>(Intent)) Pending.Kind = EPolitikaPrototypeOrder::Sprint;
    else Pending.Kind = EPolitikaPrototypeOrder::Stop;
    const uint64 Sequence = Pending.Request.sequence;
    PendingOrders.Add(MoveTemp(Pending));
    return Sequence;
}

politika::rules::OrderResult APolitikaPrototypeMatch::ValidateOrder(const FPendingOrder& Pending) const
{
    const APolitikaPrototypeUnit* Issuer = Pending.Unit.Get();
    if (Issuer && !Units.Contains(const_cast<APolitikaPrototypeUnit*>(Issuer)))
        return {politika::rules::OrderRejection::MissingIssuer};
    if (Issuer && !RuntimeParticipants.IsEmpty())
    {
        const FPolitikaPrototypeRuntimeParticipant* Participant = RuntimeParticipants.FindByPredicate(
            [&Pending](const FPolitikaPrototypeRuntimeParticipant& Candidate)
            { return Candidate.RuntimePlayerId == Pending.Request.participant_id; });
        if (!Participant)
            return {politika::rules::OrderRejection::Unauthorized};
        if (Issuer->GetOwnerPlayer() != Pending.Request.participant_id
            || Issuer->GetFactionId() != Participant->FactionId)
            return {politika::rules::OrderRejection::NotOwned};
    }
    politika::rules::Entity IssuerRules;
    if (Issuer) { IssuerRules.id = Issuer->GetStableId(); IssuerRules.owner_player = Issuer->GetOwnerPlayer();
        IssuerRules.movable = Issuer->CanMove(); IssuerRules.can_attack = Issuer->CanAttack();
        IssuerRules.alive = Issuer->IsAlive(); IssuerRules.operational = Issuer->IsAlive();
        IssuerRules.team = Issuer->GetTeamId(); }

    politika::rules::Entity TargetRules;
    const APolitikaPrototypeUnit* Target = Pending.Target.Get();
    if (Target)
    {
        TargetRules.id = Target->GetStableId();
        TargetRules.owner_player = Target->GetOwnerPlayer();
        TargetRules.attackable = Target->IsAttackable();
        TargetRules.alive = Target->IsAlive();
        TargetRules.team = Target->GetTeamId();
        TargetRules.observed = IsUnitObservedByParticipant(Target, Pending.Request.participant_id);
    }
    // Under fog an unresolvable target ID is indistinguishable from a hidden one, so a
    // client cannot probe whether an unseen entity exists.
    uint64 RequestedTargetId = 0;
    if (const auto* Attack = std::get_if<politika::rules::AttackOrder>(&Pending.Request.intent)) RequestedTargetId = Attack->target_id;
    else if (const auto* Grenade = std::get_if<politika::rules::GrenadeOrder>(&Pending.Request.intent)) RequestedTargetId = Grenade->target_id;
    const bool bHiddenUnknownTarget = bFogOfWar && !Target && RequestedTargetId != 0;
    if (bHiddenUnknownTarget) { TargetRules.id = RequestedTargetId; TargetRules.observed = false; }
    const auto Admission = politika::rules::AdmitCommand(Pending.Request, MatchGeneration,
        bContentReady && Lifecycle.IsActive(),
        Issuer ? &IssuerRules : nullptr,
        {PlayableMax.X - PlayableMin.X, PlayableMax.Y - PlayableMin.Y},
        Target || bHiddenUnknownTarget ? &TargetRules : nullptr);
    if (!Admission) return Admission;
    if ((Pending.Kind == EPolitikaPrototypeOrder::Grenade && !Issuer->CanUseGrenade(Target))
        || (Pending.Kind == EPolitikaPrototypeOrder::Reload && !Issuer->CanReload())
        || (Pending.Kind == EPolitikaPrototypeOrder::Sprint && !Issuer->CanSprint()))
        return {politika::rules::OrderRejection::AbilityUnavailable};
    return Admission;
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
        case politika::rules::OrderRejection::AbilityUnavailable: return TEXT("Ability unavailable: check range, ammunition or cooldown.");
        case politika::rules::OrderRejection::MalformedCommand: return TEXT("The command was malformed and was not applied.");
        case politika::rules::OrderRejection::DuplicateCommand: return TEXT("That command was already processed.");
        case politika::rules::OrderRejection::RateLimited: return TEXT("Too many pending commands; try again.");
        case politika::rules::OrderRejection::TargetNotVisible: return TEXT("That target is not visible to your team.");
        case politika::rules::OrderRejection::None: return TEXT("Order rejected.");
    }
    return TEXT("Order rejected.");
}

void APolitikaPrototypeMatch::PublishGameplayEvent(politika::rules::GameplayEvent&& Event)
{
#if !UE_BUILD_SHIPPING
    ObserveNavigationAcceptanceEvent(Event);
    ObserveNavigationFailureEvent(Event);
    if (ParticipantAuthorityAcceptance.bRunning)
    {
        using namespace politika::rules;
        if (const auto* Outcome = std::get_if<CommandOutcomeEvent>(&Event))
        {
            auto Record = [Outcome](uint64 ExpectedSequence, int32& Count, bool* bAccepted,
                OrderRejection* Rejection)
            {
                if (ExpectedSequence == 0 || Outcome->sequence != ExpectedSequence) return;
                ++Count;
                if (bAccepted) *bAccepted = Outcome->accepted;
                if (Rejection) *Rejection = Outcome->rejection;
            };
            Record(ParticipantAuthorityAcceptance.LocalSequence, ParticipantAuthorityAcceptance.LocalOutcomes,
                &ParticipantAuthorityAcceptance.bLocalAccepted, nullptr);
            Record(ParticipantAuthorityAcceptance.ForeignSeedSequence, ParticipantAuthorityAcceptance.ForeignSeedOutcomes,
                &ParticipantAuthorityAcceptance.bForeignSeedAccepted, nullptr);
            Record(ParticipantAuthorityAcceptance.ForeignSequence, ParticipantAuthorityAcceptance.ForeignOutcomes,
                nullptr, &ParticipantAuthorityAcceptance.ForeignRejection);
            Record(ParticipantAuthorityAcceptance.SystemSequence, ParticipantAuthorityAcceptance.SystemOutcomes,
                nullptr, &ParticipantAuthorityAcceptance.SystemRejection);
            Record(ParticipantAuthorityAcceptance.UnassignedSequence, ParticipantAuthorityAcceptance.UnassignedOutcomes,
                nullptr, &ParticipantAuthorityAcceptance.UnassignedRejection);
            Record(ParticipantAuthorityAcceptance.DeadSequence, ParticipantAuthorityAcceptance.DeadOutcomes,
                nullptr, &ParticipantAuthorityAcceptance.DeadRejection);
            Record(ParticipantAuthorityAcceptance.DestroyedSequence, ParticipantAuthorityAcceptance.DestroyedOutcomes,
                nullptr, &ParticipantAuthorityAcceptance.DestroyedRejection);
        }
        else if (const auto* Changed = std::get_if<OrderChangedEvent>(&Event);
            Changed && ParticipantAuthorityAcceptance.bForeignBaselineCaptured
            && Changed->entity_id == ParticipantAuthorityAcceptance.ForeignEntityId)
            ++ParticipantAuthorityAcceptance.ForeignOrderChanges;
    }
#endif
#if !UE_BUILD_SHIPPING
    ObserveUnitScaleEvent(Event);
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
    if (const auto* Outcome = std::get_if<politika::rules::CommandOutcomeEvent>(&PublishedEvents.Last()))
    {
        const politika::rules::CommandOutcomeEvent Routed = *Outcome;
        RouteCommandOutcome(Routed);
    }
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

bool APolitikaPrototypeMatch::IsCompleteNavigationResult(const FPathFindingResult& Result, FString& OutFailure)
{
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
    if (Result.Path->GetPathPoints().IsEmpty())
    {
        OutFailure = TEXT("EmptyPath");
        return false;
    }
    OutFailure.Reset();
    return true;
}

bool APolitikaPrototypeMatch::ResolveNavigationRoute(APolitikaPrototypeUnit* Unit, const FVector& Destination,
    TArray<FVector>& OutWaypoints, FVector& OutResolvedDestination, FString& OutFailure,
    uint64 OrderSequence)
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
    const bool bProjectedStart = NavSystem->ProjectPointToNavigation(Unit->GetActorLocation(), ProjectedStart, QueryExtent, NavData);
#if !UE_BUILD_SHIPPING
    if (NavigationFailureAcceptance.bRunning)
    {
        ++NavigationFailureAcceptance.ProjectionChecks;
        NavigationFailureAcceptance.bOriginProjected = bProjectedStart;
        if (bProjectedStart) NavigationFailureAcceptance.ProjectedOrigin = ProjectedStart.Location;
    }
#endif
    if (!bProjectedStart)
    {
        OutFailure = TEXT("OriginNotNavigable");
        return false;
    }
    const bool bProjectedGoal = NavSystem->ProjectPointToNavigation(Destination, ProjectedGoal, QueryExtent, NavData);
#if !UE_BUILD_SHIPPING
    if (NavigationFailureAcceptance.bRunning)
    {
        ++NavigationFailureAcceptance.ProjectionChecks;
        NavigationFailureAcceptance.bGoalProjected = bProjectedGoal;
        if (bProjectedGoal)
        {
            NavigationFailureAcceptance.ProjectedGoal = ProjectedGoal.Location;
            NavigationFailureAcceptance.ProjectionDisplacement = FVector::Dist(ProjectedGoal.Location, Destination);
        }
    }
#endif
    if (!bProjectedGoal || FVector::Dist(ProjectedGoal.Location, Destination) > 150.0f)
    {
        OutFailure = TEXT("DestinationNotNavigable");
#if !UE_BUILD_SHIPPING
        if (NavigationFailureAcceptance.bRunning && NavigationFailureAcceptance.bDestinationProjectionCase)
        {
            NavigationFailureAcceptance.LastFailure = OutFailure;
            UE_LOG(LogPolitikaStartup, Display,
                TEXT("NAV_FAILURE_PROJECTION generation=%llu unit=%llu sequence=%llu nav=%s origin_projected=%d goal_projected=%d extent=80.000000,80.000000,180.000000 requested=%.6f,%.6f,%.6f projected_origin=%.6f,%.6f,%.6f projected_goal=%s displacement=%s threshold=150.000000 query_executed=0 failure=%s"),
                MatchGeneration, Unit->GetStableId(), OrderSequence, *NavData->GetPathName(),
                NavigationFailureAcceptance.bOriginProjected ? 1 : 0, bProjectedGoal ? 1 : 0,
                Destination.X, Destination.Y, Destination.Z,
                NavigationFailureAcceptance.ProjectedOrigin.X, NavigationFailureAcceptance.ProjectedOrigin.Y,
                NavigationFailureAcceptance.ProjectedOrigin.Z,
                bProjectedGoal ? *FString::Printf(TEXT("%.6f,%.6f,%.6f"), ProjectedGoal.Location.X,
                    ProjectedGoal.Location.Y, ProjectedGoal.Location.Z) : TEXT("NA"),
                bProjectedGoal ? *FString::Printf(TEXT("%.6f"), FVector::Dist(ProjectedGoal.Location, Destination)) : TEXT("NA"),
                *OutFailure);
        }
#endif
        return false;
    }
    FPathFindingQuery Query(Unit, *NavData, ProjectedStart.Location, ProjectedGoal.Location);
    Query.SetAllowPartialPaths(false);
    Query.SetRequireNavigableEndLocation(true);
    const double QueryStartSeconds = FPlatformTime::Seconds();
    const FPathFindingResult Result = NavSystem->FindPathSync(Query);
#if !UE_BUILD_SHIPPING
    if (NavigationFailureAcceptance.bRunning) NavigationFailureAcceptance.LastFailure = OutFailure;
    UE_LOG(LogPolitikaStartup, Display,
        TEXT("RTS_NAV_QUERY generation=%llu unit=%llu sequence=%llu result=%u partial=%d points=%d duration_us=%.1f"),
        MatchGeneration, Unit->GetStableId(), OrderSequence, static_cast<uint32>(Result.Result),
        Result.IsPartial() ? 1 : 0, Result.Path.IsValid() ? Result.Path->GetPathPoints().Num() : 0,
        (FPlatformTime::Seconds() - QueryStartSeconds) * 1000000.0);
    if (NavigationFailureAcceptance.bRunning)
    {
        auto& Trace = NavigationFailureAcceptance;
        ++Trace.Queries;
        Trace.LastQueryResult = static_cast<uint32>(Result.Result);
        Trace.bLastQueryPartial = Result.IsPartial();
        Trace.bLastPathValid = Result.Path.IsValid();
        Trace.LastPathPoints = Result.Path.IsValid() ? Result.Path->GetPathPoints().Num() : 0;
        const FVector LastPoint = Result.Path.IsValid() && !Result.Path->GetPathPoints().IsEmpty()
            ? Result.Path->GetPathPoints().Last().Location : FVector::ZeroVector;
        const TCHAR* Classification = Result.Result != ENavigationQueryResult::Success || !Result.Path.IsValid()
            ? TEXT("NoCompletePath") : Result.Path->IsPartial() ? TEXT("PartialPath")
            : Result.Path->GetPathPoints().IsEmpty() ? TEXT("EmptyPath") : TEXT("CompletePath");
        UE_LOG(LogPolitikaStartup, Display,
            TEXT("NAV_FAILURE_QUERY generation=%llu unit=%llu sequence=%llu profile=%s nav=%s start=%.6f,%.6f,%.6f requested=%.6f,%.6f,%.6f projected_start=%.6f,%.6f,%.6f projected_goal=%.6f,%.6f,%.6f allow_partial=0 require_navigable_end=1 result=%u partial=%d path_valid=%d points=%d endpoint=%.6f,%.6f,%.6f failure=%s"),
            MatchGeneration, Unit->GetStableId(), OrderSequence,
            MapDefinition ? *MapDefinition->MovementProfileId.ToString() : TEXT("missing"), *NavData->GetPathName(),
            Unit->GetActorLocation().X, Unit->GetActorLocation().Y, Unit->GetActorLocation().Z,
            Destination.X, Destination.Y, Destination.Z,
            ProjectedStart.Location.X, ProjectedStart.Location.Y, ProjectedStart.Location.Z,
            ProjectedGoal.Location.X, ProjectedGoal.Location.Y, ProjectedGoal.Location.Z,
            static_cast<uint32>(Result.Result), Result.IsPartial() ? 1 : 0,
            Result.Path.IsValid() ? 1 : 0, Trace.LastPathPoints,
            LastPoint.X, LastPoint.Y, LastPoint.Z, Classification);
    }
#endif
    if (!IsCompleteNavigationResult(Result, OutFailure)) return false;
    const TArray<FNavPathPoint>& Points = Result.Path->GetPathPoints();
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
    Units.RemoveAll([](const TObjectPtr<APolitikaPrototypeUnit>& Unit) { return !IsValid(Unit); });
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
            UE_LOG(LogPolitikaStartup, Display, TEXT("RTS_ORDER_REJECTED unit=%llu sequence=%llu outcome=%u reason=%s participant=%u correlation=%llu"),
                RejectedUnit ? RejectedUnit->GetStableId() : 0, Pending.Request.sequence,
                static_cast<uint32>(Result.rejection), *Reason.Replace(TEXT(" "), TEXT("_")),
                Pending.Request.participant_id, Pending.Request.correlation_id);
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
            case EPolitikaPrototypeOrder::Grenade:
                Unit->BeginGrenade(Pending.Target.Get()); LastCommandFeedback = TEXT("Grenade launcher preparing."); break;
            case EPolitikaPrototypeOrder::Reload:
                Unit->BeginReload(); LastCommandFeedback = TEXT("Reloading magazine."); break;
            case EPolitikaPrototypeOrder::Sprint:
                Unit->SprintRemaining = Unit->InfantryBehavior->SprintSeconds;
                Unit->SprintCooldown = Unit->InfantryBehavior->SprintCooldownSeconds;
                UE_LOG(LogPolitikaStartup, Display, TEXT("INFANTRY_SPRINT_BEGIN unit=%llu seconds=%.2f multiplier=%.2f"),
                    Unit->GetStableId(), Unit->SprintRemaining, Unit->InfantryBehavior->SprintMultiplier);
                LastCommandFeedback = TEXT("Sprint active; firing suspended."); break;
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
        const bool bOrderTransitioned = Pending.Kind != EPolitikaPrototypeOrder::Reload
            && Pending.Kind != EPolitikaPrototypeOrder::Sprint
            && (PreviousOrder != Unit->GetCurrentOrder()
            || (PreviousOrder != EPolitikaPrototypeOrder::Idle && Pending.Kind != EPolitikaPrototypeOrder::Stop));
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
        static const TCHAR* OrderNames[] = { TEXT("Idle"), TEXT("Move"), TEXT("Attack"), TEXT("AttackMove"), TEXT("Stop"), TEXT("Guard"), TEXT("Grenade"), TEXT("Reload"), TEXT("Sprint") };
        const int32 OrderIndex = static_cast<int32>(Pending.Kind);
        UE_LOG(LogPolitikaStartup, Display, TEXT("RTS_ORDER_ADMITTED order=%s unit=%llu target=%llu sequence=%llu participant=%u correlation=%llu generation=%llu"),
            OrderNames[OrderIndex], AdmittedUnit ? AdmittedUnit->GetStableId() : 0,
            AdmittedTarget ? AdmittedTarget->GetStableId() : 0, Pending.Request.sequence,
            Pending.Request.participant_id, Pending.Request.correlation_id, Pending.Request.match_generation);
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
        if (!IsValid(Unit)) continue;
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
    // Team vision follows movement so acquisition, firing and target loss use current
    // positions; offline (no fog) this marks every unit observed and changes nothing.
    UpdateTeamVisibility();
    for (APolitikaPrototypeUnit* Unit : Units) if (IsValid(Unit)) Unit->ResolveCombat(DeltaSeconds);
    AdvanceGrenades(DeltaSeconds);
    for (APolitikaPrototypeUnit* Unit : Units)
    {
        if (!IsValid(Unit)) continue;
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
        if (!IsValid(Unit)) continue;
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
        if (IsValid(Unit) && !Unit->IsAlive())
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
            for (auto& Selection : SelectionByPlayer) Selection.Value.Units.Remove(Unit);
            Unit->BeginDeathPresentation();
        }
    }
    Units.RemoveAll([](const TObjectPtr<APolitikaPrototypeUnit>& Unit) { return !IsValid(Unit) || !Unit->IsAlive(); });
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
                // The capturing participant wins; offline this is always the local human.
                const uint32 Winner = Objective->GetCapturingPlayerId() != 0
                    ? Objective->GetCapturingPlayerId() : LocalRuntimePlayerId;
                const uint8 WinningTeam = Objective->GetCapturingTeamId() != politika::rules::kNoTeam
                    ? Objective->GetCapturingTeamId() : GetTeamForParticipant(Winner);
                CommittedResult.Emplace(FPolitikaPrototypeMatchResult{Winner, TEXT("objective_secured"), WinningTeam});
                LastCommandFeedback = TEXT("Objective secured. Match results are ready; press Escape to return to menu.");
                UE_LOG(LogPolitikaStartup, Display, TEXT("MATCH_RESULTS outcome=%s player=%u committed=1 generation=%llu step=%llu team=%u"),
                    *CommittedResult->Reason.ToString(), CommittedResult->WinningPlayerId, MatchGeneration,
                    SimulationStepsExecuted, WinningTeam);
                SyncReplicatedLifecycle();
                NoteNetworkResultCommitted();
            }
        }
    }
    // Publication is the final simulation phase; observers get a const snapshot
    // until the next authoritative step, with step IDs to avoid duplicate handling.
    PublishGameplayEvents();
#if !UE_BUILD_SHIPPING
    AfterNavigationAcceptanceStep(DeltaSeconds);
    AfterNavigationFailureStep();
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
    // The RTS camera is client-local; its transform is never server state.
    SetReplicatingMovement(false);
}

#if !UE_BUILD_SHIPPING
namespace
{
// "c0,c1,...,c9": member count of every control-group slot, for structured evidence.
FString ControlGroupCounts(const politika::selection::ControlGroups& Groups)
{
    FString Counts;
    for (std::size_t Slot = 0; Slot < politika::selection::kControlGroupSlots; ++Slot)
        Counts += FString::Printf(TEXT("%s%d"), Slot == 0 ? TEXT("") : TEXT(","), static_cast<int32>(Groups.Members(Slot).size()));
    return Counts;
}

void LogSelectionSet(const APlayerController& Controller, TConstArrayView<APolitikaPrototypeUnit*> Selected, uint32 PlayerId,
    const TCHAR* Source, const TCHAR* OpName, int32 Hits)
{
    FString Ids;
    for (const APolitikaPrototypeUnit* Unit : Selected)
        Ids += FString::Printf(TEXT("%s%llu"), Ids.IsEmpty() ? TEXT("") : TEXT(","), Unit->GetStableId());
    UE_LOG(LogPolitikaStartup, Display, TEXT("RTS_SELECTION_SET player=%u source=%s op=%s hits=%d count=%d ids=%s"),
        PlayerId, Source, OpName, Hits, Selected.Num(), *Ids);
    // Screen positions let the packaged harness click a specific selected unit.
    for (const APolitikaPrototypeUnit* Unit : Selected)
    {
        FVector2D Screen;
        if (Controller.ProjectWorldLocationToScreen(Unit->GetActorLocation(), Screen, true))
            UE_LOG(LogPolitikaStartup, Display, TEXT("RTS_SELECTION_SCREEN unit=%llu x=%.0f y=%.0f"),
                Unit->GetStableId(), Screen.X, Screen.Y);
    }
}
}
#endif

void APolitikaPrototypePlayerController::BeginPlay()
{
    Super::BeginPlay();
    ResolveMatch();
    bShowMouseCursor = true;
    SetInputMode(FInputModeGameAndUI());
#if !UE_BUILD_SHIPPING
    bHostileProbeEnabled = IsLocalController() && FParse::Param(FCommandLine::Get(), TEXT("PolitikaMPProbe"));
#endif
    if (IsLocalController() && GetNetMode() == NM_Client)
        UE_LOG(LogPolitikaStartup, Display, TEXT("MP_CLIENT_CONNECTED controller=%s"), *GetName());
}

void APolitikaPrototypePlayerController::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    if (!bMapCameraApplied) ResolveMatch();
    UpdateSelectDrag();
    // Keeps group counts current for the HUD: dead or lost members drop out, and a new
    // generation, participant or inactive match clears every slot.
    if (IsLocalController()) SyncControlGroups(Match.Get());
#if !UE_BUILD_SHIPPING
    if (bHostileProbeEnabled && IsLocalController()) TickHostileProbe();
#endif
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
    // An observer match learns its local participant from this connection's assignment.
    if (!Match->HasAuthority() && IsLocalController()) Match->SetLocalViewerParticipant(AssignedRuntimePlayerId);
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
    FVector Focus = Match->GetObjective()->GetActorLocation();
    if (GetNetMode() != NM_Standalone)
    {
        // Networked seats start looking at their own authored start, not the shared
        // objective. The camera pawn is client-local; moving it is never server state.
        const uint32 Participant = GetCommandPlayerId();
        if (Participant == 0) return;
        const FPolitikaPrototypeRuntimeParticipant* Seat = Match->GetRuntimeParticipants().FindByPredicate(
            [Participant](const FPolitikaPrototypeRuntimeParticipant& Candidate) { return Candidate.RuntimePlayerId == Participant; });
        const FPolitikaPrototypePlayerSlot* Slot = Seat ? Definition->PlayerSlots.FindByPredicate(
            [Seat](const FPolitikaPrototypePlayerSlot& Candidate) { return Candidate.SlotId == Seat->SlotId; }) : nullptr;
        if (!Slot) return;
        Focus = Slot->StartLocation;
        CameraPawn->SetActorLocation(FVector(Focus.X, Focus.Y, CameraPawn->GetActorLocation().Z));
    }
    CameraPawn->GetCamera()->SetRelativeLocation(Definition->CameraOffset);
    CameraPawn->GetCamera()->SetFieldOfView(Definition->CameraFieldOfView);
    CameraPawn->GetCamera()->SetWorldRotation((Focus - CameraPawn->GetCamera()->GetComponentLocation()).Rotation());
    bMapCameraApplied = true;
}

void APolitikaPrototypePlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();
    InputComponent->BindKey(EKeys::LeftMouseButton, IE_Pressed, this, &APolitikaPrototypePlayerController::BeginSelectPress);
    InputComponent->BindKey(EKeys::LeftMouseButton, IE_Released, this, &APolitikaPrototypePlayerController::EndSelectPress);
    InputComponent->BindKey(EKeys::RightMouseButton, IE_Pressed, this, &APolitikaPrototypePlayerController::OrderUnderCursor);
    // Order keys live in DefaultInput.ini and must not share a key with the W/A/S/D pan axes,
    // or every order press would also move the camera by a frame of pan speed.
    InputComponent->BindAction(TEXT("PrototypeStop"), IE_Pressed, this, &APolitikaPrototypePlayerController::StopSelected);
    InputComponent->BindAction(TEXT("PrototypeGuard"), IE_Pressed, this, &APolitikaPrototypePlayerController::GuardSelected);
    InputComponent->BindAction(TEXT("PrototypeGrenade"), IE_Pressed, this, &APolitikaPrototypePlayerController::GrenadeSelected);
    InputComponent->BindAction(TEXT("PrototypeReload"), IE_Pressed, this, &APolitikaPrototypePlayerController::ReloadSelected);
    InputComponent->BindAction(TEXT("PrototypeSprint"), IE_Pressed, this, &APolitikaPrototypePlayerController::SprintSelected);
    InputComponent->BindAction(TEXT("PrototypeAttackMove"), IE_Pressed, this, &APolitikaPrototypePlayerController::AttackMoveSelected);
    // Control groups (4C-G): one action per digit; ControlGroupInput reads the modifier.
    DECLARE_DELEGATE_OneParam(FPolitikaControlGroupKey, int32);
    for (int32 Slot = 0; Slot < static_cast<int32>(politika::selection::kControlGroupSlots); ++Slot)
        InputComponent->BindAction<FPolitikaControlGroupKey>(FName(*FString::Printf(TEXT("PrototypeControlGroup%d"), Slot)),
            IE_Pressed, this, &APolitikaPrototypePlayerController::ControlGroupKey, Slot);
    InputComponent->BindAction(TEXT("PrototypeSubgroupCycle"), IE_Pressed, this, &APolitikaPrototypePlayerController::CycleSubgroup);
    InputComponent->BindKey(EKeys::Escape, IE_Pressed, this, &APolitikaPrototypePlayerController::ReturnToMenu);
    InputComponent->BindAxis(TEXT("PrototypePanForward"), this, &APolitikaPrototypePlayerController::PanForward);
    InputComponent->BindAxis(TEXT("PrototypePanRight"), this, &APolitikaPrototypePlayerController::PanRight);
    InputComponent->BindAxis(TEXT("PrototypeZoom"), this, &APolitikaPrototypePlayerController::Zoom);
}

politika::selection::SelectionOp APolitikaPrototypePlayerController::ModifierSelectionOp(bool bBox) const
{
    using politika::selection::SelectionOp;
    const bool bShift = IsInputKeyDown(EKeys::LeftShift) || IsInputKeyDown(EKeys::RightShift);
    const bool bCtrl = IsInputKeyDown(EKeys::LeftControl) || IsInputKeyDown(EKeys::RightControl);
    // Shift adds (box) or toggles (click); Ctrl+drag removes. Ctrl+click is reserved for
    // same-archetype selection (4C-G) and is a plain click until then.
    if (bShift) return bBox ? SelectionOp::Union : SelectionOp::Toggle;
    if (bCtrl && bBox) return SelectionOp::Subtract;
    return SelectionOp::Replace;
}

void APolitikaPrototypePlayerController::BeginSelectPress()
{
    FVector2D Cursor;
    if (!ResolveMatch() || !GetMousePosition(Cursor.X, Cursor.Y)) return;
    // The group bar captures its clicks: no world selection stroke starts over it.
    if (const int32 BarSlot = GroupBarSlotUnderCursor(); BarSlot != INDEX_NONE)
    {
        ControlGroupInput(BarSlot, TEXT("bar"));
        return;
    }
    bSelectPressed = true;
    bBoxSelecting = false;
    SelectReleaseSeenSeconds = -1.0;
    SelectPressPosition = Cursor;
    SelectDragPosition = Cursor;
}

void APolitikaPrototypePlayerController::UpdateSelectDrag()
{
    if (!bSelectPressed) return;
    // The key state turns up as soon as the release arrives, but its IE_Released binding
    // runs later in the frame. Only a release that never reaches the binding (modal capture,
    // a dropped event) cancels the stroke, after a short grace; it never becomes a click.
    const double Now = GetWorld() ? GetWorld()->GetRealTimeSeconds() : 0.0;
    if (IsInputKeyDown(EKeys::LeftMouseButton)) SelectReleaseSeenSeconds = -1.0;
    else if (SelectReleaseSeenSeconds < 0.0) SelectReleaseSeenSeconds = Now;
    else if (Now - SelectReleaseSeenSeconds > 0.25)
    {
        CancelSelectPress(TEXT("ReleaseLost"));
        return;
    }
    if (!IsViewportFocused())
    {
        CancelSelectPress(TEXT("FocusLost"));
        return;
    }
    FVector2D Cursor;
    if (!GetMousePosition(Cursor.X, Cursor.Y)) return;
    SelectDragPosition = Cursor;
    if (!bBoxSelecting)
    {
        const float DpiScale = GEngine && GEngine->GameViewport ? GEngine->GameViewport->GetDPIScale() : 1.0f;
        bBoxSelecting = politika::selection::ExceedsDragThreshold({SelectPressPosition.X, SelectPressPosition.Y},
            {Cursor.X, Cursor.Y}, DpiScale);
    }
}

void APolitikaPrototypePlayerController::EndSelectPress()
{
    if (!bSelectPressed) return;
    // Focus loss flushes held keys as simulated releases; those cancel instead of selecting.
    if (!IsViewportFocused())
    {
        CancelSelectPress(TEXT("FocusLost"));
        return;
    }
    // The box ends at the last position sampled while the button was held: releasing mouse
    // capture moves the cursor back to where the press began, so it is not read here.
    const bool bBox = bBoxSelecting;
    bSelectPressed = false;
    bBoxSelecting = false;
    if (bBox) BoxSelect(SelectPressPosition, SelectDragPosition, ModifierSelectionOp(true));
    else SelectUnderCursor(ModifierSelectionOp(false));
}

void APolitikaPrototypePlayerController::CancelSelectPress(const TCHAR* Reason)
{
    if (!bSelectPressed) return;
#if !UE_BUILD_SHIPPING
    UE_LOG(LogPolitikaStartup, Display, TEXT("RTS_SELECTION_CANCELLED participant=%u box=%d reason=%s"),
        GetCommandPlayerId(), bBoxSelecting ? 1 : 0, Reason);
#endif
    bSelectPressed = false;
    bBoxSelecting = false;
}

bool APolitikaPrototypePlayerController::IsViewportFocused() const
{
    const ULocalPlayer* LocalPlayer = GetLocalPlayer();
    const FViewport* Viewport = LocalPlayer && LocalPlayer->ViewportClient ? LocalPlayer->ViewportClient->Viewport : nullptr;
    // Without a viewport (automation, a server-side controller) there is no focus to lose.
    return !Viewport || Viewport->HasFocus();
}

bool APolitikaPrototypePlayerController::GetSelectionBox(FVector2D& OutMin, FVector2D& OutMax) const
{
    if (!bSelectPressed || !bBoxSelecting) return false;
    OutMin = FVector2D(FMath::Min(SelectPressPosition.X, SelectDragPosition.X), FMath::Min(SelectPressPosition.Y, SelectDragPosition.Y));
    OutMax = FVector2D(FMath::Max(SelectPressPosition.X, SelectDragPosition.X), FMath::Max(SelectPressPosition.Y, SelectDragPosition.Y));
    return true;
}

void APolitikaPrototypePlayerController::SelectUnderCursor(politika::selection::SelectionOp Op)
{
    APolitikaPrototypeMatch* ActiveMatch = ResolveMatch();
    if (!ActiveMatch) return;
    FHitResult Hit;
    APolitikaPrototypeUnit* Unit = GetHitResultUnderCursorByChannel(UEngineTypes::ConvertToTraceType(ECC_Visibility), true, Hit)
        ? Cast<APolitikaPrototypeUnit>(Hit.GetActor()) : nullptr;
    const bool bOwnUnit = IsValid(Unit) && Unit->IsAlive() && Unit->GetOwnerPlayer() == GetCommandPlayerId();
    const bool bCtrl = IsInputKeyDown(EKeys::LeftControl) || IsInputKeyDown(EKeys::RightControl);
    if (Op == politika::selection::SelectionOp::Replace)
    {
        // Ctrl+click or a double-click on an own unit selects its archetype on screen (4C-G).
        // Ctrl+click elsewhere changes nothing.
        const double Now = GetWorld() ? GetWorld()->GetRealTimeSeconds() : 0.0;
        const bool bDoubleClick = bOwnUnit && LastClickUnit == Unit->GetStableId() && LastClickSeconds >= 0.0
            && Now - LastClickSeconds <= politika::selection::kDoubleClickSeconds;
        LastClickUnit = bOwnUnit && !bDoubleClick ? Unit->GetStableId() : 0;
        LastClickSeconds = Now;
        if (bCtrl || bDoubleClick)
        {
            if (bOwnUnit) SelectSameArchetype(Unit, bCtrl ? TEXT("ctrl_click") : TEXT("double_click"));
            return;
        }
        SelectUnitForInput(Unit);
        return;
    }
    // Shift+click on empty ground or a unit this participant cannot command changes nothing.
    if (!IsValid(Unit) || !Unit->IsAlive() || Unit->GetOwnerPlayer() != GetCommandPlayerId()) return;
    ApplySelection(MakeArrayView(&Unit, 1), Op, TEXT("click"));
}

void APolitikaPrototypePlayerController::BoxSelect(const FVector2D& CornerA, const FVector2D& CornerB,
    politika::selection::SelectionOp Op)
{
    using namespace politika::selection;
    APolitikaPrototypeMatch* ActiveMatch = ResolveMatch();
    if (!ActiveMatch) return;
    const uint32 PlayerId = GetCommandPlayerId();
    const ScreenRect Box = RectFromCorners({CornerA.X, CornerA.Y}, {CornerB.X, CornerB.Y});
    TArray<APolitikaPrototypeUnit*> Hits;
    for (APolitikaPrototypeUnit* Unit : ActiveMatch->GetUnits())
    {
        // Only this participant's own living, presented units are candidates; hostile,
        // allied, Unaligned and fog-hidden units never enter the commandable set.
        if (!IsValid(Unit) || !Unit->IsAlive() || PlayerId == 0 || Unit->GetOwnerPlayer() != PlayerId
            || Unit->IsLocalFogHidden() || !Unit->GetRootComponent())
            continue;
        // Selection proxy: the projected clickable body, so a partially boxed unit counts.
        const FBox Bounds = Unit->GetRootComponent()->Bounds.GetBox();
        bool bProjected = false;
        ScreenRect Proxy{{TNumericLimits<double>::Max(), TNumericLimits<double>::Max()},
            {TNumericLimits<double>::Lowest(), TNumericLimits<double>::Lowest()}};
        for (int32 Corner = 0; Corner < 8; ++Corner)
        {
            const FVector Point((Corner & 1) ? Bounds.Max.X : Bounds.Min.X, (Corner & 2) ? Bounds.Max.Y : Bounds.Min.Y,
                (Corner & 4) ? Bounds.Max.Z : Bounds.Min.Z);
            FVector2D Screen;
            if (!ProjectWorldLocationToScreen(Point, Screen, true)) continue;
            bProjected = true;
            Proxy.min = {FMath::Min(Proxy.min.x, Screen.X), FMath::Min(Proxy.min.y, Screen.Y)};
            Proxy.max = {FMath::Max(Proxy.max.x, Screen.X), FMath::Max(Proxy.max.y, Screen.Y)};
        }
        if (bProjected && Intersects(Box, Proxy)) Hits.Add(Unit);
    }
    ApplySelection(Hits, Op, TEXT("box"));
}

void APolitikaPrototypePlayerController::ApplySelection(TConstArrayView<APolitikaPrototypeUnit*> Hits,
    politika::selection::SelectionOp Op, const TCHAR* Source)
{
    APolitikaPrototypeMatch* ActiveMatch = ResolveMatch();
    if (!ActiveMatch) return;
    if (!ActiveMatch->IsLocalControllerBound(this))
    {
        ClearSelectionState();
        return;
    }
    const uint32 PlayerId = GetCommandPlayerId();
    const TArray<APolitikaPrototypeUnit*> Current = ActiveMatch->GetSelectedUnitsForPlayer(PlayerId);
    std::vector<politika::selection::EntityId> CurrentIds;
    std::vector<politika::selection::EntityId> HitIds;
    for (const APolitikaPrototypeUnit* Unit : Current) CurrentIds.push_back(Unit->GetStableId());
    for (const APolitikaPrototypeUnit* Unit : Hits)
        if (IsValid(Unit)) HitIds.push_back(Unit->GetStableId());
    const std::vector<politika::selection::EntityId> NextIds = politika::selection::Apply(CurrentIds, HitIds, Op);
    // Resolve the normalized IDs back to the units they came from; nothing is looked up
    // beyond the current selection and this stroke's own candidates.
    TArray<APolitikaPrototypeUnit*> Next;
    for (const politika::selection::EntityId Id : NextIds)
    {
        APolitikaPrototypeUnit* const* Found = Hits.FindByPredicate(
            [Id](const APolitikaPrototypeUnit* Unit) { return IsValid(Unit) && Unit->GetStableId() == Id; });
        if (!Found) Found = Current.FindByPredicate([Id](const APolitikaPrototypeUnit* Unit) { return Unit->GetStableId() == Id; });
        if (Found) Next.Add(*Found);
    }
    ActiveMatch->SetSelectionSet(PlayerId, Next);
    ActiveSubgroup = 0;
#if !UE_BUILD_SHIPPING
    static const TCHAR* OpNames[] = {TEXT("replace"), TEXT("union"), TEXT("toggle"), TEXT("subtract")};
    LogSelectionSet(*this, ActiveMatch->GetSelectedUnitsForPlayer(PlayerId), PlayerId, Source, OpNames[static_cast<uint8>(Op)],
        Hits.Num());
#endif
}

void APolitikaPrototypePlayerController::SelectSameArchetype(APolitikaPrototypeUnit* Unit, const TCHAR* Source)
{
    APolitikaPrototypeMatch* ActiveMatch = ResolveMatch();
    if (!ActiveMatch || !IsValid(Unit)) return;
    const uint32 PlayerId = GetCommandPlayerId();
    int32 ViewWidth = 0;
    int32 ViewHeight = 0;
    GetViewportSize(ViewWidth, ViewHeight);
    // Only units the player can see right now: inside the viewport and not fog-hidden. This is
    // never a scan of the hidden world (GRP-17).
    TArray<APolitikaPrototypeUnit*> Hits;
    for (APolitikaPrototypeUnit* Candidate : ActiveMatch->GetUnits())
    {
        if (!IsValid(Candidate) || !Candidate->IsAlive() || Candidate->GetOwnerPlayer() != PlayerId
            || Candidate->GetArchetypeId() != Unit->GetArchetypeId() || Candidate->IsLocalFogHidden())
            continue;
        FVector2D Screen;
        if (Candidate == Unit || (ProjectWorldLocationToScreen(Candidate->GetActorLocation(), Screen, true)
            && Screen.X >= 0 && Screen.Y >= 0 && Screen.X < ViewWidth && Screen.Y < ViewHeight))
            Hits.Add(Candidate);
    }
    ApplySelection(Hits, politika::selection::SelectionOp::Replace, Source);
}

void APolitikaPrototypePlayerController::SelectUnitForInput(APolitikaPrototypeUnit* Unit)
{
    APolitikaPrototypeMatch* ActiveMatch = ResolveMatch();
    if (!ActiveMatch) return;
    if (!ActiveMatch->IsLocalControllerBound(this))
    {
        ClearSelectionState();
        return;
    }
    const uint32 PlayerId = GetCommandPlayerId();
    if (!IsValid(Unit) || !Unit->IsAlive() || Unit->GetOwnerPlayer() != PlayerId)
    {
        ActiveMatch->SetSelection(PlayerId, nullptr);
#if !UE_BUILD_SHIPPING
        if (Unit)
            UE_LOG(LogPolitikaStartup, Display, TEXT("RTS_SELECTION_REJECTED player=%u unit=%llu reason=NotOwned"),
                PlayerId, Unit->GetStableId());
        UE_LOG(LogPolitikaStartup, Display, TEXT("RTS_SELECTION_SET player=%u source=click op=replace hits=0 count=0 ids="),
            PlayerId);
#endif
        return;
    }
    ActiveMatch->SetSelection(PlayerId, Unit);
    if (ActiveMatch->GetSelectedUnitForPlayer(PlayerId) != Unit)
    {
#if !UE_BUILD_SHIPPING
        UE_LOG(LogPolitikaStartup, Display, TEXT("RTS_SELECTION_REJECTED player=%u unit=%llu reason=Unavailable"),
            PlayerId, Unit->GetStableId());
#endif
        return;
    }
#if !UE_BUILD_SHIPPING
    UE_LOG(LogPolitikaStartup, Display, TEXT("RTS_SELECTION_SET player=%u source=click op=replace hits=1 count=1 ids=%llu"),
        PlayerId, Unit->GetStableId());
#endif
}

int32 APolitikaPrototypePlayerController::GroupBarSlotUnderCursor() const
{
    FVector2D Cursor;
    int32 ViewWidth = 0;
    int32 ViewHeight = 0;
    if (!GetMousePosition(Cursor.X, Cursor.Y)) return INDEX_NONE;
    GetViewportSize(ViewWidth, ViewHeight);
    if (ViewWidth <= 0 || ViewHeight <= 0) return INDEX_NONE;
    const std::size_t Slot = politika::selection::GroupBarSlotAt({Cursor.X, Cursor.Y}, ViewWidth, ViewHeight,
        APolitikaPrototypeHUD::UiScaleFor(ViewWidth));
    return Slot < politika::selection::kControlGroupSlots ? static_cast<int32>(Slot) : INDEX_NONE;
}

bool APolitikaPrototypePlayerController::SyncControlGroups(APolitikaPrototypeMatch* ActiveMatch)
{
    const uint32 PlayerId = GetCommandPlayerId();
    const bool bActive = ActiveMatch && PlayerId != 0 && ActiveMatch->IsLocalControllerBound(this)
        && ActiveMatch->GetMatchPhase() == politika::rules::MatchPhase::Active;
    if (!bActive)
    {
#if !UE_BUILD_SHIPPING
        if (!ControlGroups.Empty())
            UE_LOG(LogPolitikaStartup, Display, TEXT("RTS_CONTROL_GROUPS_CLEARED participant=%u reason=inactive"),
                ControlGroups.Participant());
#endif
        ControlGroups.Clear();
        RecallTaps.Reset();
        return false;
    }
    const uint64 Generation = ActiveMatch->GetMatchGeneration();
    if (ControlGroups.Bind(Generation, PlayerId))
    {
        RecallTaps.Reset();
#if !UE_BUILD_SHIPPING
        UE_LOG(LogPolitikaStartup, Display, TEXT("RTS_CONTROL_GROUPS_CLEARED participant=%u generation=%llu reason=rebound"),
            PlayerId, Generation);
#endif
    }
    // Permanently invalid members (dead, destroyed, no longer this participant's) leave every
    // slot at once. The prototype has no temporarily inoperable state, so nothing is held back.
    const std::size_t Dropped = ControlGroups.Prune([ActiveMatch, PlayerId](politika::selection::EntityId Id)
        { return ActiveMatch->IsCommandableUnitForPlayer(ActiveMatch->FindUnitByStableId(Id), PlayerId); });
#if !UE_BUILD_SHIPPING
    if (Dropped > 0)
        UE_LOG(LogPolitikaStartup, Display, TEXT("RTS_CONTROL_GROUPS_PRUNED participant=%u generation=%llu dropped=%llu counts=%s"),
            PlayerId, Generation, static_cast<uint64>(Dropped), *ControlGroupCounts(ControlGroups));
#else
    (void)Dropped;
#endif
    return true;
}

void APolitikaPrototypePlayerController::ControlGroupKey(int32 Slot)
{
    // A held selection stroke finishes first; group keys never interrupt it.
    if (bSelectPressed) return;
    ControlGroupInput(Slot, TEXT("key"));
}

void APolitikaPrototypePlayerController::ControlGroupInput(int32 Slot, const TCHAR* Source)
{
    using namespace politika::selection;
    if (Slot < 0 || Slot >= static_cast<int32>(kControlGroupSlots)) return;
    APolitikaPrototypeMatch* ActiveMatch = ResolveMatch();
    if (!SyncControlGroups(ActiveMatch))
    {
        ShowLocalFeedback(TEXT("Control groups are available during an active match."));
        return;
    }
    ApplyControlGroupInput(ActiveMatch, Slot, IsInputKeyDown(EKeys::LeftControl) || IsInputKeyDown(EKeys::RightControl),
        IsInputKeyDown(EKeys::LeftShift) || IsInputKeyDown(EKeys::RightShift),
        IsInputKeyDown(EKeys::LeftAlt) || IsInputKeyDown(EKeys::RightAlt), Source);
}

void APolitikaPrototypePlayerController::ApplyControlGroupInput(APolitikaPrototypeMatch* ActiveMatch, int32 Slot, bool bCtrl,
    bool bShift, bool bAlt, const TCHAR* Source)
{
    using namespace politika::selection;
    const int32 Modifiers = (bCtrl ? 1 : 0) + (bShift ? 1 : 0) + (bAlt ? 1 : 0);
    if (Modifiers > 1)
    {
        RecallTaps.Reset();
        ShowLocalFeedback(FString::Printf(TEXT("Group %d unchanged: use one modifier (Ctrl sets, Shift adds, Alt moves)."), Slot));
        return;
    }
    if (Modifiers == 0)
    {
        const double Now = GetWorld() ? GetWorld()->GetRealTimeSeconds() : 0.0;
        RecallControlGroup(ActiveMatch, Slot, RecallTaps.Tap(static_cast<std::size_t>(Slot), Now), Source);
        return;
    }
    RecallTaps.Reset();
    const GroupOp Op = bCtrl ? GroupOp::Replace : bShift ? GroupOp::Union : GroupOp::Transfer;
    std::vector<EntityId> Ids;
    for (const APolitikaPrototypeUnit* Unit : GetCommandableSelectedUnits(ActiveMatch)) Ids.push_back(Unit->GetStableId());
    std::array<std::size_t, kControlGroupSlots> Before{};
    for (std::size_t Index = 0; Index < kControlGroupSlots; ++Index) Before[Index] = ControlGroups.Members(Index).size();
    const bool bApplied = ControlGroups.Assign(static_cast<std::size_t>(Slot), Op, Ids);
    const int32 Members = static_cast<int32>(ControlGroups.Members(static_cast<std::size_t>(Slot)).size());
    FString Feedback;
    if (!bApplied)
        Feedback = FString::Printf(TEXT("Select units first: group %d unchanged."), Slot);
    else if (Op == GroupOp::Replace)
        Feedback = FString::Printf(TEXT("Group %d set: %d units"), Slot, Members);
    else
    {
        Feedback = FString::Printf(TEXT("%s %d units %s group %d (now %d)"), Op == GroupOp::Union ? TEXT("Added") : TEXT("Moved"),
            static_cast<int32>(Ids.size()), Op == GroupOp::Union ? TEXT("to") : TEXT("into"), Slot, Members);
        for (std::size_t Other = 0; Other < kControlGroupSlots; ++Other)
            if (static_cast<int32>(Other) != Slot && ControlGroups.Members(Other).size() != Before[Other])
                Feedback += FString::Printf(TEXT("  ·  group %d: %d → %d"), static_cast<int32>(Other), static_cast<int32>(Before[Other]),
                    static_cast<int32>(ControlGroups.Members(Other).size()));
    }
    ShowLocalFeedback(Feedback);
#if !UE_BUILD_SHIPPING
    static const TCHAR* OpNames[] = {TEXT("replace"), TEXT("union"), TEXT("transfer")};
    FString MemberIds;
    for (const EntityId Id : ControlGroups.Members(static_cast<std::size_t>(Slot)))
        MemberIds += FString::Printf(TEXT("%s%llu"), MemberIds.IsEmpty() ? TEXT("") : TEXT(","), Id);
    UE_LOG(LogPolitikaStartup, Display,
        TEXT("RTS_CONTROL_GROUP_ASSIGN participant=%u generation=%llu slot=%d op=%s source=%s selection=%d applied=%d members=%d ids=%s counts=%s"),
        GetCommandPlayerId(), ControlGroups.Generation(), Slot, OpNames[static_cast<uint8>(Op)], Source, static_cast<int32>(Ids.size()),
        bApplied ? 1 : 0, Members, *MemberIds, *ControlGroupCounts(ControlGroups));
#endif
}

void APolitikaPrototypePlayerController::RecallControlGroup(APolitikaPrototypeMatch* ActiveMatch, int32 Slot, bool bFocus,
    const TCHAR* Source)
{
    const uint32 PlayerId = GetCommandPlayerId();
    const std::vector<politika::selection::EntityId>& Ids = ControlGroups.Members(static_cast<std::size_t>(Slot));
    if (Ids.empty())
    {
        // An empty slot keeps the current selection.
        ShowLocalFeedback(FString::Printf(TEXT("Group %d is empty: Ctrl+%d assigns the selected units."), Slot, Slot));
#if !UE_BUILD_SHIPPING
        UE_LOG(LogPolitikaStartup, Display,
            TEXT("RTS_CONTROL_GROUP_RECALL participant=%u generation=%llu slot=%d source=%s members=0 selected=%d focus=0"),
            PlayerId, ControlGroups.Generation(), Slot, Source, ActiveMatch->GetSelectedUnitsForPlayer(PlayerId).Num());
#endif
        return;
    }
    // Recall only replaces the local selection through the normal revalidating setter.
    TArray<APolitikaPrototypeUnit*> Units;
    for (const politika::selection::EntityId Id : Ids)
        if (APolitikaPrototypeUnit* Unit = ActiveMatch->FindUnitByStableId(Id)) Units.Add(Unit);
    ActiveMatch->SetSelectionSet(PlayerId, Units);
    ActiveSubgroup = 0;
    const TArray<APolitikaPrototypeUnit*> Selected = ActiveMatch->GetSelectedUnitsForPlayer(PlayerId);
    if (bFocus) FocusCameraOn(ActiveMatch, Selected, Slot);
    ShowLocalFeedback(FString::Printf(TEXT("Group %d selected: %d units%s"), Slot, Selected.Num(),
        bFocus ? TEXT("  ·  camera centred") : TEXT("")));
#if !UE_BUILD_SHIPPING
    UE_LOG(LogPolitikaStartup, Display,
        TEXT("RTS_CONTROL_GROUP_RECALL participant=%u generation=%llu slot=%d source=%s members=%d selected=%d focus=%d"),
        PlayerId, ControlGroups.Generation(), Slot, Source, static_cast<int32>(Ids.size()), Selected.Num(), bFocus ? 1 : 0);
    LogSelectionSet(*this, Selected, PlayerId, TEXT("group"), TEXT("replace"), Units.Num());
#endif
}

void APolitikaPrototypePlayerController::FocusCameraOn(APolitikaPrototypeMatch* ActiveMatch,
    TConstArrayView<APolitikaPrototypeUnit*> Units, int32 Slot)
{
    APolitikaPrototypeCameraPawn* CameraPawn = Cast<APolitikaPrototypeCameraPawn>(GetPawn());
    if (Units.IsEmpty() || !CameraPawn || !CameraPawn->GetCamera()) return;
    FVector Centre = FVector::ZeroVector;
    for (const APolitikaPrototypeUnit* Unit : Units) Centre += Unit->GetActorLocation();
    Centre /= static_cast<double>(Units.Num());
    // The ground point at screen centre, on the plane through the group's centre.
    const auto ScreenCentreGround = [CameraPawn, &Centre](FVector& Out)
    {
        const FVector Origin = CameraPawn->GetCamera()->GetComponentLocation();
        const FVector Forward = CameraPawn->GetCamera()->GetForwardVector();
        if (Forward.Z >= -UE_SMALL_NUMBER) return false;
        Out = Origin + Forward * ((Centre.Z - Origin.Z) / Forward.Z);
        return true;
    };
    FVector Before;
    if (!ScreenCentreGround(Before)) return;
    // Camera only: the pawn is client-local and moving it is never match state.
    FVector Location = CameraPawn->GetActorLocation() + FVector(Centre.X - Before.X, Centre.Y - Before.Y, 0.0);
    const FVector2D Min = ActiveMatch->GetCameraPanMin();
    const FVector2D Max = ActiveMatch->GetCameraPanMax();
    Location.X = FMath::Clamp(Location.X, Min.X, Max.X);
    Location.Y = FMath::Clamp(Location.Y, Min.Y, Max.Y);
    CameraPawn->SetActorLocation(Location);
#if !UE_BUILD_SHIPPING
    FVector After = Before;
    ScreenCentreGround(After);
    UE_LOG(LogPolitikaStartup, Display,
        TEXT("RTS_CONTROL_GROUP_FOCUS participant=%u slot=%d units=%d centre_x=%.1f centre_y=%.1f before_x=%.1f before_y=%.1f after_x=%.1f after_y=%.1f error_cm=%.1f"),
        GetCommandPlayerId(), Slot, Units.Num(), Centre.X, Centre.Y, Before.X, Before.Y, After.X, After.Y,
        FVector2D::Distance(FVector2D(After), FVector2D(Centre)));
#endif
}

TArray<APolitikaPrototypeUnit*> APolitikaPrototypePlayerController::GetActiveSubgroupUnits(APolitikaPrototypeMatch* ActiveMatch)
{
    const TArray<APolitikaPrototypeUnit*> Selected = GetCommandableSelectedUnits(ActiveMatch);
    std::vector<std::pair<politika::selection::EntityId, FName>> Members;
    for (const APolitikaPrototypeUnit* Unit : Selected) Members.emplace_back(Unit->GetStableId(), Unit->GetArchetypeId());
    const std::vector<FName> Order = politika::selection::SubgroupOrder(std::move(Members));
    if (Order.size() <= 1) return Selected;
    const FName Active = Order[static_cast<std::size_t>(ActiveSubgroup) % Order.size()];
    return Selected.FilterByPredicate([Active](const APolitikaPrototypeUnit* Unit) { return Unit->GetArchetypeId() == Active; });
}

void APolitikaPrototypePlayerController::CycleSubgroup()
{
    APolitikaPrototypeMatch* ActiveMatch = ResolveMatch();
    const TArray<APolitikaPrototypeUnit*> Selected = GetCommandableSelectedUnits(ActiveMatch);
    if (Selected.IsEmpty()) return;
    std::vector<std::pair<politika::selection::EntityId, FName>> Members;
    for (const APolitikaPrototypeUnit* Unit : Selected) Members.emplace_back(Unit->GetStableId(), Unit->GetArchetypeId());
    const std::vector<FName> Order = politika::selection::SubgroupOrder(std::move(Members));
    const bool bBackward = IsInputKeyDown(EKeys::LeftShift) || IsInputKeyDown(EKeys::RightShift);
    ActiveSubgroup = static_cast<int32>(politika::selection::CycleSubgroup(static_cast<std::size_t>(ActiveSubgroup),
        Order.size(), !bBackward));
    // Tab refines which members receive subgroup abilities; it never changes saved groups.
    const TArray<APolitikaPrototypeUnit*> Active = GetActiveSubgroupUnits(ActiveMatch);
    ShowLocalFeedback(FString::Printf(TEXT("Subgroup %d of %d: %s (%d units)"), ActiveSubgroup + 1,
        static_cast<int32>(Order.size()), *Active[0]->GetUnitName().ToString(), Active.Num()));
#if !UE_BUILD_SHIPPING
    UE_LOG(LogPolitikaStartup, Display, TEXT("RTS_SUBGROUP_CYCLE participant=%u subgroups=%d active=%d archetype=%s units=%d backward=%d"),
        GetCommandPlayerId(), static_cast<int32>(Order.size()), ActiveSubgroup, *Active[0]->GetArchetypeId().ToString(),
        Active.Num(), bBackward ? 1 : 0);
#endif
}

void APolitikaPrototypePlayerController::ClearSelectionState()
{
    bSelectPressed = false;
    bBoxSelecting = false;
    CommandBatches.Reset();
    // Selection and every control group end together (Return, rematch, revoked seat).
#if !UE_BUILD_SHIPPING
    if (!ControlGroups.Empty())
        UE_LOG(LogPolitikaStartup, Display, TEXT("RTS_CONTROL_GROUPS_CLEARED participant=%u reason=selection_reset"),
            ControlGroups.Participant());
#endif
    ControlGroups.Clear();
    RecallTaps.Reset();
    ActiveSubgroup = 0;
    LastClickUnit = 0;
    if (APolitikaPrototypeMatch* ActiveMatch = Match.Get())
    {
        const uint32 LocalPlayerId = ActiveMatch->GetLocalRuntimePlayerId();
        if (LocalPlayerId != 0) ActiveMatch->SetSelection(LocalPlayerId, nullptr);
    }
}

TArray<APolitikaPrototypeUnit*> APolitikaPrototypePlayerController::GetCommandableSelectedUnits(
    APolitikaPrototypeMatch* ActiveMatch)
{
    if (!ActiveMatch) return {};
    if (!ActiveMatch->IsLocalControllerBound(this))
    {
        ClearSelectionState();
        return {};
    }
    return ActiveMatch->GetSelectedUnitsForPlayer(GetCommandPlayerId());
}

APolitikaPrototypeUnit* APolitikaPrototypePlayerController::GetCommandableSelectedUnit(
    APolitikaPrototypeMatch* ActiveMatch)
{
    const TArray<APolitikaPrototypeUnit*> Selected = GetCommandableSelectedUnits(ActiveMatch);
    return Selected.IsEmpty() ? nullptr : Selected[0];
}

void APolitikaPrototypePlayerController::SubmitGroupIntent(APolitikaPrototypeMatch* ActiveMatch,
    TConstArrayView<APolitikaPrototypeUnit*> Issuers, APolitikaPrototypeUnit* Target, const politika::rules::Order& Order)
{
    if (!ActiveMatch || Issuers.IsEmpty()) return;
    FCommandBatch Batch;
    Batch.Id = NextBatchId++;
    Batch.OrderKind = static_cast<uint8>(Order.index());
    Batch.FirstCorrelation = NextCorrelationId;
    Batch.LastCorrelation = NextCorrelationId + static_cast<uint32>(Issuers.Num()) - 1;
    Batch.Tally.expected = static_cast<uint32>(Issuers.Num());
    // Bounded history: outcomes for older batches only fall back to per-command feedback.
    if (CommandBatches.Num() >= 8) CommandBatches.RemoveAt(0);
    CommandBatches.Add(Batch);
#if !UE_BUILD_SHIPPING
    UE_LOG(LogPolitikaStartup, Display,
        TEXT("RTS_GROUP_ORDER_SENT participant=%u batch=%u order=%u issuers=%d first_correlation=%u last_correlation=%u"),
        GetCommandPlayerId(), Batch.Id, Batch.OrderKind, Issuers.Num(), Batch.FirstCorrelation, Batch.LastCorrelation);
#endif
    // The same requested point for every issuer (§3 step 4); spacing is a later feature.
    for (APolitikaPrototypeUnit* Issuer : Issuers) SubmitIntent(ActiveMatch, Issuer, Target, Order);
}

bool APolitikaPrototypePlayerController::RecordBatchOutcome(const FPolitikaNetCommandOutcome& Outcome)
{
    FCommandBatch* Batch = CommandBatches.FindByPredicate([&Outcome](const FCommandBatch& Candidate)
        { return Outcome.CorrelationId >= Candidate.FirstCorrelation && Outcome.CorrelationId <= Candidate.LastCorrelation; });
    if (!Batch || Batch->Tally.expected <= 1) return false;
    if (!Batch->Tally.Record(Outcome.bAccepted, Outcome.Rejection)) return true;
    static const TCHAR* OrderNames[] = {
        TEXT("Move"), TEXT("Attack"), TEXT("Attack-move"), TEXT("Stop"), TEXT("Guard"),
        TEXT("Grenade"), TEXT("Reload"), TEXT("Sprint")
    };
    const TCHAR* OrderName = Batch->OrderKind < UE_ARRAY_COUNT(OrderNames) ? OrderNames[Batch->OrderKind] : TEXT("Order");
    const auto& Tally = Batch->Tally;
    FString Feedback = FString::Printf(TEXT("%s: %u of %u units accepted"), OrderName, Tally.accepted, Tally.expected);
    if (Tally.rejected > 0)
        Feedback += FString::Printf(TEXT("  ·  %u rejected (%s)"), Tally.rejected,
            *APolitikaPrototypeHUD::RejectionText(static_cast<uint8>(Tally.DominantRejection())));
    if (!Tally.Complete()) Feedback += FString::Printf(TEXT("  ·  %u pending"), Tally.expected - Tally.accepted - Tally.rejected);
    if (APolitikaPrototypeHUD* PrototypeHUD = Cast<APolitikaPrototypeHUD>(GetHUD())) PrototypeHUD->ShowLocalFeedback(Feedback);
#if !UE_BUILD_SHIPPING
    if (Tally.Complete())
        UE_LOG(LogPolitikaStartup, Display,
            TEXT("RTS_GROUP_ORDER_OUTCOME participant=%u batch=%u order=%u expected=%u accepted=%u rejected=%u dominant_rejection=%u"),
            GetCommandPlayerId(), Batch->Id, Batch->OrderKind, Tally.expected, Tally.accepted, Tally.rejected,
            Tally.rejected > 0 ? static_cast<uint32>(Tally.DominantRejection()) : 0u);
#endif
    return true;
}

void APolitikaPrototypePlayerController::OrderUnderCursor()
{
    // A held selection stroke never turns into an order, and the group bar captures its clicks.
    if (bSelectPressed || GroupBarSlotUnderCursor() != INDEX_NONE) return;
    APolitikaPrototypeMatch* ActiveMatch = ResolveMatch();
    const TArray<APolitikaPrototypeUnit*> Selected = GetCommandableSelectedUnits(ActiveMatch);
    if (Selected.IsEmpty()) return;
    FHitResult Hit;
    if (!GetHitResultUnderCursorByChannel(UEngineTypes::ConvertToTraceType(ECC_Visibility), true, Hit))
    {
        ShowLocalFeedback(TEXT("Order rejected: click valid map ground or a hostile unit."));
        return;
    }
    if (APolitikaPrototypeUnit* Target = Cast<APolitikaPrototypeUnit>(Hit.GetActor()))
    {
        SubmitGroupIntent(ActiveMatch, Selected, Target, politika::rules::AttackOrder{Target->GetStableId()});
    }
    else
    {
        const FVector2D MapPosition(Hit.ImpactPoint.X - ActiveMatch->GetPlayableMin().X,
            Hit.ImpactPoint.Y - ActiveMatch->GetPlayableMin().Y);
        SubmitGroupIntent(ActiveMatch, Selected, nullptr, politika::rules::MoveOrder{{MapPosition.X, MapPosition.Y}});
    }
}

void APolitikaPrototypePlayerController::StopSelected()
{
    APolitikaPrototypeMatch* ActiveMatch = ResolveMatch();
    SubmitGroupIntent(ActiveMatch, GetCommandableSelectedUnits(ActiveMatch), nullptr, politika::rules::StopOrder{});
}

void APolitikaPrototypePlayerController::GuardSelected()
{
    APolitikaPrototypeMatch* ActiveMatch = ResolveMatch();
    SubmitGroupIntent(ActiveMatch, GetCommandableSelectedUnits(ActiveMatch), nullptr, politika::rules::GuardOrder{});
}

void APolitikaPrototypePlayerController::AttackMoveSelected()
{
    APolitikaPrototypeMatch* ActiveMatch = ResolveMatch();
    const TArray<APolitikaPrototypeUnit*> Selected = GetCommandableSelectedUnits(ActiveMatch);
    if (Selected.IsEmpty() || GroupBarSlotUnderCursor() != INDEX_NONE) return;
    FHitResult Hit;
    if (GetHitResultUnderCursorByChannel(UEngineTypes::ConvertToTraceType(ECC_Visibility), true, Hit))
    {
        const FVector2D MapPosition(Hit.ImpactPoint.X - ActiveMatch->GetPlayableMin().X,
            Hit.ImpactPoint.Y - ActiveMatch->GetPlayableMin().Y);
        SubmitGroupIntent(ActiveMatch, Selected, nullptr, politika::rules::AttackMoveOrder{{MapPosition.X, MapPosition.Y}});
    }
    else
        ShowLocalFeedback(TEXT("Attack-move rejected: click valid map ground."));
}

void APolitikaPrototypePlayerController::ReturnToMenu()
{
    // Esc first cancels a held selection stroke; it leaves the match only when idle.
    if (bSelectPressed)
    {
        CancelSelectPress(TEXT("Escape"));
        return;
    }
    // The host/standalone authority ends its match; a remote client only leaves.
    // The server observes that departure through Logout.
    if (APolitikaPrototypeMatch* ActiveMatch = ResolveMatch(); ActiveMatch && ActiveMatch->HasAuthority())
        ActiveMatch->BeginReturnToMenu();
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

APolitikaPrototypeHUD::APolitikaPrototypeHUD()
{
    // Map music is a local-viewer cosmetic, so it lives with the HUD that exists on
    // each rendering player (host and remote) rather than on the server-only GameMode.
    CreateDefaultSubobject<UPolitikaTestMapMusicComponent>(TEXT("TestMapMusic"));
}

void APolitikaPrototypeHUD::AttachMatch(APolitikaPrototypeMatch* InMatch)
{
    if (Match.Get() == InMatch) return;
    Match = InMatch;
    CommandFeedback.Reset();
}

void APolitikaPrototypeHUD::ShowCommandOutcome(const FPolitikaNetCommandOutcome& Outcome)
{
    static const TCHAR* OrderNames[] = {
        TEXT("Move"), TEXT("Attack"), TEXT("Attack-move"), TEXT("Stop"), TEXT("Guard"),
        TEXT("Grenade"), TEXT("Reload"), TEXT("Sprint")
    };
    const uint8 OrderIndex = Outcome.OrderKind;
    const TCHAR* OrderName = OrderIndex < UE_ARRAY_COUNT(OrderNames) ? OrderNames[OrderIndex] : TEXT("Command");
    if (Outcome.bAccepted)
    {
        CommandFeedback = FString::Printf(TEXT("%s order accepted  ·  #%u"), OrderName, Outcome.CorrelationId);
        return;
    }
    CommandFeedback = FString::Printf(TEXT("%s rejected: %s  ·  #%u"),
        OrderName, *RejectionText(Outcome.Rejection), Outcome.CorrelationId);
}

FString APolitikaPrototypeHUD::RejectionText(uint8 Rejection)
{
    static const TCHAR* RejectionNames[] = {
        TEXT("No rejection"), TEXT("You cannot command that unit."), TEXT("That unit cannot move."),
        TEXT("That unit cannot attack."), TEXT("The map bounds are invalid."),
        TEXT("That destination is outside the map."), TEXT("That target is no longer available."),
        TEXT("That target cannot be attacked."), TEXT("Orders cannot target friendly units."),
        TEXT("The selected unit no longer exists."), TEXT("That unit is unavailable."),
        TEXT("You cannot command that unit."), TEXT("The match is not accepting commands."),
        TEXT("That command belongs to an earlier match."),
        TEXT("Ability unavailable: check range, ammunition or cooldown."),
        TEXT("The command was malformed and was not applied."),
        TEXT("That command was already processed."),
        TEXT("Too many pending commands; try again."),
        TEXT("That target is not visible to your team.")
    };
    static_assert(UE_ARRAY_COUNT(RejectionNames) == politika::rules::kOrderRejectionCount);
    return Rejection < UE_ARRAY_COUNT(RejectionNames) ? RejectionNames[Rejection] : TEXT("Order rejected.");
}

void APolitikaPrototypeHUD::DrawHUD()
{
    Super::DrawHUD();
    if (!Canvas || !GetWorld()) return;
    APolitikaPrototypeMatch* ActiveMatch = Match.Get();
    if (!ActiveMatch) return;

    // UCanvas text is drawn in physical pixels, unlike the Slate shell which
    // receives the project's DPI scale from the viewport layer manager.
    const float UiScale = UiScaleFor(Canvas->ClipX);
    float Y = 38.0f * UiScale;
    const float Left = 40.0f * UiScale;
    const FLinearColor Ink(0.91f, 0.93f, 0.84f);
    // Fog shading first so labels and HUD text stay legible on top of it.
    if (APlayerController* Controller = GetOwningPlayerController())
    {
        DrawLocalFog(*Controller, *ActiveMatch);
        DrawLastKnownContacts(*Controller, *ActiveMatch, UiScale);
    }
    // Identical placeholder skins must not hide allegiance or target health
    // from human playtesters. These are read-only views of admitted unit state.
    if (APlayerController* Controller = GetOwningPlayerController())
    {
        for (const APolitikaPrototypeUnit* Unit : ActiveMatch->GetUnits())
        {
            // A listen host keeps every unit in memory; labels follow its team's fog view.
            if (!Unit || !Unit->IsAlive() || Unit->IsLocalFogHidden()) continue;
            FVector2D Screen;
            if (!Controller->ProjectWorldLocationToScreen(Unit->GetActorLocation() + FVector(0,0,100), Screen, true)) continue;
            const uint32 Viewer = ActiveMatch->GetLocalRuntimePlayerId();
            const bool bFriendly = Unit->GetOwnerPlayer() == Viewer || politika::rules::AreAllied(
                Unit->GetOwnerPlayer(), Unit->GetTeamId(), Viewer, ActiveMatch->GetLocalViewerTeam());
            const bool bUnaligned = Unit->GetTeamId() == politika::rules::kUnalignedTeam;
            const FLinearColor TeamColor = bFriendly ? FLinearColor(0.35f,0.9f,0.65f)
                : bUnaligned ? FLinearColor(0.85f,0.82f,0.62f) : FLinearColor(1,0.4f,0.3f);
            const float LabelX = Screen.X - 65 * UiScale;
            const float LabelY = Screen.Y - 18 * UiScale;
            DrawRect(FLinearColor(0.02f,0.025f,0.025f,0.85f), LabelX, LabelY, 135*UiScale, 22*UiScale);
            DrawText(FString::Printf(TEXT("%s #%llu  %d/%d"),
                bFriendly ? TEXT("ALLY") : bUnaligned ? TEXT("UNALIGNED") : TEXT("TARGET"),
                Unit->GetStableId(), Unit->GetHealth(), Unit->GetMaxHealth()), TeamColor,
                LabelX+4*UiScale, LabelY+3*UiScale, nullptr, 0.85f*UiScale);
        }
    }
    DrawText(TEXT("POLITIKA  /  RTS DEVELOPMENT SLICE"), Ink, Left, Y, nullptr, 1.5f * UiScale); Y += 34.0f * UiScale;
    const ENetMode NetMode = GetNetMode();
    if (NetMode != NM_Standalone)
    {
        // Session identity and authoritative lifecycle, from server-owned replicated state.
        const uint32 Viewer = ActiveMatch->GetLocalRuntimePlayerId();
        const TCHAR* Role = NetMode == NM_Client ? TEXT("NETWORK CLIENT") : TEXT("LISTEN HOST");
        const uint8 ViewerTeam = ActiveMatch->GetLocalViewerTeam();
        FString Session = FString::Printf(TEXT("%s  ·  PLAYER %u  ·  TEAM %c  ·  %s + UNALIGNED  ·  MATCH %llu"), Role, Viewer,
            ViewerTeam != 0 ? TCHAR('A' + ViewerTeam - 1) : TCHAR('-'),
            UTF8_TO_TCHAR(politika::rules::LayoutName(ActiveMatch->GetSessionLayout()).data()),
            ActiveMatch->GetMatchGeneration());
        if (ActiveMatch->IsFogOfWarEnabled()) Session += TEXT("  ·  FOG");
        switch (ActiveMatch->GetMatchPhase())
        {
            case politika::rules::MatchPhase::Starting: Session += TEXT("  ·  WAITING FOR PARTICIPANTS"); break;
            case politika::rules::MatchPhase::Active: Session += TEXT("  ·  ACTIVE"); break;
            case politika::rules::MatchPhase::Results:
            case politika::rules::MatchPhase::Terminal: Session += TEXT("  ·  MATCH OVER"); break;
            default: break;
        }
        DrawText(Session, FLinearColor(0.55f, 0.82f, 0.95f), Left, Y, nullptr, UiScale); Y += 28.0f * UiScale;
        if (const TOptional<FPolitikaPrototypeMatchResult>& Result = ActiveMatch->GetCommittedResult(); Result.IsSet())
        {
            const FString Outcome = Result->WinningTeamId == 0
                ? FString::Printf(TEXT("MATCH ENDED: %s  ·  Esc returns to menu"), *Result->Reason.ToString())
                : FString::Printf(TEXT("%s  ·  TEAM %c (PLAYER %u) %s  ·  Esc returns to menu"),
                    Result->WinningTeamId == ViewerTeam ? TEXT("VICTORY") : TEXT("DEFEAT"),
                    TCHAR('A' + Result->WinningTeamId - 1), Result->WinningPlayerId, *Result->Reason.ToString());
            DrawText(Outcome, FLinearColor(0.94f, 0.78f, 0.31f), Left, Y, nullptr, 1.25f * UiScale); Y += 32.0f * UiScale;
        }
    }
    DrawText(TEXT("LMB select / drag box  ·  Shift add  ·  Ctrl+drag remove  ·  Ctrl+click or double-click same type  ·  Tab subgroup"), Ink, Left, Y, nullptr, UiScale); Y += 26.0f * UiScale;
    DrawText(TEXT("RMB move / attack  ·  Q attack-move  ·  G guard  ·  X stop  ·  WASD pan  ·  wheel zoom  ·  Esc menu"), Ink, Left, Y, nullptr, UiScale); Y += 28.0f * UiScale;
    const TArray<APolitikaPrototypeUnit*> SelectedUnits = ActiveMatch->GetSelectedUnitsForPlayer(ActiveMatch->GetLocalRuntimePlayerId());
    const APolitikaPrototypePlayerController* PrototypeOwner = Cast<APolitikaPrototypePlayerController>(GetOwningPlayerController());
    {
        APolitikaPrototypeUnit* Selected = SelectedUnits.IsEmpty() ? nullptr : SelectedUnits[0];
        if (SelectedUnits.Num() > 1)
        {
            // Group summary: count, combined health and how many are moving.
            int32 Health = 0;
            int32 MaxHealth = 0;
            int32 Moving = 0;
            for (const APolitikaPrototypeUnit* Unit : SelectedUnits)
            {
                Health += Unit->GetHealth();
                MaxHealth += Unit->GetMaxHealth();
                const EPolitikaPrototypeOrder Order = Unit->GetCurrentOrder();
                Moving += Order == EPolitikaPrototypeOrder::Move || Order == EPolitikaPrototypeOrder::AttackMove ? 1 : 0;
            }
            DrawText(FString::Printf(TEXT("%d UNITS SELECTED    HP %d / %d    %d moving"), SelectedUnits.Num(), Health, MaxHealth,
                Moving), Ink, Left, Y, nullptr, 1.25f * UiScale);
            Y += 30.0f * UiScale;
            // Mixed archetypes: name the Tab subgroup that receives grenade, reload and sprint.
            std::vector<std::pair<politika::selection::EntityId, FName>> Members;
            for (const APolitikaPrototypeUnit* Unit : SelectedUnits) Members.emplace_back(Unit->GetStableId(), Unit->GetArchetypeId());
            const std::vector<FName> Subgroups = politika::selection::SubgroupOrder(std::move(Members));
            if (Subgroups.size() > 1 && PrototypeOwner)
            {
                const std::size_t ActiveIndex = static_cast<std::size_t>(PrototypeOwner->GetActiveSubgroup()) % Subgroups.size();
                const APolitikaPrototypeUnit* const* First = SelectedUnits.FindByPredicate(
                    [&](const APolitikaPrototypeUnit* Unit) { return Unit->GetArchetypeId() == Subgroups[ActiveIndex]; });
                DrawText(FString::Printf(TEXT("SUBGROUP %d / %d: %s  ·  Tab next"), static_cast<int32>(ActiveIndex) + 1,
                    static_cast<int32>(Subgroups.size()), First ? *(*First)->GetUnitName().ToString() : TEXT("")),
                    FLinearColor(0.94f, 0.78f, 0.31f), Left, Y, nullptr, UiScale);
                Y += 26.0f * UiScale;
            }
        }
        else if (Selected)
        {
            const TCHAR* Order = TEXT("Idle");
            switch (Selected->GetCurrentOrder())
            {
                case EPolitikaPrototypeOrder::Move: Order = TEXT("Moving"); break;
                case EPolitikaPrototypeOrder::Attack: Order = TEXT("Attacking"); break;
                case EPolitikaPrototypeOrder::AttackMove: Order = TEXT("Attack-move"); break;
                case EPolitikaPrototypeOrder::Stop: Order = TEXT("Stopped"); break;
                case EPolitikaPrototypeOrder::Guard: Order = TEXT("Guarding"); break;
                case EPolitikaPrototypeOrder::Grenade:
                case EPolitikaPrototypeOrder::Reload:
                case EPolitikaPrototypeOrder::Sprint:
                case EPolitikaPrototypeOrder::Idle: break;
            }
            DrawText(FString::Printf(TEXT("%s    HP %d / %d    %s"), *Selected->GetUnitName().ToString(),
                Selected->GetHealth(), Selected->GetMaxHealth(), Order), Ink, Left, Y, nullptr, 1.25f * UiScale);
            Y += 30.0f * UiScale;
            if (Selected->HasInfantryBehavior())
            {
                DrawText(FString::Printf(TEXT("Ammo %d  %s  |  R reload  E grenade at cursor  F sprint"),
                    Selected->GetAmmo(), Selected->IsReloading() ? TEXT("RELOADING") : TEXT("")),
                    Ink, Left, Y, nullptr, UiScale);
                Y += 26.0f * UiScale;
            }
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
    // Networked processes show only this controller's own correlated feedback; the
    // server-wide string would otherwise expose another participant's commands.
    const FString DefaultHint = TEXT("Select a friendly unit; right-click ground or an enemy to issue an order.");
    const FString& Feedback = !CommandFeedback.IsEmpty() ? CommandFeedback
        : NetMode == NM_Standalone ? ActiveMatch->GetLastCommandFeedback() : DefaultHint;
    DrawText(Feedback, Ink, Left, Y, nullptr, UiScale);

    // Original text/glyph placeholders keep this prototype independent of reference-game art.
    const float PanelLeft = Left;
    const float RowHeight = 34.0f * UiScale;
    const float PanelWidth = 390.0f * UiScale;
    const float PanelTop = Canvas->ClipY - (RowHeight * 5.0f) - (36.0f * UiScale);
    DrawText(TEXT("COMMANDS"), Ink, PanelLeft, PanelTop, nullptr, 0.9f * UiScale);
    const TCHAR* Labels[] = { TEXT("MOVE"), TEXT("ATTACK"), TEXT("ATTACK-MOVE"), TEXT("STOP"), TEXT("GUARD") };
    const TCHAR* Hints[] = { TEXT("RMB ground"), TEXT("RMB hostile"), TEXT("Q + ground"), TEXT("X"), TEXT("G · provisional") };
    const TCHAR* Marks[] = { TEXT("M"), TEXT("!"), TEXT("A+"), TEXT("X"), TEXT("G") };
    const EPolitikaPrototypeOrder Orders[] = {
        EPolitikaPrototypeOrder::Move, EPolitikaPrototypeOrder::Attack,
        EPolitikaPrototypeOrder::AttackMove, EPolitikaPrototypeOrder::Stop,
        EPolitikaPrototypeOrder::Guard
    };
    const APolitikaPrototypeUnit* Selected = SelectedUnits.IsEmpty() ? nullptr : SelectedUnits[0];
    for (int32 Index = 0; Index < UE_ARRAY_COUNT(Labels); ++Index)
    {
        const float RowY = PanelTop + (18.0f * UiScale) + (Index * RowHeight);
        // A row is enabled when any selected unit can carry it out; others get per-unit rejections.
        const bool bEnabled = SelectedUnits.ContainsByPredicate([Index](const APolitikaPrototypeUnit* Unit)
            { return Unit->IsAlive() && ((Index == 0 || Index == 2) ? Unit->CanMove() : Index == 1 ? Unit->CanAttack() : true); });
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
    // Control-group bar (4C-G): 1..9 then 0 along the bottom edge. Each slot shows its member
    // count and is clickable; the slot matching the current selection is outlined.
    if (PrototypeOwner)
    {
        using namespace politika::selection;
        const ControlGroups& Groups = PrototypeOwner->GetControlGroups();
        std::vector<EntityId> SelectedIds;
        for (const APolitikaPrototypeUnit* Unit : SelectedUnits) SelectedIds.push_back(Unit->GetStableId());
        SelectedIds = Normalize(std::move(SelectedIds));
        const ScreenRect FirstSlot = GroupBarSlotRect(1, Canvas->ClipX, Canvas->ClipY, UiScale);
        DrawText(TEXT("CONTROL GROUPS   # select  ·  # # centre camera  ·  Ctrl+# set  ·  Shift+# add  ·  Alt+# move  ·  or click a slot"),
            Ink, FirstSlot.min.x, FirstSlot.min.y - 22.0f * UiScale, nullptr, 0.8f * UiScale);
        const FLinearColor Muted(0.46f, 0.50f, 0.48f);
        const FLinearColor Gold(0.94f, 0.78f, 0.31f);
        for (std::size_t Slot = 0; Slot < kControlGroupSlots; ++Slot)
        {
            const ScreenRect Rect = GroupBarSlotRect(Slot, Canvas->ClipX, Canvas->ClipY, UiScale);
            const float X0 = static_cast<float>(Rect.min.x);
            const float Y0 = static_cast<float>(Rect.min.y);
            const float W = static_cast<float>(Rect.max.x - Rect.min.x);
            const float H = static_cast<float>(Rect.max.y - Rect.min.y);
            const std::vector<EntityId>& Members = Groups.Members(Slot);
            const bool bHasMembers = !Members.empty();
            const bool bSelected = bHasMembers && Members == SelectedIds;
            DrawRect(bSelected ? FLinearColor(0.16f, 0.22f, 0.18f, 0.94f)
                : bHasMembers ? FLinearColor(0.10f, 0.14f, 0.15f, 0.88f) : FLinearColor(0.05f, 0.06f, 0.07f, 0.70f),
                X0, Y0, W, H);
            if (bSelected)
            {
                const float Thickness = FMath::Max(1.0f, 2.0f * UiScale);
                DrawLine(X0, Y0, X0 + W, Y0, Gold, Thickness);
                DrawLine(X0 + W, Y0, X0 + W, Y0 + H, Gold, Thickness);
                DrawLine(X0 + W, Y0 + H, X0, Y0 + H, Gold, Thickness);
                DrawLine(X0, Y0 + H, X0, Y0, Gold, Thickness);
            }
            DrawText(FString::FromInt(static_cast<int32>(Slot)), bHasMembers ? Ink : Muted, X0 + 5.0f * UiScale, Y0 + 3.0f * UiScale,
                nullptr, 0.8f * UiScale);
            DrawText(bHasMembers ? FString::FromInt(static_cast<int32>(Members.size())) : FString(TEXT("-")),
                bSelected ? Gold : bHasMembers ? Ink : Muted, X0 + 24.0f * UiScale, Y0 + 16.0f * UiScale, nullptr, 1.15f * UiScale);
        }
    }
    // In-progress box selection, drawn last so it stays visible over every panel.
    if (const APolitikaPrototypePlayerController* Controller = Cast<APolitikaPrototypePlayerController>(GetOwningPlayerController()))
    {
        FVector2D BoxMin;
        FVector2D BoxMax;
        if (Controller->GetSelectionBox(BoxMin, BoxMax))
        {
            DrawRect(FLinearColor(0.35f, 0.9f, 0.65f, 0.12f), BoxMin.X, BoxMin.Y, BoxMax.X - BoxMin.X, BoxMax.Y - BoxMin.Y);
            const FLinearColor Edge(0.35f, 0.9f, 0.65f, 0.95f);
            const float Thickness = FMath::Max(1.0f, UiScale);
            DrawLine(BoxMin.X, BoxMin.Y, BoxMax.X, BoxMin.Y, Edge, Thickness);
            DrawLine(BoxMax.X, BoxMin.Y, BoxMax.X, BoxMax.Y, Edge, Thickness);
            DrawLine(BoxMax.X, BoxMax.Y, BoxMin.X, BoxMax.Y, Edge, Thickness);
            DrawLine(BoxMin.X, BoxMax.Y, BoxMin.X, BoxMin.Y, Edge, Thickness);
        }
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
    ActiveMatch = GetWorld()->SpawnActor<APolitikaPrototypeMatch>(FVector::ZeroVector, FRotator::ZeroRotator);
    if (GetNetMode() == NM_ListenServer)
    {
        UE_LOG(LogPolitikaStartup, Display, TEXT("MP_LISTEN_SERVER_READY port=%d net_driver=%d protocol=%lld"),
            GetWorld()->URL.Port, GetWorld()->GetNetDriver() ? 1 : 0, FPolitikaBootstrap::Get().NetworkProtocol());
    }
    else if (GetNetMode() == NM_DedicatedServer)
    {
        // Headless evidence: no local player, renderer or audio output exists on this process.
        UE_LOG(LogPolitikaStartup, Display,
            TEXT("MP_DEDICATED_SERVER_READY port=%d net_driver=%d protocol=%lld local_players=%d can_render=%d can_render_audio=%d generation=%llu"),
            GetWorld()->URL.Port, GetWorld()->GetNetDriver() ? 1 : 0, FPolitikaBootstrap::Get().NetworkProtocol(),
            GetGameInstance() ? GetGameInstance()->GetNumLocalPlayers() : -1, FApp::CanEverRender() ? 1 : 0,
            FApp::CanEverRenderAudio() ? 1 : 0, ActiveMatch.IsValid() ? ActiveMatch->GetMatchGeneration() : 0);
    }
}
