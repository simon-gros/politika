#pragma once

// Portable group-move geometry (RTS_UNIT_FORMATIONS_AND_DISCIPLINE.md §4 and §10, FORM-F2;
// Dispersed and Perimeter FORM-F6). A selected group ordered to a point gets one distinct
// destination per member, laid out as a Line, a Column or a staggered Dispersed block facing the
// direction of travel, or as a Perimeter ring around the point. This is geometry only: formation identity,
// membership and regroup live in Formation.h (FORM-F3), and every destination is still sent
// as an ordinary single-issuer Move that the server admits and routes on its own.

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
    Line,      // wide frontage, few ranks: direct-fire coverage
    Column,    // two files deep along the travel direction: roads and bridges
    Dispersed, // staggered ranks 450 cm apart: a blast catches one soldier, but no fellow member
               // stands within the 400 cm Discipline support radius (Discipline.h)
    Perimeter, // a ring around the anchor: all-round defence of a point, fire divided
};

inline constexpr std::size_t kShapeCount = 4;

[[nodiscard]] constexpr std::string_view ShapeName(Shape shape) noexcept {
    switch (shape) {
    case Shape::Column: return "column";
    case Shape::Dispersed: return "dispersed";
    case Shape::Perimeter: return "perimeter";
    case Shape::Line: break;
    }
    return "line";
}

// The V key's cycle: Line, Column, Dispersed, Perimeter, Line.
[[nodiscard]] constexpr Shape NextShape(Shape shape) noexcept {
    const auto next = static_cast<std::uint8_t>((static_cast<std::size_t>(shape) + 1) % kShapeCount);
    return static_cast<std::size_t>(shape) < kShapeCount ? static_cast<Shape>(next) : Shape::Line;
}

struct Layout {
    // Centre-to-centre distance between neighbouring slots (infantry footprint plus room); also
    // the minimum chord between neighbours on a Perimeter ring and between its rings.
    double spacingCm = 150.0;
    std::size_t lineFrontage = 10;
    std::size_t columnFiles = 2;
    // Dispersed: wider than the 400 cm support radius, so members hold their Discipline instead
    // of earning it; odd ranks are shifted half a spacing sideways.
    double dispersedSpacingCm = 450.0;
    std::size_t dispersedFrontage = 5;
    // Perimeter: one ring as small as the spacing allows but never tighter than the minimum
    // radius; beyond the maximum radius further rings fill inwards, then outwards.
    double perimeterMinRadiusCm = 300.0;
    double perimeterMaxRadiusCm = 1200.0;
    // Below this distance from the group's centroid to the anchor the travel direction is
    // meaningless and the fallback facing is used instead.
    double minTravelCm = 75.0;
};

// Distance between neighbouring slots of a shape.
[[nodiscard]] constexpr double ShapeSpacing(Shape shape, const Layout& layout) noexcept {
    return shape == Shape::Dispersed ? layout.dispersedSpacingCm : layout.spacingCm;
}

// Files per rank of the lattice shapes (Line, Column, Dispersed). A Perimeter has no ranks of
// files; its rings are described by PerimeterRadius and RingCapacity.
[[nodiscard]] constexpr std::size_t Frontage(Shape shape, std::size_t count, const Layout& layout) noexcept {
    const std::size_t width = shape == Shape::Column ? layout.columnFiles
        : shape == Shape::Dispersed                  ? layout.dispersedFrontage
                                                     : layout.lineFrontage;
    return std::max<std::size_t>(1, std::min(count, width));
}

inline constexpr double kPi = 3.14159265358979323846;

// The most slots a ring of `radius` holds with every neighbouring chord at least `spacing`.
[[nodiscard]] inline std::size_t RingCapacity(double radius, double spacing) noexcept {
    if (!(radius > 0.0) || !(spacing > 0.0) || !std::isfinite(radius) || !std::isfinite(spacing)) return 0;
    if (spacing > 2.0 * radius) return 1;
    const double capacity = std::floor(kPi / std::asin(spacing / (2.0 * radius)) + 1e-9);
    return capacity > 1e6 ? std::size_t{1000000} : static_cast<std::size_t>(capacity);
}

// Radius of a single ring of `count` slots: neighbouring chords exactly one spacing, but never
// smaller than the minimum radius.
[[nodiscard]] inline double PerimeterRadius(std::size_t count, const Layout& layout) noexcept {
    const double tight = count < 2 ? 0.0 : layout.spacingCm / (2.0 * std::sin(kPi / static_cast<double>(count)));
    return std::max(layout.perimeterMinRadiusCm, tight);
}

// A slot in the formation frame: `lateral` to the right of the facing, `forward` along it.
// Rank 0 is the front rank; on a Perimeter the rank is the ring, in the order rings are filled.
struct Slot {
    double lateral = 0.0;
    double forward = 0.0;
    std::size_t rank = 0;
};

// Perimeter slots: `count` nominal slots on rings around the anchor, then `spare` slots on rings
// outside the outermost nominal ring. A ring's slots are evenly spaced, clockwise from the front
// (lateral = r sin a, forward = r cos a), and every chord between neighbours and every gap
// between rings is at least one spacing.
[[nodiscard]] inline std::vector<Slot> RingSlots(std::size_t count, std::size_t spare, const Layout& layout) {
    std::vector<Slot> slots;
    const double spacing = layout.spacingCm;
    if (count == 0 || !(spacing > 0.0) || !std::isfinite(spacing) || !(layout.perimeterMinRadiusCm > 0.0)
        || !std::isfinite(layout.perimeterMaxRadiusCm) || layout.perimeterMaxRadiusCm < layout.perimeterMinRadiusCm)
        return slots;
    slots.reserve(count + spare);
    std::size_t rank = 0;
    const auto addRing = [&](double radius, std::size_t n) {
        for (std::size_t index = 0; index < n; ++index) {
            const double angle = 2.0 * kPi * static_cast<double>(index) / static_cast<double>(n);
            slots.push_back({radius * std::sin(angle), radius * std::cos(angle), rank});
        }
        ++rank;
    };
    double outermost = 0.0;
    const double single = PerimeterRadius(count, layout);
    if (single <= layout.perimeterMaxRadiusCm) {
        addRing(single, count);
        outermost = single;
    } else {
        // Full rings from the maximum radius inwards, then outwards; the last ring is partial.
        std::size_t left = count;
        for (double radius = layout.perimeterMaxRadiusCm; left > 0 && radius >= layout.perimeterMinRadiusCm - 1e-9;
             radius -= spacing) {
            const std::size_t n = std::min(left, RingCapacity(radius, spacing));
            addRing(radius, n);
            left -= n;
        }
        outermost = layout.perimeterMaxRadiusCm;
        while (left > 0) {
            outermost += spacing;
            const std::size_t n = std::min(left, RingCapacity(outermost, spacing));
            addRing(outermost, n);
            left -= n;
        }
    }
    for (std::size_t left = spare; left > 0;) {
        outermost += spacing;
        const std::size_t n = std::min(left, RingCapacity(outermost, spacing));
        addRing(outermost, n);
        left -= n;
    }
    return slots;
}

// Line, Column and Dispersed: the first `count` slots form a block centred on the anchor, rank by
// rank from the front, each rank centred (the last one may be partial). The next `spare` slots
// extend full-width ranks behind it and replace nominal slots that turn out to be illegal.
// Dispersed ranks are staggered: odd ranks sit half a spacing to the right, and the block is
// shifted back by a quarter spacing so it stays centred. Perimeter slots come from RingSlots.
[[nodiscard]] inline std::vector<Slot> RelativeSlots(Shape shape, std::size_t count, std::size_t spare,
    const Layout& layout) {
    if (shape == Shape::Perimeter) return RingSlots(count, spare, layout);
    std::vector<Slot> slots;
    if (count == 0) return slots;
    const std::size_t frontage = Frontage(shape, count, layout);
    const std::size_t ranks = (count + frontage - 1) / frontage;
    const double spacing = ShapeSpacing(shape, layout);
    const double centreRank = static_cast<double>(ranks - 1) * 0.5;
    const bool staggered = shape == Shape::Dispersed && ranks > 1;
    const auto addRank = [&](std::size_t rank, std::size_t width) {
        const double centreFile = static_cast<double>(width - 1) * 0.5;
        const double stagger = staggered ? ((rank % 2 == 1 ? 0.5 : 0.0) - 0.25) * spacing : 0.0;
        for (std::size_t file = 0; file < width; ++file)
            slots.push_back({(static_cast<double>(file) - centreFile) * spacing + stagger,
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
//  - lattice shapes: members are ordered front to back along the facing and left to right inside
//    each rank, and matched to the legal slots in the same order, so neighbours do not cross paths;
//  - Perimeter: members and legal slots are both ordered clockwise around the anchor and matched
//    in that cyclic order, at the rotation with the least total squared travel (ties: the lowest
//    rotation); when legal slots run short, the members farthest from the anchor are unplaced;
//  - a single member goes to the anchor exactly, as an ordinary single-unit order does.
// `fixedFacing`, when finite and non-zero, replaces the travel direction (a regroup keeps the
// facing of the move it repairs instead of turning towards wherever the survivors stand).
template <class IsLegal>
[[nodiscard]] Plan PlanGroupMove(std::span<const Member> input, Point anchor, Shape shape, IsLegal&& isLegal,
    const Layout& layout = {}, Point fallbackFacing = {1.0, 0.0}, Point fixedFacing = {0.0, 0.0}) {
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
        || static_cast<std::size_t>(shape) >= kShapeCount || !(layout.spacingCm > 0.0) || !std::isfinite(layout.spacingCm)
        || !(ShapeSpacing(shape, layout) > 0.0) || !std::isfinite(ShapeSpacing(shape, layout)))
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
    const double fixedLength = std::hypot(fixedFacing.x, fixedFacing.y);
    plan.facing = IsFinite(fixedFacing) && fixedLength > 0.0 && std::isfinite(fixedLength)
        ? Point{fixedFacing.x / fixedLength, fixedFacing.y / fixedLength}
        : TravelFacing(centroid, anchor, fallback, layout.minTravelCm);

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

    if (shape == Shape::Perimeter) {
        // Clockwise angle around the anchor from the facing, in [0, 2 pi).
        const Point right = RightOf(plan.facing);
        const auto clockwise = [](double lateral, double forward) {
            const double angle = std::atan2(lateral, forward);
            return angle < 0.0 ? angle + 2.0 * kPi : angle;
        };
        struct Around {
            EntityId id;
            Point position;
            double angle;
            double distance;
        };
        std::vector<Around> around;
        around.reserve(count);
        for (const Member& member : members) {
            const Point position = IsFinite(member.position) ? member.position : centroid;
            const double dx = position.x - anchor.x;
            const double dy = position.y - anchor.y;
            around.push_back({member.id, position, clockwise(dx * right.x + dy * right.y, dx * plan.facing.x + dy * plan.facing.y),
                std::hypot(dx, dy)});
        }
        // Too few legal slots: the members nearest the anchor are placed, the rest keep the
        // plain shared-point order.
        std::sort(around.begin(), around.end(), [](const Around& a, const Around& b) {
            if (a.distance != b.distance) return a.distance < b.distance;
            return a.id < b.id;
        });
        const std::size_t placed = chosen.size();
        for (std::size_t index = placed; index < around.size(); ++index) {
            plan.assignments.push_back({around[index].id, anchor, kNoSlot});
            ++plan.unplaced;
        }
        around.resize(placed);
        std::sort(around.begin(), around.end(), [](const Around& a, const Around& b) {
            if (a.angle != b.angle) return a.angle < b.angle;
            return a.id < b.id;
        });
        std::vector<std::size_t> ring(placed);
        std::vector<double> slotAngle(placed);
        for (std::size_t index = 0; index < placed; ++index) {
            ring[index] = index;
            slotAngle[index] = clockwise(slots[chosen[index].index].lateral, slots[chosen[index].index].forward);
        }
        std::sort(ring.begin(), ring.end(), [&](std::size_t a, std::size_t b) {
            if (slotAngle[a] != slotAngle[b]) return slotAngle[a] < slotAngle[b];
            return chosen[a].index < chosen[b].index;
        });
        const auto squared = [](Point a, Point b) { return (a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y); };
        std::size_t bestRotation = 0;
        double bestCost = std::numeric_limits<double>::infinity();
        for (std::size_t rotation = 0; rotation < placed; ++rotation) {
            double cost = 0.0;
            for (std::size_t index = 0; index < placed && cost < bestCost; ++index)
                cost += squared(around[index].position, chosen[ring[(index + rotation) % placed]].destination);
            if (cost < bestCost) {
                bestCost = cost;
                bestRotation = rotation;
            }
        }
        for (std::size_t index = 0; index < placed; ++index) {
            const Chosen& slot = chosen[ring[(index + bestRotation) % placed]];
            plan.assignments.push_back({around[index].id, slot.destination, slot.index});
        }
        std::sort(plan.assignments.begin(), plan.assignments.end(),
            [](const Assignment& a, const Assignment& b) { return a.id < b.id; });
        return plan;
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
