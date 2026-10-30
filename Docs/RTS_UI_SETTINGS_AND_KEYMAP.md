# POLITIKA — Practical RTS UI, Settings, Menus and Input Specification

**Status: DESIGN/PROPOSED, 29 October 2026; source-grounded inventory below, not an implementation or accepted build.** This is a normative *implementation contract* for settings and inputs, supplementing the extensive [UI product overview](POLITIKA_UI_OVERVIEW.md), which remains the owner of overall visual hierarchy, faction-specific presentation and accessibility. [Engine Outline](ENGINE_OUTLINE.md) owns sequence; [Selection/Group Orders](RTS_SELECTION_AND_GROUP_ORDERS.md) owns authoritative selection; [Shell](SHELL_SKELETON.md) owns currently verified shell/recovery behavior; [Match Persistence](RTS_MATCH_PERSISTENCE_AND_REPLAY.md) owns future save/replay. Do not change gameplay authority or native assets merely because a specification is written.

**HUD-specific ownership:** [RTS HUD Architecture](RTS_HUD_ARCHITECTURE.md) owns selected-force/card semantics, contextual modules, permitted observation data, notification behavior, formation/commander/intelligence panel integration and the HUD acceptance matrix. This file remains the owner of action IDs, currently verified bindings and conflicts, menu/settings implementation, focus/back behavior and keymap tests. Neither document establishes an implemented interactive RTS HUD.

## 1. Code-grounded audit: what actually exists

Examined at source Git SHA \`8eca9a35d58298559f51500c9e04bbdae6a85dd4\`:

| Area | Actual source/config | Implemented versus missing |
| --- | --- | --- |
| Display defaults | \`Config/DefaultGameUserSettings.ini\` | 1280×720, \`FullscreenMode=2\` (windowed), VSync enabled, 60 FPS cap, basic quality scalability. This is a **default**, not a selectable settings screen. |
| UI scale | \`Config/DefaultEngine.ini\` | Shortest-side \`UIScaleCurve\`: 480→0.667, 720→1, 1080→1.5, 2160→3. The shell correctly warns against inserting a second DPI scaler. **No adjustable user preference** implemented. |
| Display persistence | \`PolitikaUserSettings.cpp\`, bootstrap module | UE \`UGameUserSettings\` subclass; temporary safe 1280×720 windowed reset and session-only apply; settings file backup/recovery exists. **No end-user video preview/Keep/Revert flow** implemented. |
| Menus | \`PolitikaShellPlayerController.cpp\` | Slate main-menu/sandbox recovery panel, prototype launch, reset safe settings, diagnostics, exit; **not** the proposed campaign/skirmish/settings/help suite. Some sandbox introduction copy is historical shell wording. |
| Tactical UI | \`PolitikaPrototypeGameplay.cpp\` | Immediate-mode Canvas HUD: unit health/order, control objective, command status, five command rows; no interactive production/minimap/notification history. |
| Audio | \`Art/Audio/Prototypes\` / audio documents | Audio prototypes and concepts exist; **no end-user mixer, volume-bus settings or tested audio routing** implemented. |
| Input | \`Config/DefaultInput.ini\`, \`APolitikaPrototypePlayerController::SetupInputComponent\` | Legacy axis mappings and direct \`BindKey\`; no player-mappable Enhanced Input bindings, no built hotkey settings/help menu. |
| Default route | \`Config/DefaultEngine.ini\` | Tracked \`GameDefaultMap=/Game/Maps/RTSVerticalSlice\`; separately qualified MainMenu packages used controlled config overlays. Do **not** assume every clean checkout boots the main menu. |

### 1.1 **All currently evidenced mouse/keyboard inputs**

These are current source bindings, **not** the intended final default profile. An action bound to a key may be inapplicable without an eligible selected unit.

| Input | Current actual behavior | Evidence and caveat |
| --- | --- | --- |
| Left mouse | Click selects one owned live unit (empty/other click clears); drag past a DPI-scaled threshold box-selects owned units; Shift+click toggles, Shift+drag adds, Ctrl+drag removes; Ctrl+click or a double click selects the clicked unit's archetype on screen; a click on the group bar acts on that slot | \`BeginSelectPress\`/\`EndSelectPress\`, \`SelectUnderCursor\`, \`BoxSelect\`, \`SelectSameArchetype\`; 4C and 4C-G records |
| 0–9 | Recall control group (double tap also centres the camera); Ctrl replaces, Shift adds, Alt moves the selection into the slot | \`PrototypeControlGroup0\`–\`9\` actions → \`ControlGroupKey\`; the handler reads the modifier; [4C-G record](RTS_CONTROL_GROUPS_4CG_ACCEPTANCE.md) |
| Tab / Shift+Tab | Cycle the selection's archetype subgroups; grenade, reload and sprint use the active subgroup | \`PrototypeSubgroupCycle\` action → \`CycleSubgroup\`; never edits saved groups |
| Right mouse, ground | Issue normal Move to world-hit map position; with two or more units selected each gets its own Line or Column slot (FORM-F2). When the selection is exactly one whole formation, RMB / Q / X send one formation Move / Attack-move / Stop that the server plans and fans out per member (FORM-F3) | \`OrderUnderCursor\`; admission/nav can reject per unit |
| V | Cycle the group-move shape, Line / Column, for the next multi-unit Move or Attack-move and the next T; local, sends nothing; an existing formation keeps its own shape | \`PrototypeGroupMoveShape\` action → \`CycleGroupMoveShape\`; [FORM-F2 record](RTS_FORMATION_F2_GROUP_MOVE_ACCEPTANCE.md) |
| T | Form the selected units (2–64) into one match-owned formation in the current V shape; members already in another formation move into the new one | \`PrototypeFormFormation\` action → \`FormSelected\`; [FORM-F3 record](RTS_FORMATION_F3_LIFECYCLE_ACCEPTANCE.md) |
| Y | Release the selected units from their formations; a formation left with one member dissolves | \`PrototypeReleaseFormation\` action → \`ReleaseSelected\`; sends nothing when no selected unit is a member |
| H | Cycle combat stance Hold Ground → Guard → Aggressive → Hold Fire. With exactly one whole formation selected it sets the formation's stance (and clears members' own); otherwise it sets each selected unit's own stance. Sends no order | \`PrototypeCycleStance\` action → \`CycleStanceSelected\`; [FORM-F4 record](RTS_FORMATION_F4_STANCE_ACCEPTANCE.md) |
| J | Cycle a whole formation's adherence Flexible → Maintain → Free. Any other selection sends nothing | \`PrototypeCycleAdherence\` action → \`CycleAdherenceSelected\` |
| Right mouse, hostile unit | Attempt Attack on hit target | \`OrderUnderCursor\`; shared order admission |
| Q | Attack-Move to the point currently under the cursor | \`PrototypeAttackMove\` action → \`AttackMoveSelected\` on press, no separate confirm cursor state |
| X | Stop selected unit | \`PrototypeStop\` action → \`StopSelected\` on press |
| G | Guard selected unit (provisional prototype semantics) | \`GuardSelected\` |
| E | First unit of the active Tab subgroup uses grenade ability | \`GrenadeSelected\`; capability/target checks still apply |
| R | Eligible infantry of the active Tab subgroup reload | \`ReloadSelected\`; not a control-group command |
| F | Eligible infantry of the active Tab subgroup sprint | \`SprintSelected\`; **F is occupied**, so an unmodified F camera-focus proposal conflicts |
| Escape | Exit current prototype match to main menu | \`ReturnToMenu\`; **not** a pause/options popup |
| W / S | Camera forward / backward | Legacy \`PrototypePanForward\` axis |
| A / D | Camera left / right | Legacy \`PrototypePanRight\` axis |
| Mouse wheel | Zoom camera | Legacy \`PrototypeZoom\` axis |

**Resolved collision (29 October 2026):** A and S used to be both camera axes and the Attack-Move/Stop orders, so camera navigation with a selection issued orders and each order press nudged the camera (this broke the PAC-10 grenade aim under load; see [Test-map terrain and camera controls](TEST_MAP_TERRAIN.md#pac-10-camera-pan-assertion-harness-defect-fixed)). By owner decision, W/A/S/D stay camera-only and the orders moved to **Q** and **X**. G/E/R/F are config action mappings too. The `input.bindings` audit check rejects any order key that reuses an axis key and, since 4C-G, any key chord that drives two actions, and the packaged `rts_interaction` replay holds A+S with a selected unit and requires zero command intents. This is a prototype keymap, not the remappable UI-C profile. The HUD currently labels Attack-Move "Q + ground", but its source invokes an immediate cursor-ray action when Q is pressed, so documentation/tooltip must not imply a two-step target-confirm workflow exists. The Canvas rows are drawn, not buttons: accessibility and click behavior are not yet supported.

### Future formation and stance controls (design only)

**FORM-F3 (30 October 2026)** implemented Form (**T**) and Release (**Y**, which covers Detach and Disband) and formation-addressed Move / Attack-move / Stop; see the [FORM-F3 record](RTS_FORMATION_F3_LIFECYCLE_ACCEPTANCE.md). Reinforce, facing/width drag and Cohesion below remain design only. **FORM-F4 (30 October 2026)** implemented stance (**H**) and adherence (**J**) as cycling keys; Withdraw and a stance panel remain design only.

[Military Formations, Tactical Stances, Cohesion and Discipline](RTS_UNIT_FORMATIONS_AND_DISCIPLINE.md) proposes explicit **Form / Reinforce / Detach / Disband** actions, with independent **shape/facing**, **combat stance** (Aggressive, Guard, Hold Ground, Hold Fire; Withdraw deferred) and **formation adherence** (Maintain, Flexible, Free). Distinguish local selected/control-group state from persistent organization, show per-member admission/slot/navigation outcomes, and display derived Cohesion alongside the *same* per-unit Discipline resolve. A facing/slot preview is advisory, not gameplay authority.

Solve the current `A`/`S` camera/order binding collision before choosing formation shortcuts. Keyboard/mouse, controller/Steam Deck and Android touch may use different gestures mapped to identical semantic intents; preserve cancel/focus, accessibility and hidden-information constraints. This UI is **not implemented**.


## 2. Recommended interaction layout (desktop baseline)

The schematic below is a compact implementation hypothesis; the dedicated [HUD Architecture](RTS_HUD_ARCHITECTURE.md#4-information-architecture-and-layout) specifies its semantic regions, event/projection data flow and cross-platform composition. Keep HUD layout choices distinct from normative keyboard and group-selection semantics in this specification.

Use a **battlefield-first, relatively low-occlusion** layout combining C&C's immediately discoverable sidebar/production grammar and StarCraft's selected-unit command/selection clarity, without borrowing protected artwork, textures or branding.

~~~text
┌────────────────────────────────────────────────────────────────────┐
│ Objective + event ticker       $ Treasury  Power  Food         │
├─────────────┬─────────────────────────────────────────┬────────────┤
│ Minimap     │                                         │ Intel /    │
│ camera box  │       PLAYFIELD / ORDERS / ROUTES        │ diplomacy  │
│ territory   │                                         │ drawer     │
│ overlays    │                                         │ (optional) │
├─────────────┴───────────────┬─────────────────────────┴────────────┤
│ Selected unit/group         │ Orders + contextual production       │
│ owner / HP / status / queue │ eligibility / bound key / why locked │
└─────────────────────────────┴──────────────────────────────────────┘
~~~

Proportions are conceptual, not fixed pixel measurements. On 16:9 desktop, avoid a permanently expanded intel panel. Objective and alert state must be readable while battles continue. The top rail shows **spendable US dollars ($)**, dollar income, minimum-building electricity capacity/load and food reserves/runway; gold, fuel and nuclear materials remain conditional drilldowns. Do not render a greyed-out sprawling resource bar full of unused icon slots. The minimap is a projection of player knowledge, not a radar that reveals hidden truth. Selected unit/group count, owner, health, active order, selection eligibility, producer queue and confirmation/rejection feedback get a stable region. Allow player-controlled panel density and at least one compact preset; test at 1280×720, 1920×1080 and 3840×2160; add Deck/mobile safe-area layouts later.

UI must consume authorized match snapshots and typed gameplay outcomes. An unavailable command should display the *reason*, with a location hint and current keybinding where meaningful. Distinguish **command accepted**, **executed**, **completed**, **rejected**, and **navigation failed**; "command accepted" does not imply arrival or damage. Never reveal enemy resources/production/orders through HUD convenience.

## 3. Full menu and modal architecture

~~~text
Boot / recovery (offline, no account requirement)
  Main Menu
    Continue [enabled only for a compatible real save; future]
    Campaign [locked/unimplemented until mission content exists]
      Mission selection → briefing/intelligence dossier → mission → debrief
    Skirmish [locked until 4F; prototype launch remains Development-labelled]
      Scenario/map → human/opponent slots → faction → options → loading
    Prototype [Development-only entry while in development]
    Settings
      Video & Display | Graphics | Audio | Controls | Gameplay/UI | Accessibility/Language
    How to Play
      Controls list | command semantics | economy/power | political/intel concepts
    Credits / Licences / Quit
In-match (when implemented)
  Battlefield HUD
    Context inspect / production / intel dossier / diplomacy overlays
    Help and keymap (read-only)
    Pause / Options / Restart or Surrender / Return / Quit [modal confirmation]
  Results
    Outcome/reason, timeline, losses/objectives, optional statistics
    Rematch / Main Menu (save replay only if implemented)
~~~

Do not display nonfunctional buttons as operational features. Either hide unavailable branches in a prototype menu, or label them as unavailable with an explanation. Preserve the existing reliable recovery path even if normal settings widgets fail to create. Every modal defines focus owner, Escape/back behavior, input consumption, selection/targeting cancellation, and return focus. Pause must freeze the **offline authoritative coordinator** (no accumulated catch-up on resume); menu-driven settings can remain responsive. A later multiplayer mode must define independent pause rules.

## 4. Video, resolution and scaling settings — precise ownership

| Setting | Recommended UI policy | Technical boundary |
| --- | --- | --- |
| Display/monitor | Enumerate live monitor identity when platform supports it; distinguish stable ID from transient index; safe fallback for unplugged monitor | Use UE/platform display adapter, not an inferred per-monitor guarantee from global resolution enumeration |
| Window mode | Windowed, Borderless Windowed (UE \`WindowedFullscreen\`), Fullscreen where supported; disable/explain unsupported exclusive modes | \`UGameUserSettings::Get/SetFullscreenMode\`; platform compositor constraints |
| Resolution | Choose from **current supported modes** for selected display/mode; 1280×720 safe recovery | \`SetScreenResolution\`; validate actual framebuffer/window result |
| Windowed size | Expose actual logical client window size separately from desktop mode where needed | Clamped to usable desktop area; remember placement safely |
| Rendering resolution scale | Scene 3D rendering resolution/quality, independently of menu text | \`GetResolutionScaleInformationEx\`, scalability; avoid attaching readability to GPU resolution |
| UI scale | Suggested player preference 75–200% with a 100% default; **candidate tuning values** | Effective layout scale = project DPI rule × user preference **once**, not double-transform via nested scalers |
| Text scale | Separate 100–150% text preference and min readability; tune with real fonts and locales | Do not assume it is the same parameter as viewport DPI or scene resolution |
| Presets | Low/Medium/High/Custom, independent toggles for VSync, FPS cap, effects/texture/shadows, reduced motion | Treat displayed changes as transactions, not immediately permanent writes |
| Accessibility | Contrast, subtitles and captions, UI opacity, screen-edge hints, reduced flashes, color-independent relations | Preserve text / icon feedback even if animations, VFX, or SFX disabled |

**Safe video change state machine (normative):** \`StoredConfirmed → Draft → PreviewApplied → [Keep/Cancel/Timeout/DisplayLost] → StoredConfirmed\`. Snapshot *both* stored values and live confirmed window tuple (mode, resolution, monitor ID/index, position where valid). Preview changes the visible window **without saving** and shows a 15-second Keep/Revert modal that remains accessible by keyboard/controller. On Keep: confirm and persist only after actual mode verification. On Cancel, timeout, focus/display loss or failed apply: restore prior confirmed tuple *and explicitly reapply it* to the live window, preserving saved bytes. UE \`RevertVideoMode\` restores settings values but **does not by itself guarantee reconfiguration of the actual platform window**; \`ApplySettings\` persists settings, so it is unsuitable for a no-save preview. Test both \`ApplyResolutionSettings\` and actual viewport/framebuffer readings on the pinned UE 5.8.3 checkout. Startup command-line \`-ResX\`/\`-ResY\`/\`-Windowed\` overrides are session-only and must not overwrite confirmed preferences.

On bad/unsupported mode, implement fallback to last confirmed mode or safe 1280×720 windowed without deleting other profile/settings files. Do not misreport a rejected settings write as saved. Use the shell's existing bootstrap backup and version guard instead of creating a second settings persistence service.

UE references: [UGameUserSettings (5.8)](https://dev.epicgames.com/documentation/unreal-engine/API/Runtime/Engine/UGameUserSettings?application_version=5.8); [DPI Scaling (5.8)](https://dev.epicgames.com/documentation/unreal-engine/dpi-scaling-in-unreal-engine?application_version=5.8); [UI Project Settings (5.8)](https://dev.epicgames.com/documentation/unreal-engine/user-interface-settings-in-the-unreal-engine-project-settings?application_version=5.8).

## 5. Audio and subtitle settings

Implement one user master gain plus separate **Music**, **Combat/SFX**, **Units/Voice Acknowledgements**, **Dialogue/Radio**, **UI**, and optional **Ambience** levels, each with immediate audition, Mute and safe persisted value. A simple first slice may combine Combat/UI and Dialogue/Voice if there are not yet distinct audio routes; do not present sliders that have no actual routed sources. A critical tactical alert requires optional ducking of music/ambience during voice/radio, not an additional spendable resource. Subtitles/transcripts must remain available when voice is muted; include sound-event direction/category for important alerts if useful. Avoid clipping by designing test signals and a documented gain conversion; UI "0–100" percent is a **presentation value**, not necessarily a linear amplitude.

Prefer Unreal **Sound Classes** to group assets, with **Audio Modulation Control Buses/Mixes** where actual category routing is needed. Epic's quick start explicitly recommends attaching buses to Sound Classes instead of individual sound assets. Audio Modulation is an optional plugin; verify enablement/build/cook on UE 5.8.3 rather than silently adding it. Preserve a no-plugin basic class-mix fallback if just master/SFX/music controls are needed. When in-game volume changes, apply to live audio and verify across a clean restart; on save failure retain documented in-memory behavior and report it.

References: [Audio Modulation Quick Start](https://dev.epicgames.com/documentation/unreal-engine/audio-modulation-quick-start-guide?application_version=5.8), [Sound Classes](https://dev.epicgames.com/documentation/unreal-engine/sound-classes-in-unreal-engine?application_version=5.8). [Audio events](../Data/audio_events.json) remains an authored registry, not proof of routed/bus audio playback.

## 6. Keymap: unique semantic actions, discoverable controls

**Implementation choice:** introduce one stable **action ID** per intent (e.g. \`rts.order.stop\`, \`camera.pan_left\`, \`ui.menu.cancel\`), with localized action name, purpose/tooltip, category, context, device and default binding. The Controls menu must be generated from the same bound action registry used by input dispatch: never hand-maintain a second contradictory "list of keys." Backward-compatible legacy bindings may remain temporarily, but the transition must not cause both old and new listeners to execute the same intent.

**Conflict resolution first:** reserve mutually distinct default keys for overlapping gameplay contexts. Recommended first desktop preset: keep W/A/S/D for camera *or* switch camera to arrow keys/middle-mouse drag and preserve A/S tactical standards—**choose, implement and test exactly one consistent baseline**. The earlier candidate default was **camera via arrow keys + middle-mouse drag**, **A Attack-Move**, **S Stop**. On 29 October 2026 the owner instead chose **W/A/S/D camera-only, Q Attack-Move, X Stop**, with G Guard, wheel Zoom, LMB Select, RMB Context Order and Esc. This is now the implemented prototype baseline; arrow-key/MMB pan may still be added as extra camera bindings. **Do not silently change defaults in a documentation-only change.** If the player binds a camera axis and an order to the same physical key in the same active context, block and explain collision (or require an explicitly distinguishable modifier/chord); resolve by rebind, swap or cancel.

### 6.1 Recommended complete categories — **proposal, not enabled key bindings**

| Action family | Suggested initial binding | What the help UI must explain |
| --- | --- | --- |
| Select / inspect / contextual move or attack | LMB, RMB, modifiers | Owner/visibility requirements; what happens when no valid target or order |
| Camera pan, zoom, group focus | W/A/S/D (implemented); optional arrows/MMB-drag; wheel; double-tap numbered group; optional remappable camera-focus action | Do **not** assign F to camera focus by default: current infantry F is Sprint; no implicit combat order |
| Tactical commands | Q Attack-Move, X Stop (implemented), G Guard, E Grenade, R Reload, F Sprint (current ability keys retained); optional later stance keys | Match current ability availability; target confirmation/command state; no accidental camera orders |
| Selection gestures | LMB, marquee drag; Shift+click toggle, Shift+drag add; Ctrl+click/double-click same-type; Ctrl+drag subtract | Commandable selection excludes hostile/neutral/dead; same-type applies only to authorized, visible on-screen entities |
| Numbered control groups (4C-G) | Ctrl+0–9 replace, 0–9 recall, double-tap recall+focus, Shift+0–9 add, Alt+0–9 exclusive transfer, Tab subgroup cycle | Ten slots including 0; overlaps allowed; no hotkey creates a formation or enqueues gameplay orders; keybind conflicts/OS interception |
| Queue waypoints | Shift+RMB (later) | Append vs replace and queue limit; not present in existing prototype |
| Alert history / base focus | Space latest alert; Backspace base focus (later) | Camera only, no new sensor/omniscient intel |
| Diplomacy/intelligence/map overlay | Candidate D, I, M (later) | These are **screen-navigation intents**, never privileged gameplay actions |
| Production/build | Contextual command card or selected producer menu (later) | Bound recipe/unlock/price/power restrictions and queue state |
| Pause/back/help | Esc contextual Pause/Back; F1 help (later) | Esc in prototype currently **returns to main menu** |
| Screenshot and system shortcuts | Platform/Steam capture handled externally unless adopted | Do not hijack Alt+Tab, OS window switching or accessibility shortcuts |

### 6.2 Proposed complete tactical selection/group actions (not currently bound)

These entries refine the summary above. [Selection and Group Orders](RTS_SELECTION_AND_GROUP_ORDERS.md) owns exact set algebra, authority and validation; [Comparative RTS Group/Input Research](RTS_CONTROL_GROUP_INPUT_RESEARCH.md) owns sourced precedent. `4C` means first multi-selection/commands; `4C-G` is the **separate mandatory core-production group-control follow-on**, not part of the historic 4F first-skirmish exit.

| Semantic action ID (candidate) | Proposed desktop binding | Operation and context |
| --- | --- | --- |
| `rts.selection.click` | LMB tactical world | Replace with one commandable unit, or clear on empty terrain; inspection remains separate |
| `rts.selection.rectangle` | LMB world press + drag beyond DPI-aware threshold | Replace with eligible proxy-intersecting IDs; never while modal/minimap/target mode captures |
| `rts.selection.toggle_member` | Shift+LMB | Toggle one eligible unit |
| `rts.selection.union_rectangle` | Shift+LMB drag | Union rectangle with current selected IDs |
| `rts.selection.subtract_rectangle` | Ctrl+LMB drag | Remove intersecting eligible IDs; drag wins over Ctrl+click at threshold |
| `rts.selection.visible_same_archetype` | Ctrl+click or double-click eligible unit | Select same authored type in currently authorized visible viewport |
| `rts.group.assign.0-9` | Ctrl+0–9 | Replace stored slot ID set with current selection; no gameplay command |
| `rts.group.recall.0-9` | 0–9 | Recall valid existing group; refuse stale generation |
| `rts.group.union.0-9` | Shift+0–9 | Add selected IDs **without stealing** from other groups |
| `rts.group.transfer.0-9` | Alt+0–9 | Transfer selected IDs out of other numbered slots into destination slot |
| `rts.group.focus` | Rapid second press of same number | Re-select and focus camera; configurable/debounced threshold, never issue a move |
| `rts.selection.subgroup.next/previous` | Tab / Shift+Tab | Cycle eligible archetype/ability subgroup; does not rewrite stored slot membership |
| `rts.order.context` | RMB tactical world/observed hostile | Dispatch per issuer via current typed request; mixed capability outcomes shown |
| `camera.pan` / `camera.zoom` | W/A/S/D (optional arrows, MMB-drag) / wheel | Camera-only actions; camera pan cannot also produce Stop/Attack-Move |
| `rts.order.queue_append` | Shift+RMB **later** | Explicit bounded queue only once a separate queue contract exists |
| `rts.formation.facing_preview` | Explicit Form/Reform tool **later** | Formation geometry/stance is not implicit in a drag-selection |

**Collision policy:** Only one semantic action can consume an input chord in the same active context unless an explicit deterministic chord/hold/release disambiguation exists. Current **F= Sprint** blocks using unmodified F as Focus. The former **A/S** order/pan conflict is resolved by Q/X order keys; `input.bindings` keeps it from returning. Ctrl/Shift/Alt combinations must be validated against desktop compositor, IME, accessibility and shortcut interception; where blocked, offer rebinding and a clickable group bar. Legacy `BindKey` listeners and new Enhanced Input mapping must not both dispatch the same request during migration.

**Gesture arbitration:** One press owns one path `Idle → PendingClick → ClickRelease | BoxSelection | Cancel`. Drag threshold is DPI-aware; hold a stable camera/view/projection snapshot or explicitly document behavior during pan. UI widgets and minimap consume pointer input before the tactical world. Escape/Back closes the deepest modal/target/drag state first; on focus loss/phase transition/rematch, clear latched modifiers and pending click states. Rectangle selection uses authored selection proxies, not just a unit pivot, with eligible/visible/command-authority filtering before one sorted/unique ID transition.

**Group HUD requirements:** up to ten clickable slots (0–9), active slot, count, empty/stale indicator, group ownership/eligible count, keyboard hints taken from actual rebound actions and optional formation badge **only if** a real formation exists. A control-group button never silently changes unit orders or produces military Discipline. With mixed selected types, distinguish common commands, subset-eligible actions and ineligible counts; feedback must match each typed command outcome.

Rebinding: support alternative bindings, modifier chords, deliberate duplicate detection, keyboard-layout display names, gamepad/touch separate profiles, Reset Category/Reset All with confirmation, accessibility alternatives to held keys, and changes visible instantly in HUD/tooltips/help. Respect system-level key capture and input-method-editor/text-entry focus. Distinguish rebind **failure** from a saved key profile. Avoid requiring keyboard shortcuts to access key settings.

UE 5.8 [Enhanced Input](https://dev.epicgames.com/documentation/unreal-engine/enhanced-input-in-unreal-engine?application_version=5.8) supports Input Actions/Mapping Contexts and platform redirects. [UEnhancedInputUserSettings](https://dev.epicgames.com/documentation/unreal-engine/API/Plugins/EnhancedInput/UEnhancedInputUserSettings?application_version=5.8) and [Map Player Key](https://dev.epicgames.com/documentation/unreal-engine/BlueprintAPI/EnhancedInput/UserSettings/MapPlayerKey?application_version=5.8) provide player-mappable binding interfaces. They do not automatically create POLITIKA's complete help, permissions or persistence semantics. [CommonUI guidance](https://dev.epicgames.com/documentation/unreal-engine/design-guidelines-for-using-commonui-in-unreal-engine?application_version=5.8) specifically notes a PC RTS need not use CommonUI for its basic HUD; consider it only if layered menus and controller navigation justify it.

## 7. Other RTS interface evidence and what to borrow

| Source title | Source-supported feature | Adaptation, not asset imitation |
| --- | --- | --- |
| Command & Conquer Remastered | EA documents a sidebar for structure production, power shortage, campaign/skirmish options and user-accessible resolution/audio/key bindings. | Fast accessible producer/action panel, conspicuous power state, functional global Settings |
| StarCraft II | Blizzard documents tooltips revealing hotkeys, control-group/Shift queue, notification recenter and selection modes. | Complete dynamically bound action help; consistent group command grammar, notification history |
| Age of Empires IV | Official guide emphasizes villager/resource rally and workers managing several commodity loops. | Useful negative comparison: avoid routine four-resource villager micromanagement |
| Company of Heroes series | Sector control and connected territory generate manpower/fuel/munition income without dedicated gatherer farming. | Political/infrastructure nodes and cut routes, with optional limited logistics, as POLITIKA's primary operating economy |

Primary refs: [EA C&C Remastered guide](https://help.ea.com/articles/command-and-conquer/command-and-conquer-remastered/how-to-play/), [Blizzard StarCraft special controls](https://news.blizzard.com/en-us/article/4552955/game-guide-special-control), [Blizzard simplified controls](https://news.blizzard.com/en-us/article/6640645/game-guide-simplified-controls), [Age of Empires IV beginner guide](https://www.ageofempires.com/news/age-of-empires-iv-tips-to-help-you-get-started/), and [Company of Heroes territory supply explanation](https://www.coh2.org/guides/1987/introduction-to-coh-franchise). These are precedent sources, not a claim of POLITIKA compatibility, licensed art or identical mechanics.

### 7A. Currency, upkeep and exchange controls (design adopted 29 October 2026)

The economy HUD must show **in-game USD ($)** as the single ordinary purchasing balance, plus dollar income; **electricity supply / required load** for operational buildings; and **food reserves / staffed-force upkeep** with runway. Each production card quotes dollar cost and distinguishes lacking funds from missing power or insufficient military provisions. A dedicated trade/commodities drawer shows acquired **gold, raw oil, natural gas and refined fuels** as physical inventory with available/reserved quantity, an eligible USD sale quote, applicable fees and confirmation. It must not offer ordinary nuclear-material sales or claim a rumor is a transferable stock.

The food procurement drawer shows actual dollar price, food quantity and optional player-limited auto-replenishment; no generic agricultural-worker harvesting interface. Low power, paused production and inadequate food must appear in concise, accessible alerts with a concrete recovery action. These are **design targets**, not working menus. The [dollar and upkeep contract](RTS_OPERATIONAL_ECONOMY_AND_POWER.md) owns transaction, cadence and shortage rules; this document owns information layout and device interactions.

### Proposed compact screen-transition and focus acceptance matrix

The [UI Overview](POLITIKA_UI_OVERVIEW.md) remains visual/product authority; this matrix defines testable interaction behavior, not a second screen router. Each implemented screen/overlay should specify entry trigger, permitted phase, primary action, Cancel/Back destination, focus restoration, pointer/controller/touch alternatives, input hint source and authoritative effects. Screens marked unavailable must not masquerade as functioning features; the current Development prototype launch/recovery path stays supported.

| Proposed gate | Test interaction |
| --- | --- |
| UI-NAV-01 | Boot into a known main-menu or prototype route with documented config; use keyboard/controller to reach each enabled action, and Cancel returns to the prior focus target without submitting gameplay commands |
| UI-NAV-02 | Open and close an in-match overlay; orders stop reaching the battlefield while modal focus captures input; gameplay pause is separate and must obey the offline match lifecycle |
| UI-NAV-03 | Choose windowed/borderless/fullscreen and valid resolution in a *preview*; Keep persists only after confirmation, Revert/timeout restores the last known-good visible mode, restart/recovery does not trap the user off-screen |
| UI-NAV-04 | User UI scale composes with the existing Unreal DPI curve once, preserving legible status and actionable targets at 1280×720, 1920×1080, 3840×2160 and 1280×800 (handheld); validate focus and safe areas, not only screenshots |
| UI-NAV-05 | All enabled actions have one semantic ID, actual displayed binding, supported remap/conflict detection and source-backed in-game help; holding W/A/S/D with a selected unit never accidentally emits Stop or Attack-Move |
| UI-NAV-06 | UI command outcome is correlated to real typed request/issuer; distinguish rejected, accepted, executing, interrupted and completed without leaking hidden opponent state or promising an unimplemented feature |
| UI-NAV-07 | Audio mixer, subtitles/captions, reduced effects and non-color alerts are user-testable where supported, and modal/menu control remains usable with no audio |
| UI-NAV-08 | Save/Return/Rematch actions appear only when supported by the match phase and verified persistence contract; no stale focused widget or selected dead unit survives a new match generation |

Implementation choices may use Unreal [Common UI](https://dev.epicgames.com/documentation/unreal-engine/common-ui-plugin-for-advanced-user-interfaces-in-unreal-engine), [Enhanced Input](https://dev.epicgames.com/documentation/unreal-engine/enhanced-input-in-unreal-engine) and [Game User Settings](https://dev.epicgames.com/documentation/unreal-engine/API/Runtime/Engine/GameFramework/UGameUserSettings), but their actual 5.8.3 behavior and any experimental integrations require local source/package verification before adoption. Steam Deck acceptance should separately check controller accessibility, correct glyphs and text readability using [Valve's compatibility guidance](https://partner.steamgames.com/doc/steamhardware/compat). These references describe candidate approaches, not currently enabled UI subsystems.

## 8. Implementation phases and acceptance

**UI-A — immediate correction:** record current keys and conflicts, establish common semantic registry/proposed defaults and native text help; test W/A/S/D held with a selected unit for **zero accidental commands**; keep current controller/command authority and packaged prototype. This is a gameplay-code change and requires Unreal Automation + rendered regression.

**UI-B — safe settings foundation:** Video/Display modal with 15 s Keep/Cancel/Timeout and actual window/monitor restore; independent UI/text scale, safe initial resolution, no data-loss; use existing bootstrap and \`UGameUserSettings\`. Include tests for unavailable monitor, unsupported mode, launch override, failed writes, safe mode, bad profile, windowed and fullscreen on supported host.

**UI-C — functional Controls + Audio:** Enhanced Input contexts, profile key rebinding and dynamic help, conflict resolution; routed sound class groups/master/categories, subtitle/caption policy. Verify the audio mix actually changes sound output, not merely slider text.

**UI-D — production-era RTS HUD:** command card (clickable where appropriate), group selection, minimap based on authorized observations, objectives, alerts, power/food/$ purchasing/production, political dossier and accessibility. Verify that video/audio settings persist and render across device profiles. Test HUD at 720p/1080p/4K, ultrawide and a handheld safe-area target; qualify Android separately.

| Gate | What must be **actually** observed |
| --- | --- |
| UI-01 | Every documented **current** binding matches source; no double bound gameplay/camera action under selected-unit state |
| UI-02 | A saved rebind changes actual input and HUD/help hint; no old listener fires |
| UI-03 | Conflict, reserved key, missing required Cancel/Select binding and invalid profile fail with explanation |
| UI-04 | Windowed/Borderless/Fullscreen behave as supported; exact stored/live tuple; 15 s timeout reverts **live** mode |
| UI-05 | UI scale and text scale independent of 3D resolution; usable and unclipped at 1280×720 through 4K |
| UI-06 | Lost monitor, unsupported resolution, failed save and restart preserve original preference bytes or safe fallback |
| UI-07 | Each audio slider affects real routed test tone; categories isolated and persisted; mute leaves subtitles available |
| UI-08 | Pause/modal blocks world commands and clears held-target state; no simulation catch-up on resume |
| UI-09 | Every visible HUD/menu action has a working or explicitly disabled target and an accessible help description |
| UI-10 | Same-process match return/retry leaves clean focus, bindings, viewport and menu state |
| UI-11 | Rendered Linux Development packaged test + real window observation and input replay; headless alone is insufficient |
| UI-12 | Accessibility: keyboard-only traversal, screen-reader-related text surfaces where platform supports them, non-color status, reduced effects, captions and controller/touch profile tests |

**Not yet qualified:** no settings or keymap code was changed by this document; no enabled Audio Modulation, CommonUI, MVVM, Enhanced Input or arbitrary full-screen/monitor support is claimed. In UE 5.8, MVVM is explicitly a Beta feature; prefer small native event-driven presentation updates unless an actual UI scope warrants that plugin. References: [UMG Viewmodel](https://dev.epicgames.com/documentation/unreal-engine/umg-viewmodel-for-unreal-engine?application_version=5.8), [Localization using FText](https://dev.epicgames.com/documentation/unreal-engine/text-localization-in-unreal-engine?application_version=5.8).
