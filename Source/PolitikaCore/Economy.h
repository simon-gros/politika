#pragma once

// Participant treasury, authorized income and dollar-priced production (ECON-E1, the first
// Stage 4B sub-slice; RTS_OPERATIONAL_ECONOMY_AND_POWER.md §6 and §8,
// RTS_PRODUCTION_AND_TECHNOLOGY.md §1 and §2).
//
// One ledger owned by the match coordinator, advanced only by its fixed simulation step. Money
// is integer US cents with checked arithmetic. Training debits the whole price exactly once at
// admission, and a rejected request changes nothing. Time is counted in authoritative steps.
// ECON-E2 adds one electricity grid per account (Power.h): a producer below its minimum load takes
// no new items and its queue pauses, without another charge. Food is a later sub-slice.
//
// The figures below are prototype fixture data, not balance decisions. The rifleman's price and
// training time follow the line-infantry entry of Docs/Units/ATLANTIC_CONCORD_UNITS.md.

#include "Power.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace politika::economy {

using Cents = std::int64_t;
using ParticipantId = std::uint32_t;
using TransactionId = std::uint32_t;
using Step = std::uint64_t;

inline constexpr Cents kStartingCents = 100'000;          // $1,000.00
inline constexpr Cents kIncomeCents = 2'000;              // $20.00
inline constexpr std::uint32_t kIncomeIntervalSteps = 200; // 10 s at 20 Hz
inline constexpr std::size_t kMaxQueue = 5;
// Requests waiting for the next step, per participant; more are refused as RateLimited.
inline constexpr std::size_t kMaxPendingRequests = 8;
// How often a finished item whose output is blocked looks for an open point again (1 s).
inline constexpr std::uint32_t kBlockedRetrySteps = 20;
// The fixed starting base's two buildings in each account's grid.
inline constexpr power::BuildingId kGeneratorBuilding = 1;
inline constexpr power::BuildingId kProducerBuilding = 2;

// Wire values are stable.
enum class Recipe : std::uint8_t { Rifleman };
inline constexpr std::uint8_t kRecipeCount = 1;

struct RecipeDefinition {
    Cents cost = 0;
    std::uint32_t durationSteps = 0;
};

[[nodiscard]] constexpr std::optional<RecipeDefinition> FindRecipe(std::uint8_t recipe) noexcept {
    if (recipe == static_cast<std::uint8_t>(Recipe::Rifleman)) return RecipeDefinition{36'000, 200}; // $360.00, 10 s
    return std::nullopt;
}

// Wire values are stable: Train is the default of a zeroed request.
enum class EconomyOp : std::uint8_t { Train, Cancel };
inline constexpr std::uint8_t kEconomyOpCount = 2;

enum class EconomyRejection : std::uint8_t {
    None,
    MalformedCommand,
    DuplicateCommand,
    Unauthorized,
    MatchInactive,
    StaleMatch,
    UnknownRecipe,
    NoProducer,
    InsufficientFunds,
    QueueFull,
    UnitCap,
    NothingToCancel,
    RateLimited,
    Overflow,
    ProducerUnpowered,
};

[[nodiscard]] constexpr std::string_view RejectionName(EconomyRejection rejection) noexcept {
    switch (rejection) {
        case EconomyRejection::None: return "none";
        case EconomyRejection::MalformedCommand: return "malformed";
        case EconomyRejection::DuplicateCommand: return "duplicate";
        case EconomyRejection::Unauthorized: return "unauthorized";
        case EconomyRejection::MatchInactive: return "match_inactive";
        case EconomyRejection::StaleMatch: return "stale_match";
        case EconomyRejection::UnknownRecipe: return "unknown_recipe";
        case EconomyRejection::NoProducer: return "no_producer";
        case EconomyRejection::InsufficientFunds: return "insufficient_funds";
        case EconomyRejection::QueueFull: return "queue_full";
        case EconomyRejection::UnitCap: return "unit_cap";
        case EconomyRejection::NothingToCancel: return "nothing_to_cancel";
        case EconomyRejection::RateLimited: return "rate_limited";
        case EconomyRejection::Overflow: return "overflow";
        case EconomyRejection::ProducerUnpowered: return "producer_unpowered";
    }
    return "unknown";
}

[[nodiscard]] constexpr bool CheckedAdd(Cents a, Cents b, Cents& out) noexcept {
    if (b > 0 && a > std::numeric_limits<Cents>::max() - b) return false;
    if (b < 0 && a < std::numeric_limits<Cents>::min() - b) return false;
    out = a + b;
    return true;
}

[[nodiscard]] constexpr bool CheckedSub(Cents a, Cents b, Cents& out) noexcept {
    if (b < 0 && a > std::numeric_limits<Cents>::max() + b) return false;
    if (b > 0 && a < std::numeric_limits<Cents>::min() + b) return false;
    out = a - b;
    return true;
}

// "$1,234.50" for the HUD and logs; never used for arithmetic.
[[nodiscard]] inline std::string FormatUsd(Cents cents) {
    const bool negative = cents < 0;
    // Through unsigned so the most negative value does not overflow.
    const std::uint64_t magnitude = negative ? std::uint64_t{0} - static_cast<std::uint64_t>(cents) : static_cast<std::uint64_t>(cents);
    std::string whole = std::to_string(magnitude / 100);
    for (std::size_t position = whole.size(); position > 3; position -= 3) whole.insert(position - 3, 1, ',');
    const std::uint64_t fraction = magnitude % 100;
    return std::string(negative ? "-$" : "$") + whole + "." + (fraction < 10 ? "0" : "") + std::to_string(fraction);
}

// Shape check only, before any authoritative state is read: Cancel carries no recipe.
[[nodiscard]] constexpr EconomyRejection CheckIntentShape(std::uint8_t op, std::uint8_t recipe) noexcept {
    if (op >= kEconomyOpCount) return EconomyRejection::MalformedCommand;
    if (static_cast<EconomyOp>(op) == EconomyOp::Cancel && recipe != 0) return EconomyRejection::MalformedCommand;
    return EconomyRejection::None;
}

struct QueueItem {
    TransactionId transaction = 0;
    Recipe recipe = Recipe::Rifleman;
    Cents paid = 0;
    std::uint32_t remainingSteps = 0;
    bool blocked = false;
    Step nextRetryStep = 0;
};

struct Account {
    ParticipantId participant = 0;
    bool hasProducer = false;
    Cents starting = 0;
    Cents available = 0;
    Cents credited = 0;
    Cents spent = 0;
    Cents refunded = 0;
    Step nextIncomeStep = 0;
    TransactionId lastTransaction = 0;
    std::uint32_t produced = 0;
    std::vector<QueueItem> queue;
    power::Grid grid;
    power::Allocation allocation;
    bool producerPowered = false;

    // Every cent has one explanation: nothing is created or lost outside the ledger.
    [[nodiscard]] bool Reconciles() const noexcept {
        Cents in = 0;
        Cents out = 0;
        return available >= 0 && CheckedAdd(starting, credited, in) && CheckedAdd(in, refunded, in)
            && CheckedAdd(available, spent, out) && in == out;
    }
};

struct RequestResult {
    EconomyRejection rejection = EconomyRejection::None;
    Cents amount = 0;
    Cents available = 0;
    std::size_t queued = 0;
    [[nodiscard]] explicit operator bool() const noexcept { return rejection == EconomyRejection::None; }
};

// What one participant's account did in one step.
struct StepResult {
    Cents income = 0;
    bool incomeOverflow = false;
    // The head item finished training and wants to appear; see CompleteHead and BlockHead.
    bool spawnDue = false;
    TransactionId transaction = 0;
    Recipe recipe = Recipe::Rifleman;
    // This step's allocation differs from the last one. headRemainingSteps is the head item's
    // remaining time before this step's production.
    bool powerChanged = false;
    std::uint32_t headRemainingSteps = 0;
};

class Ledger {
public:
    void Clear() noexcept { accounts.clear(); }

    // firstIncomeStep is the first step that may pay income. Refuses a zero or repeated
    // participant and a negative start.
    bool AddParticipant(ParticipantId participant, Cents starting, bool hasProducer, Step firstIncomeStep) {
        if (participant == 0 || starting < 0 || Find(participant)) return false;
        Account account;
        account.participant = participant;
        account.hasProducer = hasProducer;
        account.starting = starting;
        account.available = starting;
        account.nextIncomeStep = firstIncomeStep;
        if (hasProducer) {
            // A self-starting plant and one producer: the base is powered from its first step.
            account.grid.Add({kGeneratorBuilding, power::BuildingKind::Generator, power::kGeneratorAuxiliaryKw, power::kGeneratorCapacityKw});
            account.grid.Add({kProducerBuilding, power::BuildingKind::Producer, power::kProducerLoadKw, 0});
            account.allocation = account.grid.Allocate();
            account.producerPowered = account.grid.Find(kProducerBuilding)->powered;
        }
        const auto position = std::lower_bound(accounts.begin(), accounts.end(), participant,
            [](const Account& candidate, ParticipantId id) { return candidate.participant < id; });
        accounts.insert(position, std::move(account));
        return true;
    }

    [[nodiscard]] const Account* Find(ParticipantId participant) const noexcept {
        const auto found = std::find_if(accounts.begin(), accounts.end(),
            [participant](const Account& candidate) { return candidate.participant == participant; });
        return found == accounts.end() ? nullptr : &*found;
    }

    // Takes effect at the participant's next Advance. Returns false without a generator.
    bool SetGeneratorOnline(ParticipantId participant, bool online) noexcept {
        Account* account = FindMutable(participant);
        return account && account->grid.SetGenerating(kGeneratorBuilding, online);
    }

    // Stable ascending participant order, for deterministic per-step processing.
    [[nodiscard]] const std::vector<Account>& All() const noexcept { return accounts; }

    // Admits one training request: validates in order, then debits the full price once and adds
    // one queue entry. livingUnits counts the participant's units alive now; queued items count
    // towards unitCap so an admitted item can always appear.
    RequestResult Train(ParticipantId participant, TransactionId transaction, std::uint8_t recipe,
        std::int64_t livingUnits, std::int64_t unitCap) {
        Account* account = FindMutable(participant);
        if (!account) return Reject(EconomyRejection::Unauthorized, nullptr);
        if (transaction <= account->lastTransaction) return Reject(EconomyRejection::DuplicateCommand, account);
        const std::optional<RecipeDefinition> definition = FindRecipe(recipe);
        if (!definition) return Reject(EconomyRejection::UnknownRecipe, account);
        if (!account->hasProducer) return Reject(EconomyRejection::NoProducer, account);
        if (!account->producerPowered) return Reject(EconomyRejection::ProducerUnpowered, account);
        if (account->queue.size() >= kMaxQueue) return Reject(EconomyRejection::QueueFull, account);
        if (livingUnits + static_cast<std::int64_t>(account->queue.size()) >= unitCap)
            return Reject(EconomyRejection::UnitCap, account);
        if (account->available < definition->cost) return Reject(EconomyRejection::InsufficientFunds, account);
        Cents available = 0;
        Cents spent = 0;
        if (!CheckedSub(account->available, definition->cost, available) || !CheckedAdd(account->spent, definition->cost, spent))
            return Reject(EconomyRejection::Overflow, account);
        account->available = available;
        account->spent = spent;
        account->lastTransaction = transaction;
        account->queue.push_back({transaction, static_cast<Recipe>(recipe), definition->cost, definition->durationSteps});
        return {EconomyRejection::None, definition->cost, account->available, account->queue.size()};
    }

    // Cancels the newest queue entry, in training or not, and refunds exactly what it paid.
    RequestResult CancelLast(ParticipantId participant, TransactionId transaction) {
        Account* account = FindMutable(participant);
        if (!account) return Reject(EconomyRejection::Unauthorized, nullptr);
        if (transaction <= account->lastTransaction) return Reject(EconomyRejection::DuplicateCommand, account);
        if (account->queue.empty()) return Reject(EconomyRejection::NothingToCancel, account);
        const Cents paid = account->queue.back().paid;
        Cents available = 0;
        Cents refunded = 0;
        if (!CheckedAdd(account->available, paid, available) || !CheckedAdd(account->refunded, paid, refunded))
            return Reject(EconomyRejection::Overflow, account);
        account->available = available;
        account->refunded = refunded;
        account->lastTransaction = transaction;
        account->queue.pop_back();
        return {EconomyRejection::None, paid, account->available, account->queue.size()};
    }

    // One authoritative step for one account: pays income that is due, allocates power, then
    // advances only the head of the queue. An item admitted earlier in the same step does not
    // advance until the next one, so with steady power it appears exactly durationSteps after its
    // admission step. An unpowered producer holds its whole queue: no time passes, a blocked item
    // does not retry and nothing is charged again. Cancel still refunds.
    StepResult Advance(ParticipantId participant, Step step, TransactionId admittedThisStep = 0) {
        StepResult result;
        Account* account = FindMutable(participant);
        if (!account) return result;
        if (step >= account->nextIncomeStep) {
            Cents available = 0;
            Cents credited = 0;
            if (CheckedAdd(account->available, kIncomeCents, available) && CheckedAdd(account->credited, kIncomeCents, credited)) {
                account->available = available;
                account->credited = credited;
                result.income = kIncomeCents;
            } else {
                result.incomeOverflow = true;
            }
            account->nextIncomeStep = step + kIncomeIntervalSteps;
        }
        if (!account->grid.Empty()) {
            const power::Allocation allocation = account->grid.Allocate();
            const bool powered = account->grid.Find(kProducerBuilding)->powered;
            result.powerChanged = powered != account->producerPowered || allocation.suppliedKw != account->allocation.suppliedKw
                || allocation.requiredKw != account->allocation.requiredKw;
            account->allocation = allocation;
            account->producerPowered = powered;
        }
        if (account->queue.empty()) return result;
        QueueItem& head = account->queue.front();
        result.headRemainingSteps = head.remainingSteps;
        if (!account->producerPowered) return result;
        if (head.blocked) {
            if (step < head.nextRetryStep) return result;
        } else {
            if (head.transaction == admittedThisStep) return result;
            if (head.remainingSteps > 0) --head.remainingSteps;
            if (head.remainingSteps > 0) return result;
        }
        result.spawnDue = true;
        result.transaction = head.transaction;
        result.recipe = head.recipe;
        return result;
    }

    // The due head item appeared. No further money moves: it was paid for at admission.
    bool CompleteHead(ParticipantId participant, TransactionId transaction) {
        Account* account = FindMutable(participant);
        if (!account || account->queue.empty() || account->queue.front().transaction != transaction) return false;
        account->queue.erase(account->queue.begin());
        ++account->produced;
        return true;
    }

    // The due head item has no open output point: it keeps its place and its payment, holds the
    // items behind it and tries again later. It is never deleted or charged twice.
    bool BlockHead(ParticipantId participant, TransactionId transaction, Step step) {
        Account* account = FindMutable(participant);
        if (!account || account->queue.empty() || account->queue.front().transaction != transaction) return false;
        account->queue.front().blocked = true;
        account->queue.front().nextRetryStep = step + kBlockedRetrySteps;
        return true;
    }

private:
    std::vector<Account> accounts; // ascending participant

    [[nodiscard]] Account* FindMutable(ParticipantId participant) noexcept {
        const auto found = std::find_if(accounts.begin(), accounts.end(),
            [participant](const Account& candidate) { return candidate.participant == participant; });
        return found == accounts.end() ? nullptr : &*found;
    }

    [[nodiscard]] static RequestResult Reject(EconomyRejection rejection, const Account* account) noexcept {
        RequestResult result;
        result.rejection = rejection;
        if (account) {
            result.available = account->available;
            result.queued = account->queue.size();
        }
        return result;
    }
};

// One offset around a producer for its output, in a fixed order: eight points on a 250 cm ring
// then eight on a 400 cm ring, starting from `towards` (a unit vector) and turning clockwise.
inline constexpr std::size_t kOutputCandidates = 16;

struct Offset {
    double x = 0.0;
    double y = 0.0;
};

[[nodiscard]] inline std::array<Offset, kOutputCandidates> OutputOffsets(Offset towards) noexcept {
    constexpr double kDiagonal = 0.70710678118654752;
    constexpr std::array<std::array<double, 2>, 8> kTurns{{
        {1.0, 0.0}, {kDiagonal, kDiagonal}, {0.0, 1.0}, {-kDiagonal, kDiagonal},
        {-1.0, 0.0}, {-kDiagonal, -kDiagonal}, {0.0, -1.0}, {kDiagonal, -kDiagonal},
    }};
    std::array<Offset, kOutputCandidates> offsets{};
    for (std::size_t ring = 0; ring < 2; ++ring) {
        const double radius = ring == 0 ? 250.0 : 400.0;
        for (std::size_t turn = 0; turn < kTurns.size(); ++turn) {
            const double cosine = kTurns[turn][0];
            const double sine = kTurns[turn][1];
            offsets[ring * kTurns.size() + turn] = {
                radius * (towards.x * cosine - towards.y * sine),
                radius * (towards.x * sine + towards.y * cosine),
            };
        }
    }
    return offsets;
}

} // namespace politika::economy
