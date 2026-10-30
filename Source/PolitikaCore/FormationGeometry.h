#pragma once

// Portable group-move geometry (RTS_UNIT_FORMATIONS_AND_DISCIPLINE.md §4 and §10, FORM-F2).
// A selected group ordered to a point gets one distinct destination per member, laid out as a
// Line or a Column facing the direction of travel. This is geometry only: there is no
// formation instance, membership, stance or Cohesion (FORM-F3+), and every destination is
// still sent as an ordinary single-issuer Move that the server admits and routes on its own.

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <string_view>
#include <vector>

#include "Rules.h"

namespace politika::formation {

using EntityId = std::uint64_t;

// Map-space centimetres.
struct Point {
    double x = 0.0;
    double y = 0.0;
};

enum class Shape : std::uint8_t {
    Line,   // wide frontage, few ranks: direct-fire coverage
    Column, // two files deep along the travel direction: roads and bridges
};

inline constexpr std::size_t kShapeCount = 2;

[[nodiscard]] constexpr std::string_view ShapeName(Shape shape) noexcept {
    return shape == Shape::Column ? "column" : "line";
}

[[nodiscard]] constexpr Shape NextShape(Shape shape) noexcept {
    return shape == Shape::Line ? Shape::Column : Shape::Line;
}

struct Layout {
    // Centre-to-centre distance between neighbouring slots (infantry footprint plus room).
    double spacingCm = 150.0;
    std::size_t lineFrontage = 10;
    std::size_t columnFiles = 2;
    // Below this distance from the group's centroid to the anchor the travel direction is
    // meaningless and the fallback facing is used instead.
    double minTravelCm = 75.0;
};

[[nodiscard]] constexpr std::size_t Frontage(Shape shape, std::size_t count, const Layout& layout) noexcept {
    const std::size_t width = shape == Shape::Column ? layout.columnFiles : layout.lineFrontage;
    return std::max<std::size_t>(1, std::min(count, width));
}

// A slot in the formation frame: `lateral` to the right of the facing, `forward` along it.
// Rank 0 is the front rank.
struct Slot {
    double lateral = 0.0;
    double forward = 0.0;
    std::size_t rank = 0;
};

// The first `count` slots form a block centred on the anchor, rank by rank from the front,
// each rank centred (the last one may be partial). The next `spare` slots extend full-width
// ranks behind it and replace nominal slots that turn out to be illegal.
[[nodiscard]] inline std::vector<Slot> RelativeSlots(Shape shape, std::size_t count, std::size_t spare,
    const Layout& layout) {
    std::vector<Slot> slots;
    if (count == 0) return slots;
    const std::size_t frontage = Frontage(shape, count, layout);
    const std::size_t ranks = (count + frontage - 1) / frontage;
    const double spacing = layout.spacingCm;
    const double centreRank = static_cast<double>(ranks - 1) * 0.5;
    const auto addRank = [&](std::size_t rank, std::size_t width) {
        const double centreFile = static_cast<double>(width - 1) * 0.5;
        for (std::size_t file = 0; file < width; ++file)
            slots.push_back({(static_cast<double>(file) - centreFile) * spacing,
                (centreRank - static_cast<double>(rank)) * spacing, rank});
    };
    slots.reserve(count + spare + frontage);
    for (std::size_t rank = 0; rank < ranks; ++rank)
        addRank(rank, rank + 1 < ranks ? frontage : count - rank * frontage);
    for (std::size_t rank = ranks; slots.size() < count + spare; ++rank) addRank(rank, frontage);
    slots.resize(count + spare);
    return slots;
}

[[nodiscard]] inline bool IsFinite(Point point) noexcept {
    return std::isfinite(point.x) && std::isfinite(point.y);
}

// Unit vector from the members' centroid to the anchor, or `fallback` when the group is
// already standing on the anchor.
[[nodiscard]] inline Point TravelFacing(Point centroid, Point anchor, Point fallback, double minTravel) noexcept {
    const double dx = anchor.x - centroid.x;
    const double dy = anchor.y - centroid.y;
    const double length = std::hypot(dx, dy);
    if (!(length >= minTravel) || !std::isfinite(length) || length == 0.0) return fallback;
    return {dx / length, dy / length};
}

// Right-hand perpendicular of a facing in map space (x forward, y right, as in Unreal).
[[nodiscard]] constexpr Point RightOf(Point facing) noexcept {
    return {-facing.y, facing.x};
}

[[nodiscard]] constexpr Point ToMap(Point anchor, Point facing, double lateral, double forward) noexcept {
    const Point right = RightOf(facing);
    return {anchor.x + right.x * lateral + facing.x * forward, anchor.y + right.y * lateral + facing.y * forward};
}

struct Member {
    EntityId id = 0;
    Point position;
};

inline constexpr std::size_t kNoSlot = std::numeric_limits<std::size_t>::max();

struct Assignment {
    EntityId id = 0;
    Point destination;
    // Index into RelativeSlots, or kNoSlot when no legal slot was left and the member is sent
    // to the anchor itself (the shared-point behaviour of a plain group order).
    std::size_t slot = kNoSlot;
};

struct Plan {
    Shape shape = Shape::Line;
    Point facing{1.0, 0.0};
    // Sorted by member ID; exactly one per distinct member.
    std::vector<Assignment> assignments;
    std::size_t candidatesChecked = 0;
    std::size_t candidatesRejected = 0;
    std::size_t unplaced = 0;
};

// Plans one destination per member. `isLegal(Point)` is the caller's projection check for a
// candidate (map bounds, ground, navigation); it is called at most 2 x members times, in slot
// order. The result depends only on the member set, never on input order:
//  - members are ordered front to back along the facing and left to right inside each rank,
//    and matched to the legal slots in the same order, so neighbours do not cross paths;
//  - a single member goes to the anchor exactly, as an ordinary single-unit order does.
template <class IsLegal>
[[nodiscard]] Plan PlanGroupMove(std::span<const Member> input, Point anchor, Shape shape, IsLegal&& isLegal,
    const Layout& layout = {}, Point fallbackFacing = {1.0, 0.0}) {
    Plan plan;
    plan.shape = shape;
    std::vector<Member> members(input.begin(), input.end());
    std::sort(members.begin(), members.end(), [](const Member& a, const Member& b) { return a.id < b.id; });
    members.erase(std::unique(members.begin(), members.end(), [](const Member& a, const Member& b) { return a.id == b.id; }),
        members.end());
    const auto everyoneToAnchor = [&] {
        for (const Member& member : members) plan.assignments.push_back({member.id, anchor, kNoSlot});
        plan.unplaced = members.size();
        return plan;
    };
    if (members.empty()) return plan;
    if (members.size() > static_cast<std::size_t>(rules::kMaxUnitsPerParticipant) || !IsFinite(anchor)
        || !(layout.spacingCm > 0.0) || !std::isfinite(layout.spacingCm))
        return everyoneToAnchor();
    if (members.size() == 1) {
        plan.assignments.push_back({members.front().id, anchor, 0});
        return plan;
    }

    Point centroid{0.0, 0.0};
    std::size_t finite = 0;
    for (const Member& member : members)
        if (IsFinite(member.position)) {
            centroid.x += member.position.x;
            centroid.y += member.position.y;
            ++finite;
        }
    if (finite == 0) return everyoneToAnchor();
    centroid = {centroid.x / static_cast<double>(finite), centroid.y / static_cast<double>(finite)};
    const Point fallback = IsFinite(fallbackFacing) && std::hypot(fallbackFacing.x, fallbackFacing.y) > 0.0
        ? Point{fallbackFacing.x / std::hypot(fallbackFacing.x, fallbackFacing.y),
              fallbackFacing.y / std::hypot(fallbackFacing.x, fallbackFacing.y)}
        : Point{1.0, 0.0};
    plan.facing = TravelFacing(centroid, anchor, fallback, layout.minTravelCm);

    // Nominal slots first, then spares; keep the first legal ones up to one per member.
    const std::size_t count = members.size();
    const std::vector<Slot> slots = RelativeSlots(shape, count, count, layout);
    struct Chosen {
        std::size_t index;
        Point destination;
    };
    std::vector<Chosen> chosen;
    chosen.reserve(count);
    for (std::size_t index = 0; index < slots.size() && chosen.size() < count; ++index) {
        const Point candidate = ToMap(anchor, plan.facing, slots[index].lateral, slots[index].forward);
        ++plan.candidatesChecked;
        if (IsFinite(candidate) && isLegal(candidate)) chosen.push_back({index, candidate});
        else ++plan.candidatesRejected;
    }

    // Members in the formation frame around their own centroid: front-most first.
    const Point right = RightOf(plan.facing);
    struct Local {
        EntityId id;
        double lateral;
        double forward;
    };
    std::vector<Local> locals;
    locals.reserve(count);
    for (const Member& member : members) {
        const Point offset = IsFinite(member.position)
            ? Point{member.position.x - centroid.x, member.position.y - centroid.y} : Point{0.0, 0.0};
        locals.push_back({member.id, offset.x * right.x + offset.y * right.y,
            offset.x * plan.facing.x + offset.y * plan.facing.y});
    }
    std::sort(locals.begin(), locals.end(), [](const Local& a, const Local& b) {
        if (a.forward != b.forward) return a.forward > b.forward;
        return a.id < b.id;
    });

    // Chosen slots are already in rank order; match rank by rank, left to right.
    std::size_t next = 0;
    for (std::size_t begin = 0; begin < chosen.size();) {
        std::size_t end = begin;
        while (end < chosen.size() && slots[chosen[end].index].rank == slots[chosen[begin].index].rank) ++end;
        std::sort(chosen.begin() + static_cast<std::ptrdiff_t>(begin), chosen.begin() + static_cast<std::ptrdiff_t>(end),
            [&](const Chosen& a, const Chosen& b) { return slots[a.index].lateral < slots[b.index].lateral; });
        const auto rankBegin = locals.begin() + static_cast<std::ptrdiff_t>(next);
        const auto rankEnd = rankBegin + static_cast<std::ptrdiff_t>(end - begin);
        std::sort(rankBegin, rankEnd, [](const Local& a, const Local& b) {
            if (a.lateral != b.lateral) return a.lateral < b.lateral;
            return a.id < b.id;
        });
        for (std::size_t offset = 0; offset < end - begin; ++offset)
            plan.assignments.push_back({locals[next + offset].id, chosen[begin + offset].destination, chosen[begin + offset].index});
        next += end - begin;
        begin = end;
    }
    // Rear-most members left without a legal slot keep the plain shared-point order.
    for (; next < locals.size(); ++next) {
        plan.assignments.push_back({locals[next].id, anchor, kNoSlot});
        ++plan.unplaced;
    }
    std::sort(plan.assignments.begin(), plan.assignments.end(),
        [](const Assignment& a, const Assignment& b) { return a.id < b.id; });
    return plan;
}

// Smallest distance between any two distinct placed destinations (infinity for fewer than two).
[[nodiscard]] inline double MinimumSeparation(const Plan& plan) noexcept {
    double best = std::numeric_limits<double>::infinity();
    for (std::size_t a = 0; a < plan.assignments.size(); ++a)
        for (std::size_t b = a + 1; b < plan.assignments.size(); ++b) {
            if (plan.assignments[a].slot == kNoSlot || plan.assignments[b].slot == kNoSlot) continue;
            const double d = std::hypot(plan.assignments[a].destination.x - plan.assignments[b].destination.x,
                plan.assignments[a].destination.y - plan.assignments[b].destination.y);
            best = std::min(best, d);
        }
    return best;
}

} // namespace politika::formation
