# POLITIKA — Comparative RTS Control-Group and Mouse-Selection Research

**Status: design research, 29 October 2026 — NOT implementation or acceptance evidence.** This document records comparative precedents and derived POLITIKA decisions. [Selection and Group Orders](RTS_SELECTION_AND_GROUP_ORDERS.md) is the **normative interaction/command contract**; [UI Settings and Keymap](RTS_UI_SETTINGS_AND_KEYMAP.md) owns actionable bindings and conflicts; [Military Formations and Discipline](RTS_UNIT_FORMATIONS_AND_DISCIPLINE.md) owns military organization, stances and Cohesion; [Engine Outline](ENGINE_OUTLINE.md) alone owns staging. Comparisons do not establish source-game internal architectures, Unreal compatibility, or reuse rights.

## 1. Research question and ontological distinction

An RTS player must (1) **identify** units, (2) **retain or recompute** a collection of units, (3) **address** them through a convenient shortcut and (4) **issue** independently admissible orders. These operations are different from (5) establishing an enduring tactical formation, (6) prescribing its rules of engagement and (7) granting physically earned military Discipline support. The decisive distinction is that a selection or numbered control group is a **reference to entity IDs**, whereas a formation is a **match-owned operational relationship**. The latter cannot be inferred from the former.

| Concern | Identity/lifetime | State owner | Can independently alter combat, movement or Discipline? |
| --- | --- | --- | --- |
| Single/rectangle/mixed selection | Temporary, participant- and match-generation-scoped IDs | Local player interaction | No |
| Numbered control group | Stored slot (0–9), generation-scoped member ID set | Local player interaction | No |
| Dynamic selection query | Explicit observation/authority-scoped predicate evaluated on use | Local player interaction and authorized observation | No |
| Unit and tactical order | Unit issuer identity, command sequence and correlation | Match coordinator/rules | Yes, only after admission |
| Formation instance | Match-generation-bound formation ID and validated membership | Match/coordinator | Yes, through explicit formation rules and per-member orders |
| Tactical stance | Unit override or formation default | Authoritative state once implemented | Yes, through admitted behavior policy |
| Cohesion / Discipline | Derived physical organization / individual UnitResolve respectively | Match/rules | Cohesion may condition canonical Discipline; hotkey membership never does |

A player may keep the same antitank unit in hotkey groups 1 (combined arms) and 2 (antitank reserve). A soldier cannot normally occupy two incompatible *peer-level* formation slots at once. Switching hotkey groups does not change stance, active route, formation, military role or permissions.

## 2. Source-checked comparative precedents

| Title and evidence | Verified input/property | Transfer to POLITIKA | Do not infer |
| --- | --- | --- | --- |
| **StarCraft II** — Blizzard official simplified and special controls guides [1], [2] | Ctrl+number assigns; Shift+number adds/removes; number selects; rapid double-press focuses; Tab cycles subgroups. Shift+click adjusts selection; Ctrl+click or double-click selects same type in view. | Quick ten-slot recall, subgroup refinement, type filtering, double-tap focus, add/remove gestures; prefer overlapping group membership. | Precise details of undocumented internal data structures or all version-dependent alternate binding profiles. |
| **Command & Conquer 3 / Kane’s Wrath** — EA manual [3] | Click and rectangle select, Shift+click add, Ctrl+0–9 assign, 0–9 recall, double-tap focus, right-click contextual orders, multiple stances, formation move. | Familiar first-play grammar and distinct stance/formation actions; accessible C&C-derived control surface without copying assets. | Automatically equivalent formation physics, orders, or hotkey compatibility in UE. |
| **Age of Empires IV** — official Season One update [4] | Configurable control-group exclusivity; add/remove hotkeys; mouse 3/4/5 rebinding. | Treat exclusive transfer as a deliberate and remappable operation; do not assume all RTS games agree about Shift+group. | Exact default bindings in all releases and keyboard layouts. |
| **Tempest Rising** — developer patch notes, 29 April 2026 [5] | Shift+[Group] adds selected units, removes them from *other* groups and selects the group; right-button freelook introduces configurable delay to arbitrate command versus camera. | Specify ambiguous modifier semantics and click/drag conflict resolution explicitly; keep responsive C&C-like interaction. | That Tempest Rising uses StarCraft's overlapping-group semantics or that its input architecture is accessible. |
| **Beyond All Reason** — official community controls documentation [6] | Large-army selection/order conventions and command customization exemplify scalable tactical interaction. | Investigate later area/line orders and automatic assignment under separate performance/authorization gates. | Unverified default hotkeys or implementation-specific details. |

**POLITIKA choice:** Default to *overlapping* numbered control groups for flexible combined-arms command. Expose a separate **exclusive transfer** operation; a configurable exclusivity preset can be evaluated after the basic, deterministic grammar is implemented. This deliberately differs from Tempest Rising's April 2026 Shift+Group behavior. Avoid overwriting existing group memberships by accident.

## 3. Mouse and keyboard grammar to test, not currently shipped

| Gesture/action | Proposed semantic operation | Risk to resolve |
| --- | --- | --- |
| Left-click eligible friendly | Replace local selection with single entity | Pointer hit proxy, owner/authority, HUD interception |
| Left-click empty tactical ground | Clear local selection | Do not treat a modal or minimap click as empty world |
| Left-drag tactical ground | Rectangle replace, intersect eligible unit selection proxies | Threshold, DPI and camera movement |
| Shift+click | Toggle eligibility-filtered unit | Pointer and modifiers resolved once |
| Shift+drag | Union eligible rectangle members | Zero duplicates, deterministic ordering |
| Ctrl+click / double-click unit | Eligible visible same-archetype selection | Only authorized visible area; not hidden omniscient scan |
| Ctrl+drag | Remove eligible rectangle members from selection | Explicit selection-mode precedence, cancel on focus loss |
| Ctrl+0–9 | Replace saved group's ID set with selected IDs | Must not issue a gameplay order |
| 0–9 / double-tap 0–9 | Recall / recall and focus camera | Debounced rapid tap and empty group |
| Shift+0–9 | Add current selection to slot, preserving other slots | Do **not** silently steal from other groups |
| Alt+0–9 | Transfer selected members to slot exclusively | OS/IME key interception; remap possible |
| Tab / Shift+Tab | Cycle selected eligible archetype/ability subgroups | Stable ordering, no illicit selection of hidden actors |
| Right-click valid ground/observed hostile | Contextual Move/Attack through existing command admission | Mixed eligibility and partial result |
| Middle-mouse drag / wheel | Camera pan / zoom | No selection or order side-effect |
| Shift+right-click | **Deferred** waypoint order queue | No implicit queue in existing Order variant |
| Formation-facing drag | **Deferred** explicit formation tool | Must not be an ordinary selection/right-click gesture |

**Compatibility finding from actual POLITIKA source:** `Source/Politika/Private/PolitikaPrototypeGameplay.cpp` currently binds left mouse to `SelectUnderCursor`, right mouse to `OrderUnderCursor`, Q to Attack-Move, X to Stop, G to Guard, E to Grenade, R to Reload and F to Sprint; W/A/S/D drive only the pan axes. Until 29 October 2026 Attack-Move/Stop were A/S and shared the pan axes. The local selection currently addresses a **single unit**, and no ten-slot control-group system is qualified. The proposed final camera preset uses arrow keys + middle mouse pan, but no input configuration is changed by this research. Retain the existing E/R/F abilities and do not reassign F to camera focus without a conflict-resolved new profile.

## 4. Selection hit testing and gesture-state boundary

Treat each pointer press as one stateful interaction: `Idle → PendingClick → [ClickRelease | BoxSelecting | Cancelled]`. Only enter BoxSelecting after a DPI-aware cursor-motion threshold in a tactical-world context; recognize GUI/modal capture, minimap, touch pan/pinch, middle-mouse pan and explicit ability targeting **before** resolving battlefield selection. Camera movement during a drag must not change the screen-space rectangle criterion. Candidate units are found with a spatial broadphase if useful, then qualified using projected authored selection proxies (including large vehicle bounds), actual visibility and present command authority. Choose and expose partial-intersection versus enclosed-only selection policy; candidate default is **proxy intersection**.

Do not use the unit mesh render origin alone or repeatedly scan all world Actors per pointer move. On release, normalize stable entity IDs **once**, compare before/after selection sets and publish selection-change/visual updates once. Focus loss, Esc, modal interception, match generation change and cancelled targeting clear the pending gesture; none may enqueue a Move.

## 5. Stored membership, dynamic predicates and life cycle

Save ten local slots keyed by match generation and local controlling participant. Stored entries contain **runtime unit IDs**, never trusted Actor pointers or authority. A control group is a mutable set: Replace, Union/Add, Remove, ExclusiveTransfer and Recall. These operations update only local reference sets. Multiple slots may share members. Recalling revalidates against current match phase, authority, unit life/capture state and permitted observation; stale matches, dead units and permanently lost command rights are retired. Temporary incapacity may leave an eligible organizational reference but cannot submit an invalid order. Empty slots report a non-spamming status.

Dynamic selection queries (e.g., `eligible visible tanks in current view`) must be **explicitly scoped and resolved at invocation**, never be a server-side hidden-world global query presented as local player knowledge. Automatic production-group admission is an optional later feature, subject to valid owner/contract changes, capacity and explicit UI preference; it never automatically creates a military formation.

## 6. Batch dispatch and failure transparency

A local player selects a sorted, deduplicated issuer snapshot. The existing match coordinator revalidates match generation, participant authority, issuer and target and enqueues a **normal typed per-unit CommandRequest** for each eligible member. Each has a unique sequence/correlation (a separate presentation-level batch identifier may associate outcomes). Current movement resolves via the existing 0.05 s authoritative loop and navigation. In the minimal group Move every unit requests the same point; later distinct formation slots require independent legal projection and route checks. Results are **not group-atomic**: report `accepted / rejected / interrupted` counts with per-member diagnostic detail, preserve valid prior routes on rejected replacement and never claim acceptance means arrival. Unit death, capture or match end must not replay pending intents.

## 7. Formation/Discipline handshake

Control-group overlap is an addressing convenience, not physically earned support. Only an explicitly admitted formation can own persistent shape/slot/role/stance/adherence relationships; even then, Cohesion is derived from *actual nearby operational combatant geometry*, not an assigned roster count. No group hotkey grants Discipline. Apply the existing [single UnitResolve, faction exceptions and special conversion rules](PROPAGANDA_AND_MORALE.md) exactly as specified; only the existing formation owner may define military geometry and a future cohesive-support rule.

## 8. Delivery and proof

1. **4C baseline (historical exit unchanged):** existing match authority and selection baseline extended with drag/multi-select and independent per-issuer Move/Attack feedback.
2. **4C-G dedicated core-control gate:** ten numbered slots, replace/add/transfer/recall/focus, same-type selection, stable ID retirement, gesture priority, clickable HUD bar, rebinding/conflict checks and cooked-input acceptance. Target for core production usability **soon after** 4C, but not a retroactive 4F first-skirmish blocker.
3. **FORM-F2 onward:** explicit formation geometry, operational instance state, stances and Cohesion derived through the separate specialist contract; queued waypoints, advanced auto-groups and platform parity are separately qualified later.

Minimum new tests: pointer click-vs-box and HUD capture; Shift union and Ctrl subtraction; unique IDs; two overlapping slots issuing one command per issuer; Ctrl/Shift/Alt group operations; double-tap camera focus with no order; empty/stale/dead/captured/NPC/hostile members; partial command failure with preserved unaffected routes; same-process rematch cleanup; input collision A/S and F; authoritative render-step repeatability; a packaged Linux Development interaction with legible slot feedback. Evidence must name exact source SHA/engine pin and **cannot be inferred from documentation**.

## 9. References

1. Blizzard, [StarCraft II: Simplified Controls](https://news.blizzard.com/en-us/article/6640645/game-guide-simplified-controls).
2. Blizzard, [StarCraft II: Special Control](https://news.blizzard.com/en-us/article/4552955/game-guide-special-control).
3. EA, [Command & Conquer 3: Kane's Wrath PC Manual](https://shared.fastly.steamstatic.com/store_item_assets/steam/apps/24810/manuals/CC3KWpcMANfinal.pdf).
4. World's Edge, [Age of Empires IV Season One Update](https://www.ageofempires.com/news/age-of-empires-iv-season-one-update-release-notes/).
5. Tempest Rising, [April 29, 2026 Patch Notes](https://support.saber.games/hc/en/tempest-rising/articles/april-29-2026-patch-notes-24).
6. Beyond All Reason, [Commands 2.0 (Grid)](https://www.beyondallreason.info/commands-2-0-grid).
7. Epic Games, [Enhanced Input in Unreal Engine 5.8](https://dev.epicgames.com/documentation/unreal-engine/enhanced-input-in-unreal-engine?application_version=5.8).

All game comparison evidence is about **public player-facing behavior**. No proprietary source code or assets have been reproduced, and external links do not constitute accepted POLITIKA behavior.
