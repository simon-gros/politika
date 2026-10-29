# POLITIKA — Meridian Pact Unit Specifications

**Status:** PROPOSED unit gameplay specification (29 October 2026). No unit in this file is implemented. Numbers are Provisional Baseline (PB) values.
**Authority:** names and IDs come from the [Five-Faction Rosters](../POLITIKA_FIVE_FACTION_ROSTERS.md); shared rules from the [Unit Specification Framework](../UNIT_SPECIFICATION_FRAMEWORK.md); faction canon from the [faction description](../../Factions/TheMeridianPact/description.md). Owner documents win on conflict.
**Evidence labels:** I / E / E-R / P / U as in framework §1.1. A field without a label is P. Cross-faction views: [Unit Balance and Ability Matrices](../UNIT_BALANCE_AND_ABILITY_MATRICES.md). Coverage: [Unit Specification Coverage Report](../Reports/UNIT_SPECIFICATION_COVERAGE_20261029.md).
**Siblings:** [Atlantic Concord](ATLANTIC_CONCORD_UNITS.md) · [Red Directorate](RED_DIRECTORATE_UNITS.md) · [Iron Ascendancy](IRON_ASCENDANCY_UNITS.md) · [The Unaligned](UNALIGNED_UNITS.md).

## 1. Faction doctrine and faction-wide unit rules

The Pact wins through adaptability, mobility, terrain knowledge and the integration of diverse equipment rather than one class of superior hardware or industrial mass (E: faction description, *Military Doctrine* and *Gameplay*). Motorized infantry and lightly armored reconnaissance give flexibility; mobile artillery, portable anti-armor weapons and short-range air defense let dispersed formations contest ground; engineers keep crossings and supply routes open (E). It is strongest in maneuver, intelligence gathering, ambush, raids and supply disruption, and weakest in heavy industrial concentration, equipment standardization and coordinating large formations (E: *Unconventional Warfare*). Its economic identity is commerce: contracts, mercenaries, fuel-field proximity and a conventional-only Black Market.

| Rule | Unit-level implementation | Label |
| --- | --- | --- |
| Best early reconnaissance | `MP-ARM-01` Roadrunner Scout Truck: the cheapest and fastest scout in the game, lightly armed and armored. Validator: it has no sabotage ability (E) | E |
| Best early conventional sabotage | Qualified **non-spy** units `MP-INF-08` and `MP-INF-09` plant fictional attached charges from the start of the match, with no espionage research, at **$120 per charge** (other factions $300 through spies, E: more costly). No other Pact unit has the ability unless a unit definition grants it (E) | E (rule), P (values, I predicate `CanPlantExplosives` Meridian at any time) |
| Scout and light-truck prices | At the **lower band edge** (§4.4). Rifle infantry is **not cheaper** than the Directorate equivalent (E-R); every Pact infantry role price also satisfies the Directorate validator (Directorate price × 1.5 ≤ Pact price) | E-R, P |
| Discipline | Ordinary group support only; **no very-massive-army bonus** (E); individuals and very small detachments suffer the **normal isolation penalty** (E) | E (rule), P (values) |
| No advanced propaganda | No propaganda abilities or tree; Population Morale still exists and is driven by trade, fuel supply, contracts, protection of forces and battle outcomes (E) | E |
| Contracts | Contract units carry `ownerFaction`, `employer`, `contractExpiryStep` and `wagePerInterval` (E-R); wages are USD per 30 s interval, debited once; on expiry or non-payment the unit stops accepting orders and leaves without refund (framework §11.4). Contract units: MP-INF-02, MP-INF-11, MP-AIR-02, MP-AIR-06, MP-SEA-03, MP-MKT-07 | E-R (metadata), P (values) |
| Black Market | `MP-MKT-*` items are bought at `MP-BLD-15` against finite auction stock and arrive physically by MP-SUP-02, MP-SUP-09 or MP-SEA-01, so they can be interdicted (E). **Never nuclear** (E). A market unit never unlocks a native factory (E-R) | E |
| Captured equipment | `MP-ARM-13` is market-only or battlefield-captured; never produced in a Pact factory (E-R). Ownership records `sourceFaction` and `acquisitionMethod` (E-R) | E-R |
| Nuclear | No refining, research, power or weapons (E). Pact troops may seize and, with compatible captured delivery capability, deploy a **completed enemy weapon** under the nuclear contract; this never grants production (E) | E |
| Facility espionage exposure | No building-espionage specialty of its own; defends against Psy-Ops, diversion and contamination through audits, security and restoration (E) | E |
| Fuel | Starts near a crude-oil or natural-gas field (proximity, not ownership or cheaper fuel, E). Fuel classes per §4.5, no surcharge | E, P |
| Visual language | Tan, olive and desert mixed kit, shemaghs and boonie hats, a green faction emblem, mixed-origin weapons; vehicles in sand and olive with painted commercial markings; Mediterranean coastal backdrops; relaxed, multilingual voice set | P (from concept art) |

**C&C design lessons applied.** The roster is built on the GLA pattern: fast technicals and scout buggies that own the early map (Generals Technical, TW Pitbull), cheap rocket and recoilless carriers that hit and run, quad-gun AA technicals that double as anti-infantry (Generals Quad Cannon), salvage and captured equipment (Generals salvage crates, as an explicit acquisition rule), and early demolition sabotage (Generals Terrorist and Demo Trap, without suicide attacks). It also borrows the RA1 minelayer and the RA2 Gap Generator as **non-magical** mines and smoke. Lessons avoided: free salvage upgrades from every kill (Pact recovery is an explicit, slow tow), unlimited suicide units (no Pact unit dies to deliver damage; charges are attached), and stealth that cannot be found (mines, decoys and smoke all have tells).

## 2. Core roster selection

Core (16 units):

| ID | Role covered | Reason |
| --- | --- | --- |
| MP-INF-01 | Line infantry | Flexible regional rifle company |
| MP-INF-03 | Anti-armor infantry | Cheap rocket ambush |
| MP-INF-05 | Engineer / capture | Crossings, roads and capture |
| MP-INF-06 | Medic | Sustains raids far from base |
| MP-INF-08 | Faction signature: early sabotage | Defining non-spy saboteur (E) |
| MP-INF-10 | Specialist / ambush | Terrain ambush and withdrawal |
| MP-ARM-01 | Opening scout (E) | Best early reconnaissance |
| MP-ARM-02 | Light vehicle | Modular technical |
| MP-ARM-03 | Transport | Light armored van |
| MP-ARM-06 | Main battle tank | Surplus imported armor |
| MP-ARM-07 | Tank destroyer | Fast recoilless hit-and-run |
| MP-ARM-08 | Artillery | Cheap rocket rack |
| MP-ARM-09 | Mobile air defense | Quad AA technical |
| MP-SUP-01 | Constructor | Dispersed outposts |
| MP-SUP-08 | Repair | Keeps imported vehicles running |
| MP-AIR-02 | Air support | Hired escort helicopter |

**Extended:** MP-INF-04, MP-INF-07, MP-INF-09, MP-INF-12, MP-INF-13 (commander, pending U-14), MP-ARM-04, MP-ARM-05, MP-ARM-10, MP-ARM-11, MP-ARM-12, MP-SUP-02 to MP-SUP-07, MP-AIR-01, MP-AIR-03 to MP-AIR-05.
**Conditional:** MP-INF-02 and MP-INF-11 (contract), MP-ARM-13 (market or battlefield capture), MP-SUP-09 (market delivery), MP-AIR-06 (contract), MP-SEA-01 to MP-SEA-07 (authored water layer and MP-BLD-08; MP-SEA-04 also on river maps), MP-MKT-01 to MP-MKT-09 (Black Market stock).
**Scenario:** none.

## 3. Infantry, contractors and raiders

### MP-INF-01 — Regional Rifle Company

| Field | Specification |
| --- | --- |
| **Identity** | `unit.mp.inf_01` · Meridian Pact · Infantry / line rifle squad · `TPL-INF-LINE` · **Core** · Playable |
| **Role** | **Primary:** flexible, inexpensive field rifle unit; not cheaper than the Directorate equivalent (E-R). **Secondary:** dispersed outpost garrison; screening for raiding vehicles. **Doctrine fit:** regional formations that accommodate different military traditions and equipment (E). |
| **Inspiration and background** | Regional infantry with a mix of Western and Eastern rifles and a light machine gun. In-universe, the company is raised by member states and serves under Pact coordination. C&C lineage (E-R): Rebel (Gen); Rifle Infantry (RA1); Militant Squad (TW). |
| **Visual identity** | Concept: [`MP-INF-01.png`](../../Factions/TheMeridianPact/Units/MP-INF-01.png). Five members in mixed tan and olive kit, boonie hats and shemaghs, assorted rifles; green emblem patch. |
| **Statistics** | 5 × 55 HP = **275** (`armor.inf_light`) · FOOT **205 cm/s** · F0 · sight 1200 cm · 1 slot · food staff 5 · fuel none · ExperienceValue 28 |
| **Commands** | Move, Attack, AttackMove, Stop, Guard, Grenade, Reload, Sprint (I) · Garrison, Patrol, Withdraw, Scatter (P) · default stance Aggressive · formation role `frontline` |
| **Production** | **$280** (band low-middle; ≥ RD-INF-01 $175 × 1.5 = $263, E-R and validator) · `MP-BLD-03` Regional Training Compound · prerequisites: `MP-BLD-01` · **8 s** · 5 food · no cap |
| **Veterancy** | Standard |
| **AI** | `ai.unit.line_infantry`. Deltas: prefer garrisons and rough terrain; avoid open ground against armor |
| **Multiplayer** | MPA-STD |
| **Presentation** | ANIM-INF plus reload, throw, prone, garrison. Mixed rifle reports (two weapon sounds); icon of a rifle with a green bar |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.mp.inf_01.primary` mixed rifles + LMG | SMALL_ARMS | 0–650 cm | 9 per member / 1.0 s (45 DPS at 5 members); magazine 30, reload 3 s (I values) | hitscan | 3 | GROUND, AIR_LOW | 100 % / 15 % / 2 % / 25 % rotary / 8 % |
| `ability.shared.grenade` (I) | HIGH_EXPLOSIVE | 650 cm | 50, 150 cm | lobbed | 8 | GROUND, STRUCTURE | 110 % / 45 % / 12 % / — / 100 % |

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.shared.grenade`, `.sprint`, `.reload` | Active (I) | as framework §2 | — | I values | — | standard | standard |
| `ability.mp.inf_01.fieldcraft` | Conditional (P; U-03 terrain) | In forest or rough terrain the squad becomes Concealed after 3 s stationary (§9.1) and takes −15 % HIGH_EXPLOSIVE damage | Automatic | free · while in terrain · — · self | `DETECT.CONCEALED`; firing reveals for 4 s; no effect on open or urban ground | Own-only concealment icon | Hold in tree lines when outnumbered |

**Research and upgrades:** `tech.shared.infantry_protective_equipment` (55 → 65, −5 % speed). `tech.mp.modular_commercial_equipment` (E: catalogue; faction): see vehicles. `tech.mp.regional_drill` (MP-BLD-03, $500): magazine reload 3 → 2.5 s; trade-off: price +$25.

**Counters:** *Beats (hard):* agents and unarmed support. *Beats (soft):* Directorate platoons in cover, militia and contract guards. *Beaten by (hard):* HMG (RD-INF-03), flame (IA-INF-05, RD-ARM-10), grenadiers (AC-INF-04, IA-INF-03), artillery. *Beaten by (soft):* Concord and Ascendancy sections one-on-one. *Situational:* cheap, numerous and flexible; holds dispersed outposts.

**Status:** identity and "not cheaper than Directorate" E-R; grenade, sprint, reload and orders I (generic infantry behavior); all values P.

### MP-INF-02 — Contracted Guard Detail

| Field | Specification |
| --- | --- |
| **Identity** | `unit.mp.inf_02` · Meridian Pact · Infantry / contract guards · `TPL-INF-LINE` · **Conditional** (contract) · Playable |
| **Role** | **Primary:** time-limited hired defense with explicit owner, wage and expiry (E-R). **Secondary:** convoy and depot protection. **Doctrine fit:** contracting and mercenary brokerage (E). |
| **Inspiration and background** | Private security guards with rifles and body armor under a commercial contract. C&C lineage (E-R): Ranger (Gen); GDI Soldier (Ren); Devout (TT). |
| **Visual identity** | Concept: [`MP-INF-02.png`](../../Factions/TheMeridianPact/Units/MP-INF-02.png). Grey-tan uniforms, vests, company armbands instead of the Pact emblem. |
| **Statistics** | 4 × 75 HP = **300** (`armor.inf_protected`) · FOOT 200 cm/s · F0 · sight 1200 cm · 1 slot · food staff 4 · ExperienceValue 25 |
| **Commands** | as TPL-INF-LINE + Hold Post (P) · default stance Guard · formation role `rear_guard` |
| **Production** | Hire **$250** + wage **$15 per interval** · contract **300 s** (renewable once for the hire fee) · `MP-BLD-16` Mercenary Recruitment Office · prerequisites: `MP-BLD-13` · **6 s** · 4 food · no cap |
| **Veterancy** | Standard (kept for the contract's life only) |
| **AI** | `ai.unit.line_infantry`. Deltas: never leave 1200 cm of the assigned post; withdraw at 50 % HP (paid guards do not die for a cause) |
| **Multiplayer** | MPA-STD + MPA-ECON (contract state). Unpaid status visible only to the employer (framework §11.4) |
| **Presentation** | ANIM-INF. Contract timer on the unit card (owner); icon of a shield with a coin |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.mp.inf_02.primary` rifles | SMALL_ARMS | 0–650 cm | 10 per member / 1.0 s | hitscan | 3 | GROUND, AIR_LOW | 100 % / 15 % / 2 % / 25 % rotary / 8 % |

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.mp.inf_02.hold_post` | Conditional (P) | While within 600 cm of an assigned friendly structure or vehicle: +20 % damage and +10 maximum resolve | Assigned post | free · while near · — · 600 cm | Lure them away; artillery on the post | Post link (owner) | Always assign to the protected asset |
| `ability.mp.contract_terms` | Passive (E-R metadata; P values) | Wage, expiry, renewal and departure per framework §11.4; 60 s warning before expiry | Automatic | wage per interval | Drain the employer's USD | Contract timer | — |

**Research and upgrades:** `tech.mp.contract_arbitration` (MP-DEF-09 operational, $500): contract length 300 → 420 s; trade-off: wage +$5.

**Counters:** *Beats (soft):* raids on depots, saboteurs (AC-INF-11, RD-INF-11 when revealed). *Beaten by (hard):* HMG, flame, artillery, armor. *Situational:* rent defense for an expansion instead of building a permanent garrison.

**Status:** identity and contract metadata E-R; values P.

### MP-INF-03 — Light Rocket Team

| Field | Specification |
| --- | --- |
| **Identity** | `unit.mp.inf_03` · Meridian Pact · Infantry / anti-armor · `TPL-INF-AT` · **Core** · Playable |
| **Role** | **Primary:** portable anti-armor ambush and defense (E-R). **Secondary:** anti-structure; anti-boat from banks. No AA mode. **Doctrine fit:** portable anti-armor weapons that let dispersed formations contest ground (E). |
| **Inspiration and background** | Two-to-three-person team with an unguided rocket launcher and spare rockets. C&C lineage (E-R): RPG Trooper (Gen); Tankbuster (RA3); Rocket Soldier (TD). |
| **Visual identity** | Concept: [`MP-INF-03.png`](../../Factions/TheMeridianPact/Units/MP-INF-03.png). Launcher with a rocket bag, shemaghs, tan kit. |
| **Statistics** | 3 × 60 HP = **180** (`armor.inf_light`) · FOOT **210 cm/s** · F0 · sight 1200 cm · 1 slot · food staff 3 · ExperienceValue 46 |
| **Commands** | as TPL-INF-AT + Conceal (P) · default stance Guard · formation role `support_weapon` |
| **Production** | **$460** (band low edge would be $340; the Directorate validator requires ≥ RD-INF-04 $300 × 1.5 = $450) · `MP-BLD-03` · **9 s** · 3 food · no cap |
| **Veterancy** | Standard |
| **AI** | `ai.unit.anti_armor_infantry`. Deltas: fire, then *Shoot and Scoot* to new cover |
| **Multiplayer** | MPA-STD |
| **Presentation** | ANIM-INF + launcher; backblast VFX. Icon of a rocket |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.mp.inf_03.primary` unguided rocket launcher | SHAPED_CHARGE | 0–800 cm | 115 / 4.0 s; magazine 4, reload 6 s | shell_direct, 1200 cm/s, tracking slow | 3 | GROUND, STRUCTURE, WATER_SURFACE | 25 % (never auto) / 100 % / 90 % / — / 70 % |

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.shared.conceal` | Conditional (P) | Concealed after 3 s stationary | Self | free | `DETECT.CONCEALED`; firing reveals 4 s | Own-only | Default on Hold Ground |
| `ability.mp.inf_03.shoot_and_scoot` | Passive (P) | After each rocket, +25 % speed for 3 s | Automatic | free · 3 s · — · self | HMG covering the escape lane | Dust kick | Move 200–300 cm laterally after each shot |

**Research and upgrades:** `tech.mp.tandem_rockets` (MP-BLD-03, $600): damage 115 → 130 vs VEH_HEAVY only; trade-off: magazine 4 → 3.

**Counters:** *Beats (hard):* light and medium vehicles, APCs (AC-ARM-02, RD-ARM-03, IA-ARM-02/03). *Beats (soft):* MBTs from the side or in numbers. *Beaten by (hard):* HMG, flame, grenadiers, artillery, any helicopter (no AA). *Situational:* ambush on mountain roads; slow rockets miss fast vehicles (tracking slow, §6.6).

**Status:** identity E-R; values P. Price is set by the Directorate validator, not the band (U-MP-01).

### MP-INF-04 — Hillside Mortar Team

| Field | Specification |
| --- | --- |
| **Identity** | `unit.mp.inf_04` · Meridian Pact · Infantry / mortar · `TPL-INF-HW` · **Extended** · Playable |
| **Role** | **Primary:** indirect support in rough terrain (E-R). **Secondary:** smoke for raids. **Doctrine fit:** mountain and rough-terrain operations (E). |
| **Inspiration and background** | Light mortar team with base plate and bipod carried by three soldiers. C&C lineage (E-R): Grenadier (RA1); Disc Thrower (TS); Mortar Cycle (Uprising, fire role). |
| **Visual identity** | Concept: [`MP-INF-04.png`](../../Factions/TheMeridianPact/Units/MP-INF-04.png). Mortar tube and base plate on packs. |
| **Statistics** | 3 × 55 HP = **165** (`armor.inf_light`) · FOOT_HEAVY **185 cm/s**; **mountain-trained:** rough ×1.0 and steep ×0.75 instead of 0.75/0.50 · F0 · sight 1200 cm · 1 slot · food staff 3 · ExperienceValue 52 |
| **Commands** | as TPL-INF-HW + Fire Mission (P) · default stance Hold Ground when deployed · formation role `support_weapon` |
| **Production** | **$520** (≥ RD-INF-05 $340 × 1.5 = $510) · `MP-BLD-03` · **11 s** · 3 food · no cap |
| **Veterancy** | Standard; scatter bonuses apply |
| **AI** | `ai.unit.support_weapon` + artillery rules. Deltas: deploy on high ground; fire only with observation |
| **Multiplayer** | MPA-STD + MPA-INDIRECT |
| **Presentation** | ANIM-INF + deploy and drop-fire loop. Icon of a mortar |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.mp.inf_04.primary` light mortar | HIGH_EXPLOSIVE | **600**–2000 cm | 45 / 3.5 s; AmmoCapacity 30 | lobbed; full 100 cm, max 200 cm, edge 30 %; scatter 80–250 cm | 8 | GROUND, STRUCTURE | 110 % / 45 % / 12 % / — / 100 % |

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.shared.deploy` | Active (P) | 2 s deploy, 2 s undeploy; fires only deployed | Self | free | Catch it moving | Base-plate drop | Deploy behind a crest |
| `ability.mp.inf_04.smoke_rounds` | Active (P) | Lays a 400 cm smoke cylinder for 20 s at range | Ground point | 2 ammo · 20 s · 30 s · 600–2000 cm | — | Smoke VFX | To cover a raid or a withdrawal |

**Research and upgrades:** `tech.shared.supply_preservation`. `tech.mp.mountain_observers` (MP-BLD-03, $400): observed scatter ×0.7 → ×0.6 when the observer stands on higher ground (U-03); trade-off: price +$30.

**Counters:** *Beats (hard):* infantry in cover and garrisons (AC-INF-01/02, RD-INF-01/03). *Beaten by (hard):* raiders and light vehicles, marksmen (AC-INF-08, IA-INF-08), counter-battery. *Situational:* the Pact's cheapest indirect fire in hills.

**Status:** identity E-R; values P.

### MP-INF-05 — Bridge and Road Engineer

| Field | Specification |
| --- | --- |
| **Identity** | `unit.mp.inf_05` · Meridian Pact · Infantry / engineer · `TPL-INF-SUPPORT` · **Core** · Playable |
| **Role** | **Primary:** repair crossings, roads, fuel and merchant infrastructure (E-R). **Secondary:** structure capture; field repair. **No sabotage charges** (that is MP-INF-08/09). **Doctrine fit:** engineering detachments that establish crossings and maintain supply routes (E). |
| **Inspiration and background** | Civil-military engineering pair with tools, a survey kit and sandbags. C&C lineage (E-R): Engineer (RA1); Worker (Gen); Combat Engineer (RA3). |
| **Visual identity** | Concept: [`MP-INF-05.png`](../../Factions/TheMeridianPact/Units/MP-INF-05.png). Hard hats over shemaghs, tool belts. |
| **Statistics** | 2 × 60 HP = **120** (`armor.inf_light`) · FOOT 205 cm/s · F0 · sight 1000 cm · 1 slot · food staff 2 · ExperienceValue 38 |
| **Commands** | Move, Stop, Guard (I) · Capture, Repair, Restore Crossing (P) · default stance Hold Fire · formation role `protected` |
| **Production** | **$380** (≥ RD-INF-06 $240 × 1.5 = $360) · `MP-BLD-03` · **11 s** · 2 food · no cap |
| **Veterancy** | Standard (armed) |
| **AI** | `ai.unit.support_infantry`. Deltas: restore destroyed bridges on owned supply routes first; repair fuel structures second |
| **Multiplayer** | MPA-STD; crossing state is authoritative map state |
| **Presentation** | ANIM-INF + work loop. Icon of a bridge with a wrench |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.mp.inf_05.primary` carbines | SMALL_ARMS | 0–550 cm | 7 per member / 1.0 s | hitscan | 2 | GROUND | 100 % / 15 % / 2 % / — / 8 % |

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.shared.capture_structure` | Active (P) | 8 s capture (§10.6) | Eligible structure | free · 8 s | Damage interrupts | Capture bar | On order |
| `ability.shared.repair_structure` | Active (P) | §10.2 rate; **fuel and logistics structures repair at +50 %** | Friendly structure ≤ 300 cm | USD proportional | — | Sparks | Under Guard |
| `ability.mp.inf_05.restore_crossing` | Active (P) | Restores a destroyed bridge or road segment to passable (F2 capacity) in 20 s; Unaligned bridges need permission (E) | Destroyed crossing ≤ 200 cm | $150 · 20 s · — · 200 cm | Fire on the work site | Scaffold mesh | On owned supply routes |

**Research and upgrades:** `tech.mp.field_engineering` (MP-BLD-03, $500): *Restore Crossing* 20 → 14 s and capacity F3; trade-off: cost $150 → $200.

**Counters:** *Beaten by (hard):* any armed unit. *Situational:* reopens the logistics network after enemy bridge demolition; keeps fuel flowing.

**Status:** identity E-R; jurisdiction permission E; values P.

### MP-INF-06 — Portable Field Medic

| Field | Specification |
| --- | --- |
| **Identity** | `unit.mp.inf_06` · Meridian Pact · Infantry / medic · `TPL-INF-SUPPORT` · **Core** · Playable |
| **Role** | **Primary:** sustain raiding infantry far from fixed bases (E-R). **Secondary:** field replenishment. **Doctrine fit:** maintaining operations under difficult logistical conditions (E). |
| **Inspiration and background** | Medical pair with field kits and folding stretchers. C&C lineage (E-R): Field Medic (TS); Combat Engineer (KW, support); Ambulance (Gen). |
| **Visual identity** | Concept: [`MP-INF-06.png`](../../Factions/TheMeridianPact/Units/MP-INF-06.png). Green crescent-and-cross neutral medical armband, tan kit. |
| **Statistics** | 2 × 55 HP = **110** (`armor.inf_light`) · FOOT **220 cm/s** · F0 · sight 1000 cm · 1 slot · food staff 2 · no ranks |
| **Commands** | Move, Stop, Guard (I) · Heal, Field Replenishment (P) · default stance Hold Fire · formation role `protected` |
| **Production** | **$360** · `MP-BLD-03` · **10 s** · 2 food · no cap |
| **Veterancy** | none |
| **AI** | `ai.unit.support_infantry`. Deltas: follow raiding groups; replenish only when no hostile is observed within 1200 cm |
| **Multiplayer** | MPA-STD; replenishment debits USD once (framework §10.3) |
| **Presentation** | ANIM-INF + treat loop. Icon of a medical bag |

**Weapons:** sidearms only, `weapon.mp.inf_06.sidearm` SMALL_ARMS 0–450 cm, 6 per member / 1.0 s, self-defense only.

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.mp.inf_06.heal` | Active / automated under Guard (P) | 6 HP/s to one squad, capped at living members (§10.3) | Friendly infantry ≤ 300 cm | free · channel | Kill the medic | Heal ticks | Lowest HP ratio first |
| `ability.mp.inf_06.field_replenishment` | Active (P) | Restores **one** missing member to a squad in the field for that member's share × **1.0** of the unit price (producer rate is ×0.8) | Friendly squad ≤ 300 cm, out of combat 8 s | USD · 6 s per member · 20 s · 300 cm | Combat interrupts | Member fades in | Raids far from MP-BLD-03 |

**Research and upgrades:** `tech.shared.field_medical_procedures`: heal 6 → 8 HP/s.

**Counters:** *Beaten by (hard):* marksmen (AC-INF-08, IA-INF-08), raiders. *Situational:* lets MP-INF-08/10 raids continue deep in enemy territory.

**Status:** identity E-R; values P.

### MP-INF-07 — Caravan Reconnaissance Scouts

| Field | Specification |
| --- | --- |
| **Identity** | `unit.mp.inf_07` · Meridian Pact · Infantry / recon and scout marksman · `TPL-INF-SPEC` · **Extended** · Playable |
| **Role** | **Primary:** ground intelligence through observation and trade only; **no automatic classified intelligence** (E-R, E: scouting is not espionage). **Secondary:** precision fire on high-value infantry; route reconnaissance. **Doctrine fit:** reconnaissance behind opposing lines (E). |
| **Inspiration and background** | Two scouts travelling with a trade caravan cover, one carrying a scoped hunting rifle. C&C lineage (E-R): Spy (RA1, information role only); Pathfinder (Gen); Shadow Team (TW, observation role). |
| **Visual identity** | Concept: [`MP-INF-07.png`](../../Factions/TheMeridianPact/Units/MP-INF-07.png). Robes over kit, binoculars, scoped rifle wrapped in cloth. |
| **Statistics** | 2 × 60 HP = **120** (`armor.inf_light`) · FOOT **215 cm/s** · F0 · sight **1600 cm** · `DETECT.CONCEALED` 900 cm · 1 slot · food staff 2 · ExperienceValue 64 |
| **Commands** | as TPL-INF-SPEC + Trader Cover, Spot (P) · default stance Hold Fire while Concealed · formation role `scout_screen` |
| **Production** | **$640** (≥ RD-INF-09 $420 × 1.5 = $630, validator) · `MP-BLD-03` · prerequisites: `MP-BLD-02` operational · **12 s** · 2 food · no cap |
| **Veterancy** | Standard |
| **AI** | `ai.unit.specialist_infantry`. Deltas: observe convoys, fuel fields and bridges; shoot only officers, AT teams and medics |
| **Multiplayer** | MPA-STD + MPA-COVERT (disguise) |
| **Presentation** | ANIM-INF + prone, scope aim. Icon of a camel-and-eye trade mark |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.mp.inf_07.primary` scoped hunting rifle | PRECISION | 0–1000 cm | 50 / 3.0 s | hitscan, tracking medium | 8 on kill | GROUND | 160 % / 2 % / 0 % / — / 0 % |

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.shared.conceal` | Conditional (P) | Concealed after 3 s stationary | Self | free | `DETECT.CONCEALED`; firing | Own-only | Default |
| `ability.mp.inf_07.trader_cover` | Active (P) | Appears to opponents as civilian traders while moving at ≤ 60 % speed and not firing; cannot enter enemy structures and gathers **no** classified intelligence | Self | free · until broken · 30 s | `DETECT.SPY`; any hostile action; Unaligned police checks | Owner-only disguise frame | Cross observed roads |
| `ability.mp.inf_07.spot` | Active (P) | Keeps a point observed for 20 s; friendly indirect fire there uses observed scatter ×0.7 | Point in sight | free · 20 s · 25 s · sight | Suppress or kill | Marker (owner) | For MP-INF-04 and MP-ARM-08/12 |

**Research and upgrades:** `tech.shared.tactical_radio_modernization`: *Spot* 20 → 30 s.

**Counters:** *Beats (hard):* officers, AT and HW teams, medics (RD-INF-10, AC-INF-03, RD-INF-03). *Beaten by (hard):* recon infantry (RD-INF-09, AC-INF-08), scout vehicles, any vehicle, artillery once revealed. *Situational:* the eyes of a dispersed Pact army.

**Status:** identity and "observation only" E-R/E; values P.

### MP-INF-08 — Early Sabotage Specialist

| Field | Specification |
| --- | --- |
| **Identity** | `unit.mp.inf_08` · Meridian Pact · Infantry / non-spy saboteur · `TPL-INF-SPEC` · **Core** (faction signature) · Playable |
| **Role** | **Primary:** **early, low-cost, non-spy qualified saboteur** that attaches an abstract charge to an eligible enemy vehicle or building (E-R; E: sabotage doc §3). **Secondary:** route watching from concealment. **Doctrine fit:** the Pact is the strongest conventional sabotage faction, and qualified non-spy units perform the action from the start of the match (E). |
| **Inspiration and background** | A two-person demolition team from a regional militia, travelling light with satchels and a radio. The charge is a fictional inventory object; no device is modeled (E). C&C lineage (E-R): Terrorist (Gen, demolition role only, **no suicide attack**); Crazy Ivan (RA2); Demo Trap (Gen); Saboteur (TW, role translated). |
| **Visual identity** | Concept: [`MP-INF-08.png`](../../Factions/TheMeridianPact/Units/MP-INF-08.png). Shemaghs, olive work clothes, canvas satchels, carbines slung. Silhouette: two crouching figures, one carrying a large satchel. |
| **Statistics** | 2 × 55 HP = **110** (`armor.inf_light`) · FOOT **220 cm/s** · F0 · sight **1200 cm** · no detector · 1 slot · food staff 2 · ExperienceValue 42 |
| **Commands** | as TPL-INF-SPEC + Plant Charge, Conceal (P) · default stance Hold Fire while Concealed · formation role `raider` |
| **Production** | **$420** (below the agent band because it is not a spy, E: low cost; ≥ RD-INF-06 pioneer $240 × 1.5 = $360, validator) · `MP-BLD-03` · prerequisites: none beyond `MP-BLD-03` (E: early, no espionage research) · **10 s** · 2 food · no cap |
| **Veterancy** | Standard. Charge kills give **no XP** (framework §8 anti-farm list); XP comes from carbine kills only |
| **AI** | `ai.unit.covert_agent` adapted to concealment. Deltas: approach along routes outside known `DETECT.CONCEALED` footprints; prefer parked vehicles, refineries and producers; withdraw immediately after the charge is placed |
| **Multiplayer** | MPA-STD + **MPA-COVERT** (charge existence and fuse timer never replicate to the victim until detected or detonated) |
| **Presentation** | ANIM-INF + crouch-walk, kneel-place loop. Charge placement shows a satchel icon over the target (owner only). Icon of a satchel with a fuse spark |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.mp.inf_08.primary` carbines | SMALL_ARMS | 0–500 cm | 9 per member / 1.0 s (18 DPS) | hitscan, tracking fast | 2 | GROUND | 100 % / 15 % / 2 % / — / 8 % |
| `ability.shared.plant_charge` (attached charge) | DEMOLITION | contact (≤ 100 cm) | **500** after fuse | single target | — | GROUND vehicles, STRUCTURE | 0 % / 100 % / 100 % / — / 150 % soft, 120 % hardened |

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.shared.plant_charge` | Active (E permission, I predicate `CanPlantExplosives` Meridian at any time; P values) | 3 s placement channel, then a **10 s** fuse, then DEMOLITION 500 to the target. Eligible: enemy vehicles that are stationary or moving below 25 % speed, and enemy buildings. Never civilians, Unaligned or scenario-protected targets (E) | Eligible enemy target within 100 cm | **$120 per charge** (rivals $300, E: cheaper) · 10 s fuse · **30 s** · contact; **3 carried**, restocked at `MP-BLD-03` or `MP-DEF-07` | Any `DETECT.CONCEALED` or `DETECT.SPY` source within 400 cm reveals the charge; the victim's *Investigate* order removes it in 4 s; killing the team during the 3 s channel cancels it; moving the vehicle above 25 % speed before placement | Owner fuse timer; victim gets a warning only after detection | Use on the highest-value eligible target that is not inside a detector footprint |
| `ability.shared.conceal` | Conditional (P) | Concealed after 3 s stationary | Self | free | `DETECT.CONCEALED`; firing | Own-only | Wait concealed until a target stops |
| `ability.mp.inf_08.cut_and_run` | Active (P) | After a charge is placed, +30 % speed for 6 s and immune to Pinned for the duration | Self, only within 4 s after placement | free · 6 s · 30 s (tied to the charge) | Area fire, flame, fast scouts | Footstep tempo, dust | Always use after placement |

**Research and upgrades:** `tech.mp.field_demolition_kits` (MP-DEF-07 operational, $600, lane block 40 s): 4 charges carried; trade-off: −10 % speed while carrying ≥ 3 charges. `tech.shared.infantry_protective_equipment`: member HP 55 → 66 at −5 % speed.

**Counters:** *Beats (hard):* parked or sieged vehicles and soft economic buildings (refineries, depots, producers), for example RD-ARM-14 when sieged, AC-ARM-06 and IA-ARM-09 ATGM carriers sitting in ambush. *Beaten by (hard):* counter-agent and security infantry (AC-INF-11, RD-INF-07, IA-INF-12), recon units with `DETECT.CONCEALED` (RD-INF-09, AC-INF-08, IA-INF-08), any armed unit once revealed. *Situational:* strongest in the first five minutes, before rivals build detection; pair with MP-ARM-01 trucks that find targets (scouting itself never places a charge, E).

**Status:** identity, non-spy permission, early timing and cheaper-than-rivals relation E/E-R; `CanPlantExplosives` Meridian-at-any-time predicate I; charge count, fuse, damage, price and every other value P.

### MP-INF-09 — Raiding Engineer

| Field | Specification |
| --- | --- |
| **Identity** | `unit.mp.inf_09` · Meridian Pact · Infantry / raiding engineer · `TPL-INF-SUPPORT` · **Extended** · Playable |
| **Role** | **Primary:** **early non-spy qualified charge** plus repair and capture where authorized (E-R). **Secondary:** demolishing captured or abandoned structures; fixing raided vehicles in the field. **Doctrine fit:** engineers and raiders keep crossings open and damage enemy logistics (E). |
| **Inspiration and background** | A small field-engineer team with tool bags, a carbine and demolition satchels, used to break bridges, depots and fuel lines behind enemy lines. C&C lineage (E-R): Engineer (Ren); Combat Engineer (RA3); Demo Trap (Gen). |
| **Visual identity** | Concept: [`MP-INF-09.png`](../../Factions/TheMeridianPact/Units/MP-INF-09.png). Boonie hats, tool belts, canvas satchels; one member carries a field toolbox. |
| **Statistics** | 2 × 65 HP = **130** (`armor.inf_light`) · FOOT **205 cm/s** · F0 · sight **1000 cm** · 1 slot · food staff 2 · ExperienceValue 44 |
| **Commands** | Move, Stop, Guard (I); Plant Charge, Repair, Capture, Demolish (P) · default stance Hold Fire · formation role `support` |
| **Production** | **$440** (≥ RD-INF-06 $240 × 1.5 = $360, validator; premium over MP-INF-05 for the charge permission) · `MP-BLD-03` · prerequisites: `MP-BLD-04` operational · **11 s** · 2 food · no cap |
| **Veterancy** | Standard (carbine kills only) |
| **AI** | `ai.unit.support_infantry`. Deltas: if an eligible enemy vehicle or soft structure is within 1200 cm and no detector is known, plant a charge; otherwise repair the lowest-HP friendly vehicle |
| **Multiplayer** | MPA-STD + MPA-COVERT (charge state only) |
| **Presentation** | ANIM-INF + work loop, kneel-place. Icon of a wrench crossed with a satchel |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.mp.inf_09.primary` carbine | SMALL_ARMS | 0–450 cm | 9 / 1.0 s (one member armed) | hitscan | 2 | GROUND | 100 % / 15 % / 2 % / — / 8 % |
| `ability.shared.plant_charge` | DEMOLITION | contact | **450** after fuse | single target | — | GROUND vehicles, STRUCTURE | as MP-INF-08 |

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.shared.plant_charge` | Active (E permission; P values) | As MP-INF-08 but DEMOLITION **450**, a **4 s** channel and **2 charges** carried | Eligible enemy target ≤ 100 cm | **$120 per charge** · 10 s fuse · 40 s · contact | As MP-INF-08 | Owner fuse timer | Prefer bridges' defenders' vehicles and fuel storage |
| `ability.mp.inf_09.field_repair` | Active (P) | Repairs a friendly vehicle at 8 HP/s (framework §10.2 field rate, USD rule applies) | Friendly vehicle ≤ 300 cm | 25 % of unit cost per full repair · channel · — · 300 cm | Interrupt by fire; artillery on the repair point | Wrench sparks | Repair raiding vehicles after contact |
| `ability.mp.inf_09.capture` | Active (P; framework §10.6) | Captures an eligible enemy or neutral structure after 8 s uninterrupted; Unaligned structures only under scenario and jurisdiction rules (E) | Eligible structure | free · 8 s · — · contact | Any damage resets; garrisons block | Capture progress ring | Capture only depots and outposts left undefended |

**Research and upgrades:** `tech.mp.field_demolition_kits`: 3 charges carried (same trade-off as MP-INF-08). `tech.shared.vehicle_component_standardization`: field repair cost −20 %.

**Counters:** *Beats (hard):* undefended depots, fuel storage, parked logistics vehicles (RD-SUP-*, AC-ARM-13, IA-SUP-05). *Beaten by (hard):* any combat infantry, detectors (AC-INF-11, RD-INF-07, IA-INF-12), scout vehicles. *Situational:* a slower, sturdier saboteur that also keeps raiding vehicles alive; it is not concealed, so it needs MP-ARM-03 transport or night routes.

**Status:** identity and non-spy charge permission E-R/E; capture rule E (jurisdiction); all values P.

### MP-INF-10 — Mountain Ambush Squad

| Field | Specification |
| --- | --- |
| **Identity** | `unit.mp.inf_10` · Meridian Pact · Infantry / ambush specialists · `TPL-INF-SPEC` · **Core** · Playable |
| **Role** | **Primary:** terrain-bonus ambush, suppressive attack and quick withdrawal (E-R). **Secondary:** screening mountain roads; killing exposed crews and officers. **Doctrine fit:** exploitation of difficult geography, ambush and dispersed operations (E). |
| **Inspiration and background** | A mountain irregular squad with a light machine gun, a scoped rifle and rifles, trained to strike a column and vanish uphill. C&C lineage (E-R): Jarmen Kell (Gen, sniper role); Shadow Team (TW); Rebel Ambush (Gen). |
| **Visual identity** | Concept: [`MP-INF-10.png`](../../Factions/TheMeridianPact/Units/MP-INF-10.png). Layered robes, chest rigs, mixed-origin rifles, a bipod LMG. Silhouette: four figures, one with an LMG bipod and one with a scope. |
| **Statistics** | 4 × 65 HP = **260** (`armor.inf_light`) · FOOT **215 cm/s** (rough-terrain multiplier 1.0 instead of 0.8) · F0 · sight **1400 cm** · `DETECT.CONCEALED` 600 cm · 1 slot · food staff 4 · ExperienceValue 54 |
| **Commands** | as TPL-INF-SPEC + Ambush, Fall Back (P) · default stance Hold Fire while Concealed · formation role `ambush` |
| **Production** | **$540** (specialist band; ≥ RD-INF-03 HMG $260 × 1.5 = $390, validator) · `MP-BLD-03` · prerequisites: `MP-BLD-02` operational · **12 s** · 4 food · no cap |
| **Veterancy** | Standard |
| **AI** | `ai.unit.specialist_infantry` with threat weights 1.0 / 0.4 / 0 / 0 / 0. Deltas: occupy high ground or forest near a road; open fire with *Ambush* only when ≥ 2 targets are inside 600 cm; *Fall Back* when any armor or flame unit closes within 500 cm |
| **Multiplayer** | MPA-STD + MPA-COVERT (concealment) |
| **Presentation** | ANIM-INF + prone, LMG bipod deploy. Icon of a mountain peak over crossed rifles |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.mp.inf_10.primary` rifles + LMG | SMALL_ARMS | 0–700 cm | 11 per member / 1.0 s (44 DPS); LMG member suppression ×2 | hitscan, tracking fast | 5 | GROUND, AIR_LOW | 100 % / 15 % / 2 % / 25 % rotary / 8 % |
| `weapon.mp.inf_10.marksman` scoped rifle (one member) | PRECISION | 0–900 cm | 35 / 2.5 s | hitscan | 6 on kill | GROUND | 160 % / 2 % / 0 % / — / 0 % |
| `ability.shared.grenade` (I) | HIGH_EXPLOSIVE | 650 cm (I) | 50 (I), 150 cm (I) | lobbed (I) | 8 | GROUND, STRUCTURE | 110 % / 45 % / 12 % / — / 100 % |

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.shared.conceal` | Conditional (P) | Concealed after 3 s stationary | Self | free | `DETECT.CONCEALED`; firing | Own-only | Default |
| `ability.mp.inf_10.ambush` | Active (P) | The first 5 s of fire from Concealed deal +30 % damage and ×2 suppression; targets hit become Pinned if already Shaken | Self, must be Concealed | free · 5 s · 40 s · weapon range | Recon screens that reveal the squad first; armor (immune to the payoff) | Volley muzzle flashes; enemy "ambushed" bark | Open only on ≥ 2 targets or one officer |
| `ability.mp.inf_10.fall_back` | Active (P) | +40 % speed for 5 s, ignores Pinned; the squad cannot fire during it | Self | free · 5 s · 30 s | Fast vehicles, artillery on the retreat path | Sprint VFX | Use after *Ambush* or when armor closes |
| `ability.shared.sprint` | Active (I) | ×1.5 speed for 4 s | Self | free · 4 s · 12 s | — | Icon | Reposition between ambushes |

**Research and upgrades:** `tech.mp.mountain_warfare_training` (MP-BLD-03, $700, lane block 45 s): *Ambush* window 5 → 7 s; trade-off: price +$40 for new squads. `tech.shared.infantry_protective_equipment`: member HP 65 → 78 at −5 % speed.

**Counters:** *Beats (hard):* infantry columns on roads, crews of soft vehicles, officers (RD-INF-10, IA-INF-12). *Beats (soft):* light trucks (RD-ARM-01, AC scouts). *Beaten by (hard):* recon and detectors that reveal it before the ambush (RD-INF-09, AC-INF-08, IA-INF-08), flame (IA-INF-05, RD-ARM-10), mortars and artillery, any armor. *Situational:* the Pact's best defensive infantry on mountain maps; weak in open plains.

**Status:** identity and terrain role E-R/E; all values P; terrain multiplier depends on the U-03 terrain classes.

### MP-INF-11 — Contracted Veteran Marksman

| Field | Specification |
| --- | --- |
| **Identity** | `unit.mp.inf_11` · Meridian Pact · Infantry / hired marksman · `TPL-INF-SPEC` · **Conditional** (contract) · Playable |
| **Role** | **Primary:** uncommon hired precision shooter on a time-limited contract (E-R). **Secondary:** observation for artillery; officer and crew denial. **Doctrine fit:** mercenary contracting (E). |
| **Inspiration and background** | A veteran foreign marksman and spotter hired by the job, with an older bolt-action rifle and personal kit. C&C lineage (E-R): Pathfinder (Gen); Sniper (RA2); Deadeye (Ren). |
| **Visual identity** | Concept: [`MP-INF-11.png`](../../Factions/TheMeridianPact/Units/MP-INF-11.png). Ghillie-style scarf, scoped rifle, contractor armband instead of the Pact emblem. |
| **Statistics** | 2 × 70 HP = **140** (`armor.inf_light`) · FOOT **205 cm/s** · F0 · sight **1600 cm** · `DETECT.CONCEALED` 900 cm · 1 slot · food staff 2 · ExperienceValue 65 |
| **Commands** | as TPL-INF-SPEC + Spot (P) · default stance Hold Fire while Concealed · formation role `scout_screen` |
| **Production** | Hire **$650** (≥ RD-INF-09 $420 × 1.5 = $630, validator) + wage **$30 per interval** · contract **240 s** (renewable once) · `MP-BLD-16` · prerequisites: `MP-BLD-13` · **8 s** · 2 food · **limit 2** active contracts (E-R: uncommon) |
| **Veterancy** | Arrives at **Veteran** rank (E-R: veteran); further XP kept for the contract's life only |
| **AI** | `ai.unit.specialist_infantry`. Deltas: shoot officers, AT and HW teams, medics first; relocate after 2 shots |
| **Multiplayer** | MPA-STD + MPA-COVERT (concealment) + MPA-ECON (contract state) |
| **Presentation** | ANIM-INF + prone, bolt cycle. Contract timer (owner). Icon of a crosshair over a coin |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.mp.inf_11.primary` bolt-action marksman rifle | PRECISION | 0–1100 cm | 70 / 3.5 s | hitscan, tracking medium | 10 on kill | GROUND | 160 % / 2 % / 0 % / — / 0 % |

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.shared.conceal` | Conditional (P) | Concealed after 3 s stationary | Self | free | Detectors; firing | Own-only | Default |
| `ability.mp.inf_11.head_shot_discipline` | Passive (P) | Against `TPL-INF-COMMAND`, officer and crew-served targets: +25 % damage | Automatic | — | INF_PROTECTED armor still applies | Kill-cam flash (owner) | Target command units first |
| `ability.mp.contract_terms` | Passive (E-R metadata; P values) | As MP-INF-02 | Automatic | wage per interval | USD pressure | Contract timer | — |

**Research and upgrades:** `tech.mp.contract_arbitration`: contract 240 → 336 s; wage +$5.

**Counters:** *Beats (hard):* officers and commanders (RD-INF-10, AC-INF-14, IA-INF-13), AT and HW teams (RD-INF-03, AC-INF-03). *Beaten by (hard):* recon and detectors (RD-INF-09, AC-INF-08), scout vehicles, any vehicle, artillery. *Situational:* a rented answer to enemy commanders before the Pact can field its own.

**Status:** identity, contract metadata and veteran status E-R; values P; contract limit P.

### MP-INF-12 — Local Militia Levy

| Field | Specification |
| --- | --- |
| **Identity** | `unit.mp.inf_12` · Meridian Pact · Infantry / emergency levy · `TPL-INF-LINE` · **Extended** · Playable |
| **Role** | **Primary:** emergency defensive line at low tech, **morale-sensitive** (E-R). **Secondary:** cheap garrison and road watch. **Doctrine fit:** coordination of defensive operations with local forces (E). |
| **Inspiration and background** | Townspeople and farm workers mobilized with older rifles and armbands. C&C lineage (E-R): Rebel (Gen); Militant (TW); Angry Mob (Gen, formation effect). |
| **Visual identity** | Concept: none (asset requirement). Civilian clothes, armbands with the green emblem, mixed old rifles. |
| **Statistics** | 5 × 48 HP = **240** (`armor.inf_light`) · FOOT 195 cm/s · F0 · sight 1100 cm · 1 slot · food staff 5 · ExperienceValue 27 · **maximum resolve −20** (morale-sensitive) |
| **Commands** | as TPL-INF-LINE · default stance Guard · formation role `frontline` |
| **Production** | **$270** (≥ RD-INF-01 $175 × 1.5 = $263, validator) · `MP-BLD-03` **or** `MP-BLD-02` (emergency muster) · prerequisites: none · **5 s** · 5 food · no cap |
| **Veterancy** | Standard |
| **AI** | `ai.unit.line_infantry`. Deltas: garrison the nearest eligible building first; never advance beyond 1500 cm of the producer that mustered it |
| **Multiplayer** | MPA-STD |
| **Presentation** | ANIM-INF (civilian-walk variant). Uneven volley timing; icon of an armband |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.mp.inf_12.primary` old bolt and semi-automatic rifles | SMALL_ARMS | 0–600 cm | 8 per member / 1.2 s (33 DPS at 5 members) | hitscan, tracking medium | 2 | GROUND | 100 % / 15 % / 2 % / — / 8 % |

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.mp.inf_12.home_ground` | Conditional (P) | Within 1500 cm of a friendly MP-BLD-01/02/03: +15 maximum resolve (cancelling the levy penalty) and +10 % damage | Automatic | — | Lure them away; flank the building | Banner icon | Stay at home |
| `ability.mp.inf_12.population_link` | Passive (P, implementing E: Population Morale affects discipline) | Faction Population Morale ≥ 60: −10 % build time; ≤ 30: the levy starts at Shaken | Automatic | — | Strike Pact trade and fuel to lower morale | Morale tag on the card | — |

**Research and upgrades:** `tech.shared.infantry_protective_equipment`: member HP 48 → 58 at −5 % speed. No other upgrades: the levy is deliberately a low-tech stopgap.

**Counters:** *Beats (soft):* scouts and agents near home, early raids. *Beaten by (hard):* HMG (RD-INF-03), flame (IA-INF-05), mortars, any armor; Concord Psy-Ops and Ascendancy Intimidation (low resolve makes it the most eligible Pact unit). *Situational:* buys time while MP-INF-03 and MP-ARM-07 arrive.

**Status:** identity and morale sensitivity E-R; Population Morale linkage E (rule); values P.

### MP-INF-13 — Foreign Military Adviser

| Field | Specification |
| --- | --- |
| **Identity** | `unit.mp.inf_13` · Meridian Pact · Infantry / **Elite commander candidate** (E: talent doc §5, explicit approval required; U-14) · `TPL-INF-COMMAND` · **Extended** · Playable · **cap 1** |
| **Role** | **Primary:** provides purchased tactical upgrades without unlocking native rival factories (E-R). **Secondary:** if approved as commander, the Pact talent appointment: mobile command, contractors, logistics and tactical improvisation (E: talent identity). **Doctrine fit:** imported and licensed military expertise adapted to local requirements (E). |
| **Inspiration and background** | A seconded foreign officer with a small staff, a field radio and a briefcase of training manuals, attached to Pact forces under a commercial and diplomatic agreement. C&C lineage (E-R): Colonel Burton (Gen, specialist command); Black Lotus (Gen, specialist); Colonel Burton (ZH). |
| **Visual identity** | Concept: none (asset requirement). Pressed field uniform of no identifiable nation, beret, map case, sunglasses; two staff members with a radio. |
| **Statistics** | 1 × 460 HP = **460** (`armor.inf_protected`) · FOOT 215 cm/s · F0 · sight **1500 cm** · 1 slot · food staff 3 · no ordinary veterancy if approved as commander (E); otherwise standard (U-14 fallback). **Not** exempt from the isolation penalty (E: Pact Discipline) |
| **Commands** | as TPL-INF-COMMAND + Training Package + talent abilities (if approved) · default stance Guard · formation role `formation_anchor` |
| **Production** | **$2,000** (elite band; ≥ RD-INF-13 $1,250 × 1.5 = $1,875, validator) · `MP-BLD-03` · prerequisites: `MP-BLD-14` operational · **30 s** · 3 food · **cap 1**. Redeployment 90 s after death at full price, resuming the stored appointment (E: talent doc §4.4) |
| **Veterancy** | none if commander (E); fallback standard |
| **Command radius** | **1400 cm** baseline, carried in vehicles: the radius applies from MP-ARM-03 or MP-SUP-07 when it is a passenger (mobile command) |
| **Leadership effects** | Passive *Field Improvisation*: own vehicles in radius repair 2 HP/s when out of combat for 6 s (bounded; does not stack with repair vehicles) |
| **Vulnerabilities** | Lightly armed; jamming cuts links; Psy-Ops and Intimidation ineligible (§7.5); suffers the normal isolation penalty; death clears live effects (E) |
| **AI** | `ai.unit.commander`. Deltas: ride in MP-ARM-03 behind raiding groups; pick Tier I by composition (≥ 6 vehicles → Mobile Command) |
| **Multiplayer** | MPA-STD + **MPA-COMMAND** |
| **Presentation** | ANIM-INF + command gestures, map unfold. Aura ring for owner and allies; pennant for enemies. Icon of a compass over a handshake |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.mp.inf_13.primary` sidearm and staff carbines | SMALL_ARMS | 0–550 cm | 14 / 1.0 s | hitscan | 2 | GROUND | 100 % / 15 % / 2 % / — / 8 % |

**Abilities and proposed talent set** (three options per tier; one per tier; E: tier structure and sample path; P: effects; awaiting explicit commander approval, U-14)

| Tier | Option A | Option B | Option C |
| --- | --- | --- | --- |
| Unit ability (always) | **Training Package** (`ability.mp.inf_13.training_package`): spend **$300** to give one Pact squad or vehicle group in radius (≤ 6 units) one purchased drill for 90 s: *Fire Drill* (−15 % cooldown) or *Movement Drill* (+10 % speed). It never unlocks buildings, factories or research (E-R); cooldown 60 s | — | — |
| I Initial Doctrine | **Mobile Command** (E sample): radius stays active while the adviser is a passenger and moving, and own vehicles in radius get +10 % speed on roads | **Liaison Network:** command links resist jamming unless the jammer is within 600 cm | **Local Guides:** MP-INF-10/12 in radius ignore the rough-terrain speed penalty |
| II Field Specialization | **Contract Coordination** (E sample): contract units in radius (MP-INF-02/11, MP-AIR-02) gain +15 % damage and their wage is −20 % while in radius | **Supply Discipline:** resupply from MP-SUP-02/04 in radius is 25 % faster | **Salvage Teams:** `ability.shared.salvage_wreck` in radius yields 15 % instead of 10 % |
| III Advanced Command | **Raiding Doctrine** (E sample): units in radius that withdraw within 6 s after attacking take −20 % damage for 6 s | **Ambush Tutor:** *Ambush* (MP-INF-10) in radius lasts +2 s | **Battlefield Recovery Contract:** captured-equipment acquisition from wrecks (MP-ARM-13 rule) costs −15 % while the adviser is within 1400 cm of the wreck |
| IV Signature Operation | **Rapid Contract Mobilization** (E sample): for 60 s, new contracts at MP-BLD-16 are delivered in half the time and their hire fee is −25 %; cooldown 240 s; normal wages and Black Market rules still apply (E) | **Convoy Surge:** for 30 s, logistics and transport vehicles in radius +25 % speed and +20 % armor-equivalent HP; afterwards −15 % speed for 20 s; cooldown 200 s | **Coordinated Raid:** for 20 s, charges planted by MP-INF-08/09 in radius have a 6 s fuse; cooldown 180 s; charge price and eligibility unchanged (E) |

Every talent follows the commander influence invariant (talent doc §3.3) and the protected Meridian canon: no nuclear materials, no advanced propaganda, no rival-faction factories and no bypass of Black Market contract requirements (E).

**Research and upgrades:** research never grants talents (E). `tech.shared.tactical_radio_modernization`: radius +200 cm (clamp 1600 cm).

**Counters:** *Beats (soft):* nothing directly; multiplies raids, contracts and convoys. *Beaten by (hard):* marksmen (AC-INF-08, RD-INF-09), raiders (IA-INF-13), jammers (AC-ARM-10, RD-SUP-06), anything that catches its transport. *Situational:* worth the most in a vehicle-heavy, contract-heavy Pact army.

**Status:** identity E-R; talent candidacy, explicit approval requirement, cap 1 (if approved), OXP and death rules E; Training Package, price, radius and all talent effects P; commander status **U-14**.

## 4. Trucks, armored cars and mobile combat equipment

Pact vehicles come from `MP-BLD-04` Light Vehicle Assembly Yard unless stated. Imported and refitted chassis (MP-ARM-05/06) also need `MP-BLD-05` Licensed Repair Factory. Wheeled vehicles dominate: speed and road use are the Pact's armor.

### MP-ARM-01 — Roadrunner Scout Truck

| Field | Specification |
| --- | --- |
| **Identity** | `unit.mp.arm_01` · Meridian Pact · Vehicle / opening scout truck · `TPL-VEH-SCOUT` · **Core** (canonical opening scout) · Playable |
| **Role** | **Primary:** **canonical best early reconnaissance**: cheap, very fast, lightly armored and lightly armed (E: reconnaissance doc; E-R). **Secondary:** hunting enemy scouts and couriers; finding sabotage targets for MP-INF-08/09. **No sabotage permission** (E-R validator: scouting and sabotage are separately checked actions, E). **Doctrine fit:** the Pact's signature opening unit (E). |
| **Inspiration and background** | A stripped commercial pickup with a roll bar, a light machine gun and long-range radio, painted with a trading-company logo. C&C lineage (E-R): Technical (Gen); Humvee (TD); Pitbull (TW); Raider Buggy (TW). |
| **Visual identity** | none (asset requirement): sand pickup, olive canvas, roll-bar MG, a road-runner bird painted on the door; dust plume at speed. |
| **Statistics** | **180 HP** (`armor.veh_soft`; band low edge) · WHEEL_LIGHT **620 cm/s** (band top, fastest ground scout in the game; RD-ARM-01 580) · F1 · sight **1800 cm** · `DETECT.CONCEALED` 1080 cm (60 % of sight) · crew 2 · food 2 · fuel light · ExperienceValue 15 |
| **Commands** | as TPL-VEH-SCOUT + Scout Route, Road Dash (P) · default stance Hold Ground · formation role `scout_screen` |
| **Production** | **$150** (band low edge, E: cheapest; RD-ARM-01 $180) · `MP-BLD-04` (also `MP-BLD-01` before the yard exists) · prerequisites: none · **5 s** · 2 food · 1 petroleum · no cap |
| **Veterancy** | Standard |
| **AI** | `ai.unit.scout`. Deltas: in the first 3 minutes, run a loop of authored road nodes toward enemy start areas; report fuel fields, bridges and refineries; flee from anything with CV ≥ its own |
| **Multiplayer** | MPA-STD |
| **Presentation** | ANIM-VEH-WHEEL + suspension bounce, gunner traverse. High-revving engine, horn bark on contact; icon of a running bird |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.mp.arm_01.primary` light MG | SMALL_ARMS | 0–600 cm | 8 / 0.4 s (20 DPS) | hitscan, fire while moving | 2 | GROUND, AIR_LOW | 100 % / 15 % / 2 % / 25 % rotary / 8 % |

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.mp.arm_01.road_dash` | Active (P) | +20 % speed on road surfaces for 6 s; cannot fire | Self, on a road | free · 6 s · 25 s | Roadblocks (MP-DEF-01-class), mines, AT ambushes on roads | Dust trail, gear-change SFX | Use to escape or to cross observed ground |
| `ability.mp.arm_01.contact_report` | Passive (P) | Enemy units and structures this truck observes stay as `LastKnown` markers for 30 s instead of the default; contacts never carry hidden identity (no disguise reveal) | Automatic | — | Kill the truck before it sees; concealment | Map markers with a truck glyph | — |

**Research and upgrades:** `tech.mp.modular_commercial_equipment`: optional *Recoilless Kit* swap at MP-BLD-04 ($120, 6 s): replaces the MG with a SHAPED_CHARGE 90 / 4 s, 900 cm gun; trade-off −40 cm/s speed and loses *Road Dash*. `tech.shared.radar_signal_processing`: *Contact Report* 30 → 45 s.

**Counters:** *Beats (hard):* unarmed scout planes on the ground and couriers (AC-SUP-01 parked, IA-INF-14, RD-INF-11 in the open). *Beats (soft):* RD-ARM-01 at equal numbers and cost (Meridian has the better early recon, E). *Beaten by (hard):* IA-ARM-01 patrol cars, AC-ARM-01 jeeps, AT infantry, any tank, HMG (RD-INF-03), mines. *Situational:* scout planes (AC-SUP-01, IA-SUP-01) see further but cannot hold ground; the truck wins on cost, speed and contact persistence.

**Status:** identity, cheapest and fastest scout and "no sabotage" E/E-R; values P.

### MP-ARM-02 — Armed Commercial Pickup

| Field | Specification |
| --- | --- |
| **Identity** | `unit.mp.arm_02` · Meridian Pact · Vehicle / technical · `TPL-VEH-LIGHT` · **Core** · Playable |
| **Role** | **Primary:** low-cost escort with modular machine-gun or missile mounts (E-R). **Secondary:** raiding soft logistics; anti-infantry screen. **Doctrine fit:** affordable, adaptable equipment (E). |
| **Inspiration and background** | A civilian pickup with a heavy MG on a pedestal or a twin rocket pod. C&C lineage (E-R): Technical (Gen); Rocket Buggy (Gen); Buggy (TD). |
| **Visual identity** | none (asset requirement): olive pickup, mount on the bed, sandbags, the green emblem painted on the tailgate. |
| **Statistics** | **300 HP** (`armor.veh_soft`) · WHEEL_LIGHT **500 cm/s** · F1 · sight 1400 cm · crew 2 · food 2 · fuel light · ExperienceValue 35 |
| **Commands** | as TPL-VEH-LIGHT + Swap Mount (P, at MP-BLD-04 or MP-SUP-08) · default stance Aggressive · formation role `escort` |
| **Production** | **$350** (band low edge, E: Pact light trucks) · `MP-BLD-04` · **8 s** · 2 food · 1 petroleum · no cap |
| **Veterancy** | Standard |
| **AI** | `ai.unit.skirmisher`. Deltas: HMG mount hunts infantry; rocket mount hunts light vehicles; both disengage from VEH_HEAVY |
| **Multiplayer** | MPA-STD (mount type visible when observed) |
| **Presentation** | ANIM-VEH-WHEEL + mount traverse. Two mount meshes; icon of a pickup with a gun |

**Weapons** (one mount at a time)

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.mp.arm_02.hmg` (default mount) | SMALL_ARMS | 0–750 cm | 16 / 0.4 s (40 DPS) | hitscan, fire while moving | 4 | GROUND, AIR_LOW | 100 % / 15 % / 2 % / 25 % rotary / 8 % |
| `weapon.mp.arm_02.rocket_pod` (alternative) | SHAPED_CHARGE | 0–800 cm | 2 × 60 / 4.0 s; magazine 8, resupply | projectile, slow | 2 | GROUND | 25 % / 100 % / 90 % / — / 70 % |

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.mp.arm_02.swap_mount` | Active (P) | Changes HMG ↔ rocket pod in 8 s, stationary | At MP-BLD-04 or within 300 cm of MP-SUP-08 | $40 · 8 s · — | Caught while swapping | Crane animation | Swap to rockets when ≥ 3 enemy light vehicles are observed |

**Research and upgrades:** `tech.mp.modular_commercial_equipment`: swap time 8 → 4 s and in-field swap at any MP-SUP-02. `tech.shared.vehicle_component_standardization`: repair cost −20 %.

**Counters:** *Beats (hard, HMG):* infantry in the open, agents (RD-INF-01 platoons in the open, AC-INF-11). *Beats (soft, rockets):* light vehicles (RD-ARM-01/02, IA-ARM-01). *Beaten by (hard):* any tank, AT infantry (AC-INF-03, RD-INF-04), autocannon (IA-ARM-08, RD-ARM-06 in ground mode). *Situational:* cheapest escort for MP-SUP convoys.

**Status:** identity and modular mounts E-R; values P.

### MP-ARM-03 — Light Armored Van

| Field | Specification |
| --- | --- |
| **Identity** | `unit.mp.arm_03` · Meridian Pact · Vehicle / wheeled APC · `TPL-VEH-TRANSPORT` · **Core** · Playable |
| **Role** | **Primary:** transfer infantry and contracts between dispersed sites (E-R). **Secondary:** mobile command post for MP-INF-13 (if approved). **Doctrine fit:** motorized infantry and dispersed deployment (E). |
| **Inspiration and background** | An armored commercial van with firing ports, used by security firms and border forces. C&C lineage (E-R): APC (TD); Riptide ACV (RA3); Reckoner (KW, transport). |
| **Visual identity** | none (asset requirement): boxy sand van, side ports, roof hatch, painted company logo over the emblem. |
| **Statistics** | **480 HP** (`armor.veh_light`) · WHEEL_HEAVY **420 cm/s** (band top) · F2 · sight 1200 cm · **4 infantry slots**, 2 fire ports · crew 1 · food 1 · fuel medium · ExperienceValue 62 |
| **Commands** | as TPL-VEH-TRANSPORT · default stance Hold Fire · formation role `transport` |
| **Production** | **$620** (band low) · `MP-BLD-04` · **12 s** · 1 food · 3 petroleum · no cap |
| **Veterancy** | Standard (roof MG) |
| **AI** | `ai.unit.transport`. Deltas: unload at the nearest cover 600 cm short of the objective; return to the nearest MP-BLD-02 |
| **Multiplayer** | **MPA-TRANSPORT** (manifest owner-only) |
| **Presentation** | ANIM-VEH-WHEEL + rear doors. Icon of a van with a shield |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.mp.arm_03.roof_mg` | SMALL_ARMS | 0–650 cm | 10 / 0.5 s | hitscan | 2 | GROUND, AIR_LOW | 100 % / 15 % / 2 % / 25 % rotary / 8 % |

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.mp.arm_03.quick_dismount` | Passive (P) | Unload 0.5 s per slot instead of 1.0 s | Automatic | — | Ambush at the unload point | Doors slam | — |

**Research and upgrades:** `tech.mp.modular_commercial_equipment`: 5 slots; trade-off −20 cm/s. `tech.shared.vehicle_component_standardization`.

**Counters:** *Beats (soft):* lone infantry. *Beaten by (hard):* AT infantry, autocannon (IA-ARM-08, RD-ARM-06), tanks, mines (RD and Concord minefields). *Situational:* the commander's ride; losing it loaded is a severe blow.

**Status:** identity E-R; values P.

### MP-ARM-04 — Modular Cargo Gun Truck

| Field | Specification |
| --- | --- |
| **Identity** | `unit.mp.arm_04` · Meridian Pact · Vehicle / reconfigurable gun truck · `TPL-VEH-LIGHT` · **Extended** · Playable |
| **Role** | **Primary:** reconfigurable anti-personnel **or** anti-vehicle weapon mount (E-R). **Secondary:** convoy escort; point defense of outposts. **Doctrine fit:** adaptation of diverse equipment (E). |
| **Inspiration and background** | A heavy cargo truck with a flatbed gun mount: a twin 23-mm-class autocannon or a towed-style anti-tank gun on a turntable. C&C lineage (E-R): Multigunner IFV (RA3); Quad Cannon (Gen); IFV (RA2). |
| **Visual identity** | none (asset requirement): olive six-wheel truck, flatbed turntable, interchangeable gun shields. |
| **Statistics** | **400 HP** (`armor.veh_light`; armored cab) · WHEEL_HEAVY **420 cm/s** · F2 · sight 1300 cm · crew 3 · food 3 · fuel medium · ExperienceValue 55 |
| **Commands** | as TPL-VEH-LIGHT + Configure (P) · default stance Aggressive · formation role `escort` |
| **Production** | **$550** · `MP-BLD-04` · prerequisites: `tech.mp.modular_commercial_equipment` · **10 s** · 3 food · 3 petroleum · no cap |
| **Veterancy** | Standard |
| **AI** | `ai.unit.skirmisher`. Deltas: AP configuration escorts convoys; AV configuration takes Hold Ground at road chokepoints |
| **Multiplayer** | MPA-STD (configuration visible when observed) |
| **Presentation** | ANIM-VEH-WHEEL + turntable. Icon of a truck with a crossed gun-and-wrench |

**Weapons** (one configuration at a time)

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.mp.arm_04.ap_autocannon` (AP configuration) | AUTOCANNON | 0–800 cm | 2 × 14 / 0.6 s (47 DPS) | projectile, fast | 4 | GROUND, AIR_LOW | 65 % / 85 % / 10 % / 100 % rotary / 30 % |
| `weapon.mp.arm_04.at_gun` (AV configuration) | KINETIC_AP | 0–950 cm | 110 / 3.5 s | projectile, fast; only when Configured stationary | 2 | GROUND | 20 % / 90 % / 100 % / — / 40 % |

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.mp.arm_04.configure` | Active (P) | Switches AP ↔ AV configuration in 6 s; the AV gun fires only while stationary | Self | free · 6 s · 15 s | Flank during the switch | Gun shield swing | Switch to AV when ≥ 2 enemy VEH_MEDIUM/HEAVY are observed |

**Research and upgrades:** `tech.shared.vehicle_component_standardization`. `tech.mp.gun_shields` (MP-BLD-04, $400): +60 HP against SMALL_ARMS only; trade-off −20 cm/s.

**Counters:** *Beats (hard, AP):* infantry and helicopters (AC-AIR-04 low, RD-AIR-03). *Beats (soft, AV):* light and medium tanks from a prepared position (IA-ARM-03, RD-ARM-15). *Beaten by (hard):* MBTs at range, artillery, flame. *Situational:* one chassis, two answers; the switch time is the skill cost.

**Status:** identity and reconfiguration E-R; values P.

### MP-ARM-05 — Scavenged Light Tank

| Field | Specification |
| --- | --- |
| **Identity** | `unit.mp.arm_05` · Meridian Pact · Vehicle / refitted light tank · `TPL-VEH-MBT` (light tank variant) · **Extended** · Playable |
| **Role** | **Primary:** refitted imported armor with variable condition and spares (E-R). **Secondary:** fire support for raids; hunting technicals. **Doctrine fit:** imported and adapted foreign equipment (E). |
| **Inspiration and background** | Older imported light tanks rebuilt in Pact workshops with mixed parts. C&C lineage (E-R): Light Tank (TD); Scorpion Tank (Gen); Lasher Tank (YR). |
| **Visual identity** | none (asset requirement): mismatched road wheels, a replacement panel in another colour, a 76-mm-class gun, stowage everywhere. |
| **Statistics** | **620 HP** (`armor.veh_medium`) · TRACKED **360 cm/s** · F2 · sight 1250 cm · crew 3 · food 3 · fuel heavy · ExperienceValue 70 |
| **Commands** | as TPL-VEH-MBT · default stance Aggressive · formation role `frontline` |
| **Production** | **$700** · `MP-BLD-05` Licensed Repair Factory (refit) · prerequisites: `MP-BLD-04` · **14 s** · 3 food · 5 petroleum · no cap |
| **Veterancy** | Standard |
| **AI** | `ai.unit.battle_tank`. Deltas: avoid MBT duels; retreat at 40 % HP toward MP-SUP-08 |
| **Multiplayer** | MPA-STD (condition grade visible to owner only) |
| **Presentation** | ANIM-VEH-TRACK + turret. Condition grade shown as a stencil (owner); icon of a patched tank |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.mp.arm_05.gun` 76-mm-class gun | KINETIC_AP (HE round auto vs infantry: HIGH_EXPLOSIVE 45, 120 cm) | 0–850 cm | 80 / 2.8 s | projectile | 3 | GROUND | 20 % (HE 110 %) / 90 % / 100 % / — / 40 % |
| coaxial MG | SMALL_ARMS | 0–650 cm | 8 / 0.5 s | hitscan | 2 | GROUND | 100 % / 15 % / 2 % / — / 8 % |

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.mp.arm_05.condition_grade` | Passive (P) | Each unit is built with a condition grade drawn **deterministically** from the server production sequence (A, B, C cycle): A full stats; B −10 % max HP; C −10 % max HP and −10 % speed. Refit at MP-BLD-05 for $150 raises one grade | Automatic | refit $150 · 10 s | — (self-limitation) | Grade stencil (owner) | Refit C-grade units when idle |

**Research and upgrades:** `tech.mp.workshop_standards` (MP-BLD-05, $800, lane block 50 s): new units are at least grade B; trade-off +$50 price.

**Counters:** *Beats (hard):* technicals and scouts (RD-ARM-01/02, IA-ARM-01, AC-ARM-01). *Beats (soft):* APCs (RD-ARM-03, IA-ARM-02). *Beaten by (hard):* MBTs (AC-ARM-04, RD-ARM-04, IA-ARM-04), tank destroyers, AT infantry. *Situational:* cheap armor for a Pact army that cannot mass MBTs.

**Status:** identity and variable condition E-R; deterministic grade (framework §6.6: no client randomness) and values P.

### MP-ARM-06 — Surplus Battle Tank

| Field | Specification |
| --- | --- |
| **Identity** | `unit.mp.arm_06` · Meridian Pact · Vehicle / main battle tank · `TPL-VEH-MBT` · **Core** · Playable |
| **Role** | **Primary:** imported standard armor, **less dependable** than elite Concord or Ascendancy armor (E-R). **Secondary:** anchor for raids and outpost defense. **Doctrine fit:** limited heavy industrial concentration; MBTs are bought, not mass-produced (E). |
| **Inspiration and background** | A second-generation export tank bought second-hand through licensed brokers. C&C lineage (E-R): Grizzly Battle Tank (RA2); Rhino Tank (RA2); Guardian Tank (RA3). |
| **Visual identity** | none (asset requirement): sand-and-olive tank, 105-mm-class gun, extra stowage racks, foreign markings painted over. |
| **Statistics** | **850 HP** (`armor.veh_heavy`) · TRACKED **310 cm/s** · F3 · sight 1300 cm · crew 4 · food 4 · fuel heavy · ExperienceValue 100 |
| **Commands** | as TPL-VEH-MBT · default stance Aggressive · formation role `frontline` |
| **Production** | **$1,000** (band low-middle) · `MP-BLD-05` · prerequisites: `MP-BLD-04` and `MP-BLD-14` · **20 s** · 4 food · 5 petroleum · no cap |
| **Veterancy** | Standard |
| **AI** | `ai.unit.battle_tank`. Deltas: withdraw at 35 % HP (spares are scarce) |
| **Multiplayer** | MPA-STD |
| **Presentation** | ANIM-VEH-TRACK + turret. Icon of a tank with a price tag |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.mp.arm_06.gun` 105-mm-class gun | KINETIC_AP (HE vs infantry and structures: 60, 150 cm) | 0–900 cm | 120 / 3.4 s | projectile | 4 | GROUND | 20 % (HE 110 %) / 90 % / 100 % / — / 40 % (HE 100 %) |
| coaxial MG | SMALL_ARMS | 0–650 cm | 8 / 0.5 s | hitscan | 2 | GROUND | 100 % / 15 % / 2 % / — / 8 % |

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.mp.arm_06.imported_spares` | Passive (P) | Repair costs **+20 %** USD (framework §10.2) unless repaired at MP-BLD-05 or by MP-SUP-08 | Automatic | — | Kill MP-SUP-08 caravans | Spares icon on the repair tooltip | Route to MP-BLD-05 when < 50 % HP |

**Research and upgrades:** `tech.shared.vehicle_component_standardization`: cancels the +20 % surcharge. `tech.mp.add_on_armor` (MP-BLD-05, $900): +100 HP; trade-off −15 cm/s and +1 petroleum per build.

**Counters:** *Beats (hard):* APCs, technicals, light tanks. *Beats (soft):* RD-ARM-04 at equal cost. *Beaten by (hard):* AC-ARM-04 and IA-ARM-04 one-on-one, ATGMs (AC-ARM-06, IA-ARM-09), heavy tanks (RD-ARM-05, AC-ARM-05, IA-ARM-05), saboteur charges from rival spies late. *Situational:* the Pact should field few and keep them repaired.

**Status:** identity and "less dependable" E-R; values P.

### MP-ARM-07 — Mobile Recoilless-Gun Carrier

| Field | Specification |
| --- | --- |
| **Identity** | `unit.mp.arm_07` · Meridian Pact · Vehicle / wheeled tank destroyer · `TPL-VEH-TD` · **Core** · Playable |
| **Role** | **Primary:** fast anti-armor hit-and-run platform (E-R). **Secondary:** bunker busting. **Doctrine fit:** portable anti-armor and mobility instead of heavy armor (E). |
| **Inspiration and background** | A light truck carrying a 106-mm-class recoilless rifle with a spotting rifle. C&C lineage (E-R): Tank Destroyer (RA2); Rocket Buggy (Gen); Pitbull (TW). |
| **Visual identity** | none (asset requirement): open sand jeep-truck, long recoilless tube, back-blast scorch decal when firing. |
| **Statistics** | **420 HP** (`armor.veh_light`; below the TD band, trading armor for speed) · WHEEL_LIGHT **480 cm/s** (above the TD band) · F2 · sight 1400 cm · crew 2 · food 2 · fuel light · ExperienceValue 90 |
| **Commands** | as TPL-VEH-TD + Shoot and Scoot (P) · default stance Hold Ground · formation role `flank` |
| **Production** | **$900** (band low) · `MP-BLD-04` · **15 s** · 2 food · 1 petroleum · no cap |
| **Veterancy** | Standard |
| **AI** | `ai.unit.tank_destroyer`. Deltas: fire, then *Shoot and Scoot* to a new position ≥ 400 cm away; never trade shots with an MBT that has already acquired it |
| **Multiplayer** | MPA-STD |
| **Presentation** | ANIM-VEH-WHEEL + tube elevation, back-blast VFX (reveals position). Icon of a tube on wheels |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.mp.arm_07.recoilless` | SHAPED_CHARGE | 0–1150 cm | 170 / 4.5 s; stationary only | projectile, fast | 3 | GROUND | 25 % / 100 % / 90 % / — / 70 % |

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.mp.arm_07.shoot_and_scoot` | Active (P) | For 4 s after firing: +30 % speed and −25 % damage taken from KINETIC_AP | Self, within 1 s after a shot | free · 4 s · 10 s | Infantry at close range; HE artillery (unaffected) | Engine roar, dust | Always after a shot when an armed tank is within 1000 cm |

**Research and upgrades:** `tech.mp.modular_commercial_equipment`: spotting rifle adds +10 % hit chance against moving targets (scatter ×0.85). `tech.shared.vehicle_component_standardization`.

**Counters:** *Beats (hard):* MBTs and IFVs that cannot catch it (AC-ARM-04, RD-ARM-04, IA-ARM-02/04), APCs. *Beaten by (hard):* infantry and MGs (it is soft-skinned), autocannon (IA-ARM-08, RD-ARM-06), artillery, attack helicopters (AC-AIR-04, RD-AIR-03). *Situational:* the Pact's main armor answer; dies if caught standing.

**Status:** identity and hit-and-run role E-R; values P.

### MP-ARM-08 — Truck-Mounted Rocket Rack

| Field | Specification |
| --- | --- |
| **Identity** | `unit.mp.arm_08` · Meridian Pact · Vehicle / rocket artillery · `TPL-VEH-ARTY` · **Core** · Playable |
| **Role** | **Primary:** cheap indirect barrage with reload and supply vulnerabilities (E-R). **Secondary:** harassment of bases and fuel fields. **Doctrine fit:** mobile artillery for dispersed formations (E). |
| **Inspiration and background** | A light truck carrying a 12-tube light rocket rack, the classic improvised artillery of irregular forces. C&C lineage (E-R): Rocket Buggy (Gen); V2 Rocket Launcher (RA1); MLRS (TD). |
| **Visual identity** | none (asset requirement): flatbed truck with a tilted tube rack, tarp over the cab, launch smoke. |
| **Statistics** | **360 HP** (`armor.veh_soft`; below the band: unarmored truck) · WHEEL_HEAVY 400 cm/s · F2 · sight 1100 cm · crew 2 · food 2 · fuel light · ExperienceValue 90 · death blast (ammunition, U-07) |
| **Commands** | as TPL-VEH-ARTY + Fire Mission (P) · default stance Hold Ground · formation role `artillery` |
| **Production** | **$900** (band low edge) · `MP-BLD-04` · **18 s** · 2 food · 1 petroleum · no cap |
| **Veterancy** | Standard |
| **AI** | `ai.unit.artillery`. Deltas: fire one full salvo then displace; never fire without an observer (MP-INF-07, MP-ARM-01, MP-MKT-09) |
| **Multiplayer** | **MPA-INDIRECT** |
| **Presentation** | ANIM-VEH + rack elevation, ripple-launch VFX with a long smoke trail (shooter tell). Icon of a rack |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.mp.arm_08.rockets` | HIGH_EXPLOSIVE | **900–3200 cm** | 12 rockets × 40, radius 200 cm, ripple over 3 s; **1 salvo per magazine, 2 magazines carried**; reload 20 s from the carried stock, then resupply | ballistic, scatter base 250 cm (observed ×0.7) | 10 per hit | GROUND, STRUCTURE | 110 % / 45 % / 12 % / — / 100 % |

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.mp.arm_08.reload_from_stock` | Automated (P) | After the second salvo the rack is empty until a MP-SUP-02/04 resupplies it or it returns to MP-BLD-04 (framework §10.4) | Automatic | resupply per §10.4 | Kill the supply trucks; strike during the 20 s reload | Empty-rack icon | Call resupply when empty |

**Research and upgrades:** `tech.mp.rocket_rack_reloads` (MP-BLD-04, $500): 3 magazines carried; trade-off: death blast radius +100 cm.

**Counters:** *Beats (hard):* infantry blocks (RD-INF-01), soft structures, deployed teams. *Beaten by (hard):* any direct-fire unit that reaches it, counter-battery (AC-ARM-12), aircraft (AC-AIR-02, IA-AIR-02), raids. *Situational:* cheap shock with long gaps; supply discipline decides its value.

**Status:** identity and supply vulnerability E-R; values P; death blast U-07.

### MP-ARM-09 — Quad Anti-Air Technical

| Field | Specification |
| --- | --- |
| **Identity** | `unit.mp.arm_09` · Meridian Pact · Vehicle / mobile AA · `TPL-VEH-AA` · **Core** · Playable |
| **Role** | **Primary:** mobile anti-air and anti-infantry escort (E-R). **Secondary:** shredding technicals. **Doctrine fit:** short-range air defense for dispersed formations (E). |
| **Inspiration and background** | A truck with a quad heavy-MG or twin light autocannon mount, the classic dual-purpose escort. C&C lineage (E-R): Quad Cannon (Gen); Gattling Tank (Gen); Slingshot (KW). |
| **Visual identity** | none (asset requirement): open truck with a quad mount and a gunner's seat, ammunition boxes. |
| **Statistics** | **460 HP** (`armor.veh_light`) · WHEEL_HEAVY **400 cm/s** · F2 · sight 1300 cm / **2200 cm air (visual only, no radar)** · crew 2 · food 2 · fuel light · ExperienceValue 80 |
| **Commands** | as TPL-VEH-AA · default stance Aggressive (air) · formation role `air_cover` |
| **Production** | **$800** (band low edge) · `MP-BLD-04` · **14 s** · 2 food · 1 petroleum · no cap |
| **Veterancy** | Standard |
| **AI** | `ai.unit.air_defense` with ground weights raised: 0.8 / 0.6 / 0 / 1.5 / 0. Deltas: engage infantry when no air is observed |
| **Multiplayer** | MPA-STD |
| **Presentation** | ANIM-VEH + quad traverse, tracer streams. Icon of four barrels |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.mp.arm_09.quad` | AUTOCANNON | 0–1000 cm (air 1100 cm) | 4 × 9 / 0.5 s (72 DPS raw) | projectile, fast | 5 | GROUND, AIR_LOW, AIR_HIGH | 65 % / 85 % / 10 % / 100 % rotary, 60 % fixed / 30 % |

**Abilities:** none unit-specific; its value is two-role coverage at a low price. It has **no radar**, so it needs MP-MKT-05 or sight to see incoming aircraft.

**Research and upgrades:** `tech.mp.modular_commercial_equipment`: gun shield +60 HP vs SMALL_ARMS; trade-off −20 cm/s. `tech.shared.radar_signal_processing`: air sight 2200 → 2600 cm.

**Counters:** *Beats (hard):* helicopters (AC-AIR-04/09, RD-AIR-03/08, IA-AIR-03), infantry in the open. *Beats (soft):* strike jets on low passes (AC-AIR-02, IA-AIR-02). *Beaten by (hard):* tanks, artillery, high-altitude bombers outside its envelope (AC-AIR-08, RD-AIR-07). *Situational:* the escort every Pact raid should bring.

**Status:** identity and dual role E-R; values P.

### MP-ARM-10 — Mobile Smoke-Screen Truck

| Field | Specification |
| --- | --- |
| **Identity** | `unit.mp.arm_10` · Meridian Pact · Vehicle / smoke generator · `TPL-VEH-EW` · **Extended** · Playable |
| **Role** | **Primary:** temporarily obscures lanes; **not invisibility** (E-R; E: roster rule 7). **Secondary:** covering retreats, river crossings and convoy runs. **Doctrine fit:** camouflage and difficult-terrain operations (E). |
| **Inspiration and background** | A truck with a thermal smoke generator and a stack of smoke pots. C&C lineage (E-R): Mobile Gap Generator (RA1); Gap Generator (RA2, concealment function); Mobile Stealth Generator (TS). |
| **Visual identity** | none (asset requirement): tanker-style truck with a rear smoke nozzle, oil drums, a smoke-grey stripe. |
| **Statistics** | **380 HP** (`armor.veh_light`) · WHEEL_HEAVY 380 cm/s · F2 · sight 1200 cm · crew 2 · food 2 · fuel light · no veterancy |
| **Commands** | Move, Stop, Guard (I); Smoke Lane, Smoke Pot (P) · default stance Hold Fire · formation role `support` |
| **Production** | **$650** · `MP-BLD-04` · prerequisites: `MP-BLD-05` · **12 s** · 2 food · 1 petroleum · cap 3 |
| **Veterancy** | none (unarmed) |
| **AI** | `ai.unit.electronic_warfare`. Deltas: lay a lane across observed enemy sight lines toward a retreating friendly group; never smoke friendly artillery observers |
| **Multiplayer** | MPA-STD (smoke volumes are world state visible to all) |
| **Presentation** | ANIM-VEH + nozzle. Dense smoke VFX with LOD budget; icon of a cloud |

**Weapons:** none (unarmed)

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.mp.arm_10.smoke_lane` | Active (P; framework §9.5 smoke) | While moving, lays up to **5 smoke cylinders** (400 cm radius, 20 s each) along the path, one per 300 cm; they block line of sight and fire, units inside are Concealed | Self, moving | 1 smoke charge per lane · 20 s per cylinder · 45 s · path | Wind is not modeled; detectors still see within the Concealed range; artillery fires blind into smoke; the truck itself is visible at the lane's head | Visible smoke for everyone | Cover the withdrawal of MP-ARM-07/MP-INF-10 |
| `ability.mp.arm_10.smoke_pot` | Active (P) | Single cylinder at a point within 900 cm | Point | free · 20 s · 25 s · 900 cm | as above | Pot arc | Cover a stalled convoy |

**Research and upgrades:** `tech.mp.chemical_smoke_mix` (MP-BLD-05, $400): cylinders last 30 s; trade-off: the truck takes +25 % damage from INCENDIARY (fuel load).

**Counters:** *Beats (soft):* artillery spotting, ATGM lines (AC-ARM-06, IA-ARM-09 lose line of sight). *Beaten by (hard):* any armed unit; detectors (RD-INF-09, AC-INF-08) still see inside within range; flame. *Situational:* makes Pact hit-and-run work against long-range armies.

**Status:** identity and "not invisibility" E-R/E; smoke model framework §9.5 P; values P.

### MP-ARM-11 — Ambush Mine Carrier

| Field | Specification |
| --- | --- |
| **Identity** | `unit.mp.arm_11` · Meridian Pact · Vehicle / minelayer · `TPL-VEH-ENGINEER` · **Extended** · Playable |
| **Role** | **Primary:** lays abstract combat mines that are **detectable and clearable** (E-R). **Secondary:** denial of roads and fords on retreat. **Doctrine fit:** ambush and route denial (E). |
| **Inspiration and background** | A light tracked carrier with a mine chute. Mines are abstract game objects; no device is modeled (E: brief). C&C lineage (E-R): Minelayer (RA1); Demo Trap (Gen); Burrow Mines (TT). |
| **Visual identity** | none (asset requirement): olive carrier, rear chute, marker-flag rack. |
| **Statistics** | **520 HP** (`armor.veh_light`) · TRACKED 340 cm/s · F2 · sight 1100 cm · crew 2 · food 2 · fuel medium · no veterancy |
| **Commands** | Move, Stop, Guard (I); Lay Minefield, Recover Mines (P) · default stance Hold Fire · formation role `support` |
| **Production** | **$700** · `MP-BLD-04` · prerequisites: `MP-DEF-05` · **13 s** · 2 food · 3 petroleum · no cap |
| **Veterancy** | none (unarmed) |
| **AI** | `ai.unit.engineer_vehicle`. Deltas: mine chokepoints on the predicted enemy route; never mine roads used by own convoys within the last 60 s |
| **Multiplayer** | **MPA-COVERT** (mine positions are owner and allies only until detected) |
| **Presentation** | ANIM-VEH + chute. Mines show to the owner as flagged discs; icon of a mine with a flag |

**Weapons:** none (unarmed; mines are deployed objects)

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.mp.arm_11.lay_minefield` | Active (P) | Lays an anti-vehicle mine per 2 s while moving slowly; each mine deals **DEMOLITION 300** to the first enemy vehicle within 80 cm (never infantry, matrix 0 %). Mines are Concealed objects | Ground, path | **$40 per mine** · persistent · 2 s · **6 mines carried**; **24 active per participant** | `DETECT.CONCEALED` reveals mines within its range; mine detectors (RD-ARM-11, AC-ARM-14, MP-MKT-02) reveal at 500–600 cm and clear them; any weapon can destroy a revealed mine; Unaligned roads under jurisdiction rules (E) | Owner flags; enemy sees a warning only after detection | Lay at bridges and passes |
| `ability.mp.arm_11.recover_mines` | Active (P) | Picks up own mines within 300 cm (refund 50 %) | Own mines | free · 2 s each | — | Chute reverse | Recover before advancing |

**Research and upgrades:** `tech.mp.field_demolition_kits`: 8 mines carried. Trade-off: the carrier gains a death blast (radius 250 cm, U-07).

**Counters:** *Beats (hard):* unscouted armor columns and trucks on roads (RD-ARM-02/04, IA-ARM-02/04, AC-ARM-01/02). *Beaten by (hard):* mine-clearing and detector units (RD-ARM-11, AC-ARM-14), recon vanguards; the carrier itself dies to anything armed. *Situational:* turns Pact mountain roads into kill zones; does nothing against infantry or aircraft.

**Status:** identity and detectable/clearable mines E-R; values P.

### MP-ARM-12 — Rough-Terrain Artillery Truck

| Field | Specification |
| --- | --- |
| **Identity** | `unit.mp.arm_12` · Meridian Pact · Vehicle / self-propelled howitzer-mortar · `TPL-VEH-ARTY` · **Extended** · Playable |
| **Role** | **Primary:** flexible medium self-propelled mortar or howitzer (E-R). **Secondary:** counter-battery against slow artillery; smoke. **Doctrine fit:** mobile artillery across difficult terrain (E). |
| **Inspiration and background** | An all-terrain truck carrying a 120-mm-class mortar-howitzer that fires from the bed. C&C lineage (E-R): Artillery (TD); Mortar Cycle (Uprising); Pacifier FAV (Uprising, siege analogue). |
| **Visual identity** | none (asset requirement): high-clearance six-wheel truck, stabilizer legs, short thick barrel. |
| **Statistics** | **450 HP** (`armor.veh_light`) · WHEEL_HEAVY **360 cm/s** (rough-terrain multiplier 0.9 instead of 0.7) · F2 · sight 1100 cm · crew 3 · food 3 · fuel medium · ExperienceValue 110 |
| **Commands** | as TPL-VEH-ARTY + Deploy, Fire Mission, Smoke Round (P) · default stance Hold Ground · formation role `artillery` |
| **Production** | **$1,100** (band low) · `MP-BLD-04` · prerequisites: `MP-BLD-05` · **20 s** · 3 food · 3 petroleum · no cap |
| **Veterancy** | Standard |
| **AI** | `ai.unit.artillery`. Deltas: deploy on high ground; displace after 3 salvos |
| **Multiplayer** | **MPA-INDIRECT** |
| **Presentation** | ANIM-VEH + legs deploy, elevation, recoil. Icon of a truck with a short barrel |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.mp.arm_12.howitzer` | HIGH_EXPLOSIVE | **800–2800 cm** (deployed) | 110, radius 250 cm / 5.0 s; magazine 20, resupply | ballistic, scatter base 200 cm (observed ×0.7) | 12 | GROUND, STRUCTURE | 110 % / 45 % / 12 % / — / 100 % |

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.mp.arm_12.deploy` | Active (P) | 3 s deploy to fire; 2 s undeploy | Self | free | Caught while deploying | Legs animation | Deploy only at a planned firing point |
| `ability.mp.arm_12.smoke_round` | Active (P) | One smoke cylinder (framework §9.5) at a point in range | Point | uses 1 magazine round · 20 s · 30 s | as smoke | Smoke shell trail | Screen MP-ARM-07 withdrawals |

**Research and upgrades:** `tech.shared.tactical_radio_modernization`: observed scatter ×0.6 with MP-INF-07 or MP-INF-11 spotting. `tech.mp.workshop_standards`: deploy 3 → 2 s.

**Counters:** *Beats (hard):* infantry in cover, HW teams, light structures, towed and wheeled artillery (RD-ARM-07). *Beaten by (hard):* counter-battery (AC-ARM-12), aircraft, raids. *Situational:* the Pact's reliable artillery, more accurate and sustained than MP-ARM-08.

**Status:** identity E-R; values P; terrain multipliers U-03.

### MP-ARM-13 — Captured Heavy Armor

| Field | Specification |
| --- | --- |
| **Identity** | `unit.mp.arm_13` · Meridian Pact · Vehicle / captured heavy tank · `TPL-VEH-HEAVY` · **Conditional** (market or battlefield capture) · Playable |
| **Role** | **Primary:** heavy armor obtained **only from the market or by battlefield capture**; it cannot emerge from a Pact factory by default (E-R). **Secondary:** a rare breakthrough tool. **Doctrine fit:** acquisition and adaptation of foreign systems; stolen equipment never rewrites manufacturing permission (E-R). |
| **Inspiration and background** | An enemy heavy tank recovered from the battlefield or bought through brokers, crewed by Pact tankers working from captured manuals. C&C lineage (E-R): Mammoth Tank (TD); Overlord Tank (Gen); Apocalypse Tank (RA2). |
| **Visual identity** | none (asset requirement): the source faction's heavy-tank mesh with Pact sand overpaint, crossed-out source insignia and the green emblem on the turret. |
| **Statistics** | Inherits the source unit's HP, armor, speed and weapons at **−10 % max HP** (unfamiliar crews and scarce parts). Default market variant: **1,300 HP** (`armor.veh_heavy`) · TRACKED_HEAVY 250 cm/s · F3 · sight 1300 cm · crew 4 · food 4 · fuel heavy · ExperienceValue 190. Ownership records `sourceFaction` and `acquisitionMethod` (E-R) |
| **Commands** | as TPL-VEH-HEAVY · default stance Aggressive · formation role `spearhead` |
| **Production** | **Not producible.** (a) **Market:** bought at `MP-BLD-15` against finite auction stock at **$1,900** plus delivery by MP-SUP-02 or MP-SEA-01 (interdictable, E). (b) **Battlefield capture:** MP-SUP-08 or MP-MKT-01 tows an eligible enemy heavy or MBT wreck (for example AC-ARM-05, RD-ARM-05, IA-ARM-05; MBTs AC-ARM-04, RD-ARM-04, IA-ARM-04) to MP-BLD-05 within 90 s; the refit costs **50 % of the source unit's price**, takes 30 s and spawns at 40 % HP. Prototypes (AC-ARM-15, RD-ARM-14, IA-ARM-06) are **not** eligible. **cap 2** active |
| **Veterancy** | Standard (resets on capture) |
| **AI** | `ai.unit.heavy_armor`. Deltas: never alone; retreat at 30 % HP to MP-BLD-05 (it is irreplaceable) |
| **Multiplayer** | MPA-STD. `sourceFaction` is visible to observers (overpainted mesh); `acquisitionMethod` is owner-only |
| **Presentation** | Source animation set + overpaint material. Icon of the source tank with a hook |

**Weapons:** the source unit's weapons, damage −0 % (the gun is the same gun) but cooldown **+10 %** (unfamiliar loading drill). Market default:

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.mp.arm_13.gun` heavy tank gun | KINETIC_AP (HE vs infantry and structures: 80, 200 cm) | 0–950 cm | 170 / 4.2 s | projectile | 5 | GROUND | 20 % (HE 110 %) / 90 % / 100 % / — / 45 % (HE 100 %) |

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.mp.arm_13.foreign_parts` | Passive (P) | Field repair caps at 80 % max HP; only MP-BLD-05 restores it fully. Never unlocks the source faction's factories, research or ammunition production (E-R) | Automatic | — | Deny the return to MP-BLD-05 | Parts icon | — |

**Research and upgrades:** none. Upgrades bought by the source faction do not transfer (P). `tech.mp.workshop_standards`: refit time 30 → 20 s.

**Counters:** *Beats (hard):* anything the source heavy tank beats: MBT lines, fortifications. *Beaten by (hard):* ATGMs, tank destroyers and heavy tanks of the source faction itself, attack aircraft, saboteur and spy charges. *Situational:* punishes rivals who leave heavy wrecks behind; Concord and Ascendancy recovery vehicles (AC-ARM-13, IA-SUP-05) compete for the same wrecks.

**Status:** identity, "never from a Pact factory" and ownership fields E-R; capture route, market price, prototype exclusion, cap and all values P; new item **U-MP-01** (capture eligibility list).

## 5. Construction, economic and contract logistics

Logistics vehicles are unarmed unless stated, have no veterancy (framework §8) and use `ai.unit.logistics` or `ai.unit.constructor`. Tankers carry their cargo type visibly (MPA-ECON) and have death blasts (U-07). Fuel stocks follow the Pact rules: proximity to a crude-oil or gas field, never free ownership or cheaper fuel (E).

### MP-SUP-01 — Forward Trade Constructor

| Field | Specification |
| --- | --- |
| **Identity** | `unit.mp.sup_01` · Meridian Pact · Support / constructor · `TPL-VEH-CONSTRUCTOR` · **Core** · Playable |
| **Role** | **Primary:** establish dispersed depots and authorized contracts near territory (E-R). **Secondary:** expand toward fuel fields; rebuild after raids. **Doctrine fit:** geographically dispersed installations; activity continues when administrative centres are disrupted (E). |
| **Inspiration and background** | A convoy truck with a crane, prefabricated stalls and a site office. C&C lineage (E-R): Worker (Gen); MCV (TD); Prospector (RA3). |
| **Visual identity** | none (asset requirement): sand flatbed with a crane, folded market canopies, a site-office container. |
| **Statistics** | **500 HP** (`armor.veh_medium`) · WHEEL_HEAVY **400 cm/s** · F2 · sight 1000 cm · crew 2 · food 2 · fuel medium |
| **Commands** | Move, Stop (I); Build, Deploy Outpost (P) · default stance Hold Fire · formation role `support` |
| **Production** | **$600** · `MP-BLD-01` · **12 s** · 2 food · 3 petroleum · no cap |
| **Veterancy** | none |
| **AI** | `ai.unit.constructor`. Deltas: prefer sites within 2500 cm of a fuel field or road junction; abort if hostile units are observed within 1500 cm |
| **Multiplayer** | MPA-ECON |
| **Presentation** | ANIM-VEH + crane. Icon of a crane over a stall |

**Weapons:** none (unarmed)

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.mp.sup_01.build` | Active (P) | Builds Pact BLD/DEF structures within 600 cm, subject to power and prerequisites (E) | Valid site | structure price | Kill the constructor; deny the site | Build ghost and scaffolding | Expand toward fuel and roads |
| `ability.mp.sup_01.deploy_outpost` | Active (P) | Unpacks directly into `MP-BLD-02` Forward Bazaar Outpost in 20 s at **−25 %** of its price (the vehicle is consumed) | Valid site | outpost price −25 % · 20 s | Raids during unpack | Canopy unfold | Use at the edge of contested territory |

**Research and upgrades:** `tech.mp.modular_commercial_equipment`: build speed +15 %; trade-off: constructor price +$60.

**Counters:** *Beaten by (hard):* any armed unit, especially raiders (IA-ARM-01, RD-ARM-01) and artillery. *Situational:* the Pact's expansion engine; escort with MP-ARM-02.

**Status:** identity E-R; dispersal and decentralization E; values P.

### MP-SUP-02 — Merchant Delivery Truck

| Field | Specification |
| --- | --- |
| **Identity** | `unit.mp.sup_02` · Meridian Pact · Support / delivery truck · `TPL-VEH-LOGISTICS` · **Extended** · Playable |
| **Role** | **Primary:** transfer bought assets through actual road traffic (E-R; E: market delivery is physical and interdictable). **Secondary:** ammunition resupply for Pact artillery and rocket racks. **Doctrine fit:** commercial networks and military supply (E). |
| **Inspiration and background** | A covered commercial lorry with a company name, carrying crates. C&C lineage (E-R): Supply Truck (Gen); Technical (Gen); Cargo Plane (TD, delivery concept). |
| **Visual identity** | none (asset requirement): blue-and-sand covered lorry, painted trade logo, roof rack. |
| **Statistics** | **420 HP** (`armor.veh_soft`) · WHEEL_HEAVY 400 cm/s · F2 · sight 1000 cm · cargo 1 market item or 1 light vehicle **or** resupply stock 12 · crew 1 · food 1 · fuel light |
| **Commands** | Move, Stop, Guard (I); Deliver, Resupply (P) · default stance Hold Fire · formation role `convoy` |
| **Production** | **$300** · `MP-BLD-04` or `MP-BLD-17` · **8 s** · 1 food · 1 petroleum · no cap |
| **Veterancy** | none |
| **AI** | `ai.unit.logistics`. Deltas: follow the dispatch route from MP-BLD-17; reroute around observed threats |
| **Multiplayer** | **MPA-ECON** (cargo identity owner-only; enemies see a covered lorry) |
| **Presentation** | ANIM-VEH-WHEEL + tailgate. Icon of a crate truck |

**Weapons:** none (unarmed)

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.mp.sup_02.deliver_market_item` | Automated (P; E: physical delivery) | Carries one purchased market item from the border or port spawn to the buyer's MP-BLD-15 or a chosen MP-BLD-02; the item spawns on arrival. If destroyed, the item is lost and the auction fee is not refunded (E: interdiction) | Assigned delivery | delivery fee $50 · travel time | Interdict the road; mines; raids | Delivery ETA (owner) | Avoid observed hostiles |
| `ability.shared.resupply` | Aura (P; framework §10.4) | Resupplies limited-ammunition Pact units within 400 cm: 1 ammo unit per 2 s, up to 4 targets | Friendly units | per §10.4 · stock 12 | Kill the truck | Ammo crate icon | Park behind MP-ARM-08/12 |

**Research and upgrades:** `tech.shared.supply_preservation`: stock 12 → 16.

**Counters:** *Beaten by (hard):* everything armed; mines (MP-ARM-11-class enemy mines, RD minefields). *Situational:* the Black Market is only as good as the roads; escorts matter.

**Status:** identity and physical interdictable delivery E-R/E; values P.

### MP-SUP-03 — Crude Tanker Convoy

| Field | Specification |
| --- | --- |
| **Identity** | `unit.mp.sup_03` · Meridian Pact · Support / crude tanker · `TPL-VEH-LOGISTICS` · **Extended** · Playable |
| **Role** | **Primary:** collect and transport crude **after** legal or contested access to a field (E-R). **Secondary:** none. **Doctrine fit:** resource sovereignty and fossil-fuel proximity without free ownership (E). |
| **Inspiration and background** | Two-trailer road tankers running from a wellhead to a refinery. C&C lineage (E-R): Supply Truck (Gen); Ore Truck (RA1); Ore Collector (RA3). |
| **Visual identity** | none (asset requirement): black-and-sand tanker with two trailers, hazard plates, flame decal on death. |
| **Statistics** | **600 HP** (`armor.veh_soft`) · WHEEL_HEAVY 340 cm/s (loaded 300) · F3 · sight 1000 cm · crude capacity per the [Resource Economy](../RESOURCE_ECONOMY.md) field model · crew 1 · food 1 · fuel light · **death blast** when loaded (radius 350 cm, HIGH_EXPLOSIVE 120, U-07) |
| **Commands** | Move, Stop (I); Collect, Deliver (P) · default stance Hold Fire · formation role `convoy` |
| **Production** | **$500** · `MP-BLD-04` · prerequisites: `MP-BLD-09` · **10 s** · 1 food · 1 petroleum · no cap |
| **Veterancy** | none |
| **AI** | `ai.unit.logistics`. Deltas: loop field ↔ MP-BLD-09; abort the trip if the field is contested by observed hostile units |
| **Multiplayer** | **MPA-ECON** (cargo type visible; amount owner-only) |
| **Presentation** | ANIM-VEH-WHEEL + hose connect. Icon of a tanker with a drop |

**Weapons:** none (unarmed)

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.mp.sup_03.collect_crude` | Automated (P; E: access rules) | Loads crude only at a field the participant legally holds or actively contests under the economy rules; proximity alone grants nothing (E) | Eligible field | per economy doc | Deny access; raid the field | Fill gauge | Loop automatically |

**Research and upgrades:** `tech.shared.supply_preservation`: no crude lost when the tanker is damaged above 50 % HP.

**Counters:** *Beaten by (hard):* any raider (RD-ARM-01, IA-ARM-01, AC-ARM-01), artillery and saboteur charges from rival spies late. *Situational:* the death blast makes clumped tanker queues a liability.

**Status:** identity and access rule E-R/E; values P; death blast U-07.

### MP-SUP-04 — Petroleum Product Tanker

| Field | Specification |
| --- | --- |
| **Identity** | `unit.mp.sup_04` · Meridian Pact · Support / product tanker · `TPL-VEH-LOGISTICS` · **Extended** · Playable |
| **Role** | **Primary:** deliver processed fuel to vehicles, clients and market depots (E-R). **Secondary:** forward resupply of fuel-hungry captured armor; contract fuel sales. **Doctrine fit:** fuel supply as a morale and trade driver (E). |
| **Inspiration and background** | A single-trailer petroleum tanker with a pump. C&C lineage (E-R): Harvester (TD, logistics analogue); Supply Truck (Gen); Supply Drop (TT). |
| **Visual identity** | none (asset requirement): white tanker with a green stripe, pump reel. |
| **Statistics** | **480 HP** (`armor.veh_soft`) · WHEEL_HEAVY 360 cm/s · F2 · sight 1000 cm · petroleum capacity 40 · crew 1 · food 1 · fuel light · death blast when loaded (radius 300 cm, HE 100, U-07) |
| **Commands** | Move, Stop (I); Deliver, Refuel Outpost (P) · default stance Hold Fire · formation role `convoy` |
| **Production** | **$450** · `MP-BLD-04` · prerequisites: `MP-BLD-11` · **9 s** · 1 food · 1 petroleum · no cap |
| **Veterancy** | none |
| **AI** | `ai.unit.logistics` |
| **Multiplayer** | **MPA-ECON** (cargo type visible) |
| **Presentation** | ANIM-VEH-WHEEL + pump reel. Icon of a tanker with a fuel can |

**Weapons:** none (unarmed)

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.mp.sup_04.refuel_outpost` | Active (P) | Transfers petroleum to an MP-BLD-02, MP-BLD-11 or MP-BLD-12 so that petroleum-consuming production at the outpost is not stalled by fuel starvation (framework §4.5) | Friendly structure ≤ 400 cm | cargo · 1 unit per s | Kill the tanker en route | Hose VFX | Keep outposts above 10 petroleum |
| `ability.shared.resupply` | Aura (P) | As MP-SUP-02, ammunition only for vehicles; stock 8 | Friendly vehicles | per §10.4 | Kill the tanker | Crate icon | — |

**Research and upgrades:** `tech.shared.supply_preservation`: capacity 40 → 50.

**Counters:** *Beaten by (hard):* raiders, aircraft, artillery. *Situational:* keeps a dispersed Pact economy running during fuel starvation.

**Status:** identity E-R; no mandatory refuelling chore (E: framework §4.5); values P.

### MP-SUP-05 — Commercial Fuel Bowser

| Field | Specification |
| --- | --- |
| **Identity** | `unit.mp.sup_05` · Meridian Pact · Support / kerosene bowser · `TPL-VEH-LOGISTICS` · **Extended** · Playable |
| **Role** | **Primary:** deliver refined kerosene to air assets (E-R). **Secondary:** forward helicopter refuelling at MP-BLD-02. **Doctrine fit:** kerosene and petroleum are distinct inventories (E). |
| **Inspiration and background** | An airport refuelling bowser with a hose boom. C&C lineage (E-R): Chinook (Gen, supply role); Supply Truck (Gen). |
| **Visual identity** | none (asset requirement): yellow airport bowser, hose boom, "JET A-1"-style stencil in fictional script. |
| **Statistics** | **420 HP** (`armor.veh_soft`) · WHEEL_HEAVY 360 cm/s · F2 · sight 1000 cm · kerosene capacity 30 · crew 1 · food 1 · fuel light · death blast when loaded (radius 300 cm, HE 100, U-07) |
| **Commands** | Move, Stop (I); Deliver, Forward Arming Point (P) · default stance Hold Fire · formation role `convoy` |
| **Production** | **$450** · `MP-BLD-04` · prerequisites: `MP-BLD-06` or `MP-BLD-07` · **9 s** · 1 food · 1 petroleum · no cap |
| **Veterancy** | none |
| **AI** | `ai.unit.logistics` |
| **Multiplayer** | **MPA-ECON** (cargo type visible) |
| **Presentation** | ANIM-VEH-WHEEL + boom. Icon of a bowser with a wing |

**Weapons:** none (unarmed)

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.mp.sup_05.forward_arming_point` | Active (P) | Deployed next to an MP-BLD-02, it lets Pact **helicopters** rearm and pay sortie kerosene there instead of at MP-BLD-07; fixed-wing aircraft still need an airstrip | Self, deployed ≤ 400 cm from MP-BLD-02 | kerosene from cargo · while deployed · — | Strike the outpost; the bowser's death blast | Pad marker | Deploy behind the raiding front |

**Research and upgrades:** `tech.shared.supply_preservation`: capacity 30 → 40.

**Counters:** *Beaten by (hard):* anything armed; air strikes. *Situational:* extends MP-AIR-01/02 reach.

**Status:** identity and distinct kerosene stock E-R/E; values P.

### MP-SUP-06 — Cargo Brokerage Van

| Field | Specification |
| --- | --- |
| **Identity** | `unit.mp.sup_06` · Meridian Pact · Support / brokerage van · `TPL-VEH-LOGISTICS` · **Extended** · Playable |
| **Role** | **Primary:** move conventional weapon components with **merchant-paperwork detection** (E-R). **Secondary:** carry the Pact's Black Market stock between hubs without revealing it to casual observers. **Doctrine fit:** commerce and brokerage (E). |
| **Inspiration and background** | A plain van with crates of "agricultural machinery parts" and a broker's paperwork. C&C lineage (E-R): Radar Van (Gen, civilian chassis); Sudden Transport (RA3, concealed cargo role). |
| **Visual identity** | none (asset requirement): white civilian van, no military markings, broker's folder on the dashboard. |
| **Statistics** | **320 HP** (`armor.veh_soft`) · WHEEL_LIGHT 480 cm/s · F1 · sight 1000 cm · cargo 1 component lot · crew 1 · food 1 · fuel light |
| **Commands** | Move, Stop (I); Deliver, Present Papers (P) · default stance Hold Fire · formation role `convoy` |
| **Production** | **$350** · `MP-BLD-04` · prerequisites: `MP-BLD-13` · **8 s** · 1 food · 1 petroleum · no cap |
| **Veterancy** | none |
| **AI** | `ai.unit.logistics`. Deltas: prefer Unaligned civilian roads; stop for checkpoints |
| **Multiplayer** | **MPA-ECON** with explicit `falsified_manifest` state (framework §15): opponents see a civilian van; the true manifest is owner-only until an inspection succeeds |
| **Presentation** | ANIM-VEH-WHEEL. Uses the civilian van appearance profile; icon (owner) of a van with a folder |

**Weapons:** none (unarmed)

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.mp.sup_06.merchant_paperwork` | Passive (P; E-R detection) | Appears to opponents as an Unaligned-style commercial van (`falsified_manifest`). Any `DETECT.SPY` source or Unaligned police inspection within 400 cm reveals the true identity and cargo; detection is the counterplay, not a bypass | Automatic | — | `DETECT.SPY` (AC-INF-11, RD-INF-07, IA-INF-12), Unaligned police and border guards (UN-INF-03, UN-INF-04, UN-INF-11) | Owner sees a "papers" tag | Avoid known detectors |
| `ability.mp.sup_06.component_delivery` | Automated (P) | Delivers a component lot that MP-BLD-05 needs to refit MP-ARM-05/13 or to fill a market order | Assigned delivery | — | Interdiction | ETA (owner) | — |

**Research and upgrades:** none (paperwork does not get better through research; it is a social disguise).

**Counters:** *Beaten by (hard):* any detector unit and the Unaligned counterintelligence (E: strongest counterintelligence), and anything armed once revealed. *Situational:* lets Pact logistics move through roads that military convoys cannot; misuse against the Unaligned raises their response tier (framework §14.3).

**Status:** identity and paperwork detection E-R; `falsified_manifest` state framework P; values P.

### MP-SUP-07 — Mercenary Personnel Carrier

| Field | Specification |
| --- | --- |
| **Identity** | `unit.mp.sup_07` · Meridian Pact · Support / contract troop carrier · `TPL-VEH-TRANSPORT` · **Extended** · Playable |
| **Role** | **Primary:** transport contracted troops while keeping hiring and contract tags (E-R). **Secondary:** delivering hires from MP-BLD-16 straight to the front. **Doctrine fit:** mercenary brokerage (E). |
| **Inspiration and background** | A wheeled armored troop carrier owned by a security company. C&C lineage (E-R): Troop Crawler (Gen); Riptide ACV (RA3); Armadillo (TT). |
| **Visual identity** | none (asset requirement): grey six-wheeler with a company logo instead of the emblem. |
| **Statistics** | **560 HP** (`armor.veh_light`) · WHEEL_HEAVY 400 cm/s · F2 · sight 1200 cm · **6 infantry slots** (contract units only), 2 fire ports · crew 2 · food 2 · fuel medium · ExperienceValue 70 |
| **Commands** | as TPL-VEH-TRANSPORT · default stance Hold Fire · formation role `transport` |
| **Production** | **$700** · `MP-BLD-04` · prerequisites: `MP-BLD-16` · **13 s** · 2 food · 3 petroleum · no cap |
| **Veterancy** | Standard (roof MG) |
| **AI** | `ai.unit.transport` |
| **Multiplayer** | **MPA-TRANSPORT** + MPA-ECON (contract tags carried with the manifest) |
| **Presentation** | ANIM-VEH-WHEEL + ramp. Icon of a carrier with a coin |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.mp.sup_07.roof_mg` | SMALL_ARMS | 0–650 cm | 10 / 0.5 s | hitscan | 2 | GROUND, AIR_LOW | 100 % / 15 % / 2 % / 25 % rotary / 8 % |

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.mp.sup_07.contract_clock_pause` | Passive (P) | Contract units **loaded** in it do not consume contract time (expiry pauses; wages still accrue) | Automatic | — | Kill it loaded | Paused timer (owner) | Move hires in it |

**Research and upgrades:** `tech.mp.contract_arbitration`: +2 slots; trade-off −20 cm/s.

**Counters:** *Beats (soft):* lone infantry. *Beaten by (hard):* AT infantry, autocannon, tanks. *Situational:* saves contract time, so long contracts last longer at the front.

**Status:** identity and contract tags E-R; values P.

### MP-SUP-08 — Repair Caravan

| Field | Specification |
| --- | --- |
| **Identity** | `unit.mp.sup_08` · Meridian Pact · Support / mobile workshop · `TPL-VEH-LOGISTICS` · **Core** · Playable |
| **Role** | **Primary:** workshop unit supporting battered imported vehicles (E-R). **Secondary:** wreck salvage and the tow that feeds MP-ARM-13 capture. **Doctrine fit:** maintaining operations under difficult logistical conditions (E). |
| **Inspiration and background** | A workshop lorry and a small crane trailer, with mechanics who can keep any make running. C&C lineage (E-R): Mobile Repair Vehicle (TS); MRT (KW); Spanner (TT). |
| **Visual identity** | none (asset requirement): olive workshop lorry, crane trailer, spare wheels, welding glow. |
| **Statistics** | **620 HP** (`armor.veh_light`) · WHEEL_HEAVY 360 cm/s · F2 · sight 1000 cm · crew 3 · food 3 · fuel medium |
| **Commands** | Move, Stop, Guard (I); Repair, Salvage, Tow, Swap Mount support (P) · default stance Hold Fire · formation role `support` |
| **Production** | **$750** · `MP-BLD-04` · prerequisites: `MP-BLD-05` · **12 s** · 3 food · 3 petroleum · no cap |
| **Veterancy** | none |
| **AI** | `ai.unit.logistics`. Deltas: follow the army 600 cm behind; repair the most valuable damaged vehicle first; tow eligible heavy wrecks when no enemy is observed within 1500 cm |
| **Multiplayer** | MPA-ECON |
| **Presentation** | ANIM-VEH + crane, welding sparks. Icon of a wrench on a lorry |

**Weapons:** none (unarmed)

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.shared.repair` | Active (P; framework §10.2) | 20 HP/s to one vehicle within 300 cm; full repair costs 25 % of unit cost; cancels the MP-ARM-06 spares surcharge | Friendly vehicle | per §10.2 | Kill the caravan; artillery on the repair point | Sparks | Highest value first |
| `ability.shared.salvage_wreck` | Active (P; framework §10.5) | Converts a wreck within 300 cm into USD equal to 10 % of the wreck's unit cost in 6 s | Wreck | 6 s | Contest the wreck | Crane lift | Salvage between fights |
| `ability.mp.sup_08.tow_wreck` | Active (P) | Tows an eligible enemy heavy or MBT wreck at 50 % speed to MP-BLD-05 for MP-ARM-13 refit (90 s limit) | Eligible wreck | free · ≤ 90 s | Kill or block the tow; enemy recovery vehicles reach it first | Tow cable | Only with escort |

**Research and upgrades:** `tech.shared.vehicle_component_standardization`: repair cost −20 %. `tech.mp.workshop_standards`: repair 20 → 24 HP/s.

**Counters:** *Beaten by (hard):* raids and artillery. *Situational:* the multiplier for every Pact vehicle; the second most valuable target after the commander.

**Status:** identity E-R; salvage and capture rules framework P; values P.

### MP-SUP-09 — Black-Market Delivery Aircraft

| Field | Specification |
| --- | --- |
| **Identity** | `unit.mp.sup_09` · Meridian Pact · Support / market delivery aircraft · `TPL-AIR-TRANSPORT` · **Conditional** (market delivery) · Playable |
| **Role** | **Primary:** brings **purchased conventional cargo only**; interdiction and arrival delay apply (E-R; E: never nuclear). **Secondary:** none. **Doctrine fit:** the Black Market's fast delivery lane (E). |
| **Inspiration and background** | An ageing four-engine freighter chartered from a cargo airline. C&C lineage (E-R): Cargo Plane (TD); Supply Drop (Gen); V-35 Ox (TW). |
| **Visual identity** | none (asset requirement): silver-and-white freighter, faded airline livery, no military markings. |
| **Statistics** | **520 HP** (`armor.air_fixed`) · FIXED_WING **1500 cm/s** · sight 1400 cm · cargo 1 market item (up to one MP-ARM-13 or two light items) · aircrew 3 + 1 (food 4) · kerosene 6 per sortie |
| **Commands** | Move (I); Air-land, Return (P) · default stance Hold Fire · formation role `air_transport` |
| **Production** | Not built: chartered per market order at `MP-BLD-15` for **$250** charter fee, flying from the map edge to `MP-BLD-06` · requires `MP-BLD-06` operational · arrival delay **25 s** after purchase |
| **Veterancy** | none |
| **AI** | `ai.unit.air_transport`. Deltas: fly the dispatch corridor; abort and return the cargo to stock if ≥ 2 enemy AA systems are observed in the corridor |
| **Multiplayer** | **MPA-AIR** + MPA-ECON (cargo owner-only) |
| **Presentation** | ANIM-AIR-FIXED + ramp. Icon of a freighter with a coin |

**Weapons:** none (unarmed)

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.mp.sup_09.air_delivery` | Automated (P; E: physical, interdictable) | Lands at MP-BLD-06 and unloads the purchased item; if shot down the cargo is lost and the charter fee is not refunded. Never carries nuclear items or research (E) | Market order | charter $250 · 25 s delay | AA, interceptors, destroying MP-BLD-06 | Inbound marker (owner) | Abort on a heavy AA corridor |

**Research and upgrades:** none.

**Counters:** *Beaten by (hard):* interceptors (AC-AIR-01, RD-AIR-01, IA-AIR-01), SAMs (RD-ARM-13, AC-ARM-09), any AA on the approach. *Situational:* faster than MP-SUP-02 road delivery, but its corridor is predictable.

**Status:** identity, conventional-only cargo and interdiction E-R/E; charter fee and delay P.

## 6. Aircraft and reconnaissance support

Pact aircraft are few, purchased and dependent on `MP-BLD-06` Open Airstrip (fixed wing) or `MP-BLD-07` Merchant Helipad (rotary). Sortie model per U-04. The Pact has **no nuclear delivery** (E).

### MP-AIR-01 — Light Utility Helicopter

| Field | Specification |
| --- | --- |
| **Identity** | `unit.mp.air_01` · Meridian Pact · Air / utility helicopter · `TPL-AIR-HELI-TRANSPORT` · **Extended** · Playable |
| **Role** | **Primary:** reconnaissance, supply and courier flights, **not superior to the opening scout trucks** (E-R). **Secondary:** lifting one team to a hilltop or across a river. **Doctrine fit:** rapid deployment across difficult terrain (E). |
| **Inspiration and background** | A light civil helicopter in commercial service, with a door gunner on contract flights. C&C lineage (E-R): Chinook (RA1); Transport Helicopter (Ren); NightHawk (RA2). |
| **Visual identity** | none (asset requirement): small white-and-green helicopter, skids, sliding door. |
| **Statistics** | **450 HP** (`armor.air_rotary`) · ROTARY **650 cm/s** · sight 1400 cm · **2 infantry slots** · aircrew 2 + 1 (food 3) · kerosene 2 per sortie |
| **Commands** | as TPL-AIR-HELI-TRANSPORT + Resupply Drop (P) · default stance Hold Fire · formation role `air_transport` |
| **Production** | **$900** (band low edge) · `MP-BLD-07` · **14 s** · 3 food · kerosene 2 per sortie · no cap |
| **Veterancy** | none (unarmed) |
| **AI** | `ai.unit.air_transport`. Deltas: never used as the opening scout by the AI (the trucks are cheaper and better, E) |
| **Multiplayer** | MPA-AIR + MPA-TRANSPORT |
| **Presentation** | ANIM-AIR-ROTARY + door. Icon of a small helicopter |

**Weapons:** none (unarmed)

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.mp.air_01.resupply_drop` | Active (P) | Drops a crate that resupplies 4 limited-ammunition units within 400 cm (framework §10.4) and restocks 1 charge for MP-INF-08/09 | Point within 600 cm | $80 · instant · 60 s | AA, MGs during the hover | Crate parachute | Support raiders far from base |

**Research and upgrades:** `tech.shared.radar_signal_processing`: sight 1400 → 1700 cm (still below MP-ARM-01's 1800 cm).

**Counters:** *Beaten by (hard):* any AA (RD-ARM-06, IA-ARM-08, AC-ARM-09, UN-ARM-08), HMGs, rotary-capable infantry fire. *Situational:* keeps deep MP-INF-08/10 raids alive.

**Status:** identity and "not superior to the trucks" E-R/E; values P.

### MP-AIR-02 — Armed Escort Helicopter

| Field | Specification |
| --- | --- |
| **Identity** | `unit.mp.air_02` · Meridian Pact · Air / hired escort helicopter · `TPL-AIR-HELI-ATTACK` · **Core** · Playable (contract) |
| **Role** | **Primary:** basic **hired** helicopter escort for convoys (E-R). **Secondary:** anti-vehicle support for raids. **Doctrine fit:** contracts and convoy protection (E). |
| **Inspiration and background** | A medium helicopter with rocket pods and a nose gun, flown by contract aircrew. C&C lineage (E-R): Hind (RA1); Twinblade (RA3); Harpy (TS). |
| **Visual identity** | none (asset requirement): sand-and-olive helicopter, rocket pods, contractor tail number. |
| **Statistics** | **480 HP** (`armor.air_rotary`) · ROTARY **700 cm/s** · sight 1600 cm · aircrew 2 + 1 (food 3) · kerosene 3 per sortie · ExperienceValue 120 |
| **Commands** | as TPL-AIR-HELI-ATTACK + Escort (P) · default stance Aggressive · formation role `air_escort` |
| **Production** | Hire **$1,200** + wage **$30 per interval** · contract **360 s** (renewable once) · `MP-BLD-16` (delivered to `MP-BLD-07`) · prerequisites: `MP-BLD-07` · **18 s** · 3 food · no cap |
| **Veterancy** | Standard (for the contract's life) |
| **AI** | `ai.unit.attack_helicopter`. Deltas: *Escort* the assigned convoy; never leave 1500 cm of it; retreat on AA lock |
| **Multiplayer** | MPA-AIR + MPA-ECON (contract) |
| **Presentation** | ANIM-AIR-ROTARY + rocket ripple. Contract timer (owner); icon of a helicopter with a coin |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.mp.air_02.rockets` | HIGH_EXPLOSIVE | 0–1000 cm | 4 × 45, radius 150 cm / 6 s; magazine 4 salvos, rearm at MP-BLD-07 or MP-SUP-05 | projectile, unguided | 8 | GROUND, STRUCTURE | 110 % / 45 % / 12 % / — / 100 % |
| `weapon.mp.air_02.nose_gun` | AUTOCANNON | 0–900 cm | 18 / 0.4 s | projectile, fast | 4 | GROUND, AIR_LOW | 65 % / 85 % / 10 % / 100 % rotary / 30 % |

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.mp.air_02.escort` | Active (P) | Assigned to a convoy or unit: follows it and auto-engages its attackers; +10 % damage against units that damaged the escorted target in the last 5 s | Friendly unit | free · until changed | Pull it away with bait; AA near the route | Escort link (owner) | Always on convoys |
| `ability.mp.contract_terms` | Passive (E-R metadata; P values) | As MP-INF-02 | Automatic | wage | USD pressure | Contract timer | — |

**Research and upgrades:** `tech.mp.contract_arbitration`: contract 360 → 504 s; wage +$5.

**Counters:** *Beats (hard):* technicals, trucks and APCs (RD-ARM-02/03, IA-ARM-01/02, AC-ARM-01). *Beats (soft):* infantry in the open. *Beaten by (hard):* mobile AA (RD-ARM-06/13, IA-ARM-08, AC-ARM-09), interceptors against helicopters only where eligible, AA emplacements. *Situational:* a rented air arm; expensive over long matches.

**Status:** identity and contract E-R; values P.

### MP-AIR-03 — Imported Fighter-Bomber

| Field | Specification |
| --- | --- |
| **Identity** | `unit.mp.air_03` · Meridian Pact · Air / fighter-bomber · `TPL-AIR-STRIKE` · **Extended** · Playable |
| **Role** | **Primary:** limited late-technology tactical air sorties using purchased components (E-R). **Secondary:** air-to-air self-defense against helicopters and slow aircraft. **Doctrine fit:** imported technology and licensed production (E). |
| **Inspiration and background** | An export-model second-generation fighter-bomber assembled from purchased kits. C&C lineage (E-R): Harrier (RA2); Yak (RA1); Vindicator (RA3). |
| **Visual identity** | none (asset requirement): swept-wing jet in sand-and-blue camouflage, two bombs and two IR missiles. |
| **Statistics** | **400 HP** (`armor.air_fixed`) · FIXED_WING **1800 cm/s** · sight 1800 cm · aircrew 1 + 1 (food 2) · kerosene 5 per sortie · ExperienceValue 170 |
| **Commands** | as TPL-AIR-STRIKE · default stance Hold Fire until ordered · formation role `air_strike` |
| **Production** | **$1,700** · `MP-BLD-06` · prerequisites: `MP-BLD-14` and a component lot delivered by MP-SUP-06 (one per airframe) · **26 s** · 2 food · kerosene 5 per sortie · **limit 4** |
| **Veterancy** | Standard |
| **AI** | `ai.unit.air_strike` |
| **Multiplayer** | MPA-AIR |
| **Presentation** | ANIM-AIR-FIXED + bomb release. Icon of a jet with a bomb |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.mp.air_03.bombs` | HIGH_EXPLOSIVE | overflight | 2 × 220, radius 300 cm, per sortie | bomb, overflight scatter 150 cm | 15 | GROUND, STRUCTURE | 110 % / 45 % / 12 % / — / 100 % |
| `weapon.mp.air_03.ir_missiles` | FLAK_AA | 0–1200 cm | 2 × 140 per sortie | guided (jammable, §9.5) | — | AIR_LOW, AIR_HIGH | — / — / — / 110 % rotary, 100 % fixed / — |

**Abilities:** none unit-specific; the cost is in the component lot and the airframe limit.

**Research and upgrades:** `tech.mp.modular_commercial_equipment`: alternative rocket-pod loadout (SHAPED_CHARGE 6 × 70) instead of bombs. `tech.shared.electronic_countermeasures`: one flare decoy per sortie (first guided AA hit is negated).

**Counters:** *Beats (hard):* artillery, logistics and static AA gaps (RD-ARM-07, AC-ARM-07, RD-SUP-03). *Beats (soft):* helicopters. *Beaten by (hard):* interceptors (AC-AIR-01, RD-AIR-01, IA-AIR-01), SAM carriers (AC-ARM-09, RD-ARM-13). *Situational:* a few precious sorties; losing airframes is costly to replace because of the component lot.

**Status:** identity and purchased components E-R; values P.

### MP-AIR-04 — Cargo Transport Plane

| Field | Specification |
| --- | --- |
| **Identity** | `unit.mp.air_04` · Meridian Pact · Air / cargo transport · `TPL-AIR-TRANSPORT` · **Extended** · Playable |
| **Role** | **Primary:** move contracted assets and vital merchant cargo between airstrips (E-R). **Secondary:** air-landing troops at a second MP-BLD-06. **Doctrine fit:** transport between geographically separated regions (E). |
| **Inspiration and background** | A twin-turboprop tactical freighter owned by a regional airline. C&C lineage (E-R): Cargo Plane (TD); V-35 Ox (TW); Century Bomber (RA3, transport role). |
| **Visual identity** | none (asset requirement): high-wing twin-prop freighter, rear ramp, regional-airline livery. |
| **Statistics** | **600 HP** (`armor.air_fixed`) · FIXED_WING **1400 cm/s** · sight 1400 cm · **8 infantry slots or 1 light vehicle** · aircrew 3 + 1 (food 4) · kerosene 4 per sortie |
| **Commands** | as TPL-AIR-TRANSPORT (air-land only; no paradrop) · default stance Hold Fire · formation role `air_transport` |
| **Production** | **$1,300** · `MP-BLD-06` · **20 s** · 4 food · kerosene 4 per sortie · limit 2 |
| **Veterancy** | none |
| **AI** | `ai.unit.air_transport` |
| **Multiplayer** | MPA-AIR + MPA-TRANSPORT |
| **Presentation** | ANIM-AIR-FIXED + ramp. Icon of a twin-prop freighter |

**Weapons:** none (unarmed)

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.mp.air_04.strip_to_strip` | Active (P) | Air-lands its load at another friendly MP-BLD-06 (or a captured airfield) | Friendly airstrip | kerosene 4 · flight time | AA on the approach; destroy the strip | Route line (owner) | Reinforce a distant base |

**Research and upgrades:** `tech.shared.supply_preservation`: kerosene per sortie 4 → 3.

**Counters:** *Beaten by (hard):* interceptors and SAMs. *Situational:* links dispersed bases on large maps.

**Status:** identity E-R; values P.

### MP-AIR-05 — Coastal Patrol Aircraft

| Field | Specification |
| --- | --- |
| **Identity** | `unit.mp.air_05` · Meridian Pact · Air / maritime patrol · `TPL-AIR-SURVEILLANCE` · **Extended** · Playable |
| **Role** | **Primary:** detect shipping and verify reports at extended distance (E-R). **Secondary:** submarine search over coastal water. **Doctrine fit:** maritime trade security (E). Verification uses ordinary game evidence; it never turns a planted report into a fact by itself (E). |
| **Inspiration and background** | A twin-engine maritime patrol aircraft with a search radar, used for fisheries and smuggling patrols. C&C lineage (E-R): Spy Plane (RA1); ORCA (TW, reconnaissance role); Surveillance Sweep (RA3). |
| **Visual identity** | none (asset requirement): white-and-grey twin-prop plane with a belly radome. |
| **Statistics** | **420 HP** (`armor.air_fixed`) · FIXED_WING (loiter) 1200 cm/s · sight **3200 cm** over water, 2000 cm over land · `DETECT.SUBMERGED` 1200 cm (sonobuoy, while orbiting water) · aircrew 3 + 1 (food 4) · kerosene 3 per sortie, loiter 90 s |
| **Commands** | as TPL-AIR-SURVEILLANCE · default stance Hold Fire · formation role `air_sensor` |
| **Production** | **$1,100** · `MP-BLD-06` · prerequisites: `MP-BLD-08` · **18 s** · 4 food · no cap |
| **Veterancy** | none (unarmed) |
| **AI** | `ai.unit.air_surveillance`. Deltas: orbit trade routes; retreat on interceptor contact |
| **Multiplayer** | MPA-AIR + MPA-SENSOR |
| **Presentation** | ANIM-AIR-FIXED + radome spin. Icon of a plane over a wave |

**Weapons:** none (unarmed)

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.mp.air_05.verify_report` | Active (P; E: investigation through game evidence) | Orbiting the location of a received intelligence report for 10 s observes whatever is actually there; the UI then shows "observed" or "nothing observed". It never reveals who planted the report | Report location ≤ 3200 cm | free · 10 s · 30 s | Interceptors; AA at the site; jamming suppresses radar contacts (§9.5) | Report card status change | Verify reports about naval threats |

**Research and upgrades:** `tech.shared.radar_signal_processing`: loiter 90 → 120 s.

**Counters:** *Beats (situational):* submarines and stealthy boats (AC-SEA-05, RD-SEA-04, IA-SEA-04 near the surface). *Beaten by (hard):* interceptors, ship SAMs (RD-SEA-05, AC-SEA-04). *Situational:* the Pact's answer to Unaligned false naval reports.

**Status:** identity E-R; report handling E; values P.

### MP-AIR-06 — Hired Air Support Flight

| Field | Specification |
| --- | --- |
| **Identity** | `unit.mp.air_06` · Meridian Pact · Air / contracted strike flight · `TPL-AIR-STRIKE` · **Conditional** (contract) · Playable |
| **Role** | **Primary:** externally contracted **temporary** strike, **not an indigenous nuclear bomber** (E-R). **Secondary:** none. **Doctrine fit:** contracting, no nuclear path (E). Differentiated from MP-MKT-07 as a **recurring contract** (U-15). |
| **Inspiration and background** | Two ground-attack jets flown by a foreign contract squadron on a retainer. C&C lineage (E-R): A-10 Strike (Gen); Spectre Gunship (Gen); Orca Strike (TW). |
| **Visual identity** | none (asset requirement): two grey jets with contractor tail codes; they enter from the map edge. |
| **Statistics** | 2 aircraft × **380 HP** (`armor.air_fixed`) · FIXED_WING 1900 cm/s · sight 1800 cm · aircrew 2 (food 2 during the contract) · kerosene: paid by the contractor |
| **Commands** | Strike Area (P) only · formation role `air_strike` |
| **Production** | Retainer **$900** + **$400 per sortie** · contract **300 s**, at most 3 sorties per contract · `MP-BLD-16` · prerequisites: `MP-BLD-06` operational · sortie lead time 15 s (aircraft enter from the map edge) |
| **Veterancy** | none (contract crews rotate) |
| **AI** | `ai.unit.air_strike`. Deltas: one pass, then exit at the map edge |
| **Multiplayer** | MPA-AIR + MPA-ECON (contract) |
| **Presentation** | ANIM-AIR-FIXED. Inbound warning to enemies in the strike area 3 s before release (counterplay tell). Icon of two jets with a coin |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.mp.air_06.strafe` | AUTOCANNON | strafe line 1200 cm × 200 cm | 2 × 16 hits × 30 | strafe | 10 | GROUND | 65 % / 85 % / 10 % / — / 30 % |
| `weapon.mp.air_06.rockets` | SHAPED_CHARGE | strafe line | 2 × 4 × 90 | projectile | 4 | GROUND, STRUCTURE | 25 % / 100 % / 90 % / — / 70 % |

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.mp.air_06.call_sortie` | Active (P) | Orders one strafing pass along a line the owner observes | Observed line | $400 · 15 s lead · 45 s between sorties | AA on the approach (aircraft can be shot down; the contract ends if both are lost); dispersal after the warning | Inbound marker; enemy warning 3 s | Use on observed convoys and artillery |
| `ability.mp.contract_terms` | Passive (E-R metadata) | As MP-INF-02, with retainer instead of wage | Automatic | retainer | — | Contract timer | — |

**Research and upgrades:** none (external contractor).

**Counters:** *Beats (hard):* convoys, artillery lines and light vehicles caught on roads. *Beaten by (hard):* mobile AA (RD-ARM-06/13, IA-ARM-08, AC-ARM-09) and dispersal after the warning. *Situational:* firepower the Pact cannot build, rented by the minute; never nuclear (E).

**Status:** identity, temporary contract and "not nuclear" E-R/E; differentiation from MP-MKT-07 per U-15 P; values P.

## 7. Naval and amphibious commerce

All Pact naval units need an authored water layer and `MP-BLD-08` Harbor Exchange (Conditional). MP-SEA-04 also works on river maps. Pact naval power protects trade; it never gains a nuclear or superweapon role (E).

### MP-SEA-01 — Merchant Coastal Barge

| Field | Specification |
| --- | --- |
| **Identity** | `unit.mp.sea_01` · Meridian Pact · Naval / cargo barge · `TPL-NAV-TRANSPORT` · **Conditional** · Playable |
| **Role** | **Primary:** move goods and units across the Adriatic or navigable rivers (E-R). **Secondary:** sea delivery lane for market items (framework §11.5). **Doctrine fit:** dependence on commercial shipping (E). |
| **Inspiration and background** | A motor coaster or river barge with a deck crane. C&C lineage (E-R): Transport Ship (RA1); Amphibious Transport (RA2); Hovercraft (TD). |
| **Visual identity** | none (asset requirement): rust-red hull, deck crane, stacked crates, company flag. |
| **Statistics** | **700 HP** (`armor.nav_light`) · SURFACE_SHALLOW **420 cm/s** · sight 1200 cm · **6 infantry slots or 2 light vehicles** or 1 market item · crew 4 (food 4) · fuel medium |
| **Commands** | as TPL-NAV-TRANSPORT (docks only; no beaching) · default stance Hold Fire · formation role `convoy` |
| **Production** | **$700** · `MP-BLD-08` · **16 s** · 4 food · 3 petroleum · no cap |
| **Veterancy** | none |
| **AI** | `ai.unit.transport` (naval). Deltas: follow coastal routes covered by friendly patrol boats |
| **Multiplayer** | MPA-TRANSPORT + MPA-ECON |
| **Presentation** | ANIM-NAV + crane. Icon of a barge |

**Weapons:** none (unarmed)

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.mp.sea_01.dock_transfer` | Active (P) | Loads or unloads at any friendly harbor, MP-SEA-07 anchorage or neutral port with permission, 1 s per slot | Dock | free | Strike the dock; blockade | Crane VFX | Deliver to the nearest dock to the buyer |

**Research and upgrades:** `tech.shared.supply_preservation`: +2 slots.

**Counters:** *Beaten by (hard):* torpedo boats and submarines (RD-SEA-03/04, AC-SEA-05, IA-SEA-03/04), attack aircraft. *Situational:* the cheapest heavy lift on water maps.

**Status:** identity E-R; values P.

### MP-SEA-02 — Fast Maritime Scout

| Field | Specification |
| --- | --- |
| **Identity** | `unit.mp.sea_02` · Meridian Pact · Naval / fast scout boat · `TPL-NAV-PATROL` · **Conditional** · Playable |
| **Role** | **Primary:** rapid reconnaissance and anti-boat defense (E-R). **Secondary:** escorting barges. **Doctrine fit:** the Pact's reconnaissance identity at sea (E). |
| **Inspiration and background** | A fast fibreglass patrol boat with a heavy MG. C&C lineage (E-R): Gunboat (RA1); Yari Mini-Sub (RA3, scout analogy). |
| **Visual identity** | none (asset requirement): white boat, green stripe, twin outboards, MG on the bow. |
| **Statistics** | **320 HP** (`armor.nav_light`) · SURFACE_SHALLOW **720 cm/s** (above band: fastest Pact hull) · sight **1800 cm** · `DETECT.CONCEALED` 60 % of sight · crew 3 (food 3) · fuel light · ExperienceValue 55 |
| **Commands** | as TPL-NAV-PATROL · default stance Aggressive · formation role `scout_screen` |
| **Production** | **$500** (band low edge) · `MP-BLD-08` · **12 s** · 3 food · 1 petroleum · no cap |
| **Veterancy** | Standard |
| **AI** | `ai.unit.naval_patrol` with scout flee rules |
| **Multiplayer** | MPA-STD |
| **Presentation** | ANIM-NAV + wake spray. Icon of a speedboat |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.mp.sea_02.hmg` | SMALL_ARMS | 0–750 cm | 16 / 0.4 s | hitscan | 4 | GROUND (shore), NAVAL, AIR_LOW | 100 % / 15 % / 2 % / 25 % rotary / 8 %; NAV_L 30 % |

**Abilities:** none unit-specific; speed and sight are its role.

**Research and upgrades:** `tech.mp.modular_commercial_equipment`: HMG → twin autocannon (AUTOCANNON 2 × 14 / 0.6 s, NAV_L 90 %); trade-off −60 cm/s.

**Counters:** *Beats (hard):* landing craft and launches (AC-SEA-01, IA-SEA-02, RD-SEA-02 unescorted). *Beaten by (hard):* missile boats and gunboats (AC-SEA-03, IA-SEA-05, RD-SEA-03), helicopters. *Situational:* finds submarines only with MP-AIR-05.

**Status:** identity E-R; values P.

### MP-SEA-03 — Contract Security Patrol Boat

| Field | Specification |
| --- | --- |
| **Identity** | `unit.mp.sea_03` · Meridian Pact · Naval / contract patrol boat · `TPL-NAV-PATROL` · **Conditional** (contract) · Playable |
| **Role** | **Primary:** escort merchant convoys as a **billable service** (E-R). **Secondary:** harbor defense. **Doctrine fit:** contracts and maritime trade security (E). |
| **Inspiration and background** | A security company's patrol boat with an autocannon. C&C lineage (E-R): Sea Scorpion (RA2); Hydrofoil (RA3); Gunboat (TD). |
| **Visual identity** | none (asset requirement): grey patrol boat, company logo, bow autocannon. |
| **Statistics** | **480 HP** (`armor.nav_light`) · SURFACE_SHALLOW 560 cm/s · sight 1600 cm · crew 5 (food 5) · ExperienceValue 70 |
| **Commands** | as TPL-NAV-PATROL + Escort (P) · default stance Aggressive · formation role `escort` |
| **Production** | Hire **$600** + wage **$20 per interval** · contract **360 s** · `MP-BLD-16` (delivered at `MP-BLD-08`) · prerequisites: `MP-BLD-08` · **10 s** · 5 food |
| **Veterancy** | Standard (contract life) |
| **AI** | `ai.unit.naval_patrol`. Deltas: *Escort* the assigned barge |
| **Multiplayer** | MPA-STD + MPA-ECON (contract) |
| **Presentation** | ANIM-NAV + turret. Contract timer (owner) |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.mp.sea_03.autocannon` | AUTOCANNON | 0–900 cm | 2 × 16 / 0.6 s | projectile, fast | 4 | NAVAL, GROUND (shore), AIR_LOW | 65 % / 85 % / 10 % / 100 % rotary / 30 %; NAV_L 90 %, NAV_H 20 % |

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.mp.sea_03.convoy_escort` | Active (P) | Escorted vessel takes −15 % damage from NAV_LIGHT attackers while the boat is within 600 cm | Friendly vessel | free · while escorting | Torpedoes and aircraft (unaffected) | Escort link | Always on barges |
| `ability.mp.contract_terms` | Passive (E-R metadata) | As MP-INF-02 | Automatic | wage | — | Contract timer | — |

**Research and upgrades:** `tech.mp.contract_arbitration`.

**Counters:** *Beats (hard):* raider boats (IA-SEA-03), launches. *Beaten by (hard):* missile boats, frigates, submarines, aircraft. *Situational:* rented convoy safety.

**Status:** identity and billable contract E-R; values P.

### MP-SEA-04 — Amphibious Cargo Carrier

| Field | Specification |
| --- | --- |
| **Identity** | `unit.mp.sea_04` · Meridian Pact · Naval / amphibious carrier · `TPL-NAV-TRANSPORT` (AMPHIBIOUS) · **Conditional** (water or river maps) · Playable |
| **Role** | **Primary:** transfer light vehicles and troops to shoreline roads (E-R). **Secondary:** river crossings where no bridge stands. **Doctrine fit:** amphibious units and river operations (E). |
| **Inspiration and background** | A wheeled amphibious lorry. C&C lineage (E-R): Riptide ACV (RA3); Amphibious APC (TS); Bullfrog (RA3, troop handling). |
| **Visual identity** | none (asset requirement): boat-hulled six-wheeler, olive with a white waterline. |
| **Statistics** | **520 HP** (`armor.veh_light` on land, `armor.nav_light` afloat) · AMPHIBIOUS 360 cm/s land, 300 cm/s water · sight 1200 cm · **4 infantry slots or 1 light vehicle** · crew 2 (food 2) · fuel medium |
| **Commands** | as TPL-NAV-TRANSPORT + Beach (P) · default stance Hold Fire · formation role `transport` |
| **Production** | **$800** · `MP-BLD-08` or `MP-BLD-04` (river maps) · **14 s** · 2 food · 3 petroleum · no cap |
| **Veterancy** | none |
| **AI** | `ai.unit.transport` |
| **Multiplayer** | MPA-TRANSPORT |
| **Presentation** | ANIM-VEH-WHEEL + water transition. Icon of a wheeled boat |

**Weapons:** none (unarmed)

**Abilities:** none unit-specific; amphibious movement is its role (framework §5.2).

**Research and upgrades:** `tech.mp.modular_commercial_equipment`: roof MG (SMALL_ARMS 10 / 0.5 s); trade-off −1 slot.

**Counters:** *Beaten by (hard):* any armed boat, shore AT (MP-DEF-03-class rival defenses), aircraft. *Situational:* opens river flanks for MP-INF-08 raids.

**Status:** identity E-R; values P.

### MP-SEA-05 — Surplus Coastal Submarine

| Field | Specification |
| --- | --- |
| **Identity** | `unit.mp.sea_05` · Meridian Pact · Naval / diesel submarine · `TPL-NAV-SUB` · **Conditional** · Playable |
| **Role** | **Primary:** scarce conventional underwater acquisition with **large upkeep** (E-R). **Secondary:** blockading rival shipping. **Doctrine fit:** imported conventional systems; **no nuclear missiles** (E). |
| **Inspiration and background** | An old diesel-electric coastal submarine bought from a navy retiring it. C&C lineage (E-R): Submarine (RA1); Typhoon Submarine (RA2); Boomer Submarine (YR, torpedo role). |
| **Visual identity** | none (asset requirement): black hull, sand-coloured sail band, faded export number. |
| **Statistics** | **900 HP** (`armor.nav_heavy`) · SUBSURFACE 320 cm/s · sight 1600 cm (surfaced) · crew 6 abstracted (food 6) · fuel medium · ExperienceValue 190 |
| **Commands** | as TPL-NAV-SUB · default stance Hold Fire while submerged · formation role `naval_strike` |
| **Production** | **$1,900** · `MP-BLD-08` · prerequisites: `MP-BLD-14` · **40 s** · 6 food · 5 petroleum · **limit 2** · **upkeep $20 per interval** (E-R: large upkeep) |
| **Veterancy** | Standard |
| **AI** | `ai.unit.submarine` |
| **Multiplayer** | **MPA-COVERT** (submerged state) |
| **Presentation** | ANIM-NAV-SUB. Icon of a submarine with a price tag |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.mp.sea_05.torpedo` | TORPEDO | 0–1400 cm | 260 / 8 s; magazine 6, rearm at MP-BLD-08 | projectile, slow, homing (not jammable underwater) | — | NAVAL | NAV_L 110 % / NAV_H 100 % |

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.mp.sea_05.dive` | Active (P; framework §9.1 Submerged) | Submerged: observable only by `DETECT.SUBMERGED`; firing reveals it for 4 s | Self | free · 3 s dive | Frigates, patrol aircraft, sonar | Periscope wake (owner) | Stay submerged until a firing solution |

**Research and upgrades:** none (spares limited).

**Counters:** *Beats (hard):* barges, landing craft, unescorted surface ships (AC-SEA-02, RD-SEA-07, IA-SEA-06). *Beaten by (hard):* ASW (AC-SEA-04, UN-SEA-03), patrol aircraft (UN-AIR-08). *Situational:* a costly threat that ties down rival escorts.

**Status:** identity, scarcity and upkeep E-R; values P.

### MP-SEA-06 — Licensed Missile Corvette

| Field | Specification |
| --- | --- |
| **Identity** | `unit.mp.sea_06` · Meridian Pact · Naval / missile corvette · `TPL-NAV-COMBATANT` · **Conditional** · Playable |
| **Role** | **Primary:** export-model missile craft with **restricted availability** (E-R). **Secondary:** fleet air defense for convoys. **Doctrine fit:** missile-equipped boats protect commercial routes (E). |
| **Inspiration and background** | A licence-built missile corvette with four anti-ship missiles and a point-defense gun. C&C lineage (E-R): Destroyer (RA1); Naginata Cruiser (RA3, ship-combat role). |
| **Visual identity** | none (asset requirement): grey corvette, missile canisters amidships, green pennant. |
| **Statistics** | **1,500 HP** (`armor.nav_heavy`; band low) · SURFACE_DEEP 360 cm/s · sight 2000 cm · `DETECT.AIR_RADAR` 2600 cm · crew 12 abstracted (food 12) · fuel heavy · ExperienceValue 220 |
| **Commands** | as TPL-NAV-COMBATANT · default stance Aggressive · formation role `naval_line` |
| **Production** | **$2,200** · `MP-BLD-08` · prerequisites: `MP-BLD-14` and `MP-BLD-13` (licence) · **40 s** · 12 food · 5 petroleum · **limit 2** |
| **Veterancy** | Standard |
| **AI** | `ai.unit.naval_combatant` |
| **Multiplayer** | MPA-STD + MPA-SENSOR |
| **Presentation** | ANIM-NAV + missile launch. Icon of a corvette |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.mp.sea_06.ssm` | ANTI_SHIP | 0–2600 cm | 380 per missile / 6 s; **4 missiles**, rearm at MP-BLD-08 | guided (jammable, §9.5) | — | NAVAL, coastal STRUCTURE | NAV_L 120 % / NAV_H 100 % / STR 60 % |
| `weapon.mp.sea_06.ciws` | AUTOCANNON | 0–1000 cm | 18 / 0.3 s | projectile | 3 | AIR_LOW, AIR_HIGH, NAVAL | 100 % rotary / 60 % fixed; NAV_L 90 % |

**Abilities:** none unit-specific; the finite missile load is the constraint.

**Research and upgrades:** `tech.shared.electronic_countermeasures`: one chaff decoy per engagement.

**Counters:** *Beats (hard):* patrol boats and gunboats (AC-SEA-03, IA-SEA-05, RD-SEA-03). *Beaten by (hard):* submarines (AC-SEA-05, RD-SEA-04, IA-SEA-04), strike aircraft in numbers. *Situational:* four missiles decide a naval skirmish; then it must go home.

**Status:** identity and restricted availability E-R; values P.

### MP-SEA-07 — Floating Trade Anchorage

| Field | Specification |
| --- | --- |
| **Identity** | `unit.mp.sea_07` · Meridian Pact · Naval / forward logistics hub · `TPL-NAV-TRANSPORT` · **Conditional** · Playable |
| **Role** | **Primary:** deployable short-range forward maritime logistics hub; **not an armed superweapon** (E-R). **Secondary:** forward contract office at sea. **Doctrine fit:** forward supply installations across separated regions (E). |
| **Inspiration and background** | A converted cargo ship with cranes, fuel bunkers and a floating pier. C&C lineage (E-R): Support Crawler (TT, mobile HQ role); Naval Yard (RA1); Aircraft Carrier (RA2, sea hub). |
| **Visual identity** | none (asset requirement): long cargo ship, three cranes, floating pier that unfolds. |
| **Statistics** | **1,800 HP** (`armor.nav_heavy`) · SURFACE_DEEP 260 cm/s (mobile only) · sight 1600 cm · crew 10 abstracted (food 10) · fuel heavy |
| **Commands** | Move, Stop (I); Anchor, Weigh Anchor (P) · default stance Hold Fire · formation role `hub` |
| **Production** | **$2,000** · `MP-BLD-08` · prerequisites: `MP-BLD-17` · **45 s** · 10 food · 5 petroleum · **cap 1** |
| **Veterancy** | none (unarmed) |
| **AI** | `ai.unit.logistics`. Deltas: anchor ≥ 2000 cm from enemy shore defenses |
| **Multiplayer** | MPA-ECON |
| **Presentation** | ANIM-NAV + pier unfold. Icon of an anchor over a crate |

**Weapons:** none (unarmed)

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.mp.sea_07.anchor` | Active (P) | While anchored (20 s to deploy): acts as a dock for MP-SEA-01/04, repairs Pact vessels within 800 cm at 12 HP/s (§10.2 cost rule), resupplies naval ammunition, and can produce MP-SEA-02 and MP-SEA-04 | Self, in water | free · 20 s deploy · 60 s re-anchor | Submarines and missile boats; air strikes | Pier unfold | Anchor near the contested coast with escorts |

**Research and upgrades:** `tech.shared.supply_preservation`: repair 12 → 15 HP/s.

**Counters:** *Beaten by (hard):* submarines, anti-ship missiles (AC-SEA-03/04, UN-SEA-02), bombers. *Situational:* lets a Pact navy fight far from MP-BLD-08.

**Status:** identity and "not a superweapon" E-R; values P; anchorage parameters **U-MP-07**.

## 8. Market-exclusive assets (conventional, brokered rather than native)

Market assets are bought at `MP-BLD-15` Black Market Auction Hall against **finite auction stock** (stock per match listed per item, P), arrive physically through MP-SUP-02, MP-SUP-09 or MP-SEA-01 and can be interdicted (E). The market is **conventional only** and never lists nuclear weapons or research (E). No market unit unlocks a native factory, research or rival production (E-R). Prices below are the opening auction price; the auction model is owned by the economy documents.

### MP-MKT-01 — Imported Heavy Recovery Crawler

| Field | Specification |
| --- | --- |
| **Identity** | `unit.mp.mkt_01` · Meridian Pact · Market / heavy recovery vehicle · `TPL-VEH-LOGISTICS` (tracked) · **Conditional** (market) · Playable |
| **Role** | **Primary:** **auction or contract only**: powerful armored towing and recovery; no faction factory produces this composite variant (E-R). **Secondary:** fast acquisition route for MP-ARM-13. **Doctrine fit:** imported equipment (E). |
| **Inspiration and background** | A tracked heavy recovery vehicle assembled from two foreign designs. C&C lineage (E-R): Mobile Repair Transport (KW); Rig (TW); Crusher Crane (RA3, recovery function). |
| **Visual identity** | none (asset requirement): heavy tracked hull, A-frame crane, dozer blade, mixed paint. |
| **Statistics** | **1,100 HP** (`armor.veh_heavy`) · TRACKED 300 cm/s · sight 1000 cm · crew 3 (food 3) · fuel heavy |
| **Commands** | Move, Stop, Guard (I); Repair, Salvage, Tow (P) · default stance Hold Fire · formation role `support` |
| **Production** | Market **$1,400** · stock **1** · delivery by MP-SUP-02 or MP-SEA-01 |
| **Veterancy** | none |
| **AI** | `ai.unit.logistics` |
| **Multiplayer** | MPA-ECON |
| **Presentation** | ANIM-VEH-TRACK + crane. Icon of a crane on tracks |

**Weapons:** none (unarmed)

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.mp.mkt_01.heavy_tow` | Active (P) | As MP-SUP-08 *Tow Wreck* at **80 %** speed and with a 120 s limit; can tow under fire | Eligible wreck | free | Heavy AT fire | Tow cable | Grab heavy wrecks after a battle |
| `ability.shared.repair` | Active (P) | 24 HP/s to one vehicle within 300 cm (§10.2 cost rule) | Friendly vehicle | per §10.2 | Artillery on the repair point | Sparks | — |
| `ability.shared.salvage_wreck` | Active (P) | As MP-SUP-08 | Wreck | 6 s | — | Crane lift | — |

**Research and upgrades:** none (market item).

**Counters:** *Beaten by (hard):* ATGMs and tank guns. *Situational:* makes MP-ARM-13 capture practical in the middle of a fight.

**Status:** identity and market-only E-R; values and stock P.

### MP-MKT-02 — Experimental Mine-Clearing Robot

| Field | Specification |
| --- | --- |
| **Identity** | `unit.mp.mkt_02` · Meridian Pact · Market / remote mine clearer · `TPL-VEH-ENGINEER` · **Conditional** (market) · Playable |
| **Role** | **Primary:** **market-only** remote tracked mine detector and clearer; short range and battery-limited, **not an autonomous combat AI** (E-R). **Secondary:** route proving for convoys. **Doctrine fit:** imported technology adapted locally (E). |
| **Inspiration and background** | A small tracked remote vehicle with a detector arm, controlled by an operator in a nearby truck. C&C lineage (E-R): Terror Drone (RA2, small robot archetype); Sentry Drone (ZH); Hunter-Seeker Droid (TS, drone inspiration **without homing strike**). |
| **Visual identity** | none (asset requirement): lawnmower-sized tracked robot, detector boom, cable reel, flashing amber light. |
| **Statistics** | **160 HP** (`armor.veh_light`) · TRACKED 260 cm/s · sight 600 cm · detects mines **600 cm** · battery **120 s** of activity, recharge 60 s within 600 cm of any friendly vehicle · unmanned (food 0) · fuel none |
| **Commands** | Move, Stop (I); Clear Mines (P). **Leash:** must stay within 1500 cm of a friendly vehicle (the operator) or it stops · default stance Hold Fire |
| **Production** | Market **$600** · stock **2** |
| **Veterancy** | none |
| **AI** | `ai.unit.engineer_vehicle`. Deltas: lead convoys by 400 cm on suspected mined routes |
| **Multiplayer** | MPA-STD |
| **Presentation** | Small ANIM-VEH-TRACK. Amber beacon; icon of a robot with a boom |

**Weapons:** none (unarmed)

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.mp.mkt_02.clear_mines` | Active (P) | Reveals mines within 600 cm and removes one revealed mine every 4 s within 150 cm | Revealed mines | battery 4 s per mine | Any weapon kills it; covering fire on the minefield | Detector sweep VFX | Clear before the convoy passes |

**Research and upgrades:** none.

**Counters:** *Beats (hard):* minefields (RD and Concord mines, IA-defense mines). *Beaten by (hard):* any armed unit. *Situational:* the Pact's only dedicated mine clearer.

**Status:** identity, market-only and "not autonomous combat AI" E-R; values P.

### MP-MKT-03 — Surplus Antiship Launcher Trailer

| Field | Specification |
| --- | --- |
| **Identity** | `unit.mp.mkt_03` · Meridian Pact · Market / coastal anti-ship battery · `TPL-VEH-ARTY` (towed, ANTI_SHIP) · **Conditional** (market; suitable shoreline) · Playable |
| **Role** | **Primary:** **market-only** short-range coastal anti-ship battery for suitable map shorelines (E-R). **Secondary:** denying landing beaches. **Doctrine fit:** coastal defense (E). |
| **Inspiration and background** | A trailer with two anti-ship missile rails towed by a tractor unit. C&C lineage (E-R): Dreadnought (RA2, missile function); V3 Rocket Launcher (RA2, trailer silhouette); Naginata Cruiser (RA3, naval targeting role). |
| **Visual identity** | none (asset requirement): olive trailer, two rails, tractor cab with a radar mast. |
| **Statistics** | **480 HP** (`armor.veh_light`) · WHEEL_HEAVY 280 cm/s · sight 1100 cm, `DETECT.AIR_RADAR` none; surface search 2600 cm over water when deployed · crew 3 (food 3) · fuel medium |
| **Commands** | Move, Stop (I); Deploy, Fire Mission (P) · default stance Hold Ground |
| **Production** | Market **$1,500** · stock **1** |
| **Veterancy** | Standard |
| **AI** | `ai.unit.artillery` (naval targets only) |
| **Multiplayer** | MPA-INDIRECT |
| **Presentation** | ANIM-VEH + rail elevation. Icon of a missile on a trailer |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.mp.mkt_03.ssm` | ANTI_SHIP | 800–2800 cm (deployed, shoreline only) | 420 per missile / 10 s; **2 missiles**, reload 30 s from 4 carried, then resupply | guided (jammable) | — | NAVAL | NAV_L 120 % / NAV_H 100 % |

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.mp.mkt_03.deploy` | Active (P) | Fires only when deployed within 600 cm of an authored shoreline; 6 s deploy | Self | free · 6 s | Caught deploying; land raids | Rail raise | Deploy behind a ridge on the coast |

**Research and upgrades:** none.

**Counters:** *Beats (hard):* surface ships (AC-SEA-02/04/06, RD-SEA-05/06/07). *Beaten by (hard):* any land attacker, aircraft, shore bombardment out of its range. *Situational:* turns a Pact coastline into a no-go area.

**Status:** identity and market-only E-R; values P.

### MP-MKT-04 — Amphibious Bridging Kit Carrier

| Field | Specification |
| --- | --- |
| **Identity** | `unit.mp.mkt_04` · Meridian Pact · Market / bridging vehicle · `TPL-VEH-ENGINEER` · **Conditional** (market) · Playable |
| **Role** | **Primary:** **market-only** specialized engineering vehicle for river crossings and pontoon sites (E-R). **Secondary:** none. **Doctrine fit:** engineers keep crossings open (E). |
| **Inspiration and background** | An amphibious truck carrying folded pontoon sections. C&C lineage (E-R): Amphibious APC (TS); Riptide ACV (RA3); Rig (TW, deployment role). |
| **Visual identity** | none (asset requirement): amphibious truck with stacked pontoon boxes. |
| **Statistics** | **600 HP** (`armor.veh_light`) · AMPHIBIOUS 320 cm/s land, 260 cm/s water · sight 1000 cm · crew 2 (food 2) · fuel medium |
| **Commands** | Move, Stop (I); Lay Pontoon, Recover Pontoon (P) · default stance Hold Fire |
| **Production** | Market **$1,100** · stock **1** |
| **Veterancy** | none |
| **AI** | `ai.unit.engineer_vehicle` |
| **Multiplayer** | MPA-STD (the pontoon is world state) |
| **Presentation** | ANIM-VEH + pontoon unfold. Icon of a bridge over waves |

**Weapons:** none (unarmed)

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.mp.mkt_04.lay_pontoon` | Active (P) | Lays a pontoon bridge across an authored river gap of up to 1200 cm in 15 s; capacity F3; usable by all sides; 800 HP structure (`armor.str_soft`) | Authored crossing site | free · 15 s · — | Artillery and charges destroy it; the enemy can use it too | Bridge unfold | Lay just before the crossing |
| `ability.mp.mkt_04.recover_pontoon` | Active (P) | Recovers the bridge in 10 s | Own pontoon | free | — | Fold | Recover after crossing |

**Research and upgrades:** none.

**Counters:** *Beaten by (hard):* artillery, aircraft, saboteurs. *Situational:* decisive on river maps when bridges are blown.

**Status:** identity and market-only E-R; values P.

### MP-MKT-05 — Foreign Surveillance Radar Package

| Field | Specification |
| --- | --- |
| **Identity** | `unit.mp.mkt_05` · Meridian Pact · Market / sensor vehicle · `TPL-VEH-EW` · **Conditional** (market) · Playable |
| **Role** | **Primary:** **market-only** support vehicle with electronic detection that needs skilled operators; **no classified spy intelligence unlock** (E-R). **Secondary:** early warning for the radar-less MP-ARM-09. **Doctrine fit:** communications engineering and countermeasures (E). |
| **Inspiration and background** | A truck-mounted foreign air-search radar with an operator cabin. C&C lineage (E-R): Listening Outpost (ZH); Mobile Sensor Array (TS); Radar Van (Gen). |
| **Visual identity** | none (asset requirement): truck with a folding radar mast and generator trailer. |
| **Statistics** | **400 HP** (`armor.veh_light`) · WHEEL_HEAVY 340 cm/s · sight 1400 cm · when deployed: `DETECT.AIR_RADAR` **3000 cm**, `DETECT.EMITTER` 2400 cm · crew 2 (food 2) · fuel medium |
| **Commands** | Move, Stop, Guard (I); Deploy (P) · default stance Hold Fire |
| **Production** | Market **$1,000** · stock **2** · **skilled operators:** requires `MP-BLD-14` operational (contract technicians) |
| **Veterancy** | none |
| **AI** | `ai.unit.electronic_warfare` |
| **Multiplayer** | MPA-STD + **MPA-SENSOR** (contacts only; no identity) |
| **Presentation** | ANIM-VEH + mast deploy, radar spin. Icon of a radar dish |

**Weapons:** none (unarmed)

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.mp.mkt_05.deploy_mast` | Active (P) | 8 s deploy; while deployed it provides radar and emitter contacts to all own units (lets MP-ARM-09 engage contacts at full air range) and is itself a `DETECT.EMITTER` contact to enemies within 3000 cm | Self | free · 8 s | Jamming suppresses contacts (§9.5); emitter-hunting strikes | Mast animation | Deploy behind the AA screen |

**Research and upgrades:** `tech.shared.radar_signal_processing`: air radar 3000 → 3400 cm.

**Counters:** *Beats (situational):* surprise air raids. *Beaten by (hard):* jammers (AC-ARM-10, RD-SUP-06, IA-ARM-14), raiders, strike aircraft. *Situational:* gives the Pact a radar layer it cannot build natively.

**Status:** identity, operator requirement and "no spy intel" E-R; values P.

### MP-MKT-06 — Commercial Decoy Convoy

| Field | Specification |
| --- | --- |
| **Identity** | `unit.mp.mkt_06` · Meridian Pact · Market / physical decoy group · `TPL-VEH-LOGISTICS` · **Conditional** (market) · Playable |
| **Role** | **Primary:** **market-only** unarmed dummy truck group that attracts misdirected sightings **without revealing truth** (E-R). These are **physical** decoys, not false reports; only the Unaligned plants false intelligence (E). **Doctrine fit:** deception through commerce. |
| **Inspiration and background** | Three empty lorries dressed as a fuel or armor convoy. C&C lineage (E-R): Decoy Army (TW, power lineage); Fake Supply Stash (ZH, decoy lineage); Sudden Transport (RA3, disguise function). |
| **Visual identity** | none (asset requirement): three lorries with tarps over wooden frames shaped like tanks or tanker barrels. |
| **Statistics** | 3 vehicles × **200 HP** (`armor.veh_soft`) · WHEEL_HEAVY 400 cm/s · sight 800 cm · crew 1 each (food 3) · fuel light |
| **Commands** | Move, Stop (I); Set Disguise (P) · default stance Hold Fire |
| **Production** | Market **$450** · stock **3** |
| **Veterancy** | none |
| **AI** | `ai.unit.logistics`. Deltas: drive the route the real convoy is not using |
| **Multiplayer** | **MPA-COVERT** (true identity owner-only; opponents receive the impersonated archetype per framework §9.1 *Decoy*) |
| **Presentation** | Uses the impersonated archetype's appearance profile to opponents; icon (owner) of a mask over a truck |

**Weapons:** none (unarmed)

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.mp.mkt_06.set_disguise` | Active (P) | Each truck impersonates one Pact archetype: MP-SUP-03/04 (tanker) or MP-ARM-06 (tank). Revealed by an observer within 400 cm, any hit, or `DETECT.SPY` (framework §9.1) | Self | free · until revealed · 20 s to change | Close inspection; shoot it once | Owner mask icon | Pair with real convoys on a parallel road |

**Research and upgrades:** none.

**Counters:** *Beaten by (hard):* any hit or close observation; `DETECT.SPY` (AC-INF-11, RD-INF-07, IA-INF-12, UN-INF-11). *Situational:* draws raids and artillery onto empty lorries; wasted against an opponent with good recon.

**Status:** identity, physical decoy and "no false reports" E-R/E; values P.

### MP-MKT-07 — Contracted Light Strike Flight

| Field | Specification |
| --- | --- |
| **Identity** | `unit.mp.mkt_07` · Meridian Pact · Market / single-sortie strike · `TPL-AIR-STRIKE` · **Conditional** (market contract) · Playable |
| **Role** | **Primary:** **hired service** via contractor contract and airport access, **limited single sortie**; **no nuclear payload** (E-R). Differentiated from MP-AIR-06: a **single market sortie** bought from auction stock (U-15). **Doctrine fit:** contracts; no nuclear path (E). |
| **Inspiration and background** | One light attack jet flown by a freelance pilot for one job. C&C lineage (E-R): A-10 Strike (Gen, support power); Orca Strike (TW, support power); Surgical Strike (RA3, support protocol). |
| **Visual identity** | none (asset requirement): single light jet with no national markings. |
| **Statistics** | 1 aircraft × **360 HP** (`armor.air_fixed`) · FIXED_WING 1900 cm/s · sight 1800 cm · aircrew 1 (food 0: contractor) |
| **Commands** | Strike Area (P) only |
| **Production** | Market **$700** per sortie · stock **3** · requires `MP-BLD-06` operational (airport access, E-R) · lead time 20 s |
| **Veterancy** | none |
| **AI** | `ai.unit.air_strike` (one pass, exit) |
| **Multiplayer** | MPA-AIR |
| **Presentation** | ANIM-AIR-FIXED. Enemy warning 3 s before release. Icon of a jet with a ticket |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.mp.mkt_07.bombs` | HIGH_EXPLOSIVE | overflight | 4 × 160, radius 250 cm in a line | bomb | 15 | GROUND, STRUCTURE | 110 % / 45 % / 12 % / — / 100 % |

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.mp.mkt_07.single_sortie` | Active (P) | One bombing pass on an observed line; the item is consumed whether or not the jet survives | Observed line | market price · 20 s lead | AA on the approach; dispersal after the warning | Inbound marker | Use on clustered artillery or bases |

**Research and upgrades:** none.

**Counters:** *Beaten by (hard):* AA on the approach (RD-ARM-06/13, AC-ARM-09, IA-ARM-08). *Situational:* a one-off punch; never nuclear (E).

**Status:** identity, single sortie and "no nuclear payload" E-R/E; differentiation U-15 P; values P.

### MP-MKT-08 — High-End Coastal Interceptor

| Field | Specification |
| --- | --- |
| **Identity** | `unit.mp.mkt_08` · Meridian Pact · Market / hydrofoil · `TPL-NAV-PATROL` · **Conditional** (market; water) · Playable |
| **Role** | **Primary:** **rare market-only** conventional patrol hydrofoil, not an unlimited free shipyard unit (E-R). **Secondary:** hunting raider boats. **Doctrine fit:** coastal forces (E). |
| **Inspiration and background** | A foreign-built missile hydrofoil. C&C lineage (E-R): Hydrofoil (RA3, hull/movement role); Gunboat (RA1); Sea Scorpion (RA2). |
| **Visual identity** | none (asset requirement): sleek white hydrofoil, foils raised at rest. |
| **Statistics** | **520 HP** (`armor.nav_light`) · SURFACE_SHALLOW **800 cm/s** foil-borne · sight 1800 cm · crew 6 (food 6) · fuel medium · ExperienceValue 110 |
| **Commands** | as TPL-NAV-PATROL · default stance Aggressive |
| **Production** | Market **$1,100** · stock **1** |
| **Veterancy** | Standard |
| **AI** | `ai.unit.naval_patrol` |
| **Multiplayer** | MPA-STD |
| **Presentation** | ANIM-NAV + foil transition. Icon of a hydrofoil |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.mp.mkt_08.gun` | AUTOCANNON | 0–1000 cm | 28 / 0.6 s | projectile, fast | 4 | NAVAL, AIR_LOW | NAV_L 90 % / NAV_H 20 % / 100 % rotary |
| `weapon.mp.mkt_08.ssm` | ANTI_SHIP | 0–2200 cm | 2 × 300 per load | guided (jammable) | — | NAVAL | NAV_L 120 % / NAV_H 100 % |

**Abilities:** none unit-specific; speed is its role.

**Research and upgrades:** none.

**Counters:** *Beats (hard):* patrol boats, raiders and landing craft. *Beaten by (hard):* submarines, aircraft, frigates. *Situational:* one rare ship that out-runs everything afloat.

**Status:** identity and rarity E-R; values P.

### MP-MKT-09 — Artillery Observation Balloon

| Field | Specification |
| --- | --- |
| **Identity** | `unit.mp.mkt_09` · Meridian Pact · Market / tethered observation balloon · `TPL-VEH-EW` (stationary air object) · **Conditional** (market) · Playable |
| **Role** | **Primary:** **market-only** tethered low-tech forward observation; **stationary, fragile and weather-constrained** (E-R). **Secondary:** spotting for MP-ARM-08/12. **Doctrine fit:** low-cost adaptation for observation (E). |
| **Inspiration and background** | A small tethered balloon with an observer basket and a field telephone line, winched from a truck. C&C lineage (E-R): Spy Plane (RA1, spotting role); Recon Drone (TT, spotting power); Mobile Sensor Array (TS, information role). |
| **Visual identity** | none (asset requirement): grey balloon on a cable, winch truck below. |
| **Statistics** | Balloon **150 HP** (`armor.air_rotary`, low-altitude air object) · winch truck 300 HP (`armor.veh_soft`) WHEEL_HEAVY 340 cm/s · sight **2600 cm** when raised · crew 2 (food 2) |
| **Commands** | Move (truck), Raise, Lower (P) · default stance Hold Fire |
| **Production** | Market **$500** · stock **2** |
| **Veterancy** | none |
| **AI** | `ai.unit.electronic_warfare` (observation) |
| **Multiplayer** | MPA-STD |
| **Presentation** | Balloon sway animation; visible from far away (tell). Icon of a balloon |

**Weapons:** none (unarmed)

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.mp.mkt_09.raise_balloon` | Active (P) | 10 s raise; while raised the truck cannot move, sight 2600 cm and friendly artillery uses observed scatter ×0.7 on targets it sees. **Weather:** cannot be raised while a scenario "high wind" or storm state is active (U-MP-04) | Self | free · 10 s raise · 6 s lower | Any AA or MG fire (150 HP); weather | Balloon visible to all | Raise behind the front line |

**Research and upgrades:** none.

**Counters:** *Beaten by (hard):* any AA, helicopters, autocannon. *Situational:* cheap long-range eyes for Pact artillery, seen by everyone.

**Status:** identity and constraints E-R; values P; weather dependency **U-MP-04**.

## 9. Faction roster summary

USD and build seconds are PB. Contract units show hire + wage; market units show the opening auction price. Speeds in cm/s.

| ID | Name | Cat. | Template | Tier | Primary role | HP | Armor | Speed | USD | Build s | Producer | Hard counters (to it) | It hard-counters |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| MP-INF-01 | Regional Rifle Company | INF | INF-LINE | Core | Line rifles | 275 | inf_light | 205 | 280 | 8 | BLD-03 | HMG, flame, artillery | Agents, scouts |
| MP-INF-02 | Contracted Guard Detail | INF | INF-LINE | Cond. | Hired post guard | 300 | inf_protected | 200 | 250 + 15/int | 6 | BLD-16 | HMG, armor | Raiders at posts |
| MP-INF-03 | Light Rocket Team | INF | INF-AT | Core | AT ambush | 180 | inf_light | 210 | 460 | 9 | BLD-03 | Infantry, MGs | Vehicles |
| MP-INF-04 | Hillside Mortar Team | INF | INF-HW | Ext. | Mortar | 165 | inf_light | 185 | 520 | 11 | BLD-03 | Raids, counter-battery | Infantry in cover |
| MP-INF-05 | Bridge and Road Engineer | INF | INF-SUPPORT | Core | Crossings, capture | 120 | inf_light | 205 | 380 | 11 | BLD-03 | Any armed unit | — |
| MP-INF-06 | Portable Field Medic | INF | INF-SUPPORT | Core | Heal, replenish | 110 | inf_light | 220 | 360 | 10 | BLD-03 | Marksmen, raiders | — |
| MP-INF-07 | Caravan Reconnaissance Scouts | INF | INF-SPEC | Ext. | Recon, marksman | 120 | inf_light | 215 | 640 | 12 | BLD-03 | Recon, vehicles | Officers, HW teams |
| MP-INF-08 | Early Sabotage Specialist | INF | INF-SPEC | Core | Non-spy charges | 110 | inf_light | 220 | 420 (+120/charge) | 10 | BLD-03 | Detectors, security | Parked vehicles, soft buildings |
| MP-INF-09 | Raiding Engineer | INF | INF-SUPPORT | Ext. | Charges, repair, capture | 130 | inf_light | 205 | 440 | 11 | BLD-03 | Combat infantry | Depots, logistics |
| MP-INF-10 | Mountain Ambush Squad | INF | INF-SPEC | Core | Terrain ambush | 260 | inf_light | 215 | 540 | 12 | BLD-03 | Recon, flame, armor | Road columns, officers |
| MP-INF-11 | Contracted Veteran Marksman | INF | INF-SPEC | Cond. | Hired marksman | 140 | inf_light | 205 | 650 + 30/int | 8 | BLD-16 | Recon, vehicles | Commanders, AT teams |
| MP-INF-12 | Local Militia Levy | INF | INF-LINE | Ext. | Emergency line | 240 | inf_light | 195 | 270 | 5 | BLD-03/02 | HMG, Psy-Ops, Intimidation | Scouts near home |
| MP-INF-13 | Foreign Military Adviser | INF | INF-COMMAND | Ext. | Commander candidate | 460 | inf_protected | 215 | 2,000 | 30 | BLD-03 | Marksmen, jammers | — (multiplier) |
| MP-ARM-01 | Roadrunner Scout Truck | ARM | VEH-SCOUT | Core | Opening scout | 180 | veh_soft | 620 | 150 | 5 | BLD-04 | Patrol cars, AT, mines | Couriers, parked scouts |
| MP-ARM-02 | Armed Commercial Pickup | ARM | VEH-LIGHT | Core | Modular technical | 300 | veh_soft | 500 | 350 | 8 | BLD-04 | Tanks, AT infantry | Infantry (HMG) |
| MP-ARM-03 | Light Armored Van | ARM | VEH-TRANSPORT | Core | 4-slot APC | 480 | veh_light | 420 | 620 | 12 | BLD-04 | AT, autocannon | — |
| MP-ARM-04 | Modular Cargo Gun Truck | ARM | VEH-LIGHT | Ext. | AP / AV mount | 400 | veh_light | 420 | 550 | 10 | BLD-04 | MBTs, artillery | Infantry, helicopters |
| MP-ARM-05 | Scavenged Light Tank | ARM | VEH-MBT (light) | Ext. | Refitted light tank | 620 | veh_medium | 360 | 700 | 14 | BLD-05 | MBTs, TDs | Technicals, scouts |
| MP-ARM-06 | Surplus Battle Tank | ARM | VEH-MBT | Core | MBT | 850 | veh_heavy | 310 | 1,000 | 20 | BLD-05 | Elite MBTs, ATGMs | APCs, light armor |
| MP-ARM-07 | Mobile Recoilless-Gun Carrier | ARM | VEH-TD | Core | Hit-and-run AT | 420 | veh_light | 480 | 900 | 15 | BLD-04 | Infantry, autocannon | MBTs, IFVs |
| MP-ARM-08 | Truck-Mounted Rocket Rack | ARM | VEH-ARTY | Core | Cheap barrage | 360 | veh_soft | 400 | 900 | 18 | BLD-04 | Raids, counter-battery | Infantry blocks |
| MP-ARM-09 | Quad Anti-Air Technical | ARM | VEH-AA | Core | AA + anti-infantry | 460 | veh_light | 400 | 800 | 14 | BLD-04 | Tanks, artillery | Helicopters, infantry |
| MP-ARM-10 | Mobile Smoke-Screen Truck | ARM | VEH-EW | Ext. | Smoke lanes | 380 | veh_light | 380 | 650 | 12 | BLD-04 | Any armed unit | ATGM and spotting lines |
| MP-ARM-11 | Ambush Mine Carrier | ARM | VEH-ENGINEER | Ext. | Minelayer | 520 | veh_light | 340 | 700 (+40/mine) | 13 | BLD-04 | Mine clearers | Road columns |
| MP-ARM-12 | Rough-Terrain Artillery Truck | ARM | VEH-ARTY | Ext. | Howitzer-mortar | 450 | veh_light | 360 | 1,100 | 20 | BLD-04 | Counter-battery, raids | Infantry in cover |
| MP-ARM-13 | Captured Heavy Armor | ARM | VEH-HEAVY | Cond. | Captured heavy tank | 1,300* | veh_heavy | 250* | 1,900 mkt / 50 % refit | 30 (refit) | BLD-15 / BLD-05 | Source-faction AT | MBT lines, forts |
| MP-SUP-01 | Forward Trade Constructor | SUP | VEH-CONSTRUCTOR | Core | Constructor | 500 | veh_medium | 400 | 600 | 12 | BLD-01 | Raiders | — |
| MP-SUP-02 | Merchant Delivery Truck | SUP | VEH-LOGISTICS | Ext. | Market delivery, resupply | 420 | veh_soft | 400 | 300 | 8 | BLD-04/17 | Any armed unit | — |
| MP-SUP-03 | Crude Tanker Convoy | SUP | VEH-LOGISTICS | Ext. | Crude haul | 600 | veh_soft | 340 | 500 | 10 | BLD-04 | Raiders | — |
| MP-SUP-04 | Petroleum Product Tanker | SUP | VEH-LOGISTICS | Ext. | Fuel delivery | 480 | veh_soft | 360 | 450 | 9 | BLD-04 | Raiders | — |
| MP-SUP-05 | Commercial Fuel Bowser | SUP | VEH-LOGISTICS | Ext. | Kerosene, forward pad | 420 | veh_soft | 360 | 450 | 9 | BLD-04 | Any armed unit | — |
| MP-SUP-06 | Cargo Brokerage Van | SUP | VEH-LOGISTICS | Ext. | Covert component haul | 320 | veh_soft | 480 | 350 | 8 | BLD-04 | `DETECT.SPY`, police | — |
| MP-SUP-07 | Mercenary Personnel Carrier | SUP | VEH-TRANSPORT | Ext. | Contract troop carrier | 560 | veh_light | 400 | 700 | 13 | BLD-04 | AT, autocannon | — |
| MP-SUP-08 | Repair Caravan | SUP | VEH-LOGISTICS | Core | Repair, salvage, tow | 620 | veh_light | 360 | 750 | 12 | BLD-04 | Raids, artillery | — |
| MP-SUP-09 | Black-Market Delivery Aircraft | SUP | AIR-TRANSPORT | Cond. | Market air delivery | 520 | air_fixed | 1500 | 250 charter | — | BLD-15 | Interceptors, SAMs | — |
| MP-AIR-01 | Light Utility Helicopter | AIR | AIR-HELI-TRANSPORT | Ext. | Courier, resupply drop | 450 | air_rotary | 650 | 900 | 14 | BLD-07 | Any AA | — |
| MP-AIR-02 | Armed Escort Helicopter | AIR | AIR-HELI-ATTACK | Core | Hired convoy escort | 480 | air_rotary | 700 | 1,200 + 30/int | 18 | BLD-16 | Mobile AA | Technicals, APCs |
| MP-AIR-03 | Imported Fighter-Bomber | AIR | AIR-STRIKE | Ext. | Strike, light A2A | 400 | air_fixed | 1800 | 1,700 | 26 | BLD-06 | Interceptors, SAMs | Artillery, logistics |
| MP-AIR-04 | Cargo Transport Plane | AIR | AIR-TRANSPORT | Ext. | Strip-to-strip lift | 600 | air_fixed | 1400 | 1,300 | 20 | BLD-06 | Interceptors | — |
| MP-AIR-05 | Coastal Patrol Aircraft | AIR | AIR-SURVEILLANCE | Ext. | Maritime patrol, verify | 420 | air_fixed | 1200 | 1,100 | 18 | BLD-06 | Interceptors | Submarines (detect) |
| MP-AIR-06 | Hired Air Support Flight | AIR | AIR-STRIKE | Cond. | Recurring hired strike | 2 × 380 | air_fixed | 1900 | 900 + 400/sortie | — | BLD-16 | Mobile AA | Convoys, artillery |
| MP-SEA-01 | Merchant Coastal Barge | SEA | NAV-TRANSPORT | Cond. | Sea cargo | 700 | nav_light | 420 | 700 | 16 | BLD-08 | Torpedoes, subs | — |
| MP-SEA-02 | Fast Maritime Scout | SEA | NAV-PATROL | Cond. | Naval scout | 320 | nav_light | 720 | 500 | 12 | BLD-08 | Missile boats | Launches, landing craft |
| MP-SEA-03 | Contract Security Patrol Boat | SEA | NAV-PATROL | Cond. | Hired escort | 480 | nav_light | 560 | 600 + 20/int | 10 | BLD-16 | Missile boats, subs | Raider boats |
| MP-SEA-04 | Amphibious Cargo Carrier | SEA | NAV-TRANSPORT | Cond. | Amphibious lift | 520 | veh_light / nav_light | 360 / 300 | 800 | 14 | BLD-08/04 | Armed boats | — |
| MP-SEA-05 | Surplus Coastal Submarine | SEA | NAV-SUB | Cond. | Torpedo raider | 900 | nav_heavy | 320 | 1,900 (+20/int) | 40 | BLD-08 | ASW | Barges, transports |
| MP-SEA-06 | Licensed Missile Corvette | SEA | NAV-COMBATANT | Cond. | Anti-ship | 1,500 | nav_heavy | 360 | 2,200 | 40 | BLD-08 | Submarines | Patrol boats |
| MP-SEA-07 | Floating Trade Anchorage | SEA | NAV-TRANSPORT | Cond. | Forward naval hub | 1,800 | nav_heavy | 260 | 2,000 | 45 | BLD-08 | Subs, anti-ship | — |
| MP-MKT-01 | Imported Heavy Recovery Crawler | MKT | VEH-LOGISTICS | Cond. | Heavy recovery | 1,100 | veh_heavy | 300 | 1,400 mkt | — | BLD-15 | ATGMs, tank guns | — |
| MP-MKT-02 | Experimental Mine-Clearing Robot | MKT | VEH-ENGINEER | Cond. | Mine clearing | 160 | veh_light | 260 | 600 mkt | — | BLD-15 | Any armed unit | Minefields |
| MP-MKT-03 | Surplus Antiship Launcher Trailer | MKT | VEH-ARTY | Cond. | Coastal anti-ship | 480 | veh_light | 280 | 1,500 mkt | — | BLD-15 | Land attackers, air | Surface ships |
| MP-MKT-04 | Amphibious Bridging Kit Carrier | MKT | VEH-ENGINEER | Cond. | Pontoon bridge | 600 | veh_light | 320 | 1,100 mkt | — | BLD-15 | Artillery, charges | — |
| MP-MKT-05 | Foreign Surveillance Radar Package | MKT | VEH-EW | Cond. | Air and emitter radar | 400 | veh_light | 340 | 1,000 mkt | — | BLD-15 | Jammers, raids | Surprise air raids |
| MP-MKT-06 | Commercial Decoy Convoy | MKT | VEH-LOGISTICS | Cond. | Physical decoys | 3 × 200 | veh_soft | 400 | 450 mkt | — | BLD-15 | Any hit, `DETECT.SPY` | — (misdirection) |
| MP-MKT-07 | Contracted Light Strike Flight | MKT | AIR-STRIKE | Cond. | Single hired sortie | 360 | air_fixed | 1900 | 700 mkt | — | BLD-15 | Mobile AA | Clusters, bases |
| MP-MKT-08 | High-End Coastal Interceptor | MKT | NAV-PATROL | Cond. | Fast hydrofoil | 520 | nav_light | 800 | 1,100 mkt | — | BLD-15 | Subs, aircraft | Patrol and raider boats |
| MP-MKT-09 | Artillery Observation Balloon | MKT | VEH-EW | Cond. | Tethered observation | 150 + 300 | air_rotary / veh_soft | 340 (truck) | 500 mkt | — | BLD-15 | Any AA, weather | — (spotter) |

\* MP-ARM-13 inherits source-unit values at −10 % max HP; the figures shown are the market default.

## 10. Abilities and upgrades index

Shared abilities (`ability.shared.*`: grenade, sprint, conceal, plant_charge, repair, resupply, salvage_wreck) are defined in the framework and sibling files; the Meridian parameters of `ability.shared.plant_charge` are given in MP-INF-08 and MP-INF-09. This index lists Pact-specific abilities and technologies.

| ID | Unit(s) | Kind | Cost | Cooldown | Range | Requires | Counters |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.mp.contract_terms` | MP-INF-02/11, MP-AIR-02/06, MP-SEA-03 | Passive | wage per interval | — | — | Contract | USD pressure |
| `ability.mp.inf_01.fieldcraft` | MP-INF-01 | Conditional | free | — | self | Rough terrain | Open ground |
| `ability.mp.inf_02.hold_post` | MP-INF-02 | Conditional | free | — | 600 cm | Assigned post | Lure away |
| `ability.mp.inf_03.shoot_and_scoot` | MP-INF-03 | Passive | free | — | self | After firing | Area fire |
| `ability.mp.inf_04.smoke_rounds` | MP-INF-04 | Active | 2 ammo | 30 s | 600–2000 cm | — | Detectors |
| `ability.mp.inf_05.restore_crossing` | MP-INF-05 | Active | $150 | — | 200 cm | Damaged crossing | Fire on engineers |
| `ability.mp.inf_06.heal` | MP-INF-06 | Active / automated | free | — | 300 cm | — | Kill the medic |
| `ability.mp.inf_06.field_replenishment` | MP-INF-06 | Active | USD | 20 s | 300 cm | — | Interrupt |
| `ability.mp.inf_07.trader_cover` | MP-INF-07 | Active | free | 30 s | self | Moving ≤ 60 % | `DETECT.SPY`, police |
| `ability.mp.inf_07.spot` | MP-INF-07 | Active | free | 25 s | sight | — | Suppress |
| `ability.mp.inf_08.cut_and_run` | MP-INF-08 | Active | free | 30 s | self | Charge placed | Fast scouts, area fire |
| `ability.mp.inf_09.field_repair` | MP-INF-09 | Active | 25 % per full repair | — | 300 cm | — | Interrupt |
| `ability.mp.inf_09.capture` | MP-INF-09 | Active | free | — | contact | Eligible structure | Damage resets |
| `ability.mp.inf_10.ambush` | MP-INF-10 | Active | free | 40 s | weapon | Concealed | Recon screens |
| `ability.mp.inf_10.fall_back` | MP-INF-10 | Active | free | 30 s | self | — | Fast vehicles |
| `ability.mp.inf_11.head_shot_discipline` | MP-INF-11 | Passive | — | — | weapon | Command/crew target | INF_PROTECTED |
| `ability.mp.inf_12.home_ground` | MP-INF-12 | Conditional | — | — | 1500 cm of home | Pact HQ/outpost/compound | Lure away |
| `ability.mp.inf_12.population_link` | MP-INF-12 | Passive | — | — | faction | Population Morale | Strike trade |
| `ability.mp.inf_13.training_package` | MP-INF-13 | Active | $300 | 60 s | 1400 cm | — | Kill or jam the adviser |
| `ability.mp.arm_01.road_dash` | MP-ARM-01 | Active | free | 25 s | self | On road | Roadblocks, mines |
| `ability.mp.arm_01.contact_report` | MP-ARM-01 | Passive | — | — | sight | — | Kill before it sees |
| `ability.mp.arm_02.swap_mount` | MP-ARM-02 | Active | $40 | — | self | BLD-04 or SUP-08 | Caught swapping |
| `ability.mp.arm_03.quick_dismount` | MP-ARM-03 | Passive | — | — | — | — | Ambush at unload |
| `ability.mp.arm_04.configure` | MP-ARM-04 | Active | free | 15 s | self | — | Flank during switch |
| `ability.mp.arm_05.condition_grade` | MP-ARM-05 | Passive | refit $150 | — | — | — | — |
| `ability.mp.arm_06.imported_spares` | MP-ARM-06 | Passive | +20 % repair | — | — | — | Kill SUP-08 |
| `ability.mp.arm_07.shoot_and_scoot` | MP-ARM-07 | Active | free | 10 s | self | After a shot | Infantry, HE |
| `ability.mp.arm_08.reload_from_stock` | MP-ARM-08 | Automated | §10.4 | — | — | Resupply | Kill supply trucks |
| `ability.mp.arm_10.smoke_lane` | MP-ARM-10 | Active | 1 smoke charge | 45 s | path | Moving | Detectors |
| `ability.mp.arm_10.smoke_pot` | MP-ARM-10 | Active | free | 25 s | 900 cm | — | Detectors |
| `ability.mp.arm_11.lay_minefield` | MP-ARM-11 | Active | $40 per mine | 2 s | path | MP-DEF-05 | Mine clearers, detectors |
| `ability.mp.arm_11.recover_mines` | MP-ARM-11 | Active | free | — | 300 cm | Own mines | — |
| `ability.mp.arm_12.deploy` | MP-ARM-12 | Active | free | — | self | — | Caught deploying |
| `ability.mp.arm_12.smoke_round` | MP-ARM-12 | Active | 1 round | 30 s | 800–2800 cm | Deployed | Detectors |
| `ability.mp.arm_13.foreign_parts` | MP-ARM-13 | Passive | — | — | — | — | Deny MP-BLD-05 |
| `ability.mp.sup_01.build` | MP-SUP-01 | Active | structure price | — | 600 cm | Prerequisites, power | Kill constructor |
| `ability.mp.sup_01.deploy_outpost` | MP-SUP-01 | Active | outpost −25 % | — | self | Valid site | Raids |
| `ability.mp.sup_02.deliver_market_item` | MP-SUP-02 | Automated | $50 fee | — | route | Market order | Interdiction |
| `ability.mp.sup_03.collect_crude` | MP-SUP-03 | Automated | — | — | field | Legal or contested access | Deny access |
| `ability.mp.sup_04.refuel_outpost` | MP-SUP-04 | Active | cargo | — | 400 cm | Fuel cargo | Kill en route |
| `ability.mp.sup_05.forward_arming_point` | MP-SUP-05 | Active | kerosene | — | 400 cm | Near MP-BLD-02 | Strike the outpost |
| `ability.mp.sup_06.merchant_paperwork` | MP-SUP-06 | Passive | — | — | — | — | `DETECT.SPY`, police |
| `ability.mp.sup_06.component_delivery` | MP-SUP-06 | Automated | — | — | route | Market or refit order | Interdiction |
| `ability.mp.sup_07.contract_clock_pause` | MP-SUP-07 | Passive | — | — | — | Loaded contract units | Kill it loaded |
| `ability.mp.sup_08.tow_wreck` | MP-SUP-08 | Active | free | — | 90 s limit | Eligible wreck | Block the tow |
| `ability.mp.sup_09.air_delivery` | MP-SUP-09 | Automated | $250 charter | — | corridor | MP-BLD-06 | AA, interceptors |
| `ability.mp.air_01.resupply_drop` | MP-AIR-01 | Active | $80 | 60 s | 600 cm | — | AA, MGs |
| `ability.mp.air_02.escort` | MP-AIR-02 | Active | free | — | 1500 cm | — | Bait, AA |
| `ability.mp.air_04.strip_to_strip` | MP-AIR-04 | Active | kerosene 4 | — | map | Second airstrip | AA, strip destruction |
| `ability.mp.air_05.verify_report` | MP-AIR-05 | Active | free | 30 s | 3200 cm | Received report | Interceptors, jamming |
| `ability.mp.air_06.call_sortie` | MP-AIR-06 | Active | $400 | 45 s | observed line | Contract | AA, dispersal |
| `ability.mp.sea_01.dock_transfer` | MP-SEA-01 | Active | free | — | dock | Dock | Blockade |
| `ability.mp.sea_03.convoy_escort` | MP-SEA-03 | Active | free | — | 600 cm | — | Torpedoes, aircraft |
| `ability.mp.sea_05.dive` | MP-SEA-05 | Active | free | — | self | — | ASW |
| `ability.mp.sea_07.anchor` | MP-SEA-07 | Active | free | 60 s | 800 cm | In water | Subs, missiles |
| `ability.mp.mkt_01.heavy_tow` | MP-MKT-01 | Active | free | — | 120 s limit | Eligible wreck | Heavy AT |
| `ability.mp.mkt_02.clear_mines` | MP-MKT-02 | Active | battery | — | 150 / 600 cm | Leash 1500 cm | Any armed unit |
| `ability.mp.mkt_03.deploy` | MP-MKT-03 | Active | free | — | self | Shoreline | Land raids |
| `ability.mp.mkt_04.lay_pontoon` | MP-MKT-04 | Active | free | — | 1200 cm gap | Authored site | Artillery, charges |
| `ability.mp.mkt_04.recover_pontoon` | MP-MKT-04 | Active | free | — | — | Own pontoon | — |
| `ability.mp.mkt_05.deploy_mast` | MP-MKT-05 | Active | free | — | 3000 cm | MP-BLD-14 | Jammers |
| `ability.mp.mkt_06.set_disguise` | MP-MKT-06 | Active | free | 20 s | self | — | Hits, `DETECT.SPY` |
| `ability.mp.mkt_07.single_sortie` | MP-MKT-07 | Active | market price | — | observed line | MP-BLD-06 | AA |
| `ability.mp.mkt_09.raise_balloon` | MP-MKT-09 | Active | free | — | 2600 cm sight | No high wind | AA, weather |
| `tech.mp.modular_commercial_equipment` | MP-ARM-01/02/03/04/07/09, MP-SUP-01, MP-AIR-03, MP-SEA-02/04 | Faction tech | per research doc | — | — | — | — |
| `tech.mp.contract_arbitration` | MP-INF-02/11, MP-AIR-02, MP-SUP-07, MP-SEA-03 | Upgrade | $500 | — | — | MP-DEF-09 | Wage +$5 trade-off |
| `tech.mp.regional_drill` | MP-INF-01 | Upgrade | $500 | — | — | MP-BLD-03 | — |
| `tech.mp.tandem_rockets` | MP-INF-03 | Upgrade | $600 | — | — | MP-BLD-03 | Magazine 4 → 3 |
| `tech.mp.mountain_observers` | MP-INF-04 | Upgrade | $400 | — | — | MP-BLD-03 | — |
| `tech.mp.field_engineering` | MP-INF-05 | Upgrade | $500 | — | — | MP-BLD-03 | Restore cost +$50 |
| `tech.mp.field_demolition_kits` | MP-INF-08/09, MP-ARM-11 | Upgrade | $600 | — | — | MP-DEF-07 | −10 % speed when loaded; carrier death blast |
| `tech.mp.mountain_warfare_training` | MP-INF-10 | Upgrade | $700 | — | — | MP-BLD-03 | +$40 unit price |
| `tech.mp.gun_shields` | MP-ARM-04 | Upgrade | $400 | — | — | MP-BLD-04 | −20 cm/s |
| `tech.mp.workshop_standards` | MP-ARM-05/12/13, MP-SUP-08 | Upgrade | $800 | — | — | MP-BLD-05 | +$50 on MP-ARM-05 |
| `tech.mp.add_on_armor` | MP-ARM-06 | Upgrade | $900 | — | — | MP-BLD-05 | −15 cm/s, +1 petroleum |
| `tech.mp.rocket_rack_reloads` | MP-ARM-08 | Upgrade | $500 | — | — | MP-BLD-04 | Larger death blast |
| `tech.mp.chemical_smoke_mix` | MP-ARM-10 | Upgrade | $400 | — | — | MP-BLD-05 | +25 % INCENDIARY damage taken |

## 11. Faction tactical coverage, gaps and balance risks

**Coverage per role.**

| Role | Coverage | Units |
| --- | --- | --- |
| Line infantry | Adequate, not cheap (E-R) | MP-INF-01/12, MP-INF-02 (hired) |
| Anti-armor | Good through mobility rather than armor | MP-INF-03, MP-ARM-07, MP-ARM-04 (AV), MP-ARM-06, MP-ARM-13 |
| Anti-infantry | Good | MP-INF-10, MP-ARM-02 (HMG), MP-ARM-09, MP-ARM-08/12 |
| Anti-air | Adequate at low altitude; no native radar or SAM vehicle | MP-ARM-09, MP-ARM-04 (AP), MP-AIR-03, MP-SEA-06, MP-MKT-05 (radar) |
| Artillery | Adequate, supply-dependent | MP-INF-04, MP-ARM-08/12 |
| Recon and detection | **Best early in the game** (E) | MP-ARM-01, MP-INF-07, MP-SEA-02, MP-AIR-05, MP-MKT-09 |
| Sabotage | **Best early conventional sabotage** (E) | MP-INF-08/09 |
| Logistics, recovery and capture | Excellent but soft-skinned | MP-SUP-01 to 09, MP-MKT-01 |
| Engineering | Good | MP-INF-05/09, MP-ARM-11, MP-MKT-02/04 |
| Contracts and market | Distinctive | MP-INF-02/11, MP-AIR-02/06, MP-SEA-03, MP-MKT-01 to 09 |
| Naval | Conditional, trade-focused | MP-SEA-01 to 07, MP-MKT-03/08 |
| Propaganda and nuclear | **None** (E) | — |

**Weak spots and intended answers.**

1. *Heavy armor.* The Pact cannot mass MBTs and has no native heavy tank (E: weakest in heavy industrial concentration). Intended answers: MP-ARM-07 hit-and-run, MP-INF-03 ambushes, MP-ARM-11 mines, MP-INF-08 charges on sieged or parked tanks, and MP-ARM-13 capture.
2. *High-altitude air.* No radar SAM vehicle. Answers: MP-MKT-05 radar plus MP-ARM-09, MP-AIR-03 missiles, MP-DEF-02 emplacements; this is a deliberate gap that rewards Concord and Directorate bombers.
3. *Soft logistics.* Most Pact vehicles are VEH_SOFT or VEH_LIGHT. Answers: escorts (MP-ARM-02/09, MP-AIR-02, MP-SEA-03), MP-ARM-10 smoke, dispersal through MP-SUP-01 outposts.
4. *Isolation penalty.* Small Pact raids suffer the normal Discipline penalty (E) and are prime Intimidation and Psy-Ops targets. Answers: raid in groups of 3+, MP-INF-13 *Raiding Doctrine*, quick withdrawals.
5. *Late-game sabotage parity.* Rivals obtain spy charges later at $300 (E). The Pact's lead must come from timing, price and the number of carriers, not from charge damage alone.

**Balance risks.**

- **Early charge rush.** Three MP-INF-08 teams at minute 2 can threaten a single refinery before any detector exists. Mitigations: 3 s placement channel, 10 s fuse with *Investigate* counterplay, no XP from charge kills, `DETECT.CONCEALED` on every faction's early recon (RD-INF-09, AC-INF-08, IA-INF-08) and Directorate rail security (RD-INF-07). Telemetry gate: if more than 25 % of matches see a producer destroyed by charges before minute 5, raise the fuse to 15 s before raising the price.
- **Scout-truck dominance.** MP-ARM-01 at $150 and 620 cm/s could snowball map control. Mitigations: 180 HP, weak MG, *Road Dash* cannot fire, mines and patrol cars counter it. Its *Recoilless Kit* must stay an opportunity cost (loses speed and *Road Dash*).
- **Contract stacking.** Hired units plus MP-INF-13 *Contract Coordination* and *Rapid Contract Mobilization* could create bursts of rented power. Mitigations: wages scale with time, MP-INF-11 limit 2, the commander invariant (talent doc §3.3) and USD drain.
- **Captured-equipment snowball.** MP-ARM-13 plus MP-MKT-01 could turn every won fight into free heavy tanks. Mitigations: 50 % refit price, 40 % HP spawn, cap 2, prototype exclusion, 80 % field-repair ceiling, and competition from rival recovery vehicles.
- **Decoys and paperwork.** MP-MKT-06 and MP-SUP-06 must never leak true identity through replicated class, animation or sound (MPA-COVERT exploit surface, framework §15).

**Open items.** Framework U-03 (terrain classes for MP-INF-04/10 and MP-ARM-12), U-04 (sortie model), U-05/06 (food and fuel penalties), U-07 (death blasts for MP-ARM-08, tankers and MP-ARM-11), U-09 (resupply pricing), U-10 (refunds), U-14 (**MP-INF-13 commander approval**) and U-15 (MP-AIR-06 vs MP-MKT-07, resolved here as recurring contract vs single market sortie). Faction-specific items:

| ID | Decision | Recommendation |
| --- | --- | --- |
| U-MP-01 | MP-ARM-13 capture eligibility | MBT and heavy wrecks of the three rivals; prototypes (AC-ARM-15, RD-ARM-14, IA-ARM-06) and Unaligned armor excluded; refit 50 % of source price |
| U-MP-02 | Meridian charge parameters | $120, 3 s channel, 10 s fuse, 30 s cooldown, 3 carried (MP-INF-08) and 2 carried (MP-INF-09); confirm the ratio against the rivals' $300 (pairs with U-AC-05) |
| U-MP-03 | MP-ARM-05 condition grades | Deterministic A/B/C cycle from the server production sequence; no client-side randomness |
| U-MP-04 | Weather dependency of MP-MKT-09 | Requires a scenario weather state; if no weather system ships, drop the constraint and keep fragility only |
| U-MP-05 | Black Market stock per match and restock rules | Stock values in §8 as opening stock; restock owned by the economy documents |
| U-MP-06 | MP-SUP-06 on Unaligned roads | A revealed falsified manifest counts as an R1 incident for the Unaligned response ladder (framework §14.3) |
| U-MP-07 | MP-SEA-07 anchorage parameters and production list | 20 s anchor, 60 s re-anchor, produces MP-SEA-02/04 only |
| U-MP-08 | MP-INF-13 fallback if commander approval is refused | Keep it as a non-commander adviser with *Training Package* and *Field Improvisation*, ordinary veterancy, cap 1 |
