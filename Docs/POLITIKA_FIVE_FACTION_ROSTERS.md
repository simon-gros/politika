# POLITIKA — Proposed Five-Faction Unit and Building Rosters

**Status:** Original design proposal / naming and unit-taxonomy reference; **not a statement that these units, structures, mechanics or statistics are implemented or approved as canon**.  
**Period:** alternate-history Yugoslavia, 1984–1989.  
**Compiled:** 27 October 2026.  
**Source corpus:** the user-provided *Command & Conquer Series — Faction Unit and Building Roster Reference* covering twelve entries; the live POLITIKA GitHub faction descriptions and system documents, consulted 27 October 2026.  
**Technical target:** an eventual UE 5.8.3 / C++ / C# / Rust RTS content-design inventory; no Python runtime dependency is proposed for the game.

## Design approach, scope and rules

This is **one new native conceptual roster for each of the five POLITIKA factions**, not twelve imported game rosters or a C&C mod. Every item has (a) a new **English** unit/structure name, (b) a plain gameplay purpose and (c) *original named Command & Conquer correlates* with their source release. Many source designs converge into one proposed POLITIKA archetype; some are represented only by function, not form. The lineages indicate genre/design inspiration **without authorizing copied visual models, trademarks, animations, dialogue, statistics or exact gameplay code**. Nothing here requires an exact one-to-one clone.

**Source key:** `TD` = *Command & Conquer / Tiberian Dawn*; `RA1` = *Red Alert*; `RA2` = *Red Alert 2*; `YR` = *Yuri's Revenge*; `TS` = *Tiberian Sun*; `Ren` = *Renegade*; `Gen` = *Generals*; `ZH` = *Generals: Zero Hour*; `TW` = *Tiberium Wars*; `KW` = *Kane's Wrath*; `RA3` = *Red Alert 3*; `Uprising` = *Red Alert 3: Uprising*; `TT` = *Tiberian Twilight*. A **source unit named in the wrong genre** may provide *only a function or silhouette analogy* (e.g., a TW walker informs a Cold War tracked artillery vehicle). Source game abbreviations distinguish identical recurring names.

**Available vs proposed:** the four externally playable forces are Atlantic Concord, Red Directorate, Iron Ascendancy, and Meridian Pact. **The Unaligned is always NPC/system AI**: its roster represents neutral federal, republican, municipal, civilian and Yugoslav People's Army/territorial-defense assets, largely *pre-placed or dynamically NPC-spawned*, **never a fifth player build menu**. The match has 2–4 player slots plus separate Unaligned authority. Concepts marked *conditional*, *scenario*, *contract*, *market*, or *prototype* are explicitly not ordinary starting assets. Exact production cost, balance, tier, health, ammunition, speed and unit cap are intentionally unspecified.

**Hard fictional-world constraints from the repository:**

1. **Opening scouts:** Atlantic Concord and Iron Ascendancy each have cheap **unarmed reconnaissance planes**; Red Directorate has rifle-armed **motorcycle scouts**; Meridian Pact has the best overall early reconnaissance through **very fast, inexpensive, lightly armed/armored trucks**. None of these observation units automatically performs espionage, and Meridian trucks do not automatically carry sabotage charges.
2. **Sabotage:** Meridian can use qualified **non-spy** troops for cheap early conventional attached-charge sabotage against eligible vehicles/buildings; the other three playable forces unlock **more costly spy-only** attached-charge sabotage later. Conventional sabotage is distinct from the special espionage systems below.
3. **Special intelligence:** Atlantic Concord alone has **Psy-Ops** against eligible unit Discipline/Morale and facility destruction/waste; Red Directorate alone has **slightly cheaper and faster basic intelligence theft** and budget-to-budget resource diversion; Iron Ascendancy alone has **Intimidation** of fully broken *isolated* eligible units and abstract fictional staff/facility contamination effects; Unaligned AI alone has dominant counterintelligence and **planted false intelligence** for sovereignty/stability. None of these is generic Yuri-style mind control or global omniscience.
4. **Economy and recruitment:** Concord has a **slightly lower cost of fossil-fuel refining**, not free crude; Directorate infantry recruitment is **far cheaper** than any other faction's; Ascendancy combat has **slightly greater raw destructive power** plus high-risk extreme-aggression behaviors; Meridian starts **near a fossil field** without automatic ownership and excels at contracting and **non-nuclear Black Market** commerce.
5. **Nuclear restrictions:** Concord and Directorate are established nuclear powers and can perform abstract nuclear processing, build **civil nuclear power** and very expensive long-production tactical bombs; Concord bomb delivery is by **nuclear bomber aircraft**, Directorate delivery by **ICBM**. Ascendancy is initially non-nuclear and can produce only its **first bomb** after completing its spy progression and successfully stealing *both* refining and weapon intel from a qualifying Concord/Directorate research facility, then fulfilling all resource/time gates; **completion immediately wins**, without a strike. Ascendancy cannot build nuclear power plants. Meridian cannot process nuclear materials or build nuclear weapons and the Black Market **never lists nuclear bombs**, although Meridian units may seize and deploy *stolen completed weapons* using compatible captured delivery capability. Unaligned is non-nuclear and never launches a player nuclear attack.
6. **Media:** radio, newspaper and television propaganda research belongs to Concord, Directorate and Ascendancy, plus **NPC-controlled Unaligned**. Meridian has Morale but **no advanced propaganda tree**. All factions have the same underlying resolve resource named **Discipline** for combat units and **Morale** for civilian/noncombat units.
7. **Period and tone:** armored tracked/wheeled vehicles, late-Cold-War tactical aviation, ferries, river crossings, the Adriatic, mountain passes, railway hubs, fuel terminals and 1980s electronics. Alien technology, true psychic domination, instant teleportation, nanocore fabrication, energy shields, giant walkers, fictional Tiberium and invisibility are **not** literally imported. An original unit may instead inspire a credible *gameplay role*, with high-tier prototypes remaining visually plausible alternate-history 1980s equipment.

**Categories:** row IDs (`AC-INF-01`, etc.) are stable *proposal identifiers* usable in a future data table, not executable asset names. Each row proposes **one distinct roster archetype**. Airstrike call-ins, deployed emplacements, maps/neutral structures and special deployment modes are described without pretending they are normally trainable separate units. Similar functional archetypes may have different faction names and technologies to preserve asymmetry.

## At-a-glance faction roles

| Faction | Primary battlefield doctrine | Economic/diplomatic identity | Unique cross-system constraint |
| --- | --- | --- | --- |
| Atlantic Concord | Elite combined arms; air coordination; precision targeting | Advanced multinational contractors; slightly cheaper fossil refining | Concord Psy-Ops; bomber-based nuclear delivery |
| Red Directorate | Mass infantry, heavy armor, artillery, railway logistics | State industry; major infantry recruitment discount | Intelligence theft efficiency and covert resource diversion; ICBM delivery |
| Iron Ascendancy | High-damage elite assault, militarized heavy prototypes, covert coercion | High infrastructure cost, repression, hostile federal standing | Intimidation and facility contamination; conditional first-bomb instant win; no civil nuclear power |
| Meridian Pact | Recon trucks, flexible mobile raiding, early sabotage | Merchant and mercenary broker, fossil-field proximity, Black Market | No nuclear manufacture/refining, no nuclear market sales, no advanced propaganda |
| The Unaligned (NPC) | Graduated police/JNA/territorial-defense response | Yugoslav socialist self-management, federal public infrastructure | Independent sovereignty, misinformation/counterintelligence AI, zero player control |

## Status/availability conventions

- `Native proposal` (default): candidate faction equipment or structure when technology, resources and map rules permit. **Not currently implemented**.
- `Conditional`: explicitly gated by story, research, intelligence, access, exceptional cost or late technology.
- `Market/contract`: the item must pass purchase/delivery/service contract constraints; its provenance is not replaced with faction-native manufacture.
- `Scenario / pre-placed`: an actor or facility normally controlled by the mission/map or the Unaligned AI, not a player build-tree slot.
- `Ability lineage`: the cited original is a support power, deployable state or campaign object; it is **not** claimed to be a directly produced C&C unit.

## Atlantic Concord

**Playable.** Elite Western-style combined arms, multi-domain operations, contractor-based logistics, air power, media strategy and distinct Psy-Ops. Advanced capabilities require specialized support and fuel rather than free precision advantages.

### Infantry, specialists, engineers and intelligence

| ID | POLITIKA unit / building (new English name) | Purpose / faction-specific conditions | Original C&C design correlates (name and release) |
| --- | --- | --- | --- |
| `AC-INF-01` | **Professional Rifle Section** | Disciplined general-purpose rifle squad; high training and operating cost. | GDI Minigunner (TD); Ranger (Gen); Rifleman Squad (TW) |
| `AC-INF-02` | **Urban Security Team** | Close-range shotguns, building clearing, checkpoint defense. | GDI Shotgun Trooper (Ren); Peacekeeper (RA3) |
| `AC-INF-03` | **Shoulder Missile Team** | Anti-armor and limited short-range anti-air infantry. | Rocket Soldier (TD); Missile Defender (Gen); Javelin Soldier (RA3) |
| `AC-INF-04` | **Grenadier Section** | Area suppression and assault of occupied structures. | Grenadier (TD); Grenadier Squad (TW) |
| `AC-INF-05` | **Field Medical Corps** | Restore squad casualties and resist attrition. | Medic (RA1); Field Medic (TS); Ambulance (Gen) |
| `AC-INF-06` | **Combat Engineering Team** | Repair or capture authorized facilities; clear obstacles. | Engineer (RA1); Engineer (TW); Hotwire (Ren) |
| `AC-INF-07` | **Airborne Assault Section** | Fast-deployed riflemen for limited heliborne or parachute operations. | Paratroopers (RA1); GDI Airborne (TW); Century Bomber (RA3, transport role) |
| `AC-INF-08` | **Long-Range Marksman Pair** | Stealthy observation and precision anti-infantry shots. | Sniper (RA2); Deadeye (Ren); Sniper Team (TW) |
| `AC-INF-09` | **Coastal Reconnaissance Detachment** | Amphibious commando for raids, ports and shoreline objectives. | Navy SEAL (RA2/YR); Tanya (RA3); Commando (TW) |
| `AC-INF-10` | **Counterintelligence Officer** | Identify infiltrators and investigate security incidents. | Attack Dog (RA1); Attack Dog (RA3, detection role); Detention Camp (Gen) |
| `AC-INF-11` | **Concord Field Agent** | Later-game spy; intelligence collection and *expensive, spy-only* attached-charge sabotage. | Spy (RA1); Spy (RA2); Shadow Team (TW) |
| `AC-INF-12` | **Psychological Operations Officer** | **Exclusive Concord Psy-Ops**: eligible resolve breakdown, bounded friendly-fire rampage or facility disruption; not mind control. | Yuri Clone (YR, role adapted); Mastermind (TW, role adapted); Spy (RA3) |
| `AC-INF-13` | **Signals Intercept Team** | Intercept transmissions and verify reports; vulnerable to planted Unaligned evidence. | Hacker (Gen); Mobile Sensor Array (TS); Spy Satellite (Gen) |
| `AC-INF-14` | **Special Operations Commander** | Unique high-cost infiltration/strike leader; limited availability. | Commando (TD); Havoc (Ren); Colonel Burton (Gen) |

### Combat vehicles and artillery

| ID | POLITIKA unit / building (new English name) | Purpose / faction-specific conditions | Original C&C design correlates (name and release) |
| --- | --- | --- | --- |
| `AC-ARM-01` | **Escort Jeep** | Light armed patrol and convoy protection; *not* the faction's canonical opening scout. | Humvee (TD); Ranger (RA1); Humvee (Gen) |
| `AC-ARM-02` | **Rapid Armored Carrier** | Protected infantry taxi with limited combat support. | APC (TD); Riptide ACV (RA3); APC (TW) |
| `AC-ARM-03` | **Modular Infantry Support Vehicle** | Weapon loadout changes with boarded squad or equipment package. | IFV (RA2); Multigunner IFV (RA3) |
| `AC-ARM-04` | **Concord Battle Tank** | Reliable precision-oriented main battle tank. | Medium Tank (TD); Grizzly Battle Tank (RA2); Predator Tank (TW) |
| `AC-ARM-05` | **Heavy Breakthrough Tank** | Rare, expensive two-weapon tracked armored spearhead. | Mammoth Tank (TD); Mammoth Tank (TW); FutureTank X-1 (Uprising, role adapted) |
| `AC-ARM-06` | **Anti-Tank Missile Carrier** | Long-range vehicle-killer with limited close protection. | Tank Destroyer (RA2); Guardian Tank (RA3); Javelin Soldier (RA3) |
| `AC-ARM-07` | **Concord Self-Propelled Gun** | Conventional mobile indirect artillery supporting combined arms. | Artillery (RA1); Juggernaut (TW, legged chassis adapted to tracked) |
| `AC-ARM-08` | **Precision Missile Battery** | Long-range guided rocket strikes; costly ammunition. | MLRS (TD); Tomahawk Missile Launcher (Gen); Specter (KW) |
| `AC-ARM-09` | **Skyguard Missile Carrier** | Radar-directed air-defense vehicle and escort. | Slingshot (KW); Avenger (ZH); Aegis Cruiser (RA2, function adapted) |
| `AC-ARM-10` | **Electronic Warfare Van** | Disrupt enemy targeting, local radar and communications. | Mobile Radar Jammer (RA1); Radar Van (Gen); Hydrofoil weapon-jamming mode (RA3) |
| `AC-ARM-11` | **Mobile Concealment Truck** | Limited field camouflage/communications screening, *not invisibility*. | Mobile Gap Generator (RA1); Mobile Stealth Generator (TS); Disruption Tower (TW) |
| `AC-ARM-12` | **Counter-Battery Radar Carrier** | Detect hostile artillery activity and cue fire missions. | Mobile Sensor Array (TS); Pitbull (TW); Recon Drone (TT) |
| `AC-ARM-13` | **Repair and Recovery Vehicle** | Field repairs, salvage and recovery. | Mobile Repair Vehicle (TS); Mobile Repair Transport (KW); Rig (TW) |
| `AC-ARM-14` | **Assault Engineer Breacher** | Protected, vehicle-borne engineering for bridge or strongpoint assault. | Combat Engineer (KW); Dozer (TT); Riptide ACV (RA3) |
| `AC-ARM-15` | **Prototype Siege Tank** | Exceptional single-vehicle heavy armor, limited slots and maintenance. | MARV (KW); Mastodon (TT); FutureTank X-1 (Uprising) |

### Reconnaissance, economy and construction vehicles

| ID | POLITIKA unit / building (new English name) | Purpose / faction-specific conditions | Original C&C design correlates (name and release) |
| --- | --- | --- | --- |
| `AC-SUP-01` | **Swiftwing Recon Plane** | **Canonical cheap opening scout:** small, unarmed fixed-wing aircraft; cannot fight. | Spy Plane (RA1, support-role adapted); Surveillance Sweep (RA3); Orca (TW, observation role) |
| `AC-SUP-02` | **Mobile Works Unit** | Deployable command/industrial construction vehicle. | MCV (TD); Prospector (RA3); Surveyor (TW) |
| `AC-SUP-03` | **Contract Construction Crew** | Establish forward sites, roads and production buildings. | Construction Dozer (Gen); Engineer (TD); Prospector Command Hub (RA3) |
| `AC-SUP-04` | **Petroleum Tanker** | Move crude oil/refined petroleum under convoy risk. | Supply Truck (Gen); Ore Truck (RA1); Harvester (TW) |
| `AC-SUP-05` | **Airport Fuel Bowser** | Carry separately refined kerosene to airfields. | Chinook supply role (Gen); Supply Truck (Gen); Supply Drop Zone (Gen) |
| `AC-SUP-06` | **Forward Logistics Convoy** | Deliver ammunition, spares and supplies. | Chinook (Gen); Supply Truck (Gen); Supply Drop (TT) |
| `AC-SUP-07` | **Portable Listening Post** | Deploy temporary local detection/recon data node. | Surveyor/Outpost (TW); Mobile Sensor Array (TS); Rig (TW) |

### Aircraft and air-support platforms

| ID | POLITIKA unit / building (new English name) | Purpose / faction-specific conditions | Original C&C design correlates (name and release) |
| --- | --- | --- | --- |
| `AC-AIR-01` | **Interceptor Fighter** | Air superiority and hostile bomber interception. | Apollo Fighter (RA3); Firehawk (TW); Raptor (Gen) |
| `AC-AIR-02` | **Ground-Attack Jet** | Conventional strike aircraft for armor and fortified targets. | Harrier (RA2); Vindicator (RA3); ORCA Assault Craft (Ren, mission analog) |
| `AC-AIR-03` | **Precision Strike Fighter** | High-tier guided-weapons fighter-bomber. | Stealth Fighter (Gen); Firehawk (TW); Black Eagle (RA2) |
| `AC-AIR-04` | **Attack Helicopter** | Low-altitude anti-armor close air support. | ORCA (TW); Longbow (RA1); Comanche (Gen) |
| `AC-AIR-05` | **Utility Transport Helicopter** | Move infantry/engineers and evacuate casualties. | Chinook (RA1); NightHawk (RA2); Transport Helicopter (Ren) |
| `AC-AIR-06` | **Heavy Transport Aircraft** | Drop troops and field supply payloads; not a bomber by default. | Century Bomber (RA3); V-35 Ox (TW); Chinook (Gen) |
| `AC-AIR-07` | **Electronic Surveillance Aircraft** | Expensive airborne early-warning and signal collection. | Spy Plane (RA1); Spy Satellite (Gen, function); Recon Drone (TT, function) |
| `AC-AIR-08` | **Tactical Bomber** | Large conventional payload at high cost and airfield footprint. | Aurora Bomber (Gen); Century Bomber (RA3); Harbinger Gunship (Uprising, payload role) |
| `AC-AIR-09` | **Armed Observation Helicopter** | Persistent low-volume patrol with ground attack option. | Hammerhead (KW); Orca (TT); Spanner (TT, support role) |
| `AC-AIR-10` | **Airborne Support Gunship** | Specialist suppressive fire while protecting deployments. | Spectre Gunship (Gen, call-in role); Harbinger Gunship (Uprising); Hammerhead (KW) |
| `AC-AIR-11` | **Nuclear Delivery Bomber** | **Concord-only completed tactical-bomb delivery vehicle**; requires all abstract nuclear gates. | Aurora Bomber (Gen, airframe archetype); Century Bomber (RA3); Vindicator (RA3) |

### Naval, riverine and amphibious assets

| ID | POLITIKA unit / building (new English name) | Purpose / faction-specific conditions | Original C&C design correlates (name and release) |
| --- | --- | --- | --- |
| `AC-SEA-01` | **Port Security Launch** | Patrol and anti-small-boat defense. | Gunboat (RA1); Gunboat (TD); Hydrofoil (RA3) |
| `AC-SEA-02` | **Amphibious Landing Carrier** | Move soldiers/vehicles to Adriatic shore access points. | Amphibious Transport (RA2); Riptide ACV (RA3); Hovercraft (TD) |
| `AC-SEA-03` | **Coastal Missile Boat** | Small fast antiship and coastal interdiction platform. | Destroyer (RA1, simplified); Sea Scorpion (RA2); Naginata Cruiser (RA3, role) |
| `AC-SEA-04` | **Guided-Missile Frigate** | Fleet air defense and convoy escort; costly fuel. | Aegis Cruiser (RA2); Destroyer (RA1); Hydrofoil (RA3) |
| `AC-SEA-05` | **Adriatic Attack Submarine** | Covert sea-area denial and shipping pressure. | Submarine (RA1); Typhoon Attack Submarine (RA2); Akula Submarine (RA3) |
| `AC-SEA-06` | **Offshore Fire-Support Ship** | Coastal bombardment in suitable deep water, map limited. | Cruiser (RA1); Assault Destroyer (RA3); Shogun Battleship (RA3, function) |
| `AC-SEA-07` | **Fleet Air-Control Vessel** | Scenario-tier ship coordinating naval aviation rather than ubiquitous carrier production. | Aircraft Carrier (RA2); Aircraft Carrier (RA3); Planetary Assault Carrier (TW, deployment function) |

### Production, economic and command buildings

| ID | POLITIKA unit / building (new English name) | Purpose / faction-specific conditions | Original C&C design correlates (name and release) |
| --- | --- | --- | --- |
| `AC-BLD-01` | **Joint Operations Headquarters** | Main build authority, deployment permissions and command. | Construction Yard (TD); Command Center (Gen); Construction Yard (RA3) |
| `AC-BLD-02` | **Portable Forward Command Post** | Limited build-radius and logistics extension. | Surveyor Outpost (TW); Prospector Command Hub (RA3); Rig Battle Base (TW) |
| `AC-BLD-03` | **Contractor Works Depot** | Creates construction crews and mobile works units. | Weapons Factory (TD); Construction Dozer (Gen, builder-function) |
| `AC-BLD-04` | **Personnel Training Center** | Produces soldiers, engineers and medics. | Barracks (TD); Boot Camp (RA3); Barracks (Gen) |
| `AC-BLD-05` | **Armored Vehicle Works** | Tanks, IFVs, rocket artillery, repair carriers. | War Factory (RA1); Weapons Factory (Ren); Armor Facility (RA3) |
| `AC-BLD-06` | **Military Airbase** | Fighters, fixed-wing scouts, strike bombers. | Airfield (Gen); Airbase (RA3); Airfield (TW) |
| `AC-BLD-07` | **Rotorcraft Pad** | Helicopter rearmament, recovery and transport. | Helipad (TD); Helipad (TS); Airfield (RA3) |
| `AC-BLD-08` | **Adriatic Naval Station** | Naval assembly, port access and maintenance. | Naval Yard (RA1); Seaport (RA3); Naval Shipyard (RA2) |
| `AC-BLD-09` | **Oil Processing Refinery** | Convert crude oil into petroleum fuel and kerosene at **Concord's lower refining cost**. | Ore Refinery (RA1, role translated); Supply Center (Gen); Tiberium Refinery (TW, role translated) |
| `AC-BLD-10` | **Natural Gas Terminal** | Extraction/transfer point for gas; jurisdiction and ownership checks. | Ore Refinery (RA2, economy analogue); Supply Center (Gen); Growth Accelerator (TW, output role) |
| `AC-BLD-11` | **Commercial Supply Depot** | Store and dispatch fuel/industrial goods; no automatic infinite income. | Supply Center (Gen); Tiberium Silo (TD); Supply Drop Zone (Gen) |
| `AC-BLD-12` | **Grid Power Station** | Conventional electricity with fuel/logistics costs. | Power Plant (TD); Cold Fusion Reactor (Gen, electrical role, not fictional reactor physics); Power Plant (RA3) |
| `AC-BLD-13` | **Nuclear Materials Laboratory** | Permitted nuclear intelligence validation and abstract processing research. | Tech Center (TW); Battle Lab (RA2); Detention Camp (Gen, intelligence role) |
| `AC-BLD-14` | **Strategic Material Processing Site** | Transform abstract crude nuclear-resource tokens into processed inputs. | Tech Center (TS); Battle Lab (RA2); Nuclear Reactor (RA2, tech prerequisite inspiration) |
| `AC-BLD-15` | **Civilian Nuclear Power Plant** | **Concord-legal only after processed resource and grid prerequisites.** | Nuclear Reactor (RA2); Super Reactor (RA3); Nuclear Reactor (Gen China, power analogue) |
| `AC-BLD-16` | **Combined Arms Doctrine Center** | Unlock communications, precision-guidance and formation upgrades. | Strategy Center (Gen); Command Post (TW); Defense Bureau (RA3) |
| `AC-BLD-17` | **Maintenance and Repair Workshop** | Vehicle recovery, upgrade and repairs. | Service Depot (RA1); Repair Facility (TD); Armory (TW) |
| `AC-BLD-18` | **Media Coordination Bureau** | Faction-approved radio, print and television campaign planning. | Propaganda Center (Gen); Speaker Tower (Gen, influence analogue); Communications Center (TD) |

### Defenses, intelligence facilities and strategic installations

| ID | POLITIKA unit / building (new English name) | Purpose / faction-specific conditions | Original C&C design correlates (name and release) |
| --- | --- | --- | --- |
| `AC-DEF-01` | **Checkpoint Bunker** | Cheap line-of-sight roadblock and garrison defense. | Pillbox (RA1); Guardian Cannon (TW); Bunker (TT) |
| `AC-DEF-02` | **Multirole Gun Emplacement** | Modular anti-infantry / anti-vehicle defensive turret. | Multigunner Turret (RA3); Component Tower (TS); Advanced Guard Tower (TD) |
| `AC-DEF-03` | **Anti-Air Missile Battery** | Static SAM site with radar dependency. | Patriot Missile System (RA2); SAM Site (TD); AA Battery (TW) |
| `AC-DEF-04` | **Heavy Anti-Armor Position** | Fortified long-range anti-tank defense. | Grand Cannon (RA2); Guardian Cannon (TW); Focus Beam (TT, anti-armor role) |
| `AC-DEF-05` | **Electronic Observation Mast** | Local sensor reveal and counterintelligence support. | Radar Dome (RA1); Radar Installation (TS); Space Command Uplink (TW) |
| `AC-DEF-06` | **Radar Interference Station** | Interrupt radar acquisition; no magic fog erasure. | Gap Generator (RA1); Disruption Tower (TW); Detention Camp (Gen, information role) |
| `AC-DEF-07` | **Protected Fuel Reservoir** | Hardened refinery output storage and supply hub. | Ore Silo (RA1); Tiberium Silo (TW); Supply Center (Gen) |
| `AC-DEF-08` | **Base Perimeter and Gate** | Walls, barriers and controlled road entry. | Concrete Wall (TD); Fortress Wall (RA3); Gate (TS) |
| `AC-DEF-09` | **Press and Radio House** | Propaganda coverage: newspapers + radio; vulnerable to competing media. | Communications Center (TD); Propaganda Center (Gen); Speaker Tower (Gen) |
| `AC-DEF-10` | **Television Relay Station** | High-tier morale communication, not mind control. | Communications Center (TD); SpySat Uplink (RA2, coverage role) |
| `AC-DEF-11` | **Long-Range Missile Defense Site** | Expensive strategic intercept/anti-air installation. | Patriot Missile System (Gen); Advanced Guard Tower (Ren); Skystrike Artillery (TT, emplacement role) |
| `AC-DEF-12` | **Tactical Weapon Assembly Vault** | Very long **Concord nuclear bomb production**; completion alone never wins. | Particle Cannon (Gen, facility-cost archetype); Nuclear Missile Silo (RA2, strategic site analogue) |
| `AC-DEF-13` | **Strategic Air Command Hangar** | Secure sortie authorization for completed bomb by nuclear delivery bomber. | Air Force Command Headquarters (RA2); Airbase (RA3); Air Field (Gen) |

## Red Directorate

**Playable.** Soviet-inspired state-industrial mass doctrine; far cheaper infantry, large mechanized armies, deep artillery and centralized infrastructure. Intelligence theft/diversion is its spy specialty; ICBM is its nuclear delivery method.

### Infantry, personnel and intelligence

| ID | POLITIKA unit / building (new English name) | Purpose / faction-specific conditions | Original C&C design correlates (name and release) |
| --- | --- | --- | --- |
| `RD-INF-01` | **Directorate Rifle Platoon** | Extremely cheap mass-produced line infantry; doctrine rewards formations. | Conscript (RA2); Red Guard (Gen); Militant Squad (TW) |
| `RD-INF-02` | **Motor-Rifle Section** | Infantry with organic transport discipline and anti-armor teamwork. | Rifle Infantry (RA1); Rifleman Squad (TW); Red Guard (Gen) |
| `RD-INF-03` | **Heavy Machine-Gun Team** | Suppression and checkpoint overwatch. | GDI Officer (Ren); Zone Defender (TT, squad role); Nod Officer (Ren) |
| `RD-INF-04` | **Portable Rocket Section** | Anti-vehicle and limited AA at low infantry cost. | Rocket Infantry (TS); Tank Hunter (Gen); Flak Trooper (RA3) |
| `RD-INF-05` | **Mortar Infantry Team** | Organic portable indirect fire for urban or mountain terrain. | Grenadier (RA1); Mortar Cycle (Uprising, indirect-fire concept); Disc Thrower (TS, area role) |
| `RD-INF-06` | **Combat Pioneer** | Repair, bridge improvement, capture authorized facilities. | Engineer (RA1); Combat Engineer (RA3); Engineer (TT) |
| `RD-INF-07` | **Rail Security Troop** | Low-cost guard formation protecting trains and depots. | Militant Rocket Squad (TW); Red Guard (Gen); GDI Soldier (Ren) |
| `RD-INF-08` | **Chemical Defense Detachment** | Protective gear, decontamination and contamination counterplay; no biological weapon. | Desolator (RA2, protection role only); Zone Trooper (TW, hazardous-environment protection) |
| `RD-INF-09` | **Special Recon Riflemen** | Veteran ground observation, flanking and ambush suppression. | Sniper Team (TW); Pathfinder (Gen); Shadow Team (TW, recon role) |
| `RD-INF-10` | **Political Liaison Officer** | Supports friendly unit Discipline; does not control enemy minds. | Confessor Cabal (KW, morale role); Speaker Tower (Gen); Propaganda Tower (Gen, upgrade role) |
| `RD-INF-11` | **Directorate Intelligence Courier** | **Directorate cheap/faster intelligence theft**, infiltrates eligible enemy sites. | Spy (RA1); Black Lotus (Gen); Saboteur (TW) |
| `RD-INF-12` | **Resource Diversion Agent** | Covert transfer of permitted resources from victim account into Directorate budget. | Thief (RA1); Hacker (Gen); Black Lotus (Gen) |
| `RD-INF-13` | **Special Forces Colonel** | Limited elite raid and target designation commander. | Boris (YR); Commando (TD); Natasha (RA3) |

### Armored, mechanized and artillery forces

| ID | POLITIKA unit / building (new English name) | Purpose / faction-specific conditions | Original C&C design correlates (name and release) |
| --- | --- | --- | --- |
| `RD-ARM-01` | **Patrol Motorcycle** | **Canonical opening scout** with basic-rifle fire and little armor. | Recon Bike (TD); Attack Cycle (TS); Attack Bike (KW); Mortar Cycle (Uprising, chassis only) |
| `RD-ARM-02` | **Motorized Infantry Truck** | Cheap troop delivery and logistics support. | Flak Track (RA2); Technical (Gen); Troop Crawler (Gen, role) |
| `RD-ARM-03` | **Tracked Personnel Carrier** | Heavy infantry transport for mass formations. | APC (TD); Armadillo (TT); Amphibious APC (TS, role) |
| `RD-ARM-04` | **People's Battle Tank** | Affordable mass battle tank; supports broad armor fronts. | Heavy Tank (RA1); Rhino Heavy Tank (RA2); Battlemaster Tank (Gen) |
| `RD-ARM-05` | **Improved Heavy Battle Tank** | Late conventional assault armor, durable but fuel hungry. | Apocalypse Tank (RA2); Overlord Tank (Gen); Mammoth Tank (TD) |
| `RD-ARM-06` | **Mobile Anti-Air Gun** | Tracked anti-air cannon supporting columns. | Gattling Tank (Gen); Flak Track (RA2); Slingshot (KW) |
| `RD-ARM-07` | **Rocket Artillery Regiment** | Wide-area unguided barrage from mobile launchers. | V2 Rocket Launcher (RA1); V3 Rocket Launcher (RA2); V4 Rocket Launcher (RA3) |
| `RD-ARM-08` | **Heavy Self-Propelled Gun** | Long-range counter-battery and siege pressure. | Artillery (TS); Nuke Cannon (Gen, conventional artillery role only); Juggernaut (TW) |
| `RD-ARM-09` | **Mortar Carrier** | Mobile, short-range indirect fire supporting infantry. | Mortar Cycle (Uprising); Inferno Cannon (Gen, saturation role) |
| `RD-ARM-10` | **Flame Assault Vehicle** | Costly close-range fortified-position attack; abstract game weapon. | Flame Tank (TD); Dragon Tank (Gen); Devil's Tongue (TS) |
| `RD-ARM-11` | **Mine-Clearing Tank** | Breach obstacle lines and secure armored advance. | Grinder (Uprising, obstacle/crush role); Dozer (TT); Terror Drone (RA2, clearance need) |
| `RD-ARM-12` | **Defensive Entrenchment Tank** | Can dig in for defense; loses flexibility while deployed. | Tick Tank (TS); Spartan Tank (TT) |
| `RD-ARM-13` | **Field Anti-Air Missile Carrier** | Long-range escort air defense requiring radar support. | SAM Site (TD, role vehicleized); Mantis (KW); Reaper (Uprising, anti-air role) |
| `RD-ARM-14` | **Armored Siege Command Tank** | Rare heavyweight centerpiece with infantry support sockets. | MARV (KW); Behemoth (KW); Mastodon (TT) |
| `RD-ARM-15` | **Veteran Reserve Tank** | Older but inexpensive refurbished armored hull. | Light Tank (TD); Grizzly Battle Tank (RA2); Scorpion Tank (TW) |

### Construction, supply and field support

| ID | POLITIKA unit / building (new English name) | Purpose / faction-specific conditions | Original C&C design correlates (name and release) |
| --- | --- | --- | --- |
| `RD-SUP-01` | **Industrial Constructor** | Mobile deployment of construction and heavy production works. | MCV (TD); Construction Dozer (Gen); Sputnik (RA3) |
| `RD-SUP-02` | **Rail Supply Tractor** | Carry finished goods between rail and forward depots. | Supply Truck (Gen); Ore Truck (RA1); Heavy Harvester (KW) |
| `RD-SUP-03` | **Bulk Fuel Transport** | Sustain mechanized divisions with refined fuel. | Supply Truck (Gen); Harvester (TW); Ore Collector (RA3) |
| `RD-SUP-04` | **Mobile Repair Crew** | Repair heavy armor and rail-connected structures. | Mobile Repair Vehicle (TS); MRT (KW); Spanner (TT) |
| `RD-SUP-05` | **Engineer Bridging Vehicle** | Temporary crossings and mountain pass restoration. | Amphibious APC (TS); Rig (TW); Combat Engineer (KW) |
| `RD-SUP-06` | **Electronic Countermeasure Vehicle** | Limited battlefield interference and anti-air alerting. | Mobile Radar Jammer (RA1); Mobile Sensor Array (TS); Conductor (TT) |
| `RD-SUP-07` | **Reserve Reinforcement Carrier** | Deploy infantry from mobilization pool with logistical limits. | Troop Crawler (Gen); Reckoner (KW); Bullfrog (RA3, delivery role) |

### Aircraft and airborne support

| ID | POLITIKA unit / building (new English name) | Purpose / faction-specific conditions | Original C&C design correlates (name and release) |
| --- | --- | --- | --- |
| `RD-AIR-01` | **Frontline Interceptor** | Conventional multirole air-to-air fighter. | MiG (RA1); MiG (Gen); MiG Fighter (RA3) |
| `RD-AIR-02` | **Tactical Attack Aircraft** | Air-to-ground strike in support of tank spearheads. | Yak (RA1); Vindicator (RA3); Raptor (Gen, strike role) |
| `RD-AIR-03` | **Close-Support Gunship** | Ground attack helicopter with short patrol duration. | Hind (RA1); Twinblade (RA3); Apache (Ren, role) |
| `RD-AIR-04` | **Assault Transport Helicopter** | Transport airborne assault detachments and supplies. | Chinook (RA1); Twinblade (RA3); NightHawk (RA2) |
| `RD-AIR-05` | **Heavy Transport Aircraft** | Paratrooper and logistics flights; no independent strategic-bomber unlock. | Paratroopers (RA1, delivery role); Century Bomber (RA3, transport role); V-35 Ox (TW) |
| `RD-AIR-06` | **Radar Reconnaissance Aircraft** | High-altitude observation and incoming raid detection. | Spy Plane (RA1); Surveillance Sweep (RA3); Eyes of the People (TT, information role) |
| `RD-AIR-07` | **Frontline Bomber** | Conventional carpet/area bombing support, limited precision. | Badger Bomber (RA1, support); Carpet Bomber (Gen); Kirov Airship (RA2, heavy bombing role) |
| `RD-AIR-08` | **Artillery Targeting Helicopter** | Spot and correct rocket/artillery fire. | Orca (TW, observation); Harpy (TS, armed recon); Hammerhead (KW) |
| `RD-AIR-09` | **Strategic Missile Transporter** | Oversized missile-erector convoy, *not a nuclear-producing unit*. | V4 Rocket Launcher (RA3); V3 Rocket Launcher (RA2); Nuke Cannon (Gen, chassis role) |

### Naval, riverine and amphibious assets

| ID | POLITIKA unit / building (new English name) | Purpose / faction-specific conditions | Original C&C design correlates (name and release) |
| --- | --- | --- | --- |
| `RD-SEA-01` | **River Patrol Boat** | Protect ferry crossings and supply bridges. | Gunboat (RA1); Stingray (RA3, patrol role) |
| `RD-SEA-02` | **Amphibious Motor-Rifle Carrier** | Move soldiers through shallow river crossings. | Amphibious Transport (RA2); Bullfrog (RA3); Amphibious APC (TS) |
| `RD-SEA-03` | **Coastal Torpedo Boat** | Interdict port access and nearshore convoys. | Sea Scorpion (RA2, escort role); Yari Mini-Sub (RA3, light marine concept) |
| `RD-SEA-04` | **Conventional Attack Submarine** | Silent offshore interdiction, deep-water restricted. | Submarine (RA1); Typhoon Attack Submarine (RA2); Akula Submarine (RA3) |
| `RD-SEA-05` | **Anti-Air Escort Destroyer** | AA and fleet protection along the Adriatic. | Destroyer (RA1); Aegis Cruiser (RA2); Dreadnought (RA3, hull role) |
| `RD-SEA-06` | **Shore Bombardment Cruiser** | Slow large-caliber coastal siege vessel. | Cruiser (RA1); Dreadnought (RA2); Dreadnought (RA3) |
| `RD-SEA-07` | **Amphibious Headquarters Vessel** | Floating logistics hub for limited coastal missions. | Transport Ship (RA1); Aircraft Carrier (RA2, command function); Kodiak (TT, mobile support function) |

### Industrial and civic-military buildings

| ID | POLITIKA unit / building (new English name) | Purpose / faction-specific conditions | Original C&C design correlates (name and release) |
| --- | --- | --- | --- |
| `RD-BLD-01` | **Central Operational Headquarters** | Directorate construction and command authority. | Construction Yard (RA1); Command Center (Gen); Defense Crawler (TT, deployment function) |
| `RD-BLD-02` | **Five-Year Planning Office** | Tech/policy control over mobilization and production. | Propaganda Center (Gen); Strategy Center (Gen); Operations Center (TW) |
| `RD-BLD-03` | **Conscript Muster Barracks** | Fast low-cost infantry training. | Barracks (RA2); Barracks (Gen); Barracks (RA3) |
| `RD-BLD-04` | **Military Engineering Works** | Train pioneers and construct bridging crews. | Service Depot (RA1); Combat Engineer bunker (RA3); Armory (TW) |
| `RD-BLD-05` | **State Armor Combine** | Main tank, artillery and APC factory. | War Factory (RA2); War Factory (Gen); War Factory (RA3) |
| `RD-BLD-06` | **Heavy Tank Assembly Hall** | Advanced mass armor; larger power/fuel draw. | War Factory (TW); Reclamator Hub (KW); War Factory (Gen) |
| `RD-BLD-07` | **Tactical Air Regiment Field** | Fighters, helicopters and attack craft. | Airfield (RA1); Airfield (Gen); Airfield (RA3) |
| `RD-BLD-08` | **Naval Construction Basin** | Build river/coastal fleets and amphibious transport. | Submarine Pen (RA1); Naval Shipyard (RA2); Naval Yard (RA3) |
| `RD-BLD-09` | **Crude Oil Refinery** | Crude -> finished petroleum or kerosene; **no Concord discount**. | Ore Refinery (RA1, economy); Supply Center (Gen); Refinery (TW, economy) |
| `RD-BLD-10` | **Gas Treatment Station** | Gas extraction, treatment, storage and grid delivery. | Ore Refinery (RA2, resource role); Supply Center (Gen); Extractor (TW, economic analogue) |
| `RD-BLD-11` | **Railway Marshalling Depot** | Strategic logistics routing, supply traffic and troop movement. | Supply Center (Gen); Supply Drop Zone (Gen); Tiberium Silo (TW, logistics role) |
| `RD-BLD-12` | **Conventional Thermal Station** | Bulk electricity; vulnerable fuel/transmission lines. | Power Plant (RA1); Nuclear Reactor (Gen, **electrical-output role only**, not nuclear permissions); Reactor (RA3) |
| `RD-BLD-13` | **State Nuclear Research Bureau** | Legal nuclear intelligence validation, research and secure storage. | Battle Lab (RA2); Tech Center (TW); Nuclear Missile Silo (Gen, tech lineage) |
| `RD-BLD-14` | **Nuclear Material Processing Complex** | Convert abstract nuclear-material tokens after research gate. | Battle Lab (RA2); Nuclear Reactor (RA2, resource prerequisite inspiration) |
| `RD-BLD-15` | **Civil Nuclear Generating Station** | **Directorate-legal** nuclear grid power after required processing. | Nuclear Reactor (RA2); Super Reactor (RA3); Nuclear Reactor (Gen) |
| `RD-BLD-16` | **National Radio Administration** | Radio/newspaper/television propaganda and public messaging tech. | Propaganda Center (Gen); Speaker Tower (Gen); Communications Center (TD) |
| `RD-BLD-17` | **Army Maintenance Workshops** | Repairs, veterancy support and replacement parts. | Service Depot (RA1); Service Depot (TS); Armory (TW) |

### Defenses, strategic buildings and command installations

| ID | POLITIKA unit / building (new English name) | Purpose / faction-specific conditions | Original C&C design correlates (name and release) |
| --- | --- | --- | --- |
| `RD-DEF-01` | **Infantry Fighting Bunker** | Cheap entrenchment supporting mass infantry. | Bunker (Gen); Battle Bunker (YR); Bunker (TT) |
| `RD-DEF-02` | **Guard Machine-Gun Post** | Basic anti-personnel line defense. | Sentry Gun (RA2); Sentry Gun (RA3); Watchtower (TW) |
| `RD-DEF-03` | **Flak Battery** | Local anti-air cannon coverage. | Flak Cannon (RA2); Flak Cannon (RA3); AA Battery (TW) |
| `RD-DEF-04` | **SAM Regiment Site** | Mid/long-range static missile air defense. | SAM Site (RA1); SAM Site (TD); Plasma Missile Battery (TW, role only) |
| `RD-DEF-05` | **Heavy Anti-Tank Casemate** | Powerful direct-fire bunker against armor. | Gun Turret (TD); Tesla Coil (RA1, anti-armor role without electricity beam); Guardian Cannon (TW) |
| `RD-DEF-06` | **Fire Coordination Tower** | Improves effectiveness of mass fire coordination. | Radar Dome (RA1); Radar Tower (RA2); Radar Installation (TS) |
| `RD-DEF-07` | **Deep Radar Listening Site** | Fog-of-war and aircraft early-warning detection. | Communications Center (TD); Radar Tower (RA2); Radar Installation (TS) |
| `RD-DEF-08` | **Political Radio Transmitter** | State propaganda reach and military Discipline bonus where eligible. | Speaker Tower (Gen); Voice of Kane (KW, morale role); Propaganda Center (Gen) |
| `RD-DEF-09` | **State Printing House** | Newspaper capacity, dependent on paper/delivery. | Communications Center (TD); Propaganda Center (Gen, media function) |
| `RD-DEF-10` | **Television Broadcast Center** | National-level propaganda media coverage, subject to censorship/credibility. | Propaganda Center (Gen, medium function); Radar Tower (RA2, broadcast structure analogue) |
| `RD-DEF-11` | **Protected Ammunition Warehouse** | Stores supplies; explosion risk abstracted. | Ore Silo (RA1); Tiberium Silo (TD); Supply Center (Gen) |
| `RD-DEF-12` | **Minefield Control Office** | Deploy minefields and preserve corridor maps. | Minelayer (RA1, emplacement role); Cluster Mines (Gen); Demo Trap (Gen) |
| `RD-DEF-13` | **Fortified Perimeter Section** | Fences, gates, blast walls and access controls. | Concrete Wall (RA1); Fortress Wall (RA3); Gate (TS) |
| `RD-DEF-14` | **Long-Range Missile Command** | **ICBM delivery control** for completed tactical nuclear bomb; no completion win. | Missile Silo (RA1); Nuclear Missile Silo (Gen); Temple of Nod (TW, delivery pattern) |
| `RD-DEF-15` | **Secure Tactical Warhead Depot** | Long-duration nuclear production and guarded storage. | Nuclear Missile Silo (RA2); Nuclear Missile Silo (Gen); Temple of Nod (TD, strategic-site role) |

## Iron Ascendancy

**Playable.** Fictional fascist totalitarian military machine, with slightly stronger attacks, costly advanced armor and dangerous political overextension. It is the narrative aggressor and a hostile actor to federal authorities; coercion carries consequences, not rewards without cost.

### Infantry, security and clandestine personnel

| ID | POLITIKA unit / building (new English name) | Purpose / faction-specific conditions | Original C&C design correlates (name and release) |
| --- | --- | --- | --- |
| `IA-INF-01` | **Garrison Riflemen** | Conventional infantry; slightly higher attack, harsh attrition risks. | Nod Minigunner (TD); Nod Militant (TW); Devout (TT) |
| `IA-INF-02` | **Shock Assault Section** | Close assault specialists and aggressive breach behavior. | Black Hand (KW); Confessor Cabal (KW); Nod Shotgun Trooper (Ren) |
| `IA-INF-03` | **Heavy Grenadier Squad** | Close-quarters explosives and area suppression. | Grenadier (TD); Fanatics (TW, risk behavior); Disc Thrower (TS, area role) |
| `IA-INF-04` | **Portable Anti-Armor Cell** | Guided rockets for ambush against vehicles. | RPG Trooper (Gen); Militant Rocket Squad (TW); Tankbuster (RA3) |
| `IA-INF-05` | **Urban Flame Detachment** | High-damage short-range assault; fuel and exposure risk. | Flamethrower (Ren); Black Hand (TW); Flamethrower Infantry (TD) |
| `IA-INF-06` | **Frontier Combat Medic** | Sustains elite groups; medical service does not provide immunity. | Field Medic (TS); Engineer (TT, repair role) |
| `IA-INF-07` | **Technical Demolition Engineer** | Repair, wire-cutting and obstacle work; **cannot** use early conventional sabotage charges. | Combat Engineer (RA3); Saboteur (TW, engineer role); Technician (Ren) |
| `IA-INF-08` | **Covert Reconnaissance Team** | Observe traffic routes, disguise movement and report locations. | Shadow Team (TW); Shinobi (RA3); Chameleon Spy (TS, campaign) |
| `IA-INF-09` | **Ascendancy Intelligence Agent** | Limited starting spy tree; later espionage upgrades and **spy-only expensive explosive sabotage**. | Spy (RA2); Black Lotus (Gen); Shadow Team (TW) |
| `IA-INF-10` | **Facility Contamination Operative** | Exclusive abstract poisoning/disease *status* on eligible staffed buildings; bounded disruption/recovery. | Chemical Warrior (TD, fictional effect only); Toxin Tractor (Gen, area-denial analogue); Desolator Trooper (Uprising, role adapted) |
| `IA-INF-11` | **Intimidation Officer** | Exclusive coercion of *isolated* enemies only after total Discipline/Morale break; no generic mind control. | Yuri Clone (YR, interaction analogue); Cultists (KW, interaction analogue); High Confessor (TT, morale role) |
| `IA-INF-12` | **Political Security Enforcer** | Detain, police and enforce harsh zone control; attracts Unaligned hostility. | Confessor Cabal (KW); Black Hand (KW); Reaper (TT, role) |
| `IA-INF-13` | **Special Operations Raider** | Limited elite assault/target marking; cannot bypass spy-only charge rule. | Cyborg Commando (TS, combat role); Commando (TW); Raveshaw (Ren) |
| `IA-INF-14` | **Civilian Disguise Courier** | Clandestine supply/intelligence movement with detection and jurisdiction risks. | Sudden Transport (RA3); Stealth Black Hand (Ren); Spy (RA1) |

### Armored vehicles, assault platforms and artillery

| ID | POLITIKA unit / building (new English name) | Purpose / faction-specific conditions | Original C&C design correlates (name and release) |
| --- | --- | --- | --- |
| `IA-ARM-01` | **Light Border Patrol Car** | Cheap armed road recon, but **not** canonical opening scout (plane). | Raider Buggy (TW); Buggy (TD); Attack Buggy (TS) |
| `IA-ARM-02` | **Armored Personnel Half-Track** | Infantry transport, exposed to anti-tank ambush. | APC (Ren); Flak Track (RA2); Reckoner (KW) |
| `IA-ARM-03` | **Fast Assault Tank** | Aggressive mobile armored strike, limited protection. | Light Tank (TD); Scorpion Tank (TW); Hammer Tank (RA3) |
| `IA-ARM-04` | **Frontier Battle Tank** | Main battle tank with slightly stronger raw firepower. | Medium Tank (TD); Predator Tank (TW); Titan (KW, tracked chassis conversion) |
| `IA-ARM-05` | **Heavy Siege Tank** | Slow, expensive high-damage assault against fortifications. | Mammoth Tank (TD); Apocalypse Tank (RA2); Overlord Tank (Gen) |
| `IA-ARM-06` | **Superheavy Prototype Tank** | Single rare late-game experimental multi-turret design; huge fuel and repair costs. | Mammoth Mk. II (TS, walker translated); Redeemer (KW); King Oni (RA3, walker translated) |
| `IA-ARM-07` | **Mobile Flame Assault Vehicle** | Assault fortifications at extreme short range. | Flame Tank (TD); Devil's Tongue (TS); Purifier (KW, flame role) |
| `IA-ARM-08` | **Rapid Anti-Air Autocannon** | Protect heavy columns from aircraft. | Mantis (KW); Slingshot (KW); Gattling Tank (Gen) |
| `IA-ARM-09` | **Heavy Anti-Tank Gun Carrier** | High-damage vehicle ambush with weak lateral protection. | Tank Destroyer (RA2); Beam Cannon (TW, long-range role); Refractor (TT, role) |
| `IA-ARM-10` | **Mobile Rocket Launcher** | Barrage weapon with inaccurate saturation and supply demands. | SSM Launcher (TD); V3 Rocket Launcher (RA2); Specter (KW) |
| `IA-ARM-11` | **Self-Propelled Breaching Artillery** | Long-range counter-fortification artillery. | Artillery (TS); Pacifier FAV (Uprising); Juggernaut (TW, tracked chassis) |
| `IA-ARM-12` | **Mobile Field Bunker** | Armored hull deploys as infantry garrison near attack routes. | Reckoner (KW); Tick Tank (TS); Battle Bunker (YR) |
| `IA-ARM-13` | **Attack Engineering Bulldozer** | Clears obstacles and can build cover; not a mass-destruction superweapon. | Grinder (Uprising, mechanical role); Dozer (TT); Crusher Crane (RA3) |
| `IA-ARM-14` | **Tracked Signal Jammer** | Suppresses hostile reconnaissance reports within radius. | Radar Van (Gen); Mobile Radar Jammer (RA1); Disruption Tower (TW) |
| `IA-ARM-15` | **Armored Heavy Gun Emplacement Carrier** | Deployable endgame heavy assault platform, transport cost high. | Behemoth (KW); Mastodon (TT); Rig (TW) |

### Reconnaissance, support and field logistics

| ID | POLITIKA unit / building (new English name) | Purpose / faction-specific conditions | Original C&C design correlates (name and release) |
| --- | --- | --- | --- |
| `IA-SUP-01` | **Nightjar Recon Plane** | **Canonical cheap unarmed opening scout plane**; no strike weapon or covert intel theft. | Spy Plane (RA1, observation); Surveillance Sweep (RA3); Recon Drone (TT, information role) |
| `IA-SUP-02` | **Mobile Works Command Vehicle** | Expands construction with vulnerable deployed works. | MCV (TD); Emissary (TW); Sputnik (RA3) |
| `IA-SUP-03` | **Forced-Priority Construction Tractor** | Build sites under militarized production priorities, high unrest costs. | Construction Dozer (Gen); Worker (Gen); Ore Collector expansion role (RA3) |
| `IA-SUP-04` | **Fuel Column Tanker** | Delivers refined vehicle/aviation fuels along contested routes. | Supply Truck (Gen); Harvester (TD); Supply Drop (TT) |
| `IA-SUP-05` | **Battlefield Recovery Tractor** | Field recovery of expensive armor; short range. | Mobile Repair Vehicle (TS); MRT (KW); Spanner (TT) |
| `IA-SUP-06` | **Camouflaged Supply Truck** | Carries limited materiel using falsified manifests. | Sudden Transport (RA3); Radar Van (Gen); Technical (Gen) |

### Air units and airborne delivery

| ID | POLITIKA unit / building (new English name) | Purpose / faction-specific conditions | Original C&C design correlates (name and release) |
| --- | --- | --- | --- |
| `IA-AIR-01` | **Interceptor Fighter** | Defends controlled approaches; costly airframes. | MiG (RA1); Apollo Fighter (RA3); Jet Tengu (RA3, role without transformation) |
| `IA-AIR-02` | **Low-Level Attack Jet** | Fast antitank and interdiction sorties. | Harrier (RA2); Vindicator (RA3); Banshee (TS, role without sci-fi weapons) |
| `IA-AIR-03` | **Assault Helicopter** | Anti-infantry and anti-armor support. | Apache (Ren); Harpy (TS); Venom (TW, role adapted) |
| `IA-AIR-04` | **Night Transport Helicopter** | Move small covert teams by heliborne insertion. | NightHawk (RA2); Chinook (TD); Twinblade (RA3) |
| `IA-AIR-05` | **Tactical Bomber** | Bomb ground targets with conventional munitions; vulnerable in air defenses. | Vertigo Bomber (TW, stealth behavior abstracted); Aurora Bomber (Gen); Century Bomber (RA3) |
| `IA-AIR-06` | **Gunship Escort Aircraft** | Persistent support to assault columns. | Hammerhead (KW); Salamander (TT); Harbinger Gunship (Uprising) |
| `IA-AIR-07` | **Prototype Ground-Attack Aircraft** | Endgame high-firepower prototype; longer turnaround. | Firehawk (TW); Harbinger Gunship (Uprising); ZOCOM Orca (KW) |
| `IA-AIR-08` | **Covert Air Courier** | Infiltration/supply support; carries no free spy ability. | Transport Helicopter (Ren); Nod Cargo Plane (TD); Sudden Transport (RA3, stealth function) |

### Maritime and amphibious forces

| ID | POLITIKA unit / building (new English name) | Purpose / faction-specific conditions | Original C&C design correlates (name and release) |
| --- | --- | --- | --- |
| `IA-SEA-01` | **Harbor Patrol Cutter** | Protect clandestine landings, susceptible to official maritime policing. | Gunboat (RA1); Yari Mini-Sub (RA3, light boat role) |
| `IA-SEA-02` | **Landing Transport Craft** | Infantry and light-armor coastal movement. | Amphibious Transport (RA2); Hovercraft (TD); Bullfrog (RA3, delivery role) |
| `IA-SEA-03` | **Fast Raider Boat** | Short-range antiship ambush in narrow waters. | Sea Scorpion (RA2); Naginata Cruiser (RA3, light role) |
| `IA-SEA-04` | **Covert Attack Submarine** | Expensive underwater route for raid and interdiction. | Submarine (RA1); Typhoon Attack Submarine (RA2); Boomer Submarine (YR, conventional torpedo role) |
| `IA-SEA-05` | **Armored Coastal Gunboat** | Gunfire support from available sea/river sectors. | Destroyer (RA1); Assault Destroyer (RA3) |
| `IA-SEA-06` | **Auxiliary Seaborne Headquarters** | Temporary maritime deployment center, not a permanent carrier. | Transport Ship (RA1); Amphibious Transport (RA2); Support Crawler (TT, mobile hub role) |

### Military-industrial and command structures

| ID | POLITIKA unit / building (new English name) | Purpose / faction-specific conditions | Original C&C design correlates (name and release) |
| --- | --- | --- | --- |
| `IA-BLD-01` | **Supreme Field Command** | Main construction, occupation and operational command. | Construction Yard (TD); Command Center (Gen); Construction Yard (RA3) |
| `IA-BLD-02` | **Occupation Command Post** | Expand claims in hostile territory; severe Unaligned jurisdiction risk. | Emissary Outpost (TW); Surveyor Outpost (TW); Defense Crawler (TT, forward deployment) |
| `IA-BLD-03` | **Paramilitary Training Camp** | Trains common infantry and security formations. | Hand of Nod (TW); Barracks (RA2); Hand of Nod (Ren) |
| `IA-BLD-04` | **Special Operations School** | Train qualified clandestine agents and late-game spy upgrades. | Secret Shrine (TW); Detention Camp (Gen); Stasis Chamber (TW, tech role) |
| `IA-BLD-05` | **Mechanized Assault Factory** | Build tanks, half-tracks and gun carriers. | War Factory (TW); Airstrip (TD, delivery concept); War Factory (RA3) |
| `IA-BLD-06` | **Experimental Armor Laboratory** | Unlock limited exotic-looking but tracked prototype assault chassis. | Tech Lab (TW); Battle Lab (RA2); Redeemer Engineering Facility (KW) |
| `IA-BLD-07` | **Combat Airfield** | Produce light recon scout, fighters and strike aircraft. | Airfield (RA1); Air Tower (TW); Airfield (Gen) |
| `IA-BLD-08` | **Aircraft Dispersal Pad** | Helicopter sortie/refuel and concealment infrastructure. | Helipad (TD); Air Tower (TW); Airfield (RA3) |
| `IA-BLD-09` | **Militarized Dockyard** | Naval production in maps with water access. | Naval Yard (RA1); Naval Shipyard (RA2); Seaport (RA3) |
| `IA-BLD-10` | **Occupied Fuel Refinery** | Convert crude to petroleum fuel or kerosene at normal costs, subject to supply risks. | Refinery (TW, resource analogue); Ore Refinery (RA1, analogue); Supply Center (Gen) |
| `IA-BLD-11` | **Gas Compressor Station** | Process/distribute natural gas when legally or forcibly accessed. | Ore Refinery (RA2, economy role); Extractor (TW, economy role) |
| `IA-BLD-12` | **Arms Storage Compound** | Stockpile fuel, components and equipment, with disruption risks. | Tiberium Silo (TW, role); Supply Center (Gen) |
| `IA-BLD-13` | **Industrial Electricity Plant** | Fuel-burning power with grid constraints, no nuclear electricity. | Power Plant (TD); Reactor (RA3); Cold Fusion Reactor (Gen, grid role) |
| `IA-BLD-14` | **State Armaments Research Bureau** | Military R&D and long progression for espionage prerequisites. | Tech Lab (TW); Secret Shrine (TW); Battle Lab (RA2) |
| `IA-BLD-15` | **Conditional Nuclear Material Processing Site** | **Locked** until full spy progression + successful authorized nuclear research infiltration. Processes abstract resources for first bomb **only**; never nuclear power. | Nuclear Missile Silo (Gen, strategic-tech footprint); Tiberium Chemical Plant (TW, processing-role inspiration) |
| `IA-BLD-16` | **First Weapon Assembly Bunker** | **Conditional** costly first bomb build; *own completion immediately wins*, no launch required. | Temple of Nod (TD, endgame building function); Nuclear Missile Silo (RA2, build progress analogue) |
| `IA-BLD-17` | **State Broadcast Commissariat** | Authorized propaganda radio, television and newspapers; repression drawbacks. | Propaganda Center (Gen); Voice of Kane (KW); Psychic Radar (YR, messaging role only) |

### Fortifications, occupation and secret infrastructure

| ID | POLITIKA unit / building (new English name) | Purpose / faction-specific conditions | Original C&C design correlates (name and release) |
| --- | --- | --- | --- |
| `IA-DEF-01` | **Border Guard Tower** | Basic vehicle/infantry screening post. | Guard Tower (TD); Watchtower (TW); Viper Turret (TT) |
| `IA-DEF-02` | **Heavy Machine-Gun Bunker** | Anti-infantry static defense with strong garrison. | Battle Bunker (YR); Bunker (Gen); Reckoner (KW, deployed role) |
| `IA-DEF-03` | **Anti-Armor Strongpoint** | Very high physical projectile attack, restricted traverse. | Gun Turret (TD); Guardian Cannon (TW); Obelisk of Light (TD, strongpoint role; no energy beam) |
| `IA-DEF-04` | **Anti-Air Flak Position** | Defense against reconnaissance planes and bombers. | Flak Cannon (RA2); SAM Site (TD); Rocket Pod (TT) |
| `IA-DEF-05` | **Mobile Air-Defense Radar** | Combined detection and SAM cueing site. | Radar Tower (RA2); Operations Center (TW); Radar Installation (TS) |
| `IA-DEF-06` | **Military Road Barrier** | Fortified checkpoint, wall and gate network. | Concrete Wall (TD); Gate (TS); Fortress Wall (RA3) |
| `IA-DEF-07` | **Covert Supply Cache** | Concealed local fuel/ammo depot; discoverable by investigation. | Supply Stash (Gen); Tunnel Network (Gen); Fake Supply Stash (ZH) |
| `IA-DEF-08` | **Underground Transit Entrance** | Permitted map-based short covert exit routes; no burrowing armored army. | Tunnel Network (Gen); Subterranean APC (TS); Burrow Tunnel (TT) |
| `IA-DEF-09` | **Coercive Loudspeaker Mast** | Morale pressure, propaganda influence with resistance/legitimacy trade-offs. | Speaker Tower (Gen); Voice of Kane (KW); Propaganda Center (Gen) |
| `IA-DEF-10` | **Restricted Print and Radio Studio** | Propaganda/media structure, not a mind-control device. | Psychic Radar (YR, visibility); Communications Center (TD); Propaganda Center (Gen) |
| `IA-DEF-11` | **High-Security Detention Facility** | Capture-related scenario/security structure; triggers diplomatic costs. | Detention Camp (Gen); Stasis Chamber (TW, confinement analog) |
| `IA-DEF-12` | **Abstract Contamination Operations Cell** | Authorized *fictional facility-status ability* and recovery counters, never modeled as pathogen engineering. | Bio-Research Laboratory (TD, scenario analogue); Tiberium Chemical Plant (TW, gameplay role) |

## Meridian Pact

**Playable.** Transcontinental commercial coalition emphasizing early fast truck scouting, qualified low-cost demolition sabotage, flexible regional infantry, fuel access, merchant contracts and a conventional-equipment auction network. No special media/psychological or nuclear production tree.

### Infantry, contractors and raiders

| ID | POLITIKA unit / building (new English name) | Purpose / faction-specific conditions | Original C&C design correlates (name and release) |
| --- | --- | --- | --- |
| `MP-INF-01` | **Regional Rifle Company** | Flexible inexpensive field rifle unit; not cheaper than Directorate equivalent. | Rebel (Gen); Rifle Infantry (RA1); Militant Squad (TW) |
| `MP-INF-02` | **Contracted Guard Detail** | Time-limited hired defense; explicit owner, wage and expiry. | Ranger (Gen); GDI Soldier (Ren); Devout (TT) |
| `MP-INF-03` | **Light Rocket Team** | Portable anti-armor ambush and defense. | RPG Trooper (Gen); Tankbuster (RA3); Rocket Soldier (TD) |
| `MP-INF-04` | **Hillside Mortar Team** | Indirect support in rough terrain. | Grenadier (RA1); Disc Thrower (TS); Mortar Cycle (Uprising, fire role) |
| `MP-INF-05` | **Bridge and Road Engineer** | Repair crossings, roads, fuel and merchant infrastructure. | Engineer (RA1); Worker (Gen); Combat Engineer (RA3) |
| `MP-INF-06` | **Portable Field Medic** | Sustain raiding infantry far from fixed bases. | Field Medic (TS); Combat Engineer (KW, support); Ambulance (Gen) |
| `MP-INF-07` | **Caravan Reconnaissance Scouts** | Ground intelligence through observation/trade only, no automatic classified intel. | Spy (RA1, information role only); Pathfinder (Gen); Shadow Team (TW, observation role) |
| `MP-INF-08` | **Early Sabotage Specialist** | **Early, low-cost non-spy qualified saboteur:** abstract charge on eligible vehicle/building. | Terrorist (Gen, demolition role); Crazy Ivan (RA2); Demo Trap (Gen); Saboteur (TW, role translated) |
| `MP-INF-09` | **Raiding Engineer** | **Early non-spy qualified charge** plus repair/capture permissions where authorized. | Engineer (Ren); Combat Engineer (RA3); Demo Trap (Gen) |
| `MP-INF-10` | **Mountain Ambush Squad** | Terrain bonus, suppressive attacks and quick withdrawal. | Jarmen Kell (Gen, sniper role); Shadow Team (TW); Rebel Ambush (Gen) |
| `MP-INF-11` | **Contracted Veteran Marksman** | Uncommon hired precision shooter, time-limited contract. | Pathfinder (Gen); Sniper (RA2); Deadeye (Ren) |
| `MP-INF-12` | **Local Militia Levy** | Emergency defensive line at low tech, morale-sensitive. | Rebel (Gen); Militant (TW); Angry Mob (Gen, formation effect) |
| `MP-INF-13` | **Foreign Military Adviser** | Provides purchased tactical upgrades without unlocking native rival factories. | Colonel Burton (Gen, specialist command); Black Lotus (Gen, specialist); Colonel Burton (ZH) |

### Trucks, armored cars and mobile combat equipment

| ID | POLITIKA unit / building (new English name) | Purpose / faction-specific conditions | Original C&C design correlates (name and release) |
| --- | --- | --- | --- |
| `MP-ARM-01` | **Roadrunner Scout Truck** | **Canonical best early reconnaissance:** cheap, very fast, lightly armored/armed; no automatic sabotage permission. | Technical (Gen); Humvee (TD); Pitbull (TW); Raider Buggy (TW) |
| `MP-ARM-02` | **Armed Commercial Pickup** | Low-cost escort with modular machine-gun/missile mounts. | Technical (Gen); Rocket Buggy (Gen); Buggy (TD) |
| `MP-ARM-03` | **Light Armored Van** | Transfer infantry and contracts between dispersed sites. | APC (TD); Riptide ACV (RA3); Reckoner (KW, transport) |
| `MP-ARM-04` | **Modular Cargo Gun Truck** | Reconfigurable anti-personnel or anti-vehicle weapon mount. | Multigunner IFV (RA3); Quad Cannon (Gen); IFV (RA2) |
| `MP-ARM-05` | **Scavenged Light Tank** | Refitted imported armor; variable condition and spares. | Light Tank (TD); Scorpion Tank (Gen); Lasher Tank (YR) |
| `MP-ARM-06` | **Surplus Battle Tank** | Imported standard armor, less dependable than elite Concord/Ascendancy armor. | Grizzly Battle Tank (RA2); Rhino Tank (RA2); Guardian Tank (RA3) |
| `MP-ARM-07` | **Mobile Recoilless-Gun Carrier** | Fast anti-armor hit-and-run platform. | Tank Destroyer (RA2); Rocket Buggy (Gen); Pitbull (TW) |
| `MP-ARM-08` | **Truck-Mounted Rocket Rack** | Cheap indirect barrage, reload/supply vulnerabilities. | Rocket Buggy (Gen); V2 Rocket Launcher (RA1); MLRS (TD) |
| `MP-ARM-09` | **Quad Anti-Air Technical** | Mobile antiair and anti-infantry escort. | Quad Cannon (Gen); Gattling Tank (Gen); Slingshot (KW) |
| `MP-ARM-10` | **Mobile Smoke-Screen Truck** | Temporarily obscures lanes, not invisibility. | Mobile Gap Generator (RA1); Gap Generator (RA2, concealment function); Mobile Stealth Generator (TS) |
| `MP-ARM-11` | **Ambush Mine Carrier** | Lays abstract combat mines; detectable and clearable. | Minelayer (RA1); Demo Trap (Gen); Burrow Mines (TT) |
| `MP-ARM-12` | **Rough-Terrain Artillery Truck** | Flexible medium self-propelled mortar/howitzer. | Artillery (TD); Mortar Cycle (Uprising); Pacifier FAV (Uprising, siege analogue) |
| `MP-ARM-13` | **Captured Heavy Armor** | **Market-only or battlefield-captured**, cannot emerge from Pact factory by default. | Mammoth Tank (TD); Overlord Tank (Gen); Apocalypse Tank (RA2) |

### Construction, economic and contract logistics

| ID | POLITIKA unit / building (new English name) | Purpose / faction-specific conditions | Original C&C design correlates (name and release) |
| --- | --- | --- | --- |
| `MP-SUP-01` | **Forward Trade Constructor** | Establish dispersed depots and authorized contracts near territory. | Worker (Gen); MCV (TD); Prospector (RA3) |
| `MP-SUP-02` | **Merchant Delivery Truck** | Transfer bought assets through actual road traffic. | Supply Truck (Gen); Technical (Gen); Cargo Plane (TD, delivery concept) |
| `MP-SUP-03` | **Crude Tanker Convoy** | Collect and transport crude *after* legal or contested access to field. | Supply Truck (Gen); Ore Truck (RA1); Ore Collector (RA3) |
| `MP-SUP-04` | **Petroleum Product Tanker** | Deliver processed fuel to vehicles, clients and market depots. | Harvester (TD, logistics analogue); Supply Truck (Gen); Supply Drop (TT) |
| `MP-SUP-05` | **Commercial Fuel Bowser** | Deliver refined kerosene to air assets. | Chinook (Gen, supply role); Supply Truck (Gen) |
| `MP-SUP-06` | **Cargo Brokerage Van** | Move conventional weapon components with merchant-paperwork detection. | Radar Van (Gen, civilian chassis); Sudden Transport (RA3, concealed cargo role) |
| `MP-SUP-07` | **Mercenary Personnel Carrier** | Transport contracted troops, retains hiring and contract tags. | Troop Crawler (Gen); Riptide ACV (RA3); Armadillo (TT) |
| `MP-SUP-08` | **Repair Caravan** | Workshop unit supporting battered imported vehicles. | Mobile Repair Vehicle (TS); MRT (KW); Spanner (TT) |
| `MP-SUP-09` | **Black-Market Delivery Aircraft** | Brings *purchased conventional* cargo only; interdiction and arrival delay apply. | Cargo Plane (TD); Supply Drop (Gen); V-35 Ox (TW) |

### Aircraft and reconnaissance support

| ID | POLITIKA unit / building (new English name) | Purpose / faction-specific conditions | Original C&C design correlates (name and release) |
| --- | --- | --- | --- |
| `MP-AIR-01` | **Light Utility Helicopter** | Recon supply and courier flights, not superior to opening scout trucks. | Chinook (RA1); Transport Helicopter (Ren); NightHawk (RA2) |
| `MP-AIR-02` | **Armed Escort Helicopter** | Basic hired helicopter escort for convoys. | Hind (RA1); Twinblade (RA3); Harpy (TS) |
| `MP-AIR-03` | **Imported Fighter-Bomber** | Limited late-technology tactical air sorties using purchased components. | Harrier (RA2); Yak (RA1); Vindicator (RA3) |
| `MP-AIR-04` | **Cargo Transport Plane** | Move contracted assets and vital merchant cargo between airstrips. | Cargo Plane (TD); V-35 Ox (TW); Century Bomber (RA3, transport role) |
| `MP-AIR-05` | **Coastal Patrol Aircraft** | Detect shipping and verify reports at extended distance. | Spy Plane (RA1); ORCA (TW, reconnaissance role); Surveillance Sweep (RA3) |
| `MP-AIR-06` | **Hired Air Support Flight** | Externally contracted temporary strike, not an indigenous nuclear bomber. | A-10 Strike (Gen); Spectre Gunship (Gen); Orca Strike (TW) |

### Naval and amphibious commerce

| ID | POLITIKA unit / building (new English name) | Purpose / faction-specific conditions | Original C&C design correlates (name and release) |
| --- | --- | --- | --- |
| `MP-SEA-01` | **Merchant Coastal Barge** | Move goods/units across the Adriatic or navigable rivers. | Transport Ship (RA1); Amphibious Transport (RA2); Hovercraft (TD) |
| `MP-SEA-02` | **Fast Maritime Scout** | Rapid reconnaissance and anti-boat defense. | Gunboat (RA1); Yari Mini-Sub (RA3, scout analogy) |
| `MP-SEA-03` | **Contract Security Patrol Boat** | Escort merchant convoys, billable service. | Sea Scorpion (RA2); Hydrofoil (RA3); Gunboat (TD) |
| `MP-SEA-04` | **Amphibious Cargo Carrier** | Transfer light vehicles and troops to shoreline roads. | Riptide ACV (RA3); Amphibious APC (TS); Bullfrog (RA3, troop handling) |
| `MP-SEA-05` | **Surplus Coastal Submarine** | Scarce conventional underwater market acquisition, large upkeep. | Submarine (RA1); Typhoon Submarine (RA2); Boomer Submarine (YR, torpedo role) |
| `MP-SEA-06` | **Licensed Missile Corvette** | Export-model missile craft with restricted availability. | Destroyer (RA1); Naginata Cruiser (RA3, ship-combat role) |
| `MP-SEA-07` | **Floating Trade Anchorage** | Deployable short-range forward maritime logistics hub; not an armed superweapon. | Support Crawler (TT, mobile HQ role); Naval Yard (RA1); Aircraft Carrier (RA2, sea hub) |

### Commercial production and administration

| ID | POLITIKA unit / building (new English name) | Purpose / faction-specific conditions | Original C&C design correlates (name and release) |
| --- | --- | --- | --- |
| `MP-BLD-01` | **Pact Trade Headquarters** | Primary construction, contract registry and logistics authority. | Command Center (Gen); Construction Yard (RA1); Drone Platform (TW, deployable HQ role) |
| `MP-BLD-02` | **Forward Bazaar Outpost** | Deploy modest local merchant supply and information network. | GLA Tunnel Network (Gen, distributed-node role); Outpost (TW); Prospector Command Hub (RA3) |
| `MP-BLD-03` | **Regional Training Compound** | Recruit native Pact infantry, engineers and saboteurs. | Barracks (Gen); Hand of Nod (TW); Instant Dojo (RA3, role) |
| `MP-BLD-04` | **Light Vehicle Assembly Yard** | Build native light trucks, techs, carriers and modular mounts. | Arms Dealer (Gen); War Factory (TW); Mecha Bay (RA3) |
| `MP-BLD-05` | **Licensed Repair Factory** | Refurbish conventional surplus purchases and maintain vehicles. | Service Depot (RA1); Arms Dealer (Gen); Crusher Crane (RA3, repair role) |
| `MP-BLD-06` | **Open Airstrip** | Limited fixed-wing airframes and cargo arrivals. | Airfield (Gen); Airstrip (TD); Airbase (RA3) |
| `MP-BLD-07` | **Merchant Helipad** | Rotary-wing refueling and cargo lift. | Helipad (TD); Air Tower (TW); Airfield (RA3) |
| `MP-BLD-08` | **Harbor Exchange** | Civilian port, contracts and licensed maritime manufacture. | Naval Yard (RA1); Seaport (RA3); Imperial Docks (RA3) |
| `MP-BLD-09` | **Crude Oil Refinery** | Native crude -> fuel/kerosene refining; nearby spawn gives **no free ownership**. | Ore Refinery (RA1, economy); Supply Stash (Gen, economic analogue); Refinery (TW) |
| `MP-BLD-10` | **Gas Processing Depot** | Natural gas supply and local electrical feed. | Supply Stash (Gen, economic analogue); Ore Refinery (RA2, economy); Extractor (TW) |
| `MP-BLD-11` | **Fuel Storage Farm** | Stores oil, kerosene and petroleum as **distinct inventories**. | Tiberium Silo (TD); Supply Center (Gen); Tiberium Silo (TW) |
| `MP-BLD-12` | **Diesel Generator Station** | Low-cost limited electricity; consumes legal processed fuel. | Power Plant (TD); Instant Generator (RA3); Power Plant (RA3) |
| `MP-BLD-13` | **Commercial Ledger Office** | Tracks purchase orders, payment, insurance and contract expiry. | Black Market (Gen); Supply Center (Gen); Detention Camp (Gen, information role) |
| `MP-BLD-14` | **Foreign Exchange House** | Trade income, price changes and non-nuclear commodity brokerage. | Black Market (Gen); Supply Drop Zone (Gen); Grinder (YR, resale analogue) |
| `MP-BLD-15` | **Black Market Auction Hall** | **Signature** exchange for *conventional* otherwise-unbuildable equipment; **never lists completed nuclear bombs**. | Black Market (Gen); Supply Stash (Gen, economy role); Arms Dealer (Gen) |
| `MP-BLD-16` | **Mercenary Recruitment Office** | Offer troop services with wages, owner/employer and expiry metadata. | Barracks (Gen); GLA Palace (Gen, garrison/mercenary role); Hand of Nod (Ren, purchasing role) |
| `MP-BLD-17` | **Cross-Border Dispatch Station** | Schedule physical truck/plane/boat delivery with risks. | Airstrip (TD); Supply Center (Gen); Cargo Plane (TD, delivery role) |

### Market-exclusive assets (conventional, brokered rather than native)

| ID | POLITIKA unit / building (new English name) | Purpose / faction-specific conditions | Original C&C design correlates (name and release) |
| --- | --- | --- | --- |
| `MP-MKT-01` | **Imported Heavy Recovery Crawler** | **Auction/contract only**: powerful armored towing and recovery; no faction factory produces this exact composite variant. | Mobile Repair Transport (KW); Rig (TW); Crusher Crane (RA3, recovery function) |
| `MP-MKT-02` | **Experimental Mine-Clearing Robot** | **Market-only** remote tracked mine detector/clearer; short range and battery limited, not an autonomous combat AI. | Terror Drone (RA2, small robot archetype); Sentry Drone (ZH); Hunter-Seeker Droid (TS, drone inspiration without homing strike) |
| `MP-MKT-03` | **Surplus Antiship Launcher Trailer** | **Market-only** short-range coastal antiship battery for suitable map shorelines. | Dreadnought (RA2, missile function); V3 Rocket Launcher (RA2, trailer silhouette); Naginata Cruiser (RA3, naval targeting role) |
| `MP-MKT-04` | **Amphibious Bridging Kit Carrier** | **Market-only** specialized engineering vehicle for river crossings and pontoon sites. | Amphibious APC (TS); Riptide ACV (RA3); Rig (TW, deployment role) |
| `MP-MKT-05` | **Foreign Surveillance Radar Package** | **Market-only** support vehicle with electronic detection, requires skilled operators; no classified spy intel unlock. | Listening Outpost (ZH); Mobile Sensor Array (TS); Radar Van (Gen) |
| `MP-MKT-06` | **Commercial Decoy Convoy** | **Market-only** unarmed dummy truck group, attracts misdirected sightings without revealing truth. | Decoy Army (TW, power lineage); Fake Supply Stash (ZH, decoy lineage); Sudden Transport (RA3, disguise function) |
| `MP-MKT-07` | **Contracted Light Strike Flight** | **Hired service** via contractor contract and airport access, limited single sortie; no nuclear payload. | A-10 Strike (Gen, support power); Orca Strike (TW, support power); Surgical Strike (RA3, support protocol) |
| `MP-MKT-08` | **High-End Coastal Interceptor** | **Rare market-only** conventional patrol hydrofoil, not an unlimited free shipyard unit. | Hydrofoil (RA3, hull/movement role); Gunboat (RA1); Sea Scorpion (RA2) |
| `MP-MKT-09` | **Artillery Observation Balloon** | **Market-only** tethered low-tech forward observation gear; stationary, fragile and weather constrained. | Spy Plane (RA1, spotting role); Recon Drone (TT, spotting power); Mobile Sensor Array (TS, information role) |

### Defenses and special transaction sites

| ID | POLITIKA unit / building (new English name) | Purpose / faction-specific conditions | Original C&C design correlates (name and release) |
| --- | --- | --- | --- |
| `MP-DEF-01` | **Roadblock Fighting Position** | Cheap local anti-personnel protection. | Stinger Site (Gen, fortified-site role); Bunker (Gen); Pillbox (RA1) |
| `MP-DEF-02` | **Portable Anti-Air Emplacement** | Lightweight air denial for scattered sites. | Stinger Site (Gen); SAM Site (TD); Flak Cannon (RA2) |
| `MP-DEF-03` | **Light Anti-Tank Gun Pit** | Ambush armored vehicles on narrow roads. | Gun Turret (TD); Guardian Cannon (TW); Tank Bunker (YR) |
| `MP-DEF-04` | **Tunnel Supply Entrance** | Short-range map-dependent freight/infantry network; inspectable. | Tunnel Network (Gen); Burrow Tunnel (TT); Subterranean APC (TS) |
| `MP-DEF-05` | **Field Mine Control Shed** | Limited mine stocks and route marking. | Demo Trap (Gen); Cluster Mines (Gen); Minelayer (RA1) |
| `MP-DEF-06` | **Trade Route Watchtower** | Reports hostile traffic and detects incoming convoys. | Watchtower (TW); Radar Van (Gen); Radar Dome (RA1) |
| `MP-DEF-07` | **Safehouse Distribution Depot** | Serves raider/saboteur deployment; not an advanced spy lab. | GLA Palace (Gen, fortified hideout); Secret Shrine (TW, tech lineage); Supply Stash (Gen) |
| `MP-DEF-08` | **Licensed Contract Vault** | Keeps registered conventional import equipment for sale/lease. | Black Market (Gen); Service Depot (RA1); Grinder (YR, material recovery) |
| `MP-DEF-09` | **Contract Arbitration Office** | Manages mercenary loyalty, pay disputes and termination. | Command Center (Gen); Detention Camp (Gen, information/holding analogue) |

## The Unaligned (NPC)

**Never playable.** Yugoslav sovereign federal, regional and municipal authorities; JNA and Territorial Defence; living civilian economy and autonomous security response. These rows are *NPC and scenario entities*, not human-recruitable unit buttons.

### Federal, Territorial Defence and security personnel

| ID | POLITIKA unit / building (new English name) | Purpose / faction-specific conditions | Original C&C design correlates (name and release) |
| --- | --- | --- | --- |
| `UN-INF-01` | **Yugoslav Army Rifle Section** | JNA line infantry for autonomous federal security; **NPC-controlled**. | Rifle Infantry (RA1); GDI Minigunner (TD); Rifleman Squad (TW) |
| `UN-INF-02` | **Territorial Defence Reserve** | Locally mobilized defensive platoon; autonomous, no player barracks. | Militant Squad (TW); Rebel (Gen); Devout (TT) |
| `UN-INF-03` | **Federal Border Guards** | Control crossing points and observe international movements. | GDI Officer (Ren); Peacekeeper (RA3); Rifle Infantry (RA1) |
| `UN-INF-04` | **Federal Police Patrol** | Town security, investigation and graded force. | Peacekeeper (RA3); Attack Dog (RA1, screening role); GDI Soldier (Ren) |
| `UN-INF-05` | **Railway Protection Detachment** | Secure civilian transport and key bridges. | GI (RA2); Rifleman Squad (TW); GDI Officer (Ren, guard role) |
| `UN-INF-06` | **JNA Rocket Infantry** | Anti-armor defense of military facilities. | Rocket Soldier (TD); Missile Squad (TW); Tank Hunter (Gen) |
| `UN-INF-07` | **Federal Engineers** | Repair public infrastructure, bridges, hospitals and military sites. | Engineer (TD); Combat Engineer (RA3); Engineer (TT) |
| `UN-INF-08` | **JNA Field Medical Unit** | Military medical support for NPC mobilization. | Medic (RA1); Field Medic (TS); Ambulance (Gen) |
| `UN-INF-09` | **Mountain Reconnaissance Patrol** | Local scout infantry with terrain knowledge. | Sniper Team (TW, scouting); Pathfinder (Gen); Shadow Team (TW, reconnaissance role) |
| `UN-INF-10` | **State Security Investigator** | Investigates facility intrusions, fraud and alleged foreign plots. | Spy (RA1); Detention Camp (Gen, information role); Black Lotus (Gen, spy interaction) |
| `UN-INF-11` | **Federal Counterintelligence Officer** | **Strongest counterintelligence:** intercepts, corroborates and blocks infiltration. | Detention Camp (Gen); Spy (RA3); Mobile Sensor Array (TS) |
| `UN-INF-12` | **Deception Bureau Liaison** | NPC-only **planted false intelligence / false flags**; never a purchasable player spy. | Decoy Army (TW); Fake Structures (RA1); Radar Jamming Missile (TW) |
| `UN-INF-13` | **Public Continuity Commissioner** | Coordinates Tito-era official communications and civilian morale, NPC-only. | Propaganda Center (Gen); Voice of Kane (KW, influence only); Speaker Tower (Gen) |
| `UN-INF-14` | **Federal Honor Guard** | Scenario-protected symbolic/ceremonial detachment, not recruited by players. | Commando (TD, special-only concept); Tanya (RA2, unique-person role) |

### Military vehicles, patrols and field response

| ID | POLITIKA unit / building (new English name) | Purpose / faction-specific conditions | Original C&C design correlates (name and release) |
| --- | --- | --- | --- |
| `UN-ARM-01` | **Municipal Patrol Car** | Police response and traffic checks; jurisdiction-sensitive. | Ranger (RA1); Humvee (TD); Attack Buggy (TS, chassis role) |
| `UN-ARM-02` | **Federal Recon Jeep** | Army visual patrol of roads and bases. | Humvee (TD); Pitbull (TW); Ranger (RA1) |
| `UN-ARM-03` | **Border Surveillance Truck** | Radio-equipped mobile border watch. | Radar Van (Gen); Mobile Sensor Array (TS); Mobile Gap Generator (RA1, sensor role) |
| `UN-ARM-04` | **JNA Armored Personnel Carrier** | Protected federal infantry transport. | APC (TD); Riptide ACV (RA3, land role); Armadillo (TT) |
| `UN-ARM-05` | **Territorial Defence Truck** | Inexpensive reserve transport and ammunition hauling. | Technical (Gen); Troop Crawler (Gen); Flak Track (RA2, chassis) |
| `UN-ARM-06` | **Federal Battle Tank** | Conventional Yugoslav-era armored formation, medium/heavy. | Medium Tank (TD); Rhino Tank (RA2); Predator Tank (TW) |
| `UN-ARM-07` | **Reserve Tank** | Older stored equipment mobilized near crises. | Light Tank (TD); Grizzly Tank (RA2); Scorpion Tank (TW) |
| `UN-ARM-08` | **JNA Anti-Air Gun Carrier** | Mobile short-range air defense. | Flak Track (RA2); Gattling Tank (Gen); Slingshot (KW) |
| `UN-ARM-09` | **Army Rocket Artillery** | Conventional mobile multiple-launch rockets. | MLRS (TD); V3 Rocket Launcher (RA2); V4 Rocket Launcher (RA3) |
| `UN-ARM-10` | **Field Howitzer Tractor** | Towed or self-propelled artillery for defensive fighting. | Artillery (RA1); Juggernaut (TW, artillery role) |
| `UN-ARM-11` | **Military Engineering Dozer** | Move barriers and repair bridge approaches. | Construction Dozer (Gen); Dozer (TT); Crusher Crane (RA3, recovery role) |
| `UN-ARM-12` | **Federal Mobile Repair Unit** | Stabilizes damaged JNA/public vehicles. | Mobile Repair Vehicle (TS); Mobile Repair Transport (KW); Rig (TW) |
| `UN-ARM-13` | **Anti-Armor Defense Vehicle** | Protect sensitive installations against raiders. | Tank Destroyer (RA2); Guardian Tank (RA3); Mantis (KW, counter unit role) |
| `UN-ARM-14` | **Reserve Mobilization Carrier** | Move territorial defense squads under NPC orders. | Troop Crawler (Gen); Reckoner (KW); Amphibious APC (TS) |
| `UN-ARM-15` | **Crisis Command Vehicle** | Small mobile command post for public-order escalation. | Support Crawler (TT, mobile HQ inspiration); Rig (TW); Mobile Construction Vehicle (TD) |

### Civilian, municipal and service vehicles

| ID | POLITIKA unit / building (new English name) | Purpose / faction-specific conditions | Original C&C design correlates (name and release) |
| --- | --- | --- | --- |
| `UN-CIV-01` | **Civilian Passenger Car** | Ambient traffic and continuity; not recruitable. | Ranger (RA1, traffic analogue); Humvee (TD, road unit chassis) |
| `UN-CIV-02` | **Intercity Bus** | Move civilians and evacuation groups under incident rules. | APC (TD, transport role); Troop Crawler (Gen, capacity analogy) |
| `UN-CIV-03` | **Freight Lorry** | Factories, warehouses and shops maintain economy flows. | Supply Truck (Gen); Ore Truck (RA1); Harvester (TW, supply role) |
| `UN-CIV-04` | **Railway Freight Train** | Schedules resource and merchant convoys; rail asset and mission target. | Cargo Plane (TD, delivery-event analogue); Supply Truck (Gen, convoy role); Harvester (TD, economic flow) |
| `UN-CIV-05` | **Oil Tanker Lorry** | Civilian crude oil and finished petroleum remain separately tracked. | Ore Truck (RA1); Supply Truck (Gen); Ore Collector (RA3) |
| `UN-CIV-06` | **Public Ambulance** | Casualty response and morale stabilization after incidents. | Ambulance (Gen); Medic (RA1, field role) |
| `UN-CIV-07` | **Civilian Fire Engine** | Emergency building damage response, no combat. | Mobile Repair Vehicle (TS); Rig (TW, recovery role) |
| `UN-CIV-08` | **Electrical Maintenance Van** | Repair power lines, substations and street outages. | Repair Facility (TD, repair concept); Engineer (RA1); Spanner (TT) |
| `UN-CIV-09` | **Postal and Press Delivery Van** | Newspaper dissemination, observable and disruptable. | Supply Truck (Gen); Technical (Gen, carrier role) |
| `UN-CIV-10` | **Agricultural Tractor** | Rural economy and market goods; non-military. | Worker (Gen, work role); Ore Collector (RA3, industry role) |
| `UN-CIV-11` | **Commercial Fishing Boat** | Neutral regional water economy, scenario-only. | Gunboat (RA1, small hull only); Yari Mini-Sub (RA3, small hull analogue) |
| `UN-CIV-12` | **River Ferry** | Realistic civilian crossing; can be closed by NPC jurisdiction. | Transport Ship (RA1); Amphibious Transport (RA2) |

### Air force, civil aviation and surveillance

| ID | POLITIKA unit / building (new English name) | Purpose / faction-specific conditions | Original C&C design correlates (name and release) |
| --- | --- | --- | --- |
| `UN-AIR-01` | **JNA Interceptor Fighter** | Defend sovereign airspace from unauthorized intrusions. | MiG (RA1); Apollo Fighter (RA3); Firehawk (TW) |
| `UN-AIR-02` | **JNA Ground-Attack Jet** | Conventional air response when sovereignty is violated. | Yak (RA1); Vindicator (RA3); Harrier (RA2) |
| `UN-AIR-03` | **Federal Transport Helicopter** | Military personnel and evacuation logistics. | Chinook (TD); Transport Helicopter (Ren); Twinblade (RA3, lift role) |
| `UN-AIR-04` | **Army Attack Helicopter** | JNA rotary close support, heavy escalation. | Hind (RA1); Apache (Ren); Orca (TW, attack role) |
| `UN-AIR-05` | **Federal Reconnaissance Aircraft** | Monitor borders, mountain passes and maritime coastlines. | Spy Plane (RA1); Surveillance Sweep (RA3); Recon Drone (TT) |
| `UN-AIR-06` | **Civil Passenger Airliner** | Timetabled neutral flights; civilian, not a recruitable attack aircraft. | Century Bomber (RA3, large airframe only); V-35 Ox (TW, flight behavior) |
| `UN-AIR-07` | **Emergency Airlift Plane** | Evacuate civilians/food; mission scripted, not base-trained. | V-35 Ox (TW); Century Bomber (RA3, transport capacity) |
| `UN-AIR-08` | **Adriatic Maritime Patrol Plane** | Identify shipping, foreign landings and offshore incursions. | ORCA Fighter (TS, observation role); Spy Plane (RA1); Orca (TT, recon role) |

### Coastal, riverine and naval forces

| ID | POLITIKA unit / building (new English name) | Purpose / faction-specific conditions | Original C&C design correlates (name and release) |
| --- | --- | --- | --- |
| `UN-SEA-01` | **JNA Coastal Patrol Boat** | Routine sea and port inspection. | Gunboat (RA1); Gunboat (TD); Yari Mini-Sub (RA3, small craft role) |
| `UN-SEA-02` | **Federal Missile Patrol Boat** | Coastal anti-surface missile response. | Sea Scorpion (RA2); Destroyer (RA1); Naginata Cruiser (RA3, role) |
| `UN-SEA-03` | **Adriatic Frigate Escort** | Protect shipping and defend federal waters. | Destroyer (RA1); Aegis Cruiser (RA2, AA function) |
| `UN-SEA-04` | **JNA Attack Submarine** | Protect coastal approaches; deep water required. | Submarine (RA1); Akula Submarine (RA3); Typhoon Attack Submarine (RA2) |
| `UN-SEA-05` | **Amphibious Military Transport** | Ferry JNA and Territorial Defence forces during crises. | Amphibious Transport (RA2); Hovercraft (TD); Bullfrog (RA3, delivery role) |
| `UN-SEA-06` | **River Police Launch** | Patrol Sava, Drava, Danube and other navigable sectors. | Gunboat (RA1); Stingray (RA3, hull role) |
| `UN-SEA-07` | **Civil Port Tug** | Maintain port access, civilian/naval yard services. | Transport Ship (RA1, utility hull); Gunboat (RA1, hull size) |
| `UN-SEA-08` | **Federal Search-and-Rescue Vessel** | Rescue stranded civilians and manage maritime incidents. | Hydrofoil (RA3, service motion); Transport Ship (RA1, carry capacity) |

### Federal, republican and civilian infrastructure

| ID | POLITIKA unit / building (new English name) | Purpose / faction-specific conditions | Original C&C design correlates (name and release) |
| --- | --- | --- | --- |
| `UN-BLD-01` | **Federal Executive Secretariat** | Nationwide NPC administration, not a player construction yard. | Construction Yard (TD, faction centerpiece); Command Center (Gen) |
| `UN-BLD-02` | **Republic Government House** | Regional legitimacy and authority; existing map structure. | Command Center (Gen); Construction Yard (RA1, strategic target function) |
| `UN-BLD-03` | **Municipal Assembly** | Local governance, permits and incident jurisdiction. | Tiberium Spike (TW, neutral map-objective pattern); Command Center (Gen, authority role) |
| `UN-BLD-04` | **Workers' Council Hall** | Yugoslav self-management civic facility; economic participation. | Propaganda Center (Gen, public-morale analogue); Supply Center (Gen, economy role) |
| `UN-BLD-05` | **JNA Barracks Complex** | NPC military reinforcement and garrison; not player-buildable. | Barracks (TD); GDI Barracks (Ren); Boot Camp (RA3) |
| `UN-BLD-06` | **Territorial Defence Depot** | Mobilize reserve infantry and local patrols. | GLA Palace (Gen, troop-garrison role); Barracks (Gen) |
| `UN-BLD-07` | **Federal Tank and Repair Works** | NPC construction/repair for JNA vehicles, not player factory. | Weapons Factory (Ren); War Factory (RA2); Service Depot (RA1) |
| `UN-BLD-08` | **Public Fuel Refinery** | Federal/domestic crude -> finished fuel/kerosene; process separate materials. | Refinery (TD, econ); Ore Refinery (RA2); Supply Center (Gen) |
| `UN-BLD-09` | **Public Gas Processing Plant** | Administer natural gas fields, pipelines and supplies. | Ore Refinery (RA1, resource role); Supply Center (Gen) |
| `UN-BLD-10` | **Electric Power Station** | Civilian electricity network node; not nuclear. | Power Plant (TD); Power Plant (RA3); Reactor (RA3, output role) |
| `UN-BLD-11` | **Federal Substation** | Distributes grid power through owned connectors, target for restoration. | Power Turbine (TS, generation upgrade); Power Plant (TW, network role) |
| `UN-BLD-12` | **Railway Freight Terminal** | Unaligned cargo access, deliveries and train state. | Supply Center (Gen); Tiberium Silo (TW, storage role) |
| `UN-BLD-13` | **Harbor Authority** | Sovereign dock access, maritime inspection, port economy. | Naval Yard (RA1); Seaport (RA3); Submarine Pen (RA1) |
| `UN-BLD-14` | **Federal University and Laboratory** | Research objective, but **not player nuclear research by ownership alone**. | Tech Center (TS); Battle Lab (RA2); Armory (TW) |
| `UN-BLD-15` | **State Hospital** | Heal and stabilize civilian casualties in scripted incidents. | Hospital (TD); Field Medic (TS, health role); Armory (TW, treatment function) |
| `UN-BLD-16` | **Bank and Clearing Office** | Civilians and financial ledger system; protects payments. | Supply Drop Zone (Gen, finance analogue); Black Market (Gen, exchange function but not illicit) |

### Security, counterintelligence and media facilities

| ID | POLITIKA unit / building (new English name) | Purpose / faction-specific conditions | Original C&C design correlates (name and release) |
| --- | --- | --- | --- |
| `UN-SEC-01` | **Federal Police Headquarters** | Dispatch investigators and manage local arrests. | Detention Camp (Gen, intelligence role); Communications Center (TD); Command Center (Gen) |
| `UN-SEC-02` | **Border Customs Station** | Check trade, shipments and foreign armed patrols. | Watchtower (TW); Radar Dome (RA1); Detention Camp (Gen) |
| `UN-SEC-03` | **State Security Directorate** | **Strongest NPC counterintelligence**; evaluate reports, trace spies. | Detention Camp (Gen); Secret Shrine (TW, intelligence institution); Operations Center (TW) |
| `UN-SEC-04` | **Federal Deception Cell** | NPC-only fabricated reports and false-flag attribution under stability policy. | Decoy Army (TW); Fake Structures (RA1); Radar Jamming Missile (TW) |
| `UN-SEC-05` | **Air Defense Radar** | Identifies aircraft near protected airspace. | Radar Installation (TS); Radar Dome (RA1); Radar Tower (RA2) |
| `UN-SEC-06` | **Federation Radio House** | Official public radio, morale/continuity broadcasting. | Propaganda Center (Gen); Communications Center (TD); Speaker Tower (Gen) |
| `UN-SEC-07` | **Television Network Center** | Public television and protected historical story broadcasts. | SpySat Uplink (RA2, network coverage); Communications Center (TD); Propaganda Center (Gen) |
| `UN-SEC-08` | **Federal Printing Works** | Press/newspaper operations, delivery-dependent. | Propaganda Center (Gen); Communications Center (TD) |
| `UN-SEC-09` | **Classified Continuity Archive** | NPC narrative evidence vault for Tito-era mystery; no universal vision. | Technology Center (TD, special objective); Tech Center (TW); Detention Camp (Gen, intel storage role) |
| `UN-SEC-10` | **Regional Emergency Headquarters** | Coordinate crisis containment and relief; scaled response. | Command Center (Gen); Defense Crawler (TT, mobile response role) |
| `UN-SEC-11` | **City Police Checkpoint** | Garrison and inspection barrier. | Pillbox (RA1); Guard Tower (TD); Bunker (Gen) |
| `UN-SEC-12` | **Federal SAM Emplacement** | Defend airspace/airfields against incursion. | SAM Site (RA1); Flak Cannon (RA2); AA Battery (TW) |
| `UN-SEC-13` | **JNA Reinforced Bunker** | Protect military supplies and power facilities. | Bunker (Gen); Battle Bunker (YR); Guardian Cannon (TW) |
| `UN-SEC-14` | **Federal Protective Perimeter** | Gates, fences and authorized checkpoints. | Concrete Wall (TD); Gate (TS); Fortress Wall (RA3) |

## Cross-faction mechanical consistency and design review

| System | Concord | Directorate | Ascendancy | Meridian | Unaligned |
| --- | --- | --- | --- | --- | --- |
| Cheap starting scout | **Unarmed light plane** | **Rifle motorcycle** | **Unarmed light plane** | **Fast armed scout truck** | NPC patrols only |
| Conventional attached-charge sabotage | Later, **spies only** | Later, **spies only** | Later, **spies only** | **Early, eligible non-spies** | Investigates/responds only |
| Special spy gameplay | Psy-Ops and facility resource waste/self-destruction | Efficient theft and account diversion | Intimidation and abstract contamination | Limited ordinary intelligence, prioritizes sabotage/trading | Counterintelligence, decoys, false-flag reporting |
| Advanced propaganda | Radio/press/TV | Radio/press/TV | Radio/press/TV | **No** | Radio/press/TV, NPC-directed |
| Nuclear materials processing | **Yes** | **Yes** | **Conditional; first-bomb inputs only** | **Never** | **Never** |
| Build civilian nuclear power | **Yes** | **Yes** | **Never** | **Never** | **Never** |
| Manufacture own nuclear bomb | Long, costly | Long, costly | Conditional; first completed bomb wins | **Never** | **Never** |
| Nuclear delivery | Bomber | ICBM | No launch needed for own completed first bomb; captured delivery only when compatible | Compatible stolen completed weapon/delivery only | Never launches |
| Black Market nuclear sales | Forbidden | Forbidden | Forbidden | **Forbidden** | No such market |
| Build/playable roster | Yes | Yes | Yes | Yes | **No** |

### Suggested production model (not canon)

- *Common chassis families* (engine-managed) can support faction-specific textures/hulls/functions: `RifleSquad`, `RocketTeam`, `Engineer`, `APC`, `MainBattleTank`, `MissileArtillery`, `AA`, `AttackHelicopter`, `Fighter`, `Transport`, `NavalPatrol`, `LogisticsVehicle`, `Refinery`, `Power`, `Radar`, `Turret`.
- *Gameplay traits* should be data-driven rather than hard-coded as alternate visual skins: `Recon_OpeningAirUnarmed`, `Recon_OpeningMotorcycleRifle`, `Recon_OpeningScoutTruck`, `Spy_PsyOps`, `Spy_IntelTheftBonus`, `Spy_ResourceDiversion`, `Spy_Intimidation`, `Spy_FacilityContamination`, `Sabotage_EarlyQualifiedNonSpy`, `Sabotage_LateSpyOnly`, `BlackMarket_NonNuclear`, `NPC_FalseIntel`, `Nuclear_Bomber`, `Nuclear_ICBM`, `Nuclear_FirstBombWin`, `Nuclear_Forbidden`.
- *Ownership and acquisition* need explicit fields: `ownerFaction`, `controllingPlayerOrAI`, `sourceFaction`, `acquisitionMethod` (native, captured, stolen, auction, contract, scenario), `buildPermitted`, `requiresMapWater`, `requiresFuelType`, `hasMoraleOrDiscipline`, `jurisdiction`, `techUnlocks`. *Stolen equipment ownership must not rewrite its manufacturing permissions.*
- *Balance considerations*: keep the native opening roster lean, group upgrades instead of exploding prefab counts, restrict extreme heavy tanks / military vessels by map and cost, and put unusual prototypes or black-market imports behind narrow availability limits. Avoid mirroring C&C tech tiers mechanically: these are inspiration mappings only.
- *Test cases*: no foreign infantry receives opening sabotage by default; Meridian scout trucks without a separately specified qualification cannot attach charges; three player espionage systems do not overlap; Unaligned planted reports cannot directly reveal live fog-of-war truth; Meridian never refines nuclear material or offers nuclear auctions; Ascendancy nuclear completion instant-win requires all canonical gates; no one can select Unaligned in the skirmish lobby; refinery crude and refined fuel inventories remain separate.

## Factual basis and traceability

**POLITIKA canonical documents (GitHub, main branch consulted 27 October 2026):**

- [Faction descriptions](https://github.com/simon-gros/politika/tree/main/Factions) — Atlantic Concord, Red Directorate, Iron Ascendancy, Meridian Pact and Unaligned.
- [Concepts Outline](https://github.com/simon-gros/politika/blob/main/Docs/CONCEPTS_OUTLINE.md) — faction asymmetry, trading, Black Market, nuclear and neutral AI framing.
- [Story Outline](https://github.com/simon-gros/politika/blob/main/Docs/STORY_OUTLINE.md) — 1984–1989 alternate chronology, Tito mystery and narrative tensions.
- [Early Reconnaissance and Scouting](https://github.com/simon-gros/politika/blob/main/Docs/RECONNAISSANCE_AND_SCOUTING.md) — hard opening scout variants.
- [Explosive Sabotage](https://github.com/simon-gros/politika/blob/main/Docs/SABOTAGE_SYSTEM.md) — Meridian early eligible non-spies vs three rival later spy-only charges.
- [Resources and Strategic Economy](https://github.com/simon-gros/politika/blob/main/Docs/RESOURCE_ECONOMY.md) — fossil hierarchy, refining and faction differences.
- [Nuclear Ultimate Weapons](https://github.com/simon-gros/politika/blob/main/Docs/NUCLEAR_WEAPONS_SYSTEM.md) — nuclear capability matrix and victory restrictions.
- [Propaganda and Population Morale](https://github.com/simon-gros/politika/blob/main/Docs/PROPAGANDA_AND_MORALE.md) — medium-specific influence and Meridian exclusion.
- [Espionage and Intimidation](https://github.com/simon-gros/politika/blob/main/Docs/ESPIONAGE_AND_INTIMIDATION.md), [Unaligned Counterintelligence](https://github.com/simon-gros/politika/blob/main/Docs/UNALIGNED_COUNTERINTELLIGENCE.md), and [Match Structure](https://github.com/simon-gros/politika/blob/main/Docs/MATCH_STRUCTURE.md).

**Command & Conquer lineage corpus:** *Command & Conquer series — faction unit and building roster reference*, compiled 27 October 2026 from the twelve games requested. It includes its own source links to community arsenal indexes, game manuals and tech-tree references. The mapping here cites original *names and releases*, not inferred numerical stats. **Do not call this a verified list of every campaign prop or ship availability in the original titles**; some inspirations are special/map/mission actors and marked as such in the underlying research. New roster names and identities are original proposals, not historical JNA designations, official C&C names or licensed game content.

**Revision gate:** before elevating this proposal to locked design canon, review category overlaps, all source-name spelling against shipped game data, faction-specific costs and access, production prerequisites, UI discoverability, target platform constraints, art feasibility, and civilian/NPC escalation behavior.