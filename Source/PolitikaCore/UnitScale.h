#pragma once

// Portable planning for the Development unit-scale fixture (MP-2 unit-scale budgets, U-12).
// It carries no gameplay meaning: the UE adapter projects each candidate onto the navmesh
// and spawns the seat's authored archetype there, so bandwidth and server step cost can be
// measured at RTS unit counts.

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <span>
#include <vector>

#include "Rules.h"

namespace politika::unitscale {

// The fixture keeps each seat's two authored placements, so a seat never has fewer.
inline constexpr long long kMinUnitsPerSeat = 2;
// U-12 owner decision: the absolute game-wide limit per participant, for the largest maps.
// No fixture map may exceed it.
inline constexpr long long kGameMaxUnitsPerParticipant = rules::kMaxUnitsPerParticipant;
// U-12 owner decision: the small multiplayer skirmish test map holds at most 20 units per
// participant. The fixture only runs on that map, so this is its limit.
inline constexpr long long kMaxUnitsPerSeat = 20;
static_assert(kMaxUnitsPerSeat <= kGameMaxUnitsPerParticipant);
inline constexpr double kSpacingCm = 100.0;
// Candidates closer than this to an authored placement are skipped.
inline constexpr double kClearanceCm = 60.0;
// Opposing units must spawn at least this far apart: the infantry idle acquisition range
// (800 cm) plus a margin, so the idle phase never starts the fight early (F46).
inline constexpr double kMinOpposingGapCm = 900.0;

struct Point {
    double x = 0.0;
    double y = 0.0;
};

struct Bounds {
    Point min;
    Point max;
};

[[nodiscard]] constexpr bool ValidUnitsPerSeat(long long count) noexcept {
    return count >= kMinUnitsPerSeat && count <= kMaxUnitsPerSeat;
}

[[nodiscard]] inline double DistanceSquared(Point a, Point b) noexcept {
    return (a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y);
}

// Grid cells around `own`, nearest first (ties by row, then column, so the order is
// deterministic), at most `limit` of them. A cell is skipped when it lies outside
// `bounds`, is not strictly closer to `own` than to every start in `others` (so seats
// never interleave), or is within `clearance` of an `occupied` point.
[[nodiscard]] inline std::vector<Point> PlanSeatCandidates(Point own, std::span<const Point> others,
    std::span<const Point> occupied, Bounds bounds, double spacing, double clearance, std::size_t limit) {
    std::vector<Point> out;
    if (!(spacing > 0.0) || limit == 0 || bounds.min.x >= bounds.max.x || bounds.min.y >= bounds.max.y) return out;
    const double reach = std::max({own.x - bounds.min.x, bounds.max.x - own.x, own.y - bounds.min.y, bounds.max.y - own.y});
    if (!(reach >= 0.0)) return out;
    const int rings = static_cast<int>(std::ceil(reach / spacing));
    struct Cell { long long distance; int row; int column; };
    std::vector<Cell> cells;
    cells.reserve(static_cast<std::size_t>(2 * rings + 1) * static_cast<std::size_t>(2 * rings + 1));
    for (int row = -rings; row <= rings; ++row)
        for (int column = -rings; column <= rings; ++column)
            cells.push_back({static_cast<long long>(row) * row + static_cast<long long>(column) * column, row, column});
    std::sort(cells.begin(), cells.end(), [](const Cell& a, const Cell& b) {
        if (a.distance != b.distance) return a.distance < b.distance;
        return a.row != b.row ? a.row < b.row : a.column < b.column;
    });
    const double clearanceSquared = clearance * clearance;
    for (const Cell& cell : cells) {
        const Point candidate{own.x + cell.column * spacing, own.y + cell.row * spacing};
        if (candidate.x < bounds.min.x || candidate.x > bounds.max.x || candidate.y < bounds.min.y
            || candidate.y > bounds.max.y)
            continue;
        const double toOwn = DistanceSquared(candidate, own);
        if (std::any_of(others.begin(), others.end(), [&](Point other) { return DistanceSquared(candidate, other) <= toOwn; }))
            continue;
        if (std::any_of(occupied.begin(), occupied.end(),
                [&](Point point) { return DistanceSquared(candidate, point) < clearanceSquared; }))
            continue;
        out.push_back(candidate);
        if (out.size() == limit) break;
    }
    return out;
}

} // namespace politika::unitscale
