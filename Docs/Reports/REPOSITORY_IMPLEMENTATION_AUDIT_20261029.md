# POLITIKA — Repository implementation and validation audit (29 October 2026)

**Scope and identity.** Read-only audit of GitHub `simon-gros/politika` default branch `main` at `da23ae5db74fc8ca2079d501db5ea4b06ff63385`, followed by a narrowly scoped offline-test entry-point and documentation-consistency change. The GitHub recursive tree response was complete (`truncated=false`); inventory below is a *tracked source tree* count, not an inventory of generated packages or external engine files. Historical acceptance records are evidence about their own source/build/package identities, **not** tests rerun for this audit.

## 1. Observed repository inventory

| Area | Observed tracked source at baseline | Assessment |
| --- | --- | --- |
| Source tree | 483 tracked blobs; 25 `.cpp`, 13 `.h`, 16 `.rs`; no Python source files | Unreal/C++ runtime and Rust offline harnesses; no Python runtime dependency |
| Unreal project | `Politika.uproject`; `Source/Politika`, `Source/PolitikaBootstrap`, `Source/PolitikaEditor`, `Source/PolitikaCore`; seven `.umap` and 18 `.uasset` files | Source and authored assets exist; packaging cannot be inferred from Git tree alone |
| Core/automation tests | `Tests/Core/RulesTests.cpp`; six `Source/Politika/Private/Tests/*.cpp`; four `Source/PolitikaEditor/Private/Tests/*.cpp` | Real native test bodies exist; no fresh UE or C++ suite execution in this audit |
| Offline tools | `tools/version` quality/version package; `Tools/Shell` package and runtime harnesses; `Tools/InfantryAuthoring`; `Archive/tools` Rust archive validator | Four independent Cargo manifests and locks; deliberately local/manual rather than hosted CI |
| Authored/reference inputs | Historical catalogue/indexes, five faction descriptions/rosters, cartographic research, maps, audio/terrain/third-party art manifests, one static infantry GLB | Research and source assets are distinct from production-ready, imported/cooked gameplay assets |
| Version and toolchain | `version.json`: `0.1.0-dev.0`; `tools/unreal-version.env`: UE 5.8.3, pinned source `396c9f059903aed5fec78ecd3d437a40c6415368` | No version bump or engine change warranted by this tooling audit |

## 2. Implemented systems versus acceptance evidence

**Native shell.** `Source/Politika/Private/PolitikaGameInstance.cpp`, the shell controller/profile/settings modules, bootstrap module, and shell/game-mode code provide the application foundation. `Docs/SHELL_SKELETON.md` contains historical Linux runtime evidence. The shell is not a complete RTS by itself.

**First playable tactical loop.** `Source/Politika/Private/PolitikaPrototypeGameplay.cpp`, `Source/PolitikaCore/Rules.h`, and `Tests/Core/RulesTests.cpp` implement the bounded fixed-step prototype with unit selection, typed move/attack/attack-move/stop/guard orders, combat, damage/death, a control point, and feedback. `Docs/RTS_VERTICAL_SLICE_MILESTONE.md` and `Docs/RTS_COMMAND_SIMULATION_CONTRACT.md` describe scoped *previously executed* Linux Development/UI/command acceptance. This does not establish campaign, general victory/defeat, formations, fog, AI, or economy.

**Authored content and map admission.** `PolitikaPrototypeContent.cpp`, typed data assets, `PolitikaCreateShellCommandlet.cpp`, and `Docs/STAGE3_DATA_CONTENT_MAP_ACCEPTANCE.md` show validated authored definitions, meaningful diagnostics, failed-admission rollback, and historical packaged rejection/recovery evidence. Stage 3 is accepted only for its stated Linux/fixture scope.

**Stage 4A lifecycle and participant authority.** `Docs/STAGE4A_MATCH_LIFECYCLE_ACCEPTANCE.md` qualifies objective-driven exactly-once Terminal/Results, command shutdown, return, fresh retry, and malformed participant/faction startup recovery in identified Linux Development packages. A two-slot authored roster plus separate Unaligned system authority exists; the latter is **not autonomous Unaligned AI**. `PA-08`, `PA-10` and `PA-11` are still PARTIAL (stale/dead selection and real controller/opposing or NPC command-authority cases). The broader Stage 4A contract remains incomplete.

**Navigation.** `PolitikaNavigationAcceptance.cpp`, `PolitikaNavigationFailureAcceptance.cpp`, the coordinator route executor, Editor navigation tests, and `Tools/Shell/src/navigation_obstacle.rs` implement/observe authoritative Recast route following under the 0.05-second coordinator. `Docs/RTS_RUNTIME_NAVIGATION_ACCEPTANCE.md` documents historical cooked static-obstacle/repeatability and Stop/replacement cases (`NAV-A25`, `NAV-A20`, `NAV-A12/13`). Zero-point successful-path and some invalid/partial-route cases remain unqualified: `ER-01`, `ER-02`, overall `NAV-A15` = **INCONCLUSIVE**. Do not create fabricated routes or treat subcase tests as the overall gate.

**Art.** The `AC-INF-01` GLB is a documented **static** textured playtest candidate, without skeleton, skin weights, animation, approved UE import, or cooked player-facing acceptance. Third-party art sources have provenance files but are not automatically validated or release-cleared by presence in Git. Historical source URLs do not establish redistribution rights.

**Platforms, release, persistence.** The repository plans Linux, Windows, SteamOS/Deck, and Android; older Linux Development packages are scoped evidence only. Release, other-platform builds, Shipping rendering, device performance, campaign saves and complete single-player skirmishes are not qualified by this audit.

## 3. Concrete risks and engineering priorities

| Priority | Observed gap or risk | Smallest evidence-producing next action |
| --- | --- | --- |
| P0 — validation availability | `quality core` is invoked through Cargo even though its underlying regression code is standalone C++20. This audit's container has `g++` but no Cargo, Unreal source tree, or checkout; GitHub access is connector-only, and direct clone/download DNS fails. | Add and syntactically/synthetically exercise `tools/core-test.sh` as a **standalone fallback**; run it on the real checkout as soon as that checkout is available. Keep Cargo JSON quality reporting separate. |
| P1 — controller/selection authority | `SetSelection` guards owner and alive input; `GetSelectedUnitForPlayer` currently returns the stored weak pointer without an explicit alive check at read time; `PA-08` remains partial. `PA-10/11` lack complete local-controller and Unaligned packaged replay. | Add isolated dead/stale-selection and identity-spoof assertions in the existing match-participant tests, then UE Automation and cooked-controller interactions; avoid treating static inspection as bug reproduction. |
| P1 — navigation failure behavior | Complete routes/interruptions have evidence, but `NAV-A15` failure subcases remain inconclusive. An ordinary successful path with one point is **not** proof of the required empty-path case. | Capture a genuine supported Recast invalid/partial/unreachable query through typed admission/Tick with exact failure event and no fallback, only after a reproducible fixture is identified. Keep `ER-01` inconclusive absent a matching engine result. |
| P1 — match breadth | Non-human participant role is not a decision-making opponent; economy, deployment/production, fog/observation and repeatable victory *and defeat* not implemented as a single supported loop. | Finish bounded 4A authority acceptance before selecting one first-skirmish content/economy/AI slice under `Docs/ENGINE_OUTLINE.md`; avoid new parallel gameplay authority. |
| P2 — asset qualification | `AC-INF-01` is static-only; meshes/skeletons, native import/cook, team readability and player-input collision are not qualified. | Controlled Editor import from existing source art; verify trace/collision, RTS zoom, package cook, and animation prerequisites separately. |
| P2 — platform/release metrics | No demonstrated Windows/SteamOS/Android/Shipping-rendering parity or representative scale benchmark for the current game state. | Build isolated per-platform/dev-device evidence after a complete skirmish; preserve `BLOCKED/INCONCLUSIVE` distinctions and provenance. |
| P2 — documentation freshness | Some `ENGINE_OUTLINE.md` Stage 4A paragraphs still called cooked participant/faction rejection and same-process Active retry unqualified despite later explicit packaged recovery in its owning acceptance record. | Reconcile those status summaries to the bounded packaged proof without altering historical test logs or promoting overall Stage 4A. |

## 4. Work performed in this constrained environment

1. **Standalone native core gate:** added `tools/core-test.sh`. Uses `CXX` (default `c++`), compiles `Tests/Core/RulesTests.cpp` against `Source/PolitikaCore/Rules.h` with `-std=c++20 -Wall -Wextra -Werror -pedantic`, runs the binary, and removes only its own temporary directory. No Cargo/UE/network/third-party dependencies. A missing checkout/compiler returns `BLOCKED` rather than PASS.
2. **Local script checks actually run:** `bash -n` PASS; a **synthetic two-file C++20 fixture** compiled and ran through the exact wrapper with system `c++` PASS; deliberately nonexistent `CXX` returned exit 3 and a BLOCKED diagnostic. **These do not count as the real POLITIKA core suite.**
3. **Documentation reconciliation:** corrected stale roadmap qualification wording and documented the portable fallback in the owning development/governance files.
4. **No engine or validator execution:** this sandbox has no Git working tree, Cargo/Rust toolchain, or UE 5.8.3 checkout. No Unreal Editor/game build, package, Automation, current-core C++ suite, Cargo, or compiled-game tests were run. Existing 2026-10-27/29 reports are *historical*, not refreshed acceptance.
5. **No unrelated changes:** preserved `version.json`, Unreal assets, gameplay sources, faction/game canon, build toolchain pin, and existing historical records. No hosted workflows introduced, consistent with `Docs/DEVELOPMENT_FOUNDATIONS.md`.

## 5. Reproduction and next qualified handoff

In a checked-out repository with a C++20 compiler:

```bash
bash tools/core-test.sh
# Optional when Cargo is installed: canonical structured core report
cargo run --locked --quiet --manifest-path tools/version/Cargo.toml --bin quality -- core
# Separate offline archive validation
cargo test --locked --manifest-path Archive/tools/Cargo.toml
cargo run --locked --quiet --manifest-path Archive/tools/Cargo.toml
```

For full engine work, first verify `UNREAL_ENGINE_DIR` and the exact UE pin with `bash tools/unreal.sh doctor`, then run the affected Unreal Automation and packaged-runtime acceptance gates. Treat future release/platform/art qualifications as separate evidence; do not inherit these verdicts from documentation or tool installation.

**References:** [Engine implementation order](../ENGINE_OUTLINE.md), [Stage 4A acceptance](../STAGE4A_MATCH_LIFECYCLE_ACCEPTANCE.md), [Runtime navigation acceptance](../RTS_RUNTIME_NAVIGATION_ACCEPTANCE.md), [Stage 3 acceptance](../STAGE3_DATA_CONTENT_MAP_ACCEPTANCE.md), [Validation policy](../VALIDATION_AND_GOVERNANCE.md), [Infantry prototype](../../Art/Prototypes/Infantry/AC-INF-01/README.md).
