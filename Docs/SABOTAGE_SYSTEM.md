# POLITIKA — Explosive Sabotage and Faction Access

Status: The relative faction access, timing and cost of conventional explosive sabotage are established; individual unit eligibility, charge rules, damage, cooldowns and detection remain to be balanced.  
Related: [Reconnaissance and Scouting](RECONNAISSANCE_AND_SCOUTING.md) · [Espionage and Intimidation](ESPIONAGE_AND_INTIMIDATION.md) · [Concepts Outline](CONCEPTS_OUTLINE.md) · [Resource Economy](RESOURCE_ECONOMY.md) · [UI Overview](POLITIKA_UI_OVERVIEW.md)

## 1. Definition and faction hierarchy

Conventional explosive sabotage is an RTS action in which an authorized unit attaches an abstract explosive charge to an eligible enemy vehicle or building. A successful operation leads to an in-game explosion and damage or destruction of the target under conventional combat rules.

The Meridian Pact has the strongest and most accessible sabotage capability of any playable faction. It specializes in sabotage rather than sophisticated conventional spying. The three principal military rivals also have access, but at significantly greater cost, later in the match, and exclusively through their spy units.

| Faction | Timing | Relative cost / effectiveness | Eligible initiators | Core identity |
|---|---|---|---|---|
| Meridian Pact | Early game | Cheap and strongest overall explosive-sabotage advantage | Qualified Pact non-spy units as well as any specifically authorized specialist units; not spy-only | Rapid, accessible vehicle/building sabotage paired with fast reconnaissance and mobility |
| Atlantic Concord | Later game | More expensive than the Pact | Spies only | Limited conventional explosive sabotage alongside its separate Psy-Ops specialization |
| Red Directorate | Later game | More expensive than the Pact | Spies only | Limited conventional explosive sabotage alongside efficient intelligence theft and resource diversion |
| Iron Ascendancy | Later game | More expensive than the Pact | Spies only | Limited conventional explosive sabotage alongside Intimidation and fictional facility-contamination operations |
| The Unaligned | NPC-only | Not a selectable player sabotage tech tree | Autonomous security and defensive AI | Responds to unauthorized explosive actions and defends protected infrastructure |

"Best sabotage" is a locked comparative faction advantage, not an unconditional guaranteed kill against any target. The precise degree of cost, availability and operational efficiency advantage is an open balance matter.

## 2. Sabotage targets and resolution

Only eligible enemy vehicles and buildings may be targeted. The actor must reach a valid gameplay interaction range and meet faction, unit, scenario and target eligibility requirements.

The attack sequence is abstract: `Authorized actor → eligible target → charge-placement game interaction → successful sabotage state → game-defined explosion → health/damage and ownership consequences`.

The charge is a fictional inventory/status object, not a modeled real-world device. The design does not specify real explosives, attachment methods, activation circuits, timings or placement instructions. Detonation timing, defensive discovery, abort, prevention, persistence, repair and whether a charge can be disarmed are gameplay design choices.

Effects may include lost vehicle health, damaged or destroyed buildings, production interruption, repair costs, staff/occupant losses if separately authorized by the scenario, and appropriate Unaligned jurisdiction or public consequences. Losses do not grant the saboteur automatic ownership of surviving facilities or technology.

## 3. Meridian Pact as the sabotage specialist

Meridian sabotage is an early-game capability with low relative costs and broad eligible-unit access. Qualified non-spy Pact units may carry out conventional explosive sabotage; the game must not require a full espionage tech tree or spy unit for every Pact operation.

Its cheap, very fast, lightly armored and lightly armed scout-combat trucks give it the greatest overall early-game reconnaissance advantage. Reconnaissance helps the Pact discover targets and travel routes, but scouting and explosive sabotage remain different actions with independently checked permissions. A scout truck is not automatically an explosive saboteur unless its unit definition grants that ability. Specific qualified vehicle, infantry or specialist archetypes remain to be authored.

This combination makes the Pact a distinctive early raiding and disruption faction, without granting nuclear manufacturing, advanced propaganda, or the Atlantic Concord's Psy-Ops. A Pact sabotage charge is always conventional; it is not a nuclear bomb or a purchasable nuclear Black Market listing.

## 4. Restrictions on the other three factions

For Atlantic Concord, Red Directorate and Iron Ascendancy, ordinary attached-charge explosive sabotage is:

1. Unavailable at the start of a standard match; it requires a later-game unlock.
2. More expensive in relevant game resources/cost than the Meridian Pact's equivalent.
3. Available only to authorized spy units; infantry, scouts, vehicles and non-spy support units cannot perform this specific action, even if they have other combat capabilities.

Separate faction-specific spy effects remain intact:

- Concord Psy-Ops can disrupt eligible units, or target buildings with abstract self-destruction/resource-waste results; do not assume those outcomes are the same physical action as placing a conventional charge.
- Directorate spies can steal intelligence slightly faster/cheaper and covertly divert enemy resources into Directorate budgets. Theft and explosive destruction are separate transactions.
- Ascendancy spies can induce fictional poison/disease incidents in eligible staffed buildings with bounded chain effects, and Intimidation converts eligible isolated units after breaking Discipline/Morale. These are distinct from explosive sabotage, and neither shortcut nuclear research infiltration.

The other factions may discover sabotage incidents through scouting and investigation, but their spies must still satisfy the late unlock, cost and eligibility rules for performing an actual charge action.

## 5. Simulated damage, detection and counterplay

Sabotage should integrate with building/vehicle health, conventional damage, repair and evidence/attribution systems. Detection of a suspicious actor or charge may occur before an explosion; after destruction, witnesses or security may still lack reliable attribution. The Unaligned system AI can react to unauthorized sabotage on Yugoslav territory under its usual sovereignty rules.

The defending player must have reasonable counterplay through patrols, checks, defenses, interception and possible charge removal or evacuation according to authored match rules. A detected charge can produce a clear warning; concealed actions should not leak attacker identity through omniscient UI alerts. Avoid unlimited consecutive cheap placements, unavoidable map-wide explosions or arbitrary destruction of scenario-critical civilian infrastructure.

A saboteur is not automatically a spy: unit identity, faction permission, capability and tech access are separate from detection and disguise status. Successful sabotage should not grant research, transfer resources, recruit a unit or bypass the Iron Ascendancy's first-bomb victory progression.

## 6. Engine, UI and testing contracts

Suggested data-driven state:

- `ExplosiveSabotageCapability`: enabled faction, eligible initiator tags, scenario permission, tier, available phase, cost and cooldown.
- `SabotageTargetEligibility`: enemy ownership/hostility, target class (`Vehicle` or `Building`), access conditions and protected-object exclusions.
- `PlacedCharge`: fictional status/asset ID, target ID, responsible faction, authorization and gameplay resolution state.
- `SabotageResolution`: target health change, conventional damage event, resource/production consequences and resulting repair/destroyed status.
- `SabotageDiscovery`: actor/charge detected, attribution, counterintelligence and UI visibility.
- `FactionSabotageIdentity`: early/cheap/qualified non-spy for Meridian; late/expensive/spy-only for Concord, Directorate and Ascendancy.

Acceptance tests:

1. Meridian Pact can use the action early via at least one authorized non-spy unit type; its relative cost/availability is better than each rival's.
2. Each rival faction cannot use attached-charge sabotage before its later-game unlock, even if it already owns spies.
3. Each rival faction's authorized spy can perform the action after unlock and payment; a regular infantry unit, scout aircraft or non-spy vehicle cannot.
4. Eligible enemy vehicles and buildings can be targeted, with damage/destruction resolved by conventional authoritative health rules.
5. No civilian, Unaligned or scenario-protected target becomes automatically eligible solely because it is a building or vehicle.
6. Scouting a target does not automatically place a charge. Psy-Ops, resource theft, facility contamination and Intimidation cannot silently invoke this ability.
7. Charge discovery, prevention, defender warning and attribution respect visibility/AI rules and do not disclose covert ownership prematurely.
8. Damage and repair cannot invent resources or produce a nuclear weapon. Match control remains one faction per player, 2–4 playable factions plus autonomous Unaligned.

Open balancing: exact early/late unlock milestones, relative pricing, actor archetypes, charge limit and lifetime, damage, unit/building eligibility, detection range, defensive countermeasures, and special-campaign restrictions.

Locked summary: Meridian Pact has the strongest cheap, early, non-spy-exclusive conventional explosive sabotage against eligible enemy vehicles/buildings. Atlantic Concord, Red Directorate and Iron Ascendancy can also do it, but only later, more expensively and through spies alone.
