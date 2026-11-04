# OPP-1 — The first opponent that plays: acceptance record

**Date:** 4 November 2026 · **Scope:** one slice, `9c85d89`, from `1c4af32`. `network_protocol` stays 26.

**Outcome: IMPLEMENTED and ACCEPTED as one slice on the skirmish test fixture. First Playable Skirmish stays unaccepted: AI-10 is not met and AI-11 is not measured.** `--opponent` passes 23/23 twice with identical traces, Automation passes 64/64, and the whole matrix shows 0 regressions against ECON-E32 once the two sessions that hit a GPU out-of-memory are rerun on a quiet host.

**Published before qualification finished.** The owner told me to push while the run was in progress, so `9c85d89` reached `origin/main` at 00:26 with only the identity check and the `--opponent`, `--plant` and `--oil` first runs complete. This record, not that push, is the statement of what was qualified.

## A. What the slice added

With `-PolitikaMPOpponentSeat=<n>`, a dedicated server runs one seat with no connection. Off by default, Development only (compiled out of Shipping).

- **One brain, one seat, one match generation.** Binding is refused by name: `OPP.BIND.MISSING_SEAT`, `UNALIGNED`, `LOCAL_HUMAN`, `REMOTE_HUMAN`, `HUMAN_BOUND`, `DUPLICATE`, `NOT_DEDICATED`, `NOT_AUTHORITY`, `PARTICIPANT_ZERO`, `STALE_GENERATION`, `PHASE`; and `OPP.SEAT.MALFORMED`, `OPP.SEAT.CONFLICT` for the switch. A refused switch stops the server.
- **The seat is reserved.** It is never open to a connection and counts towards starting the match. It needs one bound human, and a brain alone does not start one.
- **A pure policy decides** (`Source/PolitikaCore/Opponent.h`), with no engine types, world queries, clock or random numbers. Every 20 steps it proposes at most one economy request or one batch of up to 8 attack-moves, in priority order: Retired, Recover, Provision (buy food), Sell (gold), Produce (riflemen), Advance, Idle.
- **Every request uses a human's intake,** after the part that resolves a controller. A refusal is ordinary input and starts a cooldown.
- **It sees its own account and units.** Its advance target is the declared prototype start, not the enemy's position.
- **Leaving Active retires it,** and a rematch makes a new one.

**Provisional figures, awaiting the owner's balance review:** 20 steps a decision, food threshold 120, gold threshold 25, force 4, advance 4, 8 orders a decision, cooldowns 100 and 200 steps, pending timeout 40 steps.

## B. What qualification found

| Finding | Cause | What was done |
|---|---|---|
| **`trial-1` first run: `OPP-P19` FAIL, "forged and refused: 0 of 3 units"** | The harness looked for the wrong log line. The refusal itself was happening | Harness fix before `9c85d89`. Second trial 23/23 |
| **A background editor build failed: `PolitikaMultiplayerTests.cpp:5957`, ambiguous `TestNotEqual`** | It compiled an older copy of the file. The call does not exist at `9c85d89` | None needed. The editor build at `9c85d89` passes |
| **The check numbers skip `OPP-P20`** | The number does not appear anywhere in the repository's history, so what it was meant to be, if anything, is unknown | **Waived by the owner** on 4 November 2026, who said the documentation was rewritten. It is not counted as a pass. The harness runs 23 checks |
| **`9c85d89` pushed before the run ended** | Owner instruction | See the outcome above |
| **Other work on the host during this run** | Placeholder package builds 23:46–00:04, harness 00:04–00:13, re-import 00:13–00:14, harness 00:14:01–00:22:52. Owner playtest 00:27:26–00:28:10. Animated-placeholder build, import, packaging and five rendered harness sessions 00:36:42–00:48:22. The tactical prototype's build and packaging from 01:10:20 and its rendered sessions 01:13:52–01:19:30. RuneScape under Proton from about 01:09 to 01:19:30, holding 9.3 of 16 GB of video memory | Only the next row is attributed to it. Every other session passed with this load present |
| **`regression-listen-1v1`: `SESSION-host-loss` FAIL, the client crashed with SIGSEGV. `matrix-none`: `SESSION-2v1` and `SESSION-3v1` FAIL, 60 s timeouts waiting for participant 3 to bind, so 47 checks and `MP1-G6-HOST-LOSS` did not run** | The GPU ran out of memory. The client logs of all three failed sessions have `Failed to allocate Device Memory` or `Out Of Memory` at about 01:14 | Rerun with the same packages on a quiet host (`rerun-9c85d89/`, 12:24–12:29, 1.8 GB of video memory in use at the start): 18/18 and 75/75, with no out-of-memory line. The first comparison's 51 regressions were these failures and the checks they stopped |
| **`default-fog-memory` exit 1: `MP2-FOG-EXPLORED-PERSISTS` FAIL** | After the retreat the counts were (414, 208, 7122). ECON-E32 failed the same check with (422, 200, 7122). `fog-memory`, on the same packages, passed | Not a gate. It is not a regression against ECON-E32 |
| **`listen-none` and `profile-lag-loss` exit 1** | The three expected `NOT_RUN` checks, as in ECON-E32 | None. `matrix-none` covers the legacy refusal |
| **Owner paused this work at 01:37, after the run had ended** | The reports were reviewed and the rerun made after the owner resumed at 12:24 | The packages did not change: the rerun checks their identity first |

## C. Verification

**Identity.** Packages from clean commit `9c85d89` (`git_dirty=false`, `network_protocol` 26, engine 5.8.3). Client `dev-9c85d89/…/Politika` `fdd70735c2f6f286fd9ea12434416215c2f67c4c70a52c4e84c0e86475de0a83`; server `server-9c85d89/…/PolitikaServer` `7f324f74c7d91b0c258342397672477a00c95956d95fcbce58cbd741d697b172`. Evidence is in `/mnt/data/politika-qualification/OPP-1-20261103/`.

| Check | Result |
|---|---|
| `--opponent` ×2 | 23/23 twice, identical `opponent-trace.txt` (up to step 900) |
| `--plant` ×2, `--oil` ×2, `--works` ×2, `--buildings` ×2 | 9/9, 12/12, 12/12, 20/20 twice, each pair with identical traces that also match ECON-E32's |
| `--group-move-facing` ×2, `--group-move-shape` ×2 | 54/54 and 47/47 twice |
| `--harvest` ×2, `--gold` ×2 | 14/14 and 14/14 twice, each with identical traces that match ECON-E32's |
| `--food` ×2, `--power` ×2, `--economy` ×2 | 13/13, 12/12 and 16/16 twice, each with identical traces that match ECON-E32's |
| `--economy-limits` ×2, `--pointer-stall` ×2 | 5/5 and 25/25 twice |
| Discipline: rout, effects, stress | 13/13, 12/12, 11/11 |
| Formation: discipline, stance, lifecycle | 11/11, 15/15, 21/21 |
| `--group-orders`, `--control-groups` | 9/9, 18/18 |
| `--unit-scale` | 83/83. At 3v1 with 20 a seat, engage p99 1.6 ms and longest step 2.0 ms against the 50 ms budget, no catch-up frames |
| Offline `PAC-10` and `PA-AUTHORITY` | PASS |
| Dedicated 1v1 regression, listen 1v1 regression | 23/23; 18/18 on the rerun |
| `matrix-none`, `listen-none`, `profile-lag-loss` | 75/75 on the rerun; 74 and 47 pass plus the expected `NOT_RUN` checks |
| `--engagement`, `--fog-memory`, default-forces fog memory | 10/10, 8/8, 7 of 8 as in section B |
| Launch options | 0 sessions flag `InvalidLaunchOptions` |
| Automation, all Politika tests (`auto-final-9c85d89/`) | 64/64, 0 failures, 0 not run, with the new `Opponent`. All 64 finish with warnings, as ECON-E32's 63 did |
| Whole matrix against ECON-E32 (`compare-9c85d89-with-rerun.txt`) | 0 regressions, and `--opponent` is new. `compare-9c85d89.txt`, before the rerun, is kept |
| Portable C++20 suite (`tools/core-test.sh`); `cargo fmt --check` (Tools/Shell); Tools/Shell tests (27: 15 rts_multiplayer, 12 rts_interaction); `quality -- core`; `quality -- audit` | PASS, at `9c85d89` |
| `tools/version`: `check`, `cargo test --all-targets` (7 and 14), `cargo fmt --check` | PASS; reports protocol 26 |

## D. Against the AI-01 to AI-11 ladder ([Opponent AI](RTS_OPPONENT_AI.md))

| ID | State | Evidence |
|---|---|---|
| AI-01 | Proven | Automation `Opponent`, `OPP-P02`, `OPP-P21`, `OPP-P24` |
| AI-02 | Proven for the trace | `--opponent` ×2 with identical traces up to step 900 |
| AI-03 | Proven | Automation `Opponent`, `OPP-P09` |
| AI-04 | Proven for what it reads | Automation `Opponent`; no enemy unit in the snapshot and the target does not follow the enemy. It reads no contacts at all, so stale and hidden contacts are untested because it has none |
| AI-05 | Mostly proven | Insufficient funds, no affordable food, an unpowered producer, a refused request and a failed route each lead to a bounded idle or cooldown in the portable tests; outage in `OPP-P11` and `OPP-P12`. An unauthorized sale is not exercised: `Unauthorized` is not among the refusals the portable test feeds in, though every refusal takes the same path |
| AI-06 | Proven | `OPP-P05` to `OPP-P10` |
| AI-07 | Proven | `OPP-P15`, `OPP-P23` |
| AI-08 | Proven | `OPP-P16` |
| AI-09 | Proven | `OPP-P17`, `OPP-P13` |
| AI-10 | **Not met** | Rendered Development package and a normal result (`OPP-P14`), but on `-PolitikaTestFixture=MultiplayerSkirmish`, which the rung excludes |
| AI-11 | **Not measured** | No CPU or query-count measurement of the opponent exists |

## E. What this does not establish

- **First Playable Skirmish stays unaccepted.** AI-10 is not met.
- **Only the human's win is shown.** `OPP-P14` is the human ending the match. The contract's "both a player win and loss" is not shown: no run has this opponent winning.
- **There is no Defend or Contest branch,** which the contract's list has at step 5. It advances on a declared position and fights what it meets.
- **It does not build.** It trains riflemen and nothing else, and buys no plant or repair.
- **Dedicated server only, 1v1 exercised in the packaged session.** A 2v2 reservation is covered in Automation only.
- **`OPP-P20` was waived, not run.** See section B.
- **The figures are not balance.** See section A.
