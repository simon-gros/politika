# Stage 4A — Match Lifecycle Qualification

**Status: ACCEPTED for the bounded prototype lifecycle foundation, Linux UE 5.8.3 Development only.** This does not accept the broader Stage 4A match framework, a complete single-player match, Shipping rendering, or another platform.

## Contract and implementation

`APolitikaPrototypeMatch` owns one `politika::rules::MatchLifecycle`; portable transition legality remains in `Source/PolitikaCore/Rules.h`. The legal playable path is `SetupPending → SetupValidated → Loading → Starting → Active → Terminal → Results → Returning`. Admission failures enter `AdmissionRejected`, never `Active`; returning and rejection retire that lifecycle. Only `Active` admits ordinary command execution and authoritative fixed-step simulation.

Control-point completion is evaluated by the existing `APolitikaPrototypeObjective::UpdateCapture` after movement, combat, damage, and cleanup. When it completes, the coordinator queues the final `ObjectiveChangedEvent`, transitions `Active → Terminal → Results`, commits one in-memory result (`objective_secured`, winning player), then reaches the normal end-of-step publication phase. The result is not a durable record. Importantly, leaving `Active` clears pending command work and accumulated time, **not** the current step's unpublished gameplay events. The current-step completion event is synchronously broadcast and retained in the published snapshot; later non-active ticks do not execute or publish gameplay steps.

The narrow Development acceptance selector `-RTSAcceptance=objectivecompletion` arranges an eligible local unit at the control point, removes contesting presence, and shortens capture duration. It does not set objective progress/completion, assign lifecycle phases, or fabricate a result. Shipping excludes the selector and its objective test hook.

| Transition | Legal | Meaning |
|---|---:|---|
| SetupPending → SetupValidated / AdmissionRejected | yes | Validate admission or fail closed |
| SetupValidated → Loading / AdmissionRejected | yes | Begin initialization or reject |
| Loading → Starting / AdmissionRejected | yes | Instantiate validated content or reject |
| Starting → Active / AdmissionRejected | yes | Admit only after successful initialization |
| Active → Terminal / Returning | yes | Objective completion or shell exit; invalidate pending commands and time |
| Terminal → Results | yes | Commit and expose the prototype result |
| Results → Returning | yes | Retire match and return to the application shell |
| Returning / AdmissionRejected → any | no | Retired lifecycle is not reusable |

## Final-source verification

Source identity is `f3f0bde9609a-dirty` on `main` (HEAD `f3f0bde9609abb7aff0f41bc2677a2dddded4b6e`). No reset, branch operation, commit, or push occurred. The starting worktree was intentionally dirty; all pre-existing edits were preserved.

| Check | Result |
|---|---|
| `cargo run --locked --quiet --manifest-path tools/version/Cargo.toml --bin quality -- core` | PASS; portable C++20 rules suite, warnings as errors |
| `UNREAL_ENGINE_DIR=/mnt/data/unreal/5.8.3 bash tools/unreal.sh doctor` | PASS; project and pinned engine revision |
| `UNREAL_ENGINE_DIR=/mnt/data/unreal/5.8.3 bash tools/unreal.sh build` | PASS; `PolitikaEditor Linux Development`; final build log `build/diagnostics/stage4a-lifecycle-closure/editor-build-final2.log` |
| `Automation RunTests Politika.RTS.MatchLifecycle` | PASS; 1 discovered, 1 passed, 0 failed/skipped; `build/diagnostics/stage4a-lifecycle-closure/lifecycle-final2/report/index.json` |
| `Automation RunTests Politika.RTS.CommandContract` | PASS; 4 discovered, 4 passed (3 without warnings, 1 with a teardown warning), 0 failed; `build/diagnostics/stage4a-lifecycle-closure/command-contract-final2/report/index.json` |
| `Automation RunTests Politika.Content` | PASS; 7 discovered, 7 passed; `build/diagnostics/stage4a-lifecycle-closure/content-final2/report/index.json` |
| Portable and UE whitespace checks | PASS; `git diff --check` |

The focused coordinator case confirms legal activation; a neutral objective progressing by 0.5 per authoritative step; objective-driven `Active → Terminal → Results`; expected winning player/reason; one result commitment; one completion-event publication with correct complete/progress payload; retained snapshot; later-tick freeze; rejection of an inactive request as `MatchInactive` without retention; active Move positive control; pending-work invalidation; and legal Results return. It also confirms repeated return does not change the retired phase.

## Packaged Linux Development evidence

Both archives were built from the final source with UE 5.8.3 Linux Development `BuildCookRun`, `-build -cook -stage -pak -iostore -archive`, `-UbtArgs=-MaxParallelActions=6`. To preserve canonical source assets, the workflow's map-authoring commandlet was not run: inspection showed it saves the existing map and rewrites authored gameplay-definition/invalid-fixture assets. The existing assets were instead cooked as-is. Normal archive: `Saved/Packages/Stage4ALifecycleClosure/Linux`; negative fixture archive: `Saved/Packages/Stage4ALifecycleClosureFixture/Linux`, built with the existing isolated `-CookDir=/mnt/data/politika/Content/Politika/TestFixtures` option.

| Packaged check | Verdict / evidence |
|---|---|
| Normal package default MainMenu and shell routes | PASS; `smoke` 14/14, `Saved/ShellChecks/smoke-1793204578639` |
| Normal package rendered default startup | PASS; visible MainMenu at 1280×720, no implicit RTS start; screenshot `Saved/Packages/Stage4ALifecycleClosure/Linux/Politika/startup/main-menu.png` |
| Normal package fixture isolation | PASS; `Manifest_UFSFiles_Linux.txt` omits `DA_InvalidMapSchema.uasset` |
| Valid prototype startup and rendered commands | PASS; `rts_interaction`, PAC-10, `Saved/Packages/Stage4ALifecycleClosure/Linux/content-acceptance-interaction-1793204643` |
| Objective terminal path and Results visibility | PASS; rendered 1280×720 Development run using `-RTSAcceptance=objectivecompletion`; 3 units/1 objective; one phase-5 Terminal, one phase-6 Results, one committed `objective_secured` result, one complete ObjectiveChanged publication; screenshot `Saved/Packages/Stage4ALifecycleClosure/Linux/Politika/lifecycle-final2/results.png` |
| Results → MainMenu → fresh canonical match | PASS; Escape returned to the rendered menu and launching the canonical RTS map created a second legal Active lifecycle in-process. Its step-1 ObjectiveChanged event began at 500/1000 progress (fresh objective progress, not inherited), then independently completed. See `.../lifecycle-final2/objective-return-retry.log` and `fresh-match.png` (capture occurred after the accelerated second result). |
| Cooked invalid schema and fail-closed admission | PASS; isolated archive `content_acceptance`; expected `CONTENT.SCHEMA.UNSUPPORTED` for `map.test.invalid_schema.SchemaVersion` expected 1/actual 2, zero units/objectives/commands/simulation steps, then MainMenu; report under `Saved/Packages/Stage4ALifecycleClosureFixture/Linux/content-acceptance-1793204582` |
| Rendered rejection diagnostics and same-process valid retry | PASS; `shell_recovery`, PAC-07/PAC-09; `Saved/Packages/Stage4ALifecycleClosureFixture/Linux/content-acceptance-recovery-1793204594` |
| Post-retry valid match | PASS; the existing recovery harness verifies canonical map, 3 units, and objective in the same process after rejection |

Package UAT logs are `build/diagnostics/stage4a-lifecycle-closure/package-normal-final2.log` and `package-fixture-final2.log`; each must end with `BUILD SUCCESSFUL`. Package identity files report `git_revision=f3f0bde9609a-dirty`, `git_dirty=true`. Executable and IoStore SHA-256 hashes are recorded in `build/diagnostics/stage4a-lifecycle-closure/artifact-identity.txt`.

## Acceptance matrix

| ID | Verdict | Evidence |
|---|---|---|
| LC-C01 | PASS | Valid setup traverses legal phases to Active in coordinator automation and package logs |
| LC-C02 | PASS | The production evaluator advances and completes the control point; step-1 progress is 500/1000 and completion is 1000/1000 |
| LC-C03 | PASS | Exactly one `Active → Terminal` transition (phase 5) |
| LC-C04 | PASS | Exactly one following `Terminal → Results` transition (phase 6) |
| LC-C05 | PASS | Committed result is `objective_secured` for local player 1 in package and player 7 in coordinator fixture |
| LC-C06 | PASS | Completion ObjectiveChanged event remains staged across the lifecycle transition and reaches end-of-step publish |
| LC-C07 | PASS | Native observer receives the complete event once with correct step/progress payload |
| LC-C08 | PASS | Later ticks preserve the retained result and do not republish the completion event |
| LC-C09 | PASS | Post-terminal ticks do not advance authoritative simulation steps or gameplay state |
| LC-C10 | PASS | Pending pre-terminal request is cleared; it cannot execute on later ticks |
| LC-C11 | PASS | New inactive request yields `MatchInactive` and is not retained |
| LC-C12 | PASS | Active Move positive control is admitted and simulated |
| LC-C13 | PASS | Rendered Results-to-MainMenu transition through Escape |
| LC-C14 | PASS | Same process starts a new canonical Active match; objective progress restarts from zero, evidenced by its first-step 500/1000 event |
| LC-C15 | PASS | Cooked `InvalidMapSchema` fixture rejected with schema diagnostic (expected 1, actual 2) |
| LC-C16 | PASS | Rejection records zero units, objectives, commands, and simulation steps; no Active phase |
| LC-C17 | PASS | Rejection diagnostic remains accessible in rendered shell recovery state |
| LC-C18 | PASS | Same-process recovery starts canonical valid match after invalid rejection |
| LC-C19 | PASS | Normal UFS manifest omits `DA_InvalidMapSchema.uasset`; isolated fixture package includes it |
| LC-C20 | PASS | Final-source command-contract 4/4 and content 7/7 |
| LC-C21 | PASS | Portable C++, UE doctor/build, and lifecycle Automation 1/1 |
| LC-C22 | PASS | Fresh final-source Linux Development packages built; normal startup, objective Results, and return exercised |
| LC-C23 | PASS | Canonical asset hashes match preflight and final; direct BuildCookRun did not alter source assets |
| LC-C24 | PASS | Source/engine/test/package identities, manifests, reports, logs, and runtime evidence are recorded |

Original gate reconciliation:

| Gate | Verdict | Evidence |
|---|---|---|
| G-01 | PASS | Initial/final HEAD, branch, dirty manifest and relevant hashes recorded |
| G-02 | PASS | Every legal portable lifecycle edge exercised |
| G-03 | PASS | Full illegal transition pair matrix covered by portable tests |
| G-04 | PASS | Duplicate, terminal, rejected, and returning lifecycle irreversibility covered |
| G-05 | PASS | Active command gate and inactive rejection verified in coordinator |
| G-06 | PASS | Active-only fixed-step coordinator gate and terminal freeze verified |
| G-07 | PASS | Active Move positive control and package command replay |
| G-08 | PASS | Authoritative objective evaluator drives Terminal then Results |
| G-09 | PASS | Result identity committed once; completion event published once |
| G-10 | PASS | Leaving Active clears pending orders and accumulated simulation time |
| G-11 | PASS | Results return, fresh canonical match; invalid rejection recovery also retries in-process |
| G-12 | PASS | Isolated cooked invalid-schema fixture fails closed and recovers |
| G-13 | PASS | Portable C++ quality suite |
| G-14 | PASS | Pinned engine doctor and Editor build |
| G-15 | PASS | Focused lifecycle Automation 1/1 |
| G-16 | PASS | Command-contract 4/4 (one benign cleanup warning), content 7/7 |
| G-17 | PASS | Final-source Linux Development package built and started |
| G-18 | PASS | Packaged objective/Results/return/fresh-match sequence exercised |
| G-19 | PASS | Asset baseline and final hashes match; no generated asset restoration required |
| G-20 | PASS | Acceptance record, README, and roadmap reconciled to evidence without promoting full Stage 4 |

These verdicts cover only the bounded prototype lifecycle conditions listed here. The command-contract report contains one successful test with a pre-existing benign `UWorld::DestroyActor: World has no context` teardown warning; it has no failed tests or errors.

## Bounded participant-authority increment (28 October 2026)

This is a prototype-only implementation increment layered onto the lifecycle work above; it does not change the status of broader Stage 4A.

- Authored `SlotId`, `RuntimePlayerId`, `FactionId`, and `bHumanControlled` are converted after shared content validation into `APolitikaPrototypeMatch`'s match-local participant records. The current local human and assigned opposing/test controller kinds are explicit; the non-human slot is not described as functioning AI.
- Each spawned unit retains both its participant owner and authored faction ID. Placement owner/faction resolution uses the admitted roster. `faction.unaligned` is rejected from playable slots by the shared validator and roster conversion.
- Exactly one separate `faction.unaligned` NPC/system-authority record is initialized for a valid prototype setup. It has no playable participant ID, no spawned units, and no autonomous behavior in this milestone.
- Ordinary PlayerController selection and command entry now use the match-owned local-human binding. Numeric-ID command helpers are private trusted coordinator/test entry points; the ordinary input route cannot choose the participant ID. Runtime admission also rejects unassigned participant IDs and owner/faction mismatches.
- Invalid mapping conversion produces no candidate roster/local binding. Existing content validation remains responsible for authored duplicate IDs, slot/faction uniqueness, human count, placement references, and faction mismatch. Admission rejection and return clear roster, system authority, selection, spawned units/objective, and queued requests.

Verification against final implementation source (`f3f0bde9609abb7aff0f41bc2677a2dddded4b6e` plus dirty changes; UE 5.8.3 pinned revision verified):

| Check | Result |
|---|---|
| Portable core quality | PASS; `cargo run --locked --quiet --manifest-path tools/version/Cargo.toml --bin quality -- core` |
| UE preflight / Editor build | PASS; `tools/unreal.sh doctor` and `tools/unreal.sh build` |
| `Politika.RTS.MatchParticipants` | PASS with fixture teardown warnings; 1 discovered, 1 succeeded-with-warnings (5 teardown warnings), 0 failed/not-run; `build/diagnostics/stage4a-participants/final-focused3/report/index.json` |
| `Politika.RTS` regressions | PASS with warnings; 6 discovered, 6 succeeded (3 warning-bearing), 0 failed/not-run; includes 4 CommandContract, 1 MatchLifecycle and 1 MatchParticipants test; `build/diagnostics/stage4a-participants/final-rts2/report/index.json` |
| `Politika.Content` regressions | PASS; 7 discovered, 7 succeeded, 0 failed/not-run; `build/diagnostics/stage4a-participants/final-content2/report/index.json` |
| Linux Development package | PASS for the bounded valid prototype path; UE 5.8.3 Linux Development `BuildCookRun`, normal authored content, archive `Saved/Packages/Stage4AParticipants/Linux`; rendered PAC-10 `rts_interaction` passed with startup roster/owner/faction evidence in `content-acceptance-interaction-1793205757/rts-interaction.log` |

The focused test begins from the production catalog/map definition and exercises the same slot-to-roster conversion helper used during startup, duplicate slot/participant/faction and missing/duplicate-human rejection, invalid Unaligned playable mapping, coordinator rejection cleanup, unbound-input/system-owned command rejection, faction retention on a unit, and normal return cleanup. The packaged interaction replay exercises the real local PlayerController input path and confirms admitted owner/faction mapping from startup logs. It does **not** yet execute the entire `BuildScenario` authored-load/spawn path with fault-injected malformed definitions, nor the complete PA-01–PA-24 matrix. Specifically missing: actual local PlayerController spoofing an opponent's unit, a runtime spawned Unaligned entity, stale/destroyed selection and fresh-match authority replay, repeated Unaligned initialization fault injection, and per-case invalid-startup snapshots. These are not counted as passed.

### Participant acceptance matrix

| ID | Verdict | Evidence / limit |
|---|---|---|
| PA-01 | PASS | Canonical production slots convert through the startup roster helper |
| PA-02 | PASS | Authored local slot/player/faction/controller mapping asserted |
| PA-03 | PASS | Opposing slot maps to a distinct assigned-opponent role, not local-human |
| PA-04 | PASS | Separate Unaligned system record is present; package startup reports `unaligned_system=1`; no NPC behavior claimed |
| PA-05 | PASS | Shared validator and runtime roster conversion both reject `faction.unaligned` as playable |
| PA-06 | PASS | Cooked runtime logs show unit owners matching admitted slot participants |
| PA-07 | PASS | Cooked runtime logs show owner/faction distinct and matching each authored archetype/slot |
| PA-08 | PARTIAL | Local selection and opposing/system selection rejection are covered; stale/destroyed selection is not |
| PA-09 | PASS | Existing command-contract suite and packaged PAC-10 valid-command interaction |
| PA-10 | PARTIAL | Raw numeric-ID helpers are private and unbound sources reject; a local-controller attempt against an opposing unit is not directly replayed |
| PA-11 | PARTIAL | Unbound/system owner command rejection is tested with an isolated fixture; no packaged Unaligned unit exists |
| PA-12 | PARTIAL | Duplicate slot/participant setup rejected by conversion/validator, but not injected through full map startup |
| PA-13 | PARTIAL | Duplicate faction and contradictory human assignments reject; full controller-binding conflict startup fixture absent |
| PA-14 | PASS | Missing and duplicate local-human assignments fail candidate conversion atomically |
| PA-15 | NOT_RUN | Unknown placement slot has existing diagnostic logic but no new focused fault fixture |
| PA-16 | PASS | Existing content Automation asserts the slot/archetype faction mismatch diagnostic |
| PA-17 | PARTIAL | Coordinator rejection cleanup asserts no roster/units/objective/selection/commands/simulation; startup fault injection across every state field is incomplete |
| PA-18 | PASS | Existing active-only lifecycle and command regressions pass |
| PA-19 | PASS | Return test clears participant/system authority, selection, units and objective |
| PA-20 | NOT_RUN | Fresh participant roster generation/replay after return is not covered in this increment |
| PA-21 | PASS | Portable core, six RTS Automation tests, and seven content Automation tests pass |
| PA-22 | PASS | Final-source editor build and focused participant Automation pass |
| PA-23 | PARTIAL | Cooked/rendered valid prototype interaction passes; authority-spoof and full roster-state assertions are not in packaged harness |
| PA-24 | PASS | Branch/HEAD, patch and source hashes, UE pin, report paths, package hashes, and canonical-asset hashes recorded |

Overall bounded participant slice verdict: **INCOMPLETE** against the milestone completion criteria because PA-08/10–13/15/17/20/23 remain partial or not run, including required production-path fault injection and fresh-match isolation.

## Identity, asset safety, and limitations

- Test preflight HEAD was `f3f0bde9609abb7aff0f41bc2677a2dddded4b6e` on `main`, with the participant increment present as a dirty working-tree patch. That increment is now committed as `f050962df7564615de4108ad983e155288abcc52` and pushed to `origin/main`; the worktree is clean at that revision.
- The tested Linux Development package retains its historical identity `git_revision=f3f0bde9609a-dirty`, `git_dirty=true`; it was built from the participant implementation as an uncommitted patch and is not an exact-HEAD package for `f050962`. The current source hashes, full dirty-patch hash, test source hash, artifact hashes and engine pin from that test run are recorded in `build/diagnostics/stage4a-participants/source-identity.txt`.
- Final participant Linux Development package executable SHA-256: `b0b0df81f50abb0731146f3931da38439943451161794ac17ae2a59a7ef45af9`; `Politika-Linux.pak/.ucas/.utoc`: `b6178bee8cff3863e19f98a9de594b933dcb758d0e592e6d2c9095399106cd68`, `239d2da011e79d39145fa2bf351d3c97da77ba831371afa0ba11f57cb0924298`, `6b28dc6d0ee5c85ed5ebcb2c5b36f3fdf4d4437840f43b4b51e0165758d27056`.
- Pre-task canonical `.umap`, catalog, map definition, and invalid fixture hashes are in `build/diagnostics/stage4a-lifecycle-closure/baseline.txt`; their final hashes are in `artifact-identity.txt` and match. Direct BuildCookRun did not modify source assets. No restoration or overwrite was necessary.
- The acceptance result is an in-memory prototype result, not persistence. Qualification does not cover physical GPU/display/controller, Shipping rendering, non-Linux platforms, networked play, or a complete match framework.
- Broader Stage 4A still needs standard two-to-four canonical playable-faction setup and autonomous Unaligned controller behavior, among other rules-complete match responsibilities. This increment only records the NPC/system authority; it does not implement its independent simulation. Economy/deployment selection (fixed base, mobile command, or hybrid) remains open. Stage 4B harvesting, conversion, production, construction, and reinforcement are not started. Visibility, strategic AI, full victory/defeat rules, persistence, and the complete single-player loop remain roadmap work.
