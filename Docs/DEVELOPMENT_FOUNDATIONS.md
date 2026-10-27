# POLITIKA — Lightweight Development Foundations and Conditional Adoption

**Status (27 October 2026):** Rust content/metadata checks, unit tests, local Unreal toolchain preflight, and diagnostic JSON reporting have initial implementations. No Unreal project, runnable gameplay build, packaged playtest or runtime Automation tests are present in the repository.

**Purpose:** Adapt useful development infrastructure to a five-faction, offline-first RTS without importing an MMORPG's character, account, persistent-world or network architecture. Unreal Engine 5.8.3 and C++ own runtime simulation; Rust owns local validation and tooling. C# is optional for a specifically justified external application. Do not add scripting-language runtime dependencies, hosted GitHub workflows or unnecessary feature branches.

## 1. What is implemented now

| Candidate | Implemented source | Present capability | Important limit |
| --- | --- | --- | --- |
| **1 — Lightweight regression tests** | [Rust quality tests](../tools/version/src/bin/quality.rs), [version tests](../tools/version/src/lib.rs), [shared C++ rules](../Source/PolitikaCore/Rules.h) and [C++ regression cases](../Tests/Core/RulesTests.cpp) | Cargo tests validate metadata and diagnostic rules; a Rust command compiles and runs portable C++20 assertions for selection, move admission, resource spending and faction permissions | The C++ kernel is a **limited command/rules contract**, not navigation, the full game simulation, or Unreal integration |
| **2 — Data-driven content validation** | [Content manifest](../Data/content_manifest.json), quality `audit` | Verifies five faction IDs/names, exactly four playable factions plus the NPC authority, referenced descriptions/logos/banners, manifest schema and canonical version | The registry is a **design-stage asset index**; it does not implement faction combat rules, map loading, asset cooking or release readiness |
| **3 — Reproducible engine/build preflight** | [Unreal toolchain pin](../tools/unreal-version.env), [local Unreal launcher](../tools/unreal.sh), quality `preflight` | Pins UE version/tag/source revision and expected project location; inspects installed engine's `Build.version`, Git SHA and project path; optionally invokes Linux Editor build and future Automation invocation | `doctor` returns **BLOCKED** until the Unreal project and verified local UE installation are available; no compiler/build has been certified |
| **4 — Structured diagnostics** | Rust quality `audit`, `preflight` and `all` | Reports machine-readable check ID, status, message, game version, Git revision, dirty marker and status counts | Source checks do not imply playability or any target-device acceptance |

The canonical [`version.json`](../version.json) and the independent compatibility counters remain untouched. The quality tool is a second **binary in the existing Rust package**, not a new runtime service or a new scripting stack. The separate C++20 core test runner uses the host C++ compiler (configured with `CXX`) and requires no Unreal installation. The Cargo package keeps the original version CLI as its default binary.

## 2. Local commands

Run from the repository root with a Rust 2024-compatible toolchain (Rust 1.85 or newer):

```bash
cargo fmt --manifest-path tools/version/Cargo.toml -- --check
cargo test --locked --manifest-path tools/version/Cargo.toml --all-targets
cargo run --locked --quiet --manifest-path tools/version/Cargo.toml --bin quality -- audit
cargo run --locked --quiet --manifest-path tools/version/Cargo.toml --bin quality -- core
cargo run --locked --quiet --manifest-path tools/version/Cargo.toml --bin quality -- all
bash tools/unreal.sh doctor
```

The audit writes `build/diagnostics/audit.json`; C++ core tests write `build/diagnostics/core.json`; preflight writes `build/diagnostics/preflight.json`; `all` writes `build/diagnostics/all.json`. Reports are generated locally under ignored `build/`, not committed. No hosted CI triggers are configured.

Quality exits: `0` = all requested checks passed; `1` = test/contract failure; `2` = tool or infrastructure error; `3` = blocked by missing prerequisites; `4` = inconclusive or no completed assertion-based result. The check vocabulary additionally supports `SKIPPED` and `NOT_RUN`. A missing Editor build, playable map or recorded game must never be reported as a passed engine test.

The source pin specifies UE **5.8.3** with an explicit expected Git revision. When a local Epic source checkout is installed, set `UNREAL_ENGINE_DIR` and run `bash tools/unreal.sh doctor`; the installed checkout must match the pin. The expected project location is `UnrealPolitika/Politika.uproject`; set `POLITIKA_UPROJECT` only if its eventual real location differs. The source pin and local hardware must be independently verified before claiming a working Unreal build.

`bash tools/unreal.sh build` will attempt the Linux `PolitikaEditor` Development build only after preflight succeeds. `bash tools/unreal.sh test` is a **future hook** for Unreal Automation tests named `Politika.*`; until actual assertions and a checked report are present, its successful process exit is explicitly `INCONCLUSIVE`, not a claimed game-test pass. Windows tooling can invoke Cargo directly and later adopt native Unreal BuildTool commands without duplicating version logic.

## 3. Staged adoption

### Stage A — immediately useful, no engine installation

Maintain the five-faction content registry as design metadata, not a duplicate gameplay ruleset. Extend its references only when real files appear; reject dangling assets, duplicate IDs and invalid roles. Run `cargo test`, `quality audit` and `quality core` after changing faction metadata, versioning, command-admission rules, local tooling or manifest formats. The portable C++ rules header is the future Unreal simulation's shared admission kernel, rather than a competing Rust simulator. Keep historical/archive research separate from release-grade assets. Record the exact source revision and checks performed; no gameplay evidence is required for prose-only commits.

### Stage B — when a playable Unreal prototype exists

Integrate the existing platform-neutral [C++ rules kernel](../Source/PolitikaCore/Rules.h) into a `Politika` Unreal C++ runtime module; add Unreal Automation tests with genuine executable assertions. Prioritize: unit order admission and ownership; multi-selection and command queue; terrain/path reachability; production/building costs; faction permissions; espionage/sabotage timing and access gates; exclusive NPC world authority; and scenario/victory edge cases. Test at both a pure C++ rules layer and an Unreal integration layer, using one authoritative gameplay implementation rather than parallel Rust/C++ simulations. Introduce deterministic fixture maps and stable unit identifiers only with implemented assets.

At this stage, connect the generated C++ build identity to startup diagnostics and use Editor/Data Validation for cooked content. Require passing build and assertions before promoting a playable claim. Linux is the first development execution lane; qualify Windows, SteamOS/Steam Deck and Android in separate platform-specific runs.

### Stage C — only after real play and rendering exist

Add fixed RTS camera viewpoints and controlled scenarios: player spawn, selection, long-distance order, mixed-unit formations, fog transitions, base placement, a faction-specific action, and a reproducible victory/failure. Capture game frames, UI, events, performance counters and game-state assertions into one test manifest. Video or clean exit alone is **INCONCLUSIVE**; a pass requires the actual simulation and presentation assertions.

Profile performance by active unit count, path requests, line-of-sight/fog updates, AI decision frequency, graphics frame time and memory. Increase instrumentation only when a measured bottleneck or recurring failure justifies it; use Unreal Insights, Visual Logger, Gameplay Debugger, platform profilers or frame captures as appropriate. Keep heavy certification separate from ordinary documentation commits.

## 4. Explicitly out of scope

Do **not** transfer persistent MMO accounts, login gates, character stores, skill/progression inventories, persistent city-scale server state, custom account-server protocols, per-character pathing/control systems, citywide World Partition assumptions, or a multiplayer-first architecture. POLITIKA begins as an offline-first RTS and adds online transport, replays, multiplayer synchronization or dedicated servers only if its own game design and validated prototypes require them.

Do not copy incompatible engine scripts, historical game assets or engine-specific resource importers. Preserve license/provenance checks, but prefer POLITIKA-native data assets and Unreal interfaces. Avoid a second hand-maintained game version, a second simulation authority, a general-purpose telemetry server, automatically triggered heavy builds, and unnecessary branches.

## 5. Adoption decision rules

Before transferring any further development capability, require (1) a concrete RTS problem; (2) an existing POLITIKA subsystem that should own it; (3) a small, independently testable Rust/C++ implementation; (4) no additional runtime/format duplication; and (5) proof on an actual Unreal package before calling it operational. If a candidate does not meet those criteria, retain it as a conditional design idea only.

## 6. Further documentation practices worth adapting

The following practices are **conditional production recommendations**, not claims that the corresponding gameplay, content importers or quality gates already exist. Existing POLITIKA documents remain authoritative: [Engine Outline](ENGINE_OUTLINE.md), [Proposed Engine Layers](PROPOSED_ENGINE_LAYERS.md), [UI Overview](POLITIKA_UI_OVERVIEW.md), [Story Outline](STORY_OUTLINE.md), [Platform Support](PLATFORM_SUPPORT.md), and [Archive Rights](../Archive/RIGHTS_AND_PROVENANCE.md). Extend those owners at implementation time rather than generating a parallel roadmap for each recommendation.

| Reusable documentation pattern | RTS-specific adaptation | Priority / existing coverage |
| --- | --- | --- |
| **Evidence-based documentation and change scope** | Distinguish *proposed*, *implemented*, *unit-tested*, *Unreal-tested*, *packaged-tested*, and *platform-qualified*. Match checks to changes: prose and historical research require source/reference checks; gameplay rules require C++ regression tests; content definitions require Rust validation; live map edits require targeted navigation/render assertions. Never claim playable validation from a successful documentation or tooling check. | **Now; low effort.** Extends this guide, [AGENTS.md](../AGENTS.md) and the current Rust diagnostic tool. No hosted workflows. |
| **One owner for each roadmap decision** | Keep narrative in the story/concepts documents, RTS rules in the engine/layer documents, historical research in the archive and release criteria in version/platform documents. Source code and test evidence override optimistic progress descriptions; each handoff records changed files, source SHA, verified checks and blockers. | **Now; low.** Existing owners; avoid adding a second master roadmap. |
| **Historical-source, period ruling and game-world separation** | Differentiate archival evidence for Yugoslavia in **1980–1989**, a reviewed interpretation of historical conditions, and deliberately fictional alternate-history territorial/political changes. Preserve whether a border, administrative label, industrial facility or road derives from a period source or fictional design. A modern map cannot silently overwrite the era's world state. | **Now; low.** [Archive](../Archive/README.md) already covers provenance; add source-to-asset references when map authoring starts. |
| **Source-change review and reproducible map products** | For a political/physical map, record source identity, date, rights, usable resolution/scale, coordinate system where relevant, and any edited regions. Track deltas affecting campaign borders, objectives and locations. Use small deterministic Rust preprocessors only when real map inputs and engine importers exist. Avoid a full city-scale surveying pipeline. | **Conditional; low–medium.** See [map catalogue](../Archive/MAP_CATALOG.csv) and [map history](../Archive/MAPS_1980_1989.md). |
| **Priority tiers for 3D models and environmental art** | Protect the silhouette and readability of faction-signature units, headquarters, strategic installations and mission-critical bridges first; favor reusable models and materials for common units and infrastructure; keep background dressing cheap. Review selection indicators, team identification, damage and attack-range visibility *from tactical camera zoom*, not only model close-ups. | **First prototype; low.** Add a compact asset review checklist to existing UI/engine plans only when models exist. |
| **A restrained material/texture contract** | Standardize mesh scale/pivots, shared Unreal master materials and instances, texture roles, sRGB base color versus linear mask/normal data, licensing and provenance. Generated Unreal assets should remain derivatives of traceable inputs; avoid separate hand-maintained copies of asset truth. | **First assets; low–medium.** Extends engine content rules and archive rights policy. |
| **Historically grounded visual direction** | Let late-socialist architecture, industrial facilities, period road/rail equipment, broadcast media, original faction heraldry and recognizable maps communicate the era. Avoid permanent CRT/VHS effects that reduce real RTS command readability. Use fixed overhead and zoomed-out battlefield captures in varied lighting to verify both atmosphere and orders/UI. | **Art milestone; low.** Fold a visual acceptance rubric into the first map review. |
| **An event-driven audio catalogue** | Catalog unit acknowledgments, weapon and engine sounds, warnings, economy feedback, infiltration cues, political broadcasts and faction music. Separate UI, SFX, ambience, music and voice/broadcast routing, ensuring announcements never mask critical tactical warnings. Only introduce a Rust-validated audio asset manifest once sounds are commissioned. | **First audible prototype; low–medium.** Engine/UI cover audio generally, not its concrete event registry. |
| **Mission outcomes as data rather than UI state** | Encode scenario-objective IDs, prerequisites, success/partial success/failure, and tactical outcome events in C++-authoritative state. Campaign briefings and results screens consume those outcomes. Reuse the *approach* to branching/checkpointing, not RPG quest journals, individual progression or perpetual tasks. | **First campaign scenario; medium.** Already designed in story/concepts; schema and tests still needed. |
| **First-match teaching and whole-device acceptance** | Teach select → command → production → reconnaissance → verify intelligence → defend/retreat inside a small skirmish, with contextual prompts and repeatable help. Verify full RTS operation with mouse/keyboard, gamepad/Steam Deck and touch, including 4K and compact HUD scales. Convey important information with more than color or sound. | **First playable; medium.** UI and platform plans already define intended support; add a concrete task matrix then. |
| **Performance budgets based on actual RTS stressors** | Benchmark stable scenarios by unit count, nav/path requests, formation movement, fog of war, AI decisions, simulation time, visual frame time and memory at fixed camera/zoom/map states. Inspect cold-load shader hitches separately. Consider instancing, HLOD or World Partition only when measurements warrant their complexity. | **Playable battle; medium.** Architecture acknowledges optimization; no measured benchmark exists. |
| **Assertion-driven scenario evidence** | Define each automated skirmish with map, factions, random seed, initial resources, player orders, expected authoritative state/events, timeout, result and source identity. Camera captures and recordings are diagnostic supplements; passing requires game-state assertions, not merely a file or normal process exit. | **Playable prototype; medium.** Builds on current Rust diagnostic verdicts. |
| **Scope lock and release promotion gates** | Progress through an engine/rules prototype, one playable representative faction/match, complete faction asymmetry and NPC authority, first campaign mission, then portable builds. For a release candidate verify clean installation, basic victory/defeat, controls, save/replay compatibility if implemented, crash recovery, accessibility and measured performance separately by platform. | **Plan now; certify later.** Release/version policy already owns gating. |

### 6.1 Lowest-friction actions at the current stage

Prioritize the first three rows immediately as **documentation discipline**. When adding new historical maps, provide an archive identifier and separate the source's actual historical claim from the game's fictional use. When adding units or buildings, select importance/readability tiers before investing in textures. When adding faction data, extend the existing Rust validator rather than adding parallel authoring stores. When changing game rules, add native C++ tests; only a real Unreal executable can verify engine integration.

The first real battle/map should become a *representative vertical slice* for validating commands, fog of war, navigation, combat, buildings, neutral authority and tactical readability. Production-scale asset import, audiovisual capture, deep profiling and full release checklists should be triggered by that evidence, not by the mere existence of roadmap text.

### 6.2 Things not worth transferring

Do not borrow persistent character accounts, MMO inventories, social identity/progression services, large online-shard assumptions, pedestrian/individual-player traversal benchmarks, full-city geospatial reconstruction, historical-calendar mirroring, or always-on world simulation. POLITIKA needs map-scale strategic geography, unit/match authority, and deliberately designed faction asymmetry instead. Do not copy incompatible engine integration, proprietary reference assets or speculative performance targets.

**Tool boundary:** Rust owns local manifests, deterministic source transforms and diagnostics; C++ owns RTS rules and the authoritative simulation; Unreal Engine 5.8.3 owns assets, UI, Editor automation and packaging; C# remains optional for a justified standalone tool. No extra hosted workflows, scripting-language dependencies, branch proliferation, gameplay subsystem duplication or unverified release claims arise from these recommendations.


Related: [Engine Outline](ENGINE_OUTLINE.md), [Proposed Engine Layers](PROPOSED_ENGINE_LAYERS.md), [Versioning Roadmap](VERSIONING_RELEASE_MANAGEMENT_ROADMAP.md), [Platform Support](PLATFORM_SUPPORT.md), and [Content Manifest](../Data/content_manifest.json).
