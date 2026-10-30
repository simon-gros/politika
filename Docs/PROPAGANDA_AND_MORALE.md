# POLITIKA — Propaganda, Media and Population Morale

Status: Core gameplay principles established; faction methods, numerical effects, costs and tuning remain proposed.
Related: [Concepts Outline](CONCEPTS_OUTLINE.md) · [Resource Economy](RESOURCE_ECONOMY.md) · [Story Outline](STORY_OUTLINE.md) · [Espionage and Intimidation](ESPIONAGE_AND_INTIMIDATION.md) · [Faction descriptions](../Factions/)

## 1. Core principle

Every faction has its own population morale meter, including the four playable factions and the non-playable, AI-controlled Unaligned. Higher morale increases production output and army discipline. The precise production bonus, the meaning of military discipline, and the extent of morale losses have to be balanced in prototypes rather than assumed to be implemented already.

All factions except the Meridian Pact use distinct advanced propaganda methods to pursue gameplay objectives. The four propaganda-capable factions are the Atlantic Concord, Red Directorate, Iron Ascendancy, and Unaligned. The Meridian Pact still possesses a morale meter and benefits from good morale but does not obtain advanced propaganda technologies or an equivalent specialized propaganda campaign tree.

Radio, newspapers, and television are the three principal propaganda media. They should exist as meaningful in-world infrastructure, broadcasts, print distribution, and communications networks rather than merely decorative interface buttons.

## 2. Faction differentiation

| Faction | Advanced propaganda? | Distinct direction (design proposal) | Morale? |
|---|---|---|---|
| Atlantic Concord | Yes | Broadcast credibility, public information campaigns, political messaging and coordinated radio/television/newspaper coverage; potential emphasis on influence and targeted audiences | Yes |
| Red Directorate | Yes | Centralized state communications, organized newspaper distribution, collective mobilization and coordinated radio/television campaigns; potential emphasis on sustained industrial and military morale | Yes |
| Iron Ascendancy | Yes | Highly coercive ideological propaganda, mass rallies represented abstractly, intimidating broadcasts and tightly controlled media channels; potential strong short-term mobilization offset by repression, credibility and resistance costs | Yes |
| Meridian Pact | No | Morale from normal economic and battlefield conditions, logistics, commercial success and unit welfare, not an advanced propaganda ability or technology tree | Yes |
| The Unaligned | Yes; NPC/system-controlled | Federal radio, television and print media reinforcing administrative legitimacy, public continuity, social stability and, through a restricted security inner circle, the fictional Tito cover-up | Yes |

The listed faction-specific directions are suggested expressions, not yet locked unit names, upgrade trees, numerical effects or guaranteed mission abilities. The right to advanced propaganda and the Meridian exclusion are locked.

## 3. The three principal media

- Radio: broadcast infrastructure, stations and repeaters; signals and signal disruption can affect where media influence reaches.
- Newspapers: printing houses, newsrooms, distribution points and supply routes; shortages or disrupted deliveries can restrict circulation.
- Television: broadcast studios, stations, transmission facilities and local receivers; control of coverage can create contested strategic objectives.

The effectiveness of a propaganda action should depend on the medium, faction permissions, control/access to infrastructure, reach, credibility, duration and regional circumstances. Avoid treating the existence of a transmitter as automatic, map-wide mind control. Rumors, contradictory evidence, rival broadcasts and disrupted facilities can limit effects.

Media can also convey actual intelligence and narrative events: an official broadcast about Tito, an alternative news report or a recovered recording may affect trust, morale and the main conspiracy without disclosing information to factions that have not found it.

## 4. One resolve meter, two names

Locked naming and implementation rule: every individual unit uses one and the same underlying resolve meter. Display it as Discipline when the unit is a combatant and Morale when the unit is civilian or non-combat. It is not two separately designed meters, two currencies or two independent resistance systems. Both labels share the same numeric representation, depletion/recovery events, broken threshold and Intimidation mechanism.

| Scope / unit | Name shown | Gameplay |
|---|---|---|
| Combat unit | Discipline | Performance and stability in battle, affected by formation size, casualties, Intimidation and faction modifiers |
| Civilian / non-combat unit | Morale | The same meter, affected by relevant civilian circumstances and Intimidation; does not receive combat formation perks |
| Whole faction's population | Population Morale | A faction-level morale summary/aggregate within the shared system, influencing production output and combat Discipline baseline; not a second per-unit meter |

Population Morale exists for each of the five factions, including the NPC Unaligned and playable Meridian Pact. High Population Morale improves eligible production output and the baseline Discipline of soldiers, as established. It may be aggregated from civilian morale and other public conditions under a defined faction-level policy; precise aggregation weights and propagation rules remain to be designed. Breaking one civilian unit's Morale does not zero the entire faction's Population Morale.

Morale and Discipline respond to the same underlying meter events and broken threshold, although which modifiers apply depends on unit category. Contextual influences may include combat losses and nearby support for soldiers; fuel, power, supply, employment and propaganda for civilians. Exact curves and production/Discipline multipliers remain open.

### Formation size: combat Discipline only

Normal support rule: for combat units belonging to the Red Directorate, Iron Ascendancy, Meridian Pact or Unaligned, larger coherent formations stabilize Discipline more effectively than solo or very small fighting groups; small groups have the normal Discipline stability penalty. Support means nearby cohesive forces, not a faction's total army count or the number of icons a player selects.

Extra massive-army rule: only the Red Directorate and Iron Ascendancy receive a further Discipline stability bonus in very massive, cohesive, mutually supporting combat armies, over and above the normal group effect. Meridian and Unaligned receive the ordinary group-size benefit but not the special high-mass bonus. Numerical thresholds and caps remain open.

Atlantic Concord exception: its soldiers suffer no Discipline penalty just for operating individually or in very small groups. Their Discipline may still be broken by direct battlefield effects, casualties or Intimidation; solo operation is not general immunity.

These military Discipline modifiers do not apply to civilian/non-combat Morale. Non-combat units retain the same underlying resolve meter but receive context-appropriate modifiers. Breaking formation, reinforcement and losses should update eligible military support bonuses deterministically.

### Tactical Cohesion and formation support — proposed integration

[Military Formations, Tactical Stances, Cohesion and Discipline](RTS_UNIT_FORMATIONS_AND_DISCIPLINE.md) owns spatial formation organization and **derived** Cohesion. This is not a new individual resolve gauge: combatants continue to display the one `UnitResolve` as **Discipline**, civilians/non-combatants as **Morale**. Coherent combat support must be based on actual nearby eligible operational units, not local selection, control group IDs, or an army total regardless of distance. Recompute and cap effects on death, detach, capture, fragmentation, regrouping and rematch; never stack the same supporter multiple times through nested or overlapping groups.

Retain all locked faction rules: normal coherent-group Discipline support for Red Directorate, Iron Ascendancy, Meridian Pact and NPC Unaligned; additional very-massive coherent bonus **only** Directorate/Ascendancy; Concord incurs no penalty solely for working solo/very-small; civilians receive none. Ordinary Cohesion loss or low resolve must **not automatically invoke** faction-exclusive Psy-Ops elimination or Intimidation ownership transfer: those require their own exact authorization, isolation, broken-threshold and effect checks.


### Atlantic Concord Psy-Ops and the shared resolve meter

Atlantic Concord Psy-Ops is a separate espionage subdivision, not the radio/newspaper/television propaganda campaign system. It acts on an eligible individual enemy unit, using the exact same `UnitResolve` meter as all other morale/discipline effects. It can either:

1. Break the target's Discipline (combat) or Morale (non-combat) to the shared `BrokenThreshold`, causing an in-game self-destruction/elimination event without recruitment or allegiance transfer.
2. Trigger a short temporary friendly-fire rampage, causing the affected unit to attack eligible allied units using its valid attack actions. After the limited duration, its targeting returns to normal if it survives. No ownership change occurs. Whether the rampage must use a separate resolve trigger is left for balancing.

The destruction and temporary-rampage effects are distinct from Iron Ascendancy Intimidation, which converts eligible isolated targets only after fully breaking Discipline/Morale. Psy-Ops does not add a second morale gauge, collapse an entire faction's Population Morale merely by breaking one unit, or confer Intimidation on the Concord. An unarmed civilian/non-combat unit must have a valid authored in-game attack action before it can perform a rampage. See [Espionage and Intimidation](ESPIONAGE_AND_INTIMIDATION.md).

### Facility Psy-Ops and production disruption do not create another resolve meter

Atlantic Concord Psy-Ops may also destroy eligible enemy buildings or induce covert finite resource waste; Directorate espionage can divert rival resources to its own budget; Iron Ascendancy may apply an abstract poison/disease facility status that reduces staffed workforce/research personnel, may trigger limited nearby facility chains, and lowers or stops production. These are building, resource and workforce mechanics, not separate instances of individual-unit Discipline/Morale and not direct edits to the defender's entire Population Morale meter.

A unit's `UnitResolve` remains a single numeric meter displayed as combat Discipline or non-combat Morale; facility worker-loss counts are related economy state, not another Morale bar. Story events and publicly discovered sabotage may influence faction Population Morale through explicitly defined, limited modifier rules, but there is no automatic zeroing of a whole population from one compromised building. See [Espionage and Intimidation](ESPIONAGE_AND_INTIMIDATION.md).

### Iron Ascendancy Intimidation against both categories

Intimidation is exclusive to the Iron Ascendancy and can target eligible isolated combat or non-combat units. Isolation is necessary, not sufficient. The Ascendancy must reduce the targeted unit's single shared meter to its fully broken threshold before converting that unit:

- Combat: break the target's Discipline, then convert if still isolated and eligible.
- Civilian/non-combat: break the target's Morale, then convert if still isolated and eligible.

No instant conversion occurs simply because a target has no nearby friendly support. A worker with remaining Morale and a soldier with remaining Discipline are equally resistant to immediate allegiance transfer under the shared-meter rule. Support and eligibility restrictions remain relevant. Ordinary civilians and neutral Unaligned personnel are not automatically eligible without explicit design rules.

Intimidation's meter-reduction event and ownership transfer are separate authoritative transitions. Population Morale cannot be confused with an individual's meter; Intimidation against one civilian does not convert an entire population. See [Espionage and Intimidation](ESPIONAGE_AND_INTIMIDATION.md).

### Unaligned false intelligence and its NPC propaganda channels

The Unaligned has the most powerful counterintelligence apparatus in the game. Beyond normal interception and security, it can manufacture misleading intelligence and false-flag reports, sometimes reinforced by its NPC-run radio, television and newspaper propaganda. Fabricated intelligence and public propaganda are not identical: a secret report may deceive a specific playable faction's military command without becoming a broadcast or changing every citizen's Morale.

When a playable faction is near defeat, with low resources and a very small remaining army, the Unaligned may plant reports about rival nuclear facilities, deployments or hostile intentions to redirect the stronger powers' attention and preserve overall faction balance. Such operations are governed by AI stability priorities and bounded credibility, exposure and verification checks. A fabricated report does not manipulate `UnitResolve`, instantaneously change Population Morale or directly alter real unit/building state; discovered deception may later affect faction trust and public Morale through explicit normal gameplay events.

The false-intelligence system also supports the fictional Tito continuity propaganda, but its ordinary use does not imply that all federal officials or media employees are members of the cover-up. See [Unaligned Counterintelligence](UNALIGNED_COUNTERINTELLIGENCE.md).

## 5. Narrative and AI integration

In the central storyline, the Unaligned's media institutions support claims that Tito remains publicly active after his fictional 1984 assassination, using radio voices, televised body-double appearances and official press coverage. The restricted cover-up leadership may authorize disinformation, but ordinary Unaligned media workers and citizens do not automatically know the secret.

As evidence accumulates through 1989, increasingly contradictory broadcasts and reports can influence confidence and morale. The canonical public revelation and disintegration remain fixed; morale changes should express their consequences rather than become a mechanic that cancels the story.

Concord, Directorate and Ascendancy campaigns may use distinct propaganda missions alongside espionage, territorial control and conventional conflict. The Meridian Pact may interact with media as ordinary world infrastructure or trade, and may experience propaganda effects, but it must not acquire the advanced propaganda actions reserved for the other four.

## 6. Suggested technical contracts

[Military Discipline](RTS_UNIT_DISCIPLINE.md) (30 October 2026; DISC-D1 and DISC-D2 implemented, records [D1](RTS_UNIT_DISCIPLINE_D1_ACCEPTANCE.md) and [D2](RTS_UNIT_DISCIPLINE_D2_ACCEPTANCE.md)) makes the military side of `UnitResolve` concrete: baseline 50, earned in coherent formations up to 80 (100 for Directorate/Ascendancy formations of at least 32), fading back to the baseline outside them, a lone soldier drifting to 30 unless it is Concord, casualty shock to nearby formation members, with `FormationDisciplineSupport`, `MassArmyDisciplineBonus` and `ConcordNoSoloDisciplinePenalty` below mapped to its DISC-D1/D2 slices.

Use one `UnitResolve` component/property for every combat and non-combat unit, with `UnitCategory` selecting the HUD label `Discipline` or `Morale`. The meter uses unified value, limits, loss, recovery and break rules. Do not store separate `BattlefieldMorale`, `UnitDiscipline` and `NonCombatUnitMorale` components for the same mechanic.

- `UnitResolve`: unit identifier, value, shared maximum, broken threshold, changes and recovery.
- `UnitCategory`: combat or non-combat; maps combat to the Discipline label and non-combat to the Morale label.
- `FactionPopulationMorale`: a faction-wide rollup/summary in the same framework, used for production and military baseline Discipline; its aggregation is not a duplicate per-unit meter.
- `FormationDisciplineSupport`: qualified nearby allied combat units and capped standard discipline stability.
- `MassArmyDisciplineBonus`: extra threshold and bonus restricted to Directorate/Ascendancy combat formations.
- `ConcordNoSoloDisciplinePenalty`: removes the normal small-combat-formation penalty for Concord soldiers only.
- `IntimidationResolveBreak`: target eligibility, isolation, the shared meter reaching the break threshold and later authorized ownership transfer.
- `PsyOpsResolveBreak`: Concord-only target-specific meter reduction through the shared `UnitResolve`, followed by self-destruction on full break; `PsyOpsTemporaryRampage`: eligible unit, valid own-faction attack targets, fixed short duration and automatic restoration of targeting with ownership unchanged.
- `PropagandaCapability`: on for Concord, Directorate, Ascendancy, NPC Unaligned; off for Meridian. `MediaFacility` and `PropagandaAction` describe radio, newspapers and TV with explicit costs and reach.

Acceptance checks: all combat units display Discipline and all civilian/non-combat units display Morale; the backing meter behaves identically; every faction has Population Morale; higher Population Morale improves production and baseline Discipline; support and massive-army modifiers apply only to soldiers; Concord is exempt from the small-squad penalty; Concord Psy-Ops break-to-destruction requires full Discipline/Morale collapse, and its alternative friendly-fire rampage is temporary with no conversion; Intimidation cannot convert either kind of isolated unit until its own shared meter is broken; a single civilian's broken Morale does not erase faction-wide Population Morale; no propaganda can bypass nuclear prerequisites.

## 7. Balancing questions

Set shared `UnitResolve` ranges, recovery and break thresholds; population-morale aggregation and its feedback into production/Discipline; formation-support radii and caps; exceptional massive-army thresholds for Directorate/Ascendancy; Concord solo exemption; civilian-specific modifier rules; Intimidation timing, eligibility and counterplay; and faction propaganda reach and cost. Numerical values and map-specific eligibility remain open.

Locked summary: one meter per unit—called Discipline for combat units and Morale for civilian/non-combat units. Population Morale is the faction-level view of the same system. Larger combat formations stabilize Discipline for all except Concord's solo penalty exemption; Directorate and Ascendancy gain an additional massive-army bonus. Intimidation must break Discipline on soldiers or Morale on civilians before converting an eligible isolated unit.
