# Command & Conquer UI — basic overview

**Purpose:** summarize the player-facing UI patterns POLITIKA can learn from C&C, with C&C4: Tiberian Twilight as the primary reference.
**Status:** first-pass design note, 2026-10-27.

## 1. What the UI is for

A C&C-style RTS UI is a command surface layered over a continuously running battlefield. It must let the player answer, quickly:

1. What is happening across the map?
2. What have I selected, and what can it do?
3. What needs my attention now (threat, objective, production, cooldown, or resource)?
4. What command can I issue, and what happened after I issued it?

The interface should expose useful information without requiring the player to open a stack of modal screens. The battlefield stays primary; HUD panels summarize and route commands.

## 2. Typical in-match regions

The exact arrangement varies by game and resolution, but the recurring information groups are:

| Region | Job | Typical contents |
|---|---|---|
| Battlefield / world view | Direct observation and command targeting | Units, structures, terrain, selection indicators, order/ability targeting feedback, combat state |
| Strategic map | Navigate and understand distant activity | Map terrain, friendly/enemy/neutral markers, objective markers, camera viewport, fog/intel state |
| Selection/context panel | Explain current selection and offer available actions | Unit identity/type, health/status, group portraits, abilities, stance/formation or other orders |
| Production / deployment panel | Choose what enters play | Unit/building/upgrade categories, availability/requirements, cost/cap, queue and cancellation |
| Match status | Communicate the strategic state | Resources or command budget, team/player status, score/timer, objective progress, alerts |
| Global command layer | Reach common actions without selection-specific controls | Rally/deploy controls, tabs, pause/options, help, chat and other match controls |

Classic C&C generally uses a fixed command sidebar with construction queues and a radar/minimap. C&C4 changes the economic/production surface: the Crawler is the mobile production center, units consume Command Points, technology uses Upgrade Points, and strategic play emphasizes objectives/TCN nodes. Treat this as a C&C4 ruleset choice, not a universal C&C requirement.

## 3. Core interaction loop

The basic command loop is **select → inspect → command → confirm**:

- **Select:** click a unit, add/remove members, drag a selection box, or recall a control group. Selection should be legible both in-world and in the HUD.
- **Inspect:** show the common facts for the selected units and the valid shared commands. Mixed selections should not make unrelated commands appear usable.
- **Command:** use contextual mouse targeting, a command/ability button, a hotkey, or a strategic-map click. A command may enter a targeting mode, then resolve when the player chooses a world target.
- **Confirm:** show immediate feedback (cursor state, order marker, destination/target line, button state, sound/voice cue), and later communicate completion, failure, damage, cooldown or changed availability.

Common orders include move, attack, attack-move, stop/hold, guard, stance/formation, capture/interact, and unit-specific abilities. C&C4’s manual documents attack-move and unit abilities alongside selection commands and control groups. Exact bindings should be configurable; the manual’s listed keys are evidence of that game’s scheme, not a POLITIKA requirement.

## 4. Information hierarchy and feedback

- **Persistent:** selection health/status, current objective, essential production/cap/resource state, and enough team/match status to make decisions.
- **On demand:** detailed unit stats, ability explanations, build costs/prerequisites, command help and map legend. Tooltips are a good place for detail that would otherwise crowd the HUD.
- **Urgent and transient:** objective changes, under-attack warnings, newly available actions, order failures and major match events. Use restrained sound/visual notification and avoid repeatedly covering the battlefield.
- **World-readable:** health bars, ownership/selection colors, capture state, range/targeting previews and ability effects should be distinguishable at normal zoom and not rely on color alone.
- **Uncertainty-aware:** minimap markers and fog/shroud communicate limited information. Do not show hidden enemy data merely because the HUD has a map panel.

The C&C4 manual describes its minimap as a strategic overview with unit/structure and line-of-sight/fog information, and warns that stealth can make radar incomplete. This is a useful principle: map symbols are an information channel with game rules, not a perfect omniscient view.

## 5. C&C4-specific examples

The C&C4 PC manual labels the match screen with a battle window, contextual window, minimap, common command interface, control groups, resources, build tabs/window/queue, deployment control, experience bar and (in multiplayer) score bar. It also identifies campaign objectives in the upper-left.

Its written controls and mechanics include:

- minimap clicks to jump the battlefield camera;
- keyboard/mouse camera navigation and map bookmarks;
- single/multiple selection, select-all-combat-units, and numbered control groups;
- a selection-dependent unit pane, unit abilities and stances;
- build tabs and a production queue, with queue counts and cancellation;
- unit availability gated by Crawler tier, Command Points, Upgrade Points, and selected class;
- mission objectives and multiplayer score/status.

The same manual makes clear that C&C4 is not simply the classic sidebar/base-building loop: the Crawler is a mobile base and production source; its class affects available units/powers, and deployment/spawn points matter. For POLITIKA, reuse the information architecture and command ergonomics independently of which economy/production loop is chosen.

## 6. Menus and between-match UI

The in-game UI is only one part of the command experience. A complete C&C-like product also needs:

- front end: campaign, skirmish/multiplayer, options, profile/progression and exit;
- match setup: map, teams/factions/classes, rules, difficulty and lobby/readiness;
- pause/options: resume, controls, audio/video/accessibility settings, save/load where supported;
- results/progression: outcome, score/objectives, rewards/unlocks, rematch/next mission;
- onboarding: concise contextual prompts and a practice/tutorial path that teaches selection, camera, orders, production and objectives.

These screens should share visual language and input conventions, but not obscure match-critical state behind decorative presentation.

## 7. Implications for POLITIKA (UE 5.8.3)

Implement the HUD as a **read-only projection of game state**. The simulation owns unit health, orders, production, resources, cooldowns, objectives, visibility and score. UI widgets display those facts and submit player intents through the PlayerController/command layer; they do not directly mutate authoritative gameplay state.

Suggested first-pass pieces:

1. `UPolitikaHUD` / UI root managing match screens and input mode.
2. UMG/CommonUI widgets for selection details, command/ability palette, production queue, strategic map, objective/status strip, alerts, pause and results.
3. A selection model that supports single, multi and mixed selections, with shared-command filtering.
4. A command-targeting state machine for contextual orders and ground/actor targeting, with cancel/escape behavior.
5. A hotkey/action registry that maps remappable inputs to the same command functions as buttons.
6. Event-driven UI updates (selection/state/objective/queue changes), not per-frame polling of every unit.
7. Explicit display settings for interface scale, text scale, contrast/color assistance, tooltip timing and key remapping.

Avoid baking every panel into a single fixed-resolution texture. Keep content in logical layout units, use anchors/size constraints, and scale spacing, type and icons together. At 4K, test readability at actual sitting distance as well as pixel fit. Support compact layouts for 1080p and proportional layouts for 1440p/4K/ultrawide; use optional panel rearrangement or collapse only when it preserves discoverability and command speed.

## 8. First playable UI slice

For the first RTS vertical slice, build only:

- battlefield camera and selection feedback;
- selection panel with name/type/health and at least one available order;
- move/attack/stop plus one targeted ability, available through button and hotkey;
- minimap with camera jump and basic friendly/known-enemy markers;
- one objective/status display;
- if production exists in the chosen prototype loop, a small category list and visible queue;
- pause/options shell and resolution/interface scale settings.

Validate that the same commands work via mouse and keyboard, selection changes cannot leave stale UI, invalid orders explain why they failed, and the layout remains legible at 1080p and 3840×2160. Add richer details only after this core loop is clear.

## 9. Evidence and limitations

**Directly observed references:**

- `Manuals/CC4TTpcMAN(en_US).pdf` in the installed C&C4 folder, especially the complete controls and “Playing the Game” sections (manual pages 3–10). This documents player-facing behavior and the labeled match-screen regions.
- C&C4’s surface inventory at `/mnt/data/CommandAndConquer/REA/Command and Conquer 4 Tiberian Twilight/surface-inspection.md`, which identifies `Data/Apt.big`, map/audio/stream packages, and notes the game binary is native. This inventory does not inspect package contents or prove runtime UI implementation.
- The Tiberian Twilight mod-support XML and schemas at `/mnt/data/CommandAndConquer/CnC_Modding_Support/Tiberian Twilight/`. The pack includes global hotkey/config files, UI settings, logic command/command-set data, and unit ability button templates. This supports the observation that commands/hotkeys/buttons have data-defined authoring surfaces; it does not establish that the shipped game uses precisely these exposed definitions for every screen.

**Boundary:** The manual is strong evidence for the C&C4 player-facing interface. Statements about typical/classic C&C layout are high-level series conventions; this note is not a comparative audit of every C&C title. UE architecture and 4K guidance are POLITIKA recommendations, not claims about EA's proprietary runtime.
