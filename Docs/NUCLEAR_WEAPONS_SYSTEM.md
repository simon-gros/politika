# POLITIKA — Nuclear Ultimate Weapons System

**Status:** Established faction and victory rules; resource names, timing, balance, animations, target rules, and implementation are design work in progress  
**Setting:** Alternate-history 1984–1989 Yugoslavia  
**Related:** [Concepts Outline](CONCEPTS_OUTLINE.md) · [Story Outline](STORY_OUTLINE.md) · [Engine Outline](ENGINE_OUTLINE.md) · [UI Overview](POLITIKA_UI_OVERVIEW.md) · [Factions](../Factions)

> **Scope:** This is a fictional real-time strategy game system. Nuclear research, materials, intelligence, construction, transfer, and deployment are represented by **abstract game states and resources**, not technical weapons instructions or real-world construction specifications.

## 1. Canonical Rule

The **Atlantic Concord**, **Red Directorate**, and **Iron Ascendancy** are the three nuclear-capable playable factions. Each can ultimately **produce a tactical nuclear weapon as its faction's ultimate weapon**, provided all gameplay prerequisites are met. Such weapons are extremely costly, require **scarce strategic resources**, **nuclear-weapons intelligence**, and **a completed, lengthy production process**, and can then be **deployed on the map**.

**The Meridian Pact cannot manufacture a nuclear weapon.** It can nevertheless **sabotage, seize, steal, and deploy an already-existing nuclear weapon**, subject to its ability to control and use the captured asset. The Iron Ascendancy can likewise sabotage, steal, or deploy an existing weapon.

**Unique Iron Ascendancy victory rule:** if its forces **infiltrate an enemy facility holding nuclear-weapons intelligence**, successfully acquire that intelligence, and **complete production of their own nuclear weapon**, the Iron Ascendancy **immediately wins the active game/match**. Theft or capture of an already-finished enemy weapon **does not** qualify as building its own weapon and therefore **does not** trigger this automatic victory. Sabotage alone also does not trigger it.

The primary storyline remains Tito's concealed assassination and the 1989 exposure. The secondary storyline concerns attempts to **prevent the Iron Ascendancy from completing nuclear acquisition or independent production**, while the Pact remains outside that story as a strategic priority even though its units may opportunistically steal or sabotage nuclear assets during gameplay.

## 2. Faction Capabilities

| Faction | Produce own nuclear weapon | Sabotage or steal existing weapon | Deploy completed weapon | Special victory / canonical delivery |
|---|---|---|---|---|
| **Atlantic Concord** | **Yes**, with research/intelligence, scarce resources, and long production | General capture/sabotage subject to mission rules; not its signature mechanic | **Yes** | **Nuclear-capable bomber aircraft** delivers its own weapon; no automatic construction victory |
| **Red Directorate** | **Yes**, with research/intelligence, scarce resources, and long production | General capture/sabotage subject to mission rules; not its signature mechanic | **Yes** | **ICBM** delivery for its own weapon; no automatic construction victory |
| **Iron Ascendancy** | **Yes**, **locked until it steals nuclear intelligence by infiltrating an enemy intelligence-bearing facility**; requires scarce resources and long production | **Yes**, special infiltration, sabotage, and theft role | **Yes**, including stolen complete weapons | **Instant win immediately upon completion of its *own* weapon after successful infiltration** |
| **Meridian Pact** | **No**, under any circumstances | **Yes**, can sabotage and steal a pre-existing completed weapon | **Yes**, if it controls a compatible usable weapon and delivery method | No nuclear production tree, no automatic nuclear victory, no dedicated nuclear story missions |
| **The Unaligned** | **No**; NPC-only | Responds to theft/attacks through security AI; not a playable theft faction | Not a player nuclear faction | Neutral administrative, territorial, and security authority |

**Important distinction:** nuclear-capable **faction identity** does not imply that a fully usable weapon, production intelligence, specialist resources, and delivery assets are automatically available on every map. The Iron Ascendancy is classified as a nuclear-capable power in the setting but must unlock **the player-controlled production path** through the specified infiltration-and-intelligence condition.

The user's term **“tactical nuclear weapon”** is the game's ultimate-weapon category; the Red Directorate's **ICBM delivery system** is a long-range strategic-style delivery platform. Weapon yield, radius, flight characteristics, and whether the terminology should be revised are presentation/balance decisions, not real-world technical claims.

## 3. Prerequisites and Progression

A locally produced nuclear weapon requires all of the following independent gates:

1. **Nuclear-weapons intelligence acquired:** a persistent faction-specific information or technology unlock. It is not equivalent to merely spotting an enemy nuclear site. The Ascendancy must acquire its unlock from an **infiltrated enemy intelligence-bearing facility**.
2. **Scarce strategic game resources:** expensive, finite/contested abstract resources committed to the project. Their names and quantities are to be balanced, with **no real-world nuclear material processing or design details**.
3. **Eligible protected production site:** an expensive production capability with accessible supply, power, and faction permissions. Its precise building/tree identity is to be designed.
4. **Long production timer:** a multi-stage, interruptible game process. Completion only occurs after required resources and production time have been satisfied. The exact duration is intentionally unspecified.
5. **Completed weapon asset:** a distinct controllable inventory/world-state object, held at a facility or ready for an authorized deployment method. An intelligence unlock or partially completed weapon is **not** a nuclear weapon that can be deployed.
6. **Available deployment capability:** valid faction/platform rules, world position, range/target permissions, and a player-issued deployment order. Concord uses a **nuclear bomber**; Directorate uses an **ICBM**. Iron and stolen-weapon delivery compatibility require later design decisions.

**Production state model:** `Locked → Intelligence acquired → Resources committed → Production underway → Completed weapon in custody → Delivery assigned → Deployed / lost / seized / disabled`.

These states should remain separate: a weapon under construction cannot be stolen and fired as if it were completed; sabotage of production facilities can interrupt or damage projects, while theft of an existing completed weapon transfers custody subject to game rules.

## 4. Nuclear-Weapons Intelligence and Facility Infiltration

Intelligence is a **gameplay resource and authorization gate**. Reconnaissance can reveal that a site is strategically sensitive, but the required intelligence must be **acquired** through a mission-specific interaction or a completed information objective.

For the Iron Ascendancy, **infiltration of a hostile facility that contains nuclear-weapons intelligence** is mandatory for the special production unlock. The game should validate that the site really contains the required intelligence; infiltrating an ordinary base, an empty laboratory, or a non-nuclear archive does not satisfy the gate.

The Concord and Directorate also need nuclear-weapons intelligence to produce their ultimate weapon. Their initial campaign or match setup can determine whether they possess the appropriate unlock or must earn it. They do **not** gain the Ascendancy's instant-win condition.

A stolen *completed weapon* is independent of a stolen intelligence file. Capturing a weapon does not automatically grant permanent production knowledge.

## 5. Deployment and Weapons on the Map

The completed nuclear weapon functions as a rare **ultimate RTS attack asset**, not merely a plot prop.

- **Atlantic Concord:** assigns an eligible nuclear bomber to deliver its own completed weapon. Aircraft readiness, targeting, path exposure, and interception can matter as ordinary game mechanics.
- **Red Directorate:** deploys its own completed weapon using an **ICBM**. The interface should expose a launch-ready platform and a valid target in normal RTS terms rather than a technical missile-control simulation.
- **Iron Ascendancy:** can deploy a completed weapon it owns or steals. **Its canonical delivery platform is not specified yet**; do not silently reuse either the Directorate's ICBM or the Concord's bomber.
- **Meridian Pact:** can deploy a stolen **existing completed weapon** once it also controls a compatible means of delivery. **It has no ability to manufacture a weapon or unlock the production tree by stealing intelligence.** Its specific compatible delivery gameplay remains to be defined.

A successful nuclear deployment is an authored match event with significant destructive and geopolitical consequences, subject to ordinary gameplay targeting, visibility, defense, and consequence rules. The actual tactical radius, damage model, fallout representation, warning cadence, counterplay, and platform compatibility must be playtested before locking balance. Avoid real-world device arming, design, or targeting instructions.

Deployment **does not by itself trigger the Iron Ascendancy's special instant victory** unless the separately defined *own-weapon production-completion* condition was already satisfied (in which case the match ends at completion).

## 6. Theft, Sabotage, and Custody

The **Iron Ascendancy and Meridian Pact** both have intentionally supported mechanics for:

- **Sabotage:** temporarily disabling or damaging enemy nuclear production, storage, or delivery infrastructure, with consequences driven by standard damage/repair and security systems.
- **Theft/seizure:** capturing custody of an **already existing, completed nuclear weapon** through an authored infiltration, capture, or transport interaction.
- **Deployment of stolen weapons:** using the captured completed weapon once valid control and platform conditions are fulfilled.

**Theft never becomes production.** A stolen completed weapon is a finite asset with provenance, owner/controller, custody, readiness, and compatibility attributes. It is not an unlimited inventory unlock, and cannot be bought through the Meridian Pact's Black Market or converted into a reproducible nuclear technology tree.

Ordinary neutral facilities and Unaligned forces can respond to trespass, theft, sabotage, or dangerous deployments under their existing sovereignty/alert systems. These responses do not turn the Unaligned into a nuclear power.

## 7. Iron Ascendancy Automatic Victory

This is a **distinct victory condition**, not merely a conventional high-damage strike.

**All required conditions:**

1. The active player is **the Iron Ascendancy**.
2. Its forces have **successfully infiltrated an enemy facility carrying nuclear-weapons intelligence**.
3. The intelligence has been **acquired and recognized as the production unlock**.
4. The player has paid the necessary scarce-resource costs and **completed construction/production of its own nuclear weapon**.

**On completion:** the authoritative match rule resolves the Iron Ascendancy as **winner immediately**, without requiring transport, launch, detonation, additional attack orders, or survival of a delivery platform.

**Non-qualifying actions:** merely stealing a finished weapon, occupying a launch facility, obtaining unverified rumors, sabotaging a rival project, starting (but not finishing) production, buying ordinary weapons, or firing a captured warhead. Do not trigger the special victory from these states.

If multiple victory conditions resolve in one simulation tick, use a deterministic arbitration rule defined by the match framework. In authored campaign missions, this condition resolves the current playable mission/match and its debrief but **does not overwrite the larger canonical 1989 revelation and war**.

## 8. Story and Mode Integration

**Primary campaign story:** the 1984 assassination and cover-up, exposure in 1989, and Yugoslavia's descent into war remain fixed.

**Secondary campaign story:** Concord and Directorate missions attempt to prevent the Ascendancy from acquiring the intelligence, production capability, or already completed weapon needed to secure a nuclear advantage. Ascendancy missions develop the reverse pursuit and may culminate in the **own-weapon production instant victory**. Because the core story has a fixed ending, the mission-writing team will align the outcome and timing of this secondary confrontation with canonical events.

**Meridian Pact:** remains disinterested in the **secondary story as a campaign objective**. Its general skirmish and tactical gameplay can nevertheless **sabotage or steal and deploy** an existing weapon opportunistically; this does **not** create a Meridian nuclear campaign or independent weapons industry.

**Skirmish and future multiplayer:** expose these ultimate-weapon rules when the chosen match ruleset enables them. The Ascendancy's unique instant-win rule is integral to that ruleset and must be visible before match start. A map with no eligible enemy intelligence facility should not pretend the Ascendancy can unlock its condition: level setup must provide a fair route or explicitly disable the nuclear-victory ruleset.

**Black Market:** conventional/exotic arms only. The nuclear system is outside trade and mercenary-auction transactions.

## 9. UI and Technical Contracts

Implement as **authoritative, data-driven gameplay**, not decorative HUD timers. At minimum, the systems should represent:

- **Faction flags:** `CanProduceNuclear`, `CanStealCompletedNuclear`, `CanDeployCapturedNuclear`, delivery platform types, and `OwnNuclearProductionInstantVictory`.
- **Intel record:** `HasNuclearProductionIntel`, source facility, verified acquisition, and campaign/match scope.
- **Construction record:** unique project ID, resource commitments, completed/remaining production progress, interruption state, producing faction, and facility.
- **Completed weapon record:** unique asset ID, origin/producer, legal owner, current controller/custodian, storage/transport state, readiness, and delivery compatibility.
- **Victory-event evidence:** a completed own-produced weapon and the qualifying enemy-facility infiltration/intelligence record, resolved atomically by match authority.
- **Presentation:** locked/unlocked prerequisites, abstract scarce-resource costs, long-build progress, custody/theft/sabotage feedback, valid deployment controls, delivery readiness, and unmistakable Ascendancy victory warning.

The match authority must enforce eligibility and atomic completion/capture/victory transitions, with serializable state for local saves and replay. UI should not make its own claims of ownership or progress. Fictional numeric balance values and visual effects remain open until prototyped.

## 10. Open Design Decisions

- Exactly which **abstract strategic resources**, infrastructure, and game-time construction durations balance nuclear production.
- How Concord/Directorate nuclear intelligence is obtained or initially granted by scenario, and what protects enemy intelligence-bearing facilities from trivial infiltration.
- The Iron Ascendancy's **own nuclear deployment platform** and any captured-platform requirements.
- How stolen completed weapons become deployable for the Meridian Pact without giving it a nuclear weapons factory, tech tree, or full nuclear campaign.
- Whether nuclear strike effects use area damage, regional infrastructure damage, persistent hazards, or a simplified visual/rules treatment.
- Whether nuclear weapons may be built more than once in a long match, and how to cap spamming while preserving their “ultimate” role.
- Counterplay, early-warning opportunities, construction visibility to enemies, cooldown/readiness, and interaction with protected civilian areas.
- Treatment of a nuclear production or deployment attempt in story missions where the canonical ending must remain fixed.

## 11. Non-Negotiable Summary

**Three producers:** Atlantic Concord, Red Directorate, and Iron Ascendancy.  
**Two designated thieves/saboteurs:** Meridian Pact and Iron Ascendancy.  
**One non-producer:** Meridian Pact, even when it captures and deploys a finished weapon.  
**One unique production-victory faction:** Iron Ascendancy, after enemy-facility infiltration and own-weapon completion.  
**Two specified delivery platforms:** Red Directorate **ICBM**; Atlantic Concord **nuclear bomber**.  
**Production gates:** rare abstract resources + nuclear intelligence + long construction + completed weapon.  
**Story separation:** Tito conspiracy is primary; prevention of Ascendancy acquisition is secondary; Meridian may steal nuclear assets through general RTS mechanics without participating in the nuclear campaign plot.
