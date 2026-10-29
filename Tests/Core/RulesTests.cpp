#include "Rules.h"

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
    TeamLayout Unparsed = TeamLayout::TwoVsOne;
    assert(!ParseTeamLayout("", Unparsed) && !ParseTeamLayout("4v1", Unparsed) && !ParseTeamLayout("1v1 ", Unparsed)
        && !ParseTeamLayout("3v2", Unparsed) && Unparsed == TeamLayout::TwoVsOne);

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
