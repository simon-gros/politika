# POLITIKA — Resources and Strategic Economy

**Status:** Core resource hierarchy and Meridian Pact starting-position advantage established; quantities, conversion ratios, map distribution, costs and numerical balance remain open.
**Related:** [Concepts Outline](CONCEPTS_OUTLINE.md) · [Nuclear Ultimate Weapons System](NUCLEAR_WEAPONS_SYSTEM.md) · [Story Outline](STORY_OUTLINE.md) · [Faction descriptions](../Factions/)

## 1. Resource hierarchy

POLITIKA's economy assigns the greatest strategic value to **nuclear-weapons materials**, followed by **fossil fuels**. Electricity and other industrial or logistical resources also matter, but their detailed rankings and economies remain to be defined. The ranking is a gameplay design principle, not a real-world commodity price comparison.

| Priority | Resource group | Included examples | Strategic role |
|---|---|---|---|
| **1 — Highest value** | **Nuclear-weapons materials** | One or more scarce, **abstract** nuclear-resource tokens or deposits | Exceptional late-game production input for nuclear ultimate weapons; finite, contested and difficult to accumulate |
| **2 — Second-highest value** | **Fossil fuels** | **Crude oil**, **natural gas**, petroleum-derived fuel supplies, **kerosene** | Energy, vehicle and aircraft operations, industry, logistics, trade and strategic territorial control |
| **Additional systems** | **Electricity** | Grid generation, transmission and available power | Enables or constrains buildings, research, fuel processing and factory operation; shortages affect production |
| **Other resources — to define** | Conventional industrial, construction and logistical inputs | Candidate examples: metals, manufactured components, cash/credits, supply capacity | Conventional production, buildings, repair, trade, upgrades and transport |

**Critical distinction:** crude oil and natural gas can originate from on-map fossil-fuel fields. **Petroleum products and kerosene are processed or stored fuel categories**, not separate naturally occurring underground fields. Gameplay may abstract the refining/processing chain, but map generation should not depict kerosene wells.

**Scarcity and value:** nuclear materials are the **most prized and scarce** strategically; fossil fuels are the **second-most valued group**. Their prices, extraction rates, storage limits, resource costs, regeneration rules and exchange ratios are not fixed in this document. A resource's rank does not guarantee an equal per-unit credit price across all maps or markets.

## 2. Nuclear materials: rare strategic resource

**Nuclear-weapons materials are the most highly valued resource in the entire game.** They should be scarce, geographically contested, strategically defensible and important to late-game victory. Access may require exploration, control, contracts or specialized map objectives.

For nuclear ultimate-weapon production, the following must all be satisfied: **verified nuclear-weapons intelligence**, sufficient **nuclear-material game resources** and other required inputs, an eligible production site and an extremely long game-time production process. The details are abstract fictional game resources; this document does not specify isotopes, enrichment, chemistry, design, or production procedures.

The **Atlantic Concord** and **Red Directorate** are already nuclear powers and can manufacture their tactical nuclear ultimate weapons under these gates. The **Iron Ascendancy begins non-nuclear**: its spies must infiltrate an **Atlantic Concord or Red Directorate nuclear research facility**, acquire intelligence, then acquire the requisite materials and complete its **first atomic bomb**. **As soon as its own bomb is produced, the active game ends immediately in an Iron Ascendancy victory, without delivery or detonation**. A stolen completed weapon does not count as an own-produced bomb.

The **Meridian Pact cannot manufacture nuclear weapons**, although it can sabotage, steal and, with compatible game assets, deploy an already completed weapon. Nuclear weapons and nuclear-intelligence unlocks are not regular Black Market stock.

## 3. Fossil fuels: the second-highest-value resources

The fossil-fuel category includes four named resources or fuel forms:

- **Crude oil:** extracted from designated oil fields; a strategic feedstock for a simplified fuel-processing chain.
- **Natural gas:** extracted from designated gas fields; used as a fuel, trade commodity or energy input.
- **Petroleum / petroleum-derived fuels:** processed fuel supplies, used in vehicle fleets and industrial/logistics systems. Naming should distinguish crude oil from refined petroleum products to avoid double counting.
- **Kerosene:** a processed petroleum fuel, potentially particularly relevant to aircraft readiness, aviation support and trade.

**Refining and conversions are optional implementation details**, not a prerequisite for the first playable prototype. An initial system may use grouped fuel stockpiles while preserving these named resources for later differentiation. Fuel supply should influence movement and logistical reach, production support, trade, and potentially the availability of nuclear delivery platforms without making resource scarcity a meaningless passive modifier.

Fuel fields create contested regional value for all factions, but especially for the Meridian Pact's commercially oriented economy.

## 4. Meridian Pact starting-position advantage

**Canonical gameplay rule:** on game start, **every Meridian Pact player or AI-controlled Meridian starting position must be placed near an existing fossil-fuel source field**. Oil and natural-gas fields qualify. This is a **proximity advantage**, not free ownership, guaranteed extraction, a prebuilt refinery or infinite fuel.

Suggested map-generation and authored-map rules:

1. Identify eligible oil or natural-gas field locations before selecting Meridian start positions.
2. Choose Meridian starting positions **near at least one field** according to a later-defined, map-scale-aware distance threshold.
3. Ensure the position is **reachable, strategically usable and contestable**, with access governed by terrain, jurisdictions, neutral Unaligned patrols and map rules.
4. In procedural generation, **reserve an eligible fossil-fuel site for each Meridian start** or revise/reject the layout if the constraint cannot be met. In authored maps, validate the same invariant and surface an error if it fails.
5. Do not silently transfer Unaligned sovereign ownership of the deposit to Meridian. Normal access, capture, leasing, trade, extraction, and security rules continue to apply.
6. Other factions may also control nearby fuel sources; the Pact's advantage is **guaranteed starting proximity**, not exclusive access to fossil fuel.

This rule makes geography a signature component of Meridian gameplay. Early fuel access supports its roles as an **energy merchant, transport/logistics specialist, mercenary contractor and Black Market operator**, creating a conventional-resource alternative to the major factions' nuclear race.

## 5. Electricity and infrastructure

Electricity is an **additional game resource / utility flow**. Generation facilities, grid infrastructure, storage (if later chosen), transmission routes, and consumption by industrial buildings may affect supply and production. Electricity can be modeled as a local network capacity, an abstract resource stock, or a hybrid; the specific simulation remains open.

Potential relationships worth prototyping include:

- Industrial facilities and nuclear-weapon **game production timers** pausing or slowing when electricity is unavailable.
- Refineries, depots, factories, and research buildings depending on sufficient supply.
- Damage to substations, power plants or transmission links interrupting production or civilian services.
- Regional power availability affecting diplomacy with the Unaligned and faction reputation.

Electricity is not automatically ranked third; further conventional resources may be introduced and ranked later.

## 6. Economic ownership and trade

Maintain separate state for **deposit location**, **territorial jurisdiction**, **legal owner**, **current extractor/controller**, **production/storage**, and **transported inventory**. A natural resource deposit on Yugoslav territory may initially belong to or be administered by the Unaligned, even when a Meridian starting base is nearby.

The **Black Market** may offer conventional fuel supplies, equipment, specialist services, mercenary contracts, and permitted intelligence, under finite stock and physical delivery rules. **Nuclear materials** are not assumed to be freely tradable merely because they rank highest in value; nuclear weapons and nuclear production intelligence remain excluded from ordinary listings. The exact tradeability of abstract nuclear-material tokens must be decided separately, with balance and faction-gate constraints preserved.

Fossil fuels should be sources of **trade income and operational advantage**, not merely decorative map nodes. Convoys, pipelines (if represented), fuel depots, refineries, ports and rail corridors provide conventional RTS interactions: escort, defend, interdict, repair, reroute and seize.

## 7. Simulation data and validation

Suggested data-driven abstractions:

- `ResourceDefinition`: stable resource ID, group/rank, stock/storage rules and extraction/source type.
- `ResourceNode`: resource ID, on-map position, finite supply/regen policy, jurisdiction, owner/controller, harvest access.
- `ResourceInventory`: available/reserved/in-transit amounts by owner/faction.
- `PowerNetwork` or equivalent: optional electricity availability and connected consumers.
- `FactionStartPolicy`: `MeridianRequiresNearbyFossilField = true`, allowed field types = oil/gas and map-scale-aware proximity threshold.
- `NuclearProduction`: intelligence unlock, scarce nuclear-material cost, other abstract inputs, very long build duration, authorized faction, completed-bomb event.

**Validation checks:** every Meridian spawn has an eligible reachable fossil source in range; all declared materials exist on the map or have valid supply methods; storage cannot underflow; a fuel deposit is not confused with refined fuel; electricity outages apply predictably; nuclear production cannot complete without verified intelligence and fully committed materials; Ascendancy completion invokes immediate game victory.

## 8. Open decisions

Set actual resource names, costs, volumes, extraction rates and field distribution; determine whether crude oil and natural gas are separate from all refined fuels in the first playable slice; establish electricity as flow or stock; decide whether nuclear materials are solely captured from nodes or also earned via authored objectives; design how civilian power networks and resource ownership interact with the Unaligned; and test fuel advantages against fair map starts.

The starting rules should be enforced consistently in **campaign missions, local skirmish and any eventual multiplayer ruleset**, unless a specifically authored scenario explicitly overrides normal starting positions and tells players why.

**Locked summary:** **nuclear materials rank first; fossil fuels rank second; the Meridian Pact always starts near oil or gas fields; electricity and other resources also exist; and the Iron Ascendancy remains non-nuclear until successful spy infiltration and completion of its first bomb immediately wins the game.**
