# POLITIKA — Historical, Map, Art and Audio Production Standards

**Status:** Production policy and design-stage registries as of 27 October 2026. This document does not certify historical claims, asset rights, imported Unreal content or a playable build. Applicable stack: UE 5.8.3/C++ for in-game systems, Rust for validated offline metadata and preprocessing; optional C# external tools when justified.

**Ownership:** The [Archive](../Archive/README.md), [Historical Rights Policy](../Archive/RIGHTS_AND_PROVENANCE.md) and [Map Catalogue](../Archive/MAP_CATALOG.csv) own factual references and rights; [Story](STORY_OUTLINE.md)/[Concepts](CONCEPTS_OUTLINE.md) own alternate-history fiction; [Engine](ENGINE_OUTLINE.md) owns runtime import and simulation; [UI](POLITIKA_UI_OVERVIEW.md) owns player-facing legibility. This document provides production acceptance, not a duplicate source of truth.

## 1. Historical authenticity: three distinct layers

1. **Historical evidence:** Dated, attributable sources about Yugoslavia and relevant international events, particularly 1980–1989. A source's publication date, depicted date, map survey year and described event date may differ; record those differences. Source confidence and usage rights are different questions.
2. **Historical/fictional interpretation:** A reviewed decision identifies which real-world facts remain baseline, which diverge in the fictional 1984–1989 timeline, and which are unsupported or disputed. Never describe fictional assassinations, faction deeds, nuclear schemes or propaganda as real historical events.
3. **Game representation:** A map tile, strategic border, railway, base, unit, broadcast, document, objective or art asset derived from an approved interpretation. It may intentionally compress geography or rearrange military geography for RTS clarity; preserve the decision and source references.

[Historical decisions](../Data/historical_decisions.json) is the small, currently empty approval ledger. Future decision records should include a unique ID, scope/date, archive source IDs, evidence summary, relationship to the fictional canon, review state, production consumers, reviewer and rationale. **Empty is accurate:** no approval is inferred from a catalogue row's mere existence.

**Review triggers:** New borders, political subdivisions, dated institutions and officeholders, nationally significant infrastructure, military equipment, broadcasting styles, uniforms/insignia or any claim of historical authenticity. Use the existing chronology and political events catalogue for corroboration; disputed subjects need independently sourced interpretation. An archival link, uninspected PDF/map sheet or current-day map is not validation of a specific 1980s asset.

**Acceptance:** An artist or mission designer can answer “What is historically sourced, what is invented, what was changed for strategy, and what permits reuse?” for a significant asset. If the answer is unknown, mark the record `needs_review` rather than invent it.

## 2. Map production and change control

The [map-source registry](../Data/map_production.json) initially names two **research-only** 1981 source entries from the existing [map catalogue](../Archive/MAP_CATALOG.csv). They are *not* imported meshes, licensed images or completed playable territories. Source IDs must continue to resolve to that catalogue and cannot be promoted directly to game assets.

The production path is:

```text
Archived map metadata + rights
  → period evidence / review decision
  → strategic map design (explicit fictional overrides)
  → engine-neutral geographical/strategic source
  → UE terrain, strategic regions and minimap derivatives
  → targeted geometry/visibility/navigation review
  → packaged RTS playtest
```

**Required per candidate:** immutable source ID; date/period status; source URL and rights from the archive catalogue; real geographic scope and map scale/projection where known; intended strategic use; production status; historical ruling ID if one exists; source hash only after a real local asset file exists; generated/derived asset path only after it exists. Null values are preferable to invented scale, rights or hashes.

**Geographic priority:** Republic and autonomous-province borders, major cities, railway/road junctions, river crossings, valleys, industrial corridors, energy sites and defensible military chokepoints. Tactical maps may simplify geometry, but must remain coherent with campaign geography and the period or explicitly record an alternate-history departure.

**Change review:** Compare changed source IDs, dimensions/bounds, provincial lines, landmarks, roads, rivers, playable boundaries and affected objectives. A map change must not quietly invalidate group pathing, strategic points, fog-of-war boundaries or saved map identities. Avoid setting up a full satellite/LiDAR/worldwide geospatial pipeline; use bounded datasets selected for playable maps. Do not commit questionable archival images into production assets.

**First playable map gate:** Validate spawn territory, commandable ground, impassable features, path corridors, production space, minimap placement and faction/NPC starting control. Review at gameplay camera scale and representative zoom.

### Historical-world review for missions, maps and props

The [Yugoslav World and Campaign Systems Model](YUGOSLAV_WORLD_MODEL.md) identifies which period-world distinctions should survive translation into RTS maps and campaign fixtures. This is a **review checklist for future historically situated content**, not an additional requirement for the current abstract prototype map.

Before a dated Yugoslav mission/site is described as period-accurate, verify:

1. **Administrative and geographic scope:** correct year, republic/province/municipality, historic city name, institution and border status. Kosovo and Vojvodina were autonomous provinces within Serbia, not additional socialist republics.
2. **Physical versus institutional topology:** terrain connectivity, railway/road/bridge existence, access permissions, jurisdiction, operational infrastructure and the possible separation of controller from owner. Do not infer police jurisdiction from a pathing cell.
3. **Location-specific civil activity:** credible period economy, transport, public services, newspapers, university/workplace life, signage and languages. Distinguish population and institutional diversity from fictional military allegiance.
4. **Chronology and alternate-history ruling:** whether a claim is sourced from real 1980s history, an already locked fictional event, or a proposed divergence-sensitive interpretation. Do not mechanically import post-1980 real executive officeholders or commemorations into a story where Tito lives until 1984.
5. **Narrative evidence and rights:** a prop styled as a historical newspaper is not an actual archive record; original fictional in-world dossiers need separate authenticity/custody metadata if the mission simulates it. Source image/art/audio licensing still requires item-level approval.
6. **Visual gameplay readability:** map coverage, usable starts, passable movement profiles, protected civilians, noncombat objectives and observed/reported information must remain discernible at tactical camera distances.

Keep the source-evidence review and any future in-world intelligence/evidence state **separately identified**. Review the archive source and approved alternate-history ruling before promoting a site, event or historical person to a mission. Incomplete research is reported as `needs_review`; it is not filled by a speculative fake citation.

## 3. 3D asset standards for an RTS camera

An asset's **importance tier** reflects function and tactical recognizability, not narrative prestige alone.

| Tier | Examples | Minimum production policy |
| --- | --- | --- |
| A — signature/gameplay critical | Faction-defining units and headquarters, strategic bridges, nuclear production assets, critical mission structures | Protected silhouette, legible team identification and selectable bounds; manually inspected LOD and damage states |
| B — frequently encountered | Tanks, aircraft, infantry, common production facilities, frequent map monuments | Reusable rig/material families, bounded unique detail, good selection and damage feedback |
| C — reusable map fabric | Industrial shells, apartments, trees, rail props, fences, ordinary roads | Modularity, instances and few materials; simplified collision and scalable LOD |
| D — distant dressing | Non-interactive panorama, low-salience distant structures | Lightweight representations; avoid collision/gameplay cost unless needed |

**Technical contract when actual assets appear:** asset ID, semantic role and faction, source/model provenance, rights state, units and scale, forward/up convention, pivot/origin, collision purpose, material slots, LOD/instancing strategy, silhouette at minimum expected RTS zoom, selection highlight legibility and render performance. Shared unit category art should vary deliberately by faction without losing the ability to recognize classes in combat.

**Acceptance:** Test from the *actual top-down/angled RTS camera*, at screen sizes representative of desktop 4K, compact handhelds and Android, with clutter, fog, combat VFX, different terrain and accessible color settings. A detailed model is not production-ready merely because it looks good in a close-up render.

**Budget rule:** Do not prescribe fixed poly/texture/VRAM limits without representative packaged benchmarks. Simplify repeated objects and visual effects before sacrificing the critical unit/command silhouette.

## 4. Visual art direction: 1980s strategic political atmosphere

Ground the battlefield in the architecture, transport, media and industrial environments of Yugoslavia and the broader Cold War period. Use dated references for military and civilian technology, factories, power stations, urban infrastructure, vehicles, typography and cartographic conventions. The fictional faction identities use their own established designs; real state emblems or archive imagery are not automatically cleared for new game art.

Primary visual tests are **readability** and **period coherence**: distinguish factions, neutral authority, intelligence uncertainty, owned versus contested installations, occupied versus empty buildings, explosives/espionage alerts and strategic resources. Fog-of-war and deliberately false intelligence must be *game-state semantics*, not ambiguous decorative filters.

Use consistent palettes and reusable materials, controlled weather/lighting and camera-distance-appropriate textures. Avoid permanent VHS noise, heavy blur, exaggerated chromatic aberration or faux scanlines across the battlefield; if period media uses those effects, keep them scoped to clearly identified in-game radio, newspaper or television presentation.

**Representative future capture set:** bright daylight; overcast; low-contrast dusk; industrial night; fog/visibility test; high-density battle; dense base/production interface. Review the identical camera positions and game state where practicable. No captures are represented as existing yet.

## 5. Audio production and faction broadcasts

The [planned audio event catalogue](../Data/audio_events.json) establishes **semantic event IDs without invented audio files**. Sound assets will later attach to events via Unreal's audio routing and must carry provenance and a license/reuse decision.

| Bus | Intended role |
| --- | --- |
| `ui` | Selection feedback, critical warnings, production notices, unobstructed commands |
| `sfx` | Weapons, machinery, vehicles, explosions, world emitters |
| `ambience` | Industrial, weather, natural and battlefield backgrounds |
| `music` | Menu, campaign and limited tactical score |
| `voice` | Local unit acknowledgments and commander feedback |
| `broadcast` | Propaganda, political radio and announcements, with explicit in-world attribution |

**Cue priority:** Gameplay warnings and selection/order feedback must be understandable even when ambient sounds, music and propaganda overlap. Broadcast content may be narratively misleading when the simulation generates deception, but the interface must still indicate the distinction between a reported claim and verified intelligence. Critical alerts should have visual equivalents; do not rely on audio alone.

**First audio milestone:** import one licensed/original selection cue, one combat warning, one order acknowledgement and one broadcast sample; verify their real Unreal playback routing, volume behavior, caption/transcript strategy and recognition in a busy skirmish. Catalog entries `planned` are not recorded, licensed, implemented or mixed.

## 6. Promotion and responsibility

- **Rust** validates manifest structure, referenced archive IDs, safe file paths and metadata consistency; offline derived-content generators may be added when inputs exist.
- **C++/Unreal** owns authoritative map state, units, gameplay-facing audio triggers and actual rendering/asset validation.
- **Historical reviewers/art owners** decide authenticity, intellectual-property clearance, acceptable fictional changes and visual review—not metadata validators.
- **Build/release validation** only recognizes a completed asset when it exists in a compiled/imported Unreal project and has the required evidence.

See [Validation and Governance](VALIDATION_AND_GOVERNANCE.md) and [Performance and Release Gates](PERFORMANCE_AND_RELEASE_GATES.md).
