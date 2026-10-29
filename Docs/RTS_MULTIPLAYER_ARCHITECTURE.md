# POLITIKA — Multiplayer Architecture and Incremental Implementation Contract

**Status: APPROVED FUTURE DIRECTION / NOT IMPLEMENTED OR QUALIFIED, 29 October 2026.** Multiplayer is an intended future capability, not a release promise or an accepted runtime feature. This document owns server/client responsibility, transport-independent command and observation seams, hosting models, and the proposed multiplayer acceptance gates. [Engine Outline](ENGINE_OUTLINE.md) remains the **single owner of implementation order and milestone promotion**; [Match Structure](MATCH_STRUCTURE.md) owns roster and faction rules; [Visibility and Intelligence](RTS_VISIBILITY_AND_INTELLIGENCE.md) owns information authorization; [Match Persistence and Replay](RTS_MATCH_PERSISTENCE_AND_REPLAY.md) owns saves/replays and determinism claims; [Platform Support](PLATFORM_SUPPORT.md) owns actual device/platform qualification. Nothing in this document reclassifies existing prototype tests as network evidence.

## 1. Decision and scope

**Adopt multiplayer-aware authority boundaries now; defer full networking and services.** POLITIKA is an offline-first Unreal Engine 5.8.3 C++ RTS with a proposed future ceiling of **four human players in one match**. The same authoritative match simulation must support (a) standalone offline campaign/skirmish, (b) a player-hosted **listen server**, and (c) a separately hosted **dedicated server**, without creating another game-rule implementation. A listen-server host is one of the four human participants, not an extra fifth seat. A dedicated server has no human player of its own. Additional playable slots may be filled by server-run AI according to the selected game mode. Standard maps have **2–4 occupied playable-faction slots plus exactly one separate, NPC-only Unaligned authority**; four human players is the upper bound, not an obligation to fill every slot with humans.

This is **not** a commitment to ship networking in the first offline release. Do not split the project immediately into two unrelated applications or rewrite gameplay in a second language. UE game, server, and editor targets may share C++ gameplay/source while differing in launch mode, rendering, packaging, network ownership, and active services. No permanent server, account database, matchmaking provider, relay service, hosted billing, Steam service, or public lobby is required by the immediate work.

**Non-negotiable:** local campaign/skirmish, user settings, local profiles and save access must remain functional with the network disconnected, no external account, and no provider plugin. Network work cannot silently expand or invalidate the Stage 4A–4F First Playable Skirmish acceptance gates.

## 2. Existing repository baseline versus missing capabilities

The implementation currently provides a single authoritative prototype coordinator (`APolitikaPrototypeMatch`), 0.05-second / 20 Hz scheduled simulation, typed orders and per-command rejection reasons in `Source/PolitikaCore/Rules.h`, stable entity and participant identifiers, match generation/command sequence/correlation fields, and separate local controller/HUD. Authored runtime participants include slot ID, runtime player ID, faction and controller kind. The Unaligned system authority exists as a separate record, but autonomous Unaligned behavior is not yet present. The existing prototype controller obtains `GetLocalRuntimePlayerId()` and submits `SubmitLocalControllerCommand(...)`. These are **local-only seams**, not proof that a remote connection has an authenticated participant or that a client can call commands remotely.

The shell documents dedicated-server guards but **no compiled Server target**. No audited game-state replication, client/server RPC admission, LAN join, remote perspective, network-relevant observations, two-machine gameplay, reconnect, or four-human session is verified. The current fixed-step repeatability evidence does **not** establish cross-platform deterministic lockstep, packet-order safety, or network synchrony. See [Stage 4A](STAGE4A_MATCH_LIFECYCLE_ACCEPTANCE.md), [Navigation](RTS_RUNTIME_NAVIGATION_ACCEPTANCE.md), and [Shell](SHELL_SKELETON.md) for their narrower evidence.

### 2.1 Mandatory whole-repository multiplayer retrofit audit (before MP-1)

**Required: find, inspect, and account for ALL code already written that must be updated for multiplayer RTS gameplay.** Do **not** assume that the present coordinator, UI, actors or support systems are network-correct merely because rules are centralized, and do not limit the audit to newly added networking files. Before implementation of MP-1, perform a reproducible audit of the **current HEAD**, including **all already-authored C++ runtime/editor code, tests, UE gameplay assets and Blueprints, maps, target/build files, configuration, packaged launch scripts, content schemas, UI handlers, save/replay infrastructure, tools, and tests** wherever they can create, change, serialize, expose, or consume RTS game state. Re-run it after major gameplay additions and before multiplayer acceptance. Include code added or changed since the baseline; do not treat this dated document's examples as a complete source inventory.

For **every** gameplay-related code path, identify: (a) current file/symbol and call chain; (b) whether it runs in standalone, listen authority, listen local client, remote client, dedicated authority or editor only; (c) who currently writes/reads world state; (d) whether server-only writes, connection-bound orders, observation filtering, replication, lifecycle and tests are missing; (e) the **specific required code change**, or a justified **no-change** finding; (f) owning stage/priority and dependencies; and (g) the verifying test/evidence. A file can contain multiple issues and requires per-function or per-data-flow findings, not an arbitrary blanket PASS.

**Minimum initial search surfaces (known candidates, not an exhaustive audit):**

| Existing location / surface | Required examination and likely retrofit question |
| --- | --- |
| `Source/PolitikaCore/Rules.h`, command/admission tests | How request fields, participant attribution, generation, order sequencing, ownership, validation, events and serialization resist remote spoofing, repetition, stale matches and hidden-target side channels |
| `Source/Politika/Public/PolitikaPrototypeGameplay.h`, `Source/Politika/Private/PolitikaPrototypeGameplay.cpp` | `APolitikaPrototypeMatch` fixed-step/tick ownership; `APolitikaPrototypeUnit` movement/combat/health/state; objective capture; `APolitikaPrototypePlayerController`, HUD and GameMode; mutable local selection/feedback; `GetLocalRuntimePlayerId()` and `SubmitLocalControllerCommand()` local-only paths; authoritative versus replicated state/proxies |
| `Source/Politika/Private/PolitikaInfantryCombat.cpp` and infantry presentation/behavior source | Combat and ability entry points, projectile/damage authority, cooldowns, audio/animation/VFX side effects and visibility to remote observers |
| `Source/Politika/Private/PolitikaGameInstance.cpp`, shell controllers/GameModes, UI and settings code | Client-only profile/menu/UI initialization; dedicated-server headless safety; session/world travel; offline preservation and matchmaking independence |
| `Source/Politika/Private/PolitikaTestMapMusicComponent.cpp` and other presentation components | Cosmetics only on permitted rendering clients, not dedicated authority; no animation/audio callback may change authoritative rules |
| `Source/Politika/Private/Tests/`, portable C++ tests and all later acceptance harnesses | Adapt existing local-only tests to cover server-only execution, remote ownership enforcement, permitted observation, join/leave/rematch and simulated lag; retain original offline regression coverage |
| `Source/PolitikaEditor/`, map/content assets and validators | Authored slot/faction authority, spawn/ownership, identity/definition mapping, soft references and cooked client/server content; do not require editor services in a server |
| `Config/`, `*.Target.cs`, `*.Build.cs`, tools and packaging scripts | Replication/RPC configuration, transport/port assumptions, real Server target (when scoped), asset cooking, headless launch, version/protocol checks and platform-specific availability |
| Future economy, structures, AI, fog/intelligence, formations, research, talents, espionage and Unaligned code | Enforce this audit **as each subsystem is implemented**, even if the file did not exist at the date of this record; transactions, knowledge, NPC decisions and group orders must pass authority/visibility review |

**Deliverable:** a dated, reviewable `Docs/Reports/RTS_MULTIPLAYER_CODE_AUDIT_<YYYYMMDD>.md` (or a versioned machine-readable companion linked from it) with the exact inspected commit, engine pin, inventory method, all paths scanned, concrete per-symbol findings, priority (`must fix before MP-1`, `before MP-2/3`, `later`, `no change`), affected gameplay tests, planned remediations and open risks. Include negative findings and source coverage, including files excluded from modification and why. Link every actionable finding to an issue/implementation increment and close it **only with code + tests + evidence**, not with another design statement. Gate MP-1 on completion of the baseline audit and fixes **essential to its two-client slice**; later-stage findings remain tracked rather than forcing full multiplayer development immediately.

**Explicit audit acceptance criteria:** (1) enumerate actual current source tree and generated runtime/cooked gameplay components; (2) trace complete input→command→simulation→event→projection call paths, local and network modes; (3) identify every direct mutation of position, health, ownership, money, power, food, AI/order state and objective status and name its authoritative owner; (4) check all replicated/RPC-exposed state for per-client confidentiality and validate every trust boundary; (5) verify source code and UE assets do not assume exactly one local player or one renderer; (6) inventory phase transitions, pending work, teardown, reconnection/disconnect policy seams and generation guards; (7) verify no duplicate rules/simulation; (8) rerun source coverage after major changes. A search hit list alone does **not** satisfy the audit: inspect code and produce traceable findings. This section records a **required future audit**, not a claim that it has already been completed.

## 3. Topology and authority ownership

```text
Offline: local human + local AI -> one standalone authoritative match -> local presentation
Listen: host client + up to three remote clients -> host's authoritative match
Dedicated: two to four remote human clients -> headless authoritative match
                                           + server AI + NPC-only Unaligned
```

All simulation processes run **only where authority resides**. The following names are responsibilities, not mandatory new Unreal modules or duplicate pipelines.

| Responsibility | Authoritative process | Per-client behavior |
| --- | --- | --- |
| Participant admission, roster, teams and faction/slot association | Server/standalone match orchestration | Request admission; display validated permitted roster |
| Orders: selectability, ownership, target eligibility, admission, execution | Server command coordinator and shared C++ rules | Select local visible/controllable objects; submit intent; show pending/accepted/rejected feedback |
| Unit movement, path selection, navigation interruption, formations and discipline | Server fixed-step coordinator | Interpolate authorized updates; animate, show order cues |
| Combat, damage, health, cooldowns, abilities and deaths | Server | Render authorized state and cosmetic reactions |
| Money (`$`), commodity exchanges, electricity, food, research, production and talents | Server-ledgers and rules | Issue permitted transactions; display authorized projections |
| Playable opponent AI and NPC-only Unaligned institutions | Server, with separately scoped AI authorities | Never run competing consequential AI |
| World truth, fog, sightings, deception, espionage and diplomatic intelligence | Server world state + observer-specific projection | Consume only permitted observations and sanitized action outcomes |
| Objectives, win/loss, timed events, pause/resume and rematch state | Server | Display outcomes and lifecycle transitions |
| Camera, mouse selection, keyboard input, local hotkeys, HUD, audio and visual effects | No gameplay authority; local client | Own local user experience; may optimistically show cosmetic feedback |
| Session discovery, invites and platform sign-in | Optional future session/provider layer | Never overrides authority or blocks offline play |

Use Unreal's established ownership split: `AGameMode` for server-only admission/policy, `AGameState` for appropriately replicated match-wide state, `APlayerState` for shared participant metadata that is actually authorized to be public, `APlayerController` as the owning connection's command RPC ingress and local input boundary, and focused match/simulation objects for gameplay logic. GameMode/PlayerState are **not substitutes** for the existing coordinator or typed rules; Unreal PlayerControllers are not a globally replicated list visible to every remote client. A non-replicated internal authoritative state model is valid; actors/proxies or bounded state-delta structures may transport only the permitted presentation data.

Server and client must share **content identifiers, definitions, and rules protocol compatibility**, but clients never become co-equal simulation authorities. Unreal's built-in actor/property replication and reliable/unreliable RPCs are starting options. Select custom batch/delta mechanisms only after actual unit-scale profiling. The server's 20 Hz simulation step is **not** a mandated 20 Hz per-actor network update or rendering frequency.

## 4. Command ingress and trust boundary

Keep the existing `politika::rules::CommandRequest` / order admission path authoritative and usable by offline controllers, server-owned AI and network-controlled humans. Add only an adapter that converts a permitted remote intent into an existing validated request; **do not** add another movement, combat or resource-execution pipeline.

1. A client collects one or more unit instance IDs, intent kind, allowable parameters (target ID or map position), a correlation ID and a client-local sequence/order nonce. A batch request must have bounded size and payload.
2. The server receives it through an **owning** PlayerController or similarly connection-bound Unreal RPC. Never accept a client-supplied participant ID, team, faction, owner, authority token or server simulation step as proof of authorization.
3. The server resolves the live connection -> authenticated match participant -> occupied legal slot -> faction and current command permissions. The server creates/assigns its canonical `participant_id`, current `match_generation` and authoritative sequencing metadata. An offline or server AI caller uses the same command admission without pretending to be a network connection.
4. Validate schema version, match phase, generation, instance existence, life/status, ownership/temporary delegated control, permitted target knowledge, bounds, range/ability, queue legality and rate/size budgets. Revalidate relevant constraints when an accepted command takes effect. Stale, replayed, duplicated, malformed and out-of-order submissions must never cause unintended repeated effects.
5. Admit or reject through the **existing** C++ kernel; execute only in the server's ordered simulation step. Return a correlation-linked, participant-safe result. Rejection codes and logs must not disclose hidden actors, secret owner state or classified intelligence.
6. Send only relevant resulting state/events to each participant. The interface may locally acknowledge an input gesture without promising that the action has succeeded.

Do not replicate every mouse move or selection click as an authoritative world action. Local control groups are client-local organization, while actual issued unit orders and shared formations/stances are server-validated. Money spending, purchases, sabotage, espionage, contract transfers and unit-conversion authority require the same connection-bound admission and transactional server ledger as combat. Tests must include hostile client attempts to order an enemy's units, impersonate a different slot, spend another participant's money or modify an Unaligned entity.

## 5. Participant-specific state and information security

The server retains the single world truth `S(t)` and constructs per-participant allowed observations `O(p,t)` under [Visibility and Intelligence](RTS_VISIBILITY_AND_INTELLIGENCE.md). Replicate **only** data authorized for that connection. Visual fog alone, invisible rendered actors, and replicated-but-hidden properties are not security boundaries: remote game processes and memory can be inspected. Do not send unknown enemy positions, concealed inventory/build queues, covert actions, undiscovered technologies, fabricated-report truth flags or Unaligned internal privileged state to ineligible players.

Share legitimately known public/diplomatic information intentionally, not accidentally through a global `GameState`/`PlayerState` field. Separate live visibility, remembered last sightings, unverified reports and intentionally false reports; stale contacts must not become authoritative target locks through retained network references. Reconnect/spectator projections require independent permission models before those features are enabled. Development-only omniscient traces must not enter Shipping packets, logs or accessible client telemetry.

A state update may describe the subset of unit transforms/health/status relevant to a player, with current visibility/knowledge semantics; model effects on the client's presentation separately. The initial implementation should establish correct filtering for a minimal map before optimizing network replication volume.

## 6. Transport, scaling and rendering strategy

**Decision:** use a server-authoritative, state-replication model, **not deterministic peer-to-peer lockstep**. The existing repeatable 20 Hz coordinator is valuable for server simulation and test fixtures, but Unreal/Recast scheduling, floating point, cross-platform execution and evolving AI do not justify assuming exact distributed determinism. Local snapshots/replays are distinct from network synchronization. Never require a full engine-state broadcast every step.

Start with Unreal conventional replication for the smallest unit/objective case and correct owner-specific projections. Instrument commands per second, bytes per client, packet loss, actor relevance, burst cost, server step timing, and client render stutter. Scale to multi-unit and mass-formation fixtures before deciding whether batches, fast-array-style deltas, relevance sectors, dormancy, Replication Graph or other facilities are justified; do not prematurely require Iris, Replication Graph, Mass or a proprietary network stack. Interpolate permitted movement on clients, while the server alone chooses paths, resolves collision/arrivals, damage and authoritative target transitions. Correctness under loss/reordering and fog outranks smooth cosmetic prediction.

No architecture choice in this document guarantees that a listen server can host large battles on all advertised target devices; profile separately. A four-human limit does **not** imply only four actors or a trivial network load.

## 7. Session and hosting decisions

**Two future hosting modes, one executable ruleset:**

- **Listen server:** one of the participating human players hosts a networked match locally; total human participants across the host and clients is at most four. Host has latency and authority advantage, and host exit generally terminates that match unless a later tested migration mechanism exists. No host migration is promised.
- **Dedicated server:** an independently launched, headless server runs the same rules; up to four humans connect as clients and no local human occupies a server seat. It can run on a rented/owned computer; a managed service is optional rather than mandatory.
- **Offline standalone:** the same command/rule core runs without sockets, platform sign-in or session discovery.

The current roster permits two to four playable slots populated by humans and/or AI, with distinct canonical playable factions and separate Unaligned NPC authority. The server validates slots before `Active`, maps authenticated connection identities to participants, and rejects over-capacity, duplicate or incompatible joins. What occurs on mid-match disconnect (pause, AI takeover, surrender, grace timeout) is a **deferred policy decision**; reconnect, spectators and hot joining are not accepted capabilities. The Unaligned can never be recruited as a human/extra fifth slot. Match hosting and provider identity must not redefine unit control, temporary transfers, neutral hostility or faction uniqueness.

A basic LAN or direct-address session is the first target. Public internet player hosting may require NAT traversal, port forwarding or relays; no selected service or cross-platform compatibility is assumed. Steam/EOS/platform account services, browser discovery, friend invites, moderation, authentication and operational expenses require separately scoped work. Never treat a local profile ID as a trusted internet identity.

## 8. Compatibility, saves and lifecycle boundaries

Prior to admission, compare approved game/protocol versions, server content/map IDs and relevant content digests. Reject incompatible clients with a user-visible generic reason and useful **nonsecret** local diagnostics. Adopt an explicit on-wire protocol version when the first network payload is introduced; avoid changing global game SemVer, save schema or content IDs just for design text. Saved matches, replays and snapshots have separate versioned contracts; a networked state frame is not automatically a valid restorable save.

The server owns match phase transitions and terminal result. At end/return/rematch, revoke previous controller-to-participant mappings, flush queued/stale commands and route completions by generation, discard connection-specific observation caches and initialize a fresh match. Test stale input arriving after a match exit or a same-process rematch; never let it affect the new match. Offline pause may freeze local simulation, while network pause must be an explicitly server-controlled ruleset (not one client freezing everyone through its menu).

Server startup, failure and shutdown cannot require Slate/UMG, audio devices, a local user profile or a renderer. `-nullrhi` alone is **not** proof of a dedicated server. Create a UE Server build target and package/launch a real headless binary only in the dedicated-server implementation stage.

## 9. Dependency-ordered delivery and gates

These **proposed** gates do not change Stage 4F exit conditions and are **not achieved** merely by documentation.

| Gate | Scope and dependency | Evidence required |
| --- | --- | --- |
| **MP-0 — mandatory retrofit audit + multiplayer-ready boundaries (NOW)** | Perform and maintain the §2.1 whole-repository code audit, then during ordinary Stage 4A–4F source changes: keep the existing one-coordinator/one-rules pipeline; isolate local camera/HUD/selection and client-only side effects; make all mutable world state authoritative; retain stable participant/entity IDs, command/outcome schema and explicit permitted observation boundary. | Complete source inventory with per-symbol changes/no-change findings and tracked remediations; source review and offline regression showing no new simulation path, no unauthorized UI write and no offline boot/save regressions. No network feature claimed. |
| **MP-1 — minimum network vertical slice** | Once lifecycle/participant authority and basic move/attack admit reliably, connect **two separate playable clients** to a single listen server using the *existing* coordinator; one local host and one remote client, each controlling a distinct legal slot. May be scheduled independently after the relevant foundation; not a precondition for 4F. | Real UE networked execution (not two controllers in one standalone world), server-only damage/navigation, client-independent perspective/selection, ownership-denial and authorized results, enter/exit/rematch isolation, 0% additional authority paths. |
| **MP-2 — multi-unit and adverse-network qualification** | After MP-1 and a sufficiently integrated skirmish; add bounded ordered group commands, relevant state replication, fog-safe projections, economy transaction tests and server AI/Unaligned behavior as each becomes implemented. | Automated network emulation for latency/loss/reordering, bounded payloads and command duplicates, 2/3/4-seat joins with AI-filled legal slots, unit-scale bandwidth/server-step measurements and explicit failures with source/package identity. |
| **MP-3 — dedicated-server parity** | After MP-1 and supported build configuration; create real `PolitikaServer.Target.cs` or appropriately named Server target, headless boot and 2–4 clients with the same rules/map. | Server binary built/cooked/packaged, no UI/audio/local profile dependency, dedicated vs listen gameplay equivalence and mode-specific lifecycle/disconnect tests. |
| **MP-4 — public multiplayer readiness** | After MP-2/MP-3: choose internet sessions, identity, NAT/relay or direct IP, invitations, hosting UX, reconnect/host-loss policy, security and operations; qualify actual target platforms separately. | End-to-end joining from independent machines/networks, capacity/anti-impersonation and information-leak tests, diagnostics, failure/recovery, performance, platform/provider and release acceptance. |

### Minimum automated test inventory (future)

| ID | Scenario | Passing observable |
| --- | --- | --- |
| MP-T01 | Offline startup without network/provider | Existing offline menu, local skirmish and local storage remain available |
| MP-T02 | Two UE processes on one listen server | Both see the same authorized match outcomes; no independent client simulation |
| MP-T03 | Remote command to own unit | Exactly one accepted server command with correct participant and correlation |
| MP-T04 | Spoofed player ID/foreign unit, invalid team/Unaligned order | Server rejects; protected state unchanged; response leaks no secret target data |
| MP-T05 | Stale generation, duplicate/late/reordered command | No double action, no cross-match bleed or stale route |
| MP-T06 | Server death/arrival, attack, objective completion | Server outcome is authoritative; client visuals converge to allowed view |
| MP-T07 | Hidden unit, concealed economy/intelligence/espionage | Never serialized to unauthorized connection; no rejection side-channel |
| MP-T08 | 2/3/4 legal human/AI seat configurations | Exactly 2–4 occupied playable slots and one NPC-only Unaligned, no fifth human |
| MP-T09 | Delay, loss, bandwidth throttle and transport interruption | Controlled degraded play/diagnostic; no authority divergence or unsupported guarantee |
| MP-T10 | Server step/command bandwidth stress with formations | Measured budgets, not inferred scalability from small smoke test |
| MP-T11 | Dedicated headless parity with listen/standalone | Same rule outcomes for specified fixtures, separate mode-specific evidence |
| MP-T12 | Match exit, rematch, disconnect, host loss | Specified policy enforced; no stale participants, callbacks or allocations |

Capture exact Git commit/dirty status, Unreal 5.8.3 source pin, process mode, target and package identity, test case, network profile, map/content digest, measured stats and logs for every PASS. `NOT_RUN` and `INCONCLUSIVE` must not be reported as acceptance. Design-only documentation does not authorize changing a release matrix to green.

## 10. Unreal reference material (reviewed 29 October 2026)

- [Networking Overview](https://dev.epicgames.com/documentation/unreal-engine/networking-overview-for-unreal-engine) — client/server authority, standalone/listen/dedicated modes, plan-ahead guidance.
- [Detailed Actor Replication Flow](https://dev.epicgames.com/documentation/unreal-engine/detailed-actor-replication-flow-in-unreal-engine) — relevance, scheduling, replication and changed-property behavior.
- [Actor Relevancy](https://dev.epicgames.com/documentation/unreal-engine/actor-relevancy-in-unreal-engine) — per-connection actor filtering, which alone does **not** constitute POLITIKA's classified-information authorization.
- [Testing Multiplayer](https://dev.epicgames.com/documentation/unreal-engine/testing-multiplayer-in-unreal-engine) — editor listen/dedicated testing.
- [Network Emulation](https://dev.epicgames.com/documentation/unreal-engine/using-network-emulation-in-unreal-engine) — packet loss and lag simulation.

All technical choices above are POLITIKA proposals grounded in the referenced UE facilities. Refer to the pinned 5.8.3 API/source before implementing; tutorial examples and general features are **not** evidence that POLITIKA supports them.
