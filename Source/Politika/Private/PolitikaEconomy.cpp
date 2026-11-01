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
    DOREPLIFETIME(APolitikaPrototypeStructure, OwnerPlayer);
    DOREPLIFETIME(APolitikaPrototypeStructure, TeamId);
    DOREPLIFETIME(APolitikaPrototypeStructure, Tint);
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
    // Non-replicated component state: a listen host does not see an enemy producer either.
    if (bHiddenLocally) Body->SetHiddenInGame(true, true);
}

void APolitikaPrototypeStructure::BeginPlay()
{
    Super::BeginPlay();
    if (HasAuthority()) return;
    ApplyTint();
#if !UE_BUILD_SHIPPING
    UE_LOG(LogPolitikaStartup, Display, TEXT("RTS_STRUCTURE_RELEVANT kind=%s owner=%u team=%u x=%.1f y=%.1f"),
        IsProducer() ? TEXT("producer") : TEXT("generator"), OwnerPlayer, static_cast<uint32>(TeamId), GetActorLocation().X, GetActorLocation().Y);
#endif
}

void APolitikaPrototypeStructure::ApplyTint()
{
    if (GetNetMode() == NM_DedicatedServer) return;
    // The generator is the narrower of the two placeholder blocks, at the same height.
    if (!IsProducer()) Body->SetRelativeScale3D(FVector(2.0, 2.0, 3.0));
    if (UMaterialInstanceDynamic* Material = Body->CreateAndSetMaterialInstanceDynamic(0))
        Material->SetVectorParameterValue(TEXT("Color"), Tint);
}

bool APolitikaPrototypeMatch::IsStructureRelevantToViewer(const APolitikaPrototypeStructure* Structure, const AActor* RealViewer) const
{
    if (!bFogOfWar) return true;
    const uint32 ParticipantId = GetBoundParticipantId(Cast<APlayerController>(RealViewer));
    return ParticipantId != 0 && Structure && GetTeamForParticipant(ParticipantId) == Structure->GetTeamId();
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
                        Definition->FactionTint * 0.45f, bHiddenLocally);
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
#if !UE_BUILD_SHIPPING
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
            TEXT("RTS_ECONOMY_CLEARED participant=%u reason=%s available=%lld credited=%lld spent=%lld refunded=%lld dropped_queue=%d produced=%u reconciles=%d generation=%llu step=%llu"),
            Account.participant, Reason, Account.available, Account.credited, Account.spent, Account.refunded,
            static_cast<int32>(Account.queue.size()), Account.produced, Account.Reconciles() ? 1 : 0, MatchGeneration, SimulationStepsExecuted);
#else
    (void)Reason;
#endif
    // Money and queues belong to the active match; the next generation starts from its own start.
    Economy.Clear();
}

// ----------------------------------------------------------------------------
// Request intake
// ----------------------------------------------------------------------------

void APolitikaPrototypeMatch::SendEconomyOutcome(APlayerController* Source, const FPolitikaNetEconomyIntent& Intent,
    pe::EconomyRejection Rejection, int64 AmountCents, int64 AvailableCents, int32 Queued)
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

const APolitikaPrototypeStructure* APolitikaPrototypeMatch::FindStructureOf(uint32 ParticipantId) const
{
    for (const APolitikaPrototypeStructure* Structure : Structures)
        if (IsValid(Structure) && Structure->IsProducer() && Structure->GetOwnerPlayer() == ParticipantId) return Structure;
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
            : Economy.CancelLast(Request.ParticipantId, Request.Intent.ClientSequence);
        if (!Result)
        {
            RejectEconomyIntent(Source, Request.Intent, Request.ParticipantId, Result.rejection);
            continue;
        }
        if (Op == pe::EconomyOp::Train) FirstAdmitted.FindOrAdd(Request.ParticipantId, Request.Intent.ClientSequence);
        LogMoney(Op == pe::EconomyOp::Train ? TEXT("train") : Op == pe::EconomyOp::BuyFood ? TEXT("food") : TEXT("refund"),
            Request.ParticipantId, Request.Intent.ClientSequence, Result.amount);
        if (Op == pe::EconomyOp::BuyFood) LogFood(TEXT("purchase"), Request.ParticipantId, 0, 0, 0u);
        SendEconomyOutcome(Source, Request.Intent, pe::EconomyRejection::None, Result.amount, Result.available,
            static_cast<int32>(Result.queued));
    }

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
        APolitikaPrototypeUnit* Unit = FindProducerOutput(ParticipantId, Location, Rotation)
            ? SpawnProducedUnit(ParticipantId, Location, Rotation) : nullptr;
        if (Unit && Economy.CompleteHead(ParticipantId, Result.transaction))
        {
            bSpawned = true;
#if !UE_BUILD_SHIPPING
            UE_LOG(LogPolitikaStartup, Display,
                TEXT("RTS_PRODUCTION_SPAWN participant=%u transaction=%u entity=%llu team=%u x=%.1f y=%.1f living=%d generation=%llu step=%llu"),
                ParticipantId, Result.transaction, Unit->GetStableId(), static_cast<uint32>(Unit->GetTeamId()), Location.X, Location.Y,
                CountLivingUnits(ParticipantId), MatchGeneration, Step);
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
            if (!Account->queue.empty())
            {
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
        TEXT("RTS_ECONOMY_VIEW participant=%u source=%s valid=%d producer=%d available=%lld credited=%lld spent=%lld refunded=%lld queued=%d head_remaining=%d blocked=%d produced=%d supplied_kw=%d required_kw=%d powered=%d rations=%d upkeep=%d shortage=%d"),
        AssignedRuntimePlayerId != 0 ? AssignedRuntimePlayerId : GetCommandPlayerId(), Source, OwnEconomy.bValid ? 1 : 0,
        OwnEconomy.bHasProducer ? 1 : 0, OwnEconomy.AvailableCents, OwnEconomy.CreditedCents, OwnEconomy.SpentCents,
        OwnEconomy.RefundedCents, OwnEconomy.Queued, OwnEconomy.HeadRemainingSteps, OwnEconomy.bHeadBlocked ? 1 : 0, OwnEconomy.Produced,
        OwnEconomy.SuppliedKw, OwnEconomy.RequiredKw, OwnEconomy.bProducerPowered ? 1 : 0, OwnEconomy.Rations, OwnEconomy.UpkeepRations,
        OwnEconomy.bFoodShortage ? 1 : 0);
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
        TEXT("RTS_ECONOMY_OUTCOME participant=%u op=%u correlation=%u client_sequence=%u accepted=%d rejection=%u amount=%lld available=%lld queued=%d generation=%llu authority=%d"),
        GetCommandPlayerId(), static_cast<uint32>(Outcome.Op), Outcome.CorrelationId, Outcome.ClientSequence, Outcome.bAccepted ? 1 : 0,
        static_cast<uint32>(Outcome.Rejection), Outcome.AmountCents, Outcome.AvailableCents, Outcome.Queued, Outcome.MatchGeneration, HasAuthority() ? 1 : 0);
#endif
    const bool bFood = static_cast<pe::EconomyOp>(Outcome.Op) == pe::EconomyOp::BuyFood;
    if (!Outcome.bAccepted)
    {
        ShowLocalFeedback(FString::Printf(TEXT("%s refused: %s"), bFood ? TEXT("Food purchase") : TEXT("Training"),
            *EconomyRejectionText(static_cast<pe::EconomyRejection>(Outcome.Rejection))));
        return;
    }
    if (bFood)
    {
        ShowLocalFeedback(FString::Printf(TEXT("%lld rations bought for %s"), pe::kFoodPurchaseRations, *UsdText(Outcome.AmountCents)));
        return;
    }
    ShowLocalFeedback(static_cast<pe::EconomyOp>(Outcome.Op) == pe::EconomyOp::Train
        ? FString::Printf(TEXT("Rifleman queued for %s  ·  %d in the queue"), *UsdText(Outcome.AmountCents), Outcome.Queued)
        : FString::Printf(TEXT("Training cancelled: %s refunded  ·  %d in the queue"), *UsdText(Outcome.AmountCents), Outcome.Queued));
}
