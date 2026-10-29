# POLITIKA — Bounded Skirmish Opponent AI Contract

**Status: PROPOSED and NOT IMPLEMENTED, 29 October 2026.** The current runtime opponent slot represents participant assignment, not a thinking or acting opponent. This document owns the first autonomous *playable-faction* behavior boundary; [Engine Outline](ENGINE_OUTLINE.md#75-stage-4--first-playable-skirmish-repeatable-offline-match) controls 4D sequencing, [Match Structure](MATCH_STRUCTURE.md) owns roster, [Visibility](RTS_VISIBILITY_AND_INTELLIGENCE.md) owns information projection, and [Unaligned Counterintelligence](UNALIGNED_COUNTERINTELLIGENCE.md) owns later separate NPC administration.

**Architecture relationship:** [RTS Artificial Intelligence Ontology and Architecture](RTS_AI_ARCHITECTURE.md) is the broader comparative design reference for local unit autonomy, formations, multi-step operations, strategic policy profiles, fair observations and the NPC-only Unaligned. This document remains the **single owner of Stage 4D first-opponent behavior and AI-01–AI-11 tests**. The proposed hierarchy does not enlarge the Stage 4D or Stage 4F gates.

## 1. Decision actor versus participant authority

One AI brain is bound to one admitted non-human playable participant and one match generation. It never masquerades as local human, Unaligned, or another participant. It does not call actor mutators, modify resource ledgers, teleport units, read HUD widgets or directly set mission results. Its entire effect is to submit the **same typed command and economic requests** as any authorized participant, through the coordinator at legal boundaries. Rejection is normal input feedback, not permission to override the command path.

Proposed decision input: immutable `DecisionSnapshot{participant_id, match_generation, step, economy_summary, owned_entities, eligible_commands, own_objectives, observed_contacts, known_map_objectives, past_command_outcomes}`. In 4D, before full visibility is implemented, the scenario must explicitly designate which *prototype observations* the AI may see, and the game must not later claim those inputs were fog-limited. When visibility exists, obtain `observed_contacts` exclusively through the per-participant projection, never an all-actors world scan.

**Resource fairness (adopted design):** The AI's economic observation must distinguish its own **US-dollar balance**, available gold/fossil stock, building electricity status, food reserves, due staffed-force upkeep and eligible USD-to-food procurement. It may sell eligible commodities or buy food only using the **same** authenticated transaction API and quote/capacity limits as a human. An opponent without sufficient power cannot use an unpowered producer; no hidden "free food", no automatic income outside authored rules, no privileged view of the player's treasury or food. The [USD/power/food contract](RTS_OPERATIONAL_ECONOMY_AND_POWER.md) owns this state and the AI only chooses requests.

## 2. Small deterministic policy sufficient for Stage 4D

Adopt a bounded priority list/FSM rather than a new general AI framework:

1. `Retired` if lifecycle not Active or participant/brain generation is invalid.
2. `Recover` if last action failed (invalid issuer/resource/target/nav), on a bounded cooldown; do not spin every Tick.
3. `Provision` when due food reserve is below the chosen authored safety threshold: buy authorized food for USD if affordable, otherwise seek a lawful dollar income/trade action or recover.
4. `Produce` when USD and a **powered** functional producer allow a dollar-priced unit; defer if its intended new staffed force cannot receive legal provisions.
5. `Defend/Contest` when an eligible currently observed threat/objective needs response.
6. `Attack/Advance` otherwise, using known legal objective/route and owned units.
7. `Idle` when no legal step exists, with a diagnostic reason; do not fabricate an enemy or instant resource income.

A decision occurs on a documented **authoritative step interval** (not frame delta), with a stable order by brain ID, unit ID and candidate priority. Use deterministic tie-breaks; any randomness comes from a per-match seeded stream with recorded draw points, not global wall-clock RNG. Bound queries/order attempts per step and avoid calling synchronous Recast once per unit per render frame. All valid orders must be able to fail safely, and navigation failure should cause a cooldown/replan rather than immediate repeated path queries.

The initial AI may be deliberately weak; its qualification is making legal, observable, effective choices that can produce both a player win and loss under separate authored scenarios. Difficulty changes must be documented policy parameters or input/decision cadence, not undocumented extra vision or forged resources.

## 3. AI memory and epistemic fairness

Keep a small per-participant memory of last known enemies, recent results, current plan, target IDs with generation, source and expiry. Before acting, revalidate target visibility/ownership according to the visibility policy and issued request. Do not preserve actor pointers across generations or save them as stable serialized identities. A deliberately misleading Unaligned report is an input claim; the AI may believe or test it like a player. Do not enable false-positive intelligence via direct access to the hidden true validity bit.

During shutdown/Terminal/Results/Return or admission rejection: cancel scheduled decisions, clear pending AI intents, retire references, and never emit post-terminal orders or continue a resource loop. One coordinator remains owner of the simulation clock.

## 4. UE framework fit (conditional)

Epic documents [StateTree](https://dev.epicgames.com/documentation/unreal-engine/overview-of-state-tree-in-unreal-engine) for hierarchical state/task/transition logic, and [EQS](https://dev.epicgames.com/documentation/unreal-engine/environment-query-system-quick-start-in-unreal-engine) for selecting spatial candidates. Neither inherently enforces POLITIKA's participant authorization, observation filtering or fixed-step order. A plain C++ deterministic policy is the preferred **first** 4D implementation. Consider StateTree after overlapping independent AI states cause complexity; consider EQS only for targeted spatial-choice cases once queries and budgets are measured.

[Mass Gameplay](https://dev.epicgames.com/documentation/en-us/unreal-engine/overview-of-mass-gameplay-in-unreal-engine) may offer entity-density/LOD advantages later, but choosing it now would introduce a second scheduler and endanger existing route/authority acceptance. Do not infer suitability from general RTS unit count without a benchmark.

## 4A. Formation-aware doctrine after the ordinary opponent milestone

[Military Formations, Tactical Stances, Cohesion and Discipline](RTS_UNIT_FORMATIONS_AND_DISCIPLINE.md) defines the future organization contract. After Stage 4D AI and FORM gates, an opponent can choose shapes, stances, adherence and legal regrouping using **the same** match-authorized requests and participant-scoped observations as the human. It must not receive mass Discipline from distant units or privileged enemy positions for tactical facing. Partial path failure requires bounded cooldown/recovery without per-frame queries. The NPC-only Unaligned continues under separate system authority. This is an optional later AI expansion, not a 4D completion claim.


## 4B. Deferred strategic architecture (not Stage 4D)

Once the ordinary first skirmish has passed, Stage 5C may introduce specialist goal proposals for defense, scouting, attack and withdrawal, one commitment manager to avoid competing orders, faction/personality profiles, and bounded utility or hierarchical planning when a tested operation needs them. Formation-aware choices wait for the owning FORM acceptance; fog-dependent intelligence waits for the authoritative visibility service; building-hosted technology decisions wait for real research prerequisites and completion events. A prospective adaptive personality must use only legitimate observations rather than the opponent's hidden economy, units or orders. Higher fair difficulty should change planning quality/cadence or coordination, not silently forge resources or vision. Distinctly disclosed unfair scenario modifiers are separate future design choices, not an ordinary difficulty entitlement.

The Unaligned institutional authority remains outside a playable-participant brain: its patrol/protection, civil administration and security mandates are future Stage 5D/5E work governed by [Unaligned Counterintelligence](UNALIGNED_COUNTERINTELLIGENCE.md) and [Match Structure](MATCH_STRUCTURE.md). No faction AI receives privileged institutional data through this separation. Proposed later AI-X acceptance cases belong to the [AI architecture](RTS_AI_ARCHITECTURE.md#8-incremental-implementation-and-acceptance-ladder); they are not part of the first AI-01–AI-11 gate.

## 5. Candidate test ladder

| ID | Evidence to produce |
| --- | --- |
| AI-01 | No AI brain can attach to missing, duplicate, local-human or Unaligned participant IDs |
| AI-02 | Fixed fixture/seed/input step stream yields same decision IDs, order sequences and authoritative outcomes |
| AI-03 | AI attempts an order only through shared admission, with valid owner/faction and match generation |
| AI-04 | Unavailable/hidden/stale targets do not leak truth through the decision snapshot |
| AI-05 | Insufficient USD/food, unpowered producer, unauthorized commodity sale or path failure lead to bounded retry/idle and no state spoofing |
| AI-06 | AI gathers, purchases and issues at least one accepted combat/objective command in ordinary 4D play |
| AI-07 | Terminal/Results/Return suppress future decisions and pending requests |
| AI-08 | Same-process rematch starts new identity, seed, memory, balances and orders |
| AI-09 | Logs contain observation source, decision step, reason, candidate, request, result; debug truth not shipped |
| AI-10 | Rendered Linux Development package: human-versus-AI plays through a normal result without acceptance fixtures |
| AI-11 | CPU budget / query count tested against representative 4F numbers; fail clearly if outside budget |

**Scope:** one non-human playable opponent in 4D; asymmetric doctrine, coordinated squads, deception reasoning, large-scale combat and autonomous Unaligned are later. No AI system should be marketed as implemented from a merely authored opponent player slot.
