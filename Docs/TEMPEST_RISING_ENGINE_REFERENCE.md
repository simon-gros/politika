# Tempest Rising — Engine Architecture Reference for POLITIKA

**Status:** Comparative technical reference and proposed engineering guidance; **not an implementation specification or acceptance record**.  
**Research date:** 29 October 2026.  
**Reference product:** *Tempest Rising*, Slipgate Ironworks, Unreal Engine 5 RTS; official editor documentation and 2026 updates.  
**POLITIKA engine target:** Unreal Engine 5.8.3 (repository pin; confirm before implementation).  
**Authority:** [ENGINE_OUTLINE.md](ENGINE_OUTLINE.md) owns implementation sequence; owning gameplay specifications define POLITIKA rules; acceptance records establish only their stated evidence and source/package identities.  
**Scope:** Game-engine design, data definitions, unit and structure architecture, commands, movement, effects, research, economy, map authoring, UI, AI, scripting, runtime validation and platform implications.

> **Central decision:** Treat *Tempest Rising* as POLITIKA's **primary contemporary RTS technical and player-experience reference**, while retaining POLITIKA's independently owned C++ rules, typed command admission, 20 Hz match coordinator, historical fiction, design canon and multi-platform constraints. It is a reference, not an engine dependency or a source-code reproduction.

## 1. Purpose and evidence discipline

POLITIKA and Tempest Rising both seek to modernize the classic base-building, multi-unit, real-time military-strategy genre in Unreal Engine. The comparable design problems include ordering groups of units, constructing and powering bases, producing reinforcements, researching upgrades, implementing faction asymmetry, authoring campaign maps, and presenting large amounts of actionable battlefield information. POLITIKA additionally requires an alternate-history 1980s Yugoslav setting, four playable factions, the NPC-only Unaligned administration, US-dollar accounting, electricity and food upkeep, commander-only talents, and building-integrated research without age/era progression.

Publicly available Tempest Rising materials expose **some real engine-side architecture**, especially through its 2026 official map editor. They do **not** expose its complete proprietary engine source, full runtime class hierarchy, scheduling and replication internals, AI algorithms, navigation optimization, performance instrumentation, or determinism guarantees.

Use three evidence classes throughout this reference:

- **[CONFIRMED]** Explicitly documented in a first-party map editor manual, public editor API or official release note. Public API exposure is evidence of that interface, **not** necessarily of every private runtime detail.
- **[INFERRED]** A plausible engineering interpretation of documented behavior; never describe this as verified source implementation.
- **[PROPOSED]** A POLITIKA design recommendation whose implementation, cost and correctness still require local proof.

Do not turn this study into a duplicate roadmap. Every change must follow the owning documentation, a bounded implementation task and the repository's source/asset protection and acceptance process.

## 2. Primary and secondary source register

| ID | Source | What it establishes | Limitation |
|---|---|---|---|
| TR-01 | [Official map editor guides](https://support.saber.games/hc/en/tempest-rising/articles/map-editor-guides-26) | Publisher-maintained index of the official manuals, environment-art guide and API reference | An index, not a runtime specification |
| TR-02 | [Map Editor Manual, 33 pages](https://public-files.eu.theymes.com/saber-interactive/6597a5d1-b703-413a-a84b-b5266de59825.pdf) | Map metadata, player starts, navigation, spawners, Gameplay Ability System, definitions/modifiers, packaging and scripting constraints | Editor-facing interface; the manual explicitly limits mod support |
| TR-03 | [Unreal Editor API, 190 pages](https://public-files.eu.theymes.com/saber-interactive/6f366aa1-34db-400f-9010-54f000ca16a0.pdf) | Publicly exposed `TedPawn`, `TedAttributeSet`, `TedDefinitionModifier` and specialized actor/definition interfaces | API surface is not a complete implementation or formal architecture diagram |
| TR-04 | [Official July 7, 2026 patch notes](https://support.saber.games/hc/en/tempest-rising/articles/july-7-2026-patch-notes-27) | Unreal-based map editor, example maps, simultaneous production, production queue/spectator UI | Game update description, not subsystem source |
| TR-05 | [Official September 29, 2026 patch notes](https://support.saber.games/hc/en/tempest-rising/articles/september-29-2026-patch-notes-30) | Third playable faction, new commands and UI, definition-modifier Blueprint exposure, production/queue and order-chain bug fixes | Confirms outcomes and changes rather than private implementations |
| UE-01 | [Epic: Gameplay Ability System overview](https://dev.epicgames.com/documentation/en-us/unreal-engine/understanding-the-unreal-engine-gameplay-ability-system) | ASC, Gameplay Abilities, Attribute Sets, Effects, tags, replication, cosmetic cues | Unreal framework capabilities, not POLITIKA adoption |
| UE-02 | [Epic: Gameplay Effects](https://dev.epicgames.com/documentation/en-us/unreal-engine/gameplay-effects-for-the-gameplay-ability-system-in-unreal-engine) | Runtime effect specs, duration, modifiers, stacking and calculations | Requires explicit authority, timing and migration design in POLITIKA |
| UE-03 | [Epic: Unreal navigation](https://dev.epicgames.com/documentation/en-us/unreal-engine/navigation-system-in-unreal-engine) | Engine-supported navigation primitives | Must not displace the existing POLITIKA coordinator |
| UE-04 | [Epic: Data Validation](https://dev.epicgames.com/documentation/en-us/unreal-engine/data-validation-in-unreal-engine) | Editor and commandlet content validation | Must be combined with cooked pre-match admission checks |

**Date and version caution:** The official editor instructions require **UE 5.4.4** for Tempest Rising's *map editor* (TR-04; TR-02, page 4). That does **not** establish that every retail game executable uses precisely the same patch version or that POLITIKA should downgrade from its pinned UE 5.8.3. Any later editor/API revision must be rechecked against the linked sources.

### Page-level source guide

- TR-02 pp. 8–10: `TedMapInfo`, per-player settings, human/bot/uncontrollable slots, starts and initial player data.
- TR-02 pp. 12–14: five navigation types, top-down projection limitation, modifiers, disconnected navmesh islands and airborne navigation.
- TR-02 pp. 15–18: spawner-based entity placement, neutral buildings, repairable bridges and units.
- TR-02 pp. 23–26: custom assets, dedicated-server caveat, GAS attributes, `TedDefinitionAsset` and `TedDefinitionModifier`.
- TR-03 pp. 4–5: `TedAttributeSet` attributes; p. 20: `TedDefinitionModifier` and `TedPawn`; pp. 151 and 162: `TedStructureBase` and `TedUnitBase`.
- TR-04, July 7: simultaneous production and production/spectator interface changes.
- TR-05, September 29: definition-modifier support, updated unit command behavior, faction expansion and related fixes.

Page numbers above use the **printed page labels** in the original documents; PDF viewer indexes may be one lower.

## 3. Verified Tempest Rising architectural observations

### 3.1 A common entity foundation

**[CONFIRMED; TR-03 pp. 20, 151, 162]** The exposed API documents `TedPawn` (parent: Unreal `Pawn`) as a common class for units, structures and destructibles. `TedUnitBase` and `TedStructureBase` are more specialized descendants. `TedPawn` exposes a definition ID as a Gameplay Tag and events associated with death, command execution/completion and visibility transitions. Building-specific events include capture, sale and power-state changes.

**[INFERRED]** A shared gameplay-object identity enables common command, event, target, ownership and lifecycle interactions. It does not prove how all private systems are organized or how many engine objects are active per unit.

**[PROPOSED]** Preserve POLITIKA's existing Actors/components, stable authored definition IDs and runtime entity IDs. Introduce common capability interfaces **only when needed** for the same owner, health, command, target or effect semantics. Do not force a new base-`Pawn` migration merely to mirror a proprietary class name.

### 3.2 Definition assets and stable references

**[CONFIRMED; TR-02 pp. 24–26; TR-03]** `TedDefinitionAsset` is a Data Asset for unit/structure/weapon properties. Every definition has a unique Gameplay Tag definition ID. The editor documents properties that cannot be changed through Gameplay Effects, including names, icons and available weapons/abilities. Definitions and the runtime entities they describe are distinct.

**[PROPOSED]** Keep POLITIKA's current typed asset/ID validation (Stage 3); extend it incrementally with `StructureDefinition`, `ProductionRecipe`, `TechnologyDefinition` and narrowly scoped `CapabilityModifier` only as implemented features require them. Content IDs must be immutable across display-name, file, mesh and localization changes. An instance ID, participant ID, content revision and match-generation ID are separate concepts. Gameplay Tags may express capabilities or categories but must not silently replace existing stable IDs.

### 3.3 Gameplay Ability System

**[CONFIRMED; TR-02 pp. 23–25]** Tempest Rising states that it uses Unreal's Gameplay Ability System (GAS) for unit/structure attributes and commands. Each `TedPawn` exposes an Ability System Component. `TedAttributeSet` includes health, ammunition multipliers, damage resistances, attack parameters, movement speed, stealth/fog visibility, resource/production modifiers, construction radius and power. Applying Gameplay Effects or executing commands is documented as server/single-player-authority-only.

**[INFERRED]** GAS provides a shared mechanism for several combat, economy and support effects, but the documentation does not establish that all Tempest Rising movement, production, AI and order sequencing is natively scheduled by GAS.

**[PROPOSED]** Evaluate GAS in a **bounded experimental slice**, not as a replacement for POLITIKA's typed command pipeline or 0.05-second authoritative update. The adapter must be called *after* existing participant/command admission and must preserve deterministic source-owned event ordering. Begin with one unit and one modifier/status effect and compare results across update schedules. Gameplay Cues remain cosmetic; do not let animations or cues mutate gameplay truth. Do not enable overlapping cooldown or ability schedulers accidentally.

### 3.4 Navigation profiles and placement

**[CONFIRMED; TR-02 pp. 12–14]** Tempest Rising's editor requires five navigation representations:

1. Infantry.
2. Medium ground vehicles.
3. Large ground vehicles.
4. Square grid for structure placement.
5. Air units.

The manual describes navigation volumes restricting mines, structures, ground units or air units, `TedNavPylon` to invalidate inaccessible navmesh islands, and airborne navigation projected above underlying terrain. It explicitly documents a **vertical-overlap limitation**: a navigable bridge over a simultaneously navigable passage below is not supported by the map editor's top-down projected navigation.

**[PROPOSED]** Extend POLITIKA's existing Recast path query, route storage and 20 Hz execution with *typed movement profiles*, explicit placement occupancy and later aerial traversal. Start with two ground profiles and structure footprint legality; add representations only after a fixture demonstrates an actual need. Reject partially usable or disconnected routes according to the owning navigation contract. Do not infer that five physical navmeshes are required; profile-aware filters or separate nav data should be benchmarked first. The Balkan setting requires deliberate support tests for bridges, tunnels, steep slopes and infrastructure destruction; do not inherit Tempest Rising's vertical-overlap limitation without a documented design decision.

### 3.5 Map metadata, player relations and spawn semantics

**[CONFIRMED; TR-02 pp. 8–10, 15–18]** Maps use `TedMapInfo` metadata with minimap texture, player slot definitions and start actors. The manual distinguishes Human, Bot and Uncontrollable settings, and identifies special neutral/always-hostile actors as nonstandard player roles. Player data includes starting resources and unlocked technologies; player settings include custom relations. The manual warns authors to place *Spawner Blueprints* rather than directly placing final gameplay structure actors.

**[PROPOSED]** Preserve separation of map/geometry, match configuration, participant identity, faction affiliation and entity ownership. An authored scenario should request spawns from validated definitions, with legal placement, route reachability and initial authority checked before Active. A neutral civilian site, Yugoslav federal institution or Unaligned military unit must never become a playable participant by accident. Use an explicit **NPC-only Unaligned ownership policy**, not a Tempest Rising-style fake-player shortcut if that obscures POLITIKA's controller/slot invariants. Do not copy the commercial editor's invalid-placement failure cases as accepted behavior.

### 3.6 Definition modifiers and upgrades

**[CONFIRMED; TR-02 pp. 25–26; TR-03 p. 20; TR-05]** A `TedDefinitionModifier` is a Data Asset with a definition-type constraint, property operations and optional replication. Documented operations include Add, Remove, Multiply, Divide and Override. The editor exposes an `ApplyDefinitionModifier` Blueprint operation; modifications apply to a particular player's definition and can affect both current and future owned pawns of the chosen definition ID. **Important caveat:** the official manual says definition modifiers are **not saved automatically** in saved games and must be reapplied for persistence.

**[PROPOSED]** Treat this as evidence for a reusable effect concept, **not** a recommendation to mutate arbitrary reflected properties using strings. Use typed allow-listed numeric/stat/capability operations, explicit scopes, bounds, rounding, stacking and provenance. Store completed research/unlocks in authoritative serialized participant state and reconstruct effective attributes from immutable definitions and ordered modifier sets. Persist/reapply transient effects according to their own lifetime. The original source design's save caveat is a negative test for POLITIKA.

### 3.7 Scripts, custom maps and dedicated servers

**[CONFIRMED; TR-02 pp. 1, 23, 27–33; TR-04]** The editor creates custom maps as Unreal mod plugins with custom assets and Blueprints. Other mod types and custom campaigns are **not properly supported** by the official editor manual, and these mod packages cannot contain native code. The manual identifies game-specific trigger volumes and a dedicated-server environment for custom multiplayer games; it warns against playing/loading client-only audio on dedicated servers. The release includes example maps and Steam Workshop integration.

**[PROPOSED]** Build a **developer-first**, versioned map/scenario authoring workflow in Unreal Editor with pre-commit/CI validation and cooked package tests; public mod distribution is optional and later. Expose narrow mission scripting actions that use actual match authority, never direct money/health/ownership/mission-result mutation from arbitrary Blueprint. Separate server-safe gameplay logic from local audio/render dependencies even while multiplayer remains undecided.

### 3.8 Patching reveals important operational edge cases

**[CONFIRMED; TR-04 and TR-05]** Tempest Rising has added simultaneous multi-factory production, global queue operations, upgrade/production spectator panels, richer minimap symbols, and third-faction content. Its September patch fixed a case where multiple production queues could accept more units than a population limit permitted and thereby deduct credits for units never produced; it also fixed failures to execute chained orders after certain abilities.

**[PROPOSED]** Make these failures explicit POLITIKA negative tests: capacity/affordability checks across simultaneous queues must be atomic, refunds and reservations must reconcile, and completion/cancellation/next-order semantics must survive ability changes, unit destruction, spawn obstruction and match termination. UI commands and hotkeys must describe the same admitted actions. New faction content should add definitions/policies rather than duplicate a whole gameplay stack.

## 4. Crosswalk to POLITIKA's current system ownership

This table compares Tempest Rising's documented lessons with **POLITIKA's repository documentation as read on 29 October 2026**. A proposed document or historical passed fixture is not evidence of a fully integrated current playable game.

| Concern | POLITIKA owner/current boundary | Tempest Rising reference | Adaptation rule |
|---|---|---|---|
| Match phases and authority | [Stage 4A](STAGE4A_MATCH_LIFECYCLE_ACCEPTANCE.md); [Engine Outline](ENGINE_OUTLINE.md) | `TedPawn` authority-facing events / server-only effect calls | Preserve unique match-generation, lifecycle and participant admission |
| Definitions, maps, validation | [Gameplay Content](GAMEPLAY_CONTENT.md); Stage 3 record | `TedDefinitionAsset`, `TedMapInfo`, spawners | Extend stable IDs and fail-closed authored admission |
| Movement | [Navigation Acceptance](RTS_RUNTIME_NAVIGATION_ACCEPTANCE.md) | Five movement/placement nav representations | Add proven profiles without a second movement scheduler |
| Combat and commands | [Command Contract](RTS_COMMAND_SIMULATION_CONTRACT.md) | GAS attributes/commands | Test a small GAS adapter behind existing typed orders |
| Economy | [Resource Economy](RESOURCE_ECONOMY.md); [USD/Power/Food](RTS_OPERATIONAL_ECONOMY_AND_POWER.md) | Resources, power and production attributes | USD cents ledger remains primary purchase authority; no second currency model |
| Buildings/queues | [Production & Technology](RTS_PRODUCTION_AND_TECHNOLOGY.md) | Spawners, producer buildings, multiple queue controls | One powered fixed-base reinforcement first; placement later |
| Research | [Technological Research](RTS_TECHNOLOGICAL_RESEARCH.md) | Doctrines/upgrades; definition modifiers | Research through existing buildings; no age/era system |
| Commanders | [Commander Talents](COMMANDER_TALENT_SYSTEM.md) | Unit abilities, Gameplay Effects and conditional modifiers | Elite commanders only; separate talent choice from ordinary unit progression |
| Selection/HUD | [Selection and Group Orders](RTS_SELECTION_AND_GROUP_ORDERS.md); [UI Overview](POLITIKA_UI_OVERVIEW.md) | Multi-production, queue UI, readability improvements | One semantic command surface; rendered acceptance |
| Visibility/intelligence | [Visibility and Intelligence](RTS_VISIBILITY_AND_INTELLIGENCE.md) | Fog/vision attributes and minimap metadata | Per-participant observations; Unaligned deception never rewrites world truth |
| Opponent AI | [Opponent AI](RTS_OPPONENT_AI.md) | Autonomous RTS opponent behavior (player-facing) | Same authenticated commands, economy and visibility as a human |
| Persistence/replay | [Match Persistence](RTS_MATCH_PERSISTENCE_AND_REPLAY.md) | Definition-modifier save limitation | Persist authoritative unlocks, states and content revision; test recovery |
| Content pipeline | [Validation and Governance](VALIDATION_AND_GOVERNANCE.md) | Official editor, examples, mod packaging | Editor validation + cooked admission + reproducible package evidence |

### Do not create these new authorities

- A second gameplay simulator or actor-owned movement Tick.
- A separate research-effect engine that writes combat state directly.
- A second currency or queue ledger inside production buildings, AI or UI.
- A generic reflection-based modifier interpreter accepting any property name.
- A faction enum that equates owner, controller, playable slot and disposition.
- A mission Blueprint path that bypasses command/transaction/objective rules.
- A forced GAS migration or a speculative full custom RTS navigation rewrite.

## 5. Proposed POLITIKA data and simulation contract

The following names are **candidate contracts**, not claims about existing C++ types. Reuse current definitions and IDs before introducing any of them.

```text
Authoring (UE Editor)
  validated versioned map + scenario + typed definition assets
      -> pre-match admission -> match generation + participants
             |
Input/AI/mission-authorized intent
      -> typed request with issuer, participant, correlation, generation
      -> existing admission and permission checks
             |
             v
      ONE 20 Hz authoritative coordinator
         movement / combat / objective lifecycle
         production + economy transactions
         building service (electricity) + force upkeep (food)
         researched capability state + commander talents
             |
          ordered events
             |
      participant-filtered observation and render projections
         HUD / VFX / minimap / AI
         snapshot/replay records when separately implemented
```

### 5.1 Identity

Every content-backed runtime object resolves a **versioned stable definition ID**, while its **spawned runtime identity** belongs to exactly one match generation. Issuing participant, owning participant, controller, faction, team, neutral role and observed affiliation are distinct. Research belongs to an authorized participant or an explicitly scoped facility, not to a UI widget or to the rendered mesh.

### 5.2 Command admission

A request must be attributable to a valid controller/participant and match. Validation checks availability, possession, target/visibility policy, prerequisites, stock/capacity and spatial legality as appropriate. Rejection leaves authoritative state unchanged. Admission creates a finite, interruptible operation. Resolved events distinguish accepted intent, actual start, completion, interruption, target loss and failure.

### 5.3 Economy and production

The sole primary purchase currency is integer **US-dollar cents**. Gold and eligible fossil commodity stock may be sold under defined, exactly-once settlement rules. Food is a purchased, consumed reserve for staffed military forces. Buildings require positive minimum electricity and cannot produce when insufficiently supplied; electricity is a service/load constraint, **not another spendable wallet**. A production queue may reserve/debit funds only once, uses an explicit recipe and producer, and must settle cancellation, destruction and blocked spawn through documented policies.

### 5.4 Research, upgrades and commander effects

A technology requires one or more authorized existing structures, validated prerequisites, an explicit USD cost (plus specialized materials where designed), and a timed process. Completion records an authoritative unlock and/or a set of typed modifiers. An effective stat is derived from ordered layers:

```text
immutable authored archetype
  + faction and allowed scenario configuration
  + researched participant/facility upgrades
  + elite commander talent effects (where applicable)
  + transient combat/status effects
  => bounded effective capability/stat view
```

Define whether modifiers affect *existing* entities, *future* entities, or both. Refuse unsupported combinations rather than silently applying a partial upgrade. Research must never introduce civilization ages or eras, and commander talent progression must not spill onto ordinary infantry.

A proposed typed modifier record should contain: `modifier_id`, `source_type`, `scope`, `target_definition_or_capability`, `attribute`, `operation`, `value`, `stacking_group`, `application_order`, `lifetime`, `content_revision`. Apply only allow-listed fields; validate finite numeric values, overflow/underflow, divide-by-zero, additive versus multiplicative semantics and bounds. Keep high-value finance and stock as integer/fixed-point transactions outside floating-point GAS attributes.

### 5.5 Spatial authority

A path request must identify its movement profile and legal origin/goal; a structure request must validate footprint, slope, buildable region, collision, route preservation and current occupancy. Dynamic destruction or construction must invalidate/replan routes through the existing coordinator. Presentation previews are advisory; server/single-player authority rechecks placements at admission and completion.

### 5.6 Observation

Keep world truth independent from player-visible intelligence. A HUD, AI or mission-facing observer receives only its authorized state projection. Current sightings, stale contacts, unverified reports and deliberate disinformation are separate from actual ownership/position, especially for Unaligned political and military activity.

## 6. Reference-driven implementation order and acceptance gates

These packages **supplement** the existing Stage 4 roadmap; they do not reorder it or assert readiness to implement a later dependency early.

| Gate | Bounded addition | Required evidence / negative cases | Dependency |
|---|---|---|---|
| TR-P0 | Preserve and close Stage 4A authority/lifecycle and outstanding navigation qualification | No post-terminal orders, stale owners/selection or false navigation success; same-process clean retry; exact source/engine/package identity | Existing Stage 4A and NAV contracts |
| TR-P1 | Stage 4B powered fixed-base economy/production | USD credit/debit, one lawful source/settlement, positive power load, food purchase/upkeep, one produced reinforcement; no double charge or unpowered build | TR-P0 |
| TR-P2 | Group selection and commands | Per-unit admission, bounded group destinations, stale IDs, invalid target, mixed outcomes, stop/replacement and Recast obstacle fixture | Existing movement/participant authority |
| TR-P3 | Stage 4D opponent | AI uses its own participant, USD and food, powered producer, observed enemy data and identical command API; can lose and win authored fixtures | TR-P1/2 and Stage 4D requirements |
| TR-P4 | Placement and production specialization | Verified occupancy/reservation, blocked-output handling, simultaneous queue affordability, destruction/capture policy, correct faction units | TR-P1; separate map schema increment |
| TR-P5 | One technology modifier | Powered research structure, prerequisite, USD cost, research timer, one observable effective stat change, save/reconstruction contract | TR-P4 and owning tech spec |
| TR-P6 | One elite commander talent | Single permitted elite commander, authorized choice, scoped effect, no ordinary-unit talent selection, retirement on commander/match death | TR-P5 shared modifier semantics; owning talent spec |
| TR-P7 | Fog/minimap and authoring workflows | No hidden fact leakage; map/start/neutral/bridge/footprint validation; packaged scenario recovery | Visibility and content owners |
| TR-P8 | Scale/platform profiling | Measured unit density, path budget, spikes, CPU/GPU memory, Linux/Windows/SteamOS/Android control/render evidence as applicable | Playable coherent match |

The numbering above is a **reference-review checklist**, not a competing Stage 4 milestone vocabulary.

### 6.1 Focused GAS feasibility gate (optional, not a prerequisite for Stage 4B)

1. Verify actual engine version and gameplay module/plugin build compatibility.
2. Integrate one ASC/AttributeSet in an isolated experimental worktree or fixture, without replacing current health/order ownership.
3. Apply one server-authorized effect to one unit after typed admission and produce an observable attribute/event result.
4. Compare behavior under different render schedules but the same canonical 0.05-second step sequence.
5. Test interruption, actor death, Terminal, match restart and absence of client-only mutation.
6. Measure cost of ASC/effects at representative unit counts against existing components.
7. Retain the integration only if it is materially simpler/safer than a small native C++ modifier service; document overlap and authoritative state owner explicitly.

**No broader GAS rollout is implied by passing one feasibility gate.**

### 6.2 Navigation specialization gate

Author a cooked fixture containing an infantry-only passage, a vehicle-width restriction, a placement-exclusion region and a static obstacle detour. Validate starts and connectivity for each named profile; run real runtime routes, rejected-route checks, order replacement/death/terminal cancellation and repeatability. Add bridges/underpasses later with documented limitations and no invented vertical traversal. Ensure any new navigation data remains consumed through the current coordinator. The existing NAV-A15 inconclusive evidence must not be relabeled as PASS.

### 6.3 Economy/research end-to-end gate

Start an admitted match with an authorized powered research building, a known dollar balance and a unit definition. Submit one tech request through the same authority checks as production. Verify one debit/reservation, one timer, completion after canonical simulation steps, one modifier event, consistent existing/future unit semantics, unchanged balances for duplicates/rejections, power-loss pause, facility destruction response, a valid rematch and faithful restoration where persistence is supported. The UI must show real cost, prerequisites, duration and applied state.

### 6.4 Editor and content gate

Validate asset IDs, prerequisite graph acyclicity, referenced factions/units/structures/technologies, placement footprints, legal starts, per-profile connectivity, neutral/system ownership, scenario/script parameters, and cooked missing/invalid assets before Active. Provide actionable diagnostics with asset path, rule code and violated requirement. A map editor's visual placement is **not** sufficient evidence for valid runtime startup.

## 7. Performance, platforms and boundaries

Tempest Rising's published editor and multiplayer support do not establish a general capacity or performance benchmark for POLITIKA, and its Windows-oriented release does not prove Linux/Vulkan, Steam Deck, Android, controller or touchscreen performance. Record baseline and after-change measurements on target hardware at 1080p, 1440p, 4K and handheld resolutions where applicable; observe Game/Render Thread time, navigation query cost, queue/AI scheduler budgets, spawning spikes, animation and visual-effect LOD, memory and input latency.

Use a measured Actor-to-data-oriented transition only when verified unit counts or profiling justify it. Unreal MassEntity or World Partition may be evaluated later, but no public Tempest Rising editor source reviewed here proves that it uses either internally. Networking architecture (dedicated-authoritative vs. lockstep, replication frequency, interest management, replay data) remains a POLITIKA design decision. Do not promise deterministic cross-platform simulation without empirical proof.

## 8. Non-adoption rules and legal provenance

1. **Do not reproduce** Tempest Rising proprietary source, extracted compiled modules, private assets, class implementations or visual identifiers. General design patterns and publicly documented API concepts are references only.
2. Do not infer unexposed systems: private AI algorithms, internal class counts, actual scheduler frequency, replication topology, deterministic lockstep, MassEntity use, server tick or proprietary optimization methods.
3. Do not duplicate an inability to navigate stacked traversable terrain, invalid spawn behavior, incomplete mod persistence or production-capacity bugs.
4. Do not assume the official UE 5.4.4 *editor requirement* equals the entire shipped executable's engine patch or requires POLITIKA to migrate its pinned engine.
5. Do not confuse public editor/mod interfaces with a full SDK, full native gameplay code access or unlimited campaign mod support.
6. Keep third-party documents as links and research references; do not import copyrighted PDF contents or proprietary assets into POLITIKA unless redistribution rights are established.
7. Retain the original gameplay choices: USD, commodity sale, electricity, food provisioning, period-specific institutional infrastructure, no tech ages, elite-only commander talents, NPC-only Unaligned and four genuinely asymmetric playable factions.

## 9. Open research questions — verify before implementation

- Do official developer presentations expose the runtime's actual order queue ownership, path query batching or collision/avoidance algorithms? **Not established by the sources above.**
- What is the measured practical concurrency limit for Tempest Rising's pathing and GAS-based unit effects? **Not established; POLITIKA must benchmark independently.**
- Which Tempest Rising upgrades alter immutable definitions versus live Gameplay Effects, and how are all of those serialized in the retail campaign/save system? **The editor's modifier save limitation is confirmed; the full retail save model is not.**
- What specific multiplayer server model, replication cadence and interest-management policy does Tempest Rising use? **Dedicated-server custom games are documented; deeper details are not.**
- How are scalable formations, group target allocation and dynamic path replanning implemented? **Not revealed sufficiently by the public API.**
- Are there additional official engineering talks, slides or repositories with usable and redistribution-cleared explanations? Review new evidence and update the source ledger before elevating any inference to [CONFIRMED].

## 10. Maintenance and decision register

- **Accepted for research:** Tempest Rising is the principal current-era RTS *reference* for POLITIKA's engineering and player-facing quality.
- **Accepted as design directions, subject to owning specs:** data definitions and stable IDs; command/presentation separation; typed navigation/placement; transaction-safe production; building-integrated upgrades; scoped effect evaluation; authoritative participant and neutral separation.
- **Conditional evaluations only:** GAS adoption; additional NavMesh instances; custom modifier engine; MassEntity; multiplayer replication.
- **Explicitly unimplemented by this document:** all code, assets, maps, upgrades, queues, UI screens, AI, migrations, package tests and platform qualifications.
- **Review on:** a new official Tempest Rising technical/editor release; completion of POLITIKA Stage 4B and Stage 4D; first research/talent vertical slice; measured performance or navigation failures.
- **Document maintenance:** source-backed findings belong here, gameplay rules in their existing owning documents, implementation order in [ENGINE_OUTLINE.md](ENGINE_OUTLINE.md), and verdicts in acceptance records. Update links rather than duplicating contradictory specifications.

---

*All Tempest Rising-specific implementation claims in this document are limited to the cited public developer/editor surfaces. All POLITIKA implementation guidance remains a proposal pending a separately scoped, verified coding task.*
