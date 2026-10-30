#pragma once

// Unit resolve: Discipline for military units, Morale for civilians (RTS_UNIT_DISCIPLINE.md,
// DISC-D1).
//
// One attribute for every unit, in integer milli-points so the authoritative fixed-step result is
// the same on every run of the same schedule. Every unit starts at the baseline. A military unit
// earns Discipline only while it serves in a formation with fellow members close by; outside a
// formation the value fades back to the baseline and never below it. Civilians carry the same
// attribute but cannot be formation members, so they never earn the formation gain. In DISC-D1
// the value has no effect on any other attribute.

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>

#include "FormationGeometry.h"

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
// Fellow members of the same formation within this distance support a member.
inline constexpr double kSupportRadiusCm = 400.0;

// What changed at the last evaluation, for the HUD.
enum class Trend : std::uint8_t { Steady, Rising, Fading };

// Where a unit stands at an evaluation.
enum class Service : std::uint8_t {
    // Not a formation member (always the case for civilians).
    Outside,
    // A member that is not earning: independent of the formation's move, or without support.
    MemberHolding,
    // A member following its formation with enough fellow members close by.
    MemberEarning,
};

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

// One evaluation. An earning member gains up to the ceiling and never past it; a holding member
// neither gains nor loses; a unit outside any formation fades towards the baseline and never
// below it. A value below the baseline (only reachable through later stress rules) is held.
[[nodiscard]] constexpr Evaluation Evaluate(std::int32_t current, Service service,
    std::int32_t ceiling = kNormalCeiling) noexcept {
    current = std::clamp(current, 0, kMaximum);
    ceiling = std::clamp(ceiling, 0, kMaximum);
    switch (service) {
        case Service::MemberEarning:
            if (current >= ceiling) return {current, Trend::Steady};
            return {std::min(current + kFormationGainPerEvaluation, ceiling), Trend::Rising};
        case Service::MemberHolding:
            return {current, Trend::Steady};
        case Service::Outside:
            if (current <= kBaseline) return {current, Trend::Steady};
            return {std::max(current - kDecayPerEvaluation, kBaseline), Trend::Fading};
    }
    return {current, Trend::Steady};
}

// Whole points as displayed (rounded down).
[[nodiscard]] constexpr std::int32_t DisplayPoints(std::int32_t milli) noexcept {
    return std::clamp(milli, 0, kMaximum) / kMilliPerPoint;
}

// The band boundary (60, 70, 80, 90 or 100 points, or the baseline when fading back to it) that
// a change from `before` to `after` crosses, if any. Rising reports the highest boundary
// reached; fading reports the baseline only when the value lands on it.
[[nodiscard]] constexpr std::optional<std::int32_t> CrossedBand(std::int32_t before, std::int32_t after) noexcept {
    if (after > before) {
        for (std::int32_t band = kMaximum; band > kBaseline; band -= 10 * kMilliPerPoint)
            if (before < band && after >= band) return band / kMilliPerPoint;
        return std::nullopt;
    }
    if (after < before && after == kBaseline) return kBaseline / kMilliPerPoint;
    return std::nullopt;
}

} // namespace politika::discipline
