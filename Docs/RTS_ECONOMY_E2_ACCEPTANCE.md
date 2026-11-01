# ECON-E2 — Building electricity: acceptance record

**Date:** 1 November 2026 · **Scope:** the second sub-slice of Stage 4B in the [Engine Outline](ENGINE_OUTLINE.md): a positive electrical load on every building, one self-starting generator per seat, and a producer that pauses without power, as designed in §4 of [Operational Economy and Power](RTS_OPERATIONAL_ECONOMY_AND_POWER.md) and in [Production and Technology](RTS_PRODUCTION_AND_TECHNOLOGY.md). It builds on [ECON-E1](RTS_ECONOMY_E1_ACCEPTANCE.md).

**Outcome: IMPLEMENTED and ACCEPTED for the skirmish test map, with one limit: nothing in play can cause an outage yet. Buildings cannot be damaged, so in a normal match the base stays powered, and every outage below comes from a development fixture. Packaged power check 11/11 in two runs with identical normalized traces, by real input on a dedicated 1v1; Automation 50/50 with no failures; regression matrix on a quiet host matches the accepted ECON-E1 matrix. Stage 4B is not complete: food, a source/delivery route and the commodity sale stay open.**

## A. What changed (commit `c65019a`)

- **Grid** (`Source/PolitikaCore/Power.h`, no Unreal types). One binary grid per account. Every building has a positive load, a generator's auxiliary load included, and only a generator has capacity. Each step, generators are served first so a plant can start from its own output, then every other building, each group in ascending ID. A building gets its whole load or nothing, and a load that does not fit does not stop a smaller one behind it. Electricity holds no stock and moves no money.
- **Fixture figures** (prototype data, not balance decisions). Generator 100 kW capacity and 10 kW auxiliary load; producer 40 kW. A seat's base therefore shows 100 kW supplied against 50 kW required.
- **Base.** A narrower, darker placeholder generator stands beside each producer. It has the producer's limits: no health, capture or collision, relevant to its own team only under fog. A seat that cannot place both has no base and no producer, so the impossible case of a producer with no power source cannot start.
- **Unpowered producer.** New training is refused as `producer_unpowered` with no charge. The whole queue holds: no training time passes, a finished item with a blocked output does not retry, and nothing is charged again. Cancel still refunds in full, and income continues. Recovery resumes the same entry from where it stopped.
- **View and HUD.** The owner-only view gains supply, load and powered status. The HUD money line gains `POWER 100 / 50 kW` and turns red with `PAUSED: no power` during an outage, and a refused purchase says `the training building has no power`.
- **Network and diagnostics.** `network_protocol` 10 to 11. `RTS_POWER_START`, `RTS_POWER_STATE`, and new fields on `RTS_ECONOMY_VIEW` and `RTS_STRUCTURE_RELEVANT`.
- **Fixture.** `-PolitikaMPPowerOutage=<offset>:<duration>` (non-Shipping): the generator of the first participant to buy goes offline `offset` steps after that purchase, for `duration` steps.
- **Tests.** Portable grid and ledger cases in `Tests/Core/RulesTests.cpp`, Automation `Politika.RTS.Multiplayer.Power`, and the `--power` packaged session. `ECON-P02` now expects both buildings per client.

## B. Decisions taken

- **Step order.** Requests, then income, then power allocation, then production, inside the existing economy phase, after casualties and before the objective. Admission therefore sees the previous step's allocation: a purchase admitted in the step an outage begins is charged, and then pauses like any other item. It can be cancelled for a full refund.
- **Unpowered training is refused, not queued.** [Production and Technology](RTS_PRODUCTION_AND_TECHNOLOGY.md) makes an operational powered producer part of admission, and a refusal keeps the money with the player.
- **The outage comes from a fixture.** The design calls for generator damage or sabotage, which needs building health and targeting that do not exist yet. No player-facing switch was invented to stand in for them.
- **Binary power.** Brownout stays optional until binary on/off has been tested, as the design says.

## C. Verification

**Identity.** Packages from clean commit `c65019a` (`git_dirty=false`, `network_protocol` 11, engine 5.8.3). Client `dev-c65019a/…/Politika` `40467273084fd944e68df0b573a350f98f5c9b756c1e69466a6f5f1a9bbe2e12`; server `server-c65019a/…/PolitikaServer` `566206d0acd34e60821da962ce8b240fb20ade8dc99b09da4d83679f28be28bd`. Evidence directory: `/mnt/data/politika-qualification/ECON-E2-20261101/` (local to the development machine). The CPU and GPU mining services were stopped for the whole matrix, so its timing results come from a quiet host.

| Check | Result |
|---|---|
| Portable C++20 suite (`tools/core-test.sh`) | PASS |
| `cargo fmt --check` (Tools/Shell); Tools/Shell tests (24: 13 rts_multiplayer, 11 rts_interaction); `quality -- core`; `quality -- audit` | PASS |
| Automation, all Politika tests (`auto-all-final/`) | 50/50, 0 failures, 0 not run: 49 at ECON-E1 plus `Power`. Warnings match ECON-E1 apart from one `IsUsingWayland` notice each on `Power` and `StraightPathResultProbe` |
| `--power` ×2 (`final-c65019a/`) | 11/11 twice, identical `power-trace.txt`; a gate run on the same packages also 11/11 |
| `--economy` ×2 | 15/15 twice, identical `economy-trace.txt` |
| `--economy-limits` ×2 | 5/5 twice |
| `--pointer-stall` ×2 | 24/24 twice |
| `--group-move-facing` ×2 | 53/53 twice, normalized plans byte-identical |
| `--unit-scale` | 73/73: worst engage-phase step p99 1.5 ms against the 50 ms budget, longest step 5.4 ms, no catch-up frames |
| Every other regression session | Every check matches the ECON-E1 matrix by stable check ID (`compare-matrix.py`), including the three expected `NOT_RUN` checks in `listen-none` and `profile-lag-loss`, and the protocol-10 client refused in `matrix-none` |

`compare-matrix.py` labels any change `REGRESSION`. Against the `final-731d3db` ECON-E1 matrix it labels six checks that way, and all six went from FAIL to PASS: the five `unit-scale` timing checks that failed under CPU contention there, and `ECON-P03`, whose check was corrected after that matrix.

**The `--power` session, by check.** With the fixture at 60 and 120 steps, the first purchase in the first run was admitted at step 170.

| Check | What it shows |
|---|---|
| `PWR-P01-BASE-STARTS-POWERED` | Both seats start at 100 kW supplied and 50 kW required, with positive loads on both buildings |
| `PWR-P02-BUILDINGS-OWN-TEAM-ONLY` | Each client receives its own generator and producer and neither of the other seat's |
| `PWR-P03-OUTAGE-PAUSES-THE-QUEUE` | The outage starts exactly 60 steps after the purchase, with 0 kW supplied and 2 items held |
| `PWR-P04-UNPOWERED-REFUSES-TRAINING` | A real B press is refused as `producer_unpowered` and charges nothing |
| `PWR-P05-CANCEL-REFUNDS-IN-AN-OUTAGE` | A real Shift+B during the outage refunds $360.00 |
| `PWR-P06-RECOVERY-RESUMES-THE-ENTRY` | Power returns after exactly 120 steps with the same 141 steps remaining as when it stopped |
| `PWR-P07-UNIT-LATE-BY-THE-OUTAGE` | The unit appears 320 steps after admission: 200 of training plus 120 of outage |
| `PWR-P08-TRAINING-WORKS-AFTER-RECOVERY` | A later purchase is admitted and the ledger reconciles |
| `PWR-P09-OWNER-ONLY-OUTAGE` | The other seat's power never changes, and neither client receives the other's view |
| `PWR-P10-NO-SECOND-CHARGE` | Replaying the server's ledger lines gives 3 purchases and 1 refund, with income on its cadence through the outage |
| `PWR-P11-NEXT-GENERATION-FRESH` | After a match end the next generation starts powered, with no leftover power change |

Screenshots `client-a-02-outage.png`, `-03-refused.png` and `-04-recovered.png` show the red HUD line with the paused timer, the refusal message, the recovered line, the new unit and both buildings.

**Failed attempts.** Automation `auto-1` failed one assertion, `P-A09 late by the outage`: the unit appeared at the correct step, and the test's expected value had one step too many. It was corrected before the feature commit; `auto-2` passes.

## D. Map to the acceptance gates

| ID | Status in this increment |
|---|---|
| PWR-01 | Met for the fixed base: positive minimum load on every building and a self-starting generator |
| PWR-02 | Met through the fixture: an insufficient supply stops only the producer's functions, and the HUD states the cause. It cannot arise in normal play yet |
| PWR-03 | Open: damage, capture and repair do not exist. Only recovery after a fixture outage is shown, deterministically and with no softlock |
| PRD-13 | Partial: positive load, valid supply, and pause and resume without a second charge, through the fixture only |
| ECO-FX-04 | Met through the fixture |
| ECO-INT-01 | Partial: two packaged runs give identical normalized traces on one machine. Different render schedules were not compared directly |

## E. Not covered

- Any in-game cause of an outage: building health, damage, sabotage, capture and repair.
- Brownout, more than one generator or consumer in play, power trading, fuel and grid connections. The portable grid handles several buildings, but the game places exactly two per seat.
- Powered functions other than training, such as radar, communications and defences.
- Food and upkeep, a source/delivery route, the commodity sale and the opponent AI's use of power.
- Multiple-machine play, Shipping builds, a real desktop display and platforms other than Linux.
