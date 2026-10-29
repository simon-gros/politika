# POLITIKA — Technological Research, Upgrades and Building Modernization

**Status:** PROPOSED gameplay and technical design specification; **not implemented**.  
**Research and drafting date:** 29 October 2026.  
**Setting:** Alternate-history Yugoslavia, 1984–1989.  
**Design intent:** Make research a core element of development *within existing functional buildings*. No chronological age, era, civilization-level promotion, or compulsory universal technology tier.

**Authority and scope.** This document owns the proposed ontology and implementation contract for ordinary technological research, participant-wide upgrades, individual building modernization and technology-linked capability unlocks. It does **not** change canonical faction permissions, the nuclear victory rules, the USD economy, the original building rosters, the main engine milestone order, or the commander talent system. For those authorities see [Structures, Production and Technology](RTS_PRODUCTION_AND_TECHNOLOGY.md), [Resource Economy](RESOURCE_ECONOMY.md), [Dollar Economy, Power and Food](RTS_OPERATIONAL_ECONOMY_AND_POWER.md), [Five-Faction Rosters](POLITIKA_FIVE_FACTION_ROSTERS.md), [Nuclear Weapons](NUCLEAR_WEAPONS_SYSTEM.md), [Espionage](ESPIONAGE_AND_INTIMIDATION.md), [Commander Talents](COMMANDER_TALENT_SYSTEM.md), [UI Overview](POLITIKA_UI_OVERVIEW.md) and the [Engine Outline](ENGINE_OUTLINE.md). Research is **future scope**, not evidence of a working game system.

## 1. Executive design decision

POLITIKA should combine three established RTS patterns:

1. **Command & Conquer:** technologies and practical building improvements are researched or installed at relevant operational facilities, and completed shared upgrades often affect existing and future units.
2. **StarCraft:** independent military branches, meaningful investment decisions, research prerequisites and competing production capacity make specialization strategically consequential.
3. **Age of Empires / Warcraft:** separate military, industrial and economic research domains improve structures and units without every technology being a direct unlock for a new combatant.

Adopt those mechanics **without** Age of Empires-style ages, Civilization-style eras, arbitrary chronological epochs, or a mandatory global Tech Level 1/2/3 ladder. Progression occurs through a **validated dependency graph of specific capabilities**, constrained by operational buildings, participant ownership, faction, money, electricity and elapsed simulation time. Technologies should produce visible, auditable changes to existing RTS mechanics, not constitute an isolated minigame or passive collection system.

The core design distinction is **research knowledge belongs to a participant, while local physical modernization belongs to a building instance**. Completing a general infantry technology once should normally improve all eligible present and future infantry. Installing a new generator module should modify only the chosen generating station. Some research grants permission to install or manufacture equipment; it does not conjure physical buildings or bypass costs.

## 2. Comparative RTS research: transferable mechanisms

| Game / family | Observed pattern | Transfer to POLITIKA | Exclude or constrain |
| --- | --- | --- | --- |
| *Command & Conquer: Generals / Zero Hour* | Research and development distributed among military facilities; support facilities and doctrine choices shape operational play | Barracks, factory, airfield, command and specialist facilities host relevant technologies | Do not copy proprietary tech trees, artwork, nomenclature or prices |
| *Command & Conquer 3: Tiberium Wars* | Research at a specified faction structure; completed improvements apply to eligible existing and future units even after the original facility is lost | Persistent participant-level research acquisition, independent from the building that originally hosted it | Distinguish persistent research from a powered service dependent on a living structure |
| *Command & Conquer: Tiberian Sun* | Physical power-plant turbine additions improve particular structures | Per-instance power and industrial modernization | Avoid treating every local equipment installation as a global unlock |
| *StarCraft II* | Engineering Bay / Armory / production Tech Lab divide upgrades by production branch; cross-branch upgrades have substantial opportunity cost | Domain-specific research hosts and strategic concentration of funding | Do not impose race-specific SC2 structures or complexity |
| *Age of Empires II / IV* | Research at specialized economic and military buildings; upgrades affect various classes of entities | Broad military, economic, defensive and infrastructure improvements | **No age advancement or chronology-gated technology sets** |
| *Warcraft III* | Building-specific upgrades and successive improvements within limited branches | Short, readable improvement chains | Avoid pervasive repetitive +1 upgrades |
| *Company of Heroes 3* | Infantry, mechanized and air support choices allow costly differentiation | Optional, explicitly exclusive doctrine or facility specializations | Do not make all research branches mutually exclusive |
| *Supreme Commander* | Factory and energy infrastructure have major upgrade paths | Industrial modernization affects the macroeconomy | Do not copy an escalating multi-tier economy that displaces POLITIKA's USD model |

These are analytical correspondences, **not** a claim that all games share one engine implementation. The research bibliography in §17 includes directly checkable reference material.

## 3. Minimal research ontology

### 3.1 Classes, instances, knowledge, permissions

| Concept | Type / owner | Meaning |
| --- | --- | --- |
| Technology definition | Versioned authored content | Stable-ID description of what may be researched and what completion grants |
| Research project | Runtime transaction / queue item | Paid, scheduled development activity hosted by a valid building |
| Completed technology | Participant-owned match state | Persistent knowledge/capability entitlement after valid completion |
| Research host | Runtime building plus authored capabilities | Building may initiate and sustain only eligible research |
| Capability grant | Participant, unit class, or valid entity scope | Permission to execute an action, offer a recipe or activate a bounded effect |
| Unit upgrade effect | Participant-level eligibility applied to units | Modifies eligible current and newly produced units |
| Building modernization definition | Authored equipment / installation recipe | Defines installable improvements and compatibility |
| Installed building modernization | Building-instance state | Local physical modification with installation and removal/transfer rules |
| Technology prerequisite | Static predicate over stable IDs and allowed states | Gate for research admission, distinguishable from operation |
| Operational dependency | Dynamic condition | Ongoing access to electricity, functional radar, intact connection or other service |
| Intelligence acquisition | Provenance-bearing information event | May reveal, unlock or substantiate *explicitly authorized* technology permissions |
| Doctrine | Deliberate strategic policy selection | Optionally exclusive research-like commitment, distinct from Elite commander talent |
| Research observation | Player/AI-facing sanitized view | What the participant is authorized to see, not unrestricted simulation truth |

**Never conflate:** owner and controller; technology and equipment; research completion and building construction; building presence and building operability; true enemy research state and intelligence report; unlocked recipe and deployed unit; commander talent and army-wide research.

A valid research completion, written abstractly, is:

ResearchComplete(participant, technology, host)  
= AuthorizedOwner ∧ EligibleHost ∧ PrerequisitesSatisfied ∧ ValidReservedCost ∧ SufficientAccumulatedWork ∧ MatchingGeneration.

Completion commits the participant-level technology exactly once and applies its authored effect semantics. A new unit's effective attributes are derived from its definition plus currently applicable modifiers; research does not mutate the underlying definition asset.

### 3.2 Research effect families

- **Shared attribute modifier:** attack, armor, sight, repair throughput, training time, upkeep-related efficiency (only where separately specified).
- **Capability grant:** new command, ability, operational mode, specialist equipment eligibility or authorized tactical option.
- **Production unlock:** makes an otherwise valid unit/building/repair recipe eligible; the recipe remains dollar-priced and subject to normal producer/placement validation.
- **Modernization unlock:** permits local installation at compatible structures or service depots.
- **Economic transform modifier:** changes actual fuel processing, storage, sale handling or industrial conversion under the authoritative economy rules.
- **Information-system modifier:** improves radar processing, communications or counterintelligence without revealing hidden world truth.
- **Doctrine/policy unlock:** permits a bounded, sometimes exclusive strategic specialization under its own activation rules.

Each technology must declare its intended scope and observable effect. A technology with no meaningful runtime consequence should not enter the production research catalogue.

## 4. Building-integrated research taxonomy

The existing proposed roster supplies most of the needed institutions. This design **does not authorize creation of numerous generic new research buildings**.

| Existing proposed facility role | Example research domains | Representative effect |
| --- | --- | --- |
| Command headquarters / operations offices | Command coordination, dispatch, mobilization | Authorized command support or logistical efficiency |
| Barracks / training centers | Infantry protection, medical support, training methods | Eligible infantry protection / support capability |
| Vehicle factories / engineering works | Mechanical reliability, armor, targeting, assembly process | Shared vehicle upgrade or recipe; local factory throughput |
| Airbases and airfields | Aircraft service, avionics, reconnaissance | Authorized aviation improvements |
| Power stations | Turbine/control modernization, fuel utilization | Local electricity generation or fuel demand |
| Refineries / gas facilities | Processing efficiency, industrial control | Better authorized input-to-output conversion |
| Storage / maintenance depots | Repair, replacement parts, distribution | Local repair throughput, applicable storage efficiency |
| Radar and communication facilities | Detection and transmission processing | Better authorized sensor coverage or signal function |
| Intelligence / special operations facilities | Infiltration, counterintelligence, information verification | Faction-restricted intelligence proficiency |
| Nuclear and strategic facilities | Restricted research and special-production permissions | Only under the existing nuclear specification |
| Trade / commercial facilities | Contracts, surplus parts, licensed refurbishment | Pact-specific commercial and maintenance efficiency |

**No arbitrary disciplinary substitution:** a power station cannot research infantry marksmanship unless a specifically authored, reviewable cross-system rule justifies that relationship.

### 4.1 Shared research versus local upgrades

**Shared technology:** researched once by a participant at any eligible host; applies to all compatible entities per its effect definition. Subsequent units receive the benefit at creation. If its original host is destroyed, the acquired knowledge normally persists for the match.

**Local modernization:** purchased/installed at a particular instance; only that building benefits; it may require a previously completed shared technology. Destruction retires the installed modification along with its host. Capture and repair follow explicit transfer/retention rules.

**Operational service:** unlike permanent knowledge, a radar feed or powered service may disappear when the supporting infrastructure fails. A technology may permit a service without guaranteeing that service is always available.

Example: **Power Control Engineering** (shared research at a valid station) permits **Generator Controls Package** (individual station upgrade). Each installation costs separate USD and installation time. Outage rules still apply; no generator is exempt from positive minimum building power.

## 5. Research disciplines and faction identity

Six broad disciplines organize presentation and content; they are **labels**, not separate resource currencies, mandatory tier ladders or mandatory dedicated building classes.

1. **Military:** infantry, vehicles, aviation, defense, equipment and unit abilities.
2. **Industrial:** factory production, repair, construction methods and process reliability.
3. **Energy/economic:** electrical generation, fuel processing, storage, conversion and trade operations.
4. **Communications/reconnaissance:** radar, signals, observation and electronic countermeasures.
5. **Intelligence/security:** counterintelligence, spy skills, classified technology provenance and facility security.
6. **Organization/doctrine:** mobilization, logistics, command coordination and optional exclusive specializations.

### 5.1 Atlantic Concord

Use the roster's **Personnel Training Center (AC-BLD-04)**, **Armored Vehicle Works (AC-BLD-05)**, **Grid Power Station (AC-BLD-12)**, **Combined Arms Doctrine Center (AC-BLD-16)** and other eligible operations/security facilities for precision, professional training, communications, detection and coordinated forces.

Candidate content: infantry protective equipment; vehicle fire-control improvement; tactical radio modernization; aviation servicing; defended logistics. Its **established lower fossil-fuel refining cost** remains a canonical advantage, not a technology that must first be researched. Research cannot turn basic scouts into armed aircraft merely by sharing a technology label.

### 5.2 Red Directorate

Use **Five-Year Planning Office (RD-BLD-02)**, **Conscript Muster Barracks (RD-BLD-03)**, **State Armor Combine (RD-BLD-05)**, **Conventional Thermal Station (RD-BLD-12)** and **Army Maintenance Workshops (RD-BLD-17)** for standardized components, training throughput, repairs, mobilization and organized industry.

Candidate content: standardized vehicle components; repair capacity; improved production scheduling; coordinated defensive fire. Its **substantially cheaper infantry recruitment** is a starting faction rule, not something the player must unlock or replace through research. Food upkeep remains enforceable.

### 5.3 Iron Ascendancy

Use **Special Operations School (IA-BLD-04)**, **Mechanized Assault Factory (IA-BLD-05)**, **Experimental Armor Laboratory (IA-BLD-06)**, **State Armaments Research Bureau (IA-BLD-14)** and eligible covert/communications facilities.

Candidate content: aggressive vehicle modifications with explicit trade-offs; clandestine training; specialist security. Initial espionage is limited; nuclear intelligence cannot be purchased as an ordinary research item. Spy progression and successful enemy-facility infiltration retain the precise gates documented in [Nuclear Weapons](NUCLEAR_WEAPONS_SYSTEM.md). Its existing slightly greater raw destructive strength/high-risk combat identity is not converted into universal research superiority.

### 5.4 Meridian Pact

Use **Light Vehicle Assembly Yard (MP-BLD-04)**, **Licensed Repair Factory (MP-BLD-05)**, **Commercial Ledger Office (MP-BLD-13)**, **Foreign Exchange House (MP-BLD-14)**, and legally eligible logistics facilities.

Candidate content: modular vehicle equipment; commercial refurbishment; contract handling; repair efficiency; distribution and storage improvements. The Pact's early inexpensive truck scouting advantage and starting proximity to a fossil-fuel field remain canonical. Research does **not** authorize nuclear refining, nuclear weapon production, forbidden market goods or generic spy capabilities.

### 5.5 The Unaligned

The Unaligned is **NPC-only**, not a fifth player research participant. Authored scenario institutions may have technology states or service improvements, and its autonomous AI may operate/defend/investigate installations through valid world-authority rules. Do not expose a normal playable tech tree, automatically transfer civilian assets to a playable faction, or allow the Unaligned's fabricated intelligence reports to grant real technologies.

## 6. Illustrative starting research catalogue

These are **design candidates**, not locked balance, final asset IDs, guaranteed roster unlocks or currently playable research.

| Candidate technology | Eligible host concept | Scope | Proposed consequence |
| --- | --- | --- | --- |
| Infantry Protective Equipment | Training center | Shared | Eligible infantry protection |
| Field Medical Procedures | Training / medical facility | Shared | Eligible recovery/support modifier |
| Vehicle Component Standardization | Vehicle factory / repair workshop | Shared | Lower eligible repair expenditure |
| Factory Process Optimization | Vehicle production facility | Local or unlock+local | Better production throughput at modernized factory |
| Tactical Radio Modernization | Command / communications center | Shared | Improved eligible coordinated actions |
| Radar Signal Processing | Radar facility | Shared + requires service | Better lawful detection performance |
| Fuel Processing Efficiency | Refinery | Shared | Modifies authorized refining conversion |
| Industrial Power Controls | Power station | Shared + local install | Unlocks an individual station upgrade |
| Supply Preservation | Depot | Local | Better eligible storage/distribution |
| Precision Vehicle Targeting | Concord military facility | Shared/faction | Eligible targeting improvement |
| Industrial Mobilization Methods | Directorate planning office | Shared/faction | Authorized industrial scheduling benefit |
| Advanced Clandestine Training | Ascendancy specialist facility | Shared/faction | One step in spy proficiency progression, not nuclear grant |
| Modular Commercial Equipment | Pact repair/vehicle facility | Shared/faction | Eligible refurbishment/modification |
| Electronic Countermeasures | Eligible signals facility | Shared/service | Counter-detection subject to visible/authorized mechanics |

Designers should avoid redundant minor increases with indistinguishable outcomes. A progression chain should generally have **one or two meaningful steps**, occasionally three where faction identity justifies a longer branch. Target **approximately six to ten financially realistic research decisions per ordinary skirmish** as an initial balance hypothesis, not a hard catalogue size or enforced cap.

## 7. Dependency graph without ages or eras

Represent research and unlock relationships as a **typed directed acyclic graph of mandatory prerequisites**. Each node references stable content IDs; visual ordering must not constitute an implicit dependency.

Example chain:

~~~
Operational Command Center
    -> Tactical Radio Modernization
        -> Coordinated Reconnaissance capability
        -> Improved Fire Coordination capability
~~~

A second independent branch may follow refinery/processing improvements; a third may improve armored production. The player may follow any reachable branch that its faction, map, structures and available budget permit. There is **no** Dark/Feudal/Castle/Imperial Age, ancient/modern era, blanket Tech Level unlock, or obligatory research of unrelated equipment.

**Predicates to distinguish:**

- **Static prerequisites:** completed technology IDs, known prior capability requirements, valid faction eligibility, configured map/scenario permission.
- **Research admission requirements:** owned operational host, reserved USD, available research channel, relevant unlocks, no duplicate/forbidden project.
- **Continuous runtime requirements:** electricity, building existence/operability, service access.
- **Action-specific requirements:** unit readiness, valid target, supply, local installation materials, deployment permissions.

Optional mutually exclusive doctrines may use authored *exclusive group* IDs. They must have explicit scope, visible trade-offs, a deterministic conflict policy and a legal starting branch; routine unrelated research remains non-exclusive. For ordinary content validation, reject mandatory cycles, missing references, unreachable enabled recipes and faction-capability contradictions.

## 8. Economy, research cost and opportunity cost

The single ordinary purchasing currency is **in-game US dollars ($)**, stored as nonnegative **integer cents** in the existing participant-owned treasury. Research uses the same accounting authority as production, food purchases and construction. No additional science-point or administrative-point currency is proposed.

A research project has: (i) an authored nonnegative USD cost, (ii) a bounded duration in authoritative simulation steps, (iii) a valid host and channel, and (iv) any explicitly authorized special-material prerequisites. The application must reserve/debit **exactly once** at admission. Rejection must have no side effects; cancellation/refund policy must be defined once for research and reconciled with existing queue/reservation rules.

**Electricity:** every functional building, including research hosts and generators, has a positive minimum electricity requirement. Research at an unpowered host may not advance. Use the current grid/allocation authority; an interruption normally pauses progress without a second charge, then resumes if restored. Destruction/capture requires a terminal queue policy.

**Food:** military food remains a dollar-purchasable reserve for staffed deployed forces. Do not attach arbitrary food research costs or require a farming minigame. Technologies changing legitimate upkeep must interact with the existing due-interval system, not independently rewrite the reserve.

**Gold and fossils:** gold, crude oil, natural gas and authorized refined fuels can be sold for dollars. Technology may alter legitimate processing/handling but must never create commodities or treasury balances from nothing, combine crude and refined stocks, or sell nuclear material through the ordinary market.

**Opportunity cost:** in the first playable research slice, a host should normally share one authoritative work lane between unit production and research. Researching a substantial upgrade may delay reinforcement. Future building modernization can grant an extra *explicitly authored* lane; do not assume parallelism, double-process a project or create hidden throughput.

## 9. Research interaction with other systems

### 9.1 Units and combat

Shared unit upgrades normally affect **eligible existing and future units**, consistent with the C&C3 pattern. Calculate effective statistics from immutable base definitions plus validated research/local/temporary modifiers; do not write new values into shared content assets. Specify whether health-ratio or absolute damage is conserved when maximum health changes; avoid accidental full healing. Maintain veterancy, morale/Discipline and existing orders unless explicitly affected.

Some equipment changes may require an authorized depot refit instead of an immediate army-wide effect; this must be declared per definition. A production unlock does not spawn a unit.

### 9.2 Buildings, production and repairs

Shared engineering research can unlock local modernization at power stations, refineries or factories. Physical installations cost money and time and attach to one building ID. Destruction retires the attachment. Capture does not convey unrelated owner research. Modernization affecting build time or production price must specify whether an existing queued product retains its committed cost and time; **default: do not retroactively discount previously admitted transactions**.

### 9.3 Reconnaissance, fog and intelligence

A sensor improvement can alter authorized detection properties, but must not substitute for the future authoritative observation service. Scouting a site never automatically reveals or steals classified technology. An intercepted report, verified information, obtained blueprint and acquired research flag are distinct states. Unaligned false reports cannot fabricate real content or research permissions.

Espionage may later interrupt a legitimate research project, identify the enemy's investments, or acquire explicitly stealable non-nuclear information. Security/attribution/notification must obey the existing deception and observer-permission models.

### 9.4 Nuclear restriction (hard invariant)

- **Atlantic Concord and Red Directorate:** authorized existing nuclear powers; maintain their documented research, processing, production and distinct bomber/ICBM delivery requirements.
- **Iron Ascendancy:** starts non-nuclear; must finish its specified spy progression and successfully infiltrate a Concord/Directorate nuclear research facility to obtain both required intelligence prerequisites. It then satisfies resource processing and long own-bomb production; **completion of its first own bomb immediately wins**. The ordinary research interface cannot bypass any prerequisite.
- **Meridian Pact:** cannot obtain nuclear refining, nuclear power or own nuclear bomb production through ordinary or stolen research. Existing possible theft/deployment of an *already completed* weapon is a distinct custody/deployment route, never a technology unlock.
- **The Unaligned:** non-nuclear, NPC-only world faction; fraudulent reports have no capability-grant effect.

The authoritative rule remains [Nuclear Weapons System](NUCLEAR_WEAPONS_SYSTEM.md).

### 9.5 Commander talent boundary

The [Commander Talent System](COMMANDER_TALENT_SYSTEM.md) is **Elite commander-only** progression. It is not a substitute for shared research and may not quietly upgrade non-command units through a generic talent grant. A future interaction may require a shared technology before an Elite talent becomes available, but it must be explicitly authored, tested and justified; no automatic cross-grants.

## 10. Runtime authority and Unreal Engine data contract

Continue to use the project's existing UE 5.8.3 C++ match authority, fixed-step coordinator, typed participant/structure IDs, resource ledgers, content validation and production/research admission seams. **Do not introduce a second economy, ability, production, match or faction authority.** No UMG callback, AI node or actor animation may directly spend dollars or mark a technology complete.

### 10.1 Proposed definition types (conceptual names)

| Type | Required authored or runtime fields |
| --- | --- |
| **TechnologyDefinition** | schema version; stable ID; localization key; category; faction eligibility; valid host capabilities; required technology IDs; exclusion group; USD cents; duration steps; effect IDs; equipment/refit flag; visibility rule |
| **TechnologyEffectDefinition** | stable effect ID; effect type; target selector; modifier operation; value and cap; stacking group; conditional runtime dependencies |
| **ResearchHostCapability** | permitted technology families/IDs; lane occupancy; power threshold; ownership and building state gate |
| **ResearchQueueItem** | unique item ID; match generation; participant ID; host runtime ID; technology ID and content revision; original committed cost; progress steps; state and terminal reason |
| **ParticipantTechnologyState** | completed technology IDs; valid active/queued projects; applicable exclusivity decisions |
| **BuildingUpgradeDefinition** | stable ID; compatible structure IDs/capabilities; required technology IDs; USD/material cost; installation duration; maximum installations; effect |
| **BuildingUpgradeState** | owner structure runtime ID; installed definition IDs/levels; active/inactive status and transition provenance |

These are **proposed interfaces**, not existing UE class names or a directive to create all classes immediately. Prefer typed assets and stable IDs compatible with the current content system. Evaluate actual engine APIs before choosing Primary Data Assets, data registry use, or native structs. UE provides [Asset Management](https://dev.epicgames.com/documentation/unreal-engine/asset-management-in-unreal-engine) and [Data Validation](https://dev.epicgames.com/documentation/unreal-engine/data-validation-in-unreal-engine), but enabling or configuring them in POLITIKA must be verified rather than assumed.

### 10.2 Example content shape (illustrative only)

~~~yaml
schema_version: 1
technology_id: TECH-VEHICLE-COMMUNICATIONS
display_name_key: Tech.VehicleCommunications
faction_allowlist: [AtlanticConcord]
research_host_capabilities: [MilitaryCommunicationsResearch]
prerequisite_technology_ids: []
usd_cost_cents: 75000
research_duration_steps: 900
effect_ids: [EFFECT-VEHICLE-COORDINATION]
effect_scope: participant_eligible_units
exclusive_group_id: null
~~~

This is **not** an existing asset, final content ID or locked 900-step balancing value. IDs must be validated and mapped to the repository's actual faction/asset naming contract before authoring.

### 10.3 Admission, progress and completion

Recommended sequence:

1. Typed player/AI **request** carries issuing authority, match generation, owning participant, host runtime ID, technology stable ID and correlation ID.
2. Authoritative **preflight** verifies a current active match, correct participant control/ownership, valid host/availability, unlocked faction/prerequisite path, open research lane, non-duplication, dollar affordability and any extra input restrictions.
3. Commit an atomic **reservation or debit** and create one uniquely identifiable queue item. A rejected/stale/duplicate request must leave all state unchanged.
4. On authorized simulation ticks, advance the item only while ongoing operational conditions hold. A power outage suspends work according to the common queue/energy policy.
5. On exact completion, commit technology acquisition **once**, emit one durable event, recalculate eligible derived effects and publish filtered read-only UI/AI state.
6. On cancellation, host destruction, capture, match transition or invalidation, run the specified terminal/refund policy and retire the item. No item from a former match generation can act in a rematch.

Candidate states: **Requested → Admitted → Queued → Researching → Completed**, with **Paused** nonterminal and **Rejected / Cancelled / HostLost / MatchEnded** terminal. Derive “Available” from prerequisites and state instead of inventing a permanent availability object.

**Concurrency:** exactly one accepted project for a non-repeatable technology per participant; simultaneous requests from multiple valid buildings have deterministic arbitration. A specified repeatable/modular technology must model levels explicitly with bounded maximums. Research effects should be idempotent under duplicate events or replay.

### 10.4 Modifier evaluation

The final attribute is a defined function of base value and its applicable modifier groups:

EffectiveAttribute = Compose(BaseAttribute, SharedResearch, LocalEquipment, TemporaryEffects, ScenarioRules).

Specify additive/multiplicative order, caps, applicability and numerical representation per attribute; reject missing selectors, unknown effect operations, invalid infinity/NaN, contradictory exclusivity and exploitable stacking. Gameplay values are recomputed or cached with correct invalidation, not independently altered in UI or several actor subsystems.

## 11. Building panel, technology browser and feedback

For a selected eligible building, expose **Production | Research | Upgrades** as contextual commands where available. If a building offers only some functions, show only those or explain unavailable tabs. The research view must show: localized name, class of effect, target scope (army-wide vs local), actual USD cost, duration, correct host, prerequisite chain, exclusive conflicts, current funding/power state and why the project is locked.

A global *Technology* browser is useful for discovery and planning, but it must route all actual purchases through the owning building and normal command admission. The UI never manufactures research state, selects an ineligible building invisibly, or leaks enemy technologies hidden by the intelligence model.

Show progress, interruption cause, cancellation/refund result, completion and consequential changed capabilities. An upgrade that affects combat should be discoverable from the relevant unit card; a factory modernization should be visible on that factory. Allow keyboard, mouse, controller and touch presentations to resolve to the same semantic commands, consistent with [UI Overview](POLITIKA_UI_OVERVIEW.md) and [Practical UI and Keymap](RTS_UI_SETTINGS_AND_KEYMAP.md).

Research state shown for the **enemy** must be sourced from verified observable events or authorized intelligence. A planted report is labeled with its appropriate provenance/uncertainty, not converted into a real opposing tech flag.

## 12. AI and balancing guidance

A skirmish AI must evaluate a small candidate list through the *same* technology availability and research admission rules as the human player. Never grant free upgrades, spend from synthetic funds or read hidden enemy research when an observation service is available. Its first rule can be deterministic: fund a relevant available upgrade only when the marginal improvement is useful and sufficient reserves remain for power, food and production.

Research is strategically meaningful when money spent now competes with reinforcement now. Balance by *time to realized value*, opportunity cost of the occupied building lane, power vulnerability, expected match duration and actual impact on likely army composition. Avoid research that is a no-brainer every match or never pays back within play.

**Initial playtest hypotheses** (not locks): common chains usually 1–2 steps; 3 for highly specialized branches; six to ten consequential, realistically affordable research choices per typical skirmish; avoid full-tech completion as the default outcome. Some choices may be explicitly exclusive doctrines, but never force unrelated military/industrial upgrades into one false binary.

## 13. Content validation and acceptance matrix

The existing production contract includes **TECH-01..TECH-03** as preliminary validation expectations. Extend, rather than duplicate, those identifiers as the research slice becomes implemented.

| Gate | Required check |
| --- | --- |
| TECH-01 | Missing references, mandatory cycles, unreachable unlocked recipes and invalid host/tech IDs are rejected before match start |
| TECH-02 | Participant/faction eligibility and explicitly restricted unlocks are enforced; prohibited nuclear/market access impossible |
| TECH-03 | Technology completion does not retroactively corrupt unrelated queued prices, outputs or transaction IDs |
| TECH-04 | Authorized research starts only at a living, owned, operable, powered and capable host |
| TECH-05 | Wrong owner/controller, stale generation, missing host, ineligible faction and unknown ID reject atomically |
| TECH-06 | Research debit/reservation occurs exactly once; insufficient USD and duplicated request have zero side effects |
| TECH-07 | Equal authorized step counts yield the same completion step regardless of render-frame scheduling |
| TECH-08 | Concurrent attempts to acquire the same nonrepeatable technology yield exactly one winning project |
| TECH-09 | Outage pauses and restoration resumes with consistent progress and no second debit |
| TECH-10 | Host destruction/capture/cancellation produce the declared terminal state and exact refund treatment |
| TECH-11 | Completed participant research survives loss of its former host where authored as persistent knowledge |
| TECH-12 | Shared upgrade affects eligible existing and subsequently produced units, not unrelated units or factions |
| TECH-13 | Local modernization affects only its building instance and is retired or transferred under authored capture/destroy rules |
| TECH-14 | Derived combat/economy modifiers stack by declared rules and do not full-heal damaged units unexpectedly |
| TECH-15 | Refineries/power/food/income retain one authoritative service and transaction ledger; no free goods or energy |
| TECH-16 | Inactive sensors and covert intel do not leak through the research browser, AI or HUD |
| TECH-17 | False intelligence cannot create completed technology or satisfy nuclear permissions |
| TECH-18 | Ascendancy nuclear route requires full spy progression, valid infiltration, correct stolen intelligence and distinct weapon completion |
| TECH-19 | Pact cannot research nuclear production or refining; Unaligned is not a playable research slot |
| TECH-20 | Non-command units do not receive Elite commander talents as generic research upgrades |
| TECH-21 | Invalid fixture/content is rejected in source tests, UE Editor/commandlet tests and a packaged startup gate as appropriate |
| TECH-22 | Research queue and acquired technology state are cleared for terminal/rematch generation isolation |
| TECH-23 | Normal packaged interaction shows research cost, locked reason, progress and actual attributable unit/building effect |
| TECH-24 | AI research request is an ordinary authorized purchase, no privileged currency, no hidden-world leak |

No gate can be marked **PASS** from design text alone. Preserve exact source revision, engine pin, test commands, package identity, representative logs and limitations in the appropriate acceptance report.

## 14. Delivery plan and sequencing

**Do not expand Stage 4B or the Stage 4A–4F First Playable Skirmish gate solely to implement the full research system.** [Engine Outline §7.5](ENGINE_OUTLINE.md#75-stage-4--first-playable-skirmish-repeatable-offline-match) retains authority over the current dependent work.

| Sequence | Bounded delivery | Dependencies | Acceptance |
| --- | --- | --- | --- |
| R0 — Specification | This document, cross-system rules and content vocabulary | Existing faction/economy/roster canon | Reviewed design; no runtime claim |
| R1 — Definitions/validator | One versioned technology definition, eligibility/host graph checks, invalid fixtures | Stage 3 typed-content framework | Missing IDs, cycles and forbidden faction gates rejected |
| R2 — Smallest runtime | One building and one USD-priced, timed shared upgrade | Mature participant economy, powered producer, queue admission | Real unit attribute effect on existing/new units; deterministic interruption |
| R3 — Local modernizations | One power station or factory instance upgrade | Building lifecycle and authoritative power model | Building-only effect, outage/capture/destroy handling |
| R4 — Player interaction | Research/upgrade tabs, progress, denial explanations | Selection/UI and read-only state | Rendered normal user flow; controller/touch-ready intent |
| R5 — Branches/factions | Several legal faction-specific technologies with authored dependencies | Validated shared research | Distinct available choices, equal authority checks |
| R6 — System interactions | Refining, repair, logistics, intelligence, restricted nuclear hooks | Respective owning system implementation | No duplicate ledgers or bypasses |
| R7 — Opponent and balance | Deterministic AI research decisions and scenario tuning | Autonomous AI, observation service | Legitimate competition and measurable pacing |

**Minimum execution slice:** one existing operational training/production building; one research project with an authored price and duration; authoritative spend/queue; one army-wide effect observable on an existing and subsequently produced compatible unit; outage/cancel/rematch negative fixtures. This is sufficient to prove the ontology before adding a sprawling tech catalogue.

## 15. Deliberate exclusions

- No Age of Empires *age* advancement, Civilization *era* progression or universal tech-level promotion.
- No separate research-point treasury, automatic science accumulation, university/campus management or research-worker minigame.
- No compulsory construction of a dedicated new research laboratory for every technology branch.
- No invention of research effects without a concrete targeted gameplay mechanic.
- No forced research by every faction along an identical path.
- No bypass of existing nuclear, black-market, espionage, faction or Unaligned restrictions.
- No replacement of production/energy/food authorities, Elite commander talents, map validation or combat modification rules.
- No claim of current implementation, validated balance, multiplayer replication support or save compatibility before their own gates.

## 16. Design decisions that require playtest resolution

1. Whether the first shared research queue should be a **single lane shared with production** (recommended) or a separate lane for certain specialist hosts.
2. Whether field equipment is applied immediately, requires refit at a depot, or is authored per technology (recommended: per technology).
3. The refund policy for research cancellation and host loss (must be exact, deterministic and aligned with production accounting).
4. The correct stacking order, caps and current-health handling for research-derived attribute modifiers.
5. How captured local modernization persists or is destroyed, separately from participant-wide knowledge.
6. Which doctrines are exclusive, what may be changed during a match, and how locked choices appear in UI.
7. Which information about opponent research is observable, interceptable, forgeable or permanently hidden under the future intelligence service.
8. Cost, duration and return-on-investment balance under actual skirmish pacing and faction asymmetries.

These questions must not be silently answered by incidental implementation details.

## 17. References and provenance

### External RTS design reference material

- Electronic Arts, *Command & Conquer 3: Tiberium Wars* [PC manual](https://static.wikia.nocookie.net/cnc_gamepedia_en/images/8/8b/CCUC-CNC3-pcMANOLeUS.pdf/revision/latest) — research at a specific building and application to all eligible existing and future units, persisting after research host destruction.
- Westwood Studios, *Command & Conquer: Tiberian Sun* [PC manual](https://cnc-comm.com/files/TS-Manual.pdf) — per-building power turbine installations.
- Blizzard, [*StarCraft II — Terran Basics*](https://news.blizzard.com/en-us/article/5740263/game-guide-terran-basics) — Engineering Bay, Armory and Tech Lab division; upgrade investment by military branch.
- *Age of Empires IV*, [Blacksmith technology catalogue (AoE4 World)](https://www.aoe4world.com/explorer/civs/english/buildings/blacksmith) — facility-specific unit/defense research; **ages are explicitly excluded from the proposed POLITIKA implementation**.
- Relic Entertainment, [*Company of Heroes 3* support-center balance and specialization (patch archive)](https://help.relic.com/hc/en-us/articles/39307744455571-Company-of-Heroes-3-Patch-Notes-Archive) — infantry, mechanized and air support distinctions.
- Epic Games, [Unreal Engine 5.8 — Asset Management](https://dev.epicgames.com/documentation/unreal-engine/asset-management-in-unreal-engine) and [Data Validation](https://dev.epicgames.com/documentation/unreal-engine/data-validation-in-unreal-engine) — tools for typed, discoverable, authorable, validated assets; applicability to POLITIKA must be proved by integration.

### Internal POLITIKA references

- [RTS Game Ontology](RTS_GAME_ONTOLOGY.md): distinctions among entities, actions, information and capabilities.
- [Structures, Production Queues and Technology](RTS_PRODUCTION_AND_TECHNOLOGY.md): single production/transaction authority and preliminary TECH acceptance gates.
- [Resource Economy](RESOURCE_ECONOMY.md) and [Dollar Economy, Power and Food](RTS_OPERATIONAL_ECONOMY_AND_POWER.md): USD, commodities, electricity and military upkeep.
- [Five-Faction Unit and Building Rosters](POLITIKA_FIVE_FACTION_ROSTERS.md): proposed facility identity and faction-specific capabilities.
- [Espionage](ESPIONAGE_AND_INTIMIDATION.md), [Reconnaissance](RECONNAISSANCE_AND_SCOUTING.md), [Visibility and Intelligence](RTS_VISIBILITY_AND_INTELLIGENCE.md) and [Nuclear Weapons](NUCLEAR_WEAPONS_SYSTEM.md): sealed information and strategic restriction contracts.
- [Commander Talents](COMMANDER_TALENT_SYSTEM.md): Elite commander-only progression kept separate from shared tech.
- [UI Overview](POLITIKA_UI_OVERVIEW.md) and [Practical RTS UI/Keymap](RTS_UI_SETTINGS_AND_KEYMAP.md): presentation and input adaptation.
- [Engine Outline](ENGINE_OUTLINE.md): sequencing and implementation acceptance authority.

**Bottom line:** technological research is a **building-hosted, participant-owned, data-driven source of new capabilities**; installed modernization is a **physical, per-building upgrade**; each must affect existing authoritative game systems while respecting POLITIKA's USD economy, historical setting, short-match pacing and hard faction restrictions. No ages or eras are introduced.
