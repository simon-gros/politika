# POLITIKA — Selection, Group Commands and Order Queue Contract

**Status: PROPOSED design, 29 October 2026.** The current prototype implements one selected unit and five typed orders. This document owns proposed selection-set and multi-issuer semantics; [command contract](RTS_COMMAND_SIMULATION_CONTRACT.md) owns existing `CommandRequest` and coordinator behavior, [Stage 4A](STAGE4A_MATCH_LIFECYCLE_ACCEPTANCE.md) owns authority proof, [UI](POLITIKA_UI_OVERVIEW.md) owns widget/gesture design and [Engine Outline](ENGINE_OUTLINE.md) owns staging. Do not infer implementation or change the accepted order variant as part of this document.

## 1. Selection is presentation-adjacent state, not command authority

A local selection set is a normalized collection of **runtime entity IDs paired with match generation**, in deterministic order; it holds neither transient UObject pointer authority nor faction ownership permissions. Selection is allowed only while a local controller has a valid participant binding. On every read and immediately before group submission, revalidate that each entity still exists, is alive/operational, selectable, belongs to the controlling participant and, when an authored roster exists, matches admitted faction/source permissions.

Invalidated selection is retired, its visual indicator removed and selection-changed feedback emitted **once** per meaningful change; avoid treating a prior weak pointer as valid because it was once selected. A dead unit cannot remain in the roster after cleanup. A neutral/unbound/hostile unit cannot be selected via forged ID, drag rectangle, control-group recall or post-rematch stale context.

## 2. Canonical selection operations (candidate)

| Intent | Transition and checks |
| --- | --- |
| Single click | Set = eligible owned unit, or clear if empty/invalid hit; inspect enemy separately from selection |
| Add/toggle modifier | Union/toggle valid owned IDs only; de-duplicate and sort by stable ID |
| Drag-box | Collect owned eligible units, filter target visibility, then normalize once before changing visuals |
| Select all of same archetype (later) | Apply only within an explicit authored/visible/eligible region; never a hidden-world unit scan |
| Save control group (later) | Store IDs + match generation per local participant; no implicit command authority |
| Recall group (later) | Revalidate every ID; drop dead/captured/foreign/stale; empty group gives explicit feedback |
| UI/world focus | Selecting via minimap, roster or keyboard uses the same validated selection method |
| Return/Terminal/new match | Invalidate all selection/control-group buffers, visuals and input latched states |

**No group-atomicity by default.** A group command is a batch of single-issuer typed requests: each retained issuer has its own correlation/sequence and independent admissibility; one invalid unit must not silently invalidate every legitimate command, and one admitted request must not retroactively authorize another. The UI should aggregate outcomes (e.g., two accepted, one rejected) without inventing an all-or-nothing transaction.

## 3. Authoritative group dispatch

1. Controller maps input to one semantic intent and validated local selection snapshot. No UI may call private trusted numeric-`PlayerId` command helpers.
2. Match coordinator revalidates controller→participant, match generation and unit owner/faction; takes a deterministic sorted ID snapshot.
3. For each unit: construct an ordinary `CommandRequest` with that unit's stable issuer ID, new sequence and correlation; queue it through the existing intake. Commands are admitted at the next authoritative 0.05 s step.
4. Derive destination offsets from the existing map-space coordinate contract. Initial group Move sends the **same requested point** per issuer; any group spread/formation spacing is a later separately validated feature and must still project legal route endpoints.
5. Apply existing order-replacement semantics per unit, retain independently rejected prior active routes, and publish per-unit typed outcomes. Do not introduce per-actor `Tick` movement or second scheduler.
6. At result/return/respawn or generation mismatch, cancel group handles/queued requests without replaying stale commands.

**Queueing is deferred.** Shift-queued waypoints and build queues are different processes and must have distinct semantics. For future per-unit order chains, lock bounded capacity, valid command kinds, queue append/replace behavior, cancellation/stop, path failure, target loss, death and item-level feedback before writing code; never reinterpret current single active `Order` as an implicit FIFO.

## 4. Input and accessibility grammar

Desktop pointer: click-select; box-select only after movement threshold from tactical-world start; cancel box on Esc, modal input capture, focus loss and active targeting mode. Explicitly distinguish pan from box and avoid drag issuing a move order. Controller: semantic select/confirm/cancel and inspect/command context (consider focus/cursor hybrid only after usability tests). Touch: start with tap-select and explicit action/target mode, long-press inspected separately; multi-selection must not conflict with map pan/pinch. Each device maps to identical semantic orders, not identical gestures.

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

**Stage alignment:** 4A first closes stale selection/authority negatives; **4C** adds only GRP-01–05/06/07/09/11/12 as needed for the playable grouped interaction. Persistent control groups, waypoint queues, formations and multi-device qualification are separate work after 4F or when a bounded scenario demands them. No new runtime USTRUCTs/assets or plugin dependencies are committed by this specification.
