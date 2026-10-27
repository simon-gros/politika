# The Fundamental Ontology of Real-Time Strategy Games

**Status:** Comparative research and conceptual design reference (27 October 2026); not an implementation report or approved balance specification.  
**Scope:** General RTS ontology, classical Command & Conquer, StarCraft, genre counterexamples, and architectural implications for POLITIKA.  
**Document role:** Defines vocabulary and relationships; authoritative gameplay rules belong in [Concepts Outline](CONCEPTS_OUTLINE.md) and specialist design documents, while implementation ownership belongs in [Engine Outline](ENGINE_OUTLINE.md) and [Proposed Engine Layers](PROPOSED_ENGINE_LAYERS.md).

## 1. Purpose and method

POLITIKA draws particularly on the original *Command & Conquer* (C&C) tradition but should not mistake the conventions of one RTS series for the essential properties of the genre. This document distinguishes three levels:

1. **Fundamental RTS ontology:** the things, relations, processes, constraints, and information required for real-time strategic interaction.
2. **Classical base-building RTS grammar:** the familiar combination of units, structures, harvesting, production, technology, scouting, combat, and victory conditions characteristic of C&C, *StarCraft*, *Warcraft*, and related games.
3. **Game-specific rules:** the distinct economies, faction mechanics, unit roles, construction methods, and unconventional political/intelligence systems that make one title different from another.

The central design question is: **What is the smallest coherent model that can represent familiar RTS gameplay while also permitting POLITIKA's faction asymmetry, neutral federal authority, mercenary commerce, sabotage, propaganda, and deceptive intelligence?**

This is a **design synthesis**, not a claim that the nine-category ontology below is an academic standard or that every RTS has the same formal definition. Particularly useful points of departure are Abbadi et al.'s *Resource Entity Action* pattern [1], which abstracts RTS interactions into resources, entities, and actions, and Ontañón et al.'s survey of AI research in *StarCraft* [2], which examines simultaneous decisions, partial observability, planning, and complex action spaces. Those sources inform, but do not dictate, the ontology proposed here.

## 2. What qualifies as real-time strategy?

A working definition:

> An RTS is a continuously or regularly advancing world simulation in which one or more decision-making agents influence a shared environment through commands or policies, while independent processes and possibly competing agents advance simultaneously. The decisions concern objectives, limited capabilities or opportunities, time, and often space and imperfect information.

Several qualifications are essential:

- **Real-time** does not require updating every entity on every rendered frame. Simulation may use discrete ticks, scheduled events, and independent AI cadences while appearing continuous to the player.
- **Strategy** concerns choices with consequences for later opportunities: commitment, opportunity cost, timing, control, position, risk, and counterplay. The existence of a click-to-move interface is not sufficient on its own.
- **Agents** may be humans, AI opponents, neutral AI, mission controllers, or other autonomous decision-making systems. A faction is not the same thing as an agent.
- **Space** is central to conventional military RTS games but may be organized as tiles, graphs, sectors, territories, logistics routes, or other spatial representations rather than free movement alone.
- **Resources** include more than money or mined commodities: production time, supply, action capacity, territory, information, and access rights can all constrain decisions.
- **Combat and base building are characteristic, not logically universal.** Economic competition may replace conventional armed conflict; reinforcement-based military titles may omit resource harvesting or construction. Genre boundaries with real-time tactics and management strategy remain contested.

A useful minimal conceptual formula is:

**RTS = evolving world state + decision-making agents + commands/actions + rules/constraints + objectives under simultaneous time.**

The specific *military/base-building* RTS familiar from C&C adds the economic, construction, production, combat, reconnaissance, and technology systems discussed below.

## 3. Nine fundamental ontological categories

| Category | Central question | Typical concepts |
|---|---|---|
| **World** | In what context does play occur? | Match, scenario, environment, scenario configuration, participant set |
| **Time** | When do things happen? | Simulation ticks, durations, cooldowns, deadlines, scheduled events |
| **Entities** | What exists and has state? | Units, structures, projectiles, resources, objectives, evidence, locations |
| **Agents** | Who decides and commands? | Human participants, AI participants, autonomous world controllers |
| **Actions** | How can the world be changed? | Move, attack, produce, research, trade, capture, sabotage, investigate |
| **Rules** | What is legal, and with what consequences? | Preconditions, permissions, targeting, costs, effects, victory evaluation |
| **Resources** | What constrains choices? | Credits, fuel, energy, time, production slots, capacity, access |
| **Information** | What is perceived or believed? | Line of sight, shroud, detection, reports, uncertainty, deception |
| **Objectives** | Why are decisions meaningful? | Goals, win/loss criteria, mission tasks, strategic priorities |

These are **categories of analysis**, not a proposed mandatory set of Unreal classes, modules, or asset files. Their purpose is to prevent unrelated meanings from being compressed into one game object.

A more granular engineering taxonomy expands the nine categories:

| Technical concept | Meaning | Examples |
|---|---|---|
| Definition / archetype | Reusable authored description of a kind of entity or action | Rifle infantry archetype, rifle weapon, movement profile |
| Runtime instance | A particular entity currently existing in a match | Rifleman #1042 at a location with 65 health |
| Identity | Stable way to distinguish definitions and instances | Archetype ID, runtime entity ID |
| Property / state | Value associated with the instance | Position, facing, health, orders, ammo, power status |
| Capability | Kind of operation an entity may perform | Attack, harvest, construct, gather intelligence |
| Relationship | Association between entities or agents | Ownership, control, membership, hostility, contract |
| Command | Requested operation from a decision-maker | Player orders vehicle to attack target |
| Action / process | Authorized, executed behavior over simulation time | Vehicle travels, aims, fires; factory produces |
| Event | Recorded occurrence or emitted state transition | Damage applied, structure captured, unit destroyed |
| Spatial query | Answer about the world geometry and constraints | Reachability, visibility, occupancy, attack range |
| Rule / invariant | Requirement enforced by the simulation | Cannot attack an invalid target; cannot deploy in blocked terrain |
| Observation / report | Information about the world supplied to an agent | Visual sighting, intercepted message, forged intelligence |
| Objective state | Progress toward a match or mission goal | Control point held, target destroyed, victory reached |

### 3.1 Commands are not actions

A command is **intent**; an action is **execution**. An attack order does not immediately apply damage. The acting unit may first need a route, target acquisition, valid weapon, appropriate range, orientation, an available reload cycle, and an opportunity to fire. Execution may be delayed, interrupted, rejected, superseded, or completed.

This distinction is indispensable for queued orders, autonomous behaviors, AI, command validation, replay, and testing.

### 3.2 Ownership is not control

Ownership, operational control, affiliation, team membership, diplomatic stance, and contract rights should not be represented as a single immutable faction label. A captured vehicle, hired mercenary, neutral police patrol, and temporarily commandeered structure have different legal and gameplay relationships.

### 3.3 Existence is not operability

A refinery or power plant can physically exist and remain owned while damaged, disabled, unpowered, infiltrated, disconnected, or otherwise unable to act. *Operational state* is distinct from *entity existence* and *ownership*.

### 3.4 Presentation is not simulation truth

Meshes, animation, sound, decals, selection rings, HUD icons, fog materials, and cinematic cues represent state but must not authoritatively determine health, target legality, ownership, resource balance, or objectives. This follows the existing POLITIKA [Engine Outline](ENGINE_OUTLINE.md).

## 4. Command & Conquer and StarCraft: same ontology, different grammar

The classic C&C and StarCraft families implement much the same fundamental classes of things but organize them differently. The comparison is deliberately focused on the familiar *Tiberian Dawn / Red Alert* and *StarCraft / StarCraft II* patterns; each series and release has exceptions.

| Dimension | Classic C&C / Red Alert pattern | StarCraft / StarCraft II pattern | Generalizable abstraction |
|---|---|---|---|
| Harvesting | Tiberium/ore harvested by dedicated vehicles and delivered to refineries | Minerals/gas collected by workers | Resource node, gatherer, delivery/conversion, balance |
| Construction | Central construction-yard/production-sidebar model and map placement | Workers initiate construction; faction methods differ | Construction capability, prerequisites, placement, progress |
| Unit production | Specialized production facilities and queues | Production buildings, larva/morphing, faction-specific deployment | Producer, queue/capacity, product, cost, completion, spawn |
| Infrastructure constraint | Power generation and consumption are prominent | Supply is universal; Protoss power fields and Zerg creep are more specific | Constraint providers, consumers, spatial requirements |
| Faction distinction | Distinct arsenals, structures, tech paths, powers | Strongly asymmetric production, tech and unit mechanics | Common interface, faction-scoped capabilities/rules |
| Technology | Structures and prerequisites unlock further capability | Research, structures, upgrades and tech prerequisites | Dependency/tech graph |
| Combat | Infantry, vehicles, aircraft, defenses and special powers | Diverse combat roles, active abilities, positioning and counters | Weapons, damage, targeting, status, abilities |
| Information | Exploration/shroud, scouting and radar conventions | Fog of war, scouting, detection and intelligence through observation | Player-specific observation model |
| Victory | Destruction, mission triggers and strategic locations | Elimination and scenario/campaign-specific goals | Ruleset-dependent objective evaluator |

Sources: historical C&C manuals and documentation [3]; Blizzard's StarCraft II resources, economy, buildings, Protoss and Zerg guides [4–8].

### 4.1 The classical C&C industrial loop

A common pattern is:

`resource deposit → harvester → refinery → credits → construction / production / upgrades → forces and strategic capabilities`

This system creates recognizable dependencies: harvesting time, safe transport, refinery protection, production capacity, construction prerequisites, build space, and operating power. Control of resources and industrial facilities directly affects the ability to reproduce military strength.

The key abstraction is **production and conversion of capability over time**. A C&C-style construction yard is one implementation, not the ontological definition of constructing a building.

### 4.2 StarCraft's differentiated production

StarCraft's three races share resource and objective categories yet implement production and territorial infrastructure with different behaviors. A Zerg drone morphing into a structure is unlike conventional worker construction; larvae support Zerg unit production; Protoss pylons provide local power fields; Zerg creep affects construction, mobility, and visibility [5–8].

Hence:

> A faction may change *how* it produces, builds, and gains access to capabilities while remaining compatible with common resource, placement, prerequisite, cost, time, and completion contracts.

POLITIKA should therefore preserve C&C-style usability without hard-coding "construction yard" as the universal definition of production.

### 4.3 Genre counterexamples clarify what is optional

*World in Conflict* focuses on force deployment and tactical engagements with reinforcement points rather than conventional harvesting and base construction; it demonstrates the close boundary between RTS and real-time tactics [9].

*Offworld Trading Company* explicitly defines itself as an RTS centered on financial competition, market transactions, and economic sabotage rather than commanding combat armies [10]. It demonstrates that armed combat is not necessary to every meaningful real-time strategic contest.

Neither example implies that POLITIKA should remove the familiar C&C base/economy/army loop. They help explain why *genre ontology* must be broader than *one intended gameplay design*.

## 5. Classical RTS systems built upon the ontology

The following are the characteristic systems of a C&C-like base-building military RTS, not necessary features of every RTS genre member.

| System | Ontological inputs | Representative responsibilities |
|---|---|---|
| Match orchestration | World, agents, objectives, rules | Setup, slots, phases, start/end, results |
| World and terrain | World, space, rules | Playable bounds, terrain costs, obstacles, starts |
| Entity lifecycle | Entities, identities, time | Spawn, status changes, capture, destruction |
| Movement | Entity capabilities, space, action | Pathfinding, orders, formations, reachability, collision |
| Combat | Entities, weapons, rules, events | Targeting, attack timings, armor, damage, status effects |
| Faction / relations | Agents, relationships, rules | Roster access, ownership, control, diplomacy, hostility |
| Economy | Resources, rules, actions | Extraction, storage, conversion, expenditure |
| Construction | Entities, capabilities, resources | Placement, prerequisites, progress, cancellation |
| Production | Facilities, queues, costs, time | Recruitment, fabrication, deployment, rally points |
| Research / technology | Definitions, rules, resources | Unlocks, upgrades, tech dependencies |
| Information / fog | Observations, world, agents | Exploration, visibility, detection, information access |
| Player interaction | Agents, commands, state projections | Selection, orders, feedback, control groups, HUD |
| Autonomous AI | Agents, observations, commands | Strategic decisions, tactical orders, NPC behavior |
| Objectives / missions | Rules, world state, events | Triggers, control points, victory/defeat |
| Optional extension systems | Same common primitives | Supply networks, contracts, trade, sabotage, propaganda |

A *classical C&C-style POLITIKA* should ultimately realize the main systems above, but their presence in a design table does **not** assert that they have been coded. The implementation inventory, milestones, and tests remain authoritative elsewhere.

## 6. Formal model of state, execution, and perception

For a useful conceptual model, let the complete authoritative world at simulation time `t` be `S_t`. Its next state is:

```text
S_(t+Δt) = F(S_t, C_t, A_t, Δt)
```

where:

- `C_t` = admitted commands from participants and AI;
- `A_t` = autonomous or already-running processes (movement, firing, construction, harvesting, etc.);
- `F` = rules and update scheduling.

The process must distinguish **requested**, **admitted**, **executing**, **completed**, **interrupted**, and **rejected** operations when meaningful. A command that fails a precondition should produce an inspectable rejection rather than an unexplained silent failure.

For each participant `i`, observation is distinct from the full world state:

```text
O_i(t) = G_i(S_t, sensors_i, access_i, visibility_i)
```

where `G_i` returns the information authorized or observable for that participant. Player input and AI planning should use this projected state rather than accidentally accessing hidden world truth.

This notation is descriptive, not a promise of bitwise determinism or lockstep. The game's authoritative C++ simulation and its timing, networking, save, and replay choices remain governed by [Engine Outline](ENGINE_OUTLINE.md).

## 7. POLITIKA's special contribution: reality, evidence, and belief

Most familiar fog-of-war systems restrict what is revealed. POLITIKA's counterintelligence and propaganda concepts require *more than absence of information*: sometimes agents receive **positive but false claims**.

| Layer | Example | Meaning |
|---|---|---|
| Objective truth | No nuclear facility exists at site X | Authoritative world fact |
| Direct observation | A patrol reports empty buildings | Sensor/agent observation with source and time |
| Intelligence report | A planted document claims that X is a nuclear facility | Claim with provenance; may be fabricated |
| Belief / assessment | A rival assigns high probability to the facility's existence | Agent-specific estimate, not world truth |
| Decision | Rival diverts reconnaissance or forces to X | Consequence of acting on the assessment |

The conceptual information chain is:

`WorldState → Observation → IntelligenceReport → Belief/Assessment → Decision/Command`

The fundamental rule is:

> **A false intelligence report must never silently create its claimed facts in the authoritative world. It changes what a participant receives, infers, or believes, not what physically exists.**

This underpins The Unaligned's proposed false intelligence, counterintelligence, false flags, and balancing interventions; see [Unaligned Counterintelligence](UNALIGNED_COUNTERINTELLIGENCE.md) and [Espionage and Intimidation](ESPIONAGE_AND_INTIMIDATION.md).

Possible report fields, when implementation warrants them, include a stable report ID, source, author or purported origin, subject, timestamp, asserted facts, collection channel, access permissions, classification, evidence attachments, claimed confidence, reliability estimate, verification status, and possible disinformation attribution. **These are candidate design attributes, not an approved data schema.**

For fair gameplay, espionage and misinformation require counterplay, traceability appropriate to the fiction, and UI that can communicate uncertainty. An AI should not be permitted to consult the hidden truth to decide that fabricated information is false.

## 8. POLITIKA-specific distinctions

### 8.1 Faction, player, team, owner, controller, and relation

These concepts must remain distinct.

- **Faction** defines an in-world political/military organization, identity, roster, privileges, and design characteristics.
- **Participant / player slot** identifies one of the playable decision-making positions in a match (human or AI controlled).
- **Agent/controller** issues or chooses commands for a participant or autonomous world system.
- **Team / alliance** defines coordination or victory sharing where allowed by rules.
- **Owner** denotes legal or strategic possession of an entity.
- **Controller** denotes who may currently command it.
- **Affiliation** denotes its organizational origin or membership.
- **Diplomatic relation** expresses neutrality, hostility, cooperation, or conditional status between actors/factions/participants.

POLITIKA's [Match Structure](MATCH_STRUCTURE.md) establishes **two to four playable faction slots plus the always-present, non-playable Unaligned NPC authority**. This is a specific project rule, not a general RTS requirement.

A Meridian Pact mercenary vehicle may retain Pact affiliation while being commanded temporarily by the Atlantic Concord under a contract. A vehicle captured from an opponent may change owner without changing the identity of its original vehicle archetype. A neutral Unaligned patrol must not become hostile simply because it is AI-controlled.

The exact contract and diplomacy rules remain the responsibility of their owning gameplay documents.

### 8.2 Entity, unit, squad, base, and structure

A **game entity** is not necessarily an Unreal `AActor`, one soldier, or one selectable unit. An entity may represent a vehicle, infantry member, structure, projectile, physical resource, evidence item, or another state-bearing object.

A **squad** may group individual units under shared orders or may have a different runtime representation if later performance studies justify it. A **base** may be a functional/spatial grouping, not necessarily an independent authoritative object. A **structure** may be physically present but not currently operational.

Do not require speculative game-specific abstractions to become engine classes before a concrete behavior or test needs them.

### 8.3 Space and movement classes

Terrain should never be reduced to one universal passable/impassable value. A river, slope, bridge, urban obstacle, or road has different properties for wheeled vehicles, tracked vehicles, foot infantry, amphibious units, and aircraft.

A map definition should establish bounds, valid starts, faction slots, objectives, and spatial data. A validator should test **reachability under the relevant movement profile**, not merely graph connectivity for an imaginary all-purpose unit.

Legal placement and legal starts may also depend on occupation, buildable terrain, accessibility, hostility, and scenario-specific deployment constraints.

### 8.4 Resources and production are general capabilities

Credits, fuels, energy, supply, procurement rights, construction permissions, and production slots can all behave as strategic constraints. Particular goods or rules should not be introduced solely because they exist in another RTS.

The Meridian Pact's trading and mercenary role demonstrates why **producing, purchasing, contracting, transferring, and deploying** should be representable as distinct operations sharing some underlying concepts (cost, owner, eligibility, time, capacity, completion).

A conventional factory, Black Market purchase, hired vehicle, and scripted reinforcement need not follow the same production mechanism.

### 8.5 Information, territorial authority, and legitimacy

The Unaligned is not simply a conventional fifth opponent. Its role includes neutral authority, local institutions, responses to violations, world regulation, and intelligence interventions. More generally, an entity may be *legally present* but *politically unwelcome*, or *militarily neutral* but *subject to administrative restrictions*.

This suggests separating political permissions and legitimacy from damage/targeting hostility. The specific rules remain design-stage in [Concepts Outline](CONCEPTS_OUTLINE.md).

## 9. Candidate content and runtime vocabulary

These names are **illustrative**, not canonical existing class names, asset paths, or instructions to implement all of them immediately.

| Conceptual type | Example | Responsibility |
|---|---|---|
| `MatchDefinition` | Local skirmish | Ruleset and initial match context |
| `MapDefinition` | Regional Yugoslav test map | Bounds, starts, slots, objectives, spatial references |
| `PlayerSlotDefinition` | Playable slot 1 | Participant eligibility/configuration |
| `FactionDefinition` | Atlantic Concord | Roster and faction-specific constraints |
| `EntityDefinition` | Field Rifle Unit | Authored archetype and capabilities |
| `EntityInstance` | Rifle unit #1042 | Runtime position, condition, ownership, behavior |
| `WeaponDefinition` | Infantry rifle | Targeting and effects of weapon use |
| `MovementProfile` | Wheeled ground | Terrain access and movement cost rules |
| `AbilityDefinition` | Deploy explosive charge | Eligibility, target, duration, interruption, result |
| `ResourceDefinition` | Credits | Value type and allowed economy operations |
| `ProductionDefinition` | Vehicle fabrication | Cost, producer, duration, deployment behavior |
| `TechnologyDefinition` | Signals intelligence improvement | Research prerequisites and unlocked capabilities |
| `RelationshipState` | Concord–Unaligned neutral | Current dynamic relations |
| `ObjectiveDefinition` | Secure crossing | Conditions and evaluation |
| `Observation` | Reconnaissance sighting | Direct source-bound information |
| `IntelligenceReport` | Alleged nuclear installation | Claimed intelligence with provenance |
| `Order` | Attack target #80 | Request to act; distinct from execution |
| `SimulationEvent` | Target destroyed | Recorded occurrence and gameplay notification |

Important ID distinction: a stable **definition ID** survives asset reload and balance changes; a **runtime instance ID** identifies a particular spawned entity. Versioned definitions and explicit reference validation are required before content can be trusted at match start.

The repo's [Engine Outline](ENGINE_OUTLINE.md) already prefers stable-ID `UPrimaryDataAsset` definitions, code-owned simulation state, and authoring-time validation. This document is not an alternative technical architecture.

## 10. Actions: a reusable grammar

Actions should be modeled consistently enough to support shared validation, events, AI planning, test fixtures, and UI feedback, while permitting specialized mechanics.

| Action | Main preconditions | Typical effects |
|---|---|---|
| Move | Owned/commandable entity; valid destination; suitable movement class | Movement over time; position changes |
| Attack | Allowed initiator, weapon, target class, hostility/permission, range or pursuit | Weapon execution, damage, status changes |
| Repair | Repair capability, valid target, cost/access where required | Target condition improves over time |
| Produce | Eligible producer, cost, prerequisites, capacity, valid deployment | New entity or capability becomes available |
| Capture | Eligible entity, capturable objective, required access/time | Ownership, control, or objective state changes |
| Trade / hire | Eligible party, contract terms, funds, market access | Transfer of resources, rights, entities, or control |
| Sabotage | Suitable actor, target, access, equipment, timing | Destructive or disabling effect, possibly delayed |
| Reconnoiter | Sensor/access, target area, observation rules | New participant-specific observation |
| Disseminate false report | Eligible intelligence capability, recipient/channel, operation rules | Recipient's information is affected; world truth is unchanged |

A robust *action definition* might eventually describe: stable ID, eligible initiators, required capabilities, target schema, range, costs, duration, cooldown, preconditions, interrupt rules, consequences, and trace/event behavior.

A **weapon attack** and an **intelligence operation** can share command admission and rule interfaces without being forced through the same combat-resolution code.

## 11. What makes an RTS strategically interesting?

A list of entities alone does not create strategy. Strategy arises when choices affect future possibilities under scarcity, competition, and uncertainty.

| Dimension | Strategic question |
|---|---|
| Spatial | Where should facilities and forces be positioned? |
| Temporal | What must happen now; what can be delayed? |
| Economic | Where should scarce resources and capacity be committed? |
| Military | Which threats should be engaged, deterred, bypassed, or defended against? |
| Informational | What is known, unknown, uncertain, or intentionally falsified? |
| Organizational | Who has authority, ownership, access, command, or cooperative obligations? |
| Technological | Which future capabilities justify investment? |
| Political | Which institutions, relationships, and perceptions can be influenced? |
| Objective-oriented | Which sequence of actions best achieves the scenario's goals? |

An extra refinery may improve future income while additional tanks improve immediate strength. The significance of the choice depends on enemy pressure, time, access to resources, terrain, and what the player believes about opposing plans.

Classical C&C puts particular weight on military, economic, territorial, temporal and technological choices. StarCraft intensifies their interaction through asymmetrical faction production and high information demands. POLITIKA's planned identity places additional emphasis on **information reliability, political authority, organizational control, and commerce**.

## 12. Engineering implications and staged application

This document should inform the implementation **without expanding the next milestone into a full RTS**.

**Stage A — trusted minimal simulation and content.** Preserve the existing prototype's tested behavior while defining stable-ID unit and weapon content; a versioned fixture-map definition with bounds, slot/start rules and objective markers; valid references and actionable validation failures; movement-profile reachability; and rejection of invalid content before match admission. Keep command versus action, authoring definition versus runtime state, and presentation versus simulation truth distinct from the beginning.

**Stage B — classical RTS economy and production.** Once the narrow content and map slice is dependable, develop resource flows, construction, production, prerequisites, technology, operational power/capacity, and appropriate gameplay tests. Do not prematurely commit to every economy or unit on the design roster.

**Stage C — strategic agency and observation.** Extend the verified foundation with per-participant vision, AI planning through the same command interface, faction relationships, territorial rules and objective logic. Ensure AI decisions do not bypass visibility or admission constraints.

**Stage D — differentiated POLITIKA mechanics.** Add trading/hiring contracts, sabotage, political permissions, propaganda, and intelligence provenance/false-belief handling as individually bounded and testable features. They should reuse the foundational identity, state, action, rule, and observation contracts.

These stages are a **dependency-oriented synthesis**, not a second binding roadmap. Milestone ownership remains with [First RTS Vertical Slice](RTS_VERTICAL_SLICE_MILESTONE.md), [Engine Outline](ENGINE_OUTLINE.md), [Development Foundations](DEVELOPMENT_FOUNDATIONS.md), and whatever implementation plan supersedes them. Project documentation may describe different stages of progress; verify implementation claims against code and executable tests before treating them as completed.

### Example ontology validation assertions

Future automated tests can use assertions such as:

1. Definition IDs are nonempty and unique within their namespace; references resolve and respect expected types.
2. A runtime entity has valid identity, applicable state, and unambiguous ownership/control policy.
3. Unowned or unauthorized actors cannot execute a participant's order.
4. An admitted command may lead to execution; a rejected command cannot alter authoritative state.
5. Movement legality and start reachability are evaluated for the intended movement class.
6. Production follows declared cost, prerequisite, time, and placement rules; invalid content fails before match start.
7. Power loss may disable operational capability without deleting or transferring the affected structure.
8. A participant cannot observe hidden truth through an AI or interface side channel.
9. Forged intelligence can change recipient information without modifying objective simulation facts.
10. Objectives and victory rules evaluate authoritative state, with explicitly defined treatment of reported/believed information when a scenario requires it.

Only implement tests that correspond to supported features; future-oriented entries are acceptance guidance, not assertions that all systems already exist.

## 13. Boundaries and explicit non-goals

- Do not define RTS by the presence of tanks, infantry, one specific resource, construction yards, workers, or base destruction.
- Do not impose a homogeneous C&C production mechanic on all factions merely for code reuse.
- Do not create a generalized schema or behavior engine larger than the actual verified gameplay needs.
- Do not equate `AActor` with every domain entity, or assume that every game entity is individually selectable.
- Do not mix faction identity, player slot, AI controller, owner, alliance and diplomatic stance into a single enum.
- Do not make fog-of-war rendering the authoritative source of knowledge or visibility.
- Do not inject fabricated military facilities into world state to represent deceptive reports.
- Do not claim deterministic multiplayer, fully tested cross-platform play, complete AI, or fully authored faction rosters solely because they are expressible in this ontology.
- Do not transplant proprietary C&C or StarCraft assets or engine code. Open-source architectural references require separate license and provenance review before code reuse.

## 14. Conclusion

The generalizable RTS foundation is best described by **world, time, entities, agents, actions, rules, resources, information, and objectives**, together with the identities, states, relationships, and state transitions connecting them.

The classical C&C style then emerges from a recognizable *configuration* of these fundamentals: extraction, base construction, production, technology, terrain-aware movement, combat, scouting, and victory conditions. StarCraft offers evidence that different factions can implement these shared abstractions through markedly different mechanisms.

For POLITIKA, the greatest practical advantage of this separation is the ability to retain C&C's immediate tactical accessibility while building distinctive systems for Unaligned world authority, Meridian Pact mercenary commerce, intelligence manipulation, and political consequence **without redefining what a unit, command, faction, objective, or simulation state means each time**.

## References and further reading

Primary, academic, and official documentation sources are listed first; community references are distinguished where used.

1. Abbadi, M., Di Giacomo, F., Orsini, R., Plaat, A., Spronck, P., and Maggiore, G. (2014), "[Resource Entity Action: A Generalized Design Pattern for RTS Games](https://doi.org/10.1007/978-3-319-09165-5_21)," *Computers and Games*, LNCS 8427, pp. 244–256. See also [institutional abstract](https://pure.buas.nl/en/publications/resource-entity-action-a-generalized-design-pattern-for-rts-games/).
2. Ontañón, S., Synnaeve, G., Uriarte, A., Richoux, F., Churchill, D., and Preuss, M. (2013), "[A Survey of Real-Time Strategy Game AI Research and Competition in StarCraft](https://doi.org/10.1109/TCIAIG.2013.2286295)," *IEEE Transactions on Computational Intelligence and AI in Games* 5(4), 293–311. [Institutional bibliography](https://www.wi.uni-muenster.de/research/publications/105500).
3. *Command & Conquer* historical manuals and game documentation, including [C&C: The First Decade manual](https://manuals.plus/wp-content/plugins/pdfjs-viewer-shortcode/pdfjs/web/viewer.php?attachment_id=&dButton=true&file=%2Fm%2Ffdbe4586d1f292357af0a3a96003970522af9498870e417586b92b3c119c8764_optim.pdf&oButton=false&pButton=true&sButton=false). Mechanics vary among C&C titles.
4. Blizzard, [StarCraft II: Game Guide — Resources](https://news.blizzard.com/en-us/article/4488900/game-guide-resources) and [Economy](https://news.blizzard.com/en-us/article/4488313/game-guide-economy).
5. Blizzard, [StarCraft II: Game Guide — Buildings](https://news.blizzard.com/en-us/article/4488317/game-guide-buildings).
6. Blizzard, [StarCraft II: Game Guide — Protoss Pylon Support](https://news.blizzard.com/en-us/article/5963100/game-guide-protoss-pylon-support).
7. Blizzard, [StarCraft II: Game Guide — Zerg Creep](https://news.blizzard.com/en-us/article/5838584/game-guide-zerg-creep).
8. Blizzard, [StarCraft II: Game Guide — Protoss Offensive Building](https://news.blizzard.com/en-us/article/5963102/game-guide-protoss-offensive-building).
9. StrategyWiki (community secondary source), [World in Conflict — Gameplay](https://strategywiki.org/wiki/World_in_Conflict/Gameplay).
10. Mohawk Games / Stardock, [Offworld Trading Company FAQ](https://www.offworldgame.com/game/faq) and [Gameplay](https://www.offworldgame.com/game/gameplay).
11. OpenRA project, [Trait Reference](https://docs.openra.net/en/release/traits/). Useful *implementation-pattern reference* for modular actor behavior, not evidence of a universal RTS ontology and not a POLITIKA runtime dependency.

### Related POLITIKA documents

- [Documentation Index](DOCUMENTATION_INDEX.md)
- [C&C Gameplay Comparative Study](CNC_GAMEPLAY_COMPARATIVE_STUDY.md)
- [Engine Outline](ENGINE_OUTLINE.md)
- [Proposed Engine Layers](PROPOSED_ENGINE_LAYERS.md)
- [Concepts Outline](CONCEPTS_OUTLINE.md)
- [Match Structure](MATCH_STRUCTURE.md)
- [Resource Economy](RESOURCE_ECONOMY.md)
- [Unaligned Counterintelligence](UNALIGNED_COUNTERINTELLIGENCE.md)
- [Validation and Governance](VALIDATION_AND_GOVERNANCE.md)
