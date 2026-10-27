# POLITIKA — Documentation Index and Ownership

**Authority:** This file is for navigation and maintenance; it is not an additional game design, technical specification or release plan. Files and code currently present are not evidence of a running Unreal game.

## Game design and canon

- [Concepts Outline](CONCEPTS_OUTLINE.md) — strategic faction identity, economy, combat, espionage and mechanics.
- [Story Outline](STORY_OUTLINE.md) — alternate-history assassination plot, nuclear storyline, campaign arc and political consequences.
- [Match Structure](MATCH_STRUCTURE.md) — match roster and faction participation boundaries.
- [Resource Economy](RESOURCE_ECONOMY.md), [Nuclear Weapons](NUCLEAR_WEAPONS_SYSTEM.md), [Sabotage](SABOTAGE_SYSTEM.md), [Reconnaissance](RECONNAISSANCE_AND_SCOUTING.md), [Intelligence](ESPIONAGE_AND_INTIMIDATION.md), [Counterintelligence](UNALIGNED_COUNTERINTELLIGENCE.md), [Propaganda and Morale](PROPAGANDA_AND_MORALE.md) — specialist gameplay rules and planned mechanics.
- [Faction descriptions](../Factions/) — faction-specific narratives and visual identity.
- [Proposed Five-Faction Unit and Building Rosters](POLITIKA_FIVE_FACTION_ROSTERS.md) — design-stage faction equipment and structure catalogue with original C&C inspirations (not implemented or locked).

## Engine and development

- [Engine Outline](ENGINE_OUTLINE.md) — system responsibilities and implementation direction.
- [Prototype Gameplay Content Authoring](GAMEPLAY_CONTENT.md) — typed asset IDs, map schema v1, validator/tool usage, and known limits of the first content-driven RTS slice.
- [First RTS Vertical Slice — Milestone Record](RTS_VERTICAL_SLICE_MILESTONE.md) — bounded prototype scope and completed Linux Development acceptance evidence; not a complete skirmish or release qualification.
- [RTS Command and Simulation Contract — Milestone Record](RTS_COMMAND_SIMULATION_CONTRACT.md) — typed command intake consolidation, admission contract, and verified Linux Development evidence with remaining acceptance gaps.
- [C&C Gameplay Comparative Study](CNC_GAMEPLAY_COMPARATIVE_STUDY.md) — cross-title design patterns and their POLITIKA-specific application; it references the local UM file-level catalog at `/mnt/data/CommandAndConquer/UM/REFERENCE_CATALOG.md`.
- [Thyme Architecture Study](THYME_ARCHITECTURE_STUDY.md) — read-only patterns from a Generals: Zero Hour reimplementation, translated into POLITIKA-specific boundaries and current milestone guidance; no code reused.
- [C&C Visual/Audio Reference Study](ART_AUDIO_REFERENCE_STUDY.md) — observations from local texture, model, UI and sound references, with original POLITIKA concept and sound prototypes.
- [StarCraft Remastered Reference Analysis](STARCRAFT_REMASTERED_REFERENCE_ANALYSIS.md) — extraction-backed recommendations for POLITIKA command feedback, animation coverage, icon grammar, terrain authoring and data boundaries; assets remain reference-only.
- [RTS Command and Feedback Inventory](RTS_COMMAND_FEEDBACK_INVENTORY.md) — source-backed matrix for the five current prototype orders, their input/eligibility/feedback, and the next HUD design slice.
- [Shell Skeleton](SHELL_SKELETON.md) — implemented native offline shell, build/recovery contract and locally verified Linux package evidence.
- [Proposed Engine Layers](PROPOSED_ENGINE_LAYERS.md) — authoritative state, simulation, UI, world and tooling boundaries.
- [Platform Support](PLATFORM_SUPPORT.md) — Linux, Windows, SteamOS/Steam Deck and Android plans; future platforms.
- [UI Overview](POLITIKA_UI_OVERVIEW.md) — main menu, faction HUD, commands, accessibility and input design.
- [Development Foundations](DEVELOPMENT_FOUNDATIONS.md) — implemented local Rust/C++ foundations and staged possibilities.
- [Validation and Governance](VALIDATION_AND_GOVERNANCE.md) — change-scope testing policy and document ownership.
- [Versioning and Release Management](VERSIONING_RELEASE_MANAGEMENT_ROADMAP.md) — canonical build/game version policy.
- [Performance and Release Gates](PERFORMANCE_AND_RELEASE_GATES.md) — planned measurable RTS fixtures and release acceptance.

## Historical and media production

- [Historical Reference Archive](../Archive/README.md) — research programmes and catalogues.
- [Political history and chronology](../Archive/HISTORICAL_CHRONOLOGY_1980_1989.md), [Political Events Catalogue](../Archive/POLITICAL_EVENTS.csv) — factual historical references.
- [Map research and sources](../Archive/MAPS_1980_1989.md), [Map Catalogue](../Archive/MAP_CATALOG.csv) — dated/uncertain cartographic references.
- [Archive Rights and Provenance](../Archive/RIGHTS_AND_PROVENANCE.md) — licenses, attribution and evidence-versus-asset distinction.
- [Production Standards](PRODUCTION_STANDARDS.md) — period decisions, source-to-map lineage, RTS model and visual/audio review.

## Machine-readable design-stage registries

- [Faction content registry](../Data/content_manifest.json)
- [Historical decisions ledger](../Data/historical_decisions.json)
- [Map production sources](../Data/map_production.json)
- [Audio event catalogue](../Data/audio_events.json)
- [Performance scenarios](../Data/performance_cases.json)
- [Release readiness gates](../Data/release_matrix.json)

The register entries marked `planned`, `research_only` or `blocked` are not shipped assets, completed tests, historical certifications or releases.

## Maintenance rule

**Code and verified tests** prove what is implemented. **Game-design owning documents** define intended gameplay. **Archive catalogues** provide documented historical evidence, not automatic production rights. **Validation and production policy** define acceptance methods. **README** is the accessible public summary and links into these owners. Keep each rule in exactly one authoritative location; add links rather than duplicate paragraphs in several roadmaps.
