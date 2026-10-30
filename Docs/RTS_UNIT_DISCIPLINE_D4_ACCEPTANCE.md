# DISC-D4 — Rout: acceptance record

**Date:** 30 October 2026 · **Scope:** the rout part of the DISC-D4 slice of [Military Discipline](RTS_UNIT_DISCIPLINE.md) §7A and §8. Rout is the first consumer of the shared broken threshold of `UnitResolve`. A military unit at Discipline 0 breaks, leaves its formation, stops fighting, refuses orders and flees to its side's start until it rallies at 15. It was tested on the multiplayer skirmish test map at its cap of 20 units per participant. The other two break consumers, **Atlantic Concord Psy-Ops and Iron Ascendancy Intimidation, are not part of this record**. They need spies, faction abilities and their own acceptance ([Espionage and Intimidation](ESPIONAGE_AND_INTIMIDATION.md)). DISC-13 (the Population Morale baseline) and DISC-15 (spread) stay deferred.

**Outcome: IMPLEMENTED and ACCEPTED for the skirmish test map.**
- **Packaged rout check:** `discipline-rout` PASS 12/12 in two runs with identical rout timelines.
- **Automation:** 45/45.
- **Regression matrix:** unchanged from the DISC-D3 baseline, including the DISC-D1 to DISC-D3 checks.
- **Protocol:** `network_protocol` 5 → 6. The DISC-D3 protocol-5 client is refused.

## A. What changed

Commits `274850e` (feature), `8f0f51c` and `879b92a` (development witness fixture and harness corrections; no gameplay change).

- **Break.** A military unit whose value is at or below 0 breaks in that step. The check runs once per step after the casualties and before the evaluation, in stable-ID order (`ProcessDisciplineBreaks`), so a member shocked to 0 routs before it could earn its way back. Today casualty shock (DISC-D2) is the only rule that lowers a member that far. Civilians at Morale 0 never rout (`politika::discipline::Breaks`).
- **Formation.** The routed member leaves its formation with `LeaveReason::Routed`. The formation treats this like any other loss: it schedules the Discipline-scaled regroup of DISC-D3, or dissolves below two members.
- **Flight.** The unit drops its order, target, grenade and engagement. It flees to its participant's authored start (or its own spawn point when the participant has none) along a complete navigation route, as a plain Move that never acquires a target. Without a route it holds where it is. After the flight it stays put: a routed unit has no stance behaviour.
- **No fire.** `ResolveCombat` never acquires or fires for a routed unit. It can still be shot, and at 0 it takes 20% more damage (DISC-D3).
- **Orders.** `AdmitCommand` refuses a routed issuer with the new `OrderRejection::Routed` (19). The check comes after the ownership check, so another participant gets its usual ownership rejection and never learns the state. Form treats a routed unit as unavailable (`MemberUnavailable`). A per-unit stance setting is accepted and applies after the rally.
- **Recovery and rally.** A routed unit's evaluation is `Service::Routed`: +0.5 per evaluation wherever it is, alone or not. At 15 it rallies where it stands: idle, outside any formation, with a fresh post, taking orders again. The first evaluation is the break step itself when it falls on one, so the rally comes on the 30th evaluation counted from there.
- **Events.** `OrderEndReason::Routed` on the order change at the break; `Stopped` at the rally.
- **Network and HUD.** The routed flag is owner-only replicated (`COND_OwnerOnly`), and `network_protocol` goes 5 → 6. The HUD changes in three places:
  - A selected routed unit's line reads `ROUTED` in place of its order.
  - A selection line adds `N ROUTED`.
  - A refused order shows "That unit is routing and will not take orders."
- **Logs.** `RTS_DISCIPLINE_ROUT` lines record each stage:
  - `phase=break`: value, formation, previous and new order, route, rally point and distance.
  - `phase=flight_end`: reason and distance left.
  - `phase=rally`: steps routed and distance from the rally point.

  Alongside them: `RTS_FORMATION_MEMBER_LEFT reason=routed`, and `MP_CLIENT_SAMPLE` now carries `routed=`.
- **Development fixtures:**
  - `-PolitikaMPDisciplineRoutSeed=<points>` sets every living member of the casualty fixture's formation to `<points>` when the fixture arms, so its ordinary casualty shock takes the members near the casualties to 0.
  - `-PolitikaMPDisciplineRoutWitness` walks the opponent's lowest-ID unit, at a participant's first rout, with an ordinary Move to open ground about 1000 cm from the rally point. The point is chosen by the Move's own route query, 900 cm from the control point and 700 cm from every soldier of the routed side that can still fire. The opposing client then receives the routed units, which lets the run show their state never reaches it.
- **Tests.** Portable rules (break and rally thresholds, routed recovery, two shocks from 10 reach 0, `Routed` = 19 checked after ownership), Automation `Politika.RTS.Multiplayer.DisciplineRout`, packaged harness `rts_multiplayer --discipline-rout`.

## B. Verification

**Identity.** Packages come from clean commit `879b92a` (`git_dirty=false`, `network_protocol` 6, `game_version` 0.1.0-dev.0).
- Client `dev-879b92a/…/Politika`: `37c3889b44a2e71ef6d882d2bb031ba0c81e3cec6767727456547a86e4b7d527`.
- Server `server-879b92a/…/PolitikaServer`: `25c4c6d9a52747e899573456267ea50519ba7b8afcee620bdca2f6aef5ca37f4`.
- Evidence: `/mnt/data/politika-qualification/DISC-D4-20261030/final-879b92a/`.

| Check | Result |
|---|---|
| Portable C++20 suite (`tools/core-test.sh`) | PASS (`core-test.out`) |
| Editor build | exit 0 |
| Automation, focused | `DisciplineRout` with `DisciplineEffects`, `DisciplineStress`, `FormationDiscipline` and `FormationLifecycle` (`auto-focused-1`). The first run failed two `DisciplineRout` order checks on a test bug: the test reused the host's client sequence, so intake refused the orders as duplicates. After the fix, `DisciplineRout` passed (`auto-focused-2`, `-3`, `-4`). |
| Automation, all Politika tests | 45/45 (44 earlier + `DisciplineRout`) before the feature commit (`auto-all-pre/`), and again 45/45 at `879b92a` after an editor build that reported the target up to date (`auto-all/`) |
| Packaged `rts_multiplayer --discipline-rout`, two runs | 12/12 and 12/12 (`discipline-rout-1/`, `-2/`) |
| Regression matrix (`final-879b92a/summary.txt`) | Unchanged from the DISC-D3 baseline. Discipline effects 11/11, discipline stress 10/10, formation discipline 10/10, formation stance 14/14, formation lifecycle 20/20, group-move shape 16/16, group orders 8/8, control groups 17/17, unit-scale 73/73, offline PAC-10 and participant authority PASS, dedicated 1v1 23/23, listen 1v1 18/18, listen layouts 74, lag-loss 47, engagement 9/9, fog memory 7/7. Dedicated layouts 75/75 with the DISC-D3 protocol-5 client: `MP2-PROTOCOL-LEGACY-REFUSED` passes (`MP_PRELOGIN REJECTED … protocol=5 … MP.PROTOCOL.MISMATCH`). The only non-PASS lines are the three planned NOT_RUNs: the legacy client is not supplied to listen layouts or lag-loss, and 1v1 has no allies. |

**Earlier packaged attempts.** These were on the superseded packages, and their evidence is kept.
- **`final-274850e/discipline-rout-1` (7/12).** The game behaved as specified. The harness had three bugs:
  - It counted two fixture victims that broke and then fell.
  - It used a rally window one step off.
  - It found no opposing samples, because fog hid the routed units from the opponent.

  The witness fixture was added for the last of these.
- **`final-8f0f51c/discipline-rout-1-crashed`.** Client A crashed at start-up in the engine's Vulkan pipeline compiler (`RHIResources.h:153`, `(OldPacked & DeletingBit) == 0`), before any gameplay.
- **`final-8f0f51c/discipline-rout-1` (11/12).** Every gameplay verdict passed. The witness found no point, because its straight-walk clearance could not be met beside the 20-unit Line. Commit `879b92a` changed it to the route query and endpoint clearance.

**Automation `DisciplineRout`** runs a standalone 1v1 coordinator with exact steps:
- **Break.** A row of six members is set to 10. One casualty leaves the survivors at 5, unbroken. The second takes the four within 800 cm to 0, and they break in that step, leave the formation (it dissolves) and flee along a route to their start without a target.
- **Fire.** A lone soldier attacking a dummy is set to 0. It breaks the next step, holds in place with no route, and never fires again over 40 steps, while a control soldier keeps firing.
- **Orders.** The owner's Move to a routed unit is refused as `Routed`. The opponent's Move to the same unit is refused without `Routed`. Form refuses the routed soldiers.
- **Flight end.** The flight ends at the start, and the unit is still routed there.
- **Rally.** After 29 evaluations a routed unit is at 14.5, still routed; the 30th takes it to 15 and it rallies. The soldier broken one evaluation later rallies one evaluation later.
- **After the rally.** The rallied soldiers take orders and can be formed again.
- **Civilians.** A civilian at Morale 0 never routs.

**Packaged discipline-rout run.** Setup: dedicated 1v1, 20 units per seat, fixture held idle, casualty fixture with the rout seed of 10 and the witness armed, and participant 2 on the solo-tolerant override as the baseline reference. Client A was rendered on Xvfb at 1280×720 and driven with real X keyboard and mouse input through xdotool.

| Verdict | What it shows (run 1) |
|---|---|
| `FORM-FIXTURE-20-PER-SEAT` | 20 own units. |
| `DISC-SETUP-LINE-ARRIVED` | Ctrl+1, T and a right-click move put the 20 into a Line: 20/20 at their slots, 0 failures, minimum separation 149.3 cm. |
| `DISC-20-BREAK-AT-ZERO` | J arms the fixture at step 306: the 20 members are seeded to 10 and the victims are 11, 16, 15 and 17. Of 23 shocks in range, 10 took a member to 0, and exactly those 10 broke in the same step, at value 0 (step 318, the second casualty). These were units 2, 5, 6, 7, 8, 9, 12, 15, 17 and 18. Victims 15 and 17 had broken before the fixture killed them. 8 members beyond the shock stayed in the Line. |
| `DISC-21-LEAVES-FORMATION` | All 10 left formation 1 as `routed` in step 318 and took no later shock. The formation kept 8 members and regrouped them at step 370. |
| `DISC-22-FLEES-TO-START` | The 8 living routed soldiers each fled along a route to seat 1's start (−1300, −700), starting 828–1046 cm away, and arrived after 79–100 steps. 15 and 17 fell while routed. |
| `DISC-24-ORDERS-REFUSED` | Key 1 selects group 1 (16 soldiers) and the HUD reads `16 UNITS SELECTED … DISCIPLINE avg 6    8 ROUTED`. A right-click move was refused with code 19 for exactly the 8 routed soldiers and admitted for the other 8. The client reports `Move: 8 of 16 units accepted · 8 rejected (That unit is routing and will not take orders.)`. |
| `DISC-23-ROUTED-DO-NOT-FIGHT` | Between break and rally, no routed soldier logged a shot or a hit, and none had an order admitted. Below the baseline every shot logs its fire interval, so any shot would have been logged. |
| `DISC-26-RALLY-AT-15` | All 8 rallied at 15 at step 900 (582 steps routed). That is exactly 30 evaluations after the break: the first evaluation came at step 320. |
| `DISC-26-ORDERS-AFTER-RALLY` | The same group order was then admitted for all 8, and the client reports 16 of 16 accepted. |
| `DISC-07-OWNER-ONLY-ROUT` | The witness walked opposing unit 3 to (−977, 246), 1000 cm from the start. The opposing client then received seat 1's units in 231 samples, 168 of them soldiers that had routed. All 231 read routed 0 at 50 000. The owner's client logged 246 routed samples. |
| `DISC-D4-BASELINE-NO-ROUT` | Participant 2 never broke. |
| `FORM-MATCH-STAYS-ACTIVE` | No objective capture or terminal phase. |

Screenshots (`discipline-rout-*/client-a-*.png`): after the break, the group with its routed count, the refused order, and after the rally.

**Repeatability.** Both runs log the same 29 rout lines in the same order:
- the same seed and victims;
- the same 10 breaks of the same units at the same positions, with the same rally point;
- the same witness point;
- the same flight durations (79–100 steps);
- the same 8 rallies at step 900.

J arrived one step later in run 2, so the breaks came at step 319 and the units were routed for 581 steps. The only logged value that differs is unit 12's value at the end of its flight (2 000 against 2 500): its arrival step, 401, fell just after the evaluation at 400 instead of on it. This is a same-machine check, not cross-machine proof.

## C. Map to the DISC-D4 acceptance

| ID | Status |
|---|---|
| DISC-20 | A military unit breaks exactly in the step it reaches 0, and civilians never break. Covered by core, Automation and packaged checks. |
| DISC-21 | The routed member leaves its formation as `routed` in that step and takes no later shock. The formation regroups or dissolves. Covered by Automation and packaged checks. |
| DISC-22 | A routed unit flees to its participant's start along a route, or holds without one. Covered by Automation and packaged checks. |
| DISC-23 | A routed unit never acquires or fires. Covered by Automation (a dummy in range, with a control shooter) and packaged checks. |
| DISC-24 | The owner's orders are refused with `Routed`, and another participant never learns the state. Covered by core, Automation and packaged checks; the packaged check uses an owner-only flag observed on the opposing client through the witness. |
| DISC-25 | Form refuses a routed unit. Covered by Automation. |
| DISC-26 | Recovery runs at 0.5 per evaluation, and the rally comes at 15 on the 30th evaluation, with orders taken again. Covered by core, Automation and packaged checks. |

## D. Decisions taken (provisional)

- **Break at 0 and rally at 15.** Recovery runs at the fade rate anywhere, so 30 s from 0. A rallied soldier stands where it is, outside any formation. Beside a comrade it climbs to 50; alone it holds below the isolation floor until it has company.
- **Flee to the participant's authored start.** This is the only "home" the prototype has. Bases or a nearest friendly formation can replace it later. Routed soldiers end on the same point, because the prototype has no crowd avoidance.
- **Flight at walking speed**, as a plain Move.
- **Rout is not contagious.** A breaking soldier does not shock its former comrades. A fixture victim that broke before it fell shocks nobody, because it was no longer a member.
- **The rejection comes after ownership**, so the routed state stays owner-only. The order rejection code and the replicated flag needed a protocol change, 5 → 6.
- **Stance settings are accepted while routed.** They are settings, not orders, and apply after the rally.

## E. Not covered

- **Psy-Ops and Intimidation**, the other break consumers: they arrive with their espionage and faction systems.
- **Other ways to 0.** Only casualty shock can lower Discipline that far today. Suppression, Psy-Ops and Intimidation would add more.
- **Contagion, return to the old formation, a faster flight, flight to bases**, and a visible "routing" cue for opponents (intelligence rules) are open decisions in §10.
- **The organic path in the packaged run.** It seeds the members to 10 before the casualties. Automation also starts the row at 10. A fully organic break from 50 needs ten in-range shocks across four windows, which this map's fixture does not produce.
- **Balance:** the thresholds are first values.
- **Player playtest build:** the launcher's default package is not updated by this record.
