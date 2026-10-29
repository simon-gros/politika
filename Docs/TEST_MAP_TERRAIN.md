# Test-map terrain and camera controls

## Scope

The ordinary `/Game/Maps/RTSVerticalSlice` starts directly in the Editor and packaged game. Its existing central 40 × 40 metre command/collision ground and gameplay placements are retained. A 120 × 120 metre landscape supplies surrounding relief, with Marburgum's generated grass at four-metre tiling.

This merge contains terrain, source provenance and camera controls only. Uncommitted infantry, navigation, audio and documentation work in the shared development checkout is deliberately excluded; their combined playtest archives are not represented as evidence for this exact source tree. This remains a prototype, not production terrain, elevated traversal or a new release. Version/schema counters are unchanged.

## Assets and import

- [Elevation source and CC0 license evidence](../Art/Terrain/ambientCG-Terrain002/README.md): ambientCG Terrain002, adapted and flattened around the existing gameplay area. The original blank ground is hidden for rendering only; it remains the authoritative collision/command surface. Landscape collision and navigation contributions are disabled.
- [Grass maps, hashes and generation provenance](../Art/Terrain/MarburgumGrass/README.md): 1024-square albedo/normal/ORM. Albedo uses sRGB; normal and ORM are linear. Source OpenGL normal is green-flipped for Unreal. ORM channels are R ambient occlusion, G roughness, B metallic. Recorded model/tool licenses are not presented as a separate CC0 grant for these generated outputs.
- `/Game/Politika/Terrain/MarburgumGrass/M_TestMapGrass` samples world XY / 400 cm. The committed landscape embeds its height data and references these grass assets. Original Terrain002 color assets are generated only if recreating the initial terrain.
- `PolitikaImportTerrain -Source=...` adds missing terrain and preserves existing tagged terrain. `-GrassSource=...` explicitly changes only its material. `-TerrainAudit=ABSOLUTE_OUTPUT` records sorted actor/component transforms, collision/navigation flags, static-mesh paths and heightmap source IDs. Use client-capable Editor loading: server-only commandlets can omit render components, making resaves destructive. Material-only import refuses an empty landscape.

## Controls

- A pans left; D pans right, using the ground-projected camera right vector rather than world +X. W/S bindings are unchanged.
- Mouse wheel uses Unreal's `MouseWheelAxis`: up dollies closer, down farther. The centre ground focus and authored perspective are retained; view-ray distance is limited to 900–7500 cm. This replaces FOV-only zoom.
- Escape returns to the existing menu; explicit menu/map launch overrides remain available.

## Verification boundaries

The shared development checkout produced Linux packaged 1920 × 1080 Xvfb evidence for these same terrain and camera changes:

- Reloaded geometry audit preserves all 64 heightmap IDs and actor/component collision/navigation/transform invariants; repeat material import leaves the map byte-identical (`/mnt/data/politika/build/grass/`).
- A/D screen-space check: fixed objective x=959.7 initially, 1161.8 after A and 741.4 after D (`Saved/CameraControls20261029/visual/`).
- Wheel check: distance 3787.1 -> 2325.8 after three up notches, then 3787.1 after three down. Objective width 240 -> 392 -> 240 pixels; all 61 events respect direction and both distance limits (`Saved/CameraZoom20261029/visual/`).

Those original archives include additional pending gameplay changes. The isolated merge candidate is separately built and checked under `/mnt/data/politika-grass-merge/Saved/MergeValidation/`; see the recorded results below. The player's existing combined playtest package is not replaced with this scoped merge candidate.

### Isolated merge-candidate results

- Pinned Unreal 5.8.3 preflight and exact Editor target build: pass, exit 0 (`editor.log`).
- Reloaded map audit: all 64 heightmap IDs and geometry/collision/navigation/transform rows match the grass playtest audit (`geometry.txt`, compared byte-for-byte). Saved content/spatial validator: pass, zero diagnostics (`content-validation.log`).
- Repository quality audit: pass (`audit.log`, including documentation links). Source asset checksum manifests verify.
- Rendered exact-candidate Editor `-game` run on isolated Xvfb at 1920 × 1080: grass visually inspected; three wheel-up events reduce distance 3787.1 -> 2325.8, three down restore 3787.1, FOV remains 65. A/D pan verified by screenshots; Escape returns to menu and process exits 0. Evidence: `visual/` screenshots and runtime log.
- A fresh native package was attempted but deliberately cancelled (UAT exit 143) after it requested 569 engine/project compiler actions in the clean worktree. It is **not** counted as a successful candidate package. The exact candidate is Editor-build/render qualified; earlier successful Linux package regressions apply to the same camera/terrain changes in the separately identified combined checkout, not this full candidate tree. No new release/platform certification is claimed.

### PAC-10 camera-pan assertion (harness defect, fixed)

The packaged `rts_interaction` replay intermittently failed PAC-10 with `camera input reached the controller but its clamped position did not change: ... RTS_CAMERA_INPUT axis=right value=1.0 x=-0.0 y=0.0`. Observed once on the MP-2 `c14a6c4` Linux Development archive (`/mnt/data/politika-qualification/MP2-Multiclient-20261029/logs/offline-pac10-attempt1-fail.log`); the immediate re-run passed.

**Cause: the harness check, not the controller or a readiness race.** The old check took the right-pan log line and required `|x|` or `|y|` > 1, an absolute offset from the origin. That is not a movement test:

- The replay presses `s` (Stop) and `a` (AttackMove) earlier. In `Config/DefaultInput.ini` they also scale the `PrototypePanForward`/`PrototypePanRight` axes by −1, so each tap can pan the camera for a frame or so. At 60 fps a frame is 1400 cm/s × 1/60 s = 23.3 cm.
- In the failing evidence (`content-acceptance-interaction-1793275735/rts-interaction.log`, frame 80), those nudges had already moved the camera to (23.3, −23.3). W+D in one frame then logged `axis=forward ... x=23.3 y=0.0` followed by `axis=right ... x=-0.0 y=0.0`. The camera did move 23.3 cm screen-right (world −X) and landed exactly on the origin, so the absolute check fired.
- Pan bounds were applied. The forward line kept x=23.3 after `PanForward` clamped both axes; zero or unapplied bounds would have clamped it to 0. The map bounds are (−2000, −1850)–(2000, 2000), and the pan step runs after ~50 s of replay, long after `ApplyMapCameraIfReady`. `PanForward`/`PanRight` behave correctly, so the controller is unchanged.

**Fix (`Tools/Shell/src/bin/rts_interaction.rs`, `require_right_pan_moved`).** The forward log line is the baseline. It is logged before right in the same or an earlier frame, and `PanForward` changes only world Y. The check now requires the X change from forward to right to be at least 1 cm in the screen-right direction, world −X, since the fixture camera looks along +Y. Unit tests use the real passing line and the real failing (origin-return) line. They also reject zero, wrong-direction, missing and misordered states.

**Evidence** (`/mnt/data/politika-qualification/PAC10-CameraPanFlake-20261029/`; package `/mnt/data/politika-mp2-multiclient-20261029/Saved/MP2Multiclient20261029/Final-c14a6c4/Development`; `env -u WAYLAND_DISPLAY SDL_VIDEODRIVER=x11 xvfb-run -a -s "-screen 0 1920x1080x24"`; `loop.sh`, per-run logs and `*-summary.tsv` under `logs/`):

| Harness | Runs | Verdict | Notes |
| --- | --- | --- | --- |
| Unchanged `33ff2c0` (`bin/rts_interaction-baseline-33ff2c0`) | 10 | 10/10 PASS | No repro; the origin landing needs both nudges to cancel exactly. Nudges show in the variable starting positions, e.g. run 7 forward x=23.3 (an `a` nudge) and run 9 forward y=−6.8. |
| Fixed (`bin/rts_interaction-fixed-33ff2c0-dirty`) | 10 | 8 PASS, 2 FAIL | The camera check passed in all 8 runs that reached it. Runs 3 and 4 failed earlier with `timeout waiting for RTS_ORDER_ADMITTED order=Grenade unit=2` (see below). This loop overlapped an unrelated UAT build, so frame rates varied from ~21 to 60 fps, with first-frame right-pan steps of 23–67 cm. |
| Fixed check, unit tests | 4 new | PASS | `cargo test --locked --manifest-path Tools/Shell/Cargo.toml --bin rts_interaction`: 10/10. `cargo fmt --check` clean; clippy shows only the 5 existing warnings. |

The failure was not reproduced live in 20 runs. It needs nudges that cancel exactly. The diagnosis rests on the exact recorded failing lines, which the new check accepts as a 23.3 cm right pan.

**Open finding: the shared hotkey/pan keys also break the grenade step.** In fixed runs 3 and 4 (`content-acceptance-interaction-1793280363`, `-1793280385`), the grenade intent was sent with `target=0` and rejected (`That_target_is_no_longer_available`). In passing runs it is `target=3`, as in all 10 baseline runs. The replay video (`grenade-triage/tile-1793280363.png`, 2 fps crop around the aim point) shows the scene shifting about 15 px when `a` (AttackMove) is pressed. The fixed `577,323` grenade click then lands beside target #3. A passing run (`tile-1793280406.png`) shows no visible shift. The nudge grows with frame time, which is why it appeared under the concurrent build. This is a player-facing input conflict, not only a harness issue: pressing S (Stop) or A (AttackMove) also pans the camera, by up to one frame of pan speed. Changing the shared bindings or suppressing pan for command keys is an input-design decision. It needs a C++/config change and a new package, so it is not done here. PAC-10 can still flake on the grenade step on a loaded host until it is resolved.
