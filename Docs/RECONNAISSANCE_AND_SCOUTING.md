# POLITIKA — Early-Game Reconnaissance and Scouting

Status: Core faction-specific reconnaissance roles and relative early-game advantage are established. Unit prices, exact speed, spotting radius, field of view, counters and production prerequisites remain to be balanced.  
Related: [Concepts Outline](CONCEPTS_OUTLINE.md) · [Espionage and Intimidation](ESPIONAGE_AND_INTIMIDATION.md) · [Resource Economy](RESOURCE_ECONOMY.md) · [UI Overview](POLITIKA_UI_OVERVIEW.md) · [Faction Descriptions](../Factions/)

## 1. Canonical early-game reconnaissance asymmetry

Each of the four playable factions has a different affordable early-game scouting platform. These units reveal terrain, enemy deployments and points of interest through normal fog-of-war/visibility gameplay rather than acquiring classified intelligence by espionage.

| Playable faction | Early-game scouting unit | Armament / protection | Tactical role |
|---|---|---|---|
| Meridian Pact | Cheap, very fast, lightly armored and lightly armed truck | Light armor and light weapons | Half-scout/half-combat vehicle; rapid traversal and useful limited fighting capability; greatest overall reconnaissance advantage of all playable factions |
| Atlantic Concord | Cheap, light, unarmed reconnaissance airplane | No weapons; light and vulnerable | Quickly scout from the air at the start of the game; avoid fighting |
| Iron Ascendancy | Cheap, light, unarmed reconnaissance airplane | No weapons; light and vulnerable | Quickly scout from the air at the start of the game; avoid fighting |
| Red Directorate | Motorcycle-based reconnaissance equipped with basic infantry rifles | Basic infantry rifle armament; minimal protection | Mobile ground scouting with limited rifle fire and exposure to enemy response |
| The Unaligned | NPC world patrols, administration, police and military observation | According to NPC unit type | Autonomous world surveillance, not a fifth selectable reconnaissance faction |

The Meridian Pact's superiority is an overall, comparative early-game reconnaissance advantage, not an assertion that its truck outruns an airplane or sees through fog anywhere on the map. Its cheap production, unusually fast ground-map traversal and combined spotting/light-combat flexibility make it the strongest reconnaissance-oriented playable faction. The Pact is already guaranteed to start near an oil or natural-gas field; scouting trucks complement that advantage by exploring access routes, deposits, markets and contested infrastructure.

## 2. Meridian Pact: scout-combat trucks

The Pact can cheaply manufacture lightly armored, lightly armed trucks early in a match. Their defining characteristics are:

- Very high map-traversal speed for their ground-vehicle class.
- Low production cost and early availability, enabling several scouting routes without major investment.
- Lightweight protection and weapons: they can engage weak targets or defend themselves in limited encounters but cannot replace a proper armored combat force.
- A mixed role: roughly half scouting and half light combat, with both uses important in normal gameplay.
- Strong reconnaissance efficiency and map coverage compared with the other playable factions.

The exact scouting range, weapons/damage values, survivability, fuel consumption, terrain traversability, availability by map and unit-cap balance are not yet fixed. They remain ordinary vulnerable ground vehicles and cannot magically ignore obstacles, Unaligned jurisdiction or hostile defenses.

## 3. Atlantic Concord and Iron Ascendancy: early unarmed planes

Both factions can cheaply produce light, unarmed reconnaissance aircraft early in the game. These are dedicated air scouts rather than bombers, troop transports or attack aircraft. They provide an early aerial view across accessible terrain, but cannot attack enemy units or facilities.

Their flight speed, sensing range, anti-air vulnerability, airfield or launch requirements, supply needs and visibility counterplay are to be defined. The same platform class may have faction-specific models and production requirements later, but the shared cheap, unarmed, light early-game plane role is established.

The Atlantic Concord's lack of a solo/small-unit combat Discipline penalty concerns soldiers, not immunity from aircraft interception; its Psy-Ops specialization is separate from flying a scout. The Iron Ascendancy's initially limited espionage and required full spy progression for nuclear research infiltration are not bypassed by aerial reconnaissance.

## 4. Red Directorate: rifle-armed motorcycles

The Red Directorate's early reconnaissance uses motorbikes with basic infantry rifle armament. They are relatively light ground scouts that can observe, move and engage with basic rifle fire, but are not heavy cavalry, armored trucks or specialized infiltration teams.

Exact motorcycle cost, rider/weapon configuration, mobility, speed relative to the Pact trucks, sight range and counterplay need playtesting. Their primary purpose is early scouting and route awareness, with limited self-defense. Cheap Directorate infantry and massive-army Discipline bonuses remain distinct faction traits, not automatic enhancements to every motorcycle scout.

## 5. Scouting versus espionage, propaganda and targeting

Reconnaissance provides line-of-sight/fog-of-war information, contacts, route mapping and observed buildings. It does not independently steal research blueprints, transfer budgets, trigger Psy-Ops, spread facility contamination, create Intimidation conversions, or grant technology flags.

- Atlantic Concord Psy-Ops targets eligible units or buildings through an authorized spy action, not a basic reconnaissance-airplane flyover.
- Red Directorate's slightly faster/cheaper rival intelligence theft and covert resource transfers require appropriate infiltration actions, not merely sending motorcycles past a facility.
- Iron Ascendancy nuclear-research infiltration still requires fully upgraded spying and an eligible Concord/Directorate nuclear research facility, even if a reconnaissance airplane has already discovered it.
- Meridian reconnaissance trucks cannot refine nuclear materials, create an advanced propaganda capability or access nuclear research through scouting alone.

Scouting interactions with the Unaligned respect its system-AI neutrality, protected facilities, territorial rules and reaction to trespass or combat.

### Reconnaissance does not automatically grant sabotage

The Meridian Pact is both the game's best early reconnaissance faction and its strongest conventional sabotage faction. Its low-cost, fast, lightly armed and armored trucks support scouting, target discovery and raids, but carrying out an explosive sabotage action still requires an explicit unit ability. Some authorized non-spy Meridian units can perform early low-cost sabotage against eligible vehicles or buildings; a scouting truck does not inherit this permission just because it has a spotting role.

Atlantic Concord and Iron Ascendancy light unarmed planes, and Red Directorate rifle-armed motorbikes, are reconnaissance units and are not sabotage agents. Their three factions gain conventional attached-charge sabotage later, at greater cost, through spy units only. A scout's sighting of a target can inform an authorized spy operation but does not place a charge, grant nuclear research access or trigger other special actions. See [Explosive Sabotage](SABOTAGE_SYSTEM.md).

### False sightings and Unaligned planted intelligence

The Unaligned has the strongest counterintelligence system and may plant false reports about rival deployments, facilities or alleged nuclear construction. Such an intelligence report is not the same as a genuine sighting by a Meridian truck, Concord/Ascendancy scout aircraft or Directorate motorcycle. Ordinary reconnaissance can test or disprove a false location and should never display a reported structure as a physically observed one before confirmation.

These misleading reports may be issued when an active playable faction with very low resources and a very small army is near defeat, encouraging stronger rivals to investigate competing threats instead of eliminating it. The Unaligned cannot force the player to turn scouts around, override line of sight or conjure real buildings. See [Unaligned Counterintelligence](UNALIGNED_COUNTERINTELLIGENCE.md).

## 6. Suggested simulation and UI contracts

Represent cheap scouts with normal unit data rather than a global reveal cheat:

- `ReconUnitClass`, `FactionId`, `AvailableFromGamePhase`, `ProductionCost`, `MovementClass`, `MovementSpeed`, `ArmorClass`, `Weapons`, `SightRadius`, `DetectionCapabilities`, `CanAttack`, `FuelUse` and `SelectionRole`.
- `FactionReconIdentity`: the Pact's distinctive early truck scouting advantage; Concord/Ascendancy unarmed light aerial scouting; Directorate rifle-armed motorcycle scouting.
- `FogOfWarVisibility`: observed cells/areas, current versus stale contacts, terrain/obstacle occlusion where applicable, airborne detection rules, and diplomacy/jurisdiction.
- `ReconEfficiency`: a balancing/telemetry metric comparing scouting coverage and cost, not a universal magical spotting multiplier.

UI should show whether a scout is unarmed (light planes), lightly armed (Pact trucks) or rifle-armed (Directorate motorcycles), plus movement class, spotting capabilities and vulnerability. Recon command icons and build cards must not misrepresent light scouting aircraft as nuclear bombers or combat airplanes. The Pact's early recon superiority should be easy to recognize in faction selection and match planning.

## 7. Acceptance scenarios and open balancing

1. Each playable faction can obtain its specified cheap early scout unit during the opening phase, under normal production/resource rules.
2. Pact scouting trucks travel very quickly on passable map routes, spot enemies using ordinary visibility, and can undertake limited combat; they are not armored heavy hitters.
3. Concord and Ascendancy light reconnaissance planes can scout but cannot attack; their appearance early does not imply that they already have bombers or nuclear weapons.
4. Directorate motorcycle scouts can reveal terrain and use basic infantry rifle fire, without an unintended heavy-weapon or intelligence-theft bonus.
5. At comparable early-game cost and map conditions, Meridian is the strongest overall reconnaissance-oriented faction, assessed across scout availability, traversal, map coverage and scout-combat versatility; the precise numeric tuning remains open.
6. Scouting a nuclear research facility does not fulfill the Iron Ascendancy's special espionage technology/intelligence gate.
7. All scout units respect fog rules, route obstruction (where appropriate), hostile fire, neutral Unaligned restrictions and supply constraints.
8. In maps with 2–4 player factions plus Unaligned, reconnaissance works with any valid subset of playable factions and does not spawn a fifth selectable player.

Balance speed, price, production time, fog reveal, detection, armor, damage, scouting efficiency, terrain/air access, early counters and resource use later. Do not presume that cheap aircraft are faster at every kind of reconnaissance simply because they fly.

Locked summary: Meridian Pact has the greatest overall reconnaissance advantage through cheap, very fast, lightly armored/armed half-scout/half-combat trucks; Atlantic Concord and Iron Ascendancy use cheap, unarmed light reconnaissance airplanes; Red Directorate uses basic-rifle-armed motorbikes.
