# FORM-F6 (part one) — Dispersed and Perimeter shapes: acceptance record

**Date:** 30 October 2026 · **Scope:** the first FORM-F6 increment of [Military Formations, Tactical Stances, Cohesion and Discipline](RTS_UNIT_FORMATIONS_AND_DISCIPLINE.md) §4 and §10. It adds the two remaining base shapes, **Dispersed** and **Perimeter**, next to Line and Column, for plain group moves, formations and regroups. Tested on the multiplayer skirmish test map at its owner cap of 20 units per participant. Wedge, Echelon, Layered Screen, Convoy, mixed movement profiles, chokepoints and crowd tuning (the rest of FORM-F6) are out of scope and do not exist.

**Outcome: IMPLEMENTED and ACCEPTED for the skirmish test map. Packaged group-move shape check 26/26 in two runs, with all four shapes driven by real input; Automation 46/46; regression matrix unchanged from the DISC-D4 baseline; the protocol-6 client is now refused.**

## A. What changed (commit `df2d769`, harness fix `01d10c6`)

- **Four shapes, one key.** V cycles **Line → Column → Dispersed → Perimeter → Line**. The shape is local until it is used: a press sends nothing. It applies to a plain group move or attack-move, to **T** (the formation keeps the shape it was formed in), to formation moves and to the bounded regroup after a loss. The HUD help line, the `Group move shape` feedback and the formation line name all four shapes.
- **Dispersed** (`Source/PolitikaCore/FormationGeometry.h`): staggered ranks of five, 450 cm apart, facing the direction of travel. Odd ranks sit half a spacing to the right, and the block is shifted back a quarter spacing so it stays centred on the clicked point. 450 cm is deliberately wider than the 400 cm Discipline support radius ([Military Discipline](RTS_UNIT_DISCIPLINE.md) §3), and the nearest diagonal neighbour is 503 cm away. This is the shape's cost: a blast catches one soldier, but nobody is close enough to support anyone. A **Dispersed formation** keeps the Discipline it has (members are neither earning nor alone; HUD trend Steady). A **plain Dispersed group move** leaves every soldier alone under the DISC-D2 isolation rule, so their Discipline fades towards 30.
- **Perimeter**: one ring around the clicked point, neighbouring chords exactly 150 cm, never tighter than a 300 cm radius (8 units: 300 cm; 20 units: 479.4 cm). Neighbours on the ring stand 150 cm apart, so a Perimeter formation earns Discipline like a Line. Rings beyond 1200 cm (more units than the skirmish cap can field) fill full rings from 1200 cm inwards, then outwards; spare slots go on outer rings. Members and legal slots are both ordered clockwise around the anchor and matched in that cyclic order at the rotation with the least total squared travel (ties: the lowest rotation), so neighbours do not cross. When legal slots run short, the members farthest from the anchor are unplaced and keep the plain shared-point order, as on the other shapes.
- **Determinism and bounds.** The plan depends only on the member set, never on input order. Shapes outside 0–3 and non-finite spacing fall back to the shared point (core) and are refused as malformed on the wire. The planner still calls the caller's legality check at most twice per member.
- **Network.** The shape byte in the Form intent and the owner-only formation view gained two values, so `network_protocol` moves from 6 to 7; a protocol-6 client is refused at join.
- **Tests.** Portable suite: both shapes' slot geometry, stagger and centring, ring radius and chords, multi-ring fill and spares, clockwise assignment and its tie-break, unplaced members, input-order independence. Automation: new `Politika.RTS.Multiplayer.FormationShapes` (a Dispersed formation stores its shape, is planned 450 cm apart and holds its Discipline; a Perimeter formation is planned on a 300 cm ring of 8, earns one point per evaluation, and after a loss regroups once into an evenly spaced ring of 7 around the same point) and `GroupMoveShape` extended to all four shapes. Packaged: `rts_multiplayer --group-move-shape` drives all four shapes.

## B. Verification

**Identity.** Packages from clean commit `df2d769` (`git_dirty=false`, `network_protocol` 7). `01d10c6` changes only `Tools/Shell/src/bin/rts_multiplayer.rs`, so the game source is identical; the harness binaries used for the evidence below are built from `01d10c6`. Client `dev-df2d769/…/Politika` `501f829c239dd7f110b106cff5e03e0336735790e1191a5037ac680147935f55`; server `server-df2d769/…/PolitikaServer` `5ca4bbce6c9965a23650c239973eb4bc7cff7ed75d8c5042de3f669f0fd3f76f`. Evidence: `/mnt/data/politika-qualification/FORM-F6-20261030/final-df2d769/`.

| Check | Result |
|---|---|
| Portable C++20 suite (`tools/core-test.sh`) | PASS (`core-test.out`) |
| Rust lane: `cargo fmt`, Tools/Shell tests (21), `quality -- audit`, `quality -- core` | PASS (`rust-checks.out`) |
| Editor build | exit 0 |
| Automation, focused: `FormationShapes`, `GroupMoveShape`, `FormationLifecycle`, `FormationDiscipline`, `FormationStance` | 5/5 (`auto-focused-2`). The first run (`auto-focused-1`) failed one FormationShapes check because the test counted queued regroup orders; a member already on its new ring slot correctly gets no order, so the test now reads the formation's stored slots |
| Automation, all Politika tests (`auto-all/`) | 46/46, 0 failures. Warnings are identical to the DISC-D4 baseline (45/45) except in the two shape tests: the new `FormationShapes` adds 22 NavigationUnavailable warnings, and the extended `GroupMoveShape` goes from 42 to 57 (Automation worlds have no navmesh); plus the usual one Wayland notice per test |
| Packaged `rts_multiplayer --group-move-shape`, two runs | 26/26 and 26/26 (`group-move-shape-1/`, `-2/`) |
| Regression matrix (`final-df2d769/summary.txt`) | Unchanged from the DISC-D4 baseline: discipline-rout 12, discipline-effects 11, discipline-stress 10, formation-discipline 10, formation-stance 14, formation-lifecycle 20, group-orders 8, control-groups 17, unit-scale 73, offline PAC-10 and participant authority PASS, dedicated 1v1 23, listen 1v1 18, dedicated layouts 75 (including the protocol-6 client refused by the protocol-7 server), listen layouts 74, lag-loss 47, engagement 9, fog memory 7. The only non-PASS lines are the same 3 planned NOT_RUNs as the baseline (legacy refusal on listen layouts and lag-loss, allied fog in 1v1) |

**Packaged group-move shape run** (dedicated 1v1, 20 idle units per seat, client A rendered on Xvfb at 1280×720 and driven with real X keyboard and mouse input through xdotool; five right-click moves of the same 20 box-selected units):

| Shape | Plan | Arrival |
|---|---|---|
| Line | 20 placed, 1350 × 150 cm, min separation 150.0 cm | 20/20 within 30 cm of their slot, 0 navigation failures, min separation 149.3 cm |
| Column | 20 placed, 150 × 1350 cm, min separation 150.0 cm | 20/20, 0 failures, 149.4 cm |
| **Dispersed** | 20 placed, 2025 × 1350 cm (four staggered ranks of five), min separation 449.9 cm, 0 rejected | 20/20, 0 failures, 448.8 cm |
| **Perimeter** | 20 placed, every slot within 0.09 cm of the 479.4 cm ring, ring centre 0.1 cm from the anchor, min separation 149.9 cm, 0 rejected | 20/20, 0 failures, 148.7 cm |
| Line at the western edge | 20 placed, 11 out-of-bounds candidates replaced by spares, min separation 149.9 cm | 20/20, 0 failures, 149.9 cm |

Every move's 20 intents were received and admitted by the server one by one (`accepted=20 rejected=0`) and routed at the point the client sent. Each V press was local (0 intents sent, 0 received). The match stayed active (no capture, no terminal phase). Screenshots (`group-move-shape-*/client-a-*.png`) show the staggered Dispersed block, the ring and the HUD naming each shape.

**Repeatability.** Both runs logged the same five plans and the same 100 slot assignments, byte for byte once timestamps are stripped (anchor, facing, rejected candidates, every unit's slot and world point), and the same arrival separations. This is a same-machine check, not cross-machine proof.

**Harness fix and superseded attempt.** The first packaged run (`group-move-shape-attempt0-perimeter-click-on-soldier/`, 17 PASS, 1 FAIL) passed Line, Column and Dispersed. Its Perimeter click point (-1350, -1100) lay 79 cm from soldier 15, whom the Dispersed move had just put there, so the right-click hit that soldier: the client sent a friendly Attack, which the server rejected (code 8) and which produced no group-move plan. That is correct game behaviour and a harness mistake. Harness fix `01d10c6` moves the Perimeter centre to (-850, -1360), inside a triangle of the Dispersed block about 280 cm from its nearest soldier; the ring stays inside the map and 1250 cm clear of the control point. The same packages then passed 26/26.

## C. Map to the FORM acceptance matrix

| ID | Status in this increment |
|---|---|
| FORM-03 | Shapes outside 0–3 fail closed in the core and on the wire; a protocol-6 client is refused at join (packaged `matrix-none`). |
| FORM-04 | One legal, distinct slot per member for both new shapes, facing the travel direction (Dispersed) or centred on the point (Perimeter); out-of-bounds and blocked candidates are replaced by spares. Assignment is deterministic and crossing-free: front to back and left to right for Dispersed, clockwise at the least-travel rotation for Perimeter (core, Automation, packaged). |
| FORM-05 | Each member's slot is submitted and admitted as its own order; the server routes it at the point sent, and every member arrived with no navigation failure (packaged). |
| FORM-09 | Formations store, move and regroup in the new shapes; a Perimeter regroup closes the ring around the same point (Automation). |
| FORM-11 | A Dispersed formation cannot claim support: its members stand outside the 400 cm support radius and hold their Discipline; Perimeter neighbours support each other and earn (Automation). |
| FORM-16 | Two packaged runs produced identical plans and slot assignments for all five moves. Same machine only. |
| FORM-18 | Rendered cooked Linux Development client shows both new shapes and the HUD shape names (screenshots). |
| FORM-13, 14, 17, 19, 20 | Not in scope: faction effects of shape, AI doctrine, scale/device and save work. |

## D. Decisions taken for §4 and §12 (provisional)

- **Dispersed spacing 450 cm**, wider than the 400 cm support radius, so dispersal costs Discipline gain. Frontage five, staggered ranks. First values for the skirmish map, not balance; there is no blast or artillery yet that rewards the spacing in numbers.
- **Perimeter geometry:** 150 cm chords, minimum radius 300 cm, single ring up to 1200 cm, then extra rings inwards and outwards. Members are placed on the ring only; there is no centre slot for a protected leader or gun yet.
- **Facing:** Perimeter slots carry no outward facing; units keep the existing arrival behaviour. Facing per slot comes with facing/width drag (FORM-F3 open item).
- **Save/net schema:** shapes are not saved; `network_protocol` 7.

## E. Not covered

- **Wedge, Echelon Left/Right, Layered Screen, Convoy**, mixed movement profiles, chokepoints and crowd tuning (the rest of FORM-F6).
- **Packaged Form in the new shapes.** T, formation moves and the Perimeter regroup in Dispersed and Perimeter are covered by Automation; the packaged run drives plain group moves in all four shapes.
- **Blast and artillery payoff for Dispersed.** Grenades exist, but no check measures how many soldiers a blast catches per shape.
- **Outward facing and a protected centre for Perimeter**, divided fire arcs, and any defensive bonus.
- **The ring partly under the HUD.** At the default camera, the lower part of a 20-unit ring can sit behind the command panel; this is a camera and HUD layout matter, not a shape defect.
- **Formation speed matching, Reinforce, facing/width drag** (FORM-F3 open items) are unchanged.
