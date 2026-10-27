# The Fundamental Ontology of Real-Time Strategy Games

**Status:** Comparative research and conceptual design reference (27 October 2026); not an implementation report or approved balance specification.  
**Research expansion:** Formal semantics, REA action types, resource accounting, strategic AI hierarchy, epistemic provenance, temporal processes, and validation-oriented invariants.  
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

## 13. A stricter ontology: classes, instances, relations, processes, and propositions

The nine foundational categories are useful for explaining the genre, but a formal content model needs a second classification: *what kind of thing is each concept?* A resource amount, faction, military unit, order, visibility relation, and intelligence report do not belong to the same logical category. Treating all of them as generic objects risks producing an apparently flexible but ambiguous architecture.

The following is a **POLITIKA design synthesis** drawn primarily from the Resource–Entity–Action (REA) model [1], the StarCraft AI survey [2], and OpenRA's practical actor/trait reference [11]. It is not an ontology published verbatim by those authors.

| Logical kind | Definition | Example | What it must not be confused with |
|---|---|---|---|
| **Type / definition** | An authored reusable specification | Wheeled reconnaissance vehicle type | An individual vehicle currently present |
| **Individual / instance** | An existing thing with identity and mutable state | Recon vehicle #1042 | Its unit archetype or 3D mesh |
| **Value / attribute** | A typed property of an individual or aggregate | Health 65/100; speed; inventory count | A separate controllable entity by default |
| **Relation** | A typed association among two or more referents | OwnedBy(vehicle, Pact); Controls(Concord, vehicle) | A permanent intrinsic property of the vehicle |
| **Capability / affordance** | An action available under particular circumstances | CanRepair, CanInfiltrate, CanProduceArmor | An already executing command |
| **Command / intention** | A requested change submitted by an agent | Move vehicle to bridge | Guaranteed successful execution |
| **Process / activity** | An unfolding behavior, possibly across many ticks | Traversing a road; constructing a bunker | A single instantaneous state update |
| **Event / transition** | A fact that a change occurred at a time | Vehicle destroyed at tick N | A command that might not have succeeded |
| **Constraint / rule** | A condition delimiting legal or valid states/actions | Aircraft cannot use ground-only movement profile | A UI convention |
| **Observation** | A sensor- or access-dependent perception | Recon unit directly sees factory | The entire authoritative world state |
| **Proposition / claim** | A statement that can be true, false, or unresolved | "A nuclear site exists at X" | The site's actual existence |
| **Goal / objective** | A predicate or ordered collection of conditions to achieve | Control the crossing for a required duration | A command issued in pursuit of the goal |
| **Representation** | A presentation of the simulation for a recipient | Unit icon, fog texture, audio cue | Source of authoritative gameplay truth |

A practical *type* system should distinguish authored IDs, ephemeral instance IDs, participant IDs, content-version IDs, and intelligence-report IDs. A *relation* should make role and lifetime explicit (ownership versus operational control, permanent versus temporary contract, current hostility versus past alliance). A *process* should specify start and completion conditions; an *event* should indicate the transition that actually happened.

### 13.1 Ontological dependence

Certain concepts presuppose others:

1. An **order** refers to an issuer, an initiator or controlled subject, an action kind, and a target specification.
2. An **execution** presupposes an admitted action and the state in which its prerequisites hold.
3. A **production job** presupposes an eligible production mechanism, a product definition, resource rules, capacity, time, and output/deployment policy.
4. A **diplomatic relation** presupposes two identified parties, a relation type, an effective period, and possibly a policy authority.
5. An **intelligence claim** presupposes propositional content, a source or asserted source, recipient eligibility, and an information history; it does *not* presuppose that the claim is true.
6. An **objective** presupposes a target condition and a rule for evaluating its fulfillment.

This dependence order should guide schema validation: definitions and references first; valid instances and starting configurations next; only then match admission and action execution.

### 13.2 The unit of abstraction is a responsibility, not a class name

The taxonomy neither mandates one universal *Entity* class nor requires a separate Unreal UObject for every noun. POLITIKA can implement the same logical contract with structs, actors, components, subsystems, or authored assets where justified. Conversely, two responsibilities can deserve separate state even when represented by one Unreal Actor.

## 14. Deriving additional mechanics from the REA paper

Abbadi and colleagues [1] intentionally compress RTS interaction into **resources, entities, and actions**. In their account, numeric battle characteristics (including life and attack strength) may be modeled alongside economic quantities as scalar resources; entities hold resource vectors; actions transform those vectors and may be visualized as edges in a directed weighted graph. Their illustrative action algebra applies a transformation matrix and a time integrator to source and target resource states.

This is a **powerful programming reduction**, but it is not a claim that every meaningful RTS object, rule, or proposition is literally a transferable commodity. For POLITIKA:

- Health, ammunition, credits, supply consumption, and stored fuel can have numerical state even though their physical and rules semantics differ.
- Some quantities flow between entities; others are created, consumed, recovered, regenerated, capped, or transformed.
- Ownership, nationality, legitimate authority, territorial access, classified knowledge, and whether an intelligence report is forged are not safely represented as arbitrary additive resource balances.
- A linear resource-transfer equation is an abstraction for selected updates; spatial collision, capture, assignment of command authority, discrete spawning, and many information effects need additional typed rules.

### 14.1 Three action families in the original REA pattern

The REA paper's own language design distinguishes *constant transfer*, *mutable transfer*, and *threshold* actions [1, §4.1]. POLITIKA can borrow the conceptual distinctions without copying its Casanova-specific action language.

| REA source concept | Mechanism | Familiar RTS case | Possible POLITIKA case |
|---|---|---|---|
| **Constant transfer** | A target resource changes without spending a corresponding source stock | Turret applies damage while its attack stat remains intact | Rifle weapon changes target health after a validated shot |
| **Mutable transfer** | Resources move between or change for both source and recipient | Harvester unloads carried material at refinery | Tanker delivers fuel to a depot; merchant transfers contracted stock |
| **Threshold action** | Accumulated state reaches a threshold and triggers a discrete transition | Construction completion after sufficient work | Objective secured after hold time; demolition charge arms after a delay |

*Important distinction:* The REA paper's "constant transfer" is about the source field not being depleted by that update. It does **not** imply that a weapon fires without a reload time, ammunition rule, or other gameplay constraint; such constraints may be modeled separately.

### 14.2 Transformations, transfers, and transitions

Consider these conceptual state transformations:

~~~text
A. Damage:
   Health(target)' = clamp(Health(target) - resolved_damage, 0, max_health)

B. Physical transfer:
   Cargo(vehicle)' = Cargo(vehicle) - delivered_amount
   Inventory(depot)' = Inventory(depot) + delivered_amount

C. Work accumulation:
   Work(building)' = min(required_work, Work(building) + work_rate * dt)
   if Work(building)' >= required_work:
       BuildingState := Completed

D. Permission / ownership transition:
   if CaptureRequirementsSatisfied(actor, target, state):
       Controller(target) := capturing_participant

E. Informational consequence:
   if ForgeryOperationSucceeds(operation, recipient):
       Reports(recipient) := Reports(recipient) ∪ {new_report}
       // No change to the asserted real-world facts.
~~~

These are **illustrative semantics**, not implemented algorithms or numerical balancing rules. Operation D changes a relation; E changes an information set. Neither needs to masquerade as a resource flow merely to fit one abstraction.

### 14.3 Action signatures and effects

A reusable action contract is best understood as:

~~~text
Action =
  identity
  + eligible initiator / authority
  + typed target selector
  + explicit preconditions
  + temporal mode (instant, durative, recurring, threshold)
  + cost / reservation policy
  + effects and event policy
  + interruption, cancellation and failure rules
~~~

Action effects should be *typed*: numerical change, creation/destruction, transfer, relation change, status change, information creation, and objective update are distinct kinds of consequence. This avoids a large untyped dictionary in which every mechanic manipulates arbitrary fields.

## 15. Resource ontology: stocks, flows, capacities, and constraints

The REA paper groups numerical fields into resources for a broad calculus [1]. Blizzard's StarCraft II guides [4–8] expose a different design-level point: harvesting income, worker allocation, supply, prerequisite buildings, and production time jointly restrict possible strategies. They should not be collapsed into one undifferentiated "currency."

| Kind | Definition | RTS example | Distinct invariant |
|---|---|---|---|
| **Stock** | Quantity currently held | Credits, ore, fuel, ammunition | Legal stock is normally nonnegative |
| **Flow** | Rate of change or throughput | Credits/minute, unloading rate, repair/sec | Rate and accumulated quantity have different units |
| **Capacity** | Bound on simultaneous use or holdings | Supply cap, storage, production slots | Used capacity must respect the applicable limit |
| **Consumable** | Quantity spent to create an effect | Ammunition round, building material | Expenditure must match payment rules |
| **Regenerating pool** | Quantity replenished through time/rules | Rechargeable ability energy | Change obeys a rate and maximum |
| **Provider network** | Service made available by one or more sources | Power grid, Protoss pylon field | Operability depends on coverage/availability, not just total credits |
| **Access right** | Permission to use a service or producer | Black Market eligibility, factory control | Cannot be treated as freely transferable numerical currency |
| **Time / opportunity** | Scarce interval or alternative use | Worker construction time, larva, queue time | Concurrent claims to the same opportunity need arbitration |
| **Information advantage** | Availability of relevant evidence or claims | Reconnaissance, intercepted plans | Information may be stale, incomplete, incorrect, or non-exclusive |

### 15.1 Units, conservation, and account boundaries

A sensible resource schema needs explicit **units and accounting scopes**. "25 fuel" has no definite meaning unless its unit and location (vehicle tank, depot, faction pool, contract escrow) are known. Distinguish global balances, local inventories, cargo, pending payments, reservations, and capacities.

Transfers should generally be atomic from the authoritative simulation's perspective: crediting one depot must not succeed if the corresponding removal from the vehicle fails. However, not all effects conserve a material sum: harvesting may convert deposits into credits; repair converts spent resources into condition; a periodic income source may mint currency under game rules. **Never impose material conservation where a game rule intentionally creates or destroys a quantity.**

A more useful rule is *balanced and explained transitions*: every economic balance change should correspond to an authorized source, sink, transfer, refund, or conversion event.

### 15.2 Production is a timed constrained transformation

Represent production conceptually as:

~~~text
Product = Produce(
  recipe_or_rule,
  eligible_producer,
  cost_and_reservation_policy,
  prerequisite_state,
  production_capacity,
  duration,
  deployment_conditions
)
~~~

Blizzard's Zerg hatchery/larva model [4–8] demonstrates that "producer" and "unlocks access to a product" are not necessarily the same structure. StarCraft II pylon power fields demonstrate that a source may provide **spatially contingent operability**. Its supply system demonstrates **population capacity**, a different constraint again.

OpenRA's documented *Buildable*, *Production*, *ProductionQueue*, and conditional trait properties [11] demonstrate one practical way to keep such dependencies authored rather than embedded as faction-specific special cases.

For POLITIKA, the same ontology could represent a factory manufacturing a vehicle, a merchant contract delivering a unit, or a scenario reinforcement providing a unit without locally manufacturing it. These are different **fulfillment mechanisms**, not contradictory definitions of production.

## 16. Space as topology, geometry, access, and influence

A map has at least four conceptually different layers:

1. **Geometry:** positions, polygons, elevations, footprints, intersections and sight lines.
2. **Topology:** connectivity between regions, roads, bridges, crossings, tunnels, air approaches and deployment entrances.
3. **Legality and access:** movement class, restricted area, authorized owner, traversability, occupancy, construction permissions.
4. **Strategic influence:** coverage by sensors, weapons, production networks, power fields, supply links, territory control and reinforcement.

The same coordinates can be reachable by aircraft, inaccessible to wheeled vehicles, hazardous to tracked vehicles, and blocked to infantry. A bridge can exist geometrically but be impassable due to destruction, congestion, enemy control, or scenario rule. Thus, physical adjacency is not identical to navigability.

### 16.1 Typed reachability

Let a movement profile *m* determine a directed graph *G(m, S)* built from current terrain, blockers, access rules and relevant dynamic state. A destination is legally reachable only if a permitted path exists from the source in that graph under the action's conditions.

This suggests separately validating:

- map bounds and positions;
- movement-profile/terrain compatibility;
- static start and objective connectivity;
- legal initial placements and unclipped footprints;
- dynamic occupancy and blocked deployment;
- scenarios in which a route is intentionally unavailable, but another legal route exists;
- changes after bridge destruction, capture or area denial.

**Static authoring validation** cannot guarantee that a path will remain reachable after dynamic obstacles or opposing units act. It can guarantee that the initial fixture's intended movement and start constraints are coherent. Actual navigation must still handle changing state.

### 16.2 Geometry itself becomes a strategic resource

The StarCraft guide to offensive building placement [8] shows how power coverage and proximity affect forward deployment. OpenRA exposes locomotor and terrain-related movement features [11]. Both illustrate that the map is not just decorative: geometry changes timing, risk, force projection, and feasible construction.

For POLITIKA, roads, railway corridors, river crossings, ridgelines, industrial zones and dense urban blocks should be modeled according to relevant gameplay effects, not merely labeled as historical scenery. See [Production Standards](PRODUCTION_STANDARDS.md) for the separate source-to-map provenance boundary.

## 17. Time ontology: commands, activities, scheduling, and interrupts

The StarCraft AI survey identifies **simultaneous decision-making and durative actions** as central sources of RTS difficulty [2]. Therefore the canonical runtime model cannot assume "one turn, one completed action."

A useful action lifecycle is:

~~~text
Requested → Validated / Rejected
Validated → Queued or Admitted
Queued → Started or Cancelled
Started → Continuing / Paused / Interrupted
Continuing → Completed / Failed / Interrupted
~~~

The exact states depend on the mechanic: immediate orders may bypass queuing; autonomous retaliation may create intent without a direct human order; periodic behaviors might never have a finite completion event. The *requested* and *committed* phases are conceptually separate even if implementation combines some of them.

### 17.1 Four clocks that should not be confused

- **Simulation time:** authoritative progression for gameplay effects, cooldowns, status and rules.
- **Presentation time:** rendering and animation interpolation, which may update at a different frequency.
- **Planning cadence:** how often AI recomputes high-level intentions or tactical plans.
- **External time:** wall-clock time for diagnostics, profiling, and user-interface latency; not automatically a valid source of gameplay truth.

This separation allows a game to render rapidly, simulate with controlled steps, and plan strategically less often, without conflating real-world timestamps with gameplay durations.

### 17.2 Competing commands and shared resources

Two units may target the same bridge; two producers may contend for a limited budget; a player may cancel construction while a completion threshold is reached; a hired unit's command contract may expire while an order is in flight.

These cases need explicit conflict policies: order precedence, reservations, cancellation refunds, reference invalidation, tie-break behavior, and atomic commit points. A command that was valid at submission may later fail if its target is destroyed or access changes. A match simulation should resolve that as an observable outcome, not reinterpret it as evidence that the original command never existed.

## 18. Agency and decision-making hierarchy

The StarCraft AI survey distinguishes **strategy**, **tactics**, and **reactive control**, describing them as a continuum of spatial and temporal scope [2, §II–IV]. This is more useful for POLITIKA than assuming there is one monolithic "AI logic."

| Decision layer | Horizon / scope | Main questions | Illustrative POLITIKA output |
|---|---|---|---|
| **Strategy** | Broad, longer-term, often map-wide | Invest in forces, infrastructure, intelligence or political objectives? | Prioritize border control over factory expansion |
| **Operations / tactics** | Regional, task-force, medium-term | Which route, force composition, target or position? | Secure a river crossing with armor and engineers |
| **Reactive control** | Local, short-term | Fire, evade, reload, reposition, interrupt? | Truck reroutes around a destroyed bridge |

The labels and horizons are analytical rather than fixed tick rates. High-level goals constrain lower-level decisions, while new observations can trigger replanning upward. This is consistent with the source's discussion of multi-scale abstraction and modular bot architectures [2].

### 18.1 AI is not synonymous with a faction

There can be:

- an AI controller occupying one of the **2–4 playable faction slots** in an offline match;
- a separate **Unaligned world-authority AI** acting according to its own permitted governance and security policies;
- subordinate tactical behaviors executed by individual entities;
- local scripted mission logic that triggers events without representing another playable participant.

These participants should use *the same authoritative command and rule admission layer* wherever they perform comparable operations. An AI must not bypass economy limits, movement legality, target eligibility, or information rights just because it is code-controlled.

### 18.2 The command ownership/arbitration problem

Different AI modules might simultaneously request control over the same patrol: defensive security wants it to guard a district, intelligence operations wants it to check a report, and local combat reaction wants it to evade fire. The literature on StarCraft bots explicitly discusses hierarchical assignment and arbitration between modules competing for shared units [2].

POLITIKA should define *who may issue the next effective order* independently from the unit's owner and faction label. Possible policies include task priority, explicit command delegation, contract-limited authority, or interrupt rules. The exact algorithm can wait until the behavior exists; the ontology should not conflate the rights.

## 19. Information as a temporal and evidential object

A model of information should distinguish at least:

- **World truth:** an entity or event actually exists/occurred in authoritative simulation.
- **Sensor observation:** a participant detected a phenomenon at a particular time.
- **Memory / last-known state:** information was observed earlier but may be stale.
- **Claim / report:** somebody asserts a proposition about world truth.
- **Evidence:** a record or artifact cited in support of a claim.
- **Inference / estimate:** an agent derives a likely explanation from available material.
- **Belief / decision state:** what an agent currently acts as if true.
- **Disclosure / distribution:** which recipients can access or repeat an item.

OpenRA's *FrozenUnderFog* behavior [11] offers a concrete illustration of remembered-but-not-updated world representation: a discovered structure may remain visible as a frozen representation even when it is outside current vision. That is **not the same as live observation**. The StarCraft AI survey's partial-observability framework [2] likewise makes the distinction between hidden game truth and participant observation essential.

### 19.1 Claims require referents, times, and provenance

An intelligence report can assert "vehicle convoy at crossing X" without permanently binding to an existing convoy entity. It may be mistaken, outdated, invented or accurate at an earlier time.

Conceptually, a report should distinguish:

~~~text
ReportIdentity
AssertedProposition: "Target facility exists at location X"
ClaimedObservationTime
ActualCreationTime
PurportedSource
ActualOrigin (authoritative/hidden where necessary)
RecipientOrDisclosureScope
VerificationState
ConfidenceOrReliabilityAssessment
LinkedEvidence
~~~

The recipient should never receive secret *ActualOrigin* or authoritative truth through a UI or AI shortcut unless a specific, legitimate discovery rule permits that information. The simulation may need hidden bookkeeping to support later forensic verification; the player-facing representation should follow information-access policy.

### 19.2 Distinct ways an actor may be misled

| Mechanism | What changes | What should remain unchanged |
|---|---|---|
| **Concealment** | No observation is delivered | Hidden unit's objective existence |
| **Outdated observation** | Last-known record ages without confirmation | Current, independently advancing world state |
| **False assertion** | Recipient gets a fabricated proposition | Referent's actual state |
| **Misattribution** | Apparent perpetrator/source differs from actual origin | The action's real actor and effects |
| **Selective disclosure** | Some recipients see evidence and others do not | The evidence's underlying identity/provenance |
| **Compromised sensor** | Sensor output differs from objective observation | The real map and physical entities |

Thus fog, decoys, false reports, propaganda, forgeries, and false flags are related but not identical mechanics. A future gameplay schema should not make one generic *Deceived = true* flag stand in for all of them.

### 19.3 Information integrity is a gameplay invariant

The core anti-leak rule is:

> Changing the recipient of a report must not change the underlying world; changing world state must not silently grant every participant full knowledge of the change.

Command rejection messages can also leak information. For instance, if an attack order against an unexplored location says "No enemy structure at that location," the game has inadvertently revealed hidden truth. The validation and feedback path must disclose only the information the requesting agent is permitted to know. The detailed player-facing UX remains owned by [UI Overview](POLITIKA_UI_OVERVIEW.md).

## 20. The ontology of force, damage, and combat

REA makes an important simplifying move by interpreting combat attributes numerically [1], but a useful RTS needs a richer account of *what a weapon does to a target*.

An engagement can be decomposed as:

~~~text
Permission / hostility
  → Target class compatibility
  → Observation / acquisition rights
  → Spatial eligibility (range, line of fire, orientation)
  → Weapon readiness (cooldown, ammunition, power)
  → Attack execution
  → Hit / effect resolution
  → Damage / status / displacement
  → Lifecycle and objective consequences
~~~

Every arrow may carry a failure or interruption condition. A target may cease to exist, move out of range, become protected, or change relation before the action resolves.

### 20.1 Damage is not destruction

A weapon may damage a vehicle without disabling or destroying it. A damaged structure may still produce units. A suppressed unit may remain alive yet lose a capability temporarily. A captured structure may remain intact but change controller. This supports the distinction between hit points, mobility, operational availability, current controller, and entity lifecycle.

### 20.2 Counterplay is a relational property

The claim "weapon A counters unit B" is not simply an intrinsic property of either archetype. It depends on armor/target compatibility, range, position, maneuverability, readiness, available information, surrounding units, and opportunity cost. OpenRA's condition-gated traits and target types [11] illustrate mechanisms for expressing some of these conditions declaratively.

POLITIKA's authored rosters should describe combat roles and intended counters, while keeping actual combat behavior in tested rule code; see [C&C Gameplay Comparative Study](CNC_GAMEPLAY_COMPARATIVE_STUDY.md).

## 21. Production, technology, and infrastructure as dependency graphs

The classical RTS tech tree is better described as a *dependency graph of capabilities* than a mere list of upgrades.

Nodes may represent structures, research, faction privileges, resource providers, power coverage, controlled locations, or mission conditions. Edges may mean **requires**, **unlocks**, **enables while present**, **consumes**, **excludes**, or **replaces**. These edge types should not be conflated.

For example:

~~~text
FactoryExists AND HasCredits AND HasProductionCapacity
    → MayQueueVehicle

VehicleCompleted AND DeploymentCellLegal
    → MaySpawnVehicle

PylonFieldCoversStructure
    → StructurePoweredWhileCoverageHolds
~~~

The last example is based on Blizzard's Protoss power-field documentation [6]; it is not a proposed power rule for POLITIKA. It illustrates a *continuous dependency* rather than a permanently unlocked technology.

### 21.1 Four kinds of dependency

1. **Historical unlock:** once a condition is achieved, the capability persists (if rules say so).
2. **Current-state requirement:** a capability is available only while a prerequisite remains valid.
3. **Spatial requirement:** validity depends on position, coverage or connectivity.
4. **Exclusive constraint:** obtaining one capability forbids another under a ruleset.

A correct authoring validator should detect missing references, contradictory requirements, impossible cycles, and unreachable objectives. A cycle is not inherently wrong in every possible state machine, but an unconditional cycle of locked prerequisites with no initial satisfiable entry point is an authoring error.

### 21.2 Production is not identical to ownership transfer

A new unit might be locally built, morphed, spawned by mission reinforcement, delivered by a merchant, purchased from a market, or temporarily hired. These operations may all result in a new deployable force, but differ in provenance, cost, delay, ownership, and rights. Offworld Trading Company's market-driven competition [10] provides a useful counterexample to treating every strategic asset as the output of a military factory.

For POLITIKA, this permits a classic C&C economy and factory loop alongside the Meridian Pact's distinctive contractual procurement role without making either a universal rule.

## 22. Normative rules: permissions, hostility, and political authority

A political RTS needs a separate category of **normative game state**: who is allowed to do what, where, under which institutional or contractual conditions.

This is different from physical capability. A truck can *physically* cross a bridge, but a rule may forbid passage. An infantry unit may *technically* fire on a police patrol, but doing so may violate a neutrality agreement or trigger federal retaliation. A faction can *own* a mercenary vehicle but lack authorization to command it beyond a contract.

Useful predicates might include:

~~~text
PhysicallyPossible(action, world)
Capable(initiator, action_kind)
Authorized(controller, subject, action)
PermittedByScenario(action, zone, time)
HostileTo(initiator, target, relation_state)
ContractValid(contract, time)
ObservedOrDiscoverable(recipient, target, policy)
~~~

Which of these are required, and whether a disallowed action is blocked or merely penalized, are **design decisions**. The ontology must support both possibilities. For instance, prohibited violence may be physically possible yet produce political sanctions rather than being universally disabled.

This distinction is especially important for [Match Structure](MATCH_STRUCTURE.md): the Unaligned is an always-present NPC federal authority, not a selectable fifth player. Its administrative power need not be modeled as an infinitely strong conventional combat faction. Specific neutrality, enforcement and intervention mechanics remain in their specialist design documents.

## 23. Goals, winning, and strategic evaluation

Objectives should be expressed as state predicates (or temporal conditions), not hard-coded assumptions that victory always follows from destroying a headquarters.

| Objective form | Example | Evaluation issue |
|---|---|---|
| **Elimination** | Remove all qualifying enemy command assets | What counts as an eligible surviving asset? |
| **Acquisition / capture** | Gain control of a railway terminal | Does mere presence count, or must control transition? |
| **Hold / duration** | Maintain control for a period | Does interruption reset, pause or preserve progress? |
| **Production / technological** | Complete a specified strategic weapon | Must the product be created domestically or merely possessed? |
| **Delivery / extraction** | Bring an asset to an exit | Can an allied unit or NPC complete delivery? |
| **Survival / defense** | Protect an institution until a deadline | Which states constitute loss? |
| **Information / disclosure** | Obtain corroborated proof | Does rumor or forged evidence qualify? |
| **Economic / political** | Achieve a market or authority condition | Are holdings evaluated continuously or at checkpoints? |

The Iron Ascendancy's planned *first own-produced nuclear weapon* victory condition provides an especially sharp ontological test. **Possessing**, **stealing**, **deploying**, **detonating**, and **producing** a weapon are distinct event and provenance types. An objective defined as "produce first own weapon" cannot be correctly evaluated by merely checking "owns any nuclear bomb." The owning [Nuclear Weapons System](NUCLEAR_WEAPONS_SYSTEM.md) and [Concepts Outline](CONCEPTS_OUTLINE.md) define the actual scenario rule.

### 23.1 Goals and plans are different kinds of things

A strategic AI may pursue a goal by constructing forces, trading, reconnaissance, or coercion. These are alternative *plans*, not different definitions of the goal. If a scouting report invalidates the plan, the objective may remain unchanged while the chosen sequence of actions changes.

This fits the hierarchy described by Ontañón et al. [2]: strategic, tactical, and reactive processes operate at different horizons, with feedback between them.

## 24. Player interaction is a control interface, not a world ontology

Classic C&C's sidebar, StarCraft's command card, and controller/touch-oriented interfaces can expose overlapping game operations without defining the underlying simulated effects. Separate:

1. **Input gesture:** click, tap, button press, radial menu selection, hotkey.
2. **Selection context:** which entities the local player intends to address.
3. **Command intent:** typed request with targets and optional queue modifiers.
4. **Admission result:** accepted, rejected, or accepted with a visible limitation.
5. **Execution state:** movement, progress, cancellation, interruption.
6. **Presentation feedback:** sound, icon, animation, status indicator or reasoned rejection.

POLITIKA's planned Linux, Windows, SteamOS/Steam Deck and Android controls should submit the same canonical command grammar while differing in input mapping and layout. This avoids placing mouse-specific semantics into the authoritative match rules; see [Platform Support](PLATFORM_SUPPORT.md) and [UI Overview](POLITIKA_UI_OVERVIEW.md).

## 25. Worked cross-game interpretations

The following examples test whether the ontology explains actual mechanics rather than only listing labels.

| Example | Entities and identities | Resources / constraints | Actions and relations | Information / objective |
|---|---|---|---|---|
| **Classic C&C harvester unload** | Harvester, ore deposit, refinery | Cargo stock, refinery/credit conversion, time | Gather, transport, deliver; refinery access | Enemy raid may be detected; objective is increased production capability |
| **StarCraft Zerg larva production** | Hatchery, larva, selected product | Larva availability, minerals/gas, supply, build time | Morph/produce; unlocked morph types differ from direct producer | Scouting alters whether larva go to workers or army |
| **StarCraft Protoss pylon coverage** | Pylon, dependent structures, unit/warp point | Power-field coverage, supply, prerequisite states | Construct/warp; provider-to-dependent relation | Loss of provider can disable production |
| **OpenRA conditional trait** | Actor with a condition-gated capability | Prerequisite availability and condition state | Condition enables/disables trait | Presentation and legal order choices reflect effective state |
| **Offworld market competition** | Claims, producers, participants, market | Cash, inventories, production, prices | Buy, sell, build, sabotage, invest | Prices convey information; market domination replaces ordinary military conquest |
| **POLITIKA mercenary contract** | Pact asset, Concord customer, contract | Price, duration, access and deployment rights | Hire, transfer temporary control, terminate | Political and ownership identity differ; exact mechanic is proposed |
| **POLITIKA false intelligence** | Report, source, recipient, alleged site | Credibility, provenance, detection capability | Forge, disseminate, assess, verify | Objective world truth remains unchanged |

Sources: REA [1]; StarCraft research and official guides [2, 4–8]; OpenRA [11]; Offworld [10]. POLITIKA rows are *design applications*, not claims about currently running systems.

## 26. The ontology as a validation and test taxonomy

An abstract definition is useful only if it helps determine what must never go wrong. The following tests are **future-oriented specifications**; they should be adopted incrementally once a corresponding executable rule exists.

| Invariant family | Question | Example negative test |
|---|---|---|
| **Identity and type** | Are all required IDs/references well formed and correctly typed? | Weapon references nonexistent target class |
| **Existence and lifecycle** | Can destroyed/replaced instances still receive illegal actions? | Attack proceeds after target ID invalidation |
| **Authority and ownership** | Does controller hold a valid permission? | Pact mercenary accepts expired customer's order |
| **Temporal safety** | Are delays, cooldowns, cancellations and commits coherent? | One canceled job both refunds and produces a unit |
| **Resource accounting** | Are funds and stock changes lawful and scoped? | Concurrent purchases both spend same reserved balance |
| **Spatial legality** | Are starts and actions valid for intended movement profile? | Wheeled spawn accessible only via aircraft route |
| **Information non-leakage** | Does observation respect access scope? | Rejected fog-of-war order reveals hidden building |
| **Provenance integrity** | Can a report's asserted content alter real facts? | Forged report causes real facility to spawn |
| **Objective correctness** | Do event/provenance distinctions match win conditions? | Stolen bomb satisfies own-produced-bomb victory |
| **Presentation separation** | Are visual effects independent of authoritative changes? | Disabling fog material grants actual reconnaissance |
| **Serialization and replay** | Can snapshots preserve relevant identity and state meaning? | Loaded contract changes owner or expires incorrectly |
| **Cross-feature interference** | Does one mechanic accidentally change unrelated guarantees? | Power outage changes legal faction affiliation |

### 26.1 Validation layers

1. **Static asset validation:** required fields, stable IDs, enum/range checks, resolved references, version compatibility.
2. **Map semantic validation:** bounds, legal starts, spawn occupancy, movement-class access, faction slots, objective references and feasible initial conditions.
3. **Command admission tests:** rights, target typing, visibility/exploration policy, budget, cooldown, range and action preconditions.
4. **Simulation transition tests:** state deltas, transfer atomicity, damage, interruptions, completion and event emission.
5. **Multi-agent fixtures:** conflicting commands, AI observation limits, participant-versus-Unaligned relations.
6. **Information integrity fixtures:** false reports, stale observations, hidden truth and anti-leak error messages.
7. **Cross-platform presentation checks:** different inputs produce equivalent canonical commands, while runtime authority stays shared.

These layers are intentionally separable: a Rust metadata validator cannot prove an Unreal movement or animation invariant, and a visually clean playtest cannot by itself prove correct authoritative state. See [Validation and Governance](VALIDATION_AND_GOVERNANCE.md).

### 26.2 High-value metamorphic tests

Some domain laws can be checked without assuming one exact balance outcome. For example:

- **Cosmetic independence:** changing a mesh/animation must not change authoritative combat results under equivalent simulation inputs.
- **Observer separation:** changing one participant's intelligence reports must not alter the objective state for other participants unless authorized actions follow.
- **Identity stability:** balance changes may alter a type's values but must not silently rename its stable ID.
- **Controlled transfer:** replacing a legitimate delivery with two smaller deliveries should preserve total cargo/accounting under rules where splitting is permitted.
- **Movement specificity:** adding an aircraft-only path must not make a ground spawn legally reachable.
- **Unrelated relation independence:** changing a merchant contract should not rewrite the vehicle archetype or faction of origin.

These are proposals to guide future verified behavior, not a request to build a second simulator or a universal property-testing framework immediately.

## 27. Source-to-ontology traceability and limits

The sources serve different purposes and should not be cited as if all of them prove the same thing.

| Source | Directly supports | Added POLITIKA interpretation | Important limitation |
|---|---|---|---|
| **Abbadi et al., REA [1]** | Numerical resources, entities, resource-transforming actions; three transfer/threshold action forms | Typed action effects and explicit resource/state distinction | Not a complete theory of political authority, semantics, perception or all discrete game mechanics |
| **Ontañón et al. [2]** | Durative concurrent actions, partial observability, AI decision complexity, strategy/tactics/reactive hierarchy | Command/observation contracts and layered NPC/participant AI | AI survey is not a prescriptive Unreal design specification |
| **C&C historical references [3]** | Particular military/base/economy/production traditions | Retain an immediately readable classic RTS interaction grammar | Mechanics vary strongly by C&C title; no proprietary architecture is inferred |
| **Blizzard guides [4–8]** | Harvesting, economy, worker-specific construction, larva, pylon fields, creep, forward deployment | Separate production, prerequisites, local coverage, population and information | Race rules are examples, not default requirements for POLITIKA |
| **World in Conflict [9]** | Reinforcement-based tactical focus near genre boundary | Distinguish RTS ontology from mandatory base construction | Secondary genre comparison, not evidence for every RTS |
| **Offworld Trading Company [10]** | Real-time market competition without conventional armies | Recognize nonmilitary strategic transformations and market roles | Does not determine POLITIKA's actual Black Market economy |
| **OpenRA traits [11]** | Real implementation of modular movement, production, conditions, fog memory and prerequisites | Stable authored capability composition and validation inspiration | A specific open-source architecture; licensing, version and engine fit must be reviewed separately |

**Methodological conclusion:** REA offers a compact *interaction calculus*; the StarCraft AI survey explains the *decision problem*; official game guides provide concrete *mechanical counterexamples* to overfitted abstractions; OpenRA demonstrates one *implementable composition strategy*. POLITIKA's proposed richer ontology arises by combining and extending these perspectives. It is not claimed to be an empirically verified or uniquely necessary taxonomy.

---

## 28. Boundaries and explicit non-goals

- Do not define RTS by the presence of tanks, infantry, one specific resource, construction yards, workers, or base destruction.
- Do not impose a homogeneous C&C production mechanic on all factions merely for code reuse.
- Do not create a generalized schema or behavior engine larger than the actual verified gameplay needs.
- Do not equate `AActor` with every domain entity, or assume that every game entity is individually selectable.
- Do not mix faction identity, player slot, AI controller, owner, alliance and diplomatic stance into a single enum.
- Do not make fog-of-war rendering the authoritative source of knowledge or visibility.
- Do not inject fabricated military facilities into world state to represent deceptive reports.
- Do not claim deterministic multiplayer, fully tested cross-platform play, complete AI, or fully authored faction rosters solely because they are expressible in this ontology.
- Do not transplant proprietary C&C or StarCraft assets or engine code. Open-source architectural references require separate license and provenance review before code reuse.

## 29. Conclusion

The generalizable RTS foundation is best described by **world, time, entities, agents, actions, rules, resources, information, and objectives**, together with the identities, states, relationships, and state transitions connecting them.

The classical C&C style then emerges from a recognizable *configuration* of these fundamentals: extraction, base construction, production, technology, terrain-aware movement, combat, scouting, and victory conditions. StarCraft offers evidence that different factions can implement these shared abstractions through markedly different mechanisms.

For POLITIKA, the greatest practical advantage of this separation is the ability to retain C&C's immediate tactical accessibility while building distinctive systems for Unaligned world authority, Meridian Pact mercenary commerce, intelligence manipulation, and political consequence **without redefining what a unit, command, faction, objective, or simulation state means each time**.

## 30. References and further reading

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

- [Yugoslav World and Campaign Systems Model](YUGOSLAV_WORLD_MODEL.md) — setting-specific translation of the general model into historical institutions, geography, fictional evidence and campaign stages; not a duplicate engine schema.
- [Documentation Index](DOCUMENTATION_INDEX.md)
- [C&C Gameplay Comparative Study](CNC_GAMEPLAY_COMPARATIVE_STUDY.md)
- [Engine Outline](ENGINE_OUTLINE.md)
- [Proposed Engine Layers](PROPOSED_ENGINE_LAYERS.md)
- [Concepts Outline](CONCEPTS_OUTLINE.md)
- [Match Structure](MATCH_STRUCTURE.md)
- [Resource Economy](RESOURCE_ECONOMY.md)
- [Unaligned Counterintelligence](UNALIGNED_COUNTERINTELLIGENCE.md)
- [Validation and Governance](VALIDATION_AND_GOVERNANCE.md)
