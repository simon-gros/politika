# ECON-E5 — A free harvester, gold and food sources, and the owner fix: acceptance record

**Date:** 2 November 2026 · **Scope:** a follow-up to [ECON-E4](RTS_ECONOMY_E4_ACCEPTANCE.md) by owner decision: each side starts with one harvester, and a harvester can collect different resources, including food. It also fixes the defect recorded in [section D of the ECON-E4 record](RTS_ECONOMY_E4_ACCEPTANCE.md#d-known-defect-produced-units-have-no-network-owner).

**Outcome: IMPLEMENTED and ACCEPTED for the skirmish test map. Packaged harvest check 14/14 in two runs with identical normalized traces, by real input on a dedicated 1v1 with the normal start; Automation 53/53 with no failures; regression matrix on a quiet host matches the accepted ECON-E4 matrix with 0 regressions. The regression sessions pin their forces with the free harvester off; [section C](#c-verification) says what happens with it on.**

## A. What changed (commit `cfb41cd`)

- **Free harvester.** Each seat with a base starts with one, placed at the producer's output. It costs nothing, does not go through the queue, and sets off for gold as soon as the match is active. It counts towards food and the unit limit like one you train, so the opening force of two becomes three.
- **Food source** (`Source/PolitikaCore/Economy.h`). Sources are now typed. Each seat has a food source of 600 rations, a green slab on the other side of its start from the gold, reaching its own team only under fog. The two always stand at least 6 m apart, so a harvester is only ever at one.
- **Choosing a resource.** There is no new order. A harvester's target is whichever source it last arrived at under a player's Move, so a right-click on a cache chooses it. A load of the other resource goes home first, then it goes for the new target.
- **Food delivery.** 30 rations a trip, in the same 3 s and 2 s as gold. Food goes straight into the ration reserve, which now reconciles as starting + bought + harvested = held + issued. What a full store cannot take stays on the harvester, which keeps unloading until there is room. **N** still buys food.
- **Running out.** When a source empties, the harvester moves on to the other one by itself, and waits when both are empty. **K** is refused as `no_resource_source` only when both are empty.
- **Box selection** (`Source/PolitikaCore/GroupSelection.h`). A box over soldiers and non-combat units takes the soldiers, so a drag across a base does not pull a harvester off its route. A box over only non-combat units still takes them, and Ctrl+drag still removes whatever it covers.
- **Owner fix.** Every unit that appears after its participant bound now gets its network owner, so a trained unit's order, ammunition, ability timers, stance, resolve, rout state and category reach its owner, as an authored unit's always did.
- **View and HUD.** The owner-only view gains food carried and at the source. The food line gains `60 in the cache, 0 on the way`, the gold line reads `1 harvester`, and a carrying harvester's label says `GOLD` or `FOOD`.
- **Network and diagnostics.** `network_protocol` 13 to 14. `RTS_HARVEST_TX` replaces `RTS_GOLD_TX` and names its resource, plus a `retarget` kind. New: `RTS_FOOD_SOURCE_START`, `RTS_FOOD_SOURCE_RELEVANT`, `RTS_HARVESTER_START`, `RTS_HARVESTER_CARRY_CLIENT`, a `harvest` kind on `RTS_FOOD_TX`, and `ammo` and `carried` on `MP_CLIENT_SAMPLE`. `MATCH_CONTENT_READY` keeps counting the map's authored units.
- **Switches.** `-PolitikaMPEconomySourceFood=<participant>:<rations>` and `-PolitikaMPStartingHarvesters=0` (non-Shipping). Scripted acceptance runs and the two navigation fixture maps have no starting harvester, because they pin exact forces and timing. The navigation harnesses were not rerun for this slice.
- **Tests.** Portable cases in `Tests/Core/RulesTests.cpp`, Automation `Politika.RTS.Multiplayer.Harvest`, and the `--harvest` packaged session.

## B. Decisions taken

- **Harvesters collect food, by owner decision.** The design documents ruled out food collection as a required chore. The owner decided on 2 November 2026 that a harvester can collect different resources, including food, and the [Resource Economy](RESOURCE_ECONOMY.md) and [economy contract](RTS_OPERATIONAL_ECONOMY_AND_POWER.md) now say so. Food is still never a required chore, since N stays open and the fixed income can feed a full force.
- **A free harvester, by owner decision.** It collects gold first, because gold is what pays for everything else.
- **Gold and food only.** Crude oil and gas fit the same typed source, but each also needs a stock and a sale rule. That is a separate slice.
- **It moves on by itself.** A harvester that has run out goes to the other source, because an idle collector is the chore the design wants to avoid.
- **Harvested food follows the same recovery rule as bought food.** It ends a shortage at the next due issue paid in full, not on delivery.
- **Fixture figures** (prototype data, not balance decisions): food source 600, load 30.

## C. Verification

**Identity.** Packages from clean commit `cfb41cd` (`git_dirty=false`, `network_protocol` 14, engine 5.8.3). Client `dev-cfb41cd/…/Politika` `c80271ef589ac5458349be6db3f609e64f33354541bb6b90366a2d57a0b34c16`; server `server-cfb41cd/…/PolitikaServer` `c70e7db7e9036a5ddbe97de54348725e65d61a0accadf9a048ced8131a4df329`. Evidence directory: `/mnt/data/politika-qualification/ECON-E5-20261102/` (local to the development machine). The CPU and GPU mining services were off for the whole matrix.

| Check | Result |
|---|---|
| Portable C++20 suite (`tools/core-test.sh`) | PASS |
| `cargo fmt --check` (Tools/Shell); Tools/Shell tests (26: 14 rts_multiplayer, 12 rts_interaction); `quality -- core`; `quality -- audit` | PASS |
| Automation, all Politika tests (`auto-all-final/`) | 53/53, 0 failures, 0 not run: 52 before plus `Harvest` |
| `--harvest` ×2 (`final-cfb41cd/`) | 14/14 twice, identical `harvest-trace.txt`; a gate run on the same packages also 14/14 |
| `--gold` ×2, `--food` ×2, `--power` ×2, `--economy` ×2 | 14/14, 13/13, 12/12, 16/16 twice, each trace identical between runs and to the ECON-E4 baseline |
| `--economy-limits` ×2, `--pointer-stall` ×2 | 5/5 and 25/25 twice |
| `--group-move-facing` ×2, `--group-move-shape` ×2 | 54/54 and 47/47 twice; facing plans byte-identical |
| `--unit-scale` | 83/83: worst engage-phase step p99 1.5 ms against the 50 ms budget, longest step 2.4 ms, no catch-up frames |
| Launch options | 0 of 284 logs flag `InvalidLaunchOptions` |
| Whole matrix against ECON-E4 (`compare-matrix.py`) | 0 regressions, including the three expected `NOT_RUN` checks and the protocol-13 client refused in `matrix-none` |

**The `--harvest` session, by check.** The normal start, with seat 1's food source reduced to 60 through the fixture.

| Check | What it shows |
|---|---|
| `HARV-P01-CACHES-AND-A-FREE-HARVESTER` | Both sources and one harvester per seat, at their expected places, with no training line and full treasuries |
| `HARV-P02-CACHES-OWN-TEAM-ONLY` | Each client receives only its own two sources |
| `HARV-P03-IT-WORKS-WITH-NO-INPUT` | Its first gold load is delivered with nothing pressed |
| `HARV-P04-OWNER-FIELDS-REACH-THE-OWNER` | The owner's client receives the harvester's load and category, and a trained rifleman's ammunition. This is the fix |
| `HARV-P05-THE-ENEMY-GETS-NONE-OF-IT` | The other client receives no load updates for units it does not own |
| `HARV-P06-A-BOX-TAKES-SOLDIERS` | A real drag over the whole screen selects the three soldiers, not the harvester |
| `HARV-P07-THE-FOOD-CACHE-TAKES-OVER` | A real right-click on the food cache during a gold load moves no gold and makes food its target |
| `HARV-P08-FOOD-REACHES-THE-RESERVE` | Two food trips, each crediting the reserve once |
| `HARV-P09-IT-MOVES-ON-WHEN-THE-CACHE-EMPTIES` | With the food gone, it goes back to gold by itself |
| `HARV-P10-OWNER-ONLY` | Neither client receives the other's view |
| `HARV-P11-BOTH-RESOURCES-REPLAY` | Gold and food lines follow exactly from the one before, for both seats |
| `HARV-P12-RATIONS-AND-MONEY-REPLAY` | The reserve and the money ledger replay, and a sale pays the quote |
| `HARV-P13-NEXT-GENERATION-FRESH` | A soldier takes the objective with the harvesters still working, and the next generation has its own |
| `HARV-P00-LAUNCH-OPTIONS` | Every switch the session passes is recognized |

Screenshots `client-a-02-carrying.png` and `-03-food-delivered.png` show the green cache, the `GOLD` tag, `MORALE 50` and `Moving` on the selected harvester. All three were missing or wrong in the ECON-E4 screenshots. Automation and the portable suite also cover a food load going home first, a dead harvester's food, a full store, food-only and empty seats, and a harvested food delivery that waits for the next full issue to end a shortage.

**With the free harvester on.** The regression sessions pin their forces, so they do not exercise it. Run with it on (`POLITIKA_HARNESS_DEFAULT_FORCES=1`), on the trial packages unless noted:

| Session | Result | Why |
|---|---|---|
| `--fog-memory`, clean package | 7/8. `MP2-FOG-EXPLORED-PERSISTS` fails | The harvester at the gold cache sees the militia again, so the client clears that contact with `reason=reobserved`. A trial run passed 8/8, so it depends on timing |
| `--dedicated` | 21/23. `MP3-FOG-INITIAL` and `MP3-NEG-PROBES` fail | The first pins the exact unit list. In the second, the probes pick a unit that turns out to be the harvester, which is refused as `CannotAttack` first |
| `--engagement` | 8/10. `MP2-SHOT-CUE-FACING-OVER-SOCKETS` and `MP2-ENGAGE-FOG-MEMORY` fail | The session assumes team B never sees a militia, and team B's harvester now does |

These checks assume a force that stays put. None is in the accepted matrix. They are not gates for this slice, and they show the session assumptions, not a rule failure. The fog cases are also a real gameplay change: **a harvester's vision scouts**, so its route shows what stands near a cache.

## D. Map to the acceptance gates

| ID | Status in this increment |
|---|---|
| FOOD-01 | Met, with a second real route: harvested as well as bought |
| ECO-FX-03 | Partial: two harvesters cannot take the same load, and same-step sales cannot sell the same gold |
| ECO-INT-01 | Partial: two packaged runs give identical normalized traces on one machine. Different render schedules were not compared directly |
| ECO-INT-03 | Met for the fixture: the route runs itself from the first step |
| PRD-03 | Met for both resources |

## E. Not covered

- Crude oil, gas and any other resource.
- An authored harvester vehicle, or an authored cache: these are coloured blocks from fixture values.
- A map-authored cache position: both come from a fixed offset around the start.
- The opponent AI's use of harvesters, since no opponent AI exists yet.
- Balance review of the prototype figures, or of how far a harvester's vision should reach.
- Multiple-machine play, Shipping builds, a real desktop display and platforms other than Linux.
