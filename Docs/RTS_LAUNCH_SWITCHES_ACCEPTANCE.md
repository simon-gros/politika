# Development launch switches: acceptance record

**Date:** 2 November 2026 · **Scope:** the launch-option parser in `PolitikaBootstrap` and the packaged harnesses that pass development switches to it. No gameplay, network or content change: `network_protocol` stays 12.

**Outcome: ACCEPTED. Every development switch the harnesses pass is now recognized, and an unrecognized one now fails the run instead of passing unnoticed. Across the regression matrix, logs flagging `InvalidLaunchOptions` went from 6 to 0 with 0 regressions against [ECON-E3](RTS_ECONOMY_E3_ACCEPTANCE.md); all six navigation harnesses pass on matching normal and fixture packages with 0 of 51 logs flagged; Automation 51/51. The new check fails on the old package, as it should.**

## A. The problem

The parser treats any `-Politika…` token it does not know as an error, which raises an `InvalidLaunchOptions` issue and a `recovery_required` log event. Nine switches owned by gameplay modules were missing from its development list, and most sessions had no check for it, so their runs passed with the issue in the log.

| Switches | Used by |
|---|---|
| `-PolitikaMPFormationCasualty`, `-PolitikaMPStanceProbe`, `-PolitikaMPDisciplineCasualties`, `-PolitikaMPDisciplineRoutWitness`, `-PolitikaMPDisciplineRoutSeed=`, `-PolitikaMPDisciplineDoctrine=` | Formation and discipline sessions |
| `-PolitikaNavigationFailure=`, `-PolitikaNavigationSchedule=`, `-PolitikaNavigationInterruption=` | Navigation harnesses |

The parser only recognizes these switches; the owning modules still read and validate their values.

## B. What changed (commit `5202f2b`)

- **Parser** (`PolitikaBootstrap.cpp`). The nine switches join the development lists. They remain unavailable in Shipping, where the lists are compiled out.
- **Parser tests** (`PolitikaLaunchOptionsTests.cpp`). The Development policy accepts each switch and the Strict policy rejects it. A valued switch with no value and a flag given a value are both still errors.
- **Multiplayer harness.** `launch_options_verdict` now runs in every remaining session: 32 new checks across this matrix's runs. Its guard now covers all `-Politika` switches, so a session that passes only the test fixture switch is still checked.
- **Navigation harness.** Each launch fails if its log raises `InvalidLaunchOptions`, with a unit test of that rule.

## C. Before and after

The old column is the accepted ECON-E3 matrix on `5c526ba`. The new column is the matrix on `5202f2b`.

| Behaviour | Before | After |
|---|---|---|
| Matrix logs flagging `InvalidLaunchOptions` | 6 of 263: the server log of `discipline-effects`, `discipline-rout`, `discipline-stress`, `formation-discipline`, `formation-lifecycle` and `formation-stance` | 0 of 263 |
| `recovery_required` line in `discipline-stress` and `formation-lifecycle` server logs | 1 each | 0 each |
| Sessions in which an unrecognized switch fails the run | Few: 12 launch checks in the matrix | All: 44 checks, all passing |
| Normalized traces of `group-move-facing`, `food`, `power` and `economy` | Baseline | Byte-identical to the baseline and between runs |
| Existing check results | Baseline | Unchanged by stable check ID |

**No gameplay behaviour changed.** Only dedicated server logs were flagged in the old matrix, and a server has no menu to show the issue on, so no player-visible recovery prompt was involved there. The old fixtures still worked, because the owning modules read their switches regardless.

**Navigation logs.** Older archived runs show the same defect: 4 of 4 logs flagged in a projection-displacement run from 29 October, 12 of 12 in an interruption run and 10 of 12 in a repeatability run. Those runs predate several later changes, so this is history, not a controlled comparison. The new runs have 0 of 51 logs flagged.

**Negative control.** The new harness run against the old `5c526ba` packages gives `discipline-stress` exit 1, 10 checks passing and one failing: `FAIL DISC-D2-DEV-LAUNCH-OPTIONS` with `InvalidLaunchOptions=true` on the server. The same session passes 11/11 on the new packages. The navigation rule is shown by its unit test rather than by a run against an old package.

## D. Verification

**Identity.** Multiplayer packages from clean commit `5202f2b` (`git_dirty=false`, `network_protocol` 12, engine 5.8.3). Client `dev-5202f2b/…/Politika` `4954b3931373c459b152c6ad80e0815e72c68b05c3cb295c4a6f2294b2fbda02`; server `server-5202f2b/…/PolitikaServer` `c7ccd60c85115f31dca24bdba1ff499925feda6be7fb939abaf4d4dc31c4519b`. Source input hash `310d4bcf56ab6a8cc08ab296cc0403098525842ff8859dd3686530fcf2e739e5`. Evidence directory: `/mnt/data/politika-qualification/LAUNCH-SWITCHES-20261101/` (local to the development machine). The CPU and GPU mining services were stopped for the accepted navigation runs and the whole matrix.

| Check | Result |
|---|---|
| Portable C++20 suite; `cargo fmt --check` (Tools/Shell); Tools/Shell tests (25: 13 rts_multiplayer, 12 rts_interaction); `quality -- core`; `quality -- audit` | PASS |
| Automation `Politika.Bootstrap.LaunchOptions` | Passed before packaging (`landing/auto-launch/`) |
| Automation, all Politika tests (`auto-all-final/`) | 51/51, 0 failures, 0 not run, warnings identical to ECON-E3 |
| Regression matrix against ECON-E3 (`compare-matrix.py`) | 0 regressions, 32 added checks all PASS, including the three expected `NOT_RUN` checks and the protocol-11 client refused in `matrix-none` |
| `--unit-scale` | 83/83 (73 plus 10 launch checks): worst engage-phase step p99 1.5 ms against the 50 ms budget, longest step 2.5 ms, no catch-up frames |

**Navigation packages.** The harnesses expect a normal package that boots MainMenu with no fixtures in it, so the two packages came from a copied project at `5202f2b` using the documented qualification-only startup overlay: only that copy's `GameDefaultMap` changed to `/Game/Maps/MainMenu`. This checkout's config did not change. Their identity therefore reads `5202f2be3fe4-dirty`, because of that one line. Both executables are `63398dba531732358516705df25dbb7b345926675eda3ae226ad7a1a37c54981`, with matching `QualificationSource.sha256` sidecars. The normal manifest has 0 `TestFixtures/` entries and the fixture manifest has 11 `TestFixtures/Navigation/` entries.

| `rts_interaction` harness | Result | Logs flagged |
|---|---|---|
| `--static-obstacle` | `NAV-A25` PASS | 0 of 3 |
| `--navigation-unreachable` | NQ-01 and NQ-02 PASS | 0 of 4 |
| `--navigation-destination-failure` | DP-01-A and -B PASS | 0 of 4 |
| `--navigation-projection-displacement` | DP-02-A and -B PASS | 0 of 4 |
| `--navigation-repeatability` | `NAV-A20` PASS | 0 of 12 |
| `--navigation-interruption`, which also runs repeatability again | Interruption PASS, `NAV-A20` PASS | 0 of 24 |

**Interrupted attempt.** A first matrix run stopped after 17 sessions when the tool connection dropped. A first navigation packaging attempt stopped the same way, before producing a package. The matrix run is kept as `final-5202f2b-interrupted/` and is not part of these results. The whole matrix was rerun from the start on the same packages.

## E. Not covered

- Shipping behaviour by a packaged run. The Strict policy is covered by Automation, and the lists are compiled out of Shipping.
- A check that a harness's switches and the parser's lists stay in step at build time. A new unrecognized switch is caught when its session runs.
- `InvalidLaunchOptions` in the older accepted records: their results stand as recorded and are not rewritten.
