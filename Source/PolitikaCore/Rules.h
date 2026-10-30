#pragma once

// Engine-independent POLITIKA command admission and faction permissions.
// The first Unreal vertical slice consumes the typed orders; this is not a
// production content schema or a complete economy/AI/fog/combat rules model.
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string_view>
#include <variant>

namespace politika::rules {

// Explicit local-match lifecycle. This records authority boundaries; it does
// not prescribe a faction economy, save format, or network replication model.
enum class MatchPhase : std::uint8_t {
    SetupPending,
    SetupValidated,
    Loading,
    Starting,
    Active,
    Terminal,
    Results,
    Returning,
    AdmissionRejected,
};

[[nodiscard]] constexpr bool CanTransition(MatchPhase from, MatchPhase to) noexcept {
    switch (from) {
        case MatchPhase::SetupPending:
            return to == MatchPhase::SetupValidated || to == MatchPhase::AdmissionRejected;
        case MatchPhase::SetupValidated:
            return to == MatchPhase::Loading || to == MatchPhase::AdmissionRejected;
        case MatchPhase::Loading:
            return to == MatchPhase::Starting || to == MatchPhase::AdmissionRejected;
        case MatchPhase::Starting:
            return to == MatchPhase::Active || to == MatchPhase::AdmissionRejected;
        case MatchPhase::Active:
            return to == MatchPhase::Terminal || to == MatchPhase::Returning;
        case MatchPhase::Terminal:
            return to == MatchPhase::Results;
        case MatchPhase::Results:
            return to == MatchPhase::Returning;
        case MatchPhase::Returning:
        case MatchPhase::AdmissionRejected:
            return false;
    }
    return false;
}

struct MatchLifecycle {
    MatchPhase phase = MatchPhase::SetupPending;

    [[nodiscard]] constexpr bool TransitionTo(MatchPhase next) noexcept {
        if (!CanTransition(phase, next)) return false;
        phase = next;
        return true;
    }

    [[nodiscard]] constexpr bool IsActive() const noexcept {
        return phase == MatchPhase::Active;
    }
};

enum class Faction : std::uint8_t {
    AtlanticConcord,
    RedDirectorate,
    IronAscendancy,
    MeridianPact,
    Unaligned,
};

[[nodiscard]] constexpr bool IsPlayable(Faction faction) noexcept {
    return faction != Faction::Unaligned;
}

struct Entity {
    std::uint64_t id = 0;
    std::uint32_t owner_player = 0; // zero = neutral/NPC world ownership
    bool selectable = true;
    bool movable = true;
    bool can_attack = true;
    bool attackable = true;
    bool alive = true;
    bool operational = true;
    // Appended fields keep existing positional initializers valid.
    std::uint8_t team = 0; // zero = no alliance; only the owner is friendly
    // Requester-relative knowledge: false when fog of war hides this entity from the
    // requesting participant's team. Callers set it; the default keeps fog-free rules.
    bool observed = true;
    // False for civilians and other non-combat units: they cannot join formations (DISC-D1).
    bool military = true;
};

// Teams and seat layouts. A networked match has 2-4 playable seats split into two
// teams, plus the Unaligned NPC side that is present in every match (so "1v1" is one
// player vs one player vs the Unaligned). The Unaligned is never a seat, never allied
// to a player team and is controlled only by the server. Seats alternate teams until
// the smaller team is full, so seat 0 (the listen-server host, or the first dedicated-
// server joiner) and seat 1 are always
// opposed and adding a seat never moves an existing participant to another team.
enum class TeamLayout : std::uint8_t { OneVsOne, TwoVsOne, ThreeVsOne, TwoVsTwo };
inline constexpr std::uint8_t kNoTeam = 0;
inline constexpr std::uint8_t kTeamA = 1;
inline constexpr std::uint8_t kTeamB = 2;
inline constexpr std::uint8_t kUnalignedTeam = 3;
inline constexpr std::uint8_t kMaxPlayableSeats = 4;
// U-12 owner decision: the absolute game-wide unit limit per participant, for the largest
// maps. Individual maps set lower caps (the multiplayer skirmish test map holds 20).
inline constexpr std::int32_t kMaxUnitsPerParticipant = 300;

[[nodiscard]] constexpr std::uint8_t SeatCount(TeamLayout layout) noexcept {
    switch (layout) {
        case TeamLayout::OneVsOne: return 2;
        case TeamLayout::TwoVsOne: return 3;
        case TeamLayout::ThreeVsOne: return 4;
        case TeamLayout::TwoVsTwo: return 4;
    }
    return 0;
}

[[nodiscard]] constexpr std::uint8_t TeamForSeat(TeamLayout layout, std::uint8_t seat) noexcept {
    constexpr std::uint8_t kTeams[4][kMaxPlayableSeats] = {
        {kTeamA, kTeamB, kNoTeam, kNoTeam},
        {kTeamA, kTeamB, kTeamA, kNoTeam},
        {kTeamA, kTeamB, kTeamA, kTeamA},
        {kTeamA, kTeamB, kTeamA, kTeamB},
    };
    const auto index = static_cast<std::uint8_t>(layout);
    if (index >= 4 || seat >= SeatCount(layout)) return kNoTeam;
    return kTeams[index][seat];
}

// Session option text ("1v1", "2v1", "3v1", "2v2"); anything else fails closed.
[[nodiscard]] constexpr bool ParseTeamLayout(std::string_view text, TeamLayout& out) noexcept {
    if (text == "1v1") { out = TeamLayout::OneVsOne; return true; }
    if (text == "2v1") { out = TeamLayout::TwoVsOne; return true; }
    if (text == "3v1") { out = TeamLayout::ThreeVsOne; return true; }
    if (text == "2v2") { out = TeamLayout::TwoVsTwo; return true; }
    return false;
}

[[nodiscard]] constexpr std::string_view LayoutName(TeamLayout layout) noexcept {
    switch (layout) {
        case TeamLayout::OneVsOne: return "1v1";
        case TeamLayout::TwoVsOne: return "2v1";
        case TeamLayout::ThreeVsOne: return "3v1";
        case TeamLayout::TwoVsTwo: return "2v2";
    }
    return "invalid";
}

// How a networked session is hosted. A listen server's local player occupies seat 0, so
// it admits SeatCount - 1 remote humans. A dedicated server has no player of its own:
// every seat, including seat 0, is bound to a remote connection in join order.
enum class HostingMode : std::uint8_t { Standalone, Listen, Dedicated };

[[nodiscard]] constexpr std::uint8_t LocalSeatCount(HostingMode mode) noexcept {
    return mode == HostingMode::Dedicated ? 0 : 1;
}

[[nodiscard]] constexpr std::uint8_t RemoteSeatCount(TeamLayout layout, HostingMode mode) noexcept {
    if (mode == HostingMode::Standalone) return 0;
    const std::uint8_t seats = SeatCount(layout);
    return seats >= LocalSeatCount(mode) ? static_cast<std::uint8_t>(seats - LocalSeatCount(mode)) : 0;
}

// The match becomes active only when every seat of the layout is bound, exactly once.
[[nodiscard]] constexpr bool SessionSeatsComplete(TeamLayout layout, HostingMode mode,
    std::uint32_t local_bindings, std::uint32_t remote_bindings) noexcept {
    return mode != HostingMode::Standalone && SeatCount(layout) != 0
        && local_bindings == LocalSeatCount(mode) && remote_bindings == RemoteSeatCount(layout, mode);
}

[[nodiscard]] constexpr std::string_view HostingModeName(HostingMode mode) noexcept {
    switch (mode) {
        case HostingMode::Standalone: return "standalone";
        case HostingMode::Listen: return "listen";
        case HostingMode::Dedicated: return "dedicated";
    }
    return "invalid";
}

// Allied entities never target each other. Different owners without a shared team
// (including the owner-0 Unaligned authority) are hostile.
[[nodiscard]] constexpr bool AreAllied(std::uint32_t owner_a, std::uint8_t team_a,
    std::uint32_t owner_b, std::uint8_t team_b) noexcept {
    return owner_a == owner_b || (team_a != kNoTeam && team_a == team_b);
}

// The Unaligned is normally neutral: explicit orders may still target its units, but
// neither it nor a player side acquires the other automatically (attack-move, guard,
// idle response). Its exceptional hostilities need faction identities and its AI.
[[nodiscard]] constexpr bool CanAutoEngage(std::uint32_t owner_a, std::uint8_t team_a,
    std::uint32_t owner_b, std::uint8_t team_b) noexcept {
    return !AreAllied(owner_a, team_a, owner_b, team_b)
        && team_a != kUnalignedTeam && team_b != kUnalignedTeam;
}

// Stable, presentation-independent player/AI intent. Coordinates are prototype
// map-space values; Unreal adapters translate world hits into this domain type.
struct Position {
    double x = 0.0;
    double y = 0.0;
};

// Fog of war: a team observes a position when any of its living vision sources is
// within that source's sight radius (inclusive, horizontal distance). A team always
// observes its own entities; callers apply that rule before asking about others.
struct VisionSource {
    std::uint8_t team = kNoTeam;
    Position position;
    double sight_radius = 0.0;
};

[[nodiscard]] inline bool IsObservedBy(std::uint8_t team, Position target,
    std::span<const VisionSource> sources) noexcept {
    if (team == kNoTeam || !std::isfinite(target.x) || !std::isfinite(target.y)) return false;
    for (const VisionSource& source : sources) {
        if (source.team != team || !(source.sight_radius > 0.0)) continue;
        const double dx = source.position.x - target.x;
        const double dy = source.position.y - target.y;
        if (dx * dx + dy * dy <= source.sight_radius * source.sight_radius) return true;
    }
    return false;
}

// What one viewer may learn from a shot. A shot cue carries a target point, so it must
// follow the same team-vision rule as replication: a viewer that cannot see the shooter
// gets nothing, and one that sees the shooter but not its target gets only the shooter's
// facing (already public through its replicated rotation), never the target position.
enum class ShotCue : std::uint8_t { None, FacingOnly, Exact };

[[nodiscard]] constexpr ShotCue ShotCueFor(bool fog_of_war, bool viewer_observes_shooter,
    bool viewer_observes_target) noexcept {
    if (!fog_of_war) return ShotCue::Exact;
    if (!viewer_observes_shooter) return ShotCue::None;
    return viewer_observes_target ? ShotCue::Exact : ShotCue::FacingOnly;
}

[[nodiscard]] constexpr std::string_view ShotCueName(ShotCue cue) noexcept {
    switch (cue) {
        case ShotCue::None: return "none";
        case ShotCue::FacingOnly: return "facing";
        case ShotCue::Exact: return "exact";
    }
    return "invalid";
}

struct MoveOrder { Position destination; };
struct AttackOrder { std::uint64_t target_id = 0; };
struct AttackMoveOrder { Position destination; };
struct StopOrder {};
struct GuardOrder {};
struct GrenadeOrder { std::uint64_t target_id = 0; };
struct ReloadOrder {};
struct SprintOrder {};
using Order = std::variant<MoveOrder, AttackOrder, AttackMoveOrder, StopOrder, GuardOrder, GrenadeOrder, ReloadOrder, SprintOrder>;

// Runtime identities are match-local. Definition IDs, participant IDs and
// correlation IDs occupy separate fields and must not be substituted for one another.
struct CommandRequest {
    std::uint64_t match_generation = 0;
    std::uint64_t sequence = 0;
    std::uint32_t participant_id = 0;
    std::uint64_t issuer_entity_id = 0;
    std::uint64_t correlation_id = 0;
    Order intent = StopOrder{};
};

enum class OrderKind : std::uint8_t { Move, Attack, AttackMove, Stop, Guard, Grenade, Reload, Sprint };
enum class ExecutionOrder : std::uint8_t { Idle, Move, Attack, AttackMove, Stop, Guard };

[[nodiscard]] constexpr OrderKind KindOf(const Order& order) noexcept {
    return static_cast<OrderKind>(order.index());
}

[[nodiscard]] constexpr bool CanSelect(const Entity& entity, std::uint32_t player) noexcept {
    return player != 0 && entity.id != 0 && entity.owner_player == player && entity.selectable;
}

enum class MoveRejection : std::uint8_t {
    None,
    NotOwned,
    CannotMove,
    InvalidBounds,
    InvalidTarget,
};

enum class OrderRejection : std::uint8_t {
    None,
    NotOwned,
    CannotMove,
    CannotAttack,
    InvalidBounds,
    InvalidTarget,
    MissingTarget,
    TargetNotAttackable,
    FriendlyTarget,
    MissingIssuer,
    IssuerUnavailable,
    Unauthorized,
    MatchInactive,
    StaleMatch,
    AbilityUnavailable,
    // Network intake rejections. Appended so existing numeric codes stay stable.
    MalformedCommand,
    DuplicateCommand,
    RateLimited,
    // Fog of war: the target is not currently observed by the requester's team. Also
    // returned for unknown IDs while fog is enforced, so it is not an existence oracle.
    TargetNotVisible,
};

inline constexpr std::size_t kOrderRejectionCount = static_cast<std::size_t>(OrderRejection::TargetNotVisible) + 1;

struct OrderResult {
    OrderRejection rejection = OrderRejection::None;
    [[nodiscard]] constexpr explicit operator bool() const noexcept {
        return rejection == OrderRejection::None;
    }
};

enum class OrderEndReason : std::uint8_t { None, Replaced, Stopped, TargetLost, UnitDestroyed, Arrived, Obstructed, NavigationFailed };
struct CommandOutcomeEvent {
    std::uint64_t step = 0, sequence = 0, correlation_id = 0, issuer_id = 0;
    OrderKind order = OrderKind::Stop;
    bool accepted = false;
    OrderRejection rejection = OrderRejection::None;
};
struct OrderChangedEvent {
    std::uint64_t step = 0, entity_id = 0;
    ExecutionOrder previous = ExecutionOrder::Idle, current = ExecutionOrder::Idle;
    OrderEndReason previous_end = OrderEndReason::Replaced;
};
struct UnitDamagedEvent { std::uint64_t step = 0, entity_id = 0; std::int32_t applied = 0; };
struct UnitDestroyedEvent { std::uint64_t step = 0, entity_id = 0; };
struct TargetLostEvent { std::uint64_t step = 0, entity_id = 0, target_id = 0; };
struct ObjectiveChangedEvent {
    std::uint64_t step = 0;
    bool contested = false, complete = false;
    std::uint16_t progress_milli = 0;
};
using GameplayEvent = std::variant<CommandOutcomeEvent, OrderChangedEvent, UnitDamagedEvent,
    UnitDestroyedEvent, TargetLostEvent, ObjectiveChangedEvent>;

struct MapBounds {
    double width = 0.0;
    double height = 0.0;
};

[[nodiscard]] inline bool IsValidPosition(MapBounds bounds, Position position) noexcept {
    return std::isfinite(bounds.width) && std::isfinite(bounds.height)
        && bounds.width > 0 && bounds.height > 0
        && std::isfinite(position.x) && std::isfinite(position.y)
        && position.x >= 0 && position.y >= 0
        && position.x <= bounds.width && position.y <= bounds.height;
}

// `target` is supplied by the authoritative entity lookup. A null target is
// rejected; callers must not trust a client-provided owner or capability bit.
[[nodiscard]] inline OrderResult AdmitOrder(
    const Entity& issuer,
    std::uint32_t requesting_player,
    MapBounds bounds,
    const Order& order,
    const Entity* target = nullptr
) noexcept {
    if (requesting_player == 0 || issuer.id == 0 || issuer.owner_player != requesting_player) {
        return {OrderRejection::NotOwned};
    }
    if (const auto* move = std::get_if<MoveOrder>(&order)) {
        if (!issuer.movable) return {OrderRejection::CannotMove};
        if (!std::isfinite(bounds.width) || !std::isfinite(bounds.height)
            || bounds.width <= 0 || bounds.height <= 0) return {OrderRejection::InvalidBounds};
        if (!IsValidPosition(bounds, move->destination)) return {OrderRejection::InvalidTarget};
    } else if (const auto* attack_move = std::get_if<AttackMoveOrder>(&order)) {
        if (!issuer.movable) return {OrderRejection::CannotMove};
        if (!std::isfinite(bounds.width) || !std::isfinite(bounds.height)
            || bounds.width <= 0 || bounds.height <= 0) return {OrderRejection::InvalidBounds};
        if (!IsValidPosition(bounds, attack_move->destination)) return {OrderRejection::InvalidTarget};
    } else if (const auto* attack = std::get_if<AttackOrder>(&order)) {
        if (!issuer.can_attack) return {OrderRejection::CannotAttack};
        if (attack->target_id == 0 || target == nullptr || target->id != attack->target_id)
            return {OrderRejection::MissingTarget};
        if (!target->observed) return {OrderRejection::TargetNotVisible};
        if (!target->alive) return {OrderRejection::MissingTarget};
        if (!target->attackable) return {OrderRejection::TargetNotAttackable};
        if (target->owner_player == requesting_player
            || AreAllied(issuer.owner_player, issuer.team, target->owner_player, target->team))
            return {OrderRejection::FriendlyTarget};
    } else if (const auto* grenade = std::get_if<GrenadeOrder>(&order)) {
        return AdmitOrder(issuer, requesting_player, bounds, AttackOrder{grenade->target_id}, target);
    } else if (std::holds_alternative<ReloadOrder>(order)) {
        if (!issuer.can_attack) return {OrderRejection::CannotAttack};
    } else if (std::holds_alternative<SprintOrder>(order)) {
        if (!issuer.movable) return {OrderRejection::CannotMove};
    }
    return {};
}

[[nodiscard]] inline OrderResult AdmitCommand(
    const CommandRequest& request,
    std::uint64_t active_match_generation,
    bool match_accepting,
    const Entity* issuer,
    MapBounds bounds,
    const Entity* target = nullptr
) noexcept {
    if (!match_accepting) return {OrderRejection::MatchInactive};
    if (request.match_generation != active_match_generation) return {OrderRejection::StaleMatch};
    if (!issuer || issuer->id == 0 || issuer->id != request.issuer_entity_id)
        return {OrderRejection::MissingIssuer};
    if (!issuer->alive || !issuer->operational) return {OrderRejection::IssuerUnavailable};
    if (request.participant_id == 0 || issuer->owner_player != request.participant_id)
        return {OrderRejection::Unauthorized};
    return AdmitOrder(*issuer, request.participant_id, bounds, request.intent, target);
}

// Bounded remote intent shape. A transport adapter supplies only the order
// variant index and its parameters; participant, generation and ownership are
// resolved by the authoritative server, never taken from this payload.
struct IntentPayload {
    std::uint8_t order_kind = 0;
    std::uint64_t target_id = 0;
    double x = 0.0;
    double y = 0.0;
};

inline constexpr std::uint8_t kOrderKindCount =
    static_cast<std::uint8_t>(std::variant_size_v<Order>);

// Converts a remote payload into the existing typed order. Fields that do not
// belong to the selected variant must be zero, so a forged payload cannot carry
// ambiguous parameters. Range checks against map bounds remain in AdmitOrder.
[[nodiscard]] inline OrderResult DecodeIntent(const IntentPayload& payload, Order& out) noexcept {
    if (payload.order_kind >= kOrderKindCount) return {OrderRejection::MalformedCommand};
    if (!std::isfinite(payload.x) || !std::isfinite(payload.y)) return {OrderRejection::MalformedCommand};
    const bool has_target = payload.target_id != 0;
    const bool has_position = payload.x != 0.0 || payload.y != 0.0;
    switch (static_cast<OrderKind>(payload.order_kind)) {
        case OrderKind::Move:
            if (has_target) return {OrderRejection::MalformedCommand};
            out = MoveOrder{{payload.x, payload.y}};
            return {};
        case OrderKind::AttackMove:
            if (has_target) return {OrderRejection::MalformedCommand};
            out = AttackMoveOrder{{payload.x, payload.y}};
            return {};
        case OrderKind::Attack:
            if (has_position) return {OrderRejection::MalformedCommand};
            out = AttackOrder{payload.target_id};
            return {};
        case OrderKind::Grenade:
            if (has_position) return {OrderRejection::MalformedCommand};
            out = GrenadeOrder{payload.target_id};
            return {};
        case OrderKind::Stop:
            if (has_target || has_position) return {OrderRejection::MalformedCommand};
            out = StopOrder{};
            return {};
        case OrderKind::Guard:
            if (has_target || has_position) return {OrderRejection::MalformedCommand};
            out = GuardOrder{};
            return {};
        case OrderKind::Reload:
            if (has_target || has_position) return {OrderRejection::MalformedCommand};
            out = ReloadOrder{};
            return {};
        case OrderKind::Sprint:
            if (has_target || has_position) return {OrderRejection::MalformedCommand};
            out = SprintOrder{};
            return {};
    }
    return {OrderRejection::MalformedCommand};
}

// Per-connection freshness. Every client sequence is consumed once, accepted or
// rejected, so duplicated, replayed or reordered submissions cannot repeat an effect.
struct ClientSequenceGuard {
    std::uint32_t last_seen = 0;

    [[nodiscard]] constexpr OrderResult Consume(std::uint32_t sequence) noexcept {
        if (sequence == 0) return {OrderRejection::MalformedCommand};
        if (sequence <= last_seen) return {OrderRejection::DuplicateCommand};
        last_seen = sequence;
        return {};
    }
};

// One authoritative update has a deliberately visible order; the UE
// coordinator executes this sequence and publishes the resulting typed events last.
enum class SimulationPhase : std::uint8_t {
    AdmitOrders,
    UpdateMovement,
    ResolveCombat,
    ApplyDamage,
    CleanupDeadEntities,
    UpdateObjectives,
    PublishEvents,
};

inline constexpr SimulationPhase kSimulationPhases[] = {
    SimulationPhase::AdmitOrders,
    SimulationPhase::UpdateMovement,
    SimulationPhase::ResolveCombat,
    SimulationPhase::ApplyDamage,
    SimulationPhase::CleanupDeadEntities,
    SimulationPhase::UpdateObjectives,
    SimulationPhase::PublishEvents,
};

[[nodiscard]] inline MoveRejection AdmitMove(
    const Entity& entity,
    std::uint32_t requesting_player,
    MapBounds bounds,
    double x,
    double y
) noexcept {
    if (requesting_player == 0 || entity.id == 0 || entity.owner_player != requesting_player) {
        return MoveRejection::NotOwned;
    }
    if (!entity.movable) {
        return MoveRejection::CannotMove;
    }
    if (!std::isfinite(bounds.width) || !std::isfinite(bounds.height)
        || bounds.width <= 0 || bounds.height <= 0) {
        return MoveRejection::InvalidBounds;
    }
    if (!std::isfinite(x) || !std::isfinite(y)
        || x < 0 || y < 0 || x > bounds.width || y > bounds.height) {
        return MoveRejection::InvalidTarget;
    }
    return MoveRejection::None;
}

// Provisional resource names are intentionally abstract: production recipes,
// conversion rates and faction-specific costs remain design/implementation work.
struct ResourcePool {
    std::int64_t crude_oil = 0;
    std::int64_t natural_gas = 0;
    std::int64_t processed_nuclear = 0;
};

[[nodiscard]] constexpr bool TrySpend(ResourcePool& balance, const ResourcePool& cost) noexcept {
    if (cost.crude_oil < 0 || cost.natural_gas < 0 || cost.processed_nuclear < 0
        || balance.crude_oil < 0 || balance.natural_gas < 0 || balance.processed_nuclear < 0
        || balance.crude_oil < cost.crude_oil || balance.natural_gas < cost.natural_gas
        || balance.processed_nuclear < cost.processed_nuclear) {
        return false;
    }
    balance.crude_oil -= cost.crude_oil;
    balance.natural_gas -= cost.natural_gas;
    balance.processed_nuclear -= cost.processed_nuclear;
    return true;
}

struct NuclearIntelligence {
    bool refining_intelligence = false;
    bool weapons_intelligence = false;
};

[[nodiscard]] constexpr bool CanRefineNuclear(
    Faction faction,
    NuclearIntelligence intelligence = {}
) noexcept {
    switch (faction) {
        case Faction::AtlanticConcord:
        case Faction::RedDirectorate:
            return true;
        case Faction::IronAscendancy:
            return intelligence.refining_intelligence && intelligence.weapons_intelligence;
        case Faction::MeridianPact:
        case Faction::Unaligned:
            return false;
    }
    return false;
}

enum class SabotageOperator : std::uint8_t {
    Spy,
    Conventional,
};

[[nodiscard]] constexpr bool CanPlantExplosives(
    Faction faction,
    SabotageOperator unit,
    bool late_espionage_unlocked
) noexcept {
    switch (faction) {
        case Faction::MeridianPact:
            return true; // early low-cost conventional sabotage; exact price defined later
        case Faction::AtlanticConcord:
        case Faction::RedDirectorate:
        case Faction::IronAscendancy:
            return late_espionage_unlocked && unit == SabotageOperator::Spy;
        case Faction::Unaligned:
            return false;
    }
    return false;
}

} // namespace politika::rules
