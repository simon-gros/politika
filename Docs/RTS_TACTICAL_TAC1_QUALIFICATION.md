# TAC-1 — Prototype garrison, crush, concealment and building selection: qualification record

**Date:** 4 November 2026 · **Scope:** one slice, `b9ee323`, from `39624a8`. `network_protocol` goes from 26 to 27.

**Outcome: IMPLEMENTED on the skirmish test fixture and passes its packaged session. Not accepted as design: the owner has not playtested it, its numbers are fixtures, and specification items U-07 and U-08 stay open.** `--tactical` passes 14/14 twice, Automation passes 65/65, and no check got worse across the matrix against OPP-1. The owner chose to put it on `main` before a playtest.

## A. What the slice added

The rules, keys and cues are in [the prototype's own document](RTS_TACTICAL_PROTOTYPE.md). Nothing conceals or garrisons unless ordered, and the only change to existing input is that a click on a building with no unit in it or behind it now selects the building.

**Silent in this slice.** No authored sounds exist for these events, so each logs `RTS_TACTICAL_CUE` where a sound would play. `TAC-P04`, `P08`, `P09`, `P12` and `P13` prove that the request reached the right client, not that anything was heard.

## B. What qualification found

| Finding | Cause | What was done |
|---|---|---|
| **Earlier runs do not count** | They used a package built from uncommitted source | The slice was moved to `main` as one clean commit and its packages rebuilt. Only the results in section C are evidence |
| **An enemy harvester on its routine crushed the soldier in an earlier run (`RTS_CRUSH vehicle=16 victim=1`)** | A real bug in the crush rule. A routine drives through a base without aiming at anyone | A harvester on its routine no longer crushes. A player's order takes it off the routine. Automation `TAC-C04`, `TAC-C05` |
| **`--tactical` failed before that: a click missed a moving harvester, and `TAC-P10` stood 330 cm from the enemy** | Harness. Positions are sampled once a second, and 330 cm is inside the 420 cm a concealed unit is still seen at | The harvester is selected while loading at its source, and the distance is about 600 cm |
| **The comparison reports `regressions=1`** | `default-fog-memory`'s `MP2-FOG-EXPLORED-PERSISTS` went from FAIL to PASS, and the script counts any change. It is the known intermittent check, not a gate | No check went from PASS to anything else |
| **`listen-none` and `profile-lag-loss` exit 1** | The three expected `NOT_RUN` checks, as in OPP-1 | None. `matrix-none` covers the legacy refusal |
| **Host during the run** | A Steam client was open with no game running. Video memory at each session's start was 1.7–3.2 GB of 16 GB, and a session would have waited below 8 GB free | None needed |

## C. Verification

**Identity.** Packages from clean commit `b9ee323` (`git_dirty=false`, `network_protocol` 27, engine 5.8.3). Client `dev-b9ee323/…/Politika` `fa2ef0f8268bb480e9cf2d2f34d2fdb06f6c815da98e0fe369bc502dfed4ee59`; server `server-b9ee323/…/PolitikaServer` `6ebd11d3ec773eb768905d585548b4d2707354b9ac898c66fa6e1a89bc8bfa82`. Evidence is in `/mnt/data/politika-qualification/TAC-1-20261104/`.

| Check | Result |
|---|---|
| `--tactical` ×2 | 14/14 twice. Garrison, crush and concealment events came in the same order, with different step numbers and positions, so the traces are not identical. Screenshots of each state were read |
| `--opponent` ×2 | 23/23 twice, identical traces that also match OPP-1's |
| `--plant`, `--oil`, `--works`, `--buildings` ×2 | 9/9, 12/12, 12/12, 20/20 twice, each pair with identical traces that also match OPP-1's |
| `--harvest`, `--gold`, `--food`, `--power`, `--economy` ×2 | 14/14, 14/14, 13/13, 12/12, 16/16 twice, each pair with identical traces that also match OPP-1's |
| `--group-move-facing`, `--group-move-shape` ×2 | 54/54 and 47/47 twice |
| `--economy-limits`, `--pointer-stall` ×2 | 5/5 and 25/25 twice |
| Discipline: rout, effects, stress | 13/13, 12/12, 11/11 |
| Formation: discipline, stance, lifecycle | 11/11, 15/15, 21/21 |
| `--group-orders`, `--control-groups` | 9/9, 18/18 |
| `--unit-scale` | 83/83. At 3v1 with 20 a seat, engage p99 1.5 ms and longest step 2.5 ms against the 50 ms budget, no catch-up frames |
| Offline `PAC-10` and `PA-AUTHORITY` | PASS |
| Dedicated 1v1 regression, listen 1v1 regression | 23/23, 18/18 |
| `matrix-none`, `listen-none`, `profile-lag-loss` | 75/75; 74 and 47 pass plus the expected `NOT_RUN` checks |
| `--engagement`, `--fog-memory`, default-forces fog memory | 10/10, 8/8, 8/8 |
| Launch options | 0 sessions flag `InvalidLaunchOptions` |
| Automation, all Politika tests (`auto-final-b9ee323/`) | 65/65, 0 failures, 0 not run, with the new `Tactical`. All 65 finish with warnings, as OPP-1's 64 did |
| Whole matrix against OPP-1 with its rerun (`compare-b9ee323.txt`) | No check got worse; see section B. `--tactical` is new |
| Portable C++20 suite (`tools/core-test.sh`); `cargo fmt --check` (Tools/Shell); Tools/Shell tests (27); `quality -- core`; `quality -- audit` | PASS, at `b9ee323` |
| `tools/version`: `check`, `cargo test --all-targets`, `cargo fmt --check` | PASS; reports protocol 27 |

## D. What this does not establish

- **Nobody has played it.** The fixture numbers (garrison capacity 5, entry range 200 cm, crush radius 90 cm) are not balance.
- **U-07 and U-08 stay open.** Crush lacks the collision and navigation evidence the specification asks for, and it was built on the owner's instruction ahead of that deferral. Section 6.9's damage split is not implemented, because the game has no damage types.
- **It has no sounds.** See section A.
- **Not exercised in a packaged run:** a listen-server host, a captured building ejecting its occupants, and the sandbox's vehicles (recon, fireteam, patrol).
- **The opponent uses none of it.** It does not garrison or conceal, and it reads no contacts, so it is not affected by a concealed unit.
- **First Playable Skirmish stays unaccepted.**
