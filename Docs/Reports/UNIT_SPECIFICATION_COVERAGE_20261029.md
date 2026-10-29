# POLITIKA — Unit Specification Coverage and Consistency Report (29 October 2026)

**Status:** verification report for the PROPOSED unit gameplay specifications. It checks documentation coverage and consistency only. No unit was implemented, built or playtested for this report.
**Scope:** every unit row (categories INF, ARM, SUP, AIR, SEA, MKT, CIV) in the [Five-Faction Rosters](../POLITIKA_FIVE_FACTION_ROSTERS.md) at repository commit `4371124` (`origin/main`; the roster is unchanged in the local `239b8db`), against the five per-faction specification files, the [Unit Specification Framework](../UNIT_SPECIFICATION_FRAMEWORK.md), the [Unit Balance and Ability Matrices](../UNIT_BALANCE_AND_ABILITY_MATRICES.md) and §5 of the [C&C Gameplay Comparative Study](../CNC_GAMEPLAY_COMPARATIVE_STUDY.md). Buildings and defenses (BLD, DEF, SEC: 148 rows) are not unit entries; they appear in the specifications only as producers, prerequisites and targets.

## 1. Method

1. **Roster extraction.** Unit IDs and exact English names were read from the roster tables (`| \`ID\` | **Name** |`).
2. **Entry check.** For each faction file, every `### ID — Name` heading was compared with the roster in order. Every entry was checked for the field table (Identity row) and the **Weapons**, **Abilities**, **Research and upgrades**, **Counters** and **Status** sections.
3. **Reference check.** Every relative Markdown link was resolved against the repository. Every cross-faction unit or building ID cited anywhere was checked against the roster.
4. **Claim check.** Status lines were searched for implementation claims. The only Implemented (I) facts allowed are the generic behaviors in framework §2: orders, the infantry magazine/reload/grenade/sprint profile, sight-radius fog and server authority.
5. **Economy check.** The Directorate infantry price validator (framework §4.4) was re-run across all four playable files ([matrices §4](../UNIT_BALANCE_AND_ABILITY_MATRICES.md)).

The checks are scripted (heading, section, link and ID checks per file, plus the per-unit table below), so they can be re-run after any edit.

## 2. Faction coverage summary

Columns: roster unit rows; specified entries; IDs in roster order; exact names; entries with every required section; roster-tier split; armed entries; unit-specific abilities (shared abilities excluded); entries linking existing concept art; file size in KB.

| Faction | File | Roster | Specified | Order | Names | Complete | Tiers | Armed | Abilities | Art | KB |
| --- | --- | ---: | ---: | --- | ---: | ---: | --- | ---: | ---: | ---: | ---: |
| Atlantic Concord | [ATLANTIC_CONCORD_UNITS.md](../Units/ATLANTIC_CONCORD_UNITS.md) | 54 | 54 | yes | 54 | 54 | Core 17 · Extended 26 · Conditional 10 · Scenario 1 | 37 | 51 | 14 | 200 |
| Red Directorate | [RED_DIRECTORATE_UNITS.md](../Units/RED_DIRECTORATE_UNITS.md) | 51 | 51 | yes | 51 | 51 | Core 16 · Extended 25 · Conditional 10 · Scenario 0 | 38 | 45 | 13 | 167 |
| Iron Ascendancy | [IRON_ASCENDANCY_UNITS.md](../Units/IRON_ASCENDANCY_UNITS.md) | 49 | 49 | yes | 49 | 49 | Core 16 · Extended 19 · Conditional 14 · Scenario 0 | 37 | 48 | 13 | 195 |
| Meridian Pact | [MERIDIAN_PACT_UNITS.md](../Units/MERIDIAN_PACT_UNITS.md) | 57 | 57 | yes | 57 | 57 | Core 16 · Extended 20 · Conditional 21 · Scenario 0 | 35 | 67 | 11 | 195 |
| The Unaligned | [UNALIGNED_UNITS.md](../Units/UNALIGNED_UNITS.md) | 57 | 57 | yes | 57 | 57 | Core 17 · Extended 23 · Conditional 12 · Scenario 5 | 25 | 49 | 0 | 166 |
| **Total** | | **268** | **268** | | **268** | **268** | | **172** | **260** | **51** | |

**Result:** all 268 roster units are preserved and specified, in roster order, with exact roster names. No unit was added, removed, merged or renumbered. The Unaligned has no unit concept art in the repository (`Factions/TheUnaligned/Units/` contains only `.gitkeep`), so its entries state asset requirements instead.

## 3. Quality-assurance checklist

| # | Check | Result | Evidence |
| --- | --- | --- | --- |
| 1 | Every previously defined unit is preserved | **Pass** | §2 and §6: 268/268, order and names identical to the roster |
| 2 | Every unit has a complete identity and battlefield classification | **Pass** | Identity row (unit ID, faction, category/subclass, template, roster tier, playable/NPC/scenario) and Role row in every entry |
| 3 | Every combat unit has a combat profile | **Pass** | 172 armed entries with a weapon table: type, range, damage per cycle, model and AoE, suppression, layers and per-armor effectiveness from framework §6.3. 96 entries are explicitly unarmed |
| 4 | Every specialized ability has mechanics and limits | **Pass** | Every ability row fills kind, effect, activation/targets, cost, duration, cooldown, range, counters, feedback and AI rule. Hidden-state abilities carry an MP note |
| 5 | Economics conform to the existing economy | **Pass, with one correction** | USD prices for playable units; food staff and fuel class from framework §4.5; Meridian contracts with wages; conventional-only Black Market; Unaligned mobilization weights. The Directorate validator failed for RD-INF-03 at $330 and passes at $260 (matrices §4) |
| 6 | Technological dependencies are consistent | **Pass** | 85 unit technologies, each with a roster-building prerequisite and no ages or eras ([matrices §6](../UNIT_BALANCE_AND_ABILITY_MATRICES.md)). Late spy-only sabotage maps to the implemented `late_espionage_unlocked` flag |
| 7 | Units integrate with formations, grouping and commands | **Pass** | Commands row (orders, default stance, formation role) per entry; framework §13 |
| 8 | AI behavior addressed | **Pass** | AI row names an `ai.unit.*` profile (framework §14) plus unit deltas; Unaligned units add response tiers R0–R4 |
| 9 | Multiplayer authority accounted for | **Pass** | Multiplayer row names MPA classes (framework §15) and states hidden-state rules where relevant |
| 10 | Faction identity and asymmetry coherent | **Pass** | Faction rules tables (§1 of each file) cite the established canon; [matrices §1, §3, §5](../UNIT_BALANCE_AND_ABILITY_MATRICES.md) show role gaps and exclusive mechanics |
| 11 | Commanders and ordinary progression stay separate | **Pass** | Talents only on AC-INF-14, RD-INF-13, IA-INF-13 and MP-INF-13 (the latter two pending U-14); officer auras (RD-INF-10, UN-ARM-15) have no talents; the Unaligned has no talent tree (E) |
| 12 | Counters intelligible | **Pass** | Counters line with hard, soft and situational counters per entry, citing roster IDs; counter web in matrices §2 |
| 13 | Documentation links valid | **Pass** | All relative links in the new and edited files resolve; no unknown unit or building IDs |
| 14 | No proposal presented as implemented | **Pass** | No Status line claims a roster unit is implemented; I labels appear only for the generic behaviors of framework §2 |

## 4. Corrections made during verification

| Item | Before | After | Reason |
| --- | --- | --- | --- |
| RD-INF-09 Recon squad price | higher band value | $420 | Directorate validator against AC-INF-08 Marksman Pair; IA-INF-08 and MP-INF-07 priced above $630 |
| RD-INF-03 HMG team price | $330 | $260 | Directorate validator against AC-INF-04 ($400) and IA-INF-03 ($420) |
| Matrices: officer row | IA-INF-12 listed as officer support | IA-INF-12 removed (security enforcer without an aura) | Consistency with the IA-INF-12 entry |
| U-15 duplicate contracted strike | MP-AIR-06 and MP-MKT-07 overlapped | AIR-06 recurring contract; MKT-07 single market sortie | Roster near-duplicate |

## 5. Remaining design gaps, by priority

Priority reflects what blocks implementation or the first playtest of the Core roster. Shared items are numbered as in the framework's unresolved register; faction items are numbered per file.

**P1: blocks the Core vertical slice**

| ID | Decision needed | Proposed default | Owner |
| --- | --- | --- | --- |
| U-01 | Squad representation | One authoritative entity with a member count | Gameplay / Engine Outline |
| U-02 | Armor and damage taxonomy acceptance | Adopt framework §6 for Core; tune by playtest | Gameplay |
| U-03 | Nav-agent profiles and terrain classes | FOOT, WHEEL, TRACKED, TRACKED_HEAVY plus water layers; terrain areas in map schema v2 | Navigation |
| U-05 | Food shortage penalty | −20 % cooldown speed and no Discipline recovery | Economy |
| U-12 | Per-participant entity cap | Measured: about 260 converged units per match before 80 % of the send rate; provisional 50 per participant until F44 ([record](../RTS_MULTIPLAYER_MP2_UNIT_SCALE_ACCEPTANCE.md)) | Performance / MP |
| U-14 | Commander eligibility of IA-INF-13 and MP-INF-13 | Approve both; fallbacks U-IA-05 and U-MP-08 | Design |

**P2: needed before the Extended roster and multiplayer balance**

| ID | Decision needed | Proposed default |
| --- | --- | --- |
| U-04 | Air sortie model | Rearm at airfields for jets; loiter with a timer for scouts and AEW |
| U-06 | Fuel starvation | Framework §4.5 |
| U-07 | Crush, overrun and death blasts | Defer crush; adopt death blasts for tankers and ammunition carriers |
| U-08 | Garrison and map cover | Garrison for Core; map cover later |
| U-09, U-10 | Resupply pricing; cancellation refunds | Quarter of per-shot share; full refund of unspent reserve |
| U-11 | Heavy-armor battlefield recovery | Concord and Ascendancy recovery vehicles only |
| U-16 | Visibility of enemy rank and resolve | Rank visible; resolve only through intelligence |
| U-AC-02, U-IA-02, U-IA-03, U-RD-02, U-MP-02, U-AC-05 | Covert-ability numbers: Psy-Ops threshold, Intimidation rate, contamination, diversion, charge prices | Values in each file; owner decisions under Espionage and Sabotage |
| U-UN-01, U-UN-02, U-UN-04 | Civilian targeting, civilian replication budget, mobilization budget curve | Values in the Unaligned file |

**P3: content and scenario detail**

| ID | Decision needed |
| --- | --- |
| U-13 | RD-AIR-09 as a conventional theater missile (ICBM stays on the RD-DEF-14 silo) |
| U-15 | MP-AIR-06 / MP-MKT-07 split (proposed resolution applied) |
| U-AC-01, U-AC-03, U-AC-04 | Concord raid scope, mobile works deploy target, nuclear custody on bomber loss |
| U-RD-01, U-RD-03, U-RD-04 | Price-rule scope for agents and the commander, forward-base detection, fuel surcharge |
| U-IA-01, U-IA-04, U-IA-06 to U-IA-08 | Ascendancy deploy target, wire-guided ATGM jamming immunity, radar SAM option, anchorage, heavy-hull surcharges |
| U-MP-01, U-MP-03 to U-MP-07 | Capture eligibility, condition grades, weather, market stock, falsified manifests on Unaligned roads, anchorage |
| U-UN-03, U-UN-05 | Unaligned airfield (none in the roster); detention and survivors |

## 6. Faction completeness, coverage and balance risks

| Faction | Completeness | Tactical coverage | Main balance risks |
| --- | --- | --- | --- |
| Atlantic Concord | 54/54; 17 Core; 14 entries with concept art | Full combined arms; strongest air, reconnaissance, precision fire and EW; no flame, mortar carrier or AA gun by design | Price premium versus solo resilience; Psy-Ops rampage threshold (U-AC-02); counter-battery and precision artillery dominance |
| Red Directorate | 51/51; 16 Core | Mass infantry and artillery, integrated air defense; no medic, tank destroyer or concealment by design | Cheap-infantry spam under the mass Discipline bonus; rail and depot dependence; validator must be re-run on any price change |
| Iron Ascendancy | 49/49; 16 Core; commander candidate pending U-14 | Elite mechanized assault, flame, siege armor; no mobile SAM by design (U-IA-06) | +8 % damage stacking with self-endangering abilities; Intimidation snowball (U-IA-02); Unaligned hostility +1 |
| Meridian Pact | 57/57; 16 Core; commander candidate pending U-14 | Fastest scouting and raids, early sabotage, contracts and market; no native heavy armor, bombers, propaganda or nuclear path (E) | Early sabotage tempo; contract wages over long matches; market stock (U-MP-05) |
| The Unaligned | 57/57; 17 Core (NPC first slice); no concept art | Graduated R0–R4 response, strongest counterintelligence, civil world; off-map air (U-UN-03) | Kingmaking through forged attribution; federal snowball against one participant; civilian simulation cost |

## 7. Per-unit coverage matrix

Columns: roster ID and exact roster name; specification file; name match; roster tier; armed or unarmed; count of unit-specific abilities; concept art linked; evidence labels used in the Status line (E-R roster, E established, I implemented generic behavior, P proposed, U unresolved).

| ID | Roster name | File | Name | Tier | Weapons | Abilities | Art | Status labels |
| --- | --- | --- | --- | --- | --- | ---: | --- | --- |
| AC-INF-01 | Professional Rifle Section | [Atlantic Concord](../Units/ATLANTIC_CONCORD_UNITS.md) | ✓ | Core | armed | 1 | yes | E-R / E / I / P |
| AC-INF-02 | Urban Security Team | [Atlantic Concord](../Units/ATLANTIC_CONCORD_UNITS.md) | ✓ | Extended | armed | 2 | yes | E-R / I / P |
| AC-INF-03 | Shoulder Missile Team | [Atlantic Concord](../Units/ATLANTIC_CONCORD_UNITS.md) | ✓ | Core | armed | 1 | yes | E-R / I / P |
| AC-INF-04 | Grenadier Section | [Atlantic Concord](../Units/ATLANTIC_CONCORD_UNITS.md) | ✓ | Core | armed | 1 | yes | E-R / I / P |
| AC-INF-05 | Field Medical Corps | [Atlantic Concord](../Units/ATLANTIC_CONCORD_UNITS.md) | ✓ | Core | unarmed | 3 | yes | E-R / P |
| AC-INF-06 | Combat Engineering Team | [Atlantic Concord](../Units/ATLANTIC_CONCORD_UNITS.md) | ✓ | Core | armed | 2 | yes | E-R / E / P |
| AC-INF-07 | Airborne Assault Section | [Atlantic Concord](../Units/ATLANTIC_CONCORD_UNITS.md) | ✓ | Extended | armed | 2 | yes | E-R / I / P |
| AC-INF-08 | Long-Range Marksman Pair | [Atlantic Concord](../Units/ATLANTIC_CONCORD_UNITS.md) | ✓ | Core | armed | 1 | yes | E-R / E / P |
| AC-INF-09 | Coastal Reconnaissance Detachment | [Atlantic Concord](../Units/ATLANTIC_CONCORD_UNITS.md) | ✓ | Extended | armed | 2 | yes | E-R / E / P / U |
| AC-INF-10 | Counterintelligence Officer | [Atlantic Concord](../Units/ATLANTIC_CONCORD_UNITS.md) | ✓ | Extended | armed | 2 | yes | E-R / E / P |
| AC-INF-11 | Concord Field Agent | [Atlantic Concord](../Units/ATLANTIC_CONCORD_UNITS.md) | ✓ | Conditional | unarmed | 1 | yes | E-R / E / I / P |
| AC-INF-12 | Psychological Operations Officer | [Atlantic Concord](../Units/ATLANTIC_CONCORD_UNITS.md) | ✓ | Conditional | unarmed | 3 | yes | E / P / U |
| AC-INF-13 | Signals Intercept Team | [Atlantic Concord](../Units/ATLANTIC_CONCORD_UNITS.md) | ✓ | Extended | armed | 2 | yes | E-R / E / P |
| AC-INF-14 | Special Operations Commander | [Atlantic Concord](../Units/ATLANTIC_CONCORD_UNITS.md) | ✓ | Extended | armed | 0 | none | E-R / E / P |
| AC-ARM-01 | Escort Jeep | [Atlantic Concord](../Units/ATLANTIC_CONCORD_UNITS.md) | ✓ | Core | armed | 1 | none | E-R / P |
| AC-ARM-02 | Rapid Armored Carrier | [Atlantic Concord](../Units/ATLANTIC_CONCORD_UNITS.md) | ✓ | Core | armed | 1 | yes | E-R / P |
| AC-ARM-03 | Modular Infantry Support Vehicle | [Atlantic Concord](../Units/ATLANTIC_CONCORD_UNITS.md) | ✓ | Extended | armed | 1 | none | E-R / P |
| AC-ARM-04 | Concord Battle Tank | [Atlantic Concord](../Units/ATLANTIC_CONCORD_UNITS.md) | ✓ | Core | armed | 0 | none | E-R / E / P |
| AC-ARM-05 | Heavy Breakthrough Tank | [Atlantic Concord](../Units/ATLANTIC_CONCORD_UNITS.md) | ✓ | Extended | armed | 1 | none | E-R / P |
| AC-ARM-06 | Anti-Tank Missile Carrier | [Atlantic Concord](../Units/ATLANTIC_CONCORD_UNITS.md) | ✓ | Core | armed | 0 | none | E-R / P |
| AC-ARM-07 | Concord Self-Propelled Gun | [Atlantic Concord](../Units/ATLANTIC_CONCORD_UNITS.md) | ✓ | Core | armed | 1 | none | E-R / E / P |
| AC-ARM-08 | Precision Missile Battery | [Atlantic Concord](../Units/ATLANTIC_CONCORD_UNITS.md) | ✓ | Extended | armed | 1 | none | E-R / P |
| AC-ARM-09 | Skyguard Missile Carrier | [Atlantic Concord](../Units/ATLANTIC_CONCORD_UNITS.md) | ✓ | Core | armed | 1 | none | E-R / P |
| AC-ARM-10 | Electronic Warfare Van | [Atlantic Concord](../Units/ATLANTIC_CONCORD_UNITS.md) | ✓ | Extended | unarmed | 0 | none | E-R / P |
| AC-ARM-11 | Mobile Concealment Truck | [Atlantic Concord](../Units/ATLANTIC_CONCORD_UNITS.md) | ✓ | Extended | unarmed | 1 | none | E-R / P |
| AC-ARM-12 | Counter-Battery Radar Carrier | [Atlantic Concord](../Units/ATLANTIC_CONCORD_UNITS.md) | ✓ | Extended | unarmed | 1 | none | E-R / P |
| AC-ARM-13 | Repair and Recovery Vehicle | [Atlantic Concord](../Units/ATLANTIC_CONCORD_UNITS.md) | ✓ | Core | armed | 0 | none | E-R / P / U |
| AC-ARM-14 | Assault Engineer Breacher | [Atlantic Concord](../Units/ATLANTIC_CONCORD_UNITS.md) | ✓ | Extended | armed | 2 | none | E-R / E / P |
| AC-ARM-15 | Prototype Siege Tank | [Atlantic Concord](../Units/ATLANTIC_CONCORD_UNITS.md) | ✓ | Conditional | armed | 1 | none | E-R / P |
| AC-SUP-01 | Swiftwing Recon Plane | [Atlantic Concord](../Units/ATLANTIC_CONCORD_UNITS.md) | ✓ | Core | unarmed | 1 | none | E / P |
| AC-SUP-02 | Mobile Works Unit | [Atlantic Concord](../Units/ATLANTIC_CONCORD_UNITS.md) | ✓ | Core | unarmed | 1 | none | E-R / E / P / U |
| AC-SUP-03 | Contract Construction Crew | [Atlantic Concord](../Units/ATLANTIC_CONCORD_UNITS.md) | ✓ | Extended | unarmed | 0 | none | E-R / E / P |
| AC-SUP-04 | Petroleum Tanker | [Atlantic Concord](../Units/ATLANTIC_CONCORD_UNITS.md) | ✓ | Extended | unarmed | 0 | none | E-R / E / P |
| AC-SUP-05 | Airport Fuel Bowser | [Atlantic Concord](../Units/ATLANTIC_CONCORD_UNITS.md) | ✓ | Extended | unarmed | 1 | none | E-R / E / P |
| AC-SUP-06 | Forward Logistics Convoy | [Atlantic Concord](../Units/ATLANTIC_CONCORD_UNITS.md) | ✓ | Extended | unarmed | 0 | none | E-R / P |
| AC-SUP-07 | Portable Listening Post | [Atlantic Concord](../Units/ATLANTIC_CONCORD_UNITS.md) | ✓ | Extended | unarmed | 1 | none | E-R / P |
| AC-AIR-01 | Interceptor Fighter | [Atlantic Concord](../Units/ATLANTIC_CONCORD_UNITS.md) | ✓ | Core | armed | 0 | none | E-R / E / P |
| AC-AIR-02 | Ground-Attack Jet | [Atlantic Concord](../Units/ATLANTIC_CONCORD_UNITS.md) | ✓ | Extended | armed | 0 | none | E-R / P |
| AC-AIR-03 | Precision Strike Fighter | [Atlantic Concord](../Units/ATLANTIC_CONCORD_UNITS.md) | ✓ | Extended | armed | 1 | none | E-R / E / P |
| AC-AIR-04 | Attack Helicopter | [Atlantic Concord](../Units/ATLANTIC_CONCORD_UNITS.md) | ✓ | Core | armed | 1 | none | E-R / P |
| AC-AIR-05 | Utility Transport Helicopter | [Atlantic Concord](../Units/ATLANTIC_CONCORD_UNITS.md) | ✓ | Extended | armed | 1 | none | E-R / P |
| AC-AIR-06 | Heavy Transport Aircraft | [Atlantic Concord](../Units/ATLANTIC_CONCORD_UNITS.md) | ✓ | Extended | unarmed | 2 | none | E-R / P |
| AC-AIR-07 | Electronic Surveillance Aircraft | [Atlantic Concord](../Units/ATLANTIC_CONCORD_UNITS.md) | ✓ | Extended | unarmed | 1 | none | E-R / P |
| AC-AIR-08 | Tactical Bomber | [Atlantic Concord](../Units/ATLANTIC_CONCORD_UNITS.md) | ✓ | Extended | armed | 1 | none | E-R / P |
| AC-AIR-09 | Armed Observation Helicopter | [Atlantic Concord](../Units/ATLANTIC_CONCORD_UNITS.md) | ✓ | Extended | armed | 1 | none | E-R / P |
| AC-AIR-10 | Airborne Support Gunship | [Atlantic Concord](../Units/ATLANTIC_CONCORD_UNITS.md) | ✓ | Extended | armed | 1 | none | E-R / P |
| AC-AIR-11 | Nuclear Delivery Bomber | [Atlantic Concord](../Units/ATLANTIC_CONCORD_UNITS.md) | ✓ | Conditional | unarmed | 1 | none | E / P / U |
| AC-SEA-01 | Port Security Launch | [Atlantic Concord](../Units/ATLANTIC_CONCORD_UNITS.md) | ✓ | Conditional | armed | 0 | none | E-R / P |
| AC-SEA-02 | Amphibious Landing Carrier | [Atlantic Concord](../Units/ATLANTIC_CONCORD_UNITS.md) | ✓ | Conditional | unarmed | 0 | none | E-R / P |
| AC-SEA-03 | Coastal Missile Boat | [Atlantic Concord](../Units/ATLANTIC_CONCORD_UNITS.md) | ✓ | Conditional | armed | 0 | none | E-R / P |
| AC-SEA-04 | Guided-Missile Frigate | [Atlantic Concord](../Units/ATLANTIC_CONCORD_UNITS.md) | ✓ | Conditional | armed | 0 | none | E-R / P |
| AC-SEA-05 | Adriatic Attack Submarine | [Atlantic Concord](../Units/ATLANTIC_CONCORD_UNITS.md) | ✓ | Conditional | armed | 0 | none | E-R / P |
| AC-SEA-06 | Offshore Fire-Support Ship | [Atlantic Concord](../Units/ATLANTIC_CONCORD_UNITS.md) | ✓ | Conditional | armed | 0 | none | E-R / P |
| AC-SEA-07 | Fleet Air-Control Vessel | [Atlantic Concord](../Units/ATLANTIC_CONCORD_UNITS.md) | ✓ | Scenario | armed | 2 | none | E-R / P |
| RD-INF-01 | Directorate Rifle Platoon | [Red Directorate](../Units/RED_DIRECTORATE_UNITS.md) | ✓ | Core | armed | 1 | yes | E-R / E / I / P |
| RD-INF-02 | Motor-Rifle Section | [Red Directorate](../Units/RED_DIRECTORATE_UNITS.md) | ✓ | Core | armed | 1 | yes | E-R / E / P |
| RD-INF-03 | Heavy Machine-Gun Team | [Red Directorate](../Units/RED_DIRECTORATE_UNITS.md) | ✓ | Core | armed | 1 | yes | E-R / E / P |
| RD-INF-04 | Portable Rocket Section | [Red Directorate](../Units/RED_DIRECTORATE_UNITS.md) | ✓ | Core | armed | 0 | yes | E-R / E / P |
| RD-INF-05 | Mortar Infantry Team | [Red Directorate](../Units/RED_DIRECTORATE_UNITS.md) | ✓ | Extended | armed | 1 | yes | E-R / E / P |
| RD-INF-06 | Combat Pioneer | [Red Directorate](../Units/RED_DIRECTORATE_UNITS.md) | ✓ | Core | armed | 2 | yes | E-R / E / P |
| RD-INF-07 | Rail Security Troop | [Red Directorate](../Units/RED_DIRECTORATE_UNITS.md) | ✓ | Extended | armed | 1 | yes | E-R / E / P |
| RD-INF-08 | Chemical Defense Detachment | [Red Directorate](../Units/RED_DIRECTORATE_UNITS.md) | ✓ | Extended | armed | 2 | yes | E-R / E / P |
| RD-INF-09 | Special Recon Riflemen | [Red Directorate](../Units/RED_DIRECTORATE_UNITS.md) | ✓ | Core | armed | 2 | yes | E-R / E / P |
| RD-INF-10 | Political Liaison Officer | [Red Directorate](../Units/RED_DIRECTORATE_UNITS.md) | ✓ | Core | armed | 2 | yes | E-R / E / P |
| RD-INF-11 | Directorate Intelligence Courier | [Red Directorate](../Units/RED_DIRECTORATE_UNITS.md) | ✓ | Conditional | unarmed | 1 | yes | E-R / E / I / P |
| RD-INF-12 | Resource Diversion Agent | [Red Directorate](../Units/RED_DIRECTORATE_UNITS.md) | ✓ | Conditional | unarmed | 1 | yes | E-R / E / P / U |
| RD-INF-13 | Special Forces Colonel | [Red Directorate](../Units/RED_DIRECTORATE_UNITS.md) | ✓ | Extended | armed | 0 | yes | E-R / E / P / U |
| RD-ARM-01 | Patrol Motorcycle | [Red Directorate](../Units/RED_DIRECTORATE_UNITS.md) | ✓ | Core | armed | 0 | none | E-R / E / P |
| RD-ARM-02 | Motorized Infantry Truck | [Red Directorate](../Units/RED_DIRECTORATE_UNITS.md) | ✓ | Core | armed | 1 | none | E-R / P |
| RD-ARM-03 | Tracked Personnel Carrier | [Red Directorate](../Units/RED_DIRECTORATE_UNITS.md) | ✓ | Core | armed | 0 | none | E-R / P |
| RD-ARM-04 | People's Battle Tank | [Red Directorate](../Units/RED_DIRECTORATE_UNITS.md) | ✓ | Core | armed | 0 | none | E-R / P |
| RD-ARM-05 | Improved Heavy Battle Tank | [Red Directorate](../Units/RED_DIRECTORATE_UNITS.md) | ✓ | Extended | armed | 2 | none | E-R / P |
| RD-ARM-06 | Mobile Anti-Air Gun | [Red Directorate](../Units/RED_DIRECTORATE_UNITS.md) | ✓ | Core | armed | 0 | none | E-R / P |
| RD-ARM-07 | Rocket Artillery Regiment | [Red Directorate](../Units/RED_DIRECTORATE_UNITS.md) | ✓ | Core | armed | 1 | none | E-R / P |
| RD-ARM-08 | Heavy Self-Propelled Gun | [Red Directorate](../Units/RED_DIRECTORATE_UNITS.md) | ✓ | Extended | armed | 1 | none | E-R / P |
| RD-ARM-09 | Mortar Carrier | [Red Directorate](../Units/RED_DIRECTORATE_UNITS.md) | ✓ | Extended | armed | 1 | none | E-R / P |
| RD-ARM-10 | Flame Assault Vehicle | [Red Directorate](../Units/RED_DIRECTORATE_UNITS.md) | ✓ | Extended | armed | 0 | none | E-R / P |
| RD-ARM-11 | Mine-Clearing Tank | [Red Directorate](../Units/RED_DIRECTORATE_UNITS.md) | ✓ | Extended | unarmed | 2 | none | E-R / P |
| RD-ARM-12 | Defensive Entrenchment Tank | [Red Directorate](../Units/RED_DIRECTORATE_UNITS.md) | ✓ | Extended | armed | 1 | none | E-R / P |
| RD-ARM-13 | Field Anti-Air Missile Carrier | [Red Directorate](../Units/RED_DIRECTORATE_UNITS.md) | ✓ | Extended | armed | 1 | none | E-R / P |
| RD-ARM-14 | Armored Siege Command Tank | [Red Directorate](../Units/RED_DIRECTORATE_UNITS.md) | ✓ | Conditional | armed | 2 | none | E-R / P |
| RD-ARM-15 | Veteran Reserve Tank | [Red Directorate](../Units/RED_DIRECTORATE_UNITS.md) | ✓ | Extended | armed | 0 | none | E-R / P |
| RD-SUP-01 | Industrial Constructor | [Red Directorate](../Units/RED_DIRECTORATE_UNITS.md) | ✓ | Core | unarmed | 1 | none | E-R / E / P |
| RD-SUP-02 | Rail Supply Tractor | [Red Directorate](../Units/RED_DIRECTORATE_UNITS.md) | ✓ | Extended | unarmed | 1 | none | E-R / E / P |
| RD-SUP-03 | Bulk Fuel Transport | [Red Directorate](../Units/RED_DIRECTORATE_UNITS.md) | ✓ | Extended | unarmed | 1 | none | E-R / E / P |
| RD-SUP-04 | Mobile Repair Crew | [Red Directorate](../Units/RED_DIRECTORATE_UNITS.md) | ✓ | Core | unarmed | 0 | none | E-R / P / U |
| RD-SUP-05 | Engineer Bridging Vehicle | [Red Directorate](../Units/RED_DIRECTORATE_UNITS.md) | ✓ | Extended | unarmed | 2 | none | E-R / E / P |
| RD-SUP-06 | Electronic Countermeasure Vehicle | [Red Directorate](../Units/RED_DIRECTORATE_UNITS.md) | ✓ | Extended | unarmed | 2 | none | E-R / P |
| RD-SUP-07 | Reserve Reinforcement Carrier | [Red Directorate](../Units/RED_DIRECTORATE_UNITS.md) | ✓ | Extended | unarmed | 2 | none | E-R / E / P |
| RD-AIR-01 | Frontline Interceptor | [Red Directorate](../Units/RED_DIRECTORATE_UNITS.md) | ✓ | Extended | armed | 0 | none | E-R / P |
| RD-AIR-02 | Tactical Attack Aircraft | [Red Directorate](../Units/RED_DIRECTORATE_UNITS.md) | ✓ | Extended | armed | 0 | none | E-R / P |
| RD-AIR-03 | Close-Support Gunship | [Red Directorate](../Units/RED_DIRECTORATE_UNITS.md) | ✓ | Core | armed | 1 | none | E-R / P |
| RD-AIR-04 | Assault Transport Helicopter | [Red Directorate](../Units/RED_DIRECTORATE_UNITS.md) | ✓ | Extended | armed | 1 | none | E-R / P |
| RD-AIR-05 | Heavy Transport Aircraft | [Red Directorate](../Units/RED_DIRECTORATE_UNITS.md) | ✓ | Extended | unarmed | 1 | none | E-R / P |
| RD-AIR-06 | Radar Reconnaissance Aircraft | [Red Directorate](../Units/RED_DIRECTORATE_UNITS.md) | ✓ | Extended | unarmed | 1 | none | E-R / P |
| RD-AIR-07 | Frontline Bomber | [Red Directorate](../Units/RED_DIRECTORATE_UNITS.md) | ✓ | Extended | armed | 0 | none | E-R / E / P |
| RD-AIR-08 | Artillery Targeting Helicopter | [Red Directorate](../Units/RED_DIRECTORATE_UNITS.md) | ✓ | Extended | unarmed | 1 | none | E-R / P |
| RD-AIR-09 | Strategic Missile Transporter | [Red Directorate](../Units/RED_DIRECTORATE_UNITS.md) | ✓ | Conditional | armed | 1 | none | E-R / P / U |
| RD-SEA-01 | River Patrol Boat | [Red Directorate](../Units/RED_DIRECTORATE_UNITS.md) | ✓ | Conditional | armed | 0 | none | E-R / P |
| RD-SEA-02 | Amphibious Motor-Rifle Carrier | [Red Directorate](../Units/RED_DIRECTORATE_UNITS.md) | ✓ | Extended | armed | 1 | none | E-R / P |
| RD-SEA-03 | Coastal Torpedo Boat | [Red Directorate](../Units/RED_DIRECTORATE_UNITS.md) | ✓ | Conditional | armed | 0 | none | E-R / P |
| RD-SEA-04 | Conventional Attack Submarine | [Red Directorate](../Units/RED_DIRECTORATE_UNITS.md) | ✓ | Conditional | armed | 0 | none | E-R / P |
| RD-SEA-05 | Anti-Air Escort Destroyer | [Red Directorate](../Units/RED_DIRECTORATE_UNITS.md) | ✓ | Conditional | armed | 0 | none | E-R / P |
| RD-SEA-06 | Shore Bombardment Cruiser | [Red Directorate](../Units/RED_DIRECTORATE_UNITS.md) | ✓ | Conditional | armed | 0 | none | E-R / P |
| RD-SEA-07 | Amphibious Headquarters Vessel | [Red Directorate](../Units/RED_DIRECTORATE_UNITS.md) | ✓ | Conditional | armed | 2 | none | E-R / P |
| IA-INF-01 | Garrison Riflemen | [Iron Ascendancy](../Units/IRON_ASCENDANCY_UNITS.md) | ✓ | Core | armed | 1 | yes | E-R / E / I / P |
| IA-INF-02 | Shock Assault Section | [Iron Ascendancy](../Units/IRON_ASCENDANCY_UNITS.md) | ✓ | Core | armed | 1 | yes | E-R / P |
| IA-INF-03 | Heavy Grenadier Squad | [Iron Ascendancy](../Units/IRON_ASCENDANCY_UNITS.md) | ✓ | Extended | armed | 2 | yes | E-R / P |
| IA-INF-04 | Portable Anti-Armor Cell | [Iron Ascendancy](../Units/IRON_ASCENDANCY_UNITS.md) | ✓ | Core | armed | 1 | yes | E-R / P |
| IA-INF-05 | Urban Flame Detachment | [Iron Ascendancy](../Units/IRON_ASCENDANCY_UNITS.md) | ✓ | Extended | armed | 2 | yes | E-R / P |
| IA-INF-06 | Frontier Combat Medic | [Iron Ascendancy](../Units/IRON_ASCENDANCY_UNITS.md) | ✓ | Core | armed | 2 | yes | E-R / P |
| IA-INF-07 | Technical Demolition Engineer | [Iron Ascendancy](../Units/IRON_ASCENDANCY_UNITS.md) | ✓ | Core | armed | 2 | yes | E-R / E / P |
| IA-INF-08 | Covert Reconnaissance Team | [Iron Ascendancy](../Units/IRON_ASCENDANCY_UNITS.md) | ✓ | Extended | armed | 2 | yes | E-R / E / P |
| IA-INF-09 | Ascendancy Intelligence Agent | [Iron Ascendancy](../Units/IRON_ASCENDANCY_UNITS.md) | ✓ | Conditional | unarmed | 2 | yes | E-R / E / I / P |
| IA-INF-10 | Facility Contamination Operative | [Iron Ascendancy](../Units/IRON_ASCENDANCY_UNITS.md) | ✓ | Conditional | unarmed | 1 | yes | E-R / E / P |
| IA-INF-11 | Intimidation Officer | [Iron Ascendancy](../Units/IRON_ASCENDANCY_UNITS.md) | ✓ | Core | armed | 2 | yes | E-R / E / P |
| IA-INF-12 | Political Security Enforcer | [Iron Ascendancy](../Units/IRON_ASCENDANCY_UNITS.md) | ✓ | Extended | armed | 2 | yes | E-R / P |
| IA-INF-13 | Special Operations Raider | [Iron Ascendancy](../Units/IRON_ASCENDANCY_UNITS.md) | ✓ | Extended | armed | 0 | yes | E-R / E / P / U |
| IA-INF-14 | Civilian Disguise Courier | [Iron Ascendancy](../Units/IRON_ASCENDANCY_UNITS.md) | ✓ | Conditional | unarmed | 1 | none | E-R / P |
| IA-ARM-01 | Light Border Patrol Car | [Iron Ascendancy](../Units/IRON_ASCENDANCY_UNITS.md) | ✓ | Extended | armed | 0 | none | E-R / P |
| IA-ARM-02 | Armored Personnel Half-Track | [Iron Ascendancy](../Units/IRON_ASCENDANCY_UNITS.md) | ✓ | Core | armed | 1 | none | E-R / P |
| IA-ARM-03 | Fast Assault Tank | [Iron Ascendancy](../Units/IRON_ASCENDANCY_UNITS.md) | ✓ | Core | armed | 1 | none | E-R / P |
| IA-ARM-04 | Frontier Battle Tank | [Iron Ascendancy](../Units/IRON_ASCENDANCY_UNITS.md) | ✓ | Core | armed | 1 | none | E-R / E / P |
| IA-ARM-05 | Heavy Siege Tank | [Iron Ascendancy](../Units/IRON_ASCENDANCY_UNITS.md) | ✓ | Extended | armed | 1 | none | E-R / P |
| IA-ARM-06 | Superheavy Prototype Tank | [Iron Ascendancy](../Units/IRON_ASCENDANCY_UNITS.md) | ✓ | Conditional | armed | 2 | none | E-R / P |
| IA-ARM-07 | Mobile Flame Assault Vehicle | [Iron Ascendancy](../Units/IRON_ASCENDANCY_UNITS.md) | ✓ | Extended | armed | 1 | none | E-R / P |
| IA-ARM-08 | Rapid Anti-Air Autocannon | [Iron Ascendancy](../Units/IRON_ASCENDANCY_UNITS.md) | ✓ | Core | armed | 1 | none | E-R / P |
| IA-ARM-09 | Heavy Anti-Tank Gun Carrier | [Iron Ascendancy](../Units/IRON_ASCENDANCY_UNITS.md) | ✓ | Core | armed | 1 | none | E-R / P |
| IA-ARM-10 | Mobile Rocket Launcher | [Iron Ascendancy](../Units/IRON_ASCENDANCY_UNITS.md) | ✓ | Core | armed | 1 | none | E-R / P |
| IA-ARM-11 | Self-Propelled Breaching Artillery | [Iron Ascendancy](../Units/IRON_ASCENDANCY_UNITS.md) | ✓ | Extended | armed | 1 | none | E-R / P |
| IA-ARM-12 | Mobile Field Bunker | [Iron Ascendancy](../Units/IRON_ASCENDANCY_UNITS.md) | ✓ | Extended | armed | 0 | none | E-R / P |
| IA-ARM-13 | Attack Engineering Bulldozer | [Iron Ascendancy](../Units/IRON_ASCENDANCY_UNITS.md) | ✓ | Extended | armed | 3 | none | E-R / P |
| IA-ARM-14 | Tracked Signal Jammer | [Iron Ascendancy](../Units/IRON_ASCENDANCY_UNITS.md) | ✓ | Extended | unarmed | 0 | none | E-R / E / P |
| IA-ARM-15 | Armored Heavy Gun Emplacement Carrier | [Iron Ascendancy](../Units/IRON_ASCENDANCY_UNITS.md) | ✓ | Conditional | armed | 0 | none | E-R / P |
| IA-SUP-01 | Nightjar Recon Plane | [Iron Ascendancy](../Units/IRON_ASCENDANCY_UNITS.md) | ✓ | Core | unarmed | 1 | none | E / P |
| IA-SUP-02 | Mobile Works Command Vehicle | [Iron Ascendancy](../Units/IRON_ASCENDANCY_UNITS.md) | ✓ | Core | unarmed | 1 | none | E-R / P / U |
| IA-SUP-03 | Forced-Priority Construction Tractor | [Iron Ascendancy](../Units/IRON_ASCENDANCY_UNITS.md) | ✓ | Extended | unarmed | 1 | none | E-R / E / P |
| IA-SUP-04 | Fuel Column Tanker | [Iron Ascendancy](../Units/IRON_ASCENDANCY_UNITS.md) | ✓ | Extended | unarmed | 0 | none | E-R / E / P |
| IA-SUP-05 | Battlefield Recovery Tractor | [Iron Ascendancy](../Units/IRON_ASCENDANCY_UNITS.md) | ✓ | Core | unarmed | 0 | none | E-R / P / U |
| IA-SUP-06 | Camouflaged Supply Truck | [Iron Ascendancy](../Units/IRON_ASCENDANCY_UNITS.md) | ✓ | Conditional | unarmed | 1 | none | E-R / P |
| IA-AIR-01 | Interceptor Fighter | [Iron Ascendancy](../Units/IRON_ASCENDANCY_UNITS.md) | ✓ | Extended | armed | 0 | none | E-R / P |
| IA-AIR-02 | Low-Level Attack Jet | [Iron Ascendancy](../Units/IRON_ASCENDANCY_UNITS.md) | ✓ | Extended | armed | 1 | none | E-R / P |
| IA-AIR-03 | Assault Helicopter | [Iron Ascendancy](../Units/IRON_ASCENDANCY_UNITS.md) | ✓ | Core | armed | 0 | none | E-R / P |
| IA-AIR-04 | Night Transport Helicopter | [Iron Ascendancy](../Units/IRON_ASCENDANCY_UNITS.md) | ✓ | Extended | armed | 1 | none | E-R / P |
| IA-AIR-05 | Tactical Bomber | [Iron Ascendancy](../Units/IRON_ASCENDANCY_UNITS.md) | ✓ | Extended | armed | 0 | none | E-R / P |
| IA-AIR-06 | Gunship Escort Aircraft | [Iron Ascendancy](../Units/IRON_ASCENDANCY_UNITS.md) | ✓ | Extended | armed | 1 | none | E-R / P |
| IA-AIR-07 | Prototype Ground-Attack Aircraft | [Iron Ascendancy](../Units/IRON_ASCENDANCY_UNITS.md) | ✓ | Conditional | armed | 1 | none | E-R / P |
| IA-AIR-08 | Covert Air Courier | [Iron Ascendancy](../Units/IRON_ASCENDANCY_UNITS.md) | ✓ | Conditional | unarmed | 2 | none | E-R / P |
| IA-SEA-01 | Harbor Patrol Cutter | [Iron Ascendancy](../Units/IRON_ASCENDANCY_UNITS.md) | ✓ | Conditional | armed | 1 | none | E-R / P |
| IA-SEA-02 | Landing Transport Craft | [Iron Ascendancy](../Units/IRON_ASCENDANCY_UNITS.md) | ✓ | Conditional | unarmed | 0 | none | E-R / P |
| IA-SEA-03 | Fast Raider Boat | [Iron Ascendancy](../Units/IRON_ASCENDANCY_UNITS.md) | ✓ | Conditional | armed | 0 | none | E-R / P |
| IA-SEA-04 | Covert Attack Submarine | [Iron Ascendancy](../Units/IRON_ASCENDANCY_UNITS.md) | ✓ | Conditional | armed | 1 | none | E-R / P |
| IA-SEA-05 | Armored Coastal Gunboat | [Iron Ascendancy](../Units/IRON_ASCENDANCY_UNITS.md) | ✓ | Conditional | armed | 0 | none | E-R / P |
| IA-SEA-06 | Auxiliary Seaborne Headquarters | [Iron Ascendancy](../Units/IRON_ASCENDANCY_UNITS.md) | ✓ | Conditional | armed | 1 | none | E-R / P / U |
| MP-INF-01 | Regional Rifle Company | [Meridian Pact](../Units/MERIDIAN_PACT_UNITS.md) | ✓ | Core | armed | 1 | yes | E-R / I / P |
| MP-INF-02 | Contracted Guard Detail | [Meridian Pact](../Units/MERIDIAN_PACT_UNITS.md) | ✓ | Conditional | armed | 2 | yes | E-R / P |
| MP-INF-03 | Light Rocket Team | [Meridian Pact](../Units/MERIDIAN_PACT_UNITS.md) | ✓ | Core | armed | 1 | yes | E-R / P / U |
| MP-INF-04 | Hillside Mortar Team | [Meridian Pact](../Units/MERIDIAN_PACT_UNITS.md) | ✓ | Extended | armed | 1 | yes | E-R / P |
| MP-INF-05 | Bridge and Road Engineer | [Meridian Pact](../Units/MERIDIAN_PACT_UNITS.md) | ✓ | Core | armed | 1 | yes | E-R / E / P |
| MP-INF-06 | Portable Field Medic | [Meridian Pact](../Units/MERIDIAN_PACT_UNITS.md) | ✓ | Core | armed | 2 | yes | E-R / P |
| MP-INF-07 | Caravan Reconnaissance Scouts | [Meridian Pact](../Units/MERIDIAN_PACT_UNITS.md) | ✓ | Extended | armed | 2 | yes | E-R / E / P |
| MP-INF-08 | Early Sabotage Specialist | [Meridian Pact](../Units/MERIDIAN_PACT_UNITS.md) | ✓ | Core | armed | 1 | yes | E-R / E / I / P |
| MP-INF-09 | Raiding Engineer | [Meridian Pact](../Units/MERIDIAN_PACT_UNITS.md) | ✓ | Extended | armed | 2 | yes | E-R / E / P |
| MP-INF-10 | Mountain Ambush Squad | [Meridian Pact](../Units/MERIDIAN_PACT_UNITS.md) | ✓ | Core | armed | 2 | yes | E-R / E / P / U |
| MP-INF-11 | Contracted Veteran Marksman | [Meridian Pact](../Units/MERIDIAN_PACT_UNITS.md) | ✓ | Conditional | armed | 2 | yes | E-R / P |
| MP-INF-12 | Local Militia Levy | [Meridian Pact](../Units/MERIDIAN_PACT_UNITS.md) | ✓ | Extended | armed | 2 | none | E-R / E / P |
| MP-INF-13 | Foreign Military Adviser | [Meridian Pact](../Units/MERIDIAN_PACT_UNITS.md) | ✓ | Extended | armed | 0 | none | E-R / E / P / U |
| MP-ARM-01 | Roadrunner Scout Truck | [Meridian Pact](../Units/MERIDIAN_PACT_UNITS.md) | ✓ | Core | armed | 2 | none | E-R / E / P |
| MP-ARM-02 | Armed Commercial Pickup | [Meridian Pact](../Units/MERIDIAN_PACT_UNITS.md) | ✓ | Core | armed | 1 | none | E-R / P |
| MP-ARM-03 | Light Armored Van | [Meridian Pact](../Units/MERIDIAN_PACT_UNITS.md) | ✓ | Core | armed | 1 | none | E-R / P |
| MP-ARM-04 | Modular Cargo Gun Truck | [Meridian Pact](../Units/MERIDIAN_PACT_UNITS.md) | ✓ | Extended | armed | 1 | none | E-R / P |
| MP-ARM-05 | Scavenged Light Tank | [Meridian Pact](../Units/MERIDIAN_PACT_UNITS.md) | ✓ | Extended | armed | 1 | none | E-R / P |
| MP-ARM-06 | Surplus Battle Tank | [Meridian Pact](../Units/MERIDIAN_PACT_UNITS.md) | ✓ | Core | armed | 1 | none | E-R / P |
| MP-ARM-07 | Mobile Recoilless-Gun Carrier | [Meridian Pact](../Units/MERIDIAN_PACT_UNITS.md) | ✓ | Core | armed | 1 | none | E-R / P |
| MP-ARM-08 | Truck-Mounted Rocket Rack | [Meridian Pact](../Units/MERIDIAN_PACT_UNITS.md) | ✓ | Core | armed | 1 | none | E-R / P / U |
| MP-ARM-09 | Quad Anti-Air Technical | [Meridian Pact](../Units/MERIDIAN_PACT_UNITS.md) | ✓ | Core | armed | 0 | none | E-R / P |
| MP-ARM-10 | Mobile Smoke-Screen Truck | [Meridian Pact](../Units/MERIDIAN_PACT_UNITS.md) | ✓ | Extended | unarmed | 2 | none | E-R / E / P |
| MP-ARM-11 | Ambush Mine Carrier | [Meridian Pact](../Units/MERIDIAN_PACT_UNITS.md) | ✓ | Extended | unarmed | 2 | none | E-R / P |
| MP-ARM-12 | Rough-Terrain Artillery Truck | [Meridian Pact](../Units/MERIDIAN_PACT_UNITS.md) | ✓ | Extended | armed | 2 | none | E-R / P / U |
| MP-ARM-13 | Captured Heavy Armor | [Meridian Pact](../Units/MERIDIAN_PACT_UNITS.md) | ✓ | Conditional | armed | 1 | none | E-R / P / U |
| MP-SUP-01 | Forward Trade Constructor | [Meridian Pact](../Units/MERIDIAN_PACT_UNITS.md) | ✓ | Core | unarmed | 2 | none | E-R / E / P |
| MP-SUP-02 | Merchant Delivery Truck | [Meridian Pact](../Units/MERIDIAN_PACT_UNITS.md) | ✓ | Extended | unarmed | 1 | none | E-R / E / P |
| MP-SUP-03 | Crude Tanker Convoy | [Meridian Pact](../Units/MERIDIAN_PACT_UNITS.md) | ✓ | Extended | unarmed | 1 | none | E-R / E / P / U |
| MP-SUP-04 | Petroleum Product Tanker | [Meridian Pact](../Units/MERIDIAN_PACT_UNITS.md) | ✓ | Extended | unarmed | 1 | none | E-R / E / P |
| MP-SUP-05 | Commercial Fuel Bowser | [Meridian Pact](../Units/MERIDIAN_PACT_UNITS.md) | ✓ | Extended | unarmed | 1 | none | E-R / E / P |
| MP-SUP-06 | Cargo Brokerage Van | [Meridian Pact](../Units/MERIDIAN_PACT_UNITS.md) | ✓ | Extended | unarmed | 2 | none | E-R / P |
| MP-SUP-07 | Mercenary Personnel Carrier | [Meridian Pact](../Units/MERIDIAN_PACT_UNITS.md) | ✓ | Extended | armed | 1 | none | E-R / P |
| MP-SUP-08 | Repair Caravan | [Meridian Pact](../Units/MERIDIAN_PACT_UNITS.md) | ✓ | Core | unarmed | 1 | none | E-R / P |
| MP-SUP-09 | Black-Market Delivery Aircraft | [Meridian Pact](../Units/MERIDIAN_PACT_UNITS.md) | ✓ | Conditional | unarmed | 1 | none | E-R / E / P |
| MP-AIR-01 | Light Utility Helicopter | [Meridian Pact](../Units/MERIDIAN_PACT_UNITS.md) | ✓ | Extended | unarmed | 1 | none | E-R / E / P |
| MP-AIR-02 | Armed Escort Helicopter | [Meridian Pact](../Units/MERIDIAN_PACT_UNITS.md) | ✓ | Core | armed | 2 | none | E-R / P |
| MP-AIR-03 | Imported Fighter-Bomber | [Meridian Pact](../Units/MERIDIAN_PACT_UNITS.md) | ✓ | Extended | armed | 0 | none | E-R / P |
| MP-AIR-04 | Cargo Transport Plane | [Meridian Pact](../Units/MERIDIAN_PACT_UNITS.md) | ✓ | Extended | unarmed | 1 | none | E-R / P |
| MP-AIR-05 | Coastal Patrol Aircraft | [Meridian Pact](../Units/MERIDIAN_PACT_UNITS.md) | ✓ | Extended | unarmed | 1 | none | E-R / E / P |
| MP-AIR-06 | Hired Air Support Flight | [Meridian Pact](../Units/MERIDIAN_PACT_UNITS.md) | ✓ | Conditional | armed | 2 | none | E-R / E / P / U |
| MP-SEA-01 | Merchant Coastal Barge | [Meridian Pact](../Units/MERIDIAN_PACT_UNITS.md) | ✓ | Conditional | unarmed | 1 | none | E-R / P |
| MP-SEA-02 | Fast Maritime Scout | [Meridian Pact](../Units/MERIDIAN_PACT_UNITS.md) | ✓ | Conditional | armed | 0 | none | E-R / P |
| MP-SEA-03 | Contract Security Patrol Boat | [Meridian Pact](../Units/MERIDIAN_PACT_UNITS.md) | ✓ | Conditional | armed | 2 | none | E-R / P |
| MP-SEA-04 | Amphibious Cargo Carrier | [Meridian Pact](../Units/MERIDIAN_PACT_UNITS.md) | ✓ | Conditional | unarmed | 0 | none | E-R / P |
| MP-SEA-05 | Surplus Coastal Submarine | [Meridian Pact](../Units/MERIDIAN_PACT_UNITS.md) | ✓ | Conditional | armed | 1 | none | E-R / P |
| MP-SEA-06 | Licensed Missile Corvette | [Meridian Pact](../Units/MERIDIAN_PACT_UNITS.md) | ✓ | Conditional | armed | 0 | none | E-R / P |
| MP-SEA-07 | Floating Trade Anchorage | [Meridian Pact](../Units/MERIDIAN_PACT_UNITS.md) | ✓ | Conditional | unarmed | 1 | none | E-R / P / U |
| MP-MKT-01 | Imported Heavy Recovery Crawler | [Meridian Pact](../Units/MERIDIAN_PACT_UNITS.md) | ✓ | Conditional | unarmed | 1 | none | E-R / P |
| MP-MKT-02 | Experimental Mine-Clearing Robot | [Meridian Pact](../Units/MERIDIAN_PACT_UNITS.md) | ✓ | Conditional | unarmed | 1 | none | E-R / P |
| MP-MKT-03 | Surplus Antiship Launcher Trailer | [Meridian Pact](../Units/MERIDIAN_PACT_UNITS.md) | ✓ | Conditional | armed | 1 | none | E-R / P |
| MP-MKT-04 | Amphibious Bridging Kit Carrier | [Meridian Pact](../Units/MERIDIAN_PACT_UNITS.md) | ✓ | Conditional | unarmed | 2 | none | E-R / P |
| MP-MKT-05 | Foreign Surveillance Radar Package | [Meridian Pact](../Units/MERIDIAN_PACT_UNITS.md) | ✓ | Conditional | unarmed | 1 | none | E-R / P |
| MP-MKT-06 | Commercial Decoy Convoy | [Meridian Pact](../Units/MERIDIAN_PACT_UNITS.md) | ✓ | Conditional | unarmed | 1 | none | E-R / E / P |
| MP-MKT-07 | Contracted Light Strike Flight | [Meridian Pact](../Units/MERIDIAN_PACT_UNITS.md) | ✓ | Conditional | armed | 1 | none | E-R / E / P / U |
| MP-MKT-08 | High-End Coastal Interceptor | [Meridian Pact](../Units/MERIDIAN_PACT_UNITS.md) | ✓ | Conditional | armed | 0 | none | E-R / P |
| MP-MKT-09 | Artillery Observation Balloon | [Meridian Pact](../Units/MERIDIAN_PACT_UNITS.md) | ✓ | Conditional | unarmed | 1 | none | E-R / P / U |
| UN-INF-01 | Yugoslav Army Rifle Section | [The Unaligned](../Units/UNALIGNED_UNITS.md) | ✓ | Core | armed | 1 | none | E-R / E / P |
| UN-INF-02 | Territorial Defence Reserve | [The Unaligned](../Units/UNALIGNED_UNITS.md) | ✓ | Core | armed | 2 | none | E-R / E / P |
| UN-INF-03 | Federal Border Guards | [The Unaligned](../Units/UNALIGNED_UNITS.md) | ✓ | Core | armed | 2 | none | E-R / E / P |
| UN-INF-04 | Federal Police Patrol | [The Unaligned](../Units/UNALIGNED_UNITS.md) | ✓ | Core | armed | 2 | none | E-R / E / P |
| UN-INF-05 | Railway Protection Detachment | [The Unaligned](../Units/UNALIGNED_UNITS.md) | ✓ | Extended | armed | 1 | none | E-R / P |
| UN-INF-06 | JNA Rocket Infantry | [The Unaligned](../Units/UNALIGNED_UNITS.md) | ✓ | Core | armed | 0 | none | E-R / P |
| UN-INF-07 | Federal Engineers | [The Unaligned](../Units/UNALIGNED_UNITS.md) | ✓ | Core | unarmed | 2 | none | E-R / P |
| UN-INF-08 | JNA Field Medical Unit | [The Unaligned](../Units/UNALIGNED_UNITS.md) | ✓ | Extended | unarmed | 1 | none | E-R / P |
| UN-INF-09 | Mountain Reconnaissance Patrol | [The Unaligned](../Units/UNALIGNED_UNITS.md) | ✓ | Extended | armed | 0 | none | E-R / P |
| UN-INF-10 | State Security Investigator | [The Unaligned](../Units/UNALIGNED_UNITS.md) | ✓ | Extended | unarmed | 1 | none | E-R / E / P |
| UN-INF-11 | Federal Counterintelligence Officer | [The Unaligned](../Units/UNALIGNED_UNITS.md) | ✓ | Core | unarmed | 2 | none | E-R / E / P |
| UN-INF-12 | Deception Bureau Liaison | [The Unaligned](../Units/UNALIGNED_UNITS.md) | ✓ | Scenario | unarmed | 3 | none | E / P |
| UN-INF-13 | Public Continuity Commissioner | [The Unaligned](../Units/UNALIGNED_UNITS.md) | ✓ | Scenario | unarmed | 1 | none | E-R / E / P |
| UN-INF-14 | Federal Honor Guard | [The Unaligned](../Units/UNALIGNED_UNITS.md) | ✓ | Scenario | armed | 1 | none | E-R / P |
| UN-ARM-01 | Municipal Patrol Car | [The Unaligned](../Units/UNALIGNED_UNITS.md) | ✓ | Core | unarmed | 2 | none | E-R / E / P |
| UN-ARM-02 | Federal Recon Jeep | [The Unaligned](../Units/UNALIGNED_UNITS.md) | ✓ | Core | armed | 0 | none | E-R / P |
| UN-ARM-03 | Border Surveillance Truck | [The Unaligned](../Units/UNALIGNED_UNITS.md) | ✓ | Extended | unarmed | 1 | none | E-R / P |
| UN-ARM-04 | JNA Armored Personnel Carrier | [The Unaligned](../Units/UNALIGNED_UNITS.md) | ✓ | Core | armed | 0 | none | E-R / P |
| UN-ARM-05 | Territorial Defence Truck | [The Unaligned](../Units/UNALIGNED_UNITS.md) | ✓ | Extended | unarmed | 0 | none | E-R / P |
| UN-ARM-06 | Federal Battle Tank | [The Unaligned](../Units/UNALIGNED_UNITS.md) | ✓ | Core | armed | 0 | none | E-R / E / P |
| UN-ARM-07 | Reserve Tank | [The Unaligned](../Units/UNALIGNED_UNITS.md) | ✓ | Extended | armed | 0 | none | E-R / P |
| UN-ARM-08 | JNA Anti-Air Gun Carrier | [The Unaligned](../Units/UNALIGNED_UNITS.md) | ✓ | Core | armed | 1 | none | E-R / E / P |
| UN-ARM-09 | Army Rocket Artillery | [The Unaligned](../Units/UNALIGNED_UNITS.md) | ✓ | Core | armed | 0 | none | E-R / P |
| UN-ARM-10 | Field Howitzer Tractor | [The Unaligned](../Units/UNALIGNED_UNITS.md) | ✓ | Extended | armed | 1 | none | E-R / P |
| UN-ARM-11 | Military Engineering Dozer | [The Unaligned](../Units/UNALIGNED_UNITS.md) | ✓ | Extended | unarmed | 2 | none | E-R / P |
| UN-ARM-12 | Federal Mobile Repair Unit | [The Unaligned](../Units/UNALIGNED_UNITS.md) | ✓ | Extended | unarmed | 0 | none | E-R / P |
| UN-ARM-13 | Anti-Armor Defense Vehicle | [The Unaligned](../Units/UNALIGNED_UNITS.md) | ✓ | Extended | armed | 0 | none | E-R / P |
| UN-ARM-14 | Reserve Mobilization Carrier | [The Unaligned](../Units/UNALIGNED_UNITS.md) | ✓ | Extended | unarmed | 0 | none | E-R / P |
| UN-ARM-15 | Crisis Command Vehicle | [The Unaligned](../Units/UNALIGNED_UNITS.md) | ✓ | Extended | unarmed | 1 | none | E-R / E / P |
| UN-CIV-01 | Civilian Passenger Car | [The Unaligned](../Units/UNALIGNED_UNITS.md) | ✓ | Core | unarmed | 1 | none | E-R / E / P |
| UN-CIV-02 | Intercity Bus | [The Unaligned](../Units/UNALIGNED_UNITS.md) | ✓ | Extended | unarmed | 1 | none | E-R / P |
| UN-CIV-03 | Freight Lorry | [The Unaligned](../Units/UNALIGNED_UNITS.md) | ✓ | Core | unarmed | 1 | none | E-R / E / P |
| UN-CIV-04 | Railway Freight Train | [The Unaligned](../Units/UNALIGNED_UNITS.md) | ✓ | Conditional | unarmed | 2 | none | E-R / E / P |
| UN-CIV-05 | Oil Tanker Lorry | [The Unaligned](../Units/UNALIGNED_UNITS.md) | ✓ | Extended | unarmed | 1 | none | E-R / E / P |
| UN-CIV-06 | Public Ambulance | [The Unaligned](../Units/UNALIGNED_UNITS.md) | ✓ | Core | unarmed | 1 | none | E-R / P |
| UN-CIV-07 | Civilian Fire Engine | [The Unaligned](../Units/UNALIGNED_UNITS.md) | ✓ | Extended | unarmed | 2 | none | E-R / P |
| UN-CIV-08 | Electrical Maintenance Van | [The Unaligned](../Units/UNALIGNED_UNITS.md) | ✓ | Extended | unarmed | 1 | none | E-R / E / P |
| UN-CIV-09 | Postal and Press Delivery Van | [The Unaligned](../Units/UNALIGNED_UNITS.md) | ✓ | Extended | unarmed | 1 | none | E-R / E / P |
| UN-CIV-10 | Agricultural Tractor | [The Unaligned](../Units/UNALIGNED_UNITS.md) | ✓ | Extended | unarmed | 0 | none | E-R / P |
| UN-CIV-11 | Commercial Fishing Boat | [The Unaligned](../Units/UNALIGNED_UNITS.md) | ✓ | Conditional | unarmed | 0 | none | E-R / P |
| UN-CIV-12 | River Ferry | [The Unaligned](../Units/UNALIGNED_UNITS.md) | ✓ | Conditional | unarmed | 2 | none | E-R / E / P |
| UN-AIR-01 | JNA Interceptor Fighter | [The Unaligned](../Units/UNALIGNED_UNITS.md) | ✓ | Core | armed | 1 | none | E-R / E / P |
| UN-AIR-02 | JNA Ground-Attack Jet | [The Unaligned](../Units/UNALIGNED_UNITS.md) | ✓ | Extended | armed | 0 | none | E-R / E / P |
| UN-AIR-03 | Federal Transport Helicopter | [The Unaligned](../Units/UNALIGNED_UNITS.md) | ✓ | Extended | unarmed | 1 | none | E-R / P |
| UN-AIR-04 | Army Attack Helicopter | [The Unaligned](../Units/UNALIGNED_UNITS.md) | ✓ | Extended | armed | 0 | none | E-R / P |
| UN-AIR-05 | Federal Reconnaissance Aircraft | [The Unaligned](../Units/UNALIGNED_UNITS.md) | ✓ | Extended | unarmed | 1 | none | E-R / E / P |
| UN-AIR-06 | Civil Passenger Airliner | [The Unaligned](../Units/UNALIGNED_UNITS.md) | ✓ | Scenario | unarmed | 1 | none | E-R / E / P |
| UN-AIR-07 | Emergency Airlift Plane | [The Unaligned](../Units/UNALIGNED_UNITS.md) | ✓ | Scenario | unarmed | 1 | none | E-R / P |
| UN-AIR-08 | Adriatic Maritime Patrol Plane | [The Unaligned](../Units/UNALIGNED_UNITS.md) | ✓ | Conditional | unarmed | 1 | none | E-R / P |
| UN-SEA-01 | JNA Coastal Patrol Boat | [The Unaligned](../Units/UNALIGNED_UNITS.md) | ✓ | Conditional | armed | 1 | none | E-R / P |
| UN-SEA-02 | Federal Missile Patrol Boat | [The Unaligned](../Units/UNALIGNED_UNITS.md) | ✓ | Conditional | armed | 0 | none | E-R / P |
| UN-SEA-03 | Adriatic Frigate Escort | [The Unaligned](../Units/UNALIGNED_UNITS.md) | ✓ | Conditional | armed | 0 | none | E-R / P |
| UN-SEA-04 | JNA Attack Submarine | [The Unaligned](../Units/UNALIGNED_UNITS.md) | ✓ | Conditional | armed | 0 | none | E-R / P |
| UN-SEA-05 | Amphibious Military Transport | [The Unaligned](../Units/UNALIGNED_UNITS.md) | ✓ | Conditional | armed | 0 | none | E-R / P |
| UN-SEA-06 | River Police Launch | [The Unaligned](../Units/UNALIGNED_UNITS.md) | ✓ | Conditional | armed | 0 | none | E-R / P |
| UN-SEA-07 | Civil Port Tug | [The Unaligned](../Units/UNALIGNED_UNITS.md) | ✓ | Conditional | unarmed | 1 | none | E-R / P |
| UN-SEA-08 | Federal Search-and-Rescue Vessel | [The Unaligned](../Units/UNALIGNED_UNITS.md) | ✓ | Conditional | unarmed | 1 | none | E-R / P |

## 8. Publication notes

- Verdicts (Validation and Governance §3): link/reference, roster-identity, section, canon-label and price-validator checks **PASS** on the commit base `4371124`. Unreal build, automation, packaged gameplay and multiplayer tests: **NOT_RUN** (documentation-only change; no unit is implemented).
- New files: `Docs/UNIT_SPECIFICATION_FRAMEWORK.md`, `Docs/UNIT_BALANCE_AND_ABILITY_MATRICES.md`, five files under `Docs/Units/` and this report.
- Edited canonical documents: `Docs/POLITIKA_FIVE_FACTION_ROSTERS.md` (links from the character sheet to the specifications, rows unchanged) and `Docs/CNC_GAMEPLAY_COMPARATIVE_STUDY.md` (new §5, classic unit-system reference).
- `Docs/DOCUMENTATION_INDEX.md` and `README.md` were **not** edited: both had uncommitted multiplayer edits in the working checkout when this report was written. The roster and the comparative study link the new files. Recommended index change once that work lands: a "Unit specifications" entry linking `UNIT_SPECIFICATION_FRAMEWORK.md`, `UNIT_BALANCE_AND_ABILITY_MATRICES.md`, the per-faction files in `Units/` and `Reports/UNIT_SPECIFICATION_COVERAGE_20261029.md`.
