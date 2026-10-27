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
#include "InputCoreTypes.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "UObject/ConstructorHelpers.h"
#include "PolitikaGameInstance.h"
#include "PolitikaPrototypeContent.h"
#include "Rules.h"

namespace
{
constexpr float UnitGroundZ = 42.0f;

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
    bool bInAttackable, UStaticMesh* InMesh, const FVector& InMeshScale, const FLinearColor& InColor)
{
    StableId = InStableId;
    OwnerPlayer = InOwner;
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
    ApplyPrototypeColor(Body, InColor);
    SelectionIndicator->SetStaticMesh(LoadPrototypeMesh(TEXT("/Engine/BasicShapes/Cylinder.Cylinder")));
    SelectionIndicator->SetRelativeLocation(FVector(0, 0, -34.0f / InMeshScale.Z));
    SelectionIndicator->SetRelativeScale3D(FVector(2.1f / InMeshScale.X, 2.1f / InMeshScale.Y, 0.025f / InMeshScale.Z));
    ApplyPrototypeColor(SelectionIndicator, FLinearColor(0.96f, 0.83f, 0.30f));
    SetActorEnableCollision(true);
}

void APolitikaPrototypeUnit::SetMoveOrder(EPolitikaPrototypeOrder Kind, const FVector& Destination)
{
    CurrentOrder = Kind;
    OrderDestination = Destination;
    OrderTarget.Reset();
}

void APolitikaPrototypeUnit::SetAttackOrder(APolitikaPrototypeUnit* Target)
{
    CurrentOrder = EPolitikaPrototypeOrder::Attack;
    OrderTarget = Target;
}

void APolitikaPrototypeUnit::SetGuardOrder()
{
    CurrentOrder = EPolitikaPrototypeOrder::Guard;
    OrderTarget.Reset();
}

void APolitikaPrototypeUnit::StopOrder()
{
    CurrentOrder = EPolitikaPrototypeOrder::Stop;
    OrderTarget.Reset();
    OrderDestination = GetActorLocation();
}

void APolitikaPrototypeUnit::SetSelectedVisual(bool bSelected)
{
    SelectionIndicator->SetVisibility(bSelected, true);
}

void APolitikaPrototypeUnit::SimulateMovement(float DeltaSeconds)
{
    if (!IsAlive() || MoveSpeed <= 0.0f) return;
    const bool bCanAdvance = CurrentOrder == EPolitikaPrototypeOrder::Move
        || CurrentOrder == EPolitikaPrototypeOrder::AttackMove
        || ((CurrentOrder == EPolitikaPrototypeOrder::Attack || CurrentOrder == EPolitikaPrototypeOrder::Guard)
            && OrderTarget.IsValid());
    if (!bCanAdvance) return;

    FVector Destination = OrderDestination;
    if ((CurrentOrder == EPolitikaPrototypeOrder::Attack || CurrentOrder == EPolitikaPrototypeOrder::Guard)
        && OrderTarget.IsValid())
    {
        Destination = OrderTarget->GetActorLocation();
        const FVector ToTarget = Destination - GetActorLocation();
        if (ToTarget.Size2D() <= WeaponRange)
        {
            Destination = GetActorLocation();
        }
        else
        {
            Destination -= ToTarget.GetSafeNormal2D() * WeaponRange;
        }
    }
    Destination.Z = GetActorLocation().Z;
    const FVector Next = FMath::VInterpConstantTo(GetActorLocation(), Destination, DeltaSeconds, MoveSpeed);
    FHitResult Hit;
    SetActorLocation(Next, true, &Hit, ETeleportType::None);
    if (Hit.bBlockingHit && CurrentOrder == EPolitikaPrototypeOrder::Move)
    {
        StopOrder();
    }
    if (CurrentOrder == EPolitikaPrototypeOrder::Move
        && FlatDistance(GetActorLocation(), OrderDestination) < 18.0f)
    {
        CurrentOrder = EPolitikaPrototypeOrder::Idle;
    }
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

void APolitikaPrototypeUnit::ApplyPendingDamage()
{
    if (PendingDamage <= 0.0f || !IsAlive()) return;
    Health = FMath::Max(0, Health - FMath::CeilToInt(PendingDamage));
    PendingDamage = 0.0f;
    if (Health == 0) StopOrder();
}

void APolitikaPrototypeUnit::CleanupDeadTarget()
{
    if (APolitikaPrototypeUnit* Target = OrderTarget.Get(); Target && !Target->IsAlive())
    {
        OrderTarget.Reset();
        if (CurrentOrder == EPolitikaPrototypeOrder::Attack) CurrentOrder = EPolitikaPrototypeOrder::Idle;
    }
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
    BuildScenario();
#if !UE_BUILD_SHIPPING
    if (bContentReady) BeginAcceptanceHarness();
#endif
}

void APolitikaPrototypeMatch::BuildScenario()
{
    FPolitikaPrototypeContentSnapshot Snapshot;
    TArray<FPolitikaContentDiagnostic> Diagnostics;
    if (!FPolitikaPrototypeContentLoader::LoadAndValidate(GetWorld(), UPolitikaPrototypeContentValidator::PrototypeMapAssetId, Snapshot, Diagnostics))
    {
        for (const FPolitikaContentDiagnostic& Diagnostic : Diagnostics)
            UE_LOG(LogPolitikaStartup, Error, TEXT("CONTENT_VALIDATION %s"), *UPolitikaPrototypeContentValidator::FormatDiagnostic(Diagnostic));
        LastCommandFeedback = TEXT("Prototype setup failed: gameplay content validation failed. See CONTENT_VALIDATION diagnostics.");
        return;
    }
    ContentCatalog = Snapshot.Catalog;
    MapDefinition = Snapshot.MapDefinition;
    PlayableMin = MapDefinition->PlayableMin;
    PlayableMax = MapDefinition->PlayableMax;
    const FPolitikaPrototypePlayerSlot* HumanSlot = MapDefinition->PlayerSlots.FindByPredicate([](const FPolitikaPrototypePlayerSlot& Slot) { return Slot.bHumanControlled; });
    if (!HumanSlot) return;
    LocalRuntimePlayerId = static_cast<uint32>(HumanSlot->RuntimePlayerId);

    Diagnostics.Reset();
    if (!UPolitikaPrototypeContentValidator::ValidateMapSpatial(GetWorld(), MapDefinition, Diagnostics))
    {
        for (const FPolitikaContentDiagnostic& Diagnostic : Diagnostics)
            UE_LOG(LogPolitikaStartup, Error, TEXT("CONTENT_VALIDATION %s"), *UPolitikaPrototypeContentValidator::FormatDiagnostic(Diagnostic));
        LastCommandFeedback = TEXT("Prototype setup failed: map traversal validation failed. See CONTENT_VALIDATION diagnostics.");
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
        const FPolitikaPrototypePlayerSlot* Slot = MapDefinition->PlayerSlots.FindByPredicate([&](const FPolitikaPrototypePlayerSlot& Candidate) { return Candidate.SlotId == Placement.SlotId; });
        if (!Definition || !Weapon || !Slot) return;
        APolitikaPrototypeUnit* Unit = GetWorld()->SpawnActor<APolitikaPrototypeUnit>(Placement.Location, Placement.Rotation);
        if (!Unit) return;
        Unit->Configure(NextStableId++, Slot->RuntimePlayerId, Definition->DisplayName, Definition->MaxHealth,
            Definition->MovementSpeedCmPerSecond, Weapon->RangeCm, Weapon->Damage, Weapon->CooldownSeconds,
            Definition->bAttackable, Definition->VisualMesh, Definition->VisualScale, Definition->FactionTint);
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
        return;
    }
    UE_LOG(LogPolitikaStartup, Display, TEXT("MATCH_CONTENT_READY map=%s units=%d objective=%s"),
        *MapDefinition->StableId.ToString(), Units.Num(), *MapDefinition->Objective.StableId.ToString());
}

void APolitikaPrototypeMatch::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    if (!bContentReady) return;
    StepAccumulator += FMath::Min(DeltaSeconds, 0.25f);
    int32 Steps = 0;
    while (StepAccumulator >= SimulationStepSeconds && Steps < 5)
    {
        AdmitPendingOrders();
        RunSimulationStep(SimulationStepSeconds);
        StepAccumulator -= SimulationStepSeconds;
        ++Steps;
    }
    if (Steps == 5 && StepAccumulator >= SimulationStepSeconds) StepAccumulator = 0.0f;
#if !UE_BUILD_SHIPPING
    TickAcceptanceHarness(DeltaSeconds);
#endif
}

#if !UE_BUILD_SHIPPING
void APolitikaPrototypeMatch::BeginAcceptanceHarness()
{
    FParse::Value(FCommandLine::Get(), TEXT("RTSAcceptance="), AcceptanceMode);
    AcceptanceMode = AcceptanceMode.ToLower();
    if (AcceptanceMode != TEXT("capturepause") && AcceptanceMode != TEXT("attackerdeath")
        && AcceptanceMode != TEXT("combined")) return;

    UE_LOG(LogTemp, Display, TEXT("RTS_ACCEPTANCE_BEGIN mode=%s"), *AcceptanceMode);
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
        Capturer->SetMoveOrder(EPolitikaPrototypeOrder::Move, Objective->GetActorLocation());
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
            Capturer->SetMoveOrder(EPolitikaPrototypeOrder::Move, FVector(-500.0f, -400.0f, UnitGroundZ));
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
    if (APolitikaPrototypeUnit* Previous = GetSelectedUnitForPlayer(PlayerId))
        Previous->SetSelectedVisual(false);
    if (!Unit || Unit->GetOwnerPlayer() != PlayerId || !Unit->IsAlive())
    {
        SelectionByPlayer.Remove(PlayerId);
        return;
    }
    SelectionByPlayer.Add(PlayerId, Unit);
    Unit->SetSelectedVisual(true);
}

void APolitikaPrototypeMatch::SubmitMove(uint32 PlayerId, APolitikaPrototypeUnit* Unit, const FVector& Destination, bool bAttackMove)
{
    FPendingOrder Pending;
    Pending.PlayerId = PlayerId;
    Pending.Unit = Unit;
    Pending.Destination = Destination;
    Pending.Kind = bAttackMove ? EPolitikaPrototypeOrder::AttackMove : EPolitikaPrototypeOrder::Move;
    PendingOrders.Add(Pending);
}

void APolitikaPrototypeMatch::SubmitAttack(uint32 PlayerId, APolitikaPrototypeUnit* Unit, APolitikaPrototypeUnit* Target)
{
    FPendingOrder Pending;
    Pending.PlayerId = PlayerId;
    Pending.Unit = Unit;
    Pending.Target = Target;
    Pending.Kind = EPolitikaPrototypeOrder::Attack;
    PendingOrders.Add(Pending);
}

void APolitikaPrototypeMatch::SubmitStop(uint32 PlayerId, APolitikaPrototypeUnit* Unit)
{
    FPendingOrder Pending;
    Pending.PlayerId = PlayerId; Pending.Unit = Unit; Pending.Kind = EPolitikaPrototypeOrder::Stop;
    PendingOrders.Add(Pending);
}

void APolitikaPrototypeMatch::SubmitGuard(uint32 PlayerId, APolitikaPrototypeUnit* Unit)
{
    FPendingOrder Pending;
    Pending.PlayerId = PlayerId; Pending.Unit = Unit; Pending.Kind = EPolitikaPrototypeOrder::Guard;
    PendingOrders.Add(Pending);
}

bool APolitikaPrototypeMatch::ValidateOrder(const FPendingOrder& Pending, FString& OutReason) const
{
    const APolitikaPrototypeUnit* Issuer = Pending.Unit.Get();
    if (!Issuer) { OutReason = TEXT("The selected unit no longer exists."); return false; }
    politika::rules::Entity IssuerRules;
    IssuerRules.id = Issuer->GetStableId();
    IssuerRules.owner_player = Issuer->GetOwnerPlayer();
    IssuerRules.movable = Pending.Kind == EPolitikaPrototypeOrder::Move || Pending.Kind == EPolitikaPrototypeOrder::AttackMove;
    IssuerRules.can_attack = true;

    politika::rules::Order Order = politika::rules::StopOrder{};
    const FVector2D MapPosition(Pending.Destination.X - PlayableMin.X, Pending.Destination.Y - PlayableMin.Y);
    switch (Pending.Kind)
    {
        case EPolitikaPrototypeOrder::Move: Order = politika::rules::MoveOrder{{MapPosition.X, MapPosition.Y}}; break;
        case EPolitikaPrototypeOrder::AttackMove: Order = politika::rules::AttackMoveOrder{{MapPosition.X, MapPosition.Y}}; break;
        case EPolitikaPrototypeOrder::Attack:
        {
            const APolitikaPrototypeUnit* Target = Pending.Target.Get();
            if (!Target) { OutReason = TEXT("That target is no longer available."); return false; }
            Order = politika::rules::AttackOrder{Target->GetStableId()};
            break;
        }
        case EPolitikaPrototypeOrder::Guard: Order = politika::rules::GuardOrder{}; break;
        case EPolitikaPrototypeOrder::Stop:
        case EPolitikaPrototypeOrder::Idle: Order = politika::rules::StopOrder{}; break;
    }

    politika::rules::Entity TargetRules;
    const APolitikaPrototypeUnit* Target = Pending.Target.Get();
    if (Target)
    {
        TargetRules.id = Target->GetStableId();
        TargetRules.owner_player = Target->GetOwnerPlayer();
        TargetRules.attackable = Target->IsAttackable();
    }
    const auto Result = politika::rules::AdmitOrder(IssuerRules, Pending.PlayerId,
        {PlayableMax.X - PlayableMin.X, PlayableMax.Y - PlayableMin.Y}, Order, Target ? &TargetRules : nullptr);
    if (Result) return true;
    switch (Result.rejection)
    {
        case politika::rules::OrderRejection::NotOwned: OutReason = TEXT("You cannot command that unit."); break;
        case politika::rules::OrderRejection::CannotMove: OutReason = TEXT("That unit cannot move."); break;
        case politika::rules::OrderRejection::CannotAttack: OutReason = TEXT("That unit cannot attack."); break;
        case politika::rules::OrderRejection::InvalidBounds: OutReason = TEXT("The map bounds are invalid."); break;
        case politika::rules::OrderRejection::InvalidTarget: OutReason = TEXT("That destination is outside the map."); break;
        case politika::rules::OrderRejection::MissingTarget: OutReason = TEXT("That target is no longer available."); break;
        case politika::rules::OrderRejection::TargetNotAttackable: OutReason = TEXT("That target cannot be attacked."); break;
        case politika::rules::OrderRejection::FriendlyTarget: OutReason = TEXT("Orders cannot target friendly units."); break;
        case politika::rules::OrderRejection::None: OutReason = TEXT("Order rejected."); break;
    }
    return false;
}

void APolitikaPrototypeMatch::AdmitPendingOrders()
{
    for (const FPendingOrder& Pending : PendingOrders)
    {
        FString Reason;
        APolitikaPrototypeUnit* Unit = Pending.Unit.Get();
        if (!ValidateOrder(Pending, Reason))
        {
            LastCommandFeedback = Reason;
            continue;
        }
        switch (Pending.Kind)
        {
            case EPolitikaPrototypeOrder::Move: Unit->SetMoveOrder(Pending.Kind, Pending.Destination); LastCommandFeedback = TEXT("Move order accepted."); break;
            case EPolitikaPrototypeOrder::AttackMove: Unit->SetMoveOrder(Pending.Kind, Pending.Destination); LastCommandFeedback = TEXT("Attack-move order accepted."); break;
            case EPolitikaPrototypeOrder::Attack: Unit->SetAttackOrder(Pending.Target.Get()); LastCommandFeedback = TEXT("Attack order accepted."); break;
            case EPolitikaPrototypeOrder::Guard: Unit->SetGuardOrder(); LastCommandFeedback = TEXT("Guard order accepted."); break;
            case EPolitikaPrototypeOrder::Stop: Unit->StopOrder(); LastCommandFeedback = TEXT("Unit stopped."); break;
            case EPolitikaPrototypeOrder::Idle: break;
        }
    }
    PendingOrders.Reset();
}

void APolitikaPrototypeMatch::RunSimulationStep(float DeltaSeconds)
{
    // Explicit simulation phase order: movement → target/combat intent → damage
    // application → dead-entity cleanup → objective evaluation. UI only reads.
    for (APolitikaPrototypeUnit* Unit : Units) if (Unit) Unit->SimulateMovement(DeltaSeconds);
    for (APolitikaPrototypeUnit* Unit : Units) if (Unit) Unit->ResolveCombat(DeltaSeconds);
    for (APolitikaPrototypeUnit* Unit : Units) if (Unit) Unit->ApplyPendingDamage();
    for (APolitikaPrototypeUnit* Unit : Units) if (Unit) Unit->CleanupDeadTarget();
    for (APolitikaPrototypeUnit* Unit : Units)
    {
        if (Unit && !Unit->IsAlive())
        {
#if !UE_BUILD_SHIPPING
            if (AcceptancePhase == EAcceptancePhase::CombatResolve && Unit == AcceptanceAttacker.Get())
                bAcceptanceAttackerOrderCleared = Unit->GetCurrentOrder() == EPolitikaPrototypeOrder::Stop;
#endif
            LastCommandFeedback = FString::Printf(TEXT("%s destroyed."), *Unit->GetUnitName().ToString());
            Unit->Destroy();
        }
    }
    Units.RemoveAll([](const TObjectPtr<APolitikaPrototypeUnit>& Unit) { return !Unit || !Unit->IsAlive(); });
    if (Objective)
    {
        TArray<APolitikaPrototypeUnit*> RawUnits;
        RawUnits.Reserve(Units.Num());
        for (APolitikaPrototypeUnit* Unit : Units) RawUnits.Add(Unit);
        Objective->UpdateCapture(RawUnits, DeltaSeconds);
    }
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
        ApplyMapCameraIfReady();
        return Existing;
    }
    if (!GetWorld()) return nullptr;
    TActorIterator<APolitikaPrototypeMatch> MatchIt(GetWorld());
    if (!MatchIt) return nullptr;
    Match = *MatchIt;
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
        if (Unit && Unit->GetOwnerPlayer() == GetCommandPlayerId() && Unit->IsAlive())
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
        ActiveMatch->SubmitAttack(GetCommandPlayerId(), SelectedUnit.Get(), Target);
    }
    else
    {
        ActiveMatch->SubmitMove(GetCommandPlayerId(), SelectedUnit.Get(), Hit.ImpactPoint);
}
}

void APolitikaPrototypePlayerController::StopSelected()
{
    if (APolitikaPrototypeMatch* ActiveMatch = ResolveMatch(); ActiveMatch && SelectedUnit.IsValid())
        ActiveMatch->SubmitStop(GetCommandPlayerId(), SelectedUnit.Get());
}

void APolitikaPrototypePlayerController::GuardSelected()
{
    if (APolitikaPrototypeMatch* ActiveMatch = ResolveMatch(); ActiveMatch && SelectedUnit.IsValid())
        ActiveMatch->SubmitGuard(GetCommandPlayerId(), SelectedUnit.Get());
}

void APolitikaPrototypePlayerController::AttackMoveSelected()
{
    APolitikaPrototypeMatch* ActiveMatch = ResolveMatch();
    if (!ActiveMatch || !SelectedUnit.IsValid()) return;
    FHitResult Hit;
    if (GetHitResultUnderCursorByChannel(UEngineTypes::ConvertToTraceType(ECC_Visibility), true, Hit))
        ActiveMatch->SubmitMove(GetCommandPlayerId(), SelectedUnit.Get(), Hit.ImpactPoint, true);
    else
        ActiveMatch->ReportRejectedCommand(TEXT("Attack-move rejected: click valid map ground."));
}

void APolitikaPrototypePlayerController::ReturnToMenu()
{
    if (UPolitikaGameInstance* Game = GetGameInstance<UPolitikaGameInstance>()) Game->ReturnToMenu(this);
}

void APolitikaPrototypePlayerController::PanForward(float Value)
{
    if (FMath::IsNearlyZero(Value) || !GetPawn()) return;
    GetPawn()->AddActorWorldOffset(FVector(0, Value * 1400.0f * GetWorld()->GetDeltaSeconds(), 0), true);
    FVector Location = GetPawn()->GetActorLocation();
    if (APolitikaPrototypeMatch* ActiveMatch = ResolveMatch())
    {
        const FVector2D Min = ActiveMatch->GetCameraPanMin(); const FVector2D Max = ActiveMatch->GetCameraPanMax();
        Location.X = FMath::Clamp(Location.X, Min.X, Max.X);
        Location.Y = FMath::Clamp(Location.Y, Min.Y, Max.Y);
    }
    GetPawn()->SetActorLocation(Location);
}

void APolitikaPrototypePlayerController::PanRight(float Value)
{
    if (FMath::IsNearlyZero(Value) || !GetPawn()) return;
    GetPawn()->AddActorWorldOffset(FVector(Value * 1400.0f * GetWorld()->GetDeltaSeconds(), 0, 0), true);
    FVector Location = GetPawn()->GetActorLocation();
    if (APolitikaPrototypeMatch* ActiveMatch = ResolveMatch())
    {
        const FVector2D Min = ActiveMatch->GetCameraPanMin(); const FVector2D Max = ActiveMatch->GetCameraPanMax();
        Location.X = FMath::Clamp(Location.X, Min.X, Max.X);
        Location.Y = FMath::Clamp(Location.Y, Min.Y, Max.Y);
    }
    GetPawn()->SetActorLocation(Location);
}

void APolitikaPrototypePlayerController::Zoom(float Value)
{
    if (FMath::IsNearlyZero(Value)) return;
    if (APolitikaPrototypeCameraPawn* CameraPawn = Cast<APolitikaPrototypeCameraPawn>(GetPawn()))
    {
        CameraPawn->GetCamera()->SetFieldOfView(FMath::Clamp(CameraPawn->GetCamera()->FieldOfView - Value * 4.0f, 38.0f, 82.0f));
    }
}

void APolitikaPrototypeHUD::DrawHUD()
{
    Super::DrawHUD();
    if (!Canvas || !GetWorld()) return;
    APolitikaPrototypeMatch* Match = nullptr;
    TActorIterator<APolitikaPrototypeMatch> MatchIt(GetWorld());
    if (MatchIt) Match = *MatchIt;
    if (!Match) return;

    // UCanvas text is drawn in physical pixels, unlike the Slate shell which
    // receives the project's DPI scale from the viewport layer manager.
    const float UiScale = FMath::Clamp(Canvas->ClipX / 1920.0f, 1.0f, 2.0f);
    float Y = 38.0f * UiScale;
    const float Left = 40.0f * UiScale;
    const FLinearColor Ink(0.91f, 0.93f, 0.84f);
    DrawText(TEXT("POLITIKA  /  RTS DEVELOPMENT SLICE"), Ink, Left, Y, nullptr, 1.5f * UiScale); Y += 34.0f * UiScale;
    DrawText(TEXT("LMB select  ·  RMB move / attack  ·  A attack-move  ·  G guard  ·  S stop  ·  WASD pan  ·  wheel zoom  ·  Esc menu"), Ink, Left, Y, nullptr, UiScale); Y += 28.0f * UiScale;
    if (APolitikaPrototypePlayerController* PC = Cast<APolitikaPrototypePlayerController>(PlayerOwner))
    {
        APolitikaPrototypeUnit* Selected = Match->GetSelectedUnitForPlayer(Match->GetLocalRuntimePlayerId());
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
    if (APolitikaPrototypeObjective* Objective = Match->GetObjective())
    {
        DrawText(FString::Printf(TEXT("CONTROL POINT  /  %s"), *Objective->GetStatusText().ToString()),
            FLinearColor(0.94f, 0.78f, 0.31f), Left, Y, nullptr, 1.25f * UiScale);
        Y += 30.0f * UiScale;
    }
    DrawText(Match->GetLastCommandFeedback(), Ink, Left, Y, nullptr, UiScale);
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
