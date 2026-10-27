# POLITIKA — Lightweight Development Foundations and Conditional Adoption

**Status (27 October 2026):** Rust content/metadata checks, unit tests, local Unreal toolchain preflight, and diagnostic JSON reporting have initial implementations. No Unreal project, runnable gameplay build, packaged playtest or runtime Automation tests are present in the repository.

**Purpose:** Adapt useful development infrastructure to a five-faction, offline-first RTS without importing an MMORPG's character, account, persistent-world or network architecture. Unreal Engine 5.8.3 and C++ own runtime simulation; Rust owns local validation and tooling. C# is optional for a specifically justified external application. Do not add scripting-language runtime dependencies, hosted GitHub workflows or unnecessary feature branches.

## 1. What is implemented now

| Candidate | Implemented source | Present capability | Important limit |
| --- | --- | --- | --- |
| **1 — Lightweight regression tests** | [Rust quality tests](../tools/version/src/bin/quality.rs) alongside [version-contract tests](../tools/version/src/lib.rs) | Cargo regression tests exercise faction invariants, unsafe paths, duplicate IDs, toolchain parsing, and explicit diagnostic verdicts | These are source/contract tests, **not** C++ gameplay tests; no compiled Unreal project exists yet |
| **2 — Data-driven content validation** | [Content manifest](../Data/content_manifest.json), quality `audit` | Verifies five faction IDs/names, exactly four playable factions plus the NPC authority, referenced descriptions/logos/banners, manifest schema and canonical version | The registry is a **design-stage asset index**; it does not implement faction combat rules, map loading, asset cooking or release readiness |
| **3 — Reproducible engine/build preflight** | [Unreal toolchain pin](../tools/unreal-version.env), [local Unreal launcher](../tools/unreal.sh), quality `preflight` | Pins UE version/tag/source revision and expected project location; inspects installed engine's `Build.version`, Git SHA and project path; optionally invokes Linux Editor build and future Automation invocation | `doctor` returns **BLOCKED** until the Unreal project and verified local UE installation are available; no compiler/build has been certified |
| **4 — Structured diagnostics** | Rust quality `audit`, `preflight` and `all` | Reports machine-readable check ID, status, message, game version, Git revision, dirty marker and status counts | Source checks do not imply playability or any target-device acceptance |

The canonical [`version.json`](../version.json) and the independent compatibility counters remain untouched. The quality tool is a second **binary in the existing Rust package**, not a new runtime service or a new scripting stack. The Cargo package keeps the original version CLI as its default binary.

## 2. Local commands

Run from the repository root with a Rust 2024-compatible toolchain (Rust 1.85 or newer):

```bash
cargo fmt --manifest-path tools/version/Cargo.toml -- --check
cargo test --locked --manifest-path tools/version/Cargo.toml --all-targets
cargo run --locked --quiet --manifest-path tools/version/Cargo.toml --bin quality -- audit
cargo run --locked --quiet --manifest-path tools/version/Cargo.toml --bin quality -- all
bash tools/unreal.sh doctor
```

The audit writes `build/diagnostics/audit.json`; preflight writes `build/diagnostics/preflight.json`; `all` writes `build/diagnostics/all.json`. Reports are generated locally under ignored `build/`, not committed. No hosted CI triggers are configured.

Quality exits: `0` = all requested checks passed; `1` = test/contract failure; `2` = tool or infrastructure error; `3` = blocked by missing prerequisites; `4` = inconclusive or no completed assertion-based result. The check vocabulary additionally supports `SKIPPED` and `NOT_RUN`. A missing Editor build, playable map or recorded game must never be reported as a passed engine test.

The source pin specifies UE **5.8.3** with an explicit expected Git revision. When a local Epic source checkout is installed, set `UNREAL_ENGINE_DIR` and run `bash tools/unreal.sh doctor`; the installed checkout must match the pin. The expected project location is `UnrealPolitika/Politika.uproject`; set `POLITIKA_UPROJECT` only if its eventual real location differs. The source pin and local hardware must be independently verified before claiming a working Unreal build.

`bash tools/unreal.sh build` will attempt the Linux `PolitikaEditor` Development build only after preflight succeeds. `bash tools/unreal.sh test` is a **future hook** for Unreal Automation tests named `Politika.*`; until actual assertions and a checked report are present, its successful process exit is explicitly `INCONCLUSIVE`, not a claimed game-test pass. Windows tooling can invoke Cargo directly and later adopt native Unreal BuildTool commands without duplicating version logic.

## 3. Staged adoption

### Stage A — immediately useful, no engine installation

Maintain the five-faction content registry as design metadata, not a duplicate gameplay ruleset. Extend its references only when real files appear; reject dangling assets, duplicate IDs and invalid roles. Run `cargo test` and `quality audit` when changing faction metadata, versioning, local tooling or manifest formats. Keep historical/archive research separate from release-grade assets. Record the exact source revision and checks performed; no gameplay evidence is required for prose-only commits.

### Stage B — when a playable Unreal prototype exists

Add a small `Politika` C++ runtime module and Unreal Automation tests with genuine executable assertions. Prioritize: unit order admission and ownership; multi-selection and command queue; terrain/path reachability; production/building costs; faction permissions; espionage/sabotage timing and access gates; exclusive NPC world authority; and scenario/victory edge cases. Test at both a pure C++ rules layer and an Unreal integration layer, using one authoritative gameplay implementation rather than parallel Rust/C++ simulations. Introduce deterministic fixture maps and stable unit identifiers only with implemented assets.

At this stage, connect the generated C++ build identity to startup diagnostics and use Editor/Data Validation for cooked content. Require passing build and assertions before promoting a playable claim. Linux is the first development execution lane; qualify Windows, SteamOS/Steam Deck and Android in separate platform-specific runs.

### Stage C — only after real play and rendering exist

Add fixed RTS camera viewpoints and controlled scenarios: player spawn, selection, long-distance order, mixed-unit formations, fog transitions, base placement, a faction-specific action, and a reproducible victory/failure. Capture game frames, UI, events, performance counters and game-state assertions into one test manifest. Video or clean exit alone is **INCONCLUSIVE**; a pass requires the actual simulation and presentation assertions.

Profile performance by active unit count, path requests, line-of-sight/fog updates, AI decision frequency, graphics frame time and memory. Increase instrumentation only when a measured bottleneck or recurring failure justifies it; use Unreal Insights, Visual Logger, Gameplay Debugger, platform profilers or frame captures as appropriate. Keep heavy certification separate from ordinary documentation commits.

## 4. Explicitly out of scope

Do **not** transfer persistent MMO accounts, login gates, character stores, skill/progression inventories, persistent city-scale server state, custom account-server protocols, per-character pathing/control systems, citywide World Partition assumptions, or a multiplayer-first architecture. POLITIKA begins as an offline-first RTS and adds online transport, replays, multiplayer synchronization or dedicated servers only if its own game design and validated prototypes require them.

Do not copy incompatible engine scripts, historical game assets or engine-specific resource importers. Preserve license/provenance checks, but prefer POLITIKA-native data assets and Unreal interfaces. Avoid a second hand-maintained game version, a second simulation authority, a general-purpose telemetry server, automatically triggered heavy builds, and unnecessary branches.

## 5. Adoption decision rules

Before transferring any further development capability, require (1) a concrete RTS problem; (2) an existing POLITIKA subsystem that should own it; (3) a small, independently testable Rust/C++ implementation; (4) no additional runtime/format duplication; and (5) proof on an actual Unreal package before calling it operational. If a candidate does not meet those criteria, retain it as a conditional design idea only.

Related: [Engine Outline](ENGINE_OUTLINE.md), [Proposed Engine Layers](PROPOSED_ENGINE_LAYERS.md), [Versioning Roadmap](VERSIONING_RELEASE_MANAGEMENT_ROADMAP.md), [Platform Support](PLATFORM_SUPPORT.md), and [Content Manifest](../Data/content_manifest.json).
