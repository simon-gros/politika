# POLITIKA — RTS Runtime Navigation Increment

Status: **source implemented; NAV-A25 cooked static-obstacle detour PASS; NAV-A20 five-schedule repeatability PASS for the identified Linux Development snapshot; broader navigation milestone incomplete** (2026-10-29). This is a bounded continuation of the existing tactical prototype, not Stage 4 completion or First Playable Skirmish acceptance.

> **Future formation design handoff (29 October 2026; no change to NAV evidence):** [Military Formations, Tactical Stances, Cohesion and Discipline](RTS_UNIT_FORMATIONS_AND_DISCIPLINE.md) proposes per-member legal formation slot routes, distinct admission versus route outcomes, bounded regrouping and crowd/scale performance validation. Current NAV-A20 repeatability and NAV-A25 cooked detour qualify their documented *single-unit* cases; they do **not** qualify group-formation movement, local avoidance, mixed unit speeds or massive formation pathfinding. FORM-F0 requires separate measurement and applicable NAV failure/lifecycle closure before FORM-F2. All historical acceptance verdicts remain as recorded.


## Baseline and decision

- Local `/mnt/data/politika` was authoritative: `main`, HEAD `7d6aab9d730a707e898012bbe7acf8d4d6ea470e`, aligned with `origin/main`, clean at preflight. No existing source changes were overwritten. UE doctor verified `/mnt/data/unreal/5.8.3` and pinned source `396c9f059903aed5fec78ecd3d437a40c6415368`.
- Existing unit behavior directly interpolated toward the requested destination under the coordinator's 0.05 s step and swept actor movement. Stage 3 already selected default navigation data and projected starts/goals with an 80×80×180 cm extent; it checked authored start-to-objective path length.
- Selected: UE default Recast navigation data, synchronous `FPathFindingQuery` at accepted Move/Attack-Move execution, copied match-local waypoint state, authoritative fixed-step following. The project already depends on `NavigationSystem`; no additional UE module was required.
- Rejected for this increment: `UPathFollowingComponent`/AI `MoveTo` because it would own movement outside POLITIKA's coordinator; a custom movement component because no measured need justifies another scheduler; Mover/NavMover and MassNavigation because they add unrelated framework/ownership complexity; custom grid/graph because Stage 3 has already authored Recast connectivity; async queries because this prototype has no measured blocking/scaling evidence and would add completion ordering/stale callback concerns. Existing straight-line interpolation remains unsuitable around blockers. Async/budgeted queries, strategic graphs, dynamic topology and crowd handling require later measured need.
- Unreal 5.8 documentation reviewed: [Navigation System](https://dev.epicgames.com/documentation/unreal-engine/navigation-system-in-unreal-engine?application_version=5.8), [Basic Navigation](https://dev.epicgames.com/documentation/unreal-engine/basic-navigation-in-unreal-engine?application_version=5.8), [UNavigationSystemV1](https://dev.epicgames.com/documentation/unreal-engine/API/Runtime/NavigationSystem/UNavigationSystemV1?application_version=5.8), [FNavigationPath](https://dev.epicgames.com/documentation/unreal-engine/API/Runtime/NavigationSystem/FNavigationPath?application_version=5.8), [UPathFollowingComponent](https://dev.epicgames.com/documentation/unreal-engine/API/Runtime/AIModule/UPathFollowingComponent?application_version=5.8), [NavMesh modification](https://dev.epicgames.com/documentation/unreal-engine/overview-of-how-to-modify-the-navigation-mesh-in-unreal-engine?application_version=5.8), [Movement Components](https://dev.epicgames.com/documentation/unreal-engine/movement-components-in-unreal-engine?application_version=5.8), [Unreal Insights](https://dev.epicgames.com/documentation/unreal-engine/unreal-insights-in-unreal-engine?application_version=5.8). Exact query/result and path point signatures were checked in the pinned `NavigationSystem.h`, `NavigationSystemTypes.h`, `NavigationData.h`, and `NavigationPath.h`.
- UE 5.8.4 was released 2026-10-20. Epic's [official hotfix notes](https://forums.unrealengine.com/t/5-8-4-hotfix-released/2842775) list fixes/features in Visual Studio integration, Sequencer/Mover/Landscape/modeling/MetaHuman/Movie Render stability, rendering/shaders, UAT logging/Horde extraction, Mac/iOS packaging and EOS; they do not establish a relevant Linux Recast navigation fix. The project remains pinned to 5.8.3; no migration was attempted.

## Implemented contract

- Route planning uses the existing default navigation data after successful typed command/participant admission. A complete route must project the origin and goal within the fixed query bounds, have finite path points, be non-partial, and end within 150 cm of the projected goal. It is rejected for missing data, build-in-progress, non-navigable endpoints, partial/failed/empty paths or bad points. There is no straight-line fallback.
- A unit stores resolved navigation destination, waypoint array/cursor, match generation and order sequence. A new order replaces route state; Stop and order modes without a route clear it. Leaving Active clears movement, and a mismatched match generation fails closed. Synchronous queries cannot return late callbacks; order sequence remains recorded for diagnostics/future safety.
- Only `APolitikaPrototypeMatch::RunSimulationStep` advances waypoint routes. Each 0.05 s step is capped at authored `MoveSpeed * DeltaSeconds`, consumes at most 16 route points, uses the existing swept `SetActorLocation`, stops on a blocking hit, and completes Move/Attack-Move within 18 cm. Waypoint elevation is derived relative to the projected start, not forced to a global ground Z.
- Attack/Guard target approach shares NavMesh queries on initial route need or a target displacement above 100 cm. Exhausted target routes have a maximum of three attempts with a 0.5 s cooldown; a materially relocated target resets that bounded retry count. It does not query every step. Attack-Move combat acquisition continues through existing combat resolution. Stop/replacement/death/lifecycle cleanup discard route state.
- Existing typed command admission remains authoritative. The command event reports successful intent admission; if no usable route can be established, a subsequent `OrderChangedEvent` with `NavigationFailed` makes the executable-order failure explicit. No schema version changed.

## Original increment verification evidence (before cooked-obstacle qualification)

| Gate | Verdict | Evidence |
|---|---|---|
| Source/engine identity | PASS | `main` / `7d6aab9`; `tools/unreal.sh doctor` verified project, UE 5.8.3 and pinned SHA. |
| Editor target build | PASS | `UNREAL_ENGINE_DIR=/mnt/data/unreal/5.8.3 bash tools/unreal.sh build` succeeded on final source. |
| Navigation Automation | PASS | `Politika.RTS.Navigation`: 2 discovered/executed; 2 succeeded-with-warnings (expected warning logs), 0 failed/skipped. Report: `build/diagnostics/runtime-navigation/final-source-Politika-RTS/index.json`. |
| Command/lifecycle/participant regressions | PASS | `Politika.RTS`: 9 discovered/executed; 3 clean, 6 succeeded-with-warnings, 0 failed/skipped. |
| Content regressions | PASS | `Politika.Content`: 7 discovered/executed, 7 passed, 0 warning-bearing/failed/skipped. |
| Saved-map obstacle detour | PASS (Editor Automation) | Opens `/Game/Maps/RTSVerticalSlice`, adds an unsaved static blocking wall, builds navigation in-memory, proves the direct segment hits the wall, routes around it, executes through the production coordinator, maintains authored speed, keeps the collision body outside the blocker, and emits one arrival. This is not the cooked obstacle gate. |
| Missing navigation | PASS | Actual navigation-disabled transient world; a typed Move is admitted but terminated as NavigationFailed with unchanged position and no fallback. |
| Fixed-step route repeatability | INCONCLUSIVE | Existing 30-step schedules agree for no-NavMesh fail-closed behavior and the rest of the script, but no same-route fixture was replayed under multiple render delta schedules. |
| Linux Development package build | PASS | Pinned UE 5.8.3 Linux Development `BuildCookRun`, normal cook (no fixture selector/CookDir), archive `Saved/Packages/RuntimeNavigationMilestoneFinal/Linux`; log `build/diagnostics/runtime-navigation/package-milestone-final/buildcookrun.log`. |
| Packaged rendered interaction | PASS | Existing `rts_interaction` acceptance ran from MainMenu at 1280×720, exercised movement commands and query logs, observed `OrderChanged ... reason=5` (Move arrival), exercised combat/objective/event regressions and returned to MainMenu. Evidence: `Saved/Packages/RuntimeNavigationMilestoneFinal/Linux/content-acceptance-interaction-1793210728/`; PAC-10 result in `results.tsv`. |
| Packaged obstacle navigation | NOT_RUN | The obstacle was transient and Editor-only; no cooked Development obstacle fixture was created or executed. |
| Performance sample | INCONCLUSIVE | Four successful packaged route-query samples on the exercised single-unit route were 14.9, 11.8, 13.0 and 14.7 μs (Development diagnostic timing, Intel Core Ultra 9 285K / Linux CachyOS 7.3.0). This is a smoke sample, not a benchmark or evidence for 8/32/many units; per-step cost, memory/cohorts and scaling remain unmeasured. |
| Shipping isolation | NOT_RUN | No Shipping rebuild was performed. Development-only test remains guarded by Unreal Automation compilation; no new Shipping claim is made. |
| Canonical assets | PASS | Pre-package SHA-256 values for the saved map, catalog, map definition, unit and weapon assets were saved at `build/diagnostics/runtime-navigation/canonical-assets-before-package.sha256`; `sha256sum -c` passed after the Editor obstacle run and package build. No `.umap`/`.uasset` appears in the source diff. |

Final package identity is `7d6aab9d730a-dirty`, game `0.1.0-dev.0`, engine `5.8.3-0+UE5`, schema counters unchanged at 1. Executable SHA-256 is `d71c30649342a7e02aa068f0f2c2744c57c5720fb2d8676be411988e233e6195`; Pak/IoStore hashes are in `build/diagnostics/runtime-navigation/package-milestone-final/artifact-sha256.txt`. The package is local verification evidence, not a release. The interaction replay produced rendered screenshots/video plus authoritative runtime logs.

## Acceptance matrix (NAV-A01–NAV-A30)

| Gate | Verdict | Qualification |
|---|---|---|
| NAV-A01 Source and UE pin verified | PASS | Local HEAD `7d6aab9`, UE 5.8.3 pinned SHA doctor check. |
| NAV-A02 Existing movement baseline recorded | PASS | Direct `VInterpConstantTo` baseline and 0.05 s coordinator identified before replacement. |
| NAV-A03 Architecture selected/documented | PASS | ADR and alternatives above. |
| NAV-A04 Runtime uses actual UE navigation data | PASS | Runtime `FPathFindingQuery` against default Recast data. |
| NAV-A05 Valid Move produces a complete route | PASS | Saved-map Editor test and packaged successful query/arrival. |
| NAV-A06 Waypoints follow through 20 Hz coordinator | PASS | Editor obstacle test invokes `RunSimulationStep`; package runtime also emitted movement/arrival. |
| NAV-A07 Static blocker detour | PASS (scoped cooked fixture) | Original Editor evidence retained above; final cooked qualification below proves the real blocker and complete alternate route. |
| NAV-A08 Collision prevents obstacle penetration | PASS (scoped cooked fixture) | Actual scaled collision-body sweeps and independent numerical swept-sphere checks cover all route and traversal segments. |
| NAV-A09 Stable, single arrival | PASS (scoped cooked fixture) | Exactly one production termination and one event publication, followed by 20 additional authoritative steps. |
| NAV-A10 Attack-Move route/combat semantics | INCONCLUSIVE | Packaged Attack-Move and combat regression ran, but target acquisition/resume over a valid detour was not isolated. |
| NAV-A11 Attack/Guard approach around blockers | INCONCLUSIVE | Shared code path exists; no target-behind-blocker fixture was run. |
| NAV-A12 Stop cancellation | PASS (valid cooked route, identified Linux Development snapshot) | OI-01 below: post-step-40 typed Stop admitted at 41 before movement, route/identity cleared, one Stopped publication, stationary through 81 including duplicate Stop at 61; four packaged processes. |
| NAV-A13 Replacement order invalidates old route | PASS (valid cooked route, identified Linux Development snapshot) | OI-02 below: one Replaced at 41, fresh complete return route installed/executed, one replacement arrival and no stale original arrival; four processes. OI-03 independently proves admission rejection preserves the executing route. |
| NAV-A14 Unreachable destination failure | PASS (cooked Linux Development, 2026-10-29) | New isolated disconnected-region qualification below; valid endpoints, production no-complete-path query, retired route, exactly-once failure and 21 inert steps in two fresh processes. |
| NAV-A15 Missing/invalid/partial nav cannot silently succeed | INCONCLUSIVE | DP-01 and DP-02 destination-projection subcases plus NQ-01/NQ-02 have scoped packaged evidence; empty/invalid route-result subcases remain unqualified. No silent fallback is observed in the qualified cases. |
| NAV-A16 Replan behavior is bounded | INCONCLUSIVE | Source sets three target retries / 0.5 s cooldown; exhausted-target/replan tests not run. |
| NAV-A17 Dead units stop routes | INCONCLUSIVE | Death/cleanup regression passes, but unit death while navigating a route is not isolated. |
| NAV-A18 Inactive match stops routes | INCONCLUSIVE | Lifecycle and fail-closed movement regressions pass; active valid-route termination at phase exit is not isolated. |
| NAV-A19 Stale order/match routes rejected | INCONCLUSIVE | Synchronous query removes late callbacks and generation is checked; installed-route stale identity tests not run. |
| NAV-A20 Fixed-step route repeatability | PASS (identified Linux Development snapshot) | Ten independent cooked-runtime executions, five frozen update schedules, complete matching authoritative-step traces; see the 2026-10-29 qualification below. |
| NAV-A21 Existing RTS command/combat/objective/event regressions | PASS | 9/9 `Politika.RTS` and rendered PAC-10 regression. |
| NAV-A22 Content validation/map integrity | PASS | 7/7 `Politika.Content`; canonical asset hashes unchanged. |
| NAV-A23 Final-source Editor build/Automation | PASS | UE Editor target build; 9 RTS plus 7 content tests with zero failed/skipped. |
| NAV-A24 Linux Development package | PASS | Original rendered replay retained above; final follow-on normal/fixture packages build and boot MainMenu, with the cooked detour below. |
| NAV-A25 Cooked obstacle scenario | PASS | Final-source UE 5.8.3 Linux Development executable/cooked level; SO-05–SO-14 pass in the qualification below. |
| NAV-A26 Rendered movement evidence | PASS | 1280×720 package screenshots/video and authoritative route/arrival logs. |
| NAV-A27 Performance baseline | INCONCLUSIVE | One-unit query timing smoke samples only; no cohort, follower cost or memory baseline. |
| NAV-A28 Source/assets preserved | PASS | Original clean increment and this dirty follow-on remain distinguished; 13 pre-existing assets unchanged, only isolated fixture assets added; no reset/commit/push. |
| NAV-A29 Source/engine/package/evidence traceability | PASS | Exact dirty source-input/patch/untracked hashes, pinned engine, matching final package identities and measured runtime report below. |
| NAV-A30 Documentation/status integrity | PASS | This record does not claim Stage 4 or First Playable acceptance. |

## Acceptance boundary and next dependency

The bounded source foundation, cooked static-obstacle gate, identified-snapshot NAV-A20 schedule repeatability and valid-route NAV-A12/A13 interruption below are qualified. The broader milestone remains incomplete: NAV-A10–A19 still include unqualified failure/target/lifecycle behavior and NAV-A27 lacks scale evidence. The next dependencies are separately bounded failure and target-approach qualifications. Do not promote Stage 4A or start First Playable Skirmish.

## Cooked static-obstacle qualification — 2026-10-28

### A. Initial state and exact identity

This follow-on started on **dirty `main`**, HEAD/origin-main `7d6aab9d730a707e898012bbe7acf8d4d6ea470e`. Initial navigation source, three modified coordinator tests, `Rules.h`, four documentation edits, the untracked acceptance document, runtime navigation tests and Editor obstacle test were authoritative pre-existing work. The original increment's earlier clean preflight above is historical, not this follow-on's initial state. `NAV-A25` started **NOT_RUN**.

Evidence root, abbreviated **D** below: `/mnt/data/politika/build/diagnostics/navigation-static-obstacle-20261028/`. Initial status, full tracked binary patch, untracked inventory and original untracked archive are `D/initial-status.txt`, `initial.patch`, `initial-untracked.txt`, `initial-untracked.tar`. The requested preflight commands, including staged diff and whitespace check, completed without reset/clean/checkout/branch switch/merge/rebase/commit/push. Final branch and HEAD are unchanged; the dirty worktree is retained.

Pinned installation: `/mnt/data/unreal/5.8.3`, UE **5.8.3**, engine Git HEAD **`396c9f059903aed5fec78ecd3d437a40c6415368`**, Editor and game targets Linux Development. Doctor passes project/install/revision (`D/engine-doctor.json`). Game remains `0.1.0-dev.0`; compatibility counters remain 1.

The exact accepted build-input manifest is `D/accepted-source-inputs.sha256`, covering all source/config/tool inputs, fixture and canonical assets, project and version files. Its SHA-256 is **`f322a9162e8afb1212d690e600568c549b1fda3976d2b285ee21cef7719dbe7d`**. Both accepted archives contain that value in `QualificationSource.sha256`, alongside `BuildIdentity.json` (`7d6aab9d730a-dirty`) and `unreal-version.env`. Every listed input passed post-package verification in `D/accepted-source-inputs-final-check.txt`. Evidence-only documentation updates occur after compilation/cooking and do not change those inputs.

Accepted pre-documentation tracked patch: `D/accepted-build.patch`, SHA-256 **`6ab71b7cb89746d7b3669e987b03140610b82dabea1a58377ace46b8162a1c31`**. Untracked source/fixture hashes are separately recorded in `D/accepted-untracked-hashes.txt`; HEAD or this tracked patch alone cannot identify the dirty build.

### B. Fixture and minimum implementation

- Cooked level: `/Game/Politika/TestFixtures/Navigation/W_StaticObstacleDetour`.
- Definition: `/Game/Politika/TestFixtures/Navigation/DA_StaticObstacleDetour.DA_StaticObstacleDetour`, ID `map.test.static_obstacle_detour`.
- Explicit non-Shipping selector: `-PolitikaTestFixture=StaticObstacleDetour`, with the explicit level URL. Without both the intended map and fixture selection, ordinary startup is unchanged; the fixture is not a normal catalog map.
- Shared Editor/runtime-fixture geometry constants and static solid-interior authoring helper: `Source/Politika/Public/PolitikaNavigationObstacleFixture.h`.
- Authoring: dedicated early-return `-CreateStaticObstacleDetourFixture` branch in `PolitikaCreateShellCommandlet.cpp`. It refuses to overwrite an existing fixture, modifies the canonical level only in memory, saves only the named fixture destination, reloads it, and runs the unchanged map structural/spatial validators. It never invokes `-CreateContent` or the canonical map/negative-fixture regeneration branches.
- The fixture duplicates the existing validated map definition, preserves the two admitted participant slots and canonical catalog/unit/weapon references, places **one unchanged `unit.mechanized_fireteam`** at the authored start, and moves only this fixture's objective to `(0,1200,4)` so it cannot terminate the movement scenario. Its speed is **210 cm/s**, not an acceptance-specific speed.
- Static obstacle: tagged `PolitikaNavigationStaticObstacle`, Engine Cube, location `(0,-600,800)`, scale `(2,5,20)`, axis-aligned collision bounds `(-100,-850,-200)` to `(100,-350,1800)`, Static mobility, BlockAll, navigation relevance enabled. A saved static `UNavModifierComponent` with `UNavArea_Null` excludes its solid interior. Ground/nav bounds are copied, not regenerated in the canonical level. Runtime Recast generation is **Static (0)**; agent radius **35 cm**.

The early authoring probes exposed two fixture issues, not a production navigation defect: pending mesh compilation suppressed navigation relevance, and the tall mesh surfaces enclosed a disconnected walkable island inside solid collision. The authoring path now finishes mesh compilation, flushes navigation updates and explicitly excludes the solid interior. Failed/partial attempts and logs remain under D (`partial-first-save.umap`, `partial-v3.umap`, `fixture-v2/`, `fixture-authoring-v*.log`); no historical evidence was deleted. The successful authoring run is **`fixture-authoring-v12.log`, exit 0, zero validation diagnostics**. A conflicting UBT attempt is retained in `package-NormalFinal.log`; it was an infrastructure mutex conflict, not a missing SDK. Subsequent compiler/package actions were serialized.

Runtime instrumentation is isolated in `PolitikaNavigationAcceptance.cpp`, with small non-Shipping hooks in the existing coordinator/query/publication pathway and opt-in unit swept-subsegment collection. The production query and follower were **not replaced or behaviorally repaired**. Only `RunSimulationStep(0.05)` advances movement; no Actor Tick movement, timer, AI MoveTo, direct relocation, supplied waypoint list or Rust-side simulation was introduced. The trusted Development scenario calls the existing `SubmitMove` typed-command path and still passes participant, ownership, generation and command checks. The existing Rust `rts_interaction` binary selects its narrow observer module through `--static-obstacle`; no new pathfinding system or scenario framework exists.

Task implementation files: the new acceptance `.cpp`, shared fixture header, Rust observer module and two fixture assets; narrow modifications to `PolitikaPrototypeGameplay.cpp/.h`, `PolitikaPrototypeContent.cpp`, the existing Editor obstacle test, `PolitikaCreateShellCommandlet.cpp`, and `Tools/Shell/src/bin/rts_interaction.rs`. Documentation changes are confined to this owner and necessary status/fixture-usage references. Unrelated pre-existing tests/rules/doc changes remain intact.

### C. Packaged geometry and production navigation

Final runtime evidence directory, abbreviated **R**: `/mnt/data/politika/Saved/Packages/Stage4ANavigationObstacleFixtureAccepted20261028/Linux/navigation-obstacle-acceptance-1793212464/`.

`R/static-obstacle.log` establishes cooked level resolution, structural/spatial validation, participant admission, legal lifecycle progression into Active, entity **1**, owner **1**, match generation **1**, Move sequence/correlation **1**, and `movement.prototype_ground`. Navigation data is the cooked level's `PersistentLevel.RecastNavMesh-Default`, ready before the query; no runtime rebuild is enabled.

The direct collision trace identifies the tagged wall itself (`hit=1`, collision enabled/blocking); an actual full-body sweep also blocks, and the interior projection returns no navigable point (`nav_excluded=1`). The default production `FindPathSync` returns **Success (3), partial=0**, projecting start/goal to Z=-10 and resolving the final actor destination back to Z=42. Requested start/goal are unchanged horizontally and are legal navigation positions.

Ordered **production** route, in centimeters:

```text
(-700,-600,42) -> (-133,-893,42) -> (133,-893,42) -> (800,-600,42)
```

Path length **1632.748039 cm**, direct distance **1500 cm**. Every route segment is clear under an actual scaled body sweep against the intended wall; independent Rust swept-sphere/rectangle checks corroborate the measured trajectory. This is geometric detour proof, not a waypoint-count assertion. The horizontal section stays 43 cm from the wall edge; the body radius is approximately 42.500004 cm. The body setup is checked at runtime to contain exactly one centered sphere; the visual bounding extent `(42.5,42.5,67.5)` is not incorrectly substituted for its round collision footprint.

### D. Authoritative traversal and arrival

R contains **176 consecutive authoritative step samples** and **157 actual swept subsegments**. Before each step, the previously observed position must still match; every subsegment is collected immediately after the production swept `SetActorLocation`. After each step, ownership, Active phase, current generation/sequence, collision enablement/responses, unchanged blocker bounds/body scale and route progression are checked. The Rust observer independently checks consecutive step identities, ordered subsegments, route proximity/cursor progression, full-body clearance, measured travel, speed budget and absence of unobserved movement.

The fixed-step budget is **210 × 0.05 = 10.5 cm**. Independent reconstructed maximum travel is **10.500001 cm**, within the documented **0.01 cm** floating-point tolerance. Travel is summed over actual subsegments, including steps that consume a waypoint; it is not merely the endpoint chord or rendered frame delta. Collision verification uses the actual unit component geometry swept against the cooked wall and the independently measured radius/box geometry; every segment passes.

Arrival occurs at **step 156**, final actor/resolved destination **`(800,-600,42)`**, measured destination error **0 cm** (production tolerance 18 cm). The Move becomes Idle; the waypoint array is empty and cursor zero. Production returns **one `OrderEndReason::Arrived` termination** and publishes **one `OrderChangedEvent` with Arrived** for this entity/order; command admission is observed exactly once and no obstruction/navigation failure is reported. The trace counts calls at actual publication, not retained snapshot observations. Original event identity is entity+step; its sequence is joined through the continuously checked current navigation order rather than adding a new event contract. **Twenty additional normal simulation steps** (through step 176) keep the retired path inert and do not republish arrival. Only then is terminal PASS emitted and orderly exit requested. Process exit 0 is corroboration, not the acceptance assertion.

`R/results.tsv` records **NAV-A25 PASS**; `R/artifact-identity.txt` joins the executable hash and exact dirty input identity. This final scenario ran **headless (`-nullrhi`)**: no new screenshot, rendered obstacle qualification or physical-device claim is made. Earlier rendered package evidence above remains historical evidence for its own source/artifact.

### E. Automation and quality

Exact final-source invocation pattern (one process/report per filter):

```sh
/mnt/data/unreal/5.8.3/Engine/Binaries/Linux/UnrealEditor-Cmd \
  /mnt/data/politika/Politika.uproject -unattended -nop4 -nullrhi -nosplash \
  '-ExecCmds=Automation RunTests Politika.RTS.Navigation;Quit' \
  -ReportExportPath=/mnt/data/politika/build/diagnostics/navigation-static-obstacle-20261028/qualified-Politika-RTS-Navigation \
  -ABSLOG=/mnt/data/politika/build/diagnostics/navigation-static-obstacle-20261028/qualified-Politika-RTS-Navigation/run.log
```

The same command used `Politika.RTS` / `qualified-Politika-RTS` and `Politika.Content` / `qualified-Politika-Content`. Each process exited 0; exported `index.json` reports were inspected, not inferred from exit status.

| Filter | Discovered/executed | Clean successes | Warning-bearing successes | Failed / skipped / in process |
|---|---:|---:|---:|---:|
| `Politika.RTS.Navigation` | 2/2 | 0 | 2 | 0 / 0 / 0 |
| `Politika.RTS` (includes Navigation) | 9/9 | 3 | 6 | 0 / 0 / 0 |
| `Politika.Content` | 7/7 | 7 | 0 | 0 / 0 / 0 |

Exact navigation registrations remain `Politika.RTS.Navigation.MissingNavigationFailsClosed` and `Politika.RTS.Navigation.SavedMapStaticObstacleDetour`. The focused warnings are the intended `NavigationUnavailable` negative and Editor map loading's `UCrowdManager`/Recast warning. Broader RTS warnings include expected missing-navigation and transient-world cleanup diagnostics; none are silently reported as clean passes. `D/automation-summary.json` retains every discovered name and warning/error count. This gives 16 distinct RTS/content tests, not 18 independent tests: the two Navigation tests also occur in RTS.

The pinned Editor build passes (`D/editor-build-authoring-v12.log`); UAT subsequently reconfirms the Editor target. Portable C++ `quality -- core` and repository `quality -- audit` pass (`quality-core-final.log`, `quality-audit-final.log`). Rust `cargo test --locked --manifest-path Tools/Shell/Cargo.toml --all-targets` passes with **3 substantive verifier tests**, zero failures (`D/rust-tests-accepted.log`); other harness targets contain zero unit tests and are not counted as behavioral validation. `rustfmt --edition 2024 --check Tools/Shell/src/bin/rts_interaction.rs` checks the edited binary and its observer module. Unrelated baseline formatting was preserved. An initially incorrect synthetic clearance-test expectation was corrected before the accepted archives; the earlier failed test log remains under D. `git diff --check` passes.

### F. Accepted packages and reproducible commands

Both accepted archives used:

```sh
/mnt/data/unreal/5.8.3/Engine/Build/BatchFiles/RunUAT.sh BuildCookRun \
  -project=/mnt/data/politika/Politika.uproject -noP4 -platform=Linux \
  -clientconfig=Development -build -cook -stage -pak -iostore -archive \
  -unattended -utf8output -UbtArgs=-MaxParallelActions=6 \
  -archivedirectory=/mnt/data/politika/Saved/Packages/Stage4ANavigationObstacleNormalAccepted20261028
```

Fixture archive changes only archive directory to `Stage4ANavigationObstacleFixtureAccepted20261028` and adds **`-CookDir=/mnt/data/politika/Content/Politika/TestFixtures/Navigation`**. Logs `D/package-NormalAccepted.log` and `package-FixtureAccepted.log` both report `BUILD SUCCESSFUL`, UAT exit 0. No accepted historical archive was overwritten. Earlier task-local Normal/Qualified packages also remain, but are not substituted for the final accepted artifacts.

Both launchable executable paths end in **`Linux/Politika/Binaries/Linux/Politika`** (actual case verified, not the historical uppercase spelling). Both executable SHA-256 values are **`c6af3ce93a78b48d9550284f294535b6f606abd3052598a8b43f5a95299bb107`**.

| Artifact | Normal accepted SHA-256 | Fixture accepted SHA-256 |
|---|---|---|
| `Politika-Linux.pak` | `b6178bee8cff3863e19f98a9de594b933dcb758d0e592e6d2c9095399106cd68` | `2d2801d998b9c4a3d48f392b45752acb183a2f0f5d8a8803ee1d3dc819c5f476` |
| `Politika-Linux.ucas` | `239d2da011e79d39145fa2bf351d3c97da77ba831371afa0ba11f57cb0924298` | `a452c12d3c1bf641d0e1c8a7b579d633ca5297a78bf56a3a6e66132269a3de64` |
| `Politika-Linux.utoc` | `6b28dc6d0ee5c85ed5ebcb2c5b36f3fdf4d4437840f43b4b51e0165758d27056` | `3c2e359904422461c4d7dc455e2d221dfc26f8c7fe84a62e48fdf1d07f9633e9` |

Full artifact/global IoStore hashes: `D/accepted-package-artifact-identity.txt`. Actual UFS/NonUFS manifests are retained at each archive's `Linux/` root. Normal UFS contains **no `TestFixtures/` entries**. Fixture UFS contains exactly these additional test assets, with no schema/participant negative fixture:

```text
Politika/Content/Politika/TestFixtures/Navigation/W_StaticObstacleDetour.umap
Politika/Content/Politika/TestFixtures/Navigation/DA_StaticObstacleDetour.uasset
```

Their real runtime load, NavMesh query, wall collision and traversal establish usable cooked dependencies beyond a source-file/manifest-only claim. No default-map, Asset Manager, always-cook directory, permanent packaging config or production catalog change was made. Both package executables are identical and their source sidecars match. The harness separately boots **both packages without a selector**, observes normal MainMenu startup and absence of navigation acceptance, and exits them cleanly (`R/normal-startup.log`, `R/fixture-ordinary-startup.log`).

Exact final harness command, exit 0 (`D/packaged-harness-accepted.log`):

```sh
cargo run --locked --manifest-path Tools/Shell/Cargo.toml --bin rts_interaction -- \
  /mnt/data/politika/Saved/Packages/Stage4ANavigationObstacleFixtureAccepted20261028/Linux/Politika/Binaries/Linux/Politika \
  --static-obstacle \
  /mnt/data/politika/Saved/Packages/Stage4ANavigationObstacleNormalAccepted20261028/Linux/Politika/Binaries/Linux/Politika
```

It verifies package identity/manifests, launches the real executable with explicit map/selector, fresh isolated `-UserDir` and `-ABSLOG`, waits at most 90 seconds per process, rejects crashes/missing terminal evidence/stalls, independently checks measured geometry/traversal and writes machine-readable results. No Rust route is generated. The scenario's successful process terminates through Unreal's orderly exit request. Missing fixture, invalid authority, invalid query/traversal/collision/arrival or incomplete samples cannot pass.

### G. Authored-content preservation

All **13 pre-existing canonical/negative assets** retain byte-identical original/final SHA-256 hashes. The complete two lists are `D/canonical-before.sha256` and `canonical-final.sha256` (byte-for-byte `cmp` succeeds); final per-file verification is `D/accepted-canonical-final-check.txt`. Relevant key hashes, **original = final**:

| Asset | SHA-256 (both) |
|---|---|
| `Content/Maps/RTSVerticalSlice.umap` | `2cb40d47d0974bb5d9db3688130bc0648f2f4855e0a0a180679b2798dfbd88a6` |
| `Definitions/DA_PrototypeContentCatalog.uasset` | `a434d4ab8ab3fc4339bcf95a0f580f6ac9fbf105fe0223fddffda02394492d77` |
| `Definitions/Maps/DA_RTSVerticalSlice.uasset` | `12295e9e1c5239eeb070172221a8f240266966f6a9868e104a5566c48219220c` |
| `Definitions/Units/DA_MechanizedFireteam.uasset` | `7882e27078bff2566e40e183e8f6216be2918db0fe3510eee2ff920d13cc7d16` |
| `Definitions/Units/DA_OpposingPatrol.uasset` | `ab3dc157251c8dfc1f9ce90c54d0551b256d025311c83eb296c52134515ffff5` |
| `Definitions/Units/DA_ReconVehicle.uasset` | `055b97e257a7435eb5177c709b5b3eee155ee04ee1a573fa1b581bfd93c9840c` |
| `Definitions/Weapons/W_Fireteam.uasset` | `80506a53073c4b9934f400aaedb03c75b99cd8d60ccf10b1637ced18c020d2d1` |
| `Definitions/Weapons/W_Patrol.uasset` | `4eb81c88025d0855b9f46906f9f881b1111809d1ef3953a1e79180f1b5a3e509` |
| `Definitions/Weapons/W_Recon.uasset` | `1ba7399c8dd72c21b23d3181536fb6c996a0eefefc2c0d3ee86884af09093009` |
| `TestFixtures/DA_InvalidMapSchema.uasset` | `b9ca1faaec4dae6977727ad37cc6d3753146d16270b66691600105d5dc5d558a` |
| `TestFixtures/DA_InvalidParticipantFaction.uasset` | `b07e3e43fde442a15d1737e705b0c0258081917f87dbda33691f4046467afd9a` |

Definition/TestFixtures paths above are below `Content/Politika/`. MainMenu and OfflineSandbox hashes also match in the complete lists. New saved fixture hashes: map **`86d86c2813fd9a912ae22ee1b70cbac43d635ef081103a93cc6d1f86403561e8`**, definition **`587121af4d281019efb14e3f56cf0c6aa6dc9950f4120973f3c0307b7b0b28fe`**. No pre-existing authored asset needed restoration. Packaging output stays under ignored build/Intermediate/Saved locations; only the two intended fixture assets are new source content. Initial work/evidence are retained, and task-generated partial assets were moved into D rather than deleted or restored over user work.

### H. SO-01–SO-24 acceptance

| ID | Verdict | Evidence |
|---|---|---|
| SO-01 | PASS | Dirty baseline snapshot/patch/untracked archive; existing NAV-A25 NOT_RUN reconciled. |
| SO-02 | PASS | Doctor JSON, engine version and exact source HEAD. |
| SO-03 | PASS | Final-source Navigation report discovers/executes both tests; saved-map detour succeeds. |
| SO-04 | PASS | Dedicated isolated authoring, two named assets, 13 preserved pre-existing hashes. |
| SO-05 | PASS | R direct wall trace + actual body sweep + excluded interior. |
| SO-06 | PASS | Real production Success/non-partial query, four ordered clear route points. |
| SO-07 | PASS | Cook manifests and actual cooked level/Recast/collision resolution. |
| SO-08 | PASS | Shared validators and legal lifecycle transition to Active. |
| SO-09 | PASS | Typed Move admission, current owner/participant/sequence/correlation. |
| SO-10 | PASS | 176 step samples / 157 swept subsegments on the computed route. |
| SO-11 | PASS | Authored 210 cm/s, 0.05 s, max reconstructed 10.500001 cm within 0.01 cm tolerance. |
| SO-12 | PASS | Real collision-body sweeps plus independent rounded-footprint clearance. |
| SO-13 | PASS | Resolved final position `(800,-600,42)`, zero error. |
| SO-14 | PASS | One termination/publication, no contradictory failure, retired path, 20 follow-up steps. |
| SO-15 | PASS | Existing query/admission/coordinator only; measured actual swept moves, no supplied route/teleport. |
| SO-16 | PASS | Navigation 2/2 executed/passed, warnings explicitly recorded. |
| SO-17 | PASS | RTS 9/9 and Content 7/7; no failures/skips. |
| SO-18 | PASS | Final-source accepted Linux Development UAT logs/exit 0 and artifact hashes. |
| SO-19 | PASS | Matching final-source normal manifest excludes all TestFixtures; normal startup MainMenu. |
| SO-20 | PASS | Isolated manifest contains exactly the two navigation test assets; actual runtime resolution. |
| SO-21 | PASS | Original/final canonical hash lists identical; no canonical asset changed. |
| SO-22 | PASS | Dirty baseline and archives preserved; no prohibited Git operation. |
| SO-23 | PASS | HEAD + dirty input/patch/untracked hashes, engine pin, package sidecars/hashes and R reports. |
| SO-24 | PASS | This owner and narrowly scoped roadmap/usage references updated without broad promotion. |

**NAV-A25 = PASS**, limited to this observed cooked static-obstacle scenario on Linux Development UE 5.8.3. SO-05–SO-14 are actual packaged-runtime results, not Editor substitutions.

### I. Deferred boundaries

No fresh Shipping build or Shipping artifact inspection was performed: new selector/scenario/diagnostics are guarded out in source, but **fresh Shipping isolation acceptance remains NOT_RUN**. This qualification does not promote render-delta repeatability, unreachable/partial paths, cancellation/replacement, Attack/Guard approach, death/lifecycle active-path invalidation, dynamic obstacles, crowd/formation/unit coordination, large-army performance or any other platform. The Unaligned remains a separate NPC/system authority with no expanded behavior. Stage 4A and the broader navigation milestone remain incomplete; First Playable Skirmish was not started. The next smallest engineering dependency is a bounded valid-route repeatability run using this saved fixture under multiple render-delta schedules.

## NAV-A20 authoritative navigation repeatability — 2026-10-29

### A. Baseline, scope and preservation

**NAV-A20 = PASS**, limited to the exact Linux Development runtime/content snapshot identified here. **NAV-A25 = PASS** is independently re-established in each of the ten runs; its historical evidence and measurements above are retained unchanged. Neither verdict promotes broader Stage 4A, First Playable Skirmish, other platforms, Shipping, crowd navigation or cross-platform bitwise determinism.

Evidence root **Q**: `/mnt/data/politika/build/diagnostics/navigation-repeatability-20261028-1793228/`.
Runtime root **R**: `/mnt/data/politika/Saved/Packages/NAVA20FixtureQualified20261029-1793228/Linux/navigation-repeatability-1793228699090540499/`.

Initial branch `main`, full HEAD `7d6aab9d730a707e898012bbe7acf8d4d6ea470e`, initially reported one commit behind its remote tracking branch. HEAD remained unchanged. Staged patch was empty; the checkout contained extensive authoritative, pre-existing unstaged/untracked navigation, infantry, terrain, content and documentation work. The initial binary tracked patch is `Q/initial.patch`, SHA256 `696cc4ac5d58faf89ce55217f99acae39fbf4328a083052304e787b88cc7b343`; complete status, staged patch, untracked inventory/hashes and authored-content hashes are retained alongside it. This dirty build is not identified by HEAD alone. No reset/clean/switch/rebase/merge/commit/push was performed by this task.

Engine doctor verified UE **5.8.3**, `/mnt/data/unreal/5.8.3`, revision **396c9f059903aed5fec78ecd3d437a40c6415368**. Linux Development compiler: pinned v26 Clang **20.1.8**, ISPC **1.24.0**, bundled libc++. `Q/doctor.log`, `editor-build.log`, engine version hash and package logs preserve exact tool/build evidence.

Historical NAV-A25 fixture executable still hashes to `c6af3ce93a78b48d9550284f294535b6f606abd3052598a8b43f5a95299bb107`. Its historical input-manifest/patch hashes were rechecked (`f322a916…`, `6ab71b7c…`). The accepted fixture level and definition remain byte-identical: SHA256 `86d86c2813fd9a912ae22ee1b70cbac43d635ef081103a93cc6d1f86403561e8` and `587121af4d281019efb14e3f56cf0c6aa6dc9950f4120973f3c0307b7b0b28fe`. The fixture was not regenerated or rewritten.

**Concurrent-work boundary:** other activity changed the canonical terrain map, terrain importer, music component, AGENTS and documentation while this task ran, and started competing compiler/package jobs. None of those changes was restored or overwritten. Initial canonical map SHA256 was `c48b55f041b0bcc2e3c0d62d9a56d56e9074f3d0e00db584861f43012b131e65`; the isolated snapshot captured `d68ccd4efb14bc96a6bcf024ab4f9dcd68506d761c3079951da284e653349d70`, with later current-worktree hashes separately recorded. Consequently these archives are **internal immutable regression evidence, not the latest-asset player-facing playtest or qualification of every subsequently changing worktree input**. Integrity differences and final status are recorded in Q; this task performs no authored `.uasset`/`.umap` writes.

### B. Minimal implementation and sole simulation owner

`APolitikaPrototypeMatch::Tick` remains the sole runtime update/accumulator owner. The existing policy is unchanged: float accumulator, `min(delta,0.25)` contribution, 0.05 s steps, at most five steps per update, discard residual backlog only when at least a step remains after the cap. `RunSimulationStep` still owns movement → combat → damage/death cleanup → objective evaluation → event publication. Unit movement does not run from Actor Tick, animation, Rust, timers or a second scheduler.

Only the already explicit non-Shipping static-obstacle fixture acceptance path recognizes `-PolitikaNavigationSchedule=RS-01` through `RS-05`. A running selected fixture overrides the input delta immediately before the existing production accumulator loop. Each ordinary Actor Tick invokes that loop once. The selected acceptance capture stops exactly at arrival + 20 steps even within a multi-step update. Ordinary gameplay and original `--static-obstacle` behavior remain intact. Invalid selectors/initial step state fail closed.

Initialization validates the loaded saved/cooked map, ready static Recast data, excluded wall interior, direct physical wall/body blockage, actual collision shape, authored start, ownership and Active lifecycle. The normal typed `SubmitMove` queues one intent at authoritative **step 0**. Admission at step 1 runs the production complete/non-partial Recast query; no stored route geometry is substituted. Match generation and order sequence are checked throughout. `OrderChangedEvent`/`Arrived` remains the production termination/publication contract; snapshots are not counted as new events.

Outer update records (`NAV_REPEATABILITY_UPDATE`) separately report ordinal, supplied/clamped delta, executed steps, first/last authoritative index, remainder and cap status. Fixed-step samples additionally record path-point status, waypoint cursor, current order, per-step travel, actual swept-subsegment count, owner/generation/order identity, collision validity, cumulative terminations/publications/failures. Production event records retain ordered payloads and actual step indices. Capture verifies no between-step movement; Rust rejects missing/duplicate steps, unaccounted traversal, incorrect speed/clearance, incomplete arrival/post-arrival capture, crash or timeout.

New saved-map Automation registration: **`Politika.RTS.Navigation.SavedMapDetourRenderScheduleRepeatability`**. It loads the accepted saved fixture, requires actual navigation, disables normal actor tick on its explicitly driven test coordinator, calls production `Tick` with each schedule, and captures after each fixed step through a `WITH_DEV_AUTOMATION_TESTS` observation callback. It does not invoke `RunSimulationStep` directly. Runtime selectors/instrumentation are guarded by `!UE_BUILD_SHIPPING`; fresh Shipping acceptance is **NOT_RUN**.

### C. Frozen comparison contract and schedule matrix

`Q/frozen-contract.json` was written before qualification. Position (3D), route points and per-step travel tolerate at most **0.1 cm** deviation; total route length at most **0.25 cm**. This is consistent with six-decimal centimeter telemetry and the historical 0.01 cm independent speed assertion; neither obstacle nor speed/arrival tolerances were relaxed. Discrete identity, step, order/cursor/path state, collision validity, swept counts, termination/publication counts, event payload/order/step and retirement fields compare strictly. Only run-local match generation is normalized, after independent within-run identity consistency checks. Timestamps/process/log paths are outside structured authoritative records.

Canonical reference: validated **RS-01 repetition 1**. Every second repetition compares against its own schedule's first run; every run also compares against the canonical reference, including full ordered geometry, all 176 step records, traversed subsegments, events and terminal state. `comparison.tsv` records any first divergent record/step/field and maximum deviations; it does not compare wall time or rendered-frame indices.

| Schedule | Frozen update deltas (seconds) | Repetitions | Updates per run | Executed-step histogram per run | Result |
|---|---|---:|---:|---|---|
| RS-01 | constant 1/120 | 2 | 1056 | 880×0, 176×1 | PASS / PASS |
| RS-02 | constant 1/60 | 2 | 528 | 352×0, 176×1 | PASS / PASS |
| RS-03 | constant 1/30 | 2 | 264 | 88×0, 176×1 | PASS / PASS |
| RS-04 | repeat [0.0125,0.0375] | 2 | 352 | 176×0, 176×1 | PASS / PASS |
| RS-05 | repeat [0.01,0.20,0.04] | 2 | 107 | 36×0, 36×1, 35×4 | PASS / PASS |

All deltas enter as float32, as production Tick requires. RS-05 explicitly produces four steps on its 0.20 s updates; it exercises catch-up without hitting the five-step cap or discarding backlog. The last outer update is deliberately truncated at the completed observation horizon, not used to change the production catch-up policy. No equal wall-clock progress is claimed.

Run IDs are exactly `RS-01-rep-1` through `RS-05-rep-2`, each under R, each a separate fresh process with isolated `runtime-user/`, `runtime.log`, structured key/value `trace.txt` and independent `obstacle.tsv`. Ten processes exited cleanly; all used the same executable and cooked content, without rebuild between runs.

### D. Actual movement, routes, events and comparison

Every run independently measures the production route:

```text
(-700,-600,42) → (-133,-893,42) → (133,-893,42) → (800,-600,42)
```

All ten queries succeed, non-partial, using `movement.prototype_ground`, static `RecastNavMesh-Default` (agent radius 35 cm), the same map ID `map.test.static_obstacle_detour` and saved level `/Game/Politika/TestFixtures/Navigation/W_StaticObstacleDetour`. Path length is **1632.748039 cm** versus direct **1500 cm**. Each run has **176** consecutive authoritative step samples and **157** observed swept subsegments, authored speed **210 cm/s**, step budget **10.5 cm**, independent reconstructed maximum travel **10.500001 cm**, and no obstructed traversal or body penetration. These values are actual measurements, not hard-coded pass outputs.

| Schedule (both repetitions) | Position max Δ cm | Waypoint max Δ cm | Path-length max Δ cm | Step-travel max Δ cm | Arrival step | Samples | Events / obstacle |
|---|---:|---:|---:|---:|---:|---:|---|
| RS-01 | 0 | 0 | 0 | 0 | 156 | 176 | PASS / PASS |
| RS-02 | 0 | 0 | 0 | 0 | 156 | 176 | PASS / PASS |
| RS-03 | 0 | 0 | 0 | 0 | 156 | 176 | PASS / PASS |
| RS-04 | 0 | 0 | 0 | 0 | 156 | 176 | PASS / PASS |
| RS-05 | 0 | 0 | 0 | 0 | 156 | 176 | PASS / PASS |

There is no divergent step or state field. Every strict field agrees; all waypoint cursors, current orders, path statuses and swept counts match at each authoritative index. The common event sequence is: step 1 accepted `CommandOutcome` (sequence/correlation/entity 1), step 1 `OrderChanged` Idle→Move/None, step **156** `OrderChanged` Move→Idle/**Arrived**. Exactly one production arrival termination and one corresponding event occur per run. Final destination is `(800,-600,42)`, error **0 cm**. Steps 157–176 retain Idle, empty path/cursor zero, no movement and no duplicate arrival. `Q/runtime-measurements.json` preserves per-run route, events, step counts and outer-update histograms; R/comparison.tsv and verdict.tsv are the decisive machine-readable comparison.

### E. Build, Automation and quality

The pinned Editor wrapper succeeds (`Q/editor-build.log`). `quality -- core`, `quality -- audit`, full `cargo test --locked --manifest-path Tools/Shell/Cargo.toml` and full `cargo fmt --manifest-path Tools/Shell/Cargo.toml -- --check` pass. Rust executes **five substantive tests**, including first-field/step/geometry/event/missing-sample comparator rejection, missing-boundary/update failure, and the retained independent collision verifier tests. Other zero-test binaries are not counted as behavioral tests. Initial Rust inference and baseline formatting failures were corrected, not concealed; before-format copies/logs remain. Formatting-only changes to two initially clean harness files do not alter gameplay.

Each actual filter was run against both the authoritative source build and the isolated packaging snapshot; exported reports were inspected. Commands use pinned `UnrealEditor-Cmd PROJECT -unattended -nop4 -nullrhi -nosplash '-ExecCmds=Automation RunTests FILTER;Quit' -ReportExportPath=REPORT -ABSLOG=REPORT/run.log`.

| Filter | Discovered | Executed | Passed | Clean | Warning-bearing success | Failed | Skipped/not run |
|---|---:|---:|---:|---:|---:|---:|---:|
| Politika.RTS.Navigation | 3 | 3 | 3 | 0 | 3 | 0 | 0 |
| Politika.RTS | 10 | 10 | 10 | 3 | 7 | 0 | 0 |
| Politika.Content | 7 | 7 | 7 | 7 | 0 | 0 | 0 |

Reports: `Q/Politika.RTS.Navigation/`, `Q/Politika.RTS/`, `Q/Politika.Content/` and their `snapshot-` counterparts; counts also in JSON. Navigation warnings: expected missing-navigation negative and pre-existing map-load crowd-manager missing-Recast warning. The new repeatability test emits six instances of that existing map-load category (five fixture loads plus menu return); no new warning category or skipped spatial assertion is hidden. The command/lifecycle negative warning-bearing tests remain explicit. An initial JSON-summary helper rejected UE's UTF-8 BOM; corrected parsing inspected the original complete passing report, without calling the helper failure a gameplay failure.

### F. Packages, identities and reproducible execution

Both qualified archives are under `/mnt/data/politika/Saved/Packages/`:

- Normal: **`NAVA20NormalQualified20261029-1793228/Linux`**.
- Fixture: **`NAVA20FixtureQualified20261029-1793228/Linux`**.
- Both native executable SHA256: **`0c359d18de4acd780ca03b2efc77affe22b8b39deb3610aeca23aec903717cc6`**.
- Exact snapshot source/tool/content manifest: `Q/package-source-inputs.sha256`, SHA256 **`e4512316daecb47203d74dd59de6c167c2b787f02876ef9bc43f04b70d379884`**; identical `QualificationSource.sha256` sidecars, `BuildIdentity.json` and engine pin in both archives. Build identity is `7d6aab9d730a-dirty`, not clean HEAD.
- Executable, Pak, global/game IoStore hashes are complete in `Q/Normal-artifact-sha256.txt` and `Q/Fixture-artifact-sha256.txt`, and pass post-run verification. Real UFS/NonUFS manifests remain at each Linux root. Normal UFS contains no `TestFixtures/`; fixture adds exactly the two navigation assets, no negative fixture.

The dirty authoritative startup had already moved to RTSVerticalSlice for infantry/terrain playtesting. UAT's initial command-line ini override did not persist into staged runtime configuration; its otherwise successfully built archives are retained but **not** final startup evidence. Instead the preserved source/content snapshot at `/mnt/data/politika-qualification/NAVA20-20261028-1793228` has only its **copied** `GameDefaultMap` changed to MainMenu. Canonical config is untouched. R/normal-startup.log and fixture-ordinary-startup.log prove actual no-map/no-selector MainMenu boot and no navigation acceptance activity in both qualified archives.

Original same-runtime-source compile/cook commands and logs are `Q/package-final.sh`, `package-Normal.log`, `package-Fixture.log` (both `-build`, terminal success). Relocation initially triggered lock conflicts and a 604-action engine cache rebuild; only this task's relocation build was stopped, its partial log retained, never counted as successful. `Q/compiled-source-to-snapshot.json` proves runtime source matches the successful freshly compiled executable; the only Source difference is the concurrent unused terrain-authoring Editor commandlet. Final recooking/packages reuse that identical verified executable, not historical NAV-A25 binaries. Exact serialized final UAT script is **`Q/package-snapshot.sh`**:

```sh
RunUAT.sh BuildCookRun -project=SNAPSHOT/Politika.uproject -noP4 \
  -platform=Linux -clientconfig=Development -skipbuild -nocompileeditor \
  -cook -stage -pak -iostore -archive -unattended -utf8output \
  -WaitForUATMutex '-UbtArgs=-WaitMutex -MaxParallelActions=6' \
  -archivedirectory=QUALIFIED_ARCHIVE
# Fixture only: -CookDir=SNAPSHOT/Content/Politika/TestFixtures/Navigation
```

Both final UAT operations terminate `BUILD SUCCESSFUL`/exit 0 (`Q/package-Normal-final.log`, `package-Fixture-final.log`). Cooked content/NavMesh is actually exercised below. No compiler actions from this task run concurrently; mutex waiting handles externally started jobs. `Q/command-outcomes.tsv` records successful, failed, interrupted and diagnostic command outcomes, rather than promoting zero exits without tests.

Exact accepted matrix command (exit 0):

```sh
cargo run --locked --quiet --manifest-path Tools/Shell/Cargo.toml --bin rts_interaction -- \
 /mnt/data/politika/Saved/Packages/NAVA20FixtureQualified20261029-1793228/Linux/Politika/Binaries/Linux/Politika \
 --navigation-repeatability \
 /mnt/data/politika/Saved/Packages/NAVA20NormalQualified20261029-1793228/Linux/Politika/Binaries/Linux/Politika
```

The original `--static-obstacle` entry is preserved. Rust only launches, parses, verifies and compares Unreal measurements. Each child has a 90-second deadline; incomplete traces/crashes/timeouts fail closed. All ten are headless runtime qualification, not rendered/physical-device qualification.

### G. NR-01–NR-27 and next dependencies

| Gate | Verdict | Evidence |
|---|---|---|
| NR-01 | PASS | Initial dirty source/patch/untracked inventory, engine pin and snapshot identity. |
| NR-02 | PASS | Historical accepted manifest/patch/executable/fixture reconciled, retained. |
| NR-03 | PASS | Unchanged Tick/clamp/catch-up/step/event/lifecycle ownership; Q/architecture-audit.md. |
| NR-04 | PASS | Frozen contract before runs, five exact schedules, unchanged thresholds. |
| NR-05 | PASS | R contains ten fresh independent process logs/user dirs. |
| NR-06 | PASS | Identical immutable executable and cooked artifacts, source sidecars. |
| NR-07 | PASS | Actual production Tick accumulator, separate outer and fixed-step records. |
| NR-08 | PASS | One runtime Tick path; explicit Editor harness disables normal ticking. |
| NR-09 | PASS | Same authored start/goal/unit/owner/faction, typed Move queued at step 0. |
| NR-10 | PASS | Ten ready static Recast Success/non-partial queries. |
| NR-11 | PASS | Ordered route-point max deviation 0 cm. |
| NR-12 | PASS | 176 records/run, 1760 complete matching authoritative samples. |
| NR-13 | PASS | Strict per-step cursor/order/path-status equality. |
| NR-14 | PASS | Independent per-run sweeps/speed plus strict subsegment/clearance comparison. |
| NR-15 | PASS | Every arrival step 156, destination error 0 cm. |
| NR-16 | PASS | Full ordered event payload/step equality. |
| NR-17 | PASS | One termination/publication per process, admission once. |
| NR-18 | PASS | Twenty subsequent steps, retired path, no movement/duplicate arrival. |
| NR-19 | PASS | Each independent obstacle.tsv is NAV-A25 PASS. |
| NR-20 | PASS | No missing/duplicate/unaccounted samples, crash or timeout. |
| NR-21 | PASS | Focused registered test discovered/passes within Navigation 3/3. |
| NR-22 | PASS | RTS 10/10, Content 7/7, source and snapshot reports. |
| NR-23 | PASS | Core/audit, five Rust tests, final full Cargo fmt, diff check. |
| NR-24 | PASS | Same-source compile, corrected snapshot packages, manifests/hashes/startup checks. |
| NR-25 | PASS (task preservation; concurrent edits disclosed) | No task-authored content writes/restores; exact fixture hashes unchanged; initial/snapshot/final differences separately recorded. Not a claim that external edits did not occur. |
| NR-26 | PASS | R/comparison.tsv, per-run full traces/obstacle reports, Q/runtime-measurements.json. |
| NR-27 | PASS | Current NAV-A20 verdict updated; historical records preserved; broader gaps retained. |

Smallest next navigation dependency: separately qualify valid-route cancellation/replacement or no-complete-route failure; then bound target approach/lifecycle invalidation as independent gates. Dynamic navigation, formations, many-unit performance and broader NAV-A10–A19/A27 remain unqualified. Infantry discovery/readiness was assessed separately outside this navigation publication scope, with no effect on these runtime results.

### H. Final integrity and task-owned changes

`Q/asset-integrity-comparison.json` compares all authored assets, not merely selected examples: **92 initial**, **96 final**; **91 initial files unchanged**, one externally modified canonical terrain map and four externally added MarburgumGrass material/textures. The current canonical map is `8e8b8bba057f076cdeb719488e2e846d70626a55fdb60dfb1c5ecc1d41ba8afc`. Every canonical definition/catalog/negative fixture and the accepted navigation fixture retains its initial bytes. The complete before/final hashes and each added asset's hash are in Q. These terrain changes were observed during concurrent import/build activity and are not attributed to this task; no blind restoration occurred.

Task-owned files: `Source/Politika/Private/PolitikaPrototypeGameplay.cpp`, `Public/PolitikaPrototypeGameplay.h`, `Private/PolitikaNavigationAcceptance.cpp`, `Public/PolitikaNavigationObstacleFixture.h`; new `Source/PolitikaEditor/Private/Tests/PolitikaNavigationScheduleTests.cpp`; `Tools/Shell/src/navigation_obstacle.rs`, `src/bin/rts_interaction.rs`, plus rustfmt-only `src/bin/content_acceptance.rs` and `shell_recovery.rs`; this document, `Docs/ENGINE_OUTLINE.md`, `Docs/DOCUMENTATION_INDEX.md`, new `Docs/INFANTRY_ASSET_READINESS.md`. No content asset or canonical config is task-owned. All evidence/snapshot outputs use separate diagnostic/archive paths.

Final HEAD is still `7d6aab9d730a707e898012bbe7acf8d4d6ea470e`, dirty `main`; remote tracking was observed two commits ahead later in the task, without a task-initiated fetch or merge. `Q/final-status.txt`, `final.patch`, `final-staged.patch`, `final-untracked.sha256`, `task-files.sha256` and `final-identities.txt` identify the final evolving checkout and the exact task outputs separately from the tested immutable snapshot. Initial/final tracked patches include others' work and are not standalone build identities. Final fixture/normal artifact hash verification, snapshot-input verification, full Cargo formatting, documentation audit (54 index links) and `git diff --check` pass. The infantry report independently records its actual binary identity and missing readiness prerequisites; it does not alter NAV-A20.

## Authoritative navigation order interruption qualification — 2026-10-29

**PASS for this bounded Linux Development snapshot.** Valid-route Stop, duplicate Stop, valid Move replacement and rejected out-of-bounds Move replacement are qualified. This does not close broader navigation or Stage 4A.

### Identity, baseline and preserved historical evidence

Evidence root **INT-D**: `build/diagnostics/navigation-interruption-20261029-1793229768/`. Full A–L engineering report and INT-01–INT-34 verdicts: [archived engineering report](Reports/NAVIGATION_INTERRUPTION_20261029.md). Initial/final branch `main`, HEAD `7d6aab9d730a707e898012bbe7acf8d4d6ea470e`, tracking `0 3` behind origin/main, no staged changes. Substantial existing dirty source/content/config/untracked work preserved. Initial/final binary patches and scoped/full input inventories identify the dirty source, not HEAD alone. UE 5.8.3 pin `396c9f059903aed5fec78ecd3d437a40c6415368`; Linux Editor/Game Development builds passed. Historical NAV-A20/NAV-A25 reports and executable `0c359d18…` remain unchanged and separately identified above.

New immutable INTERNAL qualification archives:

- Normal: `Saved/Packages/INTNormal20261029-1793229768/Linux/`.
- Fixture: `Saved/Packages/INTFixture20261029-1793229768/Linux/`.
- Both executable SHA256: `b16dc3c1ac6e8a9c0f47091bda32944ef3312835eda685ccfb701bca37eac6a8`.
- Runner's scoped source manifest SHA256: `8ee813c24d407ce43b480e7f3b20185d90cb067f897a4ee151e664c71dcab680`.
- Supplementary COMPLETE source/content/config/tool/build-input manifest SHA256: `b7f856209490eea1f3485774cba9d7800eea9e5c39329806e66fb9ba61e98cad`; includes ignored local generated media omitted by default ripgrep inventory. Both archives contain `CompleteQualificationSource.sha256`. All manifest entries verified in the preserved copied qualification project.
- Full executable/Pak/IoStore identities: `INT-D/Normal-artifacts.sha256`, `Fixture-artifacts.sha256`; UFS/NonUFS manifests in each archive; UAT logs and exact serial invocation in `INT-D/package.sh`.

The copied qualification project `/mnt/data/politika-qualification/INT-20261029-1793229768` uses current runtime source, content and freshly compiled binaries/receipts. Only its copied GameDefaultMap is overlaid to MainMenu, verified at runtime for both archives. Canonical startup remains RTSVerticalSlice, unchanged. Normal cook excludes all TestFixtures; fixture adds only the accepted Navigation CookDir. These internal archives are not promoted to the ordinary player-facing launcher. No Shipping verification was run.

Accepted fixture reused unchanged: `/Game/Politika/TestFixtures/Navigation/W_StaticObstacleDetour`, definition ID `map.test.static_obstacle_detour`, profile `movement.prototype_ground`, authored `unit.mechanized_fireteam`, stable runtime unit/owner `1/1`, 210 cm/s and unchanged scaled spherical collision body. Original start `(-700,-600,42)`, goal `(800,-600,42)`. Actual complete four-point original Recast route length `1632.748039 cm`. Frozen step-40 position `(-326.874538,-792.814392,42)`, cursor 1, four waypoints, order Move, generation/sequence `1/1`, no arrival/failure.

### Authoritative boundary and unchanged semantics

`SubmitMove`/`SubmitStop` are existing trusted coordinator adapters to typed `SubmitCommand`; world positions become map-space offsets relative to playable minimum. Participant/unit authorization and portable `ValidateOrder`/`AdmitCommand` precede route query/mutation in `AdmitPendingOrders`. The sole coordinator Tick uses 0.05 s steps, 0.25 s contribution clamp and five-step catch-up cap. No direct simulation-step driving, secondary scheduler, AI MoveTo, fabricated routes or event bus was introduced.

Development acceptance enqueues only, after movement/combat/objective processing AND event publication of step 40, before the next loop iteration admits at step 41. Duplicate Stop is queued after 60, admitted at 61. This works inside one catch-up Tick, not just between rendered frames. Pre-movement diagnostic snapshots labelled `NAV_INTERRUPTION_ADMITTED` at 41 and 61 are state snapshots; actual command admissions/publications are exclusively the typed CommandOutcome timeline (there is no new command at 61 in OI-02/OI-03).

Successful Stop preserves CurrentOrder=Stop, empties route, resets cursor, clears executable generation/sequence. Duplicate Stop has its normal accepted outcome without another OrderChanged. Replacement queries from the authoritative current position, installs a new generation/sequence-owned route and publishes Replaced. Rejection occurs before query/mutation. Existing arrival clears path/cursor and becomes Idle, retaining non-executable diagnostic identity. No production defect was reproduced and no production movement/admission semantics were changed. OrderChanged gained no command-sequence field; outcomes/entity/step/ordered history correlate transitions.

### Frozen schedules, tolerances and results

Interruption schedules: **RS-02 constant 1/60 s**, **RS-05 repeating [0.01,0.20,0.04] s**. Each scenario has two fresh processes per schedule. Position/waypoint/travel comparison tolerances remain **0.1 cm**, route length **0.25 cm**; discrete states, enums, identity consistency, cursors, admissions/arrivals, transitions and event counts/steps use strict equality. Only run-local generation may be normalized after within-run identity verification.

| Scenario | Runs | Verdict | Authoritative outcome |
|---|---:|---|---|
| OI-01 Stop + duplicate | 4 | PASS | Initial sequence 1, accepted Stop sequence 2 at 41, one Stopped transition. Route/cursor/identity retired before movement. Position unchanged through step 81. Duplicate sequence 3 accepted at 61; no extra termination/transition, arrival or failure. |
| OI-02 valid Move replacement | 4 | PASS | Sequence 2 admitted at 41; one original Replaced. Fresh complete two-point Recast route, measured length 420 cm, returns to original start. Only sequence 2 executes after replacement; arrival at 80 once, route retired, inert through 100. No original-destination arrival. |
| OI-03 invalid Move replacement | 4 | PASS | Destination world (3000,3000,42), outside [-2000,2000] XY, typed map offset (5000,5000). Sequence 2 rejected at 41 with InvalidTarget (enum 5). One query/install total, original route/geometry/cursor/progress/identity unchanged; original sequence 1 arrives at 156 once, inert through 176. |

Every moving step has complete indexed traversal telemetry, actual body-sweep clearance and independent Rust swept-sphere/route/speed checks. Maximum travel `10.500001 cm` (6-decimal reconstruction), budget `10.5 cm`; frozen existing numerical tolerance retained. Trace coverage has no missing/duplicate steps, unobserved movement, stale route execution or duplicate publications.

Interruption runtime root **INT-R**: `Saved/Packages/INTFixture20261029-1793229768/Linux/navigation-interruption-1793230144532581186/`. `matrix.tsv` lists all 12 process/schedule/repetition identities. Each run has isolated UserDir, bounded 90-second timeout/clean teardown, `runtime.log`, `result.tsv`, `comparison.tsv`, `trace.txt` and command/route/event timeline. Six scenario/schedule comparison reports and three cross-schedule reports sit at INT-R. Maximum position/waypoint/length/travel deviation across repetitions and schedules: **0/0/0/0 cm**, no first divergent step/field.

### Final-source regression and rejected-command reference

After those 12 processes, the same runner launched **ten fresh NAV-A20 runs** (RS-01–RS-05 twice): **10/10 PASS**, each independently **NAV-A25 PASS**, each 176 steps, arrival 156 plus 20 inert steps, identical original path/events and zero numerical deviations. Regression root **INT-N**: `Saved/Packages/INTFixture20261029-1793229768/Linux/navigation-repeatability-1793230193769197417/`; inspect `comparison.tsv`, `verdict.tsv` and each `obstacle.tsv`/trace. Startup probes are separate and not counted toward 22 scenario runs.

The four OI-03 runs then compare against same-schedule final-source INT-N repetition-1 traces. Every movement/order/collision/traversal field and original event sequence matches; exactly one additional rejected CommandOutcome is permitted and independently asserted. All four `continuity.tsv` reports PASS with zero position/waypoint/length/travel deviation and identical arrival step. No unnecessary duplicate reference process was launched.

Reproduce the entire 12+10 qualification (both older CLI modes remain supported):

```bash
cargo run --locked --manifest-path Tools/Shell/Cargo.toml --bin rts_interaction -- \
  /mnt/data/politika/Saved/Packages/INTFixture20261029-1793229768/Linux/Politika/Binaries/Linux/Politika \
  --navigation-interruption \
  /mnt/data/politika/Saved/Packages/INTNormal20261029-1793229768/Linux/Politika/Binaries/Linux/Politika
```

### Automation, preservation and remaining gates

Exported actual reports: Navigation **4 discovered/executed/passed**, RTS **11**, Content **7**, zero failed/unrun/in-process. Warning-bearing successes: 4/8/0 respectively. New `Politika.RTS.Navigation.SavedMapDetourOrderInterruption` is discovered/executed. It loads the accepted saved map and real NavMesh, disables actor ticking before manually supplying production Tick deltas, and covers all three scenarios plus uninterrupted reference. Known missing-nav negatives, crowd-manager map-load warnings and transient-world DestroyActor warnings remain disclosed in `INT-D/automation-summary.json`; no new teardown-warning category. Editor and game build, doctor, core/audit, six Rust tests, full formatting and diff checks PASS.

All **62 initially captured nonignored authored assets** retain exact SHA256. The inventory review additionally checks **34 ignored generated local media assets** against their unchanged identified historical hashes; all **96** current assets match their applicable baseline. The default ripgrep omission and supplementary full inventory are explicit in `INT-D/asset-integrity.json`; no asset was generated, regenerated, imported or saved by this task. Accepted fixture binaries are unchanged. Source and package rechecks found no concurrent relevant mutation, and all pre-existing staged/unstaged/untracked work remains preserved; no reset/clean/branch switch/merge/rebase/commit/push.

NAV-A12/A13 are now valid-route qualified for this static fixture/Linux Development snapshot. NAV-A20/A25 independently qualify on this final source. **Not general unreachable/partial-path acceptance**: target approach, death/active-route lifecycle termination, dynamic obstacles, multi-unit/formation/crowd behavior, scale, other platforms and Shipping remain separate. Economy, autonomous AI, infantry repair/import and First Playable Skirmish remain outside this increment. The smallest justified next qualification is an isolated unreachable/partial-route failure gate through the same production admission/query/Tick machinery.

## Latest-main publication qualification — 2026-10-29

**PASS.** The authorized merge isolates navigation and its required foundation onto upstream `56e9a12388addd710ef4fd16988875846aa71243`, rather than publishing the combined dirty checkout. Upstream grass/camera controls, optional authored mesh-material support, Rust authoring and archive updates are retained. Unmerged infantry presentation/abilities/HUD, music, default-content rewrites, Python scripts and unrelated reference-document edits are excluded. Two older Rust harness files receive formatting-only normalization for the full formatting gate. The archived original report above remains historical; its package identity is not substituted for this candidate.

Fresh candidate evidence root: `build/diagnostics/navigation-merge-20261029/` in the shared repository. Pinned UE 5.8.3 Editor and Game Development builds PASS; the Game build completed all 392 worktree-local compiler/link actions. Exported Navigation 4/4, RTS 11/11 and Content 7/7 reports PASS, zero failed/unrun, warning-bearing successes 4/8/0 with the existing categories. Portable core/audit, six Rust tests, full formatting and diff checks PASS. Source inputs remain frozen and hashed; only documentation changes follow qualification.

Fresh INTERNAL archives, each actually boots MainMenu through the documented copied-config overlay:

- `Saved/Packages/INTMergeNormal20261029/Linux/` — no TestFixtures in the UFS manifest.
- `Saved/Packages/INTMergeFixture20261029/Linux/` — accepted fixture and map definition included.
- Both executable SHA256: `6d20fdc03f7734581f58822b7539d38a984873d27a4cf522fcf48335604e328a`.
- Complete package source/content/config/tool manifest SHA256: `267cf950b851f14d3aa08284439f1c53b2856baffd8812f1e82ee04b262df0d1` (`package-inputs.sha256`, checked in the copied qualification project).
- Compiler/runtime build identity is `56e9a12388ad-dirty` plus the manifest above; the later publication commit does not relabel these binaries. UAT logs, exact commands and container hashes are retained under the fresh evidence root.

A second complete **22/22 fresh-process qualification PASS** belongs to this isolated source: twelve OI-01/02/03 runs (RS-02 and RS-05 twice), then ten existing five-schedule NAV-A20 runs, each independently NAV-A25 PASS. Interruption report: `Saved/Packages/INTMergeFixture20261029/Linux/navigation-interruption-1793231431926579706/`; regression: `navigation-repeatability-1793231481175564746/` beside it. All four OI-03 same-schedule uninterrupted-reference reports PASS. Maximum position/waypoint/length/travel deviation is 0/0/0/0 cm. Stop holds through81 including duplicate61; replacement arrives80/holds100; rejection preserves original arrival156/hold176. Existing frozen tolerances, collision/speed legality, event counts and command boundaries are unchanged.

Authored fixture bytes remain exact: map SHA256 `86d86c2813fd9a912ae22ee1b70cbac43d635ef081103a93cc6d1f86403561e8`, definition `587121af4d281019efb14e3f56cf0c6aa6dc9950f4120973f3c0307b7b0b28fe`. No canonical content was modified by the merge. Historical reports/packages and the ordinary player-facing launcher remain preserved. This is still bounded Linux Development navigation acceptance, not Shipping, cross-platform, broad Stage 4A or First Playable qualification.

## Unreachable and partial-path failure qualification — 2026-10-29

This follow-on closes **NAV-A14** for the isolated Linux UE 5.8.3 Development scenario and independently exercises a genuine partial result. It does not promote every NAV-A15 failure subcase or Stage 4A. Prior NAV-A12/A13/A20/A25 records above retain their historical source and package identities.

### Source, fixture and query evidence

- Source branch: `codex/nav-unreachable-partial-20261029`; verified start `a4dcd64091ef5ce9fc9adcceb742c87362bd0b59` (reconciliation merge over `origin/main` `ed596024c4ed98581a884f41492b1c58ac278edd` and local `main` `5c9db959360f4d41ba693268126f9aac79f66164`). The shared primary checkout stayed on `main` and its staged/unstaged/untracked work was not altered. UE version/pin remains 5.8.3 / `396c9f059903aed5fec78ecd3d437a40c6415368`.
- Isolated fixture assets: `/Game/Politika/TestFixtures/Navigation/W_DisconnectedNavigation`, `/Game/Politika/TestFixtures/Navigation/DA_DisconnectedNavigation`, stable map ID `map.test.disconnected_navigation`. They are not registered in the production catalog or permanent always-cook configuration. The optional target is distinct from required starts/objective, which remain reachable on the origin side.
- The saved cooked Recast data is `PersistentLevel.RecastNavMesh-Default`, static (`runtime_generation=0`). The valid origin `(-700,-600,42)` and requested goal `(800,-600,42)` are within `[-2000,2000]` XY. Both project to `(-700,-600,-10)` and `(800,-600,-10)` with 52 cm offsets (within the 80 cm runtime tolerance). The static collision/navigation barrier bounds X `[-100,100]`, Y `[-5000,5000]`; it fully divides the playable region without a gap or route around its ends. Both endpoints were independently observed projectable in both packaged runs; the package passed shared content/spatial validation and reached Active.
- Production remains complete-route-only: `allow_partial=0`, `require_navigable_end=1`. Actual `FindPathSync` returned `ENavigationQueryResult=2` (`Fail`), `partial=1`, valid diagnostic path with 2 points ending at `(-152,-988,-10)`. This is a failed/no-complete-path production result—not a successful complete route and not evidence that the diagnostic partial was installed. Failure classification was `NoCompletePath`; route installation was rejected and route state retired.
- Separate NQ-02 query used the same real cooked nav data and `FindPathSync` with `allow_partial=1`. It returned `result=3` (`Success`), `partial=1`, valid 2-point partial path ending 1028.031128 cm from the requested projected goal. The same production `IsCompleteNavigationResult` gate rejected it as `PartialPath`; trace confirms `installed=0`. No partial flag or query result was fabricated.

### Packaged results and reproducibility

Final package roots are `Saved/Packages/NavigationUnreachableNormalMainMenu20261029/Linux/` and `Saved/Packages/NavigationUnreachableFixtureMainMenu20261029/Linux/`. The normal UFS manifest has no `TestFixtures/`; the isolated fixture manifest contains the disconnected map/definition and its generated `.ubulk`, alongside the existing navigation obstacle fixture. The two executable hashes are identical: `60dc37505953a04ec3a47d174171764df104cee05fb171d3d954ba6c0c0de94e`. `Politika-Linux.utoc` SHA-256 is `227b31cda6bd4c5097c37792e28a08250df1ddc43fa8149934a4c27120a25a88` (normal) and `8f1eaf4bc99423342fc9d0fb898a9017d33d5af73a30611c77494d39258744f0` (fixture). Package sidecars retain `BuildIdentity.json`, `QualificationSource.sha256` and `unreal-version.env`.

The acceptance package used the established copied-config MainMenu overlay because the tracked development project default is `RTSVerticalSlice`; the isolated `Config/DefaultEngine.ini` was restored byte-for-byte after cooking. No canonical/default-map change is part of this slice. Both packages passed no-selector MainMenu startup; PC-01 also ran the existing complete static-obstacle route control and passed.

Exact negative command:

```sh
cargo run --locked --manifest-path Tools/Shell/Cargo.toml --bin rts_interaction -- \
  Saved/Packages/NavigationUnreachableFixtureMainMenu20261029/Linux/Politika/Binaries/Linux/Politika \
  --navigation-unreachable \
  Saved/Packages/NavigationUnreachableNormalMainMenu20261029/Linux/Politika/Binaries/Linux/Politika
```

The final run's evidence directory is `Saved/Packages/NavigationUnreachableFixtureMainMenu20261029/Linux/navigation-unreachable-1793233889117661520/`. It contains two fresh process logs and copied runtime logs, unique user directories, `artifact-identity.txt` and `acceptance.tsv`. **NQ-01-A/B PASS**: both show one real production query (`result=2`, `partial=1`), one authorized typed Move outcome, exactly one `NavigationFailed` `OrderChangedEvent`, zero arrival, retired executable route/cursor/identities, and 21 fixed-step samples (failure step plus 20 further steps) at the unchanged start position. Match remains Active; unit remains alive and owned. **NQ-02-A/B PASS**: each fresh process's genuine partial diagnostic query was rejected by the production validator and never installed. The failed first packaging attempt (fixture definition was not resolved by the Development loader) is retained under the earlier `NavigationUnreachableFixture20261029` archive and is not counted as evidence.

### Automation, quality and bounded verdicts

- Focused `Politika.RTS.Navigation.SavedMapDisconnectedDestinationFailsClosed`: 1 discovered, 1 succeeded-with-warning, 0 failed/skipped. Full `Politika.RTS`: 12 discovered, 3 succeeded, 9 succeeded-with-warnings, 0 failed/skipped; includes the new test, command contract, participant authority, lifecycle and navigation regressions. `Politika.Content`: 7 discovered/passed, 0 failed/skipped. Warnings are the existing expected missing-navigation/rejection and transient Editor-world Recast cleanup diagnostics; exported JSON reports are under `build/diagnostics/navigation-failure/automation*`.
- Pinned `tools/unreal.sh doctor`, Editor build, both final Linux Development UAT packages, portable C++ `quality -- core`, repository `quality -- audit`, Rust `cargo test --locked --manifest-path Tools/Shell/Cargo.toml` (6 passed) and `cargo fmt --manifest-path Tools/Shell/Cargo.toml -- --check` pass. `git diff --check` passes. Pre/post SHA-256 checks match for all inventoried canonical maps, definitions, catalog and existing fixture assets; only the two scoped disconnected fixture assets are new.
- `NAV-A14 = PASS` for valid, in-bounds, individually projectable endpoints disconnected under the supported ground profile, production query failure, fail-closed execution and authoritative failure publication on the final cooked Development source.
- NAV-A15 subcases: missing navigation = previously qualified PASS; invalid/unprojectable goal = not newly qualified; disconnected no-complete-path = PASS; genuine partial-result rejection = PASS twice; empty/invalid-path-specific query result = not separately constructed; no silent fallback = PASS for the missing-data, disconnected and real-partial exercised paths. Overall `NAV-A15 = INCONCLUSIVE` because invalid/unprojectable and empty/invalid-path cases remain outside this evidence.
- The implementation changes no production movement algorithm or command/event contract; it adds shared result validation at the existing route boundary, Development-only fixture/probe instrumentation and the test observer. No Shipping, target-approach, death/lifecycle-route termination, dynamic/multi-unit, scale, Windows, SteamOS or Android claims are made.

The scoped implementation is committed on the isolated task branch. Publication was intentionally not attempted: after qualification, `origin/main` advanced and no longer has a fast-forward relationship with the task branch, and advancing the shared dirty `main` would collide with pre-existing user changes in `README.md`, `Docs/ENGINE_OUTLINE.md`, navigation gameplay/content sources, the Editor commandlet and `Tools/Shell/src/bin/rts_interaction.rs`. The task branch and its accepted evidence are preserved for a later safe reconciliation; no force, reset, stash or shared-checkout mutation was used.

## NAV-A15 DP-02 excessive projection displacement qualification — 2026-10-29

This isolated follow-on qualifies only the successful-but-excessive destination projection boundary. It does not close empty/invalid path-result (ER-01/ER-02) cases or overall NAV-A15.

### Baseline, source and feasibility

- Canonical repository: `https://github.com/simon-gros/politika`. A read-only live `git ls-remote origin refs/heads/main` and fetched reference agreed at `3994baa5e2646adaefba9dda27645b4e6a1fa7d2`. The isolated branch `codex/nav-a15-projection-displacement-20261029` and worktree `/mnt/data/politika-nav-a15-projection-displacement-20261029` start exactly there. The shared `/mnt/data/politika` checkout remains at `5c9db959360f4d41ba693268126f9aac79f66164`, one ahead / 35 behind its fetched origin, with unrelated staged/unstaged/untracked user work; it was not edited. The protected DP-01 and path-results worktrees were inspected read-only and left unchanged.
- Pinned UE is 5.8.3, source revision `396c9f059903aed5fec78ecd3d437a40c6415368`. Doctor passed. The engine installation was already dirty at preflight; none of its pre-existing changes was modified.
- The production typed Move transmits map-relative XY. `SubmitCommand` reconstructs world XY by adding playable-min XY and uses the unchanged unit-center Z `42.0`. The saved test submits raw target `(1300,1300)` (inside `[-2000,2000]²`), map-relative XY `(3300,3300)`, and the actual production request `(1300,1300,42)`. It applies no height adjustment.
- The separate collision trace hits authored raised ground at `(1300,1300,200)`, so the production-request-to-ground Z difference is `-158 cm` (requested center is below the collision top); this is not substituted into the navigation request. The connected, collision-bearing elevated surface resolves at projected NavMesh point `(1300,1300,210)`.
- UE 5.8.3 source inspection followed `UNavigationSystemV1::ProjectPointToNavigation` into the pinned Recast `ProjectPoint` / `findNearestPoly2D` implementation, including vertical query bounds and returned nearest-poly location. The saved fixture's real Recast probe produced a complete path from the origin to the elevated surface; it is not a disconnected-path substitute or unsupported floating polygon.
- Production remains unchanged at query extent `(80,80,180)` and strict guard `FVector::Dist(ProjectedGoal.Location, Destination) > 150.0f`. The measured horizontal delta is `0 cm`, vertical delta `168 cm`, and actual full 3D distance `168.000000000 cm`; `168 > 150` strictly. Both points are within the declared query extent. The correct `DestinationNotNavigable` branch returns before production `FindPathSync` construction/execution.

### Saved fixture and packaged runtime

- Only two new saved assets were authored: `/Game/Politika/TestFixtures/Navigation/W_ProjectionDisplacement` and `DA_ProjectionDisplacement`, selected by map ID `map.test.projection_displacement` and opt-in Development selector `-PolitikaTestFixture=ProjectionDisplacement`. The optional runtime destination is not a required start/objective. The fixture validates with zero map-check errors and reaches Active before command admission. A first ramp-orientation candidate failed the connected-path probe and was rejected; the corrected saved geometry was then reloaded and re-probed. Both candidate records remain under `Saved/Diagnostics/dp02-candidate-v1/` and the authoring report under `build/diagnostics/projection-displacement-authoring-v5.log`.
- Two fresh independent cooked Linux Development fixture-package processes, each with its own `-UserDir` and runtime log, passed. Full logs and reports: `Saved/Packages/DP02Fixture/Linux/navigation-projection-displacement-1793241113774646052/acceptance.tsv` and `DP-02-A.log` / `DP-02-B.log` in that directory. Both measured request `(1300,1300,42)`, projected point `(1300,1300,210)`, ground hit `(1300,1300,200)`, and `168.000000000 cm`.
- Both commands were admitted as authorized typed Moves against a ready static `RecastNavMesh-Default` and the supported `movement.prototype_ground` profile. Actual projection diagnostics record origin success, goal success, unchanged extent, requested/projected vectors, displacement and `query_executed=0`. The terminal state reports `DestinationNotNavigable`, query/path result fields `NA` (no path query occurred), no executable route, retired route identity/cursor, one accepted command, exactly one failure publication, zero arrival publications, and a living unit with unchanged ownership. Failure-step plus 20 subsequent authoritative fixed steps (21 recorded positions total) are stationary; no duplicate failure appears.
- The focused saved-map Automation `Politika.RTS.Navigation.SavedMapExcessiveProjectionDisplacementFailsClosed` discovered and executed 1/1, passed with an expected Editor/Recast cleanup warning, 0 failed/skipped. Its actual measurements and report are in `build/diagnostics/dp02-automation/final.log` and `index.json`.

### Controls, packages and checks

- Packaged controls on the same executable/source identity: DP-01-A/B **PASS** (goal projection false, correctly not DP-02); NQ-01-A/B **PASS** (real disconnected query, result 2/partial); NQ-02-A/B **PASS** (real partial result rejected as `PartialPath`); PC-01 static obstacle route **PASS** (4 points, length 1632.748039 cm, 176 fixed steps, one arrival); PC-02 ordinary no-selector startup **PASS** in both archives. Evidence directories are retained under `Saved/Packages/DP02Fixture/Linux/navigation-destination-failure-*`, `navigation-unreachable-*`, and `navigation-obstacle-acceptance-*`.
- Normal Linux Development UAT archive: `/mnt/data/politika-qualification/nav-a15-dp02-20261029-3994baa/Saved/Packages/DP02Normal/Linux`; fixture archive: matching `DP02Fixture/Linux`. Both UAT BuildCookRun operations exited 0. Their packaged executable SHA-256 is `2eac6f63dbaa70ccfcc685ab645a1bfcc9645960e07e970105cce967202f5586`. The normal UFS manifest excludes `TestFixtures/`; fixture UFS includes this map/definition and cooked `.ubulk` Recast data. Both archives boot MainMenu without a selector via the copied-project qualification overlay only; its `DefaultEngine.ini` SHA-256 is `f25e45739054d02c75a7f0c07c1ab5d320329ee097c2c578f9b3e65966f5abb9`. The tracked source config hash remains `2af4a7f33dd93f7b272faeb9fdd8083c2311f9cf724a60dcfc4271f3c2b2ac5c` and still selects RTSVerticalSlice; no canonical startup-default change is claimed.
- Package hashes: normal UFS manifest `c0ce23dcb0e06aff640bf533d994d4411bb25da266e543c41dad333e7092620d`, `.pak` `532ff7cf0113cd7738953297745952ee37283d69a2e3e5cff6484999b7960728`, `.ucas` `fbb77d42e9810a490bfa20cb79070f9476e4c0b1b57087ee54a0b547d5f1054a`, `.utoc` `227b31cda6bd4c5097c37792e28a08250df1ddc43fa8149934a4c27120a25a88`; fixture UFS manifest `443d5fbf5c1f8f5b420dd4b27959a4c9e748c321c8cdd682c62a72bb47046c1e`, `.pak` `64148d2cb306641cb25acd65667e45c0c87a686fbf7b26931880acab3b1426aa`, `.ucas` `fa5a3bb0f2f43a287009362a15a4c7db038893bfad04ee96b2ad07ed633ffe69`, `.utoc` `d6912c16a0408cf27c5ac2559af71cb0fc9cb964247d8767a5d60ede798e820f`. The global IoStore containers match between archives.
- `Politika.RTS.Navigation`: 7 discovered/executed, 0 ordinary successes, 7 warning-bearing successes, 0 failed/skipped. `Politika.RTS`: 14/14, 3 ordinary successes, 11 warning-bearing successes, 0 failed/skipped. `Politika.Content`: 7/7 ordinary successes, 0 warnings/failures/skips. Exported reports and logs are in `build/diagnostics/automation-nav-all/`, `automation-Politika-RTS/`, and `automation-Politika-Content/`. The expected warnings arise from negative-path or transient Editor/Recast cleanup.
- Passed: pinned `tools/unreal.sh doctor`; Editor Linux Development build; Linux Game Development BuildCookRun; portable `quality -- core`; repository `quality -- audit`; Rust Shell tests (6/6); Rust format check; and `git diff --check`. UAT logs: `build/diagnostics/dp02-normal-uat.log` and `dp02-fixture-uat.log`. The fixture package is opt-in and no test fixtures leaked into the normal cook.
- The normal/fixture UFS manifest check and executable identity passed. Canonical RTS map, map definition, content catalog, unit definition, existing static-obstacle/disconnected/DP-01 fixture hashes were inventoried before/after and unchanged. New DP-02 assets: definition SHA-256 `685c8df8b9a399b40c93b3179e6615095b25c49e6441486284d00c097caa1449`; map SHA-256 `fc195b468fb10df2399b918c087ab652d4ed8da9e0febe914b9150c7fb66e45b`.
- Build sidecar identity: game version `0.1.0-dev.0`, build label `local-dp02-20261029`, source revision `3994baa5e264-dirty`, pinned engine revision above. The exact source-input manifest is retained at `build/diagnostics/dp02-final-source-inputs.sha256`, SHA-256 `44692db5a504ca146989c3e9a271214acdecd80d5745d299a4da5c36e1ecbb8a`; identical `QualificationSource.sha256` sidecars were placed beside both archives. Executable SHA-256 is identical across normal and fixture packages.

### Scoped result and integration status

`DP-02 = PASS` for this identified source/engine/package snapshot. This closes only the successful projection with displacement strictly above the existing 150 cm tolerance. `DP-01`, NQ-01, NQ-02 and PC-01/PC-02 controls pass; `ER-01` and `ER-02` remain `INCONCLUSIVE`; overall `NAV-A15 = INCONCLUSIVE`.

A scoped implementation commit is recorded on the isolated branch. No publication was attempted: the branch is a child of `3994baa5...`, while local `main` is divergent (`5c9db959...`) and has dirty paths that overlap task source/docs/harness. A safe local-main fast-forward is impossible from its current ancestry/state, and publishing without satisfying the requested safe local synchronization gate is prohibited. No force-push, reset, stash, merge, overwrite or shared-checkout mutation was performed. Preserve this qualification worktree and its package/evidence artifacts for a later explicit reconciliation; do not mistake this qualification for a published release.

### DP-02 implementation-gate ledger

| Gate | Verdict | Evidence |
|---|---|---|
| DP2-A01–A05 | PASS | Shared/remote/protected-worktree preflight, dirty work preservation, fresh worktree at current `origin/main`, and pinned UE 5.8.3 revision recorded above. |
| DP2-A06–A10 | PASS | Owning docs/source reviewed; typed XY/Z conversion, production extent/threshold, pinned Recast implementation and actual successful 168 cm probe recorded. |
| DP2-A11–A16 | PASS | Isolated saved fixture validates/activates; origin and ground hit verified; typed target in bounds; goal projects successfully; displacement is strictly above 150 cm. |
| DP2-A17–A21 | PASS | Actual guard is `DestinationNotNavigable`; query did not execute; route not installed and identity/cursor retired. |
| DP2-A22–A25 | PASS | One failure, no arrival, failure step plus 20 inert authoritative steps; Active match and live owned unit retained. |
| DP2-A26–A27 | PASS | Two fresh package processes agree exactly at serialized 1e-6 cm coordinates and 1e-9 cm displacement precision; no divergent step/field. |
| DP2-A28–A31 | PASS | DP-01, NQ-01/NQ-02, PC-01 complete route, and PC-02 MainMenu startup controls pass. |
| DP2-A32–A35 | PASS | Focused and broad Automation filters, Editor/Game builds, UAT, portable core/audit, Rust tests/fmt and diff checks pass. |
| DP2-A36–A40 | PASS | Normal/fixture cook isolation, package/source/engine hashes and canonical asset preservation checked; owning docs updated. |
| DP2-A41 | PASS | Scoped local commit recorded on the isolated task branch; no unrelated paths included. |
| DP2-A42 | BLOCKED | Local `main` is divergent from this `origin/main` base and has overlapping dirty paths; no non-destructive synchronization path exists from the current state. |
| DP2-A43 | BLOCKED | Conditional publication stopped because A42 cannot be established; no push attempted. |
| DP2-A44 | BLOCKED | Shared local main left untouched; cannot safely fast-forward it while preserving the divergent history and dirty user work. |
| DP2-A45 | SKIPPED | No remote publication or local-main movement was attempted, so no new integrated SHA exists to compare. |

## NAV-A15 destination-projection and invalid-result qualification — 2026-10-29

This isolated follow-on is based on fetched `origin/main` `2fe95b6e726a64a9e748381735abf429759c177f`, branch `codex/nav-a15-destination-20261029`, worktree `/mnt/data/politika-nav-a15-destination-20261029`. The shared checkout remained on `main` at `5c9db959360f4d41ba693268126f9aac79f66164`, ahead 1/behind 34, with its tracked/untracked dirty paths preserved. The previous NAV-A14 commit `cf56f4594ae3ed69aa018f2de58e491dcb9e8a30` was not merged: only its 13-file first-parent navigation delta was selectively applied. Its 237 other origin-to-commit paths, including unrelated environment assets and research, were excluded. The required obstacle fixture helper and existing movement/query infrastructure were already present at this upstream base.

Pinned engine: Unreal 5.8.3, source revision `396c9f059903aed5fec78ecd3d437a40c6415368`; `tools/unreal.sh doctor` passed. The engine checkout was already dirty at preflight: 16 tracked third-party/build helper script paths and one untracked `Engine/Config/DefaultEngine.ini`; `git diff --binary` SHA-256 is `8206134d2318bde2dc2c13695c470985455932d4c4cc10755813ef2065231546`, and the untracked config SHA-256 is `2a20b23636bc31095df4bfd6770da4ea6f91b915667d9293b66c04c7af15e984`. None was modified by this task. Editor Development build, portable core, repository audit, six Rust tests, Rust format and `git diff --check` passed. The required Automation filters discovered/executed: Navigation 6/6 (0 ordinary successes, 6 warning-bearing successes, 0 failed/skipped), RTS 13/13 (3 successes, 10 warning-bearing successes, 0 failed/skipped), Content 7/7 (7 successes, 0 warnings, 0 failed/skipped). Their JSON indexes are under `Saved/NavA15Automation/Politika_RTS_Navigation/`, `Politika_RTS/` and `Politika_Content/`; the new `SavedMapUnprojectableDestinationFailsClosed` is in all applicable counts. Reported warnings are existing Editor/Recast and expected negative-path diagnostics.

### DP-01 fixture and production evidence

The Development-only selector is `-PolitikaTestFixture=UnprojectableDestination`, map ID `map.test.unprojectable_destination`, level `/Game/Politika/TestFixtures/Navigation/W_UnprojectableDestination`. The optional runtime Move target is not a required start or objective. Its XY is `(1300,1300)`, inside playable XY bounds `[-2000,2000]`; the authored ground collision trace hit the tagged ground at `(1300,1300,-14)` within 5 cm. A separate static `NavModifierVolume` with `UNavArea_Null` bounds `[1000,1600] x [1000,1600] x [-58,142]` excludes the NavMesh while leaving the ground collision query usable. The saved-level authoring commandlet and its reload check recorded origin projection success and destination projection failure with extent `(80,80,180)`; runtime Recast was ready and static (`runtime_generation=0`).

The typed portable order carries XY only; the existing production adapter reconstructs its requested unit-center Z as 42 cm. Therefore the runtime `NAV_FAILURE_PROJECTION` requested vector is `(1300,1300,42)`, while the independent ground-surface check records `Z=-14`; this is the established unit-center conversion, not a different XY target or a fabricated projection point. On both fresh packaged runs the actual production branch logged `origin_projected=1`, `goal_projected=0`, `query_executed=0`, and `DestinationNotNavigable`. Query result, partial flag, path validity, point count, projected goal and displacement were explicitly `NA`, because `FindPathSync` did not execute.

DP-01-A/B both passed in independent Linux Development processes from the same fixture package. Each log records one accepted typed Move, one `NavigationFailed` order event, no arrival, zero path queries, retired route/cursor/identity, stationary live owned unit, and 21 authoritative samples (the failed step plus 20 further fixed steps). The final result is `PASS`, `failure_classification=DestinationNotNavigable`, `queries=0`; the terminal record uses `NA` for every query/path field. No AI movement, retry, stale route or alternate projected target was observed. Runtime logs and unique `-UserDir` paths are retained in `Saved/Packages/NAVA15FixtureFinal/Linux/navigation-destination-failure-1793235847693472564/`.

### Real-result investigation and final-source controls

- `DP-02` excessive projection displacement: `INCONCLUSIVE`, not separately reproduced. No tolerance/extent was changed, and no floating or synthetic destination was used to claim this condition.
- `ER-01` empty usable path and `ER-02` invalid path/waypoint: `INCONCLUSIVE`. The bounded genuine packaged queries exercised production complete-only (`allow_partial=0`: result `2`/Fail, `partial=1`, valid two-point diagnostic result) and an actual diagnostic partial-enabled query (`allow_partial=1`: result `3`/Success, `partial=1`, valid two-point partial, endpoint error 1028.031128 cm), plus the static-obstacle complete-route Success control. None returned an empty or malformed path. Source inspection confirms that the production route boundary rejects non-success, partial, invalid/empty-path and non-finite waypoint results; source inspection alone is not runtime qualification. No result was synthesized.
- Final-source NQ-01 disconnected no-complete-path: **2/2 PASS**, actual production query result `2`, partial flag `1`, route rejected as `NoCompletePath`; each process had one failure publication and 21 samples. NQ-02 genuine partial-result rejection: **2/2 PASS**, actual result `3`, partial `1`, two points and production classification `PartialPath`, never installed. The same immutable fixture package also passed one packaged complete static-obstacle detour (`arrival_step=156`, final step 176, no failures).
- PC-02 no-selector startup: fixture selection and acceptance mode were absent in both normal and fixture archive launches and no navigation acceptance markers appeared, but both actually loaded `/Game/Maps/RTSVerticalSlice` and reached Active. The tracked `origin/main` `Config/DefaultEngine.ini` sets `GameDefaultMap=/Game/Maps/RTSVerticalSlice`, contrary to this increment's required MainMenu startup. The separate dirty shared-checkout menu configuration was not imported, and this slice did not change the default startup configuration. Therefore PC-02 is **FAIL against the stated MainMenu requirement**, despite the normal UFS manifest excluding every `TestFixtures/` asset. This is an explicit publication blocker, not a MainMenu pass.

### Package identity and verdict

Final archives are `Saved/Packages/NAVA15NormalFinal/Linux/` and `Saved/Packages/NAVA15FixtureFinal/Linux/`. Both came from the same final executable SHA-256 `675d899857248d4c90656ca29c9f8340a41313270cde5991775de2c3de220590`; build identity is `2fe95b6e726a-dirty`, engine `5.8.3-0+UE5`. Scoped qualification source input manifest hash: `769108ec786c00a0eb1128bdb379052e9f5d9aa6bb6bd34603b4e55c49e2851a`. Normal UFS contains no `TestFixtures/`; fixture UFS contains DP-01, disconnected and static-obstacle fixtures. Pak/IoStore hashes: normal `.pak` `6ef0866cecc8ecb74152bcce67ccc8cb426f6060de303d6424127ec9f3463b59`, `.ucas` `fbb77d42e9810a490bfa20cb79070f9476e4c0b1b57087ee54a0b547d5f1054a`, `.utoc` `227b31cda6bd4c5097c37792e28a08250df1ddc43fa8149934a4c27120a25a88`; fixture `.pak` `f43c753589d0e0f8883381dae336d41b83eea970c4ddcc626ffbbfb622f260f4`, `.ucas` `f16a62179e3cf1fa47e052f852671f90b148289a5398f55b310bcf72ab171364`, `.utoc` `952351314f11bc3b7f119fc42d7df11483438d06a8b0c0327d139bb3f1d27a1d`. Cook logs are under `Saved/NavA15Packaging/`.

Subcase verdicts: DP-01 **PASS**; DP-02 **INCONCLUSIVE**; NQ-01 **PASS**; NQ-02 **PASS**; ER-01/ER-02 **INCONCLUSIVE**; no-silent-fallback for DP-01 **PASS**; package fixture exclusion/inclusion **PASS**; PC-02 MainMenu startup **FAIL**. Overall **NAV-A15 INCONCLUSIVE**; do not claim complete closure. Because PC-02 failed and upstream/local-main reconciliation is unsafe (the shared main is divergent and dirty, with incoming conflicts in navigation sources, docs, config and Rust harness), no commit or push was made. The isolated worktree and evidence are preserved; no force/reset/stash/merge/rebase or shared-checkout edit was performed.

### Final copied-config startup qualification and publication reconciliation — 2026-10-29

The preceding PC-02 failure applies to the first `NAVA15NormalFinal` / `NAVA15FixtureFinal` archives, whose tracked source config defaults to RTSVerticalSlice. It is superseded by the final copied-config qualification archives below; the project checkout's default config was not changed.

An isolated snapshot at `/mnt/data/politika-qualification/NAVA15-Destination-20261029` was copied from this exact source/content state. Only the snapshot's `Config/DefaultEngine.ini` changed `GameDefaultMap` from `/Game/Maps/RTSVerticalSlice` to `/Game/Maps/MainMenu`; `EditorStartupMap` remained RTSVerticalSlice. The authoritative worktree config remained SHA-256 `2af4a7f33dd93f7b272faeb9fdd8083c2311f9cf724a60dcfc4271f3c2b2ac5c`; the snapshot override is SHA-256 `f25e45739054d02c75a7f0c07c1ab5d320329ee097c2c578f9b3e65966f5abb9`. This is the documented qualification-only startup overlay, not a source/default-map change. Both archives were cooked/staged serially from the same copied project and exact already-built executable; UAT logs are `Saved/NavA15Packaging/normal-mainmenu-final.log` and `fixture-mainmenu-final.log`, both `BUILD SUCCESSFUL`, exit 0. The sidecars retain the source-input manifest hash `769108ec786c00a0eb1128bdb379052e9f5d9aa6bb6bd34603b4e55c49e2851a` and engine pin. The overlay hash is separately recorded in `Saved/NavA15Packaging/mainmenu-overlay-config.sha256`.

Final archives: `Saved/Packages/NAVA15NormalMainMenuFinal/Linux/` and `Saved/Packages/NAVA15FixtureMainMenuFinal/Linux/`. Both executable SHA-256 values are `675d899857248d4c90656ca29c9f8340a41313270cde5991775de2c3de220590`. Normal UFS has zero `TestFixtures/` entries; fixture UFS includes the static-obstacle, disconnected and unprojectable-destination maps/definitions. Normal game Pak/IoStore SHA-256: `.pak` `a0cc583dcfba94abb50908cef552f713114329ed01e244d03c1b2a8c67ed4bb6`, `.ucas` `fbb77d42e9810a490bfa20cb79070f9476e4c0b1b57087ee54a0b547d5f1054a`, `.utoc` `227b31cda6bd4c5097c37792e28a08250df1ddc43fa8149934a4c27120a25a88`. Fixture game Pak/IoStore SHA-256: `.pak` `571d165370df9974e96b062433053a0170a4d49eb8812f2d55d21c1d41794426`, `.ucas` `f16a62179e3cf1fa47e052f852671f90b148289a5398f55b310bcf72ab171364`, `.utoc` `952351314f11bc3b7f119fc42d7df11483438d06a8b0c0327d139bb3f1d27a1d`; shared global IoStore hashes are `global.ucas` `75d53a95f496cdb12f519a135db424c0f99b4a9d4847467f2cf69cb0a3a8c1e5`, `global.utoc` `6ac326ab2c7910be85a12d071d7e3fa55785f52ee5e6995b390b784f68e5c5f2`. The Rust `--static-obstacle` acceptance mode launched both archives without map/fixture selectors and recorded `/Game/Maps/MainMenu` for each, then ran the explicit cooked complete-route fixture: `Saved/Packages/NAVA15FixtureMainMenuFinal/Linux/navigation-obstacle-acceptance-1793238213/results.tsv` is NAV-A25 **PASS**, four route points, 176 authoritative samples, 157 swept segments, one arrival at step 156 and 20 inert post-arrival steps.

The mandatory two final-source negative runs were repeated against the copied-config archives: `Saved/Packages/NAVA15FixtureMainMenuFinal/Linux/navigation-destination-failure-1793238228712187666/acceptance.tsv` records DP-01-A/B **PASS**. Each fresh process reports ground hit at `(1300,1300,-14)`, in-bounds/static NavMesh exclusion, requested unit-center goal `(1300,1300,42)`, ready `RecastNavMesh-Default`, origin projection true, goal projection false, production extent `(80,80,180)`, query `NOT_EXECUTED` (serialized result fields `NA`), `DestinationNotNavigable`, one accepted Move, one failure publication, zero arrivals, retired route identity and 21 stationary authoritative samples. The terminal records are in `DP-01-A/B-runtime.log`; each process also has an isolated `-UserDir` and original command line. NQ-01 and NQ-02 were likewise rerun on these archives: `navigation-unreachable-1793238240072148615/acceptance.tsv` records two genuine disconnected failures and two real partial-path rejections, all **PASS**.

Final verdicts: DP-01 **PASS**; PC-01 complete-route control **PASS**; PC-02 no-selector MainMenu startup **PASS with the explicit copied-config qualification overlay**; NQ-01/NQ-02 **PASS**; DP-02 and ER-01/ER-02 remain **INCONCLUSIVE**. Therefore `NAV-A15 — invalid/unprojectable-goal subcase = PASS`, while `NAV-A15 — empty/invalid-path-result subcase = INCONCLUSIVE` and `NAV-A15 — overall = INCONCLUSIVE`, as required. Do not use the first failed archives as final package evidence.

### UG-A01–UG-A36 reconciliation

| ID | Verdict | Evidence summary |
|---|---|---|
| UG-A01 | PASS | Accepted navigation commit `cf56f4594ae3ed69aa018f2de58e491dcb9e8a30`; exact first-parent navigation delta selectively carried to the verified `origin/main` base. |
| UG-A02 | PASS | Shared dirty checkout remained untouched; initial status/inventory preserved in task preflight. |
| UG-A03 | PASS | Isolated branch/worktree `codex/nav-a15-destination-20261029` at `/mnt/data/politika-nav-a15-destination-20261029`. |
| UG-A04 | PASS | Local `main` was ahead 1/behind 34; remote history and non-fast-forward accepted branch ancestry inspected; unrelated merge/assets excluded. |
| UG-A05 | PASS | UE 5.8.3 doctor passed against pinned revision `396c9f059903aed5fec78ecd3d437a40c6415368`. |
| UG-A06 | PASS | Required owning and supporting acceptance/design/governance documents reviewed. |
| UG-A07 | PASS | Production typed-admission/projection/query/route/event pathway audited; projection failure precedes `FindPathSync`. |
| UG-A08 | PASS | Saved map `/Game/Politika/TestFixtures/Navigation/W_UnprojectableDestination`, ID `map.test.unprojectable_destination`; only isolated test assets added. |
| UG-A09 | PASS | Cooked fixture validates and match reaches Active. |
| UG-A10 | PASS | Active lifecycle observed in the packaged runtime trace. |
| UG-A11 | PASS | Correct unit origin projects onto ready static Recast data. |
| UG-A12 | PASS | Finite in-bounds goal on tagged collision-bearing ground; ground trace and bounds evidence recorded. |
| UG-A13 | PASS | Actual production goal projection boolean is false. |
| UG-A14 | PASS | Explicit diagnostic says `query_executed=0`; no query record exists for either projection-failure attempt. |
| UG-A15 | PASS | One authorized typed Move is accepted. |
| UG-A16 | PASS | No executable route is installed. |
| UG-A17 | PASS | Waypoints/cursor and executable navigation generation/sequence retire. |
| UG-A18 | PASS | One `NavigationFailed` publication per attempt. |
| UG-A19 | PASS | No arrival publication. |
| UG-A20 | PASS | Failure step plus 20 later authoritative steps remain stationary. |
| UG-A21 | PASS | Match remains Active; unit remains alive, owned, and valid. |
| UG-A22 | PASS | Two fresh packaged DP-01 runs pass. |
| UG-A23 | PASS | Final copied-config package passes cooked complete-route static-obstacle control. |
| UG-A24 | PASS | Final package disconnected NQ-01 2/2 and genuine partial NQ-02 2/2 pass. |
| UG-A25 | PASS | Registered `SavedMapUnprojectableDestinationFailsClosed` discovered in Navigation 6/6 passing-with-warnings report. |
| UG-A26 | PASS | RTS 13/13 and Content 7/7; zero failed/skipped. |
| UG-A27 | PASS | Pinned Editor Development build and Linux Development Game/UAT packaging succeeded. |
| UG-A28 | PASS | Normal and fixture UAT archives succeeded from final-source snapshot. |
| UG-A29 | PASS | Normal manifest excludes fixtures; no-selector MainMenu verified in normal archive. |
| UG-A30 | PASS | Fixture archive retains MainMenu overlay and includes intended cooked navigation fixture dependencies. |
| UG-A31 | PASS | Core/audit, six Rust tests, Rust format, and source diff checks passed. |
| UG-A32 | PASS | No canonical authored assets were modified; only the two scoped fixture assets were added. |
| UG-A33 | PASS | Source-input manifest, engine pin, executable, Pak/IoStore and runtime evidence identities recorded. |
| UG-A34 | PASS | Owning navigation record and outline reconciled without promoting overall NAV-A15. |
| UG-A35 | PASS | The reviewed 15-file task patch was committed and fast-forward-published to GitHub main from its exact `origin/main` base; see the publication reconciliation below. No unrelated history or paths were included. |
| UG-A36 | BLOCKED | GitHub main publication is verified. Shared local `main` remains at `5c9db959360f4d41ba693268126f9aac79f66164`, ahead/behind remote and carrying dirty overlapping user work; it was deliberately not advanced or cleaned. A separately reviewed local reconciliation is still required. |

The DP-01 destination-projection qualification is published on GitHub main. The shared local checkout was not modified as part of publication: its branch and uncommitted work remain preserved pending a separate safe local-main reconciliation. The remaining technical gap is the empty/invalid-path-result subcase; overall NAV-A15 remains INCONCLUSIVE.

### Scoped publication reconciliation — 2026-10-29

After rechecking live `origin/main` at `2fe95b6e726a64a9e748381735abf429759c177f`, the isolated task branch was confirmed to be based exactly on that commit. The reviewed change set comprised only 15 navigation implementation, fixture, test/harness and owning documentation paths listed in the scoped commit. No unrelated history was imported. The commit was pushed as a normal fast-forward to GitHub main and the remote ref was independently verified at the resulting commit.

The shared checkout `/mnt/data/politika` was intentionally left untouched. Its local `main` is `5c9db959360f4d41ba693268126f9aac79f66164` and has extensive pre-existing staged/unstaged/untracked work, including overlapping navigation source, commandlet, documentation and Rust harness files. It cannot be fast-forwarded safely in-place without a separate scoped reconciliation. No reset, clean, stash, merge, rebase, or force-push was performed. The task worktree is clean after the scoped commit; other pre-existing worktrees and branches were not modified.

## NAV-A15 ER-01 genuine empty-path source feasibility — 2026-10-29

This isolated continuation starts from the accepted DP-02 commit `0061531fc9a5d4f992b71ec9905627593e6a4020` (parent `3994baa5e2646adaefba9dda27645b4e6a1fa7d2`) in branch `codex/nav-a15-er01-empty-path-20261029`, worktree `/mnt/data/politika-nav-a15-er01-empty-path-20261029`. The worktree was initially clean. It records only this feasibility report; it contains no fixture, production-code change, injected result, or Automation placeholder.

### Baseline and preservation

- Refreshed `origin/main` is `3994baa5e2646adaefba9dda27645b4e6a1fa7d2`. Shared `/mnt/data/politika` remains at `5c9db959360f4d41ba693268126f9aac79f66164`, one commit ahead and 35 behind, with extensive staged, unstaged and untracked non-navigation work. Nothing in that checkout was edited, built, cleaned, stashed, switched or overwritten.
- The accepted DP-02 worktree is clean at `0061531fc9a5d4f992b71ec9905627593e6a4020`; its single outgoing commit is the accepted excessive-projection qualification. Its parent is current remote `main`. The destination and path-results worktrees are clean; the protected path-results worktree has a pre-existing modification to its acceptance document and was left alone. The unprojectable-goal run-2 worktree is clean. The older unreachable-path worktree is three commits ahead and 13 behind its tracking base and was left untouched.
- During preflight, a UE Editor-Cmd infantry import was actively running against the shared checkout. No UBT, UAT, Editor, Game or fixture-generation action was started for this investigation. The process was not interrupted.
- UE installation `/mnt/data/unreal/5.8.3` is pinned at `396c9f059903aed5fec78ecd3d437a40c6415368`. Its pre-existing working tree contains modified third-party `.bat` scripts and untracked `Engine/Config/DefaultEngine.ini`; none was changed. The inspected NavigationSystem/Recast/Detour source files themselves are clean in Git. No source/build/package identity from a historical runtime was relabeled as fresh ER-01 evidence.
- Inspected engine-source SHA-256: `NavigationSystem.cpp` `c1a7627988734d09b8fb73196d75da07c93658bd9f1443b9fa1bef26428b5035`; `NavigationPath.cpp` `dad5d5098f98b806183f2e81ab9c16f700ac020a7ef0cdaac60bb957e005dc87`; `NavigationData.h` `601bf1bf54e7797c09d2ad7d713f3adbd287115832259c848a3ee0c666af68f0`; `RecastNavMesh.cpp` `e93db68b6fc73c87224c2e76dd2aa8e3262312bfca15b9ffa4169fc1d755997f`; `PImplRecastNavMesh.cpp` `e65edd6bacf87cf6e3f13381f5a987c4af26ba0c2d74de6c878dcade4b7070e2`; `NavMeshPath.cpp` `af0cb3c7aac2e49713d302cf02147771e6fa868113d8ae2b8f7b54af0dfdbccc`; `DetourNavMeshQuery.cpp` `ba021719f76f531059443c06fea2c766fed2091983553f6122d3b3768afa02fa`.
- Relevant POLITIKA source hashes at this base: `PolitikaPrototypeGameplay.cpp` `09c2713f4ea336b814c4878fa273211f031d2e413c6eb2c0c14f34fa8b4b4c9c`; `PolitikaPrototypeGameplay.h` `f43966d74aeb2cc827791b4b04158d556ac31f8860d2fe9b65832ed5c609ff18`. The pre-edit owner-document hash was `17e405aff7f1444abe293b121a405eeb41e04899d59875516ee581146cc5afd0`.

### Production guard and validity semantics

The isolated source confirms `APolitikaPrototypeMatch::IsCompleteNavigationResult` checks, in order: `Success` and non-null shared path pointer (`Result.Path.IsValid()`), non-partial path object, then non-empty `GetPathPoints()`, returning `EmptyPath` only at the final check (`Source/Politika/Private/PolitikaPrototypeGameplay.cpp:1371-1389`). Production `ResolveNavigationRoute` projects both endpoints using the unchanged `(80,80,180)` extent, uses the supported movement profile/default NavData, sets `allow_partial=false` and `require_navigable_end=true`, calls `FindPathSync`, then invokes this validator (`:1392-1497`). If the validator rejected an empty result, no waypoint copy or route install follows.

The two `IsValid` calls must not be conflated:

- `FNavPathSharedPtr::IsValid()` in `Result.Path.IsValid()` tests whether the shared pointer exists. The production guard does **not** call `Result.Path->IsValid()`.
- Pinned `FNavigationPath::IsValid()` is `bIsReady && PathPoints.Num() > 1 && bUpToDate` (`Engine/Source/Runtime/NavigationSystem/Public/NavigationData.h:94-97`). Thus a zero-point object is invalid by the engine's path-object contract even if a shared pointer to it is non-null. `FPathFindingResult::IsPartial()` separately returns `(Result != Error) && Path.IsValid() && Path->IsPartial()` (`NavigationData.h:1176-1178`); partial status is not inferred from the point count.

### Pinned engine result-construction audit

`UNavigationSystemV1::FindPathSync` selects the query's NavData (or default data for the overload without explicit agent properties), dispatches to that NavData's synchronous `FindPath`, and returns its `FPathFindingResult` without replacing the path (`Engine/Source/Runtime/NavigationSystem/Private/NavigationSystem.cpp:1887-1932`). The production query names the default Recast data and does not change query settings.

The pinned Recast construction has two relevant branches (`Engine/Source/Runtime/NavigationSystem/Private/NavMesh/RecastNavMesh.cpp:3853-3895`):

1. When the projected query start and adjusted end are nearly equal, Recast resets the point array, adds exactly one `FNavPathPoint(AdjustedEndLocation)`, and sets `Result=Success`. It does not return zero points. This includes coincident and sufficiently near-coincident endpoints that enter this branch.
2. Otherwise it calls `FPImplRecastNavMesh::FindPath`. A non-searchable query returns Error. A Detour partial result sets the path's partial bit; the Recast wrapper maps it to Success only if partial paths are allowed, otherwise Fail. Production disallows partial paths and requires a navigable end.

For a successful non-degenerate Recast query, `PostProcessPathInternal` calls `PostProcessPath` and then marks the path ready (`PImplRecastNavMesh.cpp:1339-1388`). Detour `findPath` validates both polygon references and, when start and end refs match, adds the start ref to its result before returning Success (`Engine/Source/Runtime/Navmesh/Private/Detour/DetourNavMeshQuery.cpp:1625-1649`). On the normal success path, Recast only trims a terminal nav-link polygon when corridor size is greater than one (`PImplRecastNavMesh.cpp:1530-1543`), so it cannot reduce a one-polygon successful corridor to zero. With string pulling enabled (the `FNavMeshPath` default), `PerformStringPulling` calls `FindStraightPath`; Detour validates the first polygon and appends the closest start vertex before processing the corridor, then appends the end vertex on successful completion (`Engine/Source/Runtime/Navmesh/Private/Detour/DetourNavMeshQuery.cpp:2585-2610, 2915-2916`). The Recast adapter resets and fills `PathPoints` from those returned straight-path vertices (`PImplRecastNavMesh.cpp:1638-1675`). If string pulling is disabled, Recast explicitly appends both start and end points (`PImplRecastNavMesh.cpp:1612-1616`). Thus normal successful straight-path construction has at least one returned point (possibly one coalesced point for a degenerate path).

There is, however, an important unchecked second-stage failure path: Detour `findStraightPath` can return failure if the first/last corridor polygon cannot be resolved (`DetourNavMeshQuery.cpp:2591-2603`). `FPImplRecastNavMesh::FindStraightPath` resets `PathPoints` before it checks the straight-path status, leaving it empty on such failure (`PImplRecastNavMesh.cpp:1646-1653`). `FNavMeshPath::PerformStringPulling` records that boolean, but its caller does not propagate it into the already-successful `FindPathStatus`; `PostProcessPathInternal` returns the original Detour route-search status (`PImplRecastNavMesh.cpp:1360-1388`). Therefore source alone does **not** exclude a Success/non-partial/zero-point result if a genuine query's route search succeeds and its subsequent straight-path extraction fails. The identified failure preconditions appear inconsistent with the valid corridor produced and consumed synchronously from the same static Recast data, but that invariant is not asserted at the call boundary. No supported ordinary geometry/setup that triggers this branch was established here.

This conclusion is scoped to the pinned engine's normal `UNavigationSystemV1::FindPathSync` → default static `ARecastNavMesh` path with POLITIKA's unchanged settings. It is not a claim about custom `ANavigationData`, test-owned path mutation, corrupted Detour internals, other engine revisions, or fabricated results. The latter are outside ER-01's permitted reproduction conditions.

### Feasibility and bounded-probe decision

| Candidate | Source result under production-equivalent conditions | Runtime probe |
|---|---|---|
| Ordinary separated endpoints, same polygon, or short nonzero separation | Normal valid-corridor processing appends at least one string-path point. | Four genuine `FindPathSync` queries ran on the existing saved static Recast map; see results below. None returned zero points. |
| Coincident/nearly coincident projected endpoints | Recast's explicit success fast path returns exactly one point. It cannot qualify as zero points. | SP-01 observed Success, non-partial, one point. |
| Disconnected/partial route | No complete route returns non-success; a partial route is marked partial and production maps it to Fail with `allow_partial=false`. Neither meets the required signature. | Existing historical controls remain historical; not rerun or counted as ER-01 evidence. |
| Missing/unready NavData or failed initialization | FindPathSync/Recast returns Error, not successful empty output. | Not a candidate for ER-01. |

Classification: **`SOURCE_UNRESOLVED`**. The normal pinned Recast branches produce at least one point, but the unchecked string-path extraction failure leaves open a genuine engine-returned empty path if a real successful route corridor becomes invalid before extraction. No supported geometry or stable static-nav setup triggering that condition was identified. The separate `FNavigationPath::IsValid()` predicate would be false for a hypothetical zero-point object, while POLITIKA's production guard only requires a valid shared pointer. This distinction means `EmptyPath` is the later production branch if such a result is returned, but it is not evidence that Recast returns one.

#### Bounded real-query probe

A Development-only Editor Automation observer was added at `Politika.RTS.Navigation.StraightPathResultProbe`; it loads the existing `/Game/Politika/TestFixtures/Navigation/W_DisconnectedNavigation` map and its saved static `RecastNavMesh-Default`. It uses a real `APolitikaPrototypeUnit` as `FPathFindingQuery` owner and the production query settings (`allow_partial=false`, `require_navigable_end=true`), then calls `UNavigationSystemV1::FindPathSync` directly. It does not mutate query results, alter production navigation settings/code, admit a typed Move, install a route, or create a fixture. The four requested endpoints independently projected successfully; origin was `(-700,-600,-10)`. Runtime reports include the projected endpoint and actual separation:

| Case | Projected separation | Result | Shared path | Path object `IsValid()` | Partial | Points |
|---|---:|---|---|---|---:|---:|
| SP-01 coincident | 0 cm | Success (3) | valid | false | false | 1 |
| SP-02 short | 1 cm | Success (3) | valid | true | false | 2 |
| SP-03 same-area separated | 100 cm | Success (3) | valid | true | false | 2 |
| SP-04 same-area separated | 250 cm | Success (3) | valid | true | false | 2 |

This demonstrates the distinction in the real engine: the coincident Success result has a non-null shared path and one point while the path object's own `IsValid()` is false; the three separated Success results have non-null, valid, non-partial paths and two points. **None** meets the ER-01 signature (Success + valid shared pointer + non-partial + zero points). This is bounded negative runtime evidence for these four supported static-nav queries only; it neither exercises the identified string-path extraction failure nor proves that branch unreachable. The Automation record reports one warning, `LogCrowdFollowing: Unable to find RecastNavMesh instance while trying to create UCrowdManager instance`; the focused test completed Success with zero errors.

Evidence: Linux UE 5.8.3 Editor Development build succeeded using pinned engine revision `396c9f059903aed5fec78ecd3d437a40c6415368`; produced Editor executable SHA-256 `8c225784bffd4b9c9b7744541d4d8ca41b585a414b9b8f4c661c7b1d66d5c6df`. The focused Automation report is `build/diagnostics/er01-straight-path-probe/index.json` (SHA-256 `d6d05d3d48ff7f2a56f34cc4c3ef51e0b5eae546d57d924fdb99a7c75e373be6`); captured run log is adjacent `run.log`. Observer source SHA-256 is `27fa973908f82360493b095856cbe36636772c792e5b6f3b21e66255e9cfb8b1`. No Linux game build/cook/package, packaged runtime, normal-cook fixture exclusion test, or POLITIKA production resolver/typed-Move rejection was executed.

### Verdicts and remaining work

The source feasibility category is not an acceptance PASS. `ER-01 = INCONCLUSIVE` (no qualifying runtime result observed in the four bounded queries); `ER-02 = INCONCLUSIVE`; overall `NAV-A15 = INCONCLUSIVE`. No claim is made that packaged `EmptyPath` rejection, exactly-once event publication, stationary movement, fresh-process repetition, package controls, or full acceptance gates ER1-12 through ER1-40 passed. The smallest justified next dependency is a newly identified supported Unreal/Recast source branch or reproducible engine defect that can produce a successful complete result with no points without mutating the result; absent that, do not author a broader fixture suite or synthetic result. ER-02 remains separate future work.

| Gate | Verdict | Evidence / boundary |
|---|---|---|
| ER1-01 | PASS | Shared checkout and every listed navigation worktree inspected. |
| ER1-02 | PASS | Dirty shared checkout, dirty path-results doc, engine working-tree changes and active import process preserved. |
| ER1-03 | PASS | Accepted DP-02 commit and parent verified against refreshed `origin/main`. |
| ER1-04 | PASS | Unique ER-01 worktree created from exact DP-02 commit; initially clean. |
| ER1-05 | PASS | UE 5.8.3 revision verified; inspected engine-source paths are clean. |
| ER1-06 | PASS | Required owning/source/governance documents and current navigation status reviewed before recording this result. |
| ER1-07 | PASS | Production validator order and query settings confirmed in isolated source. |
| ER1-08 | PASS | FindPathSync, Recast, FPImplRecast and Detour construction/return flow inspected. |
| ER1-09 | PASS | Shared-pointer validity, path-object validity and partial semantics distinguished. |
| ER1-10 | PASS | Feasibility classified `SOURCE_UNRESOLVED`; unchecked string-pull failure remains unproven. |
| ER1-11 | SKIPPED | The four ordinary static-nav queries below ran, but no supported geometry/query setup was identified for the unchecked corridor-extraction failure itself. |
| ER1-12 | INCONCLUSIVE | No actual new successful zero-point query result was observed. |
| ER1-13 | NOT_RUN | No qualifying typed Move or endpoint-projection trace. |
| ER1-14 | NOT_RUN | No ER-01 typed Move admission. |
| ER1-15 | PASS | Four direct synchronous `FindPathSync` calls ran against the genuine saved static Recast NavData with production owner/settings; they were not routed through POLITIKA's production order resolver. |
| ER1-16 | PASS | All four observed `Success` (3). |
| ER1-17 | PASS | All four observed a valid shared path pointer. |
| ER1-18 | PASS | All four observed non-partial results. |
| ER1-19 | PASS | Observed point counts: 1, 2, 2, 2; zero-point result not reproduced. |
| ER1-20 | NOT_RUN | Production `EmptyPath` was not reached. |
| ER1-21 | NOT_RUN | No ER-01 route-state outcome. |
| ER1-22 | NOT_RUN | No ER-01 route identity/cursor evidence. |
| ER1-23 | NOT_RUN | No ER-01 failure-event count. |
| ER1-24 | NOT_RUN | No ER-01 arrival observation. |
| ER1-25 | NOT_RUN | No 20-step ER-01 stationary trace. |
| ER1-26 | NOT_RUN | No ER-01 post-failure operational-state check. |
| ER1-27 | NOT_RUN | No fresh packaged ER-01 processes. |
| ER1-28 | NOT_RUN | No ER-01 independent-run comparison. |
| ER1-29 | PASS | Focused Development Editor query-observer Automation discovered and executed once; this is not packaged ER-01 acceptance. |
| ER1-30 | NOT_RUN | No new RTS/content regression run. |
| ER1-31 | NOT_RUN | Historical controls were not rerun against a new ER-01 source/package identity. |
| ER1-32 | NOT_RUN | No new complete-route positive control. |
| ER1-33 | NOT_RUN | No new MainMenu startup control. |
| ER1-34 | PASS | Linux UE Editor Development build succeeded; no Game target build. |
| ER1-35 | NOT_RUN | Core, audit, Rust tests and format were not run; `git diff --check` passed. |
| ER1-36 | NOT_RUN | No normal/fixture UAT package. |
| ER1-37 | NOT_RUN | No normal-cook fixture exclusion check. |
| ER1-38 | NOT_RUN | No fixture-cook/NavMesh check. |
| ER1-39 | PASS | Source, engine and pre-existing package identity limitations recorded; no new runtime/package identity claimed. |
| ER1-40 | PASS | Canonical assets and protected work were not changed by this investigation. |
| ER1-41 | PASS | ER-01, ER-02 and overall NAV-A15 verdicts remain accurate and unpromoted. |
| ER1-42 | SKIPPED | No runtime-qualified implementation commit was created. |
| ER1-43 | SKIPPED | No outgoing integration or remote publication attempted. |
| ER1-44 | SKIPPED | No GitHub/local-main synchronization claimed. |

These verdicts do not promote NAV-A15 or the Stage 4A milestone.
