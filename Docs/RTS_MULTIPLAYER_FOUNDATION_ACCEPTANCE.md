# MP-1 — Server-authoritative multiplayer foundation: acceptance record

**Date:** 29 October 2026 · **Owner decisions:** server-authoritative replication (not lockstep); stable entity IDs; commands through a server call; connection → participant mapping; replicated state and events; fog-of-war visibility decided now; a two-client test; layouts **1v1, 2v1, 3v1, 2v2** (at most four playable factions), each **plus the NPC-only Unaligned side**.

**Outcome: MP-1 IMPLEMENTED; packaged qualification PASS for the stated scope.** Two independent packaged Linux Development processes ran over 127.0.0.1 in the 1v1 + Unaligned layout. 2v1, 3v1 and 2v2 seat binding, teams, alliances and fog are implemented and **Automation-qualified only**; packaged 3–4-process runs are MP-2. Nothing here qualifies internet/LAN hosting across machines, cross-platform play, dedicated servers, scalability or release multiplayer.

Related: [Multiplayer Architecture](RTS_MULTIPLAYER_ARCHITECTURE.md) (§5.1 fog decision, §7.1 layouts) · [MP-0 code audit](Reports/RTS_MULTIPLAYER_CODE_AUDIT_20261029.md) · [Command contract](RTS_COMMAND_SIMULATION_CONTRACT.md) · [Match Structure](MATCH_STRUCTURE.md).

## A. What works

- **Connections and seats.** A listen-server host and an independent remote client connect through Unreal's `IpNetDriver`. The server binds each connection to exactly one participant. The host takes the authored human slot (seat 0, team A) and the client takes seat 1 (team B). A third connection is refused at `PreLogin` before any controller or seat exists. Client-supplied identity is never trusted: the intent payload has no participant field.
- **Commands.** The host and the remote client issue real-input Move, Attack-move, Stop, Guard and Sprint orders through **one** intake (`SubmitControllerIntent`) and the unchanged `PolitikaCore` admission. Each outcome reaches only its issuer.
- **Hostile input.** 24 forged, malformed, stale, duplicate, hidden-target, hidden-issuer and post-termination intents sent over the real RPC are all rejected with the expected reasons.
- **Server-only simulation.** Movement, navigation, combat, damage, abilities, objective and lifecycle advance only on the server. Clients render replicated state; every damaged-health state and death a client observed is server-originated.
- **Fog of war.** At start the client receives only its own two units. Host units are replicated to it only once its team observes them. The listen host renders only its team's view. The Unaligned side's units are fogged like enemies.
- **Results and lifecycle.**
  - The objective result is committed exactly once and shown on both processes (host VICTORY, client DEFEAT).
  - Both processes return to MainMenu, and a same-process rematch gets a fresh match generation and fresh bindings.
  - When a remote client is killed, the host detects it in 10.0 s, ends the match exactly once (`participant_disconnected`) and returns to MainMenu.
  - When the host is killed, the client detects it in 10.0 s and recovers to MainMenu with an actionable `NetworkConnectionLost` reason.
- **Offline compatibility.** Offline play is unchanged. The packaged PAC-10 rendered replay and the participant-authority replay pass, as do all 27 pre-existing Automation tests.

## B. Source changes (branch `codex/mp1-listen-server-20261029`)

| Area | Files | Change |
|---|---|---|
| Portable rules | `Source/PolitikaCore/Rules.h`, `Tests/Core/RulesTests.cpp` | `IntentPayload`/`DecodeIntent`, `ClientSequenceGuard`, `TeamLayout` + `SeatCount`/`TeamForSeat`/`ParseTeamLayout`, `AreAllied`, `CanAutoEngage`, `kUnalignedTeam`, `VisionSource`/`IsObservedBy`, `Entity::team/observed`; rejections `MalformedCommand=15`, `DuplicateCommand=16`, `RateLimited=17`, `TargetNotVisible=18` (appended). |
| Network boundary | `Source/Politika/Private/PolitikaMultiplayer.cpp` (new) | Replicated-property lists; session layout; team vision; `IsNetRelevantFor`; bindings, `PreLogin`/`PostLogin`/`Logout`; single intake; outcome routing; observer init/registry; Development-only probes and sampling. |
| Coordinator / units / HUD | `PolitikaPrototypeGameplay.{h,cpp}`, `PolitikaInfantryCombat.cpp` | Authority guards on every simulation entry point; team-aware hostility; fog-aware admission, acquisition and target loss; team capture and result; Unaligned spawn; per-seat camera focus; cosmetic multicast; HUD team/layout/fog/result. |
| Content | `PolitikaPrototypeContent.{h,cpp}`, `PolitikaCreateShellCommandlet.cpp`, `Content/Politika/TestFixtures/Multiplayer/*` | `SightRadiusCm`, `UnalignedPlacements` (+ validation), the 4-seat + Unaligned Development fixture with a spatial-validation gate. |
| Process / session | `PolitikaGameInstance.{h,cpp}`, `PolitikaLocalPlayer.{h,cpp}`, `PolitikaBootstrap.*`, `Config/DefaultEngine.ini`, `Politika.uproject`, `Politika.Build.cs` | Protocol login option; network-failure recovery; MainMenu recovery default; Generic replication; IpNetDriver timeouts and `RelevantTimeout=0.25`; `OnlineSubsystemUtils` (IpNetDriver only). |
| Tests and tools | `Private/Tests/PolitikaMultiplayerTests.cpp`, `Tools/Shell/src/bin/rts_multiplayer.rs`, `Tools/Shell/package-multiplayer.sh` | Five Automation tests; the two-process packaged runner; immutable Development/Shipping packaging with manifest checks. |

## C. MP-0 audit

The full inventory and the F01–F42 register are in the [audit report](Reports/RTS_MULTIPLAYER_CODE_AUDIT_20261029.md).

- **Fixed for MP-1.** Single-local-player authority; local-only ingress; unguarded simulation; missing replication; unit-list count leak; fog relevancy; target-existence oracle; owner-only hostility; local-only capture and winner; HUD leaks of other participants' commands; acknowledgement audio on every machine; music and map lighting only on the authority; client-side destroy; camera replication; remote menu exit mutating the match; network-failure recovery; protocol and capacity admission; disconnect termination; intake freshness; the Iris bypass; Unaligned authoring.
- **Deferred.**
  - Cosmetic shot target points can reveal unseen third parties in 3–4-seat games (F27, MP-2).
  - The listen host holds all actors in memory (F29, MP-3).
  - There is no Server target (F32, MP-3).
  - Client fog shading and last-seen contacts are missing (F33, MP-2).
  - The disconnect policy beyond ending the match is open (F35).
  - Bandwidth has not been measured (F36, MP-2).
  - Packaged 3–4-process layouts and AI-filled seats are open (F42, MP-2).

## D. Reference checkpoint

| Question | Source | Class | Finding / disposition |
|---|---|---|---|
| Do 2/3/4-player skirmish maps exist as authored data? | Tempest `politika-transfer/asset-index.csv` (`/Game/Tempest/Maps/Multiplayer/{2Player,3Player,4Player}/…_MapInfo`) | Registry-only | Supports map-authored player counts. POLITIKA keeps authored slots in the map definition and selects seats with a session layout. No Tempest content imported. |
| How do map slots bind to starts? | `relationship-crosswalk.csv` `TedMapInfo.Players → TedPlayerStart` (manual.txt:232–304) | Documented | Consistent with POLITIKA's `PlayerSlots[].StartLocation`; the networked camera focuses on the seat's start. |
| Fog / reveal semantics | `tag-index.csv` `Status.FoWRevealed`, `Definition.PassiveAbility.RevealFogOfWar`, `Ability.RevealStealth` | Registry-only (tags unreviewed) | Indicates per-unit reveal statuses and abilities. POLITIKA's MP-1 uses server-computed team sight circles; reveal abilities and stealth are future work. No runtime claim is made from tags. |
| Lobby / session UX | `tag-index.csv` `UI.GameCreationContexts.Lobby.*`, `GenericValue.Lobby.StartGameIssue` | Registry-only | A lobby exists in the reference; POLITIKA MP-1 deliberately has no lobby (direct address + URL layout option); MP-4. |
| UE APIs | Pinned UE 5.8.3 source: `UWorld::Listen` (World.cpp:7923), `UNetDriver` relevancy loop and `RelevantTimeout` (NetDriver.cpp:5735–5800, BaseEngine.ini `RelevantTimeout=5.0`), `AActor::IsNetRelevantFor`, `UEngine::WillNetDriverUseIris` + `UGameInstance::GetDesiredReplicationSystem`, `APlayerController::IsLocalController`, `AGameModeBase::PreLogin/PostLogin/Logout`, `FURL` options/`-port` | Verified in source | Used as implemented; Generic replication is pinned because the fog filter depends on `IsNetRelevantFor`. |

## E. Verdicts

Vocabulary: PASS / FAIL / BLOCKED / INFRA_ERROR / INCONCLUSIVE / SKIPPED / NOT_RUN. Final package results are recorded in §F.

| ID | Requirement | Environment | Verdict | Evidence |
|---|---|---|---|---|
| CORE | Portable rules (layouts, alliances, Unaligned, fog, intent decode, sequence guard) | `quality -- core`, C++20 `-Werror` | PASS | `build/diagnostics/core.json` |
| AUD | `quality -- audit` (docs index links, pins) | Rust | PASS | `build/diagnostics/audit.json` |
| AUTO-MP | `Politika.RTS.Multiplayer.*` — 5 tests: ParticipantBindingAndLayouts (MP1-01/02/03/04 for 1v1/2v1/3v1/2v2 + standalone + layout failures), CommandIntakeAndHostileInput (MP1-05–09, alliances, Unaligned, post-termination), FogOfWarVisibilityAndRelevancy, AuthorityOnlySimulation (MP1-10), ResultCommitAndCleanup (MP1-11/12, disconnect) | Editor Automation, real listen `IpNetDriver`, `-nullrhi` | PASS 5/5 | `build/diagnostics/mp1-listen-server/automation-mp/` |
| AUTO-ALL | Full `Politika` suite (27 existing + 5 new) | Same | PASS 32/32 | `build/diagnostics/mp1-listen-server/automation-all/` |
| MP1-F-LISTEN, LAYOUT | Host listens on the explicit port; 1v1 session = 2 seats + 2 Unaligned units, fog on | Packaged | PASS | `final-evidence.json` |
| MP1-01 / MP1-02 / G4 | Host → participant 1 team A; remote → participant 2 team B; two pids connected | Packaged | PASS | host/client logs |
| MP1-03 | Third connection refused at `PreLogin` (`MP.SESSION.IN_PROGRESS`); no seat created | Packaged (third process, `-nullrhi`) | PASS | `third.log` |
| MP1-FOG-INITIAL / REVEAL | Client initially receives only its own 2 units; host units relevant only once observed; host hides unobserved enemy/Unaligned units | Packaged | PASS | `MP_CLIENT_UNIT_RELEVANT`, `MP_SERVER_SAMPLE host_hidden` |
| MP1-POS-REMOTE / HOST-ORDERS | Real X11 input: remote Sprint/Stop/Guard/Move; host Move/Stop/Guard/Attack-move — all through the same intake | Packaged | PASS | logs + screenshots |
| MP1-NEG-PROBES | 24 hostile probes: foreign, hidden-issuer sweep 1–12 (incl. Unaligned), unknown issuer, hidden target (`TargetNotVisible`), invalid kind, NaN, shape, out of bounds, stale generation, protocol mismatch, friendly target, duplicate, reordered, after termination | Packaged (real RPC) | PASS | client `MP_PROBE_SENT`/`MP_COMMAND_OUTCOME` correlation |
| MP1-POS-DAMAGE | Every client-observed damaged health and death has a server record | Packaged | PASS | |
| MP1-11 | Exactly one result; team A; both processes observe it | Packaged | PASS | results screenshots |
| G6-NORMAL / MP1-12 | Both return to MainMenu; rematch generation 1 → 2 with fresh bindings | Packaged | PASS | |
| G6-CLIENT-LOSS / HOST-LOSS | Client SIGKILL → host unbinds/ends once (10.0 s); host SIGKILL → client recovers with reason (10.0 s) | Packaged | PASS | |
| OFFLINE-PAC10 | Existing rendered offline interaction replay | Packaged Development, Xvfb | PASS | |
| OFFLINE-AUTHORITY | Existing participant-authority packaged replay | Same | PASS | |
| SHIPPING | Linux Shipping package builds; fixture excluded; default offline MainMenu boot | Packaged Shipping | see §F | |
| 2v1 / 3v1 / 2v2 packaged | 3–4 process runs | — | NOT_RUN (MP-2) | Automation covers binding/teams/fog |

## F. Package and evidence identity

Filled from the final run on the committed tree; see the section appended below.

## G. Limitations and next work

- **Limitations of this evidence.**
  - Loopback on one Linux host only.
  - Xvfb with virtual input; no physical display or controller.
  - Audio acknowledgement routing is not measured.
  - Cosmetic multicast is unreliable by design.
  - The listen host has every actor in memory.
  - Fog is a relevancy filter without client fog shading.
- **Not implemented.**
  - AI-filled seats.
  - Reconnect, late join, host migration and spectators.
  - Lobby/session UI.
  - A dedicated server.
  - Online providers.
  - Replay/save over the network.
- **Next MP-2.**
  - 3–4-process packaged runs for 2v1/3v1/2v2.
  - Network emulation (latency/loss/reorder).
  - Bandwidth and server-step measurements.
  - Redacting cosmetic target points (F27).
  - Client fog shading and last-seen contacts.
  - AI-filled seats when opponent AI lands.
- **Next MP-3.**
  - A `PolitikaServer` target with a headless boot (no UI, audio or profile).
  - Listen/dedicated parity for the same fixtures.
