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

## 4. Population morale and gameplay effects

**Every faction has a separately tracked population morale value**. The Unaligned's morale is simulated and AI-managed rather than player-controlled; the Meridian Pact's morale is affected by events and conditions despite its lack of advanced propaganda actions.

Two positive effects are **core requirements**:

1. **Production output:** higher morale improves eligible industrial/factory output, recruitment throughput or other data-defined production rates. Do not assume that the bonus reduces the amount of nuclear material required, bypasses refinery inputs, grants technology, or accelerates the Iron Ascendancy's instant-win path without explicit balancing.
2. **Army discipline:** higher population morale improves the starting conditions and recovery of battlefield unit morale. **Effective unit morale/discipline is also affected by formation size and allied support** for every faction except the Atlantic Concord. Higher effective discipline improves reliability in executing orders, maintaining formations and resisting panic or disorder. Exact attributes, thresholds and caps remain to be balanced.

Morale can respond to **military performance, casualties, supply, power and fuel availability, economic conditions, public services, occupation, propaganda, credible information and revelations**. The response should be legible and bounded; no faction should be locked into an irreversible death spiral solely because of a single information event.

Faction-specific modifiers can determine whether morale is more sensitive to resource shortages, battlefield losses, repression, public confidence, or communications continuity. These are candidate design parameters, **not universal historical claims**.

### Formation-size morale and battlefield discipline

**Locked baseline rule:** for **Red Directorate, Iron Ascendancy, Meridian Pact and Unaligned**, larger coherent army formations possess **more stable battlefield morale and discipline** than individual soldiers or very small detachments. Their effective morale resists deterioration more effectively when fighting in mutually supporting groups. Conversely, lone soldiers and very small groups suffer the normal **formation-size morale/discipline disadvantage**.

**Additional massive-army advantage (Red Directorate and Iron Ascendancy only):** when either of these factions organizes a **very massive, coherent and mutually supporting army**, it receives a **further substantial morale/discipline stability bonus above the ordinary large-formation benefit**. This is a distinct positive faction trait, not merely the absence of the small-unit penalty. The Directorate's inexpensive infantry reinforces its mass-army strategy; the Ascendancy can sustain unusually aggressive mass attacks while remaining vulnerable to losses and disruption. **Meridian Pact and Unaligned receive only the ordinary formation-size benefit, not the special massive-army bonus.**

**The Atlantic Concord is the sole exception**: its units **do not lose morale or discipline simply because they are deployed individually or in very small groups**. They retain their normal disciplined baseline without allied formation-size support; they can still suffer morale loss from casualties, supply deprivation, enemy effects or other eligible causes. The Concord's exemption is **not blanket immunity** to morale damage or the Iron Ascendancy's Intimidation.

**Keep two related but separate values:** (a) **population morale**, tracked per faction and relevant to production and army-wide baseline discipline; and (b) **battlefield unit/formation morale**, tracked at a combat-entity or coherent formation level and affected by support, group size and local combat. A larger army's discipline bonus is not an automatic increase to civilian/population morale; likewise, high population morale does not by itself eliminate the consequences of battlefield isolation.

**Size means genuinely supported formation strength**, not the total number of units a faction owns or the number of separate, distant squads selected in the UI. Eligible nearby allied personnel, cohesion, command radius and capped support calculations should determine the baseline group-size effect. A **separate, very-massive-army threshold and additional stability modifier apply only to Red Directorate and Iron Ascendancy**. Exact thresholds and bonus magnitudes are open for balancing. Splitting or heavily depleting a massive army removes its extra bonus when it no longer qualifies; regrouping may restore it. Avoid unlimited buffs from stacking units in one location.

### Interaction with Iron Ascendancy Intimidation

The **Iron Ascendancy alone** can intimidate an **eligible isolated enemy unit**. **Isolation is necessary but not sufficient for conversion:** Intimidation must first **reduce the target's battlefield morale to its defined broken/zero threshold**. **Only after morale is fully broken can the unit change allegiance to the Iron Ascendancy.** A target whose morale is still above that threshold stays with its current faction, even if isolated and under active intimidation.

Normal formation-size fragility may make lone non-Concord troops easier to demoralize. **Atlantic Concord troops do not receive that automatic small-group morale penalty**, so they retain discipline while isolated; however, direct intimidation pressure and other applicable battlefield morale effects can still break their morale. Nearby allies can prevent isolation and thus stop conversion regardless of morale. Reinforcement, withdrawal, countermeasures and morale recovery give defenders opportunities to interrupt the process.

Intimidation's morale damage and the conversion outcome are **two separate authoritative transitions**. Damaging health, suppressing a unit or applying an intimidation visual effect must not instantly switch its owner. The threshold scale, action duration, recovery rules and eligible targets remain balance decisions; see [Espionage and Intimidation](ESPIONAGE_AND_INTIMIDATION.md).

## 5. Narrative and AI integration

In the central storyline, the Unaligned's **media institutions** support claims that Tito remains publicly active after his fictional 1984 assassination, using radio voices, televised body-double appearances and official press coverage. The restricted cover-up leadership may authorize disinformation, but **ordinary Unaligned media workers and citizens do not automatically know the secret**.

As evidence accumulates through 1989, increasingly contradictory broadcasts and reports can influence confidence and morale. The canonical public revelation and disintegration remain fixed; morale changes should express their consequences rather than become a mechanic that cancels the story.

Concord, Directorate and Ascendancy campaigns may use distinct propaganda missions alongside espionage, territorial control and conventional conflict. The Meridian Pact may **interact with media as ordinary world infrastructure or trade**, and may experience propaganda effects, but it must **not** acquire the advanced propaganda actions reserved for the other four.

## 6. Suggested technical contracts

Track morale and media influence with authoritative, data-driven state:

- `FactionMorale`: faction ID, bounded value, current modifiers, expiry/decay and history.
- `PropagandaCapability`: enabled for Concord, Directorate, Ascendancy and Unaligned; **disabled for Meridian**.
- `MediaFacility`: medium type (radio/newspaper/television), jurisdiction, owner/controller, operational status, power/supply needs and reachable audience.
- `PropagandaAction`: faction eligibility, selected medium, valid facility, game cost, reach, duration, competing influence and observable completion.
- `ProductionMoraleModifier` and `DisciplineMoraleModifier`: formulas and allowed targets defined by match rules, evaluated from authoritative morale rather than UI estimates.
- `BattlefieldMorale`: entity or coherent formation identifier, current morale, morale-loss/recovery events, broken threshold and effective discipline state.
- `FormationSupport`: qualified nearby friendly units, coherent group size/support calculation, capped baseline stability modifier and faction exception flag (`ConcordNoSmallGroupPenalty`).
- `MassArmyDiscipline`: a separate massive-formation eligibility threshold and capped morale/discipline modifier **restricted to `RedDirectorate` and `IronAscendancy`**, which is lost when the formation no longer qualifies.
- `IntimidationMoraleBreak`: eligible isolated target, active pressure, authoritative morale reaching its broken threshold, then a separately validated allegiance-transfer event.

**Core acceptance checks:** all five factions have population morale; only four can issue advanced propaganda actions; higher population morale improves production and army-wide baseline discipline; larger supported formations retain baseline battlefield discipline better for Directorate, Ascendancy, Meridian and Unaligned; **only Directorate and Ascendancy earn the extra very-massive-army morale/discipline bonus**; Concord solo/small-group units suffer **no size-based morale penalty**; Concord can still be demoralized by other causes; an isolated enemy with non-broken morale **cannot** be converted by Intimidation; an isolated eligible unit becomes convertible only **after its battlefield morale is broken**; propaganda cannot bypass nuclear prerequisites; and Unaligned stays NPC-only.

## 7. Balancing questions

Set the population and battlefield morale scales, baseline and update cadence; propaganda cost, audience, reach and credibility; duration and stacking; production modifiers; formation-support radius/size and capped ordinary morale stability; **Directorate/Ascendancy massive-army eligibility, threshold, cap and bonus magnitude**; battlefield morale loss/recovery and break threshold; Intimidation pressure timing and counterplay; and whether regional sub-morale feeds each faction's aggregate meter. **No precise percentages or unit values are canonical yet.**

**Locked summary:** all five factions have population morale; high morale raises production and baseline army discipline; **larger supported formations normally stabilize battlefield morale; Red Directorate and Iron Ascendancy get an additional bonus specifically for very massive armies, while Atlantic Concord soldiers avoid all solo/small-group morale penalties**; **the Iron Ascendancy must first break an isolated target's battlefield morale before Intimidation can convert it**; all except Meridian have advanced propaganda through radio, newspapers and television.
