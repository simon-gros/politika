#pragma once

// Combat stance and formation adherence (RTS_UNIT_FORMATIONS_AND_DISCIPLINE.md §5, FORM-F4).
//
// Stance decides what an *idle* unit does on its own: whether it acquires and fires at
// enemies, and how far it may chase one before it gives up and walks back to its post.
// Adherence is a formation setting: how tightly members keep to their slots after such a
// chase, and whether the formation closes its ranks after losses. Neither one is an order.
// Changing a stance never replaces the unit's current order, and an explicit order (Attack,
// Attack-move, Guard order) never clears the stance; it simply takes precedence while it
// runs, so an explicit Attack on a Hold Fire unit fires at that target.
//
// Precedence of the effective stance: a per-unit override, then the unit's formation
// stance, then the archetype default. Form and Release never change a unit's effective
// stance: the caller re-derives the override from the stance the unit had before.

#include <cmath>
#include <cstdint>
#include <optional>
#include <string_view>

#include "FormationGeometry.h"

namespace politika::stance {

// Wire values are stable: HoldGround is 0 so a default-initialised field is the default.
enum class CombatStance : std::uint8_t { HoldGround, Guard, Aggressive, HoldFire };
inline constexpr std::uint8_t kStanceCount = 4;
// Today's idle behaviour before stances existed: fire at enemies in range, never move.
inline constexpr CombatStance kDefaultStance = CombatStance::HoldGround;

enum class Adherence : std::uint8_t { Flexible, Maintain, Free };
inline constexpr std::uint8_t kAdherenceCount = 3;
inline constexpr Adherence kDefaultAdherence = Adherence::Flexible;

// Chase leash: the farthest an idle unit may move from its post while engaging on its own.
inline constexpr double kGuardLeashCm = 600.0;
inline constexpr double kAggressiveLeashCm = 1500.0;
// Maintain keeps members closer to their slots.
inline constexpr double kMaintainLeashScale = 0.5;
// A unit closer than this to its post after an engagement does not walk back.
inline constexpr double kReturnToleranceCm = 50.0;

[[nodiscard]] constexpr std::string_view StanceName(CombatStance stance) noexcept {
    switch (stance) {
        case CombatStance::HoldGround: return "hold_ground";
        case CombatStance::Guard: return "guard";
        case CombatStance::Aggressive: return "aggressive";
        case CombatStance::HoldFire: return "hold_fire";
    }
    return "invalid";
}

[[nodiscard]] constexpr std::string_view AdherenceName(Adherence adherence) noexcept {
    switch (adherence) {
        case Adherence::Flexible: return "flexible";
        case Adherence::Maintain: return "maintain";
        case Adherence::Free: return "free";
    }
    return "invalid";
}

// The key cycles in this order; HoldFire wraps back to HoldGround.
[[nodiscard]] constexpr CombatStance NextStance(CombatStance stance) noexcept {
    switch (stance) {
        case CombatStance::HoldGround: return CombatStance::Guard;
        case CombatStance::Guard: return CombatStance::Aggressive;
        case CombatStance::Aggressive: return CombatStance::HoldFire;
        case CombatStance::HoldFire: return CombatStance::HoldGround;
    }
    return kDefaultStance;
}

[[nodiscard]] constexpr Adherence NextAdherence(Adherence adherence) noexcept {
    switch (adherence) {
        case Adherence::Flexible: return Adherence::Maintain;
        case Adherence::Maintain: return Adherence::Free;
        case Adherence::Free: return Adherence::Flexible;
    }
    return kDefaultAdherence;
}

[[nodiscard]] constexpr bool IsValidStance(std::uint8_t value) noexcept { return value < kStanceCount; }
[[nodiscard]] constexpr bool IsValidAdherence(std::uint8_t value) noexcept { return value < kAdherenceCount; }

// Precedence: an explicit per-unit override, else the formation's stance, else the default.
[[nodiscard]] constexpr CombatStance Effective(std::optional<CombatStance> unitOverride,
    std::optional<CombatStance> formationStance) noexcept {
    return unitOverride ? *unitOverride : formationStance ? *formationStance : kDefaultStance;
}

// The override a unit needs to keep `previous` as its effective stance under `inherited`.
[[nodiscard]] constexpr std::optional<CombatStance> OverrideFor(CombatStance previous, CombatStance inherited) noexcept {
    return previous == inherited ? std::nullopt : std::optional<CombatStance>(previous);
}

// What an idle unit does on its own.
struct IdleBehavior {
    bool acquire = false;        // picks enemy targets by itself
    bool chase = false;          // moves towards a target outside weapon range
    double leashCm = 0.0;        // farthest distance from its post while chasing
    double acquireRangeCm = 0.0; // 0: the archetype's own acquisition range
    bool returnToPost = false;   // walks back to its post when the engagement ends
};

// `adherence` is the formation's setting for a member, or nullopt for a unit outside any
// formation (which always returns to where it stood).
[[nodiscard]] constexpr IdleBehavior BehaviorFor(CombatStance stance, std::optional<Adherence> adherence) noexcept {
    IdleBehavior behavior;
    switch (stance) {
        case CombatStance::HoldFire:
            return behavior;
        case CombatStance::HoldGround:
            behavior.acquire = true;
            return behavior;
        case CombatStance::Guard:
            behavior.leashCm = kGuardLeashCm;
            break;
        case CombatStance::Aggressive:
            behavior.leashCm = kAggressiveLeashCm;
            // Aggressive units look out to their leash, still only at enemies their team sees.
            behavior.acquireRangeCm = kAggressiveLeashCm;
            break;
    }
    behavior.acquire = true;
    behavior.chase = true;
    behavior.returnToPost = !adherence || *adherence != Adherence::Free;
    if (adherence && *adherence == Adherence::Maintain) behavior.leashCm *= kMaintainLeashScale;
    return behavior;
}

[[nodiscard]] inline double Distance(formation::Point a, formation::Point b) noexcept {
    return std::hypot(a.x - b.x, a.y - b.y);
}

// A chasing unit may engage `target` only when it can fire at it from inside its leash:
// otherwise it would chase out, give up and walk back forever.
[[nodiscard]] inline bool WithinReach(formation::Point post, formation::Point target, double leashCm,
    double weaponRangeCm) noexcept {
    return formation::IsFinite(post) && formation::IsFinite(target) && leashCm > 0.0
        && Distance(post, target) <= leashCm + weaponRangeCm;
}

// Clamps a chase destination onto the leash circle around the post.
[[nodiscard]] inline formation::Point ClampToLeash(formation::Point post, formation::Point destination,
    double leashCm) noexcept {
    const double distance = Distance(post, destination);
    if (!(distance > leashCm) || distance <= 0.0) return destination;
    const double scale = leashCm / distance;
    return {post.x + (destination.x - post.x) * scale, post.y + (destination.y - post.y) * scale};
}

} // namespace politika::stance
