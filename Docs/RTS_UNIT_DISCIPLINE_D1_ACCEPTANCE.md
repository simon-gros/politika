# DISC-D1 — Discipline earned in formation: acceptance record

**Date:** 30 October 2026 · **Scope:** the DISC-D1 slice of [Military Discipline](RTS_UNIT_DISCIPLINE.md) §2–5 and §8: one resolve attribute per unit (Discipline on military units, Morale on civilians), earned by serving in a formation, held when not earning, fading back to the baseline outside a formation, Form refusing civilians, owner-only replication and HUD. Tested on the multiplayer skirmish test map at its owner cap of 20 units per participant. Discipline drives **no other attribute** in this slice. Stress, faction rules (D2), attribute effects (D3) and break consumers (D4) do not exist.

**Outcome: IMPLEMENTED and ACCEPTED for the skirmish test map; packaged formation-discipline PASS (10/10, two runs), Automation 42/42, regression matrix unchanged from the FORM-F4 baseline, and the protocol-4 client is now refused.**

## A. What changed (commit `a6e4d80`)

- **One attribute, two names.** `Source/PolitikaCore/Discipline.h` holds the rules. Every unit carries one value in integer milli-points (`0…100 000`), starting at the baseline of 50 points. Military units show it as Discipline and civilians as Morale. A unit archetype now has a `UnitCategory` (Military or NonCombat), appended with a Military default so every existing archetype keeps its meaning; no civilian archetype exists yet.
- **Earning (§3).** Every 20 authoritative steps (1 s), after the step's movement and casualties, the server evaluates every unit. A formation member *earns* when it is not independent of its formation's move and at least `min(2, members − 1)` other living members of the **same** formation stand within 400 cm. It gains 1 point, up to the ceiling of 80, and never past it. Any other member *holds*: no gain, no loss. A dead member gives no support, and members of another formation never count.
- **Outside a formation** a unit fades by 0.5 point per evaluation towards the baseline and stops there; a value already at or below the baseline does not change. Form, Release and an individual order change no value: they only change what the next evaluation does.
- **Independence.** A member made independent by an individual order (FORM-F3) holds. Independence exists once the formation has a move; before its first formation move every member counts as following. Autonomous stance chases (FORM-F4) do not make a member independent.
- **Form refuses civilians** with a new rejection code `NotMilitary` ("Civilians cannot form formations."), appended after the FORM-F3/F4 codes. A refused Form changes nothing.
- **Owner-only replication.** The value, its trend and the unit's category replicate to the owning connection only (`COND_OwnerOnly`); an opponent's copy of the unit keeps the default.
- **HUD.** A selected unit's line ends `DISCIPLINE 72 rising` (`fading`, or nothing when steady; `MORALE` on a civilian). A multi-selection, including a whole formation, shows `DISCIPLINE avg 64` over its military units.
- **Network.** `network_protocol` moves from 4 to 5; a protocol-4 client is refused at join.
- **Development logs.** Every 100 steps the server logs `RTS_DISCIPLINE_SAMPLE` per formation, `RTS_DISCIPLINE_PARTICIPANT` per participant and `RTS_DISCIPLINE_HOLDING` for each member that is not earning, plus `RTS_DISCIPLINE_BAND` when a unit crosses 60–100 or lands back on 50. The client's `MP_CLIENT_SAMPLE` now ends with `resolve=` for every unit it receives.
- **Harness.** `rts_multiplayer --formation-discipline` (below).

## B. Verification

**Identity.** Packages from clean commit `a6e4d80` (`git_dirty=false`, `network_protocol` 5, `game_version` 0.1.0-dev.0). Client `dev-a6e4d80/…/Politika` `1ba601a9c7c6bb71b1b2fdef14281e4d6952841caf0a249e4da3a93c7eab7468`; server `server-a6e4d80/…/PolitikaServer` `116f40712c09f1c99106f0960887f2939f39d39e19af282d91c22179052b78cc`. Evidence: `/mnt/data/politika-qualification/DISC-D1-20261030/final-a6e4d80/`.

| Check | Result |
|---|---|
| Portable C++20 suite (`tools/core-test.sh`): gain, ceiling, hold, fade and clamp per `Evaluate`; 30 evaluations from 50 reach 80 and 60 bring it back; `SupportCount` at and beyond 400 cm, unavailable members and out-of-range indices; band crossings; `RequiredSupport`; Form refusing a non-military member as `NotMilitary` | PASS (`core-test.out`) |
| `quality -- audit` (`input.bindings`: 26 order bindings, no shared chord; no new key) | PASS (`quality-audit.out`) |
| Editor build | exit 0 |
| Automation, focused: new `Politika.RTS.Multiplayer.FormationDiscipline` plus `FormationLifecycle`, `FormationStance` | 3/3 (`auto-focused-2`). `auto-focused-1` crashed on an engine assertion because the first version of the new test called `GetLifetimeReplicatedProps` directly; the test was corrected, no game code changed, and owner-only replication is proven by the packaged run instead |
| Automation, all Politika tests (`auto-all/`) | 42/42 (41 earlier + `FormationDiscipline`); state and warning count identical per test to the FORM-F4 baseline |
| Packaged `rts_multiplayer --formation-discipline`, two runs | 10/10 and 10/10 (`formation-discipline-1/`, `-2/`) |
| Regression matrix (`final-a6e4d80/summary.txt`) | Unchanged from the FORM-F4 baseline: formation stance 14/14, formation lifecycle 20/20, group-move shape 16/16, group orders 8/8, control groups 17/17, unit-scale 73/73, offline PAC-10 and participant authority PASS, dedicated 1v1 23/23, listen 1v1 18/18, listen layouts 74, lag-loss 47, engagement 9/9, fog memory 7/7. Dedicated layouts 75/75: the run supplied the FORM-F4 protocol-4 client and `MP2-PROTOCOL-LEGACY-REFUSED` passes (`MP_PRELOGIN REJECTED … protocol=4 … MP.PROTOCOL.MISMATCH`). The only non-PASS lines are the three planned NOT_RUNs (legacy client not supplied to listen layouts and lag-loss, no allies in 1v1) |

**Automation `FormationDiscipline`** (standalone 1v1 coordinator, no navmesh, evaluations stepped exactly): every unit starts at 50 and the civilian is NonCombat; Form with a civilian is refused as `NotMilitary` and creates nothing; units outside a formation stay at 50; Form changes no value; at the next evaluation each supported member gains exactly 1 point and shows Rising; two 2-unit formations whose members each stand next to the *other* formation's member get no support and hold; a stranded member (1050 cm away) holds and shows Steady while the others gain; after a formation move and an individual Stop the independent member holds while a following neighbour gains; a dead member gives no support; a member back in the ranks gains again and every following member stops at exactly 80 with a Steady trend; after Release values fade 0.5 per evaluation, reach 50.5 after 59 evaluations and 50 after 60, never below; the civilian stays at 50 throughout. Its 7 warnings are 6 expected `RTS_NAVIGATION_FAILED reason=NavigationUnavailable` and the SDL start-up line.

**Packaged formation-discipline run** (dedicated 1v1, 20 units per seat, fixture held idle, development stance probe armed, client A rendered on Xvfb at 1280×720 and driven with real X keyboard and mouse input through xdotool):

| Verdict | What it shows (run 1) |
|---|---|
| `FORM-FIXTURE-20-PER-SEAT` | 20 own units. |
| `DISC-01-BASELINE-AT-START` | Before any formation: 20 units, all at 50 000. |
| `DISC-SETUP-LINE-ARRIVED` | After T and a formation Move, 20/20 arrive at their slots, 0 navigation failures, minimum separation 149.3 cm. |
| `DISC-02-FORM-CHANGES-NOTHING` | Formed at step 203; no band crossing at or before Form; the first formation sample (step 300) reads 55 000 for all 20. |
| `DISC-03-MEMBERS-EARN-ONE-POINT-PER-SECOND` | Two consecutive samples 100 steps apart: all 20 earning, 60 000 → 65 000. |
| `DISC-04-INDEPENDENT-MEMBER-HOLDS` | Group 2 + X makes unit 1 independent at step 514; it holds at 65 000 over three samples while the formation's maximum climbs 70 000 → 75 000 → 80 000. |
| `DISC-03-CEILING-80` | All 19 following members cross 80 rising; nobody crosses 90 or 100; the next sample reads max 80 000, min 65 000 (unit 1), avg 79 250. |
| `DISC-07-OWNER-ONLY` | After H (Guard) the probe walks an opposing unit next to the formation, so the opposing client receives participant 1's units: 55 of its samples cover them and every one reads `resolve=50000`, while the owner's client logged 780 samples above 50 000. |
| `DISC-06-RELEASE-FADES-TO-BASELINE` | Y releases the formation at step 1008. The participant maximum falls exactly 2 500 per 100-step sample (77 500 → 50 000), the minimum never goes below 50 000, all 20 units land on 50 fading, and all are at 50 by step 2200 (1192 steps, 59.6 s, after Release). |
| `FORM-MATCH-STAYS-ACTIVE` | No objective capture or terminal phase during the run. |

Screenshots (`formation-discipline-*/client-a-*.png`): the independent unit selected (`DISCIPLINE 65`), the whole formation at the ceiling (`DISCIPLINE avg 79`, 19 at 80 and one held at 65), the released unit fading (`DISCIPLINE 63 fading`) and every unit back at 50. The HUD lines are legible and do not overlap at 1280×720.

**Repeatability.** Both runs formed at step 203, so the first evaluation after Form fell on step 220 in both. Relative to that evaluation, all 58 rising band crossings (19 members crossing 60, 70 and 80 after exactly 10, 20 and 30 evaluations, and unit 1 crossing 60) are identical between the runs. Relative to the first evaluation after Release, all 20 landings back on 50 are identical. Unit 1 held at 65 000 in both runs. Events triggered by real input differ by 1–5 steps between the runs (formation Move 212/207, Stop 514/513, Release 1008/1006), because input arrives at a slightly different step each time. This is a same-machine check, not cross-machine proof.

## C. Map to the DISC acceptance matrix

| ID | Status in this slice |
|---|---|
| DISC-01 | Every unit starts at 50 (Automation, packaged); Form refuses a civilian and changes nothing (core, Automation). No civilian archetype exists, so the packaged run has none. |
| DISC-02 | Form, selection, control groups and V change no value (Automation; packaged: no crossing at Form). |
| DISC-03 | +1 per evaluation, stopping at exactly 80 (core, Automation, packaged). |
| DISC-04 | Stranded and independent members hold (Automation); the independent member holds in the packaged run. |
| DISC-05 | A neighbouring formation and a dead member give no support (Automation). |
| DISC-06 | After Release values fade 0.5 per evaluation to 50 and never below (core, Automation, packaged). |
| DISC-07 | Owner-only: the opposing client receives participant 1's units with the default value only (packaged). |
| DISC-08 | Values belong to the match's units and go with them at Terminal/Return; no client request can change a value, so a stale match generation has nothing to change. |
| DISC-09 | Integer milli-points updated only on fixed steps (core); two packaged runs, see Repeatability. Same machine only. |
| DISC-10–13 | Not in scope (DISC-D2). |

## D. Decisions taken (provisional)

- **Support** counts living members of the same formation within 400 cm, including an independent member standing in the ranks; `RequiredSupport` is two, or every other member of a two-member formation.
- **Independence before the first formation move** does not exist: an individual order on a member of a formation that has never moved does not stop it earning, but walking away from the ranks does.
- **HUD:** the trend is a word (`rising`/`fading`) rather than an arrow glyph, and labels and the formation line are unchanged.
- **Logs:** values in milli-points; samples every 100 steps.

## E. Not covered

- **Any effect of Discipline** on defense, accuracy, reload, suppression, stance or regroup (DISC-D3).
- **Stress:** casualty shock, isolation below the baseline and the Concord exemption (DISC-D2).
- **Faction rules:** the Directorate/Ascendancy ceiling of 100 for ≥ 32 eligible members (DISC-D2; it cannot be reached on the 20-unit skirmish map) and a baseline from Population Morale.
- **Civilian units:** none exist yet, so Morale is visible only through Automation.
- **Save/load:** values are not saved.
- **Player playtest build:** the launcher's default package is not updated by this record.
