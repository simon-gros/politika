#include "Rules.h"

#include <cassert>
#include <iterator>
#include <limits>

using namespace politika::rules;

int main() {
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
