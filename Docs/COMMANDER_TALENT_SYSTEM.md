# POLITIKA — Commander Talent System: Ontology, Design and Implementation Roadmap

**Status:** PROPOSED DESIGN ROADMAP — 29 October 2026. **Not implemented, playtested, balance-approved or promoted to locked faction canon.**  
**Scope:** Match-local talent specialization for explicitly designated **Elite commander units only** in POLITIKA's classical RTS structure.  
**Implementation authority:** [Engine Outline](ENGINE_OUTLINE.md) owns the authoritative development sequence and promotion gates. This document owns the proposed commander-talent feature contract *if approved*; it does not amend the Stage 4A–4F exit requirements.  
**Design authorities:** [RTS Game Ontology](RTS_GAME_ONTOLOGY.md), [Concepts Outline](CONCEPTS_OUTLINE.md), [Match Structure](MATCH_STRUCTURE.md), [Faction Rosters](POLITIKA_FIVE_FACTION_ROSTERS.md), [Gameplay Content](GAMEPLAY_CONTENT.md), [Command and Simulation Contract](RTS_COMMAND_SIMULATION_CONTRACT.md), [UI Overview](POLITIKA_UI_OVERVIEW.md), [Operational Economy](RTS_OPERATIONAL_ECONOMY_AND_POWER.md), [Production and Technology](RTS_PRODUCTION_AND_TECHNOLOGY.md), [Visibility and Intelligence](RTS_VISIBILITY_AND_INTELLIGENCE.md), [Opponent AI](RTS_OPPONENT_AI.md), [Match Persistence and Replay](RTS_MATCH_PERSISTENCE_AND_REPLAY.md).

## 1. Product thesis and non-negotiable boundaries

POLITIKA should adapt the strategic structure of the *Heroes of the Storm* (HotS) talent system to a **small population of exceptional battlefield commanders**, while remaining an army/economy/territory-focused RTS inspired by classical *Command & Conquer*. A commander progresses through successive **mutually exclusive, match-local talent choices** that develop its leadership, reconnaissance, logistics, combat coordination, or doctrinal role. The system's primary function is to create *operational choices* rather than merely inflate individual combat statistics.

**Non-negotiable rules:**

1. **Only explicitly designated Elite commander archetypes may have talent trees.** Ordinary infantry, special forces, veteran troops, agents, medics, engineers, vehicles, structures and NPC units do not acquire talents by fighting, surviving or gaining ordinary veterancy. "Elite" as an adjective in a roster description is insufficient authorization; eligibility is a typed archetype property.
2. **Talent progression is confined to one admitted match generation.** Campaign meta-progression, cosmetic unlocks and ordinary technology upgrades remain separate. No hidden permanent hero-level progression.
3. **The faction's army earns progression; the commander makes the specialization choices.** Operational Experience (OXP) is initially a match-local faction-participant progression counter, **not** a new purchasable currency, commodity, spendable resource or alternative to the approved USD treasury.
4. **Talents cannot override any existing faction, map, scenario or legal-authority restrictions.** They do not grant forbidden nuclear refining/production, rival manufacturing trees, free espionage, unrestricted Intimidation, hidden intelligence or NPC Unaligned ownership. They cannot turn ordinary units into independently talented Heroes.
5. **One coordinator and one authoritative simulation.** Implement as extensions of validated definitions, existing participant authority, typed command admission, fixed-step scheduling and gameplay events. No parallel ability-command queue, second game clock, presentation-owned combat state or new standalone match controller.
6. **The RTS remains primary.** Talent choice is contextual, optional until the player commits, inspectable, and nonblocking for normal orders. Most benefits should alter conditional relationships between commander and eligible subordinate forces. No automatic match victory from a commander talent.
7. **Design does not imply acceptance.** Source classes, authored assets, UI, playtesting and package qualifications must be implemented and evidenced independently.

## 2. Comparative ontology: what to borrow from Heroes of the Storm

Blizzard's [Core Concepts: Introduction to Heroes of the Storm](https://news.blizzard.com/en-us/article/20720242/core-concepts-introduction-to-heroes-of-the-storm) describes a **shared team experience pool**, successive Hero-specific talent choices, and a major Heroic Ability choice at level 10. The conventional HotS tier progression is **1 / 4 / 7 / 10 / 13 / 16 / 20**, with level-20 upgrades; see the [Heroes of the Storm talent reference](https://heroesofthestorm.fandom.com/wiki/Talent). These exact numbers are **source-game facts, not POLITIKA requirements**.

| Ontological element | HotS | Proposed POLITIKA translation |
| --- | --- | --- |
| Progressing subject | Shared team level | Shared *match-participant* OXP tier entitlement |
| Specializing subject | Individual playable Hero | Explicitly eligible Elite commander appointment/instance |
| Decision | One talent chosen from alternatives within a tier | Authoritative, once-only commander talent selection |
| Ability modification | Hero skills, attacks, attributes and utility | Commander abilities, command coverage, targeting support, conditional subordinate effects |
| Timing | Team-earned Hero level thresholds | OXP thresholds calibrated against actual RTS match pacing |
| Opportunity cost | Competing choices and foregone builds | Mutually exclusive commander operational doctrines |
| Major power spike | Heroic Ability/Storm-tier choices | Bounded signature command capability at final tier |
| Counterplay | Opposing composition, terrain and tactical responses | Reconnaissance, distance, counter-battery, logistics, disruption and commander vulnerability |
| Persistence | During one HotS match | During one POLITIKA match; explicit policy for commander death/replacement |
| Unit scale | One Hero controlled by each player | An army with **at most one active command appointment per participant initially** |

**Transfer the decision structure, not the entire MOBA progression economy.** POLITIKA does not need HotS levels, seven talent tiers, a Hero item shop, a Hero-focused win condition, or unrestricted resurrection.

A useful secondary reference is *Warcraft III*: its Heroes and army-supporting auras illustrate the feasibility of a commander affecting subordinate units without those units independently becoming Heroes. POLITIKA should implement the underlying command relationship in its own existing rules.

## 3. Formal ontology and ownership

### 3.1. Classes, instances and relations

| Concept | Meaning and owning state |
| --- | --- |
| Commander eligibility | Boolean/typed role attached to a **unit archetype definition**; explicitly validated |
| Commander definition | Stable-ID definition linking eligible archetype, faction, available talent tree and permissible command scope |
| Talent definition | Stable-ID authored description of a choice, effect types, constraints, UI metadata, prerequisites |
| Talent tree | Versioned ordered tiers; valid talent IDs, exclusivity, prerequisites and permitted effects |
| OXP ruleset | Authored award policy, event classes, caps, anti-farm constraints, tier thresholds, match-mode policy |
| Participant OXP state | Match-local total and unlocked tiers under canonical participant identity and match generation |
| Commander appointment | Match-local relation between an eligible commander and its permitted participant/controller; has stable ID distinct from an Actor pointer |
| Commander talent state | Which choices were committed for which appointment, with source references and effect handles |
| Commander command relation | Permission/scoped influence between a commander and a subordinate unit at simulation step N |
| Talent selection request | Typed **intent** submitted by a human or AI through existing command admission |
| Talent effect | Code-owned, deterministic modifier or grant conditional on the authoritative current state |
| Progression/selection event | Auditable state transition; published once with appropriate per-participant visibility |

A commander archetype is not the same as a commander instance; an appointment is not a player or faction; a talent definition is not live talent state; an award of OXP is not a selected talent; an accepted request is not an executed effect.

A talent may change the operational capabilities of ordinary units *via a valid commander-to-subordinate relationship*, but does not add talents, independent experience bars or talent-selection permissions to those units.

**Required relationship tests:** canonical unit owner vs. current controller vs. source faction vs. acquisition method vs. match participant. Do not confuse the faction badge on a unit with its authorized issuer. The [Match Structure](MATCH_STRUCTURE.md) rules continue to apply: 2–4 selectable faction slots, one independent NPC-only Unaligned authority; no fifth playable faction.

### 3.2. Selection invariant

For participant P, appointment C, talent T and tier K, a valid selection requires all of:

- Current admitted match generation is **Active**, and the command is issued by P through its legitimate controller/AI authority.
- C refers to a live, legally controllable **eligible commander appointment** belonging to P under the declared death/replacement policy.
- T belongs to the validated talent tree assigned to C; K is unlocked through authoritative OXP and is not already spent.
- All definition prerequisites and incompatibilities hold, including faction/scenario permission and any valid prior talent reference.
- The same request/sequence cannot commit twice; invalid or stale requests do not alter OXP, selections, effects, resources, or cooldowns.

An admitted selection commits one tier exactly once, then grants code-defined capability/effect handles and emits one authoritative selection result. All HUD changes project that result; no widget/Blueprint directly grants the talent.

### 3.3. Commander influence invariant

An aura or command modifier applies to unit U only while its effect's explicitly authored predicate is satisfied:

- C remains valid and operational (or follows a separately specified retained-effect rule).
- U is an eligible friendly subordinate under the current **controller**, owner/contract rights, unit category and scenario rules.
- U lies within the effect's declared radius / formation / communications coverage, with applicable terrain, network and line-of-sight restrictions.
- Required power, communications, supplies, fuel, target observation, ability availability and cooldown conditions hold where the talent uses them.
- An enemy, Unaligned NPC or newly captured unit never inherits bonuses merely because it happens to be nearby or shares an old faction label.

Use an event-driven or fixed-step recomputation policy with bounded cost; distinguish an *effect source* from the final combined effective value to avoid permanently mutating the archetype baseline.

## 4. Proposed experience and tier policy

### 4.1. Four tiers, not seven

Start with four tiers, each with **two or three** mutually exclusive choices. **Tier I** should be selectable on deployment, without needing an OXP grind; tiers II–IV unlock through faction OXP.

| Tier | Purpose | Provisional intended pacing |
| --- | --- | --- |
| **I — Initial Doctrine** | Commit to an early role: recon, maneuver, defense, coordination | At deployment |
| **II — Field Specialization** | Respond to discovered threats and current map conditions | Early-to-mid match, approximately first 20–30% of representative duration |
| **III — Advanced Command** | Deepen a path or introduce a deliberate counter-specialization | Approximately 45–60% |
| **IV — Signature Operation** | Gain a bounded high-impact command option with visible counters | Approximately 70–85% |

The percentages are **tuning hypotheses**, not elapsed-time auto-unlocks. A short scenario must remain viable, and a match ending early need not guarantee all tiers. Select thresholds from observed match length and award telemetry; do not invent fixed XP values now.

Three choices at four tiers give at most **3^4 = 81** selection sequences per commander. This is a count of combinations, **not** a promise that all are strategically distinct, valid or balanced.

### 4.2. OXP award contract

OXP is a nonnegative integer, accounted against the match participant and generation. Initial eligible sources:

| Authoritative achievement | Tentative award principle | Required anti-farm/validation rule |
| --- | --- | --- |
| First legitimate capture of a valuable objective | High | Objective and capture instance IDs; repetition cap |
| Completion of authored operational task | High | Award only from actual one-shot objective completion |
| Destruction of qualifying enemy strategic asset | Moderate | Valid hostile asset and source event; no duplicate death payout |
| Successful meaningful defense | Moderate | Real hostile pressure and qualifying defended site; cooldown and cap |
| First useful reconnaissance of a relevant location | Low/moderate | Per-participant knowledge/history; no duplicate sighting loop |
| Ordinary combat participation | Low and capped | Bounded contribution, no idle attack/repair farm |
| Passive time, money accumulation or routine clicks | **No reward** | No passive OXP drip by default |
| Damage to civilians/neutral infrastructure for exploitation | **No reward** | Respect eligibility, legal/hostility and scenario consequences |

**Do not award XP based on claimed intelligence.** POLITIKA distinguishes simulation truth, observed information, stale sightings and planted reports. A false Unaligned report may affect a player's decisions but cannot by itself fabricate objective completion or progression.

Award events use stable source IDs, deterministic ordering and an idempotence ledger. A repeated actor death, canceled objective, imported stale event or duplicated submission must not grant OXP again. No OXP transaction ever debits or credits USD, food, electricity or commodity stock.

### 4.3. Party and commander cardinality

The initial ruleset allows **one active Elite commander appointment per playable participant**, to limit micromanagement and global stacking. The participant's OXP unlocks tiers for that appointment and the player's selected build remains per appointment; OXP is not independently earned by each unit. The four canonical factions may each have a distinct candidate archetype, but the faction/slot model remains unchanged.

Future multiple-commander rulesets, ally sharing, global team XP, scripted officer succession and campaign persistence are **out of scope**. In the standard initial skirmish, participant OXP is not pooled across opponents or allied participants unless a later explicitly approved ruleset changes this.

### 4.4. Commander death, redeployment and capture

**Recommended default, subject to scenario approval:**

- The *participant's earned OXP* and already committed *appointment talent choices* survive a commander unit's death within the same admitted match generation.
- The dead commander provides **no live effects**, command aura, active targeting or phantom ability use. Clear its active effects and cooldown execution handles safely.
- Redeployment is **not automatic**; any replacement requires a legitimate eligible production/mission rule, availability cap, cost, power and food constraints, and timing. The approved replacement resumes the same appointment/build after spawning.
- If replacement is prohibited or permanently disabled in that scenario, the stored build remains historical state but cannot produce effects without an authorized commander.
- Campaign missions can expressly declare a named commander permanently lost or mission-critical. No unconditional Hero resurrection.
- Enemy capture/Intimidation cannot silently grant ownership of a commander's talent appointment or all its faction-wide OXP. **Default: Elite commanders are ineligible for forced control conversion** until a narrowly specified transfer rule is designed and tested. This is a proposed talent-system constraint; harmonize with the existing eligible-target matrix before promoting it.
- On terminal phase, rejection, return to menu or rematch, all OXP state, choices, modifiers, pending commands, source handles, cooldowns and ledger entries are discarded with the previous match generation.

This balances meaningful commander losses against the excessively punitive effect of losing all earned development in one kill.

## 5. Faction-specific commanders and initial talent catalogue

The [proposed faction rosters](POLITIKA_FIVE_FACTION_ROSTERS.md) already contain candidates. Their presence in that document **does not** mean they are implemented, designated as mechanically eligible, or balance-approved.

| Playable faction | Candidate ID | Proposed talent identity | Canon approval needed |
| --- | --- | --- | --- |
| Atlantic Concord | **AC-INF-14** — Special Operations Commander | Precision, surveillance, secure communications, combined-arms coordination | Approve talent eligibility and exact effects |
| Red Directorate | **RD-INF-13** — Special Forces Colonel | Formations, artillery observation, discipline, reserve organization | Approve talent eligibility and exact effects |
| Iron Ascendancy | **IA-INF-13** — Special Operations Raider | Assault tempo, suppression, isolation, offensive concentration | **Explicitly approve this unit as a commander**, not merely an Elite raider |
| Meridian Pact | **MP-INF-13** — Foreign Military Adviser | Mobile command, contractors, logistics, tactical improvisation | **Explicitly approve this unit as a commander**, not merely an adviser |

The Unaligned remains an NPC-controlled federal/system authority. It has **no player-selectable talent tree** under this specification. Its existing counterintelligence capabilities are not replaced by a fifth talent catalogue.

### 5.1. Illustrative four-tier builds (NOT final stats)

| Tier | Atlantic Concord | Red Directorate | Iron Ascendancy | Meridian Pact |
| --- | --- | --- | --- | --- |
| I | **Advanced Communications:** command coverage improvement | **Formation Discipline:** eligible cohesion support | **Shock Leadership:** close-assault coordination | **Mobile Command:** mobile-unit coordination |
| II | **Forward Observer:** authorized target designation | **Artillery Coordination:** coordinated indirect fire | **Suppression Doctrine:** bounded suppression support | **Contract Coordination:** authorized contracted response |
| III | **Coordinated Assault:** temporary formation synergy | **Operational Reserves:** organize existing available reserves | **Isolation Tactics:** encourage coordinated attacks against truly isolated targets | **Raiding Doctrine:** improve eligible hit-and-withdraw operations |
| IV | **Joint Operations Network:** timed observed-target coordination | **Combined Front Offensive:** temporary synchronized formation action | **Concentrated Breakthrough:** high-tempo assault with exposure trade-off | **Rapid Contract Mobilization:** bounded improvement to legitimate contract response |

These are **one coherent sample path per faction, not a complete mutually exclusive choice set**. Each tier ultimately needs 2–3 authored alternatives with actual target conditions, duration, cooldown, costs and counterplay.

**Protected faction canon:**

- Concord does not obtain automatic omniscience, unconstrained Psy-Ops, unlimited airstrikes or free nuclear bomber deployment.
- Directorate does not lose its existing infantry-price doctrine or gain free manpower by selecting a commander talent.
- Ascendancy cannot bypass limited spy progression, spy-only sabotage permissions, the full Discipline/Morale-break condition for Intimidation, conditional nuclear research theft, or its special first-bomb victory prerequisites.
- Meridian cannot produce restricted nuclear materials, gain advanced propaganda, create rival faction factories or ignore Black Market contract requirements.
- The same underlying Discipline/Morale model, food upkeep, dollar purchases, minimum building power and real ownership/controller relations continue to govern effects.

### 5.2. Effect design grammar

Favor command-system choices over raw combat-stat amplification. Initially implement effects in this order:

1. **Passive commander-only scalar:** a typed, bounded modification such as command radius, subject to an effect clamp.
2. **Conditional subordinate support:** an eligible unit receives a temporary calculated modifier while inside the command relationship; no write to its unit definition.
3. **Active targeted command operation:** a normal admitted ability with target legality, range, observation, cooldown, interruption and authorization.
4. **Signature tier effect:** bounded multi-unit operation with finite duration, clear warning, meaningful counterplay and finite refresh/stacking policy.

Every effect definition must identify affected entity classes, eligibility predicate, value type, duration/timebase, stacking group and order, source/expiry policy, visibility policy, gameplay counters, deterministic evaluation and presentation key. Avoid a generic arbitrary scripting hook in v1.

An ability never inherits a cheap/free cost simply because a talent unlocked it: declare and validate its existing resource/technology/visibility constraints.

### 5.3. Formation-scoped effects (proposed, deferred)

[Military Formations, Tactical Stances, Cohesion and Discipline](RTS_UNIT_FORMATIONS_AND_DISCIPLINE.md) models formations independently of commander eligibility. Explicitly eligible Elite commander talents may eventually grant bounded rally/reorganization, coordination or recovery benefits **only to currently qualifying subordinates** inside valid commander relationships/communications coverage. They do not give ordinary soldiers talent trees or create a second Discipline meter. Loss of appointment, subordinate death/detachment, capture, communication loss or match termination invalidates effects without permanently rewriting archetype statistics. Exact talent effects and balance values remain open.


## 6. Unreal / C++ architecture and data contracts

### 6.1. Authoring and definitions

Extend the **existing** stable-ID, versioned gameplay-content architecture in [Gameplay Content](GAMEPLAY_CONTENT.md), preferably with dedicated **UPrimaryDataAsset** definitions where asset-bundle/asset-manager loading is useful. Candidate types (design names, **not existing classes**):

- **CommanderDefinition:** stable ID, schema version, faction, eligible unit archetype ID, unique-per-participant policy, replacement policy, tree ID, command-capability constraints.
- **CommanderTalentTreeDefinition:** tree ID, version, ordered tier records, number of choices per tier, allowed talent ID references, prerequisite compatibility.
- **CommanderTalentDefinition:** stable ID, tier, faction/archetype requirements, localized name/description, icon, effect specification references, active/passive kind, targeting/counterplay and optional cooldown/consumable policy.
- **CommanderProgressionRules:** mode/ruleset ID, allowed award event classes, integer thresholds, bounded award values, duplicate/cap policies and activation phase.

Runtime state belongs to the authoritative match/simulation coordinator, not mutable data assets:

- **ParticipantProgressionState:** match-generation ID, canonical participant ID, integer OXP total, unlocked tier set, deduplication/award ledger.
- **CommanderAppointmentState:** appointment stable runtime ID, linked entity ID or empty, controlling participant, selected talent IDs by tier, redeployment policy/state, effect-source handles.
- **ActiveCommanderEffect:** source appointment/talent ID, target relationship and scope, start/end simulation steps, applicable effect and recomputation/expiry state.

All runtime records should be serializable by **stable IDs, schema versions and step counts**, not raw pointers, localized strings, UI widget IDs or transient asset memory addresses. Preserve the current source-of-truth separation among authored definitions, instances, owner/controller identity, projected knowledge and UI.

### 6.2. Admission and execution

Add **one typed commander talent selection request** to the existing canonical command domain (extend the existing typed request/variant when implementation begins). Do not put talent mutations directly in the player controller or create a second order pipeline.

Proposed order:

1. Input/UI/AI selects a valid appointment and talent ID, producing a typed request with issuer, match generation, command sequence, appointment ID, tier ID and talent ID.
2. Existing coordinator admits/rejects against participant authority, active match phase, catalog version, unlocked/unspent tier, controller, archetype, prerequisites and deduplication.
3. On acceptance, commit the selected talent ID exactly once in a fixed-step state transition; install effect source handles or grant a code-owned capability; emit correlation and authoritative event.
4. Execute active commander abilities through the existing ability/order processing with their own legal target checks, cooldowns and interruption outcomes.
5. Presentation reads an authorized commander/progression snapshot. It never applies buffs, drains OXP or creates a unit as a side effect of rendering a button.

The existing **0.05-second coordinator step** is the initial scheduling reference for effects and OXP. No new free-running Actor Tick, independent per-commander gameplay clock or render-frame-based duration logic. If the command-domain variant must change, preserve existing five prototype commands and extend their regression tests; do not replace a proven command boundary for this feature.

### 6.3. Effect stacking and reversibility

Before authoring live buffs, define a **single modifier aggregation rule**: baseline attribute, filtered relevant active sources, typed additive/multiplicative stages, rounding policy, bounds/clamps and deterministic source order. The exact arithmetic belongs to the owning combat/command simulation contract.

- Same-scope commander auras must not multiply endlessly through unit overlap; v1 uses the **strongest eligible source in each exclusive stacking group**, or a specifically authored nonstacking rule.
- A unit leaving radius, switching controller, dying, losing communications, or losing the required commander must immediately stop receiving that source's conditional effect under the declared evaluation schedule.
- Deactivating a source must not damage or alter permanently the unit archetype, faction technology, currency ledger or permanent health cap unless an explicit code-owned rule says so.
- A replay, restart, or replacement spawn must rebuild effects **once**, without additive duplication.

### 6.4. GAS decision

Epic's [Gameplay Ability System](https://dev.epicgames.com/documentation/unreal-engine/gameplay-ability-system-for-unreal-engine) offers abilities, attributes, effects, tags, costs and cooldowns. It is a legitimate later candidate for complex commander effects. **Do not adopt GAS or migrate ordinary combat merely because this design introduces talents.** First qualify one passive and one active effect through the existing C++ command/simulation boundary; adopt GAS only after a focused comparative prototype demonstrates a net integration benefit and does not duplicate state ownership.

Use UE's [Data Validation](https://dev.epicgames.com/documentation/unreal-engine/data-validation-in-unreal-engine) for asset authoring diagnostics, and follow [UPrimaryDataAsset](https://dev.epicgames.com/documentation/unreal-engine/API/Runtime/Engine/UPrimaryDataAsset) conventions where used. Keep editor-only validation dependencies out of Shipping.

### 6.5. Invalid content rejection

Catalog and scenario validation must fail before match admission for:

- Missing/duplicate/tombstoned talent or tree IDs; schema mismatch; dangling commander/archetype/faction references.
- Missing/duplicate tier index; no legal choice; talent in the wrong tier; cycle or unreachable prerequisite; mutually exclusive selections required together.
- NaN/infinite/negative or invalid duration/range/cooldown/stacking parameter; unknown effect type or missing UI/loc keys if mandatory.
- Talent granting a forbidden faction capability, unsupported weapon profile, nonexistent target class, ineligible ordinary-unit promotion or invalid scenario commander assignment.
- No legal starter commander appointment/starting deployment when the selected scenario explicitly requires one.

Emit stable diagnostic codes with asset ID, path, field, expected/actual and severity. A bad optional tree cannot silently fall back to a stronger unrestricted talent.

## 7. UX, AI, knowledge and match modes

**UI:** extend the contextual selection/command panel from [UI Overview](POLITIKA_UI_OVERVIEW.md) and [Practical RTS UI](RTS_UI_SETTINGS_AND_KEYMAP.md). Eligible commander selection shows portrait/identity, tier progress, pending choice badge, compact 2–3 option descriptions and cooldowns. The battlefield remains operable; opening the panel cannot leak a movement/attack input or trap focus. Never show an empty talent tree for an ordinary unit. Provide keyboard/controller/touch paths consistent with platform settings, dynamically mapped hotkeys, screen readers/text equivalents, and test 1280×720 before larger display profiles.

**Feedback:** distinguish **tier earned**, **selection requested**, **accepted/rejected**, **ability started**, **effect active/interrupted**, and **expired**. Tooltips explain eligibility, exact effect, area, duration, cooldown, cost, operating requirements and counterplay. Display a reason for disabled choices; don't expose live enemy hidden choices through inspect panels.

**Visibility:** player and AI must see only authorized observations. A visible enemy ability may reveal an observable effect; an undiscovered tree choice or hidden commander location remains secret unless a proper intelligence event reveals it. An opponent can counter an observed aura with tactical action, not omniscient map data.

**AI:** use the same participant OXP awards, legal selection requests and admitted active abilities as the human. Initially choose among 2–3 authored options with a deterministic, bounded policy based on authorized observations, faction doctrine and scenario role. No special AI instant unlock, fabricated experience, secret enemy builds or direct effect grant.

**Modes:** first release is offline skirmish after Stage 4F; authored campaigns may later override tier pacing, deployment/replacement and available talent sets. Replay/save compatibility and multiplayer are deferred: design stable payloads and versioning now, but **do not claim those features work**.

## 8. Bounded implementation roadmap — after First Playable Skirmish

**Dependency gate:** do **not** add talent work to the required Stage 4A–4F closure. First qualify existing participant authority, 4B dollar economy/power/food/production, 4C ordinary multiunit commands, 4D opposing AI, 4E victory/defeat, and 4F repeatable packaged skirmish according to [Engine Outline §7.5](ENGINE_OUTLINE.md#75-stage-4--first-playable-skirmish-repeatable-offline-match). This is a feature-roadmap sequence, **not** a new numbered Engine Outline stage or claim of completion.

| Increment | Smallest bounded change | Acceptance gate and evidence |
| --- | --- | --- |
| **T0 — Design approval** | Approve commander eligibility for four candidates; agree appointment/death policy, OXP sources, thresholds as tunables, prohibited bypass matrix | Faction owner review; explicit canonical decisions recorded without touching runtime |
| **T1 — Catalog/eligibility** | One commander prototype, one versioned talent tree, two Tier-I passive options, editor/shared content validation | Ordinary units ineligible; duplicate/missing/invalid trees fail before admission; canonical behavior unchanged without feature |
| **T2 — Request/commit** | One typed SelectTalent request via existing command boundary; one selected tier recorded at authoritative step | Legitimate accept once; stale/foreign/duplicate/wrong-phase/tier-spent requests reject with zero effects |
| **T3 — OXP/tier unlock** | One event source (objective completion), integer OXP, two tier thresholds, idempotent award accounting | Exact thresholds, no duplicate event reward, different render schedules same event/state results |
| **T4 — Conditional support** | One commander-only passive + one supported friendly-unit command modifier with range and expiry | Modifier applies only to valid current subordinates; removal on loss/exit/ownership change; deterministic stacking |
| **T5 — Active ability** | One targeted commander operation with real prereqs/cooldown and interruption | Existing ability/order admission, correct execution/rejection events, power/visibility/target restrictions retained |
| **T6 — UI** | Selected-commander panel, choices, explanations, key/controller navigation and feedback | Rendered Development package; buttons cannot mutate authoritative state; ordinary-unit/group commands unaffected |
| **T7 — Loss/replacement** | Appointment persistence and legal replacement path; terminal/reset clearing | Death removes live effects, approved replacement retains choices only once; no stale state after rematch |
| **T8 — AI/faction expansion** | Four approved commanders, basic legal AI choice policy, representative faction talents | No faction bypass; correct observations; reproducible AI selection; 2–4 player and Unaligned constraints retained |
| **T9 — Balance and release gate** | Calibrate match-duration tiers, audit stacking, counters and costs; package and device profiling | Named scenarios, source/UE pin, clean build, Linux Development/Shipping content isolation, no regressions; other platforms separately qualified |

**First coding prompt (T1–T2 only):** Implement one Elite-commander prototype definition with a validated two-choice initial talent tier, a typed selection intent through the existing coordinator, an authoritative once-only selection record, correlated accept/reject events, and negative tests. Do **not** implement OXP, commander death/replacement, complete four-faction trees, GAS migration, new HUD screens or unrelated gameplay rules in that first slice.

## 9. Acceptance matrix (minimum cases)

| ID | Invariant / scenario | Expected verdict |
| --- | --- | --- |
| **TAL-01** | Ordinary rifleman, veteran/specialist or vehicle submits a talent request | Rejected; no changes to any unit or match state |
| **TAL-02** | Eligible commander selects legal Tier I talent during Active | Exactly one committed selection, one result/event |
| **TAL-03** | Same tier selected twice, concurrent choices or replay of request ID | First legal selection wins according to admitted order; duplicates rejected/no effect |
| **TAL-04** | Wrong participant, stale match generation, old commander entity or NPC Unaligned issuer | Rejected with zero side effects |
| **TAL-05** | Request in SetupPending/AdmissionRejected/terminal or return-to-menu | Rejected; no latent effects or queued work |
| **TAL-06** | Invalid tree reference, duplicate ID, cycle or illegal faction ability | Actionable content validation; atomic admission rejection |
| **TAL-07** | Objective completion award versus repeated duplicate event | Single OXP grant, exact integer total |
| **TAL-08** | Objective recapture, friendly/civilian farming, idle timer | No repeated/fabricated OXP beyond explicit cap/policy |
| **TAL-09** | Tier locked, threshold just crossed, pending multiple tiers | Correct availability; no automatic choice or accidental extra grant |
| **TAL-10** | Command aura with eligible friendly versus enemy/neutral/non-eligible unit | Only authorized subordinate receives effect |
| **TAL-11** | Move out of range, break comms, death, ownership transfer or interrupt | Effect removed/paused exactly once under rule |
| **TAL-12** | Two overlapping same-group effect sources | Stacking policy honored, order-independent when intended, clamped |
| **TAL-13** | Same command/OXP fixture under varied render frames | Identical qualifying authoritative event/order/state sequence |
| **TAL-14** | Commander active ability with invalid target, stale visibility or insufficient prerequisites | Rejection with no cooldown/cost charge |
| **TAL-15** | Active ability interrupts, expires or cancels | One terminal outcome; no lingering buff or repeated debit |
| **TAL-16** | Selection during loss/redeploy or attempted enemy commander conversion | Explicit appointment policy enforced; no tree theft |
| **TAL-17** | Repeat match / invalid startup / clean rematch in same process | No previous-generation OXP, choices, event ledger or effect sources |
| **TAL-18** | Faction restriction regression: nuclear, espionage, Intimidation, contracts | No talent-based bypass of existing canon |
| **TAL-19** | AI progression/choice using projected knowledge | No omniscience, direct grants or rule exemptions |
| **TAL-20** | Cooked HUD/gamepad/keyboard and ordinary mixed selection | Select/inspect/cancel/focus, legibility and commands remain correct |
| **TAL-21** | Content-package validation and Shipping fixture isolation | Production content present, test-only assets excluded; no fabricated acceptance |
| **TAL-22** | 2–4 playable participants plus distinct Unaligned authority | Per-participant progression isolation and original roster invariants remain valid |

Every implementation increment should report source commit SHA, exact UE 5.8.3 revision/pin if still current, content/asset hashes, commands executed, fixture IDs, observed verdicts, and Linux/package scope; **PASS, FAIL, BLOCKED and INCONCLUSIVE** must not be conflated. Preserve existing worktrees and authored assets, and qualify changes in an isolated development worktree where the implementation workflow uses one.

## 10. Balancing and telemetry plan

Track at least: match duration and outcome, commander deployment/death/replacement time, OXP by event category, tier unlock timestamps, selection timing, choice frequency, tier reach rate, talent effect uptime, effective unit coverage, active ability usage, counterplay interruption, and win rates by faction/map/build **within adequate sample sizes**. Avoid logging private user identifiers or per-user secrets. Analyze early snowballing, "always-pick" talents, choices left unselected, and high-micro burdens on RTS control.

Initial balance constraints:

- Most talents create **conditional tactical opportunities**; avoid unconditional global army damage/health increases.
- Each tier has at least two contextually plausible options with clearly different benefits.
- Signature operations are finite, telegraphed, interruptible/counterable and never trigger special victory outcomes by themselves.
- Keep OXP pacing credible in short and long scenarios; a player should not need repetitive "farming" of a trivial action.
- Heavy logistical and communications prerequisites remain meaningful; a powerful commander does not eliminate resource or territorial strategy.
- Individual talents and full-build combinations receive separate tests; 81 theoretical combinations do not imply 81 viable strategies.

## 11. Decisions explicitly left open

1. **Commander eligibility approval:** Are IA-INF-13 and MP-INF-13 formally designated Elite commanders, or will different roster archetypes fill that role? AC-INF-14 and RD-INF-13 also require explicit talent approval.
2. **One-command appointment rule:** Is one active commander per faction sufficient for all intended ordinary skirmishes? **Recommended yes initially.**
3. **Tier timing and OXP award magnitudes:** Requires representative recorded match durations; no locked numerical values yet.
4. **Talent-choice count and prerequisites:** Begin 2-choice T1 slice, expand to 2–3 per tier after the underlying predicates and UI are proven.
5. **Loss/replacement pricing and permanence:** Requires coordination with production, food, power and campaign mode; default policy above is provisional.
6. **Stacking/arithmetic:** Exact order, clamping, rounding and percentage policy must be resolved in the owning simulation layer.
7. **GAS adoption:** Deferred pending a focused cost/benefit prototype against the existing C++ code.
8. **Multi-commander, cross-ally shared XP, respec, persistent career progression, spectator inspection and multiplayer:** Explicitly deferred, not implicit v1 commitments.
9. **AI and competitive tuning:** No assumption of fairness or balance until instrumented playtests show it.

## 12. Reference sources

**External design and UE documentation**

- [Blizzard — Heroes of the Storm: Core Concepts](https://news.blizzard.com/en-us/article/20720242/core-concepts-introduction-to-heroes-of-the-storm) — team XP, successive Hero-specific talent choices, level-10 Heroic Ability and opponent adaptation.
- [Blizzard — Core Concepts: Team Leveling](https://news.blizzard.com/en-us/article/20845104/core-concepts-team-leveling-part-1) — objective-driven shared team progression and talent power spikes.
- [Heroes of the Storm Wiki — Talent](https://heroesofthestorm.fandom.com/wiki/Talent) — conventional seven-tier reference sequence; secondary reference, not design authority.
- [Epic — Gameplay Ability System (UE 5.8)](https://dev.epicgames.com/documentation/unreal-engine/gameplay-ability-system-for-unreal-engine) — optional later ability/effect architecture.
- [Epic — Data Validation (UE 5.8)](https://dev.epicgames.com/documentation/unreal-engine/data-validation-in-unreal-engine) — content validation mechanisms.
- [Epic — UPrimaryDataAsset (UE 5.8)](https://dev.epicgames.com/documentation/unreal-engine/API/Runtime/Engine/UPrimaryDataAsset) — stable asset-management authoring pattern.

**Project-specific authorities**

- [Engine Outline](ENGINE_OUTLINE.md) — **authoritative implementation order**, notably First Playable Skirmish.
- [RTS Game Ontology](RTS_GAME_ONTOLOGY.md) — existing entity/action/authority/time/knowledge semantics.
- [Match Structure](MATCH_STRUCTURE.md) — faction/participant/Unaligned boundaries.
- [Faction Rosters](POLITIKA_FIVE_FACTION_ROSTERS.md) — provisional commander candidates and protected asymmetry.
- [Gameplay Content](GAMEPLAY_CONTENT.md), [Command Contract](RTS_COMMAND_SIMULATION_CONTRACT.md), [UI Overview](POLITIKA_UI_OVERVIEW.md) — implementation and presentation boundaries.
- [Operational Economy](RTS_OPERATIONAL_ECONOMY_AND_POWER.md), [Production](RTS_PRODUCTION_AND_TECHNOLOGY.md), [Visibility](RTS_VISIBILITY_AND_INTELLIGENCE.md), [Opponent AI](RTS_OPPONENT_AI.md) — restrictions that talents must not bypass.

---

**Promotion rule:** This document may guide design and implementation tasks, but **only explicit approval of the open decisions plus passing code/content/runtime acceptance** can promote its proposals into authoritative POLITIKA gameplay. It is not a report of working commander talents, a revision to existing Stage 4 gates, or permission to bypass another feature owner's specification.
