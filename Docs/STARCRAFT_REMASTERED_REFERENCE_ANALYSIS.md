# StarCraft Remastered — POLITIKA reference analysis

**Status:** research guidance, 2026-10-27. This analyzes the supported extraction set and a bounded static Ghidra pass; it is not a behavioral audit, implementation, or approval to ship extracted game content. C&C4 remains POLITIKA's primary design/asset reference. SC:R is a secondary reference for classic 2D RTS presentation and content organization.

## Summary

The extraction is most useful for three bounded questions:

1. How can an RTS keep a large command vocabulary discoverable in a compact, consistent icon system?
2. How can animation frames, offsets, masks, and sprite metadata be represented as content rather than bespoke per-unit code?
3. How can terrain, unit states, and command feedback remain legible at strategic zoom?

It is not directly useful as POLITIKA production art: SC:R's sprites, iconography, tile art, interface graphics, recordings, and game-specific tables are proprietary and sci-fi themed. POLITIKA is a 1980s Yugoslav/Balkan conflict strategy game and uses UE 5.8.3; SC:R's extracted 2D assets do not establish an appropriate 3D art pipeline, historical identity, gameplay balance, or UE behavior.

## Evidence inspected

The validated run contains 3,715 files (~3.1 GiB) across nine requested categories. It is a curated, supported extraction—not the complete 21,030-entry CASC archive. All outputs had SHA-256 entries and passed the recorded verification. No game code was executed.

| Extracted evidence | Observation | What it can inform | What it cannot establish |
|---|---|---|---|
| `command-icons/cmdicons.json` and its atlas | 390 slots at 128×128; 30 named command-button mappings include Move, Stop, Patrol, AttackMove, HoldPosition, Cancel, plus ability/upgrade examples. The atlas is 2176×2944. | A semantic ID-to-icon mapping, fixed source cells, consistent command families, missing/unused slot accounting, and a compact HUD icon audit. | That POLITIKA needs 390 actions, this icon style, or the source artwork. The atlas includes non-command images and blank slots. |
| HD4 animation outputs | 866 ANIM files, 1,023 PNGs (including team-color masks), and 868 JSON metadata files. A sample metadata record (`main_119.json`) has frame count, atlas dimensions, pixels-per-unit, frame rectangles, and per-frame offsets. | A content-import schema: stable asset ID, frame bounds, pivot/offset, cadence, render scale, optional mask, and explicit state/variant links. It gives a concrete example of separating frame data from rendered pixels. | That the extractor's numeric `main_N` names are meaningful POLITIKA IDs or that SC:R's proprietary animation bytecode should become a runtime dependency. |
| SD animation set | 868 PNGs and `mainSD.anim`. | The existence of alternate-resolution asset sets raises a useful resolution/LOD question for 2D content. | That two manually maintained resolutions are the right strategy for UE 3D assets or POLITIKA's 4K target. |
| Eight HD4 tilesets | Eight source VR4 files and PNG conversions (e.g. `jungle.dds.png` is 128×128). | How a 2D RTS groups terrain families and their tile-source data; useful when creating a small, cohesive original POLITIKA terrain test kit. | Terrain scale, material behavior, or map semantics in POLITIKA. PNG conversion alone does not expose all original encoding/shader intent. |
| 11 game data tables | Eight DATs, two TBLs, and `iscript.bin`, covering categories including units, weapons, orders, upgrades, sprites/images, and text. | A reminder to keep authored definitions, localization, and presentation references separately inspectable and versioned; relate a unit's capabilities to its orders and presentation through stable IDs. | A complete design spec, safe/portable schema, or balance values suitable for copying. These are game-specific binaries and tables. |
| Command cursors, menu media, effects, sounds | 18 cursor strips, 25 main-menu media files, two GRP effects, and 13 of 15 requested OGG targets. | Reference for maintaining semantic states across cursor, HUD, animation, and audio feedback. The incomplete sound set warns against treating this as a complete sound library. | Full audio coverage, mix quality, menu interaction behavior, or what was omitted from the extraction. |
| Ghidra analysis of x86 and x64 `StarCraft.exe` | Both installed PE binaries are imported into the saved Ghidra project. x86 auto-analysis and save completed with numerous diagnostics; x64 remained incomplete after a 600-second timeout. No function-by-function semantic review was performed. | A reusable local workspace for a narrowly scoped follow-up question about a specific binary/data relationship, if corroborated against extracted content. | Gameplay architecture, runtime behavior, performance, module loading, reliable complexity counts, or portable implementation guidance. Analyzer counts and anomalous address-space bounds are not design evidence. |

Source of truth for exact paths, hashes, extraction limitations, and category logs: the [run manifest](</mnt/data/CommandAndConquer/UM/StarcraftRemastered/extracted/2026-10-27/extraction-manifest.md>) and [UM reference landing page](</mnt/data/CommandAndConquer/UM/StarcraftRemastered/extraction-manifest.md>). Ghidra import scope, diagnostics, and hashes: [surface inspection](</mnt/data/CommandAndConquer/GHIDRA/StarcraftRemastered/surface-inspection.md>).

## Applications to POLITIKA

### 1. Turn the existing command contract into a feedback contract

POLITIKA already routes typed `CommandRequest`s through coordinator admission; rejected requests do not change orders. The next useful SC:R-derived design exercise is not adding commands—it is making the result readable. Define an original POLITIKA command-icon catalogue keyed by the existing order/ability IDs, with fields for label/localization key, icon asset, availability state, input prompt, and feedback event. For each command, specify ready, unavailable, queued/accepted, and rejected presentation states. Keep the icon mapping in presentation/content data; the simulation remains authoritative.

An initial prototype should cover only current commands (Move, Attack, Stop, Guard, Attack-Move) plus a rejected-target case. Review them at 4K, 1080p, and a compact handheld layout with keyboard/mouse, controller, and touch-safe hit targets in mind. Use silhouette and label/tooltip to reinforce meaning; do not encode meaning by color alone. This complements [the RTS command/simulation contract](RTS_COMMAND_SIMULATION_CONTRACT.md) and [UI overview](POLITIKA_UI_OVERVIEW.md).

### 2. Add animation-state coverage to the unit presentation spec

Use the extracted metadata only as a schema example. For each original POLITIKA unit archetype, list the visual states needed to communicate orders and combat: idle, moving, turning, firing, suppressed, damaged, disabled/retreating, destroyed, and selected. A per-state coverage sheet can expose missing feedback before a full art set exists. For UE skeletal/3D actors, map these to UE animation assets, state machines/montages, material parameters, and gameplay tags as appropriate; do not reproduce the SC:R atlas/ANIM pipeline by default.

For a later 2D minimap/briefing/icon need, the JSON layout offers a useful import-data shape (frame rectangles plus offsets/pivots). A small Rust offline importer/validator is only justified if POLITIKA actually adopts original sprite atlases; it should produce owned UE-importable content and stay outside runtime. Avoid shipping the original SC:R extracted images or binary format readers in the game.

### 3. Build an original tactical icon grammar, not a copied atlas

Use the 390-slot atlas as a density/readability stress reference: how many distinct concepts can be scanned, which icons are ambiguous, and how blank/disabled states read. Translate this into a much smaller POLITIKA set grounded in its own design: movement, engagement, hold/defend, reconnaissance, repair, suppression/morale, objective interaction, and faction-specific orders only where their mechanics differ. The visual language should be original and period-appropriate—maps, radio/rank markings, stencils, field signs, and equipment silhouettes—subject to historical research and faction identity review.

Build the catalogue from stable POLITIKA IDs rather than atlas positions or filenames. Keep a source/provenance field and lint missing icon references, duplicate IDs, and unavailable-state variants in the owned content pipeline. Reuse the interface principles, not StarCraft unit silhouettes, symbols, logos, or art.

### 4. Use tile sets to improve POLITIKA terrain composition tests

The extracted eight terrain families suggest checking whether each POLITIKA theater has enough visual variation to communicate route, cover, elevation, and objective area at gameplay camera scale. This should produce an original terrain kit—e.g. Adriatic/coastal, Dinaric highland, Pannonian plain, and industrial/urban surfaces—with roads, fields, forest edges, concrete, and damage states. The SC:R source tiles are not a substitute for historical map research, landscape/material authoring, or a 3D UE landscape workflow.

### 5. Preserve the data boundary already proposed for POLITIKA

The collection of `units`, `weapons`, `orders`, `upgrades`, `images`, and text tables is a useful prompt to check cross-references in POLITIKA's own content definitions. It supports the existing `UPrimaryDataAsset`/DataTable direction, not importing these tables or mirroring their exact schema. POLITIKA IDs, versioning, validation, authored values, and localization must be independently owned. Keep per-instance simulation state out of definitions and keep UI/animation/audio as observers of simulation events, as specified in [the engine outline](ENGINE_OUTLINE.md).

### 6. Treat Ghidra as a targeted lookup tool, not a source of design conclusions

The Ghidra result is a persistent analysis workspace, not a recovered StarCraft architecture. The x86 analysis completed but emitted many disassembly, RTTI, p-code, and decompiler diagnostics; x64 remained partial even after 600 seconds. The reported function/symbol counts and address ranges therefore should not be used to estimate complexity or infer subsystem boundaries. No runtime behavior, active expansion/module relationship, or function semantics have been established. See the [Ghidra surface report](</mnt/data/CommandAndConquer/GHIDRA/StarcraftRemastered/surface-inspection.md>) for exact scope and limitations.

For POLITIKA, the actionable code-level reference is its own verified boundary: typed command requests are admitted by the coordinator, typed gameplay events are published through a native C++ delegate, and stable-ID content assets are validated on the POLITIKA side. The HUD is still not an event subscriber and shows a last-feedback string. The productive next step is to connect presentation to those existing typed events and build the original five-command panel described in the [command feedback inventory](RTS_COMMAND_FEEDBACK_INVENTORY.md), rather than reconstruct undocumented SC:R internals. If a future Ghidra question is worth pursuing, scope it to a named behavior or data relationship, inspect only relevant functions, and corroborate with extracted assets; do not infer module loading from the uninspected `1911.dll`.

## Recommended work order

1. **Now — bounded implementation:** implement the original five-command placeholder panel and wire its presentation to the existing typed event delegate; preserve admission authority in the coordinator and show correlated accepted/rejected outcomes. The inventory and current HUD gap are already documented; no reference art needs to enter the project.
2. **Next — content review:** produce one unit-state coverage sheet and review current prototype entities against it. Prioritize only states that communicate order, combat, or morale.
3. **Next art slice:** author a small, original terrain composition and command-icon sample; judge both at real gameplay zoom and across the project's target UI scales.
4. **Later, gated by need:** prototype an Unreal animation/data import path only if the actual POLITIKA production content uses sprite atlases. Otherwise retain UE's native 3D asset path.
5. **Do not schedule:** porting SC:R data tables, sprite binaries, icons, audio, or balance values into POLITIKA. They are reference-only and technically mismatched to the project's design and engine boundaries.

## Limits and open questions

- This pass used extraction metadata and representative output, not a complete visual/audio review of all 3,715 files.
- The extracted animations are primarily sprite/atlas-oriented; no SC:R-to-POLITIKA animation importer or UE asset compatibility has been demonstrated.
- The DAT/TBL/IScript data was extracted but not semantically decoded into a full relational schema here. Do not infer mechanics or balance from filenames alone.
- Only supported requested asset families were extracted. Missing sound targets are recorded in the manifest; this is not evidence they are absent from the installed game.
- Universal Modder extraction, REA surface inspection, and this Ghidra pass do not establish runtime UX or gameplay behavior. Ghidra's x64 analysis is partial, and the completed x86 analysis has diagnostics; no semantic function review was performed.

## Source and provenance

All inspected files remain in `/mnt/data/CommandAndConquer/UM/StarcraftRemastered/extracted/2026-10-27/`; no SC:R game assets were added to POLITIKA's content tree. The installation and extraction provenance is in the linked manifest. This report is design/technical research and does not grant a license to redistribute or ship the extracted proprietary assets.
