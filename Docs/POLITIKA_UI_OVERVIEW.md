# POLITIKA UI — product overview

**Status:** first-pass product/UI direction, 2026-10-27
**Engine target:** Unreal Engine 5.8.3
**Design emphasis:** readable RTS commands, map awareness and fast tactical interaction
**Product constraints:** no cinematics; no mandatory internet connection or online-account login
**Display target:** full 4K (3840×2160) with readable, scalable UI

## 1. Product direction

POLITIKA's interface should make the player feel like a commander managing a fluid operation, not an operator navigating a software dashboard. It should keep the map and tactical decisions central, surface the next useful action, and make the state of squads, vehicles, objectives, and orders easy to read at a glance.

Use a compact RTS command loop—select, inspect, issue an order, see clear feedback—without committing the layout to any particular economic or production system. Establish POLITIKA-specific terminology and strategic rules as those systems are decided.

The visual language should evoke plausible 1980s Yugoslav military and civil-administration materials: printed topographic maps, field reports, typed labels, stamped status marks, restrained institutional color, and period-appropriate symbols. Use these as framing and texture, not as a reason to reduce contrast, legibility, or interaction clarity. Avoid making every screen look like a faux terminal.

## 2. Design principles

1. **Battlefield first:** the map remains visible and is the primary command target; HUD panels organize decisions around it.
2. **Fast, predictable commands:** mouse actions, hotkeys, and buttons should invoke the same order and share clear cancel/invalid-target behavior.
3. **Show what matters now:** persistent HUD space is reserved for objectives, selected-force status, essential resource/production state, and match-critical alerts.
4. **Explain consequences:** show costs, prerequisites, target requirements, cooldowns, and unavailable states before commitment where possible.
5. **Respect uncertainty:** map and HUD only show what the player's units/sensors are allowed to know; stale or uncertain intel must not look current.
6. **Readable at every supported resolution:** 4K is a design baseline, not a pixel-doubled 1080p layout. Support useful scale and text settings.
7. **Offline by default:** the game should reach its main menu and support its single-player modes without internet, account creation, or sign-in.
8. **Story without cinematics:** narrative context is delivered in concise briefings, radio dialogue, mission events, debriefs, and documents; gameplay remains the visual focus.

## 3. Main navigation and local-first operation

The front end should prioritize direct play:

- **Continue** when a resumable local campaign save exists;
- **Campaign** for mission selection and briefing;
- **Skirmish** for local match setup against AI;
- **Load / Save** for local progress where supported;
- **Options** for display, audio, controls, accessibility and gameplay settings;
- **Credits** and **Quit**.

No menu should block on account login, entitlement checks requiring a network, or a connection to a publisher service. Saves, settings, campaign progress, and unlocks should be stored locally. If optional online or LAN features are later chosen, they must be additive: offline campaign/skirmish remains usable, and sign-in failure cannot strand the player outside local content. Whether multiplayer is in scope is still a separate product decision.

## 4. Campaign and narrative screens (no cinematics)

Use authored in-game UI and audio rather than pre-rendered or real-time cinematic sequences:

- **Mission briefing:** map/region, situation, friendly forces, known enemy activity, primary/secondary objectives, constraints, and a concise commander's note. Present it as a field dossier or operations map, with optional voice/radio delivery and captions.
- **Mission start:** transition from briefing to playable map with a short title/objective cue, not a cinematic intro.
- **In-mission story:** objective updates, radio exchanges, map annotations, field reports, and authored gameplay events. Keep dialogue skippable and never pause tactical input unexpectedly in an active match.
- **Debrief:** mission outcome, objective completion, casualties or relevant performance summary, campaign consequences, and next action. Avoid mandatory cinematic reward sequences.

Briefings and dialogue should be concise, pausable/skippable, subtitle-capable, and available in text for players who mute audio or need to revisit an instruction.

### Parallel storylines, refining and faction ability presentation

Keep the **primary Tito assassination/cover-up narrative** and **secondary Ascendancy first-bomb nuclear race** independently legible in mission briefings and debriefs.

**Only the Atlantic Concord and Red Directorate start with nuclear power**. Both have rare tactical nuclear ultimate weapons: Concord deploys with a **nuclear bomber** and Directorate via an **ICBM**. The **Iron Ascendancy begins non-nuclear and with limited espionage**. It must **fully upgrade its spy capabilities** before it can infiltrate a Concord or Directorate nuclear research facility, obtain **both nuclear-refinement and weapons intelligence**, refine nuclear materials for bomb production, and complete the long build. **As soon as its first own-built bomb completes, it immediately wins the current game**—there is no launch/deployment action.

**Nuclear materials rank highest by value, fossil fuels second.** The UI needs distinct inventory/progress indicators for crude oil, petroleum fuel, kerosene, crude and refined nuclear game materials, electricity and other resources. Crude oil is processed in **refineries**, at **lower cost for the Concord**. Only Concord and Directorate can construct **nuclear power plants**. Meridian cannot refine nuclear material, and Ascendancy cannot construct nuclear power plants. Meridian must always spawn near an eligible oil or gas field, without automatically owning it.

**Specialized espionage:** Concord and Directorate each have their own **different special spying features**, with exact abilities not yet defined. Show the Ascendancy's **spy-upgrade progress** and clearly communicate that **nuclear research infiltration requires full development**. The Ascendancy alone has **Intimidation**, a unit action that can **convert isolated eligible enemy units** to Ascendancy control. Its target cursor/tooltip must distinguish **isolated and eligible** from **supported or ineligible**, with clear success/failure and ownership-change feedback. Never display Intimidation as an available native action for any other faction.

Use existing objective, production, selected-unit and intel UI patterns. Show only information the faction has legitimately discovered: do not expose hidden enemy nuclear-research locations or spy identities without detection. Nuclear weapons cannot be purchased in the Black Market, and stealing a completed bomb does not confer manufacturing knowledge or Ascendancy instant victory.

See [Espionage and Intimidation](ESPIONAGE_AND_INTIMIDATION.md), [Resource Economy](RESOURCE_ECONOMY.md), and [Nuclear Ultimate Weapons System](NUCLEAR_WEAPONS_SYSTEM.md).

### Population morale and propaganda controls

Display the **population morale meter for every faction**, including Meridian Pact and NPC Unaligned where the game's information/fog rules permit it. Make the two principal positive consequences—**production output** and **army discipline**—understandable through tooltips, modifiers, unit inspection or production panels.

Propaganda-capable factions (**Atlantic Concord, Red Directorate, Iron Ascendancy and NPC Unaligned**) use distinct methods expressed through **radio, newspapers and television**. Player-facing propaganda controls belong only to the three playable eligible factions; the Unaligned is AI-controlled. **Do not display an advanced propaganda action tree for the Meridian Pact**, even though its morale remains present and mechanically relevant. Media control, regional coverage and propaganda effects should be reflected in mission objectives and event notifications. See [Propaganda and Morale](PROPAGANDA_AND_MORALE.md).

### Two-to-four-player match setup

Map/match setup supports **2, 3 or 4 participant slots**, with **one distinct playable faction per slot** (Concord, Directorate, Ascendancy or Meridian). In local single-player, remaining slots may be filled by AI opponents. **The Unaligned always appears as a separate non-editable NPC/system AI presence**, not a fifth selectable faction or player slot. Explain the map population as **2+1, 3+1 or 4+1**; the +1 is always Unaligned.

Match validation should flag too few/many participants, duplicate faction assignments, missing Unaligned initialization, missing Meridian nearby fossil-fuel source, or unavailable special nuclear infiltration objectives where no qualifying Concord/Directorate research facility is present. See [Match Structure](MATCH_STRUCTURE.md).

## 5. In-match UI structure

Exact placement can be tuned with prototypes, but the initial layout should provide these functional regions:

| Region | POLITIKA role |
|---|---|
| Battlefield | Main view for movement, combat, selection, target designation, and tactical feedback |
| Selection/force panel | Selected unit or squad identity, health/readiness, relevant status, and shared valid commands |
| Command/ability panel | Contextual orders and selected unit abilities, with hotkey hints and unavailable reasons |
| Strategic map | Camera navigation, objectives, friendly positions, and only currently known enemy/intel markers |
| Objective panel | Current primary/secondary objectives, state changes, and explicit completion/failure feedback |
| Resources/force status | The chosen POLITIKA economy, deployment/production state, and any unit/force cap that affects decisions |
| Alert/status layer | Under-attack, completed order, lost communication, objective change, and other time-sensitive notices |

The resource and production panel should reflect POLITIKA's own gameplay rules, whether those use mobile command posts, fixed-base production, reinforcements, or a hybrid. Show only the resources and limits the player can act upon.

### Command interaction

Use one consistent cycle:

1. Select an individual unit, squad, vehicle, or group.
2. Show identity, health/readiness, important effects, and commands valid for the full selection.
3. Issue move, attack, attack-move, stop/hold, guard, capture/interact, or a unit-specific ability using contextual click, a button, or hotkey.
4. Confirm the order in-world and in the HUD; communicate failure if the order is invalid, blocked, out of range, or interrupted.

Support selection boxes, modifier-based add/remove, and numbered control groups. Mixed selections should present common commands clearly and avoid silently applying a command to only a subset. A targeted ability/order must have visible target mode and a reliable cancel action.

## 6. Supporting screens and information

- **Skirmish setup:** map, sides/factions if applicable, teams, AI difficulty, match rules, and start. No network lobby or account prompt for local play.
- **Pause menu:** resume, save/load where allowed, controls, options, restart, abandon mission, and quit. It must clearly communicate whether simulation is paused.
- **Options:** display/resolution and UI scale; text scale; graphics; audio and voice/subtitle settings; key remapping; camera and selection behavior; accessibility choices.
- **Map/intel view:** legend, objective status, and known operational information. It should preserve the distinction between current vision, radar contact, and stale intelligence if those mechanics exist.
- **Post-match/debrief:** result, objective status, concise performance information, and next steps. Do not require account-bound progression or online rewards.
- **Tutorial/help:** explain selection, camera, orders, objectives, and any distinctive POLITIKA mechanic in-context; allow replay from a local help screen.

## 7. Presentation and accessibility

Use a restrained military-document palette, clear faction/unit iconography, disciplined typography, and strong visual hierarchy. Reserve distressed paper, stamps, grain, or map-grid decoration for backgrounds and transitions; never put important labels on noisy surfaces without a solid backing.

The interface must not convey ownership, danger, or order state by color alone. Pair colors with icons, shape, labels, patterns, or animation. Provide adjustable text/UI scale, high-contrast option, subtitle controls, and remappable bindings. Keep cursor and selection feedback visible against both bright snow/sky and dark terrain.

At 4K, scale UI in logical units and preserve ergonomic text/icon sizes; do not simply anchor tiny 1080p controls to the corners. Validate 1920×1080, 2560×1440, 3840×2160, and ultrawide layouts, including display changes at runtime. Keep the tactical camera/world scale independent from HUD scale.

## 8. Technical ownership in UE

Treat UI as a presentation layer over the game state:

- C++ match/simulation systems own units, orders, objectives, visibility, resources and match results.
- PlayerController/input actions translate mouse and keyboard input into player command intents.
- UMG/CommonUI presents selection, objectives, resources, menus, map and notifications.
- UI reacts to state events and view models; it must not become the authority for gameplay rules or poll every unit every frame.
- Local save/settings services handle progress and configuration without requiring a remote identity.
- A shared command/action definition should drive both button availability and hotkey execution to prevent inconsistent behavior.

This is the proposed POLITIKA presentation/simulation boundary. See [ENGINE_OUTLINE.md](ENGINE_OUTLINE.md) and [PROPOSED_ENGINE_LAYERS.md](PROPOSED_ENGINE_LAYERS.md) for related architectural decisions.

## 9. First playable UI slice

Start with a single offline mission/test map containing:

- camera, selection, and clear selected/hovered/owned state;
- move, attack, stop, and one targeted ability, accessible by both button and hotkey;
- compact selection panel with type/name, health/readiness, and status;
- one objective display with update/completion/failure feedback;
- strategic map with camera jump and visibility-correct markers;
- the minimum production/reinforcement/resource panel needed by the prototype's rules;
- pause, local save/load if in scope, and basic display/UI scale options;
- a short text/radio briefing and debrief instead of cinematic sequences.

Acceptance for the slice: no online login is needed to launch or play; keyboard and mouse commands agree; invalid orders are understandable; selection changes cannot leave stale panel data; subtitles/readable text work at 1080p and 4K; and the tactical screen remains uncluttered during combat.

## 10. Decisions still open

- Final economy, production, reinforcement and unit-cap model.
- Whether any multiplayer is planned (and if so, local/LAN/online scope); this overview only fixes offline single-player access as a requirement.
- Which unit categories are individually controlled versus grouped as squads.
- The exact campaign briefing/debrief presentation and radio-dialogue cadence.
- Supported display aspect ratios, minimum hardware, and the final accessibility/settings list.
- Whether the strategic map is always visible, collapsible, or switchable to a larger planning view.

These should be decided through interaction prototypes. The present document defines the UI's product posture, not final screen mockups or locked bindings.
