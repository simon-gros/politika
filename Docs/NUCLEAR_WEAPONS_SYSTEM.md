# POLITIKA — Nuclear Ultimate Weapons System

**Status:** Core faction, progression, and victory rules established; resource balancing and technical implementation remain provisional  
**Setting:** Alternate-history Yugoslavia, 1984–1989  
**Related:** [Concepts Outline](CONCEPTS_OUTLINE.md) · [Story Outline](STORY_OUTLINE.md) · [Engine Outline](ENGINE_OUTLINE.md) · [UI Overview](POLITIKA_UI_OVERVIEW.md) · [Factions](../Factions/)

> **Design boundary:** Nuclear weapons, intelligence and scarce inputs are fictional, abstract RTS assets and states. This specification describes game mechanics, not real-world nuclear-weapons design, fabrication, handling or technical deployment.

## 1. Canonical distinction: two nuclear powers, one conditional producer

The **Atlantic Concord** and **Red Directorate** are the game's **two established nuclear powers**. Their ultimate weapons are **tactical nuclear bombs** produced after acquiring intelligence, **refining crude nuclear materials into a processed game resource**, and completing the expensive, very long production process.

**The Iron Ascendancy is NOT a nuclear power at the start of the game.** It has **no independent nuclear weapon, nuclear stockpile or already unlocked nuclear research**. Its **fully developed spies** must infiltrate a **nuclear research facility belonging to either the Atlantic Concord or the Red Directorate** and successfully acquire **nuclear-refining technology intelligence and nuclear-weapons intelligence**. It can then **refine crude nuclear materials for bomb production only**, but **cannot build nuclear power plants**, and may attempt to construct **its first nuclear bomb**, subject to scarce resources and the full, very long construction period.

**As soon as the Iron Ascendancy successfully finishes building its own nuclear bomb, the current game immediately ends and it wins.** No bomber, missile, launch command, deployment, explosion or target selection is necessary. This is a unique production-triggered instant-victory condition, not a bonus to damage or a normal nuclear attack.

The **Meridian Pact cannot refine nuclear materials at all**, and has no nuclear research, nuclear power-plant construction or weapon-production capability. Both the Meridian Pact and Iron Ascendancy can, however, **sabotage, seize or steal an already completed nuclear weapon** and may deploy it on the map if they control a compatible means of delivery. **Theft is not production**: capturing and deploying a finished enemy bomb does not make either faction a nuclear producer, and never satisfies the Iron Ascendancy's special instant-win condition.

The **Unaligned** is a **non-nuclear, NPC-only** Yugoslav world authority, not an additional playable nuclear power.

## 2. Faction capability matrix

| Faction | Nuclear status at match start | May produce own bomb? | Steal/sabotage a completed bomb? | May deploy? | Victory/delivery rule |
|---|---|---|---|---|---|
| **Atlantic Concord** | Established nuclear power | **Yes** — intelligence + scarce resources + long construction | General site capture/defense subject to scenario rules | **Yes** | Deliver completed weapon with a **nuclear bomber airplane**; building it does not automatically win |
| **Red Directorate** | Established nuclear power | **Yes** — intelligence + scarce resources + long construction | General site capture/defense subject to scenario rules | **Yes** | Deliver completed weapon using an **ICBM**; building it does not automatically win |
| **Iron Ascendancy** | **Non-nuclear** | **Only after its espionage progression is fully developed, and its spies infiltrate a Concord or Directorate nuclear research facility and obtain both refining and weapons intelligence**; then full build requirements apply | **Yes** — signature espionage/sabotage/theft | **Yes, if it steals an existing ready bomb and compatible delivery capability** | **Immediate game victory on successful completion of its own first bomb, with no deployment required** |
| **Meridian Pact** | Non-nuclear | **Never** | **Yes** — signature sabotage/theft | **Yes, with a stolen ready bomb and compatible delivery capability** | No nuclear research tree, nuclear production or instant-build victory; no dedicated nuclear story campaign |
| **The Unaligned** | Non-nuclear NPC | **Never** | Responsive security AI only | No player nuclear attack | Remains autonomous neutral Yugoslav authority |

**Terminology:** the Concord and Directorate are the **two existing nuclear powers**; the Ascendancy is the **third faction capable of manufacturing a nuclear weapon only after espionage**, and completing its first bomb makes it the winner. The Meridian Pact is the other non-nuclear playable faction, but may operate a stolen bomb. Thus **ability to capture/deploy** does not equal **nuclear-power status or ability to produce**.

## 3. Nuclear refining and production gate

**Crude nuclear materials must first be refined at an eligible facility** into an abstract processed nuclear-material game resource. Only processed materials can be used to construct nuclear power plants or nuclear weapons.

**Atlantic Concord and Red Directorate:** can refine nuclear material and build **nuclear power plants** and rare tactical nuclear weapons. **Iron Ascendancy:** starts without refining capability; it must steal **refining-technology intelligence from a Concord or Directorate nuclear research facility**, as well as weapons intelligence. After those unlocks, it may refine materials **for weapons only**, not power plants; it **cannot construct and is not interested in nuclear power plants**. **Meridian Pact:** cannot refine crude nuclear materials at all. The Unaligned remains a non-nuclear NPC world authority.

**Abstract game chain:** `CrudeNuclearMaterial → RefinedNuclearMaterial → NuclearPowerPlant or NuclearBomb`; the **Ascendancy has only the bomb endpoint**, and Meridian has neither endpoint. Electricity generated from completed Concord/Directorate nuclear power plants feeds the game's power network. These are game-state transformations, not physical refining instructions. See [Resource Economy](RESOURCE_ECONOMY.md).



Every locally manufactured nuclear bomb requires:

1. **Two intelligence unlocks:** nuclear-refining technology **and** nuclear-weapons production intelligence. Concord and Directorate may already possess these in eligible scenarios; the Ascendancy must **first fully develop its spy progression** and then steal both through verified infiltration of an **actual Concord or Directorate nuclear research facility**.
2. **Scarce strategic resources.** Finite, expensive, abstract RTS resource commitments (quantities and names determined during balancing).
3. **An eligible production facility.** A controllable, secured game structure with necessary inputs and player-faction permissions.
4. **Very long, vulnerable gameplay-time construction.** An interruptible build that forces players to defend their facility while rivals can investigate, raid, or sabotage it.
5. **Actual completed bomb.** Intelligence, partial build progress and stored resources are not equivalent to ownership of a finished weapon.

**Common progression:** `Refinement and weapon intelligence validated → Crude nuclear materials acquired → Refined nuclear materials produced → Long bomb construction → Bomb completed`.

**Branch on completion:**

- **Atlantic Concord:** completed bomb enters custody; **nuclear bomber** deployment becomes available if eligible.
- **Red Directorate:** completed bomb enters custody; **ICBM** deployment becomes available if eligible.
- **Iron Ascendancy:** completion of its **own first bomb**, with the qualifying Concord/Directorate research-facility infiltration record, triggers **instant victory** and ends the match **before any deployment step**.

There must be no alternate Ascendancy starting blueprint, preexisting arsenal, generic research purchase, Black Market unlock or scripted shortcut bypassing the mandatory enemy-facility spy objective.

### Iron Ascendancy's full espionage prerequisite

The Iron Ascendancy begins with **more limited espionage** than either the Atlantic Concord or Red Directorate. Their respective special intelligence features are faction-specific, while the Ascendancy must **fully develop its spy progression to its maximum eligible level before it can infiltrate a nuclear research facility**. A basic or partially upgraded Ascendancy spy cannot qualify, regardless of proximity, disguise or local ownership.

Its required path is **full spy development → successful spy infiltration of an Atlantic Concord or Red Directorate nuclear research facility → theft of nuclear-refining technology and bomb-production intelligence → refining crude nuclear materials for weapons only → full, lengthy construction → immediate Ascendancy match victory**. Its unique **Intimidation** ability can convert isolated hostile units, but does **not** substitute for espionage development or nuclear research intelligence.

See [Espionage and Intimidation](ESPIONAGE_AND_INTIMIDATION.md).

## 4. Spy infiltration and intelligence provenance

The Iron Ascendancy's special path is uniquely asymmetric. An Ascendancy-controlled spy with **fully developed faction espionage** must penetrate a **nuclear research facility owned by the Atlantic Concord or Red Directorate**, acquire both **nuclear-refining technology intelligence** and **nuclear-weapons production intelligence**, and have those events independently recognized by match authority. One qualified facility can contain both required pieces of intelligence.

A regular barracks, arbitrary archive, empty research lab, another Ascendancy site or Meridian Pact merchant location **does not qualify**. An Ascendancy spy without **fully upgraded espionage capabilities** cannot begin the qualifying nuclear infiltration even at a valid facility. Merely spotting a research complex also does not qualify. The intelligence record should contain **source faction, eligible facility, spy acquisition event, verified completion, and owning player/match identity**.

The Concord and Directorate can defend their nuclear research sites through standard RTS mechanics, giving them a chance to prevent the Ascendancy's instant-victory progression before its costly, lengthy build begins.

**In-world distinction:** this is the Ascendancy's attempt to acquire its **first** nuclear capability, not a contest over another item added to an existing Ascendancy arsenal.

## 5. Deployment of completed weapons

Nuclear ultimate weapons are **actual map-deployable assets** when a game/match permits a finished weapon to be used.

- **Atlantic Concord:** a completed owned bomb is delivered via a **nuclear bomber aircraft**. Aircraft deployment, interception, readiness and targeting are part of conventional RTS presentation and balance.
- **Red Directorate:** a completed owned bomb is delivered using an **ICBM**. Launch availability, warning and counterplay belong to the abstract RTS ruleset.
- **Meridian Pact:** cannot create a bomb, but can deploy an **already-finished bomb** it successfully captured if it also controls a usable, compatible deployment capability. Exact compatibility rules remain under design.
- **Iron Ascendancy:** can likewise deploy an **already-finished stolen enemy bomb** with compatible delivery; **the unique own-built bomb never needs to be deployed because its completion ends the match immediately**.

A deployment is an authored high-impact RTS event with gameplay-defined map consequences and counterplay. Exact attack radius, damage, warnings and aftermath remain balance/visual design questions, not realistic nuclear-weapon specifications.

## 6. Sabotage, theft and inventory integrity

Both the **Meridian Pact** and **Iron Ascendancy** explicitly support sabotage or theft of an enemy nuclear weapon. A weapon eligible for theft must already be **completed**; capturing a production facility that has not yet finished a bomb is a different objective.

**Theft of a finished bomb:** transfers authoritative custody of one finite asset, subject to interception, control, security and compatible delivery rules. It does **not** copy technology, satisfy intelligence acquisition or create an unlimited production supply.

**Sabotage:** can interrupt production, damage or disable a protected nuclear asset, or disrupt its supporting infrastructure through normal RTS interaction. It does not create a new bomb and does not confer instant victory.

**Black Market restriction:** nuclear bombs and the nuclear-research unlock are **never purchasable, auctionable or brokered as Meridian Pact Black Market goods**. Direct in-world theft remains separate from legal or illicit commerce.

The Unaligned AI responds to territorial incursions, damaged infrastructure, smuggling, security incidents and deployments without becoming a playable nuclear faction.

## 7. Iron Ascendancy instant-victory predicate

Evaluate the condition from **authoritative simulation and mission state**, not a UI animation:

1. Active player is **Iron Ascendancy**.
2. The Ascendancy had **fully developed its espionage capabilities**, and one of its spies **successfully infiltrated an Atlantic Concord or Red Directorate nuclear research facility**.
3. The spy acquired and validated **both nuclear-refining technology intelligence and nuclear-weapons intelligence**.
4. Using both unlocks, the required **processed nuclear-material game resources**, and the full long build, the Iron Ascendancy **successfully completed production of its own first nuclear bomb**.
5. **Immediately resolve WIN for the Iron Ascendancy and GAME OVER for the current match.** No launch, delivery, detonation, or additional turn is required.

Not sufficient: seeing a facility, possessing generic intelligence, entering a non-qualifying building, stealing an already-completed enemy bomb, possessing a captured weapon, sabotaging a facility, collecting resources, beginning production, or firing a stolen weapon.

**Design invariant:** `InstantWin = IronPlayer && SpyProgressionComplete && QualifiedResearchInfiltration && ValidNuclearRefinementIntel && ValidNuclearWeaponIntel && OwnFirstBombProductionCompleted`.

The event must be atomic and happen at the **production completion event**. If other victory conditions are met on the same simulation tick, apply the deterministic winner-resolution policy of the match framework. In story campaigns, **game over** refers to the active game/match or mission, not erasure of the canonical 1989 revelation and Yugoslavia's collapse.

## 8. Secondary storyline versus sandbox gameplay

**Primary story:** Tito is assassinated in 1984, concealed through doubles, and exposed in 1989; Yugoslavia disintegrates into war.

**Secondary story:** the **Atlantic Concord and Red Directorate**, who already possess nuclear capabilities, attempt to prevent the **non-nuclear Iron Ascendancy** from gaining its **first bomb** through espionage and production. Ascendancy victory is the instant result of **own bomb completion after the mandated facility infiltration**.

**Meridian Pact:** has **no dedicated nuclear side-story mission chain or nuclear research**. Its mercenaries and units may nonetheless **sabotage, steal or deploy** an already finished bomb in normal RTS gameplay; the absence of story participation does not disable that gameplay capability.

**Skirmish and optional multiplayer:** use the same rules when nuclear ultimate weapons are enabled. Maps must include an accessible (though defensible) **Concord or Directorate nuclear research facility** if an Ascendancy player is expected to unlock the special victory condition. If not, explicitly communicate that the condition is unavailable in that ruleset. No fictitious pre-armed Ascendancy nuclear starting state.

**Black Market:** conventional exotic weapons and services only; not nuclear bombs or nuclear intelligence unlocks.

## 9. Engine and UI responsibilities

The system should maintain distinct, inspectable authoritative states:

- **Faction capability:** existing nuclear power (Concord/Directorate only), conditional production permission (Ascendancy), no-production (Meridian), NPC neutrality (Unaligned).
- **Spy progression:** per-faction level/max level, Ascendancy full-development completion and qualifying infiltration permission; **Intimidation** is separate from nuclear tech acquisition.
- **Intelligence state:** distinct `HasNuclearRefinementIntel` and `HasNuclearWeaponIntel`, plus `IntelSourceFaction`, `NuclearResearchFacilityID`, `SpyInfiltrationVerified`; Ascendancy unlock only for Concord/Directorate research sites.
- **Scarce-resource state:** separate crude/processed nuclear inventories, authorized nuclear-refinement jobs, output custody, resource commitments and consumption.
- **Build state:** `NuclearBuildInProgress`, `NuclearBuildCompleted`, remaining time, current owner/producer and producing facility.
- **Finished-asset state:** unique bomb ID, original producer, present custodian, readiness, stolen status and compatible delivery capability.
- **Victory state:** explicit `IronOwnBombCompleted` event checked together with the mandatory spy-intelligence provenance.

Present appropriate prerequisites, resource scarcity, long production progress, facility warning, theft/sabotage feedback, completed-bomb inventory, bomber/ICBM readiness, deployment interfaces and immediate Ascendancy victory messaging. A **stolen bomb** should be visibly distinct from **own completed production**.

## 10. Open balancing and authoring choices

- Exact fictional scarce resource names, costs and build durations.
- Intel unlock availability for Concord/Directorate in individual scenarios.
- Facility infiltration difficulty, counterintelligence and attack/defense pacing.
- Faction-compatible delivery for stolen bombs, particularly the Meridian Pact.
- The effects of an on-map tactical nuclear deployment and suitable gameplay counterplay.
- How full-length canonical campaign episodes handle the Ascendancy instant victory without contradicting the fixed historical-fiction ending.

## 11. Locked summary

**Starting nuclear powers:** **Atlantic Concord and Red Directorate only**.  
**Conditional third producer:** **Iron Ascendancy**, initially non-nuclear, unlocked **only after fully developing spying and then** achieving successful spy infiltration of a **Concord or Directorate nuclear research facility** and acquisition of its nuclear intelligence.  
**Production completion:** verified refining and weapons intelligence + **refined nuclear materials** + **very long build time** + finished bomb.  
**Iron Ascendancy win:** **immediate game over on completion of its own first bomb; no deployment required**.  
**Concord delivery:** **nuclear bomber**.  
**Directorate delivery:** **ICBM**.  
**Designated theft/sabotage factions:** **Meridian Pact and Iron Ascendancy**; both may **deploy a captured existing bomb** with compatible delivery.  
**Non-producer:** **Meridian Pact**, even after successful theft.  
**NPC world:** **Unaligned**.  
**Lore continuity:** primary Tito conspiracy and 1989 collapse remain canonical.
