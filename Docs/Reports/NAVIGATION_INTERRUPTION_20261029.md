# POLITIKA — authoritative navigation interruption engineering report

This is the archived original dirty-snapshot report. Latest-main publication validation is recorded separately in the [owning navigation record](../RTS_RUNTIME_NAVIGATION_ACCEPTANCE.md#latest-main-publication-qualification--2026-10-29); do not substitute this snapshot's executable or source identity for the merge candidate.

## A. Baseline — PASS

Authoritative dirty `/mnt/data/politika`, initial/final branch **main**, full HEAD **7d6aab9d730a707e898012bbe7acf8d4d6ea470e**, tracking HEAD...origin/main **0 3**, no staged changes. The brief's two-behind value was stale. All mandatory Git preflight commands executed before source edits; no reset/clean/switch/fetch/pull/merge/rebase/commit/push. Initial 40 modified/deleted tracked paths plus untracked navigation, infantry, audio, terrain/source/docs preserved. Six binary modifications were pre-existing (RTSVerticalSlice, catalog/map/unit definitions and two negative fixtures). No active UBT/UAT/Editor compiler at preflight, no relevant concurrent input mutation found afterward. `initial-status.txt`, `initial.patch`, `initial-staged.patch`, `initial-untracked.txt`, initial/final input and patch SHA256 manifests preserve details.

Pinned engine doctor PASS: `/mnt/data/unreal/5.8.3`, UE **5.8.3**, engine source **396c9f059903aed5fec78ecd3d437a40c6415368**, Linux Development Editor and Game. Historical NAV-A20/NAV-A25 evidence in `navigation-repeatability-20261028-1793228/` remains separate and unchanged; old qualified executable SHA256 **0c359d18de4acd780ca03b2efc77affe22b8b39deb3610aeca23aec903717cc6** rechecked. It was not reused for new qualification.

## B. Implementation — PASS

Task-owned repository changes only:

1. `Source/Politika/Private/PolitikaNavigationAcceptance.cpp`: Development-only scenario-aware observation, typed follow-up injection at completed-step boundary, Stop/duplicate/replacement/rejection counters, pre-movement snapshots, route episode and terminal checks.
2. `Source/Politika/Private/PolitikaPrototypeGameplay.cpp`: Development diagnostic route points include existing order sequence; movement termination log attributes actual installed sequence.
3. `Source/Politika/Public/PolitikaPrototypeGameplay.h`: Development acceptance fields and new Editor test friend access.
4. `Source/PolitikaEditor/Private/Tests/PolitikaNavigationInterruptionTests.cpp`: new saved-map production-Tick integration registration.
5. `Tools/Shell/src/navigation_obstacle.rs`: scenario-aware process launcher/validator/comparator, exactly 12 interruption then ten existing regression runs, reference reuse, fail-closed diagnostic comparator regression.
6. `Tools/Shell/src/bin/rts_interaction.rs`: additive `--navigation-interruption` dispatch; existing modes retained.
7. `Docs/RTS_RUNTIME_NAVIGATION_ACCEPTANCE.md`: owning bounded acceptance and updated NAV-A12/A13 evidence.
8. `Docs/ENGINE_OUTLINE.md`: narrowly scoped verified status update. Documentation index unchanged.

**No reproduced production defect; no production behavior correction.** All gameplay edits are Development telemetry/test hooks or test access. No new scheduler/event bus/schema/authoritative ID, no navigation redesign. Existing project Recast, saved fixture, typed adapters, Tick and Rust observer were adequate; no new dependency/service was warranted. Full sequence audit: `architecture-audit.md`.

## C. Command contract — PASS

Controller -> SubmitLocalControllerCommand -> SubmitCommand; trusted coordinator SubmitMove/SubmitStop adapters also enter SubmitCommand. World XY is converted to map offsets from PlayableMin. Submission stamps match/sequence/correlation/participant/issuer and queues; it is not authoritative admission. Tick's sole accumulator contributes min(delta,0.25), runs at most five 0.05 s steps. Each iteration resets queued event batch, calls AdmitPendingOrders -> ValidateOrder/portable AdmitCommand -> optional production synchronous complete Recast query -> execution mutator/identity installation -> queues outcome/transition. RunSimulationStep increments step, movement/combat/damage/cleanup/objectives execute, then queued events publish. Post-publication observer captures state and **only queues** follow-up after step40, before step41 admission/movement, including within one catch-up Tick. Duplicate queued after60, admitted61. No reentrant Tick/RunSimulationStep, mutator call or direct route/order assignment in this hook.

StopOrder sets Stop (not Idle), resets route/cursor and clears executable generation/sequence. Valid Move replacement installs sequence2 route after successful query; one Replaced retires sequence1. Rejected InvalidTarget branches before query/mutation. Arrival sets Idle and empties path/cursor, retaining diagnostic sequence/generation (not executable). OrderChanged has no command-sequence field; ordered outcome/entity/step history plus query/install snapshots correlate episodes. `NAV_INTERRUPTION_ADMITTED` at41/61 is a diagnostic pre-movement snapshot, not a fabricated additional command; only actual CommandOutcome events establish admission.

Accepted fixture unchanged: level `/Game/Politika/TestFixtures/Navigation/W_StaticObstacleDetour`, definition `map.test.static_obstacle_detour`, movement profile `movement.prototype_ground`, authored `unit.mechanized_fireteam`, runtime unit1/owner1, speed210 cm/s, authored scaled spherical collision. Catalog/content validation, intended local-human admission and Active are verified in every package process. Static Recast generation0, agent radius35cm, intended blocker collision/null-area exclusion unchanged. Start (-700,-600,42), goal (800,-600,42), bounds [-2000,2000] XY.

Common frozen step40: position **(-326.874538,-792.814392,42)**, Move, cursor1, four waypoints, executable generation/sequence1/1, unit/owner1/1, 40 legal movement samples, no arrival/failure. Original route measured **1632.748039cm**. Follow-up sequence2 submitted only after this state/publication, admitted41. All initial Moves submitted before step1, admitted1.

## D. OI-01 — PASS

All four processes: accepted sequence2 Stop at41, exactly one Move->Stop **Stopped** publication; route empty/cursor0/generation0/sequence0 before step41 movement. No step41 displacement and none through81. Duplicate sequence3 accepted61, no additional transition/termination; no restored path, arrival or navigation failure. First stationary window41–60 and second61–81 fully traced. Final Stop is intentional; no arrival requirement. `NAV_ACCEPTANCE_TERMINATION` counts movement-return terminations, so remains0 here; admission retirement is separately counted as Stopped=1 via the actual OrderChanged publication.

## E. OI-02 — PASS

All four processes: sequence2 Move back to authored start admitted41, one **Replaced** retires original sequence1. Production Recast query from frozen current position returns a fresh complete two-point route, measured **420.000000cm**, not a hard-coded geometry/arrival. Route installation snapshot41 belongs to generation1/sequence2; all subsequent movement is checked against that route and identity. Original destination never arrives. Replacement reaches (-700,-600,42), one movement Arrived termination and one OrderChanged publication at **80**; path/cursor retired and Idle, no displacement or duplicate arrival through **100**. Both original and return movement portions retain collision and authored speed legality.

## F. OI-03 — PASS

Sequence2 typed Move requested world **(3000,3000,42)**, portable offset **(5000,5000)** against map extent(4000,4000). Rejected at41 with **OrderRejection::InvalidTarget (5)**, accepted=false, correlation2, issuer1. Exactly one query/install in the process (original sequence1); no rejected query, route reset, new route, Stop/Replaced/NavigationFailed or progress mutation. Original Move arrives at **156** exactly once, remains inert through **176**.

All four processes compare against same-schedule final-source uninterrupted repetition1: identical position, cursor, route geometry/identity, order, travel, swept collision results, original command/order event sequence, arrival step and final state. Only the independently asserted rejected CommandOutcome is excluded from original-stream equivalence. Each `continuity.tsv`: **PASS**, max position/waypoint/route-length/travel deviation **0/0/0/0cm**, no divergent step/field.

## G. Packaged interruption matrix — PASS

Evidence root **R**: `/mnt/data/politika/Saved/Packages/INTFixture20261029-1793229768/Linux/navigation-interruption-1793230144532581186/`.

| Scenario | Schedule | Repetitions | Verdict | Steps | Arrival | Max position/waypoint/length/travel deviation cm |
|---|---|---|---|---:|---:|---|
| OI-01 | RS-02 | 1,2 | PASS | 81 | none | 0/0/0/0 |
| OI-01 | RS-05 | 1,2 | PASS | 81 | none | 0/0/0/0 |
| OI-02 | RS-02 | 1,2 | PASS | 100 | 80 | 0/0/0/0 |
| OI-02 | RS-05 | 1,2 | PASS | 100 | 80 | 0/0/0/0 |
| OI-03 | RS-02 | 1,2 | PASS | 176 | 156 | 0/0/0/0 |
| OI-03 | RS-05 | 1,2 | PASS | 176 | 156 | 0/0/0/0 |

All **12 fresh processes/matches**, names `OI-0N-RS-SS-rep-R`, isolated UserDirs, unique logs/results, 90s process timeout, clean exit/teardown. `R/matrix.tsv`; per-run `result.tsv`, `comparison.tsv`, `runtime.log`, `trace.txt`, command/route/event timeline. Six `OI-0N-RS-SS-comparison.tsv` and three `OI-0N-cross-schedule.tsv` reports identify the compared rows; each row was checked against scenario canonical RS-02 rep1 and independently against same-schedule repetition1. No Stop-vs-replacement trajectory comparison. All trace records contiguous by authoritative step, separately validated outer-update accumulator/catch-up telemetry. RS-05 exercises multi-step catch-up at the injection boundary.

Frozen schedules: RS-02 1/60; RS-05 repeating [0.01,0.20,0.04]. Frozen tolerances position/waypoint/travel0.1cm, length0.25cm; discrete states/enums/cursors/outcome/arrival/publication steps and counts strict. No tolerance widening. Independent body sweeps/numerical swept-sphere checks clear for every moving segment. Maximum reconstructed travel **10.500001cm**, authored budget10.5cm, within established 0.01cm speed arithmetic allowance. No unobserved movement/teleport or stale route.

## H. Existing final-source navigation regression — PASS

After12 interruption processes, **ten** fresh processes: five existing schedules twice. Evidence root **N**: `/mnt/data/politika/Saved/Packages/INTFixture20261029-1793229768/Linux/navigation-repeatability-1793230193769197417/`.

RS-01 1/120, RS-02 1/60, RS-03 1/30, RS-04 alternating0.0125/0.0375, RS-05 repeating0.01/0.20/0.04. Each: 176 complete step samples, arrival156 plus20 inert steps, actual complete four-point route1632.748039cm, exactly one arrival/termination, independently **NAV-A25 PASS** in `obstacle.tsv`. All ten comparisons PASS with max position/waypoint/length/travel **0/0/0/0cm**. `N/comparison.tsv`, `verdict.tsv`, traces/events. OI-03 reuses N's matching-schedule traces; no duplicate reference processes. Two startup probes are not counted in the required22.

## I. Automation and quality — PASS

Exact commands/logs: `commands.tsv`, `run-regressions.sh`, `package.sh`, `packaged-matrix.log`.

| Filter/check | Discovered | Executed | Passed | Warning-bearing success | Failed | Skipped/unrun |
|---|---:|---:|---:|---:|---:|---:|
| Politika.RTS.Navigation | 4 | 4 | 4 | 4 | 0 | 0 |
| Politika.RTS | 11 | 11 | 11 | 8 | 0 | 0 |
| Politika.Content | 7 | 7 | 7 | 0 | 0 | 0 |
| Rust Tools/Shell | 6 | 6 | 6 | 0 | 0 | 0 |

New filter **Politika.RTS.Navigation.SavedMapDetourOrderInterruption** discovered/executed, not a zero-test success. Actual saved fixture/real Recast with normal actor Tick disabled; test supplies production Tick only. Includes uninterrupted reference, all three scenarios, duplicate, identity and event assertions under both schedules. RTS includes Navigation (counts are not unique totals across filters). All JSON reports inspected; `automation-summary.json` retains individual tests/entries.

Known warnings: crowd manager cannot find Recast instance during map teardown/load (new test adds repetitions of the same category); expected NavigationUnavailable negative-test logs; existing transient-world DestroyActor world-context warnings in rejection/lifecycle/participant tests. No new teardown warning category; Content clean. Optional unavailable platform SDK/module and shader-autogen startup diagnostics are not claimed platform coverage.

Doctor, Editor build, game build, portable quality core and audit, cargo test, full cargo fmt check and final git diff --check PASS. Compiler success established from terminal Result: Succeeded, not partial logs. UBT/UAT compiler/cook operations serialized; mutex flags used. No production compiler correction needed. Initial harness enum expectations were corrected against Rules.h before packaged execution; no failed final-source runtime run was discarded.

## J. Packaging and preservation — PASS

Archives:

- Normal `/mnt/data/politika/Saved/Packages/INTNormal20261029-1793229768/Linux/`.
- Fixture `/mnt/data/politika/Saved/Packages/INTFixture20261029-1793229768/Linux/`.

Both executable SHA256 **b16dc3c1ac6e8a9c0f47091bda32944ef3312835eda685ccfb701bca37eac6a8**, identical to freshly built authoritative game binary. Scoped QualificationSource **8ee813c24d407ce43b480e7f3b20185d90cb067f897a4ee151e664c71dcab680**. Supplementary complete source/config/content/tools/build input manifest **b7f856209490eea1f3485774cba9d7800eea9e5c39329806e66fb9ba61e98cad**, file `complete-qualification-inputs.sha256`; both package sidecars `CompleteQualificationSource.sha256`. BuildIdentity says 7d6aab9d730a-dirty/game0.1.0-dev.0/schemas1 and is not used alone as source identity.

Normal pak **ab65d50942c914e90de9f54260ec8cea3d70b53c89678262a525abf017644465**, UCAS **eceb803e17981520c83562d4e88b123ce6d9eca3354033f36220f113dea5ddc1**, UTOC **48225f955f85bc1fc54ab1bf9fc36d7325a114c6064454e248e8d7e4cbc7eb97**.

Fixture pak **5ba10d7a3ef04dc64fe8fe9243f81414ab965ca95b890ae4844b7ccb6bb34532**, UCAS **f5cac6e4b64adf6640e837d31dcc61ced66a58dbd21ddb91d15675813fac654e**, UTOC **c3c397178ae5a5180812a432bcb1c49d12d3c9be38324dc9307a5ac590307f28**. Global containers/full hashes and rechecks: Normal/Fixture-artifacts.sha256 and final-artifact-check.txt. Actual manifests are in each archive.

Copied project `/mnt/data/politika-qualification/INT-20261029-1793229768` preserves byte-identical relevant source/content/binaries/receipts; source diff and complete hash check PASS. Its copied **GameDefaultMap=MainMenu** is the only config overlay. Both archived no-map startup probes actually load MainMenu. The canonical dirty configuration still starts RTSVerticalSlice and was not changed. Internal normal/fixture archives are not the default player launcher and do not claim a refreshed player-facing playtest. Latest integrated canonical assets are retained. Normal UFS manifest excludes all TestFixtures; fixture contains accepted level/definition/cooked nav, explicit `-PolitikaTestFixture=StaticObstacleDetour` plus level URL, with Development-only scenario argument. BuildCookRun used freshly built binaries with skipbuild/nocompileeditor, cook/stage/pak/iostore/archive, unique archives and serial cook, terminal BUILD SUCCESSFUL/exit0 for both.

Asset preservation: **62 initially captured nonignored authored assets unchanged**. Audit found default ripgrep omitted34 ignored generated local media assets; all34 match the previously identified historical hashes and current copied-project hashes, bringing the complete check to **96/96 unchanged**. This supplementary timing/provenance distinction is explicit in `asset-integrity.json`, not misrepresented as an early96-file capture. No asset saved/generated/imported by this task; canonical binaries and accepted fixture remain unchanged. Pre-existing dirty binary/source/config/untracked changes preserved. Original untracked observer files match preserved historical snapshot hashes and are additionally copied under `initial-untracked-source/`; initial/final binary patches distinguish all tracked edits.

No relevant source/content changed concurrently. Rechecks: final-worktree-input-check, runtime-source-snapshot-check, complete-snapshot-final-check, final-initial-assets-check, final-all-assets-check and package final-artifact checks. Historical packages/reports unchanged. Shipping verification **NOT_RUN**; other platforms and rendered/physical interaction **NOT_RUN** for this increment.

## K. Acceptance — PASS

| Gate | Verdict | Evidence |
|---|---|---|
| INT-01 local dirty source/engine pin | PASS | A, doctor, initial/final identities |
| INT-02 historical NAV evidence | PASS | A, retained report/executable hash |
| INT-03 command/simulation audit | PASS | C, architecture-audit.md |
| INT-04 actual saved/cooked fixture | PASS | C, every runtime fixture/query record |
| INT-05 initial typed Move before1 | PASS | submitted boundary0; admission1 |
| INT-06 complete executing route at40 | PASS | frozen common state and original query/geometry |
| INT-07 follow-up boundary40/admission41 | PASS | all12 command timelines; catch-up snapshots |
| INT-08 sole coordinator/no reentrancy | PASS | C, hook audit, outer-update validation |
| INT-09 Stop retirement before movement41 | PASS | D, snapshot41 and stationary sample |
| INT-10 one original Stopped | PASS | D, singular event41 |
| INT-11 stationary through60 | PASS | D, consecutive samples41–60 |
| INT-12 duplicate Stop inert/singular | PASS | D, command61 and samples through81 |
| INT-13 original replaced once | PASS | E, Replaced41 |
| INT-14 new complete real route | PASS | E, sequence2 query/geometry |
| INT-15 only new route movement | PASS | E, identity and segment comparison from41 |
| INT-16 no stale original arrival | PASS | E, exact event sequence |
| INT-17 replacement arrives once | PASS | E, arrival80/hold100 |
| INT-18 correct admission rejection | PASS | F, InvalidTarget5 step41 |
| INT-19 no invalid query/install | PASS | F, exactly one query/route |
| INT-20 original route preserved | PASS | F, route/identity/cursor traces |
| INT-21 uninterrupted equivalence | PASS | four continuity.tsv reports |
| INT-22 original arrival singular | PASS | F, arrival156/hold176 |
| INT-23 collision/speed/fixed-step | PASS | G, every route/sweep/travel/outer update |
| INT-24 all12 fresh interruption runs | PASS | R/matrix.tsv |
| INT-25 repetitions/cross-schedule | PASS | R/comparisons, zero deviations |
| INT-26 focused Navigation Automation | PASS | 4/4 and actual new registration |
| INT-27 RTS/Content regressions | PASS | 11/11 and7/7 inspected reports |
| INT-28 portable/Rust quality | PASS | core/audit; six tests; fmt |
| INT-29 ten-run final-source NAV-A20 | PASS | N/comparison.tsv and verdict |
| INT-30 NAV-A25 independently retained | PASS | ten individual N/obstacle.tsv reports |
| INT-31 archives isolated | PASS | startup logs/UFS manifests/config overlay |
| INT-32 canonical assets unchanged | PASS | J, 62 initial +34 historical/current matching hashes |
| INT-33 identities/artifacts recorded | PASS | J, source/patch/package/container manifests |
| INT-34 accurate bounded docs | PASS | owning record and narrow outline update |

**NAV-A12 PASS**, valid complete static cooked-route cancellation including duplicate Stop. **NAV-A13 PASS**, valid route replacement/no stale arrival. **NAV-A20 PASS**, final-source five-schedule/two-repeat matrix. **NAV-A25 PASS**, independently in all ten final-source uninterrupted processes; obstacle/speed assertions additionally preserved through each interruption's moving portions. All verdicts apply to the identified Linux Development snapshot, not general navigation or Stage4A.

## L. Remaining dependencies

Smallest next navigation increment: separately qualify unreachable and partial-route query failures through production typed admission/Recast/Tick, with explicit failure/no fallback and event atomicity. Do not implement it in this task. Attack/Guard approach, death/active-route lifecycle termination, dynamic invalidation, multi-unit navigation/formations/crowds, scale, cross-platform determinism and Shipping remain separate. No economy/production, autonomous AI, infantry geometry/rigging/import/rights work or First Playable Skirmish undertaken.

Reproduction: see the exact absolute `--navigation-interruption` invocation in the [owning navigation record](../RTS_RUNTIME_NAVIGATION_ACCEPTANCE.md#authoritative-navigation-order-interruption-qualification--2026-10-29). That command runs12 interruption processes then10 regression processes and continuity comparison; existing `--static-obstacle` and `--navigation-repeatability` remain supported.
