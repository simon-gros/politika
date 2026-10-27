# POLITIKA — Espionage, Psy-Ops, Counterintelligence and Intimidation

Status: Core faction asymmetry and the Iron Ascendancy nuclear-infiltration gate are canon; detailed abilities, progression costs and balancing values remain in design.  
Related: [Concepts Outline](CONCEPTS_OUTLINE.md) · [Nuclear Ultimate Weapons System](NUCLEAR_WEAPONS_SYSTEM.md) · [Story Outline](STORY_OUTLINE.md) · [Resource Economy](RESOURCE_ECONOMY.md) · [Faction Descriptions](../Factions/)

## 1. Faction intelligence identities

Espionage is asymmetric, not a universal identical spy-unit tree. The three principal military factions play differently.

| Faction | Established espionage identity | Exclusive or special rules |
|---|---|---|
| Atlantic Concord | Dedicated spying subdivision: Psy-Ops | Cause a target's Discipline (combat) or Morale (civilian/non-combat) to break, leading to self-destruction, or temporarily force friendly-fire attacks in a short rampage; no ownership transfer |
| Red Directorate | Slightly better ordinary infiltration for intelligence acquisition | Steals rival-faction intelligence for modestly lower operation costs and shorter completion time than competing factions |
| Iron Ascendancy | More limited espionage capabilities than the Concord and Directorate; high-value infiltration requires major investment and full development of its own spying progression | Only faction with Intimidation, which can force isolated eligible enemy units to join its forces; access to nuclear-research infiltration is locked until the Ascendancy's spy capabilities are fully developed |
| Meridian Pact | Commercial, mercenary and informant networks remain distinct from the main factions' specialist espionage programs | No Intimidation and no nuclear-refinement or nuclear-bomb production unlock |
| The Unaligned | Neutral NPC security, policing and counterintelligence; most personnel do not know Tito's assassination secret | Not playable and cannot use the Ascendancy's exclusive Intimidation power |

Atlantic Concord Psy-Ops and Red Directorate's more efficient basic intelligence theft are now established specializations, not merely example ideas. Detailed action names, eligibility, numerical cost/time modifiers, defenses and upgrade tiers remain design tasks. The Iron Ascendancy retains limited spying and exclusive Intimidation.

### 1.1 Atlantic Concord: Psy-Ops

Psy-Ops is an Atlantic Concord espionage subdivision that acts upon an individual enemy unit's existing `UnitResolve` value, called **Discipline** for combatants and **Morale** for civilian/non-combat units. An eligible operation can produce one of two mutually exclusive outcomes:

1. **Resolve breakdown and self-destruction:** the operation reduces the target's Discipline or Morale until it reaches `BrokenThreshold`. The affected unit then **self-destructs as a game-unit elimination event**. It does not become a Concord unit, and no realistic destructive method is simulated or specified.
2. **Short-lived friendly-fire rampage:** a successful alternative operation makes the targeted combat or non-combat unit briefly **attack eligible units belonging to its own faction**. This is a temporary hostile-targeting override with a definite expiry and return to ordinary targeting if the unit survives. It does not change faction ownership or recruit the unit for the Concord. Non-combat units require an authored, valid in-game attack action for this outcome; ability details remain to be balanced.

Both are bounded, counterable operations subject to faction ownership, target eligibility, cooldown/cost, valid range and success checks. The self-destruction outcome specifically requires a **fully broken** meter; the rampage is an **alternative successful Psy-Ops effect**, not necessarily a permanent break or a conversion. The precise activation threshold for rampage is open. Defensive responses may include spotting/interruption, protection effects, target recovery and preventing repeated ability chains.

Psy-Ops does not replace Concord radio/newspaper/television propaganda; the former affects individual units, while the latter operates through faction media and Population Morale. It is also distinct from Iron Ascendancy Intimidation: **Intimidation breaks Discipline/Morale to transfer a target's allegiance**, whereas Psy-Ops can **eliminate** a broken target or **temporarily redirect its attacks** without transferring ownership.

### 1.2 Red Directorate: efficient infiltration and intelligence theft

The Red Directorate's **basic spy infiltration is slightly more advanced specifically for obtaining rival intelligence**. In comparable, authorized intelligence-theft operations, Directorate agents require **slightly less gameplay time and slightly fewer game resources/costs** than other factions' agents. No exact percentage or currency price is canonical yet.

The advantage applies to stealing rival-faction intelligence, **not** to Intimidation, Psy-Ops, combat damage, mandatory facility permissions or guaranteed mission success. Target defenses, access, reconnaissance, detection and legitimate intelligence provenance still apply. The Directorate does not acquire a Concord Psy-Ops action tree, and does not bypass the Iron Ascendancy's separate maximum-spy-upgrade gate for infiltrating a Concord/Directorate nuclear research facility.

## 2. Iron Ascendancy's limited espionage and full-development gate

The Iron Ascendancy may perform ordinary limited covert actions through its basic spy capabilities, but it cannot infiltrate a rival nuclear research facility at a low or intermediate espionage level. The espionage progression must be fully developed / at its maximum eligible tier before nuclear infiltration becomes an allowed mission interaction.

Locked progression for its first independently produced atomic bomb:

1. Develop the Iron Ascendancy espionage technology/progression tree to full completion. Intermediate spy upgrades do not satisfy the gate.
2. Deploy an eligible Ascendancy spy and successfully infiltrate an Atlantic Concord or Red Directorate nuclear research facility. No other faction's facility satisfies this special condition.
3. Acquire and validate both nuclear-material refinement technology intelligence and nuclear-weapons production intelligence. These are separate required game-state flags; scenario authors can put both in one qualifying research facility.
4. Acquire the highest-value crude nuclear-material game resources and refine them after the stolen-technology unlock. The Ascendancy's processed nuclear materials can be used for bomb production only; it cannot build nuclear power plants.
5. Complete the expensive, very long production process for the Ascendancy's first own-produced atomic bomb.
6. Immediately win the active game on completion. No deployment, bombing mission or detonation is required.

The game must validate both the maxed spy progression and the correct research-facility infiltration event. Infiltration without full espionage development is not valid; maxing spying alone does not grant nuclear research. Merely scouting a site, stealing an already completed bomb, or persuading an enemy unit through Intimidation does not fulfill this nuclear-intelligence requirement. Intimidation cannot replace the spy upgrade or research-theft gates.

## 3. Iron Ascendancy exclusive ability: Intimidation

Intimidation belongs exclusively to the Iron Ascendancy. It can target eligible isolated combat units and eligible isolated non-combat/civilian units. Intimidation does not instantly convert an isolated unit: it must fully break the target's meter before ownership changes.

### One meter, two names

Discipline and morale are the same underlying unit meter, not two separate resistance systems. The distinction is exclusively the unit category and its user-facing label:

| Target category | UI / documentation term | One shared underlying meter | Requirement before conversion |
|---|---|---|---|
| Combat unit | Discipline | `UnitResolve` | `UnitResolve <= BrokenThreshold` (Discipline broken) |
| Civilian or other eligible non-combat unit | Morale | `UnitResolve` | `UnitResolve <= BrokenThreshold` (Morale broken) |

Both categories use the same state representation, depletion/recovery mechanism, break threshold and conversion pipeline. Do not implement parallel `UnitDiscipline` and `NonCombatUnitMorale` systems. Gameplay modifiers may depend on unit type: the formation-size discipline bonuses apply to combat units, while civilian morale can respond to civilian conditions and propaganda.

Population morale remains the faction-wide morale indicator for all five factions, representing the broader civilian/population state and informing production and combat units' baseline discipline. It is a faction-level view/aggregation of the same morale/resolve framework, not a third independently invented meter with incompatible thresholds. Breaking one civilian's morale does not zero the whole faction's population morale or convert other units.

### Eligibility and isolation

The target must be an eligible, isolated individual unit, without qualifying nearby friendly support. A combat unit and a non-combat unit may both be targets. Civilians, workers, engineers, couriers and support units are not automatically excluded simply because they do not fight, but must pass authored eligibility and hostility/relationship rules. Neutral Unaligned civilians do not become universally convertible targets by default; eligibility and sovereignty consequences must be explicit.

Isolation is necessary but never sufficient for conversion. While eligible and isolated, an Intimidation action erodes that unit's shared `UnitResolve` meter. The target stays with its original controller as long as its relevant Discipline (combat) or Morale (non-combat) is above the broken threshold. Only after the meter is fully broken may control of that same individual unit transfer to the Ascendancy.

### Formation-size and faction asymmetry

For combat units, larger coherently supported groups stabilize Discipline for the Red Directorate, Iron Ascendancy, Meridian Pact and Unaligned. Red Directorate and Iron Ascendancy alone gain an extra Discipline stability bonus in very massive coherent armies. Atlantic Concord alone suffers no Discipline penalty for deploying individual soldiers or tiny groups, but Concord combat units can still have their Discipline eroded by direct Intimidation. There is no equivalent “massive army” bonus for civilian Morale, because civilians use the same meter but different contextual rules.

### Intimidation sequence and counterplay

The sequence is the same for both kinds of targets:

`Eligible isolated target → Ascendancy Intimidation applies shared-meter reduction → UnitResolve reaches BrokenThreshold → validated conversion to Ascendancy control`.

Label the reduction Discipline loss for combat targets and Morale loss for civilian/non-combat targets. Valid counterplay includes regrouping and reinforcement (to end isolation), interrupting the intimidator, withdrawal and meter recovery. Eligibility, range, timing and the precise threshold remain balance decisions.

Authoritative invariant: conversion requires `ActorFaction == IronAscendancy && TargetEligible && TargetIsolated && UnitResolve <= BrokenThreshold && IntimidationResolutionAuthorized`. The same predicate applies to combat and non-combat units; the UI label comes from target classification, not a second resource. Mere isolation, partial depletion, damage, a visual effect or population-wide propaganda does not convert a unit. Ownership transfer is a separate validated event.

## 4. Story and AI integration

The Iron Ascendancy's limited espionage creates a distinctive difficulty curve: it is comparatively less capable of early information operations than the two intelligence-specialist nuclear powers, but it has Intimidation as a coercive tactical alternative. Its first-bomb instant victory becomes a late-game strategic goal precisely because it must fully develop spying, infiltrate protected Concord/Directorate research, steal both intelligence types, refine the rarest materials, and finish lengthy production.

The Concord can conduct Psy-Ops against eligible enemy combat/civilian units, while the Directorate retrieves rival intelligence at modestly lower time and resource costs. Both retain ordinary defensive counterintelligence around nuclear research sites, power infrastructure and valuable archives. The Unaligned reacts to coercion, attacks and restricted-site violations through its neutral sovereignty and security rules.

The primary Tito-assassination cover-up remains the core 1984–1989 storyline. Espionage asymmetry and intimidation are gameplay and mission-design systems; they do not change the canonical 1989 revelation.

### Population morale, unit resolve and propaganda

All factions show population morale and benefit from higher production and army discipline. Population morale is the faction-level indicator of the same underlying morale/resolve framework. Every unit has one resolve meter: it appears as Discipline on combat units and Morale on civilian/non-combat units. Its depletion and recovery work identically; formation bonuses apply only to combatants, while civilian contexts can use other modifiers. The Concord, Directorate, Ascendancy and NPC Unaligned have advanced propaganda through radio, newspapers and television; Meridian does not. Propaganda does not grant extra spy tiers, bypass nuclear research or automatically convert units. See [Propaganda and Morale](PROPAGANDA_AND_MORALE.md).

## 5. Data contracts and acceptance tests

Suggested authoritative state (names provisional):

- `FactionSpyProgressionTier`, `FactionSpyMaxTier`, `SpyProgressionComplete` and `NuclearFacilityInfiltrationUnlocked`.
- `ResearchFacilityOwner`, `ResearchFacilityType`, `SpyInfiltrationEvent` and `VerifiedIntelUnlocks`.
- `EspionageActionType`, `IntelTheftBaseCost`, `IntelTheftBaseDuration`, `DirectorateIntelCostModifier`, `DirectorateIntelDurationModifier`; modest discounts apply only for eligible Directorate rival-intelligence theft.
- `PsyOpsOutcome` (`SelfDestruct` or `BriefRampage`), `PsyOpsTargetEligible`, `PsyOpsBreakConfirmed`, `RampageEndTime`, `RampageTargetFilter`, `OriginalFaction` and `OriginalController`; never confuse a temporary target override with ownership transfer.
- One shared per-unit meter: `UnitResolve`, `BrokenThreshold`, `ResolveDelta`, `ResolveRecovery` and `TargetUnitCategory`; do not store separate combat-discipline and civilian-morale meters for the same unit.
- `FactionPopulationMorale` as faction-level summary/input using that morale framework, not a second unit resistance system.
- `NearbyFormationSupport`, `ConcordNoSmallGroupDisciplinePenalty`, `DirectorateAscendancyMassArmyDisciplineBonus`, `TargetIsolated` and `IntimidationResolution`.
- `OriginalOwner`, `CurrentController` and unit identity provenance.

Acceptance tests:

0. Concord alone can use Psy-Ops. In the self-destruction mode, the target must have its own Discipline (combat) or Morale (non-combat) fully broken before an in-game elimination event. A rampage lasts only a short bounded duration, makes a target attack valid own-faction units, and never permanently transfers ownership; combat and eligible civilian/non-combat target categories are supported.
0a. For an otherwise identical permitted rival-intelligence infiltration, Directorate time and cost are each modestly below the baseline for other factions. Its bonus does not grant automatic success or enable Concord Psy-Ops or Ascendancy Intimidation.

1. A partially developed Ascendancy spy cannot infiltrate a nuclear research facility; maxed spying permits a qualifying attempt at a Concord/Directorate site, but no intelligence unlock without success.
2. An eligible isolated combat unit with remaining Discipline can lose meter value under Intimidation but cannot be converted until its shared meter reaches the broken threshold.
3. An eligible isolated civilian/non-combat unit with remaining Morale behaves identically: no conversion until the same shared-meter break condition is met.
4. A morale-broken non-combat unit and discipline-broken combat unit each convert only while still isolated and otherwise eligible; reinforcement blocks either conversion.
5. Concord combat units suffer no small-group Discipline penalty, but can be affected by Intimidation; other factions incur the ordinary discipline penalty when unsupported.
6. Very massive, coherently supported Directorate and Ascendancy combat armies receive an extra Discipline stability bonus. Meridian and Unaligned do not; civilian units never inherit the army bonus.
7. Breaking a single civilian's Morale does not zero the faction-wide Population Morale and does not convert the entire population.
8. The UI shows Discipline on combat targets and Morale on non-combat targets, but the data model stores one `UnitResolve` mechanism.
9. All other factions lack Intimidation; coercive conversion does not bypass the Ascendancy's espionage and nuclear-research prerequisites.
10. Completing the Ascendancy's first self-produced bomb after full spying, valid research infiltration, dual intelligence unlock, refining and construction ends the active game before deployment; theft does not.

Open balancing: Concord Psy-Ops eligibility, outcome selection, rampage duration, valid non-combat attacks and defense; Directorate intelligence-theft cost/time reductions; Ascendancy spy upgrade costs and timing, defenses/detection rules, exact formation-support radius, massive-army threshold and exclusive Directorate/Ascendancy stability bonus, shared resolve/discipline/morale curves, a common broken threshold, depletion and recovery, valid intimidator and target archetypes, intimidation reliability/counterplay, and multiplayer arbitration.
