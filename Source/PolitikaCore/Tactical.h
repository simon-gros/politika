#pragma once

// Garrison, crush and concealment for the bounded prototype (UNIT_SPECIFICATION_FRAMEWORK.md 5.3, 6.9, 9.1).
//
// No damage types exist yet, so a garrison is not split by weapon: occupants simply cannot be hit, and
// the building takes the fire. Numbers the specification does not give are marked as fixtures.

#include <algorithm>
#include <cstdint>

namespace politika::tactical {

// Fixture, not balance.
inline constexpr std::int32_t kGarrisonCapacity = 5;
// Fixture, not balance: the same ring as a capture or a repair.
inline constexpr float kGarrisonEntryRangeCm = 200.0f;
// Section 6.9: fire through the building's ports at +15 % range, and leave at 20 % structure health.
inline constexpr std::int32_t kGarrisonRangePermille = 1150;
inline constexpr std::int32_t kGarrisonEjectPermille = 200;

[[nodiscard]] constexpr bool EjectsOccupants(std::int32_t health, std::int32_t maxHealth) noexcept {
    return static_cast<std::int64_t>(health) * 1000 <= static_cast<std::int64_t>(maxHealth) * kGarrisonEjectPermille;
}

// Fixture, not balance: about a vehicle's half width, measured from the unit's centre.
inline constexpr double kCrushRadiusCm = 90.0;
// Fixture: one crush cue per vehicle in two seconds, however many it runs over.
inline constexpr std::uint64_t kCrushCueSteps = 40;

// Whether a vehicle that moved in a straight line this step passed over a victim.
[[nodiscard]] constexpr bool Crushes(double fromX, double fromY, double toX, double toY, double victimX, double victimY) noexcept {
    const double dx = toX - fromX;
    const double dy = toY - fromY;
    const double lengthSquared = dx * dx + dy * dy;
    if (!(lengthSquared > 0.0)) return false;
    const double along = std::clamp(((victimX - fromX) * dx + (victimY - fromY) * dy) / lengthSquared, 0.0, 1.0);
    const double offX = fromX + along * dx - victimX;
    const double offY = fromY + along * dy - victimY;
    return offX * offX + offY * offY <= kCrushRadiusCm * kCrushRadiusCm;
}

// Section 9.1, at the 20 Hz simulation step: stationary for 3 s, revealed for 4 s after firing or being hit.
inline constexpr std::int32_t kConcealSteps = 60;
inline constexpr std::int32_t kRevealSteps = 80;
inline constexpr double kConcealedObserveFloorCm = 250.0;
inline constexpr double kConcealedObserveFraction = 0.35;

// There is no invisibility: an observer this close still sees a concealed unit.
[[nodiscard]] constexpr double ConcealedObserveRange(double sightRadiusCm) noexcept {
    return std::max(kConcealedObserveFloorCm, kConcealedObserveFraction * sightRadiusCm);
}

struct Concealment {
    // The player's order. The unit conceals and reveals itself under it until the order is withdrawn.
    bool posture = false;
    bool concealed = false;
    std::int32_t stillSteps = 0;
    std::int32_t revealSteps = 0;
};

enum class ConcealChange : std::uint8_t { None, On, Off };

// One simulation step. `moving` means faster than half the unit's speed; slower does not break it.
[[nodiscard]] constexpr ConcealChange AdvanceConcealment(Concealment& state, bool moving, bool fired, bool damaged) noexcept {
    const bool wasConcealed = state.concealed;
    if (!state.posture) {
        state = Concealment{};
        return wasConcealed ? ConcealChange::Off : ConcealChange::None;
    }
    const bool exposed = fired || damaged;
    if (exposed) state.revealSteps = kRevealSteps;
    else if (state.revealSteps > 0) --state.revealSteps;
    state.stillSteps = moving || exposed ? 0 : std::min(state.stillSteps + 1, kConcealSteps);
    state.concealed = state.revealSteps == 0 && state.stillSteps >= kConcealSteps;
    if (state.concealed == wasConcealed) return ConcealChange::None;
    return state.concealed ? ConcealChange::On : ConcealChange::Off;
}

} // namespace politika::tactical
