# POLITIKA — Red Directorate Unit Specifications

**Status:** PROPOSED unit gameplay specification (29 October 2026). No unit in this file is implemented. Numbers are Provisional Baseline (PB) values.
**Authority:** names and IDs come from the [Five-Faction Rosters](../POLITIKA_FIVE_FACTION_ROSTERS.md); shared rules from the [Unit Specification Framework](../UNIT_SPECIFICATION_FRAMEWORK.md); faction canon from the [faction description](../../Factions/TheRedDirectorate/description.md). Owner documents win on conflict.
**Evidence labels:** I / E / E-R / P / U as in framework §1.1. A field without a label is P. Cross-faction views: [Unit Balance and Ability Matrices](../UNIT_BALANCE_AND_ABILITY_MATRICES.md). Coverage: [Unit Specification Coverage Report](../Reports/UNIT_SPECIFICATION_COVERAGE_20261029.md).
**Siblings:** [Atlantic Concord](ATLANTIC_CONCORD_UNITS.md) · [Iron Ascendancy](IRON_ASCENDANCY_UNITS.md) · [Meridian Pact](MERIDIAN_PACT_UNITS.md) · [The Unaligned](UNALIGNED_UNITS.md).

## 1. Faction doctrine and faction-wide unit rules

The Directorate fights with enormous conventional forces: heavily armored divisions, mechanized infantry, extensive artillery, mobile missiles, integrated air defense and tactical aviation, coordinated for operational depth and rapid exploitation of breakthroughs (E: faction description, *Military Doctrine*). Its power is industrial and logistical. Rail yards, fuel depots, repair organizations and engineers turn factory output into units in the field, and a numerically superior army loses momentum when bridges, rail lines or depots are cut (E: *Industrial Logistics*). Individual units are cheaper and plainer than Concord equivalents; the faction wins by mass, cohesion and sustained fire.

| Rule | Unit-level implementation | Label |
| --- | --- | --- |
| Infantry far cheaper | Every Directorate infantry price is **−45 % against the §4.3 band midpoint** for its role. **Validator:** each Directorate infantry role price × 1.5 must not exceed the cheapest comparable role in any other playable faction (checked in §10). Vehicles, aircraft and ships get no discount (E: not every category) | E (rule), P (values) |
| Food remains enforceable | Infantry still consume 1 food per member per interval. Cheap mass therefore costs food, which is the Directorate's soft cap (E: research doc §5.2) | E |
| Very-massive-army Discipline | 12+ coherent friendly combat units within 1500 cm in one connected support component: stress taken −20 %. Ordinary group support also applies. Individual soldiers and very small groups suffer the **normal isolation penalty** (E) | E (rule), P (values) |
| Opening scout | `RD-ARM-01` Patrol Motorcycle: **rifle-armed**, little armor, no espionage. Validator: has a rifle weapon (E) | E |
| Sabotage access | Attached charges only through `RD-INF-11` after `tech.rd.late_espionage`, at a higher price than Meridian non-spy charges (E) | E (I predicate) |
| Espionage specialty | Slightly cheaper and faster **intelligence theft** (`RD-INF-11`: −15 % time and cost against other factions' theft) and budget-to-budget **resource diversion** (`RD-INF-12`) | E (rule), P (values) |
| Propaganda | Radio, newspaper and television (RD-BLD-16, RD-DEF-08/09/10); Population Morale sets resolve baseline maximum and production output | E |
| Nuclear | Established nuclear power; delivery by **ICBM** from `RD-DEF-14`, a structure. No unit carries a nuclear weapon. `RD-AIR-09` is conventional (U-13) | E (delivery), P (U-13 resolution) |
| Fuel | No refining discount (E-R: RD-BLD-09). Heavy armor is "fuel hungry" (E-R: RD-ARM-05): higher petroleum cost per production and a harsher starvation penalty | E-R, P |
| Visual language | Olive-drab Soviet-style kit, steel helmets, Kalashnikov-pattern rifles; red bands for officers; vehicles in dark green with a red star-in-gear stencil; industrial and rail backdrops; formal, commanding voice set | P (from concept art) |

**C&C design lessons applied.** The roster uses the Soviet and GLA pattern of *cheap mass plus heavy industry*: a conscript squad that is weak alone and strong in numbers (RA2 Conscript, Generals Red Guard), an affordable main tank that wins by count (RA2 Rhino), a mobile flak vehicle that shreds infantry as well as aircraft (Generals Gattling Tank), wide-area unguided rocket artillery (RA1 V2, RA3 V4) and a dug-in defensive tank (TS Tick Tank). Lessons avoided: RA1/RA2 mass-tank spam with no logistic cost (fuel and repair dependence and the RD-ARM-05 fuel penalty answer it), and the unanswerable Apocalypse-style super tank (RD-ARM-14 is a cap-1 command vehicle, slow and air-vulnerable).

## 2. Core roster selection

Core (16 units):

| ID | Role covered | Reason |
| --- | --- | --- |
| RD-INF-01 | Line infantry | Defining cheap mass rifle platoon |
| RD-INF-02 | Line infantry with AT teamwork | Mechanized infantry that pairs with carriers |
| RD-INF-03 | Suppression / heavy weapons | The Directorate's anti-infantry anchor |
| RD-INF-04 | Anti-armor infantry (+ limited AA) | Cheap AT mass |
| RD-INF-06 | Engineer / capture | Pioneers for bridges and capture |
| RD-INF-09 | Recon infantry | Detection and ambush suppression |
| RD-INF-10 | Faction signature: officer | Discipline support that makes mass doctrine work |
| RD-ARM-01 | Opening scout (E) | Rifle-armed motorcycle |
| RD-ARM-02 | Light vehicle / cheap transport | Motorized delivery |
| RD-ARM-03 | Heavy transport | Tracked carrier for mass formations |
| RD-ARM-04 | Main battle tank | Affordable mass tank |
| RD-ARM-06 | Mobile air defense | Flak that also suppresses infantry |
| RD-ARM-07 | Artillery (signature) | Wide-area rocket barrage |
| RD-SUP-01 | Constructor | Industrial construction |
| RD-SUP-04 | Repair | Keeps mass armor running |
| RD-AIR-03 | Air support | Gunship for tank spearheads |

**Extended:** RD-INF-05, RD-INF-07, RD-INF-08, RD-INF-13 (commander), RD-ARM-05, RD-ARM-08 to RD-ARM-13, RD-ARM-15, RD-SUP-02, RD-SUP-03, RD-SUP-05, RD-SUP-06, RD-SUP-07, RD-AIR-01, RD-AIR-02, RD-AIR-04 to RD-AIR-08, RD-SEA-02 (rivers exist on land maps).
**Conditional:** RD-INF-11 and RD-INF-12 (espionage track; charges need `tech.rd.late_espionage`), RD-ARM-14 (prototype cap 1), RD-AIR-09 (strategic conventional missile, cap 2, U-13), RD-SEA-01 and RD-SEA-03 to RD-SEA-07 (authored water layer and RD-BLD-08).
**Scenario:** none.

## 3. Infantry, personnel and intelligence

### RD-INF-01 — Directorate Rifle Platoon

| Field | Specification |
| --- | --- |
| **Identity** | `unit.rd.inf_01` · Red Directorate · Infantry / mass line platoon · `TPL-INF-LINE` · **Core** · Playable |
| **Role** | **Primary:** extremely cheap mass-produced line infantry; doctrine rewards formations (E-R). **Secondary:** garrison bunkers (RD-DEF-01); screen armor; absorb fire. **Doctrine fit:** cheap infantry and the very-massive-army bonus (E). |
| **Inspiration and background** | Conscript motor-rifle infantry with Kalashnikov-pattern rifles and a squad machine gun. C&C lineage (E-R): Conscript (RA2); Red Guard (Gen); Militant Squad (TW). |
| **Visual identity** | Concept: [`RD-INF-01.png`](../../Factions/TheRedDirectorate/Units/RD-INF-01.png). Five members, steel helmets, greatcoats; red armband on the sergeant. |
| **Statistics** | 5 × 50 HP = **250** (`armor.inf_light`) · FOOT 200 cm/s · F0 · sight 1100 cm · 1 slot · food staff 5 · fuel none · ExperienceValue 18 |
| **Commands** | as AC-INF-01 (I + P set) · default stance Aggressive · formation role `frontline` |
| **Production** | **$175** (band mid $325 × 0.55) · `RD-BLD-03` Conscript Muster Barracks · prerequisites: `RD-BLD-01` · **6 s** (below band: fast muster, E-R) · 5 food · replenishment per §10.3 · no cap |
| **Veterancy** | Standard; low ExperienceValue makes ranks quick but individually small |
| **AI** | `ai.unit.line_infantry`. Deltas: advance only in groups of ≥ 3 squads; never retreat below 25 % if the very-massive bonus is active |
| **Multiplayer** | MPA-STD |
| **Presentation** | ANIM-INF + reload, throw, prone. 7.62-class tracers. Icon: rifle with a red star |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.rd.inf_01.primary` assault rifles + LMG | SMALL_ARMS | 0–640 cm | 8 per member / 1.0 s (40 DPS at 5); magazine 30, reload 3 s (I) | hitscan | 3 | GROUND, AIR_LOW | 100 % / 15 % / 2 % / 25 % rotary / 8 % |
| `ability.shared.grenade` (I) | HIGH_EXPLOSIVE | 650 cm | 50, 150 cm | lobbed | 8 | GROUND, STRUCTURE | 110 % / 45 % / 12 % / — / 100 % |

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.shared.grenade` / `.sprint` (I) | Active | Implemented grenade and sprint | as framework | as framework | as framework | — | Grenade garrisons; sprint to cover |
| `ability.rd.inf_01.mass_assault` | Active (P) | 8 s: +15 % speed and no Shaken cooldown penalty, but stress taken +15 % | Self; only while ≥ 4 friendly infantry squads are within 800 cm | free · 8 s · 40 s · self | HMG and artillery during the charge; flame | Battle-cry voice; status icon | Use when closing from > 600 cm onto infantry in cover |

**Research and upgrades:** `tech.rd.industrial_mobilization_methods` (E candidate, RD-BLD-02): RD-BLD-03 infantry build time −20 %; trade-off: the planning office lane for 60 s and $700. `tech.shared.infantry_protective_equipment`: member HP 50 → 60 at −5 % speed.

**Counters:** *Beats (hard):* agents, scouts, unescorted support infantry. *Beats (soft):* any infantry on cost efficiency (three platoons cost less than two Concord sections). *Beaten by (hard):* grenadiers and flame (AC-INF-04, IA-INF-05), HMG and artillery (AC-ARM-07, IA-INF-03), marksmen (AC-INF-08). *Situational:* weak alone (normal isolation penalty, E); decisive in mass.

**Status:** identity E-R; price rule and mass Discipline E; generic infantry behavior I; everything else P.

### RD-INF-02 — Motor-Rifle Section

| Field | Specification |
| --- | --- |
| **Identity** | `unit.rd.inf_02` · Red Directorate · Infantry / mechanized rifle section · `TPL-INF-LINE` · **Core** · Playable |
| **Role** | **Primary:** infantry with organic transport discipline and anti-armor teamwork (E-R). **Secondary:** dismount assault from RD-ARM-02/03. **Doctrine fit:** mechanized infantry integrated with armor (E). |
| **Inspiration and background** | Motor-rifle section with a rocket-propelled grenade gunner. C&C lineage (E-R): Rifle Infantry (RA1); Rifleman Squad (TW); Red Guard (Gen). |
| **Visual identity** | Concept: [`RD-INF-02.png`](../../Factions/TheRedDirectorate/Units/RD-INF-02.png). Four members, one with an RPG tube. |
| **Statistics** | 4 × 60 HP = **240** (`armor.inf_light`) · FOOT 205 cm/s · F0 · sight 1150 cm · 1 slot · food staff 4 · ExperienceValue 21 |
| **Commands** | as RD-INF-01 · default stance Aggressive · formation role `frontline` |
| **Production** | **$210** (−45 % rule with an AT element; ×1.5 = $315 < AC-INF-01 $360) · `RD-BLD-03` · prerequisites: `RD-BLD-05` · **7 s** · 4 food · no cap |
| **Veterancy** | Standard |
| **AI** | `ai.unit.line_infantry`. Deltas: re-board the nearest friendly carrier when it is within 600 cm and the squad is Shaken |
| **Multiplayer** | MPA-STD |
| **Presentation** | ANIM-INF + RPG kneel. Icon: rifle and RPG |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.rd.inf_02.primary` assault rifles (3 members) | SMALL_ARMS | 0–640 cm | 9 per member / 1.0 s | hitscan | 3 | GROUND, AIR_LOW | 100 % / 15 % / 2 % / 25 % rotary / 8 % |
| `weapon.rd.inf_02.rpg` RPG gunner | SHAPED_CHARGE | 50–600 cm | 45 / 4.0 s | shell_direct 1200 cm/s, tracking slow | 2 | GROUND, STRUCTURE | 25 % / 100 % / 90 % / — / 70 % |

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.rd.inf_02.mounted_drill` | Passive (P) | Loads and unloads from RD-ARM-02, RD-ARM-03 and RD-SEA-02 at 0.25 s per slot (twice as fast) and counts its carrier as a coherent friendly unit for group support | Self | free | Killing the carrier | — | Dismount next to cover |
| `ability.shared.grenade` / `.sprint` (I) | Active | as framework | — | — | — | — | — |

**Research and upgrades:** `tech.rd.industrial_mobilization_methods`; `tech.shared.infantry_protective_equipment` (60 → 70, −5 % speed).

**Counters:** *Beats (hard):* scouts and light vehicles caught in the open (RPG). *Beats (soft):* rifle infantry. *Beaten by (hard):* HMG, flame, grenadiers, artillery. *Situational:* strongest with its carrier nearby.

**Status:** identity E-R; price rule E; everything else P.

### RD-INF-03 — Heavy Machine-Gun Team

| Field | Specification |
| --- | --- |
| **Identity** | `unit.rd.inf_03` · Red Directorate · Infantry / heavy-weapons team · `TPL-INF-HW` · **Core** · Playable |
| **Role** | **Primary:** suppression and checkpoint overwatch (E-R). **Secondary:** anti-helicopter fire; defend bunkers. **Doctrine fit:** concentrated firepower that pins enemies for the mass to close (E). |
| **Inspiration and background** | Tripod-mounted 12.7 mm-class heavy machine gun with a crew of three. C&C lineage (E-R): GDI Officer (Ren); Zone Defender (TT, squad role); Nod Officer (Ren). |
| **Visual identity** | Concept: [`RD-INF-03.png`](../../Factions/TheRedDirectorate/Units/RD-INF-03.png). Three members, a tripod HMG and ammunition boxes. |
| **Statistics** | 3 × 60 HP = **180** (`armor.inf_light`) · FOOT_HEAVY 180 cm/s · F0 · sight 1200 cm · 1 slot · food staff 3 · ExperienceValue 33 |
| **Commands** | as TPL-INF-HW (Deploy/Undeploy) · Hold Ground when deployed · formation role `support_weapon` |
| **Production** | **$260** (band mid $625 × 0.55 ≈ $344, lowered to $260 so that × 1.5 = $390 stays below AC-INF-04 $400 and IA-INF-03 $420, validator §1) · `RD-BLD-03` · prerequisites: `RD-BLD-03` · **9 s** · 3 food · no cap |
| **Veterancy** | Standard |
| **AI** | `ai.unit.support_weapon` |
| **Multiplayer** | MPA-STD |
| **Presentation** | ANIM-INF + deploy (tripod). Heavy tracer stream with a 90° arc indicator. Icon: tripod MG |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.rd.inf_03.primary` HMG (deployed) | SMALL_ARMS | 0–900 cm | 16 / 0.2 s (80 DPS) deployed; undeployed: 0–500 cm, 8 / 0.3 s; arc 90° when deployed | hitscan | **6** | GROUND, AIR_LOW | 100 % / 15 % (VEH_SOFT 50 %) / 2 % / 25 % rotary / 8 % |

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.shared.deploy` | Active (P) | Deployed: full HMG profile in a 90° arc; immobile | Self, facing direction | free · 3 s deploy / 2 s undeploy | Flanking outside the arc; mortars; marksmen | Arc cone (owner) | Deploy facing the observed threat axis |
| `ability.rd.inf_03.suppressive_fire` | Active (P) | 6 s area fire into a 300 cm circle within the arc: +100 % suppression stress, −50 % damage | Ground point in arc | free · 6 s · 25 s · 900 cm | Smoke; units already Pinned | Tracer sweep | Use on ≥ 2 infantry squads advancing in the open |

**Research and upgrades:** `tech.rd.coordinated_defensive_fire` (research doc §5.2 candidate; RD-BLD-02, $600): deployed HMGs within 1000 cm of an RD-DEF-06 tower get +100 cm range; trade-off: the tower must be powered.

**Counters:** *Beats (hard):* infantry in the open (AC-INF-01/02/04, IA-INF-01, MP-INF-01). *Beats (soft):* scouts, helicopters at 25 %. *Beaten by (hard):* marksmen (AC-INF-08, IA-INF-08, MP-INF-07), mortars and artillery, any armored vehicle. *Situational:* arc-bound; flanking kills it.

**Status:** identity E-R; price rule E; everything else P.

### RD-INF-04 — Portable Rocket Section

| Field | Specification |
| --- | --- |
| **Identity** | `unit.rd.inf_04` · Red Directorate · Infantry / anti-armor and limited AA · `TPL-INF-AT` · **Core** · Playable |
| **Role** | **Primary:** anti-vehicle and limited AA at low infantry cost (E-R). **Secondary:** anti-structure fire. **Doctrine fit:** cheap AT mass that does not need guidance technology. |
| **Inspiration and background** | Unguided RPG gunners with one man-portable SAM per section. C&C lineage (E-R): Rocket Infantry (TS); Tank Hunter (Gen); Flak Trooper (RA3). |
| **Visual identity** | Concept: [`RD-INF-04.png`](../../Factions/TheRedDirectorate/Units/RD-INF-04.png). Three members with RPG tubes and spare rockets in back packs. |
| **Statistics** | 3 × 60 HP = **180** (`armor.inf_light`) · FOOT 200 cm/s · F0 · sight 1100 cm · 1 slot · food staff 3 · ExperienceValue 30 |
| **Commands** | as TPL-INF-AT · default stance Guard · formation role `support_weapon` |
| **Production** | **$300** (−45 % rule; ×1.5 = $450 < AC-INF-03 $560) · `RD-BLD-03` · prerequisites: `RD-BLD-05` · **8 s** · 3 food · no cap |
| **Veterancy** | Standard |
| **AI** | `ai.unit.anti_armor_infantry` |
| **Multiplayer** | MPA-STD |
| **Presentation** | ANIM-INF + launcher, backblast. Icon: RPG over a tank |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.rd.inf_04.at` RPGs (3) | SHAPED_CHARGE | 50–650 cm | 45 per member / 4.0 s (33.8 DPS) | shell_direct 1200 cm/s, tracking slow (not jammable) | 2 | GROUND, STRUCTURE | 25 % / 100 % / 90 % / — / 70 % |
| `weapon.rd.inf_04.aa` one SAM per section | FLAK_AA | 0–850 cm | 40 / 8 s | guided_missile | — | AIR_LOW | — / — / — / 110 % rotary, 100 % low fixed / — |

**Abilities:** no unit-specific ability. The AA missile fires automatically at AIR_LOW targets; no mode switch (cheaper and simpler than AC-INF-03, weaker in AA).

**Research and upgrades:** `tech.rd.tandem_warheads` (RD-BLD-02, $600): RPG damage 45 → 52; trade-off: cooldown 4.0 → 4.4 s.

**Counters:** *Beats (hard):* light vehicles and APCs (AC-ARM-01/02, MP-ARM-01/02/04, IA-ARM-01). *Beats (soft):* MBTs in towns and in mass. *Beaten by (hard):* HMG and flame, marksmen, artillery; fast vehicles (tracking slow: −25 % against targets > 150 cm/s). *Situational:* unguided, so jamming does not affect it.

**Status:** identity E-R; price rule E; everything else P.

### RD-INF-05 — Mortar Infantry Team

| Field | Specification |
| --- | --- |
| **Identity** | `unit.rd.inf_05` · Red Directorate · Infantry / mortar team · `TPL-INF-HW` · **Extended** · Playable |
| **Role** | **Primary:** organic portable indirect fire for urban or mountain terrain (E-R). **Secondary:** smoke screens for mass assaults. **Doctrine fit:** artillery at every level of the formation (E). |
| **Inspiration and background** | 82 mm-class infantry mortar team. C&C lineage (E-R): Grenadier (RA1); Mortar Cycle (Uprising, indirect-fire concept); Disc Thrower (TS, area role). |
| **Visual identity** | Concept: [`RD-INF-05.png`](../../Factions/TheRedDirectorate/Units/RD-INF-05.png). Three members, mortar tube with bipod and baseplate. |
| **Statistics** | 3 × 60 HP = **180** (`armor.inf_light`) · FOOT_HEAVY 175 cm/s · F0 · sight 1100 cm · 1 slot · food staff 3 · ExperienceValue 34 |
| **Commands** | as TPL-INF-HW + Fire Mission · Hold Ground · formation role `support_weapon` |
| **Production** | **$340** · `RD-BLD-03` · prerequisites: `RD-BLD-05` · **10 s** · 3 food · no cap |
| **Veterancy** | Standard |
| **AI** | `ai.unit.artillery` (infantry variant) |
| **Multiplayer** | MPA-INDIRECT |
| **Presentation** | ANIM-INF + deploy, drop-and-fire. Icon: mortar |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.rd.inf_05.primary` 82 mm mortar (deployed) | HIGH_EXPLOSIVE | 350–1900 cm | 45 / 4.0 s; AmmoCapacity 40 | ballistic; full 70, max 160, edge 35 %; scatter 80–220 | 8 | GROUND, STRUCTURE | 110 % / 45 % / 12 % / — / 100 % |

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.shared.deploy` | Active (P) | Must deploy to fire; 3 s deploy, 2 s undeploy | Self | free | Raids | Baseplate mesh | Deploy 1200–1500 cm behind the front |
| `ability.rd.inf_05.smoke_rounds` | Active (P) | §9.5 smoke cylinder | Ground point in range | 2 ammo · 20 s · 35 s · 350–1900 cm | Detectors | Smoke | Screen mass assaults crossing open ground |

**Research and upgrades:** `tech.rd.coordinated_defensive_fire` applies (+100 cm near RD-DEF-06). `tech.shared.supply_preservation`: resupply −25 %.

**Counters:** *Beats (hard):* infantry in cover, HMG and AT teams, garrisons. *Beaten by (hard):* raiders and marksmen; counter-battery (AC-ARM-12). *Situational:* mountain and urban maps where line of sight is short.

**Status:** identity E-R; price rule E; everything else P.

### RD-INF-06 — Combat Pioneer

| Field | Specification |
| --- | --- |
| **Identity** | `unit.rd.inf_06` · Red Directorate · Infantry / engineer · `TPL-INF-SUPPORT` · **Core** · Playable |
| **Role** | **Primary:** repair, bridge improvement, capture authorized facilities (E-R). **Secondary:** clear mines and obstacles; restore rail lines. **Doctrine fit:** engineering troops establish crossings and restore facilities (E: *Industrial Logistics*). |
| **Inspiration and background** | Combat pioneers with tools, mine probes and bridging kit. C&C lineage (E-R): Engineer (RA1); Combat Engineer (RA3); Engineer (TT). |
| **Visual identity** | Concept: [`RD-INF-06.png`](../../Factions/TheRedDirectorate/Units/RD-INF-06.png). Three members, shovels, probes, tool boxes. |
| **Statistics** | 3 × 55 HP = **165** (`armor.inf_light`) · FOOT 200 cm/s · F0 · sight 1000 cm · detects mines 300 cm · 1 slot · food staff 3 · no veterancy |
| **Commands** | Move, Stop, Guard (I) · Capture, Repair, Clear, Improve Bridge (P) · Hold Fire · formation role `protected` |
| **Production** | **$240** (−45 % rule; ×1.5 = $360 < AC-INF-06 $450) · `RD-BLD-04` Military Engineering Works (or `RD-BLD-03`) · **9 s** · 3 food · no cap |
| **Veterancy** | none |
| **AI** | `ai.unit.support_infantry` |
| **Multiplayer** | MPA-STD |
| **Presentation** | ANIM-INF + work loop. Icon: shovel and wrench |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.rd.inf_06.sidearm` carbines | SMALL_ARMS | 0–450 cm | 6 per member / 1.2 s | hitscan | 1 | GROUND | 100 % / 15 % / 2 % / — / 8 % |

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.shared.capture_structure` | Active (P) | 8 s capture (§10.6) | Eligible structure | free · 8 s | Damage interrupts | Capture ring | Explicit order only |
| `ability.shared.field_repair` | Active (P) | 8 HP/s vehicles; also **12 HP/s to rail-connected structures and bridges** | Friendly vehicle or structure ≤ 300 cm | USD per HP | Suppression | Sparks | Auto under Guard |
| `ability.rd.inf_06.improve_bridge` | Active (P) | Repairs a destroyed authored bridge or pass segment to passable (300 HP) in 20 s, or reinforces an intact one (+50 % HP) | Authored bridge or pass segment ≤ 200 cm; Unaligned infrastructure only with permission (E) | $150 · 20 s · — · 200 cm | Artillery on the crossing | Scaffold mesh | Restore broken crossings on the army route |
| `ability.rd.inf_06.clear_obstacle` | Active (P) | Mines, obstacles and wrecks removed in 6 s | ≤ 150 cm | free · 6 s | Covering fire | Probe animation | Clear observed mines |

**Research and upgrades:** `tech.shared.vehicle_component_standardization`: repair USD −20 %.

**Counters:** *Beaten by (hard):* any armed unit. *Situational:* essential where bridges and rail lines decide logistics (E).

**Status:** identity E-R; jurisdiction E; everything else P.

### RD-INF-07 — Rail Security Troop

| Field | Specification |
| --- | --- |
| **Identity** | `unit.rd.inf_07` · Red Directorate · Infantry / security guard platoon · `TPL-INF-LINE` · **Extended** · Playable |
| **Role** | **Primary:** low-cost guard formation protecting trains and depots (E-R). **Secondary:** spy screening at depots. **Doctrine fit:** rail yards and depots are the arteries of Directorate power (E). |
| **Inspiration and background** | Interior and railway security troops with rifles and a guard dog handler. C&C lineage (E-R): Militant Rocket Squad (TW); Red Guard (Gen); GDI Soldier (Ren). |
| **Visual identity** | Concept: [`RD-INF-07.png`](../../Factions/TheRedDirectorate/Units/RD-INF-07.png). Four members in blue-tabbed security uniforms. |
| **Statistics** | 4 × 50 HP = **200** (`armor.inf_light`) · FOOT 195 cm/s · F0 · sight 1150 cm · **`DETECT.SPY` 400 cm** · 1 slot · food staff 4 · ExperienceValue 16 |
| **Commands** | as RD-INF-01 · default stance Guard · formation role `rear_guard` |
| **Production** | **$160** · `RD-BLD-03` · prerequisites: `RD-BLD-11` · **6 s** · 4 food · no cap |
| **Veterancy** | Standard |
| **AI** | `ai.unit.line_infantry`. Deltas: Guard the nearest RD-BLD-11, RD-DEF-11 or RD-SUP-02/03 route node; never leave a 1000 cm leash |
| **Multiplayer** | MPA-STD |
| **Presentation** | ANIM-INF; guard-dog cosmetic member. Icon: rifle over rails |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.rd.inf_07.primary` rifles | SMALL_ARMS | 0–620 cm | 8 per member / 1.0 s; +20 % within 800 cm of RD-BLD-11, RD-DEF-11 or an authored rail spline (*Depot Defense*) | hitscan | 3 | GROUND | 100 % / 15 % / 2 % / — / 8 % |

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.rd.inf_07.depot_defense` | Conditional passive (P) | +20 % weapon damage and `DETECT.SPY` 400 → 600 cm within 800 cm of a depot or rail spline | Self | free | Fighting it away from depots | Rail icon | Stay on the leash |

**Research and upgrades:** `tech.rd.industrial_mobilization_methods`.

**Counters:** *Beats (hard):* agents and saboteurs at depots (AC-INF-11, IA-INF-09/10, MP-INF-08/09). *Beaten by (hard):* HMG, flame, artillery, any armor. *Situational:* cheap rear security that frees combat squads.

**Status:** identity E-R; price rule E; everything else P.

### RD-INF-08 — Chemical Defense Detachment

| Field | Specification |
| --- | --- |
| **Identity** | `unit.rd.inf_08` · Red Directorate · Infantry / protection and decontamination · `TPL-INF-SUPPORT` · **Extended** · Playable |
| **Role** | **Primary:** protective gear, decontamination and contamination counterplay; **no biological weapon** (E-R). **Secondary:** clear abstract contamination from friendly facilities; screen against incendiary area denial. **Doctrine fit:** large formations must keep operating through hostile disruption (E: *Military Doctrine*). |
| **Inspiration and background** | Chemical-defense troops in rubberized suits and respirators with a decontamination sprayer. C&C lineage (E-R): Desolator (RA2, protection role only); Zone Trooper (TW, hazardous-environment protection). |
| **Visual identity** | Concept: [`RD-INF-08.png`](../../Factions/TheRedDirectorate/Units/RD-INF-08.png). Three members in olive protective suits and gas masks; one carries a backpack sprayer. |
| **Statistics** | 3 × 65 HP = **195** (`armor.inf_protected`) · FOOT 185 cm/s · F0 · sight 1100 cm · 1 slot · food staff 3 · ExperienceValue 28 |
| **Commands** | Move, Stop, Guard, AttackMove (I) · Decontaminate (P) · default stance Guard · formation role `protected` |
| **Production** | **$280** (support band ≈ $510 × 0.55) · `RD-BLD-03` · prerequisites: `RD-BLD-04` · **9 s** · 3 food · no cap |
| **Veterancy** | Standard |
| **AI** | `ai.unit.support_infantry`. Deltas: move to any friendly facility or unit group carrying a contamination status; otherwise follow the nearest armor group at 600 cm |
| **Multiplayer** | MPA-STD. Contamination status on own facilities is owner-only (E: discovery may lag) |
| **Presentation** | ANIM-INF + spray loop, mask breathing SFX. White decon mist VFX. Icon: respirator |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.rd.inf_08.sidearm` carbines | SMALL_ARMS | 0–500 cm | 7 per member / 1.1 s | hitscan | 2 | GROUND | 100 % / 15 % / 2 % / — / 8 % |

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.rd.inf_08.decontaminate` | Active (P) | Removes an abstract contamination status (E: Ascendancy facility contamination) from one friendly building in 15 s; stops further local spread from it and restores its workforce output from the next upkeep step. Removes `burning` from friendly units in 300 cm instantly | Friendly building ≤ 300 cm, or self-centred 300 cm | $120 · 15 s channel · 30 s · 300 cm | Killing the detachment; repeated contamination; Unaligned jurisdiction (neutral sites need permission) | Mist VFX; status clear tick | Prioritize producers, then research buildings |
| `ability.rd.inf_08.protective_posture` | Passive (P) | Squad and friendly infantry within 300 cm take −30 % INCENDIARY damage and ignore the stress of contamination events | Self aura 300 cm | free | Massed small arms (suits do not stop bullets) | Faint aura (owner) | Stay with assault infantry against flame users |

**Research and upgrades:** `tech.rd.collective_protection` (RD-BLD-02, $500): *Decontaminate* 15 → 10 s; trade-off: price +$40 and speed −5 %. `tech.shared.infantry_protective_equipment` (65 → 75, −5 % speed).

**Counters:** *Beats (hard):* Ascendancy facility contamination (IA-INF-10 and its sabotage track) as a remedy, not a kill. *Beats (soft):* flame users (AC-INF-04 grenades excepted) through *Protective Posture*. *Beaten by (hard):* any combat infantry, HMG, artillery. *Situational:* worthless in a match with no contamination or flame threat; build reactively.

**Status:** identity and "no biological weapon" E-R; contamination as an abstract facility interaction E (Espionage §2); all values P.

### RD-INF-09 — Special Recon Riflemen

| Field | Specification |
| --- | --- |
| **Identity** | `unit.rd.inf_09` · Red Directorate · Infantry / recon and ambush · `TPL-INF-SPEC` · **Core** · Playable · "elite" in description only, not talent-eligible (E: framework §17) |
| **Role** | **Primary:** veteran ground observation, flanking and ambush suppression (E-R). **Secondary:** spot for RD-ARM-07/08 and RD-INF-05; hunt enemy agents and marksmen. **Doctrine fit:** reconnaissance inside the combined-arms system (E). |
| **Inspiration and background** | Reconnaissance troops with scoped designated-marksman rifles and binoculars. C&C lineage (E-R): Sniper Team (TW); Pathfinder (Gen); Shadow Team (TW, recon role). |
| **Visual identity** | Concept: [`RD-INF-09.png`](../../Factions/TheRedDirectorate/Units/RD-INF-09.png). Two members in camouflage smocks, one with a scoped rifle, one with binoculars and radio. |
| **Statistics** | 2 × 70 HP = **140** (`armor.inf_light`) · FOOT 215 cm/s · F0 · sight **1600 cm** · `DETECT.CONCEALED` 900 cm · Concealed after 3 s stationary · 1 slot · food staff 2 · ExperienceValue 45 |
| **Commands** | as TPL-INF-SPEC + Designate Target · default stance Hold Fire (ambush) · formation role `scout_screen` |
| **Production** | **$420** (specialist band mid $625 × 0.55 ≈ $344 + recon premium; ×1.5 = $630 < AC-INF-08 $650, IA-INF-08 $640, MP-INF-07 $640) · `RD-BLD-03` · prerequisites: `RD-BLD-05` · **12 s** · 2 food · no cap |
| **Veterancy** | Standard; starts at Veteran rank (P: "veteran" in the roster role) without the Veteran HP bonus being stacked twice |
| **AI** | `ai.unit.specialist_infantry`. Deltas: designate artillery before firing its own weapon when an RD artillery unit is in range of the target |
| **Multiplayer** | MPA-STD; concealment per MPA-COVERT rules (concealed position never replicates) |
| **Presentation** | ANIM-INF + prone aim, binocular idle. Scope glint tell when concealed and within 600 cm of an enemy. Icon: binoculars |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.rd.inf_09.primary` designated-marksman rifle | PRECISION | 0–1000 cm | 55 / 2.5 s (single member) | hitscan | 8 on kill | GROUND | 160 % / 2 % / 0 % / — / 0 % |
| `weapon.rd.inf_09.carbine` spotter carbine | SMALL_ARMS | 0–550 cm | 9 / 1.0 s | hitscan | 2 | GROUND | 100 % / 15 % / 2 % / — / 8 % |

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.rd.inf_09.spot_for_artillery` | Active (P) | Designates an observed point or unit for 20 s: friendly indirect fire there has observed-aim scatter (×0.7, §6.6) and may fire on it although the gun is out of sight | Observed ground or unit ≤ 1600 cm | free · 20 s · 25 s · 1600 cm | Breaking line of sight; smoke; killing the spotter | Red laser-less flare marker (owner and allies) | Use on clusters of ≥ 3 enemy units |
| `ability.rd.inf_09.ambush_volley` | Active (P) | From Concealed: next marksman shot +50 % damage and applies 15 suppression stress to the target squad and 8 to squads within 300 cm | Self while Concealed | free · instant · 30 s | `DETECT.CONCEALED` sources reveal before it fires | Muzzle flash; target flinch | Open on officers, AT and HW teams first |

**Research and upgrades:** `tech.shared.tactical_radio_modernization`: designate range +200 cm. `tech.rd.recon_field_training` (RD-BLD-02, $500): concealment delay 3 → 2 s; trade-off: build time +2 s (a price rise would break the Directorate price validator, §1).

**Counters:** *Beats (hard):* officers, agents, HW and AT teams (AC-INF-03/10/11, IA-INF-03/09, MP-INF-03/08). *Beats (soft):* line infantry at range. *Beaten by (hard):* scouts and dogs with detection, armored cars, artillery once revealed, enemy marksmen (AC-INF-08, IA-INF-08, MP-INF-07). *Situational:* the eyes of every Directorate artillery park.

**Status:** identity E-R; reconnaissance rules E (no theft, no sabotage permission); all values P.

### RD-INF-10 — Political Liaison Officer

| Field | Specification |
| --- | --- |
| **Identity** | `unit.rd.inf_10` · Red Directorate · Infantry / **officer** (no talents) · `TPL-INF-COMMAND` (officer subset) · **Core** · Playable |
| **Role** | **Primary:** supports friendly unit Discipline; **does not control enemy minds** (E-R). **Secondary:** formation anchor for mass infantry; counter to enemy resolve attacks. **Doctrine fit:** the party apparatus inside the army; cohesion makes mass work (E). |
| **Inspiration and background** | Political officer with a peaked cap, pistol and loudhailer. C&C lineage (E-R): Confessor Cabal (KW, morale role); Speaker Tower (Gen); Propaganda Tower (Gen, upgrade role). |
| **Visual identity** | Concept: [`RD-INF-10.png`](../../Factions/TheRedDirectorate/Units/RD-INF-10.png). One officer with a red-band peaked cap and a loudhailer, plus a cosmetic flag bearer. |
| **Statistics** | 1 × 220 HP = **220** (`armor.inf_protected`) · FOOT 210 cm/s · F0 · sight 1300 cm · 1 slot · food staff 2 · ExperienceValue 40 |
| **Commands** | Move, Stop, Guard, AttackMove (I) · Rally (P) · default stance Guard · formation role `formation_anchor` (E: officers only, framework §13.4) |
| **Production** | **$420** (officer band, −45 % rule; ×1.5 = $630, no cheaper comparable officer exists) · `RD-BLD-03` · prerequisites: `RD-BLD-16` · **14 s** · 2 food · **cap 4 active** |
| **Veterancy** | Standard (officers are ordinary units, not commanders) |
| **AI** | `ai.unit.officer`. Deltas: stay inside the largest coherent RD infantry cluster; retreat at 35 % HP |
| **Multiplayer** | MPA-STD; aura membership is server-only; enemies see the officer and its banner when observed |
| **Presentation** | ANIM-INF + loudhailer gesture, salute. Owner and allies see the aura ring; enemies see the flag only. Icon: red star on a cap |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.rd.inf_10.sidearm` pistol | SMALL_ARMS | 0–400 cm | 12 / 1.0 s | hitscan | 1 | GROUND | 100 % / 15 % / 2 % / — / 8 % |

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.rd.inf_10.political_presence` | Passive aura (P) | Friendly RD combat infantry within 900 cm: resolve recovery +30 %; stress taken −10 %. Does not stack with a second officer or commander aura (highest applies). Scales with Population Morale baseline (E) | Self aura 900 cm | free | Killing the officer; jamming cuts links that pass through the field (§9.5) | Owner ring | Always on |
| `ability.rd.inf_10.rally` | Active (P) | Friendly Breaking or Pinned RD infantry within 900 cm gain +30 resolve instantly (capped at their maximum); Breaking units stop auto-withdraw | Self-centred 900 cm | free · instant · 60 s | Sustained suppression re-pins them; killing the officer first | Loudhailer cry; flag wave | Use when ≥ 2 squads in radius are Pinned or Breaking |

**Research and upgrades:** `tech.rd.mass_political_education` (RD-BLD-16, $700): aura radius 900 → 1100 cm; trade-off: officer cap 4 → 3. Propaganda structures (RD-DEF-08/09/10) raise Population Morale and therefore the resolve baseline maximum (E), not the aura itself.

**Counters:** *Beats (soft):* Concord Psy-Ops attrition and Ascendancy Intimidation set-ups (keeps squads above Broken and out of isolation; it never makes them immune). *Beaten by (hard):* marksmen (AC-INF-08, IA-INF-08, MP-INF-07), artillery, jammers cutting aura links (AC-ARM-10, IA-ARM-14). *Situational:* no effect on enemies; no mind control (E-R).

**Status:** identity and "does not control enemy minds" E-R; resolve model E (one meter); officer anchor rule P (framework §13.4); all values P.

### RD-INF-11 — Directorate Intelligence Courier

| Field | Specification |
| --- | --- |
| **Identity** | `unit.rd.inf_11` · Red Directorate · Infantry / covert agent · `TPL-INF-COVERT` · **Conditional** (espionage track) · Playable |
| **Role** | **Primary:** **cheaper and faster rival-intelligence theft** by infiltrating eligible enemy sites (E). **Secondary:** later, spy-only attached-charge sabotage at a higher price than Meridian (E). **Doctrine fit:** repeated reconnaissance and rival-research acquisition (E: faction description). |
| **Inspiration and background** | Courier-cover intelligence officer with a satchel. Abstract game interactions only. C&C lineage (E-R): Spy (RA1); Black Lotus (Gen); Saboteur (TW). |
| **Visual identity** | Concept: [`RD-INF-11.png`](../../Factions/TheRedDirectorate/Units/RD-INF-11.png). True form (owner only): grey coat, flat cap, document satchel. Opponents see the disguise archetype. |
| **Statistics** | 1 × 100 HP = **100** (`armor.inf_light`) · FOOT 215 cm/s · F0 · sight 1200 cm · no detector · 1 slot · food staff 1 · no veterancy |
| **Commands** | Move, Stop (I) · Disguise, Infiltrate, Plant Charge, Withdraw (P) · default stance Hold Fire · formation role `protected` |
| **Production** | **$640** (agent band mid $1,150 × 0.55; ×1.5 = $960 < AC-INF-11 $1,300) · `RD-BLD-03` · prerequisites: `RD-BLD-02` and `RD-DEF-07` operational · **16 s** · 1 food · cap 4 active |
| **Veterancy** | none (§8) |
| **AI** | `ai.unit.covert_agent`. Deltas: prefer research buildings, then producers; never target a building with an observed `DETECT.SPY` source within 1200 cm |
| **Multiplayer** | **MPA-COVERT**: true archetype, intelligence tokens and charge timers never replicate to opponents until detection or detonation |
| **Presentation** | ANIM-INF + civilian walk. Owner sees a true-form overlay. Icon: satchel with a star |

**Weapons:** none (unarmed; attached charges are an ability).

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.shared.disguise` | Active (P) | As AC-INF-11 | Self | free · until broken · 10 s after reveal | `DETECT.SPY` (AC-INF-10, IA-INF-12, UN-INF-10/11 strongest), hostile action | Owner mask icon | Re-disguise out of detector range |
| `ability.rd.inf_11.steal_intelligence` | Active (P; E advantage) | **10.2 s** inside an eligible enemy producer, research or HQ building (the shared 12 s base −15 %): reveals the victim's production queue and completed technologies for 90 s and grants one intelligence token for that building class. Uses the same detection checks as other factions; no guaranteed success (E) | Enemy producer, research or HQ building | **$170** (shared $200 base −15 %) · 10.2 s · 60 s · contact | `DETECT.SPY` within 900 cm aborts; garrisons; Unaligned forged reports may pollute later readings (E) | Owner intel panel; victim receives no alert unless detected | Use on the highest-tier observed building |
| `ability.shared.plant_charge` | Active (P; E permission) | Fictional attached charge: after a 15 s fuse, DEMOLITION 450 to an eligible vehicle or building | Requires `tech.rd.late_espionage`; stationary target ≤ 100 cm | **$300 per charge** (above Meridian, E) · 15 s fuse · 45 s · contact; 2 carried | `DETECT.SPY` reveals agent and charge; victim *Investigate* removes it | Owner fuse timer | Only on stationary high-value targets with no detector within 1200 cm |

**Research and upgrades:** `tech.rd.late_espionage` (RD-BLD-02, $1,200, 90 s; maps to I `late_espionage_unlocked`): enables *Plant Charge*. `tech.rd.courier_networks` ($600): intelligence token reveal 90 → 120 s; trade-off: courier cap 4 → 3.

**Counters:** *Beats:* information denial; repeated research theft. *Beaten by (hard):* `DETECT.SPY` sources and escorts; Unaligned counterintelligence (UN-INF-10/11), which can also plant false reports (E). *Situational:* the Directorate's −15 % applies only to rival-intelligence theft, not to charges or diversion (E).

**Status:** identity E-R; theft advantage, spy-only later costlier charges and Unaligned forgery risk E; `CanPlantExplosives` I; −15 % and every value P (no canonical percentage, E).

### RD-INF-12 — Resource Diversion Agent

| Field | Specification |
| --- | --- |
| **Identity** | `unit.rd.inf_12` · Red Directorate · Infantry / covert agent · `TPL-INF-COVERT` · **Conditional** (espionage track) · Playable |
| **Role** | **Primary:** covert transfer of permitted resources from the victim's account into the Directorate budget (E). **Secondary:** economic harassment that may go unnoticed until an audit or counterintelligence discovery (E). **Doctrine fit:** state intelligence serving the planned economy. |
| **Inspiration and background** | Ministry auditor cover identity with a ledger case. No real-world financial method is modeled (E). C&C lineage (E-R): Thief (RA1); Hacker (Gen); Black Lotus (Gen). |
| **Visual identity** | Concept: [`RD-INF-12.png`](../../Factions/TheRedDirectorate/Units/RD-INF-12.png). True form (owner only): suit, spectacles, ledger case. |
| **Statistics** | 1 × 90 HP = **90** (`armor.inf_light`) · FOOT 205 cm/s · F0 · sight 1150 cm · 1 slot · food staff 1 · no veterancy |
| **Commands** | Move, Stop (I) · Disguise, Infiltrate, Divert, Withdraw (P) · default stance Hold Fire · formation role `protected` |
| **Production** | **$700** (agent band, −45 % rule; ×1.5 = $1,050 below every other faction's agent) · `RD-BLD-03` · prerequisites: `RD-BLD-02`, `RD-DEF-07` and `tech.rd.resource_diversion_doctrine` · **18 s** · 1 food · **cap 2 active** |
| **Veterancy** | none |
| **AI** | `ai.unit.covert_agent`. Deltas: target the observed enemy economic building with the highest stored resource of a transferable type |
| **Multiplayer** | **MPA-COVERT + MPA-ECON**: the ledger transaction is server-authoritative, debits and credits in one operation (E: conservation), and is not attributed to the victim until discovery |
| **Presentation** | ANIM-INF + briefcase. Owner sees a transfer ticker. Icon: ledger with an arrow |

**Weapons:** none (unarmed).

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.shared.disguise` | Active (P) | As RD-INF-11 | Self | free | `DETECT.SPY` | Owner mask | as RD-INF-11 |
| `ability.rd.inf_12.divert_resources` | Active channel (P; E rule) | While inside an eligible enemy economic building (refinery, depot, treasury-class office, warehouse): every 5 s transfers 4 % of the victim's stock of **one** transferable resource type linked to that building (USD, petroleum, kerosene or food), max $400 equivalent per 5 s and $2,400 per operation, into the Directorate stock of the same type. Storage limits apply; overflow is lost and recorded as a fee (E). Never touches nuclear materials (E) | Eligible enemy economic building; one resource type chosen at start | $150 · up to 30 s (6 ticks) · 120 s · contact | `DETECT.SPY`; the victim's audit (counterintelligence) reveals the transaction log and attribution | Owner ticker; victim sees only lower stock | Abort after 3 ticks if a detector is observed approaching |

**Research and upgrades:** `tech.rd.resource_diversion_doctrine` (RD-BLD-02, $900, 75 s): unlocks the unit. No percentage upgrade is proposed: the open balance question is recorded as U-RD-02.

**Counters:** *Beaten by (hard):* `DETECT.SPY` sources at economic buildings (AC-INF-10, IA-INF-12; Unaligned UN-INF-10/11 at neutral sites). *Situational:* only useful against an opponent with stocked resources; the ledger audit is its long-term answer.

**Status:** identity E-R; conservation (debit = credit), possible non-attribution and no-nuclear rule E; all rates and caps P (U-RD-02).

### RD-INF-13 — Special Forces Colonel

| Field | Specification |
| --- | --- |
| **Identity** | `unit.rd.inf_13` · Red Directorate · Infantry / **Elite commander** (talent candidate, E: talent doc §5) · `TPL-INF-COMMAND` · **Extended** · Playable · **cap 1** (E) |
| **Role** | **Primary:** limited elite raid and target-designation commander (E-R). **Secondary:** coordination hub for formations, artillery observation, discipline and reserve organization (E: talent identity). **Doctrine fit:** operational planning that makes a mass army act in concert. |
| **Inspiration and background** | Special-forces colonel in a camouflage smock with a radio operator. C&C lineage (E-R): Boris (YR); Commando (TD); Natasha (RA3). |
| **Visual identity** | Concept: [`RD-INF-13.png`](../../Factions/TheRedDirectorate/Units/RD-INF-13.png). Officer with a beret, carbine and binoculars; cosmetic radio operator; command pennant at RTS zoom. |
| **Statistics** | 1 × 500 HP = **500** (`armor.inf_protected`) · FOOT 220 cm/s · F0 · sight **1500 cm** · `DETECT.CONCEALED` 800 cm · 1 slot · food staff 2 · no ordinary veterancy (E) |
| **Commands** | as TPL-INF-COMMAND + talent abilities · default stance Guard · formation role `formation_anchor` |
| **Production** | **$1,250** (elite band mid $2,200 × 0.55; ×1.5 = $1,875 < AC-INF-14 $2,400; commander pricing under the infantry rule is U-RD-01) · `RD-BLD-03` · prerequisites: `RD-BLD-02` and `RD-DEF-06` operational · **30 s** · 2 food · **cap 1** (E). Redeployment: 90 s after death at full price, resuming the stored appointment (E: talent doc §4.4) |
| **Veterancy** | none (E: commanders progress only by talents) |
| **Command radius** | **1400 cm** baseline (larger than Concord's 1200: mass formations, short individual range). Links cut through enemy jamming (§9.5) |
| **Leadership effects** | Passive *Iron Cohesion*: own combat units in radius count as **coherent** for the very-massive-army bonus at 10 units instead of 12 (E: bonus exists; P: threshold change, bounded, never below 10) |
| **Vulnerabilities** | Marksmen, artillery and jamming; Psy-Ops and Intimidation ineligible (§7.5); death clears all live effects (E) |
| **AI** | `ai.unit.commander`. Deltas: stay 600–900 cm behind the largest coherent group; pick talents by the size of the owned army (≥ 25 units → Formation path) |
| **Multiplayer** | MPA-STD + **MPA-COMMAND** |
| **Presentation** | ANIM-INF + command gestures, radio call. Aura ring for owner and allies; pennant only for enemies. Icon: star over binoculars |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.rd.inf_13.primary` folding-stock carbine | SMALL_ARMS | 0–750 cm | 26 / 0.8 s (32.5 DPS) | hitscan | 3 | GROUND, AIR_LOW | 100 % / 15 % / 2 % / 25 % rotary / 8 % |
| `ability.shared.grenade` (I) | HIGH_EXPLOSIVE | 650 cm | 50, 150 cm | lobbed | 8 | GROUND, STRUCTURE | as RD-INF-01 |

**Abilities and proposed talent set** (three options per tier; one per tier; E: tier structure and sample path, P: effects, awaiting owner approval under talent doc §5)

| Tier | Option A | Option B | Option C |
| --- | --- | --- | --- |
| I Initial Doctrine | **Formation Discipline** (E sample): eligible coherent units in radius take −10 % stress (stacks with the very-massive bonus, cap −30 % total) | **Raid Leadership:** groups of ≤ 4 units within radius move +10 % and ignore the isolation penalty while the colonel is with them | **Signals Discipline:** links resist jamming unless the jammer is within 600 cm |
| II Field Specialization | **Artillery Coordination** (E sample): designate an observed point for 20 s; RD indirect fire there has scatter ×0.5 and −15 % cooldown; cooldown 45 s | **Target Designation:** mark one observed vehicle or structure for 15 s; friendly direct fire on it +15 % damage; cooldown 40 s | **Engineer Priority:** RD-INF-06, RD-SUP-04/05 in radius work +30 % faster |
| III Advanced Command | **Operational Reserves** (E sample): choose up to 6 **existing** idle RD units within 4000 cm; they receive a Move order to the colonel and +20 % speed for 15 s; no units created (E: no free manpower); cooldown 120 s | **Hold the Line:** 15 s, units in radius on Hold Ground cannot drop below Pinned; cooldown 120 s | **Rail Tempo:** RD-SUP-02/03/07 in radius load and unload at double speed for 20 s; cooldown 90 s |
| IV Signature Operation | **Combined Front Offensive** (E sample): 20 s, units in radius ordered to AttackMove get −15 % cooldown and +10 % speed; afterwards they suffer −10 % cooldown speed for 10 s (exhaustion); cooldown 180 s | **Fire Wall:** 3 designated points; RD artillery in range fires one free salvo of its own weapon at each (ammunition still consumed); cooldown 240 s | **Deep Strike:** for 30 s the colonel and up to 3 adjacent infantry squads are Concealed while moving at ≤ 75 % speed; cooldown 200 s |

Every talent follows the commander influence invariant (talent doc §3.3) and the protected canon: no free manpower and no loss of the infantry-price doctrine (E).

**Research and upgrades:** research never grants talents (E). `tech.shared.tactical_radio_modernization`: radius +200 cm (clamp 2000 cm).

**Counters:** *Beats:* nothing alone; multiplies mass formations and artillery. *Beaten by (hard):* marksmen (AC-INF-08, IA-INF-08, MP-INF-07), artillery, jammers (AC-ARM-10, IA-ARM-14). *Situational:* strongest with RD-ARM-07, RD-INF-09 and large infantry blocks.

**Status:** identity E-R; talent candidacy, cap 1, OXP and death rules E; price, radius and all talent effects P; owner approval pending (talent doc §5; U-RD-01 for price).

## 4. Armored, mechanized and artillery forces

Vehicle prices sit at the **lower-middle** of each §4.3 band: plainer, standardized hulls (E: standardized equipment) with no faction discount (E: not every category). Heavy hulls pay the *fuel-hungry* surcharge: production petroleum +40 % for `heavy` and `super` classes (E-R: RD-ARM-05; P value).

### RD-ARM-01 — Patrol Motorcycle

| Field | Specification |
| --- | --- |
| **Identity** | `unit.rd.arm_01` · Red Directorate · Vehicle / **canonical opening scout** (E) · `TPL-VEH-SCOUT` · **Core** · Playable |
| **Role** | **Primary:** opening reconnaissance with basic rifle fire and little armor (E). **Secondary:** chase off enemy scouts; picket rail lines. **Doctrine fit:** cheap ground reconnaissance; no espionage (E). |
| **Inspiration and background** | Military motorcycle with sidecar, a rider and a rifleman passenger. C&C lineage (E-R): Recon Bike (TD); Attack Cycle (TS); Attack Bike (KW). |
| **Visual identity** | No vehicle sheet: none (asset requirement). Olive motorcycle and sidecar; rifleman with a steel helmet; red star stencil on the sidecar nose. |
| **Statistics** | **200 HP** (`armor.veh_soft`) · WHEEL_LIGHT **580 cm/s** · F1 · sight **1800 cm** · `DETECT.CONCEALED` 1080 cm (60 % of sight) · crew 2 · food 2 · fuel `light` · ExperienceValue 18 |
| **Commands** | Move, Attack, AttackMove, Stop, Guard (I) · Patrol (P) · default stance Aggressive · formation role `scout_screen` |
| **Production** | **$180** · `RD-BLD-05` (also `RD-BLD-01` before the Combine exists) · prerequisites: none · **6 s** · 2 food · 1 petroleum · no cap |
| **Veterancy** | Standard |
| **AI** | `ai.unit.scout`. Deltas: may engage targets with CV ≤ 1.2 × its own (rifle-armed, E) |
| **Multiplayer** | MPA-STD |
| **Presentation** | ANIM-VEH-WHEEL + rider lean; passenger fires from the sidecar. Icon: motorcycle with a rifle |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.rd.arm_01.primary` passenger rifle (E: rifle-armed) | SMALL_ARMS | 0–600 cm | 10 / 1.0 s; fire while moving at ×0.5 cooldown speed | hitscan | 2 | GROUND, AIR_LOW | 100 % / 15 % / 2 % / 25 % rotary / 8 % |

**Abilities:** no unit-specific ability. Scouting never steals intelligence, diverts resources or plants charges (E).

**Research and upgrades:** `tech.rd.motorized_reconnaissance` (RD-BLD-02, $300): sight 1800 → 2000 cm; trade-off: price +$20.

**Counters:** *Beats (hard):* enemy agents and unescorted engineers in the open. *Beats (soft):* MP-ARM-01 at equal numbers only (Meridian has the better early recon, E). *Beaten by (hard):* any armored vehicle, HMG, rifle squads at close range. *Situational:* scout planes (AC-SUP-01, IA-SUP-01) see further and cannot be shot by it; the motorcycle's edge is that it can fight scouts and agents on the ground.

**Status:** rifle-armed opening scout and no espionage E (validator: has a rifle weapon); identity E-R; values P.

### RD-ARM-02 — Motorized Infantry Truck

| Field | Specification |
| --- | --- |
| **Identity** | `unit.rd.arm_02` · Red Directorate · Vehicle / light transport · `TPL-VEH-TRANSPORT` · **Core** · Playable |
| **Role** | **Primary:** cheap troop delivery and logistics support (E-R). **Secondary:** emergency resupply of mortars and rockets. **Doctrine fit:** move mass infantry along roads. |
| **Inspiration and background** | Six-wheeled army lorry with canvas tilt and a cab-mounted machine gun. C&C lineage (E-R): Flak Track (RA2); Technical (Gen); Troop Crawler (Gen, role). |
| **Visual identity** | none (asset requirement). Dark-green lorry, canvas tilt, red star on the doors. |
| **Statistics** | **340 HP** (`armor.veh_soft`) · WHEEL_HEAVY 440 cm/s · F1 · sight 1100 cm · **3 infantry slots**, no fire ports · crew 1 · food 1 · fuel `light` · ExperienceValue 25 |
| **Commands** | Move, Attack, Stop, Guard (I) · Load/Unload, Resupply (P) · default stance Guard · formation role `protected` |
| **Production** | **$250** · `RD-BLD-05` · **8 s** · 1 food · 1 petroleum · no cap |
| **Veterancy** | Standard (armed) |
| **AI** | `ai.unit.transport` |
| **Multiplayer** | MPA-STD + MPA-TRANSPORT |
| **Presentation** | ANIM-VEH-WHEEL + tailgate. Icon: truck with figures |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.rd.arm_02.mg` cab MG | SMALL_ARMS | 0–600 cm | 9 / 0.3 s | hitscan | 2 | GROUND, AIR_LOW | 100 % / 15 % / 2 % / 25 % rotary / 8 % |

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.rd.arm_02.ammunition_load` | Passive (P) | Carries 12 ammunition units; Resupply aura (§10.4) to RD-INF-05 and RD-INF-04 within 400 cm; refills at RD-BLD-11 or RD-DEF-11 | Self aura | §10.4 pricing (U-09) | Killing the truck (death blast when carrying ammunition, §6.10) | Crate icon | Park 600 cm behind mortars |

**Research and upgrades:** `tech.shared.supply_preservation`: resupply cost −25 %.

**Counters:** *Beats:* nothing. *Beaten by (hard):* any AT weapon, autocannon, scouts with MGs. *Situational:* cheapest way to make mass infantry mobile.

**Status:** identity E-R; transport and resupply rules P.

### RD-ARM-03 — Tracked Personnel Carrier

| Field | Specification |
| --- | --- |
| **Identity** | `unit.rd.arm_03` · Red Directorate · Vehicle / tracked APC · `TPL-VEH-IFV` · **Core** · Playable |
| **Role** | **Primary:** heavy infantry transport for mass formations (E-R). **Secondary:** fire support with a heavy machine gun; carrier for RD-INF-02. **Doctrine fit:** mechanized infantry riding with tanks (E). |
| **Inspiration and background** | Tracked armored personnel carrier with a roof HMG turret. C&C lineage (E-R): APC (TD); Armadillo (TT); Amphibious APC (TS, role). |
| **Visual identity** | none (asset requirement). Low tracked hull, roof cupola HMG, rear doors, red star-in-gear stencil. |
| **Statistics** | **560 HP** (`armor.veh_light`) · TRACKED 350 cm/s · F2 · sight 1200 cm · **5 infantry slots**, 2 fire ports · crew 2 · food 2 · fuel `medium` · ExperienceValue 70 |
| **Commands** | as TPL-VEH-IFV · default stance Guard · formation role `frontline` |
| **Production** | **$700** · `RD-BLD-05` · **13 s** · 2 food · 3 petroleum · no cap |
| **Veterancy** | Standard |
| **AI** | `ai.unit.ifv` |
| **Multiplayer** | MPA-STD + MPA-TRANSPORT |
| **Presentation** | ANIM-VEH-TRACK + rear ramp. Icon: tracked box with figures |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.rd.arm_03.hmg` roof 14.5 mm-class HMG | AUTOCANNON | 0–750 cm | 14 / 0.4 s (35 DPS) | hitscan | 4 | GROUND, AIR_LOW | 65 % / 85 % / 10 % / 100 % rotary / 30 % |

**Abilities:** shared *Load/Unload* with fire ports (§10.1). RD-INF-02 loads twice as fast (its *Mounted Drill*).

**Research and upgrades:** `tech.shared.vehicle_component_standardization`: repair −20 %. `tech.rd.applique_armor` (RD-BLD-05, $500): HP +80 (all RD-ARM-03/04/15); trade-off: speed −5 %.

**Counters:** *Beats (hard):* infantry in the open, light trucks, helicopters at short range. *Beaten by (hard):* AT infantry (AC-INF-03, IA-INF-04, MP-INF-03), MBTs, ATGM carriers (AC-ARM-06). *Situational:* its value is the 5 slots: one carrier moves a whole platoon block.

**Status:** identity E-R; values P.

### RD-ARM-04 — People's Battle Tank

| Field | Specification |
| --- | --- |
| **Identity** | `unit.rd.arm_04` · Red Directorate · Vehicle / main battle tank · `TPL-VEH-MBT` · **Core** · Playable |
| **Role** | **Primary:** affordable mass battle tank; supports broad armor fronts (E-R). **Secondary:** infantry support with HE rounds. **Doctrine fit:** heavily armored divisions (E). |
| **Inspiration and background** | Late-1970s Soviet-style MBT with an autoloader and a 125 mm-class smoothbore gun. C&C lineage (E-R): Heavy Tank (RA1); Rhino Heavy Tank (RA2); Battlemaster Tank (Gen). |
| **Visual identity** | none (asset requirement). Low dome turret, long gun, side skirts, dark green with red star-in-gear stencil. |
| **Statistics** | **880 HP** (`armor.veh_heavy`) · TRACKED 315 cm/s · F3 · sight 1250 cm · crew 3 · food 3 · fuel `heavy` · ExperienceValue 105 |
| **Commands** | as TPL-VEH-MBT · default stance Aggressive · formation role `frontline` |
| **Production** | **$1,050** (band lower-middle; AC-ARM-04 sits at the upper edge) · `RD-BLD-05` · **19 s** · 3 food · 7 petroleum (5 + 40 %) · no cap |
| **Veterancy** | Standard |
| **AI** | `ai.unit.battle_tank`. Deltas: never engage alone; wait for 2 more tanks or an IFV within 800 cm |
| **Multiplayer** | MPA-STD |
| **Presentation** | ANIM-VEH-TRACK + recoil, autoloader clank SFX. Icon: tank with a star |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.rd.arm_04.ap` 125 mm-class gun, AP | KINETIC_AP | 0–880 cm | 85 / 3.6 s (23.6 DPS) | shell_direct 3000 cm/s, tracking medium | — | GROUND, STRUCTURE | 20 % / 90 % / 100 % / — / 45 % hardened |
| `weapon.rd.arm_04.he` same gun, HE (auto-selected against infantry and soft structures) | HIGH_EXPLOSIVE | 0–880 cm | 60 / 3.6 s; full 60, max 140 cm, edge 40 % | shell_direct | 8 | GROUND, STRUCTURE | 110 % / 45 % / 12 % / — / 100 % soft |
| `weapon.rd.arm_04.coax` coaxial MG | SMALL_ARMS | 0–650 cm | 9 / 0.3 s | hitscan | 2 | GROUND | 100 % / 15 % / 2 % / — / 8 % |

**Abilities:** no unit-specific ability. Shell selection between AP and HE is automatic per target class (no micro chore).

**Research and upgrades:** `tech.rd.applique_armor`; `tech.shared.vehicle_component_standardization`; `tech.rd.standardized_hull_production` (RD-BLD-05, $900): build time 19 → 16 s for RD-ARM-03/04/15; trade-off: the Combine lane is blocked for 60 s.

**Counters:** *Beats (hard):* IFVs, APCs and light armor. *Beats (soft):* equal-cost Concord tanks by count (≈ 1.2 tanks per AC-ARM-04 for the same USD). *Beaten by (hard):* ATGM carriers (AC-ARM-06, IA-ARM-09), attack helicopters (AC-AIR-04, IA-AIR-03), tank destroyers. *Situational:* an AC-ARM-04 wins 1-v-1; RD wins on frontage and replacement speed.

**Status:** identity E-R; values P.

### RD-ARM-05 — Improved Heavy Battle Tank

| Field | Specification |
| --- | --- |
| **Identity** | `unit.rd.arm_05` · Red Directorate · Vehicle / heavy tank · `TPL-VEH-HEAVY` · **Extended** · Playable |
| **Role** | **Primary:** late conventional assault armor, **durable but fuel hungry** (E-R). **Secondary:** breakthrough spearhead against fortified lines. **Doctrine fit:** concentrated firepower at the decisive point. |
| **Inspiration and background** | Heavy-armored late-1980s MBT with reactive armor blocks. C&C lineage (E-R): Apocalypse Tank (RA2); Overlord Tank (Gen); Mammoth Tank (TD). |
| **Visual identity** | none (asset requirement). Wide turret covered in reactive-armor blocks, long gun, heavy skirts. |
| **Statistics** | **1,450 HP** (`armor.veh_heavy`) · TRACKED_HEAVY 245 cm/s · F3 · sight 1300 cm · crew 4 · food 4 · fuel `heavy` · ExperienceValue 200 |
| **Commands** | as TPL-VEH-HEAVY · default stance Aggressive · formation role `frontline` |
| **Production** | **$2,000** · `RD-BLD-06` Heavy Tank Assembly Hall · prerequisites: `RD-BLD-17` · **32 s** · 4 food · **10 petroleum** (fuel hungry: 5 × 2.0) · no cap |
| **Veterancy** | Standard |
| **AI** | `ai.unit.heavy_armor` |
| **Multiplayer** | MPA-STD |
| **Presentation** | ANIM-VEH-TRACK + reactive-block detonation VFX on SHAPED_CHARGE hits. Icon: heavy tank with two stars |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.rd.arm_05.ap` 125 mm-class gun, AP | KINETIC_AP | 0–950 cm | 110 / 3.8 s | shell_direct 3000 cm/s, tracking medium | — | GROUND, STRUCTURE | 20 % / 90 % / 100 % / — / 45 % |
| `weapon.rd.arm_05.he` same gun, HE | HIGH_EXPLOSIVE | 0–950 cm | 75 / 3.8 s; full 70, max 160 cm | shell_direct | 10 | GROUND, STRUCTURE | 110 % / 45 % / 12 % / — / 100 % / 60 % hardened |
| `weapon.rd.arm_05.aa_mg` cupola 12.7 mm MG | SMALL_ARMS | 0–700 cm | 12 / 0.3 s | hitscan | 3 | GROUND, AIR_LOW | 100 % / 15 % / 2 % / 25 % rotary / 8 % |

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.rd.arm_05.reactive_armor` | Passive charges (P) | The first 3 SHAPED_CHARGE hits each deal −50 % damage; charges restore at RD-BLD-17 or near RD-SUP-04 (1 per 20 s) | Self | 3 charges | KINETIC_AP ignores it; sustained ATGM fire strips it | Block-pop VFX; charge pips | — |
| `ability.rd.arm_05.fuel_hungry` | Conditional passive (P; E-R) | During fuel starvation (§4.5) its speed penalty is −40 % instead of −25 % | Self | — | Interdicting RD-SUP-03 and RD-BLD-09 | Fuel icon | Hold near depots when petroleum < 20 |

**Research and upgrades:** `tech.rd.applique_armor` does not apply (already armored). `tech.rd.engine_efficiency_program` (RD-BLD-02, $900): production petroleum 10 → 8; trade-off: speed −5 %.

**Counters:** *Beats (hard):* MBTs, bunkers and casemates, IFVs. *Beaten by (hard):* ATGM air (AC-AIR-04 with sustained fire), heavy TD fire (IA-ARM-09), precision artillery (AC-ARM-08), fuel interdiction. *Situational:* slow; needs RD-ARM-06/13 escort against air.

**Status:** identity and "fuel hungry" E-R; values P.

### RD-ARM-06 — Mobile Anti-Air Gun

| Field | Specification |
| --- | --- |
| **Identity** | `unit.rd.arm_06` · Red Directorate · Vehicle / mobile air defense (gun) · `TPL-VEH-AA` · **Core** · Playable |
| **Role** | **Primary:** tracked anti-air cannon supporting columns (E-R). **Secondary:** shreds infantry and light vehicles. **Doctrine fit:** integrated air defense moving with the army (E). |
| **Inspiration and background** | Quad 23 mm-class radar-directed self-propelled AA gun. C&C lineage (E-R): Gattling Tank (Gen); Flak Track (RA2); Slingshot (KW). |
| **Visual identity** | none (asset requirement). Flat turret with four barrels and a dish radar. |
| **Statistics** | **560 HP** (`armor.veh_medium`) · TRACKED 340 cm/s · F2 · sight 1300 cm / **3000 cm air** · `DETECT.AIR_RADAR` 3000 cm · crew 3 · food 3 · fuel `medium` · ExperienceValue 90 |
| **Commands** | as TPL-VEH-AA · default stance Aggressive (air) · formation role `escort` |
| **Production** | **$900** · `RD-BLD-05` · **15 s** · 3 food · 3 petroleum · no cap |
| **Veterancy** | Standard |
| **AI** | `ai.unit.air_defense`. Deltas: engage ground infantry at ≥ 15 % only when no air contact is within 3000 cm |
| **Multiplayer** | MPA-STD + MPA-SENSOR (radar footprint is owner-only) |
| **Presentation** | ANIM-VEH-TRACK + radar spin; flak tracer streams. Icon: four barrels |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.rd.arm_06.quad` quad 23 mm-class autocannon | AUTOCANNON | 0–1000 cm | 12 × 4 / 0.6 s (80 DPS) | hitscan | 5 | GROUND, AIR_LOW, AIR_HIGH | 65 % / 85 % / 10 % / 100 % rotary, 60 % fixed / 30 % |

**Abilities:** no unit-specific ability. Its radar is a `DETECT.EMITTER` contact to enemy EW (§9.2), which is its tell.

**Research and upgrades:** `tech.shared.radar_signal_processing`: air sight 3000 → 3400 cm.

**Counters:** *Beats (hard):* helicopters (AC-AIR-04/09, IA-AIR-03, MP-AIR-02), infantry. *Beats (soft):* low-flying jets. *Beaten by (hard):* MBTs, ATGMs, artillery; jamming (AC-ARM-10) blanks its radar contacts but not visual targeting. *Situational:* its gun is short-ranged against stand-off jets; pair with RD-ARM-13.

**Status:** identity E-R; values P.

### RD-ARM-07 — Rocket Artillery Regiment

| Field | Specification |
| --- | --- |
| **Identity** | `unit.rd.arm_07` · Red Directorate · Vehicle / multiple rocket launcher · `TPL-VEH-ARTY` · **Core** · Playable · faction signature |
| **Role** | **Primary:** wide-area **unguided** barrage from mobile launchers (E-R). **Secondary:** saturate infantry, soft bases and AA sites. **Doctrine fit:** extensive artillery formations (E). |
| **Inspiration and background** | Truck-mounted 40-tube 122 mm-class rocket launcher. C&C lineage (E-R): V2 Rocket Launcher (RA1); V3 Rocket Launcher (RA2); V4 Rocket Launcher (RA3). |
| **Visual identity** | none (asset requirement). Six-wheeled truck with a tube box that elevates; dust cloud on launch. |
| **Statistics** | **420 HP** (`armor.veh_light`) · WHEEL_HEAVY 310 cm/s · F2 · sight 1100 cm · crew 3 · food 3 · fuel `medium` · ExperienceValue 120 |
| **Commands** | as TPL-VEH-ARTY + Fire Mission, Deploy · default stance Hold Ground · formation role `support_weapon` |
| **Production** | **$1,200** · `RD-BLD-05` · prerequisites: `RD-DEF-06` · **20 s** · 3 food · 3 petroleum · no cap |
| **Veterancy** | Standard |
| **AI** | `ai.unit.artillery`. Deltas: fire full salvoes only on clusters of ≥ 4 units or on structures |
| **Multiplayer** | MPA-INDIRECT |
| **Presentation** | ANIM-VEH-WHEEL + launcher elevation, ripple-fire with long smoke trails (a large launch tell). Icon: tubes |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.rd.arm_07.salvo` 122 mm-class rockets | HIGH_EXPLOSIVE | **900–3800 cm** | 16 rockets × 40 per salvo over 3 s; AmmoCapacity 2 salvoes; reload from logistics | ballistic; each rocket full 60, max 150, edge 30 %; scatter 150–450 cm | 10 | GROUND, STRUCTURE | 110 % / 45 % / 12 % / — / 100 % |

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.shared.deploy` | Active (P) | Must deploy (3 s) to fire; undeploy 3 s | Self | free | Raiders during the window | Stabilizer jacks | Deploy 2800–3400 cm from the target area |
| `ability.rd.arm_07.coordinated_barrage` | Active (P) | When ≥ 3 RD-ARM-07 fire at aim points within 600 cm of each other inside 2 s, every rocket in the combined salvo applies +50 % suppression stress | Fire Mission | free · instant · per salvo | Dispersing; counter-battery (AC-ARM-12, `DETECT.ARTILLERY`) | Shared salvo marker | Group launchers in batteries of 3 |

**Research and upgrades:** `tech.rd.coordinated_defensive_fire`; `tech.rd.extended_rocket_motors` (RD-BLD-02, $800): range 3800 → 4200 cm; trade-off: scatter max 450 → 520 cm.

**Counters:** *Beats (hard):* massed infantry, soft structures, deployed HW teams, SAM sites. *Beaten by (hard):* counter-battery (AC-ARM-12 with AC-ARM-07/08), strike aircraft (AC-AIR-02, IA-AIR-02), raiders (MP-ARM-01/07). *Situational:* poor against moving tanks (scatter); its launch smoke reveals it.

**Status:** identity and "unguided" E-R; values P.

### RD-ARM-08 — Heavy Self-Propelled Gun

| Field | Specification |
| --- | --- |
| **Identity** | `unit.rd.arm_08` · Red Directorate · Vehicle / heavy artillery · `TPL-VEH-ARTY` · **Extended** · Playable |
| **Role** | **Primary:** long-range counter-battery and siege pressure (E-R). **Secondary:** precision-ish destruction of hardened structures. **Doctrine fit:** operational-depth fire (E). |
| **Inspiration and background** | 203 mm-class tracked self-propelled gun. C&C lineage (E-R): Artillery (TS); Nuke Cannon (Gen, conventional role only); Juggernaut (TW). |
| **Visual identity** | none (asset requirement). Open-mounted giant barrel on a tracked chassis with a rear spade. |
| **Statistics** | **520 HP** (`armor.veh_light`) · TRACKED 270 cm/s · F3 · sight 1100 cm · crew 4 · food 4 · fuel `medium` · ExperienceValue 160 |
| **Commands** | as TPL-VEH-ARTY · default stance Hold Ground · formation role `support_weapon` |
| **Production** | **$1,600** · `RD-BLD-06` · prerequisites: `RD-DEF-06` · **26 s** · 4 food · 3 petroleum · no cap |
| **Veterancy** | Standard |
| **AI** | `ai.unit.artillery`. Deltas: prefer `DETECT.ARTILLERY` contacts and observed artillery as targets |
| **Multiplayer** | MPA-INDIRECT |
| **Presentation** | ANIM-VEH-TRACK + spade drop, huge muzzle blast. Icon: long barrel |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.rd.arm_08.shell` 203 mm-class shell | HIGH_EXPLOSIVE | **1000–4200 cm** | 220 / 8.0 s; AmmoCapacity 20 | ballistic; full 100, max 220, edge 35 %; scatter 100–260 cm | 12 | GROUND, STRUCTURE | 110 % / 45 % / 12 % / — / 100 % / 60 % hardened |

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.shared.deploy` | Active (P) | Must deploy (5 s) to fire | Self | free | Raids | Spade | as RD-ARM-07 |
| `ability.rd.arm_08.counter_battery_fire` | Conditional (P) | Against a `DETECT.ARTILLERY` contact or observed artillery: scatter ×0.6 | Automatic | — | Displacement after firing (enemy AI displaces after 3 salvoes) | Crosshair marker | Answer every enemy battery contact |

**Research and upgrades:** `tech.rd.coordinated_defensive_fire`.

**Counters:** *Beats (hard):* artillery (AC-ARM-07, IA-ARM-10/11, MP-ARM-08/12), bunkers, static defenses. *Beaten by (hard):* aircraft, raids, any direct fire. *Situational:* needs RD-INF-09, RD-AIR-08 or RD-AIR-06 observation.

**Status:** identity E-R; values P.

### RD-ARM-09 — Mortar Carrier

| Field | Specification |
| --- | --- |
| **Identity** | `unit.rd.arm_09` · Red Directorate · Vehicle / mobile mortar · `TPL-VEH-ARTY` · **Extended** · Playable |
| **Role** | **Primary:** mobile short-range indirect fire supporting infantry (E-R). **Secondary:** smoke and saturation of trench lines. **Doctrine fit:** artillery that keeps pace with the motor-rifle advance. |
| **Inspiration and background** | APC hull with a roof-hatch 120 mm-class mortar. C&C lineage (E-R): Mortar Cycle (Uprising); Inferno Cannon (Gen, saturation role). |
| **Visual identity** | none (asset requirement). RD-ARM-03 hull with open roof hatches and a mortar tube. |
| **Statistics** | **520 HP** (`armor.veh_light`) · TRACKED 340 cm/s · F2 · sight 1100 cm · crew 3 · food 3 · fuel `medium` · ExperienceValue 90 |
| **Commands** | as TPL-VEH-ARTY (no deploy) · default stance Guard · formation role `support_weapon` |
| **Production** | **$900** · `RD-BLD-05` · **16 s** · 3 food · 3 petroleum · no cap |
| **Veterancy** | Standard |
| **AI** | `ai.unit.artillery` |
| **Multiplayer** | MPA-INDIRECT |
| **Presentation** | ANIM-VEH-TRACK + roof hatches. Icon: mortar on tracks |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.rd.arm_09.mortar` 120 mm-class mortar | HIGH_EXPLOSIVE | 500–2400 cm | 80 / 4.5 s; AmmoCapacity 40; fires 2 s after stopping, no deploy | ballistic; full 80, max 180, edge 35 %; scatter 80–240 | 9 | GROUND, STRUCTURE | 110 % / 45 % / 12 % / — / 100 % |

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.rd.arm_09.smoke_barrage` | Active (P) | Three §9.5 smoke cylinders in a 600 cm line | Ground point in range | 3 ammo · 20 s · 40 s · 500–2400 cm | Detectors; wind is not modeled | Smoke | Screen the advance of tank columns |

**Research and upgrades:** `tech.shared.supply_preservation`.

**Counters:** *Beats (hard):* infantry in cover, HW teams. *Beaten by (hard):* MBTs, AT infantry at short range. *Situational:* shoots and scoots without deploy time, unlike RD-INF-05.

**Status:** identity E-R; values P.

### RD-ARM-10 — Flame Assault Vehicle

| Field | Specification |
| --- | --- |
| **Identity** | `unit.rd.arm_10` · Red Directorate · Vehicle / flame assault · `TPL-VEH-HEAVY` (assault subset) · **Extended** · Playable |
| **Role** | **Primary:** costly close-range attack on fortified positions; **abstract game weapon** (E-R). **Secondary:** clear garrisons. **Doctrine fit:** urban breakthrough. |
| **Inspiration and background** | Tank-hull flame vehicle with an armored fuel cell. No real weapon detail (E-R). C&C lineage (E-R): Flame Tank (TD); Dragon Tank (Gen); Devil's Tongue (TS). |
| **Visual identity** | none (asset requirement). Tank hull, short projector, rear armored tank; scorch decals. |
| **Statistics** | **900 HP** (`armor.veh_medium`) · TRACKED 300 cm/s · F3 · sight 1100 cm · crew 2 · food 2 · fuel `heavy` · ExperienceValue 130 |
| **Commands** | as TPL-VEH-MBT · default stance Aggressive · formation role `frontline` |
| **Production** | **$1,300** · `RD-BLD-05` · prerequisites: `RD-BLD-17` · **20 s** · 2 food · 7 petroleum · no cap |
| **Veterancy** | Standard |
| **AI** | `ai.unit.heavy_armor`. Deltas: target garrisons and bunkers first; never engage tanks unless ordered |
| **Multiplayer** | MPA-STD |
| **Presentation** | ANIM-VEH-TRACK + stream VFX. Death blast (fuel-laden, §6.10). Icon: flame |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.rd.arm_10.flame` projector (abstract) | INCENDIARY | 0–450 cm | 30 / 0.5 s (60 DPS); stream; 100 cm splash; applies `burning` 4 s | stream | 10 | GROUND, STRUCTURE | 160 % / 25 % / 5 % / — / 80 % soft, 40 % hardened; garrison ×2 |

**Abilities:** no unit-specific ability; garrison ×2 INCENDIARY is a framework rule (§6.9).

**Research and upgrades:** `tech.rd.applique_armor` (does not apply: medium armor class); none proposed.

**Counters:** *Beats (hard):* garrisoned infantry, bunkers and other STR_HARDENED infantry positions, infantry in trenches. *Beaten by (hard):* tanks, ATGMs, AT infantry from outside 450 cm. *Situational:* its death blast punishes friendly clumping.

**Status:** identity and abstract-weapon rule E-R; values P.

### RD-ARM-11 — Mine-Clearing Tank

| Field | Specification |
| --- | --- |
| **Identity** | `unit.rd.arm_11` · Red Directorate · Vehicle / armored engineer · `TPL-VEH-ENGINEER` · **Extended** · Playable |
| **Role** | **Primary:** breach obstacle lines and secure the armored advance (E-R). **Secondary:** push wrecks out of lanes. **Doctrine fit:** engineering capabilities at the front (E). |
| **Inspiration and background** | Tank hull with a mine plough and a line-charge launcher (abstract). C&C lineage (E-R): Grinder (Uprising, obstacle/crush role); Dozer (TT); Terror Drone (RA2, clearance need). |
| **Visual identity** | none (asset requirement). MBT hull, no turret gun, front plough, rear launcher box. |
| **Statistics** | **1,000 HP** (`armor.veh_heavy`) · TRACKED 280 cm/s · F3 · sight 1200 cm · detects mines 600 cm · crew 2 · food 2 · fuel `heavy` · no veterancy |
| **Commands** | Move, Stop, Guard (I) · Clear, Breach (P) · default stance Hold Fire · formation role `frontline` |
| **Production** | **$1,000** · `RD-BLD-05` · prerequisites: `RD-BLD-04` · **18 s** · 2 food · 7 petroleum · no cap |
| **Veterancy** | none |
| **AI** | `ai.unit.engineer_vehicle` |
| **Multiplayer** | MPA-STD |
| **Presentation** | ANIM-VEH-TRACK + plough dust; mine-detonation VFX. Icon: plough |

**Weapons:** none (unarmed; the line charge is an ability).

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.rd.arm_11.plough` | Passive (P) | Detonates or clears mines it drives over and within 150 cm of its path; takes 10 % of the mine damage | Self | free | AT fire while it is slow | Plough sparks | Lead columns through suspected minefields |
| `ability.rd.arm_11.breach_line` | Active (P) | Clears all mines and obstacles (wire, dragon's teeth, MP-ARM-11 mines) in a 200 × 1200 cm lane ahead | Direction ≤ 1200 cm | $100 · 3 s · 60 s · 1200 cm | Covering fire on the vehicle during the 3 s | Lane outline | Use on the first observed obstacle line |

**Research and upgrades:** `tech.shared.vehicle_component_standardization`.

**Counters:** *Beats:* minefields (MP-ARM-11, RD-DEF-12 equivalents) and obstacle lines. *Beaten by (hard):* any anti-armor; it cannot fight back. *Situational:* decisive against Meridian mine play.

**Status:** identity E-R; values P.

### RD-ARM-12 — Defensive Entrenchment Tank

| Field | Specification |
| --- | --- |
| **Identity** | `unit.rd.arm_12` · Red Directorate · Vehicle / dug-in tank · `TPL-VEH-MBT` (deploy variant) · **Extended** · Playable |
| **Role** | **Primary:** can dig in for defense; **loses flexibility while deployed** (E-R). **Secondary:** hold bridges and rail junctions. **Doctrine fit:** layered defensive positions (E). |
| **Inspiration and background** | Medium tank fitted with a self-entrenching blade. C&C lineage (E-R): Tick Tank (TS); Spartan Tank (TT). |
| **Visual identity** | none (asset requirement). Medium hull with a front blade; deployed state sinks into an earth berm. |
| **Statistics** | **760 HP** (`armor.veh_heavy`) · TRACKED 300 cm/s · F3 · sight 1250 cm · crew 3 · food 3 · fuel `heavy` · ExperienceValue 90 |
| **Commands** | as TPL-VEH-MBT + Deploy · default stance Hold Ground · formation role `rear_guard` |
| **Production** | **$950** · `RD-BLD-05` · **18 s** · 3 food · 7 petroleum · no cap |
| **Veterancy** | Standard |
| **AI** | `ai.unit.battle_tank`. Deltas: dig in at chokepoints named by the player or AI defense plan |
| **Multiplayer** | MPA-STD |
| **Presentation** | ANIM-VEH-TRACK + dig loop; berm mesh in deployed state. Icon: tank in a berm |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.rd.arm_12.gun` 100 mm-class gun | KINETIC_AP | 0–850 cm (deployed 1000 cm) | 70 / 3.4 s | shell_direct, tracking medium | — | GROUND, STRUCTURE | 20 % / 90 % / 100 % / — / 45 % |
| `weapon.rd.arm_12.coax` coaxial MG | SMALL_ARMS | 0–650 cm | 9 / 0.3 s | hitscan | 2 | GROUND | 100 % / 15 % / 2 % / — / 8 % |

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.rd.arm_12.dig_in` | Active (P) | Deployed: armor counts as `armor.str_hardened` against HIGH_EXPLOSIVE and KINETIC_AP from the front 180° (−35 % damage), range +150 cm; immobile; turret 360° | Self | free · 6 s dig / 4 s undig | Flanking, artillery from above (normal damage), DEMOLITION | Berm | Undeploy when outflanked |

**Research and upgrades:** `tech.rd.coordinated_defensive_fire`: +100 cm range near RD-DEF-06 when dug in.

**Counters:** *Beats (hard):* frontal armor assaults, raiders. *Beaten by (hard):* artillery, aircraft, flanking TDs, saboteurs (stationary target). *Situational:* turns rail and bridge chokepoints into fortresses.

**Status:** identity and deploy trade-off E-R; values P.

### RD-ARM-13 — Field Anti-Air Missile Carrier

| Field | Specification |
| --- | --- |
| **Identity** | `unit.rd.arm_13` · Red Directorate · Vehicle / mobile SAM · `TPL-VEH-AA` · **Extended** · Playable |
| **Role** | **Primary:** long-range escort air defense **requiring radar support** (E-R). **Secondary:** area denial for jets and bombers. **Doctrine fit:** integrated air defense (E). |
| **Inspiration and background** | Tracked SAM launcher with three missiles; target acquisition from a radar vehicle or site. C&C lineage (E-R): SAM Site (TD, role vehicleized); Mantis (KW); Reaper (Uprising, anti-air role). |
| **Visual identity** | none (asset requirement). Tracked chassis with three rail-launched missiles; no dish. |
| **Statistics** | **520 HP** (`armor.veh_light`) · TRACKED 320 cm/s · F2 · sight 1300 cm (no own air radar) · crew 3 · food 3 · fuel `medium` · ExperienceValue 105 |
| **Commands** | as TPL-VEH-AA · default stance Aggressive (air) · formation role `escort` |
| **Production** | **$1,050** · `RD-BLD-05` · prerequisites: `RD-DEF-07` · **17 s** · 3 food · 3 petroleum · no cap |
| **Veterancy** | Standard |
| **AI** | `ai.unit.air_defense`. Deltas: stay within 2000 cm of an RD-ARM-06, RD-DEF-04, RD-DEF-07 or RD-AIR-06 |
| **Multiplayer** | MPA-STD + MPA-SENSOR |
| **Presentation** | ANIM-VEH-TRACK + launcher elevation; smoke-trail missiles. Icon: missile on tracks |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.rd.arm_13.sam` SAM | FLAK_AA | 300–**2000 cm** (only 900 cm without radar support) | 160 / 6.0 s; AmmoCapacity 6 | guided_missile (jammable) | — | AIR_LOW, AIR_HIGH | — / — / — / 110 % rotary, 100 % fixed / — |

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.rd.arm_13.radar_link` | Conditional passive (P; E-R) | Full 2000 cm range only while a friendly `DETECT.AIR_RADAR` source (RD-ARM-06, RD-DEF-04, RD-DEF-07, RD-AIR-06) has the target as an air contact | Self | free | Killing or jamming the radar source (AC-ARM-10, AC-AIR-07) | Link line (owner) | Hold position inside radar coverage |

**Research and upgrades:** `tech.shared.radar_signal_processing`; `tech.shared.electronic_countermeasures`: guided missile resists jamming within 600 cm of the launcher.

**Counters:** *Beats (hard):* jets and bombers (AC-AIR-02/03/08/11, IA-AIR-02/05/07, MP-AIR-03). *Beaten by (hard):* ground raids, artillery, SEAD-style strikes, jamming of the radar link. *Situational:* worthless alone; deadly in a network.

**Status:** identity and radar dependence E-R; values P.

### RD-ARM-14 — Armored Siege Command Tank

| Field | Specification |
| --- | --- |
| **Identity** | `unit.rd.arm_14` · Red Directorate · Vehicle / super-heavy command vehicle · `TPL-VEH-HEAVY` (super-heavy) · **Conditional** (prototype) · Playable · **cap 1** |
| **Role** | **Primary:** rare heavyweight centerpiece with **infantry support sockets** (E-R). **Secondary:** mobile support hub (repair, resupply, Discipline) for a mass front. **Doctrine fit:** experimental equipment that requires large industrial investment (E); a command vehicle, not a talent appointment (framework §13.5). |
| **Inspiration and background** | Alternate-history super-heavy tracked command and siege vehicle with two turrets and armored infantry bays. C&C lineage (E-R): MARV (KW); Behemoth (KW); Mastodon (TT). |
| **Visual identity** | none (asset requirement). Huge eight-track hull, main turret, secondary turret, four armored side bays with firing slits, command mast. Silhouette must stay plausible 1980s heavy engineering (roster rule 7). |
| **Statistics** | **2,600 HP** (`armor.veh_heavy`) · TRACKED_HEAVY 170 cm/s · F4 · sight 1400 cm · **4 infantry slots, all 4 are fire-port sockets** · crew 6 · food 6 · fuel `super` · ExperienceValue 400 |
| **Commands** | as TPL-VEH-HEAVY + Load/Unload · default stance Aggressive · formation role `frontline` (vehicles cannot hold the anchor preference, §13.4) |
| **Production** | **$4,200** · `RD-BLD-06` · prerequisites: `RD-BLD-17` and `tech.rd.siege_command_program` · **55 s** · 6 food · **14 petroleum** (10 + 40 %) · **cap 1** |
| **Veterancy** | Standard |
| **AI** | `ai.unit.heavy_armor`. Deltas: never advance without ≥ 2 AA units within 1200 cm; keep sockets filled with RD-INF-04 or RD-INF-03 |
| **Multiplayer** | MPA-STD + MPA-TRANSPORT |
| **Presentation** | ANIM-VEH-TRACK + two turrets, bay hatches; enemies see "occupied" bay lights only when observed. Icon: fortress on tracks |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.rd.arm_14.main` 152 mm-class gun | KINETIC_AP / HIGH_EXPLOSIVE (auto) | 0–1050 cm | AP 150 / 4.5 s; HE 120, full 100, max 200 cm | shell_direct | 12 (HE) | GROUND, STRUCTURE | AP: 20 / 90 / 100 / — / 45 %; HE: 110 / 45 / 12 / — / 100 % |
| `weapon.rd.arm_14.secondary` 30 mm-class autocannon turret | AUTOCANNON | 0–900 cm | 15 / 0.4 s | hitscan | 4 | GROUND, AIR_LOW | 65 % / 85 % / 10 % / 100 % rotary / 30 % |
| Socket fire | passengers' weapons | per passenger | fire-port rule: **no** −25 % penalty for sockets | per passenger | per passenger | per passenger | per passenger |

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.rd.arm_14.field_command_post` | Passive aura (P) | Friendly RD units within 1200 cm: resolve recovery +20 %; vehicles self-repair 3 HP/s out of combat (8 s rule); limited-ammo weapons Resupply aura as a logistics truck (4 targets) | Self aura 1200 cm | Resupply cost per §10.4 | Killing it; jamming cuts the aura through the field | Command mast pulse (owner) | Always on |
| `ability.rd.arm_14.siege_mode` | Active (P) | Deployed: main gun range 1050 → 1400 cm and HE vs STR_HARDENED 60 % → 80 %; immobile | Self | free · 8 s deploy / 6 s undeploy · 30 s | Aircraft; artillery; flanking ATGMs | Stabilizer legs | Deploy within 1400 cm of fortified lines |

**Research and upgrades:** `tech.rd.siege_command_program` (RD-BLD-02, $2,000, 120 s): unlocks the unit; trade-off: blocks the planning office lane. No further upgrades (anti-snowball).

**Counters:** *Beats (hard):* fortifications, MBT lines, infantry. *Beaten by (hard):* attack helicopters and strike jets (AC-AIR-02/04, IA-AIR-02/07), concentrated ATGMs (AC-ARM-06, IA-ARM-09), saboteur charges while sieged (MP-INF-08, AC-INF-11). *Situational:* lesson from the Apocalypse tank: slow, cap 1, air-vulnerable, fuel-starved by interdiction.

**Status:** identity, rarity and infantry sockets E-R; cap 1 P (framework §11.3); values P.

### RD-ARM-15 — Veteran Reserve Tank

| Field | Specification |
| --- | --- |
| **Identity** | `unit.rd.arm_15` · Red Directorate · Vehicle / older MBT · `TPL-VEH-MBT` · **Extended** · Playable |
| **Role** | **Primary:** older but **inexpensive refurbished armored hull** (E-R). **Secondary:** second-line defense, rail-route escorts, cheap fodder for mass fronts. **Doctrine fit:** standardized equipment kept in reserve (E). |
| **Inspiration and background** | Refurbished 1950s–60s medium tank with a 100 mm-class rifled gun. C&C lineage (E-R): Light Tank (TD); Grizzly Battle Tank (RA2); Scorpion Tank (TW). |
| **Visual identity** | none (asset requirement). Domed early-turret medium tank, faded paint, newer stencil. |
| **Statistics** | **700 HP** (`armor.veh_medium`) · TRACKED 320 cm/s · F3 · sight 1200 cm · crew 4 · food 4 · fuel `heavy` · ExperienceValue 60 |
| **Commands** | as TPL-VEH-MBT · default stance Aggressive · formation role `rear_guard` |
| **Production** | **$600** · `RD-BLD-05` or `RD-BLD-17` (refurbishment) · **14 s** · 4 food · 5 petroleum (no fuel-hungry surcharge: older, smaller engine) · no cap |
| **Veterancy** | Standard |
| **AI** | `ai.unit.battle_tank` |
| **Multiplayer** | MPA-STD |
| **Presentation** | ANIM-VEH-TRACK; visibly older model. Icon: tank with a wrench |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.rd.arm_15.gun` 100 mm-class gun | KINETIC_AP / HIGH_EXPLOSIVE (auto) | 0–820 cm | AP 60 / 3.8 s; HE 50, max 120 cm | shell_direct, tracking medium | 6 (HE) | GROUND, STRUCTURE | AP 20 / 90 / 100 / — / 45 %; HE 110 / 45 / 12 / — / 100 % |
| `weapon.rd.arm_15.coax` coaxial MG | SMALL_ARMS | 0–650 cm | 9 / 0.3 s | hitscan | 2 | GROUND | 100 % / 15 % / 2 % / — / 8 % |

**Abilities:** no unit-specific ability.

**Research and upgrades:** `tech.rd.applique_armor` (HP +80, −5 % speed); `tech.rd.standardized_hull_production`.

**Counters:** *Beats (hard):* light vehicles, APCs, infantry. *Beats (soft):* MP-ARM-05/06 at cost parity. *Beaten by (hard):* every modern MBT one-on-one, ATGMs, AT infantry. *Situational:* the cheapest hull per HP in the faction; lets RD-ARM-04 production go to the front.

**Status:** identity E-R; values P.

## 5. Construction, supply and field support

### RD-SUP-01 — Industrial Constructor

| Field | Specification |
| --- | --- |
| **Identity** | `unit.rd.sup_01` · Red Directorate · Vehicle / constructor · `TPL-VEH-CONSTRUCTOR` · **Core** · Playable |
| **Role** | **Primary:** mobile deployment of construction and heavy production works (E-R). **Secondary:** expansion toward rail junctions and resource sites. **Doctrine fit:** large-scale industrial construction (E: *Gameplay*). |
| **Inspiration and background** | Heavy tracked construction vehicle carrying a prefabricated works module and crane. C&C lineage (E-R): MCV (TD); Construction Dozer (Gen); Sputnik (RA3). |
| **Visual identity** | none (asset requirement). Wide tracked chassis, folded crane, prefabricated panels, red planning-office banner. |
| **Statistics** | **900 HP** (`armor.veh_light`) · TRACKED_HEAVY 230 cm/s · F4 · sight 1100 cm · crew 3 · food 3 · fuel `heavy` · no veterancy |
| **Commands** | Move, Stop, Guard (I) · Deploy (to RD-BLD-01 class works), Build (P) · default stance Hold Fire · formation role `protected` |
| **Production** | **$2,200** · `RD-BLD-01` or `RD-BLD-05` · **35 s** · 3 food · 7 petroleum · no cap (each additional deployed HQ is subject to the construction owner's rules) |
| **Veterancy** | none |
| **AI** | `ai.unit.constructor`. Deltas: deploy within 1500 cm of an authored rail node when one is reachable |
| **Multiplayer** | MPA-STD |
| **Presentation** | ANIM-VEH-TRACK + unfold sequence (8 s). Icon: crane |

**Weapons:** none (unarmed).

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.rd.sup_01.deploy_works` | Active (P) | Unfolds into a forward construction authority (an RD-BLD-01-class build radius of 2500 cm); undeploy allowed | Legal flat ground; Unaligned jurisdiction permission where required (E) | free · 8 s deploy · — | Attack during unfolding (takes +25 % damage) | Footprint preview | Expand to rail nodes and resource sites |

**Research and upgrades:** `tech.rd.industrial_mobilization_methods`: build speed of structures placed by it +10 %.

**Counters:** *Beaten by (hard):* everything armed. *Situational:* losing it costs a long rebuild; escort with RD-ARM-06 and RD-INF-01.

**Status:** identity E-R; construction ownership E (construction docs); values P.

### RD-SUP-02 — Rail Supply Tractor

| Field | Specification |
| --- | --- |
| **Identity** | `unit.rd.sup_02` · Red Directorate · Vehicle / logistics · `TPL-VEH-LOGISTICS` · **Extended** · Playable |
| **Role** | **Primary:** carry finished goods between rail and forward depots (E-R). **Secondary:** ammunition resupply of artillery parks. **Doctrine fit:** railways connect production to units in the field (E). |
| **Inspiration and background** | Heavy wheeled tractor with a flatbed trailer of crates. C&C lineage (E-R): Supply Truck (Gen); Ore Truck (RA1); Heavy Harvester (KW). |
| **Visual identity** | none (asset requirement). Six-wheel tractor, flatbed trailer, tarpaulin over crates. |
| **Statistics** | **480 HP** (`armor.veh_soft`) · WHEEL_HEAVY 360 cm/s · F2 · sight 1000 cm · cargo 40 ammunition units or 1 delivery package · crew 1 · food 1 · fuel `light` · no veterancy |
| **Commands** | Move, Stop, Guard (I) · Deliver, Resupply (P) · default stance Hold Fire · formation role `protected` |
| **Production** | **$450** · `RD-BLD-11` Railway Marshalling Depot · **10 s** · 1 food · 1 petroleum · no cap |
| **Veterancy** | none |
| **AI** | `ai.unit.logistics` |
| **Multiplayer** | MPA-STD + MPA-ECON (cargo amount owner-only; cargo type visible: crates) |
| **Presentation** | ANIM-VEH-WHEEL; crate stack shrinks as cargo empties. Icon: crates on rails |

**Weapons:** none (unarmed).

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.rd.sup_02.rail_shuttle` | Automated (P) | Loads at RD-BLD-11 (8 s) and unloads at RD-DEF-11 or a forward depot, raising that depot's stored ammunition; while within 400 cm of limited-ammo units it provides the Resupply aura (§10.4) | Assigned route | resupply per §10.4 | Route interdiction; death blast while loaded (§6.10) | Route line (owner) | Reroute around observed threats |

**Research and upgrades:** `tech.shared.supply_preservation`; `tech.rd.rail_scheduling` (RD-BLD-11, $500): load time 8 → 5 s; trade-off: RD-BLD-11 power draw +10 %.

**Counters:** *Beaten by (hard):* raiders (MP-ARM-01/02/07), helicopters, saboteurs. *Situational:* the rail artery; cutting it slows every RD artillery park.

**Status:** identity E-R; logistics dependence E; values P.

### RD-SUP-03 — Bulk Fuel Transport

| Field | Specification |
| --- | --- |
| **Identity** | `unit.rd.sup_03` · Red Directorate · Vehicle / fuel logistics · `TPL-VEH-LOGISTICS` · **Extended** · Playable |
| **Role** | **Primary:** sustain mechanized divisions with refined fuel (E-R). **Secondary:** relieve fuel starvation at forward depots. **Doctrine fit:** heavy formations consume substantial fuel (E). |
| **Inspiration and background** | Army tanker lorry. C&C lineage (E-R): Supply Truck (Gen); Harvester (TW); Ore Collector (RA3). |
| **Visual identity** | none (asset requirement). Cylindrical tanker body with hazard stencil. |
| **Statistics** | **420 HP** (`armor.veh_soft`) · WHEEL_HEAVY 360 cm/s · F2 · sight 1000 cm · cargo 30 petroleum **or** 30 kerosene (one type per trip, never mixed, E) · crew 1 · food 1 · fuel `light` · no veterancy |
| **Commands** | Move, Stop (I) · Deliver (P) · default stance Hold Fire · formation role `protected` |
| **Production** | **$420** · `RD-BLD-11` or `RD-BLD-09` · **10 s** · 1 food · 1 petroleum · no cap |
| **Veterancy** | none |
| **AI** | `ai.unit.logistics`. Deltas: prioritize delivery when the participant's petroleum stock < 20 % of storage |
| **Multiplayer** | MPA-ECON: cargo type visible, amount owner-only; delivery credited once at arrival (anti double-delivery) |
| **Presentation** | ANIM-VEH-WHEEL. Large death blast (§6.10). Icon: fuel drop |

**Weapons:** none (unarmed).

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.rd.sup_03.deliver_fuel` | Automated (P) | Moves refined fuel from RD-BLD-09 or RD-BLD-11 into the participant stock at a forward RD-DEF-11 or RD-BLD-07 (kerosene). Arrival credits the stock once; destruction loses the cargo | Assigned route | free | Interdiction; death blast | Fuel gauge (owner) | Convoy with RD-ARM-06 when air threats are observed |

**Research and upgrades:** `tech.shared.supply_preservation`: cargo +20 %; trade-off: HP −10 %.

**Counters:** *Beaten by (hard):* any attack; raiders and strike aircraft hunt it. *Situational:* keeps RD-ARM-05 out of the fuel-hungry penalty.

**Status:** identity E-R; fuel separation E; values P.

### RD-SUP-04 — Mobile Repair Crew

| Field | Specification |
| --- | --- |
| **Identity** | `unit.rd.sup_04` · Red Directorate · Vehicle / repair · `TPL-VEH-LOGISTICS` (repair subset) · **Core** · Playable |
| **Role** | **Primary:** repair heavy armor and rail-connected structures (E-R). **Secondary:** wreck salvage. **Doctrine fit:** large repair organizations make replacement efficient (E). |
| **Inspiration and background** | Armored recovery and repair vehicle with a crane on an APC hull. C&C lineage (E-R): Mobile Repair Vehicle (TS); MRT (KW); Spanner (TT). |
| **Visual identity** | none (asset requirement). RD-ARM-03 hull with a crane, toolboxes and a spare roadwheel. |
| **Statistics** | **600 HP** (`armor.veh_light`) · TRACKED 330 cm/s · F2 · sight 1000 cm · crew 3 · food 3 · fuel `medium` · no veterancy |
| **Commands** | Move, Stop, Guard (I) · Repair, Salvage (P) · default stance Hold Fire · formation role `protected` |
| **Production** | **$700** · `RD-BLD-05` or `RD-BLD-17` · **12 s** · 3 food · 3 petroleum · no cap |
| **Veterancy** | none |
| **AI** | `ai.unit.logistics`. Deltas: repair the highest-value damaged vehicle within 600 cm under Guard |
| **Multiplayer** | MPA-STD |
| **Presentation** | ANIM-VEH-TRACK + crane loop; welding sparks. Icon: wrench on tracks |

**Weapons:** none (unarmed).

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.shared.field_repair` (vehicle rate) | Active / automated (P) | 20 HP/s to one friendly vehicle; **28 HP/s to RD-ARM-04/05/14/15** (standardized parts); 12 HP/s to rail-connected structures and bridges | Target ≤ 300 cm | §10.2 pricing (25 % of cost for a full repair) | Suppression of the crew is not possible; kill it | Sparks | Auto under Guard |
| `ability.shared.salvage_wreck` | Active (P) | §10.5 salvage: 10 % of wreck cost in 6 s | Wreck ≤ 300 cm | free · 6 s | Fire on the wreck area | Crane hook | Salvage after the fight |

**Research and upgrades:** `tech.shared.vehicle_component_standardization` (repair USD −20 %); `tech.rd.repair_capacity` (RD-BLD-17, $600): repair rates +20 %; trade-off: price +$80.

**Counters:** *Beaten by (hard):* anything armed, artillery. *Situational:* doubles the staying power of a tank front; kill it first.

**Status:** identity E-R; repair rule P (framework §10.2); battlefield recovery (§10.5, U-11) not granted: the Directorate replaces rather than recovers (P).

### RD-SUP-05 — Engineer Bridging Vehicle

| Field | Specification |
| --- | --- |
| **Identity** | `unit.rd.sup_05` · Red Directorate · Vehicle / bridge layer · `TPL-VEH-ENGINEER` · **Extended** · Playable |
| **Role** | **Primary:** temporary crossings and mountain-pass restoration (E-R). **Secondary:** repair destroyed authored bridges faster than pioneers. **Doctrine fit:** engineering troops establish crossings (E). |
| **Inspiration and background** | Tank hull with a folding scissors bridge. C&C lineage (E-R): Amphibious APC (TS); Rig (TW); Combat Engineer (KW). |
| **Visual identity** | none (asset requirement). MBT hull with a folded bridge on top. |
| **Statistics** | **850 HP** (`armor.veh_heavy`) · TRACKED 280 cm/s · F3 · sight 1100 cm · crew 2 · food 2 · fuel `heavy` · no veterancy |
| **Commands** | Move, Stop (I) · Lay Bridge, Recover Bridge, Restore Crossing (P) · default stance Hold Fire · formation role `protected` |
| **Production** | **$1,000** · `RD-BLD-04` · **18 s** · 2 food · 7 petroleum · cap 3 |
| **Veterancy** | none |
| **AI** | `ai.unit.engineer_vehicle` |
| **Multiplayer** | MPA-STD; a laid bridge is a replicated world object visible to all observers of the area |
| **Presentation** | ANIM-VEH-TRACK + bridge-launch sequence (unique animation). Icon: bridge |

**Weapons:** none (unarmed).

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.rd.sup_05.lay_bridge` | Active (P) | Lays a 1600 cm, 800 HP temporary bridge across an authored gap (river ≤ 1600 cm, ravine) usable by all ground units (including enemies). The vehicle stays as the anchor; if destroyed, the bridge collapses | Authored gap edge; Unaligned waterways need jurisdiction permission (E) | free · 10 s · 30 s · contact | Artillery on the bridge; kill the anchor | Bridge mesh | Lay at the chosen crossing; recover after the army passes |
| `ability.rd.sup_05.restore_crossing` | Active (P) | Restores a destroyed authored bridge or pass segment in 10 s (pioneers take 20 s) | ≤ 200 cm | $150 · 10 s | as RD-INF-06 | Scaffold | Rebuild the army route |

**Research and upgrades:** `tech.rd.heavy_bridging` ($600): span 1600 → 2200 cm; trade-off: deploy 10 → 14 s.

**Counters:** *Beaten by (hard):* any anti-armor. *Situational:* lets a heavy army choose its crossing instead of fighting for authored bridges.

**Status:** identity E-R; jurisdiction E; values P.

### RD-SUP-06 — Electronic Countermeasure Vehicle

| Field | Specification |
| --- | --- |
| **Identity** | `unit.rd.sup_06` · Red Directorate · Vehicle / EW · `TPL-VEH-EW` · **Extended** · Playable |
| **Role** | **Primary:** **limited** battlefield interference and anti-air alerting (E-R). **Secondary:** cut enemy commander aura links. **Doctrine fit:** electronic warfare within the combined-arms system (E). |
| **Inspiration and background** | Truck with a shelter and folding antennas. C&C lineage (E-R): Mobile Radar Jammer (RA1); Mobile Sensor Array (TS); Conductor (TT). |
| **Visual identity** | none (asset requirement). Six-wheel truck, box shelter, telescopic mast. |
| **Statistics** | **400 HP** (`armor.veh_light`) · WHEEL_HEAVY 380 cm/s · F2 · sight 1100 cm · `DETECT.AIR_RADAR` 2600 cm · `DETECT.EMITTER` 3000 cm · crew 2 · food 2 · fuel `light` · no veterancy |
| **Commands** | Move, Stop (I) · Deploy Mast, Jam (P) · default stance Hold Fire · formation role `support_weapon` |
| **Production** | **$950** · `RD-BLD-05` · prerequisites: `RD-DEF-07` · **15 s** · 2 food · 1 petroleum · cap 3 |
| **Veterancy** | none |
| **AI** | `ai.unit.electronic_warfare` |
| **Multiplayer** | **MPA-SENSOR**: footprint owner-only; its active jammer is a `DETECT.EMITTER` contact to enemies within 3000 cm (§9.5 tell) |
| **Presentation** | ANIM-VEH-WHEEL + mast raise; static ripple VFX at the field edge for its owner. Icon: lightning over a mast |

**Weapons:** none (unarmed).

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.rd.sup_06.jamming_field` | Toggle (P) | §9.5 jamming field, radius **1200 cm** (lower band edge: "limited", E-R). Requires the mast deployed (4 s) | Self | free while deployed | Emitter detection → artillery or air strike; AC-ARM-10 counter-jamming has no extra effect (fields do not cancel) | Owner field outline | Jam when enemy guided weapons or air radar are within the field |
| `ability.rd.sup_06.raid_alert` | Passive (P) | Air contacts in its 2600 cm radar footprint are shared with every RD AA unit and RD-ARM-13 radar link | Self | free | Jamming (AC-AIR-07, AC-ARM-10) suppresses its contacts | Contact pips | Park between the front and the AA belt |

**Research and upgrades:** `tech.shared.electronic_countermeasures`: field +200 cm; trade-off: emitter detection range 3000 → 3600 cm.

**Counters:** *Beats:* guided weapons (AC-ARM-06/08, IA-INF-04), commander links (AC-INF-14). *Beaten by (hard):* emitter-cued artillery and aircraft; raiders. *Situational:* the field never hides units from sight (E).

**Status:** identity and "limited" E-R; EW rules P (framework §9.5).

### RD-SUP-07 — Reserve Reinforcement Carrier

| Field | Specification |
| --- | --- |
| **Identity** | `unit.rd.sup_07` · Red Directorate · Vehicle / reinforcement transport · `TPL-VEH-TRANSPORT` · **Extended** · Playable |
| **Role** | **Primary:** deploy infantry from the mobilization pool **with logistical limits** (E-R). **Secondary:** forward replenishment point for depleted squads. **Doctrine fit:** mass replacement through organization, never free manpower (E: talent doc protected canon). |
| **Inspiration and background** | Large armored troop carrier carrying reservists from rail heads. C&C lineage (E-R): Troop Crawler (Gen); Reckoner (KW); Bullfrog (RA3, delivery role). |
| **Visual identity** | none (asset requirement). Eight-wheel armored carrier with a large passenger box and roof hatches. |
| **Statistics** | **620 HP** (`armor.veh_light`) · WHEEL_HEAVY 380 cm/s · F2 · sight 1100 cm · **8 infantry slots** · crew 2 · food 2 · fuel `medium` · no veterancy |
| **Commands** | Move, Stop (I) · Load/Unload, Muster, Replenish (P) · default stance Hold Fire · formation role `reserve` |
| **Production** | **$800** · `RD-BLD-05` · prerequisites: `RD-BLD-11` · **14 s** · 2 food · 3 petroleum · cap 3 |
| **Veterancy** | none |
| **AI** | `ai.unit.transport` |
| **Multiplayer** | MPA-TRANSPORT; queued muster orders owner-only |
| **Presentation** | ANIM-VEH-WHEEL + ramp. Icon: carrier with a "+" |

**Weapons:** none (unarmed).

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.rd.sup_07.field_muster` | Active (P) | Queues RD-INF-01, -02 or -04 **at normal price and food**, produced at RD-BLD-03 and loaded instantly into the carrier while it is within 800 cm of RD-BLD-03 or RD-BLD-11. Away from them it cannot muster | Self, near a muster point | unit price · unit build time · — | Destroying RD-BLD-11 or the carrier | Queue panel | Muster when the front needs replacements |
| `ability.rd.sup_07.forward_replenishment` | Active (P) | Friendly RD squads within 600 cm can replenish here as at a producer (§10.3), at 1.2× the replenishment cost | Self aura 600 cm, stationary | §10.3 × 1.2 · up to 3 squads at a time | Kill the carrier | Medical/supply icon | Park 800 cm behind the front |

**Research and upgrades:** `tech.rd.industrial_mobilization_methods` applies to its musters (RD-BLD-03 time −20 %).

**Counters:** *Beaten by (hard):* AT weapons, artillery while parked. *Situational:* the Directorate's tempo engine; losing it with 8 squads aboard is catastrophic (passengers eject at −50 % HP, §6.10).

**Status:** identity and "logistical limits" E-R; no free manpower E; values P.

## 6. Aircraft and airborne support

Fixed-wing aircraft follow the sortie model (U-04): launch from `RD-BLD-07`, kerosene per sortie, rearm on return.

### RD-AIR-01 — Frontline Interceptor

| Field | Specification |
| --- | --- |
| **Identity** | `unit.rd.air_01` · Red Directorate · Aircraft / fighter · `TPL-AIR-FIGHTER` · **Extended** · Playable |
| **Role** | **Primary:** conventional multirole air-to-air fighter (E-R). **Secondary:** limited ground strafing. **Doctrine fit:** interceptor aviation inside the air-defense network (E). |
| **Inspiration and background** | Delta-wing MiG-style interceptor. C&C lineage (E-R): MiG (RA1); MiG (Gen); MiG Fighter (RA3). |
| **Visual identity** | none (asset requirement). Silver-grey delta, red stars on the fin. |
| **Statistics** | **360 HP** (`armor.air_fixed`) · FIXED_WING 2400 cm/s · FA · sight 2200 cm air · crew 1 + 1 ground crew · food 2 · fuel `kerosene` 2 per sortie · ExperienceValue 160 |
| **Commands** | Move, Attack, Patrol, Return to Base (P) · default stance Aggressive (air) · formation role `escort` |
| **Production** | **$1,550** · `RD-BLD-07` · **21 s** · 2 food · 2 kerosene per sortie · pad slots per airfield (4) |
| **Veterancy** | Standard |
| **AI** | `ai.unit.air_superiority` |
| **Multiplayer** | MPA-AIR |
| **Presentation** | ANIM-AIR-FIXED + afterburner. Icon: delta |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.rd.air_01.aam` air-to-air missiles | FLAK_AA | 0–1300 cm | 150 / 4.0 s; 4 per sortie | guided_missile | — | AIR_LOW, AIR_HIGH | — / — / — / 110 % rotary, 100 % fixed / — |
| `weapon.rd.air_01.gun` 23 mm-class cannon | AUTOCANNON | 0–700 cm | 18 / 0.3 s; 6 s of fire per sortie | hitscan | 3 | GROUND, AIR_LOW, AIR_HIGH | 65 % / 85 % / 10 % / 100 % rotary, 60 % fixed / 30 % |

**Abilities:** no unit-specific ability.

**Research and upgrades:** `tech.shared.radar_signal_processing` (shared contacts from RD-DEF-07 extend its acquisition).

**Counters:** *Beats (hard):* bombers and transports (AC-AIR-06/08/11, IA-AIR-05, MP-AIR-04/06, MP-SUP-09). *Beats (soft):* equal fighters when fighting inside RD radar coverage. *Beaten by (hard):* SAMs, AC-AIR-01 at cost parity outside RD radar. *Situational:* defensive by design.

**Status:** identity E-R; values P.

### RD-AIR-02 — Tactical Attack Aircraft

| Field | Specification |
| --- | --- |
| **Identity** | `unit.rd.air_02` · Red Directorate · Aircraft / strike jet · `TPL-AIR-STRIKE` · **Extended** · Playable |
| **Role** | **Primary:** air-to-ground strike in support of tank spearheads (E-R). **Secondary:** hit artillery and AA sites ahead of an armor push. **Doctrine fit:** tactical aviation (E). |
| **Inspiration and background** | Armored ground-attack jet with rocket pods and bombs. C&C lineage (E-R): Yak (RA1); Vindicator (RA3); Raptor (Gen, strike role). |
| **Visual identity** | none (asset requirement). Straight-wing armored jet, camouflage, rocket pods under the wings. |
| **Statistics** | **560 HP** (`armor.air_fixed`) · FIXED_WING 1600 cm/s · FA · sight 1800 cm · crew 1 + 1 · food 2 · `kerosene` 3 per sortie · ExperienceValue 190 |
| **Commands** | Attack, Move, Return to Base (P) · default stance Aggressive · formation role `escort` |
| **Production** | **$1,900** · `RD-BLD-07` · **25 s** · 2 food · 3 kerosene per sortie |
| **Veterancy** | Standard |
| **AI** | `ai.unit.air_strike` |
| **Multiplayer** | MPA-AIR |
| **Presentation** | ANIM-AIR-FIXED + weapon release. Icon: jet over a tank |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.rd.air_02.rockets` unguided rocket pods | SHAPED_CHARGE | 0–900 cm (attack run) | 16 × 30 per pass; 2 passes per sortie | shell_direct ripple, 80 cm splash | 4 | GROUND, STRUCTURE | 25 % / 100 % / 90 % / — / 70 % |
| `weapon.rd.air_02.bombs` 4 × 250 kg-class bombs | HIGH_EXPLOSIVE | overflight | 4 × 160; full 120, max 260 cm; 1 pass | bomb_drop; scatter 100–200 | 12 | GROUND, STRUCTURE | 110 % / 45 % / 12 % / — / 100 % |

**Abilities:** no unit-specific ability; *armored cockpit*: the first FLAK_AA hit each sortie deals −30 % (passive, P).

**Research and upgrades:** `tech.rd.tandem_warheads` does not apply (infantry RPGs only). None further proposed.

**Counters:** *Beats (hard):* tank columns and artillery parks without AA. *Beaten by (hard):* AA vehicles (AC-ARM-09, IA-ARM-08, MP-ARM-09), interceptors. *Situational:* time it after RD-ARM-07 has hit the enemy AA.

**Status:** identity E-R; values P.

### RD-AIR-03 — Close-Support Gunship

| Field | Specification |
| --- | --- |
| **Identity** | `unit.rd.air_03` · Red Directorate · Aircraft / attack helicopter · `TPL-AIR-HELI-ATTACK` · **Core** · Playable |
| **Role** | **Primary:** ground-attack helicopter with **short patrol duration** (E-R). **Secondary:** carry 1 infantry squad (Hind-style). **Doctrine fit:** air support for tank spearheads. |
| **Inspiration and background** | Hind-style heavily armed assault helicopter with a small troop bay. C&C lineage (E-R): Hind (RA1); Twinblade (RA3); Apache (Ren, role). |
| **Visual identity** | none (asset requirement). Tandem bubble canopies, stub wings with rocket pods, nose gun. |
| **Statistics** | **560 HP** (`armor.air_rotary`) · ROTARY 720 cm/s · FA · sight 1600 cm · **1 infantry slot** · crew 2 + 1 · food 3 · `kerosene` 2 per sortie · **loiter 90 s**, then must return · ExperienceValue 150 |
| **Commands** | Move, Attack, AttackMove, Guard, Return to Base (P) · default stance Aggressive · formation role `escort` |
| **Production** | **$1,450** · `RD-BLD-07` · **20 s** · 3 food · 2 kerosene per sortie |
| **Veterancy** | Standard |
| **AI** | `ai.unit.attack_helicopter`. Deltas: return when loiter < 15 s |
| **Multiplayer** | MPA-AIR + MPA-TRANSPORT |
| **Presentation** | ANIM-AIR-ROTARY + rocket ripple. Loiter timer pip. Icon: helicopter with rockets |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.rd.air_03.gun` nose 12.7 mm-class gun | AUTOCANNON | 0–900 cm | 14 / 0.3 s | hitscan | 4 | GROUND, AIR_LOW | 65 % / 85 % / 10 % / 100 % rotary / 30 % |
| `weapon.rd.air_03.rockets` rocket pods | SHAPED_CHARGE | 0–1000 cm | 8 × 30 per volley / 6 s; 4 volleys per sortie | shell_direct, 80 cm splash | 4 | GROUND, STRUCTURE | 25 % / 100 % / 90 % / — / 70 % |

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.rd.air_03.short_patrol` | Passive limit (P; E-R) | 90 s loiter per sortie; then automatic Return to Base | Self | — | — | Timer pip | — |

**Research and upgrades:** `tech.shared.electronic_countermeasures`: first guided AA hit −30 %; trade-off: kerosene +1 per sortie.

**Counters:** *Beats (hard):* light vehicles, infantry, APCs. *Beats (soft):* MBTs (rockets). *Beaten by (hard):* AA guns (AC-ARM-09, IA-ARM-08, MP-ARM-09), MANPADS (AC-INF-03), interceptors. *Situational:* the short loiter prevents the RA2 permanent-air-cover pattern.

**Status:** identity and short patrol E-R; values P.

### RD-AIR-04 — Assault Transport Helicopter

| Field | Specification |
| --- | --- |
| **Identity** | `unit.rd.air_04` · Red Directorate · Aircraft / transport helicopter · `TPL-AIR-HELI-TRANSPORT` · **Extended** · Playable |
| **Role** | **Primary:** transport airborne assault detachments and supplies (E-R). **Secondary:** sling-load ammunition to artillery parks. **Doctrine fit:** operational depth. |
| **Inspiration and background** | Large medium-lift transport helicopter. C&C lineage (E-R): Chinook (RA1); Twinblade (RA3); NightHawk (RA2). |
| **Visual identity** | none (asset requirement). Big single-rotor hull, rear clamshell, door gun. |
| **Statistics** | **620 HP** (`armor.air_rotary`) · ROTARY 680 cm/s · FA · sight 1400 cm · **4 infantry slots** or 20 ammunition units · crew 3 · food 3 · `kerosene` 2 per sortie · ExperienceValue 120 |
| **Commands** | Move, Load/Unload, Fast-Rope, Return to Base (P) · default stance Guard · formation role `protected` |
| **Production** | **$1,150** · `RD-BLD-07` · **17 s** · 3 food |
| **Veterancy** | Standard (armed) |
| **AI** | `ai.unit.air_transport` |
| **Multiplayer** | MPA-AIR + MPA-TRANSPORT |
| **Presentation** | ANIM-AIR-ROTARY + ropes. Icon: helicopter with figures |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.rd.air_04.door_gun` door MG | SMALL_ARMS | 0–650 cm | 10 / 0.3 s | hitscan | 3 | GROUND, AIR_LOW | 100 % / 15 % / 2 % / 25 % rotary / 8 % |

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.shared.fast_rope` | Active (P) | Unloads all passengers in 6 s while hovering (exposure window, §10.1) | Ground point | free · 6 s | AA during the hover | Ropes | Only where no AA is observed |
| `ability.rd.air_04.sling_resupply` | Active (P) | Drops 20 ammunition units to friendly limited-ammo units within 400 cm of the drop point | Ground point | §10.4 pricing | AA; drop point under fire | Pallet | Resupply cut-off artillery |

**Research and upgrades:** none proposed.

**Counters:** *Beaten by (hard):* every AA weapon; interceptors. *Situational:* flanking bridgehead seizure.

**Status:** identity E-R; values P.

### RD-AIR-05 — Heavy Transport Aircraft

| Field | Specification |
| --- | --- |
| **Identity** | `unit.rd.air_05` · Red Directorate · Aircraft / transport · `TPL-AIR-TRANSPORT` · **Extended** · Playable |
| **Role** | **Primary:** paratrooper and logistics flights; **no independent strategic-bomber unlock** (E-R). **Secondary:** airlift fuel and ammunition to a forward airstrip. **Doctrine fit:** operational depth (E). |
| **Inspiration and background** | Four-engine high-wing military transport. C&C lineage (E-R): Paratroopers (RA1, delivery role); Century Bomber (RA3, transport role); V-35 Ox (TW). |
| **Visual identity** | none (asset requirement). High-wing four-turboprop transport with a rear ramp. |
| **Statistics** | **700 HP** (`armor.air_fixed`) · FIXED_WING 1500 cm/s · FA · sight 1600 cm · **6 infantry slots** or 1 light vehicle or 30 fuel/ammunition units · crew 4 + 1 · food 5 · `kerosene` 4 per sortie · no veterancy |
| **Commands** | Move, Paradrop, Airland, Return to Base (P) · default stance Hold Fire · formation role `protected` |
| **Production** | **$1,600** · `RD-BLD-07` · **26 s** · 5 food · cap 2 |
| **Veterancy** | none |
| **AI** | `ai.unit.air_transport` |
| **Multiplayer** | MPA-AIR + MPA-TRANSPORT |
| **Presentation** | ANIM-AIR-FIXED + ramp, paradrop canopies. Icon: transport with a parachute |

**Weapons:** none (unarmed).

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.shared.paradrop` | Active (P) | Drops passengers along a 1200 cm line during a 10 s exposure window; squads land Shaken | Ground line in observed or reported area | free · 10 s | AA along the path; interceptors | Canopies | Only where no AA was observed in the last 60 s |
| `ability.rd.air_05.air_bridge` | Active (P) | Delivers 30 fuel or ammunition units to an owned forward airstrip or depot | Owned RD-BLD-07, RD-DEF-11 | free · landing 8 s | Interceptors | Pallet | When the rail route is cut |

**Research and upgrades:** none. It cannot be upgraded into a bomber (E-R).

**Counters:** *Beaten by (hard):* interceptors and SAMs. *Situational:* rescues a cut-off front.

**Status:** identity and "no bomber unlock" E-R; values P.

### RD-AIR-06 — Radar Reconnaissance Aircraft

| Field | Specification |
| --- | --- |
| **Identity** | `unit.rd.air_06` · Red Directorate · Aircraft / airborne early warning · `TPL-AIR-SURVEILLANCE` · **Extended** · Playable |
| **Role** | **Primary:** high-altitude observation and incoming-raid detection (E-R). **Secondary:** radar link for RD-ARM-13 and RD-AIR-01. **Doctrine fit:** integrated air defense (E). |
| **Inspiration and background** | Turboprop airborne-radar aircraft with a rotodome. C&C lineage (E-R): Spy Plane (RA1); Surveillance Sweep (RA3); Eyes of the People (TT, information role). |
| **Visual identity** | none (asset requirement). Four-turboprop airframe with a dorsal rotodome. |
| **Statistics** | **520 HP** (`armor.air_fixed`) · FIXED_WING 1500 cm/s (loiter) · FA · visual sight 2600 cm · `DETECT.AIR_RADAR` **4500 cm** · crew 4 + 1 · food 5 · `kerosene` 4 per 120 s loiter · no veterancy |
| **Commands** | Move, Patrol (orbit), Return to Base (P) · default stance Hold Fire |
| **Production** | **$2,100** · `RD-BLD-07` · prerequisites: `RD-DEF-07` · **28 s** · 5 food · **cap 1** |
| **Veterancy** | none |
| **AI** | `ai.unit.air_surveillance` |
| **Multiplayer** | MPA-AIR + MPA-SENSOR (contacts only; identity requires visual sight) |
| **Presentation** | ANIM-AIR-FIXED + rotodome spin. Icon: dome over a plane |

**Weapons:** none (unarmed).

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.rd.air_06.raid_warning` | Passive (P) | Air contacts within 4500 cm become shared radar contacts for all RD units; RD-ARM-13 within the footprint gets its full range | Orbit | free; loiter 120 s | Jamming (AC-AIR-07, AC-ARM-10); interceptors | Contact pips; warning cue for incoming raids | Orbit behind the SAM belt |

**Research and upgrades:** `tech.shared.radar_signal_processing`: footprint +500 cm.

**Counters:** *Beaten by (hard):* interceptors (AC-AIR-01, IA-AIR-01), long-range SAMs. *Situational:* contacts are never identity; ground units under it are not revealed except by visual sight.

**Status:** identity E-R; detection rules P (framework §9).

### RD-AIR-07 — Frontline Bomber

| Field | Specification |
| --- | --- |
| **Identity** | `unit.rd.air_07` · Red Directorate · Aircraft / bomber · `TPL-AIR-STRIKE` (bomber) · **Extended** · Playable |
| **Role** | **Primary:** conventional carpet and area bombing support, **limited precision** (E-R). **Secondary:** break fortified lines and soft bases. **Doctrine fit:** concentrated firepower. Conventional only; not a nuclear delivery unit (E: ICBM delivery). |
| **Inspiration and background** | Swing-wing twin-engine frontline bomber. C&C lineage (E-R): Badger Bomber (RA1, support); Carpet Bomber (Gen); Kirov Airship (RA2, heavy bombing role). |
| **Visual identity** | none (asset requirement). Large swept-wing jet bomber with a glazed nose. |
| **Statistics** | **700 HP** (`armor.air_fixed`) · FIXED_WING 1600 cm/s · FA · sight 1800 cm · crew 3 + 1 · food 4 · `kerosene` 5 per sortie · ExperienceValue 240 |
| **Commands** | Attack (ground line), Return to Base (P) · default stance Hold Fire until ordered |
| **Production** | **$2,400** · `RD-BLD-07` · prerequisites: `RD-BLD-06` · **32 s** · 4 food · cap 3 |
| **Veterancy** | Standard |
| **AI** | `ai.unit.air_strike`. Deltas: only along routes with ≤ 1 observed AA system |
| **Multiplayer** | MPA-AIR |
| **Presentation** | ANIM-AIR-FIXED + bomb bay. Icon: bomber with a line of bombs |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.rd.air_07.carpet` carpet stick, 12 bombs | HIGH_EXPLOSIVE | overflight; 2400 cm stick | 12 × 140; full 120, max 260 cm; 1 pass per sortie | bomb_drop; scatter 150–350 (limited precision) | 12 | GROUND, STRUCTURE | 110 % / 45 % / 12 % / — / 100 % / 60 % hardened |

**Abilities:** no unit-specific ability; the stick line is chosen by a two-point order.

**Research and upgrades:** none proposed (no precision path, E-R).

**Counters:** *Beats (hard):* infantry blocks, soft bases, static lines. *Beaten by (hard):* interceptors, SAM networks. *Situational:* friendly fire risk at the stick edge (§6.7).

**Status:** identity and "limited precision" E-R; conventional-only E; values P.

### RD-AIR-08 — Artillery Targeting Helicopter

| Field | Specification |
| --- | --- |
| **Identity** | `unit.rd.air_08` · Red Directorate · Aircraft / observation helicopter · `TPL-AIR-SCOUT` (rotary) · **Extended** · Playable |
| **Role** | **Primary:** spot and correct rocket and artillery fire (E-R). **Secondary:** route reconnaissance. **Doctrine fit:** reconnaissance feeding artillery (E). |
| **Inspiration and background** | Light observation helicopter with a mast-mounted optic. C&C lineage (E-R): Orca (TW, observation); Harpy (TS, armed recon); Hammerhead (KW). |
| **Visual identity** | none (asset requirement). Small two-seat helicopter with a roof optic. |
| **Statistics** | **320 HP** (`armor.air_rotary`) · ROTARY 760 cm/s · FA · sight **2200 cm** · `DETECT.CONCEALED` 1300 cm · crew 2 · food 3 · `kerosene` 1 per 120 s · no veterancy |
| **Commands** | Move, Patrol, Designate Target, Return to Base (P) · default stance Hold Fire · formation role `scout_screen` |
| **Production** | **$800** · `RD-BLD-07` · **14 s** · 3 food · cap 3 |
| **Veterancy** | none |
| **AI** | `ai.unit.air_scout`. Deltas: hover 1800–2100 cm from designated targets |
| **Multiplayer** | MPA-AIR |
| **Presentation** | ANIM-AIR-ROTARY. Icon: helicopter with crosshair |

**Weapons:** none (unarmed).

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.rd.air_08.fire_correction` | Active (P) | 25 s: RD indirect fire at a designated observed point gets scatter ×0.6 (better than the §6.6 ×0.7 spotter rule) and RD-ARM-07 salvoes reload 20 % faster from logistics | Observed point ≤ 2200 cm | free · 25 s · 30 s | AA; smoke over the point | Correction marker | Designate the largest observed cluster |

**Research and upgrades:** `tech.shared.tactical_radio_modernization`: designate range +200 cm.

**Counters:** *Beaten by (hard):* AA guns, MANPADS, interceptors. *Situational:* with RD-ARM-07 it turns unguided barrages into reliable zone denial.

**Status:** identity E-R; values P.

### RD-AIR-09 — Strategic Missile Transporter

| Field | Specification |
| --- | --- |
| **Identity** | `unit.rd.air_09` · Red Directorate · Ground missile convoy (listed under aircraft in the roster) · `TPL-VEH-ARTY` (theater missile) · **Conditional** · Playable · cap 2 |
| **Role** | **Primary:** oversized missile-erector convoy, **not a nuclear-producing unit** (E-R). P resolution: a **conventional** theater missile against fixed targets (U-13). The ICBM stays the RD-DEF-14 silo (E). **Doctrine fit:** mobile missile systems (E). |
| **Inspiration and background** | Eight-wheel transporter-erector-launcher with a single large conventional missile. C&C lineage (E-R): V4 Rocket Launcher (RA3); V3 Rocket Launcher (RA2); Nuke Cannon (Gen, chassis role). |
| **Visual identity** | none (asset requirement). Very long eight-wheel truck, erector arm, missile under canvas. |
| **Statistics** | **620 HP** (`armor.veh_light`) · WHEEL_HEAVY 260 cm/s · F4 · sight 1000 cm · crew 4 · food 4 · fuel `medium` · ExperienceValue 200 |
| **Commands** | Move, Stop (I) · Erect, Launch (Fire Mission), Stow (P) · default stance Hold Fire · formation role `protected` |
| **Production** | **$2,600** · `RD-BLD-06` · prerequisites: `RD-BLD-02` and `tech.rd.theater_missile_program` · **40 s** · 4 food · 3 petroleum · **cap 2** · each missile reload costs $600 at RD-DEF-11 |
| **Veterancy** | none |
| **AI** | `ai.unit.artillery`. Deltas: only fire on observed or reported high-value structures; displace after every launch |
| **Multiplayer** | MPA-INDIRECT (launch flash visible to observers; the impact is a large public event) |
| **Presentation** | ANIM-VEH-WHEEL + erector rise (12 s); launch plume. Icon: missile on a truck |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.rd.air_09.missile` conventional theater missile | HIGH_EXPLOSIVE | **3000–9000 cm** | 900 per launch; full 250, max 500 cm, edge 30 %; AmmoCapacity 1 | ballistic; scatter 300–600 (U-13) | 20 | GROUND, STRUCTURE | 110 % / 45 % / 12 % / — / 100 % / 60 % hardened |

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.rd.air_09.erect_and_launch` | Active (P) | 12 s erection (immobile, observed silhouette changes), then launch at an observed or reported structure; 30 s flight time; enemy receives a launch warning when any of its units or radars observes the launcher or the missile | Ground point 3000–9000 cm | missile reload $600 · 12 s + 30 s · one missile | Killing it while erect; SAMs cannot intercept it, but the warning lets the victim evacuate | Erector, plume, warning | Launch only at targets valued ≥ 3× the missile cost |

**Research and upgrades:** `tech.rd.theater_missile_program` (RD-BLD-02, $1,800, 120 s): unlocks the unit. No nuclear upgrade exists (E).

**Counters:** *Beats (hard):* static high-value structures. *Beaten by (hard):* raids and aircraft during transit and erection. *Situational:* strategic pressure without crossing the nuclear rules.

**Status:** identity and "not nuclear-producing" E-R; conventional role P pending **U-13**; values P.

## 7. Naval, riverine and amphibious assets

Naval units need an authored water layer and `RD-BLD-08` Naval Construction Basin (E-R: map-restricted). RD-SEA-02 is the exception: it is amphibious and leaves the basin for land maps with rivers.

### RD-SEA-01 — River Patrol Boat

| Field | Specification |
| --- | --- |
| **Identity** | `unit.rd.sea_01` · Red Directorate · Naval / patrol boat · `TPL-NAV-PATROL` · **Conditional** (water layer) · Playable |
| **Role** | **Primary:** protect ferry crossings and supply bridges (E-R). **Secondary:** hunt small boats and amphibious landings. **Doctrine fit:** protect the logistics arteries (E). |
| **Inspiration and background** | Armored river gunboat with a tank-style turret. C&C lineage (E-R): Gunboat (RA1); Stingray (RA3, patrol role). |
| **Visual identity** | none (asset requirement). Low armored hull with a small turret and a twin MG. |
| **Statistics** | **420 HP** (`armor.nav_light`) · SURFACE_SHALLOW 560 cm/s · FN 300 cm · sight 1600 cm · crew 5 · food 5 · fuel `medium` · ExperienceValue 70 |
| **Commands** | Move, Attack, AttackMove, Stop, Guard, Patrol (P) · default stance Guard · formation role `escort` |
| **Production** | **$700** · `RD-BLD-08` · **14 s** · 5 food · 3 petroleum |
| **Veterancy** | Standard |
| **AI** | `ai.unit.naval_patrol`. Deltas: Guard the nearest bridge or ferry node |
| **Multiplayer** | MPA-STD |
| **Presentation** | ANIM-NAV + turret yaw. Icon: boat with a turret |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.rd.sea_01.gun` 76 mm-class turret gun | HIGH_EXPLOSIVE | 0–850 cm | 55 / 3.0 s; max 120 cm | shell_direct | 6 | GROUND, WATER_SURFACE, STRUCTURE | 110 % / 45 % / 12 % / — / 100 %; NAV_L 60 % |
| `weapon.rd.sea_01.mg` twin 14.5 mm-class MG | AUTOCANNON | 0–750 cm | 12 / 0.4 s | hitscan | 4 | GROUND, WATER_SURFACE, AIR_LOW | 65 % / 85 % / 10 % / 100 % rotary / 30 %; NAV_L 90 % |

**Abilities:** no unit-specific ability.

**Research and upgrades:** none proposed.

**Counters:** *Beats (hard):* boats and barges (AC-SEA-01/02, IA-SEA-02/03, MP-SEA-01/02/04), infantry on banks. *Beaten by (hard):* missile boats (AC-SEA-03), aircraft, MBTs on the bank. *Situational:* river maps only.

**Status:** identity E-R; values P.

### RD-SEA-02 — Amphibious Motor-Rifle Carrier

| Field | Specification |
| --- | --- |
| **Identity** | `unit.rd.sea_02` · Red Directorate · Vehicle / amphibious APC · `TPL-VEH-IFV` (amphibious) · **Extended** · Playable |
| **Role** | **Primary:** move soldiers through shallow river crossings (E-R). **Secondary:** fire support on the far bank. **Doctrine fit:** rapid exploitation across rivers (E). |
| **Inspiration and background** | Eight-wheel amphibious APC with a turret HMG and a water jet. C&C lineage (E-R): Amphibious Transport (RA2); Bullfrog (RA3); Amphibious APC (TS). |
| **Visual identity** | none (asset requirement). Boat-shaped eight-wheel hull, small turret, trim vane. |
| **Statistics** | **520 HP** (`armor.veh_light`) · WHEEL_HEAVY 400 cm/s on land; AMPHIBIOUS ford ×0.70, deep water ×0.45 · F2 · sight 1200 cm · **4 infantry slots**, 2 fire ports · crew 2 · food 2 · fuel `medium` · ExperienceValue 70 |
| **Commands** | as TPL-VEH-IFV · default stance Guard · formation role `frontline` |
| **Production** | **$750** · `RD-BLD-05` (land) or `RD-BLD-08` · **14 s** · 2 food · 3 petroleum |
| **Veterancy** | Standard |
| **AI** | `ai.unit.ifv`. Deltas: cross where no authored bridge is within 2000 cm of the order path |
| **Multiplayer** | MPA-STD + MPA-TRANSPORT |
| **Presentation** | ANIM-VEH-WHEEL + trim vane, bow wave in water. Icon: APC over waves |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.rd.sea_02.hmg` turret 14.5 mm-class HMG | AUTOCANNON | 0–750 cm | 14 / 0.4 s | hitscan | 4 | GROUND, WATER_SURFACE, AIR_LOW | 65 % / 85 % / 10 % / 100 % rotary / 30 % |

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.rd.sea_02.river_crossing` | Passive (P) | Enters and leaves water at authored ford or bank edges; passengers stay loaded in water (cannot unload in deep water) | Self | — | Fire while it is slow in the water | Wake | — |

**Research and upgrades:** `tech.rd.applique_armor` does not apply (amphibious buoyancy). None proposed.

**Counters:** *Beats (hard):* infantry on the far bank. *Beaten by (hard):* AT weapons, especially while swimming (speed ×0.45). *Situational:* turns rivers from barriers into options; Unaligned waterway rules still apply (E).

**Status:** identity E-R; values P.

### RD-SEA-03 — Coastal Torpedo Boat

| Field | Specification |
| --- | --- |
| **Identity** | `unit.rd.sea_03` · Red Directorate · Naval / fast attack craft · `TPL-NAV-PATROL` · **Conditional** · Playable |
| **Role** | **Primary:** interdict port access and nearshore convoys (E-R). **Secondary:** ambush large ships in narrow waters. **Doctrine fit:** sea denial rather than sea control. |
| **Inspiration and background** | Hydrofoil torpedo boat. C&C lineage (E-R): Sea Scorpion (RA2, escort role); Yari Mini-Sub (RA3, light marine concept). |
| **Visual identity** | none (asset requirement). Small hydrofoil, two torpedo tubes, twin gun. |
| **Statistics** | **340 HP** (`armor.nav_light`) · SURFACE_SHALLOW **700 cm/s** · FN 250 cm · sight 1600 cm · crew 4 · food 4 · fuel `medium` · ExperienceValue 80 |
| **Commands** | as TPL-NAV-PATROL · default stance Aggressive · formation role `flank` |
| **Production** | **$800** · `RD-BLD-08` · **14 s** · 4 food · 3 petroleum |
| **Veterancy** | Standard |
| **AI** | `ai.unit.naval_patrol`. Deltas: torpedo only NAV_HEAVY or loaded transports |
| **Multiplayer** | MPA-STD |
| **Presentation** | ANIM-NAV + foil spray. Icon: boat with torpedo |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.rd.sea_03.torpedo` torpedoes | TORPEDO | 300–1400 cm | 260 / 10 s; AmmoCapacity 2 | torpedo, 600 cm/s, tracking medium | — | WATER_SURFACE | NAV_L 110 %, NAV_H 100 % |
| `weapon.rd.sea_03.gun` twin 30 mm-class gun | AUTOCANNON | 0–800 cm | 12 / 0.4 s | hitscan | 3 | WATER_SURFACE, AIR_LOW | NAV_L 90 %; 100 % rotary |

**Abilities:** no unit-specific ability.

**Research and upgrades:** none proposed.

**Counters:** *Beats (hard):* frigates and transports without escorts (AC-SEA-02/04, MP-SEA-01/04/06). *Beaten by (hard):* patrol launches, helicopters, frigate guns. *Situational:* glass cannon; retreat after the torpedoes are spent.

**Status:** identity E-R; values P.

### RD-SEA-04 — Conventional Attack Submarine

| Field | Specification |
| --- | --- |
| **Identity** | `unit.rd.sea_04` · Red Directorate · Naval / submarine · `TPL-NAV-SUB` · **Conditional** · Playable |
| **Role** | **Primary:** silent offshore interdiction, **deep-water restricted** (E-R). **Secondary:** hunt enemy submarines. **Doctrine fit:** Adriatic sea denial. |
| **Inspiration and background** | Diesel-electric attack submarine. C&C lineage (E-R): Submarine (RA1); Typhoon Attack Submarine (RA2); Akula Submarine (RA3). |
| **Visual identity** | none (asset requirement). Black hull, tall sail. |
| **Statistics** | **950 HP** (`armor.nav_heavy`) · SUBSURFACE 340 cm/s (submerged ×0.8) · FN 600 cm · sight 1600 cm · `DETECT.SUBMERGED` 1200 cm · crew 6 (abstracted) · food 6 · fuel `medium` · ExperienceValue 190 |
| **Commands** | Move, Attack, Stop, Patrol, Dive/Surface (P) · default stance Hold Fire · formation role `flank` |
| **Production** | **$1,800** · `RD-BLD-08` · **40 s** · 6 food · 3 petroleum · cap 3 |
| **Veterancy** | Standard |
| **AI** | `ai.unit.submarine` |
| **Multiplayer** | MPA-STD + Submerged state (position never replicates to enemies without `DETECT.SUBMERGED`) |
| **Presentation** | ANIM-SUB + dive/surface. Icon: submarine |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.rd.sea_04.torpedo` heavy torpedoes | TORPEDO | 300–1400 cm | 320 / 8 s; AmmoCapacity 8 | torpedo; reveals for 4 s | — | WATER_SURFACE, WATER_SUBSURFACE | NAV_L 110 %, NAV_H 100 % |

**Abilities:** shared *Dive/Surface* (P). No unit-specific ability.

**Research and upgrades:** none proposed.

**Counters:** *Beats (hard):* surface ships without sonar (MP-SEA-01/04, AC-SEA-02/06). *Beaten by (hard):* frigates and destroyers with `DETECT.SUBMERGED` (AC-SEA-04), maritime patrol aircraft (MP-AIR-05, UN-AIR-08). *Situational:* deep water only.

**Status:** identity E-R; values P.

### RD-SEA-05 — Anti-Air Escort Destroyer

| Field | Specification |
| --- | --- |
| **Identity** | `unit.rd.sea_05` · Red Directorate · Naval / major combatant · `TPL-NAV-COMBATANT` · **Conditional** · Playable |
| **Role** | **Primary:** AA and fleet protection along the Adriatic (E-R). **Secondary:** anti-submarine screen. **Doctrine fit:** integrated air defense at sea (E). |
| **Inspiration and background** | Guided-missile destroyer with twin SAM launchers and gun turrets. C&C lineage (E-R): Destroyer (RA1); Aegis Cruiser (RA2); Dreadnought (RA3, hull role). |
| **Visual identity** | none (asset requirement). Grey hull, two SAM rails, radar masts, forward turret. |
| **Statistics** | **2,000 HP** (`armor.nav_heavy`) · SURFACE_DEEP 340 cm/s · FN 700 cm · sight 2000 cm · `DETECT.AIR_RADAR` 4000 cm · `DETECT.SUBMERGED` 1400 cm · crew 14 (abstracted) · food 14 · fuel `heavy` · ExperienceValue 260 |
| **Commands** | as TPL-NAV-COMBATANT · default stance Aggressive (air) · formation role `escort` |
| **Production** | **$2,600** · `RD-BLD-08` · **45 s** · 14 food · 7 petroleum · cap 2 |
| **Veterancy** | Standard |
| **AI** | `ai.unit.naval_combatant`. Deltas: stay within 1500 cm of escorted ships |
| **Multiplayer** | MPA-STD + MPA-SENSOR |
| **Presentation** | ANIM-NAV + launcher rails, radar spin. Icon: ship with a missile |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.rd.sea_05.sam` naval SAM | FLAK_AA | 300–2400 cm | 170 / 4 s; AmmoCapacity 24 | guided_missile | — | AIR_LOW, AIR_HIGH | 110 % rotary, 100 % fixed |
| `weapon.rd.sea_05.gun` 130 mm-class turret | HIGH_EXPLOSIVE | 0–1600 cm | 90 / 3.5 s; max 150 cm | shell_direct | 8 | WATER_SURFACE, GROUND, STRUCTURE | NAV_L 60 %, NAV_H 25 %; ground per matrix |
| `weapon.rd.sea_05.asw` ASW rockets | TORPEDO | 0–1200 cm | 180 / 12 s | lobbed | — | WATER_SUBSURFACE | NAV_H 100 % |

**Abilities:** no unit-specific ability.

**Research and upgrades:** `tech.shared.radar_signal_processing`.

**Counters:** *Beats (hard):* aircraft over water, submarines that reveal themselves. *Beaten by (hard):* coordinated anti-ship missiles (AC-SEA-03, MP-SEA-06, MP-MKT-03), torpedo ambushes. *Situational:* deep-water maps only.

**Status:** identity E-R; values P.

### RD-SEA-06 — Shore Bombardment Cruiser

| Field | Specification |
| --- | --- |
| **Identity** | `unit.rd.sea_06` · Red Directorate · Naval / major combatant · `TPL-NAV-COMBATANT` · **Conditional** · Playable · cap 1 |
| **Role** | **Primary:** slow large-calibre coastal siege vessel (E-R). **Secondary:** heavy surface action. **Doctrine fit:** concentrated firepower against coastal fortifications. |
| **Inspiration and background** | Gun cruiser with three triple 152 mm-class turrets. C&C lineage (E-R): Cruiser (RA1); Dreadnought (RA2); Dreadnought (RA3). |
| **Visual identity** | none (asset requirement). Long grey hull, three large turrets. |
| **Statistics** | **2,600 HP** (`armor.nav_heavy`) · SURFACE_DEEP **300 cm/s** · FN 900 cm · sight 2000 cm · crew 20 (abstracted) · food 20 · fuel `heavy` · ExperienceValue 320 |
| **Commands** | as TPL-NAV-COMBATANT + Fire Mission · default stance Hold Ground · formation role `support_weapon` |
| **Production** | **$3,200** · `RD-BLD-08` · prerequisites: `RD-BLD-06` · **55 s** · 20 food · 7 petroleum · **cap 1** |
| **Veterancy** | Standard |
| **AI** | `ai.unit.naval_combatant` (artillery behavior for shore fire) |
| **Multiplayer** | MPA-STD + MPA-INDIRECT |
| **Presentation** | ANIM-NAV + turret salvoes. Icon: ship with three barrels |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.rd.sea_06.main` 9 × 152 mm-class guns | HIGH_EXPLOSIVE | **1000–3600 cm** (shore fire) | 9 × 70 per salvo / 10 s | ballistic; full 80, max 180 cm; scatter 150–400 | 12 | GROUND, STRUCTURE, WATER_SURFACE | per matrix; NAV_H 25 % |

**Abilities:** no unit-specific ability; indirect shore fire needs observation (§6.8).

**Research and upgrades:** none proposed.

**Counters:** *Beats (hard):* coastal defenses, ports, shoreline bases. *Beaten by (hard):* submarines, anti-ship missiles, strike aircraft. *Situational:* slowest unit in the fleet; requires an escort.

**Status:** identity E-R; values P.

### RD-SEA-07 — Amphibious Headquarters Vessel

| Field | Specification |
| --- | --- |
| **Identity** | `unit.rd.sea_07` · Red Directorate · Naval / command and logistics ship · `TPL-NAV-TRANSPORT` · **Conditional** · Playable · cap 1 |
| **Role** | **Primary:** floating logistics hub for **limited** coastal missions (E-R). **Secondary:** landing-force transport and command post (framework §13.5: command vehicle, not a talent appointment). **Doctrine fit:** operational depth from the sea. |
| **Inspiration and background** | Amphibious command ship with a well deck. C&C lineage (E-R): Transport Ship (RA1); Aircraft Carrier (RA2, command function); Kodiak (TT, mobile support function). |
| **Visual identity** | none (asset requirement). Large hull, stern well deck, antennas. |
| **Statistics** | **2,200 HP** (`armor.nav_heavy`) · SURFACE_DEEP 320 cm/s · FN 900 cm · sight 2000 cm · **8 infantry slots + 4 vehicle slots** · crew 16 · food 16 · fuel `heavy` · no veterancy |
| **Commands** | Move, Stop (I) · Load/Unload, Beach Landing (P) · default stance Hold Fire · formation role `protected` |
| **Production** | **$2,800** · `RD-BLD-08` · **50 s** · 16 food · 7 petroleum · **cap 1** |
| **Veterancy** | none |
| **AI** | `ai.unit.transport` (naval) |
| **Multiplayer** | MPA-TRANSPORT + MPA-ECON |
| **Presentation** | ANIM-NAV + well-deck doors. Icon: ship with a flag |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.rd.sea_07.ciws` twin 30 mm-class guns | AUTOCANNON | 0–900 cm | 12 / 0.3 s | hitscan | 3 | WATER_SURFACE, AIR_LOW, AIR_HIGH | NAV_L 90 %; 100 % rotary, 60 % fixed |

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.rd.sea_07.floating_depot` | Passive (P) | Acts as a producer for replenishment (§10.3) and a Resupply source (§10.4) for units within 800 cm, including units on the shore | Self aura 800 cm | §10.3 and §10.4 pricing | Sinking it (passengers die unless adjacent to shore, §6.10) | Supply ring | Anchor 600 cm off the landing beach |
| `ability.rd.sea_07.limited_mission` | Passive limit (P; E-R) | Supply stock 60 units; refills only at RD-BLD-08 | Self | — | Blockade | Stock bar | Return at 10 % stock |

**Research and upgrades:** none proposed.

**Counters:** *Beaten by (hard):* submarines, missile boats, aircraft. *Situational:* enables a landing; it cannot sustain one indefinitely (E-R "limited").

**Status:** identity E-R; values P.

## 8. Faction roster summary

| ID | Name | Cat. | Template | Tier | Primary role | HP | Armor | Speed | USD | Build s | Producer | Hard counters (to it) | It hard-counters |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| RD-INF-01 | Directorate Rifle Platoon | INF | INF-LINE | Core | Mass line | 250 | inf_light | 200 | 175 | 6 | BLD-03 | HMG, flame, artillery | Agents, scouts |
| RD-INF-02 | Motor-Rifle Section | INF | INF-LINE | Core | Mech rifles + RPG | 240 | inf_light | 205 | 210 | 7 | BLD-03 | HMG, flame | Light vehicles |
| RD-INF-03 | Heavy Machine-Gun Team | INF | INF-HW | Core | Suppression | 180 | inf_light | 180 | 260 | 9 | BLD-03 | Marksmen, mortars | Infantry in open |
| RD-INF-04 | Portable Rocket Section | INF | INF-AT | Core | Cheap AT + AA | 180 | inf_light | 200 | 300 | 8 | BLD-03 | HMG, artillery | Light armor |
| RD-INF-05 | Mortar Infantry Team | INF | INF-HW | Ext | Indirect fire | 180 | inf_light | 175 | 340 | 10 | BLD-03 | Raiders, marksmen | Infantry in cover |
| RD-INF-06 | Combat Pioneer | INF | INF-SUPPORT | Core | Engineer, bridges | 165 | inf_light | 200 | 240 | 9 | BLD-04/03 | Any armed unit | Mines, obstacles |
| RD-INF-07 | Rail Security Troop | INF | INF-LINE | Ext | Depot guard, spy screen | 200 | inf_light | 195 | 160 | 6 | BLD-03 | HMG, armor | Agents at depots |
| RD-INF-08 | Chemical Defense Detachment | INF | INF-SUPPORT | Ext | Decontamination | 195 | inf_protected | 185 | 280 | 9 | BLD-03 | Combat infantry | Contamination (remedy) |
| RD-INF-09 | Special Recon Riflemen | INF | INF-SPEC | Core | Recon, spotting | 140 | inf_light | 215 | 420 | 12 | BLD-03 | Scouts, marksmen | Officers, agents, HW |
| RD-INF-10 | Political Liaison Officer | INF | INF-COMMAND | Core | Discipline aura | 220 | inf_protected | 210 | 420 | 14 | BLD-03 | Marksmen, jammers | Resolve attrition (soft) |
| RD-INF-11 | Directorate Intelligence Courier | INF | INF-COVERT | Cond | Intel theft, charges | 100 | inf_light | 215 | 640 | 16 | BLD-03 | DETECT.SPY | Research secrecy |
| RD-INF-12 | Resource Diversion Agent | INF | INF-COVERT | Cond | Resource diversion | 90 | inf_light | 205 | 700 | 18 | BLD-03 | DETECT.SPY, audits | Stockpiles |
| RD-INF-13 | Special Forces Colonel | INF | INF-COMMAND | Ext | Commander (cap 1) | 500 | inf_protected | 220 | 1,250 | 30 | BLD-03 | Marksmen, jammers | — (multiplier) |
| RD-ARM-01 | Patrol Motorcycle | ARM | VEH-SCOUT | Core | Opening scout (rifle) | 200 | veh_soft | 580 | 180 | 6 | BLD-05/01 | Any armor, HMG | Agents, engineers |
| RD-ARM-02 | Motorized Infantry Truck | ARM | VEH-TRANSPORT | Core | Cheap transport | 340 | veh_soft | 440 | 250 | 8 | BLD-05 | AT, autocannon | — |
| RD-ARM-03 | Tracked Personnel Carrier | ARM | VEH-IFV | Core | Heavy APC (5 slots) | 560 | veh_light | 350 | 700 | 13 | BLD-05 | AT infantry, MBT | Infantry, trucks |
| RD-ARM-04 | People's Battle Tank | ARM | VEH-MBT | Core | Mass MBT | 880 | veh_heavy | 315 | 1,050 | 19 | BLD-05 | ATGM, attack heli | IFVs, light armor |
| RD-ARM-05 | Improved Heavy Battle Tank | ARM | VEH-HEAVY | Ext | Breakthrough | 1,450 | veh_heavy | 245 | 2,000 | 32 | BLD-06 | TD, air, fuel cut | MBTs, bunkers |
| RD-ARM-06 | Mobile Anti-Air Gun | ARM | VEH-AA | Core | Flak + anti-infantry | 560 | veh_medium | 340 | 900 | 15 | BLD-05 | MBT, ATGM | Helicopters, infantry |
| RD-ARM-07 | Rocket Artillery Regiment | ARM | VEH-ARTY | Core | Area barrage | 420 | veh_light | 310 | 1,200 | 20 | BLD-05 | Counter-battery, air | Infantry masses, SAM sites |
| RD-ARM-08 | Heavy Self-Propelled Gun | ARM | VEH-ARTY | Ext | Counter-battery, siege | 520 | veh_light | 270 | 1,600 | 26 | BLD-06 | Air, raids | Artillery, bunkers |
| RD-ARM-09 | Mortar Carrier | ARM | VEH-ARTY | Ext | Mobile mortar | 520 | veh_light | 340 | 900 | 16 | BLD-05 | MBT, AT | Infantry in cover |
| RD-ARM-10 | Flame Assault Vehicle | ARM | VEH-HEAVY | Ext | Fortification assault | 900 | veh_medium | 300 | 1,300 | 20 | BLD-05 | Tanks, ATGM | Garrisons, bunkers |
| RD-ARM-11 | Mine-Clearing Tank | ARM | VEH-ENGINEER | Ext | Breaching | 1,000 | veh_heavy | 280 | 1,000 | 18 | BLD-05 | Any AT | Minefields |
| RD-ARM-12 | Defensive Entrenchment Tank | ARM | VEH-MBT | Ext | Dug-in defense | 760 | veh_heavy | 300 | 950 | 18 | BLD-05 | Artillery, flanks | Frontal assaults |
| RD-ARM-13 | Field Anti-Air Missile Carrier | ARM | VEH-AA | Ext | Radar-linked SAM | 520 | veh_light | 320 | 1,050 | 17 | BLD-05 | Raids, jamming | Jets, bombers |
| RD-ARM-14 | Armored Siege Command Tank | ARM | VEH-HEAVY | Cond | Super-heavy hub (cap 1) | 2,600 | veh_heavy | 170 | 4,200 | 55 | BLD-06 | Air, massed ATGM | Fortifications, lines |
| RD-ARM-15 | Veteran Reserve Tank | ARM | VEH-MBT | Ext | Cheap old MBT | 700 | veh_medium | 320 | 600 | 14 | BLD-05/17 | Modern MBTs | Light vehicles |
| RD-SUP-01 | Industrial Constructor | SUP | VEH-CONSTRUCTOR | Core | Construction | 900 | veh_light | 230 | 2,200 | 35 | BLD-01/05 | Anything armed | — |
| RD-SUP-02 | Rail Supply Tractor | SUP | VEH-LOGISTICS | Ext | Rail-to-depot goods | 480 | veh_soft | 360 | 450 | 10 | BLD-11 | Raiders | — |
| RD-SUP-03 | Bulk Fuel Transport | SUP | VEH-LOGISTICS | Ext | Fuel delivery | 420 | veh_soft | 360 | 420 | 10 | BLD-11/09 | Raiders, air | — |
| RD-SUP-04 | Mobile Repair Crew | SUP | VEH-LOGISTICS | Core | Repair, salvage | 600 | veh_light | 330 | 700 | 12 | BLD-05/17 | Artillery | — |
| RD-SUP-05 | Engineer Bridging Vehicle | SUP | VEH-ENGINEER | Ext | Bridge layer | 850 | veh_heavy | 280 | 1,000 | 18 | BLD-04 | Any AT | River barriers |
| RD-SUP-06 | Electronic Countermeasure Vehicle | SUP | VEH-EW | Ext | Jamming, raid alert | 400 | veh_light | 380 | 950 | 15 | BLD-05 | Emitter-cued strikes | Guided weapons |
| RD-SUP-07 | Reserve Reinforcement Carrier | SUP | VEH-TRANSPORT | Ext | Muster, replenish | 620 | veh_light | 380 | 800 | 14 | BLD-05 | AT, artillery | — |
| RD-AIR-01 | Frontline Interceptor | AIR | AIR-FIGHTER | Ext | Air-to-air | 360 | air_fixed | 2,400 | 1,550 | 21 | BLD-07 | SAMs | Bombers, transports |
| RD-AIR-02 | Tactical Attack Aircraft | AIR | AIR-STRIKE | Ext | Ground strike | 560 | air_fixed | 1,600 | 1,900 | 25 | BLD-07 | AA, fighters | Columns, artillery |
| RD-AIR-03 | Close-Support Gunship | AIR | AIR-HELI-ATTACK | Core | Attack heli (90 s) | 560 | air_rotary | 720 | 1,450 | 20 | BLD-07 | AA guns, MANPADS | Light vehicles, infantry |
| RD-AIR-04 | Assault Transport Helicopter | AIR | AIR-HELI-TRANSPORT | Ext | Air assault | 620 | air_rotary | 680 | 1,150 | 17 | BLD-07 | All AA | — |
| RD-AIR-05 | Heavy Transport Aircraft | AIR | AIR-TRANSPORT | Ext | Paradrop, airlift | 700 | air_fixed | 1,500 | 1,600 | 26 | BLD-07 | Fighters, SAMs | — |
| RD-AIR-06 | Radar Reconnaissance Aircraft | AIR | AIR-SURVEILLANCE | Ext | AEW (cap 1) | 520 | air_fixed | 1,500 | 2,100 | 28 | BLD-07 | Fighters | — |
| RD-AIR-07 | Frontline Bomber | AIR | AIR-STRIKE | Ext | Carpet bombing | 700 | air_fixed | 1,600 | 2,400 | 32 | BLD-07 | Fighters, SAM nets | Infantry blocks, bases |
| RD-AIR-08 | Artillery Targeting Helicopter | AIR | AIR-SCOUT | Ext | Fire correction | 320 | air_rotary | 760 | 800 | 14 | BLD-07 | AA | — (multiplier) |
| RD-AIR-09 | Strategic Missile Transporter | AIR | VEH-ARTY | Cond | Conventional theater missile (cap 2) | 620 | veh_light | 260 | 2,600 | 40 | BLD-06 | Raids, air | Static high-value sites |
| RD-SEA-01 | River Patrol Boat | SEA | NAV-PATROL | Cond | River patrol | 420 | nav_light | 560 | 700 | 14 | BLD-08 | Missile boats, air | Boats, barges |
| RD-SEA-02 | Amphibious Motor-Rifle Carrier | SEA | VEH-IFV | Ext | River crossing APC | 520 | veh_light | 400 | 750 | 14 | BLD-05/08 | AT (in water) | Bank infantry |
| RD-SEA-03 | Coastal Torpedo Boat | SEA | NAV-PATROL | Cond | Torpedo ambush | 340 | nav_light | 700 | 800 | 14 | BLD-08 | Patrol craft, helis | Unescorted ships |
| RD-SEA-04 | Conventional Attack Submarine | SEA | NAV-SUB | Cond | Sea denial | 950 | nav_heavy | 340 | 1,800 | 40 | BLD-08 | Sonar ships, MPA | Surface ships |
| RD-SEA-05 | Anti-Air Escort Destroyer | SEA | NAV-COMBATANT | Cond | Fleet AA | 2,000 | nav_heavy | 340 | 2,600 | 45 | BLD-08 | Anti-ship salvoes | Aircraft over water |
| RD-SEA-06 | Shore Bombardment Cruiser | SEA | NAV-COMBATANT | Cond | Shore siege (cap 1) | 2,600 | nav_heavy | 300 | 3,200 | 55 | BLD-08 | Subs, anti-ship | Coastal defenses |
| RD-SEA-07 | Amphibious Headquarters Vessel | SEA | NAV-TRANSPORT | Cond | Floating depot (cap 1) | 2,200 | nav_heavy | 320 | 2,800 | 50 | BLD-08 | Subs, air | — |

**Price validator check (§1):** RD-INF-01 $175 × 1.5 = $263 < MP-INF-12 $270, MP-INF-01 and AC-INF-01; RD-INF-02 $210 × 1.5 = $315 < AC-INF-02 $380; RD-INF-03 $260 × 1.5 = $390 < AC-INF-04 $400 and IA-INF-03 $420; RD-INF-05 $340 × 1.5 = $510 < MP-INF-04 $520; RD-INF-04 $300 × 1.5 = $450 < AC-INF-03 $560; RD-INF-06 $240 × 1.5 = $360 < AC-INF-06 $450; RD-INF-09 $420 × 1.5 = $630 < AC-INF-08 $650; RD-INF-11 $640 × 1.5 = $960 < AC-INF-11 $1,300; RD-INF-13 $1,250 × 1.5 = $1,875 < AC-INF-14 $2,400. The Iron Ascendancy and Meridian files must keep their comparable roles above these products (cross-file check in the [matrices](../UNIT_BALANCE_AND_ABILITY_MATRICES.md)).

## 9. Abilities and upgrades index

| ID | Unit(s) | Kind | Cost | Cooldown | Range | Requires | Counters |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.rd.inf_01.mass_assault` | RD-INF-01 | Active | free | 40 s | self | ≥ 4 squads in 800 cm | HMG, artillery, flame |
| `ability.rd.inf_02.mounted_drill` | RD-INF-02 | Passive | free | — | self | — | Kill the carrier |
| `ability.rd.inf_03.suppressive_fire` | RD-INF-03 | Active | free | 25 s | 900 cm | Deployed | Smoke, flanking |
| `ability.rd.inf_05.smoke_rounds` | RD-INF-05 | Active | 2 ammo | 35 s | 350–1900 cm | Deployed | Detectors |
| `ability.rd.inf_06.improve_bridge` | RD-INF-06 | Active | $150 | — | 200 cm | Jurisdiction (E) | Artillery |
| `ability.rd.inf_06.clear_obstacle` | RD-INF-06 | Active | free | — | 150 cm | — | Covering fire |
| `ability.rd.inf_07.depot_defense` | RD-INF-07 | Conditional | free | — | 800 cm | Near depot or rail | Fight away from depots |
| `ability.rd.inf_08.decontaminate` | RD-INF-08 | Active | $120 | 30 s | 300 cm | — | Repeated contamination |
| `ability.rd.inf_08.protective_posture` | RD-INF-08 | Passive aura | free | — | 300 cm | — | Small arms |
| `ability.rd.inf_09.spot_for_artillery` | RD-INF-09 | Active | free | 25 s | 1600 cm | Observation | Smoke, LOS break |
| `ability.rd.inf_09.ambush_volley` | RD-INF-09 | Active | free | 30 s | weapon | Concealed | Detectors |
| `ability.rd.inf_10.political_presence` | RD-INF-10 | Passive aura | free | — | 900 cm | — | Kill officer, jamming |
| `ability.rd.inf_10.rally` | RD-INF-10 | Active | free | 60 s | 900 cm | — | Re-suppression |
| `ability.rd.inf_11.steal_intelligence` | RD-INF-11 | Active | $170 | 60 s | contact | Eligible building | DETECT.SPY |
| `ability.rd.inf_12.divert_resources` | RD-INF-12 | Channel | $150 | 120 s | contact | `tech.rd.resource_diversion_doctrine` | DETECT.SPY, audit |
| RD-INF-13 talents (12 options) | RD-INF-13 | Talent | free | 40–240 s | radius 1400 cm | Owner approval (talent doc §5) | Marksmen, jamming |
| `ability.rd.arm_02.ammunition_load` | RD-ARM-02 | Passive | §10.4 | — | 400 cm | — | Kill truck |
| `ability.rd.arm_05.reactive_armor` | RD-ARM-05 | Passive charges | — | 20 s per charge | self | — | KINETIC_AP |
| `ability.rd.arm_05.fuel_hungry` | RD-ARM-05 | Conditional | — | — | self | Fuel starvation | Fuel interdiction |
| `ability.rd.arm_07.coordinated_barrage` | RD-ARM-07 | Active | free | per salvo | 3800 cm | ≥ 3 launchers | Counter-battery |
| `ability.rd.arm_08.counter_battery_fire` | RD-ARM-08 | Conditional | — | — | 4200 cm | Battery contact | Displacement |
| `ability.rd.arm_09.smoke_barrage` | RD-ARM-09 | Active | 3 ammo | 40 s | 500–2400 cm | — | Detectors |
| `ability.rd.arm_11.plough` | RD-ARM-11 | Passive | free | — | 150 cm | — | AT fire |
| `ability.rd.arm_11.breach_line` | RD-ARM-11 | Active | $100 | 60 s | 1200 cm | — | Covering fire |
| `ability.rd.arm_12.dig_in` | RD-ARM-12 | Active | free | — | self | — | Artillery, flanking |
| `ability.rd.arm_13.radar_link` | RD-ARM-13 | Conditional | free | — | 2000 cm | Friendly air radar | Kill or jam radar |
| `ability.rd.arm_14.field_command_post` | RD-ARM-14 | Passive aura | §10.4 | — | 1200 cm | — | Kill it, jamming |
| `ability.rd.arm_14.siege_mode` | RD-ARM-14 | Active | free | 30 s | 1400 cm | — | Air, ATGM |
| `ability.rd.sup_01.deploy_works` | RD-SUP-01 | Active | free | — | 2500 cm radius | Jurisdiction (E) | Attack during unfold |
| `ability.rd.sup_02.rail_shuttle` | RD-SUP-02 | Automated | §10.4 | — | route | RD-BLD-11 | Interdiction |
| `ability.rd.sup_03.deliver_fuel` | RD-SUP-03 | Automated | free | — | route | RD-BLD-09/11 | Interdiction |
| `ability.rd.sup_05.lay_bridge` | RD-SUP-05 | Active | free | 30 s | 1600 cm span | Authored gap | Artillery, anchor kill |
| `ability.rd.sup_05.restore_crossing` | RD-SUP-05 | Active | $150 | — | 200 cm | Authored bridge | Artillery |
| `ability.rd.sup_06.jamming_field` | RD-SUP-06 | Toggle | free | — | 1200 cm | Mast deployed | Emitter detection |
| `ability.rd.sup_06.raid_alert` | RD-SUP-06 | Passive | free | — | 2600 cm | — | Jamming |
| `ability.rd.sup_07.field_muster` | RD-SUP-07 | Active | unit price | build time | 800 cm of BLD-03/11 | — | Destroy RD-BLD-11 |
| `ability.rd.sup_07.forward_replenishment` | RD-SUP-07 | Active | §10.3 × 1.2 | — | 600 cm | Stationary | Kill carrier |
| `ability.rd.air_03.short_patrol` | RD-AIR-03 | Passive limit | — | 90 s loiter | — | — | — |
| `ability.rd.air_04.sling_resupply` | RD-AIR-04 | Active | §10.4 | — | 400 cm | — | AA |
| `ability.rd.air_05.air_bridge` | RD-AIR-05 | Active | free | — | owned depot | — | Interceptors |
| `ability.rd.air_06.raid_warning` | RD-AIR-06 | Passive | free | — | 4500 cm | — | Jamming, fighters |
| `ability.rd.air_08.fire_correction` | RD-AIR-08 | Active | free | 30 s | 2200 cm | Observation | AA, smoke |
| `ability.rd.air_09.erect_and_launch` | RD-AIR-09 | Active | $600 reload | one missile | 3000–9000 cm | `tech.rd.theater_missile_program` | Raids during erection |
| `ability.rd.sea_02.river_crossing` | RD-SEA-02 | Passive | — | — | self | Authored banks | Fire in water |
| `ability.rd.sea_07.floating_depot` | RD-SEA-07 | Passive | §10.3/§10.4 | — | 800 cm | — | Sinking |
| `ability.rd.sea_07.limited_mission` | RD-SEA-07 | Passive limit | — | — | — | — | Blockade |
| `tech.rd.industrial_mobilization_methods` | RD-INF-01/02/07, RD-SUP-07 musters | Faction tech (E candidate) | $700 | — | — | RD-BLD-02 | Lane block 60 s |
| `tech.rd.coordinated_defensive_fire` | RD-INF-03/05, RD-ARM-07/08/12 | Faction tech (E candidate) | $600 | — | — | RD-BLD-02, RD-DEF-06 powered | Tower must be powered |
| `tech.rd.tandem_warheads` | RD-INF-04 | Unit tech | $600 | — | — | RD-BLD-02 | Cooldown +10 % |
| `tech.rd.collective_protection` | RD-INF-08 | Unit tech | $500 | — | — | RD-BLD-02 | Price +$40, speed −5 % |
| `tech.rd.recon_field_training` | RD-INF-09 | Unit tech | $500 | — | — | RD-BLD-02 | Build time +2 s |
| `tech.rd.mass_political_education` | RD-INF-10 | Unit tech | $700 | — | — | RD-BLD-16 | Officer cap 4 → 3 |
| `tech.rd.late_espionage` | RD-INF-11 | Espionage gate (I flag) | $1,200 | — | — | RD-BLD-02 | — |
| `tech.rd.courier_networks` | RD-INF-11 | Unit tech | $600 | — | — | RD-BLD-02 | Cap 4 → 3 |
| `tech.rd.resource_diversion_doctrine` | RD-INF-12 | Unlock | $900 | — | — | RD-BLD-02 | — |
| `tech.rd.motorized_reconnaissance` | RD-ARM-01 | Unit tech | $300 | — | — | RD-BLD-02 | Price +$20 |
| `tech.rd.applique_armor` | RD-ARM-03/04/15 | Unit tech | $500 | — | — | RD-BLD-05 | Speed −5 % |
| `tech.rd.standardized_hull_production` | RD-ARM-03/04/15 | Faction tech (E candidate: standardized components) | $900 | — | — | RD-BLD-05 | Lane block 60 s |
| `tech.rd.engine_efficiency_program` | RD-ARM-05 | Unit tech | $900 | — | — | RD-BLD-02 | Speed −5 % |
| `tech.rd.extended_rocket_motors` | RD-ARM-07 | Unit tech | $800 | — | — | RD-BLD-02 | Scatter +15 % |
| `tech.rd.siege_command_program` | RD-ARM-14 | Unlock | $2,000 | — | — | RD-BLD-02 | Lane block 120 s |
| `tech.rd.rail_scheduling` | RD-SUP-02 | Unit tech | $500 | — | — | RD-BLD-11 | Power +10 % |
| `tech.rd.repair_capacity` | RD-SUP-04 | Faction tech (E candidate) | $600 | — | — | RD-BLD-17 | Price +$80 |
| `tech.rd.heavy_bridging` | RD-SUP-05 | Unit tech | $600 | — | — | RD-BLD-04 | Deploy +4 s |
| `tech.rd.theater_missile_program` | RD-AIR-09 | Unlock | $1,800 | — | — | RD-BLD-02 | Lane block 120 s |

## 10. Faction tactical coverage, gaps and balance risks

**Coverage per role.**

| Role | Coverage | Units |
| --- | --- | --- |
| Line infantry | Excellent (cheap mass) | RD-INF-01/02/07 |
| Anti-armor | Good, unguided | RD-INF-04, RD-ARM-04/05, RD-AIR-02/03 |
| Anti-infantry | Excellent | RD-INF-03/05, RD-ARM-06/07/09/10, RD-AIR-07 |
| Anti-air | Excellent, layered and networked | RD-INF-04 (limited), RD-ARM-06/13, RD-AIR-01/06, RD-SEA-05 |
| Artillery | Excellent | RD-INF-05, RD-ARM-07/08/09, RD-AIR-09, RD-SEA-06 |
| Recon and detection | Adequate | RD-ARM-01, RD-INF-09, RD-AIR-06/08, RD-SUP-06; `DETECT.SPY` only from RD-INF-07 |
| Logistics | Deep but fragile | RD-SUP-02/03/04/07, RD-AIR-04/05, RD-SEA-07 |
| Engineering | Excellent | RD-INF-06, RD-ARM-11, RD-SUP-05 |
| Espionage | Specialized (theft and diversion) | RD-INF-11/12 |
| Naval | Conditional, complete | RD-SEA-01 to 07 |

**Weak spots and intended answers.**

1. *No guided anti-tank weapon or tank destroyer.* Intended: mass RD-ARM-04 frontage, RD-INF-04 numbers and RD-AIR-03 rockets. Concord ATGM range (AC-ARM-06) is the classic answer to RD armor; RD answers it with artillery (RD-ARM-07 against a soft VEH_MEDIUM/LIGHT carrier) rather than a like-for-like unit.
2. *Few `DETECT.SPY` sources.* RD-INF-07 is depot-bound, so enemy agents in forward bases are a real threat. Intended: RD-INF-09 hunting and garrison discipline; flagged as risk R3.
3. *Logistics dependence.* Interdiction of RD-SUP-02/03 and RD-BLD-11 starves RD-ARM-05 and the artillery parks (E: intended faction weakness).
4. *Individual unit quality.* Each RD unit loses its 1-v-1 against a Concord equivalent; the Directorate must fight concentrated (normal isolation penalty, E).

**Balance risks.**

| ID | Risk | Mitigation |
| --- | --- | --- |
| R1 | Cheap infantry + very-massive bonus + RD-INF-10 + RD-INF-13 Tier I stack into near-unbreakable blobs | Stress reductions capped at −30 % total (RD-INF-13 Tier I text); area weapons and flame remain hard counters |
| R2 | RD-ARM-07 batteries with RD-AIR-08 correction delete static defenses from out of range | Launch smoke tell, AmmoCapacity 2, counter-battery (AC-ARM-12), 900 cm minimum range |
| R3 | Agent pressure on forward RD bases with only depot-bound spy detection | Playtest; fallback: give RD-INF-10 a `DETECT.SPY` 300 cm (U-RD-03) |
| R4 | Food upkeep is the only brake on mass infantry; without a shortage penalty value (U-05) the soft cap is untested | Resolve U-05 before Core balance |
| R5 | RD-AIR-09 conventional missile overlaps the nuclear strategic layer emotionally | Launch warning, long erection, cap 2, and clear "conventional" UI labelling; U-13 |

**Open items.** Framework: U-01, U-03, U-04, U-05, U-06, U-07, U-09, U-11 (Directorate excluded from recovery), U-12, **U-13**. Faction-specific:

| ID | Decision | Recommendation |
| --- | --- | --- |
| U-RD-01 | Does the −45 % infantry price rule apply to the elite commander (RD-INF-13) and agents (RD-INF-11/12)? | Yes for consistency (E: "infantry far cheaper"); revisit if playtests show agent spam, which caps already limit |
| U-RD-02 | Resource diversion rates and per-operation caps | 4 % per 5 s, $400 per tick, $2,400 per operation; owner decision under Espionage §1.2/§2 |
| U-RD-03 | Additional spy detection for forward bases | Add `DETECT.SPY` 300 cm to RD-INF-10 if R3 materializes |
| U-RD-04 | Fuel-hungry surcharge value | +40 % production petroleum for heavy/super; RD-ARM-05 ×2.0 |
