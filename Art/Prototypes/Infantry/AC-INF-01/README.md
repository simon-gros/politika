# AC-INF-01 — Atlantic Concord infantry static prototype

**Asset:** `AC-INF-01` — Professional Rifle Section (visual prototype v2). **Status:** original concept-guided, static, textured **PLAYTEST CANDIDATE**, not an approved, imported or packaged Unreal asset.

This source-art directory preserves an actual **single-mesh glTF 2.0 GLB** for review in Blender and optional presentation in POLITIKA's UE 5.8.3 RTS prototype. It is based on the [AC-INF-01 concept sheet](../../../../Factions/TheAtlanticConcord/Units/AC-INF-01.png) and [roster](../../../../Docs/POLITIKA_FIVE_FACTION_ROSTERS.md). It is **not** a mechanically implemented Atlantic Concord roster unit: the current test map's authored archetypes are prototype gameplay definitions.

## Contents

| File | Purpose |
| --- | --- |
| `SM_AC_INF_01_Playtest.glb` | 1.7 MB textured, triangulated, **one-piece static** mesh with embedded PNG atlas; Blender/UE import source. |
| `manifest.json` | Stable ID, dimensions, source provenance, SHA-256, capabilities and qualification states. |
| [Rust authoring tool](../../../../Tools/InfantryAuthoring/README.md) | Cargo commands to generate named review components and the atlas, merge a static playtest GLB, or inspect existing exports. Offline only; not runtime scripting or a build requirement. |
| `.gitignore` | Ignores regeneration-only files in `Generated/`. |

The GLB has **22,544 triangles**, one named mesh, UVs and an embedded color texture; bounds approximately **1.423 × 1.010 × 1.914 m**, standing **1.914 m** high. Original Blender-style source has many individually named components, but this playtest export is combined for one `UStaticMesh`. This is a posed soldier, with helmet/goggles, field clothing, webbing, rucksack and rifle. **No skeleton, skin weights, movement animation, sockets, authored LODs, physics asset, collision mesh, or baked normal map.**

## Import to Unreal Engine 5.8.3 (one-time Editor step; not yet performed)

1. Enable the project's necessary **Interchange/glTF import plugins** in the local Editor as required (the `.uproject` disables engine plugins by default). Import the GLB through the Content Browser with *static mesh*, material and texture import enabled; do **not** select Skeletal Mesh. See [Epic Interchange guide](https://dev.epicgames.com/documentation/en-us/unreal-engine/importing-assets-using-interchange-in-unreal-engine).
2. Import and **Save All** native assets at `/Game/Politika/Prototypes/Infantry/SM_AC_INF_01_Playtest` (on disk: `Content/Politika/Prototypes/Infantry/`). Verify that the output is **one static mesh** plus its assigned material/texture and that the texture color survives the import. The source `.glb` under `Art/` alone is **not cooked**.
3. Verify size in the Unreal Static Mesh Editor: **~191 cm** from boots to helmet and suitable orientation. The source is metre-scale, +Z up, source forward −Y; the importer may convert units and axes. **Do not blindly apply an additional 100× scale**. Verify pivot and feet relative to the existing test-unit actor origin, which currently spawns at `UnitGroundZ = 42 cm`; adjust the *imported mesh pivot* or asset placement deliberately if the feet float. Do not move the authoritative simulation actor to compensate.
4. Generate/author a **simple bounding collision** suitable for visibility traces; make sure collision is query enabled and the existing unit selection ray can hit it. Do not substitute detailed rifle triangles as physical blocking geometry. Recheck the selection indicator's position, diameter, and occlusion.
5. For a **controlled visual-only substitution** in the existing test match, edit the `DA_MechanizedFireteam` unit archetype **in a disposable test copy / controlled test change**: set `VisualMesh` to the imported `SM_AC_INF_01_Playtest`, choose `VisualScale` by measured Unreal dimensions (normally `1,1,1` after a correct cm import), and enable `bUseAuthoredMeshMaterials`. This preserves the external texture instead of applying the current simple-shape faction tint. This **does not change its gameplay archetype ID, faction identity, stats or combat behaviour**. Do not claim it becomes `AC-INF-01` gameplay content merely by swapping its model.
6. Save all changed Unreal assets and verify `DA_PrototypeContentCatalog` still resolves its units. Exercise *selection, orders, attack, death, visibility traces, world lighting, team differentiation* in the Editor and in a **freshly cooked Linux Development package**. The imported mesh must be referenced by a cooked `UPrimaryDataAsset`; test cooked-package loading, not just an Editor preview.

The current `APolitikaPrototypeUnit` uses a `UStaticMeshComponent` as its root/clickable mesh, and its `Configure` method normally replaces the imported material with a color-only `BasicShapeMaterial`. The new opt-in `bUseAuthoredMeshMaterials` data field clears the prototype material override **only for authored meshes**; existing assets retain their old tint and gameplay behaviour by default. **Collision and unit-ground offset still require real UE verification.**

## Playtest acceptance / release gate

| Check | Current assessment |
| --- | --- |
| Original concept present, provenance linked | VERIFIED — concept sheet is in `Factions/` |
| Real GLB mesh, geometry, UVs, texture, and offline GLB readback | VERIFIED (offline) |
| Blender editable review of model | REVIEW CANDIDATE — import for inspection |
| UE Interchange import, scale/orientation, saved `.uasset` | **NOT RUN** |
| Selection traces and mesh collision in Unreal fixture | **NOT RUN** |
| Screen readability at gameplay zoom; faction contrast / accessibility | **NOT RUN** |
| Linux Development **cooked** player-facing playtest | **NOT RUN** |
| Rig, animation, authored LOD and optimized mobile profile | **NOT IMPLEMENTED** |

**Playtest tasks:** confirm a soldier is visible in map lighting without palette loss; remains selectable; moves when ordered (static pose is expected); remains identifiable among multiple units; shoots and dies according to existing mechanics; works after cooking; check object count, LOD and overdraw before scaling to squads or Android. An accepted **visual playtest** does not certify final character art, animation or historical accuracy.

**Important:** Do not overwrite, delete or regenerate existing `.uasset` assets automatically. The local Unreal Editor/native asset import and package validation are required before marking player-facing use as complete.

## Regeneration (optional)

Run with Rust/Cargo from the repository root (not inside the engine runtime):

```bash
cargo run --locked --manifest-path Tools/InfantryAuthoring/Cargo.toml -- generate
cargo run --locked --manifest-path Tools/InfantryAuthoring/Cargo.toml -- merge
cargo run --locked --manifest-path Tools/InfantryAuthoring/Cargo.toml -- inspect Art/Prototypes/Infantry/AC-INF-01/Generated/SM_AC_INF_01_Playtest.glb
```

No existing output is overwritten. For another generation, choose a fresh `generate --output-dir DIR` and use matching `merge --input DIR/AC-INF-01_Professional_Rifle_Section_v2.glb --output DIR/SM_AC_INF_01_Playtest.glb` paths. See the [tool documentation and migration evidence](../../../../Tools/InfantryAuthoring/README.md).

The Rust recipe retains 299 components and 22,544 triangles, with a deterministic 20-region PNG atlas. Its noise sequence and circular-primitive orientation differ from the legacy export, and varying-ellipse cap winding is corrected. Regeneration is **not byte-identical** to the existing committed GLB, which is preserved unchanged.

Regenerated exports can vary with dependency versions. Do not silently replace the committed GLB without visual review, geometry/texture verification, a new manifest/hash, and an updated acceptance record. The input sketch is original project concept artwork; colors are deliberately restrained approximations. No historical-uniform or insignia authenticity has been certified.
