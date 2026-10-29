# POLITIKA — RTS Artificial Intelligence Ontology and Architecture

**Status: PROPOSED CORE SYSTEM DESIGN / NOT IMPLEMENTED OR ACCEPTED — 29 October 2026.** This is a comparative research and cross-system integration specification, **not** proof of an autonomous opponent, working formation coordination, fog, economic planner or Unaligned agency. It introduces no gameplay code, test verdict or release qualification.

**Ownership and authority:** [Engine Outline](ENGINE_OUTLINE.md) alone owns staging and promotion; [Bounded Skirmish Opponent AI](RTS_OPPONENT_AI.md) owns the narrow Stage 4D policy and AI-01–AI-11 acceptance; [Command/Simulation](RTS_COMMAND_SIMULATION_CONTRACT.md) and [core rules](../Source/PolitikaCore/Rules.h) own typed order and match authority; [Visibility and Intelligence](RTS_VISIBILITY_AND_INTELLIGENCE.md) owns observer-scoped information; [Formations and Discipline](RTS_UNIT_FORMATIONS_AND_DISCIPLINE.md) owns actual persistent formation semantics; [USD Economy and Power](RTS_OPERATIONAL_ECONOMY_AND_POWER.md), [Production/Technology](RTS_PRODUCTION_AND_TECHNOLOGY.md) and [Research](RTS_TECHNOLOGICAL_RESEARCH.md) own economic and research legality; [Match Structure](MATCH_STRUCTURE.md) and [Unaligned Counterintelligence](UNALIGNED_COUNTERINTELLIGENCE.md) own NPC identity/mandates. This document owns **AI ontological vocabulary, decision-level boundaries, policy composition, cross-system integration and longer-horizon AI validation proposals**. More specific owners take precedence.

## 1. Comparative RTS evidence and transfer decisions

A source game's observed interface or authored rules are evidence for *possible architecture*, not evidence of undisclosed engine internals or permission to copy code/assets/identifiers. In particular, distinguish directly documented features, inspected source/authoring data, community analysis and POLITIKA proposals.

| Reference and evidence | Evidence class and limitation | Candidate transfer to POLITIKA | Disposition / owning boundary |
| --- | --- | --- | --- |
| [Electronic Arts C&C source release and modding FAQ](https://www.ea.com/games/command-and-conquer/news/modding-faq), [Generals / Zero Hour source repository](https://github.com/electronicarts/CnC_Generals_Zero_Hour) | Official source availability; useful for inspection of responsibilities, not direct compatibility or copying | Keep distinct player-level decisions, group operations, unit behavior and path services | **Adopt concept**; coordinator/command + this architecture |
| [Blizzard StarCraft II s2client-api](https://github.com/Blizzard/s2client-api/blob/master/docs/doxygen/mainpage.md) | First-party API documents separate Agent, Observation and Action interfaces; not full shipping bot implementation | Agents receive authorized observations and produce typed intents/actions | **Adopt concept**; visibility/command |
| [Age of Empires II AI scripting encyclopedia](https://airef.github.io/) | Community-maintained description of documented rule/fact/action and strategic-parameter vocabulary, not an internal architecture audit | Configurable rules and inspectable priorities for limited policies | **Adopt concept**; Stage 4D bounded FSM and later data profiles |
| [Forged Alliance Sorian AI guide](https://steamcommunity.com/sharedfiles/filedetails/?id=2221791900) | Community description of a particular AI modification, **not** a claim about every retail version | Separate strategic goal management, threat interpretation and force allocation | **Redefine** for POLITIKA's smaller, objective-led maps; later operational planning |
| [OpenRA trait documentation](https://docs.openra.net/en/release/traits/), [OpenRA AI configuration](https://github.com/OpenRA/OpenRA/blob/bleed/mods/ra/rules/ai.yaml) | Public engine docs and authored bot configuration expose squad and unit-production modules; version-specific | Data-configured production/defense/squad roles using a common simulation | **Adopt concept**, not OpenRA's per-tick implementation or numeric defaults |
| [Tempest Rising 29 April 2026 patch notes](https://support.saber.games/hc/en/tempest-rising/articles/april-29-2026-patch-notes-24) | First-party player-facing behavior: Adaptive opponent reacts to player pace; Insane explicitly grants income/production cheats; **no proof** of internal planner data structures | Keep personality separate from difficulty; optional declared unfair mode; adaptation only from authorized observations | **Adopt concept with stricter information fairness**; AI profiles and scenario settings |
| [Epic Unreal 5.8 StateTree](https://dev.epicgames.com/documentation/en-us/unreal-engine/overview-of-state-tree-in-unreal-engine), [EQS](https://dev.epicgames.com/documentation/unreal-engine/environment-query-system-quick-start-in-unreal-engine), [Mass Gameplay](https://dev.epicgames.com/documentation/en-us/unreal-engine/overview-of-mass-gameplay-in-unreal-engine) | Official framework documentation, not proof of POLITIKA integration | Evaluate optional hierarchical state/spatial search/data-oriented scale tooling after a measured need | **Defer**, with project-owned C++ 4D policy as first implementation |

**Research interpretation:** The transferable lesson is modular decisions and accountable state ownership. POLITIKA is not obliged to use a particular game's AI algorithms, fixed scripts, cheats, build orders, unit counts or platform assumptions. Prefer demonstrated needs over abstract frameworks. Follow the [Engine Outline comparative-evidence workflow](ENGINE_OUTLINE.md#71-cross-stage-engineering-and-promotion-rules), including Tempest source-evidence class, disposition, ownership and bounded tests.

## 2. Ontology: decision, knowledge, action and organization

An RTS world advances under one authoritative simulation. Participants and system institutions may delegate *choice* to agents, but never delegate unrestricted world-state authority. Terms below are distinct even where a first prototype uses the same object to represent several responsibilities.

| Term | Meaning and invariant |
| --- | --- |
| World truth | Full authoritative entities, resources, positions, ownership, processes, objectives and events; an AI policy must not freely scan it |
| Participant | Admitted playable match slot with a controller identity and faction assignment; one AI brain belongs to one nonhuman participant and current match generation |
| Institutional NPC authority | The Unaligned's separate system-owned decision rights; **not** a fifth playable participant, and not impersonation of a playable controller |
| Agent / policy evaluator | A bounded decision producer with an observation view, capability list, scheduling policy and memory; it does not directly mutate the simulation |
| Observation | Authenticated participant/institution-scoped projection with source and observation step |
| Belief | A revisionable assessment based on sightings, last-known positions, reports, confidence and uncertainty; can be wrong without changing truth |
| Goal | Desired condition, e.g. secure an objective, maintain food runway, hold a bridge, investigate intelligence |
| Plan / operation | Limited staged means of realizing a goal, with owners, commitments, success/failure/cancel states; **not** automatically an admitted order |
| Intent / command request | Candidate action that must pass the existing typed admission boundary with participant, issuer, generation, sequence and correlation |
| Execution / process | World-owned movement, combat, transaction, production or research after legal admission; may subsequently fail or be interrupted |
| Feedback | Accepted, rejected, completed, interrupted or invalidated outcome that updates the agent's later choice and bounded retry |
| Selection / control group | Local human/UI convenience; no automatic formation identity or command privilege |
| Formation | Explicit admitted, persistent tactical organization with member roles, slots, shape, adherence and derived cohesion; not interchangeable with saved selection |
| Unit stance | A persistent rules-of-engagement default governing delegated autonomy, independent from explicit move/attack/guard orders |
| Discipline / Morale | The established **one** per-unit UnitResolve displayed as combat Discipline or civilian Morale; not a new AI confidence/loyalty gauge |
| AI personality | Policy/priority/risk preferences, independent of legal faction capabilities and difficulty |
| Difficulty | Explicit policy quality, cadence and bounded coordination settings; never implicit income, resources or fog privileges |

**Separations to preserve:** faction is not player or agent; authority is not mere ownership; observation is not truth; a report is not proof; planning is not execution; control groups are not formations; cohesion is not Discipline; a combat stance is not a queued explicit order; and the Unaligned institutional director is not an ordinary AI enemy player.

## 3. Hierarchy of decision responsibilities

These are *logical responsibilities*, not a requirement for one UObject, Actor or module per layer.

| Layer | Owns decisions | May produce | Must not do |
| --- | --- | --- | --- |
| Local unit autonomy | Acquire permitted threats, orient/fire/reload, interrupt or resume under existing orders, recover from local movement failures | Scoped unit-level proposals inside an already admitted delegated order; explicitly admitted ability orders when required | Change ownership, invent hidden targets, override Stop/Hold policies or create a second combat simulator |
| Tactical formation | Retain eligible membership, role/slot assignment, legal movement coordination, combat engagement stance, regroup/withdrawal proposals | Individually authorized commands through shared group dispatch, once formation capability exists | Treat a hotkey group as formation; teleport members; confer Discipline bonus from roster size; override a specific individual command |
| Operational planner | Allocate force commitments to defend, scout, attack, escort, capture, withdraw; stage and recover multistep operations | Bounded mission/operation intents for eligible subordinate units | Command already committed units with conflicting goals; promise inaccessible path/target; loop on rejected routes |
| Strategic playable-faction planner | Select objectives, production, buildings, food/power restoration, commodity trading, technology/research and force composition | Candidate authenticated command/economic/production/research transactions | Directly credit USD, forge fog information, ignore prerequisites, create actors or claim victory |
| Unaligned institutional policies | Jurisdictional patrol/protection, public administration, security escalation, limited intelligence and later balance intervention | System-authorized actions bounded by mandated institution and observation access | Occupy a player slot, share all secrets with all departments or veto a lawful victory |

For human players, **local unit autonomy still runs**; operational and strategic AI normally do not select their objectives. A future optional assistance/automation feature would need explicit opt-in and separate ownership controls, never a silent takeover.

**Directive precedence:** hard gameplay legality and command authority > admitted explicit individual order/override > admitted operation and formation directives > formation default stance > archetype's local authorized behavior > idle. Specific active-order semantics from existing gameplay always win over a proposed AI heuristic.

## 4. One authoritative pipeline

```text
Authoritative match state (current match generation; 0.05 s coordinator step)
  -> authorized participant / institutional observation projection
  -> AI policy + limited memory + validated authoring profile
  -> candidate goals / scored plans / selected bounded intent
  -> ordinary authenticated typed command / economy / research intake
  -> coordinator validation -> accepted or rejected
  -> world-owned movement, production, combat and events
  -> observer-filtered feedback -> bounded memory/reassessment
```

**Do not** add a second world clock, navigation executor, resource ledger, HUD-owned truth, per-unit strategy Tick loop or trusted AI mutation path. Current `Source/PolitikaCore/Rules.h` provides typed requests and match phases. AI extends the *source of legal requests*, not the authority that executes them. A unit locally firing within a valid existing combat order remains simulation-owned order execution, not a bypass to the public AI submission boundary.

A future observation record may contain: authenticated observer, generation, observed step, own legal assets/capabilities/economy, observed enemy contacts and expiration, known public objectives, intel claims with source/confidence and prior command outcomes. The [visibility owner](RTS_VISIBILITY_AND_INTELLIGENCE.md) governs which of these fields may exist and what the AI can infer. For Stage 4D, use expressly declared *prototype-visible* inputs instead of claiming unimplemented fog fairness.

### 4.1 Bounded planning policy

Stage 4D is deliberately simple: Retired -> Recover -> Provision -> Produce -> Defend/Contest -> Attack/Advance -> Idle, with deterministic priority and legal requests as defined in [the owning contract](RTS_OPPONENT_AI.md). **Do not bring utility/HTN/StateTree, faction doctrines, fog or institutional AI into 4D acceptance.**

Later policies may produce competing goal proposals rather than simultaneously issuing commands. Candidate ranking can use normalized benefit, survival, intelligence value, USD/time cost, operational risk, uncertainty and commitment/switching cost. An illustrative non-normative score is:

```text
score(goal) = wV * victory_value + wS * survival_value + wI * information_value
              - wC * total_cost - wR * risk - wK * switching_cost
```

Validate *hard constraints first* (capability, participant, visibility, real affordability, power, food, prerequisite, route legality, mission restrictions). Weighting cannot override those constraints. Scores need deterministic tie-breaking and an explicit hysteresis/commitment policy to avoid constant plan churn. Track the actual command outcomes, never assert goal success from a proposal alone.

### 4.2 Scheduling, reactivity and budgets

Decisions run at documented authoritative coordinator-step boundaries, not render frame rate. Local reactions may be evaluated frequently **only inside existing execution budgets**; strategic choice is slower and can be event-triggered (loss of a base, visible threat, shortage, completion or failed order). These are design principles, not chosen cadence numbers. Keep one scheduler, stable order by generation/brain/issuer/command ID, per-step caps on candidates, commands, route/EQS queries and retries, and seeded/recorded random draws where used. Staggered evaluations must be replayable. Large groups should reuse group/nav queries when empirically appropriate; never query Recast per soldier per rendered frame.

A rejected/blocked path or economic action records a reason and bounded retry backoff. Prevent retry storms, oscillating targets, starvation of defensive work, repeatedly replacing a valid order, and unfinished plans leaking through Terminal/Results/Return/rematch.

## 5. Cross-system decisions

### 5.1 Unit autonomy, formations and commanders

Unit behaviors (including proposed auto-acquisition, range/aim, ammunition and stance) must remain subject to explicit orders and hostility/visibility rules. The current infantry behavior data asset contains an idle auto-acquire flag and aiming/reload parameters; this **does not** establish a general purpose unit or tactical AI. A defensive Guard order must not cause an unbounded chase.

Formation AI may later allocate roles, choose Line/Column/Dispersed/Perimeter, orient, regroup, split, reinforce or withdraw **only through the implemented formation owner**. Individual members preserve their own move/path/attack rejection and identity. Derived Cohesion can modulate the *existing* Discipline effects where the faction contract permits; it cannot introduce a second resolve meter or bypass exclusive Psy-Ops/Intimidation break/isolation requirements. See [Formations](RTS_UNIT_FORMATIONS_AND_DISCIPLINE.md).

Commander talents apply only to eligible Elite commanders under the [Talent System](COMMANDER_TALENT_SYSTEM.md). An AI may choose a legal talent when its progression allows it; no commander policy grants new faction-exclusive powers, an additional faction slot, illegal production or an unearned research unlock.

### 5.2 USD economy, electricity, food, trade and research

Strategic AI observes its **own** USD-cents treasury, gold/fossil inventory and permitted quotes, food reserve and due upkeep, powered/unpowered buildings and valid queues. It must submit the **same authenticated transactions** as a human: sale, purchase food, production, building activation and later research. An unpowered factory does not produce; staffed troops incur real food demand; an AI has no hidden minimum treasury, free food or unlimited material. A plan must account for committed and already-reserved expenditure to prevent multiple modules double-spending. The economy ledger, not AI memory, settles transactions.

Candidate later research choices account for building prerequisites, affordability, research capacity and faction-legal effects. POLITIKA has **no age/era advancement system**. Research unlocks may influence the desirability of unit compositions/formation organization, but cannot modify unit stats or access directly from a planner. See [Research](RTS_TECHNOLOGICAL_RESEARCH.md).

### 5.3 Knowledge, espionage and false reports

An AI may interpret CurrentlyObserved, LastKnown, Reported and Unknown differently. Report confidence, age and source allow intelligence to change *belief*, not actual enemy position or actor existence. A reported nuclear facility must not become a real structure or a valid target by ID. Bad information may cause a rational but mistaken reconnaissance/defense decision. Verification and correction require real authorized events. Attack-ground, if designed later, targets a place and cannot reveal a hidden unit. Visibility-denial text and diagnostics must avoid hidden-truth leaks.

### 5.4 Faction doctrine as policies, not parallel controllers

Validated faction AI policy parameters may later bias permissible strategic choices:

| Playable faction | Proposed doctrine bias (not implemented) | Hard boundary |
| --- | --- | --- |
| Atlantic Concord | Combined arms, precise threat selection, aerial reconnaissance, smaller flexible groups, conditional Psy-Ops | No invented Psy-Ops outcomes or mandatory small-unit bonus; existing faction rules win |
| Red Directorate | Industrial force concentration, mass formations and ordinary intelligence acquisition | Large-group Discipline effects only from real coherent operational groups |
| Iron Ascendancy | Concentrated mechanized assault, higher offense risk, specialized Intimidation | BrokenThreshold/isolation and nuclear-victory prerequisites always enforced |
| Meridian Pact | Mobility, reconnaissance, fossil logistics, opportunistic early sabotage | Legal source access, rights and role limits; no automatic ownership of independent market services |

Personality (Balanced/Defensive/Aggressive/Adaptive), faction doctrine, and difficulty are distinct. Adaptive decisions use **observed** enemy aggression/expansion and never assume access to enemy hidden dollars or orders. Higher fair difficulties can improve scheduling, coordination and planning; any resource/production bonuses must be an **explicit separate unfair mode**, disclosed at match setup and executed via a documented scenario modifier. No such mode is currently adopted or implemented.

## 6. The Unaligned is institutional agency, not a fifth opponent

The Unaligned remains a **single NPC-only faction and system authority** in all normal 2–4 playable-participant matches. Its projected institutional policies may cover JNA/territorial-defense patrol and protection, federal civil-service/infrastructure continuity, internal security escalation, intelligence reception/verification and the exceptional balancing/counterintelligence director. They share a faction authority, but do **not** automatically share knowledge or mandates.

Implement lawful patrol/protection and response first under Stage 5D. Balance intervention, fabricated reports and false-flag attribution require the separate [Unaligned Counterintelligence](UNALIGNED_COUNTERINTELLIGENCE.md) constraints: bounded provenance, costs, confidence, recipients, cooldown/expiry, counterplay and discoverability. Any *privileged strategic aggregate* permitted to a balancing director must be narrowly scoped; it cannot be laundered into omniscient unit targeting or visible player intelligence. No Unaligned decision may forcibly stop an ordinary valid match win, create a human slot, or ignore its special hostility rules.

## 7. Unreal Engine 5.8.3 design choice

Start with a **plain C++ fixed-step priority/FSM** for 4D. Add no new Unreal module simply to mirror this diagram. UE Data Assets may eventually hold stable-ID profile versions, faction/personality tags, legal policy parameters and named capability references, with the same editor/runtime validation and cooked discovery as other gameplay content. Invalid IDs, cycles, illegal faction actions, missing strategies or malformed profile values must fail with actionable diagnostics; do not silently use generic aggressive fallback when a mandatory profile is invalid.

StateTree may later improve complex local or institutional state transitions; EQS may propose sampled tactical cover/rally positions, subject to legal visibility and nav checks; Unreal Mass requires profiling and cannot become a second authoritative simulation. Unreal Behavior Trees and HTN/utility methods are optional designs, not mandatory engine features. C++ policy tests must work independent of UMG, VFX, animation or physical controller devices. Any multiplayer authority, save/replay and platform-specific deterministic parity are later individually accepted features.

## 8. Incremental implementation and acceptance ladder

**These IDs are NEW PROPOSALS, not tests already in the repository or passing verdicts.** The existing AI-01–AI-11 matrix in [Stage 4D AI owner](RTS_OPPONENT_AI.md) continues to control first-opponent acceptance.

| Increment / ID | Narrow deliverable | Prove / defer |
| --- | --- | --- |
| 4D / AI-01–AI-11 | One legal participant-bound brain, bounded deterministic priority/FSM, prototype-scoped observations, food/production/attack, shutdown | Accepted orders, lawful resource transactions, reproducibility and rendered package; **no** strategic utility framework |
| 4E–4F | Human win/loss, result, return, fresh same-process skirmish | No post-terminal intents, stale memory or participant/generation reuse |
| AI-X01 / Stage 5C | Local threat/stance reaction integrated with existing unit orders | Stop/Hold/Guard semantics; no hidden targets or unauthorized pursuit |
| AI-X02 / Stage 5C | Defensive/attack/scout/withdraw operational plan with bounded force allocation | No conflicting assignments, route-loop storms or commands for nonowned units |
| AI-X03 / after FORM acceptance | 8-unit formation role/slot/stance coordination and legal regroup/interrupt | Formation/hotkey separation, per-member failures, Cohesion/Discipline correctness |
| AI-X04 / after visibility acceptance | One observed contact, stale contact, false report and verification decision sequence | No truth leak in observations, rejections, logs or command targeting |
| AI-X05 / after economy/research extension | Replan across power loss, due food purchase, unaffordable tech and valid research completion | Dollar/reservation conservation, zero free production, no era systems |
| AI-X06 / Stage 5D | Unaligned legal patrol/protect/investigate institutional policy | NPC-only controller, allowed hostility/jurisdiction, mandate-specific knowledge |
| AI-X07 / after deception contract | Unaligned planted claim is acted upon and later disproved through legal investigation | False report does not instantiate facility, alter objective truth or veto victory |
| AI-X08 / later profile balancing | Two doctrines and personality/difficulty combinations produce distinct, explainable decisions | No illegal privilege or hidden wealth/vision; seeded fixed scenarios |
| AI-X09 / Stage 6 scale | Recorded representative skirmish, grouped movement and fog/AI workload | Simulation-time p95/p99, query counts, candidate count, memory and reproducible logs |

Record `decision_step`, generation, agent/participant/mandate, source observation step and kind, selected goal/reason, legal candidate set summary, submitted request/correlation, admission result and actual outcome. Expose complete world-truth diagnostics only through isolated Development tooling, never Shipping participant telemetry. Tests must cover missing actors, destroyed/stale targets, interrupted navigation, power/food starvation, rematch, false claims, duplicate intents and fairness. Do not label unexecuted acceptance as PASS.

**Roadmap constraint:** [Stage 4D](ENGINE_OUTLINE.md#75-stage-4--first-playable-skirmish-repeatable-offline-match) stays small; later operational and faction doctrine work is Stage 5C, Unaligned agency is Stage 5D/5E, and performance optimization is Stage 6. No full AI system or new feature is promoted from design-only Markdown.

## 9. Verification and document maintenance

Before code work, check current main SHA, worktrees/dirty state, exact pinned UE environment, all relevant acceptance evidence and the specific owner documents. Implement through isolated worktrees and promote only tested scope. For documentation-only changes, verify all relative Markdown links and keep README/index discovery synchronized; no Unreal run or SemVer increment is implied by design edits. The cross-title evidence above does not establish compatibility of borrowed assets, algorithm runtime superiority or proprietary-code reuse.

**Design resolution:** POLITIKA's AI is a family of bounded **decision policies** consuming permissible observations and proposing lawful actions to one match authority. Existing unit behavior and the first 4D opponent are the narrow starting points. Group tactics, strategic planning, political institutions and believable errors caused by false intelligence are **planned extensions with separate gates**.
