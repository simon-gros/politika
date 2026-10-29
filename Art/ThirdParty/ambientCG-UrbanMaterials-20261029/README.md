# ambientCG urban/industrial PBR material sample

Acquired 2026-10-29 from the sources recommended in `Docs/RTS_ASSET_RESOURCE_RESEARCH.md`.
These are source textures for later Unreal material authoring; they have **not** been imported, cooked, or wired into gameplay/content definitions.

## Included candidates

| Asset | Intended POLITIKA use | Source |
| --- | --- | --- |
| Asphalt012 | Worn/cracked road and yard surfaces | https://ambientcg.com/a/Asphalt012 |
| Concrete035 | Aged concrete/plaster for industrial and municipal exteriors | https://ambientcg.com/a/Concrete035 |
| Metal009 | Painted/industrial metal panels and props | https://ambientcg.com/a/Metal009 |

Each package is the publisher's 2K-JPG download and contains the color and available PBR maps, plus source material-description files. For Unreal, use the `*_Color.jpg`, `*_NormalGL.jpg`, and `*_Roughness.jpg` maps; `Metal009` also includes `*_Metalness.jpg`. Treat displacement as optional pending an explicit performance/art review. These surfaces are generic material references, not proof of a historically accurate Yugoslav location.

## License and provenance

- Publisher: ambientCG / Lennart Demes.
- License: CC0 1.0 Universal, per the publisher's license page. The publisher permits copying, modification, commercial use, and inclusion of raw assets in games; attribution is optional. Credit and this record are retained voluntarily.
- License source: https://docs.ambientcg.com/license/
- Captured license page: `LICENSE-source.html`.
- License text: https://creativecommons.org/publicdomain/zero/1.0/legalcode.en
- Retrieval date: 2026-10-29.
- Resolution/package: 2K-JPG for each material.

`SHA256SUMS` records every downloaded/extracted file. This directory is isolated from the Unreal `Content/` tree so that licensing, import settings, material packing, and platform budgets can be reviewed before integration.
