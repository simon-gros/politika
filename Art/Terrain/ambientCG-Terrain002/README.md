# Terrain002 — CC0 test-map terrain

## Source and rights

- Asset: [Terrain002, ambientCG](https://ambientcg.com/a/Terrain002), by ambientCG / Lennart Demes.
- Retrieved: 2026-10-28. License: **Creative Commons CC0 1.0 Universal**.
- Publisher's [license statement](https://docs.ambientcg.com/license/): “All ambientCG assets are provided under the Creative Commons CC0 1.0 Universal License.” It explicitly permits copying, modification, commercial use and including raw files in a game. Attribution is optional; this record retains source credit.
- [CC0 legal text](https://creativecommons.org/publicdomain/zero/1.0/legalcode.en).
- `LICENSE-source.html` is a saved copy of the publisher's license page. `SHA256SUMS` identifies the downloaded elevation/color files and derived R16 input.
- Elevation download: `https://ambientcg.com/get?file=Terrain002_2K.exr`.
- Color download: `https://ambientcg.com/get?file=Terrain002_2K-PNG.zip`, member `Terrain002_2K_Color.png`.
- Some command-line clients do not encode the publisher's redirect paths; the actual download path must encode spaces as `%20` and `^` as `%5E`.

This is an adapted generic landscape, **not a historically sourced or geographically accurate Yugoslav location**. No satellite imagery, reference-game assets, paid shaders or third-party engine packages are included.

## Adaptation and import

The original 2048² EXR elevation and 16-bit color PNG are preserved. ImageMagick generates a 505² unsigned little-endian height input:

```sh
magick Art/Terrain/ambientCG-Terrain002/Terrain002_2K.exr -colorspace Gray -auto-level -resize '505x505!' -depth 16 -endian LSB gray:build/terrain/Terrain002_505.r16
cp Art/Terrain/ambientCG-Terrain002/Terrain002_2K_Color.png build/terrain/Terrain002_2K_Color.png
UNREAL_ENGINE_DIR=/mnt/data/unreal/5.8.3 bash tools/unreal.sh build -MaxParallelActions=6
/mnt/data/unreal/5.8.3/Engine/Binaries/Linux/UnrealEditor-Cmd /mnt/data/politika/Politika.uproject -run=PolitikaImportTerrain -Source=/mnt/data/politika/build/terrain -unattended -nop4 -nullrhi -nosound
```

The commandlet adds a native Unreal Landscape to `/Game/Maps/RTSVerticalSlice`, with 64 components / 255,025 vertices over 120 × 120 metres. The imported relief is rescaled and smoothly flattened across the existing 40 × 40 metre test area; resulting surrounding relief is approximately 10.3 metres high. The original import maps its color across the terrain once and generates assets under `/Game/Politika/Terrain/Terrain002`. The committed map instead uses the separately documented [Marburgum grass](../MarburgumGrass/README.md) at four-metre tiling; reapply with the importer's explicit `-GrassSource=` mode after recreating a map from scratch.

This deliberately preserves the authored collision plane, navmesh, unit/objective placements and menu return route. Startup selects this map by default in the Editor and packaged game. Only the old plane's rendering is hidden; the Landscape is non-colliding and navigation-neutral scenery. Elevated terrain traversal is **not implemented** by this import, and orders beyond the existing playable bounds remain invalid. Existing imported terrain is detected by tag and preserved on subsequent importer runs. Background music and pending infantry/navigation changes from the shared development checkout are not part of this scoped terrain/camera merge.

Before the first import, the existing dirty map was backed up under ignored `build/terrain/before-import/`, together with hashes of the shell and prototype maps. The importer does not regenerate authored maps or modify the main menu/sandbox.
