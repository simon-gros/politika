# POLITIKA — Selection, Group Commands and Order Queue Contract

**Status: 4C baseline (GRP-01 to GRP-12) IMPLEMENTED 30 October 2026** — multi-selection, box/Shift/Ctrl gestures and per-issuer group orders; see the [group orders record](RTS_GROUP_ORDERS_4C_ACCEPTANCE.md). Control groups (4C-G) and the rest of this document remain **PROPOSED design, 29 October 2026**. The inspected prototype currently maintains one selected unit and routes typed gameplay orders through the existing coordinator; its infantry behavior also includes grenade, reload and sprint inputs. Persistent numbered groups and marquee selection remain design-only. This document owns proposed selection-set and multi-issuer semantics; [command contract](RTS_COMMAND_SIMULATION_CONTRACT.md) owns existing `CommandRequest` and coordinator behavior, [Stage 4A](STAGE4A_MATCH_LIFECYCLE_ACCEPTANCE.md) owns authority proof, [UI](POLITIKA_UI_OVERVIEW.md) owns widget/gesture design and [Engine Outline](ENGINE_OUTLINE.md) owns staging. Do not infer implementation or change the accepted order variant as part of this document.

## Scope and comparative provenance

The [RTS HUD Architecture](RTS_HUD_ARCHITECTURE.md#51-selected-force-and-control-groups) specifies selection-card and control-group-bar presentation, mixed eligibility, current key prompts and event feedback; it **does not** define group membership, grant authority, change control-group algebra or alter the per-issuer command pipeline owned here.

[Comparative Control-Group and Mouse-Selection Research](RTS_CONTROL_GROUP_INPUT_RESEARCH.md) records official StarCraft II, C&C3, Age of Empires IV and Tempest Rising input precedents plus proposed POLITIKA adaptations. It is **research**, not an alternative normative keymap or a verified feature. [Military Formations and Discipline](RTS_UNIT_FORMATIONS_AND_DISCIPLINE.md) alone owns match-state tactical organization/stance/Cohesion. The ten-slot group system in this document is a core production feature targeted in dedicated **4C-G** after 4C's foundational multi-selection; existing 4F first-playable acceptance is not silently expanded.

## 1. Selection is presentation-adjacent state, not command authority

A local selection set is a normalized collection of **runtime entity IDs paired with match generation**, in deterministic order; it holds neither transient UObject pointer authority nor faction ownership permissions. Selection is allowed only while a local controller has a valid participant binding. On every read and immediately before group submission, revalidate that each entity still exists, is alive/operational, selectable, has present command-selection authority for the controlling participant (currently ownership-based; later valid contractual control is distinct from legal ownership) and, when an authored roster exists, matches admitted faction/source permissions.

Invalidated selection is retired, its visual indicator removed and selection-changed feedback emitted **once** per meaningful change; avoid treating a prior weak pointer as valid because it was once selected. A dead unit cannot remain in the roster after cleanup. A neutral/unbound/hostile unit cannot be *command-selected* via forged ID, drag rectangle, control-group recall or post-rematch stale context. Authorized enemy/neutral **inspection** is a separate, read-only, information-filtered selection mode; neither inspection nor a hotkey slot confers command authority.

## 2. Canonical selection operations (candidate)

| Intent | Transition and checks |
| --- | --- |
| Single click | Set = eligible owned unit, or clear if empty/invalid hit; inspect enemy separately from selection |
| Add/toggle modifier | Shift+click toggles one eligible owned/commandable entity; Shift+drag unions an eligible rectangle; de-duplicate and sort by stable ID |
| Drag-box | After a screen-space drag threshold, intersect eligible, authorized, visible selection proxies with the rectangle; normalize once before changing visuals |
| Same-archetype (4C-G) | Ctrl+click or double-click selects eligible units of same authored archetype within current visible viewport; never a hidden-world unit scan |
| Assign/control group (4C-G) | Ctrl+0–9 replaces stored slot membership; Shift+0–9 unions selection into slot; Alt+0–9 transfers selected units from other numbered slots into this slot; no implicit order or authority |
| Recall group (4C-G) | 0–9 selects a revalidated snapshot; double-press within configured threshold focuses the camera without issuing an order; stale/dead/captured/unauthorized retired, empty group feedback |
| UI/world focus | Selecting via minimap, roster or keyboard uses the same validated selection method |
| Return/Terminal/new match | Invalidate all selection/control-group buffers, visuals and input latched states |

### 2A. Distinct selection, control, organization and order entities

| Object | Owner / lifetime | Effect and exclusion |
| --- | --- | --- |
| Selection set | Local player, participant + match generation, ephemeral sorted set of runtime IDs | Names the present issuers, optional primary unit; does not change game state |
| Control-group slot | Local player, numbered 0–9, generation-scoped sorted IDs | Persistent recall convenience; a unit may occur in multiple slots; no formation, combat bonus or authority |
| Dynamic selection rule | Local player, evaluated on explicit invocation against authorized observed candidates | `same archetype in current view`, later filter/rally/auto-group; cannot scan invisible enemy/world truth |
| Tactical formation | Match-owned unique formation ID, explicit Form/Reform/Detach/Disband admission | Shapes, member slots, stance/adherence/Cohesion; normally one active peer-level formation per unit, even if multiple hotkey groups |
| Gameplay order | Existing coordinator, typed per-issuer `CommandRequest` sequence and result | Mutates game state only after ordinary authority/target/route admission; hotkey recall never executes it |

Never identify a control-group slot (such as `2`) with a formation instance or a unit archetype; neither slot number nor actor weak pointer is stable match authority. A selected antitank squad may be in groups 1 and 2, while its single formation assignment and independently admitted active order remain unchanged.

### 2B. Control-group algebra and exact semantics (4C-G core feature)

For selected eligible entity IDs `S`, stored group `G[n]` and all other stored groups `G[m]`, normalize all sets by stable runtime ID under the **current** match generation:

| Gesture / operation | Definition | Feedback |
| --- | --- | --- |
| Ctrl+0–9 / Replace | `G[n] := S` (including an explicitly confirmed empty replacement, if offered) | Slot n count and members update; no world order |
| 0–9 / Recall | `Selection := Revalidate(G[n])` | Unique selected highlights; empty/invalid slot status |
| Rapid double-tap n / Focus | Recall then frame living eligible members in camera | Only camera position changes; no route or targeting |
| Shift+0–9 / Union | `G[n] := G[n] ∪ S` | Preserve membership of other groups; no silent transfer |
| Alt+0–9 / Exclusive transfer | For each `m ≠ n`, `G[m] := G[m] \ S`; then `G[n] := G[n] ∪ S` | Show changed slot counts; command-selection remains S |
| Explicit removal in group UI | `G[n] := G[n] \\ S` for requested slot only | Other slots unaffected |
| Tab / Shift+Tab | Cycle currently selected eligible archetype/ability subgroups in stable order | Refines active command subgroup, not saved membership by default |

**Decision:** overlapping membership is the default; **exclusive transfer** is an explicit, distinct operation. Tempest Rising's April 2026 Shift+Group uses an exclusive transfer, which POLITIKA deliberately assigns to Alt+number so Shift+number can express additive membership. Group add/transfer must not automatically recall or replace the current selection; this avoids unintended orders after composition editing. Optional exclusive-group preferences may be considered later only after remapping/gesture tests.

Store ten slots including **0**; use physical digit keys or configurable device shortcuts through semantic action IDs, never hardcode scan codes into authoritative gameplay rules. Group assignments can be updated while units are moving or in combat without cancelling orders. Optional automatic production/rally grouping is a **later** feature and must not create formations or Discipline support.

### 2C. Pointer and rectangle-selection interaction (4C baseline then 4C-G)

Local input state machine: `Idle → PendingClick → ClickRelease | BoxSelecting | Cancelled`. The viewport stores press position, modifiers, hit-context and active interaction mode. On movement beyond a **DPI-aware** pixel threshold from a tactical-world press, enter BoxSelecting; on release, find candidate units by spatial broadphase (when measured necessary), intersect their *authored selection proxies* with the screen rectangle, filter present selectable/observed/commandable status and apply one normalized set operation:

- Unmodified click/drag **replaces** the selection; clicking empty tactical ground clears it.
- Shift+click **toggles** one member; Shift+drag **unions** eligible members.
- Ctrl+drag **subtracts** eligible members; Ctrl+click / double-click a unit selects matching *eligible on-screen archetype* only. Ctrl+drag takes precedence over Ctrl+click after crossing threshold.
- One input stroke must create at most one meaningful selection transition. Hover/highlight is provisional; selection ring changes cannot claim a gameplay command was admitted.
- UI widget, minimap, modal, ability-target mode, middle-mouse camera pan, text/IME input or gesture capture prevents a world-selection start. Focus loss, Esc, phase change, rematch and cancelled mode clear any held press/box; **do not** synthesize a click/order during cancellation.
- Default selection proxy policy: **intersection** with projected proxy shape (not Actor pivot), so partially boxed vehicles are selectable. Expose a stricter enclosed-only user setting only if UX testing warrants it. Overlap, edge coordinates, DPI scaling, changing camera matrices, selection of units beyond viewport and rendered frame scheduling need explicit fixtures.

Friendly-but-not-commandable, inspected neutral and enemy entities must never enter the commandable set. Do not reveal hidden enemy positions through same-type selection, global all-units queries, minimap shortcuts or error messages.

### 2D. Dynamic group behavior and lifecycle (4C-G)

Control groups are manually maintained **membership snapshots** by default; dynamic *selection predicates* are evaluated only when invoked and bounded by the participant's authorized observation (e.g. eligible armored units in the current view), not continuously server-mutated rosters. Later optional automatic assignment of newly produced units requires its own rule, enabled player preference, stable archetype identity and lawful control; it must not silently join a formation.

Store `(matchGeneration, participantBinding, slot, sortedEntityIds)`; never a trusted raw pointer array. On recall, command dispatch and relevant entity transitions, classify each member as: **valid and currently order-eligible**, **temporarily inoperable but still affiliated**, or **permanently invalid** (dead, destroyed, permanently transferred, wrong generation, revoked command rights). Drop permanently invalid references; preserve temporarily inoperable references only when they remain legitimately affiliated, show the reason and never submit them as active issuers. At Active→Terminal/Return/rematch unconditionally invalidate selection, all slots, pending drag/target state, presentation and pending batch handles. The existing unit-invalidation event must not fan out duplicate selection-change events.

**No group-atomicity by default.** A group command is a batch of single-issuer typed requests: each retained issuer has its own correlation/sequence and independent admissibility; one invalid unit must not silently invalidate every legitimate command, and one admitted request must not retroactively authorize another. The UI should aggregate outcomes (e.g., two accepted, one rejected) without inventing an all-or-nothing transaction.

## 3. Authoritative group dispatch

1. Controller maps input to one semantic intent and validated local selection snapshot. No UI may call private trusted numeric-`PlayerId` command helpers.
2. Match coordinator revalidates controller→participant, match generation and unit owner/faction; takes a deterministic sorted ID snapshot.
3. For each unit: construct an ordinary `CommandRequest` with that unit's stable issuer ID, **unique** sequence and correlation; optionally associate a UI-only batch correlation handle for outcome aggregation; queue it through the existing intake. Membership overlap never causes duplicate issuers in one batch. Commands are admitted at the next authoritative 0.05 s step.
4. Derive destination offsets from the existing map-space coordinate contract. Initial group Move sends the **same requested point** per issuer; any group spread/formation spacing is a later separately validated feature and must still project legal route endpoints.
5. Apply existing order-replacement semantics per unit, retain independently rejected prior active routes, and publish per-unit typed outcomes. Do not introduce per-actor `Tick` movement or second scheduler.
6. At result/return/respawn or generation mismatch, cancel group handles/queued requests without replaying stale commands.

**Queueing is deferred.** Shift-queued waypoints and build queues are different processes and must have distinct semantics. For future per-unit order chains, lock bounded capacity, valid command kinds, queue append/replace behavior, cancellation/stop, path failure, target loss, death and item-level feedback before writing code; never reinterpret current single active `Order` as an implicit FIFO.

### 3A. Mixed capability and per-member order policy

Right-click ground resolves each eligible selected unit's existing `MoveOrder`; observed hostile target resolves an existing `AttackOrder` for each attack-capable member; non-attackers remain selected but receive explicit **not eligible** feedback rather than hidden forced movement. Every order is admitted or rejected per unit. Unsupported abilities (grenade/reload/sprint) likewise apply only to explicitly previewed compatible members and must not pretend a common ability exists. Rejected replacements do not corrupt another member's active route. Later formation slots, queue append and stances belong to separate accepted contracts.

### 3B. Explicit tactical formations (separate later capability)

[Military Formations, Tactical Stances, Cohesion and Discipline](RTS_UNIT_FORMATIONS_AND_DISCIPLINE.md) defines authoritative formation identities, membership, shapes, facing, slots/roles, combat stance, adherence and derived Cohesion. **A local control group or a drag-selected roster is not a formation.** The 4C baseline remains a batch of individually admitted and correlated existing `CommandRequest` orders, with partial success/rejection. A later explicit Form/Reform/Detach/Disband operation can establish organizational state and compute distinct slot endpoints, but those must be legal for each member's movement class and must preserve independent route replacement, Stop, death and generation cleanup. Do not reinterpret the current `Order` variant to implement a new stance or assume group atomicity.


## 4. Input and accessibility grammar

Desktop pointer: click-select; drag/box-select only after a DPI-aware movement threshold from tactical-world start; cancel box on Esc, modal input capture, focus loss and active targeting mode. Explicitly distinguish pan from box and avoid drag issuing a move order. Controller: semantic select/confirm/cancel and inspect/command context (consider focus/cursor hybrid only after usability tests). Touch: start with tap-select and explicit action/target mode, long-press inspected separately; multi-selection must not conflict with map pan/pinch. Each device maps to identical semantic orders, not identical gestures.

Epic's [Enhanced Input](https://dev.epicgames.com/documentation/unreal-engine/enhanced-input-in-unreal-engine) supports input actions and dynamically prioritized mapping contexts, suitable for modal targeting and future rebinding. [CommonUI's own guidelines](https://dev.epicgames.com/documentation/unreal-engine/design-guidelines-for-using-commonui-in-unreal-engine) explicitly caution that a simple PC RTS may not need its full input-routing framework: consider it for the layered cross-platform shell, not as a prerequisite to dispatch Move.

Feedback must distinguish: input invalid before submission; per-unit admission rejection; admitted order later unable to navigate; command replaced/stopped; and actual arrival/attack completion. For mixed group outcome, show counts and allow inspection of failing units without disclosing hostile hidden state. Render selected/faction outline at RTS zoom and keyboard/controller focus; never require color alone.

## 5. Acceptance scenarios

| ID | Contract |
| --- | --- |
| GRP-01 | Single selection baseline remains unchanged and owner-bound |
| GRP-02 | Drag/multi selection includes every valid friendly, excludes hostile, neutral, dead and unselectable |
| GRP-03 | Same unit from multiple hit sources remains one ID; deterministic ordering |
| GRP-04 | Two-unit Move generates two distinct correlated typed requests admitted on next step |
| GRP-05 | Mixed valid/invalid targets preserve good requests; correct rejection for each bad issuer |
| GRP-06 | Hostile/control-spoof and unbound controller cannot submit valid group orders |
| GRP-07 | Dead entity deselects once and cannot return through cached weak pointer or group recall |
| GRP-08 | Stop/replace on one unit does not corrupt another's route episode |
| GRP-09 | Rematch clears group storage, old generations, pending orders and visuals |
| GRP-10 | Non-20 Hz render schedules produce equivalent per-unit authoritative event traces for selected test case |
| GRP-11 | Mouse drag/pan, modal focus and controller/touch gesture conflicts fail safely |
| GRP-12 | Partial admission feedback remains legible in a rendered Linux Development package |
| GRP-13 | Ctrl+0–9 replaces group; Shift+0–9 unions without stealing; Alt+0–9 transfers only selected IDs; repeated operations are stable and idempotent |
| GRP-14 | Overlapping slots share IDs without duplicate command requests or any formation/Discipline bonus |
| GRP-15 | Slot recall and rapid double-tap camera focus make no changes to orders, stances, movement, resources or authority |
| GRP-16 | Empty, dead, captured, revoked, temporarily inoperable and next-generation references have distinct documented selection/dispatch outcomes |
| GRP-17 | Ctrl+click/double-click same-archetype selection only finds eligible visible on-screen units; Tab cycles only valid selected subgroups |
| GRP-18 | DPI-independent click/drag threshold, partially intersected proxy, drag reversal, overlap, camera movement and UI/modal capture pass deterministic selection fixtures |
| GRP-19 | Keyboard, keymap-help and camera inputs have no collisions; current A/S double-binding is eliminated before accepted control groups; F remains infantry sprint unless explicitly remapped |
| GRP-20 | Mixed combat/noncombat or mixed-ability group commands show actual eligible/accepted/rejected/unsupported counts; one issuer's failure never revokes another's route |
| GRP-21 | 0–9 group bar, counts, selected state, partial failures and hotkey hints are legible and clickable in a cooked Linux Development gameplay pass; no unimplemented icon implies functionality |
| GRP-22 | Touch/controller alternatives preserve semantic selection and order authority with independent device-profile qualification; no false cross-platform acceptance |

**Stage alignment:** 4A first closes stale selection/authority negatives; **4C baseline** adds GRP-01–12 needed for the existing first-playable grouped interaction. **4C-G (mandatory core-production follow-on)** delivers GRP-13–21 as a bounded increment immediately after stable 4C multi-selection; it should be scheduled before wider tactical/formation expansion and eventual production readiness, **without retroactively changing 4F's historically defined First Playable Skirmish gate**. Queued waypoints, FORM-F2+ formations/Cohesion, broad auto-group rules and cross-platform device qualification remain independently evidenced. No code, runtime USTRUCTs/assets or plugin dependency is declared implemented by a design document.
