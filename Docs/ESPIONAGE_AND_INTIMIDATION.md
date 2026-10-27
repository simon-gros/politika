# POLITIKA — Espionage, Counterintelligence and Intimidation

**Status:** Core faction asymmetry and the Iron Ascendancy nuclear-infiltration gate are canon; detailed abilities, progression costs and balancing values remain in design.  
**Related:** [Concepts Outline](CONCEPTS_OUTLINE.md) · [Nuclear Ultimate Weapons System](NUCLEAR_WEAPONS_SYSTEM.md) · [Story Outline](STORY_OUTLINE.md) · [Resource Economy](RESOURCE_ECONOMY.md) · [Faction Descriptions](../Factions/)

## 1. Faction intelligence identities

Espionage is **asymmetric, not a universal identical spy-unit tree**. The three principal military factions play differently.

| Faction | Established espionage identity | Exclusive or special rules |
|---|---|---|
| **Atlantic Concord** | **Own special intelligence and espionage features**, separate from the Directorate's abilities; good information access fits its broader reconnaissance and precision doctrine | Exact special abilities and progression are **not yet specified** and should not be treated as implemented or locked |
| **Red Directorate** | **Own special intelligence and espionage features**, different from those of the Concord; fits its security, counterintelligence and organization emphasis | Exact special abilities and progression are **not yet specified** and should not be treated as implemented or locked |
| **Iron Ascendancy** | **More limited espionage capabilities** than the Concord and Directorate; high-value infiltration requires major investment and full development of its own spying progression | **Only faction with Intimidation**, which can force **isolated eligible enemy units** to join its forces; access to nuclear-research infiltration is **locked until the Ascendancy's spy capabilities are fully developed** |
| **Meridian Pact** | Commercial, mercenary and informant networks remain distinct from the main factions' specialist espionage programs | **No Intimidation** and no nuclear-refinement or nuclear-bomb production unlock |
| **The Unaligned** | Neutral NPC security, policing and counterintelligence; most personnel do not know Tito's assassination secret | **Not playable** and cannot use the Ascendancy's exclusive Intimidation power |

The Concord and Directorate **must not share exactly the same special espionage mechanics**. Their precise faction-specific upgrades, abilities and countermeasures are design tasks, not factual claims about already implemented systems. Possible future directions include Concord signal-intercept/reconnaissance advantages and Directorate human-source/counterintelligence advantages; these are illustrative proposals only.

## 2. Iron Ascendancy's limited espionage and full-development gate

The Iron Ascendancy may perform ordinary limited covert actions through its basic spy capabilities, but **it cannot infiltrate a rival nuclear research facility at a low or intermediate espionage level**. The espionage progression must be **fully developed / at its maximum eligible tier** before nuclear infiltration becomes an allowed mission interaction.

**Locked progression for its first independently produced atomic bomb:**

1. **Develop the Iron Ascendancy espionage technology/progression tree to full completion.** Intermediate spy upgrades do **not** satisfy the gate.
2. Deploy an eligible Ascendancy spy and **successfully infiltrate an Atlantic Concord or Red Directorate nuclear research facility**. No other faction's facility satisfies this special condition.
3. Acquire and validate **both nuclear-material refinement technology intelligence and nuclear-weapons production intelligence**. These are separate required game-state flags; scenario authors can put both in one qualifying research facility.
4. Acquire the **highest-value crude nuclear-material game resources** and refine them after the stolen-technology unlock. The Ascendancy's processed nuclear materials can be used for **bomb production only**; it **cannot build nuclear power plants**.
5. Complete the expensive, very long production process for the Ascendancy's **first own-produced atomic bomb**.
6. **Immediately win the active game on completion. No deployment, bombing mission or detonation is required.**

The game must validate **both** the maxed spy progression and the correct research-facility infiltration event. Infiltration without full espionage development is not valid; maxing spying alone does not grant nuclear research. Merely scouting a site, stealing an already completed bomb, or persuading an enemy unit through Intimidation does not fulfill this nuclear-intelligence requirement. **Intimidation cannot replace the spy upgrade or research-theft gates.**

## 3. Iron Ascendancy exclusive ability: Intimidation

**Intimidation is exclusive to the Iron Ascendancy.** No Atlantic Concord, Red Directorate, Meridian Pact or Unaligned unit gains this ability through native production, copying a technology tree or a Black Market purchase.

Its purpose is to **break the battlefield morale of an isolated enemy unit and only then force it to change sides and join the Ascendancy**. An eligible target must first lose its remaining morale under an intimidation attempt; until its morale reaches the defined **fully broken threshold**, it stays loyal to its original faction. After that threshold is reached and all eligibility checks pass, the surviving unit changes controllable allegiance/ownership instead of being destroyed or replaced with a new spawned unit.

**Core target rule:** an eligible enemy combat unit must be **isolated from supporting friendly units**, and its **battlefield morale must be fully broken before conversion**. A supported squad or a unit with nearby allies does not qualify as isolated regardless of morale, and an isolated unit with intact morale is not yet convertible. Neutral civilians or ordinary non-hostile Unaligned personnel should not be freely convertible. Isolation can enable applying Intimidation morale pressure, but **is never an instant-conversion trigger**.

### Formation size and the Concord exception

Except for the **Atlantic Concord**, larger coherent army formations have **more stable morale/discipline** than individuals or very small detachments. For the **Red Directorate, Iron Ascendancy, Meridian Pact and Unaligned**, separation from allies and small formation size make battlefield morale more vulnerable. **The Red Directorate and Iron Ascendancy alone also gain an extra morale/discipline stability bonus when operating very massive, coherently supported armies**—a distinct positive advantage above normal large-group stability. The Meridian Pact and Unaligned do not have this special massive-army bonus. **Concord is the sole faction without a morale/discipline penalty for deploying individuals or very small groups**; its troops retain their normal battlefield discipline under those conditions.

**The Concord exception is not immunity to Intimidation.** A lone Concord unit can still lose morale under direct intimidation pressure and other legitimate adverse battlefield effects. However, it is **not automatically softened by the small-group penalty**. To convert it, an Ascendancy intimidator must still fully break its morale while it remains an eligible isolated enemy target. See [Propaganda and Morale](PROPAGANDA_AND_MORALE.md).

### Intimidation progression and counterplay

**Required sequence:** `eligible enemy + isolation → active Ascendancy intimidation → target battlefield morale reduced to fully broken/zero threshold → validated conversion into Ascendancy allegiance`. No partial morale depletion is sufficient for conversion. If isolation or eligibility ends before the threshold is met, conversion cannot occur; whether pressure immediately stops and how morale recovers are tuning decisions.

**Potential balancing/counterplay, pending implementation details:**
- Clearly expose an eligibility indicator, isolation condition, **remaining unit morale**, and whether the **broken-morale threshold** has been reached.
- Limit the action by unit archetype, range, time, cooldown, risk or other game costs; values are not yet locked.
- Permit opposing players to respond by reinforcing or regrouping an isolated unit, detecting an intimidator, protecting morale, or interrupting the **morale-breaking phase** before conversion.
- Define how leadership, specialist and vehicle units interact with the mechanic, rather than allowing unrestricted conversions by default.
- Track the target's original faction and the Ascendancy as the new controller, with effects on diplomatic reputation, supply and AI recognition.
- Keep the intimidation action separate from the general damaging attacks that sometimes prioritize destruction at the expense of the Ascendancy unit's survival.

**Authoritative rules invariant:** Intimidation conversion may resolve only when `ActorFaction == IronAscendancy`, actor has Intimidation, `TargetIsEnemyCombatUnit`, `TargetIsIsolated`, and **`TargetBattlefieldMorale <= BrokenMoraleThreshold`** following an authorized morale-reducing Intimidation action. Battlefield morale must be updated and validated **before** ownership changes. **Morale damage and conversion are separate game-state events**; UI effects alone never change allegiance.

## 4. Story and AI integration

The Iron Ascendancy's **limited espionage** creates a distinctive difficulty curve: it is comparatively less capable of early information operations than the two intelligence-specialist nuclear powers, but it has **Intimidation** as a coercive tactical alternative. Its first-bomb instant victory becomes a late-game strategic goal precisely because it must fully develop spying, infiltrate protected Concord/Directorate research, steal both intelligence types, refine the rarest materials, and finish lengthy production.

The Concord and Directorate can pursue their own different spy-focused objectives and deploy defensive counterintelligence around nuclear research sites, power infrastructure and valuable archives. The Unaligned reacts to coercion, attacks and restricted-site violations through its neutral sovereignty and security rules.

The primary Tito-assassination cover-up remains the core 1984–1989 storyline. Espionage asymmetry and intimidation are gameplay and mission-design systems; they do not change the canonical 1989 revelation.

### Population morale and propaganda are distinct

All factions have **population morale**, improving **production output and baseline army discipline** when high. Separately, each battlefield entity/formation has **local morale**, affected by real supporting formation size (except Concord's immunity to the *small-group penalty*) and active effects such as Intimidation. The Concord, Directorate, Ascendancy and NPC Unaligned have distinct advanced propaganda methods through **radio, newspapers and television**; Meridian has no advanced propaganda. Population propaganda never grants spy tiers, instantly completes infiltration, or converts a unit. **Only Iron Ascendancy Intimidation** can compel an eligible isolated enemy to change allegiance, and **only after that unit's battlefield morale has been completely broken**. See [Propaganda and Morale](PROPAGANDA_AND_MORALE.md).

## 5. Data contracts and acceptance tests

**Suggested authoritative state** (names provisional):

- `FactionSpyProgressionTier`, `FactionSpyMaxTier`, `SpyProgressionComplete` and `NuclearFacilityInfiltrationUnlocked`.
- `ResearchFacilityOwner`, `ResearchFacilityType`, `SpyInfiltrationEvent`, and `VerifiedIntelUnlocks`.
- `BattlefieldMorale`, `MoraleBreakThreshold`, `MoralePressure`, `NearbyFormationSupport`, `ConcordNoSmallGroupPenalty` and `MoraleRecovery`.
- `IntimidationCapability`, `TargetCombatantStatus`, `TargetIsolated`, `TargetMoraleBroken`, `TargetFaction`, `CurrentController`, `OriginalOwner`, `IntimidationResolution`.

**Tests:**
1. A basic or partially developed Ascendancy spy **cannot** perform the qualifying nuclear-research infiltration.
2. With **fully developed spying**, an eligible Ascendancy spy can attempt to infiltrate a real **Concord or Directorate nuclear research facility**, subject to defenses, and may then unlock the two intelligence flags only on success.
3. Full spying with **no infiltration**, or infiltration into any other faction's site, does not unlock the Ascendancy bomb path.
4. Intimidation against an **isolated but high-morale enemy unit** reduces morale but **does not convert it** until its battlefield morale reaches the fully broken threshold.
5. Once an isolated eligible unit's morale has been **fully broken by Intimidation**, it switches allegiance; the same morale-broken unit **with allied support nearby** cannot be converted.
6. Lone/very small Concord formations suffer **no formation-size morale penalty**, whereas equal-sized Directorate, Ascendancy, Meridian and Unaligned units do; Concord units can still be demoralized by direct Intimidation pressure. **Very massive coherent Directorate and Ascendancy armies receive a distinct extra morale/discipline stability bonus**, which equally massive Meridian or Unaligned formations do not.
7. All other factions **lack Intimidation**, including captured or mercenary assets unless an explicitly authored exception is later introduced.
8. Intimidation alone does not unlock nuclear refining, weapon research or instant victory.
9. Theft of a finished bomb does not trigger the Ascendancy's own-bomb completion victory.
10. Completing its own first atomic bomb after the **max-spy-tier + qualifying research infiltration + two intelligence unlocks + refining + full build** chain immediately wins the active game, before any deployment.

**Open balancing:** distinct Concord/Directorate special espionage abilities, spy upgrade costs and timing, defenses/detection rules, exact formation-support radius, massive-army threshold and exclusive Directorate/Ascendancy stability bonus, ordinary morale/stability curves, fully broken threshold, morale erosion and recovery, valid intimidator and target archetypes, intimidation reliability/counterplay, and multiplayer arbitration.
