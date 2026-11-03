// The first playable opponent (OPP-1, Stage 4D; Docs/RTS_OPPONENT_AI.md).
//
// politika::opponent (PolitikaCore/Opponent.h) decides. This file binds one brain to one seat, builds
// the snapshot it may read, and sends what it proposes through the intake a human's request uses, after
// the part that resolves a controller. It changes no unit, ledger entry or result itself.
//
// Nothing here is replicated, and an opponent has no connection, so its account view, outcomes and memory
// reach no client. The logs below are the server's own.

#include "PolitikaPrototypeGameplay.h"

#include "PolitikaBootstrap.h"
#include "PolitikaPrototypeContent.h"
#include "Opponent.h"
#include "UnitScale.h"

#include "Misc/CommandLine.h"
#include "Misc/Parse.h"

namespace po = politika::opponent;
namespace pe = politika::economy;

bool APolitikaPrototypeMatch::ResolveOpponentSeatSwitch(FName& OutFailure)
{
#if !UE_BUILD_SHIPPING
    static const FString Prefix = TEXT("-PolitikaMPOpponentSeat=");
    const TCHAR* Stream = FCommandLine::Get();
#if WITH_DEV_AUTOMATION_TESTS
    if (!OpponentSwitchForTest.IsEmpty()) Stream = *OpponentSwitchForTest;
#endif
    TArray<FString> Values;
    FString Token;
    while (FParse::Token(Stream, Token, false))
        if (Token.StartsWith(Prefix, ESearchCase::IgnoreCase)) Values.Add(Token.RightChop(Prefix.Len()));
    if (Values.IsEmpty()) return true;
    const auto Refuse = [&OutFailure](FName Reason, const FString& Value)
    {
        OutFailure = Reason;
        UE_LOG(LogPolitikaStartup, Warning, TEXT("RTS_OPPONENT_BIND_REFUSED participant=%s reason=%s"), *Value, *Reason.ToString());
        return false;
    };
    for (const FString& Value : Values)
        if (Value != Values[0]) return Refuse(TEXT("OPP.SEAT.CONFLICT"), Value);
    const FTCHARToUTF8 Text(*Values[0]);
    const uint32 ParticipantId = po::ParseOpponentSeat(std::string_view(Text.Get(), Text.Length()));
    if (ParticipantId == 0) return Refuse(TEXT("OPP.SEAT.MALFORMED"), Values[0]);
    return BindOpponentBrain(ParticipantId, MatchGeneration, OutFailure);
#else
    (void)OutFailure;
    return true;
#endif
}

bool APolitikaPrototypeMatch::BindOpponentBrain(uint32 ParticipantId, uint64 Generation, FName& OutFailure)
{
    const auto Refuse = [ParticipantId, &OutFailure](const TCHAR* Reason)
    {
        OutFailure = Reason;
        UE_LOG(LogPolitikaStartup, Warning, TEXT("RTS_OPPONENT_BIND_REFUSED participant=%u reason=%s"), ParticipantId, Reason);
        return false;
    };
    if (!HasAuthority()) return Refuse(TEXT("OPP.BIND.NOT_AUTHORITY"));
    // A listen host holds every actor in memory and shows command feedback on its own screen. The first
    // opponent has only been qualified where neither applies.
    if (!IsDedicatedHosting()) return Refuse(TEXT("OPP.BIND.NOT_DEDICATED"));
    if (ParticipantId == 0) return Refuse(TEXT("OPP.BIND.PARTICIPANT_ZERO"));
    if (Generation != MatchGeneration) return Refuse(TEXT("OPP.BIND.STALE_GENERATION"));
    switch (Lifecycle.phase)
    {
        case politika::rules::MatchPhase::SetupPending:
        case politika::rules::MatchPhase::SetupValidated:
        case politika::rules::MatchPhase::Loading:
        case politika::rules::MatchPhase::Starting:
            break;
        default:
            return Refuse(TEXT("OPP.BIND.PHASE"));
    }
    const FPolitikaPrototypeRuntimeParticipant* Seat = RuntimeParticipants.FindByPredicate(
        [ParticipantId](const FPolitikaPrototypeRuntimeParticipant& Candidate) { return Candidate.RuntimePlayerId == ParticipantId; });
    if (!Seat) return Refuse(TEXT("OPP.BIND.MISSING_SEAT"));
    if (Seat->FactionId == UnalignedSystemAuthority.FactionId) return Refuse(TEXT("OPP.BIND.UNALIGNED"));
    if (Seat->ControllerKind == EPolitikaPrototypeControllerKind::LocalHuman) return Refuse(TEXT("OPP.BIND.LOCAL_HUMAN"));
    if (Seat->ControllerKind == EPolitikaPrototypeControllerKind::RemoteHuman) return Refuse(TEXT("OPP.BIND.REMOTE_HUMAN"));
    if (ParticipantBindings.ContainsByPredicate([ParticipantId](const FParticipantBinding& Binding) { return Binding.RuntimePlayerId == ParticipantId; }))
        return Refuse(TEXT("OPP.BIND.HUMAN_BOUND"));
    if (OpponentBrain.IsSet()) return Refuse(TEXT("OPP.BIND.DUPLICATE"));

    FOpponentBrain Brain;
    Brain.ParticipantId = ParticipantId;
    Brain.Generation = Generation;
    Brain.Memory = po::NewMemory(Generation);
    OpponentBrain = Brain;
    OutFailure = NAME_None;
#if !UE_BUILD_SHIPPING
    UE_LOG(LogPolitikaStartup, Display, TEXT("RTS_OPPONENT_BOUND participant=%u slot=%d faction=%s team=%u brain=%u.%llu generation=%llu next_sequence=%u kind=%s"),
        ParticipantId, Seat->SlotId, *Seat->FactionId.ToString(), Seat->TeamId, ParticipantId, Generation, Generation, Brain.Memory.nextSequence, TEXT("AssignedOpponent"));
#endif
    return true;
}

po::Snapshot APolitikaPrototypeMatch::BuildOpponentSnapshot() const
{
    po::Snapshot Snapshot;
    if (!OpponentBrain.IsSet()) return Snapshot;
    const uint32 ParticipantId = OpponentBrain->ParticipantId;
    Snapshot.participant = ParticipantId;
    Snapshot.generation = MatchGeneration;
    Snapshot.step = SimulationStepsExecuted;
    Snapshot.active = bContentReady && Lifecycle.IsActive();
    const FPolitikaPrototypeRuntimeParticipant* Seat = RuntimeParticipants.FindByPredicate(
        [ParticipantId](const FPolitikaPrototypeRuntimeParticipant& Candidate) { return Candidate.RuntimePlayerId == ParticipantId; });
    const pe::Account* Account = Economy.Find(ParticipantId);
    // Its seat must still be an unbound opponent: a seat a human holds has no brain.
    Snapshot.authorityValid = Seat && Account && !OpponentBrain->bRetired
        && Seat->ControllerKind == EPolitikaPrototypeControllerKind::AssignedOpponent
        && !ParticipantBindings.ContainsByPredicate([ParticipantId](const FParticipantBinding& Binding) { return Binding.RuntimePlayerId == ParticipantId; });
    if (!Seat || !Account) return Snapshot;

    Snapshot.available = Account->available;
    Snapshot.rations = Account->rations;
    Snapshot.foodShortage = Account->foodShortage;
    Snapshot.goldHeld = Account->goldHeld;
    Snapshot.hasProducer = Account->hasProducer;
    Snapshot.producerPowered = Account->producerPowered;
    Snapshot.queued = static_cast<int32>(Account->queue.size());
    for (const pe::QueueItem& Item : Account->queue) Snapshot.queuedRiflemen += Item.recipe == pe::Recipe::Rifleman ? 1 : 0;
    Snapshot.unitCap = static_cast<int32>(politika::unitscale::kMaxUnitsPerSeat);

    // Only its own units are read, in ascending id, whatever order the list holds them in.
    TArray<const APolitikaPrototypeUnit*, TInlineAllocator<32>> Own;
    for (const APolitikaPrototypeUnit* Unit : Units)
        if (IsValid(Unit) && Unit->IsAlive() && Unit->GetOwnerPlayer() == ParticipantId) Own.Add(Unit);
    Own.Sort([](const APolitikaPrototypeUnit& A, const APolitikaPrototypeUnit& B) { return A.GetStableId() < B.GetStableId(); });
    Snapshot.livingUnits = Own.Num();
    for (const APolitikaPrototypeUnit* Unit : Own)
    {
        if (Snapshot.unitCount == Snapshot.units.size()) break;
        const EPolitikaPrototypeOrder Order = Unit->GetCurrentOrder();
        Snapshot.units[Snapshot.unitCount++] = {Unit->GetStableId(), Unit->CanAttack(), !Unit->IsRouted(),
            Order == EPolitikaPrototypeOrder::Idle || Order == EPolitikaPrototypeOrder::Stop};
    }

    // The declared prototype observation: the authored start of the lowest opposing seat. It comes from
    // the map definition, not from any unit, and it is not a sighting.
    if (MapDefinition)
    {
        const FPolitikaPrototypeRuntimeParticipant* Opposing = nullptr;
        for (const FPolitikaPrototypeRuntimeParticipant& Candidate : RuntimeParticipants)
            if (Candidate.TeamId != Seat->TeamId && (!Opposing || Candidate.RuntimePlayerId < Opposing->RuntimePlayerId)) Opposing = &Candidate;
        const FPolitikaPrototypePlayerSlot* Slot = Opposing ? MapDefinition->PlayerSlots.FindByPredicate(
            [Opposing](const FPolitikaPrototypePlayerSlot& Candidate) { return Candidate.SlotId == Opposing->SlotId; }) : nullptr;
        if (Slot)
        {
            Snapshot.hasAdvanceTarget = true;
            Snapshot.advanceTarget = {Slot->StartLocation.X - PlayableMin.X, Slot->StartLocation.Y - PlayableMin.Y};
        }
    }
    return Snapshot;
}

bool APolitikaPrototypeMatch::SubmitOpponentCommand(const FPolitikaNetCommandIntent& Intent)
{
    if (!HasAuthority() || !OpponentBrain.IsSet() || OpponentBrain->bRetired || OpponentBrain->Generation != MatchGeneration) return false;
    return SubmitAuthorizedCommandIntent(nullptr, OpponentBrain->Sequence, OpponentBrain->ParticipantId, false, Intent);
}

bool APolitikaPrototypeMatch::SubmitOpponentEconomy(const FPolitikaNetEconomyIntent& Intent)
{
    if (!HasAuthority() || !OpponentBrain.IsSet() || OpponentBrain->bRetired || OpponentBrain->Generation != MatchGeneration) return false;
    return SubmitAuthorizedEconomyIntent(nullptr, OpponentBrain->Sequence, OpponentBrain->ParticipantId, Intent);
}

void APolitikaPrototypeMatch::RunOpponentDecision()
{
    // The one schedule: this runs once for each fixed step, so a catch-up frame cannot decide twice.
    if (!HasAuthority() || !OpponentBrain.IsSet() || OpponentBrain->bRetired) return;
    const uint64 Step = SimulationStepsExecuted;
    if (!po::IsDecisionStep(Step)) return;
    FOpponentBrain& Brain = *OpponentBrain;
    const po::Snapshot Snapshot = BuildOpponentSnapshot();
    const po::Decision Decision = po::Decide(Snapshot, Brain.Memory);
    const int32 ProtocolVersion = static_cast<int32>(FPolitikaBootstrap::Get().NetworkProtocol());

    const TCHAR* Submission = TEXT("none");
    if (Decision.hasEconomy)
    {
        FPolitikaNetEconomyIntent Intent;
        Intent.ProtocolVersion = ProtocolVersion;
        Intent.Op = static_cast<uint8>(Decision.economyOp);
        Intent.Recipe = Decision.economyOp == pe::EconomyOp::Train ? static_cast<uint8>(Decision.recipe) : 0;
        Intent.ClientSequence = Decision.economySequence;
        Intent.CorrelationId = Decision.economySequence;
        Intent.MatchGeneration = Brain.Generation;
        SubmitOpponentEconomy(Intent);
        // A refusal at intake has already come back through the outcome path.
        Submission = Brain.Memory.pendingEconomy == Decision.economySequence ? TEXT("submitted") : TEXT("refused_at_intake");
    }
    for (size_t Index = 0; Index < Decision.orderCount; ++Index)
    {
        FPolitikaNetCommandIntent Intent;
        Intent.ProtocolVersion = ProtocolVersion;
        const politika::rules::Order Proposed = politika::rules::AttackMoveOrder{Decision.target};
        Intent.OrderKind = static_cast<uint8>(Proposed.index());
        Intent.DestinationX = Decision.target.x;
        Intent.DestinationY = Decision.target.y;
        Intent.IssuerEntityId = Decision.orders[Index].issuer;
        Intent.ClientSequence = Decision.orders[Index].sequence;
        Intent.CorrelationId = Decision.orders[Index].sequence;
        Intent.MatchGeneration = Brain.Generation;
        SubmitOpponentCommand(Intent);
        Submission = TEXT("submitted");
    }

#if !UE_BUILD_SHIPPING
    FString Candidate = TEXT("none");
    if (Decision.hasEconomy)
    {
        static const TCHAR* const OpNames[] = {TEXT("train"), TEXT("cancel"), TEXT("buy_food"), TEXT("sell_gold"), TEXT("sell_oil"), TEXT("build_plant")};
        Candidate = OpNames[static_cast<int32>(Decision.economyOp)];
        if (Decision.economyOp == pe::EconomyOp::Train) Candidate += TEXT(":rifleman");
    }
    else if (Decision.orderCount > 0)
    {
        Candidate = TEXT("attack_move:");
        for (size_t Index = 0; Index < Decision.orderCount; ++Index)
            Candidate += FString::Printf(TEXT("%s%llu"), Index == 0 ? TEXT("") : TEXT("+"), Decision.orders[Index].issuer);
    }
    const uint32 FirstSequence = Decision.hasEconomy ? Decision.economySequence : Decision.orderCount > 0 ? Decision.orders[0].sequence : 0;
    const uint32 LastSequence = Decision.hasEconomy ? Decision.economySequence : Decision.orderCount > 0 ? Decision.orders[Decision.orderCount - 1].sequence : 0;
    int32 Riflemen = 0;
    for (size_t Index = 0; Index < Snapshot.unitCount; ++Index) Riflemen += Snapshot.units[Index].rifleman ? 1 : 0;
    UE_LOG(LogPolitikaStartup, Display,
        TEXT("RTS_OPPONENT_DECISION participant=%u generation=%llu step=%llu brain=%u.%llu observation=own_account+own_units target_source=%s branch=%s reason=%s candidate=%s sequence_first=%u sequence_last=%u submission=%s recover_until=%llu next_step=%llu available=%lld rations=%lld gold=%lld powered=%d shortage=%d queued=%d living=%d riflemen=%d target_x=%.1f target_y=%.1f"),
        Brain.ParticipantId, Brain.Generation, Step, Brain.ParticipantId, Brain.Generation,
        Snapshot.hasAdvanceTarget ? TEXT("declared_prototype_start") : TEXT("none"),
        UTF8_TO_TCHAR(po::BranchName(Decision.branch).data()), UTF8_TO_TCHAR(po::ReasonName(Decision.reason).data()), *Candidate,
        FirstSequence, LastSequence, Submission, Decision.recoverUntil, Decision.nextDecisionStep,
        Snapshot.available, Snapshot.rations, Snapshot.goldHeld, Snapshot.producerPowered ? 1 : 0, Snapshot.foodShortage ? 1 : 0,
        Snapshot.queued, Snapshot.livingUnits, Riflemen, Snapshot.advanceTarget.x, Snapshot.advanceTarget.y);
#endif
    if (Decision.branch == po::Branch::Retired)
        RetireOpponentBrain(UTF8_TO_TCHAR(po::ReasonName(Decision.reason).data()));
}

void APolitikaPrototypeMatch::RetireOpponentBrain(const TCHAR* Reason)
{
    if (!OpponentBrain.IsSet() || OpponentBrain->bRetired) return;
    FOpponentBrain& Brain = *OpponentBrain;
    Brain.bRetired = true;
    Brain.Memory.retired = true;
    const uint32 ParticipantId = Brain.ParticipantId;
    // Only its own requests: another participant's queue is that participant's business.
    const int32 ClearedEconomy = PendingEconomyRequests.RemoveAll(
        [ParticipantId](const FPendingEconomyRequest& Request) { return Request.ParticipantId == ParticipantId; });
    const int32 ClearedOrders = PendingOrders.RemoveAll(
        [ParticipantId](const FPendingOrder& Pending) { return Pending.Request.participant_id == ParticipantId; });
#if !UE_BUILD_SHIPPING
    UE_LOG(LogPolitikaStartup, Display,
        TEXT("RTS_OPPONENT_RETIRED participant=%u generation=%llu step=%llu reason=%s cleared_economy=%d cleared_orders=%d unresolved_economy=%d unresolved_orders=%d next_sequence=%u"),
        ParticipantId, Brain.Generation, SimulationStepsExecuted, Reason, ClearedEconomy, ClearedOrders,
        Brain.Memory.pendingEconomy != 0 ? 1 : 0, static_cast<int32>(Brain.Memory.pendingOrderCount), Brain.Memory.nextSequence);
#else
    (void)Reason; (void)ClearedEconomy; (void)ClearedOrders;
#endif
}

void APolitikaPrototypeMatch::NoteOpponentCommandOutcome(const politika::rules::CommandOutcomeEvent& Outcome, uint32 Sequence)
{
    if (!OpponentBrain.IsSet() || OpponentBrain->bRetired) return;
    po::NoteOrderOutcome(OpponentBrain->Memory, Sequence, Outcome.rejection, Outcome.step);
#if !UE_BUILD_SHIPPING
    UE_LOG(LogPolitikaStartup, Display,
        TEXT("RTS_OPPONENT_OUTCOME participant=%u generation=%llu step=%llu kind=command sequence=%u issuer=%llu status=%s rejection=%u server_sequence=%llu"),
        OpponentBrain->ParticipantId, OpponentBrain->Generation, Outcome.step, Sequence, Outcome.issuer_id,
        Outcome.accepted ? TEXT("accepted") : TEXT("rejected"), static_cast<uint32>(Outcome.rejection), Outcome.sequence);
#endif
}

void APolitikaPrototypeMatch::NoteOpponentEconomyOutcome(const APlayerController* Source, uint32 ParticipantId,
    const FPolitikaNetEconomyIntent& Intent, pe::EconomyRejection Rejection)
{
    // A human whose controller has gone also has no source, so the seat decides.
    if (Source || !IsOpponentSeat(ParticipantId) || OpponentBrain->bRetired) return;
    po::NoteEconomyOutcome(OpponentBrain->Memory, Intent.ClientSequence, Rejection, SimulationStepsExecuted);
#if !UE_BUILD_SHIPPING
    UE_LOG(LogPolitikaStartup, Display,
        TEXT("RTS_OPPONENT_OUTCOME participant=%u generation=%llu step=%llu kind=economy sequence=%u op=%u status=%s rejection=%s"),
        ParticipantId, OpponentBrain->Generation, SimulationStepsExecuted, Intent.ClientSequence, static_cast<uint32>(Intent.Op),
        Rejection == pe::EconomyRejection::None ? TEXT("accepted") : TEXT("rejected"), UTF8_TO_TCHAR(pe::RejectionName(Rejection).data()));
#endif
}

void APolitikaPrototypeMatch::NoteOpponentOrderChanged(const politika::rules::OrderChangedEvent& Changed)
{
    if (!OpponentBrain.IsSet() || OpponentBrain->bRetired) return;
    if (Changed.previous_end != politika::rules::OrderEndReason::NavigationFailed
        && Changed.previous_end != politika::rules::OrderEndReason::Obstructed) return;
    if (!OpponentBrain->Memory.HasAdvanced(Changed.entity_id)) return;
    po::NoteRouteFailure(OpponentBrain->Memory, Changed.entity_id, Changed.step);
#if !UE_BUILD_SHIPPING
    UE_LOG(LogPolitikaStartup, Display,
        TEXT("RTS_OPPONENT_OUTCOME participant=%u generation=%llu step=%llu kind=route unit=%llu status=failed reason=%u recover_until=%llu"),
        OpponentBrain->ParticipantId, OpponentBrain->Generation, Changed.step, Changed.entity_id,
        static_cast<uint32>(Changed.previous_end), OpponentBrain->Memory.recoverUntil);
#endif
}
