#pragma once

// Participant treasury, authorized income and dollar-priced production (ECON-E1, the first
// Stage 4B sub-slice; RTS_OPERATIONAL_ECONOMY_AND_POWER.md §6 and §8,
// RTS_PRODUCTION_AND_TECHNOLOGY.md §1 and §2).
//
// One ledger owned by the match coordinator, advanced only by its fixed simulation step. Money
// is integer US cents with checked arithmetic. Training debits the whole price exactly once at
// admission, and a rejected request changes nothing. Time is counted in authoritative steps.
// ECON-E2 adds one electricity grid per account (Power.h): a producer below its minimum load takes
// no new items and its queue pauses, without another charge. ECON-E3 adds military food: a reserve
// of rations, one upkeep debit per due interval for the living force, and a purchase that debits
// dollars and credits rations atomically. A shortage stops reinforcement until upkeep is paid in
// full again, and never touches the units themselves. ECON-E4 adds a finite gold source, harvesters
// that haul it to the base on their own, and a sale that debits gold and credits net dollars
// atomically. ECON-E5 makes a source typed: a harvester collects gold or food, whichever its player
// last sent it to, and each seat starts with one. ECON-E6 gives the generator and the producer
// health: damaging the generator takes it offline until it is repaired, and destroying the producer
// refunds its queue once. A source is not a building and is not on the grid.
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
// A captured works pays on the same steps as the fixed income (ECON-E28), so holding it never moves
// the base income's schedule. A prototype figure.
inline constexpr Cents kWorksIncomeCents = 1'000;          // $10.00
inline constexpr std::size_t kMaxQueue = 5;
// Requests waiting for the next step, per participant; more are refused as RateLimited.
inline constexpr std::size_t kMaxPendingRequests = 8;
// How often a finished item whose output is blocked looks for an open point again (1 s).
inline constexpr std::uint32_t kBlockedRetrySteps = 20;
// The fixed starting base's two buildings in each account's grid.
inline constexpr power::BuildingId kGeneratorBuilding = 1;
inline constexpr power::BuildingId kProducerBuilding = 2;

using Rations = std::int64_t;
inline constexpr Rations kStartingRations = 240;
inline constexpr Rations kRationsPerUnit = 1;
inline constexpr std::uint32_t kFoodUpkeepIntervalSteps = 200; // 10 s at 20 Hz
inline constexpr Rations kFoodPurchaseRations = 50;
inline constexpr Cents kFoodPurchaseCents = 5'000;             // $50.00
inline constexpr Rations kFoodCapacityRations = 1'000;

using Gold = std::int64_t;
using UnitId = std::uint64_t;
inline constexpr Gold kSourceGold = 500;
inline constexpr Gold kLoadGold = 25;
inline constexpr Rations kSourceFood = 600;
inline constexpr Rations kLoadFood = 30;
// How often a harvester holding food looks for room in a full store again (1 s).
inline constexpr std::uint32_t kHarvesterStoreRetrySteps = 20;
inline constexpr std::uint32_t kHarvesterLoadSteps = 60;   // 3 s at the source
inline constexpr std::uint32_t kHarvesterUnloadSteps = 40; // 2 s at the base
inline constexpr Gold kSaleLotGold = 25;
inline constexpr Cents kGoldRateCents = 250;             // $2.50 per unit of gold
inline constexpr std::int64_t kGoldFeeBasisPoints = 300; // 3%
// ECON-E30: crude oil. Fixture figures, worth less a unit than gold, not balance decisions.
inline constexpr Gold kSourceOil = 400;
inline constexpr Gold kLoadOil = 20;
inline constexpr Gold kSaleLotOil = 20;
inline constexpr Cents kOilRateCents = 150;             // $1.50 per unit of crude
inline constexpr std::int64_t kOilFeeBasisPoints = 300; // 3%

// Wire values are stable.
enum class Recipe : std::uint8_t { Rifleman, Harvester };
inline constexpr std::uint8_t kRecipeCount = 2;

struct RecipeDefinition {
    Cents cost = 0;
    std::uint32_t durationSteps = 0;
};

[[nodiscard]] constexpr std::optional<RecipeDefinition> FindRecipe(std::uint8_t recipe) noexcept {
    if (recipe == static_cast<std::uint8_t>(Recipe::Rifleman)) return RecipeDefinition{36'000, 200}; // $360.00, 10 s
    if (recipe == static_cast<std::uint8_t>(Recipe::Harvester)) return RecipeDefinition{30'000, 300}; // $300.00, 15 s
    return std::nullopt;
}

// Wire values are stable: Train is the default of a zeroed request.
enum class EconomyOp : std::uint8_t { Train, Cancel, BuyFood, SellGold, SellOil };
inline constexpr std::uint8_t kEconomyOpCount = 5;

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
    FoodShortage,
    FoodStoreFull,
    NothingToSell,
    NoResourceSource,
    NoOilToSell,
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
        case EconomyRejection::FoodShortage: return "food_shortage";
        case EconomyRejection::FoodStoreFull: return "food_store_full";
        case EconomyRejection::NothingToSell: return "nothing_to_sell";
        case EconomyRejection::NoResourceSource: return "no_resource_source";
        case EconomyRejection::NoOilToSell: return "no_oil_to_sell";
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

struct SaleQuote {
    Gold quantity = 0;
    Cents gross = 0;
    Cents fee = 0;
    Cents net = 0;
};

// What selling quantity at this rate and fee pays. The fee rounds up to a whole cent, against the
// seller, so splitting a sale can never earn more than selling it whole. Empty for a quantity or
// rate that is not positive, a fee outside 0-100% and a gross that does not fit.
[[nodiscard]] constexpr std::optional<SaleQuote> QuoteSale(Gold quantity, Cents rate, std::int64_t feeBasisPoints) noexcept {
    if (quantity <= 0 || rate <= 0 || feeBasisPoints < 0 || feeBasisPoints > 10'000) return std::nullopt;
    if (quantity > std::numeric_limits<Cents>::max() / rate) return std::nullopt;
    const Cents gross = quantity * rate;
    const Cents fee = gross / 10'000 * feeBasisPoints + (gross % 10'000 * feeBasisPoints + 9'999) / 10'000;
    return SaleQuote{quantity, gross, fee, gross - fee};
}

// The next sale from a holding: one lot, or all that is left if that is less.
[[nodiscard]] constexpr std::optional<SaleQuote> QuoteGoldLot(Gold held) noexcept {
    return QuoteSale(std::min(held, kSaleLotGold), kGoldRateCents, kGoldFeeBasisPoints);
}

[[nodiscard]] constexpr std::optional<SaleQuote> QuoteOilLot(Gold held) noexcept {
    return QuoteSale(std::min(held, kSaleLotOil), kOilRateCents, kOilFeeBasisPoints);
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

// Shape check only, before any authoritative state is read: only Train carries a recipe.
[[nodiscard]] constexpr EconomyRejection CheckIntentShape(std::uint8_t op, std::uint8_t recipe) noexcept {
    if (op >= kEconomyOpCount) return EconomyRejection::MalformedCommand;
    if (static_cast<EconomyOp>(op) != EconomyOp::Train && recipe != 0) return EconomyRejection::MalformedCommand;
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

// Wire values are stable.
enum class Resource : std::uint8_t { Gold, Food, Oil };
inline constexpr std::uint8_t kResourceCount = 3;

[[nodiscard]] constexpr std::string_view ResourceName(Resource resource) noexcept {
    return resource == Resource::Gold ? "gold" : resource == Resource::Food ? "food" : "oil";
}

[[nodiscard]] constexpr Gold LoadSize(Resource resource) noexcept {
    return resource == Resource::Gold ? kLoadGold : resource == Resource::Food ? kLoadFood : kLoadOil;
}

// One seat's source of one resource. Everything taken from it is on a harvester, delivered or lost.
struct ResourceSource {
    bool present = false;
    Gold start = 0;
    Gold remaining = 0;
    Gold delivered = 0;
    Gold lost = 0;
};

// What a harvester is doing. Starting is the one step between appearing and its first order. Waiting
// covers everything its routine did not send it on: a player's order, nothing left to collect or a
// route that failed. Holding is the load it will not deliver until a living producer exists again.
// It keeps any load while it waits or holds.
enum class HarvesterPhase : std::uint8_t { Starting, ToSource, Loading, ToBase, Unloading, Holding, Waiting };

[[nodiscard]] constexpr std::string_view PhaseName(HarvesterPhase phase) noexcept {
    switch (phase) {
        case HarvesterPhase::Starting: return "starting";
        case HarvesterPhase::ToSource: return "to_source";
        case HarvesterPhase::Loading: return "loading";
        case HarvesterPhase::ToBase: return "to_base";
        case HarvesterPhase::Unloading: return "unloading";
        case HarvesterPhase::Holding: return "holding";
        case HarvesterPhase::Waiting: return "waiting";
    }
    return "unknown";
}

struct Harvester {
    UnitId unit = 0;
    HarvesterPhase phase = HarvesterPhase::Waiting;
    // What it collects: gold at first, then whichever source it was last sent to or moved on to.
    Resource target = Resource::Gold;
    // What its load is, which differs from the target after it is sent to the other source.
    Resource loadKind = Resource::Gold;
    Gold load = 0;
    std::uint32_t remainingSteps = 0;
    // The living producer a trip home already failed to reach. A different one is worth another trip.
    UnitId refusedDepot = 0;
    // Set when a player's order took it off the routine, so a new producer does not pull it back.
    bool released = false;
};

// Where a harvester stands this step, as the coordinator measured it.
struct HarvesterSituation {
    std::optional<Resource> atSource;
    bool atBase = false;
    bool moving = false;
    // At rest because of a player's stop or a failed route, not because it arrived somewhere.
    bool halted = false;
    // False only when the coordinator has no living producer to unload at. Existing callers
    // that do not say still have one.
    bool hasDepot = true;
    // Which living producer that is. Zero when the caller does not name one.
    UnitId depotId = 0;
};

// What one harvester did in one step. goToSource and goToBase ask the coordinator for a move.
struct HarvesterStep {
    Resource resource = Resource::Gold; // Of loaded and delivered.
    Gold loaded = 0;
    Gold delivered = 0;
    std::optional<Resource> goToSource;
    bool goToBase = false;
    bool arrivedAtSource = false; // Its routine reached the source and loading has begun.
    bool retargeted = false;      // Its source ran out and it moved on to the other one.
    bool stoppedWaiting = false;  // A player's order led it back to work.
    bool startedWaiting = false;  // Its routine ended: nothing left to collect, or a route failed.
    bool heldForDepot = false;   // The load stays aboard because there is no living producer.
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
    std::uint32_t worksHeld = 0;
    TransactionId lastTransaction = 0;
    std::uint32_t produced = 0;
    std::vector<QueueItem> queue;
    power::Grid grid;
    power::Allocation allocation;
    bool producerPowered = false;
    Rations startingRations = 0;
    Rations rations = 0;
    Rations rationsBought = 0;
    Rations rationsIssued = 0;
    Step nextUpkeepStep = 0;
    bool foodShortage = false;
    ResourceSource goldSource;
    ResourceSource foodSource;
    Gold goldHeld = 0;
    Gold goldSold = 0;
    // ECON-E30: crude oil has its own stock. It is not gold and not fuel, and only a sale turns it into anything.
    ResourceSource oilSource;
    Gold oilHeld = 0;
    Gold oilSold = 0;
    Cents saleCredited = 0;
    std::vector<Harvester> harvesters; // ascending unit

    [[nodiscard]] const ResourceSource& Source(Resource resource) const noexcept {
        return resource == Resource::Gold ? goldSource : resource == Resource::Food ? foodSource : oilSource;
    }
    [[nodiscard]] ResourceSource& Source(Resource resource) noexcept {
        return resource == Resource::Gold ? goldSource : resource == Resource::Food ? foodSource : oilSource;
    }

    [[nodiscard]] Gold Carried(Resource resource) const noexcept {
        Gold carried = 0;
        for (const Harvester& harvester : harvesters)
            if (harvester.loadKind == resource) carried += harvester.load;
        return carried;
    }
    [[nodiscard]] Gold GoldCarried() const noexcept { return Carried(Resource::Gold); }
    [[nodiscard]] Gold FoodCarried() const noexcept { return Carried(Resource::Food); }
    [[nodiscard]] Gold OilCarried() const noexcept { return Carried(Resource::Oil); }

    // The target if it has anything left, otherwise gold or food if that has. Nothing moves on to oil
    // by itself: a harvester goes to the field when it is sent there, and stays until it runs out.
    [[nodiscard]] std::optional<Resource> NextSource(Resource preferred) const noexcept {
        if (Source(preferred).remaining > 0) return preferred;
        const Resource other = preferred == Resource::Gold ? Resource::Food : Resource::Gold;
        if (Source(other).remaining > 0) return other;
        if (preferred == Resource::Oil && foodSource.remaining > 0) return Resource::Food;
        return std::nullopt;
    }

    // Whether a harvester would have anything to haul, if only by being sent to the field.
    [[nodiscard]] bool HasAnythingToHaul() const noexcept {
        return NextSource(Resource::Gold).has_value() || oilSource.remaining > 0;
    }

    // Every cent has one explanation: nothing is created or lost outside the ledger.
    [[nodiscard]] bool Reconciles() const noexcept {
        Cents in = 0;
        Cents out = 0;
        return available >= 0 && CheckedAdd(starting, credited, in) && CheckedAdd(in, refunded, in)
            && CheckedAdd(in, saleCredited, in) && CheckedAdd(available, spent, out) && in == out;
    }

    // Every ration has one explanation too, and the reserve is never negative.
    [[nodiscard]] bool RationsReconcile() const noexcept {
        return rations >= 0 && startingRations + rationsBought + foodSource.delivered == rations + rationsIssued;
    }

    // All the source's gold is still there, on the way, lost, held or sold.
    [[nodiscard]] bool GoldReconciles() const noexcept {
        return goldSource.remaining >= 0 && goldHeld >= 0
            && goldSource.start == goldSource.remaining + GoldCarried() + goldSource.delivered + goldSource.lost
            && goldSource.delivered == goldHeld + goldSold;
    }

    // The same for the oil field.
    [[nodiscard]] bool OilReconciles() const noexcept {
        return oilSource.remaining >= 0 && oilHeld >= 0
            && oilSource.start == oilSource.remaining + OilCarried() + oilSource.delivered + oilSource.lost
            && oilSource.delivered == oilHeld + oilSold;
    }

    // The same for the food source. Delivered food is in the ration reserve's own reconciliation.
    [[nodiscard]] bool FoodSourceReconciles() const noexcept {
        return foodSource.remaining >= 0
            && foodSource.start == foodSource.remaining + FoodCarried() + foodSource.delivered + foodSource.lost;
    }
};

struct RequestResult {
    EconomyRejection rejection = EconomyRejection::None;
    Cents amount = 0;
    Cents available = 0;
    std::size_t queued = 0;
    // A sale: what left the stock and how the net amount above was reached.
    SaleQuote sale;
    [[nodiscard]] explicit operator bool() const noexcept { return rejection == EconomyRejection::None; }
};

// What one participant's account did in one step.
struct StepResult {
    Cents income = 0;
    Cents worksIncome = 0;
    bool incomeOverflow = false;
    // The head item finished training and wants to appear; see CompleteHead and BlockHead.
    bool spawnDue = false;
    TransactionId transaction = 0;
    Recipe recipe = Recipe::Rifleman;
    // This step's allocation differs from the last one. headRemainingSteps is the head item's
    // remaining time before this step's production.
    bool powerChanged = false;
    std::uint32_t headRemainingSteps = 0;
    // Upkeep fell due this step: what was owed and what the reserve could pay.
    bool upkeepDue = false;
    Rations upkeepOwed = 0;
    Rations upkeepPaid = 0;
};

class Ledger {
public:
    void Clear() noexcept { accounts.clear(); }

    // firstIncomeStep is the first step that may pay income or charge upkeep. Refuses a zero or
    // repeated participant and a negative start.
    bool AddParticipant(ParticipantId participant, Cents starting, bool hasProducer, Step firstIncomeStep) {
        if (participant == 0 || starting < 0 || Find(participant)) return false;
        Account account;
        account.participant = participant;
        account.hasProducer = hasProducer;
        account.starting = starting;
        account.available = starting;
        account.nextIncomeStep = firstIncomeStep;
        account.startingRations = kStartingRations;
        account.rations = kStartingRations;
        account.nextUpkeepStep = firstIncomeStep;
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

    // Sets a participant's opening reserve, before anything has moved. Refuses a negative reserve,
    // one over the store's capacity and a participant that has already bought or issued food.
    bool SetStartingRations(ParticipantId participant, Rations rations) noexcept {
        Account* account = FindMutable(participant);
        if (!account || rations < 0 || rations > kFoodCapacityRations || account->rationsBought != 0 || account->rationsIssued != 0) return false;
        account->startingRations = rations;
        account->rations = rations;
        return true;
    }

    // Fills the reserve to capacity. The fill is recorded as bought so the ledger still reconciles.
    // A delivery into a full store stays on the harvester.
    bool FillFoodStore(ParticipantId participant) noexcept {
        Account* account = FindMutable(participant);
        if (!account || account->rations > kFoodCapacityRations) return false;
        const Rations room = kFoodCapacityRations - account->rations;
        account->rations += room;
        account->rationsBought += room;
        return true;
    }

    // Gives a participant a source, before anything has been taken from it. Refuses a negative amount.
    bool SetSource(ParticipantId participant, Resource resource, Gold amount) noexcept {
        Account* account = FindMutable(participant);
        if (!account || amount < 0) return false;
        ResourceSource& source = account->Source(resource);
        if (source.start != source.remaining || source.delivered != 0 || source.lost != 0) return false;
        source = ResourceSource{true, amount, amount, 0, 0};
        return true;
    }

    [[nodiscard]] const Harvester* FindHarvester(ParticipantId participant, UnitId unit) const noexcept {
        const Account* account = Find(participant);
        if (!account) return nullptr;
        const auto found = std::find_if(account->harvesters.begin(), account->harvesters.end(),
            [unit](const Harvester& harvester) { return harvester.unit == unit; });
        return found == account->harvesters.end() ? nullptr : &*found;
    }

    // A new harvester takes its first order at its next step. Refuses a zero or repeated unit and a
    // seat with no source. It waits if there is nothing left to collect.
    bool AddHarvester(ParticipantId participant, UnitId unit) {
        Account* account = FindMutable(participant);
        if (!account || unit == 0 || (!account->goldSource.present && !account->foodSource.present && !account->oilSource.present) || FindHarvester(participant, unit)) return false;
        const auto position = std::lower_bound(account->harvesters.begin(), account->harvesters.end(), unit,
            [](const Harvester& candidate, UnitId id) { return candidate.unit < id; });
        Harvester harvester;
        harvester.unit = unit;
        harvester.phase = account->NextSource(Resource::Gold) ? HarvesterPhase::Starting : HarvesterPhase::Waiting;
        account->harvesters.insert(position, harvester);
        return true;
    }

    // A player's order takes the harvester off its routine. It keeps its load. Returns false if it
    // was not on its routine.
    bool DetachHarvester(ParticipantId participant, UnitId unit) noexcept {
        Harvester* harvester = FindHarvesterMutable(participant, unit);
        if (!harvester || harvester->phase == HarvesterPhase::Waiting) return false;
        harvester->phase = HarvesterPhase::Waiting;
        harvester->remainingSteps = 0;
        harvester->released = true;
        return true;
    }

    // A harvester died: its load is gone and does not return to the source.
    Gold LoseHarvester(ParticipantId participant, UnitId unit) {
        Account* account = FindMutable(participant);
        if (!account) return 0;
        const auto found = std::find_if(account->harvesters.begin(), account->harvesters.end(),
            [unit](const Harvester& harvester) { return harvester.unit == unit; });
        if (found == account->harvesters.end()) return 0;
        const Gold load = found->load;
        account->Source(found->loadKind).lost += load;
        account->harvesters.erase(found);
        return load;
    }

    // One step of a harvester's routine. A load leaves the source when loading ends, so two harvesters
    // at one source cannot take the same load: units are stepped in ascending order and the later one
    // takes what is left. When its source runs out it moves on to the other one. A waiting harvester
    // that has arrived at a source takes that as its new target; with a load of the other resource it
    // goes home first. One that has arrived at the base with a load unloads it. That is how a player
    // sends it to work. A player's stop holds it where it is. With no living producer, a trip home or
    // an unload already under way keeps the load and does not credit it; the next living producer
    // sends it on again. Food goes straight into the ration reserve; what a full store cannot take
    // stays on the harvester, unloading, until there is room.
    HarvesterStep StepHarvester(ParticipantId participant, UnitId unit, HarvesterSituation situation) {
        HarvesterStep result;
        Account* account = FindMutable(participant);
        Harvester* harvester = FindHarvesterMutable(participant, unit);
        if (!account || !harvester) return result;
        const auto beginLoading = [&] {
            harvester->phase = HarvesterPhase::Loading;
            harvester->remainingSteps = kHarvesterLoadSteps;
        };
        const auto beginUnloading = [&] {
            harvester->phase = HarvesterPhase::Unloading;
            harvester->remainingSteps = kHarvesterUnloadSteps;
        };
        const auto wait = [&] {
            harvester->phase = HarvesterPhase::Waiting;
            result.startedWaiting = true;
        };
        const auto goHome = [&] {
            harvester->phase = HarvesterPhase::ToBase;
            result.goToBase = true;
        };
        const auto holdForDepot = [&] {
            harvester->phase = HarvesterPhase::Holding;
            harvester->remainingSteps = 0;
            result.heldForDepot = true;
        };
        const auto seekSource = [&] {
            const std::optional<Resource> next = account->NextSource(harvester->target);
            if (!next) {
                wait();
                return;
            }
            result.retargeted = *next != harvester->target;
            harvester->target = *next;
            harvester->phase = HarvesterPhase::ToSource;
            result.goToSource = next;
        };
        switch (harvester->phase) {
            case HarvesterPhase::Starting:
                seekSource();
                break;
            case HarvesterPhase::ToSource:
                if (account->Source(harvester->target).remaining <= 0) {
                    seekSource();
                } else if (situation.atSource == harvester->target) {
                    beginLoading();
                    result.arrivedAtSource = true;
                } else if (!situation.moving) {
                    wait();
                }
                break;
            case HarvesterPhase::Loading: {
                if (--harvester->remainingSteps > 0) break;
                ResourceSource& source = account->Source(harvester->target);
                const Gold taken = std::min(source.remaining, LoadSize(harvester->target) - harvester->load);
                source.remaining -= taken;
                harvester->load += taken;
                harvester->loadKind = harvester->target;
                result.resource = harvester->target;
                result.loaded = taken;
                if (harvester->load > 0) goHome();
                else seekSource();
                break;
            }
            case HarvesterPhase::ToBase:
                if (!situation.hasDepot) holdForDepot();
                else if (situation.atBase) beginUnloading();
                else if (!situation.moving) {
                    // A failed route is already Stop by the time this runs. A player's stop left the
                    // routine earlier, so this is the trip that could not reach the producer.
                    harvester->released = false;
                    harvester->refusedDepot = situation.depotId;
                    wait();
                }
                break;
            case HarvesterPhase::Unloading: {
                if (!situation.hasDepot) {
                    holdForDepot();
                    break;
                }
                if (--harvester->remainingSteps > 0) break;
                Gold credited = harvester->load;
                if (harvester->loadKind == Resource::Gold) {
                    account->goldHeld += credited;
                } else if (harvester->loadKind == Resource::Oil) {
                    account->oilHeld += credited;
                } else {
                    credited = std::min(credited, kFoodCapacityRations - account->rations);
                    account->rations += credited;
                }
                account->Source(harvester->loadKind).delivered += credited;
                harvester->load -= credited;
                result.resource = harvester->loadKind;
                result.delivered = credited;
                if (harvester->load > 0) harvester->remainingSteps = kHarvesterStoreRetrySteps;
                else seekSource();
                break;
            }
            case HarvesterPhase::Holding:
                if (!situation.hasDepot) break;
                if (situation.atBase) beginUnloading();
                else goHome();
                result.stoppedWaiting = true;
                break;
            case HarvesterPhase::Waiting:
                if (situation.moving || situation.halted) break;
                if (harvester->load > 0 && situation.atBase) {
                    beginUnloading();
                    result.stoppedWaiting = true;
                } else if (harvester->load > 0 && situation.hasDepot && !harvester->released
                    && situation.depotId != 0 && situation.depotId != harvester->refusedDepot) {
                    goHome();
                    result.stoppedWaiting = true;
                } else if (situation.atSource) {
                    const Resource here = *situation.atSource;
                    const bool refusedThisDepot = situation.depotId != 0 && situation.depotId == harvester->refusedDepot;
                    if (harvester->load > 0 && (harvester->loadKind != here || harvester->load >= LoadSize(here))) {
                        if (!refusedThisDepot) {
                            harvester->target = here;
                            goHome();
                            result.stoppedWaiting = true;
                        }
                    } else if (account->Source(here).remaining > 0) {
                        harvester->target = here;
                        beginLoading();
                        result.stoppedWaiting = true;
                    }
                }
                break;
        }
        return result;
    }

    // Takes effect at the participant's next Advance. Returns false without a generator, and false
    // when damage is keeping that generator offline.
    bool SetGeneratorOnline(ParticipantId participant, bool online) noexcept {
        Account* account = FindMutable(participant);
        return account && account->grid.SetGenerating(kGeneratorBuilding, online);
    }

    struct BuildingDamage {
        std::int32_t applied = 0;
        bool generatorOffline = false;
        bool producerLost = false;
        Cents refunded = 0;
    };

    // Damages one building already on this participant's grid. Does not add a building. Destroying
    // the producer refunds every queued item once and refuses further training as no producer.
    BuildingDamage DamageBuilding(ParticipantId participant, power::BuildingId id, std::int32_t amount) {
        BuildingDamage result;
        Account* account = FindMutable(participant);
        if (!account) return result;
        const std::size_t before = account->grid.Count();
        const power::Damage hit = account->grid.ApplyDamage(id, amount);
        result.applied = hit.applied;
        result.generatorOffline = hit.generatorOffline;
        if (account->grid.Count() != before) return result;
        if (!(hit.destroyed && id == kProducerBuilding && account->hasProducer)) return result;
        Cents sum = 0;
        for (const QueueItem& item : account->queue)
            if (!CheckedAdd(sum, item.paid, sum)) return result;
        Cents available = 0;
        Cents refunded = 0;
        if (!CheckedAdd(account->available, sum, available) || !CheckedAdd(account->refunded, sum, refunded)) return result;
        account->available = available;
        account->refunded = refunded;
        account->queue.clear();
        account->hasProducer = false;
        result.producerLost = true;
        result.refunded = sum;
        return result;
    }

    // Restores up to `amount` health to the generator and returns what it restored. At full health
    // the next Advance can power the same queue entry. A producer, a source and a missing building
    // are not repaired.
    std::int32_t RepairBuilding(ParticipantId participant, power::BuildingId id, std::int32_t amount) noexcept {
        Account* account = FindMutable(participant);
        return account ? account->grid.Repair(id, amount) : 0;
    }

    struct CaptureResult {
        bool captured = false;
        bool refused = false;
        bool producer = false;
        Cents refunded = 0;
        power::BuildingId newId = 0;
    };

    // Moves one works to `to`. `from` is 0 for a neutral works, and otherwise must hold one. Money it
    // already paid stays where it is: only future income moves.
    bool TransferWorks(ParticipantId from, ParticipantId to) {
        if (to == 0 || from == to) return false;
        Account* winner = FindMutable(to);
        Account* loser = from == 0 ? nullptr : FindMutable(from);
        if (!winner || (from != 0 && (!loser || loser->worksHeld == 0))) return false;
        if (loser) --loser->worksHeld;
        ++winner->worksHeld;
        return true;
    }

    // Moves one living building from one account to another. A producer refunds its queue once and
    // is refused when the capturer already has a living producer. A generator keeps its health and
    // takes a free id. A dead building, a missing id or the same account changes nothing.
    CaptureResult CaptureBuilding(ParticipantId from, ParticipantId to, power::BuildingId id) {
        CaptureResult result;
        if (from == 0 || to == 0 || from == to) return result;
        Account* loser = FindMutable(from);
        Account* winner = FindMutable(to);
        if (!loser || !winner) return result;
        const power::Building* existing = loser->grid.Find(id);
        if (!existing || existing->health <= 0) return result;
        const bool producer = existing->kind == power::BuildingKind::Producer;
        result.producer = producer;
        if (producer) {
            const power::Building* held = winner->grid.Find(kProducerBuilding);
            if (winner->hasProducer && held && held->health > 0) {
                result.refused = true;
                return result;
            }
        }
        Cents sum = 0;
        Cents available = 0;
        Cents refundedTotal = 0;
        if (producer) {
            for (const QueueItem& item : loser->queue)
                if (!CheckedAdd(sum, item.paid, sum)) return result;
            if (!CheckedAdd(loser->available, sum, available) || !CheckedAdd(loser->refunded, sum, refundedTotal)) return result;
        }
        std::optional<power::Building> moved = loser->grid.Remove(id);
        if (!moved) return result;
        power::Building stored = *moved;
        if (producer) {
            if (const power::Building* dead = winner->grid.Find(kProducerBuilding); dead && dead->health <= 0)
                winner->grid.Remove(kProducerBuilding);
            stored.id = kProducerBuilding;
        } else stored.id = winner->grid.NextId();
        if (!winner->grid.Add(stored)) {
            stored.id = id;
            loser->grid.Add(stored);
            return result;
        }
        if (producer) {
            loser->available = available;
            loser->refunded = refundedTotal;
            loser->queue.clear();
            loser->hasProducer = false;
            winner->hasProducer = true;
            result.refunded = sum;
        }
        result.captured = true;
        result.newId = stored.id;
        return result;
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
        // A harvester with nothing to haul would only eat.
        if (static_cast<Recipe>(recipe) == Recipe::Harvester && !account->HasAnythingToHaul())
            return Reject(EconomyRejection::NoResourceSource, account);
        if (!account->producerPowered) return Reject(EconomyRejection::ProducerUnpowered, account);
        if (account->foodShortage) return Reject(EconomyRejection::FoodShortage, account);
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
        return {EconomyRejection::None, definition->cost, account->available, account->queue.size(), {}};
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
        return {EconomyRejection::None, paid, account->available, account->queue.size(), {}};
    }

    // Buys one fixed pack of rations: debits its price and credits the reserve once, or changes
    // nothing. Always open during a shortage, because it is the way out of one.
    RequestResult BuyFood(ParticipantId participant, TransactionId transaction) {
        Account* account = FindMutable(participant);
        if (!account) return Reject(EconomyRejection::Unauthorized, nullptr);
        if (transaction <= account->lastTransaction) return Reject(EconomyRejection::DuplicateCommand, account);
        if (account->rations + kFoodPurchaseRations > kFoodCapacityRations) return Reject(EconomyRejection::FoodStoreFull, account);
        if (account->available < kFoodPurchaseCents) return Reject(EconomyRejection::InsufficientFunds, account);
        Cents available = 0;
        Cents spent = 0;
        if (!CheckedSub(account->available, kFoodPurchaseCents, available) || !CheckedAdd(account->spent, kFoodPurchaseCents, spent))
            return Reject(EconomyRejection::Overflow, account);
        account->available = available;
        account->spent = spent;
        account->rations += kFoodPurchaseRations;
        account->rationsBought += kFoodPurchaseRations;
        account->lastTransaction = transaction;
        return {EconomyRejection::None, kFoodPurchaseCents, account->available, account->queue.size(), {}};
    }

    // Sells one lot at the fixed quote: debits the stock and credits the net dollars once, or changes
    // nothing. Always open during an outage or a shortage, because it can pay the way out of one.
    RequestResult SellGold(ParticipantId participant, TransactionId transaction) {
        return Sell(participant, transaction, Resource::Gold);
    }
    RequestResult SellOil(ParticipantId participant, TransactionId transaction) {
        return Sell(participant, transaction, Resource::Oil);
    }

    // One authoritative step for one account: pays income that is due, allocates power, charges
    // upkeep that is due for staffedUnits (the force alive now), then advances only the head of
    // the queue. An item admitted earlier in the same step does not
    // advance until the next one, so with steady power it appears exactly durationSteps after its
    // admission step. An unpowered producer holds its whole queue: no time passes, a blocked item
    // does not retry and nothing is charged again, and a food shortage holds it the same way.
    // Cancel still refunds. A due debit takes what the reserve has, never going below zero. Only a
    // due debit paid in full ends a shortage, so a purchase takes effect at the next interval.
    StepResult Advance(ParticipantId participant, Step step, TransactionId admittedThisStep = 0, std::int64_t staffedUnits = 0) {
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
            const Cents worksPay = static_cast<Cents>(account->worksHeld) * kWorksIncomeCents;
            if (worksPay > 0) {
                if (CheckedAdd(account->available, worksPay, available) && CheckedAdd(account->credited, worksPay, credited)) {
                    account->available = available;
                    account->credited = credited;
                    result.worksIncome = worksPay;
                } else {
                    result.incomeOverflow = true;
                }
            }
            account->nextIncomeStep = step + kIncomeIntervalSteps;
        }
        if (!account->grid.Empty()) {
            const power::Allocation allocation = account->grid.Allocate();
            const power::Building* producer = account->grid.Find(kProducerBuilding);
            const bool powered = producer && producer->powered;
            result.powerChanged = powered != account->producerPowered || allocation.suppliedKw != account->allocation.suppliedKw
                || allocation.requiredKw != account->allocation.requiredKw;
            account->allocation = allocation;
            account->producerPowered = powered;
        }
        if (step >= account->nextUpkeepStep) {
            result.upkeepDue = true;
            result.upkeepOwed = std::max<std::int64_t>(staffedUnits, 0) * kRationsPerUnit;
            result.upkeepPaid = std::min(account->rations, result.upkeepOwed);
            account->rations -= result.upkeepPaid;
            account->rationsIssued += result.upkeepPaid;
            account->foodShortage = result.upkeepPaid < result.upkeepOwed;
            account->nextUpkeepStep = step + kFoodUpkeepIntervalSteps;
        }
        if (account->queue.empty()) return result;
        QueueItem& head = account->queue.front();
        result.headRemainingSteps = head.remainingSteps;
        if (!account->producerPowered || account->foodShortage) return result;
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

    [[nodiscard]] Harvester* FindHarvesterMutable(ParticipantId participant, UnitId unit) noexcept {
        Account* account = FindMutable(participant);
        if (!account) return nullptr;
        const auto found = std::find_if(account->harvesters.begin(), account->harvesters.end(),
            [unit](const Harvester& harvester) { return harvester.unit == unit; });
        return found == account->harvesters.end() ? nullptr : &*found;
    }

    RequestResult Sell(ParticipantId participant, TransactionId transaction, Resource resource) {
        Account* account = FindMutable(participant);
        if (!account) return Reject(EconomyRejection::Unauthorized, nullptr);
        if (transaction <= account->lastTransaction) return Reject(EconomyRejection::DuplicateCommand, account);
        const bool oil = resource == Resource::Oil;
        Gold& held = oil ? account->oilHeld : account->goldHeld;
        Gold& sold = oil ? account->oilSold : account->goldSold;
        const std::optional<SaleQuote> quote = oil ? QuoteOilLot(held) : QuoteGoldLot(held);
        if (!quote) return Reject(oil ? EconomyRejection::NoOilToSell : EconomyRejection::NothingToSell, account);
        Cents available = 0;
        Cents saleCredited = 0;
        if (!CheckedAdd(account->available, quote->net, available) || !CheckedAdd(account->saleCredited, quote->net, saleCredited))
            return Reject(EconomyRejection::Overflow, account);
        account->available = available;
        account->saleCredited = saleCredited;
        held -= quote->quantity;
        sold += quote->quantity;
        account->lastTransaction = transaction;
        return {EconomyRejection::None, quote->net, account->available, account->queue.size(), *quote};
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
