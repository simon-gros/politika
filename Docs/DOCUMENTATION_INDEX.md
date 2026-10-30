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

**Development priority (29 October 2026):** Multiplayer is an **active near-term engineering track**, not an optional post-skirmish extension. The single [Engine Outline](ENGINE_OUTLINE.md) schedules it in parallel with Stage 4; the [Multiplayer Architecture](RTS_MULTIPLAYER_ARCHITECTURE.md) owns its mandatory pre-implementation code audit and independent gates. Historical qualification reports remain historically accurate and do not imply a working network build.

## Engine and development

- [Test-map terrain and camera controls](TEST_MAP_TERRAIN.md) — grass/elevation provenance, material-only import, camera pan/zoom fixes and scoped package evidence.

- [Engine Outline](ENGINE_OUTLINE.md) — **the single authoritative implementation sequence**, including incomplete Stage 4A closure and the dependency-ordered First Playable Skirmish milestone; owns implementation order and promotion boundaries.
- [RTS Multiplayer Architecture and Retrofit Audit](RTS_MULTIPLAYER_ARCHITECTURE.md) — server-authoritative model and MP gates. **MP-0 audit complete; MP-1 implemented**: listen server, connection-bound seats, one command intake, team fog of war, 1v1/2v1/3v1/2v2 + Unaligned layouts. The MP-1 packaged qualification is two-process 1v1 loopback. **MP-3 first increment implemented:** a real headless `PolitikaServer` dedicated server with all-remote seats and session reset, packaged 1v1 parity PASS. **MP-2 first increment implemented:** packaged dedicated 2v1/3v1/2v2 with three or four rendered clients and fog-safe per-connection shot cues (network protocol 2). **MP-2 second increment implemented:** packaged layouts under engine-emulated lag, loss, reordering and duplication, with per-connection bandwidth and server step-cost measurements (F36 closed). Client fog shading and an Unaligned engagement fixture (MP-2) come next; public services are MP-4.
- [MP-1 Multiplayer Foundation Acceptance](RTS_MULTIPLAYER_FOUNDATION_ACCEPTANCE.md) — per-test verdicts, package identity, evidence paths, reference checkpoint and limitations for the first networked slice.
- [MP-3 Dedicated Server Acceptance](RTS_MULTIPLAYER_DEDICATED_SERVER_ACCEPTANCE.md) — headless `PolitikaServer` target, dedicated seat binding and session reset, packaged three-process verdicts, listen/dedicated parity, package identity and limits.
- [MP-2 Multi-client Layouts Acceptance](RTS_MULTIPLAYER_MP2_MULTICLIENT_ACCEPTANCE.md) — packaged 2v1/3v1/2v2 dedicated runs (4–5 processes), fog-safe shot cues (F27), allied probes, protocol-2 refusal of old clients, package identity and limits.
- [MP-2 Network Conditions Acceptance](RTS_MULTIPLAYER_MP2_NETWORK_CONDITIONS_ACCEPTANCE.md) — packaged dedicated 1v1/2v1/3v1/2v2 under engine lag/loss/reorder/duplicate emulation, exactly-once ordered intake, per-connection bandwidth and server step cost (F36), commands and limits.
- [MP-2 Fog Memory Acceptance](RTS_MULTIPLAYER_MP2_FOG_MEMORY_ACCEPTANCE.md) — viewer-side fog shading and last-known contacts (F33): portable rules, listen-host and client paths, packaged dedicated fog-memory session and per-layout no-leak audit, limits.
- [MP-2 Unaligned Engagement Acceptance](RTS_MULTIPLAYER_MP2_UNALIGNED_ENGAGEMENT_ACCEPTANCE.md) — packaged explicit attacks on the Unaligned side producing facing-only and withheld shot cues over sockets, ground-probe harness input, finding F43 (facing endpoint placement, closed by the 150 cm muzzle stub), limits.
- [MP-2 Listen Layouts Acceptance](RTS_MULTIPLAYER_MP2_LISTEN_LAYOUTS_ACCEPTANCE.md) — packaged listen-hosted 2v1/3v1/2v2 with a rendered host and 2–3 clients; host fog, last-known contact and shot-cue audits; F42 closed for listen hosting.
- [MP-2 Unit-scale Acceptance](RTS_MULTIPLAYER_MP2_UNIT_SCALE_ACCEPTANCE.md) — packaged 1v1/2v2 at 25–200 units per seat (52–802 units) as over-density stress data, and the skirmish test map at its 20-per-participant cap (1v1/2v2/3v1, up to 82 units): per-phase bandwidth and step cost, idle replication completeness, order admission, shot cues at scale; F44 (send-rate saturation, mitigated by a 400 KB/s send rate) and F45 (whole-army admission spike, open), F46 (four-seat spawn overlap, closed by the 20 cap and a fail-closed gap guard); U-12: 300 per participant is the game-wide limit, 20 on the test map.
- [Prototype Gameplay Content Authoring](GAMEPLAY_CONTENT.md) — typed asset IDs, map schema v1, validator/tool usage, and known limits of the first content-driven RTS slice.
- [Rifle-infantry reference import](CNC3_RIFLEMEN_IMPORT.md) — neutral-named mesh/animation/MP3 import, source provenance, interrupted-package recovery and scoped local audio playtest evidence.
- [Stage 3 Data/Content/Map Acceptance](STAGE3_DATA_CONTENT_MAP_ACCEPTANCE.md) — scoped ST3-01–ST3-18 verdicts, exact evidence identities, and remaining acceptance gaps.
- [First RTS Vertical Slice Milestone](RTS_VERTICAL_SLICE_MILESTONE.md) — accepted scoped Linux Development prototype, validation evidence, remaining platform gaps and follow-on content/model work; not a finished RTS or release.
- [RTS Command and Simulation Contract — Milestone Record](RTS_COMMAND_SIMULATION_CONTRACT.md) — accepted scoped Linux command/admission/simulation contract with test matrices, packaged interaction/recovery, Shipping isolation and known platform limits.
- [RTS Runtime Navigation Acceptance](RTS_RUNTIME_NAVIGATION_ACCEPTANCE.md) — authoritative Recast movement, cooked-obstacle/repeatability and valid-route interruption qualification; source/package identities and bounded remaining gates.
- [Stage 4A Match Lifecycle Acceptance](STAGE4A_MATCH_LIFECYCLE_ACCEPTANCE.md) — bounded lifecycle qualification plus the 2026-10-29 integrated participant-authority/implemented-infantry Linux Development acceptance, historical authored-startup evidence, final source/package evidence location, and explicit remaining Stage 4A boundary.
- [RTS Game Ontology](RTS_GAME_ONTOLOGY.md) — foundational RTS categories, C&C and StarCraft comparison, and POLITIKA-specific distinctions between entities, agents, ownership, actions, resources, information, and objectives; conceptual reference, not an implementation claim.
- [Cross-Genre RTS Ontology and Competitive Reference Study](RTS_CROSS_GENRE_COMPARATIVE_STUDY.md) — dated analysis of POLITIKA's source-qualified present and proposed complete game against classical, tactical, large-scale and political/economic RTS families; **research, not superiority or gameplay acceptance**.
- [RTS Competitive Benchmark and Acceptance Register](RTS_COMPETITIVE_BENCHMARK_REGISTER.md) — 17 proposed comparison questions (CMP-01–17), evidence requirements, metrics and cross-system scenarios; **no tests executed, no new Stage 4F requirement**.
- [Military Formations, Tactical Stances, Cohesion and Discipline](RTS_UNIT_FORMATIONS_AND_DISCIPLINE.md) — proposed **core unit-behavior design**: persistent organization distinct from overlapping local 0–9 control groups, shapes, stances, adherence, per-unit navigation, derived Cohesion, canonical Discipline/faction rules, FORM-F0–F8 and acceptance; **not implemented**.

- [Commander Talent System — Design and Implementation Roadmap](COMMANDER_TALENT_SYSTEM.md) — proposed Heroes of the Storm-inspired **Elite commander-only** four-tier talents, participant OXP, faction constraints, authoritative Unreal integration, phased delivery and acceptance matrix; **not implemented**.
- [Technological Research and Building Modernization](RTS_TECHNOLOGICAL_RESEARCH.md) — proposed C&C/StarCraft/AoE-informed, building-integrated research ontology, shared and local upgrades, faction restrictions, authoritative UE contract, validation and delivery roadmap; **not implemented**, no ages/eras.
- [RTS Systems Completeness Matrix](RTS_SYSTEMS_COMPLETENESS_MATRIX.md) — 29 October 2026 source/doc coverage audit, explicit missing contracts, recommended Unreal 5.8 framework choices and staged validation gates; **design**, not acceptance.
- [Practical RTS UI, Settings and Keymap](RTS_UI_SETTINGS_AND_KEYMAP.md) — actual current keyboard/mouse bindings, A/S conflict, windowed/fullscreen/resolution rollback, UI/text scaling, sound routing, menus, binding registry, other RTS interface precedents and test gates; proposed implementations.
- [RTS HUD Ontology, Comparative Design and Implementation Architecture](RTS_HUD_ARCHITECTURE.md) — **CORE PROPOSED** battlefield-first contextual HUD, comparative C&C/StarCraft/AoE/Tempest precedents, observer-filtered view/command contracts, selection/formation/economy/intelligence widgets, Unreal integration, HUD-01–11 roadmap and HUD-01–13 proposed tests; **not implemented**.
- [USD Economy, Commodity Sales, Building Electricity and Military Food](RTS_OPERATIONAL_ECONOMY_AND_POWER.md) — adopted in-game US dollars ($) as primary purchasing resource, gold/fossil exchanges for dollars, minimum building power, dollar-purchased military food reserves and scoped accounting/shortage test contracts (not implemented).
- [Overlooked RTS Mechanics Review](RTS_OVERLOOKED_MECHANICS_REVIEW.md) — implementation gaps in onboarding, tactical command stance, communications, civilian/legitimacy consequences, infrastructure, pacing and accessibility; proposed, not implemented.
- [Authoritative Visibility and Intelligence](RTS_VISIBILITY_AND_INTELLIGENCE.md) — proposed per-participant knowledge, fog, stale sightings and AI/UI information authorization.
- [Selection, Control Groups, Group Commands and Queues](RTS_SELECTION_AND_GROUP_ORDERS.md) — **normative** proposed mouse/keyboard gesture state machine, selectable ID sets, numbered 0–9 overlapping groups (replace/add/exclusive transfer/recall/focus), dynamic visible selection predicates, per-issuer order admission, 4C-G core-production gate, queueing deferred; **4C baseline implemented 30 Oct 2026** (multi-selection, box/Shift/Ctrl gestures, per-issuer group orders); control groups and queueing not implemented.
- [4C Group Orders Acceptance](RTS_GROUP_ORDERS_4C_ACCEPTANCE.md) — packaged 20-unit box selection, Shift toggle, Esc cancel, 19-unit group move admitted per unit and arriving on a dedicated server, pending bound 16 + living units; Automation GroupSelectionAndOrders; F47 (reliable-RPC count for 300-unit orders, open).
- [4C-G Control Groups Acceptance](RTS_CONTROL_GROUPS_4CG_ACCEPTANCE.md) — control groups 0–9 (Ctrl replace, Shift add, Alt transfer, recall, double-tap camera focus), clickable group bar, Ctrl+click/double-click same-type selection, Tab subgroups; packaged dedicated 20-unit runs with real input, Automation ControlGroups, one-action-per-chord input audit.
- [Comparative RTS Grouping and Mouse Input Research](RTS_CONTROL_GROUP_INPUT_RESEARCH.md) — public official StarCraft II, C&C3, Age of Empires IV and Tempest Rising control references, POLITIKA ontology/interaction synthesis, evidence versus design distinction; **research, not acceptance**.
- [Structures, Production and Technology](RTS_PRODUCTION_AND_TECHNOLOGY.md) — proposed one-resource Stage 4B reinforcement flow, ledger, placement checks, producer queues and deferred prerequisite graphs.
- [RTS Artificial Intelligence Ontology and Architecture](RTS_AI_ARCHITECTURE.md) — proposed **core cross-system AI design** and comparative C&C, StarCraft II, AoE II, OpenRA, Supreme Commander/Sorian and Tempest Rising evidence; separates local autonomy, formation tactics, operational planning, playable-faction strategy and NPC Unaligned institutions; **not implemented**.
- [Bounded Skirmish Opponent AI](RTS_OPPONENT_AI.md) — owning narrow Stage 4D deterministic, participant-bound first opponent and AI-01–AI-11 acceptance; **not implemented**.
- [Match Persistence, Replay and Determinism](RTS_MATCH_PERSISTENCE_AND_REPLAY.md) — proposed snapshot/replay ownership, compatibility, integrity and repeatability boundaries; **deferred** beyond first skirmish.
- [Tempest Rising — Engine Architecture Reference](TEMPEST_RISING_ENGINE_REFERENCE.md) — primary contemporary RTS reference; first-party editor/API evidence, verified-versus-inferred architecture, POLITIKA adaptation rules, and bounded acceptance proposals (**research only**, not acceptance).
- [C&C Gameplay Comparative Study](CNC_GAMEPLAY_COMPARATIVE_STUDY.md) — cross-title design patterns and POLITIKA-specific application; extracted local reference material is optional evidence, not a portable repository dependency.
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
- [MP-0 Multiplayer Code Audit — 29 October 2026](Reports/RTS_MULTIPLAYER_CODE_AUDIT_20261029.md) — whole-source inventory, path trace, mutation/exposure inventories and the F01–F42 findings register with remediations and deferred owners.
- [Repository Implementation Audit — 29 October 2026](Reports/REPOSITORY_IMPLEMENTATION_AUDIT_20261029.md) — tracked-source inventory, verified versus unqualified milestones, prioritized risks, and Rust-free native core test fallback; an audit, not a new gameplay acceptance.
- [Navigation Interruption Report — 29 October 2026](Reports/NAVIGATION_INTERRUPTION_20261029.md) — archived dirty-snapshot evidence; consult the owning navigation record for later publication qualification.
- [Versioning and Release Management](VERSIONING_RELEASE_MANAGEMENT_ROADMAP.md) — canonical build/game version policy.
- [Performance and Release Gates](PERFORMANCE_AND_RELEASE_GATES.md) — planned measurable RTS fixtures and release acceptance.

## Historical and media production

- [Historical Reference Archive](../Archive/README.md) — research programmes and catalogues.
- [Political history and chronology](../Archive/HISTORICAL_CHRONOLOGY_1980_1989.md), [Political Events Catalogue](../Archive/POLITICAL_EVENTS.csv) — factual historical references.
- [Map research and sources](../Archive/MAPS_1980_1989.md), [Map Catalogue](../Archive/MAP_CATALOG.csv) — dated/uncertain cartographic references.
- [Archive Rights and Provenance](../Archive/RIGHTS_AND_PROVENANCE.md) — licenses, attribution and evidence-versus-asset distinction.
- [Production Standards](PRODUCTION_STANDARDS.md) — period decisions, source-to-map lineage, RTS model, licensing and visual/audio review.
- [RTS Asset Resource Research](RTS_ASSET_RESOURCE_RESEARCH.md) — third-party acquisition leads, staged CC0 assets and UE import criteria; staging does not prove cooking or distribution clearance.

## Machine-readable design-stage registries

- [Faction content registry](../Data/content_manifest.json)
- [Historical decisions ledger](../Data/historical_decisions.json)
- [Map production sources](../Data/map_production.json)
- [Audio event catalogue](../Data/audio_events.json)
- [Performance scenarios](../Data/performance_cases.json)
- [Release readiness gates](../Data/release_matrix.json)

The register entries marked `planned`, `research_only` or `blocked` are not shipped assets, completed tests, historical certifications or releases.

## Status and evidence reading order

Use the [Systems Completeness Matrix](RTS_SYSTEMS_COMPLETENESS_MATRIX.md) to locate an owning document; its assessment is a dated snapshot, not current-source acceptance. Then consult relevant [Stage 3](STAGE3_DATA_CONTENT_MAP_ACCEPTANCE.md), [Stage 4A](STAGE4A_MATCH_LIFECYCLE_ACCEPTANCE.md), [command](RTS_COMMAND_SIMULATION_CONTRACT.md) and [navigation](RTS_RUNTIME_NAVIGATION_ACCEPTANCE.md) records, comparing **Git SHA, dirty status, engine pin, content identity and package hash**. [Validation and Governance](VALIDATION_AND_GOVERNANCE.md) defines proposed unified evidence fields, whereas [release_matrix.json](../Data/release_matrix.json) is a release-gate ledger. Technical acceptance does not grant distribution rights: see [Archive Rights](../Archive/RIGHTS_AND_PROVENANCE.md) and [Production Standards](PRODUCTION_STANDARDS.md).

## Maintenance rule

**Code and verified tests** prove what is implemented. **Game-design owning documents** define intended gameplay. **Archive catalogues** provide documented historical evidence, not automatic production rights. **Validation and production policy** define acceptance methods. **README** is the accessible public summary and links into these owners. Keep each rule in exactly one authoritative location; add links rather than duplicate paragraphs in several roadmaps.
