# DISC-D2 — Discipline under stress: acceptance record

**Date:** 30 October 2026 · **Scope:** the DISC-D2 slice of [Military Discipline](RTS_UNIT_DISCIPLINE.md) §2, §3A, §6 and §8: casualty shock, isolation below the baseline with the Atlantic Concord exemption, the Red Directorate / Iron Ascendancy mass-formation ceiling, and unit doctrine from content faction IDs. Tested on the multiplayer skirmish test map at its owner cap of 20 units per participant. Discipline still drives **no other attribute**. The faction baseline from Population Morale (DISC-13) is **not** in this slice: no Population Morale system exists in Source, so the baseline stays the constant 50.

**Outcome: IMPLEMENTED and ACCEPTED for the skirmish test map, except DISC-13 (deferred); packaged discipline-stress PASS (10/10, two runs with identical evaluation results), Automation 43/43, regression matrix unchanged from the DISC-D1 baseline including the DISC-D1 formation-discipline check, and the protocol-4 client is still refused.**

## A. What changed (commit `0aab078`)

- **Isolation (§3A).** At each evaluation (every 20 steps) a military unit that is not a formation member and has no living friendly military unit within 400 cm is *alone*. Friendly means the same owner, or the same non-zero alliance team. A lone unit fades 0.5 point per evaluation towards a floor of 30 and holds there. Outside a formation and not alone, a value below 50 now returns to 50 at the same rate (DISC-D1 held it). Concord units (solo-tolerant doctrine) and civilians are never isolated; formation members are never isolated (a stranded member still holds).
- **Casualty shock (§3A).** When a formation member dies, every surviving member of the same formation within 800 cm of where it fell loses 5 points in the step of the death. Each unit's first shock opens its own 100-step (5 s) window in which it loses at most 15 points. The meter never goes below 0. Members dying in the same step do not shock each other. Non-members, other formations and opponents are untouched.
- **Mass formations.** A mass-formation doctrine formation (Directorate, Ascendancy) earns up to 100 at an evaluation where at least 32 of its members earn; otherwise up to 80. An earning member above its ceiling fades to it at 0.5 per evaluation.
- **Doctrine.** `faction.red_directorate` / `faction.iron_ascendancy` → mass formation, `faction.atlantic_concord` → solo-tolerant, everything else (Meridian Pact, Unaligned, the prototype test factions) → standard. Development builds accept `-PolitikaMPDisciplineDoctrine=<participant>:<standard|mass|solo>[,…]`; Automation sets the same participant override.
- **Bands.** `CrossedBand` also reports falling through 40, 30, 20, 10 and 0, and landing on 50 from below.
- **Logs.** `RTS_DISCIPLINE_SHOCK` for every surviving member at each formation casualty (`in_range`, distance, before/after, applied, window), `RTS_DISCIPLINE_ISOLATED` every 100 steps per lone soldier (doctrine, `exempt`, value), `RTS_DISCIPLINE_MASS` for a formation at the mass ceiling, `RTS_DISCIPLINE_DOCTRINE_OVERRIDE`.
- **Development fixture** `-PolitikaMPDisciplineCasualties`: the first adherence change (J) on a formation of at least 8 kills its member farthest from the centre and that member's three nearest fellow members, one per step from 10 steps later (`MP_DISCIPLINE_CASUALTIES_ARMED`, `MP_DISCIPLINE_CASUALTY`).
- **Network.** No replicated field changed: the value and trend replicate owner-only as in DISC-D1. `network_protocol` stays 5.
- **HUD.** Unchanged; values below 50 show as `DISCIPLINE 30`, recovery as `rising`.
- **Harness.** `rts_multiplayer --discipline-stress` (below).

## B. Verification

**Identity.** Packages from clean commit `0aab078` (`git_dirty=false`, `network_protocol` 5, `game_version` 0.1.0-dev.0). Client `dev-0aab078/…/Politika` `d2a5eb85e0cc5e1c450dfa086dfc5da191c448b8a4cfcef804aee15acca543de`; server `server-0aab078/…/PolitikaServer` `90bc7c997443dc12086982830e54d345b35819485d2436c46594d23fd6bc73c2`. Evidence: `/mnt/data/politika-qualification/DISC-D2-20261030/final-0aab078/`.

| Check | Result |
|---|---|
| Portable C++20 suite (`tools/core-test.sh`): isolation fade and floor (40 evaluations 50 → 30, then held; recovery 30 → 50 in 40), recovery below the baseline outside a formation, fading down to a lower ceiling, doctrine mapping per faction, `IsolationApplies` (Concord and civilians exempt), `FormationCeiling` at 31/32 members, mass climb 80 → 100 in 20, casualty shock sequence 5/5/5/0 then a new window at +100 steps, floor at 0, falling band crossings | PASS (`core-test.out`) |
| `quality -- audit` | PASS (`quality-audit.out`) |
| Editor build | exit 0 |
| Automation, focused: new `Politika.RTS.Multiplayer.DisciplineStress` plus `FormationDiscipline`, `FormationLifecycle`, `FormationStance` | 4/4 (`auto-focused-3`). `FormationDiscipline` needed one expectation change: its independent member stands 150 cm from a member killed later, so it now loses the casualty shock (`auto-focused-1` shows the old expectation failing for exactly that reason) |
| Automation, all Politika tests (`auto-all/`) | 43/43 (42 earlier + `DisciplineStress`); state and warning count identical per test to the DISC-D1 baseline; the new test's only warning is the SDL start-up line |
| Packaged `rts_multiplayer --discipline-stress`, two runs | 10/10 and 10/10 (`discipline-stress-1/`, `-2/`) |
| Regression matrix (`final-0aab078/summary.txt`) | Unchanged from the DISC-D1 baseline: formation discipline 10/10, formation stance 14/14, formation lifecycle 20/20, group-move shape 16/16, group orders 8/8, control groups 17/17, unit-scale 73/73, offline PAC-10 and participant authority PASS, dedicated 1v1 23/23, listen 1v1 18/18, listen layouts 74, lag-loss 47, engagement 9/9, fog memory 7/7. Dedicated layouts 75/75 with the FORM-F4 protocol-4 client: `MP2-PROTOCOL-LEGACY-REFUSED` passes (`MP_PRELOGIN REJECTED … protocol=4 … MP.PROTOCOL.MISMATCH`). The only non-PASS lines are the three planned NOT_RUNs (legacy client not supplied to listen layouts and lag-loss, no allies in 1v1) |

**Automation `DisciplineStress`** (standalone 1v1 coordinator, no navmesh, evaluations stepped exactly; the opponent on the solo-tolerant override, the host first on its test faction's Standard doctrine): a lone soldier fades 0.5 per evaluation, reaches 30 after 40 evaluations and holds; an enemy soldier beside it is no company; a host pair and a soldier beside a formation stay at 50; lone Concord soldiers and a lone civilian stay at 50; a stranded formation member holds at 50 and is not isolated; a Standard formation of 34 stops at 80. Moved beside a comrade, the lone soldier recovers 0.5 per evaluation back to 50. With the host switched to the mass doctrine, the 34-member formation climbs past 80 to exactly 100 in 20 evaluations and holds, while an 8-member mass-doctrine formation stays at 80; three members stranded leave 31 earning, and the others fade 0.5 per evaluation from 100 while the stranded hold. Casualty shock on a row of 7 plus a stranded member: the first casualty costs the members at 150–750 cm 5 points each and leaves the member at 900 cm, the stranded member and a non-member beside the row untouched; after three casualties the nearest members have lost 15, the fourth casualty costs them nothing while a member that had lost only 10 reaches its 15; 100 steps after the first casualty, a new casualty costs a member whose window has closed 5 again and a member whose window opened one step later nothing.

**Packaged discipline-stress run** (dedicated 1v1, 20 units per seat, fixture held idle, stance probe and casualty fixture armed, participant 2 on the solo-tolerant override, client A rendered on Xvfb at 1280×720 and driven with real X keyboard and mouse input through xdotool):

| Verdict | What it shows (run 1) |
|---|---|
| `FORM-FIXTURE-20-PER-SEAT` | 20 own units. |
| `DISC-01-BASELINE-AT-START` | First participant sample (step 100): 20 units, all at 50 000. |
| `DISC-SETUP-LINE-ARRIVED` | Unit 1 alone in group 2 is sent behind the army; T and a move put the other 19 into a Line: 19/19 at their slots, 0 failures, minimum separation 149.6 cm. |
| `DISC-11-LONE-SOLDIER-FADES-TO-30` | Unit 1's isolated samples every 100 steps: 48 500, 46 000, 43 500, 41 000, 38 500, 36 000, 33 500, 31 000, 30 000, 30 000. Doctrine standard, not exempt; never below 30 000. The HUD reads `DISCIPLINE 30`. |
| `DISC-11-CONCORD-EXEMPT` | H (Guard) makes the probe walk opposing unit 3 away from its comrades: it is logged alone in 2 samples as `doctrine=solo exempt=1 value=50000`, and participant 2's minimum stays 50 000 for the whole run. |
| `DISC-10-SHOCK-REACHES-ONLY-MEMBERS-IN-RANGE` | J arms the fixture on the 19-member formation: victims 22, 20, 14 and 21 fall on steps 1581–1584. Every surviving member is listed once per casualty (18, 17, 16, 15). 40 in-range shocks, all at ≤ 751.7 cm; 26 out-of-range entries, all at ≥ 838.4 cm with nothing applied. |
| `DISC-10-SHOCK-5-CAPPED-AT-15` | Every in-range shock costs exactly 5 until the member's 15 is spent; 7 members lost 5, 5, 5 and then 0 on the fourth casualty; nobody lost more than 15. After the casualties the formation reads `DISCIPLINE avg 71`. |
| `DISC-11-RECOVERS-BESIDE-COMRADES` | Group 2 walks unit 1 back into the Line: it lands on 50 rising at step 2480, 40 evaluations after it reached its comrades; the owner client's values rise without a dip from 30 to 50, and the HUD reads `DISCIPLINE 50`. |
| `DISC-07-OWNER-ONLY` | The opposing client received participant 1's units in 55 samples, every one at 50 000, while the owner's client logged 110 samples below 50 000. |
| `FORM-MATCH-STAYS-ACTIVE` | No objective capture or terminal phase. |

Screenshots (`discipline-stress-*/client-a-*.png`): the lone soldier selected at `DISCIPLINE 30`, the formation after the casualties at `DISCIPLINE avg 71` (15 of 19 members, Degraded), and the lone soldier back at `DISCIPLINE 50`. The HUD lines are legible and do not overlap at 1280×720.

**Repeatability.** Both runs produce the identical lone-soldier series at the identical steps (its isolation began at the same evaluation in both runs), the same probe unit exempt at step 1300, the same four victims and the same per-member shock costs, distances and in/out-of-range counts. The casualties fell on steps 1581–1584 and 1597–1600 because J arrived at a different step; recovery landed at steps 2480 and 2500 for the same reason. This is a same-machine check, not cross-machine proof.

## C. Map to the DISC acceptance matrix

| ID | Status |
|---|---|
| DISC-01–09 | Unchanged from DISC-D1; the packaged `--formation-discipline` check passes 10/10 on these packages. DISC-06 now also covers recovery from below the baseline. |
| DISC-10 | Shock reaches only surviving members of the fallen unit's formation within 800 cm and respects the 15-per-window cap (core, Automation, packaged). |
| DISC-11 | Isolation drift applies to lone soldiers except Concord units and civilians; recovery beside a comrade (core, Automation, packaged). |
| DISC-12 | Mass ceiling 100 only for mass-doctrine formations with ≥ 32 earning members; back to 80 below that (core, Automation). Not reachable on the 20-unit skirmish map. |
| DISC-13 | **Deferred**: no Population Morale system exists; the baseline stays 50. |

## D. Decisions taken (provisional)

- **Company against isolation** is any living friendly soldier (same owner or alliance team) within the 400 cm support radius, member or not; civilians and enemies are not company.
- **Formation members are never isolated**; a stranded or independent member holds, as in DISC-D1.
- **Shock** applies to members of the fallen unit's own formation only, within 800 cm, measured at the step of the death; the per-unit window opens at that unit's first shock.
- **Recovery** below the baseline uses the fade rate (0.5 per evaluation); an earning member below the baseline gains at the normal +1.
- **Doctrine overrides** are a development and Automation tool; shipping builds take doctrine from faction IDs only.
- **No protocol change:** the rules are server-side and use the existing owner-only value and trend.

## E. Not covered

- **DISC-13:** faction baseline from Population Morale.
- **Any effect of Discipline** on defense, accuracy, reload, suppression, stance or regroup (DISC-D3).
- **Packaged mass ceiling:** needs a large map (300 units per participant); Automation only.
- **Real faction content:** the test map uses test faction IDs; Concord is exercised through the participant override.
- **A HUD cue for a lone soldier** beyond its value and trend.
- **Player playtest build:** the launcher's default package is not updated by this record.
