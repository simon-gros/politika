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

Its purpose is to **force an isolated enemy unit to change sides and join the Ascendancy**. The unit should remain present in the battlefield simulation and transfer controllable allegiance/ownership when the intimidation action succeeds, rather than being simply destroyed, spawned anew or treated as a temporary mercenary.

**Core target rule:** an enemy unit must be **isolated from supporting friendly units**, based on an inspectable in-game proximity/support condition. A surrounded squad in formation or a unit supported by nearby allies does not count as isolated. Target hostility/identity must be validated: neutral civilians or ordinary non-hostile Unaligned personnel should not become freely convertible targets.

**Potential balancing/counterplay, pending implementation details:**
- Clearly expose an eligibility indicator and the isolation condition to the player.
- Limit the action by unit archetype, range, time, cooldown, risk or other game costs; values are not yet locked.
- Permit opposing players to respond by reinforcing or regrouping an isolated unit, detecting an intimidator, or interrupting the action.
- Define how leadership, specialist and vehicle units interact with the mechanic, rather than allowing unrestricted conversions by default.
- Track the target's original faction and the Ascendancy as the new controller, with effects on diplomatic reputation, supply and AI recognition.
- Keep the intimidation action separate from the general damaging attacks that sometimes prioritize destruction at the expense of the Ascendancy unit's survival.

**Authoritative rules invariant:** Intimidation can resolve only when `ActorFaction == IronAscendancy`, the actor has the ability, `TargetIsEnemyCombatUnit`, and `TargetIsIsolated`; game-state ownership changes on a validated successful resolution. No conversion can occur from a cosmetic animation or UI event alone.

## 4. Story and AI integration

The Iron Ascendancy's **limited espionage** creates a distinctive difficulty curve: it is comparatively less capable of early information operations than the two intelligence-specialist nuclear powers, but it has **Intimidation** as a coercive tactical alternative. Its first-bomb instant victory becomes a late-game strategic goal precisely because it must fully develop spying, infiltrate protected Concord/Directorate research, steal both intelligence types, refine the rarest materials, and finish lengthy production.

The Concord and Directorate can pursue their own different spy-focused objectives and deploy defensive counterintelligence around nuclear research sites, power infrastructure and valuable archives. The Unaligned reacts to coercion, attacks and restricted-site violations through its neutral sovereignty and security rules.

The primary Tito-assassination cover-up remains the core 1984–1989 storyline. Espionage asymmetry and intimidation are gameplay and mission-design systems; they do not change the canonical 1989 revelation.

### Population morale and propaganda are distinct

All factions have **population morale**, which increases **production output and army discipline** when high. The Concord, Directorate, Ascendancy and NPC Unaligned have distinct advanced propaganda methods through **radio, newspapers and television**; Meridian has no advanced propaganda. Neither morale nor propaganda gives additional spy tiers, instantly completes infiltration, or converts an isolated enemy unit. **Only Iron Ascendancy Intimidation** can directly compel eligible isolated enemies to change allegiance, using its separate ability rules. See [Propaganda and Morale](PROPAGANDA_AND_MORALE.md).

## 5. Data contracts and acceptance tests

**Suggested authoritative state** (names provisional):

- `FactionSpyProgressionTier`, `FactionSpyMaxTier`, `SpyProgressionComplete` and `NuclearFacilityInfiltrationUnlocked`.
- `ResearchFacilityOwner`, `ResearchFacilityType`, `SpyInfiltrationEvent`, and `VerifiedIntelUnlocks`.
- `IntimidationCapability`, `TargetCombatantStatus`, `TargetIsolated`, `TargetFaction`, `CurrentController`, `OriginalOwner`, `IntimidationResolution`.

**Tests:**
1. A basic or partially developed Ascendancy spy **cannot** perform the qualifying nuclear-research infiltration.
2. With **fully developed spying**, an eligible Ascendancy spy can attempt to infiltrate a real **Concord or Directorate nuclear research facility**, subject to defenses, and may then unlock the two intelligence flags only on success.
3. Full spying with **no infiltration**, or infiltration into any other faction's site, does not unlock the Ascendancy bomb path.
4. A successfully intimidated **isolated enemy unit** changes sides; the same enemy unit **with allied support nearby** cannot be converted through this power.
5. All other factions **lack Intimidation**, including captured or mercenary assets unless an explicitly authored exception is later introduced.
6. Intimidation alone does not unlock nuclear refining, weapon research or instant victory.
7. Theft of a finished bomb does not trigger the Ascendancy's own-bomb completion victory.
8. Completing its own first atomic bomb after the **max-spy-tier + qualifying research infiltration + two intelligence unlocks + refining + full build** chain immediately wins the active game, before any deployment.

**Open balancing:** distinct Concord/Directorate special espionage abilities, spy upgrade costs and timing, defenses/detection rules, exact isolation geometry, valid intimidator and target archetypes, intimidation reliability/counterplay, and multiplayer arbitration.
