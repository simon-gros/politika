#include "Rules.h"
#include "Metrics.h"
#include "Fog.h"
#include "UnitScale.h"
#include "GroupSelection.h"
#include "FormationGeometry.h"
#include "Formation.h"
#include "Stance.h"
#include "Discipline.h"
#include "Economy.h"

#include <cassert>
#include <cmath>
#include <iterator>
#include <limits>
#include <vector>

using namespace politika::rules;

int main() {
    // The production lifecycle is the single portable authority for match phase.
    // Cover every legal edge, then verify illegal and duplicate requests are inert.
    const auto Reach = [](MatchPhase Target) {
        MatchLifecycle State;
        if (Target == MatchPhase::AdmissionRejected) {
            assert(State.TransitionTo(MatchPhase::AdmissionRejected));
            return State;
        }
        constexpr MatchPhase Path[] = {MatchPhase::SetupValidated, MatchPhase::Loading,
            MatchPhase::Starting, MatchPhase::Active, MatchPhase::Terminal,
            MatchPhase::Results, MatchPhase::Returning};
        if (Target == MatchPhase::SetupPending) return State;
        for (const MatchPhase Phase : Path) {
            assert(State.TransitionTo(Phase));
            if (State.phase == Target) break;
        }
        return State;
    };
    MatchLifecycle Lifecycle;
    assert(Lifecycle.phase == MatchPhase::SetupPending && !Lifecycle.IsActive());
    constexpr MatchPhase LegalEdges[][2] = {
        {MatchPhase::SetupPending, MatchPhase::SetupValidated},
        {MatchPhase::SetupPending, MatchPhase::AdmissionRejected},
        {MatchPhase::SetupValidated, MatchPhase::Loading},
        {MatchPhase::SetupValidated, MatchPhase::AdmissionRejected},
        {MatchPhase::Loading, MatchPhase::Starting},
        {MatchPhase::Loading, MatchPhase::AdmissionRejected},
        {MatchPhase::Starting, MatchPhase::Active},
        {MatchPhase::Starting, MatchPhase::AdmissionRejected},
        {MatchPhase::Active, MatchPhase::Terminal},
        {MatchPhase::Active, MatchPhase::Returning},
        {MatchPhase::Terminal, MatchPhase::Results},
        {MatchPhase::Results, MatchPhase::Returning},
    };
    for (const auto& Edge : LegalEdges) {
        MatchLifecycle State = Reach(Edge[0]);
        assert(State.phase == Edge[0]);
        assert(State.TransitionTo(Edge[1]));
        assert(State.phase == Edge[1]);
        assert(State.IsActive() == (Edge[1] == MatchPhase::Active));
    }
    constexpr MatchPhase Phases[] = {MatchPhase::SetupPending, MatchPhase::SetupValidated,
        MatchPhase::Loading, MatchPhase::Starting, MatchPhase::Active, MatchPhase::Terminal,
        MatchPhase::Results, MatchPhase::Returning, MatchPhase::AdmissionRejected};
    for (const MatchPhase From : Phases) for (const MatchPhase To : Phases) {
        if (CanTransition(From, To)) continue;
        MatchLifecycle State = Reach(From);
        assert(State.phase == From);
        assert(!State.TransitionTo(To));
        assert(State.phase == From);
    }
    for (const MatchPhase Phase : {MatchPhase::Active, MatchPhase::Terminal, MatchPhase::Results,
            MatchPhase::AdmissionRejected, MatchPhase::Returning}) {
        MatchLifecycle State = Reach(Phase);
        assert(!State.TransitionTo(Phase));
        assert(State.phase == Phase);
        assert(State.IsActive() == (Phase == MatchPhase::Active));
        if (Phase != MatchPhase::Active) assert(!State.TransitionTo(MatchPhase::Active));
    }
    MatchLifecycle Rejected;
    assert(Rejected.TransitionTo(MatchPhase::AdmissionRejected));
    assert(!Rejected.TransitionTo(MatchPhase::AdmissionRejected));
    assert(!Rejected.TransitionTo(MatchPhase::Active));
    MatchLifecycle Finished = Reach(MatchPhase::Terminal);
    assert(!Finished.TransitionTo(MatchPhase::Terminal));
    assert(Finished.TransitionTo(MatchPhase::Results));
    assert(!Finished.TransitionTo(MatchPhase::Results));
    assert(!Finished.TransitionTo(MatchPhase::Active));

    // Unit selection is player-owned; neutral/NPC and enemy units are not selectable.
    constexpr Entity own{1, 2, true, true};
    constexpr Entity enemy{2, 3, true, true};
    constexpr Entity neutral{3, 0, true, true};
    static_assert(CanSelect(own, 2));
    static_assert(!CanSelect(own, 1));
    static_assert(!CanSelect(enemy, 2));
    static_assert(!CanSelect(neutral, 2));
    static_assert(!CanSelect(own, 0));
    static_assert(!IsPlayable(Faction::Unaligned));
    static_assert(IsPlayable(Faction::MeridianPact));

    // Orders are validated before navigation; this is not pathfinding.
    assert(AdmitMove(own, 2, {100, 100}, 50, 80) == MoveRejection::None);
    assert(AdmitMove(own, 3, {100, 100}, 50, 80) == MoveRejection::NotOwned);
    assert(AdmitMove(neutral, 2, {100, 100}, 50, 80) == MoveRejection::NotOwned);
    assert(AdmitMove(Entity{4, 2, true, false}, 2, {100, 100}, 10, 10)
        == MoveRejection::CannotMove);
    assert(AdmitMove(own, 2, {0, 100}, 10, 10) == MoveRejection::InvalidBounds);
    assert(AdmitMove(own, 2, {100, 100}, -1, 10) == MoveRejection::InvalidTarget);
    assert(AdmitMove(own, 2, {100, 100}, 101, 10) == MoveRejection::InvalidTarget);
    assert(AdmitMove(own, 2, {100, 100}, std::numeric_limits<double>::quiet_NaN(), 0)
        == MoveRejection::InvalidTarget);

    // The shared typed-order boundary rejects invalid input before runtime state mutates.
    const Order move_order = MoveOrder{{40, 70}};
    const Order attack_move_order = AttackMoveOrder{{100, 100}};
    const Entity hostile{9, 3, true, true};
    const Entity unattackable{10, 3, true, true, true, false};
    assert(KindOf(move_order) == OrderKind::Move);
    assert(KindOf(attack_move_order) == OrderKind::AttackMove);
    assert(AdmitOrder(own, 2, {100, 100}, move_order));
    assert(AdmitOrder(own, 3, {100, 100}, move_order).rejection == OrderRejection::NotOwned);
    assert(AdmitOrder(own, 2, {100, 100}, Order{MoveOrder{{101, 0}}}).rejection == OrderRejection::InvalidTarget);
    assert(AdmitOrder(own, 2, {100, 100}, attack_move_order));
    assert(AdmitOrder(own, 2, {100, 100}, Order{AttackOrder{9}}, &hostile));
    assert(AdmitOrder(own, 2, {100, 100}, Order{AttackOrder{9}}).rejection == OrderRejection::MissingTarget);
    assert(AdmitOrder(own, 2, {100, 100}, Order{AttackOrder{1}}, &own).rejection == OrderRejection::FriendlyTarget);
    assert(AdmitOrder(own, 2, {100, 100}, Order{AttackOrder{10}}, &unattackable).rejection == OrderRejection::TargetNotAttackable);
    assert(AdmitOrder(own, 2, {100, 100}, Order{StopOrder{}}));
    assert(AdmitOrder(own, 2, {100, 100}, Order{GuardOrder{}}));
    assert(KindOf(Order{GrenadeOrder{9}}) == OrderKind::Grenade);
    assert(AdmitOrder(own, 2, {100, 100}, Order{GrenadeOrder{9}}, &hostile));
    assert(AdmitOrder(own, 2, {100, 100}, Order{GrenadeOrder{1}}, &own).rejection == OrderRejection::FriendlyTarget);
    assert(AdmitOrder(own, 2, {100, 100}, Order{GrenadeOrder{9}}).rejection == OrderRejection::MissingTarget);
    assert(AdmitOrder(own, 3, {100, 100}, Order{ReloadOrder{}}).rejection == OrderRejection::NotOwned);
    assert(AdmitOrder(own, 2, {100, 100}, Order{ReloadOrder{}}));
    assert(AdmitOrder(own, 2, {100, 100}, Order{SprintOrder{}}));
    // Shared request metadata separates participant, match, runtime entity and
    // correlation identities; failed admission is a result, not a mutation.
    const CommandRequest request{7, 3, 2, 1, 99, MoveOrder{{40, 70}}};
    const Entity operationalOwn{1, 2, true, true, true, true, true, true};
    assert(AdmitCommand(request, 7, true, &operationalOwn, {100, 100}));
    assert(AdmitCommand(request, 7, false, &operationalOwn, {100, 100}).rejection == OrderRejection::MatchInactive);
    assert(AdmitCommand(request, 8, true, &operationalOwn, {100, 100}).rejection == OrderRejection::StaleMatch);
    assert(AdmitCommand(request, 7, true, nullptr, {100, 100}).rejection == OrderRejection::MissingIssuer);
    const Entity wrongIssuer{2, 2};
    assert(AdmitCommand(request, 7, true, &wrongIssuer, {100, 100}).rejection == OrderRejection::MissingIssuer);
    const Entity unauthorized{1, 4};
    assert(AdmitCommand(request, 7, true, &unauthorized, {100, 100}).rejection == OrderRejection::Unauthorized);
    const Entity deadIssuer{1, 2, true, true, true, true, false, false};
    assert(AdmitCommand(request, 7, true, &deadIssuer, {100, 100}).rejection == OrderRejection::IssuerUnavailable);
    const Entity immobile{1, 2, true, false};
    assert(AdmitCommand(request, 7, true, &immobile, {100, 100}).rejection == OrderRejection::CannotMove);
    const CommandRequest attackRequest{7, 4, 2, 1, 100, AttackOrder{9}};
    const Entity nonAttacker{1, 2, true, true, false};
    assert(AdmitCommand(attackRequest, 7, true, &nonAttacker, {100, 100}, &hostile).rejection == OrderRejection::CannotAttack);
    const Entity deadTarget{9, 3, true, true, true, true, false, false};
    assert(AdmitCommand(attackRequest, 7, true, &operationalOwn, {100, 100}, &deadTarget).rejection == OrderRejection::MissingTarget);
    assert(request.sequence < attackRequest.sequence && request.correlation_id != request.issuer_entity_id);
    const GameplayEvent acceptedEvent = CommandOutcomeEvent{5, request.sequence, request.correlation_id,
        request.issuer_entity_id, KindOf(request.intent), true, OrderRejection::None};
    const auto* acceptedOutcome = std::get_if<CommandOutcomeEvent>(&acceptedEvent);
    assert(acceptedOutcome && acceptedOutcome->step == 5 && acceptedOutcome->accepted
        && acceptedOutcome->order == OrderKind::Move && acceptedOutcome->rejection == OrderRejection::None);
    const GameplayEvent arrivalEvent = OrderChangedEvent{6, 1, ExecutionOrder::Move,
        ExecutionOrder::Idle, OrderEndReason::Arrived};
    const auto* transition = std::get_if<OrderChangedEvent>(&arrivalEvent);
    assert(transition && transition->previous == ExecutionOrder::Move
        && transition->current == ExecutionOrder::Idle && transition->previous_end == OrderEndReason::Arrived);
    constexpr SimulationPhase ExpectedPhases[] = {
        SimulationPhase::AdmitOrders, SimulationPhase::UpdateMovement,
        SimulationPhase::ResolveCombat, SimulationPhase::ApplyDamage,
        SimulationPhase::CleanupDeadEntities, SimulationPhase::UpdateObjectives,
        SimulationPhase::PublishEvents
    };
    for (std::size_t Index = 0; Index < std::size(kSimulationPhases); ++Index)
        assert(kSimulationPhases[Index] == ExpectedPhases[Index]);

    // Remote intent payloads decode to exactly one existing typed order. They
    // carry no participant/owner field; malformed shapes fail before admission.
    static_assert(kOrderKindCount == 8);
    static_assert(static_cast<std::uint8_t>(OrderRejection::AbilityUnavailable) == 14);
    static_assert(static_cast<std::uint8_t>(OrderRejection::MalformedCommand) == 15);
    static_assert(static_cast<std::uint8_t>(OrderRejection::DuplicateCommand) == 16);
    static_assert(static_cast<std::uint8_t>(OrderRejection::RateLimited) == 17);
    Order decoded = StopOrder{};
    assert(DecodeIntent({static_cast<std::uint8_t>(OrderKind::Move), 0, 40, 70}, decoded));
    assert(std::holds_alternative<MoveOrder>(decoded) && std::get<MoveOrder>(decoded).destination.x == 40);
    assert(DecodeIntent({static_cast<std::uint8_t>(OrderKind::AttackMove), 0, 5, 6}, decoded)
        && std::holds_alternative<AttackMoveOrder>(decoded));
    assert(DecodeIntent({static_cast<std::uint8_t>(OrderKind::Attack), 9, 0, 0}, decoded)
        && std::get<AttackOrder>(decoded).target_id == 9);
    assert(DecodeIntent({static_cast<std::uint8_t>(OrderKind::Grenade), 9, 0, 0}, decoded)
        && std::get<GrenadeOrder>(decoded).target_id == 9);
    assert(DecodeIntent({static_cast<std::uint8_t>(OrderKind::Stop), 0, 0, 0}, decoded)
        && std::holds_alternative<StopOrder>(decoded));
    assert(DecodeIntent({static_cast<std::uint8_t>(OrderKind::Guard), 0, 0, 0}, decoded)
        && std::holds_alternative<GuardOrder>(decoded));
    assert(DecodeIntent({static_cast<std::uint8_t>(OrderKind::Reload), 0, 0, 0}, decoded)
        && std::holds_alternative<ReloadOrder>(decoded));
    assert(DecodeIntent({static_cast<std::uint8_t>(OrderKind::Sprint), 0, 0, 0}, decoded)
        && std::holds_alternative<SprintOrder>(decoded));
    const Order beforeRejectedDecode = decoded;
    assert(DecodeIntent({kOrderKindCount, 0, 0, 0}, decoded).rejection == OrderRejection::MalformedCommand);
    assert(DecodeIntent({250, 0, 0, 0}, decoded).rejection == OrderRejection::MalformedCommand);
    assert(DecodeIntent({static_cast<std::uint8_t>(OrderKind::Move), 0, std::nan(""), 1}, decoded).rejection
        == OrderRejection::MalformedCommand);
    assert(DecodeIntent({static_cast<std::uint8_t>(OrderKind::Move), 0, 1, INFINITY}, decoded).rejection
        == OrderRejection::MalformedCommand);
    assert(DecodeIntent({static_cast<std::uint8_t>(OrderKind::Move), 3, 1, 1}, decoded).rejection
        == OrderRejection::MalformedCommand);
    assert(DecodeIntent({static_cast<std::uint8_t>(OrderKind::Attack), 3, 1, 0}, decoded).rejection
        == OrderRejection::MalformedCommand);
    assert(DecodeIntent({static_cast<std::uint8_t>(OrderKind::Stop), 3, 0, 0}, decoded).rejection
        == OrderRejection::MalformedCommand);
    assert(DecodeIntent({static_cast<std::uint8_t>(OrderKind::Guard), 0, 0, 2}, decoded).rejection
        == OrderRejection::MalformedCommand);
    assert(decoded.index() == beforeRejectedDecode.index());
    // A decoded, finite but out-of-map destination is still rejected by the shared bounds rule.
    assert(DecodeIntent({static_cast<std::uint8_t>(OrderKind::Move), 0, 1.0e6, 1.0e6}, decoded));
    assert(AdmitOrder(own, 2, {100, 100}, decoded).rejection == OrderRejection::InvalidTarget);

    // Client sequences are single-use and monotonic per bound connection.
    ClientSequenceGuard guard;
    assert(guard.Consume(0).rejection == OrderRejection::MalformedCommand && guard.last_seen == 0);
    assert(guard.Consume(1) && guard.last_seen == 1);
    assert(guard.Consume(1).rejection == OrderRejection::DuplicateCommand);
    assert(guard.Consume(5) && guard.last_seen == 5);
    assert(guard.Consume(4).rejection == OrderRejection::DuplicateCommand && guard.last_seen == 5);
    assert(guard.Consume(6));

    // Team layouts: 2-4 seats, two teams, host seat 0 and seat 1 always opposed.
    static_assert(static_cast<std::uint8_t>(OrderRejection::TargetNotVisible) == 18);
    static_assert(SeatCount(TeamLayout::OneVsOne) == 2 && SeatCount(TeamLayout::TwoVsOne) == 3);
    static_assert(SeatCount(TeamLayout::ThreeVsOne) == 4 && SeatCount(TeamLayout::TwoVsTwo) == 4);
    constexpr TeamLayout AllLayouts[] = {TeamLayout::OneVsOne, TeamLayout::TwoVsOne,
        TeamLayout::ThreeVsOne, TeamLayout::TwoVsTwo};
    constexpr int ExpectedTeamA[] = {1, 2, 3, 2};
    constexpr int ExpectedTeamB[] = {1, 1, 1, 2};
    for (std::size_t Index = 0; Index < std::size(AllLayouts); ++Index) {
        const TeamLayout Layout = AllLayouts[Index];
        assert(SeatCount(Layout) <= kMaxPlayableSeats);
        assert(TeamForSeat(Layout, 0) == kTeamA && TeamForSeat(Layout, 1) == kTeamB);
        int TeamA = 0, TeamB = 0;
        for (std::uint8_t Seat = 0; Seat < kMaxPlayableSeats + 2; ++Seat) {
            const std::uint8_t Team = TeamForSeat(Layout, Seat);
            if (Seat >= SeatCount(Layout)) { assert(Team == kNoTeam); continue; }
            assert(Team == kTeamA || Team == kTeamB);
            (Team == kTeamA ? TeamA : TeamB) += 1;
        }
        assert(TeamA == ExpectedTeamA[Index] && TeamB == ExpectedTeamB[Index]);
        TeamLayout Parsed = TeamLayout::TwoVsTwo;
        assert(ParseTeamLayout(LayoutName(Layout), Parsed) && Parsed == Layout);
    }
    assert(TeamForSeat(static_cast<TeamLayout>(9), 0) == kNoTeam);

    // Hosting: a listen host fills seat 0 itself; a dedicated server binds every seat remotely.
    static_assert(LocalSeatCount(HostingMode::Listen) == 1 && LocalSeatCount(HostingMode::Dedicated) == 0);
    static_assert(RemoteSeatCount(TeamLayout::OneVsOne, HostingMode::Standalone) == 0);
    constexpr int ExpectedListenRemote[] = {1, 2, 3, 3};
    constexpr int ExpectedDedicatedRemote[] = {2, 3, 4, 4};
    for (std::size_t Index = 0; Index < std::size(AllLayouts); ++Index) {
        const TeamLayout Layout = AllLayouts[Index];
        assert(RemoteSeatCount(Layout, HostingMode::Listen) == ExpectedListenRemote[Index]);
        assert(RemoteSeatCount(Layout, HostingMode::Dedicated) == ExpectedDedicatedRemote[Index]);
        // No mode exceeds four humans, and both modes play the same number of seats.
        assert(RemoteSeatCount(Layout, HostingMode::Dedicated) <= kMaxPlayableSeats);
        assert(LocalSeatCount(HostingMode::Listen) + RemoteSeatCount(Layout, HostingMode::Listen)
            == LocalSeatCount(HostingMode::Dedicated) + RemoteSeatCount(Layout, HostingMode::Dedicated));
        const auto Remote = static_cast<std::uint32_t>(ExpectedDedicatedRemote[Index]);
        assert(SessionSeatsComplete(Layout, HostingMode::Dedicated, 0, Remote));
        assert(!SessionSeatsComplete(Layout, HostingMode::Dedicated, 0, Remote - 1));
        assert(!SessionSeatsComplete(Layout, HostingMode::Dedicated, 1, Remote - 1)); // no local seat exists
        assert(!SessionSeatsComplete(Layout, HostingMode::Dedicated, 0, Remote + 1));
        assert(SessionSeatsComplete(Layout, HostingMode::Listen, 1, Remote - 1));
        assert(!SessionSeatsComplete(Layout, HostingMode::Listen, 0, Remote));
        assert(!SessionSeatsComplete(Layout, HostingMode::Standalone, 0, 0));
    }
    assert(RemoteSeatCount(static_cast<TeamLayout>(9), HostingMode::Dedicated) == 0);
    assert(!SessionSeatsComplete(static_cast<TeamLayout>(9), HostingMode::Dedicated, 0, 0));
    static_assert(HostingModeName(HostingMode::Dedicated) == "dedicated" && HostingModeName(HostingMode::Listen) == "listen");

    // Shot cues never carry an unseen target point; offline play keeps full presentation.
    static_assert(ShotCueFor(false, false, false) == ShotCue::Exact);
    static_assert(ShotCueFor(true, false, true) == ShotCue::None);
    static_assert(ShotCueFor(true, false, false) == ShotCue::None);
    static_assert(ShotCueFor(true, true, false) == ShotCue::FacingOnly);
    static_assert(ShotCueFor(true, true, true) == ShotCue::Exact);
    static_assert(ShotCueName(ShotCue::FacingOnly) == "facing" && ShotCueName(static_cast<ShotCue>(7)) == "invalid");

    // Development network measurements: nearest-rank percentiles and non-negative rates.
    {
        using politika::metrics::NearestRank;
        using politika::metrics::RatePerSecond;
        constexpr int Ten[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
        static_assert(NearestRank(Ten, 10, 50) == 5 && NearestRank(Ten, 10, 95) == 10 && NearestRank(Ten, 10, 91) == 10);
        static_assert(NearestRank(Ten, 10, 90) == 9 && NearestRank(Ten, 10, 0) == 1 && NearestRank(Ten, 10, 100) == 10);
        static_assert(NearestRank(Ten, 10, 150) == 10 && NearestRank(Ten, 1, 99) == 1);
        static_assert(NearestRank<int>(nullptr, 3, 50) == 0 && NearestRank(Ten, 0, 50) == 0);
        constexpr float Twenty[] = {0.f, 1.f, 2.f, 3.f, 4.f, 5.f, 6.f, 7.f, 8.f, 9.f, 10.f, 11.f, 12.f, 13.f, 14.f, 15.f,
            16.f, 17.f, 18.f, 19.f};
        static_assert(NearestRank(Twenty, 20, 95) == 18.f && NearestRank(Twenty, 20, 5) == 0.f);
        static_assert(RatePerSecond(5000, 2.0) == 2500.0 && RatePerSecond(10, 0.0) == 0.0 && RatePerSecond(-4, 1.0) == 0.0);
    }

    // Development unit-scale fixture: per-seat counts and candidate placement around a start.
    {
        using namespace politika::unitscale;
        static_assert(!ValidUnitsPerSeat(1) && ValidUnitsPerSeat(2) && ValidUnitsPerSeat(20) && !ValidUnitsPerSeat(21));
        static_assert(!ValidUnitsPerSeat(300) && kGameMaxUnitsPerParticipant == 300);
        static_assert(!ValidUnitsPerSeat(0) && !ValidUnitsPerSeat(-5));
        const Bounds Map{{-2000.0, -2000.0}, {2000.0, 2000.0}};
        const Point Own{0.0, 0.0};
        // Alone: nearest first, the start itself, then its four neighbours in row/column order.
        const auto Alone = PlanSeatCandidates(Own, {}, {}, Map, 100.0, 0.0, 5);
        assert(Alone.size() == 5 && Alone[0].x == 0.0 && Alone[0].y == 0.0);
        assert(Alone[1].y == -100.0 && Alone[2].x == -100.0 && Alone[3].x == 100.0 && Alone[4].y == 100.0);
        // Another seat start: no candidate is as close to it as to our own start.
        const Point Other{600.0, 0.0};
        const auto Split = PlanSeatCandidates(Own, std::span(&Other, 1), {}, Map, 100.0, 0.0, 400);
        assert(Split.size() == 400);
        for (const Point& P : Split) assert(DistanceSquared(P, Own) < DistanceSquared(P, Other) && P.x < 300.0);
        // Occupied points keep their clearance; bounds clip; the limit is exact.
        const Point Taken{0.0, 0.0};
        const auto Clear = PlanSeatCandidates(Own, {}, std::span(&Taken, 1), Map, 100.0, 60.0, 3);
        assert(Clear.size() == 3 && !(Clear[0].x == 0.0 && Clear[0].y == 0.0));
        const Bounds Tight{{-150.0, -150.0}, {150.0, 150.0}};
        assert(PlanSeatCandidates(Own, {}, {}, Tight, 100.0, 0.0, 1000).size() == 9);
        assert(PlanSeatCandidates(Own, {}, {}, Map, 0.0, 0.0, 10).empty());
        assert(PlanSeatCandidates(Own, {}, {}, {{1.0, 1.0}, {0.0, 0.0}}, 100.0, 0.0, 10).empty());
    }
    TeamLayout Unparsed = TeamLayout::TwoVsOne;
    assert(!ParseTeamLayout("", Unparsed) && !ParseTeamLayout("4v1", Unparsed) && !ParseTeamLayout("1v1 ", Unparsed)
        && !ParseTeamLayout("3v2", Unparsed) && Unparsed == TeamLayout::TwoVsOne);

    // Selection sets and group-order tallies (RTS_SELECTION_AND_GROUP_ORDERS.md, GRP-02/03/05).
    {
        using namespace politika::selection;
        using Ids = std::vector<EntityId>;
        // GRP-03: the same unit from several hit sources stays one ID, in stable order; 0 is never an entity.
        assert((Normalize({7, 3, 7, 0, 3, 12}) == Ids{3, 7, 12}));
        assert(Normalize({}).empty() && Normalize({0, 0}).empty());
        const Ids Current{2, 4, 6};
        assert((Apply(Current, {9, 1, 9}, SelectionOp::Replace) == Ids{1, 9}));
        assert((Apply(Current, {}, SelectionOp::Replace).empty()));
        assert((Apply(Current, {4, 8}, SelectionOp::Union) == Ids{2, 4, 6, 8}));
        assert((Apply(Current, {4, 8}, SelectionOp::Toggle) == Ids{2, 6, 8}));
        assert((Apply(Current, {4, 8, 4}, SelectionOp::Subtract) == Ids{2, 6}));
        // Unnormalized input is normalized first; repeated operations are stable.
        assert((Apply({6, 2, 6}, {2}, SelectionOp::Toggle) == Ids{6}));
        assert((Apply(Apply(Current, {8}, SelectionOp::Union), {8}, SelectionOp::Union) == Ids{2, 4, 6, 8}));
        assert((Apply(Apply(Current, {8}, SelectionOp::Toggle), {8}, SelectionOp::Toggle) == Current));
        // DPI-aware drag threshold: 6 logical px, scaled; a non-positive scale counts as 1.
        static_assert(!ExceedsDragThreshold({100, 100}, {105, 100}, 1.0) && ExceedsDragThreshold({100, 100}, {106, 100}, 1.0));
        static_assert(!ExceedsDragThreshold({100, 100}, {110, 100}, 2.0) && ExceedsDragThreshold({100, 100}, {100, 112}, 2.0));
        static_assert(ExceedsDragThreshold({0, 0}, {4, 5}, 1.0) && !ExceedsDragThreshold({0, 0}, {3, 4}, 0.0));
        // Any drag direction spans the same rectangle; partial overlap and touching edges intersect.
        static_assert(RectFromCorners({50, 10}, {10, 40}).min.x == 10 && RectFromCorners({50, 10}, {10, 40}).max.y == 40);
        constexpr ScreenRect Box{{10, 10}, {50, 50}};
        static_assert(Intersects(Box, {{45, 45}, {80, 80}}) && Intersects(Box, {{50, 0}, {60, 10}}));
        static_assert(!Intersects(Box, {{51, 10}, {60, 20}}) && !Intersects(Box, {{10, 50.5}, {20, 60}}));
        // Pending-command bound: a fixed allowance plus one per living unit, capped by the game limit.
        static_assert(PendingCommandLimit(0) == 16 && PendingCommandLimit(20) == 36 && PendingCommandLimit(-3) == 16);
        static_assert(PendingCommandLimit(10000) == 16 + kMaxUnitsPerParticipant && kMaxUnitsPerParticipant == 300);
        // GRP-05: each issuer counts on its own; nothing past the expected count is recorded.
        BatchTally<4> Tally{3};
        assert(!Tally.Complete() && Tally.DominantRejection() == 4);
        assert(Tally.Record(true, 0) && Tally.Record(false, 2) && !Tally.Complete());
        assert(Tally.Record(false, 9) && Tally.Complete() && !Tally.Record(true, 0));
        assert(Tally.accepted == 1 && Tally.rejected == 2 && Tally.byRejection[2] == 1 && Tally.DominantRejection() == 2);
        BatchTally<4> Tie{4};
        Tie.Record(false, 3); Tie.Record(false, 1); Tie.Record(false, 3); Tie.Record(false, 1);
        assert(Tie.DominantRejection() == 1 && Tie.rejected == 4);
        static_assert(BatchTally<2>{0}.Complete());
    }

    // Control groups 0-9 (4C-G, GRP-13/14/16/17/21).
    {
        using namespace politika::selection;
        using Ids = std::vector<EntityId>;
        ControlGroups Groups;
        // Unbound slots never accept members.
        assert(!Groups.Assign(1, GroupOp::Replace, {5}) && Groups.Empty());
        assert(!Groups.Bind(1, 3) && Groups.Generation() == 1 && Groups.Participant() == 3);
        // GRP-13: Ctrl replaces, Shift unions without stealing, Alt transfers only the selected IDs.
        assert(Groups.Assign(1, GroupOp::Replace, {9, 2, 9, 0}) && (Groups.Members(1) == Ids{2, 9}));
        assert(Groups.Assign(2, GroupOp::Replace, {2, 4}) && (Groups.Members(2) == Ids{2, 4}));
        assert(Groups.Assign(1, GroupOp::Union, {4, 7}) && (Groups.Members(1) == Ids{2, 4, 7, 9}));
        assert((Groups.Members(2) == Ids{2, 4}));
        assert(Groups.Assign(0, GroupOp::Transfer, {4, 9}) && (Groups.Members(0) == Ids{4, 9}));
        assert((Groups.Members(1) == Ids{2, 7}) && (Groups.Members(2) == Ids{2}));
        // Repeating each operation is idempotent.
        assert(Groups.Assign(0, GroupOp::Transfer, {4, 9}) && (Groups.Members(0) == Ids{4, 9}) && (Groups.Members(1) == Ids{2, 7}));
        assert(Groups.Assign(1, GroupOp::Union, {7}) && (Groups.Members(1) == Ids{2, 7}));
        assert(Groups.Assign(2, GroupOp::Replace, {2}) && (Groups.Members(2) == Ids{2}));
        // An empty selection edits nothing; slot 10 does not exist.
        assert(!Groups.Assign(1, GroupOp::Replace, {}) && !Groups.Assign(1, GroupOp::Replace, {0}) && (Groups.Members(1) == Ids{2, 7}));
        assert(!Groups.Assign(kControlGroupSlots, GroupOp::Replace, {5}) && Groups.Members(kControlGroupSlots).empty());
        // GRP-14: overlapping slots share an ID; recall of both stays one normalized ID each.
        assert((Apply(Groups.Members(1), Groups.Members(2), SelectionOp::Union) == Ids{2, 7}));
        // Explicit removal touches one slot only.
        assert(Groups.Remove(1, {2}) && (Groups.Members(1) == Ids{7}) && (Groups.Members(2) == Ids{2}) && !Groups.Remove(1, {2}));
        // GRP-16: permanently invalid references leave every slot at once.
        assert(Groups.Assign(3, GroupOp::Replace, {2, 7, 9}));
        assert(Groups.Prune([](EntityId Id) { return Id != 2 && Id != 9; }) == 4);
        assert(Groups.Members(2).empty() && (Groups.Members(3) == Ids{7}) && Groups.Members(0) == Ids{4});
        assert(Groups.Prune([](EntityId) { return true; }) == 0);
        // A new match generation or another participant clears every slot.
        assert(!Groups.Bind(1, 3) && !Groups.Empty());
        assert(Groups.Bind(2, 3) && Groups.Empty() && Groups.Generation() == 2);
        assert(Groups.Assign(5, GroupOp::Replace, {11}) && Groups.Bind(2, 4) && Groups.Empty());
        assert(!Groups.Bind(2, 4));
        Groups.Clear();
        assert(Groups.Participant() == 0 && !Groups.Assign(5, GroupOp::Replace, {11}));

        // GRP-15: a double tap is the same slot within the window; a third tap is a plain recall.
        RecallTapTracker Taps;
        assert(!Taps.Tap(1, 10.0) && Taps.Tap(1, 10.2) && !Taps.Tap(1, 10.3) && Taps.Tap(1, 10.5));
        assert(!Taps.Tap(2, 20.0) && !Taps.Tap(3, 20.1) && !Taps.Tap(3, 20.6) && !Taps.Tap(2, 20.7));
        assert(!Taps.Tap(4, 0.0) && Taps.Tap(4, kGroupDoubleTapSeconds));
        Taps.Reset();
        assert(!Taps.Tap(4, 31.0));
        assert(!Taps.Tap(6, 40.0) && !Taps.Tap(6, 39.0));

        // GRP-17: subgroups follow each archetype's lowest ID; Tab wraps both ways.
        assert((SubgroupOrder<int>({{9, 2}, {3, 1}, {5, 2}, {1, 7}}) == std::vector<int>{7, 1, 2}));
        assert(SubgroupOrder<int>({}).empty());
        static_assert(CycleSubgroup(0, 3, true) == 1 && CycleSubgroup(2, 3, true) == 0);
        static_assert(CycleSubgroup(0, 3, false) == 2 && CycleSubgroup(0, 1, true) == 0 && CycleSubgroup(4, 0, true) == 0);

        // GRP-21: the bar shows 1..9 then 0, centred, and hit-tests each slot exactly.
        static_assert(GroupBarSlotAtPosition(0) == 1 && GroupBarSlotAtPosition(8) == 9 && GroupBarSlotAtPosition(9) == 0);
        constexpr ScreenRect One = GroupBarSlotRect(1, 1920, 1080, 1.0);
        constexpr ScreenRect Zero = GroupBarSlotRect(0, 1920, 1080, 1.0);
        static_assert(One.min.x == 613.0 && One.max.x == 677.0 && One.min.y == 1012.0 && One.max.y == 1058.0);
        static_assert(Zero.min.x == 1243.0 && Zero.max.x == 1307.0);
        static_assert(GroupBarSlotAt({620, 1030}, 1920, 1080, 1.0) == 1 && GroupBarSlotAt({1300, 1030}, 1920, 1080, 1.0) == 0);
        static_assert(GroupBarSlotAt({679, 1030}, 1920, 1080, 1.0) == kControlGroupSlots);
        static_assert(GroupBarSlotAt({620, 1000}, 1920, 1080, 1.0) == kControlGroupSlots);
        static_assert(GroupBarSlotAt({1240, 2060}, 3840, 2160, 2.0) == 1 && GroupBarSlotRect(1, 3840, 2160, 2.0).min.x == 1226.0);
        // A narrow view moves the bar right of the command panel instead of under it.
        static_assert(GroupBarSlotRect(1, 1280, 720, 1.0).min.x == 446.0 && GroupBarSlotRect(0, 1280, 720, 1.0).max.x == 1140.0);
        static_assert(GroupBarSlotAt({450, 670}, 1280, 720, 1.0) == 1 && GroupBarSlotAt({300, 670}, 1280, 720, 1.0) == kControlGroupSlots);
    }

    // FORM-F2 group-move geometry: Line/Column slots, facing, unique legal destinations and a
    // deterministic, crossing-free assignment. Geometry only; no formation state exists.
    {
        using namespace politika::formation;
        const auto Always = [](Point) { return true; };
        const auto Near = [](Point a, Point b) { return std::abs(a.x - b.x) < 1e-9 && std::abs(a.y - b.y) < 1e-9; };
        static_assert(NextShape(Shape::Line) == Shape::Column && NextShape(Shape::Column) == Shape::Dispersed
            && NextShape(Shape::Dispersed) == Shape::Perimeter && NextShape(Shape::Perimeter) == Shape::Wedge
            && NextShape(Shape::Wedge) == Shape::EchelonLeft && NextShape(Shape::EchelonLeft) == Shape::EchelonRight
            && NextShape(Shape::EchelonRight) == Shape::Line && NextShape(static_cast<Shape>(9)) == Shape::Line);
        static_assert(PreviousShape(Shape::Line) == Shape::EchelonRight && PreviousShape(Shape::Column) == Shape::Line
            && PreviousShape(Shape::Wedge) == Shape::Perimeter && PreviousShape(static_cast<Shape>(9)) == Shape::Line);
        static_assert(ShapeName(Shape::Dispersed) == "dispersed" && ShapeName(Shape::Perimeter) == "perimeter"
            && ShapeName(Shape::Wedge) == "wedge" && ShapeName(Shape::EchelonLeft) == "echelon-left"
            && ShapeName(Shape::EchelonRight) == "echelon-right" && ShapeName(Shape::Line) == "line" && kShapeCount == 7);
        static_assert(ShapeName(Shape::Column) == "column" && Frontage(Shape::Line, 8, {}) == 8
            && Frontage(Shape::Line, 20, {}) == 10 && Frontage(Shape::Column, 20, {}) == 2 && Frontage(Shape::Column, 1, {}) == 1);

        // Eight units: one centred rank of eight for Line; four ranks of two for Column.
        const std::vector<Slot> Line8 = RelativeSlots(Shape::Line, 8, 0, {});
        assert(Line8.size() == 8 && Line8.front().lateral == -525.0 && Line8.back().lateral == 525.0);
        assert(std::all_of(Line8.begin(), Line8.end(), [](const Slot& s) { return s.forward == 0.0 && s.rank == 0; }));
        const std::vector<Slot> Column8 = RelativeSlots(Shape::Column, 8, 0, {});
        assert(Column8[0].lateral == -75.0 && Column8[1].lateral == 75.0 && Column8[0].forward == 225.0);
        assert(Column8[7].forward == -225.0 && Column8[7].rank == 3);
        // A partial last rank is centred; spares are full ranks behind the block.
        const std::vector<Slot> Line13 = RelativeSlots(Shape::Line, 13, 13, {});
        assert(Line13.size() == 26 && Line13[10].rank == 1 && Line13[10].lateral == -150.0 && Line13[12].lateral == 150.0);
        assert(Line13[13].rank == 2 && Line13[13].lateral == -675.0 && Line13[13].forward == -225.0);
        assert(RelativeSlots(Shape::Line, 0, 4, {}).empty());

        // Eight units standing in a cluster west of the anchor, ordered due east.
        std::vector<Member> Group;
        for (EntityId Id = 1; Id <= 8; ++Id)
            Group.push_back({Id, {1000.0 + static_cast<double>((Id - 1) % 4) * 40.0, 2000.0 + static_cast<double>((Id - 1) / 4) * 40.0}});
        const Point Anchor{3000.0, 2020.0};
        const Plan Line = PlanGroupMove(Group, Anchor, Shape::Line, Always);
        assert(Line.assignments.size() == 8 && Line.unplaced == 0 && Line.candidatesRejected == 0);
        assert(Near(Line.facing, {1.0, 0.0}));
        // FORM-04: unique, finite destinations a full spacing apart, centred on the anchor.
        assert(std::abs(MinimumSeparation(Line) - 150.0) < 1e-9);
        Point Sum{0, 0};
        for (const Assignment& A : Line.assignments) {
            assert(IsFinite(A.destination) && A.slot < 8 && A.destination.x == 3000.0);
            Sum = {Sum.x + A.destination.x, Sum.y + A.destination.y};
        }
        assert(std::abs(Sum.x / 8 - Anchor.x) < 1e-9 && std::abs(Sum.y / 8 - Anchor.y) < 1e-9);
        for (std::size_t Index = 1; Index < Line.assignments.size(); ++Index)
            assert(Line.assignments[Index - 1].id < Line.assignments[Index].id);
        // Facing east, right is +y: members keep their left-to-right order, so nobody crosses.
        const auto DestinationOf = [](const Plan& P, EntityId Id) {
            return std::find_if(P.assignments.begin(), P.assignments.end(), [Id](const Assignment& A) { return A.id == Id; })->destination;
        };
        assert(DestinationOf(Line, 1).y < DestinationOf(Line, 5).y && DestinationOf(Line, 4).y < DestinationOf(Line, 8).y);

        // Input order and duplicates do not change the plan.
        std::vector<Member> Shuffled(Group.rbegin(), Group.rend());
        Shuffled.push_back(Group[2]);
        const Plan Again = PlanGroupMove(Shuffled, Anchor, Shape::Line, Always);
        assert(Again.assignments.size() == 8);
        for (std::size_t Index = 0; Index < 8; ++Index)
            assert(Again.assignments[Index].id == Line.assignments[Index].id
                && Near(Again.assignments[Index].destination, Line.assignments[Index].destination));

        // Column: two files along the travel direction; the front pair is the east-most pair.
        const Plan Column = PlanGroupMove(Group, Anchor, Shape::Column, Always);
        assert(Column.assignments.size() == 8 && std::abs(MinimumSeparation(Column) - 150.0) < 1e-9);
        double MinX = 1e9, MaxX = -1e9, MinY = 1e9, MaxY = -1e9;
        for (const Assignment& A : Column.assignments) {
            MinX = std::min(MinX, A.destination.x); MaxX = std::max(MaxX, A.destination.x);
            MinY = std::min(MinY, A.destination.y); MaxY = std::max(MaxY, A.destination.y);
        }
        assert(MaxX - MinX == 450.0 && MaxY - MinY == 150.0);
        assert(DestinationOf(Column, 4).x == 3225.0 && DestinationOf(Column, 1).x == 2775.0);

        // Facing follows the travel direction (here due north, -y), so the line runs east-west.
        const Plan North = PlanGroupMove(Group, {1060.0, 0.0}, Shape::Line, Always);
        assert(Near(North.facing, {0.0, -1.0}) && std::abs(MinimumSeparation(North) - 150.0) < 1e-9);
        assert(std::all_of(North.assignments.begin(), North.assignments.end(), [](const Assignment& A) { return A.destination.y == 0.0; }));
        // An anchor inside the group gives no travel direction: the fallback facing is used.
        const Plan InPlace = PlanGroupMove(Group, {1060.0, 2020.0}, Shape::Line, Always, {}, {0.0, 2.0});
        assert(Near(InPlace.facing, {0.0, 1.0}));

        // Blocked slots (here: everything with y < 1800, the three left-most of the line) are
        // skipped and replaced by spare slots in the rank behind, never by an overlapping
        // destination: 5 nominal + 3 spare placed after 14 probes.
        std::size_t Probes = 0;
        const auto Open = [&Probes](Point P) { ++Probes; return P.y >= 1800.0; };
        const Plan Blocked = PlanGroupMove(Group, Anchor, Shape::Line, Open);
        assert(Blocked.unplaced == 0 && Blocked.candidatesRejected == 6 && Probes == Blocked.candidatesChecked && Probes == 14);
        assert(std::all_of(Blocked.assignments.begin(), Blocked.assignments.end(),
            [](const Assignment& A) { return A.slot != kNoSlot && A.destination.y >= 1800.0; }));
        assert(std::count_if(Blocked.assignments.begin(), Blocked.assignments.end(),
            [](const Assignment& A) { return A.destination.x == 2850.0; }) == 3);
        assert(std::abs(MinimumSeparation(Blocked) - 150.0) < 1e-9);
        // With no legal slot at all, everyone keeps the plain shared-point order, and the
        // projection is probed at most twice per member.
        Probes = 0;
        const auto Nowhere = [&Probes](Point) { ++Probes; return false; };
        const Plan None = PlanGroupMove(Group, Anchor, Shape::Column, Nowhere);
        assert(None.unplaced == 8 && Probes == 16 && std::isinf(MinimumSeparation(None)));
        assert(std::all_of(None.assignments.begin(), None.assignments.end(),
            [&](const Assignment& A) { return A.slot == kNoSlot && Near(A.destination, Anchor); }));
        // Too few legal slots: the rear-most members fall back to the anchor, the rest are placed.
        int Allowed = 5;
        const Plan Partial = PlanGroupMove(Group, Anchor, Shape::Line, [&Allowed](Point) { return Allowed-- > 0; });
        assert(Partial.unplaced == 3 && std::count_if(Partial.assignments.begin(), Partial.assignments.end(),
            [](const Assignment& A) { return A.slot == kNoSlot; }) == 3);

        // A single member goes exactly where it was ordered, without a projection probe.
        Probes = 0;
        const Plan Single = PlanGroupMove(std::span<const Member>(Group.data(), 1), Anchor, Shape::Line, Nowhere);
        assert(Single.assignments.size() == 1 && Near(Single.assignments[0].destination, Anchor) && Probes == 0);
        // Malformed input fails closed to the shared point.
        assert(PlanGroupMove(std::span<const Member>{}, Anchor, Shape::Line, Always).assignments.empty());
        const Plan BadAnchor = PlanGroupMove(Group, {std::nan(""), 0.0}, Shape::Line, Always);
        assert(BadAnchor.unplaced == 8);
        Layout Zero; Zero.spacingCm = 0.0;
        assert(PlanGroupMove(Group, Anchor, Shape::Line, Always, Zero).unplaced == 8);
        std::vector<Member> Huge;
        for (EntityId Id = 1; Id <= 301; ++Id) Huge.push_back({Id, {0.0, static_cast<double>(Id)}});
        assert(PlanGroupMove(Huge, Anchor, Shape::Line, Always).unplaced == 301);
        // The skirmish-map cap of 20: Line is two ranks of ten, every slot distinct.
        std::vector<Member> Twenty;
        for (EntityId Id = 1; Id <= 20; ++Id) Twenty.push_back({Id, {500.0 + static_cast<double>(Id % 5) * 100.0, 500.0 + static_cast<double>(Id / 5) * 100.0}});
        const Plan Twenty20 = PlanGroupMove(Twenty, {4000.0, 600.0}, Shape::Line, Always);
        assert(Twenty20.unplaced == 0 && std::abs(MinimumSeparation(Twenty20) - 150.0) < 1e-9);
        const Plan Column20 = PlanGroupMove(Twenty, {4000.0, 600.0}, Shape::Column, Always);
        assert(Column20.unplaced == 0 && std::abs(MinimumSeparation(Column20) - 150.0) < 1e-9);
        // A fixed facing overrides the travel direction (regroups keep the repaired move's facing).
        const Plan Fixed = PlanGroupMove(Group, Anchor, Shape::Line, Always, {}, {1.0, 0.0}, {0.0, -2.0});
        assert(Near(Fixed.facing, {0.0, -1.0}) && Fixed.unplaced == 0);
        assert(Near(PlanGroupMove(Group, Anchor, Shape::Line, Always, {}, {1.0, 0.0}, {std::nan(""), 1.0}).facing, {1.0, 0.0}));
    }

    // FORM-F6 Dispersed and Perimeter: a staggered block beyond the Discipline support radius,
    // and rings around the anchor with every neighbouring chord at least one spacing.
    {
        using namespace politika::formation;
        const auto Always = [](Point) { return true; };
        const auto Distance = [](Point a, Point b) { return std::hypot(a.x - b.x, a.y - b.y); };
        static_assert(Frontage(Shape::Dispersed, 20, {}) == 5 && Frontage(Shape::Dispersed, 3, {}) == 3);
        static_assert(ShapeSpacing(Shape::Dispersed, {}) == 450.0 && ShapeSpacing(Shape::Perimeter, {}) == 150.0);
        static_assert(Layout{}.dispersedSpacingCm > politika::discipline::kSupportRadiusCm);

        // Dispersed: ranks of five 450 cm apart, odd ranks half a spacing to the right, the
        // block centred on the anchor.
        const std::vector<Slot> Dispersed20 = RelativeSlots(Shape::Dispersed, 20, 0, {});
        assert(Dispersed20.size() == 20 && Dispersed20[0].rank == 0 && Dispersed20[19].rank == 3);
        assert(Dispersed20[0].lateral == -1012.5 && Dispersed20[5].lateral == -787.5 && Dispersed20[0].forward == 675.0);
        double LateralSum = 0.0, ForwardSum = 0.0;
        for (const Slot& S : Dispersed20) { LateralSum += S.lateral; ForwardSum += S.forward; }
        assert(std::abs(LateralSum) < 1e-9 && std::abs(ForwardSum) < 1e-9);
        // One rank is not staggered; spares continue the stagger behind the block.
        const std::vector<Slot> Dispersed4 = RelativeSlots(Shape::Dispersed, 4, 4, {});
        assert(Dispersed4[0].lateral == -675.0 && Dispersed4[3].lateral == 675.0 && Dispersed4[4].rank == 1);
        const std::vector<Slot> Dispersed7 = RelativeSlots(Shape::Dispersed, 7, 5, {});
        assert(Dispersed7[5].lateral == -112.5 && Dispersed7[7].rank == 2 && Dispersed7[7].lateral == -1012.5);

        std::vector<Member> Twenty;
        for (EntityId Id = 1; Id <= 20; ++Id)
            Twenty.push_back({Id, {500.0 + static_cast<double>(Id % 5) * 100.0, 500.0 + static_cast<double>(Id / 5) * 100.0}});
        const Plan Spread = PlanGroupMove(Twenty, {4000.0, 600.0}, Shape::Dispersed, Always);
        assert(Spread.shape == Shape::Dispersed && Spread.unplaced == 0 && Spread.assignments.size() == 20);
        assert(std::abs(MinimumSeparation(Spread) - 450.0) < 1e-9);
        // Every member stands beyond the support radius of every other: none can earn Discipline.
        for (const Assignment& A : Spread.assignments)
            for (const Assignment& B : Spread.assignments)
                assert(A.id == B.id || Distance(A.destination, B.destination) > politika::discipline::kSupportRadiusCm);
        // Shuffled input gives the same plan.
        std::vector<Member> Reversed(Twenty.rbegin(), Twenty.rend());
        const Plan SpreadAgain = PlanGroupMove(Reversed, {4000.0, 600.0}, Shape::Dispersed, Always);
        for (std::size_t Index = 0; Index < 20; ++Index)
            assert(SpreadAgain.assignments[Index].id == Spread.assignments[Index].id
                && Distance(SpreadAgain.assignments[Index].destination, Spread.assignments[Index].destination) == 0.0);
        Layout BadSpread; BadSpread.dispersedSpacingCm = std::nan("");
        assert(PlanGroupMove(Twenty, {4000.0, 600.0}, Shape::Dispersed, Always, BadSpread).unplaced == 20);

        // Perimeter radius and ring capacity: chords of exactly one spacing, at least 300 cm.
        assert(PerimeterRadius(2, {}) == 300.0 && PerimeterRadius(12, {}) == 300.0);
        assert(std::abs(2.0 * PerimeterRadius(20, {}) * std::sin(kPi / 20.0) - 150.0) < 1e-9);
        assert(RingCapacity(1200.0, 150.0) == 50 && RingCapacity(300.0, 150.0) == 12 && RingCapacity(50.0, 150.0) == 1);
        assert(RingCapacity(0.0, 150.0) == 0 && RingCapacity(300.0, std::nan("")) == 0);
        assert(RingCapacity(PerimeterRadius(20, {}), 150.0) == 20);

        // Twenty: one ring centred on the anchor, slot 0 straight ahead, clockwise.
        const std::vector<Slot> Ring20 = RelativeSlots(Shape::Perimeter, 20, 20, {});
        assert(Ring20.size() == 40 && Ring20[0].lateral == 0.0 && Ring20[0].rank == 0 && Ring20[20].rank == 1);
        assert(Ring20[5].lateral > 0.0 && std::abs(Ring20[5].forward) < 1e-9);
        const double R20 = PerimeterRadius(20, {});
        for (std::size_t Index = 0; Index < 20; ++Index)
            assert(std::abs(std::hypot(Ring20[Index].lateral, Ring20[Index].forward) - R20) < 1e-9);
        // Spares sit on the next ring out, one spacing beyond.
        assert(std::abs(std::hypot(Ring20[20].lateral, Ring20[20].forward) - (R20 + 150.0)) < 1e-9);
        const Point Centre{4000.0, 600.0};
        const Plan Ring = PlanGroupMove(Twenty, Centre, Shape::Perimeter, Always);
        assert(Ring.shape == Shape::Perimeter && Ring.unplaced == 0 && Ring.candidatesRejected == 0);
        assert(std::abs(MinimumSeparation(Ring) - 150.0) < 1e-9);
        Point Sum{0.0, 0.0};
        std::vector<std::size_t> Used;
        for (const Assignment& A : Ring.assignments) {
            assert(A.slot < 20 && std::abs(Distance(A.destination, Centre) - R20) < 1e-9);
            Sum = {Sum.x + A.destination.x, Sum.y + A.destination.y};
            Used.push_back(A.slot);
        }
        std::sort(Used.begin(), Used.end());
        assert(std::unique(Used.begin(), Used.end()) == Used.end());
        assert(std::abs(Sum.x / 20 - Centre.x) < 1e-6 && std::abs(Sum.y / 20 - Centre.y) < 1e-6);
        // Fellow members within the support radius: each has its two ring neighbours.
        for (const Assignment& A : Ring.assignments)
            assert(std::count_if(Ring.assignments.begin(), Ring.assignments.end(), [&](const Assignment& B) {
                return B.id != A.id && Distance(A.destination, B.destination) <= politika::discipline::kSupportRadiusCm;
            }) >= 2);
        const Plan RingAgain = PlanGroupMove(Reversed, Centre, Shape::Perimeter, Always);
        for (std::size_t Index = 0; Index < 20; ++Index)
            assert(RingAgain.assignments[Index].id == Ring.assignments[Index].id
                && Distance(RingAgain.assignments[Index].destination, Ring.assignments[Index].destination) == 0.0);

        // Members already standing around the anchor keep their side of it: eight soldiers on a
        // 400 cm circle each take the ring slot nearest them.
        std::vector<Member> Around;
        for (EntityId Id = 1; Id <= 8; ++Id) {
            const double Angle = 2.0 * kPi * static_cast<double>((Id * 3) % 8) / 8.0;
            Around.push_back({Id, {1000.0 + 400.0 * std::cos(Angle), 1000.0 + 400.0 * std::sin(Angle)}});
        }
        const Plan InPlace = PlanGroupMove(Around, {1000.0, 1000.0}, Shape::Perimeter, Always, {}, {1.0, 0.0}, {1.0, 0.0});
        for (const Assignment& A : InPlace.assignments) {
            const Point From = std::find_if(Around.begin(), Around.end(), [&](const Member& M) { return M.id == A.id; })->position;
            assert(Distance(From, A.destination) < 100.0 + 1e-9);
        }

        // Blocked ring slots (everything east of x = 4300) are replaced from the outer spare
        // ring; with too few legal slots the members farthest from the anchor are unplaced.
        std::size_t Probes = 0;
        const auto West = [&Probes](Point P) { ++Probes; return P.x <= 4300.0; };
        const Plan Blocked = PlanGroupMove(Twenty, Centre, Shape::Perimeter, West);
        assert(Blocked.unplaced == 0 && Blocked.candidatesRejected > 0 && Probes == Blocked.candidatesChecked && Probes <= 40);
        assert(std::all_of(Blocked.assignments.begin(), Blocked.assignments.end(),
            [](const Assignment& A) { return A.slot != kNoSlot && A.destination.x <= 4300.0; }));
        assert(MinimumSeparation(Blocked) >= 150.0 - 1e-9);
        int Allowed = 15;
        const Plan Short = PlanGroupMove(Twenty, Centre, Shape::Perimeter, [&Allowed](Point) { return Allowed-- > 0; });
        assert(Short.unplaced == 5);
        double PlacedFarthest = 0.0, UnplacedNearest = 1e18;
        for (const Assignment& A : Short.assignments) {
            const double From = Distance(std::find_if(Twenty.begin(), Twenty.end(), [&](const Member& M) { return M.id == A.id; })->position, Centre);
            if (A.slot == kNoSlot) UnplacedNearest = std::min(UnplacedNearest, From);
            else PlacedFarthest = std::max(PlacedFarthest, From);
        }
        assert(PlacedFarthest <= UnplacedNearest);

        // Large groups: full rings from 1200 cm inwards (50, 43, 37, 31, 25, 18, 12 = 216 slots),
        // then outwards from 1350 cm; every slot unique and one spacing from every other.
        const std::vector<Slot> Ring300 = RelativeSlots(Shape::Perimeter, 300, 0, {});
        assert(Ring300.size() == 300 && Ring300[0].rank == 0 && Ring300[49].rank == 0 && Ring300[50].rank == 1);
        assert(std::abs(std::hypot(Ring300[50].lateral, Ring300[50].forward) - 1050.0) < 1e-9);
        assert(std::abs(std::hypot(Ring300[216].lateral, Ring300[216].forward) - 1350.0) < 1e-9);
        double Closest = 1e18;
        for (std::size_t A = 0; A < Ring300.size(); ++A)
            for (std::size_t B = A + 1; B < Ring300.size(); ++B)
                Closest = std::min(Closest, std::hypot(Ring300[A].lateral - Ring300[B].lateral, Ring300[A].forward - Ring300[B].forward));
        assert(Closest >= 150.0 - 1e-9);
        std::vector<Member> Many;
        for (EntityId Id = 1; Id <= 300; ++Id)
            Many.push_back({Id, {static_cast<double>(Id % 20) * 100.0, static_cast<double>(Id / 20) * 100.0}});
        const Plan Big = PlanGroupMove(Many, {6000.0, 6000.0}, Shape::Perimeter, Always);
        assert(Big.unplaced == 0 && Big.assignments.size() == 300 && MinimumSeparation(Big) >= 150.0 - 1e-9);
        // Two members: opposite sides of the minimum ring.
        const Plan Pair = PlanGroupMove(std::span<const Member>(Twenty.data(), 2), Centre, Shape::Perimeter, Always);
        assert(std::abs(MinimumSeparation(Pair) - 600.0) < 1e-9);
        Layout BadRing; BadRing.perimeterMaxRadiusCm = 100.0;
        assert(RingSlots(20, 0, BadRing).empty() && PlanGroupMove(Twenty, Centre, Shape::Perimeter, Always, BadRing).unplaced == 20);
        assert(PlanGroupMove(Twenty, Centre, static_cast<Shape>(7), Always).unplaced == 20);
    }

    // FORM-F6 Wedge and Echelon: an arrowhead on a triangular lattice and a Line turned 45
    // degrees, both one spacing between neighbours and centred on the anchor.
    {
        using namespace politika::formation;
        const auto Always = [](Point) { return true; };
        const auto Distance = [](Point a, Point b) { return std::hypot(a.x - b.x, a.y - b.y); };
        const auto Closest = [](const std::vector<Slot>& Slots) {
            double Best = 1e18;
            for (std::size_t A = 0; A < Slots.size(); ++A)
                for (std::size_t B = A + 1; B < Slots.size(); ++B)
                    Best = std::min(Best, std::hypot(Slots[A].lateral - Slots[B].lateral, Slots[A].forward - Slots[B].forward));
            return Best;
        };
        const auto Mean = [](const std::vector<Slot>& Slots, std::size_t Count) {
            Point Sum{0.0, 0.0};
            for (std::size_t Index = 0; Index < Count; ++Index) Sum = {Sum.x + Slots[Index].lateral, Sum.y + Slots[Index].forward};
            return Point{Sum.x / static_cast<double>(Count), Sum.y / static_cast<double>(Count)};
        };
        static_assert(EchelonSide(Shape::EchelonRight) == 1 && EchelonSide(Shape::EchelonLeft) == -1 && EchelonSide(Shape::Wedge) == 0);

        // Wedge of 20: ranks of 1, 2, 3, 4, 5 and a partial 5 of 6, sqrt(3)/2 spacings apart;
        // the point alone in front, centred laterally, the nominal centroid on the anchor.
        const std::vector<Slot> Wedge20 = RelativeSlots(Shape::Wedge, 20, 20, {});
        assert(Wedge20.size() == 40 && Wedge20[0].rank == 0 && Wedge20[1].rank == 1 && Wedge20[14].rank == 4 && Wedge20[15].rank == 5);
        assert(Wedge20[19].rank == 5 && Wedge20[20].rank == 6);
        assert(std::abs(Closest(Wedge20) - 150.0) < 1e-9);
        assert(std::abs(Mean(Wedge20, 20).x) < 1e-9 && std::abs(Mean(Wedge20, 20).y) < 1e-9);
        assert(std::all_of(Wedge20.begin() + 1, Wedge20.end(), [&](const Slot& S) { return S.forward < Wedge20[0].forward; }));
        assert(std::abs(Wedge20[0].forward - Wedge20[1].forward - 150.0 * kHalfSqrt3) < 1e-9);
        assert(std::abs(Wedge20[1].lateral - Wedge20[0].lateral + 75.0) < 1e-9 && std::abs(Wedge20[2].lateral - Wedge20[1].lateral - 150.0) < 1e-9);
        // The partial last rank keeps lattice positions: five of the six, the right-most left out.
        assert(std::abs(Wedge20[15].lateral - Wedge20[10].lateral + 75.0) < 1e-9);
        assert(std::abs(Wedge20[19].lateral - Wedge20[15].lateral - 600.0) < 1e-9);
        // Every rank-r slot of a full rank has its lattice neighbours one spacing away; nothing
        // is closer, also for the partial rank against the rank in front (checked by Closest).
        const std::vector<Slot> Wedge7 = RelativeSlots(Shape::Wedge, 7, 0, {});
        assert(Wedge7.size() == 7 && Wedge7[6].rank == 3 && std::abs(Closest(Wedge7) - 150.0) < 1e-9);
        assert(std::abs(Mean(Wedge7, 7).x) < 1e-9 && std::abs(Mean(Wedge7, 7).y) < 1e-9);
        const std::vector<Slot> Wedge8 = RelativeSlots(Shape::Wedge, 8, 0, {});
        assert(std::abs(Closest(Wedge8) - 150.0) < 1e-9 && std::abs(Mean(Wedge8, 8).x) < 1e-9);
        assert(RelativeSlots(Shape::Wedge, 0, 4, {}).empty() && RelativeSlots(Shape::Wedge, 1, 0, {})[0].forward == 0.0);
        Layout BadWedge; BadWedge.spacingCm = std::nan("");
        assert(WedgeSlots(20, 0, BadWedge).empty());

        std::vector<Member> Twenty;
        for (EntityId Id = 1; Id <= 20; ++Id)
            Twenty.push_back({Id, {500.0 + static_cast<double>(Id % 5) * 100.0, 500.0 + static_cast<double>(Id / 5) * 100.0}});
        std::vector<Member> Reversed(Twenty.rbegin(), Twenty.rend());
        const Point Target{4000.0, 600.0};
        const Plan Arrow = PlanGroupMove(Twenty, Target, Shape::Wedge, Always);
        assert(Arrow.shape == Shape::Wedge && Arrow.unplaced == 0 && Arrow.candidatesRejected == 0 && Arrow.assignments.size() == 20);
        assert(std::abs(MinimumSeparation(Arrow) - 150.0) < 1e-9);
        // The point member stands farthest along the travel direction.
        const auto Point0 = std::find_if(Arrow.assignments.begin(), Arrow.assignments.end(), [](const Assignment& A) { return A.slot == 0; });
        for (const Assignment& A : Arrow.assignments)
            assert(A.slot == 0 || (A.destination.x - Target.x) * Arrow.facing.x + (A.destination.y - Target.y) * Arrow.facing.y
                < (Point0->destination.x - Target.x) * Arrow.facing.x + (Point0->destination.y - Target.y) * Arrow.facing.y);
        // Every Wedge member has a fellow member within the support radius: it can earn.
        for (const Assignment& A : Arrow.assignments)
            assert(std::count_if(Arrow.assignments.begin(), Arrow.assignments.end(), [&](const Assignment& B) {
                return B.id != A.id && Distance(A.destination, B.destination) <= politika::discipline::kSupportRadiusCm;
            }) >= 2);
        const Plan ArrowAgain = PlanGroupMove(Reversed, Target, Shape::Wedge, Always);
        for (std::size_t Index = 0; Index < 20; ++Index)
            assert(ArrowAgain.assignments[Index].id == Arrow.assignments[Index].id
                && Distance(ArrowAgain.assignments[Index].destination, Arrow.assignments[Index].destination) == 0.0);

        // Echelon Right: the Line block turned so the right flank falls back; distances unchanged.
        const std::vector<Slot> Line20 = RelativeSlots(Shape::Line, 20, 20, {});
        const std::vector<Slot> Right20 = RelativeSlots(Shape::EchelonRight, 20, 20, {});
        const std::vector<Slot> Left20 = RelativeSlots(Shape::EchelonLeft, 20, 20, {});
        assert(Right20.size() == 40 && Left20.size() == 40 && std::abs(Closest(Right20) - 150.0) < 1e-9 && std::abs(Closest(Left20) - 150.0) < 1e-9);
        for (std::size_t Index = 0; Index < 40; ++Index) {
            assert(Right20[Index].rank == Line20[Index].rank && Left20[Index].rank == Line20[Index].rank);
            const Slot Back = EchelonTurn(Right20[Index], -1);
            assert(std::abs(Back.lateral - Line20[Index].lateral) < 1e-9 && std::abs(Back.forward - Line20[Index].forward) < 1e-9);
        }
        // Front rank of ten: the right-most file is the farthest back, the left-most the farthest
        // forward, 9 x 150 / sqrt(2) apart in depth.
        assert(Right20[9].forward < Right20[0].forward && std::abs(Right20[0].forward - Right20[9].forward - 1350.0 * kHalfSqrt2) < 1e-9);
        assert(Left20[0].forward < Left20[9].forward && std::abs(Left20[9].forward - Left20[0].forward - 1350.0 * kHalfSqrt2) < 1e-9);
        // Left is the mirror image of Right: the same slot of the mirrored Line, turned right.
        for (std::size_t Index = 0; Index < 40; ++Index) {
            const Slot Mirrored = EchelonTurn({-Line20[Index].lateral, Line20[Index].forward, 0}, 1);
            assert(std::abs(Left20[Index].forward - Mirrored.forward) < 1e-9 && std::abs(Left20[Index].lateral + Mirrored.lateral) < 1e-9);
        }
        assert(std::abs(Mean(Right20, 20).x) < 1e-9 && std::abs(Mean(Right20, 20).y) < 1e-9);

        for (const Shape Echelon : {Shape::EchelonLeft, Shape::EchelonRight}) {
            const Plan Stepped = PlanGroupMove(Twenty, Target, Echelon, Always);
            assert(Stepped.shape == Echelon && Stepped.unplaced == 0 && Stepped.assignments.size() == 20);
            assert(std::abs(MinimumSeparation(Stepped) - 150.0) < 1e-9);
            const Plan SteppedAgain = PlanGroupMove(Reversed, Target, Echelon, Always);
            for (std::size_t Index = 0; Index < 20; ++Index)
                assert(SteppedAgain.assignments[Index].id == Stepped.assignments[Index].id
                    && Distance(SteppedAgain.assignments[Index].destination, Stepped.assignments[Index].destination) == 0.0);
        }

        // A row of ten walking north keeps its left-to-right order in every new shape: the
        // left-most member takes a left-most slot of its rank (no crossing).
        std::vector<Member> Row;
        for (EntityId Id = 1; Id <= 10; ++Id) Row.push_back({Id, {0.0, static_cast<double>(Id) * 150.0}});
        for (const Shape Turned : {Shape::Wedge, Shape::EchelonLeft, Shape::EchelonRight}) {
            const Plan North = PlanGroupMove(Row, {5000.0, 825.0}, Turned, Always);
            assert(North.unplaced == 0 && std::abs(North.facing.x - 1.0) < 1e-9);
            // Facing +x, right is +y: within a rank, slot lateral order equals member y order.
            for (const Assignment& A : North.assignments)
                for (const Assignment& B : North.assignments) {
                    const std::vector<Slot> Slots = RelativeSlots(Turned, 10, 10, {});
                    if (A.id < B.id && Slots[A.slot].rank == Slots[B.slot].rank) assert(Slots[A.slot].lateral < Slots[B.slot].lateral);
                }
        }

        // Blocked slots are replaced from the spares behind; unique legal slots only.
        std::size_t Probes = 0;
        const auto South = [&Probes](Point P) { ++Probes; return P.y <= 1000.0; };
        for (const Shape Turned : {Shape::Wedge, Shape::EchelonRight}) {
            Probes = 0;
            const Plan Blocked = PlanGroupMove(Twenty, Target, Turned, South);
            assert(Probes == Blocked.candidatesChecked && Probes <= 40);
            assert(std::all_of(Blocked.assignments.begin(), Blocked.assignments.end(),
                [](const Assignment& A) { return A.slot == kNoSlot || A.destination.y <= 1000.0; }));
            assert(MinimumSeparation(Blocked) >= 150.0 - 1e-9);
        }
        Layout BadEchelon; BadEchelon.spacingCm = -1.0;
        assert(PlanGroupMove(Twenty, Target, Shape::EchelonLeft, Always, BadEchelon).unplaced == 20);
    }

    // FORM-FD right-drag facing and frontage: the drag's midpoint is the anchor, the formation
    // faces the right-hand perpendicular of the drag (so the drag runs from its left flank to its
    // right flank) and the drag length sets the files per rank of the shapes that have files.
    {
        using namespace politika::formation;
        const auto Always = [](Point) { return true; };
        const auto Near = [](Point a, Point b, double tolerance = 1e-9) {
            return std::abs(a.x - b.x) < tolerance && std::abs(a.y - b.y) < tolerance;
        };
        const auto Same = [](const DragFacing& a, const DragFacing& b) {
            return a.valid == b.valid && a.anchor.x == b.anchor.x && a.anchor.y == b.anchor.y && a.facing.x == b.facing.x
                && a.facing.y == b.facing.y && a.lengthCm == b.lengthCm && a.frontage == b.frontage;
        };
        const double Nan = std::numeric_limits<double>::quiet_NaN();
        const double Inf = std::numeric_limits<double>::infinity();
        static_assert(HasFrontage(Shape::Line) && HasFrontage(Shape::Column) && HasFrontage(Shape::Dispersed)
            && HasFrontage(Shape::EchelonLeft) && HasFrontage(Shape::EchelonRight) && !HasFrontage(Shape::Wedge)
            && !HasFrontage(Shape::Perimeter) && !HasFrontage(static_cast<Shape>(7)));
        static_assert(Frontage(Shape::Line, 20, {}, 4) == 4 && Frontage(Shape::Column, 20, {}, 5) == 5
            && Frontage(Shape::Line, 8, {}, 50) == 8 && Frontage(Shape::Line, 20, {}, 0) == 10 && Frontage(Shape::Dispersed, 20, {}, 1) == 1);

        // Unit facings within 1e-3 on the squared length; the zero sentinel is not a facing.
        assert(IsUnitFacing({1.0, 0.0}) && IsUnitFacing({0.6, -0.8}) && IsUnitFacing({1.0004, 0.0}));
        assert(!IsUnitFacing({0.0, 0.0}) && !IsUnitFacing({1.001, 0.0}) && !IsUnitFacing({0.5, 0.0})
            && !IsUnitFacing({Nan, 1.0}) && !IsUnitFacing({Inf, 0.0}));

        // Left to right along +x: facing (0, -1), whose right is +x; reversed, the same midpoint
        // and width with the opposite facing.
        const DragFacing East = DragToFacing({1000.0, 1000.0}, {1600.0, 1000.0}, 150.0, 20);
        assert(East.valid && Near(East.anchor, {1300.0, 1000.0}) && Near(East.facing, {0.0, -1.0}) && East.frontage == 5);
        assert(Near(RightOf(East.facing), {1.0, 0.0}) && East.lengthCm == 600.0);
        const DragFacing West = DragToFacing({1600.0, 1000.0}, {1000.0, 1000.0}, 150.0, 20);
        assert(West.valid && Near(West.anchor, East.anchor) && West.frontage == East.frontage
            && Near(West.facing, {-East.facing.x, -East.facing.y}));
        const DragFacing Up = DragToFacing({0.0, 0.0}, {0.0, 300.0}, 150.0, 20);
        assert(Up.valid && Near(Up.facing, {1.0, 0.0}) && Near(Up.anchor, {0.0, 150.0}) && Up.frontage == 3);
        const DragFacing Diagonal = DragToFacing({0.0, 0.0}, {300.0, 300.0}, 150.0, 20);
        assert(Diagonal.valid && Near(Diagonal.facing, {kHalfSqrt2, -kHalfSqrt2}, 1e-12) && Diagonal.frontage == 4);
        assert(IsUnitFacing(Diagonal.facing) && Near(RightOf(Diagonal.facing), {kHalfSqrt2, kHalfSqrt2}, 1e-12));
        // Shorter than one spacing: facing and midpoint, default width.
        const DragFacing Short = DragToFacing({0.0, 0.0}, {100.0, 0.0}, 150.0, 20);
        assert(Short.valid && Near(Short.facing, {0.0, -1.0}) && Near(Short.anchor, {50.0, 0.0}) && Short.frontage == 0);
        // Exactly one spacing: two files, whose span equals the drag.
        assert(DragToFacing({0.0, 0.0}, {150.0, 0.0}, 150.0, 20).frontage == 2);
        assert(DragToFacing({0.0, 0.0}, {149.999, 0.0}, 150.0, 20).frontage == 0);
        // Rounding: halves round up, deterministically.
        assert(DragToFacing({0.0, 0.0}, {225.0, 0.0}, 150.0, 20).frontage == 3);
        assert(DragToFacing({0.0, 0.0}, {224.999, 0.0}, 150.0, 20).frontage == 2);
        assert(DragToFacing({0.0, 0.0}, {375.0, 0.0}, 150.0, 20).frontage == 4);
        // Clamped to the members, without overflow on a huge drag.
        assert(DragToFacing({0.0, 0.0}, {3000.0, 0.0}, 150.0, 8).frontage == 8);
        assert(DragToFacing({0.0, 0.0}, {1e300, 0.0}, 150.0, kMaxFormationMembers).frontage == kMaxFormationMembers);
        assert(DragToFacing({0.0, 0.0}, {600.0, 0.0}, 150.0, 1).frontage == 1);
        const DragFacing Nobody = DragToFacing({0.0, 0.0}, {600.0, 0.0}, 150.0, 0);
        assert(Nobody.valid && Nobody.frontage == 0);
        // No direction: never a malformed facing.
        for (const DragFacing& Bad : {DragToFacing({5.0, 5.0}, {5.0, 5.0}, 150.0, 20), DragToFacing({5.0, 5.0}, {5.5, 5.0}, 150.0, 20),
                 DragToFacing({Nan, 0.0}, {300.0, 0.0}, 150.0, 20), DragToFacing({0.0, 0.0}, {Inf, 0.0}, 150.0, 20),
                 DragToFacing({0.0, 0.0}, {300.0, Nan}, 150.0, 20), DragToFacing({0.0, 0.0}, {300.0, 0.0}, 0.0, 20),
                 DragToFacing({0.0, 0.0}, {300.0, 0.0}, -150.0, 20), DragToFacing({0.0, 0.0}, {300.0, 0.0}, Inf, 20),
                 DragToFacing({0.0, 0.0}, {300.0, 0.0}, Nan, 20), DragToFacing({-1e308, 0.0}, {1e308, 0.0}, 150.0, 20)})
            assert(!Bad.valid && Bad.facing.x == 0.0 && Bad.facing.y == 0.0 && Bad.frontage == 0);
        // Repeatable to the bit.
        assert(Same(DragToFacing({12.3, 45.6}, {789.1, -234.5}, 150.0, 20), DragToFacing({12.3, 45.6}, {789.1, -234.5}, 150.0, 20)));
        // Per shape: Dispersed counts its own 450 cm spacing; Wedge and Perimeter keep the facing only.
        assert(ShapeDrag({0.0, 0.0}, {900.0, 0.0}, Shape::Dispersed, 20).frontage == 3);
        assert(ShapeDrag({0.0, 0.0}, {900.0, 0.0}, Shape::Line, 20).frontage == 7);
        assert(ShapeDrag({0.0, 0.0}, {900.0, 0.0}, Shape::EchelonRight, 20).frontage == 7);
        for (const Shape Fixed : {Shape::Wedge, Shape::Perimeter}) {
            const DragFacing Facing = ShapeDrag({0.0, 0.0}, {900.0, 0.0}, Fixed, 20);
            assert(Facing.valid && Facing.frontage == 0 && Near(Facing.facing, {0.0, -1.0}));
        }

        // Zero override is the default for every shape.
        for (std::size_t Index = 0; Index < kShapeCount; ++Index) {
            const Shape Each = static_cast<Shape>(Index);
            const std::vector<Slot> Default = RelativeSlots(Each, 20, 20, {});
            const std::vector<Slot> Zero = RelativeSlots(Each, 20, 20, {}, 0);
            assert(Default.size() == Zero.size());
            for (std::size_t Slot = 0; Slot < Default.size(); ++Slot)
                assert(Default[Slot].lateral == Zero[Slot].lateral && Default[Slot].forward == Zero[Slot].forward
                    && Default[Slot].rank == Zero[Slot].rank);
        }
        const auto Extent = [](const std::vector<Slot>& Slots, std::size_t Count) {
            double MinL = Slots[0].lateral, MaxL = MinL, MinF = Slots[0].forward, MaxF = MinF;
            for (std::size_t Index = 0; Index < Count; ++Index) {
                MinL = std::min(MinL, Slots[Index].lateral); MaxL = std::max(MaxL, Slots[Index].lateral);
                MinF = std::min(MinF, Slots[Index].forward); MaxF = std::max(MaxF, Slots[Index].forward);
            }
            return Point{MaxL - MinL, MaxF - MinF};
        };
        // Line of twenty at four files: five full ranks, (4 - 1) x 150 across, 4 x 150 deep, centred.
        const std::vector<Slot> Line4 = RelativeSlots(Shape::Line, 20, 20, {}, 4);
        assert(Line4.size() == 40 && Near(Extent(Line4, 20), {450.0, 600.0}) && Line4[19].rank == 4 && Line4[20].rank == 5);
        assert(Line4[0].lateral == -225.0 && Line4[3].lateral == 225.0 && Line4[0].forward == 300.0 && Line4[19].forward == -300.0);
        // Spares continue four-file ranks behind.
        assert(Line4[20].lateral == -225.0 && Line4[23].lateral == 225.0 && Line4[24].rank == 6);
        // A partial last rank stays centred: ten at four files are ranks of 4, 4 and 2.
        const std::vector<Slot> Line10 = RelativeSlots(Shape::Line, 10, 0, {}, 4);
        assert(Line10[8].rank == 2 && Line10[8].lateral == -75.0 && Line10[9].lateral == 75.0 && Line10[0].lateral == -225.0);
        const std::vector<Slot> Column5 = RelativeSlots(Shape::Column, 20, 0, {}, 5);
        assert(Near(Extent(Column5, 20), {600.0, 450.0}) && Column5[19].rank == 3);
        const std::vector<Slot> Spread3 = RelativeSlots(Shape::Dispersed, 8, 0, {}, 3);
        assert(Spread3[7].rank == 2 && Spread3[6].rank == 2 && Spread3[5].rank == 1);
        // Echelon: the four-file Line turned 45 degrees; the held-back flank is the right (Echelon
        // Right) or left (Echelon Left) end of the front rank, 3 x 150 / sqrt(2) behind the other.
        const std::vector<Slot> Line12 = RelativeSlots(Shape::Line, 12, 12, {}, 4);
        const std::vector<Slot> Right12 = RelativeSlots(Shape::EchelonRight, 12, 12, {}, 4);
        const std::vector<Slot> Left12 = RelativeSlots(Shape::EchelonLeft, 12, 12, {}, 4);
        assert(Right12.size() == 24 && Left12.size() == 24);
        for (std::size_t Index = 0; Index < 24; ++Index) {
            const Slot TurnedRight = EchelonTurn(Line12[Index], 1);
            const Slot TurnedLeft = EchelonTurn(Line12[Index], -1);
            assert(Right12[Index].lateral == TurnedRight.lateral && Right12[Index].forward == TurnedRight.forward
                && Left12[Index].lateral == TurnedLeft.lateral && Left12[Index].forward == TurnedLeft.forward
                && Right12[Index].rank == Line12[Index].rank);
        }
        assert(std::abs(Right12[0].forward - Right12[3].forward - 450.0 * kHalfSqrt2) < 1e-9);
        assert(std::abs(Left12[3].forward - Left12[0].forward - 450.0 * kHalfSqrt2) < 1e-9);
        // Wedge and Perimeter ignore any frontage.
        for (const Shape Fixed : {Shape::Wedge, Shape::Perimeter}) {
            const std::vector<Slot> Plain = RelativeSlots(Fixed, 20, 20, {});
            const std::vector<Slot> Wide = RelativeSlots(Fixed, 20, 20, {}, 4);
            assert(Plain.size() == Wide.size());
            for (std::size_t Index = 0; Index < Plain.size(); ++Index)
                assert(Plain[Index].lateral == Wide[Index].lateral && Plain[Index].forward == Wide[Index].forward);
        }

        // Planned moves with an explicit facing and frontage.
        std::vector<Member> Twenty;
        for (EntityId Id = 1; Id <= 20; ++Id)
            Twenty.push_back({Id, {500.0 + static_cast<double>(Id % 5) * 100.0, 500.0 + static_cast<double>(Id / 5) * 100.0}});
        const std::vector<Member> Reversed(Twenty.rbegin(), Twenty.rend());
        const Point Target{4000.0, 600.0};
        // Facing north (-y here) although the group travels east: the drag decides.
        const Point North{0.0, -1.0};
        const Plan Wide = PlanGroupMove(Twenty, Target, Shape::Line, Always, {}, {1.0, 0.0}, North, 4);
        assert(Wide.fixedFacing && Near(Wide.facing, North) && Wide.requestedFrontage == 4 && Wide.frontage == 4);
        assert(Wide.unplaced == 0 && Wide.candidatesChecked == 20 && Wide.candidatesRejected == 0);
        assert(std::abs(MinimumSeparation(Wide) - 150.0) < 1e-9);
        double MinAcross = Inf, MaxAcross = -Inf, MinAlong = Inf, MaxAlong = -Inf;
        Point Sum{0.0, 0.0};
        for (const Assignment& A : Wide.assignments) {
            const Point Offset{A.destination.x - Target.x, A.destination.y - Target.y};
            const double Across = Offset.x * RightOf(North).x + Offset.y * RightOf(North).y;
            const double Along = Offset.x * North.x + Offset.y * North.y;
            MinAcross = std::min(MinAcross, Across); MaxAcross = std::max(MaxAcross, Across);
            MinAlong = std::min(MinAlong, Along); MaxAlong = std::max(MaxAlong, Along);
            Sum = {Sum.x + A.destination.x, Sum.y + A.destination.y};
        }
        assert(std::abs(MaxAcross - MinAcross - 450.0) < 1e-9 && std::abs(MaxAlong - MinAlong - 600.0) < 1e-9);
        assert(Near({Sum.x / 20.0, Sum.y / 20.0}, Target, 1e-9));
        // The same plan from any input order.
        const Plan WideAgain = PlanGroupMove(Reversed, Target, Shape::Line, Always, {}, {1.0, 0.0}, North, 4);
        for (std::size_t Index = 0; Index < 20; ++Index)
            assert(WideAgain.assignments[Index].id == Wide.assignments[Index].id
                && WideAgain.assignments[Index].destination.x == Wide.assignments[Index].destination.x
                && WideAgain.assignments[Index].destination.y == Wide.assignments[Index].destination.y);
        // Zero frontage is the default width; an over-wide frontage is clamped to the members.
        const Plan Default = PlanGroupMove(Twenty, Target, Shape::Line, Always, {}, {1.0, 0.0}, North);
        const Plan ZeroWidth = PlanGroupMove(Twenty, Target, Shape::Line, Always, {}, {1.0, 0.0}, North, 0);
        assert(Default.frontage == 10 && ZeroWidth.frontage == 10 && !PlanGroupMove(Twenty, Target, Shape::Line, Always).fixedFacing);
        for (std::size_t Index = 0; Index < 20; ++Index)
            assert(Near(Default.assignments[Index].destination, ZeroWidth.assignments[Index].destination, 0.0 + 1e-12));
        assert(PlanGroupMove(Twenty, Target, Shape::Line, Always, {}, {1.0, 0.0}, North, 500).frontage == 20);
        // Reversing the drag mirrors the block through the anchor (full ranks).
        const Plan Mirrored = PlanGroupMove(Twenty, Target, Shape::Line, Always, {}, {1.0, 0.0}, {0.0, 1.0}, 4);
        for (const Assignment& A : Wide.assignments) {
            const Point Reflected{2.0 * Target.x - A.destination.x, 2.0 * Target.y - A.destination.y};
            assert(std::any_of(Mirrored.assignments.begin(), Mirrored.assignments.end(),
                [&](const Assignment& B) { return Near(B.destination, Reflected, 1e-9); }));
        }
        // Every lattice shape and Echelon keeps its spacing and its legality budget.
        for (const Shape Each : {Shape::Line, Shape::Column, Shape::Dispersed, Shape::EchelonLeft, Shape::EchelonRight}) {
            std::size_t Probes = 0;
            const auto South = [&Probes](Point P) { ++Probes; return P.y <= 900.0; };
            const Plan Blocked = PlanGroupMove(Twenty, Target, Each, South, {}, {1.0, 0.0}, North, 3);
            assert(Probes == Blocked.candidatesChecked && Probes <= 40 && Blocked.frontage == 3);
            assert(MinimumSeparation(Blocked) >= ShapeSpacing(Each, {}) - 1e-9);
            const Plan Open = PlanGroupMove(Twenty, Target, Each, Always, {}, {1.0, 0.0}, North, 3);
            const Plan OpenAgain = PlanGroupMove(Reversed, Target, Each, Always, {}, {1.0, 0.0}, North, 3);
            assert(Open.unplaced == 0 && std::abs(MinimumSeparation(Open) - ShapeSpacing(Each, {})) < 1e-9);
            for (std::size_t Index = 0; Index < 20; ++Index)
                assert(OpenAgain.assignments[Index].id == Open.assignments[Index].id
                    && Near(OpenAgain.assignments[Index].destination, Open.assignments[Index].destination, 0.0 + 1e-12));
        }
        // Wedge and Perimeter: the dragged facing applies, the frontage never does.
        for (const Shape Fixed : {Shape::Wedge, Shape::Perimeter}) {
            const Plan Facing = PlanGroupMove(Twenty, Target, Fixed, Always, {}, {1.0, 0.0}, North, 0);
            const Plan Framed = PlanGroupMove(Twenty, Target, Fixed, Always, {}, {1.0, 0.0}, North, 4);
            assert(Near(Facing.facing, North) && Facing.frontage == 0 && Framed.frontage == 0 && Framed.requestedFrontage == 4);
            for (std::size_t Index = 0; Index < 20; ++Index)
                assert(Framed.assignments[Index].id == Facing.assignments[Index].id
                    && Framed.assignments[Index].slot == Facing.assignments[Index].slot
                    && Near(Framed.assignments[Index].destination, Facing.assignments[Index].destination, 0.0 + 1e-12));
        }

        // A formation stores a dragged facing and frontage, its regroup reuses them clamped to the
        // survivors, and a plain move resets both.
        const auto At = [](EntityId Id) { return Point{1000.0 + static_cast<double>(Id) * 10.0, 1000.0}; };
        std::vector<Entity> Eight;
        for (EntityId Id = 1; Id <= 8; ++Id) { Entity E{Id, 1}; Eight.push_back(E); }
        Registry Formations;
        const FormationId Id = Formations.Form(1, Eight, Shape::Line, 1).formed;
        assert(Id && !Formations.Find(Id)->explicitFacing && Formations.Find(Id)->frontage == 0);
        const Formation Before = *Formations.Find(Id);
        const auto Unchanged = [&](const Formation& Now) {
            return Now.hasDestination == Before.hasDestination && Now.explicitFacing == Before.explicitFacing
                && Now.frontage == Before.frontage && Now.anchor.x == Before.anchor.x && Now.anchor.y == Before.anchor.y
                && Now.facing.x == Before.facing.x && Now.facing.y == Before.facing.y
                && Now.FollowingCount() == Before.FollowingCount();
        };
        // Malformed overrides and a foreign participant are rejected without any change.
        assert(Formations.PlanMove(1, Id, {3000, 1000}, false, At, Always, {}, {0.5, 0.0}).rejection == FormationRejection::MalformedCommand);
        assert(Formations.PlanMove(1, Id, {3000, 1000}, false, At, Always, {}, {Nan, 1.0}).rejection == FormationRejection::MalformedCommand);
        assert(Formations.PlanMove(1, Id, {3000, 1000}, false, At, Always, {}, {0.0, 1.0}, kMaxFormationMembers + 1).rejection
            == FormationRejection::MalformedCommand);
        assert(Formations.PlanMove(1, Id, {3000, 1000}, false, At, Always, {}, {0.0, 0.0}, 3).rejection == FormationRejection::MalformedCommand);
        assert(Formations.PlanMove(2, Id, {3000, 1000}, false, At, Always, {}, {0.0, 1.0}, 3).rejection == FormationRejection::UnknownFormation);
        assert(Unchanged(*Formations.Find(Id)));
        const MovePlan Dragged = Formations.PlanMove(1, Id, {3000, 1000}, false, At, Always, {}, {0.0, 1.0}, 4);
        assert(Dragged.rejection == FormationRejection::None && Dragged.orders.size() == 8 && Dragged.plan.frontage == 4
            && Dragged.plan.fixedFacing && Near(Dragged.plan.facing, {0.0, 1.0}));
        const Formation* Stored = Formations.Find(Id);
        assert(Stored->explicitFacing && Stored->frontage == 4 && Near(Stored->facing, {0.0, 1.0}) && Near(Stored->anchor, {3000, 1000}));
        const Formation AfterDrag = *Stored;
        // A rejected request after an accepted drag leaves the stored drag in place.
        assert(Formations.PlanMove(1, Id, {3000, 1000}, false, At, Always, {}, {2.0, 0.0}, 4).rejection == FormationRejection::MalformedCommand);
        assert(Stored->explicitFacing && Stored->frontage == AfterDrag.frontage && Near(Stored->facing, AfterDrag.facing));
        // Three losses: one regroup, same facing, four files for five survivors.
        (void)Formations.MemberLost(8, 100);
        (void)Formations.MemberLost(7, 101);
        (void)Formations.MemberLost(6, 102);
        const MovePlan Closed = Formations.PlanRegroup(Id, 140, At, Always);
        assert(Closed.rejection == FormationRejection::None && Closed.plan.assignments.size() == 5 && Stored->regroups == 1);
        assert(Closed.plan.fixedFacing && Near(Closed.plan.facing, {0.0, 1.0}) && Closed.plan.frontage == 4 && Closed.plan.requestedFrontage == 4);
        assert(Stored->explicitFacing && Stored->frontage == 4);
        // Two more losses: three survivors, so the dragged width is clamped to three files.
        (void)Formations.MemberLost(5, 200);
        (void)Formations.MemberLost(4, 201);
        const MovePlan Narrow = Formations.PlanRegroup(Id, 240, At, Always);
        assert(Narrow.plan.assignments.size() == 3 && Narrow.plan.frontage == 3 && Near(Narrow.plan.facing, {0.0, 1.0}) && Stored->regroups == 2);
        // A plain move faces the travel direction at the default width again.
        const MovePlan Plain = Formations.PlanMove(1, Id, {5000, 1000}, false, At, Always);
        assert(Plain.rejection == FormationRejection::None && !Plain.plan.fixedFacing && Near(Plain.plan.facing, {1.0, 0.0}));
        assert(!Stored->explicitFacing && Stored->frontage == 0 && Plain.plan.frontage == 3);
        // A wider-than-the-formation drag is stored at the member count.
        (void)Formations.PlanMove(1, Id, {5000, 1000}, false, At, Always, {}, {0.0, 1.0}, 20);
        assert(Stored->frontage == 3);
        // A Wedge formation keeps the dragged facing but never a frontage.
        std::vector<Entity> Others;
        for (EntityId Other = 11; Other <= 16; ++Other) { Entity E{Other, 1}; Others.push_back(E); }
        const FormationId Arrow = Formations.Form(1, Others, Shape::Wedge, 300).formed;
        const MovePlan Pointed = Formations.PlanMove(1, Arrow, {3000, 3000}, false, At, Always, {}, {-1.0, 0.0}, 4);
        assert(Pointed.rejection == FormationRejection::None && Near(Pointed.plan.facing, {-1.0, 0.0}) && Pointed.plan.frontage == 0);
        assert(Formations.Find(Arrow)->explicitFacing && Formations.Find(Arrow)->frontage == 0);
        // Re-forming starts clean: no inherited drag.
        const FormationId Again = Formations.Form(1, std::span<const Entity>(Others.data(), 3), Shape::Line, 301).formed;
        assert(Again && !Formations.Find(Again)->explicitFacing && Formations.Find(Again)->frontage == 0
            && !Formations.Find(Again)->hasDestination);
        (void)Formations.Clear();
        assert(Formations.Empty());

        // Wire shape (network_protocol 9): facing and frontage only on Move/AttackMove.
        const EntityId Two[] = {4, 7};
        const EntityId Dup[] = {4, 4};
        const auto Move = [](double FacingX, double FacingY, std::uint32_t Files, std::uint8_t Op = 2) {
            return CheckIntentShape({Op, 3, 0, {}, 10.0, 20.0, 0, FacingX, FacingY, Files});
        };
        assert(Move(0.0, 0.0, 0) == FormationRejection::None);
        assert(Move(0.0, 1.0, 0) == FormationRejection::None && Move(1.0, 0.0, 0) == FormationRejection::None);
        assert(Move(0.6, -0.8, 4) == FormationRejection::None && Move(kHalfSqrt2, kHalfSqrt2, 64) == FormationRejection::None);
        assert(Move(1.0004, 0.0, 1) == FormationRejection::None && Move(0.0, -1.0, 7, 3) == FormationRejection::None);
        assert(Move(kHalfSqrt2, kHalfSqrt2, 65) == FormationRejection::MalformedCommand);
        assert(Move(0.0, 0.0, 3) == FormationRejection::MalformedCommand);
        assert(Move(0.5, 0.0, 0) == FormationRejection::MalformedCommand && Move(0.0, 0.3, 0) == FormationRejection::MalformedCommand);
        assert(Move(2.0, 0.0, 0) == FormationRejection::MalformedCommand && Move(1.001, 0.0, 0) == FormationRejection::MalformedCommand);
        assert(Move(Nan, 0.0, 0) == FormationRejection::MalformedCommand && Move(0.0, Inf, 0) == FormationRejection::MalformedCommand);
        assert(Move(Nan, Nan, 0) == FormationRejection::MalformedCommand);
        assert(CheckIntentShape({0, 0, 0, Two, 0.0, 0.0, 0, 0.0, 1.0, 0}) == FormationRejection::MalformedCommand);
        assert(CheckIntentShape({0, 0, 0, Two, 0.0, 0.0, 0, 0.0, 0.0, 2}) == FormationRejection::MalformedCommand);
        assert(CheckIntentShape({1, 0, 0, Two, 0.0, 0.0, 0, 1.0, 0.0, 0}) == FormationRejection::MalformedCommand);
        assert(CheckIntentShape({4, 3, 0, {}, 0.0, 0.0, 0, 0.0, 1.0, 0}) == FormationRejection::MalformedCommand);
        assert(CheckIntentShape({4, 3, 0, {}, 0.0, 0.0, 0, 0.0, 0.0, 1}) == FormationRejection::MalformedCommand);
        assert(CheckIntentShape({5, 3, 0, {}, 0.0, 0.0, 1, 0.0, 1.0, 0}) == FormationRejection::MalformedCommand);
        assert(CheckIntentShape({6, 3, 0, {}, 0.0, 0.0, 1, 0.0, 0.0, 1}) == FormationRejection::MalformedCommand);
        assert(CheckIntentShape({4, 3, 0, {}, 0.0, 0.0, 0, Nan, 0.0, 0}) == FormationRejection::MalformedCommand);
        // The existing rules still hold with the new fields zero.
        assert(CheckIntentShape({0, 0, 0, Two, 0.0, 0.0, 0, 0.0, 0.0, 0}) == FormationRejection::None);
        assert(CheckIntentShape({0, 0, 0, Dup, 0.0, 0.0, 0, 0.0, 0.0, 0}) == FormationRejection::DuplicateMember);
        assert(CheckIntentShape({2, 0, 0, {}, 10.0, 20.0, 0, 0.0, 1.0, 0}) == FormationRejection::MalformedCommand);
        assert(CheckIntentShape({2, 3, 1, {}, 10.0, 20.0, 0, 0.0, 1.0, 0}) == FormationRejection::MalformedCommand);
        assert(CheckIntentShape({2, 3, 0, Two, 10.0, 20.0, 0, 0.0, 1.0, 0}) == FormationRejection::MalformedCommand);
        assert(CheckIntentShape({2, 3, 0, {}, 10.0, 20.0, 1, 0.0, 1.0, 0}) == FormationRejection::MalformedCommand);
    }

    // FORM-F3 formation instances: explicit Form/Release, one formation per unit, member loss,
    // independent overrides and one bounded regroup per loss window.
    {
        using namespace politika::formation;
        const auto Always = [](Point) { return true; };
        const auto Near = [](Point a, Point b) { return std::abs(a.x - b.x) < 1e-9 && std::abs(a.y - b.y) < 1e-9; };
        const auto Unit = [](EntityId Id, std::uint32_t Owner) { Entity E{Id, Owner}; return E; };
        std::vector<Entity> Eight;
        for (EntityId Id = 1; Id <= 8; ++Id) Eight.push_back(Unit(Id, 1));
        const auto At = [](EntityId Id) { return Point{1000.0 + static_cast<double>(Id) * 10.0, 1000.0}; };
        static_assert(OpName(FormationOp::AttackMove) == "attack_move" && StateName(FormationState::Degraded) == "degraded");
        static_assert(kFormationRejectionCount == 16 && !FormationId{} && FormationId{3});

        // Payload shape checks fail closed before any authoritative state is read.
        const EntityId Two[] = {4, 7};
        const EntityId Dup[] = {4, 4};
        const EntityId ZeroId[] = {0, 4};
        const EntityId One[] = {4};
        assert(CheckIntentShape({0, 0, 0, Two}) == FormationRejection::None);
        assert(CheckIntentShape({0, 0, 1, Two}) == FormationRejection::None);
        assert(CheckIntentShape({0, 0, 2, Two}) == FormationRejection::None);
        assert(CheckIntentShape({0, 0, 3, Two}) == FormationRejection::None);
        assert(CheckIntentShape({0, 0, 4, Two}) == FormationRejection::None);
        assert(CheckIntentShape({0, 0, 5, Two}) == FormationRejection::None);
        assert(CheckIntentShape({0, 0, 6, Two}) == FormationRejection::None);
        assert(CheckIntentShape({0, 0, 7, Two}) == FormationRejection::MalformedCommand);
        assert(CheckIntentShape({0, 5, 0, Two}) == FormationRejection::MalformedCommand);
        assert(CheckIntentShape({0, 0, 0, Two, 1.0, 0.0}) == FormationRejection::MalformedCommand);
        assert(CheckIntentShape({0, 0, 0, Dup}) == FormationRejection::DuplicateMember);
        assert(CheckIntentShape({0, 0, 0, ZeroId}) == FormationRejection::MalformedCommand);
        assert(CheckIntentShape({0, 0, 0, One}) == FormationRejection::TooFewMembers);
        assert(CheckIntentShape({0, 0, 0, {}}) == FormationRejection::TooFewMembers);
        std::vector<EntityId> SixtyFive(65);
        for (std::size_t Index = 0; Index < SixtyFive.size(); ++Index) SixtyFive[Index] = Index + 1;
        assert(CheckIntentShape({0, 0, 0, SixtyFive}) == FormationRejection::TooManyMembers);
        assert(CheckIntentShape({1, 0, 0, One}) == FormationRejection::None);
        assert(CheckIntentShape({1, 0, 1, One}) == FormationRejection::MalformedCommand);
        assert(CheckIntentShape({1, 0, 0, {}}) == FormationRejection::MalformedCommand);
        assert(CheckIntentShape({2, 3, 0, {}, 10.0, 20.0}) == FormationRejection::None);
        assert(CheckIntentShape({3, 3, 0, {}, 10.0, 20.0}) == FormationRejection::None);
        assert(CheckIntentShape({2, 0, 0, {}, 10.0, 20.0}) == FormationRejection::MalformedCommand);
        assert(CheckIntentShape({2, 3, 0, Two, 10.0, 20.0}) == FormationRejection::MalformedCommand);
        assert(CheckIntentShape({2, 3, 0, {}, std::nan(""), 20.0}) == FormationRejection::MalformedCommand);
        assert(CheckIntentShape({4, 3, 0, {}}) == FormationRejection::None);
        assert(CheckIntentShape({4, 3, 0, {}, 1.0, 0.0}) == FormationRejection::MalformedCommand);
        assert(CheckIntentShape({7, 3, 0, {}}) == FormationRejection::MalformedCommand);
        // Formation ops carry no stance/adherence value.
        assert(CheckIntentShape({4, 3, 0, {}, 0.0, 0.0, 1}) == FormationRejection::MalformedCommand);

        // Form validates every member against authoritative state and changes nothing on rejection.
        Registry Formations;
        assert(Formations.Form(0, Eight, Shape::Line, 1).rejection == FormationRejection::Unauthorized);
        assert(Formations.Form(1, std::span<const Entity>(Eight.data(), 1), Shape::Line, 1).rejection == FormationRejection::TooFewMembers);
        std::vector<Entity> Bad = Eight;
        Bad[3].owner_player = 2;
        assert(Formations.Form(1, Bad, Shape::Line, 1).rejection == FormationRejection::MemberUnavailable);
        Bad = Eight; Bad[2].alive = false;
        assert(Formations.Form(1, Bad, Shape::Line, 1).rejection == FormationRejection::MemberUnavailable);
        Bad = Eight; Bad[2].movable = false;
        assert(Formations.Form(1, Bad, Shape::Line, 1).rejection == FormationRejection::MemberUnavailable);
        Bad = Eight; Bad[5].id = 2;
        assert(Formations.Form(1, Bad, Shape::Line, 1).rejection == FormationRejection::DuplicateMember);
        // DISC-D1: civilians cannot serve in formations.
        Bad = Eight; Bad[4].military = false;
        assert(Formations.Form(1, Bad, Shape::Line, 1).rejection == FormationRejection::NotMilitary);
        assert(Formations.Empty());
        const Change Formed = Formations.Form(1, Eight, Shape::Column, 1);
        assert(Formed.Accepted() && Formed.formed == FormationId{1} && Formed.dissolved.empty());
        const Formation* First = Formations.FindOwned(FormationId{1}, 1);
        assert(First && First->members.size() == 8 && First->shape == Shape::Column && First->formedMembers == 8);
        assert(!First->hasDestination && First->FollowingCount() == 0 && First->state == FormationState::Organized);
        assert(Formations.FormationOf(5) == FormationId{1} && !Formations.FormationOf(9));
        // Another participant cannot see or address it: unknown and foreign IDs look the same.
        assert(!Formations.FindOwned(FormationId{1}, 2) && !Formations.FindOwned(FormationId{9}, 1));
        assert(Formations.PlanMove(2, FormationId{1}, {3000, 1000}, false, At, Always).rejection == FormationRejection::UnknownFormation);
        assert(Formations.Stop(2, FormationId{1}) == FormationRejection::UnknownFormation);
        const EntityId Foreign[] = {3};
        assert(Formations.Release(2, Foreign, 1).rejection == FormationRejection::NotMember);

        // A formation move plans one slot per member with the stored shape; all follow it.
        assert(Formations.PlanMove(1, FormationId{1}, {std::nan(""), 0}, false, At, Always).rejection
            == FormationRejection::InvalidDestination);
        const MovePlan Moved = Formations.PlanMove(1, FormationId{1}, {3000, 1000}, false, At, Always);
        assert(Moved.rejection == FormationRejection::None && Moved.orders.size() == 8 && Moved.plan.shape == Shape::Column);
        assert(std::is_sorted(Moved.orders.begin(), Moved.orders.end(), [](const MemberOrder& A, const MemberOrder& B) { return A.id < B.id; }));
        assert(std::abs(MinimumSeparation(Moved.plan) - 150.0) < 1e-9);
        First = Formations.Find(FormationId{1});
        assert(First->hasDestination && First->FollowingCount() == 8 && Near(First->facing, {1.0, 0.0}));

        // FORM-07: an individual order for one member leaves the others following and keeps
        // membership; the member's own formation orders keep it following.
        assert(Formations.NoteOrderAdmitted(3, FormationId{1}) == false && First->Find(3)->following);
        assert(Formations.NoteOrderAdmitted(3, FormationId{}) == true);
        assert(!First->Find(3)->following && First->FollowingCount() == 7 && First->members.size() == 8);
        assert(Formations.NoteOrderAdmitted(3, FormationId{}) == false);
        assert(Formations.NoteOrderAdmitted(42, FormationId{}) == false);

        // FORM-08: two losses inside one window share one regroup, which keeps the anchor and
        // facing and moves only following members whose slot changed.
        const Change Lost = Formations.MemberLost(8, 100);
        assert(Lost.left.size() == 1 && Lost.left[0].reason == LeaveReason::Destroyed && Lost.dissolved.empty());
        assert(Lost.regroups.size() == 1 && Lost.regroups[0].dueStep == 100 + kRegroupDelaySteps);
        assert(First->state == FormationState::Degraded && First->losses == 1);
        const Change Lost2 = Formations.MemberLost(7, 110);
        assert(Lost2.regroups.empty() && First->regroupDueStep == 140);
        assert(Formations.DueRegroups(139).empty() && Formations.DueRegroups(140).size() == 1);
        const MovePlan Regroup = Formations.PlanRegroup(FormationId{1}, 140, At, Always);
        assert(Regroup.rejection == FormationRejection::None && Regroup.plan.assignments.size() == 5);
        assert(std::none_of(Regroup.orders.begin(), Regroup.orders.end(), [](const MemberOrder& O) { return O.id == 3; }));
        assert(!Regroup.orders.empty() && Regroup.orders.size() <= 5 && Near(Regroup.plan.facing, {1.0, 0.0}));
        assert(First->state == FormationState::Organized && First->regroups == 1 && First->regroupDueStep == 0);
        assert(Formations.DueRegroups(1000).empty());
        // Losing the independent member leaves the following set unchanged: the rate-limited
        // regroup re-plans the same slots and re-sends nothing.
        const Change Lost3 = Formations.MemberLost(3, 150);
        assert(Lost3.regroups.size() == 1 && Lost3.regroups[0].dueStep == 190);  // rate limit: >= last + 40
        const MovePlan Regroup2 = Formations.PlanRegroup(FormationId{1}, 190, At, Always);
        assert(Regroup2.plan.assignments.size() == 5 && Regroup2.orders.empty() && First->regroups == 2);
        // A regroup is only planned when one is due.
        assert(Formations.PlanRegroup(FormationId{1}, 200, At, Always).rejection == FormationRejection::UnknownFormation);

        // Losing all but one member dissolves the formation.
        for (EntityId Id : {1, 2, 4, 5}) (void)Formations.MemberLost(Id, 300);
        assert(Formations.Find(FormationId{1}) == nullptr && Formations.Empty());
        assert(Formations.MemberLost(6, 301).rejection == FormationRejection::NotMember);

        // One formation per unit: forming a subset moves it and dissolves an emptied formation;
        // IDs never repeat. Release removes listed members; a formation left with one dissolves.
        const Change A = Formations.Form(1, Eight, Shape::Line, 400);
        assert(A.formed == FormationId{2});
        const Change B = Formations.Form(1, std::span<const Entity>(Eight.data(), 3), Shape::Line, 401);
        assert(B.formed == FormationId{3} && B.left.size() == 3 && B.dissolved.empty());
        assert(Formations.Find(FormationId{2})->members.size() == 5 && Formations.FormationOf(1) == FormationId{3});
        assert(B.regroups.empty());  // formation 2 had no destination
        const Change C = Formations.Form(1, std::span<const Entity>(Eight.data() + 3, 5), Shape::Column, 402);
        assert(C.formed == FormationId{4} && C.dissolved.size() == 1 && C.dissolved[0].reason == DissolveReason::Reformed);
        assert(!Formations.Find(FormationId{2}) && Formations.All().size() == 2);
        const EntityId ReleaseTwo[] = {1, 2, 99};
        const Change Released = Formations.Release(1, ReleaseTwo, 403);
        assert(Released.Accepted() && Released.left.size() == 2 && Released.dissolved.size() == 1
            && Released.dissolved[0].formation == FormationId{3} && Released.dissolved[0].reason == DissolveReason::Released);
        assert(!Formations.FormationOf(3) && Formations.FormationOf(4) == FormationId{4});

        // Stop forgets the destination: following ends, and losses schedule no regroup.
        (void)Formations.PlanMove(1, FormationId{4}, {3000, 1000}, true, At, Always);
        assert(Formations.Find(FormationId{4})->attackMove && Formations.Find(FormationId{4})->FollowingCount() == 5);
        assert(Formations.Stop(1, FormationId{4}) == FormationRejection::None);
        assert(Formations.Find(FormationId{4})->FollowingCount() == 0);
        assert(Formations.NoteOrderAdmitted(5, FormationId{4}) == false && !Formations.Find(FormationId{4})->Find(5)->following);
        assert(Formations.MemberLost(5, 500).regroups.empty());
        // DISC-D3: the server passes a Discipline-scaled delay per loss; it also sets the spacing
        // after the previous regroup.
        (void)Formations.PlanMove(1, FormationId{4}, {3000, 1000}, false, At, Always);
        const Change Disciplined = Formations.MemberLost(6, 600, LeaveReason::Destroyed, 31);
        assert(Disciplined.regroups.size() == 1 && Disciplined.regroups[0].dueStep == 631);
        (void)Formations.PlanRegroup(FormationId{4}, 631, At, Always);
        const Change Veteran = Formations.MemberLost(4, 640, LeaveReason::Destroyed, 25);
        assert(Veteran.regroups.size() == 1 && Veteran.regroups[0].dueStep == 665);
        (void)Formations.PlanRegroup(FormationId{4}, 665, At, Always);

        // Per-participant limit, counted after formations this Form would empty.
        Registry Limited;
        std::vector<Entity> Many;
        for (EntityId Id = 1; Id <= 22; ++Id) Many.push_back(Unit(Id, 1));
        for (std::size_t Index = 0; Index < kMaxFormationsPerParticipant; ++Index)
            assert(Limited.Form(1, std::span<const Entity>(Many.data() + Index * 2, 2), Shape::Line, 1).Accepted());
        assert(Limited.CountOwned(1) == kMaxFormationsPerParticipant);
        assert(Limited.Form(1, std::span<const Entity>(Many.data() + 20, 2), Shape::Line, 1).rejection == FormationRejection::FormationLimit);
        assert(Limited.Form(1, std::span<const Entity>(Many.data(), 2), Shape::Column, 1).Accepted());  // re-forms formation 1's pair
        std::vector<Entity> OtherOwner{Unit(30, 2), Unit(31, 2)};
        assert(Limited.Form(2, OtherOwner, Shape::Line, 1).Accepted());
        // Clear retires every formation exactly once; IDs keep increasing afterwards.
        const Change Cleared = Limited.Clear();
        assert(Cleared.dissolved.size() == kMaxFormationsPerParticipant + 1 && Limited.Empty() && Limited.Clear().dissolved.empty());
        assert(Limited.Form(1, std::span<const Entity>(Many.data(), 2), Shape::Line, 2).formed == FormationId{13});
    }

    // FORM-F4 combat stances and formation adherence: idle behaviour per stance, precedence,
    // overrides that keep a unit's stance across Form/Release, leash geometry, and Free
    // formations that keep membership but stop closing their ranks.
    {
        using namespace politika::formation;
        using namespace politika::stance;
        static_assert(kDefaultStance == CombatStance::HoldGround && static_cast<int>(CombatStance::HoldGround) == 0);
        static_assert(kDefaultAdherence == Adherence::Flexible && static_cast<int>(Adherence::Flexible) == 0);
        static_assert(StanceName(CombatStance::HoldFire) == "hold_fire" && AdherenceName(Adherence::Maintain) == "maintain");
        static_assert(OpName(FormationOp::SetStance) == "set_stance" && OpName(FormationOp::SetAdherence) == "set_adherence");
        // The stance key visits every stance once and wraps; adherence likewise.
        static_assert(NextStance(NextStance(NextStance(NextStance(CombatStance::HoldGround)))) == CombatStance::HoldGround);
        static_assert(NextStance(CombatStance::HoldGround) == CombatStance::Guard
            && NextStance(CombatStance::Guard) == CombatStance::Aggressive
            && NextStance(CombatStance::Aggressive) == CombatStance::HoldFire);
        static_assert(NextAdherence(NextAdherence(NextAdherence(Adherence::Flexible))) == Adherence::Flexible);
        static_assert(IsValidStance(3) && !IsValidStance(4) && IsValidAdherence(2) && !IsValidAdherence(3));

        // Precedence: unit override > formation stance > default (FORM-10).
        static_assert(Effective(std::nullopt, std::nullopt) == CombatStance::HoldGround);
        static_assert(Effective(std::nullopt, CombatStance::Guard) == CombatStance::Guard);
        static_assert(Effective(CombatStance::HoldFire, CombatStance::Guard) == CombatStance::HoldFire);
        static_assert(!OverrideFor(CombatStance::Guard, CombatStance::Guard).has_value());
        static_assert(*OverrideFor(CombatStance::HoldFire, CombatStance::Guard) == CombatStance::HoldFire);

        // Idle behaviour: Hold Ground is the pre-stance behaviour (fire in range, never move).
        constexpr IdleBehavior Ground = BehaviorFor(CombatStance::HoldGround, std::nullopt);
        static_assert(Ground.acquire && !Ground.chase && Ground.leashCm == 0.0 && !Ground.returnToPost);
        constexpr IdleBehavior Fire = BehaviorFor(CombatStance::HoldFire, Adherence::Flexible);
        static_assert(!Fire.acquire && !Fire.chase && !Fire.returnToPost);
        constexpr IdleBehavior Guarding = BehaviorFor(CombatStance::Guard, std::nullopt);
        static_assert(Guarding.acquire && Guarding.chase && Guarding.leashCm == kGuardLeashCm && Guarding.returnToPost
            && Guarding.acquireRangeCm == 0.0);
        constexpr IdleBehavior Pursuing = BehaviorFor(CombatStance::Aggressive, Adherence::Flexible);
        static_assert(Pursuing.leashCm == kAggressiveLeashCm && Pursuing.acquireRangeCm == kAggressiveLeashCm && Pursuing.returnToPost);
        // Adherence: Maintain halves the leash; Free chases but does not walk back.
        static_assert(BehaviorFor(CombatStance::Guard, Adherence::Maintain).leashCm == kGuardLeashCm * kMaintainLeashScale);
        static_assert(BehaviorFor(CombatStance::Guard, Adherence::Free).chase && !BehaviorFor(CombatStance::Guard, Adherence::Free).returnToPost);
        static_assert(BehaviorFor(CombatStance::HoldGround, Adherence::Maintain).leashCm == 0.0);

        // Leash geometry: engage only what can be fired at from inside the leash; clamp the chase.
        assert(WithinReach({0, 0}, {1200, 0}, 600.0, 650.0) && !WithinReach({0, 0}, {1300, 0}, 600.0, 650.0));
        assert(!WithinReach({0, 0}, {10, 0}, 0.0, 650.0) && !WithinReach({0, 0}, {std::nan(""), 0}, 600.0, 650.0));
        const Point Clamped = ClampToLeash({100, 100}, {100, 1100}, 600.0);
        assert(std::abs(Clamped.x - 100.0) < 1e-9 && std::abs(Clamped.y - 700.0) < 1e-9);
        const Point Inside = ClampToLeash({0, 0}, {300, 400}, 600.0);
        assert(Inside.x == 300.0 && Inside.y == 400.0);

        // Intent shapes: a formation or a unit list, never both; values bounded; no position.
        const EntityId Three[] = {4, 7, 9};
        const EntityId Dup[] = {4, 4};
        const EntityId ZeroId[] = {0, 4};
        assert(CheckIntentShape({5, 3, 0, {}, 0.0, 0.0, 3}) == FormationRejection::None);
        assert(CheckIntentShape({5, 0, 0, Three, 0.0, 0.0, 1}) == FormationRejection::None);
        assert(CheckIntentShape({5, 3, 0, Three, 0.0, 0.0, 1}) == FormationRejection::MalformedCommand);
        assert(CheckIntentShape({5, 0, 0, {}, 0.0, 0.0, 1}) == FormationRejection::MalformedCommand);
        assert(CheckIntentShape({5, 3, 0, {}, 0.0, 0.0, 4}) == FormationRejection::MalformedCommand);
        assert(CheckIntentShape({5, 3, 1, {}, 0.0, 0.0, 1}) == FormationRejection::MalformedCommand);
        assert(CheckIntentShape({5, 3, 0, {}, 5.0, 0.0, 1}) == FormationRejection::MalformedCommand);
        assert(CheckIntentShape({5, 0, 0, Dup, 0.0, 0.0, 1}) == FormationRejection::DuplicateMember);
        assert(CheckIntentShape({5, 0, 0, ZeroId, 0.0, 0.0, 1}) == FormationRejection::MalformedCommand);
        std::vector<EntityId> TooMany(static_cast<std::size_t>(kMaxUnitsPerParticipant) + 1);
        for (std::size_t Index = 0; Index < TooMany.size(); ++Index) TooMany[Index] = Index + 1;
        assert(CheckIntentShape({5, 0, 0, TooMany, 0.0, 0.0, 1}) == FormationRejection::MalformedCommand);
        assert(CheckIntentShape({6, 3, 0, {}, 0.0, 0.0, 2}) == FormationRejection::None);
        assert(CheckIntentShape({6, 3, 0, {}, 0.0, 0.0, 3}) == FormationRejection::MalformedCommand);
        assert(CheckIntentShape({6, 0, 0, Three, 0.0, 0.0, 1}) == FormationRejection::MalformedCommand);

        // Registry: a formation starts with the stance it is given and the default adherence;
        // only its owner can change either.
        const auto Always = [](Point) { return true; };
        const auto At = [](EntityId Id) { return Point{1000.0 + static_cast<double>(Id) * 10.0, 1000.0}; };
        std::vector<Entity> Six;
        for (EntityId Id = 1; Id <= 6; ++Id) Six.push_back(Entity{Id, 1});
        Registry Formations;
        const Change Formed = Formations.Form(1, Six, Shape::Line, 1, CombatStance::Guard);
        assert(Formed.Accepted());
        const FormationId Id = Formed.formed;
        assert(Formations.Find(Id)->stance == CombatStance::Guard && Formations.Find(Id)->adherence == Adherence::Flexible);
        assert(Formations.Form(1, Six, Shape::Line, 1, static_cast<CombatStance>(9)).rejection == FormationRejection::MalformedCommand);
        assert(Formations.SetStance(2, Id, CombatStance::HoldFire) == FormationRejection::UnknownFormation);
        assert(Formations.SetStance(1, FormationId{99}, CombatStance::HoldFire) == FormationRejection::UnknownFormation);
        assert(Formations.SetStance(1, Id, static_cast<CombatStance>(4)) == FormationRejection::MalformedCommand);
        assert(Formations.SetStance(1, Id, CombatStance::HoldFire) == FormationRejection::None);
        assert(Formations.Find(Id)->stance == CombatStance::HoldFire);
        assert(Formations.SetAdherence(2, Id, Adherence::Free) == FormationRejection::UnknownFormation);
        assert(Formations.SetAdherence(1, Id, static_cast<Adherence>(3)) == FormationRejection::MalformedCommand);
        // Changing stance or adherence is not an order: following and destination are untouched.
        (void)Formations.PlanMove(1, Id, {3000, 1000}, false, At, Always);
        assert(Formations.SetStance(1, Id, CombatStance::Aggressive) == FormationRejection::None);
        assert(Formations.Find(Id)->FollowingCount() == 6 && Formations.Find(Id)->hasDestination);

        // Flexible and Maintain close ranks after a loss; Free keeps membership and does not.
        assert(Formations.SetAdherence(1, Id, Adherence::Maintain) == FormationRejection::None);
        const Change Lost = Formations.MemberLost(6, 100);
        assert(Lost.regroups.size() == 1 && Formations.Find(Id)->regroupDueStep == 100 + kRegroupDelaySteps);
        // Switching to Free cancels the pending regroup; later losses schedule none.
        assert(Formations.SetAdherence(1, Id, Adherence::Free) == FormationRejection::None);
        assert(Formations.Find(Id)->regroupDueStep == 0 && Formations.DueRegroups(1000).empty());
        const Change LostFree = Formations.MemberLost(5, 200);
        assert(LostFree.Accepted() && LostFree.regroups.empty() && Formations.Find(Id)->members.size() == 4);
        assert(Formations.Find(Id)->state == FormationState::Degraded && Formations.Find(Id)->FollowingCount() == 4);
        // Back to Flexible: the next loss regroups again.
        assert(Formations.SetAdherence(1, Id, Adherence::Flexible) == FormationRejection::None);
        assert(Formations.MemberLost(4, 300).regroups.size() == 1);
    }

    // Alliance: same owner or shared non-zero team. The Unaligned (owner 0, no team) is hostile to all.
    static_assert(AreAllied(1, kTeamA, 1, kTeamA) && AreAllied(1, kTeamA, 3, kTeamA));
    static_assert(!AreAllied(1, kTeamA, 2, kTeamB) && !AreAllied(1, kNoTeam, 2, kNoTeam));
    static_assert(!AreAllied(1, kTeamA, 0, kNoTeam) && AreAllied(2, kNoTeam, 2, kNoTeam));
    // The Unaligned is its own side in every layout: never allied to a seat team,
    // explicitly attackable, but never auto-engaged in either direction.
    static_assert(!AreAllied(1, kTeamA, 0, kUnalignedTeam) && !AreAllied(0, kUnalignedTeam, 2, kTeamB));
    static_assert(AreAllied(0, kUnalignedTeam, 0, kUnalignedTeam));
    static_assert(CanAutoEngage(1, kTeamA, 2, kTeamB) && !CanAutoEngage(1, kTeamA, 3, kTeamA));
    static_assert(!CanAutoEngage(1, kTeamA, 0, kUnalignedTeam) && !CanAutoEngage(0, kUnalignedTeam, 2, kTeamB));
    static_assert(CanAutoEngage(1, kNoTeam, 0, kNoTeam)); // team-less legacy fixtures keep owner rules
    for (const TeamLayout Layout : AllLayouts)
        for (std::uint8_t Seat = 0; Seat < kMaxPlayableSeats; ++Seat)
            assert(TeamForSeat(Layout, Seat) != kUnalignedTeam);
    Entity HostUnit{11, 1}; HostUnit.team = kTeamA;
    Entity AllyUnit{12, 3}; AllyUnit.team = kTeamA;
    Entity EnemyUnit{13, 2}; EnemyUnit.team = kTeamB;
    assert(AdmitOrder(HostUnit, 1, {100, 100}, AttackOrder{12}, &AllyUnit).rejection == OrderRejection::FriendlyTarget);
    assert(AdmitOrder(HostUnit, 1, {100, 100}, GrenadeOrder{12}, &AllyUnit).rejection == OrderRejection::FriendlyTarget);
    assert(AdmitOrder(HostUnit, 1, {100, 100}, AttackOrder{13}, &EnemyUnit));
    assert(AdmitOrder(AllyUnit, 1, {100, 100}, StopOrder{}).rejection == OrderRejection::NotOwned);
    Entity UnalignedUnit{14, 0}; UnalignedUnit.team = kUnalignedTeam;
    assert(AdmitOrder(HostUnit, 1, {100, 100}, AttackOrder{14}, &UnalignedUnit));
    assert(AdmitOrder(UnalignedUnit, 1, {100, 100}, StopOrder{}).rejection == OrderRejection::NotOwned);
    assert(AdmitOrder(UnalignedUnit, 0, {100, 100}, StopOrder{}).rejection == OrderRejection::NotOwned);

    // Fog of war: an unobserved target is rejected before liveness, so a hidden live
    // unit and a hidden dead/unknown unit are indistinguishable to the requester.
    Entity HiddenEnemy = EnemyUnit; HiddenEnemy.observed = false;
    assert(AdmitOrder(HostUnit, 1, {100, 100}, AttackOrder{13}, &HiddenEnemy).rejection == OrderRejection::TargetNotVisible);
    HiddenEnemy.alive = false;
    assert(AdmitOrder(HostUnit, 1, {100, 100}, AttackOrder{13}, &HiddenEnemy).rejection == OrderRejection::TargetNotVisible);
    Entity DeadVisible = EnemyUnit; DeadVisible.alive = false;
    assert(AdmitOrder(HostUnit, 1, {100, 100}, AttackOrder{13}, &DeadVisible).rejection == OrderRejection::MissingTarget);
    // Move/stop orders never depend on target knowledge.
    assert(AdmitOrder(HostUnit, 1, {100, 100}, MoveOrder{{10, 10}}, &HiddenEnemy));

    const VisionSource Sources[] = {
        {kTeamA, {0, 0}, 100.0}, {kTeamA, {500, 0}, 0.0}, {kTeamB, {1000, 0}, 300.0}};
    assert(IsObservedBy(kTeamA, {60, 80}, Sources));           // exactly on the radius
    assert(!IsObservedBy(kTeamA, {60, 81}, Sources));
    assert(!IsObservedBy(kTeamA, {500, 0}, Sources));          // zero-radius source sees nothing
    assert(IsObservedBy(kTeamB, {800, 0}, Sources) && !IsObservedBy(kTeamA, {800, 0}, Sources));
    assert(!IsObservedBy(kNoTeam, {0, 0}, Sources));           // no team, no shared vision
    assert(!IsObservedBy(kTeamA, {std::nan(""), 0}, Sources));
    assert(!IsObservedBy(kTeamA, {0, 0}, std::span<const VisionSource>{}));

    // Spending is atomic, never accepts negative costs, and never overdrafts.
    ResourcePool balance{10, 5, 1};
    assert(TrySpend(balance, {3, 2, 0}));
    assert(balance.crude_oil == 7 && balance.natural_gas == 3);
    assert(!TrySpend(balance, {8, 0, 0}));
    assert(balance.crude_oil == 7 && balance.natural_gas == 3);
    assert(!TrySpend(balance, {-1, 0, 0}));
    assert(balance.crude_oil == 7 && balance.natural_gas == 3);
    assert(!TrySpend(balance, {0, 0, 2}));
    assert(balance.processed_nuclear == 1);

    // Viewer-side fog: the current mask comes only from the viewer team's own sources;
    // explored terrain persists after vision leaves; other teams' sources are ignored.
    {
        FogMap fog({{-500, -500}, {500, 500}, 100});
        assert(fog.Columns() == 10 && fog.Rows() == 10 && fog.Count(FogCell::Unexplored) == 100);
        const VisionSource Near[] = {{kTeamA, {-50, -50}, 100}, {kTeamB, {450, 450}, 400}};
        fog.Update(kTeamA, Near);
        assert(fog.At(4, 4) == FogCell::Visible && fog.At(0, 0) == FogCell::Unexplored);
        assert(fog.At(9, 9) == FogCell::Unexplored);               // team B's sight is not ours
        const std::size_t Seen = fog.Count(FogCell::Visible);
        assert(Seen == 5);                                         // own cell + 4 neighbours at 100 cm
        const VisionSource Moved[] = {{kTeamA, {450, -450}, 100}};
        fog.Update(kTeamA, Moved);
        assert(fog.At(4, 4) == FogCell::Explored && fog.At(9, 0) == FogCell::Visible);
        assert(fog.Count(FogCell::Explored) == Seen);
        fog.Update(kNoTeam, Moved);                                // an unseated viewer sees nothing
        assert(fog.Count(FogCell::Visible) == 0);
        FogMap invalid({{0, 0}, {0, 10}, 100});
        assert(invalid.Columns() == 0 && invalid.Count(FogCell::Visible) == 0);
    }

    // Last-known contacts: recorded only for contacts last seen alive, replaced by the
    // live entity when it is received again, and cleared as revealed only when the
    // remembered spot is clearly inside the viewer's own sight (radius minus margin).
    {
        ContactMemory memory;
        using W = ContactMemory::Withdrawal;
        assert(memory.Withdrawn({0, 2, kTeamB, {0, 0}, 1}, true) == W::Ignored);
        assert(memory.Withdrawn({7, 2, kTeamB, {1000, 0}, 10}, true) == W::Recorded);
        assert(memory.Withdrawn({7, 2, kTeamB, {1100, 0}, 20}, true) == W::Recorded);  // updated, not duplicated
        assert(memory.Contacts().size() == 1 && memory.Find(7)->position.x == 1100 && memory.Find(7)->last_seen_ms == 20);
        assert(memory.Withdrawn({8, 2, kTeamB, {0, 900}, 30}, false) == W::Ignored);   // seen dying: nothing kept
        assert(memory.Withdrawn({9, 0, kUnalignedTeam, {-900, 0}, 40}, true) == W::Recorded);
        assert(memory.Withdrawn({9, 0, kUnalignedTeam, {-900, 0}, 41}, false) == W::ForgottenDead);
        assert(memory.Reobserved(7) && !memory.Reobserved(7) && memory.Contacts().empty());

        assert(memory.Withdrawn({7, 2, kTeamB, {1000, 0}, 50}, true) == W::Recorded);
        std::size_t Cleared = 0;
        const auto Count = [&Cleared](const LastKnownContact&) { ++Cleared; };
        const VisionSource Edge[] = {{kTeamA, {0, 0}, 1100}};          // inside radius, inside margin band
        memory.ClearRevealed(kTeamA, Edge, Count);
        assert(Cleared == 0 && memory.Contacts().size() == 1);
        const VisionSource Enemy[] = {{kTeamB, {1000, 0}, 1200}};       // the contact's own team does not reveal for us
        memory.ClearRevealed(kTeamA, Enemy, Count);
        assert(Cleared == 0);
        const VisionSource Close[] = {{kTeamA, {0, 0}, 1150}};          // 1150 - 150 = 1000: exactly clear
        memory.ClearRevealed(kTeamA, Close, Count);
        assert(Cleared == 1 && memory.Contacts().empty());

        assert(memory.Withdrawn({3, 2, kTeamB, {5, 5}, 60}, true) == W::Recorded);
        memory.Clear(Count);
        assert(Cleared == 2 && memory.Contacts().empty());
        static_assert(ContactClearReasonName(ContactClearReason::Revealed) == "revealed");
    }

    // DISC-D1: one resolve attribute (Discipline / Morale), earned only in formation.
    {
        namespace pd = politika::discipline;
        using pd::Service;
        using pd::Trend;
        static_assert(pd::kBaseline == 50000 && pd::kNormalCeiling == 80000 && pd::kMaximum == 100000);
        static_assert(static_cast<int>(pd::UnitCategory::Military) == 0);
        static_assert(pd::RequiredSupport(2) == 1 && pd::RequiredSupport(3) == 2 && pd::RequiredSupport(20) == 2);
        // Earning: +1 point per evaluation up to the ceiling, never past it.
        static_assert(pd::Evaluate(50000, Service::MemberEarning).value == 51000);
        static_assert(pd::Evaluate(50000, Service::MemberEarning).trend == Trend::Rising);
        static_assert(pd::Evaluate(79500, Service::MemberEarning).value == 80000);
        static_assert(pd::Evaluate(80000, Service::MemberEarning).value == 80000);
        static_assert(pd::Evaluate(80000, Service::MemberEarning).trend == Trend::Steady);
        // Above a lower ceiling (a mass formation that shrank): fade down to it.
        static_assert(pd::Evaluate(90000, Service::MemberEarning).value == 89500);
        static_assert(pd::Evaluate(90000, Service::MemberEarning).trend == Trend::Fading);
        static_assert(pd::Evaluate(80200, Service::MemberEarning).value == 80000);
        static_assert(pd::Evaluate(99500, Service::MemberEarning, pd::kMaximum).value == 100000);
        // Holding: no gain, no loss.
        static_assert(pd::Evaluate(64000, Service::MemberHolding).value == 64000);
        static_assert(pd::Evaluate(64000, Service::MemberHolding).trend == Trend::Steady);
        // Outside: fade half a point towards the baseline, never below it.
        static_assert(pd::Evaluate(80000, Service::Outside).value == 79500);
        static_assert(pd::Evaluate(80000, Service::Outside).trend == Trend::Fading);
        static_assert(pd::Evaluate(50200, Service::Outside).value == 50000);
        static_assert(pd::Evaluate(50000, Service::Outside).value == 50000);
        static_assert(pd::Evaluate(50000, Service::Outside).trend == Trend::Steady);
        // DISC-D2: below the baseline, outside a formation and not alone, recover at the fade rate.
        static_assert(pd::Evaluate(40000, Service::Outside).value == 40500);
        static_assert(pd::Evaluate(40000, Service::Outside).trend == Trend::Rising);
        static_assert(pd::Evaluate(49800, Service::Outside).value == 50000);
        // Out-of-range input is clamped.
        static_assert(pd::Evaluate(250000, Service::Outside).value == 99500);
        static_assert(pd::Evaluate(-5, Service::MemberHolding).value == 0);
        // 30 evaluations from the baseline reach the ceiling; 60 bring it back.
        {
            std::int32_t Value = pd::kBaseline;
            for (int Second = 0; Second < 30; ++Second) Value = pd::Evaluate(Value, Service::MemberEarning).value;
            assert(Value == pd::kNormalCeiling);
            assert(pd::Evaluate(Value, Service::MemberEarning).value == pd::kNormalCeiling);
            for (int Second = 0; Second < 59; ++Second) Value = pd::Evaluate(Value, Service::Outside).value;
            assert(Value == 50500);
            assert(pd::Evaluate(Value, Service::Outside).value == pd::kBaseline);
        }
        static_assert(pd::DisplayPoints(72999) == 72 && pd::DisplayPoints(-1) == 0 && pd::DisplayPoints(100001) == 100);
        // Band crossings: rising through 60/70/80/90/100, fading only when landing on the baseline.
        static_assert(pd::CrossedBand(59000, 60000) == 60);
        static_assert(pd::CrossedBand(60000, 61000) == std::nullopt);
        static_assert(pd::CrossedBand(79000, 80000) == 80);
        static_assert(pd::CrossedBand(99000, 100000) == 100);
        static_assert(pd::CrossedBand(50000, 51000) == std::nullopt);
        static_assert(pd::CrossedBand(50500, 50000) == 50);
        static_assert(pd::CrossedBand(70000, 69500) == std::nullopt);
        // DISC-D2: falling below the baseline reports the lowest band reached; rising back lands on 50.
        static_assert(pd::CrossedBand(50000, 45000) == std::nullopt);
        static_assert(pd::CrossedBand(40500, 40000) == 40);
        static_assert(pd::CrossedBand(45000, 25000) == 30);
        static_assert(pd::CrossedBand(5000, 0) == 0);
        static_assert(pd::CrossedBand(49500, 50000) == 50);
        static_assert(pd::CrossedBand(39500, 40000) == std::nullopt);
        // Support: other members within 400 cm; each counts once; unavailable members never count.
        const double NaN = std::numeric_limits<double>::quiet_NaN();
        const politika::formation::Point Line[] = {{0, 0}, {150, 0}, {300, 0}, {450, 0}, {1000, 0}, {NaN, NaN}};
        assert(pd::SupportCount(Line, 0) == 2);       // 150 and 300; 450 is beyond 400 cm
        assert(pd::SupportCount(Line, 1) == 3);
        assert(pd::SupportCount(Line, 3) == 2);       // 150 and 300; 1000 is 550 cm away
        assert(pd::SupportCount(Line, 4) == 0);       // stranded
        assert(pd::SupportCount(Line, 5) == 0);       // unavailable
        assert(pd::SupportCount(Line, 9) == 0);       // out of range
        const politika::formation::Point Edge[] = {{0, 0}, {400, 0}};
        assert(pd::SupportCount(Edge, 0) == 1);       // exactly 400 cm supports

        // DISC-D2 isolation: a lone soldier fades to the floor of 30 and holds there.
        static_assert(pd::Evaluate(50000, Service::Isolated).value == 49500);
        static_assert(pd::Evaluate(50000, Service::Isolated).trend == Trend::Fading);
        static_assert(pd::Evaluate(70000, Service::Isolated).value == 69500); // above the baseline: the same fade
        static_assert(pd::Evaluate(30200, Service::Isolated).value == 30000);
        static_assert(pd::Evaluate(30000, Service::Isolated).value == 30000);
        static_assert(pd::Evaluate(30000, Service::Isolated).trend == Trend::Steady);
        static_assert(pd::Evaluate(20000, Service::Isolated).value == 20000); // already below: held
        {
            std::int32_t Value = pd::kBaseline;
            for (int Second = 0; Second < 39; ++Second) Value = pd::Evaluate(Value, Service::Isolated).value;
            assert(Value == 30500);
            Value = pd::Evaluate(Value, Service::Isolated).value;
            assert(Value == pd::kIsolationFloor && pd::Evaluate(Value, Service::Isolated).value == pd::kIsolationFloor);
            // Rejoining comrades: back to the baseline in another 40 evaluations.
            for (int Second = 0; Second < 40; ++Second) Value = pd::Evaluate(Value, Service::Outside).value;
            assert(Value == pd::kBaseline);
        }
        // Doctrine: Concord tolerates solo work; civilians have no isolation; mass formations reach 100.
        static_assert(pd::DoctrineOf(politika::rules::Faction::AtlanticConcord) == pd::Doctrine::SoloTolerant);
        static_assert(pd::DoctrineOf(politika::rules::Faction::RedDirectorate) == pd::Doctrine::MassFormation);
        static_assert(pd::DoctrineOf(politika::rules::Faction::IronAscendancy) == pd::Doctrine::MassFormation);
        static_assert(pd::DoctrineOf(politika::rules::Faction::MeridianPact) == pd::Doctrine::Standard);
        static_assert(pd::DoctrineOf(politika::rules::Faction::Unaligned) == pd::Doctrine::Standard);
        static_assert(pd::IsolationApplies(pd::Doctrine::Standard, pd::UnitCategory::Military));
        static_assert(pd::IsolationApplies(pd::Doctrine::MassFormation, pd::UnitCategory::Military));
        static_assert(!pd::IsolationApplies(pd::Doctrine::SoloTolerant, pd::UnitCategory::Military));
        static_assert(!pd::IsolationApplies(pd::Doctrine::Standard, pd::UnitCategory::NonCombat));
        static_assert(pd::FormationCeiling(pd::Doctrine::MassFormation, 31) == pd::kNormalCeiling);
        static_assert(pd::FormationCeiling(pd::Doctrine::MassFormation, 32) == 100000);
        static_assert(pd::FormationCeiling(pd::Doctrine::Standard, 64) == pd::kNormalCeiling);
        static_assert(pd::FormationCeiling(pd::Doctrine::SoloTolerant, 64) == pd::kNormalCeiling);
        static_assert(pd::Evaluate(80000, Service::MemberEarning, pd::kMassCeiling).value == 81000);
        {
            std::int32_t Value = pd::kNormalCeiling;
            for (int Second = 0; Second < 25; ++Second) Value = pd::Evaluate(Value, Service::MemberEarning, pd::kMassCeiling).value;
            assert(Value == pd::kMassCeiling);
            assert(pd::Evaluate(Value, Service::MemberEarning, pd::kMassCeiling).value == pd::kMassCeiling);
        }

        // DISC-D2 casualty shock: -5 per fallen comrade, at most -15 in a 100-step window opened by
        // the first shock; a new window after it; never below zero.
        {
            pd::ShockWindow Window;
            pd::ShockResult R = pd::ApplyCasualtyShock(70000, Window, 300);
            assert(R.value == 65000 && R.applied == 5000 && R.window.open && R.window.start == 300 && R.window.taken == 5000);
            R = pd::ApplyCasualtyShock(R.value, R.window, 310);
            assert(R.value == 60000 && R.window.start == 300);
            R = pd::ApplyCasualtyShock(R.value, R.window, 320);
            assert(R.value == 55000 && R.window.taken == 15000);
            R = pd::ApplyCasualtyShock(R.value, R.window, 399); // cap spent until step 400
            assert(R.value == 55000 && R.applied == 0 && R.window.taken == 15000);
            R = pd::ApplyCasualtyShock(R.value, R.window, 400); // a new window
            assert(R.value == 50000 && R.applied == 5000 && R.window.start == 400 && R.window.taken == 5000);
            const pd::ShockResult Low = pd::ApplyCasualtyShock(3000, {}, 10);
            assert(Low.value == 0 && Low.applied == 3000 && Low.window.taken == 5000);
            const pd::ShockResult Floor = pd::ApplyCasualtyShock(0, Low.window, 11);
            assert(Floor.value == 0 && Floor.applied == 0 && Floor.window.taken == 10000);
        }

        // DISC-D3 effects: one factor F = (D - 50) / 50, linear coefficients per attribute.
        {
            namespace pm = politika::modifiers;
            using pm::Attribute;
            static_assert(pd::FactorPermille(50000) == 0 && pd::FactorPermille(80000) == 600);
            static_assert(pd::FactorPermille(100000) == 1000 && pd::FactorPermille(30000) == -400);
            static_assert(pd::FactorPermille(0) == -1000 && pd::FactorPermille(-7) == -1000 && pd::FactorPermille(150000) == 1000);
            static_assert(pd::FactorPermille(71000) == 420 && pd::FactorPermille(30500) == -390);
            // At the baseline every attribute is unchanged.
            for (const pd::Effect& Effect : pd::kEffects) assert(pd::EffectPermille(Effect.attribute, pd::kBaseline) == 1000);
            // The 80 and 100 columns of section 7.
            static_assert(pd::EffectPermille(Attribute::DamageTaken, 80000) == 880);
            static_assert(pd::EffectPermille(Attribute::DamageTaken, 100000) == 800);
            static_assert(pd::EffectPermille(Attribute::Spread, 80000) == 880);
            static_assert(pd::EffectPermille(Attribute::Spread, 100000) == 800);
            static_assert(pd::EffectPermille(Attribute::FireInterval, 80000) == 910);
            static_assert(pd::EffectPermille(Attribute::FireInterval, 100000) == 850);
            static_assert(pd::EffectPermille(Attribute::ReloadTime, 80000) == 910);
            static_assert(pd::EffectPermille(Attribute::ReloadTime, 100000) == 850);
            static_assert(pd::EffectPermille(Attribute::ReturnSpeed, 80000) == 1200);
            static_assert(pd::EffectPermille(Attribute::ReturnSpeed, 100000) == 1333);
            static_assert(pd::EffectPermille(Attribute::RegroupDelay, 80000) == 775);
            static_assert(pd::EffectPermille(Attribute::RegroupDelay, 100000) == 625);
            // Below the baseline the same coefficients work against the unit; never immunity.
            static_assert(pd::EffectPermille(Attribute::DamageTaken, 30000) == 1080);
            static_assert(pd::EffectPermille(Attribute::DamageTaken, 0) == 1200);
            static_assert(pd::EffectPermille(Attribute::FireInterval, 30000) == 1060);
            static_assert(pd::EffectPermille(Attribute::ReturnSpeed, 30000) == 867);
            static_assert(pd::EffectPermille(Attribute::RegroupDelay, 30000) == 1150);
            // Monotonic: more Discipline never takes more damage or shoots slower.
            for (std::int32_t Value = 0; Value < pd::kMaximum; Value += 500)
            {
                assert(pd::EffectPermille(Attribute::DamageTaken, Value + 500) <= pd::EffectPermille(Attribute::DamageTaken, Value));
                assert(pd::EffectPermille(Attribute::FireInterval, Value + 500) <= pd::EffectPermille(Attribute::FireInterval, Value));
                assert(pd::EffectPermille(Attribute::ReturnSpeed, Value + 500) >= pd::EffectPermille(Attribute::ReturnSpeed, Value));
                assert(pd::EffectPermille(Attribute::DamageTaken, Value) >= 800);
            }
            // Regroup delay 40 steps: 31 at 80, 25 at 100, 46 at 30, 55 at 0, never below one step.
            static_assert(pd::RegroupDelaySteps(50000, 40) == 40 && pd::RegroupDelaySteps(80000, 40) == 31);
            static_assert(pd::RegroupDelaySteps(100000, 40) == 25 && pd::RegroupDelaySteps(30000, 40) == 46);
            static_assert(pd::RegroupDelaySteps(0, 40) == 55 && pd::RegroupDelaySteps(100000, 1) == 1);

            // The registry: one entry per source and attribute, replaced rather than stacked.
            pm::Registry Registry;
            assert(Registry.Combined(Attribute::DamageTaken) == 1000 && Registry.Of(pm::Source::Discipline, Attribute::Spread) == 1000);
            pd::ApplyEffects(Registry, 80000, pd::UnitCategory::Military);
            assert(Registry.Combined(Attribute::DamageTaken) == 880 && Registry.Combined(Attribute::ReturnSpeed) == 1200);
            pd::ApplyEffects(Registry, 80000, pd::UnitCategory::Military);
            assert(Registry.Combined(Attribute::DamageTaken) == 880); // applied twice, counted once
            pd::ApplyEffects(Registry, 30000, pd::UnitCategory::Military);
            assert(Registry.Combined(Attribute::DamageTaken) == 1080 && Registry.Combined(Attribute::FireInterval) == 1060);
            // Civilian Morale drives no combat effect.
            pd::ApplyEffects(Registry, 30000, pd::UnitCategory::NonCombat);
            for (const pd::Effect& Effect : pd::kEffects) assert(Registry.Combined(Effect.attribute) == 1000);
            Registry.Set(pm::Source::Discipline, Attribute::FireInterval, -4);
            assert(Registry.Combined(Attribute::FireInterval) == 0);
            Registry.Clear(pm::Source::Discipline);
            assert(Registry.Combined(Attribute::FireInterval) == 1000);
            static_assert(pm::AttributeName(Attribute::DamageTaken) == "damage_taken" && pm::SourceName(pm::Source::Discipline) == "discipline");
        }

        // DISC-D4 rout: a soldier breaks at 0 and rallies at 15; civilians never break.
        {
            static_assert(pd::Breaks(0, pd::UnitCategory::Military) && pd::Breaks(-1, pd::UnitCategory::Military));
            static_assert(!pd::Breaks(1, pd::UnitCategory::Military) && !pd::Breaks(pd::kBaseline, pd::UnitCategory::Military));
            static_assert(!pd::Breaks(0, pd::UnitCategory::NonCombat));
            static_assert(!pd::Rallies(14999) && pd::Rallies(15000) && pd::Rallies(pd::kBaseline));
            // A routed unit recovers 0.5 per evaluation wherever it is: 30 evaluations to rally.
            pd::Evaluation Routed{0, pd::Trend::Steady};
            int Evaluations = 0;
            while (!pd::Rallies(Routed.value) && Evaluations < 100)
            {
                Routed = pd::Evaluate(Routed.value, pd::Service::Routed);
                assert(Routed.trend == pd::Trend::Rising);
                ++Evaluations;
            }
            assert(Evaluations == 30 && Routed.value == pd::kRallyThreshold);
            // Unlike a lone soldier below the floor (which holds), a routed one recovers.
            static_assert(pd::Evaluate(0, pd::Service::Isolated).value == 0);
            static_assert(pd::Evaluate(0, pd::Service::Routed).value == 500);
            // Rout recovery never overshoots the baseline.
            static_assert(pd::Evaluate(pd::kBaseline, pd::Service::Routed).value == pd::kBaseline);
            // Casualty shock is what takes a member to the threshold: 10 points is two shocks.
            const pd::ShockResult First = pd::ApplyCasualtyShock(10000, {}, 5);
            const pd::ShockResult Second = pd::ApplyCasualtyShock(First.value, First.window, 6);
            assert(!pd::Breaks(First.value, pd::UnitCategory::Military) && pd::Breaks(Second.value, pd::UnitCategory::Military));
            static_assert(pd::CrossedBand(5000, 0) == 0);
        }
    }

    // DISC-D4: a routed issuer is refused after ownership is settled, so only its owner learns it.
    {
        static_assert(static_cast<std::uint8_t>(OrderRejection::Routed) == 19 && kOrderRejectionCount == 20);
        Entity RoutedIssuer{7, 1, true, true, true, true, true, true};
        RoutedIssuer.routed = true;
        CommandRequest Request{7, 1, 1, 7, 70, MoveOrder{{10, 10}}};
        assert(AdmitCommand(Request, 7, true, &RoutedIssuer, {100, 100}).rejection == OrderRejection::Routed);
        CommandRequest Foreign = Request;
        Foreign.participant_id = 2;
        assert(AdmitCommand(Foreign, 7, true, &RoutedIssuer, {100, 100}).rejection == OrderRejection::Unauthorized);
        RoutedIssuer.routed = false;
        assert(AdmitCommand(Request, 7, true, &RoutedIssuer, {100, 100}));
    }

    // Faction nuclear permissions are independent of ownership and unit count.
    static_assert(CanRefineNuclear(Faction::AtlanticConcord));
    static_assert(CanRefineNuclear(Faction::RedDirectorate));
    static_assert(!CanRefineNuclear(Faction::MeridianPact));
    static_assert(!CanRefineNuclear(Faction::Unaligned));
    static_assert(!CanRefineNuclear(Faction::IronAscendancy));
    static_assert(!CanRefineNuclear(Faction::IronAscendancy, {true, false}));
    static_assert(!CanRefineNuclear(Faction::IronAscendancy, {false, true}));
    static_assert(CanRefineNuclear(Faction::IronAscendancy, {true, true}));

    // Early conventional sabotage is a distinct playable-faction capability.
    static_assert(CanPlantExplosives(Faction::MeridianPact, SabotageOperator::Conventional, false));
    static_assert(!CanPlantExplosives(Faction::AtlanticConcord, SabotageOperator::Spy, false));
    static_assert(CanPlantExplosives(Faction::AtlanticConcord, SabotageOperator::Spy, true));
    static_assert(!CanPlantExplosives(Faction::RedDirectorate, SabotageOperator::Conventional, true));
    static_assert(!CanPlantExplosives(Faction::IronAscendancy, SabotageOperator::Conventional, true));
    static_assert(!CanPlantExplosives(Faction::Unaligned, SabotageOperator::Spy, true));

    // ECON-E5: a box over soldiers and a harvester takes the soldiers only.
    {
        namespace ps = politika::selection;
        const auto Combat = [](int Unit) { return Unit > 0; };
        assert((ps::PreferCombat<int>({1, -2, 3}, ps::SelectionOp::Replace, Combat) == std::vector<int>{1, 3}));
        assert((ps::PreferCombat<int>({1, -2}, ps::SelectionOp::Union, Combat) == std::vector<int>{1}));
        assert((ps::PreferCombat<int>({-2, -4}, ps::SelectionOp::Replace, Combat) == std::vector<int>{-2, -4}));
        assert((ps::PreferCombat<int>({1, -2}, ps::SelectionOp::Subtract, Combat) == std::vector<int>{1, -2}));
    }

    // ECON-E4: a finite source, harvesters that haul it on their own, and an atomic sale at a fixed quote.
    {
        namespace pe = politika::economy;
        using pe::EconomyRejection;
        using pe::HarvesterPhase;
        static_assert(pe::kSourceGold > 0 && pe::kLoadGold > 0 && pe::kSaleLotGold > 0 && pe::kGoldRateCents > 0);
        static_assert(pe::kHarvesterLoadSteps > 0 && pe::kHarvesterUnloadSteps > 0);
        // The fee rounds up to a whole cent, so a split sale never earns more than a whole one.
        static_assert(pe::QuoteSale(25, 250, 300)->gross == 6'250 && pe::QuoteSale(25, 250, 300)->fee == 188 && pe::QuoteSale(25, 250, 300)->net == 6'062);
        static_assert(pe::QuoteSale(40, 250, 300)->net == 9'700);
        static_assert(pe::QuoteSale(25, 250, 300)->net + pe::QuoteSale(15, 250, 300)->net <= pe::QuoteSale(40, 250, 300)->net);
        static_assert(pe::QuoteSale(1, 1, 1)->fee == 1 && pe::QuoteSale(1, 1, 1)->net == 0);
        static_assert(pe::QuoteSale(10, 250, 0)->fee == 0 && pe::QuoteSale(10, 250, 10'000)->net == 0);
        static_assert(!pe::QuoteSale(0, 250, 300) && !pe::QuoteSale(-1, 250, 300) && !pe::QuoteSale(1, 0, 300));
        static_assert(!pe::QuoteSale(1, 250, -1) && !pe::QuoteSale(1, 250, 10'001));
        static_assert(!pe::QuoteSale(std::numeric_limits<pe::Gold>::max(), 2, 0));
        // The fee on the largest gross that fits does not overflow either.
        static_assert(pe::QuoteSale(std::numeric_limits<pe::Gold>::max(), 1, 10'000)->net == 0);
        static_assert(pe::QuoteGoldLot(pe::kSaleLotGold + 1)->quantity == pe::kSaleLotGold);
        static_assert(pe::QuoteGoldLot(5)->quantity == 5 && !pe::QuoteGoldLot(0));
        assert(pe::CheckIntentShape(static_cast<std::uint8_t>(pe::EconomyOp::SellGold), 0) == EconomyRejection::None);
        assert(pe::CheckIntentShape(static_cast<std::uint8_t>(pe::EconomyOp::SellGold), 1) == EconomyRejection::MalformedCommand);
        assert(pe::CheckIntentShape(pe::kEconomyOpCount, 0) == EconomyRejection::MalformedCommand);
        constexpr std::uint8_t RiflemanRecipe = static_cast<std::uint8_t>(pe::Recipe::Rifleman);
        constexpr std::uint8_t HarvesterRecipe = static_cast<std::uint8_t>(pe::Recipe::Harvester);
        assert(pe::FindRecipe(HarvesterRecipe) && !pe::FindRecipe(pe::kRecipeCount));
        constexpr pe::Resource GoldKind = pe::Resource::Gold;
        constexpr pe::Resource FoodKind = pe::Resource::Food;
        const pe::HarvesterSituation Driving{std::nullopt, false, true};
        const pe::HarvesterSituation AtSource{GoldKind, false, true};
        const pe::HarvesterSituation ArrivedAtSource{GoldKind, false, false};
        const pe::HarvesterSituation ArrivedAtFood{FoodKind, false, false};
        const pe::HarvesterSituation AtFood{FoodKind, false, true};
        const pe::HarvesterSituation AtBase{std::nullopt, true, true};
        const pe::HarvesterSituation ArrivedAtBase{std::nullopt, true, false};
        const pe::HarvesterSituation Stopped{std::nullopt, false, false};
        // A new harvester takes its first order at its first step.
        const auto Hire = [&Driving](pe::Ledger& Ledger, pe::ParticipantId Participant, pe::UnitId Unit) {
            return Ledger.AddHarvester(Participant, Unit) && Ledger.StepHarvester(Participant, Unit, Driving).goToSource;
        };
        const auto Run = [](pe::Ledger& Ledger, pe::ParticipantId Participant, pe::UnitId Unit, pe::HarvesterSituation Situation, std::uint32_t Steps) {
            pe::HarvesterStep Last;
            for (std::uint32_t Index = 0; Index < Steps; ++Index) Last = Ledger.StepHarvester(Participant, Unit, Situation);
            return Last;
        };

        pe::Ledger Ledger;
        assert(Ledger.AddParticipant(1, pe::kStartingCents, true, 200) && Ledger.AddParticipant(2, pe::kStartingCents, true, 200));
        const pe::Account& A = *Ledger.Find(1);
        const pe::Account& B = *Ledger.Find(2);
        // A seat without a source cannot train a harvester or keep one.
        assert(Ledger.Train(1, 1, HarvesterRecipe, 2, 20).rejection == EconomyRejection::NoResourceSource && A.available == pe::kStartingCents && A.lastTransaction == 0);
        assert(!Ledger.AddHarvester(1, 10));
        assert(!Ledger.SetSource(9, pe::Resource::Gold, 10) && !Ledger.SetSource(1, pe::Resource::Gold, -1));
        assert(Ledger.SetSource(1, pe::Resource::Gold, pe::kLoadGold + 10) && Ledger.SetSource(2, pe::Resource::Gold, pe::kLoadGold));
        assert(A.goldSource.present && A.goldSource.remaining == pe::kLoadGold + 10 && A.GoldReconciles());

        // Nothing to sell yet, and the refusal uses up no transaction.
        assert(Ledger.SellGold(1, 1).rejection == EconomyRejection::NothingToSell && A.available == pe::kStartingCents && A.lastTransaction == 0);
        assert(Ledger.SellGold(9, 1).rejection == EconomyRejection::Unauthorized);

        // A harvester is one unit, once, on one seat.
        assert(!Ledger.AddHarvester(1, 0) && !Ledger.AddHarvester(9, 10));
        assert(Ledger.AddHarvester(1, 10) && !Ledger.AddHarvester(1, 10) && !Ledger.FindHarvester(2, 10));
        const pe::Harvester* H = Ledger.FindHarvester(1, 10);
        assert(H->phase == HarvesterPhase::Starting && H->load == 0 && H->target == GoldKind);
        // Its first order comes at its first step, whatever it stands beside.
        assert(Ledger.StepHarvester(1, 10, Stopped).goToSource == GoldKind && H->phase == HarvesterPhase::ToSource);
        // Unknown units do nothing.
        assert(Ledger.StepHarvester(1, 99, AtSource).loaded == 0 && Ledger.StepHarvester(9, 10, AtSource).loaded == 0);

        // Driving to the source changes nothing, and loading takes its full time.
        Run(Ledger, 1, 10, Driving, 5);
        assert(H->phase == HarvesterPhase::ToSource && A.goldSource.remaining == pe::kLoadGold + 10);
        assert(Ledger.StepHarvester(1, 10, AtSource).loaded == 0 && H->phase == HarvesterPhase::Loading);
        Run(Ledger, 1, 10, AtSource, pe::kHarvesterLoadSteps - 1);
        assert(H->phase == HarvesterPhase::Loading && H->load == 0 && A.goldSource.remaining == pe::kLoadGold + 10);
        pe::HarvesterStep Step = Ledger.StepHarvester(1, 10, AtSource);
        assert(Step.loaded == pe::kLoadGold && Step.goToBase && !Step.goToSource && H->phase == HarvesterPhase::ToBase);
        assert(H->load == pe::kLoadGold && A.GoldCarried() == pe::kLoadGold && A.goldSource.remaining == 10 && A.goldHeld == 0 && A.GoldReconciles());

        // Driving home, then unloading: carried is not held until it ends, and it counts once.
        Run(Ledger, 1, 10, Driving, 5);
        assert(H->phase == HarvesterPhase::ToBase && A.goldHeld == 0);
        assert(Ledger.StepHarvester(1, 10, AtBase).delivered == 0 && H->phase == HarvesterPhase::Unloading);
        Run(Ledger, 1, 10, AtBase, pe::kHarvesterUnloadSteps - 1);
        assert(A.goldHeld == 0 && H->load == pe::kLoadGold);
        Step = Ledger.StepHarvester(1, 10, AtBase);
        assert(Step.delivered == pe::kLoadGold && Step.goToSource == GoldKind && !Step.goToBase && H->phase == HarvesterPhase::ToSource);
        assert(H->load == 0 && A.goldHeld == pe::kLoadGold && A.goldSource.delivered == pe::kLoadGold && A.GoldReconciles());

        // A player's order takes it off the routine. It goes where it was sent and stays there.
        assert(!Ledger.DetachHarvester(1, 99) && Ledger.DetachHarvester(1, 10) && H->phase == HarvesterPhase::Waiting);
        assert(!Ledger.DetachHarvester(1, 10));
        Run(Ledger, 1, 10, Driving, 5);
        Run(Ledger, 1, 10, Stopped, 5);
        assert(H->phase == HarvesterPhase::Waiting && A.goldSource.remaining == 10);
        // Passing the source under a player's order is not a return to work.
        assert(!Ledger.StepHarvester(1, 10, AtSource).stoppedWaiting && H->phase == HarvesterPhase::Waiting);
        // Nor is a player's stop at the source: it holds there until it is sent somewhere.
        assert(!Ledger.StepHarvester(1, 10, {GoldKind, false, false, true}).stoppedWaiting && H->phase == HarvesterPhase::Waiting);
        // Arriving at the source is: the last load is what is left.
        Step = Ledger.StepHarvester(1, 10, ArrivedAtSource);
        assert(Step.stoppedWaiting && H->phase == HarvesterPhase::Loading);
        Step = Run(Ledger, 1, 10, AtSource, pe::kHarvesterLoadSteps);
        assert(Step.loaded == 10 && Step.goToBase && A.goldSource.remaining == 0 && H->load == 10 && A.GoldReconciles());

        // Sent away with a load, it keeps it, and stopping at the base delivers it.
        assert(Ledger.DetachHarvester(1, 10));
        Run(Ledger, 1, 10, Stopped, 5);
        assert(H->phase == HarvesterPhase::Waiting && H->load == 10);
        Step = Ledger.StepHarvester(1, 10, ArrivedAtBase);
        assert(Step.stoppedWaiting && H->phase == HarvesterPhase::Unloading);
        // With the source empty the routine ends, and the empty source does not restart it.
        Step = Run(Ledger, 1, 10, AtBase, pe::kHarvesterUnloadSteps);
        assert(Step.delivered == 10 && Step.startedWaiting && !Step.goToSource && H->phase == HarvesterPhase::Waiting);
        assert(!Ledger.StepHarvester(1, 10, ArrivedAtSource).stoppedWaiting && H->phase == HarvesterPhase::Waiting);
        assert(A.goldHeld == pe::kLoadGold + 10 && A.GoldReconciles());
        // An empty source refuses a new harvester, and a seat's rifleman is unaffected.
        assert(Ledger.Train(1, 1, HarvesterRecipe, 3, 20).rejection == EconomyRejection::NoResourceSource && A.lastTransaction == 0);
        assert(!Ledger.SetSource(1, GoldKind, 100) && A.goldSource.start == pe::kLoadGold + 10);

        // A route that fails on the way ends the routine rather than spinning.
        pe::Ledger Lost;
        assert(Lost.AddParticipant(1, pe::kStartingCents, true, 200) && Lost.SetSource(1, pe::Resource::Gold, 2 * pe::kLoadGold));
        const pe::Account& L = *Lost.Find(1);
        assert(Hire(Lost, 1, 7));
        Step = Lost.StepHarvester(1, 7, Stopped);
        assert(Step.startedWaiting && Lost.FindHarvester(1, 7)->phase == HarvesterPhase::Waiting);
        // A dead harvester's load is gone: not held, and not back at the source.
        assert(Lost.LoseHarvester(1, 99) == 0 && Lost.StepHarvester(1, 7, ArrivedAtSource).stoppedWaiting);
        Run(Lost, 1, 7, AtSource, pe::kHarvesterLoadSteps);
        assert(Lost.FindHarvester(1, 7)->load == pe::kLoadGold && L.goldSource.remaining == pe::kLoadGold);
        assert(Lost.LoseHarvester(1, 7) == pe::kLoadGold && L.goldSource.lost == pe::kLoadGold && L.goldSource.remaining == pe::kLoadGold && L.goldHeld == 0);
        assert(L.harvesters.empty() && !Lost.FindHarvester(1, 7) && Lost.LoseHarvester(1, 7) == 0 && L.GoldReconciles());

        // Two harvesters at one source: the lower unit takes the load, the other what is left.
        pe::Ledger Shared;
        assert(Shared.AddParticipant(1, pe::kStartingCents, true, 200) && Shared.SetSource(1, pe::Resource::Gold, pe::kLoadGold + 5));
        const pe::Account& S2 = *Shared.Find(1);
        assert(Hire(Shared, 1, 9) && Hire(Shared, 1, 3) && S2.harvesters[0].unit == 3 && S2.harvesters[1].unit == 9);
        Run(Shared, 1, 3, AtSource, pe::kHarvesterLoadSteps);
        Run(Shared, 1, 9, AtSource, pe::kHarvesterLoadSteps);
        assert(Shared.FindHarvester(1, 3)->phase == HarvesterPhase::Loading && Shared.FindHarvester(1, 9)->phase == HarvesterPhase::Loading);
        assert(Shared.StepHarvester(1, 3, AtSource).loaded == pe::kLoadGold);
        assert(Shared.StepHarvester(1, 9, AtSource).loaded == 5 && S2.goldSource.remaining == 0 && S2.GoldCarried() == pe::kLoadGold + 5 && S2.GoldReconciles());
        // One that arrives at a source just emptied goes back to waiting, with nothing taken.
        pe::Ledger Late;
        assert(Late.AddParticipant(1, pe::kStartingCents, true, 200) && Late.SetSource(1, pe::Resource::Gold, pe::kLoadGold));
        assert(Hire(Late, 1, 1) && Hire(Late, 1, 2));
        Run(Late, 1, 1, AtSource, pe::kHarvesterLoadSteps + 1);
        assert(Late.StepHarvester(1, 2, AtSource).startedWaiting && Late.FindHarvester(1, 2)->load == 0);
        // A harvester trained after the source emptied starts by waiting.
        assert(Late.AddHarvester(1, 3) && Late.FindHarvester(1, 3)->phase == HarvesterPhase::Waiting);

        // No living depot: a load on the way home, or already unloading, is not credited. When a
        // depot exists again the harvester is sent there, and only then unloads. A player's order
        // still takes it off that hold.
        {
            pe::Ledger Held;
            assert(Held.AddParticipant(1, pe::kStartingCents, true, 200) && Held.SetSource(1, pe::Resource::Gold, pe::kLoadGold));
            const pe::Account& D = *Held.Find(1);
            assert(Hire(Held, 1, 4));
            Run(Held, 1, 4, AtSource, pe::kHarvesterLoadSteps + 1);
            const pe::Harvester* DH = Held.FindHarvester(1, 4);
            assert(DH->phase == HarvesterPhase::ToBase && DH->load == pe::kLoadGold && D.goldHeld == 0);
            pe::HarvesterSituation NoDepot = Driving;
            NoDepot.hasDepot = false;
            Step = Held.StepHarvester(1, 4, NoDepot);
            assert(Step.heldForDepot && !Step.goToBase && DH->phase == HarvesterPhase::Holding && DH->load == pe::kLoadGold && D.goldHeld == 0);
            Run(Held, 1, 4, NoDepot, 5);
            assert(DH->phase == HarvesterPhase::Holding && D.goldHeld == 0 && D.goldSource.delivered == 0 && D.GoldReconciles());
            assert(Held.DetachHarvester(1, 4) && DH->phase == HarvesterPhase::Waiting);
            assert(!Held.StepHarvester(1, 4, NoDepot).goToBase && DH->phase == HarvesterPhase::Waiting && DH->load == pe::kLoadGold);
            assert(Held.StepHarvester(1, 4, ArrivedAtBase).stoppedWaiting && DH->phase == HarvesterPhase::Unloading);
            Step = Held.StepHarvester(1, 4, NoDepot);
            assert(Step.heldForDepot && Step.delivered == 0 && DH->phase == HarvesterPhase::Holding && DH->load == pe::kLoadGold && D.goldHeld == 0);
            Step = Held.StepHarvester(1, 4, Driving);
            assert(Step.goToBase && Step.stoppedWaiting && DH->phase == HarvesterPhase::ToBase);
            Step = Run(Held, 1, 4, AtBase, pe::kHarvesterUnloadSteps + 1);
            assert(Step.delivered == pe::kLoadGold && D.goldHeld == pe::kLoadGold && D.goldSource.delivered == pe::kLoadGold && D.GoldReconciles());
        }

        // A failed trip home remembers that producer. The same one is not asked again. A different
        // living producer is. A player's order is not pulled along by either.
        {
            pe::Ledger Retry;
            assert(Retry.AddParticipant(1, pe::kStartingCents, true, 200) && Retry.SetSource(1, pe::Resource::Gold, pe::kLoadGold));
            const pe::Account& R = *Retry.Find(1);
            assert(Hire(Retry, 1, 8));
            Run(Retry, 1, 8, AtSource, pe::kHarvesterLoadSteps + 1);
            const pe::Harvester* RH = Retry.FindHarvester(1, 8);
            assert(RH->phase == HarvesterPhase::ToBase && RH->load == pe::kLoadGold);
            pe::HarvesterSituation Home = Driving;
            Home.moving = false;
            Home.hasDepot = true;
            Home.depotId = 11;
            Step = Retry.StepHarvester(1, 8, Home);
            assert(Step.startedWaiting && !Step.goToBase && RH->phase == HarvesterPhase::Waiting && RH->refusedDepot == 11 && RH->load == pe::kLoadGold && R.goldHeld == 0);
            assert(!Retry.StepHarvester(1, 8, Home).goToBase && RH->phase == HarvesterPhase::Waiting);
            Home.depotId = 22;
            Step = Retry.StepHarvester(1, 8, Home);
            assert(Step.goToBase && Step.stoppedWaiting && RH->phase == HarvesterPhase::ToBase && RH->load == pe::kLoadGold);
            assert(Retry.DetachHarvester(1, 8) && RH->released && RH->phase == HarvesterPhase::Waiting);
            Home.depotId = 33;
            assert(!Retry.StepHarvester(1, 8, Home).goToBase && RH->phase == HarvesterPhase::Waiting && RH->load == pe::kLoadGold && R.goldHeld == 0);
        }

        // One lot: gold out and net dollars in, once.
        pe::Ledger Seller;
        assert(Seller.AddParticipant(1, pe::kStartingCents, true, 200) && Seller.SetSource(1, pe::Resource::Gold, 30));
        const pe::Account& C = *Seller.Find(1);
        const auto Haul = [&](pe::UnitId Unit) {
            assert(Hire(Seller, 1, Unit));
            Run(Seller, 1, Unit, AtSource, pe::kHarvesterLoadSteps + 1);
            Run(Seller, 1, Unit, AtBase, pe::kHarvesterUnloadSteps + 1);
        };
        Haul(1);
        Haul(2);
        assert(C.goldHeld == 30 && C.goldSource.remaining == 0);
        const pe::RequestResult First = Seller.SellGold(1, 1);
        assert(First && First.amount == 6'062 && First.sale.quantity == pe::kSaleLotGold && First.sale.gross == 6'250 && First.sale.fee == 188);
        assert(C.available == pe::kStartingCents + 6'062 && C.saleCredited == 6'062 && C.credited == 0);
        assert(C.goldHeld == 5 && C.goldSold == pe::kSaleLotGold && C.Reconciles() && C.GoldReconciles());
        // A repeated request changes nothing.
        assert(Seller.SellGold(1, 1).rejection == EconomyRejection::DuplicateCommand && C.available == pe::kStartingCents + 6'062 && C.goldSold == pe::kSaleLotGold);
        // Less than a lot sells what is left.
        const pe::RequestResult Second = Seller.SellGold(1, 2);
        assert(Second && Second.sale.quantity == 5 && Second.sale.gross == 1'250 && Second.sale.fee == 38 && Second.amount == 1'212);
        assert(C.goldHeld == 0 && C.goldSold == 30 && C.saleCredited == 7'274 && C.Reconciles() && C.GoldReconciles());
        assert(Seller.SellGold(1, 3).rejection == EconomyRejection::NothingToSell && C.lastTransaction == 2);
        // The other seat's gold never moved.
        assert(B.goldSource.remaining == pe::kLoadGold && B.goldHeld == 0 && B.available == pe::kStartingCents);

        // Selling works during a food shortage and an outage: it can pay the way out of both.
        pe::Ledger Short;
        assert(Short.AddParticipant(1, pe::kStartingCents, true, 200) && Short.SetSource(1, pe::Resource::Gold, pe::kLoadGold) && Short.SetStartingRations(1, 0));
        const pe::Account& S = *Short.Find(1);
        assert(Hire(Short, 1, 1));
        Run(Short, 1, 1, AtSource, pe::kHarvesterLoadSteps + 1);
        Run(Short, 1, 1, AtBase, pe::kHarvesterUnloadSteps + 1);
        Short.Advance(1, 200, 0, 3);
        assert(Short.SetGeneratorOnline(1, false));
        Short.Advance(1, 201, 0, 3);
        assert(S.foodShortage && !S.producerPowered && Short.SellGold(1, 1));

        // Sale proceeds pay for training, in arrival order within one step, and a harvester queues
        // and refunds like any other recipe.
        pe::Ledger Poor;
        const pe::Cents Price = pe::FindRecipe(RiflemanRecipe)->cost;
        assert(Poor.AddParticipant(1, Price - 1, true, 200) && Poor.SetSource(1, pe::Resource::Gold, 2 * pe::kLoadGold));
        const pe::Account& P = *Poor.Find(1);
        assert(Hire(Poor, 1, 1));
        Run(Poor, 1, 1, AtSource, pe::kHarvesterLoadSteps + 1);
        Run(Poor, 1, 1, AtBase, pe::kHarvesterUnloadSteps + 1);
        assert(Poor.Train(1, 1, RiflemanRecipe, 2, 20).rejection == EconomyRejection::InsufficientFunds && P.lastTransaction == 0);
        assert(Poor.SellGold(1, 1) && Poor.Train(1, 2, RiflemanRecipe, 2, 20) && P.queue.size() == 1 && P.Reconciles());
        // A cancel refunds the price, not the sale.
        assert(Poor.CancelLast(1, 3) && P.available == Price - 1 + P.saleCredited && P.goldSold == pe::kSaleLotGold && P.Reconciles());
        const pe::RequestResult Truck = Poor.Train(1, 4, HarvesterRecipe, 2, 20);
        assert(Truck && Truck.amount == pe::FindRecipe(HarvesterRecipe)->cost && P.queue.back().recipe == pe::Recipe::Harvester);
        const pe::Cents WithTruck = P.available;
        assert(Poor.CancelLast(1, 5) && P.available == WithTruck + Truck.amount && P.Reconciles());

        // A sale that would overflow the treasury changes nothing.
        pe::Ledger Rich;
        assert(Rich.AddParticipant(1, std::numeric_limits<pe::Cents>::max() - 1, false, 200) && Rich.SetSource(1, pe::Resource::Gold, pe::kLoadGold));
        const pe::Account& R = *Rich.Find(1);
        assert(Hire(Rich, 1, 1));
        Run(Rich, 1, 1, AtSource, pe::kHarvesterLoadSteps + 1);
        Run(Rich, 1, 1, AtBase, pe::kHarvesterUnloadSteps + 1);
        assert(Rich.SellGold(1, 1).rejection == EconomyRejection::Overflow && R.goldHeld == pe::kLoadGold && R.goldSold == 0 && R.lastTransaction == 0);

        // ECON-E5: food is a second typed source, and a harvester collects either.
        pe::Ledger Mixed;
        assert(Mixed.AddParticipant(1, pe::kStartingCents, true, 200) && Mixed.SetSource(1, GoldKind, 2 * pe::kLoadGold) && Mixed.SetSource(1, FoodKind, pe::kLoadFood + 5));
        const pe::Account& M = *Mixed.Find(1);
        assert(M.foodSource.present && M.foodSource.remaining == pe::kLoadFood + 5 && M.FoodSourceReconciles());
        assert(Hire(Mixed, 1, 1));
        const pe::Harvester* MH = Mixed.FindHarvester(1, 1);
        assert(MH->target == GoldKind);
        // Passing the other source on the way changes nothing.
        Run(Mixed, 1, 1, AtFood, 5);
        assert(MH->phase == HarvesterPhase::ToSource && M.foodSource.remaining == pe::kLoadFood + 5);
        // A player sends it to the food during loading: no gold has moved, and food becomes its target.
        assert(Mixed.StepHarvester(1, 1, AtSource).arrivedAtSource && Mixed.DetachHarvester(1, 1));
        assert(M.goldSource.remaining == 2 * pe::kLoadGold);
        Step = Mixed.StepHarvester(1, 1, ArrivedAtFood);
        assert(Step.stoppedWaiting && MH->phase == HarvesterPhase::Loading && MH->target == FoodKind);
        Step = Run(Mixed, 1, 1, AtFood, pe::kHarvesterLoadSteps);
        assert(Step.loaded == pe::kLoadFood && Step.resource == FoodKind && Step.goToBase);
        assert(MH->loadKind == FoodKind && MH->load == pe::kLoadFood && M.FoodCarried() == pe::kLoadFood && M.GoldCarried() == 0);
        assert(M.FoodSourceReconciles() && M.GoldReconciles());
        // Food goes straight into the reserve, once, and leaves the gold stock alone.
        const pe::Rations FedBefore = M.rations;
        Step = Mixed.StepHarvester(1, 1, AtBase);
        Step = Run(Mixed, 1, 1, AtBase, pe::kHarvesterUnloadSteps);
        assert(Step.delivered == pe::kLoadFood && Step.resource == FoodKind && Step.goToSource == FoodKind && !Step.retargeted);
        assert(M.rations == FedBefore + pe::kLoadFood && M.foodSource.delivered == pe::kLoadFood && M.goldHeld == 0);
        assert(M.RationsReconcile() && M.FoodSourceReconciles());
        // It keeps to food: the part load, then the food has run out and it moves on to the gold.
        assert(Mixed.StepHarvester(1, 1, AtFood).arrivedAtSource);
        Step = Run(Mixed, 1, 1, AtFood, pe::kHarvesterLoadSteps);
        assert(Step.loaded == 5 && M.foodSource.remaining == 0);
        Mixed.StepHarvester(1, 1, AtBase);
        Step = Run(Mixed, 1, 1, AtBase, pe::kHarvesterUnloadSteps);
        assert(Step.delivered == 5 && Step.retargeted && Step.goToSource == GoldKind && MH->target == GoldKind);
        // Sent to the food with a gold load, it takes the gold home first, then goes for the food.
        pe::Ledger Sent;
        assert(Sent.AddParticipant(1, pe::kStartingCents, true, 200) && Sent.SetSource(1, GoldKind, 2 * pe::kLoadGold) && Sent.SetSource(1, FoodKind, pe::kLoadFood));
        const pe::Account& N = *Sent.Find(1);
        assert(Hire(Sent, 1, 1));
        const pe::Harvester* NH = Sent.FindHarvester(1, 1);
        Sent.StepHarvester(1, 1, AtSource);
        Run(Sent, 1, 1, AtSource, pe::kHarvesterLoadSteps);
        assert(NH->load == pe::kLoadGold && NH->phase == HarvesterPhase::ToBase && Sent.DetachHarvester(1, 1));
        Step = Sent.StepHarvester(1, 1, ArrivedAtFood);
        assert(Step.stoppedWaiting && Step.goToBase && NH->target == FoodKind && NH->loadKind == GoldKind && N.foodSource.remaining == pe::kLoadFood);
        Sent.StepHarvester(1, 1, AtBase);
        Step = Run(Sent, 1, 1, AtBase, pe::kHarvesterUnloadSteps);
        assert(Step.delivered == pe::kLoadGold && Step.resource == GoldKind && N.goldHeld == pe::kLoadGold && Step.goToSource == FoodKind && !Step.retargeted);
        // A dead harvester's food is lost to the food source's account, not the gold's.
        Sent.StepHarvester(1, 1, AtFood);
        Run(Sent, 1, 1, AtFood, pe::kHarvesterLoadSteps);
        assert(Sent.LoseHarvester(1, 1) == pe::kLoadFood && N.foodSource.lost == pe::kLoadFood && N.goldSource.lost == 0);
        assert(N.FoodSourceReconciles() && N.GoldReconciles());
        // Nothing left of either: a harvester is refused, and one that is hired anyway waits.
        pe::Ledger Bare;
        assert(Bare.AddParticipant(1, pe::kStartingCents, true, 200) && Bare.SetSource(1, GoldKind, 0) && Bare.SetSource(1, FoodKind, 0));
        assert(Bare.Train(1, 1, HarvesterRecipe, 2, 20).rejection == EconomyRejection::NoResourceSource);
        assert(Bare.AddHarvester(1, 1) && Bare.FindHarvester(1, 1)->phase == HarvesterPhase::Waiting);
        // Food alone is enough for a harvester, and it starts on that.
        pe::Ledger FoodOnly;
        assert(FoodOnly.AddParticipant(1, pe::kStartingCents, true, 200) && FoodOnly.SetSource(1, FoodKind, pe::kLoadFood));
        assert(FoodOnly.Train(1, 1, HarvesterRecipe, 2, 20));
        assert(FoodOnly.AddHarvester(1, 1));
        Step = FoodOnly.StepHarvester(1, 1, Stopped);
        assert(Step.goToSource == FoodKind && Step.retargeted);
        // A full store takes what fits. The rest stays on the harvester until there is room.
        pe::Ledger FullStore;
        assert(FullStore.AddParticipant(1, pe::kStartingCents, true, 200) && FullStore.SetSource(1, FoodKind, pe::kLoadFood));
        assert(FullStore.SetStartingRations(1, pe::kFoodCapacityRations - 10));
        const pe::Account& U = *FullStore.Find(1);
        assert(Hire(FullStore, 1, 1));
        const pe::Harvester* UH = FullStore.FindHarvester(1, 1);
        FullStore.StepHarvester(1, 1, AtFood);
        Run(FullStore, 1, 1, AtFood, pe::kHarvesterLoadSteps);
        FullStore.StepHarvester(1, 1, AtBase);
        Step = Run(FullStore, 1, 1, AtBase, pe::kHarvesterUnloadSteps);
        assert(Step.delivered == 10 && U.rations == pe::kFoodCapacityRations && UH->load == pe::kLoadFood - 10);
        assert(UH->phase == HarvesterPhase::Unloading && U.RationsReconcile() && U.FoodSourceReconciles());
        Step = Run(FullStore, 1, 1, AtBase, pe::kHarvesterStoreRetrySteps);
        assert(Step.delivered == 0 && UH->phase == HarvesterPhase::Unloading && UH->load == pe::kLoadFood - 10);
        FullStore.Advance(1, 200, 0, 5);
        Step = Run(FullStore, 1, 1, AtBase, pe::kHarvesterStoreRetrySteps);
        assert(Step.delivered == 5 && U.rations == pe::kFoodCapacityRations && U.RationsReconcile());
        // Harvested food ends a shortage the same way bought food does: at the next full issue.
        pe::Ledger Hungry;
        assert(Hungry.AddParticipant(1, pe::kStartingCents, true, 200) && Hungry.SetSource(1, FoodKind, pe::kLoadFood) && Hungry.SetStartingRations(1, 0));
        const pe::Account& Y = *Hungry.Find(1);
        Hungry.Advance(1, 200, 0, 3);
        assert(Y.foodShortage && Hire(Hungry, 1, 1));
        Hungry.StepHarvester(1, 1, AtFood);
        Run(Hungry, 1, 1, AtFood, pe::kHarvesterLoadSteps);
        Hungry.StepHarvester(1, 1, AtBase);
        Run(Hungry, 1, 1, AtBase, pe::kHarvesterUnloadSteps);
        assert(Y.rations == pe::kLoadFood && Y.foodShortage);
        Hungry.Advance(1, 400, 0, 3);
        assert(!Y.foodShortage && Y.rations == pe::kLoadFood - 3 && Y.RationsReconcile());

        // A cleared ledger keeps nothing for the next match.
        Ledger.Clear();
        assert(!Ledger.Find(1) && !Ledger.FindHarvester(1, 10));
    }

    // ECON-E3: one debit per due interval, an atomic purchase, and a shortage with a way out.
    {
        namespace pe = politika::economy;
        using pe::EconomyRejection;
        static_assert(pe::kStartingRations > 0 && pe::kRationsPerUnit > 0 && pe::kFoodPurchaseRations > 0 && pe::kFoodPurchaseCents > 0);
        static_assert(pe::kStartingRations <= pe::kFoodCapacityRations);
        // A shortage is recoverable at the unit limit: fixed income alone can feed a full force.
        static_assert(pe::kFoodUpkeepIntervalSteps == pe::kIncomeIntervalSteps);
        static_assert(20 * pe::kRationsPerUnit * pe::kFoodPurchaseCents <= pe::kIncomeCents * pe::kFoodPurchaseRations);
        constexpr std::uint8_t Rifleman = static_cast<std::uint8_t>(pe::Recipe::Rifleman);
        const pe::Cents Price = pe::FindRecipe(Rifleman)->cost;
        const std::uint32_t Duration = pe::FindRecipe(Rifleman)->durationSteps;
        constexpr pe::Step Interval = pe::kFoodUpkeepIntervalSteps;
        assert(pe::CheckIntentShape(static_cast<std::uint8_t>(pe::EconomyOp::BuyFood), 0) == EconomyRejection::None);
        assert(pe::CheckIntentShape(static_cast<std::uint8_t>(pe::EconomyOp::BuyFood), 1) == EconomyRejection::MalformedCommand);

        pe::Ledger Ledger;
        assert(Ledger.AddParticipant(1, 10 * Price, true, Interval) && Ledger.AddParticipant(2, 10 * Price, true, Interval));
        const pe::Account& A = *Ledger.Find(1);
        const pe::Account& B = *Ledger.Find(2);
        assert(A.rations == pe::kStartingRations && !A.foodShortage && A.RationsReconcile());
        assert(!Ledger.SetStartingRations(1, -1) && !Ledger.SetStartingRations(1, pe::kFoodCapacityRations + 1) && !Ledger.SetStartingRations(9, 1));
        assert(Ledger.SetStartingRations(1, 5) && A.rations == 5 && A.startingRations == 5 && A.RationsReconcile());

        // Nothing is due before the interval, and one due step charges once, however many calls.
        pe::StepResult Early = Ledger.Advance(1, Interval - 1, 0, 3);
        assert(!Early.upkeepDue && A.rations == 5);
        pe::StepResult First = Ledger.Advance(1, Interval, 0, 3);
        Ledger.Advance(2, Interval, 0, 2);
        assert(First.upkeepDue && First.upkeepOwed == 3 && First.upkeepPaid == 3 && A.rations == 2 && !A.foodShortage && A.rationsIssued == 3);
        assert(B.rations == pe::kStartingRations - 2);
        assert(!Ledger.Advance(1, Interval, 0, 3).upkeepDue && A.rations == 2);
        // The next due step counts the force as it is now.
        assert(Ledger.SetStartingRations(1, 1) == false);

        // Training is admitted while fed; the reserve then runs out and stops at zero.
        assert(Ledger.Train(1, 1, Rifleman, 3, 20) && Ledger.Train(1, 2, Rifleman, 3, 20));
        pe::Step Step = Interval + 1;
        Ledger.Advance(1, Step, 0, 3);
        assert(A.queue.front().remainingSteps == Duration - 1);
        const pe::StepResult Short = Ledger.Advance(1, 2 * Interval, 0, 3);
        assert(Short.upkeepDue && Short.upkeepOwed == 3 && Short.upkeepPaid == 2 && A.rations == 0 && A.foodShortage && A.RationsReconcile());
        // A shortage holds the queue from that step, refuses new training and charges nothing.
        assert(Short.headRemainingSteps == Duration - 1 && A.queue.front().remainingSteps == Duration - 1);
        const pe::Cents Held = A.available;
        for (Step = 2 * Interval + 1; Step < 2 * Interval + 20; ++Step) assert(!Ledger.Advance(1, Step, 0, 3).spawnDue);
        assert(A.queue.front().remainingSteps == Duration - 1 && A.available == Held);
        assert(Ledger.Train(1, 3, Rifleman, 3, 20).rejection == EconomyRejection::FoodShortage && A.available == Held && A.lastTransaction == 2);
        // Cancel still refunds, and the other participant is untouched.
        assert(Ledger.CancelLast(1, 3) && A.queue.size() == 1 && A.available == Held + Price);
        assert(!B.foodShortage);
        // A purchase is open during a shortage, atomic and once only, and does not end it yet.
        const pe::Cents BeforeFood = A.available;
        assert(Ledger.BuyFood(1, 3).rejection == EconomyRejection::DuplicateCommand && A.rations == 0 && A.available == BeforeFood);
        const pe::RequestResult Bought = Ledger.BuyFood(1, 4);
        assert(Bought && Bought.amount == pe::kFoodPurchaseCents && A.rations == pe::kFoodPurchaseRations);
        assert(A.available == BeforeFood - pe::kFoodPurchaseCents && A.foodShortage && A.Reconciles() && A.RationsReconcile());
        assert(Ledger.Train(1, 5, Rifleman, 3, 20).rejection == EconomyRejection::FoodShortage);
        assert(!Ledger.Advance(1, Step, 0, 3).spawnDue && A.queue.front().remainingSteps == Duration - 1);
        // The next due debit paid in full ends it, in the same step production resumes.
        const pe::StepResult Recovered = Ledger.Advance(1, 3 * Interval, 0, 3);
        assert(Recovered.upkeepDue && Recovered.upkeepPaid == 3 && !A.foodShortage && A.rations == pe::kFoodPurchaseRations - 3);
        assert(Recovered.headRemainingSteps == Duration - 1 && A.queue.front().remainingSteps == Duration - 2);
        assert(Ledger.Train(1, 6, Rifleman, 3, 20));
        // A smaller force eats less, and an empty one owes nothing and is never short.
        const pe::StepResult Fewer = Ledger.Advance(1, 4 * Interval, 0, 1);
        assert(Fewer.upkeepOwed == 1 && Fewer.upkeepPaid == 1);
        assert(Ledger.SetStartingRations(2, 1) == false);
        const pe::StepResult Nobody = Ledger.Advance(1, 5 * Interval, 0, 0);
        assert(Nobody.upkeepDue && Nobody.upkeepOwed == 0 && !A.foodShortage);

        // Too little money or a full store leaves everything alone.
        pe::Ledger Poor;
        assert(Poor.AddParticipant(1, pe::kFoodPurchaseCents - 1, false, Interval));
        const pe::Account& P = *Poor.Find(1);
        assert(Poor.BuyFood(1, 1).rejection == EconomyRejection::InsufficientFunds && P.rations == pe::kStartingRations && P.spent == 0);
        assert(Poor.BuyFood(9, 1).rejection == EconomyRejection::Unauthorized);
        pe::Ledger Full;
        assert(Full.AddParticipant(1, 10 * pe::kFoodPurchaseCents, false, Interval) && Full.SetStartingRations(1, pe::kFoodCapacityRations - pe::kFoodPurchaseRations + 1));
        const pe::Account& F = *Full.Find(1);
        const pe::Rations FullStore = F.rations;
        assert(Full.BuyFood(1, 1).rejection == EconomyRejection::FoodStoreFull && F.rations == FullStore && F.spent == 0);
        // Food needs no producer: a seat with no base still eats and can still buy.
        assert(!F.hasProducer && Full.SetStartingRations(1, 0) && Full.BuyFood(1, 1) && F.rations == pe::kFoodPurchaseRations);
    }

    // ECON-E2: electricity is capacity and service, and an outage pauses a queue without charging.
    {
        namespace pw = politika::power;
        namespace pe = politika::economy;
        using pe::EconomyRejection;
        static_assert(pw::kGeneratorAuxiliaryKw > 0 && pw::kProducerLoadKw > 0);
        static_assert(pw::kGeneratorCapacityKw >= pw::kGeneratorAuxiliaryKw + pw::kProducerLoadKw);

        // Every building has a positive load, and only a generator has capacity.
        pw::Grid Bad;
        assert(!Bad.Add({0, pw::BuildingKind::Producer, 5, 0}));
        assert(!Bad.Add({1, pw::BuildingKind::Producer, 0, 0}));
        assert(!Bad.Add({1, pw::BuildingKind::Generator, -1, 10}));
        assert(!Bad.Add({1, pw::BuildingKind::Generator, 1, -1}));
        assert(!Bad.Add({1, pw::BuildingKind::Producer, 5, 5}));
        assert(Bad.Empty() && Bad.Add({1, pw::BuildingKind::Producer, 5, 0}) && !Bad.Add({1, pw::BuildingKind::Producer, 5, 0}));
        assert(!Bad.SetGenerating(1, false) && !Bad.SetGenerating(9, false));
        const pw::Allocation NoPlant = Bad.Allocate();
        assert(NoPlant.suppliedKw == 0 && NoPlant.requiredKw == 5 && !Bad.Find(1)->powered);

        // A plant starts from its own output, then lower IDs win and a smaller load still fits.
        pw::Grid Base;
        assert(Base.Add({7, pw::BuildingKind::Producer, 60, 0}) && Base.Add({8, pw::BuildingKind::Producer, 50, 0}));
        assert(Base.Add({9, pw::BuildingKind::Producer, 20, 0}) && Base.Add({3, pw::BuildingKind::Generator, 10, 100}));
        pw::Allocation Shared = Base.Allocate();
        assert(Shared.suppliedKw == 100 && Shared.requiredKw == 140);
        assert(Base.Find(3)->powered && Base.Find(7)->powered && !Base.Find(8)->powered && Base.Find(9)->powered);
        // Offline, nothing is powered, the plant included, and the same inputs give the same result.
        assert(Base.SetGenerating(3, false));
        Shared = Base.Allocate();
        assert(Shared.suppliedKw == 0 && Shared.requiredKw == 140 && !Base.Find(3)->powered && !Base.Find(7)->powered);
        assert(Base.SetGenerating(3, true));
        Shared = Base.Allocate();
        assert(Shared.suppliedKw == 100 && Base.Find(3)->powered && Base.Find(7)->powered && !Base.Find(8)->powered && Base.Find(9)->powered);

        // The starting base is legal: powered from the start, with no prior step.
        constexpr std::uint8_t Rifleman = static_cast<std::uint8_t>(pe::Recipe::Rifleman);
        const pe::Cents Price = pe::FindRecipe(Rifleman)->cost;
        const std::uint32_t Duration = pe::FindRecipe(Rifleman)->durationSteps;
        pe::Ledger Ledger;
        assert(Ledger.AddParticipant(1, 3 * Price, true, 1'000'000) && Ledger.AddParticipant(2, 3 * Price, true, 1'000'000));
        assert(Ledger.AddParticipant(3, 3 * Price, false, 1'000'000));
        const pe::Account& A = *Ledger.Find(1);
        const pe::Account& B = *Ledger.Find(2);
        assert(A.producerPowered && A.allocation.suppliedKw == pw::kGeneratorCapacityKw);
        assert(A.allocation.requiredKw == pw::kGeneratorAuxiliaryKw + pw::kProducerLoadKw);
        // A seat with no base has no grid, and its refusal stays NoProducer.
        assert(!Ledger.SetGeneratorOnline(3, false) && !Ledger.SetGeneratorOnline(9, false));
        assert(Ledger.Train(3, 1, Rifleman, 0, 20).rejection == EconomyRejection::NoProducer);

        assert(Ledger.Train(1, 1, Rifleman, 0, 20) && Ledger.Train(1, 2, Rifleman, 0, 20));
        assert(Ledger.Train(2, 1, Rifleman, 0, 20));
        pe::Step Step = 1;
        assert(!Ledger.Advance(1, Step).powerChanged);
        Ledger.Advance(2, Step);
        assert(A.queue.front().remainingSteps == Duration - 1);

        // An outage takes effect at the next Advance. Until then admission still sees the power.
        assert(Ledger.SetGeneratorOnline(1, false));
        assert(A.producerPowered);
        ++Step;
        const pe::StepResult Off = Ledger.Advance(1, Step);
        Ledger.Advance(2, Step);
        assert(Off.powerChanged && !A.producerPowered && A.allocation.suppliedKw == 0 && Off.headRemainingSteps == Duration - 1);
        // No time passes, no money moves, and the other participant carries on.
        const pe::Cents During = A.available;
        for (int Paused = 0; Paused < 50; ++Paused) {
            ++Step;
            const pe::StepResult Held = Ledger.Advance(1, Step);
            Ledger.Advance(2, Step);
            assert(!Held.spawnDue && !Held.powerChanged);
        }
        assert(A.queue.front().remainingSteps == Duration - 1 && A.available == During && A.Reconciles());
        assert(B.producerPowered && B.queue.front().remainingSteps == Duration - 52);
        // New training is refused with no side effect, and the transaction ID stays usable.
        const pe::RequestResult Refused = Ledger.Train(1, 3, Rifleman, 0, 20);
        assert(Refused.rejection == EconomyRejection::ProducerUnpowered && A.available == During && A.queue.size() == 2 && A.lastTransaction == 2);
        // Cancel still refunds in full, here the waiting item.
        assert(Ledger.CancelLast(1, 3) && A.queue.size() == 1 && A.available == During + Price && A.Reconciles());

        // Recovery resumes the same entry from where it stopped, with no second charge.
        assert(Ledger.SetGeneratorOnline(1, true));
        ++Step;
        const pe::StepResult On = Ledger.Advance(1, Step);
        Ledger.Advance(2, Step);
        assert(On.powerChanged && A.producerPowered && On.headRemainingSteps == Duration - 1 && A.queue.front().remainingSteps == Duration - 2);
        assert(!Ledger.Advance(1, Step).powerChanged);
        pe::StepResult Due;
        std::uint32_t Remaining = A.queue.front().remainingSteps;
        while (Remaining > 0) {
            ++Step;
            Due = Ledger.Advance(1, Step);
            --Remaining;
        }
        assert(Due.spawnDue && Due.transaction == 1 && A.spent == 2 * Price && A.refunded == Price);
        // A finished item with a blocked output does not retry during an outage.
        assert(Ledger.BlockHead(1, 1, Step));
        assert(Ledger.SetGeneratorOnline(1, false));
        for (std::uint32_t Wait = 0; Wait < 2 * pe::kBlockedRetrySteps; ++Wait) assert(!Ledger.Advance(1, ++Step).spawnDue);
        assert(Ledger.SetGeneratorOnline(1, true));
        const pe::StepResult Retry = Ledger.Advance(1, ++Step);
        assert(Retry.powerChanged && Retry.spawnDue && Ledger.CompleteHead(1, 1) && A.produced == 1 && A.Reconciles());
    }

    // ECON-E6: damage takes the generator offline and a dead producer fails its queue once.
    {
        namespace pe = politika::economy;
        namespace pw = politika::power;
        using pe::EconomyRejection;
        constexpr std::uint8_t Rifleman = static_cast<std::uint8_t>(pe::Recipe::Rifleman);
        const pe::Cents Price = pe::FindRecipe(Rifleman)->cost;
        const std::uint32_t Duration = pe::FindRecipe(Rifleman)->durationSteps;
        pe::Ledger Ledger;
        assert(Ledger.AddParticipant(1, 4 * Price, true, 1'000'000));
        const pe::Account& A = *Ledger.Find(1);
        const std::size_t Buildings = A.grid.Count();
        assert(Buildings == 2 && A.grid.Find(pe::kGeneratorBuilding)->health == pw::kGeneratorHealth);
        assert(Ledger.Train(1, 1, Rifleman, 0, 20));
        pe::Step Step = 1;
        assert(Ledger.Advance(1, Step).headRemainingSteps == 0 || A.queue.front().remainingSteps == Duration - 1);
        const std::uint32_t Held = A.queue.front().remainingSteps;
        const pe::Cents Spent = A.spent;
        const auto Scratch = Ledger.DamageBuilding(1, pe::kGeneratorBuilding, 1);
        assert(Scratch.applied == 1 && Scratch.generatorOffline && !Scratch.producerLost);
        assert(A.grid.Count() == Buildings && !A.grid.Find(pe::kGeneratorBuilding)->generating);
        assert(!Ledger.SetGeneratorOnline(1, true));
        const pe::StepResult Off = Ledger.Advance(1, ++Step);
        assert(Off.powerChanged && !A.producerPowered && A.queue.front().remainingSteps == Held);
        // One point of damage takes the plant offline. The building is still there, so its load stays.
        assert(A.allocation.requiredKw == pw::kGeneratorAuxiliaryKw + pw::kProducerLoadKw && A.allocation.suppliedKw == 0);
        const pe::RequestResult Refused = Ledger.Train(1, 2, Rifleman, 0, 20);
        assert(Refused.rejection == EconomyRejection::ProducerUnpowered && A.spent == Spent && A.queue.size() == 1);
        assert(Ledger.RepairBuilding(1, pe::kGeneratorBuilding));
        assert(A.grid.Find(pe::kGeneratorBuilding)->health == pw::kGeneratorHealth);
        const pe::StepResult On = Ledger.Advance(1, ++Step);
        assert(On.powerChanged && A.producerPowered && A.queue.front().transaction == 1
            && A.queue.front().remainingSteps == Held - 1 && A.spent == Spent);

        // The producer is on the grid. Damaging it does not add a building or cut the generator.
        const auto Nick = Ledger.DamageBuilding(1, pe::kProducerBuilding, 1);
        assert(Nick.applied == 1 && !Nick.producerLost && A.grid.Count() == Buildings
            && A.grid.Find(pe::kGeneratorBuilding)->generating && A.hasProducer);
        const pe::Cents BeforeLoss = A.available;
        const auto Lost = Ledger.DamageBuilding(1, pe::kProducerBuilding, pw::kProducerHealth);
        assert(Lost.producerLost && Lost.refunded == Price && A.queue.empty() && !A.hasProducer
            && A.available == BeforeLoss + Price && A.refunded == Price && A.Reconciles());
        const auto Again = Ledger.DamageBuilding(1, pe::kProducerBuilding, pw::kProducerHealth);
        assert(Again.applied == 0 && !Again.producerLost && Again.refunded == 0 && A.refunded == Price);
        assert(Ledger.Train(1, 3, Rifleman, 0, 20).rejection == EconomyRejection::NoProducer);
        // The dead producer stays on the grid and no longer draws its load. The living plant still supplies.
        const pe::StepResult Husk = Ledger.Advance(1, ++Step);
        assert(Husk.powerChanged && A.grid.Count() == Buildings && A.grid.Find(pe::kProducerBuilding)->health == 0
            && !A.grid.Find(pe::kProducerBuilding)->powered
            && A.allocation.requiredKw == pw::kGeneratorAuxiliaryKw && A.allocation.suppliedKw == pw::kGeneratorCapacityKw);
        assert(Ledger.DamageBuilding(1, pe::kGeneratorBuilding, pw::kGeneratorHealth).applied == pw::kGeneratorHealth);
        const pe::StepResult Dark = Ledger.Advance(1, ++Step);
        assert(Dark.powerChanged && A.allocation.requiredKw == 0 && A.allocation.suppliedKw == 0
            && !A.grid.Find(pe::kGeneratorBuilding)->powered);
        assert(Ledger.RepairBuilding(1, pe::kGeneratorBuilding));
        const pe::StepResult Back = Ledger.Advance(1, ++Step);
        assert(Back.powerChanged && A.allocation.requiredKw == pw::kGeneratorAuxiliaryKw
            && A.allocation.suppliedKw == pw::kGeneratorCapacityKw && A.grid.Find(pe::kGeneratorBuilding)->powered);

        // A source, or any id that is not a building, does not join the grid.
        const auto Miss = Ledger.DamageBuilding(1, 99, 50);
        assert(Miss.applied == 0 && A.grid.Count() == Buildings && !Ledger.RepairBuilding(1, pe::kProducerBuilding));
    }

    // ECON-E9: a captured producer refunds once; a second living producer is refused; a generator moves.
    {
        namespace pe = politika::economy;
        constexpr std::uint8_t Rifleman = static_cast<std::uint8_t>(pe::Recipe::Rifleman);
        const pe::Cents Price = pe::FindRecipe(Rifleman)->cost;
        pe::Ledger Ledger;
        assert(Ledger.AddParticipant(1, 4 * Price, true, 1'000'000));
        assert(Ledger.AddParticipant(2, 4 * Price, true, 1'000'000));
        assert(Ledger.Train(2, 1, Rifleman, 0, 20));
        const pe::Account& Loser = *Ledger.Find(2);
        const pe::Account& Winner = *Ledger.Find(1);
        assert(Ledger.CaptureBuilding(2, 1, pe::kProducerBuilding).refused);
        assert(Loser.hasProducer && Loser.queue.size() == 1);
        assert(Ledger.DamageBuilding(1, pe::kProducerBuilding, politika::power::kProducerHealth).producerLost);
        const pe::Cents Before = Loser.available;
        const auto Taken = Ledger.CaptureBuilding(2, 1, pe::kProducerBuilding);
        assert(Taken.captured && Taken.producer && Taken.refunded == Price && Taken.newId == pe::kProducerBuilding);
        assert(!Loser.hasProducer && Loser.queue.empty() && Loser.available == Before + Price && !Loser.grid.Find(pe::kProducerBuilding));
        assert(Winner.hasProducer && Winner.grid.Find(pe::kProducerBuilding)->health == politika::power::kProducerHealth);
        const std::int32_t GeneratorHealth = Loser.grid.Find(pe::kGeneratorBuilding)->health;
        const auto Plant = Ledger.CaptureBuilding(2, 1, pe::kGeneratorBuilding);
        assert(Plant.captured && !Plant.producer && Plant.newId != pe::kGeneratorBuilding && Plant.refunded == 0);
        assert(!Loser.grid.Find(pe::kGeneratorBuilding));
        assert(Winner.grid.Find(Plant.newId)->health == GeneratorHealth && Winner.grid.Find(Plant.newId)->kind == politika::power::BuildingKind::Generator);
        assert(!Ledger.CaptureBuilding(1, 2, 99).captured);
    }

    // ECON-E1: one treasury per participant, income on a fixed cadence, training paid once.
    {
        namespace pe = politika::economy;
        using pe::EconomyRejection;
        constexpr std::int64_t Cap = 20;
        constexpr std::uint8_t Rifleman = static_cast<std::uint8_t>(pe::Recipe::Rifleman);
        const pe::Cents Price = pe::FindRecipe(Rifleman)->cost;
        const std::uint32_t Duration = pe::FindRecipe(Rifleman)->durationSteps;
        static_assert(pe::kStartingCents > 0 && pe::kIncomeCents > 0);
        assert(!pe::FindRecipe(pe::kRecipeCount));

        // Checked arithmetic refuses instead of wrapping.
        pe::Cents Out = 7;
        assert(!pe::CheckedAdd(std::numeric_limits<pe::Cents>::max(), 1, Out) && Out == 7);
        assert(!pe::CheckedSub(std::numeric_limits<pe::Cents>::min(), 1, Out) && Out == 7);
        assert(pe::CheckedAdd(2, 3, Out) && Out == 5);
        assert(pe::CheckedSub(2, 3, Out) && Out == -1);

        assert(pe::FormatUsd(0) == "$0.00" && pe::FormatUsd(5) == "$0.05" && pe::FormatUsd(36'000) == "$360.00");
        assert(pe::FormatUsd(123'456'789) == "$1,234,567.89" && pe::FormatUsd(100'000) == "$1,000.00" && pe::FormatUsd(-250) == "-$2.50");
        assert(pe::FormatUsd(std::numeric_limits<pe::Cents>::min()) == "-$92,233,720,368,547,758.08");

        // Wire shape.
        assert(pe::CheckIntentShape(0, 0) == EconomyRejection::None);
        assert(pe::CheckIntentShape(1, 0) == EconomyRejection::None);
        assert(pe::CheckIntentShape(1, 1) == EconomyRejection::MalformedCommand);
        assert(pe::CheckIntentShape(pe::kEconomyOpCount, 0) == EconomyRejection::MalformedCommand);

        // Accounts: valid owner, no duplicates, no negative start, stable order.
        pe::Ledger Ledger;
        assert(!Ledger.AddParticipant(0, 100, true, 1));
        assert(!Ledger.AddParticipant(9, -1, true, 1));
        assert(Ledger.AddParticipant(2, pe::kStartingCents, true, pe::kIncomeIntervalSteps));
        assert(Ledger.AddParticipant(1, pe::kStartingCents, true, pe::kIncomeIntervalSteps));
        assert(!Ledger.AddParticipant(1, pe::kStartingCents, true, pe::kIncomeIntervalSteps));
        assert(Ledger.All().size() == 2 && Ledger.All()[0].participant == 1 && Ledger.All()[1].participant == 2);

        const auto Same = [](const pe::Account& A, const pe::Account& B) {
            return A.available == B.available && A.spent == B.spent && A.credited == B.credited
                && A.refunded == B.refunded && A.queue.size() == B.queue.size()
                && A.lastTransaction == B.lastTransaction && A.produced == B.produced;
        };

        // USD-01 / PRD-05: a purchase debits once and creates one queue entry.
        const pe::RequestResult First = Ledger.Train(1, 1, Rifleman, 2, Cap);
        assert(First && First.amount == Price && First.available == pe::kStartingCents - Price && First.queued == 1);
        assert(Ledger.Find(1)->Reconciles());
        // The other participant's money is untouched.
        assert(Ledger.Find(2)->available == pe::kStartingCents && Ledger.Find(2)->queue.empty());

        // Rejections change nothing (USD-01, PRD-04, PRD-09).
        pe::Account Snapshot = *Ledger.Find(1);
        assert(Ledger.Train(1, 1, Rifleman, 2, Cap).rejection == EconomyRejection::DuplicateCommand);
        assert(Ledger.Train(1, 0, Rifleman, 2, Cap).rejection == EconomyRejection::DuplicateCommand);
        assert(Ledger.Train(1, 2, pe::kRecipeCount, 2, Cap).rejection == EconomyRejection::UnknownRecipe);
        assert(Ledger.Train(7, 2, Rifleman, 2, Cap).rejection == EconomyRejection::Unauthorized);
        assert(Ledger.Train(1, 2, Rifleman, Cap - 1, Cap).rejection == EconomyRejection::UnitCap);
        assert(Ledger.CancelLast(1, 1).rejection == EconomyRejection::DuplicateCommand);
        assert(Ledger.CancelLast(7, 2).rejection == EconomyRejection::Unauthorized);
        assert(Same(*Ledger.Find(1), Snapshot));
        // A rejected transaction ID stays usable: only an admitted one is spent.
        assert(Ledger.Train(1, 2, Rifleman, 2, Cap));
        Snapshot = *Ledger.Find(1);
        assert(Snapshot.available == pe::kStartingCents - 2 * Price && Snapshot.available < Price);
        assert(Ledger.Train(1, 3, Rifleman, 2, Cap).rejection == EconomyRejection::InsufficientFunds);
        assert(Same(*Ledger.Find(1), Snapshot));

        // Cancel refunds exactly what the newest entry paid, and only once.
        const pe::RequestResult Cancelled = Ledger.CancelLast(1, 3);
        assert(Cancelled && Cancelled.amount == Price && Cancelled.queued == 1);
        assert(Ledger.Find(1)->refunded == Price && Ledger.Find(1)->Reconciles());
        assert(Ledger.CancelLast(1, 3).rejection == EconomyRejection::DuplicateCommand);
        assert(Ledger.Find(1)->refunded == Price);

        // Income is due on its exact steps and is paid once per interval, in any call pattern.
        pe::Ledger Paid;
        assert(Paid.AddParticipant(1, 0, false, 200));
        for (pe::Step Step = 1; Step < 200; ++Step) assert(Paid.Advance(1, Step).income == 0);
        assert(Paid.Advance(1, 200).income == pe::kIncomeCents);
        assert(Paid.Advance(1, 200).income == 0);
        for (pe::Step Step = 201; Step < 400; ++Step) assert(Paid.Advance(1, Step).income == 0);
        assert(Paid.Advance(1, 400).income == pe::kIncomeCents);
        assert(Paid.Find(1)->credited == 2 * pe::kIncomeCents && Paid.Find(1)->available == 2 * pe::kIncomeCents);
        assert(Paid.Find(1)->Reconciles());
        // A producer-less participant has money but cannot train.
        assert(Paid.Train(1, 1, Rifleman, 0, Cap).rejection == EconomyRejection::NoProducer);

        // Income that would overflow is refused, not wrapped, and the next interval still comes.
        pe::Ledger Full;
        assert(Full.AddParticipant(1, std::numeric_limits<pe::Cents>::max(), false, 1));
        const pe::StepResult Overflowing = Full.Advance(1, 1);
        assert(Overflowing.incomeOverflow && Overflowing.income == 0);
        assert(Full.Find(1)->available == std::numeric_limits<pe::Cents>::max() && Full.Find(1)->credited == 0);
        assert(Full.Find(1)->nextIncomeStep == 1 + pe::kIncomeIntervalSteps);

        // PRD-06: training counts authoritative steps from the step after admission; only the
        // head advances, and a second item waits for the first.
        pe::Ledger Queue;
        assert(Queue.AddParticipant(1, 5 * Price, true, 1'000'000));
        assert(Queue.Train(1, 1, Rifleman, 0, Cap));
        assert(!Queue.Advance(1, 10, 1).spawnDue);
        assert(Queue.Find(1)->queue[0].remainingSteps == Duration);
        assert(Queue.Train(1, 2, Rifleman, 0, Cap));
        for (std::uint32_t Elapsed = 1; Elapsed < Duration; ++Elapsed) assert(!Queue.Advance(1, 10 + Elapsed).spawnDue);
        assert(Queue.Find(1)->queue[1].remainingSteps == Duration);
        const pe::StepResult Due = Queue.Advance(1, 10 + Duration);
        assert(Due.spawnDue && Due.transaction == 1 && Due.recipe == pe::Recipe::Rifleman);

        // A blocked output keeps its place and its payment, retries on a cadence and holds the rest.
        const pe::Cents BeforeBlock = Queue.Find(1)->available;
        assert(!Queue.BlockHead(1, 2, 10 + Duration));
        assert(Queue.BlockHead(1, 1, 10 + Duration));
        for (std::uint32_t Wait = 1; Wait < pe::kBlockedRetrySteps; ++Wait) assert(!Queue.Advance(1, 10 + Duration + Wait).spawnDue);
        assert(Queue.Find(1)->queue[1].remainingSteps == Duration);
        assert(Queue.Advance(1, 10 + Duration + pe::kBlockedRetrySteps).spawnDue);
        assert(Queue.Find(1)->available == BeforeBlock && Queue.Find(1)->queue.size() == 2);
        assert(!Queue.CompleteHead(1, 2));
        assert(Queue.CompleteHead(1, 1));
        assert(Queue.Find(1)->produced == 1 && Queue.Find(1)->available == BeforeBlock && Queue.Find(1)->Reconciles());
        assert(Queue.Find(1)->queue.size() == 1 && Queue.Find(1)->queue[0].transaction == 2);
        // Cancelling a blocked or training head refunds it too.
        assert(Queue.CancelLast(1, 3) && Queue.Find(1)->queue.empty() && Queue.Find(1)->Reconciles());
        assert(Queue.CancelLast(1, 4).rejection == EconomyRejection::NothingToCancel);

        // The queue is bounded, and queued items count towards the unit cap.
        pe::Ledger Rich;
        assert(Rich.AddParticipant(1, 100 * Price, true, 1'000'000));
        pe::TransactionId Next = 1;
        for (std::size_t Index = 0; Index < pe::kMaxQueue; ++Index) assert(Rich.Train(1, Next++, Rifleman, 0, Cap));
        assert(Rich.Train(1, Next, Rifleman, 0, Cap).rejection == EconomyRejection::QueueFull);
        assert(Rich.Find(1)->queue.size() == pe::kMaxQueue && Rich.Find(1)->spent == static_cast<pe::Cents>(pe::kMaxQueue) * Price);
        pe::Ledger NearCap;
        assert(NearCap.AddParticipant(1, 100 * Price, true, 1'000'000));
        assert(NearCap.Train(1, 1, Rifleman, Cap - 2, Cap));
        assert(NearCap.Train(1, 2, Rifleman, Cap - 2, Cap));
        assert(NearCap.Train(1, 3, Rifleman, Cap - 2, Cap).rejection == EconomyRejection::UnitCap);

        // PRD-06: equal request sequences give equal traces, whatever the call schedule around them.
        const auto Trace = [&](int Variant) {
            pe::Ledger Run;
            assert(Run.AddParticipant(1, pe::kStartingCents, true, pe::kIncomeIntervalSteps));
            std::vector<pe::Cents> Balances;
            for (pe::Step Step = 1; Step <= 900; ++Step) {
                pe::TransactionId Admitted = 0;
                if (Step == 5 || Step == 6) {
                    Admitted = static_cast<pe::TransactionId>(Step);
                    assert(Run.Train(1, Admitted, Rifleman, 2, Cap));
                }
                if (Variant == 1) (void)Run.Find(1); // reads never change the account
                const pe::StepResult Result = Run.Advance(1, Step, Admitted);
                if (Result.spawnDue) assert(Run.CompleteHead(1, Result.transaction));
                Balances.push_back(Run.Find(1)->available);
            }
            assert(Run.Find(1)->produced == 2 && Run.Find(1)->Reconciles());
            return Balances;
        };
        assert(Trace(0) == Trace(1));

        // PRD-10: clearing leaves nothing behind for the next match.
        Ledger.Clear();
        assert(Ledger.All().empty() && !Ledger.Find(1));
        assert(Ledger.AddParticipant(1, pe::kStartingCents, true, pe::kIncomeIntervalSteps));
        assert(Ledger.Find(1)->available == pe::kStartingCents && Ledger.Find(1)->queue.empty() && Ledger.Find(1)->lastTransaction == 0);

        // Output candidates: a fixed order, two rings, the first along the requested direction.
        const auto Offsets = pe::OutputOffsets({0.0, 1.0});
        assert(Offsets[0].x == 0.0 && Offsets[0].y == 250.0);
        assert(std::abs(Offsets[2].x + 250.0) < 1e-9 && std::abs(Offsets[2].y) < 1e-9);
        assert(std::abs(std::hypot(Offsets[8].x, Offsets[8].y) - 400.0) < 1e-9);
        for (std::size_t Left = 0; Left < Offsets.size(); ++Left)
            for (std::size_t Right = Left + 1; Right < Offsets.size(); ++Right)
                assert(std::hypot(Offsets[Left].x - Offsets[Right].x, Offsets[Left].y - Offsets[Right].y) > 100.0);
    }

    return 0;
}
