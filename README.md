<img src="https://d2ol7oe51mr4n9.cloudfront.net/user_3K3YY250wbFw8kz81nbtkGhNhkg/16fb7d8d-420c-49b0-9a05-b06db0a2f1a1.png" alt="POLITIKA game banner" width="100%">

# POLITIKA

An alternate-history real-time strategy game set in Yugoslavia, 1984–1989.

Inspired by the original *Command & Conquer* games, POLITIKA combines asymmetric military strategy, Cold War espionage, political intrigue, nuclear brinkmanship, and competing ideologies in a fictionalized 1980s Yugoslavia. Three rival geopolitical powers struggle for influence, a fourth playable faction brokers weapons and mercenaries, and the Yugoslav federation itself operates as a persistent, AI-controlled authority.

The country is more than a battlefield. Its cities, industries, transport networks, armed forces, civilians, borders, and institutions form a living political environment. Players must decide when to cooperate with that environment, when to operate secretly within it, and when the consequences of open conflict are worth the cost.

> Project status: POLITIKA is in its foundational design and documentation stage. The faction identities, central story premise, and overarching design principles are established; unit rosters, balance, detailed economic rules, and many gameplay systems remain proposals rather than implemented features. This README describes the intended game, not a currently playable release.

## The setting

POLITIKA takes place in an alternate-history Socialist Federal Republic of Yugoslavia during the final years of the Cold War. The federation stands at the intersection of rival political and economic systems, yet is neither an Atlantic satellite nor a subordinate of the Soviet-style bloc. Its distinctive combination of socialist self-management, federal institutions, and nonalignment makes it both strategically valuable and difficult for outside powers to control.

The campaign spans 1984–1989, moving through intelligence operations, border incidents, infrastructure crises, political disputes, and escalating military confrontation. The world draws on Yugoslavia's republics and autonomous provinces, the Yugoslav People's Army (JNA), Territorial Defence, federal and local administration, economic institutions, and the complex international relationships of the period.

This is fiction, not a claim about real history. Josip Broz Tito died on 4 May 1980 in reality. In POLITIKA's timeline, he survives beyond that date. What happens to him in 1984—and what Yugoslav authorities subsequently conceal—drives the central conspiracy.

## Five factions, different roles

POLITIKA has four playable factions, but they do not all serve the same strategic function. The Atlantic Concord, Red Directorate, and Iron Ascendancy are the three principal military-political rivals; of these, only the Atlantic Concord and Red Directorate begin as nuclear powers. The Meridian Pact is a fourth, non-nuclear and commercially oriented playable faction that also supplies the wider world with goods and mercenaries. The Unaligned is never player-controlled: it is the AI-governed Yugoslav state and the neutral institutional framework within which the other factions act.

| Faction | Role | Strategic character |
|---|---|---|
| <img src="Factions/TheAtlanticConcord/1logo.png" alt="Atlantic Concord insignia" width="52"> [The Atlantic Concord](Factions/TheAtlanticConcord/description.md) | Principal playable rival | Western-inspired parliamentary and market-oriented coalition; intelligence, reconnaissance, professional forces, advanced technology, air support, and precision operations. High-value forces are costly and logistically demanding, but the Concord has modestly cheaper fuel refining. |
| <img src="Factions/TheRedDirectorate/2logo.png" alt="Red Directorate insignia" width="52"> [The Red Directorate](Factions/TheRedDirectorate/description.md) | Principal playable rival | Soviet-inspired centralized socialist power; industrial mobilization, armored formations, artillery, engineering, air defense, and sustained large-scale operations. Infantry production is markedly cheaper than for every other faction, enabling massed forces, though logistical and command constraints remain. |
| <img src="Factions/TheIronAscendancy/3logo.png" alt="Iron Ascendancy insignia" width="52"> [The Iron Ascendancy](Factions/TheIronAscendancy/description.md) | Principal playable rival | Fictional fascist and supremacist authoritarian power; clandestine cells, infiltration, sabotage, coercion, and rapid destabilizing attacks. Its units have a slight advantage in raw destructive power and sometimes carry out high-risk, self-sacrificing attacks to maximize damage, at significant losses. Its ideology also produces diplomatic isolation, restricted access, and hostility from identified Yugoslav authorities. |
| <img src="Factions/TheMeridianPact/4logo.png" alt="Meridian Pact insignia" width="52"> [The Meridian Pact](Factions/TheMeridianPact/description.md) | Specialist playable faction; merchant and mercenary network | Diverse, postcolonial and strategically independent coalition with mobile forces and international trading networks. Specializes in imports, mercenaries, the Black Market, cheap and fast early scout-combat trucks, the game's strongest conventional sabotage, and starting near fossil-fuel fields. |
| <img src="Factions/TheUnaligned/5logo.png" alt="Unaligned insignia" width="52"> [The Unaligned](Factions/TheUnaligned/description.md) | NPC / system AI only | The Yugoslav federation: civil administration, JNA, Territorial Defence, police, border control, infrastructure, and population. Generally neutral to other factions, but able to investigate, restrict, defend, and intervene. |

Nuclear balance: only the Atlantic Concord and Red Directorate start as nuclear powers. Both can manufacture exceptionally expensive, slow-produced tactical nuclear bombs when nuclear intelligence, the necessary nuclear-material game resources, and facilities are available; Concord delivers with a nuclear bomber, Directorate with an ICBM. The Iron Ascendancy starts without nuclear weapons. Its spies must infiltrate a Concord or Directorate nuclear research facility, obtain the required intelligence and construct its first atomic bomb using the scarce nuclear materials. The instant that first self-produced bomb is completed, the game ends in an Iron Ascendancy victory—no deployment is required. Both the Ascendancy and the Meridian Pact may also sabotage, steal and deploy already completed weapons; stolen bombs never trigger the Ascendancy's production victory, and the Pact cannot manufacture them.

Resource hierarchy and faction economy: Nuclear materials are the most valuable resources, followed by fossil fuels, while electricity and other industrial inputs support production. Crude oil must be processed in refinery facilities to yield petroleum fuel or kerosene; the Atlantic Concord has a modest fossil-fuel refining cost advantage, supporting its slightly cheaper fuel economy. Crude nuclear materials also require refining before use in nuclear power plants or weapons. The Atlantic Concord and Red Directorate can refine these materials and construct nuclear power plants and bombs. The Meridian Pact cannot refine nuclear materials at all. The Iron Ascendancy must steal both nuclear-refining and bomb-production intelligence from a Concord or Directorate nuclear research facility, then may refine for its first bomb only, never for nuclear power plants; completing its first bomb ends the game immediately in its victory. Directorate infantry remains substantially cheaper than any other faction's, Ascendancy units have slightly greater damage and occasional high-risk attack tactics, and Meridian always starts near a crude-oil or natural-gas field. See [Resource Economy](Docs/RESOURCE_ECONOMY.md) and [Nuclear Ultimate Weapons System](Docs/NUCLEAR_WEAPONS_SYSTEM.md).

Espionage and coercion: Atlantic Concord specializes in Psy-Ops, which can make an eligible enemy unit self-destruct after breaking its Discipline (combat) or Morale (civilian/non-combat), or temporarily attack its own allies in a short rampage. Red Directorate agents can steal rival intelligence slightly faster and more cheaply through basic infiltration. The Iron Ascendancy has more limited spying and must fully develop its espionage abilities before its operatives can infiltrate an enemy Concord or Directorate nuclear research facility and steal the intelligence needed to refine nuclear materials and build its first bomb. Instead, the Ascendancy alone possesses Intimidation: it can force isolated eligible enemy units to change sides and join its army. No other faction can use this power, and intimidation does not replace nuclear espionage. Unlike Concord Psy-Ops, Intimidation permanently converts the eligible isolated, fully broken unit rather than eliminating it or briefly disrupting its targeting. See [Espionage and Intimidation](Docs/ESPIONAGE_AND_INTIMIDATION.md).

Faction differences are intended to extend beyond visual identity. Each playable power should have its own units, technologies, military doctrine, production constraints, intelligence capabilities, logistical needs, and strategic weaknesses. Asymmetry must create different viable approaches to victory, rather than one faction simply having stronger versions of another's units.

### Facility espionage and industrial disruption

The three main military factions have distinct ways to target enemy buildings. Atlantic Concord Psy-Ops can force an eligible building to self-destruct or quietly waste resources; the wasted supplies disappear rather than becoming Concord income. Red Directorate spies can instead divert resources from an infiltrated rival facility or budget into the Directorate's own accounts. Either financial effect may go unattributed or unnoticed until the victim investigates.

Iron Ascendancy espionage can place eligible enemy facilities under a fictional poisoning/disease disruption effect. Workers, researchers or other assigned staff may die, depleting the workforce and slowing or stopping production; a bounded chain can affect other nearby eligible facilities. Counterintelligence, containment, staffing replacement and recovery can stop the losses. This is an abstract RTS building-status system, not a representation of actual chemical or biological techniques. These abilities do not bypass the established espionage upgrade requirements for the Ascendancy's first nuclear weapon. See [Espionage and Intimidation](Docs/ESPIONAGE_AND_INTIMIDATION.md) and [Resource Economy](Docs/RESOURCE_ECONOMY.md).

### Early reconnaissance and conventional sabotage

Each playable faction has an affordable early scout with a distinct role. The Meridian Pact can mass-produce cheap, very fast, lightly armored and lightly armed trucks that scout and fight lightly; these make it the strongest overall reconnaissance faction. Atlantic Concord and Iron Ascendancy deploy inexpensive unarmed light scouting planes, while Red Directorate uses motorbikes armed with basic infantry rifles.

Meridian Pact also has the game's strongest conventional explosive sabotage, a separate advantage from its reconnaissance and commercial activities. Eligible Pact units—including qualified non-spy units—can undertake cheap, early-game sabotaging of enemy vehicles and buildings through an abstract attached-charge explosion. The other three playable factions unlock similar conventional explosive sabotage later, at greater cost, and only for spy units. This is different from the Concord's Psy-Ops, Directorate's covert resource theft and Ascendancy's fictional facility-contamination and Intimidation effects. See [Reconnaissance and Scouting](Docs/RECONNAISSANCE_AND_SCOUTING.md) and [Explosive Sabotage](Docs/SABOTAGE_SYSTEM.md).

### The Unaligned: the world is not unclaimed territory

The Unaligned is neither a conventional fifth opponent nor a pool of ownerless neutral units. Its forces follow their own institutional orders, and its roads, factories, checkpoints, ports, airfields, broadcasters, and military installations exist independently of player activity.

Atlantic Concord, Red Directorate, and Meridian Pact forces are ordinarily treated as neutral unless their conduct, location, or circumstances justify a response. Identified Iron Ascendancy personnel are an exception: the Unaligned treats this extremist destabilizing faction as hostile. Hidden operatives may evade identification temporarily, but detection, suspicion, and counterintelligence provide counterplay.

Player behavior can change these relationships. Unauthorized military movement might lead to inspections or restricted access; an attack on Yugoslav personnel or public infrastructure could trigger security operations or an armed response. Federal, republican, local, military, civilian, and covert security bodies need not react identically, particularly as the campaign's political crisis deepens.

Crucially, the vast majority of ordinary Unaligned personnel do not know the central conspiracy. A narrow covert inner circle acts to preserve state continuity while other institutions continue their normal duties.

### The Meridian Pact: commerce is a form of power

The Meridian Pact remains fully playable, but its signature function differs from the three principal rival powers. It operates as a foreign trading, procurement, and mercenary intermediary. Pact-controlled services can also appear as AI-run world facilities even when no player chooses the faction.

Its defining institution is the Black Market: an exchange for rare or exotic conventional weapons, vehicles, components, intelligence, equipment, and specialist services that individual factions cannot normally produce themselves. The planned system supports finite stock, prices or auctions, access conditions, and physical delivery rather than unlimited instant purchases. Nuclear weapons are not offered or brokered through the Black Market; no Meridian player or service NPC participates in the nuclear acquisition plot.

The Pact can also provide mercenary contracts for temporary squads, escorts, specialists, and other support. Contracts introduce duration, payment, ownership, loyalty, and deployment constraints. Hiring a mercenary does not turn that unit into a permanently produced asset, and purchasing rare equipment does not erase the distinct identity of each faction's native roster.

A player-controlled Meridian Pact should not automatically control every neutral merchant or auction listing. Its playable economy and its shared market-service role are designed to coexist without giving one player unrestricted command of the market.

### Propaganda and population morale

Every faction has a population morale meter, including the Meridian Pact and the NPC-controlled Unaligned. Higher morale improves production output and military discipline. Every individual unit uses one shared resolve meter: the UI calls it Discipline for combat units and Morale for civilian/non-combat units. These are the same mechanic, not two different meters; Population Morale is its faction-wide summary. The Atlantic Concord, Red Directorate, Iron Ascendancy and Unaligned have distinct advanced propaganda methods, relying principally on radio, newspapers and television. The Meridian Pact has no advanced propaganda capabilities, but still benefits from morale generated by ordinary economic, social and military conditions. Broadcast infrastructure, printing facilities and media control create gameplay objectives, while propaganda and morale cannot bypass nuclear-resource, research or faction eligibility gates. Combat Discipline normally becomes more stable in supported formations; Atlantic Concord has no solo/small-group Discipline penalty, and Red Directorate and Iron Ascendancy alone gain an extra Discipline bonus for very massive coherent armies. Iron Ascendancy Intimidation can force an isolated target to switch sides only after breaking its Discipline if combatant or its Morale if civilian/non-combat. See [Propaganda and Morale](Docs/PROPAGANDA_AND_MORALE.md).

## Core gameplay direction

POLITIKA retains the immediate tactical foundations of a classic RTS—selection, movement, combat, positioning, reconnaissance, and control of strategic locations—while extending the significance of diplomacy, information, commerce, and the surrounding society.

The foundational concepts call for:

- Asymmetric armies: faction-specific infantry, vehicles, support formations, abilities, production options, and countermeasures—including Concord's modest refinery cost discount, Directorate's substantially cheaper infantry, Ascendancy's slight destructive-power advantage and occasionally self-sacrificing attacks, and Meridian's guaranteed nearby fossil-fuel fields.
- A functioning neutral world: AI-controlled Yugoslav military units, police, civilians, logistics, services, protected facilities, and jurisdiction-dependent responses.
- Identity and political standing: actual ownership, visible or disguised identity, diplomatic relationships, and hostile conduct are separate conditions. An unidentified infiltrator is not necessarily perceived as its true faction.
- Intelligence and secrecy: reconnaissance, interception, surveillance and evidence; Concord Psy-Ops can trigger resolve breakdown/self-destruction or a short-lived friendly-fire rampage, while Directorate basic infiltration steals rival intelligence slightly faster and cheaper. Ascendancy spying is more limited and must be fully upgraded for nuclear infiltration.
- Exclusive Intimidation: Iron Ascendancy can intimidate eligible isolated combat and non-combat units; it must fully break the target's shared resolve meter (shown as Discipline on soldiers, Morale on civilians) before conversion. No other faction has the ability.
- Meaningful infrastructure and logistics: roads, bridges, railways, depots, ports, airfields, border crossings, delivery routes, and the consequences of disrupting them.
- Trade and mercenary systems: imported, non-producible equipment acquired through the Black Market; finite supply and contracts create strategic choices and interception risks.
- Consequential intervention: cooperation, occupation, sabotage, civilian harm, restrictions, reputation, and political escalation can change what a faction can do next.
- Multiple objective types: military engagements can coexist with intelligence recovery, witness protection, clandestine extraction, procurement, infrastructure defense, and other mission-specific goals.
- Secondary nuclear race and ultimate weapons: Atlantic Concord and Red Directorate can manufacture rare, slow-built tactical nuclear weapons. Iron Ascendancy starts non-nuclear and must fully upgrade its espionage progression, infiltrate a Concord or Directorate nuclear research facility, steal both nuclear-refinement and bomb-production intelligence, refine scarce nuclear materials, and finish constructing its first bomb. Its completion immediately wins the game without deployment. Iron Ascendancy and Meridian Pact can separately steal and deploy an existing finished weapon; neither may purchase nuclear bombs on the Black Market.

The intended strategic rhythm is to establish a presence, obtain information and resources, act through military or political means, manage the resulting consequences, and complete objectives. Which parts dominate depends on the faction, mission, and game mode. The detailed construction, production, economy, and unit-cap rules have not yet been finalized.

See the [Concepts Outline](Docs/CONCEPTS_OUTLINE.md) for the proposed faction interactions, Black Market mechanics, AI behaviors, initial playable prototype, and open design questions.

## Two interwoven storylines spanning five years

The primary campaign storyline is built around a single, exceptionally sensitive political secret. What first appears to be an assortment of military incidents and Cold War intelligence disputes gradually reveals a hidden struggle over the legitimacy and survival of the Yugoslav federation.

The four playable factions encounter different portions of the truth, act on incomplete information, and face conflicting incentives. The Iron Ascendancy has particular knowledge of the original crime; the other factions follow separate diplomatic, military, economic, and intelligence leads. The Unaligned attempts to maintain public order while an inner security apparatus protects a secret whose exposure could destroy the legitimacy of the state itself.

Alongside the main narrative, the secondary storyline follows the non-nuclear Iron Ascendancy's effort to develop its first atomic bomb, opposed by the nuclear-armed Concord and Directorate. Ascendancy spies must first fully develop their espionage progression, then infiltrate a Concord or Directorate nuclear research facility and steal both nuclear-refining technology and weapons intelligence; only then can it refine nuclear materials for bomb production and begin a very long, resource-intensive build. It cannot construct nuclear power plants. The Ascendancy wins the active game instantly on completion, with no deployment needed. The Meridian Pact has no dedicated nuclear campaign but can opportunistically steal and deploy a completed enemy bomb as a normal RTS mechanic.

The planned campaigns follow a shared 1984–1989 chronology. Player choices shape tactical outcomes, the survival of witnesses, the custody of evidence, faction influence, and the consequences of individual operations. They do not rewrite the foundational mystery or its canonical conclusion.

<details>
<summary>Central narrative premise and ending — major spoilers</summary>

In 1984, agents of the Iron Ascendancy assassinate Tito by arranging for his food to be poisoned. A small group within the Unaligned's highest security apparatus conceals the killing to prevent a perceived crisis of national unity.

A body double replaces Tito during tightly controlled public appearances, while a separate voice double supplies radio broadcasts, recordings, and propaganda. Neither double actually governs: a clandestine circle issues decisions in Tito's name. Officials and witnesses who approach the truth may be intimidated, detained, discredited, or silenced.

The assassination cell and the Yugoslav cover-up are distinct conspiracies: the Iron Ascendancy commits the murder, but it does not control the federal decision to conceal it. Evidence survives through medical records, security logs, recordings, witnesses, and operational documentation.

In 1989, corroborating evidence finally becomes public. The revelation of the murder and years of deception destroys the credibility of the institutions that claimed to preserve the federation. Combined with existing economic, constitutional, political, and regional tensions, it precipitates Yugoslavia's disintegration into war in the game's canonical ending.

</details>

For the complete act-by-act chronology, principal character functions, and mission ideas, see the [Story Outline](Docs/STORY_OUTLINE.md) (contains full spoilers).

### Player and faction limits per map

Each player controls exactly one playable faction. Every ordinary map supports 2–4 player-faction slots, selected from the Atlantic Concord, Red Directorate, Iron Ascendancy and Meridian Pact, plus one always-present, independently AI-controlled Unaligned faction. In shorthand: 2+1, 3+1 or 4+1 faction presences; the Unaligned is never a fifth player slot. In offline single-player, AI opponents occupy the other playable positions. Standard matches use one distinct playable faction per player. See [Match Structure](Docs/MATCH_STRUCTURE.md).

## Planned modes and presentation

Campaign is a sequence of faction-specific missions within the common alternate-history narrative. The Concord and Directorate have prevention missions, and the initially non-nuclear Iron Ascendancy has infiltration/first-bomb production missions; the Meridian Pact has no dedicated nuclear side story. Storytelling takes place through gameplay, operational briefings, radio communication, intelligence reports, documents, objectives, and debriefings—not mandatory cinematics.

Skirmish is intended to support stand-alone battles with AI opponents and objectives independent of the campaign's fixed ending. The Unaligned continues to govern the map, while the three principal rivals—and the Meridian Pact when supported by the ruleset—pursue their own strategies.

Multiplayer and scenario/editor support remain potential future extensions, not confirmed first-release features. The immediate product requirement is an offline-first single-player experience: local campaign and skirmish should be accessible without a mandatory internet connection, external account, or sign-in.

## Technology and interface

POLITIKA's proposed technical foundation is Unreal Engine 5.8.3, with primarily C++ gameplay and RTS simulation systems, Unreal-native content assets, and a clear separation between authoritative game state and its visual presentation. Map authoring, faction definitions, unit data, AI, missions, and validation tools are intended to be maintainable and data-driven.

The interface is being designed for readable tactical control at 4K (3840 × 2160) while also scaling appropriately at lower resolutions. Its visual direction takes cues from 1980s Yugoslav topographic maps, field reports, military documentation, and civil-administration materials, without sacrificing clarity, accessibility, or command responsiveness.

These are architecture and product targets, not evidence that the corresponding engine systems already exist. The exact economy, multiplayer scope, performance targets, and several simulation decisions remain under design.

## Documentation and repository guide

| Location | What it contains |
|---|---|
| [Story Outline](Docs/STORY_OUTLINE.md) | Canonical assassination conspiracy, parallel nuclear-weapons storyline, 1984–1989 campaign acts, and ending (spoilers). |
| [Concepts Outline](Docs/CONCEPTS_OUTLINE.md) | Five-faction structure, neutrality, nuclear-weapon victory asymmetry, faction economy/combat bonuses, conventional Black Market, mercenaries, intelligence, and RTS systems. |
| [Resource Economy](Docs/RESOURCE_ECONOMY.md) | Nuclear/fossil resource hierarchy, mandatory crude-oil and nuclear refining, Concord refinery discount, nuclear power plants, electricity, low-cost Directorate infantry, and Meridian fuel-field spawning. |
| [Propaganda and Morale](Docs/PROPAGANDA_AND_MORALE.md) | Radio, newspaper and television propaganda for four factions; universal population morale improving production and army discipline. |
| [Reconnaissance and Scouting](Docs/RECONNAISSANCE_AND_SCOUTING.md) | Early-game faction scouts: Meridian fast scout-combat trucks, Concord/Ascendancy unarmed light planes, Directorate rifle-armed motorbikes. |
| [Explosive Sabotage](Docs/SABOTAGE_SYSTEM.md) | Meridian early, cheap non-spy-capable vehicle/building sabotage; Concord, Directorate and Ascendancy later, expensive and spy-only. |
| [Espionage, Psy-Ops and Intimidation](Docs/ESPIONAGE_AND_INTIMIDATION.md) | Unit and building Psy-Ops, concealed resource waste and Directorate diversion, Ascendancy facility contamination, spy progression, Intimidation, and Discipline/Morale. |
| [Nuclear Ultimate Weapons System](Docs/NUCLEAR_WEAPONS_SYSTEM.md) | Stolen nuclear-refining and weapons intelligence, processed nuclear materials, long production, bomber/ICBM delivery, nuclear power plants, theft, and Ascendancy's instant first-bomb victory. |
| [Engine Outline](Docs/ENGINE_OUTLINE.md) | Proposed Unreal Engine foundation, RTS systems, technical requirements, 4K support, and implementation priorities. |
| [Proposed Engine Layers](Docs/PROPOSED_ENGINE_LAYERS.md) | Separation of platform, presentation, match rules, simulation, world, content, and authoring tools. |
| [UI Overview](Docs/POLITIKA_UI_OVERVIEW.md) | Offline-first user experience, battlefield HUD, controls, campaign screens, interface scale, and accessibility goals. |
| [Faction files](Factions/) | Detailed ideological, political, economic, and military descriptions, plus faction logos and banners. |
| [Archive](Archive/) | Archived reference and development material. |

## Development and contributions

POLITIKA is a developing independent RTS project, and the repository currently focuses on its design documents and faction artwork. The intended implementation will be developed against those materials, with changes evaluated for consistency with the alternate-history canon and the asymmetrical, neutral-world gameplay design.

Feedback, bug reports, design discussions, documentation improvements, and contributions can be proposed through this repository's [issues](https://github.com/simon-gros/politika/issues) and pull requests. Proposed features should distinguish between established canon, design proposals, and working implementations, and should not treat planned mechanics as already completed.

Licensing note: The repository does not currently contain a standalone LICENSE file. A public repository is not automatically a grant of rights to reuse or redistribute its contents; licensing terms should be clarified before assuming those permissions.

---

*POLITIKA is an independent game project inspired by classic real-time strategy design. It is not an official Command & Conquer title and is not affiliated with that franchise's rights holders.*
