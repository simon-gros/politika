# POLITIKA — RTS HUD Ontology, Comparative Design and Implementation Architecture

**Status: CORE HUD DESIGN / PROPOSED — 29 October 2026. Not implemented, tested, or accepted.**
**Engine baseline:** Unreal Engine 5.8.3, with the currently verified scoped Linux tactical prototype preserved.
**Scope:** In-match HUD presentation, interaction composition, information integrity, cross-platform layouts, performance and bounded acceptance. The existing Canvas HUD and Slate shell are the implementation baseline; this document does not assert that UMG, CommonUI, MVVM, minimap, production or formation widgets exist.

## 1. Decision and authority

**Decision:** Treat the HUD as a core RTS *observation-and-command interface*: an observer-authorized projection of simulation state with discoverable ways to express legal command intent. Choose a **battlefield-first hybrid HUD**: a compact always-visible strategic rail; a contextual selected-force inspector and command card; a tactical minimap; an optional global production drawer; and on-demand intelligence, commander, formation and logistics views. A player must be able to determine **what is known, what may be commanded, and what actually happened**.

This document owns HUD-specific **semantic presentation, region hierarchy, widget contracts, integration seams, and acceptance**. It does **not** supersede:
- [UI Overview](POLITIKA_UI_OVERVIEW.md) — product-wide interface, faction presentation, accessibility, menus and narrative.
- [Settings and Keymap](RTS_UI_SETTINGS_AND_KEYMAP.md) — video/audio preferences, semantic input actions, current key collisions and remapping.
- [Selection and Group Orders](RTS_SELECTION_AND_GROUP_ORDERS.md) — selection set algebra, control groups, per-unit order and admission.
- [Military Formations and Discipline](RTS_UNIT_FORMATIONS_AND_DISCIPLINE.md) — match-owned formations, shapes, stance, adherence, Cohesion and resolve.
- [Visibility and Intelligence](RTS_VISIBILITY_AND_INTELLIGENCE.md) — observer-specific authorization, contact freshness, uncertainty and claims.
- [USD Economy and Power](RTS_OPERATIONAL_ECONOMY_AND_POWER.md), [Production and Technology](RTS_PRODUCTION_AND_TECHNOLOGY.md), [Propaganda and Morale](PROPAGANDA_AND_MORALE.md) — authoritative economy, production, research and resolve.
- [Engine Outline](ENGINE_OUTLINE.md) — dependency order, acceptance promotion and existing stage meanings; [Systems Completeness Matrix](RTS_SYSTEMS_COMPLETENESS_MATRIX.md) — source-audited evidence boundary.

**Non-goals:** no second command-admission path; no presentation-owned game rules or ledger; no new fixed-step scheduler; no speculative implementation of fog, talents, economy or formation mechanics; no automatic conversion of UI sketches into working functionality; no copied third-party game art or UI assets.

## 2. Comparative RTS evidence and justified transfers

| Reference game / primary source | Observed design principle | POLITIKA adaptation | Boundary |
| --- | --- | --- | --- |
| *Command & Conquer 3: Tiberium Wars* [PC manual](https://static.wikia.nocookie.net/cnc_gamepedia_en/images/8/8b/CCUC-CNC3-pcMANOLeUS.pdf/revision/latest) | Sidebar grants production from anywhere on the map, contextual selected-object information, radar and objective access | Optional persistent/collapsible build and reinforcement access, with producer inspection and genuine disabled reasons | Do not copy branded sidebar artwork or imply that POLITIKA already has production |
| *StarCraft II* [Special Control](https://news.blizzard.com/en-us/article/4552955/game-guide-special-control), [Simplified Controls](https://news.blizzard.com/en-us/article/6640645/game-guide-simplified-controls) | Selection, control groups, subgroup cycling, hotkey-labeled contextual orders, notification navigation | Contextual force inspector; distinct 0–9 group buttons; dynamic actual-key prompts and focused event history | POLITIKA's owning group algebra remains normative; do not import another title's conflicting hotkey behavior |
| *Age of Empires IV* [principal UX designer's case study](https://kevinhustler.me/aoe4) | Reduced permanent occlusion, resource proximity to purchase commands, adaptive text scaling and information hierarchy | Spendable USD near purchases; expand logistics on demand; selectable density and text scale | POLITIKA deliberately avoids a resource-farming dashboard |
| *Tempest Rising* [10 March 2026 update](https://steamdb.info/patchnotes/22114885/) | Grouped/ungrouped selection composition, power output in production tooltips, sortable/pinnable global queue | Optional group aggregation, producer-specific power status, sortable queue only after genuine queues exist | A patch note is feature evidence, not a verified copyable engine implementation |
| *Company of Heroes* tactical-HUD pattern | Squad condition and positional/tactical state are prominent to decision-making | Readiness, Cohesion, Discipline, stance and operational location cues | Do not conflate suppression with the existing UnitResolve or add undocumented states |
| *Supreme Commander* strategic presentation pattern | Spatial aggregation at wider viewing scales | Later candidate: tactical-map/strategic-zoom representation after spatial and visibility validation | Not a first-playable blocker or an alternate sensor |
| *Zero-K* configurable widgets / large-unit-scale precedent | Customizable HUD density can support many simultaneous entities | Later candidate: compact/detailed presets, bounded aggregation and repositionable drawers | Avoid early freeform editor complexity |

The table is comparative design research, **not** proof that POLITIKA features exist or that another title's implementation is available for reuse. Transferred functionality must be implemented with original assets, tested against the current engine pin, and justified by POLITIKA's own gameplay semantics.

## 3. Minimal HUD ontology

The HUD is a *view and interaction layer*, not an additional world entity. For participant **p** at simulation step **t**, let **S(t)** be authoritative world state; **O(p,t)** is the participant-authorized observation; **L(p,t)** is local selection and UI preference; **H(p,t)** is the visible interface:

> H(p,t) = Present(O(p,t), L(p,t), admissible-capability metadata, authorized events).

**No HUD reader may substitute unrestricted S(t) for O(p,t).** An input gesture yields a semantic intent; the existing coordinator validates participant, generation, issuer, capability, target/observation policy and phase before state changes. A button is never a source of authority.

| Kind | Definition and typical widget |
| --- | --- |
| Entity | Stable runtime unit/building identity; selected entity card or map contact |
| Attribute | Health, ammo, Discipline, dollar balance; gauge/text derived from authorized state |
| Relation | Owner/controller, formation membership, target, producer; roster or breadcrumb |
| Activity/state | Moving, reloading, producing, unpowered, disrupted; status chip |
| Capability | Move, Guard, Research, Purchase Food; command button only when authored/valid |
| Constraint | Insufficient USD, blocked spawn, no electricity, missing tech; precise disabled reason |
| Intent | Player requests an order/action; selection, targeting and confirmation state |
| Outcome | Local input failure, request admitted/rejected, executing, interrupted, completed; feedback |
| Observation | Currently visible or last-known contact; minimap marker with freshness |
| Claim | Unverified, disputed or disproved intelligence; source-linked report/marker |
| Event | Attack warning, queue completion or objective transition; actionable log entry |
| Objective | Validated game goal; concise persistent mission card |
| Presentation | Region, overlay, icon, tooltip, audio or world marker; never gameplay authority |

**Distinct objects:** local selection set != numbered control group != match-owned formation != unit subgroup != gameplay order. Their cards may appear adjacent but must not share IDs, lifetimes or implicit mutations. Combatant Discipline and civilian Morale are two labels for the same per-unit resolve; *formation Cohesion* and *population Morale* are distinct aggregate concepts. Authorized owner != legally commanded controller; hostile inspection is read-only.

## 4. Information architecture and layout

### 4.1 Desktop functional zones

Use **one root HUD host** and independent modules (conceptual; layout dimensions remain prototypes):

~~~text
┌───────────────────────────────────────────────────────────────────────┐
│ Mission / important alerts     $ Treasury + income   Power   Food    │
├───────────────────────────────────────────────────────────────────────┤
│                                                                       │
│                     BATTLEFIELD / ORDERS / CAMERA                     │
│                                                  Optional production  │
│                                                  or intel drawer      │
│                                                                       │
├─────────────────┬──────────────────────────┬──────────────────────────┤
│ Tactical minimap│ Selected entity / force  │ Orders and ability card  │
│ + overlay legend│ + 0–9 control groups     │ + formation context      │
└─────────────────┴──────────────────────────┴──────────────────────────┘
~~~

1. **Persistent rail:** mission/objective urgency; spendable USD and income; generated electricity versus allocated required load; current military food reserve/runway; high-priority and recent alerts. Hide irrelevant secondary currencies by default.
2. **Battlefield:** world hit targets, selection/rally/placement/formation previews and confirmed order feedback. Decorative transparent HUD content must not intercept pointer/world hits.
3. **Selection inspector:** unit identity, legal control owner, selected count, health/readiness, active order, aggregated subgroup composition, real formation state; eligible 0–9 quick group recall where space permits.
4. **Contextual command card:** actions valid for full selection and explicitly labeled subset-valid actions; actual remapped shortcuts, precise costs, prerequisites, cooldowns and cancel/targeting state.
5. **Tactical minimap:** viewer-scoped terrain/contact/objective projection and click-to-focus/order input only after its own authorization and interaction gate.
6. **Conditional drawers:** production/building/research, trade/logistics, intelligence/map, commander talents, institutional/diplomatic status and full notification history; open on demand and preserve battlefield visibility.

**Interaction principle:** selection -> inspect availability -> express semantic intent -> targeting/confirm if required -> coordinator admission -> execution/outcome feedback. The HUD cannot claim a route completed merely because its move request was admitted.

### 4.2 Density, responsiveness and priority

Provide at least **Standard** and **Compact** composition presets as future options. The visible battlefield stays dominant; alerts, selected force and valid commands outrank decorative panels. At narrow viewport widths, collapse production/intelligence first, then condense the minimap and multi-selection roster while preserving a reachable command surface. Content and layout must adapt across 1280x720, 1920x1080, 2560x1440, 3840x2160, ultrawide, Steam Deck-class displays and representative Android landscape safe areas; no desktop 4K assumptions for handheld/mobile. Design test targets must be checked on rendered packages/devices, not derived from screenshots alone.

Use 1980s Yugoslav cartography, field dossiers, typography and institutional markings as stylistic vocabulary **only**. Never let distressed textures impede text contrast; use icon shape plus text for faction, ownership, error, uncertainty and urgency, not hue alone. Preserve original faction insignia licensing/provenance policy.

## 5. Widget presentation contracts

### 5.1 Selected force and control groups

- **Single friendly commandable entity:** ID/label, archetype, verified owner/controller, health, permitted resolve label, active command state, capability availability, abilities and source-authored status. Do not show capabilities the archetype does not possess.
- **Homogeneous multi-select:** total eligible count, meaningful aggregate health/status and individual inspection; mixed outcomes never collapsed to a fictional global success.
- **Heterogeneous multi-select:** subgroup archetype counts with expansion/cycling; shared commands versus subset-valid abilities (e.g., 2 of 7 eligible), explicit target mode, per-issuer rejection summary.
- **Enemy/Unaligned inspection:** never transforms into owned selection or grants commands; render only actually observable details. NPC Unaligned always retains its separate affiliation.
- **Ten hotkey-group slots (0–9):** active/empty state, actual revalidated member count, key hints, optional focus; **recall does not form a military formation**. Obey the existing Ctrl/Shift/Alt group algebra, double-tap focus and match-generation retirement from the selection owner. Bar remains optional in compact layout.

### 5.2 Formation, stance, adherence and resolve

Once the independent FORM gates land, expose **Form, Reinforce, Detach, Reform, Regroup, Disband** according to real capability. Separate axes: **shape and facing**, **combat stance** (Aggressive / Guard / Hold Ground / Hold Fire as currently proposed), and **adherence** (Maintain / Flexible / Free). A formation exists only after admitted formation creation; a preview is advisory, slots require legal routing, and invalid members retain independent rejection/status. Show match-owned formation ID, count, facing, optional member roles, derived **Cohesion**, and each applicable member's **Discipline** (not a duplicate Morale). Civilian entities show Morale under the single resolve system. Large-army/leadership bonuses are presented only when actually authorized by the formation/resolve rules, never inferred from how many portraits are selected.

### 5.3 Economy, production and research

| HUD surface | Persistent minimum | Detail on demand |
| --- | --- | --- |
| USD treasury | Available dollar balance (integer cents formatted as USD) and meaningful income rate | Reserved funds, source, costs/commitments; purchases driven by authoritative transactions |
| Power | Generated capacity / required building load, unpowered warning | Affected structures, supply and priorities; zero power != zero dollars |
| Food | Reserve, due military upkeep and estimated runway with unit/time basis | Valid dollar-funded food purchase, stock management and optional tested restock |
| Saleable commodities | Context-specific shortage/status only | Gold, crude oil, natural gas, petroleum/kerosene separately; eligible quote, fees, reservation and atomic transaction result |
| Nuclear stocks | Faction/objective-permitted status only | Authorized strategic inventory/refinement; **never** generic sell-to-USD card |
| Buildings/queues | Compact production status when relevant | Producer, powered/paused state, recipe USD cost, prerequisites, ETA, spawn failure and authored cancellation refund |
| Research | Active upgrades and relevance | Building-hosted research dependency/upgrade changes; **no ages or eras** |

The global production queue should link back to individual actual producers, not maintain an independent queue or charge balances through UI. Distinguish Queue Requested / Admitted / Producing / Paused / Completed / Rejected / Cancelled / SpawnBlocked based on the owning simulation's real events; reconcile all funds and reserved commodities to authoritative data. A missing subsystem must appear as unimplemented or disabled in Development builds, not as interactive mock data in shipping gameplay.

### 5.4 Commander talents, intelligence and political systems

- **Elite commander only:** show the experience/talent indicator and contextual talent inspector only for a commander with a valid authored talent system. A selected ordinary rifleman never receives empty talent-tree filler or talent authority. Talent unlocks/modifiers apply to actual rules, not HUD-side combat buffs.
- **Intelligence:** separately mark current observation, stale last-known contact, allegation/unverified claim, disputed claim and disproved evidence. Include time/source/provenance where authorized and useful. No hidden enemy actor pointer or fabricated-report truth may appear through tooltip, icon, target-cursor denial or debug label.
- **Covert incidents:** costs, valid targets and attacker-known outcomes may be shown. Defender notices cover only observed/discovered losses, attribution and remediation. Do not announce a secret attack on the defender's HUD before actual detection.
- **Political/administrative:** population Morale and faction-authorized propaganda options; Unaligned is NPC-only. Verified ownership/authority is not inferred from faction color or a false-flag allegation.
- **Nuclear race:** permitted strategic progress for Concord, Directorate and Ascendancy per their existing faction rules; Iron Ascendancy's first own-built bomb is a victory trigger, not a launch command. Never expose hidden research to non-observers.

### 5.5 Tactical map, objectives and alerts

The minimap and optional enlarged operational map are **projections of an authorized observation service**, not additional sensors. Render currently observed friendly/hostile/neutral entities, last-known markers, objective locations and uncertain report footprints distinctly. Do not reveal a hidden death/movement, espionage actor, location or identity from global world scans. If the visibility service is absent, show only source-evidenced allowed information in the current prototype; do not claim complete fog-of-war support.

Prioritize notifications by *decision urgency*, not spectacle: attack/outage/mission status -> operational disruption -> routine queue completion. Distinguish **claim received** from **fact verified** and **order accepted** from **order finished**. Event history retains authorized messages for later review, with optional focus/inspect actions that revalidate referenced entities and observation rights. Minimap and alerts must never permit global debug omniscience in Shipping.

## 6. Input, focus and feedback state machine

- Use stable semantic input intents shared across pointer/hotkey, controller/Steam Input and touch; source key hints from active remapping. [Keymap](RTS_UI_SETTINGS_AND_KEYMAP.md) owns defaults and conflict resolution.
- **Known prototype defect:** A and S currently collide between camera axes and Attack-Move/Stop; F currently triggers infantry Sprint. Resolve before considering a HUD or formation keybinding accepted.
- State route: **Idle -> Selection / Context Inspect -> optional Targeting Preview -> Submitted -> Admitted or Rejected -> Executing -> Completed / Interrupted / Navigation Failed**. Input cancellation can occur before submission; stale results require generation and correlation validation.
- For a mixed selection, report per-unit accepted/rejected/failed status and do not erase successful active routes when a different member fails.
- Modal menus/rebinding own focus and suppress gameplay hotkeys/world clicks; targeting and UI pointer capture are mutually exclusive with accidental world selection or issuing a second order. Esc/back and device focus loss clear appropriate targeting state, not authoritative ongoing orders.
- Disabled buttons explain: no eligible issuers, insufficient USD, missing prerequisite, out of range, unpowered, wrong target category, invalid formation/member, unsupported technology, or permissions, **without leaking hidden truth**.

## 7. UE 5.8.3 technical seam

~~~text
Authoritative match coordinator (fixed-step rules, permissions, lifecycles)
    -> participant observation and accepted event projection
    -> local selection/group state + optional HUD presenter / narrow view models
    -> UMG/Slate HUD regions and world-space overlays
Input device -> mapping/context -> semantic intent -> coordinator admission
Coordinator outcomes -> participant event projection -> matching HUD feedback
~~~

**Existing integration first:** retain the present Canvas prototype HUD and existing shell behavior as regression baseline. Introduce a *bounded* read-only UMG/Slate widget for the selected unit and order result; prove parity in the same packaged fixture before replacing Canvas rows. No concurrent second UI-driven command subsystem.

The presenter consumes value snapshots/events with participant ID, match generation, observation step, stable runtime entity or contact ID and optional command correlation. Subscribe/unsubscribe on match start, return, death, terminal transition and rematch; never retain authority through stale UObject pointers or old event subscriptions. UI-only preferences (density, expansion, pinned drawers) belong to local user state. Domain state remains authoritative elsewhere.

Use [Epic UMG optimization guidance](https://dev.epicgames.com/documentation/unreal-engine/optimization-guidelines-for-umg-in-unreal-engine): prefer change-driven delegates over per-frame bound getters; avoid large nested Canvas layouts, excessive hidden widget construction, and per-frame world actor scans; use invalidation or lazy widget construction where measured. Profile with Unreal Insights / Slate Insights before selecting numeric budgets. [Epic UMG Viewmodel](https://dev.epicgames.com/documentation/unreal-engine/umg-viewmodel-for-unreal-engine) is **Beta** in UE 5.8: optional adapter only after pinned-engine validation, not a mandatory dependency. [Epic CommonUI design guidance](https://dev.epicgames.com/documentation/unreal-engine/design-guidelines-for-using-commonui-in-unreal-engine) explicitly distinguishes complex layered/cross-platform navigation from ordinary PC RTS HUD needs; consider it selectively for controller-accessible menus, not the default for all battlefield widgets.

## 8. Dependency-ordered HUD roadmap (new work packages, not established milestone names)

| Gate | Smallest deliverable | Dependency / no-claim boundary |
| --- | --- | --- |
| HUD-01 | Read-only selected-unit UMG/Slate inspector with health/order/objective/event parity | Existing prototype; no new command authority |
| HUD-02 | Clickable order card uses the same typed intents and admission as hotkeys; target cancel and denied reason | HUD-01; current A/S collision removed before binding acceptance |
| HUD-03 | Multi-select and heterogeneous subgroup summaries, per-issuer outcomes | 4C selection/group order implementation, not UI-only fake members |
| HUD-04 | 0–9 persistent local group display, group counts, dynamic hints and lifecycle cleanup | Independently accepted 4C-G contract |
| HUD-05 | Dollar/power/food status, producer queue and actual transactions | Authoritative 4B economy/production fixture, never UI-funded money |
| HUD-06 | Authorized minimap and event history; observation freshness | Participant-scoped observation contract or explicitly limited no-fog fixture |
| HUD-07 | Formation shape/facing/stance/adherence, Cohesion/Discipline inspection | Corresponding FORM gates; cannot precede real formation state |
| HUD-08 | Building research/production, faction-specific commander talent UI | Validated research/talent rules when implemented |
| HUD-09 | Intelligence/diplomacy/propaganda/Unaligned layers | Verified knowledge, provenance, faction permissions and scenario rules |
| HUD-10 | Compact, handheld/controller, Android touch and accessibility profile | Existing semantic intent, safe-area/focus and target platform evidence |
| HUD-11 | Larger force aggregation, performance/accessibility and cross-device qualification | Representative battle test scenes and measured thresholds |

**Staging rule:** HUD-01/02 may accompany 4C work, but do not retroactively change First Playable Skirmish 4F or 4C-G acceptance. Later formation, research, commander and intelligence layers are independent feature gates. The [Engine Outline](ENGINE_OUTLINE.md) owns promotion of each actual stage, with exact Git revision, pinned engine, build/package identity, fixtures, logs and visual input proof.

## 9. Bounded verification matrix

| ID | Positive and negative acceptance |
| --- | --- |
| HUD-01 | Read-only new view matches the current prototype's health/order/objective on a recorded local fixture; no simulation change when opening/closing |
| HUD-02 | HUD click and remapped hotkey submit the same participant, issuer, generation and typed intent, yielding the same authorized outcome |
| HUD-03 | Press/hold camera A/S replacement bindings while unit selected: no unintended order; mouse click on HUD never also issues world order |
| HUD-04 | Invalid/dead/captured/stale group member disappears from selected roster; rematch cannot show previous generation or event |
| HUD-05 | Mixed selection reveals full/partial eligibility; rejected member never overwrites valid member route; acceptance != movement completion |
| HUD-06 | Formation preview does not create FORM membership or resolve bonuses; invalid slot admission produces an explainable partial result |
| HUD-07 | Economy balances, reserved dollars, power shortages and food due match authorized ledger exactly; widgets cannot mint/refund resources |
| HUD-08 | Hidden enemy, undiscovered covert attack and fabricated intelligence truth are absent from HUD/minimap/tooltips; denied targeting does not act as sensor |
| HUD-09 | Event entry/camera focus revalidates observation rights and stable entity identity before navigating or inspecting |
| HUD-10 | Target mode/cancel/modal focus prevent click-through and persistent latched inputs; rebound labels are accurate for active device |
| HUD-11 | Standard/Compact layouts have no clipped interactive targets at 720p/1080p/4K, ultrawide and separately tested Steam Deck/Android when in scope |
| HUD-12 | Representative selection/alerts/queue stress produces measured UMG/Slate and frame data with an explicit performance gate; no invented universal budget |
| HUD-13 | Local packaged Linux Development interaction is visually observed; Shipping has no dev-only omniscient/debug widgets; other platforms remain NOT_RUN until tested |

Test in layers: portable C++ presenter semantics and typed projection contract -> UE automation for subscription, focus and serialization boundaries -> cooked/packaged rendered keyboard/pointer runs -> distinct device validation. Record exact result categories (PASS, FAIL, BLOCKED, INFRA_ERROR, INCONCLUSIVE, SKIPPED, NOT_RUN) and do not promote design or editor-only screenshots to package acceptance.

## 10. Design and documentation maintenance checklist

- Maintain a small UI style guide: logical spacing, typography, icon/shape vocabulary, semantic colors, text contrast, focus ring, alert urgency and report freshness; never embed third-party protected textures.
- Before implementing any displayed gameplay capability, identify the owning authoritative source and permitted observation projection. If missing, author and test that contract first.
- For every interactive widget define semantic action ID, participant/generation scope, target/cancel handling, enabled and disabled reasons, expected admission outcome and regression fixture.
- For every new resource, military state, faction mechanic, research or commander skill define its *normal, empty, shortage, stale, unauthorized and terminal* HUD representation.
- Align this HUD contract with the [UI Overview](POLITIKA_UI_OVERVIEW.md), [Keymap](RTS_UI_SETTINGS_AND_KEYMAP.md), [Engine Outline](ENGINE_OUTLINE.md), [Formations](RTS_UNIT_FORMATIONS_AND_DISCIPLINE.md), [Selection](RTS_SELECTION_AND_GROUP_ORDERS.md), [Visibility](RTS_VISIBILITY_AND_INTELLIGENCE.md) and [Completeness Matrix](RTS_SYSTEMS_COMPLETENESS_MATRIX.md) without copying domain-rule authority into this file.

**Bottom line:** a good POLITIKA HUD displays the consequence of *authorized knowledge and action*. Its novelty is the integrated treatment of tactics, formation organization, dollar-funded logistics, production, state institutions and uncertain political intelligence; its technical integrity depends on retaining a single authoritative simulation and one typed command route.
