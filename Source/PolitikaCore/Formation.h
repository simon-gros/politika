#pragma once

// Match-owned formation instances (RTS_UNIT_FORMATIONS_AND_DISCIPLINE.md §3, §3.2 and §7,
// FORM-F3): a formation ID, its owning participant, its sorted member IDs, its shape and the
// last formation move, plus explicit Form/Release transitions, member loss and one bounded
// regroup after losses. Selection and control groups never create one; only an admitted Form
// does. Each formation carries a combat stance and an adherence (FORM-F4, Stance.h); there is
// no Cohesion or Discipline effect (FORM-F5+), and the registry never moves a unit itself: it returns per-member destinations that the caller
// submits as ordinary, individually admitted Move/AttackMove commands.

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string_view>
#include <vector>

#include "FormationGeometry.h"
#include "Rules.h"
#include "Stance.h"

namespace politika::formation {

// Provisional size bounds (open decision §12). Two units are the smallest formation; 64 keeps
// one formation move within a bounded number of projections and path queries.
inline constexpr std::size_t kMinFormationMembers = 2;
inline constexpr std::size_t kMaxFormationMembers = 64;
inline constexpr std::size_t kMaxFormationsPerParticipant = 10;
// A Release names at most one participant's whole force.
inline constexpr std::size_t kMaxReleaseIds = static_cast<std::size_t>(rules::kMaxUnitsPerParticipant);
// Regroup hysteresis in authoritative steps (0.05 s): losses within this window share one
// regroup, and two regroups of one formation are at least this far apart.
inline constexpr std::uint64_t kRegroupDelaySteps = 40;
// A regroup re-sends a member only when its slot moved at least this far.
inline constexpr double kRegroupMinShiftCm = 1.0;

// Formation IDs are their own namespace: never a runtime entity ID or a definition ID.
struct FormationId {
    std::uint32_t value = 0;
    [[nodiscard]] constexpr explicit operator bool() const noexcept { return value != 0; }
    friend constexpr bool operator==(FormationId, FormationId) noexcept = default;
    friend constexpr auto operator<=>(FormationId, FormationId) noexcept = default;
};

// Organized: every member holds its slot assignment. Degraded: a member was lost since the
// last plan and the ranks have not closed yet. A dissolved formation no longer exists.
enum class FormationState : std::uint8_t { Organized, Degraded };

[[nodiscard]] constexpr std::string_view StateName(FormationState state) noexcept {
    return state == FormationState::Degraded ? "degraded" : "organized";
}

// SetStance addresses a whole formation (its stance; member overrides are cleared) or a list
// of units (per-unit overrides). SetAdherence addresses a formation only.
enum class FormationOp : std::uint8_t { Form, Release, Move, AttackMove, Stop, SetStance, SetAdherence };
inline constexpr std::uint8_t kFormationOpCount = 7;

[[nodiscard]] constexpr std::string_view OpName(FormationOp op) noexcept {
    switch (op) {
        case FormationOp::Form: return "form";
        case FormationOp::Release: return "release";
        case FormationOp::Move: return "move";
        case FormationOp::AttackMove: return "attack_move";
        case FormationOp::Stop: return "stop";
        case FormationOp::SetStance: return "set_stance";
        case FormationOp::SetAdherence: return "set_adherence";
    }
    return "invalid";
}

// Separate from rules::OrderRejection so neither enum's stable numeric codes move.
enum class FormationRejection : std::uint8_t {
    None,
    MalformedCommand,
    DuplicateCommand,
    RateLimited,
    MatchInactive,
    StaleMatch,
    Unauthorized,
    TooFewMembers,
    TooManyMembers,
    DuplicateMember,
    // Unknown, dead, foreign, non-operational or immobile member.
    MemberUnavailable,
    // Unknown ID or another participant's formation: one code, so it is not an existence oracle.
    UnknownFormation,
    FormationLimit,
    NotMember,
    InvalidDestination,
    // A civilian or other non-combat unit: only military units serve in formations (DISC-D1).
    NotMilitary,
};
inline constexpr std::size_t kFormationRejectionCount = static_cast<std::size_t>(FormationRejection::NotMilitary) + 1;

enum class LeaveReason : std::uint8_t { Destroyed, Released, Reformed, Unavailable };
enum class DissolveReason : std::uint8_t { Released, TooFewMembers, Reformed, MatchEnded };

[[nodiscard]] constexpr std::string_view LeaveReasonName(LeaveReason reason) noexcept {
    switch (reason) {
        case LeaveReason::Destroyed: return "destroyed";
        case LeaveReason::Released: return "released";
        case LeaveReason::Reformed: return "reformed";
        case LeaveReason::Unavailable: return "unavailable";
    }
    return "invalid";
}

[[nodiscard]] constexpr std::string_view DissolveReasonName(DissolveReason reason) noexcept {
    switch (reason) {
        case DissolveReason::Released: return "released";
        case DissolveReason::TooFewMembers: return "too_few_members";
        case DissolveReason::Reformed: return "reformed";
        case DissolveReason::MatchEnded: return "match_ended";
    }
    return "invalid";
}

// Bounded remote payload (transport adapter fields only; participant, generation and
// ownership are resolved by the server).
struct IntentPayload {
    std::uint8_t op = 0;
    std::uint32_t formation_id = 0;
    std::uint8_t shape = 0;
    std::span<const EntityId> members;
    double x = 0.0;
    double y = 0.0;
    // SetStance: stance::CombatStance; SetAdherence: stance::Adherence; zero otherwise.
    std::uint8_t value = 0;
};

// Shape check only: unused fields must be zero and member lists bounded, unique and non-zero.
// Ownership, liveness and map bounds are checked against authoritative state afterwards.
[[nodiscard]] inline FormationRejection CheckIntentShape(const IntentPayload& payload) noexcept {
    if (payload.op >= kFormationOpCount || !std::isfinite(payload.x) || !std::isfinite(payload.y))
        return FormationRejection::MalformedCommand;
    const auto op = static_cast<FormationOp>(payload.op);
    const bool hasPosition = payload.x != 0.0 || payload.y != 0.0;
    if (op == FormationOp::SetStance || op == FormationOp::SetAdherence) {
        const bool valid = op == FormationOp::SetStance ? stance::IsValidStance(payload.value)
                                                        : stance::IsValidAdherence(payload.value);
        if (!valid || payload.shape != 0 || hasPosition) return FormationRejection::MalformedCommand;
        // Exactly one address: a formation, or (SetStance only) a list of units.
        if (payload.formation_id != 0) return payload.members.empty() ? FormationRejection::None : FormationRejection::MalformedCommand;
        if (op == FormationOp::SetAdherence || payload.members.empty() || payload.members.size() > kMaxReleaseIds)
            return FormationRejection::MalformedCommand;
        std::vector<EntityId> sorted(payload.members.begin(), payload.members.end());
        std::sort(sorted.begin(), sorted.end());
        if (sorted.front() == 0) return FormationRejection::MalformedCommand;
        if (std::adjacent_find(sorted.begin(), sorted.end()) != sorted.end()) return FormationRejection::DuplicateMember;
        return FormationRejection::None;
    }
    if (payload.value != 0) return FormationRejection::MalformedCommand;
    const bool takesMembers = op == FormationOp::Form || op == FormationOp::Release;
    if (takesMembers) {
        if (payload.formation_id != 0 || hasPosition) return FormationRejection::MalformedCommand;
        if (op == FormationOp::Release && payload.shape != 0) return FormationRejection::MalformedCommand;
        if (op == FormationOp::Form && payload.shape >= kShapeCount) return FormationRejection::MalformedCommand;
        const std::size_t limit = op == FormationOp::Form ? kMaxFormationMembers : kMaxReleaseIds;
        if (payload.members.size() > limit)
            return op == FormationOp::Form ? FormationRejection::TooManyMembers : FormationRejection::MalformedCommand;
        if (payload.members.empty()) return op == FormationOp::Form ? FormationRejection::TooFewMembers : FormationRejection::MalformedCommand;
        std::vector<EntityId> sorted(payload.members.begin(), payload.members.end());
        std::sort(sorted.begin(), sorted.end());
        if (sorted.front() == 0) return FormationRejection::MalformedCommand;
        if (std::adjacent_find(sorted.begin(), sorted.end()) != sorted.end()) return FormationRejection::DuplicateMember;
        if (op == FormationOp::Form && sorted.size() < kMinFormationMembers) return FormationRejection::TooFewMembers;
        return FormationRejection::None;
    }
    if (payload.formation_id == 0 || !payload.members.empty() || payload.shape != 0)
        return FormationRejection::MalformedCommand;
    if (op == FormationOp::Stop && hasPosition) return FormationRejection::MalformedCommand;
    return FormationRejection::None;
}

struct MemberSlot {
    EntityId id = 0;
    // Slot index and map destination of the latest formation plan; kNoSlot before any move.
    std::size_t slot = kNoSlot;
    Point destination;
    // True while the member's current order is this formation's move. An individual order
    // clears it: the member stays in the formation but a regroup no longer moves it.
    bool following = false;
};

struct Formation {
    FormationId id;
    std::uint32_t owner = 0;
    Shape shape = Shape::Line;
    stance::CombatStance stance = stance::kDefaultStance;
    stance::Adherence adherence = stance::kDefaultAdherence;
    std::vector<MemberSlot> members; // sorted by ID, unique
    FormationState state = FormationState::Organized;
    std::size_t formedMembers = 0;
    // The latest formation Move/AttackMove; a regroup reuses its anchor and facing.
    bool hasDestination = false;
    bool attackMove = false;
    Point anchor;
    Point facing{1.0, 0.0};
    std::uint64_t regroupDueStep = 0; // 0: none pending
    std::uint64_t lastRegroupStep = 0;
    std::uint32_t regroups = 0;
    std::uint32_t losses = 0;

    [[nodiscard]] const MemberSlot* Find(EntityId member) const noexcept {
        const auto found = std::lower_bound(members.begin(), members.end(), member,
            [](const MemberSlot& slot, EntityId value) { return slot.id < value; });
        return found != members.end() && found->id == member ? &*found : nullptr;
    }
    [[nodiscard]] MemberSlot* Find(EntityId member) noexcept {
        return const_cast<MemberSlot*>(static_cast<const Formation*>(this)->Find(member));
    }
    [[nodiscard]] std::size_t FollowingCount() const noexcept {
        return static_cast<std::size_t>(std::count_if(members.begin(), members.end(),
            [](const MemberSlot& slot) { return slot.following; }));
    }
};

// Everything one registry call changed, for logs, replication and feedback.
struct Change {
    struct Left {
        FormationId formation;
        EntityId member = 0;
        LeaveReason reason = LeaveReason::Released;
    };
    struct Dissolved {
        FormationId formation;
        std::uint32_t owner = 0;
        DissolveReason reason = DissolveReason::Released;
    };
    FormationRejection rejection = FormationRejection::None;
    FormationId formed;
    std::vector<Left> left;
    std::vector<Dissolved> dissolved;
    struct Scheduled {
        FormationId formation;
        std::uint64_t dueStep = 0;
    };
    // Regroups newly scheduled by this change.
    std::vector<Scheduled> regroups;
    [[nodiscard]] bool Accepted() const noexcept { return rejection == FormationRejection::None; }
};

// One planned destination to submit as an ordinary member command.
struct MemberOrder {
    EntityId id = 0;
    Point destination;
    std::size_t slot = kNoSlot;
};

struct MovePlan {
    FormationRejection rejection = FormationRejection::None;
    Plan plan;
    std::vector<MemberOrder> orders; // sorted by member ID
};

// All formations of one match generation. Formation IDs only increase within a registry and
// Clear() retires every one of them, so a later generation cannot inherit a stale handle.
class Registry {
public:
    [[nodiscard]] const std::vector<Formation>& All() const noexcept { return formations_; }
    [[nodiscard]] bool Empty() const noexcept { return formations_.empty(); }

    [[nodiscard]] const Formation* Find(FormationId id) const noexcept {
        for (const Formation& formation : formations_)
            if (formation.id == id) return &formation;
        return nullptr;
    }
    // The participant's own formation, or null for an unknown or foreign ID.
    [[nodiscard]] const Formation* FindOwned(FormationId id, std::uint32_t owner) const noexcept {
        const Formation* formation = Find(id);
        return formation && owner != 0 && formation->owner == owner ? formation : nullptr;
    }
    [[nodiscard]] FormationId FormationOf(EntityId member) const noexcept {
        for (const Formation& formation : formations_)
            if (formation.Find(member)) return formation.id;
        return {};
    }
    [[nodiscard]] std::size_t CountOwned(std::uint32_t owner) const noexcept {
        return static_cast<std::size_t>(std::count_if(formations_.begin(), formations_.end(),
            [owner](const Formation& formation) { return formation.owner == owner; }));
    }

    // Admits a new formation of `members` (authoritative entity snapshots) for `participant`.
    // Members already in another formation move into the new one; a formation left with fewer
    // than two members dissolves; one that keeps two or more closes its ranks like after a
    // loss. Rejections change nothing.
    [[nodiscard]] Change Form(std::uint32_t participant, std::span<const rules::Entity> members, Shape shape,
        std::uint64_t step, stance::CombatStance initialStance = stance::kDefaultStance) {
        Change change;
        if (participant == 0) return Reject(change, FormationRejection::Unauthorized);
        if (static_cast<std::uint8_t>(shape) >= kShapeCount || !stance::IsValidStance(static_cast<std::uint8_t>(initialStance)))
            return Reject(change, FormationRejection::MalformedCommand);
        if (members.size() < kMinFormationMembers) return Reject(change, FormationRejection::TooFewMembers);
        if (members.size() > kMaxFormationMembers) return Reject(change, FormationRejection::TooManyMembers);
        std::vector<EntityId> ids;
        ids.reserve(members.size());
        for (const rules::Entity& entity : members) {
            if (entity.id == 0 || entity.owner_player != participant || !entity.alive || !entity.operational
                || !entity.movable)
                return Reject(change, FormationRejection::MemberUnavailable);
            if (!entity.military) return Reject(change, FormationRejection::NotMilitary);
            ids.push_back(entity.id);
        }
        std::sort(ids.begin(), ids.end());
        if (std::adjacent_find(ids.begin(), ids.end()) != ids.end()) return Reject(change, FormationRejection::DuplicateMember);
        // Formation limit after the formations this Form empties have dissolved.
        std::size_t remainingOwned = 0;
        for (const Formation& formation : formations_) {
            if (formation.owner != participant) continue;
            const std::size_t staying = static_cast<std::size_t>(std::count_if(formation.members.begin(), formation.members.end(),
                [&ids](const MemberSlot& slot) { return !std::binary_search(ids.begin(), ids.end(), slot.id); }));
            remainingOwned += staying >= kMinFormationMembers ? 1 : 0;
        }
        if (remainingOwned >= kMaxFormationsPerParticipant) return Reject(change, FormationRejection::FormationLimit);

        for (EntityId id : ids) RemoveMember(id, LeaveReason::Reformed, DissolveReason::Reformed, step, change);
        Formation formation;
        formation.id = FormationId{nextId_++};
        formation.owner = participant;
        formation.shape = shape;
        formation.stance = initialStance;
        formation.formedMembers = ids.size();
        for (EntityId id : ids) formation.members.push_back({id, kNoSlot, {}, false});
        change.formed = formation.id;
        formations_.push_back(std::move(formation));
        return change;
    }

    // Removes the participant's listed units from their formations. IDs that are not members
    // of one of the participant's formations are ignored; if none is, nothing changes.
    [[nodiscard]] Change Release(std::uint32_t participant, std::span<const EntityId> ids, std::uint64_t step) {
        Change change;
        if (participant == 0) return Reject(change, FormationRejection::Unauthorized);
        std::vector<EntityId> members;
        for (EntityId id : ids) {
            const FormationId formation = FormationOf(id);
            if (formation && Find(formation)->owner == participant) members.push_back(id);
        }
        if (members.empty()) return Reject(change, FormationRejection::NotMember);
        std::sort(members.begin(), members.end());
        members.erase(std::unique(members.begin(), members.end()), members.end());
        for (EntityId id : members) RemoveMember(id, LeaveReason::Released, DissolveReason::Released, step, change);
        return change;
    }

    // A member died or left authoritative control. Losing the second-to-last member
    // dissolves the formation; otherwise it is Degraded and, when it has a destination and
    // members still following it, one regroup is scheduled (coalesced and rate-limited).
    Change MemberLost(EntityId member, std::uint64_t step, LeaveReason reason = LeaveReason::Destroyed) {
        Change change;
        if (!FormationOf(member)) return Reject(change, FormationRejection::NotMember);
        RemoveMember(member, reason, DissolveReason::TooFewMembers, step, change);
        return change;
    }

    // Plans a formation Move/AttackMove for every current member: a Line/Column block facing
    // the travel direction, stored as the formation's destination. Every member is marked as
    // following; the caller submits one ordinary command per returned order.
    template <class IsLegal, class PositionOf>
    [[nodiscard]] MovePlan PlanMove(std::uint32_t participant, FormationId id, Point anchor, bool attackMove,
        PositionOf&& positionOf, IsLegal&& isLegal, const Layout& layout = {}) {
        MovePlan result;
        Formation* formation = MutableOwned(id, participant);
        if (!formation) { result.rejection = FormationRejection::UnknownFormation; return result; }
        if (!IsFinite(anchor)) { result.rejection = FormationRejection::InvalidDestination; return result; }
        std::vector<Member> members;
        members.reserve(formation->members.size());
        for (const MemberSlot& slot : formation->members) members.push_back({slot.id, positionOf(slot.id)});
        result.plan = PlanGroupMove(members, anchor, formation->shape, isLegal, layout);
        formation->hasDestination = true;
        formation->attackMove = attackMove;
        formation->anchor = anchor;
        formation->facing = result.plan.facing;
        formation->regroupDueStep = 0;
        formation->state = FormationState::Organized;
        for (const Assignment& assignment : result.plan.assignments) {
            MemberSlot* slot = formation->Find(assignment.id);
            if (!slot) continue;
            slot->slot = assignment.slot;
            slot->destination = assignment.destination;
            slot->following = true;
            result.orders.push_back({assignment.id, assignment.destination, assignment.slot});
        }
        return result;
    }

    // Formation stance: every member without an override fights by it.
    [[nodiscard]] FormationRejection SetStance(std::uint32_t participant, FormationId id, stance::CombatStance value) {
        Formation* formation = MutableOwned(id, participant);
        if (!formation) return FormationRejection::UnknownFormation;
        if (!stance::IsValidStance(static_cast<std::uint8_t>(value))) return FormationRejection::MalformedCommand;
        formation->stance = value;
        return FormationRejection::None;
    }

    // Formation adherence. Free gives up closing the ranks, so a pending regroup is cancelled.
    [[nodiscard]] FormationRejection SetAdherence(std::uint32_t participant, FormationId id, stance::Adherence value) {
        Formation* formation = MutableOwned(id, participant);
        if (!formation) return FormationRejection::UnknownFormation;
        if (!stance::IsValidAdherence(static_cast<std::uint8_t>(value))) return FormationRejection::MalformedCommand;
        formation->adherence = value;
        if (value == stance::Adherence::Free) formation->regroupDueStep = 0;
        return FormationRejection::None;
    }

    // Formation Stop: forget the destination (nothing to regroup towards) and stop following.
    [[nodiscard]] FormationRejection Stop(std::uint32_t participant, FormationId id) {
        Formation* formation = MutableOwned(id, participant);
        if (!formation) return FormationRejection::UnknownFormation;
        formation->hasDestination = false;
        formation->regroupDueStep = 0;
        for (MemberSlot& slot : formation->members) slot.following = false;
        return FormationRejection::None;
    }

    // An order for `member` was admitted. A formation order keeps it following while the
    // formation has a destination; any other order (fromFormation is none or another ID)
    // makes it independent, so a later regroup leaves its route alone. Returns true when the
    // member stopped following.
    bool NoteOrderAdmitted(EntityId member, FormationId fromFormation) noexcept {
        for (Formation& formation : formations_) {
            MemberSlot* slot = formation.Find(member);
            if (!slot) continue;
            const bool wasFollowing = slot->following;
            slot->following = fromFormation == formation.id && formation.hasDestination;
            return wasFollowing && !slot->following;
        }
        return false;
    }

    // Formations whose regroup is due at `step`, in ID order.
    [[nodiscard]] std::vector<FormationId> DueRegroups(std::uint64_t step) const {
        std::vector<FormationId> due;
        for (const Formation& formation : formations_)
            if (formation.regroupDueStep != 0 && formation.regroupDueStep <= step) due.push_back(formation.id);
        std::sort(due.begin(), due.end());
        return due;
    }

    // Closes the ranks of a degraded formation: re-plans the members still following its move
    // towards the same anchor with the same facing and shape, and returns orders only for
    // members whose slot changed. Independent members keep their own orders.
    template <class IsLegal, class PositionOf>
    [[nodiscard]] MovePlan PlanRegroup(FormationId id, std::uint64_t step, PositionOf&& positionOf, IsLegal&& isLegal,
        const Layout& layout = {}) {
        MovePlan result;
        Formation* formation = Mutable(id);
        if (!formation || formation->regroupDueStep == 0) { result.rejection = FormationRejection::UnknownFormation; return result; }
        formation->regroupDueStep = 0;
        formation->lastRegroupStep = step;
        ++formation->regroups;
        formation->state = FormationState::Organized;
        if (!formation->hasDestination) return result;
        std::vector<Member> members;
        for (const MemberSlot& slot : formation->members)
            if (slot.following) members.push_back({slot.id, positionOf(slot.id)});
        if (members.empty()) return result;
        result.plan = PlanGroupMove(members, formation->anchor, formation->shape, isLegal, layout, {1.0, 0.0}, formation->facing);
        for (const Assignment& assignment : result.plan.assignments) {
            MemberSlot* slot = formation->Find(assignment.id);
            if (!slot) continue;
            const bool moved = slot->slot == kNoSlot || assignment.slot != slot->slot
                || std::hypot(assignment.destination.x - slot->destination.x, assignment.destination.y - slot->destination.y)
                    >= kRegroupMinShiftCm;
            slot->slot = assignment.slot;
            slot->destination = assignment.destination;
            if (moved) result.orders.push_back({assignment.id, assignment.destination, assignment.slot});
        }
        return result;
    }

    // Retires every formation (match Terminal, Return or teardown). Later IDs never reuse
    // retired ones within this registry.
    Change Clear() {
        Change change;
        for (const Formation& formation : formations_)
            change.dissolved.push_back({formation.id, formation.owner, DissolveReason::MatchEnded});
        formations_.clear();
        return change;
    }

private:
    std::vector<Formation> formations_;
    std::uint32_t nextId_ = 1;

    static Change& Reject(Change& change, FormationRejection rejection) {
        change.rejection = rejection;
        return change;
    }
    Formation* Mutable(FormationId id) noexcept { return const_cast<Formation*>(Find(id)); }
    Formation* MutableOwned(FormationId id, std::uint32_t owner) noexcept { return const_cast<Formation*>(FindOwned(id, owner)); }

    void RemoveMember(EntityId member, LeaveReason leave, DissolveReason dissolve, std::uint64_t step, Change& change) {
        for (auto it = formations_.begin(); it != formations_.end(); ++it) {
            const auto found = std::lower_bound(it->members.begin(), it->members.end(), member,
                [](const MemberSlot& slot, EntityId value) { return slot.id < value; });
            if (found == it->members.end() || found->id != member) continue;
            it->members.erase(found);
            change.left.push_back({it->id, member, leave});
            if (it->members.size() < kMinFormationMembers) {
                change.dissolved.push_back({it->id, it->owner, dissolve});
                formations_.erase(it);
                return;
            }
            ++it->losses;
            it->state = FormationState::Degraded;
            // A Free formation keeps its membership but does not close its ranks.
            if (it->adherence != stance::Adherence::Free && it->hasDestination && it->FollowingCount() > 0
                && it->regroupDueStep == 0) {
                const std::uint64_t earliest = std::max(step + kRegroupDelaySteps,
                    it->regroups > 0 ? it->lastRegroupStep + kRegroupDelaySteps : 0);
                it->regroupDueStep = std::max<std::uint64_t>(earliest, 1);
                change.regroups.push_back({it->id, it->regroupDueStep});
            }
            return;
        }
    }
};

} // namespace politika::formation
