# POLITIKA — RTS Competitive Benchmark and Acceptance Register

**Status:** PROPOSED QUALITY/RESEARCH REGISTER, 29 October 2026. **No new tests executed, metrics recorded or competitor superiority demonstrated.** Companion to [Cross-Genre RTS Comparative Study](RTS_CROSS_GENRE_COMPARATIVE_STUDY.md).

**Snapshot:** Source baseline main `a2648971a1a211a6bb8a20da3b211e52d4401797`, 29 October 2026. Existing package evidence has its own historical identity; consult the owning acceptance files before assigning an implemented or accepted verdict. The benchmark cases below are **proposals**, not additions to First Playable Skirmish Stage 4A–4F acceptance.

**Authority and ownership:** [Engine Outline](ENGINE_OUTLINE.md) alone schedules implementations; [RTS Systems Completeness Matrix](RTS_SYSTEMS_COMPLETENESS_MATRIX.md) owns status inventory; [Validation and Governance](VALIDATION_AND_GOVERNANCE.md) owns evidence identity; [Performance and Release Gates](PERFORMANCE_AND_RELEASE_GATES.md) owns production performance workloads; subsystem documents own behavioral contracts. This register owns **comparative research, candidate observations, measurable goals and test-case mappings**, avoiding a second roadmap or independent benchmark simulation.

## 1. Method: from reference feature to POLITIKA evidence

A proposed competitive advantage must be independently demonstrable. Follow this sequence:

1. Identify a competitor **title + version/patch + platform + source**. Distinguish official feature description, independent hands-on observation and measured data.
2. Identify one **underlying mechanic** independent of proprietary assets or implementation details. State how its decision costs, feedback and counterplay work.
3. Map it to the owning POLITIKA rule, stage and exact existing runtime API. If the capability is unscheduled, mark **PROPOSED / DEFERRED**.
4. Write a bounded **positive + negative** fixture. Test legal authorization, failure/interrupt, visibility, game-generation teardown, recovery and UI feedback where relevant.
5. Record source Git SHA + dirty marker; Unreal pin; content/map IDs and hashes; packaged executable/platform; hardware; ruleset; scenario seed; instructions; test outputs; time distributions; limitations.
6. Compare **equivalent decisions** under similar workload, input and rendering profiles. Do not compare a cinematic marketing demonstration with a reproducible POLITIKA capture.
7. Retain the original evidence, classify results as **PASS, FAIL, INCONCLUSIVE, BLOCKED, NOT RUN**, and promote only through the owning milestone.

The terms **DESIGN**, **SOURCE-IMPLEMENTED**, **HISTORICALLY PACKAGE-QUALIFIED**, **CURRENT-BUILD-QUALIFIED** and **PLATFORM-QUALIFIED** are not interchangeable. A document is never itself a test pass. Public descriptions of competitors are not measurements of their performance, AI fairness, accessibility or balance.

### Meaning of competitive verdicts

| Verdict | When warranted |
| --- | --- |
| Unmeasured | No equivalent gameplay, device or task evidence has been collected; default state of the proposed register |
| Baseline verified | POLITIKA case has reproducible, identity-bound acceptance at its declared scope; **no comparison claim** |
| Comparable observation | Both titles were tested under documented reasonably equivalent conditions, with differences and uncertainty noted |
| Demonstrated improvement (specific metric only) | The defined metric meets a predeclared comparison criterion under the named scenario/hardware, without implying overall superiority |
| Inconclusive | Evidence exists but workload, versions, instrumentation, confidence or negative cases are insufficient |
| Blocked | Required system, fixture, executable, platform or legal right is unavailable |

Do not assign a numerical rating, ranked order or universal “better than” label to a title from promotional copy, developer assertions or subjective intuition.

## 2. Core benchmark cases — proposed, not run

| ID | Domain and representative reference | Comparative question | Candidate POLITIKA task / negative proof | Owner / earliest context |
| --- | --- | --- | --- | --- |
| **CMP-01** | C&C3 / Tempest Rising — input | Can a player select, command and understand acknowledgement immediately? | Selection/Move/Attack/Stop feedback; invalid owner/target; A/S binding collision; no stray command from camera pan | [UI keymap](RTS_UI_SETTINGS_AND_KEYMAP.md), [command feedback](RTS_COMMAND_FEEDBACK_INVENTORY.md); 4C and separate UI gates |
| **CMP-02** | StarCraft II / AoE IV — production | Is production/economy affordance clear while tactical play continues? | USD deposit/spend, power load, food purchase/upkeep, queue blocked-spawn and cancellation; no duplicate credit/charge | [Economy](RTS_OPERATIONAL_ECONOMY_AND_POWER.md), [Production](RTS_PRODUCTION_AND_TECHNOLOGY.md); 4B |
| **CMP-03** | Zero-K / BAR — many-unit orders | Can a player organize overlapping groups and issue reliable mass commands without micromanagement? | 0–9 control groups, multi-select, partial admission, death/transfer/rematch cleanup; no duplicate requests | [Selection](RTS_SELECTION_AND_GROUP_ORDERS.md); 4C-G after 4C |
| **CMP-04** | WARNO / Total War — formations | Are facing, spacing, stances and actual Cohesion useful and observable? | Line/Column/Dispersed/Perimeter slots; invalid member/path; mixed movement classes; no morale bonus from mere selection | [Formations](RTS_UNIT_FORMATIONS_AND_DISCIPLINE.md); separate FORM gate |
| **CMP-05** | Company of Heroes 3 / Dawn of War — tactical readability | Do positioning, role and Discipline change decisions in an understandable way? | Comparable infantry/vehicle engagement with LOS, terrain and morale as implemented; no unexplained target or stat change | [Morale](PROPAGANDA_AND_MORALE.md), [Engine](ENGINE_OUTLINE.md); post-foundation |
| **CMP-06** | StarCraft II / Tempest Rising — asymmetric design | Can players identify genuine faction mechanical tradeoffs and counterplay? | Same objective under each playable faction, test viable openings/counters; no prohibited faction/nuclear ability | [Factions](POLITIKA_FIVE_FACTION_ROSTERS.md), [Concepts](CONCEPTS_OUTLINE.md); after faction implementations |
| **CMP-07** | Zero-K / AI War 2 — AI behavior | Does AI make legal, non-cheating effective decisions and explain them? | Deterministic fixture seed, participant-scoped observations, resource/power/food rejection, no post-terminal orders | [Opponent](RTS_OPPONENT_AI.md) 4D; [AI](RTS_AI_ARCHITECTURE.md) later |
| **CMP-08** | Dune: Spice Wars — intelligence/politics | Does information influence decisions independently of world truth? | Inject sourced incorrect claim; record recipient decision, verification/correction; forbidden hidden truth never leaked | [Visibility](RTS_VISIBILITY_AND_INTELLIGENCE.md), [Unaligned](UNALIGNED_COUNTERINTELLIGENCE.md); Stage 5+ |
| **CMP-09** | Sins II / Rise of Nations — institutional/economic interaction | Does control of infrastructure produce an alternative strategy? | Disable powered site, legal emergency recovery, changed production/observation, Unaligned local response; no unrecoverable scripted penalty | [Economy](RTS_OPERATIONAL_ECONOMY_AND_POWER.md), [AI](RTS_AI_ARCHITECTURE.md); Stage 5+ |
| **CMP-10** | Warcraft III — commander identity | Do Elite choices alter doctrine without trivializing ordinary forces? | Match-local four-tier OXP, exclusive choice, removal/redeployment, faction restriction, no ordinary-unit talent leak | [Talents](COMMANDER_TALENT_SYSTEM.md); separate post-4F gate |
| **CMP-11** | AoE / StarCraft — research | Do technology choices create alternate operational paths? | Host structure powered/eligible, prereq graph, shared versus local upgrade, blocked/paused research, no invented ages | [Research](RTS_TECHNOLOGICAL_RESEARCH.md); post-4F |
| **CMP-12** | BAR / Ashes — scaling | Does simulation remain responsive at increased representative entity density? | Small skirmish → group pathing → AI/fog → render stress, with p50/p95/p99 sim/frame timing, spikes and command delay | [Performance](PERFORMANCE_AND_RELEASE_GATES.md); continuous then Stage 6 |
| **CMP-13** | Halo Wars 2 — input portability | Can all essential intents be expressed on an actual controller/touch device? | Device-specific select, inspect, target, cancel, groups, production; no accidental attack from focus change | [UI](POLITIKA_UI_OVERVIEW.md), [Platforms](PLATFORM_SUPPORT.md); Stage 6 |
| **CMP-14** | StarCraft II / Tempest Rising — narrative/missions | Does a mission teach meaningful rules and complete with valid outcomes? | Canonical 1984–1989 scenario, briefing, optional and primary goals, failure/retry, historical and rights evidence | [Story](STORY_OUTLINE.md), [World](YUGOSLAV_WORLD_MODEL.md); Stage 5 |
| **CMP-15** | FAF / AoE — replay and competitive integrity | Is a recorded game reproducible with compatible content and no authority spoof? | Versioned snapshots/commands, corrupted replay rejection, mismatch detection, rematch isolation; future network spoof/reconnect tests | [Persistence](RTS_MATCH_PERSISTENCE_AND_REPLAY.md); optional future |
| **CMP-16** | Tempest Rising / modern RTS — presentation | Are ownership, orders, abilities and threats legible at 1080p, 4K and normal RTS camera distance? | Group labels/icons, power/food/affordability notifications, color-independent identity, HUD interaction and accessible scale | [HUD](RTS_HUD_ARCHITECTURE.md), [UI](POLITIKA_UI_OVERVIEW.md); HUD gates |
| **CMP-17** | Campaign/economic RTS hybrids — cross-system synthesis | Does a single operation connect information, infrastructure, combat and institutions? | Optional **rail junction / substation** proof once prerequisites exist; false report must not create real enemy; institutional action must have mandate | [Comparative Study](RTS_CROSS_GENRE_COMPARATIVE_STUDY.md), [Engine](ENGINE_OUTLINE.md); post-5 dependencies |

These **CMP IDs identify comparison research questions**, not executable test registrations, implementation milestones or versioned JSON schema. Actual acceptance test IDs and definitions remain solely in their existing owning documents.

## 3. Proposed initial measurement dimensions

| Quality | Candidate measurement | Acceptance condition to settle before running |
| --- | --- | --- |
| Interaction latency | Timestamp from input capture → admitted order → first visible outcome; p50/p95/p99 | Device, update policy, resolution and allowable delay explicitly fixed |
| Command correctness | Accepted/rejected orders versus legal outcomes; state/event consistency | **Zero unexplained illegal state transitions** in the bounded fixture |
| Navigation | Legal route success, invalid/partial reasons, collision, stuck time, path queries | All authored positive and negative cases have inspectable terminal/interrupt outcomes |
| Grouping | Keystrokes/clicks to form/recover groups and command units | No stale/foreign IDs; no duplicate issuer command from overlapping membership |
| AI fairness | Authorized observations and transactions per decision; plan changes | No cross-participant information or resource mutation, reason logged in development builds |
| Economy integrity | Account reconciliation and transaction retries | Zero negative/duplicated/overflowed balances; documented shortage recovery |
| Strategic depth | Distinct successful plans / matchup viability / response options | Playtested role/counter and non-dominant strategic alternatives across seeded maps |
| Legibility/accessibility | Identification/command success tasks by users at normal play distance | Define target cohort, device, task time and permitted errors; not only screenshots |
| Performance | CPU sim budget, render FPS and p95/p99 hitches, memory, nav workload | Reference hardware tiers and scene identity must be fixed before threshold claims |
| Reliability | Crash rate, return/rematch isolation, interrupted process/recovery | No stale participant/resource/order after legitimate teardown; tests on shipped target |
| Scenario and campaign | Mission completion/failure and comprehensible choices | A normal human can understand cause/effect and reach at least one legitimate result |
| Modding/authoring | Content-validator quality, scenario editing to running game | Referenced IDs and rights valid; malformed content rejected before match |

**Initial proposal only:** a 60-FPS desktop presentation target may be evaluated after specifying hardware and graphical presets. Do not turn 60 FPS, a unit ceiling, an AI strength claim or an input latency number into a current feature promise. Measure real geometry, 4K and handheld tiers independently.

## 4. Multi-system scenario gate — future only

The following is an example of testing **interactions**, not a demand to integrate every later mechanic into Stage 4F.

- **Preconditions:** legal map/starts, typed participant IDs, powered and unpowered site states, USD/food accounting, a formation, two observer-specific contact views and a separate Unaligned mandate.
- **Positive:** scouts receive time-stamped observations; units navigate and the player commits resources; an eligible facility outage alters actual powered service and dependent production; NPC patrols react through real observations/permissions; the match reaches an ordinary result.
- **Negative:** unauthorized sabotage/command is rejected; false report changes belief only; actor identity/selection survives neither destruction nor rematch; outage does not forge money or remove food arbitrarily; forbidden hidden target and NPC command cannot be issued through the local controller.
- **Recovery:** emergency generation or alternative access restores legitimate production; invalid report correction updates knowledge without retroactively changing earlier actual events; actor failures produce diagnostic and visible command feedback.
- **Proof:** pair deterministic event/state trace with rendered human interaction and log the scenario/build/content identity. An MP4 alone is not an oracle.

Dependencies are the completed 4F integrated match plus actual later visibility, formation, power and NPC implementations. **Mark CMP-17 BLOCKED until each dependency is separately accepted.**

## 5. Maintenance and data collection policy

Review this register upon a *verified* POLITIKA milestone, a changed benchmark reference release, a new representative gameplay mechanism, or a measured regression. No arbitrary cadence or recurring monitoring task is implied.

For each completed comparison capture:

| Field | Example format / interpretation |
| --- | --- |
| Benchmark ID | CMP-xx and exact owning acceptance test ID |
| Current source | Git SHA, branch, dirty marker |
| Engine/content | UE pin, map/definition version and content hashes |
| Game competitor | Title, exact patch, edition, mode, platform, citation |
| Hardware/input | CPU, GPU, RAM, OS, graphics profile, input device |
| Scenario | Unit count/type, map, objectives, initial state, seed |
| Observations | Source of measurement, timing units, sample count, thresholds, raw artifact paths |
| Negative cases | Invalid commands, hidden targets, loss/recovery, terminal/rematch |
| Verdict | Unmeasured / Baseline verified / Comparable observation / Specific improvement / Inconclusive / Blocked |
| Limitations | Unsupported platforms, missing data, workload mismatch, different game rules |
| Next owner | Owning document and stage; no new standalone roadmap |

**Acceptance discipline:** Current source and acceptance evidence override this research. If a feature exists as an Unreal class but its gameplay is unqualified, keep it **source-implemented / unqualified**, not PASS. If comparative information is stale, update the reference version and source before making a new claim. A comparison may generate a new test proposal but cannot silently redefine the authoritative milestones.

## 6. External and internal source boundaries

The [Cross-Genre Study §10](RTS_CROSS_GENRE_COMPARATIVE_STUDY.md#10-reference-sources-and-provenance) lists public first-party/developer references for Tempest Rising (including the 2026 Veti expansion), BAR, Zero-K, WARNO, Company of Heroes 3, Dune: Spice Wars, Sins II, AI War 2 and Unreal profiling. Game-specific deeper comparisons remain in the existing [C&C study](CNC_GAMEPLAY_COMPARATIVE_STUDY.md), [Tempest reference](TEMPEST_RISING_ENGINE_REFERENCE.md), [AI study](RTS_AI_ARCHITECTURE.md), [formation study](RTS_UNIT_FORMATIONS_AND_DISCIPLINE.md), [control group research](RTS_CONTROL_GROUP_INPUT_RESEARCH.md) and [HUD architecture](RTS_HUD_ARCHITECTURE.md).

This register does **not** authenticate third-party assets or authorize proprietary source import. Every asset requires [rights/provenance](../Archive/RIGHTS_AND_PROVENANCE.md) and [production standards](PRODUCTION_STANDARDS.md) before shipping.
