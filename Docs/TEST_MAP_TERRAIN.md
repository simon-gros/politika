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
