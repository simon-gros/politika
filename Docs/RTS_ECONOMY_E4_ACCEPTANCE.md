# ECON-E4 — Harvesters, gold delivery and sale: acceptance record

**Date:** 2 November 2026 · **Scope:** the fourth sub-slice of Stage 4B in the [Engine Outline](ENGINE_OUTLINE.md): one finite source per seat, a harvester that hauls from it to the base, and a sale that turns the gold into dollars, as designed in §2, §3 and §8 of [Operational Economy and Power](RTS_OPERATIONAL_ECONOMY_AND_POWER.md). It builds on [ECON-E1](RTS_ECONOMY_E1_ACCEPTANCE.md), [ECON-E2](RTS_ECONOMY_E2_ACCEPTANCE.md) and [ECON-E3](RTS_ECONOMY_E3_ACCEPTANCE.md).

**Outcome: IMPLEMENTED and ACCEPTED for the skirmish test map, with one recorded defect ([section D](#d-known-defect-produced-units-have-no-network-owner)). Packaged gold check 14/14 in two runs with identical normalized traces, by real input on a dedicated 1v1; Automation 52/52 with no failures; regression matrix on a quiet host matches the accepted launch-switch matrix with 0 regressions. The core contracts the Engine Outline lists for 4B now pass on the fixture; the open gates are in [section E](#e-map-to-the-acceptance-gates).**

## A. What changed (commit `1530190`)

- **Source** (`Source/PolitikaCore/Economy.h`). Each seat with a base has one finite gold source of 500, a low gold slab to one side of its start that turns grey when empty. It reaches its own team only under fog, and is not a building: it has no electrical load and no grid entry.
- **Harvester.** **K** trains one at the producer for $300.00 in 15 s, through the same queue, refund and unit limit as a rifleman. An empty or missing source refuses it as `no_gold_source`. It is an unarmed non-combat placeholder vehicle, built from fixture values in code rather than an authored asset. It has 400 health, can be shot, eats like any crewed unit, and neither takes nor contests a control point.
- **Routine.** With no further input it drives to the source, loads 25 gold in 3 s, drives to the producer, unloads in 2 s and repeats. Its moves go through the same command path as a player's Move. Gold leaves the source when loading ends, so two harvesters cannot take the same gold: units step in ascending order and the later one takes what is left.
- **Player control.** A player's admitted order takes it off the routine with its load kept. X holds it where it is. Arriving at the source with room, or at the base with a load, sends it back to work. There is no new order type.
- **Losses.** A dead harvester's load is lost and does not return to the source.
- **Sale.** **L** sells one lot of 25 or what is left, at a fixed quote of $2.50 a unit less a 3% fee, so 25 gold pays $60.62. It debits gold and credits net dollars in one step, or changes nothing, through the same intake, sequence check and step as training. It stays open during an outage or a food shortage.
- **Ledger.** All the source's gold is always at the source, on a harvester, lost, held or sold (`GoldReconciles`), and the money reconciliation gains sale proceeds as a separate term.
- **View and HUD.** The owner-only view gains gold held, carried and at the source, the harvester count and the recipe in training. A new HUD line reads `GOLD 15  ·  0 in the cache  ·  1 harvesters, 0 on the way  ·  L sell 15 for $36.37  ·  cache empty`.
- **Network and diagnostics.** `network_protocol` 12 to 13. `RTS_GOLD_START`, `RTS_GOLD_TX`, `RTS_GOLD_SOURCE_RELEVANT`, new fields on `RTS_ECONOMY_VIEW`, `RTS_ECONOMY_CLEARED` and `RTS_PRODUCTION_SPAWN`, and a `sale` kind on `RTS_ECONOMY_TX`.
- **Fixture.** `-PolitikaMPEconomySourceGold=<participant>:<gold>` (non-Shipping).
- **Tests.** Portable cases in `Tests/Core/RulesTests.cpp`, Automation `Politika.RTS.Multiplayer.Gold`, and the `--gold` packaged session.

## B. Decisions taken

- **A dedicated harvester, by owner decision.** The first version let any unit collect by walking past the source. The owner replaced it with a harvester unit on 2 November 2026, before anything was published. That version (`6c73a94`) was never pushed.
- **The sale key is L.** The command card labels Move as `[M]`, and the [keymap](RTS_UI_SETTINGS_AND_KEYMAP.md) reserves M for a later map screen.
- **No free starting harvester in this slice.** It keeps existing matches unchanged, so the regression comparison stays clean. The owner has since asked for one per side, as the next slice.
- **The fee rounds up to a whole cent**, against the seller, so splitting a sale can never earn more than selling it whole. A portable assertion holds this.
- **The quote is fixed and settles at once.** There is no quote expiry or reservation step, because nothing else can use the gold yet. Both become necessary once refining or player trade exists.
- **Step order.** Requests, then harvesters, income, power, food upkeep and production. Gold delivered in a step can be sold from the next one.
- **Fixture figures** (prototype data, not balance decisions): source 500, load 25, 3 s and 2 s, $300.00 and 15 s, 400 health, 280 cm/s.

## C. Verification

**Identity.** Packages from clean commit `1530190` (`git_dirty=false`, `network_protocol` 13, engine 5.8.3). Client `dev-1530190/…/Politika` `c49512221fb6f1f1bd3b39f7a0b8a6ab8bdfdbd8726d9d8e96d414bac9451a37`; server `server-1530190/…/PolitikaServer` `925df49f48fdc5d7f56bcb27ff079c92aa33ffa376ea5aa7106f3e37fd8333a7`. Evidence directory: `/mnt/data/politika-qualification/ECON-E4-20261102/` (local to the development machine). The CPU and GPU mining services were off for the whole matrix.

| Check | Result |
|---|---|
| Portable C++20 suite (`tools/core-test.sh`) | PASS |
| `cargo fmt --check` (Tools/Shell); Tools/Shell tests (26: 14 rts_multiplayer, 12 rts_interaction); `quality -- core`; `quality -- audit` | PASS |
| Automation, all Politika tests (`auto-all-final/`) | 52/52, 0 failures, 0 not run: 51 before plus `Gold` |
| `--gold` ×2 (`final-1530190/`) | 14/14 twice, identical `gold-trace.txt`; a gate run on the same packages also 14/14 |
| `--food` ×2, `--power` ×2, `--economy` ×2 | 13/13, 12/12, 16/16 twice, each trace identical between runs and to the launch-switch baseline |
| `--economy-limits` ×2, `--pointer-stall` ×2 | 5/5 and 25/25 twice |
| `--group-move-facing` ×2, `--group-move-shape` ×2 | 54/54 and 47/47 twice; facing plans byte-identical |
| `--unit-scale` | 83/83: worst engage-phase step p99 1.6 ms against the 50 ms budget, longest step 2.7 ms, no catch-up frames |
| Launch options | 0 of 271 logs flag `InvalidLaunchOptions` |
| Whole matrix, 34 sessions, against the launch-switch matrix (`compare-matrix.py`) | 0 regressions, including the three expected `NOT_RUN` checks and the protocol-12 client refused in `matrix-none` |

**The `--gold` session, by check.** Seat 1's source holds 40 through the fixture, one load and a part load. Seat 2 keeps the normal 500.

| Check | What it shows |
|---|---|
| `GOLD-P01-SOURCES-AT-THE-START` | 40 and 500, at their expected, mirrored places |
| `GOLD-P02-SOURCE-OWN-TEAM-ONLY` | Each client receives only its own source |
| `GOLD-P03-NOTHING-TO-SELL-YET` | A real L press is refused as `nothing_to_sell` with no gold line |
| `GOLD-P04-K-TRAINS-A-HARVESTER` | A real K press debits $300.00, and the harvester appears exactly 300 steps later |
| `GOLD-P05-THE-FIRST-TRIP-DRIVES-ITSELF` | One load out and home with no further input, at no more than its speed, with its full loading and unloading times |
| `GOLD-P06-A-LOT-AT-THE-QUOTE` | L sells 25 for $62.50 less $1.88, crediting $60.62 once |
| `GOLD-P07-A-PLAYER-ORDER-PARKS-IT` | A right-click during loading takes it off the routine before any gold moves, and it stays where it was sent |
| `GOLD-P08-THE-CACHE-SENDS-IT-BACK` | A right-click on the source resumes it. The last load is the 15 left, and the routine ends when it is home |
| `GOLD-P09-WHAT-IS-LEFT-THEN-NOTHING` | L sells the 15, then is refused, and K is refused as `no_gold_source` |
| `GOLD-P10-OWNER-ONLY` | Seat 2 has no gold lines, and neither client receives the other's view |
| `GOLD-P11-GOLD-REPLAYS` | Every gold line follows exactly from the one before |
| `GOLD-P12-MONEY-REPLAYS` | The money ledger replays with exactly two sales |
| `GOLD-P13-NEXT-GENERATION-FRESH` | After a match end the next generation starts from full sources with no gold lines |
| `GOLD-P00-LAUNCH-OPTIONS` | Every switch the session passes is recognized |

Automation also covers what the packaged session does not: a rifleman at the source takes nothing, a malformed sale, an attack order refused with the harvester never firing, X holding it, several sales in one step, a harvester taking no control point, and a carrier's death. The portable suite covers two harvesters at one source, a treasury overflow, and selling during a shortage and an outage.

**Failed attempts.** Automation caught a real defect before the commit: a stopped harvester counted as still moving, because a stop leaves `Stop` as the current order, so it could never go back to work. Two trial `--gold` runs then failed on harness faults: a session selected the moving harvester at a stale position and parked it too late, and a `kind=load` match also caught `kind=loading`. All of this came before the commit.

## D. Known defect: produced units have no network owner

**Fixed by [ECON-E5](RTS_ECONOMY_E5_ACCEPTANCE.md).**

Screenshot `client-a-02-carrying.png` shows the harvester with 25 gold on the way and no `GOLD` tag on its label, and `client-a-05-source-empty.png` shows it labelled `DISCIPLINE` and `Idle`, not `MORALE`. The cause predates this slice. Owner-only unit fields replicate only to an actor's network owner, and `RefreshUnitNetOwnership` sets that when a participant binds. A unit produced later never gets one.

So since ECON-E1, the owning client has not received the order, ammunition, ability timers, stance, resolve, rout state or category of a **produced** unit, including riflemen. Server authority is unaffected: orders, combat and the ledger are correct. No check looked at these fields on a produced unit, so the matrix did not catch it. The HUD's gold totals come from the view and are correct. It is the first fix in the next slice, with a packaged check.

## E. Map to the acceptance gates

| ID | Status in this increment |
|---|---|
| PRD-03 | Met: a load is delivered once, and a unit that is not carrying delivers nothing |
| USD-02 | Partial: gold only. Crude oil, gas and refined fuel do not exist yet |
| USD-03 | Partial: fee, rounding, overflow and nonexistent goods are covered. Quote expiry and reservation do not exist |
| USD-04 | Met by absence: gold is the only saleable good |
| USD-05 | Met for sale proceeds and the next generation |
| ECO-FX-02 | Met for an instant fixed quote, with no reservation step |
| ECO-FX-03 | Partial: same-step sales cannot sell the same gold. Nothing else competes for gold yet |
| ECO-FX-06 | Met for gold: a match end clears it, and the next generation starts fresh |
| ECO-INT-01 | Partial: two packaged runs give identical normalized traces on one machine. Different render schedules were not compared directly |
| ECO-INT-03 | Met for the fixture: the route runs itself once a harvester exists |
| PWR-03, PRD-08, PRD-12 | Open: buildings cannot be damaged or captured, so nothing in play can cause an outage yet |

## F. Reference checkpoint

**Question.** How should a mature RTS shape a resource collector: a dedicated unit or any unit, and how much does the player drive it?

| Evidence | Class | Disposition |
|---|---|---|
| Tempest `api.txt`: `Command Type Harvest`, `Command Type Auto Harvest`, `Command Type Move Harvester Refinery`, `Status Mutable Harvester Idle`, `Status Mutable Autoharvesting` | documented | adopt-concept: a dedicated collector with an automatic loop and a distinct idle state |
| `art-of-tempest-rising.txt`, GDF Harvester and Refinery: unarmed yet resilient, carries one load per trip, and each refinery includes one harvester | documented, art book, not a rules source | adopt-concept for unarmed and per-trip. The free harvester is the next slice |
| `Def_GDF_Unit_Harvester`, `Def_DYN_Unit_Harvester` in the asset index | registry-only | No semantics taken |

**Not transferred:** values, names, assets, and separate harvest order types. POLITIKA keeps a plain Move, so the slice adds no order to the protocol. **Limits:** nothing here was observed at runtime. **Owner:** this record and §8 of the [economy contract](RTS_OPERATIONAL_ECONOMY_AND_POWER.md).

## G. Not covered

- A free starting harvester, and other resources including food, which the owner has asked for as the next slice.
- An authored harvester vehicle: this one is a coloured block from fixture values in code.
- Crude oil, gas, refined fuel, a trade panel, quote expiry and player trade.
- Any balance review of the prototype figures.
- The opponent AI's use of gold, since no opponent AI exists yet.
- Multiple-machine play, Shipping builds, a real desktop display and platforms other than Linux.
