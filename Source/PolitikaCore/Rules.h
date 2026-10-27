#pragma once

// Engine-independent POLITIKA command admission and faction permissions.
// The first Unreal vertical slice consumes the typed orders; this is not a
// production content schema or a complete economy/AI/fog/combat rules model.
#include <cmath>
#include <cstdint>
#include <variant>

namespace politika::rules {

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

enum class OrderKind : std::uint8_t { Move, Attack, AttackMove, Stop, Guard };

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
};

struct OrderResult {
    OrderRejection rejection = OrderRejection::None;
    [[nodiscard]] constexpr explicit operator bool() const noexcept {
        return rejection == OrderRejection::None;
    }
};

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
        if (attack->target_id == 0 || target == nullptr || target->id != attack->target_id)
            return {OrderRejection::MissingTarget};
        if (!target->attackable) return {OrderRejection::TargetNotAttackable};
        if (target->owner_player == requesting_player) return {OrderRejection::FriendlyTarget};
    }
    return {};
}

// One authoritative update has a deliberately visible order. The first UE
// prototype coordinator mirrors it; presentation consumes resulting state only.
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
