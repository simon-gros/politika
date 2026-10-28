# Infantry authoring (Rust, offline only)

Replaces the two former AC-INF-01 Python authoring scripts. Uses `glam` for primitive transforms, `gltf` for independent GLB readback, `image` for PNG, `sha2` for hashes and `zip` for the review bundle. Cargo.lock pins the dependencies. No Python, numpy, trimesh, Pillow or Unreal runtime dependency.

From the repository root:

```bash
cargo run --locked --manifest-path Tools/InfantryAuthoring/Cargo.toml -- generate
cargo run --locked --manifest-path Tools/InfantryAuthoring/Cargo.toml -- merge
cargo run --locked --manifest-path Tools/InfantryAuthoring/Cargo.toml -- inspect Art/Prototypes/Infantry/AC-INF-01/Generated/SM_AC_INF_01_Playtest.glb
```

`generate` creates an ignored `Art/Prototypes/Infantry/AC-INF-01/Generated/` review directory containing:

- `AC-INF-01_Professional_Rifle_Section_v2.glb`: 299 named procedural mesh components, 22,544 triangles, UVs, normals and an embedded 1280 × 1024 PNG atlas.
- `AC-INF-01_color_atlas.png`: external review copy of the deterministic 20-region atlas.
- `AC-INF-01_manifest_v2.json`: component counts, provenance, dimensions, limitations and review-GLB SHA-256.
- `AC-INF-01_README_v2.md`: Blender/import review notes.
- `AC-INF-01_v2_Blender_Review_Package.zip`: the four files above.

`merge` bakes the selected scene's parent/node transforms (including normal transforms and reflected winding), combines its components into one `SM_AC_INF_01_Playtest` mesh, and preserves vertex positions, triangle indices, UVs and the embedded PNG. Legacy inputs without normals receive calculated normals. It writes `Generated/SM_AC_INF_01_Playtest.glb` by default and prints geometry statistics and SHA-256 after readback. `inspect` verifies the same static GLB profile without writing.

This is a bounded AC-INF-01 tool, not a general glTF converter: exactly 22,544 triangles and a standing height between 1.83 and 2.05 m are required. It requires UV channel 0 and the original opaque, double-sided, roughness 0.92/nonmetallic material profile with one shared embedded PNG atlas. External buffers/images, non-triangle primitives, skinning, animation, morph targets, singular transforms and unsupported material changes are rejected instead of silently discarded. Reader checks cover finite coordinates, unit normals, valid UV ranges and index bounds. Source coordinates remain metres, +Z up, forward −Y, matching the existing source asset; verify importer axis/unit conversion.

## Output protection and interruption recovery

Existing files are never overwritten, including committed GLBs and `.uasset` files. A second generation fails if its outputs already exist. For another run (or a run interrupted during file writing), keep the previous directory and choose a fresh one:

```bash
cargo run --locked --manifest-path Tools/InfantryAuthoring/Cargo.toml -- generate --output-dir /tmp/ac-inf-review-next
cargo run --locked --manifest-path Tools/InfantryAuthoring/Cargo.toml -- merge --input /tmp/ac-inf-review-next/AC-INF-01_Professional_Rifle_Section_v2.glb --output /tmp/ac-inf-review-next/SM_AC_INF_01_Playtest.glb
```

Generation and merge validate the encoded GLB before writing it. Writes use exclusive creation; failure/interruption can still leave partial files, which are not accepted as proof of success. Inspect the completed GLBs and compare the printed hashes. The tool never promotes new exports into the committed art directory automatically.

## Migration qualification (2026-10-28)

- Original design recipe retained, including all 299 components and 22,544 triangles; the malformed legacy cheekbone/brow statement is replaced by valid Rust calls.
- Varying-ellipse primitive cap winding corrected to face outward. Rust noise uses fixed-seed xorshift/Box–Muller rather than numpy. Circular primitives use `glam` quaternion alignment; their roll can differ slightly from the legacy dependency's alignment. New exports are **not byte-identical** or visually certified replacements.
- Regenerated bounds: min `[-0.59377444, -0.54928255, 0.005]`, max `[0.82926494, 0.45950001, 1.91900003]` m. Height remains 1.914 m; legacy depth 1.010469 m becomes 1.008783 m.
- The existing committed `Art/Prototypes/Infantry/AC-INF-01/SM_AC_INF_01_Playtest.glb` is unchanged, SHA-256 `65e01a7a609bcbd477429651116b867c39e619d2fe36bb18dcce6520410674b7`. Its manifest is also unchanged. The Rust reader verified its geometry/UV/atlas profile.
- Six regression tests cover recipe/GLB roundtrip, hierarchy/nonuniform scale/reflection, rejection of unsupported/malformed inputs, legacy missing normals, repeatable generation/merge/ZIP contents and overwrite protection, and corrected cap winding.
- Cargo tests, formatting, Clippy with warnings denied, and the project metadata `quality audit` passed. This is an offline tooling qualification: Blender visual review, Unreal import and a cooked gameplay package were **not run** for the Rust exports. No new game version or release claim.

Validation commands:

```bash
cargo test --locked --manifest-path Tools/InfantryAuthoring/Cargo.toml --all-targets
cargo fmt --manifest-path Tools/InfantryAuthoring/Cargo.toml -- --check
cargo clippy --locked --manifest-path Tools/InfantryAuthoring/Cargo.toml --all-targets -- -D warnings
cargo run --locked --quiet --manifest-path tools/version/Cargo.toml --bin quality -- audit
```
