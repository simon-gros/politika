# FORM-F6 (part two) — Wedge and Echelon shapes: acceptance record

**Date:** 31 October 2026 · **Scope:** the second FORM-F6 increment of [Military Formations, Tactical Stances, Cohesion and Discipline](RTS_UNIT_FORMATIONS_AND_DISCIPLINE.md) §4 and §10. It adds **Wedge**, **Echelon Left** and **Echelon Right** next to Line, Column, Dispersed and Perimeter ([part one](RTS_FORMATION_F6_SHAPES_ACCEPTANCE.md)), for plain group moves, formations and regroups, and lets **Shift+V** step the shape cycle backwards. Tested on the multiplayer skirmish test map at its owner cap of 20 units per participant. Layered Screen and Convoy need unit roles, perception boundaries and vehicles that do not exist yet; they, mixed movement profiles, chokepoints and crowd tuning are out of scope.

**Outcome: IMPLEMENTED and ACCEPTED for the skirmish test map. Packaged group-move shape check 46/46 in three runs (all seven shapes and Shift+V driven by real input, identical plans and slot assignments); Automation 46/46; regression matrix unchanged from the FORM-F6 part one baseline; the protocol-7 client is now refused.****

## A. What changed (commit `4f9db3e`, harness fix `7096f4e`)

- **Seven shapes, one key.** V cycles **Line → Column → Dispersed → Perimeter → Wedge → Echelon Left → Echelon Right → Line**; **Shift+V** steps back (the same rule as Tab and Shift+Tab for subgroups, so `V` stays one chord in `DefaultInput.ini`). A press is local and sends nothing. The HUD help line reads `V / Shift+V group shape: …`, and the feedback line, the formation line and the Form outcome name all seven shapes. The development log line `RTS_GROUP_MOVE_SHAPE` records `backward=0|1`.
- **Wedge** (`Source/PolitikaCore/FormationGeometry.h`, `WedgeSlots`): an arrowhead on a triangular lattice. Rank r holds r + 1 slots one spacing (150 cm) apart, and ranks are √3/2 spacings (129.9 cm) apart, so every neighbour in the same rank and in the ranks beside it is exactly 150 cm away. Ranks fill from the point backwards. A partial last rank takes the lattice positions nearest its centre line (ties: the left one), so it never falls between lattice points, where it would stand 130 cm from the rank in front. The nominal slots are shifted so their centroid is the clicked point. Twenty units: ranks of 1, 2, 3, 4, 5 and 5 of 6, 675 cm across and 649.5 cm deep. The point member leads alone, one rank ahead of everyone else: the shape's cost.
- **Echelon Left / Echelon Right**: the Line block (ranks of ten) turned 45 degrees so that the left or the right flank falls back. A file one spacing to that side of the centre ends up 0.71 spacings behind it. A rotation keeps every distance, so an Echelon keeps the Line's 150 cm spacing; twenty units span 1060.7 cm both ways. Members are matched to slots in the Line's own frame (their offsets are turned back first), so the Line's crossing-free order carries over.
- **Discipline.** Wedge and Echelon neighbours stand 150 cm apart, inside the 400 cm support radius, so their formations earn Discipline like a Line.
- **Determinism and bounds.** Unchanged rules: the plan depends only on the member set, the planner calls the caller's legality check at most twice per member, blocked or out-of-bounds slots are replaced by spares behind the block, and shapes outside 0–6 fall back to the shared point (core) and are refused as malformed on the wire.
- **Network.** The shape byte in the Form intent and the owner-only formation view gained three values, so `network_protocol` moves from 7 to 8; a protocol-7 client is refused at join.
- **Tests.** Portable suite: Wedge rank widths, lattice spacing, centring, the partial-rank rule, spares, the point member; Echelon as the turned Line, mirror symmetry of Left and Right, held-back flank depth; input-order independence; left-to-right order kept within each rank; blocked slots; bad layouts. Automation: `GroupMoveShape` extended (an eastward Wedge of eight: ranks 1, 2, 3, 2, 300 × 389.7 cm, the point a rank ahead, centred; Echelon Left and Right of eight: 742.5 cm both ways, the held-back flank's end member the rear-most, centred; V back to Line) and `FormationShapes` extended (a Wedge formation of seven stores its shape, is planned 150 cm apart with one leader and earns one point per evaluation; an Echelon Right formation is planned 150 cm apart with its right flank farthest back). Packaged: `rts_multiplayer --group-move-shape` drives all seven shapes and Shift+V.

## B. Verification

**Identity.** Packages from clean commit `4f9db3e` (`git_dirty=false`, `network_protocol` 8). `7096f4e` changes only `Tools/Shell/src/bin/rts_multiplayer.rs`, so the game source is identical. Client `dev-4f9db3e/…/Politika` `0251acb4a6ea69f255942a60434fd2bdbc13b944bebc84dc2c174a534cb1893e`; server `server-4f9db3e/…/PolitikaServer` `0c2ace3b9b4d1c5f8d7680581063821c5eb901ad06a817af7ef2adf9a43dac40`. Evidence: `/mnt/data/politika-qualification/FORM-F6B-20261031/final-4f9db3e/`.

| Check | Result |
|---|---|
| Portable C++20 suite (`tools/core-test.sh`) | PASS (`core-test.out`) |
| Rust lane: `cargo fmt`, Tools/Shell tests (21), `quality -- audit`, `quality -- core` | PASS (`rust-checks.out`) |
| Editor build | exit 0 (`editor-build-1.log`; again at `7096f4e` before full Automation) |
| Automation, focused: `GroupMoveShape`, `FormationShapes`, `FormationLifecycle`, `FormationDiscipline`, `FormationStance` | 5/5 (`auto-focused-1`) |
| Automation, all Politika tests (`auto-all/`) | 46/46, 0 failures. Warnings are identical to the FORM-F6 part one baseline except in the two extended shape tests: `GroupMoveShape` goes from 57 to 81 NavigationUnavailable warnings and `FormationShapes` from 22 to 36 (Automation worlds have no navmesh); plus the usual one Wayland notice per test |
| Packaged `rts_multiplayer --group-move-shape` | 46/46 in runs 1, 3 and 4 (`group-move-shape-1/`, `-3/`, `-4/`); run 2 42/46, see below |
| Regression matrix (`final-4f9db3e/summary.txt`) | Unchanged from the FORM-F6 part one baseline: discipline-rout 12, discipline-effects 11, discipline-stress 10, formation-discipline 10, formation-stance 14, formation-lifecycle 20, group-orders 8, control-groups 17, unit-scale 73, offline PAC-10 and participant authority PASS, dedicated 1v1 23, listen 1v1 18, dedicated layouts 75 (including the protocol-7 client refused by the protocol-8 server), listen layouts 74, lag-loss 47, engagement 9, fog memory 7. The only non-PASS lines are the same 3 planned NOT_RUNs as the baseline |

**Packaged group-move shape run** (dedicated 1v1, 20 idle units per seat, client A rendered on Xvfb at 1280×720 and driven with real X keyboard and mouse input through xdotool; eight right-click moves of the same 20 box-selected units):

| Shape | Plan | Arrival |
|---|---|---|
| Line, Column, Dispersed, Perimeter | as in part one: 20 placed, 0 rejected, shape extents exact | 20/20 each, 0 navigation failures, min separation 149.3, 149.4, 448.8 and 148.7 cm |
| **Wedge** | 20 placed, 675.0 × 649.5 cm, the point slot 129.9 cm ahead of the next, min separation 149.9 cm, 0 rejected | 20/20, 0 failures, 148.4 cm |
| **Echelon Left** | 20 placed, 1060.7 cm both ways, the left end slot 848.6 cm behind the right end, min separation 149.9 cm, 0 rejected | 20/20, 0 failures, 148.4 cm |
| **Echelon Right** | 20 placed, 1060.7 × 1060.8 cm, the right end slot 848.5 cm behind the left end, min separation 149.9 cm, 0 rejected | 20/20, 0 failures, 149.3 cm |
| Line at the western edge (after Shift+V, V, V) | 20 placed, 7 out-of-bounds candidates replaced by spares, min separation 150.0 cm | 20/20, 0 failures, 148.3 cm |

Every move's 20 intents were received and admitted by the server one by one (`accepted=20 rejected=0`) and routed at the point the client sent. Each V and Shift+V press was local (0 intents sent, 0 received); Shift+V logged `backward=1` and stepped Echelon Right back to Echelon Left, and V twice returned to Line through Echelon Right. The match stayed active. Screenshots (`group-move-shape-*/client-a-*.png`) show the arrowhead, both echelons and the HUD line naming each shape (`V / Shift+V group shape: ECHELON RIGHT` fits the 1280-pixel window). At the test camera's yaw an Echelon whose travel direction runs diagonally across the screen can read on screen as two straight rows; the slot geometry is the turned Line.

**Repeatability.** Runs 1, 3 and 4 logged the same eight plans and the same 160 slot assignments, byte for byte once timestamps are stripped. Run 2 matched them for the first seven moves. This is a same-machine check, not cross-machine proof.

**Harness fix and the failed run.** Run 2 (the first step of the regression matrix) passed all seven shapes but failed the Shift+V step: the game logged `shape=line backward=0`, so it read Shift as up and cycled forward; the next two presses and the edge move (then a Dispersed move) failed with it. The harness pressed `xdotool key shift+v`, which can release Shift inside the same frame as V; the game reads Shift when the V binding fires, as it does for Shift+Tab. A player holds Shift for many frames. Harness fix `7096f4e` holds Shift across the V press with the existing `modified_key` helper (as the control-group and subgroup checks already do); runs 3 and 4 on the same packages then passed 46/46.

## C. Map to the FORM acceptance matrix

| ID | Status in this increment |
|---|---|
| FORM-03 | Shapes outside 0–6 fail closed in the core and on the wire; a protocol-7 client is refused at join (packaged `matrix-none`). |
| FORM-04 | One legal, distinct slot per member for the three new shapes, facing the travel direction; out-of-bounds and blocked candidates are replaced by spares. Assignment is deterministic and keeps left-to-right order within each rank: front to back for the Wedge, in the Line's frame for an Echelon (core, Automation, packaged). |
| FORM-05 | Each member's slot is submitted and admitted as its own order; the server routes it at the point sent, and every member arrived with no navigation failure (packaged). |
| FORM-09 | Formations store and move in the new shapes (Automation). The regroup path is the same code as for every other shape; a Wedge or Echelon regroup after a loss is not separately exercised. |
| FORM-11 | Wedge members support each other and earn Discipline (Automation). |
| FORM-16 | Three packaged runs produced identical plans and slot assignments for all eight moves. Same machine only. |
| FORM-18 | Rendered cooked Linux Development client shows the new shapes and the HUD shape names (screenshots). |
| FORM-13, 14, 17, 19, 20 | Not in scope: faction effects of shape, AI doctrine, scale/device and save work. |

## D. Decisions taken for §4 and §12 (provisional)

- **Wedge geometry:** a filled arrowhead on a triangular lattice at the infantry spacing, not a hollow chevron. A hollow chevron of 20 would be ten ranks deep and 28 m wide; the filled wedge stays compact (6.75 × 6.5 m) and every member keeps two or more neighbours within the support radius. The partial last rank stays on the lattice.
- **Echelon geometry:** the Line turned exactly 45 degrees, ranks of ten. No separate echelon frontage or angle setting yet.
- **Reverse cycle:** Shift+V, read in the V handler like Shift+Tab, instead of a second action mapping.
- **Facing:** slots carry no facing of their own; units keep the existing arrival behaviour.
- **Save/net schema:** shapes are not saved; `network_protocol` 8.

## E. Not covered

- **Layered Screen and Convoy** (need unit roles, perception boundaries and vehicles), mixed movement profiles, chokepoints and crowd tuning.
- **Packaged Form in the new shapes.** T and formation moves in Wedge and Echelon are covered by Automation; the packaged run drives plain group moves in all seven shapes.
- **Combat payoff.** Nothing measures how a Wedge's point or an Echelon's held-back flank fares in a fight; there is no flanking or frontal-fire model that would reward them in numbers.
- **Facing/width drag, speed matching, Reinforce** (FORM-F3 open items) are unchanged.
