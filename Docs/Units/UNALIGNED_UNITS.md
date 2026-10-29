# POLITIKA — The Unaligned Unit Specifications

**Status:** PROPOSED unit gameplay specification (29 October 2026). No unit in this file is implemented. Numbers are Provisional Baseline (PB) values.
**Authority:** names and IDs come from the [Five-Faction Rosters](../POLITIKA_FIVE_FACTION_ROSTERS.md); shared rules from the [Unit Specification Framework](../UNIT_SPECIFICATION_FRAMEWORK.md); faction canon from the [faction description](../../Factions/TheUnaligned/description.md) and [Unaligned Counterintelligence](../UNALIGNED_COUNTERINTELLIGENCE.md). Owner documents win on conflict.
**Evidence labels:** I / E / E-R / P / U as in framework §1.1. A field without a label is P. Cross-faction views: [Unit Balance and Ability Matrices](../UNIT_BALANCE_AND_ABILITY_MATRICES.md). Coverage: [Unit Specification Coverage Report](../Reports/UNIT_SPECIFICATION_COVERAGE_20261029.md).
**Siblings:** [Atlantic Concord](ATLANTIC_CONCORD_UNITS.md) · [Red Directorate](RED_DIRECTORATE_UNITS.md) · [Iron Ascendancy](IRON_ASCENDANCY_UNITS.md) · [Meridian Pact](MERIDIAN_PACT_UNITS.md).

**Never playable (E).** Every unit in this file is an NPC or scenario entity controlled by the Unaligned institutional AI and the world simulation. No unit has a player price, a player production button or a player command card. Where other files state USD, this file states a **mobilization weight** (framework §4.4, §11.6) and a **response tier** (framework §14.3).

## 1. Faction doctrine and faction-wide unit rules

The Unaligned is the sovereign administrative authority of the fictional 1980s Yugoslav federation, not a fifth conventional superpower (E). Its armed forces are the JNA and the Territorial Defence under the doctrine of Total People's Defence: conventional forces combined with territorial mobilization and decentralized resistance (E). It does not pursue conquest; it administers territory, protects infrastructure and answers violations of sovereignty through a **graduated response** from warnings and inspections to police intervention, mobilization and direct JNA engagement (E). It has the **strongest counterintelligence** in the game, including NPC-only false intelligence and false-flag attribution (E), and its own NPC-operated propaganda and Population Morale (E).

| Rule | Unit-level implementation | Label |
| --- | --- | --- |
| NPC-only | No unit is recruitable or commandable by a player. Units appear through the institutional AI's mobilization budget, scenario scripts or ambient schedules (framework §11.6). Cooperation agreements may change Unaligned **behavior** toward a participant, never its ownership | E |
| Mobilization weight | Each unit has a USD-equivalent weight that the institutional AI spends from a scenario-authored budget that refills over time and scales with Unaligned Population Morale. The AI never receives player money or forged resources (E) | E (rule), P (values) |
| Graduated response | Units are released only at or above their response tier (R0 to R4, framework §14.3). Hostility is **per participant**; the Unaligned never becomes a fifth playable side and never vetoes a lawful victory (E) | E (rule), P (tiers) |
| Ascendancy hostility | Identified Ascendancy forces carry a standing hostility modifier: response tier +1 for incidents attributed to them (P implementing E) | E, P |
| Discipline | Ordinary group support; **no very-massive-army bonus**; individuals and tiny detachments suffer the normal isolation penalty (E). Military units use Discipline; civilians use Morale; one resolve meter underneath (E) | E |
| Intimidation and Psy-Ops eligibility | Neutral or non-hostile Unaligned personnel are **not** eligible for Ascendancy Intimidation; units become eligible only while hostile to that participant under authored rules, and conversion still needs a full break (E). Civilians rampage under Concord Psy-Ops only where an authored attack action exists (E) | E |
| Counterintelligence | Strongest `DETECT.SPY` sources in the game (UN-INF-04/10/11, UN-ARM-01/03); intercept, fabricate and misattribute are NPC capabilities with coverage, cost, cooldown and expiry, never omniscience (E) | E (rule), P (values) |
| False intelligence | Only UN-INF-12 operations plant false reports. Reports are information objects; they never change units, buildings, resources, nuclear state or victory (E) | E |
| Propaganda | NPC-only media apparatus through radio, newspapers and television (UN-INF-13, UN-CIV-09, UN-SEC-06/07/08). Media buildings are not freely commandeered by players (E) | E |
| Nuclear | Non-nuclear (E). No unit carries nuclear weapons or processes nuclear materials | E |
| Jurisdiction | Civilian vehicles, structures and personnel are never automatically valid targets; attacking them is an incident that raises the response tier (E) | E |
| Food and fuel | Unaligned units draw on the institutional budget, not a participant ledger. Food and fuel are abstracted into the mobilization weight (P). Civilian traffic consumes nothing | P |
| Visual language | JNA olive drab, Territorial Defence mixed reserve kit, blue police uniforms, red-star roundels, federal coat of arms; municipal and enterprise liveries on civilian vehicles; Adriatic ports, mountain passes, railway hubs | P |

**C&C design lessons applied.** The Unaligned borrows the neutral-faction and civilian-population lessons of the series: Tiberian Dawn and Red Alert civilians and neutral structures as map texture and objectives, Generals' civilian traffic and capturable tech buildings as economic hooks, and Renegade-style police and guard archetypes. It deliberately avoids two classic failures: civilians as free kill targets with no consequence (here every civilian casualty is an incident with escalation) and neutral armies that ignore who attacked them (here hostility is per participant and attribution can be investigated or forged, E).

## 2. Core roster selection

For an NPC faction, **Core** means "present in the first playable slice's institutional AI and ambient world": the minimum set that makes graduated response and a living economy visible.

Core (17 units):

| ID | Role covered | Reason |
| --- | --- | --- |
| UN-INF-01 | Line infantry (JNA) | R3 baseline infantry |
| UN-INF-02 | Territorial mobilization | Total People's Defence signature |
| UN-INF-03 | Border control | First tripwire on every border map |
| UN-INF-04 | Police / `DETECT.SPY` | R1–R2 response and investigation |
| UN-INF-06 | Anti-armor infantry | Defends JNA sites at R3 |
| UN-INF-07 | Engineer / support | Repairs public infrastructure |
| UN-INF-11 | Faction signature: counterintelligence | Strongest counterintelligence (E) |
| UN-ARM-01 | Scout / light vehicle | Police patrol car |
| UN-ARM-02 | Military scout | JNA recon jeep |
| UN-ARM-04 | Transport (APC) | JNA mechanized infantry |
| UN-ARM-06 | Main battle tank | R4 armored formation |
| UN-ARM-08 | Mobile air defense | Sovereign airspace at low level |
| UN-ARM-09 | Artillery | R4 rocket artillery |
| UN-CIV-01 | Ambient traffic | Living world baseline |
| UN-CIV-03 | Civilian economy | Freight flows and targets |
| UN-CIV-06 | Civil emergency | Incident aftermath and morale |
| UN-AIR-01 | Air sovereignty | Answers unauthorized aircraft |

Buildings (UN-BLD, UN-SEC) are not unit entries; they appear only as spawn sites, prerequisites and targets.

**Extended:** UN-INF-05, UN-INF-08, UN-INF-09, UN-INF-10, UN-ARM-03, UN-ARM-05, UN-ARM-07, UN-ARM-10 to UN-ARM-15, UN-CIV-02, UN-CIV-05, UN-CIV-07 to UN-CIV-10, UN-AIR-02 to UN-AIR-05.
**Conditional:** UN-CIV-04 (rail maps), UN-CIV-11 and UN-CIV-12 (water and river maps), UN-AIR-08, UN-SEA-01 to UN-SEA-08 (authored water layer; UN-SEA-06 on river maps).
**Scenario:** UN-INF-12 (NPC director operations, not a battlefield spawn by default), UN-INF-13 (campaign and media scenarios), UN-INF-14 (scenario-protected), UN-AIR-06 and UN-AIR-07 (timetabled or scripted flights).

## 3. Federal, Territorial Defence and security personnel

Infantry entries use `TPL-NPC-SECURITY` unless stated, `ai.unit.npc_security` with a response tier, and MPA-NPC. Commands are **Unaligned institutional authority only** (E). Infantry weapons reuse the implemented magazine, reload and grenade behavior where noted (I, generic behavior only).

### UN-INF-01 — Yugoslav Army Rifle Section

| Field | Specification |
| --- | --- |
| **Identity** | `unit.un.inf_01` · The Unaligned · Infantry / JNA line section · `TPL-NPC-SECURITY` (as `TPL-INF-LINE`) · **Core** · NPC-only |
| **Role** | **Primary:** JNA line infantry for autonomous federal security (E-R). **Secondary:** garrison of JNA sites; escort for engineers and convoys at R3+. **Doctrine fit:** conventional component of Total People's Defence (E). |
| **Inspiration and background** | A JNA rifle section in olive drab with assault rifles, a squad machine gun and a rifle grenadier. C&C lineage (E-R): Rifle Infantry (RA1); GDI Minigunner (TD); Rifleman Squad (TW). |
| **Visual identity** | Concept: none (asset requirement). Olive M59-style uniforms, steel helmets with a red star, assault rifles, one LMG. |
| **Statistics** | 5 × 55 HP = **275** (`armor.inf_light`) · FOOT 205 cm/s · F0 · sight 1200 cm · 1 slot · ExperienceValue 30 (weight/10) |
| **Commands** | Institutional authority only: Move, Attack, AttackMove, Guard, Patrol, Garrison, Withdraw · default stance Guard · formation role `frontline` |
| **Production** | Mobilization weight **300** · response tier **R3** · spawned at `UN-BLD-05` JNA Barracks Complex or a scenario mobilization point · 8 s mobilization step |
| **Veterancy** | Standard (framework §8); XP only from hostile participants' units |
| **AI** | `ai.unit.npc_security` with `ai.unit.line_infantry` weights. Deltas: engage only units of the participant whose hostility exceeds R3; never pursue beyond the incident jurisdiction |
| **Multiplayer** | **MPA-NPC** (hostility scores never replicated) |
| **Presentation** | ANIM-INF. Serbo-Croatian voice set with formal commands; icon of a red star over a rifle |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.un.inf_01.primary` assault rifles + LMG | SMALL_ARMS | 0–650 cm | 10 per member / 1.0 s (50 DPS at 5); magazine 30, reload 3 s (I values) | hitscan | 3 | GROUND, AIR_LOW | 100 % / 15 % / 2 % / 25 % rotary / 8 % |
| `ability.shared.grenade` (I) | HIGH_EXPLOSIVE | 650 cm (I) | 50 (I), 150 cm (I) | lobbed (I) | 8 | GROUND, STRUCTURE | 110 % / 45 % / 12 % / — / 100 % |

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.shared.grenade` | Active (I) | Lobbed HE grenade | Hostile garrison or cluster | free · 3 s reload (I) | Scatter | Arc preview (none for NPC) | Only on hostile participants |
| `ability.un.inf_01.cordon` | Active (P) | The section forms a cordon line; hostile infantry crossing within 300 cm of it are revealed to all Unaligned units for 10 s | Line ≤ 800 cm | free · 60 s · 90 s | Avoid the cordon; artillery | Barrier tape VFX | Use at R2–R3 around incident sites |

**Research and upgrades:** none player-facing. Scenario flag `un.mobilization_readiness` (P): +10 % HP for all JNA infantry in high-tension scenarios.

**Counters:** *Beats (soft):* light raids (MP-INF-08 teams, RD-ARM-01, IA-ARM-01) that trigger R3. *Beaten by (hard):* armor, artillery and flame of any playable faction once at war with the federation. *Situational:* its presence, not its power, is the deterrent.

**Status:** identity and NPC-only E-R/E; graduated response E; values and tiers P.

### UN-INF-02 — Territorial Defence Reserve

| Field | Specification |
| --- | --- |
| **Identity** | `unit.un.inf_02` · The Unaligned · Infantry / reserve platoon · `TPL-NPC-SECURITY` (as `TPL-INF-LINE`) · **Core** · NPC-only |
| **Role** | **Primary:** locally mobilized defensive platoon; autonomous, no player barracks (E-R). **Secondary:** holding towns and depots while the JNA arrives. **Doctrine fit:** territorial mobilization and decentralized resistance (E). |
| **Inspiration and background** | Factory workers and farmers called up with stored rifles under Territorial Defence plans. C&C lineage (E-R): Militant Squad (TW); Rebel (Gen); Devout (TT). |
| **Visual identity** | Concept: none (asset requirement). Mixed olive and civilian clothes, TO armbands, older rifles. |
| **Statistics** | 6 × 45 HP = **270** (`armor.inf_light`) · FOOT 195 cm/s · F0 · sight 1100 cm · 1 slot · **maximum resolve −10** · ExperienceValue 18 |
| **Commands** | Institutional authority only · default stance Guard · formation role `frontline` |
| **Production** | Mobilization weight **180** · response tier **R3** · spawned at `UN-BLD-06` Territorial Defence Depot or any municipal building with a TO plan · 5 s step |
| **Veterancy** | Standard |
| **AI** | `ai.unit.npc_security` (line infantry weights). Deltas: garrison buildings within the home municipality; never leave it |
| **Multiplayer** | MPA-NPC |
| **Presentation** | ANIM-INF (civilian-walk variant). Icon of a TO shield |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.un.inf_02.primary` stored rifles | SMALL_ARMS | 0–600 cm | 8 per member / 1.2 s | hitscan | 2 | GROUND | 100 % / 15 % / 2 % / — / 8 % |

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.un.inf_02.home_municipality` | Conditional (P) | Inside its home municipality: +15 maximum resolve (cancelling the reserve penalty) and +10 % damage when garrisoned | Automatic | — | Lure it out; flame on garrisons | Municipality banner | Stay home |
| `ability.un.inf_02.mass_mobilization` | Automated (P; E: Total People's Defence) | At R3, each TO depot releases one platoon per 60 s up to 4, scaled by Unaligned Population Morale (≥ 60: up to 6) | Institutional | weight per platoon | Lower Unaligned morale; avoid R3 | Siren audio | Institutional AI only |

**Research and upgrades:** none.

**Counters:** *Beats (soft):* small foreign detachments in towns. *Beaten by (hard):* HMG (RD-INF-03), flame (IA-INF-05, RD-ARM-10), armor, artillery. *Situational:* makes occupying towns expensive and slow; it is the main reason to avoid R3.

**Status:** identity E-R; Total People's Defence E; values P.

### UN-INF-03 — Federal Border Guards

| Field | Specification |
| --- | --- |
| **Identity** | `unit.un.inf_03` · The Unaligned · Infantry / border guards · `TPL-NPC-SECURITY` · **Core** · NPC-only |
| **Role** | **Primary:** control crossing points and observe international movements (E-R). **Secondary:** reporting incidents that raise the response tier. **Doctrine fit:** border authorities and customs (E). |
| **Inspiration and background** | Federal border guards with rifles, binoculars and a dog at a barrier. C&C lineage (E-R): GDI Officer (Ren); Peacekeeper (RA3); Rifle Infantry (RA1). |
| **Visual identity** | Concept: none (asset requirement). Green-grey uniforms, peaked caps, binoculars, a guard dog. |
| **Statistics** | 3 × 60 HP = **180** (`armor.inf_light`) · FOOT 200 cm/s · sight **1500 cm** · `DETECT.CONCEALED` 900 cm · `DETECT.SPY` **500 cm** · ExperienceValue 22 |
| **Commands** | Institutional authority only · default stance Guard · formation role `checkpoint` |
| **Production** | Mobilization weight **220** · response tier **R0** (permanent at crossings) · spawned at `UN-SEC-02` Border Customs Station · 6 s step |
| **Veterancy** | Standard |
| **AI** | `ai.unit.npc_security`. Deltas: inspect units crossing the border line; report and withdraw rather than fight a superior force |
| **Multiplayer** | MPA-NPC (inspection results reach only the inspected participant) |
| **Presentation** | ANIM-INF + inspect gesture. Barrier-arm prop; icon of a barrier |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.un.inf_03.primary` rifles | SMALL_ARMS | 0–650 cm | 10 per member / 1.0 s | hitscan | 3 | GROUND | 100 % / 15 % / 2 % / — / 8 % |

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.un.inf_03.border_inspection` | Automated (P) | Units crossing a federal border within 600 cm are inspected for 4 s: disguised agents within `DETECT.SPY` range are revealed; military units without a crossing agreement create an **R1 incident** for their participant | Crossing units | free · 4 s · per unit | Cross elsewhere; agreements; kill the post (R2+ incident) | Inspection icon to the inspected participant | Always at crossings |
| `ability.un.inf_03.incident_report` | Automated (P) | Observed violations are reported to the institutional AI with the observed participant (attribution may be wrong if forged, E) | Observed hostile act | — | Kill observers before they report (the kill itself is an incident) | — | Report everything |

**Research and upgrades:** none.

**Counters:** *Beats (hard):* disguised agents crossing borders (AC-INF-11, RD-INF-11, IA-INF-09/14, MP-INF-07 *Trader Cover*). *Beaten by (hard):* any assault, at the price of escalation. *Situational:* the first tripwire of the graduated response.

**Status:** identity E-R; graduated response and attribution rules E; values P.

### UN-INF-04 — Federal Police Patrol

| Field | Specification |
| --- | --- |
| **Identity** | `unit.un.inf_04` · The Unaligned · Infantry / police patrol · `TPL-NPC-SECURITY` · **Core** · NPC-only |
| **Role** | **Primary:** town security, investigation and **graded force** (E-R). **Secondary:** arrest attempts on revealed agents; checkpoint staffing. **Doctrine fit:** police forces regulate access and respond to disturbances (E). |
| **Inspiration and background** | Two federal police officers on foot patrol with pistols and a submachine gun. C&C lineage (E-R): Peacekeeper (RA3); Attack Dog (RA1, screening role); GDI Soldier (Ren). |
| **Visual identity** | Concept: none (asset requirement). Blue-grey police uniforms, white belts, caps, radio. |
| **Statistics** | 2 × 60 HP = **120** (`armor.inf_light`) · FOOT 210 cm/s · sight 1200 cm · `DETECT.SPY` **600 cm** · ExperienceValue 15 |
| **Commands** | Institutional authority only · default stance Hold Fire below R2 · formation role `patrol` |
| **Production** | Mobilization weight **150** · response tier **R0** (patrol), **R2** (armed response) · spawned at `UN-SEC-01` Federal Police Headquarters or `UN-SEC-11` City Police Checkpoint · 5 s step |
| **Veterancy** | Standard |
| **AI** | `ai.unit.npc_security`. Deltas: R0–R1 warn and inspect; R2 arrest and use force only on the offending participant's infantry and soft vehicles |
| **Multiplayer** | MPA-NPC |
| **Presentation** | ANIM-INF + whistle and wave gestures. Siren audio at R2; icon of a police cap |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.un.inf_04.primary` pistols + SMG | SMALL_ARMS | 0–450 cm | 8 per member / 0.8 s | hitscan | 2 | GROUND | 100 % / 15 % / 2 % / — / 8 % |

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.un.inf_04.arrest` | Active (P) | 5 s channel within 150 cm on a **revealed** agent, saboteur or Pinned/Broken lone infantry of an R2+ participant: the target is removed (detained), with no XP to anyone and no Intimidation-style ownership transfer | Eligible revealed infantry | free · 5 s · 20 s | Escorts; killing the police (R3 escalation); not usable on squads that are Steady | Handcuff icon | Prioritize revealed agents |
| `ability.un.inf_04.inspection` | Active (P) | Checks a vehicle or unit within 300 cm for 4 s: reveals `falsified_manifest` (MP-SUP-06), disguise and decoys (MP-MKT-06) | Unit | free · 4 s · 15 s | Avoid patrols | Inspection icon | At R1+ on suspicious traffic |

**Research and upgrades:** none.

**Counters:** *Beats (hard):* agents, couriers and saboteurs (AC-INF-11, RD-INF-11/12, IA-INF-09/10/14, MP-INF-08/09). *Beaten by (hard):* any combat unit, at the price of escalation. *Situational:* the most common reason a covert plan fails in a town.

**Status:** identity and graded force E-R/E; arrest and inspection mechanics P.

### UN-INF-05 — Railway Protection Detachment

| Field | Specification |
| --- | --- |
| **Identity** | `unit.un.inf_05` · The Unaligned · Infantry / rail security · `TPL-NPC-SECURITY` · **Extended** · NPC-only |
| **Role** | **Primary:** secure civilian transport and key bridges (E-R). **Secondary:** escort UN-CIV-04 trains. **Doctrine fit:** protection of transport hubs (E). |
| **Inspiration and background** | Railway security troops guarding a bridge and a freight yard. C&C lineage (E-R): GI (RA2); Rifleman Squad (TW); GDI Officer (Ren, guard role). |
| **Visual identity** | Concept: none (asset requirement). Dark-blue railway security uniforms, rifles, lamp. |
| **Statistics** | 4 × 60 HP = **240** (`armor.inf_light`) · FOOT 200 cm/s · sight 1300 cm · `DETECT.CONCEALED` 600 cm · ExperienceValue 24 |
| **Commands** | Institutional authority only · default stance Guard · formation role `rear_guard` |
| **Production** | Mobilization weight **240** · response tier **R1** · spawned at `UN-BLD-12` Railway Freight Terminal · 7 s step |
| **Veterancy** | Standard |
| **AI** | `ai.unit.npc_security`. Deltas: hold within 1000 cm of an assigned bridge or yard; ride trains as escorts |
| **Multiplayer** | MPA-NPC |
| **Presentation** | ANIM-INF. Icon of a rail and shield |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.un.inf_05.primary` rifles | SMALL_ARMS | 0–650 cm | 10 per member / 1.0 s | hitscan | 3 | GROUND | 100 % / 15 % / 2 % / — / 8 % |

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.un.inf_05.bridge_watch` | Passive (P) | Charges planted on a bridge or rail asset within 800 cm are revealed to the detachment after 3 s | Automatic | — | Kill the detachment first (incident) | Alarm bell | Investigate revealed charges |

**Research and upgrades:** none.

**Counters:** *Beats (hard):* saboteurs on bridges (MP-INF-08/09). *Beaten by (hard):* any assault. *Situational:* makes rail sabotage a loud act.

**Status:** identity E-R; values P.

### UN-INF-06 — JNA Rocket Infantry

| Field | Specification |
| --- | --- |
| **Identity** | `unit.un.inf_06` · The Unaligned · Infantry / anti-armor team · `TPL-NPC-SECURITY` (as `TPL-INF-AT`) · **Core** · NPC-only |
| **Role** | **Primary:** anti-armor defense of military facilities (E-R). **Secondary:** ambushing armor on federal roads at R3+. **Doctrine fit:** JNA defense of strategic installations (E). |
| **Inspiration and background** | A JNA team with recoilless and shoulder-fired anti-tank rockets. C&C lineage (E-R): Rocket Soldier (TD); Missile Squad (TW); Tank Hunter (Gen). |
| **Visual identity** | Concept: none (asset requirement). Olive uniforms, launcher tubes, spare rockets on backs. |
| **Statistics** | 3 × 60 HP = **180** (`armor.inf_light`) · FOOT 200 cm/s · sight 1200 cm · ExperienceValue 40 |
| **Commands** | Institutional authority only · default stance Guard · formation role `anti_armor` |
| **Production** | Mobilization weight **400** · response tier **R3** · `UN-BLD-05` · 9 s step |
| **Veterancy** | Standard |
| **AI** | `ai.unit.npc_security` with `ai.unit.anti_armor_infantry` weights |
| **Multiplayer** | MPA-NPC |
| **Presentation** | ANIM-INF + launcher. Icon of a rocket tube with a red star |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.un.inf_06.primary` AT rockets | SHAPED_CHARGE | 0–800 cm | 110 / 4.0 s; magazine 4, resupply at UN-BLD-05 or UN-ARM-05 | projectile | 3 | GROUND | 25 % / 100 % / 90 % / — / 70 % |

**Abilities:** none unit-specific.

**Research and upgrades:** none.

**Counters:** *Beats (hard):* vehicles that violate JNA sites (AC-ARM-04, RD-ARM-04, IA-ARM-03/04, MP-ARM-06). *Beaten by (hard):* infantry, MGs, artillery. *Situational:* why attacking a JNA barracks with tanks alone is costly.

**Status:** identity E-R; values P.

### UN-INF-07 — Federal Engineers

| Field | Specification |
| --- | --- |
| **Identity** | `unit.un.inf_07` · The Unaligned · Infantry / engineers · `TPL-NPC-SECURITY` (as `TPL-INF-SUPPORT`) · **Core** · NPC-only |
| **Role** | **Primary:** repair public infrastructure, bridges, hospitals and military sites (E-R). **Secondary:** clearing incident debris; removing discovered charges. **Doctrine fit:** engineering units and infrastructure continuity (E). |
| **Inspiration and background** | Federal engineering troops with tool trucks and hard hats. C&C lineage (E-R): Engineer (TD); Combat Engineer (RA3); Engineer (TT). |
| **Visual identity** | Concept: none (asset requirement). Olive overalls, helmets, toolboxes. |
| **Statistics** | 3 × 55 HP = **165** (`armor.inf_light`) · FOOT 200 cm/s · sight 1000 cm · no ranks (unarmed support) |
| **Commands** | Institutional authority only · default stance Hold Fire |
| **Production** | Mobilization weight **250** · response tier **R0** · `UN-BLD-05` or `UN-BLD-03` Municipal Assembly · 8 s step |
| **Veterancy** | none |
| **AI** | `ai.unit.npc_security` with `ai.unit.support_infantry` behavior. Deltas: repair the damaged public structure with the highest morale value first (hospitals, power) |
| **Multiplayer** | MPA-NPC |
| **Presentation** | ANIM-INF + work loop. Icon of a wrench with a red star |

**Weapons:** none (unarmed)

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.un.inf_07.public_repair` | Active (P) | Repairs Unaligned structures at 10 HP/s and restores a destroyed bridge span in 30 s; costs institutional budget, not a participant ledger | Unaligned structure ≤ 300 cm | budget · channel | Escort removal; incidents nearby pause it | Scaffolding | Repair after incidents |
| `ability.un.inf_07.remove_charge` | Active (P) | Removes a revealed charge in 4 s (as *Investigate*) | Revealed charge | free · 4 s | Kill them (incident) | Defuse icon | Always on revealed charges |

**Research and upgrades:** none.

**Counters:** *Beaten by (hard):* any armed unit. *Situational:* undoes sabotage over time, so a raid on public infrastructure buys only a window.

**Status:** identity E-R; values P.

### UN-INF-08 — JNA Field Medical Unit

| Field | Specification |
| --- | --- |
| **Identity** | `unit.un.inf_08` · The Unaligned · Infantry / military medics · `TPL-NPC-SECURITY` (as `TPL-INF-SUPPORT`) · **Extended** · NPC-only |
| **Role** | **Primary:** military medical support for NPC mobilization (E-R). **Secondary:** civilian casualty response alongside UN-CIV-06. **Doctrine fit:** sustaining territorial defense (E). |
| **Inspiration and background** | A JNA medical team with stretchers and a red-cross brassard. C&C lineage (E-R): Medic (RA1); Field Medic (TS); Ambulance (Gen). |
| **Visual identity** | Concept: none (asset requirement). Olive uniforms, red-cross armbands, stretcher. |
| **Statistics** | 2 × 55 HP = **110** (`armor.inf_light`) · FOOT 210 cm/s · sight 1000 cm · no ranks |
| **Commands** | Institutional authority only · default stance Hold Fire |
| **Production** | Mobilization weight **200** · response tier **R3** · `UN-BLD-05` or `UN-BLD-15` State Hospital · 8 s step |
| **Veterancy** | none |
| **AI** | `ai.unit.support_infantry` behavior; heal Unaligned units only |
| **Multiplayer** | MPA-NPC |
| **Presentation** | ANIM-INF + heal loop. Icon of a red cross |

**Weapons:** none (unarmed)

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.un.inf_08.heal` | Active (P; framework §10.3) | 6 HP/s to one Unaligned squad within 300 cm | Friendly Unaligned squad | free · channel | Kill the medics (incident; attacking marked medics is an **R+1** escalation, P) | Heal VFX | Lowest HP ratio first |

**Research and upgrades:** none.

**Counters:** *Beaten by (hard):* any armed unit. *Situational:* prolongs R3 engagements.

**Status:** identity E-R; values and the medic-escalation rule P.

### UN-INF-09 — Mountain Reconnaissance Patrol

| Field | Specification |
| --- | --- |
| **Identity** | `unit.un.inf_09` · The Unaligned · Infantry / mountain recon · `TPL-NPC-SECURITY` (as `TPL-INF-SPEC`) · **Extended** · NPC-only |
| **Role** | **Primary:** local scout infantry with terrain knowledge (E-R). **Secondary:** finding concealed foreign units in passes. **Doctrine fit:** territorial knowledge (E). |
| **Inspiration and background** | Mountain troops on skis or foot with binoculars and scoped rifles. C&C lineage (E-R): Sniper Team (TW, scouting); Pathfinder (Gen); Shadow Team (TW, reconnaissance role). |
| **Visual identity** | Concept: none (asset requirement). Mountain caps, windproof smocks, binoculars. |
| **Statistics** | 2 × 60 HP = **120** (`armor.inf_light`) · FOOT 215 cm/s (rough terrain ×1.0) · sight **1700 cm** · `DETECT.CONCEALED` **1100 cm** · ExperienceValue 30 |
| **Commands** | Institutional authority only · default stance Hold Fire while Concealed |
| **Production** | Mobilization weight **300** · response tier **R1** · `UN-BLD-05` or scenario mountain post · 10 s step |
| **Veterancy** | Standard |
| **AI** | `ai.unit.npc_security` with `ai.unit.specialist_infantry` behavior. Deltas: patrol passes; report, do not engage, below R3 |
| **Multiplayer** | MPA-NPC + MPA-COVERT (concealment) |
| **Presentation** | ANIM-INF + prone. Icon of a peak and binoculars |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.un.inf_09.primary` scoped rifle | PRECISION | 0–1000 cm | 45 / 3.0 s | hitscan | 6 on kill | GROUND | 160 % / 2 % / 0 % / — / 0 % |

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.shared.conceal` | Conditional (P) | Concealed after 3 s stationary | Self | free | Detectors; firing | — | Observe concealed |

**Research and upgrades:** none.

**Counters:** *Beats (hard):* concealed infantry in mountains (MP-INF-10, AC-INF-08, IA-INF-08). *Beaten by (hard):* any vehicle, detectors. *Situational:* why ambushes on federal mountain roads are rarely secret.

**Status:** identity E-R; values P.

### UN-INF-10 — State Security Investigator

| Field | Specification |
| --- | --- |
| **Identity** | `unit.un.inf_10` · The Unaligned · Infantry / investigator · `TPL-INF-COVERT` (NPC) · **Extended** · NPC-only |
| **Role** | **Primary:** investigates facility intrusions, fraud and alleged foreign plots (E-R). **Secondary:** attribution of sabotage, diversion and contamination incidents. **Doctrine fit:** internal security and counterintelligence (E). |
| **Inspiration and background** | A plain-clothes state security officer with a notebook and a driver. C&C lineage (E-R): Spy (RA1); Detention Camp (Gen, information role); Black Lotus (Gen, spy interaction). |
| **Visual identity** | Concept: none (asset requirement). Grey suit, raincoat, briefcase. |
| **Statistics** | 1 × 110 HP = **110** (`armor.inf_light`) · FOOT 210 cm/s · sight 1200 cm · `DETECT.SPY` **800 cm** · no ranks |
| **Commands** | Institutional authority only · default stance Hold Fire |
| **Production** | Mobilization weight **500** · response tier **R2** · `UN-SEC-03` State Security Directorate · 14 s step |
| **Veterancy** | none |
| **AI** | `ai.unit.covert_agent` (NPC). Deltas: travel to incident sites; avoid combat |
| **Multiplayer** | MPA-NPC (investigation progress and findings never replicated except as the outcome to affected participants) |
| **Presentation** | ANIM-INF (civilian walk). Icon of a magnifier |

**Weapons:** none (unarmed; armed escort optional)

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.un.inf_10.investigate_incident` | Active (P; E: attribution) | 20 s at an incident site: produces an attribution finding (true participant, or a false-flag attribution if one was planted and not yet contradicted, E). A true finding raises that participant's response tier | Incident site ≤ 600 cm | budget · 20 s · 60 s | Kill the investigator (a new incident); leave no evidence (no detection during the act) | Finding notice to the attributed participant | Investigate every R2+ incident |

**Research and upgrades:** none.

**Counters:** *Beats (hard):* covert operations after the fact. *Beaten by (hard):* any armed unit. *Situational:* converts sabotage into political cost.

**Status:** identity E-R; attribution and forgery rules E; values P.

### UN-INF-11 — Federal Counterintelligence Officer

| Field | Specification |
| --- | --- |
| **Identity** | `unit.un.inf_11` · The Unaligned · Infantry / counterintelligence · `TPL-INF-COVERT` (NPC) · **Core** (faction signature) · NPC-only |
| **Role** | **Primary:** **strongest counterintelligence**: intercepts, corroborates and blocks infiltration (E-R; E). **Secondary:** protecting UN-BLD-14 laboratories and UN-SEC sites. **Doctrine fit:** the Unaligned's strategic counterintelligence (E). |
| **Inspiration and background** | A counterintelligence officer with a radio-intercept case and two assistants. C&C lineage (E-R): Detention Camp (Gen); Spy (RA3); Mobile Sensor Array (TS). |
| **Visual identity** | Concept: none (asset requirement). Dark suit, headphones on a case, one assistant with a camera. |
| **Statistics** | 1 × 130 HP = **130** (`armor.inf_light`) · FOOT 205 cm/s · sight 1300 cm · `DETECT.SPY` **1100 cm** (the largest in the game) · `DETECT.EMITTER` 2000 cm · no ranks |
| **Commands** | Institutional authority only · default stance Hold Fire |
| **Production** | Mobilization weight **700** · response tier **R0** at protected sites, **R2** mobile · `UN-SEC-03` · 18 s step · per-map cap 3 (P) |
| **Veterancy** | none |
| **AI** | `ai.unit.covert_agent` (NPC). Deltas: stand watch at the highest-value Unaligned site; move to counter observed foreign agents |
| **Multiplayer** | MPA-NPC + MPA-SENSOR (intercepts never replicated as raw data) |
| **Presentation** | ANIM-INF (civilian walk), headphones idle. Icon of an ear over a star |

**Weapons:** none (unarmed)

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.un.inf_11.block_infiltration` | Passive (P) | Infiltration channels (intelligence theft, diversion, contamination, spy charges) on Unaligned sites within 1100 cm take **+50 % time** and fail if the agent is revealed during the channel | Automatic | — | Kill or lure the officer (incident); act outside range | Site status (none to the agent until revealed) | Always at the most valuable site |
| `ability.un.inf_11.intercept` | Automated (P; E: intercept capability) | Foreign `DETECT.EMITTER` contacts and covert actions within coverage become intelligence for the institutional AI with a source and confidence; coverage is bounded, never omniscient (E) | Coverage 2000 cm | budget | Radio silence; act outside coverage | — | Feeds UN-INF-12 and response tiers |

**Research and upgrades:** none.

**Counters:** *Beats (hard):* every faction's spies and saboteurs (AC-INF-11, RD-INF-11/12, IA-INF-09/10/11/14, MP-INF-07/08/09). *Beaten by (hard):* any armed unit, at a heavy escalation cost. *Situational:* foreign espionage against the federation must deal with this unit first.

**Status:** identity and "strongest counterintelligence, not omniscience" E-R/E; values P.

### UN-INF-12 — Deception Bureau Liaison

| Field | Specification |
| --- | --- |
| **Identity** | `unit.un.inf_12` · The Unaligned · Infantry / deception liaison · `TPL-INF-COVERT` (NPC) · **Scenario** (director operations) · NPC-only |
| **Role** | **Primary:** NPC-only **planted false intelligence and false flags**; never a purchasable player spy (E-R). **Secondary:** balance-preservation diversions when a participant faces imminent elimination (E). **Doctrine fit:** fabricate and misattribute capabilities (E). |
| **Inspiration and background** | A quiet liaison officer who moves forged documents through couriers and radio channels. C&C lineage (E-R): Decoy Army (TW); Fake Structures (RA1); Radar Jamming Missile (TW). |
| **Visual identity** | Concept: none (asset requirement). Civilian clothes, attaché case; usually unseen. |
| **Statistics** | 1 × 100 HP = **100** (`armor.inf_light`) · FOOT 210 cm/s · sight 1200 cm · usually **not spawned**: operations run from `UN-SEC-04` Federal Deception Cell; the unit appears only when an operation needs a physical courier · no ranks |
| **Commands** | NPC director only |
| **Production** | Mobilization weight **900** per physical courier run · `UN-SEC-04` · operation cost and cooldown from the director budget (E: bounded cost, cooldown, credibility) |
| **Veterancy** | none |
| **AI** | Unaligned director (not a unit profile). Physical courier uses `ai.unit.covert_agent` |
| **Multiplayer** | **MPA-NPC**: claim truth flags, origin and intended recipients are never replicated (E: false-report truth flags) |
| **Presentation** | ANIM-INF (civilian walk). No icon to players |

**Weapons:** none (unarmed)

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.un.inf_12.plant_false_report` | Automated (E rule; P values) | Delivers an `IntelClaim` (troop move, facility, nuclear construction, attack) to chosen recipients through an intercept or source channel. It never changes ground truth: no unit, building, resource, nuclear or victory state is created (E) | Recipients | director budget · claim expiry 120–300 s · cooldown per target 240 s · credibility drops with overuse (E) | Physical reconnaissance, MP-AIR-05 *Verify Report*, independent sources, contradictions (E) | Report appears in the recipient's intel queue | Balance pressure or escalation only |
| `ability.un.inf_12.misattribute` | Automated (E rule; P values) | Attaches a false attribution to a real or authored incident; UN-INF-10 findings follow it until contradicted (E) | Incident | director budget · cooldown 300 s | Counter-investigation evidence | — | Rare; credibility-limited |
| Courier interception (P) | Conditional | A physical courier can be killed or detained by any participant's `DETECT.SPY` unit; doing so exposes the operation's origin to that participant only | Courier | — | `DETECT.SPY` | Exposure notice | — |

**Research and upgrades:** none.

**Counters:** *Beaten by (hard):* verification and counter-evidence; `DETECT.SPY` on the courier. *Situational:* the only false-intelligence source in the game (E); its claims must remain investigable.

**Status:** identity, NPC-only, ground-truth immunity and counterplay E; budget, expiry, cooldowns and courier mechanics P.

### UN-INF-13 — Public Continuity Commissioner

| Field | Specification |
| --- | --- |
| **Identity** | `unit.un.inf_13` · The Unaligned · Infantry / media and continuity official · `TPL-INF-COMMAND` (NPC, non-combat) · **Scenario** · NPC-only |
| **Role** | **Primary:** coordinates official communications and civilian morale, NPC-only (E-R). **Secondary:** campaign and continuity-mystery scenes (see the [story outline](../STORY_OUTLINE.md)). **Doctrine fit:** the NPC-only propaganda apparatus (E). |
| **Inspiration and background** | A senior federal communications official with aides and a press photographer. C&C lineage (E-R): Propaganda Center (Gen); Voice of Kane (KW, influence only); Speaker Tower (Gen). |
| **Visual identity** | Concept: none (asset requirement). Dark suit, party pin, aides with folders and a camera. |
| **Statistics** | 1 × 150 HP = **150** (`armor.inf_light`) · FOOT 190 cm/s · sight 1200 cm · Morale (non-combat) |
| **Commands** | NPC director and scenario only |
| **Production** | Mobilization weight **600** · scenario spawn at `UN-SEC-06` Federation Radio House, `UN-SEC-07` Television Network Center or `UN-BLD-01` |
| **Veterancy** | none |
| **AI** | `ai.unit.officer` (NPC, non-combat). Deltas: stay inside protected sites |
| **Multiplayer** | MPA-NPC |
| **Presentation** | ANIM-INF + speech gestures. Icon of a microphone |

**Weapons:** none (unarmed)

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.un.inf_13.official_broadcast` | Automated (P; E: NPC propaganda) | While inside an operational UN-SEC-06/07/08: Unaligned Population Morale recovery +25 % for 120 s after an incident; can amplify a UN-INF-12 narrative where the counterintelligence rules allow (E) | Media site | director budget · 120 s · 300 s | Psy-Ops or sabotage on media buildings (incidents); lowered morale | World-media ticker | After incidents |

**Research and upgrades:** none.

**Counters:** *Beaten by (hard):* attacks on media sites (at escalation cost). *Situational:* a scenario unit; Psy-Ops and Intimidation ineligible (scenario-protected, framework §7.5).

**Status:** identity and NPC-only media E-R/E; mechanics P.

### UN-INF-14 — Federal Honor Guard

| Field | Specification |
| --- | --- |
| **Identity** | `unit.un.inf_14` · The Unaligned · Infantry / ceremonial guard · `TPL-NPC-SECURITY` · **Scenario** (scenario-protected) · NPC-only |
| **Role** | **Primary:** scenario-protected symbolic and ceremonial detachment, **not recruited by players** (E-R). **Secondary:** guarding the leadership in campaign scenes. **Doctrine fit:** continuity of federal authority (E). |
| **Inspiration and background** | The federal guard in dress uniform with rifles and white gloves. C&C lineage (E-R): Commando (TD, special-only concept); Tanya (RA2, unique-person role). |
| **Visual identity** | Concept: none (asset requirement). Dress uniforms, white belts and gloves, ceremonial rifles. |
| **Statistics** | 6 × 80 HP = **480** (`armor.inf_protected`) · FOOT 200 cm/s · sight 1300 cm · `DETECT.SPY` 600 cm · ExperienceValue 0 (no XP may be earned from it) |
| **Commands** | Scenario only |
| **Production** | Not mobilized; placed by scenario at `UN-BLD-01` or a ceremonial site |
| **Veterancy** | none |
| **AI** | `ai.unit.npc_security`, scenario script |
| **Multiplayer** | MPA-NPC |
| **Presentation** | ANIM-INF + drill animations. Icon of a laurel |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.un.inf_14.primary` rifles | SMALL_ARMS | 0–650 cm | 12 per member / 1.0 s | hitscan | 3 | GROUND | 100 % / 15 % / 2 % / — / 8 % |

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.un.inf_14.scenario_protected` | Passive (E-R; P mechanics) | Not targetable by player auto-acquisition; a deliberate attack is an immediate **R4** incident for that participant; ineligible for Psy-Ops and Intimidation (framework §7.5) | Automatic | — | Scenario only | — | — |

**Research and upgrades:** none.

**Counters:** none by design (scenario-protected).

**Status:** identity and protection E-R; mechanics P.

## 4. Military vehicles, patrols and field response

Vehicles use `ai.unit.npc_security` with the template's profile weights, MPA-NPC, and spawn from `UN-BLD-05` JNA Barracks Complex, `UN-BLD-07` Federal Tank and Repair Works or `UN-BLD-06` Territorial Defence Depot unless stated. Fuel and crew food are inside the mobilization weight.

### UN-ARM-01 — Municipal Patrol Car

| Field | Specification |
| --- | --- |
| **Identity** | `unit.un.arm_01` · The Unaligned · Vehicle / police car · `TPL-NPC-SECURITY` (as `TPL-VEH-SCOUT`) · **Core** · NPC-only |
| **Role** | **Primary:** police response and traffic checks; **jurisdiction-sensitive** (E-R). **Secondary:** carrying UN-INF-04 to incidents; road inspections. **Doctrine fit:** police regulate access and respond to disturbances (E). |
| **Inspiration and background** | A Yugoslav-built compact sedan in police livery with a light bar. C&C lineage (E-R): Ranger (RA1); Humvee (TD); Attack Buggy (TS, chassis role). |
| **Visual identity** | none (asset requirement): white-and-blue compact car, "MILICIJA"-style fictional lettering, blue light bar. |
| **Statistics** | **200 HP** (`armor.veh_soft`) · WHEEL_LIGHT 520 cm/s · sight 1400 cm · `DETECT.SPY` 500 cm · 2 police (counts as 1 UN-INF-04 when dismounted) · no ranks |
| **Commands** | Institutional authority only · default stance Hold Fire below R2 |
| **Production** | Mobilization weight **180** · response tier **R0** · `UN-SEC-01` · 5 s step |
| **Veterancy** | none |
| **AI** | `ai.unit.npc_security` (scout weights). Deltas: patrol municipal roads; respond to incidents within the municipality; never cross a republic border |
| **Multiplayer** | MPA-NPC |
| **Presentation** | ANIM-VEH-WHEEL + siren and lights at R1+. Icon of a police car |

**Weapons:** none mounted; occupants dismount with `weapon.un.inf_04.primary` at R2.

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.un.arm_01.traffic_stop` | Active (P) | Stops one foreign or civilian vehicle within 300 cm for an *inspection* (as UN-INF-04) | Vehicle | free · 4 s · 15 s | Do not stop: driving on is an R1 incident | Siren whoop | R1+ on suspicious traffic |
| `ability.un.arm_01.roadblock` | Active (P) | Parks across a road and creates a 600 cm checkpoint zone for 60 s: foreign military units passing without an agreement create an R1 incident | Road point | free · 60 s · 120 s | Alternate roads | Cones and lights | Block incident exits |

**Research and upgrades:** none.

**Counters:** *Beats (hard):* disguised couriers and paperwork vans (IA-INF-14, MP-SUP-06, MP-MKT-06 decoys). *Beaten by (hard):* any armed unit, at escalation cost. *Situational:* the visible face of jurisdiction.

**Status:** identity and jurisdiction sensitivity E-R/E; values P.

### UN-ARM-02 — Federal Recon Jeep

| Field | Specification |
| --- | --- |
| **Identity** | `unit.un.arm_02` · The Unaligned · Vehicle / army recon jeep · `TPL-NPC-SECURITY` (as `TPL-VEH-SCOUT`) · **Core** · NPC-only |
| **Role** | **Primary:** army visual patrol of roads and bases (E-R). **Secondary:** spotting for UN-ARM-09/10 at R4. **Doctrine fit:** JNA reconnaissance (E). |
| **Inspiration and background** | A light army jeep with a pintle MG and radio. C&C lineage (E-R): Humvee (TD); Pitbull (TW); Ranger (RA1). |
| **Visual identity** | none (asset requirement): olive jeep, canvas top, red-star roundel, whip antenna. |
| **Statistics** | **240 HP** (`armor.veh_soft`) · WHEEL_LIGHT 560 cm/s · sight **1800 cm** · `DETECT.CONCEALED` 1080 cm · crew 2 · ExperienceValue 20 |
| **Commands** | Institutional authority only · default stance Hold Ground |
| **Production** | Mobilization weight **200** · response tier **R0** (patrol) · `UN-BLD-05` · 6 s step |
| **Veterancy** | Standard |
| **AI** | `ai.unit.npc_security` (scout weights) |
| **Multiplayer** | MPA-NPC |
| **Presentation** | ANIM-VEH-WHEEL + gunner. Icon of a jeep with a star |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.un.arm_02.primary` pintle MG | SMALL_ARMS | 0–700 cm | 12 / 0.4 s | hitscan, fire while moving | 3 | GROUND, AIR_LOW | 100 % / 15 % / 2 % / 25 % rotary / 8 % |

**Abilities:** none unit-specific; sight and detection are its role.

**Research and upgrades:** none.

**Counters:** *Beats (soft):* opening scouts that trespass (MP-ARM-01, RD-ARM-01). *Beaten by (hard):* any tank, AT infantry. *Situational:* it is how the federation notices a hidden army.

**Status:** identity E-R; values P.

### UN-ARM-03 — Border Surveillance Truck

| Field | Specification |
| --- | --- |
| **Identity** | `unit.un.arm_03` · The Unaligned · Vehicle / border sensor truck · `TPL-NPC-SECURITY` (as `TPL-VEH-EW`) · **Extended** · NPC-only |
| **Role** | **Primary:** radio-equipped mobile border watch (E-R). **Secondary:** emitter intercept for UN-INF-11. **Doctrine fit:** border authorities observe international movements (E). |
| **Inspiration and background** | A box-body truck with a mast, ground radar and radio intercept set. C&C lineage (E-R): Radar Van (Gen); Mobile Sensor Array (TS); Mobile Gap Generator (RA1, sensor role). |
| **Visual identity** | none (asset requirement): olive box truck, telescopic mast, generator. |
| **Statistics** | **380 HP** (`armor.veh_light`) · WHEEL_HEAVY 360 cm/s · sight 1400 cm; deployed: ground surveillance 2600 cm (moving vehicles only, contacts), `DETECT.EMITTER` 2400 cm, `DETECT.SPY` 400 cm · crew 3 · no ranks |
| **Commands** | Institutional authority only · default stance Hold Fire |
| **Production** | Mobilization weight **450** · response tier **R1** · `UN-SEC-02` or `UN-BLD-05` · 10 s step |
| **Veterancy** | none |
| **AI** | `ai.unit.electronic_warfare`. Deltas: deploy on high ground near the border |
| **Multiplayer** | MPA-NPC + MPA-SENSOR |
| **Presentation** | ANIM-VEH + mast. Icon of a mast with waves |

**Weapons:** none (unarmed)

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.un.arm_03.border_watch` | Active (P) | 8 s deploy: moving ground vehicles within 2600 cm become contacts for the institutional AI (position band, no identity); crossings without agreement become R1 incidents | Self | free · 8 s | Move slowly (< 25 % speed is not detected); kill it (incident) | Mast up | Deploy on border ridges |

**Research and upgrades:** none.

**Counters:** *Beaten by (hard):* any armed unit; jamming (AC-ARM-10, RD-SUP-06, IA-ARM-14) suppresses contacts. *Situational:* makes border crossings by large forces visible to the federation.

**Status:** identity E-R; values P.

### UN-ARM-04 — JNA Armored Personnel Carrier

| Field | Specification |
| --- | --- |
| **Identity** | `unit.un.arm_04` · The Unaligned · Vehicle / APC · `TPL-NPC-SECURITY` (as `TPL-VEH-TRANSPORT`) · **Core** · NPC-only |
| **Role** | **Primary:** protected federal infantry transport (E-R). **Secondary:** fire support at R3+. **Doctrine fit:** JNA mechanized infantry (E). |
| **Inspiration and background** | A domestic eight-wheeled or tracked APC with a 12.7-mm-class MG. C&C lineage (E-R): APC (TD); Riptide ACV (RA3, land role); Armadillo (TT). |
| **Visual identity** | none (asset requirement): olive APC, roof MG, red-star roundel. |
| **Statistics** | **560 HP** (`armor.veh_light`) · TRACKED 360 cm/s · sight 1200 cm · **5 infantry slots**, 2 fire ports · crew 2 · ExperienceValue 60 |
| **Commands** | Institutional authority only · default stance Guard |
| **Production** | Mobilization weight **650** · response tier **R3** · `UN-BLD-05` · 12 s step |
| **Veterancy** | Standard |
| **AI** | `ai.unit.npc_security` with `ai.unit.transport` behavior |
| **Multiplayer** | MPA-NPC + MPA-TRANSPORT |
| **Presentation** | ANIM-VEH-TRACK + rear doors. Icon of an APC with a star |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.un.arm_04.hmg` | SMALL_ARMS | 0–800 cm | 18 / 0.5 s | hitscan | 4 | GROUND, AIR_LOW | 100 % / 15 % / 2 % / 25 % rotary / 8 % |

**Abilities:** none unit-specific.

**Research and upgrades:** none.

**Counters:** *Beats (soft):* infantry and scouts. *Beaten by (hard):* AT infantry and tanks of any faction. *Situational:* the R3 delivery system for UN-INF-01 and UN-INF-06.

**Status:** identity E-R; values P.

### UN-ARM-05 — Territorial Defence Truck

| Field | Specification |
| --- | --- |
| **Identity** | `unit.un.arm_05` · The Unaligned · Vehicle / reserve truck · `TPL-NPC-SECURITY` (as `TPL-VEH-TRANSPORT`) · **Extended** · NPC-only |
| **Role** | **Primary:** inexpensive reserve transport and ammunition hauling (E-R). **Secondary:** resupply for UN-INF-06 and UN-ARM-09/10. **Doctrine fit:** territorial mobilization (E). |
| **Inspiration and background** | A civilian-pattern six-wheeled lorry requisitioned by the TO. C&C lineage (E-R): Technical (Gen); Troop Crawler (Gen); Flak Track (RA2, chassis). |
| **Visual identity** | none (asset requirement): olive lorry with a canvas tilt and TO markings. |
| **Statistics** | **320 HP** (`armor.veh_soft`) · WHEEL_HEAVY 400 cm/s · sight 1100 cm · **3 infantry slots** or ammunition stock 10 · crew 1 · no ranks |
| **Commands** | Institutional authority only · default stance Hold Fire |
| **Production** | Mobilization weight **180** · response tier **R3** · `UN-BLD-06` · 6 s step |
| **Veterancy** | none |
| **AI** | `ai.unit.npc_security` with `ai.unit.logistics` behavior |
| **Multiplayer** | MPA-NPC + MPA-TRANSPORT |
| **Presentation** | ANIM-VEH-WHEEL. Icon of a lorry with a TO shield |

**Weapons:** none (unarmed)

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.shared.resupply` | Aura (P; framework §10.4) | Resupplies Unaligned limited-ammunition units within 400 cm | Unaligned units | stock 10 | Kill the truck | Crate icon | Behind TO lines |

**Research and upgrades:** none.

**Counters:** *Beaten by (hard):* any armed unit. *Situational:* cutting TO trucks shortens an R3 mobilization.

**Status:** identity E-R; values P.

### UN-ARM-06 — Federal Battle Tank

| Field | Specification |
| --- | --- |
| **Identity** | `unit.un.arm_06` · The Unaligned · Vehicle / main battle tank · `TPL-NPC-SECURITY` (as `TPL-VEH-MBT`) · **Core** · NPC-only |
| **Role** | **Primary:** conventional Yugoslav-era armored formation, medium or heavy (E-R). **Secondary:** defending JNA installations and federal institutions at R4. **Doctrine fit:** JNA conventional armored formations (E). |
| **Inspiration and background** | A licence-built second-generation MBT with a 125-mm-class gun and autoloader. C&C lineage (E-R): Medium Tank (TD); Rhino Tank (RA2); Predator Tank (TW). |
| **Visual identity** | none (asset requirement): olive low-profile tank, red star on the turret, unit number. |
| **Statistics** | **900 HP** (`armor.veh_heavy`) · TRACKED 320 cm/s · sight 1300 cm · crew 3 · ExperienceValue 110 |
| **Commands** | Institutional authority only · default stance Aggressive (against the violating participant only) |
| **Production** | Mobilization weight **1,100** · response tier **R4** · `UN-BLD-07` or `UN-BLD-05` · 18 s step |
| **Veterancy** | Standard |
| **AI** | `ai.unit.npc_security` with `ai.unit.battle_tank` weights. Deltas: engage only the violating participant; return to base when the incident is resolved |
| **Multiplayer** | MPA-NPC |
| **Presentation** | ANIM-VEH-TRACK + turret. Icon of a tank with a star |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.un.arm_06.gun` 125-mm-class gun | KINETIC_AP (HE vs infantry and structures: 65, 160 cm) | 0–900 cm | 130 / 3.4 s | projectile | 4 | GROUND | 20 % (HE 110 %) / 90 % / 100 % / — / 40 % (HE 100 %) |
| coaxial MG | SMALL_ARMS | 0–650 cm | 8 / 0.5 s | hitscan | 2 | GROUND | 100 % / 15 % / 2 % / — / 8 % |

**Abilities:** none unit-specific; its weight and tier gate it.

**Research and upgrades:** none.

**Counters:** *Beats (hard):* light and medium armor, technicals, APCs. *Beats (soft):* most playable MBTs at equal numbers. *Beaten by (hard):* ATGMs (AC-ARM-06, IA-ARM-09), heavy tanks (AC-ARM-05, RD-ARM-05, IA-ARM-05), attack aircraft. *Situational:* R4 is the federation's answer to an invasion; the best counter is not to provoke it.

**Status:** identity E-R; R4 engagement rule E; values P.

### UN-ARM-07 — Reserve Tank

| Field | Specification |
| --- | --- |
| **Identity** | `unit.un.arm_07` · The Unaligned · Vehicle / older reserve tank · `TPL-NPC-SECURITY` (as `TPL-VEH-MBT`, light variant) · **Extended** · NPC-only |
| **Role** | **Primary:** older stored equipment mobilized near crises (E-R). **Secondary:** TO armor support at R3. **Doctrine fit:** territorial reserves (E). |
| **Inspiration and background** | A stored first-generation tank with an 85- or 100-mm-class gun. C&C lineage (E-R): Light Tank (TD); Grizzly Tank (RA2); Scorpion Tank (TW). |
| **Visual identity** | none (asset requirement): rounded older turret, worn olive paint, TO markings. |
| **Statistics** | **680 HP** (`armor.veh_medium`) · TRACKED 300 cm/s · sight 1200 cm · crew 4 · ExperienceValue 60 |
| **Commands** | Institutional authority only · default stance Guard |
| **Production** | Mobilization weight **600** · response tier **R3** · `UN-BLD-06` · 14 s step |
| **Veterancy** | Standard |
| **AI** | `ai.unit.npc_security` with `ai.unit.battle_tank` weights; defensive positions only |
| **Multiplayer** | MPA-NPC |
| **Presentation** | ANIM-VEH-TRACK. Icon of an old tank |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.un.arm_07.gun` 100-mm-class gun | KINETIC_AP (HE 55, 150 cm vs infantry) | 0–850 cm | 95 / 3.6 s | projectile | 3 | GROUND | 20 % (HE 110 %) / 90 % / 100 % / — / 40 % |

**Abilities:** none unit-specific.

**Research and upgrades:** none.

**Counters:** *Beats (hard):* technicals and APCs. *Beaten by (hard):* modern MBTs, ATGMs, AT infantry. *Situational:* fills R3 lines before R4 armor arrives.

**Status:** identity E-R; values P.

### UN-ARM-08 — JNA Anti-Air Gun Carrier

| Field | Specification |
| --- | --- |
| **Identity** | `unit.un.arm_08` · The Unaligned · Vehicle / mobile AA gun · `TPL-NPC-SECURITY` (as `TPL-VEH-AA`) · **Core** · NPC-only |
| **Role** | **Primary:** mobile short-range air defense (E-R). **Secondary:** answering unauthorized helicopters at R2+. **Doctrine fit:** defense of sovereign airspace (E). |
| **Inspiration and background** | A tracked self-propelled triple 20-mm-class or twin 30-mm-class AA gun. C&C lineage (E-R): Flak Track (RA2); Gattling Tank (Gen); Slingshot (KW). |
| **Visual identity** | none (asset requirement): olive tracked chassis, open gun turret, optical sight. |
| **Statistics** | **520 HP** (`armor.veh_light`) · TRACKED 330 cm/s · sight 1300 cm / air 2600 cm (optical, linked to UN-SEC-05 radar contacts when in coverage) · crew 3 · ExperienceValue 70 |
| **Commands** | Institutional authority only · default stance Aggressive (air) |
| **Production** | Mobilization weight **800** · response tier **R2** for unauthorized aircraft, **R3** otherwise · `UN-BLD-05` · 14 s step |
| **Veterancy** | Standard |
| **AI** | `ai.unit.npc_security` with `ai.unit.air_defense` weights. Deltas: fire only at aircraft of participants without an overflight agreement or above R2 |
| **Multiplayer** | MPA-NPC |
| **Presentation** | ANIM-VEH + gun traverse, tracer streams. Icon of three barrels |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.un.arm_08.aa_guns` | AUTOCANNON | 0–1000 cm ground, 1200 cm air | 3 × 12 / 0.5 s | projectile, fast | 4 | AIR_LOW, AIR_HIGH, GROUND | 65 % / 85 % / 10 % / 100 % rotary, 60 % fixed / 30 % |

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.un.arm_08.airspace_warning` | Automated (P) | Before firing at R2, aircraft of the trespassing participant receive a 5 s warning; leaving within 5 s avoids an R3 incident | Trespassing aircraft | free | Comply or suppress the gun | Warning to that participant | Always warn first at R2 |

**Research and upgrades:** none.

**Counters:** *Beats (hard):* helicopters (AC-AIR-04/05/09, RD-AIR-03/04/08, IA-AIR-03/04, MP-AIR-01/02). *Beaten by (hard):* tanks, artillery, high-altitude jets. *Situational:* helicopters crossing federal territory need agreements or escorts.

**Status:** identity E-R; warning rule P implementing graduated response E; values P.

### UN-ARM-09 — Army Rocket Artillery

| Field | Specification |
| --- | --- |
| **Identity** | `unit.un.arm_09` · The Unaligned · Vehicle / multiple rocket launcher · `TPL-NPC-SECURITY` (as `TPL-VEH-ARTY`) · **Core** · NPC-only |
| **Role** | **Primary:** conventional mobile multiple-launch rockets (E-R). **Secondary:** destroying occupying forces around seized federal sites at R4. **Doctrine fit:** JNA artillery (E). |
| **Inspiration and background** | A domestic truck-mounted 128-mm-class rocket launcher. C&C lineage (E-R): MLRS (TD); V3 Rocket Launcher (RA2); V4 Rocket Launcher (RA3). |
| **Visual identity** | none (asset requirement): olive six-wheel truck, 32-tube launcher, cab shutters. |
| **Statistics** | **420 HP** (`armor.veh_light`) · WHEEL_HEAVY 320 cm/s · sight 1100 cm · crew 3 · ExperienceValue 120 · death blast (U-07) |
| **Commands** | Institutional authority only · default stance Hold Ground |
| **Production** | Mobilization weight **1,200** · response tier **R4** · `UN-BLD-05` · 20 s step |
| **Veterancy** | Standard |
| **AI** | `ai.unit.npc_security` with `ai.unit.artillery` weights. Deltas: fire only on targets observed by Unaligned units and owned by the violating participant; never fire within 600 cm of civilians |
| **Multiplayer** | MPA-NPC + MPA-INDIRECT |
| **Presentation** | ANIM-VEH + ripple launch. Icon of a rocket rack with a star |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.un.arm_09.rockets` | HIGH_EXPLOSIVE | **1000–3800 cm** | 16 rockets × 55, radius 220 cm, ripple 4 s; 1 salvo per magazine, 2 magazines; reload 25 s | ballistic, scatter 260 cm (observed ×0.7) | 12 per hit | GROUND, STRUCTURE | 110 % / 45 % / 12 % / — / 100 % |

**Abilities:** none unit-specific; the civilian-safety rule is an AI constraint.

**Research and upgrades:** none.

**Counters:** *Beats (hard):* massed infantry and occupying bases. *Beaten by (hard):* counter-battery (AC-ARM-12), aircraft, raids. *Situational:* R4 punishment for seizing federal sites.

**Status:** identity E-R; values P.

### UN-ARM-10 — Field Howitzer Tractor

| Field | Specification |
| --- | --- |
| **Identity** | `unit.un.arm_10` · The Unaligned · Vehicle / towed howitzer and tractor · `TPL-NPC-SECURITY` (as `TPL-VEH-ARTY`) · **Extended** · NPC-only |
| **Role** | **Primary:** towed or self-propelled artillery for defensive fighting (E-R). **Secondary:** counter-battery at fixed defenses. **Doctrine fit:** territorial defense in depth (E). |
| **Inspiration and background** | A 122- or 152-mm-class towed howitzer behind an artillery tractor. C&C lineage (E-R): Artillery (RA1); Juggernaut (TW, artillery role). |
| **Visual identity** | none (asset requirement): olive tractor with a howitzer on tow; crew dismounts to fire. |
| **Statistics** | **480 HP** (`armor.veh_light`; tractor and gun as one entity) · WHEEL_HEAVY 280 cm/s · sight 1100 cm · crew 6 · ExperienceValue 120 |
| **Commands** | Institutional authority only · default stance Hold Ground |
| **Production** | Mobilization weight **1,100** · response tier **R4** · `UN-BLD-05` or `UN-BLD-06` · 22 s step |
| **Veterancy** | Standard |
| **AI** | `ai.unit.npc_security` with `ai.unit.artillery` weights; same civilian-safety rule as UN-ARM-09 |
| **Multiplayer** | MPA-NPC + MPA-INDIRECT |
| **Presentation** | ANIM-VEH + unlimber (8 s). Icon of a towed gun |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.un.arm_10.howitzer` | HIGH_EXPLOSIVE | **1000–4000 cm** (unlimbered) | 150, radius 280 cm / 6.0 s | ballistic, scatter 220 cm (observed ×0.7) | 14 | GROUND, STRUCTURE | 110 % / 45 % / 12 % / — / 100 % |

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.un.arm_10.unlimber` | Active (P) | 8 s unlimber to fire; 8 s limber | Self | free | Caught limbered | Crew animation | Unlimber at prepared positions |

**Research and upgrades:** none.

**Counters:** *Beats (hard):* fortifications, infantry, artillery. *Beaten by (hard):* counter-battery, aircraft, fast raiders. *Situational:* slow but long-ranged.

**Status:** identity E-R; values P.

### UN-ARM-11 — Military Engineering Dozer

| Field | Specification |
| --- | --- |
| **Identity** | `unit.un.arm_11` · The Unaligned · Vehicle / engineering dozer · `TPL-NPC-SECURITY` (as `TPL-VEH-ENGINEER`) · **Extended** · NPC-only |
| **Role** | **Primary:** move barriers and repair bridge approaches (E-R). **Secondary:** clearing wrecks from federal roads (no salvage income). **Doctrine fit:** engineering units (E). |
| **Inspiration and background** | A tracked military dozer with an armored cab. C&C lineage (E-R): Construction Dozer (Gen); Dozer (TT); Crusher Crane (RA3, recovery role). |
| **Visual identity** | none (asset requirement): olive dozer, armored cab, blade. |
| **Statistics** | **800 HP** (`armor.veh_heavy`) · TRACKED 280 cm/s · sight 1000 cm · crew 1 · no ranks |
| **Commands** | Institutional authority only · default stance Hold Fire |
| **Production** | Mobilization weight **600** · response tier **R0** (works), **R3** (barriers) · `UN-BLD-05` · 14 s step |
| **Veterancy** | none |
| **AI** | `ai.unit.engineer_vehicle` |
| **Multiplayer** | MPA-NPC |
| **Presentation** | ANIM-VEH + blade. Icon of a dozer blade |

**Weapons:** none (unarmed)

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.un.arm_11.build_barrier` | Active (P) | Raises a 600 cm earth barrier in 12 s that blocks vehicle movement (800 HP, `armor.str_hardened`) | Point | budget · 12 s | Demolition, engineers, heavy HE | Earth VFX | Block roads to seized federal sites at R3 |
| `ability.un.arm_11.clear_route` | Active (P) | Removes wrecks, rubble and barriers in 6 s each | Obstacle ≤ 200 cm | free · 6 s | Fire on the dozer | Push animation | Keep federal roads open |

**Research and upgrades:** none.

**Counters:** *Beaten by (hard):* AT weapons. *Situational:* blockades isolate occupiers from their supply.

**Status:** identity E-R; values P.

### UN-ARM-12 — Federal Mobile Repair Unit

| Field | Specification |
| --- | --- |
| **Identity** | `unit.un.arm_12` · The Unaligned · Vehicle / repair truck · `TPL-NPC-SECURITY` (as `TPL-VEH-LOGISTICS`) · **Extended** · NPC-only |
| **Role** | **Primary:** stabilizes damaged JNA and public vehicles (E-R). **Secondary:** repair of civilian vehicles after incidents. **Doctrine fit:** continuity of public services (E). |
| **Inspiration and background** | A workshop truck with a crane. C&C lineage (E-R): Mobile Repair Vehicle (TS); Mobile Repair Transport (KW); Rig (TW). |
| **Visual identity** | none (asset requirement): olive workshop truck, crane, red-star door. |
| **Statistics** | **500 HP** (`armor.veh_light`) · WHEEL_HEAVY 350 cm/s · sight 1000 cm · crew 2 · no ranks |
| **Commands** | Institutional authority only · default stance Hold Fire |
| **Production** | Mobilization weight **500** · response tier **R0** · `UN-BLD-07` · 12 s step |
| **Veterancy** | none |
| **AI** | `ai.unit.logistics` |
| **Multiplayer** | MPA-NPC |
| **Presentation** | ANIM-VEH + crane. Icon of a wrench on a truck |

**Weapons:** none (unarmed)

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.shared.repair` | Active (P) | 20 HP/s to one Unaligned vehicle within 300 cm; paid from the institutional budget | Unaligned vehicle | budget | Kill it (incident) | Sparks | Highest value first |

**Research and upgrades:** none.

**Counters:** *Beaten by (hard):* any armed unit. *Situational:* extends R4 operations.

**Status:** identity E-R; values P.

### UN-ARM-13 — Anti-Armor Defense Vehicle

| Field | Specification |
| --- | --- |
| **Identity** | `unit.un.arm_13` · The Unaligned · Vehicle / ATGM carrier · `TPL-NPC-SECURITY` (as `TPL-VEH-TD`) · **Extended** · NPC-only |
| **Role** | **Primary:** protect sensitive installations against raiders (E-R). **Secondary:** blocking armored thrusts on federal roads at R4. **Doctrine fit:** defense of strategic installations (E). |
| **Inspiration and background** | A wheeled reconnaissance carrier with a wire-guided ATGM launcher. C&C lineage (E-R): Tank Destroyer (RA2); Guardian Tank (RA3); Mantis (KW, counter unit role). |
| **Visual identity** | none (asset requirement): olive four-wheeler with a missile rail on the roof. |
| **Statistics** | **480 HP** (`armor.veh_medium`) · WHEEL_HEAVY 380 cm/s · sight 1400 cm · crew 3 · ExperienceValue 100 |
| **Commands** | Institutional authority only · default stance Hold Ground |
| **Production** | Mobilization weight **1,000** · response tier **R3** at protected installations · `UN-BLD-05` · 16 s step |
| **Veterancy** | Standard |
| **AI** | `ai.unit.npc_security` with `ai.unit.tank_destroyer` weights |
| **Multiplayer** | MPA-NPC |
| **Presentation** | ANIM-VEH + launcher. Icon of a missile and shield |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.un.arm_13.atgm` wire-guided | SHAPED_CHARGE | 0–1300 cm | 220 / 6.0 s; magazine 6 | guided (wire: not jammable) | 3 | GROUND | 25 % / 100 % / 90 % / 60 % rotary / 70 % |

**Abilities:** none unit-specific.

**Research and upgrades:** none.

**Counters:** *Beats (hard):* raiding armor (MP-ARM-06/07, IA-ARM-03, RD-ARM-04). *Beaten by (hard):* infantry, artillery, smoke (MP-ARM-10 blocks the wire's line of sight). *Situational:* the guard of laboratories and refineries.

**Status:** identity E-R; values P.

### UN-ARM-14 — Reserve Mobilization Carrier

| Field | Specification |
| --- | --- |
| **Identity** | `unit.un.arm_14` · The Unaligned · Vehicle / reserve troop carrier · `TPL-NPC-SECURITY` (as `TPL-VEH-TRANSPORT`) · **Extended** · NPC-only |
| **Role** | **Primary:** move Territorial Defence squads under NPC orders (E-R). **Secondary:** evacuating UN-INF-07 teams. **Doctrine fit:** reserve mobilization (E). |
| **Inspiration and background** | An older armored carrier from TO stocks. C&C lineage (E-R): Troop Crawler (Gen); Reckoner (KW); Amphibious APC (TS). |
| **Visual identity** | none (asset requirement): boxy older APC in faded olive, TO shield. |
| **Statistics** | **500 HP** (`armor.veh_light`) · WHEEL_HEAVY 360 cm/s (amphibious ×0.4) · sight 1100 cm · **4 infantry slots** · crew 2 · no ranks |
| **Commands** | Institutional authority only · default stance Hold Fire |
| **Production** | Mobilization weight **450** · response tier **R3** · `UN-BLD-06` · 10 s step |
| **Veterancy** | none |
| **AI** | `ai.unit.transport` |
| **Multiplayer** | MPA-NPC + MPA-TRANSPORT |
| **Presentation** | ANIM-VEH-WHEEL. Icon of a carrier with a TO shield |

**Weapons:** none (unarmed)

**Abilities:** none unit-specific.

**Research and upgrades:** none.

**Counters:** *Beaten by (hard):* AT weapons and autocannon. *Situational:* cutting carriers isolates TO platoons in their municipalities.

**Status:** identity E-R; values P.

### UN-ARM-15 — Crisis Command Vehicle

| Field | Specification |
| --- | --- |
| **Identity** | `unit.un.arm_15` · The Unaligned · Vehicle / mobile command post · `TPL-NPC-SECURITY` (as `TPL-VEH-EW`) · **Extended** · NPC-only |
| **Role** | **Primary:** small mobile command post for public-order escalation (E-R). **Secondary:** coordinating police and TO at R2–R3. **Doctrine fit:** decentralized but coordinated territorial response (E). It has **no talents**; the Unaligned has no talent tree (E). |
| **Inspiration and background** | A command truck with radio masts and a staff cabin. C&C lineage (E-R): Support Crawler (TT, mobile HQ inspiration); Rig (TW); Mobile Construction Vehicle (TD). |
| **Visual identity** | none (asset requirement): olive box truck, two masts, awning. |
| **Statistics** | **520 HP** (`armor.veh_light`) · WHEEL_HEAVY 340 cm/s · sight 1400 cm · `DETECT.EMITTER` 2000 cm · crew 4 · no ranks |
| **Commands** | Institutional authority only · default stance Hold Fire |
| **Production** | Mobilization weight **700** · response tier **R2** · `UN-SEC-10` Regional Emergency Headquarters · 14 s step · one per incident |
| **Veterancy** | none |
| **AI** | `ai.unit.officer` (NPC) |
| **Multiplayer** | MPA-NPC |
| **Presentation** | ANIM-VEH + masts. Icon of a mast over a star |

**Weapons:** none (unarmed)

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.un.arm_15.incident_command` | Aura (P; officer aura, framework §17) | Unaligned units within 1600 cm: +10 maximum resolve and −15 % mobilization step time at the incident; removed by jamming | Unaligned units | free · while deployed | Jamming (AC-ARM-10, RD-SUP-06, IA-ARM-14); kill it (incident) | Command ring (none to players) | Deploy at the incident centre |

**Research and upgrades:** none.

**Counters:** *Beaten by (hard):* any armed unit; jammers. *Situational:* removing it slows the federation's escalation, at a political cost.

**Status:** identity and "no talents" E-R/E; values P.

## 5. Civilian, municipal and service vehicles

Civilian entries use `TPL-CIV`, `ai.unit.civilian_ambient`, MPA-NPC and ANIM-CIV unless stated. They are **never commandable by anyone** and have no response tier: they run on system schedules (routes, timetables, incident dispatch) authored by the map and the world simulation (E). They use **Morale**, not Discipline (E). They carry no weapons. Attacking one is an **incident** for the attacker: R2 at the first civilian casualty or destroyed civilian vehicle, R3 on repetition within 180 s (P implementing E). Instead of a mobilization weight they have an **ambient density** (vehicles per authored route or district) and, where relevant, an **economic flow value** (USD-equivalent per delivery that the Unaligned economy credits, never a participant).

Shared civilian rules (P):

| Rule | Value |
| --- | --- |
| Flee | Leave any position within 1000 cm of weapon fire or an explosion; seek the nearest shelter or route exit (`ai.unit.civilian_ambient`) |
| Morale | 100 maximum; −30 when fired upon, −15 when a nearby civilian is hit. At 0 Morale the driver abandons the vehicle (it stops and blocks its lane until UN-ARM-11 or UN-CIV-07 clears it or 90 s pass). Concord Psy-Ops may push Morale to collapse **only where an authored attack action exists** (E); civilian vehicles have none, so Psy-Ops can only cause abandonment |
| Intimidation | Not eligible while the Unaligned is non-hostile to the Ascendancy participant (E). Civilian vehicles never become eligible: they have no Discipline to break and no combat role (P, consistent with E) |
| Capture | Never capturable or commandeerable by players (E: never player-commandable). Destroyed civilian vehicles leave wrecks that only Unaligned units clear |
| Food and fuel | Consume nothing from any participant ledger |
| Targeting | Auto-acquire never selects a civilian (P). An explicit attack order is accepted only with a confirmation prompt that names the incident cost (P; U-UN-01) |

### UN-CIV-01 — Civilian Passenger Car

| Field | Specification |
| --- | --- |
| **Identity** | `unit.un.civ_01` · The Unaligned · Civilian / road vehicle · `TPL-CIV` · **Core** · ambient, not recruitable |
| **Role** | **Primary:** ambient traffic and continuity; not recruitable (E-R). **Secondary:** an observable sign of normality; traffic thins as incidents rise. **Doctrine fit:** the living civilian world the Unaligned protects (E). |
| **Inspiration and background** | Small Yugoslav family saloons and hatchbacks on town and highway roads. C&C lineage (E-R): Ranger (RA1, traffic analogue); Humvee (TD, road unit chassis). |
| **Visual identity** | Concept: none (asset requirement). Four or five low-poly saloon variants in period colours, Yugoslav plates; two-tone horn audio. |
| **Statistics** | **120 HP** (`armor.veh_soft`) · WHEEL_LIGHT 480 cm/s on roads (road-locked) · sight 800 cm · F1 · Morale 100 |
| **Commands** | None (system routes) · Hold Fire |
| **Production** | Ambient density 2–6 per km of authored road, scaled by the district's Population Morale and the current response tier (R3: ×0.5, R4: ×0.2) · spawned at route sources |
| **Veterancy** | none |
| **AI** | `ai.unit.civilian_ambient`. Deltas: pull over for Unaligned emergency vehicles (UN-ARM-01, UN-CIV-06/07) |
| **Multiplayer** | MPA-NPC. Positions are server-authoritative; with large counts, observers receive quantized transforms at reduced frequency (P, U-UN-02) |
| **Presentation** | ANIM-CIV (drive, brake lights, flee swerve, abandoned state). No selection ring; tooltip only |

**Weapons:** none (unarmed)

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.un.civ.traffic_witness` | Automated (P) | A civilian vehicle within 1000 cm of a hostile act reports it to the institutional AI (the same report channel as UN-INF-03 *incident_report*); attribution follows the observed participant and can be wrong if forged (E) | Observed hostile act | free · instant | Act away from traffic; kill witnesses (another incident) | none to players | Always |

**Research and upgrades:** none.

**Counters:** *Beaten by (hard):* anything armed, at the price of escalation. *Situational:* traffic is cover for disguised agents (AC-INF-11, IA-INF-09) and a witness network against them.

**Status:** identity and "not recruitable" E-R/E; witness reporting and density values P.

### UN-CIV-02 — Intercity Bus

| Field | Specification |
| --- | --- |
| **Identity** | `unit.un.civ_02` · The Unaligned · Civilian / passenger transport · `TPL-CIV` (WHEEL_HEAVY) · **Extended** · ambient and incident-dispatched |
| **Role** | **Primary:** move civilians and evacuation groups under incident rules (E-R). **Secondary:** timetabled intercity traffic. **Doctrine fit:** civil protection within Total People's Defence (E). |
| **Inspiration and background** | Long-distance coaches of republican transport enterprises. C&C lineage (E-R): APC (TD, transport role); Troop Crawler (Gen, capacity analogy). |
| **Visual identity** | Concept: none (asset requirement). Long coach in enterprise livery, roof rack, destination board. |
| **Statistics** | **300 HP** (`armor.veh_soft`) · WHEEL_HEAVY 400 cm/s (road-locked) · sight 800 cm · F2 · **evacuation capacity 4 civilian groups** · Morale 100 |
| **Commands** | None · Hold Fire |
| **Production** | Timetable 1 per 120 s per intercity route; in incidents, dispatched by `UN-SEC-10` Regional Emergency Headquarters (2 per incident at R2+) |
| **Veterancy** | none |
| **AI** | `ai.unit.civilian_ambient`. Deltas: in evacuation mode, drives to the incident district, loads for 10 s and leaves along the safest authored route |
| **Multiplayer** | MPA-NPC (passenger count not replicated; full/empty indicator only) |
| **Presentation** | ANIM-CIV + door open. Icon of a bus with a cross in evacuation mode |

**Weapons:** none (unarmed)

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.un.civ_02.evacuate` | Automated (P) | Removes up to 4 civilian groups from an incident district; each evacuated group prevents the −2 Unaligned Population Morale a civilian casualty would cause | Incident district | free · 10 s load | Blocking roads; firing near it (incident) | Evacuation marker visible to all (public event) | Dispatch at R2+ |

**Research and upgrades:** none.

**Counters:** *Beaten by (hard):* anything armed (a destroyed evacuation bus is a severe incident: R3 immediately). *Situational:* an evacuation in progress signals to every participant that an incident is escalating.

**Status:** identity E-R; evacuation mechanics and values P.

### UN-CIV-03 — Freight Lorry

| Field | Specification |
| --- | --- |
| **Identity** | `unit.un.civ_03` · The Unaligned · Civilian / goods vehicle · `TPL-CIV` (WHEEL_HEAVY) · **Core** · ambient |
| **Role** | **Primary:** factories, warehouses and shops maintain economy flows (E-R). **Secondary:** cover traffic for smuggling and courier routes; mission targets in scenarios. **Doctrine fit:** the self-managed civilian economy (E). |
| **Inspiration and background** | Medium lorries of socially owned enterprises. C&C lineage (E-R): Supply Truck (Gen); Ore Truck (RA1); Harvester (TW, supply role). |
| **Visual identity** | Concept: none (asset requirement). Cab-over lorry with tarpaulin, enterprise name on the doors. |
| **Statistics** | **220 HP** (`armor.veh_soft`) · WHEEL_HEAVY 380 cm/s (road-locked) · sight 800 cm · F1 · Morale 100 |
| **Commands** | None · Hold Fire |
| **Production** | Ambient: 1 per 60 s per active industrial route (`UN-BLD-12`, industrial buildings and markets) · economic flow value **$150 per delivery** to the Unaligned economy |
| **Veterancy** | none |
| **AI** | `ai.unit.civilian_ambient` |
| **Multiplayer** | MPA-NPC + MPA-ECON (delivery state server-only) |
| **Presentation** | ANIM-CIV + tarp flap. No icon |

**Weapons:** none (unarmed)

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.un.civ_03.economic_flow` | Automated (P; E: living economy) | Each completed delivery credits the Unaligned economy and adds +0.5 to the destination district's Population Morale (cap +5 per 300 s). Lost deliveries reduce both | Route destinations | — | Route interdiction (an incident) | none | — |

**Research and upgrades:** none.

**Counters:** *Beaten by (hard):* anything armed. *Situational:* raiding civilian freight is cheap and always an incident; it lowers Unaligned morale, which reduces Territorial Defence mobilization (UN-INF-02) and Unaligned production output (E: morale improves production).

**Status:** identity E-R; morale effect of production E; flow values P.

### UN-CIV-04 — Railway Freight Train

| Field | Specification |
| --- | --- |
| **Identity** | `unit.un.civ_04` · The Unaligned · Civilian / rail convoy · `TPL-CIV` (RAIL) · **Conditional** (rail maps) · scheduled |
| **Role** | **Primary:** schedules resource and merchant convoys; rail asset and mission target (E-R). **Secondary:** delivers fuel and goods to `UN-BLD-12` Railway Freight Terminal; rail traffic participants may contract under authored agreements. **Doctrine fit:** federal rail transport (E). |
| **Inspiration and background** | A diesel locomotive hauling flatcars, box wagons and tank wagons. C&C lineage (E-R): Cargo Plane (TD, delivery-event analogue); Supply Truck (Gen, convoy role); Harvester (TD, economic flow). |
| **Visual identity** | Concept: none (asset requirement). Blue-and-cream diesel locomotive; 4–8 modular wagons (box, flat, tank); horn audio audible at long range. |
| **Statistics** | Locomotive **900 HP** (`armor.veh_light`); each wagon **300 HP** (`armor.veh_soft`) · RAIL **700 cm/s** on authored splines, accel 6 s · sight 800 cm · F4 per car · Morale 100 (crew) |
| **Commands** | None (timetable) · Hold Fire |
| **Production** | Timetable 1 per 240 s per authored line · economic flow value **$600 per arrival**; tank wagons carry fuel tracked separately from crude (E: crude and finished petroleum are separate) |
| **Veterancy** | none |
| **AI** | `ai.unit.civilian_ambient`. Deltas: emergency stop when the track ahead is blocked or a bridge is damaged; waits for UN-INF-07 / UN-ARM-11 repair; escorted by UN-INF-05 at R1+ |
| **Multiplayer** | MPA-NPC + MPA-ECON. Train position along the spline is a single server value (distance + speed); clients interpolate (P) |
| **Presentation** | ANIM-CIV (rail bogies, coupling sway, derailment destruction set). Timetable visible on the minimap to all participants as a public schedule (P) |

**Weapons:** none (unarmed)

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.un.civ_04.consist` | Passive (P) | Each wagon is a separate damage entity; destroying the locomotive stops the train and creates a blocking wreck; destroying a tank wagon loses its cargo and creates a burning zone (HIGH_EXPLOSIVE 60/s within 300 cm for 20 s) | — | — | — | Burning wagon VFX | — |
| `ability.un.civ_04.line_closure` | Automated (P) | At R2+ in a district the institutional AI may halt trains through it, cutting deliveries for every participant that relies on the line | Institutional | — | Avoid incidents near the line | Public notice | — |

**Research and upgrades:** none.

**Counters:** *Beaten by (hard):* demolition on bridges and track (Meridian saboteurs MP-INF-08/09 at the price of an R3 incident), artillery. *Situational:* a high-value scenario target and the main reason railway bridges matter.

**Status:** identity and mission-target role E-R; separate crude/finished tracking E; consist, closure and values P.

### UN-CIV-05 — Oil Tanker Lorry

| Field | Specification |
| --- | --- |
| **Identity** | `unit.un.civ_05` · The Unaligned · Civilian / fuel road tanker · `TPL-CIV` (WHEEL_HEAVY) · **Extended** · ambient |
| **Role** | **Primary:** carry civilian crude oil and finished petroleum, which remain separately tracked (E-R). **Secondary:** feeds `UN-BLD-08` Public Fuel Refinery and filling stations. **Doctrine fit:** the federal fuel economy (E). |
| **Inspiration and background** | Articulated tank lorries of the state petroleum enterprise. C&C lineage (E-R): Ore Truck (RA1); Supply Truck (Gen); Ore Collector (RA3). |
| **Visual identity** | Concept: none (asset requirement). Cylindrical tank trailer; **cargo marker visible to all** (crude: black band; finished fuel: orange band). |
| **Statistics** | **260 HP** (`armor.veh_soft`) · WHEEL_HEAVY 360 cm/s (road-locked) · sight 800 cm · F2 · Morale 100 |
| **Commands** | None · Hold Fire |
| **Production** | Ambient: 1 per 90 s per active oil route · cargo 20 units crude **or** 15 units finished fuel (never both) |
| **Veterancy** | none |
| **AI** | `ai.unit.civilian_ambient` |
| **Multiplayer** | MPA-NPC + MPA-ECON (cargo type visible, amount server-only, matching framework §15 "tankers: yes") |
| **Presentation** | ANIM-CIV. Large fire on destruction |

**Weapons:** none (unarmed)

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.un.civ_05.fuel_fire` | Passive (P) | On destruction: burning zone, INCENDIARY 40/s within 350 cm for 25 s; the cargo is lost, never transferred to the attacker | — | — | Stay clear | Fire column | — |

**Research and upgrades:** none.

**Counters:** *Beaten by (hard):* anything armed. *Situational:* never a resource source for players; destroying tankers only lowers Unaligned fuel supply and morale, and raises the response tier.

**Status:** identity and separate tracking E-R/E; fire behavior and values P.

### UN-CIV-06 — Public Ambulance

| Field | Specification |
| --- | --- |
| **Identity** | `unit.un.civ_06` · The Unaligned · Civilian / emergency medical · `TPL-CIV` (WHEEL_LIGHT) · **Core** · incident-dispatched |
| **Role** | **Primary:** casualty response and morale stabilization after incidents (E-R). **Secondary:** carries casualties to `UN-BLD-15` State Hospital. **Doctrine fit:** civil protection (E). |
| **Inspiration and background** | White van ambulances of municipal health centres. C&C lineage (E-R): Ambulance (Gen); Medic (RA1, field role). |
| **Visual identity** | Concept: none (asset requirement). White van, red cross, blue lights and siren. |
| **Statistics** | **160 HP** (`armor.veh_soft`) · WHEEL_LIGHT 520 cm/s (road-preferring, can go off-road ×0.6) · sight 900 cm · F1 · Morale 120 (trained crew) |
| **Commands** | None · Hold Fire |
| **Production** | Dispatched from `UN-BLD-15` (1 per civilian-casualty incident, up to 3 active per district) |
| **Veterancy** | none |
| **AI** | `ai.unit.civilian_ambient` (emergency variant): drives **toward** incidents after combat within 600 cm has stopped for 5 s; flees if fire resumes |
| **Multiplayer** | MPA-NPC |
| **Presentation** | ANIM-CIV + light bar. Siren audio; icon of a red cross |

**Weapons:** none (unarmed)

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.un.civ_06.casualty_response` | Automated (P) | Restores up to 50 % of the Population Morale lost to the incident in that district over 30 s; heals Unaligned infantry within 400 cm by 6 HP/s (no member replenishment) | Incident site | free · 30 s | Continuing combat delays it; destroying it (R3 incident) | Red-cross marker | After fire stops |

**Research and upgrades:** none.

**Counters:** *Beaten by (hard):* anything armed; attacking it is the worst civilian incident (R3 immediately). *Situational:* a visible signal that an incident is being closed out; the response tier decays faster after it completes (P).

**Status:** identity E-R; morale effects and values P.

### UN-CIV-07 — Civilian Fire Engine

| Field | Specification |
| --- | --- |
| **Identity** | `unit.un.civ_07` · The Unaligned · Civilian / fire and rescue · `TPL-CIV` (WHEEL_HEAVY) · **Extended** · incident-dispatched |
| **Role** | **Primary:** emergency building damage response; no combat (E-R). **Secondary:** extinguishes burning zones and clears abandoned vehicles. **Doctrine fit:** civil protection (E). |
| **Inspiration and background** | Red municipal fire engines with ladders and water tanks. C&C lineage (E-R): Mobile Repair Vehicle (TS); Rig (TW, recovery role). |
| **Visual identity** | Concept: none (asset requirement). Red truck, ladder, hose reel; water jet VFX. |
| **Statistics** | **400 HP** (`armor.veh_soft`) · WHEEL_HEAVY 400 cm/s · sight 900 cm · F2 · Morale 120 |
| **Commands** | None · Hold Fire |
| **Production** | Dispatched from a municipal fire station (authored map marker on `UN-BLD-03`), 1 per burning Unaligned or civilian structure, up to 2 per district |
| **Veterancy** | none |
| **AI** | `ai.unit.civilian_ambient` (emergency variant) as UN-CIV-06 |
| **Multiplayer** | MPA-NPC |
| **Presentation** | ANIM-CIV + ladder, hose. Siren audio |

**Weapons:** none (unarmed)

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.un.civ_07.extinguish` | Active (system) (P) | Ends burning on one structure or burning zone within 600 cm after 8 s; then repairs that **Unaligned or civilian** structure at 10 HP/s up to 50 % HP. Never repairs participant buildings | Unaligned/civilian structure or zone | free · 8 s | Continuing combat | Water jet | After fire stops |
| `ability.un.civ_07.clear_lane` | Automated (P) | Tows an abandoned civilian vehicle off the road in 10 s | Abandoned vehicle | free · 10 s | — | — | When blocking a lane |

**Research and upgrades:** none.

**Counters:** *Beaten by (hard):* anything armed (R3 incident). *Situational:* restores federal infrastructure that incendiary weapons (IA-INF-05, RD-ARM-10) set alight.

**Status:** identity E-R; extinguish, repair cap and values P.

### UN-CIV-08 — Electrical Maintenance Van

| Field | Specification |
| --- | --- |
| **Identity** | `unit.un.civ_08` · The Unaligned · Civilian / grid maintenance · `TPL-CIV` (WHEEL_LIGHT) · **Extended** · incident-dispatched |
| **Role** | **Primary:** repair power lines, substations and street outages (E-R). **Secondary:** restores Unaligned grid nodes after sabotage. **Doctrine fit:** the electricity network as owned, restorable infrastructure (E). |
| **Inspiration and background** | Utility vans of the electricity distribution enterprise with a cherry-picker. C&C lineage (E-R): Repair Facility (TD, repair concept); Engineer (RA1); Spanner (TT). |
| **Visual identity** | Concept: none (asset requirement). Orange-striped van with a folded boom; lightning-bolt door logo. |
| **Statistics** | **150 HP** (`armor.veh_soft`) · WHEEL_LIGHT 480 cm/s · sight 800 cm · F1 · Morale 100 |
| **Commands** | None · Hold Fire |
| **Production** | Dispatched from `UN-BLD-10` Electric Power Station or `UN-BLD-11` Federal Substation, 1 per damaged grid node, up to 2 per district |
| **Veterancy** | none |
| **AI** | `ai.unit.civilian_ambient` (emergency variant) |
| **Multiplayer** | MPA-NPC |
| **Presentation** | ANIM-CIV + boom raise. Sparks VFX during repair |

**Weapons:** none (unarmed)

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.un.civ_08.restore_line` | Active (system) (P) | Repairs one **Unaligned** grid node (`UN-BLD-11`, power line segment) at 15 HP/s and restores its connection when above 50 % HP. Participant grid connections that pass through Unaligned nodes recover with it | Unaligned grid node within 300 cm | free · until repaired | Kill it (incident); repeat the sabotage | Boom and sparks | Highest-load node first |

**Research and upgrades:** none.

**Counters:** *Beaten by (hard):* anything armed. *Situational:* the counterplay to grid sabotage; makes power cuts on federal nodes temporary.

**Status:** identity E-R; grid ownership and restoration E; repair values P.

### UN-CIV-09 — Postal and Press Delivery Van

| Field | Specification |
| --- | --- |
| **Identity** | `unit.un.civ_09` · The Unaligned · Civilian / press distribution · `TPL-CIV` (WHEEL_LIGHT) · **Extended** · scheduled |
| **Role** | **Primary:** newspaper dissemination, observable and disruptable (E-R). **Secondary:** carries mail for scenario objectives. **Doctrine fit:** the NPC-only Unaligned media apparatus; newspapers depend on distribution routes (E). |
| **Inspiration and background** | Yellow PTT vans and newspaper delivery vans leaving the printing works at dawn. C&C lineage (E-R): Supply Truck (Gen); Technical (Gen, carrier role). |
| **Visual identity** | Concept: none (asset requirement). Yellow PTT van; bundles of newspapers on the roof rack. |
| **Statistics** | **130 HP** (`armor.veh_soft`) · WHEEL_LIGHT 500 cm/s (road-locked) · sight 800 cm · F1 · Morale 100 |
| **Commands** | None · Hold Fire |
| **Production** | Scheduled from `UN-SEC-08` Federal Printing Works, 1 per 150 s per newspaper route |
| **Veterancy** | none |
| **AI** | `ai.unit.civilian_ambient` |
| **Multiplayer** | MPA-NPC. Whether a van carries a false-report edition is a **truth flag that is never replicated** (E: false-report truth flags are hidden) |
| **Presentation** | ANIM-CIV. Bundle drop animation at kiosks |

**Weapons:** none (unarmed)

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.un.civ_09.distribution` | Automated (P; E: newspapers require distribution) | Each delivered route completes the Unaligned newspaper reach for its district for 300 s; carries UN-INF-13 continuity messaging and, when the Deception Bureau runs a press operation, the false report as content (never changing ground truth, E) | Kiosks on the route | — | Interdiction (incident); counterintelligence corroboration exposes the false content | none | — |

**Research and upgrades:** none.

**Counters:** *Beaten by (hard):* anything armed. *Situational:* stopping a delivery limits the reach of a planted press story, but it is an incident and visible as such.

**Status:** identity and media dependency E-R/E; reach values P.

### UN-CIV-10 — Agricultural Tractor

| Field | Specification |
| --- | --- |
| **Identity** | `unit.un.civ_10` · The Unaligned · Civilian / farm vehicle · `TPL-CIV` (WHEEL_HEAVY, off-road) · **Extended** · ambient |
| **Role** | **Primary:** rural economy and market goods; non-military (E-R). **Secondary:** slow road traffic that delays convoys on rural lanes. **Doctrine fit:** agricultural cooperatives and food supply (E: food reserves). |
| **Inspiration and background** | Small domestic tractors with trailers on country roads and fields. C&C lineage (E-R): Worker (Gen, work role); Ore Collector (RA3, industry role). |
| **Visual identity** | Concept: none (asset requirement). Red or blue tractor, two-wheel trailer with hay or crates. |
| **Statistics** | **180 HP** (`armor.veh_soft`) · WHEEL_HEAVY **180 cm/s** · sight 700 cm · F1 · Morale 100 |
| **Commands** | None · Hold Fire |
| **Production** | Ambient: 1–3 per authored farm district |
| **Veterancy** | none |
| **AI** | `ai.unit.civilian_ambient`. Deltas: field loops; occasional road trips to market |
| **Multiplayer** | MPA-NPC |
| **Presentation** | ANIM-CIV. Engine-chug audio |

**Weapons:** none (unarmed)

**Abilities:** none unit-specific (`ability.un.civ.traffic_witness` applies).

**Research and upgrades:** none.

**Counters:** *Beaten by (hard):* anything armed. *Situational:* rural witness and road obstacle; harming it is a civilian incident like any other.

**Status:** identity E-R; values P.

### UN-CIV-11 — Commercial Fishing Boat

| Field | Specification |
| --- | --- |
| **Identity** | `unit.un.civ_11` · The Unaligned · Civilian / fishing vessel · `TPL-CIV` (SURFACE_SHALLOW) · **Conditional** (water maps) · scenario-only ambient |
| **Role** | **Primary:** neutral regional water economy; scenario-only (E-R). **Secondary:** cover traffic for maritime smuggling and covert landings. **Doctrine fit:** the Adriatic coastal economy (E). |
| **Inspiration and background** | Wooden trawlers and purse-seiners from Dalmatian harbours. C&C lineage (E-R): Gunboat (RA1, small hull only); Yari Mini-Sub (RA3, small hull analogue). |
| **Visual identity** | Concept: none (asset requirement). White-and-blue wooden hull, wheelhouse, nets and gulls. |
| **Statistics** | **160 HP** (`armor.nav_light`) · SURFACE_SHALLOW 360 cm/s · sight 900 cm · FN 300 cm · Morale 100 |
| **Commands** | None · Hold Fire |
| **Production** | Ambient: 1–4 per authored fishing ground; returns to a harbour (`UN-BLD-13`) at R3+ |
| **Veterancy** | none |
| **AI** | `ai.unit.civilian_ambient` (naval) |
| **Multiplayer** | MPA-NPC |
| **Presentation** | ANIM-NAV (small wake, nets). Foghorn audio |

**Weapons:** none (unarmed)

**Abilities:** none unit-specific (`ability.un.civ.traffic_witness` applies at sea).

**Research and upgrades:** none.

**Counters:** *Beaten by (hard):* any naval or air weapon. *Situational:* maritime witnesses; their reports feed UN-AIR-08 and UN-SEA-01 tasking.

**Status:** identity and scenario-only E-R; values P.

### UN-CIV-12 — River Ferry

| Field | Specification |
| --- | --- |
| **Identity** | `unit.un.civ_12` · The Unaligned · Civilian / river crossing · `TPL-CIV` (SURFACE_SHALLOW, fixed route) · **Conditional** (river maps) · scheduled |
| **Role** | **Primary:** realistic civilian crossing that NPC jurisdiction can close (E-R). **Secondary:** crossing for **any** participant's ground units where authored agreements allow. **Doctrine fit:** crossing is not authorization (E). |
| **Inspiration and background** | Flat cable or motor ferries on the Sava, Drava and Danube. C&C lineage (E-R): Transport Ship (RA1); Amphibious Transport (RA2). |
| **Visual identity** | Concept: none (asset requirement). Flat deck with ramps at both ends, small wheelhouse, guide cable. |
| **Statistics** | **500 HP** (`armor.nav_light`) · SURFACE_SHALLOW 200 cm/s on its fixed route · FN 600 cm · **deck capacity 4 vehicle slots (F1–F3) or 8 infantry squads** · crossing time ≈ 25 s |
| **Commands** | None for the ferry. Participants **request passage** by ordering a unit to the ramp (a Move order that ends on the ferry marker), never by commanding the ferry |
| **Production** | Fixed map entity at authored crossings; respawned by the institutional AI 180 s after destruction if Population Morale ≥ 40 |
| **Veterancy** | none |
| **AI** | `ai.unit.civilian_ambient`. Deltas: shuttles while open; stops at the near bank when closed |
| **Multiplayer** | MPA-NPC + MPA-TRANSPORT (deck manifest visible to observers who can see the ferry; it is an open deck) |
| **Presentation** | ANIM-NAV + ramp. Ramp-gate light (green open, red closed) |

**Weapons:** none (unarmed)

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.un.civ_12.jurisdiction_closure` | Automated (P; E: crossings can be closed) | At R1+ for a participant, that participant's units are refused at the ramp (typed `OrderRejection` "crossing closed"); at R3 in the district the ferry stops for everyone | Per participant | — | Keep the response tier low; bridging equipment; amphibious units | Ramp light and rejection message | Institutional AI |
| `ability.un.civ_12.passage` | Automated (P) | Carries eligible units across; units on deck are unable to fire; if sunk, vehicles aboard are destroyed and infantry lose 50 % HP and swim ashore | Eligible units | free · ≈ 25 s crossing | Attack the ferry mid-river (severe incident) | Deck manifest | — |

**Research and upgrades:** none.

**Counters:** *Beaten by (hard):* anything armed. *Situational:* a contested crossing whose availability depends on how lawfully each participant behaves; amphibious units (RD-SEA-02, AC-SEA-02, MP-SEA-04) and engineer bridging bypass it.

**Status:** identity and jurisdiction closure E-R/E; passage mechanics and values P.

## 6. Air force, civil aviation and surveillance

The roster gives the Unaligned **no airfield building** (UN-BLD and UN-SEC rows contain radar, SAM and civil infrastructure only). Military aircraft therefore **enter from authored off-map air corridors** and leave the same way; they do not land, rearm or occupy pads on the map (P; **U-UN-03**). One sortie is one mobilization spend. Civil flights use authored airport markers or map-edge corridors. Military aircraft are released only at their response tier and only against the participant whose violation triggered it (E). `UN-SEC-05` Air Defense Radar identifies aircraft entering protected airspace and is the usual trigger for UN-AIR-01 (E-R). Kerosene is abstracted into the mobilization weight (P).

### UN-AIR-01 — JNA Interceptor Fighter

| Field | Specification |
| --- | --- |
| **Identity** | `unit.un.air_01` · The Unaligned · Air / interceptor · `TPL-NPC-SECURITY` (as `TPL-AIR-FIGHTER`) · **Core** · NPC-only |
| **Role** | **Primary:** defend sovereign airspace from unauthorized intrusions (E-R). **Secondary:** escorting UN-AIR-02 at R4. **Doctrine fit:** sovereign airspace defense of the JNA air force (E). |
| **Inspiration and background** | A pair of delta-wing interceptors scrambled from a federal air base. C&C lineage (E-R): MiG (RA1); Apollo Fighter (RA3); Firehawk (TW). |
| **Visual identity** | Concept: none (asset requirement). Natural-metal or grey delta-wing jets, red-star roundels, Yugoslav fin flash. |
| **Statistics** | Flight of 2 × **360 HP** (`armor.air_fixed`) · FIXED_WING **2,500 cm/s** · sight 2200 cm (air) · ExperienceValue 150 per aircraft |
| **Commands** | Institutional authority only: Intercept (Attack), Escort, Return off-map · Aggressive (air) |
| **Production** | Mobilization weight **1,800 per flight** · response tier **R1** to shadow and warn, **R2** to engage aircraft of a participant that ignored a warning, **R4** for general engagement · off-map corridor · 30 s from trigger to arrival |
| **Veterancy** | Standard (per sortie only; aircraft leave after the sortie) |
| **AI** | `ai.unit.air_superiority`. Deltas: *shadow-and-warn* first (fly alongside for 10 s, warning to the intruding participant); engage only if the intruder stays in protected airspace; never chase beyond federal borders |
| **Multiplayer** | MPA-NPC + MPA-AIR (warning delivered only to the intruding participant) |
| **Presentation** | ANIM-AIR-FIXED + afterburner. Warning voice line on the intruder's radio channel; icon of a delta wing with a red star |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.un.air_01.aam` short-range air-to-air missiles | FLAK_AA | 0–1300 cm | 160 / 4.0 s; magazine 2 per aircraft | guided, flares (`ability` of the target) can break lock | — | AIR_HIGH, AIR_LOW | — / — / — / 110 % rotary, 100 % fixed / — |
| `weapon.un.air_01.cannon` | AUTOCANNON | 0–700 cm | 25 / 0.3 s; 6 s of fire | projectile | — | AIR_HIGH, AIR_LOW | — / — / — / 100 % rotary, 60 % fixed / — |

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.un.air_01.shadow_and_warn` | Automated (P; E: graduated response) | Flies alongside the intruder for 10 s; the intruding participant receives a warning and a 15 s window to leave protected airspace before R2 engagement | Intruding aircraft | free · 10 s + 15 s | Leave; fly below `UN-SEC-05` coverage; agreements | Warning to the intruder only | Always before firing, except at R4 |

**Research and upgrades:** none. Scenario flag `un.air_alert` (P): arrival time 30 → 20 s.

**Counters:** *Beats (hard):* transports and surveillance aircraft (AC-AIR-06/07, RD-AIR-05/06, MP-AIR-04/05, IA-AIR-08). *Beats (soft):* strike jets. *Beaten by (hard):* participant interceptors (AC-AIR-01, RD-AIR-01, IA-AIR-01) and SAMs (RD-ARM-13). *Situational:* makes unauthorized flights over federal territory costly; low flight below radar is the intended answer.

**Status:** identity E-R; graduated response E; warning window and values P.

### UN-AIR-02 — JNA Ground-Attack Jet

| Field | Specification |
| --- | --- |
| **Identity** | `unit.un.air_02` · The Unaligned · Air / ground-attack jet · `TPL-NPC-SECURITY` (as `TPL-AIR-STRIKE`) · **Extended** · NPC-only |
| **Role** | **Primary:** conventional air response when sovereignty is violated (E-R). **Secondary:** attacking columns that attack JNA sites. **Doctrine fit:** JNA close air support at R4 (E). |
| **Inspiration and background** | Domestic light attack jets with rocket pods and bombs. C&C lineage (E-R): Yak (RA1); Vindicator (RA3); Harrier (RA2). |
| **Visual identity** | Concept: none (asset requirement). Camouflaged twin-engine light attack jets, underwing pods. |
| **Statistics** | Flight of 2 × **420 HP** (`armor.air_fixed`) · FIXED_WING **1,900 cm/s** · sight 1800 cm · ExperienceValue 160 per aircraft |
| **Commands** | Institutional authority only: Strike (one target area), Return off-map · Aggressive |
| **Production** | Mobilization weight **2,200 per flight** · response tier **R4** · off-map corridor · 45 s to arrival · at most 1 flight per 120 s |
| **Veterancy** | Standard (per sortie) |
| **AI** | `ai.unit.air_strike`. Deltas: target only units of the violating participant; **never** release on a target within 800 cm of a civilian unit or civilian structure (E: no indiscriminate attacks; P radius) |
| **Multiplayer** | MPA-NPC + MPA-AIR |
| **Presentation** | ANIM-AIR-FIXED + weapon release. Jet roar audio before arrival (a 3 s public tell) |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.un.air_02.rockets` | HIGH_EXPLOSIVE | 0–900 cm (dive) | 6 × 50, radius 150 cm, one pass | projectile, unguided, seeded scatter 100 cm | 10 | GROUND, STRUCTURE | 110 % / 45 % / 12 % / — / 100 % |
| `weapon.un.air_02.bombs` | HIGH_EXPLOSIVE | overflight | 2 × 220, radius 300 cm, one pass | ballistic, seeded scatter 150 cm | 15 | GROUND, STRUCTURE | 110 % / 45 % / 12 % / — / 100 % |

**Abilities:** none unit-specific (strike rules are in the AI deltas).

**Research and upgrades:** none.

**Counters:** *Beats (hard):* massed infantry and soft columns in the open. *Beats (soft):* light armor. *Beaten by (hard):* mobile AA (AC-ARM-09, RD-ARM-06/13, IA-ARM-08, MP-ARM-09) and interceptors. *Situational:* the clearest R4 consequence; escort AA protects columns that may provoke the federation.

**Status:** identity E-R; R4 gating and civilian restriction E; values P.

### UN-AIR-03 — Federal Transport Helicopter

| Field | Specification |
| --- | --- |
| **Identity** | `unit.un.air_03` · The Unaligned · Air / transport helicopter · `TPL-NPC-SECURITY` (as `TPL-AIR-HELI-TRANSPORT`) · **Extended** · NPC-only |
| **Role** | **Primary:** military personnel and evacuation logistics (E-R). **Secondary:** inserting UN-INF-09 patrols and engineers on mountain sites. **Doctrine fit:** air mobility for the JNA and civil protection (E). |
| **Inspiration and background** | Medium transport helicopters in JNA olive with red-cross panels when used for evacuation. C&C lineage (E-R): Chinook (TD); Transport Helicopter (Ren); Twinblade (RA3, lift role). |
| **Visual identity** | Concept: none (asset requirement). Olive medium helicopter; removable red-cross door panels in evacuation mode. |
| **Statistics** | **560 HP** (`armor.air_rotary`) · ROTARY 680 cm/s · sight 1400 cm · **3 infantry slots or 3 civilian groups** · no ranks |
| **Commands** | Institutional authority only: Move, Land, Load, Unload, Return · Hold Fire |
| **Production** | Mobilization weight **1,000** · response tier **R2** (evacuation) / **R3** (troop lift) · off-map corridor or `UN-BLD-05` helipad marker · 20 s |
| **Veterancy** | none |
| **AI** | `ai.unit.air_transport`. Deltas: land only outside active weapon ranges; abort if an AA lock occurs |
| **Multiplayer** | MPA-NPC + MPA-AIR + MPA-TRANSPORT |
| **Presentation** | ANIM-AIR-ROTARY + doors. Rotor audio; icon of a helicopter with a red star |

**Weapons:** none (unarmed)

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.un.air_03.medevac` | Automated (P) | In evacuation mode, carries civilian groups like UN-CIV-02 (same morale protection) and Unaligned infantry casualties to `UN-BLD-15` | Incident site | free · 8 s load | AA; attacking it is an R3 incident | Red-cross panels | At R2+ when roads are blocked |

**Research and upgrades:** none.

**Counters:** *Beaten by (hard):* any AA or autocannon. *Situational:* rapid federal reinforcement of mountain passes and islands.

**Status:** identity E-R; values P.

### UN-AIR-04 — Army Attack Helicopter

| Field | Specification |
| --- | --- |
| **Identity** | `unit.un.air_04` · The Unaligned · Air / attack helicopter · `TPL-NPC-SECURITY` (as `TPL-AIR-HELI-ATTACK`) · **Extended** · NPC-only |
| **Role** | **Primary:** JNA rotary close support at heavy escalation (E-R). **Secondary:** hunting raiders near protected installations. **Doctrine fit:** JNA army aviation (E). |
| **Inspiration and background** | Light attack helicopters with ATGM rails and rocket pods. C&C lineage (E-R): Hind (RA1); Apache (Ren); Orca (TW, attack role). |
| **Visual identity** | Concept: none (asset requirement). Small olive helicopter, stub wings with four missile tubes and a rocket pod. |
| **Statistics** | **460 HP** (`armor.air_rotary`) · ROTARY 760 cm/s · sight 1600 cm · ExperienceValue 120 |
| **Commands** | Institutional authority only: Attack, AttackMove, Return · Aggressive |
| **Production** | Mobilization weight **1,500** · response tier **R4** · off-map corridor · 25 s · at most 2 active |
| **Veterancy** | Standard (per sortie) |
| **AI** | `ai.unit.attack_helicopter`. Deltas: violating participant only; break off at 40 % HP or on an AA lock |
| **Multiplayer** | MPA-NPC + MPA-AIR |
| **Presentation** | ANIM-AIR-ROTARY + missile launch |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.un.air_04.atgm` | SHAPED_CHARGE | 0–1200 cm | 200 / 5.0 s; magazine 4 | guided (wire/command; smoke blocks) | 3 | GROUND | 25 % / 100 % / 90 % / — / 70 % |
| `weapon.un.air_04.rockets` | HIGH_EXPLOSIVE | 0–1000 cm | 4 × 40, radius 150 cm / 6 s; magazine 3 salvos | projectile, unguided | 8 | GROUND, STRUCTURE | 110 % / 45 % / 12 % / — / 100 % |

**Abilities:** none unit-specific.

**Research and upgrades:** none.

**Counters:** *Beats (hard):* armor without AA cover (RD-ARM-04, IA-ARM-03, MP-ARM-06/07). *Beaten by (hard):* mobile AA (AC-ARM-09, RD-ARM-06, IA-ARM-08, MP-ARM-09), autocannon. *Situational:* punishes armor sent into federal installations.

**Status:** identity E-R; values P.

### UN-AIR-05 — Federal Reconnaissance Aircraft

| Field | Specification |
| --- | --- |
| **Identity** | `unit.un.air_05` · The Unaligned · Air / reconnaissance aircraft · `TPL-NPC-SECURITY` (as `TPL-AIR-SCOUT`) · **Extended** · NPC-only |
| **Role** | **Primary:** monitor borders, mountain passes and maritime coastlines (E-R). **Secondary:** confirming incident attribution with photographs. **Doctrine fit:** federal surveillance of sovereign territory (E). |
| **Inspiration and background** | A twin-engine reconnaissance jet with camera pods. C&C lineage (E-R): Spy Plane (RA1); Surveillance Sweep (RA3); Recon Drone (TT). |
| **Visual identity** | Concept: none (asset requirement). Grey jet with a camera nose. |
| **Statistics** | **340 HP** (`armor.air_fixed`) · FIXED_WING 2,000 cm/s · sight **2600 cm** · `DETECT.CONCEALED` 900 cm along its track |
| **Commands** | Institutional authority only: Reconnoitre route, Return · Hold Fire |
| **Production** | Mobilization weight **900** · response tier **R1** · off-map corridor · 30 s · 1 per 180 s |
| **Veterancy** | none |
| **AI** | `ai.unit.air_scout`. Deltas: one pass along a border, pass or coast segment where an incident was reported |
| **Multiplayer** | MPA-NPC + MPA-AIR. What it sees goes to the institutional AI only; it never shares vision with participants (P) |
| **Presentation** | ANIM-AIR-FIXED. Camera-shutter audio |

**Weapons:** none (unarmed)

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.un.air_05.attribution_photo` | Automated (P) | A pass over an incident site within 120 s of the incident **corroborates** attribution: forged attribution older than its expiry is not reinforced; a correct attribution becomes certain and cannot be overturned by later false reports (E: corroboration constrains deception) | Incident site | free · one pass | AA; act and leave quickly; concealment | none | Fly to the freshest incident |

**Research and upgrades:** none.

**Counters:** *Beaten by (hard):* interceptors and SAMs (AC-AIR-01, RD-ARM-13). *Situational:* shortens the window in which covert actions remain deniable.

**Status:** identity E-R; corroboration rule E; mechanics and values P.

### UN-AIR-06 — Civil Passenger Airliner

| Field | Specification |
| --- | --- |
| **Identity** | `unit.un.air_06` · The Unaligned · Civilian / airliner · `TPL-CIV` (FIXED_WING) · **Scenario** · timetabled |
| **Role** | **Primary:** timetabled neutral flights; civilian, not a recruitable attack aircraft (E-R). **Secondary:** a background sign of normality and a strict incident trigger. **Doctrine fit:** the federal civil airline (E). |
| **Inspiration and background** | Twin-jet airliners of the national flag carrier. C&C lineage (E-R): Century Bomber (RA3, large airframe only); V-35 Ox (TW, flight behavior). |
| **Visual identity** | Concept: none (asset requirement). White fuselage with a blue-and-red cheatline; high-altitude contrails. |
| **Statistics** | **900 HP** (`armor.air_fixed`) · FIXED_WING 1,800 cm/s at AIR_HIGH on authored corridors · sight 800 cm · no weapons |
| **Commands** | None (timetable) |
| **Production** | Timetable 1 per 300 s per authored corridor; suspended over a district at R4 |
| **Veterancy** | none |
| **AI** | `ai.unit.civilian_ambient` (air) |
| **Multiplayer** | MPA-NPC |
| **Presentation** | ANIM-AIR-FIXED (cruise only). Tooltip "civil flight" |

**Weapons:** none (unarmed)

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.un.air_06.civil_flight_protection` | Passive (P; E: civilians are never automatically valid targets) | Never auto-acquired by any AA weapon or SAM; an explicit attack order requires the civilian-incident confirmation (U-UN-01). Downing it is an **immediate R4** incident for the attacker | — | — | — | "Civil flight" label | — |

**Research and upgrades:** none.

**Counters:** none relevant; it is protected world texture. *Situational:* civil corridors mark where friendly-fire mistakes would be catastrophic.

**Status:** identity and civilian status E-R/E; protection mechanics and values P.

### UN-AIR-07 — Emergency Airlift Plane

| Field | Specification |
| --- | --- |
| **Identity** | `unit.un.air_07` · The Unaligned · Civilian-military / airlift · `TPL-NPC-SECURITY` (as `TPL-AIR-TRANSPORT`) · **Scenario** · mission-scripted |
| **Role** | **Primary:** evacuate civilians and deliver food; mission scripted, not base-trained (E-R). **Secondary:** relieving besieged federal sites in scenarios. **Doctrine fit:** civil protection and food reserves (E). |
| **Inspiration and background** | Twin-turboprop military transports used for disaster relief. C&C lineage (E-R): V-35 Ox (TW); Century Bomber (RA3, transport capacity). |
| **Visual identity** | Concept: none (asset requirement). Olive high-wing transport with red-cross markings in relief missions. |
| **Statistics** | **800 HP** (`armor.air_fixed`) · FIXED_WING 1,500 cm/s · sight 1400 cm · capacity 6 civilian groups or 30 food units |
| **Commands** | None (scenario script) |
| **Production** | Scenario trigger only; landing at an authored strip or airdrop of food pallets |
| **Veterancy** | none |
| **AI** | `ai.unit.air_transport` (scripted) |
| **Multiplayer** | MPA-NPC + MPA-AIR |
| **Presentation** | ANIM-AIR-FIXED + cargo ramp, parachute pallets |

**Weapons:** none (unarmed)

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.un.air_07.relief_drop` | Scripted (P) | Delivers food units to an Unaligned site or evacuates civilian groups; relief food is never capturable by participants (P, so scenarios cannot become a food farm, E: no excessive resource farming) | Scenario target | scripted | AA (immediate R4 incident) | Parachute VFX | Scripted |

**Research and upgrades:** none.

**Counters:** none relevant. *Situational:* a scenario objective (protect or escort).

**Status:** identity E-R; scripted mechanics and values P.

### UN-AIR-08 — Adriatic Maritime Patrol Plane

| Field | Specification |
| --- | --- |
| **Identity** | `unit.un.air_08` · The Unaligned · Air / maritime patrol · `TPL-NPC-SECURITY` (as `TPL-AIR-SURVEILLANCE`) · **Conditional** (sea maps) · NPC-only |
| **Role** | **Primary:** identify shipping, foreign landings and offshore incursions (E-R). **Secondary:** cueing UN-SEA-02/04 against hostile ships and submarines. **Doctrine fit:** defense of federal waters (E). |
| **Inspiration and background** | A turboprop patrol aircraft with surface radar and a sonobuoy rack. C&C lineage (E-R): ORCA Fighter (TS, observation role); Spy Plane (RA1); Orca (TT, recon role). |
| **Visual identity** | Concept: none (asset requirement). Grey high-wing turboprop with a belly radome. |
| **Statistics** | **520 HP** (`armor.air_fixed`) · FIXED_WING 1,400 cm/s (loiter orbit) · sight 3200 cm (surface) · `DETECT.SUBMERGED` 1200 cm (sonobuoys) |
| **Commands** | Institutional authority only: Orbit, Return · Hold Fire |
| **Production** | Mobilization weight **1,200** · response tier **R0** (routine patrol, 1 per 300 s) and **R2** (incident search) · off-map corridor |
| **Veterancy** | none |
| **AI** | `ai.unit.air_surveillance`. Deltas: orbit the incident sea area for 90 s; retreat on interceptor contact |
| **Multiplayer** | MPA-NPC + MPA-AIR + MPA-SENSOR (contacts go to the institutional AI only) |
| **Presentation** | ANIM-AIR-FIXED + radome. Sonobuoy splash VFX |

**Weapons:** none (unarmed)

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.un.air_08.sonobuoy_line` | Active (P) | Drops a line of 4 buoys (each `DETECT.SUBMERGED` 1200 cm, 90 s) | Water area | free · 90 s · 120 s | Destroy buoys (any surface weapon); run deep and slow | Buoy markers (none to players) | Where a submarine was last reported |

**Research and upgrades:** none.

**Counters:** *Beats (hard, as a sensor):* submarines and covert landings (AC-SEA-05, RD-SEA-04, IA-SEA-04, MP-SEA-05, IA-SEA-03). *Beaten by (hard):* interceptors. *Situational:* makes uncontested naval movement in federal waters short-lived.

**Status:** identity E-R; values P.

## 7. Coastal, riverine and naval forces

Naval units exist only on maps with an authored water layer (E-R: naval play is map-restricted). They spawn at `UN-BLD-13` Harbor Authority or a scenario naval-base marker. Military vessels follow the same response tiers and per-participant hostility as land units (E). Crews are abstracted into the mobilization weight (P).

### UN-SEA-01 — JNA Coastal Patrol Boat

| Field | Specification |
| --- | --- |
| **Identity** | `unit.un.sea_01` · The Unaligned · Naval / patrol boat · `TPL-NPC-SECURITY` (as `TPL-NAV-PATROL`) · **Conditional** · NPC-only |
| **Role** | **Primary:** routine sea and port inspection (E-R). **Secondary:** first response to maritime incidents. **Doctrine fit:** the naval tripwire of the graduated response (E). |
| **Inspiration and background** | A small JNA navy patrol boat with a twin cannon. C&C lineage (E-R): Gunboat (RA1); Gunboat (TD); Yari Mini-Sub (RA3, small craft role). |
| **Visual identity** | Concept: none (asset requirement). Grey hull, pennant number, twin gun mount forward. |
| **Statistics** | **400 HP** (`armor.nav_light`) · SURFACE_SHALLOW 620 cm/s · sight 1600 cm · `DETECT.SPY` 400 cm (boarding party) · ExperienceValue 80 |
| **Commands** | Institutional authority only: Patrol, Inspect, Attack, Guard · default stance Guard |
| **Production** | Mobilization weight **700** · response tier **R0** (patrol) / **R2** (armed response) · `UN-BLD-13` · 14 s |
| **Veterancy** | Standard |
| **AI** | `ai.unit.naval_patrol` + `ai.unit.npc_security` tier rules |
| **Multiplayer** | MPA-NPC |
| **Presentation** | ANIM-NAV + turret. Loud-hailer audio during inspections |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.un.sea_01.twin_cannon` | AUTOCANNON | 0–850 cm | 20 / 0.4 s | projectile | 4 | WATER_SURFACE, GROUND (shore), AIR_LOW | 65 % / 85 % / 10 % / 100 % rotary / 30 % (NAV_L 90 %) |

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.un.sea_01.maritime_inspection` | Active (P) | As UN-INF-04 *inspection*, for vessels within 400 cm: reveals disguised agents, falsified manifests (MP-SUP-06 cargo shipped by sea, MP-SEA-01) and military cargo without agreements (R1 incident) | Vessel | free · 6 s · 20 s | Avoid patrol lanes; outrun it (MP-SEA-02) | Inspection icon to the inspected participant | R0–R1 on unknown vessels |

**Research and upgrades:** none.

**Counters:** *Beats (hard):* smuggling and courier vessels (MP-SEA-01, IA-SEA-03 when caught). *Beaten by (hard):* missile boats and frigates (AC-SEA-03/04, MP-SEA-06), attack aircraft. *Situational:* the naval equivalent of the border post.

**Status:** identity E-R; values P.

### UN-SEA-02 — Federal Missile Patrol Boat

| Field | Specification |
| --- | --- |
| **Identity** | `unit.un.sea_02` · The Unaligned · Naval / missile boat · `TPL-NPC-SECURITY` (as `TPL-NAV-COMBATANT`, NAV_LIGHT) · **Conditional** · NPC-only |
| **Role** | **Primary:** coastal anti-surface missile response (E-R). **Secondary:** escort of amphibious transports at R4. **Doctrine fit:** defense of federal waters (E). |
| **Inspiration and background** | Fast missile boats with two anti-ship missile containers. C&C lineage (E-R): Sea Scorpion (RA2); Destroyer (RA1); Naginata Cruiser (RA3, role). |
| **Visual identity** | Concept: none (asset requirement). Low grey hull, two box launchers aft. |
| **Statistics** | **520 HP** (`armor.nav_light`) · SURFACE_SHALLOW 680 cm/s · sight 2000 cm (radar) · ExperienceValue 110 |
| **Commands** | Institutional authority only · Aggressive |
| **Production** | Mobilization weight **1,300** · response tier **R4** · `UN-BLD-13` · 20 s |
| **Veterancy** | Standard |
| **AI** | `ai.unit.naval_combatant`. Deltas: fire from maximum range then withdraw to reload at harbor |
| **Multiplayer** | MPA-NPC + MPA-SENSOR |
| **Presentation** | ANIM-NAV + launcher elevate. Missile trail VFX |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.un.sea_02.ssm` | ANTI_SHIP | 0–2600 cm | 380 / 12 s; magazine 2, reload at `UN-BLD-13` | guided, sea-skimming; can be shot down by ship AA | — | WATER_SURFACE | — / — / — / — / 60 % (NAV_L 120 %, NAV_H 100 %) |
| `weapon.un.sea_02.gun` | AUTOCANNON | 0–850 cm | 18 / 0.4 s | projectile | 3 | WATER_SURFACE, AIR_LOW | (NAV_L 90 %) / 100 % rotary |

**Abilities:** none unit-specific.

**Research and upgrades:** none.

**Counters:** *Beats (hard):* surface ships of a violating participant (AC-SEA-04/06, RD-SEA-05/06, IA-SEA-05, MP-SEA-06). *Beaten by (hard):* aircraft, submarines (torpedo). *Situational:* two missiles then a long trip home; baiting the salvo is the counterplay.

**Status:** identity E-R; values P.

### UN-SEA-03 — Adriatic Frigate Escort

| Field | Specification |
| --- | --- |
| **Identity** | `unit.un.sea_03` · The Unaligned · Naval / frigate · `TPL-NPC-SECURITY` (as `TPL-NAV-COMBATANT`) · **Conditional** · NPC-only |
| **Role** | **Primary:** protect shipping and defend federal waters (E-R). **Secondary:** naval air defense and anti-submarine screen. **Doctrine fit:** the largest federal warship on the map (E). |
| **Inspiration and background** | A light frigate with a gun, SAM launcher and ASW rockets. C&C lineage (E-R): Destroyer (RA1); Aegis Cruiser (RA2, AA function). |
| **Visual identity** | Concept: none (asset requirement). Grey frigate, single funnel, pennant numbers, Yugoslav naval ensign. |
| **Statistics** | **1,900 HP** (`armor.nav_heavy`) · SURFACE_DEEP 340 cm/s · sight 2000 cm; air radar 3000 cm (`DETECT.AIR_RADAR`); sonar `DETECT.SUBMERGED` 1200 cm · ExperienceValue 300 |
| **Commands** | Institutional authority only · Guard (escort) |
| **Production** | Mobilization weight **2,800** · response tier **R4** (R0 for escort of civilian shipping) · `UN-BLD-13` · 45 s · at most 1 per map |
| **Veterancy** | Standard |
| **AI** | `ai.unit.naval_combatant`. Deltas: stay with the escorted convoy; engage only the violating participant |
| **Multiplayer** | MPA-NPC + MPA-SENSOR |
| **Presentation** | ANIM-NAV + turret, launcher, radar spin |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.un.sea_03.gun` 76 mm-class | HIGH_EXPLOSIVE | 0–2400 cm | 90, radius 200 cm / 2.0 s | ballistic | 8 | WATER_SURFACE, GROUND (shore), STRUCTURE | 110 % / 45 % / 12 % / — / 100 % (NAV_L 60 %) |
| `weapon.un.sea_03.sam` | FLAK_AA | 0–1800 cm | 150 / 5 s; magazine 8, reload at harbor | guided | — | AIR_HIGH, AIR_LOW | — / — / — / 110 % rotary, 100 % fixed / — |
| `weapon.un.sea_03.asw` | TORPEDO | 0–1200 cm | 180 / 10 s | ballistic depth charges | — | WATER_SUBSURFACE | (NAV_H 100 %) |

**Abilities:** none unit-specific (sensors above).

**Research and upgrades:** none.

**Counters:** *Beats (hard):* aircraft over the sea, submarines that reveal themselves. *Beats (soft):* gunboats. *Beaten by (hard):* massed anti-ship missiles (AC-SEA-03, MP-SEA-06), submarines that strike first (AC-SEA-05, RD-SEA-04). *Situational:* a fleet in being that deters naval R4 incidents.

**Status:** identity E-R; values P.

### UN-SEA-04 — JNA Attack Submarine

| Field | Specification |
| --- | --- |
| **Identity** | `unit.un.sea_04` · The Unaligned · Naval / submarine · `TPL-NPC-SECURITY` (as `TPL-NAV-SUB`) · **Conditional** (deep water required) · NPC-only |
| **Role** | **Primary:** protect coastal approaches; deep water required (E-R). **Secondary:** shadowing foreign warships at R3. **Doctrine fit:** JNA navy submarine flotilla (E). |
| **Inspiration and background** | A small diesel-electric patrol submarine. C&C lineage (E-R): Submarine (RA1); Akula Submarine (RA3); Typhoon Attack Submarine (RA2). |
| **Visual identity** | Concept: none (asset requirement). Black hull, sail with pennant number when surfaced. |
| **Statistics** | **900 HP** (`armor.nav_heavy`) · SUBSURFACE 320 cm/s (submerged ×0.8) · sight 1600 cm (surface); sonar `DETECT.SUBMERGED` 1400 cm · concealed while submerged · ExperienceValue 220 |
| **Commands** | Institutional authority only: Patrol, Shadow, Attack, Dive/Surface · Hold Fire while submerged |
| **Production** | Mobilization weight **2,000** · response tier **R3** (shadow) / **R4** (attack) · scenario naval base or `UN-BLD-13` with deep water · 40 s · at most 1 per map |
| **Veterancy** | Standard |
| **AI** | `ai.unit.submarine`. Deltas: shadow a violating fleet at R3 and report; attack only at R4 |
| **Multiplayer** | **MPA-NPC + MPA-COVERT** (submerged position never replicated to participants without `DETECT.SUBMERGED` coverage) |
| **Presentation** | ANIM-NAV-SUB. Sonar ping audio only for detected contacts |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.un.sea_04.torpedo` | TORPEDO | 0–1400 cm | 320 / 10 s; magazine 6 | guided, slow (480 cm/s) | — | WATER_SURFACE, WATER_SUBSURFACE | (NAV_L 110 %, NAV_H 100 %) |

**Abilities:** none unit-specific (Dive/Surface from the template). Firing reveals it for 6 s (template rule).

**Research and upgrades:** none.

**Counters:** *Beats (hard):* surface combatants without ASW. *Beaten by (hard):* ASW ships and patrol aircraft (UN-SEA-03 against rivals; AC-SEA-04, RD-SEA-05, AC-AIR-07). *Situational:* the hidden reason a hostile fleet should not linger in federal waters.

**Status:** identity and deep-water rule E-R; values P.

### UN-SEA-05 — Amphibious Military Transport

| Field | Specification |
| --- | --- |
| **Identity** | `unit.un.sea_05` · The Unaligned · Naval / landing ship · `TPL-NPC-SECURITY` (as `TPL-NAV-TRANSPORT`) · **Conditional** · NPC-only |
| **Role** | **Primary:** ferry JNA and Territorial Defence forces during crises (E-R). **Secondary:** reinforcing islands and coastal towns at R3–R4. **Doctrine fit:** island defense under Total People's Defence (E). |
| **Inspiration and background** | A tank landing ship with a bow ramp. C&C lineage (E-R): Amphibious Transport (RA2); Hovercraft (TD); Bullfrog (RA3, delivery role). |
| **Visual identity** | Concept: none (asset requirement). Grey landing ship, bow doors, deck crane. |
| **Statistics** | **1,100 HP** (`armor.nav_light`) · SURFACE_SHALLOW 360 cm/s · sight 1200 cm · **capacity 4 vehicles (F1–F3) or 8 squads** |
| **Commands** | Institutional authority only: Move, Beach, Unload, Return · Hold Fire |
| **Production** | Mobilization weight **1,400** · response tier **R3** · `UN-BLD-13` · 30 s |
| **Veterancy** | none |
| **AI** | `ai.unit.transport`. Deltas: beach only on authored landing beaches out of enemy weapon range |
| **Multiplayer** | MPA-NPC + MPA-TRANSPORT |
| **Presentation** | ANIM-NAV + ramp |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.un.sea_05.aa_gun` twin AA gun | AUTOCANNON | 0–850 cm | 16 / 0.4 s | projectile | 2 | AIR_LOW, WATER_SURFACE | 100 % rotary / (NAV_L 90 %) |

**Abilities:** none unit-specific.

**Research and upgrades:** none.

**Counters:** *Beaten by (hard):* missile boats, submarines, strike aircraft. *Situational:* sinking it loaded is the biggest single blow to a federal counter-landing (passengers die per framework §6.10).

**Status:** identity E-R; values P.

### UN-SEA-06 — River Police Launch

| Field | Specification |
| --- | --- |
| **Identity** | `unit.un.sea_06` · The Unaligned · Naval / river police · `TPL-NPC-SECURITY` (as `TPL-NAV-PATROL`) · **Conditional** (river maps) · NPC-only |
| **Role** | **Primary:** patrol the Sava, Drava, Danube and other navigable sectors (E-R). **Secondary:** inspection of river barges and ferries. **Doctrine fit:** police jurisdiction on inland waters (E). |
| **Inspiration and background** | A small river police launch with a pintle machine gun. C&C lineage (E-R): Gunboat (RA1); Stingray (RA3, hull role). |
| **Visual identity** | Concept: none (asset requirement). Blue-and-white launch, "MILICIJA" lettering, blue light. |
| **Statistics** | **260 HP** (`armor.nav_light`) · SURFACE_SHALLOW 600 cm/s · sight 1400 cm · `DETECT.SPY` 500 cm · ExperienceValue 40 |
| **Commands** | Institutional authority only · Hold Fire below R2 |
| **Production** | Mobilization weight **350** · response tier **R0** (patrol) / **R2** (armed) · river police station marker on `UN-SEC-01` · 8 s |
| **Veterancy** | Standard |
| **AI** | `ai.unit.naval_patrol` + `ai.unit.npc_security` tier rules (police) |
| **Multiplayer** | MPA-NPC |
| **Presentation** | ANIM-NAV. Siren audio at R2 |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.un.sea_06.mg` | SMALL_ARMS | 0–650 cm | 14 / 0.5 s | hitscan | 4 | WATER_SURFACE, GROUND (bank) | 100 % / 15 % / 2 % / — / 8 % (NAV_L 30 %) |

**Abilities:** `ability.un.sea_01.maritime_inspection` (shared with UN-SEA-01, river variant: 300 cm, also ferries UN-CIV-12).

**Research and upgrades:** none.

**Counters:** *Beats (hard):* river couriers and disguised agents crossing by boat (RD-SEA-01 crews ashore, MP-SEA-02 when caught). *Beaten by (hard):* any armed boat (RD-SEA-01, AC-SEA-01, IA-SEA-01). *Situational:* makes river crossings a jurisdiction question, not only a terrain question.

**Status:** identity E-R; values P.

### UN-SEA-07 — Civil Port Tug

| Field | Specification |
| --- | --- |
| **Identity** | `unit.un.sea_07` · The Unaligned · Civilian-service / tug · `TPL-CIV` (SURFACE_SHALLOW) · **Conditional** · ambient |
| **Role** | **Primary:** maintain port access and civilian/naval yard services (E-R). **Secondary:** clears wrecks from harbor channels. **Doctrine fit:** port authority services (E). |
| **Inspiration and background** | A stubby harbor tug with fenders. C&C lineage (E-R): Transport Ship (RA1, utility hull); Gunboat (RA1, hull size). |
| **Visual identity** | Concept: none (asset requirement). Black-and-red tug, tyre fenders, water cannon. |
| **Statistics** | **350 HP** (`armor.nav_light`) · SURFACE_SHALLOW 300 cm/s · sight 900 cm · Morale 100 |
| **Commands** | None (port schedule) · Hold Fire |
| **Production** | Ambient: 1–2 per `UN-BLD-13` |
| **Veterancy** | none |
| **AI** | `ai.unit.civilian_ambient` (naval) |
| **Multiplayer** | MPA-NPC |
| **Presentation** | ANIM-NAV. Water-cannon VFX |

**Weapons:** none (unarmed)

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.un.sea_07.clear_channel` | Automated (P) | Removes a sunken wreck blocking a harbor channel in 30 s | Wreck in a harbor channel | free · 30 s | Further sinkings (incidents) | — | When a channel is blocked |

**Research and upgrades:** none.

**Counters:** *Beaten by (hard):* anything armed (civilian incident). *Situational:* blocking a harbor by sinking ships in its channel is temporary.

**Status:** identity E-R; values P.

### UN-SEA-08 — Federal Search-and-Rescue Vessel

| Field | Specification |
| --- | --- |
| **Identity** | `unit.un.sea_08` · The Unaligned · Civilian-service / SAR vessel · `TPL-CIV` (SURFACE_SHALLOW) · **Conditional** · incident-dispatched |
| **Role** | **Primary:** rescue stranded civilians and manage maritime incidents (E-R). **Secondary:** recovers survivors of any sunk vessel, including participants' crews, as prisoners returned after the incident closes (P). **Doctrine fit:** civil protection at sea (E). |
| **Inspiration and background** | A fast SAR cutter with an orange superstructure. C&C lineage (E-R): Hydrofoil (RA3, service motion); Transport Ship (RA1, carry capacity). |
| **Visual identity** | Concept: none (asset requirement). White hull, orange superstructure, rescue nets. |
| **Statistics** | **450 HP** (`armor.nav_light`) · SURFACE_SHALLOW 560 cm/s · sight 1400 cm · capacity 4 civilian groups · Morale 120 |
| **Commands** | None · Hold Fire |
| **Production** | Dispatched from `UN-BLD-13` after any sinking or civilian maritime incident; 1 per incident |
| **Veterancy** | none |
| **AI** | `ai.unit.civilian_ambient` (emergency variant): approaches after fire within 800 cm has stopped for 5 s |
| **Multiplayer** | MPA-NPC |
| **Presentation** | ANIM-NAV + rescue net. Orange flare VFX |

**Weapons:** none (unarmed)

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.un.sea_08.rescue` | Automated (P) | Recovers survivors at a sinking site: restores the Population Morale lost to a civilian maritime incident by 50 %; participant crews recovered generate no unit and no resource for anyone | Sinking site | free · 20 s | Continuing combat | Rescue markers | After fire stops |

**Research and upgrades:** none.

**Counters:** none relevant (attacking it is an R3 incident). *Situational:* closes maritime incidents, letting the response tier decay.

**Status:** identity E-R; values P.

## 8. Faction roster summary

"Weight" is the mobilization weight (framework §11.6). Civilians show their spawn rule. Build s is the mobilization step or arrival time. Tier is the roster tier (framework §1.2). The response tier at which the unit first appears in its armed role is shown in brackets after the weight.

| ID | Name | Category | Template | Tier | Primary role | HP | Armor | Speed | Weight (tier) | Build s | Producer | Hard counters (to it) | It hard-counters |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| UN-INF-01 | Yugoslav Army Rifle Section | Infantry | `TPL-INF-LINE` | Core | JNA line infantry for autonomous federal security | 275 | inf_light | 205 FOOT | 300 (R3) | 8 | UN-BLD-05 | armor, artillery, flame | light raids |
| UN-INF-02 | Territorial Defence Reserve | Infantry | `TPL-INF-LINE` | Core | Locally mobilized defensive platoon; autonomous, no… | 270 | inf_light | 195 FOOT | 180 (R3) | 5 | UN-BLD-06 | HMG, flame (IA-INF-05), artillery | small detachments in towns |
| UN-INF-03 | Federal Border Guards | Infantry | `TPL-NPC-SECURITY` | Core | Control crossing points and observe international movements | 180 | inf_light | 200 FOOT | 220 (R0) | 6 | UN-SEC-02 | any assault (incident) | agents crossing borders |
| UN-INF-04 | Federal Police Patrol | Infantry | `TPL-NPC-SECURITY` | Core | Town security, investigation and graded force | 120 | inf_light | 210 FOOT | 150 (R0) | 5 | UN-SEC-01, UN-SEC-11 | any combat unit (incident) | agents, couriers, saboteurs |
| UN-INF-05 | Railway Protection Detachment | Infantry | `TPL-NPC-SECURITY` | Extended | Secure civilian transport and key bridges | 240 | inf_light | 200 FOOT | 240 (R1) | 7 | UN-BLD-12 | armor, artillery | bridge and rail saboteurs |
| UN-INF-06 | JNA Rocket Infantry | Infantry | `TPL-INF-AT` | Core | Anti-armor defense of military facilities | 180 | inf_light | 200 FOOT | 400 (R3) | 9 | UN-BLD-05 | HMG, flame, artillery | raiding armor |
| UN-INF-07 | Federal Engineers | Infantry | `TPL-INF-SUPPORT` | Core | Repair public infrastructure, bridges, hospitals and… | 165 | inf_light | 200 FOOT | 250 (R0) | 8 | UN-BLD-05, UN-BLD-03 | any combat unit | sabotage (repairs it) |
| UN-INF-08 | JNA Field Medical Unit | Infantry | `TPL-INF-SUPPORT` | Extended | Military medical support for NPC mobilization | 110 | inf_light | 210 FOOT | 200 (R3) | 8 | UN-BLD-05, UN-BLD-15 | any combat unit | — |
| UN-INF-09 | Mountain Reconnaissance Patrol | Infantry | `TPL-INF-SPEC` | Extended | Local scout infantry with terrain knowledge | 120 | inf_light | 215 FOOT | 300 (R1) | 10 | UN-BLD-05 | marksmen, artillery | concealed scouts in mountains |
| UN-INF-10 | State Security Investigator | Infantry | `TPL-INF-COVERT` | Extended | Investigates facility intrusions, fraud and alleged… | 110 | inf_light | 210 FOOT | 500 (R2) | 14 | UN-SEC-03 | assassination (incident) | deniable covert action |
| UN-INF-11 | Federal Counterintelligence Officer | Infantry | `TPL-INF-COVERT` | Core | Strongest counterintelligence: intercepts, corroborates… | 130 | inf_light | 205 FOOT | 700 (R0) | 18 | UN-SEC-03 | kill or avoid its coverage | infiltration, theft, disguise |
| UN-INF-12 | Deception Bureau Liaison | Infantry | `TPL-INF-COVERT` | Scenario | NPC-only planted false intelligence and false flags;… | 100 | inf_light | 210 FOOT | 900 | — | UN-SEC-04 | corroboration, expiry | — (information only) |
| UN-INF-13 | Public Continuity Commissioner | Infantry | `TPL-INF-COMMAND` | Scenario | Coordinates official communications and civilian morale,… | 150 | inf_light | 190 FOOT | 600 | — | UN-SEC-06, UN-SEC-07, UN-BLD-01 | media-building sabotage | — |
| UN-INF-14 | Federal Honor Guard | Infantry | `TPL-NPC-SECURITY` | Scenario | Scenario-protected symbolic and ceremonial detachment,… | 480 | inf_protected | 200 FOOT | ambient | — | UN-BLD-01 | — (scenario-protected) | — |
| UN-ARM-01 | Municipal Patrol Car | Vehicle | `TPL-VEH-SCOUT` | Core | Police response and traffic checks; jurisdiction-sensitive | 200 | veh_soft | 520 WHEEL_LIGHT | 180 (R0) | 5 | UN-SEC-01 | any armed unit (incident) | smugglers, disguised vehicles |
| UN-ARM-02 | Federal Recon Jeep | Vehicle | `TPL-VEH-SCOUT` | Core | Army visual patrol of roads and bases | 240 | veh_soft | 560 WHEEL_LIGHT | 200 (R0) | 6 | UN-BLD-05 | any armed vehicle | concealed road movement |
| UN-ARM-03 | Border Surveillance Truck | Vehicle | `TPL-VEH-EW` | Extended | Radio-equipped mobile border watch | 380 | veh_light | 360 WHEEL_HEAVY | 450 (R1) | 10 | UN-SEC-02, UN-BLD-05 | jammers, any armed unit | covert border crossings |
| UN-ARM-04 | JNA Armored Personnel Carrier | Vehicle | `TPL-VEH-TRANSPORT` | Core | Protected federal infantry transport | 560 | veh_light | 360 TRACKED | 650 (R3) | 12 | UN-BLD-05 | AT infantry, tanks | infantry in the open |
| UN-ARM-05 | Territorial Defence Truck | Vehicle | `TPL-VEH-TRANSPORT` | Extended | Inexpensive reserve transport and ammunition hauling | 320 | veh_soft | 400 WHEEL_HEAVY | 180 (R3) | 6 | UN-BLD-06 | any armed unit | — |
| UN-ARM-06 | Federal Battle Tank | Vehicle | `TPL-VEH-MBT` | Core | Conventional Yugoslav-era armored formation, medium or heavy | 900 | veh_heavy | 320 TRACKED | 1,100 (R4) | 18 | UN-BLD-07, UN-BLD-05 | ATGM, tank destroyers, attack helicopters | light and medium armor |
| UN-ARM-07 | Reserve Tank | Vehicle | `TPL-VEH-MBT` | Extended | Older stored equipment mobilized near crises | 680 | veh_medium | 300 TRACKED | 600 (R3) | 14 | UN-BLD-06 | modern MBTs, ATGM | light vehicles |
| UN-ARM-08 | JNA Anti-Air Gun Carrier | Vehicle | `TPL-VEH-AA` | Core | Mobile short-range air defense | 520 | veh_light | 330 TRACKED | 800 (R2) | 14 | UN-BLD-05 | artillery, tanks | helicopters, low aircraft |
| UN-ARM-09 | Army Rocket Artillery | Vehicle | `TPL-VEH-ARTY` | Core | Conventional mobile multiple-launch rockets | 420 | veh_light | 320 WHEEL_HEAVY | 1,200 (R4) | 20 | UN-BLD-05 | counter-battery, air strikes | massed infantry, soft columns |
| UN-ARM-10 | Field Howitzer Tractor | Vehicle | `TPL-VEH-ARTY` | Extended | Towed or self-propelled artillery for defensive fighting | 480 | veh_light | 280 WHEEL_HEAVY | 1,100 (R4) | 22 | UN-BLD-05, UN-BLD-06 | counter-battery, raids | static positions |
| UN-ARM-11 | Military Engineering Dozer | Vehicle | `TPL-VEH-ENGINEER` | Extended | Move barriers and repair bridge approaches | 800 | veh_heavy | 280 TRACKED | 600 (R0) | 14 | UN-BLD-05 | AT weapons | blocked routes, barriers |
| UN-ARM-12 | Federal Mobile Repair Unit | Vehicle | `TPL-VEH-LOGISTICS` | Extended | Stabilizes damaged JNA and public vehicles | 500 | veh_light | 350 WHEEL_HEAVY | 500 (R0) | 12 | UN-BLD-07 | any armed unit | — |
| UN-ARM-13 | Anti-Armor Defense Vehicle | Vehicle | `TPL-VEH-TD` | Extended | Protect sensitive installations against raiders | 480 | veh_medium | 380 WHEEL_HEAVY | 1,000 (R3) | 16 | UN-BLD-05 | infantry, artillery, smoke | raiding armor |
| UN-ARM-14 | Reserve Mobilization Carrier | Vehicle | `TPL-VEH-TRANSPORT` | Extended | Move Territorial Defence squads under NPC orders | 500 | veh_light | 360 WHEEL_HEAVY | 450 (R3) | 10 | UN-BLD-06 | AT weapons, autocannon | — |
| UN-ARM-15 | Crisis Command Vehicle | Vehicle | `TPL-VEH-EW` | Extended | Small mobile command post for public-order escalation | 520 | veh_light | 340 WHEEL_HEAVY | 700 (R2) | 14 | UN-SEC-10 | jammers, any armed unit | — |
| UN-CIV-01 | Civilian Passenger Car | Civilian | `TPL-CIV` | Core | Ambient traffic and continuity; not recruitable | 120 | veh_soft | 480 WHEEL_LIGHT | ambient | — | off-map / route | anything armed (incident) | — |
| UN-CIV-02 | Intercity Bus | Civilian | `TPL-CIV` | Extended | Move civilians and evacuation groups under incident rules | 300 | veh_soft | 400 WHEEL_HEAVY | ambient | — | UN-SEC-10 | anything armed (incident) | — |
| UN-CIV-03 | Freight Lorry | Civilian | `TPL-CIV` | Core | Factories, warehouses and shops maintain economy flows | 220 | veh_soft | 380 WHEEL_HEAVY | ambient | — | UN-BLD-12 | anything armed (incident) | — |
| UN-CIV-04 | Railway Freight Train | Civilian | `TPL-CIV` | Conditional | Schedules resource and merchant convoys; rail asset and… | 900 | veh_light | 700 RAIL | ambient | — | off-map / route | anything armed (incident) | — |
| UN-CIV-05 | Oil Tanker Lorry | Civilian | `TPL-CIV` | Extended | Carry civilian crude oil and finished petroleum, which… | 260 | veh_soft | 360 WHEEL_HEAVY | ambient | — | off-map / route | anything armed (incident) | — |
| UN-CIV-06 | Public Ambulance | Civilian | `TPL-CIV` | Core | Casualty response and morale stabilization after incidents | 160 | veh_soft | 520 WHEEL_LIGHT | ambient | — | UN-BLD-15 | anything armed (incident) | — |
| UN-CIV-07 | Civilian Fire Engine | Civilian | `TPL-CIV` | Extended | Emergency building damage response; no combat | 400 | veh_soft | 400 WHEEL_HEAVY | ambient | — | UN-BLD-03 | anything armed (incident) | — |
| UN-CIV-08 | Electrical Maintenance Van | Civilian | `TPL-CIV` | Extended | Repair power lines, substations and street outages | 150 | veh_soft | 480 WHEEL_LIGHT | ambient | — | UN-BLD-10, UN-BLD-11 | anything armed (incident) | — |
| UN-CIV-09 | Postal and Press Delivery Van | Civilian | `TPL-CIV` | Extended | Newspaper dissemination, observable and disruptable | 130 | veh_soft | 500 WHEEL_LIGHT | ambient | — | UN-SEC-08 | anything armed (incident) | — |
| UN-CIV-10 | Agricultural Tractor | Civilian | `TPL-CIV` | Extended | Rural economy and market goods; non-military | 180 | veh_soft | 180 WHEEL_HEAVY | ambient | — | off-map / route | anything armed (incident) | — |
| UN-CIV-11 | Commercial Fishing Boat | Civilian | `TPL-CIV` | Conditional | Neutral regional water economy; scenario-only | 160 | nav_light | 360 SURFACE_SHALLOW | ambient | — | UN-BLD-13 | anything armed (incident) | — |
| UN-CIV-12 | River Ferry | Civilian | `TPL-CIV` | Conditional | Realistic civilian crossing that NPC jurisdiction can close | 500 | nav_light | 200 SURFACE_SHALLOW | ambient | — | off-map / route | anything armed (incident) | — |
| UN-AIR-01 | JNA Interceptor Fighter | Air | `TPL-AIR-FIGHTER` | Core | Defend sovereign airspace from unauthorized intrusions | 360 | air_fixed | 2,500 FIXED_WING | 1,800 (R1) | 30 | off-map / route | interceptors, SAMs | transports, recon aircraft |
| UN-AIR-02 | JNA Ground-Attack Jet | Air | `TPL-AIR-STRIKE` | Extended | Conventional air response when sovereignty is violated | 420 | air_fixed | 1,900 FIXED_WING | 2,200 (R4) | 45 | off-map / route | mobile AA, interceptors | infantry and soft columns |
| UN-AIR-03 | Federal Transport Helicopter | Air | `TPL-AIR-HELI-TRANSPORT` | Extended | Military personnel and evacuation logistics | 560 | air_rotary | 680 ROTARY | 1,000 (R2) | 20 | UN-BLD-05 | AA, autocannon | — |
| UN-AIR-04 | Army Attack Helicopter | Air | `TPL-AIR-HELI-ATTACK` | Extended | JNA rotary close support at heavy escalation | 460 | air_rotary | 760 ROTARY | 1,500 (R4) | 25 | off-map / route | mobile AA, autocannon | armor without AA |
| UN-AIR-05 | Federal Reconnaissance Aircraft | Air | `TPL-AIR-SCOUT` | Extended | Monitor borders, mountain passes and maritime coastlines | 340 | air_fixed | 2,000 FIXED_WING | 900 (R1) | 30 | off-map / route | interceptors, SAMs | deniability of covert acts |
| UN-AIR-06 | Civil Passenger Airliner | Civilian | `TPL-CIV` | Scenario | Timetabled neutral flights; civilian, not a recruitable… | 900 | air_fixed | 1,800 FIXED_WING | ambient | — | off-map / route | anything armed (incident) | — |
| UN-AIR-07 | Emergency Airlift Plane | Civilian-military | `TPL-AIR-TRANSPORT` | Scenario | Evacuate civilians and deliver food; mission scripted,… | 800 | air_fixed | 1,500 FIXED_WING | ambient | — | off-map / route | anything armed (incident) | — |
| UN-AIR-08 | Adriatic Maritime Patrol Plane | Air | `TPL-AIR-SURVEILLANCE` | Conditional | Identify shipping, foreign landings and offshore incursions | 520 | air_fixed | 1,400 FIXED_WING | 1,200 (R0) | — | off-map / route | interceptors | submarines, covert landings |
| UN-SEA-01 | JNA Coastal Patrol Boat | Naval | `TPL-NAV-PATROL` | Conditional | Routine sea and port inspection | 400 | nav_light | 620 SURFACE_SHALLOW | 700 (R0) | 14 | UN-BLD-13 | missile boats, aircraft | smuggling vessels |
| UN-SEA-02 | Federal Missile Patrol Boat | Naval | `TPL-NAV-COMBATANT` | Conditional | Coastal anti-surface missile response | 520 | nav_light | 680 SURFACE_SHALLOW | 1,300 (R4) | 20 | UN-BLD-13 | aircraft, submarines | surface ships |
| UN-SEA-03 | Adriatic Frigate Escort | Naval | `TPL-NAV-COMBATANT` | Conditional | Protect shipping and defend federal waters | 1,900 | nav_heavy | 340 SURFACE_DEEP | 2,800 (R4) | 45 | UN-BLD-13 | anti-ship salvos, submarines | aircraft over sea, submarines |
| UN-SEA-04 | JNA Attack Submarine | Naval | `TPL-NAV-SUB` | Conditional | Protect coastal approaches; deep water required | 900 | nav_heavy | 320 SUBSURFACE | 2,000 (R3) | 40 | UN-BLD-13 | ASW ships, patrol aircraft | surface ships without ASW |
| UN-SEA-05 | Amphibious Military Transport | Naval | `TPL-NAV-TRANSPORT` | Conditional | Ferry JNA and Territorial Defence forces during crises | 1,100 | nav_light | 360 SURFACE_SHALLOW | 1,400 (R3) | 30 | UN-BLD-13 | missile boats, submarines, strike aircraft | — |
| UN-SEA-06 | River Police Launch | Naval | `TPL-NAV-PATROL` | Conditional | Patrol the Sava, Drava, Danube and other navigable sectors | 260 | nav_light | 600 SURFACE_SHALLOW | 350 (R0) | 8 | UN-SEC-01 | any armed boat | river couriers |
| UN-SEA-07 | Civil Port Tug | Civilian-service | `TPL-CIV` | Conditional | Maintain port access and civilian/naval yard services | 350 | nav_light | 300 SURFACE_SHALLOW | ambient | — | UN-BLD-13 | anything armed (incident) | — |
| UN-SEA-08 | Federal Search-and-Rescue Vessel | Civilian-service | `TPL-CIV` | Conditional | Rescue stranded civilians and manage maritime incidents | 450 | nav_light | 560 SURFACE_SHALLOW | ambient | — | UN-BLD-13 | anything armed (incident) | — |

## 9. Abilities and upgrades index

The Unaligned has no player research. Scenario flags are listed as upgrades. Shared abilities (`ability.shared.*`) are not repeated.

| ID | Unit(s) | Kind | Cost | Cooldown | Range | Requires | Counters |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.un.inf_01.cordon` | UN-INF-01 | Active | free | 90 s | ≤ 800 cm | response tier R3 | Avoid the cordon; artillery |
| `ability.un.inf_02.home_municipality` | UN-INF-02 | Conditional | — | — | — | response tier R3 | Lure it out; flame on garrisons |
| `ability.un.inf_02.mass_mobilization` | UN-INF-02 | Automated | weight per platoon | — | — | response tier R3 | Lower Unaligned morale; avoid R3 |
| `ability.un.inf_03.border_inspection` | UN-INF-03 | Automated | free | per unit | — | response tier R0 | Cross elsewhere; agreements; kill the post (R2+ incident) |
| `ability.un.inf_03.incident_report` | UN-INF-03 | Automated | — | — | — | response tier R0 | Kill observers before they report (the kill itself is an incident) |
| `ability.un.inf_04.arrest` | UN-INF-04 | Active | free | 20 s | — | response tier R0 | Escorts; killing the police (R3 escalation); not usable on squads that are Steady |
| `ability.un.inf_04.inspection` | UN-INF-04 | Active | free | 15 s | — | response tier R0 | Avoid patrols |
| `ability.un.inf_05.bridge_watch` | UN-INF-05 | Passive | — | — | — | response tier R1 | Kill the detachment first (incident) |
| `ability.un.inf_07.public_repair` | UN-INF-07 | Active | budget | — | ≤ 300 cm | response tier R0 | Escort removal; incidents nearby pause it |
| `ability.un.inf_07.remove_charge` | UN-INF-07 | Active | free | — | — | response tier R0 | Kill them (incident) |
| `ability.un.inf_08.heal` | UN-INF-08 | Active | free | — | — | response tier R3 | Kill the medics (incident; attacking marked medics is an **R+1** escalation, P) |
| `ability.un.inf_10.investigate_incident` | UN-INF-10 | Active | budget | 60 s | ≤ 600 cm | response tier R2 | Kill the investigator (a new incident); leave no evidence (no detection during the act) |
| `ability.un.inf_11.block_infiltration` | UN-INF-11 | Passive | — | — | — | response tier R0 | Kill or lure the officer (incident); act outside range |
| `ability.un.inf_11.intercept` | UN-INF-11 | Automated | budget | — | 2000 cm | response tier R0 | Radio silence; act outside coverage |
| `ability.un.inf_12.plant_false_report` | UN-INF-12 | Automated | director budget | cooldown per target 240 s | — | scenario / schedule | Physical reconnaissance, MP-AIR-05 *Verify Report*, independent sources, contradictions (E) |
| `ability.un.inf_12.misattribute` | UN-INF-12 | Automated | director budget | — | — | scenario / schedule | Counter-investigation evidence |
| `ability.un.inf_13.official_broadcast` | UN-INF-13 | Automated | director budget | 300 s | — | scenario / schedule | Psy-Ops or sabotage on media buildings (incidents); lowered morale |
| `ability.un.inf_14.scenario_protected` | UN-INF-14 | Passive | — | — | — | scenario / schedule | Scenario only |
| `ability.un.arm_01.traffic_stop` | UN-ARM-01 | Active | free | 15 s | — | response tier R0 | Do not stop: driving on is an R1 incident |
| `ability.un.arm_01.roadblock` | UN-ARM-01 | Active | free | 120 s | — | response tier R0 | Alternate roads |
| `ability.un.arm_03.border_watch` | UN-ARM-03 | Active | free | — | — | response tier R1 | Move slowly (< 25 % speed is not detected); kill it (incident) |
| `ability.un.arm_08.airspace_warning` | UN-ARM-08 | Automated | free | — | — | response tier R2 | Comply or suppress the gun |
| `ability.un.arm_10.unlimber` | UN-ARM-10 | Active | free | — | — | response tier R4 | Caught limbered |
| `ability.un.arm_11.build_barrier` | UN-ARM-11 | Active | budget | — | — | response tier R0 | Demolition, engineers, heavy HE |
| `ability.un.arm_11.clear_route` | UN-ARM-11 | Active | free | — | ≤ 200 cm | response tier R0 | Fire on the dozer |
| `ability.un.arm_15.incident_command` | UN-ARM-15 | Aura | free | — | — | response tier R2 | Jamming (AC-ARM-10, RD-SUP-06, IA-ARM-14); kill it (incident) |
| `ability.un.civ.traffic_witness` | UN-CIV-01 | Automated | free | — | — | scenario / schedule | Act away from traffic; kill witnesses (another incident) |
| `ability.un.civ_02.evacuate` | UN-CIV-02 | Automated | free | — | — | scenario / schedule | Blocking roads; firing near it (incident) |
| `ability.un.civ_03.economic_flow` | UN-CIV-03 | Automated | — | — | — | scenario / schedule | Route interdiction (an incident) |
| `ability.un.civ_04.consist` | UN-CIV-04 | Passive | — | — | — | scenario / schedule | — |
| `ability.un.civ_04.line_closure` | UN-CIV-04 | Automated | — | — | — | scenario / schedule | Avoid incidents near the line |
| `ability.un.civ_05.fuel_fire` | UN-CIV-05 | Passive | — | — | — | scenario / schedule | Stay clear |
| `ability.un.civ_06.casualty_response` | UN-CIV-06 | Automated | free | — | — | scenario / schedule | Continuing combat delays it; destroying it (R3 incident) |
| `ability.un.civ_07.extinguish` | UN-CIV-07 | Active | free | — | — | scenario / schedule | Continuing combat |
| `ability.un.civ_07.clear_lane` | UN-CIV-07 | Automated | free | — | — | scenario / schedule | — |
| `ability.un.civ_08.restore_line` | UN-CIV-08 | Active | free | — | 300 cm | scenario / schedule | Kill it (incident); repeat the sabotage |
| `ability.un.civ_09.distribution` | UN-CIV-09 | Automated | — | — | — | scenario / schedule | Interdiction (incident); counterintelligence corroboration exposes the false content |
| `ability.un.civ_12.jurisdiction_closure` | UN-CIV-12 | Automated | — | — | — | scenario / schedule | Keep the response tier low; bridging equipment; amphibious units |
| `ability.un.civ_12.passage` | UN-CIV-12 | Automated | free | — | — | scenario / schedule | Attack the ferry mid-river (severe incident) |
| `ability.un.air_01.shadow_and_warn` | UN-AIR-01 | Automated | free | — | — | response tier R1 | Leave; fly below `UN-SEC-05` coverage; agreements |
| `ability.un.air_03.medevac` | UN-AIR-03 | Automated | free | — | — | response tier R2 | AA; attacking it is an R3 incident |
| `ability.un.air_05.attribution_photo` | UN-AIR-05 | Automated | free | — | — | response tier R1 | AA; act and leave quickly; concealment |
| `ability.un.air_06.civil_flight_protection` | UN-AIR-06 | Passive | — | — | — | scenario / schedule | — |
| `ability.un.air_07.relief_drop` | UN-AIR-07 | Scripted | scripted | — | — | scenario / schedule | AA (immediate R4 incident) |
| `ability.un.air_08.sonobuoy_line` | UN-AIR-08 | Active | free | 120 s | — | response tier R0 | Destroy buoys (any surface weapon); run deep and slow |
| `ability.un.sea_01.maritime_inspection` | UN-SEA-01 | Active | free | 20 s | — | response tier R0 | Avoid patrol lanes; outrun it (MP-SEA-02) |
| `ability.un.sea_07.clear_channel` | UN-SEA-07 | Automated | free | — | — | scenario / schedule | Further sinkings (incidents) |
| `ability.un.sea_08.rescue` | UN-SEA-08 | Automated | free | — | — | scenario / schedule | Continuing combat |
| `un.mobilization_readiness` (scenario flag) | UN-INF-01 and JNA infantry | Upgrade (scenario) | scenario | — | — | high-tension scenario | — |
| `un.air_alert` (scenario flag) | UN-AIR-01 | Upgrade (scenario) | scenario | — | — | scenario | Fly low below radar |

## 10. Faction tactical coverage, gaps and balance risks

**10.1 Coverage by role**

| Role | Units | Assessment |
| --- | --- | --- |
| Line infantry | UN-INF-01, UN-INF-02, UN-INF-05 | Deep. TO platoons scale with Population Morale; JNA sections are the R3 backbone |
| Anti-armor infantry | UN-INF-06 | Single unit, backed by UN-ARM-13 and UN-AIR-04 at R4 |
| Police, border and inspection | UN-INF-03, UN-INF-04, UN-ARM-01, UN-ARM-03, UN-SEA-01, UN-SEA-06 | The densest detection net in the game, by design (E: strongest counterintelligence) |
| Counterintelligence and deception | UN-INF-10, UN-INF-11, UN-INF-12 | Signature. Bounded by coverage, cost, cooldown and expiry, never omniscient (E) |
| Engineering and repair | UN-INF-07, UN-ARM-11, UN-ARM-12, UN-CIV-07, UN-CIV-08, UN-SEA-07 | Infrastructure always recovers unless the attacker keeps paying incidents |
| Scouting | UN-ARM-02, UN-INF-09, UN-AIR-05, UN-AIR-08 | Feeds the institutional AI only; never shares vision with participants |
| Armor and transport | UN-ARM-04, UN-ARM-06, UN-ARM-07, UN-ARM-14, UN-ARM-05 | R3–R4 only; older reserve stock is deliberately weaker than playable MBTs |
| Artillery | UN-ARM-09, UN-ARM-10 | R4 only; never fires near civilians |
| Air defense | UN-ARM-08, UN-AIR-01, UN-SEA-03, plus `UN-SEC-12` | Low-level gun AA and interceptors; SAM coverage is static |
| Air power | UN-AIR-02, UN-AIR-04 | R4 only, rate-limited, off-map entry |
| Naval | UN-SEA-01 to UN-SEA-05 | Conditional; one frigate and one submarine cap |
| Civil world | UN-CIV-01 to UN-CIV-12, UN-AIR-06, UN-AIR-07, UN-SEA-07, UN-SEA-08 | Witnesses, economic flows, incident close-out; never targets, never resources |

**10.2 Weak spots and intended answers**

- **Slow escalation.** The federation reaches armor only at R4. A participant who commits one decisive blow and withdraws may avoid the JNA entirely. *Intended answer:* attribution persists (UN-AIR-05 corroboration) and Ascendancy hostility +1 keeps the most aggressive faction closer to R4.
- **Off-map air.** Without an airfield the Unaligned cannot lose its air force on the map, and players cannot pre-empt it. *Intended answer:* rate limits, 30–45 s arrival tells and per-participant targeting. See U-UN-03.
- **Thin anti-armor at R3.** UN-INF-06 alone may not stop a Concord or Ascendancy armored raid on a federal site before R4. *Intended answer:* UN-ARM-13 at protected installations from R3.
- **Civilians as shields.** Strict civilian protection (no strikes within 800 cm, no auto-acquire) could let participants park near civilians. *Intended answer:* civilians flee combat, and occupying civilian districts is itself an R1–R3 trigger.

**10.3 Balance risks**

| Risk | Why | Mitigation (P) |
| --- | --- | --- |
| Kingmaking by provocation | A participant may steer federal hostility toward a rival with forged attribution | Allowed only through the E deception rules; corroboration (UN-AIR-05, UN-INF-11) and report expiry; hostility scores never replicated |
| Federal snowball | An R4 response against one participant could decide a four-player match | Response is per participant and limited to the incident's jurisdiction; mobilization budget is finite and refills slowly; the Unaligned never vetoes a lawful victory (E) |
| Civilian simulation cost | Dense traffic, trains and ferries add server and network load for four players | Density scaling by tier; quantized low-rate replication for ambient units (U-UN-02) |
| Free resources from civilians | Destroying tankers, freight or airlifts could look like economic warfare against a rival | Civilian cargo is never captured; losses affect only the Unaligned economy and raise the attacker's response tier |
| Psy-Ops and Intimidation abuse | Concord and Ascendancy covert tools could farm civilians | E eligibility rules: civilians rampage only with authored attack actions; neutral personnel never convertible; civilian vehicles have no Discipline (§5) |

**10.4 Open items**

Framework register items that apply here: U-01 (squad model), U-03 (mobility and nav-agent classes, especially RAIL and civilian lanes), U-05 (food shortage penalty, abstracted for the Unaligned), U-07 (crush, relevant to civilian traffic), plus the framework's mobilization-budget and response-tier tuning items.

| ID | Item | Proposed resolution (P) |
| --- | --- | --- |
| U-UN-01 | Explicit attack orders on civilians | Accept only with a confirmation that names the incident cost; never auto-acquire. Needs an owner decision on whether civilians may be targeted at all |
| U-UN-02 | Civilian replication budget | Quantized, reduced-rate transforms for ambient units outside combat; full rate within 2000 cm of any participant unit. Needs network profiling |
| U-UN-03 | No Unaligned airfield in the roster | Military aircraft enter and leave through authored off-map corridors. Alternative: add an airfield building to the roster (owner decision; this file does not add one) |
| U-UN-04 | Mobilization-weight economy | Budget refill rate and Population Morale scaling are scenario-authored; needs a default curve and playtest |
| U-UN-05 | Survivors and detention | Arrested agents (UN-INF-04) and recovered crews (UN-SEA-08) generate nothing for anyone; whether participants may negotiate their return is open |
