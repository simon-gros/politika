#pragma once

// Unit resolve: Discipline for military units, Morale for civilians (RTS_UNIT_DISCIPLINE.md,
// DISC-D1 and DISC-D2).
//
// One attribute for every unit, in integer milli-points so the authoritative fixed-step result is
// the same on every run of the same schedule. Every unit starts at the baseline. A military unit
// earns Discipline only while it serves in a formation with fellow members close by; outside a
// formation the value returns to the baseline. Civilians carry the same attribute but cannot be
// formation members, so they never earn the formation gain.
//
// DISC-D2 adds stress: a fallen formation member shocks nearby members of its formation (capped
// per window), a soldier alone outside any formation drifts below the baseline unless its doctrine
// tolerates solo work (Atlantic Concord), and mass-formation doctrine (Red Directorate, Iron
// Ascendancy) raises the ceiling to the maximum for a formation with enough earning members.
// The value still has no effect on any other attribute.

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>

#include "FormationGeometry.h"
#include "Rules.h"

namespace politika::discipline {

// Wire values are stable: Military is 0 so a default-initialised field is a military unit.
enum class UnitCategory : std::uint8_t { Military, NonCombat };

inline constexpr std::int32_t kMilliPerPoint = 1000;
inline constexpr std::int32_t kMaximum = 100 * kMilliPerPoint;
inline constexpr std::int32_t kBaseline = 50 * kMilliPerPoint;
// Defined for the break consumers (DISC-D4); nothing reads it yet.
inline constexpr std::int32_t kBrokenThreshold = 0;
inline constexpr std::int32_t kNormalCeiling = 80 * kMilliPerPoint;

// One evaluation per second at 20 steps per second.
inline constexpr std::uint64_t kEvaluationIntervalSteps = 20;
inline constexpr std::int32_t kFormationGainPerEvaluation = 1 * kMilliPerPoint;
inline constexpr std::int32_t kDecayPerEvaluation = kMilliPerPoint / 2;
// Fellow members of the same formation within this distance support a member; a soldier outside
// a formation with no friendly soldier within it is alone.
inline constexpr double kSupportRadiusCm = 400.0;

// DISC-D2 stress. A lone soldier drifts at the fade rate down to this floor.
inline constexpr std::int32_t kIsolationFloor = 30 * kMilliPerPoint;
// A fallen formation member costs each surviving member of its formation within the radius this
// much, at most kCasualtyShockCap per unit in any window of kCasualtyShockWindowSteps (5 s).
inline constexpr std::int32_t kCasualtyShock = 5 * kMilliPerPoint;
inline constexpr std::int32_t kCasualtyShockCap = 15 * kMilliPerPoint;
inline constexpr std::uint64_t kCasualtyShockWindowSteps = 100;
inline constexpr double kCasualtyShockRadiusCm = 800.0;
// Mass-formation doctrine: a formation with at least this many earning members earns up to the maximum.
inline constexpr std::size_t kMassFormationMembers = 32;
inline constexpr std::int32_t kMassCeiling = kMaximum;

// What changed at the last evaluation, for the HUD.
enum class Trend : std::uint8_t { Steady, Rising, Fading };

// Where a unit stands at an evaluation.
enum class Service : std::uint8_t {
    // Not a formation member and not alone (always the case for civilians).
    Outside,
    // A member that is not earning: independent of the formation's move, or without support.
    MemberHolding,
    // A member following its formation with enough fellow members close by.
    MemberEarning,
    // A soldier outside any formation with no friendly soldier close by, whose doctrine does not
    // tolerate solo work (DISC-D2).
    Isolated,
};

// Faction rules that change how the meter moves (RTS_UNIT_DISCIPLINE.md section 6).
enum class Doctrine : std::uint8_t {
    // Meridian Pact, Unaligned and the prototype test factions.
    Standard,
    // Red Directorate and Iron Ascendancy: the mass-formation ceiling.
    MassFormation,
    // Atlantic Concord: no isolation drift.
    SoloTolerant,
};

[[nodiscard]] constexpr Doctrine DoctrineOf(rules::Faction faction) noexcept {
    switch (faction) {
        case rules::Faction::RedDirectorate:
        case rules::Faction::IronAscendancy: return Doctrine::MassFormation;
        case rules::Faction::AtlanticConcord: return Doctrine::SoloTolerant;
        case rules::Faction::MeridianPact:
        case rules::Faction::Unaligned: return Doctrine::Standard;
    }
    return Doctrine::Standard;
}

// Ceiling for the earning members of one formation at this evaluation.
[[nodiscard]] constexpr std::int32_t FormationCeiling(Doctrine doctrine, std::size_t earningMembers) noexcept {
    return doctrine == Doctrine::MassFormation && earningMembers >= kMassFormationMembers ? kMassCeiling : kNormalCeiling;
}

// Whether a unit outside any formation with no friendly soldier close by drifts below the
// baseline. Civilians have no formation support to lose, and the Concord fights alone.
[[nodiscard]] constexpr bool IsolationApplies(Doctrine doctrine, UnitCategory category) noexcept {
    return category == UnitCategory::Military && doctrine != Doctrine::SoloTolerant;
}

// Fellow members a member needs within the support radius: two, or every other member of a
// smaller formation.
[[nodiscard]] constexpr std::size_t RequiredSupport(std::size_t members) noexcept {
    return members < 2 ? 1 : std::min<std::size_t>(2, members - 1);
}

// Other members of the same formation within the support radius of `positions[index]`. Each
// member counts once; non-finite positions (unavailable members) never support anyone.
[[nodiscard]] inline std::size_t SupportCount(std::span<const formation::Point> positions, std::size_t index,
    double radiusCm = kSupportRadiusCm) noexcept {
    if (index >= positions.size() || !formation::IsFinite(positions[index])) return 0;
    const formation::Point self = positions[index];
    const double radiusSquared = radiusCm * radiusCm;
    std::size_t count = 0;
    for (std::size_t other = 0; other < positions.size(); ++other) {
        if (other == index || !formation::IsFinite(positions[other])) continue;
        const double dx = positions[other].x - self.x;
        const double dy = positions[other].y - self.y;
        if (dx * dx + dy * dy <= radiusSquared) ++count;
    }
    return count;
}

struct Evaluation {
    std::int32_t value = kBaseline;
    Trend trend = Trend::Steady;
};

// Moves `current` towards `target` at the fade rate, landing on it exactly.
[[nodiscard]] constexpr Evaluation DriftTowards(std::int32_t current, std::int32_t target) noexcept {
    if (current > target) return {std::max(current - kDecayPerEvaluation, target), Trend::Fading};
    if (current < target) return {std::min(current + kDecayPerEvaluation, target), Trend::Rising};
    return {current, Trend::Steady};
}

// One evaluation. An earning member gains up to the ceiling and never past it, and above a lower
// ceiling (a mass formation that shrank) fades down to it; a holding member neither gains nor
// loses; a unit outside any formation returns towards the baseline from either side at the fade
// rate; a lone soldier fades towards the isolation floor and, already below it, holds.
[[nodiscard]] constexpr Evaluation Evaluate(std::int32_t current, Service service,
    std::int32_t ceiling = kNormalCeiling) noexcept {
    current = std::clamp(current, 0, kMaximum);
    ceiling = std::clamp(ceiling, 0, kMaximum);
    switch (service) {
        case Service::MemberEarning:
            if (current > ceiling) return DriftTowards(current, ceiling);
            if (current == ceiling) return {current, Trend::Steady};
            return {std::min(current + kFormationGainPerEvaluation, ceiling), Trend::Rising};
        case Service::MemberHolding:
            return {current, Trend::Steady};
        case Service::Outside:
            return DriftTowards(current, kBaseline);
        case Service::Isolated:
            if (current <= kIsolationFloor) return {current, Trend::Steady};
            return DriftTowards(current, kIsolationFloor);
    }
    return {current, Trend::Steady};
}

// Per-unit casualty-shock budget: the window opens at the first shock and lasts
// kCasualtyShockWindowSteps; within it a unit loses at most kCasualtyShockCap.
struct ShockWindow {
    std::uint64_t start = 0;
    std::int32_t taken = 0;
    bool open = false;
};

struct ShockResult {
    std::int32_t value = kBaseline;
    // What this casualty actually cost (0 when the window's cap is spent).
    std::int32_t applied = 0;
    ShockWindow window;
};

// One fallen comrade's shock at `step`. The meter never goes below zero; the budget counts what
// the rule charged, so a unit already at zero still uses up its window.
[[nodiscard]] constexpr ShockResult ApplyCasualtyShock(std::int32_t current, ShockWindow window, std::uint64_t step) noexcept {
    current = std::clamp(current, 0, kMaximum);
    if (!window.open || step >= window.start + kCasualtyShockWindowSteps || step < window.start)
        window = {step, 0, true};
    const std::int32_t charge = std::clamp(kCasualtyShockCap - window.taken, 0, kCasualtyShock);
    window.taken += charge;
    const std::int32_t after = std::max(current - charge, 0);
    return {after, current - after, window};
}

// Whole points as displayed (rounded down).
[[nodiscard]] constexpr std::int32_t DisplayPoints(std::int32_t milli) noexcept {
    return std::clamp(milli, 0, kMaximum) / kMilliPerPoint;
}

// The band boundary that a change from `before` to `after` crosses, if any: above the baseline
// 60, 70, 80, 90 or 100 when rising (the highest reached); below it 40, 30, 20, 10 or 0 when
// falling (the lowest reached); the baseline itself when the value lands on it from either side.
[[nodiscard]] constexpr std::optional<std::int32_t> CrossedBand(std::int32_t before, std::int32_t after) noexcept {
    constexpr std::int32_t step = 10 * kMilliPerPoint;
    if (after == before) return std::nullopt;
    if (after == kBaseline) return kBaseline / kMilliPerPoint;
    if (after > before) {
        for (std::int32_t band = kMaximum; band > kBaseline; band -= step)
            if (before < band && after >= band) return band / kMilliPerPoint;
        return std::nullopt;
    }
    for (std::int32_t band = 0; band < kBaseline; band += step)
        if (before > band && after <= band) return band / kMilliPerPoint;
    return std::nullopt;
}

} // namespace politika::discipline
