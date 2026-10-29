# POLITIKA UI — product overview

Status: expanded RTS interaction, information-integrity and multi-platform UI design contract (2026-10-27); implementation is narrower and separately evidenced
Engine target: Unreal Engine 5.8.3
Design emphasis: readable RTS commands, map awareness and fast tactical interaction
Product constraints: no cinematics; no mandatory internet connection or online-account login
Display target: desktop 4K (3840×2160) plus readable, adaptive handheld/mobile resolutions
Platform targets: Linux, Windows, SteamOS/Steam Deck and Android; console/Apple platforms are possible later expansions

**Detailed settings/input implementation guide (29 October 2026):** [Practical RTS UI, Settings and Keymap](RTS_UI_SETTINGS_AND_KEYMAP.md) records the exact source-backed current controls (including the current A/S camera-and-order collisions), suggested conflict-free default profile, real video preview/revert transactions, UI versus render scale, audio bus categories, input mapping/help, menu hierarchy, and staged tests. This document remains the authority for overall UI product design and faction presentation; the linked guide is a proposed engineering contract, **not a working settings screen or remappable keymap**.

**HUD-specific design authority (29 October 2026):** [RTS HUD Architecture](RTS_HUD_ARCHITECTURE.md) defines the comparative RTS design rationale, HUD semantic ontology, module/widget contracts, participant-safe presentation, technical seams and HUD-01–HUD-11 implementation slices with HUD-01–HUD-13 proposed test IDs. It is **design-only**; this overview retains product/visual direction and does not imply a UMG-based interactive HUD is built.

**Cross-system boundary (29 October 2026):** The [Selection and Group Orders](RTS_SELECTION_AND_GROUP_ORDERS.md) specification owns authoritative selection membership, per-unit group submission and rejected-order semantics. [Visibility and Intelligence](RTS_VISIBILITY_AND_INTELLIGENCE.md) owns per-participant observable state; this UI must render that view rather than unfiltered simulation truth. [RTS Systems Completeness Matrix](RTS_SYSTEMS_COMPLETENESS_MATRIX.md) records which behaviors remain design-only. These links do not qualify a new UI feature or plugin.

## 1. Product direction

POLITIKA's interface should make the player feel like a commander managing a fluid operation, not an operator navigating a software dashboard. It should keep the map and tactical decisions central, surface the next useful action, and make the state of squads, vehicles, objectives, and orders easy to read at a glance.

Use a compact RTS command loop—select, inspect, issue an order, see clear feedback—without committing the layout to any particular economic or production system. Establish POLITIKA-specific terminology and strategic rules as those systems are decided.

The visual language should evoke plausible 1980s Yugoslav military and civil-administration materials: printed topographic maps, field reports, typed labels, stamped status marks, restrained institutional color, and period-appropriate symbols. Use these as framing and texture, not as a reason to reduce contrast, legibility, or interaction clarity. Avoid making every screen look like a faux terminal.

## 2. Design principles

1. Battlefield first: the map remains visible and is the primary command target; HUD panels organize decisions around it.
2. Fast, predictable commands: keyboard/mouse, handheld controller and touch UI actions should invoke the same orders and share clear cancel/invalid-target behavior.
3. Show what matters now: persistent HUD space is reserved for objectives, selected-force status, essential resource/production state, and match-critical alerts.
4. Explain consequences: show costs, prerequisites, target requirements, cooldowns, and unavailable states before commitment where possible.
5. Respect uncertainty: map and HUD only show what the player's units/sensors are allowed to know; stale or uncertain intel must not look current.
6. Readable at every supported resolution: 4K is a desktop capability, not the required render size on Steam Deck or Android. Support responsive layouts, screen safe areas, suitable touch targets and useful scale/text settings.
7. Offline by default: the game should reach its main menu and support its single-player modes without internet, account creation, or sign-in.
8. Story without cinematics: narrative context is delivered in concise briefings, radio dialogue, mission events, debriefs, and documents; gameplay remains the visual focus.

## 3. Main navigation and local-first operation

The front end should prioritize direct play:

- Continue when a resumable local campaign save exists;
- Campaign for mission selection and briefing;
- Skirmish for local match setup against AI;
- Load / Save for local progress where supported;
- Options for display, audio, controls, accessibility and gameplay settings;
- Credits and Quit.

No menu should block on account login, entitlement checks requiring a network, or a connection to a publisher service. Saves, settings, campaign progress, and unlocks should be stored locally. **Networking is an immediate engineering priority**; LAN/online session UI is staged after minimal MP-1/MP-3 transport, and it must be additive: offline campaign/skirmish remains usable, and sign-in failure cannot strand the player outside local content. **Multiplayer is an immediate development priority**, with 2–4 legal playable-faction slots, up to four human connections and player-hosted listen/dedicated-server architecture. It is not yet an implemented menu flow or a release guarantee. The [Multiplayer Architecture](RTS_MULTIPLAYER_ARCHITECTURE.md) requires auditing all existing player-input/HUD/menu code for remote-client and server authority boundaries before networking; optional lobby/join/hosting UI remains deferred.

**Immediate UI integration gate:** [MP-0](RTS_MULTIPLAYER_ARCHITECTURE.md#21-mandatory-whole-repository-multiplayer-retrofit-audit-before-mp-1) must inspect existing controllers, selection, HUD, input routing, GameInstance/menu initialization and presentation callbacks for single-local-player assumptions. MP-1 needs only diagnostic join/leave and command feedback, not a finished multiplayer lobby. No UI may claim online availability until independently tested.

## 4. Campaign and narrative screens (no cinematics)

Use authored in-game UI and audio rather than pre-rendered or real-time cinematic sequences:

- Mission briefing: map/region, situation, friendly forces, known enemy activity, primary/secondary objectives, constraints, and a concise commander's note. Present it as a field dossier or operations map, with optional voice/radio delivery and captions.
- Mission start: transition from briefing to playable map with a short title/objective cue, not a cinematic intro.
- In-mission story: objective updates, radio exchanges, map annotations, field reports, and authored gameplay events. Keep dialogue skippable and never pause tactical input unexpectedly in an active match.
- Debrief: mission outcome, objective completion, casualties or relevant performance summary, campaign consequences, and next action. Avoid mandatory cinematic reward sequences.

Briefings and dialogue should be concise, pausable/skippable, subtitle-capable, and available in text for players who mute audio or need to revisit an instruction.

### Parallel storylines, refining and faction ability presentation

Keep the primary Tito assassination/cover-up narrative and secondary Ascendancy first-bomb nuclear race independently legible in mission briefings and debriefs.

Only the Atlantic Concord and Red Directorate start with nuclear power. Both have rare tactical nuclear ultimate weapons: Concord deploys with a nuclear bomber and Directorate via an ICBM. The Iron Ascendancy begins non-nuclear and with limited espionage. It must fully upgrade its spy capabilities before it can infiltrate a Concord or Directorate nuclear research facility, obtain both nuclear-refinement and weapons intelligence, refine nuclear materials for bomb production, and complete the long build. As soon as its first own-built bomb completes, it immediately wins the current game—there is no launch/deployment action.

**The primary purchasing currency is in-game US dollars ($).** The primary economy HUD shows available **$**, current income, **electricity capacity/required load** (every operational building needs minimum power), and **food reserve/upkeep/runway** for staffed military forces, who must be fed with food purchased using dollars. Gold, crude oil, natural gas and petroleum/kerosene inventories are available as secondary tradeable stocks with eligible USD sale quotes; buying and selling must use real owned inventory and authorized settlement. The established **strategic** ranking is nuclear materials first and fossil fuels second; ordinary nuclear-stock exchange remains forbidden. Expandable panels handle the longer refining and nuclear-chain information rather than overfilling the battle HUD. See [USD economy, commodity exchange and upkeep](RTS_OPERATIONAL_ECONOMY_AND_POWER.md) for state and fail-safe rules.

Specialized espionage: provide a Concord Psy-Ops view with two outcomes—fully break Discipline (combat) or Morale (non-combat) and cause in-game self-destruction, or temporarily force a friendly-fire rampage with a clear short duration and expiry. The UI must identify valid targets, the shared `UnitResolve` state (labeled for the unit category), result, range, and prevention options without suggesting permanent capture. The Red Directorate shows a modest time and cost reduction for rival-intelligence theft in ordinary spy infiltration tooltips, with no promise of guaranteed success. Show the Ascendancy's spy-upgrade progress and clearly communicate that nuclear research infiltration requires full development. The Ascendancy alone has Intimidation, which may convert eligible isolated combat or non-combat units only after their Discipline or Morale has been fully broken. Its target cursor/tooltip must distinguish isolated and eligible from supported or ineligible, with clear success/failure and ownership-change feedback. Never display Intimidation as an available native action for any other faction.

Use existing objective, production, selected-unit and intel UI patterns. Show only information the faction has legitimately discovered: do not expose hidden enemy nuclear-research locations or spy identities without detection. Nuclear weapons cannot be purchased in the Black Market, and stealing a completed bomb does not confer manufacturing knowledge or Ascendancy instant victory.

See [Espionage and Intimidation](ESPIONAGE_AND_INTIMIDATION.md), [Resource Economy](RESOURCE_ECONOMY.md), and [Nuclear Ultimate Weapons System](NUCLEAR_WEAPONS_SYSTEM.md).

### Population morale and propaganda controls

Display the population morale meter for every faction, including Meridian Pact and NPC Unaligned where the game's information/fog rules permit it. Make the two principal positive consequences—production output and army discipline—understandable through tooltips, modifiers, unit inspection or production panels.

Propaganda-capable factions (Atlantic Concord, Red Directorate, Iron Ascendancy and NPC Unaligned) use distinct methods expressed through radio, newspapers and television. Player-facing propaganda controls belong only to the three playable eligible factions; the Unaligned is AI-controlled. Do not display an advanced propaganda action tree for the Meridian Pact, even though its morale remains present and mechanically relevant. Media control, regional coverage and propaganda effects should be reflected in mission objectives and event notifications. See [Propaganda and Morale](PROPAGANDA_AND_MORALE.md).

### Unaligned intelligence claims, false flags and verification

The Unaligned can inject reports into a playable faction's intelligence feed, even though the source may appear to be a rival transmission, covert observation or seemingly credible third-party account. A report might allege that another player is building a nuclear facility when no such structure exists. Represent it as an intelligence claim or alleged-map marker, not a genuinely scouted building or real nuclear research asset.

Support confidence/provenance cues, `unverified` or disputed states, report age, investigation or independent-reconnaissance actions, and correction when a claim is disproved. Do not show secret `is_fabricated` or the Unaligned's responsibility before discovery; equally do not suppress direct factual scouting that finds an empty location. False-flag attribution can change who an unverified report alleges is responsible without changing actual unit ownership or diplomacy.

The Unaligned's NPC balance director may plant such reports when a playable faction has very few resources and surviving forces, attempting to divert stronger players and preserve overall competitive equilibrium. The UI must leave final decisions to the human player, provide no compulsory retargeting and not reveal the privileged strength analysis or fabricated nature of a report by default. Critical victory conditions and the truth of nuclear research are never decided by the presence of a report. See [Unaligned Counterintelligence](UNALIGNED_COUNTERINTELLIGENCE.md).

### Building espionage actions and incident visibility

Facility target actions must be distinct from the individual-unit Discipline/Morale effects. A selected Concord spy can see eligible building Psy-Ops outcomes—self-destruction or resource waste; a Directorate infiltrator may redirect eligible resource budgets; an Ascendancy operative can initiate an abstract staffed-facility contamination action. None of these require reducing a building's `UnitResolve` (buildings do not use that meter).

The attacker should see authorized action progress/costs and its own valid outcomes. The defender may initially see only ordinary-looking budget shortages or production losses: espionage success, discovery and attribution are separate. The UI must not automatically announce a covert waste or diversion when it has not been discovered. If the incident is detected, show contextual investigation, containment/cleanup, workforce replacement, repair and security options as appropriate.

Building-inspection panels can represent assigned/required workers or researchers and effective throughput. Iron Ascendancy sabotage can cause fictional staff casualties, a production slowdown/stoppage, and a bounded local chain of impacted facilities; show nearby warning indicators only if observed/discovered. Avoid medical or technical real-world toxin/pathogen procedures. Directorate diversion should show a valid debit in the enemy's actual inventory and credit in the Directorate's, even if the enemy has no attacker-attribution. Nuclear infiltration and victory prerequisites remain independent of these sabotage controls.

### Reconnaissance commands and sabotage targeting UI

Early build cards must distinguish the cheap, fast, lightly armored and lightly armed Meridian Pact truck (dual scout/light-combat role; strongest overall reconnaissance faction), cheap unarmed Concord/Ascendancy light scouting airplanes, and rifle-armed Directorate motorbike scouts. Unarmed planes may reveal fog but cannot display a usable attack command. The scouting interface should convey movement type, visibility coverage, cost and threat exposure without implying a flyover steals classified intelligence.

An eligible Meridian non-spy saboteur can target an enemy vehicle or building with early low-cost fictional explosive-charge sabotage. For the other three factions, the same type of sabotage must appear locked until late-game technology and must not be actionable by anything except an eligible spy. Targeting cursors should distinguish valid enemy vehicles/buildings, forbidden/protected targets, invalid actors, missing costs and unavailable tech. An ordinary scouting truck receives a charge command only if its unit definition grants it; rival scouting planes and motorcycles never gain the spy-only ability by default.

The defending interface distinguishes whether a charge or responsible infiltrator is discovered from whether an explosion has occurred. Successful sabotage uses ordinary conventional target health and damage results, separate from Psy-Ops self-destruction, budget theft, facility contamination or nuclear armaments. See [Reconnaissance and Scouting](RECONNAISSANCE_AND_SCOUTING.md) and [Explosive Sabotage](SABOTAGE_SYSTEM.md).

### Discipline and Morale: one meter, contextual labels

Unit inspection: show “Discipline” for a combat unit and “Morale” for a civilian/non-combat unit, using the same underlying unit resolve value, scale, break threshold and recovery behavior. Do not display both gauges simultaneously for the same individual. Population Morale remains the faction-level public indicator and may appear on the strategic/economic HUD, with production and baseline army Discipline effects.

Formation tooltips: Red Directorate, Iron Ascendancy, Meridian and Unaligned combat units gain standard Discipline stability from coherent allies. Directorate and Ascendancy gain a separate very-massive-army Discipline bonus; Concord solo/small-group combat units do not incur a group-size Discipline penalty. Civilian Morale does not show those military formation bonuses.

Intimidation target feedback: eligible isolated targets show the relevant label: Discipline if combat, Morale if non-combat. Indicate Intimidation's meter reduction and conversion availability only after the meter is fully broken. Eligibility and fog rules control how much of an enemy gauge can be seen; the UI must not infer conversion simply from an isolation indicator. See [Espionage and Intimidation](ESPIONAGE_AND_INTIMIDATION.md).

### Two-to-four-player match setup

Map/match setup supports 2, 3 or 4 participant slots, with one distinct playable faction per slot (Concord, Directorate, Ascendancy or Meridian). In local single-player, remaining slots may be filled by AI opponents. The Unaligned always appears as a separate non-editable NPC/system AI presence, not a fifth selectable faction or player slot. Explain the map population as 2+1, 3+1 or 4+1; the +1 is always Unaligned.

Match validation should flag too few/many participants, duplicate faction assignments, missing Unaligned initialization, missing Meridian nearby fossil-fuel source, or unavailable special nuclear infiltration objectives where no qualifying Concord/Directorate research facility is present. See [Match Structure](MATCH_STRUCTURE.md).

## 5. In-match UI structure

Exact placement can be tuned with prototypes, but the initial layout should provide these functional regions:

| Region | POLITIKA role |
|---|---|
| Battlefield | Main view for movement, combat, selection, target designation, and tactical feedback |
| Selection/force panel | Selected individual/mixed-force identity, health/readiness, subgroup/formation status and common versus subset-valid commands; optional 0–9 clickable control-group bar |
| Command/ability panel | Contextual orders and selected unit abilities, with hotkey hints and unavailable reasons |
| Strategic map | Camera navigation, objectives, friendly positions, and only currently known enemy/intel markers |
| Objective panel | Current primary/secondary objectives, state changes, and explicit completion/failure feedback |
| Resources/force status | The chosen POLITIKA economy, deployment/production state, and any unit/force cap that affects decisions |
| Alert/status layer | Under-attack, completed order, lost communication, objective change, and other time-sensitive notices |

The resource and production panel should reflect POLITIKA's own gameplay rules, whether those use mobile command posts, fixed-base production, reinforcements, or a hybrid. Show only the resources and limits the player can act upon. Until economy implementation is qualified, money/power/food controls must be disabled or Development-labelled rather than presenting invented live values. Once active, show actual **$ affordability**, a food purchase/auto-restock option, and why a producer is disabled by inadequate electricity; never label a shortage merely "out of resources".

### Command interaction

Use one consistent cycle:

1. Select an individual unit, squad, vehicle, or group.
2. Show identity, health/readiness, important effects, and commands valid for the full selection.
3. Issue move, attack, attack-move, stop/hold, guard, capture/interact, or a unit-specific ability using contextual click, a button, or hotkey.
4. Confirm the order in-world and in the HUD; communicate failure if the order is invalid, blocked, out of range, or interrupted.

Support selection boxes, modifier-based add/remove, and numbered control groups. Mixed selections should present common commands clearly and avoid silently applying a command to only a subset. A targeted ability/order must have visible target mode and a reliable cancel action.

## 6. Supporting screens and information

- Skirmish setup: map, sides/factions if applicable, teams, AI difficulty, match rules, and start. No network lobby or account prompt for local play.
- Pause menu: resume, save/load where allowed, controls, options, restart, abandon mission, and quit. It must clearly communicate whether simulation is paused.
- Options: display/resolution and UI scale; text scale; graphics; audio and voice/subtitle settings; key remapping; camera and selection behavior; accessibility choices.
- Map/intel view: legend, objective status, and known operational information. It should preserve the distinction between current vision, radar contact, and stale intelligence if those mechanics exist.
- Post-match/debrief: result, objective status, concise performance information, and next steps. Do not require account-bound progression or online rewards.
- Tutorial/help: explain selection, camera, orders, objectives, and any distinctive POLITIKA mechanic in-context; allow replay from a local help screen.

## 7. Presentation and accessibility

Use a restrained military-document palette, clear faction/unit iconography, disciplined typography, and strong visual hierarchy. Reserve distressed paper, stamps, grain, or map-grid decoration for backgrounds and transitions; never put important labels on noisy surfaces without a solid backing.

The interface must not convey ownership, danger, or order state by color alone. Pair colors with icons, shape, labels, patterns, or animation. Provide adjustable text/UI scale, high-contrast option, subtitle controls, and remappable bindings. Keep cursor and selection feedback visible against both bright snow/sky and dark terrain.

At desktop 4K, scale UI in logical units and preserve ergonomic text/icon sizes; do not simply anchor tiny 1080p controls to the corners. Validate 1920×1080, 2560×1440, 3840×2160 and ultrawide desktop layouts, and separately validate Steam Deck-class handheld and representative Android portrait/landscape constraints (with landscape-first gameplay if validated). Respect mobile system insets, device pixel density, touch occlusion and handheld viewing distance. Keep the tactical camera/world scale independent from HUD scale.

## 8. Technical ownership in UE

Treat UI as a presentation layer over the game state:

- C++ match/simulation systems own units, orders, objectives, visibility, resources and match results.
- PlayerController/input actions translate keyboard/mouse, handheld controller/Steam Input and Android touch actions into the same player command intents; gameplay remains independent of the originating device.
- UMG/Slate presents selection, objectives, resources, menus, map and notifications. CommonUI is a conditional choice for layered modal menus and cross-platform focus, not a mandatory framework for the tactical HUD.
- UI reacts to state events and view models; it must not become the authority for gameplay rules or poll every unit every frame.
- Local save/settings services handle progress and configuration without requiring a remote identity.
- A shared command/action definition should drive both button availability and hotkey execution to prevent inconsistent behavior.

This is the proposed POLITIKA presentation/simulation boundary. See [ENGINE_OUTLINE.md](ENGINE_OUTLINE.md) and [PROPOSED_ENGINE_LAYERS.md](PROPOSED_ENGINE_LAYERS.md) for related architectural decisions.

## 9. UI implementation baseline and staged slices

**Verified baseline (27 October 2026):** the [First RTS Vertical Slice](RTS_VERTICAL_SLICE_MILESTONE.md) has scoped Linux Development package acceptance for a camera, single-unit selection, move/attack (and related guard/stop) orders, two visible archetypes, health/death, command feedback, a read-only objective/selection HUD, and a default main-menu route. Its rendered checks include 3840×2160 keyboard/mouse interaction and selected negative-command/objective cases. This evidence **does not** establish a complete minimap, multi-selection, control groups, production HUD, fog-of-war, native UMG/CommonUI implementation, working campaign save/load, controller/touch gameplay, or Windows/SteamOS/Android qualification. The existing shell/early tactical UI may remain Slate/C++ until a measured migration is justified.

**HUD implementation coordination:** The [dedicated HUD contract](RTS_HUD_ARCHITECTURE.md#8-dependency-ordered-hud-roadmap-new-work-packages-not-established-milestone-names) proposes read-only and command-card parity before advanced panels. These are cross-cutting *design work packages*, not new accepted engine milestones or retroactive changes to Stage 4F/4C-G scope.

**Current roadmap:** Stage 3 content/map validation is accepted within its recorded Linux scope. The next product goal is the [First Playable Skirmish](ENGINE_OUTLINE.md#75-stage-4--first-playable-skirmish-repeatable-offline-match); the tactical shell does **not** yet expose an integrated economy, production or AI match. UI implementation must follow the corresponding authoritative gameplay increment and remain a projection/intent layer.

**Minimum First Playable Skirmish controls and feedback:**

- Multi-unit selection and drag-selection, with clear membership feedback.
- Multi-unit movement and attack requests routed through existing command admission; rejected commands must explain actionable causes without leaking hidden information.
- Distinct friendly, hostile and neutral/system ownership cues, not color alone.
- Selected-unit identity/status/current order and feedback when orders are accepted, rejected, interrupted or completed.
- Resource balance and collection/delivery feedback; production queue, affordability, progress/completion and cancellation only where authoritative rules support it.
- Objective and victory/defeat condition status, terminal result/debrief, return-to-menu and rematch navigation.

**Later refinements, not first-match blockers:** advanced minimap interaction, sophisticated command queues, formation previews, adaptive tactical overlays, special-ability panels, full intelligence dossiers and CommonUI migration. **Persistent numbered control groups are separately required for core production usability under 4C-G, though they are not a retroactive 4F first-match blocker.** Preserve the current shell/main-menu and Slate/C++ presentation unless a measured need warrants an incremental change. Linux/Windows desktop input is the immediate focus; SteamOS/Steam Deck and Android remain planned until their interactions are built and tested on actual devices.

**Acceptance principle:** rendered package interaction must establish that the player can understand selection, ownership, orders, economy, production, objective state and result—not merely that simulation tests passed. The player gesture is translated to semantic intent; shared command admission and authoritative C++ state decide its effect; events/snapshots drive UI feedback. Input device specifics must not enter core simulation rules. Proposed screens elsewhere in this document remain future requirements unless explicitly marked implemented above.

## 10. Decisions still open

- Final economy, production, reinforcement and unit-cap model.
- **Multiplayer is planned and prioritized now**; decide only the scope of first LAN/direct-address lobby and online/provider features when MP-1/MP-3 tests can support them. Preserve offline single-player access as a requirement.
- Which unit categories are individually controlled versus grouped as squads.
- The exact campaign briefing/debrief presentation and radio-dialogue cadence.
- Supported display aspect ratios, per-platform minimum hardware, mobile/handheld orientation and the final accessibility/settings list.
- Whether the strategic map is always visible, collapsible, or switchable to a larger planning view.

These should be decided through interaction prototypes. The present document defines the UI's product posture, not final screen mockups or locked bindings.

## 11. Multi-platform command and HUD profiles

The first planned platforms are Windows, Linux, SteamOS/Steam Deck and Android. Preserve the same underlying order vocabulary and information integrity across them, while allowing purpose-built presentation and control mappings. These are product specifications for future prototypes, not evidence of shipped ports. See [Platform Support and Portability Plan](PLATFORM_SUPPORT.md).

- **Desktop (Linux/Windows):** pointer-precise unit selection, multi-select, drag selection, context orders and keyboard shortcuts; a scalable 4K-capable HUD.
- **Handheld (SteamOS/Steam Deck):** gamepad-friendly focus paths, radial or context command menus where prototyping supports them, scalable overlays, legible mission text and selection without requiring a hardware mouse. Steam Input mapping and optional trackpad/touchscreen behavior should be tested on actual handheld hardware.
- **Android:** tap/drag selection and camera movement without relying on hover/right-click; explicit target-confirm and cancel affordances, large enough touch controls, UI safe areas and a compressed or collapsible information hierarchy. Evaluate multi-touch, pause/slow-time accessibility and optional gamepad support through prototypes, not assumptions.
- **Consistency:** all adaptations generate the same authoritative selection/order intents. Gameplay balance, fog-of-war visibility, enemy information and victory rules must not depend on controller type. Validate accidental orders, camera gestures, occlusion, readability and tutorial discoverability for each device.

Consoles (PlayStation, Xbox, Nintendo) and Apple systems (macOS and possibly iOS/iPadOS) may be assessed later. Do not claim controller or mobile UX parity, platform certification or first-release commitments until validated.

## 12. Interface contract derived from the RTS ontology

The [RTS Ontology](RTS_GAME_ONTOLOGY.md#24-player-interaction-is-a-control-interface-not-a-world-ontology) clarifies that the UI is a *control and observation interface*, not an additional collection of world-state rules. Every on-screen element must either: **present an authorized observation**, **express a possible player intent**, **show the admission/execution result of an intent**, or **organize non-gameplay navigation and settings**. A decorative element may provide atmosphere, but must never be mistaken for a verified tactical report.

### 12.1 Ontology-to-UI traceability

| Underlying concern | What the player needs to distinguish | Owning surface | Non-negotiable rule |
|---|---|---|---|
| Definitions versus instances | Archetype and capabilities versus one unit's identity, damage and orders | Selected-unit card; roster tooltip | A changed display name/portrait cannot change stable gameplay identity |
| Faction versus owner/controller | Unit's origin, present permitted commander, political affiliation and hostility | Inspection; allegiance badge; command eligibility | Owning a faction, hiring an asset and commanding an individual unit are different rights |
| Match and world | Scenario, phase, timer, participant slots and terminal result | Setup, HUD objective summary, pause, results | A visual win banner is emitted **after** the authoritative result |
| Space and movement class | Passable ground, legal destination, route uncertainty and camera bounds | Terrain cursor, path preview, minimap | A visual route or plausible map position cannot authorize an illegal movement-class order |
| State and operability | Health, cooldown, order, status, facility service and availability | Selection, ability, facility and production views | Owning a factory does not imply it can operate or produce |
| Resources and processes | Stock, flow, cost, reserved budget, capacity, progress, failure/cancel | Resource rail; production queue; detail panel | Funds are not deducted by a button animation; completion is not the same as a purchase request |
| Commands and events | Requested, admitted/rejected, executing, interrupted, completed | Command card, feedback, event history | An input gesture is not proof the action happened |
| Information and claims | Currently seen truth, last observation, report allegation, unknown and disproved | Battlefield intel markers; minimap; dossier | Neither a rumor nor a fabricated report may generate a real revealed entity |
| Objectives and provenance | What exact state/event qualifies and what remains unknown | Objective list, progress, debrief | Own-producing a bomb is not equivalent to possessing or stealing it |

This matrix is a **UI acceptance taxonomy**, not a proposal to build one class per row. Refer back to the [minimal gameplay contract](ENGINE_OUTLINE.md#minimal-rts-gameplay-contract) when questions of state authority arise.

### 12.2 One canonical interaction pipeline

```text
device gesture -> semantic local action -> selection + target-mode state
               -> typed command intent (issuer, selected instance IDs, target, modifiers)
               -> authoritative command admission
               -> accepted / rejected / limited admission feedback
               -> simulation process and state events
               -> participant-authorized observation projection
               -> HUD, in-world feedback, notification and event history
```

A command preview represents the *intended* operation and its locally knowable restrictions, not a promise of acceptance. The final check always occurs in the current authoritative rules context. An accepted order can still become interrupted, delayed, out of range or impossible during execution; the UI must distinguish those outcomes. The HUD must not implement damage, capture, purchase, ownership conversion, construction or victory as independent widget-side side effects.

Use a small common response vocabulary for all device profiles: **ready**, **requires target**, **provisional preview**, **requested**, **accepted**, **rejected**, **in progress**, **interrupted**, **completed** and **cancelled**. Do not present "successful" until the required simulation event exists. A command rejected for a hidden target must use a permission-safe generic reason rather than identifying a concealed unit or facility.

### 12.3 Player observation, not omniscient game state

Propose a *participant-specific presentation snapshot* containing only the fields permitted for that participant at that time. It should distinguish:

- **Authoritative own/authorized assets:** current controllable health, status, orders and supported ability details, subject to any communications or gameplay restrictions.
- **Currently observed foreign entities:** fields visible under reconnaissance, detection, diplomacy and mission rules; visual contact need not grant full statistics or classified content.
- **Remembered contacts:** position and last-known type/status **at observation time**, with timestamp/age and no implied ongoing real-time tracking.
- **Intelligence claims:** a report alleging a fact, with source *as represented to the player*, confidence, report timestamp, verification state and permitted attribution. A purported source can be false.
- **Unknown/hidden:** no data beyond public or legitimately inferred scenario knowledge; disabled HUD widgets, minimap overlays, hovering and diagnostics must not leak it.
- **Disproved/retracted:** retained in an intelligence history if useful, but explicitly no longer presented as live enemy presence.

Prohibit UI code from grabbing unfiltered lists of enemy Actors or querying hidden objects merely to decide whether an ability button or cursor should be green. An inaccessible object's existence must not be detectable through hover hit testing, different rejection messages, minimap bounds, sound cues or a fluctuating resource readout. Distinguish **player-known** from **world-true** even when both happen to agree.

## 13. Tactical screen information architecture

### 13.1 Glance, inspection and investigation

The battlefield is the main work surface. Organize content into **three levels of attention** rather than filling the screen with every possible statistic:

1. **Persistent, glanceable:** selected-force headline, available direct orders, current primary objective, urgent alerts and only currently actionable strategic stocks/queues. Units and the strategic map remain the dominant visual signals.
2. **On-demand inspection:** detailed per-unit and mixed-force comparison, weapon and movement capabilities, formation modifiers, production prerequisites, facility operability, resource throughput and richer objective descriptions.
3. **Deliberate investigation/planning:** full map, intelligence claims and provenance, diplomatic/political developments, archival campaign documents, faction technology, Black Market and multi-step intelligence workflows—only once implemented.

An unusual strategic emergency may elevate a lower-level datum temporarily (for example, power outage, nuclear-threat warning *as perceived*, mission deadline or suspected false-flag allegation) without making an entire investigation panel permanent.

### 13.2 Desktop baseline HUD arrangement

This is a *functional layout hypothesis*, not locked pixel coordinates or a reproduction of any C&C/StarCraft interface:

```text
+---------------------------------------------------------------------+
| mission/phase  | objective + alert strip                | compact stocks |
|---------------------------------------------------------------------|
|                                                                     |
|                  BATTLEFIELD / PRIMARY TARGET AREA                  |
|  contextual world markers, selection, path/target previews          |
|                                                                     |
|  [optional urgent alert]                                            |
|                                                                     |
|-----------------------+----------------------+----------------------|
| strategic map         | selection / group    | orders / abilities   |
| + viewport footprint  | portrait + state     | target/cost/ready    |
| + intel legend        | aggregate/details    | production if active |
+-----------------------+----------------------+----------------------+
```

The design intentionally groups *strategic orientation* (map), *what is selected* (identity), and *what can be done* (orders) while minimizing pointer travel. Its compact desktop bottom-band form should be tested against a more transparent edge-anchored variant with larger battlefield visibility; the winning version should be chosen through usability evidence, not genre nostalgia. At lower vertical resolutions, convert secondary information to collapsible drawers instead of shrinking labels beyond legibility.

| HUD zone | Always visible when implemented | Expandable/conditional content | Main failure to prevent |
|---|---|---|---|
| Main viewport | World, selection outline, active target state, tactical order acknowledgement | Route/formation/area preview, warnings, interaction ranges | Battlefield concealed by large permanent panels |
| Strategic map | Friendly/authorized entities, currently allowed information, viewport footprint, important objectives | Layers, legends, filters, ping history, claim age, full planning mode | Hidden truth leaking through minimap actors or unexplained enemy dots |
| Selection band | Count/type, identity, grouped status, order headline and key health/resolve indicators | Detailed health distribution, member list, formation, permitted cargo/worker details | One representative's health masquerading as a mixed group's health |
| Context command card | Most relevant legal or potentially legal orders, target mode and cancellation; ability cooldown | Prerequisites, cost breakdown, queue, technology reasons | Greyed-out unexplained controls or buttons that bypass order validation |
| Objective/status rail | Current primary objective, nearest deadline/contested state, essential progress | Secondary objectives, mission log, narrative context | Objective completing from an animation rather than match state |
| Resources and production | Implemented actionable stock/capacity and active processes only | Full refining chains, allocation, energy grid, factories, auctions | Presenting design-only resources as if an actual economy exists |
| Notification layer | New urgent event and acknowledgement; replayable event log | Filters, map-focus, pause/review | Flood that masks world clicks or exposes covert attacker identity |

**Spatial policy:** fixed HUD regions should reserve touch/click input only within their actual interactive shapes; transparent decoration must not swallow battlefield input. Selection rectangle, minimap clicks, scroll/pan and tooltip hit regions need deterministic priority. World indicators (range, move target, damage, detection) should scale and declutter across zoom, use predictable z-order/occlusion, and be hidden when the observer is not entitled to their meaning.

### 13.3 Minimap as a separate projection of knowledge

The strategic map is not a downsized unrestricted world render. Derive its transform from validated playable bounds and camera orientation, not hardcoded pixel assumptions or archival-map art. Allow camera focus/pan and, when the command grammar allows, orders through the minimap using the *same* authorization and terrain-target rules as the main battlefield. Provide a legend for friendly/ally, observed hostile, neutral/Unaligned, objective, and reported/remembered markers. A reported nuclear site should use a distinct glyph or outline from an observed real structure. Preserve contact age and uncertainty when zooming between tactical view, minimap and full planning map. Icon clustering should not erase a critical objective or invent precision about uncertain contacts.

For rivers, mountain passes, bridges, borders, industrial zones and road/rail networks, a map overlay may show **known** infrastructure and navigation classes. Historical map provenance is an authoring matter in the [Archive](../Archive/README.md) and [Production Standards](PRODUCTION_STANDARDS.md); it must never determine gameplay visibility by itself.

## 14. Selection, orders and interaction grammar

### 14.1 Selection invariants

Selection is local player intent over currently visible/eligible entities, not an ownership transfer and not a match rule. The future implementation should support single selection, marquee/box selection, additive/subtractive modifiers, select-similar/type, double-select when appropriate, cycling, and remembered numbered groups. The existing prototype only establishes single-unit selection. Before broadening it:

- Define **owned/controllable**, **friendly but not controllable**, **allied under contract**, **neutral Unaligned** and **hostile/unknown** selection behavior. Inspecting a non-controllable public or discovered asset must never expose its owner-only commands.
- Keep a stable selected *instance-ID set*, not widget indices. Remove dead, transferred, no-longer-authorized or genuinely undiscoverable instances according to explicit selection policy; a recycled pointer must not select the next spawned entity by accident.
- For a mixed group show count, role composition, actionable common orders and distribution of health/resolve; use "mixed / n of m eligible" rather than a misleading uniform value. Provide optional subgroup tabs or filters when complexity warrants them.
- Expose the difference between **common to all**, **available to an eligible subset**, **temporarily unavailable**, and **not possessed** as separate command-card states. Do not silently send partial orders. If an explicitly requested subset action is supported, preview and acknowledge affected/omitted counts and allow confirmation where risk warrants it.
- Numbered control groups store eligible instance sets, support assign/recall/focus and handle death/changed controller gracefully. Never automatically grant control after faction conversion without a new authority check.

Selection presentation should preserve map readability: clear rings/shape outlines, a subtle group bounding cue, category/ownership symbol and hover feedback. Selection effect animation is not confirmation of an admitted gameplay order.

### 14.1a. Persistent control-group bar and selection gestures (proposed core production interaction)

The numbered **control-group bar** is a compact discoverable alternative to Ctrl/Shift/Alt+0–9. Each clickable slot must show its number (0–9), current eligible/total count where meaningful, active selection, a distinct empty/missing indicator and a bound-key hint from the live input registry. Click recalls; an explicitly identified focus action frames units without issuing orders. Do not show a formation icon unless that group references real formed units; selecting an overlapping group **never** creates a formation or affects Discipline. Slot assignment is a local input operation, not a privileged gameplay call. On screen layouts with insufficient horizontal room, collapse or scroll the bar while preserving accessible slot selection (do not cover tactical play).

Mouse/keyboard grammar: LMB single/clear, LMB-drag rectangle after a **DPI-aware** threshold, Shift+click toggle, Shift+drag add, Ctrl+click/double-click visible same archetype, Ctrl+drag subtract; RMB context order and MMB pan/wheel zoom remain distinct. Ctrl+0–9 replaces a slot, Shift+0–9 **adds without removing other associations**, Alt+0–9 **transfers selected units exclusively**, number recalls, rapid repeated number focuses, and Tab cycles selected archetype/ability subgroups. All are **proposed** and must be remappable. The actual prototype currently has no marquee/numbered group interface and additionally binds **E Grenade, R Reload, F Sprint**; F must not be reused for camera focus. A and S currently conflict with WASD camera axes, and must be remapped/tested before this expanded grammar is accepted.

Visual interaction is a state machine, not a sequence of independent click callbacks: one gesture is either click, rectangle, camera pan, UI capture, or cancelled targeting. Project authored unit selection proxies (not just Actor positions) into screen space; apply visibility/selection-authority filters and one normalized ID transition. A click on minimap/UI must not leak into world orders. Cancel, focus loss, phase change and rematch release held gesture/modifier state. On mixed selections, the command card must show common versus subset-valid actions and per-member accepted/rejected outcomes.

[Selection and Group Orders](RTS_SELECTION_AND_GROUP_ORDERS.md) owns normative membership and order admission; [Comparative Selection/Input Research](RTS_CONTROL_GROUP_INPUT_RESEARCH.md) owns precedent; [Practical UI Keymap](RTS_UI_SETTINGS_AND_KEYMAP.md) owns physical bindings; [Engine Outline](ENGINE_OUTLINE.md) schedules this as **4C-G**, the mandatory core-production follow-on to 4C, without changing the historic 4F First Playable Skirmish exit gate.

### 14.1b. Tactical formation controls (future behavior)

[Military Formations, Tactical Stances, Cohesion and Discipline](RTS_UNIT_FORMATIONS_AND_DISCIPLINE.md) specifies explicitly created tactical formations, separate from local multi-select or control-group hotkeys. For an eligible group, present a contextual **Form** operation and shape, facing, stance and adherence as separate settings; an existing formation may remain intact while only some members are selected. Show role/slot overrides, reinforcement/detach, valid destination previews, partial member rejection, route failure and organizational status (Assembling/Deployed/Degraded/Fragmented/Dissolved). Inspect individual combat Discipline using the canonical resolve value; never imply a second Morale meter from derived Cohesion. UI cannot teleport units or reveal hidden enemies and must not claim formation capability is currently shipped.


### 14.2 Order families and targeting states

| Family | Typical target | Required UI behavior | Failure/interrupt distinction |
|---|---|---|---|
| Move / attack-move | Legal ground or route point | Ground marker, queued waypoint preview where supported, movement-class hint | Invalid surface, inaccessible route, later route obstruction |
| Attack / guard / hold / stop | Observed hostile actor, area or local stance | Target cursor and admitted stance/order; friendly/neutral permissions respected | Invalid/hidden target, cooldown, target death, attacker death |
| Interact / capture / secure | Declared site/region/objective | Radius/contest and progress only when observable, qualifying unit shown | Not eligible, contested, paused, destroyed site |
| Produce / deploy / reinforce | Eligible queue, producer or legal placement | True cost, reservation, time, capacity and cancellation terms | Insufficient resources, missing prerequisite, inactive building |
| Recon / intel / inspect | Region, sensed contact or report | Scanner/vision scope with information-class label and timestamps | Range/detection restrictions, old or disproved report |
| Sabotage / infiltration / ability | Legal actor/site/area selected by an eligible unit | Availability reason, targeting mode, cost, commitment and delay | Failed admission, detected action, interrupted process, completed effect |
| Mercenary / contract / market | Offer, contract target, permitted hired asset | Price, ownership-versus-command rights, expiry/cancellation | Unavailable offer, expired permission, rival purchase |
| Propaganda / Psy-Ops / Intimidation | Eligible media region, facility or unit | Faction-specific unlock and target/resolve/knowledge criteria | Wrong faction, unsupported target, insufficient intel, immunity |

Canonical **interaction states** are idle → selecting → optional targeting/preview → submitted → accepted/rejected → executing → completed/interrupted. Preview overlays must clear on cancel, deselection, modal opening, ownership loss, invalidated target and match end. Escape/Back should cancel the deepest temporary state first, then close an overlay, and only subsequently open pause when no more local target state needs cancellation; verify these rules for each device input mapping.

**Queue modifiers:** when order queuing is implemented, distinguish an order appended to a queue from an immediate replacement. The queue UI should expose only actually admitted steps and explain admission failure without pretending later steps executed. Queues require an explicit policy for path interruption, target death, ownership change and loaded-save restoration.

### 14.3 Feedback semantics and attention control

Acknowledge an order in both a low-clutter world cue and a small semantic status channel: *requested* may have a temporary cursor/ping, *accepted* an order marker, *rejected* a short accessible reason, *executing* a status indicator, *interrupted* a distinct notification. Use audio as redundant reinforcement, not the only channel. Avoid modal confirmations for ordinary frequent move/attack orders; reserve confirmation for irreversible or high-cost actions, explicitly destructive actions, unambiguous mixed-selection subset operations, and abandoning/restarting a match. Do not allow generic action sounds to imply a successful infiltration or production job before the simulation confirms it.

Notification arbitration should prioritize immediate player harm or mission-critical state over routine successes. Coalesce identical low-priority damage alerts during high-entity battles, keep a scrollable local incident history with timestamp/map focus where permitted, provide alert-volume controls, and avoid repeated sounds overwhelming tactical attention. Contextual map pings must carry the same epistemic class as their originating event.

## 15. Epistemic UX: fog, intelligence, fabricated reports and deception

POLITIKA's unusual advantage over a conventional RTS is the distinction between *world truth* and what a faction believes about it. A trustworthy interface must be honest about the **status of evidence** without always knowing or revealing the truth. Apply this principle across tactical HUD, minimap, mission dossier, espionage buttons, notification wording and post-match review.

### 15.1 Intelligence presentation types

| Information kind | Proposed visual grammar | Detail panel should disclose | Must not imply |
|---|---|---|---|
| Live optical/sensor sighting | Solid current-contact marker with time-sensitive detection qualifier | Observed identity/type/position and sensor context actually known | Every hidden statistic or permanent tracking |
| Last-known sighting | Muted/ghosted outline; explicit last-seen time | Recorded location, age, uncertainty and possible movement | That the unit remains at its old coordinate |
| Intercepted communication | Signal/report icon, alleged author and capture time | Claim text and disclosed source/classification | Cryptographically verified origin or physically observed facility |
| Anonymous or allegedly official report | Distinct document/rumor marker, uncertainty badge | Claimed fact, confidence *as estimated by recipient*, report history | That a reported source is truthful |
| Corroborated in-world evidence | Verified-evidence state with traceable local sources | What was corroborated and by which accessible steps | Universal knowledge outside recipients or final objective completion without a qualifying event |
| Disputed/disproved claim | Struck/retracted history entry; remove live-looking dot | Contradiction, correction time and remaining uncertainty | That absence of one sighting proves an entire region clear |
| Unknown / fully hidden | No targetable gameplay entity marker | No secret type, cost, threat or facility data | That a locked action diagnoses an unseen building's existence |

**Confidence is not truth.** A confident fabricated report should remain a confident *claim* until disproved; a low-confidence report could happen to be accurate. A detector that finds no facility at a claimed coordinate can update the belief/history without removing a facility that was never there. The same authored message may present different details to different players based on who has access to the corroboration.

### 15.2 Concrete adversarial flows

**Fabricated nuclear site:** an Unaligned-origin report purports to be enemy radio intelligence alleging that the Directorate has a nuclear facility. The recipient's feed displays an *unverified alleged location* with report age and attributed source; the main map does not spawn a selectable Directorate nuclear plant. A recon flight finds an empty site and the claim becomes disputed or disproved according to verification policy; it does not spawn/destroy any game entity. Other players neither gain the claim nor see the verification unless it is legitimately shared.

**Covert budget diversion:** when an actual Directorate resource theft succeeds, the defender may see lower stocks or reduced production but not an enemy spy's identity; after a separately simulated detection/investigation event, the incident dossier may reveal attribution. The attacker sees its authorized outcome and credited stocks. Identical public resource deltas need not produce identical privileged alerts for both participants.

**Neutral authority:** Unaligned patrols and institutions display as observed NPC actors with their own affiliation. The player's inability to issue commands to them must not depend on whether the Unaligned secretly knows the assassination story. The political interface may surface actual publicly known jurisdiction/mandate, but not internal hidden stability calculations, classified surveillance or conspiracy participation.

**Nuclear provenance:** a stolen bomb icon may identify possession/custody if legitimately known; it must not grant a "produced our first bomb" victory indicator. For Ascendancy, the win panel must depend on the authoritative completion of its own qualified production event. The full design remains governed by [Nuclear Weapons](NUCLEAR_WEAPONS_SYSTEM.md).

### 15.3 Intelligence and visibility testing

Test the same fixture from **recipient, uninformed rival, genuine observer and privileged debug** viewpoints. The first three must differ only according to authorized information; development overlays may expose truth only behind explicit non-shipping controls. Negative assertions include: hovering over fog cannot discover a secret building; a false marker cannot be attack-targeted as a real Actor; a rejected command cannot reveal hidden armor/health; an unaligned false report cannot satisfy a research/victory prerequisite; an expired contract cannot leave control buttons enabled; minimap and dossier cannot disagree about whether a contact is live or reported. Screenshot testing alone is insufficient—assert the underlying projection records and emitted commands.

## 16. Strategic and faction-specific surfaces

These UI extensions are **conditional on the corresponding implemented rule**; Sections 4 and 5 already define their narrative and faction contracts. Do not construct simulated dashboards for mechanics that are still design proposals.

### 16.1 Economy, infrastructure and deployment

Show each actionable resource with a type/unit, current available stock, any reserved amount, production/consumption rate *if observable*, capacity when meaningful and an explicit source for negative balances or service interruptions. Keep oil and gas deposits distinct from refined petroleum/kerosene; distinguish raw versus refined nuclear material, energy-generation versus operability and ownership versus functional throughput. A breakdown drawer can show where an input was produced, refined, transferred, spent or stolen only to the level available to that player. Avoid reducing all shortages to one universal "credits" meter if the game's resource rules are more specific.

Production/reinforcement controls should state the specific source/producer, recipe prerequisites, build time, queue position, reservation/cancellation policy, blocked state and legal deployment destination. Distinguish *can afford*, *is permitted*, *has required infrastructure*, *producer is operational*, *destination is legal* and *currently in progress*. A disabled card needs one or more actionable reason phrases, not a generic red tint.

### 16.2 Faction-specific command and policy grammar

| Faction | Distinct UI burden | Hard rule |
|---|---|---|
| Atlantic Concord | Precision/scouting, cheap aircraft reconnaissance, intelligence/Psy-Ops, cheaper oil refining and nuclear bomber readiness | An unarmed scout aircraft cannot receive an attack action; Psy-Ops may affect unit resolve or eligible buildings but is not an Intimidation capture |
| Red Directorate | Mass formation/Discipline, rifle-motorcycle reconnaissance, intelligence theft cost/time advantage, industrial/nuclear ICBM processes | Do not portray economy theft as a standard combat hit or claim a nuclear launch without actual launch rules |
| Iron Ascendancy | Spy progression, eligible isolated-unit Intimidation, espionage gate for research, production-dependent first-bomb win | Only this faction can issue Intimidation; break threshold is not conversion until a separate permitted action succeeds |
| Meridian Pact | Truck reconnaissance, early conventional sabotage, mercenary contract state, Black Market, fuel-field opening constraint | Trading/contract rights do not equal faction ownership; no advanced propaganda action tree or nuclear refining |
| The Unaligned (NPC) | Seen patrols, institutions, public alerts, contested authority, occasional reports and verification | Never show a playable Unaligned build/command panel or reveal internal balance director |

Population Morale (faction-wide) and unit Discipline/combat versus civilian Morale (same per-unit resolve field with contextual labels) must never be collapsed into one UI variable. For mass-selection, aggregate military Discipline as a distribution/low-watermark where useful; civilian Morale and faction Population Morale stay separately labeled.

### 16.3 Market, missions and political boundaries

The Black Market is an explicitly authored transaction view, not a shortcut for purchasing units whose rules forbid acquisition. Display seller/offer *as publicly known*, price, eligible buyers, uniqueness/stock, auction state, ownership versus command authorization and transaction verification. Nuclear weapons cannot become Black Market listings. A mercenary's expiration should withdraw the customer's buttons and update eligible selection without changing the underlying archetype's origin.

Mission objectives need event provenance: *capture*, *hold for duration*, *protect*, *extract*, *intercept*, *verify evidence*, *produce* and *survive* are distinct conditions. The panel must show the actual qualifying predicates or a spoiler-safe player description, the progress policy (reset/pause/persist), eligibility and failure state. A fictional assassination clue should not be described as historically verified evidence; historical source approval remains a production task, not part of the fictional intelligence system.

## 17. Front-end, focus hierarchy and navigation flows

### 17.1 Screen/state map

```text
Application shell
  Main menu
    Continue -> confirmed compatible local campaign save -> briefing/mission
    Campaign -> mission selection -> dossier/briefing -> gameplay
    Skirmish -> validated map + 2–4 faction slots (+ Unaligned NPC) -> gameplay
    Settings -> display/audio/controls/interface/accessibility -> return
    Help / credits / quit
Gameplay
  Tactical HUD + non-blocking notifications
    Optional inspection drawer / strategic-map expansion / intel dossier
    Command targeting preview -> confirm/cancel -> tactical HUD
    Pause (blocks world commands)
      resume / settings / save where valid / restart / exit
    Terminal result -> debrief -> next action
Error/recovery
  missing/unsupported save, invalid content, failed map admission, settings rollback
  -> actionable explanation -> recover to shell (never partial match)
```

Maintain a focus/back stack that restores the prior meaningful control and does not allow a gameplay click through a transparent modal. Modal transitions must release target modes and clear held/queued input according to the canonical input policy. A pause screen must communicate whether simulation time is stopped (local offline match) or continuing (possible later online match); do not promise online pause semantics before networking design exists.

The skirmish setup should validate distinct playable factions, supported slot count (2–4), map legal starts and Unaligned auto-injection before enabling Start. Treat scenario-specific Meridian fuel access and special infiltration preconditions as checks according to the selected gameplay/map profile, not as conditions for the Stage 3 test fixture. If an invalid map cannot be started, show a useful localized reason without disclosing hidden campaign spoilers.

### 17.2 Narrative without cinematics

Campaign briefing, radio captions and debrief must support text-first consumption, transcript history, skip/review, neutral map annotations and user-adjustable dialogue pace. Use audiovisual layering for atmosphere—archival paper/typography, static dossier maps, stamps and broadcast indicators—but do not obstruct urgent tactical orders with mandatory spoken dialogue. A brief can withhold the game's alternate-history secrets while remaining mechanically honest about known objectives. Later transcript/dossier navigation should avoid autoplay media that conceals unsent commands or forces an unwanted pause.

## 18. Adaptive device interaction and accessibility specification

### 18.1 Shared semantic actions, distinct device grammars

| Player intent | Linux/Windows mouse + keyboard | Desktop/Steam Deck controller | Android touch-first |
|---|---|---|---|
| Camera pan/zoom | Edge or keyboard pan; wheel/keys | Stick/trackpad pan; explicit zoom controls | Drag empty ground to pan; pinch to zoom; avoid accidental unit orders |
| Select single / inspect | Click, modifier inspect | Cursor/reticle, focus/select confirmation | Tap entity; tap/hold for details where tested |
| Multi-select / control group | Drag rectangle; modifiers; number groups | Selection modifier/radial and group list | Multi-select mode or drag selection that cannot conflict with camera pan |
| Context order | Context click with safe inference | Explicit command/target selection and confirm | Context toolbar + target tap/confirmation |
| Targeted ability | Button/hotkey + preview + target | Action wheel/card + reticle + cancel | Large ability card + preview + target + cancel |
| Minimap/focus | Click/drag viewport footprint | Focusable map with accessible legend | Expandable map so finger does not occlude tactical area |
| Cancel / back / pause | Escape and configured commands | Dedicated Back/Cancel; pause | Visible Back/Cancel and pause reachable without hover |
| Rebinding/help | Keyboard/mouse mapping panel | Gamepad mapping + device prompts | Gesture tutorial and settings appropriate to touch |

Do not pretend physical gestures are semantically equivalent just because they emit the same event ID; test accidental taps, pointer travel, target disambiguation and the full select → command → feedback loop separately for each profile. Avoid a mandatory synthetic mouse cursor for controller RTS gameplay unless a prototype establishes that it is quicker and clearer than focus/reticle controls. Controller support is a planned feature for both Linux and Windows, not only the Deck; Android may additionally support controllers.

### 18.2 Responsive layouts and legibility

- **Scale model:** use UE DPI scaling, logical size constraints, anchors and safe-area widgets; keep world camera zoom independent of HUD scale. Provide separately adjustable interface scale and text/subtitle scale with recovery if a selection makes controls inaccessible. Avoid coupling HUD text size to 3D screen percentage.
- **Desktop checkpoints:** test at 1920×1080, 2560×1440, 3840×2160, at least one ultrawide aspect ratio and window resizing. Ensure selection, production, minimap, tooltip and objective rails do not collide as labels expand.
- **Handheld/mobile checkpoints:** design a compact mode with contextual bottom drawer, easily focused actions, touch-safe minimap expansion, appropriate system insets and a non-hover information path. Validate at representative native aspect ratios, DPI/font scales and device orientation; landscape is the proposed Android starting point, not a shipped guarantee.
- **Touch targets:** adopt and test a project-defined minimum comfortable physical target size per device tier rather than equating Unreal logical pixels with millimeters. Keep destructive/match-exit controls separated and require confirmation. Measure reach and finger occlusion on real devices.
- **Contrast:** use WCAG 2.2's 4.5:1 ordinary-text, 3:1 large-text, and 3:1 non-text active-control contrast as useful *design review targets*, not as a claim of certified WCAG compliance or a universal game standard. Audit against real snowy, bright-sky, forest and night maps, including translucent HUD overlays. Pair color with shape/text/pattern.
- **Accessibility:** remappable actions, alternate input paths, clear keyboard/gamepad focus, reduced motion/flashes, subtitle speaker labels, adjustable radio/subtitle timing, notification history, scalable cursor and selection rings, high-contrast/tactical-intel visual modes and color-vision-safe faction distinctions. Provide screen-reader/assistive-text hooks where feasible and test rather than assume accessibility from library presence.
- **Localization:** use localized `FText` rather than raw user-facing IDs or unlocalized string concatenation; allow sentence-length expansion, language-dependent labels, numerical/date formatting and diacritics appropriate to Yugoslav languages. Do not assume compact English hotkey hints or fixed-width Latin glyphs are sufficient.

### 18.3 Theme and visual language

Create a small source-controlled UI style vocabulary before commissioning production screens: typography tiers, spacing/density tokens, HUD panel opacities, cursor/selection outlines, semantic colors *plus* symbols, alert priority, distinct contact/evidence glyphs, focus-ring states, icon size tiers and animation durations. Faction crests/logos remain visually stable under color-scheme changes and should not be substituted with invented icons. Use authentic-period map visual conventions **as a reviewed aesthetic reference**, never as a replacement for accessible contrast. Test readability at both strategic zoom and close combat without borrowing proprietary C&C interface sprites.

## 19. Unreal Engine 5.8 technical implementation standard

Epic's official documentation defines reusable UE UI technology and performance recommendations, **not a mandatory RTS HUD layout standard**. In fact, [CommonUI's design guidance](https://dev.epicgames.com/documentation/en-us/unreal-engine/design-guidelines-for-using-commonui-in-unreal-engine) explicitly notes that a simple PC-only RTS is a case where CommonUI may add unnecessary complexity, while complex multi-layered, cross-platform navigation is a stronger fit. POLITIKA has both battlefield and multi-device menu needs; make the distinction deliberately.

### 19.1 Recommended responsibility boundaries

```text
UE 5.8 Enhanced Input / pointer / touch adapters  ->  local semantic intents
Local PlayerController  ->  selection + targeting + camera state
                       ->  typed command request to match rules
Authoritative C++ simulation -> admitted orders, state changes, events
Participant observation service -> filtered read-only UI snapshot/events
Presentation adapter (C++) -> normalized labels, command affordances, statuses
UMG/Slate battlefield widgets -> HUD and world indicators
Optional CommonUI activatable menus -> modal focus/back/navigation
User settings/local profile -> UI scale, bindings, accessibility, layouts
```

Do **not** add these as mandatory new Unreal modules or a duplicate gameplay database. Keep the prototype's directly functional C++/Slate HUD until a UI component actually needs reuse or sophisticated authoring. Presentation adapters may be ordinary C++ objects or small view models owned by local player context. A controller-facing tool should receive participant-filtered data, never privileged world-state pointers.

| UE feature | Appropriate POLITIKA use | Caution / adoption criterion |
|---|---|---|
| **UMG / Slate** | Main tactical HUD, selection/production cards, minimap presentation, overlays and menus | Avoid tick-driven list scans and hidden simulation writes in widgets |
| **Enhanced Input** | Contextual semantic RTS actions, mapping contexts, player rebinding, device-specific adapters | Registering mappable contexts is not equivalent to activating them; keep modal/world routing explicit |
| **CommonUI** | Candidate for front-end/settings, skirmish setup, pause, nested dossiers and controller focus/back | Not needed for every battlefield widget; prototype before opting into `CommonGameViewportClient` and an input-router migration |
| **UMG Viewmodel (MVVM)** | Optional efficient field-notify projection for reused, complex reactive widgets | Epic documents the feature as **Beta**; evaluate version-specific shipping maturity, correctness and cost; a small manual event adapter is sufficient initially |
| **UI Invalidation / Retainers** | Cache infrequently changing panels; conditional retainer use after measurements | Do not assume every dynamic HUD panel should be invalidated/retained; cache invalidation and texture memory have costs |
| **DPI Scaling / Safe Zones** | Responsive anchors, viewport-dependent scaling, screen insets | Test actual on-device bounds, not only editor simulator |
| **Localized `FText` and asset styles** | Translatable labels, descriptions, alerts and theme consistency | Gameplay IDs/asset paths are not player-visible localized names |
| **Automation, screenshot tools, Slate Insights** | Interaction assertions, editor comparisons, invalidation/update traces | A passing screenshot/editor test cannot certify packaged gameplay or physical devices |

### 19.2 Input routing and focus discipline

Use [Enhanced Input](https://dev.epicgames.com/documentation/en-us/unreal-engine/enhanced-input-in-unreal-engine) action IDs and contexts for gameplay. Present actions such as `RTS.Select`, `RTS.ContextOrder`, `RTS.AttackMove`, `RTS.Stop`, `RTS.Guard`, `Camera.Pan`, `Camera.Zoom` and targeted ability intents independent of physical bindings. Follow the detailed [Engine input contract](ENGINE_OUTLINE.md#input-contract-implementation-notes), including active-context priority, player-mappable settings, conflict resolution, reset/persistence and device prompts.

Use explicit UI routing states: **nonblocking tactical HUD**, **targeting mode**, **modal menu/rebind**, and **passive notification**. Modals must consume UI Accept/Back and prevent world orders behind them; closing a modal restores focus and contexts, clears held-trigger leakage and does not duplicate input listeners. For advanced multi-layer menus, [CommonUI Quickstart](https://dev.epicgames.com/documentation/en-us/unreal-engine/common-ui-quickstart-guide-for-unreal-engine) covers `CommonGameViewportClient`, activatable widgets, Back/Confirm, controller data and focus; this is a *candidate architecture*, not a claim the plugin is already enabled. Do not adopt CommonUI's Enhanced Input bridging merely because both plugins exist—the [integration documentation](https://dev.epicgames.com/documentation/en-us/unreal-engine/using-commonui-with-enhanced-input-in-unreal-engine) carries explicit maturity cautions and must be checked against the pinned 5.8.3 build before relying on it for shipping-critical routing.

### 19.3 Data-flow and update performance

Prefer event/delegate-driven UI refresh for selection changes, observed health/order changes, objective events and known contact updates, not per-frame bound text getters or scans across every Actor. View-model or adapter updates should be keyed by selected instance ID and observation revision; coalesce high-frequency world events into a bounded view update cadence *where the design tolerates latency*, without delaying command acknowledgement or critical warnings. On selection change, unsubscribe stale sources and immediately clear outdated tooltips, targets and bound widgets.

Build command buttons from the same C++ capability/admission descriptions used to check orders, while allowing a conservative *preview* that cannot leak hidden state. An enabled button does not guarantee admission after time has advanced; rejected requests must still have a legitimate result path. Data asset edits may change displayed stats, ranges and costs, but may not authorize ability use independently of the simulation.

Epic's [UMG optimization guidelines](https://dev.epicgames.com/documentation/en-us/unreal-engine/optimization-guidelines-for-umg-in-unreal-engine) recommend reducing unnecessary widget Tick/OnPaint work, avoiding frequently evaluated bound attributes, and using invalidation caching where appropriate. Profile live tactical scenes with [Slate Insights](https://dev.epicgames.com/documentation/en-us/unreal-engine/slate-insights-in-unreal-engine), which identifies widgets being updated, repainted or invalidated; measure HUD cost within the overall RTS frame budget instead of declaring a fixed UI performance budget before target hardware is measured.

### 19.4 Map/world-space rendering and assets

Choose world-space widgets selectively. Distant health bars, floating names, capture markers, target indicators and selection effects should use a stable tested rendering strategy with clustering/LOD and access filtering. CommonUI's own guidelines discourage its use in WidgetComponents for world-space UI: favor simpler UMG/Slate or render primitives for such markers. Never spawn a permanent screen widget per every hidden unit. Keep navigation/minimap transforms independent of the rendering asset's projected image; map markers must be based on known state.

Treat icon atlases, fonts, themes and portraits as content with provenance, source-control rules, localization coverage, fallback policy and validation. Art-only variations cannot change command availability, hitboxes for gameplay targets, faction permissions or authoritative visibility.

### 19.5 Sources and supported inference

Epic sources informing this technical specification (all are general UE guidance rather than bespoke RTS interface standards):

- [CommonUI design guidelines](https://dev.epicgames.com/documentation/en-us/unreal-engine/design-guidelines-for-using-commonui-in-unreal-engine) — when *not* to adopt it for a simple PC RTS, and when layered multi-platform navigation justifies it.
- [Common UI overview](https://dev.epicgames.com/documentation/en-us/unreal-engine/overview-of-advanced-multiplatform-user-interfaces-with-common-ui-for-unreal-engine) and [Quickstart](https://dev.epicgames.com/documentation/en-us/unreal-engine/common-ui-quickstart-guide-for-unreal-engine) — activatable input routing, back actions, controller prompt assets.
- [Enhanced Input](https://dev.epicgames.com/documentation/en-us/unreal-engine/enhanced-input-in-unreal-engine) and [UEnhancedInputUserSettings](https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Plugins/EnhancedInput/UEnhancedInputUserSettings) — semantic contexts, device mappings and rebinding.
- [UMG Viewmodel](https://dev.epicgames.com/documentation/en-us/unreal-engine/umg-viewmodel-for-unreal-engine) — optional push-updated binding layer, labelled Beta.
- [UMG optimization](https://dev.epicgames.com/documentation/en-us/unreal-engine/optimization-guidelines-for-umg-in-unreal-engine) and [Slate Insights](https://dev.epicgames.com/documentation/en-us/unreal-engine/slate-insights-in-unreal-engine) — low-overhead event updates, caching and measurement.
- [UMG Safe Zones](https://dev.epicgames.com/documentation/en-us/unreal-engine/umg-safe-zones-in-unreal-engine), [DPI Scaling](https://dev.epicgames.com/documentation/en-us/unreal-engine/dpi-scaling-in-unreal-engine), [FText](https://dev.epicgames.com/documentation/en-us/unreal-engine/ftext-in-unreal-engine) and [UI Accessibility](https://dev.epicgames.com/documentation/en-us/unreal-engine/designing-ui-for-accessibility-in-unreal-engine) — resilient layouts, localization and accessibility hooks.
- [Automation Test Framework](https://dev.epicgames.com/documentation/en-us/unreal-engine/automation-test-framework-in-unreal-engine), [Screenshot Comparison](https://dev.epicgames.com/documentation/en-us/unreal-engine/screenshot-comparison-tool-in-unreal-engine) — independent functional and visual evidence.
- [MDN/WCAG contrast explanation](https://developer.mozilla.org/en-US/docs/Web/Accessibility/Guides/Understanding_WCAG/Perceivable/Color_contrast) — optional numerical contrast design-review reference.

## 20. UI validation matrix and staged acceptance gates

### 20.1 Test observable behavior rather than mockup existence

| Fixture | State/interaction assertion | Visual / accessibility assertion | Earliest applicable stage |
|---|---|---|---|
| Selected friendly unit vs hostile/Unaligned | Only controllable friendly receives orders; identity and health update; enemy cannot become friendly by click | Distinct selection/inspection glyph and no stale selection card | Existing slice regression |
| Successful, rejected and interrupted move/attack | Correct typed command and rejection reason; damage/death and clearing preserved | Non-confusing accepted/invalid/order-complete feedback | Existing slice regression |
| Control-point progress, contest and pause | Authoritative progress pauses/advances/completes correctly | Progress/contested/secured states remain legible and synchronized | Existing slice regression |
| Content asset changed, invalid map rejected | Widget shows new authored stat; bad content blocks admission without partial match | Recoverable error and return to main menu; no empty HUD | Stage 3 |
| Multi-select + mixed commands | Count, common/partial eligibility and issued recipient IDs exactly match | Group health/readiness not misrepresented as one unit | Stage 4 |
| Production, refinery and stopped facility | Available stock/queue/reservation match rules; unavailable operation cannot be requested | Costs/throughput/disabled reasons explicit | Stage 4+ |
| Fog/no-contact/last-known | Participant-specific projection excludes hidden truth; old markers carry age | No hidden unit hover/map/tooltip leak | Stage 4+ |
| False-flag report and correction | Claimed marker not Actor; independent reconnaissance updates recipient belief | Claim/sighting/disproof have distinct iconography | Stage 5 |
| Mercenary expiry / Intimidation conversion | Contract/individual controller changes affect legal commands, not faction slots | Selection and badges update immediately | Corresponding faction mechanic |
| Ascendancy stolen vs own-produced bomb | Only qualifying own-production event wins | Custody, production progress and victory clearly differentiated | Nuclear system slice |
| UI modal opened during targeted order | Modal blocks gameplay; Cancel clears target state; no post-close click-through | Focus returns to legitimate control; visible Back | Cross-stage |
| Device-specific command parity | Mouse/controller/touch issue equivalent typed intents for same legal action | Prompts, safe zones and focus readable per device | Device qualification |
| Save/reload/rebind/UI scale | Restored local preferences and valid gameplay state; no stale selection leak | UI scale/labels/focus remain in view on restart | Relevant persistence stage |

Use automated pure C++ tests for command/projection invariants; Unreal Automation/Functional/Automation Driver for HUD state and simulated input; screenshot baselines for layout regressions; and real packaged, rendered gameplay interaction for milestone claims. Use the same negative-fixture discipline as the [Engine Roadmap](ENGINE_OUTLINE.md#78-immediate-next-action-and-milestone-handoff). If test data is absent, mark **BLOCKED** or **NOT_RUN**, not PASS.

### 20.2 Test passes by screen and display

1. **Tactical viewport:** at desktop 1080p, 1440p and 4K, and a supported ultrawide mode, test single selection, right/context click, rejected order, target mode, objective contest, rapid selection/death and pause. Capture the actual viewport, UI scale, labels and command-result telemetry.
2. **Modal/focus:** while a target preview is active, open pause/settings/intel screens; hold world-command inputs, navigate via keyboard/controller, cancel, close and resume. Ensure exactly one semantic intent is delivered to its correct recipient and never to a covered battlefield.
3. **Intel/no-leak:** use two observer fixtures with unequal information and a deliberately fabricated claim; compare field-level projections, minimap contents and targetability. Repeat after claim expiry/retraction and fog rediscovery.
4. **Stress and performance:** simulate large selections, rapid deaths, repeated combat alerts, many reported contacts and shrinking/expanding production queues. Collect Slate/UMG invalidation counts, frame time and memory; avoid rebuilding every row every tick.
5. **Device-specific:** exercise handheld gamepad without keyboard, trackpad alternative if offered, and Android touch on *actual hardware* including system insets and text scale. Desktop simulated controls do not prove physical usability.
6. **Offline and recovery:** start without network, return from invalid content/save/setting, reload the main shell and re-enter a valid local scenario. Confirm UI status text never promises unavailable campaign or multiplayer modes.

For screenshot comparisons, use Epic's tools as a baseline **in addition to** semantic assertions, not as a replacement. UI-only screenshots can appear correct while selection authority, fog filtering or move requests are wrong. Logs should record semantic action ID, selected instance set, command admission rule, observation revision and test result, with privileged details omitted from shipping logs.

### 20.3 Acceptance sequencing and explicit exclusions

**Now:** preserve the accepted Linux Development vertical-slice HUD and shell, fix any incorrect presentation of currently simulated data, and bind display values to Stage 3 validated unit/weapon definitions when they exist. No new UI subsystem is a dependency for typed content validation.

**First complete match:** implement only the command-card, mixed selection, minimap, resource/queue, fog, objective and results surfaces required for the concrete match-loop rules. Verify the minimum independently playable offline loop and command/state consistency before expanding the roster.

**First campaign/AI mission:** add briefing/debrief, mission log, faction-specific actions and intelligence reporting along with genuine world/claim provenance. Test misleading reports as a designed game phenomenon without misleading the player about what is verified.

**Later platforms/release:** qualify remapping, controller/touch tactical command parity, accessibility settings, localization, safe areas, performance and recoverable package behavior on each target device. Multiplayer/replay, full economy, all rosters, production art, screenshots-as-proof and console certification remain outside near-term Stage 3 scope.

**Document governance:** this file owns the *player-facing semantic contract and interface presentation*. It does not supersede mechanics in [Concepts](CONCEPTS_OUTLINE.md), world truth in [Engine](ENGINE_OUTLINE.md), faction rules in specialist documents, evidence in the [Archive](../Archive/README.md), or release qualification in [Performance and Release Gates](PERFORMANCE_AND_RELEASE_GATES.md). When an underlying mechanic changes, update its owner and the corresponding UI acceptance row together. Preserve status distinctions: **design proposal ≠ source implementation ≠ packaged acceptance ≠ platform qualification**.
