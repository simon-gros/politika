# ECON-E1 — Participant treasury, income and dollar-priced training: acceptance record

**Date:** 1 November 2026 · **Scope:** the first sub-slice of Stage 4B in the [Engine Outline](ENGINE_OUTLINE.md): a US-dollar treasury, one authorized income source, one fixed producer and one dollar-priced training queue, as designed in [Operational Economy and Power](RTS_OPERATIONAL_ECONOMY_AND_POWER.md) and [Production and Technology](RTS_PRODUCTION_AND_TECHNOLOGY.md). The qualification also found and fixed a mouse-input defect that predates ECON-E1; see [section C](#c-mouse-input-fix-found-during-qualification).

**Outcome: IMPLEMENTED and ACCEPTED for the skirmish test map, with one recorded host condition. Packaged economy check 15/15 in two runs and limits check 5/5 in two runs, all by real input on a dedicated 1v1; new packaged mouse check 24/24 in two runs; Automation 49/49 with no failures; regression matrix unchanged from the FORM-FD baseline except `unit-scale`, which passes 73/73 on a quiet host and fails its timing check when other work is contending for the CPU ([section B](#b-verification)). Stage 4B is not complete: electricity, food, a source/delivery route and the commodity sale stay open.**

## A. What changed (feature `f9dd92f`, harness `2f5ead2`, `756b84e`, `f50e812`)

- **Ledger** (`Source/PolitikaCore/Economy.h`, no Unreal types). One ledger per match in integer USD cents with checked arithmetic. Each participant starts with $1,000.00 and receives $20.00 every 200 simulation steps.
- **Producer and recipe.** One fixed placeholder producer per seat, relevant to its own team only under fog, training the seat's authored rifle unit for $360.00 in 200 steps. It has no health, capture or electricity requirement.
- **Input.** **B** trains and **Shift+B** cancels the newest queue entry for a full refund. No selection is needed.
- **Server authority.** A request carries no price or participant. It is checked for shape and freshness on arrival and admitted in the next step's economy phase, after casualties and before the objective, in ascending participant order.
- **Limits.** The queue holds 5 entries. Queued entries count towards the map's 20-unit limit. A blocked output keeps its place and payment and retries every 20 steps.
- **Match end.** The end refuses waiting requests and clears the ledger, and the next generation starts fresh.
- **View.** An owner-only replicated view and one HUD line show money, income, the training entry and the keys.
- **Network and diagnostics.** `network_protocol` 9 to 10. `RTS_ECONOMY_*` and `RTS_PRODUCTION_*` logs, and the `-PolitikaMPEconomyStartCents` development fixture.
- **Tests.** Portable ledger tests in `Tests/Core/RulesTests.cpp`, Automation `Politika.RTS.Multiplayer.Economy`, and the `--economy` and `--economy-limits` packaged sessions.

## B. Verification

**Identity.** Packages from clean commit `731d3db` (`git_dirty=false`, `network_protocol` 10, engine 5.8.3). Client `dev-731d3db/…/Politika` `ae8d33e750183b144004eb106591c85c0cd05ded2536b46692d66349785ab2de`; server `server-731d3db/…/PolitikaServer` `9d1a9fc68e89809341f9a405042f2d1b5b3091dee49683a1e4a8a7bc00ad0b64`. Source input hash `c705757f87703025d7982bd5bd1e5a6833030fb05a89351b48eea74d524d2469` over `Source`, `Config`, `Content`, `Politika.uproject` and `version.json`, unchanged at `f50e812`, which changes only the harness. Evidence directory: `/mnt/data/politika-qualification/ECON-E1-20261101/` (local to the development machine).

| Check | Result |
|---|---|
| Portable C++20 suite (`tools/core-test.sh`) | PASS (`ptr/core-test.out`) |
| `cargo fmt --check` (tools/version, Tools/Shell); Tools/Shell tests (24: 13 rts_multiplayer, 11 rts_interaction); `quality -- core`; `quality -- audit` | PASS (`ptr/rust-tests.out`, `ptr/quality-core.out`) |
| Automation, all Politika tests (`auto-all-final/`) | 49/49, 0 failures, 0 not run: 47 at the FORM-FD baseline plus `Economy` and `PointerEventPosition`. Warnings match the baseline except for those two new tests one extra `IsUsingWayland` notice on four saved-map navigation tests, and one warning fewer on `SavedMapDetourRenderScheduleRepeatability` |
| `--economy` ×2 (`rerun-731d3db/`) | 15/15 twice |
| `--economy-limits` ×2 (`final-731d3db/`) | 5/5 twice |
| `--pointer-stall` ×2 (`final-731d3db/`) | 24/24 twice |
| `--group-move-facing` ×2 (`final-731d3db/`) | 53/53 twice, normalized plans byte-identical; a third gate run on the same packages also 53/53 (`gate-731d3db/`) |
| `--group-move-shape` ×2 and the other regression sessions (`final-731d3db/`) | Every check unchanged from the FORM-FD baseline by stable check ID (`compare-matrix.py`), including the three expected `NOT_RUN` checks in `listen-none` and `profile-lag-loss`, and the protocol-9 client refused in `matrix-none` |
| `--unit-scale`, quiet host (`quiet-731d3db/`) | 73/73 |
| `--unit-scale`, contended host (`final-731d3db/`) | 68/73: five `UNITSCALE-STEP-WITHIN-BUDGET` failures, explained below |

**Economy behaviour shown by the packaged runs.** The server logs $1,000.00 per seat at the start. A purchase debits once, a refused third purchase changes nothing, a cancel refunds the full price, and the unit appears exactly 200 steps after admission and takes orders. The enemy client does not receive that unit while it is out of vision. Replaying the server's ledger lines reproduces its balances. A match end with an item in training logs no money or spawn lines afterwards, and the next generation starts at $1,000.00. Neither client receives the other's money view. The limits session fills the queue to 5, counts queued items towards the unit limit, and nets exactly one price for one unit after five purchases and four refunds. Screenshots `client-a-02-training.png` and `client-a-03-refused.png` show the HUD money line, the training timer, the refusal message and the placeholder producer.

**`unit-scale` and host contention.** The check allows no server frames that hit the catch-up limit, which takes a frame of about 250 ms. Full runs on 1 November had both CPU and GPU mining services running on the development machine, with 22 of 24 CPU threads mining and a desktop session in use. The FORM-FD baseline also had the miners running, at 02:16 with less other activity, and passed.

| Run | Host | Result | Catch-up frames | Server main-thread CPU | Main-thread run-queue wait |
|---|---|---|---|---|---|
| FORM-FD `e694e34`, A/B 1 | contended | 73/73 | 0 | 15.87 s | 32.26 s |
| ECON-E1 `2f5ead2`, sampled | contended | 72/73 | 1 | 15.41 s | 43.21 s |
| ECON-E1 `2f5ead2`, A/B | contended | 70/73 | 3 | 15.51 s | 42.05 s |
| FORM-FD `e694e34`, A/B 2 | contended | 68/73 | 14 | 16.10 s | 74.67 s |
| ECON-E1 `731d3db` | miners stopped | 73/73 | 0 | 13.40 s | 0.11 s |

Each slow server frame matched in the sampled ECON-E1 run took about 2 ms on the CPU and 120–190 ms in the run queue (`diag/`). The accepted FORM-FD package fails the same check under the same load, so this is not an ECON-E1 regression, and ECON-E1 adds no measurable main-thread CPU cost. On the quiet host the worst engage-phase step p99 is 1.6 ms against the 50 ms budget, the longest step 2.4 ms and the longest frame 34.2 ms. The quiet run is a single run; the full matrix was run with the miners on.

**Failed attempts.** Every attempt is kept in the evidence directory.

1. Matrix on `2f5ead2`: two FORM-FD checks pinned protocol 9; fixed in `756b84e`.
2. Reruns on `2f5ead2`: one `group-move-facing` run missed its box select (`hits=0`) during an 888 ms client stall, which led to [section C](#c-mouse-input-fix-found-during-qualification).
3. Local commit `59f782b`: read the release position with the cursor still hidden during capture, so every box collapsed to its press point. Mouse sessions failed across `final-59f782b/`.
4. Local commit `6d7a68d`: captured only press positions, and failed the focused gate (`gate-6d7a68d/`).
5. Matrix on `731d3db`: `ECON-P03-TRAIN-DEBITS-ONCE` failed once. The ledger was correct (100000 + 2000 − 72000 = 30000). On that slower run, income landed between the two purchases, and the check assumed only the price changes `available`. Fixed in `f50e812`, which compares `spent` and reconciles `available` with `credited`. The corrected check passes that run's ledger lines and rejects a simulated double debit.

## C. Mouse-input fix found during qualification

**Fix `7ebcda2`, harness `731d3db`.** This changes how the cursor behaves in play, for every mouse drag.

- **Cause.** `FInputModeGameAndUI` defaults `bHideCursorDuringCapture` to true. On each button press the viewport hid the cursor and switched to relative mouse movement, and on release it put the cursor back at the press point. On the old package, the X pointer returned to (24, 24) after a full-screen drag ending at (1256, 696). `f4878c5` had worked around the restored position by ending boxes at the last held sample, which leaves a box on an old sample when a long frame hides the rest of the drag.
- **Fix.** The prototype controller's input mode no longer hides the cursor during capture, and an `APlayerController::InputKey` override keeps each left and right button press and release at the position the viewport delivered it. Box selects, clicks, right-click orders and facing drags use those positions, falling back to the last held sample if a release has none. `RTS_SELECTION_STROKE` logs both corners in non-Shipping builds.
- **Check.** `--pointer-stall` runs real X input against a client capped at 2 FPS. Automation `PointerEventPosition` covers the controller's position handling with a stand-in cursor.

| `--pointer-stall` check | Old `2f5ead2` package | Fixed `731d3db` package |
|---|---|---|
| Box select spanning frames, selects 20 | 3 of 5 | 5 of 5, both runs |
| Box select inside one frame, selects 20 | 0 of 5 | 5 of 5, both runs |
| Pointer stays where the button came up | no, returns to the press point | yes |
| Right-drag inside one frame commits as a drag | no, a click | yes, between its true ends |
| Right-click then pointer move inside one frame orders at the press | yes | yes (does not distinguish the two packages) |

The old package has no `RTS_SELECTION_STROKE` line, so its `-CORNERS` checks fail by construction and are left out of this table.

**Limits of this evidence.** The original `hits=0` run has no corner log, so its cause is inferred from the frame timing, the engine source and the failures reproduced here. All of this ran under Xvfb on Linux: a pointer position is measured, but the visible cursor on a real desktop was not observed, and other platforms were not run.

## D. Not covered

- Electricity, food, upkeep, a source/delivery route, the commodity sale and a sale UI.
- Producer health, capture, loss and placement, more recipes, and technology.
- Multiple-machine play, Shipping builds and platforms other than Linux.
- Keyboard-aimed orders (Q attack-move, grenade) still read the cursor when their binding runs, so a long frame can move their target; `--pointer-stall` does not exercise them.
