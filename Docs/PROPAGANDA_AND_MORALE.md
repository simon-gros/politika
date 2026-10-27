# POLITIKA — Propaganda, Media and Population Morale

**Status:** Core gameplay principles established; faction methods, numerical effects, costs and tuning remain proposed.
**Related:** [Concepts Outline](CONCEPTS_OUTLINE.md) · [Resource Economy](RESOURCE_ECONOMY.md) · [Story Outline](STORY_OUTLINE.md) · [Espionage and Intimidation](ESPIONAGE_AND_INTIMIDATION.md) · [Faction descriptions](../Factions/)

## 1. Core principle

**Every faction has its own population morale meter**, including the four playable factions and the non-playable, AI-controlled Unaligned. **Higher morale increases production output and army discipline.** The precise production bonus, the meaning of military discipline, and the extent of morale losses have to be balanced in prototypes rather than assumed to be implemented already.

**All factions except the Meridian Pact use distinct advanced propaganda methods to pursue gameplay objectives.** The four propaganda-capable factions are the **Atlantic Concord, Red Directorate, Iron Ascendancy, and Unaligned**. The Meridian Pact still possesses a morale meter and benefits from good morale but **does not obtain advanced propaganda technologies or an equivalent specialized propaganda campaign tree**.

**Radio, newspapers, and television** are the three principal propaganda media. They should exist as meaningful in-world infrastructure, broadcasts, print distribution, and communications networks rather than merely decorative interface buttons.

## 2. Faction differentiation

| Faction | Advanced propaganda? | Distinct direction (design proposal) | Morale? |
|---|---|---|---|
| **Atlantic Concord** | **Yes** | Broadcast credibility, public information campaigns, political messaging and coordinated radio/television/newspaper coverage; potential emphasis on influence and targeted audiences | **Yes** |
| **Red Directorate** | **Yes** | Centralized state communications, organized newspaper distribution, collective mobilization and coordinated radio/television campaigns; potential emphasis on sustained industrial and military morale | **Yes** |
| **Iron Ascendancy** | **Yes** | Highly coercive ideological propaganda, mass rallies represented abstractly, intimidating broadcasts and tightly controlled media channels; potential strong short-term mobilization offset by repression, credibility and resistance costs | **Yes** |
| **Meridian Pact** | **No** | Morale from normal economic and battlefield conditions, logistics, commercial success and unit welfare, **not** an advanced propaganda ability or technology tree | **Yes** |
| **The Unaligned** | **Yes; NPC/system-controlled** | Federal radio, television and print media reinforcing administrative legitimacy, public continuity, social stability and, through a restricted security inner circle, the fictional Tito cover-up | **Yes** |

The listed faction-specific directions are **suggested expressions**, not yet locked unit names, upgrade trees, numerical effects or guaranteed mission abilities. The **right to advanced propaganda and the Meridian exclusion are locked**.

## 3. The three principal media

- **Radio:** broadcast infrastructure, stations and repeaters; signals and signal disruption can affect where media influence reaches.
- **Newspapers:** printing houses, newsrooms, distribution points and supply routes; shortages or disrupted deliveries can restrict circulation.
- **Television:** broadcast studios, stations, transmission facilities and local receivers; control of coverage can create contested strategic objectives.

The effectiveness of a propaganda action should depend on the **medium, faction permissions, control/access to infrastructure, reach, credibility, duration and regional circumstances**. Avoid treating the existence of a transmitter as automatic, map-wide mind control. Rumors, contradictory evidence, rival broadcasts and disrupted facilities can limit effects.

Media can also convey **actual intelligence and narrative events**: an official broadcast about Tito, an alternative news report or a recovered recording may affect trust, morale and the main conspiracy without disclosing information to factions that have not found it.

## 4. One resolve meter, two names

**Locked naming and implementation rule:** every individual unit uses **one and the same underlying resolve meter**. Display it as **Discipline** when the unit is a **combatant** and **Morale** when the unit is **civilian or non-combat**. It is **not** two separately designed meters, two currencies or two independent resistance systems. Both labels share the same numeric representation, depletion/recovery events, broken threshold and Intimidation mechanism.

| Scope / unit | Name shown | Gameplay |
|---|---|---|
| **Combat unit** | **Discipline** | Performance and stability in battle, affected by formation size, casualties, Intimidation and faction modifiers |
| **Civilian / non-combat unit** | **Morale** | The same meter, affected by relevant civilian circumstances and Intimidation; does not receive combat formation perks |
| **Whole faction's population** | **Population Morale** | A faction-level morale summary/aggregate within the shared system, influencing production output and combat Discipline baseline; **not a second per-unit meter** |

Population Morale exists for each of the five factions, including the NPC Unaligned and playable Meridian Pact. **High Population Morale improves eligible production output and the baseline Discipline of soldiers**, as established. It may be aggregated from civilian morale and other public conditions under a defined faction-level policy; precise aggregation weights and propagation rules remain to be designed. **Breaking one civilian unit's Morale does not zero the entire faction's Population Morale**.

Morale and Discipline respond to the same underlying meter events and broken threshold, although **which modifiers apply depends on unit category**. Contextual influences may include combat losses and nearby support for soldiers; fuel, power, supply, employment and propaganda for civilians. Exact curves and production/Discipline multipliers remain open.

### Formation size: combat Discipline only

**Normal support rule:** for combat units belonging to the **Red Directorate, Iron Ascendancy, Meridian Pact or Unaligned**, larger coherent formations stabilize **Discipline** more effectively than solo or very small fighting groups; small groups have the normal Discipline stability penalty. Support means nearby cohesive forces, not a faction's total army count or the number of icons a player selects.

**Extra massive-army rule:** **only the Red Directorate and Iron Ascendancy** receive a **further Discipline stability bonus** in very massive, cohesive, mutually supporting combat armies, over and above the normal group effect. Meridian and Unaligned receive the ordinary group-size benefit but **not** the special high-mass bonus. Numerical thresholds and caps remain open.

**Atlantic Concord exception:** its soldiers suffer **no Discipline penalty just for operating individually or in very small groups**. Their Discipline may still be broken by direct battlefield effects, casualties or Intimidation; solo operation is not general immunity.

These **military Discipline modifiers do not apply to civilian/non-combat Morale**. Non-combat units retain the same underlying resolve meter but receive context-appropriate modifiers. Breaking formation, reinforcement and losses should update eligible military support bonuses deterministically.

### Iron Ascendancy Intimidation against both categories

**Intimidation is exclusive to the Iron Ascendancy** and can target **eligible isolated combat or non-combat units**. Isolation is necessary, not sufficient. The Ascendancy must reduce the targeted unit's **single shared meter to its fully broken threshold** before converting that unit:

- **Combat:** break the target's **Discipline**, then convert if still isolated and eligible.
- **Civilian/non-combat:** break the target's **Morale**, then convert if still isolated and eligible.

**No instant conversion** occurs simply because a target has no nearby friendly support. A worker with remaining Morale and a soldier with remaining Discipline are equally resistant to immediate allegiance transfer under the shared-meter rule. Support and eligibility restrictions remain relevant. Ordinary civilians and neutral Unaligned personnel are not automatically eligible without explicit design rules.

Intimidation's meter-reduction event and ownership transfer are **separate authoritative transitions**. Population Morale cannot be confused with an individual's meter; Intimidation against one civilian does not convert an entire population. See [Espionage and Intimidation](ESPIONAGE_AND_INTIMIDATION.md).

## 5. Narrative and AI integration

In the central storyline, the Unaligned's **media institutions** support claims that Tito remains publicly active after his fictional 1984 assassination, using radio voices, televised body-double appearances and official press coverage. The restricted cover-up leadership may authorize disinformation, but **ordinary Unaligned media workers and citizens do not automatically know the secret**.

As evidence accumulates through 1989, increasingly contradictory broadcasts and reports can influence confidence and morale. The canonical public revelation and disintegration remain fixed; morale changes should express their consequences rather than become a mechanic that cancels the story.

Concord, Directorate and Ascendancy campaigns may use distinct propaganda missions alongside espionage, territorial control and conventional conflict. The Meridian Pact may **interact with media as ordinary world infrastructure or trade**, and may experience propaganda effects, but it must **not** acquire the advanced propaganda actions reserved for the other four.

## 6. Suggested technical contracts

Use one **`UnitResolve`** component/property for **every combat and non-combat unit**, with `UnitCategory` selecting the HUD label `Discipline` or `Morale`. The meter uses unified value, limits, loss, recovery and break rules. **Do not store separate `BattlefieldMorale`, `UnitDiscipline` and `NonCombatUnitMorale` components** for the same mechanic.

- `UnitResolve`: unit identifier, value, shared maximum, broken threshold, changes and recovery.
- `UnitCategory`: combat or non-combat; maps combat to the **Discipline** label and non-combat to the **Morale** label.
- `FactionPopulationMorale`: a faction-wide rollup/summary in the same framework, used for production and military baseline Discipline; its aggregation is not a duplicate per-unit meter.
- `FormationDisciplineSupport`: qualified nearby allied **combat** units and capped standard discipline stability.
- `MassArmyDisciplineBonus`: extra threshold and bonus restricted to **Directorate/Ascendancy combat formations**.
- `ConcordNoSoloDisciplinePenalty`: removes the normal small-combat-formation penalty for Concord soldiers only.
- `IntimidationResolveBreak`: target eligibility, isolation, the shared meter reaching the break threshold and later authorized ownership transfer.
- `PropagandaCapability`: on for Concord, Directorate, Ascendancy, NPC Unaligned; off for Meridian. `MediaFacility` and `PropagandaAction` describe radio, newspapers and TV with explicit costs and reach.

**Acceptance checks:** all combat units display Discipline and all civilian/non-combat units display Morale; the backing meter behaves identically; every faction has Population Morale; higher Population Morale improves production and baseline Discipline; support and massive-army modifiers apply only to soldiers; Concord is exempt from the small-squad penalty; Intimidation cannot convert either kind of isolated unit until **its own shared meter is broken**; a single civilian's broken Morale does not erase faction-wide Population Morale; no propaganda can bypass nuclear prerequisites.

## 7. Balancing questions

Set shared `UnitResolve` ranges, recovery and break thresholds; population-morale aggregation and its feedback into production/Discipline; formation-support radii and caps; exceptional massive-army thresholds for Directorate/Ascendancy; Concord solo exemption; civilian-specific modifier rules; Intimidation timing, eligibility and counterplay; and faction propaganda reach and cost. Numerical values and map-specific eligibility remain open.

**Locked summary:** **one meter per unit**—called **Discipline** for combat units and **Morale** for civilian/non-combat units. **Population Morale** is the faction-level view of the same system. Larger combat formations stabilize Discipline for all except Concord's solo penalty exemption; Directorate and Ascendancy gain an additional massive-army bonus. Intimidation must break **Discipline** on soldiers or **Morale** on civilians before converting an eligible isolated unit.
