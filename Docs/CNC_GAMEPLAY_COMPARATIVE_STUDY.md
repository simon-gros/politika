# C&C comparative gameplay and content study for POLITIKA

**Purpose:** extract design lessons from the locally installed/extracted Command & Conquer titles and translate them into POLITIKA-owned design guidance. This is an analytical reference, not an implementation specification and not a source of assets to ship.

**Evidence boundary:** UM extraction manifests establish which archives/files are available, not that every file was decoded or validated in-game. The classic Westwood formats expose more human-readable rules, map, and campaign data; later SAGE titles expose useful file taxonomies, map packages, UI manifests, and asset groupings, but much gameplay behavior is in proprietary/cooked formats. Conclusions below distinguish direct structural evidence from design synthesis. Paths are relative to `/mnt/data/CommandAndConquer/UM/` unless stated otherwise.

**Unit-system reference:** §5 adds directly inspected classic unit, armor, warhead, veterancy and acquisition data (TD/RA/TS/RA2/Generals) behind the [Unit Specification Framework](UNIT_SPECIFICATION_FRAMEWORK.md).

**Broader comparison:** This C&C-specific, extraction-limited study remains the owner of local C&C source observations. For the wider genre taxonomy (StarCraft, Tempest Rising, large-scale RTS, tactical military games and political/economic hybrids), see [Cross-Genre RTS Comparative Study](RTS_CROSS_GENRE_COMPARATIVE_STUDY.md) and the proposed [Competitive Benchmark Register](RTS_COMPETITIVE_BENCHMARK_REGISTER.md). Those references do not verify local proprietary extraction data or qualify POLITIKA runtime behavior.

## 1. Comparative lenses

| Title family | What the local extraction is especially good for studying | Design pattern to retain (at the level of principle) | POLITIKA application |
|---|---|---|---|
| **Tiberian Dawn / Red Alert** | Classic rule, scenario, map, briefing, unit art/audio and mission data; the Red Alert extraction includes classic MIX content. | A small, legible combat vocabulary can carry strategic depth when production choices, map routes, and timing interact. Missions can teach a rule through a concrete situation. | Keep a comprehensible shared baseline of infantry, armor, indirect fire, reconnaissance, and support. Let faction doctrine, logistics, intelligence, and terrain alter the decisions rather than multiplying near-identical units. Build campaign missions around one or two learnable strategic pressures. |
| **Tiberian Sun / Firestorm; Red Alert 2 / Yuri’s Revenge** | Large campaign/multiplayer map collections and classic data, including the RA2 `rulesmd.ini` and many `.map` files. | Distinct battlefield environments and map geometry make the same roster play differently; faction identity is readable through silhouette, animation, and battlefield role. | Make Balkan geography operational: river crossings, ridgelines, rail/road corridors, industrial sites, and dense settlements should create meaningful routes and exposure. Require faction units to remain distinguishable at gameplay zoom and in the 4K HUD. |
| **Generals / Zero Hour** | Readable INI/map-oriented content, map scripts/configuration, separate faction art/audio groups; Zero Hour adds a variant/commander-style expansion layer. | Unit/structure behavior and map scripting can be data-driven; faction specializations and selectable strategic variants add replayability, but can expand balance/test combinations rapidly. | Keep POLITIKA’s stable faction identity in its five rosters; express doctrines as a small set of bounded command doctrines or operational packages, not an extra full roster layer. Author map scenario rules as data, then validate them against the common match contract. |
| **Tiberium Wars / Kane’s Wrath** | SAGE-era grouped content (`Apt`, streams, terrain, WBData, maps, libraries) and broad campaign/multiplayer map packages. | A strong three-sided faction triangle can be readable, but subfactions and larger upgrade/ability sets raise the cost of learning, UI, and balance. | Preserve the asymmetric strategic roles in POLITIKA’s five factions while enforcing shared counterplay categories and a consistent command grammar. Make every specialist ability communicate cost, range, duration, counter, and strategic consequence. |
| **Red Alert 3 / Uprising** | Rich UI manifests, ability-button assets, campaign/tutorial/multiplayer map packages, and large separate audio/content groups. | Highly differentiated faction presentation and explicit secondary abilities can make unit identity immediate; water/land/air interaction creates multi-axis map choices. | Treat the Adriatic and major rivers as real operational spaces only where the map and scenario justify them. Give units a compact, inspectable ability set; make naval/riverine assets strategic choices rather than a mandatory parallel tech tree on every map. |
| **Tiberian Twilight (C&C4)** | Tactical HUD/shell manifests, map and WorldBuilder data, objective and unit-ability UI taxonomy. Its extraction report lists 20,001 files (~4.9 GiB) in original formats. | Clear objective presentation and contextual command UI can make a complex battlefield legible. The class/loadout and objective-driven direction is worth studying, but its reduced base-building/economy is a deliberate departure from POLITIKA’s intended RTS. | Borrow the *clarity* of objective, command, and selected-unit information—not the stripped economy or class/loadout rules. Retain POLITIKA’s base/economic/strategic layer and ensure objectives are subordinate to the match’s resource, territory, and political pressures. |
| **Renegade** | Partial surface/extraction only: the manifest reports no bulk-extracted MIX1 members. | A first-person perspective offers a useful contrast for scale, hit feedback, and how infantry/vehicles read up close, but it is not a reliable source for this top-down RTS content study. | Use only as an optional presentation reference; do not infer RTS unit balance or content schema from the incomplete extraction. |

## 2. Cross-title patterns translated into POLITIKA rules

### Units, weapons, and abilities

1. **Give each unit one primary battlefield job.** Secondary utility should create a choice or vulnerability, not erase the unit’s counter. POLITIKA roster entries should state role, target class, threat range, mobility, durability, cost/upkeep, counters, and unlock path before detailed stats are tuned.
2. **Share the combat grammar; vary the doctrine.** Across five factions, retain recognizable shared categories (line infantry, anti-armor, reconnaissance, direct-fire armor, indirect fire, air defense, logistics/engineering, command/support). Differentiate through access, deployment, information, sustainment, and tactical tradeoffs—not just larger damage numbers.
3. **Make abilities bounded and previewable.** Every active ability needs an explicit activation condition, target/range, duration, cooldown or charge, resource/ammunition cost, interruption/counter, and UI feedback. Avoid stacking hidden bonuses across faction, doctrine, morale, and terrain.
4. **Keep weapon-counter relationships teachable.** Show why a unit is effective or vulnerable in the HUD/log; test mixed-force engagements rather than isolated damage-per-second tables. Resolve, suppression, visibility, and terrain can modify outcomes, but must not obscure the base weapon role.
5. **Protect roster scale.** Treat proposed roster entries as a design pool, not a launch promise. Prototype a representative slice per faction, then add units only when they introduce a distinct decision and a clear counter.

### Factions and progression

- Maintain five asymmetric strategic identities, including the NPC Unaligned, while preserving cross-faction role equivalence sufficient for map validation and match fairness. The playable factions should not be forced into mirror rosters.
- Use the existing POLITIKA-specific strategic identity—Atlantic Concord expeditionary reach, Red Directorate state-industrial capacity, Iron Ascendancy coercive occupation, Meridian Pact commercial/logistical leverage, and Unaligned local/federal response—as the basis for equipment and mechanics. No C&C faction identity should be transplanted.
- Make progression expose meaningful tradeoffs: a player should choose between capabilities or timing windows, not simply unlock a strictly superior version of every unit. For doctrine variants, track the combinatorial balance and UI burden explicitly.
- Preserve counterplay for information warfare and coercion. Reconnaissance, counterintelligence, sabotage, propaganda, and morale effects should have tells, attribution uncertainty where intended, response options, and recoverable consequences.

### Maps and scenarios

- Evaluate maps by **decision topology**, not visual novelty alone: routes, chokepoints, flank paths, high ground/line of sight, cover, bridges/crossings, resource access, buildable area, reinforcement paths, and objective approach time.
- Ensure asymmetry is intentional and tested. Starting positions need comparable strategic opportunity, not necessarily identical terrain; faction-specific deployment constraints must not silently make a starting slot unusable.
- Separate multiplayer map rules from campaign scripting. Scripted objectives and reinforcements may create authored exceptions, but must be declared in the map profile and tested for a valid completion path.
- Include 1v1, team, and 2–4-player layouts where supported; test resource contest timing, reachability, travel times, and pathing at representative force sizes. Water should be a deliberate strategic axis, not decorative access that only one roster can exploit.

### User interface and feedback

- Keep the shared command loop stable: select → inspect role/state → issue order/ability → receive immediate acknowledgement → see ongoing status and counterplay.
- Make selected-unit information contextual, but put common commands and critical state (health, suppression/resolve, ammunition or cooldown, orders, veterancy/progression where used) in predictable locations.
- Distinguish objective state, unit ability readiness, resource changes, and intelligence confidence with more than color alone. Support readable scaling and input at 4K and at lower resolutions.
- Present the strategic map and tactical HUD as complementary views. A minimap marker must not be the only indication of a critical event; provide accessible text/icon/audio cues and configurable event emphasis.

## 3. Concrete POLITIKA study and prototype agenda

Use the existing design owners rather than duplicating mechanics here:

1. **Roster pass:** annotate each candidate in [POLITIKA_FIVE_FACTION_ROSTERS.md](POLITIKA_FIVE_FACTION_ROSTERS.md) with unique decision, role, counters, and unlock/economic burden. Remove or merge entries that fail the unique-decision test. *First proposed pass (29 October 2026): the [Unit Specification Framework](UNIT_SPECIFICATION_FRAMEWORK.md) and the per-faction files in [`Units/`](Units/) specify all 268 roster units, assign Core/Extended/Conditional/Scenario tiers and record overlaps as unresolved items. They do not delete roster entries. See §5 for the classic-data basis.*
2. **Combat slice:** prototype one line infantry, specialist, scout, tank, artillery, air-defense, logistics, and support unit per representative faction pair. Measure whether role and counters are understandable from the HUD; then expand to the five factions.
3. **Map set:** select one compact urban/industrial map, one river-crossing map, one mountainous corridor map, and one larger team map. Check spawn fairness, reachable objectives/resources, route diversity, time-to-contact, build space, and Unaligned intervention rules using the [map validation contract](ENGINE_OUTLINE.md#map-authoring-contract-and-validation-pass).
4. **Ability and UI pass:** take a representative ability from each strategic family (recon, sabotage, morale/information, logistics, direct combat), and verify preview → commit → feedback → counterplay in the [POLITIKA UI overview](POLITIKA_UI_OVERVIEW.md).
5. **Scenario pass:** compare a campaign/tutorial mission against multiplayer rules to ensure the tutorial teaches systems that remain true in the main game; document intentional exceptions.

These are design-study recommendations, not claims that the prototypes or validations have been implemented.

## 4. Source map and limits

The canonical local index is [`UM/INDEX.md`](</mnt/data/CommandAndConquer/UM/INDEX.md>); the generated [file-level reference catalog](</mnt/data/CommandAndConquer/UM/REFERENCE_CATALOG.md>) filters extracted members by title, source archive, extension, and heuristic purpose, with the complete JSONL records alongside it. Per-title extraction manifests enumerate extracted archives and explain limits. Representative local evidence includes:

- Classic rules/maps: `Command & Conquer Red Alert II/extracted/expandmd01/rulesmd.ini`, `Command & Conquer Red Alert II/extracted/MULTI/*.map`, and `Command & Conquer Tiberian Sun/extracted/multi/*.map`.
- Generals data/maps: `Command and Conquer Generals/extracted/INI/Data/INI/Object/*.ini`, `.../INI/Data/INI/GameData.ini`, and `.../maps/maps/*/map.ini`; Zero Hour’s manifest records `INIZH.big` and its extracted data groups.
- SAGE maps/UI: `Command and Conquer 3 Tiberium Wars/extracted/SP/1.8/patch8/data/maps/official/`, `Command and Conquer Red Alert 3/extracted/Data/Maps*/data/maps/official/`, and `Command and Conquer Red Alert 3/extracted/Data/Apt/data/aptui/*ability*`.
- C&C4: `Command and Conquer 4 Tiberian Twilight/extracted/Apt/data/aptui/`, `.../WBData/data/worldbuilder.manifest`, and the per-title extraction manifest.
- `Command & Conquer Renegade/extraction-manifest.md` records the incomplete MIX1 extraction; absence of bulk-extracted files is not evidence that the game lacks those systems.

For detailed mechanics, pair extracted data with official manuals/tutorials or in-game observation. Some proprietary formats are not decoded by UM, and extracted files alone do not establish runtime behavior. See also [C&C extraction report index](</mnt/data/CommandAndConquer/UM/INDEX.md>) and [archive rights and provenance](../Archive/RIGHTS_AND_PROVENANCE.md). Keep extracted assets local; this study records design observations and does not copy game assets into POLITIKA.

## 5. Classic unit-system reference for the POLITIKA unit specifications

**Added 29 October 2026.** This section is the comparative design reference behind the [Unit Specification Framework](UNIT_SPECIFICATION_FRAMEWORK.md) and the per-faction unit files in [`Units/`](Units/). It records **directly inspected data** from the local classic titles and what POLITIKA does with each principle. Values are quoted only to analyse design relationships. No C&C statistic, unit identity, description or asset is copied into POLITIKA content, and every POLITIKA number is an original Provisional Baseline. Paths are relative to `/mnt/data/CommandAndConquer/` and stay outside the repository.

### 5.1 Evidence inspected

| Title | File (SHA-256 prefix) | Evidence class | What was read |
| --- | --- | --- | --- |
| Tiberian Dawn | `CnC_Tiberian_Dawn/CONST.CPP` (`76f13bad…`), `DEFINES.H` (EA GPL v3 source release, see `CnC_Red_Alert/LICENSE.md`) | source-inspected | Warhead table `Warheads[WARHEAD_COUNT]`; armor enumeration `ARMOR_NONE/WOOD/ALUMINUM/STEEL/CONCRETE` |
| Red Alert | `UM/Command & Conquer Red Alert/extracted/REDALERT/LOCAL.MIX` (`6b308392…`, embedded rules text) | data-inspected | Warheads `[SA] [HE] [AP] [Fire] [HollowPoint] [Super] [Organic] [Nuke]`; units `E1 E2 E3 E4 E6 E7 SPY THF MEDI DOG 1TNK 2TNK 3TNK 4TNK ARTY V2RL JEEP APC MNLY MRJ MGG SS DD CA PT LST MIG`; weapons |
| Tiberian Sun | `UM/Command & Conquer Tiberian Sun/extracted/patch/rules.ini` (`79c2251b…`) | data-inspected | `[General]` veterancy keys; units `E1 E2 E3 BIKE TTNK MMCH ART2 STNK SAPC GHOST MEDIC LPST CYBORG JUMPJET` |
| Red Alert 2 / Yuri's Revenge | `UM/Command & Conquer Red Alert II/extracted/expandmd01/rulesmd.ini` (`3d341ef8…`) | data-inspected | Veterancy, refund and repair keys; `Verses` armor order and acquisition notes; warheads `SA HE AP Fire HollowPoint FlakWH DeathWH` |
| Generals | `UM/Command and Conquer Generals/extracted/INI/Data/INI/Armor.ini` (`b0ef0f1a…`), `GameData.ini` (`69e8acfb…`), `Object/FactionUnit.ini` (`de5ee857…`) | data-inspected | Armor templates; veterancy health and weapon bonuses; `BuildCost`, `BuildTime`, `MaxHealth`, `ExperienceValue` and `ExperienceRequired` of representative units |

Limits: these files establish authored data, not observed runtime behavior. RA rules were read from the embedded INI text, not from a running game. Where this section names a well-known play pattern (for example "engineer rush"), it is a design-risk characterization, not a measured claim.

### 5.2 Armor and damage systems across titles

**Tiberian Dawn** (source): five armor classes. Warhead modifiers are stored in 1/256 units, converted here to percent in the order none / wood / light ("aluminum") / heavy ("steel") / concrete:

| Warhead | none | wood | light | heavy | concrete | Source comment |
| --- | ---: | ---: | ---: | ---: | ---: | --- |
| SA (small arms) | 100 | 50 | 56 | 25 | 25 | "good against infantry" |
| HE | 88 | 75 | 56 | 25 | 100 | "good against buildings & infantry" |
| AP | 25 | 75 | 75 | 100 | 50 | "good against armor" |
| Fire | 88 | 100 | 69 | 25 | 50 | "good against flammables" |
| Hollow point (sniper) | 100 | 3 | 3 | 3 | 3 | "Sniper bullet type" |

**Red Alert** (`Verses=` in the same order): `SA 100/50/60/25/25`, `HE 90/75/60/25/100`, `AP 30/75/75/100/50`, `Fire 90/100/60/25/50`, `HollowPoint 100/5/5/5/5`, `Organic 100/0/0/0/0` (infantry only; dogs and healing), `Super 100` (special cases), `Nuke` = fire profile with wall and ore damage.

**Red Alert 2 / Yuri's Revenge** widens the matrix to 11 columns: none, flak, plate (infantry); light, medium, heavy (vehicles); wood, steel, concrete (buildings); special_1 and special_2. Examples: `SA 100/80/80/50/25/25/75/50/25`, `AP 25/25/15/75/100/100/65/45/60`, `Fire 600/500/200/60/30/5/150/100/2`, `HollowPoint 200/100/100/1/1/1/1/1/1`, `FlakWH 150/80/50/100/100/20/0/0/0` ("no buildings"). The file documents **acquisition semantics** in comments: 0 % means "no force fire, no retaliate, no passive acquire", 1 % means "no retaliate, no passive acquire" and 2 % means "no passive acquire". RA2 also records `ProneDamage` (for example SA 70 %, Fire 600 %) for prone infantry.

**Generals** replaces fixed columns with **armor templates** that list percentages per damage type. Examples: `HumanArmor` takes ARMOR_PIERCING 10 %, FLAME 150 %, SNIPER 200 % and CRUSH 200 %. `TankArmor` takes SMALL_ARMS 25 %, GATTLING 10 %, FLAME 25 %, SNIPER 1 %, MELEE 0 % and CRUSH 50 %. `TruckArmor` takes SMALL_ARMS 50 %. `AirplaneArmor` takes SMALL_ARMS 120 % and JET_MISSILES 25 % ("aircraft do less damage to one another in the skies"). `StructureArmor` takes SMALL_ARMS 50 %, GATTLING 10 %, LAND_MINE 0 % and AURORA_BOMB 250 %. `UpgradedTankArmor` is a separate template, so armor upgrades swap the template.

**Observed principles → POLITIKA:**

1. Every title keeps a **small number of armor classes** (5 → 11). Damage types grow with the roster. POLITIKA uses 12 armor classes and 11 damage types ([framework §6](UNIT_SPECIFICATION_FRAMEWORK.md)), comparable to RA2 and much smaller than Generals' long damage-type list.
2. **Hard counters are encoded as near-zero cells** (TD and RA hollow point at 3–5 % against vehicles; Generals SNIPER 1 % against tanks). POLITIKA keeps that legibility, using 0 % for ineligible targets and 1–14 % for nuisance fire.
3. **RA2's 0/1/2 % comments are an elegant acquisition rule.** POLITIKA adopts the concept as three bands (0 % rejected, 1–14 % no auto-acquire or retaliation, ≥ 15 % normal).
4. **Extreme cells create binary fights.** RA2 Fire is 600 % against unarmored infantry and 2 % against concrete. POLITIKA caps any cell at 160 % so composition still matters.
5. **Upgrade by template swap** (Generals `UpgradedTankArmor`) is cleaner than many stacked percentages. POLITIKA research modifies effective values through one modifier pipeline ([framework §18.4](UNIT_SPECIFICATION_FRAMEWORK.md)) and never mutates definitions.

### 5.3 Representative unit data and the curves it implies

| Title | Unit (role) | HP / armor | Speed | Cost | Weapon (damage, ROF frames, range cells, warhead) | Lesson |
| --- | --- | --- | ---: | ---: | --- | --- |
| RA | E1 rifle infantry | 50 / none | 4 | 100 | M1Carbine 15, 20, 3, SA | Cheap baseline; many units per tank |
| RA | E3 rocket soldier | 45 / none | 3 | 300 | RedEye (AA) 50, 50, 7.5 + Dragon (AT) 35, 50, 5, AP | One infantry body, two target layers |
| RA | 1TNK light tank | 300 / heavy | 9 | 700 | 75mm 25, 40, 4, AP | Speed buys survival |
| RA | 2TNK medium tank | 400 / heavy | 8 | 800 | 90mm 30, 50, 4.75 | +33 % HP for +14 % cost over the light tank |
| RA | 3TNK heavy tank | 400 / heavy | 7 | 950 | 105mm ×2 | A second barrel, not more HP |
| RA | 4TNK mammoth | 600 / heavy | 4 | 1700 | 120mm burst 2 + MammothTusk HE missiles | Slow two-weapon spearhead |
| RA | ARTY | 75 / light | 6 | 600 | 155mm 150, 65, 6, HE (ballistic) | **Glass cannon**: 75 HP, long range |
| RA | V2RL | 150 / light | 7 | 700 | SCUD 600, 400, 10, HE; `Ammo=1` | Huge alpha, one shot, then reload |
| RA | JEEP / APC | 150 light / 200 heavy | 10 / 10 | 600 / 800 | M60mg 15, 20, 4, SA; APC `Passengers=5` | Fast patrol; protected taxi |
| RA | MNLY minelayer | 100 / heavy | 9 | 800 | `Ammo=5` mines | Area denial as limited ammunition |
| RA | E6, SPY, THF | 25 / none | 4 | 500 | `Infiltrate=yes` | Building interaction is a role |
| RA | E7 commando | 100 / none | 5 | 1200 | Colt45 50, HollowPoint ×2; `C4=yes` | Hero infantry; instant building kills |
| RA | MEDI | 80 / none | 4 | 800 | Heal −50 (Organic) | Healing as a negative-damage weapon |
| RA | MIG | 50 / light | 20 | 1200 | Maverick 50 AP ×2; `Ammo=3` | Sortie aircraft return to rearm |
| TS | BIKE / TTNK / MMCH | 150 wood / 350 light / 400 heavy | 12 / 6 / 4 | 600 / 800 / 800 | missiles / 90mm / 120mm | Speed and armor trade at an equal price |
| TS | ART2 / STNK / LPST | 300 light / 180 light / 600 wood | 5 / 6 / 6 | 975 / 1100 / 950 | 155mm / Dragon, `Cloakable` / none, sight 10 | Deployable artillery; stealth; mobile sensor |
| Generals | Ranger / Red Guard | 180 / 120 (Human) | — | 225 / 300 | — | Build time 5 s / 10 s |
| Generals | Crusader / Overlord | 480 / 1100 (Tank) | — | 900 / 2000 | — | 10 s / 25 s; ExperienceValue 100 / 200 |

Curves POLITIKA keeps (as shapes, not values): the MBT ladder buys **one** distinct attribute per step (speed, a second gun, a second weapon type). Artillery is fragile and needs spotting. Ammunition-limited weapons create logistics value (V2, minelayer, MIG). Agents and engineers are fragile and high-value. The POLITIKA translation is in [framework §4.3](UNIT_SPECIFICATION_FRAMEWORK.md) and the per-unit bands.

### 5.4 Veterancy across titles

| Title | Rule (data) | POLITIKA |
| --- | --- | --- |
| TS | `VeteranRatio=10.0` (destroy 10× own value per level), `VeteranCombat=.25`, `VeteranSpeed=.30`, `VeteranArmor=.25`, `VeteranROF=.20`, `VeteranCap=2`; per-unit `EliteAbilities` (`SELF_HEAL`, `SENSORS`, `SCATTER`, `RADAR_INVISIBLE`, `EXPLODES`) | Elite flavor beyond flat buffs is adopted (Elite first aid and self-repair) |
| RA2 | `VeteranRatio=3.0`, `VeteranCombat=1.1`, `VeteranSpeed=1.2`, `VeteranArmor=1.5`, `VeteranROF=0.6`, `VeteranCap=2`; `VeteranAbilities` / `EliteAbilities` lists | The +50 % armor rank effect is a snowball risk. POLITIKA caps HP gains at +10 / 20 / 30 % |
| Generals | `HealthBonus_Veteran/Elite/Heroic` = 120 / 130 / 150 %; weapon bonus Veteran ROF 120 % and damage 110 %, Elite ROF 140 % and damage 120 %; `ExperienceRequired` (Crusader `0 200 300 600`) ≈ 2× `ExperienceValue` (`100`) for the first level | POLITIKA: 3 ranks at 2.0× / 4.5× / 8.0× own ExperienceValue (from `USD/10`), credited by damage share |
| Tempest Rising (tags) | `Definition.Status.Veterancy_Veteran` / `_Hardened` / `_Elite` | Three named ranks as a status ([framework §19](UNIT_SPECIFICATION_FRAMEWORK.md)) |

### 5.5 The twenty analysis dimensions

| # | Dimension | Classic pattern (evidence) | Why it worked / failure mode | POLITIKA translation | Disposition |
| --- | --- | --- | --- | --- | --- |
| 1 | Roles and specialization | One job per unit; E3 covers two layers with two weapons | Clear purchase decisions; the hybrid unit stays fragile | Every spec states a primary role, secondary roles and roster tier | adopt-concept |
| 2 | Infantry classes | Rifle, rocket, grenadier, flame, engineer, medic, spy, commando (RA) | Small vocabulary, distinct counters | `TPL-INF-*` templates; faction-unique specialists (Psy-Ops, Intimidation, diversion) | adopt + redefine |
| 3 | Vehicle armor, mobility, weapons | Light/medium/heavy/mammoth ladder with speed as the currency | Avoids "bigger is better" | TRACKED vs TRACKED_HEAVY, tracking-class damage reduction for fast units | adopt-concept |
| 4 | Air and naval | Sortie aircraft with `Ammo`; naval `Sensors` and submarines `Cloakable` | Bases and airfields become targets | FIXED_WING sortie model, rearm costs kerosene, naval map-limited, `DETECT.SUBMERGED` | adopt (U-04) |
| 5 | Damage types and effectiveness | TD/RA warheads, RA2 11 columns, Generals templates | Readable counters; extremes cause binary fights | 11 × 12 matrix capped at 160 % | adopt + redefine |
| 6 | Armor classes and resistances | 5 → 11 classes | Few classes keep it learnable | 12 classes | adopt-concept |
| 7 | Speed, terrain, maneuverability | Integer speed classes; tracked vs wheeled (`Tracked=yes`) | Terrain matters through the locomotor | Mobility classes with terrain multipliers (needs nav profiles, U-03) | adopt (U-03) |
| 8 | ROF, range, accuracy, reload | ROF frames; `Burst`; ballistic vs hitscan projectiles | Projectile type decides counterplay | Deterministic direct fire; seeded, server-only scatter for indirect | redefine |
| 9 | Special abilities | RA2 deploy modes; Generals command-set abilities | Bounded actives add decisions; too many add UI burden | Abilities with full parameter rows; 0–2 unit-specific per ordinary unit | adopt-concept |
| 10 | Veterancy | TS / RA2 / Generals (§5.4) | Rewards preservation; snowball risk | 3 ranks, modest bonuses, anti-farm | adopt + redefine |
| 11 | Prerequisites and progression | `Prerequisite=`, `TechLevel` | Simple gates; tech levels resemble ages | Building + `tech.*` prerequisites; no ages or eras (E) | redefine |
| 12 | Economic cost and value | Cost/HP/weapon curves (§5.3) | Price signals role | USD bands; CV per $ review tool; faction modifiers | adopt-concept |
| 13 | Counters and combat relationships | Warhead × armor; RA dogs against spies | Teachable rock-paper-scissors with soft edges | Hard, soft and situational counters in every unit spec; counter matrices | adopt |
| 14 | Stealth, detection, reconnaissance, EW | Gap generators (shroud), stealth tanks, mobile sensors, radar jammers | Information war is powerful; invisibility frustrates | Concealment and disguise with detector classes and tells; **no invisibility** (E) | redefine / exclude invisibility |
| 15 | Repair, resupply, transport, logistics | Service depots, mobile repair, APC passengers, `Ammo` | Logistics creates targets | Repair 25 % of cost; resupply aura; manifests owner-only | adopt-concept |
| 16 | Morale, suppression, formations | Generals and C&C3 suppression; C&C3 stances | Pinning creates tactical tempo | One `UnitResolve` with tiers (E); stances; formations owned elsewhere | adopt (E) |
| 17 | Faction asymmetry | GDI/Nod, Allies/Soviets, GLA asymmetry | Identity through access and doctrine | Faction modifiers and exclusive abilities from POLITIKA canon only | adopt-concept |
| 18 | AI behavior | Scripted target priorities; retaliation flags | Predictable but exploitable | Deterministic scoring with observation-only inputs | redefine |
| 19 | Controls and feedback | Command bar, veterancy chevrons, health colors (`ConditionYellow=50%`, `ConditionRed=25%`) | Instant state reading | HUD per unit: health, rank, resolve tier, ammunition, statuses (never color-only) | adopt-concept |
| 20 | Unit mechanics versus strategy | Superweapons and general powers shape macro play | Strong late spikes need warnings and counters | Nuclear and talent rules stay in their owner contracts; unit abilities bounded | adopt-concept |

### 5.6 Distinctive classic units and the lesson each contributes

| Classic unit (title) | What made it distinctive | POLITIKA application (original units) |
| --- | --- | --- |
| Rifle infantry (TD/RA E1, TW squads) | Cheapest line; mass value; squads in TW | Squad entity with member count; Directorate price doctrine |
| Rocket soldier (RA E3) | Two weapons covering two layers | Shoulder-missile teams with AT plus limited AA (AC-INF-03, RD-INF-04) |
| Engineer (RA E6) | Instant structure capture | Capture as an 8 s interruptible channel; Unaligned jurisdiction |
| Commando (RA E7 / TD) | Infantry hard counter + C4 | Special-operations units with attached charges **only where canon allows**, with timers and disarm |
| Spy / Thief (RA) | Infiltration effects by building type | Faction-exclusive facility effects (Psy-Ops, diversion, contamination) with audit trails |
| Tank ladder (RA 1TNK–4TNK) | Each step buys one attribute | MBT → heavy → prototype ladders with caps |
| Artillery / V2 (RA) | Fragile, long range, ammunition-limited | `TPL-VEH-ARTY`: VEH_LIGHT, spotting, `AmmoCapacity`, counter-battery |
| Mobile Gap Generator / Radar Jammer (RA) | Information denial | Concealment screen and jamming field; detectable emitters |
| Mobile Sensor Array (TS LPST) | Deployable detection | Listening posts and counter-battery radars |
| IFV (RA2) | Weapon changes with the passenger | AC-ARM-03 modular infantry support vehicle |
| Tick tank (TS) | Deploys to dig in | RD-ARM-12 entrenchment tank; IA-ARM-12 field bunker |
| MIG (RA) | Sortie with 3 munitions | Fixed-wing sortie model |
| Kirov (RA2) | Slow, overwhelming bomber demands AA | Bombers are expensive, AA-dependent and interceptable |
| Hacker / Black Lotus (Generals) | Information and economy as weapons | Directorate theft and diversion; Unaligned counterintelligence |
| Zero Hour generals | Variants multiply balance and UI cost | One commander talent tree per faction, at most 81 paths |

### 5.7 Problematic patterns and the POLITIKA safeguard

| Pattern | Evidence | Risk | Safeguard |
| --- | --- | --- | --- |
| Instant capture or instant kill by infiltrators | RA `E6`/`SPY`/`THF` `Infiltrate=yes`; `E7` `C4=yes` | Low counterplay; "rush" openings | Channels, charge timers, detection, disarm, Unaligned response |
| Binary multipliers | RA2 Fire 600 % / HollowPoint 1 % | Composition stops mattering | Matrix cap 160 % |
| Veterancy snowball | RA2 `VeteranArmor=1.5` | Early wins compound | +10 / 20 / 30 % HP; no XP from farming |
| Cheap-unit spam versus expensive units | RA E1 $100 versus 4TNK $1700 | Unit-count overload | Food upkeep; Directorate mass bonus is coherent-formation-only (E) |
| Invisibility | TS `Cloakable=yes`; shroud generators | Frustration without tells | Concealment radius plus detectors; no invisibility (E) |
| Unit-variant explosion | Zero Hour generals | Balance matrix grows multiplicatively | Roster tiers (Core first) |
| Crush instant-kills | Generals `CRUSH 200 %` against humans | Pathing exploits | Deferred (framework U-07) |

### 5.8 Boundaries

This study supports design decisions only. Classic values were read for comparison and are not used as POLITIKA balance. Extracted files, rules text and source code remain outside the repository under their own licenses. Copying EA's GPL-released TD/RA source into POLITIKA is out of scope. The section makes no claim that the cited units behave identically in every patch or edition.
