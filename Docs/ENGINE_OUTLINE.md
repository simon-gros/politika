# POLITIKA Engine — foundational outline

**Status:** first-pass architecture, 2026-10-27  
**Target runtime:** Unreal Engine 5.8.3  
**Design emphasis:** clear RTS command interactions and POLITIKA-specific mechanics  
**Resolution requirement:** full native 4K (3840×2160), including usable/readable UI

## 1. Goal and boundary

The goal is a complete, polished real-time strategy game, not a reproduction of a proprietary engine. POLITIKA should use UE as its rendering, platform, editor, asset, and packaging foundation, with a game-specific RTS layer implemented primarily in C++ and UE authoring/configuration assets. C# remains suitable for external content/build tools where it earns its keep; it is not required in the runtime.

The game-specific layer needs to support faction/unit content, squad/vehicle combat, tactical objectives, campaign missions, skirmish and possible multiplayer rules, AI, maps, in-game briefings and dialogue, audio, save/replay, and localization. The economy, production, deployment, reinforcement and objective-control rules remain POLITIKA design decisions; this outline establishes systems capable of supporting those choices.

## 2. Proposed layers

| Layer | Responsibilities | UE-facing implementation |
|---|---|---|
| Platform & application | Startup, settings, input devices, display modes, save/config paths, localization | UE GameInstance, GameUserSettings, Enhanced Input, SaveGame, platform services |
| Presentation | Camera, selection feedback, HUD, menus, audio, VFX, animation, mission briefings and dialogue | PlayerController, Camera components, UMG/CommonUI, Niagara, Audio |
| RTS match framework | Match lifecycle, teams/players, faction selection, rules, objectives, victory/defeat, pause/spectate | GameMode/GameState/PlayerState plus POLITIKA subsystem/components |
| Simulation | Commands, movement, targeting, combat, health/status, abilities, resources, vision/fog, squad cohesion | C++ components/systems; authoritative simulation boundary kept separate from UI and effects |
| World & navigation | Maps, terrain, roads, cover, blockers, capture/resource locations, pathing, visibility | UE World/landscape plus RTS-specific grid/region and navigation data |
| Content definitions | Factions, units, weapons, abilities, upgrades, buildings, objectives, AI profiles | PrimaryDataAssets/DataTables and validated asset references; values data-driven, rules in code |
| Content pipeline & tools | Map authoring, validation, import, localization, balance iteration, packaging | UE Editor utilities and dedicated validation/import tools; World Partition only if map scale warrants it |

Keep authoritative game rules independent of presentation: UI, animations, sound, and VFX observe simulation events rather than own game state. This allows campaign, skirmish, replay, and eventual multiplayer to share the same match rules.

## 3. Core RTS systems

1. **Command and selection:** single/multi-select, group control, contextual orders, attack-move/guard/stop, rally points, command queue, hotkeys, feedback and cancellation.
2. **World movement:** order resolution, path requests, formation spacing, collision/avoidance, terrain constraints, bridges/roads and blocked routes. Use UE navigation where it fits; add an RTS cost/grid layer for tactical movement and deterministic query needs.
3. **Combat:** weapon/ability execution, range and line-of-sight rules, damage/armor, suppression or morale if desired, cooldowns, target priorities, death/destruction and readable combat feedback.
4. **Squads and entities:** shared unit identity/selection with individual members where animation and casualties matter; vehicles and structures as separate archetypes. Do not force every entity into the same movement/health model.
5. **Strategic loop:** resources, production/deployment, upgrades/loadouts, capture/control points, reinforcements and respawn/redeployment rules. Make the chosen POLITIKA loop a ruleset rather than an assumption inside generic unit code.
6. **Information:** player ownership/alliances, vision, fog/shroud, detection, radar/intel and observer/spectator visibility.
7. **Objectives and scripting:** campaign objectives, triggers, waves, dialogue, scripted events, checkpoints, success/failure and mission transitions.
8. **AI:** strategic planner, production/deployment choices, tactical squad coordinator, navigation/engagement behavior, difficulty/personality profiles and debug visualization.
9. **Match shell:** teams, faction/loadout choice, lobby/rules, map rotation, results, progression hooks, pause and reconnect expectations.

### Resource conversion and faction-specific nuclear permissions

The game has **two required resource-conversion chains**. **Crude oil** from oil fields must be refined in **refinery facilities** into **petroleum fuel or kerosene**, with the **Atlantic Concord paying a lower refining cost**. Natural gas is a separate fossil-fuel source. **Crude nuclear materials** (the highest-valued resources) must be converted into **processed nuclear-material game resources** before supporting nuclear plants or weapons. Fossil fuels are second by value; electricity and other resources matter as infrastructure constraints.

The **Atlantic Concord and Red Directorate** are the only **starting nuclear powers** and may refine nuclear materials to construct **nuclear power plants** or their rare, very-long-build tactical weapons. Concord deploys bombs by **nuclear bomber**, Directorate by **ICBM**. **The Meridian Pact cannot refine nuclear materials at all**, has no nuclear construction tree, but can seize completed weapons. It must **always start near a valid oil or gas field**. The **Iron Ascendancy starts without nuclear capability**, and its spies must infiltrate a **Concord or Directorate nuclear research facility** to obtain **both nuclear-refining technology intelligence and weapons-production intelligence**; only then may it refine for its **own first bomb** and it **cannot construct nuclear power plants**. **Completion of its first own bomb triggers immediate active-match victory without deployment**, whereas theft does not.

Implement these as **authoritative, data-driven rules**: validated production recipes and costs; input/output inventories; permission and intelligence checks; immutable construction provenance; interruptible refinery/production queues; electricity generation/consumption; map-generation spawn constraints; and a match-level completion-to-victory event. Protect against refinery bypass, double spending, illegal construction permissions and completion through stolen inventory. Exact quantities and timing remain balance work, not real-world technical specifications.

**Other asymmetric rules:** the Atlantic Concord and Red Directorate have different specialist espionage features, and the Iron Ascendancy alone has Intimidation to convert isolated enemy units. Red Directorate infantry is **much cheaper to produce** than other factions' infantry; Iron Ascendancy has a **modest destructive-power advantage** and some **high-risk sacrificial attack behavior**; Concord refining is modestly cheaper; Meridian starts near fossil-fuel fields. Balance this alongside standard faction doctrine and the Unaligned's neutral sovereignty AI.

See [Resource Economy](RESOURCE_ECONOMY.md) and [Nuclear Ultimate Weapons System](NUCLEAR_WEAPONS_SYSTEM.md). The 1984–1989 Tito conspiracy remains the primary authored story, independent of individual match victory results.

### Espionage progression and Intimidation as gameplay systems

**Distinct espionage abilities:** the Atlantic Concord and Red Directorate have separate specialist spying features (exact abilities still to be authored). The Iron Ascendancy has **more limited espionage**, with an explicit faction-specific upgrade tree whose **full completion** is an authoritative gate to infiltrating **Concord/Directorate nuclear research facilities**. The already-required stolen refining and weapon technology flags are earned only after that gated infiltration succeeds.

**Intimidation:** an **Iron Ascendancy-exclusive command/ability** can convert **eligible isolated enemy units** to Ascendancy allegiance. The simulation must verify faction identity, ability ownership, target hostility, isolation and resolution before transferring control. Allied reinforcement and interruption are possible counterplay. No other faction can use Intimidation, and conversion does not replace nuclear research infiltration.

Represent spy progression, facility ownership, unlocked infiltration actions, intelligence provenance, isolation conditions and unit controller transfers as **authoritative state**, not user-interface assumptions. Add separate test cases for partial versus complete Ascendancy spy upgrades, valid versus invalid research facilities, supported versus isolated intimidation targets, and unit ownership/persistence after a successful conversion. See [Espionage and Intimidation](ESPIONAGE_AND_INTIMIDATION.md).

### Advanced propaganda and faction morale

Provide an **authoritative per-faction morale state for all five factions**, including the NPC Unaligned and non-propaganda Meridian Pact. **High morale boosts eligible production throughput and army discipline** by data-driven, capped modifiers. **Propaganda actions are available only to Atlantic Concord, Red Directorate, Iron Ascendancy and Unaligned**, with different faction-specific methods expressed through **radio, newspapers and television**. Meridian Pact has no advanced propaganda ability tree.

Media facilities have controllable faction/jurisdiction, functioning/power state, distribution or broadcast reach, counteraction and objective events. The match/simulation layer owns morale and outcomes; AI selects and applies Unaligned media actions; content assets define faction eligibility, audience, cost, duration and modifiers; UI displays morale and accessible actions. Avoid implementing morale as a visual-only bar. See [Propaganda and Morale](PROPAGANDA_AND_MORALE.md).

### Match-size and player-to-faction invariant

**Ordinary matches have exactly 2, 3 or 4 occupied playable faction slots**, drawn from Concord, Directorate, Ascendancy and Meridian, **plus one always-present Unaligned NPC/system-AI faction**. Each player or AI-opponent controller is assigned **exactly one distinct playable faction**; Unaligned has its own autonomous system controller and does **not** consume a player slot. The local single-player case may therefore be one human and one or more AI-controlled playable opponents.

The match framework validates unique faction selection, player/faction mappings and capacity (2–4), and spawns Unaligned separately. World/content validation ensures correct spawn counts and Meridian's nearby fossil field. The UI must distinguish **player slots** from **faction presences** and never show the Unaligned as a selectable player. An Ascendancy nuclear-infiltration victory requires a qualifying opposing Concord or Directorate research facility when enabled; the framework must not manufacture one from the Unaligned just to satisfy the rule. See [Match Structure](MATCH_STRUCTURE.md).

## 4. Content model and authoring

Represent each faction, unit, weapon, ability, upgrade, objective type and AI personality as versioned data assets with stable IDs. Definitions should reference reusable gameplay components/abilities, not hide behavior in arbitrary per-unit blueprints. Blueprint subclasses are appropriate for presentation and authored special cases; shared simulation rules belong in tested C++ systems.

Keep content definitions inspectable and data-oriented. Unit and vehicle archetypes should explicitly describe owning faction, available commands, armor, speed, weapons, health, collision geometry, vision, and reusable behavior modules. Keep map data, skirmish AI, mission parameters, progression, audio and interface definitions in validated, versioned UE-native assets or clearly specified schemas. This enables designers to review and balance content without reverse-engineering the gameplay code.

Maps need authored playable bounds, terrain/navigation representation, spawn/deployment points, control/resource objectives, blockers, scripted regions, camera bounds, lighting/audio setup, and validation rules. Build a map validation pass early (missing spawns, unreachable objectives, invalid references, camera bounds, faction-specific constraints).

## 5. 4K as a first-class target

Treat 3840×2160 as a supported baseline, not a later high-resolution toggle.

- **Display:** windowed, borderless, and fullscreen modes; correct 16:9 aspect; runtime resolution changes; DPI-aware UI scale; safe margins; configurable text/UI scale.
- **UI:** design in logical layout units, anchor responsively, and test HUD, menus, tactical map, tooltips, text, selection rings and cursor at 1080p, 1440p, ultrawide, and 4K. At 4K, preserve readable world scale without making the command surface tiny.
- **Rendering:** configure UE scalability and screen-percentage/upscaling options; validate temporal upscaling, anti-aliasing, shadow quality, and post-processing at 4K. Expose quality presets instead of assuming all hardware renders native 4K at max settings.
- **Assets:** provide appropriate source resolution/mips for UI, terrain, decals and faction insignia; use texture streaming and memory budgets so 4K does not require every asset resident at maximum mip.
- **Camera/input:** ensure selection, placement and cursor precision work across DPI scaling and display changes; keep world-space camera behavior independent from HUD scaling.
- **Acceptance target:** no clipped/overlapping controls, text legible at normal viewing distance, correct aspect/camera bounds, and stable input after resolution/DPI changes on a 4K display.

4K support is not equivalent to a 4K render target alone: UI scaling, asset sharpness, memory/performance, and usability must all pass.

## 6. Performance, determinism, and networking posture

Set target player count, entity counts, map size, minimum hardware, and frame-rate target before selecting detailed simulation strategies. Avoid one heavyweight Actor/Tick per small gameplay concern: batch perception, timers, movement updates and UI notifications; profile representative combat scenes early. UE Mass can be evaluated for very high entity counts, but should not be adopted without a prototype demonstrating a concrete benefit.

Keep simulation state and commands explicit and serializable. If multiplayer is required, use server-authoritative rules and replicate commands/state/events rather than trusting clients; design bandwidth and late-join/reconnect behavior deliberately. If deterministic replay or lockstep is required, validate floating-point and physics assumptions in a prototype first—UE's general simulation should not be presumed deterministic across machines. A replay can instead record timestamped player commands plus synchronized state checkpoints if that meets the product goal.

## 7. Suggested implementation sequence

1. **Project foundation:** establish UE project/module layout, build targets, input/settings/save shell, coding conventions, CI/package baseline and source-control asset policy.
2. **Vertical RTS slice:** one test map, camera, selection, move/attack orders, two unit archetypes, health/damage/death, simple HUD and one objective. Validate interaction at 4K immediately.
3. **Simulation/content contracts:** formalize faction/entity/weapon/ability data assets, ownership, orders, pathing, combat events, and map validation.
4. **Full match loop:** resources/deployment according to POLITIKA's chosen strategic loop, capture/objectives, fog/intel, victory/defeat, pause and results.
5. **Mission and AI:** mission scripting/objective framework, skirmish AI and debugging tools; author a representative campaign mission.
6. **Scale and delivery:** benchmark target entity/map counts, add multiplayer/replay if in scope, optimize assets and validate 4K across quality tiers and supported hardware.

## 8. Decisions to lock before detailed implementation

- Exact strategic loop: fixed-base construction/economy, mobile command-post deployment with loadouts and objectives, or a POLITIKA-specific hybrid.
- Infantry control granularity: individual soldiers, squads as the controllable entity, or mixed by archetype.
- Single-player/campaign, skirmish, multiplayer and modding scope for the first release.
- Required platforms, minimum/target hardware, unit counts, map sizes, frame-rate and network player count.
- Replay requirements and whether deterministic simulation is actually needed.
- Whether maps/mods are editor-only or user-authored and loadable at runtime.

## 9. Scope and validation limits

- The architecture described here is a proposal for POLITIKA; documentation and faction artwork alone do not establish that corresponding Unreal modules or gameplay systems have been implemented.
- The chosen subsystem boundaries, simulation update model, gameplay mechanics and technical targets must be validated against actual prototypes and source code as development progresses.
- Before locking the design, test command latency, mixed-unit selection, pathfinding around blockers and bridges, gameplay-state serialization, and HUD usability at supported resolutions. Benchmark representative entity counts and frame-time budgets rather than assuming scalability.

This outline is a starting architecture contract, not a claim of implemented features or a locked strategic design.
