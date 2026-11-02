// Participant treasury, income and dollar-priced production (ECON-E1, first Stage 4B sub-slice).
//
// politika::economy::Ledger (PolitikaCore/Economy.h) owns the money rules. This file connects it
// to the coordinator: a request is checked for shape and freshness when it arrives and waits for
// the next simulation step, where RunEconomyPhase admits requests, pays income and advances
// production in ascending participant order. Clients only receive their own owner-only view.

#include "PolitikaPrototypeGameplay.h"

#include "PolitikaBootstrap.h"
#include "Economy.h"
#include "UnitScale.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Net/UnrealNetwork.h"
#include "UObject/ConstructorHelpers.h"

namespace pe = politika::economy;

namespace
{
constexpr double kProducerOffsetFromStartCm = 500.0;
constexpr double kGeneratorOffsetFromProducerCm = 350.0;
constexpr double kOutputClearanceCm = 100.0;
constexpr FVector::FReal kStructureHalfHeightCm = 150.0;
// A placed unit's actor origin stands this far above the ground.
constexpr FVector::FReal kUnitOriginAboveGroundCm = 42.0;
// Steps per HUD refresh of the training countdown: a quarter second at 20 Hz.
constexpr int32 kViewRemainingGranularity = 5;
// ECON-E4: where a seat's sources stand: to either side of its start, this far away, or nearer if
// that is not open ground. The first distance keeps them clear of the opening force, and two sources
// keep far enough apart that a harvester is only ever at one.
constexpr double kSourceOffsetCm = 1000.0;
constexpr double kSourceNearOffsetCm = 700.0;
constexpr double kSourceSeparationCm = 600.0;
constexpr FVector::FReal kSourceHalfHeightCm = 30.0;
// How close a harvester must be: to the source to load, to the producer to unload.
constexpr double kGoldPickupRadiusCm = 250.0;
constexpr double kGoldDepotRadiusCm = 350.0;
// The harvester's fixture figures: a placeholder, not a balance decision or an authored vehicle.
constexpr int32 kHarvesterMaxHealth = 400;
constexpr float kHarvesterSpeedCmPerSecond = 280.0f;
const FLinearColor kGoldSourceTint(0.95f, 0.72f, 0.12f);
const FLinearColor kFoodSourceTint(0.3f, 0.72f, 0.2f);
const FLinearColor kSourceEmptyTint(0.22f, 0.2f, 0.16f);
const FLinearColor kDeadBuildingTint(0.22f, 0.22f, 0.22f);
// The plant is the same faction color as the producer, darkened so the two blocks stay distinct.
constexpr float kGeneratorTintScale = 0.45f;

APolitikaPrototypeMatch* FindEconomyMatch(UWorld* World)
{
    if (!World) return nullptr;
    TActorIterator<APolitikaPrototypeMatch> It(World);
    return It ? *It : nullptr;
}

FString UsdText(int64 Cents) { return UTF8_TO_TCHAR(pe::FormatUsd(Cents).c_str()); }

FString EconomyRejectionText(pe::EconomyRejection Rejection)
{
    switch (Rejection)
    {
        case pe::EconomyRejection::InsufficientFunds: return TEXT("not enough money");
        case pe::EconomyRejection::QueueFull: return FString::Printf(TEXT("the queue is full (%d)"), static_cast<int32>(pe::kMaxQueue));
        case pe::EconomyRejection::UnitCap: return TEXT("your force is at its unit limit");
        case pe::EconomyRejection::NothingToCancel: return TEXT("nothing is in training");
        case pe::EconomyRejection::NoProducer: return TEXT("you have no training building");
        case pe::EconomyRejection::ProducerUnpowered: return TEXT("the training building has no power");
        case pe::EconomyRejection::FoodShortage: return TEXT("your force is short of food");
        case pe::EconomyRejection::FoodStoreFull: return TEXT("the food store is full");
        case pe::EconomyRejection::NothingToSell: return TEXT("no gold has reached your base");
        case pe::EconomyRejection::NoResourceSource: return TEXT("your caches are empty");
        case pe::EconomyRejection::MatchInactive: return TEXT("the match is not active");
        case pe::EconomyRejection::StaleMatch: return TEXT("that request was for an earlier match");
        case pe::EconomyRejection::RateLimited: return TEXT("too many requests at once");
        default: return UTF8_TO_TCHAR(pe::RejectionName(Rejection).data());
    }
}
}

// ----------------------------------------------------------------------------
// Fixed starting producer
// ----------------------------------------------------------------------------

APolitikaPrototypeStructure::APolitikaPrototypeStructure()
{
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube"));
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> ShapeMaterial(TEXT("/Engine/BasicShapes/BasicShapeMaterial"));
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;
    bAlwaysRelevant = false;
    Body = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Body"));
    SetRootComponent(Body);
    // A placeholder: ground clicks, selection and navigation all pass through it.
    Body->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Body->SetGenerateOverlapEvents(false);
    Body->SetCanEverAffectNavigation(false);
    if (CubeMesh.Succeeded()) Body->SetStaticMesh(CubeMesh.Object);
    if (ShapeMaterial.Succeeded()) Body->SetMaterial(0, ShapeMaterial.Object);
    Body->SetRelativeScale3D(FVector(3.0, 3.0, 3.0));
}

void APolitikaPrototypeStructure::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(APolitikaPrototypeStructure, Kind);
    DOREPLIFETIME(APolitikaPrototypeStructure, StableId);
    DOREPLIFETIME(APolitikaPrototypeStructure, BuildingId);
    DOREPLIFETIME(APolitikaPrototypeStructure, Health);
    DOREPLIFETIME(APolitikaPrototypeStructure, OwnerPlayer);
    DOREPLIFETIME(APolitikaPrototypeStructure, TeamId);
    DOREPLIFETIME(APolitikaPrototypeStructure, Tint);
    DOREPLIFETIME(APolitikaPrototypeStructure, bWithdrawn);
}

void APolitikaPrototypeStructure::ApplyPresence()
{
    if (!Body || !bWithdrawn) return;
    Body->SetHiddenInGame(true, true);
    Body->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

bool APolitikaPrototypeStructure::IsNetRelevantFor(const AActor* RealViewer, const AActor* ViewTarget, const FVector& SrcLocation) const
{
    const APolitikaPrototypeMatch* Match = AuthorityMatch.Get();
    if (!Match) return Super::IsNetRelevantFor(RealViewer, ViewTarget, SrcLocation);
    return Match->IsStructureRelevantToViewer(this, RealViewer);
}

void APolitikaPrototypeStructure::Configure(APolitikaPrototypeMatch* InMatch, politika::power::BuildingKind InKind, uint32 InOwner,
    uint8 InTeam, const FLinearColor& InTint, bool bHiddenLocally)
{
    AuthorityMatch = InMatch;
    Kind = static_cast<uint8>(InKind);
    OwnerPlayer = InOwner;
    TeamId = InTeam;
    Tint = InTint;
    ApplyTint();
    // Query only, so a click can choose the building without blocking movement or navigation.
    Body->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    Body->SetCollisionResponseToAllChannels(ECR_Ignore);
    Body->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
    // Non-replicated component state: a listen host does not see an enemy producer either.
    if (bHiddenLocally) Body->SetHiddenInGame(true, true);
}

void APolitikaPrototypeStructure::SetBuildingIdentity(uint64 InStableId, uint32 InBuildingId, int32 InHealth)
{
    StableId = InStableId;
    BuildingId = InBuildingId;
    Health = FMath::Max(0, InHealth);
}

int32 APolitikaPrototypeMatch::DamageOwnedBuilding(APolitikaPrototypeStructure* Structure, int32 Amount)
{
    if (!HasAuthority() || !Structure || !Structure->IsBuilding() || Amount <= 0) return 0;
    const pe::Ledger::BuildingDamage Hit = Economy.DamageBuilding(Structure->GetOwnerPlayer(), Structure->GetBuildingId(), Amount);
    if (const pe::Account* Account = Economy.Find(Structure->GetOwnerPlayer()))
        if (const politika::power::Building* Building = Account->grid.Find(Structure->GetBuildingId()))
            Structure->SetHealth(Building->health);
#if !UE_BUILD_SHIPPING
    UE_LOG(LogPolitikaStartup, Display,
        TEXT("RTS_BUILDING_DAMAGE participant=%u kind=%s applied=%d health=%d offline=%d producer_lost=%d refunded=%lld"),
        Structure->GetOwnerPlayer(), Structure->IsGenerator() ? TEXT("generator") : TEXT("producer"),
        Hit.applied, Structure->GetHealth(), Hit.generatorOffline ? 1 : 0, Hit.producerLost ? 1 : 0, Hit.refunded);
#endif
    return Hit.applied;
}

bool APolitikaPrototypeMatch::RepairOwnedBuilding(APolitikaPrototypeStructure* Structure)
{
    if (!HasAuthority() || !Structure || !Structure->IsGenerator()) return false;
    if (!Economy.RepairBuilding(Structure->GetOwnerPlayer(), Structure->GetBuildingId())) return false;
    if (const pe::Account* Account = Economy.Find(Structure->GetOwnerPlayer()))
        if (const politika::power::Building* Building = Account->grid.Find(Structure->GetBuildingId()))
            Structure->SetHealth(Building->health);
#if !UE_BUILD_SHIPPING
    UE_LOG(LogPolitikaStartup, Display, TEXT("RTS_BUILDING_REPAIR participant=%u kind=generator health=%d"),
        Structure->GetOwnerPlayer(), Structure->GetHealth());
#endif
    return true;
}

void APolitikaPrototypeStructure::ConfigureSource(APolitikaPrototypeMatch* InMatch, pe::Resource InResource, uint32 InOwner, uint8 InTeam, bool bHiddenLocally)
{
    AuthorityMatch = InMatch;
    Kind = InResource == pe::Resource::Gold ? GoldSourceKind : FoodSourceKind;
    OwnerPlayer = InOwner;
    TeamId = InTeam;
    Tint = InResource == pe::Resource::Gold ? kGoldSourceTint : kFoodSourceTint;
    ApplyTint();
    if (bHiddenLocally) Body->SetHiddenInGame(true, true);
}

void APolitikaPrototypeStructure::MarkSourceEmpty()
{
    if (Tint == kSourceEmptyTint) return;
    Tint = kSourceEmptyTint;
    ApplyTint();
}

void APolitikaPrototypeStructure::BeginPlay()
{
    Super::BeginPlay();
    if (HasAuthority()) return;
    ApplyTint();
#if !UE_BUILD_SHIPPING
    if (IsSource())
    {
        UE_LOG(LogPolitikaStartup, Display, TEXT("%s owner=%u team=%u x=%.1f y=%.1f"),
            IsGoldSource() ? TEXT("RTS_GOLD_SOURCE_RELEVANT") : TEXT("RTS_FOOD_SOURCE_RELEVANT"),
            OwnerPlayer, static_cast<uint32>(TeamId), GetActorLocation().X, GetActorLocation().Y);
        return;
    }
    UE_LOG(LogPolitikaStartup, Display, TEXT("RTS_STRUCTURE_RELEVANT kind=%s owner=%u team=%u x=%.1f y=%.1f"),
        IsProducer() ? TEXT("producer") : TEXT("generator"), OwnerPlayer, static_cast<uint32>(TeamId), GetActorLocation().X, GetActorLocation().Y);
#endif
}

void APolitikaPrototypeStructure::ApplyTint()
{
    if (GetNetMode() == NM_DedicatedServer) return;
    // The generator is the narrower of the two placeholder blocks, at the same height. A source is
    // a low slab, so a unit standing at it stays visible.
    if (IsSource()) Body->SetRelativeScale3D(FVector(1.5, 1.5, kSourceHalfHeightCm / 50.0));
    else if (!IsProducer()) Body->SetRelativeScale3D(FVector(2.0, 2.0, 3.0));
    if (UMaterialInstanceDynamic* Material = Body->CreateAndSetMaterialInstanceDynamic(0))
        Material->SetVectorParameterValue(TEXT("Color"), GetPresentedTint());
}

FLinearColor APolitikaPrototypeStructure::GetPresentedTint() const
{
    return IsBuilding() && Health <= 0 ? kDeadBuildingTint : Tint;
}

bool APolitikaPrototypeMatch::IsStructureRelevantToViewer(const APolitikaPrototypeStructure* Structure, const AActor* RealViewer) const
{
    if (!bFogOfWar) return true;
    // Unbound connections receive no structure. Everyone else gets what that participant can see.
    const uint32 ParticipantId = GetBoundParticipantId(Cast<APlayerController>(RealViewer));
    return ParticipantId != 0 && IsStructureObservedByParticipant(Structure, ParticipantId);
}

void APolitikaPrototypeStructure::Adopt(uint32 InOwner, uint8 InTeam, uint32 InBuildingId, int32 InHealth, const FLinearColor& InTint)
{
    OwnerPlayer = InOwner;
    TeamId = InTeam;
    BuildingId = InBuildingId;
    Health = FMath::Max(0, InHealth);
    Tint = InTint;
    ApplyTint();
}

FLinearColor APolitikaPrototypeMatch::BuildingTintFor(uint32 ParticipantId, bool bGenerator) const
{
    FLinearColor Tint = FLinearColor::White;
    const FPolitikaPrototypeRuntimeParticipant* Participant = RuntimeParticipants.FindByPredicate(
        [ParticipantId](const FPolitikaPrototypeRuntimeParticipant& Candidate) { return Candidate.RuntimePlayerId == ParticipantId; });
    if (Participant && MapDefinition && ContentCatalog)
    {
        const FPolitikaPrototypeUnitPlacement* Template = MapDefinition->UnitPlacements.FindByPredicate(
            [Participant](const FPolitikaPrototypeUnitPlacement& Candidate) { return Candidate.SlotId == Participant->SlotId; });
        if (const UPolitikaUnitArchetypeDefinition* Definition = Template ? ContentCatalog->FindUnit(Template->UnitArchetypeId) : nullptr)
            Tint = Definition->FactionTint;
    }
    return bGenerator ? Tint * kGeneratorTintScale : Tint;
}

void APolitikaPrototypeStructure::SetLocalFogHidden(bool bHidden)
{
    if (bLocalFogApplied && bLocalFogHidden == bHidden) return;
    const bool bChanged = bLocalFogApplied && bLocalFogHidden != bHidden;
    bLocalFogApplied = true;
    bLocalFogHidden = bHidden;
    // Component state only. Actor bHidden would replicate and hide the mesh for every connection.
    // A withdrawn producer stays gone even when its owner would otherwise draw it.
    if (Body) Body->SetHiddenInGame(bWithdrawn || bHidden, true);
#if !UE_BUILD_SHIPPING
    if (bChanged)
        UE_LOG(LogPolitikaStartup, Display, TEXT("MP_HOST_STRUCTURE_FOG entity=%llu owner=%u team=%u hidden=%d x=%.1f y=%.1f"),
            StableId, OwnerPlayer, static_cast<uint32>(TeamId), bHidden ? 1 : 0, GetActorLocation().X, GetActorLocation().Y);
#endif
}

bool APolitikaPrototypeMatch::BeginCapture(APolitikaPrototypeUnit* Unit, APolitikaPrototypeStructure* Structure)
{
    if (!HasAuthority() || !Unit || !Unit->IsAlive() || !Unit->CanAttack() || !Structure || !Structure->IsBuilding() || !Structure->IsAlive())
        return false;
    if (Structure->GetOwnerPlayer() == 0 || Structure->GetOwnerPlayer() == Unit->GetOwnerPlayer()) return false;
    if (IsFogOfWarEnabled() && !IsStructureObservedByParticipant(Structure, Unit->GetOwnerPlayer())) return false;
    if (Structure->IsProducer())
    {
        const pe::Account* Winner = Economy.Find(Unit->GetOwnerPlayer());
        const politika::power::Building* Held = Winner ? Winner->grid.Find(pe::kProducerBuilding) : nullptr;
        if (Winner && Winner->hasProducer && Held && Held->health > 0) return false;
    }
    Unit->StopOrder();
    Unit->HoldForCapture(Structure);
#if !UE_BUILD_SHIPPING
    const float Distance = FVector::Dist2D(Unit->GetActorLocation(), Structure->GetActorLocation());
    UE_LOG(LogPolitikaStartup, Display,
        TEXT("RTS_BUILDING_CAPTURE phase=%s unit=%llu structure=%llu distance=%.1f"),
        Distance <= politika::power::kCaptureRangeCm ? TEXT("hold") : TEXT("approach"),
        Unit->GetStableId(), Structure->GetStableId(), Distance);
#endif
    return true;
}

bool APolitikaPrototypeMatch::SupplyCaptureApproachRoute(const APolitikaPrototypeUnit& Unit, const FVector& Destination, TArray<FVector>& OutRoute) const
{
#if WITH_DEV_AUTOMATION_TESTS
    return CaptureRouteForTest && CaptureRouteForTest(Unit, Destination, OutRoute);
#else
    (void)Unit;
    (void)Destination;
    (void)OutRoute;
    return false;
#endif
}

void APolitikaPrototypeMatch::AdvanceCaptures()
{
    if (!HasAuthority()) return;
    for (APolitikaPrototypeUnit* Unit : Units)
    {
        if (!IsValid(Unit)) continue;
        APolitikaPrototypeStructure* Structure = Unit->GetCaptureTarget();
        if (!Structure) continue;
        const bool bCanHold = Unit->IsAlive() && Structure->IsBuilding() && Structure->IsAlive()
            && Structure->GetOwnerPlayer() != Unit->GetOwnerPlayer()
            && (!IsFogOfWarEnabled() || IsStructureObservedByParticipant(Structure, Unit->GetOwnerPlayer()));
        const bool bInRange = FVector::Dist2D(Unit->GetActorLocation(), Structure->GetActorLocation()) <= politika::power::kCaptureRangeCm;
        // Out of range with no steps yet is the walk in. Leaving after the hold has started cancels it.
        if (!bCanHold || (Unit->GetCaptureSteps() > 0 && !bInRange))
        {
            Unit->ClearCapture();
            continue;
        }
        if (!bInRange) continue;
        const int32 Steps = Unit->GetCaptureSteps() + 1;
        Unit->SetCaptureSteps(Steps);
        if (Steps < politika::power::kCaptureSteps) continue;
        const uint32 PreviousOwner = Structure->GetOwnerPlayer();
        const pe::Ledger::CaptureResult Result = Economy.CaptureBuilding(PreviousOwner, Unit->GetOwnerPlayer(), Structure->GetBuildingId());
        if (Result.captured)
            Structure->Adopt(Unit->GetOwnerPlayer(), GetTeamForParticipant(Unit->GetOwnerPlayer()), Result.newId, Structure->GetHealth(),
                BuildingTintFor(Unit->GetOwnerPlayer(), Structure->IsGenerator()));
#if !UE_BUILD_SHIPPING
        UE_LOG(LogPolitikaStartup, Display,
            TEXT("RTS_BUILDING_CAPTURE unit=%llu from=%u to=%u kind=%s captured=%d refused=%d refunded=%lld new_id=%u"),
            Unit->GetStableId(), PreviousOwner, Unit->GetOwnerPlayer(), Structure->IsProducer() ? TEXT("producer") : TEXT("generator"),
            Result.captured ? 1 : 0, Result.refused ? 1 : 0, Result.refunded, Result.newId);
#endif
        Unit->ClearCapture();
    }
}

FString APolitikaPrototypeMatch::BuildingCursorText(const APolitikaPrototypeStructure* Structure, uint32 ViewerId) const
{
    if (!Structure || !Structure->IsBuilding() || ViewerId == 0) return FString();
    if (IsFogOfWarEnabled() && !IsStructureObservedByParticipant(Structure, ViewerId)) return FString();
    const bool bOurs = Structure->GetOwnerPlayer() == ViewerId;
    const uint8 ViewerTeam = GetTeamForParticipant(ViewerId);
    const bool bAllied = !bOurs && ViewerTeam != politika::rules::kNoTeam && ViewerTeam == Structure->GetTeamId();
    const TCHAR* Who = bOurs ? TEXT("Your") : bAllied ? TEXT("Allied") : TEXT("Enemy");
    const TCHAR* Kind = Structure->IsProducer() ? TEXT("producer") : TEXT("generator");
    // A producer at zero has left the map. A generator husk is still there and can be repaired.
    if (!Structure->IsAlive() && !Structure->IsGenerator()) return FString::Printf(TEXT("%s %s  destroyed"), Who, Kind);
    const int32 MaxHealth = Structure->IsProducer() ? politika::power::kProducerHealth : politika::power::kGeneratorHealth;
    FString State;
    if (Structure->IsGenerator())
    {
        const pe::Account* Account = Economy.Find(Structure->GetOwnerPlayer());
        const politika::power::Building* Plant = Account ? Account->grid.Find(Structure->GetBuildingId()) : nullptr;
        const bool bOnline = Plant && Plant->generating;
        State = bOnline ? TEXT("  \u00b7  online") : TEXT("  \u00b7  offline");
        if (bOurs && !bOnline) State += TEXT("  \u00b7  click to repair");
    }
    const APolitikaPrototypeUnit* Captor = nullptr;
    for (const APolitikaPrototypeUnit* Unit : Units)
    {
        if (!IsValid(Unit) || Unit->GetCaptureTarget() != Structure) continue;
        Captor = Unit;
        if (Unit->GetOwnerPlayer() == ViewerId) break;
    }
    if (Captor)
    {
        const bool bInRange = FVector::Dist2D(Captor->GetActorLocation(), Structure->GetActorLocation()) <= politika::power::kCaptureRangeCm;
        if (!bInRange && Captor->GetCaptureSteps() == 0)
            State += TEXT("  \u00b7  moving to capture");
        else
            State += FString::Printf(TEXT("  \u00b7  capturing %d / %d"), Captor->GetCaptureSteps(), politika::power::kCaptureSteps);
    }
    return FString::Printf(TEXT("%s %s  %d / %d%s"), Who, Kind, Structure->GetHealth(), MaxHealth, *State);
}

bool APolitikaPrototypeMatch::DemolishOwnedBuilding(APolitikaPrototypeStructure* Structure, uint32 ParticipantId)
{
    if (!HasAuthority() || !Structure || !Structure->IsBuilding() || !Structure->IsAlive()) return false;
    if (ParticipantId == 0 || Structure->GetOwnerPlayer() != ParticipantId) return false;
    const pe::Account* Before = Economy.Find(ParticipantId);
    const pe::Cents RefundedBefore = Before ? Before->refunded : 0;
    if (DamageOwnedBuilding(Structure, Structure->GetHealth()) <= 0) return false;
#if !UE_BUILD_SHIPPING
    const pe::Account* After = Economy.Find(ParticipantId);
    UE_LOG(LogPolitikaStartup, Display,
        TEXT("RTS_BUILDING_DEMOLISH participant=%u kind=%s health=%d refunded=%lld"),
        ParticipantId, Structure->IsProducer() ? TEXT("producer") : TEXT("generator"), Structure->GetHealth(),
        (After ? After->refunded : RefundedBefore) - RefundedBefore);
#endif
    return true;
}

// ----------------------------------------------------------------------------
// Setup and teardown
// ----------------------------------------------------------------------------

void APolitikaPrototypeMatch::InitializeEconomy()
{
    if (!HasAuthority()) return;
    Economy.Clear();
    PendingEconomyRequests.Reset();
#if !UE_BUILD_SHIPPING
    // Development fixture "offset:duration" in steps; see ApplyPowerOutageFixture.
    OutageOffsetSteps = -1;
    OutageDurationSteps = 0;
    OutageParticipant = 0;
    OutageBeginStep = 0;
    FString OutageFixture;
    FString OutageOffsetText;
    FString OutageDurationText;
    if (FParse::Value(FCommandLine::Get(), TEXT("PolitikaMPPowerOutage="), OutageFixture)
        && OutageFixture.Split(TEXT(":"), &OutageOffsetText, &OutageDurationText)
        && OutageOffsetText.IsNumeric() && OutageDurationText.IsNumeric())
    {
        OutageOffsetSteps = FCString::Atoi(*OutageOffsetText);
        OutageDurationSteps = FCString::Atoi(*OutageDurationText);
    }
#endif
    for (APolitikaPrototypeStructure* Structure : Structures)
        if (IsValid(Structure)) Structure->Destroy();
    Structures.Reset();
    if (!MapDefinition || !ContentCatalog) return;

    pe::Cents StartingCents = pe::kStartingCents;
#if !UE_BUILD_SHIPPING
    // Development fixture for packaged queue and unit-cap checks; never in a Shipping build.
    int64 FixtureCents = 0;
    if (FParse::Value(FCommandLine::Get(), TEXT("PolitikaMPEconomyStartCents="), FixtureCents) && FixtureCents >= 0)
        StartingCents = FixtureCents;
#endif
#if !UE_BUILD_SHIPPING
    // Development fixture "participant:rations" for packaged shortage checks; never in a Shipping build.
    uint32 FixtureFoodParticipant = 0;
    int64 FixtureFoodRations = -1;
    FString FoodFixture;
    FString FoodParticipantText;
    FString FoodRationsText;
    if (FParse::Value(FCommandLine::Get(), TEXT("PolitikaMPEconomyStartFood="), FoodFixture)
        && FoodFixture.Split(TEXT(":"), &FoodParticipantText, &FoodRationsText)
        && FoodParticipantText.IsNumeric() && FoodRationsText.IsNumeric())
    {
        FixtureFoodParticipant = static_cast<uint32>(FCString::Atoi(*FoodParticipantText));
        FixtureFoodRations = FCString::Atoi64(*FoodRationsText);
    }
    // Development fixtures "participant:amount" for packaged checks of an emptied source.
    const auto ParseSourceFixture = [](const TCHAR* Switch, uint32& OutParticipant, int64& OutAmount)
    {
        FString Fixture;
        FString ParticipantText;
        FString AmountText;
        if (FParse::Value(FCommandLine::Get(), Switch, Fixture) && Fixture.Split(TEXT(":"), &ParticipantText, &AmountText)
            && ParticipantText.IsNumeric() && AmountText.IsNumeric())
        {
            OutParticipant = static_cast<uint32>(FCString::Atoi(*ParticipantText));
            OutAmount = FCString::Atoi64(*AmountText);
        }
    };
    uint32 FixtureGoldParticipant = 0;
    int64 FixtureGold = -1;
    ParseSourceFixture(TEXT("PolitikaMPEconomySourceGold="), FixtureGoldParticipant, FixtureGold);
    uint32 FixtureFoodSourceParticipant = 0;
    int64 FixtureFoodSource = -1;
    ParseSourceFixture(TEXT("PolitikaMPEconomySourceFood="), FixtureFoodSourceParticipant, FixtureFoodSource);
#endif
    // Each seat with a base starts with one harvester. Regression sessions that pin exact forces turn
    // it off with -PolitikaMPStartingHarvesters=0, and Automation fixtures opt in.
    bool bStartingHarvesters = true;
#if !UE_BUILD_SHIPPING
    int32 StartingHarvesterSwitch = 1;
    if (FParse::Value(FCommandLine::Get(), TEXT("PolitikaMPStartingHarvesters="), StartingHarvesterSwitch) && StartingHarvesterSwitch == 0)
        bStartingHarvesters = false;
    // A scripted acceptance run and the navigation fixtures pin their own exact forces and timing.
    FString ScriptedMode;
    if ((FParse::Value(FCommandLine::Get(), TEXT("RTSAcceptance="), ScriptedMode) && ScriptedMode != TEXT("multiplayer"))
        || MapDefinition->StableId == TEXT("map.test.projection_displacement") || MapDefinition->StableId == TEXT("map.test.static_obstacle_detour"))
        bStartingHarvesters = false;
#endif
#if WITH_DEV_AUTOMATION_TESTS
    if (GIsAutomationTesting) bStartingHarvesters = StartingHarvestersForTest;
#endif
    const FVector2D Centre = (PlayableMin + PlayableMax) * 0.5;
    const uint8 LocalTeam = LocalRuntimePlayerId != 0 ? GetTeamForParticipant(LocalRuntimePlayerId) : politika::rules::kNoTeam;
    for (const FPolitikaPrototypeRuntimeParticipant& Participant : RuntimeParticipants)
    {
        const FPolitikaPrototypePlayerSlot* Slot = MapDefinition->PlayerSlots.FindByPredicate(
            [&Participant](const FPolitikaPrototypePlayerSlot& Candidate) { return Candidate.SlotId == Participant.SlotId; });
        // The producer trains the unit this seat's scenario starts with: no new content.
        const FPolitikaPrototypeUnitPlacement* Template = MapDefinition->UnitPlacements.FindByPredicate(
            [&Participant](const FPolitikaPrototypeUnitPlacement& Candidate) { return Candidate.SlotId == Participant.SlotId; });
        const UPolitikaUnitArchetypeDefinition* Definition = Template ? ContentCatalog->FindUnit(Template->UnitArchetypeId) : nullptr;
        APolitikaPrototypeStructure* Structure = nullptr;
        if (Slot && Definition)
        {
            // Behind the start as seen from the middle of the map, inside the playable area.
            FVector2D Away = FVector2D(Slot->StartLocation.X, Slot->StartLocation.Y) - Centre;
            Away = Away.IsNearlyZero() ? FVector2D(0.0, -1.0) : Away.GetSafeNormal();
            const FVector2D Point = FVector2D(Slot->StartLocation.X, Slot->StartLocation.Y) + Away * kProducerOffsetFromStartCm;
            const FVector Location(FMath::Clamp(Point.X, PlayableMin.X, PlayableMax.X),
                FMath::Clamp(Point.Y, PlayableMin.Y, PlayableMax.Y), Template->Location.Z - kUnitOriginAboveGroundCm + kStructureHalfHeightCm);
            Structure = GetWorld()->SpawnActor<APolitikaPrototypeStructure>(Location, FRotator::ZeroRotator);
            if (Structure)
            {
                const bool bHiddenLocally = bFogOfWar && LocalTeam != politika::rules::kNoTeam && LocalTeam != Participant.TeamId;
                Structure->Configure(this, politika::power::BuildingKind::Producer, Participant.RuntimePlayerId, Participant.TeamId,
                    Definition->FactionTint, bHiddenLocally);
                Structure->SetBuildingIdentity(NextStableId++, politika::economy::kProducerBuilding, politika::power::kProducerHealth);
                Structures.Add(Structure);
                // Beside the producer: the seat's self-starting plant. Without one there is no
                // base, and the seat has no producer either, so no impossible power cycle exists.
                const FVector2D Side(-Away.Y, Away.X);
                const FVector2D GeneratorPoint = FVector2D(Location.X, Location.Y) + Side * kGeneratorOffsetFromProducerCm;
                const FVector GeneratorLocation(FMath::Clamp(GeneratorPoint.X, PlayableMin.X, PlayableMax.X),
                    FMath::Clamp(GeneratorPoint.Y, PlayableMin.Y, PlayableMax.Y), Location.Z);
                APolitikaPrototypeStructure* Generator = GetWorld()->SpawnActor<APolitikaPrototypeStructure>(GeneratorLocation, FRotator::ZeroRotator);
                if (Generator)
                {
                    Generator->Configure(this, politika::power::BuildingKind::Generator, Participant.RuntimePlayerId, Participant.TeamId,
                        Definition->FactionTint * kGeneratorTintScale, bHiddenLocally);
                    Generator->SetBuildingIdentity(NextStableId++, politika::economy::kGeneratorBuilding, politika::power::kGeneratorHealth);
                    Structures.Add(Generator);
                }
                else
                {
                    Structures.Remove(Structure);
                    Structure->Destroy();
                    Structure = nullptr;
                }
            }
        }
        Economy.AddParticipant(Participant.RuntimePlayerId, StartingCents, Structure != nullptr, pe::kIncomeIntervalSteps);
        // A source needs a base to deliver to.
        if (Structure)
        {
            const FVector2D Start(Slot->StartLocation.X, Slot->StartLocation.Y);
            FVector2D Away = Start - Centre;
            Away = Away.IsNearlyZero() ? FVector2D(0.0, -1.0) : Away.GetSafeNormal();
            PlaceSources(Participant.RuntimePlayerId, Participant.TeamId, Start, FVector2D(-Away.Y, Away.X), Template->Location.Z,
                bFogOfWar && LocalTeam != politika::rules::kNoTeam && LocalTeam != Participant.TeamId);
        }
#if !UE_BUILD_SHIPPING
        if (Participant.RuntimePlayerId == FixtureGoldParticipant && Economy.Find(Participant.RuntimePlayerId)->goldSource.present)
            Economy.SetSource(Participant.RuntimePlayerId, pe::Resource::Gold, FixtureGold);
        if (Participant.RuntimePlayerId == FixtureFoodSourceParticipant && Economy.Find(Participant.RuntimePlayerId)->foodSource.present)
            Economy.SetSource(Participant.RuntimePlayerId, pe::Resource::Food, FixtureFoodSource);
        if (const pe::Account* Account = Economy.Find(Participant.RuntimePlayerId))
        {
            const APolitikaPrototypeStructure* GoldActor = FindSourceOf(Participant.RuntimePlayerId, pe::Resource::Gold);
            const APolitikaPrototypeStructure* FoodActor = FindSourceOf(Participant.RuntimePlayerId, pe::Resource::Food);
            UE_LOG(LogPolitikaStartup, Display, TEXT("RTS_GOLD_START participant=%u source=%lld has_source=%d x=%.1f y=%.1f generation=%llu"),
                Participant.RuntimePlayerId, Account->goldSource.remaining, Account->goldSource.present ? 1 : 0,
                GoldActor ? GoldActor->GetActorLocation().X : 0.0, GoldActor ? GoldActor->GetActorLocation().Y : 0.0, MatchGeneration);
            UE_LOG(LogPolitikaStartup, Display, TEXT("RTS_FOOD_SOURCE_START participant=%u source=%lld has_source=%d x=%.1f y=%.1f generation=%llu"),
                Participant.RuntimePlayerId, Account->foodSource.remaining, Account->foodSource.present ? 1 : 0,
                FoodActor ? FoodActor->GetActorLocation().X : 0.0, FoodActor ? FoodActor->GetActorLocation().Y : 0.0, MatchGeneration);
        }
        if (Participant.RuntimePlayerId == FixtureFoodParticipant) Economy.SetStartingRations(Participant.RuntimePlayerId, FixtureFoodRations);
        UE_LOG(LogPolitikaStartup, Display, TEXT("RTS_ECONOMY_START participant=%u available=%lld producer=%d x=%.1f y=%.1f generation=%llu"),
            Participant.RuntimePlayerId, StartingCents, Structure ? 1 : 0, Structure ? Structure->GetActorLocation().X : 0.0,
            Structure ? Structure->GetActorLocation().Y : 0.0, MatchGeneration);
        if (const pe::Account* Account = Economy.Find(Participant.RuntimePlayerId))
            UE_LOG(LogPolitikaStartup, Display,
                TEXT("RTS_POWER_START participant=%u supplied_kw=%lld required_kw=%lld generator_load_kw=%d producer_load_kw=%d producer_powered=%d generation=%llu"),
                Participant.RuntimePlayerId, Account->allocation.suppliedKw, Account->allocation.requiredKw,
                Structure ? politika::power::kGeneratorAuxiliaryKw : 0, Structure ? politika::power::kProducerLoadKw : 0,
                Account->producerPowered ? 1 : 0, MatchGeneration);
        if (const pe::Account* Account = Economy.Find(Participant.RuntimePlayerId))
            UE_LOG(LogPolitikaStartup, Display, TEXT("RTS_FOOD_START participant=%u rations=%lld generation=%llu"),
                Participant.RuntimePlayerId, Account->rations, MatchGeneration);
#endif
        if (Structure && bStartingHarvesters) SpawnStartingHarvester(Participant.RuntimePlayerId);
    }
}

void APolitikaPrototypeMatch::PlaceSources(uint32 ParticipantId, uint8 InTeamId, const FVector2D& Start, const FVector2D& Side,
    float GroundZ, bool bHiddenLocally)
{
    // A fixed order of candidates, so equal maps place equal sources: gold takes the first open place
    // and food the next one far enough from it. A seat with no open ground for a source goes without
    // it, and its HUD says nothing about that resource.
    TArray<FVector2D, TInlineAllocator<2>> Placed;
    for (const pe::Resource Resource : {pe::Resource::Gold, pe::Resource::Food})
    {
        bool bFound = false;
        for (const double Distance : {kSourceOffsetCm, kSourceNearOffsetCm})
        {
            for (const double Sign : {1.0, -1.0})
            {
                if (bFound) continue;
                const FVector2D Point = Start + Side * Sign * Distance;
                if (Placed.ContainsByPredicate([&Point](const FVector2D& Other) { return FVector2D::Distance(Other, Point) < kSourceSeparationCm; })) continue;
                const FVector2D MapPosition = Point - PlayableMin;
                bool bOpen = false;
#if WITH_DEV_AUTOMATION_TESTS
                if (FormationSlotProbeForTest) bOpen = FormationSlotProbeForTest(MapPosition);
                else
#endif
                bOpen = IsGroundSlotOpen(GetWorld(), PlayableMin, PlayableMax, MapPosition, GroundZ);
                if (!bOpen) continue;
                bFound = true;
                const FVector Location(Point.X, Point.Y, GroundZ - kUnitOriginAboveGroundCm + kSourceHalfHeightCm);
                APolitikaPrototypeStructure* Source = GetWorld()->SpawnActor<APolitikaPrototypeStructure>(Location, FRotator::ZeroRotator);
                if (!Source) continue;
                Source->ConfigureSource(this, Resource, ParticipantId, InTeamId, bHiddenLocally);
                // An id so an attack order can find it and refuse it. It is still not a building.
                Source->SetBuildingIdentity(NextStableId++, 0, 0);
                Structures.Add(Source);
                Placed.Add(Point);
                Economy.SetSource(ParticipantId, Resource, Resource == pe::Resource::Gold ? pe::kSourceGold : pe::kSourceFood);
            }
        }
    }
}

void APolitikaPrototypeMatch::ClearEconomy(const TCHAR* Reason)
{
    if (!HasAuthority()) return;
    TArray<FPendingEconomyRequest> Waiting = MoveTemp(PendingEconomyRequests);
    PendingEconomyRequests.Reset();
    for (const FPendingEconomyRequest& Request : Waiting)
        if (APlayerController* Source = Request.Source.Get())
            RejectEconomyIntent(Source, Request.Intent, Request.ParticipantId, pe::EconomyRejection::MatchInactive);
#if !UE_BUILD_SHIPPING
    for (const pe::Account& Account : Economy.All())
        UE_LOG(LogPolitikaStartup, Display,
            TEXT("RTS_ECONOMY_CLEARED participant=%u reason=%s available=%lld credited=%lld spent=%lld refunded=%lld dropped_queue=%d produced=%u reconciles=%d generation=%llu step=%llu sale_credited=%lld gold_held=%lld gold_reconciles=%d"),
            Account.participant, Reason, Account.available, Account.credited, Account.spent, Account.refunded,
            static_cast<int32>(Account.queue.size()), Account.produced, Account.Reconciles() ? 1 : 0, MatchGeneration, SimulationStepsExecuted,
            Account.saleCredited, Account.goldHeld, Account.GoldReconciles() ? 1 : 0);
#else
    (void)Reason;
#endif
    // Money, queues and loads belong to the active match; the next generation starts from its own start.
    Economy.Clear();
    HarvesterOrderSequences.Reset();
    for (APolitikaPrototypeUnit* Unit : Units)
        if (IsValid(Unit)) Unit->SetCarriedResource(std::nullopt);
}

// ----------------------------------------------------------------------------
// Request intake
// ----------------------------------------------------------------------------

void APolitikaPrototypeMatch::SendEconomyOutcome(APlayerController* Source, const FPolitikaNetEconomyIntent& Intent,
    pe::EconomyRejection Rejection, int64 AmountCents, int64 AvailableCents, int32 Queued, int32 GoldSold)
{
    APolitikaPrototypePlayerController* Controller = Cast<APolitikaPrototypePlayerController>(Source);
    if (!Controller) return;
    FPolitikaNetEconomyOutcome Outcome;
    Outcome.CorrelationId = Intent.CorrelationId;
    Outcome.ClientSequence = Intent.ClientSequence;
    Outcome.Op = Intent.Op;
    Outcome.Rejection = static_cast<uint8>(Rejection);
    Outcome.bAccepted = Rejection == pe::EconomyRejection::None;
    Outcome.AmountCents = AmountCents;
    Outcome.GoldSold = GoldSold;
    Outcome.AvailableCents = AvailableCents;
    Outcome.Queued = Queued;
    Outcome.MatchGeneration = MatchGeneration;
#if WITH_DEV_AUTOMATION_TESTS
    TestEconomyOutcomes.Emplace(Source, Outcome);
#endif
    if (Controller->IsLocalSeatController()) Controller->ClientReceiveEconomyOutcome_Implementation(Outcome);
    else Controller->ClientReceiveEconomyOutcome(Outcome);
}

void APolitikaPrototypeMatch::RejectEconomyIntent(APlayerController* Source, const FPolitikaNetEconomyIntent& Intent,
    uint32 ParticipantId, pe::EconomyRejection Rejection)
{
#if !UE_BUILD_SHIPPING
    UE_LOG(LogPolitikaStartup, Display,
        TEXT("RTS_ECONOMY_REJECT participant=%u op=%u recipe=%u correlation=%u client_sequence=%u rejection=%s generation=%llu step=%llu"),
        ParticipantId, static_cast<uint32>(Intent.Op), static_cast<uint32>(Intent.Recipe), Intent.CorrelationId, Intent.ClientSequence,
        UTF8_TO_TCHAR(pe::RejectionName(Rejection).data()), MatchGeneration, SimulationStepsExecuted);
#endif
    const pe::Account* Account = Economy.Find(ParticipantId);
    SendEconomyOutcome(Source, Intent, Rejection, 0, Account ? Account->available : 0,
        Account ? static_cast<int32>(Account->queue.size()) : 0);
}

bool APolitikaPrototypeMatch::SubmitEconomyIntent(APlayerController* Source, const FPolitikaNetEconomyIntent& Intent)
{
    using pe::EconomyRejection;
    if (!HasAuthority()) return false;
    FParticipantBinding* Binding = ResolveCommandBinding(Source);
    if (!Binding)
    {
        UE_LOG(LogPolitikaStartup, Display, TEXT("MP_ECONOMY_UNBOUND controller=%s correlation=%u client_sequence=%u"),
            Source ? *Source->GetName() : TEXT("none"), Intent.CorrelationId, Intent.ClientSequence);
        if (const APolitikaPrototypePlayerController* Prototype = Cast<APolitikaPrototypePlayerController>(Source);
            Prototype && !Prototype->IsLocalSeatController())
            SendEconomyOutcome(Source, Intent, EconomyRejection::Unauthorized, 0, 0, 0);
        return false;
    }
    const uint32 ParticipantId = Binding->RuntimePlayerId;
    if (Intent.ProtocolVersion != static_cast<int32>(FPolitikaBootstrap::Get().NetworkProtocol()))
    {
        RejectEconomyIntent(Source, Intent, ParticipantId, EconomyRejection::MalformedCommand);
        return true;
    }
    if (const politika::rules::OrderResult Fresh = Binding->Sequence.Consume(Intent.ClientSequence); !Fresh)
    {
        RejectEconomyIntent(Source, Intent, ParticipantId,
            Fresh.rejection == politika::rules::OrderRejection::DuplicateCommand
                ? EconomyRejection::DuplicateCommand : EconomyRejection::MalformedCommand);
        return true;
    }
    if (const EconomyRejection Shape = pe::CheckIntentShape(Intent.Op, Intent.Recipe); Shape != EconomyRejection::None)
    {
        RejectEconomyIntent(Source, Intent, ParticipantId, Shape);
        return true;
    }
    if (!bContentReady || !Lifecycle.IsActive())
    {
        RejectEconomyIntent(Source, Intent, ParticipantId, EconomyRejection::MatchInactive);
        return true;
    }
    if (Intent.MatchGeneration != MatchGeneration)
    {
        RejectEconomyIntent(Source, Intent, ParticipantId, EconomyRejection::StaleMatch);
        return true;
    }
    int32 Waiting = 0;
    for (const FPendingEconomyRequest& Request : PendingEconomyRequests)
        Waiting += Request.ParticipantId == ParticipantId ? 1 : 0;
    if (Waiting >= static_cast<int32>(pe::kMaxPendingRequests))
    {
        RejectEconomyIntent(Source, Intent, ParticipantId, EconomyRejection::RateLimited);
        return true;
    }
    PendingEconomyRequests.Add({Source, ParticipantId, Intent});
    return true;
}

// ----------------------------------------------------------------------------
// Simulation phase
// ----------------------------------------------------------------------------

int32 APolitikaPrototypeMatch::CountLivingUnits(uint32 ParticipantId) const
{
    int32 Living = 0;
    for (const APolitikaPrototypeUnit* Unit : Units)
        Living += IsValid(Unit) && Unit->IsAlive() && Unit->GetOwnerPlayer() == ParticipantId ? 1 : 0;
    return Living;
}

const APolitikaPrototypeStructure* APolitikaPrototypeMatch::FindSourceOf(uint32 ParticipantId, pe::Resource Resource) const
{
    for (const APolitikaPrototypeStructure* Structure : Structures)
        if (IsValid(Structure) && Structure->GetOwnerPlayer() == ParticipantId
            && (Resource == pe::Resource::Gold ? Structure->IsGoldSource() : Structure->IsFoodSource()))
            return Structure;
    return nullptr;
}

void APolitikaPrototypeMatch::AssignNetOwner(APolitikaPrototypeUnit* Unit) const
{
    // A unit that appears after its participant bound would otherwise never have a network owner, and
    // its owner-only fields would not reach that participant's client. RefreshUnitNetOwnership covers
    // the units that exist at binding.
    for (const FParticipantBinding& Binding : ParticipantBindings)
        if (Binding.RuntimePlayerId == Unit->GetOwnerPlayer() && Binding.Controller.IsValid())
            Unit->SetOwner(Binding.Controller.Get());
}

void APolitikaPrototypeMatch::SpawnStartingHarvester(uint32 ParticipantId)
{
    FVector Location = FVector::ZeroVector;
    FRotator Rotation = FRotator::ZeroRotator;
    APolitikaPrototypeUnit* Unit = FindProducerOutput(ParticipantId, Location, Rotation)
        ? SpawnHarvester(ParticipantId, Location, Rotation) : nullptr;
    if (!Unit || !Economy.AddHarvester(ParticipantId, Unit->GetStableId())) return;
#if !UE_BUILD_SHIPPING
    UE_LOG(LogPolitikaStartup, Display, TEXT("RTS_HARVESTER_START participant=%u entity=%llu x=%.1f y=%.1f generation=%llu"),
        ParticipantId, Unit->GetStableId(), Location.X, Location.Y, MatchGeneration);
#endif
}

const APolitikaPrototypeStructure* APolitikaPrototypeMatch::FindStructureOf(uint32 ParticipantId) const
{
    // The withdrawn husk stays in the list so the ledger can remember it. Delivery and new
    // units use the living producer, including one taken from the other side.
    for (const APolitikaPrototypeStructure* Structure : Structures)
        if (IsValid(Structure) && Structure->IsProducer() && Structure->IsAlive() && !Structure->IsWithdrawn()
            && Structure->GetOwnerPlayer() == ParticipantId)
            return Structure;
    return nullptr;
}

bool APolitikaPrototypeMatch::FindProducerOutput(uint32 ParticipantId, FVector& OutLocation, FRotator& OutRotation) const
{
    const APolitikaPrototypeStructure* Structure = FindStructureOf(ParticipantId);
    const FPolitikaPrototypeRuntimeParticipant* Seat = RuntimeParticipants.FindByPredicate(
        [ParticipantId](const FPolitikaPrototypeRuntimeParticipant& Candidate) { return Candidate.RuntimePlayerId == ParticipantId; });
    const FPolitikaPrototypeUnitPlacement* Template = Seat ? MapDefinition->UnitPlacements.FindByPredicate(
        [Seat](const FPolitikaPrototypeUnitPlacement& Candidate) { return Candidate.SlotId == Seat->SlotId; }) : nullptr;
    if (!Structure || !Template) return false;
    const FVector Origin = Structure->GetActorLocation();
    const FVector2D Centre = (PlayableMin + PlayableMax) * 0.5;
    FVector2D Towards = Centre - FVector2D(Origin.X, Origin.Y);
    Towards = Towards.IsNearlyZero() ? FVector2D(0.0, 1.0) : Towards.GetSafeNormal();
    const float GroundZ = Template->Location.Z;
    for (const pe::Offset& Offset : pe::OutputOffsets({Towards.X, Towards.Y}))
    {
        const FVector Candidate(Origin.X + Offset.x, Origin.Y + Offset.y, GroundZ);
        const FVector2D MapPosition(Candidate.X - PlayableMin.X, Candidate.Y - PlayableMin.Y);
        bool bOpen = false;
#if WITH_DEV_AUTOMATION_TESTS
        if (FormationSlotProbeForTest) bOpen = FormationSlotProbeForTest(MapPosition);
        else
#endif
        bOpen = IsGroundSlotOpen(GetWorld(), PlayableMin, PlayableMax, MapPosition, GroundZ);
        if (!bOpen) continue;
        const bool bOccupied = Units.ContainsByPredicate([&Candidate](const APolitikaPrototypeUnit* Unit)
        {
            return IsValid(Unit) && Unit->IsAlive()
                && FVector::DistSquared2D(Unit->GetActorLocation(), Candidate) < kOutputClearanceCm * kOutputClearanceCm;
        });
        if (bOccupied) continue;
        OutLocation = Candidate;
        OutRotation = Template->Rotation;
        return true;
    }
    return false;
}

const UPolitikaUnitArchetypeDefinition* APolitikaPrototypeMatch::GetHarvesterDefinition()
{
    if (HarvesterDefinition) return HarvesterDefinition;
    // Transient and identical on every process, so clients can present a unit whose archetype no
    // catalog holds. A harvester has no weapon: CanAttack is false and attack orders are refused.
    HarvesterDefinition = NewObject<UPolitikaUnitArchetypeDefinition>(this, TEXT("HarvesterFixture"), RF_Transient);
    HarvesterDefinition->StableId = HarvesterArchetypeId();
    HarvesterDefinition->DisplayName = FText::FromString(TEXT("Harvester [Playtest Placeholder]"));
    HarvesterDefinition->UnitCategory = EPolitikaUnitCategory::NonCombat;
    HarvesterDefinition->MaxHealth = kHarvesterMaxHealth;
    HarvesterDefinition->MovementSpeedCmPerSecond = kHarvesterSpeedCmPerSecond;
    HarvesterDefinition->VisualMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
    HarvesterDefinition->VisualScale = FVector(1.1f, 0.7f, 0.6f);
    HarvesterDefinition->FactionTint = FLinearColor(0.85f, 0.6f, 0.12f);
    return HarvesterDefinition;
}

APolitikaPrototypeUnit* APolitikaPrototypeMatch::SpawnHarvester(uint32 ParticipantId, const FVector& Location, const FRotator& Rotation)
{
    const FPolitikaPrototypeRuntimeParticipant* Seat = RuntimeParticipants.FindByPredicate(
        [ParticipantId](const FPolitikaPrototypeRuntimeParticipant& Candidate) { return Candidate.RuntimePlayerId == ParticipantId; });
    const UPolitikaUnitArchetypeDefinition* Definition = GetHarvesterDefinition();
    if (!Seat || !Definition) return nullptr;
    APolitikaPrototypeUnit* Unit = GetWorld()->SpawnActor<APolitikaPrototypeUnit>(Location, Rotation);
    if (!Unit) return nullptr;
    Unit->Configure(NextStableId++, ParticipantId, Definition->DisplayName, Definition->MaxHealth, Definition->MovementSpeedCmPerSecond,
        0.0f, 0.0f, 0.0f, true, Definition->VisualMesh, Definition->VisualScale, Definition->FactionTint, Seat->FactionId, false);
    Unit->SetArchetypeId(Definition->StableId);
    Unit->SetUnitCategory(Definition->UnitCategory);
    Unit->SetTeamId(Seat->TeamId);
    Unit->SetSightRadius(Definition->SightRadiusCm);
    Unit->AuthorityMatch = this;
    Units.Add(Unit);
    AssignNetOwner(Unit);
    return Unit;
}

APolitikaPrototypeUnit* APolitikaPrototypeMatch::SpawnProducedUnit(uint32 ParticipantId, const FVector& Location, const FRotator& Rotation)
{
    const FPolitikaPrototypeRuntimeParticipant* Seat = RuntimeParticipants.FindByPredicate(
        [ParticipantId](const FPolitikaPrototypeRuntimeParticipant& Candidate) { return Candidate.RuntimePlayerId == ParticipantId; });
    const FPolitikaPrototypeUnitPlacement* Template = Seat ? MapDefinition->UnitPlacements.FindByPredicate(
        [Seat](const FPolitikaPrototypeUnitPlacement& Candidate) { return Candidate.SlotId == Seat->SlotId; }) : nullptr;
    if (!Template) return nullptr;
    FPolitikaPrototypeUnitPlacement Placement = *Template;
    Placement.Location = Location;
    Placement.Rotation = Rotation;
    return SpawnPlacedUnit(Placement, false);
}

APolitikaPrototypeUnit* APolitikaPrototypeMatch::SpawnPlacedUnit(const FPolitikaPrototypeUnitPlacement& Placement, bool bUnaligned)
{
    const UPolitikaUnitArchetypeDefinition* Definition = ContentCatalog->FindUnit(Placement.UnitArchetypeId);
    const UPolitikaWeaponDefinition* Weapon = Definition ? ContentCatalog->FindWeapon(Definition->WeaponId) : nullptr;
    const FPolitikaPrototypeRuntimeParticipant* Slot = bUnaligned ? nullptr : RuntimeParticipants.FindByPredicate(
        [&](const FPolitikaPrototypeRuntimeParticipant& Candidate) { return Candidate.SlotId == Placement.SlotId; });
    if (!Definition || !Weapon || (!Slot && !bUnaligned)) return nullptr;
    if (bUnaligned && Definition->FactionId != UnalignedSystemAuthority.FactionId) return nullptr;
    APolitikaPrototypeUnit* Unit = GetWorld()->SpawnActor<APolitikaPrototypeUnit>(Placement.Location, Placement.Rotation);
    if (!Unit) return nullptr;
    Unit->Configure(NextStableId++, Slot ? Slot->RuntimePlayerId : 0u, Definition->DisplayName, Definition->MaxHealth,
        Definition->MovementSpeedCmPerSecond, Weapon->RangeCm, Weapon->Damage, Weapon->CooldownSeconds,
        Definition->bAttackable, Definition->VisualMesh, Definition->VisualScale, Definition->FactionTint,
        Slot ? Slot->FactionId : UnalignedSystemAuthority.FactionId, Definition->bUseAuthoredMeshMaterials);
    Unit->ConfigureInfantryPresentation(Definition->InfantryPresentation);
    Unit->ConfigureInfantryBehavior(Definition->InfantryBehavior);
    Unit->SetArchetypeId(Definition->StableId);
    Unit->SetUnitCategory(Definition->UnitCategory);
    Unit->SetTeamId(Slot ? Slot->TeamId : politika::rules::kUnalignedTeam);
    Unit->SetSightRadius(Definition->SightRadiusCm);
    Unit->AuthorityMatch = this;
    Units.Add(Unit);
    AssignNetOwner(Unit);
    return Unit;
}

#if !UE_BUILD_SHIPPING
void APolitikaPrototypeMatch::ApplyPowerOutageFixture(uint64 Step, uint32 FirstBuyer)
{
    if (OutageOffsetSteps < 0) return;
    if (OutageParticipant == 0)
    {
        if (FirstBuyer == 0) return;
        OutageParticipant = FirstBuyer;
        OutageBeginStep = Step + OutageOffsetSteps;
    }
    Economy.SetGeneratorOnline(OutageParticipant, Step < OutageBeginStep || Step >= OutageBeginStep + OutageDurationSteps);
}
#endif

void APolitikaPrototypeMatch::LogHarvest(const TCHAR* Kind, uint32 ParticipantId, uint64 UnitId, pe::Resource Resource, int64 Amount,
    int64 GrossCents, int64 FeeCents, uint64 Step) const
{
#if !UE_BUILD_SHIPPING
    if (const pe::Account* Account = Economy.Find(ParticipantId))
    {
        const pe::ResourceSource& Source = Account->Source(Resource);
        const bool bGold = Resource == pe::Resource::Gold;
        UE_LOG(LogPolitikaStartup, Display,
            TEXT("RTS_HARVEST_TX participant=%u kind=%s resource=%s unit=%llu amount=%lld gross=%lld fee=%lld source=%lld carried=%lld held=%lld delivered=%lld sold=%lld lost=%lld harvesters=%d reconciles=%d generation=%llu step=%llu"),
            ParticipantId, Kind, UTF8_TO_TCHAR(pe::ResourceName(Resource).data()), UnitId, Amount, GrossCents, FeeCents, Source.remaining,
            Account->Carried(Resource), bGold ? Account->goldHeld : Account->rations, Source.delivered, bGold ? Account->goldSold : int64(0), Source.lost,
            static_cast<int32>(Account->harvesters.size()), Account->GoldReconciles() && Account->FoodSourceReconciles() && Account->RationsReconcile() ? 1 : 0,
            MatchGeneration, Step);
    }
#else
    (void)Kind; (void)ParticipantId; (void)UnitId; (void)Resource; (void)Amount; (void)GrossCents; (void)FeeCents; (void)Step;
#endif
}

void APolitikaPrototypeMatch::SendHarvester(APolitikaPrototypeUnit* Unit, const FVector& Destination)
{
    // The same command path as a player's Move, as its owner. Remembering the sequence is what tells
    // this order apart from a player's when it is admitted.
    const politika::rules::Position Target{Destination.X - PlayableMin.X, Destination.Y - PlayableMin.Y};
    HarvesterOrderSequences.Add(SubmitCommand(Unit->GetOwnerPlayer(), Unit, nullptr, politika::rules::MoveOrder{Target}));
}

void APolitikaPrototypeMatch::NoteHarvesterOrderAdmitted(const APolitikaPrototypeUnit* Unit, uint64 Sequence)
{
    if (HarvesterOrderSequences.Remove(Sequence) > 0 || !Unit) return;
    const uint32 ParticipantId = Unit->GetOwnerPlayer();
    if (!Economy.DetachHarvester(ParticipantId, Unit->GetStableId())) return;
    LogHarvest(TEXT("detached"), ParticipantId, Unit->GetStableId(), Economy.FindHarvester(ParticipantId, Unit->GetStableId())->target, 0, 0, 0, SimulationStepsExecuted);
}

void APolitikaPrototypeMatch::RunHarvesters(uint64 Step)
{
    TArray<uint32> Participants;
    for (const pe::Account& Account : Economy.All()) Participants.Add(Account.participant);
    for (const uint32 ParticipantId : Participants)
    {
        const APolitikaPrototypeStructure* Depot = FindStructureOf(ParticipantId);
        const APolitikaPrototypeStructure* GoldActor = FindSourceOf(ParticipantId, pe::Resource::Gold);
        const APolitikaPrototypeStructure* FoodActor = FindSourceOf(ParticipantId, pe::Resource::Food);
        const auto Near = [](const APolitikaPrototypeUnit* Unit, const APolitikaPrototypeStructure* Structure, double RadiusCm)
        {
            return Structure && FVector::DistSquared2D(Unit->GetActorLocation(), Structure->GetActorLocation()) <= RadiusCm * RadiusCm;
        };
        // Ascending unit order: a copy, because this step's casualties leave the list.
        TArray<pe::UnitId> Harvesters;
        for (const pe::Harvester& Harvester : Economy.Find(ParticipantId)->harvesters) Harvesters.Add(Harvester.unit);
        for (const pe::UnitId UnitId : Harvesters)
        {
            const TObjectPtr<APolitikaPrototypeUnit>* Found = Units.FindByPredicate([UnitId, ParticipantId](const APolitikaPrototypeUnit* Unit)
            {
                return IsValid(Unit) && Unit->IsAlive() && Unit->GetStableId() == UnitId && Unit->GetOwnerPlayer() == ParticipantId;
            });
            if (!Found)
            {
                const pe::Resource LoadKind = Economy.FindHarvester(ParticipantId, UnitId)->loadKind;
                const pe::Gold Lost = Economy.LoseHarvester(ParticipantId, UnitId);
                LogHarvest(TEXT("lost"), ParticipantId, UnitId, LoadKind, Lost, 0, 0, Step);
                continue;
            }
            APolitikaPrototypeUnit* Unit = *Found;
            pe::HarvesterSituation Situation;
            if (Near(Unit, GoldActor, kGoldPickupRadiusCm)) Situation.atSource = pe::Resource::Gold;
            else if (Near(Unit, FoodActor, kGoldPickupRadiusCm)) Situation.atSource = pe::Resource::Food;
            Situation.atBase = Near(Unit, Depot, kGoldDepotRadiusCm);
            Situation.hasDepot = Depot != nullptr;
            // Stop stays the current order after a player's X or a failed route; arriving ends in Idle.
            Situation.moving = Unit->GetCurrentOrder() == EPolitikaPrototypeOrder::Move;
            Situation.halted = Unit->GetCurrentOrder() == EPolitikaPrototypeOrder::Stop;
            const pe::HarvesterStep Result = Economy.StepHarvester(ParticipantId, UnitId, Situation);
            const pe::Harvester* State = Economy.FindHarvester(ParticipantId, UnitId);
            if (Result.arrivedAtSource) LogHarvest(TEXT("loading"), ParticipantId, UnitId, State->target, 0, 0, 0, Step);
            if (Result.loaded > 0) LogHarvest(TEXT("load"), ParticipantId, UnitId, Result.resource, Result.loaded, 0, 0, Step);
            if (Result.delivered > 0) LogHarvest(TEXT("deliver"), ParticipantId, UnitId, Result.resource, Result.delivered, 0, 0, Step);
            if (Result.retargeted) LogHarvest(TEXT("retarget"), ParticipantId, UnitId, State->target, 0, 0, 0, Step);
            if (Result.heldForDepot) LogHarvest(TEXT("holding"), ParticipantId, UnitId, State->target, 0, 0, 0, Step);
            if (Result.startedWaiting) LogHarvest(TEXT("waiting"), ParticipantId, UnitId, State->target, 0, 0, 0, Step);
            if (Result.stoppedWaiting) LogHarvest(TEXT("resumed"), ParticipantId, UnitId, State->target, 0, 0, 0, Step);
#if !UE_BUILD_SHIPPING
            if (Result.delivered > 0 && Result.resource == pe::Resource::Food)
                if (const pe::Account* Account = Economy.Find(ParticipantId))
                    UE_LOG(LogPolitikaStartup, Display,
                        TEXT("RTS_FOOD_TX participant=%u kind=harvest owed=0 paid=%lld rations=%lld bought=%lld issued=%lld shortage=%d queued=%d head_remaining=0 reconciles=%d generation=%llu step=%llu"),
                        ParticipantId, Result.delivered, Account->rations, Account->rationsBought, Account->rationsIssued, Account->foodShortage ? 1 : 0,
                        static_cast<int32>(Account->queue.size()), Account->RationsReconcile() ? 1 : 0, MatchGeneration, Step);
#endif
            Unit->SetCarriedResource(State->load > 0 ? std::optional<pe::Resource>(State->loadKind) : std::nullopt);
            if (Result.goToBase && Depot) SendHarvester(Unit, Depot->GetActorLocation());
            if (Result.goToSource)
                if (const APolitikaPrototypeStructure* Destination = *Result.goToSource == pe::Resource::Gold ? GoldActor : FoodActor)
                    SendHarvester(Unit, Destination->GetActorLocation());
        }
        const pe::Account* Account = Economy.Find(ParticipantId);
        for (APolitikaPrototypeStructure* Structure : Structures)
        {
            if (Structure == GoldActor && Account->goldSource.remaining == 0) Structure->MarkSourceEmpty();
            if (Structure == FoodActor && Account->foodSource.remaining == 0) Structure->MarkSourceEmpty();
        }
    }
}

void APolitikaPrototypeMatch::RunEconomyPhase()
{
    const uint64 Step = SimulationStepsExecuted;
    const auto LogFood = [&](const TCHAR* Kind, uint32 ParticipantId, int64 Owed, int64 Paid, uint32 HeadRemaining)
    {
#if !UE_BUILD_SHIPPING
        if (const pe::Account* Account = Economy.Find(ParticipantId))
            UE_LOG(LogPolitikaStartup, Display,
                TEXT("RTS_FOOD_TX participant=%u kind=%s owed=%lld paid=%lld rations=%lld bought=%lld issued=%lld shortage=%d queued=%d head_remaining=%u reconciles=%d generation=%llu step=%llu"),
                ParticipantId, Kind, Owed, Paid, Account->rations, Account->rationsBought, Account->rationsIssued, Account->foodShortage ? 1 : 0,
                static_cast<int32>(Account->queue.size()), HeadRemaining, Account->RationsReconcile() ? 1 : 0, MatchGeneration, Step);
#else
        (void)Kind; (void)ParticipantId; (void)Owed; (void)Paid; (void)HeadRemaining;
#endif
    };
    const auto LogMoney = [&](const TCHAR* Kind, uint32 ParticipantId, uint32 Transaction, int64 Amount)
    {
#if !UE_BUILD_SHIPPING
        if (const pe::Account* Account = Economy.Find(ParticipantId))
            UE_LOG(LogPolitikaStartup, Display,
                TEXT("RTS_ECONOMY_TX participant=%u kind=%s transaction=%u amount=%lld available=%lld credited=%lld spent=%lld refunded=%lld queued=%d reconciles=%d generation=%llu step=%llu"),
                ParticipantId, Kind, Transaction, Amount, Account->available, Account->credited, Account->spent, Account->refunded,
                static_cast<int32>(Account->queue.size()), Account->Reconciles() ? 1 : 0, MatchGeneration, Step);
#else
        (void)Kind; (void)ParticipantId; (void)Transaction; (void)Amount;
#endif
    };

    // 1. Requests, in ascending participant order and arrival order within one participant.
    TArray<FPendingEconomyRequest> Requests = MoveTemp(PendingEconomyRequests);
    PendingEconomyRequests.Reset();
    Requests.StableSort([](const FPendingEconomyRequest& A, const FPendingEconomyRequest& B) { return A.ParticipantId < B.ParticipantId; });
    TMap<uint32, uint32> FirstAdmitted;
    for (const FPendingEconomyRequest& Request : Requests)
    {
        APlayerController* Source = Request.Source.Get();
        const pe::EconomyOp Op = static_cast<pe::EconomyOp>(Request.Intent.Op);
        const pe::RequestResult Result = Op == pe::EconomyOp::Train
            ? Economy.Train(Request.ParticipantId, Request.Intent.ClientSequence, Request.Intent.Recipe,
                CountLivingUnits(Request.ParticipantId), politika::unitscale::kMaxUnitsPerSeat)
            : Op == pe::EconomyOp::BuyFood ? Economy.BuyFood(Request.ParticipantId, Request.Intent.ClientSequence)
            : Op == pe::EconomyOp::SellGold ? Economy.SellGold(Request.ParticipantId, Request.Intent.ClientSequence)
            : Economy.CancelLast(Request.ParticipantId, Request.Intent.ClientSequence);
        if (!Result)
        {
            RejectEconomyIntent(Source, Request.Intent, Request.ParticipantId, Result.rejection);
            continue;
        }
        if (Op == pe::EconomyOp::Train) FirstAdmitted.FindOrAdd(Request.ParticipantId, Request.Intent.ClientSequence);
        LogMoney(Op == pe::EconomyOp::Train ? TEXT("train") : Op == pe::EconomyOp::BuyFood ? TEXT("food")
            : Op == pe::EconomyOp::SellGold ? TEXT("sale") : TEXT("refund"),
            Request.ParticipantId, Request.Intent.ClientSequence, Result.amount);
        if (Op == pe::EconomyOp::BuyFood) LogFood(TEXT("purchase"), Request.ParticipantId, 0, 0, 0u);
        if (Op == pe::EconomyOp::SellGold) LogHarvest(TEXT("sale"), Request.ParticipantId, 0, pe::Resource::Gold, Result.sale.quantity, Result.sale.gross, Result.sale.fee, Step);
        SendEconomyOutcome(Source, Request.Intent, pe::EconomyRejection::None, Result.amount, Result.available,
            static_cast<int32>(Result.queued), static_cast<int32>(Result.sale.quantity));
    }

    // Harvesters after requests, so gold delivered in this step can be sold from the next one.
    RunHarvesters(Step);

#if !UE_BUILD_SHIPPING
    {
        TArray<uint32> Buyers;
        FirstAdmitted.GetKeys(Buyers);
        Buyers.Sort();
        ApplyPowerOutageFixture(Step, Buyers.IsEmpty() ? 0u : Buyers[0]);
    }
#endif

    // 2. Income, power, food upkeep, then production, per account in ascending participant order.
    // The force is counted now: this step's casualties have left it, and a unit spawned below joins
    // it for the next interval.
    bool bSpawned = false;
    TArray<uint32> Participants;
    for (const pe::Account& Account : Economy.All()) Participants.Add(Account.participant);
    for (const uint32 ParticipantId : Participants)
    {
        const uint32* Admitted = FirstAdmitted.Find(ParticipantId);
        const pe::StepResult Result = Economy.Advance(ParticipantId, Step, Admitted ? *Admitted : 0u, CountLivingUnits(ParticipantId));
        if (Result.upkeepDue) LogFood(TEXT("upkeep"), ParticipantId, Result.upkeepOwed, Result.upkeepPaid, Result.headRemainingSteps);
        if (Result.income != 0) LogMoney(TEXT("income"), ParticipantId, 0u, Result.income);
#if !UE_BUILD_SHIPPING
        if (Result.incomeOverflow)
            UE_LOG(LogPolitikaStartup, Warning, TEXT("RTS_ECONOMY_INCOME_OVERFLOW participant=%u step=%llu"), ParticipantId, Step);
        if (Result.powerChanged)
            if (const pe::Account* Account = Economy.Find(ParticipantId))
                UE_LOG(LogPolitikaStartup, Display,
                    TEXT("RTS_POWER_STATE participant=%u supplied_kw=%lld required_kw=%lld producer_powered=%d queued=%d head_remaining=%u available=%lld generation=%llu step=%llu"),
                    ParticipantId, Account->allocation.suppliedKw, Account->allocation.requiredKw, Account->producerPowered ? 1 : 0,
                    static_cast<int32>(Account->queue.size()), Result.headRemainingSteps, Account->available, MatchGeneration, Step);
#endif
        if (!Result.spawnDue) continue;
        FVector Location = FVector::ZeroVector;
        FRotator Rotation = FRotator::ZeroRotator;
        const bool bHarvester = Result.recipe == pe::Recipe::Harvester;
        APolitikaPrototypeUnit* Unit = !FindProducerOutput(ParticipantId, Location, Rotation) ? nullptr
            : bHarvester ? SpawnHarvester(ParticipantId, Location, Rotation) : SpawnProducedUnit(ParticipantId, Location, Rotation);
        if (Unit && Economy.CompleteHead(ParticipantId, Result.transaction))
        {
            bSpawned = true;
            if (bHarvester)
            {
                Economy.AddHarvester(ParticipantId, Unit->GetStableId());
                LogHarvest(TEXT("harvester"), ParticipantId, Unit->GetStableId(), pe::Resource::Gold, 0, 0, 0, Step);
            }
#if !UE_BUILD_SHIPPING
            UE_LOG(LogPolitikaStartup, Display,
                TEXT("RTS_PRODUCTION_SPAWN participant=%u transaction=%u entity=%llu team=%u x=%.1f y=%.1f living=%d generation=%llu step=%llu recipe=%u"),
                ParticipantId, Result.transaction, Unit->GetStableId(), static_cast<uint32>(Unit->GetTeamId()), Location.X, Location.Y,
                CountLivingUnits(ParticipantId), MatchGeneration, Step, static_cast<uint32>(Result.recipe));
#endif
        }
        else
        {
            Economy.BlockHead(ParticipantId, Result.transaction, Step);
#if !UE_BUILD_SHIPPING
            UE_LOG(LogPolitikaStartup, Display, TEXT("RTS_PRODUCTION_BLOCKED participant=%u transaction=%u retry_step=%llu generation=%llu step=%llu"),
                ParticipantId, Result.transaction, Step + pe::kBlockedRetrySteps, MatchGeneration, Step);
#endif
        }
    }
    // A new unit counts for team vision and relevancy from the step it appears in.
    if (bSpawned) UpdateTeamVisibility();
    SyncEconomyReplication();
}

void APolitikaPrototypeMatch::SyncEconomyReplication()
{
    if (!HasAuthority()) return;
    for (const FParticipantBinding& Binding : ParticipantBindings)
    {
        APolitikaPrototypePlayerController* Controller = Cast<APolitikaPrototypePlayerController>(Binding.Controller.Get());
        if (!Controller) continue;
        FPolitikaNetEconomyView View;
        if (const pe::Account* Account = Economy.Find(Binding.RuntimePlayerId))
        {
            View.bValid = true;
            View.bHasProducer = Account->hasProducer;
            View.AvailableCents = Account->available;
            View.CreditedCents = Account->credited;
            View.SpentCents = Account->spent;
            View.RefundedCents = Account->refunded;
            View.Queued = static_cast<int32>(Account->queue.size());
            View.Produced = static_cast<int32>(Account->produced);
            View.SuppliedKw = static_cast<int32>(Account->allocation.suppliedKw);
            View.RequiredKw = static_cast<int32>(Account->allocation.requiredKw);
            View.bProducerPowered = Account->producerPowered;
            View.Rations = static_cast<int32>(Account->rations);
            View.UpkeepRations = static_cast<int32>(CountLivingUnits(Binding.RuntimePlayerId) * pe::kRationsPerUnit);
            View.bFoodShortage = Account->foodShortage;
            View.bHasGoldSource = Account->goldSource.present;
            View.GoldHeld = static_cast<int32>(Account->goldHeld);
            View.GoldCarried = static_cast<int32>(Account->GoldCarried());
            View.GoldAtSource = static_cast<int32>(Account->goldSource.remaining);
            View.bHasFoodSource = Account->foodSource.present;
            View.FoodCarried = static_cast<int32>(Account->FoodCarried());
            View.FoodAtSource = static_cast<int32>(Account->foodSource.remaining);
            View.Harvesters = static_cast<int32>(Account->harvesters.size());
            if (!Account->queue.empty())
            {
                View.HeadRecipe = static_cast<uint8>(Account->queue.front().recipe);
                const int32 Remaining = static_cast<int32>(Account->queue.front().remainingSteps);
                View.HeadRemainingSteps = FMath::DivideAndRoundUp(Remaining, kViewRemainingGranularity) * kViewRemainingGranularity;
                View.bHeadBlocked = Account->queue.front().blocked;
            }
        }
        Controller->SetOwnEconomy(View);
    }
}

// ----------------------------------------------------------------------------
// Player controller
// ----------------------------------------------------------------------------

void APolitikaPrototypePlayerController::SetOwnEconomy(const FPolitikaNetEconomyView& View)
{
    if (!HasAuthority() || View == OwnEconomy) return;
    OwnEconomy = View;
    if (IsLocalSeatController()) LogOwnEconomy(TEXT("authority"));
}

void APolitikaPrototypePlayerController::OnRep_OwnEconomy()
{
    LogOwnEconomy(TEXT("replicated"));
}

void APolitikaPrototypePlayerController::LogOwnEconomy(const TCHAR* Source) const
{
#if !UE_BUILD_SHIPPING
    UE_LOG(LogPolitikaStartup, Display,
        TEXT("RTS_ECONOMY_VIEW participant=%u source=%s valid=%d producer=%d available=%lld credited=%lld spent=%lld refunded=%lld queued=%d head_remaining=%d blocked=%d produced=%d supplied_kw=%d required_kw=%d powered=%d rations=%d upkeep=%d shortage=%d gold_held=%d gold_carried=%d gold_source=%d harvesters=%d food_carried=%d food_source=%d"),
        AssignedRuntimePlayerId != 0 ? AssignedRuntimePlayerId : GetCommandPlayerId(), Source, OwnEconomy.bValid ? 1 : 0,
        OwnEconomy.bHasProducer ? 1 : 0, OwnEconomy.AvailableCents, OwnEconomy.CreditedCents, OwnEconomy.SpentCents,
        OwnEconomy.RefundedCents, OwnEconomy.Queued, OwnEconomy.HeadRemainingSteps, OwnEconomy.bHeadBlocked ? 1 : 0, OwnEconomy.Produced,
        OwnEconomy.SuppliedKw, OwnEconomy.RequiredKw, OwnEconomy.bProducerPowered ? 1 : 0, OwnEconomy.Rations, OwnEconomy.UpkeepRations,
        OwnEconomy.bFoodShortage ? 1 : 0, OwnEconomy.GoldHeld, OwnEconomy.GoldCarried, OwnEconomy.GoldAtSource, OwnEconomy.Harvesters, OwnEconomy.FoodCarried, OwnEconomy.FoodAtSource);
#else
    (void)Source;
#endif
}

void APolitikaPrototypePlayerController::TrainOrCancelAtProducer()
{
    const bool bCancel = IsInputKeyDown(EKeys::LeftShift) || IsInputKeyDown(EKeys::RightShift);
    SubmitEconomyRequest(bCancel ? pe::EconomyOp::Cancel : pe::EconomyOp::Train,
        bCancel ? 0 : static_cast<uint8>(pe::Recipe::Rifleman));
}

void APolitikaPrototypePlayerController::BuyFood()
{
    SubmitEconomyRequest(pe::EconomyOp::BuyFood, 0);
}

void APolitikaPrototypePlayerController::TrainHarvester()
{
    SubmitEconomyRequest(pe::EconomyOp::Train, static_cast<uint8>(pe::Recipe::Harvester));
}

void APolitikaPrototypePlayerController::SellGold()
{
    SubmitEconomyRequest(pe::EconomyOp::SellGold, 0);
}

bool APolitikaPrototypePlayerController::SubmitEconomyRequest(pe::EconomyOp Op, uint8 Recipe)
{
    APolitikaPrototypeMatch* ActiveMatch = ResolveMatch();
    if (!ActiveMatch) return false;
    FPolitikaNetEconomyIntent Intent;
    Intent.ProtocolVersion = static_cast<int32>(FPolitikaBootstrap::Get().NetworkProtocol());
    Intent.Op = static_cast<uint8>(Op);
    Intent.Recipe = Recipe;
    Intent.ClientSequence = NextClientSequence++;
    Intent.CorrelationId = NextCorrelationId++;
    Intent.MatchGeneration = ActiveMatch->GetMatchGeneration();
#if !UE_BUILD_SHIPPING
    UE_LOG(LogPolitikaStartup, Display, TEXT("RTS_ECONOMY_SENT participant=%u op=%u recipe=%u correlation=%u client_sequence=%u generation=%llu protocol=%d authority=%d"),
        GetCommandPlayerId(), static_cast<uint32>(Intent.Op), static_cast<uint32>(Intent.Recipe), Intent.CorrelationId, Intent.ClientSequence,
        Intent.MatchGeneration, Intent.ProtocolVersion, HasAuthority() ? 1 : 0);
#endif
    if (HasAuthority()) return ActiveMatch->SubmitEconomyIntent(this, Intent);
    ServerSubmitEconomyIntent(Intent);
    return true;
}

void APolitikaPrototypePlayerController::ServerSubmitEconomyIntent_Implementation(const FPolitikaNetEconomyIntent& Intent)
{
    if (APolitikaPrototypeMatch* ActiveMatch = FindEconomyMatch(GetWorld()))
        ActiveMatch->SubmitEconomyIntent(this, Intent);
}

void APolitikaPrototypePlayerController::ClientReceiveEconomyOutcome_Implementation(const FPolitikaNetEconomyOutcome& Outcome)
{
#if !UE_BUILD_SHIPPING
    UE_LOG(LogPolitikaStartup, Display,
        TEXT("RTS_ECONOMY_OUTCOME participant=%u op=%u correlation=%u client_sequence=%u accepted=%d rejection=%u amount=%lld available=%lld queued=%d generation=%llu authority=%d gold_sold=%d"),
        GetCommandPlayerId(), static_cast<uint32>(Outcome.Op), Outcome.CorrelationId, Outcome.ClientSequence, Outcome.bAccepted ? 1 : 0,
        static_cast<uint32>(Outcome.Rejection), Outcome.AmountCents, Outcome.AvailableCents, Outcome.Queued, Outcome.MatchGeneration, HasAuthority() ? 1 : 0, Outcome.GoldSold);
#endif
    const bool bFood = static_cast<pe::EconomyOp>(Outcome.Op) == pe::EconomyOp::BuyFood;
    const bool bSale = static_cast<pe::EconomyOp>(Outcome.Op) == pe::EconomyOp::SellGold;
    if (!Outcome.bAccepted)
    {
        ShowLocalFeedback(FString::Printf(TEXT("%s refused: %s"), bFood ? TEXT("Food purchase") : bSale ? TEXT("Gold sale") : TEXT("Training"),
            *EconomyRejectionText(static_cast<pe::EconomyRejection>(Outcome.Rejection))));
        return;
    }
    if (bFood)
    {
        ShowLocalFeedback(FString::Printf(TEXT("%lld rations bought for %s"), pe::kFoodPurchaseRations, *UsdText(Outcome.AmountCents)));
        return;
    }
    if (bSale)
    {
        ShowLocalFeedback(FString::Printf(TEXT("%d gold sold for %s"), Outcome.GoldSold, *UsdText(Outcome.AmountCents)));
        return;
    }
    ShowLocalFeedback(static_cast<pe::EconomyOp>(Outcome.Op) == pe::EconomyOp::Train
        ? FString::Printf(TEXT("Queued for %s  ·  %d in the queue"), *UsdText(Outcome.AmountCents), Outcome.Queued)
        : FString::Printf(TEXT("Training cancelled: %s refunded  ·  %d in the queue"), *UsdText(Outcome.AmountCents), Outcome.Queued));
}
