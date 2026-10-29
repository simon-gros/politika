# POLITIKA — RTS Asset Resource Research and Sourcing Recommendations

**Research date:** 2026-10-29  
**Status:** Research/evaluation plus selected CC0 source-asset acquisitions; no assets purchased, imported into Unreal, cooked, or integrated
**Project:** [POLITIKA](https://github.com/simon-gros/politika) — alternate-history real-time strategy game set in Yugoslavia, 1984–1989  
**Target engine:** Unreal Engine 5.8.3  
**Planned platforms:** Linux, Windows, SteamOS/Steam Deck, Android (not all qualified or released)

## Executive assessment

POLITIKA can acquire much of its environmental art, landscape materials, civilian scenery, selected vehicles, animations, and sound effects from existing libraries. The most sustainable approach is a **hybrid art pipeline**: prioritize openly redistributable assets for publicly versioned source material, evaluate commercial packs for specific gaps, and create original faction-specific infantry, insignia, installations, and signature vehicles.

The immediate opportunity is to improve the existing bounded RTS prototype's visual presentation without modifying authoritative gameplay, unit IDs, movement profiles, combat calculations, objective behavior, or platform-support claims.

**Leading recommendations:** [Poly Haven](https://polyhaven.com) and [ambientCG](https://ambientcg.com) for realistic environment materials; [Kenney](https://kenney.nl/assets) and selected [KayKit](https://kaylousberg.itch.io/city-builder-bits) packs for freely redistributable rapid prototypes; Eastern European building packs on [Fab](https://www.fab.com) for later, license-cleared production art. Use third-party military units only selectively because POLITIKA's distinct faction identities should be visible in gameplay.

## 1. Free and open asset resources

| Library | Candidate material | Value for POLITIKA | License / integration caveat |
| --- | --- | --- | --- |
| [Poly Haven](https://polyhaven.com) | Realistic 3D objects, HDRIs, PBR textures, terrain and rock surfaces | **Very high:** ground, rocks, outdoor scenery, realistic rendering baseline | [CC0](https://polyhaven.com/license); record attribution/provenance even though attribution is not required |
| [ambientCG](https://ambientcg.com) | Concrete, asphalt, brick, metal, soil, industrial and architectural PBR surfaces | **Very high:** terrain and period-appropriate architecture material library | CC0 offerings; check each selected asset and preserve metadata |
| [Kenney](https://kenney.nl/assets) | Modular 3D urban and industrial pieces, UI assets, sound packs | **High for prototyping:** readable, compact RTS fixtures | Many packs CC0; confirm individual pack license and visual consistency |
| [KayKit City Builder Bits](https://kaylousberg.itch.io/city-builder-bits) | Optimized stylized buildings, streets and urban props | **Good for low-cost map prototypes** and lower-spec hardware testing | Verify the individual download license and format before adding to the public tree |
| [OpenGameArt](https://opengameart.org/content/3d-assets-cc0) | Searchable selection of military, building and environmental 3D models | **Variable:** potentially useful one-off models | Check **per-file** license and authorship; quality and rights are not consistent across the whole site |
| [Quaternius](https://quaternius.com) | Animated vehicles, humanoids, weapons, nature, animation collections | **Good for rapid experiments** with readable, inexpensive models | **License caution:** current [QAL](https://quaternius.com/license.html) restricts asset redistribution; some older assets may have different licensing; verify the exact pack/version acquired |

**Quaternius clarification (October 2026):** the Quaternius Asset License v1.0 page identifies an update date of **2026-08-28** and restricts redistribution of asset files as assets, while allowing incorporation into completed products. Older CC0 statements may remain elsewhere on the site. Save a license snapshot tied to the specific acquired file; do not assume everything on the site can be committed to a public GitHub repository.

### Free-source priorities

1. Establish terrain and civilian material samples from Poly Haven and ambientCG.
2. Add a handful of license-verified, low-cost industrial or settlement props to an isolated prototype fixture.
3. Avoid mixing the visual languages of photorealistic, low-poly, and highly stylized packs in the same production scene without a deliberate art-direction review.
4. Preserve original author, download URL, date, license, original filenames, texture resolution and mesh statistics.

## 2. Professional and marketplace pack candidates

These are **candidates for inspection**, not approved production dependencies. Listings, prices, file coverage and engine support may change.

| Pack | Link | Observed offering / fit | Priority and caveats |
| --- | --- | --- | --- |
| **Soviet Panels — Eastern European Buildings Kit** | [Fab listing](https://www.fab.com/listings/e0fb40c4-05a7-4e49-9fd6-a10bbcca0ef8) | Eastern European apartment blocks, roads, civilian vehicles, streetscape and vegetation; listed for Unreal/Blender/Unity | **High:** Yugoslav civilian maps and Unaligned settings; review regional historical fidelity and source-file rights |
| **Extensive Modular Kit Soviet 1960s #1** | [Fab listing](https://www.fab.com/listings/c9577a24-73af-4809-9cee-483c5763d566) | Large photorealistic modular residential environment; listing describes 115 components, 60 premade building sections and 24 houses | **High:** dense urban districts; assess triangle/texture budgets and repetitiveness at RTS scale |
| **Eastern European Buildings Pack — Large** | [Fab listing](https://www.fab.com/listings/24cc006a-3a1b-4a7e-b0ff-3040d8949e55) | Listing describes 22 mid-poly building/modular assets and PBR textures | **Medium–high:** secondary settlements; listing indicates no collision and one LOD, so import needs further work |
| **T-54/T-55 Soviet Main Battle Tank** | [Fab listing](https://www.fab.com/listings/c30b308b-820c-471b-ab1e-287a836386db) | Period-relevant armored vehicle; listing describes about 28,851 triangles and separately modeled objects | **Medium:** reference or first Cold War vehicle test; check rig, pivot points, LODs, collision, platform budget and license |
| **Synty POLYGON — Military Pack** | [Fab listing](https://www.fab.com/listings/c2723e9a-5a11-4dba-a1a2-74b5426c97a0) | Large cohesive stylized military collection; publisher describes 1,500+ prefabs/models | **Medium for temporary test coverage; low as final art direction** unless POLITIKA adopts this style; check animation exclusions and current price |

**Historical-art caution:** Soviet-era architecture is relevant as a source of broad construction patterns, but **Yugoslav cities are not interchangeable with Soviet cities**. Use region-specific roofs, signage, roads, rail infrastructure, housing, public institutions, and civil/industrial facilities. Exclude anachronistic post-1989 objects from supposedly contemporary 1984–1989 maps.

**Cost caveat:** Specific commercial prices have not been validated as fixed/current prices in this report. Obtain a live quote and license review before purchase.

## 3. Animation and sound resources

| Resource | URL | Proposed use | Licensing note |
| --- | --- | --- | --- |
| Adobe Mixamo | https://www.mixamo.com | Humanoid rigging and walk/run/idle/combat/death animations for the first infantry test | Read current Adobe terms; redistribution of raw character or animation files is distinct from inclusion in a finished game |
| Quaternius Universal Animation Library | https://quaternius.com/packs/universalanimationlibrary.html | Directional movement, armed poses, crawling and death sequences | Pack-specific version/license check required; see QAL caveat |
| Sonniss GDC sound bundle | https://gdc.sonniss.com/ | Professional effects for firearms, vehicles, ambient conditions and battlefield presentation | Read the edition-specific production license; avoid publishing standalone licensed audio files |
| Kenney Impact Sounds | https://kenney.nl/assets/impact-sounds | Interim hit, impact and command-feedback audio | Verify the pack's CC0 terms |
| Freesound | https://freesound.org | Radio static, industrial environments, period-like mechanical and general sound recordings | Filter and audit each file's CC0 / CC BY / other license separately |

Prioritize unit acknowledgements, tracked vehicle movement, combustion engines, weapon impacts, distant artillery, radio communication, surveillance audio, warning sounds, and quiet civilian ambience. Sound assets must remain presentation-only; game rules and event timing are authoritative in C++.

## 4. Faction-based asset sourcing

| Faction | Asset types to prioritize | Distinctive art work to retain in-house |
| --- | --- | --- |
| **Atlantic Concord** | Western Cold War transport, airfields, radar installations, military roads, modernized infrastructure | Distinctive uniforms, markings, special units and faction-specific command buildings |
| **Red Directorate** | Eastern Bloc tanks, artillery, heavy trucks, depots, industrial installations and barracks | Faction-specific armor silhouettes, insignia, weapons/production facilities |
| **Iron Ascendancy** | Fortifications, covert installations and conventional hardware suitable for modification | Original uniforms, emblems, asymmetric/specialist troops and covert-operation structures |
| **Meridian Pact** | Civilian trucks, technicals, mobile camps, markets, cargo equipment, workshops and temporary depots | Unique sabotage, mercenary, trade and procurement visual identity |
| **The Unaligned** (NPC/system AI) | Yugoslav municipal and military infrastructure, police, JNA-era vehicles, railways, bridges, residential and industrial districts | Region-specific historical details, federal/local administration, original units and recognizable Yugoslav setting |

The Unaligned is particularly well positioned to benefit from environmental sourcing: its civilian world-building requires a broad variety of period-appropriate everyday objects rather than only a combat roster.

## 5. Licensing and public-repository policy

POLITIKA is open source. **A license permitting assets to appear in a shipped game does not necessarily permit the same assets to be placed in a public source repository.**

- **CC0:** Generally appropriate for redistribution, modification and public GitHub source inclusion. Preserve provenance voluntarily.
- **CC BY:** Potentially suitable with required attribution and license compliance; verify whether the selected license is compatible with the intended source and distribution workflows.
- **Fab Standard License:** Allows licensed asset use and modification within projects and commercial distribution of projects incorporating assets; restricts standalone asset redistribution. Sharing with collaborators through approved private workflows is not equivalent to publishing unrestricted source assets.
- **Quaternius QAL:** Permits incorporation in games, but restricts redistribution of asset files, including modified assets, as assets. The precise asset/version/license must be proven.
- **Mixamo, Sonniss and other product licenses:** Validate the actual terms, download edition, and permitted sharing arrangements before any source commit or release.
- **Rights uncertainty:** If origin, rights, or the redistribution allowance cannot be established, do not import the file into the public repository.

Primary license references: [Poly Haven](https://polyhaven.com/license), [Fab EULA](https://www.fab.com/eula?lang=en), [Quaternius QAL](https://quaternius.com/license.html).

### Recommended asset provenance manifest

Track each adopted third-party asset with at least the following:

| Field | Meaning |
| --- | --- |
| Stable asset identifier | Project-owned identity independent of vendor filename |
| Vendor / creator / source URL | Original provenance |
| Exact release and retrieval date | Reproducibility of rights and asset bytes |
| License name, version and captured text/link | Auditable redistribution decision |
| Public source redistribution permitted? | Explicit yes / no / unresolved |
| Attribution / notices required | Credits and license obligations |
| Original format, imported format, hashes | Reproducible asset pipeline |
| Material/texture and mesh specifications | Resource and memory budgets |
| Rig / animations / colliders / LODs | Runtime integration requirements |
| Supported platforms and test evidence | Actual packaging and performance evidence |
| Transformation and original-vs-derived relationship | Traceability of modifications |

Use a restricted local/authorized dependency workflow for proprietary assets unless an explicit redistribution grant permits public hosting. A public README or manifest may **link** to restricted assets without including their binaries.

## 6. Unreal integration and acceptance requirements

POLITIKA's documented engine architecture separates authored definitions and visual representations from authoritative gameplay state; maintain that boundary when importing meshes and animation.

### Import and presentation standards

1. **Formats:** prefer documented FBX / glTF / Blender export workflows supported by the actual Unreal Engine build. Check axis, scale, orientation, pivots, naming and texture channel packing.
2. **Definition identity:** associate imported visuals with existing stable-ID game definitions without treating names or mesh file paths as gameplay identity.
3. **Character rigs:** establish compatible skeleton, animation retargeting, locomotion cycles, weapon attachment points and clear death presentation.
4. **Vehicles:** verify forward vector, center and collision, hull/turret independence, ground contact, weapon muzzle points and animation.
5. **Map geometry:** provide simple collision and intentional navigation passability/blockers. Decorative meshes may not silently alter map acceptance or valid starts.
6. **Performance:** evaluate draw calls, material slots, instance batching, texture pool, LODs, silhouette readability, shadow and material cost at real RTS camera distances.
7. **Multiplatform:** retain reduced-detail variants for Steam Deck and Android; do not assume desktop Nanite rendering or 4K texture budgets transfer to mobile.
8. **Licensing:** block unapproved source redistribution before import/commit. Keep third-party license notices, provenance and asset hashes.
9. **Verification:** test cooked Linux Development builds first, then expand platform claims only after device/packaged acceptance. Verify selection, hover, movement, combat, destruction and objective behavior unchanged.

### Clearance versus playtest-qualification boundary

The implemented `RifleInfantry` is technically useful as a controlled local reference for preserving selection, orders, animation and packaged playback, but [its provenance](CNC3_RIFLEMEN_IMPORT.md) documents external commercial game mesh/audio conversion. **Technical acceptance is not permission to redistribute** the derived source, committed intermediates or cooked media. For a publicly downloadable playable build, require documented rights-holder authorization or an original/independently cleared replacement with comparable rig, motion and audio integration tests; verify no restricted assets remain in release packages. The [archive rights policy](../Archive/RIGHTS_AND_PROVENANCE.md#repository-wide-copyright-and-distribution-separation-29-october-2026) and [production standards](PRODUCTION_STANDARDS.md#asset-intake-technical-qualification-and-release-clearance) own the checks. No asset removal or implementation change is performed by this documentation update.

## 7. Suggested bounded first milestone: environment and unit visual integration

**Objective:** raise the visual quality of the already accepted RTS slice without expanding the gameplay systems or faction rosters.

1. **Environment sample:** import a small number of approved CC0 asphalt, ground, concrete and industrial materials, plus a few civilian props. Validate mesh collisions and static navigation obstacles on the existing test map.
2. **Infantry sample:** retain the implemented `RifleInfantry` reference prototype and its [accepted import/audio workflow](CNC3_RIFLEMEN_IMPORT.md). The `AC-INF-01` faction concept remains a future production-art reference; its superseded static prototype is removed. Any future production replacement needs separate approval and idle, move, attack and death validation.
3. **Vehicle sample:** evaluate an era-appropriate tank, such as the T-55, in an isolated presentation test with correct hull, turret, collision and scalable LOD representation.
4. **Quality gate:** validate content provenance, UE import, cooked package inclusion, visual legibility, render/CPU/memory costs, and no regressions in selection, orders, navigation, combat, damage/death or objective outcomes.
5. **Review gate:** record screenshots or comparable acceptance evidence at representative desktop RTS zoom levels; test lower-tier presentation rather than projecting desktop quality assumptions onto Android.

### Concrete deliverables for a later implementation task

- Small, explicitly approved asset candidate list with per-file license records.
- An asset provenance manifest (and restricted asset exclusion rules).
- Imported materials and sample assets bound only to presentation definitions.
- Automated/editor validation for missing meshes, materials, collision and required import metadata where practical.
- Repeatable cooked-build and runtime visual checks, with platform limitations clearly recorded.

**Out of scope for this milestone:** acquisition of an entire paid military pack; complete faction rosters; wholesale map replacement; new economy, AI or espionage functionality; unverified claims of Android/Steam Deck support; bulk conversion of proprietary asset collections.

## 8. Decision summary

**Adopt first:** Poly Haven and ambientCG for realistic environmental materials; Kenney and individually verified KayKit/CC0 content for inexpensive prototype fixtures.

**Inspect for production:** Soviet Panels, Extensive Modular Kit Soviet 1960s and Eastern European Buildings Pack for historically inspired civilian maps. Buy or import only after a historical-art, performance, and licensing review.

**Use selectively:** third-party T-54/T-55 assets, Mixamo animations, Sonniss effects, and Quaternius components where the actual license and technical fit are acceptable.

**Produce as original POLITIKA art:** recognizable infantry silhouettes, faction-specific vehicles and installations, insignia, specialist abilities and important campaign landmarks.

The objective is a distinctive, period-sensitive RTS visual identity with reproducible provenance, open-source compatibility, and realistic desktop/handheld/mobile performance budgets.

## 9. Local CC0 acquisition record — 2026-10-29

Selected source assets have been downloaded into the project for later art-direction and Unreal import review. They are not yet engine content or gameplay dependencies.

| Source set | Acquired assets | Local provenance record |
| --- | --- | --- |
| Poly Haven props | Concrete Road Barrier, Barrel 03 | [`Art/ThirdParty/PolyHaven-Props-20261029`](../Art/ThirdParty/PolyHaven-Props-20261029/README.md) |
| Poly Haven props (additional) | Old Military Crate, Metal Jerrycan Green, Modular Chainlink Fence, Modular Electricity Poles | [`Art/ThirdParty/PolyHaven-Props-Additional-20261029`](../Art/ThirdParty/PolyHaven-Props-Additional-20261029/README.md) |
| ambientCG materials | Asphalt012, Concrete035, Metal009 | [`Art/ThirdParty/ambientCG-UrbanMaterials-20261029`](../Art/ThirdParty/ambientCG-UrbanMaterials-20261029/README.md) |
| Poly Haven / ambientCG environment | Boulder 01, Shrub 01, Bricks 105 | [`Art/ThirdParty/Environment-Assets-20261029`](../Art/ThirdParty/Environment-Assets-20261029/README.md) |
| Poly Haven settlement assets | Modular Urban Apartments Facade, Modular Street Seating | [`Art/ThirdParty/PolyHaven-Settlement-Assets-20261029`](../Art/ThirdParty/PolyHaven-Settlement-Assets-20261029/README.md) |
| Poly Haven / ambientCG industrial assets | Modular Factory Facade, Metal 063 | [`Art/ThirdParty/Industrial-Assets-20261029`](../Art/ThirdParty/Industrial-Assets-20261029/README.md) |
| Poly Haven infrastructure | Modular Industrial Pipes 01, Old Military Compressor, Large Iron Gate | [`Art/ThirdParty/PolyHaven-Infrastructure-20261029`](../Art/ThirdParty/PolyHaven-Infrastructure-20261029/README.md) |

All selected assets above are recorded as CC0 by their publishers. Each acquisition folder preserves the applicable license page, source URLs/API metadata, and SHA-256 checksums. Treat them as generic environment/prop materials, not proof of Yugoslav historical accuracy. Verify import, asset scale, collisions, RTS-camera readability, and platform budgets before integration.

---

## Source and project references

- [POLITIKA README](../README.md)
- [POLITIKA Engine Outline](ENGINE_OUTLINE.md)
- [POLITIKA UI Overview](POLITIKA_UI_OVERVIEW.md)
- [POLITIKA Platform Support](PLATFORM_SUPPORT.md)
- [POLITIKA Concepts Outline](CONCEPTS_OUTLINE.md)
- [Poly Haven](https://polyhaven.com)
- [Poly Haven license](https://polyhaven.com/license)
- [ambientCG](https://ambientcg.com)
- [Kenney assets](https://kenney.nl/assets)
- [KayKit City Builder Bits](https://kaylousberg.itch.io/city-builder-bits)
- [OpenGameArt CC0 3D assets](https://opengameart.org/content/3d-assets-cc0)
- [Quaternius](https://quaternius.com) / [QAL](https://quaternius.com/license.html)
- [Fab Standard License](https://www.fab.com/eula?lang=en)
- The five linked Fab pack listings in Section 2
- [Adobe Mixamo](https://www.mixamo.com)
- [Sonniss](https://gdc.sonniss.com/)
- [Freesound](https://freesound.org)

**Research limitation:** this is a curated sourcing report, not an asset audit, proof of UE 5.8.3 compatibility, vendor procurement approval or legal opinion. Prices, asset counts, pack contents and license terms should be rechecked at acquisition time.
