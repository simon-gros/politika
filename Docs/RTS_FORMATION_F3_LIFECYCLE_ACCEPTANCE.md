# FORM-F3 — Match-owned formations: identity, membership, lifecycle and bounded regroup: acceptance record

**Date:** 30 October 2026 · **Scope:** the FORM-F3 increment of [Military Formations, Tactical Stances, Cohesion and Discipline](RTS_UNIT_FORMATIONS_AND_DISCIPLINE.md) §3, §7 and §10: a formation ID owned by the match, explicit membership, Form and Release, formation-addressed Move, Attack-move and Stop, individual member overrides, member loss, one bounded regroup and match-end cleanup. Tested on the multiplayer skirmish test map at its owner cap of 20 units per participant. Stances, adherence, Cohesion and Discipline effects (FORM-F4 to F8) are out of scope and do not exist.

**Outcome: IMPLEMENTED and ACCEPTED for the skirmish test map; packaged formation-lifecycle PASS (20/20, two runs with identical step timelines), Automation 40/40, regression matrix unchanged from the FORM-F2 baseline, and the protocol-2 client is now refused.**

## A. What changed (commit `e2c4488`, harness fixes `3ddade4` and `4cfcfa3`)

- **Formations are match state, not selection state.** A formation exists only after the server admits an explicit **Form**. Selection, control groups and the V shape choice never create one. The match keeps one registry per match generation (`Source/PolitikaCore/Formation.h`, `politika::formation::Registry`); formation IDs are their own namespace, only increase within a match and are never reused after a formation dissolves.
- **Form (T).** Forms the selected units (2–64, all own, alive and mobile) into one formation in the current V shape (Line or Column). A unit belongs to at most one formation: members of another formation move into the new one, and a formation left with one member dissolves. A participant can hold at most 10 formations. Forming sends no unit order; the units stay where they are until the first formation move.
- **Release (Y).** Removes the selected units from their formations; a formation left with fewer than two members dissolves. Y on units that are in no formation sends nothing. Release covers the design's Detach and Disband.
- **Formation-addressed orders.** When the selection is exactly one whole formation (every living member, nothing else), RMB ground, Q and X send **one** formation Move, Attack-move or Stop. The server plans the block with the FORM-F2 geometry using the formation's stored shape (not the player's current V choice), checks each slot on its own navmesh and ground, and submits one ordinary `MoveOrder`/`AttackMoveOrder`/`StopOrder` per member. Each member order is admitted on its own exactly like a 4C group order and tallied under the request's correlation. Any other selection (part of a formation, a formation plus other units, two formations) keeps the FORM-F2 per-unit batch.
- **Individual overrides (FORM-07).** An individual order to a member (for example X on one unit recalled by its control group) is an ordinary order. The unit stays a member but becomes *independent*: later regroups leave its route alone. The next formation Move makes it follow again.
- **Member loss and one bounded regroup (FORM-08/09).** A member that dies or leaves its owner's control leaves the formation, which turns *Degraded*. If the formation has a destination and members still following it, one regroup is scheduled 40 authoritative steps (2 s) later. Further losses inside that window share the same regroup, and two regroups of one formation are at least 40 steps apart. The regroup re-plans the still-following members towards the same anchor with the same facing and shape and re-sends only members whose slot moved (≥ 1 cm). It closes the ranks and the formation is *Organized* again. There is no per-frame re-planning and no regroup without a new loss.
- **Formation Stop** stops every member and forgets the destination, so a later loss schedules nothing.
- **Lifecycle (FORM-15).** Leaving the Active phase (Terminal or Return) and match teardown retire every formation once, clear the owner view and refuse later formation requests; a stale match generation is refused.
- **Owner-only view.** Each player controller replicates only its own participant's formations (ID, shape, state, members, following count, regroups, whether it has a destination). An opposing client receives nothing. The HUD shows `FORMATION n · LINE/COLUMN · members · ORGANIZED/DEGRADED` when a whole formation is selected, an `F<n>` tag on each member's label, the latest outcome message, and `T form · Y release` in the help line.
- **Rejections.** A separate `FormationRejection` code set (so the `OrderRejection` codes do not move): malformed, duplicate, rate-limited (the shared 4C pending-command bound), match inactive, stale match, unauthorized, too few / too many / duplicate members, member unavailable, unknown formation (also used for another participant's formation, so it is not an existence oracle), formation limit, not a member and invalid destination. A rejected request changes nothing.
- **Network.** One new server RPC carries a bounded formation intent (op, formation ID, shape, member IDs, point) and one client RPC its outcome. `network_protocol` moves from 2 to 3; a protocol-2 client is refused at join (`matrix-none` legacy-client check).
- **Input.** `PrototypeFormFormation` (T) and `PrototypeReleaseFormation` (Y) in `DefaultInput.ini`; `quality -- audit` `input.bindings` reports 24 order bindings and no shared chord.
- **Development logs.** `RTS_FORMATION_SENT`, `_OUTCOME`, `_FORMED`, `_MOVE`, `_MEMBER_INDEPENDENT`, `_MEMBER_LEFT`, `_REGROUP_SCHEDULED`, `_REGROUP`, `_DISSOLVED`, `RTS_FORMATIONS_CLEARED` and the client's `RTS_FORMATION_VIEW`. The Development-only fixture switch behind the packaged casualty logs `MP_FORMATION_CASUALTY_ARMED` and `MP_FORMATION_CASUALTY`.

## B. Verification

**Identity.** Packages from clean commit `3ddade4` (`git_dirty=false`, `network_protocol` 3, `game_version` 0.1.0-dev.0). `3ddade4` is the feature commit `e2c4488` plus a `Tools/Shell`-only harness fix, so the game source is identical. Client `dev-3ddade4/…/Politika` `725bbec6af2bace9f8172c4b4dcafd3304480eb6e63be1cce9a7c47e2ee4ff56`; server `server-3ddade4/…/PolitikaServer` `35445117b5c4d2f8650ae53f7776613cc3a713520b0b5813460e1dfcf0108353`. Evidence: `/mnt/data/politika-qualification/FORM-F3-20261030/final-3ddade4/`.

| Check | Result |
|---|---|
| Portable C++20 suite (`tools/core-test.sh`): intent shape checks for every op, Form/Release/loss transitions, one formation per unit, dissolve below two members, formation limit, regroup coalescing and spacing, regroup re-sends only moved slots, ID monotonicity after Clear | PASS |
| `quality -- audit` (`input.bindings`: 24 order bindings, no shared chord) | PASS |
| Editor build | exit 0 |
| Automation, focused: new `Politika.RTS.Multiplayer.FormationLifecycle` plus `GroupMoveShape`, `GroupSelectionAndOrders`, `ControlGroups` | 4/4 at the feature source (`auto-focused-1` caught a Form failure during development; `auto-focused-2` is the rerun after the fix that went into `e2c4488`) |
| Automation, all Politika tests (`auto-all/`) | 40/40 (39 earlier + `FormationLifecycle`); warnings identical per test to the FORM-F2 baseline. The new test has 14 warnings: 13 expected `RTS_NAVIGATION_FAILED reason=NavigationUnavailable` (the Automation world has no navmesh) and one SDL start-up line |
| Packaged `rts_multiplayer --formation-lifecycle`, two runs | 20/20 and 20/20; every authoritative step number identical between the runs (Form 203, casualty 255, Release 588, re-Form 638, Terminal clear 649) |
| Regression matrix (`final-3ddade4/summary.txt`) | Unchanged from the FORM-F2 baseline: group-move shape 16/16, group orders 8/8, control groups 17/17, unit-scale 73/73, offline PAC-10 and participant authority PASS, dedicated 1v1 23/23 (two runs, see below), listen 1v1 18/18, listen layouts 74, lag-loss 47, engagement 9/9, fog memory 7/7. Dedicated layouts now 75/75: because `network_protocol` moved to 3, the run supplied the FORM-F2 protocol-2 client and `MP2-PROTOCOL-LEGACY-REFUSED` passes (`MP_PRELOGIN REJECTED … protocol=2 … MP.PROTOCOL.MISMATCH`) instead of being NOT_RUN. The only non-PASS lines are three planned NOT_RUNs (legacy client not supplied to listen layouts and lag-loss, no allies in 1v1). |

**Packaged formation-lifecycle run** (dedicated 1v1, 20 units per seat, fixture held idle, client A rendered on Xvfb at 1280×720 and driven with real X keyboard and mouse input through xdotool):

| Verdict | What it shows |
|---|---|
| `FORM-FIXTURE-20-PER-SEAT`, `FORM-01-BOX-SELECTS-OWN-UNITS` | A drag selects exactly the 20 own units. |
| `FORM-SETUP-GROUPS`, `FORM-02-GROUPS-CREATE-NO-FORMATION` | Control groups 1 (all 20) and 2 (unit 1) are assigned; no formation exists. |
| `FORM-F3-FORM-ADMITTED`, `FORM-02-FORM-ISSUES-NO-ORDER` | T sends one Form; the server forms formation 1 (Line, 20 members) and the client's replicated view shows it. No unit order is sent. |
| `FORM-F3-OWNER-ONLY` | The opposing client logs no formation view. |
| `FORM-04-SERVER-PLAN`, `FORM-06-MEMBER-ORDERS-ADMITTED-INDIVIDUALLY` | RMB with the whole formation selected sends one formation Move; the server plans 20 slots 150.0 cm apart, nothing rejected, and admits 20 member Moves individually under one correlation. The client sends no unit intents. |
| `FORM-07-INDIVIDUAL-STOP-KEEPS-MEMBERSHIP` | Group 2 recall + X stops unit 1 with an ordinary order; it stays a member, now independent (19 following). |
| `FORM-09-LOSS-DEGRADES-AND-SCHEDULES-ONE-REGROUP` | The fixture kills unit 22 in transit: it leaves (`destroyed`), the formation turns Degraded and one regroup is scheduled 40 steps later, then runs: 18 following members re-planned, 13 whose slot moved re-sent. |
| `FORM-07-REGROUP-LEAVES-INDEPENDENT-MEMBER`, `FORM-07-OTHER-MEMBERS-UNAFFECTED` | The regroup orders exclude unit 1; no non-formation order reaches a following member. |
| `FORM-08-ARRIVED-SPACED-AFTER-ONE-REGROUP` | 18/18 followers arrive within 30 cm of their slot, 0 navigation failures, minimum separation 148.2 cm, exactly one regroup. |
| `FORM-F3-CLIENT-VIEW-AFTER-REGROUP` | The owner view reads `organized 19/20 f18 r1`. |
| `FORM-10-SECOND-MOVE-USES-FORMATION-SHAPE` | A second formation Move (west) re-plans the 19 members in Line; 19/19 arrive at their slot, minimum separation 149.4 cm; unit 1 follows again. |
| `FORM-09-RELEASE-DISSOLVES` | Y on the whole formation dissolves it (`released`); the owner view is empty and the next RMB is an ordinary FORM-F2 per-unit batch. |
| `FORM-F3-REFORM-NEW-ID` | After V → Column, T forms formation 2: a new ID, never 1 again. |
| `FORM-MATCH-STAYS-ACTIVE` | No objective capture or terminal phase during the run. |
| `FORM-15-TERMINAL-RETIRES-ONCE` | Ending the match clears formation 2 once (`match_ended`); no formation line follows. |

Screenshots (`formation-lifecycle-1/client-a-*.png`): formation 1 formed, the closed Line after the regroup with the independent unit 1 apart, the HUD with the whole formation selected and formation 2 in Column.

**Dedicated 1v1 menu capture race.** In the matrix run, dedicated 1v1 failed one check: `client-a-05-menu.png` was a blank 250-byte frame, and the session stopped before its last six verdicts. The harness took the post-results menu capture one second after the server's reset. Each client, though, schedules its rematch travel three seconds after it reaches the menu, which falls at nearly the same moment. In the FORM-F2 and 4C-G runs the capture came 7 ms and 36 ms before the rejoin's map load; here it came 150 ms after it. The server's reset-to-ready time was the same as before (4.0 s). This is a harness race that predates FORM-F3. Harness fix `4cfcfa3` (Tools/Shell only) captures the menu one second after both clients reach it. Two reruns against the same packages passed 23/23 and 23/23 with normal menu captures (`regression-dedicated-1v1-1/`, `-2/`). The failed run is kept as `regression-dedicated-1v1-attempt0-menu-race/`.

**Superseded attempt.** The first qualification at the feature commit (`superseded-e2c4488/`) had two problems. The harness waited for `MP_FORMATION_CASUALTY` and matched the earlier `MP_FORMATION_CASUALTY_ARMED` banner, so it read `unit=0` and failed waiting for that unit to leave (10 PASS before it); the rerun with the corrected match passed 22/22 lines. The client package also carried `git_dirty=true` because documentation was edited while it packaged. Both were rebuilt from clean commit `3ddade4`, and all evidence above is from those packages.

## C. Map to the FORM acceptance matrix

| ID | Status in this increment |
|---|---|
| FORM-01 | Foreign, unknown and enemy-issued Form/Move are refused (Automation); the opposing client sees nothing (packaged). Unaligned units are never a participant's own units, so they cannot be members. |
| FORM-02 | Selection, control groups and V create no formation; Form issues no unit order (Automation, packaged). |
| FORM-03 | Bad shape, duplicate or zero IDs, size bounds, older protocol and stale generation fail closed (core, Automation). Content-defined shapes do not exist yet. |
| FORM-04 | Server plan uses the stored shape: unique slots, 150 cm spacing, facing (Automation, packaged). |
| FORM-05 | Each member's slot is checked on the server's navmesh and ground before submission and then admitted, projected and routed as an ordinary order; 0 navigation failures in the packaged runs. |
| FORM-06 | Member orders are admitted individually and tallied under one correlation (Automation 8 of 8, packaged 20 of 20). |
| FORM-07 | An individual Stop keeps membership, makes the member independent and does not touch other members; regroups skip it (Automation, packaged). |
| FORM-08 | Exactly one regroup per loss window, only moved slots re-sent, no further regroup without a new loss (core, Automation, packaged). |
| FORM-09 | Death, Release and re-Form update membership; dissolve below two members (core, Automation, packaged). Capture, role change, reinforcement and topology changes do not exist yet. |
| FORM-15 | Terminal retires every formation once and refuses later requests; stale generation refused (Automation, packaged). |
| FORM-16 | Two packaged runs produced identical authoritative step numbers for every formation event. This is a same-machine schedule check, not cross-machine proof. |
| FORM-18 | Rendered cooked Linux Development client shows formation state and readable outcomes (screenshots). |
| FORM-10–14, 17, 19, 20 | Not in scope: stances, adherence, Cohesion, faction effects, AI and scale/device work (FORM-F4+). |

## D. Decisions taken for §12 (provisional)

- **Size:** 2–64 members per formation, at most 10 formations per participant, one formation per unit.
- **Regroup hysteresis:** 40 authoritative steps (2 s); re-send threshold 1 cm.
- **Facing on regroup:** fixed to the last formation move; the anchor does not follow the survivors.
- **Shape:** stored at Form; V only affects the next Form or the next non-formation group move.
- **Save/net schema:** formations are not saved; `network_protocol` 3.

## E. Not covered

- **Speed matching and waiting.** Members move at their own speed; the formation does not hold its shape in transit, only at the destination.
- **Reinforce** (adding units to an existing formation without forming a new one), facing/width drag and a pre-click preview.
- **Packaged Attack-move and formation Stop.** Both go through the same server planner and are covered by Automation (Stop) and the shared path (Attack-move); the packaged run drives Move only.
- **Partial-formation selections** are ordinary per-unit batches; there is no "select whole formation" gesture besides control groups or box selection.
- **HUD wording.** The selected-formation line reported `N in formation` for members currently following the formation's move; directly after Form it read `0 in formation`. Fixed in FORM-F4: it now reads `N following its move` ([FORM-F4 record](RTS_FORMATION_F4_STANCE_ACCEPTANCE.md)).
- **Dedicated-client navmesh precheck** for non-formation group moves (FORM-F2 limit) is unchanged; formation moves are checked on the server.
- 300-unit orders on large maps still need batched intents (F47); a 64-member formation move is 64 reliable member orders on the server side but one client RPC.
