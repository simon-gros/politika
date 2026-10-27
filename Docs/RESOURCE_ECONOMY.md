# POLITIKA — Resources and Strategic Economy

Status: Core resource hierarchy and Meridian Pact starting-position advantage established; quantities, conversion ratios, map distribution, costs and numerical balance remain open.
Related: [Concepts Outline](CONCEPTS_OUTLINE.md) · [Nuclear Ultimate Weapons System](NUCLEAR_WEAPONS_SYSTEM.md) · [Espionage and Intimidation](ESPIONAGE_AND_INTIMIDATION.md) · [Story Outline](STORY_OUTLINE.md) · [Faction descriptions](../Factions/)

## 1. Resource hierarchy

POLITIKA's economy assigns the greatest strategic value to nuclear-weapons materials, followed by fossil fuels. Electricity and other industrial or logistical resources also matter, but their detailed rankings and economies remain to be defined. The ranking is a gameplay design principle, not a real-world commodity price comparison.

| Priority | Resource group | Included examples | Strategic role |
|---|---|---|---|
| 1 — Highest value | Nuclear-weapons materials | One or more scarce, abstract nuclear-resource tokens or deposits | Exceptional late-game production input for nuclear ultimate weapons; finite, contested and difficult to accumulate |
| 2 — Second-highest value | Fossil fuels | Crude oil, natural gas, petroleum-derived fuel supplies, kerosene | Energy, vehicle and aircraft operations, industry, logistics, trade and strategic territorial control |
| Additional systems | Electricity | Grid generation, transmission and available power | Enables or constrains buildings, research, fuel processing and factory operation; shortages affect production |
| Other resources — to define | Conventional industrial, construction and logistical inputs | Candidate examples: metals, manufactured components, cash/credits, supply capacity | Conventional production, buildings, repair, trade, upgrades and transport |

Critical distinction: crude oil and natural gas can originate from on-map fossil-fuel fields. Petroleum products and kerosene are processed or stored fuel categories, not separate naturally occurring underground fields. Refining is a required gameplay transformation: crude oil must be processed in a refinery into petroleum-derived fuel or kerosene. Map generation should not depict petroleum-product or kerosene wells.

Scarcity and value: nuclear materials are the most prized and scarce strategically; fossil fuels are the second-most valued group. Their prices, extraction rates, storage limits, resource costs, regeneration rules and exchange ratios are not fixed in this document. A resource's rank does not guarantee an equal per-unit credit price across all maps or markets.

## 1.1 Faction-specific economic and combat asymmetry

The resource values above are shared across the game, but faction effective costs and tactical doctrine differ. These advantages are canonical relative balance rules; no percentages, absolute costs or specific damage multipliers are locked.

| Faction | Established advantage | Implementation / balancing boundary |
|---|---|---|
| Atlantic Concord | Fossil fuels cost slightly less than for other factions | Apply lower fuel-refining costs than other factions, yielding a slight overall fossil-fuel cost advantage; this does not increase deposit yield, grant free fuel, or discount nuclear processing |
| Red Directorate | Infantry is far cheaper to produce than for every other faction | Major reduction to infantry recruitment/production cost for comparable roles; preserves the strategic value of mass infantry, but does not make infantry free or remove supply constraints |
| Iron Ascendancy | Slightly stronger raw destructive power and occasional high-risk, potentially suicidal attacks designed to inflict maximum damage | A modest offensive advantage and distinct attack behavior at greater risk of losing units; does not imply superior economy, invulnerability or limitless damage |
| Meridian Pact | Guaranteed starting proximity to existing fossil-fuel fields | Spawn-location benefit near oil/gas source nodes, not guaranteed ownership or an extraction-rate bonus |
| The Unaligned | Neutral NPC world administration, not a selectable economic competitor | Existing civilian/industrial networks follow simulation rules and cannot be commandeered by players without the relevant interactions |

For the Atlantic Concord, lower refinery processing costs are the explicit source of its fossil-fuel economic edge. A slight effective fuel-cost advantage is established, but any independent discount on raw-fuel purchases is a separate balancing decision. Resource markets and local scarcity can still fluctuate. For the Red Directorate, infantry-production prices must remain substantially below every other faction's, not merely slightly below one rival. The Ascendancy's offensive trade-off is specified in combat doctrine rather than encoded as a generalized resource or currency discount.

### Espionage and coercion effects on resource control

The Atlantic Concord's distinct espionage specialization is Psy-Ops, with fully broken Discipline/Morale leading to a targeted unit's in-game self-destruction or, alternatively, a short friendly-fire rampage. The Red Directorate gains slightly cheaper and faster basic infiltration for stealing rival intelligence, rather than a general discount on all spies or industrial research. The Iron Ascendancy begins with more limited spying, and its full espionage progression remains a prerequisite before any eligible spy can infiltrate a Concord/Directorate nuclear research facility for nuclear-refining and bomb-production intelligence. It alone has Intimidation, which can force eligible isolated enemy units to join its army. Conversion changes unit allegiance, not legal ownership of mineral deposits, refining technology, or nuclear production rights. Intimidating an eligible hostile combat unit cannot substitute for the designated research-facility infiltration objective or transfer resource ownership automatically. No other faction has Intimidation. See [Espionage and Intimidation](ESPIONAGE_AND_INTIMIDATION.md).

### Facility espionage: resource waste, theft and workforce attrition

Espionage affects budgets and production as well as intelligence. These three faction-specific facility attacks have distinct, auditable accounting:

| Faction | Facility effect | Budget / production consequence |
|---|---|---|
| Atlantic Concord | Psy-Ops causes self-destruction of an eligible building, or finite, potentially unnoticed resource waste | The target loses a structure or consumes/wastes its own resources; the Concord receives no stolen resource credit |
| Red Directorate | Covert resource diversion from an eligible enemy facility or budget | The enemy loses a bounded transferable amount and the Directorate gains the corresponding amount in its own account; transfer can remain unattributed |
| Iron Ascendancy | Abstract poisoning/disease status affecting staffed enemy facilities and a limited chain of nearby eligible facilities | Simulated worker/researcher deaths lower available workforce, research and production throughput, possibly stopping production; costs include replacement, downtime and disrupted inputs |

The engine must separately track the actual resource transaction, loss and responsible covert operation. Visibility/attribution may be withheld from the defending player's intelligence view, but never from authoritative accounting. **Resource waste is a sink; Directorate diversion is a debit-plus-credit transfer; facility contamination depletes worker capacity and indirectly wastes inputs and output.** These are not interchangeable balance effects.

Staff losses apply to staffed facilities according to gameplay workforce requirements; a building at or below a configured minimum staff count has reduced or zero throughput. Replacement staff and restoration are legitimate time/resource expenditures, not instant free replenishment. Nearby facility contamination is a bounded fictional adjacency/status propagation mechanic with maximum hops/affected facilities, time limits, containment and recovery—not a real epidemiological or poisoning simulation.

A targeted nuclear refinery, research building or other late-game installation follows these same rules only if explicitly eligible, but such sabotage **does not transfer research knowledge or count as the Ascendancy's required nuclear-facility spy-intelligence acquisition**. See [Espionage and Intimidation](ESPIONAGE_AND_INTIMIDATION.md).

### Sabotage economy and scout production

The Meridian Pact's cheap early-game production of very fast, lightly armored and lightly armed reconnaissance/combat trucks gives it the strongest overall scouting advantage. Its other asymmetry is the game's best conventional explosive sabotage: eligible non-spy Pact units may place fictional charges on enemy vehicles/buildings early and cheaply. Atlantic Concord, Red Directorate and Iron Ascendancy may conduct similar attached-charge sabotage only later and at greater cost, using spies alone.

Model charges and other expenses as abstract unit ability costs, separate from nuclear-resource production or refinery recipes. Explosive damage can destroy structures or vehicles, create conventional repairs/replacement costs and interrupt production; it must not debit/credit a player's resources as if it were Directorate financial diversion, Concord hidden waste or Ascendancy workforce contamination unless an explicit separate effect applies. See [Explosive Sabotage](SABOTAGE_SYSTEM.md) and [Reconnaissance and Scouting](RECONNAISSANCE_AND_SCOUTING.md).

## 2. Nuclear materials: rare strategic resource

Nuclear-weapons materials are the most highly valued resource in the entire game. They should be scarce, geographically contested, strategically defensible and important to late-game victory. Access may require exploration, control, contracts or specialized map objectives.

Crude nuclear materials must first pass through an eligible nuclear-refining facility as an abstract RTS processing step. The resulting processed nuclear-material game resource can support nuclear power plants or nuclear-weapons production, subject to faction eligibility. For ultimate-weapon production, all of the following must be satisfied: verified nuclear-weapons intelligence, processed nuclear materials, other required inputs, an eligible production site and an extremely long game-time production process. The details are abstract fictional game resources; this document does not specify isotopes, enrichment, chemistry, design, or production procedures.

The Atlantic Concord and Red Directorate are already nuclear powers. Both can refine crude nuclear materials, and construct nuclear power plants or tactical nuclear ultimate weapons under their respective construction and research gates. The Iron Ascendancy begins non-nuclear: its spies must fully develop its espionage progression, then infiltrate an Atlantic Concord or Red Directorate nuclear research facility and steal nuclear-refinement technology intelligence and nuclear-weapons intelligence. It may then refine crude nuclear materials for bomb production only and complete its first atomic bomb, but cannot construct nuclear power plants and has no interest in doing so. As soon as its own bomb is produced, the active game ends immediately in an Iron Ascendancy victory, without delivery or detonation. A stolen completed weapon does not count as an own-produced bomb.

The Meridian Pact cannot refine crude nuclear materials at all, cannot manufacture nuclear weapons, and has no nuclear power-plant production path; it may nevertheless sabotage, steal and deploy an already completed weapon with compatible game assets. Nuclear weapons and nuclear-intelligence unlocks are not regular Black Market stock.

## 3. Fossil fuels: the second-highest-value resources

The fossil-fuel category includes four named resources or fuel forms:

- Crude oil: extracted from designated oil fields; a strategic feedstock for a simplified fuel-processing chain.
- Natural gas: extracted from designated gas fields; used as a fuel, trade commodity or energy input.
- Petroleum / petroleum-derived fuels: processed fuel supplies, used in vehicle fleets and industrial/logistics systems. Naming should distinguish crude oil from refined petroleum products to avoid double counting.
- Kerosene: a processed petroleum fuel, potentially particularly relevant to aircraft readiness, aviation support and trade.

Refining is mandatory for the finished economic design: crude oil is an input to a refinery facility, which outputs petroleum-derived fuel or kerosene. These are distinct game resources, not a freely interchangeable combined stockpile. Early prototypes may stub the conversion with abstract formulas, but may not silently treat crude oil as usable refined fuel. The Atlantic Concord refines at lower cost than the other factions. Fuel supply should influence movement and logistical reach, production support, trade, and potentially the availability of nuclear delivery platforms without making resource scarcity a meaningless passive modifier.

Fuel fields create contested regional value for all factions, but especially for the Meridian Pact's commercially oriented economy.

## 3.1 Two abstract refining chains

Fossil-fuel refinery: `CrudeOil + refinery operating cost → PetroleumFuel OR Kerosene`. The processing facility, available crude inventory, processing time, output storage and electricity/operating costs are part of the economy. Atlantic Concord pays a lower refinery cost than other factions. The user does not need a simulation of real chemical processing: recipes and resource ratios are fictional game values.

Nuclear-material refinery: `CrudeNuclearMaterial + authorized refinement facility → ProcessedNuclearMaterial`. This represents an abstract fictional game transformation, not a real-world materials procedure. Atlantic Concord and Red Directorate have authorized refinement technology and can use the processed output for nuclear power-plant construction or nuclear ultimate-weapon production. Iron Ascendancy must first fully develop its espionage capabilities and then steal nuclear-refinement intelligence from a Concord or Directorate nuclear research facility before it can operate such a process, and may use it only for weapon production, never nuclear power plants. Meridian Pact has no nuclear-material refinement capability whatsoever. The Unaligned remains non-nuclear and NPC-only.

Electricity output: an eligible completed nuclear power plant contributes to the power network. Only Concord and Directorate may construct such plants. Grid-generation amounts, facility power needs, processing costs and conservation/consumption rates are balancing parameters.

Resource-state invariant: crude and processed materials must be stored and tracked separately. Neither crude oil nor crude nuclear material can bypass its required refinement step when used for a finished product.

## 4. Meridian Pact starting-position advantage

Canonical gameplay rule: on game start, every Meridian Pact player or AI-controlled Meridian starting position must be placed near an existing fossil-fuel source field. Oil and natural-gas fields qualify. This is a proximity advantage, not free ownership, guaranteed extraction, a prebuilt refinery or infinite fuel.

Suggested map-generation and authored-map rules:

1. Identify eligible oil or natural-gas field locations before selecting Meridian start positions.
2. Choose Meridian starting positions near at least one field according to a later-defined, map-scale-aware distance threshold.
3. Ensure the position is reachable, strategically usable and contestable, with access governed by terrain, jurisdictions, neutral Unaligned patrols and map rules.
4. In procedural generation, reserve an eligible fossil-fuel site for each Meridian start or revise/reject the layout if the constraint cannot be met. In authored maps, validate the same invariant and surface an error if it fails.
5. Do not silently transfer Unaligned sovereign ownership of the deposit to Meridian. Normal access, capture, leasing, trade, extraction, and security rules continue to apply.
6. Other factions may also control nearby fuel sources; the Pact's advantage is guaranteed starting proximity, not exclusive access to fossil fuel.

This rule makes geography a signature component of Meridian gameplay. Early fuel access supports its roles as an energy merchant, transport/logistics specialist, mercenary contractor and Black Market operator, creating a conventional-resource alternative to the major factions' nuclear race.

### Population morale and production

Every faction, including Meridian Pact and NPC Unaligned, has a population morale meter. Higher morale improves eligible production output, in addition to improving army discipline. Concord, Directorate, Ascendancy and Unaligned have distinct advanced propaganda methods using radio, newspapers and television, while Meridian lacks advanced propaganda. Morale does not bypass resource refinement, scarcity or construction prerequisites. See [Propaganda and Morale](PROPAGANDA_AND_MORALE.md).

## 5. Electricity and infrastructure

Electricity is an additional game resource / utility flow. Nuclear power plants are a potential generation source built by the Atlantic Concord and Red Directorate only, using refined nuclear-material game resources; the Iron Ascendancy cannot build and does not seek them, and the Meridian Pact cannot refine nuclear materials. Generation facilities, grid infrastructure, storage (if later chosen), transmission routes, and consumption by industrial buildings may affect supply and production. Electricity can be modeled as a local network capacity, an abstract resource stock, or a hybrid; the specific simulation remains open.

Potential relationships worth prototyping include:

- Industrial facilities and nuclear-weapon game production timers pausing or slowing when electricity is unavailable.
- Refineries, depots, factories, and research buildings depending on sufficient supply.
- Damage to substations, power plants or transmission links interrupting production or civilian services.
- Regional power availability affecting diplomacy with the Unaligned and faction reputation.

Electricity is not automatically ranked third; further conventional resources may be introduced and ranked later.

## 6. Economic ownership and trade

Maintain separate state for deposit location, territorial jurisdiction, legal owner, current extractor/controller, production/storage, and transported inventory. A natural resource deposit on Yugoslav territory may initially belong to or be administered by the Unaligned, even when a Meridian starting base is nearby.

The Black Market may offer conventional fuel supplies, equipment, specialist services, mercenary contracts, and permitted intelligence, under finite stock and physical delivery rules. Nuclear materials are not assumed to be freely tradable merely because they rank highest in value; nuclear weapons and nuclear production intelligence remain excluded from ordinary listings. The exact tradeability of abstract nuclear-material tokens must be decided separately, with balance and faction-gate constraints preserved.

Fossil fuels should be sources of trade income and operational advantage, not merely decorative map nodes. Convoys, pipelines (if represented), fuel depots, refineries, ports and rail corridors provide conventional RTS interactions: escort, defend, interdict, repair, reroute and seize.

## 7. Simulation data and validation

Suggested data-driven abstractions:

- `ResourceDefinition`: stable resource ID, group/rank, stock/storage rules and extraction/source type.
- `ResourceNode`: resource ID, on-map position, finite supply/regen policy, jurisdiction, owner/controller, harvest access.
- `ResourceInventory`: available/reserved/in-transit amounts by owner/faction.
- `CovertResourceTransaction`: action ID, source/destination or sink, currency/resource type, budget cap, actual debits/credits, detection/attribution status and audit metadata.
- `FacilityWorkforce`: required/available staff, losses, replacement queue, staffing throughput modifier and stoppage state.
- `FacilityIncidentChain`: fictional contamination status, nearby eligible building associations, bounded propagation count/depth, expiry and containment/recovery.
- `PowerNetwork` or equivalent: optional electricity availability and connected consumers.
- `FactionStartPolicy`: `MeridianRequiresNearbyFossilField = true`, allowed field types = oil/gas and map-scale-aware proximity threshold.
- `RefinementJob`: facility ID, input resource type, reserved crude stock, abstract operating cost, duration, output product and faction eligibility.
- `NuclearRefinementIntel`: source Concord/Directorate research site, qualifying Ascendancy spy acquisition, verified ownership, and weapon-only restriction.
- `NuclearProduction`: both nuclear intelligence unlocks, processed nuclear-material cost, other abstract inputs, very long build duration, authorized faction, completed-bomb event.

Validation checks: every Meridian spawn has an eligible reachable fossil source in range; all declared materials exist on the map or have valid supply methods; storage cannot underflow; a fuel deposit is not confused with refined fuel; electricity outages apply predictably; neither crude oil nor crude nuclear material bypasses refining; Meridian nuclear refinement and nuclear-plant construction are unavailable; Ascendancy nuclear-material refining and weapon construction require stolen Concord/Directorate refinement intelligence and cannot create power plants; nuclear production requires fully committed processed nuclear materials; Ascendancy completion invokes immediate game victory.

### Map population and resource placement

Every playable map supports 2–4 player factions plus one autonomous Unaligned NPC/system-AI faction. A player or AI-opponent controller owns only one playable faction. The Unaligned is not a fifth player. Maps must validate that the resources and field proximity relevant to each selected faction are available; in particular, every selected Meridian Pact spawn must be near an existing oil or gas field even on a 2-player map. The resource system must not allocate extra player slots for neutral Unaligned resource ownership or independent market services. See [Match Structure](MATCH_STRUCTURE.md).

## 8. Open decisions

Set actual resource names, costs, volumes, extraction rates, refinery operating costs and field distribution; implement crude oil and refined fuels as distinct resource states; establish electricity as flow or stock; decide whether nuclear materials are solely captured from nodes or also earned via authored objectives; design how civilian power networks and resource ownership interact with the Unaligned; and test fuel advantages against fair map starts.

Mandatory technology restrictions: Meridian has no nuclear refinery or nuclear power-plant capability; Iron Ascendancy must steal refinement intelligence from a Concord/Directorate nuclear research facility, may then process materials exclusively toward its first bomb, and cannot build nuclear power plants.

The starting rules should be enforced consistently in campaign missions, local skirmish and any eventual multiplayer ruleset, unless a specifically authored scenario explicitly overrides normal starting positions and tells players why.

Locked summary: nuclear materials rank first and must be refined before nuclear power plants or bombs; fossil fuels rank second and crude oil must be refined into petroleum fuel or kerosene; Concord has cheaper fuel refining; Meridian always starts near oil/gas fields and cannot refine nuclear materials; Iron must steal Concord/Directorate nuclear-refining and weapons intelligence, may refine only for weapons and cannot build nuclear power plants; its first completed bomb instantly wins the game.
