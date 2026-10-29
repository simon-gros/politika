#pragma once

// Engine-independent viewer-side fog-of-war knowledge. Everything here is derived only
// from data a viewer already legitimately holds (its own team's vision sources and the
// contacts it has been sent); it never consults authoritative hidden state. The server
// remains the sole authority for what each connection receives (team vision + relevancy).
#include "Rules.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <span>
#include <utility>
#include <vector>

namespace politika::rules {

// Currently visible cells are a separate mask from explored (historical) terrain.
enum class FogCell : std::uint8_t { Unexplored, Explored, Visible };

struct FogGridSpec {
    Position min;
    Position max;
    double cell_size = 100.0;
};

class FogMap {
public:
    FogMap() = default;
    explicit FogMap(const FogGridSpec& spec) { Reset(spec); }

    void Reset(const FogGridSpec& spec) {
        spec_ = spec;
        if (!(spec_.cell_size > 0.0) || !(spec_.max.x > spec_.min.x) || !(spec_.max.y > spec_.min.y)) {
            columns_ = rows_ = 0;
            cells_.clear();
            return;
        }
        columns_ = static_cast<std::uint32_t>(std::ceil((spec_.max.x - spec_.min.x) / spec_.cell_size));
        rows_ = static_cast<std::uint32_t>(std::ceil((spec_.max.y - spec_.min.y) / spec_.cell_size));
        cells_.assign(static_cast<std::size_t>(columns_) * rows_, FogCell::Unexplored);
    }

    // Recomputes the current mask from `team`'s own sources: a cell is visible when its
    // centre is observed (the same inclusive radius rule the server uses for units).
    // Previously visible cells become explored; explored terrain is never forgotten.
    void Update(std::uint8_t team, std::span<const VisionSource> sources) {
        for (FogCell& cell : cells_)
            if (cell == FogCell::Visible) cell = FogCell::Explored;
        if (team == kNoTeam || cells_.empty()) return;
        for (const VisionSource& source : sources) {
            if (source.team != team || !(source.sight_radius > 0.0)) continue;
            const auto [c0, r0] = CellOf({source.position.x - source.sight_radius, source.position.y - source.sight_radius});
            const auto [c1, r1] = CellOf({source.position.x + source.sight_radius, source.position.y + source.sight_radius});
            const double r2 = source.sight_radius * source.sight_radius;
            for (std::uint32_t row = r0; row <= r1 && row < rows_; ++row)
                for (std::uint32_t column = c0; column <= c1 && column < columns_; ++column) {
                    const Position centre = CellCentre(column, row);
                    const double dx = centre.x - source.position.x;
                    const double dy = centre.y - source.position.y;
                    if (dx * dx + dy * dy <= r2) cells_[Index(column, row)] = FogCell::Visible;
                }
        }
    }

    [[nodiscard]] std::uint32_t Columns() const noexcept { return columns_; }
    [[nodiscard]] std::uint32_t Rows() const noexcept { return rows_; }
    [[nodiscard]] const FogGridSpec& Spec() const noexcept { return spec_; }
    [[nodiscard]] FogCell At(std::uint32_t column, std::uint32_t row) const noexcept {
        return column < columns_ && row < rows_ ? cells_[Index(column, row)] : FogCell::Unexplored;
    }
    [[nodiscard]] Position CellCentre(std::uint32_t column, std::uint32_t row) const noexcept {
        return {spec_.min.x + (column + 0.5) * spec_.cell_size, spec_.min.y + (row + 0.5) * spec_.cell_size};
    }
    [[nodiscard]] std::size_t Count(FogCell state) const noexcept {
        return static_cast<std::size_t>(std::count(cells_.begin(), cells_.end(), state));
    }

private:
    [[nodiscard]] std::size_t Index(std::uint32_t column, std::uint32_t row) const noexcept {
        return static_cast<std::size_t>(row) * columns_ + column;
    }
    // Clamped to the grid; callers bound loops by columns_/rows_.
    [[nodiscard]] std::pair<std::uint32_t, std::uint32_t> CellOf(Position point) const noexcept {
        const auto clamp = [](double value, std::uint32_t count) -> std::uint32_t {
            if (!(value > 0.0) || count == 0) return 0;
            return static_cast<std::uint32_t>(std::min<double>(value, count - 1));
        };
        return {clamp((point.x - spec_.min.x) / spec_.cell_size, columns_),
            clamp((point.y - spec_.min.y) / spec_.cell_size, rows_)};
    }

    FogGridSpec spec_;
    std::uint32_t columns_ = 0;
    std::uint32_t rows_ = 0;
    std::vector<FogCell> cells_;
};

// A remembered contact: where a viewer last legitimately saw a non-friendly entity.
// It is a stale, presentation-only marker — never a command target (orders still need a
// currently observed entity, see OrderRejection::TargetNotVisible).
struct LastKnownContact {
    std::uint64_t entity = 0;
    std::uint32_t owner = 0;
    std::uint8_t team = kNoTeam;
    Position position;
    std::uint64_t last_seen_ms = 0;
};

enum class ContactClearReason : std::uint8_t { Reobserved, Revealed, Died, Reset };

[[nodiscard]] constexpr std::string_view ContactClearReasonName(ContactClearReason reason) noexcept {
    switch (reason) {
        case ContactClearReason::Reobserved: return "reobserved";
        case ContactClearReason::Revealed: return "revealed";
        case ContactClearReason::Died: return "died";
        case ContactClearReason::Reset: return "reset";
    }
    return "unknown";
}

// A remembered position is cleared as "revealed" only when it lies clearly inside the
// viewer's own sight. The margin absorbs replication delay: a contact is withdrawn the
// moment the server judges its *current* position unobserved, so its last replicated
// position can still sit just inside the viewer's radius.
inline constexpr double kContactRevealMarginCm = 150.0;

class ContactMemory {
public:
    // The viewer stopped receiving a contact. Only a contact last known alive is
    // remembered; one whose death the viewer saw is forgotten. A death the viewer did not
    // see is not revealed: that contact simply stays remembered.
    enum class Withdrawal : std::uint8_t { Ignored, Recorded, ForgottenDead };
    Withdrawal Withdrawn(const LastKnownContact& contact, bool last_known_alive) {
        if (contact.entity == 0) return Withdrawal::Ignored;
        if (!last_known_alive) return Erase(contact.entity) ? Withdrawal::ForgottenDead : Withdrawal::Ignored;
        if (LastKnownContact* existing = FindMutable(contact.entity)) *existing = contact;
        else contacts_.push_back(contact);
        return Withdrawal::Recorded;
    }

    // The contact is being received again; its live entity replaces the memory.
    bool Reobserved(std::uint64_t entity) { return Erase(entity); }

    // Drops remembered contacts whose position the viewer now clearly sees: had the entity
    // still been there, the server would be sending it. Cleared IDs are appended to `out`.
    template <typename Out>
    void ClearRevealed(std::uint8_t team, std::span<const VisionSource> sources, Out&& out) {
        std::erase_if(contacts_, [&](const LastKnownContact& contact) {
            if (!IsClearlyObserved(team, contact.position, sources)) return false;
            out(contact);
            return true;
        });
    }

    template <typename Out>
    void Clear(Out&& out) {
        for (const LastKnownContact& contact : contacts_) out(contact);
        contacts_.clear();
    }

    [[nodiscard]] std::span<const LastKnownContact> Contacts() const noexcept { return contacts_; }
    [[nodiscard]] const LastKnownContact* Find(std::uint64_t entity) const noexcept {
        const auto it = std::find_if(contacts_.begin(), contacts_.end(),
            [entity](const LastKnownContact& contact) { return contact.entity == entity; });
        return it == contacts_.end() ? nullptr : &*it;
    }

    [[nodiscard]] static bool IsClearlyObserved(std::uint8_t team, Position target,
        std::span<const VisionSource> sources) noexcept {
        if (team == kNoTeam) return false;
        for (const VisionSource& source : sources) {
            const double radius = source.sight_radius - kContactRevealMarginCm;
            if (source.team != team || !(radius > 0.0)) continue;
            const double dx = source.position.x - target.x;
            const double dy = source.position.y - target.y;
            if (dx * dx + dy * dy <= radius * radius) return true;
        }
        return false;
    }

private:
    LastKnownContact* FindMutable(std::uint64_t entity) noexcept {
        const auto it = std::find_if(contacts_.begin(), contacts_.end(),
            [entity](const LastKnownContact& contact) { return contact.entity == entity; });
        return it == contacts_.end() ? nullptr : &*it;
    }
    bool Erase(std::uint64_t entity) {
        return std::erase_if(contacts_, [entity](const LastKnownContact& c) { return c.entity == entity; }) > 0;
    }

    std::vector<LastKnownContact> contacts_;
};

} // namespace politika::rules
