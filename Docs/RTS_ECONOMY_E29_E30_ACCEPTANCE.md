# ECON-E29 and ECON-E30 — Look through units, and crude oil: acceptance record

**Date:** 3 November 2026 · **Scope:** two slices qualified together on one package: `82495f0` (ECON-E29) and `adf3339` (ECON-E30), with the harness at `39dab65`, which changes no package input.

**Outcome: IMPLEMENTED and ACCEPTED for the skirmish test map. The packaged `--oil` session passes 10/10 in two runs with identical traces, `--works` 12/12 and `--buildings` 20/20 twice with identical traces, all on real input with two rendered remote clients. Automation 62/62 with no failures. The regression matrix has one flag against ECON-E28, `MP2-FOG-EXPLORED-PERSISTS` in `default-fog-memory`, which went from FAIL to PASS and which earlier records call timing-dependent and not a gate. The owner paused the matrix once and it resumed on the same packages. The prices and quantities are fixture values, and an idle harvester still cannot be clicked at its parking spot (section D).**

## A. What the slices added

**ECON-E29, C and Z look through units.** A unit passing the cursor used to take the press: the client refused it without telling the server, and the cursor text went blank.

- C, Z and the cursor text now find the building or works behind a unit.
- Each press logs `RTS_BUILDING_PRESS`, with `through_unit=1` when it looked through a unit, and the cursor logs `RTS_BUILDING_CURSOR_THROUGH`.
- `press_on_building` in the harness presses once, so a press that does nothing now fails instead of being retried.

**ECON-E30, crude oil.**

- **One finite field per seat, behind its base.** Each field is visible only to its own side. The default is 400 units, and the `--oil` session sets seat 1's to 35 with `-PolitikaMPEconomySourceOil`.
- **A harvester goes there only when sent.** A right-click on the field sends it, and it keeps going until the field runs out. When the field is empty it moves on to gold, then food, if any is left. It never moves on to oil by itself from gold or food. A load is 20 units.
- **Crude has its own stock.** Nothing turns it into fuel or gold.
- **O sells one lot.** A lot is 20 units, or what is left, at $1.50 a unit less 3%, at the server's quote. 20 units gross $30.00, less a $0.90 fee, for $29.10. 15 units gross $22.50, less $0.68, for $21.82.
- **Crude alone is enough to train a harvester.** K is refused only when both caches and the field are empty.
- **The HUD has a crude line:** stock, what is left in the field, what is on the way, and either the O quote or `nothing to sell yet`.

`network_protocol` 24 to 25.

## B. What qualification found

| Finding | Cause | What was done |
|---|---|---|
| **`trial-1` (`6ba1eaa`): `OIL-P00` failed with `InvalidLaunchOptions=true`** | `-PolitikaMPEconomySourceOil` was not in the launch-option parser's list | Added to the parser and its test |
| **`a68754b` did not link** | `ld.lld` reported an undefined `APolitikaPrototypeMatch::PlaceWorks`, in both the client and server package builds. Nothing ran on it | Replaced by `a7a2616`, which links. Neither commit left this machine |
| **`trial-2` (`a7a2616`): no field appeared** | `RTS_OIL_START` logged `has_source=0`. The base is near a map corner, and 1500 cm directly behind it is off the map, and the wider search below found a position, so the earlier candidates were evidently refused | The search now tries 1500, 1200 and 1000 cm, and adds ±45°, in `adf3339`. The field lands at (-1644.8, -1849.4), mirrored for seat 2 |
| **`trial-3` (`adf3339`): the harness could not select the harvester** | An idle harvester parks at the producer's centre, where a click finds the building | The harness takes it with a small box (`39dab65`). **The game is unchanged,** so a player has the same problem (section D) |
| **`trial-4` and `trial-5`: `--works` 8/11 on `WRK-P10`, which also failed the steps after it** | Zoomed out, the unit is a few pixels tall, and aim points low on its body see the ground | `WRK-P10` zooms in and sweeps the unit (`39dab65`). `trial-6` passes 12/12. **The look-through is proven on the unit's body, not at every zoom and aim point** (section F) |
| **Automation 62/63 on an earlier tree (`auto-1`, `auto-2`)** | A new `BuildingThroughUnits` test failed: the fixture world has no physics scene, so the trace finds nothing, and its own control check showed that | The test was removed. **ECON-E29 has no Automation,** only the packaged `WRK-P10` and `WRK-P11` |
| **The matrix stopped during `group-move-shape-2`** | The owner paused it at about 19:30. Not a test failure | `qualify-resume-adf3339.sh` ran that step onward from 19:48 on the same packages and harness, after checking their hashes, and the gate check still ran. The partial run is in `interrupted-adf3339/`. Steps finished before it stand |
| **The comparer exits 1 and flags `default-fog-memory`** | `MP2-FOG-EXPLORED-PERSISTS` went from FAIL at ECON-E28, with (422, 200, 7122) after the retreat, to PASS, with (408, 214, 7122). It is the only flag, and it is an improvement the comparer counts as a change | Not a gate, as in the [ECON-E28 record](RTS_ECONOMY_E28_ACCEPTANCE.md). It was not rerun, and this pass does not show it is fixed |
| **`listen-none` and `profile-lag-loss` exit 1** | The three expected `NOT_RUN` checks, the same as at ECON-E28: `MP2-PROTOCOL-LEGACY-REFUSED` twice, and `MP2-1V1-LAG-LOSS-FOG-ALLIED-SHARED` | None. `matrix-none` covers the legacy refusal, with the ECON-E4 client refused and passing |

## C. Verification

**Identity.** Packages from clean commit `adf3339` (`git_dirty=false`, `network_protocol` 25, engine 5.8.3). Client `dev-adf3339/…/Politika` `523bd1b3ee8fa90f96647a43537e2eb1287dc2e901cd8fcb7ce6cadf1419ac1c`; server `server-adf3339/…/PolitikaServer` `4aa797f91a93d4699cec6f73fda1a5afd883b1f79cc5cf3a41c54962a4ffc0c6`, checked before each part of the matrix. `39dab65` changes only `Tools/Shell/src/bin/rts_multiplayer.rs`. A release rebuild of the harness at `39dab65` matches the hashes of the binaries the matrix used. Automation ran with the editor library built from `adf3339` at 18:58, and the tree was clean when the resumed matrix started and is clean now.

| Check | Result |
|---|---|
| Portable C++20 suite (`tools/core-test.sh`), on a clean export of `39dab65` | PASS |
| `cargo fmt --check` (Tools/Shell); Tools/Shell tests (26: 14 rts_multiplayer, 12 rts_interaction); `quality -- core`; `quality -- audit` | PASS |
| `tools/version`: `check`, `cargo test --all-targets` (7 and 14), `cargo fmt --check` | PASS; reports protocol 25 |
| Automation, all Politika tests (`auto-final/`) | 62/62, 0 failures, 0 not run, with the new `Oil`. All 62 finish with warnings, as ECON-E28's 61 did |
| `--oil` ×2 | 10/10 twice, identical `oil-trace.txt` |
| `--works` ×2 | 12/12 twice, identical `works-trace.txt` |
| `--buildings` ×2 | 20/20 twice, identical `buildings-trace.txt` |
| `--harvest` ×2, `--gold` ×2, `--food` ×2, `--power` ×2, `--economy` ×2 | 14/14, 14/14, 13/13, 12/12, 16/16 twice, each with identical traces |
| `--economy-limits` ×2, `--pointer-stall` ×2 | 5/5 and 25/25 twice |
| `--group-move-facing` ×2, `--group-move-shape` ×2 | 54/54 and 47/47 twice |
| `--unit-scale` | 83/83, including every step-budget check: at 3v1 with 20 a seat, engage p99 1.2 ms and longest step 1.9 ms against the 50 ms budget, no catch-up frames |
| Launch options | 0 sessions flag `InvalidLaunchOptions` |
| Whole matrix against ECON-E28 (`compare-matrix.py`, `compare-adf3339.txt`) | 1 flagged, `default-fog-memory`, as in section B. Only `--works` gained checks, `WRK-P10` and `WRK-P11`. The three expected `NOT_RUN` checks are unchanged |

**The `--oil` session.**

| Check | What it shows |
|---|---|
| `OIL-P01-A-FIELD-FOR-EACH-BASE-OWN-SIDE-ONLY` | Each seat has a field on its own side, at mirrored positions |
| `OIL-P02-O-REFUSES-WITH-NOTHING-AT-THE-BASE` | O with no crude is refused by the server |
| `OIL-P03-A-HARVESTER-DOES-NOT-GO-THERE-BY-ITSELF` | A harvester delivering gold does not move on to oil |
| `OIL-P04-SENT-THERE-IT-HAULS-CRUDE-TO-ITS-OWN-STOCK` | Sent there, it loads 20 and delivers to the crude stock |
| `OIL-P05-O-SELLS-A-LOT-AT-THE-QUOTE` | 20 units gross $30.00, fee $0.90, net $29.10 |
| `OIL-P06-THE-FIELD-RUNS-OUT-AND-IT-STOPS` | It keeps going, takes the last 15, and waits |
| `OIL-P07-THE-STOCKS-STAY-APART` | Selling gold does not touch crude |
| `OIL-P08-OWNER-ONLY` | Neither client receives the other's oil, and seat 2's oil does not change |
| `OIL-P09-LEDGER-WHOLE-AND-THE-NEXT-MATCH-FRESH` | The ledger reconciles and generation 2 has fresh fields |
| `OIL-P00-LAUNCH-OPTIONS` | No session flags its launch options |

**The look-through.** `WRK-P10` presses C through a unit and the hold starts, logging `through_unit=1`. `WRK-P11` is its control: a unit over bare ground finds no building and nothing is captured. Each run of `--buildings` also logged one press with `through_unit=1`, in a session that does not aim for it. That fits the cause ECON-E27 inferred for `BLD-P10`, without proving it.

One screenshot was read, `oil-1/client-a-03-crude-sold.png`. It shows `CRUDE OIL 0 · 15 in the field, 0 on the way · nothing to sell yet` and `20 crude oil sold for $29.10`, matching the trace. Log checks stand for the rest.

## D. Decisions for the owner

- **An idle harvester cannot be clicked where it parks.** It stands at the producer's centre, where a click finds the building, so a player needs a box select. The harness works around this and the game does not. It was seen with the harvester in `--oil`, and other idle harvesters were not checked. The choice is between parking it elsewhere and making a click prefer the unit.
- **The field is not 1500 cm behind the base.** The base is near a map corner, so the field landed at (-1644.8, -1849.4). That is a fixture-map result, and a real map would need an authored position.
- **$1.50 a unit, 3%, 400 units and a load of 20 are placeholders.** A full field is worth about $582 net. Gold sells at $2.50 a unit less the same 3%, so crude is worth less per unit.
- **Crude has no use except sale.** The [economy design](RTS_OPERATIONAL_ECONOMY_AND_POWER.md) keeps refining and fuel as separate work.

## E. Map to the acceptance gates

| ID | Status in this increment |
|---|---|
| USD-02 | Partial: gold and crude oil. Gas and refined fuel do not exist yet |
| USD-03 | Partial: fee, rounding, overflow and nonexistent goods are covered for crude, as for gold. Quote expiry and reservation do not exist |
| USD-04 | Met by absence: gold and crude oil are the only saleable goods |
| USD-05 | Met for crude sale proceeds and the next generation |

## F. Not covered

- The look-through at other zoom levels and aim points. `WRK-P10` zooms in and sweeps because a low aim point on a zoomed-out unit sees the ground, and that case is not fixed by this slice.
- Automation of the look-through, since the fixture world has no physics scene.
- More than one harvester on a field, allies with oil, and a harvester lost while carrying crude in a packaged session.
- The same play under packet loss or latency.
- The six navigation harnesses, which were not rerun, as in ECON-E5 to E28.
- Multiple-machine play, Shipping builds, a real desktop display and platforms other than Linux.
