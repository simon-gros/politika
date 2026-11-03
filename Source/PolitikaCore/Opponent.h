#pragma once

// The first playable opponent's decision policy (OPP-1, Stage 4D; Docs/RTS_OPPONENT_AI.md).
//
// A pure function of one immutable snapshot and a small memory, with no engine types, world queries,
// clock or random numbers: the same snapshots and outcomes give the same decisions. It proposes at
// most one economy request or one bounded batch of attack-moves per decision. It changes nothing in
// the game: the coordinator submits what it proposes through the same intake as a human's request,
// and those rules decide what happens. A proposal that comes back refused is ordinary input.
//
// The figures below are provisional and await the owner's balance review. None is a balance decision.

#include "Economy.h"
#include "Rules.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <string_view>

namespace politika::opponent {

using Step = economy::Step;
using Sequence = std::uint32_t;
using UnitId = economy::UnitId;

// One decision per simulated second at 20 Hz.
inline constexpr Step kDecisionIntervalSteps = 20;
// About half the opening reserve: long enough to fight on, and early enough to buy before a shortage.
inline constexpr economy::Rations kFoodReserveThreshold = 120;
// One sale lot, so a delivery is sold as soon as it can be.
inline constexpr economy::Gold kGoldSaleThreshold = economy::kSaleLotGold;
// A seat starts with two riflemen: this trains two more and then stops, which is what lets Advance run.
inline constexpr std::int32_t kDesiredRiflemanForce = 4;
inline constexpr std::int32_t kMinimumAdvanceForce = 4;
inline constexpr std::size_t kMaximumOrdersPerDecision = 8;
inline constexpr Step kRecoveryCooldownSteps = 100;           // 5 s
inline constexpr Step kNavigationFailureCooldownSteps = 200;  // 10 s
inline constexpr Step kPendingOutcomeTimeoutSteps = 40;       // 2 s
// What one snapshot holds. Far above the small map's seat cap of 20; lowest ids first if ever exceeded.
inline constexpr std::size_t kMaxSnapshotUnits = 64;

enum class Branch : std::uint8_t { Retired, Recover, Provision, Sell, Produce, Advance, Idle };

[[nodiscard]] constexpr std::string_view BranchName(Branch branch) noexcept {
    switch (branch) {
        case Branch::Retired: return "retired";
        case Branch::Recover: return "recover";
        case Branch::Provision: return "provision";
        case Branch::Sell: return "sell";
        case Branch::Produce: return "produce";
        case Branch::Advance: return "advance";
        case Branch::Idle: return "idle";
    }
    return "unknown";
}

enum class Reason : std::uint8_t {
    MatchInactive,
    StaleGeneration,
    AuthorityInvalid,
    RejectionCooldown,
    NavigationCooldown,
    FoodBelowThreshold,
    GoldHeld,
    ForceBelowTarget,
    ForceReady,
    AwaitingOutcome,
    NoAffordableProvision,
    NoProducer,
    PowerUnavailable,
    FoodShortage,
    CannotProvisionForce,
    QueueFull,
    UnitCapReached,
    InsufficientFunds,
    AwaitingProduction,
    NoValidTarget,
    AlreadyAdvancing,
    InsufficientReadyRiflemen,
};

[[nodiscard]] constexpr std::string_view ReasonName(Reason reason) noexcept {
    switch (reason) {
        case Reason::MatchInactive: return "match_inactive";
        case Reason::StaleGeneration: return "stale_generation";
        case Reason::AuthorityInvalid: return "authority_invalid";
        case Reason::RejectionCooldown: return "rejection_cooldown";
        case Reason::NavigationCooldown: return "navigation_cooldown";
        case Reason::FoodBelowThreshold: return "food_below_threshold";
        case Reason::GoldHeld: return "gold_held";
        case Reason::ForceBelowTarget: return "force_below_target";
        case Reason::ForceReady: return "force_ready";
        case Reason::AwaitingOutcome: return "awaiting_outcome";
        case Reason::NoAffordableProvision: return "no_affordable_provision";
        case Reason::NoProducer: return "no_producer";
        case Reason::PowerUnavailable: return "power_unavailable";
        case Reason::FoodShortage: return "food_shortage";
        case Reason::CannotProvisionForce: return "cannot_provision_force";
        case Reason::QueueFull: return "queue_full";
        case Reason::UnitCapReached: return "unit_cap_reached";
        case Reason::InsufficientFunds: return "insufficient_funds";
        case Reason::AwaitingProduction: return "awaiting_production";
        case Reason::NoValidTarget: return "no_valid_target";
        case Reason::AlreadyAdvancing: return "already_advancing";
        case Reason::InsufficientReadyRiflemen: return "insufficient_ready_riflemen";
    }
    return "unknown";
}

// One of the opponent's own living units. Nothing about any other participant's unit is representable.
struct OwnedUnit {
    UnitId id = 0;
    bool rifleman = false;
    bool operational = true; // False while routed: it refuses every order until it rallies.
    bool atRest = false;     // Idle, or stopped by a failed route.
};

// Everything one decision may read, built once from authoritative state and not changed afterwards.
struct Snapshot {
    economy::ParticipantId participant = 0;
    std::uint64_t generation = 0;
    Step step = 0;
    bool active = false;
    bool authorityValid = false;

    // Its own account.
    economy::Cents available = 0;
    economy::Rations rations = 0;
    bool foodShortage = false;
    economy::Gold goldHeld = 0;
    bool hasProducer = false;
    bool producerPowered = false;
    std::int32_t queued = 0;
    std::int32_t queuedRiflemen = 0;
    std::int32_t livingUnits = 0;
    std::int32_t unitCap = 0;

    // Its own units, ascending id.
    std::array<OwnedUnit, kMaxSnapshotUnits> units{};
    std::size_t unitCount = 0;

    // A declared prototype observation, not a sighting: the authored start of the opposing seat.
    bool hasAdvanceTarget = false;
    rules::Position advanceTarget{};
};

struct PendingOrder {
    Sequence sequence = 0;
    UnitId issuer = 0;
};

// What the policy keeps between decisions. Owned by one brain and discarded with it, so it can never
// reach another match generation. Unit ids only, never an actor.
struct Memory {
    std::uint64_t generation = 0;
    bool retired = false;
    Sequence nextSequence = 1;
    Step lastDecisionStep = 0;
    bool decided = false;
    Branch lastBranch = Branch::Idle;
    Step recoverUntil = 0;
    Reason recoverReason = Reason::RejectionCooldown;
    // The last refusal, as the rules named it.
    economy::EconomyRejection lastEconomyRejection = economy::EconomyRejection::None;
    rules::OrderRejection lastOrderRejection = rules::OrderRejection::None;
    Sequence pendingEconomy = 0;
    Step pendingEconomyStep = 0;
    std::array<PendingOrder, kMaximumOrdersPerDecision> pendingOrders{};
    std::size_t pendingOrderCount = 0;
    Step pendingOrdersStep = 0;
    // Units already sent on the advance, so one that arrives is not sent again.
    std::array<UnitId, kMaxSnapshotUnits> advanced{};
    std::size_t advancedCount = 0;
    std::uint32_t lastAdvanceBatch = 0;

    [[nodiscard]] constexpr bool HasAdvanced(UnitId unit) const noexcept {
        for (std::size_t index = 0; index < advancedCount; ++index)
            if (advanced[index] == unit) return true;
        return false;
    }

    constexpr bool ForgetAdvanced(UnitId unit) noexcept {
        for (std::size_t index = 0; index < advancedCount; ++index)
            if (advanced[index] == unit) {
                advanced[index] = advanced[--advancedCount];
                advanced[advancedCount] = 0;
                return true;
            }
        return false;
    }
};

struct ProposedOrder {
    UnitId issuer = 0;
    Sequence sequence = 0;
};

struct Decision {
    Branch branch = Branch::Idle;
    Reason reason = Reason::AwaitingOutcome;
    Step step = 0;
    bool hasEconomy = false;
    economy::EconomyOp economyOp = economy::EconomyOp::Train;
    economy::Recipe recipe = economy::Recipe::Rifleman;
    Sequence economySequence = 0;
    std::array<ProposedOrder, kMaximumOrdersPerDecision> orders{};
    std::size_t orderCount = 0;
    rules::Position target{};
    Step recoverUntil = 0;
    Step nextDecisionStep = 0;
};

[[nodiscard]] constexpr bool IsDecisionStep(Step step) noexcept {
    return step != 0 && step % kDecisionIntervalSteps == 0;
}

// The seat a -PolitikaMPOpponentSeat= value names, or zero: digits only, in range, not zero.
[[nodiscard]] constexpr economy::ParticipantId ParseOpponentSeat(std::string_view text) noexcept {
    if (text.empty()) return 0;
    std::uint64_t value = 0;
    for (const char digit : text) {
        if (digit < '0' || digit > '9') return 0;
        value = value * 10 + static_cast<std::uint64_t>(digit - '0');
        if (value > std::numeric_limits<economy::ParticipantId>::max()) return 0;
    }
    return static_cast<economy::ParticipantId>(value);
}

constexpr void BeginRecovery(Memory& memory, Step until, Reason reason) noexcept {
    memory.recoverUntil = std::max(memory.recoverUntil, until);
    memory.recoverReason = reason;
}

// The authoritative result of a proposed economy request. A refusal starts a bounded cooldown.
constexpr void NoteEconomyOutcome(Memory& memory, Sequence sequence, economy::EconomyRejection rejection, Step step) noexcept {
    if (sequence == 0 || memory.pendingEconomy != sequence) return;
    memory.pendingEconomy = 0;
    memory.lastEconomyRejection = rejection;
    if (rejection != economy::EconomyRejection::None) BeginRecovery(memory, step + kRecoveryCooldownSteps, Reason::RejectionCooldown);
}

// The authoritative result of one proposed order. A refused unit can be sent again after the cooldown.
constexpr void NoteOrderOutcome(Memory& memory, Sequence sequence, rules::OrderRejection rejection, Step step) noexcept {
    for (std::size_t index = 0; index < memory.pendingOrderCount; ++index) {
        if (memory.pendingOrders[index].sequence != sequence) continue;
        const UnitId issuer = memory.pendingOrders[index].issuer;
        memory.pendingOrders[index] = memory.pendingOrders[--memory.pendingOrderCount];
        memory.pendingOrders[memory.pendingOrderCount] = {};
        memory.lastOrderRejection = rejection;
        if (rejection != rules::OrderRejection::None) {
            memory.ForgetAdvanced(issuer);
            BeginRecovery(memory, step + kRecoveryCooldownSteps, Reason::RejectionCooldown);
        }
        return;
    }
}

// An admitted order ended without arriving because its route failed or was obstructed.
constexpr void NoteRouteFailure(Memory& memory, UnitId unit, Step step) noexcept {
    if (memory.ForgetAdvanced(unit)) BeginRecovery(memory, step + kNavigationFailureCooldownSteps, Reason::NavigationCooldown);
}

[[nodiscard]] constexpr Memory NewMemory(std::uint64_t generation) noexcept {
    Memory memory;
    memory.generation = generation;
    return memory;
}

// One evaluation; the first branch that applies wins. Call on a decision step only: the coordinator
// owns the clock. A second call for the same step proposes nothing.
[[nodiscard]] constexpr Decision Decide(const Snapshot& snapshot, Memory& memory) noexcept {
    Decision decision;
    decision.step = snapshot.step;
    decision.nextDecisionStep = snapshot.step + kDecisionIntervalSteps;
    const auto finish = [&](Branch branch, Reason reason) {
        decision.branch = branch;
        decision.reason = reason;
        decision.recoverUntil = memory.recoverUntil;
        memory.lastBranch = branch;
        return decision;
    };

    if (!snapshot.active || !snapshot.authorityValid || snapshot.generation != memory.generation || memory.retired) {
        memory.retired = true;
        return finish(Branch::Retired, !snapshot.active ? Reason::MatchInactive
            : snapshot.generation != memory.generation ? Reason::StaleGeneration : Reason::AuthorityInvalid);
    }
    if (memory.decided && memory.lastDecisionStep == snapshot.step) return finish(Branch::Idle, Reason::AwaitingOutcome);
    memory.decided = true;
    memory.lastDecisionStep = snapshot.step;

    // A request whose outcome never came counts as a failure, so it cannot hold the policy forever.
    if (memory.pendingEconomy != 0 && snapshot.step >= memory.pendingEconomyStep + kPendingOutcomeTimeoutSteps) {
        memory.pendingEconomy = 0;
        BeginRecovery(memory, snapshot.step + kRecoveryCooldownSteps, Reason::RejectionCooldown);
    }
    if (memory.pendingOrderCount != 0 && snapshot.step >= memory.pendingOrdersStep + kPendingOutcomeTimeoutSteps) {
        for (std::size_t index = 0; index < memory.pendingOrderCount; ++index) {
            memory.ForgetAdvanced(memory.pendingOrders[index].issuer);
            memory.pendingOrders[index] = {};
        }
        memory.pendingOrderCount = 0;
        BeginRecovery(memory, snapshot.step + kRecoveryCooldownSteps, Reason::RejectionCooldown);
    }

    if (snapshot.step < memory.recoverUntil) return finish(Branch::Recover, memory.recoverReason);
    if (memory.pendingEconomy != 0 || memory.pendingOrderCount != 0) return finish(Branch::Idle, Reason::AwaitingOutcome);

    const auto propose = [&](economy::EconomyOp op, Branch branch, Reason reason) {
        decision.hasEconomy = true;
        decision.economyOp = op;
        decision.economySequence = memory.nextSequence++;
        memory.pendingEconomy = decision.economySequence;
        memory.pendingEconomyStep = snapshot.step;
        return finish(branch, reason);
    };

    const bool foodLow = snapshot.rations < kFoodReserveThreshold;
    if (foodLow && snapshot.available >= economy::kFoodPurchaseCents
        && snapshot.rations + economy::kFoodPurchaseRations <= economy::kFoodCapacityRations)
        return propose(economy::EconomyOp::BuyFood, Branch::Provision, Reason::FoodBelowThreshold);

    if (snapshot.goldHeld >= kGoldSaleThreshold) return propose(economy::EconomyOp::SellGold, Branch::Sell, Reason::GoldHeld);

    std::int32_t riflemen = 0;
    for (std::size_t index = 0; index < snapshot.unitCount; ++index) riflemen += snapshot.units[index].rifleman ? 1 : 0;
    const economy::Cents riflemanCost = economy::FindRecipe(static_cast<std::uint8_t>(economy::Recipe::Rifleman))->cost;
    // The force the policy wants is below the cap, so production ends and Advance can run. A loss
    // brings Produce back.
    const bool wantsRifleman = riflemen + snapshot.queuedRiflemen < kDesiredRiflemanForce;
    Reason produceBlock = Reason::AwaitingProduction;
    if (wantsRifleman) {
        const std::int64_t forceAfter = std::int64_t{snapshot.livingUnits} + snapshot.queued + 1;
        if (!snapshot.hasProducer) produceBlock = Reason::NoProducer;
        else if (!snapshot.producerPowered) produceBlock = Reason::PowerUnavailable;
        else if (snapshot.foodShortage) produceBlock = Reason::FoodShortage;
        else if (snapshot.queued >= static_cast<std::int32_t>(economy::kMaxQueue)) produceBlock = Reason::QueueFull;
        else if (forceAfter > snapshot.unitCap) produceBlock = Reason::UnitCapReached;
        else if (snapshot.rations < forceAfter * economy::kRationsPerUnit) produceBlock = Reason::CannotProvisionForce;
        else if (snapshot.available < riflemanCost) produceBlock = Reason::InsufficientFunds;
        else {
            decision.recipe = economy::Recipe::Rifleman;
            return propose(economy::EconomyOp::Train, Branch::Produce, Reason::ForceBelowTarget);
        }
    }

    // Reinforcements join a force that is already big enough; otherwise it waits until it is.
    std::array<UnitId, kMaxSnapshotUnits> ready{};
    std::size_t readyCount = 0;
    std::int32_t advancing = 0;
    for (std::size_t index = 0; index < snapshot.unitCount; ++index) {
        const OwnedUnit& unit = snapshot.units[index];
        if (!unit.rifleman) continue;
        if (memory.HasAdvanced(unit.id)) ++advancing;
        else if (unit.operational && unit.atRest) ready[readyCount++] = unit.id;
    }
    const bool canAdvance = readyCount != 0 && snapshot.queuedRiflemen == 0
        && static_cast<std::int32_t>(readyCount) + advancing >= kMinimumAdvanceForce;
    if (canAdvance && snapshot.hasAdvanceTarget) {
        // Forget units that have died, so the set can only hold living ones.
        for (std::size_t index = memory.advancedCount; index > 0; --index) {
            const UnitId held = memory.advanced[index - 1];
            bool living = false;
            for (std::size_t unit = 0; unit < snapshot.unitCount; ++unit) living = living || snapshot.units[unit].id == held;
            if (!living) memory.ForgetAdvanced(held);
        }
        decision.target = snapshot.advanceTarget;
        for (std::size_t index = 0; index < readyCount && decision.orderCount < kMaximumOrdersPerDecision; ++index) {
            const Sequence sequence = memory.nextSequence++;
            decision.orders[decision.orderCount++] = {ready[index], sequence};
            memory.pendingOrders[memory.pendingOrderCount++] = {sequence, ready[index]};
            memory.advanced[memory.advancedCount++] = ready[index];
        }
        memory.pendingOrdersStep = snapshot.step;
        ++memory.lastAdvanceBatch;
        return finish(Branch::Advance, Reason::ForceReady);
    }

    if (foodLow) return finish(Branch::Idle, Reason::NoAffordableProvision);
    if (wantsRifleman) return finish(Branch::Idle, produceBlock);
    if (snapshot.queuedRiflemen != 0) return finish(Branch::Idle, Reason::AwaitingProduction);
    if (canAdvance) return finish(Branch::Idle, Reason::NoValidTarget);
    if (readyCount == 0 && advancing > 0) return finish(Branch::Idle, Reason::AlreadyAdvancing);
    return finish(Branch::Idle, Reason::InsufficientReadyRiflemen);
}

} // namespace politika::opponent
