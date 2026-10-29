# POLITIKA — Structures, Production Queues and Technology Contract

**Status: PROPOSED system architecture, 29 October 2026.** Owns the gameplay mechanics for producing/placing RTS entities and gating recipes. [Resource Economy](RESOURCE_ECONOMY.md) owns commodity hierarchy and conversion/economic balance; [Faction Rosters](POLITIKA_FIVE_FACTION_ROSTERS.md) own candidate content; [Gameplay Content](GAMEPLAY_CONTENT.md) owns existing typed assets and schema validation; [Engine Outline](ENGINE_OUTLINE.md#75-stage-4--first-playable-skirmish-repeatable-offline-match) owns implementation stage. Existing `TrySpend` in `Source/PolitikaCore/Rules.h` is a portable resource helper, **not** a complete production or construction implementation.

**Adopted dollar/upkeep requirement (29 October 2026; NOT IMPLEMENTED):** [Dollar Economy, Commodity Sales, Building Power and Military Food](RTS_OPERATIONAL_ECONOMY_AND_POWER.md) locks **USD ($)** as the only ordinary purchase denomination, with authorized gold/fossil sales, **positive minimum power per operational building**, and **military food reserves purchased with dollars**. This supersedes a separate Operational Budget. Keep the existing 4B source/delivery→USD→production proof and add a bounded powered base, dollar-funded food purchase and per-step upkeep/shortage fixture before the full skirmish. Do not create independent currency, power or food ledgers in each producer.

## 1. Minimal 4B loop and typed responsibility

Implement one source/delivery or grant to a participant-owned **USD-cents treasury**, a fixed starting producer with declared **positive electricity minimum** and actual supply, one **dollar-priced** recipe and reinforcement, and an initial staffed force with purchased **food reserve** and one due upkeep interval. Author explicit power deficit and food shortage/recovery fixture cases. The starting structure does not require free-form placement. Gold mining, live fossil sale markets, refinery chains and regional grids are separate expansion slices.

Proposed future authoring concepts (only materialize versioned Unreal definitions when used): `ResourceDefinition` (stable ID, unit, storage policy), `StructureDefinition` (footprint, faction, buildable/capturable flags, production lanes), `ProductionRecipe` (stable recipe ID, product ID, required structure/capabilities, nonnegative costs, duration, output placement contract), `TechnologyDefinition` (prerequisites/unlocks) and `MapEconomicSetup` (starting balances, resource nodes, fixed bases). Keep **definition IDs**, **runtime entity IDs**, **participant IDs**, **order correlations** and **match generation** separate. Existing map schema cannot be assumed to have these fields.

## 2. Transactions, ownership and production state

Production follows a single authoritative coordinator sequence: validate participant and operational **powered** producer; validate the available dollar-priced recipe/prerequisites and a lawful $ balance; atomically reserve/debit **integer USD cents**; commit a queue item with unique ID and bounded duration; on authorized simulation ticks advance it; on completion validate an allowed spawn near the producing structure, allocate runtime ID and owner/faction, and expose the product to normal control. Never charge in UMG, duplicate the ledger inside the AI, or spawn before admission. Invalid or stale command attempts must leave ledger, reservations, queue, occupancy and unit count unchanged.

Candidate queue item lifecycle: `Requested → Admitted → Queued → Producing → Completed`, with terminal `Rejected/Cancelled/ProducerLost/SpawnBlocked` outcomes. Time is measured in authoritative steps, not frame time. Define a single ordering rule for simultaneous deposits, production completion and refunds. If output spawn is blocked, either keep the completed item pending with bounded retry/status or abort using a specified refund policy; **do not silently delete resources or clip the spawned unit into obstacles**.

**USD accounting:** available cents, reserved cents and spent cents must reconcile. Food is a separate consumable supply reserve and electricity is a capacity/load constraint, neither a second spending currency. Authorized commodity sale has its own atomic stock-debit/USD-credit settlement. Reserve-or-debit should be atomic; no negative amounts, overflow, overdrafts, unrelated participant access or double charges. Choose up front whether cancellation refunds reserved-only, consumed fractions or a fixed authored fraction; integer fixed-point quantities are preferable to float currency unless clearly justified. Save/replay needs unambiguous queue transaction IDs and the original definition revision; only after its own implementation gate.

## 3. Structure foundations and construction (later stage)

A structure has authoritative ownership/faction, health, construction state, producer capacity, occupancy/footprint, power/logistics modifiers where implemented, and a distinct visual representation. Proposed phases: preview (pure query) → request → authoritative placement validation → resource reservation → construction → operational → damaged/disabled/captured/destroyed. The placement validator must check: legal authored playable region; correct map and movement/placement layer; slope/elevation and collision; overlap with units, roads, objectives and protected civilian structures; supply/territorial permissions; adjacency or connection rules if authored; path continuity for required movement classes; resources and builder permissions.

Reserve site coordinates/footprint transactionally at the moment of acceptance; deterministic same-step contention gets exactly one winner. Preview UI may be stale and can never bypass authoritative final checks. On capture, explicit transition resolves producer queues and owned units without spoofing a participant's roster; the Unaligned faction is not converted to a playable slot. On destruction, retire outstanding production and footprint/nav obstacle and publish ordered events. Any Recast rebuild must respect the existing coordinator and route-invalidation contract; no direct route-free teleport fallback.

## 4. Technology and capability graph (deferred)

**Specialist research specification (proposed, not implemented):** [Technological Research, Upgrades and Building Modernization](RTS_TECHNOLOGICAL_RESEARCH.md) defines the building-hosted research ontology, participant-wide technology ownership, individual building modernization, research queues, faction and nuclear gates, and test roadmap. It does not pull general research into Stage 4B or supersede this document's production/transaction authority.

A technology or building prerequisite is a dependency edge over **stable content IDs**, not a screen label. Validate all references exist, that the graph is acyclic for mandatory prerequisites, and that at least one legally reachable starting state can reach every enabled non-hidden recipe; isolate deliberately exclusive branches. Ownership, faction exclusivity and espionage unlocks are independent predicates. The nuclear and Black Market systems have separate restrictions and may only be integrated when their owning specifications and tests are ready.

Author actual cost/duration/build radius values in content assets, not in this document. Do not create a general `GameplayAbilitySystem`/Gameplay Tags dependency for ordinary production; [GAS](https://dev.epicgames.com/documentation/en-us/unreal-engine/understanding-the-unreal-engine-gameplay-ability-system) is useful for rich ability/effect systems, but is not automatically the appropriate authority for a discrete RTS economic ledger. [Asset Management](https://dev.epicgames.com/documentation/unreal-engine/asset-management-in-unreal-engine) and [Data Validation](https://dev.epicgames.com/documentation/unreal-engine/data-validation-in-unreal-engine) support the existing stable-ID authoring approach.

## 5. Regression and acceptance gates

| ID | Positive and negative check |
| --- | --- |
| PRD-01 | Missing/duplicate/stale recipe and producer IDs fail content/startup validation |
| PRD-02 | Resource account has valid participant owner, integer/fixed-point safe amounts, no negative or overflow |
| PRD-03 | One deposit applied once; invalid/double delivery leaves state unchanged |
| PRD-04 | Insufficient funds or wrong owner rejects before reserving and spawning |
| PRD-05 | Valid purchase debits/reserves exactly once and creates one queue entry |
| PRD-06 | Different render-frame schedules complete at the same authoritative step |
| PRD-07 | Produced unit has correct new ID, faction, owner and command authority |
| PRD-08 | Producer death/capture, cancellation, bad spawn and route blockage follow explicit refund/block policy |
| PRD-09 | Two same-step purchases cannot overspend; contention order deterministic |
| PRD-10 | Terminal/rejected/rematch clears queues, reservations, producers and participant balances without affecting a new generation |
| PRD-11 | Normal authored content package includes the definitions; invalid fixture package rejects atomically; non-fixture package excludes test assets |
| PRD-12 | Rendered package: valid delivery/income → actual USD → purchased food/paid upkeep → powered producer → $-priced queue → spawn → order → combat |
| PRD-13 | Every functional producer has positive minimum electricity load, power is allocated from valid supply and outage/repair pauses/resumes queues without charging again |
| PRD-14 | Deployed staffed forces consume food once per due step; USD-to-food purchase is atomic and initial reserves are adequate; deficits are recoverable |
| PRD-15 | Gold/fossil sale, when implemented, consumes actual unreserved stock and credits quoted net USD exactly once; rejects nuclear goods |

Later: `BLD-01` legal/illegal placement, `BLD-02` same-tile contention, `BLD-03` path obstruction/rebuild, `BLD-04` capture/destroy/restore; `TECH-01` cycle/unreachable prerequisites, `TECH-02` faction grants/blocks, `TECH-03` tech changes do not retroactively mutate unrelated queues.

**Non-goals for 4B:** multi-region power grid (a minimal generator/positive building load is mandatory), mobile bases, supply cap beyond food upkeep, refinery chains, dynamic gold/fossil markets, technology research, agriculture, population morale, Black Market, factory parallelism and full faction asymmetry. Those are genre options or later POLITIKA design, not prerequisites for a credible minimal economic loop.
