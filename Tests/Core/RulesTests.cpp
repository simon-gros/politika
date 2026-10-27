#include "Rules.h"

#include <cassert>
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
