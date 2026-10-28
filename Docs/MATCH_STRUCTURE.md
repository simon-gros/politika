# POLITIKA — Match, Player and Faction Structure

Status: Core match population and faction-control rules locked; online multiplayer and UI implementation details remain provisional.
Related: [README](../README.md) · [Concepts Outline](CONCEPTS_OUTLINE.md) · [Engine Outline](ENGINE_OUTLINE.md) · [Proposed Engine Layers](PROPOSED_ENGINE_LAYERS.md) · [UI Overview](POLITIKA_UI_OVERVIEW.md) · [Nuclear Ultimate Weapons](NUCLEAR_WEAPONS_SYSTEM.md)

## Implemented prototype boundary

The current UE prototype uses a narrower authored test profile than the standard match rules below. Its `faction.player` / `faction.opposition` identities are test identities, not assignments to the four canonical playable factions. Runtime startup preserves authored slot, participant, controller-kind, and faction mapping and creates a separate Unaligned NPC/system authority record, but creates no Unaligned world units or autonomous behavior. The 2026-10-28 Editor Automation exercises malformed authored participant/placement rejection through `BuildScenario`; successful valid retry after such a failure and cooked malformed-participant recovery remain unproven. This prototype does not qualify standard 2–4 faction match setup or functioning AI.

### First Playable Skirmish profile (planned)

The initial integrated match is one human-controlled canonical playable faction versus one AI-controlled canonical playable faction, plus a separately initialized Unaligned neutral/system authority. This is a two-participant match; the Unaligned is not a third selectable army or a proxy for AI behavior. The first scenario uses a compact authored map, two legal starting areas, one resource source, one fixed production base per playable participant, a contestable objective and routes that support gathering and combat. Select actual factions and author historical/map content through the owning scenario/content process; the present test identities are not silently promoted to canon.

The final acceptance requires validated setup and runtime ownership mappings, initial entities, legitimate human and AI command authority, normal terminal outcomes, result presentation, return to menu and a clean same-process rematch. It does not require all four factions, the complete always-simulated Unaligned society, 3–4 participant setup, campaign rules or multiplayer. The general 2–4-plus-Unaligned design above remains the target model, while this first profile is deliberately two playable slots.

## 1. Fixed rule: 2–4 players plus the Unaligned

A POLITIKA map has two to four player-controlled faction slots, plus one separate, always-present Unaligned NPC/system-AI faction. Each player controls exactly one playable faction; no player controls two factions at once. The Unaligned does not occupy a player slot and cannot be selected by a human player.

| Match size | Playable participants / controlled factions | Automatic NPC faction | Faction presences on the map |
|---|---:|---|---:|
| 2-player map | 2 | +1 The Unaligned | 3 (2 + 1) |
| 3-player map | 3 | +1 The Unaligned | 4 (3 + 1) |
| 4-player map | 4 | +1 The Unaligned | 5 (4 + 1) |

Allowed player count: 2, 3 or 4. A single human may play a local skirmish against AI-controlled player slots; this still constitutes a 2–4-participant match. Thus player count is the number of occupied playable faction slots, not necessarily the number of human users. Campaign missions can likewise place an AI opponent in other slots while remaining offline-first.

The four selectable playable factions are The Atlantic Concord, The Red Directorate, The Iron Ascendancy and The Meridian Pact. A match draws its two to four player-faction slots from these four factions. As this is a count of faction presences rather than arbitrary copies of the same army, default match setup uses one slot per chosen faction, without duplicate selections. Faction mirror matches would need an expressly defined future ruleset and should not silently violate the 2+1–4+1 model.

## 2. The fifth faction is always NPC-controlled

The Unaligned is not a fifth player, a selectable fourth-slot alternative or a passive neutral decoration. It is the autonomous Yugoslav federal administration and its institutions, civil infrastructure, JNA, Territorial Defence, police, NPC military and civilians.

Its own population morale, propaganda, legal authority, patrols, structures, economy and diplomatic/hostility responses remain simulated regardless of the number of playable participants. Its units are system-AI controlled, even when the match has four occupied player slots. Normal neutrality and its exceptional hostility toward identified Iron Ascendancy units apply under the same world rules.

Player actions may alter relationships with the Unaligned, but cannot directly transfer control of its faction controller to a fifth player.

## 3. Match setup and modes

Skirmish: choose a map and a match size of 2, 3 or 4 playable participants; fill slots with human or AI controllers consistent with the selected mode. Choose one distinct playable faction for each slot. The Unaligned is injected automatically in the world simulation, not selectable, not counted in the slot total. Match setup validates player and Unaligned spawns and any faction-specific environmental needs.

Campaign: one human controls the selected campaign faction, while any other participating playable faction slots are controlled by scripted or AI opponents/allies. The active mission adheres to 2–4 participant factions + autonomous Unaligned, unless a specifically authored narrative scene is non-combat or otherwise exempt from ordinary playable-map rules. Narrative viewpoints do not allow simultaneously controlling multiple factions.

Potential future multiplayer: the same 2–4 occupied slot limit applies. The available mix of human players, AI replacements, spectators and reconnecting players can be determined later, but spectators do not count as combatant slots and never control the Unaligned.

Meridian special roles: if Meridian Pact occupies a playable slot, that player controls only their chosen Pact forces and authorized assets. It does not thereby assume command of all independent Black Market merchants or system-owned mercenaries. System services remain autonomous.

## 4. Faction-specific map validation

- Unaligned guaranteed: every playable map contains a separate Unaligned world/system faction initialized automatically and not assigned a human/AI player slot.
- Meridian fuel position: whenever a selected participant is Meridian Pact, its start must be near an eligible crude-oil or natural-gas field. This guarantees proximity only; normal access and ownership rules remain.
- Ascendancy nuclear-victory prerequisites: only Concord and Directorate provide eligible enemy nuclear research facilities. In a 2–4 participant match featuring Ascendancy, the special nuclear-victory objective must be enabled only if a qualifying Concord or Directorate research facility exists and can be infiltrated under the authored rules. Otherwise communicate that the condition is unavailable; do not invent a new nuclear-owning faction or count the Unaligned as an eligible research owner.
- Player faction uniqueness: an active player slot maps to exactly one of the four selectable factions, and each selected faction maps to at most one ordinary match participant.
- Match roster validation: playable participant count is 2–4, Unaligned present exactly once as autonomous NPC, and faction-control records do not overlap.

The resource, morale, Intimidation, propaganda, Black Market and nuclear rules remain faction specific regardless of whether a playable slot is occupied by a human or AI. In particular, a player cannot add a fifth controllable faction by converting an isolated enemy unit with Intimidation: that ability transfers unit control, not faction/player ownership.

### Captured individual units versus faction control

The Iron Ascendancy's Intimidation affects eligible isolated combat units after Discipline is broken and eligible isolated civilian/non-combat units after Morale is broken. They are the same underlying resolve meter with category-dependent names. Conversion transfers an individual unit's controller and never allocates a new player/faction slot or allows one player to command two factions. The match remains 2–4 playable factions + Unaligned, regardless of how many units change allegiance.

### Facility espionage does not modify match membership

Atlantic Concord may destroy or make eligible enemy buildings waste resources; the Red Directorate may transfer stolen resources into its budget; the Iron Ascendancy may disrupt staffed facilities with a bounded fictional poisoning/disease chain. These affect structures, workers and economic state inside the existing four selectable faction identities; they cannot create new player slots or convert Unaligned AI into a playable fifth faction. The map still includes exactly 2–4 player factions plus one autonomous Unaligned NPC faction. See [Espionage and Intimidation](ESPIONAGE_AND_INTIMIDATION.md).

### Reconnaissance and sabotage across playable slots

For a match of 2–4 playable factions plus autonomous Unaligned, every participating faction retains its distinct cheap starting scout: Meridian truck, Concord/Ascendancy unarmed light plane, and Directorate rifle-armed motorcycle. Meridian's superior reconnaissance and cheap early explosive sabotage are faction traits, not additional player slots or a general privilege of all neutral vehicles.

Meridian may authorize non-spy saboteurs against eligible enemy buildings/vehicles from the early game; the other three playable factions may perform the comparable conventional explosive-charge action only after later technology, at greater cost and using their spies alone. Unaligned remains NPC-controlled and may independently defend its territory, repair damaged buildings and respond to sabotage. See [Reconnaissance and Scouting](RECONNAISSANCE_AND_SCOUTING.md) and [Explosive Sabotage](SABOTAGE_SYSTEM.md).

### Unaligned balance intervention in a 2–4-player match

The Unaligned remains a separately controlled NPC faction, with the most powerful counterintelligence and the ability to plant false reports and false-flag attributions. Its balance-preservation AI evaluates only the actually participating playable factions for low resources, small surviving armies and imminent elimination threats. It can attempt to distract a dominant aggressor with plausible but false threats about another active rival's nuclear construction, deployments or actions.

The +1 Unaligned actor does not gain a player slot, add phantom player factions, forcibly save an endangered player or modify real production. Two-player maps must work without inventing a third rival; fabricated information may still refer to real rivals' purported plans or to alleged non-existent facilities as mere claims. No false report can change canonical nuclear production eligibility or the immediate victory on Ascendancy first-bomb completion. See [Unaligned Counterintelligence](UNALIGNED_COUNTERINTELLIGENCE.md).

## 5. Engine/UI contracts and acceptance tests

Suggested authoritative fields: `ParticipantSlots` (2–4), `ParticipantControllerType` (human or AI), `ParticipantFactionId` (one of four), `UnalignedSystemFaction` (always true, non-slot), `MatchRosterValidated`, `MapSpawnValidated`, and optional `SpectatorControllers` (separate from playing participants).

Recommended checks:
1. Reject a playable map with fewer than 2 or more than 4 occupied playable faction slots.
2. Reject simultaneous control of multiple factions by one player.
3. Reject duplicate playable faction selection under the standard ruleset.
4. Always create exactly one autonomous Unaligned faction; prevent selecting it as a player faction or counting it as a player.
5. Validate 2+1, 3+1 and 4+1 faction-presence counts against the spawned match state.
6. A local skirmish with one human and one AI opponent is a valid 2-player match, plus the Unaligned.
7. Validate Meridian's nearby fuel-field start, when selected.
8. Do not expose Ascendancy nuclear instant-victory progression without an eligible Concord/Directorate nuclear research facility.
9. Ensure militia, captured vehicles and Intimidation-converted units remain owned by a player faction, never becoming new faction/player slots.

The menu should present 2–4 playable positions and a separate non-editable Unaligned: NPC/system AI status. UI should not label a 2-player map “3 players” merely because the Unaligned is present.

Locked summary: 2–4 players = 2–4 playable factions, one per player; plus one non-selectable Unaligned NPC faction = 3–5 faction presences per map.
