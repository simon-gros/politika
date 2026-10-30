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
#include <utility>
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

// Numbered control groups (4C-G, §2B and §2D). A slot is a local membership snapshot of stable
// IDs for one participant in one match generation. It is never a formation, never grants
// authority and never issues an order; recall only replaces the local selection.
inline constexpr std::size_t kControlGroupSlots = 10;

// A second press of the same slot within this many seconds focuses the camera on the group.
inline constexpr double kGroupDoubleTapSeconds = 0.35;

// A second click on the same own unit within this many seconds selects its on-screen archetype.
inline constexpr double kDoubleClickSeconds = 0.45;

enum class GroupOp : std::uint8_t {
    Replace,  // Ctrl+n:  G[n] := S
    Union,    // Shift+n: G[n] := G[n] u S, other slots unchanged
    Transfer, // Alt+n:   G[m] := G[m] \ S for every m != n, then G[n] := G[n] u S
};

class ControlGroups {
public:
    // Binds the slots to a participant and match generation. Any change of either clears every
    // slot, so no reference survives a rematch, a new match or a reassigned seat. Returns true
    // when slots were cleared.
    bool Bind(std::uint64_t generation, std::uint32_t participant) {
        if (generation == generation_ && participant == participant_) return false;
        const bool hadMembers = !Empty();
        Clear();
        generation_ = generation;
        participant_ = participant;
        return hadMembers;
    }

    void Clear() {
        for (std::vector<EntityId>& slot : slots_) slot.clear();
        generation_ = 0;
        participant_ = 0;
    }

    [[nodiscard]] std::uint64_t Generation() const noexcept { return generation_; }
    [[nodiscard]] std::uint32_t Participant() const noexcept { return participant_; }

    [[nodiscard]] bool Empty() const noexcept {
        return std::all_of(slots_.begin(), slots_.end(), [](const std::vector<EntityId>& slot) { return slot.empty(); });
    }

    // An empty selection never edits a slot (there is no confirmed empty replacement), and an
    // unbound participant has no slots. Returns false when nothing was applied.
    bool Assign(std::size_t slot, GroupOp op, std::vector<EntityId> selection) {
        if (slot >= kControlGroupSlots || participant_ == 0) return false;
        selection = Normalize(std::move(selection));
        if (selection.empty()) return false;
        switch (op) {
            case GroupOp::Replace: slots_[slot] = std::move(selection); return true;
            case GroupOp::Transfer:
                for (std::size_t other = 0; other < kControlGroupSlots; ++other)
                    if (other != slot) slots_[other] = Apply(slots_[other], selection, SelectionOp::Subtract);
                [[fallthrough]];
            case GroupOp::Union: slots_[slot] = Apply(slots_[slot], selection, SelectionOp::Union); return true;
        }
        return false;
    }

    // Removes the selection from one slot only (explicit removal in the group UI).
    bool Remove(std::size_t slot, const std::vector<EntityId>& selection) {
        if (slot >= kControlGroupSlots) return false;
        const std::size_t before = slots_[slot].size();
        slots_[slot] = Apply(slots_[slot], selection, SelectionOp::Subtract);
        return slots_[slot].size() != before;
    }

    [[nodiscard]] const std::vector<EntityId>& Members(std::size_t slot) const {
        static const std::vector<EntityId> none;
        return slot < kControlGroupSlots ? slots_[slot] : none;
    }

    // Drops every permanently invalid reference (dead, destroyed, transferred, revoked) from all
    // slots. Returns how many references were dropped.
    template <class IsValid>
    std::size_t Prune(IsValid&& isValid) {
        std::size_t dropped = 0;
        for (std::vector<EntityId>& slot : slots_) {
            const auto end = std::remove_if(slot.begin(), slot.end(), [&](EntityId id) { return !isValid(id); });
            dropped += static_cast<std::size_t>(std::distance(end, slot.end()));
            slot.erase(end, slot.end());
        }
        return dropped;
    }

private:
    std::array<std::vector<EntityId>, kControlGroupSlots> slots_{};
    std::uint64_t generation_ = 0;
    std::uint32_t participant_ = 0;
};

// Detects a rapid second recall of the same slot. Any other slot, or a slow second press,
// starts a new sequence; a detected double tap resets so a third press is a plain recall.
class RecallTapTracker {
public:
    bool Tap(std::size_t slot, double nowSeconds, double windowSeconds = kGroupDoubleTapSeconds) noexcept {
        const bool isDouble = hasLast_ && slot == lastSlot_ && nowSeconds >= lastSeconds_
            && nowSeconds - lastSeconds_ <= windowSeconds;
        hasLast_ = !isDouble;
        lastSlot_ = slot;
        lastSeconds_ = nowSeconds;
        return isDouble;
    }

    void Reset() noexcept { hasLast_ = false; }

private:
    bool hasLast_ = false;
    std::size_t lastSlot_ = 0;
    double lastSeconds_ = 0.0;
};

// Tab/Shift+Tab subgroup cycling (§2B): the selection splits into one subgroup per archetype,
// ordered by each archetype's lowest member ID, so the order is stable for a given selection.
// Returns the archetype key of every subgroup in that order.
template <class Key>
[[nodiscard]] std::vector<Key> SubgroupOrder(std::vector<std::pair<EntityId, Key>> members) {
    std::sort(members.begin(), members.end(), [](const auto& a, const auto& b) { return a.first < b.first; });
    std::vector<Key> order;
    for (const auto& member : members)
        if (std::find(order.begin(), order.end(), member.second) == order.end()) order.push_back(member.second);
    return order;
}

// The next active subgroup index, wrapping in both directions. Zero subgroups gives zero.
[[nodiscard]] constexpr std::size_t CycleSubgroup(std::size_t current, std::size_t count, bool forward) noexcept {
    if (count == 0) return 0;
    current %= count;
    return forward ? (current + 1) % count : (current + count - 1) % count;
}

// The clickable 0-9 group bar, centred along the bottom edge in keyboard order (1..9 then 0).
// Logical sizes are scaled by the HUD's UI scale; geometry is in viewport pixels.
struct GroupBarLayout {
    double slotWidth = 64.0;
    double slotHeight = 46.0;
    double gap = 6.0;
    double bottomMargin = 22.0;
    // Keeps the bar clear of the command panel (left 40 + width 390 + gap 16) on narrow views.
    double minLeft = 446.0;
};

[[nodiscard]] constexpr std::size_t GroupBarSlotAtPosition(std::size_t position) noexcept {
    return (position + 1) % kControlGroupSlots;
}

[[nodiscard]] constexpr ScreenRect GroupBarSlotRect(std::size_t slot, double viewWidth, double viewHeight, double uiScale,
    GroupBarLayout layout = {}) noexcept {
    const double scale = uiScale > 0.0 ? uiScale : 1.0;
    const double w = layout.slotWidth * scale;
    const double h = layout.slotHeight * scale;
    const double gap = layout.gap * scale;
    const double total = static_cast<double>(kControlGroupSlots) * w + static_cast<double>(kControlGroupSlots - 1) * gap;
    const std::size_t position = (slot + kControlGroupSlots - 1) % kControlGroupSlots;
    const double barLeft = std::max((viewWidth - total) * 0.5, layout.minLeft * scale);
    const double left = barLeft + static_cast<double>(position) * (w + gap);
    const double top = viewHeight - layout.bottomMargin * scale - h;
    return {{left, top}, {left + w, top + h}};
}

// The slot under a viewport point, or kControlGroupSlots when the point is not on the bar.
[[nodiscard]] constexpr std::size_t GroupBarSlotAt(ScreenPoint point, double viewWidth, double viewHeight, double uiScale,
    GroupBarLayout layout = {}) noexcept {
    for (std::size_t slot = 0; slot < kControlGroupSlots; ++slot) {
        const ScreenRect rect = GroupBarSlotRect(slot, viewWidth, viewHeight, uiScale, layout);
        if (point.x >= rect.min.x && point.x < rect.max.x && point.y >= rect.min.y && point.y < rect.max.y) return slot;
    }
    return kControlGroupSlots;
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
