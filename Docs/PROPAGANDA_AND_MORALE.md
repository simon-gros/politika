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
2. **Army discipline:** higher morale improves the reliability with which units carry out orders, maintain formations, resist panic or disorder, and sustain operational cohesion. Exact combat attributes, desertion behavior, command-delay effects and caps are to be defined.

Morale can respond to **military performance, casualties, supply, power and fuel availability, economic conditions, public services, occupation, propaganda, credible information and revelations**. The response should be legible and bounded; no faction should be locked into an irreversible death spiral solely because of a single information event.

Faction-specific modifiers can determine whether morale is more sensitive to resource shortages, battlefield losses, repression, public confidence, or communications continuity. These are candidate design parameters, **not universal historical claims**.

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
- `ProductionMoraleModifier` and `DisciplineMoraleModifier`: formula and allowed targets defined by match rules, evaluated from authoritative morale rather than UI estimates.

**Core acceptance checks:** every faction, including Meridian and Unaligned, has morale; only four factions can issue advanced propaganda actions; radio/newspaper/television facilities have defined roles; morale improves eligible production and discipline at higher values; broken infrastructure affects media reach; propaganda cannot grant nuclear intelligence or bypass material/refinement/build prerequisites; and the Unaligned remains NPC-only.

## 7. Balancing questions

Set the morale meter's scale, baseline and update cadence; propaganda cost, audience, reach and credibility; duration and stacking rules; morale multipliers for production and discipline; counters to broadcast influence; and whether local/regional sub-morale feeds each faction's aggregate meter. **No precise percentages or unit values are canonical yet.**

**Locked summary:** **all five factions have population morale; high morale raises production and army discipline; all except the Meridian Pact have different advanced propaganda capabilities; and radio, newspapers and television are the main propaganda media.**
