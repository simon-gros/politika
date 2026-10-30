# FORM-F2 — Line and Column group-move geometry: acceptance record

**Date:** 30 October 2026 · **Scope:** the FORM-F2 increment of [Military Formations, Tactical Stances, Cohesion and Discipline](RTS_UNIT_FORMATIONS_AND_DISCIPLINE.md) §4 and §10: Line and Column geometry, facing, unique legal slots and bounded deterministic slot assignment for a selected group's Move and Attack-move. Tested on the multiplayer skirmish test map at its owner cap of 20 units per participant. Formation identity, membership, lifecycle, regroup, stances, adherence, Cohesion and Discipline effects (FORM-F3 to F8) are out of scope and do not exist.

**Outcome: IMPLEMENTED and ACCEPTED for the skirmish test map; packaged group-move-shape PASS (16/16, two runs), Automation 39/39, regression matrix unchanged from the 4C-G baseline.**

## A. What changed (commit `6540151`, harness fix `9bf1dc9`)

- **One destination per member (closes the 4C stacking limit).** A right-click Move or a Q Attack-move with two or more selected units now sends each unit its own point instead of the clicked point for all. A single unit still goes to the exact clicked point, and Attack, Stop, Guard and abilities are unchanged.
- **Line and Column.** **Line** (default) puts up to 10 units side by side per rank, centred on the clicked point; 20 units make two ranks of 10. **Column** puts units two abreast, front to back along the direction of travel; 20 units make ten ranks of 2. Slots are 150 cm apart. The block faces from the group's centre to the clicked point; when the group already stands on the point, it faces map +X.
- **Deterministic, crossing-free assignment.** Members are ordered front to back along the facing, then left to right inside each rank, and matched to the slots in the same order, so neighbours do not cross paths. Ties break by stable unit ID. The result depends only on the member set, never on selection order.
- **Legal slots and spare slots.** Each candidate is checked on the issuing client: inside the playable area with a 50 cm margin; static-world ground under it within 120 cm of the clicked point's height and no steeper than about 41°; and, where the process has navigation (standalone, listen host), projected onto the navmesh within the server's own 150 cm threshold. A rejected candidate is replaced by the next slot in full ranks behind the block. The client checks at most two candidates per member. If no legal slot is left, the rear-most members keep the plain shared-point order rather than a bad slot.
- **Authority unchanged.** The check is advisory. Every destination is still an ordinary single-issuer `MoveOrder`/`AttackMoveOrder` that the server admits, projects and routes on its own, and each member is accepted or rejected individually. `network_protocol` stays 2. There is no group atomicity and no formation state.
- **Input and HUD.** **V** cycles Line and Column (`PrototypeGroupMoveShape` in `DefaultInput.ini`). The choice is local to the player, sends nothing and survives until changed. The help line shows `V group shape: LINE`/`COLUMN`, and the latest group move's destinations are drawn as small green rings for 2.5 s.
- **Portable core.** `Source/PolitikaCore/FormationGeometry.h`: `RelativeSlots`, `TravelFacing`, `PlanGroupMove` (with a caller-supplied legality check) and `MinimumSeparation`, with C++20 tests.
- **Development logs.** `RTS_GROUP_MOVE_SHAPE`, `RTS_GROUP_MOVE_PLAN` (shape, anchor, playable bounds, facing, candidates checked/rejected, unplaced, minimum separation) and one `RTS_GROUP_MOVE_SLOT` per member (map and world point).

## B. Verification

**Identity.** Packages from clean commit `6540151` (`git_dirty=false`, `network_protocol` 2, `game_version` 0.1.0-dev.0). Client `dev-6540151/…/Politika` `bf0143120ebce9a5092e501663a2a27870e556035a6f1724ae12da986c11aff7`; server `server-6540151/…/PolitikaServer` `af952d5c6e84ad79ad67a2bd04a47328bb89ec4fe2179ce52166d1af636343e6`. The harness fix `9bf1dc9` changes only `Tools/Shell`. Evidence: `/mnt/data/politika-qualification/FORM-F2-20261030/final-6540151/`.

| Check | Result |
|---|---|
| Portable C++20 suite (`tools/core-test.sh`): slot layouts, facing, centring, crossing-free and order-independent assignment, blocked-slot replacement, probe bound, single member, malformed input, 300-member cap | PASS |
| `quality -- audit` (`input.bindings`: 22 order bindings, V shares no key) | PASS |
| Editor build | exit 0 |
| Automation, focused: new `Politika.RTS.Multiplayer.GroupMoveShape`, `GroupSelectionAndOrders`, `ControlGroups` | 3/3 (GroupMoveShape warnings are the expected `NavigationUnavailable` lines: the test world has no navmesh) |
| Automation, all Politika tests (`auto-all/`) | 39/39 (38 earlier + `GroupMoveShape`); warnings identical per test to the 4C-G baseline except the new test's 42 expected `NavigationUnavailable`/`RTS_NAVIGATION_FAILED` lines (no navmesh in the Automation world) |
| Packaged `rts_multiplayer --group-move-shape`, two runs | 16/16 and 16/16 |
| Regression matrix (`final-6540151/summary.txt`) | Unchanged from the 4C-G baseline: group orders 8/8, control groups 17/17, unit-scale 73/73, offline PAC-10 and participant authority PASS, dedicated 1v1 23/23, listen 1v1 18/18, dedicated and listen layouts 74 + 74, lag-loss 47, engagement 9/9, fog memory 7/7. The only non-PASS lines are the four planned NOT_RUNs (no legacy client supplied ×3, no allies in 1v1). |

**Packaged group-move-shape run** (dedicated 1v1, 20 units per seat, fixture held idle, client A rendered on Xvfb at 1280×720 and driven with real X keyboard and mouse input through xdotool):

| Verdict | What it shows |
|---|---|
| `FORM-FIXTURE-20-PER-SEAT`, `FORM-01-BOX-SELECTS-OWN-UNITS` | A drag selects exactly the 20 own units. |
| `FORM-04-LINE-PLAN`, `FORM-04-LINE-SHAPE` | Right-click east of the start: 20 slots for the 20 units, 150.0 cm apart, 1 350 cm across and 150 cm deep (two ranks of ten), no candidate rejected. |
| `FORM-05-LINE-SERVER-ROUTES` | The server received 20 move intents; each was projected and routed at the exact point the client sent (within 1 cm), resolved points 150.0 cm apart; 20 of 20 accepted. |
| `FORM-LINE-ARRIVED-SPACED` | 20/20 arrived, 0 navigation failures, every unit within 30 cm of its resolved slot, final minimum separation 149.3 cm. |
| `FORM-02-SHAPE-KEY-IS-LOCAL` | V switches to Column and sends no intent; the server receives nothing. |
| `FORM-04/05-COLUMN-*`, `FORM-COLUMN-ARRIVED-SPACED` | Column west: 150 cm across, 1 350 cm deep (ten ranks of two); same server and arrival checks; final minimum separation 149.4 cm. |
| `FORM-SHAPE-CYCLES-BACK-TO-LINE` | A second V returns to Line. |
| `FORM-04/05-EDGE-*`, `FORM-EDGE-ARRIVED-SPACED` | Line ordered 100 cm inside the western playable edge: 8 candidates outside the playable area were replaced by spare slots behind, all 20 slots in bounds and 149.9 cm apart, 20/20 routed and arrived, final minimum separation 149.5 cm. |
| `FORM-MATCH-STAYS-ACTIVE` | No objective capture or terminal phase during the run. |

Screenshots (`group-move-shape-1/client-a-*.png`): the destination rings just after the Line order, the arrived two-rank line, the HUD reading `V group shape: COLUMN`, the arrived two-file column and the block at the western edge.

The matrix run was interrupted by a session stop during the engagement step; every earlier step had completed. The partial engagement evidence is kept as `engagement-interrupted/`, and `resume-matrix.sh` reran engagement and fog memory on the same packages before full Automation.

The first attempt (`group-move-shape-attempt0-objective-capture/`) ordered the Line onto the control point; the 20 parked units captured it and ended the match while the Column was moving, so the Column arrival verdict failed. Its Line verdicts all passed. The harness now keeps every destination at least 500 cm from the control point and checks that the match stays Active.

## C. Map to the FORM acceptance matrix

| ID | Status in this increment |
|---|---|
| FORM-02 | V and the plan create no membership, stance, Cohesion or bonus; packaged V sends nothing. |
| FORM-04 | Unique, finite, centred slots with facing; deterministic and order-independent assignment (core tests, Automation, packaged). |
| FORM-05 | Each member's point is projected and routed by the server under its own movement; out-of-bounds candidates are replaced, never sent; no navigation failure in the packaged runs. A slot the client cannot see as blocked (a dedicated client has no navmesh) still reaches the server, which fails only that member as before. |
| FORM-06 | Each member is admitted on its own (Automation tally, packaged 20 of 20). |
| FORM-18 | Rendered cooked Linux Development client shows placement and readable outcomes (screenshots). |
| FORM-01, 03, 07–17, 19, 20 | Not in scope: they need formation identity, lifecycle, stances or Cohesion (FORM-F3+), or scale/device work. |

## D. Not covered

- **Facing and width control.** The facing always follows the travel direction; there is no right-drag to set facing or frontage, and no formation preview before the click.
- **Blocked slots on a dedicated client.** A dedicated client has no navmesh, so its pre-check uses bounds and ground only. A slot on ground the navmesh excludes (for example a null-area modifier) still reaches the server and fails that one member with `NavigationFailed`. The skirmish map has no such area in the tested region; the listen host and standalone game use the navmesh check.
- **Mixed movement classes and footprints.** One spacing (150 cm) for all members; the map has only infantry.
- **Speed matching.** Members move at their own speed and do not wait for each other. Regroup after casualties with hysteresis was added by FORM-F3 ([record](RTS_FORMATION_F3_LIFECYCLE_ACCEPTANCE.md)) for explicit formations only.
- **First FORM fixture items that need F3:** member loss during transit, explicit formation Stop and match-return cleanup of formation state — covered by the [FORM-F3 record](RTS_FORMATION_F3_LIFECYCLE_ACCEPTANCE.md).
- **Attack-move** uses the same planner but was not driven in the packaged run (Automation covers Move only; the code path is shared).
- 300-unit orders on large maps still need batched intents (F47).
