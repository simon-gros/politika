# POLITIKA — Match Save, Replay and Determinism Boundaries

**Status: PROPOSED, not implemented or qualified, 29 October 2026.** The existing shell/local profile save mechanism does **not** establish campaign/match state saving, replays, spectator mode, networking or lockstep determinism. [Engine Outline](ENGINE_OUTLINE.md) owns when these are prioritized; [Versioning](VERSIONING_RELEASE_MANAGEMENT_ROADMAP.md) owns compatibility counters; [Shell](SHELL_SKELETON.md) owns existing profile reliability; [Command Contract](RTS_COMMAND_SIMULATION_CONTRACT.md) owns current event and fixed-step semantics.

## 1. Three distinct persistence products

| Product | Intended persisted data | Not included |
| --- | --- | --- |
| Profile/preferences (existing shell) | Local profile identity, UI/device settings, current integrity envelope and schema | Match units, fog contacts, opponent state |
| Campaign/match snapshot (future) | Stable content/map/mission identities; schema and game revision; match generation/step; participants/team/neutral state; live entities/orders/routes; resources/queues; objectives/results; observer claims where required; AI seed/memory | Raw UObject pointers; local developer-only fixture selectors; foreign UI truth |
| Command replay (future) | Initial scenario/content identity + deterministic seed, ordered stamped player/AI intents, admitted/rejected outcomes, version compatibility and periodic checksums | Automatic promise of bitwise engine or physics determinism |

A **recorded command log is not necessarily a replay**. Rendering can vary by GPU, and UE physics/Recast/AI behavior may change across engine builds. A replay is validated only if re-simulation reaches specified normalized authoritative checkpoints with matching outcomes. Otherwise retain it as an *event trace for diagnostics*.

**Future snapshot dependency:** a restored economy must preserve in-game **USD cents**, owned/reserved gold and fossil commodities, each functional building's positive minimum electricity load and current eligible supply, and staffed-force food stocks plus pending purchases/upkeep counters. Recompute power allocation and evaluate overdue food only at a documented authoritative restore boundary; do not double-charge elapsed intervals, repeat a gold/fuel sale or duplicate a fulfilled food purchase. This is a proposed save-format requirement, **not** a claim match saving exists. See [USD economy and upkeep](RTS_OPERATIONAL_ECONOMY_AND_POWER.md).

## 2. Snapshot boundaries and integrity

Write only at a documented quiescent authoritative step, after command admission and publication, not halfway through combat/death cleanup. Never serialize a raw `AActor*`, `TWeakObjectPtr`, transient UAsset location string as a live ID, or stale controller association. Use stable definition IDs with schema/content hash, participant and entity numeric IDs scoped to match generation, and an explicit unit of measure for fixed-point time/currency.

The save envelope should identify `format/schema`, game/runtime and engine compatibility, authored content checksum/version, map ID, current step, integrity hash and payload size. Strict upper bounds prevent corrupt/truncated files from allocating unbounded memory. Before deserializing Unreal objects, validate envelope length, checksum and supported schema; unsupported future-version content is preserved rather than silently overwritten. Separate temporary write/commit and directory/file preservation policies per platform. Failed write/load returns a structured recovery reason and must leave prior valid save bytes unchanged.

On restore, validate content references, eligible starts/occupied cells, unique runtime IDs, participant/faction/Unaligned authority, pending requests and event ordering. Rebuild runtime pointers/navigation from stable state under a documented policy, and fail closed atomically if restoration is impossible. A restored HUD is regenerated from the authorized observation snapshot; it cannot become an omniscient overlay.

## 3. Replay format and authoritative determinism

Candidate event-envelope fields: `replay_version`, `game_version`, `world_schema`, `engine_pin`, `content_digest`, `map_id`, `initial_seed`, `initial_state_digest`, and ordered `(step, within_step_sequence, participant_id, issuer_id, intent, target/map position, correlation_id)`. Record outcome IDs/rejections and periodic canonical state hashes (not a hash of camera/UI frame). If command payloads or gameplay rules change, choose an explicit migration or incompatible verdict; do not assert old recordings will replay seamlessly.

The current 0.05-second fixed-step coordinator with capped accumulation is valuable for repeatable **specified** test fixtures, but is not proof of universal bitwise determinism. Distinguish repeatability under different render schedules **on a fixed build/platform** from cross-CPU/platform determinism. Recast query order, floating-point math, actor order, task timing and asynchronous work require careful control before any lockstep or network replay claim. Multiplayer/peer networking is an independent future architecture decision; do not bolt it onto a save format.

## 4. Unreal 5.8 integration evidence

Epic's [Saving and Loading Your Game](https://dev.epicgames.com/documentation/unreal-engine/saving-and-loading-your-game-in-unreal-engine) describes custom `USaveGame` classes and sync/async slot functions, with `AsyncSaveGameToSlot` recommended to avoid hitches for larger data. This provides a storage interface, **not** an automatically valid POLITIKA world snapshot or atomic replacement guarantee. The existing local profile integrity model is a useful pattern; reuse validation discipline without commingling profile and campaign slots.

Epic's [Automation Framework](https://dev.epicgames.com/documentation/unreal-engine/automation-test-framework-in-unreal-engine) supports isolated tests and [Unreal Insights](https://dev.epicgames.com/documentation/unreal-engine/unreal-insights-in-unreal-engine) helps investigate performance and timing, but neither confers determinism. Keep replay/test artifacts out of player-saved slots and respect privacy when collecting analytics/crash traces.

## 5. Deferred acceptance matrix

| ID | Required proof |
| --- | --- |
| SAV-01 | Baseline existing profile still loads independently with zero campaign slots |
| SAV-02 | Valid match snapshot saves at declared step; restore reproduces entities, orders, resources, objective and permitted knowledge |
| SAV-03 | Truncated/checksum-failing/oversized/unsupported-future saves reject without modifying original bytes |
| SAV-04 | Failed write leaves previous valid file intact; later idempotent retry succeeds |
| SAV-05 | Missing content/map/definition ID fails before partial Actors or ownership commit |
| SAV-06 | New match/generation rejects all earlier commands, contacts, handles and stale replay records |
| SAV-07 | Replay of a known scenario matches periodic normalized state/event hashes on the **qualified** same platform/build |
| SAV-08 | Deliberately perturbed ordering/seed/content fails with a localized divergence report |
| SAV-09 | Two stable profiles retain their respective saves/settings without cross-profile clobber |
| SAV-10 | UI shows save/pause/compatibility and corruption recovery accurately; verify in cooked package |

Save/replay is **not** a Stage 4F exit requirement, as specified in the existing roadmap. Document this architecture now to avoid accidental coupling, then implement only for a concrete campaign, player continuation, debugging or multiplayer requirement.
