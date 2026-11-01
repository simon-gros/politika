#pragma once

// Building electricity (ECON-E2; RTS_OPERATIONAL_ECONOMY_AND_POWER.md §4). Electricity is capacity
// and service: no stock, no currency and no money moves here. Every building has a positive load,
// a generator's own auxiliary load included. One binary allocation per authoritative step decides
// which buildings receive their minimum.
//
// The figures below are prototype fixture data, not balance decisions.

#include <algorithm>
#include <cstdint>
#include <vector>

namespace politika::power {

using Kilowatts = std::int32_t;
using BuildingId = std::uint32_t;

// Wire values are stable.
enum class BuildingKind : std::uint8_t { Producer, Generator };

inline constexpr Kilowatts kGeneratorCapacityKw = 100;
inline constexpr Kilowatts kGeneratorAuxiliaryKw = 10;
inline constexpr Kilowatts kProducerLoadKw = 40;

struct Building {
    BuildingId id = 0;
    BuildingKind kind = BuildingKind::Producer;
    Kilowatts loadKw = 0;
    Kilowatts capacityKw = 0;
    bool generating = true;
    bool powered = false;
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
        const auto position = std::lower_bound(buildings.begin(), buildings.end(), building.id,
            [](const Building& candidate, BuildingId id) { return candidate.id < id; });
        buildings.insert(position, building);
        return true;
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
        found->generating = generating;
        return true;
    }

    // Generators first, so a plant can start from its own output, then every other building, each
    // group in ascending ID. A building gets its whole load or nothing; a load that does not fit
    // does not stop a smaller one behind it.
    Allocation Allocate() noexcept {
        Allocation result;
        for (const Building& building : buildings) {
            if (building.kind == BuildingKind::Generator && building.generating) result.suppliedKw += building.capacityKw;
            result.requiredKw += building.loadKw;
        }
        std::int64_t remaining = result.suppliedKw;
        const auto serve = [&remaining](Building& building) {
            building.powered = remaining >= building.loadKw;
            if (building.powered) remaining -= building.loadKw;
        };
        for (Building& building : buildings)
            if (building.kind == BuildingKind::Generator) serve(building);
        for (Building& building : buildings)
            if (building.kind != BuildingKind::Generator) serve(building);
        return result;
    }

private:
    std::vector<Building> buildings; // ascending ID
};

} // namespace politika::power
