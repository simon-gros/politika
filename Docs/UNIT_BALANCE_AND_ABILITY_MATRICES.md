# POLITIKA — Unit Balance, Counter, Ability and Upgrade Matrices

**Status:** PROPOSED balance and interaction reference (29 October 2026). It is derived from the [Unit Specification Framework](UNIT_SPECIFICATION_FRAMEWORK.md) and the per-faction unit files. Nothing here is implemented or balance-approved, and every price, value and counter judgment is a Provisional Baseline for playtesting.
**Single source of truth:** each unit's own entry in its faction file. **Per-unit matrices** (role, HP, armor, speed, USD, producer, hard counters) live in each faction file's *Faction roster summary* and *Abilities and upgrades index*: [Atlantic Concord](Units/ATLANTIC_CONCORD_UNITS.md) · [Red Directorate](Units/RED_DIRECTORATE_UNITS.md) · [Iron Ascendancy](Units/IRON_ASCENDANCY_UNITS.md) · [Meridian Pact](Units/MERIDIAN_PACT_UNITS.md) · [The Unaligned](Units/UNALIGNED_UNITS.md). This document holds only the **cross-faction** views. If a value here conflicts with a unit entry, the unit entry wins and this document must be corrected.

**Counter vocabulary.** A **hard counter** wins the matchup at comparable cost because of an effectiveness, eligibility or layer advantage: for example, a 0–15 % cell against the victim, or a layer the victim cannot shoot back at. A **soft counter** wins the matchup at comparable cost through range, speed, AoE or cost efficiency, but loses if misused. A **situational advantage** depends on terrain, observation, garrison, formation, jamming or timing.

## 1. Role coverage by faction

`—` means the faction has no ordinary unit in this role, which is an intended asymmetry unless the coverage report flags it. *Italic* entries are Conditional or Scenario units.

| Role | Atlantic Concord | Red Directorate | Iron Ascendancy | Meridian Pact | The Unaligned (NPC) |
| --- | --- | --- | --- | --- | --- |
| Opening scout (E) | AC-SUP-01 unarmed plane | RD-ARM-01 rifle motorcycle | IA-SUP-01 unarmed plane | MP-ARM-01 fast scout truck (best overall) | UN-ARM-01/02 patrols |
| Line infantry | AC-INF-01 | RD-INF-01, RD-INF-02, RD-INF-07 | IA-INF-01 | MP-INF-01, MP-INF-12 | UN-INF-01, UN-INF-02, UN-INF-05 |
| Close assault infantry | AC-INF-02 | — | IA-INF-02, IA-INF-05 (flame) | — | UN-INF-04 (police) |
| Anti-armor infantry | AC-INF-03 (+ limited AA) | RD-INF-04 (+ limited AA) | IA-INF-04 | MP-INF-03 | UN-INF-06 |
| Suppression and indirect infantry | AC-INF-04 | RD-INF-03 HMG, RD-INF-05 mortar | IA-INF-03 | MP-INF-04 mortar, MP-INF-10 ambush | — |
| Medic | AC-INF-05 | — (replenishment doctrine) | IA-INF-06 | MP-INF-06 | UN-INF-08 |
| Engineer and capture | AC-INF-06 | RD-INF-06 | IA-INF-07 | MP-INF-05, MP-INF-09 | UN-INF-07 |
| Airborne or air-assault infantry | AC-INF-07 | (RD-AIR-04/05 lift) | (IA-AIR-04 insertion) | — | — |
| Marksman and recon infantry | AC-INF-08 | RD-INF-09 | IA-INF-08 | MP-INF-07, MP-INF-11 | UN-INF-09 |
| Special forces / commando | AC-INF-09 | (RD-INF-13 commander) | (IA-INF-13 commander candidate) | MP-INF-10 | *UN-INF-14 honor guard* |
| Counterintelligence and detection | AC-INF-10, AC-INF-13 | RD-INF-07 (P: `DETECT.SPY`) | IA-INF-12 | MP-INF-02 (P, contract guard) | UN-INF-10, UN-INF-11 (strongest, E), UN-INF-04 |
| Spy (late, spy-only sabotage for three factions, E) | AC-INF-11 | RD-INF-11 | IA-INF-09 | — (non-spy saboteurs instead, E) | UN-INF-10 investigates |
| Faction-exclusive covert specialty (E) | AC-INF-12 Psy-Ops | RD-INF-12 diversion; RD-INF-11 efficient theft | IA-INF-10 contamination; IA-INF-11 Intimidation | MP-INF-08/09 early conventional sabotage | UN-INF-12 false intelligence |
| Officer / Discipline support | — (solo exemption, E) | RD-INF-10 | — (IA-INF-12 is a security enforcer without an aura) | — | UN-INF-13, UN-ARM-15 |
| Elite commander (talents) | AC-INF-14 | RD-INF-13 | IA-INF-13 (U-14) | MP-INF-13 (U-14) | — (E: no talent tree) |
| Light armed vehicle | AC-ARM-01 | RD-ARM-02 (truck) | IA-ARM-01 | MP-ARM-02, MP-ARM-04 | UN-ARM-01, UN-ARM-02 |
| APC / IFV | AC-ARM-02, AC-ARM-03 | RD-ARM-03 | IA-ARM-02 | MP-ARM-03 | UN-ARM-04 |
| Light or fast tank | — | RD-ARM-15 | IA-ARM-03 | MP-ARM-05 | UN-ARM-07 |
| Main battle tank | AC-ARM-04 | RD-ARM-04 | IA-ARM-04 | MP-ARM-06 | UN-ARM-06 |
| Heavy tank | AC-ARM-05 | RD-ARM-05 | IA-ARM-05 | *MP-ARM-13 (market or captured only)* | — |
| Prototype / centerpiece (cap 1) | *AC-ARM-15* | *RD-ARM-14* | *IA-ARM-06*, IA-ARM-15 | — | — |
| Tank destroyer / ATGM | AC-ARM-06 | — (mass armor, AT infantry, dug-in tanks) | IA-ARM-09 | MP-ARM-07 | UN-ARM-13 |
| Gun artillery | AC-ARM-07 | RD-ARM-08 | IA-ARM-11 | MP-ARM-12 | UN-ARM-10 |
| Rocket artillery | AC-ARM-08 (precision) | RD-ARM-07 (area) | IA-ARM-10 (saturation) | MP-ARM-08 | UN-ARM-09 |
| Mortar carrier | — | RD-ARM-09 | — | (MP-ARM-12) | — |
| Mobile AA gun | — | RD-ARM-06 | IA-ARM-08 | MP-ARM-09 | UN-ARM-08 |
| Mobile SAM | AC-ARM-09 | RD-ARM-13 | — | — | — |
| Flame | — | RD-ARM-10 | IA-ARM-07, IA-INF-05 | — | — |
| EW / jamming | AC-ARM-10 | RD-SUP-06 | IA-ARM-14 | *MP-MKT-05 (radar package)* | UN-ARM-03 |
| Concealment or smoke | AC-ARM-11 | — | — | MP-ARM-10 | — |
| Counter-battery and sensors | AC-ARM-12, AC-SUP-07 | (RD-DEF-07 structure) | — | *MP-MKT-09 balloon* | UN-ARM-03 |
| Mines and obstacles | AC-ARM-14 (breach) | RD-ARM-11 (clear) | IA-ARM-13 (dozer) | MP-ARM-11 (lay), *MP-MKT-02 (clear)* | UN-ARM-11 |
| Repair and recovery | AC-ARM-13 | RD-SUP-04 | IA-SUP-05 | MP-SUP-08, *MP-MKT-01* | UN-ARM-12 |
| Bridging | AC-ARM-14 | RD-SUP-05 | — | *MP-MKT-04*, MP-INF-05 | UN-ARM-11 |
| Construction | AC-SUP-02, AC-SUP-03 | RD-SUP-01 | IA-SUP-02, IA-SUP-03 | MP-SUP-01 | — (institutional) |
| Fuel and supply logistics | AC-SUP-04/05/06 | RD-SUP-02/03 | IA-SUP-04, IA-SUP-06 | MP-SUP-02..06, MP-SUP-09 | UN-CIV-03/04/05 |
| Reinforcement delivery | — | RD-SUP-07 | — | MP-SUP-07 (contract troops) | UN-ARM-14 |
| Fighter / interceptor | AC-AIR-01 | RD-AIR-01 | IA-AIR-01 | — (MP-AIR-03 fighter-bomber) | UN-AIR-01 |
| Strike jet | AC-AIR-02, AC-AIR-03 | RD-AIR-02 | IA-AIR-02, IA-AIR-07 | MP-AIR-03 | UN-AIR-02 |
| Bomber | AC-AIR-08 | RD-AIR-07 | IA-AIR-05 | — | — |
| Attack helicopter | AC-AIR-04, AC-AIR-09 | RD-AIR-03 | IA-AIR-03 | MP-AIR-02 | UN-AIR-04 |
| Gunship / contracted strike | AC-AIR-10 | — | IA-AIR-06 | MP-AIR-06, *MP-MKT-07* | — |
| Transport helicopter | AC-AIR-05 | RD-AIR-04 | IA-AIR-04 | MP-AIR-01 | UN-AIR-03 |
| Fixed-wing transport | AC-AIR-06 | RD-AIR-05 | IA-AIR-08 | MP-AIR-04, MP-SUP-09 | UN-AIR-07 |
| Surveillance aircraft | AC-AIR-07 | RD-AIR-06, RD-AIR-08 | — | MP-AIR-05 | UN-AIR-05, UN-AIR-08 |
| Strategic delivery (E) | *AC-AIR-11 nuclear bomber* | ICBM = RD-DEF-14 structure; RD-AIR-09 conventional (U-13) | none needed: first own bomb wins (E) | captured weapon + compatible delivery only (E) | never (E) |
| Naval patrol | AC-SEA-01 | RD-SEA-01 | IA-SEA-01 | MP-SEA-02, MP-SEA-03 | UN-SEA-01, UN-SEA-06 |
| Fast attack or missile craft | AC-SEA-03 | RD-SEA-03 | IA-SEA-03 | MP-SEA-06, *MP-MKT-08* | UN-SEA-02 |
| Major surface combatant | AC-SEA-04, AC-SEA-06 | RD-SEA-05, RD-SEA-06 | IA-SEA-05 | — | UN-SEA-03 |
| Submarine | AC-SEA-05 | RD-SEA-04 | IA-SEA-04 | MP-SEA-05 | UN-SEA-04 |
| Amphibious and sea lift | AC-SEA-02 | RD-SEA-02 | IA-SEA-02 | MP-SEA-01, MP-SEA-04 | UN-SEA-05 |
| Floating HQ | *AC-SEA-07* | RD-SEA-07 | IA-SEA-06 | MP-SEA-07 | — |
| Decoys and deception | — | — | IA-SUP-06 (falsified manifests) | *MP-MKT-06 decoy convoy* | UN-INF-12 (reports, E) |

## 2. Archetype counter web (P)

Rows are the attacking archetype and columns the target archetype. `++` is a hard counter, `+` a soft counter (favorable), `=` even, `−` unfavorable (the target soft-counters the attacker), `−−` hard-countered, and `·` means the attacker cannot meaningfully engage the target. Judgments assume comparable cost and observation, and use [framework §6.3](UNIT_SPECIFICATION_FRAMEWORK.md#63-effectiveness-matrix-percent-of-base-damage).

| Attacker ↓ / Target → | Line INF | AT INF | Flame / HW INF | Marksman | Agent | Scout veh | Light veh | APC/IFV | MBT | Heavy | Artillery | Mobile AA | Heli | Jet | Structure |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| Line infantry | = | + | − | − | ++ | + | = | − | · | · | + (if reached) | + (if reached) | − | · | − |
| AT infantry | − | = | − | − | + | ++ | ++ | ++ | + | = | ++ | + | = (AA-capable) / · | · (shoulder AA: −) | + |
| HMG / flame / grenadier | ++ | ++ | = | + | ++ | + | = | − | · | · | + | = | · (HMG −) | · | + (garrison ++) |
| Marksman (concealed) | ++ | ++ | ++ | = | ++ | · | · | · | · | · | · | · | · | · | · |
| Scout vehicle | + | − | − | − | + | = | − | −− | −− | −− | + | + | − | · | · |
| Light armed vehicle | + | −− | = | + | ++ | ++ | = | − | −− | −− | ++ | + | + (MG) | · | − |
| APC / IFV | + | − | + | + | + | ++ | + | = | − | −− | + | + | + (autocannon) | · | − |
| Main battle tank | = | − (in cover) | + | · | · | ++ | ++ | ++ | = | − | ++ | ++ | · | · | + |
| Heavy / siege tank | + | = | + | · | · | ++ | ++ | ++ | + | = | ++ | ++ | · | · | ++ |
| Tank destroyer / ATGM | − | − | − | − | · | + | ++ | ++ | ++ | + | + | + | · | · | = |
| Artillery (observed) | ++ | ++ | ++ | + | + | − (fast) | = | + | − | − | + (counter-battery) | + | · | · | ++ |
| Mobile AA | = (gun) / · (SAM) | − | − | · | · | + (gun) | + (gun) | − | −− | −− | · | = | ++ | ++ | · |
| Attack helicopter | + | − (AA-capable) / ++ | + | + | + | ++ | ++ | ++ | ++ | + | ++ | −− | = | −− | = |
| Strike jet / bomber | + | + | + | + | · | − (fast) | + | ++ | ++ | ++ | ++ | −− | − | −− | ++ |
| Fighter / interceptor | · | · | · | · | · | · | · | · | · | · | · | − | ++ | ++ (bombers, transports) | · |

**Readings that matter for play.**
- *Infantry against infantry:* rifles trade evenly. HMG and flame beat massed rifles, marksmen beat every infantry type from concealment, and grenadiers and flame clear garrisons (garrison 200 % INCENDIARY, +50 % grenade; framework §6.9).
- *Infantry against armor:* AT teams hard-counter light vehicles and APCs and soft-counter MBTs from cover. They lose to HMG, flame, artillery and infantry escorts.
- *Light against heavy vehicles:* light vehicles cannot hurt heavy armor (SMALL_ARMS 2 %, AUTOCANNON 10 %), but they outrun tank guns (tracking-class rule, framework §6.6) and kill artillery, logistics and agents.
- *Artillery against mobile forces:* artillery needs observation and loses to anything that reaches it, to counter-battery and to jets. Fast movers inside the scatter disk take partial hits.
- *Air against ground defenses:* helicopters dominate ground forces without AA and die to mobile AA guns. Jets strike anything and die to SAM coverage. Fighters control the air layer only.
- *Anti-air specialization:* AA units are near-useless against ground armor, which is the price of air denial.
- *Reconnaissance against stealth:* concealed units are visible inside 35 % of observer sight or to detectors. Agents are revealed by `DETECT.SPY`, so detector placement is the counter to covert play.
- *Support against direct combat:* medics, engineers and logistics units are unarmed or weak. They multiply a force's endurance and are priority targets for raiders and marksmen.

## 3. Faction strengths, weaknesses and asymmetry (E doctrine, P mechanics)

| Faction | Signature strengths | Exploitable weaknesses | Asymmetric mechanics | Typical counter-strategy against it |
| --- | --- | --- | --- | --- |
| Atlantic Concord | Air power and reconnaissance; precision artillery (AC-ARM-08); EW and concealment; resilient small detachments (E); expensive, durable professionals; Psy-Ops (E) | High prices and replacement cost; dependence on airfields, kerosene, communications and radar; no flame, mortar carrier, light tank or AA gun | No small-group Discipline penalty (E); Psy-Ops elimination or rampage (E); bomber nuclear delivery (E); cheaper refining (E) | Deny the air layer with AA and SAM; jam and destroy radar and comms nodes; trade cheap units for expensive ones; strike airfields and fuel |
| Red Directorate | Cheapest infantry by far (E); mass formations with the very-massive Discipline bonus (E); deep artillery; integrated air defense; railway logistics | Units are individually weaker; heavy armor is fuel-hungry; rail, bridge and depot dependence (E); no medic, no tank destroyer, no concealment | Infantry −45 % price (P of E); efficient intelligence theft and resource diversion (E); ICBM (E) | Precision artillery and air against massed formations; cut rail, bridges and depots; Psy-Ops against officers; isolate detachments |
| Iron Ascendancy | +8 % raw damage (P of E); concentrated elite mechanized assault; flame; heavy siege armor; Intimidation and contamination (E) | Costly losses; limited espionage early (E); Unaligned standing hostility (E); no mobile SAM; high-risk abilities self-damage | Mass Discipline bonus (E); Intimidation of isolated, fully broken units (E); facility contamination (E); first own bomb wins (E) | Keep units supported (no isolation); trade at range with AT and air; protect nuclear research sites from tier-3 spies; exploit Unaligned hostility |
| Meridian Pact | Best early recon (E); fastest raiders; cheap early non-spy sabotage (E); contracts and market flexibility; ambush terrain play | Light armor; no native heavy armor (market or captured only); no bombers; no advanced propaganda (E); contract expiry and wages; normal small-group penalty | Early conventional charges (E); Black Market conventional imports (E); fuel-field start (E) | Detection screens around bases; heavy armor and HMG escorts; interdict market deliveries; out-sustain wage costs |
| The Unaligned (NPC) | Sovereignty response at R0–R4; strongest counterintelligence (E); false intelligence (E); home terrain and civilian infrastructure | Reactive, not expansionist; per-participant hostility; scenario-budgeted mobilization | NPC-only (E); planted reports (E); no talents (E) | Avoid incidents; negotiate access; use legitimate reconnaissance to verify reports |


## 4. Cost and value across factions (P)

Prices come from each faction file's roster summary. Ranges show the cheapest and dearest unit on that template, with the unit count in brackets. Meridian contract units show the hire price; their wage per interval is extra. The Unaligned has no prices (mobilization weights only).

| Template | Atlantic Concord | Red Directorate | Iron Ascendancy | Meridian Pact | Reading |
| --- | --- | --- | --- | --- | --- |
| INF-LINE | 360–420 (4) | 160–210 (3) | 320–460 (3) | 250–280 (3) | Directorate mass; Concord premium (E) |
| INF-AT | 560 | 300 | 520 | 460 | Directorate AT is the cheapest and also carries limited AA |
| INF-HW | — (AC-INF-04 is line-templated, $400) | 260–340 (2) | 420–480 (2) | 520 | Directorate suppression is about half the price |
| INF-SUPPORT | 420–450 (2) | 240–280 (2) | 380–440 (2) | 360–440 (3) | |
| INF-SPEC | 600–1,200 (4) | 420 | 640 | 420–650 (4) | Meridian saboteurs are cheap; each charge costs extra |
| INF-COVERT | 1,300–1,600 (2) | 640–700 (2) | 300–1,400 (3) | — | Directorate theft is cheaper (E); Meridian has no spy (E) |
| INF-COMMAND | 2,400 | 420–1,250 (2) | 1,200–2,300 (2) | 2,000 | Includes officers and the Intimidation specialist |
| VEH-SCOUT | — (air scout) | 180 | — (air scout) | 150 | Meridian cheapest ground scout (E) |
| VEH-LIGHT | 560 | — | 400 | 350–550 (2) | |
| VEH-TRANSPORT / IFV | 900 / 1,000 | 250–800 (2) / 700–750 (2) | 850–1,150 (2) / — | 620–700 (2) / — | |
| VEH-MBT | 1,300 | 600–1,050 (3) | 1,050–1,300 (3) | 1,000 (+700 light) | |
| VEH-HEAVY | 2,300–4,200 (2) | 1,300–4,200 (3) | 2,400–4,500 (3) | 1,900 (market/captured) | Prototypes at the top of each range, cap 1 |
| VEH-TD | 1,250 | — | 1,250 | 900 | |
| VEH-ARTY | 1,500–1,700 (2) | 900–2,600 (4) | 1,500–1,700 (2) | 900–1,500 (3) | |
| VEH-AA | 1,200 | 900–1,050 (2) | 1,150 | 800 | |
| AIR-FIGHTER | 1,900 | 1,550 | 2,050 | — | Meridian has no native fighter |
| AIR-STRIKE | 2,000–3,200 (4) | 1,900–2,400 (2) | 2,150–3,300 (3) | 700–1,700 (3, hired/market) | |
| AIR-HELI-ATTACK | 1,000–1,650 (2) | 1,450 | 1,750–1,900 (2) | 1,200 (hired) | |
| NAV-SUB | 2,100 | 1,800 | 2,200 | 1,900 | |

**Directorate price validator (framework §4.4).** Each Directorate infantry role price × 1.5 must not exceed the cheapest comparable role in any other playable faction. Checked on 29 October 2026 against the final files:

| Directorate unit | Price × 1.5 | Cheapest comparable role elsewhere | Result |
| --- | --- | --- | --- |
| RD-INF-01 line | $263 | MP-INF-12 $270 (MP-INF-01 $280, IA-INF-01 $320, AC-INF-01 $360) | Pass |
| RD-INF-02 motorized rifles | $315 | AC-INF-02 $380 (IA-INF-02 $460) | Pass |
| RD-INF-03 HMG | $390 | AC-INF-04 $400 (IA-INF-03 $420) | Pass after the RD-INF-03 change from $330 to $260 |
| RD-INF-04 AT | $450 | MP-INF-03 $460 | Pass |
| RD-INF-05 mortar | $510 | MP-INF-04 $520 | Pass |
| RD-INF-06 engineer | $360 | MP-INF-05 $380 | Pass |
| RD-INF-07 depot guard / spy screen | $240 | MP-INF-02 $250 + wage | Pass |
| RD-INF-08 decontamination | $420 | none comparable | n/a (unique role) |
| RD-INF-09 recon | $630 | MP-INF-07 $640, IA-INF-08 $640 | Pass |
| RD-INF-10 political officer | $630 | none comparable (IA-INF-11 Intimidation $1,200 is a different role) | n/a |
| RD-INF-11 spy | $960 | IA-INF-09 $1,200 | Pass |
| RD-INF-12 diversion agent | $1,050 | none comparable | n/a (unique, E) |
| RD-INF-13 commander | $1,875 | MP-INF-13 $2,000 | Pass |

The validator is a proposed editor and runtime check (framework §18.3). It applies to base prices and to any research that changes a Directorate infantry price, which is why Directorate infantry technologies trade build time or speed rather than price where a comparable role exists. Any later price change on either side must re-run it.

## 5. Cross-faction ability matrix

Rows are ability families; cells name the units that carry them. E marks established canon, I the generic implemented behavior, and everything else is P.

| Ability family | Atlantic Concord | Red Directorate | Iron Ascendancy | Meridian Pact | The Unaligned (NPC) | Owning rule |
| --- | --- | --- | --- | --- | --- | --- |
| Infantry magazine, reload, grenade, sprint | All armed infantry (I behavior) | All armed infantry (I) | All armed infantry (I) | All armed infantry (I) | Armed personnel (I) | Framework §2 |
| Deploy, dig-in, emplace | AT and HW teams, artillery | HMG, mortar, dug-in tanks | HW, artillery | Mortar, recoilless, mines | Howitzer unlimber | Framework §13.3 |
| Concealment and smoke | Marksmen, SF, EW concealment | Recon only | Ambush AT, agents | Ambush squad, smoke carrier (not invisibility) | Recon patrol | Framework §9.1 |
| `DETECT.SPY` | AC-INF-10, AC-INF-13 | RD-INF-07 | IA-INF-12 | MP-INF-02 (contract guard) | UN-INF-03/04/10/11, UN-ARM-01/03 (strongest, E) | Framework §9.2 |
| Spy-only sabotage (late) | AC-INF-11 (E) | RD-INF-11 (E) | IA-INF-09 after espionage tiers (E) | — (E) | — | `tech.{ac,rd,ia}.late_espionage` → `late_espionage_unlocked` flag |
| Early conventional sabotage, no spy | — | — | — | MP-INF-08/09 (E) | — | Sabotage doc |
| Intelligence theft | AC-INF-11 | RD-INF-11 (faster and cheaper, E) | IA-INF-09 (nuclear infiltration gate, E) | — | — | Espionage doc |
| Psy-Ops (resolve collapse or rampage) | AC-INF-12 only (E) | — | — | — | Target only, under eligibility rules | Propaganda doc §Psy-Ops |
| Resource diversion | — | RD-INF-12 (E) | — | — | — | Espionage doc |
| Facility contamination | — | — | IA-INF-10 (E) | — | — | Espionage doc |
| Intimidation (conversion of broken, isolated units) | — | — | IA-INF-11 only (E) | — | Eligible only while hostile (E) | Espionage doc |
| False intelligence and false flags | — | — | IA-SUP-06 falsified manifests (P) | MP-SUP-06 falsified manifest, MP-MKT-06 decoys | UN-INF-12 only (E) | Unaligned Counterintelligence |
| Propaganda media | Yes (E) | Yes (E) | Yes (E) | **No** (E) | NPC media (E) | Propaganda and Morale |
| Nuclear delivery | AC-AIR-11 (E) | RD-DEF-14 silo (E); RD-AIR-09 conventional (U-13) | First bomb wins without delivery (E) | **No** (E) | **No** (E) | Nuclear Weapons System |
| Commander talents | AC-INF-14 | RD-INF-13 | IA-INF-13 (U-14) | MP-INF-13 (U-14) | **None** (E) | Commander Talent System |
| Officer resolve auras (no talents) | — (solo exemption, E) | RD-INF-10 | — | — | UN-ARM-15, UN-INF-13 | Framework §17 |
| Mass-army Discipline bonus | — | Yes (E) | Yes (E) | — | No (E) | Formations and Discipline |
| Small-group penalty exemption | Yes (E) | — | — | — | — | Formations and Discipline |
| Repair and salvage | Recovery vehicles (U-11) | RD-SUP-04 | Recovery vehicle (U-11) | Tow and capture (MP-ARM-13 source) | UN-ARM-11/12, UN-CIV-07/08 | Framework §10 |
| Medical replenishment | AC-INF-05 | — (replenish at producers) | IA-INF-06 | MP-INF-06 | UN-INF-08, UN-CIV-06 | Framework §10.3 |
| EW and jamming | AC-ARM-10 | RD-SUP-06 | IA-ARM-14 | Market radar package MP-MKT-05 | Detection only | Framework §9.5 |
| Counter-battery | AC-ARM-12 | Artillery spotting (RD-AIR-08) | — | — | — | Framework §9.2 |
| Contracts and wages | — | — | — | Contract units (E) | — | Framework §11.4 |
| Black Market (conventional only) | — | — | — | MP-MKT-01 to 09 (E) | — | Framework §11.5 |
| Graduated response and jurisdiction | Subject to it | Subject to it | Subject to it (+1 hostility, E) | Subject to it | Owner (E) | Framework §14.3 |

## 6. Upgrade and research relationships

Every unit-specific technology from the four playable faction files. No ages or eras (E). "Time" is research time. The Unaligned has no research; its two scenario flags are in its own file.

| Technology | Faction | Unit(s) | Kind | Cost | Time | Requires | Trade-off / counter |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `tech.ac.urban_security_procedures` | Atlantic Concord | AC-INF-02 | Tech | $500 | — | AC-BLD-04 | — |
| `tech.ac.improved_grenade_munitions` | Atlantic Concord | AC-INF-04 | Tech | $600 | — | AC-BLD-16 | — |
| `tech.ac.airborne_first_aid_kits` | Atlantic Concord | AC-INF-07 | Tech | $500 | — | AC-BLD-04 | — |
| `tech.ac.field_fortification` | Atlantic Concord | AC-INF-06 | Tech | $500 | — | AC-BLD-16 | — |
| `tech.ac.marksman_optics` | Atlantic Concord | AC-INF-08 | Tech | $500 | — | AC-BLD-16 | — |
| `tech.ac.security_vetting` | Atlantic Concord | AC-INF-10 | Tech | $600 | — | AC-BLD-16 | — |
| `tech.ac.late_espionage` | Atlantic Concord | AC-INF-11 | Tech (E gate) | $1,200 | — | AC-BLD-16 | — |
| `tech.ac.tradecraft` | Atlantic Concord | AC-INF-11 | Tech | $700 | — | AC-BLD-16 | — |
| `tech.ac.psychological_operations` | Atlantic Concord | AC-INF-12 | Tech (unlock) | $1,500 | — | AC-BLD-18 | — |
| `tech.ac.broadcast_saturation` | Atlantic Concord | AC-INF-12 | Tech | $900 | — | AC-BLD-18 | — |
| `tech.ac.precision_vehicle_targeting` | Atlantic Concord | AC-ARM-03/04/05/06, AC-INF-03, AC-AIR-04 | Tech (E candidate) | $900 | — | AC-BLD-16 | — |
| `tech.ac.light_vehicle_armor_kits` | Atlantic Concord | AC-ARM-01 | Tech | $500 | — | AC-BLD-05 | — |
| `tech.ac.add_on_armor` | Atlantic Concord | AC-ARM-02 | Tech | $600 | — | AC-BLD-05 | — |
| `tech.ac.modular_mounts` | Atlantic Concord | AC-ARM-03 | Tech | $600 | — | AC-BLD-16 | — |
| `tech.ac.composite_armor_package` | Atlantic Concord | AC-ARM-04 | Tech | $800 | — | AC-BLD-16 | — |
| `tech.ac.heavy_armor_program` | Atlantic Concord | AC-ARM-05 | Tech (unlock) | $1,500 | — | AC-BLD-16 | — |
| `tech.ac.improved_fire_control` | Atlantic Concord | AC-ARM-07, AC-SEA-06 | Tech | $800 | — | AC-BLD-16 | — |
| `tech.ac.guided_munitions_program` | Atlantic Concord | AC-ARM-08, AC-AIR-03 | Tech (unlock) | $1,200 | — | AC-BLD-16 | — |
| `tech.ac.recovery_doctrine` | Atlantic Concord | AC-ARM-13 | Tech | $600 | — | AC-BLD-17 | — |
| `tech.ac.prototype_armor_program` | Atlantic Concord | AC-ARM-15 | Tech (unlock) | $2,500 | — | AC-BLD-16, AC-BLD-17 | — |
| `tech.ac.defended_logistics` | Atlantic Concord | AC-SUP-03/04/06 | Tech (research doc candidate) | $600 | — | AC-BLD-16 | — |
| `tech.ac.aviation_servicing` | Atlantic Concord | all AC aircraft | Tech (research doc candidate) | $600 | — | AC-BLD-06 | — |
| `tech.ac.armored_airframe` | Atlantic Concord | AC-AIR-02 | Tech | $700 | — | AC-BLD-06 | — |
| `tech.ac.naval_servicing` | Atlantic Concord | AC-SEA-01..06 | Tech | $600 | — | AC-BLD-08 | — |
| `tech.rd.industrial_mobilization_methods` | Red Directorate | RD-INF-01/02/07, RD-SUP-07 musters | Faction tech (E candidate) | $700 | — | RD-BLD-02 | Lane block 60 s |
| `tech.rd.coordinated_defensive_fire` | Red Directorate | RD-INF-03/05, RD-ARM-07/08/12 | Faction tech (E candidate) | $600 | — | RD-BLD-02, RD-DEF-06 powered | Tower must be powered |
| `tech.rd.tandem_warheads` | Red Directorate | RD-INF-04 | Unit tech | $600 | — | RD-BLD-02 | Cooldown +10 % |
| `tech.rd.collective_protection` | Red Directorate | RD-INF-08 | Unit tech | $500 | — | RD-BLD-02 | Price +$40, speed −5 % |
| `tech.rd.recon_field_training` | Red Directorate | RD-INF-09 | Unit tech | $500 | — | RD-BLD-02 | Build time +2 s |
| `tech.rd.mass_political_education` | Red Directorate | RD-INF-10 | Unit tech | $700 | — | RD-BLD-16 | Officer cap 4 → 3 |
| `tech.rd.late_espionage` | Red Directorate | RD-INF-11 | Espionage gate (I flag) | $1,200 | — | RD-BLD-02 | — |
| `tech.rd.courier_networks` | Red Directorate | RD-INF-11 | Unit tech | $600 | — | RD-BLD-02 | Cap 4 → 3 |
| `tech.rd.resource_diversion_doctrine` | Red Directorate | RD-INF-12 | Unlock | $900 | — | RD-BLD-02 | — |
| `tech.rd.motorized_reconnaissance` | Red Directorate | RD-ARM-01 | Unit tech | $300 | — | RD-BLD-02 | Price +$20 |
| `tech.rd.applique_armor` | Red Directorate | RD-ARM-03/04/15 | Unit tech | $500 | — | RD-BLD-05 | Speed −5 % |
| `tech.rd.standardized_hull_production` | Red Directorate | RD-ARM-03/04/15 | Faction tech (E candidate: standardized components) | $900 | — | RD-BLD-05 | Lane block 60 s |
| `tech.rd.engine_efficiency_program` | Red Directorate | RD-ARM-05 | Unit tech | $900 | — | RD-BLD-02 | Speed −5 % |
| `tech.rd.extended_rocket_motors` | Red Directorate | RD-ARM-07 | Unit tech | $800 | — | RD-BLD-02 | Scatter +15 % |
| `tech.rd.siege_command_program` | Red Directorate | RD-ARM-14 | Unlock | $2,000 | — | RD-BLD-02 | Lane block 120 s |
| `tech.rd.rail_scheduling` | Red Directorate | RD-SUP-02 | Unit tech | $500 | — | RD-BLD-11 | Power +10 % |
| `tech.rd.repair_capacity` | Red Directorate | RD-SUP-04 | Faction tech (E candidate) | $600 | — | RD-BLD-17 | Price +$80 |
| `tech.rd.heavy_bridging` | Red Directorate | RD-SUP-05 | Unit tech | $600 | — | RD-BLD-04 | Deploy +4 s |
| `tech.rd.theater_missile_program` | Red Directorate | RD-AIR-09 | Unlock | $1,800 | — | RD-BLD-02 | Lane block 120 s |
| `tech.ia.espionage_tier_1` (= `advanced_clandestine_training`) | Iron Ascendancy | IA-INF-08/09/14, IA-SUP-06, IA-AIR-04/08 | Research | $800 | 60 s | IA-BLD-04 | — |
| `tech.ia.espionage_tier_2` | Iron Ascendancy | IA-INF-09/10 | Research | $1,400 | 120 s | tier 1, IA-BLD-14 | — |
| `tech.ia.espionage_tier_3` | Iron Ascendancy | IA-INF-09 | Research (E gate) | $2,200 | 180 s | tier 2 | — |
| `tech.ia.late_espionage` | Iron Ascendancy | IA-INF-09 | Research (I flag) | $1,200 | 90 s | IA-BLD-04 | — |
| `tech.ia.assault_doctrine` | Iron Ascendancy | IA-INF-01/02 | Research | $700 | — | IA-BLD-14 | trade-off: resolve loss / cooldown |
| `tech.ia.improved_launcher_rounds` | Iron Ascendancy | IA-INF-03 | Research | $600 | — | IA-BLD-14 | magazine −1 |
| `tech.ia.wire_guidance_improvement` | Iron Ascendancy | IA-INF-04 | Research | $650 | — | IA-BLD-14 | slower missile |
| `tech.ia.pressurised_projectors` | Iron Ascendancy | IA-INF-05, IA-ARM-07 | Research | $650 | — | IA-BLD-14 | less ammunition |
| `tech.ia.combat_pharmacology` | Iron Ascendancy | IA-INF-06 | Research | $700 | — | IA-BLD-14 | medic price +$60 |
| `tech.ia.sapper_training` | Iron Ascendancy | IA-INF-07, IA-ARM-13 | Research | $500 | — | IA-BLD-04 | price +$40 |
| `tech.ia.contamination_doctrine` | Iron Ascendancy | IA-INF-10 | Research | $1,000 | — | IA-BLD-14 | Unaligned incident level +1 |
| `tech.ia.coercion_methods` | Iron Ascendancy | IA-INF-11 | Research | $900 | — | IA-BLD-17 | Unaligned incident level +1 |
| `tech.ia.internal_security_apparatus` | Iron Ascendancy | IA-INF-12 | Research | $600 | — | IA-BLD-17 | Unaligned incident level +1 |
| `tech.ia.courier_networks` | Iron Ascendancy | IA-INF-14, IA-SUP-06, IA-AIR-08 | Research | $500 | — | IA-BLD-04 | courier cap −1 |
| `tech.ia.smoke_dischargers` | Iron Ascendancy | IA-ARM-01/03/04 | Research | $400 | — | IA-BLD-05 | price +$30 |
| `tech.ia.armored_bay_covers` | Iron Ascendancy | IA-ARM-02 | Research | $500 | — | IA-BLD-05 | slower, slower dismount |
| `tech.ia.uprated_engines` | Iron Ascendancy | IA-ARM-03 | Research | $800 | — | IA-BLD-06 | more self-damage |
| `tech.ia.composite_armor_upgrade` | Iron Ascendancy | IA-ARM-04/05 | Research | $1,000 | — | IA-BLD-06 | −5 % speed, +1 petroleum |
| `tech.ia.prototype_program` | Iron Ascendancy | IA-ARM-06/15, IA-AIR-07 | Research | $2,000 | 150 s | IA-BLD-06 | lane block |
| `tech.ia.tungsten_penetrators` | Iron Ascendancy | IA-ARM-09 | Research | $900 | — | IA-BLD-14 | price +$100, slower fire |
| `tech.ia.extended_range_rockets` | Iron Ascendancy | IA-ARM-10 | Research | $800 | — | IA-BLD-14 | more scatter |
| `tech.ia.fire_direction_center` | Iron Ascendancy | IA-ARM-10/11/15, IA-SEA-05 | Research | $900 | — | IA-BLD-14 | no unobserved fire |
| `tech.ia.reinforced_bunker_walls` | Iron Ascendancy | IA-ARM-12 | Research | $600 | — | IA-BLD-06 | slower deploy |
| `tech.ia.armored_fuel_bowsers` | Iron Ascendancy | IA-SUP-04 | Research | $500 | — | IA-BLD-05 | −10 % speed |
| `tech.ia.long_range_tanks` | Iron Ascendancy | IA-SUP-01 | Research | $400 | — | IA-BLD-07 | kerosene 1 → 2 |
| `tech.ia.extended_hardpoints` | Iron Ascendancy | IA-AIR-02 | Research | $700 | — | IA-BLD-07 | kerosene +1 |
| `tech.ia.helicopter_armor` | Iron Ascendancy | IA-AIR-03/06 | Research | $700 | — | IA-BLD-08 | −8 % speed |
| `tech.ia.heavy_bomb_load` | Iron Ascendancy | IA-AIR-05 | Research | $900 | — | IA-BLD-07 | −10 % speed |
| `tech.ia.quiet_propulsion` | Iron Ascendancy | IA-SEA-04 | Research | $900 | — | IA-BLD-14 | −10 % speed |
| `tech.mp.modular_commercial_equipment` | Meridian Pact | MP-ARM-01/02/03/04/07/09, MP-SUP-01, MP-AIR-03, MP-SEA-02/04 | Faction tech | per research doc | — | — | — |
| `tech.mp.contract_arbitration` | Meridian Pact | MP-INF-02/11, MP-AIR-02, MP-SUP-07, MP-SEA-03 | Upgrade | $500 | — | MP-DEF-09 | Wage +$5 trade-off |
| `tech.mp.regional_drill` | Meridian Pact | MP-INF-01 | Upgrade | $500 | — | MP-BLD-03 | — |
| `tech.mp.tandem_rockets` | Meridian Pact | MP-INF-03 | Upgrade | $600 | — | MP-BLD-03 | Magazine 4 → 3 |
| `tech.mp.mountain_observers` | Meridian Pact | MP-INF-04 | Upgrade | $400 | — | MP-BLD-03 | — |
| `tech.mp.field_engineering` | Meridian Pact | MP-INF-05 | Upgrade | $500 | — | MP-BLD-03 | Restore cost +$50 |
| `tech.mp.field_demolition_kits` | Meridian Pact | MP-INF-08/09, MP-ARM-11 | Upgrade | $600 | — | MP-DEF-07 | −10 % speed when loaded; carrier death blast |
| `tech.mp.mountain_warfare_training` | Meridian Pact | MP-INF-10 | Upgrade | $700 | — | MP-BLD-03 | +$40 unit price |
| `tech.mp.gun_shields` | Meridian Pact | MP-ARM-04 | Upgrade | $400 | — | MP-BLD-04 | −20 cm/s |
| `tech.mp.workshop_standards` | Meridian Pact | MP-ARM-05/12/13, MP-SUP-08 | Upgrade | $800 | — | MP-BLD-05 | +$50 on MP-ARM-05 |
| `tech.mp.add_on_armor` | Meridian Pact | MP-ARM-06 | Upgrade | $900 | — | MP-BLD-05 | −15 cm/s, +1 petroleum |
| `tech.mp.rocket_rack_reloads` | Meridian Pact | MP-ARM-08 | Upgrade | $500 | — | MP-BLD-04 | Larger death blast |
| `tech.mp.chemical_smoke_mix` | Meridian Pact | MP-ARM-10 | Upgrade | $400 | — | MP-BLD-05 | +25 % INCENDIARY damage taken |

## 7. Combined-arms reference compositions (P)

These are AI and tutorial reference groups, not rules. Each shows the faction's intended answer to the counter web in §2.

| Faction | Opening (first 3–4 minutes) | Mid-game core group | Late or signature group | Main vulnerability of the group |
| --- | --- | --- | --- | --- |
| Atlantic Concord | AC-SUP-01 scout plane, 2 × AC-INF-01, AC-INF-03 | 4 × AC-INF-01, AC-INF-03, AC-ARM-02, 2 × AC-ARM-04, AC-ARM-09 | AC-ARM-08 precision rockets under AC-AIR-07 observation, AC-AIR-04 helicopters, AC-ARM-10 EW cover | Losses are expensive; airfield and radar strikes |
| Red Directorate | RD-ARM-01 motorcycle, 3 × RD-INF-01, RD-INF-04 | 2 platoons of RD-INF-01 with RD-INF-10, RD-INF-03, RD-ARM-03, 3 × RD-ARM-04, RD-ARM-06 | RD-ARM-07 area rockets and RD-ARM-08 guns under RD-ARM-13 SAM cover, rail-fed | Artillery and flame against massed infantry; rail and depot interdiction |
| Iron Ascendancy | IA-SUP-01 scout plane, 2 × IA-INF-01, IA-INF-04 | IA-INF-02 assault squads, IA-INF-05 flame, IA-ARM-02, 2 × IA-ARM-04, IA-ARM-08 | IA-ARM-05 heavy tanks with IA-ARM-10 saturation rockets; IA-INF-11 Intimidation behind the front | No mobile SAM; Unaligned hostility; costly losses |
| Meridian Pact | MP-ARM-01 scout trucks, MP-INF-08 saboteurs, 2 × MP-INF-01 | MP-ARM-02/04 raiders, MP-INF-03 AT ambush, MP-ARM-07 recoilless, MP-ARM-09 quad AA | Market or captured heavy armor (MP-ARM-13), hired air (MP-AIR-06), MP-INF-10 ambush terrain play | Light armor; contract expiry; no bombers or propaganda |
| The Unaligned | R0: UN-INF-03/04, UN-ARM-01 | R3: UN-INF-01/02/06, UN-ARM-04, UN-ARM-13 at installations | R4: UN-ARM-06, UN-ARM-09, UN-AIR-02/04, UN-SEA-02 | Slow escalation; finite mobilization budget |

## 8. Implementation reference (RTS systems)

The unit specifications plug into existing systems; none introduces a parallel system. The [framework §18](UNIT_SPECIFICATION_FRAMEWORK.md) holds the field-to-data mapping, runtime placement, validators, modifier order and proposed acceptance IDs.

| Matrix concept | Runtime owner (existing doc) | Unit-spec contract |
| --- | --- | --- |
| Orders, stances, attack-move, guard, patrol | [Command Simulation Contract](RTS_COMMAND_SIMULATION_CONTRACT.md), [Selection and Group Orders](RTS_SELECTION_AND_GROUP_ORDERS.md) | Framework §13; abilities issue orders through the same order queue and typed `OrderRejection` |
| Control groups and mixed selections | [Control Group Input Research](RTS_CONTROL_GROUP_INPUT_RESEARCH.md) | Framework §13.4; command card shows the intersection of abilities |
| Formations, Discipline and resolve | [Unit Formations and Discipline](RTS_UNIT_FORMATIONS_AND_DISCIPLINE.md) | Framework §7; one resolve meter (E) |
| Unit and opponent AI | [AI Architecture](RTS_AI_ARCHITECTURE.md), [Opponent AI](RTS_OPPONENT_AI.md) | Framework §14 profiles; AI obeys the same eligibility and fog rules |
| Production, prerequisites, research | [Production and Technology](RTS_PRODUCTION_AND_TECHNOLOGY.md), [Technological Research](RTS_TECHNOLOGICAL_RESEARCH.md) | Framework §11–12; §6 above |
| Economy, food, fuel, power | [Operational Economy and Power](RTS_OPERATIONAL_ECONOMY_AND_POWER.md), [Resource Economy](RESOURCE_ECONOMY.md) | Framework §4.5 |
| Visibility, detection, intelligence | [Visibility and Intelligence](RTS_VISIBILITY_AND_INTELLIGENCE.md) | Framework §9 |
| Multiplayer authority | [Multiplayer Architecture](RTS_MULTIPLAYER_ARCHITECTURE.md) | Framework §15 (MPA classes) |
| Commanders | [Commander Talent System](COMMANDER_TALENT_SYSTEM.md) | Framework §17; talents never apply to ordinary units |
