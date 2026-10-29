# POLITIKA — Documentation Index and Ownership

**Authority:** This file is for navigation and maintenance; it is not an additional game design, technical specification or release plan. Source files and committed assets alone do not prove runtime behavior; consult the scoped acceptance records for historically verified Linux Development builds and packages.

## Game design and canon

- [Concepts Outline](CONCEPTS_OUTLINE.md) — strategic faction identity, economy, combat, espionage and mechanics.
- [Story Outline](STORY_OUTLINE.md) — alternate-history assassination plot, nuclear storyline, campaign arc and political consequences.
- [Yugoslav World and Campaign Systems Model](YUGOSLAV_WORLD_MODEL.md) — translates the common RTS concepts into period institutions, physical geography, civilian life, provenance and the fixed storyline, distinguishing historical evidence from candidate gameplay.
- [Match Structure](MATCH_STRUCTURE.md) — match roster and faction participation boundaries.
- [Resource Economy](RESOURCE_ECONOMY.md), [Nuclear Weapons](NUCLEAR_WEAPONS_SYSTEM.md), [Sabotage](SABOTAGE_SYSTEM.md), [Reconnaissance](RECONNAISSANCE_AND_SCOUTING.md), [Intelligence](ESPIONAGE_AND_INTIMIDATION.md), [Counterintelligence](UNALIGNED_COUNTERINTELLIGENCE.md), [Propaganda and Morale](PROPAGANDA_AND_MORALE.md) — specialist gameplay rules and planned mechanics.
- [Faction descriptions](../Factions/) — faction-specific narratives and visual identity.
- [Proposed Five-Faction Unit and Building Rosters](POLITIKA_FIVE_FACTION_ROSTERS.md) — design-stage faction equipment and structure catalogue with original C&C inspirations (not implemented or locked).

## Engine and development

- [Test-map terrain and camera controls](TEST_MAP_TERRAIN.md) — grass/elevation provenance, material-only import, camera pan/zoom fixes and scoped package evidence.

- [Engine Outline](ENGINE_OUTLINE.md) — **the single authoritative implementation sequence**, including incomplete Stage 4A closure and the dependency-ordered First Playable Skirmish milestone; owns implementation order and promotion boundaries.
- [Prototype Gameplay Content Authoring](GAMEPLAY_CONTENT.md) — typed asset IDs, map schema v1, validator/tool usage, and known limits of the first content-driven RTS slice.
- [Rifle-infantry reference import](CNC3_RIFLEMEN_IMPORT.md) — neutral-named mesh/animation/MP3 import, source provenance, interrupted-package recovery and scoped local audio playtest evidence.
- [Stage 3 Data/Content/Map Acceptance](STAGE3_DATA_CONTENT_MAP_ACCEPTANCE.md) — scoped ST3-01–ST3-18 verdicts, exact evidence identities, and remaining acceptance gaps.
- [First RTS Vertical Slice Milestone](RTS_VERTICAL_SLICE_MILESTONE.md) — accepted scoped Linux Development prototype, validation evidence, remaining platform gaps and follow-on content/model work; not a finished RTS or release.
- [RTS Command and Simulation Contract — Milestone Record](RTS_COMMAND_SIMULATION_CONTRACT.md) — accepted scoped Linux command/admission/simulation contract with test matrices, packaged interaction/recovery, Shipping isolation and known platform limits.
- [RTS Runtime Navigation Acceptance](RTS_RUNTIME_NAVIGATION_ACCEPTANCE.md) — authoritative Recast movement, cooked-obstacle/repeatability and valid-route interruption qualification; source/package identities and bounded remaining gates.
- [Stage 4A Match Lifecycle Acceptance](STAGE4A_MATCH_LIFECYCLE_ACCEPTANCE.md) — bounded lifecycle qualification plus the 2026-10-29 integrated participant-authority/implemented-infantry Linux Development acceptance, historical authored-startup evidence, final source/package evidence location, and explicit remaining Stage 4A boundary.
- [RTS Game Ontology](RTS_GAME_ONTOLOGY.md) — foundational RTS categories, C&C and StarCraft comparison, and POLITIKA-specific distinctions between entities, agents, ownership, actions, resources, information, and objectives; conceptual reference, not an implementation claim.
- [RTS Systems Completeness Matrix](RTS_SYSTEMS_COMPLETENESS_MATRIX.md) — 29 October 2026 source/doc coverage audit, explicit missing contracts, recommended Unreal 5.8 framework choices and staged validation gates; **design**, not acceptance.
- [Practical RTS UI, Settings and Keymap](RTS_UI_SETTINGS_AND_KEYMAP.md) — actual current keyboard/mouse bindings, A/S conflict, windowed/fullscreen/resolution rollback, UI/text scaling, sound routing, menus, binding registry, other RTS interface precedents and test gates; proposed implementations.
- [USD Economy, Commodity Sales, Building Electricity and Military Food](RTS_OPERATIONAL_ECONOMY_AND_POWER.md) — adopted in-game US dollars ($) as primary purchasing resource, gold/fossil exchanges for dollars, minimum building power, dollar-purchased military food reserves and scoped accounting/shortage test contracts (not implemented).
- [Overlooked RTS Mechanics Review](RTS_OVERLOOKED_MECHANICS_REVIEW.md) — implementation gaps in onboarding, tactical command stance, communications, civilian/legitimacy consequences, infrastructure, pacing and accessibility; proposed, not implemented.
- [Authoritative Visibility and Intelligence](RTS_VISIBILITY_AND_INTELLIGENCE.md) — proposed per-participant knowledge, fog, stale sightings and AI/UI information authorization.
- [Selection, Group Commands and Queues](RTS_SELECTION_AND_GROUP_ORDERS.md) — proposed multi-selection, per-unit typed dispatch, stale identity cleanup and deferred control-group/queued-order semantics.
- [Structures, Production and Technology](RTS_PRODUCTION_AND_TECHNOLOGY.md) — proposed one-resource Stage 4B reinforcement flow, ledger, placement checks, producer queues and deferred prerequisite graphs.
- [Bounded Skirmish Opponent AI](RTS_OPPONENT_AI.md) — proposed deterministic, authority-respecting, observation-scoped Stage 4D opponent with acceptance tests.
- [Match Persistence, Replay and Determinism](RTS_MATCH_PERSISTENCE_AND_REPLAY.md) — proposed snapshot/replay ownership, compatibility, integrity and repeatability boundaries; **deferred** beyond first skirmish.
- [C&C Gameplay Comparative Study](CNC_GAMEPLAY_COMPARATIVE_STUDY.md) — cross-title design patterns and their POLITIKA-specific application; its local UM file-level reference catalog is at `/mnt/data/CommandAndConquer/UM/REFERENCE_CATALOG.md` (external to this repository).
- [Thyme Architecture Study](THYME_ARCHITECTURE_STUDY.md) — read-only patterns from a Generals: Zero Hour reimplementation, translated into POLITIKA-specific boundaries and current milestone guidance; no code reused.
- [C&C Visual/Audio Reference Study](ART_AUDIO_REFERENCE_STUDY.md) — observations from local texture, model, UI and sound references, with original POLITIKA concept and sound prototypes.
- [StarCraft Remastered Reference Analysis](STARCRAFT_REMASTERED_REFERENCE_ANALYSIS.md) — extraction-backed recommendations for POLITIKA command feedback, animation coverage, icon grammar, terrain authoring and data boundaries; assets remain reference-only.
- [RTS Command and Feedback Inventory](RTS_COMMAND_FEEDBACK_INVENTORY.md) — source-backed matrix for the five current prototype orders, their input/eligibility/feedback, and the next HUD design slice.
- [Shell Skeleton](SHELL_SKELETON.md) — implemented native offline shell, build/recovery contract and locally verified Linux package evidence; not RTS gameplay.
- [Proposed Engine Layers](PROPOSED_ENGINE_LAYERS.md) — authoritative state, simulation, UI, world and tooling boundaries.
- [Platform Support](PLATFORM_SUPPORT.md) — Linux, Windows, SteamOS/Steam Deck and Android plans; future platforms.
- [UI Overview](POLITIKA_UI_OVERVIEW.md) — main menu, faction HUD, commands, accessibility and input design.
- [Development Foundations](DEVELOPMENT_FOUNDATIONS.md) — implemented local Rust/C++ foundations and staged possibilities.
- [Validation and Governance](VALIDATION_AND_GOVERNANCE.md) — change-scope testing policy and document ownership.
- [Repository Implementation Audit — 29 October 2026](Reports/REPOSITORY_IMPLEMENTATION_AUDIT_20261029.md) — tracked-source inventory, verified versus unqualified milestones, prioritized risks, and Rust-free native core test fallback; an audit, not a new gameplay acceptance.
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
