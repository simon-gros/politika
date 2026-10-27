# POLITIKA UI — product overview

**Status:** first-pass product/UI direction, 2026-10-27
**Engine target:** Unreal Engine 5.8.3
**Design reference:** C&C4's command readability and RTS interaction patterns
**Product constraints:** no cinematics; no mandatory internet connection or online-account login
**Display target:** full 4K (3840×2160) with readable, scalable UI

## 1. Product direction

POLITIKA's interface should make the player feel like a commander managing a fluid operation, not an operator navigating a software dashboard. It should keep the map and tactical decisions central, surface the next useful action, and make the state of squads, vehicles, objectives, and orders easy to read at a glance.

Borrow C&C's compact command loop—select, inspect, issue an order, see clear feedback—without copying every panel or assuming C&C4's exact economy. Establish POLITIKA-specific terminology and strategic rules as those systems are decided.

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

Do not lock the resource and production panel to the C&C4 Crawler/Command Point model. The HUD should reflect POLITIKA's eventual gameplay loop, whether it uses a mobile HQ, fixed production, reinforcements, or a hybrid.

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

This is a proposed POLITIKA structure, not a claim about C&C4's proprietary implementation. See [ENGINE_OUTLINE.md](ENGINE_OUTLINE.md) for the broader engine boundaries and [CNC_UI_OVERVIEW.md](CNC_UI_OVERVIEW.md) for reference-game UI observations.

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
