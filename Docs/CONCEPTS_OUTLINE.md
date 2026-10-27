# POLITIKA — Concepts Outline

**Status:** Foundational game-design concepts; mechanics, balance values, and implementation details are proposals unless explicitly marked as core principles  
**Setting:** Alternate-history Yugoslavia, 1984–1989  
**Genre:** Real-time strategy, factional conflict, political conspiracy, nuclear deterrence, intelligence, logistics, and asymmetric warfare  
**Purpose:** Define the core playable systems and relationships underpinning POLITIKA, distinct from the chronological [Story Outline](STORY_OUTLINE.md).  
**Related material:** [Story Outline](STORY_OUTLINE.md) · [Nuclear Ultimate Weapons System](NUCLEAR_WEAPONS_SYSTEM.md) · [Engine Outline](ENGINE_OUTLINE.md) · [UI Overview](POLITIKA_UI_OVERVIEW.md) · [Faction descriptions](../Factions)

> **Design boundary:** This document distinguishes **established premises** from **suggested implementations**. It is not an assertion that these gameplay systems are already coded. The assassination and its concealment are *fictional alternate-history events*: historically, Tito died in 1980.

## 1. Overall Concept

POLITIKA is an asymmetric RTS set inside a politically charged and ostensibly independent Yugoslav federation. The country is not simply a neutral map on which external factions happen to fight: it is a populated, administered, surveilled, economically active, militarily defended world with a standing authority. Players must operate *inside* this shared system, respecting, exploiting, infiltrating, corrupting, or provoking it.

Five factions exist, but their responsibilities are deliberately unequal:

| Faction | Player control | Principal role | Strategic identity |
|---|---|---|---|
| **The Unaligned** | **Never playable; system-controlled** | Yugoslav state, military, police, civilians, institutions, and ordinary world order | Neutral world authority, with a concealed coercive security apparatus |
| **The Atlantic Concord** | Playable | One of the **three principal rivals** | Advanced conventional forces, intelligence, precision, strategic access |
| **The Red Directorate** | Playable | One of the **three principal rivals** | Organized mass forces, industrial support, entrenchment, influence networks |
| **The Iron Ascendancy** | Playable | One of the **three principal rivals** | Covert aggression, sabotage, coercion, deniable deployments; uniquely constrained by widespread hostility |
| **The Meridian Pact** | Playable | **Specialist fourth playable faction**, also a world-facing service provider | Merchants, mercenaries, smuggling, rare equipment, Black Market intermediary |

**Fundamental structure:** one AI-governed neutral federation; three primary nuclear-armed military-political antagonists; and one non-nuclear, playable but commercially oriented mercenary network. All four playable factions remain capable of completing campaigns or matches, though not by identical strategic means.

## 2. The Central Secret as a Game-System Concept

The principal story mystery is also a systemic organizing principle: in **1984**, an Iron Ascendancy operation murders Tito using poisoned food. Yugoslav officials conceal his death and sustain public continuity through a **body double for controlled appearances** and a **separate voice double for official recordings**. The core conspiracy is known only to a small inner circle of the Unaligned. Public exposure in **1989** is fixed in the canonical story and precipitates a broader political collapse and war, in combination with pre-existing crises.

The concept should affect play **without immediately telling players the truth**:

- **Information is fragmented.** Archives, intercepted signals, unexplained security zones, witnesses, propaganda inconsistencies, and physical evidence each reveal partial information.
- **Not all knowledge is equivalent.** Rumor, suspicion, verified evidence, and publicly credible proof should be distinct states.
- **Evidence has custody and exposure risk.** Holding a document, moving a witness, broadcasting a recording, and persuading an audience are different tasks.
- **The Unaligned protects continuity.** Routine security AI may investigate intrusions, while a small hidden command layer escalates selectively when the conspiracy is threatened.
- **The player can change local outcomes, not the canonical historical endpoint.** Campaign actions influence the survival of witnesses, control over evidence, diplomatic consequences, tactical victories, and accountability; they do not avert the eventual revelation in 1989.

**Design proposal — intelligence/evidence system:** associate evidence objects with provenance, authenticity, discoverability, custody, secrecy, and corroboration. A *disclosure threshold* would require several independent credible sources rather than a single collectible. Campaign scripts may gate major revelations; skirmish may omit the narrative layer or use a generic intelligence objective.

**Design constraint:** the poisoning itself is a narrative event and **not** a reproducible poisoning tutorial or weapon-production mechanic.

### 2.1 Secondary nuclear-weapons contest and ultimate weapons

**Established side-story premise:** the Iron Ascendancy seeks to obtain nuclear-weapons intelligence and create an **own-produced nuclear weapon** or seize an already completed one. The Atlantic Concord and Red Directorate attempt to stop the acquisition. All three are nuclear-capable powers in the fictional setting; however, **Ascendancy ultimate-weapon production begins locked** until its operatives infiltrate an **enemy facility containing nuclear-weapons intelligence** and acquire that intelligence.

**Established RTS ultimate-weapon system:** the Atlantic Concord, Red Directorate, and Iron Ascendancy can each ultimately **produce a tactical nuclear weapon**. Production requires **scarce abstract strategic resources**, **nuclear-weapons intelligence**, an eligible facility, and a **very long production period** ending in a distinct **completed weapon**. Once produced, the Concord deploys through a **nuclear bomber aircraft** and the Directorate through an **ICBM**; the Ascendancy's own delivery platform is to be defined. A weapon is a rare **deployable high-impact ultimate asset**, not merely a narrative collectible.

**Iron Ascendancy instant-victory condition:** **infiltrate an eligible enemy intelligence-bearing facility → obtain nuclear-weapons intelligence → commit resources and complete production of an own nuclear weapon → immediately win the active game/match**. Capturing or firing a stolen finished weapon does **not** satisfy this special victory condition. As a nuclear-capable geopolitical power it may have an existing strategic deterrent in the lore, but that does not bypass its required **gameplay unlock** for construction and special victory.

The **Meridian Pact is the only non-nuclear playable power**, unable to independently **produce** nuclear weapons or unlock nuclear manufacturing by stealing intelligence. **Both the Meridian Pact and Iron Ascendancy can sabotage, steal, and deploy an already existing completed nuclear weapon**; theft grants custody of a finite asset and requires a compatible usable deployment method. The Pact remains **outside the dedicated nuclear side-story campaign** and has no nuclear production tree, even though opportunistic sabotage and theft are valid RTS actions. Nuclear weapons may **never be purchased or auctioned on the Black Market**.

The **Unaligned remains non-nuclear and NPC-only**; its world AI may protect sites, investigate theft, and respond to nuclear use. The primary assassination-cover-up plot and canonical 1989 revelation remain fixed, whereas the nuclear ultimate-weapon mechanics govern match objectives and victory conditions. See the full [Nuclear Ultimate Weapons System](NUCLEAR_WEAPONS_SYSTEM.md) for the state machine, delivery distinctions, capture rules, and technical boundaries.

## 3. The Unaligned: The World's Neutral Operating Authority

The Unaligned is not simply another computer opponent, a fifth player slot, or a faction permanently at war with every intruder. It is the **anonymous, ambient governing framework** of the entire game world: the Yugoslav federal administration, Yugoslav People's Army, Territorial Defence, law enforcement, transport authorities, publicly operated infrastructure, local governments, and ordinary civilians.

Its units are initially **neutral to the Atlantic Concord, Red Directorate, and Meridian Pact**, unless those factions violate local rules or create hostile conditions. **The Iron Ascendancy is the exceptional adversary**: its known personnel and exposed operations are subject to default hostility because the faction is recognized as an extremist destabilizing force. Concealed Ascendancy agents may temporarily pass as civilians or otherwise unrecognized actors until detected.

This distinction is important: **the Unaligned's units are neutral actors, not ownerless units that any player can command.** Neutral means that they follow system AI and their own institutional orders, not that they can be freely recruited, looted, or attacked without consequences.

### 3.1 Ambient world simulation

The Unaligned can operate traffic, patrols, military convoys, civil services, stations, factories, markets, checkpoints, airfields, ports, media installations, and government buildings. Each exists for a plausible purpose even when no player is interacting with it. The federation should feel governed and lived in rather than like an empty competitive arena.

### 3.2 Layered AI rather than one omniscient enemy

1. **Civilian/economic AI:** commercial traffic, workers, public transport, logistics, essential services, and noncombatant reactions.
2. **Local authority AI:** police response, traffic control, inspection, municipal facilities, ordinary law enforcement.
3. **Federal military AI:** garrisons, border troops, convoy escorts, restricted installations, defensive mobilization.
4. **Covert continuity AI:** specifically authorized surveillance, document recovery, cover-up enforcement, witness suppression, and protection of the false Tito narrative.
5. **Crisis/fracture AI:** defections, conflicting orders, republican-level dissent, local autonomy, and eventual institutional breakdown.

Not every member of the Unaligned knows the assassination secret or endorses the inner circle's crimes. Ordinary neutral units must not behave like omniscient conspiracy enforcers.

### 3.3 Diplomatic, legal, and military state

Suggested relationship model, tracked separately for each player and the Unaligned:

| State | AI response | Example cause |
|---|---|---|
| Recognized/neutral | Monitor and permit ordinary passage where appropriate | Lawful movement, authorized trading |
| Suspicious | Inspect, shadow, warn, limit access | Unidentified weapons, clandestine approaches |
| Restricted | Deny entry, seize contraband, dispatch patrols | Entering a military zone or moving prohibited goods |
| Hostile | Engage detected combatants and defend installations | Attacking federal personnel, exposed Ascendancy units |
| Emergency | Reinforcements, curfews, roadblocks, intelligence operations | Major attacks or risks to state continuity |
| Fractured | Divergent regional and institutional orders | Late-campaign legitimacy collapse |

These are **behavior states**, not a mandatory six-step ladder. Jurisdiction, unit identity, disguise, local reputation, visibility, and direct aggression may alter the response. The Iron Ascendancy starts with an exceptional hostile baseline **when identified**, while other factions can also make themselves hostile through their actions. An alliance or trade relationship must not grant immunity from local law.

### 3.4 Neutrality as a gameplay resource

Neutral roads, bases, broadcasters, depots, hospitals, industry, and border crossings create strategic options. Players may seek access permits, escort contracts, safe passage, trade privileges, or intelligence cooperation. Attacking neutral infrastructure can provide short-term advantage but provoke reprisals, economic interruption, reputation loss, and civilian costs.

**Proposed rule:** neutral asset capture should require an explicit mechanic (political control, infiltration, occupation, or combat), not instant ownership merely because a player reaches it.

## 4. Three Principal Playable Factions: Distinct Armed Forces

The main military contest centers on three primary factions with meaningfully distinct **unit rosters, equipment, mobility, capabilities, production constraints, doctrine, and intelligence styles**. The difference cannot be merely cosmetic.

### 4.1 Atlantic Concord — precision and access

**Candidate identity:** elite, expensive professional formations; high situational awareness; advanced communications, reconnaissance, air support, and precision weapons. A small, coordinated force is effective when supported by intelligence and logistics, but costly to replace and vulnerable if deprived of information or supply access.

**Possible unit archetypes:** reconnaissance teams; professional infantry squads; electronic-warfare vehicles; fast armored transports; precision-fire platforms; surveillance aircraft; specialist signals teams.

**Potential limitations:** higher procurement/maintenance costs, dependence on surveillance and interoperable networks, slower replacement of elite losses, sensitivity to diplomatic legitimacy.

**Ultimate weapon:** long-produced tactical nuclear weapon delivered by a **nuclear bomber**, requiring nuclear intelligence and scarce strategic resources.

### 4.2 Red Directorate — mass and sustained operations

**Candidate identity:** organized combined-arms formations, robust industrial logistics, fortified defense, attritional combat, and ideologically linked local networks. It can contest territory at scale but may be less tactically agile or precise than the Concord in some circumstances.

**Possible unit archetypes:** line infantry; heavy armored formations; artillery; engineer companies; field logistics; mobile air defense; propaganda or political-organizing detachments.

**Potential limitations:** larger supply and movement footprint, slower redeployment, command rigidity, exposure to decentralized disruption.

**Ultimate weapon:** long-produced tactical nuclear weapon delivered via **ICBM**, requiring nuclear intelligence and scarce strategic resources.

### 4.3 Iron Ascendancy — infiltration and destabilization

**Candidate identity:** covert action, sabotage, disinformation, rapid clandestine strikes, hidden cells, intimidation, and exploitation of disorder. Its ideology and role as the assassination perpetrator give it **specific advantages but equally meaningful drawbacks**. It is **not** simply an objectively stronger faction or the only one capable of immoral conduct.

**Possible unit archetypes:** disguised operatives; saboteurs; clandestine couriers; infiltrators; light raiding groups; improvised or captured vehicles; counterintelligence cells; specialized shock troops.

**Advantages to test:** hidden deployment, false identities, surprise attacks, clandestine recruitment, disruption of enemy economies, propaganda manipulation, quick access to irregular forces.

**Limitations to test:** hostile standing with detected federal forces, fewer openly legal supply routes, restricted public procurement, limited access to official checkpoints and services, greater scrutiny, diplomatic isolation, weak conventional logistics, and destabilizing consequences for its own occupied territory.

**Unique nuclear victory:** obtain the nuclear intelligence by infiltrating an enemy intelligence-bearing facility, then complete construction of an own tactical nuclear weapon for **immediate match victory**. It may also sabotage and steal completed weapons, but stolen weapons do not trigger that victory. Its delivery platform remains an open design decision.

The identity of its forces should be legible to the player but not automatically known to enemy AI: **gameplay faction ownership** and **in-world recognized identity** are separate variables. Concealment is a bounded ability with countermeasures and costs, not invulnerability.

### 4.4 Unit diversity principle

Each primary faction should eventually receive unique infantry, vehicles, specialists, support units, buildings, technology unlocks, deployment animations, sound, and tactical abilities. Shared baseline categories may exist for balance, but no two factions should have identical command trees with different skins.

## 5. The Meridian Pact: Merchant, Mercenary, and Fourth Playable Power

The Meridian Pact occupies a different strategic niche. It is **playable**, including in its own scenarios and potentially in skirmishes, but its signature world function is supplying the others as a **foreign merchant, brokerage, and mercenary organization**. It is **non-nuclear and without a dedicated nuclear story arc**, but its units **can sabotage, steal, and deploy existing completed nuclear weapons**. It cannot produce any. It need not be morally or politically neutral in every deal: commercial flexibility is its defining behavior.

### 5.1 Two distinct roles

- **As a playable faction:** a player controls Pact personnel, trade routes, contracted forces, specialists, mobile infrastructure, and financial relationships; wins through specified military or economic objectives.
- **As a non-player service provider:** Pact traders, auctioneers, smugglers, and contract managers provide a shared market to other participants, controlled by system AI.

These modes should use compatible underlying economic and contractual rules but avoid circular control: **a player selecting Meridian Pact must not automatically gain omnipotent control of the neutral market used by other players.** The market can be operated by an independent Pact service layer, with transparent rules for pricing and access.

### 5.2 Mercenaries

The Pact can offer time-limited infantry, escorts, reconnaissance teams, armored support, logistical specialists, or other contracted services. Units need explicit **employer, legal owner, original affiliation, payment schedule, deployment conditions, contract expiry, and rules of engagement**.

Proposed contract mechanics include up-front fees, ongoing wages, duration, casualties, loyalty, extraction, reputation, equipment loss, and breach penalties. Contracted units fight for the employer while remaining identifiable as mercenary assets. A player-controlled Pact may hire others or earn income by supplying contracted forces; the system should prevent unlimited instant mercenary armies.

### 5.3 Commerce as military strategy

The Pact can prosper from scarcity, conflict, rare imports, border access, transport, and intermediary relationships. Its combat capacity may be limited by cash flow, market access, contract obligations, and the reliability of purchased assets rather than by the production doctrine of the main three factions.

**Balance requirement:** trading, contract fulfillment, and logistics must be viable routes to competitive strength, not a passive menu outside the RTS. However, purchasing every powerful item must not make native faction identities irrelevant.

## 6. The Black Market: Cross-Faction Auction House

**Core concept:** the Meridian Pact administers the **Black Market**, a special exchange where factions can buy **conventional weapons, vehicles, equipment, intelligence, rare components, specialist services, and other assets they cannot manufacture within their own production trees**. **Nuclear weapons are categorically excluded as Black Market listings:** the Pact cannot sell, auction, broker, or manufacture nuclear weapons through market services. Separately, **Meridian Pact military units can sabotage, seize, and deploy completed nuclear weapons obtained by in-world theft**, which is not a market transaction.

The Black Market is both a strategic economic system and a potential physical location or network of locations in the world. It should have a visible game-facing interface while remaining governed by availability, risk, money, delivery, and the wider simulation.

### 6.1 Trading structure

Possible listing types:

- **Fixed-price sale:** purchase immediately if funds, access, and stock permit.
- **Timed auction:** competing bids with a defined close, settlement, and delivery.
- **Limited procurement contract:** order an item for future delivery with explicit conditions.
- **Mercenary contract:** recruit an individual, squad, specialist, or temporary support formation.
- **Intelligence listing:** acquire a lead, map data, interception, false documentation, or partial evidence, without guaranteeing it is trustworthy.

A minimal first version should launch with fixed-price listings and explicit stock. Timed bidding and more complex markets can follow only after the simpler economy works.

### 6.2 Economic principles

- Purchased items must come from **finite or rule-governed stock**, not unrestricted free spawning.
- Listings have faction access rules, condition/quality, rarity, price or starting bid, expiration, location, and delivery requirements.
- Prices can respond to supply, demand, embargoes, convoy losses, territorial access, and campaign events.
- Procurement is separate from **deployment**: buying a vehicle need not teleport it onto the battlefield.
- Smuggling, customs checks, seizure, theft, interception, and convoy protection make the marketplace tactically consequential.
- Trading visibility can be partial: other players may know that an auction occurred without automatically knowing the winner's identity.
- Funds, purchased inventory, transfers, losses, and cancellations must be authoritative and auditable in multiplayer.

### 6.3 The uniqueness rule

An **exotic** or market-only asset is unavailable for normal faction production, even if a main faction has the money to buy the same item on the market. Conversely, the existence of the Black Market should not let factions buy another faction's entire unique production tree without restriction.

**Suggested balance levers:** low supply, lengthy shipping, variable condition, replacement-part scarcity, contractor loyalty, specialist operation requirements, hard caps, reputation restrictions, and exposure risk.

### 6.4 Market neutrality and Iron Ascendancy

The Pact may sell to the Iron Ascendancy clandestinely, but the Unaligned's hostility makes open delivery into federal jurisdiction dangerous. The auction house itself is **not** automatically a safe haven or exempt from security enforcement. Market access, identity concealment, secure delivery, and interception are meaningful decisions rather than guarantees.

## 7. Faction Relationships: Allegiance, Identity, and Conduct

POLITIKA should model at least four distinct concepts rather than folding them into a single friend-or-foe flag:

| Concept | Question it answers |
|---|---|
| **Actual ownership** | Who controls this unit, structure, cargo, or contract? |
| **Recognized identity** | Who does a given observer believe controls it? |
| **Diplomatic standing** | What relationship exists between the recognized parties? |
| **Local conduct/threat** | What has the unit done in this jurisdiction, and what is it doing now? |

Additional state can include permits, disguises, wanted status, trade access, warrants, military authorization, mercenary employer, and local reputation. A system-controlled neutral unit may therefore ignore a Concord patrol, inspect a Directorate convoy, escort a Meridian shipment, and engage an identified Ascendancy saboteur on the same map.

**Suggested default:** Unaligned units respond defensively and proportionately according to their AI mandate, rather than instantly engaging every adjacent object. Some security zones can enforce shoot-on-sight rules regardless of nominal diplomatic neutrality.

## 8. Proposed Core Gameplay Loop

The distinctive loop is not merely **build base → produce army → destroy enemy base**. It combines conventional RTS control with living-world constraints:

1. **Establish presence:** deploy forces or a command element and secure legal, clandestine, or militarily controlled access.
2. **Acquire information:** scout terrain, identify threats, investigate activity, and uncover faction-specific intelligence.
3. **Obtain resources:** produce, trade, contract, seize, or transport supplies; manage costs and routes.
4. **Exercise power:** maneuver armies, capture positions, protect infrastructure, sabotage targets, or negotiate arrangements.
5. **Manage consequences:** react to neutral enforcement, diplomatic changes, supply disruption, insurgency, suspicion, or retaliation.
6. **Complete objectives:** achieve military, intelligence, economic, or covert victory conditions.

The weight of each action varies by mode and faction. A battle-focused skirmish can emphasize production and conquest; a story mission can emphasize evidence and escape; a Meridian scenario can emphasize contracts, delivery, and financial control.

## 9. Strategic Systems Worth Prototyping

### 9.1 Production and procurement

Primary factions should manufacture or train their normal roster using faction-specific structures, unlocks, and resources. Unaligned assets are managed by the world AI. Meridian assets are a mixture of its own deployable force, purchased stock, and mercenary contracts. Define unit **origin**, **current controller**, and **production eligibility** independently.

### 9.2 Supply and movement

Roads, rail, bridges, ports, airfields, depots, border crossings, and checkpoints should matter. Supply routes are attractive targets and may depend on neutral authorities. A purchased Black Market weapon may require escort through an Unaligned checkpoint before it can be fielded.

### 9.3 Intelligence, concealment, and counterintelligence

Fog of war, line of sight, sensors, impersonation, document forgery as an abstract ability, surveillance, informants, intercepted communications, and evidence custody can generate decisions without replacing ordinary combat. Detection should be a calculable contest with counterplay, not arbitrary script-driven omniscience.

### 9.4 Legitimacy, fear, and reputation

Suggested reputation dimensions: federal standing, regional standing, market reliability, civilian harm, visibility, and political credibility. These should have mechanical consequences such as access restrictions, public compliance, rebellion, recruitment, prices, and checkpoint attention. **No single moral score** should decide every faction's behavior.

### 9.5 Territory and infrastructure

Consider differentiated control: military occupation, lawful ownership, clandestine influence, commercial access, and intelligence penetration. A player can influence an industrial complex or broadcaster without necessarily capturing every square meter around it.

### 9.6 Civilians and collateral consequences

Civilians and noncombatant infrastructure make the neutral federation credible. Attacks on civilians must not become a trivial resource-generation shortcut. Civilian losses, service outages, displacement, and institutional stress should feed campaign and strategic consequences.

### 9.7 Nuclear ultimate-weapon production, theft, and victory

**Three production-capable factions:** Concord, Directorate, and Ascendancy require a specific intelligence unlock, **scarce abstract resources**, expensive facilities, and **very long game-time production** before receiving a completed tactical nuclear weapon. Ownership of intelligence and ownership of a ready weapon are separate states. Production can be interrupted by ordinary RTS combat or sabotage.

**Faction delivery:** the **Red Directorate uses an ICBM**; the **Atlantic Concord uses a nuclear bomber aircraft**. The Ascendancy's delivery platform is not yet specified. The Meridian Pact cannot produce its own weapon but, like the Ascendancy, can **sabotage, steal and deploy** a completed enemy weapon once a compatible delivery capability is controlled.

**Ascendancy alternate victory:** a successful infiltration of an **enemy facility with nuclear-weapons intelligence** unlocks its nuclear production. **Completion of its own weapon immediately wins the match**; stealing a completed device, starting production, or merely acquiring intelligence does not trigger victory. These conditions must be authoritative, inspectable, and balanced against a long, resource-intensive construction period.

The Pact **does not receive a dedicated nuclear campaign storyline** despite its optional theft and use of captured weapons. Nuclear weapons and production intelligence are **not ordinary Black Market purchases**. The Unaligned's neutral AI may respond to sabotage, stolen cargo, and deployment. This system should use abstract game states, not real-world construction specifications. See [Nuclear Ultimate Weapons System](NUCLEAR_WEAPONS_SYSTEM.md).

## 10. Game Modes and Canon

**Campaign:** linked 1984–1989 missions, faction-specific perspectives, continuity of evidence, and a fixed core secret and final revelation. A **separate nuclear-weapon intelligence, production, acquisition/interdiction chain** is reserved for the Atlantic Concord, Red Directorate, and Iron Ascendancy. The Ascendancy's qualifying production completion wins the active game; the Meridian Pact receives no dedicated nuclear missions despite its general ability to steal a ready weapon. The Iron Ascendancy may have privileged early knowledge within its assassination cell, but its ordinary commanders should still operate under partial information.

**Skirmish:** a self-contained match in which the three principal factions and optionally the Meridian Pact compete; the Unaligned populates and governs the map. Nuclear weapons are **late-game, resource-intensive ultimate weapons** when the selected ruleset enables them; they are never market purchases. The Ascendancy's infiltration-then-own-production completion triggers an immediate win. The storyline's canonical 1989 outcome does not need to occur in every match.

**Multiplayer (proposed scope):** mirror the skirmish rules with authoritative simulation, synchronized economy and auctions, clear faction identity and mercenary ownership. Decide whether Meridian must be present as a player, a service AI, or either; the Black Market should not disappear merely because no human selects Meridian.

**Scenario/editor play (proposed):** allow missions to configure Unaligned alert levels, institutional jurisdictions, available market stock, faction permissions, forbidden goods, evidence objectives, and victory rules. Expose data-driven nuclear intel access, costly/long production, completed-weapon custody, capture/sabotage, deployable delivery platforms, and the Ascendancy instant-win condition under enabled rules. Use abstract fiction-only resources and no real-world engineering model.

## 11. Asymmetry and Fairness

The factions need not be equivalent in every situation, but each playable faction must have **multiple meaningful ways to win**. In particular:

- Neutral Unaligned military strength is a world constraint, **not** a free army for the other playable sides.
- The Ascendancy's systemic hostility must be balanced by covert and asymmetric capability; it should not face constant automatic destruction from omniscient AI.
- The Meridian Pact's wealth and special procurement access must be balanced by the cost, time, vulnerability, and finite supply of imported assets. Stealing and deploying a completed nuclear weapon must be difficult and never grant its own production capacity.
- Concord precision and Directorate mass should each have counters and logistical tradeoffs; the Concord's nuclear-bomber approach and Directorate's ICBM delivery should remain distinct and counterable.
- Concealment, market purchases, and mercenary hiring must expose meaningful signals or counterplay to opponents. Scarce resources, nuclear intelligence, long production, detection of theft, and the Ascendancy's instant-win race must allow meaningful warning/counterplay.
- Diplomatic and civilian systems must affect matches without letting random neutral incidents override players' strategic agency.

## 12. Initial Vertical Slice

A practical first playable prototype should prove the **five-faction structure** on one compact Yugoslav map before attempting the full campaign:

1. A neutral Unaligned police patrol, military guard, civilian traffic lane, and guarded checkpoint, controlled solely by world AI.
2. One representative combat squad and one specialist or support unit each for Atlantic Concord, Red Directorate, and Iron Ascendancy.
3. Iron Ascendancy recognized as hostile at the checkpoint, with a testable covert/disguise alternative and a detection countermeasure.
4. One Meridian Pact mercenary squad available under a time-limited contract.
5. One Black Market kiosk/interface with a finite exotic item listing, purchase, delivery, and usable inventory.
6. A small resource loop and one tactical objective that all playable factions can complete in different ways.
7. An Unaligned escalation test: neutral passage → inspection → restriction → armed response after documented player conduct.
8. Debug views for actual ownership, recognized identity, standing, contract expiry, market inventory, suspicion, and AI orders.

**Acceptance:** the world AI behaves neutrally by default, the Ascendancy's special status functions without omniscient cheating, all four playable factions have a valid command loop, and Black Market items and mercenaries obey stock, payment, delivery, and ownership rules.

## 13. Open Design Decisions

Before implementing full rosters and economy, resolve:

- Whether match economies prioritize base construction, mobile deployment, territorial income, trade, or a hybrid.
- Whether infantry is controlled as individual soldiers, squads, or both.
- How far legal authorization, permits, customs, and disguise are simulated rather than scripted.
- Whether all four playable factions are permitted in every competitive ruleset.
- Whether mercenaries count toward population cap and how contracts expire during combat.
- Who supplies Black Market stock (excluding nuclear weapons), whether human Meridian players can list their own goods, and how anti-exploit settlement works.
- Balance values for scarce abstract nuclear production resources and timers, nuclear asset security and capture, the Ascendancy's production victory, and bomber/ICBM counterplay.
- The Iron Ascendancy's own nuclear delivery platform and the Meridian Pact's compatible deployment method for stolen weapons.
- Narrative placement and outcome of the Iron Ascendancy nuclear intelligence acquisition and the three-power interdiction race.
- Whether Unaligned institutions can fracture into multiple AI-controlled authorities in late-campaign missions.
- Whether political victory, economic victory, and intelligence victory exist outside authored missions.
- What initial map scale, unit counts, match duration, resources, and performance targets are realistic.

## 14. Core Principles to Preserve

1. **The assassination conspiracy anchors the primary plot; a nuclear-intelligence/weapon acquisition and prevention race is the secondary plot, while nuclear ultimate weapons also function in RTS gameplay.**
2. **The Unaligned is a non-playable, generally neutral, living governing environment—not a conventional fifth competitor.**
3. **The Atlantic Concord, Red Directorate, and Iron Ascendancy are the three nuclear-capable powers with producible nuclear ultimate weapons; the Iron Ascendancy uniquely wins immediately upon qualifying own production completion.**
4. **The Iron Ascendancy has distinctive covert advantages and systemic hostility/disadvantages when identified.**
5. **The Meridian Pact is a non-producing, non-nuclear playable merchant/mercenary power with no dedicated nuclear story or nuclear Black Market sales, but can sabotage, steal and deploy completed nuclear weapons.**
6. **Conventional exotic equipment uses the finite Black Market; nuclear weapons instead require intelligence, scarce resources and long production, or in-world theft of an existing completed weapon.**
7. **Identity, diplomacy, hostility, ownership, and employment contracts remain separate gameplay concepts.**
8. **Faction asymmetry is intentional, but viable counterplay and strategic agency are mandatory.**
9. **The 1984–1989 campaign canon and the open-ended skirmish rule set must coexist without contradiction.**
10. **Systems in this outline remain design concepts until implemented, tested, and explicitly promoted to completed features.**
