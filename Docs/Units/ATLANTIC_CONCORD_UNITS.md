# POLITIKA — Atlantic Concord Unit Specifications

**Status:** PROPOSED unit gameplay specification (29 October 2026). No unit in this file is implemented. Numbers are Provisional Baseline (PB) values.
**Authority:** names and IDs come from the [Five-Faction Rosters](../POLITIKA_FIVE_FACTION_ROSTERS.md); shared rules from the [Unit Specification Framework](../UNIT_SPECIFICATION_FRAMEWORK.md); faction canon from the [faction description](../../Factions/TheAtlanticConcord/description.md). Owner documents win on conflict.
**Evidence labels:** I / E / E-R / P / U as in framework §1.1. A field without a label is P. Cross-faction views: [Unit Balance and Ability Matrices](../UNIT_BALANCE_AND_ABILITY_MATRICES.md). Coverage: [Unit Specification Coverage Report](../Reports/UNIT_SPECIFICATION_COVERAGE_20261029.md).
**Siblings:** [Red Directorate](RED_DIRECTORATE_UNITS.md) · [Iron Ascendancy](IRON_ASCENDANCY_UNITS.md) · [Meridian Pact](MERIDIAN_PACT_UNITS.md) · [The Unaligned](UNALIGNED_UNITS.md).

## 1. Faction doctrine and faction-wide unit rules

The Concord converts economic and technological strength into favorable conditions before it commits valuable forces: it identifies enemy formations early, keeps communications reliable, and coordinates armor, infantry, artillery and aviation against selected objectives (E: faction description, *Military Doctrine*, *Strategic Philosophy*). Its units are individually strong and versatile, and balanced by high acquisition cost, specialist maintenance and slower replacement (E: *Gameplay*). Small formations matter when communications, mobility and supply work together, and become isolated when junctions, depots or airfields are lost. High equipment cost rewards preservation, recovery and repair (E).

| Rule | Unit-level implementation | Label |
| --- | --- | --- |
| Solitary-deployment Discipline | No isolation penalty to resolve recovery or stress taken (framework §7.4). Ordinary group support (+50 % recovery with 3+ coherent units) still applies. Concord units remain vulnerable to deliberate Discipline damage and to Ascendancy Intimidation once fully broken and isolated | E (rule), P (values) |
| Cost position | Unit prices at the upper edge of each §4.3 band ("professional premium"). Replacement through replenishment at 0.8 × price per missing share (§10.3) | E (high cost), P (values) |
| Fossil-fuel refining | Refinery operating cost −12 % (economy-side; unit prices unchanged). Petroleum and kerosene costs of production and sorties are therefore slightly cheaper to sustain, never free | E (rule), P (value) |
| Opening scout | `AC-SUP-01` Swiftwing Recon Plane: cheap, **unarmed**, cannot fight, no espionage by overflight. Validator: no weapon slot | E |
| Sabotage access | Attached-charge sabotage only through `AC-INF-11` after `tech.ac.late_espionage` (maps to I `late_espionage_unlocked`), at a higher price than Meridian non-spy charges. No other Concord unit carries charges | E (I predicate `CanPlantExplosives`) |
| Espionage specialty | **Psy-Ops** (unit breakdown or bounded rampage; building self-destruction or concealed resource waste) through `AC-INF-12` only | E (rule), P (values) |
| Propaganda | Radio, newspaper and television research (AC-BLD-18, AC-DEF-09, AC-DEF-10) raises Population Morale, which sets resolve baseline maximum (80–100) and production output | E |
| Nuclear | Established nuclear power; completed bombs delivered only by `AC-AIR-11`; completion never wins by itself | E |
| Food and fuel | 1 food per staff member per 30 s interval (§4.5). Petroleum per production (light 1, medium 3, heavy 5, super 10); kerosene per sortie | E (upkeep exists), P (values) |
| Visual language | Graphite concept sheets of late-Cold-War Western kit; production palette woodland green and grey with a **pale-blue faction band** on every unit; calm, procedural radio voice set | P (from concept art) |

**C&C design lessons applied.** The roster deliberately uses the GDI and Allied pattern of *expensive, specialised, information-led* forces: a cheap observation aircraft instead of an armed scout (RA1 spy plane, Generals radar van), a dedicated medic and repair economy that makes preservation pay (RA1 medic, TS mobile repair vehicle), long-range precision rocket artillery that needs spotting (Generals Tomahawk), a modular IFV whose loadout follows its passenger (RA2 IFV), and radar-dependent air defence (Patriot). Lessons avoided: the RA1 engineer rush (capture is an 8 s channel, §10.6), the RA2 IFV "one vehicle does everything" problem (each module is weaker than the specialist it borrows from), and invisibility (`AC-ARM-11` only conceals stationary vehicles inside a visible, emitting screen).

## 2. Core roster selection

Core (17 units; framework §1.2):

| ID | Role covered | Reason |
| --- | --- | --- |
| AC-INF-01 | Line infantry | Expensive, durable rifle squad that holds ground and spots |
| AC-INF-03 | Anti-armor infantry (+ limited air) | Infantry answer to vehicles and helicopters |
| AC-INF-04 | Suppression and garrison clearing | Concord's only area-effect infantry |
| AC-INF-05 | Medic and replenishment | Makes the preservation doctrine playable |
| AC-INF-06 | Engineer / capture / repair | Capture, field repair, obstacle clearing |
| AC-INF-08 | Marksman and recon | Concealed spotter that makes precision artillery work |
| AC-SUP-01 | Opening scout (E) | Canonical unarmed scout plane |
| AC-SUP-02 | Constructor | Mobile works unit for forward expansion |
| AC-ARM-01 | Light vehicle | Escort, anti-infantry raider, convoy guard |
| AC-ARM-02 | Transport | Protected infantry carrier |
| AC-ARM-04 | Main battle tank | Precision MBT, the armored backbone |
| AC-ARM-06 | Tank destroyer / ATGM | Long-range vehicle killer that trades range for close protection |
| AC-ARM-07 | Artillery | Conventional observed indirect fire |
| AC-ARM-09 | Mobile air defense | Radar-directed SAM escort |
| AC-ARM-13 | Repair and recovery | Field repair and salvage (preservation doctrine) |
| AC-AIR-01 | Fighter | Air superiority (faction-defining air power, E) |
| AC-AIR-04 | Faction signature | Attack helicopter: air-mobile anti-armor strike |

**Extended:** AC-INF-02, AC-INF-07, AC-INF-09 (cap 2), AC-INF-10, AC-INF-13, AC-INF-14 (commander; talents follow the first playable skirmish, E: talent doc §8), AC-ARM-03, AC-ARM-05, AC-ARM-08, AC-ARM-10, AC-ARM-11, AC-ARM-12, AC-ARM-14, AC-SUP-03 to AC-SUP-07, AC-AIR-02, AC-AIR-03, AC-AIR-05 to AC-AIR-10. Added after the Core counter web is playtested.
**Conditional:** AC-INF-11 and AC-INF-12 (espionage track; charges need `tech.ac.late_espionage`), AC-ARM-15 (prototype cap 1), AC-AIR-11 (all nuclear gates), AC-SEA-01 to AC-SEA-06 (only on maps with an authored naval layer and an AC-BLD-08 site; E-R map-restricted).
**Scenario:** AC-SEA-07 (roster: "scenario-tier ship").

## 3. Infantry, specialists, engineers and intelligence

### AC-INF-01 — Professional Rifle Section

| Field | Specification |
| --- | --- |
| **Identity** | `unit.ac.inf_01` · Atlantic Concord · Infantry / line rifle squad · `TPL-INF-LINE` · **Core** · Playable |
| **Role** | **Primary:** disciplined general-purpose rifle squad that holds ground and screens armor (E-R). **Secondary:** urban garrison; forward observation for indirect fire; harassment of soft vehicles. **Doctrine fit:** few highly trained soldiers who keep Discipline when detached (E: Concord solo/small-unit exemption), expensive to replace, so preservation matters (E: faction description, Strategic Philosophy). |
| **Inspiration and background** | Late-Cold-War professional Western rifle section: intermediate-calibre service rifles, a squad automatic weapon and a section radio. This is the equipment class, not a specific historical unit. In-universe, it is the multinational contract-trained infantry that the Atlantic Council deploys under negotiated permissions. C&C lineage (E-R): GDI Minigunner (TD); Ranger (Gen); Rifleman Squad (TW). |
| **Visual identity** | Concept: [`AC-INF-01.png`](../../Factions/TheAtlanticConcord/Units/AC-INF-01.png) (graphite sheet: PASGT-style helmet, load-bearing vest, rifle, equipment layout). Silhouette: four members, one with a radio whip antenna; restrained woodland-grey uniform, pale-blue shoulder band. |
| **Statistics** | 4 × 75 HP = **300** (`armor.inf_light`; +15 member HP over the RRS for professional training) · FOOT **210 cm/s** · F0 · sight **1300 cm** · no detector · 1 transport slot · food staff 4 · fuel none · ExperienceValue 36 |
| **Commands** | Move, Attack, AttackMove, Stop, Guard, Grenade, Reload, Sprint (I) · Garrison, Patrol, Withdraw, Scatter (P) · default stance Aggressive · formation role `frontline` |
| **Production** | **$360** (band top: Concord professional premium) · producer `AC-BLD-04` Personnel Training Center · prerequisites: `AC-BLD-01` operational and powered (E: producer power) · **9 s** · upkeep 4 food/interval · replenishment 80 % of the missing share at AC-BLD-04 or AC-INF-05 · no unique cap |
| **Veterancy** | Standard (§8); Elite first aid is especially valuable because of the high replacement cost |
| **AI** | `ai.unit.line_infantry`. Deltas: retreat to the nearest friendly medic or producer at 40 % HP (Concord preservation); garrison the nearest eligible building when enemy armor is observed and no friendly AT unit is within 800 cm |
| **Multiplayer** | MPA-STD. No unit-specific sensitive state |
| **Presentation** | ANIM-INF (I baseline set: idle, run, fire, death) plus reload, throw, prone/pinned, garrison. One member mesh with 2 head variants ×4; radio member shows an antenna. 5.56-class tracer VFX; calm, professional voice set; icon of a rifle silhouette with a blue band |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.ac.inf_01.primary` service rifles + SAW | SMALL_ARMS | 0–680 cm | 11 per member / 1.0 s (44 DPS at 4 members); magazine 30, reload 3 s (I values) | hitscan, tracking fast | 3 | GROUND, AIR_LOW | 100 % / 15 % / 2 % (never auto) / 25 % rotary only / 8 % (never auto) |
| `ability.shared.grenade` (I) | HIGH_EXPLOSIVE (P) | 650 cm (I) | 50 (I), radius 150 cm (I) | lobbed (I) | 8 | GROUND, STRUCTURE | 110 % / 45 % / 12 % / — / 100 % |

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.shared.grenade` | Active (I) | Lobbed HE grenade (implemented wind-up 2 s, reload 3 s) | Ordered on an observed hostile unit or garrisoned structure | free · instant · 3 s reload (I) · 650 cm (I) | Scatter, garrison exit, killing the squad during the 2 s wind-up | Throw animation, arc preview, impact VFX | Use on garrisons or ≥ 3 infantry inside the radius |
| `ability.shared.sprint` | Active (I) | ×1.5 speed for 4 s (I) | Self | free · 4 s · 12 s (I) | — | Footstep tempo, icon | Use to reach cover when Shaken, or to escape when Breaking |
| `ability.ac.inf_01.forward_observer` | Upgrade-unlocked passive (P) | Indirect fire at targets this squad observes uses scatter ×0.6 (instead of the generic observed ×0.7) | Automatic while the squad has line of sight | requires `tech.shared.tactical_radio_modernization` · continuous · — · own sight | Kill or suppress the radio member (Pinned disables it); jamming fields cut it (§9.5) | Radio icon on the health bar when active | Hold Ground near the front when friendly artillery is in range |

**Research and upgrades:** `tech.shared.infantry_protective_equipment`: member HP 75 → 90 at −5 % speed (weight trade-off). `tech.shared.tactical_radio_modernization`: unlocks *Forward Observer*. `tech.shared.field_medical_procedures`: replenishment time −30 %.

**Counters:** *Beats (hard):* agents, unarmed scouts, support infantry in the open. *Beats (soft):* motorcycle and truck scouts (RD-ARM-01, MP-ARM-01), and small Directorate platoons in cover. *Beaten by (hard):* flame (IA-INF-05, RD-ARM-10, IA-ARM-07), HMG teams (RD-INF-03), mortars and artillery. *Beaten by (soft):* massed Directorate platoons on cost efficiency. *Situational:* cannot hurt heavy armor, so it needs AC-INF-03 support. Most valuable when holding urban points and spotting for AC-ARM-07 and AC-ARM-08.

**Status:** identity and role E-R; Concord Discipline exemption E; grenade, sprint, reload, magazine and orders I (generic infantry behavior, not this roster unit); all numbers and abilities P.

### AC-INF-02 — Urban Security Team

| Field | Specification |
| --- | --- |
| **Identity** | `unit.ac.inf_02` · Atlantic Concord · Infantry / close-assault security team · `TPL-INF-LINE` · **Extended** · Playable |
| **Role** | **Primary:** close-range building clearing and checkpoint defense (E-R). **Secondary:** escort of agents, engineers and Psy-Ops officers inside towns; base-gate security. **Doctrine fit:** small professional teams that hold urban junctions and protect high-value assets rather than mass assault (E: faction description). |
| **Inspiration and background** | 1980s Western military-police and counter-terrorist security teams: pump-action shotguns, submachine guns, body armour vests and riot-control kit. In-universe, they protect Concord embassies, depots and checkpoints under host-nation permissions. C&C lineage (E-R): GDI Shotgun Trooper (Ren); Peacekeeper (RA3). |
| **Visual identity** | Concept: [`AC-INF-02.png`](../../Factions/TheAtlanticConcord/Units/AC-INF-02.png). Silhouette: four members, shotgun and vest outline, white "MP"-style armband over the pale-blue band; one member carries a door-breaching ram. |
| **Statistics** | 4 × 70 HP = **280** (`armor.inf_light`) · FOOT 210 cm/s · F0 · sight **1100 cm** · `DETECT.SPY` 300 cm while stationary under Guard (*Checkpoint Inspection*) · 1 slot · food staff 4 · fuel none · ExperienceValue 38 |
| **Commands** | as AC-INF-01 · Garrison, Breach and Clear (P) · default stance Guard · formation role `escort` |
| **Production** | **$380** · producer `AC-BLD-04` · prerequisites: `AC-BLD-04` operational · **10 s** · 4 food · replenishment per §10.3 · no cap |
| **Veterancy** | Standard |
| **AI** | `ai.unit.line_infantry`. Deltas: preferred distance 70 % of max range; prefer garrisoned and in-building targets ×1.5; never auto-chase beyond 600 cm from the guarded point under Guard |
| **Multiplayer** | MPA-STD. *Checkpoint Inspection* reveal is a normal detection event; it must not leak disguised archetypes outside 300 cm |
| **Presentation** | ANIM-INF + door breach, shotgun pump reload. Shotgun blast VFX with short tracer cone; whistle and challenge voice lines ("Halt — papers"). Icon: shotgun over a barrier |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.ac.inf_02.primary` shotguns and SMGs | SMALL_ARMS | 0–380 cm | 24 per member / 1.2 s (80 DPS at 4 members); magazine 8, reload 4 s | hitscan, tracking fast | 4 | GROUND | 100 % / 15 % / 2 % / — / 8 % |
| `ability.shared.grenade` (I) | HIGH_EXPLOSIVE | 650 cm (I) | 50 (I), 150 cm (I) | lobbed (I) | 8 | GROUND, STRUCTURE | as AC-INF-01 |

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.ac.inf_02.breach_and_clear` | Active (P) | The team enters a garrisoned hostile or neutral-permitted building; for 6 s its shotguns deal +60 % to occupants and occupants cannot use the +15 % port-range bonus against it. Survivors are ejected when the clearance completes with the team inside | Adjacent (≤ 150 cm) structure with hostile occupants; Unaligned buildings only with scenario permission (E) | free · 6 s · 30 s · contact | Flame or grenades against the building while the team is inside (garrison 200 % INCENDIARY applies to them too); occupants' own shotgun-class units | Door-breach animation, flash-bang VFX, building outline pulse | Use when a building holds ≥ 1 hostile squad and friendly HP ≥ 60 % |
| `ability.ac.inf_02.checkpoint_inspection` | Conditional passive (P) | `DETECT.SPY` at 300 cm while stationary ≥ 3 s under Guard | Self | free · continuous · — · 300 cm | Agents routing around the checkpoint; killing or suppressing the team (Pinned disables it) | Barrier icon; disguised units that enter range show a "?" before reveal | Station at gates and bridges in own territory |
| `ability.shared.sprint` | Active (I) | ×1.5 for 4 s | Self | free · 4 s · 12 s | — | — | Close the distance to garrisons |

**Research and upgrades:** `tech.shared.infantry_protective_equipment`: member HP 70 → 85 at −5 % speed. `tech.ac.urban_security_procedures` (AC-BLD-04, $500, one lane for 45 s): *Checkpoint Inspection* radius 300 → 450 cm; trade-off: the team's Guard leash shrinks to 400 cm.

**Counters:** *Beats (hard):* garrisoned infantry when breaching; agents near checkpoints (AC-INF-11-class enemies such as RD-INF-11, IA-INF-09). *Beats (soft):* line infantry at under 400 cm, MP-INF-08 saboteurs. *Beaten by (hard):* anything that outranges it in the open: marksmen (RD-INF-09, IA-INF-08), HMG (RD-INF-03), artillery. *Beaten by (soft):* rifle squads at 600 cm. *Situational:* a defensive and urban unit; weak in open fields.

**Status:** identity and role E-R; I generic infantry behavior; everything else P.

### AC-INF-03 — Shoulder Missile Team

| Field | Specification |
| --- | --- |
| **Identity** | `unit.ac.inf_03` · Atlantic Concord · Infantry / anti-armor team with limited air defense · `TPL-INF-AT` · **Core** · Playable |
| **Role** | **Primary:** anti-armor infantry (E-R). **Secondary:** limited short-range anti-air against helicopters and slow aircraft (E-R); anti-structure fire. **Doctrine fit:** few, expensive, guided weapons that let small Concord detachments hold against armor (E). |
| **Inspiration and background** | 1980s Western man-portable guided anti-tank launchers and shoulder-fired air-defense missiles in mixed teams. Equipment class only. C&C lineage (E-R): Rocket Soldier (TD); Missile Defender (Gen); Javelin Soldier (RA3). |
| **Visual identity** | Concept: [`AC-INF-03.png`](../../Factions/TheAtlanticConcord/Units/AC-INF-03.png). Silhouette: three members, two long launcher tubes on the shoulder; launcher tubes painted with a blue tip band. |
| **Statistics** | 3 × 70 HP = **210** (`armor.inf_light`) · FOOT 200 cm/s · F0 · sight 1200 cm · no detector · 1 slot · food staff 3 · fuel none · ExperienceValue 56 |
| **Commands** | as TPL-INF-AT; Switch Mode (AT ↔ AA) (P) · default stance Guard · formation role `support_weapon` |
| **Production** | **$560** (specialist band upper part) · `AC-BLD-04` · prerequisites: `AC-BLD-05` operational (missile supply chain) · **11 s** · 3 food · no cap |
| **Veterancy** | Standard |
| **AI** | `ai.unit.anti_armor_infantry`. Deltas: switch to AA mode when an observed hostile AIR_LOW unit is within 900 cm and no friendly AA unit covers the team; prefer flank shots (targets whose facing is > 90° away) |
| **Multiplayer** | MPA-STD. Mode is replicated as a visible status (launcher raised) |
| **Presentation** | ANIM-INF + launcher aim, backblast, reload of tube. Missile smoke trail; distinct warble tone in AA mode. Icon: launcher over a tank silhouette, with a jet silhouette in AA mode |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.ac.inf_03.at` guided AT missile | SHAPED_CHARGE | 100–820 cm | 60 per launcher / 3.5 s (2 launchers: 34 raw DPS); magazine 1, reload is the cooldown | guided_missile 1600 cm/s (jammable, §6.6) | 2 | GROUND, STRUCTURE | 25 % (never auto) / 100 % / 90 % / — / 70 % |
| `weapon.ac.inf_03.aa` shoulder SAM (AA mode) | FLAK_AA | 0–950 cm | 45 per launcher / 5 s | guided_missile 2200 cm/s | — | AIR_LOW only | — / — / — / 110 % rotary, 100 % low fixed-wing / — |
| `weapon.ac.inf_03.sidearm` carbine (third member) | SMALL_ARMS | 0–600 cm | 8 / 1.0 s | hitscan | 2 | GROUND | 100 % / 15 % / 2 % / — / 8 % |

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.ac.inf_03.switch_mode` | Active (P) | Toggle between AT and AA launchers; 2 s transition during which neither fires | Self | free · 2 s · 4 s · — | Mixed air-ground attacks that force repeated switching | Launcher swap animation, mode icon | Per AI delta above |
| `ability.shared.sprint` | Active (I) | ×1.5 for 4 s | Self | free · 4 s · 12 s | — | — | Relocate after 3 volleys if a hostile tank turret faces the team |

**Research and upgrades:** `tech.ac.precision_vehicle_targeting`: AT missile tracking unaffected by target speed and +60 cm range; trade-off: $700 and the AC-BLD-16 lane for 60 s. `tech.shared.electronic_countermeasures`: guided missiles keep 50 % of their accuracy inside enemy jamming fields (scatter max 150 cm instead of 300).

**Counters:** *Beats (hard):* light vehicles, APCs and scouts (RD-ARM-01/02/03, MP-ARM-01/02/04, IA-ARM-01/02); helicopters in AA mode (RD-AIR-03, MP-AIR-02). *Beats (soft):* MBTs from cover or flanks (RD-ARM-04, IA-ARM-04, MP-ARM-06). *Beaten by (hard):* HMG and flame (RD-INF-03, IA-INF-05), mortars (RD-INF-05, MP-INF-04), rifle squads (25 % effectiveness). *Beaten by (soft):* jamming (RD-SUP-06, IA-ARM-14). *Situational:* AA mode cannot reach AIR_HIGH jets; needs AC-ARM-09 for that.

**Status:** identity, role and AA limitation E-R; generic infantry behavior I; everything else P.

### AC-INF-04 — Grenadier Section

| Field | Specification |
| --- | --- |
| **Identity** | `unit.ac.inf_04` · Atlantic Concord · Infantry / area-suppression squad · `TPL-INF-LINE` · **Core** · Playable |
| **Role** | **Primary:** area suppression and assault of occupied structures (E-R). **Secondary:** smoke screening for withdrawals; anti-clump fire against massed infantry. **Doctrine fit:** gives a small professional force the area effect it otherwise lacks against cheap mass infantry (E: Directorate infantry far cheaper). |
| **Inspiration and background** | Rifle-grenade and under-barrel/stand-alone 40 mm grenade launchers of 1980s Western infantry. C&C lineage (E-R): Grenadier (TD); Grenadier Squad (TW). |
| **Visual identity** | Concept: [`AC-INF-04.png`](../../Factions/TheAtlanticConcord/Units/AC-INF-04.png). Silhouette: four members with stubby wide-barrel launchers and bandoliers of rounds. |
| **Statistics** | 4 × 70 HP = **280** (`armor.inf_light`) · FOOT 205 cm/s · F0 · sight 1200 cm · 1 slot · food staff 4 · fuel none · ExperienceValue 40 |
| **Commands** | as AC-INF-01 + Fire Mission (attack ground, P) · default stance Aggressive · formation role `support_weapon` |
| **Production** | **$400** · `AC-BLD-04` · prerequisites: `AC-BLD-04` operational · **10 s** · 4 food · no cap |
| **Veterancy** | Standard; Hardened and Elite scatter reductions matter most for this unit |
| **AI** | `ai.unit.line_infantry` with weights 1.4 / 0.5 / 0.1 / 0 / 1.0. Deltas: prefer targets where ≥ 3 enemy squads or a garrison lie inside 150 cm; never fire when expected friendly share in the AoE exceeds 25 % |
| **Multiplayer** | MPA-STD; lobbed shots use the server scatter stream (§6.6) |
| **Presentation** | ANIM-INF + launcher break-open reload. Distinct "thoomp" launch and small HE puff. Icon: grenade with an arc |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.ac.inf_04.primary` 40 mm grenade launchers | HIGH_EXPLOSIVE | 150–760 cm | 20 per member / 2.0 s (40 DPS at 4 members) | lobbed 900 cm/s; full 60 cm, max 150 cm, edge 40 %; scatter 40–120 cm | 8 | GROUND, STRUCTURE | 110 % / 45 % / 12 % / — / 100 % |
| `weapon.ac.inf_04.sidearm` carbines (inside 150 cm) | SMALL_ARMS | 0–400 cm | 6 per member / 1.0 s | hitscan | 2 | GROUND | 100 % / 15 % / 2 % / — / 8 % |

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.ac.inf_04.smoke_rounds` | Active (P) | Fires a smoke cylinder (§9.5: 400 cm, 20 s; blocks line of sight and fire; units inside count as Concealed) | Ground point within 760 cm | free · 20 s · 40 s · 760 cm | Detectors (`DETECT.CONCEALED`), artillery on the smoke, waiting it out | Grey smoke VFX; cylinder outline for owner and allies | Use to cover a friendly Withdraw or a breaching engineer |
| `ability.shared.grenade` (I) | Active | Standard grenade (in addition to launchers) | as AC-INF-01 | as AC-INF-01 | as AC-INF-01 | as AC-INF-01 | Garrisons only |
| `ability.shared.sprint` | Active (I) | ×1.5 for 4 s | Self | free · 4 s · 12 s | — | — | Escape when Breaking |

**Research and upgrades:** `tech.shared.infantry_protective_equipment`: member HP 70 → 85 at −5 % speed. `tech.ac.improved_grenade_munitions` (AC-BLD-16, $600, 50 s): edge damage 40 % → 60 %; trade-off: the minimum range rises to 200 cm.

**Counters:** *Beats (hard):* massed and garrisoned infantry (RD-INF-01/02/07 conscript mass; IA-INF-01 in buildings); HMG teams (RD-INF-03) once observed. *Beats (soft):* light vehicles at 45 %. *Beaten by (hard):* marksmen (RD-INF-09, IA-INF-08, MP-INF-07), armored vehicles that close within 150 cm (MBTs, IFVs). *Beaten by (soft):* scouts that kite outside 760 cm. *Situational:* strongest in towns; smoke is the Concord's main infantry escape tool.

**Status:** identity and role E-R; generic infantry behavior I; everything else P.

### AC-INF-05 — Field Medical Corps

| Field | Specification |
| --- | --- |
| **Identity** | `unit.ac.inf_05` · Atlantic Concord · Infantry / medical support team · `TPL-INF-SUPPORT` · **Core** · Playable |
| **Role** | **Primary:** restore squad casualties and resist attrition (E-R). **Secondary:** field replenishment far from producers; stabilise squads against suppression. **Doctrine fit:** the clearest unit expression of "preservation, recovery and repair" (E: Strategic Philosophy). |
| **Inspiration and background** | Western combat medics and casualty collection teams with stretchers and aid bags. C&C lineage (E-R): Medic (RA1); Field Medic (TS); Ambulance (Gen). |
| **Visual identity** | Concept: [`AC-INF-05.png`](../../Factions/TheAtlanticConcord/Units/AC-INF-05.png). Silhouette: two members, one with a folded stretcher and white medical brassards (the team is unarmed, so the protective emblem is never shown on an armed unit). |
| **Statistics** | 2 × 60 HP = **120** (`armor.inf_light`) · FOOT 215 cm/s · F0 · sight 1000 cm · 1 slot · food staff 2 · fuel none · no veterancy (non-combat) |
| **Commands** | Move, Stop, Guard (I) · Heal, Casualty Replacement, Withdraw (P) · default stance Hold Fire · formation role `protected` |
| **Production** | **$420** · `AC-BLD-04` · prerequisites: `AC-BLD-04` operational · **10 s** · 2 food · no cap |
| **Veterancy** | none (non-combat; §8) |
| **AI** | `ai.unit.support_infantry`. Deltas: prioritise squads with missing members, then lowest HP ratio; never enter a smoke cylinder that contains hostiles |
| **Multiplayer** | MPA-STD. Replenishment debits USD exactly once per member restored (UNIT-02) |
| **Presentation** | ANIM-INF + kneel-and-treat loop, stretcher carry. Green "+" heal pulse VFX visible to owner and allies only; enemies see the animation. Icon: medical bag |

**Weapons:** none (unarmed).

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.ac.inf_05.heal` | Automated / active (P) | 6 HP/s to one friendly infantry squad, capped at living members × member HP (§10.3) | Friendly or allied infantry ≤ 300 cm; not while the medic is Pinned | free · continuous · — · 300 cm | Killing the medic first; AoE fire on the treatment point | Heal beam pulse; HP bar ticks | Under Guard, auto-heal within 600 cm |
| `ability.ac.inf_05.casualty_replacement` | Active (P) | Restores missing members of one adjacent Concord squad in the field at the §10.3 price (`missing/members × price × 0.8`), 2.5 s per member (field rate, slower than a producer's 1.5 s) | Own infantry squad ≤ 200 cm that has not taken damage for 5 s | USD as stated · 2.5 s per member · 20 s · 200 cm | Interrupt by damage (channel restarts); deny the 5 s quiet window with suppression | Stretcher-team animation, member silhouettes fill in, ledger debit toast | Use when a squad is ≤ 50 % members, out of combat and ≥ 1500 cm from a producer |
| `ability.ac.inf_05.stabilise` | Passive (P) | Infantry within 400 cm recover resolve +25 % faster (stacks with group support, capped at framework cap) | Own and allied infantry | free · continuous · — · 400 cm | Killing or isolating the medic | Faint aura ring (owner only) | Stay 300 cm behind the most-stressed squad |

**Research and upgrades:** `tech.shared.field_medical_procedures`: heal 6 → 8 HP/s and replenishment time −30 %; trade-off: research lane and $600. No combat upgrade.

**Counters:** *Beats:* nothing (unarmed). *Enables:* attrition wars against RD and MP raiders. *Beaten by (hard):* marksmen and raiders (RD-INF-09, MP-ARM-01/02, IA-INF-08), artillery on the treatment point. *Situational:* most valuable on long fronts far from AC-BLD-04.

**Status:** identity and role E-R; heal and replenishment rules are framework P; everything else P.

### AC-INF-06 — Combat Engineering Team

| Field | Specification |
| --- | --- |
| **Identity** | `unit.ac.inf_06` · Atlantic Concord · Infantry / engineer · `TPL-INF-SUPPORT` · **Core** · Playable |
| **Role** | **Primary:** repair or capture authorized facilities; clear obstacles (E-R). **Secondary:** clear mines and wreck obstructions; field repair of vehicles; build sandbag cover. **Doctrine fit:** protects and restores expensive equipment and infrastructure (E). |
| **Inspiration and background** | Western combat engineer squads with tool kits, mine detectors and demolition-clearing kit for obstacles (no hostile attached charges; E). C&C lineage (E-R): Engineer (RA1); Engineer (TW); Hotwire (Ren). |
| **Visual identity** | Concept: [`AC-INF-06.png`](../../Factions/TheAtlanticConcord/Units/AC-INF-06.png). Silhouette: three members with a mine detector, tool roll and engineer's pick; yellow-and-blue engineer arm patch. |
| **Statistics** | 3 × 60 HP = **180** (`armor.inf_light`) · FOOT 205 cm/s · F0 · sight 1000 cm · detects mines within 300 cm · 1 slot · food staff 3 · fuel none · no veterancy (support) |
| **Commands** | Move, Stop, Guard (I) · Capture, Repair, Clear, Build Cover, Withdraw (P) · default stance Hold Fire · formation role `protected` |
| **Production** | **$450** · `AC-BLD-04` · prerequisites: `AC-BLD-04` · **12 s** · 3 food · no cap |
| **Veterancy** | none |
| **AI** | `ai.unit.support_infantry`. Deltas: under Guard, auto-repair damaged friendly vehicles within 600 cm when no medic task competes; never capture without an explicit order (anti-exploit and sovereignty) |
| **Multiplayer** | MPA-STD. Capture progress is server state; interruption and ownership change are atomic events |
| **Presentation** | ANIM-INF + work loop (repair sparks, digging). Wrench icon; capture progress ring visible to all observers of the structure |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.ac.inf_06.sidearm` pistols and carbines | SMALL_ARMS | 0–450 cm | 6 per member / 1.2 s (self-defense) | hitscan | 1 | GROUND | 100 % / 15 % / 2 % / — / 8 % |

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.shared.capture_structure` | Active (P, §10.6) | Transfers ownership of an eligible structure after an uninterrupted 8 s channel | Eligible hostile or neutral-permitted structure ≤ 100 cm; Unaligned only under scenario and jurisdiction rules (E) | free · 8 s · — · contact | Any damage interrupts; garrison inside blocks capture until cleared; `DETECT`-independent | Capture ring, owner-change flash and alert to the victim | Only on explicit order or authored AI objective |
| `ability.shared.field_repair` | Active / automated (P, §10.2) | 8 HP/s to one vehicle, 25 % of unit cost per full repair, charged per HP | Friendly or allied vehicle ≤ 300 cm | USD per HP · continuous · — · 300 cm | Killing the team; suppression pauses work | Sparks VFX; ledger tick | Auto under Guard |
| `ability.ac.inf_06.clear_obstacle` | Active (P) | Removes a mine, a field obstacle, roadblock or wreck within 6 s (structures excluded) | Mine, obstacle or wreck ≤ 150 cm | free · 6 s · — · 150 cm | Covering fire on the obstacle | Detector sweep animation; mine marker removal | Clear mines on the friendly route when detected |
| `ability.ac.inf_06.build_cover` | Active (P, U-08) | Places a light-cover sandbag volume (−25 % SMALL_ARMS, PRECISION, HE for infantry inside) for 180 s | Ground point ≤ 200 cm on open terrain | $40 · 10 s build · 30 s · 200 cm | HE and grenades destroy it (200 HP STR_SOFT) | Sandbag mesh | Build where a friendly squad holds a Guard point for > 30 s |

**Research and upgrades:** `tech.shared.vehicle_component_standardization`: field repair USD −20 %. `tech.ac.field_fortification` (AC-BLD-16, $500, 40 s): *Build Cover* creates heavy cover (−50 %); trade-off: 20 s build and $80.

**Counters:** *Beats:* nothing in combat. *Beaten by (hard):* any armed unit; raiders and marksmen. *Situational:* decisive for capture objectives, minefields and forward repair; Unaligned property is never a free capture target.

**Status:** identity and role E-R; capture is an 8 s channel (framework P); sabotage restriction E; everything else P.

### AC-INF-07 — Airborne Assault Section

| Field | Specification |
| --- | --- |
| **Identity** | `unit.ac.inf_07` · Atlantic Concord · Infantry / air-assault rifle squad · `TPL-INF-LINE` · **Extended** · Playable |
| **Role** | **Primary:** fast-deployed riflemen for limited heliborne or parachute operations (E-R). **Secondary:** seize bridges, junctions and objectives behind the line; reinforce isolated positions. **Doctrine fit:** air-mobile forces respond quickly when airfields and transport corridors are available (E: Military Doctrine). |
| **Inspiration and background** | Western airborne and air-assault light infantry: lighter carbines, parachute harness, minimal heavy kit. C&C lineage (E-R): Paratroopers (RA1); GDI Airborne (TW); Century Bomber (RA3, transport role). |
| **Visual identity** | Concept: [`AC-INF-07.png`](../../Factions/TheAtlanticConcord/Units/AC-INF-07.png). Silhouette: four members with jump helmets and reserve chute packs; maroon-tinted beret silhouette on the squad leader. |
| **Statistics** | 4 × 70 HP = **280** (`armor.inf_light`) · FOOT 215 cm/s · F0 · sight 1300 cm · 1 slot · food staff 4 · fuel none · ExperienceValue 42 |
| **Commands** | as AC-INF-01 · Paradrop and Fast-Rope eligible (P) · default stance Aggressive · formation role `flank` |
| **Production** | **$420** · `AC-BLD-04` · prerequisites: `AC-BLD-06` or `AC-BLD-07` operational · **11 s** · 4 food · no cap |
| **Veterancy** | Standard |
| **AI** | `ai.unit.line_infantry`. Deltas: after an air insertion, move to the nearest authored objective or bridge within 1500 cm and Guard it; retreat threshold 30 % (it cannot easily withdraw) |
| **Multiplayer** | MPA-STD. Drop zones are resolved server-side; enemies see the transport, then the landing squad when observed |
| **Presentation** | ANIM-INF + parachute landing, fast-rope descent. Canopy VFX visible to observers of the drop zone. Icon: parachute over a rifle |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.ac.inf_07.primary` carbines + LMG | SMALL_ARMS | 0–620 cm | 10 per member / 1.0 s (40 DPS); magazine 30, reload 3 s | hitscan | 3 | GROUND, AIR_LOW | 100 % / 15 % / 2 % / 25 % rotary / 8 % |
| `ability.shared.grenade` (I) | HIGH_EXPLOSIVE | 650 cm | 50, 150 cm | lobbed | 8 | GROUND, STRUCTURE | as AC-INF-01 |

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.ac.inf_07.airborne_qualified` | Passive (P) | Only units with this flag may Paradrop from AC-AIR-06; Fast-Rope exposure window from AC-AIR-05 is 6 s for this squad instead of 10 s | Self | free · — · — · — | AA in the corridor; ground fire on the drop zone during the window | Parachute icon on the card | Request air insertion for objectives ≥ 2500 cm away |
| `ability.ac.inf_07.objective_seizure` | Conditional passive (P) | For 30 s after landing, +25 % resolve recovery and the squad counts as a coherent group of 3 for support purposes | Self, after Paradrop or Fast-Rope | free · 30 s · per insertion · self | Contact during the landing window | Brief chevron glow | — |
| `ability.shared.sprint` | Active (I) | ×1.5 for 4 s | Self | free · 4 s · 12 s | — | — | Clear the drop zone |

**Research and upgrades:** `tech.shared.infantry_protective_equipment` does **not** apply (jump weight limit). Instead `tech.ac.airborne_first_aid_kits` (AC-BLD-04, $500, 40 s): out-of-combat self-heal 0.5 % HP/s after 8 s without damage at any rank; trade-off: the squad's replenishment price rises 10 %. `tech.ac.aviation_servicing`: transports that carry this squad turn around 20 % faster.

**Counters:** *Beats (hard):* artillery, logistics and support units behind lines (RD-ARM-07/08, MP-SUP-02, IA-SUP-04). *Beats (soft):* lightly held objectives. *Beaten by (hard):* AA covering the corridor (RD-ARM-06/13, IA-ARM-08, MP-ARM-09), HMG and flame at the landing zone. *Situational:* its value is the insertion, not the firefight; it is ordinary infantry once landed.

**Status:** identity and role E-R; generic infantry behavior I; everything else P.

### AC-INF-08 — Long-Range Marksman Pair

| Field | Specification |
| --- | --- |
| **Identity** | `unit.ac.inf_08` · Atlantic Concord · Infantry / marksman and observation team · `TPL-INF-SPEC` · **Core** · Playable |
| **Role** | **Primary:** stealthy observation and precision anti-infantry shots (E-R). **Secondary:** artillery and air spotting; officer and specialist removal; detection of concealed infantry. **Doctrine fit:** "identify opposing formations early" and make few precise weapons decisive (E). |
| **Inspiration and background** | Two-person sniper/observer team with a bolt-action or semi-automatic precision rifle, spotting scope and radio. C&C lineage (E-R): Sniper (RA2); Deadeye (Ren); Sniper Team (TW). |
| **Visual identity** | Concept: [`AC-INF-08.png`](../../Factions/TheAtlanticConcord/Units/AC-INF-08.png). Silhouette: two members, one with a long scoped rifle, one with a spotting scope; scrim-netted helmets. |
| **Statistics** | 2 × 60 HP = **120** (`armor.inf_light`) · FOOT 205 cm/s · F0 · sight **1600 cm** · `DETECT.CONCEALED` 1000 cm · Concealed after 3 s stationary (§9.1) · 1 slot · food staff 2 · fuel none · ExperienceValue 65 |
| **Commands** | as TPL-INF-SPEC + Designate Target / Spot (P) · default stance Hold Fire while concealed · formation role `scout_screen` |
| **Production** | **$650** · `AC-BLD-04` · prerequisites: `AC-BLD-16` operational · **13 s** · 2 food · no cap |
| **Veterancy** | Standard; Elite scatter bonus does not apply (direct fire) |
| **AI** | `ai.unit.specialist_infantry`. Deltas: shoot only when the shot kills a member (projected damage ≥ remaining member HP) or the target is an officer, commander, AT or HW team; relocate after 2 shots |
| **Multiplayer** | MPA-STD + MPA-COVERT (concealment). Opponents receive nothing while it is concealed; firing reveals for 4 s exactly once per shot (UNIT-07 style test) |
| **Presentation** | ANIM-INF + prone/crawl, scope glint (visible only to observers that already see the unit — never a fog leak). Suppressed crack SFX. Icon: crosshair |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.ac.inf_08.primary` precision rifle | PRECISION | 0–1150 cm | 50 / 3.0 s (one shooter) | hitscan, tracking medium | 8 on a kill shot | GROUND | 160 % (80 dmg, kills one 60–75 HP member per shot) / 2 % (never) / 0 % / — / 0 % |
| `weapon.ac.inf_08.sidearm` observer carbine | SMALL_ARMS | 0–550 cm | 8 / 1.0 s | hitscan | 1 | GROUND | 100 % / 15 % / 2 % / — / 8 % |

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.shared.conceal` | Conditional (P, §9.1) | Concealed after 3 s stationary; broken by firing (4 s), fast movement or damage | Self | free · — · — · — | `DETECT.CONCEALED` sources (RD-INF-09, IA-INF-08, MP-INF-07 and scouts), observers within 35 % sight | Owner-only concealment icon | Default state |
| `ability.ac.inf_08.designate` | Active (P) | Marks an observed target or ground point for 20 s: friendly indirect fire on it uses scatter ×0.6, friendly air strikes on it gain +10 % damage | Observed point or unit ≤ 1600 cm | free · 20 s · 30 s · 1600 cm | Kill or suppress the team (Pinned cancels); smoke blocks line of sight; jamming cuts the link | Laser-dot icon visible to owner and allies only | Designate artillery clusters and fortifications when AC-ARM-07/08 or aircraft are available |

**Research and upgrades:** `tech.ac.precision_vehicle_targeting` does not apply. `tech.ac.marksman_optics` (AC-BLD-16, $500, 40 s): range 1150 → 1300 cm; trade-off: cooldown 3.0 → 3.3 s.

**Counters:** *Beats (hard):* all infantry from concealment, especially officers and support (RD-INF-10, IA-INF-12, MP-INF-13 escorts, AT teams). *Beaten by (hard):* scouts and detectors (RD-ARM-01, MP-ARM-01, IA-INF-08), any vehicle (it cannot hurt them), artillery once revealed. *Situational:* force multiplier for AC-ARM-07/08; useless against armor-only forces.

**Status:** identity and role E-R; no invisibility (E); everything else P.

### AC-INF-09 — Coastal Reconnaissance Detachment

| Field | Specification |
| --- | --- |
| **Identity** | `unit.ac.inf_09` · Atlantic Concord · Infantry / amphibious special forces · `TPL-INF-SPEC` · **Extended** (special-operations cap 2) · Playable |
| **Role** | **Primary:** amphibious commando for raids, ports and shoreline objectives (E-R). **Secondary:** beach reconnaissance before landings; target designation for naval and air fire. **Doctrine fit:** small, highly trained detachments that prepare objectives before valuable forces are committed (E). |
| **Inspiration and background** | Western naval special-warfare and combat-swimmer reconnaissance teams of the 1980s. C&C lineage (E-R): Navy SEAL (RA2/YR); Tanya (RA3); Commando (TW). |
| **Visual identity** | Concept: [`AC-INF-09.png`](../../Factions/TheAtlanticConcord/Units/AC-INF-09.png). Silhouette: three members with dive masks pushed up, fins on packs and compact suppressed weapons. |
| **Statistics** | 3 × 85 HP = **255** (`armor.inf_light`) · FOOT 215 cm/s; **swims** shallow and deep water at 130 cm/s (special mobility flag) · F0 · sight 1500 cm · `DETECT.CONCEALED` 800 cm · Concealed after 2 s stationary and while swimming slower than 50 % speed · 1 slot · food staff 3 · ExperienceValue 120 |
| **Commands** | as TPL-INF-SPEC + Swim, Designate Target, Capture (port and coastal structures only) (P) · default stance Hold Fire · formation role `scout_screen` |
| **Production** | **$1,200** · `AC-BLD-08` Adriatic Naval Station (or `AC-BLD-04` if the map has no naval station) · prerequisites: `AC-BLD-16` · **18 s** · 3 food · **cap 2** |
| **Veterancy** | Standard |
| **AI** | `ai.unit.specialist_infantry`. Deltas: approach coastal objectives by water; engage only boats, sentries and support units; withdraw to water at 50 % HP |
| **Multiplayer** | MPA-STD + MPA-COVERT. Swimming concealment uses the same Concealed rules; no submerged state |
| **Presentation** | ANIM-INF + swim cycle, wade-out, prone. Ripple VFX only when observed. Icon: trident over waves |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.ac.inf_09.primary` suppressed carbines | SMALL_ARMS | 0–600 cm | 14 per member / 1.0 s (42 DPS); firing from concealment reveals for 2 s instead of 4 s | hitscan | 3 | GROUND, WATER_SURFACE | 100 % / 15 % / 2 % / — / 8 %; NAV_LIGHT 30 % |

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.ac.inf_09.shore_reconnaissance` | Active (P) | Reveals a 1200 cm radius of shoreline (observation, not identity of concealed units) for 15 s and marks landing points for friendly AC-SEA-02 | Coastline point ≤ 300 cm | free · 15 s · 45 s · 1200 cm | Coastal detectors and patrol boats (RD-SEA-01, UN-SEA-01) | Shoreline highlight for owner/allies | Use before any amphibious landing order |
| `ability.ac.inf_09.designate` | Active (P) | As AC-INF-08 *Designate*, and AC-SEA-06 naval gunfire on the mark gets scatter ×0.5 | Observed point ≤ 1500 cm | free · 20 s · 30 s · 1500 cm | as AC-INF-08 | Laser-dot icon | Mark coastal batteries and port defenses |
| `ability.shared.capture_structure` | Active (P) | 8 s capture, **port and coastal structures only** | Eligible coastal structure | free · 8 s · — · contact | Garrisons and damage | Capture ring | Explicit order only |

**Research and upgrades:** `tech.shared.infantry_protective_equipment` (member HP 85 → 100, swim speed −10 %). No charge upgrade: Concord attached charges belong to spies only (E).

**Counters:** *Beats (hard):* patrol-boat crews and harbour support units; unescorted port logistics. *Beats (soft):* sentries and coastal infantry from concealment. *Beaten by (hard):* `DETECT.CONCEALED` patrol craft and coastal detectors, HMG on the beach, Unaligned border police (UN-INF-03). *Situational:* value only on coastal maps; the roster's "raids" are delivered by designation, capture and small arms, not by charges (U-AC-01).

**Status:** identity and role E-R; sabotage restriction E; everything else P. **U-AC-01** records the reconciliation.

### AC-INF-10 — Counterintelligence Officer

| Field | Specification |
| --- | --- |
| **Identity** | `unit.ac.inf_10` · Atlantic Concord · Infantry / counterintelligence specialist (not an officer for formation purposes, framework §13.4) · `TPL-INF-SPEC` · **Extended** · Playable |
| **Role** | **Primary:** identify infiltrators and investigate security incidents (E-R). **Secondary:** clear concealed sabotage and contamination effects from friendly buildings; escort the HQ. **Doctrine fit:** protects sensitive infrastructure and research sites, which are Concord points of failure (E). |
| **Inspiration and background** | Military security and counter-espionage officers with credentials, a sidearm and a small investigation kit. C&C lineage (E-R): Attack Dog (RA1); Attack Dog (RA3, detection role); Detention Camp (Gen). |
| **Visual identity** | Concept: [`AC-INF-10.png`](../../Factions/TheAtlanticConcord/Units/AC-INF-10.png). Silhouette: single figure in a field jacket with a document case and a sidearm. |
| **Statistics** | 1 × 130 HP = **130** (`armor.inf_light`) · FOOT 210 cm/s · F0 · sight 1300 cm · **`DETECT.SPY` 900 cm**, `DETECT.CONCEALED` 600 cm · 1 slot · food staff 1 · ExperienceValue 70 |
| **Commands** | Move, Attack, Stop, Guard (I) · Investigate, Withdraw (P) · default stance Guard · formation role `protected` |
| **Production** | **$700** · `AC-BLD-04` · prerequisites: `AC-BLD-16` · **14 s** · 1 food · no cap |
| **Veterancy** | Standard (it can kill revealed agents) |
| **AI** | `ai.unit.specialist_infantry` with weights: agents 2.0, other infantry 0.4. Deltas: patrol between producer, research and nuclear buildings under Guard; investigate any friendly building whose output dropped unexpectedly (owner-visible signal only) |
| **Multiplayer** | MPA-STD. Detection reveals the true archetype exactly once to the detecting team (UNIT-07); *Investigate* results are owner-only |
| **Presentation** | ANIM-INF + inspect loop. Magnifier status icon over revealed agents. Icon: eye over a shield |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.ac.inf_10.sidearm` service pistol | SMALL_ARMS | 0–450 cm | 16 / 1.0 s; +100 % against revealed covert agents (special-target bonus) | hitscan | 2 | GROUND | 100 % / 15 % / 2 % / — / 8 % |

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.ac.inf_10.detect_spy` | Passive (P) | `DETECT.SPY` 900 cm; revealed agents lose Disguise | Self | free · continuous · — · 900 cm | Killing the officer at range; agents routing around | Revealed-agent icon for owner/allies | Default |
| `ability.ac.inf_10.investigate` | Active (P) | After 6 s inside or adjacent to a friendly building: reveals and removes hidden attached charges; ends concealed resource waste or diversion and attributes it to the source participant; contains facility contamination (stops spread from that building) | Own building ≤ 150 cm | free · 6 s channel · 45 s · contact | Killing the officer; repeated operations faster than the cooldown | Audit result panel (owner); attribution notice to owner only | Use on any building with an unexplained production or stock drop |

**Research and upgrades:** `tech.shared.radar_signal_processing` does not apply. `tech.ac.security_vetting` (AC-BLD-16, $600, 45 s): `DETECT.SPY` 900 → 1200 cm; trade-off: officer price +$100.

**Counters:** *Beats (hard):* agents and saboteurs (RD-INF-11/12, IA-INF-09/10/11, MP-INF-08/09). *Beaten by (hard):* any combat unit; marksmen. *Situational:* the cheapest Concord insurance for nuclear and research sites; it does not detect Unaligned planted *reports* (those are intelligence objects, E).

**Status:** identity and role E-R; espionage outcome rules E (Espionage §1.3); everything else P.

### AC-INF-11 — Concord Field Agent

| Field | Specification |
| --- | --- |
| **Identity** | `unit.ac.inf_11` · Atlantic Concord · Infantry / covert agent · `TPL-INF-COVERT` · **Conditional** (espionage track) · Playable |
| **Role** | **Primary:** later-game spy for intelligence collection and expensive, spy-only attached-charge sabotage (E-R, E). **Secondary:** infiltrate to reveal production and research; scout under disguise. **Doctrine fit:** intelligence leads operations; sabotage is a costly, late, precise tool (E: Espionage §1.4). |
| **Inspiration and background** | Civilian-cover intelligence officers of the Cold War. Abstract game interactions only. C&C lineage (E-R): Spy (RA1); Spy (RA2); Shadow Team (TW). |
| **Visual identity** | Concept: [`AC-INF-11.png`](../../Factions/TheAtlanticConcord/Units/AC-INF-11.png). Silhouette (true form, owner only): trench coat and briefcase. Opponents see the disguise archetype mesh and animation set. |
| **Statistics** | 1 × 100 HP = **100** (`armor.inf_light`) · FOOT 215 cm/s · F0 · sight 1200 cm · no detector · 1 slot · food staff 1 · ExperienceValue 130 |
| **Commands** | Move, Stop (I) · Disguise, Infiltrate, Plant Charge, Withdraw (P) · default stance Hold Fire · formation role `protected` |
| **Production** | **$1,300** · `AC-BLD-04` · prerequisites: `AC-BLD-16` and `AC-BLD-18` operational · **20 s** · 1 food · cap 3 active |
| **Veterancy** | none (agents do not earn XP from operations; §8) |
| **AI** | `ai.unit.covert_agent`. Deltas: AI uses infiltration only against observed producer or research buildings with no known `DETECT.SPY` source inside 1200 cm |
| **Multiplayer** | **MPA-COVERT**. True archetype, charges and fuse timers never replicate to opponents until detection or detonation; animation, voice and footstep sets follow the disguise (exploit surface) |
| **Presentation** | ANIM-INF + civilian walk. Owner sees a translucent true-form overlay. Icon: hat and briefcase |

**Weapons:** none (unarmed; attached charges are an ability).

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.shared.disguise` | Active (P) | Opponents receive a chosen disguise archetype (Unaligned civilian on foot, or an observed enemy line-infantry archetype) | Self; enemy archetype must have been observed this match | free · until broken · 10 s after reveal · — | `DETECT.SPY` (AC-INF-10, RD-INF-07, IA-INF-12, UN-INF-10/11 strongest), hostile action | Owner-only mask icon | Re-disguise when out of detector range |
| `ability.ac.inf_11.infiltrate_intel` | Active (P) | 12 s inside an eligible enemy producer or research building: reveals that participant's production queue and completed technologies for 90 s, and grants one *intelligence token* for that building class. The agent exits disguised | Enemy producer, research or HQ building | $200 · 12 s · 60 s · contact | `DETECT.SPY` inside 900 cm aborts; garrison or guard units; Unaligned false reports may pollute later readings (E) | Owner panel; victim receives no alert unless detected | Use on the highest-tier observed building |
| `ability.shared.plant_charge` | Active (P; E permission) | Attaches a fictional charge: after a 15 s fuse, DEMOLITION 450 to the vehicle or building | Requires `tech.ac.late_espionage`; eligible enemy vehicle or building ≤ 100 cm | **$300 per charge** (≥ 2× the Meridian non-spy charge, E) · 15 s fuse · 45 s · contact; 2 carried, refilled at AC-BLD-04 | `DETECT.SPY` reveals the agent and lets the victim see the charge; AC-INF-10-class *Investigate* removes it; moving vehicles must be stationary for the 3 s attach | Owner sees the fuse timer; victim sees nothing until detected | Only on a stationary high-value target with no detector in 1200 cm |

**Research and upgrades:** `tech.ac.late_espionage` (AC-BLD-16, $1,200, 90 s; maps to I `late_espionage_unlocked`): enables *Plant Charge*. `tech.ac.tradecraft` ($700): infiltration 12 → 9 s; trade-off: agent price +$150.

**Counters:** *Beats:* static infrastructure; intelligence denial. *Beaten by (hard):* `DETECT.SPY` sources and their escorts; Unaligned counterintelligence (UN-INF-10/11). *Situational:* Directorate agents remain slightly faster and cheaper at intelligence theft (E: RD-INF-11).

**Status:** identity and role E-R; spy-only, later and more costly charges E; `CanPlantExplosives` predicate I; everything else P.

### AC-INF-12 — Psychological Operations Officer

| Field | Specification |
| --- | --- |
| **Identity** | `unit.ac.inf_12` · Atlantic Concord · Infantry / Psy-Ops officer (offensive espionage; no talents) · `TPL-INF-COVERT` · **Conditional** · Playable |
| **Role** | **Primary:** exclusive Concord Psy-Ops: eligible resolve breakdown, a bounded friendly-fire rampage, or facility disruption; not mind control (E-R, E). **Secondary:** amplify Discipline damage during an assault. **Doctrine fit:** "selective disruption of defenses" before committing valuable forces (E: Strategic Philosophy). |
| **Inspiration and background** | Psychological-operations staff with loudspeaker teams, leaflets and radio — represented as an abstract game operation (E: no realistic method). C&C lineage (E-R): Yuri Clone (YR, role adapted); Mastermind (TW, role adapted); Spy (RA3). |
| **Visual identity** | Concept: [`AC-INF-12.png`](../../Factions/TheAtlanticConcord/Units/AC-INF-12.png). Silhouette: officer in field cap with a portable radio and loudspeaker; owner sees true form, opponents see a disguise when it is active. |
| **Statistics** | 1 × 110 HP = **110** (`armor.inf_light`) · FOOT 210 cm/s · F0 · sight 1300 cm · 1 slot · food staff 1 · no veterancy |
| **Commands** | Move, Stop (I) · Psy-Op (unit), Psy-Op (building), Disguise, Withdraw (P) · default stance Hold Fire · formation role `protected` |
| **Production** | **$1,600** · `AC-BLD-04` · prerequisites: `AC-BLD-18` Media Coordination Bureau and `tech.ac.psychological_operations` · **22 s** · 1 food · **cap 2** active |
| **Veterancy** | none |
| **AI** | `ai.unit.covert_agent`. Deltas: target selection prefers eligible units already Pinned or worse inside a hostile group of ≥ 3 (rampage value) or isolated Shaken units (breakdown value); never targets units outside §7.5 eligibility |
| **Multiplayer** | MPA-COVERT. Operation success, rampage expiry and building waste totals are server-only; the victim's ledger stays accurate (E); attribution hidden until discovered |
| **Presentation** | ANIM-INF + loudspeaker/radio channel. Victim units show a cracked-shield status icon when observed; rampaging units show a red outline to all observers. Icon: broadcast waves over a broken shield |

**Weapons:** none (unarmed).

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.ac.inf_12.psyops_breakdown` | Active (P; E outcome) | Applies −12 resolve per second for 6 s (−72) to one eligible unit. If the target reaches **Broken (0)** during the effect or within 10 s after, and is still `PsyOpsEligible`, it is eliminated (self-destruction event, no XP, no wreck salvage value). Otherwise resolve recovers normally | Observed `PsyOpsEligible` enemy unit (§7.5) ≤ 700 cm; 4 s channel with line of sight | $250 · 4 s + 6 s · **75 s** · 700 cm | Officers' rally auras (RD-INF-10, IA-INF-12), group support recovery, interrupting the channel by damage, `DETECT.SPY` before channel, killing the officer | Channel beam (owner/allies); victim's resolve icon flickers | Use on isolated or Shaken eligible targets whose projected resolve after −72 is ≤ 0 |
| `ability.ac.inf_12.psyops_rampage` | Active (P; E outcome) | Target must be at **Pinned or lower (≤ 34)**: for 10 s it treats its own faction's units as hostile and auto-acquires the nearest; ownership never changes; no XP for any kill; it then returns to ordinary targeting. Non-combat units qualify only if their definition has an attack action (E) | Observed eligible enemy unit ≤ 700 cm; 4 s channel | $350 · 10 s · **90 s**; the same victim cannot be rampaged again for 120 s · 700 cm | Victim's allies kill or disperse it; commander/officer rally restores it above 34 during the channel; Breaking/Broken units already Hold Fire (no effect beyond 10 s) | Red outline on victim for all observers; owner gets an outcome toast | Use when the target is inside a group of ≥ 3 hostile units |
| `ability.ac.inf_12.psyops_facility` | Active (P; E outcome) | Infiltrate an eligible enemy building (10 s inside). Choose **(a) Self-destruction:** building takes damage equal to 60 % of its max HP after a 20 s delay; HQ, nuclear custody and Unaligned buildings are ineligible; the officer is consumed. **(b) Resource waste:** for 120 s the owner's USD is debited 2 % of current balance per 10 s, capped at $1,500 total; debits are real and never credited to Concord (E); the officer exits | Eligible enemy producer, depot, refinery or power building | (a) $800 · 20 s · officer consumed; (b) $400 · 120 s · 120 s · contact | `DETECT.SPY` at entry aborts; *Investigate* (AC-INF-10, RD-INF-07, IA-INF-12, UN units) ends waste and attributes it; audits | Building destruction is a major observable event (E); waste shows only as ledger debits labelled "operational losses" until discovered | (a) on late-tier producers at ≤ 60 % HP to finish them; (b) when the victim's balance is ≥ $5,000 |

**Research and upgrades:** `tech.ac.psychological_operations` (AC-BLD-18, $1,500, 120 s): unlocks the unit. `tech.ac.broadcast_saturation` ($900): breakdown −12 → −14 per second; trade-off: cooldown 75 → 90 s.

**Counters:** *Beats:* isolated or suppressed ordinary units and soft vehicles; economy buildings. *Beaten by (hard):* counterintelligence escorts, officers' rally and mass Discipline bonuses (RD and IA very-massive-army bonus, E); medium and heavy armor are ineligible (§7.5). *Situational:* the Concord's answer to cheap Directorate mass when combined with AC-INF-04 suppression.

**Status:** identity, exclusivity and both outcomes E; ownership never transfers (E); rampage threshold is **U-AC-02**; all numbers P.

### AC-INF-13 — Signals Intercept Team

| Field | Specification |
| --- | --- |
| **Identity** | `unit.ac.inf_13` · Atlantic Concord · Infantry / signals intelligence team · `TPL-INF-SPEC` · **Extended** · Playable |
| **Role** | **Primary:** intercept transmissions and verify reports; vulnerable to planted Unaligned evidence (E-R). **Secondary:** locate jammers, radars and command vehicles; guide strikes onto emitters. **Doctrine fit:** "integration of sensors and command networks" (E: Science and Technology). |
| **Inspiration and background** | Man-portable direction-finding and intercept teams. C&C lineage (E-R): Hacker (Gen); Mobile Sensor Array (TS); Spy Satellite (Gen). |
| **Visual identity** | Concept: [`AC-INF-13.png`](../../Factions/TheAtlanticConcord/Units/AC-INF-13.png). Silhouette: two members with a folding directional antenna and headphones. |
| **Statistics** | 2 × 60 HP = **120** (`armor.inf_light`) · FOOT 200 cm/s · F0 · sight 1200 cm · **`DETECT.EMITTER` 3200 cm** when deployed · 1 slot · food staff 2 · no veterancy |
| **Commands** | Move, Stop, Guard (I) · Deploy Antenna, Verify Report (P) · default stance Hold Fire · formation role `protected` |
| **Production** | **$600** · `AC-BLD-04` · prerequisites: `AC-BLD-16` · **12 s** · 2 food · no cap |
| **Veterancy** | none |
| **AI** | `ai.unit.electronic_warfare`. Deltas: deploy 600–1000 cm behind the frontline; relocate if its own position is targeted by artillery |
| **Multiplayer** | MPA-STD + MPA-SENSOR. Emitter contacts are projected as contacts only (position band), never as identities |
| **Presentation** | ANIM-INF + antenna deploy (3 s). Pulsing contact markers on the minimap for owner/allies. Icon: antenna with waves |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.ac.inf_13.sidearm` carbines | SMALL_ARMS | 0–500 cm | 6 per member / 1.2 s | hitscan | 1 | GROUND | 100 % / 15 % / 2 % / — / 8 % |

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.ac.inf_13.deploy_antenna` | Active (P) | Deployed: `DETECT.EMITTER` 3200 cm; immobile; −50 % weapon cooldown speed | Self | free · 3 s deploy / 2 s undeploy · — · self | Artillery and raiders; it is itself silent (passive receiver) | Mast mesh | Deploy when an enemy jammer, radar or command vehicle is suspected |
| `ability.ac.inf_13.verify_report` | Active (P) | Checks one `Reported` intelligence object whose location lies inside its emitter footprint: returns *corroborated* (a matching emitter or observed unit exists now), *uncorroborated*, or *contradicted*. It never reveals world truth beyond current observation and cannot certify a planted Unaligned report as false unless real observation contradicts it (E-R vulnerability) | A `Reported` object in footprint | free · 8 s · 30 s · footprint | Unaligned planted evidence designed to corroborate (E); jamming suppresses contacts | Report card gets a corroboration badge | Verify any report about nuclear sites or massed armor before AI commits a strike |

**Research and upgrades:** `tech.shared.radar_signal_processing`: footprint 3200 → 3800 cm. `tech.shared.electronic_countermeasures`: contacts persist 5 s inside enemy jamming fields instead of being suppressed; trade-off: research lane.

**Counters:** *Beats:* enemy EW and radar play (RD-SUP-06, IA-ARM-14, MP-MKT-05) by locating them. *Beaten by (hard):* any combat unit; artillery. *Situational:* pairs with AC-ARM-08 and AC-AIR-02 to strike detected emitters.

**Status:** identity and vulnerability E-R; report semantics E (Observation contract, Unaligned counterintelligence); everything else P.

### AC-INF-14 — Special Operations Commander

| Field | Specification |
| --- | --- |
| **Identity** | `unit.ac.inf_14` · Atlantic Concord · Infantry / **Elite commander** (talent-eligible, E: talent doc §5) · `TPL-INF-COMMAND` · **Extended** · Playable · **cap 1** (E) |
| **Role** | **Primary:** unique high-cost infiltration and strike leader with limited availability (E-R). **Secondary:** coordination hub for precision, surveillance and combined arms (E: talent identity). **Doctrine fit:** the commander multiplies small, well-informed formations rather than fighting as a hero. |
| **Inspiration and background** | Western special-operations task-force commander of the 1980s with a radio operator. C&C lineage (E-R): Commando (TD); Havoc (Ren); Colonel Burton (Gen). |
| **Visual identity** | No dedicated sheet: none (asset requirement). Silhouette: officer with beret, carbine and a radio operator figure attached as cosmetic escort; command pennant over the head at RTS zoom. |
| **Statistics** | 1 × 480 HP = **480** (`armor.inf_protected`) · FOOT 225 cm/s · F0 · sight **1500 cm** · `DETECT.CONCEALED` 800 cm · Concealed after 3 s stationary (specialist rule) · 1 slot · food staff 2 (commander + radio operator) · no ordinary veterancy (E) |
| **Commands** | as TPL-INF-COMMAND + talent abilities · default stance Guard · formation role `formation_anchor` |
| **Production** | **$2,400** · `AC-BLD-04` · prerequisites: `AC-BLD-16` operational · **30 s** · 2 food · **cap 1 per participant** (E). Redeployment after death is not automatic: a new commander may be queued after 90 s at full price and resumes the stored appointment (E: talent doc §4.4) |
| **Veterancy** | none: commanders progress only through the talent appointment (E: talent doc §1; framework §8) |
| **Command radius** | **1200 cm** baseline. Aura links need communications: inside an enemy jamming field, links that pass through the field are cut (§9.5) |
| **Leadership effects** | Passive *Calm Command*: own units in radius recover resolve +25 % (bounded, does not stack with other commander auras) |
| **Vulnerabilities** | Low HP for its cost; jamming cuts its links; Psy-Ops and Intimidation ineligible (§7.5), but it can be killed normally; its death clears all live effects (E) |
| **AI** | `ai.unit.commander`. Deltas: stay 400–800 cm behind the frontline; retreat at 50 % HP; select talents by the doctrine score of observed enemy composition (air-heavy → Air Liaison path) |
| **Multiplayer** | MPA-STD + **MPA-COMMAND**: appointment, talent state, aura membership and OXP are server-only; opponents see commander identity and aura indicator when observed (UNIT-10 cap test) |
| **Presentation** | ANIM-INF + command gestures, radio call. Aura ring visible to owner/allies; enemies see the pennant only. Icon: star over a crosshair |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.ac.inf_14.primary` suppressed carbine | SMALL_ARMS | 0–750 cm | 28 / 0.8 s (35 DPS) | hitscan | 3 | GROUND, AIR_LOW | 100 % / 15 % / 2 % / 25 % rotary / 8 % |
| `ability.shared.grenade` (I) | HIGH_EXPLOSIVE | 650 cm | 50, 150 cm | lobbed | 8 | GROUND, STRUCTURE | as AC-INF-01 |

**Abilities and proposed talent set** (three options per tier; one choice per tier; E: tier structure, P: all effects, awaiting owner approval under talent doc §5)

| Tier | Option A | Option B | Option C |
| --- | --- | --- | --- |
| I Initial Doctrine (at deployment) | **Advanced Communications** (E sample): radius 1200 → 1600 cm; links resist jamming (cut only if the jammer is within 600 cm of the commander) | **Reconnaissance Network:** friendly scouts and marksmen in radius share `DETECT.CONCEALED` at 50 % of their sight with every friendly unit in radius | **Small-Unit Autonomy:** detached groups of ≤ 2 units within 2000 cm (not 1200) get the *Calm Command* bonus |
| II Field Specialization | **Forward Observer** (E sample): designate an observed point for 20 s; friendly indirect fire there uses scatter ×0.5; cooldown 40 s | **Secure Logistics:** repair, heal and resupply rates in radius +30 % | **Air Liaison:** friendly aircraft ordered against a target designated by the commander rearm 20 % faster on their next turnaround |
| III Advanced Command | **Coordinated Assault** (E sample): 12 s, units in radius attacking the same designated target get −15 % cooldown; cooldown 90 s | **Electronic Screen:** 20 s bubble of 800 cm in which enemy jamming has no effect on friendly units; cooldown 120 s | **Rapid Extraction:** 15 s, transports in radius load and unload at double speed and passengers ignore the Pinned forward-move lock while boarding; cooldown 120 s |
| IV Signature Operation | **Joint Operations Network** (E sample): 30 s, every friendly unit in radius shares the observations of every other unit in radius (observation only, never hidden truth); friendly indirect and air strikes in radius scatter ×0.6; cooldown 180 s | **Precision Decapitation:** mark one observed enemy structure; the next friendly air strike on it within 60 s deals +25 %; cooldown 240 s | **Strategic Withdrawal:** 15 s, own units in radius may Withdraw while Pinned and take −20 % damage while moving away from the threat; cooldown 150 s |

Every talent effect follows the commander influence invariant (talent doc §3.3): only eligible friendly subordinates inside the radius and coverage, with no effect on Unaligned or captured units, and nothing after the commander's death. The protected canon applies: no omniscience, no free airstrikes, no free nuclear bomber deployment (E).

**Research and upgrades:** research never grants talents (E). `tech.shared.tactical_radio_modernization`: radius +200 cm (applies before talents; clamp 2000 cm).

**Counters:** *Beats:* nothing on its own; multiplies precision play. *Beaten by (hard):* marksmen (RD-INF-09, IA-INF-08, MP-INF-07), artillery, jamming (RD-SUP-06, IA-ARM-14) that cuts links. *Situational:* strongest with AC-INF-08, AC-ARM-07/08 and air support.

**Status:** identity E-R; talent eligibility, cap 1, OXP and death rules E (talent doc); all effect values P; owner approval pending (talent doc §5).

## 4. Combat vehicles and artillery

### AC-ARM-01 — Escort Jeep

| Field | Specification |
| --- | --- |
| **Identity** | `unit.ac.arm_01` · Atlantic Concord · Vehicle / light armed patrol car · `TPL-VEH-LIGHT` · **Core** · Playable |
| **Role** | **Primary:** light armed patrol and convoy protection (E-R). **Secondary:** anti-infantry raiding; hunting agents and scouts; rapid reaction between junctions. **Not** the faction's opening scout (E-R: AC-SUP-01 is). **Doctrine fit:** cheap mobility that protects the logistics lines Concord depends on (E). |
| **Inspiration and background** | 1980s Western 4×4 utility vehicle with a pintle heavy machine gun. C&C lineage (E-R): Humvee (TD); Ranger (RA1); Humvee (Gen). |
| **Visual identity** | No concept sheet: none (asset requirement). Silhouette: low boxy 4×4 with a ring-mounted HMG and gunner; pale-blue roof band for air recognition. |
| **Statistics** | **320 HP** (`armor.veh_soft`) · WHEEL_LIGHT **510 cm/s** · F1 · sight 1400 cm · crew 2 · fuel light · ExperienceValue 56 |
| **Commands** | Move, Attack, AttackMove, Stop, Guard (I) · Patrol, Escort, Withdraw (P) · default stance Aggressive · formation role `flank` |
| **Production** | **$560** · `AC-BLD-05` Armored Vehicle Works · prerequisites: `AC-BLD-05` operational · **9 s** · 2 food · 1 petroleum · no cap |
| **Veterancy** | Standard |
| **AI** | `ai.unit.skirmisher`. Deltas: under *Escort*, stay within 600 cm of the escorted unit and auto-engage anything attacking it |
| **Multiplayer** | MPA-STD |
| **Presentation** | ANIM-VEH-WHEEL + gunner traverse. Heavy MG tracer. Icon: jeep with MG |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.ac.arm_01.primary` heavy MG | SMALL_ARMS | 0–780 cm | 14 / 0.3 s (46.7 DPS); fire while moving | hitscan, tracking fast; 360° ring mount | 4 | GROUND, AIR_LOW | 100 % / 15 % (VEH_SOFT 50 %) / 2 % / 25 % rotary / 8 % |

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.ac.arm_01.escort` | Active order (P) | Follows a friendly unit at 300–600 cm, matching its speed, and treats attackers of the escorted unit as priority targets ×1.5 | Friendly ground unit | free · until cancelled · — · — | Ambush by AT infantry | Escort line (owner only) | Assign to tankers and convoys automatically when idle |

**Research and upgrades:** `tech.shared.vehicle_component_standardization`: repair cost −20 %. `tech.ac.light_vehicle_armor_kits` (AC-BLD-05, $500, 40 s): HP 320 → 380, `armor.veh_soft` → `armor.veh_light`; trade-off: speed 510 → 460.

**Counters:** *Beats (hard):* infantry in the open without AT, agents, scouts (RD-ARM-01, MP-ARM-01), logistics trucks. *Beats (soft):* helicopters at 25 %. *Beaten by (hard):* AT infantry (RD-INF-04, IA-INF-04, MP-INF-03), any tank or IFV. *Situational:* the cheapest Concord unit that can protect fuel convoys.

**Status:** identity E-R; not the opening scout E-R; everything else P.

### AC-ARM-02 — Rapid Armored Carrier

| Field | Specification |
| --- | --- |
| **Identity** | `unit.ac.arm_02` · Atlantic Concord · Vehicle / tracked APC (amphibious) · `TPL-VEH-TRANSPORT` · **Core** · Playable |
| **Role** | **Primary:** protected infantry taxi with limited combat support (E-R). **Secondary:** casualty evacuation; river crossing with infantry. **Doctrine fit:** mechanized infantry is a principal Concord land capability (E). |
| **Inspiration and background** | Aluminium-hulled tracked armored personnel carrier with a cupola machine gun; swims calm water. C&C lineage (E-R): APC (TD); Riptide ACV (RA3); APC (TW). |
| **Visual identity** | Concept: [`AC-ARM-02.png`](../../Factions/TheAtlanticConcord/Units/AC-ARM-02.png). Silhouette: box hull, rear ramp, cupola MG. |
| **Statistics** | **600 HP** (`armor.veh_light`) · TRACKED + AMPHIBIOUS **400 cm/s** (water ×0.45) · F2 · sight 1200 cm · **2 infantry slots** · crew 2 · fuel medium · ExperienceValue 90 |
| **Commands** | Move, Attack, AttackMove, Stop, Guard (I) · Load, Unload, Withdraw (P) · default stance Aggressive · formation role `escort` |
| **Production** | **$900** · `AC-BLD-05` · prerequisites: `AC-BLD-05` · **14 s** · 2 food · 3 petroleum · no cap |
| **Veterancy** | Standard |
| **AI** | `ai.unit.transport`. Deltas: unload at the nearest cover within 400 cm of the order point when under AT fire |
| **Multiplayer** | **MPA-TRANSPORT** (manifest owner-only, UNIT-08) |
| **Presentation** | ANIM-VEH-TRACK + ramp, swim wake. Loaded indicator light when observed. Icon: APC with a squad pip |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.ac.arm_02.primary` cupola HMG | SMALL_ARMS | 0–720 cm | 13 / 0.3 s (43 DPS) | hitscan; 360° | 4 | GROUND, AIR_LOW | 100 % / 15 % / 2 % / 25 % rotary / 8 % |

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.shared.transport` | Active (P, §10.1) | Carries 2 infantry squads; load 0.5 s, unload 1.0 s per slot; no fire ports | Own infantry | free · — · — · 150 cm | AT fire kills passengers' ride; ejection with 50 % HP loss on death | Ramp animation | Unload behind cover |
| `ability.ac.arm_02.medevac` | Passive (P) | Passengers heal 3 HP/s up to their living-member cap | Passengers | free · continuous | — | Cross icon on card (owner) | Load squads under 50 % HP if a medic is not available |

**Research and upgrades:** `tech.shared.vehicle_component_standardization`. `tech.ac.add_on_armor` (AC-BLD-05, $600, 45 s): HP 600 → 700, loses amphibious capability; trade-off is the river-crossing role.

**Counters:** *Beats (soft):* infantry in the open, scouts. *Beaten by (hard):* AT infantry, tanks, TDs, attack helicopters. *Situational:* river and canal crossings without bridges.

**Status:** identity E-R; everything else P.

### AC-ARM-03 — Modular Infantry Support Vehicle

| Field | Specification |
| --- | --- |
| **Identity** | `unit.ac.arm_03` · Atlantic Concord · Vehicle / modular IFV · `TPL-VEH-IFV` · **Extended** · Playable |
| **Role** | **Primary:** weapon loadout changes with the boarded squad or equipment package (E-R). **Secondary:** fire support for mechanized infantry; anti-light-vehicle and anti-helicopter fire. **Doctrine fit:** versatile units multiplied by integration (E). |
| **Inspiration and background** | 1980s tracked infantry fighting vehicle with a 25 mm-class autocannon and a dismount team. C&C lineage (E-R): IFV (RA2); Multigunner IFV (RA3). |
| **Visual identity** | No concept sheet: none (asset requirement). Silhouette: angular IFV with a two-man turret; module pod on the turret side changes shape with the module. |
| **Statistics** | **650 HP** (`armor.veh_medium`) · TRACKED **370 cm/s** · F2 · sight 1300 cm · **1 infantry slot (the module)** · crew 3 · fuel medium · ExperienceValue 100 |
| **Commands** | as TPL-VEH-IFV · default stance Aggressive · formation role `frontline` |
| **Production** | **$1,000** (5 % over band top: module flexibility) · `AC-BLD-05` · prerequisites: `AC-BLD-16` · **15 s** · 3 food · 3 petroleum · no cap |
| **Veterancy** | Standard |
| **AI** | `ai.unit.ifv`. Deltas: choose the module from the observed threat: AT squad vs vehicles, rifle vs infantry, AT in AA mode vs helicopters |
| **Multiplayer** | MPA-TRANSPORT. Opponents see the module pod shape (visible design tell) but not the squad's health |
| **Presentation** | ANIM-VEH-TRACK + turret, pod swap. Icon changes with the module |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.ac.arm_03.primary` 25 mm autocannon | AUTOCANNON | 0–850 cm | 20 / 0.5 s (40 DPS) | shell_direct, tracking fast; turret | 4 | GROUND, AIR_LOW | 65 % / 85 % (VEH_MEDIUM 45 %) / 10 % / 100 % rotary / 30 % |
| `weapon.ac.arm_03.module` (per boarded unit) | per module | per module | see *Module Loadout* | — | — | — | — |

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.ac.arm_03.module_loadout` | Conditional passive (P) | The boarded squad adds a module weapon at 70 % of its own output: **AC-INF-01/07** coax MG (SMALL_ARMS 10 / 0.3 s); **AC-INF-03** twin missile pod (its AT or AA weapon, 2 rounds then 12 s reload); **AC-INF-04** automatic grenade launcher (HE 16 / 1.0 s, 100 cm AoE); **AC-INF-06** field repair 6 HP/s to adjacent vehicles; **AC-INF-05** heal 4 HP/s aura 300 cm; **AC-INF-08** sight +300 cm and `DETECT.CONCEALED` 900 cm. Any other unit: no module | Boarded friendly squad | free · while boarded · — · per module | Killing the IFV removes both; each module is weaker than the dismounted specialist | Module icon on the card; pod mesh | Board the specialist that answers the dominant observed threat |

**Research and upgrades:** `tech.ac.precision_vehicle_targeting`: autocannon tracking unaffected by target speed. `tech.ac.modular_mounts` ($600): module output 70 → 85 %; trade-off: the IFV loses its slot for non-module passengers.

**Counters:** *Beats (hard):* helicopters (RD-AIR-03, MP-AIR-02) and light vehicles. *Beats (soft):* infantry. *Beaten by (hard):* tanks and TDs (RD-ARM-04, IA-ARM-09, MP-ARM-07), AT infantry. *Situational:* a flexible screen; never a substitute for a dedicated AA or AT unit.

**Status:** identity and modular concept E-R; everything else P.

### AC-ARM-04 — Concord Battle Tank

| Field | Specification |
| --- | --- |
| **Identity** | `unit.ac.arm_04` · Atlantic Concord · Vehicle / main battle tank · `TPL-VEH-MBT` · **Core** · Playable |
| **Role** | **Primary:** reliable precision-oriented MBT (E-R). **Secondary:** anti-structure fire; escort for artillery. **Doctrine fit:** fewer, better tanks that hit first and are recovered after damage (E). |
| **Inspiration and background** | Late-1980s Western MBT: stabilised 105–120 mm gun, laser rangefinder, thermal sight. C&C lineage (E-R): Medium Tank (TD); Grizzly Battle Tank (RA2); Predator Tank (TW). |
| **Visual identity** | No concept sheet: none (asset requirement). Silhouette: low, wide turret with a long gun and bustle; side skirts; blue turret band. |
| **Statistics** | **950 HP** (`armor.veh_heavy`) · TRACKED **330 cm/s** · F3 · sight 1300 cm · crew 4 · fuel heavy · ExperienceValue 130 |
| **Commands** | as TPL-VEH-MBT · default stance Aggressive · formation role `frontline` |
| **Production** | **$1,300** · `AC-BLD-05` · prerequisites: `AC-BLD-05` and `AC-BLD-12` powered · **22 s** · 4 food · 5 petroleum · no cap |
| **Veterancy** | Standard |
| **AI** | `ai.unit.battle_tank`; retreat at 35 % HP when a repair source is known (Concord preservation, framework §14.2) |
| **Multiplayer** | MPA-STD |
| **Presentation** | ANIM-VEH-TRACK + turret, recoil, stabilised gun bob. Sharp gun crack; tracer for APFSDS. Icon: tank with a crosshair |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.ac.arm_04.main` stabilised tank gun | KINETIC_AP | 0–950 cm | 80 / 2.6 s (30.8 DPS); fire while moving at no penalty (stabiliser) | shell_direct 6000 cm/s, tracking medium (fast with research) | 6 (heavy hit) | GROUND, STRUCTURE | 20 % (never auto) / 90 % / 100 % / — / 40–45 % |
| `weapon.ac.arm_04.coax` coaxial MG | SMALL_ARMS | 0–700 cm | 9 / 0.3 s (30 DPS) | hitscan | 3 | GROUND | 100 % / 15 % / 2 % / — / 8 % |

**Abilities:** no unit-specific ability. Shared: none (repair is external). The first-shot advantage is expressed by stabilised fire-on-move.

**Research and upgrades:** `tech.ac.precision_vehicle_targeting` (E candidate, AC-BLD-16): tracking medium → fast (no −25 % against targets faster than 350 cm/s) and +50 cm range; trade-off: $900 and 60 s lane. `tech.shared.vehicle_component_standardization`: repair cost −20 %. `tech.ac.composite_armor_package` ($800): HP 950 → 1,050; speed 330 → 310.

**Counters:** *Beats (hard):* light vehicles, APCs and IFVs (RD-ARM-03, IA-ARM-02, MP-ARM-03), artillery. *Beats (soft):* enemy MBTs on first-shot and accuracy (RD-ARM-04 at equal numbers). *Beaten by (hard):* TDs (IA-ARM-09, MP-ARM-07), attack helicopters (RD-AIR-03, IA-AIR-03), heavy tanks (RD-ARM-05, IA-ARM-05). *Beaten by (soft):* massed AT infantry (RD-INF-04) in towns. *Situational:* expensive; trades well only with support and recovery.

**Status:** identity E-R; research name E candidate (research doc §6); everything else P.

### AC-ARM-05 — Heavy Breakthrough Tank

| Field | Specification |
| --- | --- |
| **Identity** | `unit.ac.arm_05` · Atlantic Concord · Vehicle / heavy tank · `TPL-VEH-HEAVY` · **Extended** · Playable |
| **Role** | **Primary:** rare, expensive two-weapon tracked armored spearhead (E-R). **Secondary:** fortification assault; anti-helicopter self-protection. **Doctrine fit:** concentrated effective force at a chosen point (E). |
| **Inspiration and background** | Alternate-history heavy breakthrough tank with a large-calibre main gun and a secondary autocannon mount — plausible 1980s engineering, no fantasy elements (E: roster rule 7). C&C lineage (E-R): Mammoth Tank (TD); Mammoth Tank (TW); FutureTank X-1 (Uprising, role adapted). |
| **Visual identity** | No concept sheet: none (asset requirement). Silhouette: longer hull, heavy turret with a roof autocannon; recognisably bigger than AC-ARM-04 at RTS zoom. |
| **Statistics** | **1,500 HP** (`armor.veh_heavy`) · TRACKED_HEAVY **250 cm/s** · F3 · sight 1300 cm · crew 4 · fuel heavy · ExperienceValue 230 |
| **Commands** | as TPL-VEH-HEAVY · default stance Aggressive · formation role `frontline` |
| **Production** | **$2,300** · `AC-BLD-05` · prerequisites: `AC-BLD-17` and `tech.ac.heavy_armor_program` · **34 s** · 4 food · 5 petroleum · cap 4 (soft: "rare", E-R) |
| **Veterancy** | Standard |
| **AI** | `ai.unit.heavy_armor`; never alone; requests AC-ARM-09 escort |
| **Multiplayer** | MPA-STD |
| **Presentation** | ANIM-VEH-TRACK + multi-turret. Deeper gun report. Icon: heavy tank with two barrels |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.ac.arm_05.main` heavy tank gun | KINETIC_AP | 0–1000 cm | 120 / 3.4 s (35 DPS) | shell_direct, tracking medium | 8 | GROUND, STRUCTURE | 20 % / 90 % / 100 % / — / 40–45 % |
| `weapon.ac.arm_05.secondary` roof autocannon | AUTOCANNON | 0–800 cm | 16 / 0.5 s (32 DPS); independent arc 360° | shell_direct, tracking fast | 4 | GROUND, AIR_LOW | 65 % / 85 % / 10 % / 100 % rotary / 30 % |

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.ac.arm_05.breakthrough_drive` | Active (P) | 10 s: ignores the Rattled tier's cooldown penalty and takes −15 % damage from KINETIC_AP to the front arc; speed −20 % | Self | free · 10 s · 60 s · self | Flank shots; air attack | Engine roar, dust; status icon | Use when advancing into ≥ 2 observed AT sources |

**Research and upgrades:** `tech.ac.heavy_armor_program` (AC-BLD-16, $1,500, 90 s): unlocks the unit. `tech.ac.precision_vehicle_targeting` applies to the main gun.

**Counters:** *Beats (hard):* light and medium vehicles, helicopters that close within 800 cm. *Beats (soft):* MBTs and fortifications. *Beaten by (hard):* TDs and ATGM teams at range (IA-ARM-09, MP-ARM-07, RD-INF-04 massed), strike aircraft. *Situational:* slow; needs bridging and AA escort.

**Status:** identity E-R; everything else P.

### AC-ARM-06 — Anti-Tank Missile Carrier

| Field | Specification |
| --- | --- |
| **Identity** | `unit.ac.arm_06` · Atlantic Concord · Vehicle / ATGM carrier · `TPL-VEH-TD` · **Core** · Playable |
| **Role** | **Primary:** long-range vehicle killer with limited close protection (E-R). **Secondary:** overwatch of armored approaches; anti-structure strikes. **Doctrine fit:** precision guided weapons employed from prepared positions (E). |
| **Inspiration and background** | Tracked carrier with an elevating twin ATGM launcher and thermal sight. C&C lineage (E-R): Tank Destroyer (RA2); Guardian Tank (RA3); Javelin Soldier (RA3). |
| **Visual identity** | No concept sheet: none (asset requirement). Silhouette: APC-like hull with a raised twin-tube launcher arm. |
| **Statistics** | **540 HP** (`armor.veh_medium`) · TRACKED **360 cm/s** · F2 · sight **1500 cm** · crew 3 · fuel medium · ExperienceValue 125 |
| **Commands** | as TPL-VEH-TD · default stance Hold Ground · formation role `support_weapon` |
| **Production** | **$1,250** · `AC-BLD-05` · prerequisites: `AC-BLD-16` · **18 s** · 3 food · 3 petroleum · no cap |
| **Veterancy** | Standard |
| **AI** | `ai.unit.tank_destroyer`. Deltas: fire from max range; reposition 300 cm after each volley if observed |
| **Multiplayer** | MPA-STD; ammunition is owner-only |
| **Presentation** | ANIM-VEH + launcher elevate, missile reload. Wire-guided missile trail. Ammo pip. Icon: tracked vehicle with missile |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.ac.arm_06.main` twin ATGM | SHAPED_CHARGE | 300–1300 cm | 150 / 5.0 s (30 DPS); **AmmoCapacity 10**; cannot fire while moving | guided_missile 1500 cm/s (jammable) | 6 | GROUND, STRUCTURE | 25 % (never auto) / 100 % / 90 % (135 per hit) / — / 70 % |
| `weapon.ac.arm_06.secondary` pintle MG | SMALL_ARMS | 0–600 cm | 9 / 0.3 s | hitscan | 2 | GROUND | 100 % / 15 % / 2 % / — / 8 % |

**Abilities:** no unit-specific ability. Shared: resupply from AC-SUP-06 or AC-BLD-05 (§10.4).

**Research and upgrades:** `tech.ac.precision_vehicle_targeting`: range 1300 → 1400 cm. `tech.shared.electronic_countermeasures`: inside jamming, scatter max 150 cm instead of 300.

**Counters:** *Beats (hard):* MBTs and heavy tanks at range (RD-ARM-04/05, IA-ARM-04/05, MP-ARM-06/13), APCs. *Beaten by (hard):* infantry inside 300 cm (minimum range), artillery, jamming (RD-SUP-06, IA-ARM-14), attack helicopters. *Situational:* runs dry without logistics.

**Status:** identity E-R; everything else P.

### AC-ARM-07 — Concord Self-Propelled Gun

| Field | Specification |
| --- | --- |
| **Identity** | `unit.ac.arm_07` · Atlantic Concord · Vehicle / self-propelled howitzer · `TPL-VEH-ARTY` · **Core** · Playable |
| **Role** | **Primary:** conventional mobile indirect artillery supporting combined arms (E-R). **Secondary:** counter-battery fire with AC-ARM-12; smoke missions. **Doctrine fit:** coordinated artillery with good observation (E). |
| **Inspiration and background** | 155 mm-class tracked self-propelled howitzer. C&C lineage (E-R): Artillery (RA1); Juggernaut (TW, legged chassis adapted to tracked). |
| **Visual identity** | No concept sheet: none (asset requirement). Silhouette: large rear turret, long barrel, spade at the rear. |
| **Statistics** | **480 HP** (`armor.veh_light`) · TRACKED **290 cm/s** · F2 · sight 1100 cm · crew 4 · fuel medium · ExperienceValue 150 · death blast (ammunition, §6.10) |
| **Commands** | as TPL-VEH-ARTY · default stance Hold Ground · formation role `support_weapon` |
| **Production** | **$1,500** · `AC-BLD-05` · prerequisites: `AC-BLD-16` · **22 s** · 4 food · 3 petroleum · no cap |
| **Veterancy** | Standard; scatter reductions apply |
| **AI** | `ai.unit.artillery` |
| **Multiplayer** | **MPA-INDIRECT** (scatter seed server-only, UNIT-03) |
| **Presentation** | ANIM-VEH + elevation, recoil, spade. Muzzle flash only to observers of the gun. Icon: howitzer |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.ac.arm_07.main` 155 mm howitzer | HIGH_EXPLOSIVE | **900–3600 cm** | 115 / 6.0 s; **AmmoCapacity 30**; cannot fire while moving | ballistic 1800 cm/s; full 100 cm, max 250 cm, edge 30 %; scatter 100–350 cm (×0.7 observed) | 12 | GROUND, STRUCTURE | 110 % / 45 % / 12 % / — / 100 % (STR_H 60 %) |

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.ac.arm_07.smoke_mission` | Active (P) | 3 smoke rounds create overlapping §9.5 smoke cylinders along a 900 cm line | Observed ground point in range | 3 ammo · 20 s · 45 s · 900–3600 cm | Detectors; waiting it out | Smoke VFX; line preview | Cover friendly withdrawals or river crossings |

**Research and upgrades:** `tech.ac.improved_fire_control` (AC-BLD-16, $800): scatter max 350 → 280 cm; trade-off: cooldown 6.0 → 6.5 s. `tech.shared.supply_preservation`: resupply cost −25 %.

**Counters:** *Beats (hard):* massed infantry and static defenses (RD-INF-01/02, RD-INF-03/05, structures). *Beats (soft):* artillery via counter-battery. *Beaten by (hard):* raiders and helicopters that reach it (MP-ARM-02/04, RD-AIR-03), counter-battery (RD-ARM-07/08 with RD-DEF-07), strike jets. *Situational:* needs observation (AC-INF-08, AC-SUP-01, AC-AIR-09).

**Status:** identity E-R; observation requirement E (Visibility); everything else P.

### AC-ARM-08 — Precision Missile Battery

| Field | Specification |
| --- | --- |
| **Identity** | `unit.ac.arm_08` · Atlantic Concord · Vehicle / guided rocket artillery · `TPL-VEH-ARTY` · **Extended** · Playable |
| **Role** | **Primary:** long-range guided rocket strikes with costly ammunition (E-R). **Secondary:** destroy high-value structures, emitters and artillery. **Doctrine fit:** precision employment of technologically sophisticated forces (E). |
| **Inspiration and background** | Wheeled or tracked launcher with guided long-range rockets — 1980s guidance, no futuristic munitions. C&C lineage (E-R): MLRS (TD); Tomahawk Missile Launcher (Gen); Specter (KW). |
| **Visual identity** | No concept sheet: none (asset requirement). Silhouette: heavy truck with a boxed four-tube launcher. |
| **Statistics** | **420 HP** (`armor.veh_light`) · WHEEL_HEAVY **330 cm/s** · F2 · sight 1100 cm · crew 3 · fuel medium · ExperienceValue 170 · death blast |
| **Commands** | as TPL-VEH-ARTY · default stance Hold Fire (manual missions) · formation role `support_weapon` |
| **Production** | **$1,700** · `AC-BLD-05` · prerequisites: `AC-BLD-16` and `tech.ac.guided_munitions_program` · **26 s** · 3 food · 3 petroleum · no cap |
| **Veterancy** | Standard |
| **AI** | `ai.unit.artillery`. Deltas: fire only at designated or observed high-value targets (artillery, emitters, structures, heavy armor); never at lone infantry |
| **Multiplayer** | MPA-INDIRECT |
| **Presentation** | ANIM-VEH + launcher elevation, missile ripple. Long smoke trails visible to observers. Icon: missile over a crosshair |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.ac.arm_08.main` guided rockets | HIGH_EXPLOSIVE | **1000–4200 cm** | salvo 4 × 130 over 2 s, then 10 s; **AmmoCapacity 8** (2 salvos) | guided_missile 2500 cm/s; scatter 0–60 cm (jamming: ballistic, 300 cm); full 120 cm, max 220 cm, edge 30 % | 12 | GROUND, STRUCTURE | 110 % / 45 % / 12 % / — / 100 % |

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.ac.arm_08.precision_strike` | Active (P) | A designated target (AC-INF-08, AC-INF-09, AC-INF-14 or AC-AIR-09 mark) takes +25 % from the salvo | Designated target in range | 4 ammo · instant · 20 s · range | Smoke breaks designation; jamming degrades guidance | Mark-to-impact line (owner) | Prefer designated emitters and artillery |

**Research and upgrades:** `tech.ac.guided_munitions_program` (AC-BLD-16, $1,200, 75 s): unlocks unit. Resupply is expensive: 1 ammo unit costs $55 (P, U-09 rule).

**Counters:** *Beats (hard):* artillery, radar and EW vehicles (RD-SUP-06, IA-ARM-14), structures. *Beaten by (hard):* raiders (MP-ARM-02/04), helicopters, jamming, ammunition starvation. *Situational:* decisive when observation is good; weak in prolonged attrition.

**Status:** identity and costly ammunition E-R; everything else P.

### AC-ARM-09 — Skyguard Missile Carrier

| Field | Specification |
| --- | --- |
| **Identity** | `unit.ac.arm_09` · Atlantic Concord · Vehicle / radar SAM carrier · `TPL-VEH-AA` · **Core** · Playable |
| **Role** | **Primary:** radar-directed air-defense vehicle and escort (E-R). **Secondary:** air radar picture for the group. **Doctrine fit:** protect scarce high-value assets (E). |
| **Inspiration and background** | Tracked carrier with short-range SAMs and a search radar. C&C lineage (E-R): Slingshot (KW); Avenger (ZH); Aegis Cruiser (RA2, function adapted). |
| **Visual identity** | No concept sheet: none (asset requirement). Silhouette: tracked hull, rotating radar dish, two four-round missile boxes. |
| **Statistics** | **540 HP** (`armor.veh_medium`) · TRACKED **330 cm/s** · F2 · sight 1300 cm ground / **`DETECT.AIR_RADAR` 3000 cm** · crew 3 · fuel medium · ExperienceValue 120 |
| **Commands** | as TPL-VEH-AA · default stance Aggressive (air) / Hold Fire (ground) · formation role `escort` |
| **Production** | **$1,200** · `AC-BLD-05` · prerequisites: `AC-BLD-12` powered and `AC-BLD-16` · **17 s** · 3 food · 3 petroleum · no cap |
| **Veterancy** | Standard |
| **AI** | `ai.unit.air_defense` |
| **Multiplayer** | MPA-STD + MPA-SENSOR (radar is a `DETECT.EMITTER` contact to enemies within 3000 cm) |
| **Presentation** | ANIM-VEH + radar spin. Missile launch smoke. Icon: missile up |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.ac.arm_09.main` radar-guided SAM | FLAK_AA | 0–1900 cm | 85 / 3.0 s; **AmmoCapacity 8**; reload by resupply | guided_missile 3000 cm/s; jamming degrades to scatter | — | AIR_LOW, AIR_HIGH | — / — / — / 110 % rotary, 100 % fixed / — |

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.ac.arm_09.radar_emission_control` | Active toggle (P) | Radar off: no `DETECT.AIR_RADAR`, SAM range 1900 → 1000 cm (visual), no emitter contact for enemies | Self | free · toggle · 3 s · self | Air units that are not seen visually | Dish stops; icon | Turn radar off when an enemy emitter detector or anti-radiation threat is observed |

**Research and upgrades:** `tech.shared.radar_signal_processing`: air radar 3000 → 3600 cm. `tech.shared.electronic_countermeasures`: guidance keeps scatter max 150 cm in jamming.

**Counters:** *Beats (hard):* helicopters and jets (RD-AIR-01/02/03, IA-AIR-01/03/05, MP-AIR-02/03). *Beaten by (hard):* any ground attacker (no ground weapon), artillery, jamming. *Situational:* must sit inside the protected group.

**Status:** identity and radar dependence E-R; everything else P.

### AC-ARM-10 — Electronic Warfare Van

| Field | Specification |
| --- | --- |
| **Identity** | `unit.ac.arm_10` · Atlantic Concord · Vehicle / jammer · `TPL-VEH-EW` · **Extended** · Playable |
| **Role** | **Primary:** disrupt enemy targeting, local radar and communications (E-R). **Secondary:** cut enemy commander and officer aura links; protect armor from guided missiles. **Doctrine fit:** electronic warfare improves awareness and permits coordinated attacks or withdrawals (E). |
| **Inspiration and background** | Truck-mounted communications and radar jammer with a mast. C&C lineage (E-R): Mobile Radar Jammer (RA1); Radar Van (Gen); Hydrofoil weapon-jamming mode (RA3). |
| **Visual identity** | No concept sheet: none (asset requirement). Silhouette: box-body truck with a folding mast and dish array. |
| **Statistics** | **380 HP** (`armor.veh_light`) · WHEEL_HEAVY **380 cm/s** · F2 · sight 1400 cm · crew 2 · fuel medium · ExperienceValue 95 |
| **Commands** | as TPL-VEH-EW · default stance Hold Fire · formation role `protected` |
| **Production** | **$950** · `AC-BLD-05` · prerequisites: `AC-BLD-16` and `AC-DEF-05` or `AC-DEF-06` built · **14 s** · 2 food · 3 petroleum · no cap |
| **Veterancy** | none (non-combat) |
| **AI** | `ai.unit.electronic_warfare` |
| **Multiplayer** | MPA-STD + MPA-SENSOR (field coverage details never replicated to opponents) |
| **Presentation** | ANIM-VEH + mast deploy. Field ring visible to owner/allies; enemies inside see a "Jammed" status icon on their own units. Icon: lightning over a dish |

**Weapons:** none (unarmed).

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.shared.jam_field` | Active deploy (P, §9.5) | Deployed: 1400 cm jamming field: suppresses enemy air-radar contacts; enemy guided weapons degrade to ballistic scatter; cuts enemy aura links through the field; blocks enemy `DETECT.ARTILLERY`. Never hides units from sight. Emits a `DETECT.EMITTER` contact within 3000 cm | Self | free · while deployed · 4 s deploy · 1400 cm | Emitter detectors (RD-SUP-06 counterpart, AC-INF-13-class), artillery, raiders | Field ring (owner/allies) | Deploy when enemy guided weapons, air radar or an enemy commander are present |

**Research and upgrades:** `tech.shared.electronic_countermeasures`: radius 1400 → 1600 cm; trade-off: emitter contact range 3000 → 3600 cm.

**Counters:** *Beats (soft):* ATGM-heavy and SAM-heavy compositions, enemy commanders' auras (RD-INF-13, IA-INF-13). *Beaten by (hard):* any direct attacker; artillery guided by emitter detection. *Situational:* force multiplier in combined arms, useless alone.

**Status:** identity E-R; no fog erasure E-R; everything else P.

### AC-ARM-11 — Mobile Concealment Truck

| Field | Specification |
| --- | --- |
| **Identity** | `unit.ac.arm_11` · Atlantic Concord · Vehicle / field camouflage screen · `TPL-VEH-EW` · **Extended** · Playable |
| **Role** | **Primary:** limited field camouflage and communications screening, *not invisibility* (E-R). **Secondary:** hide artillery positions and staging areas. **Doctrine fit:** preparation before commitment of valuable forces (E). |
| **Inspiration and background** | Truck carrying camouflage nets, radar-reflective screens and decoy emitters. C&C lineage (E-R): Mobile Gap Generator (RA1); Mobile Stealth Generator (TS); Disruption Tower (TW). |
| **Visual identity** | No concept sheet: none (asset requirement). Silhouette: truck with rolled net bales and short masts. |
| **Statistics** | **360 HP** (`armor.veh_light`) · WHEEL_HEAVY **380 cm/s** · F2 · sight 1200 cm · crew 2 · fuel medium · no veterancy |
| **Commands** | Deploy Screen, Move, Stop (P) · default stance Hold Fire · formation role `protected` |
| **Production** | **$900** · `AC-BLD-05` · prerequisites: `AC-BLD-16` · **13 s** · 2 food · 3 petroleum · cap 3 |
| **Veterancy** | none |
| **AI** | `ai.unit.electronic_warfare`. Deltas: deploy at the artillery park or staging area |
| **Multiplayer** | MPA-STD + MPA-SENSOR. Screened units are Concealed (MPA-COVERT rules for observation) |
| **Presentation** | ANIM-VEH + net deploy (6 s). Screen dome visible to owner/allies; enemies see the deployed truck when it is observed. Icon: net over a vehicle |

**Weapons:** none (unarmed).

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.ac.arm_11.concealment_screen` | Active deploy (P) | Deployed: friendly **stationary** ground units within 900 cm are Concealed (§9.1). Units that fire are revealed for 4 s. The truck itself is not concealed and is an emitter contact within 2000 cm | Self | free · while deployed · 6 s deploy · 900 cm | `DETECT.CONCEALED` scouts and masts; observers inside 35 % sight; artillery on the truck | Screen dome | Deploy over artillery and support parks |

**Research and upgrades:** `tech.shared.electronic_countermeasures`: radius 900 → 1100 cm; trade-off: deploy 6 → 8 s.

**Counters:** *Beats (soft):* artillery spotting and air reconnaissance (AC-SUP-01 analogues IA-SUP-01, RD-AIR-06). *Beaten by (hard):* detectors and scouts (MP-ARM-01, RD-ARM-01), any attack on the truck. *Situational:* no invisibility (E-R); moving units are never concealed.

**Status:** identity and "not invisibility" E-R; everything else P.

### AC-ARM-12 — Counter-Battery Radar Carrier

| Field | Specification |
| --- | --- |
| **Identity** | `unit.ac.arm_12` · Atlantic Concord · Vehicle / counter-battery radar · `TPL-VEH-EW` · **Extended** · Playable |
| **Role** | **Primary:** detect hostile artillery activity and cue fire missions (E-R). **Secondary:** locate mortar teams. **Doctrine fit:** answer industrial artillery with information (E: Directorate deep artillery). |
| **Inspiration and background** | Tracked or wheeled phased-array artillery-locating radar. C&C lineage (E-R): Mobile Sensor Array (TS); Pitbull (TW); Recon Drone (TT). |
| **Visual identity** | No concept sheet: none (asset requirement). Silhouette: flat radar panel raised on a carrier. |
| **Statistics** | **400 HP** (`armor.veh_light`) · TRACKED **320 cm/s** · F2 · sight 1100 cm · **`DETECT.ARTILLERY` 4500 cm** when deployed · crew 2 · fuel medium · no veterancy |
| **Commands** | Deploy, Move, Stop (P) · default stance Hold Fire · formation role `protected` |
| **Production** | **$850** · `AC-BLD-05` · prerequisites: `AC-BLD-16` · **13 s** · 2 food · 3 petroleum · no cap |
| **Veterancy** | none |
| **AI** | `ai.unit.electronic_warfare` |
| **Multiplayer** | MPA-SENSOR. Origin points are 10 s contacts (§9.2), not identities |
| **Presentation** | ANIM-VEH + panel raise (4 s). Contact markers for owner/allies. Icon: radar with a shell |

**Weapons:** none (unarmed).

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.ac.arm_12.counter_battery_cue` | Automated (P) | When deployed, each detected origin becomes a 10 s contact; friendly artillery under Hold Ground with that contact in range may fire one counter-battery salvo at it at observed scatter (×0.7) | Enemy indirect-fire origins ≤ 4500 cm | free · 10 s per contact · — · 4500 cm | Jamming blocks it (§9.5); shoot-and-scoot artillery; killing the radar | Origin marker | Always deployed behind the artillery park |

**Research and upgrades:** `tech.shared.radar_signal_processing`: range 4500 → 5200 cm.

**Counters:** *Beats (soft):* enemy artillery (RD-ARM-07/08/09, IA-ARM-10/11, MP-ARM-08/12). *Beaten by (hard):* jamming (RD-SUP-06, IA-ARM-14) and raiders. *Situational:* value scales with friendly artillery count.

**Status:** identity E-R; everything else P.

### AC-ARM-13 — Repair and Recovery Vehicle

| Field | Specification |
| --- | --- |
| **Identity** | `unit.ac.arm_13` · Atlantic Concord · Vehicle / armored recovery and repair · `TPL-VEH-LOGISTICS` (armored variant) · **Core** · Playable |
| **Role** | **Primary:** field repairs, salvage and recovery (E-R). **Secondary:** clear wrecks from routes. **Doctrine fit:** "high equipment costs encourage preservation, recovery and repair" (E). |
| **Inspiration and background** | Armored recovery vehicle on a tank chassis with crane, winch and dozer blade. C&C lineage (E-R): Mobile Repair Vehicle (TS); Mobile Repair Transport (KW); Rig (TW). |
| **Visual identity** | No concept sheet: none (asset requirement). Silhouette: turretless tank hull with an A-frame crane. |
| **Statistics** | **750 HP** (`armor.veh_medium`) · TRACKED **310 cm/s** · F3 · sight 1000 cm · crew 3 · fuel heavy · no veterancy |
| **Commands** | Move, Stop, Guard (I) · Repair, Salvage, Tow, Withdraw (P) · default stance Hold Fire · formation role `rear_guard` |
| **Production** | **$950** · `AC-BLD-05` · prerequisites: `AC-BLD-17` · **15 s** · 3 food · 5 petroleum · no cap |
| **Veterancy** | none |
| **AI** | `ai.unit.logistics`. Deltas: follow the armored group at 600 cm; repair the highest-value damaged vehicle first |
| **Multiplayer** | MPA-STD + MPA-ECON (salvage credits once; UNIT-09 style) |
| **Presentation** | ANIM-VEH + crane, welding sparks. Icon: wrench over a tank |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.ac.arm_13.secondary` self-defense MG | SMALL_ARMS | 0–600 cm | 8 / 0.3 s | hitscan | 2 | GROUND | 100 % / 15 % / 2 % / — / 8 % |

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.shared.repair_vehicle` | Active / automated (P, §10.2) | 20 HP/s to one vehicle; 25 % of unit cost per full repair, per HP | Friendly or allied vehicle ≤ 300 cm | USD per HP · continuous · — · 300 cm | Killing or suppressing the ARV | Sparks; ledger tick | Auto under Guard |
| `ability.shared.salvage_wreck` | Active (P, §10.5) | Wreck → 10 % of its unit cost in USD (owner); allied wrecks cleared for 0 | Wreck ≤ 300 cm | free · 6 s · — · 300 cm | Fire on the wreck site | Crane loop; USD toast | Salvage hostile wrecks after a fight |
| `ability.shared.battlefield_recovery` | Active (P, U-11) | Tow a wreck of **own** heavy or super-heavy armor to AC-BLD-17 within 60 s: it is re-created at 25 % HP for 40 % of its cost | Own VEH_HEAVY wreck | 40 % cost · tow at 60 % speed · — · contact | Kill the ARV while towing; distance | Tow cable; timer | Recover AC-ARM-04/05/15 wrecks when AC-BLD-17 is ≤ 3000 cm away |

**Research and upgrades:** `tech.shared.vehicle_component_standardization`: repair USD −20 %. `tech.ac.recovery_doctrine` ($600): tow speed 60 → 75 %.

**Counters:** *Beats:* attrition. *Beaten by (hard):* raiders, artillery, aircraft. *Situational:* the Concord's main answer to its own replacement cost.

**Status:** identity E-R; recovery rule U-11; everything else P.

### AC-ARM-14 — Assault Engineer Breacher

| Field | Specification |
| --- | --- |
| **Identity** | `unit.ac.arm_14` · Atlantic Concord · Vehicle / armored engineer · `TPL-VEH-ENGINEER` · **Extended** · Playable |
| **Role** | **Primary:** protected, vehicle-borne engineering for bridge or strongpoint assault (E-R). **Secondary:** minefield lanes; short gap bridging. **Doctrine fit:** fortified targets may demand sustained conventional combat (E). |
| **Inspiration and background** | Tank-chassis combat engineer vehicle with dozer blade, demolition gun and mine-clearing line charge, plus an alternate scissor-bridge layer pack. C&C lineage (E-R): Combat Engineer (KW); Dozer (TT); Riptide ACV (RA3). |
| **Visual identity** | No concept sheet: none (asset requirement). Silhouette: tank hull with a dozer blade and a stubby wide gun. |
| **Statistics** | **950 HP** (`armor.veh_heavy`) · TRACKED **280 cm/s** · F3 · sight 1100 cm · detects mines 500 cm · crew 4 · fuel heavy · ExperienceValue 140 |
| **Commands** | Move, Attack, Stop, Guard (I) · Clear, Breach, Bridge (P) · default stance Hold Fire · formation role `frontline` |
| **Production** | **$1,400** · `AC-BLD-05` · prerequisites: `AC-BLD-17` · **20 s** · 4 food · 5 petroleum · cap 3 |
| **Veterancy** | Standard |
| **AI** | `ai.unit.engineer_vehicle` |
| **Multiplayer** | MPA-STD. Bridge spans are map-state changes subject to Unaligned jurisdiction (E) |
| **Presentation** | ANIM-VEH + blade, bridge scissor arm. Icon: dozer blade over a bridge |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.ac.arm_14.main` demolition gun | DEMOLITION | 0–450 cm | 180 / 8.0 s; **structures only** (target layer STRUCTURE; not attached-charge sabotage) | shell_direct; full 80 cm, max 150 cm, edge 50 % | 10 | STRUCTURE | — / — / — / — / 150 % STR_S, 120 % STR_H |

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.ac.arm_14.clear_lane` | Active (P) | Line charge clears all mines in a 150 × 1200 cm lane ahead | Ground direction | 1 charge (AmmoCapacity 2) · 3 s · 30 s · 1200 cm | Covering fire; mine relaying | Rocket-and-line VFX; lane marker | Clear observed minefields on the order route |
| `ability.ac.arm_14.lay_bridge` | Active (P) | Deploys a scissor bridge over a gap ≤ 1200 cm usable by all ground units; retrieving it takes 8 s | Authored gap volume | free · 10 s deploy · — · contact | Destroying the bridge (600 HP STR_SOFT); Unaligned jurisdiction | Bridge mesh | Bridge only on explicit order |

**Research and upgrades:** `tech.ac.field_fortification` does not apply. `tech.shared.vehicle_component_standardization`.

**Counters:** *Beats (hard):* bunkers, walls and fortified positions (RD-DEF/IA-DEF/MP-DEF structures). *Beaten by (hard):* TDs and AT infantry, artillery. *Situational:* opens fortified lines and minefields (MP-ARM-11, UN-ARM-11).

**Status:** identity E-R; jurisdiction E; demolition gun is a weapon, not attached-charge sabotage (E boundary); everything else P.

### AC-ARM-15 — Prototype Siege Tank

| Field | Specification |
| --- | --- |
| **Identity** | `unit.ac.arm_15` · Atlantic Concord · Vehicle / super-heavy prototype · `TPL-VEH-HEAVY` · **Conditional** · Playable · **cap 1** |
| **Role** | **Primary:** exceptional single-vehicle heavy armor with limited slots and maintenance (E-R). **Secondary:** siege of fortified base cores. **Doctrine fit:** limited prototype platforms that are expensive and depend on specialist operators (E: Science and Technology). |
| **Inspiration and background** | Alternate-history 1980s super-heavy tracked siege vehicle: a long-barrel heavy gun and a guided-missile pod, credible engineering only. C&C lineage (E-R): MARV (KW); Mastodon (TT); FutureTank X-1 (Uprising). |
| **Visual identity** | No concept sheet: none (asset requirement). Silhouette: very long twin-track hull, central heavy turret, missile box; unique paint stencils. |
| **Statistics** | **2,600 HP** (`armor.veh_heavy`) · TRACKED_HEAVY **175 cm/s** · F4 · sight 1400 cm · crew 6 · fuel super · ExperienceValue 420 |
| **Commands** | as TPL-VEH-HEAVY · default stance Aggressive · formation role `frontline` |
| **Production** | **$4,200** · `AC-BLD-05` · prerequisites: `AC-BLD-17`, `AC-BLD-16` and `tech.ac.prototype_armor_program` · **55 s** · 6 food · 10 petroleum · **cap 1** |
| **Veterancy** | Standard |
| **AI** | `ai.unit.heavy_armor`; never advances without AA escort and an ARV |
| **Multiplayer** | MPA-STD. Maintenance state owner-only |
| **Presentation** | ANIM-VEH-TRACK + multi-turret, ground shake (cosmetic). Icon: prototype stencil |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.ac.arm_15.main` heavy siege gun | KINETIC_AP | 0–1100 cm | 190 / 4.0 s (47.5 DPS) | shell_direct, tracking slow | 10 | GROUND, STRUCTURE | 20 % / 90 % / 100 % / — / 40–45 % |
| `weapon.ac.arm_15.missile` guided missile pod | SHAPED_CHARGE | 300–1300 cm | 2 × 110 / 10 s; AmmoCapacity 12 | guided_missile (jammable) | 6 | GROUND, STRUCTURE | 25 % / 100 % / 90 % / — / 70 % |

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.ac.arm_15.maintenance_due` | Conditional passive (P) | After 120 s of cumulative firing, speed −20 % and cooldowns +20 % until serviced for 20 s at AC-BLD-17 or by AC-ARM-13 | Self | free · until serviced · — · — | Forces withdrawal cycles; kill the ARV | Wrench warning icon | Withdraw for service when no target is in range |

**Research and upgrades:** `tech.ac.prototype_armor_program` (AC-BLD-16, $2,500, 150 s): unlocks. No further upgrade chain (one prototype is enough).

**Counters:** *Beats (hard):* everything on the ground in front of it at comparable cost if escorted. *Beaten by (hard):* strike aircraft and attack helicopters (RD-AIR-02/03, IA-AIR-02/03), massed TDs and ATGMs; *Situational:* maintenance cycles and bridges limit it; losing it is a large economic swing.

**Status:** identity and limited slots E-R; cap 1 framework P; everything else P.

## 5. Reconnaissance, economy and construction vehicles

### AC-SUP-01 — Swiftwing Recon Plane

| Field | Specification |
| --- | --- |
| **Identity** | `unit.ac.sup_01` · Atlantic Concord · **Aircraft** / light reconnaissance plane (ID prefix SUP; category is authoritative, framework §1.3) · `TPL-AIR-SCOUT` · **Core** · Playable |
| **Role** | **Primary:** canonical cheap opening scout: small, unarmed fixed-wing aircraft that cannot fight (E). **Secondary:** observation for artillery; confirming reports by real sight. **Doctrine fit:** identify opposing formations early (E). |
| **Inspiration and background** | Single-engine high-wing light observation aircraft. C&C lineage (E-R): Spy Plane (RA1, support-role adapted); Surveillance Sweep (RA3); Orca (TW, observation role). |
| **Visual identity** | No concept sheet: none (asset requirement). Silhouette: small high-wing monoplane with blue wingtips. |
| **Statistics** | **180 HP** (`armor.air_fixed`) · FIXED_WING (loiter) **1600 cm/s**, layer **AIR_LOW** · FA · sight **2600 cm** (ground footprint) · `DETECT.CONCEALED` at 60 % of sight is **not** granted (aircraft; P) · crew 1 + 1 ground crew = food 2 · kerosene 1 per sortie · no veterancy |
| **Commands** | Move, Stop (I) · Patrol / Orbit, Return to Base (P) · stance Hold Fire (unarmed) · formation role `scout_screen` |
| **Production** | **$220** · `AC-BLD-06` Military Airbase · prerequisites: `AC-BLD-06` powered · **6 s** · 2 food · 1 kerosene per sortie · no cap |
| **Veterancy** | none (unarmed) |
| **AI** | `ai.unit.air_scout` |
| **Multiplayer** | MPA-AIR. Validator: no weapon slot (E). Overflight never grants espionage or sabotage permission (E) |
| **Presentation** | ANIM-AIR-FIXED. Light engine drone. Icon: small plane with an eye |

**Weapons:** none (unarmed; E).

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.ac.sup_01.loiter_sortie` | Automated (P, U-04) | Loiters 90 s per sortie, then returns to base to refuel (kerosene 1); relaunch after 15 s | Self | 1 kerosene · 90 s · 15 s turnaround · — | AA guns and MANPADS (RD-ARM-06, AC-INF-03-class enemies, IA-ARM-08) | Fuel timer on the card | Orbit outside known AA footprints |

**Research and upgrades:** `tech.ac.aviation_servicing` (AC-BLD-06, $600): loiter 90 → 120 s. Research never arms it (E: research doc §5.1).

**Counters:** *Beats:* fog. *Beaten by (hard):* any AA (RD-ARM-06/13, IA-ARM-08, MP-ARM-09, UN-ARM-08), fighters. *Situational:* the Concord opening; trades worse than Meridian scout trucks over a long early game because it must return to refuel (E: Meridian best early recon).

**Status:** unarmed, cheap, opening scout and no espionage E; everything else P.

### AC-SUP-02 — Mobile Works Unit

| Field | Specification |
| --- | --- |
| **Identity** | `unit.ac.sup_02` · Atlantic Concord · Vehicle / mobile construction vehicle · `TPL-VEH-CONSTRUCTOR` · **Core** · Playable |
| **Role** | **Primary:** deployable command and industrial construction vehicle (E-R). **Secondary:** establishes a forward build radius. **Doctrine fit:** distributed infrastructure that sustains separate fronts (E: Economic Mechanics). |
| **Inspiration and background** | Heavy wheeled engineering and site-establishment convoy truck. C&C lineage (E-R): MCV (TD); Prospector (RA3); Surveyor (TW). |
| **Visual identity** | No concept sheet: none (asset requirement). Silhouette: 8×8 truck with folded cabins and a crane. |
| **Statistics** | **1,100 HP** (`armor.veh_medium`) · WHEEL_HEAVY **260 cm/s** · F3 · sight 1000 cm · crew 4 · fuel heavy · no veterancy |
| **Commands** | Move, Stop (I) · Deploy (P) · default stance Hold Fire · formation role `protected` |
| **Production** | **$2,600** · `AC-BLD-03` Contractor Works Depot · prerequisites: `AC-BLD-01` · **40 s** · 4 food · 5 petroleum · cap 2 |
| **Veterancy** | none |
| **AI** | `ai.unit.constructor` |
| **Multiplayer** | MPA-ECON. Deploy is an atomic server transition (unit removed, structure spawned) |
| **Presentation** | ANIM-VEH + unfold deploy (8 s). Icon: crane over a building |

**Weapons:** none (unarmed).

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.ac.sup_02.deploy_forward_post` | Active (P) | Deploys into `AC-BLD-02` Portable Forward Command Post (build-radius and logistics extension). Undeploying is not supported. **Deploying into a second `AC-BLD-01` HQ is not proposed** while the fixed-base profile is the prototype default (E, U-AC-03) | Valid, permitted build site | free · 8 s · — · site | Kill during deploy (vulnerable 8 s) | Footprint preview | Deploy at secured junctions near resources |

**Research and upgrades:** `tech.shared.vehicle_component_standardization`. None other.

**Counters:** *Beaten by (hard):* anything armed. *Situational:* a high-value target; escort with AC-ARM-01/04.

**Status:** identity E-R; fixed-base default E (Resource Economy, Stage 4B); deploy target P; **U-AC-03**.

### AC-SUP-03 — Contract Construction Crew

| Field | Specification |
| --- | --- |
| **Identity** | `unit.ac.sup_03` · Atlantic Concord · Vehicle / construction crew truck · `TPL-VEH-CONSTRUCTOR` · **Extended** · Playable |
| **Role** | **Primary:** establish forward sites, roads and production buildings (E-R). **Secondary:** repair friendly structures. **Doctrine fit:** advanced multinational contractors (E-R faction identity). |
| **Inspiration and background** | Civilian construction contractor crew with a flatbed and a small excavator. Native Concord contractor, **not** a Meridian contract unit (no wage, no expiry). C&C lineage (E-R): Construction Dozer (Gen); Engineer (TD); Prospector Command Hub (RA3). |
| **Visual identity** | No concept sheet: none (asset requirement). Silhouette: flatbed truck with a compact digger, contractor hi-vis crew. |
| **Statistics** | **420 HP** (`armor.veh_soft`) · WHEEL_HEAVY **360 cm/s** · F1 · sight 1000 cm · crew 3 · fuel light · no veterancy |
| **Commands** | Move, Stop (I) · Build, Repair Structure (P) · default stance Hold Fire · formation role `protected` |
| **Production** | **$500** · `AC-BLD-03` · prerequisites: `AC-BLD-01` · **10 s** · 3 food · 1 petroleum · no cap |
| **Veterancy** | none |
| **AI** | `ai.unit.constructor` |
| **Multiplayer** | MPA-ECON. Building placement admission is server-side, never client-predicted |
| **Presentation** | ANIM-VEH + dig/build loop. Icon: hard hat |

**Weapons:** none (unarmed).

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.shared.build_structure` | Active (P; owned by Production) | Constructs an admitted structure inside a friendly build radius; the structure's price is reserved by the production contract | Valid site | structure price · structure build time · — · site | Killing the crew pauses construction | Scaffolding | Build per the AI build plan |
| `ability.shared.repair_structure` | Active / automated (P) | 15 HP/s to one friendly structure at 25 % of structure cost per full repair | Friendly structure ≤ 200 cm | USD per HP · continuous | Suppression; raiders | Sparks | Auto under Guard |

**Research and upgrades:** `tech.ac.defended_logistics` (research doc §5.1 candidate; AC-BLD-16, $600): construction crews and logistics trucks gain +15 % HP; trade-off: −5 % speed.

**Counters:** *Beaten by (hard):* any raider (MP-ARM-01/02, RD-ARM-01). *Situational:* faster expansion than the Mobile Works Unit, but only inside an existing radius.

**Status:** identity E-R; production contract E; everything else P.

### AC-SUP-04 — Petroleum Tanker

| Field | Specification |
| --- | --- |
| **Identity** | `unit.ac.sup_04` · Atlantic Concord · Vehicle / fuel tanker · `TPL-VEH-LOGISTICS` · **Extended** · Playable |
| **Role** | **Primary:** move crude oil or refined petroleum under convoy risk (E-R). **Secondary:** supply forward depots. **Doctrine fit:** fuel and transport corridors are strategic multipliers (E). |
| **Inspiration and background** | 1980s civilian-pattern tanker truck. C&C lineage (E-R): Supply Truck (Gen); Ore Truck (RA1); Harvester (TW). |
| **Visual identity** | No concept sheet: none (asset requirement). Silhouette: cab with cylindrical tank, hazard placards. |
| **Statistics** | **450 HP** (`armor.veh_soft`) · WHEEL_HEAVY **360 cm/s** · F1 · sight 1000 cm · cargo 100 petroleum *or* crude units (cargo type visible to observers, framework §15) · crew 1 · fuel light · **death blast** (HE 45, 250 cm) · no veterancy |
| **Commands** | Move, Stop, Guard (I) · Deliver, Load (P) · default stance Hold Fire · formation role `protected` |
| **Production** | **$450** · `AC-BLD-03` · prerequisites: `AC-BLD-09` or `AC-BLD-11` · **9 s** · 1 food · 1 petroleum · no cap |
| **Veterancy** | none |
| **AI** | `ai.unit.logistics` |
| **Multiplayer** | MPA-ECON. Cargo is credited once at delivery; a destroyed tanker delivers nothing |
| **Presentation** | ANIM-VEH-WHEEL. Fireball death VFX. Icon: fuel drop |

**Weapons:** none (unarmed).

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.shared.deliver_cargo` | Active / automated (P, MPA-ECON) | Loads at a source (refinery, terminal, depot) and unloads at a destination; 6 s each way | Owned or authorized source and destination | free · 6 s · — · contact | Interdiction; Unaligned jurisdiction on public roads (E) | Cargo bar | Assigned routes; reroute around observed threats |

**Research and upgrades:** `tech.shared.supply_preservation`: cargo 100 → 120. `tech.ac.defended_logistics`: +15 % HP.

**Counters:** *Beaten by (hard):* raiders and aircraft (MP-ARM-02/04, RD-AIR-03). *Situational:* Concord refining is cheaper (E), so tankers carry more value per trip than the cost of the truck.

**Status:** identity E-R; economy rules E; everything else P.

### AC-SUP-05 — Airport Fuel Bowser

| Field | Specification |
| --- | --- |
| **Identity** | `unit.ac.sup_05` · Atlantic Concord · Vehicle / kerosene bowser · `TPL-VEH-LOGISTICS` · **Extended** · Playable |
| **Role** | **Primary:** carry separately refined kerosene to airfields (E-R). **Secondary:** enable forward helicopter operations at AC-BLD-07 pads. **Doctrine fit:** air power requires fuel and maintenance (E). |
| **Inspiration and background** | Airfield refuelling bowser. C&C lineage (E-R): Chinook supply role (Gen); Supply Truck (Gen); Supply Drop Zone (Gen). |
| **Visual identity** | No concept sheet: none (asset requirement). Silhouette: short tanker with hose reels; aviation-yellow stripe. |
| **Statistics** | **380 HP** (`armor.veh_soft`) · WHEEL_HEAVY 370 cm/s · F1 · sight 1000 cm · cargo 60 kerosene (never interchangeable with petroleum, E) · crew 1 · fuel light · death blast · no veterancy |
| **Commands** | as AC-SUP-04 · formation role `protected` |
| **Production** | **$400** · `AC-BLD-03` · prerequisites: `AC-BLD-09` · **8 s** · 1 food · 1 petroleum · no cap |
| **Veterancy** | none |
| **AI** | `ai.unit.logistics` |
| **Multiplayer** | MPA-ECON |
| **Presentation** | ANIM-VEH-WHEEL + hose reel. Icon: fuel drop with wings |

**Weapons:** none (unarmed).

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.shared.deliver_cargo` | Active / automated (P) | Kerosene delivery to AC-BLD-06, AC-BLD-07 or AC-BLD-08 stocks | Airfield or pad | free · 6 s | Interdiction | Cargo bar | Keep each airfield above 20 kerosene |
| `ability.ac.sup_05.forward_arming_point` | Active (P) | Parked next to an AC-BLD-07 pad or a cleared landing zone for 10 s: helicopters landing within 400 cm can refuel from the bowser's cargo | Self | cargo · while parked · — · 400 cm | Artillery and raiders on the FARP | Pad marker | Deploy behind helicopter operations |

**Research and upgrades:** `tech.shared.supply_preservation`: cargo 60 → 75.

**Counters:** *Beaten by (hard):* raiders and aircraft. *Situational:* without it, kerosene starvation grounds fixed-wing sorties (§4.5).

**Status:** identity E-R; kerosene separation E; everything else P.

### AC-SUP-06 — Forward Logistics Convoy

| Field | Specification |
| --- | --- |
| **Identity** | `unit.ac.sup_06` · Atlantic Concord · Vehicle / ammunition and spares carrier · `TPL-VEH-LOGISTICS` · **Extended** · Playable |
| **Role** | **Primary:** deliver ammunition, spares and supplies (E-R). **Secondary:** sustain limited-ammunition weapons (ATGMs, rockets, SAMs) at the front. **Doctrine fit:** ammunition, maintenance and supply sustain Concord forces when air support is missing (E). |
| **Inspiration and background** | Armored cargo trucks with palletised ammunition. C&C lineage (E-R): Chinook (Gen); Supply Truck (Gen); Supply Drop (TT). |
| **Visual identity** | No concept sheet: none (asset requirement). Silhouette: two-vehicle convoy rendered as one unit: lead truck and trailer with pallets. |
| **Statistics** | **560 HP** (`armor.veh_light`) · WHEEL_HEAVY 350 cm/s · F2 · sight 1000 cm · stock 40 ammunition units · crew 2 · fuel medium · death blast · no veterancy |
| **Commands** | Move, Stop, Guard (I) · Resupply, Deliver (P) · default stance Hold Fire · formation role `rear_guard` |
| **Production** | **$700** · `AC-BLD-03` · prerequisites: `AC-BLD-11` · **12 s** · 2 food · 3 petroleum · no cap |
| **Veterancy** | none |
| **AI** | `ai.unit.logistics` |
| **Multiplayer** | MPA-ECON. Each ammunition unit delivered debits USD once (UNIT-09) |
| **Presentation** | ANIM-VEH-WHEEL + crate unload. Icon: ammunition crate |

**Weapons:** none (unarmed).

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.shared.resupply_aura` | Automated (P, §10.4) | 1 ammunition unit per 2 s per target, up to 4 targets within 400 cm; refills its own stock at AC-BLD-11 or AC-BLD-05 | Friendly limited-ammo units | USD per unit (U-09) · continuous · — · 400 cm | Killing the convoy; death blast hurts nearby friendlies | Crate icon on resupplied units | Follow artillery and ATGM groups at 600 cm |

**Research and upgrades:** `tech.shared.supply_preservation`: stock 40 → 50; `tech.ac.defended_logistics`: +15 % HP.

**Counters:** *Beaten by (hard):* raiders, artillery and aircraft. *Situational:* required for AC-ARM-06/08/09 in long fights.

**Status:** identity E-R; everything else P.

### AC-SUP-07 — Portable Listening Post

| Field | Specification |
| --- | --- |
| **Identity** | `unit.ac.sup_07` · Atlantic Concord · Vehicle / deployable sensor node · `TPL-VEH-EW` · **Extended** · Playable |
| **Role** | **Primary:** deploy a temporary local detection and reconnaissance data node (E-R). **Secondary:** watch flanks and river crossings. **Doctrine fit:** a well-positioned reconnaissance system improves several nearby forces (E: Gameplay). |
| **Inspiration and background** | Light 4×4 carrying ground-surveillance radar and unattended sensors. C&C lineage (E-R): Surveyor/Outpost (TW); Mobile Sensor Array (TS); Rig (TW). |
| **Visual identity** | No concept sheet: none (asset requirement). Silhouette: jeep with a tripod radar head. |
| **Statistics** | **260 HP** (`armor.veh_soft`) · WHEEL_LIGHT 480 cm/s · F1 · sight 1300 cm; deployed sight 1800 cm, `DETECT.CONCEALED` 1000 cm, ground-movement contacts 2600 cm · crew 2 · fuel light · no veterancy |
| **Commands** | Move, Stop (I) · Deploy (P) · default stance Hold Fire · formation role `scout_screen` |
| **Production** | **$500** · `AC-BLD-05` · prerequisites: `AC-BLD-16` · **9 s** · 2 food · 1 petroleum · no cap |
| **Veterancy** | none |
| **AI** | `ai.unit.electronic_warfare`. Deltas: deploy at authored chokepoints |
| **Multiplayer** | MPA-SENSOR. Movement contacts are position bands, never identities |
| **Presentation** | ANIM-VEH + tripod deploy (4 s). Icon: ear |

**Weapons:** none (unarmed).

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.ac.sup_07.sensor_node` | Active deploy (P) | Deployed: sight 1800, `DETECT.CONCEALED` 1000, and moving ground units within 2600 cm appear as contacts (not identities) | Self | free · while deployed · 4 s · 2600 cm | Units moving under 50 % speed are not reported; jamming suppresses contacts; it is an emitter within 2000 cm | Contact pings | Deploy on flanks |

**Research and upgrades:** `tech.shared.radar_signal_processing`: contact range 2600 → 3100 cm.

**Counters:** *Beats:* concealment play (MP-INF-10, IA-INF-08). *Beaten by (hard):* any attacker, jammers. *Situational:* cheap early-warning.

**Status:** identity E-R; everything else P.

## 6. Aircraft and air-support platforms

Shared aircraft rules (P, U-04): fixed-wing **sortie** aircraft launch from an owned airfield (`AC-BLD-06`), fly an ordered mission, and return to rearm automatically when munitions are spent or at 40 % HP. Rearm and refuel take 12 s per pad slot plus the listed kerosene. Helicopters rearm at `AC-BLD-07` pads. Aircraft crews count 1 food each plus 1 ground-crew equivalent (§4.5).

### AC-AIR-01 — Interceptor Fighter

| Field | Specification |
| --- | --- |
| **Identity** | `unit.ac.air_01` · Atlantic Concord · Aircraft / interceptor · `TPL-AIR-FIGHTER` · **Core** · Playable |
| **Role** | **Primary:** air superiority and hostile bomber interception (E-R). **Secondary:** escort for strike aircraft. **Doctrine fit:** contest airspace (E: Air and Maritime Power). |
| **Inspiration and background** | Late-1980s Western single-seat air-superiority fighter with medium and short-range air-to-air missiles and a gun. C&C lineage (E-R): Apollo Fighter (RA3); Firehawk (TW); Raptor (Gen). |
| **Visual identity** | No concept sheet: none (asset requirement). Silhouette: twin-tail swept-wing fighter, low-visibility grey, blue fin flash. |
| **Statistics** | **380 HP** (`armor.air_fixed`) · FIXED_WING **2500 cm/s**, AIR_HIGH · FA · sight 2200 cm (air) · crew 1 + 1 = food 2 · kerosene 2 per sortie · ExperienceValue 190 |
| **Commands** | Attack, Move, Stop (I) · Patrol, Return to Base (P) · default stance Aggressive (air) · formation role `escort` |
| **Production** | **$1,900** · `AC-BLD-06` · prerequisites: `AC-BLD-06` and `AC-BLD-12` powered · **24 s** · 2 food · 2 kerosene/sortie · no cap |
| **Veterancy** | Standard |
| **AI** | `ai.unit.air_superiority` |
| **Multiplayer** | MPA-AIR |
| **Presentation** | ANIM-AIR-FIXED + afterburner, flares. Icon: fighter |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.ac.air_01.aam` air-to-air missiles | FLAK_AA | 0–1400 cm | 120 / 2.0 s; AmmoCapacity 4 per sortie | guided_missile 4000 cm/s | — | AIR_LOW, AIR_HIGH | — / — / — / 110 % rotary, 100 % fixed / — |
| `weapon.ac.air_01.gun` cannon (air only) | AUTOCANNON | 0–600 cm | 25 / 0.5 s; AmmoCapacity 20 bursts | shell_direct | — | AIR_LOW, AIR_HIGH | — / — / — / 100 % rotary, 60 % fixed / — |

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.shared.combat_air_patrol` | Active (P) | Orbits a point for 60 s and auto-intercepts air contacts that become observed within 2500 cm | Ground point | kerosene as sortie · 60 s · — · 2500 cm | SAM belts under the patrol point | Orbit ring | Patrol over the main base when enemy bombers are known |

**Research and upgrades:** `tech.ac.aviation_servicing`: rearm 12 → 9 s. `tech.shared.radar_signal_processing`: air sight 2200 → 2600 cm.

**Counters:** *Beats (hard):* bombers, transports and helicopters (RD-AIR-07, IA-AIR-05, RD-AIR-04/05, MP-AIR-01/04). *Beats (soft):* enemy fighters on radar support. *Beaten by (hard):* SAM sites and mobile SAMs (RD-ARM-13, RD-DEF AA). *Situational:* cannot attack ground.

**Status:** identity E-R; air power E; everything else P.

### AC-AIR-02 — Ground-Attack Jet

| Field | Specification |
| --- | --- |
| **Identity** | `unit.ac.air_02` · Atlantic Concord · Aircraft / ground-attack jet · `TPL-AIR-STRIKE` · **Extended** · Playable |
| **Role** | **Primary:** conventional strike aircraft for armor and fortified targets (E-R). **Secondary:** close air support on designated targets. **Doctrine fit:** aviation supports land operations (E). |
| **Inspiration and background** | Subsonic, armored ground-attack jet with unguided rockets and retarded bombs. C&C lineage (E-R): Harrier (RA2); Vindicator (RA3); ORCA Assault Craft (Ren, mission analog). |
| **Visual identity** | No concept sheet: none (asset requirement). Silhouette: straight-wing jet with many pylons; dark green-grey. |
| **Statistics** | **600 HP** (`armor.air_fixed`) · FIXED_WING **1700 cm/s**, AIR_HIGH · FA · sight 1800 cm · food 2 · kerosene 3 per sortie · ExperienceValue 200 |
| **Commands** | Attack (I) · Strike Area, Return to Base (P) · default stance Hold Fire until ordered · formation role `reserve` |
| **Production** | **$2,000** · `AC-BLD-06` · prerequisites: `AC-BLD-16` · **26 s** · 2 food · 3 kerosene/sortie · no cap |
| **Veterancy** | Standard |
| **AI** | `ai.unit.air_strike` |
| **Multiplayer** | MPA-AIR |
| **Presentation** | ANIM-AIR-FIXED + pylons, rocket ripple. Icon: jet with rockets |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.ac.air_02.rockets` unguided rocket pods | SHAPED_CHARGE | 0–900 cm (attack run) | 8 × 70 per pass; AmmoCapacity 16 (2 passes) | ballistic; scatter 60–150 cm; full 60 cm, max 120 cm, edge 40 % | 6 | GROUND, STRUCTURE | 25 % / 100 % / 90 % / — / 70 % |
| `weapon.ac.air_02.bombs` retarded bombs | HIGH_EXPLOSIVE | overflight | 2 × 220 per pass; AmmoCapacity 4 | bomb_drop; full 150 cm, max 300 cm, edge 30 % | 12 | GROUND, STRUCTURE | 110 % / 45 % / 12 % / — / 100 % |

**Abilities:** no unit-specific ability; uses designation bonuses from AC-INF-08/09/14 and AC-AIR-09.

**Research and upgrades:** `tech.ac.aviation_servicing`. `tech.ac.armored_airframe` ($700): HP 600 → 680; kerosene +1 per sortie.

**Counters:** *Beats (hard):* armor columns and artillery without AA (RD-ARM-04/05/07, IA-ARM-04/05). *Beaten by (hard):* SAMs (RD-ARM-13, AC-DEF-03 analogues), AA guns (RD-ARM-06, IA-ARM-08, MP-ARM-09), fighters. *Situational:* after AA is suppressed.

**Status:** identity E-R; everything else P.

### AC-AIR-03 — Precision Strike Fighter

| Field | Specification |
| --- | --- |
| **Identity** | `unit.ac.air_03` · Atlantic Concord · Aircraft / guided-weapons fighter-bomber · `TPL-AIR-STRIKE` · **Extended** · Playable |
| **Role** | **Primary:** high-tier guided-weapons fighter-bomber (E-R). **Secondary:** self-escort with short-range AAMs. **Doctrine fit:** precision employment and guided munitions (E). |
| **Inspiration and background** | 1980s multirole fighter with laser-guided bombs — no stealth technology claim. C&C lineage (E-R): Stealth Fighter (Gen); Firehawk (TW); Black Eagle (RA2). |
| **Visual identity** | No concept sheet: none (asset requirement). Silhouette: twin-engine multirole jet with a targeting pod. |
| **Statistics** | **420 HP** (`armor.air_fixed`) · FIXED_WING **2300 cm/s**, AIR_HIGH · FA · sight 2000 cm · food 2 · kerosene 3 per sortie · ExperienceValue 260 |
| **Commands** | as AC-AIR-02 · formation role `reserve` |
| **Production** | **$2,600** · `AC-BLD-06` · prerequisites: `tech.ac.guided_munitions_program` · **30 s** · 2 food · 3 kerosene/sortie · no cap |
| **Veterancy** | Standard |
| **AI** | `ai.unit.air_strike`; prefers designated structures and heavy armor |
| **Multiplayer** | MPA-AIR |
| **Presentation** | ANIM-AIR-FIXED + bay/pylons, laser-spot cue (owner/allies). Icon: jet with a guided bomb |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.ac.air_03.lgb` guided bombs | HIGH_EXPLOSIVE | release from 1200 cm | 2 × 380 per sortie; AmmoCapacity 2 | guided bomb (jammable); scatter 0–40 cm designated, 200 cm undesignated; full 120 cm, max 250 cm, edge 30 % | 12 | GROUND, STRUCTURE | 110 % / 45 % / 12 % (46 vs heavy) / — / 100 % STR_S, 60 % STR_H |
| `weapon.ac.air_03.aam` short-range AAM | FLAK_AA | 0–1000 cm | 100 / 3 s; AmmoCapacity 2 | guided_missile | — | AIR_LOW, AIR_HIGH | — / — / — / 110 / 100 % / — |

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.ac.air_03.stand_off_release` | Passive (P) | Releases guided bombs at 1200 cm from the target and turns away; does not overfly the target | Designated or observed target | — | SAM range > 1200 cm | Release arc preview | Strike designated targets only |

**Research and upgrades:** `tech.ac.guided_munitions_program` unlocks. `tech.shared.electronic_countermeasures`: jamming scatter 300 → 150 cm.

**Counters:** *Beats (hard):* structures, emitters and heavy armor when designated. *Beaten by (hard):* long-range SAMs (RD-ARM-13, RD-DEF air defense), fighters. *Situational:* depends on designators.

**Status:** identity E-R; no stealth (E roster rule 7); everything else P.

### AC-AIR-04 — Attack Helicopter

| Field | Specification |
| --- | --- |
| **Identity** | `unit.ac.air_04` · Atlantic Concord · Aircraft / attack helicopter · `TPL-AIR-HELI-ATTACK` · **Core** · Playable |
| **Role** | **Primary:** low-altitude anti-armor close air support (E-R). **Secondary:** hunt light vehicles and artillery. **Doctrine fit:** air-mobile response, the faction-signature unit of the Core (E). |
| **Inspiration and background** | Tandem-seat attack helicopter with ATGMs and a chin cannon. C&C lineage (E-R): ORCA (TW); Longbow (RA1); Comanche (Gen). |
| **Visual identity** | No concept sheet: none (asset requirement). Silhouette: narrow tandem fuselage, stub wings with missile racks. |
| **Statistics** | **520 HP** (`armor.air_rotary`) · ROTARY **760 cm/s**, AIR_LOW · FA · sight 1600 cm · crew 2 + 1 = food 3 · kerosene 2 per rearm · ExperienceValue 165 |
| **Commands** | as TPL-AIR-HELI-ATTACK · default stance Aggressive · formation role `flank` |
| **Production** | **$1,650** · `AC-BLD-07` Rotorcraft Pad · prerequisites: `AC-BLD-07` · **20 s** · 3 food · 2 kerosene/rearm · no cap |
| **Veterancy** | Standard |
| **AI** | `ai.unit.attack_helicopter` |
| **Multiplayer** | MPA-AIR |
| **Presentation** | ANIM-AIR-ROTARY + chin-gun traverse. Icon: helicopter with missile |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.ac.air_04.atgm` ATGMs | SHAPED_CHARGE | 200–1150 cm | 110 / 4.0 s; AmmoCapacity 8 | guided_missile (jammable) | 6 | GROUND, STRUCTURE, WATER_SURFACE | 25 % / 100 % / 90 % / — / 70 % |
| `weapon.ac.air_04.gun` chin cannon | AUTOCANNON | 0–800 cm | 18 / 0.4 s (45 DPS); AmmoCapacity 60 bursts | shell_direct | 4 | GROUND, AIR_LOW | 65 % / 85 % / 10 % / 100 % rotary / 30 % |

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.ac.air_04.nap_of_earth` | Active (P) | 8 s: flies low; enemy air-radar contacts are suppressed for this unit, but small-arms effectiveness against it rises 25 % → 35 % | Self | free · 8 s · 30 s · self | Visual sight; MGs | Low-altitude icon | Use when approaching a known radar SAM |

**Research and upgrades:** `tech.ac.precision_vehicle_targeting`: ATGM +100 cm range. `tech.ac.aviation_servicing`: rearm faster.

**Counters:** *Beats (hard):* tanks, APCs and artillery without AA (RD-ARM-04, IA-ARM-04, MP-ARM-06). *Beaten by (hard):* AA guns and SAMs (RD-ARM-06/13, IA-ARM-08, MP-ARM-09), AT infantry in AA mode, fighters. *Situational:* strongest against Directorate armor columns before their AA arrives.

**Status:** identity E-R; everything else P.

### AC-AIR-05 — Utility Transport Helicopter

| Field | Specification |
| --- | --- |
| **Identity** | `unit.ac.air_05` · Atlantic Concord · Aircraft / utility helicopter · `TPL-AIR-HELI-TRANSPORT` · **Extended** · Playable |
| **Role** | **Primary:** move infantry and engineers and evacuate casualties (E-R). **Secondary:** fast-rope insertions. **Doctrine fit:** air-mobile forces (E). |
| **Inspiration and background** | 1980s twin-engine utility helicopter with door guns. C&C lineage (E-R): Chinook (RA1); NightHawk (RA2); Transport Helicopter (Ren). |
| **Visual identity** | No concept sheet: none (asset requirement). Silhouette: medium utility helicopter, sliding doors. |
| **Statistics** | **600 HP** (`armor.air_rotary`) · ROTARY 700 cm/s, AIR_LOW · FA · sight 1400 cm · **2 infantry slots** · crew 3 + 1 = food 4 · kerosene 2 per rearm · ExperienceValue 120 |
| **Commands** | Move, Stop (I) · Load, Unload, Fast-Rope (P) · default stance Hold Fire · formation role `protected` |
| **Production** | **$1,200** · `AC-BLD-07` · prerequisites: `AC-BLD-07` · **16 s** · 4 food · 2 kerosene · no cap |
| **Veterancy** | Standard (door gun can earn XP) |
| **AI** | `ai.unit.air_transport` |
| **Multiplayer** | MPA-AIR + MPA-TRANSPORT |
| **Presentation** | ANIM-AIR-ROTARY + doors, rope. Icon: helicopter with squad pip |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.ac.air_05.door_gun` door MG | SMALL_ARMS | 0–650 cm | 10 / 0.3 s | hitscan | 3 | GROUND | 100 % / 15 % / 2 % / — / 8 % |

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.shared.fast_rope` | Active (P, §10.1) | Unloads passengers while hovering; exposure window 10 s (6 s for AC-INF-07) | Ground point | free · 6–10 s · — · — | Ground fire during the window | Ropes | Fast-rope only if no AA observed within 1500 cm |
| `ability.ac.air_05.casualty_evacuation` | Passive (P) | Passengers heal 4 HP/s; squads delivered to AC-BLD-04 replenish 25 % faster | Passengers | free | — | Cross icon | Evacuate squads under 40 % members |

**Research and upgrades:** `tech.ac.aviation_servicing`.

**Counters:** *Beaten by (hard):* AA of every kind; fighters. *Situational:* mobility for engineers and AC-INF-07.

**Status:** identity E-R; everything else P.

### AC-AIR-06 — Heavy Transport Aircraft

| Field | Specification |
| --- | --- |
| **Identity** | `unit.ac.air_06` · Atlantic Concord · Aircraft / fixed-wing transport · `TPL-AIR-TRANSPORT` · **Extended** · Playable |
| **Role** | **Primary:** drop troops and field supply payloads; not a bomber by default (E-R). **Secondary:** emergency ammunition resupply. **Doctrine fit:** transport aviation (E). |
| **Inspiration and background** | Four-engine tactical airlifter. C&C lineage (E-R): Century Bomber (RA3); V-35 Ox (TW); Chinook (Gen). |
| **Visual identity** | No concept sheet: none (asset requirement). Silhouette: high-wing four-engine transport with rear ramp. |
| **Statistics** | **900 HP** (`armor.air_fixed`) · FIXED_WING 1600 cm/s, AIR_HIGH (descends to AIR_LOW for drops) · FA · sight 1400 cm · **4 infantry slots** or one supply payload · crew 4 + 1 = food 5 · kerosene 4 per sortie · no veterancy |
| **Commands** | Move (I) · Paradrop, Supply Drop, Return (P) · default stance Hold Fire · formation role `protected` |
| **Production** | **$2,200** · `AC-BLD-06` · prerequisites: `AC-BLD-06` · **30 s** · 5 food · 4 kerosene · cap 2 |
| **Veterancy** | none |
| **AI** | `ai.unit.air_transport` |
| **Multiplayer** | MPA-AIR + MPA-TRANSPORT |
| **Presentation** | ANIM-AIR-FIXED + ramp, parachutes. Icon: transport plane |

**Weapons:** none (unarmed; never a bomber by default, E-R).

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.ac.air_06.paradrop` | Active (P) | Drops up to 4 `airborne_qualified` squads in a 600 cm scatter around a point; the aircraft is AIR_LOW for 8 s over the drop zone | Ground point | kerosene · 8 s exposure · per sortie · — | AA under the corridor | Canopies | Drop only if no AA observed within 2000 cm |
| `ability.ac.air_06.supply_drop` | Active (P) | Drops a 30-unit ammunition crate that acts as a 60 s resupply point (§10.4) | Ground point | $600 crate + kerosene · 60 s · per sortie | Enemy capture of the crate is not possible; enemies can destroy it (200 HP) | Crate with smoke | Use for cut-off groups |

**Research and upgrades:** `tech.ac.aviation_servicing`.

**Counters:** *Beaten by (hard):* fighters (RD-AIR-01, IA-AIR-01), SAMs. *Situational:* vertical envelopment and supply of isolated forces.

**Status:** identity E-R; everything else P.

### AC-AIR-07 — Electronic Surveillance Aircraft

| Field | Specification |
| --- | --- |
| **Identity** | `unit.ac.air_07` · Atlantic Concord · Aircraft / AEW and signals aircraft · `TPL-AIR-SURVEILLANCE` · **Extended** · Playable |
| **Role** | **Primary:** expensive airborne early warning and signal collection (E-R). **Secondary:** cue interceptors. **Doctrine fit:** observe extensive territories (E). |
| **Inspiration and background** | Turboprop or jet AEW aircraft with a rotodome. C&C lineage (E-R): Spy Plane (RA1); Spy Satellite (Gen, function); Recon Drone (TT, function). |
| **Visual identity** | No concept sheet: none (asset requirement). Silhouette: airliner-like body with a rotodome. |
| **Statistics** | **700 HP** (`armor.air_fixed`) · FIXED_WING (loiter) 1500 cm/s, AIR_HIGH · FA · sight 3200 cm (ground) · **`DETECT.AIR_RADAR` 6000 cm**, **`DETECT.EMITTER` 6000 cm** · crew 6 (abstract) + 1 = food 7 · kerosene 4 per sortie · no veterancy |
| **Commands** | Move (I) · Orbit, Return (P) · default stance Hold Fire · formation role `protected` |
| **Production** | **$2,600** · `AC-BLD-06` · prerequisites: `AC-BLD-16` and `tech.shared.radar_signal_processing` · **32 s** · 7 food · 4 kerosene · cap 1 |
| **Veterancy** | none |
| **AI** | `ai.unit.air_surveillance` |
| **Multiplayer** | MPA-AIR + MPA-SENSOR (contacts only) |
| **Presentation** | ANIM-AIR-FIXED + rotodome spin. Icon: radar dome |

**Weapons:** none (unarmed).

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.ac.air_07.surveillance_orbit` | Active (P) | 120 s orbit: air contacts and emitter contacts within 6000 cm; friendly fighters intercept contacts in its footprint without needing visual acquisition first (they still need sight to fire) | Orbit point | 4 kerosene · 120 s · 20 s turnaround | Fighters; long-range SAMs; jamming | Footprint ring (owner) | Orbit 2000 cm behind friendly lines |

**Research and upgrades:** `tech.shared.electronic_countermeasures`: contacts persist 5 s in jamming.

**Counters:** *Beaten by (hard):* fighters (RD-AIR-01, IA-AIR-01), long-range SAMs. *Situational:* information superiority without omniscience (contacts, not identities; E: talent protected canon).

**Status:** identity E-R; everything else P.

### AC-AIR-08 — Tactical Bomber

| Field | Specification |
| --- | --- |
| **Identity** | `unit.ac.air_08` · Atlantic Concord · Aircraft / conventional bomber · `TPL-AIR-STRIKE` · **Extended** · Playable |
| **Role** | **Primary:** large conventional payload at high cost and airfield footprint (E-R). **Secondary:** destroy fortified lines and bases. **Doctrine fit:** air power is decisive but resource-hungry (E). |
| **Inspiration and background** | Swing-wing or twin-engine tactical bomber. C&C lineage (E-R): Aurora Bomber (Gen); Century Bomber (RA3); Harbinger Gunship (Uprising, payload role). |
| **Visual identity** | No concept sheet: none (asset requirement). Silhouette: large swept-wing bomber, long fuselage. |
| **Statistics** | **1,000 HP** (`armor.air_fixed`) · FIXED_WING 1500 cm/s, AIR_HIGH · FA · sight 1800 cm · crew 2 + 1 = food 3 · kerosene 5 per sortie · ExperienceValue 280 |
| **Commands** | Attack (I) · Strike Area, Return (P) · default stance Hold Fire · formation role `reserve` |
| **Production** | **$2,800** · `AC-BLD-06` · prerequisites: `AC-BLD-16`, `AC-BLD-12` powered · **34 s** · 3 food · 5 kerosene · cap 3 (airfield footprint) |
| **Veterancy** | Standard |
| **AI** | `ai.unit.air_strike` |
| **Multiplayer** | MPA-AIR |
| **Presentation** | ANIM-AIR-FIXED + bomb bay. Bomb-line preview. Icon: bomber |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.ac.air_08.bombs` bomb stick | HIGH_EXPLOSIVE | overflight; line 1600 cm | 8 × 200 along the line; AmmoCapacity 8 | bomb_drop; each bomb full 120 cm, max 260 cm, edge 30 %; scatter 80–200 cm | 12 | GROUND, STRUCTURE | 110 % / 45 % / 12 % / — / 100 % |

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.ac.air_08.carpet_line` | Active (P) | Orders a bomb line between two points ≤ 1600 cm apart | Two ground points (observed for normal scatter, unobserved doubles scatter) | ammo · one pass · — | SAM and fighters on the approach | Line preview (owner) | Use on fortified lines and clustered bases |

**Research and upgrades:** `tech.ac.aviation_servicing`.

**Counters:** *Beats (hard):* bases, fortifications, massed infantry (RD-INF mass), artillery parks. *Beaten by (hard):* fighters, SAM belts (RD-ARM-13, RD-DEF air defense). *Situational:* needs AC-AIR-01 escort.

**Status:** identity E-R; everything else P.

### AC-AIR-09 — Armed Observation Helicopter

| Field | Specification |
| --- | --- |
| **Identity** | `unit.ac.air_09` · Atlantic Concord · Aircraft / light scout helicopter · `TPL-AIR-HELI-ATTACK` · **Extended** · Playable |
| **Role** | **Primary:** persistent low-volume patrol with ground attack option (E-R). **Secondary:** target designation; concealed-unit detection. **Doctrine fit:** reconnaissance that improves several nearby forces (E). |
| **Inspiration and background** | Light observation helicopter with a mast sight, MG pod and light rockets. C&C lineage (E-R): Hammerhead (KW); Orca (TT); Spanner (TT, support role). |
| **Visual identity** | No concept sheet: none (asset requirement). Silhouette: small egg-shaped helicopter with a mast-mounted sight ball. |
| **Statistics** | **380 HP** (`armor.air_rotary`) · ROTARY **800 cm/s**, AIR_LOW · FA · sight **1900 cm** · `DETECT.CONCEALED` 1000 cm · crew 2 + 1 = food 3 · kerosene 1 per rearm · ExperienceValue 100 |
| **Commands** | as TPL-AIR-HELI-ATTACK + Designate (P) · default stance Guard · formation role `scout_screen` |
| **Production** | **$1,000** · `AC-BLD-07` · prerequisites: `AC-BLD-07` · **14 s** · 3 food · 1 kerosene · no cap |
| **Veterancy** | Standard |
| **AI** | `ai.unit.attack_helicopter` with scout weights (0.6 / 0.6 / 0 / 0 / 0.2); designates for friendly fire |
| **Multiplayer** | MPA-AIR |
| **Presentation** | ANIM-AIR-ROTARY + sight-ball turn. Icon: small helicopter with an eye |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.ac.air_09.mg` MG pod | SMALL_ARMS | 0–700 cm | 12 / 0.3 s | hitscan | 3 | GROUND | 100 % / 15 % / 2 % / — / 8 % |
| `weapon.ac.air_09.rockets` light rockets | HIGH_EXPLOSIVE | 0–850 cm | 6 × 40 per volley / 10 s; AmmoCapacity 12 | ballistic; scatter 50–120; full 50, max 120, edge 40 % | 6 | GROUND, STRUCTURE | 110 % / 45 % / 12 % / — / 100 % |

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.ac.air_09.designate` | Active (P) | As AC-INF-08 *Designate*, from the air (1900 cm) | Observed target | free · 20 s · 30 s · 1900 cm | AA; smoke | Laser-dot | Designate for AC-ARM-07/08 and AC-AIR-02/03 |

**Research and upgrades:** `tech.shared.radar_signal_processing` does not apply. `tech.ac.aviation_servicing`.

**Counters:** *Beats (hard):* scouts, infantry in the open, concealed teams. *Beaten by (hard):* any AA, AC-INF-03-class MANPADS, AA guns. *Situational:* observation platform first, attacker second.

**Status:** identity E-R; everything else P.

### AC-AIR-10 — Airborne Support Gunship

| Field | Specification |
| --- | --- |
| **Identity** | `unit.ac.air_10` · Atlantic Concord · Aircraft / fixed-wing gunship (orbit) · `TPL-AIR-STRIKE` (loiter variant) · **Extended** · Playable |
| **Role** | **Primary:** specialist suppressive fire while protecting deployments (E-R). **Secondary:** cover airborne landings. **Doctrine fit:** air support for rapid deployment (E). |
| **Inspiration and background** | Transport-derived side-firing gunship with autocannon and a light howitzer. C&C lineage (E-R): Spectre Gunship (Gen, call-in role); Harbinger Gunship (Uprising); Hammerhead (KW). |
| **Visual identity** | No concept sheet: none (asset requirement). Silhouette: four-engine transport with side gun ports. |
| **Statistics** | **1,100 HP** (`armor.air_fixed`) · FIXED_WING orbit 1200 cm/s; **AIR_LOW while orbiting** (deliberately vulnerable to AA guns) · FA · sight 2000 cm · crew 8 (abstract) + 1 = food 9 · kerosene 5 per sortie · ExperienceValue 280 |
| **Commands** | Move (I) · Orbit Fire, Return (P) · default stance Aggressive inside the orbit · formation role `reserve` |
| **Production** | **$2,800** · `AC-BLD-06` · prerequisites: `AC-BLD-16` · **34 s** · 9 food · 5 kerosene · cap 1 |
| **Veterancy** | Standard |
| **AI** | `ai.unit.air_strike`; orbit only where no observed SAM covers the orbit |
| **Multiplayer** | MPA-AIR + MPA-INDIRECT (howitzer scatter) |
| **Presentation** | ANIM-AIR-FIXED + side guns. Orbit ring visible to all observers of the aircraft. Icon: plane with side guns |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.ac.air_10.cannon` side autocannon | AUTOCANNON | inside orbit (1200 cm radius) | 20 / 0.4 s | shell_direct | 5 | GROUND | 65 % / 85 % / 10 % / — / 30 % |
| `weapon.ac.air_10.howitzer` side howitzer | HIGH_EXPLOSIVE | inside orbit | 90 / 5 s; full 100, max 220, edge 30 %; scatter 60–150 | ballistic | 12 | GROUND, STRUCTURE | 110 % / 45 % / 12 % / — / 100 % |

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.ac.air_10.orbit_fire` | Active (P) | 60 s orbit around a point; fires at observed hostiles inside 1200 cm | Ground point | 5 kerosene · 60 s · per sortie | AA guns and MANPADS (AIR_LOW); SAMs | Orbit ring | Cover landing zones and defended deployments |

**Research and upgrades:** `tech.ac.aviation_servicing`.

**Counters:** *Beats (hard):* infantry and light vehicles inside the orbit. *Beaten by (hard):* AA guns (RD-ARM-06, IA-ARM-08, MP-ARM-09), MANPADS, SAMs, fighters. *Situational:* only after AA suppression.

**Status:** identity E-R; everything else P.

### AC-AIR-11 — Nuclear Delivery Bomber

| Field | Specification |
| --- | --- |
| **Identity** | `unit.ac.air_11` · Atlantic Concord · Aircraft / strategic delivery bomber · `TPL-AIR-STRIKE` · **Conditional** · Playable · **cap 1** |
| **Role** | **Primary:** Concord-only completed tactical-bomb delivery vehicle; requires all abstract nuclear gates (E). **Secondary:** none (it never carries conventional bombs, to keep the warning readable). **Doctrine fit:** established nuclear power whose delivery is by bomber (E: Nuclear system). |
| **Inspiration and background** | Strategic or tactical jet bomber airframe. Abstract game interaction only. C&C lineage (E-R): Aurora Bomber (Gen, airframe archetype); Century Bomber (RA3); Vindicator (RA3). |
| **Visual identity** | No concept sheet: none (asset requirement). Silhouette: large swept-wing bomber with a distinct anti-flash white underside. |
| **Statistics** | **1,200 HP** (`armor.air_fixed`) · FIXED_WING 1700 cm/s, AIR_HIGH · FA · sight 1800 cm · crew 3 + 1 = food 4 · kerosene 6 per sortie · no veterancy |
| **Commands** | Move (I) · Deliver Weapon, Return (P) · default stance Hold Fire · formation role `protected` |
| **Production** | **$3,200** for the airframe · `AC-BLD-06` with `AC-DEF-13` Strategic Air Command Hangar operational · prerequisites: a **completed bomb in custody** from `AC-DEF-12` (E) · **40 s** · 4 food · 6 kerosene · **cap 1** (E-bound to the completed weapon) |
| **Veterancy** | none |
| **AI** | `ai.unit.air_strike` under the nuclear contract only; AI never launches without both custody and authorization |
| **Multiplayer** | MPA-AIR + **MPA-STRATEGIC** (custody and delivery state; warnings exactly as the nuclear contract defines) |
| **Presentation** | ANIM-AIR-FIXED + bay. Strategic warning presentation owned by the nuclear system. Icon: bomber with a strategic marker |

**Weapons:** none conventional. Carries one completed weapon object (E). Its effect, warning and damage are owned by the [Nuclear Weapons System](../NUCLEAR_WEAPONS_SYSTEM.md), not by this file.

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.ac.air_11.deliver_weapon` | Active (P; E gates) | Flies to a target point and releases the completed weapon; resolution per the nuclear contract. If the bomber is destroyed before release, the weapon's custody outcome follows the nuclear contract (U-AC-04) | Target point; custody and all nuclear gates satisfied | weapon consumed · flight time · — | Interceptors, SAM belts, AC-DEF-11-class missile defense | Strategic warning per contract | Only under explicit authorization |

**Research and upgrades:** none. No unit upgrade can touch nuclear restrictions (E: research doc §9.4).

**Counters:** *Beaten by (hard):* fighters (RD-AIR-01, IA-AIR-01), SAM belts and missile-defense sites. *Situational:* completion alone never wins (E).

**Status:** delivery role, gates and custody E; airframe numbers P; **U-AC-04** (loss-before-release custody).

## 7. Naval, riverine and amphibious assets

Shared naval rules (P): naval units exist only on maps with an authored water layer and an owned `AC-BLD-08` Adriatic Naval Station (E-R map restriction). Crews are abstracted for food (§4.5). Ships repair at `AC-BLD-08` at 12 HP/s under §10.2 pricing. Ports, transit permissions and Unaligned territorial waters follow jurisdiction rules (E).

### AC-SEA-01 — Port Security Launch

| Field | Specification |
| --- | --- |
| **Identity** | `unit.ac.sea_01` · Atlantic Concord · Naval / patrol boat · `TPL-NAV-PATROL` · **Conditional** (naval map) · Playable |
| **Role** | **Primary:** patrol and anti-small-boat defense (E-R). **Secondary:** harbour picket against swimmers; river patrol. **Doctrine fit:** protect shipping and ports (E: Air and Maritime Power). |
| **Inspiration and background** | Fast harbour patrol launch with an HMG and a grenade launcher. C&C lineage (E-R): Gunboat (RA1); Gunboat (TD); Hydrofoil (RA3). |
| **Visual identity** | No concept sheet: none (asset requirement). Silhouette: small grey launch with a forward gun tub. |
| **Statistics** | **380 HP** (`armor.nav_light`) · SURFACE_SHALLOW **650 cm/s** · FN 250 cm · sight 1600 cm · `DETECT.CONCEALED` 900 cm (swimmers and boats) · crew 4 · fuel light · ExperienceValue 65 |
| **Commands** | as TPL-NAV-PATROL · default stance Aggressive · formation role `scout_screen` |
| **Production** | **$650** · `AC-BLD-08` · prerequisites: `AC-BLD-08` · **12 s** · 4 food · 1 petroleum · no cap |
| **Veterancy** | Standard |
| **AI** | `ai.unit.naval_patrol` |
| **Multiplayer** | MPA-STD |
| **Presentation** | ANIM-NAV + bow spray. Icon: small boat |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.ac.sea_01.hmg` HMG | SMALL_ARMS | 0–750 cm | 14 / 0.3 s | hitscan | 4 | GROUND, WATER_SURFACE, AIR_LOW | 100 % / 15 % / 2 % / 25 % rotary / 8 %; NAV_LIGHT 30 % |
| `weapon.ac.sea_01.gl` automatic grenade launcher | HIGH_EXPLOSIVE | 100–800 cm | 18 / 1.0 s; full 50, max 120, edge 40 % | lobbed | 6 | GROUND, WATER_SURFACE | 110 % / 45 % / 12 % / — / 100 %; NAV_LIGHT 60 % |

**Abilities:** no unit-specific ability (detection is a stat).

**Research and upgrades:** `tech.ac.naval_servicing` (AC-BLD-08, $600): naval repair +30 %.

**Counters:** *Beats (hard):* swimmers, landing craft, small boats (RD-SEA-01, MP-SEA-02/03, IA-SEA-01). *Beaten by (hard):* missile boats and frigates, attack helicopters. *Situational:* port and river control.

**Status:** identity E-R; everything else P.

### AC-SEA-02 — Amphibious Landing Carrier

| Field | Specification |
| --- | --- |
| **Identity** | `unit.ac.sea_02` · Atlantic Concord · Naval / landing craft · `TPL-NAV-TRANSPORT` · **Conditional** · Playable |
| **Role** | **Primary:** move soldiers and vehicles to Adriatic shore access points (E-R). **Secondary:** river ferry where permitted. **Doctrine fit:** support coastal deployments (E). |
| **Inspiration and background** | Utility landing craft with a bow ramp. C&C lineage (E-R): Amphibious Transport (RA2); Riptide ACV (RA3); Hovercraft (TD). |
| **Visual identity** | No concept sheet: none (asset requirement). Silhouette: flat-bottomed craft with a bow ramp and wheelhouse aft. |
| **Statistics** | **900 HP** (`armor.nav_light`) · SURFACE_SHALLOW 450 cm/s; can **beach** on authored landing points · FN 400 cm · sight 1200 cm · **4 infantry slots + 2 vehicle slots** (MBT = 2 slots) · crew 6 · fuel medium · no veterancy |
| **Commands** | Move, Stop (I) · Load, Unload, Beach (P) · default stance Hold Fire · formation role `protected` |
| **Production** | **$1,100** · `AC-BLD-08` · prerequisites: `AC-BLD-08` · **18 s** · 6 food · 3 petroleum · no cap |
| **Veterancy** | none |
| **AI** | `ai.unit.transport` |
| **Multiplayer** | MPA-TRANSPORT |
| **Presentation** | ANIM-NAV + ramp. Icon: landing craft |

**Weapons:** none (unarmed).

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.shared.transport` | Active (P) | As §10.1; unload only while beached (ramp down, 3 s) | Own units | free | Shore defenses during the ramp window | Ramp | Beach at points marked by AC-INF-09 *Shore Reconnaissance* |

**Research and upgrades:** `tech.ac.naval_servicing`.

**Counters:** *Beaten by (hard):* coastal artillery, patrol and missile boats, aircraft. *Situational:* opens a second front on coastal maps.

**Status:** identity E-R; everything else P.

### AC-SEA-03 — Coastal Missile Boat

| Field | Specification |
| --- | --- |
| **Identity** | `unit.ac.sea_03` · Atlantic Concord · Naval / fast attack craft · `TPL-NAV-COMBATANT` (NAV_LIGHT) · **Conditional** · Playable |
| **Role** | **Primary:** small fast anti-ship and coastal interdiction platform (E-R). **Secondary:** strike coastal structures. **Doctrine fit:** maritime corridors equal inland territory in value (E). |
| **Inspiration and background** | Fast missile boat with four anti-ship missiles and a light gun. C&C lineage (E-R): Destroyer (RA1, simplified); Sea Scorpion (RA2); Naginata Cruiser (RA3, role). |
| **Visual identity** | No concept sheet: none (asset requirement). Silhouette: sleek hull with angled missile canisters aft. |
| **Statistics** | **520 HP** (`armor.nav_light`) · SURFACE_SHALLOW **720 cm/s** · FN 300 cm · sight 1800 cm · crew 8 · fuel medium · ExperienceValue 150 |
| **Commands** | as TPL-NAV-COMBATANT · default stance Aggressive · formation role `flank` |
| **Production** | **$1,500** · `AC-BLD-08` · prerequisites: `AC-BLD-16` · **20 s** · 8 food · 3 petroleum · no cap |
| **Veterancy** | Standard |
| **AI** | `ai.unit.naval_combatant` |
| **Multiplayer** | MPA-STD |
| **Presentation** | ANIM-NAV + canister launch. Icon: boat with missile |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.ac.sea_03.asm` anti-ship missiles | ANTI_SHIP | 400–2600 cm | 220 / 4 s; AmmoCapacity 4 | guided_missile (jammable) | — | WATER_SURFACE, STRUCTURE (coastal) | — / 40 % / 30 % / — / 60 %; NAV_L 120 %, NAV_H 100 % |
| `weapon.ac.sea_03.gun` 40 mm gun | AUTOCANNON | 0–900 cm | 22 / 0.6 s | shell_direct | 4 | WATER_SURFACE, AIR_LOW, GROUND | 65 % / 85 % / 10 % / 100 % rotary / 30 %; NAV_L 90 % |

**Abilities:** no unit-specific ability.

**Research and upgrades:** `tech.shared.electronic_countermeasures`: guided missiles keep scatter 150 cm in jamming.

**Counters:** *Beats (hard):* ships (RD-SEA-05, IA-SEA-05), landing craft, coastal structures. *Beaten by (hard):* attack helicopters and strike jets, frigate SAM-gun combinations, submarines. *Situational:* hit-and-run; runs dry after 4 missiles.

**Status:** identity E-R; everything else P.

### AC-SEA-04 — Guided-Missile Frigate

| Field | Specification |
| --- | --- |
| **Identity** | `unit.ac.sea_04` · Atlantic Concord · Naval / frigate · `TPL-NAV-COMBATANT` · **Conditional** · Playable |
| **Role** | **Primary:** fleet air defense and convoy escort with costly fuel (E-R). **Secondary:** anti-submarine screen. **Doctrine fit:** surface combatants protect shipping (E). |
| **Inspiration and background** | 1980s guided-missile frigate with an area SAM, a medium gun and ASW torpedoes. C&C lineage (E-R): Aegis Cruiser (RA2); Destroyer (RA1); Hydrofoil (RA3). |
| **Visual identity** | No concept sheet: none (asset requirement). Silhouette: single funnel, mast radar, forward gun, aft hangar. |
| **Statistics** | **2,000 HP** (`armor.nav_heavy`) · SURFACE_DEEP **340 cm/s** · FN 700 cm · sight 2000 cm · **`DETECT.AIR_RADAR` 3600 cm**, **`DETECT.SUBMERGED` 1800 cm** · crew 14 (abstract) · fuel heavy · ExperienceValue 300 |
| **Commands** | as TPL-NAV-COMBATANT · default stance Aggressive · formation role `escort` |
| **Production** | **$3,000** · `AC-BLD-08` · prerequisites: `AC-BLD-16`, `AC-BLD-12` powered · **48 s** · 14 food · **10 petroleum** ("costly fuel", E-R) · cap 3 |
| **Veterancy** | Standard |
| **AI** | `ai.unit.naval_combatant` |
| **Multiplayer** | MPA-STD + MPA-SENSOR |
| **Presentation** | ANIM-NAV + turrets, radar. Icon: frigate |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.ac.sea_04.sam` area SAM | FLAK_AA | 0–2400 cm | 110 / 3 s; AmmoCapacity 16 | guided_missile (jammable) | — | AIR_LOW, AIR_HIGH | — / — / — / 110 / 100 % / — |
| `weapon.ac.sea_04.gun` 76 mm gun | HIGH_EXPLOSIVE | 0–2000 cm | 45 / 1.5 s; scatter 40–120 | shell_direct (water) / ballistic (shore) | 6 | WATER_SURFACE, GROUND, STRUCTURE | 110 % / 45 % / 12 % / — / 100 %; NAV_L 60 %, NAV_H 25 % |
| `weapon.ac.sea_04.asw` ASW torpedo | TORPEDO | 0–1400 cm | 260 / 8 s; AmmoCapacity 6 | torpedo | — | WATER_SUBSURFACE, WATER_SURFACE | NAV_L 110 %, NAV_H 100 % |

**Abilities:** no unit-specific ability (sensors are stats).

**Research and upgrades:** `tech.shared.radar_signal_processing`: air radar +600 cm. `tech.ac.naval_servicing`.

**Counters:** *Beats (hard):* aircraft over water, submarines (RD-SEA-04, IA-SEA-04). *Beats (soft):* light craft. *Beaten by (hard):* massed missile boats and submarines that close undetected, strike jets with anti-ship weapons. *Situational:* the fleet's umbrella.

**Status:** identity E-R; everything else P.

### AC-SEA-05 — Adriatic Attack Submarine

| Field | Specification |
| --- | --- |
| **Identity** | `unit.ac.sea_05` · Atlantic Concord · Naval / attack submarine · `TPL-NAV-SUB` · **Conditional** · Playable |
| **Role** | **Primary:** covert sea-area denial and shipping pressure (E-R). **Secondary:** reconnaissance of ports. **Doctrine fit:** maritime corridors (E). |
| **Inspiration and background** | Diesel-electric or nuclear-powered attack submarine (propulsion abstract). C&C lineage (E-R): Submarine (RA1); Typhoon Attack Submarine (RA2); Akula Submarine (RA3). |
| **Visual identity** | No concept sheet: none (asset requirement). Silhouette: teardrop hull and sail when surfaced; wake ripple when submerged and observed. |
| **Statistics** | **950 HP** (`armor.nav_heavy`) · SUBSURFACE **340 cm/s** (submerged ×0.8) · FN 500 cm · sight 1600 cm (surfaced), periscope 1200 cm (submerged) · Submerged state (§9.1) · crew 6 (abstract) · fuel heavy · ExperienceValue 210 |
| **Commands** | as TPL-NAV-SUB · default stance Hold Fire while submerged · formation role `flank` |
| **Production** | **$2,100** · `AC-BLD-08` · prerequisites: `AC-BLD-16` · **40 s** · 6 food · 5 petroleum · cap 2 |
| **Veterancy** | Standard |
| **AI** | `ai.unit.submarine` |
| **Multiplayer** | **MPA-COVERT** (submerged position never replicated to opponents without `DETECT.SUBMERGED`) |
| **Presentation** | ANIM-NAV-SUB + dive/surface. Torpedo wake. Icon: submarine |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.ac.sea_05.torpedo` heavyweight torpedoes | TORPEDO | 0–1400 cm | 320 / 6 s; AmmoCapacity 8; firing reveals for 4 s | torpedo 900 cm/s | — | WATER_SURFACE, WATER_SUBSURFACE | NAV_L 110 %, NAV_H 100 % |

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.shared.dive_surface` | Active (P) | Toggle Submerged (§9.1); 4 s transition; rearm and repair only when surfaced at `AC-BLD-08` | Self | free · 4 s · 6 s | `DETECT.SUBMERGED` frigates, ASW aircraft, coastal sonar | Depth icon | Stay submerged until a firing solution exists |

**Research and upgrades:** `tech.ac.naval_servicing`.

**Counters:** *Beats (hard):* surface ships without ASW, landing craft, tankers at sea. *Beaten by (hard):* ASW frigates (RD-SEA-05/06), maritime patrol aircraft. *Situational:* sea denial; cannot touch land.

**Status:** identity E-R; everything else P.

### AC-SEA-06 — Offshore Fire-Support Ship

| Field | Specification |
| --- | --- |
| **Identity** | `unit.ac.sea_06` · Atlantic Concord · Naval / naval gunfire ship · `TPL-NAV-COMBATANT` · **Conditional** · Playable |
| **Role** | **Primary:** coastal bombardment in suitable deep water, map-limited (E-R). **Secondary:** support amphibious landings. **Doctrine fit:** fortified targets may need sustained conventional fire (E). |
| **Inspiration and background** | Destroyer-sized gun ship with two dual-purpose turrets. C&C lineage (E-R): Cruiser (RA1); Assault Destroyer (RA3); Shogun Battleship (RA3, function). |
| **Visual identity** | No concept sheet: none (asset requirement). Silhouette: two large forward turrets, tall bridge. |
| **Statistics** | **2,400 HP** (`armor.nav_heavy`) · SURFACE_DEEP **300 cm/s** · FN 800 cm · sight 2000 cm · crew 18 (abstract) · fuel heavy · ExperienceValue 320 |
| **Commands** | as TPL-NAV-COMBATANT + Fire Mission (P) · default stance Hold Ground · formation role `support_weapon` |
| **Production** | **$3,200** · `AC-BLD-08` · prerequisites: `AC-BLD-16` · **52 s** · 18 food · 10 petroleum · cap 2 |
| **Veterancy** | Standard |
| **AI** | `ai.unit.artillery` (naval) |
| **Multiplayer** | MPA-STD + MPA-INDIRECT |
| **Presentation** | ANIM-NAV + turret recoil. Icon: ship with shells |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.ac.sea_06.main` twin naval guns | HIGH_EXPLOSIVE | **1200–3600 cm** (shore) | 2 × 140 / 7 s; AmmoCapacity 40; full 120, max 280, edge 30 %; scatter 120–380 (×0.5 on AC-INF-09 marks) | ballistic | 12 | GROUND, STRUCTURE, WATER_SURFACE | 110 % / 45 % / 12 % / — / 100 %; NAV_L 60 %, NAV_H 25 % |
| `weapon.ac.sea_06.ciws` close-in gun | AUTOCANNON | 0–700 cm | 15 / 0.2 s | shell_direct | — | AIR_LOW, AIR_HIGH | — / — / — / 100 % rotary, 60 % fixed / — |

**Abilities:** no unit-specific ability; Fire Mission is the shared order.

**Research and upgrades:** `tech.ac.naval_servicing`; `tech.ac.improved_fire_control` applies (scatter −20 %).

**Counters:** *Beats (hard):* coastal defenses, shore infantry and structures. *Beaten by (hard):* submarines, anti-ship missiles (RD-SEA-03, IA-SEA-03, coastal batteries), strike jets. *Situational:* map-limited to deep water within range of the shore.

**Status:** identity and map limit E-R; everything else P.

### AC-SEA-07 — Fleet Air-Control Vessel

| Field | Specification |
| --- | --- |
| **Identity** | `unit.ac.sea_07` · Atlantic Concord · Naval / aviation-control ship · `TPL-NAV-TRANSPORT` (HQ ship) · **Scenario** · Playable only where a scenario places it |
| **Role** | **Primary:** scenario-tier ship coordinating naval aviation rather than ubiquitous carrier production (E-R). **Secondary:** mobile helicopter rearm point; air picture. **Doctrine fit:** air and maritime power combined (E). |
| **Inspiration and background** | Helicopter-carrying command ship. C&C lineage (E-R): Aircraft Carrier (RA2); Aircraft Carrier (RA3); Planetary Assault Carrier (TW, deployment function). |
| **Visual identity** | No concept sheet: none (asset requirement). Silhouette: flat aft flight deck, island superstructure. |
| **Statistics** | **3,000 HP** (`armor.nav_heavy`) · SURFACE_DEEP 280 cm/s · FN 900 cm · sight 2200 cm · `DETECT.AIR_RADAR` 5000 cm · crew 20 (abstract) · fuel heavy · no veterancy |
| **Commands** | Move, Stop (I) · Coordinate Air (P) · default stance Hold Fire · formation role `protected` |
| **Production** | **Not trainable.** Placed by scenario authoring only (E-R scenario tier). If a scenario enables production: $4,000, `AC-BLD-08`, 60 s, cap 1 |
| **Veterancy** | none |
| **AI** | `ai.unit.naval_combatant` with defensive posture |
| **Multiplayer** | MPA-STD + MPA-SENSOR |
| **Presentation** | ANIM-NAV + deck lights. Icon: carrier deck |

**Weapons**

| ID | Type | Range | Damage / cycle | Model / AoE | Suppression | Layers | INF / VEH-L / VEH-H / AIR / STR |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `weapon.ac.sea_07.ciws` point-defense guns | AUTOCANNON | 0–700 cm | 15 / 0.2 s | shell_direct | — | AIR_LOW, AIR_HIGH | — / — / — / 100 % rotary, 60 % fixed / — |

**Abilities**

| ID | Kind | Effect | Activation / targets | Cost · duration · cooldown · range | Counters | Feedback | AI rule |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.ac.sea_07.helicopter_deck` | Passive (P) | Acts as an `AC-BLD-07` pad with 2 slots for rearm and repair | Own helicopters | kerosene from ship stock (40) | Sinking the ship | Deck slots | Rearm helicopters operating offshore |
| `ability.ac.sea_07.air_coordination` | Passive (P) | Friendly aircraft within 3000 cm see air contacts from its radar | Own aircraft | free | Jamming | Contact ring | — |

**Research and upgrades:** none (scenario asset).

**Counters:** *Beaten by (hard):* submarines, anti-ship missiles. *Situational:* scenario centrepiece; never a skirmish build item.

**Status:** identity and scenario tier E-R; everything else P.

## 8. Faction roster summary

USD and build seconds are PB. "Hard counters (to it)" lists opposing roster IDs; "It hard-counters" lists archetypes or IDs.

| ID | Name | Category | Template | Tier | Primary role | HP | Armor | Speed | USD | Build s | Producer | Hard counters (to it) | It hard-counters |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| AC-INF-01 | Professional Rifle Section | Infantry | INF-LINE | Core | Line rifle squad | 300 | INF_L | 210 | 360 | 9 | AC-BLD-04 | IA-INF-05, RD-INF-03, artillery | Agents, scouts |
| AC-INF-02 | Urban Security Team | Infantry | INF-LINE | Ext | Building clearing, checkpoints | 280 | INF_L | 210 | 380 | 10 | AC-BLD-04 | RD-INF-09, IA-INF-08 | Garrisons, agents |
| AC-INF-03 | Shoulder Missile Team | Infantry | INF-AT | Core | Anti-armor, limited AA | 210 | INF_L | 200 | 560 | 11 | AC-BLD-04 | RD-INF-03, IA-INF-05, MP-INF-04 | Light vehicles, APCs, helicopters |
| AC-INF-04 | Grenadier Section | Infantry | INF-LINE | Core | Area suppression | 280 | INF_L | 205 | 400 | 10 | AC-BLD-04 | Marksmen, armor | Massed and garrisoned infantry |
| AC-INF-05 | Field Medical Corps | Infantry | INF-SUPPORT | Core | Healing, replenishment | 120 | INF_L | 215 | 420 | 10 | AC-BLD-04 | Raiders, marksmen | — |
| AC-INF-06 | Combat Engineering Team | Infantry | INF-SUPPORT | Core | Capture, repair, clearing | 180 | INF_L | 205 | 450 | 12 | AC-BLD-04 | Any armed unit | Mines, obstacles |
| AC-INF-07 | Airborne Assault Section | Infantry | INF-LINE | Ext | Air-inserted riflemen | 280 | INF_L | 215 | 420 | 11 | AC-BLD-04 | AA in corridor, HMG | Rear-area support |
| AC-INF-08 | Long-Range Marksman Pair | Infantry | INF-SPEC | Core | Concealed precision, spotting | 120 | INF_L | 205 | 650 | 13 | AC-BLD-04 | Scouts, detectors, vehicles | All infantry |
| AC-INF-09 | Coastal Reconnaissance Detachment | Infantry | INF-SPEC | Ext (cap 2) | Amphibious raids, designation | 255 | INF_L | 215 | 1,200 | 18 | AC-BLD-08 | Patrol boats, UN-INF-03 | Harbour crews |
| AC-INF-10 | Counterintelligence Officer | Infantry | INF-SPEC | Ext | `DETECT.SPY`, investigation | 130 | INF_L | 210 | 700 | 14 | AC-BLD-04 | Any combat unit | Agents, saboteurs |
| AC-INF-11 | Concord Field Agent | Infantry | INF-COVERT | Cond | Intel, late spy charges | 100 | INF_L | 215 | 1,300 | 20 | AC-BLD-04 | `DETECT.SPY`, UN-INF-10/11 | Static infrastructure |
| AC-INF-12 | Psychological Operations Officer | Infantry | INF-COVERT | Cond (cap 2) | Psy-Ops | 110 | INF_L | 210 | 1,600 | 22 | AC-BLD-04 | Counterintelligence, officers | Isolated or suppressed units |
| AC-INF-13 | Signals Intercept Team | Infantry | INF-SPEC | Ext | Emitter detection, report check | 120 | INF_L | 200 | 600 | 12 | AC-BLD-04 | Any combat unit | Hidden EW (locates) |
| AC-INF-14 | Special Operations Commander | Infantry | INF-COMMAND | Ext (cap 1) | Talent commander | 480 | INF_P | 225 | 2,400 | 30 | AC-BLD-04 | Marksmen, jamming, artillery | — (multiplier) |
| AC-ARM-01 | Escort Jeep | Vehicle | VEH-LIGHT | Core | Escort, anti-infantry | 320 | VEH_SOFT | 510 | 560 | 9 | AC-BLD-05 | AT infantry, any tank | Infantry, scouts |
| AC-ARM-02 | Rapid Armored Carrier | Vehicle | VEH-TRANSPORT | Core | APC (amphibious) | 600 | VEH_L | 400 | 900 | 14 | AC-BLD-05 | AT infantry, tanks | — |
| AC-ARM-03 | Modular Infantry Support Vehicle | Vehicle | VEH-IFV | Ext | Modular IFV | 650 | VEH_M | 370 | 1,000 | 15 | AC-BLD-05 | Tanks, TDs | Helicopters, light vehicles |
| AC-ARM-04 | Concord Battle Tank | Vehicle | VEH-MBT | Core | MBT | 950 | VEH_H | 330 | 1,300 | 22 | AC-BLD-05 | IA-ARM-09, MP-ARM-07, RD-AIR-03 | IFVs, APCs, artillery |
| AC-ARM-05 | Heavy Breakthrough Tank | Vehicle | VEH-HEAVY | Ext | Heavy spearhead | 1,500 | VEH_H | 250 | 2,300 | 34 | AC-BLD-05 | TDs at range, strike jets | Medium and light vehicles |
| AC-ARM-06 | Anti-Tank Missile Carrier | Vehicle | VEH-TD | Core | Long-range ATGM | 540 | VEH_M | 360 | 1,250 | 18 | AC-BLD-05 | Infantry inside 300 cm, jamming | MBTs, heavy tanks |
| AC-ARM-07 | Concord Self-Propelled Gun | Vehicle | VEH-ARTY | Core | Howitzer | 480 | VEH_L | 290 | 1,500 | 22 | AC-BLD-05 | Raiders, helicopters, counter-battery | Massed infantry, defenses |
| AC-ARM-08 | Precision Missile Battery | Vehicle | VEH-ARTY | Ext | Guided rockets | 420 | VEH_L | 330 | 1,700 | 26 | AC-BLD-05 | Raiders, jamming | Artillery, emitters, structures |
| AC-ARM-09 | Skyguard Missile Carrier | Vehicle | VEH-AA | Core | Radar SAM | 540 | VEH_M | 330 | 1,200 | 17 | AC-BLD-05 | Any ground attacker | Helicopters, jets |
| AC-ARM-10 | Electronic Warfare Van | Vehicle | VEH-EW | Ext | Jamming | 380 | VEH_L | 380 | 950 | 14 | AC-BLD-05 | Direct attackers | Guided weapons, auras (soft) |
| AC-ARM-11 | Mobile Concealment Truck | Vehicle | VEH-EW | Ext (cap 3) | Concealment screen | 360 | VEH_L | 380 | 900 | 13 | AC-BLD-05 | Detectors, scouts | Spotting (soft) |
| AC-ARM-12 | Counter-Battery Radar Carrier | Vehicle | VEH-EW | Ext | Artillery location | 400 | VEH_L | 320 | 850 | 13 | AC-BLD-05 | Jamming, raiders | Artillery (soft) |
| AC-ARM-13 | Repair and Recovery Vehicle | Vehicle | VEH-LOGISTICS | Core | Repair, salvage, recovery | 750 | VEH_M | 310 | 950 | 15 | AC-BLD-05 | Raiders, artillery | — |
| AC-ARM-14 | Assault Engineer Breacher | Vehicle | VEH-ENGINEER | Ext (cap 3) | Breach, lanes, bridges | 950 | VEH_H | 280 | 1,400 | 20 | AC-BLD-05 | TDs, AT infantry | Bunkers, walls, mines |
| AC-ARM-15 | Prototype Siege Tank | Vehicle | VEH-HEAVY | Cond (cap 1) | Super-heavy siege | 2,600 | VEH_H | 175 | 4,200 | 55 | AC-BLD-05 | Strike aircraft, massed TDs | Ground units at cost |
| AC-SUP-01 | Swiftwing Recon Plane | Aircraft | AIR-SCOUT | Core | Unarmed opening scout | 180 | AIR_F | 1,600 | 220 | 6 | AC-BLD-06 | Any AA | — |
| AC-SUP-02 | Mobile Works Unit | Vehicle | VEH-CONSTRUCTOR | Core (cap 2) | Forward post constructor | 1,100 | VEH_M | 260 | 2,600 | 40 | AC-BLD-03 | Any attacker | — |
| AC-SUP-03 | Contract Construction Crew | Vehicle | VEH-CONSTRUCTOR | Ext | Builder | 420 | VEH_SOFT | 360 | 500 | 10 | AC-BLD-03 | Raiders | — |
| AC-SUP-04 | Petroleum Tanker | Vehicle | VEH-LOGISTICS | Ext | Fuel transport | 450 | VEH_SOFT | 360 | 450 | 9 | AC-BLD-03 | Raiders, aircraft | — |
| AC-SUP-05 | Airport Fuel Bowser | Vehicle | VEH-LOGISTICS | Ext | Kerosene, FARP | 380 | VEH_SOFT | 370 | 400 | 8 | AC-BLD-03 | Raiders, aircraft | — |
| AC-SUP-06 | Forward Logistics Convoy | Vehicle | VEH-LOGISTICS | Ext | Resupply | 560 | VEH_L | 350 | 700 | 12 | AC-BLD-03 | Raiders, artillery | — |
| AC-SUP-07 | Portable Listening Post | Vehicle | VEH-EW | Ext | Sensor node | 260 | VEH_SOFT | 480 | 500 | 9 | AC-BLD-05 | Any attacker, jamming | Concealed units (detects) |
| AC-AIR-01 | Interceptor Fighter | Aircraft | AIR-FIGHTER | Core | Air superiority | 380 | AIR_F | 2,500 | 1,900 | 24 | AC-BLD-06 | SAMs | Bombers, transports, helicopters |
| AC-AIR-02 | Ground-Attack Jet | Aircraft | AIR-STRIKE | Ext | Anti-armor strike | 600 | AIR_F | 1,700 | 2,000 | 26 | AC-BLD-06 | SAMs, AA guns, fighters | Armor columns, artillery |
| AC-AIR-03 | Precision Strike Fighter | Aircraft | AIR-STRIKE | Ext | Guided bombs | 420 | AIR_F | 2,300 | 2,600 | 30 | AC-BLD-06 | Long-range SAMs, fighters | Structures, emitters |
| AC-AIR-04 | Attack Helicopter | Aircraft | AIR-HELI-ATTACK | Core | Anti-armor CAS | 520 | AIR_R | 760 | 1,650 | 20 | AC-BLD-07 | AA guns, SAMs, MANPADS | Tanks, APCs, artillery |
| AC-AIR-05 | Utility Transport Helicopter | Aircraft | AIR-HELI-TRANSPORT | Ext | Air mobility, medevac | 600 | AIR_R | 700 | 1,200 | 16 | AC-BLD-07 | Any AA | — |
| AC-AIR-06 | Heavy Transport Aircraft | Aircraft | AIR-TRANSPORT | Ext (cap 2) | Paradrop, supply drop | 900 | AIR_F | 1,600 | 2,200 | 30 | AC-BLD-06 | Fighters, SAMs | — |
| AC-AIR-07 | Electronic Surveillance Aircraft | Aircraft | AIR-SURVEILLANCE | Ext (cap 1) | AEW, emitter detection | 700 | AIR_F | 1,500 | 2,600 | 32 | AC-BLD-06 | Fighters | Hidden air raids (detects) |
| AC-AIR-08 | Tactical Bomber | Aircraft | AIR-STRIKE | Ext (cap 3) | Bomb line | 1,000 | AIR_F | 1,500 | 2,800 | 34 | AC-BLD-06 | Fighters, SAM belts | Bases, fortifications |
| AC-AIR-09 | Armed Observation Helicopter | Aircraft | AIR-HELI-ATTACK | Ext | Scout and designator | 380 | AIR_R | 800 | 1,000 | 14 | AC-BLD-07 | Any AA | Scouts, concealed teams |
| AC-AIR-10 | Airborne Support Gunship | Aircraft | AIR-STRIKE (loiter) | Ext (cap 1) | Orbit fire support | 1,100 | AIR_F (low) | 1,200 | 2,800 | 34 | AC-BLD-06 | AA guns, SAMs, fighters | Infantry, light vehicles |
| AC-AIR-11 | Nuclear Delivery Bomber | Aircraft | AIR-STRIKE | Cond (cap 1) | Strategic delivery (E) | 1,200 | AIR_F | 1,700 | 3,200 | 40 | AC-BLD-06 + AC-DEF-13 | Fighters, missile defense | — |
| AC-SEA-01 | Port Security Launch | Naval | NAV-PATROL | Cond | Harbour patrol | 380 | NAV_L | 650 | 650 | 12 | AC-BLD-08 | Missile boats, helicopters | Swimmers, small boats |
| AC-SEA-02 | Amphibious Landing Carrier | Naval | NAV-TRANSPORT | Cond | Landing craft | 900 | NAV_L | 450 | 1,100 | 18 | AC-BLD-08 | Coastal guns, boats | — |
| AC-SEA-03 | Coastal Missile Boat | Naval | NAV-COMBATANT | Cond | Anti-ship strike | 520 | NAV_L | 720 | 1,500 | 20 | AC-BLD-08 | Helicopters, jets, subs | Ships, landing craft |
| AC-SEA-04 | Guided-Missile Frigate | Naval | NAV-COMBATANT | Cond (cap 3) | Fleet AA, ASW | 2,000 | NAV_H | 340 | 3,000 | 48 | AC-BLD-08 | Massed missile boats, subs | Aircraft over water, subs |
| AC-SEA-05 | Adriatic Attack Submarine | Naval | NAV-SUB | Cond (cap 2) | Sea denial | 950 | NAV_H | 340 | 2,100 | 40 | AC-BLD-08 | ASW frigates, patrol aircraft | Surface ships |
| AC-SEA-06 | Offshore Fire-Support Ship | Naval | NAV-COMBATANT | Cond (cap 2) | Shore bombardment | 2,400 | NAV_H | 300 | 3,200 | 52 | AC-BLD-08 | Subs, anti-ship missiles | Coastal defenses |
| AC-SEA-07 | Fleet Air-Control Vessel | Naval | NAV-TRANSPORT | Scenario | Naval aviation hub | 3,000 | NAV_H | 280 | scenario | — | scenario | Subs, anti-ship missiles | — |

## 9. Abilities and upgrades index

Unit-specific abilities and technologies only. Shared abilities (`ability.shared.*`) and shared technologies are defined in the framework.

| ID | Unit(s) | Kind | Cost | Cooldown | Range | Requires | Counters |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `ability.ac.inf_01.forward_observer` | AC-INF-01 | Upgrade passive | — | — | own sight | `tech.shared.tactical_radio_modernization` | Suppression, jamming |
| `ability.ac.inf_02.breach_and_clear` | AC-INF-02 | Active | — | 30 s | contact | — | Flame, grenades |
| `ability.ac.inf_02.checkpoint_inspection` | AC-INF-02 | Conditional passive | — | — | 300 cm | stationary, Guard | Routing around |
| `ability.ac.inf_03.switch_mode` | AC-INF-03 | Active | — | 4 s | — | — | Mixed air-ground attack |
| `ability.ac.inf_04.smoke_rounds` | AC-INF-04 | Active | — | 40 s | 760 cm | — | Detectors |
| `ability.ac.inf_05.heal` | AC-INF-05 | Automated | — | — | 300 cm | — | Kill the medic |
| `ability.ac.inf_05.casualty_replacement` | AC-INF-05 | Active | §10.3 USD | 20 s | 200 cm | 5 s without damage | Suppression |
| `ability.ac.inf_05.stabilise` | AC-INF-05 | Passive | — | — | 400 cm | — | Isolate the medic |
| `ability.ac.inf_07.airborne_qualified` | AC-INF-07 | Passive | — | — | — | AC-AIR-05/06 | Corridor AA |
| `ability.ac.inf_07.objective_seizure` | AC-INF-07 | Conditional passive | — | per insertion | self | air insertion | Landing-window fire |
| `ability.ac.inf_08.designate` | AC-INF-08 | Active | — | 30 s | 1600 cm | — | Smoke, jamming |
| `ability.ac.inf_09.shore_reconnaissance` | AC-INF-09 | Active | — | 45 s | 1200 cm | coastline | Coastal detectors |
| `ability.ac.inf_09.designate` | AC-INF-09 | Active | — | 30 s | 1500 cm | — | Smoke, jamming |
| `ability.ac.inf_10.detect_spy` | AC-INF-10 | Passive | — | — | 900 cm | — | Range |
| `ability.ac.inf_10.investigate` | AC-INF-10 | Active | — | 45 s | contact | own building | Kill the officer |
| `ability.ac.inf_11.infiltrate_intel` | AC-INF-11 | Active | $200 | 60 s | contact | — | `DETECT.SPY` |
| `ability.ac.inf_12.psyops_breakdown` | AC-INF-12 | Active | $250 | 75 s | 700 cm | eligible target | Rally, group support |
| `ability.ac.inf_12.psyops_rampage` | AC-INF-12 | Active | $350 | 90 s | 700 cm | target ≤ 34 resolve | Rally; kill the victim |
| `ability.ac.inf_12.psyops_facility` | AC-INF-12 | Active | $800 / $400 | 120 s | contact | eligible building | `DETECT.SPY`, Investigate |
| `ability.ac.inf_13.deploy_antenna` | AC-INF-13 | Active | — | — | self | — | Artillery |
| `ability.ac.inf_13.verify_report` | AC-INF-13 | Active | — | 30 s | footprint | deployed | Planted evidence (E) |
| AC-INF-14 talents (12 options) | AC-INF-14 | Talent | — | per option | 1200 cm radius | talent tier | Jamming, killing the commander |
| `ability.ac.arm_01.escort` | AC-ARM-01 | Order | — | — | — | — | AT ambush |
| `ability.ac.arm_02.medevac` | AC-ARM-02 | Passive | — | — | passengers | — | — |
| `ability.ac.arm_03.module_loadout` | AC-ARM-03 | Conditional | — | — | per module | boarded squad | Specialists outperform modules |
| `ability.ac.arm_05.breakthrough_drive` | AC-ARM-05 | Active | — | 60 s | self | — | Flank shots, air |
| `ability.ac.arm_07.smoke_mission` | AC-ARM-07 | Active | 3 ammo | 45 s | 900–3600 cm | — | Detectors |
| `ability.ac.arm_08.precision_strike` | AC-ARM-08 | Active | 4 ammo | 20 s | range | designation | Smoke, jamming |
| `ability.ac.arm_09.radar_emission_control` | AC-ARM-09 | Toggle | — | 3 s | self | — | Visual-only air |
| `ability.ac.arm_11.concealment_screen` | AC-ARM-11 | Deploy | — | — | 900 cm | — | Detectors |
| `ability.ac.arm_12.counter_battery_cue` | AC-ARM-12 | Automated | — | — | 4500 cm | deployed | Jamming |
| `ability.ac.arm_14.clear_lane` | AC-ARM-14 | Active | 1 charge | 30 s | 1200 cm | — | Covering fire |
| `ability.ac.arm_14.lay_bridge` | AC-ARM-14 | Active | — | — | contact | authored gap | Bridge destruction, jurisdiction |
| `ability.ac.arm_15.maintenance_due` | AC-ARM-15 | Conditional | — | — | — | 120 s firing | Kill the ARV |
| `ability.ac.sup_01.loiter_sortie` | AC-SUP-01 | Automated | 1 kerosene | 15 s | — | — | AA |
| `ability.ac.sup_02.deploy_forward_post` | AC-SUP-02 | Active | — | — | site | permitted site | Kill during deploy |
| `ability.ac.sup_05.forward_arming_point` | AC-SUP-05 | Active | cargo | — | 400 cm | parked 10 s | Artillery |
| `ability.ac.sup_07.sensor_node` | AC-SUP-07 | Deploy | — | — | 2600 cm | — | Slow movement, jamming |
| `ability.ac.air_03.stand_off_release` | AC-AIR-03 | Passive | — | — | 1200 cm | — | Long-range SAM |
| `ability.ac.air_04.nap_of_earth` | AC-AIR-04 | Active | — | 30 s | self | — | MGs |
| `ability.ac.air_05.casualty_evacuation` | AC-AIR-05 | Passive | — | — | passengers | — | — |
| `ability.ac.air_06.paradrop` | AC-AIR-06 | Active | kerosene | per sortie | — | airborne-qualified squads | Corridor AA |
| `ability.ac.air_06.supply_drop` | AC-AIR-06 | Active | $600 | per sortie | — | — | Crate destruction |
| `ability.ac.air_07.surveillance_orbit` | AC-AIR-07 | Active | 4 kerosene | 20 s | 6000 cm | — | Fighters |
| `ability.ac.air_08.carpet_line` | AC-AIR-08 | Active | ammo | — | 1600 cm line | — | SAM, fighters |
| `ability.ac.air_09.designate` | AC-AIR-09 | Active | — | 30 s | 1900 cm | — | AA, smoke |
| `ability.ac.air_10.orbit_fire` | AC-AIR-10 | Active | 5 kerosene | per sortie | 1200 cm | — | AA guns |
| `ability.ac.air_11.deliver_weapon` | AC-AIR-11 | Active | weapon | — | target | custody + nuclear gates (E) | Interceptors, missile defense |
| `ability.ac.sea_07.helicopter_deck` | AC-SEA-07 | Passive | kerosene | — | deck | — | Sinking |
| `ability.ac.sea_07.air_coordination` | AC-SEA-07 | Passive | — | — | 3000 cm | — | Jamming |
| `tech.ac.urban_security_procedures` | AC-INF-02 | Tech | $500 | — | — | AC-BLD-04 | — |
| `tech.ac.improved_grenade_munitions` | AC-INF-04 | Tech | $600 | — | — | AC-BLD-16 | — |
| `tech.ac.airborne_first_aid_kits` | AC-INF-07 | Tech | $500 | — | — | AC-BLD-04 | — |
| `tech.ac.field_fortification` | AC-INF-06 | Tech | $500 | — | — | AC-BLD-16 | — |
| `tech.ac.marksman_optics` | AC-INF-08 | Tech | $500 | — | — | AC-BLD-16 | — |
| `tech.ac.security_vetting` | AC-INF-10 | Tech | $600 | — | — | AC-BLD-16 | — |
| `tech.ac.late_espionage` | AC-INF-11 | Tech (E gate) | $1,200 | — | — | AC-BLD-16 | — |
| `tech.ac.tradecraft` | AC-INF-11 | Tech | $700 | — | — | AC-BLD-16 | — |
| `tech.ac.psychological_operations` | AC-INF-12 | Tech (unlock) | $1,500 | — | — | AC-BLD-18 | — |
| `tech.ac.broadcast_saturation` | AC-INF-12 | Tech | $900 | — | — | AC-BLD-18 | — |
| `tech.ac.precision_vehicle_targeting` | AC-ARM-03/04/05/06, AC-INF-03, AC-AIR-04 | Tech (E candidate) | $900 | — | — | AC-BLD-16 | — |
| `tech.ac.light_vehicle_armor_kits` | AC-ARM-01 | Tech | $500 | — | — | AC-BLD-05 | — |
| `tech.ac.add_on_armor` | AC-ARM-02 | Tech | $600 | — | — | AC-BLD-05 | — |
| `tech.ac.modular_mounts` | AC-ARM-03 | Tech | $600 | — | — | AC-BLD-16 | — |
| `tech.ac.composite_armor_package` | AC-ARM-04 | Tech | $800 | — | — | AC-BLD-16 | — |
| `tech.ac.heavy_armor_program` | AC-ARM-05 | Tech (unlock) | $1,500 | — | — | AC-BLD-16 | — |
| `tech.ac.improved_fire_control` | AC-ARM-07, AC-SEA-06 | Tech | $800 | — | — | AC-BLD-16 | — |
| `tech.ac.guided_munitions_program` | AC-ARM-08, AC-AIR-03 | Tech (unlock) | $1,200 | — | — | AC-BLD-16 | — |
| `tech.ac.recovery_doctrine` | AC-ARM-13 | Tech | $600 | — | — | AC-BLD-17 | — |
| `tech.ac.prototype_armor_program` | AC-ARM-15 | Tech (unlock) | $2,500 | — | — | AC-BLD-16, AC-BLD-17 | — |
| `tech.ac.defended_logistics` | AC-SUP-03/04/06 | Tech (research doc candidate) | $600 | — | — | AC-BLD-16 | — |
| `tech.ac.aviation_servicing` | all AC aircraft | Tech (research doc candidate) | $600 | — | — | AC-BLD-06 | — |
| `tech.ac.armored_airframe` | AC-AIR-02 | Tech | $700 | — | — | AC-BLD-06 | — |
| `tech.ac.naval_servicing` | AC-SEA-01..06 | Tech | $600 | — | — | AC-BLD-08 | — |

## 10. Faction tactical coverage, gaps and balance risks

**Coverage by role.** Scout: AC-SUP-01 (E), AC-AIR-09, AC-SUP-07. Line infantry: AC-INF-01, AC-INF-07. AT infantry: AC-INF-03. Area infantry: AC-INF-04. Support: AC-INF-05, AC-INF-06, AC-ARM-13, AC-SUP-06. Light vehicle: AC-ARM-01. Transport: AC-ARM-02, AC-AIR-05, AC-AIR-06, AC-SEA-02. IFV: AC-ARM-03. MBT/heavy: AC-ARM-04, AC-ARM-05, AC-ARM-15. TD: AC-ARM-06. Artillery: AC-ARM-07, AC-ARM-08, AC-SEA-06. AA: AC-ARM-09, AC-INF-03 (low only), AC-AIR-01. EW and sensors: AC-ARM-10/11/12, AC-INF-13, AC-AIR-07. Espionage: AC-INF-10/11/12. Commander: AC-INF-14. Air strike: AC-AIR-02/03/04/08/10. Naval: AC-SEA-01–06.

**Weak spots and intended answers.**

| Gap | Effect | Intended answer |
| --- | --- | --- |
| No flame, no mortar team, no AA gun | Garrison clearing and cheap anti-infantry AoE rely on AC-INF-04 and artillery | AC-INF-02 *Breach and Clear*, AC-INF-04, AC-ARM-07; AA guns are replaced by the AC-ARM-03 autocannon module |
| No light tank | Mid-weight gap between AC-ARM-01 and AC-ARM-04 | AC-ARM-03 and AC-AIR-04 fill the fast anti-light role |
| High prices everywhere | Loses even trades against Directorate mass | Preservation tools (AC-INF-05, AC-ARM-13), precision (AC-ARM-08) and Psy-Ops |
| Air dependence on kerosene and airfields | Kerosene starvation grounds sorties (§4.5) | AC-SUP-05, redundant AC-BLD-06/07, cheaper refining (E) |
| Communications dependence | Jamming cuts AC-INF-14 auras and degrades guided weapons | AC-INF-13 locates jammers; `tech.shared.electronic_countermeasures` |

**Balance risks.** (1) Designation stacking: AC-INF-08, AC-INF-09, AC-INF-14 and AC-AIR-09 all improve scatter; the framework clamp (§18.4) must take the single best scatter multiplier, not multiply them. (2) Psy-Ops elimination against expensive Pinned units could snowball; §7.5 eligibility excludes medium and heavy armor, and the 75 s cooldown and cap 2 limit throughput. (3) AC-ARM-03 modules can make one vehicle cover too many roles; 70 % module output keeps specialists better. (4) AC-SUP-01 must stay cheaper than any Directorate AA answer to it, or Concord opens blind.

**Open items.** Framework U-01 (squads), U-02 (taxonomy), U-04 (air sortie model), U-05/06 (food and fuel penalties), U-09 (resupply price), U-10 (refunds), U-11 (heavy recovery), U-14 is not Concord-relevant. Faction items:

| ID | Decision | Recommendation |
| --- | --- | --- |
| U-AC-01 | AC-INF-09 "raids" versus the spy-only charge rule (E) | Raids by designation, capture of coastal structures and small arms; no charges |
| U-AC-02 | Psy-Ops rampage activation threshold (E: open) | Pinned or lower (≤ 34 resolve) |
| U-AC-03 | AC-SUP-02 deploy target under the fixed-base prototype default | Deploy into AC-BLD-02 only; revisit when mobile or hybrid construction is approved |
| U-AC-04 | Custody of a completed weapon if AC-AIR-11 is destroyed before release | Owned by the nuclear contract; recommend the weapon is lost |
| U-AC-05 | Price of Concord spy charges relative to Meridian | $300 versus Meridian $120 (MP-INF-08); confirm the "more costly" ratio |
