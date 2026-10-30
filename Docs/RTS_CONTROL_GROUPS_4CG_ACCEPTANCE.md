# 4C-G — Control groups 0–9: acceptance record

**Date:** 30 October 2026 · **Scope:** the 4C-G increment (GRP-13 to GRP-21) of [Selection, Group Commands and Order Queue Contract](RTS_SELECTION_AND_GROUP_ORDERS.md), on the multiplayer skirmish test map at its owner cap of 20 units per participant. Formations, queued waypoints, automatic rally grouping, key rebinding and controller/touch input (GRP-22) are out of scope.

**Outcome: IMPLEMENTED; packaged control groups PASS (two runs, 17/17 each), Automation 38/38, regression matrix unchanged from the 4C baseline, group bar legible at 1920×1080 and 3840×2160.**

## A. What changed (commit `eac4e0e`)

- **Ten local slots, 0–9 (§2B).** Each slot is a sorted set of stable unit IDs owned by the local controller and bound to its participant and match generation. A slot never issues an order, grants authority or forms a formation.
  - **Ctrl+n** replaces slot n with the selection.
  - **Shift+n** adds the selection to slot n and leaves other slots alone, so a unit can sit in several slots.
  - **Alt+n** moves the selection: it is removed from every other slot, then added to n.
  - None of the three changes the current selection. An empty selection, or two modifiers at once, leaves every slot unchanged and says so.
- **Recall and focus.** **n** replaces the selection with slot n through the normal revalidating selection setter. A second press of n within 0.35 s also centres the camera on the group's centre; only the client-local camera pawn moves. An empty slot keeps the current selection and shows "Group n is empty".
- **Cleanup (§2D).** Every controller tick drops permanently invalid members (dead, destroyed, no longer this participant's commandable unit) from all slots at once. A new match generation, another participant, a match that is not Active, a revoked seat or Return clears all ten slots. The prototype has no temporarily inoperable unit state, so no member is held back as "affiliated but unavailable".
- **Group bar (GRP-21).** The HUD draws ten slots along the bottom edge in keyboard order (1…9, 0), each with its digit and member count; the slot equal to the current selection is outlined. A click on a slot acts like its digit key (with the held modifier), and a double click centres the camera. The bar captures clicks: no box selection starts on it and right-click, Q and E do nothing over it. On views narrower than the command panel plus the bar it shifts right of the command panel instead of overlapping it. A hint line names the keys.
- **Same-type selection (GRP-17).** Ctrl+click, or a double click within 0.45 s on the same own unit, selects that unit's own archetype among units inside the viewport that are not fog-hidden. It never scans hidden or off-screen units. Ctrl+click on ground or a foreign unit changes nothing.
- **Tab subgroups.** Tab / Shift+Tab cycle the selection's archetype subgroups, ordered by each archetype's lowest unit ID. Grenade (first unit of the subgroup), reload and sprint go to the active subgroup; move, attack, attack-move, stop and guard still go to the whole selection. The HUD names the active subgroup when the selection mixes archetypes. Tab never edits saved slots.
- **Keymap (GRP-19).** `PrototypeControlGroup0`–`9` (digit keys) and `PrototypeSubgroupCycle` (Tab) are `DefaultInput.ini` action mappings; each digit handler reads the modifier itself. `quality -- audit` (`input.bindings`) now also fails when one key chord drives two actions, in addition to the existing pan-axis rule. F stays Sprint; focus is the double tap.
- **Portable core.** `GroupSelection.h` adds `ControlGroups` (bind, assign, remove, prune), `RecallTapTracker`, `SubgroupOrder`/`CycleSubgroup` and the group-bar layout and hit test, with C++20 tests.
- **Unchanged.** `network_protocol` stays 2. Control groups are client-local; the server sees only the ordinary per-unit commands of a later order. No authority moved to the client.

## B. Verification

**Identity.** Packages from clean commit `eac4e0e` (`git_dirty=false`, `network_protocol` 2, `game_version` 0.1.0-dev.0). Client `dev-eac4e0e/…/Politika` `ac20fbd9d827742dc5215d2a0992ddcef0f8fedb97d517e83804317f5f9f0bf9`; server `server-eac4e0e/…/PolitikaServer` `908d8848f215c650ccef0e5334e40838a049edec9c06446a8df7d7a155241385`. Evidence: `/mnt/data/politika-qualification/CG-20261030/final-eac4e0e/`.

| Check | Result |
|---|---|
| Portable C++20 suite (`tools/core-test.sh`), including `ControlGroups`, `RecallTapTracker`, subgroup order and bar hit test | PASS |
| `quality -- audit` `input.bindings` (axis/order separation and one action per chord) | PASS |
| Editor build | exit 0 |
| Automation, all Politika tests | **38/38** (succeeded with the usual environment warnings), including the new `Politika.RTS.Multiplayer.ControlGroups`; `auto-all/` |
| Packaged `rts_multiplayer --control-groups`, two runs | **17/17 and 17/17**, identical verdicts |
| Packaged `rts_multiplayer --group-orders` (4C baseline) | 8/8 |
| Regression: unit-scale 5/10/20 × 1v1/2v2/3v1 | 73/73 |
| Regression: offline interaction (`rts_interaction`) and participant authority | PASS, PASS |
| Regression: dedicated 1v1, listen 1v1 | 23/23, 18/18 |
| Regression: dedicated and listen layouts 2v1/3v1/2v2, lag-loss 1v1/2v2 | 74 + 74 + 47 PASS; the only non-PASS lines are the same four planned NOT_RUNs as the 4C and `af643d0` baselines (no legacy client supplied; 1v1 has no allies) |
| Regression: Unaligned engagement, fog memory | 9/9, 7/7 |
| Rendered group bar, offline `RTSVerticalSlice` at 1920×1080 and 3840×2160 | legible, on screen, clear of the command panel (below) |

**Packaged control-group run** (dedicated 1v1, 20 units per seat, fixture held idle, client A rendered on Xvfb at 1280×720 and driven with real X keyboard and mouse input through xdotool; both runs identical):

| Verdict | What it shows |
|---|---|
| `CG-SETUP-BOX-20` | A drag over the view selects the 20 own units. |
| `CG-13-CTRL-REPLACE` | Ctrl+1 stores those 20 in slot 1. |
| `CG-SETUP-FIVE` | Five units picked by screen position and Shift+click; the selection is exactly those five. |
| `CG-14-OVERLAP` | Ctrl+2 stores the five in slot 2 while they stay in slot 1. |
| `CG-13-ALT-TRANSFER` | Alt+3 moves the five into slot 3 and out of slots 1 and 2. |
| `CG-13-SHIFT-UNION` | Shift+1 adds them back to slot 1 (20 members) and leaves slot 3 alone. |
| `CG-13-ASSIGN-KEEPS-SELECTION` | No selection change during Ctrl+2, Alt+3 and Shift+1. |
| `CG-15-RECALL` | 1 selects the 20 members of slot 1. |
| `CG-16-EMPTY-SLOT-KEEPS-SELECTION` | 7 (empty) keeps the 20-unit selection. |
| `CG-17-CTRL-CLICK-SAME-TYPE` | Ctrl+click on one rifleman selects all 20 visible own riflemen. |
| `CG-17-DOUBLE-CLICK-SAME-TYPE` | A double click does the same. |
| `CG-17-TAB-SINGLE-ARCHETYPE` | Tab with one archetype reports one subgroup and changes nothing. |
| `CG-15-DOUBLE-TAP-FOCUS` | After panning away, 3 3 moves the camera 731 cm to slot 3's centre (error 0.0 cm). |
| `CG-21-BAR-CLICK-RECALLS` | A click on bar slot 1 recalls slot 1 (`source=bar`). |
| `CG-21-BAR-CAPTURES-RIGHT-CLICK` | A right-click on the bar sends no order. |
| `CG-15-NO-ORDERS-FROM-GROUP-OPS` | Across every group operation above: 0 client intents, 0 server commands from participant 1. |
| `CG-14-ONE-REQUEST-PER-UNIT` | A right-click move with slot 1 recalled (its members also sat in slot 3 earlier): the server receives 20 intents from 20 distinct issuers, no duplicates; the client tallies 20 of 20 accepted. |

Screenshots (`control-groups-1/client-a-0*.png`): groups assigned with the bar counts, the panned-away view, the camera centred on group 3, the bar with slot 1 outlined after a bar click, and group 1 moving.

**Group bar at desktop resolutions.** The offline `RTSVerticalSlice` (packaged `eac4e0e`, Xvfb, real input) was run at 1920×1080 and 3840×2160: box select, Ctrl+1, Ctrl+4, then recall 1. Both frames (`CG-20261030/hud/hud-1920x1080.png` `7477ea2c…0036`, `hud-3840x2160.png` `40962fd1…3554`) show the ten slots in keyboard order centred along the bottom edge, fully on screen and clear of the command panel. Slots 1 and 4 read 1 and are outlined, because the single selected unit is in both, and the other slots read "-". The 4K frame is the same layout at twice the scale (DPI scaling), so digits, counts and the hint line keep the same apparent size. The hint line is the smallest text on the HUD, about the size of the command-panel key labels; it is readable at both resolutions but would benefit from the UI-C text-scale setting. At 1280×720 the bar shifts right of the command panel (packaged run above).

## C. Not covered

- Explicit removal from one slot in the group UI (the portable `Remove` exists; no gesture is bound).
- Mixed-archetype Tab cycling in a packaged session: the skirmish map has one archetype per seat, so the packaged run sees one subgroup. The `ControlGroups` Automation test covers two archetypes, subgroup reload and wrap-around.
- Dead-member pruning and generation clearing in a packaged session (Automation covers both).
- Key rebinding and a conflict UI (UI-C); controller and touch alternatives (GRP-22); DPI scales other than 1 on a physical display.
- Formation spacing: resolved by FORM-F2 ([record](RTS_FORMATION_F2_GROUP_MOVE_ACCEPTANCE.md)); a recalled group now moves in Line or Column slots. 300-unit orders on large maps still need batched intents (F47).
