# POLITIKA — Cross-Genre RTS Ontology and Competitive Reference Study

**Status:** RESEARCH AND PROPOSED DESIGN, 29 October 2026. **Not implemented, balanced, benchmarked, accepted, or evidence of superiority.** This is a comparative reference rather than an alternative gameplay canon or implementation roadmap.

**Repository baseline:** GitHub main commit `a2648971a1a211a6bb8a20da3b211e52d4401797` (29 October 2026). The state labels below summarize the source-owning [RTS Systems Completeness Matrix](RTS_SYSTEMS_COMPLETENESS_MATRIX.md), [Stage 4A acceptance](STAGE4A_MATCH_LIFECYCLE_ACCEPTANCE.md), [vertical slice](RTS_VERTICAL_SLICE_MILESTONE.md) and [Engine Outline](ENGINE_OUTLINE.md) at the time of research. Older qualified Linux Development evidence belongs to its exact recorded source/engine/package, **not** automatically to this commit. No fresh Unreal build, native tests, rendered playtest, competitor hands-on measurement or device qualification was performed for this document.

**Authority:** The [Engine Outline](ENGINE_OUTLINE.md) alone controls implementation ordering and promotion; the [RTS Game Ontology](RTS_GAME_ONTOLOGY.md) owns general vocabulary; the [Concepts Outline](CONCEPTS_OUTLINE.md), [Match Structure](MATCH_STRUCTURE.md), [Faction Rosters](POLITIKA_FIVE_FACTION_ROSTERS.md), [USD/power/food economy](RTS_OPERATIONAL_ECONOMY_AND_POWER.md) and specialist documents own POLITIKA mechanics. The companion [Competitive Benchmark and Acceptance Register](RTS_COMPETITIVE_BENCHMARK_REGISTER.md) owns proposed comparison fixtures and promotion evidence. This study introduces **no** new acceptance obligations for Stages 4A–4F.

## 1. Research question and methodological limits

**Question:** How can POLITIKA eventually compete with the strongest existing RTS games in every important dimension while offering a recognizable, original strategic experience?

Genre superiority is not established by a longer feature checklist. Different products optimize competing objectives: tactical precision, large-scale simulation, accessibility, realistic combined arms, short matches, campaign narrative, complexity, performance and modding. Literal dominance over every game for every player and hardware target is neither a meaningful current finding nor a guarantee. We instead seek **competitive parity on essential qualities and demonstrable differentiated strengths without unnecessary regressions**.

For game G, consider a multi-criterion quality vector Q(G) composed of measured command correctness, latency, legibility, AI fairness, tactical depth, economy, content, scale, accessibility and stability. A claim that game A Pareto-dominates B requires actual measurements under comparable scenarios and preferences. **No such measurements or numerical rankings are made here.** Reference titles are selected for transferable design mechanisms, not ranked by overall quality or alleged private engine internals.

The catalogue is a broad **representative candidate set**, not literally every RTS, every patch, or every hybrid ever released. For each item distinguish (a) publicly described game behavior, (b) original POLITIKA adaptation, (c) runtime implementation evidence. Official product descriptions are evidence of described features, **not independent proof of performance, balance, networking quality or actual player experience**. Date-sensitive source details must be rechecked when the register is updated.

## 2. Ontological dimensions used for comparison

| Domain | Ontological question | Examples of conditional implementation |
| --- | --- | --- |
| World and time | What persists and changes on the authoritative simulation clock? | Maps, scenario generations, events, objective states |
| Entity and capability | Which objects can act, suffer, produce, or be observed? | Infantry, vehicles, buildings, supplies, roads |
| Agent and mandate | Who decides, and what authority may they exercise? | Local human, playable opponent, NPC institution |
| Command and process | How does a requested intent become admitted execution and feedback? | Move, Attack, production, trade, research |
| Space and topology | What is reachable, visible, occupiable and strategically influential? | Terrain, LOS, roads, crossings, footprint |
| Military organization | How do individuals act as selected groups, persistent formations and battlegroups? | Control groups, shape, stance, cohesion |
| Economy and industry | Which stocks, flows, capacities, rights and time constraints enable action? | USD cents, commodity sale, power service, food upkeep |
| Knowledge and evidence | What can an agent see, remember, infer, verify or mistakenly believe? | Fog, reports, false intelligence, provenance |
| Institutions and politics | Which rules govern access, neutrality, legitimacy and intervention? | Unaligned military/civilian/security mandates |
| Victory and narrative | What counts as success and what persists across missions? | Skirmish outcome, objectives, campaign evidence |
| Presentation and input | How can players express and understand these relationships? | Contextual HUD, hotkeys, controller/touch |
| Production quality | How reliably is the experience delivered and tested? | Replay, packaging, scale, accessibility, balance |

**Important separations:** selection is not control authority; a numbered control group is not a physical formation; faction identity is not the current decision-maker; a claimed enemy position is not world truth; owning a facility is not having it powered; command admission is not the completion of that command; strategic capability is not equivalent to additional unit count. This table **extends the comparison axes** of the existing nine-category ontology; it does not replace its definitions or require twelve new Unreal modules.

## 3. POLITIKA now versus POLITIKA as proposed

| Capability | Present source/evidence boundary | Proposed completed-game intent | Primary owner |
| --- | --- | --- | --- |
| Shell, settings and recovery | Native UE shell and scoped Linux Development validation | Production-quality menus, display/audio/accessibility across qualified targets | [UI](POLITIKA_UI_OVERVIEW.md), [UI settings](RTS_UI_SETTINGS_AND_KEYMAP.md) |
| Military interaction | Single-unit selection; typed Move/Attack/Attack-Move/Stop/Guard and infantry prototype; scoped navigation/order evidence | Stable multi-selection, 0–9 overlapping control groups, queued/group orders, formal formations | [Selection](RTS_SELECTION_AND_GROUP_ORDERS.md), [Formations](RTS_UNIT_FORMATIONS_AND_DISCIPLINE.md) |
| Simulation and admission | Existing 0.05-second coordinator, events, authority/lifecycle foundation | Shared authoritative admission for human, AI, mission and later network inputs | [Command contract](RTS_COMMAND_SIMULATION_CONTRACT.md) |
| Combat | Bounded movement, damage, ability and objective prototype | Readable combined arms, suppression/Discipline, tactical stances, counters | [Engine](ENGINE_OUTLINE.md), [Morale](PROPAGANDA_AND_MORALE.md) |
| Economy, buildings and research | Resource helpers/typed content; **no end-to-end USD/power/food/production loop** | Money-primary economy, commodity conversions, powered buildings, purchased provisions, production and building-hosted research; **no age/era progression** | [Economy](RTS_OPERATIONAL_ECONOMY_AND_POWER.md), [Research](RTS_TECHNOLOGICAL_RESEARCH.md) |
| AI | Opponent participant slot **not** autonomous opponent | Fair first skirmish opponent; later strategic/operational/tactical policy hierarchy and different doctrines | [4D opponent](RTS_OPPONENT_AI.md), [AI architecture](RTS_AI_ARCHITECTURE.md) |
| World knowledge | No qualified participant-specific visibility/claim system | Fog, sightings, provenance, uncertain/fabricated intelligence, verification | [Visibility](RTS_VISIBILITY_AND_INTELLIGENCE.md) |
| Five factions | Narrative/roster design, limited prototype test identities; distinct NPC authority record | Four playable factions plus permanent separate Unaligned Yugoslav NPC institution | [Match](MATCH_STRUCTURE.md), [Factions](POLITIKA_FIVE_FACTION_ROSTERS.md) |
| Neutral institutional simulation | System identity, **not** autonomous patrol/political response | Local, institutional and strategic Unaligned policies; no hidden fifth playable player | [Counterintelligence](UNALIGNED_COUNTERINTELLIGENCE.md), [AI](RTS_AI_ARCHITECTURE.md) |
| Campaign/story | 1984–1989 alternate-history canon and substantial archive | Scenario-driven campaigns, historical setting, evidence/provenance-aware narrative | [Story](STORY_OUTLINE.md), [World](YUGOSLAV_WORLD_MODEL.md) |
| Content, visuals, platforms | Data-driven authored assets and prototype models; limited Linux Development package evidence | Finished original licensed content and qualified Linux/Windows/SteamOS/Android builds | [Production](PRODUCTION_STANDARDS.md), [Platform](PLATFORM_SUPPORT.md) |
| Replay and multiplayer | Neither accepted | Optional future verified save/replay/networking and competitive modes | [Persistence](RTS_MATCH_PERSISTENCE_AND_REPLAY.md), [Engine](ENGINE_OUTLINE.md) |

The central risk is integration debt: a hundred proposed specialist systems do not form a playable match until commands, economy, opponents, feedback and terminal conditions work together. First complete **Stage 4F** as presently defined; the following comparative ideas must not secretly expand its exit conditions.

## 4. Comparative families and reference lessons

### 4.1 Classical industrial RTS: Command & Conquer, Red Alert, Generals, Tempest Rising, 9-Bit Armies

**Distinctive ontology:** production facilities, rapid harvesting/conversion, short command-to-effect loop, legible units and counters, offensive tempo, decisive mission objectives. Traditional C&C exposes an industrial loop; Generals/Zero Hour varies specialized powers and army identities; Tempest Rising is the primary contemporary gameplay-quality and presentation reference.

**Transfer:** fast selectable units, clean order feedback, clear counters, faction silhouettes, reliable base operation, concise production UI and rapid resumption after interruption. The [C&C comparative study](CNC_GAMEPLAY_COMPARATIVE_STUDY.md) already owns title-by-title extracted-reference limits; the [Tempest Rising reference](TEMPEST_RISING_ENGINE_REFERENCE.md) owns publicly grounded UE observations.

**Avoid:** imported proprietary content or undocumented assumptions about Tempest's internal engine. At the **29 September 2026** expansion baseline, Tempest Rising includes the Veti third faction; its new campaign and multiplayer/skirmish content must be included in future comparisons, rather than benchmarking only the two-faction launch state [S1].

### 4.2 Competitive asymmetric RTS: StarCraft/Brood War, StarCraft II, Warcraft III, Age of Empires II/IV

**Distinctive ontology:** stable command grammar; economic opportunity costs; production/technology trees; faction mechanisms that genuinely differ; timing windows and inspectable combat counters. Warcraft III adds hero experience, auras and abilities; Age of Empires adds economic allocation and formations.

**Transfer:** asymmetry as different **production, information and operational choices**, not merely numerical buffs. Commander specialization should remain exclusive to POLITIKA's eligible Elite commanders and follow the existing four-tier OXP proposal; technological improvements belong to functional buildings without Age-of-Empires-style ages/eras.

**Avoid:** making ordinary units individually talented, hero domination of the military game, universal historical age progression, and balance claims without matchup samples.

### 4.3 Large-scale macro RTS: Total Annihilation, Supreme Commander/Forged Alliance, Beyond All Reason, Zero-K, Planetary Annihilation, Ashes of the Singularity

**Distinctive ontology:** many entities, multi-front control, strategic zoom, distributed industrial capacity, command automation, large unit-group behaviors, expensive spatial simulation. BAR publicly describes individual projectile simulation and very large battles [S2]; Zero-K advertises player-drawn formation geometry, automation, terrain modification and non-cheating AI [S3].

**Transfer:** optional formation and area orders, command queues, automation under an explicit doctrine, strategic map/focus, entity and rendering LOD, representative performance fixtures, workload-based scalable navigation.

**Avoid:** promising thousands of highly detailed NPC soldiers without measurement; rewriting the accepted UE coordinator around a different engine's design; assuming per-projectile physics or terrain deformation is essential to POLITIKA.

### 4.4 Military tactical RTS: Company of Heroes, Dawn of War, Wargame: Red Dragon, WARNO, Steel Division 2, Regiments, Broken Arrow, Men of War

**Distinctive ontology:** cover, reconnaissance, suppression/resolve, combined-arms ranges, sight lines, armor/anti-armor roles, battlegroups and counter-battery pressure. WARNO's official description emphasizes Cold-War battlegroups and its separate Army General campaign layer [S4]. Company of Heroes 3's tactical pause illustrates a **single-player** command-assistance option [S5].

**Transfer:** period-appropriate Yugoslav combined arms, meaningful front/rear exposure, sighting and positioning, recognizable unit roles, military formation spacing and combat discipline; consider optional offline tactical pause under separate mode policy.

**Avoid:** compulsory full ballistic, crew, ammunition and fuel micromanagement; importing divisional deck-building as a universal faction requirement; conflating turn-based operational campaigns with a match-local RTS mechanic.

### 4.5 Political/economic RTS and real-time strategy hybrids: Dune: Spice Wars, Sins of a Solar Empire II, Rise of Nations, Northgard, Stronghold, Offworld Trading Company, Total War

**Distinctive ontology:** territory as economic/institutional control, contracts, politics, influence, changing access, supply, intelligence and alternate victory incentives. Dune: Spice Wars explicitly includes combat, espionage, political resolutions and economic competition [S6]; Sins II describes fleet reinforcement automation, minor faction trading/influence and time-limited diplomacy [S7].

**Transfer:** legible jurisdiction and neutral-response rules; player-initiated trade of eligible gold/fossil stocks for USD; non-farming industrial control; counterable espionage; concrete consequences for power failures and disrupted infrastructure.

**Avoid:** parallel, mandatory civilian-farming economies, duplicate currencies, opaque unavoidable political punishments, and nationwide grand-strategy simulation inside every compact skirmish.

### 4.6 AI and alternative command paradigms: AI War 2, Zero-K, Halo Wars 2, Homeworld, They Are Billions, Stormgate, ZeroSpace

**Distinctive ontology:** adaptive strategic pressure (AI War 2 [S8]), inspectable automated unit policies (Zero-K), controller-first RTS UI (Halo Wars 2), fleet formations (Homeworld), defensive preparation and optional pause (They Are Billions), contemporary onboarding/custom-mode experiments (Stormgate/ZeroSpace).

**Transfer:** layered opponent strategy using **participant-observed** state, explicit difficulty policy, optional local tactical automation, readable controller/touch mappings and scenario-driven onboarding.

**Avoid:** universal claim that any such game has an ideal AI, universal tactical pause in competitive modes, or implicit support for third-party online services within POLITIKA's offline-first first-skirmish scope.

## 5. Wider candidate catalogue — coverage, not ranking

The following is a study queue, not a claim to exhaustive enumeration.

| Candidate / family | Primary feature to inspect | Competitive lens |
| --- | --- | --- |
| C&C Tiberian Dawn / Red Alert 1–3 / Tiberian Sun / C&C3 / Generals / C&C4 | Command feedback, production and faction grammar | Classic responsiveness and legibility |
| Tempest Rising (including September 2026 Veti) / 9-Bit Armies | Modern classic RTS pacing and presentation | Quality and onboarding |
| StarCraft: Brood War / StarCraft II / Warcraft III | Micro, macrogame, asymmetric production, heroes | Competitive clarity |
| Age of Empires II / III / IV / Age of Mythology: Retold / Empire Earth | Technology, economy, multi-unit tactics and counterplay | Strategy variety |
| Total Annihilation / Supreme Commander FA / Planetary Annihilation | Map-wide management and automation | Macro-scale control |
| BAR / Zero-K / Ashes of the Singularity: Escalation | Simulation density, terrain, huge forces | Performance and hierarchy |
| Company of Heroes 1–3 / Dawn of War 1–3 | Squads, cover, discipline, tactical feedback | Small-unit combat |
| Wargame: Red Dragon / WARNO / Steel Division 2 / Regiments | Combined arms, detection, deployment | Cold-War military credibility |
| Broken Arrow / Men of War / Gates of Hell | Specialized equipment, direct tactical control | Tactical detail |
| Homeworld 1–3 / Deserts of Kharak | Formation geometry and fleet doctrine | Group organization |
| Dune: Spice Wars / Sins of a Solar Empire 1–2 / Rise of Nations | Real-time diplomacy and resource influence | Political-economic depth |
| Northgard / Stronghold / They Are Billions | Defensible infrastructure, population/territory pressure | Economy and defensive play |
| Offworld Trading Company | Strategic resource exchange without armies | Market mechanics |
| AI War 1–2 | Strategic adversary reacting to expansion and threat | Nontrivial AI agency |
| Halo Wars 1–2 | Context and controller semantics | Handheld/controller adaptation |
| Stormgate / ZeroSpace | Contemporary faction design, multiplayer and campaign experimentation | Production process |
| Total War tactical battles and campaigns | Formations, morale, layered strategy | Interface between strategy levels |

Out-of-scope and unreleased/announced candidates may be added to later register reviews with dates and **availability** clearly marked; do not treat an announced game, unreleased feature or teaser as a verified released implementation.

## 6. Desired synthesis for POLITIKA

### 6.1 Preserve the indispensable classical loop

A participant must be able to enter a scenario, select and command legal units, observe understandable combat, acquire and spend **USD**, sustain a **powered** producer and **staffed-force food reserve**, reinforce, contest an objective, encounter an autonomous AI opponent, win **or** lose, and return to a new match without stale state. The exact evidence gate is [Stage 4F](ENGINE_OUTLINE.md#75-stage-4--first-playable-skirmish-repeatable-offline-match).

### 6.2 Differentiate through relations, not feature quantity

The long-term combined design should support interactions like:

**Scout → receive sourced report → verify or misjudge contact → authorize a military operation → disable infrastructure → alter production/visibility → prompt permitted Unaligned institutional reaction → create a new operational objective.**

This is a **proposed cross-system scenario**, not coded gameplay or campaign canon. Each relation must use the existing authoritative command boundary, provenance/visibility projection, transaction and objective owners. Uncertain reports alter information available to participants, **never** world truth. Faction rights, legitimacy and the Unaligned's NPC-only status remain intact.

### 6.3 Five faction doctrines and institutional differentiation

- **Atlantic Concord:** precise combined arms, intelligence, deployment flexibility.
- **Red Directorate:** sustained industrial mobilization and coherent military mass.
- **Iron Ascendancy:** clandestine coercion, infiltration and destabilization subject to existing restriction/counterplay rules.
- **Meridian Pact:** contracts, mobility, commodity exchange and mercenary access.
- **Unaligned:** a *single NPC-only* Yugoslav system authority with separately informed military, civilian, security and communications mandates. It is **not** a fifth playable economy or an omniscient duplicate opponent.

Existing faction and nuclear restrictions are binding; this study does not add abilities, bonus formulas, unit permissions, or victory exceptions.

### 6.4 Automation protects player agency

Allow eventual tactical automation, formation adherence and high-level doctrine **only** through inspectable, interruptible policies. Direct player orders take precedence according to an explicit arbitration contract; neither a saved selection group nor a UI overlay grants gameplay authority. Use the [Selection](RTS_SELECTION_AND_GROUP_ORDERS.md), [Formations](RTS_UNIT_FORMATIONS_AND_DISCIPLINE.md) and [AI architecture](RTS_AI_ARCHITECTURE.md) owners.

### 6.5 Economic and simulation complexity must earn their cost

USD is the ordinary spendable balance; electricity is a service/load capacity; food is a purchased consumable upkeep stock; eligible commodity sales are atomic authorized exchanges. Adding regional grids, connected logistics or market interactions is justified only when they produce observable alternatives, exploit/counterplay, and recoverable shortage states. Do not require endless manual worker farming or automatic loss after one power outage.

## 7. Competitive tradeoffs and acceptance philosophy

| Improvement target | Potential harm | Guardrail |
| --- | --- | --- |
| Higher entity counts | Tick/path/physics spikes | Representative p95/p99 profiling, LOD, aggregation after measurement |
| Richer combat | Unreadable battles, slow commands | Zoom-level readability, clear counters and order feedback |
| Advanced group automation | Hidden loss of control | Explicit adherence and override semantics |
| Complex diplomacy | Arbitrary results | Visible causal history, bounded uncertainty and recoverable responses |
| Sophisticated AI | Fog cheating, fabricated resources | Participant-scoped observations and authorized transactions |
| More currencies/upkeep | Busywork and deadlocks | USD-primary ledger, automatic/manual bounded procurement |
| More factions/units | Content and balance explosion | Distinct-decision test, scoped matchup grid, provenance |
| Better graphics | Poor hardware scalability and silhouettes | Tiered rendering and visual/thermal measurements |
| More supported platforms | Broken controls and unqualified ports | Platform-specific input/display/device gates |
| More planned features | Core match never completes | 4A–4F hard gate; specialist additions deferred |

A desirable quality vector cannot be inferred from marketing claims or self-assigned superiority scores. Compare concrete input outcomes, frame/sim timings, legibility, task completion and match diversity on **equivalent scenarios and defined hardware**; preserve raw artifacts and the comparator's version.

## 8. Representative cross-system demonstration: rail junction and power station

**Future optional benchmark scenario, not a Stage 4F requirement or implemented mission:** an industrial rail junction adjoining a powered civilian/military site. The human may inspect an uncertain claim of enemy reinforcement, choose verification, fund an operation using USD, maintain provisions, maneuver a formation, disable or defend the power supply, and encounter a rule-compliant Unaligned response. An opponent should respond with only information it could have acquired, and both actors must have meaningful recovery/counterplay.

This scenario is valuable because it links **information → economic opportunity → military position → infrastructure service → institutional response**, exposing conflicts between subsystems. First implement each prerequisite in its owning bounded milestone; only then make an integrated scenario. A valid test asserts authoritative state **and** player-visible feedback, not just a video or authored design document.

## 9. Priorities and document responsibilities

The ordered roadmap remains [Engine Outline §7](ENGINE_OUTLINE.md#7-suggested-implementation-sequence). Comparative guidance favors:

1. Finish Stage 4A–4F's **ordinary offline skirmish** before broader systems; preserve current UE coordinator, command admission and failure recovery.
2. Reach classical RTS control parity with 4C baseline and later separate **4C-G** control groups; resolve A/S key conflicts and provide actionable HUD feedback.
3. Add formations, stances and usable faction combat roles behind explicit NAV/FORM proof; measure large-group performance.
4. Introduce observer-scoped intelligence, bounded strategic AI, Unaligned patrol/protection and one comprehensible political consequence loop in later Stage 5 slices.
5. Prove cross-system differentiation and campaign mission quality before multiplying rosters, grids, espionage branches, commanders, research and naval systems.
6. Qualify actual platform/device performance and release rights through [Performance and Release Gates](PERFORMANCE_AND_RELEASE_GATES.md), rather than treating planned targets as shipped.

No additional roadmap stages or feature implementation decisions are locked here. **Proposed benchmarks, objective metrics, evidence status and maintenance cadence are defined only in the companion [Benchmark Register](RTS_COMPETITIVE_BENCHMARK_REGISTER.md).**

## 10. Reference sources and provenance

Public first-party/developer descriptions consulted 29 October 2026:

- **[S1]** [Tempest Rising — 29 September 2026 patch notes](https://support.saber.games/hc/en/tempest-rising/articles/september-29-2026-patch-notes-30), new Veti faction/11-mission campaign and competitive/skirmish update.
- **[S2]** [Beyond All Reason — official site](https://www.beyondallreason.info/), large-unit battles, real-time projectiles and terrain.
- **[S3]** [Zero-K — official site](https://zero-k.info/Home/), formations, orders, non-cheating AI and terrain mechanics.
- **[S4]** [WARNO — Eugen Systems](https://eugensystems.de/games/warno/), Cold War combined arms and Army General.
- **[S5]** [Company of Heroes 3 console design interview](https://news.xbox.com/en-us/2023/09/07/company-of-heroes-3-console-interview/), controller and tactical pause rationale.
- **[S6]** [Dune: Spice Wars — official site](https://www.dunespicewars.com/), military, espionage, political and economic mechanisms.
- **[S7]** [Sins of a Solar Empire II — gameplay](https://www.sinsofasolarempire2.com/gameplay), fleet management, minor factions and diplomacy.
- **[S8]** [AI War 2 — Arcen Games](https://www.arcengames.com/ai-war-2/), adaptive AI attention/expansion pressure.
- **[S9]** [Epic Games — Unreal Insights and performance profiling](https://dev.epicgames.com/documentation/en-us/unreal-engine/introduction-to-performance-profiling-and-configuration-in-unreal-engine), tool reference for future measured fixtures.

**Source caveat:** Public descriptions do not independently validate quality, latency, balance, opponent honesty, player preference or private engine implementation. Use the source game's actual release/patch identity and test conditions before recording a comparative verdict. No third-party source code, meshes, models, logos, UI art or audio is imported by this study.
