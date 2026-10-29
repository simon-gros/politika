# Stage 4A — Match Lifecycle Qualification

**Status: ACCEPTED for the bounded prototype lifecycle foundation and the integrated participant-authority/implemented-infantry contract, Linux UE 5.8.3 Development only.** This does not accept the broader Stage 4A match framework, a complete single-player match, Shipping rendering, or another platform. The combined participant/infantry evidence is recorded in the 29 October integration section below; its exact final source/package identity and reports are in `build/diagnostics/stage4a-integration/final-evidence.json` for the isolated qualification worktree.

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
| `Politika.RTS.MatchParticipants.AuthoredStartupAdmission` | PASS; 1 discovered/executed, 0 failed/skipped/warning-bearing; `build/diagnostics/stage4a-authored-admission/focused-final/index.json` |
| `Politika.RTS` regressions | PASS with existing teardown warnings; 7 discovered/executed, 4 succeeded and 3 succeeded-with-warnings, 0 failed/skipped; `build/diagnostics/stage4a-authored-admission/rts-final/index.json` |
| `Politika.Content` regressions | PASS; 7 discovered/executed, 7 succeeded, 0 failed/skipped; `build/diagnostics/stage4a-authored-admission/content/index.json` |
| Portable core quality | PASS; `build/diagnostics/core.json` |
| Linux Development package | PASS for the bounded valid prototype path; final-source UE 5.8.3 Linux Development `BuildCookRun`, archive `Saved/Packages/Stage4AAuthoredAdmissionFinal/Linux`; rendered PAC-10 `rts_interaction` passed in `content-acceptance-interaction-1793206836/rts-interaction.log` |

The focused authored-admission test and package results below supersede only the earlier source-path limitation; other uncovered participant behaviors remain unqualified.

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
| PA-12 | PASS | Duplicate slot and runtime participant IDs injected at loader resolution and rejected through `BuildScenario` |
| PA-13 | PASS | Duplicate faction and zero/multiple local-human mappings rejected by shared validation through startup; authored schema has no separate controller-binding field |
| PA-14 | PASS | Missing and duplicate local-human assignments fail through production startup validation |
| PA-15 | PASS | Valid archetype/location with only an undeclared placement slot injected through production startup; `MAP.SLOT.REFERENCE_UNRESOLVED` asserted |
| PA-16 | PASS | Authored archetype/slot faction mismatch now also rejects through `BuildScenario`; existing content Automation remains green |
| PA-17 | PASS | Editor startup assertions and cooked Development rejection snapshot show empty authority/gameplay state after structural faction mismatch |
| PA-18 | PASS | Existing active-only lifecycle and command regressions pass |
| PA-19 | PASS | Return test clears participant/system authority, selection, units and objective |
| PA-20 | PASS (packaged process scope) | Editor transient-world retry remains blocked at `MAP.LEVEL.MISMATCH`; independently, rendered packaged recovery starts a fresh canonical Active match in the same PID after the one-shot faction fixture |
| PA-21 | PASS | Portable core, seven RTS Automation tests (three warning-bearing successes), and seven content Automation tests pass |
| PA-22 | PASS | Final-source editor build and focused participant Automation pass |
| PA-23 | PASS | Isolated cooked faction-mismatch package rejects through the shared validator, renders recovery, and retries canonically in-process; normal Development and Shipping manifests exclude fixtures |
| PA-24 | PASS | Branch/HEAD, patch and source hashes, UE pin, report paths, package hashes, and canonical-asset hashes recorded |

Overall bounded authored-startup slice verdict: **PARTICIPANT RECOVERY QUALIFIED** for Linux UE 5.8.3 Development, with the Editor transient-world retry still accurately blocked and independently superseded by the packaged process proof below. At this historical 28 October checkpoint, PA-08/PA-10/PA-11 retained partial spoof/stale-selection limits; the later combined-source qualification below supersedes those specific bounded verdicts without erasing this checkpoint. The broader Stage 4A remains incomplete independently.

### Authored participant startup admission and recovery increment — 2026-10-28

**Production path exercised:** `APolitikaPrototypeMatch::BuildScenario` → `FPolitikaPrototypeContentLoader::LoadAndValidate` (Asset Manager production catalog resolution and shared `ValidateCatalog`/`ValidateMapStructure`) → admission rejection/`RecoverToShellAfterAdmissionFailure`. An Editor-only `WITH_DEV_AUTOMATION_TESTS` one-shot map candidate is substituted only at the loader's map-resolution boundary. Each candidate is a transient duplicate of the canonical authored map; catalog loading and validation are not bypassed. The override is consumed on that load and explicitly cleared after every case. It is unavailable to packaged Development and Shipping code. The suite does not use or modify the canonical content assets.

`Politika.RTS.MatchParticipants.AuthoredStartupAdmission` covers these canonical-map mutations through actual `BuildScenario` startup:

| Mutation | Primary diagnostic asserted | Startup/rejection result |
|---|---|---|
| Duplicate `PlayerSlots[].SlotId` | `MAP.SLOT.DUPLICATE`, `PlayerSlots.SlotId` | AdmissionRejected; no roster or gameplay state |
| Duplicate `PlayerSlots[].RuntimePlayerId` | `MAP.PLAYER.DUPLICATE`, `PlayerSlots.RuntimePlayerId` | AdmissionRejected; no roster or gameplay state |
| Duplicate playable faction | `MAP.FACTION.DUPLICATE`, `PlayerSlots.FactionId` | AdmissionRejected; secondary placement faction mismatch retained |
| Zero local-human slots | `MAP.SLOTS.HUMAN_COUNT`, `PlayerSlots.bHumanControlled` | AdmissionRejected |
| Two local-human slots | `MAP.SLOTS.HUMAN_COUNT`, `PlayerSlots.bHumanControlled` | AdmissionRejected |
| Unaligned in ordinary playable slot | `MAP.FACTION.UNALIGNED_PLAYABLE`, `PlayerSlots[0].FactionId` | AdmissionRejected; secondary faction mismatch retained |
| Authored unit archetype faction conflicts with its declared participant slot | `CONTENT.FACTION.MISMATCH`, `UnitPlacements[0].UnitArchetypeId` | AdmissionRejected through the startup loader/validator path |
| Unknown placement slot | `MAP.SLOT.REFERENCE_UNRESOLVED`, `UnitPlacements[0].SlotId` | AdmissionRejected; the valid archetype/location remain unchanged |

For each case, assertions cover diagnostic error severity, code, canonical map ID, property path and actual invalid relationship; lifecycle never reaches Active; runtime/local/system authority, units, objective, selection, queued orders, content-ready flag, step accumulator, simulation-step count, command generation, published/pending gameplay events and result are empty/inactive; and the world contains no live authoritative unit/objective actors. Expected Error-severity validator output is explicitly registered with Unreal Automation so it remains visible without misclassifying correct rejection as an Automation failure.

A further same-process boundary case uses a structurally valid canonical map in a test world without the real level. It confirms the roster is committed, production spatial validation rejects at `MAP.LEVEL.MISMATCH`/missing ground/NavMesh, and recovery retires roster, local binding, Unaligned authority, units, objective and content-ready state. A fresh coordinator in that same process then resolves the canonical map after the one-shot malformed override was consumed. **This is not a successful retry:** the test world lacks the referenced map and navigation data, so the canonical attempt also correctly rejects before Active. Thus AA-15/16/17's complete successful-retry claim and PA-20 remain BLOCKED.

#### AA-01–AA-26 verdicts for this increment

| ID | Verdict | Evidence |
|---|---|---|
| AA-01 | PASS | Call chain above; `BeginPlay` invokes `BuildScenario`, shared structural validation precedes roster commitment, spatial validation follows roster commitment, then unit/objective creation and Active transition |
| AA-02 | PASS | Duplicate slot startup diagnostic and rejection asserted |
| AA-03 | PASS | Duplicate runtime participant startup diagnostic and rejection asserted |
| AA-04 | PASS | Duplicate faction startup diagnostic and rejection asserted |
| AA-05 | PASS | Zero/multiple local-human startup diagnostics and rejection asserted |
| AA-06 | PASS | Unknown placement slot startup diagnostic asserted with other placement fields unchanged |
| AA-07 | PASS | Unaligned playable-slot startup diagnostic asserted |
| AA-08 | PASS | Rejected structural candidates publish no roster; post-roster spatial rejection clears committed roster |
| AA-09 | PASS | Local binding and Unaligned system authority absent after rejection |
| AA-10 | PASS | Coordinator collections and live world actor iteration are empty after structural rejection; post-roster case also leaves no authoritative actors |
| AA-11 | PASS | Selection and pending orders empty |
| AA-12 | PASS | No content-ready match, accumulator or executed steps |
| AA-13 | PASS | Structured diagnostic fields asserted and diagnostics retained on coordinator |
| AA-14 | PASS | Candidate map identity asserted; test hook is consumed once; unknown IDs continue to fail closed in existing production selector |
| AA-15 | BLOCKED (Editor world) / PASS (packaged process) | Editor transient world still cannot load the saved level/NavMesh; rendered package independently reaches canonical Active after rejection |
| AA-16 | PASS (packaged process) | Fresh retry reports two playable participants, local player 1/faction.player, separate Unaligned system authority, canonical 3-unit owner/faction mapping and objective |
| AA-17 | PASS (bounded gameplay step) | New canonical lifecycle traverses phases 1–4 and executes its first authoritative simulation step after the rejected lifecycle is retired |
| AA-18 | PASS | RTS 7/7 and Content 7/7 Automation executed; 3 existing RTS warning-bearing successes; portable core quality passes |
| AA-19 | PASS | UE 5.8.3 pinned Linux Development Editor build passes |
| AA-20 | PASS | Final-source Linux Development normal package and rendered PAC-10 interaction pass |
| AA-21 | PASS | `DA_InvalidParticipantFaction` is isolated under TestFixtures and appears in the fixture-enabled UFS manifest (with the existing schema fixture) |
| AA-22 | PASS | Rendered PAC-07/PAC-09 recovery verifies faction rejection, menu action, same PID, canonical Active retry and MainMenu return |
| AA-23 | PASS | Normal Development and Shipping manifests omit both negative fixtures; final-source Shipping build succeeds and selector strings are absent |
| AA-24 | PASS | Canonical source asset hashes match before/after build, automation and packaging |
| AA-25 | PASS | Source/worktree, engine, package and artifact identity recorded in `build/diagnostics/stage4a-authored-admission/source-identity.txt` |
| AA-26 | PASS | This owning acceptance record updated without promoting unrelated PA rows or broad Stage 4A |

## Identity, asset safety, and limitations

- Test preflight HEAD was `f3f0bde9609abb7aff0f41bc2677a2dddded4b6e` on `main`, with the participant increment present as a dirty working-tree patch. That increment is now committed as `f050962df7564615de4108ad983e155288abcc52` and pushed to `origin/main`; the worktree is clean at that revision.
- The tested Linux Development package retains its historical identity `git_revision=f3f0bde9609a-dirty`, `git_dirty=true`; it was built from the participant implementation as an uncommitted patch and is not an exact-HEAD package for `f050962`. The current source hashes, full dirty-patch hash, test source hash, artifact hashes and engine pin from that test run are recorded in `build/diagnostics/stage4a-participants/source-identity.txt`.
- Final participant Linux Development package executable SHA-256: `b0b0df81f50abb0731146f3931da38439943451161794ac17ae2a59a7ef45af9`; `Politika-Linux.pak/.ucas/.utoc`: `b6178bee8cff3863e19f98a9de594b933dcb758d0e592e6d2c9095399106cd68`, `239d2da011e79d39145fa2bf351d3c97da77ba831371afa0ba11f57cb0924298`, `6b28dc6d0ee5c85ed5ebcb2c5b36f3fdf4d4437840f43b4b51e0165758d27056`.
- Pre-task canonical `.umap`, catalog, map definition, and invalid fixture hashes are in `build/diagnostics/stage4a-lifecycle-closure/baseline.txt`; their final hashes are in `artifact-identity.txt` and match. Direct BuildCookRun did not modify source assets. No restoration or overwrite was necessary.
- The acceptance result is an in-memory prototype result, not persistence. Qualification does not cover physical GPU/display/controller, Shipping rendering, non-Linux platforms, networked play, or a complete match framework.
- Broader Stage 4A still needs standard two-to-four canonical playable-faction setup and autonomous Unaligned controller behavior, among other rules-complete match responsibilities. This increment only records the NPC/system authority; it does not implement its independent simulation. The first economy prototype now selects a fixed starting base as a bounded default; the broader fixed/mobile/hybrid strategic model remains open. Stage 4B harvesting, conversion, production, construction, and reinforcement are not started. Visibility, strategic AI, full victory/defeat rules, persistence, and the complete single-player loop remain roadmap work.

### Roadmap reconciliation verification — 2026-10-28

The authored-startup fixture matrix was refined to include a **valid catalog unit archetype assigned to a different declared slot faction**, not merely a duplicate faction ID or an Unaligned-slot error that also produces secondary placement diagnostics. The case is injected as a transient duplicate and reaches production `BuildScenario` → loader → shared validator. `Politika.RTS.MatchParticipants.AuthoredStartupAdmission` passes with the exact `CONTENT.FACTION.MISMATCH` diagnostic at `UnitPlacements[0].UnitArchetypeId` and the same no-partial-state assertions as the other startup cases.

| Check | Command / report | Observed result |
|---|---|---|
| UE preflight | `UNREAL_ENGINE_DIR=/mnt/data/unreal/5.8.3 bash tools/unreal.sh doctor` | PASS; UE 5.8.3 install and pinned source revision match |
| Portable C++ rules | `cargo run --locked --quiet --manifest-path tools/version/Cargo.toml --bin quality -- core` | PASS; warnings-as-errors rules tests |
| Linux Development Editor build | `UNREAL_ENGINE_DIR=/mnt/data/unreal/5.8.3 bash tools/unreal.sh build` | PASS; compiled the refined `PolitikaAuthoredStartupAdmissionTests.cpp` and linked `PolitikaEditor` |
| Focused authored-startup suite | `Politika.RTS.MatchParticipants.AuthoredStartupAdmission`; `build/diagnostics/stage4a-authored-admission/reconciliation-focused-final/index.json` | PASS; 1 discovered/executed, 0 warning-bearing, 0 failed/skipped |
| RTS regressions | `Politika.RTS`; `build/diagnostics/stage4a-authored-admission/reconciliation-rts-final/index.json` | PASS with existing teardown warnings; 7 executed (4 succeeded, 3 succeeded-with-warnings), 0 failed/skipped |
| Content regressions | `Politika.Content`; `build/diagnostics/stage4a-authored-admission/reconciliation-content/index.json` | PASS; 7 executed, 0 warning-bearing, 0 failed/skipped |
| Integrity checks | `git diff --check`; canonical asset SHA-256 comparison against the values above | PASS; no whitespace errors and the six recorded canonical map/catalog/fixture assets retain their recorded hashes |

At the time of this earlier follow-up, successful malformed-authored rejection → valid Active rematch and cooked malformed-participant recovery were blocked. The focused Editor retry limitation remains; the packaged proof below supersedes only the packaged-recovery gap. The running source at that earlier point was `ede371bd083a42723d979a208671fe2c21a86e2b` plus a dirty worktree; that earlier Automation source hash is retained as historical evidence.

### Cooked malformed-participant recovery qualification — 2026-10-28

This bounded qualification increment uses the existing content loader, shared validator, `BuildScenario`, match coordinator and `UGameInstance` menu recovery. It does not reactivate a retired lifecycle, add a coordinator, or alter normal startup. The Editor test world still correctly reports `MAP.LEVEL.MISMATCH` for its canonical retry because it has no saved RTS level/NavMesh; that Editor retry remains **BLOCKED**. The independent packaged process test resolves the real saved level and navigation and proves the required case-3 recovery contract.

**Fixture:** `/Game/Politika/TestFixtures/DA_InvalidParticipantFaction` (`map.test.invalid_participant_faction`, schema 1) is a genuine cooked Unreal asset copied from the canonical definition. Its only gameplay-field fault is `UnitPlacements[0].UnitArchetypeId = unit.opposing_patrol`; the expected player slot remains `faction.player`, while the valid catalog archetype is `faction.opposition`. The production catalog is unchanged. The dedicated `-CreateInvalidParticipantFactionFixture` commandlet path was inspected and run without `-CreateContent`; it writes only this fixture and does not load/save the canonical RTS level or regenerate navigation. Editor Automation resolves this asset through `LoadAndValidate` and asserts exactly one `CONTENT.FACTION.MISMATCH` diagnostic at the intended property.

**Packaged Development evidence:**

- Normal archive: `Saved/Packages/Stage4AParticipantRecoveryNormal/Linux`; UFS manifest excludes both negative fixtures. `smoke` passes 14/14 at `Saved/ShellChecks/smoke-1793208805604`; rendered PAC-10 passes at `content-acceptance-interaction-1793208816`.
- Fixture archive: `Saved/Packages/Stage4AParticipantRecoveryFixture/Linux`; the manifest includes `DA_InvalidParticipantFaction.uasset` and the retained `DA_InvalidMapSchema.uasset`. It is not a single-fixture-only archive.
- Headless participant acceptance: `content-acceptance-1793208770`; the exact structured mismatch, expected/actual factions, selected map ID, rejected lifecycle, zero roster/local binding/Unaligned authority/units/objectives/selection/pending commands/sequences/steps/content readiness/result, and MainMenu route are observed. Repeated diagnostic, unknown-selector fail-closed behavior, canonical startup controls, and default menu also pass.
- Existing schema regression: `content-acceptance-1793208779` against the same fixture archive; `InvalidMapSchema` still rejects with its schema diagnostic and repeats consistently. The null-renderer run labels PAC-07 `INCONCLUSIVE`; it is not used as rendered evidence.
- Rendered same-process recovery: `content-acceptance-recovery-1793208907`, report `results.tsv`, child PID `146843`. The same retained process handle remains live through invalid startup, MainMenu transition, actual click on the existing prototype action, canonical retry, and Escape return to MainMenu. The harness also verifies there was no `RTS_ORDER_ADMITTED` or simulation-step event before canonical retry. Screenshots `01-rejected-menu.png`, `02-valid-match-after-rejection.png`, and `03-menu-after-valid-retry.png` were inspected; the recovery notice is readable and both gameplay/menu states render at 1280×720.
- The rejected attempt emits `CONTENT.FACTION.MISMATCH|error|map.test.invalid_participant_faction|UnitPlacements[0].UnitArchetypeId|...|expected=faction.player|actual=faction.opposition`, then phase 8 and a zero-state `MATCH_ADMISSION_STATE` record. The retry emits legal lifecycle phases 1, 2, 3, 4 (Active), then `MATCH_SIMULATION_STEP first=1`. It admits two playable participants (local player 1/faction.player and a distinct assigned opponent), a separate Unaligned NPC/system-authority record, canonical owner/faction mapping for three units (players 1/1/2), and `objective.central_control`. This proves a fresh active context and an authoritative simulation step; no command execution is claimed for this recovery harness.

**Shipping isolation:** final-source Linux Shipping `BuildCookRun` succeeds at `Saved/Packages/Stage4AParticipantRecoveryShipping/Linux`. Its UFS manifest contains neither test asset; its binary contains no fixture selector/map ID, `MATCH_SIMULATION_STEP`, or expanded Development rejection-snapshot strings. The detailed snapshot and first-step marker are behind non-Shipping guards. This is a Shipping content/code-isolation result, not a Shipping rendering qualification.

| ID | Verdict | Evidence |
|---|---|---|
| PR-01 | PASS | Actual initial HEAD `ede371bd083a42723d979a208671fe2c21a86e2b`, `main`; intentionally dirty baseline, UE 5.8.3 pin and engine SHA verified |
| PR-02 | PASS | Focused authored-startup report `build/diagnostics/stage4a-participant-recovery/editor-focused-final/index.json`, 1/1 |
| PR-03 | PASS (qualified limitation) | Editor transient-world retry remains blocked at `MAP.LEVEL.MISMATCH`; independent packaged same-process proof below passes |
| PR-04 | PASS | Genuine cooked `DA_InvalidParticipantFaction` in isolated fixture directory |
| PR-05 | PASS | Authored fixture check verifies stable ID/schema/level and placement-only gameplay difference; canonical source hashes unchanged |
| PR-06 | PASS | One-shot Development selector resolves the fixture by explicit ID; unknown ID fails closed |
| PR-07 | PASS | Shared validator diagnostic/property and expected/actual factions observed in both headless and rendered package logs |
| PR-08 | PASS | AdmissionRejected/phase 8 before any Active transition; no content-ready marker for invalid attempt |
| PR-09 | PASS | Coordinator post-cleanup snapshot reports empty participants, local/system authority, world units/objectives, selection, pending commands, sequences, steps, readiness and result |
| PR-10 | PASS | Existing `UGameInstance` route loads and completes MainMenu |
| PR-11 | PASS | Rendered recovery notice inspected in `01-rejected-menu.png`; process/window responsive at verified 1280×720 |
| PR-12 | PASS | Existing MainMenu prototype action clicked through xdotool; no test shortcut added |
| PR-13 | PASS | One child handle/PID 146843 spans rejection and retry; process is checked alive before and after action |
| PR-14 | PASS | Full retry phases 1–4, explicit Active log, plus first authoritative simulation-step marker |
| PR-15 | PASS | Fresh local/opponent/Unaligned roster, unit owner/faction and objective records match canonical authored setup |
| PR-16 | PASS (bounded) | New level/match context after rejected world; canonical identities and fresh unit IDs logged, with no second rejection or stale fixture selection |
| PR-17 | PASS (bounded) | First legitimate authoritative simulation step executes; no command-operation claim |
| PR-18 | PASS | Existing `InvalidMapSchema` path still passes the content acceptance harness |
| PR-19 | PASS | `Politika.RTS` 7/7 (4 clean, 3 warning-bearing); `Politika.Content` 7/7 clean |
| PR-20 | PASS | UE doctor/build, portable core and docs/content audit pass |
| PR-21 | PASS | Normal Development UFS manifest excludes both negative fixtures |
| PR-22 | PASS | Fixture Development manifest contains both the intended participant fixture and retained schema fixture |
| PR-23 | PASS | Shipping package succeeds and excludes fixture assets/selector strings |
| PR-24 | PASS | Canonical level, catalog, map, referenced unit/weapon and existing schema-fixture hashes match preflight |
| PR-25 | PASS | Dirty source, file hashes, engine pin, three package manifests/binary/IoStore hashes, UAT logs and reports captured in `build/diagnostics/stage4a-participant-recovery/`; package identity records the final runtime source state |
| PR-26 | PASS | Owning Stage 4A/content/engine/readme status reconciled without accepting full Stage 4A |

**Bounded verdict at this historical recovery checkpoint:** cooked participant rejection and same-process rendered recovery are qualified for Linux UE 5.8.3 Development. This did **not** complete Stage 4A or First Playable Skirmish. PA-08, PA-10 and PA-11 were then partial; the transient-world Editor retry remained blocked; no gameplay command was exercised by that recovery harness. The 29 October integration below supersedes the PA gaps only for its stated combined-source Development scope. The transient-world Editor retry and broader Stage 4A limitations remain.

## Participant-authority and infantry integration — 2026-10-29

This bounded integration reconciles the qualified participant-authority implementation with the latest rifle-infantry line on GitHub main. It is an integration/regression increment only: no additional infantry mechanics, autonomous Unaligned behavior, Stage 4B economy, multiplayer or broader skirmish systems are claimed.

### Source reconciliation

| Item | Verified identity/result |
|---|---|
| Current remote-main baseline at preflight | `1dcaf03288e2a3bbb5df40fa6a398b3f456e6433` |
| Latest remote-main after mid-qualification advancement | `8cdc9429695fb67bdcf977aed066f49d51fb2125` (`tools: add offline Tempest reference indexer`); incorporated by rebasing the scoped integration series with no conflicts |
| Qualified authority branch/tip | `codex/stage4a-participant-authority-20261029` / `71e4e3a225f95539593fb74a1b29da371f3a9c3e` |
| Actual merge base | `83ec26703886a32d857291f4380e2bc2524f6c7c` |
| Pinned Unreal source revision | `396c9f059903aed5fec78ecd3d437a40c6415368` (UE 5.8.3; clean at preflight) |
| Isolated worktree | `/mnt/data/politika-stage4a-authority-infantry-integration-20261029` |
| Integrated gameplay/test changes after latest-main rebase | `de980bb` plus the scoped preceding authority/harness/bootstrap commits; exact outgoing chain is in the external evidence manifest |
| Scoped replay-harness refinements | Exact commit chain is recorded in `build/diagnostics/stage4a-integration/final-evidence.json` |
| Final docs/integration commit and package identity | See `build/diagnostics/stage4a-integration/final-evidence.json`; this generated evidence file is outside Git and names the exact source and runtime identity |

The divergent histories shared changes in `PolitikaPrototypeGameplay.cpp` and `.h`. The gameplay source was reconciled manually: authority commandability/selection revalidation, stale queued-issuer rejection and death-selection cleanup are retained alongside current infantry grenade simulation, typed Grenade/Reload/Sprint variants, combat, movement and presentation. The header retains authority test access and the latest rifleman state/API. Four scoped authority/harness/bootstrap commits were selected; unrelated history and binary assets were not imported. During qualification GitHub main advanced with the separate Tempest-reference-indexer commit listed above. The eight added tool files were preserved by rebasing the scoped integration commits onto that current tip; the final qualification and publication checks use the rebased source. The Git-worktree-aware bootstrap dependency fix is retained.

The current player-controller ability inputs now resolve `GetCommandableSelectedUnit` before forwarding Grenade, Reload or Sprint. Ability availability remains an additional check after participant/issuer/ownership/faction admission. Combined assertions exercise foreign-owned Grenade/Reload/Sprint and local attempts against system-owned infantry, verifying `NotOwned`, unchanged order/ammunition/cooldowns/health and an empty pending queue. Stale dead cached selection is retired before each ability input. Existing death presentation keeps the actor available for presentation while removing it from commandable membership and selection.

### Qualification evidence

The committed gameplay integration source passed Linux UE 5.8.3 Development Editor and Game builds; portable core and repository audit; Rust Shell tests (6 executed, 6 passed) and formatting; and Unreal Automation: focused participant authority, full `Politika.RTS` (16 warning-bearing successes, 0 failures), `Politika.Content` (7/7), and `Politika.Infantry` (4/4). The warning-bearing Automation successes include the headless SDL Wayland-before-initialization warning; test assertions reported zero errors. Exact report files and final-source reruns are enumerated in `build/diagnostics/stage4a-integration/final-evidence.json`.

A fresh Linux Development UAT package from the final integration source includes the current RifleInfantry archetype/behavior/presentation, mesh/material, animation and audio dependencies; the normal package contains no separately authored authority fixture. The manifest explicitly verifies these cooked dependencies. The package sidecar, generated runtime identity and actual startup `git_revision` must all match; archive hash verification and executable/content hashes are in the external final evidence manifest.

Two separate fresh rendered package runs are required and recorded there: participant-authority mode (`--participant-authority`) and canonical PAC-10 mode (no authority selector). Each uses a separate process, fresh `-UserDir`, isolated runtime log and 1280×720 rendered capture. The authority replay asserts authorized local interaction, foreign/system/unassigned/dead/destroyed rejection, stale-selection retirement and preservation of legitimate opposing state. The canonical replay validates selection, Move, Sprint, Guard, AttackMove's valid movement-or-in-range-combat path, targeted Grenade launch/impact, Reload admission/execution, friendly-target rejection, rifle damage/death, objective state, camera/render change and return to MainMenu. Historical package runs are not substitutes for the exact final identity recorded in the evidence manifest.

### Current bounded verdict and limits

| Requirement | Verdict |
|---|---|
| PA-08 — invalid/dead/destroyed/non-member selections and issuers | PASS for the tested Linux Development scope |
| PA-10 — opposing playable participant selection/command rejection | PASS for the tested Linux Development scope |
| PA-11 — unassigned/system/Unaligned authority is not ordinary player authority | PASS for the tested Linux Development scope; no autonomous Unaligned behavior is implied |
| Existing implemented infantry prototype and typed orders | PASS for the tested combined scope; only currently supported rifleman features are claimed |
| Integrated-source package identity and rendered replays | See exact final-source verdicts in `build/diagnostics/stage4a-integration/final-evidence.json` |
| Broad Stage 4A / full skirmish / Shipping / other platform | NOT ACCEPTED |
| NAV-A15 | INCONCLUSIVE; unchanged |

Stage 4A remains incomplete: standard participant/faction orchestration, full match rules, economy/production, autonomous playable opponent, broader recovery cases, Shipping rendering and other platform qualification remain outside this increment. The transient-world Editor retry remains blocked by its missing authored level/NavMesh precondition.
