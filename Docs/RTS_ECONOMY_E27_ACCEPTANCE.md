# ECON-E27 — Repair takes a unit and takes time: acceptance record

**Date:** 3 November 2026 · **Scope:** one slice (`ce48eb9`), two harness fixes that qualification needed (`e92fd0b`, `ec094bc`). It answers the first decision in the [ECON-E6 to E26 record](RTS_ECONOMY_E6_E26_ACCEPTANCE.md#d-decisions-for-the-owner), where one right-click undid any damage from anywhere.

**Outcome: IMPLEMENTED and ACCEPTED for the skirmish test map. On the final harness, the packaged `--buildings` session passes 20/20 in two runs with identical traces, real input on two rendered remote clients; Automation 59/59 with no failures; the regression matrix has no new failure against the [ECON-E6 to E26](RTS_ECONOMY_E6_E26_ACCEPTANCE.md) matrix. One gate run failed on the same packages (`BLD-P10`, [section B](#b-what-qualification-found)). Its cause is inferred from timing and code. The harness retry written for it never fired in the passing runs, so neither the cause nor the fix has been shown by a rerun. Repair is still free.**

## A. What the slice added

- **Repair needs a unit.** A right-click on your damaged generator sends the nearest selected unit that can move, or keeps the one already on it. It walks to within 2 m.
- **Repair takes time.** It restores one point a step at 20 Hz, so a husk takes 5 s and a scratch far less. Nothing is restored from a distance.
- **The plant stays offline until it is full.** It is back on its load as soon as it has any health, and supplies nothing until 100/100, so training stays refused.
- **The work can end early.** An order, leaving the 2 m ring or a hit on the unit ends it, and the health restored so far stays. A hit on the plant takes only its own points, and the unit stays on the job. A plant captured during the work ends it too.
- **One unit at a time.** A second unit is refused on a plant that has a repairer, with `Another unit is already repairing that generator.`
- **Harvesters can do it**, so a side with no soldiers left can restore its power. A harvester that takes the job leaves its routine, and its queued trip is dropped.
- **Reading it.** The cursor reads `moving to repair`, then `repairing`, and the unit's order label reads `Repairing`. `RTS_BUILDING_REPAIR` gains the phases `approach`, `hold`, `work`, `hit` and `end`.

`ServerRepairBuilding` becomes `ServerBeginRepair`, and the unit replicates its repair target and step count. `network_protocol` 22 to 23. The range and rate are fixture values, not balance decisions.

## B. What qualification found

Neither gate failure is a fault in the slice as far as the evidence shows. Both needed a harness fix, and neither fix changed the game.

| Finding | Cause | Fix |
|---|---|---|
| **First `--buildings` on `ce48eb9`: 18 pass, `BLD-P19` and `BLD-P09` fail** (`final-ce48eb9/buildings-1`). The gate stopped the run before the matrix | `MP_CLIENT_SAMPLE` logs once a second, so `order_attack` aimed its first click at where defender unit 3 had been while walking. It took five right-clicks and 13.4 s to target unit 3, and the attacker shot the generator meanwhile, so the repair ran 269 steps | `e92fd0b`: wait for a sample logged after the call, Stop a stray order before the next try, and have `BLD-P09` count the second repair from the health the first one kept. On the `ce48eb9` packages, which share their source inputs with `e92fd0b`, the first click hit unit 3 and the session passed 20/20 (`harness-trial-ce48eb9/`). The Stop retry path did not run there |
| **`--buildings` run 2 on `e92fd0b`: 14 pass, `BLD-P10` and `SESSION-buildings` fail** (`final-e92fd0b/buildings-2`). Run 1 had passed 20/20. `BLD-P12` to `P16` did not run, which accounts for 5 of the 8 regressions on the first comparison | C began no capture: the server log has no `RTS_BUILDING_CAPTURE` after the press, and the 40 s wait ran out. Unit 1 stayed in the enemy base, and enemy unit 18 killed it at 14:58:29, so the next selection failed with `could not select entity 1`. **Why C began nothing is inferred.** The press is not logged. The client's cursor text was blank from 14:58:15.436 to 15.636 between two lines naming the enemy generator, and `CaptureUnderCursor` makes no server call when something other than a building is under the cursor. The likely passer is enemy harvester 16 | `ec094bc`: `press_on_building` presses C or Z again until the server logs the start, up to five presses, and records the count as an evidence fact |
| **The first matrix attempt stopped partway through** at 16:15, during `discipline-stress` | The exec running it hit a 30 minute limit. This is a tooling limit, not a game fault | `qualify-resume-e92fd0b.sh` ran `discipline-stress` onward on the same packages and harness. The partial attempt is kept in `interrupted-e92fd0b/`. Steps finished before it stand |

**What the rerun does and does not show.** Both final runs logged `capture_generator_presses`, `capture_producer_presses`, `demolish_generator_presses` and `demolish_producer_presses` as 1, so the retry never fired. The runs show that the session passes on the final harness, with the same trace as the earlier 20/20 run. They do not reproduce the failure or exercise the fix. `BLD-P10` has now passed in 3 of 4 runs on these packages, and the true rate of that failure is unknown. If `BLD-P10` fails again with `capture_generator_presses` at 5, the cause above is wrong.

## C. Verification

**Identity.** Packages from clean commit `e92fd0b` (`git_dirty=false`, `network_protocol` 23, engine 5.8.3). Client `dev-e92fd0b/…/Politika` `96a1179c8e9cbffaadd36e9ecd8c4f89b04528377b7e53f9a85f243f7414e116`; server `server-e92fd0b/…/PolitikaServer` `41db1829705bdabdc4f4dd512ad1c8c7226cf596698db3ca84de39fa7f3b61a3`, checked before and after the reruns. `ec094bc` changes only the harness, so the packages stand: the hash of `Source`, `Config`, `Content`, `Politika.uproject` and `version.json` is the same at `ce48eb9`, `e92fd0b` and `ec094bc`. The matrix ran on the `e92fd0b` harness (`bin/`) and the two final `--buildings` runs on the `ec094bc` harness (`bin-ec094bc/`). Evidence directory: `/mnt/data/politika-qualification/ECON-E27-20261103/` (local to the development machine).

Automation ran with the editor library built at 14:54:59 from `ce48eb9`, which has the same source inputs. Uncommitted work on the next slice was in the tree by then and was not compiled into it. No mining service or miner process was present when the final runs started at 16:42; the host was not sampled for this during the matrix itself.

| Check | Result |
|---|---|
| Portable C++20 suite (`tools/core-test.sh`), on a clean checkout | PASS |
| `cargo fmt --check` (Tools/Shell); Tools/Shell tests (26: 14 rts_multiplayer, 12 rts_interaction); `quality -- core`; `quality -- audit` | PASS |
| Automation, all Politika tests (`auto-final/`) | 59/59, 0 failures, 0 not run, with the new `BuildingRepair`. All 59 finish with warnings, as ECON-E6 to E26's 58 did |
| `--buildings` ×2, final harness (`buildings-retry-ec094bc/`) | 20/20 twice, identical `buildings-trace.txt`, also identical to the 20/20 run in `final-e92fd0b/buildings-1` |
| `--buildings` on the `e92fd0b` harness (`final-e92fd0b/`) | Run 1 20/20; run 2 **failed** as in section B |
| `--harvest` ×2, `--gold` ×2, `--food` ×2, `--power` ×2, `--economy` ×2 | 14/14, 14/14, 13/13, 12/12, 16/16 twice, each trace identical between runs and to the ECON-E6 to E26 baseline |
| `--economy-limits` ×2, `--pointer-stall` ×2 | 5/5 and 25/25 twice |
| `--group-move-facing` ×2, `--group-move-shape` ×2 | 54/54 and 47/47 twice |
| `--unit-scale` | 83/83: worst engage-phase step p99 1.5 ms against the 50 ms budget, longest step 2.4 ms, no catch-up frames |
| Launch options | 0 of 222 logs flag `InvalidLaunchOptions` |
| Whole matrix against ECON-E6 to E26 (`compare-matrix.py`) | `compare-e92fd0b.txt`: 8 flagged, being the failed `buildings-2` (7) and `default-fog-memory` (1). `compare-e92fd0b-retry.txt`, with the final runs in place of the `buildings` steps: 1 flagged, `default-fog-memory`. The three expected `NOT_RUN` checks are unchanged |

The one remaining flag is not a new failure. `MP2-FOG-EXPLORED-PERSISTS` went from FAIL to PASS, and the comparer counts any change in a non-PASS status. `--fog-memory` with the free harvester on was 7/8 at the baseline and is 8/8 here. It is not a gate, and the ECON-E6 to E26 record already notes that it depends on timing, so this is not evidence of a fix.

**The `--buildings` session, what changed.** The other checks read as in the [ECON-E6 to E26 table](RTS_ECONOMY_E6_E26_ACCEPTANCE.md#c-verification).

| Check | What it shows |
|---|---|
| `BLD-P04-A-RIGHT-CLICK-REPAIRS-YOUR-GENERATOR` | The right-click walks a unit over, the cursor reads `repairing`, training is refused during the work, and it is accepted again once the plant is full |
| `BLD-P18-AN-ORDER-STOPS-THE-REPAIR-AND-KEEPS-THE-HEALTH` | **X** ends the work, the plant keeps what it had (for example 20), and the hint comes back |
| `BLD-P19-A-HIT-ON-THE-UNIT-STOPS-THE-REPAIR` | Another seat shoots the repairer: one hit ends the work, and the health stays |
| `BLD-P09-ITS-OWNER-REPAIRS-IT` | The second repair takes exactly the steps the first one left (for example 35, then 65), and its owner queues two units on the restored power |
| `BLD-P10-C-CAPTURES-THE-GENERATOR` and the other C and Z checks | Unchanged in meaning. The presses now go through `press_on_building` and are counted |

Automation `BuildingRepair` (BR-01 to BR-14) covers the refusals, one point a step, one unit at a time, an order, leaving and a hit each ending the work, a hit on the plant during it, offline until full, a scratch, no route, the walk in, a harvester, and capture during the work. Portable rules cover the amounts, the cap and a switched-off plant.

## D. Decisions for the owner

- **Repair is still free.** It now costs a unit's time and exposes that unit, but no money. A cost is a separate decision.
- **A unit passing under the cursor takes a C or Z press.** The player sees `Point at a hostile building to capture it.` and nothing happens. The harness now presses again, but a player meets the same thing. Pressing C or Z on the building behind a passing unit would remove it, as a separate change.
- **A hit ends the work, but the plant being shot does not.** A repairer can stand and restore a generator while another unit fires at the plant, at 1 point a step against about 8 points per 0.2 s. These are prototype figures.

## E. Map to the acceptance gates

| ID | Status in this increment |
|---|---|
| PWR-03 | Partial, as before: damage, capture and repair recover deterministically by real input. A side with no soldiers can now restore its power with a harvester, in Automation only. Reconnection during an outage is not exercised |
| ECO-FX-04 | Partial for a real outage, as before: training is refused during the work and accepted after it, by real damage and repair |

## F. Not covered

- The `press_on_building` retry, which has not fired in a packaged session. The `order_attack` Stop retry did not run in its trial and was not checked separately in later runs.
- Leaving the ring, a harvester repairing, a second unit refused, a hit on the plant during the work, and capture during the work have Automation only. No packaged session plays them.
- No negative control was run for `BLD-P18` or `BLD-P19`. The `ce48eb9` failure shows that `BLD-P19` and `BLD-P09` can fail, but for a harness reason.
- A listen host repairing, the same work under packet loss or latency, and more than two seats.
- The six navigation harnesses, which were not rerun, as in ECON-E5 and ECON-E6 to E26.
- Multiple-machine play, Shipping builds, a real desktop display and platforms other than Linux.
