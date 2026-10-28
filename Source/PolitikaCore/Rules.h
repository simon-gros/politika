#pragma once

// Engine-independent POLITIKA command admission and faction permissions.
// The first Unreal vertical slice consumes the typed orders; this is not a
// production content schema or a complete economy/AI/fog/combat rules model.
#include <cmath>
#include <cstdint>
#include <variant>

namespace politika::rules {

// Explicit local-match lifecycle. This records authority boundaries; it does
// not prescribe a faction economy, save format, or network replication model.
enum class MatchPhase : std::uint8_t {
    SetupPending,
    SetupValidated,
    Loading,
    Starting,
    Active,
    Terminal,
    Results,
    Returning,
    AdmissionRejected,
};

[[nodiscard]] constexpr bool CanTransition(MatchPhase from, MatchPhase to) noexcept {
    switch (from) {
        case MatchPhase::SetupPending:
            return to == MatchPhase::SetupValidated || to == MatchPhase::AdmissionRejected;
        case MatchPhase::SetupValidated:
            return to == MatchPhase::Loading || to == MatchPhase::AdmissionRejected;
        case MatchPhase::Loading:
            return to == MatchPhase::Starting || to == MatchPhase::AdmissionRejected;
        case MatchPhase::Starting:
            return to == MatchPhase::Active || to == MatchPhase::AdmissionRejected;
        case MatchPhase::Active:
            return to == MatchPhase::Terminal || to == MatchPhase::Returning;
        case MatchPhase::Terminal:
            return to == MatchPhase::Results;
        case MatchPhase::Results:
            return to == MatchPhase::Returning;
        case MatchPhase::Returning:
        case MatchPhase::AdmissionRejected:
            return false;
    }
    return false;
}

struct MatchLifecycle {
    MatchPhase phase = MatchPhase::SetupPending;

    [[nodiscard]] constexpr bool TransitionTo(MatchPhase next) noexcept {
        if (!CanTransition(phase, next)) return false;
        phase = next;
        return true;
    }

    [[nodiscard]] constexpr bool IsActive() const noexcept {
        return phase == MatchPhase::Active;
    }
};

enum class Faction : std::uint8_t {
    AtlanticConcord,
    RedDirectorate,
    IronAscendancy,
    MeridianPact,
    Unaligned,
};

[[nodiscard]] constexpr bool IsPlayable(Faction faction) noexcept {
    return faction != Faction::Unaligned;
}

struct Entity {
    std::uint64_t id = 0;
    std::uint32_t owner_player = 0; // zero = neutral/NPC world ownership
    bool selectable = true;
    bool movable = true;
    bool can_attack = true;
    bool attackable = true;
    bool alive = true;
    bool operational = true;
};

// Stable, presentation-independent player/AI intent. Coordinates are prototype
// map-space values; Unreal adapters translate world hits into this domain type.
struct Position {
    double x = 0.0;
    double y = 0.0;
};

struct MoveOrder { Position destination; };
struct AttackOrder { std::uint64_t target_id = 0; };
struct AttackMoveOrder { Position destination; };
struct StopOrder {};
struct GuardOrder {};
using Order = std::variant<MoveOrder, AttackOrder, AttackMoveOrder, StopOrder, GuardOrder>;

// Runtime identities are match-local. Definition IDs, participant IDs and
// correlation IDs occupy separate fields and must not be substituted for one another.
struct CommandRequest {
    std::uint64_t match_generation = 0;
    std::uint64_t sequence = 0;
    std::uint32_t participant_id = 0;
    std::uint64_t issuer_entity_id = 0;
    std::uint64_t correlation_id = 0;
    Order intent = StopOrder{};
};

enum class OrderKind : std::uint8_t { Move, Attack, AttackMove, Stop, Guard };
enum class ExecutionOrder : std::uint8_t { Idle, Move, Attack, AttackMove, Stop, Guard };

[[nodiscard]] constexpr OrderKind KindOf(const Order& order) noexcept {
    return static_cast<OrderKind>(order.index());
}

[[nodiscard]] constexpr bool CanSelect(const Entity& entity, std::uint32_t player) noexcept {
    return player != 0 && entity.id != 0 && entity.owner_player == player && entity.selectable;
}

enum class MoveRejection : std::uint8_t {
    None,
    NotOwned,
    CannotMove,
    InvalidBounds,
    InvalidTarget,
};

enum class OrderRejection : std::uint8_t {
    None,
    NotOwned,
    CannotMove,
    CannotAttack,
    InvalidBounds,
    InvalidTarget,
    MissingTarget,
    TargetNotAttackable,
    FriendlyTarget,
    MissingIssuer,
    IssuerUnavailable,
    Unauthorized,
    MatchInactive,
    StaleMatch,
};

struct OrderResult {
    OrderRejection rejection = OrderRejection::None;
    [[nodiscard]] constexpr explicit operator bool() const noexcept {
        return rejection == OrderRejection::None;
    }
};

enum class OrderEndReason : std::uint8_t { None, Replaced, Stopped, TargetLost, UnitDestroyed, Arrived, Obstructed };
struct CommandOutcomeEvent {
    std::uint64_t step = 0, sequence = 0, correlation_id = 0, issuer_id = 0;
    OrderKind order = OrderKind::Stop;
    bool accepted = false;
    OrderRejection rejection = OrderRejection::None;
};
struct OrderChangedEvent {
    std::uint64_t step = 0, entity_id = 0;
    ExecutionOrder previous = ExecutionOrder::Idle, current = ExecutionOrder::Idle;
    OrderEndReason previous_end = OrderEndReason::Replaced;
};
struct UnitDamagedEvent { std::uint64_t step = 0, entity_id = 0; std::int32_t applied = 0; };
struct UnitDestroyedEvent { std::uint64_t step = 0, entity_id = 0; };
struct TargetLostEvent { std::uint64_t step = 0, entity_id = 0, target_id = 0; };
struct ObjectiveChangedEvent {
    std::uint64_t step = 0;
    bool contested = false, complete = false;
    std::uint16_t progress_milli = 0;
};
using GameplayEvent = std::variant<CommandOutcomeEvent, OrderChangedEvent, UnitDamagedEvent,
    UnitDestroyedEvent, TargetLostEvent, ObjectiveChangedEvent>;

struct MapBounds {
    double width = 0.0;
    double height = 0.0;
};

[[nodiscard]] inline bool IsValidPosition(MapBounds bounds, Position position) noexcept {
    return std::isfinite(bounds.width) && std::isfinite(bounds.height)
        && bounds.width > 0 && bounds.height > 0
        && std::isfinite(position.x) && std::isfinite(position.y)
        && position.x >= 0 && position.y >= 0
        && position.x <= bounds.width && position.y <= bounds.height;
}

// `target` is supplied by the authoritative entity lookup. A null target is
// rejected; callers must not trust a client-provided owner or capability bit.
[[nodiscard]] inline OrderResult AdmitOrder(
    const Entity& issuer,
    std::uint32_t requesting_player,
    MapBounds bounds,
    const Order& order,
    const Entity* target = nullptr
) noexcept {
    if (requesting_player == 0 || issuer.id == 0 || issuer.owner_player != requesting_player) {
        return {OrderRejection::NotOwned};
    }
    if (const auto* move = std::get_if<MoveOrder>(&order)) {
        if (!issuer.movable) return {OrderRejection::CannotMove};
        if (!std::isfinite(bounds.width) || !std::isfinite(bounds.height)
            || bounds.width <= 0 || bounds.height <= 0) return {OrderRejection::InvalidBounds};
        if (!IsValidPosition(bounds, move->destination)) return {OrderRejection::InvalidTarget};
    } else if (const auto* attack_move = std::get_if<AttackMoveOrder>(&order)) {
        if (!issuer.movable) return {OrderRejection::CannotMove};
        if (!std::isfinite(bounds.width) || !std::isfinite(bounds.height)
            || bounds.width <= 0 || bounds.height <= 0) return {OrderRejection::InvalidBounds};
        if (!IsValidPosition(bounds, attack_move->destination)) return {OrderRejection::InvalidTarget};
    } else if (const auto* attack = std::get_if<AttackOrder>(&order)) {
        if (!issuer.can_attack) return {OrderRejection::CannotAttack};
        if (attack->target_id == 0 || target == nullptr || target->id != attack->target_id || !target->alive)
            return {OrderRejection::MissingTarget};
        if (!target->attackable) return {OrderRejection::TargetNotAttackable};
        if (target->owner_player == requesting_player) return {OrderRejection::FriendlyTarget};
    }
    return {};
}

[[nodiscard]] inline OrderResult AdmitCommand(
    const CommandRequest& request,
    std::uint64_t active_match_generation,
    bool match_accepting,
    const Entity* issuer,
    MapBounds bounds,
    const Entity* target = nullptr
) noexcept {
    if (!match_accepting) return {OrderRejection::MatchInactive};
    if (request.match_generation != active_match_generation) return {OrderRejection::StaleMatch};
    if (!issuer || issuer->id == 0 || issuer->id != request.issuer_entity_id)
        return {OrderRejection::MissingIssuer};
    if (!issuer->alive || !issuer->operational) return {OrderRejection::IssuerUnavailable};
    if (request.participant_id == 0 || issuer->owner_player != request.participant_id)
        return {OrderRejection::Unauthorized};
    return AdmitOrder(*issuer, request.participant_id, bounds, request.intent, target);
}

// One authoritative update has a deliberately visible order; the UE
// coordinator executes this sequence and publishes the resulting typed events last.
enum class SimulationPhase : std::uint8_t {
    AdmitOrders,
    UpdateMovement,
    ResolveCombat,
    ApplyDamage,
    CleanupDeadEntities,
    UpdateObjectives,
    PublishEvents,
};

inline constexpr SimulationPhase kSimulationPhases[] = {
    SimulationPhase::AdmitOrders,
    SimulationPhase::UpdateMovement,
    SimulationPhase::ResolveCombat,
    SimulationPhase::ApplyDamage,
    SimulationPhase::CleanupDeadEntities,
    SimulationPhase::UpdateObjectives,
    SimulationPhase::PublishEvents,
};

[[nodiscard]] inline MoveRejection AdmitMove(
    const Entity& entity,
    std::uint32_t requesting_player,
    MapBounds bounds,
    double x,
    double y
) noexcept {
    if (requesting_player == 0 || entity.id == 0 || entity.owner_player != requesting_player) {
        return MoveRejection::NotOwned;
    }
    if (!entity.movable) {
        return MoveRejection::CannotMove;
    }
    if (!std::isfinite(bounds.width) || !std::isfinite(bounds.height)
        || bounds.width <= 0 || bounds.height <= 0) {
        return MoveRejection::InvalidBounds;
    }
    if (!std::isfinite(x) || !std::isfinite(y)
        || x < 0 || y < 0 || x > bounds.width || y > bounds.height) {
        return MoveRejection::InvalidTarget;
    }
    return MoveRejection::None;
}

// Provisional resource names are intentionally abstract: production recipes,
// conversion rates and faction-specific costs remain design/implementation work.
struct ResourcePool {
    std::int64_t crude_oil = 0;
    std::int64_t natural_gas = 0;
    std::int64_t processed_nuclear = 0;
};

[[nodiscard]] constexpr bool TrySpend(ResourcePool& balance, const ResourcePool& cost) noexcept {
    if (cost.crude_oil < 0 || cost.natural_gas < 0 || cost.processed_nuclear < 0
        || balance.crude_oil < 0 || balance.natural_gas < 0 || balance.processed_nuclear < 0
        || balance.crude_oil < cost.crude_oil || balance.natural_gas < cost.natural_gas
        || balance.processed_nuclear < cost.processed_nuclear) {
        return false;
    }
    balance.crude_oil -= cost.crude_oil;
    balance.natural_gas -= cost.natural_gas;
    balance.processed_nuclear -= cost.processed_nuclear;
    return true;
}

struct NuclearIntelligence {
    bool refining_intelligence = false;
    bool weapons_intelligence = false;
};

[[nodiscard]] constexpr bool CanRefineNuclear(
    Faction faction,
    NuclearIntelligence intelligence = {}
) noexcept {
    switch (faction) {
        case Faction::AtlanticConcord:
        case Faction::RedDirectorate:
            return true;
        case Faction::IronAscendancy:
            return intelligence.refining_intelligence && intelligence.weapons_intelligence;
        case Faction::MeridianPact:
        case Faction::Unaligned:
            return false;
    }
    return false;
}

enum class SabotageOperator : std::uint8_t {
    Spy,
    Conventional,
};

[[nodiscard]] constexpr bool CanPlantExplosives(
    Faction faction,
    SabotageOperator unit,
    bool late_espionage_unlocked
) noexcept {
    switch (faction) {
        case Faction::MeridianPact:
            return true; // early low-cost conventional sabotage; exact price defined later
        case Faction::AtlanticConcord:
        case Faction::RedDirectorate:
        case Faction::IronAscendancy:
            return late_espionage_unlocked && unit == SabotageOperator::Spy;
        case Faction::Unaligned:
            return false;
    }
    return false;
}

} // namespace politika::rules
