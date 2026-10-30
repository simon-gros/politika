# DISC-D3 — Discipline's first effects: acceptance record

**Date:** 30 October 2026 · **Scope:** the DISC-D3 slice of [Military Discipline](RTS_UNIT_DISCIPLINE.md) §7–8: one per-unit modifier registry, and Discipline driving damage taken (defense), fire interval and reload time, return-to-post speed and the formation regroup delay. Tested on the multiplayer skirmish test map at its owner cap of 20 units per participant. **Spread is not consumed**: direct fire is deterministic with no hit roll ([Unit Specification Framework](UNIT_SPECIFICATION_FRAMEWORK.md) §6.6) and no indirect-fire weapon exists, so the spread entry is registered for a later scatter consumer and changes nothing yet. DISC-13 (the Population Morale baseline) stays deferred; the baseline is still 50.

**Outcome: IMPLEMENTED and ACCEPTED for the skirmish test map, except spread (no consumer; deferred with indirect fire); packaged discipline-effects PASS (11/11, two runs with identical effect lines apart from input timing), Automation 44/44, regression matrix unchanged from the DISC-D2 baseline including the DISC-D1 and DISC-D2 checks, and the protocol-4 client is still refused.**

## A. What changed (commit `0623ba4`)

- **Modifier registry** (`Source/PolitikaCore/Modifiers.h`). Each unit holds one integer permille multiplier per *source* and *attribute* (1000 = no change). Setting a source's entry replaces the previous one, so a source can never stack with itself; consumers read the product of all sources once per use. Attributes: damage taken, spread, fire interval, reload time, return speed, regroup delay. Discipline is the only source.
- **Effect formula.** One factor `F = (D − 50) / 50` in permille (0 at 50, +600 at 80, +1000 at 100, −400 at 30). Each attribute's multiplier is `1000 + F × coefficient`, truncated toward zero:

  | Attribute | Coefficient | At 30 | At 80 | At 100 |
  |---|---|---|---|---|
  | Damage taken | −1/5 | 1080 | 880 | 800 |
  | Spread (no consumer) | −1/5 | 1080 | 880 | 800 |
  | Fire interval | −3/20 | 1060 | 910 | 850 |
  | Reload time | −3/20 | 1060 | 910 | 850 |
  | Return-to-post speed | +1/3 | 867 | 1200 | 1333 |
  | Regroup delay (40 steps) | −3/8 | 46 steps | 31 steps | 25 steps |

  The registry is rewritten whenever a unit's Discipline changes. **Civilian Morale drives no combat effect**: a non-combat unit's entries stay neutral.
- **Defense.** Every weapon hit, direct or blast, goes through `ReceiveWeaponHit`: the damage is converted to integer milli-health, scaled, and accumulated; health loses the whole points and each unit carries the fraction to its next hit. Without the carry the effect would vanish at the current rifle (8 × 0.88 = 7.04 would round back up to 8). Direct fixture damage still rounds up as before.
- **Fire interval.** The weapon interval in whole authoritative steps is scaled and the fraction of a step is carried to the next shot (an interval of 4 steps at 80 runs 3.64 on average: 3s and 4s). The shot presentation uses the scaled interval.
- **Reload.** The magazine reload time is scaled when the reload starts; `INFANTRY_RELOAD_BEGIN/COMPLETE` now carry the step.
- **Return to post.** Under the Guard and Aggressive stances (FORM-F4), the walk back to the post moves at the scaled speed; chase speed is unchanged.
- **Regroup delay.** When a formation member falls, the server averages the surviving members' Discipline *after* that step's casualty shock (DISC-D2) and passes the scaled delay to `FormationRegistry::MemberLost`, which also uses it to space consecutive regroups.
- **Baseline unchanged.** At exactly 50 every multiplier is 1000, no fraction is carried, and fire interval, reload, return speed and regroup keep their previous integer step counts.
- **Logs.** `RTS_DISCIPLINE_EFFECT effect=<attribute>` for every use whose multiplier is not neutral, with the unit's value, the permille and the base and scaled quantity. Nothing is logged at the baseline.
- **Network and HUD.** No replicated field changed; `network_protocol` stays 5. The HUD is unchanged.
- **Tests.** Automation `Politika.RTS.Multiplayer.DisciplineEffects` (new); `FormationLifecycle` and the packaged `--formation-lifecycle` harness now expect the Discipline-scaled regroup window (their survivors sit below 50 after the casualty shock) and check it against the formula and the server's `regroup_delay` line. New packaged harness `rts_multiplayer --discipline-effects`.

## B. Verification

**Identity.** Packages from clean commit `0623ba4` (`git_dirty=false`, `network_protocol` 5, `game_version` 0.1.0-dev.0). Client `dev-0623ba4/…/Politika` `c7b9fb1c7d2471fc5e2c514ca9b59db113237f5ba2ee640bd16a3e5c2ac9d640`; server `server-0623ba4/…/PolitikaServer` `8f88ce6d5c17c4b6a7b8b5b4a4774c346b566d7ac5878deb3eba99bb1dba4f8f`. Evidence: `/mnt/data/politika-qualification/DISC-D3-20261030/final-0623ba4/`.

| Check | Result |
|---|---|
| Portable C++20 suite (`tools/core-test.sh`): factor at 0/30/50/80/100, the §7 columns at 80 and 100, the negative side below 50 and never immunity, monotonic in Discipline, regroup 40 → 31 / 25 / 46 / 55 steps and never below one, the registry replacing rather than stacking a source, civilian entries neutral | PASS (`core-test.out`) |
| `quality -- audit` | PASS (`quality-audit.out`) |
| Editor build | exit 0 |
| Automation, focused: `DisciplineEffects`, `DisciplineStress`, `FormationDiscipline` and the four infantry combat tests (`auto-focused-1`), then `DisciplineEffects`, `FormationLifecycle` and `MagazineGrenadeSprintAndAuthority` after the fix (`auto-focused-2`) | 7/7 and 3/3. The first full run (`auto-all-pre`) failed `FormationLifecycle` only, on its hard-coded 40-step regroup; it and the packaged `--formation-lifecycle` harness now expect the scaled window |
| Automation, all Politika tests (`auto-all/`), after an editor build at `0623ba4` that reported the target up to date | 44/44 (43 earlier + `DisciplineEffects`); state and warning count identical per test to the DISC-D2 baseline, including `FormationLifecycle` with its updated expectation; the new test's only warning is the SDL start-up line |
| Packaged `rts_multiplayer --discipline-effects`, two runs | 11/11 and 11/11 (`discipline-effects-1/`, `-2/`) |
| Regression matrix (`final-0623ba4/summary.txt`) | Unchanged from the DISC-D2 baseline: discipline stress 10/10, formation discipline 10/10, formation stance 14/14, formation lifecycle 20/20 (now checking the scaled regroup window), group-move shape 16/16, group orders 8/8, control groups 17/17, unit-scale 73/73, offline PAC-10 and participant authority PASS, dedicated 1v1 23/23, listen 1v1 18/18, listen layouts 74, lag-loss 47, engagement 9/9, fog memory 7/7. Dedicated layouts 75/75 with the FORM-F4 protocol-4 client: `MP2-PROTOCOL-LEGACY-REFUSED` passes (`MP_PRELOGIN REJECTED … protocol=4 …`). The only non-PASS lines are the three planned NOT_RUNs (legacy client not supplied to listen layouts and lag-loss, no allies in 1v1) |

**Automation `DisciplineEffects`** (standalone 1v1 coordinator, exact steps): three host soldiers at earned values — a formation member at 80, one of a supported pair at 50 and a lone soldier at the isolation floor of 30 — each take seven 10-damage hits from an opposing Concord soldier at the baseline: they lose 61, 70 and 75 health (8.8 and 10.8 per hit with the fraction carried). Firing a 10-step weapon at baseline dummies for 182 steps they land 20, 19 and 18 shots. Their 3 s reloads take 2.73, 3.00 and 3.18 s. Walking back to a post at 300 cm/s they cover 180, 150 and 130 cm in ten steps. One member of each of two formations falls: the formation whose survivors average 75 after the shock closes ranks after 33 steps, the one at 45 after 41. A civilian's entries stay neutral.

**Packaged discipline-effects run** (dedicated 1v1, 20 units per seat, fixture held idle, stance probe and casualty fixture armed, participant 2 on the solo-tolerant override so it stays at the baseline as a reference, client A rendered on Xvfb at 1280×720 and driven with real X keyboard and mouse input through xdotool):

| Verdict | What it shows (run 1) |
|---|---|
| `FORM-FIXTURE-20-PER-SEAT` | 20 own units. |
| `DISC-SETUP-LINE-ARRIVED` | Ctrl+1, T and a right-click move put the 20 into a Line: 20/20 at their slots, 0 failures, minimum separation 149.3 cm. |
| `DISC-D3-SETUP-EARNED-80` | The formation sample at step 800: 20 members earning, minimum, average and maximum 80 000. |
| `DISC-D3-DEFENSE-SCALES-HITS` | H walks opposing probe unit 3 into the Line. Its 6 hits on members, all at 79 or more, are each logged at 880 permille: 8 000 base milli-health → 7 040 applied. Member 20 took 42 240 milli-health over six hits and lost 42 health; the probe at the baseline lost 8 per hit (16 in a step with two hits), unscaled. |
| `DISC-D3-FIRE-INTERVAL-SHORTER` | 30 member shots by 4 members, each at 910 permille: base interval 4 steps, scaled 3 or 4 with the fraction carried (13 at 3, 17 at 4); 12 consecutive shots exactly one scaled interval apart below the base. The baseline probe fired at gaps of 4, 4, 4, 4, 4. |
| `DISC-D3-RETURN-FASTER` | 4 members walking back to their posts at 1200 permille: measured 12.6 cm per step against 10.5 at the baseline (252 cm/s instead of 210). |
| `DISC-D3-REGROUP-SOONER` | J arms the casualty fixture: victims 22, 20, 14, 21. The first loss leaves 19 survivors averaging 77 105 after the shock: 797 permille, regroup delay 32 steps instead of 40; the regroup ran exactly on its due step (1097) with 16 members, 0 unplaced, minimum separation 150 cm. |
| `DISC-D3-RELOAD-SHORTER` | R: 2 reloads by members at 68 000 (after the shock), 946 permille: 2.838 s, measured 57 steps against the baseline 60. |
| `DISC-D3-BASELINE-NEUTRAL` | 43 effect lines, none naming participant 2 and none at 50 000; participant 2 stayed at 50 000 for the whole run. |
| `DISC-07-OWNER-ONLY` | The opposing client received participant 1's units in 59 samples, every one at 50 000, while the owner's client logged 1 056 samples above 50 000. |
| `FORM-MATCH-STAYS-ACTIVE` | No objective capture or terminal phase. |

Screenshots (`discipline-effects-*/client-a-*.png`): the formation at `DISCIPLINE 80`, after the probe exchange, and after the casualties.

**Repeatability.** Both runs log the same 43 effect lines in the same order with the same units, values, permille and quantities: the same 6 scaled hits and 30 scaled shots, the same four returns at 12.6 cm per step, the same four victims and the same 32-step regroup with 16 members. Every step is 4 lower in run 2 because H arrived 4 steps earlier. The only differing values are the two reloads: R is pressed after a wait on real input, and in run 2 the two reloading members stood at 67 000 instead of 68 000 when it arrived (949 permille, 2.847 s); both runs measured 57 steps against the baseline 60. This is a same-machine check, not cross-machine proof.

## C. Map to the DISC-D3 acceptance

| ID | Status |
|---|---|
| DISC-14 | Damage taken scales with Discipline per §7, once per hit, never below 80% (core, Automation, packaged). |
| DISC-15 | Spread is registered but **deferred**: no weapon rolls for accuracy until indirect fire exists. |
| DISC-16 | Fire interval and reload time scale per §7 with the fraction of a step carried (core, Automation, packaged). |
| DISC-17 | Return-to-post speed scales per §7 under Guard and Aggressive (core, Automation, packaged). |
| DISC-18 | Regroup delay follows the survivors' average after the shock (core, Automation, packaged; FORM-F3 lifecycle checks updated). |
| DISC-19 | At the baseline every multiplier is neutral and the earlier regression results are unchanged; civilian Morale drives nothing; Discipline enters the registry once (core, Automation, matrix). |

## D. Decisions taken (provisional)

- **Fractions are carried, not rounded:** weapon damage in milli-health and fire interval in milli-steps per unit, so small multipliers act on small integer quantities. Fixture (non-weapon) damage keeps rounding up.
- **"Aim time" is the fire interval:** the prototype has no separate aim phase.
- **Regroup from the survivors' average** after the step's casualty shock; one delay per scheduled regroup, also used as the spacing between regroups.
- **Return speed only:** Discipline does not change chase speed or ordinary movement.
- **Truncation toward zero** for every permille; regroup rounds to the nearest step, at least one.
- **No protocol change:** the effects are server-side; the owner still sees only the value and trend.

## E. Not covered

- **Spread (DISC-15):** waits for indirect fire.
- **DISC-13:** faction baseline from Population Morale.
- **Packaged effects at 100:** the mass ceiling needs a large map; the 100 column is covered by the core suite.
- **Suppression resistance and break consumers:** Suppression system and DISC-D4.
- **A HUD readout of the effects** beyond the value and trend.
- **Balance:** the coefficients are the first values of §7, not tuned.
- **Player playtest build:** the launcher's default package is not updated by this record.
