# POLITIKA — Unaligned Counterintelligence and False Intelligence

Status: The Unaligned's superior counterintelligence, fabricated intelligence, false-flag activity and match-stabilization role are established game-design rules. Exact triggers, probabilities, durations and counterplay remain to be balanced.
Related: [Concepts Outline](CONCEPTS_OUTLINE.md) · [Espionage and Intimidation](ESPIONAGE_AND_INTIMIDATION.md) · [Propaganda and Morale](PROPAGANDA_AND_MORALE.md) · [Match Structure](MATCH_STRUCTURE.md) · [Nuclear Ultimate Weapons](NUCLEAR_WEAPONS_SYSTEM.md) · [Faction description](../Factions/TheUnaligned/description.md)

## 1. The fifth faction's strategic identity

The Unaligned is the permanent, non-playable Yugoslav state and the game's principal system-AI faction. It is not an extra selectable player, and it does not pursue the ordinary playable factions' conquest victory. Its special role is to maintain Yugoslav stability and a viable balance among the two to four competing playable factions.

The Unaligned has the most powerful counterintelligence system in POLITIKA. Besides detecting, intercepting and investigating hostile information-gathering, it can deliberately produce and insert false intelligence into the rivals' information networks. This includes deception, false attribution and false-flag incidents presented as intelligence or in-world events, rather than necessarily overt radio/newspaper/television propaganda.

The primary purpose is to redirect attention, consume the resources of overconfident rivals, disrupt dangerous concentrations of power and prevent the immediate elimination of a very weak playable faction where preserving it is consistent with stability. The Unaligned is not an automatic ally of the weakest faction: it retains its territorial security rules, civilian obligations, and exceptional hostility toward identified Iron Ascendancy forces.

## 2. Three intelligence capabilities

1. Intercept: the autonomous NPC counterintelligence apparatus can detect, collect and sometimes obstruct or learn from rival spying, communications and reconnaissance discoveries. Its access, coverage and reliability are data-driven; it is not guaranteed to read every rival secret.
2. Fabricate: the Unaligned can create plausible but deliberately incorrect reports and plant them through a simulated intelligence channel. The recipient may believe an enemy is developing a facility, moving a force, gaining nuclear research or preparing an attack. The report is an information object, not a change to real unit/building/resource state.
3. Misattribute: certain reports or authored incidents can be presented as the actions of another faction, creating a fictional false-flag attribution and temporary fear, urgency or mistrust. A false attribution may later be uncovered through investigation and contradictory evidence.

These capabilities supplement the Unaligned's distinct NPC-controlled propaganda through radio, newspapers and television. Propaganda influences population confidence and narratives; false intelligence affects what rival players *believe* they know about strategic conditions. An event may feed both systems, but each must have independently defined rules.

## 3. Example: a nuclear-facility rumor

A playable faction obtains what appears to be a credible intercepted report: another named faction has begun constructing a nuclear research or production facility at a particular map location. In reality, no such facility may exist there; the Unaligned deliberately planted the report.

The recipient can investigate, order reconnaissance, compare sources, watch for genuine construction and risk committing troops to a distraction. The Unaligned benefits if the stronger rivals redirect surveillance, resources and operational attention away from an endangered weaker faction.

Non-negotiable distinction: a fabricated report must never instantiate a real nuclear facility, refined nuclear materials, research intelligence or bomb. It cannot satisfy the Iron Ascendancy's mandatory fully developed spy progression, verified infiltration of a genuine Atlantic Concord or Red Directorate nuclear research site, the two required nuclear intelligence unlocks, or the own-built first-bomb instant-victory condition. A forged report about a faction's nuclear activity does not change that faction's real nuclear capabilities.

## 4. Dynamic balance-preservation behavior

A major Unaligned AI job is monitoring the relative survival position of the actual participating playable factions and attempting to keep competition from collapsing into an immediate one-sided elimination.

Candidate strategic indicators include:

- Remaining usable resource stocks and income/production capacity.
- Current surviving army strength and ability to replenish it.
- Critical buildings, territory, defensive position and production infrastructure.
- Recent attrition, pressure, expected near-term ability to resist an attack and imminent elimination risk.

For an endangered faction with very low resources and a very small remaining army, the Unaligned can raise the priority of an intelligence diversion. For example, it may plant a false warning of a threatening rival's nuclear facility, reinforce an exaggerated force estimate, or falsely attribute a nearby attack so that an aggressor reallocates scouting, money or combat forces toward another existing rival.

This is a weighted, bounded strategy—not a hardcoded rubber band or promise that the losing player cannot be defeated. It should target only active, valid match participants, including sensible behavior on two-player maps where there may be only one opponent to divert. A fabricated rival or absent player faction may not be invented. Multiple healthy factions can still fight, choose not to believe an intelligence report or eliminate the weak faction despite the intervention.

The Unaligned's counterintelligence may act in service of stability even when the specific operation is deceptive or morally questionable. That tension is part of the fictional setting; it is not evidence that the Unaligned is universally benevolent, omniscient or permanently loyal to any playable rival.

## 5. Deception scope and constraints

- Plausible false reports may concern alleged troop deployments, industrial expansion, resources, border activity, supply convoys, nuclear-facility construction or another faction's intentions. Claims and map clues must remain internally consistent enough to investigate.
- False intelligence is an epistemic layer: it may populate a rumor marker, intercepted-message queue, briefing or estimated threat overlay, not the definitive world-visibility or authoritative build state.
- A planted report may be received through an in-game intelligence system rather than an automatic public announcement. The Unaligned can also use NPC media to amplify a narrative where applicable.
- Distinguish factual interception of real events from outright fabrication and false-flag attribution, storing which sources and forged artifacts refer to what.
- Give players counterplay: verification through physical reconnaissance, independent sources, intelligence comparison, inspections, discovering contradictions, credibility tracking and eventual attribution.
- Limit repetition, cost, cooldown, report credibility, targets, deception lifetime and the amount of time/money rational opponents may spend reacting. Overuse can reveal Unaligned manipulation and reduce future credibility.
- Deception cannot force a human player's commands, directly deduct resources without the player taking a normal action, invalidate visible legitimate scouting, erase discovered buildings or override production, combat and victory resolution.
- The Unaligned's survival-balancing logic does not forcibly prevent a valid match win or veto the Iron Ascendancy's own-first-bomb immediate victory. Faction hostility rules, including toward identified Ascendancy forces, still operate.

## 6. NPC decision and information architecture

Keep three distinct concepts:

1. Ground truth: authoritative match state of units, buildings, nuclear production, resources, owners and timers.
2. Intel observations and claims: what each faction has legitimately observed or intercepted, with source, confidence, timestamps, expiry and verification.
3. False-intel operations: Unaligned-authored fictional claims and false attribution, with hidden origin, intended recipients, narrative provenance, coverage/cost, objective, detection and consequences.

The NPC director can inspect bounded strategic strength indicators to decide whether balancing is warranted, but that privileged balancing assessment must not be transferred directly into the player-visible intelligence stream. Per-faction fog-of-war restrictions still govern real observations. Unaligned interception should have its own detection/coverage/eligibility conditions rather than an unexplained ability to read every secret.

Illustrative data-driven records:

- `FactionStrategicStrength`: active faction, resource sustainability, army value, buildings and elimination-risk score.
- `BalancePressure`: current leading/threatening force, threatened faction, validity, response priority and intervention budget.
- `CounterintelligenceCoverage`: Unaligned interception capability, source kind, detection chance and incident.
- `IntelClaim`: claim ID, stated faction, stated location, type, payload, timestamp, confidence, source and recipient list.
- `IntelGroundTruthLink`: whether a claim is true, false, outdated or unverified, inaccessible to ordinary player UI.
- `UnalignedFalseIntelOperation`: target rival, recipients, intended diversion, planted source, false-flag attribution, cost, cooldown, expiry, discovery and fallout.
- `IntelVerification`: evidence obtained, independent corroboration, contradictions, verified/false classification and attribution.

No AI deception action mutates `FacilityExists`, `NuclearIntelUnlocked`, `BuildCompleted`, ownership or victory state merely by asserting a report.

## 7. Presentation and narrative integration

Players should encounter false intelligence through normal RTS mission messages, intercepted communications, intelligence panels and uncertainty/verification labels. The UI can show a plausible alleged facility marker but must not falsely present it as a physically observed object; once a scout reaches the site and finds nothing, the rumor can be challenged or disproven.

The Unaligned's propaganda channels—federal radio, newspapers and television—can sometimes reinforce stability narratives. The covert continuity leadership that maintains the fictional post-1984 Tito deception may also exploit such information channels, but routine Unaligned AI operations need not imply that all Yugoslav officials or ordinary media staff know about the assassination and impersonators. Deception interacts with the primary conspiracy and secondary nuclear race without cancelling the fixed 1989 exposure and Yugoslav collapse.

The Unaligned should be recognizable as a strategic actor capable of both protecting civilians and manipulating rivals, rather than a static scenery faction. Discovery of fabricated intelligence can affect trust, diplomacy, legitimacy, population Morale and the Unaligned's perceived neutrality under authored rules.

## 8. Acceptance tests and open questions

1. Every 2–4-player-faction match has one autonomous Unaligned system AI that can perform counterintelligence and false-intelligence actions; it never occupies a player slot.
2. A planted claim about a nonexistent Concord or Directorate nuclear site appears as a report, not as a real building or verified nuclear production state; scouting/other evidence can contradict it.
3. Interception, fabrication and false-flag attribution are separately recorded, detectable and auditable. A recipient may act on a claim without being compelled.
4. With a severely endangered playable faction and a credible stronger aggressor, the Unaligned may prioritize a diversion intended to draw pressure away from the endangered one. The result is not guaranteed and cannot protect the weak player from ordinary defeat.
5. On a two-player map, the director cannot invent a third active rival or nonexistent real unit merely to support a false report. Forged threat claims remain claims.
6. The Unaligned does not need to be allied with an Iron Ascendancy player to act as a balance-preserving NPC; its exceptional hostility toward identified Ascendancy forces remains.
7. A fake report never grants spy research, nuclear-refinement technology, nuclear material, a completed atomic bomb or special victory. A real qualifying Ascendancy bomb completion ends the match regardless of pending Unaligned deception.
8. Even a successful deception operation does not silently alter player commands, true resource inventories, visible observed units or authoritative destruction and victory outcomes.
9. Discovering that a report was forged can damage Unaligned credibility, inform player counterintelligence and affect future acceptance of its planted reports.

Balance intervention frequency, active-player strength scoring, imminence of defeat, sabotage versus deception priority, report plausibility, false-flag credibility, cost/expiry, discovery methods and political consequences through prototype tests.

Locked summary: The Unaligned is the game's strongest counterintelligence faction, intercepting enemy intelligence and deliberately planting deceptive or false-flag reports. As the permanent autonomous fifth faction, it uses that information advantage to preserve Yugoslav stability and steer competition away from the imminent destruction of very weak playable factions, without altering actual facilities, information unlocks, player agency or victory conditions.
