# 4C baseline — Multi-unit selection and group orders: acceptance record

**Date:** 30 October 2026 · **Scope:** the 4C baseline (GRP-01 to GRP-12) of [Selection, Group Commands and Order Queue Contract](RTS_SELECTION_AND_GROUP_ORDERS.md), on the multiplayer skirmish test map at its owner cap of 20 units per participant. Control groups (4C-G, GRP-13 to GRP-21), same-archetype selection, formations, queued waypoints and controller/touch input are out of scope.

**Outcome: IMPLEMENTED; packaged 20-unit group selection and orders PASS (two runs), Automation 37/37, regression matrix unchanged from the `af643d0` baseline.**

## A. What changed (commits `9ab0300` and `f4878c5`)

- **Selection is a set, not one unit.** Each participant's local selection is a sorted set of its own units (`FPolitikaSelectionSet`, stable-ID order). Every read revalidates it: dead, destroyed, foreign, Unaligned and unbound references drop out once, with their selection ring. The lowest-ID member is the primary unit for HUD details and grenade. Selection never grants authority.
- **Pointer gestures (§2C).** One left-button stroke applies one normalized operation:
  - a click selects the clicked own unit, or clears the selection on ground or a foreign unit;
  - a drag past a DPI-scaled 6-pixel threshold becomes a box. Release selects every own, living, presented unit whose projected clickable body intersects the box, so partly boxed units count;
  - Shift+click toggles one unit, Shift+drag adds, Ctrl+drag removes;
  - Esc during a drag, losing viewport focus, or a release whose binding never runs within 0.25 s cancels the stroke without selecting. Esc leaves the match only when no stroke is held. Right-click during a held stroke issues nothing. (The key state turns up before the release binding runs in the same frame, so the first packaged run cancelled every drag; `f4878c5` adds the grace.)
  - The box ends at the last cursor position sampled while the button was held. Releasing mouse capture moves the cursor back to the press point, so the release handler does not read it (found by the first packaged run, fixed in `f4878c5`).
  - The HUD draws the box while dragging.
- **Group orders (§3).** Right-click (move or attack), Q (attack-move), X (stop), G (guard), R (reload) and F (sprint) send one ordinary single-issuer intent per selected unit, each with its own client sequence and correlation, through the unchanged RPC and server intake. Each is admitted or rejected on its own; there is no group atomicity. Every member gets the same destination point; spacing belongs to later formation work. A grenade (E) is thrown by the primary unit only.
- **Outcome tally.** The client groups the correlations of one order into a batch and shows one line: "Move: 19 of 19 units accepted", or the accepted count plus the commonest rejection reason. Single-unit orders keep their existing feedback.
- **Pending bound.** The per-participant queue bound was a flat 16, which would have rejected part of a 20-unit order. It is now 16 plus the participant's living units (`politika::selection::PendingCommandLimit`, capped by the 300-unit game limit), so 36 at the test-map cap. A flood is still refused (`RateLimited`), now one past that bound.
- **Acknowledgement voice.** One acknowledgement line per world at a time (0.4 s gate), so a box selection or group order does not play twenty lines at once.
- **Portable core.** `Source/PolitikaCore/GroupSelection.h`: set operations (replace, union, toggle, subtract), drag threshold, rectangle intersection, the pending bound and the batch tally, with C++20 tests in `Tests/Core/RulesTests.cpp`. `Rules.h` gains `kMaxUnitsPerParticipant = 300` (the U-12 game limit, now shared with `UnitScale.h`) and `kOrderRejectionCount`.
- **Fixture flag.** `-PolitikaMPUnitScaleHold` (Development only, recognized by the bootstrap) keeps the unit-scale fixture idle so a packaged check can command the scaled armies itself.
- **Unchanged.** `network_protocol` stays 2: no RPC, replicated property or intent field changed. No authority moved to the client.

## B. Verification

**Identity.** Packages from clean commit `f4878c5` (`git_dirty=false`, `network_protocol` 2). Client `dev-f4878c5/…/Politika` `393bf370b7fd2a5ab051d7759ea354c0671ac53b834c57f0e3bd60628ca8d0f0`; server `server-f4878c5/…/PolitikaServer` `4c4c350e1b70ce3aba63538a3732b5ef54902ff68d92508756a7041fd3404374`. The packaged checks ran with the harness at `befc646`, whose game inputs (`Source`, `Content`, `Config`, `.uproject`) equal `f4878c5`; it only changes the right-click point. Evidence: `/mnt/data/politika-qualification/GRP-20261030/final2-f4878c5/`.

| Check | Result |
|---|---|
| Portable C++20 suite (`tools/core-test.sh`), including `GroupSelection.h` | PASS |
| Editor build | exit 0 |
| Automation, all Politika tests | **37/37** (succeeded with the usual environment warnings), including `Politika.RTS.Multiplayer.GroupSelectionAndOrders` and the updated queue-bound and stale-selection tests; `automation-f4878c5/` |
| Packaged `rts_multiplayer --group-orders`, two runs | **8/8 and 8/8** |
| Regression: unit-scale 5/10/20 × 1v1/2v2/3v1 | 73/73 |
| Regression: offline interaction (`rts_interaction`) and participant authority | PASS, PASS |
| Regression: dedicated 1v1, listen 1v1 | 23/23, 18/18 |
| Regression: dedicated and listen layouts 2v1/3v1/2v2, lag-loss 1v1/2v2 | 74 + 74 + 47 PASS; the only non-PASS lines are the same four planned NOT_RUNs as the `af643d0` baseline (no legacy client supplied; 1v1 has no allies) |
| Regression: Unaligned engagement, fog memory | 9/9, 7/7 |

**Packaged group-order run** (dedicated 1v1, 20 units per seat, fixture held idle, client A rendered on Xvfb at 1280×720 and driven with xdotool; both runs identical):

| Verdict | What it shows |
|---|---|
| `GRP-FIXTURE-20-PER-SEAT` | 20 units for participant 1 at the test-map cap, fixture `hold=1`. |
| `GRP-02-BOX-SELECTS-OWN-UNITS` | A drag over the whole view selects exactly the 20 own unit IDs (server ownership); 0 of 22 other units. |
| `GRP-TOGGLE-REMOVES-ONE` | Shift+click on unit 16 leaves exactly the other 19. |
| `GRP-11-ESC-CANCELS-DRAG` | Esc during a drag logs a cancel; the selection is unchanged and the client stays in the match. |
| `GRP-04-GROUP-ORDER-ADMITTED` | Right-click on the control point: 19 move intents, one per selected unit; the server receives exactly those 19 issuers with no intake rejection; the client tallies 19 of 19 accepted. The old flat bound of 16 would have refused 3. |
| `GRP-08-UNSELECTED-UNIT-UNTOUCHED` | Unit 16 receives no order. |
| `GRP-04-EVERY-MEMBER-MOVES` | All 19 enter Move; all 19 arrive (`arrived=19 obstructed=0 navigation_failed=0`). |
| `GRP-EMPTY-SELECTION-SENDS-NOTHING` | A click on empty ground clears the selection; X then sends no intent. |

Screenshots (`group-orders/client-a-0*.png`): the drag box, "20 UNITS SELECTED", "Move: 19 of 19 units accepted" with the MOVE row active, the 19 units capturing the control point with unit 16 left behind, and the cleared selection.

**Not run in packaged form:** Ctrl+drag subtract, Shift+drag union and the mixed-validity batch (one foreign issuer rejected alone). The portable suite and the `GroupSelectionAndOrders` Automation test cover them.

## C. Not covered

- Control groups 0–9, same-archetype selection, Tab subgroup cycling (4C-G).
- Formation spacing: every member moves to the same point, so a 19-unit group ends stacked on one spot (visible in `client-a-05-moved.png`). Destination spreading or FORM-F2 formations are the natural next slice.
- 300-unit selections on large maps: one reliable RPC per unit would approach the engine's reliable buffer (256 outstanding reliable calls per channel) at that scale. A batched intent (protocol change) or chunked submission is needed before large maps (finding F47).
- Controller and touch gestures (GRP-22); DPI scales other than 1 on a physical display; non-20 Hz render schedules (GRP-10).
- Economy, production queue and rematch parts of 4C.
