# POLITIKA — Military Discipline: earned in formation

**Status: PROPOSED DESIGN — 30 October 2026. Not implemented.** This document turns the per-unit resolve meter of [Propaganda and Morale](PROPAGANDA_AND_MORALE.md) §6 into a concrete, implementable rule for **military** units: Discipline is **earned by serving in a coherent formation** and fades outside one. In this first stage it has **no effect on any other attribute**; §7 lists the attributes it will drive later and how they plug in.

**Authority:** [Propaganda and Morale](PROPAGANDA_AND_MORALE.md) owns the single `UnitResolve` meter and its break rules; [Military Formations, Tactical Stances, Cohesion and Discipline](RTS_UNIT_FORMATIONS_AND_DISCIPLINE.md) owns formations, stances and Cohesion (this document is its §6 made concrete and replaces the FORM-F5 sketch); [Engine Outline](ENGINE_OUTLINE.md) owns sequencing. The numbers below are first values for implementation and testing, not balance.

## 1. What Discipline is

- **One meter, not a second one.** Discipline is the name a *military* unit's `UnitResolve` shows in the HUD. Civilian and other non-combat units show the same meter as Morale and **never** earn formation Discipline. There is no separate `UnitDiscipline` component.
- **Every military unit has it from spawn.** A unit outside any formation is not "without discipline"; it sits at its **baseline**. Formation service raises it above the baseline. This keeps the existing canon intact: Psy-Ops and Intimidation break the meter of *any* eligible unit, and the Atlantic Concord's "no penalty for operating solo" rule needs a meter that solo units have.
- **It is earned over time, not granted on joining.** Pressing T does nothing to the value. Members gain Discipline while they actually serve together: following the formation and standing close to fellow members. Selection, control groups and the V shape never contribute.
- **It persists on the unit.** A unit that leaves a formation keeps what it earned and loses it slowly, so a disciplined squad that breaks ranks for a moment is still disciplined, and a long-dissolved one is back to its baseline.

## 2. Values

All values are authoritative integers in **milli-points** (`0…100 000` for 0–100), updated in fixed steps on the server, so the result is the same on every run of the same schedule.

| Quantity | Value | Notes |
|---|---|---|
| Range | 0–100 | `UnitResolve` maximum 100 |
| Baseline `B` | 50 | Later from faction Population Morale (§6); a constant in DISC-D1 |
| Broken threshold | 0 | Defined, not used until DISC-D4 |
| Spawn value | `B` | |
| Evaluation cadence | every 20 steps (1 s at 20 steps/s) | Cheap and deterministic; no per-frame updates |
| Formation gain | +1.0 per second while **eligible** (§3) | 50 → 80 takes 30 s of service |
| Formation ceiling | `B + 30` = 80 (normal) | Directorate/Ascendancy mass bonus raises it (§6) |
| Decay outside formation | −0.5 per second towards `B`, never below `B` | 80 → 50 takes 60 s; earned Discipline outlasts a short break |
| Decay while in formation but not eligible | none | Holding steady; an out-of-place member neither gains nor loses |

Display rounds down to whole points. The HUD shows the value and a trend arrow (▲ gaining, ▼ fading, nothing when steady).

## 3. When a member earns Discipline

At each evaluation a unit is **eligible** only if all of these hold:

1. It is a military unit, alive, and under its owner's control.
2. It is a member of a formation, and **following** it. A member made independent by an individual order (FORM-F3) is not acting as part of the formation and does not gain. Autonomous stance chases (FORM-F4) do not make a member independent, so a Guard member who chases and returns keeps earning.
3. It has **support**: at least `min(2, members − 1)` other members of the **same formation** within **400 cm** of it. At the 150 cm formation spacing every member of a closed Line or Column has two neighbours in range. A member stranded by a long route, a chase or a failed slot does not gain until it rejoins.

Rules that keep the gain honest:

- **Only the unit's own formation supports it.** Two formations standing side by side do not support each other, and a unit is counted once per evaluation however many members are near it.
- **Dead, unavailable or captured members give no support**, even if their formation slot is geometrically correct.
- **Formation state:** an Organized or Degraded formation earns. The FORM-F3 regroup re-establishes support naturally; there is no extra bonus for regrouping.
- **Adherence and stance do not change the gain** in DISC-D1. (Later doctrine may, for example, let Maintain earn faster; see §8.)
- **Free adherence still earns** when members happen to stay together, because support is measured, not assumed.

## 4. Lifecycle

| Event | Effect on Discipline |
|---|---|
| Spawn | Starts at `B` |
| Form (T) | No change; earning starts at the next evaluation if eligible |
| Release (Y), dissolve, becoming the last member | No immediate change; decay towards `B` starts |
| Member death | Its value disappears with it. Survivors' values are unchanged in DISC-D1 (casualty shock is DISC-D2) |
| Individual order on a member | Stops earning until the next formation move makes it follow again; no loss |
| Joining a different formation | Keeps its value; earns under the new formation |
| Match Terminal / Return | Values are dropped with the match; not saved |

## 5. Authority, network and presentation

- **Server-owned.** The value lives on the authoritative unit and is updated only in the fixed-step simulation, after movement and before event publication, so a step's support uses that step's positions. Clients never compute it.
- **Owner-only exact value.** The owning player receives each own unit's value and trend. Opponents and allies receive nothing in DISC-D1; a coarse visible band for enemies (for example "steady / shaken") belongs with intelligence rules later.
- **Replication** adds the value to the existing owner-only unit state; `network_protocol` 4 → 5.
- **HUD:** selected unit `DISCIPLINE 72 ▲`; multi-selection `DISCIPLINE avg 64`; the formation line adds `DISCIPLINE avg 68 ▲`; unit labels add `D72` only while the unit is selected, to avoid more label clutter.
- **Development logs:** `RTS_DISCIPLINE_SAMPLE` every 100 steps (formation, members, eligible count, min/avg/max), and `RTS_DISCIPLINE_BAND` when a unit crosses 60, 70 or 80, or returns to `B`. No per-unit log every evaluation.

## 6. Faction rules (canon) and when they arrive

These are the existing binding rules from [Propaganda and Morale](PROPAGANDA_AND_MORALE.md) and the formations document §6, expressed in this model. They need real faction identities on units and arrive in **DISC-D2**, not DISC-D1.

| Faction | Rule in this model |
|---|---|
| Red Directorate, Iron Ascendancy | Normal formation gain and ceiling **plus** a mass bonus: when a formation has **≥ 32** eligible members, its ceiling rises by +10 (to 90). The bonus counts eligible members only, not faction totals. |
| Meridian Pact, Unaligned (NPC) | Normal gain and ceiling; no mass bonus. |
| Atlantic Concord | Normal gain and ceiling. **Exempt from the isolation penalty** (DISC-D2): a Concord unit alone never drifts below `B` for being alone. |
| Civilians / non-combat | No formation Discipline at all; their meter shows as Morale with its own modifiers. |

The 32-member threshold cannot be reached on the skirmish test map (20 units per player); it is tested in Automation and belongs to the large maps (300 units per player).

## 7. What Discipline will affect later (design only, not in DISC-D1)

Every effect reads one normalized factor, `F = (D − 50) / 50`, in `[−1, +1]`: `0` at baseline, `+0.6` at the normal ceiling of 80. Each consuming system has its own coefficient and cap, is listed in one modifier registry with a source ID, and never stacks Discipline twice.

| Attribute | Candidate effect at the normal ceiling (`F = +0.6`) | Arrives with |
|---|---|---|
| Weapon accuracy / spread | spread −12% | DISC-D3 |
| Aim and reload time | −9% | DISC-D3 |
| Suppression resistance | +18% | Suppression system |
| Stance obedience | Hold Fire never breaks under fire (already true); Guard/Aggressive return to post 20% faster | DISC-D3 |
| Regroup | Regroup delay 40 → 30 steps | DISC-D3 |
| Break threshold consumers | Psy-Ops, Intimidation and rout need the meter to fall from its current value, so high Discipline buys time | DISC-D4 |

Below baseline (isolation, casualties; DISC-D2) the same coefficients apply with the negative factor.

## 8. Delivery slices

| Slice | Scope | Acceptance focus |
|---|---|---|
| **DISC-D1** | Meter on military units (spawn at `B`), formation gain / ceiling / decay (§2–4), owner-only replication, HUD, logs, `network_protocol` 5. **No effect on any other attribute.** | DISC-01 to DISC-09 |
| DISC-D2 | Stress: casualty shock to nearby members (−5, capped −15 per 5 s), isolation drift below `B` (floor 30) with the Concord exemption, faction mass bonus, faction baseline from Population Morale | DISC-10 to DISC-13 |
| DISC-D3 | First effects (§7): accuracy, aim/reload, stance return speed, regroup delay | Measured before/after combat outcomes |
| DISC-D4 | Break threshold consumers: rout, Psy-Ops, Intimidation (their own documents) | Their own acceptance |

Possible later doctrine, not decided: Maintain adherence earning faster; veteran units keeping a higher personal floor; commander talents and research raising the ceiling ([Commander Talents](COMMANDER_TALENT_SYSTEM.md), [Research](RTS_TECHNOLOGICAL_RESEARCH.md)).

## 9. Acceptance matrix (DISC-D1 and DISC-D2)

| ID | Requirement |
|---|---|
| DISC-01 | Every military unit spawns at `B`; non-combat units never earn formation Discipline. |
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
| DISC-12 | (D2) The mass bonus applies only to Directorate/Ascendancy formations with ≥ 32 eligible members. |
| DISC-13 | (D2) Faction baseline follows Population Morale within its documented bounds. |

**DISC-D1 packaged check (skirmish map, 20 units):** form 20 units into a Line and move them; sample the server log until the members reach 80 (about 30 s of service); confirm the HUD shows `DISCIPLINE 80`; make one member independent with an individual order and confirm it stops gaining; send one member beyond 400 cm and confirm it holds; release and confirm decay to 50 over about 60 s; confirm the opposing client logs nothing.

## 10. Open decisions

- Rates, ceiling and decay above are first values; tune once DISC-D3 effects make them matter.
- Whether opponents may see a coarse band of visible enemy units' Discipline (intelligence rules).
- Whether Discipline should be saved with a persistent campaign army (no campaign persistence exists yet).
- Whether vehicles and crewed weapons earn Discipline the same way as infantry, or through their crew.
