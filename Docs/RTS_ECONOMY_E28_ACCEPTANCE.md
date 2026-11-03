# ECON-E28 — One neutral works: acceptance record

**Date:** 3 November 2026 · **Scope:** one slice (`bf98330`). It adds the first income a side can win from the map, because securing the objective ends the match and so cannot be an income source.

**Outcome: IMPLEMENTED and ACCEPTED for the skirmish test map. The packaged `--works` session passes 10/10 in two runs with identical traces, real input on two rendered remote clients; `--buildings` 20/20 twice with identical traces; Automation 61/61 with no failures. The regression matrix has one flag against ECON-E27, `MP2-FOG-EXPLORED-PERSISTS` in `default-fog-memory`, which two earlier records already list as timing-dependent and not a gate. The matrix was interrupted once by a failed agent run and resumed on the same packages (section B). The income figure is a placeholder.**

## A. What the slice added

- **One neutral works per match.** It stands to the side of the middle of the map, as far from one start as from the opposite one, and out of weapon range of the objective and the militia. Fog hides it until a side can see it.
- **C takes it,** with the same walk and 3 s hold as a building. The other seat can take it back. An ally's works is refused.
- **It pays its holder $10.00 on each 10 s income step,** on top of base income. What it has paid stays when it changes hands, and only future income moves.
- **It is not a building.** It has no health and no grid entry. It cannot be attacked, demolished or repaired, and a right-click on it is a move.
- **Reading it.** The cursor reads `Neutral works`, `Your works` or `Enemy works` and says what it pays. The HUD income line includes it.
- **A rematch starts it neutral.**

`network_protocol` 23 to 24. The amount, position and block size are fixture values, not balance decisions.

## B. What qualification found

| Finding | Cause | What was done |
|---|---|---|
| **The first works stood where the militia and objective could reach it** (`d91e78c`, `trial-1`, which passed 10/10) | At 1100 cm from the centre it was in weapon range, so a hold there could meet more than the other side | Amended to `bf98330`: 1800 cm from the centre, a purple neutral tint so it reads apart from buildings, and the harness points to match. `d91e78c` never left this machine |
| **The matrix stopped at 17:30** during `harvest-2` | The agent run holding it failed at 17:30:01, exactly an hour after that run started, and took the matrix down. The hour suggests a limit on scheduled runs, but the log only says the run failed. Not a game fault | `qualify-resume-bf98330.sh` ran `harvest-2` onward on the same packages and harness, detached so a failed run cannot end it. The partial `harvest-2` is kept in `interrupted-bf98330/`. Steps finished before it stand |
| **Nobody restarted it for 33 minutes** | The scheduled check that ran at 17:30 looked only for ECON-E27, which was already pushed, and nothing else was watching | Found when the owner asked for status |
| **The comparer flags `default-fog-memory`** | `MP2-FOG-EXPLORED-PERSISTS` went from PASS to FAIL: after the retreat, visible/explored/unexplored is (422, 200, 7122). The [ECON-E6 to E26 record](RTS_ECONOMY_E6_E26_ACCEPTANCE.md) records this check at 7/8 with (426, 196, 7122) and calls it timing-dependent and not a gate, and ECON-E27 saw it pass | Not treated as a failure of this slice. Plain `--fog-memory` passes. It was not rerun, and the works has not been shown to have no effect on it |

## C. Verification

**Identity.** Packages from clean commit `bf98330` (`git_dirty=false`, `network_protocol` 24, engine 5.8.3). Client `dev-bf98330/…/Politika` `691a40ff7d28d5c59bd2f96bfd5cd60c4e4e34ab9a6035c95fef5e7026f5ff0e`; server `server-bf98330/…/PolitikaServer` `2d8e9822d19cfb52d865daf52e0248b8377b55d2b567fc764f06711873654c8f`, checked before each part of the matrix.

Automation ran with the editor library built from `bf98330` at 17:13. Uncommitted source for the next slices was in the tree by the time it ran and was not compiled into it. An uncommitted edit to `version.json` and `Config/DefaultInput.ini` was also in the tree for about two minutes around 18:13, during `economy-2`, and was reverted before the Automation run. The harness compiles `version.json` in and the packages carry their own cooked config, so the running sessions did not read it.

| Check | Result |
|---|---|
| Portable C++20 suite (`tools/core-test.sh`), on a clean checkout of `bf98330` | PASS |
| `cargo fmt --check` (Tools/Shell); Tools/Shell tests (26: 14 rts_multiplayer, 12 rts_interaction); `quality -- core`; `quality -- audit` | PASS |
| Automation, all Politika tests (`auto-final/`) | 61/61, 0 failures, 0 not run, with the new `Works` and `WorksAllies`. All 61 finish with warnings, as ECON-E27's 59 did |
| `--works` ×2 | 10/10 twice, identical `works-trace.txt` |
| `--buildings` ×2 | 20/20 twice, identical `buildings-trace.txt` |
| `--harvest` ×2, `--gold` ×2, `--food` ×2, `--power` ×2, `--economy` ×2 | 14/14, 14/14, 13/13, 12/12, 16/16 twice |
| `--economy-limits` ×2, `--pointer-stall` ×2 | 5/5 and 25/25 twice |
| `--group-move-facing` ×2, `--group-move-shape` ×2 | 54/54 and 47/47 twice |
| `--unit-scale` | 83/83, including every step-budget check: at 3v1 with 20 a seat, engage p99 1.3 ms and longest step 2.3 ms against the 50 ms budget, no catch-up frames |
| Launch options | 0 sessions flag `InvalidLaunchOptions` |
| Whole matrix against ECON-E27 (`compare-matrix.py`, `compare-bf98330.txt`) | 1 flagged, `default-fog-memory`, as in section B. The three expected `NOT_RUN` checks are unchanged |

All six C and Z press counts were 1 in every run of `--works` and `--buildings`, so the retry from ECON-E27 did not fire here either.

**The `--works` session.**

| Check | What it shows |
|---|---|
| `WRK-P01-THE-WORKS-STARTS-NEUTRAL-AND-UNSEEN` | The server starts it with no owner where expected, and neither client has received it |
| `WRK-P02-IT-ARRIVES-WHEN-SEEN` | Seat 1 walks into sight and receives it; seat 2 still does not |
| `WRK-P03-THE-CURSOR-NAMES-IT-AND-Z-LEAVES-IT` | The cursor reads `Neutral works` and what it pays, and Z logs no demolition |
| `WRK-P04-C-TAKES-THE-NEUTRAL-WORKS` | C walks the unit in and holds, it changes hands, and the view reports it held |
| `WRK-P05-IT-PAYS-ITS-HOLDER-ON-THE-INCOME-STEP` | Two $10.00 payments an interval apart, each on a base income step, reconciling; seat 2 is paid nothing |
| `WRK-P06-A-RIGHT-CLICK-ON-IT-IS-A-MOVE` | Seat 2 reads `Enemy works`, and its right-click is a move with no attack |
| `WRK-P07-THE-OTHER-SEAT-TAKES-IT` | C from beside it goes straight to the hold and it changes hands |
| `WRK-P08-THE-PAY-MOVES-AND-WHAT-IT-PAID-STAYS` | Seat 1 is paid nothing more and keeps its money, on base income only |
| `WRK-P09-LEDGERS-WHOLE-AND-THE-NEXT-MATCH-NEUTRAL` | Both ledgers reconcile at the match end, and the next generation has one neutral works |

Automation `Works` and `WorksAllies` cover what the session does not play: it cannot be demolished, damaged or repaired, a hold short by one step, leaving, and an ally refused. One screenshot was read, `client-a-02-works-taken.png`, and shows `Your works · pays $10.00 every 10 s` and the income line on the HUD. Log checks stand for the rest.

## D. Decisions for the owner

- **$10.00 per 10 s is a placeholder.** Base income is $20.00 per 10 s, so holding the works is worth half as much again.
- **Nothing has to stay to hold it.** A capture takes 3 s, and the works then pays until the other side walks over. Whether it should pay only while staffed is a separate decision.
- **It cannot be destroyed.** Grid sabotage, on the roadmap, would have to decide whether that changes.

## E. Map to the acceptance gates

| ID | Status in this increment |
|---|---|
| USD-05 | Partial: authorized income from a captured node, with a same-process rematch owing nothing. Contract events are not implemented |

## F. Not covered

- More than one works, more than two seats contesting one, and a listen host holding it.
- Allies are in Automation only. No packaged session plays them.
- The same play under packet loss or latency.
- The six navigation harnesses, which were not rerun, as in ECON-E5 to E27.
- Multiple-machine play, Shipping builds, a real desktop display and platforms other than Linux.
