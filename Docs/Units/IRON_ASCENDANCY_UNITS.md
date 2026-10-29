# POLITIKA — Iron Ascendancy Unit Specifications

**Status:** PROPOSED unit gameplay specification (29 October 2026). No unit in this file is implemented. Numbers are Provisional Baseline (PB) values.
**Authority:** names and IDs come from the [Five-Faction Rosters](../POLITIKA_FIVE_FACTION_ROSTERS.md); shared rules from the [Unit Specification Framework](../UNIT_SPECIFICATION_FRAMEWORK.md); faction canon from the [faction description](../../Factions/TheIronAscendancy/description.md). Owner documents win on conflict.
**Evidence labels:** I / E / E-R / P / U as in framework §1.1. A field without a label is P. Cross-faction views: [Unit Balance and Ability Matrices](../UNIT_BALANCE_AND_ABILITY_MATRICES.md). Coverage: [Unit Specification Coverage Report](../Reports/UNIT_SPECIFICATION_COVERAGE_20261029.md).
**Siblings:** [Atlantic Concord](ATLANTIC_CONCORD_UNITS.md) · [Red Directorate](RED_DIRECTORATE_UNITS.md) · [Meridian Pact](MERIDIAN_PACT_UNITS.md) · [The Unaligned](UNALIGNED_UNITS.md).

## 1. Faction doctrine and faction-wide unit rules

The Ascendancy fights for decisive breakthroughs. Armor penetrates fortified positions, mechanized infantry and mobile artillery exploit the gap, reconnaissance finds the weak point, and elite assault units strike command, communications and infrastructure (E: faction description, *Strategic Philosophy*). Its equipment is advanced, heavily protected and expensive, with exceptional production, maintenance and fuel demands (E: *Technological Development*). Heavy losses are therefore costly, and the faction is weakest when the initiative is lost, supply corridors stretch and elite formations are isolated or forced into repeated engagements without maintenance (E: *Defensive Vulnerabilities*). Its coercive systems, Intimidation and facility contamination, are exclusive, explicit and counterable, and its aggression always carries a visible price.

| Rule | Unit-level implementation | Label |
| --- | --- | --- |
| Slightly greater raw destructive power | **All Ascendancy weapons +8 % damage**, applied once as a documented faction modifier (framework §4.4, §18.4). **Every damage value in this file is the base value before the +8 %**; DPS figures quoted in Counters include it | E (rule), P (value) |
| Explicit high-risk aggression | Self-endangering abilities are named per unit and always carry a self-damage, exposure, resolve or equipment cost: *No Retreat Order* (IA-INF-01), *Danger Close* (IA-INF-03), *Overdrive* (IA-ARM-03), *Ram Breach* (IA-ARM-07), *Last Stand* (IA-INF-13 talent), *Press the Breach* (IA-ARM-05). None is passive or constant (E: "not constant or unavoidably successful") | E (rule), P (abilities) |
| Costly advanced equipment | Vehicle, aircraft and ship prices sit at the **upper edge** of each §4.3 band; infantry sits at the band middle. Heavy and super-heavy hulls pay **+30 % petroleum** at production and **+25 % repair price** (E: high maintenance; P: values) | E, P |
| Discipline | Ordinary group support applies. **Very-massive-army bonus:** 12+ coherent combat units within 1500 cm, stress taken −20 % (E, shared with the Directorate). Individual soldiers and small groups suffer the **normal isolation penalty** (E) | E (rule), P (values) |
| Opening scout | `IA-SUP-01` Nightjar Recon Plane: cheap and **unarmed**; no covert intelligence theft. Validator: no weapon (E). `IA-ARM-01` is armed road recon and is **not** the opening scout (E-R) | E |
| Sabotage access | Attached charges only through `IA-INF-09` after `tech.ia.late_espionage` (maps to I `late_espionage_unlocked`), at **$300 per charge**, above Meridian's non-spy price (E). IA-INF-07 engineers, IA-INF-13 raiders, the scout plane and self-sacrificing attackers can never plant charges (E) | E (I predicate) |
| Limited espionage | Spy progression `tech.ia.espionage_tier_1` → `_2` → `_3`; only `_3` permits nuclear-research infiltration of a Concord or Directorate facility (E). `tech.ia.advanced_clandestine_training` (E: research catalogue) is the `_1` step. Base infiltration is slower than Concord and Directorate (P: +25 % channel time) | E (gate), P (values) |
| Exclusive Intimidation | `IA-INF-11` only. Conversion predicate: `ActorFaction == IronAscendancy && TargetEligible && TargetIsolated && UnitResolve <= BrokenThreshold && IntimidationResolutionAuthorized` (E). Eligibility per framework §7.5 | E (predicate), P (values) |
| Facility contamination | `IA-INF-10` only. Abstract staffing-loss status on eligible staffed enemy buildings, bounded chain spread, containment and recovery (E). No pathogen, poison or procedure is modelled (E) | E (rule), P (values) |
| Propaganda | Radio, newspaper and television (IA-BLD-17, IA-DEF-09, IA-DEF-10). Population Morale sets the resolve baseline maximum; repression and credibility costs apply (E) | E |
| Nuclear | Initially non-nuclear. First own bomb only after full spy progression and verified theft of **both** intelligence flags; **completion immediately wins** (E). No unit carries an Ascendancy nuclear weapon, and no nuclear power plant exists (E). A stolen completed enemy weapon follows the nuclear contract and never counts toward the production victory (E) | E |
| Unaligned hostility | Identified Ascendancy forces carry a standing Unaligned hostility modifier (E). Units with occupation, detention or coercion roles add jurisdiction incidents (IA-INF-12, IA-INF-11, IA-SUP-03) | E (rule), P (per unit) |
| Food and fuel | Standard food upkeep (1 per staff per interval). Fuel classes per §4.5 with the heavy-hull surcharge above | E (food), P |
| Visual language | Charcoal-black armor with crimson trim, respirator helmets with red lenses, heavy angular silhouettes. **Production risk:** some sheets (notably `IA-INF-13`) lean toward stylized fantasy; production assets must be reconciled with roster rule 7 (visually plausible 1980s equipment). Vehicles: dark grey hulls, crimson unit bands, angular stowage. Voice set: clipped, formal, aggressive | P (from concept art) |

**C&C design lessons applied.** The roster borrows the Nod and Soviet assault toolkit as gameplay functions: close-assault elites that clear buildings (KW Black Hand, TS flame infantry), flame vehicles that punish garrisons and massed infantry (TD Flame Tank), a fast light tank that trades protection for tempo (TW Scorpion), a deployable infantry bunker (TS Tick Tank, YR Battle Bunker) and saturation rocket artillery with supply limits (TD SSM, RA2 V3). Lessons avoided: the RA2 Apocalypse or Generals Overlord "no answer" super tank (IA-ARM-06 is cap 1, has huge fuel and repair costs and is air-vulnerable), Yuri-style instant mind control (Intimidation needs isolation plus a fully broken meter, E), and the TS/TW stealth-everything problem (no invisibility; concealment always has a tell, framework §9).

## 2. Core roster selection

Core (16 units):

| ID | Role covered | Reason |
| --- | --- | --- |
| IA-INF-01 | Line infantry | Hard-hitting garrison rifles with an explicit attrition gamble |
| IA-INF-02 | Faction signature: close assault | Building-clearing shock troops |
| IA-INF-04 | Anti-armor infantry | Guided ambush missiles |
| IA-INF-06 | Medic / sustainment | Keeps expensive elite infantry alive |
| IA-INF-07 | Engineer / capture | Capture, repair and obstacle clearing |
| IA-INF-11 | Faction signature: Intimidation | The exclusive coercion mechanic (E) |
| IA-SUP-01 | Opening scout (E) | Unarmed recon plane |
| IA-SUP-02 | Constructor | Mobile works for forward construction |
| IA-ARM-02 | Transport | Half-track for mechanized infantry |
| IA-ARM-03 | Light / fast tank | Breakthrough tempo |
| IA-ARM-04 | Main battle tank | Hardest-hitting standard MBT |
| IA-ARM-08 | Mobile air defense | Protects heavy columns |
| IA-ARM-09 | Tank destroyer | Ambush anti-armor |
| IA-ARM-10 | Artillery | Saturation rockets |
| IA-SUP-05 | Repair and recovery | Preservation of expensive armor |
| IA-AIR-03 | Air support | Assault helicopter |

**Extended:** IA-INF-03, IA-INF-05, IA-INF-08, IA-INF-12, IA-INF-13 (commander, pending U-14), IA-ARM-01, IA-ARM-05, IA-ARM-07, IA-ARM-11 to IA-ARM-14, IA-SUP-03, IA-SUP-04, IA-AIR-01, IA-AIR-02, IA-AIR-04, IA-AIR-05, IA-AIR-06.
**Conditional:** IA-INF-09 and IA-INF-10 (espionage track: charges need `tech.ia.late_espionage`, contamination needs `tech.ia.espionage_tier_2`), IA-INF-14 and IA-SUP-06 (covert logistics, jurisdiction risk), IA-ARM-06 (prototype cap 1), IA-ARM-15 (endgame emplacement, cap 2), IA-AIR-07 (prototype, cap 1), IA-AIR-08 (covert air courier, cap 2), IA-SEA-01 to IA-SEA-06 (authored water layer and IA-BLD-09).
**Scenario:** none.

## 3. Infantry, security and clandestine personnel

### IA-INF-01 — Garrison Riflemen

| Field | Specification |
| --- | --- |
| **Identity** | `unit.ia.inf_01` · Iron Ascendancy · Infantry / line rifle squad · `TPL-INF-LINE` · **Core** · Playable |
| **Role** | **Primary:** conventional rifle squad with slightly higher attack and harsh attrition risk (E-R). **Secondary:** garrison of occupied points; screen for armor; Intimidation set-up by pinning isolated enemies. **Doctrine fit:** the infantry that holds ground behind the armored thrust (E: mechanized infantry secures territory). |
| **Inspiration and background** | Late-Cold-War conscript-and-cadre rifle squad with battle rifles, a general-purpose machine gun and steel helmets with respirator carriers. In-universe, the occupation garrison troops raised by party militias and regular formations. C&C lineage (E-R): Nod Minigunner (TD); Nod Militant (TW); Devout (TT). |
| **Visual identity** | Concept: [`IA-INF-01.png`](../../Factions/TheIronAscendancy/Units/IA-INF-01.png). Four members, charcoal greatcoats, respirator helmets with red lenses, crimson collar tabs; one member carries the GPMG. |
| **Statistics** | 4 × 65 HP = **260** (`armor.inf_light`) · FOOT **205 cm/s** · F0 · sight **1200 cm** · no detector · 1 slot · food staff 4 · fuel none · ExperienceValue 32 |
| **Commands** | Move, Attack, AttackMove, Stop, Guard, Grenade, Reload, Sprint (I) · Garrison, Patrol, Withdraw, Scatter (P) · default stance Aggressive · formation role `frontline` |
| **Production** | **$320** (band middle) · `IA-BLD-03` Paramilitary Training Camp · prerequisites: `IA-BLD-01` operational and powered · **8 s** · 4 food/interval · replenishment per §10.3 · no cap |
| **Veterancy** | Standard (§8) |
| **AI** | `ai.unit.line_infantry`. Deltas: use *No Retreat Order* only when the local force ratio is ≥ 1.2 and the target is a garrison or an isolated enemy squad; never when Shaken or worse |
| **Multiplayer** | MPA-STD. *No Retreat Order* is visible to observers as a status icon (red banner), so opponents can read the gamble |
| **Presentation** | ANIM-INF plus reload, throw, prone/pinned, garrison, a fist-raised order gesture for *No Retreat Order*. Heavy, deliberate rifle reports; icon of a rifle with a crimson bar |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.ia.inf_01.primary` battle rifles + GPMG | SMALL_ARMS | 0–680 cm | 11 per member / 1.0 s (44 DPS base, 47.5 with +8 %); magazine 30, reload 3 s (I values) | hitscan, tracking fast | 3 | GROUND, AIR_LOW | 100 % / 15 % / 2 % (never auto) / 25 % rotary / 8 % |
| `ability.shared.grenade` (I) | HIGH_EXPLOSIVE | 650 cm (I) | 50 (I), radius 150 cm (I) | lobbed (I) | 8 | GROUND, STRUCTURE | 110 % / 45 % / 12 % / — / 100 % |

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.shared.grenade`, `.sprint`, `.reload` | Active (I) | Implemented infantry behavior | as framework §2 | I values | as AC-INF-01 | standard | as `ai.unit.line_infantry` |
| `ability.ia.inf_01.no_retreat_order` | Active, **self-endangering** (P; E: explicit high-risk aggression) | For 15 s: weapon cooldown −30 %, ignores Shaken and Pinned movement limits, cannot Withdraw and ignores the Breaking auto-withdraw. The squad takes +20 % damage. When it ends, −25 resolve | Self; Steady or Shaken only | free · 15 s · 45 s · self | HMG and HE (the +20 % damage window), flame; Psy-Ops stress after the window ends | Red banner icon visible to observers; shouted order bark | Only at a force ratio ≥ 1.2 against garrisons or isolated squads |

**Research and upgrades:** `tech.shared.infantry_protective_equipment`: member HP 65 → 80 at −5 % speed. `tech.ia.assault_doctrine` (IA-BLD-14, $700): *No Retreat Order* duration 15 → 20 s; trade-off: the post-order resolve loss rises 25 → 35. `tech.shared.field_medical_procedures`: replenishment time −30 %.

**Counters:** *Beats (hard):* agents and unarmed support in the open. *Beats (soft):* Directorate platoons (RD-INF-01) at equal cost, Meridian levies (MP-INF-12), scout vehicles (RD-ARM-01, MP-ARM-01). *Beaten by (hard):* HMG (RD-INF-03), grenadiers (AC-INF-04), mortars (RD-INF-05, MP-INF-04), artillery. *Beaten by (soft):* Concord sections in cover (AC-INF-01) on cost efficiency. *Situational:* the best Ascendancy unit for creating the isolated, broken targets IA-INF-11 needs.

**Status:** identity E-R; +8 % rule E; grenade, sprint, reload and orders I (generic infantry behavior, not this roster unit); all numbers and abilities P.

### IA-INF-02 — Shock Assault Section

| Field | Specification |
| --- | --- |
| **Identity** | `unit.ia.inf_02` · Iron Ascendancy · Infantry / close assault · `TPL-INF-LINE` (protected) · **Core** · Playable |
| **Role** | **Primary:** close-assault specialists with aggressive breach behavior (E-R). **Secondary:** clear garrisons and bunkers; kill infantry in urban terrain. **Doctrine fit:** elite assault units that strike command and communications points (E: *Strategic Philosophy*). |
| **Inspiration and background** | Body-armored assault pioneers with submachine guns, combat shotguns and breaching tools. In-universe, the regime's showcase storm detachments. C&C lineage (E-R): Black Hand (KW); Confessor Cabal (KW); Nod Shotgun Trooper (Ren). |
| **Visual identity** | Concept: [`IA-INF-02.png`](../../Factions/TheIronAscendancy/Units/IA-INF-02.png). Heavier silhouettes with flak plates, red-lens respirators and short weapons; one member carries a breaching charge pack (a tool, not a sabotage charge). |
| **Statistics** | 4 × 80 HP = **320** (`armor.inf_protected`) · FOOT **215 cm/s** · F0 · sight 1200 cm · 1 slot · food staff 4 · fuel none · ExperienceValue 46 |
| **Commands** | as TPL-INF-LINE + Breach (P) · default stance Aggressive · formation role `frontline` |
| **Production** | **$460** (specialist band; high-cost elite) · `IA-BLD-03` · prerequisites: `IA-BLD-14` operational · **11 s** · 4 food · no cap |
| **Veterancy** | Standard |
| **AI** | `ai.unit.line_infantry` with weights 1.3 / 0.4 / 0 / 0 / 1.0 (garrisons prioritized). Deltas: close to 300 cm before firing; *Breach* any observed garrison within 600 cm; avoid open ground under observed HMG fire |
| **Multiplayer** | MPA-STD. *Breach* ejection is server-resolved; ejected occupants keep their owner and orders are cleared |
| **Presentation** | ANIM-INF + breach (kick and throw) and close-quarters fire. Shotgun boom and SMG bursts; icon of a boot over a door |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.ia.inf_02.primary` SMGs and combat shotguns | SMALL_ARMS | 0–420 cm | 16 per member / 0.8 s (80 DPS base at 4 members); **+25 % within 250 cm**; magazine 20, reload 2.5 s | hitscan, tracking fast | 4 | GROUND | 100 % / 15 % / 2 % / — / 8 % |
| `ability.shared.grenade` (I) | HIGH_EXPLOSIVE | 650 cm | 50, 150 cm | lobbed | 8 | GROUND, STRUCTURE | 110 % / 45 % / 12 % / — / 100 % |

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.shared.grenade`, `.sprint`, `.reload` | Active (I) | as framework §2 | — | I values | — | standard | standard |
| `ability.ia.inf_02.breach_and_clear` | Active (P) | 3 s channel adjacent to a garrisoned structure or bunker: occupants take a HIGH_EXPLOSIVE 80 hit (garrison +50 % grenade rule applies) and are **ejected** with 1 s stun; the section may then enter if eligible | Hostile garrisoned structure or emplacement ≤ 150 cm | free · 3 s channel · 30 s · 150 cm | Killing or suppressing the section during the channel (Pinned interrupts); defenders outside the building; flame | Kick-and-throw animation; door blast VFX; channel bar | Use on any observed garrison within 600 cm when the section is Steady |

**Research and upgrades:** `tech.shared.infantry_protective_equipment`: 80 → 95 per member at −5 % speed. `tech.ia.assault_doctrine`: *Breach and Clear* channel 3 → 2 s; trade-off: cooldown 30 → 40 s.

**Counters:** *Beats (hard):* garrisoned infantry (AC-INF-02, RD-INF-07 in depots, MP-INF-02), infantry in urban close range. *Beats (soft):* line squads at < 300 cm. *Beaten by (hard):* HMG at range (RD-INF-03), flame (RD-ARM-10), any vehicle (it has no anti-armor weapon), artillery. *Beaten by (soft):* marksmen (AC-INF-08, MP-INF-11). *Situational:* the urban key that unlocks fortified towns; wasted on open ground.

**Status:** identity E-R; all values and abilities P.

### IA-INF-03 — Heavy Grenadier Squad

| Field | Specification |
| --- | --- |
| **Identity** | `unit.ia.inf_03` · Iron Ascendancy · Infantry / area suppression · `TPL-INF-HW` · **Extended** · Playable |
| **Role** | **Primary:** close-quarters explosives and area suppression (E-R). **Secondary:** smoke; anti-garrison; light-vehicle harassment. **Doctrine fit:** suppress defenders so the assault can close (E: rapid disruption of defensive formations). |
| **Inspiration and background** | Squad equipped with automatic and rifle grenade launchers and satchel-type demolition tools. C&C lineage (E-R): Grenadier (TD); Fanatics (TW, risk behavior); Disc Thrower (TS, area role). |
| **Visual identity** | Concept: [`IA-INF-03.png`](../../Factions/TheIronAscendancy/Units/IA-INF-03.png). Three members with bandoliers of rounds and drum-fed launchers. |
| **Statistics** | 3 × 70 HP = **210** (`armor.inf_light`) · FOOT_HEAVY **190 cm/s** · F0 · sight 1200 cm · 1 slot · food staff 3 · fuel none · ExperienceValue 42 |
| **Commands** | as TPL-INF-HW (no deploy needed; launchers fire on the move at a penalty) + Fire Mission (P) · default stance Aggressive · formation role `support_weapon` |
| **Production** | **$420** · `IA-BLD-03` · prerequisites: `IA-BLD-03` only · **10 s** · 3 food · no cap |
| **Veterancy** | Standard; scatter bonuses apply |
| **AI** | `ai.unit.support_weapon`. Deltas: target clusters of ≥ 3 infantry or any garrison; *Danger Close* only when an enemy squad is inside 250 cm and the grenadiers are already engaged |
| **Multiplayer** | MPA-STD + MPA-INDIRECT (lobbed scatter uses the server PCG stream) |
| **Presentation** | ANIM-INF + launcher aim/fire/reload. Thump and crump audio; arc preview for Fire Mission; icon of three grenades |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.ia.inf_03.primary` grenade launchers | HIGH_EXPLOSIVE | **250**–750 cm | 30 per member / 2.5 s; AmmoCapacity none (magazine 6, reload 4 s) | lobbed, 600 cm/s; full 100 cm, max 200 cm, edge 30 %; scatter 40–140 cm | 8 | GROUND, STRUCTURE | 110 % / 45 % / 12 % / — / 100 % |
| `weapon.ia.inf_03.smoke` | none (smoke) | 250–750 cm | — | 400 cm smoke cylinder, 20 s (§9.5) | — | GROUND | utility |

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.ia.inf_03.smoke_volley` | Active (P) | Fires `weapon.ia.inf_03.smoke` at a point | Ground point within range | free · 20 s smoke · 35 s · 750 cm | Detectors cannot see in, but neither can the squad; wind is not modelled | Smoke VFX; owner range ring | Use to cover an advance across ≥ 600 cm of open ground under observed fire |
| `ability.ia.inf_03.danger_close` | Active, **self-endangering** (P) | For 10 s, minimum range drops 250 → 0 cm. Friendly units in the AoE take **100 %** instead of 50 %, and the squad itself takes 30 % of each blast it causes within 250 cm | Self | free · 10 s · 40 s · self | Pull back and let it hurt itself; marksmen | Red warning ring around the squad | Only when enemy infantry are inside 250 cm and the squad is above 60 % HP |

**Research and upgrades:** `tech.shared.infantry_protective_equipment`. `tech.ia.improved_launcher_rounds` (IA-BLD-14, $600): damage 30 → 36; trade-off: magazine 6 → 5.

**Counters:** *Beats (hard):* garrisons, clustered infantry, pinned squads (AC-INF-01, RD-INF-01 blocks, MP-INF-10 in cover). *Beats (soft):* soft vehicles (RD-ARM-01, MP-ARM-01/02). *Beaten by (hard):* any armor that closes, marksmen (AC-INF-08, MP-INF-11), counter-mortar fire. *Situational:* anchors the shock assault behind IA-INF-02.

**Status:** identity E-R; all values and abilities P.

### IA-INF-04 — Portable Anti-Armor Cell

| Field | Specification |
| --- | --- |
| **Identity** | `unit.ia.inf_04` · Iron Ascendancy · Infantry / guided anti-armor · `TPL-INF-AT` · **Core** · Playable |
| **Role** | **Primary:** guided rockets for ambush against vehicles (E-R). **Secondary:** anti-structure; anti-boat on river banks. No anti-air mode, which sets it apart from Concord and Directorate rocket infantry. **Doctrine fit:** guided weapons from the faction's research institutions (E: *Technological Development*). |
| **Inspiration and background** | Three-person team with a wire-guided man-portable ATGM and a reload carrier. C&C lineage (E-R): RPG Trooper (Gen); Militant Rocket Squad (TW); Tankbuster (RA3). |
| **Visual identity** | Concept: [`IA-INF-04.png`](../../Factions/TheIronAscendancy/Units/IA-INF-04.png). Tripod launcher, missile tubes on backpacks, charcoal ponchos. |
| **Statistics** | 3 × 60 HP = **180** (`armor.inf_light`) · FOOT **195 cm/s** · F0 · sight 1200 cm · 1 slot · food staff 3 · fuel none · ExperienceValue 52 |
| **Commands** | as TPL-INF-AT + Conceal (P) · default stance Guard · formation role `support_weapon` |
| **Production** | **$520** (specialist band; guided premium) · `IA-BLD-03` · prerequisites: `IA-BLD-14` operational · **12 s** · 3 food · no cap |
| **Veterancy** | Standard |
| **AI** | `ai.unit.anti_armor_infantry`. Deltas: wait Concealed on Hold Ground until a vehicle enters 900 cm; relocate 300 cm after two missiles |
| **Multiplayer** | MPA-STD + MPA-COVERT (concealment). Missile flight is server-resolved; guidance loss inside jamming per §6.6 |
| **Presentation** | ANIM-INF + launcher deploy (1 s), aim, fire, reload. Wire-guided missile trail; icon of a missile over a tripod |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.ia.inf_04.primary` man-portable ATGM | SHAPED_CHARGE | **150**–1000 cm | 150 / 6.0 s (1 launcher per cell); AmmoCapacity 8, resupply §10.4 | guided_missile, 450 cm/s; no AoE | 2 | GROUND, STRUCTURE, WATER_SURFACE | 25 % (never auto) / 100 % / 90 % / — / 70 % |
| `weapon.ia.inf_04.secondary` carbines | SMALL_ARMS | 0–550 cm | 7 per member / 1.0 s | hitscan | 2 | GROUND | 100 % / 15 % / 2 % / — / 8 % |

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.shared.conceal` | Conditional (P) | Concealed after 3 s stationary (§9.1) | Self | free · until broken · — · self | `DETECT.CONCEALED` (RD-INF-09, AC-INF-08), scouts, firing reveals for 4 s | Own-only concealment icon | Default when on Hold Ground |
| `ability.ia.inf_04.ambush_volley` | Passive (P) | The first missile fired from Concealed deals +25 % damage | Automatic | free · — · resets after 10 s Concealed · weapon range | Detection before the shot; smoke | Muzzle flash; "ambush" ping to owner | Always open from concealment |

**Research and upgrades:** `tech.ia.wire_guidance_improvement` (IA-BLD-14, $650): range 1000 → 1150 cm; trade-off: missile speed 450 → 400 cm/s (more time for the target to break line of fire). `tech.shared.electronic_countermeasures` does not apply (wire guidance is unaffected by jamming fields; P design choice that distinguishes it from radio-guided missiles).

**Counters:** *Beats (hard):* MBTs and IFVs that advance without infantry (AC-ARM-04, RD-ARM-04, MP-ARM-06), APCs (AC-ARM-02, RD-ARM-03). *Beaten by (hard):* HMG (RD-INF-03), flame, grenadiers (AC-INF-04), artillery, attack helicopters (it has no AA). *Beaten by (soft):* recon infantry that detect it. *Situational:* strongest on narrow mountain roads and bridges; useless against air.

**Status:** identity E-R; jamming immunity of wire guidance P; all values P.

### IA-INF-05 — Urban Flame Detachment

| Field | Specification |
| --- | --- |
| **Identity** | `unit.ia.inf_05` · Iron Ascendancy · Infantry / flame assault · `TPL-INF-HW` · **Extended** · Playable |
| **Role** | **Primary:** high-damage short-range assault with fuel and exposure risk (E-R). **Secondary:** clear garrisons; deny streets. **Doctrine fit:** explicit, counterable high-risk aggression (E). Abstract game weapon only (E-R). |
| **Inspiration and background** | Two-person team with portable flame projectors in protective suits. C&C lineage (E-R): Flamethrower (Ren); Black Hand (TW); Flamethrower Infantry (TD). |
| **Visual identity** | Concept: [`IA-INF-05.png`](../../Factions/TheIronAscendancy/Units/IA-INF-05.png). Bulky backpack tanks with crimson hazard bands; masked faces. |
| **Statistics** | 2 × 90 HP = **180** (`armor.inf_protected`) · FOOT_HEAVY **190 cm/s** · F0 · sight 1100 cm · 1 slot · food staff 2 · fuel none (projector fuel is ammunition) · ExperienceValue 48 |
| **Commands** | as TPL-INF-HW (no deploy) + Fire Mission (ground ignition, P) · default stance Aggressive · formation role `frontline` |
| **Production** | **$480** · `IA-BLD-03` · prerequisites: `IA-BLD-14` operational · **11 s** · 2 food · no cap |
| **Veterancy** | Standard |
| **AI** | `ai.unit.line_infantry` with weights 1.5 / 0.3 / 0 / 0 / 1.2. Deltas: advance behind IA-INF-02 or a tank; never lead across open ground; withdraw when AmmoCapacity reaches 0 |
| **Multiplayer** | MPA-STD. Burning status and ground fire are server state; flame stream VFX is cosmetic |
| **Presentation** | ANIM-INF + stream fire loop, tank-rupture death. Roar and hiss audio; icon of a flame on a crimson shield |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.ia.inf_05.primary` flame projectors | INCENDIARY | 0–350 cm | 20 per member / 0.5 s (80 DPS base); applies `burning` 4 s (5 per second, stacks once); AmmoCapacity 30 bursts | stream; 150 cm wide along the line | 10 | GROUND, STRUCTURE | 160 % / 25 % / 5 % (never auto) / — / 80 % (garrisoned infantry 200 %) |

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.ia.inf_05.fire_barrier` | Active (P) | Ignites a 300 cm ground patch for 10 s: INCENDIARY 10 per second to units inside, blocks infantry pathing preference (not hard blocking) | Ground point ≤ 350 cm | 5 ammunition · 10 s · 30 s · 350 cm | Vehicles cross it; wait it out | Fire patch VFX | Use across a street when the enemy assaults a held building |
| `ability.ia.inf_05.tank_rupture` | Passive, **risk** (P; E-R: exposure risk) | On death, each member's tank bursts: INCENDIARY 40 in 200 cm. **Friendly units take full damage** | Automatic on death | — | Kill it near its own allies; marksmen at range | Distinct rupture VFX | Keep 250 cm spacing from friendly infantry |

**Research and upgrades:** `tech.shared.infantry_protective_equipment` (90 → 105). `tech.ia.pressurised_projectors` (IA-BLD-14, $650): range 350 → 420 cm; trade-off: AmmoCapacity 30 → 24.

**Counters:** *Beats (hard):* garrisons and infantry in cover (AC-INF-01/02, RD-INF-01/07, MP-INF-10), bunkers' infantry occupants. *Beaten by (hard):* any armored vehicle, marksmen (AC-INF-08, MP-INF-11), HMG at range (RD-INF-03), artillery (death bursts punish clumps). *Situational:* fortified-town assaults; punishes Directorate mass blocks.

**Status:** identity E-R; abstract weapon E-R; all values and abilities P.

### IA-INF-06 — Frontier Combat Medic

| Field | Specification |
| --- | --- |
| **Identity** | `unit.ia.inf_06` · Iron Ascendancy · Infantry / medic · `TPL-INF-SUPPORT` · **Core** · Playable |
| **Role** | **Primary:** sustains elite groups; medical service grants no immunity (E-R). **Secondary:** replenishment support for expensive specialists. **Doctrine fit:** heavy losses are costly, so preservation matters (E: *Gameplay*). |
| **Inspiration and background** | Two-person field medical team with stretchers and trauma packs. C&C lineage (E-R): Field Medic (TS); Engineer (TT, repair role). |
| **Visual identity** | Concept: [`IA-INF-06.png`](../../Factions/TheIronAscendancy/Units/IA-INF-06.png). White armband with a black border on charcoal uniforms; medical satchels. |
| **Statistics** | 2 × 55 HP = **110** (`armor.inf_light`) · FOOT **215 cm/s** · F0 · sight 1000 cm · 1 slot · food staff 2 · fuel none · no ranks (non-combat) |
| **Commands** | Move, Stop, Guard (I) · Heal (P) · default stance Hold Fire · formation role `protected` |
| **Production** | **$380** · `IA-BLD-03` · prerequisites: `IA-BLD-03` only · **10 s** · 2 food · no cap |
| **Veterancy** | none (non-combat) |
| **AI** | `ai.unit.support_infantry`. Deltas: heal INF_PROTECTED and specialist squads before line squads (higher replacement value) |
| **Multiplayer** | MPA-STD |
| **Presentation** | ANIM-INF + kneel-and-treat loop. Heal tick numbers for the owner; icon of a cross in a black circle |

**Weapons:** sidearms only, `weapon.ia.inf_06.sidearm` SMALL_ARMS 0–450 cm, 6 per member / 1.0 s, used only in self-defense (never auto-acquires under Hold Fire).

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.ia.inf_06.heal` | Active / automated under Guard (P) | 6 HP/s to one squad, capped at living members × member HP (§10.3) | Friendly infantry ≤ 300 cm | free · channel · — · 300 cm | Kill the medic first; artillery on the patient | Green tick; channel beam | Lowest HP ratio first, protected and specialist squads first |
| `ability.ia.inf_06.combat_stimulant` | Upgrade-unlocked active, **with cost** (P) | Target squad +15 % speed and immune to the Shaken tier for 15 s; afterwards −15 resolve and 5 % max HP lost | Friendly infantry ≤ 300 cm, not commanders | requires `tech.ia.combat_pharmacology` · 15 s · 40 s · 300 cm | Wait out the window and hit the squad in the crash | Pulse icon on the target | Use to push IA-INF-02 through the last 400 cm to a breach |

**Research and upgrades:** `tech.shared.field_medical_procedures`: heal 6 → 8 HP/s. `tech.ia.combat_pharmacology` (IA-BLD-14, $700; abstract game effect, no pharmacology modelled): unlocks *Combat Stimulant*; trade-off: medic price +$60.

**Counters:** *Beats:* nothing (support). *Beaten by (hard):* marksmen (AC-INF-08, MP-INF-11), raiders and any armed unit that reaches it. *Situational:* doubles the life of IA-INF-02 and IA-INF-05 in long urban fights.

**Status:** identity E-R; no immunity E-R; all values P.

### IA-INF-07 — Technical Demolition Engineer

| Field | Specification |
| --- | --- |
| **Identity** | `unit.ia.inf_07` · Iron Ascendancy · Infantry / engineer · `TPL-INF-SUPPORT` · **Core** · Playable |
| **Role** | **Primary:** repair, wire-cutting and obstacle work (E-R). **Secondary:** structure capture; build field cover. **It cannot use early conventional sabotage charges (E-R)** and has no Plant Charge ability (E: spy-only rule). **Doctrine fit:** sophisticated military engineering supports concentrated thrusts (E: *Military Establishment*). |
| **Inspiration and background** | Two-person combat-engineer team with wire cutters, demolition tools for obstacles and a toolkit. C&C lineage (E-R): Combat Engineer (RA3); Saboteur (TW, engineer role); Technician (Ren). |
| **Visual identity** | Concept: [`IA-INF-07.png`](../../Factions/TheIronAscendancy/Units/IA-INF-07.png). Tool belts, wire cutters, engineer gorget with a crimson stripe. |
| **Statistics** | 2 × 65 HP = **130** (`armor.inf_light`) · FOOT **205 cm/s** · F0 · sight 1000 cm · 1 slot · food staff 2 · fuel none · ExperienceValue 44 |
| **Commands** | Move, Stop, Guard, Attack (I) · Capture, Repair, Clear, Build Cover (P) · default stance Hold Fire · formation role `protected` |
| **Production** | **$440** · `IA-BLD-03` · prerequisites: `IA-BLD-03` only · **12 s** · 2 food · no cap |
| **Veterancy** | Standard (armed) |
| **AI** | `ai.unit.support_infantry`. Deltas: repair damaged friendly vehicles ≥ $900 first; clear obstacles on the group's path; capture only on explicit order |
| **Multiplayer** | MPA-STD. Capture progress is server state and visible to the structure's owner when observed |
| **Presentation** | ANIM-INF + work loop, wire-cut, capture. Tool clank audio; icon of crossed wrench and cutter |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.ia.inf_07.primary` carbines | SMALL_ARMS | 0–550 cm | 8 per member / 1.0 s | hitscan | 2 | GROUND | 100 % / 15 % / 2 % / — / 8 % |

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.shared.capture_structure` | Active (P) | 8 s uninterrupted channel captures an eligible structure (§10.6); Unaligned structures only under scenario and jurisdiction rules (E) | Eligible structure ≤ 100 cm | free · 8 s · — · 100 cm | Any damage interrupts; garrisons | Capture bar visible to the owner of the structure when observed | Only on explicit order |
| `ability.shared.field_repair` | Active / automated under Guard (P) | 8 HP/s to a vehicle, USD per §10.2 (+25 % for Ascendancy heavy hulls) | Friendly vehicle ≤ 300 cm | USD proportional · channel · — · 300 cm | Kill the engineers | Spark VFX | Highest-value vehicle first |
| `ability.ia.inf_07.clear_obstacle` | Active (P) | Removes wire, barricade, mine or road barrier segment in 6 s; Unaligned barriers trigger an R1 jurisdiction incident (E: crossing is not authorization) | Obstacle or mine ≤ 150 cm | free · 6 s · — · 150 cm | Covering fire on the obstacle | Cut and lift animation | Clear the lane on the group's ordered path |
| `ability.ia.inf_07.build_cover` | Active (P) | Builds a 3 m sandbag line giving light cover (−25 %, §6.9) for 120 s | Ground point ≤ 200 cm | $60 · 8 s build · 30 s · 200 cm | HE and engineers can remove it; requires map cover support (U-08) | Sandbag mesh | Under Guard when the group holds a position |

**Research and upgrades:** `tech.shared.vehicle_component_standardization`: field-repair price −20 %. `tech.ia.sapper_training` (IA-BLD-04, $500): *Clear Obstacle* 6 → 4 s; trade-off: price +$40.

**Counters:** *Beats:* obstacles, mines (MP-ARM-11 fields), capturable sites. *Beaten by (hard):* any armed unit, especially HMG and marksmen. *Situational:* keeps a heavy column moving; never a sabotage substitute (E).

**Status:** identity E-R; no-charge rule E/E-R; capture concept E-R; all values P.
### IA-INF-08 — Covert Reconnaissance Team

| Field | Specification |
| --- | --- |
| **Identity** | `unit.ia.inf_08` · Iron Ascendancy · Infantry / recon and marksman · `TPL-INF-SPEC` · **Extended** · Playable |
| **Role** | **Primary:** observe traffic routes, disguise movement and report locations (E-R). **Secondary:** precision fire on high-value infantry; spotting for IA-ARM-10/11. It performs no espionage: no theft, no charges and no nuclear intelligence (E: scouting is not espionage). **Doctrine fit:** specialized reconnaissance formations identify weaknesses (E). |
| **Inspiration and background** | Two-person long-range reconnaissance team with a suppressed precision rifle, binoculars and a burst radio. C&C lineage (E-R): Shadow Team (TW); Shinobi (RA3); Chameleon Spy (TS, campaign). |
| **Visual identity** | Concept: [`IA-INF-08.png`](../../Factions/TheIronAscendancy/Units/IA-INF-08.png). Mottled grey-brown smocks over the charcoal kit, rifle wrapped in scrim. |
| **Statistics** | 2 × 65 HP = **130** (`armor.inf_light`) · FOOT **210 cm/s** · F0 · sight **1600 cm** · `DETECT.CONCEALED` 900 cm · 1 slot · food staff 2 · ExperienceValue 64 |
| **Commands** | as TPL-INF-SPEC + Disguise (route movement), Spot (P) · default stance Hold Fire while Concealed · formation role `scout_screen` |
| **Production** | **$640** (≥ RD-INF-09 $420 × 1.5 = $630, framework §4.4 validator) · `IA-BLD-04` Special Operations School · prerequisites: `IA-BLD-04` operational · **13 s** · 2 food · no cap |
| **Veterancy** | Standard; Elite adds scatter-free spotting |
| **AI** | `ai.unit.specialist_infantry`. Deltas: hold a road-observation point; shoot only officers, AT teams, HW teams and medics; relocate after 2 shots |
| **Multiplayer** | MPA-STD + MPA-COVERT. The *Traveller Cover* disguise archetype replaces the true archetype for opponents (§9.1) |
| **Presentation** | ANIM-INF + prone, crawl, scope aim. Suppressed shot audio; icon of an eye over crosshairs |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.ia.inf_08.primary` suppressed precision rifle | PRECISION | 0–1100 cm | 60 / 3.0 s (one shooter) | hitscan, tracking medium | 8 on kill | GROUND | 160 % / 2 % / 0 % / — / 0 % |

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.shared.conceal` | Conditional (P) | Concealed after 3 s stationary | Self | free | `DETECT.CONCEALED`; firing reveals 4 s | Own-only icon | Default |
| `ability.ia.inf_08.traveller_cover` | Active (P) | Opponents see the team as a civilian pair (UN-CIV pedestrian archetype) while it moves at ≤ 60 % speed and does not fire; it cannot enter enemy structures | Self | free · until broken · 30 s · self | `DETECT.SPY` (AC-INF-10, UN-INF-10/11, IA counterparts do not apply); any hostile action; Unaligned police checks (R1) | Disguise icon (owner only) | Use to cross observed roads, never inside enemy bases |
| `ability.ia.inf_08.spot` | Active (P) | Designates an observed point for 20 s: friendly indirect fire there uses observed scatter ×0.7 and the point stays observed | Ground point or unit in sight | free · 20 s · 25 s · sight | Kill or suppress the team (Pinned cancels) | Owner laser-dot marker | Spot for IA-ARM-10/11 on clusters |

**Research and upgrades:** `tech.ia.advanced_clandestine_training` (E: catalogue; also espionage tier 1): *Traveller Cover* speed limit 60 → 75 %. `tech.shared.tactical_radio_modernization`: *Spot* duration 20 → 30 s.

**Counters:** *Beats (hard):* officers, AT and HW teams, medics (RD-INF-10, AC-INF-03, RD-INF-03, MP-INF-06), commanders' escorts. *Beaten by (hard):* recon infantry with detection (RD-INF-09, AC-INF-08), scout vehicles, any vehicle (it cannot hurt them), artillery once revealed. *Situational:* the eyes of the artillery-led breakthrough.

**Status:** identity E-R; no-espionage rule E; disguise concept E-R; all values P.

### IA-INF-09 — Ascendancy Intelligence Agent

| Field | Specification |
| --- | --- |
| **Identity** | `unit.ia.inf_09` · Iron Ascendancy · Infantry / spy · `TPL-INF-COVERT` · **Conditional** · Playable |
| **Role** | **Primary:** limited starting spy tree; later espionage upgrades and **spy-only expensive explosive sabotage** (E-R, E). **Secondary:** the **only** unit that can perform the nuclear-research infiltration, and only at `tech.ia.espionage_tier_3` against a qualifying Concord or Directorate facility (E). **Doctrine fit:** limited espionage that must be fully developed before its strategic payoff (E). |
| **Inspiration and background** | Civilian-dressed intelligence officer with forged papers and a document camera. C&C lineage (E-R): Spy (RA2); Black Lotus (Gen); Shadow Team (TW). |
| **Visual identity** | Concept: [`IA-INF-09.png`](../../Factions/TheIronAscendancy/Units/IA-INF-09.png). Overcoat and hat in the owner's view; the disguise archetype for opponents. |
| **Statistics** | 1 × 110 HP = **110** (`armor.inf_light`) · FOOT **220 cm/s** · F0 · sight 1200 cm · 1 slot · food staff 1 · no ranks (non-combat) |
| **Commands** | Move, Stop (I) · Disguise, Infiltrate, Plant Charge, Withdraw (P) · default stance Hold Fire · formation role `protected` |
| **Production** | **$1,200** · `IA-BLD-04` · prerequisites: `tech.ia.espionage_tier_1` · **20 s** · 1 food · **cap 3** (limited spy tree) |
| **Veterancy** | none |
| **AI** | `ai.unit.covert_agent`. Deltas: never attempt nuclear infiltration against a site known only from a `Reported` object (E: false intelligence); abort on any `DETECT.SPY` source within 1200 cm |
| **Multiplayer** | MPA-COVERT + MPA-STRATEGIC (nuclear intelligence flags). Infiltration outcome and both intelligence flags are server-authoritative, idempotent and keyed to a real facility ID; a planted Unaligned report never satisfies them (E) |
| **Presentation** | ANIM-INF + civilian walk, document photograph loop. Disguise uses the full disguise presentation profile (mesh, animation and voice) so nothing leaks (§15). Icon of an envelope with an eagle seal |

**Weapons:** none (unarmed). Charges are an ability, not a weapon.

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.shared.disguise` | Active (P) | Appears to opponents as a civilian or an enemy infantry archetype (§9.1) | Self | free · until broken · 20 s · self | `DETECT.SPY` (AC-INF-10, RD-INF-07, UN-INF-10/11, UN-INF-04), hostile action | Owner-only disguise frame | Always before entering enemy sight |
| `ability.ia.inf_09.infiltrate_intel` | Active (P; E: limited) | 15 s channel inside an eligible enemy structure: reveals its production queue and power state for 90 s; with tier 2 also steals one research-progress token | Enemy producer or research building | free · 15 s (25 % slower than Concord/Directorate theft) · 60 s · contact | `DETECT.SPY`; guards; Unaligned counterintelligence | Owner progress bar; victim sees nothing unless detected | Only on producers with no detector in range |
| `ability.ia.inf_09.nuclear_infiltration` | Conditional active (E gate; P values) | Infiltrates a **real** Concord or Directorate nuclear research facility: sets **refining** and/or **weapons** intelligence flags. Both are required before IA-BLD-15 unlocks (E) | Requires `tech.ia.espionage_tier_3` (E: full progression); target AC/RD nuclear research facility | free · 30 s channel · once per flag per facility · contact; the agent is consumed on success | Detection, guards, `DETECT.SPY`; false sites (E) yield nothing | Strategic event to the owner only; victim gets a discovery event only when detected | Only after tier 3 and only against a site the team has **observed**, never a report |
| `ability.shared.plant_charge` | Active (P; E permission) | Fictional attached charge: after a 15 s fuse, DEMOLITION 450 to an eligible vehicle or building | Requires `tech.ia.late_espionage`; stationary target ≤ 100 cm | **$300 per charge** (above Meridian, E) · 15 s fuse · 45 s · contact; 2 carried | `DETECT.SPY` reveals agent and charge; *Investigate* removes it | Owner fuse timer | Only on stationary high-value targets with no detector within 1200 cm |

**Research and upgrades:** `tech.ia.espionage_tier_1` (= `tech.ia.advanced_clandestine_training`, IA-BLD-04, $800, 60 s): unlocks the agent. `tech.ia.espionage_tier_2` (IA-BLD-14, $1,400, 120 s): research-token theft; unlocks IA-INF-10. `tech.ia.espionage_tier_3` (IA-BLD-14, $2,200, 180 s): nuclear infiltration permission (E: only the full tier qualifies). `tech.ia.late_espionage` (IA-BLD-04, $1,200, 90 s; maps to I `late_espionage_unlocked`): *Plant Charge*. Intimidation and research never substitute for any step (E).

**Counters:** *Beats:* unguarded producers and research sites. *Beaten by (hard):* every `DETECT.SPY` unit, above all the Unaligned (UN-INF-11, strongest counterintelligence, E). *Situational:* the gate to the Ascendancy production victory; expensive to lose because the cap and tier investment are scarce.

**Status:** identity E-R; spy-only charges, limited espionage, tier-3 nuclear gate, both-flag rule, report-never-counts E; I predicate `CanPlantExplosives` and `CanRefineNuclear` (Ascendancy only with both flags) I; values P.

### IA-INF-10 — Facility Contamination Operative

| Field | Specification |
| --- | --- |
| **Identity** | `unit.ia.inf_10` · Iron Ascendancy · Infantry / covert facility saboteur · `TPL-INF-COVERT` · **Conditional** · Playable |
| **Role** | **Primary:** exclusive abstract contamination status on eligible staffed enemy buildings; bounded disruption and recovery (E-R, E). **Secondary:** none; it is not a combat unit. **Doctrine fit:** drains enemy productive capacity rather than stealing funds or converting units (E: faction description). |
| **Inspiration and background** | Covert operative with forged facility passes and service-worker cover. No real agent, procedure or exposure mechanism is modelled (E). C&C lineage (E-R): Chemical Warrior (TD, fictional effect only); Toxin Tractor (Gen, area-denial analogue); Desolator Trooper (Uprising, role adapted). |
| **Visual identity** | Concept: [`IA-INF-10.png`](../../Factions/TheIronAscendancy/Units/IA-INF-10.png). Production must present a plain maintenance-worker cover; no laboratory or device imagery. |
| **Statistics** | 1 × 100 HP = **100** (`armor.inf_light`) · FOOT **215 cm/s** · F0 · sight 1100 cm · 1 slot · food staff 1 · no ranks |
| **Commands** | Move, Stop (I) · Disguise, Contaminate, Withdraw (P) · default stance Hold Fire · formation role `protected` |
| **Production** | **$1,400** · `IA-BLD-04` · prerequisites: `tech.ia.espionage_tier_2` and `IA-DEF-12` operational (E-R: authorized cell) · **22 s** · 1 food · **cap 2** |
| **Veterancy** | none |
| **AI** | `ai.unit.covert_agent`. Deltas: target enemy producers with the longest queues; never Unaligned civilian facilities unless a scenario rule makes them eligible |
| **Multiplayer** | MPA-COVERT + MPA-ECON. Operation success and attribution are **separate** server states (E). Staff losses and throughput loss are visible to the victim at once; the responsible operative is not (E) |
| **Presentation** | ANIM-INF + civilian walk, service-door entry. The contamination status uses a neutral hazard-placard icon and staffing counters, never a depiction of agents. Icon of a cracked placard |

**Weapons:** none (unarmed).

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.shared.disguise` | Active (P) | as IA-INF-09 | Self | free | `DETECT.SPY` | Owner-only | Always |
| `ability.ia.inf_10.contaminate_facility` | Active (P; E effect) | 12 s channel at an eligible **staffed** enemy building: applies `status.contaminated` for 120 s. Each 30 s upkeep step removes 15 % of assigned staff (minimum 1); throughput scales with remaining staff (production stops below 40 %). **Chain:** at each step, one other eligible staffed enemy building within 1500 cm may be infected; chain depth ≤ 2, at most 3 buildings per operation (E: finite, bounded) | Eligible enemy staffed building; not nuclear facilities for intelligence purposes (E: grants no nuclear intel) | $400 per operation · 120 s status · 90 s · contact | Containment: RD-INF-08 *Decontaminate*-class actions, facility shutdown (stops spread, also production), security detection, worker replacement at USD cost; status expires | Victim: placard icon, staff counter, throughput bar, chain warning on neighbours; attribution only on detection | Only when no `DETECT.SPY` source within 1200 cm of the target |

**Research and upgrades:** `tech.ia.espionage_tier_2` unlocks the unit. `tech.ia.contamination_doctrine` (IA-BLD-14, $1,000): chain cap 3 → 4 buildings; trade-off: each operation raises Unaligned hostility toward the Ascendancy by one incident level (R1 → R2) when any affected building lies in Unaligned jurisdiction.

**Counters:** *Beats:* long production queues and research racing (it stalls producers). *Beaten by (hard):* `DETECT.SPY` units (AC-INF-10, RD-INF-07, UN-INF-10/11), decontamination and recovery (RD-INF-08). *Situational:* a counter to Directorate industrial mass and Concord research leads; never a unit killer, never an ownership change (E).

**Status:** identity E-R; abstract status, staffing loss, bounded chain, containment and attribution separation E; all numbers P.

### IA-INF-11 — Intimidation Officer

| Field | Specification |
| --- | --- |
| **Identity** | `unit.ia.inf_11` · Iron Ascendancy · Infantry / coercion officer · `TPL-INF-COMMAND` (no talents) · **Core** · Playable |
| **Role** | **Primary:** exclusive coercion of *isolated* enemies only after a total Discipline/Morale break; no generic mind control (E-R, E). **Secondary:** bounded stress pressure on isolated targets. **Doctrine fit:** the regime's coercive apparatus applied on the battlefield, with visible consequences (E). |
| **Inspiration and background** | Political officer in a leather greatcoat with a loudhailer and an escort. C&C lineage (E-R): Yuri Clone (YR, interaction analogue); Cultists (KW, interaction analogue); High Confessor (TT, morale role). |
| **Visual identity** | Concept: [`IA-INF-11.png`](../../Factions/TheIronAscendancy/Units/IA-INF-11.png). Long coat, peaked cap with a crimson band, loudhailer; cosmetic escort pair. |
| **Statistics** | 1 × 160 HP = **160** (`armor.inf_protected`) · FOOT **205 cm/s** · F0 · sight 1300 cm · 1 slot · food staff 3 (with escort) · ExperienceValue 120 · not a talent commander (§17) |
| **Commands** | Move, Stop, Guard (I) · Intimidate, Withdraw (P) · default stance Hold Fire · formation role `protected` |
| **Production** | **$1,200** · `IA-BLD-03` · prerequisites: `IA-BLD-17` State Broadcast Commissariat operational · **18 s** · 3 food · **cap 2** |
| **Veterancy** | none (no ranks; specialist officer) |
| **AI** | `ai.unit.officer`. Deltas: only approach a target that is **observed**, eligible, isolated (no other combat unit of its owner within 800 cm) and at Pinned or lower; withdraw at 50 % HP |
| **Multiplayer** | MPA-STD + MPA-COMMAND (ownership transfer). Conversion is a **separate validated server event** under the E predicate; the client only submits the intent. Isolation is computed from authoritative positions, never from selection (E) |
| **Presentation** | ANIM-INF + loudhailer and pointing gesture. Distorted loudhailer audio; the target shows a "coercion" ring to both sides while the channel runs (counterplay readability). Icon of a loudhailer |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.ia.inf_11.sidearm` pistol | SMALL_ARMS | 0–450 cm | 12 / 1.2 s | hitscan | 2 | GROUND | 100 % / 15 % / 2 % / — / 8 % |

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.ia.inf_11.intimidate` | Active channel (E predicate; P values) | While the target is **eligible** (§7.5) and **isolated**, reduces its `UnitResolve` by 8 per second (label: Discipline for combatants, Morale for non-combatants; one meter, E). If `UnitResolve` reaches `BrokenThreshold` (0) while isolation still holds, a validated conversion transfers ownership of **that same unit** to the Ascendancy, with HP, rank and passengers kept and orders cleared | Observed eligible hostile unit ≤ 600 cm; never elite commanders, medium/heavy armor, airborne aircraft, scenario-protected units; neutral Unaligned only with explicit eligibility (E) | free · channel until break or interruption (max 15 s) · 60 s after a conversion, 20 s after an interrupted channel · 600 cm | Regroup (a second friendly combat unit within 800 cm ends isolation), interrupt by damaging the officer (any hit ≥ 10 % max HP), withdraw out of range, resolve recovery; RD-INF-10 *Rally* and AC/RD officers | Coercion ring on the target for both sides; meter shown to both; conversion banner | Only when the predicate is already met except the meter, and no enemy reinforcements are observed within 1200 cm |
| `ability.ia.inf_11.coercive_presence` | Passive (P) | Isolated eligible enemies within 600 cm take +25 % stress from all sources | Aura 600 cm, enemies only | free | Same as above; ending isolation | Faint red aura visible when observed | — |

**Research and upgrades:** `tech.ia.coercion_methods` (IA-BLD-17, $900): channel range 600 → 750 cm; trade-off: each conversion of a unit inside Unaligned jurisdiction raises the incident level by one. Research never relaxes the predicate (E).

**Counters:** *Beats (hard):* isolated scouts, lone engineers, stranded transports and broken squads (RD-ARM-01, MP-ARM-01, AC-INF-06 alone). *Beaten by (hard):* grouped forces (no isolation), marksmen (AC-INF-08, MP-INF-11), any direct fire during the channel. *Situational:* punishes raiding doctrines (Meridian) and over-extended Concord detachments that are otherwise exempt from the isolation penalty (E: Concord is not immune to Intimidation once broken and isolated).

**Status:** identity E-R; exclusivity, predicate, one-meter rule, same-unit transfer and counterplay list E; eligibility classes, rates and ranges P.

### IA-INF-12 — Political Security Enforcer

| Field | Specification |
| --- | --- |
| **Identity** | `unit.ia.inf_12` · Iron Ascendancy · Infantry / security and counterintelligence · `TPL-INF-LINE` · **Extended** · Playable |
| **Role** | **Primary:** detain, police and enforce harsh zone control; attracts Unaligned hostility (E-R). **Secondary:** the Ascendancy's only `DETECT.SPY` infantry; rear-area security against agents and saboteurs. **Doctrine fit:** internal security institutions and occupation administration (E: *Occupation and Expansion*). |
| **Inspiration and background** | Party security detachment with batons, sidearms and carbines. C&C lineage (E-R): Confessor Cabal (KW); Black Hand (KW); Reaper (TT, role). |
| **Visual identity** | Concept: [`IA-INF-12.png`](../../Factions/TheIronAscendancy/Units/IA-INF-12.png). Black uniforms, crimson armbands, white-lettered security helmets. |
| **Statistics** | 3 × 75 HP = **225** (`armor.inf_protected`) · FOOT 205 cm/s · F0 · sight 1300 cm · **`DETECT.SPY` 700 cm** · 1 slot · food staff 3 · ExperienceValue 40 |
| **Commands** | as TPL-INF-LINE + Detain, Checkpoint (P) · default stance Guard · formation role `rear_guard` |
| **Production** | **$400** · `IA-BLD-03` · prerequisites: `IA-DEF-11` or `IA-BLD-02` operational · **10 s** · 3 food · no cap |
| **Veterancy** | Standard |
| **AI** | `ai.unit.line_infantry` + covert screening. Deltas: patrol producers and research buildings; detain revealed agents first |
| **Multiplayer** | MPA-STD. Detection reveal is server-resolved per the §9.1 contract; the reveal goes only to the detector's team |
| **Presentation** | ANIM-INF + detain (restrain) loop. Whistle and shouted commands; icon of a baton across a shield |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.ia.inf_12.primary` carbines | SMALL_ARMS | 0–600 cm | 9 per member / 1.0 s | hitscan | 3 | GROUND | 100 % / 15 % / 2 % / — / 8 % |

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.ia.inf_12.detain` | Active (P) | 5 s channel: removes a **revealed** enemy covert agent or saboteur from play; its owner loses the unit with no refund. Against UN units this is an R2 incident (E: detention triggers diplomatic costs) | Revealed hostile `TPL-INF-COVERT` unit ≤ 150 cm | free · 5 s · 20 s · 150 cm | Escorts; the agent's own withdrawal; damage interrupts | Detain animation; capture banner to both sides | Detain any revealed agent within 600 cm |
| `ability.ia.inf_12.checkpoint` | Active (P) | Deploys for 60 s: `DETECT.SPY` 700 → 1000 cm and enemy covert units passing within 300 cm are revealed for 10 s | Self | free · 60 s · 30 s · self | Avoid the checkpoint; kill it | Barrier props | At chokepoints into the base |

**Research and upgrades:** `tech.ia.internal_security_apparatus` (IA-BLD-17, $600): `DETECT.SPY` 700 → 850 cm; trade-off: Unaligned hostility +1 incident level whenever an enforcer detains anyone inside Unaligned jurisdiction.

**Counters:** *Beats (hard):* agents and saboteurs (AC-INF-11, RD-INF-11/12, MP-INF-08/09). *Beats (soft):* light infantry raids. *Beaten by (hard):* HMG, flame, artillery, any armor. *Situational:* protects IA-BLD-15/16 against theft and sabotage in long games.

**Status:** identity E-R; Unaligned hostility and diplomatic cost E-R; all values P.

### IA-INF-13 — Special Operations Raider

| Field | Specification |
| --- | --- |
| **Identity** | `unit.ia.inf_13` · Iron Ascendancy · Infantry / **Elite commander candidate** (E: talent doc §5, explicit approval required; U-14) · `TPL-INF-COMMAND` · **Extended** · Playable · **cap 1** |
| **Role** | **Primary:** limited elite assault and target marking (E-R). **Secondary:** if approved as commander, the Ascendancy talent appointment: assault tempo, suppression, isolation and offensive concentration (E: talent identity). It **cannot bypass the spy-only charge rule** (E-R) and cannot Intimidate. **Doctrine fit:** elite assault units strike command facilities (E). |
| **Inspiration and background** | Special operations officer leading a small strike element; assault rifle, marker beacons and a radio. C&C lineage (E-R): Cyborg Commando (TS, combat role); Commando (TW); Raveshaw (Ren). |
| **Visual identity** | Concept: [`IA-INF-13.png`](../../Factions/TheIronAscendancy/Units/IA-INF-13.png). **Production risk:** the sheet leans toward stylized fantasy armor; the production model must be reconciled to plausible 1980s kit (roster rule 7): helmet, plate carrier, respirator, no powered armor. |
| **Statistics** | 1 × 520 HP = **520** (`armor.inf_protected`) · FOOT **225 cm/s** · F0 · sight **1500 cm** · `DETECT.CONCEALED` 700 cm · 1 slot · food staff 2 · no ordinary veterancy if approved as commander (E); otherwise standard veterancy (U-14 fallback) |
| **Commands** | as TPL-INF-COMMAND + Mark Target + talent abilities (if approved) · default stance Guard · formation role `formation_anchor` |
| **Production** | **$2,300** (elite band upper) · `IA-BLD-04` · prerequisites: `IA-BLD-14` operational · **32 s** · 2 food · **cap 1** (E if approved: one appointment). Redeployment: 90 s after death at full price, resuming the stored appointment (E: talent doc §4.4) |
| **Veterancy** | none if commander (E); fallback standard |
| **Command radius** | **1100 cm** baseline: the smallest of the four, reflecting a front-line assault leader |
| **Leadership effects** | Passive *Spearhead*: own combat units in radius that are moving under AttackMove gain +10 % speed (cap with other speed sources per §18.4) |
| **Vulnerabilities** | Leads from the front, so it is exposed to marksmen, flame and artillery; jamming cuts its links; Psy-Ops and Intimidation ineligible (§7.5); death clears all live effects (E) |
| **AI** | `ai.unit.commander` with retreat at 40 % HP (assault leader). Deltas: stay 300–500 cm behind IA-INF-02 or IA-ARM-03 groups; pick Tier I by army composition (≥ 6 IA-INF-02/05 → Shock Leadership) |
| **Multiplayer** | MPA-STD + **MPA-COMMAND** |
| **Presentation** | ANIM-INF + command gestures, beacon throw. Aura ring for owner and allies; pennant only for enemies. Icon of a dagger over a chevron |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.ia.inf_13.primary` assault rifle | SMALL_ARMS | 0–750 cm | 28 / 0.8 s (35 DPS base) | hitscan | 4 | GROUND, AIR_LOW | 100 % / 15 % / 2 % / 25 % rotary / 8 % |
| `ability.shared.grenade` (I) | HIGH_EXPLOSIVE | 650 cm | 50, 150 cm | lobbed | 8 | GROUND, STRUCTURE | as IA-INF-01 |

**Abilities and proposed talent set** (three options per tier; one per tier; E: tier structure and sample path; P: effects; awaiting explicit commander approval, U-14)

| Tier | Option A | Option B | Option C |
| --- | --- | --- | --- |
| Unit ability (always) | **Mark Target** (`ability.ia.inf_13.mark_target`): one observed vehicle or structure is marked for 12 s; friendly direct fire on it +15 % damage; cooldown 35 s; 1500 cm. It is not a charge (E) | — | — |
| I Initial Doctrine | **Shock Leadership** (E sample): IA-INF-02/03/05 in radius get −15 % cooldown within 400 cm of their target | **Armored Escort:** IA-ARM-03/04 in radius turn turrets +25 % faster | **Recon Coordination:** IA-INF-08 *Spot* in radius lasts +10 s |
| II Field Specialization | **Suppression Doctrine** (E sample): stress dealt by units in radius +20 % (bounded: never alone pushes a unit from Steady to Broken) | **Breach Tempo:** *Breach and Clear* channel −1 s in radius | **Counter-Jamming:** links resist jamming unless the jammer is within 600 cm |
| III Advanced Command | **Isolation Tactics** (E sample): enemy units that are isolated and within radius are revealed to all own units for 10 s on a 60 s cooldown; it never meets the Intimidation predicate by itself | **Hold the Gains:** captured or breached positions in radius give occupants +20 % HP regeneration for 30 s | **Fire Discipline:** IA artillery firing at points marked by the raider uses scatter ×0.6 |
| IV Signature Operation | **Concentrated Breakthrough** (E sample): 20 s, units in radius under AttackMove get +15 % damage and +10 % speed; afterwards they take +15 % damage for 15 s (exposure trade-off, E); cooldown 180 s | **Last Stand:** 15 s, units in radius cannot drop below 1 HP from non-artillery damage; afterwards each loses 30 % of current HP (**self-endangering**, E); cooldown 240 s | **Decapitation Raid:** for 30 s the raider and 2 adjacent squads are Concealed while moving at ≤ 75 % speed and deal +25 % damage to commanders and officers; cooldown 200 s |

Every talent follows the commander influence invariant (talent doc §3.3) and the protected canon: no bypass of spy progression, spy-only charges, the full-break Intimidation condition or the nuclear gates (E).

**Research and upgrades:** research never grants talents (E). `tech.shared.tactical_radio_modernization`: radius +200 cm (clamp 1600 cm).

**Counters:** *Beats (soft):* infantry in the open; amplifies assault groups. *Beaten by (hard):* marksmen (AC-INF-08, MP-INF-11), artillery, flame (RD-ARM-10), jammers (AC-ARM-10, RD-SUP-06). *Situational:* at its best with IA-INF-02, IA-INF-05 and IA-ARM-03 in a single concentrated thrust.

**Status:** identity E-R; talent candidacy, required explicit approval, cap 1 (if approved), OXP and death rules E; price, radius, *Mark Target* and all talent effects P; commander status **U-14**.

### IA-INF-14 — Civilian Disguise Courier

| Field | Specification |
| --- | --- |
| **Identity** | `unit.ia.inf_14` · Iron Ascendancy · Infantry / covert courier · `TPL-INF-COVERT` · **Conditional** · Playable |
| **Role** | **Primary:** clandestine supply and intelligence movement with detection and jurisdiction risks (E-R). **Secondary:** carries intelligence tokens from IA-INF-09 back to IA-BLD-04 so they can be banked; carries a small ammunition parcel. **Doctrine fit:** covert movement through a hostile federation that resists occupation (E). |
| **Inspiration and background** | Civilian-dressed courier on foot or bicycle with a parcel. C&C lineage (E-R): Sudden Transport (RA3); Stealth Black Hand (Ren); Spy (RA1). |
| **Visual identity** | none (asset requirement): civilian clothes, satchel, bicycle variant for road movement. |
| **Statistics** | 1 × 80 HP = **80** (`armor.inf_light`) · FOOT **230 cm/s** (bicycle on roads ×1.4) · F0 · sight 1000 cm · 1 slot · food staff 1 · no ranks |
| **Commands** | Move, Stop (I) · Disguise, Deliver, Withdraw (P) · default stance Hold Fire · formation role `protected` |
| **Production** | **$300** · `IA-BLD-04` · prerequisites: `tech.ia.espionage_tier_1` · **12 s** · 1 food · **cap 4** |
| **Veterancy** | none |
| **AI** | `ai.unit.covert_agent`. Deltas: route along roads away from known `DETECT.SPY` sources; abort to the nearest friendly structure if a police unit (UN-INF-04, UN-ARM-01) closes within 800 cm |
| **Multiplayer** | MPA-COVERT + MPA-ECON (carried token and parcel are owner-only) |
| **Presentation** | ANIM-CIV walk and cycle; disguise presentation profile identical to UN-CIV pedestrians. Icon of a satchel |

**Weapons:** none (unarmed).

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.shared.disguise` | Passive while unrevealed (P) | Always appears as a civilian pedestrian to opponents | Self | free | `DETECT.SPY`; Unaligned police inspection (R1); hostile action | Owner-only frame | — |
| `ability.ia.inf_14.courier_drop` | Active (P) | Hands one intelligence token or one ammunition parcel (restores 4 ammunition units to one unit) to a friendly unit or structure | Friendly unit/structure ≤ 150 cm | free · 3 s · 20 s · 150 cm | Interception; detain (IA-INF-12 counterparts, UN-INF-04/10) | Satchel hand-off | Deliver tokens first |

**Research and upgrades:** `tech.ia.courier_networks` (IA-BLD-04, $500): speed 230 → 245 cm/s; trade-off: cap 4 → 3.

**Counters:** *Beats:* nothing in combat. *Beaten by (hard):* any `DETECT.SPY` unit, police checkpoints, any armed unit once revealed. *Situational:* keeps the espionage chain alive deep in enemy or federal territory.

**Status:** identity E-R; jurisdiction risk E-R; all values P.
## 4. Armored vehicles, assault platforms and artillery

Ascendancy vehicles sit at the **upper edge** of each §4.3 band (E: costly advanced armor). Heavy and super-heavy hulls pay the faction surcharge: production petroleum **+30 %** and repair price **+25 %** (§1). Every vehicle is produced at `IA-BLD-05` Mechanized Assault Factory unless stated; prototypes also need `IA-BLD-06` Experimental Armor Laboratory. Vehicle damage values are base values before the +8 % faction modifier.

### IA-ARM-01 — Light Border Patrol Car

| Field | Specification |
| --- | --- |
| **Identity** | `unit.ia.arm_01` · Iron Ascendancy · Vehicle / armed patrol car · `TPL-VEH-LIGHT` · **Extended** · Playable |
| **Role** | **Primary:** cheap armed road recon (E-R). It is **not** the canonical opening scout, which is the IA-SUP-01 plane (E-R). **Secondary:** convoy escort; hunting enemy scouts and couriers. **Doctrine fit:** reconnaissance that finds weak points ahead of the thrust (E). |
| **Inspiration and background** | Four-wheel-drive patrol car with a pintle machine gun and a radio. C&C lineage (E-R): Raider Buggy (TW); Buggy (TD); Attack Buggy (TS). |
| **Visual identity** | none (asset requirement): dark grey open car, crimson door band, pintle MG, whip antenna. |
| **Statistics** | **260 HP** (`armor.veh_soft`) · WHEEL_LIGHT **540 cm/s** · F1 · sight **1600 cm** · `DETECT.CONCEALED` 60 % of sight · crew 2 (food 2) · fuel light · ExperienceValue 40 |
| **Commands** | as TPL-VEH-LIGHT + Patrol, Scout Route (P) · default stance Aggressive · formation role `scout_screen` |
| **Production** | **$400** · `IA-BLD-05` · prerequisites: `IA-BLD-01` · **8 s** · 2 food · petroleum 1 |
| **Veterancy** | Standard |
| **AI** | `ai.unit.skirmisher`. Deltas: patrol roads; disengage from anything with an AT weapon; chase scouts and couriers |
| **Multiplayer** | MPA-STD |
| **Presentation** | ANIM-VEH-WHEEL + gunner traverse. Engine whine; icon of a car with a gun |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.ia.arm_01.primary` pintle MG | SMALL_ARMS | 0–700 cm | 12 / 0.4 s (30 DPS base) | hitscan, fire while moving | 3 | GROUND, AIR_LOW | 100 % / 15 % / 2 % / 25 % rotary / 8 % |

**Abilities:** none unit-specific; its value is speed, sight and detection.

**Research and upgrades:** `tech.shared.vehicle_component_standardization`. `tech.ia.smoke_dischargers` (IA-BLD-05, $400; shared with IA-ARM-03/04): one-shot smoke at self (400 cm, 20 s, 40 s cooldown); trade-off: +$30 unit price.

**Counters:** *Beats (hard):* unarmed couriers and sensor vehicles (AC-SUP-07, MP-SUP-06); Meridian scout trucks (MP-ARM-01) at soft parity, infantry caught on roads. *Beaten by (hard):* AT infantry (AC-INF-03, RD-INF-04, MP-INF-03), any tank, HMG teams. *Situational:* road security against Meridian raiding trucks.

**Status:** identity and "not the opening scout" E-R; values P.

### IA-ARM-02 — Armored Personnel Half-Track

| Field | Specification |
| --- | --- |
| **Identity** | `unit.ia.arm_02` · Iron Ascendancy · Vehicle / APC (half-track) · `TPL-VEH-TRANSPORT` · **Core** · Playable |
| **Role** | **Primary:** infantry transport, exposed to anti-tank ambush (E-R). **Secondary:** MG fire support for dismounted squads. **Doctrine fit:** mechanized infantry exploit breakthroughs (E). |
| **Inspiration and background** | Open-topped armored half-track with a forward MG shield; 1980s militarized revival of a classic configuration. C&C lineage (E-R): APC (Ren); Flak Track (RA2); Reckoner (KW). |
| **Visual identity** | none (asset requirement): long angular hull, tracks at rear, open troop bay, crimson stripe. |
| **Statistics** | **540 HP** (`armor.veh_light`) · TRACKED (half-track: road ×1.15) **360 cm/s** · F2 · sight 1200 cm · **3 infantry slots**; open top: passengers take 25 % of HIGH_EXPLOSIVE and INCENDIARY AoE that hits the vehicle · crew 2 · fuel medium · ExperienceValue 85 |
| **Commands** | as TPL-VEH-TRANSPORT · default stance Aggressive · formation role `escort` |
| **Production** | **$850** · `IA-BLD-05` · prerequisites: `IA-BLD-03` operational · **13 s** · 2 food · petroleum 3 |
| **Veterancy** | Standard |
| **AI** | `ai.unit.transport`. Deltas: unload at 500 cm from the objective, then support with MG; withdraw at 40 % HP |
| **Multiplayer** | MPA-STD + MPA-TRANSPORT (manifest owner-only; the open bay shows occupied seats only to observers who can see it) |
| **Presentation** | ANIM-VEH-TRACK + passengers visible in the bay (manifest presentation must use generic infantry meshes, not true archetypes). Icon of a half-track |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.ia.arm_02.primary` shielded MG | SMALL_ARMS | 0–700 cm | 13 / 0.4 s | hitscan, front arc 180° | 3 | GROUND, AIR_LOW | 100 % / 15 % / 2 % / 25 % rotary / 8 % |

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.shared.transport` | Active (P) | Load / unload per §10.1 | Friendly infantry | free | Kill during unload | Ramp | — |
| `ability.ia.arm_02.assault_dismount` | Active (P) | Unloads all passengers in 1.0 s total (instead of 1.0 s per slot); dismounted squads get *Sprint* without its cooldown | Self, on legal ground | free · instant · 30 s · self | Prepared HMG and flame at the dismount point | Troops vault the sides | When within 500 cm of an observed objective |

**Research and upgrades:** `tech.shared.vehicle_component_standardization`. `tech.ia.armored_bay_covers` (IA-BLD-05, $500): removes the open-top penalty; trade-off: −10 % speed and dismount time 1.0 → 2.0 s.

**Counters:** *Beats (soft):* infantry in the open, scouts. *Beaten by (hard):* AT infantry and ATGMs (AC-INF-03, AC-ARM-06, RD-INF-04, MP-ARM-07), tanks; artillery hurts the passengers. *Situational:* delivers IA-INF-02/05 to a breach under the tank screen.

**Status:** identity and ambush exposure E-R; values P.

### IA-ARM-03 — Fast Assault Tank

| Field | Specification |
| --- | --- |
| **Identity** | `unit.ia.arm_03` · Iron Ascendancy · Vehicle / fast light tank · `TPL-VEH-MBT` (light) · **Core** · Playable |
| **Role** | **Primary:** aggressive mobile armored strike with limited protection (E-R). **Secondary:** exploitation into artillery and logistics; flank attack. **Doctrine fit:** operational speed and exploitation of breakthroughs (E). |
| **Inspiration and background** | Light tank with a medium-velocity gun and a powerful engine. C&C lineage (E-R): Light Tank (TD); Scorpion Tank (TW); Hammer Tank (RA3). |
| **Visual identity** | none (asset requirement): low angular turret, long hull, exhaust stacks, crimson turret band. |
| **Statistics** | **680 HP** (`armor.veh_medium`) · TRACKED **390 cm/s** · F2 · sight 1300 cm · crew 3 · fuel medium · ExperienceValue 105 |
| **Commands** | as TPL-VEH-MBT + Overdrive (P) · default stance Aggressive · formation role `flank` |
| **Production** | **$1,050** (light tank, MBT band low edge with Ascendancy premium) · `IA-BLD-05` · **16 s** · 3 food · petroleum 3 |
| **Veterancy** | Standard |
| **AI** | `ai.unit.skirmisher` with tank weights 0.6 / 1.0 / 0.6 / 0 / 0.5. Deltas: target artillery, AA and logistics first; avoid frontal duels with VEH_HEAVY; *Overdrive* to break contact or to reach artillery |
| **Multiplayer** | MPA-STD. *Overdrive* self-damage is server-applied each step |
| **Presentation** | ANIM-VEH-TRACK + turret. Exhaust flare during *Overdrive*; icon of a tank with speed lines |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.ia.arm_03.primary` medium-velocity gun | KINETIC_AP | 0–850 cm | 75 / 2.2 s; fire while moving | shell_direct, 2500 cm/s, tracking medium | 4 | GROUND | 20 % (never auto) / 90 % / 100 % / — / 45 % |
| `weapon.ia.arm_03.coax` coaxial MG | SMALL_ARMS | 0–650 cm | 10 / 0.4 s | hitscan | 3 | GROUND | 100 % / 15 % / 2 % / — / 8 % |

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.ia.arm_03.overdrive` | Active, **self-endangering** (P; E: explicit high-risk aggression) | +30 % speed for 10 s; the engine strain costs 1.5 % max HP per second; afterwards −20 % speed for 15 s | Self | free · 10 s · 50 s · self | Catch it in the slow window; AT ambush at the end of its run | Exhaust flare, engine scream; red status icon visible to observers | To reach artillery or AA within 1500 cm, or to escape at < 40 % HP |

**Research and upgrades:** `tech.shared.vehicle_component_standardization`. `tech.ia.smoke_dischargers`. `tech.ia.uprated_engines` (IA-BLD-06, $800): *Overdrive* +30 → +40 % speed; trade-off: self-damage 1.5 → 2 % per second.

**Counters:** *Beats (hard):* artillery (AC-ARM-07, RD-ARM-07, MP-ARM-08/12), AA vehicles (RD-ARM-06, MP-ARM-09), logistics. *Beats (soft):* APCs and light vehicles. *Beaten by (hard):* MBTs (AC-ARM-04, RD-ARM-04), ATGM carriers (AC-ARM-06), attack helicopters (AC-AIR-04). *Situational:* the exploitation unit; poor at holding ground.

**Status:** identity E-R; values and *Overdrive* P.

### IA-ARM-04 — Frontier Battle Tank

| Field | Specification |
| --- | --- |
| **Identity** | `unit.ia.arm_04` · Iron Ascendancy · Vehicle / main battle tank · `TPL-VEH-MBT` · **Core** · Playable |
| **Role** | **Primary:** main battle tank with slightly stronger raw firepower (E-R, E). **Secondary:** HE fire against infantry and structures. **Doctrine fit:** armored divisions that penetrate defensive positions (E). |
| **Inspiration and background** | Late-Cold-War MBT with a large-calibre smoothbore gun and composite frontal armor. C&C lineage (E-R): Medium Tank (TD); Predator Tank (TW); Titan (KW, tracked chassis conversion). |
| **Visual identity** | none (asset requirement): wedge turret, side skirts, crimson turret number, charcoal hull. |
| **Statistics** | **1,000 HP** (`armor.veh_heavy`) · TRACKED **320 cm/s** · F3 · sight 1300 cm · crew 4 · fuel heavy (petroleum 5 × 1.3 = **7**) · ExperienceValue 130 |
| **Commands** | as TPL-VEH-MBT + Ammunition Select (P) · default stance Aggressive · formation role `frontline` |
| **Production** | **$1,300** (MBT band top) · `IA-BLD-05` · prerequisites: `IA-BLD-05` · **23 s** · 4 food · petroleum 7 · repair +25 % |
| **Veterancy** | Standard |
| **AI** | `ai.unit.battle_tank` (retreat at 35 %, preservation doctrine). Deltas: switch to HE when no vehicle is observed within 1000 cm and ≥ 3 infantry are |
| **Multiplayer** | MPA-STD. Selected ammunition is owner-only state |
| **Presentation** | ANIM-VEH-TRACK + turret, recoil. Heavy gun report; icon of a wedge-turret tank |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.ia.arm_04.ap` main gun, AP round | KINETIC_AP | 0–950 cm | 115 / 3.0 s (41.4 DPS with +8 %) | shell_direct, 3000 cm/s, tracking medium | 4 | GROUND | 20 % / 90 % / 100 % / — / 45 % |
| `weapon.ia.arm_04.he` main gun, HE round | HIGH_EXPLOSIVE | 0–950 cm | 90 / 3.0 s; full 80 cm, max 160 cm, edge 40 % | shell_direct | 8 | GROUND, STRUCTURE | 110 % / 45 % / 12 % / — / 100 % |
| `weapon.ia.arm_04.coax` coaxial MG | SMALL_ARMS | 0–650 cm | 10 / 0.4 s | hitscan | 3 | GROUND | 100 % / 15 % / 2 % / — / 8 % |

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.ia.arm_04.ammunition_select` | Active toggle (P) | Switches the main gun between AP and HE; 3 s switch (one reload) | Self | free · until switched · 3 s · self | Mixed enemy forces punish the wrong choice | Shell icon on the unit card | As in the AI row |

**Research and upgrades:** `tech.shared.vehicle_component_standardization`. `tech.ia.smoke_dischargers`. `tech.ia.composite_armor_upgrade` (IA-BLD-06, $1,000): HP 1000 → 1150; trade-off: −5 % speed and petroleum +1 per tank produced.

**Counters:** *Beats (hard):* light and medium vehicles (AC-ARM-02/03, RD-ARM-03, MP-ARM-05). *Beats (soft):* RD-ARM-04 one-on-one (≈ 41 vs 33 DPS at equal HP band) and MP-ARM-06. *Beaten by (hard):* ATGM carriers (AC-ARM-06), tank destroyers (MP-ARM-07 in groups), attack helicopters (AC-AIR-04, RD-AIR-03), AT infantry in cover. *Beaten by (soft):* RD-ARM-04 at the Directorate's numbers. *Situational:* the backbone of every thrust; needs IA-ARM-08 against air.

**Status:** identity and stronger raw firepower E-R/E; values P.

### IA-ARM-05 — Heavy Siege Tank

| Field | Specification |
| --- | --- |
| **Identity** | `unit.ia.arm_05` · Iron Ascendancy · Vehicle / heavy siege tank · `TPL-VEH-HEAVY` · **Extended** · Playable |
| **Role** | **Primary:** slow, expensive high-damage assault against fortifications (E-R). **Secondary:** anchors the breach against counter-attacks. **Doctrine fit:** heavily protected assault vehicles that penetrate fortified positions (E). |
| **Inspiration and background** | Heavy tank with a short large-calibre demolition gun and thick frontal armor. C&C lineage (E-R): Mammoth Tank (TD); Apocalypse Tank (RA2); Overlord Tank (Gen). |
| **Visual identity** | none (asset requirement): boxy, very wide hull, stubby large gun, track guards, crimson-trimmed command cupola. |
| **Statistics** | **1,550 HP** (`armor.veh_heavy`) · TRACKED_HEAVY **245 cm/s** · F3 · sight 1300 cm · crew 4 · fuel heavy (petroleum 7) · ExperienceValue 240 |
| **Commands** | as TPL-VEH-HEAVY + Press the Breach (P) · default stance Aggressive · formation role `frontline` |
| **Production** | **$2,400** (heavy band top) · `IA-BLD-05` · prerequisites: `IA-BLD-06` operational · **35 s** · 4 food · petroleum 7 · repair +25 % |
| **Veterancy** | Standard |
| **AI** | `ai.unit.heavy_armor`. Deltas: target structures and emplacements first; never advance without IA-ARM-08 or IA-INF escort; *Press the Breach* only against structures |
| **Multiplayer** | MPA-STD |
| **Presentation** | ANIM-VEH-TRACK + heavy recoil, ground dust. Deep boom; icon of a tank with a broken wall |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.ia.arm_05.primary` demolition gun | HIGH_EXPLOSIVE | **250**–950 cm | 220 / 4.5 s; full 120 cm, max 220 cm, edge 40 % | shell_direct, tracking slow | 12 | GROUND, STRUCTURE | 110 % / 45 % / 12 % / — / 100 % (STR_H 60 %) |
| `weapon.ia.arm_05.secondary` anti-armor gun | KINETIC_AP | 0–850 cm | 90 / 3.5 s | shell_direct, tracking medium | 4 | GROUND | 20 % / 90 % / 100 % / — / 45 % |
| `weapon.ia.arm_05.mg` roof HMG | SMALL_ARMS | 0–700 cm | 14 / 0.4 s | hitscan | 3 | GROUND, AIR_LOW | 100 % / 15 % / 2 % / 25 % rotary / 8 % |

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.ia.arm_05.press_the_breach` | Active, **self-endangering** (P) | Demolition gun cooldown −40 % for 10 s; then the gun is unusable for 12 s (barrel cooling) and the tank takes +15 % damage for those 12 s | Self | free · 10 s + 12 s · 60 s · self | Counter-attack during the cooling window; tank destroyers | Glowing barrel VFX; status visible to observers | Only when a structure or emplacement is in range and no enemy VEH_HEAVY is observed within 1200 cm |

**Research and upgrades:** `tech.shared.vehicle_component_standardization`. `tech.ia.composite_armor_upgrade` (HP 1550 → 1750, −5 % speed).

**Counters:** *Beats (hard):* bunkers, fortifications and garrisons (enemy DEF structures), dug-in RD-ARM-12, deployed emplacements. *Beats (soft):* MBTs frontally. *Beaten by (hard):* attack helicopters and ground-attack jets (AC-AIR-02/04, RD-AIR-02/03), ATGM carriers at range (AC-ARM-06), flankers. *Situational:* the siege hammer of a deliberate assault; terrible at catching mobile forces.

**Status:** identity E-R; values and ability P.

### IA-ARM-06 — Superheavy Prototype Tank

| Field | Specification |
| --- | --- |
| **Identity** | `unit.ia.arm_06` · Iron Ascendancy · Vehicle / super-heavy multi-turret prototype · `TPL-VEH-HEAVY` (super) · **Conditional** · Playable · **cap 1** |
| **Role** | **Primary:** single, rare late-game experimental multi-turret design with huge fuel and repair costs (E-R). **Secondary:** breakthrough anchor and AA self-protection against helicopters. **Doctrine fit:** exceptionally powerful platforms that consume resources which could equip several formations (E: *Industrial Mobilization and Supply*). |
| **Inspiration and background** | Experimental tracked multi-turret heavy tank, visually plausible 1980s engineering: twin main guns in one large turret plus a secondary autocannon turret (roster rule 7; walkers are translated to a tracked hull, E-R). C&C lineage (E-R): Mammoth Mk. II (TS, walker translated); Redeemer (KW); King Oni (RA3, walker translated). |
| **Visual identity** | none (asset requirement): very long hull on eight road wheels per side, main turret with twin guns, rear autocannon turret, crimson prototype markings. |
| **Statistics** | **2,700 HP** (`armor.veh_heavy`) · TRACKED_HEAVY **170 cm/s** · **F4** · sight 1400 cm · crew 6 · fuel super (petroleum 10 × 1.3 = **13**) · ExperienceValue 450 |
| **Commands** | as TPL-VEH-HEAVY · default stance Aggressive · formation role `frontline` |
| **Production** | **$4,500** (super-heavy band top) · `IA-BLD-05` · prerequisites: `IA-BLD-06` and `IA-BLD-14` operational, `tech.ia.prototype_program` · **58 s** · 6 food · petroleum 13 · **repair +50 %** · **cap 1** (E-R: single, rare) |
| **Veterancy** | Standard |
| **AI** | `ai.unit.heavy_armor`. Deltas: never move without at least 2 escorts and one AA unit; return to a repair source when *Maintenance Burden* triggers |
| **Multiplayer** | MPA-STD. Each turret has an independent server-side target and cooldown |
| **Presentation** | ANIM-VEH-TRACK + multi-turret independent traverse. Very low engine rumble; LOD budget toward the upper hypothesis (§16.2). Icon of a twin-gun turret |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.ia.arm_06.main` twin main guns | KINETIC_AP | 0–1050 cm | 2 × 130 / 3.6 s | shell_direct, tracking medium | 6 | GROUND | 20 % / 90 % / 100 % / — / 45 % |
| `weapon.ia.arm_06.autocannon` rear autocannon turret | AUTOCANNON | 0–900 cm | 18 / 0.4 s | hitscan, independent 360° turret | 4 | GROUND, AIR_LOW | 65 % / 85 % / 10 % / 100 % rotary / 30 % |
| `weapon.ia.arm_06.mg` hull MGs | SMALL_ARMS | 0–650 cm | 2 × 10 / 0.4 s | hitscan, front arc 120° | 3 | GROUND | 100 % / 15 % / 2 % / — / 8 % |

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.ia.arm_06.maintenance_burden` | Passive, cost (P; E-R: huge repair costs) | After 180 s of combat time (time with a weapon firing or taking damage) since its last repair, −15 % speed and turret traverse until it is repaired to full at a repair source | Automatic | — | Sustained pressure that keeps it away from repair | Maintenance icon visible to the owner only | Return to IA-SUP-05 when triggered and no objective is within 1000 cm |
| `ability.ia.arm_06.fuel_starved_penalty` | Passive, cost (P) | While the owner's petroleum stock is 0, speed −50 % instead of the standard −25 % (§4.5) | Automatic | — | Fuel interdiction (attack IA-SUP-04, refineries) | Fuel icon | — |

**Research and upgrades:** `tech.ia.prototype_program` (IA-BLD-06, $2,000, 150 s): unlocks it. `tech.shared.vehicle_component_standardization`: repair price −20 % (still +50 % over base).

**Counters:** *Beats (hard):* any ground formation in a frontal fight, helicopters at short range. *Beaten by (hard):* strike aircraft and bombers (AC-AIR-02/03/08, RD-AIR-02/07), massed ATGMs (AC-ARM-06), fuel interdiction; Concord AC-ARM-15 as a like-for-like duel. *Situational:* one decisive push per match; the opportunity cost is roughly three IA-ARM-04 plus their fuel.

**Status:** identity, rarity and cost profile E-R; cap 1 P (framework §11.3); values P.

### IA-ARM-07 — Mobile Flame Assault Vehicle

| Field | Specification |
| --- | --- |
| **Identity** | `unit.ia.arm_07` · Iron Ascendancy · Vehicle / flame tank · `TPL-VEH-MBT` (assault) · **Extended** · Playable |
| **Role** | **Primary:** assault fortifications at extreme short range (E-R). **Secondary:** clear massed infantry and garrisons. Abstract game weapon only (E-R). **Doctrine fit:** explicit, counterable high-risk aggression (E). |
| **Inspiration and background** | Armored flame-projector vehicle on a medium tank hull with internal fuel tanks. C&C lineage (E-R): Flame Tank (TD); Devil's Tongue (TS); Purifier (KW, flame role). |
| **Visual identity** | none (asset requirement): twin projector nozzles, armored fuel cells on the rear deck with hazard bands. |
| **Statistics** | **720 HP** (`armor.veh_medium`) · TRACKED **310 cm/s** · F2 · sight 1200 cm · crew 3 · fuel medium · **death blast:** INCENDIARY 80 in 300 cm (fuel cells; §6.10) · ExperienceValue 120 |
| **Commands** | as TPL-VEH-MBT + Ram Breach (P) · default stance Aggressive · formation role `frontline` |
| **Production** | **$1,200** · `IA-BLD-05` · prerequisites: `IA-BLD-14` · **18 s** · 3 food · petroleum 3 |
| **Veterancy** | Standard |
| **AI** | `ai.unit.battle_tank` with weights 1.5 / 0.4 / 0.1 / 0 / 1.3. Deltas: close behind tanks; never lead into AT ambush range; keep 300 cm from friendly infantry (death blast) |
| **Multiplayer** | MPA-STD |
| **Presentation** | ANIM-VEH-TRACK + projector traverse. Roar audio; icon of a tank with a flame |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.ia.arm_07.primary` twin flame projectors | INCENDIARY | 0–450 cm | 45 / 0.5 s; `burning` 4 s; AmmoCapacity 40 bursts | stream, 200 cm wide | 10 | GROUND, STRUCTURE | 160 % / 25 % / 5 % (never auto) / — / 80 % (garrisons 200 %) |

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.ia.arm_07.ram_breach` | Active, **self-endangering** (P) | Drives to contact with a garrisoned structure or emplacement and floods it for 4 s: occupants take INCENDIARY 120 (garrison rule applies) and are ejected burning. The vehicle is immobile for the channel, takes +25 % damage and loses 8 % max HP from fuel-cell strain | Hostile garrisoned structure or emplacement ≤ 300 cm | 8 ammunition · 4 s · 45 s · 300 cm | AT fire on the immobile hull; defenders outside the building | Flooding VFX; red status visible to observers | Only when no enemy AT unit is observed within 900 cm |

**Research and upgrades:** `tech.ia.pressurised_projectors` (range 450 → 520 cm; AmmoCapacity 40 → 32). `tech.shared.vehicle_component_standardization`.

**Counters:** *Beats (hard):* garrisons, bunkers' occupants, massed infantry (RD-INF-01 blocks, MP-INF-12). *Beaten by (hard):* tanks and tank destroyers (AC-ARM-04, RD-ARM-04, MP-ARM-07), ATGMs, AT infantry at range. *Situational:* a fortified-town opener behind IA-ARM-04 cover.

**Status:** identity E-R; abstract weapon E-R; values P.

### IA-ARM-08 — Rapid Anti-Air Autocannon

| Field | Specification |
| --- | --- |
| **Identity** | `unit.ia.arm_08` · Iron Ascendancy · Vehicle / mobile gun AA · `TPL-VEH-AA` · **Core** · Playable |
| **Role** | **Primary:** protect heavy columns from aircraft (E-R). **Secondary:** shred infantry and light vehicles when no air threat exists. **Doctrine fit:** coordinated deployment of armor with protection against tactical aviation (E). |
| **Inspiration and background** | Tracked twin-autocannon air-defense vehicle with a search radar. C&C lineage (E-R): Mantis (KW); Slingshot (KW); Gattling Tank (Gen). |
| **Visual identity** | none (asset requirement): boxy turret with twin cannons and a rotating search radar dish. |
| **Statistics** | **580 HP** (`armor.veh_medium`) · TRACKED **340 cm/s** · F2 · sight 1300 cm / `DETECT.AIR_RADAR` **3000 cm** · crew 3 · fuel medium · ExperienceValue 115 |
| **Commands** | as TPL-VEH-AA + Barrage Mode (P) · default stance Aggressive (air) · formation role `escort` |
| **Production** | **$1,150** · `IA-BLD-05` · prerequisites: `IA-DEF-05` or `IA-BLD-07` operational · **16 s** · 3 food · petroleum 3 |
| **Veterancy** | Standard |
| **AI** | `ai.unit.air_defense`. Deltas: stay within 600 cm of the most valuable friendly ground unit; engage ground only when no air contact is within 3000 cm |
| **Multiplayer** | MPA-STD + MPA-SENSOR (radar footprint yields contacts only) |
| **Presentation** | ANIM-VEH-TRACK + radar spin, rapid recoil. Drumming cannon audio; icon of twin barrels pointing up |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.ia.arm_08.primary` twin 30 mm-class autocannon | AUTOCANNON | 0–1050 cm | 16 / 0.25 s (64 DPS base) | hitscan, tracking fast; vs AIR_HIGH proximity fuzing counts as 60 % | 4 | GROUND, AIR_LOW, AIR_HIGH | 65 % / 85 % / 10 % / 100 % rotary, 60 % fixed / 30 % |

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.ia.arm_08.barrage_mode` | Active (P) | 6 s of +50 % fire rate against air targets only; then 8 s with the guns cooling (no fire) | Self | free · 6 s + 8 s · 30 s · self | Bait it with a scout, then strike during the cooling window | Barrel glow; status visible to observers | When ≥ 2 air contacts are within 1050 cm |

**Research and upgrades:** `tech.shared.radar_signal_processing`: radar 3000 → 3400 cm. `tech.shared.electronic_countermeasures`: inside enemy jamming its radar loses contacts only beyond 1500 cm.

**Counters:** *Beats (hard):* helicopters (AC-AIR-04/09, RD-AIR-03/08, MP-AIR-02). *Beats (soft):* jets on low passes (AC-AIR-02, RD-AIR-02), infantry and light vehicles. *Beaten by (hard):* tanks and ATGMs, artillery; stand-off guided bombs (AC-AIR-03) from outside 1050 cm. *Situational:* mandatory escort for IA-ARM-05/06.

**Status:** identity E-R; values P.

### IA-ARM-09 — Heavy Anti-Tank Gun Carrier

| Field | Specification |
| --- | --- |
| **Identity** | `unit.ia.arm_09` · Iron Ascendancy · Vehicle / tank destroyer · `TPL-VEH-TD` · **Core** · Playable |
| **Role** | **Primary:** high-damage vehicle ambush with weak lateral protection (E-R). **Secondary:** overwatch of breakthrough flanks. **Doctrine fit:** advanced weapons balanced by explicit vulnerabilities (E). |
| **Inspiration and background** | Casemate tank destroyer with a long high-velocity gun on a tracked chassis. C&C lineage (E-R): Tank Destroyer (RA2); Beam Cannon (TW, long-range role); Refractor (TT, role). |
| **Visual identity** | none (asset requirement): low casemate, very long gun with muzzle brake, thin side plates with visible stowage. |
| **Statistics** | **580 HP** (`armor.veh_medium`) · TRACKED **320 cm/s** · F2 · sight 1400 cm · crew 3 · fuel medium · **side and rear hits** (attacker outside the ±60° front arc) take **+30 %** damage · ExperienceValue 125 |
| **Commands** | as TPL-VEH-TD + Hull Down (P) · default stance Hold Ground · formation role `rear_guard` |
| **Production** | **$1,250** · `IA-BLD-05` · prerequisites: `IA-BLD-14` · **18 s** · 3 food · petroleum 3 |
| **Veterancy** | Standard |
| **AI** | `ai.unit.tank_destroyer`. Deltas: face the expected threat axis; relocate when any enemy unit is observed outside its front arc within 800 cm |
| **Multiplayer** | MPA-STD + MPA-COVERT (concealment while hull-down) |
| **Presentation** | ANIM-VEH-TRACK + recoil and dust. Sharp crack audio; icon of a long gun |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.ia.arm_09.primary` high-velocity casemate gun | KINETIC_AP | 0–1250 cm | 170 / 4.0 s | shell_direct, 3500 cm/s, tracking medium; hull arc 30° | 6 | GROUND | 20 % (never auto) / 90 % / 100 % / — / 45 % |

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.ia.arm_09.hull_down` | Conditional (P) | After 4 s stationary on Hold Ground: frontal damage −25 % and Concealed (§9.1); broken by moving or 4 s after firing | Self | free · until broken · — · self | Flanking (removes the frontal benefit and adds +30 %), `DETECT.CONCEALED`, artillery | Dug-in icon (owner) | Default on Hold Ground |

**Research and upgrades:** `tech.ia.tungsten_penetrators` (IA-BLD-14, $900): damage 170 → 190; trade-off: unit price +$100 and cooldown 4.0 → 4.3 s.

**Counters:** *Beats (hard):* MBTs and heavy tanks from the front (AC-ARM-04/05, RD-ARM-04/05, MP-ARM-06/13). *Beaten by (hard):* infantry (it has no MG), flanking light tanks, artillery, attack helicopters. *Situational:* the counter to Directorate tank mass at chokepoints and to Concord heavy breakthroughs.

**Status:** identity and lateral weakness E-R; values P.

### IA-ARM-10 — Mobile Rocket Launcher

| Field | Specification |
| --- | --- |
| **Identity** | `unit.ia.arm_10` · Iron Ascendancy · Vehicle / rocket artillery · `TPL-VEH-ARTY` · **Core** · Playable |
| **Role** | **Primary:** barrage weapon with inaccurate saturation and supply demands (E-R). **Secondary:** suppression before an assault. **Doctrine fit:** mobile artillery exploiting breakthroughs (E). |
| **Inspiration and background** | Truck- or tracked-chassis multiple rocket launcher with a 12-tube pack. C&C lineage (E-R): SSM Launcher (TD); V3 Rocket Launcher (RA2); Specter (KW). |
| **Visual identity** | none (asset requirement): raised launcher box, armored cab, crimson ammunition markings. |
| **Statistics** | **430 HP** (`armor.veh_light`) · WHEEL_HEAVY **320 cm/s** · F2 · sight 1100 cm · crew 3 · fuel medium · **death blast** (ammunition, §6.10) · ExperienceValue 150 |
| **Commands** | as TPL-VEH-ARTY · default stance Hold Ground · formation role `support_weapon` |
| **Production** | **$1,500** · `IA-BLD-05` · prerequisites: `IA-BLD-14` · **22 s** · 3 food · petroleum 3 |
| **Veterancy** | Standard; scatter bonuses apply |
| **AI** | `ai.unit.artillery`. Deltas: fire only at clusters; displace after each full salvo; request resupply from IA-SUP-04/06 when empty |
| **Multiplayer** | MPA-STD + MPA-INDIRECT |
| **Presentation** | ANIM-VEH + elevation, ripple launch, smoke trails. Launch shriek; icon of rocket tubes |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.ia.arm_10.primary` rocket salvo | HIGH_EXPLOSIVE | **1000**–3800 cm | 12 × 40 per salvo (0.25 s between rockets), 18 s between salvos; **AmmoCapacity 2 salvos** | ballistic; each rocket full 80 cm, max 180 cm, edge 30 %; scatter 150–450 cm | 8 per rocket | GROUND, STRUCTURE | 110 % / 45 % / 12 % / — / 100 % |

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.shared.deploy` | Active (P) | 3 s deploy required to fire; 2 s undeploy | Self | free | Catch it undeployed | Jacks lower | Deploy on arrival |
| `ability.ia.arm_10.saturation_salvo` | Active (P) | Fires both loaded salvos in one ripple (24 rockets over 6 s) with scatter ×1.3 | Ground point or observed target | **both salvos** · 6 s · — (needs resupply) · weapon range | Counter-battery (AC-ARM-12, RD-ARM-08); interdict resupply | Owner impact preview | Only on a cluster of ≥ 6 enemy units or a structure group |

**Research and upgrades:** `tech.ia.extended_range_rockets` (IA-BLD-14, $800): range 3800 → 4200 cm; trade-off: scatter maximum 450 → 520 cm. `tech.shared.supply_preservation`: resupply −15 % USD.

**Counters:** *Beats (hard):* massed infantry (RD-INF-01 blocks), static defenses, clustered soft vehicles. *Beaten by (hard):* fast raiders (MP-ARM-01/02/07, AC-ARM-01), counter-battery artillery (RD-ARM-08, AC-ARM-07 cued by AC-ARM-12), aircraft. *Situational:* the "open the door" weapon before a breakthrough; empty after two salvos.

**Status:** identity, inaccuracy and supply demand E-R; values P.

### IA-ARM-11 — Self-Propelled Breaching Artillery

| Field | Specification |
| --- | --- |
| **Identity** | `unit.ia.arm_11` · Iron Ascendancy · Vehicle / heavy self-propelled howitzer · `TPL-VEH-ARTY` · **Extended** · Playable |
| **Role** | **Primary:** long-range counter-fortification artillery (E-R). **Secondary:** counter-battery fire with a spotter. **Doctrine fit:** penetrating fortified positions (E). |
| **Inspiration and background** | Heavy-calibre self-propelled howitzer on a tracked chassis with an open rear platform. C&C lineage (E-R): Artillery (TS); Pacifier FAV (Uprising); Juggernaut (TW, tracked chassis). |
| **Visual identity** | none (asset requirement): huge barrel on a low tracked hull, recoil spade, crew platform. |
| **Statistics** | **600 HP** (`armor.veh_medium`) · TRACKED_HEAVY **260 cm/s** · F3 · sight 1100 cm · crew 5 · fuel heavy (petroleum 7) · ExperienceValue 170 |
| **Commands** | as TPL-VEH-ARTY · default stance Hold Ground · formation role `support_weapon` |
| **Production** | **$1,700** (artillery band top) · `IA-BLD-05` · prerequisites: `IA-BLD-06` · **26 s** · 5 food · petroleum 7 · repair +25 % |
| **Veterancy** | Standard |
| **AI** | `ai.unit.artillery`. Deltas: prefer STR_HARDENED targets; fire only with observation (IA-INF-08, IA-SUP-01) |
| **Multiplayer** | MPA-STD + MPA-INDIRECT |
| **Presentation** | ANIM-VEH + spade deploy, heavy recoil, crew loading loop. Huge boom; icon of a howitzer with a crack |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.ia.arm_11.primary` heavy howitzer | HIGH_EXPLOSIVE | **1000**–4200 cm | 260 / 8.0 s; AmmoCapacity 12 | ballistic; full 150 cm, max 250 cm, edge 40 %; scatter 60–200 cm (accurate) | 12 | GROUND, STRUCTURE | 110 % / 45 % / 12 % / — / 100 % (STR_H 60 %) |

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.shared.deploy` | Active (P) | 5 s deploy (spade), 4 s undeploy; fires only deployed | Self | free | Catch it undeployed; raids | Spade animation | Deploy at 80 % of range from the target |
| `ability.ia.arm_11.breaching_round` | Active (P) | Next shot deals +60 % to structures and ignores garrison damage reduction; AoE radius halved | Observed structure or emplacement | 2 ammunition · next shot · 30 s · weapon range | Concealment of targets; counter-battery | Shell icon; impact preview | On any observed STR_HARDENED target |

**Research and upgrades:** `tech.shared.supply_preservation`. `tech.ia.fire_direction_center` (IA-BLD-14, $900): observed scatter ×0.7 → ×0.55 for IA-ARM-10/11; trade-off: both lose fire-on-unobserved-ground capability.

**Counters:** *Beats (hard):* bunkers, defenses and fortified producers; static artillery once located. *Beaten by (hard):* raiders and fast tanks, aircraft, counter-battery fire (RD-ARM-08 with RD-AIR-08). *Situational:* the long-range siege gun; useless against moving units.

**Status:** identity E-R; values P.

### IA-ARM-12 — Mobile Field Bunker

| Field | Specification |
| --- | --- |
| **Identity** | `unit.ia.arm_12` · Iron Ascendancy · Vehicle / deployable garrison bunker · `TPL-VEH-TRANSPORT` (deployable) · **Extended** · Playable |
| **Role** | **Primary:** armored hull that deploys as an infantry garrison near attack routes (E-R). **Secondary:** hold captured ground against counter-attack. The deployed bunker is a **state** of the same unit, not a separate unit (E-R). **Doctrine fit:** fortified installations and securing territory after the breakthrough (E). |
| **Inspiration and background** | Tracked armored carrier with hydraulic spades and fold-out armored firing walls. C&C lineage (E-R): Reckoner (KW); Tick Tank (TS); Battle Bunker (YR). |
| **Visual identity** | none (asset requirement): box hull; deployed state lowers onto spades and raises slotted steel walls. |
| **Statistics** | Mobile: **780 HP** (`armor.veh_medium`) · TRACKED **280 cm/s** · F3 · sight 1200 cm. Deployed: armor becomes **`armor.str_hardened`**, sight 1400 cm, 2 garrison slots with fire ports (§6.9). Crew 2 · fuel medium · ExperienceValue 115 |
| **Commands** | as TPL-VEH-TRANSPORT + Deploy/Undeploy, Garrison (P) · default stance Guard · formation role `rear_guard` |
| **Production** | **$1,150** · `IA-BLD-05` · prerequisites: `IA-BLD-03` operational · **18 s** · 2 food · petroleum 3 |
| **Veterancy** | Standard (by its own MG) |
| **AI** | `ai.unit.support_weapon`. Deltas: deploy at captured objectives and route chokepoints; auto-load nearby idle infantry when deploying |
| **Multiplayer** | MPA-STD + MPA-TRANSPORT. Deployed state is replicated; garrison manifest owner-only |
| **Presentation** | ANIM-VEH-TRACK + 5 s deploy sequence. Hydraulic hiss; icon of a bunker on tracks |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.ia.arm_12.primary` cupola HMG | SMALL_ARMS | 0–750 cm | 14 / 0.4 s | hitscan | 4 | GROUND, AIR_LOW | 100 % / 15 % / 2 % / 25 % rotary / 8 % |

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.shared.deploy` | Active (P) | 5 s deploy into the bunker state: armor `str_hardened`, immobile, 2 garrison squads fire through ports at +15 % range; 5 s undeploy | Self | free · until undeployed · 10 s · self | Heavy HE and siege guns, flame (garrison 200 %), engineers' demolition | Walls raise; deployed icon | As in the AI row |

**Research and upgrades:** `tech.ia.reinforced_bunker_walls` (IA-BLD-06, $600): deployed HP +300; trade-off: deploy time 5 → 7 s.

**Counters:** *Beats (hard):* infantry assaults into a held point (RD-INF-01 blocks, MP-INF-01/10). *Beaten by (hard):* artillery and siege fire (AC-ARM-07, RD-ARM-08), flame (RD-ARM-10), MBT HE. *Situational:* turns a breakthrough into held ground for IA-BLD-02 claims.

**Status:** identity and "deployed state, not a separate unit" E-R; values P.

### IA-ARM-13 — Attack Engineering Bulldozer

| Field | Specification |
| --- | --- |
| **Identity** | `unit.ia.arm_13` · Iron Ascendancy · Vehicle / armored engineer dozer · `TPL-VEH-ENGINEER` · **Extended** · Playable |
| **Role** | **Primary:** clears obstacles and can build cover; not a mass-destruction superweapon (E-R). **Secondary:** clears wrecks from roads and bridges. **Doctrine fit:** military engineering supports concentrated thrusts (E). |
| **Inspiration and background** | Armored bulldozer on a tank chassis with a hydraulic blade and a winch. C&C lineage (E-R): Grinder (Uprising, mechanical role); Dozer (TT); Crusher Crane (RA3). |
| **Visual identity** | none (asset requirement): heavy blade, armored cab, crimson hazard chevrons on the blade. |
| **Statistics** | **1,100 HP** (`armor.veh_heavy`) · TRACKED **260 cm/s** · F3 · sight 1100 cm · crew 2 · fuel heavy (petroleum 7) · ExperienceValue 105 |
| **Commands** | as TPL-VEH-ENGINEER · default stance Hold Fire · formation role `escort` |
| **Production** | **$1,050** · `IA-BLD-05` · prerequisites: `IA-BLD-05` · **16 s** · 2 food · petroleum 7 |
| **Veterancy** | Standard (by its MG) |
| **AI** | `ai.unit.engineer_vehicle`. Deltas: clear wrecks and obstacles on the group's path; build berms when the group holds |
| **Multiplayer** | MPA-STD. Terrain and obstacle changes are server-authoritative map state |
| **Presentation** | ANIM-VEH-TRACK + blade raise/lower and push. Diesel and scraping audio; icon of a blade |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.ia.arm_13.mg` cab MG | SMALL_ARMS | 0–600 cm | 10 / 0.4 s | hitscan | 3 | GROUND | 100 % / 15 % / 2 % / — / 8 % |

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.ia.arm_13.clear_obstacle` | Active (P) | Removes a barrier, wire, barricade or mine-field segment in 3 s; Unaligned barriers trigger an R1 incident (E) | Obstacle ≤ 200 cm | free · 3 s · — · 200 cm | Fire on the stationary dozer | Push animation | Path obstacles first |
| `ability.ia.arm_13.push_wreck` | Active (P) | Removes a wreck in 4 s (no salvage value) | Wreck ≤ 200 cm | free · 4 s · — · 200 cm | — | Push | Clear wrecks on roads and bridges |
| `ability.ia.arm_13.build_berm` | Active (P) | Builds a 6 m earth berm giving **heavy cover** (−50 %, §6.9) for 180 s; also blocks F1–F2 pathing | Ground ≤ 300 cm | $80 · 10 s · 30 s · 300 cm | HE and other dozers remove it; requires map cover support (U-08) | Earth mound | When the group holds a captured position |

**Research and upgrades:** `tech.shared.vehicle_component_standardization`. `tech.ia.sapper_training` also reduces *Clear Obstacle* 3 → 2 s.

**Counters:** *Beats:* obstacles, wrecks, Meridian mines (MP-ARM-11). *Beaten by (hard):* tanks and AT weapons (unarmed against vehicles). *Situational:* keeps mountain roads and bridge approaches open for heavy columns.

**Status:** identity and "not a superweapon" E-R; values P.

### IA-ARM-14 — Tracked Signal Jammer

| Field | Specification |
| --- | --- |
| **Identity** | `unit.ia.arm_14` · Iron Ascendancy · Vehicle / electronic warfare · `TPL-VEH-EW` · **Extended** · Playable |
| **Role** | **Primary:** suppresses hostile reconnaissance reports within radius (E-R). **Secondary:** breaks enemy guided weapons and command links. It never hides units from visual sight (framework §9.5). **Doctrine fit:** electronic warfare from the faction's research institutions (E). |
| **Inspiration and background** | Tracked carrier with telescopic jamming masts and generator. C&C lineage (E-R): Radar Van (Gen); Mobile Radar Jammer (RA1); Disruption Tower (TW). |
| **Visual identity** | none (asset requirement): box body with folded masts that rise on deployment; generator exhaust. |
| **Statistics** | **440 HP** (`armor.veh_light`) · TRACKED **320 cm/s** · F2 · sight 1400 cm · `DETECT.EMITTER` 2500 cm · crew 3 · fuel medium · ExperienceValue 105 |
| **Commands** | as TPL-VEH-EW · default stance Hold Fire · formation role `rear_guard` |
| **Production** | **$1,050** · `IA-BLD-05` · prerequisites: `IA-BLD-14` · **15 s** · 3 food · petroleum 3 |
| **Veterancy** | none (unarmed) |
| **AI** | `ai.unit.electronic_warfare`. Deltas: jam when an enemy air-radar unit, ATGM carrier, AEW aircraft or counter-battery radar is observed or detected as an emitter |
| **Multiplayer** | MPA-STD + MPA-SENSOR. The field's effects apply on the server to contact generation; visual observation is unchanged |
| **Presentation** | ANIM-VEH + mast deploy (4 s). Visible field ring for the owner; enemies get an emitter contact. Icon of a broken radio wave |

**Weapons:** none (unarmed).

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.shared.jam_field` | Active, deployed (P) | 1500 cm field (§9.5): suppresses enemy radar contacts, `DETECT.ARTILLERY` and `Reported` contact generation from remote sensors (radar, aircraft radar, signals intercept, listening posts) inside the field; degrades enemy radio-guided weapons to ballistic scatter; cuts enemy commander and officer links that pass through it. **Visual sight is unaffected** | Self (deployed, 4 s) | free · while deployed · — · 1500 cm | `DETECT.EMITTER` within 3000 cm (AC-INF-13, AC-ARM-12, RD-SUP-06, MP-MKT-05); home-on-jam strikes; any raid | Owner ring; enemy emitter contact | As in the AI row |

**Research and upgrades:** `tech.shared.electronic_countermeasures`: field 1500 → 1700 cm; trade-off: the emitter contact range for enemies rises 3000 → 3500 cm.

**Counters:** *Beats (hard):* Concord precision and radar play (AC-ARM-06/08/09/12, AC-AIR-07, AC-INF-14 links), RD-ARM-13 radar SAMs, RD-INF-13 and RD-INF-10 aura links. *Beaten by (hard):* any direct attack once its emitter is located; wire-guided and unguided weapons ignore it. *Situational:* the counter to a reconnaissance-and-precision opponent.

**Status:** identity E-R; jamming never hides visual units (framework §9.5, from E roster rule 7: no invisibility); values P.

### IA-ARM-15 — Armored Heavy Gun Emplacement Carrier

| Field | Specification |
| --- | --- |
| **Identity** | `unit.ia.arm_15` · Iron Ascendancy · Vehicle / deployable heavy gun emplacement · `TPL-VEH-HEAVY` (deployable) · **Conditional** · Playable · **cap 2** |
| **Role** | **Primary:** deployable endgame heavy assault platform; high transport cost (E-R). **Secondary:** long-range anti-armor and siege fire from a deployed casemate. The deployed emplacement is a **state** of the same unit (E-R). **Doctrine fit:** experimental platforms that consume resources of several formations (E). |
| **Inspiration and background** | Very heavy tracked carrier that lowers a casemated heavy gun onto outriggers. C&C lineage (E-R): Behemoth (KW); Mastodon (TT); Rig (TW). |
| **Visual identity** | none (asset requirement): extremely long carrier; deployed state with outrigger legs and an armored gun house. Must read as 1980s engineering (roster rule 7). |
| **Statistics** | Mobile: **1,800 HP** (`armor.veh_heavy`) · TRACKED_HEAVY **180 cm/s** · **F4** · sight 1300 cm. Deployed: **`armor.str_hardened`**, sight 1500 cm. Crew 6 · fuel super (petroleum **13**) · not air- or sea-transportable (high transport cost, E-R) · ExperienceValue 340 |
| **Commands** | as TPL-VEH-HEAVY + Deploy/Undeploy, Fire Mission when deployed (P) · default stance Hold Ground · formation role `support_weapon` |
| **Production** | **$3,400** · `IA-BLD-05` · prerequisites: `IA-BLD-06`, `tech.ia.prototype_program` · **48 s** · 6 food · petroleum 13 · repair +25 % · **cap 2** |
| **Veterancy** | Standard |
| **AI** | `ai.unit.heavy_armor`. Deltas: deploy only within an escorted perimeter; undeploy and retreat when ground threats come within 700 cm and escorts are gone |
| **Multiplayer** | MPA-STD + MPA-INDIRECT (deployed indirect mode) |
| **Presentation** | ANIM-VEH-TRACK + 10 s deploy (outriggers, gun-house raise). Massive gun report; icon of a casemate on tracks |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.ia.arm_15.direct` heavy gun, direct (deployed only) | KINETIC_AP | 0–1300 cm | 210 / 4.5 s | shell_direct, tracking medium; 180° arc | 6 | GROUND | 20 % / 90 % / 100 % / — / 45 % |
| `weapon.ia.arm_15.indirect` heavy gun, indirect (deployed only) | HIGH_EXPLOSIVE | 1300–3400 cm | 200 / 6.0 s; AmmoCapacity 16 | ballistic; full 120 cm, max 220 cm; scatter 80–260 cm | 12 | GROUND, STRUCTURE | 110 % / 45 % / 12 % / — / 100 % |
| `weapon.ia.arm_15.mg` defensive MGs | SMALL_ARMS | 0–650 cm | 2 × 10 / 0.4 s | hitscan | 3 | GROUND | 100 % / 15 % / 2 % / — / 8 % |

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.shared.deploy` | Active (P) | 10 s deploy into the emplacement (armor `str_hardened`, main gun active); 10 s undeploy. Main gun cannot fire while mobile | Self | free · until undeployed · 15 s · self | Catch it in the 10 s windows; aircraft; siege guns | Outriggers; deployed icon visible to observers | As in the AI row |

**Research and upgrades:** `tech.ia.prototype_program` unlocks it. `tech.ia.fire_direction_center` applies to its indirect mode.

**Counters:** *Beats (hard):* armor and fortifications inside its 1300 cm direct arc; static positions at range. *Beaten by (hard):* strike aircraft and bombers (AC-AIR-02/08, RD-AIR-02/07), counter-battery artillery, flanking raids during deployment. *Situational:* endgame positional anchor for a siege of a fortified base or IA-BLD-16 defense.

**Status:** identity, deployed-state rule and transport cost E-R; cap 2 and values P.
## 5. Reconnaissance, support and field logistics

`IA-SUP-01` is an aircraft by category; the ID prefix is not the category (framework §1.3).

### IA-SUP-01 — Nightjar Recon Plane

| Field | Specification |
| --- | --- |
| **Identity** | `unit.ia.sup_01` · Iron Ascendancy · Aircraft / unarmed reconnaissance plane · `TPL-AIR-SCOUT` · **Core** · Playable |
| **Role** | **Primary:** canonical cheap unarmed opening scout plane; no strike weapon and no covert intelligence theft (E-R, E). **Secondary:** spotting for IA-ARM-10/11 by keeping targets observed. It cannot acquire nuclear research intelligence by observation (E). **Doctrine fit:** reconnaissance identifies weaknesses before concentration (E). |
| **Inspiration and background** | Light single-engine high-wing observation aircraft with a camera pod. C&C lineage (E-R): Spy Plane (RA1, observation); Surveillance Sweep (RA3); Recon Drone (TT, information role). |
| **Visual identity** | none (asset requirement): slender charcoal airframe, crimson wingtips, ventral camera pod. |
| **Statistics** | **170 HP** (`armor.air_fixed`, AIR_LOW while loitering) · FIXED_WING (loiter) **1600 cm/s** · FA · sight **2600 cm** (ground footprint) · `DETECT.CONCEALED` 900 cm · aircrew 1 + 1 ground crew (food 2) · kerosene 1 per sortie · no ranks |
| **Commands** | Move, Stop (I) · Patrol/Orbit, Return to Base (P) · default stance Hold Fire · formation role `scout_screen` |
| **Production** | **$230** (scout band middle) · `IA-BLD-07` Combat Airfield · prerequisites: `IA-BLD-07` · **6 s** · 2 food · kerosene per sortie · no cap |
| **Veterancy** | none (unarmed) |
| **AI** | `ai.unit.air_scout`. Deltas: opening route over likely enemy start positions and fuel fields; avoid known AA footprints |
| **Multiplayer** | MPA-AIR. Sortie timer owner-only |
| **Presentation** | ANIM-AIR-FIXED. Light engine drone; icon of a plane with an eye |

**Weapons:** none (unarmed). Validator: no weapon slot (E).

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.ia.sup_01.loiter_sortie` | Active (P; U-04) | Launches for a 60 s loiter; returns to base automatically; 20 s turnaround and 1 kerosene per relaunch | Map point or route | 1 kerosene · 60 s · 20 s · map | AA guns and MANPADS (RD-ARM-06, AC-INF-03, MP-ARM-09), fighters | Sortie timer bar (owner) | Re-launch whenever no friendly sight covers a known enemy production area |

**Research and upgrades:** `tech.ia.long_range_tanks` (IA-BLD-07, $400): loiter 60 → 80 s; trade-off: kerosene 1 → 2 per sortie.

**Counters:** *Beats:* fog of war; ground scouts (RD-ARM-01, MP-ARM-01) cannot engage it, because SMALL_ARMS is 0 % against AIR_FIXED. *Beaten by (hard):* any AA (RD-ARM-06/13, AC-ARM-09, MP-ARM-09, UN-ARM-08), fighters. *Situational:* best early map awareness for the Ascendancy; ceded to Meridian trucks overall (E).

**Status:** identity, unarmed, no espionage and opening-scout role E; values P.

### IA-SUP-02 — Mobile Works Command Vehicle

| Field | Specification |
| --- | --- |
| **Identity** | `unit.ia.sup_02` · Iron Ascendancy · Vehicle / mobile construction works · `TPL-VEH-CONSTRUCTOR` · **Core** · Playable · **cap 2** |
| **Role** | **Primary:** expands construction with vulnerable deployed works (E-R). **Secondary:** forward production of an Occupation Command Post. **Doctrine fit:** expansion that creates logistical and administrative burdens (E: *Occupation and Expansion*). |
| **Inspiration and background** | Large tracked mobile works vehicle with cranes and folded site modules. C&C lineage (E-R): MCV (TD); Emissary (TW); Sputnik (RA3). |
| **Visual identity** | none (asset requirement): long tracked body, folded crane, crimson command flags. |
| **Statistics** | **1,150 HP** (`armor.veh_medium`) · TRACKED **250 cm/s** · F3 · sight 1000 cm · crew 4 · fuel heavy (petroleum 7) · no ranks |
| **Commands** | Move, Stop (I) · Deploy (P) · default stance Hold Fire · formation role `protected` |
| **Production** | **$2,700** · `IA-BLD-01` Supreme Field Command · prerequisites: `IA-BLD-05` · **42 s** · 4 food · petroleum 7 · **cap 2** |
| **Veterancy** | none |
| **AI** | `ai.unit.constructor`. Deltas: deploy only behind a held line with AA cover; never inside Unaligned jurisdiction unless the participant accepts the R1–R3 incident |
| **Multiplayer** | MPA-ECON |
| **Presentation** | ANIM-VEH + 15 s deploy unfolding into the structure. Icon of a crane |

**Weapons:** none (unarmed).

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.ia.sup_02.deploy_works` | Active (P; prototype default fixed base, U-AC-03 analogue) | 15 s deploy converts the vehicle into `IA-BLD-02` Occupation Command Post at 50 % HP, which builds up over 30 s; a destroyed post is lost (no undeploy) | Legal build site | free · 15 s · — · self | Destroy it during the 45 s vulnerable window; Unaligned response when inside federal jurisdiction (E: severe risk) | Deploy animation; jurisdiction warning in the UI before confirmation | As in the AI row |

**Research and upgrades:** `tech.shared.vehicle_component_standardization`.

**Counters:** *Beaten by (hard):* any anti-armor force during transit and deployment; aircraft. *Situational:* forward expansion that the Unaligned treats as an occupation claim (E).

**Status:** identity E-R; Unaligned jurisdiction risk of IA-BLD-02 E-R; deploy target P (open: **U-IA-01**); values P.

### IA-SUP-03 — Forced-Priority Construction Tractor

| Field | Specification |
| --- | --- |
| **Identity** | `unit.ia.sup_03` · Iron Ascendancy · Vehicle / builder · `TPL-VEH-CONSTRUCTOR` (light) · **Extended** · Playable |
| **Role** | **Primary:** builds sites under militarized production priorities, at high unrest cost (E-R). **Secondary:** repairs friendly structures. **Doctrine fit:** compulsory production priorities and coercion (E: *Economy*). |
| **Inspiration and background** | Tracked construction tractor with a crane arm and work crew. C&C lineage (E-R): Construction Dozer (Gen); Worker (Gen); Ore Collector expansion role (RA3). |
| **Visual identity** | none (asset requirement): small tracked tractor, crane arm, labour-crew figures (cosmetic). |
| **Statistics** | **450 HP** (`armor.veh_light`) · TRACKED **330 cm/s** · F2 · sight 1000 cm · crew 2 + labour detail (food 4) · fuel light · no ranks |
| **Commands** | Move, Stop (I) · Build, Repair (P) · default stance Hold Fire · formation role `protected` |
| **Production** | **$600** · `IA-BLD-01` · prerequisites: `IA-BLD-01` · **10 s** · 4 food · petroleum 1 |
| **Veterancy** | none |
| **AI** | `ai.unit.constructor`. Deltas: use *Forced Priority* only when Population Morale is above 60 and the structure is a producer or defense under threat |
| **Multiplayer** | MPA-ECON. Population Morale changes are participant-wide server state |
| **Presentation** | ANIM-VEH + work loop. Icon of a crane over a gear |

**Weapons:** none (unarmed).

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.shared.build_structure` | Active (P) | Builds structures per the production contract | Legal site | per structure | Raids | Scaffold | — |
| `ability.shared.repair_structure` | Active (P) | Repairs friendly structures at the §10.2 rate and price | Friendly structure ≤ 300 cm | USD proportional | — | Sparks | Under Guard |
| `ability.ia.sup_03.forced_priority` | Active, **cost** (P; E-R: high unrest) | +50 % build speed for 30 s on the current structure; costs **−2 Population Morale** per use and, inside Unaligned jurisdiction, an R1 incident | Structure under construction | free USD · 30 s · 60 s · self | The morale loss lowers resolve maxima and production output (E: Population Morale) | Red work banner | As in the AI row |

**Research and upgrades:** `tech.shared.vehicle_component_standardization`.

**Counters:** *Beaten by (hard):* any armed unit. *Situational:* fast fortification after a breakthrough, paid in morale.

**Status:** identity and unrest cost E-R; Population Morale link E; values P.

### IA-SUP-04 — Fuel Column Tanker

| Field | Specification |
| --- | --- |
| **Identity** | `unit.ia.sup_04` · Iron Ascendancy · Vehicle / fuel transport · `TPL-VEH-LOGISTICS` · **Extended** · Playable |
| **Role** | **Primary:** delivers refined vehicle and aviation fuels along contested routes (E-R). **Secondary:** feeds forward storage (IA-BLD-12) and IA-DEF-07 caches. Fuel stocks stay separate: petroleum and kerosene are never interchangeable (E). **Doctrine fit:** thrusts depend on fuel and secure lines of communication (E). |
| **Inspiration and background** | Military fuel tanker truck in a convoy role. C&C lineage (E-R): Supply Truck (Gen); Harvester (TD); Supply Drop (TT). |
| **Visual identity** | none (asset requirement): six-wheel tanker, hazard placards, crimson cab stripe; load type shown by a placard color (petroleum amber, kerosene blue). |
| **Statistics** | **460 HP** (`armor.veh_soft`) · WHEEL_HEAVY **350 cm/s** · F1 · sight 1000 cm · crew 2 · fuel light · **death blast** (§6.10) · cargo 20 fuel units of **one** type per load · no ranks |
| **Commands** | Move, Stop (I) · Deliver (P) · default stance Hold Fire · formation role `protected` |
| **Production** | **$480** · `IA-BLD-05` · prerequisites: `IA-BLD-10` operational · **9 s** · 2 food · petroleum 1 |
| **Veterancy** | none |
| **AI** | `ai.unit.logistics`. Deltas: shuttle between IA-BLD-10 and the nearest forward storage; reroute around observed threats |
| **Multiplayer** | MPA-ECON. Cargo type visible to observers (placard, design-visible); amount owner-only. Delivery is idempotent and void if the tanker dies before the transfer completes |
| **Presentation** | ANIM-VEH-WHEEL + hose connect. Icon of a fuel drop |

**Weapons:** none (unarmed).

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.shared.deliver_cargo` | Active / automated (P) | Loads 20 units of petroleum or kerosene at IA-BLD-10 and unloads them into a storage structure or cache within 300 cm over 6 s; the ledger moves the stock once | Owned refinery and storage | free · 6 s · — · 300 cm | Ambush on the route; any strike (death blast) | Transfer bar | Automated route |

**Research and upgrades:** `tech.shared.supply_preservation`: cargo 20 → 24. `tech.ia.armored_fuel_bowsers` (IA-BLD-05, $500): armor `veh_light`; trade-off: speed −10 %.

**Counters:** *Beaten by (hard):* raiders (MP-ARM-01/02, AC-ARM-01), aircraft, Directorate diversion of the stocks it feeds (RD-INF-12). *Situational:* the fuel line that keeps IA-ARM-06 and IA-ARM-15 moving.

**Status:** identity E-R; separate fuel stocks E; values P.

### IA-SUP-05 — Battlefield Recovery Tractor

| Field | Specification |
| --- | --- |
| **Identity** | `unit.ia.sup_05` · Iron Ascendancy · Vehicle / repair and recovery · `TPL-VEH-LOGISTICS` (armored) · **Core** · Playable |
| **Role** | **Primary:** field recovery of expensive armor at short range (E-R). **Secondary:** field repair and wreck salvage. **Doctrine fit:** heavy losses are costly, so recovery matters (E). |
| **Inspiration and background** | Armored recovery vehicle on a tank chassis with crane, winch and spade. C&C lineage (E-R): Mobile Repair Vehicle (TS); MRT (KW); Spanner (TT). |
| **Visual identity** | none (asset requirement): tank hull without turret, A-frame crane, tow bars. |
| **Statistics** | **820 HP** (`armor.veh_medium`) · TRACKED **300 cm/s** (towing ×0.6) · F3 · sight 1000 cm · crew 3 · fuel heavy (petroleum 7) · no ranks |
| **Commands** | Move, Stop, Guard (I) · Repair, Salvage, Tow (P) · default stance Hold Fire · formation role `rear_guard` |
| **Production** | **$950** · `IA-BLD-05` · prerequisites: `IA-BLD-05` · **15 s** · 3 food · petroleum 7 |
| **Veterancy** | none |
| **AI** | `ai.unit.logistics`. Deltas: repair the highest-value damaged vehicle within 600 cm; tow own heavy wrecks when no hostile is observed within 1000 cm |
| **Multiplayer** | MPA-STD + MPA-ECON (salvage credit and recovery are idempotent server events) |
| **Presentation** | ANIM-VEH-TRACK + crane and tow. Icon of a hook |

**Weapons:** none (unarmed).

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.shared.repair_vehicle` | Active / automated (P) | 20 HP/s to one vehicle within **200 cm** (short range, E-R); USD per §10.2 (+25 % for Ascendancy heavy hulls) | Friendly vehicle | USD proportional · channel · — · 200 cm | Artillery on the pair | Sparks | As in the AI row |
| `ability.shared.salvage_wreck` | Active (P) | 6 s: wreck → 10 % of its unit cost to the owner (§10.5) | Wreck ≤ 300 cm | free · 6 s · — · 300 cm | — | Cutting VFX | Enemy wrecks after the fight |
| `ability.shared.battlefield_recovery` | Active (P; U-11) | Tows an own heavy or super-heavy wreck to a repair source within 60 s; the unit is re-created at 25 % HP for 40 % of its cost | Own VEH_HEAVY wreck | 40 % cost · tow · — · contact | Kill it while slowed | Tow cable | Always for IA-ARM-05/06/15 wrecks |

**Research and upgrades:** `tech.shared.vehicle_component_standardization`: repair price −20 %.

**Counters:** *Beaten by (hard):* any combat unit; artillery. *Situational:* preserves IA-ARM-04/05/06 value; the Ascendancy's answer to attrition.

**Status:** identity and short range E-R; recovery U-11; values P.

### IA-SUP-06 — Camouflaged Supply Truck

| Field | Specification |
| --- | --- |
| **Identity** | `unit.ia.sup_06` · Iron Ascendancy · Vehicle / covert supply truck · `TPL-VEH-LOGISTICS` · **Conditional** · Playable · **cap 3** |
| **Role** | **Primary:** carries limited materiel using falsified manifests (E-R). **Secondary:** forward resupply of limited-ammunition weapons. **Doctrine fit:** covert logistics in a federation that resists occupation (E). |
| **Inspiration and background** | Civilian-pattern freight lorry carrying military stores under false paperwork. C&C lineage (E-R): Sudden Transport (RA3); Radar Van (Gen); Technical (Gen). |
| **Visual identity** | none (asset requirement): civilian lorry with commercial livery; the true presentation shows hidden ammunition crates to the owner only. |
| **Statistics** | **380 HP** (`armor.veh_soft`) · WHEEL_HEAVY **400 cm/s** · F1 · sight 1000 cm · crew 2 · fuel light · death blast · no ranks |
| **Commands** | Move, Stop (I) · Resupply, Falsified Manifest (P) · default stance Hold Fire · formation role `protected` |
| **Production** | **$650** · `IA-BLD-05` · prerequisites: `tech.ia.espionage_tier_1` · **11 s** · 2 food · petroleum 1 · **cap 3** |
| **Veterancy** | none |
| **AI** | `ai.unit.logistics` + covert routing. Deltas: travel on civilian roads; avoid `DETECT.SPY` sources and checkpoints |
| **Multiplayer** | MPA-ECON + MPA-COVERT. Explicit `falsified_manifest` state (framework §15); opponents receive the disguise archetype `UN-CIV-03` |
| **Presentation** | Disguise presentation profile identical to UN-CIV-03. Icon of a lorry with a question mark |

**Weapons:** none (unarmed).

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.ia.sup_06.falsified_manifest` | Passive while unrevealed (P) | Appears to opponents as `UN-CIV-03` Freight Lorry (§9.1 Disguised) | Automatic | — | `DETECT.SPY` (AC-INF-10, UN-INF-10/11, UN-INF-03 border checks); any hostile action; inspections at Unaligned checkpoints (R1) | Owner-only disguise frame | Keep it until the delivery point |
| `ability.shared.resupply_aura` | Active (P) | §10.4 aura: 1 ammo unit per 2 s to up to 4 units within 400 cm; stock 30 units; using it **reveals** the truck for 10 s | Friendly limited-ammo units | USD per §10.4 · channel · — · 400 cm | Exposure while resupplying | Crates open | Only out of enemy sight |

**Research and upgrades:** `tech.ia.courier_networks`: stock 30 → 36.

**Counters:** *Beaten by (hard):* any detector and any armed unit once revealed. *Situational:* keeps IA-ARM-10/11 firing behind enemy lines and deep in federal territory.

**Status:** identity and falsified manifests E-R; MPA-ECON `falsified_manifest` P; values P.

## 6. Air units and airborne delivery

Fixed-wing aircraft use the sortie model (U-04): they launch from `IA-BLD-07`, consume kerosene per sortie and rearm on return. Helicopters are produced at and rearm on `IA-BLD-08` Aircraft Dispersal Pad (P: the roster names the pad as helicopter sortie, refuel and concealment infrastructure).

### IA-AIR-01 — Interceptor Fighter

| Field | Specification |
| --- | --- |
| **Identity** | `unit.ia.air_01` · Iron Ascendancy · Aircraft / interceptor · `TPL-AIR-FIGHTER` · **Extended** · Playable |
| **Role** | **Primary:** defends controlled approaches; costly airframes (E-R). **Secondary:** escort for IA-AIR-02/05. **Doctrine fit:** high-performance aircraft from the faction's research base (E). |
| **Inspiration and background** | Single-seat 1980s interceptor with radar-guided air-to-air missiles and a cannon. C&C lineage (E-R): MiG (RA1); Apollo Fighter (RA3); Jet Tengu (RA3, role without transformation). |
| **Visual identity** | none (asset requirement): delta-wing silhouette, charcoal with crimson fin flash. |
| **Statistics** | **400 HP** (`armor.air_fixed`) · FIXED_WING **2,500 cm/s** · sight 2200 cm (air) · aircrew 1 + 1 ground (food 2) · kerosene 2 per sortie · ExperienceValue 205 |
| **Commands** | Attack, Move, Stop (I) · Patrol, Return to Base (P) · default stance Aggressive (air) · formation role `escort` |
| **Production** | **$2,050** (fighter band top) · `IA-BLD-07` · prerequisites: `IA-DEF-05` or `IA-BLD-14` · **25 s** · 2 food · kerosene per sortie |
| **Veterancy** | Standard |
| **AI** | `ai.unit.air_superiority`. Deltas: intercept bombers and transports first; never pursue into an observed SAM footprint |
| **Multiplayer** | MPA-AIR |
| **Presentation** | ANIM-AIR-FIXED + afterburner. Icon of a delta wing |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.ia.air_01.missiles` air-to-air missiles | FLAK_AA | 0–1400 cm | 120 / 2.5 s; **4 per sortie** | guided_missile | — | AIR_LOW, AIR_HIGH | — / — / — / 110 % rotary, 100 % fixed / — |
| `weapon.ia.air_01.cannon` cannon | AUTOCANNON | 0–700 cm | 20 / 0.3 s; 60 rounds per sortie | hitscan | — | AIR_LOW, AIR_HIGH | — / — / — / 100 % rotary, 60 % fixed / — |

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.shared.combat_air_patrol` | Active (P) | Orbits a point for 45 s and auto-engages air contacts entering 1400 cm | Map point | kerosene per sortie · 45 s · — · 1400 cm | SAM ambush (AC-ARM-09, RD-ARM-13) | Patrol ring | Over the main column |

**Research and upgrades:** `tech.shared.radar_signal_processing`: missile range 1400 → 1550 cm. `tech.shared.electronic_countermeasures`: its missiles keep guidance inside enemy jamming beyond 800 cm of the jammer.

**Counters:** *Beats (hard):* helicopters, bombers and transports (AC-AIR-05/06/08, RD-AIR-04/05/07, MP-SUP-09). *Beats (soft):* other fighters (AC-AIR-01, RD-AIR-01) on cost parity. *Beaten by (hard):* SAMs (AC-ARM-09, RD-ARM-13, UN-ARM-08). *Situational:* denies Concord air coordination; expensive to lose.

**Status:** identity E-R; values P.

### IA-AIR-02 — Low-Level Attack Jet

| Field | Specification |
| --- | --- |
| **Identity** | `unit.ia.air_02` · Iron Ascendancy · Aircraft / ground-attack jet · `TPL-AIR-STRIKE` · **Extended** · Playable |
| **Role** | **Primary:** fast anti-tank and interdiction sorties (E-R). **Secondary:** convoy and fuel-line interdiction. **Doctrine fit:** tactical aviation coordinated with armor (E). |
| **Inspiration and background** | Subsonic low-level attack aircraft with rocket pods and a heavy cannon. C&C lineage (E-R): Harrier (RA2); Vindicator (RA3); Banshee (TS, role without sci-fi weapons). |
| **Visual identity** | none (asset requirement): straight-wing attack jet, underwing pods, crimson nose band. |
| **Statistics** | **580 HP** (`armor.air_fixed`, flies AIR_LOW during attack runs) · FIXED_WING **1,800 cm/s** · sight 1800 cm · aircrew 1 + 1 ground (food 2) · kerosene 3 per sortie · ExperienceValue 215 |
| **Commands** | Attack (I) · Strike Area, Return to Base (P) · default stance Hold Fire until ordered · formation role `reserve` |
| **Production** | **$2,150** · `IA-BLD-07` · prerequisites: `IA-BLD-14` · **27 s** · 2 food · kerosene per sortie |
| **Veterancy** | Standard |
| **AI** | `ai.unit.air_strike`. Deltas: approach along valleys away from known AA; one pass, then return |
| **Multiplayer** | MPA-AIR. Low-level contact rule is server-applied to radar contacts |
| **Presentation** | ANIM-AIR-FIXED + rocket ripple, gun run. Icon of a jet over a tank |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.ia.air_02.rockets` anti-armor rockets | SHAPED_CHARGE | 0–1000 cm | 8 × 70 per pass; 2 passes per sortie | ballistic, scatter 40–120 cm | 4 | GROUND, STRUCTURE, WATER_SURFACE | 25 % / 100 % / 90 % / — / 70 % |
| `weapon.ia.air_02.cannon` heavy cannon | AUTOCANNON | 0–800 cm | 26 / 0.2 s during passes | hitscan | 4 | GROUND | 65 % / 85 % / 10 % / — / 30 % |

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.ia.air_02.low_level_ingress` | Active (P) | For one approach (max 20 s) enemy `DETECT.AIR_RADAR` produces contacts on it only within 1200 cm; it cannot attack on that pass's first 5 s | Self | free · ≤ 20 s · per sortie · self | Visual AA (guns, MANPADS) still sees it; AA guns punish the low pass | Ground-hugging flight | On every sortie against a radar-covered target |

**Research and upgrades:** `tech.shared.electronic_countermeasures`. `tech.ia.extended_hardpoints` (IA-BLD-07, $700): 3 passes per sortie; trade-off: kerosene 3 → 4 per sortie.

**Counters:** *Beats (hard):* tanks and columns in the open (AC-ARM-04/05, RD-ARM-04/05, MP-ARM-06), tankers and convoys. *Beaten by (hard):* AA guns (RD-ARM-06, MP-ARM-09, UN-ARM-08), MANPADS (AC-INF-03, RD-INF-04), fighters. *Situational:* punishes radar-based air defense; vulnerable to gun AA.

**Status:** identity E-R; values P.

### IA-AIR-03 — Assault Helicopter

| Field | Specification |
| --- | --- |
| **Identity** | `unit.ia.air_03` · Iron Ascendancy · Aircraft / attack helicopter · `TPL-AIR-HELI-ATTACK` · **Core** · Playable |
| **Role** | **Primary:** anti-infantry and anti-armor support (E-R). **Secondary:** flank security for assault columns. **Doctrine fit:** coordinated aviation with armor and infantry (E). |
| **Inspiration and background** | Armored attack helicopter with a chin gun, rocket pods and anti-tank missiles; can carry a small fire team (dual role, 1980s pattern). C&C lineage (E-R): Apache (Ren); Harpy (TS); Venom (TW, role adapted). |
| **Visual identity** | none (asset requirement): stepped-tandem or side-by-side gunship with stub wings, charcoal with crimson rotor tips. |
| **Statistics** | **560 HP** (`armor.air_rotary`) · ROTARY **760 cm/s** · sight 1600 cm · 1 infantry slot · aircrew 2 + 1 ground (food 3) · kerosene 1 per rearm · ExperienceValue 175 |
| **Commands** | as TPL-AIR-HELI-ATTACK · default stance Aggressive · formation role `flank` |
| **Production** | **$1,750** (attack helicopter band top) · `IA-BLD-08` · prerequisites: `IA-BLD-07` · **21 s** · 3 food · kerosene per rearm |
| **Veterancy** | Standard |
| **AI** | `ai.unit.attack_helicopter`. Deltas: rockets on infantry clusters, missiles on armor; retreat at 40 % HP or on AA lock |
| **Multiplayer** | MPA-AIR + MPA-TRANSPORT (1 slot manifest owner-only) |
| **Presentation** | ANIM-AIR-ROTARY + chin-gun traverse, missile launch. Icon of a helicopter with rockets |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.ia.air_03.gun` chin gun | AUTOCANNON | 0–900 cm | 16 / 0.3 s | hitscan | 4 | GROUND, AIR_LOW | 65 % / 85 % / 10 % / 100 % rotary / 30 % |
| `weapon.ia.air_03.rockets` rocket pods | HIGH_EXPLOSIVE | 0–1000 cm | 6 × 35 / 8 s; 4 volleys per rearm | ballistic; full 60, max 140 cm; scatter 40–150 | 8 | GROUND, STRUCTURE | 110 % / 45 % / 12 % / — / 100 % |
| `weapon.ia.air_03.atgm` anti-tank missiles | SHAPED_CHARGE | 0–1150 cm | 140 / 5 s; 4 per rearm | guided_missile | 2 | GROUND | 25 % / 100 % / 90 % / — / 70 % |

**Abilities:** none unit-specific beyond `ability.shared.transport` (1 slot, used to land IA-INF-08 or IA-INF-13).

**Research and upgrades:** `tech.shared.electronic_countermeasures`: its ATGMs keep guidance in jamming beyond 800 cm of the jammer. `tech.ia.helicopter_armor` (IA-BLD-08, $700): HP 560 → 650; trade-off: speed −8 %.

**Counters:** *Beats (hard):* tanks and IFVs without AA (AC-ARM-03/04, RD-ARM-03/04, MP-ARM-05/06), infantry in the open. *Beaten by (hard):* AA guns and SAMs (RD-ARM-06/13, AC-ARM-09, MP-ARM-09), fighters, MANPADS. *Situational:* the Ascendancy's most flexible air asset.

**Status:** identity E-R; values P.

### IA-AIR-04 — Night Transport Helicopter

| Field | Specification |
| --- | --- |
| **Identity** | `unit.ia.air_04` · Iron Ascendancy · Aircraft / covert transport helicopter · `TPL-AIR-HELI-TRANSPORT` · **Extended** · Playable |
| **Role** | **Primary:** move small covert teams by heliborne insertion (E-R). **Secondary:** casualty extraction of elite squads. **Doctrine fit:** elite units striking command facilities (E). |
| **Inspiration and background** | Medium transport helicopter with suppressed exhausts and night-flying equipment. C&C lineage (E-R): NightHawk (RA2); Chinook (TD); Twinblade (RA3). |
| **Visual identity** | none (asset requirement): flat black airframe with exhaust shrouds and door guns. |
| **Statistics** | **580 HP** (`armor.air_rotary`) · ROTARY **720 cm/s** · sight 1400 cm · **4 infantry slots** · aircrew 2 + 1 ground (food 3) · kerosene 1 per rearm · no ranks unless armed fire scores (door guns: standard) |
| **Commands** | as TPL-AIR-HELI-TRANSPORT · default stance Hold Fire · formation role `protected` |
| **Production** | **$1,350** · `IA-BLD-08` · prerequisites: `IA-BLD-04` operational · **17 s** · 3 food |
| **Veterancy** | Standard (door guns) |
| **AI** | `ai.unit.air_transport`. Deltas: use *Low-Signature Flight* over enemy territory; insert at the nearest unobserved landing zone |
| **Multiplayer** | MPA-AIR + MPA-TRANSPORT + MPA-COVERT (concealment state) |
| **Presentation** | ANIM-AIR-ROTARY + doors, fast-rope. Muffled rotor audio; icon of a helicopter with a moon |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.ia.air_04.door_guns` | SMALL_ARMS | 0–650 cm | 2 × 10 / 0.4 s | hitscan | 3 | GROUND, AIR_LOW | 100 % / 15 % / 2 % / 25 % rotary / 8 % |

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.shared.fast_rope` | Active (P) | Unloads infantry while hovering; 6 s exposure window | Ground point | free · 6 s · — · below | Fire on the hover | Ropes | At the landing zone |
| `ability.ia.air_04.low_signature_flight` | Active (P) | While moving at ≤ 75 % speed and not firing, it is Concealed (§9.1) to observers beyond 900 cm and produces no radar contact; any shot or landing reveals it for 6 s | Self | free · until broken · 15 s · self | `DETECT.CONCEALED`, AA radar within 900 cm, visual sight inside 900 cm | Owner-only concealed icon | Over enemy territory |

**Research and upgrades:** `tech.ia.advanced_clandestine_training`: the concealment distance 900 → 750 cm.

**Counters:** *Beaten by (hard):* AA guns and SAMs at short range (RD-ARM-06, MP-ARM-09, UN-ARM-08), fighters. *Situational:* inserts IA-INF-08/09/10 past ground detectors; carries no spy ability itself (E-R by analogy with IA-AIR-08).

**Status:** identity E-R; values P.

### IA-AIR-05 — Tactical Bomber

| Field | Specification |
| --- | --- |
| **Identity** | `unit.ia.air_05` · Iron Ascendancy · Aircraft / tactical bomber · `TPL-AIR-STRIKE` · **Extended** · Playable · **cap 3** |
| **Role** | **Primary:** bombs ground targets with conventional munitions; vulnerable inside air defenses (E-R). No nuclear role (E: the Ascendancy's victory is completion, not delivery). **Doctrine fit:** disruption of defenses and logistics (E). |
| **Inspiration and background** | Twin-engine swept-wing tactical bomber. Stealth is abstracted away (E-R). C&C lineage (E-R): Vertigo Bomber (TW, stealth behavior abstracted); Aurora Bomber (Gen); Century Bomber (RA3). |
| **Visual identity** | none (asset requirement): swept-wing bomber, internal bay, crimson tail band. |
| **Statistics** | **950 HP** (`armor.air_fixed`) · FIXED_WING **1,500 cm/s** · sight 1800 cm · aircrew 3 + 1 ground (food 4) · kerosene 5 per sortie · ExperienceValue 270 |
| **Commands** | Attack (I) · Strike Area, Return to Base (P) · default stance Hold Fire until ordered · formation role `reserve` |
| **Production** | **$2,700** · `IA-BLD-07` · prerequisites: `IA-BLD-14` · **34 s** · 4 food · kerosene per sortie · **cap 3** |
| **Veterancy** | Standard |
| **AI** | `ai.unit.air_strike`. Deltas: only after enemy AA in the path is reduced to ≤ 1 observed system or with fighter escort |
| **Multiplayer** | MPA-AIR |
| **Presentation** | ANIM-AIR-FIXED + bay doors. Icon of a falling bomb |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.ia.air_05.bombs` conventional bombs | HIGH_EXPLOSIVE | overflight | 8 × 140 per sortie, dropped along a 1600 cm line | bomb_drop; each full 120 cm, max 240 cm, edge 30 %; scatter 60–180 cm | 12 | GROUND, STRUCTURE | 110 % / 45 % / 12 % / — / 100 % |

**Abilities:** none unit-specific; *Strike Area* (shared order) defines the bomb line.

**Research and upgrades:** `tech.shared.electronic_countermeasures`: radar contacts on it appear only within 2200 cm. `tech.ia.heavy_bomb_load` (IA-BLD-07, $900): 10 bombs; trade-off: speed −10 %.

**Counters:** *Beats (hard):* static bases, artillery parks, clustered infantry and soft vehicles. *Beaten by (hard):* fighters (AC-AIR-01, RD-AIR-01, UN-AIR-01), SAMs. *Situational:* opens a fortified line before IA-ARM-05 arrives.

**Status:** identity E-R; values P.

### IA-AIR-06 — Gunship Escort Aircraft

| Field | Specification |
| --- | --- |
| **Identity** | `unit.ia.air_06` · Iron Ascendancy · Aircraft / heavy escort gunship (rotary) · `TPL-AIR-HELI-ATTACK` · **Extended** · Playable |
| **Role** | **Primary:** persistent support to assault columns (E-R). **Secondary:** anti-helicopter screen. **Doctrine fit:** coordinated application of air power with armor (E). |
| **Inspiration and background** | Heavy armed helicopter with a turreted autocannon and rocket pods, built to stay with a column. Fixed-wing or futuristic gunships are translated to a rotary airframe (P). C&C lineage (E-R): Hammerhead (KW); Salamander (TT); Harbinger Gunship (Uprising). |
| **Visual identity** | none (asset requirement): large twin-turbine helicopter, armored cockpit, side-mounted cannon turret. |
| **Statistics** | **720 HP** (`armor.air_rotary`) · ROTARY **650 cm/s** · sight 1600 cm · aircrew 3 + 1 ground (food 4) · kerosene 1 per rearm · ExperienceValue 190 |
| **Commands** | as TPL-AIR-HELI-ATTACK + Escort Column (P) · default stance Aggressive · formation role `escort` |
| **Production** | **$1,900** · `IA-BLD-08` · prerequisites: `IA-BLD-14` · **24 s** · 4 food |
| **Veterancy** | Standard |
| **AI** | `ai.unit.attack_helicopter`. Deltas: in *Escort Column*, stay over the assigned group and engage whatever attacks it |
| **Multiplayer** | MPA-AIR |
| **Presentation** | ANIM-AIR-ROTARY + side turret. Icon of a helicopter with a shield |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.ia.air_06.cannon` turreted autocannon | AUTOCANNON | 0–1000 cm | 20 / 0.3 s | hitscan, 270° turret | 4 | GROUND, AIR_LOW | 65 % / 85 % / 10 % / 100 % rotary / 30 % |
| `weapon.ia.air_06.rockets` rocket pods | HIGH_EXPLOSIVE | 0–1000 cm | 6 × 35 / 8 s; 4 volleys per rearm | ballistic | 8 | GROUND, STRUCTURE | 110 % / 45 % / 12 % / — / 100 % |

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.ia.air_06.escort_column` | Active (P) | Assigns the gunship to a friendly ground group: it follows the group's centre at 400 cm and gets +20 % target priority on any unit that damaged a group member in the last 5 s | Friendly unit or group | free · until cancelled · — · group | AA inside the column's path; separation from the group | Escort link line (owner) | Always assign to the main armored group |

**Research and upgrades:** `tech.ia.helicopter_armor` (720 → 820, −8 % speed).

**Counters:** *Beats (hard):* enemy attack helicopters (AC-AIR-04/09, RD-AIR-03/08), infantry ambushes on the column. *Beats (soft):* light vehicles. *Beaten by (hard):* AA guns and SAMs, fighters. *Situational:* makes the column resistant to ambush; ties air power to the ground thrust.

**Status:** identity E-R; rotary translation and values P.

### IA-AIR-07 — Prototype Ground-Attack Aircraft

| Field | Specification |
| --- | --- |
| **Identity** | `unit.ia.air_07` · Iron Ascendancy · Aircraft / prototype strike aircraft · `TPL-AIR-STRIKE` · **Conditional** · Playable · **cap 1** |
| **Role** | **Primary:** endgame high-firepower prototype with longer turnaround (E-R). **Secondary:** precision strike on prototypes and command vehicles. **Doctrine fit:** experimental weapons balanced by cost and maintenance (E). |
| **Inspiration and background** | Experimental twin-engine heavy attack aircraft with a large-calibre cannon and laser-guided missiles, plausible 1980s engineering. C&C lineage (E-R): Firehawk (TW); Harbinger Gunship (Uprising); ZOCOM Orca (KW). |
| **Visual identity** | none (asset requirement): heavy twin-engine attack jet, under-nose gun fairing, prototype crimson stripes. |
| **Statistics** | **800 HP** (`armor.air_fixed`) · FIXED_WING **1,700 cm/s** · sight 1800 cm · aircrew 2 + 2 ground (food 4) · kerosene 6 per sortie · **turnaround 60 s** (other jets 30 s) · ExperienceValue 330 |
| **Commands** | Attack (I) · Strike Area, Return to Base (P) · default stance Hold Fire until ordered · formation role `reserve` |
| **Production** | **$3,300** · `IA-BLD-07` · prerequisites: `IA-BLD-06`, `tech.ia.prototype_program` · **40 s** · 4 food · **cap 1** |
| **Veterancy** | Standard |
| **AI** | `ai.unit.air_strike`. Deltas: target heavy armor, prototypes and command vehicles; abort if 2+ AA systems cover the path |
| **Multiplayer** | MPA-AIR |
| **Presentation** | ANIM-AIR-FIXED + gun run with heavy recoil VFX. Icon of a jet with a large gun |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.ia.air_07.cannon` heavy cannon | KINETIC_AP | 0–900 cm | 45 / 0.25 s for 3 s per pass; 2 passes | hitscan | 4 | GROUND | 20 % / 90 % / 100 % / — / 45 % |
| `weapon.ia.air_07.missiles` guided missiles | SHAPED_CHARGE | 0–1500 cm | 4 × 160 per sortie | guided_missile | 2 | GROUND, STRUCTURE, WATER_SURFACE | 25 % / 100 % / 90 % / — / 70 % |

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.ia.air_07.long_turnaround` | Passive, cost (P; E-R) | Rearm takes 60 s and 6 kerosene; while kerosene is below 6 it cannot launch | Automatic | — | Kerosene interdiction; airfield attacks | Rearm timer (owner) | — |

**Research and upgrades:** `tech.ia.prototype_program` unlocks it. `tech.shared.electronic_countermeasures` applies to its missiles.

**Counters:** *Beats (hard):* heavy armor and prototypes (AC-ARM-05/15, RD-ARM-05/14, MP-ARM-13), command vehicles. *Beaten by (hard):* SAMs (AC-ARM-09, RD-ARM-13), fighters, airfield attacks. *Situational:* one decisive sortie per minute or so.

**Status:** identity and long turnaround E-R; cap 1 and values P.

### IA-AIR-08 — Covert Air Courier

| Field | Specification |
| --- | --- |
| **Identity** | `unit.ia.air_08` · Iron Ascendancy · Aircraft / covert light transport · `TPL-AIR-TRANSPORT` (light, STOL) · **Conditional** · Playable · **cap 2** |
| **Role** | **Primary:** infiltration and supply support; carries **no free spy ability** (E-R). **Secondary:** delivers intelligence tokens and supply parcels. **Doctrine fit:** covert logistics against the strongest counterintelligence actor (E). |
| **Inspiration and background** | Civil-registered short-takeoff light utility aircraft used for clandestine flights. C&C lineage (E-R): Transport Helicopter (Ren); Nod Cargo Plane (TD); Sudden Transport (RA3, stealth function). |
| **Visual identity** | none (asset requirement): high-wing STOL utility aircraft in civil livery. |
| **Statistics** | **320 HP** (`armor.air_fixed`, AIR_LOW) · FIXED_WING **1,400 cm/s**; lands on roads or authored strips · sight 1400 cm · **2 infantry slots (covert units only)** or 1 supply parcel (30 ammo units) · aircrew 1 + 1 ground (food 2) · kerosene 2 per sortie · no ranks |
| **Commands** | Move (I) · Air-land, Return to Base (P) · default stance Hold Fire · formation role `protected` |
| **Production** | **$1,150** · `IA-BLD-07` · prerequisites: `tech.ia.espionage_tier_1` · **16 s** · 2 food · **cap 2** |
| **Veterancy** | none |
| **AI** | `ai.unit.air_transport`. Deltas: fly a civil corridor; land only where no `DETECT.SPY` or AA is observed within 1500 cm |
| **Multiplayer** | MPA-AIR + MPA-TRANSPORT + MPA-COVERT (civil-aircraft disguise for radar contacts) |
| **Presentation** | Civil livery presentation profile for opponents. Icon of a light plane with an envelope |

**Weapons:** none (unarmed).

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.ia.air_08.civil_registration` | Passive while unrevealed (P) | Enemy radar contacts and visual sight show a civil light aircraft archetype; revealed by `DETECT.SPY` within range, by Unaligned air-traffic inspection (UN-AIR-05/08 within 1500 cm) or by unloading in view | Automatic | — | UN-AIR-05/08, UN-INF-11, AA that fires anyway (an incident for them only if it is truly civil) | Owner-only disguise frame | Keep it unless landing |
| `ability.ia.air_08.air_land` | Active (P) | Lands on a road or strip and unloads in 4 s, then takes off | Legal landing zone | free · 4 s · — · ground | Fire during the landing | Landing roll | Remote unobserved roads |

**Research and upgrades:** `tech.ia.courier_networks`: parcel 30 → 36 ammo units.

**Counters:** *Beaten by (hard):* Unaligned air surveillance and counterintelligence (UN-AIR-05/08, UN-INF-11), fighters once revealed, any AA. *Situational:* delivers IA-INF-09/10 deep behind lines; gives no espionage power itself (E-R).

**Status:** identity and "no free spy ability" E-R; values P.
## 7. Maritime and amphibious forces

Naval units need an authored water layer and `IA-BLD-09` Militarized Dockyard (E-R: maps with water access). Inside Unaligned territorial waters every Ascendancy vessel is subject to federal maritime policing (E: jurisdiction; standing hostility toward identified Ascendancy forces).

### IA-SEA-01 — Harbor Patrol Cutter

| Field | Specification |
| --- | --- |
| **Identity** | `unit.ia.sea_01` · Iron Ascendancy · Naval / patrol cutter · `TPL-NAV-PATROL` · **Conditional** · Playable |
| **Role** | **Primary:** protects clandestine landings; susceptible to official maritime policing (E-R). **Secondary:** anti-boat screen for IA-SEA-02. **Doctrine fit:** securing landings for concentrated operations (E). |
| **Inspiration and background** | Steel patrol cutter with a twin autocannon and machine guns. C&C lineage (E-R): Gunboat (RA1); Yari Mini-Sub (RA3, light boat role). |
| **Visual identity** | none (asset requirement): grey hull, crimson bow number, forward gun mount. |
| **Statistics** | **420 HP** (`armor.nav_light`) · SURFACE_SHALLOW **620 cm/s** · FN 300 cm · sight 1600 cm · crew 6 (food 6) · fuel light · ExperienceValue 75 |
| **Commands** | as TPL-NAV-PATROL · default stance Aggressive · formation role `escort` |
| **Production** | **$750** · `IA-BLD-09` · prerequisites: `IA-BLD-09` · **13 s** · 6 food · petroleum 1 |
| **Veterancy** | Standard |
| **AI** | `ai.unit.naval_patrol`. Deltas: escort landing craft; avoid Unaligned patrol boats unless the response tier is already R3 or higher |
| **Multiplayer** | MPA-STD |
| **Presentation** | ANIM-NAV + turret. Icon of a cutter |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.ia.sea_01.primary` twin autocannon | AUTOCANNON | 0–900 cm | 18 / 0.4 s | hitscan | 4 | WATER_SURFACE, GROUND, AIR_LOW | 65 % / 85 % / 10 % / 100 % rotary / 30 % (NAV_L 90 %) |

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.ia.sea_01.landing_screen` | Active (P) | 20 s: friendly landing craft within 600 cm take −25 % damage from NAV_LIGHT and GROUND small-arms and autocannon fire (the cutter draws fire) while the cutter takes +15 % | Self aura 600 cm | free · 20 s · 45 s · 600 cm | Anti-ship missiles and heavy guns ignore the screen | Smoke from the cutter | During any beaching within 1200 cm |

**Research and upgrades:** `tech.shared.vehicle_component_standardization` (naval repair −20 %).

**Counters:** *Beats (hard):* small boats and landing craft (AC-SEA-01/02, RD-SEA-01/02, MP-SEA-01/02/04). *Beaten by (hard):* missile boats (AC-SEA-03, UN-SEA-02), frigates, attack aircraft. *Situational:* landing security; provokes Unaligned maritime policing in federal waters.

**Status:** identity and policing exposure E-R; values P.

### IA-SEA-02 — Landing Transport Craft

| Field | Specification |
| --- | --- |
| **Identity** | `unit.ia.sea_02` · Iron Ascendancy · Naval / landing craft · `TPL-NAV-TRANSPORT` · **Conditional** · Playable |
| **Role** | **Primary:** infantry and light-armor coastal movement (E-R). **Secondary:** river crossings where the water layer allows. **Doctrine fit:** breakthroughs that need river and coastal mobility (E: river crossings and transport bottlenecks). |
| **Inspiration and background** | Bow-ramp landing craft for a platoon and one light vehicle. C&C lineage (E-R): Amphibious Transport (RA2); Hovercraft (TD); Bullfrog (RA3, delivery role). |
| **Visual identity** | none (asset requirement): flat hull, bow ramp, armored wheelhouse. |
| **Statistics** | **850 HP** (`armor.nav_light`) · SURFACE_SHALLOW **450 cm/s** · FN 450 cm · sight 1200 cm · **4 infantry slots + 1 vehicle slot (light or medium vehicle, F1–F2)** · crew 4 · fuel light · no ranks |
| **Commands** | as TPL-NAV-TRANSPORT · default stance Hold Fire · formation role `protected` |
| **Production** | **$1,100** · `IA-BLD-09` · **18 s** · 4 food · petroleum 1 |
| **Veterancy** | none |
| **AI** | `ai.unit.transport`. Deltas: beach only with IA-SEA-01 or air cover; withdraw at 40 % HP |
| **Multiplayer** | MPA-TRANSPORT |
| **Presentation** | ANIM-NAV + ramp. Icon of a landing craft |

**Weapons:** none (unarmed).

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.shared.transport` | Active (P) | Load / unload; beaching takes 3 s and unloading uses §10.1 rates | Legal shore | free | Fire on the beach | Ramp | — |

**Research and upgrades:** none unit-specific.

**Counters:** *Beaten by (hard):* any armed boat or aircraft, coastal defenses, HMG and flame on the beach. *Situational:* coastal flank landings with IA-ARM-03 or IA-ARM-02 aboard.

**Status:** identity E-R; values P.

### IA-SEA-03 — Fast Raider Boat

| Field | Specification |
| --- | --- |
| **Identity** | `unit.ia.sea_03` · Iron Ascendancy · Naval / missile raider · `TPL-NAV-COMBATANT` (light) · **Conditional** · Playable |
| **Role** | **Primary:** short-range anti-ship ambush in narrow waters (E-R). **Secondary:** interdiction of merchant barges. **Doctrine fit:** concentrated surprise against vulnerable points (E). |
| **Inspiration and background** | Fast attack craft with two anti-ship missile launchers and a gun. C&C lineage (E-R): Sea Scorpion (RA2); Naginata Cruiser (RA3, light role). |
| **Visual identity** | none (asset requirement): low hull, twin launch canisters, crimson stripe. |
| **Statistics** | **380 HP** (`armor.nav_light`) · SURFACE_SHALLOW **760 cm/s** · FN 300 cm · sight 1600 cm · crew 6 · fuel light · ExperienceValue 125 |
| **Commands** | as TPL-NAV-PATROL + Conceal (inshore) (P) · default stance Hold Ground · formation role `flank` |
| **Production** | **$1,250** · `IA-BLD-09` · prerequisites: `IA-BLD-14` · **17 s** · 6 food · petroleum 1 |
| **Veterancy** | Standard |
| **AI** | `ai.unit.naval_patrol`. Deltas: wait Concealed near a shore or island; strike NAV targets entering 1400 cm; withdraw after both missiles |
| **Multiplayer** | MPA-STD + MPA-COVERT (concealment) |
| **Presentation** | ANIM-NAV + missile launch. Icon of a boat with a missile |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.ia.sea_03.missiles` anti-ship missiles | ANTI_SHIP | 0–1400 cm | 2 × 220 per rearm (rearm at IA-BLD-09 or IA-SEA-06) | guided_missile | — | WATER_SURFACE, STRUCTURE (coastal) | — / 40 % / 30 % / — / 60 % (NAV_L 120 %, NAV_H 100 %) |
| `weapon.ia.sea_03.gun` gun | AUTOCANNON | 0–800 cm | 14 / 0.4 s | hitscan | 3 | WATER_SURFACE, AIR_LOW | (NAV_L 90 %) |

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.shared.conceal` | Conditional (P) | Concealed after 3 s stationary within 400 cm of a shore or island | Self | free | `DETECT.CONCEALED`, patrol aircraft (UN-AIR-08, MP-AIR-05), firing | Owner-only icon | Default in narrow waters |

**Research and upgrades:** `tech.shared.electronic_countermeasures`: its missiles keep guidance in jamming beyond 800 cm of the jammer.

**Counters:** *Beats (hard):* frigates and transports in narrow waters (AC-SEA-02/04, RD-SEA-05/07, MP-SEA-01/06). *Beaten by (hard):* patrol boats and aircraft once detected, helicopters. *Situational:* sea denial on the Adriatic islands and river mouths.

**Status:** identity E-R; values P.

### IA-SEA-04 — Covert Attack Submarine

| Field | Specification |
| --- | --- |
| **Identity** | `unit.ia.sea_04` · Iron Ascendancy · Naval / attack submarine · `TPL-NAV-SUB` · **Conditional** · Playable · **cap 2** |
| **Role** | **Primary:** expensive underwater route for raid and interdiction (E-R). **Secondary:** covert landing of one infantry team. **Doctrine fit:** advanced, expensive platforms (E). |
| **Inspiration and background** | Diesel-electric attack submarine with torpedoes and a small swimmer-delivery compartment. C&C lineage (E-R): Submarine (RA1); Typhoon Attack Submarine (RA2); Boomer Submarine (YR, conventional torpedo role). |
| **Visual identity** | none (asset requirement): black hull, crimson sail number. |
| **Statistics** | **950 HP** (`armor.nav_heavy`) · SUBSURFACE **330 cm/s** (submerged ×0.8) · FN 600 cm · sight 1600 cm surfaced, sonar 1200 cm · **1 infantry slot** · crew 6 (abstracted) · fuel light · ExperienceValue 220 |
| **Commands** | as TPL-NAV-SUB · default stance Hold Fire while submerged · formation role `flank` |
| **Production** | **$2,200** (submarine band top) · `IA-BLD-09` · prerequisites: `IA-BLD-14` · **42 s** · 6 food · **cap 2** |
| **Veterancy** | Standard |
| **AI** | `ai.unit.submarine`. Deltas: hunt transports and merchant barges; avoid frigates with `DETECT.SUBMERGED` |
| **Multiplayer** | MPA-COVERT (submerged) + MPA-TRANSPORT |
| **Presentation** | ANIM-NAV-SUB. Icon of a periscope |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.ia.sea_04.torpedoes` | TORPEDO | 0–1400 cm | 300 / 6.0 s; AmmoCapacity 8 | torpedo, 600 cm/s | — | WATER_SURFACE, WATER_SUBSURFACE | — (NAV_L 110 %, NAV_H 100 %) |

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.shared.dive_surface` | Active (P) | Submerged state (§9.1); 4 s transitions; firing reveals for 4 s | Self | free | `DETECT.SUBMERGED` (AC-SEA-04, UN-SEA-03, UN-AIR-08) | Depth icon | Submerged by default |
| `ability.ia.sea_04.swimmer_delivery` | Active (P) | Lands its 1 infantry team on a shore within 300 cm while submerged; 8 s exposure | Shore point | free · 8 s · — · 300 cm | Coastal observation; patrol boats | Bubbles VFX | Deliver IA-INF-08/09 behind a coastal defense line |

**Research and upgrades:** `tech.ia.quiet_propulsion` (IA-BLD-14, $900): `DETECT.SUBMERGED` range against it −20 %; trade-off: speed −10 %.

**Counters:** *Beats (hard):* transports, barges, landing craft (AC-SEA-02, MP-SEA-01/04, RD-SEA-07). *Beats (soft):* frigates without sonar escort. *Beaten by (hard):* ASW frigates and patrol aircraft (AC-SEA-04, UN-SEA-03, UN-AIR-08). *Situational:* blockade and covert insertion.

**Status:** identity E-R; cap and values P.

### IA-SEA-05 — Armored Coastal Gunboat

| Field | Specification |
| --- | --- |
| **Identity** | `unit.ia.sea_05` · Iron Ascendancy · Naval / armored gunboat (shore support) · `TPL-NAV-COMBATANT` · **Conditional** · Playable |
| **Role** | **Primary:** gunfire support from available sea or river sectors (E-R). **Secondary:** escort against light craft. **Doctrine fit:** fire support for the breakthrough along rivers and coasts (E). |
| **Inspiration and background** | Shallow-draft armored monitor with a medium gun turret and AA guns, usable on large rivers. C&C lineage (E-R): Destroyer (RA1); Assault Destroyer (RA3). |
| **Visual identity** | none (asset requirement): low armored hull, single heavy turret, crimson pennant. |
| **Statistics** | **1,500 HP** (`armor.nav_heavy`) · SURFACE_SHALLOW **360 cm/s** · FN 500 cm · sight 2000 cm · crew 12 (abstracted, food 12) · fuel medium · ExperienceValue 200 |
| **Commands** | as TPL-NAV-COMBATANT + Fire Mission (P) · default stance Aggressive · formation role `support_weapon` |
| **Production** | **$2,000** · `IA-BLD-09` · prerequisites: `IA-BLD-14` · **34 s** · 12 food · petroleum 3 |
| **Veterancy** | Standard |
| **AI** | `ai.unit.naval_combatant`. Deltas: support ground assaults within 1600 cm of the water's edge |
| **Multiplayer** | MPA-STD + MPA-INDIRECT |
| **Presentation** | ANIM-NAV + turret. Icon of a turret on a hull |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.ia.sea_05.main` medium gun | HIGH_EXPLOSIVE | 300–2400 cm | 130 / 3.5 s | ballistic beyond 1400 cm (scatter 50–180), direct inside; full 100, max 200 cm | 10 | GROUND, WATER_SURFACE, STRUCTURE | 110 % / 45 % / 12 % / — / 100 % (NAV_L 60 %) |
| `weapon.ia.sea_05.aa` twin AA guns | AUTOCANNON | 0–900 cm | 14 / 0.3 s | hitscan | 3 | AIR_LOW, AIR_HIGH, WATER_SURFACE | AIR_R 100 %, AIR_F 60 %, NAV_L 90 % |

**Abilities:** none unit-specific; shore fire uses the shared Fire Mission order.

**Research and upgrades:** `tech.ia.fire_direction_center` applies to its indirect mode.

**Counters:** *Beats (hard):* riverbank infantry, light craft, coastal defenses. *Beaten by (hard):* submarines (AC-SEA-05, RD-SEA-04, UN-SEA-04), anti-ship missiles (AC-SEA-03, UN-SEA-02, MP-MKT-03), strike aircraft. *Situational:* the Ascendancy's river artillery.

**Status:** identity E-R; values P.

### IA-SEA-06 — Auxiliary Seaborne Headquarters

| Field | Specification |
| --- | --- |
| **Identity** | `unit.ia.sea_06` · Iron Ascendancy · Naval / mobile maritime hub · `TPL-NAV-TRANSPORT` (HQ) · **Conditional** · Playable · **cap 1** |
| **Role** | **Primary:** temporary maritime deployment centre, not a permanent carrier (E-R). **Secondary:** boat rearm and repair; command vehicle (framework §13.5), **not** a talent appointment. **Doctrine fit:** operational projection that strains logistics (E). |
| **Inspiration and background** | Converted merchant ship with command and workshop modules. C&C lineage (E-R): Transport Ship (RA1); Amphibious Transport (RA2); Support Crawler (TT, mobile hub role). |
| **Visual identity** | none (asset requirement): large grey hull, antenna farm, deck cranes. |
| **Statistics** | **2,200 HP** (`armor.nav_heavy`) · SURFACE_DEEP **300 cm/s** · FN 900 cm · sight 2000 cm · 6 infantry slots + 2 vehicle slots · crew 16 (abstracted) · fuel medium · no ranks |
| **Commands** | Move, Stop (I) · Anchor (deploy), Load/Unload (P) · default stance Hold Fire · formation role `protected` |
| **Production** | **$2,900** · `IA-BLD-09` · prerequisites: `IA-BLD-14` · **46 s** · 16 food · petroleum 3 · **cap 1** |
| **Veterancy** | none |
| **AI** | `ai.unit.transport`. Deltas: anchor 1000 cm offshore from the landing zone, inside friendly AA and patrol cover |
| **Multiplayer** | MPA-TRANSPORT + MPA-ECON (production queue owner-only) |
| **Presentation** | ANIM-NAV + crane loop when anchored. Icon of a ship with a flag |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.ia.sea_06.aa` self-defense AA guns | AUTOCANNON | 0–900 cm | 12 / 0.3 s | hitscan | 3 | AIR_LOW, AIR_HIGH, WATER_SURFACE | AIR_R 100 %, AIR_F 60 %, NAV_L 90 % |

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.ia.sea_06.forward_anchorage` | Active, **temporary** (P; E-R) | Anchored for at most **240 s**: acts as a producer for IA-INF-01/02/04 and IA-SEA-01/02, repairs boats within 600 cm at the §10.2 rate and rearms IA-SEA-03/04. Afterwards it must weigh anchor and cannot re-anchor for 120 s | Self, deep water | free · 240 s · 120 s · 600 cm | Submarines and anti-ship missiles against the immobile hull; air strikes | Anchor icon and timer visible to the owner; ship visible to observers | Anchor before a landing, leave when the timer ends |

**Research and upgrades:** `tech.shared.supply_preservation`: rearm cost −15 %.

**Counters:** *Beaten by (hard):* submarines (AC-SEA-05, RD-SEA-04, UN-SEA-04), missile boats, strike aircraft. *Situational:* projects a short-lived amphibious beachhead; never a permanent naval base (E-R).

**Status:** identity and "temporary, not a permanent carrier" E-R; values P; anchorage duration **U-IA-07**.

## 8. Faction roster summary

USD and build seconds are PB. Damage modifiers are not shown. "Hard counters (to it)" lists opposing archetypes or roster IDs; "It hard-counters" lists archetypes.

| ID | Name | Cat. | Template | Tier | Primary role | HP | Armor | Speed | USD | Build s | Producer | Hard counters (to it) | It hard-counters |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| IA-INF-01 | Garrison Riflemen | INF | INF-LINE | Core | Line rifles | 260 | inf_light | 205 | 320 | 8 | BLD-03 | HMG, mortars, artillery | Agents, unarmed support |
| IA-INF-02 | Shock Assault Section | INF | INF-LINE | Core | Close assault | 320 | inf_protected | 215 | 460 | 11 | BLD-03 | HMG at range, any vehicle | Garrisons |
| IA-INF-03 | Heavy Grenadier Squad | INF | INF-HW | Ext | Area suppression | 210 | inf_light | 190 | 420 | 10 | BLD-03 | Armor, marksmen | Garrisons, clusters |
| IA-INF-04 | Portable Anti-Armor Cell | INF | INF-AT | Core | Guided AT ambush | 180 | inf_light | 195 | 520 | 12 | BLD-03 | HMG, flame, helicopters | MBTs, APCs |
| IA-INF-05 | Urban Flame Detachment | INF | INF-HW | Ext | Flame assault | 180 | inf_protected | 190 | 480 | 11 | BLD-03 | Armor, marksmen | Garrisons, infantry in cover |
| IA-INF-06 | Frontier Combat Medic | INF | INF-SUPPORT | Core | Healing | 110 | inf_light | 215 | 380 | 10 | BLD-03 | Marksmen, raiders | — |
| IA-INF-07 | Technical Demolition Engineer | INF | INF-SUPPORT | Core | Capture, repair, obstacles | 130 | inf_light | 205 | 440 | 12 | BLD-03 | Any armed unit | Obstacles, mines |
| IA-INF-08 | Covert Reconnaissance Team | INF | INF-SPEC | Ext | Recon, marksman | 130 | inf_light | 210 | 640 | 13 | BLD-04 | Detectors, vehicles | Officers, AT/HW teams |
| IA-INF-09 | Ascendancy Intelligence Agent | INF | INF-COVERT | Cond (cap 3) | Spy; nuclear infiltration | 110 | inf_light | 220 | 1,200 | 20 | BLD-04 | `DETECT.SPY` | Unguarded producers |
| IA-INF-10 | Facility Contamination Operative | INF | INF-COVERT | Cond (cap 2) | Facility contamination | 100 | inf_light | 215 | 1,400 | 22 | BLD-04 | `DETECT.SPY`, decontamination | Production queues |
| IA-INF-11 | Intimidation Officer | INF | INF-COMMAND | Core (cap 2) | Intimidation | 160 | inf_protected | 205 | 1,200 | 18 | BLD-03 | Grouped forces, marksmen | Isolated broken units |
| IA-INF-12 | Political Security Enforcer | INF | INF-LINE | Ext | `DETECT.SPY`, detention | 225 | inf_protected | 205 | 400 | 10 | BLD-03 | HMG, armor | Agents, saboteurs |
| IA-INF-13 | Special Operations Raider | INF | INF-COMMAND | Ext (cap 1) | Commander candidate (U-14) | 520 | inf_protected | 225 | 2,300 | 32 | BLD-04 | Marksmen, artillery, jammers | — (multiplier) |
| IA-INF-14 | Civilian Disguise Courier | INF | INF-COVERT | Cond (cap 4) | Covert courier | 80 | inf_light | 230 | 300 | 12 | BLD-04 | `DETECT.SPY`, police | — |
| IA-ARM-01 | Light Border Patrol Car | VEH | VEH-LIGHT | Ext | Armed road recon | 260 | veh_soft | 540 | 400 | 8 | BLD-05 | AT infantry, tanks | Couriers, sensor vehicles |
| IA-ARM-02 | Armored Personnel Half-Track | VEH | VEH-TRANSPORT | Core | APC | 540 | veh_light | 360 | 850 | 13 | BLD-05 | ATGMs, AT infantry | — |
| IA-ARM-03 | Fast Assault Tank | VEH | VEH-MBT | Core | Fast light tank | 680 | veh_medium | 390 | 1,050 | 16 | BLD-05 | MBTs, ATGMs | Artillery, AA, logistics |
| IA-ARM-04 | Frontier Battle Tank | VEH | VEH-MBT | Core | MBT | 1,000 | veh_heavy | 320 | 1,300 | 23 | BLD-05 | ATGMs, TDs, attack helicopters | Light/medium vehicles |
| IA-ARM-05 | Heavy Siege Tank | VEH | VEH-HEAVY | Ext | Siege tank | 1,550 | veh_heavy | 245 | 2,400 | 35 | BLD-05 | Aircraft, ATGMs | Fortifications |
| IA-ARM-06 | Superheavy Prototype Tank | VEH | VEH-HEAVY | Cond (cap 1) | Super-heavy | 2,700 | veh_heavy | 170 | 4,500 | 58 | BLD-05 + BLD-06 | Strike aircraft, fuel interdiction | Ground formations |
| IA-ARM-07 | Mobile Flame Assault Vehicle | VEH | VEH-MBT | Ext | Flame tank | 720 | veh_medium | 310 | 1,200 | 18 | BLD-05 | Tanks, ATGMs | Garrisons, massed infantry |
| IA-ARM-08 | Rapid Anti-Air Autocannon | VEH | VEH-AA | Core | Gun AA | 580 | veh_medium | 340 | 1,150 | 16 | BLD-05 | Tanks, artillery | Helicopters |
| IA-ARM-09 | Heavy Anti-Tank Gun Carrier | VEH | VEH-TD | Core | Tank destroyer | 580 | veh_medium | 320 | 1,250 | 18 | BLD-05 | Infantry, flankers | MBTs, heavy tanks |
| IA-ARM-10 | Mobile Rocket Launcher | VEH | VEH-ARTY | Core | Saturation rockets | 430 | veh_light | 320 | 1,500 | 22 | BLD-05 | Raiders, counter-battery | Massed infantry, statics |
| IA-ARM-11 | Self-Propelled Breaching Artillery | VEH | VEH-ARTY | Ext | Siege howitzer | 600 | veh_medium | 260 | 1,700 | 26 | BLD-05 | Raiders, aircraft | Bunkers, defenses |
| IA-ARM-12 | Mobile Field Bunker | VEH | VEH-TRANSPORT | Ext | Deployable bunker | 780 | veh_medium / str_hardened | 280 | 1,150 | 18 | BLD-05 | Artillery, flame | Infantry assaults |
| IA-ARM-13 | Attack Engineering Bulldozer | VEH | VEH-ENGINEER | Ext | Obstacles, berms | 1,100 | veh_heavy | 260 | 1,050 | 16 | BLD-05 | Tanks, AT | Obstacles, mines |
| IA-ARM-14 | Tracked Signal Jammer | VEH | VEH-EW | Ext | Jamming | 440 | veh_light | 320 | 1,050 | 15 | BLD-05 | Emitter hunters, raids | Radar, guided weapons |
| IA-ARM-15 | Armored Heavy Gun Emplacement Carrier | VEH | VEH-HEAVY | Cond (cap 2) | Deployable heavy gun | 1,800 | veh_heavy / str_hardened | 180 | 3,400 | 48 | BLD-05 | Aircraft, counter-battery | Armor, fortifications |
| IA-SUP-01 | Nightjar Recon Plane | AIR | AIR-SCOUT | Core | Unarmed opening scout | 170 | air_fixed | 1,600 | 230 | 6 | BLD-07 | AA, fighters | — |
| IA-SUP-02 | Mobile Works Command Vehicle | VEH | VEH-CONSTRUCTOR | Core (cap 2) | Forward works | 1,150 | veh_medium | 250 | 2,700 | 42 | BLD-01 | Anti-armor, aircraft | — |
| IA-SUP-03 | Forced-Priority Construction Tractor | VEH | VEH-CONSTRUCTOR | Ext | Builder | 450 | veh_light | 330 | 600 | 10 | BLD-01 | Any armed unit | — |
| IA-SUP-04 | Fuel Column Tanker | VEH | VEH-LOGISTICS | Ext | Fuel transport | 460 | veh_soft | 350 | 480 | 9 | BLD-05 | Raiders, aircraft | — |
| IA-SUP-05 | Battlefield Recovery Tractor | VEH | VEH-LOGISTICS | Core | Repair, recovery | 820 | veh_medium | 300 | 950 | 15 | BLD-05 | Any combat unit | — |
| IA-SUP-06 | Camouflaged Supply Truck | VEH | VEH-LOGISTICS | Cond (cap 3) | Covert resupply | 380 | veh_soft | 400 | 650 | 11 | BLD-05 | `DETECT.SPY` | — |
| IA-AIR-01 | Interceptor Fighter | AIR | AIR-FIGHTER | Ext | Air superiority | 400 | air_fixed | 2,500 | 2,050 | 25 | BLD-07 | SAMs | Helicopters, bombers, transports |
| IA-AIR-02 | Low-Level Attack Jet | AIR | AIR-STRIKE | Ext | Anti-armor strike | 580 | air_fixed | 1,800 | 2,150 | 27 | BLD-07 | Gun AA, MANPADS, fighters | Tanks in the open |
| IA-AIR-03 | Assault Helicopter | AIR | AIR-HELI-ATTACK | Core | Close air support | 560 | air_rotary | 760 | 1,750 | 21 | BLD-08 | AA, fighters | Tanks without AA |
| IA-AIR-04 | Night Transport Helicopter | AIR | AIR-HELI-TRANSPORT | Ext | Covert insertion | 580 | air_rotary | 720 | 1,350 | 17 | BLD-08 | Short-range AA | — |
| IA-AIR-05 | Tactical Bomber | AIR | AIR-STRIKE | Ext (cap 3) | Bombing | 950 | air_fixed | 1,500 | 2,700 | 34 | BLD-07 | Fighters, SAMs | Static bases |
| IA-AIR-06 | Gunship Escort Aircraft | AIR | AIR-HELI-ATTACK | Ext | Column escort | 720 | air_rotary | 650 | 1,900 | 24 | BLD-08 | AA, fighters | Enemy attack helicopters |
| IA-AIR-07 | Prototype Ground-Attack Aircraft | AIR | AIR-STRIKE | Cond (cap 1) | Prototype strike | 800 | air_fixed | 1,700 | 3,300 | 40 | BLD-07 | SAMs, fighters | Heavy armor, prototypes |
| IA-AIR-08 | Covert Air Courier | AIR | AIR-TRANSPORT | Cond (cap 2) | Covert delivery | 320 | air_fixed | 1,400 | 1,150 | 16 | BLD-07 | Unaligned air surveillance | — |
| IA-SEA-01 | Harbor Patrol Cutter | SEA | NAV-PATROL | Cond | Landing security | 420 | nav_light | 620 | 750 | 13 | BLD-09 | Missile boats, aircraft | Small boats |
| IA-SEA-02 | Landing Transport Craft | SEA | NAV-TRANSPORT | Cond | Landing craft | 850 | nav_light | 450 | 1,100 | 18 | BLD-09 | Armed boats, aircraft | — |
| IA-SEA-03 | Fast Raider Boat | SEA | NAV-COMBATANT | Cond | Anti-ship ambush | 380 | nav_light | 760 | 1,250 | 17 | BLD-09 | Patrol boats, aircraft | Frigates, transports |
| IA-SEA-04 | Covert Attack Submarine | SEA | NAV-SUB | Cond (cap 2) | Sea denial | 950 | nav_heavy | 330 | 2,200 | 42 | BLD-09 | ASW frigates, patrol aircraft | Transports, barges |
| IA-SEA-05 | Armored Coastal Gunboat | SEA | NAV-COMBATANT | Cond | River/shore fire | 1,500 | nav_heavy | 360 | 2,000 | 34 | BLD-09 | Submarines, anti-ship missiles | Riverbank forces |
| IA-SEA-06 | Auxiliary Seaborne Headquarters | SEA | NAV-TRANSPORT | Cond (cap 1) | Temporary maritime hub | 2,200 | nav_heavy | 300 | 2,900 | 46 | BLD-09 | Submarines, strike aircraft | — |

## 9. Abilities and upgrades index

Shared abilities (`ability.shared.*`) are defined once in the framework and the sibling files; this index lists Ascendancy-specific abilities and technologies.

| ID | Unit(s) | Kind | Cost | Cooldown | Range | Requires | Counters |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.ia.inf_01.no_retreat_order` | IA-INF-01 | Active, self-endangering | free (+20 % damage taken, −25 resolve after) | 45 s | self | Steady or Shaken | HMG, HE during the window |
| `ability.ia.inf_02.breach_and_clear` | IA-INF-02 | Active | free | 30 s | 150 cm | Garrisoned target | Suppression during the 3 s channel |
| `ability.ia.inf_03.smoke_volley` | IA-INF-03 | Active | free | 35 s | 750 cm | — | Wait out 20 s |
| `ability.ia.inf_03.danger_close` | IA-INF-03 | Active, self-endangering | 30 % self-blast | 40 s | self | — | Pull back |
| `ability.ia.inf_04.ambush_volley` | IA-INF-04 | Passive | free | 10 s Concealed | weapon | Concealed | Detection |
| `ability.ia.inf_05.fire_barrier` | IA-INF-05 | Active | 5 ammo | 30 s | 350 cm | — | Vehicles, waiting |
| `ability.ia.inf_05.tank_rupture` | IA-INF-05 | Passive risk | — | — | 200 cm | Death | Kill it near allies |
| `ability.ia.inf_06.heal` | IA-INF-06 | Active / automated | free | — | 300 cm | — | Kill the medic |
| `ability.ia.inf_06.combat_stimulant` | IA-INF-06 | Upgrade-unlocked, with cost | −15 resolve, −5 % HP after | 40 s | 300 cm | `tech.ia.combat_pharmacology` | Hit during the crash |
| `ability.ia.inf_07.clear_obstacle` | IA-INF-07 | Active | free | — | 150 cm | — | Covering fire |
| `ability.ia.inf_07.build_cover` | IA-INF-07 | Active | $60 | 30 s | 200 cm | Map cover (U-08) | HE |
| `ability.ia.inf_08.traveller_cover` | IA-INF-08 | Active disguise | free | 30 s | self | — | `DETECT.SPY`, police |
| `ability.ia.inf_08.spot` | IA-INF-08 | Active | free | 25 s | sight | — | Suppression |
| `ability.ia.inf_09.infiltrate_intel` | IA-INF-09 | Active | free | 60 s | contact | `tech.ia.espionage_tier_1` | `DETECT.SPY` |
| `ability.ia.inf_09.nuclear_infiltration` | IA-INF-09 | Conditional (E gate) | agent consumed | once per flag | contact | `tech.ia.espionage_tier_3`, real AC/RD facility | Detection, false sites |
| `ability.ia.inf_10.contaminate_facility` | IA-INF-10 | Active (E effect) | $400 | 90 s | contact | `tech.ia.espionage_tier_2`, IA-DEF-12 | Decontamination, shutdown, `DETECT.SPY` |
| `ability.ia.inf_11.intimidate` | IA-INF-11 | Active channel (E predicate) | free | 60 s / 20 s | 600 cm | Eligible, isolated, broken | Regroup, interrupt, withdraw |
| `ability.ia.inf_11.coercive_presence` | IA-INF-11 | Passive | free | — | 600 cm | Isolated enemies | End isolation |
| `ability.ia.inf_12.detain` | IA-INF-12 | Active | free | 20 s | 150 cm | Revealed covert unit | Escorts |
| `ability.ia.inf_12.checkpoint` | IA-INF-12 | Active | free | 30 s | self | — | Avoid it |
| `ability.ia.inf_13.mark_target` | IA-INF-13 | Active | free | 35 s | 1500 cm | — | Kill or jam the raider |
| IA-INF-13 talents (12 options) | IA-INF-13 | Talent (U-14) | per talent | 120–240 s | radius 1100 cm | Approval (talent doc §5) | Jamming, marksmen |
| `ability.ia.inf_14.courier_drop` | IA-INF-14 | Active | free | 20 s | 150 cm | — | Interception |
| `ability.ia.arm_02.assault_dismount` | IA-ARM-02 | Active | free | 30 s | self | Loaded | Prepared defenders |
| `ability.ia.arm_03.overdrive` | IA-ARM-03 | Active, self-endangering | 1.5 % HP/s, slow after | 50 s | self | — | Catch it slowed |
| `ability.ia.arm_04.ammunition_select` | IA-ARM-04 | Toggle | free | 3 s | self | — | Mixed forces |
| `ability.ia.arm_05.press_the_breach` | IA-ARM-05 | Active, self-endangering | gun cooling, +15 % damage taken | 60 s | self | — | Counter-attack in the window |
| `ability.ia.arm_06.maintenance_burden` | IA-ARM-06 | Passive cost | — | — | — | 180 s combat time | Keep it from repair |
| `ability.ia.arm_06.fuel_starved_penalty` | IA-ARM-06 | Passive cost | — | — | — | Petroleum 0 | Fuel interdiction |
| `ability.ia.arm_07.ram_breach` | IA-ARM-07 | Active, self-endangering | 8 ammo, 8 % HP | 45 s | 300 cm | Garrisoned target | AT on the immobile hull |
| `ability.ia.arm_08.barrage_mode` | IA-ARM-08 | Active | 8 s cooling | 30 s | self | — | Strike during cooling |
| `ability.ia.arm_09.hull_down` | IA-ARM-09 | Conditional | free | — | self | Hold Ground 4 s | Flanking |
| `ability.ia.arm_10.saturation_salvo` | IA-ARM-10 | Active | both salvos | resupply | weapon | Deployed | Counter-battery |
| `ability.ia.arm_11.breaching_round` | IA-ARM-11 | Active | 2 ammo | 30 s | weapon | Deployed | Concealment |
| `ability.ia.arm_13.clear_obstacle` / `.push_wreck` / `.build_berm` | IA-ARM-13 | Active | free / free / $80 | — / — / 30 s | 200–300 cm | — | Fire on the dozer |
| `ability.ia.sup_01.loiter_sortie` | IA-SUP-01 | Active | 1 kerosene | 20 s | map | — | AA |
| `ability.ia.sup_02.deploy_works` | IA-SUP-02 | Active | free | — | self | Legal site (U-IA-01) | Raids during 45 s |
| `ability.ia.sup_03.forced_priority` | IA-SUP-03 | Active, cost | −2 Population Morale | 60 s | self | — | Morale loss |
| `ability.ia.sup_06.falsified_manifest` | IA-SUP-06 | Passive disguise | — | — | — | Unrevealed | `DETECT.SPY` |
| `ability.ia.air_02.low_level_ingress` | IA-AIR-02 | Active | free | per sortie | self | — | Gun AA |
| `ability.ia.air_04.low_signature_flight` | IA-AIR-04 | Active | free | 15 s | self | — | Detection within 900 cm |
| `ability.ia.air_06.escort_column` | IA-AIR-06 | Active | free | — | group | — | AA on the column path |
| `ability.ia.air_07.long_turnaround` | IA-AIR-07 | Passive cost | 6 kerosene | 60 s rearm | — | — | Kerosene interdiction |
| `ability.ia.air_08.civil_registration` / `.air_land` | IA-AIR-08 | Passive disguise / active | free | — | ground | Unrevealed / landing zone | UN air surveillance |
| `ability.ia.sea_01.landing_screen` | IA-SEA-01 | Active | +15 % damage taken | 45 s | 600 cm | — | Anti-ship missiles |
| `ability.ia.sea_04.swimmer_delivery` | IA-SEA-04 | Active | free | — | 300 cm | Loaded | Coastal observation |
| `ability.ia.sea_06.forward_anchorage` | IA-SEA-06 | Active, temporary | free | 120 s | 600 cm | Deep water | Submarines, missiles |
| `tech.ia.espionage_tier_1` (= `advanced_clandestine_training`) | IA-INF-08/09/14, IA-SUP-06, IA-AIR-04/08 | Research | $800 | 60 s | — | IA-BLD-04 | — |
| `tech.ia.espionage_tier_2` | IA-INF-09/10 | Research | $1,400 | 120 s | — | tier 1, IA-BLD-14 | — |
| `tech.ia.espionage_tier_3` | IA-INF-09 | Research (E gate) | $2,200 | 180 s | — | tier 2 | — |
| `tech.ia.late_espionage` | IA-INF-09 | Research (I flag) | $1,200 | 90 s | — | IA-BLD-04 | — |
| `tech.ia.assault_doctrine` | IA-INF-01/02 | Research | $700 | — | — | IA-BLD-14 | trade-off: resolve loss / cooldown |
| `tech.ia.improved_launcher_rounds` | IA-INF-03 | Research | $600 | — | — | IA-BLD-14 | magazine −1 |
| `tech.ia.wire_guidance_improvement` | IA-INF-04 | Research | $650 | — | — | IA-BLD-14 | slower missile |
| `tech.ia.pressurised_projectors` | IA-INF-05, IA-ARM-07 | Research | $650 | — | — | IA-BLD-14 | less ammunition |
| `tech.ia.combat_pharmacology` | IA-INF-06 | Research | $700 | — | — | IA-BLD-14 | medic price +$60 |
| `tech.ia.sapper_training` | IA-INF-07, IA-ARM-13 | Research | $500 | — | — | IA-BLD-04 | price +$40 |
| `tech.ia.contamination_doctrine` | IA-INF-10 | Research | $1,000 | — | — | IA-BLD-14 | Unaligned incident level +1 |
| `tech.ia.coercion_methods` | IA-INF-11 | Research | $900 | — | — | IA-BLD-17 | Unaligned incident level +1 |
| `tech.ia.internal_security_apparatus` | IA-INF-12 | Research | $600 | — | — | IA-BLD-17 | Unaligned incident level +1 |
| `tech.ia.courier_networks` | IA-INF-14, IA-SUP-06, IA-AIR-08 | Research | $500 | — | — | IA-BLD-04 | courier cap −1 |
| `tech.ia.smoke_dischargers` | IA-ARM-01/03/04 | Research | $400 | — | — | IA-BLD-05 | price +$30 |
| `tech.ia.armored_bay_covers` | IA-ARM-02 | Research | $500 | — | — | IA-BLD-05 | slower, slower dismount |
| `tech.ia.uprated_engines` | IA-ARM-03 | Research | $800 | — | — | IA-BLD-06 | more self-damage |
| `tech.ia.composite_armor_upgrade` | IA-ARM-04/05 | Research | $1,000 | — | — | IA-BLD-06 | −5 % speed, +1 petroleum |
| `tech.ia.prototype_program` | IA-ARM-06/15, IA-AIR-07 | Research | $2,000 | 150 s | — | IA-BLD-06 | lane block |
| `tech.ia.tungsten_penetrators` | IA-ARM-09 | Research | $900 | — | — | IA-BLD-14 | price +$100, slower fire |
| `tech.ia.extended_range_rockets` | IA-ARM-10 | Research | $800 | — | — | IA-BLD-14 | more scatter |
| `tech.ia.fire_direction_center` | IA-ARM-10/11/15, IA-SEA-05 | Research | $900 | — | — | IA-BLD-14 | no unobserved fire |
| `tech.ia.reinforced_bunker_walls` | IA-ARM-12 | Research | $600 | — | — | IA-BLD-06 | slower deploy |
| `tech.ia.armored_fuel_bowsers` | IA-SUP-04 | Research | $500 | — | — | IA-BLD-05 | −10 % speed |
| `tech.ia.long_range_tanks` | IA-SUP-01 | Research | $400 | — | — | IA-BLD-07 | kerosene 1 → 2 |
| `tech.ia.extended_hardpoints` | IA-AIR-02 | Research | $700 | — | — | IA-BLD-07 | kerosene +1 |
| `tech.ia.helicopter_armor` | IA-AIR-03/06 | Research | $700 | — | — | IA-BLD-08 | −8 % speed |
| `tech.ia.heavy_bomb_load` | IA-AIR-05 | Research | $900 | — | — | IA-BLD-07 | −10 % speed |
| `tech.ia.quiet_propulsion` | IA-SEA-04 | Research | $900 | — | — | IA-BLD-14 | −10 % speed |

## 10. Faction tactical coverage, gaps and balance risks

**Coverage per role.**

| Role | Coverage | Units |
| --- | --- | --- |
| Line infantry | Good, expensive per body | IA-INF-01/02/12 |
| Anti-armor | Excellent, guided and gun-based | IA-INF-04, IA-ARM-04/09/15, IA-AIR-02/03/07 |
| Anti-infantry and anti-garrison | Excellent | IA-INF-02/03/05, IA-ARM-05/07/10, IA-AIR-05 |
| Anti-air | Adequate, gun-heavy; no SAM vehicle | IA-ARM-08, IA-ARM-06 (rotary only), IA-AIR-01/06, IA-SEA-05/06 |
| Artillery and siege | Excellent | IA-ARM-05/10/11/15, IA-SEA-05 |
| Recon and detection | Adequate early (plane), good with IA-INF-08; `DETECT.SPY` only from IA-INF-12 | IA-SUP-01, IA-ARM-01, IA-INF-08/12/13 |
| Logistics and recovery | Good but fuel-fragile | IA-SUP-04/05/06, IA-AIR-08, IA-SEA-06 |
| Engineering | Good | IA-INF-07, IA-ARM-13, IA-SUP-03 |
| Espionage and coercion | Distinctive: limited spying, exclusive Intimidation and contamination | IA-INF-09/10/11/14 |
| Naval | Conditional, complete | IA-SEA-01 to 06 |

**Weak spots and intended answers.**

1. *No radar SAM vehicle.* Intended: high-altitude jets are answered by IA-AIR-01 and by static IA-DEF-04/05. A Concord or Directorate strike-jet opening is a real risk (E-consistent: advanced aviation exists, but the Ascendancy pays for every layer). Consider an Extended upgrade path for IA-ARM-08 if playtests show bombers dominate (open: **U-IA-06**).
2. *Thin infantry reserves.* Intended by canon (E: shortages of infantry reserves in prolonged defense). Answer: IA-ARM-12 bunkers and IA-ARM-15 emplacements hold ground with few men; IA-INF-06 keeps squads alive.
3. *Fuel fragility.* Heavy hulls pay +30 % petroleum and IA-ARM-06 doubles its starvation penalty. Answer: IA-SUP-04 convoys, IA-DEF-07 caches, IA-BLD-12 storage, escorts. This is the designed counter-play for Meridian raiding and Directorate diversion.
4. *Spy detection is single-sourced* (IA-INF-12). Concord and Directorate agents are a real threat to IA-BLD-15/16. Answer: enforcer patrols and checkpoints; IA-DEF structures.

**Balance risks.**

- The +8 % faction modifier stacks with self-endangering windows (*No Retreat Order*, *Press the Breach*, *Concentrated Breakthrough*). The §18.4 order applies the faction modifier once; each window must remain a cooldown buff, never a damage multiplier on top of veterancy beyond the clamp. Playtest the IA-ARM-04 versus RD-ARM-04 ratio (≈ 1.25 DPS ratio at 1.3× price) first.
- Intimidation could snowball against Meridian raiders, which operate in small groups. Mitigations: cap 2 officers, a 15 s channel interruptible by any 10 % hit, eligibility excluding medium and heavy armor, and isolation computed from actual positions (E). Track conversion rate per match in telemetry.
- Contamination could lock a single-producer opponent out of production. Mitigations (E: recoverable): the 120 s status, a chain cap of 3, decontamination and shutdown counterplay, and worker replacement. Do not raise the chain cap before telemetry.
- IA-ARM-06 plus IA-AIR-06 escort could become a "death ball". The answer is aircraft and fuel interdiction; keep IA-ARM-06 at cap 1 and `maintenance_burden` active.

**Open items.** Framework U-04 (sortie model), U-05/06 (food and fuel penalties), U-07 (death blasts), U-08 (map cover for *Build Cover* and *Build Berm*), U-10 (refunds), U-11 (heavy-armor recovery, adopted here), **U-14 (IA-INF-13 commander approval: without it the Ascendancy has no talent appointment)**. Faction items:

| ID | Decision | Recommendation |
| --- | --- | --- |
| U-IA-01 | IA-SUP-02 deploy target under the fixed-base prototype default | Deploy into IA-BLD-02 Occupation Command Post only; it inherits the post's Unaligned jurisdiction risk |
| U-IA-02 | Intimidation channel rate, range and neutral-civilian eligibility | 8 resolve per second, 600 cm, 15 s cap; neutral Unaligned civilians ineligible unless a scenario declares them hostile-eligible |
| U-IA-03 | Contamination parameters (staff loss per step, chain radius, depth and cap) | 15 % per 30 s, 1500 cm, depth 2, 3 buildings, 120 s; owner decision under Espionage §1.3 |
| U-IA-04 | Wire-guided ATGM immunity to jamming (IA-INF-04) | Adopt: it differentiates the Ascendancy from radio-guided Concord missiles and gives a reason to choose infantry AT over vehicles |
| U-IA-05 | IA-INF-13 fallback if commander approval is refused | Keep the unit as an Elite raider with ordinary veterancy and *Mark Target* only |
| U-IA-06 | A radar SAM option for the Ascendancy | Defer; revisit after the Core air playtest |
| U-IA-07 | IA-SEA-06 anchorage duration and producer list | 240 s anchored, 120 s cooldown, infantry and small boats only |
| U-IA-08 | Heavy-hull surcharge values (+30 % petroleum, +25 % repair) | Adopt as the explicit implementation of "costly maintenance" (E); tune by telemetry |
