# POLITIKA — Military Formations, Tactical Stances, Cohesion and Discipline

**Status: CORE FEATURE DESIGN / PROPOSED — 29 October 2026. NOT IMPLEMENTED OR ACCEPTED.**

**Authority:** [Engine Outline](ENGINE_OUTLINE.md) owns implementation sequence and acceptance promotion; [Selection and Group Orders](RTS_SELECTION_AND_GROUP_ORDERS.md) owns local selection/control groups and multi-issuer order semantics; [Propaganda and Morale](PROPAGANDA_AND_MORALE.md) owns the *single* per-unit `UnitResolve`; [Runtime Navigation Acceptance](RTS_RUNTIME_NAVIGATION_ACCEPTANCE.md) owns existing route evidence. This specialist document owns **proposed** persistent formation identities, shapes, tactical stances, cohesion, eligibility, interaction boundaries and FORM acceptance. The schema examples, balance functions and type names herein are design candidates, not code already present in the project.

## 1. Foundational military ontology

An RTS **unit** is an individual match-state entity with identity, owner/controller, position, health, movement capability, weapons, active orders and an individual resolve value. A **selection set** is a local filtered collection for input; a **control group** is a saved, generation-tagged local selection shortcut. Neither is a military organization, and neither can grant an army-size Discipline effect by itself.

A **formation** is a *persistent, match-owned tactical relationship* among eligible individual units, expressed by membership, roles, shape, anchor/facing, slot assignment, combat stance, adherence and lifecycle state. Each member retains its own command admission and execution. A **formation definition** supplies immutable, validated authored constraints; a **formation instance** records current organization. A **formation slot** is a preferred *legal destination*, not a teleport or a guarantee that occupancy/path permits it. **Cohesion** is the derived degree to which real, eligible, operational members satisfy intended spatial organization. **Discipline** is the existing per-combatant display of the unit's own `UnitResolve`.

**Non-negotiable rules:**

1. Only an **explicit admitted** Form/Reform/Detach/Disband operation establishes or modifies a persistent formation. An arbitrary player hotkey group does not.
2. The existing match coordinator controls all authority, typed admission, 0.05 s steps, movement, combat and event publication. No second simulation scheduler, per-formation Actor Tick, unvalidated UObject identity, UI-owned health/position or hidden authority.
3. Group movement preserves **per-member legal route and individual rejection**. An incompatible/stale member cannot invalidate unrelated valid requests, and a rejected order cannot overwrite a preceding valid route.
4. Formation shape, stance, adherence and explicit Move/Attack/Attack-Move/Stop/Guard orders are *independent concepts*. Persistent stance policies must not silently become a new order or replace existing ones.
5. Formation support arises from **actual nearby, coherent, operational allied combatants** and is bounded. Selection count, roster count, or two distant armies cannot manufacture support.
6. The same **one `UnitResolve`** drives combatant Discipline and civilian Morale. A faction's Population Morale is its existing distinct aggregate; formation Cohesion is derived organizational state, **not another resolve meter**.
7. Existing faction distinctions are mandatory: normal formation-size support for **Red Directorate, Iron Ascendancy, Meridian Pact and NPC-only Unaligned**; extra very-massive coherent-army stability **only for Directorate and Ascendancy**; **Atlantic Concord** has no size penalty solely for individual/very-small-group deployment. Civilians never receive these military bonuses.
8. Fragmentation/low Discipline must **not automatically trigger** faction-exclusive Concord Psy-Ops elimination or Ascendancy Intimidation conversion. Their specialized eligibility, `BrokenThreshold`, isolation and effects remain independently checked.
9. Documented proposed shapes, formulae, thresholds, hotkeys and performance targets are *candidates*, not completed or balanced mechanics.

**AI integration:** The [RTS AI Ontology and Architecture](RTS_AI_ARCHITECTURE.md) proposes tactical formation decision responsibilities only *after* the independent FORM gates and ordinary Stage 4D first-opponent acceptance. Such policies may assign legal operation/formation goals but must use this document's explicit membership, stance, per-member navigation/admission, derived Cohesion and existing Discipline permissions. It cannot convert a control group into a formation, grant cohesion through invisible/distant units, or issue simultaneous conflicting individual orders. The later AI-X03 proposal does not imply implemented coordination.

## 2. Comparative RTS mechanics

| Reference | Characteristic observed | POLITIKA lesson | Boundary |
| --- | --- | --- | --- |
| *Command & Conquer 3: Tiberium Wars* | Aggressive, Guard, Hold Ground and Hold Fire are rules-of-engagement stances | Separate autonomous engagement from the immediate Move/Attack order | No proprietary asset/engine reuse or automatic original hotkeys |
| *Age of Empires II* | Group shapes such as Line, Box, Staggered/Flank; unit-type arrangement | Define shape templates and assign military roles to slots | Avoid medieval melee requirements |
| *StarCraft II* 5.0 editor | Trigger-enabled square formation and spacing; movement speed is not synchronized | Configurable formation spacing, explicit speed-policy choice | Do not claim universal default-game formation movement |
| *Company of Heroes 3* | Cover, suppression and squad spacing meaningfully influence infantry performance | Geometry and member exposure should matter in combat | No large-scale per-soldier engine migration without profiling |
| *Total War* | Facing, force concentration, deployment and morale interact | Make organization degradable and recoverable | Do not force historical regiment or melee routing rules |
| *Wargame: Red Dragon* | Stress lowers performance and may lead to unit rout | Tie organizational reliability to per-unit Discipline and realistic stress | Do not replace POLITIKA's authorized special broken-threshold outcomes |
| *Homeworld 3* | Formation and stance can be inherited by reinforcements while hotkey control groups remain separate; regrouping improves forward progress | Preserve formation identity and avoid backward oscillation | Space/3D fleet-specific steering is inappropriate |

**Synthesis:** C&C-responsive orders, Age-of-Empires-like geometric choices, Company-of-Heroes-like contextual spacing, and Wargame-like stress can be combined **without** sacrificing an individually commanded, fixed-step RTS model. Source links in §13 substantiate the comparisons; proposed POLITIKA details are original design choices, not verified source-game implementation internals.

## 3. Definitions, instances and relationships

| Concept | State owner | Required distinctions |
| --- | --- | --- |
| Unit archetype | Validated UE content | Movement class, footprint, eligible role, weapon and capabilities |
| Unit instance | Match/coordinator | Runtime ID, owner, match generation, health, command, per-unit `UnitResolve` |
| Selection/hotkey group | Local controller | Sorted runtime IDs + generation; no persistent military authority |
| Formation definition | Stable-ID validated content | Shapes, allowed sizes/classes, relative slots, roles, spacing bounds, fallback policy |
| Formation instance | Match-owned state | Formation runtime ID, owning participant, generation, member IDs, anchor, facing, shape and active intent |
| Slot assignment | Formation state | Slot index, member ID, role and legal desired position |
| Stance | Unit override or formation default | Automatic acquisition/pursuit/retaliation, independent of active order |
| Adherence | Formation policy | Maintain / Flexible / Free movement organization |
| Cohesion | Derived from authoritative state | Slot proximity, connectedness and operational participation; not inventory |
| Command relationship | Commander talent subsystem | Authorized appointment → eligible subordinate with valid coverage |

One unit can occur in several *local* control groups; initially it may belong to at most **one active peer-level formation**. Hierarchical subformations are deferred; later nesting needs acyclic membership and no double-counted supporter bonuses. Formation IDs and runtime entity IDs must be different namespaces; a definition ID never equals a spawned instance.

### 3.1. Interoperability with numbered and dynamic control groups (core 4C-G)

[Selection and Group Orders](RTS_SELECTION_AND_GROUP_ORDERS.md) defines proposed **ten-slot** local control groups and set operations: Ctrl+number Replace, Shift+number Union, Alt+number ExclusiveTransfer, number Recall, double-tap Focus and Tab subgroup cycling. [Comparative Input Research](RTS_CONTROL_GROUP_INPUT_RESEARCH.md) grounds those player-facing conventions. **These operations do not implicitly Form, Reform, move, change stance, assign slots, or change a unit's canonical `UnitResolve`.** They are an addressing layer that can overlap arbitrarily; a unit normally has only one active *peer-level* match-owned formation assignment.

When a selection contains parts of multiple formations, the UI must distinguish: (a) `Selected IDs`, (b) `Control-group slot memberships`, (c) `formation IDs and their current member/slot counts`, and (d) `individually order-eligible members`. A formation control should never automatically extend to unselected members unless its command explicitly targets the entire validated formation and previews that scope. An individual order can override one member while the rest retain formation identity. Returning to group 1 does not cancel group 2's earlier instructions or force a formation regroup.

**Ordering boundary:** once explicit formation operations exist, changes to formation membership and persistent stance require their own admitted authoritative transition; later formation destinations resolve to legal per-member requests through the existing coordinator. In contrast, control-group assignment/retrieval and type-filtering remain local. A player can issue Move to a selected set drawn from several formations, but this is an ordinary non-atomic batch, **not** an implicit mass formation. Members may end up outside formation geometry, degrading *derived* Cohesion according to real state.

**Discipline exploit test:** assign a distant rifle unit to all ten numbered slots and repeatedly add it to other groups. Its physical participation counts **once**, and neither its Discipline nor the Cohesion of any formation improves from these assignments. In particular, the Directorate/Ascendancy massive-cohesive-army exception is computed from qualified actual supporting entities, never from shortcut group size. Invalid/mismatched-generation or ownership-lost members cannot be recalled into a formation through stale IDs.

**Staging:** 4C baseline multi-selection → **4C-G** core user-facing control groups (separately qualified, not retroactive 4F requirement) → FORM-F2+ formation mechanics. A formation prototype may use explicit manually selected IDs without presupposing arbitrary auto-grouping; however, player-facing large-force usability should not be considered production-ready without 4C-G.

### 3.2. Proposed formation lifecycle

`Requested → Validated/Admitted → Assembling → Moving / Deployed → Degraded / Fragmented → Regrouping → Moving / Deployed → Dissolved`

- **Admission:** validate match Active, bound participant, owner, generation, content IDs, viable member capabilities and placement/facing constraints.
- **Assembly:** derive slots, find a stable assignment and admit movement as legal per-member orders. A preview is not authoritative.
- **Moving/Deployed:** individual navigation and combat remain the execution source of truth.
- **Degraded:** vacancies/blocked members or temporary deviations exist; show limited organization.
- **Fragmented:** spatially disconnected clusters cannot claim one coherent mutual-support army. Ordinary individual unit action continues.
- **Regrouping:** recompute only on meaningful changes with bounded interval/hysteresis; no repeated infinite slot swapping.
- **Dissolved:** retire membership and derived effects; preserve or stop members' otherwise valid orders according to a documented explicit disband policy.

Death, capture/ownership change, member detachment, incompatible reinforcement, expired command target and match Terminal/Return must produce safe membership transitions and explicit feedback. Stale next-match state cannot be recalled via weak pointers or control-group IDs.

## 4. Formation shapes and roles

Start with **Line, Column, Dispersed and Perimeter**, then gate **Wedge, Echelon Left/Right, Layered Screen and Convoy**. Shapes must respect late-Cold-War infantry and vehicle operations rather than literally adopting medieval formations.

| Formation | Operational purpose | Genuine spatial tradeoff |
| --- | --- | --- |
| Line | Defensive frontage and direct-fire coverage | Exposed flanks and constrained-width failure |
| Column | Road travel, bridge crossing, organized movement | Depth and exposure to area attacks/obstruction |
| Dispersed | Reduce concentration against explosives/artillery | May lose close mutual support and immediate concentrated fire |
| Perimeter | Defend fixed objective, leader, artillery, supply convoy stop | Fire divided across directions |
| Wedge | Directed mechanized breakthrough | Vulnerability of leading element |
| Echelon | Protect/advance one favored flank | Weaker opposite flank |
| Layered Screen | Light scouts ahead of protected heavy elements | Must preserve role and perception boundaries |
| Convoy | Vehicles/escorts through corridors | Mixed movement profiles, speeds and footprints |

A definition should validate supported movement layers, min/max size, actual footprint/radius, orientation, slot uniqueness, per-role preference and spacing. Mixed-size vehicles cannot be placed into an infantry-only lattice. Civilian protection/escort should not grant civilians combat formation support.

**Geometry:** for anchor `A`, facing `theta` and relative slot `s_i`, preferred point `p_i* = A + R(theta) s_i`. This is only a **candidate**. Project each candidate onto the member's legal movement layer/mesh, check world bounds, collision, slope and existing route limits. Choose a deterministic stable slot assignment with preference for retaining previous roles, and report partial failure instead of moving someone through walls.

## 5. Tactical stance and formation adherence

These dimensions must be independently adjustable:

| Stance | Autonomous engagement | Pursuit |
| --- | --- | --- |
| Aggressive | Engage eligible observed enemies | May leave slot within bounded leash; rejoin afterward |
| Guard | Engage threats to own/assigned defended area or unit | Limited chase then return |
| Hold Ground | Fire on legal targets within current weapon range | No autonomous pursuit |
| Hold Fire | No autonomous attacks or retaliation | Legal noncombat movement continues; direct Attack override rule must be explicit |
| Withdraw (future) | Prioritize controlled disengagement | Requires valid explicit retreat destination and failure/interrupt policy |

**Adherence:** `Maintain` favors ordered slots unless path/safety invalid; `Flexible` permits local cover and obstacle maneuver with bounded restoration; `Free` relaxes geometric enforcement while retaining membership and potential regroup. A dispersed formation may be Aggressive, and a perimeter may be Hold Fire. Both are meaningful combinations.

**Precedence:** hard game authority/capability/targeting restrictions > admitted explicit individual action or override > subformation/role directive > formation default > archetype default. Changing stance must not silently replace Move/Stop; a new explicit order cannot silently clear persistent stance. The existing executable `GuardOrder` and a future *Guard stance* require separate interfaces. Explicit Attack while Hold Fire, and the meaning of Withdraw for cornered units, remain decisions to lock before coding.

## 6. Discipline and Cohesion: one meter, two levels of organization

`UnitResolve` is an authoritative **per-individual numeric meter**. Combat display = Discipline; non-combat display = Morale. Its existing depletion, recovery and `BrokenThreshold` remain the domain of [Propaganda and Morale](PROPAGANDA_AND_MORALE.md). Faction Population Morale is distinct, already defined, and can influence soldiers' baseline resolve. **Cohesion is not an additional psychological meter.**

A candidate derived measure is `C_F = sum(w_i * q_i * o_i) / sum(w_i)` where `w_i` is a positive role weight, `q_i ∈ [0,1]` measures proximity to assigned legal slot, and `o_i ∈ [0,1]` indicates operational availability. This *example* needs connectivity and overlap validation. For support, use a bounded local proximity/connected-component graph among actual eligible friendly combat units. Two distant formations cannot collectively award a close-support bonus simply because both are selected. A geometrically accurate empty/disabled formation has no genuine support.

Possible readable states: **Organized, Degraded, Fragmented, Dissolved**. A disciplined soldier may be part of a fragmented formation; a steady formation may include a shaken individual. Do not conflate the categories.

A future candidate resolve integration is `D_i(next) = clamp(D_i + recovery + eligible_local_support + authorized_leadership - qualified_stress, 0, max)`; this is a **design sketch**, not a selected balance function. Inputs may include combat casualties, suppression, isolation, food-shortage consequences, faction Population Morale and lawful commander communication. Each modifier needs a source ID, caps, recovery/expiry rule, eligibility, deterministic application and no double-counting.

| Faction | **Existing binding rule** | Possible additional doctrine (not canon) |
| --- | --- | --- |
| Atlantic Concord | No Discipline penalty *solely* for solo or very small combat groups; no general immunity | Faster detachment/reorganization |
| Red Directorate | Ordinary coherent nearby group support **and** extra very-massive coherent bonus | Structured large combined formations |
| Iron Ascendancy | Ordinary coherent support **and** extra very-massive coherent bonus | Concentrated rigid organization |
| Meridian Pact | Ordinary coherent group support; **no** special high-mass bonus | Mobile mixed-role escort flexibility |
| Unaligned (NPC) | Ordinary coherent group support; **no** special high-mass bonus | Federal defensive/patrol formation behaviors |

The high-mass term must be capped, thresholded and derived from physically coherent effective combat strength rather than total faction inventory; exact thresholds/caps are not settled. Concord is still vulnerable to direct combat, casualties and eligible Psy-Ops; civilians get **zero military formation-size bonus**.

**Critical special effect distinction:** ordinary low Discipline, disrupted formation or a spatially isolated member may influence vulnerability only under independently coded rules; it must not bypass the existing Atlantic Concord Psy-Ops elimination or Iron Ascendancy Intimidation `BrokenThreshold` + eligibility + isolation + ownership-conversion contract. A lost formation does not instantly destroy/convert everyone inside it.

## 7. Command and navigation integration

Verified prototype foundation: [Command/Simulation Contract](RTS_COMMAND_SIMULATION_CONTRACT.md) defines a `CommandRequest` with match generation, sequence, participant, issuer and correlation; the coordinator validates at its 0.05 s step. Current [Navigation Acceptance](RTS_RUNTIME_NAVIGATION_ACCEPTANCE.md) describes per-unit synchronous Recast planning and authoritative waypoint following with an incomplete broader NAV matrix. Single-unit route evidence does **not** validate crowd movement, multi-class formations or massive-army performance. Existing [Group Orders](RTS_SELECTION_AND_GROUP_ORDERS.md) is a **proposed** multi-issuer specification, not a qualified formation system.

**Proposed execution:**

1. Controller or observation-limited AI requests Form/Reform/Move via a bound authorized participant. Snapshot eligible runtime member IDs sorted by stable ID and check generation/faction/current membership.
2. Validate shape definition, size, footprint, role constraints, anchor/facing and any requested adherence. Reject malformed/unauthorized membership before mutation.
3. Compute deterministic member-to-slot assignment and distinct candidate map-space destinations. Project/query each under its *own* movement class and current obstacles.
4. Use the **existing coordinator and independently validated member commands** (or a strictly validated envelope that fans out into them later). Each has its own sequence and correlation. Retain previous valid route on rejected member admission.
5. Let only existing fixed-step member movement advance paths, collisions and arrival. Distinguish accepted order from subsequent failed/unavailable route and from actual arrival.
6. On move/stop/attack replacement, death, capture or blocked slots, evaluate bounded regrouping with stable assignments and hysteresis. Do not replan every render frame, issue unlimited synchronous path queries, or reverse the whole army simply to make a leader rejoin.
7. On Terminal, Return or new generation, invalidate formation handles, members, modifier sources and pending derived commands exactly once.

**Constrained terrain:** a Line at a bridge may need a narrow Column while crossing then resume Line. Adaptive shape conversion is *later*, governed by adherence and explicit player-visible policy. If strict deployment is impossible, report it rather than silently alter chosen shape, overlap units or teleport through a blocker.

Unreal Engine 5.8 provides NavMesh and avoidance technologies (RVO and Detour Crowd). Epic documents them as alternative approaches with different underlying movement/controller requirements; neither is a proved drop-in for POLITIKA's Actor-based fixed-step route follower. Shared corridors, avoidance, MassEntity/MassNavigation and speed synchronization must be separately benchmarked, not implemented speculatively or by adding a second scheduler.

## 8. Tactical roles, commanders, research, supply and intelligence

- **Roles:** frontline, flank, rear guard, support weapon, scout/screen, escort, reserve and protected unit are eligibility/slot preferences, not new entity owner types. They must respect weapon firing arcs, line of fire, movement profile and actual member capabilities.
- **Elite commander talents:** only expressly designated Elite commanders may choose talents. Later talents may conditionally modify rally, regroup speed, communication coverage or eligible subordinate Discipline **through authorized appointment links**, never via a free all-army aura or talent tree for ordinary units.
- **Research:** formations/stances should be baseline RTS functionality, not an age/era tech lock. Existing building-hosted research may improve training/coordination through participant-wide or scoped modifiers with proper caps and faction permission checks.
- **Economy:** do not add formation-specific money, food or electricity ledgers. Existing USD and dollar-purchased military food may influence ordinary recovery if a future tested supply-resolve rule permits it. Power remains a building service constraint.
- **Reconnaissance:** screens may extend early observation only via real authorized sighting; formation UI must never reveal hidden enemies or the ground truth of deliberately forged reports.
- **AI and Unaligned:** opponent AI may eventually form, orient, split and regroup forces under the same participant authority, observation filters and bounded cadence as human commands. The Unaligned remains NPC-only and requires its own valid system authority.
- **Persistence:** save/replay and multiplayer transport are separate features; formation IDs, definition revisions, membership/roles, stance/adherence, generation and relevant order correlations must be explicit if later serialized. Local fixed-step repeatability is not proof of deterministic multiplayer.

## 9. Input, HUD and feedback

A selected group should offer explicit **Form, Reinforce, Detach, Reform, Regroup, Disband** once each operation is implemented. Formation controls expose shape, facing, combat stance and geometric adherence as distinct axes. Saved control hotkeys only select/recall IDs; they should never create formations implicitly.

World previews show intended member slots and orientation, but must be labeled advisory until match-authoritative projection and route admission. Inspection should show member count, owner, formation state, visible failures/partial acceptance, role and individual Discipline; display Cohesion as a **derived group condition**, not a second morale gauge. UI/AI are restricted to participant-accessible observation, including when facing a fake report.

The present A/S key collision (camera motion versus combat orders) in [UI Settings and Keymap](RTS_UI_SETTINGS_AND_KEYMAP.md) must be corrected before assigning formation hotkeys. Desktop drag/facing, controller/Steam Deck and Android touch require different interaction gestures mapped to the same semantic orders. Test panel legibility at 1280×720, 1920×1080 and 3840×2160, non-color-only warnings and real packaged keyboard/cancel behavior.

## 10. Dependency-ordered implementation roadmap

All items below are **proposed work**, not verification that any formation feature exists. Normal Stage 4F first playable skirmish may be accepted without full formation behavior; later formation development must not silently redefine its acceptance.

| Gate | Bounded increment | Hard dependency |
| --- | --- | --- |
| FORM-F0 | Finish relevant NAV lifecycle/failure qualifications; benchmark multiunit query/collision performance | Existing navigation authority |
| FORM-F1 | 4C owned-unit multi-selection with individual per-member Move/Stop, explicit mixed outcomes and generation cleanup | Group commands |
| FORM-F2 | Eight-unit Line/Column geometry, facing, unique legal slots and bounded deterministic slot assignment | F1 and NAV baseline |
| FORM-F3 | Formation ID/membership/lifecycle, casualties/detach, independent Stop and no route corruption, bounded regroup | F2 |
| FORM-F4 | Combat stances and independent adherence, explicit per-member override semantics | F3 |
| FORM-F5 | Derived Cohesion and capped canonical `UnitResolve` faction effects | F3–F4 plus actual resolve implementation |
| FORM-F6 | Dispersed/Perimeter then Wedge/Echelon/Screen/Convoy, mixed profiles, chokepoints and crowd tuning | F5 and scale evidence |
| FORM-F7 | Commander talents, legitimate research/food interactions, observation-limited AI doctrine | Respective subsystem gates |
| FORM-F8 | Broader performance, save/replay and platform/device qualification | Shipped individual slices |

First acceptable game fixture: **eight eligible infantry** with Line and Column, one legal and one blocked group movement, one member loss during transit, individual Stop, mixed command outcomes and one full match-return/rematch cleanup. Initial implementation should not introduce a new universal squad Actor hierarchy, second simulation loop, free formation bonus, automatic withdrawal or enemy-only omniscient effects.

## 11. Acceptance matrix (not yet executed)

| ID | Test / invariant |
| --- | --- |
| FORM-01 | Hostile, neutral, dead, invalid, wrong-controller or NPC Unaligned units cannot be locally formed/commanded |
| FORM-02 | Multi-select and hotkey control group cannot alone create membership, Cohesion or a Discipline bonus |
| FORM-03 | Shape definition, generation, owner, ID references and slot validity fail closed on bad content |
| FORM-04 | Deterministic unique member-slot assignment, finite coordinates, legal footprints and facing |
| FORM-05 | Each member target projected/route-validated under movement class; no partial-path silent success/teleport |
| FORM-06 | Mixed accepted/rejected group requests preserve legitimate prior member routes and publish individual outcomes |
| FORM-07 | One member's Stop, replacement or detach cannot cancel another member's accepted route |
| FORM-08 | Bounded regroup with no endless oscillation, impossible backtracking or per-frame path-query storm |
| FORM-09 | Death, capture, role change, reinforcement, disband and topology changes revalidate members/slots |
| FORM-10 | Shape, stance, adherence and individual explicit order remain independent and inspectable |
| FORM-11 | Fragmented/distant/unavailable members cannot claim a connected coherent-army bonus |
| FORM-12 | Normal four-faction support, Concord solo exception and Directorate/Ascendancy-only high-mass bonus hold |
| FORM-13 | Civilian Morale never gets combat support; individual `UnitResolve` remains singular |
| FORM-14 | No unearned Psy-Ops elimination/Intimidation ownership change from formation disruption alone |
| FORM-15 | Active->Terminal/Return/rematch retires stale formation references, modifiers and pending member intents |
| FORM-16 | Identical authoritative-step assignments/outcomes/events across tested rendered step schedules, not cross-machine proof |
| FORM-17 | AI faces same authority/visibility limitations, bounded response after partial failure |
| FORM-18 | Rendered cooked Linux Development package proves multi-member formation placement and readable outcomes |
| FORM-19 | Profile 8/24/64/128 members before 256+: path queries, CPU/frame, memory, overlap, slot error, regroup latency |
| FORM-20 | Independent device-profile, viewport and accessibility qualification; no implicit cross-platform acceptance |

**Diagnostics:** exact source/UE build pin, match generation, simulation step, participant ID, formation ID, authoritative membership change, shape/stance/adherence, slot IDs, destination projections, route correlation/outcome, coherence transitions, typed resolve-modifier sources and rejection messages. Dev diagnostics must remain bounded and not disclose hidden truth to another participant. Each PASS needs a named, reproducible source and cooked runtime evidence; BLOCKED/INCONCLUSIVE must never be promoted to PASS.

## 12. Open decisions and exclusions

Open: min/max formation sizes; movement-class and mixed-speed policy; footprint/spacing by unit role; stable matching cost; anchors and rotation controls; cohesion threshold, connectedness and hysteresis; normal/high-mass support magnitude, radius and saturation; ordinary resolve break policy; withdrawal destination/order kind; Hold Fire/explicit Attack precedence; auto Column conversions; avoidance technology; save/net schema.

Excluded initially: regiment-scale melee doctrines, invincible dense blobs, infinite size stacking, discipline gained from hotkeys, additional per-unit Morale gauge, formation food currency, speculative MassEntity migration, per-formation Tick movement, teleport/sweep bypass, all-or-nothing bulk admission, unearned Unaligned control, automatic unit ownership transfer upon ordinary stress, and claims of cross-platform determinism or shipping readiness.

## 13. Reference sources and provenance

1. EA, *Command & Conquer 3: Tiberium Wars* instruction manual mirror (stances): https://device.report/m/1341ac174031607d32c41d684c70d02e80f48d71b07d339c22c3150baf4ae2c7
2. *Age of Empires* Series Wiki, *Unit Formation*, descriptive secondary reference: https://ageofempires.fandom.com/wiki/Unit_formation
3. Blizzard, *StarCraft II 5.0 Patch Notes* (formation editor support and speed caveat): https://news.blizzard.com/en-us/article/23482838/starcraft-ii-5-0-patch-notes
4. Relic Entertainment, *Company of Heroes 3 Patch Notes Archive* (squad spacing, navigation and suppression): https://help.relic.com/hc/en-us/articles/39307744455571-Company-of-Heroes-3-Patch-Notes-Archive
5. Eugen Systems, *Wargame: Red Dragon — Morale* (official combat stress): https://www.wargame-rd.com/en/rd-gameplay-morale.html
6. *Homeworld 3* developer formation/regroup update: https://steamcommunity.com/games/1840080/announcements/detail/4145073329281827772 and later formation/stance reinforcement update: https://steamcommunity.com/app/1840080/announcements/
7. Epic, *Navigation System in Unreal Engine 5.8*: https://dev.epicgames.com/documentation/unreal-engine/navigation-system-in-unreal-engine?application_version=5.8
8. Epic, *Using Avoidance With the Navigation System*: https://dev.epicgames.com/documentation/unreal-engine/using-avoidance-with-the-navigation-system-in-unreal-engine?application_version=5.8

Game comparisons are evidence for *design inspiration*, not for Unreal internal implementation compatibility. All POLITIKA-specific formation/resolve proposals are grounded in the specialist owners cited at the top and subject to their later acceptance gates.
