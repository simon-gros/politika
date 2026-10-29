# POLITIKA — RTS Performance and Release Acceptance

**Status (27 October 2026):** Benchmark case definitions and release evidence ledger are in progress. The Linux UE 5.8.3 application shell and a bounded RTS prototype have been built and exercised in packaged Development clients; see the [shell verification](SHELL_SKELETON.md#verified-delivery--27-october-2026) and [RTS milestone record](RTS_VERTICAL_SLICE_MILESTONE.md). There is not yet a complete skirmish, representative RTS performance benchmark, platform-qualified release, or release certificate.

## 1. Performance testing: measure the game that POLITIKA is building

The canonical [performance cases](../Data/performance_cases.json) name four currently **planned** fixtures. Their map IDs, seeds, unit counts and measurements intentionally remain unset. No FPS budget or battle-size capability can be inferred from the file's presence.

| Fixture | Gameplay workload | Primary question |
| --- | --- | --- |
| `small_skirmish` | First complete player-versus-AI tactical match | Is basic input, orders, simulation tick and rendering responsive? |
| `group_pathing` | Groups maneuver around mixed obstacles and bridges | Can path requests, collision/avoidance and formations complete without stalls or units becoming trapped? |
| `fog_and_ai` | Enemy intel changes, hidden units, NPC response and faction AI updating simultaneously | Are fog visibility, information truth and AI bounded under load? |
| `render_stress` | Busy army and base, VFX, smoke, structures and terrain at multiple zooms | Is frame time predictable and tactical information still visible? |

Future fixture contract: scene/map version; deterministic seed where supported; faction configuration; unit and building counts; camera position/zoom; graphics/scalability profile; target platform/build SHA; warm/cold shader/cache state; workload duration and simulated ticks; initial conditions; actual measured samples. Record p50/p95/p99 frame time, max hitch, simulation tick p95, command-to-accept latency, path/fog/AI cost and peak memory where instrumentation permits. Clearly state which metrics were not collected.

**Policy:** Measure the C++ simulation separately from Unreal presentation whenever possible. Large numbers of units stress shared navigation, visibility and event dispatch differently from a persistent-character world. Do not set arbitrary unit limits or assert a hardware target from theoretical poly counts. Set provisional budgets only after measuring at least one complete, reproducible skirmish on each relevant device class. The current prototype demonstrates interaction and rendering but is not a benchmark fixture.

## 2. Validation ladder and promotion criteria

1. **Level P0 — design metadata:** Rust registry/source/format checks, native C++ rules tests. Implemented and checked; makes no gameplay claim.
2. **Level P1 — engine and bounded gameplay slices:** Verified UE pin, compiled Editor, accepted content/map and command contracts, lifecycle foundation, scoped Unreal Automation, selection/command loop and rendered Linux Development interaction/recovery. **Partially complete at current source:** Stage 3 and the command/simulation contract have scoped acceptance; Stage 4A lifecycle and participant foundations have evidence, including 2026-10-28 Editor authored-startup rejection coverage. Stage 4A remains incomplete: successful canonical Active retry after malformed participant admission and cooked malformed-participant recovery remain unproven. Current-worktree test evidence is recorded in [Stage 4A](STAGE4A_MATCH_LIFECYCLE_ACCEPTANCE.md); do not infer current engine build availability from this roadmap.
3. **Level P2 — First Playable Skirmish:** One complete offline human-versus-AI match from normal shell/scenario selection through economy, production, combat, normal victory/defeat, result, menu return and fresh same-process rematch. Requires subsystem tests, content validation, runtime startup, rendered player controls and an end-to-end packaged Linux Development run. **Not yet available.** Do not require complete fog/espionage for the first gate unless the selected scenario's rules depend on them.
4. **Level P3 — expanded content and performance:** Additional faction asymmetries, campaign objective variation, representative art/audio, navigation/fog regression, measured performance and reproducible original assets. Not yet available.
5. **Level P4 — release candidate:** Packaged install on supported operating systems/devices, clean-machine first run, accessibility/input coverage, no blocking defects, save/replay compatibility where implemented, stable release identity, rights clearance and a reviewed change log. Not yet available.
A skipped check does not become a pass. Release promotion must cite the evidence for the exact commit and package: test report, engine revision, content identity, platform, result and reviewer. The [release ledger](../Data/release_matrix.json) records the Linux shell and bounded RTS prototype package evidence; **the other six gates remain blocked**. A gate may be marked `pass` only when its evidence list contains verifiable records from the actual running/build output; a narrative assertion is not evidence. The metadata-only release tool reports such assertions as `INCONCLUSIVE` until independently checked. A candidate is not releasable if any mandatory gate remains blocked or failed.

The public game version follows [Versioning and Release Management](VERSIONING_RELEASE_MANAGEMENT_ROADMAP.md), not a numerical count of completed checkboxes. Documentation, archive research and tooling alone never warrant calling a release playable.

## 3. Platform-specific quality contract

| Platform | Test intent once packaged | Qualification boundary |
| --- | --- | --- |
| Linux | Native launch, graphics driver/Vulkan compatibility, complete mouse/keyboard and controller loop, local save and quit | Verify on real supported distributions/GPU drivers |
| Windows | Native executable, varied DPI/resolution/fullscreen, input devices, save/relaunch and GPU driver behavior | Verify on real Windows hardware |
| SteamOS / Steam Deck | Gamepad/Steam Input semantics, legible tactical map/HUD, suspend/resume, power/thermal frame pacing | Verify actual handheld hardware; never imply official Verified status |
| Android | Touch-select/command accuracy, dynamic resolution, memory/thermal stability, background/foreground continuity, packaging | Verify actual Android OS/device classes and mobile rendering support |

Keep one C++ match/AI/economy logic across target builds. Platform-specific rendering budgets, user interface density and input gesture mappings may differ; actual gameplay outcomes must not diverge accidentally.

## 4. Release evidence packet (eventual)

For each release candidate retain:
- canonical `game_version`, compatibility counters and Git commit;
- exact UE source revision and verified packaging configuration;
- content/map/audio manifests, source hashes and IP/license review;
- focused C++/Rust validation and relevant Unreal Automation/playtest results;
- reproducible skirmish assertion reports and known defect classifications;
- performance data with hardware, settings, build and sample methodology;
- platform-by-platform launch/input/UX/accessibility/save/quit and clean-install results;
- risk sign-off, release notes, rollback/fix-forward decision and owner approval.

Do not declare an artifact shipped because it compiled. Runtime package hashes and device testing may only be added when real packages exist.

### Rights and evidence conditions before distributing any build

A successful cook or packaged gameplay test does **not** establish copyright permission. Audit software, original POLITIKA media, CC0/attributed third-party assets, reference-only materials and extracted proprietary game media separately. The [rifle-infantry reference import](CNC3_RIFLEMEN_IMPORT.md) documents model and audio conversions from another game; neutral naming and re-encoding alone do not clear public redistribution. Before public packages, record explicit rights/permission, replacement with original or cleared material, or verified exclusion from distributable assets (including cooked output, source-control exports and sample media). Project-wide software/artwork license terms require the rights holder's decision; no license is selected by these rules.

For every candidate PASS reconcile [release_matrix.json](../Data/release_matrix.json) with the exact Git SHA/dirty state, engine revision, content and package hashes, platform/device, fixture, test command, date and linked report. A prior package PASS remains *historical evidence*, never a current-build verdict. Document rights blockers and NOT_RUN device/packaged checks separately from failed tests; follow [Validation and Governance](VALIDATION_AND_GOVERNANCE.md) for status definitions.

## 5. Low-cost next moves

Keep current Rust reports and regression tests honest, retain all four benchmark fixture definitions as *planned*, and do not add a monitoring server or a heavyweight benchmarking harness. Immediate product work remains the Stage 4A admission/recovery gaps, followed by the bounded economy/production, controls, opponent and result/rematch increments in the [Engine Outline](ENGINE_OUTLINE.md). Once the first skirmish runs, implement only `small_skirmish`, establish real frame/simulation budgets, then expand to the remaining fixtures if they expose measurable problems. Hosted build workflows remain absent by design.
