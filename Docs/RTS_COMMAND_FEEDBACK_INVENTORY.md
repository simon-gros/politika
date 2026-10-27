# RTS command and feedback inventory

**Status:** source-backed design inventory, 2026-10-27. Describes the current single-unit Linux Development slice and proposes presentation coverage; it does not claim that a command palette or final icon set exists.

## Current five-command inventory

| Order | Current player input | Current eligibility / target rule | Current accepted result | Current rejection / gap | HUD affordance to design |
|---|---|---|---|---|---|
| **Move** | Right-click map surface with a friendly unit selected. | Issuer must be owned by the local participant, alive/operational, movable; destination must be within valid map bounds. | Queued to the next 0.05-second step; then unit order becomes `Move`; global feedback reads “Move order accepted.” Selection HUD reports “Moving”. | A non-movable unit gets “That unit cannot move”; invalid destination reports “That destination is outside the map.” No path/blocked/interrupted result is implemented. | Destination/route cue plus a move symbol; show pending/accepted only when distinguishable from the unit's active order. |
| **Attack** | Right-click a unit. The hit target is submitted as an attack regardless of allegiance; rules decide validity. | Issuer must be owned/alive and able to attack; target must still exist, be alive, attackable, and hostile. | On admission, unit order becomes `Attack`; global feedback reads “Attack order accepted.” Selection HUD reports “Attacking”. | Friendly target: “Orders cannot target friendly units.” Other cases distinguish cannot attack, missing/dead target, or non-attackable target. No explicit targeting mode or valid/invalid hover preview. | Hostile-target reticle/engagement cue; friendly/invalid target must be visibly rejected and must never resemble accepted feedback. |
| **Attack-Move** | Press **A** with a selected unit; current cursor hit location is used as the destination. | Same issuer ownership/lifecycle/movement and map-bounds checks as Move. It does not require attack capability in current admission rules. | Order becomes `AttackMove`; the unit moves and can auto-acquire an enemy; global feedback reads “Attack-move order accepted.” Selection HUD reports “Attack-move”. | Non-movable issuer or invalid destination gets the corresponding move rejection. No dedicated armed/targeting mode, destination confirmation marker, or interruption feedback. | Distinct combined route + threat-awareness symbol, not just the Move icon with a color change. Explain that it travels while engaging encountered hostiles. |
| **Stop** | Press **S** with a selected unit. | Current prototype accepts for any owned, alive/operational issuer; there is no extra capability check. | Current order becomes `Stop`, destination is set to the unit's current position; global feedback reads “Unit stopped.” HUD reports “Stopped”. | Missing/unavailable/unauthorized issuer can be rejected by common admission checks. No “already stopped” suppression or group behavior. | Stop/pause-like symbol plus immediate movement cessation and persistent “Stopped” state. Avoid relying on a transient text message alone. |
| **Guard** | Press **G** with a selected unit. | Current prototype accepts for any owned, alive/operational issuer; there is no extra capability check. | Current order becomes `Guard`; global feedback reads “Guard order accepted.” HUD reports “Guarding”. | Common issuer/match rejection only. Current Guard implementation clears its target but does not define a guard radius, protected object, response policy, or an actual defensive stance. Treat behavior as prototype-only. | Shield/sector/hold cue may be explored, but the icon and tooltip must not promise protection radius or automatic response until those rules exist. |

Current rejection copy is defined in `DescribeRejection` in `PolitikaPrototypeGameplay.cpp`. Feedback is a single match-wide last-message string; it can be overwritten by a later admitted/rejected command and is not tied to a command ID, unit, world marker, or presentation event. The current HUD draws keyboard/mouse help, selected unit health/order, objective status, and that one feedback string. It has no five-button command panel, per-command icons, disabled reasons, hotkey focus states, selection-wide availability model, or explicit “command pending” state.

## Recommended presentation states

Use these as the design target for each shared order action; do not add presentation-only authority to the simulation.

| State | Meaning | Required presentation |
|---|---|---|
| **Unavailable** | No selection, or selected issuer does not meet the order's eligibility. | Disable/de-emphasize the action, state a short reason where known, and keep the input binding behavior consistent with the displayed availability. |
| **Ready** | Action may be issued immediately without a target (Move, Stop, Guard), or may enter a target phase. | Show original POLITIKA icon, localized label, current binding, and clear enabled state. |
| **Targeting** | A future explicitly armed targeted command is awaiting a valid target/location. | Show mode name, valid/invalid target feedback, and a reliable cancel action. Current prototype uses contextual clicks instead; it has no armed mode. |
| **Pending** | Submitted request has not yet been admitted at the next simulation step. | Optional short-lived acknowledgement only if latency or batching makes it perceptible; correlate it to the request so an older result cannot replace a newer command's state. Current global text is not correlated for the player. |
| **Accepted / active** | Request admitted and order is now the unit's current execution state. | Confirm near the affected unit and in the HUD; show persistent active order state. Current HUD gives selected-unit text and global acceptance copy, but no world destination marker. |
| **Rejected** | Request failed validation and did not replace the current order. | Give a concise reason and identify the affected issuer/target; distinguish rejection from accepted state. Current copy covers several reasons but only as one global string. |
| **Interrupted / completed** | Order stopped early or finished due to world state. | Explain meaningful cancellation/completion to the player and update active-order presentation. Typed authoritative events are available through the coordinator’s in-process native publication delegate; the HUD is not yet wired to consume them. Death cleanup currently clears attack orders. |

## Initial five-command icon/feedback checklist

- Give each action a stable POLITIKA-owned semantic ID; never use SC:R atlas coordinates or filenames as IDs.
- Keep label, localization key, icon asset, default input hint, eligibility query, tooltip, and accepted/rejected feedback references together in one presentation catalogue, or one authoritative shared definition used by both HUD and input.
- For this prototype, map current keyboard/mouse defaults: Move = right-click ground, Attack = right-click hostile, Attack-Move = A then current cursor position, Stop = S, Guard = G. These are not final cross-platform bindings.
- Show common commands for the selection and distinguish unavailable from valid. Since the current slice supports only one selected unit, mixed-selection policy remains deferred.
- Validate an original icon at target HUD sizes and against varied map backgrounds; pair silhouette with a label or text prompt and do not rely on color alone.
- Preserve the simulation boundary: ability checks and order admission remain in C++ rules/coordinator; HUD previews can explain known capability but cannot make an invalid order authoritative.
- Use the new typed event publication delegate as the source for a later event-driven HUD; add per-unit presentation correlation and identity/localization fields before extending beyond this prototype's one-line status. Do not make a widget poll all units every frame.

## Next bounded implementation slice

1. Create original placeholder icons and a five-row command panel for Move, Attack, Attack-Move, Stop, and Guard.
2. Derive enablement from selected unit capabilities and order semantics: Move/Attack-Move require movable; Attack requires attack capability plus a valid hostile target; Stop/Guard follow their current prototype admission behavior, with Guard clearly labeled as provisional until its gameplay semantics are designed.
3. Render action name, bound key, enabled/unavailable reason, and current active order. Keep right-click contextual behavior but give a distinct friendly-target rejection.
4. Add a destination/order marker only if supported by a stable presentation state; do not signal “accepted” before coordinator admission.
5. Review at 3840×2160 and 1920×1080, then handheld/touch layouts when those inputs are implemented.

## Evidence and ownership

- Input and HUD: `Source/Politika/Private/PolitikaPrototypeGameplay.cpp` (`SetupInputComponent`, `OrderUnderCursor`, `AttackMoveSelected`, and `APolitikaPrototypeHUD::DrawHUD`).
- Capabilities and current order state: `Source/Politika/Public/PolitikaPrototypeGameplay.h` and the same implementation file.
- Admission rules and rejection enums: `Source/PolitikaCore/Rules.h`.
- Package interaction evidence and scope: [RTS Vertical Slice milestone](RTS_VERTICAL_SLICE_MILESTONE.md) and [command/simulation contract](RTS_COMMAND_SIMULATION_CONTRACT.md).
- Product-level interaction rules: [POLITIKA UI Overview](POLITIKA_UI_OVERVIEW.md).

This checklist uses StarCraft Remastered only as a prompt to examine compact command discoverability. No SC:R images or data were copied or used as the proposed production design.
