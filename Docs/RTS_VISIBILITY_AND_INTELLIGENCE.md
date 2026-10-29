# POLITIKA — Authoritative Visibility and Intelligence Contract

**Status: PROPOSED specification, 29 October 2026. Not implemented or tested.** Owns the tactical *information authorization* seam. [RTS ontology](RTS_GAME_ONTOLOGY.md) owns philosophical classification; [Reconnaissance](RECONNAISSANCE_AND_SCOUTING.md), [Espionage](ESPIONAGE_AND_INTIMIDATION.md) and [Unaligned counterintelligence](UNALIGNED_COUNTERINTELLIGENCE.md) own faction/canon effects; [UI](POLITIKA_UI_OVERVIEW.md) owns layout; [Engine Outline](ENGINE_OUTLINE.md) owns implementation order.

## 1. Invariant: truth, observation and presentation differ

Authoritative state `S(t)` records entities/owners, positions, production, health, orders, territory and event history. An observer-specific projection `O(p,t)` is derived through deliberate sensors, diplomatic/intelligence permissions and knowledge freshness. The HUD, minimap, AI and campaign scripting must access `O(p,t)` (or narrower documented views), **not arbitrary world state**. A hidden unit in `S` is neither destroyed nor permitted to be commanded by ID simply because a UI retained an old handle. Enforce the observation gate again at **command admission/execution** for contact-targeted actions, where policy requires a currently valid target; never rely on widget masking.

Distinguish a contact's **current sensed state**, **last-known state**, **unconfirmed report**, **known-to-be-false report**, and **unknown**. The authoritative engine may know an injected report is false; the participant must not see that private classification before a legitimate verification event. The neutral Unaligned system authority has its own privileges and *no ordinary local-human participant ID*.

### Proposed minimal observation record

| Field | Ownership / constraints |
| --- | --- |
| `observer_participant_id`, `match_generation`, `observed_step` | Authenticated viewer and provenance; no cross-match reuse |
| `contact_id` / ephemeral sighting key | Identifies observation without leaking raw hidden entity identity when unverified |
| `source_kind` and `source_reference` | Visual, sensor, reconnaissance, diplomatic, intercepted, propaganda, intelligence |
| `first_seen_step`, `last_seen_step`, optional `expires_step` | Finite temporal bounds; no future knowledge |
| `visibility_state` | Unknown / LastKnown / Reported / CurrentlyObserved; state changes authoritative |
| `position_estimate`, `uncertainty`, optional `class_confidence` | May be unknown; stale positions never silently treated as current |
| `alleged_owner`, `verified_owner` | Separate claims; unknown fields do not fall back to global truth |
| `provenance_id` / `verification_status` | Explain a rumor's source to a player without disclosing its hidden authenticity |

This is a **candidate interface**, not an immediate Unreal `USTRUCT` schema or a commitment to simulate all uncertainty dimensions.

## 2. Spatial visibility and stale contacts

For later fog-of-war introduction, choose a terrain/map-space grid or another measured field representation. Project sensor radius and LOS/occlusion from authored unit/structure capabilities. Visibility at a tick is the union of eligible alive/operational sensors **for the observer's authorized sharing relation**, not all actors in the level. Explored terrain is historical knowledge; *currently visible* cells are a separate mask. Hiding a unit moves its contact to last-known (if persistence is allowed); death outside current sight cannot be revealed immediately without an information source. Static structures may have a distinct remembered silhouette policy after discovery.

Critical design decisions before coding: whether fog exists at launch or first campaign; per-faction versus per-team sharing; terrain elevation and buildings' LOS role; cloaking/detection rules; targetability of last-known positions; expiration and positional uncertainty; observer-specific reveal and reason logging. Store these as content-profile choices rather than speculative fields on every asset today.

## 3. Different consumers and security boundaries

**Human input:** selection of owned units is always constrained by live ownership; enemy information is inspected only if observable. Attack-ground, if ever added, targets a position and does **not** conjure a hidden entity ID. A contact-specific Attack requires a valid target policy; denial must not leak whether an unobserved target exists. If a hidden unit is destroyed, avoid notifying non-observers via a global event.

**AI:** receive the same participant-scoped observations and permissible source types as a human participant. Record `observation_step`, `decision_step`, `chosen_intent`, admission result and whether claimed intel was verified. AI privilege should not depend on reading all `APolitikaPrototypeUnit` instances.

**UI/minimap:** render only the authorized view; distinguish known terrain, uncertainty and deception with accessible glyphs and textual explanations. The minimap is an *information projection*, never a second sensor. Global debug overlay is permitted only in isolated Development fixtures with explicit source and Shipping isolation.

**Narrative/Unaligned:** administrative reports, misinformation and propaganda can be authored as separate typed claims. Civilian harm and political legitimacy must not be inferred from ordinary vision alone. The historical archive's provenance applies to research assets, not to fictional truth claims inside a match.

## 4. Timing, invalidation and determinism

Run sensor update after completed movement/death state at a documented coordinator phase; publish per-observer changes before the subsequent decision tick. Use stable ordering for sensors/contacts where reproducibility matters. Purge observation caches on AdmissionRejected, Terminal/Results (unless a debrief snapshot is deliberately retained), Return and new match generation. On state restoration or replay, rebuild or restore observation records consistently; never attach a new match to old contact handles.

## 5. Acceptance matrix (proposed)

| ID | Required negative/positive proof |
| --- | --- |
| VIS-01 | Two participants, one hidden enemy: observer-specific query does not enumerate enemy or reveal position/health |
| VIS-02 | Sensor becomes eligible/occluded: visibility changes only on authoritative update step |
| VIS-03 | Last-known contact ages/stales without receiving hidden movement/death |
| VIS-04 | Enemy hidden ID submitted directly: valid owner/authority alone cannot bypass target-information policy |
| VIS-05 | Stale/destroyed/neutral entity pointers cannot produce selectable owned units |
| VIS-06 | Claimed false report stays epistemically unverified until approved detection evidence |
| VIS-07 | AI decision logs contain only observation-view fields |
| VIS-08 | UI/minimap outputs are a subset of observer projection; dev omniscience disabled in normal/Shipping package |
| VIS-09 | Return/rematch clears contacts and prevents cross-generation data leaks |
| VIS-10 | Fixed input/seed/schedule gives stable visibility transitions where deterministic ordering is claimed |

Start with pure C++ projection checks, then Unreal isolated observer tests, then a genuinely cooked map and rendered minimap/selection tests; absence of a visibility implementation is **NOT_RUN**, not FAIL for the completed prototype. Cross-platform determinism remains separate.

## 6. Unreal integration decisions

Keep authoritative visibility in the game simulation coordinator, not in UI widgets, `APlayerController` rendering traces alone, `AAIController` perception or Mass fragments until measured needs justify them. `NavigationSystem`/Recast addresses traversability, **not sensor visibility**. Epic's [Gameplay Tags](https://dev.epicgames.com/documentation/unreal-engine/using-gameplay-tags-in-unreal-engine) can label sensor/camouflage capabilities but should never substitute for participant permission checks. [EQS](https://dev.epicgames.com/documentation/unreal-engine/environment-query-system-quick-start-in-unreal-engine) might help choose a cover site later; it does not implement multiplayer-style fog of war or prevent omniscient target access. [UMG Viewmodel](https://dev.epicgames.com/documentation/unreal-engine/umg-viewmodel-for-unreal-engine) is a possible Beta UI adapter, not a source of truth.

**Gate:** Define/test the observation service independently after the 4F first-skirmish baseline unless a selected mission makes fog required earlier. Preserve the fact that current 4D prototype AI decisions may be based on expressly scoped *currently visible prototype inputs*, without pretending a complete visibility subsystem exists.
