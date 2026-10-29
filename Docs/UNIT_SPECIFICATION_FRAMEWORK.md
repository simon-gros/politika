# POLITIKA — Unit Specification Framework and RTS Implementation Reference

**Status:** PROPOSED DESIGN SPECIFICATION. Drafted 29 October 2026 against repository `main` at `a10101e`. **No roster unit is implemented.** Every number in this document and in the per-faction unit specifications is a **Provisional Baseline (PB)** for prototyping, playtest and later balance work. None of it is canon, an approved balance value or a measurement of an existing game.
**Owns:** the shared unit-specification vocabulary and template. That covers archetype templates, measurement conventions, the combat taxonomy (armor classes, damage types and the effectiveness matrix), ordinary veterancy, and how resolve and suppression integrate with units. It also covers detection and concealment conventions, logistics conventions, the unit AI profile library, multiplayer authority classes, the presentation baseline, and the mapping from unit specifications to Unreal Engine 5.8.3 data.
**Does not own:** implementation order ([Engine Outline](ENGINE_OUTLINE.md)); unit names and roster IDs ([Five-Faction Rosters](POLITIKA_FIVE_FACTION_ROSTERS.md)). It also does not own the match, economy, production, research, formation, talent, espionage, sabotage, nuclear, visibility, AI or multiplayer contracts. Their owning documents win on any conflict, and this framework only applies their rules to individual units.
**Per-faction unit specifications:** [Atlantic Concord](Units/ATLANTIC_CONCORD_UNITS.md) · [Red Directorate](Units/RED_DIRECTORATE_UNITS.md) · [Iron Ascendancy](Units/IRON_ASCENDANCY_UNITS.md) · [Meridian Pact](Units/MERIDIAN_PACT_UNITS.md) · [The Unaligned (NPC)](Units/UNALIGNED_UNITS.md).
**Companions:** [Unit Balance, Counter, Ability and Upgrade Matrices](UNIT_BALANCE_AND_ABILITY_MATRICES.md) · [C&C comparative study §5 (classic unit systems)](CNC_GAMEPLAY_COMPARATIVE_STUDY.md#5-classic-unit-system-reference-for-the-politika-unit-specifications) · [Unit specification coverage and consistency report](Reports/UNIT_SPECIFICATION_COVERAGE_20261029.md).

## 1. Reading the specifications

### 1.1 Evidence labels

Every specification field carries, or inherits, exactly one evidence label. **A field with no label is P.** A number is always P unless it is explicitly labelled I.

| Label | Meaning | Typical sources | What it does **not** mean |
| --- | --- | --- | --- |
| **I — Implemented** | Verified in current source code or tests on `main` (§2) | `Source/`, Automation/Rust tests, acceptance records | That a roster unit exists; that a packaged build of a later commit behaves the same |
| **E — Established design** | Explicitly fixed by an authoritative POLITIKA design document | Faction descriptions; locked rules in Resource Economy, Operational Economy, Sabotage, Reconnaissance, Espionage, Nuclear, Propaganda/Morale, Match Structure and Formations | Implementation or balance approval |
| **E-R — Roster-established** | Fixed by the roster **as the naming and taxonomy reference**: roster ID, English name, one-line purpose and C&C lineage | [Five-Faction Rosters](POLITIKA_FIVE_FACTION_ROSTERS.md) | Canon. The roster is itself an explicit non-canon proposal |
| **P — Proposed** | Recommended by this specification; not accepted | This framework and the faction unit files | Acceptance, balance, implementation |
| **U — Unresolved** | Needs an owner or technical decision before implementation | §20 register; per-unit notes | A default that may be implemented silently |

E-R is a qualifier of E, not a fifth category. It exists because the roster states that it is "not a statement that these units ... are implemented or approved as canon". Its identities are still the stable naming authority.

### 1.2 Roster tiers (P)

The roster proposes 49–57 units per playable faction. Classic C&C rosters field roughly 15–25 trainable units per side (§5 of the [C&C study](CNC_GAMEPLAY_COMPARATIVE_STUDY.md#5-classic-unit-system-reference-for-the-politika-unit-specifications)). Every unit specification therefore assigns one roster tier:

| Tier | Meaning | Target size |
| --- | --- | --- |
| **Core** | Candidate for the first complete skirmish roster and the first multiplayer balance pass. Each Core unit must have a distinct decision, a readable counter and at least one hard counter in every opposing playable faction | 14–18 per playable faction |
| **Extended** | Added after the Core counter web is playtested; same rules as Core | Remainder of the ordinary build tree |
| **Conditional** | Gated by espionage, nuclear, contract, market, prototype or scenario rules that the roster already marks as exceptional | As authored |
| **Scenario / NPC** | Unaligned or map-placed. Never a player build button | As authored |

The tier is a production-scheduling recommendation. It never removes a roster entry.

### 1.3 Identifiers (P unless noted)

- **Roster ID** (E-R): `AC-INF-01` and so on. It is unchanged and stays the design key.
- **Unreal stable IDs** follow the implemented content rules (I: lowercase ASCII, digits, underscores and single dots; independent of display name and asset path; unique per type namespace; see [Gameplay Content](GAMEPLAY_CONTENT.md)). Proposed forms:
  - unit: `unit.<f>.<cat>_<nn>`, for example `unit.ac.inf_01`;
  - weapon: `weapon.<f>.<cat>_<nn>.<slot>`, for example `weapon.ac.inf_01.primary`;
  - ability: `ability.shared.<verb>` (for example `ability.shared.sprint`) or `ability.<f>.<cat>_<nn>.<verb>`;
  - research and upgrade: `tech.shared.<name>` or `tech.<f>.<name>`;
  - AI profile: `ai.unit.<profile>`; armor class: `armor.<class>`; damage type: `damage.<type>`.

  The faction prefixes `<f>` are `ac`, `rd`, `ia`, `mp` and `un`.
- **Faction content IDs**: `faction.unaligned` is implemented (I). The proposed playable IDs prefix the existing registry IDs in `Data/content_manifest.json`: `faction.atlantic_concord`, `faction.red_directorate`, `faction.iron_ascendancy` and `faction.meridian_pact`. The prototype test IDs (`faction.player`, `faction.opposition`, `faction.test_third`, `faction.test_fourth`) stay test-only.
- **The roster ID prefix is not the category.** `AC-SUP-01` and `IA-SUP-01` are aircraft. `RD-AIR-09` is a ground vehicle. The specification's **Category** field is authoritative for gameplay classification, and the ID is never renumbered.

## 2. Implemented baseline (I)

Gameplay code that exists today and that every unit specification builds on, verified on `main` `a10101e`:

| Area | Implemented facts | Source |
| --- | --- | --- |
| Unit archetype data | `UPolitikaUnitArchetypeDefinition` (schema 1): `StableId`, `FactionId`, `DisplayName`, `MaxHealth`, `MovementSpeedCmPerSecond`, `WeaponId`, presentation references, `InfantryBehavior`, `bAttackable` and `SightRadiusCm` (default 1200 cm; MP-1 additive field) | `Source/Politika/Public/PolitikaPrototypeContent.h` |
| Weapon data | `UPolitikaWeaponDefinition` (schema 1): `RangeCm`, `Damage`, `CooldownSeconds` | same |
| Infantry behavior data | `UPolitikaInfantryBehaviorDefinition` defaults are listed below; the authored playtest asset is validated by `Validate()`. Magazine 30, reload 3 s, line of fire required, idle auto-acquire, acquisition 800 cm, turn 360°/s, aim tolerance 11.5°. Grenade: 650 cm range, 50 damage, 150 cm radius, 900 cm/s, 180 cm arc, 2 s wind-up, 3 s reload. Sprint: ×1.5 for 4 s, 12 s cooldown | `Source/Politika/Public/PolitikaInfantryBehavior.h` |
| Orders | `Move`, `Attack`, `AttackMove`, `Stop`, `Guard`, `Grenade`, `Reload` and `Sprint` through typed `CommandRequest` admission | `Source/PolitikaCore/Rules.h`; `EPolitikaPrototypeOrder` |
| Simulation | One authoritative coordinator `APolitikaPrototypeMatch`, fixed step `SimulationStepSeconds = 0.05` (20 Hz) | `Source/Politika/Public/PolitikaPrototypeGameplay.h` |
| Rules kernel | `Faction` enum and `IsPlayable` (Unaligned not playable); `AreAllied` / `CanAutoEngage`; `IsObservedBy` with `VisionSource.sight_radius`; `ShotCueFor` (Exact / FacingOnly / None). `CanPlantExplosives`: Meridian conventional at any time; Concord, Directorate and Ascendancy only through a spy after the late-espionage unlock; Unaligned never. `CanRefineNuclear`: Concord and Directorate yes; Ascendancy only with both intelligence flags; Meridian and Unaligned never | `Source/PolitikaCore/Rules.h` |
| Multiplayer | Server-authoritative listen and dedicated hosting, connection-bound seats, one command intake, team fog of war, per-connection shot-cue redaction, protocol 2, 1v1/2v1/3v1/2v2 plus NPC Unaligned | [Multiplayer Architecture](RTS_MULTIPLAYER_ARCHITECTURE.md) and its acceptance records |
| Prototype placeholder values (**not roster units**) | `unit.recon_vehicle` 120 HP, 340 cm/s, 650 cm / 12 dmg / 0.8 s. `unit.mechanized_fireteam` 180 HP, 210 cm/s, 520 cm / 18 / 1.1 s. `unit.opposing_patrol` 150 HP, 180 cm/s, 500 cm / 10 / 1.3 s. `unit.playtest.rifle_infantry` 225 HP, 210 cm/s, 650 cm / 8 / 0.2 s. `unit.playtest.target_dummy` 600 HP | `Source/PolitikaEditor/Private/PolitikaCreateShellCommandlet.cpp`, `PolitikaImportInfantryCommandlet.cpp` |

**Not implemented (therefore never I in a unit specification):** any roster unit; canonical faction IDs other than `faction.unaligned`; armor classes and damage types; area damage other than the grenade radius; squads and casualties; veterancy; `UnitResolve`, Discipline and suppression; abilities other than grenade, reload and sprint; transport and garrison; production, prices, food and fuel; research; detection, concealment and disguise; aircraft, naval units and any movement profile other than `movement.prototype_ground`; formations and stances (only the `Guard` order exists); and AI beyond idle auto-acquisition.

## 3. The unit specification template

### 3.1 Required fields

Each unit entry in a faction file uses the template below. Sections A–J correspond to the specification brief. A field may be "n/a" only when the category makes it meaningless, for example a weapon on an unarmed civilian car. "TBD" is not allowed; an open question is written as **U** with a register reference.

| § | Field | Content rule |
| --- | --- | --- |
| A | **Identity** | Roster ID; proposed UE stable ID; English name; faction; category / subclass; template (§3.2); roster tier; playable / NPC / scenario |
| A | **Role** | Primary role; secondary roles; doctrine fit (one sentence referencing the faction description) |
| A | **Inspiration and background** | Historical and technological inspiration (period-plausible 1980s equipment class, never a claim of an exact historical model); in-universe significance; C&C lineage (E-R, from the roster) |
| A | **Visual identity** | Concept-art file if one exists, otherwise "none (asset requirement)"; silhouette and faction visual language |
| B | **Statistics** | Health (squad: members × member HP); armor class; mobility class; speed; turn and acceleration only where they deviate from the template; footprint; sight; detection; transport slots occupied; food staff; fuel class; ExperienceValue |
| C | **Weapons** | One row per weapon: ID, damage type, range (minimum–maximum), damage and cooldown (burst, magazine and reload where used), projectile model, AoE, suppression, target layers, line of fire and turret arc, performance summary against INF / LIGHT-VEH / HEAVY-VEH / AIR / STRUCTURE |
| D | **Abilities** | One row per ability: ID; kind (active / passive / conditional / automated / upgrade-unlocked); effect; activation; targets; conditions; cost; duration; cooldown; range / AoE; counters; HUD, VFX and SFX; AI rule; MP authority note |
| E | **Production and logistics** | USD cost; producer building (roster ID); prerequisites; build time; food upkeep; fuel; upkeep or contract wage; repair, replenishment and salvage; unique cap |
| F | **Research and upgrades** | Applicable shared and faction technologies with their effect and trade-off |
| G | **Command integration** | Supported orders (I or P); default stance; formation role; special command behavior |
| H | **AI profile** | Profile ID (§14) and deltas: target priorities, preferred range, engage and disengage rules, ability triggers, retreat threshold, combinations |
| I | **Multiplayer authority** | MP class (§15) and unit-specific synchronization-sensitive state and exploit surfaces |
| J | **Presentation** | Model, rig and animation requirements beyond the template set; VFX and SFX; icon; readability notes |
| — | **Counters** | Hard / soft / situational counters: what it beats, what beats it, when it is valuable |
| — | **Notes and status** | Field status summary; open items (U-refs) |

Templates (§3.2) carry the defaults for B, G, H, I and J. A faction entry repeats a default only when it changes it.

### 3.2 Archetype templates (P)

Template defaults are summary values. Weapons and abilities are always specified per unit.

| Template | Category / layer | Mobility class (§5.2) | Armor (§6.1) | Sight / detection | Default orders and stance | AI profile (§14) | MP class (§15) | Animation set (§16) |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| `TPL-INF-LINE` | Infantry squad / Ground | FOOT | INF_LIGHT | 1200 cm | Move, Attack, AttackMove, Stop, Guard, Reload, Sprint (I); Garrison, Patrol, Withdraw (P) · Aggressive | `ai.unit.line_infantry` | MPA-STD | ANIM-INF |
| `TPL-INF-AT` | Infantry team / Ground | FOOT | INF_LIGHT | 1200 cm | as LINE · Guard | `ai.unit.anti_armor_infantry` | MPA-STD | ANIM-INF + launcher |
| `TPL-INF-HW` | Heavy-weapons team (MG, mortar) / Ground | FOOT_HEAVY | INF_LIGHT | 1200 cm | as LINE + Deploy/Undeploy (P) · Hold Ground when deployed | `ai.unit.support_weapon` | MPA-STD | ANIM-INF + deploy |
| `TPL-INF-SPEC` | Specialist infantry (marksman, recon, commando) | FOOT | INF_LIGHT or INF_PROTECTED | 1500 cm; may detect CONCEALED | as LINE + Conceal (P) · Hold Fire while concealed | `ai.unit.specialist_infantry` | MPA-STD (+MPA-COVERT for concealment) | ANIM-INF + prone/crawl |
| `TPL-INF-SUPPORT` | Medic, engineer, pioneer | FOOT | INF_LIGHT | 1000 cm | Move, Stop, Guard (I); Heal / Repair / Capture / Build (P) · Hold Fire unless armed | `ai.unit.support_infantry` | MPA-STD | ANIM-INF + work loop |
| `TPL-INF-COVERT` | Agent, spy, courier | FOOT | INF_LIGHT | 1200 cm; DETECT.SPY optional | Move, Stop (I); Disguise, Infiltrate, Plant Charge (P) · Hold Fire | `ai.unit.covert_agent` | MPA-COVERT | ANIM-INF + civilian walk |
| `TPL-INF-COMMAND` | Officer or commander | FOOT | INF_PROTECTED | 1500 cm | as LINE + command abilities (P) · Guard | `ai.unit.commander` / `ai.unit.officer` | MPA-STD + MPA-COMMAND | ANIM-INF + command gestures |
| `TPL-VEH-SCOUT` | Scout vehicle (bike, truck, jeep) | WHEEL_LIGHT | VEH_SOFT | 1800 cm | Move, Attack, AttackMove, Stop, Guard (I); Patrol, Scout Route (P) · Hold Ground | `ai.unit.scout` | MPA-STD | ANIM-VEH-WHEEL |
| `TPL-VEH-LIGHT` | Armed car, technical, gun truck | WHEEL_LIGHT | VEH_SOFT or VEH_LIGHT | 1400 cm | as SCOUT · Aggressive | `ai.unit.skirmisher` | MPA-STD | ANIM-VEH-WHEEL + mount |
| `TPL-VEH-TRANSPORT` | APC, truck, carrier | WHEEL_HEAVY or TRACKED | VEH_LIGHT (truck VEH_SOFT) | 1200 cm | Move, Stop, Guard, Attack if armed (I); Load / Unload (P) · Hold Fire if unarmed | `ai.unit.transport` | MPA-TRANSPORT | ANIM-VEH + doors/ramp |
| `TPL-VEH-IFV` | IFV / modular carrier | TRACKED | VEH_MEDIUM | 1300 cm | as TRANSPORT + Attack · Aggressive | `ai.unit.ifv` | MPA-TRANSPORT | ANIM-VEH-TRACK + turret |
| `TPL-VEH-MBT` | Main battle tank, light tank | TRACKED | VEH_HEAVY (light tank VEH_MEDIUM) | 1300 cm | Move, Attack, AttackMove, Stop, Guard (I); Patrol, Withdraw (P) · Aggressive | `ai.unit.battle_tank` | MPA-STD | ANIM-VEH-TRACK + turret |
| `TPL-VEH-HEAVY` | Heavy, siege and prototype tank | TRACKED_HEAVY | VEH_HEAVY | 1300 cm | as MBT · Aggressive | `ai.unit.heavy_armor` | MPA-STD | ANIM-VEH-TRACK + multi-turret |
| `TPL-VEH-TD` | Tank destroyer / ATGM carrier | TRACKED or WHEEL_HEAVY | VEH_MEDIUM | 1400 cm | as MBT · Hold Ground | `ai.unit.tank_destroyer` | MPA-STD | ANIM-VEH + launcher |
| `TPL-VEH-ARTY` | Self-propelled gun, rocket or mortar carrier | TRACKED or WHEEL_HEAVY | VEH_LIGHT | 1100 cm | Move, Attack, Stop, Guard (I); Fire Mission (attack ground), Deploy (P) · Hold Ground | `ai.unit.artillery` | MPA-INDIRECT | ANIM-VEH + elevation/recoil |
| `TPL-VEH-AA` | Mobile air defense | TRACKED or WHEEL_HEAVY | VEH_LIGHT or VEH_MEDIUM | 1300 cm ground / 3000 cm air radar | as MBT · Aggressive (air) / Hold Fire (ground) | `ai.unit.air_defense` | MPA-STD | ANIM-VEH + radar spin |
| `TPL-VEH-EW` | Jammer, radar, concealment, sensor | WHEEL_HEAVY or TRACKED | VEH_LIGHT | 1400 cm + sensor | Move, Stop, Guard (I); Deploy, Jam, Screen (P) · Hold Fire | `ai.unit.electronic_warfare` | MPA-STD + MPA-SENSOR | ANIM-VEH + mast deploy |
| `TPL-VEH-ENGINEER` | Combat engineer vehicle, dozer, bridger, mine clearer | TRACKED | VEH_HEAVY or VEH_MEDIUM | 1100 cm | Move, Stop, Guard (I); Clear, Breach, Bridge, Build Cover (P) · Hold Fire | `ai.unit.engineer_vehicle` | MPA-STD | ANIM-VEH + blade/arm |
| `TPL-VEH-LOGISTICS` | Tanker, supply, repair or recovery | WHEEL_HEAVY | VEH_SOFT or VEH_LIGHT | 1000 cm | Move, Stop, Guard (I); Deliver, Resupply, Repair, Tow (P) · Hold Fire | `ai.unit.logistics` | MPA-ECON | ANIM-VEH-WHEEL + work |
| `TPL-VEH-CONSTRUCTOR` | Mobile works, construction tractor | WHEEL_HEAVY or TRACKED | VEH_MEDIUM | 1000 cm | Move, Stop (I); Deploy / Build (P) · Hold Fire | `ai.unit.constructor` | MPA-ECON | ANIM-VEH + deploy |
| `TPL-AIR-SCOUT` | Light reconnaissance plane | FIXED_WING (loiter) | AIR_FIXED | 2600 cm (ground footprint) | Move, Stop (I); Patrol / Orbit, Return to Base (P) · Hold Fire (unarmed) | `ai.unit.air_scout` | MPA-AIR | ANIM-AIR-FIXED |
| `TPL-AIR-FIGHTER` | Interceptor, fighter | FIXED_WING (sortie) | AIR_FIXED | 2200 cm (air) | Attack, Move, Stop (I); Patrol, Return to Base (P) · Aggressive (air) | `ai.unit.air_superiority` | MPA-AIR | ANIM-AIR-FIXED |
| `TPL-AIR-STRIKE` | Strike or ground-attack jet, bomber | FIXED_WING (sortie) | AIR_FIXED | 1800 cm | Attack (I); Strike Area / Return to Base (P) · Hold Fire until ordered | `ai.unit.air_strike` | MPA-AIR | ANIM-AIR-FIXED + bay/pylons |
| `TPL-AIR-HELI-ATTACK` | Attack helicopter | ROTARY | AIR_ROTARY | 1600 cm | Move, Attack, AttackMove, Stop, Guard (I); Patrol, Return to Pad (P) · Aggressive | `ai.unit.attack_helicopter` | MPA-AIR | ANIM-AIR-ROTARY |
| `TPL-AIR-HELI-TRANSPORT` | Utility or assault transport helicopter | ROTARY | AIR_ROTARY | 1400 cm | Move, Stop (I); Load / Unload / Fast-Rope (P) · Hold Fire | `ai.unit.air_transport` | MPA-AIR + MPA-TRANSPORT | ANIM-AIR-ROTARY + doors |
| `TPL-AIR-TRANSPORT` | Fixed-wing transport or cargo | FIXED_WING (sortie) | AIR_FIXED | 1400 cm | Move (I); Paradrop / Air-land / Return (P) · Hold Fire | `ai.unit.air_transport` | MPA-AIR + MPA-TRANSPORT | ANIM-AIR-FIXED + ramp |
| `TPL-AIR-SURVEILLANCE` | AEW, radar and maritime patrol aircraft | FIXED_WING (loiter) | AIR_FIXED | 3200 cm + radar | Move (I); Orbit, Return (P) · Hold Fire | `ai.unit.air_surveillance` | MPA-AIR + MPA-SENSOR | ANIM-AIR-FIXED + radome |
| `TPL-NAV-PATROL` | Patrol boat, launch, cutter | SURFACE_SHALLOW | NAV_LIGHT | 1600 cm | Move, Attack, AttackMove, Stop, Guard (I); Patrol (P) · Aggressive | `ai.unit.naval_patrol` | MPA-STD | ANIM-NAV |
| `TPL-NAV-COMBATANT` | Frigate, destroyer, cruiser, missile boat | SURFACE_DEEP or SURFACE_SHALLOW | NAV_HEAVY (missile boat NAV_LIGHT) | 2000 cm + sonar or radar | as PATROL · Aggressive | `ai.unit.naval_combatant` | MPA-STD + MPA-SENSOR | ANIM-NAV + turrets |
| `TPL-NAV-SUB` | Submarine | SUBSURFACE | NAV_HEAVY | 1600 cm (surface); sonar | Move, Attack, Stop (I); Dive / Surface (P) · Hold Fire while submerged | `ai.unit.submarine` | MPA-COVERT | ANIM-NAV-SUB |
| `TPL-NAV-TRANSPORT` | Landing craft, barge, amphibious carrier, HQ ship | SURFACE_SHALLOW or AMPHIBIOUS | NAV_LIGHT or NAV_HEAVY | 1200 cm | Move, Stop (I); Load / Unload / Beach (P) · Hold Fire | `ai.unit.transport` | MPA-TRANSPORT | ANIM-NAV + ramp |
| `TPL-CIV` | Civilian vehicle or vessel (Unaligned) | WHEEL_LIGHT / WHEEL_HEAVY / RAIL / SURFACE_SHALLOW / FIXED_WING | VEH_SOFT / NAV_LIGHT / AIR_FIXED | 800 cm | System-scheduled routes; never player-commandable (E) · Hold Fire | `ai.unit.civilian_ambient` | MPA-NPC | ANIM-CIV |
| `TPL-NPC-SECURITY` | Unaligned police, JNA and Territorial Defence units | per unit | per unit | per unit | Unaligned institutional authority only (E) · Guard | `ai.unit.npc_security` + response tier (§14.3) | MPA-NPC | category set |

## 4. Measurement conventions and provisional baseline

### 4.1 Units and quantization

- Distances are in centimetres (UE world units). Speeds are in cm/s, angles in degrees and rates in degrees per second.
- Durations are in seconds and are **quantized up to whole 0.05 s simulation steps** (I: 20 Hz coordinator). A 1.0 s cooldown is 20 steps, and a 0.12 s burst interval becomes 3 steps (0.15 s).
- Health and damage are integers after modifiers. Round half away from zero once, at the point of application, never per modifier.
- USD is shown in whole dollars in specifications. The ledger stores integer cents (E: [Operational Economy §6](RTS_OPERATIONAL_ECONOMY_AND_POWER.md#6-accounting-authority-and-safe-ordering)).
- Food is in food units per upkeep interval. Fuel is in fuel units of petroleum or kerosene; the two stocks are never interchangeable (E).

### 4.2 Reference unit and combat value

The **Reference Rifle Squad (RRS)** anchors all comparative values. It matches the implemented playtest placeholder within rounding: 225 HP, 650 cm, 40 raw DPS.

| Reference | Value (PB) |
| --- | --- |
| Members × member HP | 4 × 60 = 240 HP, armor INF_LIGHT |
| Speed | 200 cm/s (FOOT) |
| Sight / weapon range | 1200 cm / 650 cm |
| Weapon (per member) | SMALL_ARMS, 10 damage per 1.0 s, so 40 DPS at full strength; magazine 30 and reload 3 s (I values reused) |
| Cost / time / food | $300, 8 s at a single production lane, 4 food per interval |

**Combat value (CV)**, a P analysis tool for balance reviews: `CV = sqrt(EHP_vs_reference × EDPS_vs_reference) / CV_RRS`. Effective HP and DPS use the §6.3 effectiveness percentages against a stated opponent mix. CV per $1,000 within one faction and category should stay within ±20 % of the faction norm, unless mobility, range, detection or an ability explains the difference in writing. CV is a review aid, not a balance target.

### 4.3 Provisional baseline bands

| Category | HP (entity) | Speed cm/s | Sight cm | Typical weapon range cm | USD | Build time s | Staff (food) |
| --- | --- | --- | --- | --- | --- | --- | --- |
| Line infantry squad | 200–320 (3–5 members) | 190–220 | 1200 | 600–700 | 250–400 | 7–10 | 3–5 |
| Specialist / heavy-weapons team | 120–240 (2–3 members) | 160–220 | 1200–1600 | 550–1100 (mortar to 2000) | 350–900 | 9–14 | 2–3 |
| Agents / covert | 80–150 (1–2) | 200–230 | 1200 | 0–500 | 700–1600 | 14–22 | 1–2 |
| Elite / commander | 300–600 (1–4) | 210–240 | 1500 | 650–900 | 1600–2800 | 25–40 | 1–4 |
| Scout vehicle | 160–260 | 520–620 (ground); 1600 (air scout) | 1800 (ground) / 2600 (air) | 0–600 | 150–300 | 5–7 | 1–2 |
| Light armed vehicle | 250–380 | 420–520 | 1400 | 650–800 | 350–600 | 8–10 | 2–3 |
| APC / IFV | 450–650 | 330–420 | 1200–1300 | 700–850 | 600–950 | 12–16 | 2–3 |
| Main battle tank | 800–1000 | 300–340 | 1300 | 850–950 | 950–1300 | 18–24 | 3–4 |
| Heavy / siege tank | 1200–1600 | 230–270 | 1300 | 900–1000 | 1700–2400 | 28–36 | 4 |
| Super-heavy prototype | 2200–2800 | 160–190 | 1400 | 950–1100 | 3500–4500 | 45–60 | 5–6 |
| Tank destroyer / ATGM | 450–650 | 300–380 | 1400 | 1100–1300 | 900–1300 | 16–20 | 2–3 |
| Artillery | 350–550 | 260–320 | 1100 | 2400–4200 (minimum 700–1000) | 900–1700 | 18–26 | 3–5 |
| Mobile air defense | 450–700 | 300–360 | 1300 / 3000 air | 900–1100 (gun), 1600–2000 (SAM) | 800–1200 | 14–18 | 3 |
| Logistics / support vehicle | 300–700 | 320–420 | 1000 | 0 | 300–900 | 8–14 | 1–3 |
| Attack helicopter | 400–600 | 700–800 | 1600 | 900–1200 | 1200–1700 | 18–22 | 2 |
| Transport helicopter | 450–700 | 650–750 | 1400 | 0–600 | 900–1400 | 14–18 | 2–3 |
| Fighter / interceptor | 320–420 | 2200–2600 | 2200 (air) | 1100–1400 | 1400–2000 | 20–25 | 1 |
| Strike jet / bomber | 360–700 | 1500–2100 | 1800 | 0–1000 (bombs: overflight) | 1600–2600 | 22–32 | 1–4 |
| Patrol boat | 300–500 | 520–700 | 1600 | 700–900 | 500–900 | 12–16 | 3–6 |
| Major combatant ship | 1400–2600 | 300–400 | 2000 | 1200–2800 (shore fire up to 3600) | 1800–3200 | 35–55 | 10–20 (abstracted) |
| Submarine | 800–1100 | 300–380 | 1600 | 1400 (torpedo) | 1600–2200 | 35–45 | 6 (abstracted) |

Bands describe the expected range. A unit specification states one value and explains any departure from its band.

### 4.4 Faction-wide modifiers that implement established rules

| Faction | Established rule (E) | Provisional implementation (P) |
| --- | --- | --- |
| Atlantic Concord | Fossil fuels are slightly cheaper, through lower refining cost; high-cost elite units | Refinery operating cost −12 % (economy, not unit prices). Unit prices sit at the upper edge of each band: the professional premium |
| Red Directorate | Infantry is far cheaper than for any other playable faction | Infantry price −45 % against the band midpoint for the comparable role. **Validator rule:** each Directorate infantry role price × 1.5 must not exceed the cheapest comparable role price of any other playable faction |
| Iron Ascendancy | Slightly greater raw destructive power; explicit, counterable high-risk aggression | All Ascendancy weapons +8 % damage (one documented faction modifier, never hidden inside each weapon value). Self-endangering abilities (for example *Overdrive*, *Last Stand*) are explicit per unit and carry a self-damage or exposure cost |
| Meridian Pact | Cheapest and fastest early scouting; best early conventional sabotage; no advanced propaganda; no nuclear path | Scout and light-truck prices at the lower band edge; rifle infantry is **not** cheaper than the Directorate equivalent (E-R) |
| The Unaligned | NPC-only; not a purchasable economy | No player price. Specifications state a **mobilization weight** that the Unaligned institutional AI consumes from its scenario budget (§11.6) |

### 4.5 Upkeep conventions (P, implementing E)

- **Food (E: staffed forces need food reserves):** upkeep interval 30 s (600 steps). Consumption is 1 food unit per staff member per interval. Explicitly unmanned units consume 0. Squads count living members, vehicles count crew, aircraft count aircrew plus 1 ground-crew equivalent, and ships use an abstracted crew figure. Shortage applies the single bounded penalty owned by the [Operational Economy](RTS_OPERATIONAL_ECONOMY_AND_POWER.md#5-mandatory-food-reserves-for-staffed-forces): this framework proposes −20 % weapon cooldown speed and blocked Discipline recovery, removed at the next upkeep boundary after replenishment. **U-05.**
- **Fuel class:** `none` (infantry, animals), `light` (bikes, cars, trucks), `medium` (APCs, IFVs, artillery), `heavy` (tanks, engineer vehicles), `super` (prototypes) and `kerosene` (aircraft). Production consumes petroleum: light 1, medium 3, heavy 5, super 10 (PB). Aircraft consume kerosene per sortie or rearm (per unit). **Fuel starvation** (P, U-06): while a participant's petroleum stock is 0, its petroleum vehicles move at −25 % speed and new petroleum-consuming production pauses. While kerosene is 0, fixed-wing aircraft cannot launch new sorties. No per-second fuel burn and no refuelling chore (E: no mandatory refuelling chore).
- **Power (E):** units consume no electricity. Powered producer buildings must satisfy minimum load or production pauses.

## 5. Classification, mobility and footprint

### 5.1 Categories and target layers (P)

| Category | Subclasses | Layer | Notes |
| --- | --- | --- | --- |
| Infantry | line, anti-armor, heavy weapons, specialist, support, covert, officer/commander | GROUND | Squads are single authoritative entities with a member count (§5.4) |
| Vehicle | wheeled (light/heavy), tracked (medium/heavy/super), amphibious | GROUND (amphibious also WATER_SURFACE) | Crewed unless stated |
| Aircraft | rotary, fixed-wing (sortie or loiter) | AIR_LOW (rotary, recon planes) / AIR_HIGH (jets) | AIR_LOW can be engaged by MGs and autocannon; AIR_HIGH only by FLAK_AA and air-to-air weapons |
| Naval | surface (shallow/deep draft), subsurface | WATER_SURFACE / WATER_SUBSURFACE | Map-restricted (E-R) |
| Civilian | road vehicle, rail, vessel, aircraft | per vehicle | Non-combat **Morale** (E), never player-commandable (E) |
| Deployed state | deployed bunker, dug-in tank, deployed mast | GROUND | A state of the same unit, never a separate trainable unit (E-R) |

A weapon declares the layers it can target. A unit with no weapon that can reach a layer cannot attack that layer: this is target eligibility, not a damage percentage.

### 5.2 Mobility classes and terrain multipliers (P)

The only implemented movement profile is `movement.prototype_ground` (I). The classes below need Recast nav-agent radii and nav-area costs, and that choice is **U-03**. Multipliers apply to the unit's base speed on that surface. × means impassable.

| Class | Road | Open | Rough | Forest | Urban | Steep/mountain | Ford (shallow) | Deep water | Accel to full | Turn rate |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| FOOT | 1.10 | 1.00 | 0.85 | 0.80 | 0.95 | 0.60 | 0.50 | × | 0.2 s | 360°/s (I) |
| FOOT_HEAVY (crew-served weapons) | 1.05 | 1.00 | 0.75 | 0.70 | 0.90 | 0.50 | 0.40 | × | 0.4 s | 240°/s |
| WHEEL_LIGHT | 1.25 | 0.90 | 0.60 | 0.40 | 0.90 | 0.35 | 0.40 | × | 0.8 s | 180°/s |
| WHEEL_HEAVY | 1.20 | 0.85 | 0.55 | 0.35 | 0.85 | 0.30 | 0.35 | × | 1.2 s | 120°/s |
| TRACKED | 1.00 | 0.95 | 0.80 | 0.60 | 0.70 | 0.45 | 0.50 | × | 1.5 s | hull 60°/s, turret 90°/s |
| TRACKED_HEAVY | 0.95 | 0.90 | 0.70 | 0.50 | 0.60 | 0.30 | 0.40 | × | 2.5 s | hull 40°/s, turret 50°/s |
| AMPHIBIOUS (wheeled/tracked) | as base class | | | | | | 0.70 | 0.45 | base | base |
| ROTARY | 1.00 everywhere; ignores terrain; blocked by authored no-fly volumes | | | | | | | | 1.0 s | 120°/s, can hover |
| FIXED_WING | 1.00; minimum speed 60 % of max, turn radius 800–1400 cm, no hover; sortie from an owned airfield | | | | | | | | 2.0 s | radius-limited |
| SURFACE_SHALLOW | × on land | | | | | | 1.00 | 1.00 | 1.5 s | 50°/s |
| SURFACE_DEEP | × | | | | | | × | 1.00 | 3.0 s | 25°/s |
| SUBSURFACE | × | | | | | | × | 1.00 (submerged ×0.8) | 3.0 s | 25°/s |
| RAIL (civilian/NPC trains) | authored rail spline only | | | | | | | | 6.0 s | n/a |

Rules: (1) Terrain classes require map authoring support, and the current map schema has none (U-03). (2) Speed changes from fuel starvation, food shortage, suppression and veterancy multiply with these values, and the order is documented in §18.4. (3) Bridges, ferries and fords are traversable only under the Unaligned jurisdiction rules of the map (E: crossing is not authorization).

### 5.3 Footprint and collision classes (P)

| Class | Radius | Units | Collision |
| --- | --- | --- | --- |
| F0 | 40 cm per member; squad footprint 120 cm | Infantry | Infantry pass through friendly infantry and are blocked by vehicles and structures |
| F1 | 150 cm | Bikes, jeeps, trucks, cars | Block ground units |
| F2 | 200 cm | APCs, IFVs, medium tanks, artillery | Block |
| F3 | 250 cm | MBTs, heavy tanks, engineer vehicles | Block |
| F4 | 320 cm | Super-heavy prototypes, emplacement carriers | Block; may require wide nav agent (U-03) |
| FA | n/a while airborne; occupies a pad or runway slot when landed | Aircraft | Airborne units do not collide with ground units |
| FN | per hull, 250–900 cm | Ships, boats | Water-only collision |

Crush and overrun of infantry by vehicles is **U-07**, deferred: it needs collision and navigation evidence that the current Actor-based route follower has not produced.

### 5.4 Squad representation (P; U-01)

Recommended model: a squad is **one authoritative entity**, which preserves the implemented single-actor command, navigation and replication contract (I). It carries `SquadMembers` and `MemberHealth`:

- Entity HP = members × member HP. The living member count is `ceil(HP / MemberHealth)`, and presentation shows that many member meshes.
- Weapon output scales with living members: each member contributes its per-member weapon.
- Casualties never restore through healing alone. Healing raises HP only up to `livingMembers × MemberHealth`. **Replenishment** restores members, at a friendly producer building or through an explicit support ability, for a USD charge proportional to the missing members (§10.3).
- Transport slot and food staff use the living member count.
- Alternative (rejected for Core, U-01): independent per-member entities, which would multiply selection, navigation and network cost.

## 6. Combat model (P)

### 6.1 Armor classes

| ID | Name | Typical members | Design intent |
| --- | --- | --- | --- |
| `armor.inf_light` | INF_LIGHT | Line, anti-armor and support infantry; agents; civilians on foot | Vulnerable to small arms, fragmentation and fire |
| `armor.inf_protected` | INF_PROTECTED | Body-armored shock troops, chemical-defense troops, commanders, honor guard | About 30 % more resilient to small arms and fragmentation; still infantry |
| `armor.veh_soft` | VEH_SOFT | Motorcycles, jeeps, pickups, trucks, vans, civilian cars and lorries | Small arms matter; everything heavier ruins it |
| `armor.veh_light` | VEH_LIGHT | APCs, armored cars, self-propelled artillery, logistics carriers, EW vehicles | Stops rifles; vulnerable to autocannon and anti-tank |
| `armor.veh_medium` | VEH_MEDIUM | IFVs, light and fast tanks, tank destroyers, AA vehicles with turrets | Needs dedicated anti-tank or autocannon focus |
| `armor.veh_heavy` | VEH_HEAVY | MBTs, heavy and siege tanks, armored engineer vehicles, super-heavies | Effectively immune to small arms; anti-tank and tank guns required |
| `armor.air_rotary` | AIR_ROTARY | Helicopters | Low altitude; MGs and autocannon can hurt it |
| `armor.air_fixed` | AIR_FIXED | Jets, planes, bombers, civilian aircraft | Only AA weapons and air-to-air reach it |
| `armor.nav_light` | NAV_LIGHT | Boats, launches, landing craft, barges, missile boats | Surface; MGs and autocannon effective |
| `armor.nav_heavy` | NAV_HEAVY | Frigates, destroyers, cruisers, submarines, HQ vessels | Needs anti-ship, torpedo or heavy guns |
| `armor.str_soft` | STR_SOFT | Ordinary buildings and civilian infrastructure | High-explosive, demolition and fire |
| `armor.str_hardened` | STR_HARDENED | Bunkers, casemates, fortified positions, hardened shelters | Demolition, heavy HE, anti-tank breaching |

### 6.2 Damage types

| ID | Name | Weapons | Design intent |
| --- | --- | --- | --- |
| `damage.small_arms` | SMALL_ARMS | Rifles, carbines, pistols, LMG/HMG, shotguns | Anti-personnel baseline; suppressive |
| `damage.precision` | PRECISION | Marksman and sniper rifles | Hard anti-infantry counter; useless against armor (the RA `HollowPoint` lesson, §19) |
| `damage.high_explosive` | HIGH_EXPLOSIVE | Grenades, mortars, howitzers, unguided rockets, bombs, HE tank rounds | Area damage against infantry, soft targets and structures; weak against heavy armor |
| `damage.autocannon` | AUTOCANNON | 20–40 mm guns, quad AA mounts used against the ground, helicopter cannons | Bridges anti-personnel and anti-light-vehicle roles; good against rotary air |
| `damage.kinetic_ap` | KINETIC_AP | Tank guns, anti-tank guns, AP autocannon in AT mode | Primary anti-heavy-armor |
| `damage.shaped_charge` | SHAPED_CHARGE | RPGs, recoilless guns, ATGMs, shoulder missiles against ground targets | Infantry and vehicle anti-armor; good against structures |
| `damage.flak_aa` | FLAK_AA | SAMs, MANPADS in air mode, AA flak bursts | Air-only (eligibility) |
| `damage.incendiary` | INCENDIARY | Flamethrowers, flame vehicles | Brutal against infantry and garrisons; weak against armor; abstract game weapon only (E-R) |
| `damage.torpedo` | TORPEDO | Torpedoes, depth charges | Naval-only |
| `damage.demolition` | DEMOLITION | Attached sabotage charges, breaching charges, engineer demolition | Vehicles and structures only; never anti-personnel (E: sabotage targets vehicles and buildings) |
| `damage.anti_ship` | ANTI_SHIP | Anti-ship missiles, coastal missile launchers | Naval and coastal structures |

### 6.3 Effectiveness matrix (percent of base damage)

Rows are damage types and columns are armor classes. **0 means the target is ineligible**: it cannot be auto-acquired and an explicit attack order on it is rejected.

| Damage \ Armor | INF_L | INF_P | VEH_SOFT | VEH_L | VEH_M | VEH_H | AIR_R | AIR_F | NAV_L | NAV_H | STR_S | STR_H |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| SMALL_ARMS | 100 | 70 | 50 | 15 | 6 | 2 | 25 | 0 | 30 | 2 | 8 | 2 |
| PRECISION | 160 | 120 | 10 | 2 | 1 | 0 | 5 | 0 | 5 | 0 | 0 | 0 |
| HIGH_EXPLOSIVE | 110 | 80 | 80 | 45 | 25 | 12 | 30 | 0 | 60 | 25 | 100 | 60 |
| AUTOCANNON | 65 | 50 | 100 | 85 | 45 | 10 | 100 | 60 | 90 | 20 | 30 | 8 |
| KINETIC_AP | 20 | 20 | 70 | 90 | 100 | 100 | 30 | 0 | 70 | 70 | 40 | 45 |
| SHAPED_CHARGE | 25 | 25 | 90 | 100 | 100 | 90 | 60 | 0 | 90 | 75 | 70 | 70 |
| FLAK_AA | 0 | 0 | 0 | 0 | 0 | 0 | 110 | 100 | 0 | 0 | 0 | 0 |
| INCENDIARY | 160 | 110 | 70 | 25 | 10 | 5 | 0 | 0 | 20 | 5 | 80 | 40 |
| TORPEDO | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 110 | 100 | 0 | 0 |
| DEMOLITION | 0 | 0 | 100 | 100 | 100 | 100 | 0 | 0 | 100 | 100 | 150 | 120 |
| ANTI_SHIP | 0 | 0 | 40 | 40 | 40 | 30 | 0 | 0 | 120 | 100 | 60 | 40 |

Design reading: the matrix is deliberately less extreme than RA2's 600 % fire or 1 % hollow-point values. Hard counters still exist, for example SMALL_ARMS against VEH_HEAVY and PRECISION against any vehicle, so roles stay legible. Mixed forces still matter, because no damage type exceeds 160 %.

### 6.4 Acquisition semantics from effectiveness (P; adopted concept from RA2 `Verses` notes, §19)

| Effectiveness | Explicit attack order | Auto-acquire / retaliation |
| --- | --- | --- |
| 0 % | **Rejected** with `OrderRejection` "invalid target" (reuse the existing typed rejection; never disclose hidden information) | Never |
| 1–14 % | Accepted (nuisance fire) | Never auto-acquired; no retaliation |
| ≥ 15 % | Accepted | Auto-acquired subject to stance, visibility, `CanAutoEngage` (I) and target priority (§14.2) |

For a multi-weapon unit, eligibility is the maximum across its weapons that can reach the target layer, and each weapon fires only at targets it can damage at ≥ 15 % (or at an explicitly ordered target).

### 6.5 Weapon definition contract

Extends the implemented `UPolitikaWeaponDefinition` (`RangeCm`, `Damage`, `CooldownSeconds`; I). New fields are P (§18):

| Field | Meaning | Example (RRS rifle) |
| --- | --- | --- |
| `DamageType` | §6.2 ID | `damage.small_arms` |
| `MinRangeCm` | Cannot fire inside this distance | 0 |
| `RangeCm` (I) | Maximum range | 650 |
| `Damage` (I) | Per hit, before effectiveness, veterancy and faction modifiers | 10 (per member) |
| `CooldownSeconds` (I) | Between shots or bursts | 1.0 |
| `BurstCount` / `BurstIntervalSeconds` | Rounds per burst | 1 / 0 |
| `MagazineCapacity` / `ReloadSeconds` | Reuses infantry behavior (I) for infantry; vehicles use `AmmoCapacity` | 30 / 3 |
| `AmmoCapacity` / `ResupplyRule` | Limited ammunition for artillery, missiles, aircraft and mines (§10.4) | n/a |
| `ProjectileModel` | `hitscan`, `shell_direct`, `guided_missile`, `ballistic`, `lobbed` (I for grenades), `bomb_drop`, `torpedo`, `stream`, `attached_charge` | `hitscan` |
| `ProjectileSpeedCmPerSecond` | Flight time is computed authoritatively and damage applies at the impact step | n/a |
| `AoE` | `FullRadiusCm`, `MaxRadiusCm`, `EdgePercent` (linear falloff) | none |
| `Scatter` | `ScatterMinCm` at minimum range to `ScatterMaxCm` at maximum (ballistic and lobbed) | none |
| `TrackingClass` | `slow`, `medium`, `fast`, `guided` (§6.6) | `fast` |
| `TargetLayers` | GROUND / AIR_LOW / AIR_HIGH / WATER_SURFACE / WATER_SUBSURFACE / STRUCTURE | GROUND, AIR_LOW |
| `RequiresLineOfFire` | Reuses `bRequiresLineOfFire` (I) for direct fire; indirect weapons need **observation** of the target or aim point instead | true |
| `MountArcDegrees` / `TurretTurnRate` | 360 for turrets; hull-mounted guns need the hull facing | n/a |
| `FireWhileMoving` | true / false / penalty (×0.5 cooldown speed) | true |
| `SuppressionStress` | `UnitResolve` loss per hit on eligible targets (§7) | 3 |
| `FactionDamageModifier` | Applied once from §4.4 (Ascendancy +8 %) | 0 |

### 6.6 Projectile, accuracy and randomness (P)

Server authority is state replication, not lockstep (E: [Multiplayer Architecture §6](RTS_MULTIPLAYER_ARCHITECTURE.md#6-transport-scaling-and-rendering-strategy)). Clients never roll combat. Tests still need reproducible outcomes, so:

1. **Direct fire** (`hitscan`, `shell_direct`, `guided_missile`) is deterministic, with no hit roll. A shot that is legal at firing time hits at impact time unless the target has died, left the layer, gained full cover or been dropped by EW jamming (guided only). Moving targets reduce damage deterministically: if target speed exceeds the weapon's tracking threshold (slow 150 cm/s, medium 350, fast 700, guided unlimited), damage ×0.75. This is the readable "light vehicles dodge tank guns" rule.
2. **Indirect fire** (`ballistic`, `lobbed`, `bomb_drop`) scatters. The impact offset is drawn from the **server-only** stream `PCG32(seed = hash(match_seed, shooter_stable_id, weapon_slot, shot_counter))` inside a disk of radius `lerp(ScatterMinCm, ScatterMaxCm, d / RangeCm)`. An observed aim point (spotter, designator, artillery-targeting aircraft) scales the scatter by 0.7. A fixed seed reproduces the same impacts in tests.
3. **Guided missiles** can be defeated: inside an active jamming field (§9.5) a guided weapon falls back to the `ballistic` model with `ScatterMaxCm = 300`.
4. **No client prediction of damage.** Presentation can animate a projectile only after the server publishes a shot event for that observer (I: `ShotCueFor`).

### 6.7 Area of effect and friendly fire (P)

- Damage at distance `r` from impact is `Damage × eff% × falloff(r)`. Falloff is 100 % inside `FullRadiusCm` and linear down to `EdgePercent` at `MaxRadiusCm`.
- **Friendly and allied units inside an AoE take 50 % damage**, so artillery and mortar positioning matters. Neutral Unaligned assets take 100 % and generate a jurisdiction incident (E: graduated response).
- Squads hit by AoE lose HP as one pool. Prone or pinned infantry (§7.2) take ×0.7 from HIGH_EXPLOSIVE and SMALL_ARMS: the adopted concept is RA2 `ProneDamage`, with POLITIKA values.
- Garrisoned infantry take AoE through the building rules in §6.9.

### 6.8 Range, line of fire, arcs and minimum range (P)

- Direct fire needs line of fire (I for infantry). Vehicles use the same test from a muzzle socket height.
- Indirect fire needs the target, or the ground aim point for Fire Mission orders, to be **observed by the firer's team** at firing time (E: [Visibility](RTS_VISIBILITY_AND_INTELLIGENCE.md)). Attack-ground on unobserved ground is allowed but has double scatter. It never reveals hidden units (E: AI architecture §5.3).
- `MinRangeCm` blocks firing. AI and auto-acquire skip targets inside it, and the unit repositions only under the Aggressive stance.
- Hull-mounted weapons (tank destroyers, assault guns) need the hull within ±`MountArcDegrees`/2 of the target, so the hull turns first.

### 6.9 Garrison and cover (P; U-08)

- Garrison: eligible infantry enter eligible owned or neutral-permitted structures (Unaligned permission rules apply, E). Garrisoned squads fire through building ports at +15 % range. They take 50 % of SMALL_ARMS, PRECISION and HIGH_EXPLOSIVE damage, and **200 % INCENDIARY** plus +50 % grenade damage (flame and grenadiers clear buildings). Occupants eject at 20 % structure HP.
- Map cover: light cover gives −25 % SMALL_ARMS, PRECISION and HIGH_EXPLOSIVE; heavy cover gives −50 %. It applies only to infantry standing in authored cover volumes. The current map schema has no cover data (U-08).

### 6.10 Death, wrecks and passengers (P)

- Vehicles at 0 HP become wrecks for 60 s. Wrecks block movement for F2 and above and are salvageable (§10.5). Fuel- and ammunition-laden vehicles (tankers, artillery carriers, ammunition trucks) carry a **death blast**: HIGH_EXPLOSIVE at 10 % of max HP in a 250 cm radius (U-07 adjacent).
- Passengers of a destroyed ground transport eject with 50 % HP loss. Passengers of a destroyed aircraft or submerged submarine die. Ships beach passengers only if they are adjacent to the shore.

## 7. Resolve, suppression and Discipline (P, implementing E)

### 7.1 Meter

There is one per-unit `UnitResolve` (E: one meter, shown as Discipline for combatants and Morale for non-combatants; [Propaganda and Morale §4](PROPAGANDA_AND_MORALE.md#4-one-resolve-meter-two-names)). PB range 0–100 with `BrokenThreshold = 0` (fully broken, E wording). Population Morale sets the **baseline maximum** between 80 and 100 (P: linear in faction Population Morale) and scales production output (E).

### 7.2 Tiers and effects

Effects in this table apply to infantry. Vehicles use the "Rattled" column.

| Tier | Resolve | Infantry effect | Crewed-vehicle effect ("Rattled") |
| --- | --- | --- | --- |
| Steady | 60–100 | none | none |
| Shaken | 35–59 | −15 % weapon cooldown speed | none |
| Pinned | 10–34 | −40 % cooldown speed; cannot advance toward the stress source (only lateral or away moves are accepted, with typed rejection feedback); ×0.7 SMALL_ARMS and HIGH_EXPLOSIVE damage (prone) | −20 % cooldown speed |
| Breaking | 1–9 | Automatic **Withdraw** to the nearest friendly support cluster or producer, overriding non-Stop orders; Hold Fire | −30 % cooldown speed; turret traverse halved |
| Broken | 0 | As Breaking. **Only here** may Concord Psy-Ops elimination or Ascendancy Intimidation conversion resolve, and only if all of their own predicates hold (E: low Discipline alone never triggers them) | as Breaking |

### 7.3 Stress sources (P)

- `SuppressionStress` per hit from weapons with suppression. Defaults: SMALL_ARMS 2–4, HIGH_EXPLOSIVE 6–12, INCENDIARY 10, PRECISION 8 on a kill shot. Vehicles ignore SMALL_ARMS suppression.
- Casualties: −15 when a squad loses a member; −8 for each friendly unit destroyed within 600 cm.
- Heavy hit: −10 for any single hit of ≥ 15 % max HP, which is the main vehicle source.
- Ability stress: Psy-Ops, Intimidation, coercive loudspeakers and similar (per ability).
- Food shortage: Discipline recovery blocked (§4.5).

### 7.4 Recovery and support (P, faction rules E)

- Base recovery: +4 per second after 5 s without incoming stress.
- **Isolation** (E: normal small-group penalty for all except Concord): fewer than 2 other friendly combat units within 800 cm means recovery −50 % and stress taken +25 %. **The Atlantic Concord is exempt** (E).
- **Ordinary group support** (E: Directorate, Ascendancy, Meridian, Unaligned; Concord too, because the ordinary benefit is not denied to it): 3 or more coherent friendly combat units within 800 cm give recovery +50 %, capped.
- **Very-massive-army bonus** (E: Directorate and Ascendancy only): 12 or more coherent friendly combat units within 1500 cm in one connected support component give stress taken −20 %, capped. It is computed from actual positions, never from selection or control-group size (E).
- Officers and commanders: a bounded rally or recovery aura (per unit), subject to the [commander influence invariant](COMMANDER_TALENT_SYSTEM.md#33-commander-influence-invariant).
- Civilians and non-combatants receive **no** military formation bonus (E).

### 7.5 Eligibility classes for faction-exclusive resolve abilities (P)

The owning rules (E) are Psy-Ops in [Espionage §1.1](ESPIONAGE_AND_INTIMIDATION.md#11-atlantic-concord-psy-ops) and Intimidation in [Espionage §3](ESPIONAGE_AND_INTIMIDATION.md#3-iron-ascendancy-exclusive-ability-intimidation). This framework proposes the unit-eligibility flags that those rules call "eligible":

| Unit class | `PsyOpsEligible` | `IntimidationEligible` | Rationale |
| --- | --- | --- | --- |
| Ordinary infantry (line, AT, heavy weapons, support) | yes | yes (when isolated and broken) | Core target of both systems |
| Specialist infantry and agents | yes | yes | Same |
| Elite commanders | **no** | **no** (talent-doc default) | Avoids deleting or stealing talent appointments |
| Soft and light crewed vehicles (≤ 3 crew) | yes | yes | Bounded value at risk |
| Medium and heavy armor, super-heavies | no | no | Prevents an expensive one-shot swing; counterplay remains conventional |
| Aircraft (airborne), naval heavy | no | no | Layer and value |
| Naval light | yes | no | Psy-Ops can disrupt; conversion of vessels needs a separate rule |
| Civilian non-combat (Unaligned) | rampage only when an authored attack action exists (E) | only with explicit hostile-eligibility rules (E: not by default) | Sovereignty |
| Scenario-protected (for example UN-INF-14) | no | no | Scenario rule |

## 8. Ordinary veterancy (P)

Ordinary veterancy is **separate from commander talents** (E: [Commander Talents §1](COMMANDER_TALENT_SYSTEM.md#1-product-thesis-and-non-negotiable-boundaries)). Commanders do **not** have ordinary ranks; their progression is the talent appointment.

| Rank | Cumulative XP (multiple of own `ExperienceValue`) | Bonus (multiplicative with research) |
| --- | --- | --- |
| Regular | 0 | — |
| Veteran | 2.0× | +10 % max HP; −10 % cooldown |
| Hardened | 4.5× | +20 % max HP; −15 % cooldown; scatter ×0.85; +10 maximum resolve |
| Elite | 8.0× | +30 % max HP; −25 % cooldown; scatter ×0.75; +15 maximum resolve; out-of-combat self-repair or first aid 0.5 % max HP per second after 8 s without damage |

- `ExperienceValue` defaults to `USD cost / 10`; Unaligned units use their mobilization weight.
- Earned XP equals the victim's `ExperienceValue` × the killer's share of damage dealt in the victim's last 20 s, summed per unit. Credit goes only to units that dealt damage (P).
- **Promotion preserves the health ratio** and never fully heals (E: research doc §9.1 principle).
- **No XP** is earned from friendly or allied units, non-hostile Unaligned units and civilians, Psy-Ops rampage victims, self-destructed units, units destroyed by sabotage charges (the attacker is not a combatant kill), or the same victim twice. This anti-farm list mirrors talent OXP rules.
- Squads keep their rank when replenished. Non-combat units (unarmed logistics, constructors, unarmed scouts, civilians) have no ranks.
- Presentation: rank chevrons on the health bar plus a promotion audio cue. Ranks are **visible to observers who can see the unit** (P; Generals and RA2 precedent). Exact XP is owner-only.

The Tempest Rising config declares three veterancy statuses, `Veteran`, `Hardened` and `Elite` (§19). POLITIKA adopts the *three-rank concept* and its own names and values.

## 9. Detection, concealment, deception and electronic warfare (P, implementing E)

There is no literal invisibility (E: roster rule 7). Every hidden state has a range at which ordinary sight defeats it, plus a tell.

### 9.1 States

| State | Who | Effect on enemy observation | Broken by |
| --- | --- | --- | --- |
| **Concealed** | Specialist infantry, recon teams, marksmen and ambush squads stationary ≥ 3 s; vehicles inside an AC-ARM-11 screen; units in smoke | Observed only within `max(250 cm, 35 % of observer sight)` or by a `DETECT.CONCEALED` source | Firing (revealed 4 s), moving faster than 50 % speed, taking damage |
| **Disguised** | Covert agents with a disguise ability | Enemy observers receive the **disguise archetype ID** (civilian or enemy infantry), never the true one | `DETECT.SPY` source within range, hostile action, failed infiltration |
| **Submerged** | Submarines | Not observable except by `DETECT.SUBMERGED` within range | Firing torpedoes (revealed 4 s), surfacing |
| **Radar contact** | Any air unit inside an enemy air-radar footprint | An observer gets a *contact* (position band, layer) without identity, through the observation contract's `Reported`/`LastKnown` distinction (E) | Leaves the footprint; jamming suppresses contacts |
| **Decoy** | Market decoy convoys and false emplacements | Shown as the impersonated archetype | Within 400 cm, any hit, or `DETECT.SPY` |

### 9.2 Detector classes

| Flag | Detects | Typical sources |
| --- | --- | --- |
| `DETECT.CONCEALED` | Concealed units within detector range | Recon infantry, marksmen, scout vehicles (60 % of sight), observation masts, listening posts |
| `DETECT.SPY` | Disguised agents and decoys | Counterintelligence officers, police, security troops, state security; strongest in Unaligned units (E: strongest counterintelligence) |
| `DETECT.SUBMERGED` | Submerged submarines | Frigates, destroyers, patrol aircraft, coastal sonar |
| `DETECT.AIR_RADAR` | Air contacts (identity requires visual sight) | AA radar vehicles, SAM sites, AEW aircraft, radar structures |
| `DETECT.EMITTER` | Active jammers, radars and transmitters, as contacts | Signals intercept teams, counter-battery radar, EW vans |
| `DETECT.ARTILLERY` | Origin points of enemy indirect fire, as a contact for 10 s | Counter-battery radar carriers |

### 9.3 Reconnaissance (E rules)

- Opening scouts are established (E: [Reconnaissance](RECONNAISSANCE_AND_SCOUTING.md)): Concord and Ascendancy have cheap **unarmed** planes, the Directorate has **rifle-armed motorcycles**, and the Meridian Pact has **very fast, cheap, lightly armed and armored trucks** with the best overall early reconnaissance.
- Scouting never steals intelligence, never grants sabotage permission and never verifies a planted report by itself (E).

### 9.4 Deception and false intelligence (E)

Only the Unaligned plants false intelligence (E). Unit specifications may reference `Reported` intelligence objects but never change world truth from them. Meridian market decoys (MP-MKT-06) are **physical** dummy units, not reports (E-R).

### 9.5 Electronic warfare (P)

- **Jamming field** (EW vans and jammers): radius 1200–1600 cm. Inside it:
  - enemy `DETECT.AIR_RADAR` contacts are suppressed;
  - enemy guided weapons degrade to ballistic scatter (§6.6);
  - enemy commander and officer aura links that pass through the field are cut;
  - enemy counter-battery `DETECT.ARTILLERY` is blocked.

  Jamming **never hides units from visual sight**. An active jammer is a `DETECT.EMITTER` contact to enemies within 3000 cm: the counterplay tell.
- **Smoke** (smoke trucks, grenadier and mortar smoke rounds): a 400 cm cylinder for 20 s. It blocks line of sight and line of fire, and units inside count as Concealed.
- **Signals intercept**: reveals enemy emitters, and intercepts `Reported` intelligence objects addressed to the target participant. It is vulnerable to planted Unaligned evidence (E-R AC-INF-13).

## 10. Logistics: transport, repair, medical, resupply, salvage and capture (P)

### 10.1 Transport

- Capacity is counted in **infantry slots**. A squad, team, agent, officer or commander occupies 1 slot. Heavy transports also have **vehicle slots**: a light vehicle needs 1, and medium vehicles need 2 on the largest naval and air carriers only.
- Load 0.5 s per slot, unload 1.0 s per slot, and the unload point must be on the passengers' legal nav layer. Paradrop (fixed-wing) and fast-rope (helicopter) are separate abilities with 6–10 s exposure windows.
- Passengers cannot fire unless the vehicle has **fire ports** (`FirePortSlots`). Fire-port weapons fire at −25 % cooldown speed.
- **Transport manifests are owner-only state** (MPA-TRANSPORT, §15). Enemies see a loaded indicator only when the unit is observed and the indicator is visible (doors and ramp state are cosmetic).

### 10.2 Repair (vehicles, aircraft, ships)

- Repair vehicles and depots restore **20 HP/s** (vehicle) or **12 HP/s** (aircraft on pads) to one target within 300 cm. A full repair of a unit costs **25 % of its USD cost**, charged in proportion to HP restored, and pauses at $0 (P; C&C precedent of 15–20 %, §19). Research can reduce the price (E: research doc *Vehicle Component Standardization*).
- Field repair by engineers: 8 HP/s, same USD rule, vehicles only.
- Repair never restores destroyed units. Wreck recovery is §10.5.

### 10.3 Medical support and replenishment (infantry)

- Medics heal **6 HP/s** to one squad within 300 cm, capped at `livingMembers × MemberHealth` (§5.4). Healing is free in USD and costs time and exposure.
- **Replenishment** restores missing members to full strength. It happens at a friendly producer (Training Center, Barracks or equivalent) within 600 cm, or through a support ability (for example AC-INF-05 *Casualty Replacement*). It costs `(missing / members) × unit price × 0.8` USD and 1.5 s per member, and adds food staff from the next upkeep step (E: new units enter upkeep on the next step).

### 10.4 Ammunition and resupply

- Infantry: magazine and reload (I). There is no global ammunition limit for ordinary infantry weapons.
- Limited-ammunition weapons declare `AmmoCapacity`: rocket artillery, missile batteries, ATGM carriers, mine layers, aircraft munitions, mortars (large magazine) and charges. When empty, the weapon is unusable until resupplied by:
  1. a logistics convoy or supply truck within 400 cm (**Resupply** aura, 1 ammo unit per 2 s per target, up to 4 targets);
  2. a producer, depot or airfield;
  3. aircraft returning to base (automatic rearm costs kerosene per §4.5).
- Resupply costs USD at a quarter of the weapon's per-shot share of unit price, per unit restored (P, U-09). This gives logistics vehicles a battlefield job without a per-shot purchase chore.

### 10.5 Salvage, refunds and recovery

- Units are **not sellable** (P).
- Production cancellation and refunds are **U-10**. The production contract says cancellation must be explicitly defined (E). Recommendation: a full refund of the reserved but unspent USD; no refund after completion.
- **Wreck salvage:** recovery-capable vehicles (`ability.shared.salvage_wreck`, for example AC-ARM-13, RD-SUP-04, IA-SUP-05, MP-SUP-08, MP-MKT-01) can, within 300 cm and in 6 s, convert a wreck into USD equal to 10 % of the wreck's unit cost for their owner. Allied wrecks yield 0 USD but are cleared. Unaligned-jurisdiction wrecks need permission (E: jurisdiction).
- **Battlefield recovery** of the recovery vehicle's own team's heavy armor (P, U-11): towing a wreck of its own heavy or super-heavy unit to a repair depot within 60 s re-creates the unit at 25 % HP for 40 % of its cost. This rewards Concord and Ascendancy preservation doctrine (E: high equipment costs encourage recovery).

### 10.6 Capture

- Engineers and pioneers capture eligible **structures** (E-R), over 8 s of uninterrupted channel. Unaligned structures are capturable only under scenario and jurisdiction rules (E); capture is never instant, because the RA engineer-rush risk is avoided (§19).
- Vehicle capture: there is no general capture. Meridian MP-ARM-13 *Captured Heavy Armor* is produced through market or battlefield acquisition rules, not by a capture order (E-R). Ownership fields record `sourceFaction` and `acquisitionMethod` (E-R suggested model). Stolen equipment never rewrites manufacturing permission (E-R).
- Intimidation conversion is an Ascendancy-only ownership transfer of the **same unit** (E) and is not capture.

## 11. Production and economy conventions (P, implementing E)

### 11.1 Currency and payment

- All prices are **USD** (E: [Resource Economy](RESOURCE_ECONOMY.md), locked 29 October 2026). The cost is reserved and debited exactly once at queue admission (E), production advances only in a powered producer (E), and completion spawns the unit at the producer's exit.
- Some units also cost **petroleum** at production (§4.5). Aircraft cost **kerosene** per sortie. Nuclear materials are consumed only by nuclear-production structures (E), never by units. The exception is that nuclear delivery units carry a completed weapon object (E).

### 11.2 Producers and prerequisites

- The producer is always a roster building (E-R ID). Prerequisites are building IDs plus `tech.*` IDs (§12), with no ages or eras (E).
- A producer has **one work lane** shared by production and research (E: research doc §8 default). A second lane needs an explicit modernization.

### 11.3 Unique and limited units

| Rule | Units | Rationale |
| --- | --- | --- |
| **Commander cap 1** per participant (E: talent doc §4.3) | AC-INF-14, RD-INF-13, IA-INF-13, MP-INF-13 | One active appointment |
| **Prototype cap 1** | AC-ARM-15, IA-ARM-06, RD-ARM-14 (per unit spec) | Roster: "rare", "single" and "limited slots" (E-R) |
| **Strategic delivery cap 1** | AC-AIR-11 nuclear bomber | Bound to the completed weapon (E) |
| **Special-operations cap 2** | Coastal commandos, special forces raiders and similar (per unit) | "Limited availability" (E-R) |
| **Market stock** | MP-MKT-* | Finite auction stock and delivery (E) |
| **No global population cap** | — | Food upkeep is the soft cap (E design intent). A technical per-participant entity cap for performance and MP budgets is **U-12** |

### 11.4 Contract and mercenary units (E: Meridian contracts)

Contract units carry `ownerFaction`, `employer`, `contractExpiryStep` and `wagePerInterval` (E-R metadata). Wages are USD per 30 s upkeep interval, debited exactly once like food. On expiry or unpaid wages, the unit stops accepting new orders, walks to the nearest exit or contract office and is removed with no refund. The UI warns 60 s before expiry. Unpaid status is visible only to the employer.

### 11.5 Market units (E: Black Market is conventional only and never lists nuclear items)

Market items are bought at MP-BLD-15 under finite stock. They arrive physically via MP-SUP-09, MP-SUP-02 or MP-SEA-01 delivery, which enemies can interdict (E: interdiction and arrival delay). Price is USD at the quoted auction value. A market unit never unlocks native factories (E-R).

### 11.6 Unaligned mobilization (E: NPC-only)

Unaligned units are never purchased. Each has a **mobilization weight** (a USD-equivalent abstract budget) and a **response tier** (§14.3). The institutional AI spends a scenario-authored budget that refills over time and scales with Unaligned Population Morale. It never receives player money or forged resources (E).

## 12. Research and upgrade hooks (P, implementing E)

Ownership: [Technological Research](RTS_TECHNOLOGICAL_RESEARCH.md) owns the ontology, admission, effects and validation. Unit specifications only state **which** technologies apply to a unit and with what effect.

- Shared technologies (candidates from research doc §6, reused by name): `tech.shared.infantry_protective_equipment`, `tech.shared.field_medical_procedures`, `tech.shared.vehicle_component_standardization`, `tech.shared.tactical_radio_modernization`, `tech.shared.radar_signal_processing`, `tech.shared.electronic_countermeasures`, `tech.shared.supply_preservation`.
- Faction technologies: `tech.ac.precision_vehicle_targeting`, `tech.rd.industrial_mobilization_methods`, `tech.ia.advanced_clandestine_training`, `tech.mp.modular_commercial_equipment`. Unit-specific technologies proposed in the faction files use `tech.<f>.<name>`.
- Espionage gates: the late spy-only sabotage gate is `tech.{ac,rd,ia}.late_espionage`, which maps to the implemented `late_espionage_unlocked` flag (I, `CanPlantExplosives`). The Ascendancy's full spy progression is `tech.ia.espionage_tier_1` → `_2` → `_3`, and only `_3` permits nuclear-research infiltration (E).
- **Every upgrade has a trade-off or opportunity cost:** a USD price, a lane block, a weight or speed penalty, a fuel or kerosene cost, or an exclusive doctrine group. Pure "+X %" chains longer than two steps are rejected (E: research doc §6 guidance).
- Effects apply to existing and future eligible units (E: C&C3 pattern) and never full-heal (E).
- Nuclear restrictions are hard invariants (E: research doc §9.4). No unit upgrade can touch them.

## 13. Command integration (I/P)

### 13.1 Orders

| Order | Status | Notes |
| --- | --- | --- |
| Move, Attack, AttackMove, Stop, Guard | **I** | Existing typed admission and rejection; per-unit within groups (I: group dispatch contract §3A is proposed, the single-unit path is implemented) |
| Grenade, Reload, Sprint | **I** (infantry behavior) | Reused as shared abilities `ability.shared.grenade`, `.reload` and `.sprint` |
| Patrol | P | Loop between 2–4 points; AttackMove semantics between waypoints |
| Withdraw / Retreat | P (E: formations doc names Withdraw as future) | Valid explicit destination; Hold Fire while moving; `Breaking` units auto-withdraw |
| Fire Mission (attack ground) | P | Indirect weapons only; never reveals hidden units (E) |
| Deploy / Undeploy | P | Units with deployed states; 2–6 s transitions; explicit vulnerability window |
| Load / Unload, Garrison / Exit | P | §10.1, §6.9 |
| Repair / Heal / Resupply / Salvage | P | Support abilities; automated under Guard (§13.3) |
| Capture / Build / Clear / Breach / Bridge | P | Engineering abilities |
| Plant Charge / Infiltrate / Disguise / Psy-Op / Intimidate / Steal / Divert / Contaminate | P (permissions E) | Faction-gated per owner docs |
| Designate Target / Spot | P | Grants observation of a point or target to friendly indirect fire for a duration |
| Return to Base | P | Aircraft; automatic when sortie ammunition is spent |
| Scatter | P | Moves each member of a selection 200–400 cm away from an incoming area threat (a C&C staple) |

### 13.2 Stances (P; E: formations doc §5 names them)

Aggressive, Guard, Hold Ground and Hold Fire are independent from orders. The implemented `GuardOrder` is an order, not the Guard stance (E: formations doc). Each template declares a default stance (§3.2). A unit order never clears the stance, and a stance change never replaces the order (E).

### 13.3 Automated behaviors (P)

- Auto-acquire (I for infantry: idle auto-acquire within 800 cm). It extends to all armed units, limited by effectiveness (§6.4), stance and visibility.
- Support units under Guard auto-heal, auto-repair or auto-resupply eligible friendly units within 600 cm.
- AA units under Aggressive auto-engage air only, and auto-engage ground only if the weapon is ground-capable and the target is ≥ 15 % effective.

### 13.4 Groups, control groups and formations (E rules; P application)

- Selection and the 0–9 control groups are local and never formations (E).
- Mixed groups use per-member admission: non-capable members receive "not eligible" feedback (E: group-orders §3A).
- Each unit specification states a **formation role**: `frontline`, `flank`, `rear_guard`, `support_weapon`, `scout_screen`, `escort`, `reserve` or `protected` (E: formations doc §8 role names). Roles are slot preferences only.
- **Formation leadership:** only commanders, and officers (AC-INF-10 is not an officer; RD-INF-10, IA-INF-12, UN-INF-13 and similar), may hold the `formation_anchor` preference. This gives no authority beyond the bounded aura in their unit specification. Ordinary units never lead with a bonus (E: talent doc).

### 13.5 Special command-capable units (P)

| Kind | Units | Capability |
| --- | --- | --- |
| Elite commanders (talent-eligible) | AC-INF-14, RD-INF-13, IA-INF-13, MP-INF-13 | Command radius, talent tiers I–IV, command abilities. Eligibility for IA-INF-13 and MP-INF-13 still needs explicit owner approval (E: talent doc §5) |
| Officers (no talents) | RD-INF-10, IA-INF-12, AC-INF-12 (Psy-Ops officer, offensive), UN-INF-13 (NPC) | Bounded rally/discipline auras or faction abilities; no talent tree |
| Command vehicles | RD-ARM-14, UN-ARM-15, IA-SEA-06, RD-SEA-07, MP-SEA-07 | Mobile support hubs per unit; **not** talent appointments |

## 14. Unit AI profile library (P)

Ownership: [RTS AI Architecture](RTS_AI_ARCHITECTURE.md) owns layers, observation and precedence (E). The profiles below are **local unit autonomy** and **tactical defaults** consumed by both human-owned units (local autonomy still runs for humans, E) and computer opponents. They use only observed information (E: no hidden-truth scanning) and the same command admission (E).

### 14.1 Shared decision logic

1. **Legality first** (E): hard gameplay legality > explicit order > formation directive > stance > profile > idle.
2. **Threat identification:** observed hostile entities within acquisition range (I: 800 cm infantry default; P: `max(800, 0.9 × longest weapon range)` for vehicles). Contacts and reports are never targets (E).
3. **Target score** (deterministic; ties are broken by distance, then stable ID):
   `score = eff%(best weapon) × W_class(profile) × (1 + 0.5 × (1 − hp_ratio)) × range_factor × (1.25 if target is attacking self or a protected ally) × (1.5 if target is the explicitly designated focus)`.
   `range_factor` is 1.0 in range, 0.6 if reachable by one step of advance under Aggressive, and 0 otherwise.
4. **Re-evaluation cadence:** every 10 steps (0.5 s) or on events (damage taken, target lost, ally destroyed). There is no per-frame retargeting.
5. **Engage / disengage:** profile thresholds use HP ratio, resolve tier, local force ratio (sum of CV of observed hostiles against friendlies within 1500 cm) and ammunition.
6. **Retreat** goes to the nearest friendly repair or medical source, else producer, else the last safe waypoint. A retreat is a Withdraw order through admission (E: no hidden teleport).
7. **Ability use** follows explicit per-ability criteria in each unit specification. No ability fires on a target that the participant cannot observe.

### 14.2 Profiles

| Profile | Threat weights `W_class` (INF / VEH_L / VEH_H / AIR / STR) | Preferred distance | Engage when | Disengage / retreat | Ability and support rules | Typical combinations |
| --- | --- | --- | --- | --- | --- | --- |
| `ai.unit.line_infantry` | 1.0 / 0.6 / 0.1 / 0.3 / 0.4 | 85 % of max range | Any eligible target ≥ 15 % | Pinned: seek cover or hold; Breaking: auto-withdraw (§7) | Grenade on garrisons or clumps of 3+ infantry; sprint to cover when Shaken | With AT infantry and medics; screen for armor |
| `ai.unit.anti_armor_infantry` | 0.3 / 1.0 / 1.2 / 0.6 (if AA-capable) / 0.5 | 90 % | Vehicles first; infantry only when no vehicle is observed | Retreat at 35 % HP or when only infantry threaten at < 300 cm | Hold Ground ambush when concealed | Behind line infantry; in buildings |
| `ai.unit.support_weapon` | 1.0 / 0.5 / 0.1 / 0.2 / 0.6 | Deployed position | When deployed | Undeploy and withdraw when flanked (threat outside arc) | Deploy facing the threat axis; suppress the largest infantry cluster | Anchors a line; needs a screen |
| `ai.unit.specialist_infantry` | 1.3 (high-value infantry: officers, AT, HW) / 0.1 / 0 / 0 / 0 | max range | Concealed shot on high-value infantry | Relocate after 2 shots or on reveal | Stay concealed; spot for artillery | Paired with artillery or a scout |
| `ai.unit.support_infantry` | n/a (unarmed) | 300 cm from the patient | — | Flee from any observed threat within 600 cm | Heal or repair the lowest HP ratio first, then the highest value | Follows its assigned group |
| `ai.unit.covert_agent` | n/a | Out of detector range | — | Abort when a `DETECT.SPY` source closes within its range | Infiltrate only when the path avoids known detectors; never auto-attack | Escorted only by deception |
| `ai.unit.commander` / `ai.unit.officer` | 0.8 / 0.5 / 0.3 / 0.2 / 0.3 | Behind the frontline by 400–800 cm | Self-defense only | Retreat at 50 % HP (commander) or 35 % (officer); never lead an assault | Rally Breaking units in radius; talent abilities per criteria | Inside a perimeter formation |
| `ai.unit.scout` | 0.6 / 0.6 / 0 / 0 / 0.2 | Sight edge | Only targets ≤ its own CV | Flee from anything with CV ≥ its own; never idle in enemy base radius | Route reconnaissance; report contacts | Alone, or as a forward screen |
| `ai.unit.skirmisher` | 1.0 / 1.0 / 0.2 / 0.3 / 0.3 | 90 % (kite) | Light targets; outnumbering | Disengage from heavy armor or AT and return after 6 s | Hit-and-run; Withdraw then re-engage | Raiding groups |
| `ai.unit.transport` | n/a or 0.5 / 0.3 / 0 / 0 / 0 | Out of AT range | Self-defense | Unload and withdraw at 40 % HP | Unload at the order point, or at cover when under fire | With escorts |
| `ai.unit.ifv` | 0.9 / 1.0 / 0.4 / 0.6 / 0.4 | 85 % | Light vehicles and infantry | Withdraw at 35 % HP from heavy armor | Unload passengers near objective cover | Mechanized infantry groups |
| `ai.unit.battle_tank` | 0.5 / 0.9 / 1.2 / 0 / 0.6 | 90 % | Armor first | Withdraw at 30 % HP if a repair source is known (Concord and Ascendancy 35 %, preservation doctrine) | Turret leads, hull faces the threat | With infantry and AA |
| `ai.unit.heavy_armor` | 0.5 / 0.8 / 1.3 / 0 / 1.0 | 80 % | Fortifications and armor | Withdraw at 25 % HP; never alone | Deploy and siege abilities only with an escort present | Spearhead with an AA escort |
| `ai.unit.tank_destroyer` | 0.1 / 1.0 / 1.4 / 0 / 0.3 | max range | From ambush | Relocate when an infantry threat is < 500 cm | Hold Ground; fire then reposition | Behind a screen |
| `ai.unit.artillery` | 1.0 (clusters) / 0.8 / 0.6 / 0 / 1.2 | 70–95 % of range, outside enemy direct-fire range | Observed targets or authorized Fire Missions | Displace after 3 salvos when an enemy `DETECT.ARTILLERY` is suspected; withdraw from any direct threat | Prefer clusters (≥ 3 in AoE); never fire into friendlies below a 25 % expected friendly-damage share | Needs a spotter |
| `ai.unit.air_defense` | 0 / 0.2 / 0 / 1.5 / 0 | Inside the protected group | Air within range | Retreat when ground threats dominate | Hold Fire on the ground while air is present | Covers armor and artillery |
| `ai.unit.electronic_warfare` | n/a | Behind the frontline | — | Relocate when its emitter is detected and hostiles approach | Jam when enemy guided weapons or air radar are present within the field | With artillery and air defense |
| `ai.unit.engineer_vehicle` | n/a | Task point | — | Withdraw from direct fire | Clear, breach or bridge on order; build cover under Guard | Escorted |
| `ai.unit.logistics` / `ai.unit.constructor` | n/a | Route nodes | — | Reroute around observed threats; flee | Deliver, resupply and repair automatically when they are assigned | Convoys with escorts |
| `ai.unit.air_scout` | n/a | Orbit outside known AA footprints | — | Abort at the first AA hit | Patrol routes; never overfly known SAM | Opening reconnaissance |
| `ai.unit.air_superiority` | 0 / 0 / 0 / 1.0 (bombers 1.3, transports 1.2) / 0 | Missile range | Air targets | Return to base when ammunition or fuel is spent, or at 40 % HP | Intercept inbound raids | Escorts strike aircraft |
| `ai.unit.air_strike` | 0.6 / 1.0 / 1.2 / 0 / 1.2 | Overflight | Designated or high-value target | Abort if more than 2 observed AA systems cover the path | One pass, then return to base | After SEAD or an AA kill |
| `ai.unit.attack_helicopter` | 0.6 / 1.0 / 1.1 / 0.3 / 0.4 | 90 % | Armor outside AA cover | Retreat at 40 % HP or on AA lock | Pop up from terrain masking (if it is authored) | With ground forces |
| `ai.unit.air_transport` | n/a | Safe corridor | — | Abort the drop if AA is observed in the corridor | Unload at the nearest safe landing zone | Escorted |
| `ai.unit.air_surveillance` | n/a | Orbit at the radar edge | — | Retreat on interceptor contact | Maintain orbit | Fighter cover |
| `ai.unit.naval_patrol` / `ai.unit.naval_combatant` / `ai.unit.submarine` | per weapon | per weapon | Water-layer threats | Retreat at 30 % HP to a port | Submarines stay submerged until a firing solution exists | Map-dependent |
| `ai.unit.civilian_ambient` | n/a | Route | — | Flee from combat within 1000 cm; seek shelter | — | — |
| `ai.unit.npc_security` | per unit | per unit | Only per response tier (§14.3) | Per tier | Escalation ladder only | Institutional patrols |

### 14.3 Unaligned graduated response tiers (P, implementing E)

The E rule is that minor incidents lead to warnings, inspections and restrictions, and serious violations lead to police intervention, mobilization and JNA engagement ([Unaligned description](../Factions/TheUnaligned/description.md)). Proposed tiers:

| Tier | Trigger (examples) | Available units (by category) | Behavior |
| --- | --- | --- | --- |
| R0 Normal | No incident | Civilians, police patrols, border guards, routine JNA patrols | Ambient; inspections |
| R1 Administrative | Unauthorized movement in a jurisdiction; minor property damage | Police, border guards, investigators | Warnings, checkpoints, access restrictions |
| R2 Policing | Attacks on civilians or infrastructure; detected spies | Police response, state security, counterintelligence | Arrest attempts, armed police response, investigation of covert actions |
| R3 Mobilization | Repeated or serious violations; occupation of government sites | Territorial Defence, JNA infantry and light vehicles, rocket infantry | Defensive deployment, territorial patrols, engagement of violators |
| R4 Military engagement | Attacks on JNA sites or federal institutions; sovereignty violation by armies | JNA armor, artillery, aviation and naval forces | Direct engagement of the violating participant only |

Hostility is per participant (E: the Unaligned never becomes a fifth playable side and never vetoes a lawful victory). The identified Ascendancy carries a standing hostility modifier (E: Unaligned special hostility toward identified Ascendancy forces).

## 15. Multiplayer authority classes (P, implementing E)

The E rules come from the [Multiplayer Architecture](RTS_MULTIPLAYER_ARCHITECTURE.md): the server is authoritative, clients submit intents through the owning PlayerController, replication is filtered by team fog, the server is never lockstep, and replicated-but-hidden data is no security boundary. Every unit belongs to one or more classes:

| Class | Server-authoritative state | Replicated to observers who can see the unit | Owner-only | Never replicated to opponents | Exploit surfaces to test |
| --- | --- | --- | --- | --- | --- |
| **MPA-STD** | HP, position and facing, order, target, cooldowns, ammunition, resolve, veterancy XP, statuses, ownership | Transform (quantized), HP, archetype ID, rank, visible status icons (suppressed, burning), shot cues under `ShotCueFor` (I) | Exact order and target, cooldowns, ammunition, XP, resolve value | Hidden-unit positions; queued intents | Order spoofing for another participant's unit (I: rejected by connection binding); target-ID probing to discover hidden units (rejection must be generic, E); cooldown reset by reconnect |
| **MPA-COVERT** | True archetype, disguise archetype, concealment, planted charges, infiltration progress | **Disguise** archetype (if disguised); nothing if concealed | True identity, charge timers | True identity and charge existence until detected or detonated | Archetype leak through replicated class or animation set or sound (presentation must use the disguise profile); detection probing |
| **MPA-TRANSPORT** | Passenger manifest, slots | Loaded/empty indicator only when observed | Manifest | Manifest contents | Manifest leak in UI or tooltip; unload into an illegal layer |
| **MPA-INDIRECT** | Shell flight, impact point, scatter seed | Impact event at the observed impact point; launch flash only if the shooter is observed (I: FacingOnly or None) | Fire mission queue | Shooter position (unless observed or counter-battery detected) | Inferring hidden artillery from impact geometry beyond the design (accepted as a gameplay tell); scatter-seed prediction (the seed never leaves the server) |
| **MPA-AIR** | Sortie state, ammunition, kerosene, flight path, landing slot | Transform when observed; radar contact band when covered (§9.1) | Sortie and ammunition | Home airfield identity, if unobserved | Infinite rearm through reconnect; contact identity leak |
| **MPA-SENSOR** | Sensor footprint, jamming field, emitter status | Emitter contact for eligible detectors | Footprint | Jamming coverage details | Fog leak through the sensor footprint (projection must include only contacts) |
| **MPA-ECON** | Cargo inventories, ledger transactions, contracts | Vehicle when observed; cargo type only if the design says it is visible (tankers: yes) | Cargo amount, contract data | Ledger | Double-delivery; delivery by a destroyed vehicle; manifest falsification (MP-SUP-06 uses explicit `falsified_manifest` state) |
| **MPA-COMMAND** | Appointment, talent state, aura membership, OXP | Commander identity and visible aura indicator when observed | Talent choices, OXP | OXP, unselected options | Talent selection replay (E: once per tier); aura stacking through duplicate appointments |
| **MPA-NPC** | Institutional AI state, response tier, hostility per participant | Units when observed; jurisdiction warnings to the affected participant | n/a | Internal hostility scores, balance-intervention reasons, false-report truth flags (E) | Provoking the Unaligned against an opponent through false attribution (allowed only under the E deception rules); probing hostility |
| **MPA-STRATEGIC** | Completed nuclear weapon custody, delivery state | Strategic warnings exactly as the nuclear contract defines | Weapon custody | Location of custody, unless the design reveals it | Duplicate weapon through custody transfer; launch without both gates |

Cosmetic-only state (never authoritative): animation, idle fidgets, voice lines, muzzle flash, tracers, smoke and fire VFX, wreck debris, camera shake, selection rings, health-bar fade, recoil, track and wheel motion.

## 16. Presentation baseline (P)

### 16.1 Animation sets

| Set | Required for first integration | Required before Core sign-off | Optional |
| --- | --- | --- | --- |
| ANIM-INF | idle, run, fire, death (I: the implemented rifle-infantry profile imports idle BIDA, run RUNA, fire ATKA, death DIEA/B/C, ready AIDA, select SIFA, deselect ABTA) | reload, grenade throw, prone/pinned loop, crawl, suppressed flinch, garrison enter/exit, sprint | fidgets, cheer, salute |
| ANIM-INF + launcher / deploy / prone | aim, fire, reload of the launcher | deploy/undeploy, crew-served loop | backblast |
| ANIM-VEH-WHEEL / -TRACK | wheel or track UV scroll, suspension, turret yaw and gun pitch, recoil, destruction | damage states (smoke at 50 % HP, fire at 25 %), wreck mesh, door/ramp | dust, mud |
| ANIM-AIR-ROTARY / -FIXED | rotor spin, takeoff, landing, banking, weapon release, crash | gear up/down, afterburner (jets), countermeasure flares | contrails |
| ANIM-NAV / -SUB | wake, turret yaw, sinking | dive/surface | spray |
| ANIM-CIV | drive or walk cycles, flee | damaged state | ambient |

Unit-specific extras (deploy masts, dozer blades, bridge laying, erector rise) are listed in each unit specification.

### 16.2 Readability, feedback and performance

- **Silhouette:** at the default gameplay zoom (implemented camera view-ray distance 900–7500 cm, FOV 65; [Test-map terrain](TEST_MAP_TERRAIN.md)), infantry must read at least 40 px tall at 1920×1080. Every unit must be identifiable by silhouette and role marker alone, never by color alone (E: [Production Standards](PRODUCTION_STANDARDS.md) RTS-zoom silhouette review).
- **Faction visual language** comes from the concept art in `Factions/*/Units/`:
  - **Atlantic Concord:** monochrome graphite concept sheets of Western late-Cold-War kit (PASGT-style helmets, load-bearing vests, NATO-pattern rifles, tracked IFV hulls). Production palette is restrained woodland green and grey with a pale-blue faction band.
  - **Red Directorate:** olive-drab Soviet-style kit (steel helmets, Kalashnikov-pattern rifles, peaked caps with red bands for officers) against industrial and rail backdrops with red banners.
  - **Iron Ascendancy:** charcoal-black armor with crimson trim, respirator helmets with red lenses and heavy, angular, menacing silhouettes. **Risk:** some sheets (for example `IA-INF-13`) lean toward stylized fantasy. Production assets must be reconciled with roster rule 7 (visually plausible 1980s equipment).
  - **Meridian Pact:** tan, olive and desert mixed kit, shemaghs and boonie hats, a green faction emblem, Mediterranean coastal backdrops and mixed-origin weapons.
  - **Unaligned:** no concept art yet. Yugoslav JNA olive-grey, titovka caps, police blue and Territorial Defence mixed uniforms are asset requirements.
- **HUD per unit:** health bar with veterancy chevrons; resolve tier icon (Discipline or Morale label, E); ammunition and reload pip for limited-ammo weapons; status icons (Suppressed, Pinned, Concealed (own units only), Disguised (own only), Jammed, Fuel-starved, Food shortage, Contract-expiring). There is one ability icon per active ability, with cooldown sweep and range preview.
- **Audio** (E: Production Standards audio priority): select, move, attack, under-fire, ability-use, death and promotion. Voice priority goes to critical events (under attack, ability ready, unit lost). The implemented rifle-infantry cue roles are select, move, attack and death plus footsteps and shots (I).
- **LOD and budgets:** benchmark-determined (E: [3D Asset Workflow](3D_ASSET_PRODUCTION_WORKFLOW.md)). Starting hypotheses to test, not limits: infantry member LOD0 ≤ 10k triangles with 3 LODs; vehicles ≤ 30k with 3 LODs; aircraft ≤ 25k; ships ≤ 60k. Textures are 1024² for infantry and 2048² for vehicles.

## 17. Commanders, elites and ordinary progression (E/P)

| System | Who | Owner | Separation rule |
| --- | --- | --- | --- |
| Commander talents (OXP, tiers I–IV) | Only typed Elite commander archetypes (E): AC-INF-14, RD-INF-13; IA-INF-13 and MP-INF-13 pending explicit approval (E) | [Commander Talents](COMMANDER_TALENT_SYSTEM.md) | No ordinary veterancy for commanders (P). Talents affect subordinates only through the command relation (E) |
| Ordinary veterancy | All armed non-commander units (P) | This framework §8 | Never unlocks talents (E) |
| Research upgrades | Participant-wide or local (E) | [Research](RTS_TECHNOLOGICAL_RESEARCH.md) | Never grants talents (E: research doc §9.5) |
| Officer auras | Designated officers (P) | Unit specification | Bounded, no tree, no OXP |
| Special forces "elite" units | AC-INF-09, RD-INF-09, IA-INF-13 (if not approved as a commander), MP-INF-10/11 | Unit specification | "Elite" in a description is not talent eligibility (E) |

Commander unit specifications in the faction files define command radius, communications dependence, leadership effects, vulnerabilities, death and redeployment, and a complete **proposed** 3-option-per-tier talent set. That set awaits owner approval under talent doc §5.

## 18. Unreal Engine implementation reference (P)

### 18.1 Mapping specification fields to data

| Specification field | Existing (I) | Proposed extension | Schema note |
| --- | --- | --- | --- |
| Roster ID, category, subclass, template, tier | — | `UPolitikaUnitArchetypeDefinition`: `RosterId` (FName), `Category`, `Subclass`, `TemplateId`, `RosterTier` | Additive optional fields with explicit defaults may stay at v1 (I rule); semantic requirements (armor, mobility) need **schema v2** with explicit migration (I: content rule) |
| Health | `MaxHealth` | `SquadMembers`, `MemberHealth` (validator: `MaxHealth == SquadMembers × MemberHealth` for squads) | v2 |
| Armor | — | `ArmorClassId` → `UPolitikaArmorClassDefinition` (stable ID) | v2 |
| Mobility | `MovementSpeedCmPerSecond` | `MobilityClassId` (nav profile, terrain multipliers), `AccelerationSeconds`, `TurnRateDegPerSec`, `FootprintClass`, `MovementLayer` | v2, U-03 |
| Sight / detection | `SightRadiusCm` (I) | `DetectorFlags` + ranges, `ConcealmentProfileId` | v2 |
| Weapons | `WeaponId` (single) | `WeaponSlots[]` {WeaponId, MountSocket, ArcDegrees, TargetLayers} | v2 |
| Weapon definition | `RangeCm`, `Damage`, `CooldownSeconds` (I) | §6.5 fields | weapon v2 |
| Infantry magazine, reload, grenade, sprint | `UPolitikaInfantryBehaviorDefinition` (I) | Keep it; reference shared ability definitions for grenade and sprint | unchanged |
| Effectiveness matrix | — | `UPolitikaDamageEffectivenessTable` (one `UPrimaryDataAsset`, damage × armor integers 0–1000 in tenths of a percent) | new type |
| Abilities | Grenade, Reload, Sprint orders (I) | `UPolitikaAbilityDefinition`: ID, kind, targeting, range, AoE, duration, cooldown, cost (USD or ammunition), permissions predicate ID, presentation keys | new type; GAS not adopted (E: talent doc §6.4) |
| Production | — | `UPolitikaProductionRecipe` owned by [Production](RTS_PRODUCTION_AND_TECHNOLOGY.md) | new type |
| Upgrades | — | `tech.*` definitions owned by [Research](RTS_TECHNOLOGICAL_RESEARCH.md) §10 | new type |
| Veterancy | — | `UPolitikaVeterancyProfile` (`ExperienceValue`, thresholds, bonuses) | new type |
| AI profile | Idle auto-acquire (I) | `UPolitikaUnitAIProfile` (weights, thresholds; §14) | new type |
| Food, fuel, cap | — | `FoodStaff`, `FuelClass`, `UniqueCap`, `ContractTerms` | v2 |
| Commander eligibility | — | `bCommanderEligible` + `CommanderDefinitionId` (E: typed property, talent doc §3.1) | v2 |
| Presentation | `VisualMesh`, `InfantryPresentation`, scale and tint (I) | `PresentationProfileId` (mesh, skeleton, animation set, VFX/SFX keys, icon) | v2 |

### 18.2 Runtime placement (E: one coordinator)

All new mechanics run inside `APolitikaPrototypeMatch`'s 0.05 s step (I), with no per-unit Tick authority and no second scheduler (E). A proposed within-step order for unit mechanics, to be reconciled with the frozen Stage 4 ordering (E: [Engine Outline Stage 4 thread](ENGINE_OUTLINE.md#stage-4-cross-system-execution-and-evidence-thread-proposed)):

`admission → ability activation → movement → weapon fire (cooldowns, projectile spawn) → projectile impacts and AoE → damage → resolve stress → deaths, wrecks, passenger ejection → veterancy credit → status expiry → recovery/regeneration → events`.

### 18.3 Validators (shared editor and runtime, E: Engine Outline §7.1 rule 4)

- Every unit has an armor class, a mobility class and at least one weapon or `bUnarmed`.
- Every weapon has a damage type and target layers, and every target layer is reachable by at least one weapon class.
- Squads satisfy `MaxHealth == SquadMembers × MemberHealth`.
- The Directorate infantry price rule (§4.4) holds.
- The `UniqueCap` is honored by the production validator.
- Commanders are `bCommanderEligible` only if they are listed in §17.
- No Meridian unit references nuclear production. Concord and Ascendancy scout planes have no weapon (E). The Directorate motorcycle scout has a rifle weapon (E). The Meridian scout truck has no sabotage ability unless one is explicitly granted (E).
- Unaligned units have no producer and no player price (E).

### 18.4 Modifier evaluation order (P)

Effective value = base definition → faction modifier (§4.4) → research (participant-wide) → local modernization → veterancy → commander or officer aura → temporary status (suppression, food shortage, fuel starvation, jamming) → clamp. Each modifier has a source ID and expiry. Definitions are never mutated (E: research doc §3.1).

### 18.5 Proposed acceptance IDs (not run)

| ID | Check |
| --- | --- |
| UNIT-01 | Effectiveness table: 0 % rejects attack orders; 1–14 % is not auto-acquired; ≥ 15 % is auto-acquired under Aggressive |
| UNIT-02 | Squad casualties: member count follows HP; healing capped; replenishment charges USD once |
| UNIT-03 | Indirect scatter reproducible with a fixed seed across two packaged runs; never computed on clients |
| UNIT-04 | Veterancy: XP credit by damage share; no XP from friendly, civilian or rampage victims; promotion preserves the HP ratio |
| UNIT-05 | Resolve tiers: Pinned rejects forward moves with typed feedback; Breaking auto-withdraws; Broken alone never converts or eliminates |
| UNIT-06 | Faction Discipline: Concord no isolation penalty; Directorate and Ascendancy mass bonus only from actual coherent units, never from control groups |
| UNIT-07 | Disguise: opponents' replicated archetype ID equals the disguise; detection within range reveals the true ID exactly once |
| UNIT-08 | Transport manifest never replicated to opponents |
| UNIT-09 | Limited ammunition and resupply: empty weapon unusable; resupply debits USD once |
| UNIT-10 | Unique caps: second commander or prototype rejected without side effects |
| UNIT-11 | Scout invariants (E) and the Directorate infantry price rule validated from content |
| UNIT-12 | Unaligned units cannot be selected, commanded or purchased by any participant |

## 19. Reference checkpoint (E: AGENTS.md and Engine Outline §7.1 workflow)

| POLITIKA question | Evidence | Class / confidence | Disposition | Not transferred |
| --- | --- | --- | --- | --- |
| How many veterancy ranks, and how is veterancy represented? | Tempest Rising `Config/Tags/DefinitionTags.ini` lines 616–618: `Definition.Status.Veterancy_Elite`, `_Hardened`, `_Veteran` (client pak `pakchunk0-Windows`, SHA-256 `4e9506ed…77b81b5`). RA2 `rulesmd.ini` `VeteranCap=2`; Generals `GameData.ini` health bonus Veteran/Elite/Heroic 120/130/150 % | Tempest: data-inspected (tag declaration only; semantics unknown). RA2 and Generals: data-inspected | adopt-concept: three ranks, implemented as a status | Tempest values, names-as-identifiers, any Blueprint logic |
| Should armor class be a typed classification rather than a free-form multiplier? | Tempest `DefinitionTags.ini` lines 490–535: passive abilities `Infantry`, `HeavyInfantry`, `LightVehicle`, `HeavyVehicle`, `Vehicle`; `PawnTags.ini` lines 21–27: `PawnType.Unit.Infantry(.Heavy)`, `.Support`, `.Vehicle(.Air/.Heavy)` (SHA-256 `b64f11cc…3c038`) | data-inspected (tags only) | adopt-concept: typed armor and category IDs validated by content rules | Tempest categories as-is; POLITIKA keeps 12 armor classes |
| Is suppression a status or a second meter? | Tempest `StatusTags.ini` line 65 `Status.Mutable.Suppressed`; `CommandTags.ini` `Command.Cooldown.Suppression` (SHA-256 `c287437f…eaa`) | data-inspected (tags only) | adopt-concept: suppression as a timed status driven by the single `UnitResolve` (E) | Any Tempest suppression rules |
| Detection, stealth and marking as passive capabilities? | Tempest `DefinitionTags.ini` lines 479, 499, 523: `PassiveAbility.Detection`, `.Marking`, `.Stealth` | data-inspected (tags only) | redefine: concealment and detection flags (§9). No invisibility (E) | Stealth semantics |
| Repair and resupply as auras? | Tempest `DefinitionTags.ini` lines 468, 508: `PassiveAbility.AutoRestock`, `.RepairAura` | data-inspected (tags only) | adopt-concept: resupply and repair auras (§10) | Values |

Classic C&C data (TD warhead table, RA `[SA]/[HE]/[AP]/[Fire]/[HollowPoint]` `Verses`, TS and RA2 veterancy, RA2 `Verses` acquisition semantics, Generals armor templates and experience) is analysed in [C&C study §5](CNC_GAMEPLAY_COMPARATIVE_STUDY.md#5-classic-unit-system-reference-for-the-politika-unit-specifications). Extracted data stays outside the repository (E).

## 20. Unresolved decisions register

| ID | Decision | Recommendation | Owner |
| --- | --- | --- | --- |
| U-01 | Squad representation | One entity with member count (§5.4) | Gameplay / Engine Outline |
| U-02 | Armor and damage taxonomy acceptance | Adopt §6 for the Core slice; tune percentages through playtests | Gameplay |
| U-03 | Number of nav-agent profiles and map terrain classes | FOOT, WHEEL, TRACKED, TRACKED_HEAVY (ground) plus water layers; terrain-area authoring in map schema v2 | Navigation owner |
| U-04 | Air sortie model (fixed-wing rearm at airfield) | Adopt for jets and bombers; loiter for scouts and AEW with a fuel timer | Gameplay |
| U-05 | Food shortage penalty value | −20 % cooldown speed + no Discipline recovery | Economy owner |
| U-06 | Fuel starvation rule | §4.5 | Economy owner |
| U-07 | Crush and overrun, death blasts | Defer crush; adopt death blasts for tankers and ammunition carriers | Gameplay / Navigation |
| U-08 | Garrison and map cover | Adopt garrison for Core; defer map cover until the map schema supports it | Gameplay / Map |
| U-09 | Resupply pricing | Quarter of per-shot share | Economy |
| U-10 | Production cancellation and refunds | Full refund of unspent reserve | Production owner |
| U-11 | Heavy-armor battlefield recovery | Adopt for Concord and Ascendancy recovery vehicles only | Gameplay |
| U-12 | Technical per-participant entity cap | Measured 29 Oct 2026: with current replication a converged fight stays under 80 % of the per-client send rate at about 260 units in total ([unit-scale record](RTS_MULTIPLAYER_MP2_UNIT_SCALE_ACCEPTANCE.md)). Proposed provisional cap **50 per participant** until F44 is fixed | Performance / MP |
| U-13 | RD-AIR-09 role (conventional theater missile versus ICBM delivery) | Conventional; ICBM stays the RD-DEF-14 silo | Nuclear owner |
| U-14 | IA-INF-13 and MP-INF-13 commander eligibility | Approve both (each faction needs one appointment) | Design owner |
| U-15 | Duplicate contracted strike entries (MP-AIR-06 vs MP-MKT-07) | Differentiate: AIR-06 recurring contract, MKT-07 single market sortie | Design owner |
| U-16 | Visibility of enemy veterancy and resolve tier | Rank visible; resolve tier visible only through intelligence effects | Visibility owner |
