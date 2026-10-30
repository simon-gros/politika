# POLITIKA — Military Discipline: earned in formation

**Status: PROPOSED DESIGN — 30 October 2026. DISC-D1 IMPLEMENTED 30 October 2026** (the meter, formation gain, ceiling 80, decay, Form refusing civilians, owner-only HUD; see the [DISC-D1 record](RTS_UNIT_DISCIPLINE_D1_ACCEPTANCE.md)). **DISC-D2 IMPLEMENTED 30 October 2026** (casualty shock, isolation with the Concord exemption, the Directorate/Ascendancy mass ceiling; see the [DISC-D2 record](RTS_UNIT_DISCIPLINE_D2_ACCEPTANCE.md)) except the faction baseline from Population Morale (DISC-13), which waits for a Population Morale system. **DISC-D3 IMPLEMENTED 30 October 2026** (one per-unit modifier registry; Discipline drives damage taken, fire interval, reload time, return-to-post speed and the regroup delay; see the [DISC-D3 record](RTS_UNIT_DISCIPLINE_D3_ACCEPTANCE.md)) except spread, which waits for indirect fire. **DISC-D4 rout IMPLEMENTED 30 October 2026** (a soldier broken at 0 leaves its formation, stops fighting, refuses orders, flees to its side's start and rallies at 15; see §7A and the [DISC-D4 record](RTS_UNIT_DISCIPLINE_D4_ACCEPTANCE.md)); the other break consumers, Psy-Ops and Intimidation, arrive with their own systems. This document turns the per-unit resolve meter of [Propaganda and Morale](PROPAGANDA_AND_MORALE.md) §6 into a concrete, implementable rule for **military** units: Discipline is **earned by serving in a coherent formation** and fades outside one. DISC-D1 and D2 gave it **no effect on any other attribute**; §7 lists the attributes it drives from DISC-D3 and how they plug in.

**Authority:** [Propaganda and Morale](PROPAGANDA_AND_MORALE.md) owns the single `UnitResolve` meter and its break rules; [Military Formations, Tactical Stances, Cohesion and Discipline](RTS_UNIT_FORMATIONS_AND_DISCIPLINE.md) owns formations, stances and Cohesion (this document is its §6 made concrete and replaces the FORM-F5 sketch); [Engine Outline](ENGINE_OUTLINE.md) owns sequencing. The numbers below are first values for implementation and testing, not balance.

## 1. What Discipline is

- **One meter, not a second one.** Discipline is the name a *military* unit's `UnitResolve` shows in the HUD. Civilian and other non-combat units carry **the same attribute**, shown as **Morale**, with the same range, baseline, decay and break rules. There is no separate `UnitDiscipline` or `UnitMorale` component.
- **The one difference: civilians cannot form formations.** Form refuses any non-combat unit, so civilians never earn the formation gain. Everything else that moves the meter (decay towards the baseline, casualties, propaganda, Psy-Ops, Intimidation) applies to both.
- **Every military unit has it from spawn.** A unit outside any formation is not "without discipline"; it sits at its **baseline**. Formation service raises it above the baseline. This keeps the existing canon intact: Psy-Ops and Intimidation break the meter of *any* eligible unit, and the Atlantic Concord's "no penalty for operating solo" rule needs a meter that solo units have.
- **It is earned over time, not granted on joining.** Pressing T does nothing to the value. Members gain Discipline while they actually serve together: following the formation and standing close to fellow members. Selection, control groups and the V shape never contribute.
- **It persists on the unit.** A unit that leaves a formation keeps what it earned and loses it slowly, so a disciplined squad that breaks ranks for a moment is still disciplined, and a long-dissolved one is back to its baseline.

## 2. Values

All values are authoritative integers in **milli-points** (`0…100 000` for 0–100), updated in fixed steps on the server, so the result is the same on every run of the same schedule.

| Quantity | Value | Notes |
|---|---|---|
| Range | 0–100 | `UnitResolve` maximum 100 |
| Baseline `B` | 50 | Later from faction Population Morale (§6); a constant in DISC-D1 and DISC-D2 |
| Broken threshold | 0 | A military unit at 0 routs (DISC-D4, §7A) |
| Rally threshold (DISC-D4) | 15 | A routed unit recovers +0.5 per second wherever it is and rallies here: 0 → 15 takes 30 s |
| Spawn value | `B` | |
| Evaluation cadence | every 20 steps (1 s at 20 steps/s) | Cheap and deterministic; no per-frame updates |
| Formation gain | +1.0 per second while **eligible** (§3) | 50 → 80 takes 30 s of service |
| Formation ceiling | `B + 30` = 80 (normal) | Directorate/Ascendancy mass bonus raises it to 100 (§6) |
| Decay outside formation | −0.5 per second towards `B`, never below `B` | 80 → 50 takes 60 s; earned Discipline outlasts a short break |
| Recovery outside formation (DISC-D2) | +0.5 per second back to `B` from below, when not alone | 30 → 50 takes 40 s |
| Decay while in formation but not eligible | none | Holding steady; an out-of-place member neither gains nor loses |
| Above a lower ceiling (DISC-D2) | −0.5 per second down to the ceiling while earning | A mass formation that drops below 32 earning members fades from 100 back to 80 |
| Isolation floor (DISC-D2) | 30 | A soldier alone (§3A) fades at −0.5 per second to 30 and holds there; 50 → 30 takes 40 s |
| Casualty shock (DISC-D2) | −5 per fallen fellow member within 800 cm | At most −15 per unit in a 5 s (100-step) window that opens at its first shock; never below 0 |

Display rounds down to whole points. The HUD shows the value and its trend as a word (`rising`, `fading`, nothing when steady).

## 3. When a member earns Discipline

At each evaluation a unit is **eligible** only if all of these hold:

1. It is a military unit, alive, and under its owner's control.
2. It is a member of a formation, and **following** it. A member made independent by an individual order (FORM-F3) is not acting as part of the formation and does not gain. Independence exists once the formation has a move: before its first formation move every member counts as following. Autonomous stance chases (FORM-F4) do not make a member independent, so a Guard member who chases and returns keeps earning.
3. It has **support**: at least `min(2, members − 1)` other members of the **same formation** within **400 cm** of it. At the 150 cm formation spacing every member of a closed Line or Column has two neighbours in range. A member stranded by a long route, a chase or a failed slot does not gain until it rejoins.

Rules that keep the gain honest:

- **Only the unit's own formation supports it.** Two formations standing side by side do not support each other, and a unit is counted once per evaluation however many members are near it.
- **Dead, unavailable or captured members give no support**, even if their formation slot is geometrically correct.
- **Formation state:** an Organized or Degraded formation earns. The FORM-F3 regroup re-establishes support naturally; there is no extra bonus for regrouping.
- **Adherence and stance do not change the gain** in DISC-D1. (Later doctrine may, for example, let Maintain earn faster; see §8.)
- **Free adherence still earns** when members happen to stay together, because support is measured, not assumed.

## 3A. Stress (DISC-D2)

**Isolation.** At each evaluation a **military** unit that is **not a formation member** and has **no living friendly military unit within 400 cm** is *alone*. Friendly means the same owner, or the same alliance team. Enemy soldiers, civilians and dead units never count as company. A lone unit fades at −0.5 per second towards the isolation floor of 30 (from above the baseline it fades through it), and holds at 30. As soon as a comrade is within 400 cm it is no longer alone and returns to `B` at 0.5 per second. Exceptions:

- **Atlantic Concord** units (solo-tolerant doctrine) are never penalized for being alone: they return to `B` like any other unit outside a formation.
- **Civilians** have no isolation drift: they cannot form formations and have no formation support to lose.
- **Formation members are never isolated**, even when stranded or independent: membership is the support. A stranded member holds (§3).

**Casualty shock.** When a formation member dies, every **surviving member of the same formation** within **800 cm** of where it fell loses 5 points at once, in the step of the death. Each unit has its own window: its first shock opens a 100-step (5 s) window in which it loses at most 15 points; the next shock after the window opens a new one. Shock never takes the meter below 0. Non-members nearby, other formations and opponents are not shocked. Shock applies to every faction, including the Concord. Members dying in the same step do not shock each other.

**Mass formations.** For a Red Directorate or Iron Ascendancy formation, the ceiling for its earning members is 100 at an evaluation where **at least 32 members earn** (§3), and 80 otherwise. A member above the ceiling (because the formation shrank) fades down to it at 0.5 per second; a holding member keeps its value.

**Doctrine.** A unit's doctrine comes from its content faction ID: `faction.red_directorate` and `faction.iron_ascendancy` are mass-formation, `faction.atlantic_concord` is solo-tolerant, everything else (Meridian Pact, Unaligned and the prototype test factions) is standard. Development builds accept `-PolitikaMPDisciplineDoctrine=<participant>:<standard|mass|solo>[,…]` so the test map's test factions can stand in for real ones.

## 4. Lifecycle

| Event | Effect on Discipline |
|---|---|
| Spawn | Starts at `B` |
| Form (T) | No change; earning starts at the next evaluation if eligible |
| Release (Y), dissolve, becoming the last member | No immediate change; decay towards `B` starts |
| Member death | Its value disappears with it. Surviving members of its formation within 800 cm take casualty shock (DISC-D2, §3A) |
| Reaching 0 (DISC-D4) | The soldier routs in that step (§7A): it leaves its formation, which regroups or dissolves as after any loss |
| Rally (DISC-D4) | At 15 the soldier is idle where it stands, outside any formation, and takes orders again; the usual rules move its value from there |
| Individual order on a member | Stops earning until the next formation move makes it follow again; no loss |
| Joining a different formation | Keeps its value; earns under the new formation |
| Match Terminal / Return | Values are dropped with the match; not saved |

## 5. Authority, network and presentation

- **Server-owned.** The value lives on the authoritative unit and is updated only in the fixed-step simulation, after movement and before event publication, so a step's support uses that step's positions. Clients never compute it.
- **Owner-only exact value.** The owning player receives each own unit's value and trend. Opponents and allies receive nothing in DISC-D1; a coarse visible band for enemies (for example "steady / shaken") belongs with intelligence rules later.
- **Replication** adds the value to the existing owner-only unit state; `network_protocol` 4 → 5. DISC-D4 adds the owner-only routed flag and the `Routed` order rejection; `network_protocol` 5 → 6.
- **HUD:** a selected unit's line ends `DISCIPLINE 72 rising` (`MORALE` on a civilian); a multi-selection, including a whole formation, shows `DISCIPLINE avg 64` over its military units. Unit labels and the formation line are unchanged, to avoid more label clutter.
- **Development logs:** every 100 steps `RTS_DISCIPLINE_SAMPLE` per formation (members, earning, holding, min/avg/max), `RTS_DISCIPLINE_PARTICIPANT` per participant (members and units outside formations) and `RTS_DISCIPLINE_HOLDING` for each member that is not earning; `RTS_DISCIPLINE_BAND` when a unit crosses 60, 70, 80, 90 or 100, falls through 40, 30, 20, 10 or 0, or lands back on `B` from either side. DISC-D2 adds `RTS_DISCIPLINE_SHOCK` for every surviving member at each formation casualty (in range or not, with distance and the window), `RTS_DISCIPLINE_ISOLATED` every 100 steps for each lone soldier (with its doctrine and whether it is exempt), `RTS_DISCIPLINE_MASS` every 100 steps for a formation at the mass ceiling, and `RTS_DISCIPLINE_DOCTRINE_OVERRIDE` for a development override. The client's `MP_CLIENT_SAMPLE` carries `resolve=` for every unit it receives. No per-unit log every evaluation.

## 6. Faction rules (canon) and when they arrive

These are the existing binding rules from [Propaganda and Morale](PROPAGANDA_AND_MORALE.md) and the formations document §6, expressed in this model. **DISC-D2 implements** the mass bonus and the Concord exemption (§3A); the faction baseline from Population Morale waits for a Population Morale system, so `B` is still the constant 50 for every faction.

| Faction | Rule in this model |
|---|---|
| Red Directorate, Iron Ascendancy | Normal formation gain and ceiling **plus** a mass bonus: when a formation has **≥ 32** eligible members, its ceiling rises by +20 to the maximum of **100**. The bonus counts eligible members only, not faction totals. |
| Meridian Pact, Unaligned (NPC) | Normal gain and ceiling; no mass bonus. |
| Atlantic Concord | Normal gain and ceiling. **Exempt from the isolation penalty** (DISC-D2): a Concord unit alone never drifts below `B` for being alone. |
| Civilians / non-combat | Same attribute, shown as Morale. They cannot be formation members (Form refuses them), so they never earn the formation gain; propaganda and Population Morale move their meter instead. |

The 32-member threshold cannot be reached on the skirmish test map (20 units per player); it is tested in Automation and belongs to the large maps (300 units per player).

## 7. What Discipline affects (DISC-D3)

Every effect reads one normalized factor, `F = (D − 50) / 50`, in `[−1, +1]`: `0` at baseline, `+0.6` at the normal ceiling of 80 and `+1` at the Directorate/Ascendancy ceiling of 100. Effects scale linearly with `F`. Each consuming system has its own coefficient and cap, is listed in one modifier registry with a source ID, and never stacks Discipline twice.

| Attribute | At 80 (`F = +0.6`) | At 100 (`F = +1`) | Arrives with |
|---|---|---|---|
| **Defense** (damage taken from every weapon hit) | −12% | −20% | DISC-D3 |
| Weapon accuracy / spread | spread −12% | spread −20% | DISC-D3 registers it; consumed with indirect fire (direct fire never rolls) |
| Aim (fire interval) and reload time | −9% | −15% | DISC-D3 |
| Suppression resistance | +18% | +30% | Suppression system |
| Stance obedience | Guard/Aggressive return to post 20% faster | 33% faster | DISC-D3 |
| Regroup | Regroup delay 40 → 31 steps | 40 → 25 steps | DISC-D3 |
| Break threshold consumers | Psy-Ops, Intimidation and rout need the meter to fall from its current value, so high Discipline buys time | | Rout DISC-D4 (§7A); Psy-Ops and Intimidation with their systems |

Below baseline (isolation, casualties; DISC-D2) the same coefficients apply with the negative factor, so a shaken unit takes more damage and shoots worse. **Defense** is a multiplier on damage taken, applied once per hit after armour and before health; it is not extra health and never reaches immunity (its largest reduction is the 20% above). Civilian Morale does not drive combat effects; its later effects (work rate, panic) belong to [Propaganda and Morale](PROPAGANDA_AND_MORALE.md).

**As implemented (DISC-D3).** `F` is integer permille. Each multiplier is `1000 + F × coefficient` permille, truncated toward zero: damage taken and spread −1/5, fire interval and reload time −3/20, return speed +1/3, regroup delay −3/8 (rounded to the nearest step, at least one). The unit's registry (`Modifiers.h`) holds one entry per source and attribute and is rewritten when Discipline changes. Weapon damage is applied in milli-health and the fire interval in milli-steps, each with the fraction carried per unit, so a −12% on an 8-point hit or a 4-step interval is not lost to rounding. The regroup delay uses the surviving members' average after that step's casualty shock. At exactly 50 every quantity is unchanged.

## 7A. Rout (DISC-D4)

The broken threshold's first consumer. Today only casualty shock (§3A) lowers a member's value far enough, so a formation that keeps losing men breaks from the inside: the members near the casualties go first.

| Rule | Value |
|---|---|
| Break | A **military** unit whose value is at or below 0 breaks in that step. Breaks are checked once per step after the step's casualties and before the evaluation, in stable-ID order, so a member shocked to 0 cannot earn its way out first. Civilians at Morale 0 never rout; their break consumers are Intimidation and Psy-Ops. |
| Formation | The routed member leaves its formation (`LeaveReason::Routed`). The formation treats it as a loss: it regroups after the Discipline-scaled delay of §7, or dissolves below two members. |
| Flight | The unit drops its order, target, grenade and engagement and flees to its participant's authored start (the unit's spawn point when the participant has none) as a plain Move, which never acquires a target. Without a route it holds where it is. After the flight it stays where it ended; a routed unit has no stance behaviour. |
| Fighting | A routed unit never acquires or fires, whatever its stance or order history. It can still be shot; at 0 it takes 20% more damage (§7). |
| Orders | Every order from its owner is refused with `OrderRejection::Routed` (19), checked after ownership so no other participant learns the state. Form refuses it as an unavailable member. A stance setting is accepted and applies after the rally. |
| Recovery | +0.5 per evaluation wherever it is, alone or not, no casualty shock (it is no member). |
| Rally | At 15 (30 evaluations after a break at 0) the unit rallies where it stands: idle, outside any formation, with a fresh post, taking orders again. From there the usual rules apply: beside a comrade it returns to 50, alone it holds below the isolation floor until it has company. |
| Network and HUD | The routed flag is owner-only replicated; an opponent sees the unit run, never the state. A selected routed unit's line reads `ROUTED` instead of its order; a selection line adds `N ROUTED`. A refused order shows "That unit is routing and will not take orders." |
| Logs | `RTS_DISCIPLINE_ROUT phase=break` (value, formation, previous order, route, rally point and distance), `phase=flight_end` (arrival or obstruction) and `phase=rally` (steps routed, distance to the rally point); `RTS_FORMATION_MEMBER_LEFT reason=routed`; `RTS_ORDER_REJECTED outcome=19`; `MP_CLIENT_SAMPLE routed=`. |

Rout is not contagious in DISC-D4: a breaking soldier does not shock its former comrades (§10).

## 8. Delivery slices

| Slice | Scope | Acceptance focus |
|---|---|---|
| **DISC-D1** | Meter on military units (spawn at `B`), a combat/non-combat unit category with Form refusing non-combat units, formation gain / ceiling / decay (§2–4), owner-only replication, HUD, logs, `network_protocol` 5. **No effect on any other attribute.** | DISC-01 to DISC-09 |
| DISC-D2 | Stress: casualty shock to nearby members (−5, capped −15 per 5 s), isolation drift below `B` (floor 30) with the Concord exemption, faction mass bonus, faction baseline from Population Morale. **Implemented 30 Oct 2026** except the Population Morale baseline | DISC-10 to DISC-12 accepted; DISC-13 deferred |
| DISC-D3 | First effects (§7): defense, accuracy, aim/reload, stance return speed, regroup delay. **Implemented 30 Oct 2026** except spread (no consumer until indirect fire) | DISC-14, DISC-16 to DISC-19 accepted; DISC-15 deferred |
| DISC-D4 | Break threshold consumers. **Rout implemented 30 Oct 2026** (§7A); Psy-Ops and Intimidation arrive with their own systems ([Espionage and Intimidation](ESPIONAGE_AND_INTIMIDATION.md)) | DISC-20 to DISC-26 accepted; Psy-Ops and Intimidation by their own acceptance |

Possible later doctrine, not decided: Maintain adherence earning faster; veteran units keeping a higher personal floor; commander talents and research raising the ceiling ([Commander Talents](COMMANDER_TALENT_SYSTEM.md), [Research](RTS_TECHNOLOGICAL_RESEARCH.md)).

## 9. Acceptance matrix (DISC-D1 to DISC-D4)

| ID | Requirement |
|---|---|
| DISC-01 | Every unit spawns at `B` (Discipline for military, Morale for civilians); Form refuses a non-combat unit and changes nothing. |
| DISC-02 | Form, selection, control groups and V change no value. |
| DISC-03 | A following member with support gains exactly +1 per evaluation up to the ceiling, and never exceeds it. |
| DISC-04 | A member without support (stranded, beyond 400 cm) and an independent member do not gain and do not lose. |
| DISC-05 | Two adjacent formations do not support each other; dead or unavailable members give no support. |
| DISC-06 | After Release, values decay at −0.5 per second to `B` and never below it. |
| DISC-07 | Only the owner receives values; the opposing client logs none. |
| DISC-08 | Values are dropped at Terminal/Return; a stale match generation cannot change them. |
| DISC-09 | Two runs of the same schedule give identical values at identical steps; values are integer milli-points. |
| DISC-10 | (D2) Casualty shock reaches only members within range and respects the cap. |
| DISC-11 | (D2) Isolation drift applies to lone units except Concord units. |
| DISC-12 | (D2) The mass bonus (ceiling 100) applies only to Directorate/Ascendancy formations with ≥ 32 eligible members. |
| DISC-13 | (D2) Faction baseline follows Population Morale within its documented bounds. |
| DISC-14 | (D3) Damage taken from every weapon hit scales with `F` per §7, once per hit, and never falls below 80% of the hit. |
| DISC-15 | (D3) Spread scales with `F` per §7 for weapons that roll scatter. |
| DISC-16 | (D3) Fire interval and reload time scale with `F` per §7. |
| DISC-17 | (D3) Guard/Aggressive return-to-post speed scales with `F` per §7. |
| DISC-18 | (D3) A formation's regroup delay follows its survivors' average Discipline per §7. |
| DISC-19 | (D3) At the baseline every effect is neutral and earlier behaviour is unchanged; civilian Morale drives no combat effect; Discipline enters each attribute once. |
| DISC-20 | (D4) A military unit breaks in the step its value reaches 0, and only then; a civilian never routs. |
| DISC-21 | (D4) A routed member leaves its formation as `routed` in that step and takes no later casualty shock. |
| DISC-22 | (D4) A routed unit flees to its participant's start along a route, or holds where it is without one. |
| DISC-23 | (D4) A routed unit never acquires or fires. |
| DISC-24 | (D4) Its owner's orders are refused with `Routed`; another participant never learns the state (owner-only flag, ownership checked first). |
| DISC-25 | (D4) Form refuses a routed unit. |
| DISC-26 | (D4) It recovers 0.5 per evaluation and rallies at 15, 30 evaluations after a break at 0, idle and taking orders again. |

**DISC-D4 packaged check (skirmish map, 20 units):** form 20 units into a Line; with the rout seed, the casualty fixture sets its members to 10 and kills four at one end; confirm the members shocked to 0 break in that step and only those, leave the formation as routed and flee towards seat 1's start without firing; select the group and order a move with real input: the routed members are refused as `Routed` and the rest accepted; with the witness fixture walking an opposing soldier into sight of the start, confirm the opponent's client receives the routed units but never their routed flag; confirm each rallies at 15 after 30 evaluations and the same order is then accepted by every unit.

**DISC-D3 packaged check (skirmish map, 20 units):** form 20 units into a Line and let them earn 80; with the opponent held at 50, walk one opposing soldier into the Line under Guard and confirm each hit on a member is scaled to 88% while the opponent's losses are unscaled, members fire on a shorter interval than the opponent's, and members walk back to their posts 20% faster; trigger the casualty fixture and confirm the regroup comes on the scaled step; reload and confirm the shorter reload; confirm the opponent's units log no effect and its client still receives only the default.

**DISC-D2 packaged check (skirmish map, 20 units):** send one soldier far from the army and confirm it fades 2.5 per 5 s to 30 and holds; with the opponent on the solo-tolerant doctrine, walk one opposing soldier away from its comrades and confirm it stays at 50; trigger four casualties at one end of a 19-unit Line and confirm the shock reaches only members within 800 cm and stops at 15; walk the lone soldier back and confirm it recovers to 50 in 40 evaluations; confirm the opposing client still receives only the default. The 32-member mass ceiling cannot be reached on this map (20 per participant) and is accepted by Automation.

**DISC-D1 packaged check (skirmish map, 20 units):** form 20 units into a Line and move them; sample the server log until the members reach 80 (about 30 s of service); confirm the HUD shows `DISCIPLINE 80`; make one member independent with an individual order and confirm it stops gaining; send one member beyond 400 cm and confirm it holds; release and confirm decay to 50 over about 60 s; confirm the opposing client logs nothing.

## 10. Open decisions

- Rates, ceiling and decay above are first values; tune now that DISC-D3 effects make them matter.
- DISC-D3 coefficients are the first values of §7, not balance. Whether a HUD should show the effect (for example "−12% damage") beside the value.
- Whether opponents may see a coarse band of visible enemy units' Discipline (intelligence rules).
- Whether Discipline should be saved with a persistent campaign army (no campaign persistence exists yet).
- Whether vehicles and crewed weapons earn Discipline the same way as infantry, or through their crew.
- DISC-D2 first values: shock radius 800 cm and shock to formation members only; allied soldiers count as company against isolation; isolation floor 30; recovery at the fade rate. Tune with DISC-D3.
- Whether lone soldiers should get a coarse HUD cue ("alone"); DISC-D2 shows only the value and its trend.
- DISC-D4 first values: break at 0, rally at 15 (30 s), recovery at the fade rate anywhere, flight to the participant's authored start. Open: whether rout is contagious (a breaking soldier shocking nearby comrades), whether a rallied soldier should return to its old formation, whether routing should be faster than walking, whether the flight should prefer a nearby friendly formation or base once bases exist, and whether opponents should see that a unit is routing (intelligence rules).
