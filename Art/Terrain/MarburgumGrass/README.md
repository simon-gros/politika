# Marburgum grass — POLITIKA test-map surface

Copied byte-for-byte from `/home/simon/magna-raedarum-marburgum/project/assets/materials/grass/` on 2026-10-28. Original generation metadata is retained in `grass.provenance.json`; hashes are in `SHA256SUMS`.

This is Marburgum's generated short-lawn surface, not a texture extracted from a commercial reference game. The provenance identifies SDXL base 1.0 (CreativeML Open RAIL++-M) and PBRify Remix (CC0-1.0). These are recorded model/tool licenses, **not a claim that the generated outputs carry a separate CC0 grant**. Preserve this metadata with future distributions.

- 1024 × 1024 PNGs, intended footprint 4 × 4 metres.
- Albedo: sRGB, wrapping.
- Normal: linear, normal-map compression, source OpenGL +Y; flip green on import for Unreal.
- ORM: linear masks; R ambient occlusion, G roughness, B metallic.
- Material: `/Game/Politika/Terrain/MarburgumGrass/M_TestMapGrass`, world XY / 400 cm.

Reapply using the pinned Editor after building `PolitikaEditor`:

```sh
/mnt/data/unreal/5.8.3/Engine/Binaries/Linux/UnrealEditor-Cmd \
  /mnt/data/politika/Politika.uproject -unattended -nop4 -nullrhi \
  -run=PolitikaImportTerrain \
  -GrassSource=/mnt/data/politika/Art/Terrain/MarburgumGrass
```

This explicit material-only mode requires exactly one existing tagged test landscape. It does not regenerate heights, add actors, or change collision/navigation. Repeating it preserves the map when the material is already assigned. The original ambientCG elevation source remains unchanged and separately licensed.
