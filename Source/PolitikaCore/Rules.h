#pragma once

// Engine-independent POLITIKA command-admission and faction permissions.
// This is a small authoritative C++ rules kernel for later Unreal integration,
// NOT a complete movement, economy, AI, fog or combat simulation.
#include <cmath>
#include <cstdint>

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
};

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

struct MapBounds {
    double width = 0.0;
    double height = 0.0;
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
