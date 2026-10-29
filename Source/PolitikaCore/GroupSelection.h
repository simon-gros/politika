#pragma once

// Portable selection-set and group-order semantics (RTS_SELECTION_AND_GROUP_ORDERS.md §2,
// §2C and §3, 4C baseline). A selection is local presentation state: a sorted, duplicate-free
// set of stable entity IDs. It never grants authority; every group order is a batch of
// ordinary single-issuer commands admitted independently by the server.

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <vector>

#include "Rules.h"

namespace politika::selection {

using EntityId = std::uint64_t;

// One input stroke applies exactly one of these to the current selection.
enum class SelectionOp : std::uint8_t {
    Replace,  // unmodified click or drag
    Union,    // Shift+drag
    Toggle,   // Shift+click
    Subtract, // Ctrl+drag
};

// Sorted ascending, duplicates and the invalid ID 0 removed.
[[nodiscard]] inline std::vector<EntityId> Normalize(std::vector<EntityId> ids) {
    std::sort(ids.begin(), ids.end());
    ids.erase(std::unique(ids.begin(), ids.end()), ids.end());
    if (!ids.empty() && ids.front() == 0) ids.erase(ids.begin());
    return ids;
}

[[nodiscard]] inline std::vector<EntityId> Apply(const std::vector<EntityId>& current, std::vector<EntityId> hits,
    SelectionOp op) {
    const std::vector<EntityId> base = Normalize(current);
    hits = Normalize(std::move(hits));
    std::vector<EntityId> out;
    switch (op) {
        case SelectionOp::Replace: return hits;
        case SelectionOp::Union:
            std::set_union(base.begin(), base.end(), hits.begin(), hits.end(), std::back_inserter(out));
            break;
        case SelectionOp::Toggle:
            std::set_symmetric_difference(base.begin(), base.end(), hits.begin(), hits.end(), std::back_inserter(out));
            break;
        case SelectionOp::Subtract:
            std::set_difference(base.begin(), base.end(), hits.begin(), hits.end(), std::back_inserter(out));
            break;
    }
    return out;
}

// Screen-space geometry in physical pixels.
struct ScreenPoint {
    double x = 0.0;
    double y = 0.0;
};

struct ScreenRect {
    ScreenPoint min;
    ScreenPoint max;
};

// A press becomes a box selection once the cursor has moved this many logical pixels;
// callers scale it by the viewport's DPI factor so the gesture feels the same on every display.
inline constexpr double kDragThresholdLogicalPx = 6.0;

[[nodiscard]] constexpr bool ExceedsDragThreshold(ScreenPoint press, ScreenPoint now, double dpiScale) noexcept {
    const double threshold = kDragThresholdLogicalPx * (dpiScale > 0.0 ? dpiScale : 1.0);
    const double dx = now.x - press.x;
    const double dy = now.y - press.y;
    return dx * dx + dy * dy >= threshold * threshold;
}

// The rectangle spanned by two corners in any drag direction.
[[nodiscard]] constexpr ScreenRect RectFromCorners(ScreenPoint a, ScreenPoint b) noexcept {
    return {{std::min(a.x, b.x), std::min(a.y, b.y)}, {std::max(a.x, b.x), std::max(a.y, b.y)}};
}

// Intersection policy (§2C): a partially boxed selection proxy is selected. Touching edges count.
[[nodiscard]] constexpr bool Intersects(ScreenRect a, ScreenRect b) noexcept {
    return a.min.x <= b.max.x && b.min.x <= a.max.x && a.min.y <= b.max.y && b.min.y <= a.max.y;
}

// Per-participant bound on queued commands awaiting the next authoritative step. A legitimate
// group order needs one command per living unit; the fixed allowance covers ability and stop
// inputs issued in the same step. Bounded by the game-wide unit limit, so a flood cannot grow
// authoritative work without limit.
inline constexpr std::int32_t kPendingCommandAllowance = 16;

[[nodiscard]] constexpr std::int32_t PendingCommandLimit(std::int32_t livingUnits) noexcept {
    return kPendingCommandAllowance + std::clamp<std::int32_t>(livingUnits, 0, rules::kMaxUnitsPerParticipant);
}

// Aggregated outcome of one group order (§3A): each issuer is admitted or rejected on its own,
// and the tally reports counts rather than an all-or-nothing result.
template <std::size_t RejectionCount>
struct BatchTally {
    std::uint32_t expected = 0;
    std::uint32_t accepted = 0;
    std::uint32_t rejected = 0;
    std::array<std::uint32_t, RejectionCount> byRejection{};

    // Returns false for an outcome beyond the expected count (it is not recorded).
    constexpr bool Record(bool wasAccepted, std::size_t rejection) noexcept {
        if (Complete()) return false;
        if (wasAccepted) {
            ++accepted;
        } else {
            ++rejected;
            if (rejection < RejectionCount) ++byRejection[rejection];
        }
        return true;
    }

    [[nodiscard]] constexpr bool Complete() const noexcept { return accepted + rejected >= expected; }

    // The most frequent rejection reason (lowest code on ties), or RejectionCount when none.
    [[nodiscard]] constexpr std::size_t DominantRejection() const noexcept {
        std::size_t best = RejectionCount;
        std::uint32_t bestCount = 0;
        for (std::size_t index = 0; index < RejectionCount; ++index)
            if (byRejection[index] > bestCount) {
                best = index;
                bestCount = byRejection[index];
            }
        return best;
    }
};

} // namespace politika::selection
