# First RTS Vertical Slice — Milestone Record

**Status:** complete for the scoped Linux Development milestone; other platforms and physical-device qualification remain out of scope
**Target:** Unreal Engine 5.8.3, initial qualification on the locally supported Linux Development build
**Purpose:** move POLITIKA from an application shell to its first small, directly playable RTS interaction loop.

## Goal

In a dedicated prototype map, let the player control a small mixed force, move it, engage an enemy, and complete one readable tactical objective. The slice must prove that input becomes validated C++ orders, gameplay state changes outside the UI, and the HUD presents those changes clearly at the project's 4K desktop target.

This is a **prototype milestone**, not a promise of campaign, skirmish, faction balance, production-quality assets, full platform support, or a playable release. The existing main-menu map remains the packaged default. Keep the established shell and recovery behavior intact; make the prototype map available as an explicit development destination rather than silently turning every shell launch into a match.

## Bounded scenario

- One small, purpose-built map with traversable ground, clear boundaries, a start area, an enemy approach, and one marked control point.
- One human-controlled local player and a small fixed opposing test force. Opposing units may use simple scripted/test behavior; strategic AI is not part of this milestone.
- Two POLITIKA-owned prototype unit archetypes with visibly different silhouettes and behavior: a light reconnaissance vehicle and a mechanized infantry fireteam. Use simple original/placeholder meshes and materials; do not import or remodel C&C assets.
- One control-point objective: an uncontested player unit entering and holding the marked area fills objective progress; progress pauses while no eligible player unit is present or the area is contested. Completion and contested state are visible.
- Fixed test values for movement, health, weapon range/damage, and cooldown. These values are fixture data only, not a locked balance model or the final content schema.

## Player interaction contract

| Intent | Prototype behavior |
|---|---|
| Camera pan/zoom | Pan over the test map and zoom between useful strategic and close tactical views without changing unit state. |
| Select | Left-click a friendly unit to select it. Selection feedback is visible in the world and HUD. Multi-select, drag-box selection, control groups, and remapping are deferred. |
| Move | Right-click valid ground to issue a move order to the selected unit. Invalid/out-of-bounds orders are rejected with visible feedback and do not teleport or mutate the unit. |
| Attack | Right-click a valid hostile unit to issue an attack order. The unit moves into its weapon's legal range where required, applies cooldown-gated damage, and stops/clears the order when the target dies or becomes invalid. |
| Observe | The HUD reports selected unit identity/type, current health, current order, and objective state/progress. Damage, death, rejected orders, and objective completion each produce legible feedback. |

Use semantic player intents and the project's established input ownership direction; do not bake physical key names into simulation rules. Mouse defaults above are prototype controls, not a final cross-platform binding contract. Keyboard camera pan and mouse-wheel zoom may be used for the first desktop interaction pass; gamepad, touch, rebinding, and accessibility qualification remain later work. See Epic's versioned [Enhanced Input for UE 5.8](https://dev.epicgames.com/documentation/en-us/unreal-engine/enhanced-input-in-unreal-engine?application_version=5.8) and the [UE 5.8 PlayerController API](https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/Engine/GameFramework/APlayerController?application_version=5.8) when implementing the input-to-command boundary.

## Runtime ownership boundary

### Current implementation status

`Source/PolitikaCore/Rules.h` defines presentation-independent typed move, attack, attack-move, stop, and guard orders; admission results with explicit rejection reasons; and the authoritative phase sequence. `Source/Politika/Private/PolitikaPrototypeGameplay.cpp` now authors a bounded 20 Hz prototype coordinator, primitive test map setup, two friendly archetypes and one hostile, move/attack/attack-move/guard/stop commands, cooldown damage and death cleanup, a control-point rule, and a read-only HUD. The main menu exposes this only as a development destination. `PolitikaCreateShell` is extended to generate `/Game/Maps/RTSVerticalSlice` with its dedicated GameMode and the map is added to cooking.

The local Unreal installation at `/mnt/data/unreal/5.8.3` matches the pinned engine revision and was used to build, generate, cook, and package the scenario. The final Linux Development package was launched with Vulkan rendering at 3840×2160 and exercised with real keyboard and mouse input. Development-only automated fixtures cover the remaining capture-pause and attacker-death cases. This does not qualify other platforms or physical controllers/displays.

### Delivery and acceptance record (2026-10-27)

- **Source:** clean implementation commit `ea4dda30b828d03ec34bf0ea44a65c9851caa8a8` (`Implement first RTS vertical slice`). The acceptance-record update is documentation-only; the package below was built from this clean runtime source commit.
- **Engine:** Unreal Engine `5.8.3`, pinned source revision `396c9f059903aed5fec78ecd3d437a40c6415368`; `tools/unreal.sh doctor` passed project, install, and revision checks.
- **Build/package:** Linux `Development`, `Politika` target, UAT `BuildCookRun`; command `UNREAL_ENGINE_DIR=/mnt/data/unreal/5.8.3 cargo run --locked --quiet --manifest-path Tools/Shell/Cargo.toml -- package`; full log `Saved/BuildLogs/rts-vertical-slice-clean-package-20261027.log`, exit 0. Package root `Saved/Packages/Linux/Politika/`. `BuildIdentity.json`: game `0.1.0-dev.0`, engine `5.8.3-0+UE5`, `git_dirty=false`, revision `ea4dda30b828`, protocol/save/persistence/world schemas all `1`. SHA-256: executable `2551118497da4d7b186560fb7e338aeb4007e039f0132225b7d3f675415ce1bd`; `Politika-Linux.pak` `211e5c31ffb1deaae12308dc402e070bb4ad5c6040a5adaf6c77919cce787561`; `global.utoc` `d99eaf044d163bac0789e3f4273ab1ce248c26b0c95b3dd6cabf496215c6cda3`.
- **Map:** `/Game/Maps/RTSVerticalSlice`; generated by the `PolitikaCreateShell` commandlet and included in cooking. Source asset SHA-256: `48b253c13586b85094378eb354bb3421922ca6baeb81a847a0e6f3b19ed320e7`.
- **Rendered input pass:** final package launched in an Xvfb 3840×2160 display using Vulkan (`SF_VULKAN_SM5`). Both friendly archetypes selected and reported distinct identities/health; both moved; guard and stop feedback displayed; a friendly-target attack was rejected; a click with no valid map surface displayed rejection feedback; attack caused the opposing patrol's death and cleared the attack order; the objective visibly changed from contested to capturing and secured after the patrol died; WASD pan and wheel zoom changed the rendered frame; Escape returned to `/Game/Maps/MainMenu`. Screenshots/logs are locally captured under `Saved/ShellChecks/rts-vertical-slice-4k/`, including `frame-contested2.png`, `frame-enemy-destroyed-final.png`, `frame-secured-final.png`, `frame-invalid-final.png`, and `frame-pan-zoom-final.png`.
- **Editor PIE follow-up (2026-10-27):** Unreal Editor 5.8.3 opened `/Game/Maps/RTSVerticalSlice` in an isolated Xvfb session and Play-in-Editor was started from the editor viewport. The PIE log confirms duplicate world `/Game/Maps/UEDPIE_0_RTSVerticalSlice`, `PolitikaPrototypeGameMode`, successful world bring-up, and `PIE: Play in editor total start time 0.054643 seconds`. The rendered PIE viewport showed the RTS HUD, neutral control point, and all three scenario units; PIE was stopped normally after approximately 31 seconds. Screenshot: `Saved/ShellChecks/rts-vertical-slice-pie/editor-pie.png`; log: `Saved/ShellChecks/rts-vertical-slice-pie/Editor.log`. This verifies editor PIE startup/rendering only, not the remaining interaction checks. No project configuration was changed for the run.
- **Regression/tool checks:** Unreal Editor target build and final Linux package both succeeded. Core rules quality passed (typed-order admission/phase assertions); the existing packaged shell smoke passed **14/14**; `cargo fmt --check` and `git diff --check` passed. The normal packaged shell route remains the main menu.
- **Camera clamp follow-up (2026-10-27):** verified against the clean-source package on an isolated Xvfb/Vulkan 3840×2160 display. Holding W and D to their travel limits, then repeating the same pan beyond each limit, produced effectively unchanged rendered frames: ImageMagick absolute pixel differences were 4,502 and 2,352 pixels respectively (0.054% and 0.028% of 8,294,400 pixels; residual render noise). The map edge was visible at both limits. Evidence: `Saved/ShellChecks/rts-vertical-slice-acceptance/final-clean/camera/cam-y-edge-a.png`, `cam-y-edge-b.png`, `cam-x-edge-a.png`, `cam-x-edge-b.png`; runtime log: `Politika.log` in the same directory.
- **Capture-pause and attacker-death fixtures (2026-10-27):** added a Development-only `-RTSAcceptance=` harness to the prototype match. It invokes the same movement, capture, combat, damage, death, and cleanup phases as the normal scenario, without changing default or Shipping behavior. Values `capturepause`, `attackerdeath`, and `combined` run the matching fixture(s): the capture case moves the recon unit into the objective for 1.5 s and out; the death case sets it to 10 fixture HP, orders it to attack the opposing patrol at 440 units, and puts the patrol in Guard to return fire.
- **Capture-pause runtime check:** PASS in the final clean-source 3840×2160 Linux Development package. Log: `RTS_ACCEPTANCE_CAPTURE_PAUSE result=PASS progress_before=0.1300 progress_after=0.1300`. The HUD displayed `CONTROL POINT / CAPTURING 13%` while no eligible unit was in the radius. Screenshot: `Saved/ShellChecks/rts-vertical-slice-acceptance/final-clean/combined/capture-paused.png`; log: `Politika.log` in the same directory.
- **Attacker-death runtime check:** PASS in the same package run. Log: `RTS_ACCEPTANCE_ATTACKER_DEATH result=PASS defender_alive=1 attacker_removed=1 attacker_order_cleared=1 defender_target_cleared=1`. The attacker had an Attack order; the patrol returned fire through Guard, the attacker died in the damage phase, its order was stopped, it was removed, and the surviving defender cleared its target. Screenshots: `Saved/ShellChecks/rts-vertical-slice-acceptance/final-clean/combined/combat-active.png` and `attacker-destroyed.png`; log: `Politika.log` in the same directory.
- **Default launch regression:** PASS against the clean-source package. A no-map-argument launch loaded `/Game/Maps/MainMenu` with `PolitikaShellGameMode`; the 3840×2160 screenshot shows the normal POLITIKA menu and development RTS entry. Evidence: `Saved/ShellChecks/rts-vertical-slice-acceptance/final-clean/default/default-menu.png` and `default/Politika.log`.
- **Additional validation:** `UNREAL_ENGINE_DIR=/mnt/data/unreal/5.8.3 bash tools/unreal.sh doctor` passed; `cargo run --locked --quiet --manifest-path tools/version/Cargo.toml --bin quality -- core` passed C++20 compilation with warnings treated as errors and command-rule assertions; `git diff --check` passed before the source commit. No game-version/schema counters changed.
- **Completion:** all scoped Linux Development milestone checks are complete. The source is committed and the package was built from that clean source revision. Physical monitor/driver, controller, Windows/SteamOS/Android, production art/audio, campaign, multiplayer, and release qualification remain explicitly out of scope.

| Concern | Milestone owner |
|---|---|
| Prototype world, test-unit placement, and objective setup | A dedicated prototype GameMode/map or equivalent explicit scenario setup; not the shell GameInstance. |
| Camera, local selection, screen/world targeting, and command requests | Local PlayerController/camera interaction code. It may request actions but may not directly edit health, movement completion, or objective state. |
| Unit ownership, movement intent, health, damage, cooldown, death, and order validation | C++ gameplay classes/rules. Keep the implementation small and directly callable without widgets. Do not introduce networking, deterministic lockstep, MassEntity, or a general-purpose scripting system for this fixture. |
| Objective state and completion | A C++ objective actor/rules object with explicit owner/contest/progress state. The HUD observes it rather than determining completion. |
| HUD and feedback | Read-only projection of current selection, order, health, target, and objective state. Temporary primitive visuals are acceptable; no extracted C&C UI/art/audio is reused. |
| Prototype tuning | Narrow, named fixture values in C++ or a small project-owned prototype data asset. This does not replace the later versioned content contract or production import/validation pipeline. |

Use the C&C reference studies only for comparison prompts—command discoverability, response feedback, and role contrast. The [C&C gameplay study](CNC_GAMEPLAY_COMPARATIVE_STUDY.md) already separates observations from POLITIKA design; its specific units, code, maps, values, and assets are not copied into this slice.

## Explicit non-goals

- Economy, resource gathering, construction/production, deployment/loadouts, capture rewards, full match setup, or victory/defeat framework.
- Full faction roster, final unit balance, all-faction permissions, campaign, skirmish AI, story scripting, saves/replays, multiplayer, or server target.
- General-purpose content schema/editor, mod support, import pipeline, mass-unit optimization, or final map-lint implementation.
- Production art/audio, copied or adapted C&C source assets, or broad asset commissioning.
- Windows, SteamOS/Deck, Android, controller, and touch qualification. Linux evidence does not qualify those targets.

## Ordered acceptance criteria

1. The project builds with the pinned UE 5.8.3 toolchain, and the normal packaged launch still reaches the existing offline shell; the shell recovery/profile/settings checks are not regressed.
2. The dedicated prototype map can be launched explicitly in editor and in a Linux Development package. It loads the intended GameMode, camera, friendly units, hostile test force, and objective without runtime reference failures.
3. Camera pan and zoom remain within playable bounds and provide a useful view of the whole test scenario and the selected-unit interaction area.
4. Selecting either friendly archetype gives unambiguous world and HUD feedback. A hostile or empty-ground click cannot accidentally transfer ownership or select an enemy as friendly.
5. A valid move order changes the selected unit's position toward the requested point through the prototype's chosen movement/path solution; invalid ground commands are rejected visibly. No widget directly writes transform or movement state.
6. A valid attack order engages a hostile target, respects range and cooldown, reduces health, and stops cleanly when target or attacker dies. Friendly-fire and invalid-target behavior are explicit and rejected unless the scenario intentionally enables them.
7. Both archetypes have meaningfully different authored movement/weapon/health profiles, and each can be identified at normal RTS zoom. The values remain fixture-only.
8. The control-point rule updates from authoritative gameplay state, pauses/contests under its declared conditions, and reaches completion only when the rule is satisfied. The HUD accurately reflects idle, progress, contested, and complete states.
9. The packaged client is exercised with real rendering and mouse/keyboard input at 3840×2160; command feedback and labels remain readable, and the HUD does not obscure the main play area. `-nullrhi` or process exit status alone cannot satisfy this check.
10. Record the exact source revision, engine pin, build/package identity, test map, control steps, observed outcomes, and unresolved limits in the owning implementation/build record. Do not advance the game version merely for this planned milestone; version changes follow the repository's verified-milestone rules.

## Completion gate and follow-on

The milestone is complete only when the code path, scenario, HUD, and ordered acceptance evidence above exist. A design document, successful compile alone, shell smoke run, or screenshots without interactive gameplay evidence are insufficient. Gameplay checks should exercise both archetypes, successful and rejected commands, target death, and objective progress/completion.

After completion, use observed prototype results to refine command feel and lock only the decisions the slice actually tests. Then proceed to the broader simulation/content contracts and map validators in the [Engine Outline](ENGINE_OUTLINE.md); do not expand into economy or the full faction roster before this interaction loop is usable.

### Packaged invalid-content follow-up (2026-10-27)

A Development-only `InvalidMapSchema` fixture and selector were added after the original clean-source package recorded above. The separate Linux Development package cooked the fixture through an explicit test cook directory. Its runtime emitted `CONTENT.SCHEMA.UNSUPPORTED` for `map.test.invalid_schema.SchemaVersion` (expected 1, actual 2), then rejected admission with zero units, objectives, and simulation steps. Rendered `shell_recovery.rs` verification showed the recovery panel, launched the canonical valid fixture in the same process, then returned to the menu; the one-shot override did not contaminate retry. The package-side `rts_interaction.rs` replay verified selection, movement position updates, rejected friendly attack, stop, guard, attack-move, health reduction, enemy death, contested/uncontested objective behavior, and return to the shell, with machine-readable UE events and screenshots. Both tools run against the actual Linux Development package under Xvfb; `content_acceptance.rs` additionally passed default launch, cooked fixture resolution, repeated rejection diagnostics, no partial initialization, fresh-process valid start, and lifecycle fixtures. For the current command-contract source, a normal-route Linux Shipping archive was rebuilt without test cook options; its UFS manifest omits the fixture and executable strings, while visible startup is INCONCLUSIVE because the captured frame was black. The earlier baseline Shipping startup observation does not qualify this new source identity. See [content verification status](GAMEPLAY_CONTENT.md#packaged-negative-content-fixture); generated package logs, reports, and screenshots are under ignored package output directories.
