#include "Rules.h"
#include "Metrics.h"
#include "Fog.h"
#include "UnitScale.h"
#include "GroupSelection.h"
#include "FormationGeometry.h"
#include "Formation.h"
#include "Stance.h"
#include "Discipline.h"

#include <cassert>
#include <iterator>
#include <limits>

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
        static_assert(NextShape(Shape::Line) == Shape::Column && NextShape(Shape::Column) == Shape::Line);
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
        assert(CheckIntentShape({0, 0, 2, Two}) == FormationRejection::MalformedCommand);
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
    return 0;
}
