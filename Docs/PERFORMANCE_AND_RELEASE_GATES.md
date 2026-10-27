# POLITIKA — RTS Performance and Release Acceptance

**Status:** Planning, benchmark case definitions and incomplete release evidence ledger. A local Linux UE 5.8.3 application-shell package is [verified](SHELL_SKELETON.md#verified-delivery--27-october-2026); no RTS simulator, representative gameplay benchmarks, platform-qualified release or release certificate exists.

## 1. Performance testing: measure the game that POLITIKA is building

The canonical [performance cases](../Data/performance_cases.json) name four currently **planned** fixtures. Their map IDs, seeds, unit counts and measurements intentionally remain unset. No FPS budget or battle-size capability can be inferred from the file's presence.

| Fixture | Gameplay workload | Primary question |
| --- | --- | --- |
| `small_skirmish` | First complete player-versus-AI tactical match | Is basic input, orders, simulation tick and rendering responsive? |
| `group_pathing` | Groups maneuver around mixed obstacles and bridges | Can path requests, collision/avoidance and formations complete without stalls or units becoming trapped? |
| `fog_and_ai` | Enemy intel changes, hidden units, NPC response and faction AI updating simultaneously | Are fog visibility, information truth and AI bounded under load? |
| `render_stress` | Busy army and base, VFX, smoke, structures and terrain at multiple zooms | Is frame time predictable and tactical information still visible? |

Future fixture contract: scene/map version; deterministic seed where supported; faction configuration; unit and building counts; camera position/zoom; graphics/scalability profile; target platform/build SHA; warm/cold shader/cache state; workload duration and simulated ticks; initial conditions; actual measured samples. Record p50/p95/p99 frame time, max hitch, simulation tick p95, command-to-accept latency, path/fog/AI cost and peak memory where instrumentation permits. Clearly state which metrics were not collected.

**Policy:** Measure the C++ simulation separately from Unreal presentation whenever possible. Large numbers of units stress shared navigation, visibility and event dispatch differently from a persistent-character world. Do not set arbitrary unit limits or assert a hardware target from theoretical poly counts. Set provisional budgets only after measuring at least one playable, reproducible map on each relevant device class.

## 2. Validation ladder and promotion criteria

1. **Level P0 — design metadata:** Rust registry/source/format checks, native C++ rules tests. Implemented and checked; makes no gameplay claim.
2. **Level P1 — engine smoke:** Verified UE pin, compiled Editor, selection and command loop, representative playable map and C++/Unreal Automation assertions. Partially complete: the compiled/packaged offline shell has process and UI evidence, but the RTS selection/command loop and playable map are not yet available.
3. **Level P2 — representative skirmish:** A complete match from launch to victory/failure; all essential unit/production/orders, NPC authorities and fog/espionage tested with deterministic fixtures. Not yet available.
4. **Level P3 — content and performance:** Four faction asymmetries, campaign objective variation, representative art/audio, navigation/fog regression, measured performance and reproducible original assets. Not yet available.
5. **Level P4 — release candidate:** Packaged install on supported operating systems/devices, clean-machine first run, accessibility/input coverage, no blocking defects, save/replay compatibility where implemented, stable release identity, rights clearance and a reviewed change log. Not yet available.

A skipped check does not become a pass. Release promotion must cite the evidence for the exact commit and package: test report, engine revision, content identity, platform, result and reviewer. The [release ledger](../Data/release_matrix.json) records the Linux shell engine build with evidence; **the other six gates remain blocked**. A gate may be marked `pass` only when its evidence list contains verifiable records from the actual running/build output; a narrative assertion is not evidence. The metadata-only release tool reports such assertions as `INCONCLUSIVE` until independently checked. A candidate is not releasable if any mandatory gate remains blocked or failed.

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

## 5. Low-cost next moves

Keep current Rust reports and regression tests honest, retain all four benchmark fixture definitions as *planned*, and do not add a monitoring server or a heavyweight benchmarking harness. Once a first skirmish runs, implement only `small_skirmish`, establish real frame/simulation budgets, then expand to the remaining fixtures if they expose measurable problems. Hosted build workflows remain absent by design.
