#pragma once

// Building electricity (ECON-E2; RTS_OPERATIONAL_ECONOMY_AND_POWER.md §4). Electricity is capacity
// and service: no stock, no currency and no money moves here. Every building has a positive load,
// a generator's own auxiliary load included. One binary allocation per authoritative step decides
// which buildings receive their minimum.
//
// The figures below are prototype fixture data, not balance decisions.

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <vector>

namespace politika::power {

using Kilowatts = std::int32_t;
using BuildingId = std::uint32_t;

// Wire values are stable.
enum class BuildingKind : std::uint8_t { Producer, Generator };

inline constexpr Kilowatts kGeneratorCapacityKw = 100;
inline constexpr Kilowatts kGeneratorAuxiliaryKw = 10;
inline constexpr Kilowatts kProducerLoadKw = 40;
// Fixture health. A generator supplies power only at full health. A building at zero stays on
// the grid but draws no load; one point of damage does not.
inline constexpr std::int32_t kGeneratorHealth = 100;
inline constexpr std::int32_t kProducerHealth = 200;
// Fixture, not balance. Three seconds at 20 Hz, long enough to walk away or be hit.
inline constexpr float kCaptureRangeCm = 200.0f;
inline constexpr std::int32_t kCaptureSteps = 60;
// Fixture, not balance. One point per step at 20 Hz: a husk takes 5 seconds and a scratch far less.
inline constexpr float kRepairRangeCm = 200.0f;
inline constexpr std::int32_t kRepairPointsPerStep = 1;

struct Building {
    BuildingId id = 0;
    BuildingKind kind = BuildingKind::Producer;
    Kilowatts loadKw = 0;
    Kilowatts capacityKw = 0;
    bool generating = true;
    bool powered = false;
    std::int32_t health = 0;
    std::int32_t maxHealth = 0;
};

struct Damage {
    std::int32_t applied = 0;
    bool generatorOffline = false;
    bool destroyed = false;
};

struct Allocation {
    std::int64_t suppliedKw = 0;
    std::int64_t requiredKw = 0;
};

class Grid {
public:
    // Refuses a zero or repeated ID, a load that is not positive, a negative capacity and a
    // non-generator with capacity.
    bool Add(const Building& building) {
        if (building.id == 0 || building.loadKw <= 0 || building.capacityKw < 0 || Find(building.id)) return false;
        if (building.kind != BuildingKind::Generator && building.capacityKw != 0) return false;
        Building stored = building;
        if (stored.maxHealth <= 0) {
            stored.maxHealth = stored.kind == BuildingKind::Generator ? kGeneratorHealth : kProducerHealth;
            stored.health = stored.maxHealth;
        }
        const auto position = std::lower_bound(buildings.begin(), buildings.end(), stored.id,
            [](const Building& candidate, BuildingId id) { return candidate.id < id; });
        buildings.insert(position, stored);
        return true;
    }

    [[nodiscard]] std::size_t Count() const noexcept { return buildings.size(); }

    // The next free id on this grid. Existing 1 and 2 stay the seat's original buildings.
    [[nodiscard]] BuildingId NextId() const noexcept {
        BuildingId next = 1;
        for (const Building& building : buildings) next = std::max(next, static_cast<BuildingId>(building.id + 1));
        return next;
    }

    // Removes one building and returns it, health included. A miss removes nothing.
    std::optional<Building> Remove(BuildingId id) {
        const auto found = std::find_if(buildings.begin(), buildings.end(),
            [id](const Building& candidate) { return candidate.id == id; });
        if (found == buildings.end()) return std::nullopt;
        const Building removed = *found;
        buildings.erase(found);
        return removed;
    }

    [[nodiscard]] const Building* Find(BuildingId id) const noexcept {
        const auto found = std::find_if(buildings.begin(), buildings.end(),
            [id](const Building& candidate) { return candidate.id == id; });
        return found == buildings.end() ? nullptr : &*found;
    }

    [[nodiscard]] bool Empty() const noexcept { return buildings.empty(); }

    // Whether a generator is producing; returns false if the ID is not a generator.
    bool SetGenerating(BuildingId id, bool generating) noexcept {
        const auto found = std::find_if(buildings.begin(), buildings.end(),
            [id](const Building& candidate) { return candidate.id == id && candidate.kind == BuildingKind::Generator; });
        if (found == buildings.end()) return false;
        // Damage keeps a generator offline until it is repaired back to full health.
        if (generating && found->health < found->maxHealth) return false;
        found->generating = generating;
        return true;
    }

    // Reduces one building's health. A generator stops supplying as soon as it is below full.
    // A miss, a non-positive amount or a building already at zero changes nothing and adds no building.
    Damage ApplyDamage(BuildingId id, std::int32_t amount) noexcept {
        Damage result;
        const auto found = std::find_if(buildings.begin(), buildings.end(),
            [id](const Building& candidate) { return candidate.id == id; });
        if (found == buildings.end() || amount <= 0 || found->health <= 0 || found->maxHealth <= 0) return result;
        result.applied = std::min(found->health, amount);
        found->health -= result.applied;
        if (found->kind == BuildingKind::Generator && found->health < found->maxHealth && found->generating) {
            found->generating = false;
            result.generatorOffline = true;
        }
        result.destroyed = found->health == 0;
        return result;
    }

    // Restores up to `amount` health to a generator and returns what it restored. The plant supplies
    // again only once it is back at full health. A producer is not repaired here.
    std::int32_t Repair(BuildingId id, std::int32_t amount) noexcept {
        const auto found = std::find_if(buildings.begin(), buildings.end(),
            [id](Building& candidate) { return candidate.id == id && candidate.kind == BuildingKind::Generator; });
        if (found == buildings.end() || amount <= 0 || found->health >= found->maxHealth) return 0;
        const std::int32_t restored = std::min(found->maxHealth - found->health, amount);
        found->health += restored;
        if (found->health == found->maxHealth) found->generating = true;
        return restored;
    }

    // Generators first, so a plant can start from its own output, then every other building, each
    // group in ascending ID. A building gets its whole load or nothing; a load that does not fit
    // does not stop a smaller one behind it.
    Allocation Allocate() noexcept {
        Allocation result;
        for (Building& building : buildings) {
            // A destroyed building stays on the grid so it can be repaired, but it is not a load.
            if (building.health <= 0) {
                building.powered = false;
                continue;
            }
            if (building.kind == BuildingKind::Generator && building.generating) result.suppliedKw += building.capacityKw;
            result.requiredKw += building.loadKw;
        }
        std::int64_t remaining = result.suppliedKw;
        const auto serve = [&remaining](Building& building) {
            building.powered = remaining >= building.loadKw;
            if (building.powered) remaining -= building.loadKw;
        };
        for (Building& building : buildings)
            if (building.health > 0 && building.kind == BuildingKind::Generator) serve(building);
        for (Building& building : buildings)
            if (building.health > 0 && building.kind != BuildingKind::Generator) serve(building);
        return result;
    }

private:
    std::vector<Building> buildings; // ascending ID
};

} // namespace politika::power
