# POLITIKA — RTS Systems Completeness and Integration Matrix

**Status:** Design and source-gap assessment, 29 October 2026. **Not an implementation or milestone acceptance.** Audited against tracked GitHub `main` at `cdb85303f7e5a44d8a5469ba40531ce625cf8cc9`, the [Engine Outline](ENGINE_OUTLINE.md), [Stage 4A](STAGE4A_MATCH_LIFECYCLE_ACCEPTANCE.md), [Stage 3](STAGE3_DATA_CONTENT_MAP_ACCEPTANCE.md), [navigation record](RTS_RUNTIME_NAVIGATION_ACCEPTANCE.md), source modules and test registries. Historical Linux acceptance reports are prior-snapshot evidence; no UE/Cargo runtime was executed in this audit.

**Audit identity caution (29 October 2026):** This matrix's source snapshot is `cdb85303f7e5a44d8a5469ba40531ce625cf8cc9`. It is **not** a test of later main revisions. Read later navigation and infantry publication qualifications at their exact source/package identities in the owning acceptance records; do not promote an older dirty-build PASS into a current-clean-source claim. [Validation and Governance](VALIDATION_AND_GOVERNANCE.md) specifies evidence identity and future register fields; no current-source master acceptance ledger is asserted here.

## 1. Minimum RTS contract versus optional genre conventions

A real-time strategy game needs: (1) authoritative, progressing world state; (2) at least one player/agent controlling multiple strategically relevant entities or assets over time; (3) admissible spatial/strategic decisions; (4) observable feedback bounded by what that player can know; (5) scenario goals and terminal outcomes; and (6) a usable command/input interface. Their *specific implementations* vary. Resource harvesting, base construction, technological upgrades, fog of war, minimap, multiplayer, save/replay, formations and diplomacy are **common but not universal requirements**. For POLITIKA's stated **C&C-like, offline First Playable Skirmish**, a small economy, reinforcement production and autonomous opposing decisions have been deliberately selected as additional milestone requirements by [§7.5](ENGINE_OUTLINE.md#75-stage-4--first-playable-skirmish-repeatable-offline-match). More elaborate systems below are later design candidates, not automatically mandatory.

**Vocabulary:** *design* = specified intent; *source implemented* = code/assets present; *historically package-qualified* = an identified older build was tested, not this commit; *partial/inconclusive* = mandatory edge lacks qualifying evidence. A design table row is never runtime proof.

## 2. Coverage and ownership matrix

| Capability | Current evidence boundary | Owning specification / precise next contract | Earliest stage |
| --- | --- | --- | --- |
| App shell, settings, profile/recovery | Source; historic Linux shell qualification | [Shell](SHELL_SKELETON.md), [UI](POLITIKA_UI_OVERVIEW.md) | Existing |
| Faction, player, slot, neutral/system identity | Source + bounded recovery; PA-08/10/11 partial | [Match Structure](MATCH_STRUCTURE.md), [Stage 4A](STAGE4A_MATCH_LIFECYCLE_ACCEPTANCE.md) | 4A |
| Selection, multi-issuer orders and numbered control groups | Single-unit source + historic package; multi-select, ten-slot control groups and input rebinding remain unimplemented | [Selection and Group Orders](RTS_SELECTION_AND_GROUP_ORDERS.md), [comparative input research](RTS_CONTROL_GROUP_INPUT_RESEARCH.md), [UI keymap](RTS_UI_SETTINGS_AND_KEYMAP.md) | 4C baseline; **4C-G mandatory core-production follow-on** (not historic 4F exit) |
| Persistent military formations, tactical stances, Cohesion and Discipline integration | **Proposed design only**; no qualified runtime formations, large-group navigation, or implemented Cohesion bonuses | [Formations and Discipline](RTS_UNIT_FORMATIONS_AND_DISCIPLINE.md); [Selection](RTS_SELECTION_AND_GROUP_ORDERS.md); [Morale](PROPAGANDA_AND_MORALE.md) | FORM-F0–F8 after relevant NAV and 4C foundations, separate from normal 4F |

| Simulation tick, commands, events | 0.05 s coordinator + historic contract tests | [Command/simulation](RTS_COMMAND_SIMULATION_CONTRACT.md), [core rules](../Source/PolitikaCore/Rules.h) | Existing |
| Movement and navigation | Complete Recast routes/interruptions qualified on identified packages; NAV-A15 inconclusive | [Navigation acceptance](RTS_RUNTIME_NAVIGATION_ACCEPTANCE.md) | Now, then scale |
| Combat, damage, death, objective | Bounded tactical prototype verified historically | [Vertical Slice](RTS_VERTICAL_SLICE_MILESTONE.md); [Engine Outline](ENGINE_OUTLINE.md) | Existing; later expand |
| Participant observation, visibility/fog, scouting reports | Ontology and UI design; **no qualified authoritative visibility service** | [Visibility and Intelligence](RTS_VISIBILITY_AND_INTELLIGENCE.md) | Define now; implement after 4F unless scenario requires |
| Resources and transactions | Domain concepts and simple portable resource helper; no RTS economy loop | [Resource Economy](RESOURCE_ECONOMY.md), [Production & Technology](RTS_PRODUCTION_AND_TECHNOLOGY.md) | 4B minimal |
| Production queues, structures, placement, supply/tech | Specialized economy concepts; **no end-to-end production code** | [Production & Technology](RTS_PRODUCTION_AND_TECHNOLOGY.md) | 4B one factory; broader later |
| Opposing player decisions | Authored opponent slot but no autonomous opponent | [Opponent AI](RTS_OPPONENT_AI.md) (4D ownership), [AI architecture](RTS_AI_ARCHITECTURE.md) (cross-system reference) | 4D |
| Local autonomous behavior, tactical/operational policy hierarchy and faction AI personalities | Scoped unit combat/infantry behavior source; **no accepted coordinated tactical or strategic AI hierarchy** | [AI architecture](RTS_AI_ARCHITECTURE.md), [Formations](RTS_UNIT_FORMATIONS_AND_DISCIPLINE.md), [Command](RTS_COMMAND_SIMULATION_CONTRACT.md) | Stage 5C and FORM gates, after ordinary 4F |
| Unaligned neutral administration / deception | Separate system authority; narrative and policy designs | [Unaligned Counterintelligence](UNALIGNED_COUNTERINTELLIGENCE.md), [Concepts](CONCEPTS_OUTLINE.md) | 5+ |
| Scenario/missions, normal win and loss | Objective-driven terminal path; ordinary full skirmish not qualified | [Engine Outline](ENGINE_OUTLINE.md#75-stage-4--first-playable-skirmish-repeatable-offline-match), [Story](STORY_OUTLINE.md) | 4E–4F; campaign 5 |
| Diplomacy/markets/espionage/nuclear | Significant conceptual specs only | [Concepts](CONCEPTS_OUTLINE.md), [Specialized systems](DOCUMENTATION_INDEX.md#game-design-and-canon) | Post-4F |
| Persistence, saves, replays, desync evidence | Profile SaveGame exists; authoritative **match save/replay not implemented** | [Match Persistence and Replay](RTS_MATCH_PERSISTENCE_AND_REPLAY.md) | Deferred beyond 4F |
| UI/map overlay/minimap/accessibility | Bounded HUD + extensive UI design; complete adaptive profile not verified | [UI Overview](POLITIKA_UI_OVERVIEW.md), [Input](RTS_SELECTION_AND_GROUP_ORDERS.md) | 4C baseline, later |
| Authoring/validation and provenance | Data-driven assets and validation qualified in scoped Stage 3; archive/prototypes separate | [Gameplay Content](GAMEPLAY_CONTENT.md), [Production Standards](PRODUCTION_STANDARDS.md) | Continuous |
| Performance, density and platforms | Repeatability fixtures; no representative scale/device qualification | [Performance](PERFORMANCE_AND_RELEASE_GATES.md), [Platforms](PLATFORM_SUPPORT.md) | 4F baseline, 6 release |
| Multiplayer, replication, spectators, reconnect | No supported multiplayer release contract | [Engine Outline](ENGINE_OUTLINE.md#6-performance-determinism-and-networking-posture) | Optional future, not 4F |

## 3. High-impact conceptual omissions closed by accompanying specifications

The adjacent specifications provide *implementation-ready boundaries and negative tests*, while remaining proposed:

- [Visibility and Intelligence](RTS_VISIBILITY_AND_INTELLIGENCE.md): separate actual truth, per-participant observable facts, last-known contacts and intentionally deceptive claims; disallow omniscient UI/AI.
- [Selection and Group Orders](RTS_SELECTION_AND_GROUP_ORDERS.md): group membership, stale/dead pointer retirement, admission ordering, mixed acceptance and gesture arbitration across mouse/controller/touch.
- [Production and Technology](RTS_PRODUCTION_AND_TECHNOLOGY.md): transactional accounts, structure placement footprint, queues, interruption/refund and prerequisite graph validity without inventing a full economy.
- [Opponent AI](RTS_OPPONENT_AI.md): finite deterministic decision cycles with authorized input, fair information, reproducible replays, terminal shutdown and scenario-bounded success criteria.
- [AI Ontology and Architecture](RTS_AI_ARCHITECTURE.md): distinguish per-unit local behavior, formations, operational commitments, strategic policy, personality/difficulty and institutional Unaligned mandates. All policies submit legally admitted intents using their own permissible observations; broader AI-X test proposals remain **later**, not Stage 4D or 4F requirements.
- [Match Persistence and Replay](RTS_MATCH_PERSISTENCE_AND_REPLAY.md): separates profile data from authoritative match snapshots, replay event/order logs, schema compatibility, atomic write and integrity checks.

These documents supplement (and link to) the existing ontology, UI, economy, roadmap and acceptance records; they do **not** change existing gameplay design canon, command variants, saved Unreal assets, or the authoritative implementation sequence.

### Stage 4 cross-system dependency and whole-match criterion

The 4B USD-cent treasury, food/upkeep and positive building power are shared authoritative prerequisites for reinforcement production; they must not become separate simulation loops. Stage 4C group requests submit ordinary per-issuer typed commands. Stage 4D opponent behavior uses the same participant authority and (when implemented) participant-scoped observations. Stage 4E–4F needs both ordinary victory and defeat, Results, menu recovery and same-process rematch with no stale previous-generation money, orders or spawned units. The proposed single integration fixture and phase-order decision live in the [Engine Outline](ENGINE_OUTLINE.md#stage-4-cross-system-execution-and-evidence-thread-proposed), not in this secondary coverage matrix. Existing tests remain historically scoped, not an accepted complete skirmish.

## 4. Implementation prioritization and hard gates

1. **First close 4A**: PA-08 (dead/stale selection), PA-10 (real local controller versus opponent command), PA-11 (system/Unaligned command attempts), each with legitimate C++/Automation and, where required, packaged process evidence. Do not confuse direct test-only numeric participant access with external controller binding.
2. **Nav failures separately**: qualify a genuine supported unreachable/partial route via production admission/Tick; keep ER-01/ER-02/NAV-A15 inconclusive absent a matching actual Recast result. Never synthesize query results.
3. **4B**: a dollar-denominated treasury, one eligible source/delivery, a powered producer, basic food upkeep and reinforcement, with a shared ledger; source and cooked positive/negative evidence.
4. **4C baseline**: drag/multi-selection and per-unit group Move/Attack using existing typed request; human-readable ownership, outcome and affordability feedback. **4C-G**: separate **core production** ten-slot hotkeys, overlap/add/transfer/recall/focus, gesture/rebind conflicts, subgroup bar and acceptance; this is not retroactively a 4F requirement. Queued Shift chains, operational formations and advanced minimap remain later separate gates.
5. **4D**: small finite state/priority opponent using real participant authority and no fabricated purchases/commands; explicit scoped observations.
6. **4E–4F**: normal win and loss plus clean rematch in one rendered cooked process, with a reproducible authored scenario and measured baseline. Only then consider visibility service and broader production/map/AI complexity where the scenario justifies them.
7. **Later**: fog/intelligence, save/replay, campaign, balance/tech, crowded movement, platform qualification and multiplayer only under distinct acceptance evidence.

**Acceptance discipline:** For each new system preserve existing signed-off scope, add structural-invalid fixtures, portable tests where engine-independent, focused Unreal Automation, normal and negative cooked packages, and at least one rendered command/feedback acceptance when user-facing. Record an exact source SHA, UE engine pin, cooked asset identities and failure verdicts. Missing dependencies = BLOCKED; observed but insufficient edge = INCONCLUSIVE. Do not relabel documentation completion as product completion.

## 5. Unreal 5.8 evidence and framework selection

| Official source | Relevant capability | POLITIKA decision (recommendation, not code) |
| --- | --- | --- |
| [Epic: Asset Management](https://dev.epicgames.com/documentation/unreal-engine/asset-management-in-unreal-engine) and [Data Validation](https://dev.epicgames.com/documentation/unreal-engine/data-validation-in-unreal-engine) | Primary assets; Editor validity rule and commandlet | Continue stable-ID typed assets + existing shared validator; add editor rules only for authored stage-specific content |
| [Epic: Enhanced Input](https://dev.epicgames.com/documentation/unreal-engine/enhanced-input-in-unreal-engine) | Action/context/modifier/trigger separation | Translate device inputs into **existing semantic command boundary**; no separate simulator |
| [Epic: CommonUI design guidelines](https://dev.epicgames.com/documentation/unreal-engine/design-guidelines-for-using-commonui-in-unreal-engine) | Cross-platform and layered menus; explicitly says a simple PC RTS may not need CommonUI | Retain light tactical HUD; evaluate CommonUI only for layered cross-device shell |
| [Epic: UMG Viewmodel](https://dev.epicgames.com/documentation/unreal-engine/umg-viewmodel-for-unreal-engine) | Field-notify views; documented Beta status | Optional later HUD adapter, not a prerequisite; never expose true unseen world state |
| [Epic: StateTree](https://dev.epicgames.com/documentation/unreal-engine/overview-of-state-tree-in-unreal-engine) and [EQS](https://dev.epicgames.com/documentation/unreal-engine/environment-query-system-quick-start-in-unreal-engine) | Hierarchical decisions / spatial query | Evaluate **after** one bounded deterministic policy works; no need to install to begin 4D |
| [Epic: Mass Gameplay](https://dev.epicgames.com/documentation/en-us/unreal-engine/overview-of-mass-gameplay-in-unreal-engine) | LOD/instancing/entity density | Profile first; do not migrate actor/coordinator authority preemptively |
| [Epic: World Partition](https://dev.epicgames.com/documentation/unreal-engine/world-partition-in-unreal-engine), [Data Layers](https://dev.epicgames.com/documentation/unreal-engine/world-partition---data-layers-in-unreal-engine) | Streaming and editorial organization for large maps | Do not change existing compact RTS map because a large-world feature exists; evaluate only if measured world scale demands it |
| [Epic: SaveGame](https://dev.epicgames.com/documentation/unreal-engine/saving-and-loading-your-game-in-unreal-engine) | Custom save classes, sync/async APIs | Future match persistence needs its own schema and integrity policy distinct from profiles |
| [Epic: Automation Framework](https://dev.epicgames.com/documentation/unreal-engine/automation-test-framework-in-unreal-engine) and [Unreal Insights](https://dev.epicgames.com/documentation/unreal-engine/unreal-insights-in-unreal-engine) | Feature/functional tests; frame traces | Keep actual packaged integration evidence + trace-informed scalability decisions |

**Research date:** 2026-10-29. All links are public technical references; none is evidence a UE plugin is enabled or a POLITIKA implementation exists.

## 5A. Further UI and economic design specialization (29 October 2026)

The [Practical UI, Settings and Keymap](RTS_UI_SETTINGS_AND_KEYMAP.md) source audit identifies the immediately actionable double-assignment of **A** (camera left/Attack-Move) and **S** (camera backward/Stop), absent user-facing settings and remapping controls, and a development shell that does not yet implement the proposed UI menu hierarchy. The [USD Economy, Commodity Exchange and Upkeep](RTS_OPERATIONAL_ECONOMY_AND_POWER.md) specification now **adopts in-game dollars ($)**, gold/fossil commodity sales, building electricity minimums and purchased military food; the earlier generic-budget proposal is superseded. The [Overlooked Mechanics Review](RTS_OVERLOOKED_MECHANICS_REVIEW.md) records conditional mechanics for onboarding, battle alerts, territory logistics, rules of engagement, infrastructure outages and civilian/legitimacy effects. No runtime or schema qualification is implied; preserve existing 4A/4B–4F acceptance scope and military/faction canon.

### Economic design decision superseding a generic operational budget

Following the 29 October 2026 design lock, [Dollar Economy, Commodity Sales, Building Power and Military Food](RTS_OPERATIONAL_ECONOMY_AND_POWER.md) specifies the **primary purchasing resource as in-game USD ($)**, **gold/fossil-stock-to-dollar exchange**, **positive minimum electricity load for every operational building**, and **USD-purchasable food reserves for staffed force upkeep**. This explicitly supersedes the earlier proposed Operational Budget and elevates basic power/food provisioning to Stage 4B's required architecture. Tradeable gold/refined fuels, advanced power grids and automated supply remain later bounded slices. No UE code, runtime ledger or accepted package exists yet.

## 6. Documentation portability and validation findings

At this documentation-only change, an automated check enumerated the **11 newly added/updated Markdown documents** and resolved their repository-relative link destinations against the staged complete Git tree. **All relative links in those 11 documents resolve to tracked files/directories.** The same check identified **26 pre-existing absolute `/mnt/data/...` Markdown links in `Docs/ENGINE_OUTLINE.md`**, pointing outside the repository to local C&C reference/extraction material. These are not public or portable GitHub links; no linked file was available here for source verification. Keep this as a **separate provenance-preserving cleanup**: replace those links with stable/public references when available, or explicitly mark them as local-only evidence with an accessible bibliography; do not invent external URLs, import unlicensed content or erase technical context.

Structural check: the 11 documents have balanced fenced code blocks; no newly authored Markdown file contains trailing whitespace. Three trailing-space lines already exist in the large Engine Outline and were left untouched. This check **does not** validate external website uptime, Markdown heading-anchor behavior, factual historical provenance, native code compilation, Cargo, Unreal assets, engine startup or packaged gameplay. The source revisions in older acceptance logs remain historical.
