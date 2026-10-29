# MP-0 — Existing-code multiplayer retrofit audit (29 October 2026)

**Status:** baseline audit complete for MP-1. The MP-1 remediations below are implemented on branch `codex/mp1-listen-server-20261029`. Each one is backed by Automation and/or packaged two-process evidence; see [the MP-1 acceptance record](../RTS_MULTIPLAYER_FOUNDATION_ACCEPTANCE.md) for verdicts. Deferred findings stay open with an owning gate. This report follows [RTS Multiplayer Architecture §2.1](../RTS_MULTIPLAYER_ARCHITECTURE.md#21-mandatory-whole-repository-multiplayer-retrofit-audit-before-mp-1).

| Item | Value |
|---|---|
| Audited base | `origin/main` `ffdcf7340ba39d15c2ceb432cc5e8bf39620e714` (the historical brief baseline `b5788b4` is 85 doc-only commits older; it has no source differences relevant to the audit). |
| Remediated tree | Branch `codex/mp1-listen-server-20261029`, worktree `/mnt/data/politika-mp1-listen-server-20261029`. The commit is recorded in the acceptance record. |
| Engine | Unreal Engine 5.8.3, source revision `396c9f059903aed5fec78ecd3d437a40c6415368` (`tools/unreal-version.env`; `tools/unreal.sh doctor` PASS). |
| Owner decisions applied | Server-authoritative replication, not lockstep. Stable entity IDs. A server RPC for commands. Connection → participant binding. Replicated state and events. Fog-of-war visibility decided now. Supported layouts 1v1, 2v1, 3v1 and 2v2: at most 4 playable factions, always plus the NPC-only Unaligned side, so "1v1" is player vs player vs Unaligned. A two-client packaged test. |

## 1. Method

1. **Inventory.** Enumerate every file in `git ls-files Source Tests Config Tools tools *.uproject` plus the 170 tracked `Content/` assets, and classify each in §2.
2. **Trace the paths.** Follow input → command → admission → simulation → event → presentation by reading the code, for standalone, listen host, remote client and a dedicated-server candidate. Keyword hits (`GetFirstPlayerController`, `HasAuthority`, `LocalRuntimePlayerId`, `OwnerPlayer ==`, `Replicated`, `NM_*`) only seeded the reading; every hit was inspected in context.
3. **Mutation inventory.** For each mutation of position, health, ownership, orders, abilities, projectiles, objective, lifecycle or result, record its authoritative owner and its guard (§3).
4. **Exposure inventory.** Inventory every replicated property, RPC and log field for confidentiality (§4).
5. **Re-audit.** Re-run the audit after the MP-1 changes. New code (fog, team layouts, Unaligned placements) is audited in the same tables.

## 2. Source and asset inventory

| Path (all files in scope) | Execution context | State role | Disposition |
|---|---|---|---|
| `Source/PolitikaCore/Rules.h`, `Tests/Core/RulesTests.cpp` | Portable; every process | Typed orders, admission, lifecycle, intent decode, sequence guard, teams, vision | **Changed now.** Added team layouts, `AreAllied`, `CanAutoEngage`, `kUnalignedTeam`, `VisionSource`/`IsObservedBy`, `Entity::team/observed` and `TargetNotVisible`; tests extended. |
| `Source/Politika/Public/PolitikaPrototypeGameplay.h`, `Private/PolitikaPrototypeGameplay.cpp` | Authority simulates; client observes; HUD and controller are local | Coordinator, units, objective, controller, HUD, GameMode | **Changed now.** Findings F01–F19 and F26–F33. |
| `Source/Politika/Private/PolitikaMultiplayer.cpp` (new) | Server: binding, intake, vision, relevancy. Client: observer init. Development builds only: probes and sampling | Network boundary | **New.** Every network-specific path lives here. |
| `Source/Politika/Private/PolitikaInfantryCombat.cpp` | Authority | Reload, grenade, projectile damage | **Changed now** (F03, F08, F14). |
| `Source/Politika/Private/PolitikaInfantryPresentation.cpp`, `Public/PolitikaInfantryPresentation.h` | Any rendering process | Cosmetic animation, audio and weapon visuals | **No change.** Called only from authority paths, from multicast cosmetic RPCs or from OnRep; never mutates rules state (F14, F17). |
| `Source/Politika/Private/PolitikaInfantryBehavior.cpp`, `Public/PolitikaInfantryBehavior.h` | Data | Ability tuning | **No change.** Read-only definition, also read on clients for HUD ammo. |
| `Source/Politika/Private/PolitikaPrototypeContent.cpp`, `Public/PolitikaPrototypeContent.h` | Every process; editor | Catalog, loader, validators | **Changed now.** Added the `map.test.multiplayer_skirmish` development route, `SightRadiusCm` and `UnalignedPlacements` with structural and spatial validation (F34, F40). |
| `Source/Politika/Private/PolitikaGameInstance.cpp`, `Public/PolitikaGameInstance.h` | Process lifetime | Boot, travel, recovery | **Changed now** (F20, F26, F38). |
| `Source/Politika/Public|Private/PolitikaLocalPlayer.*` (new) | Client | Join URL options | **New** (F21). |
| `Source/Politika/Private/PolitikaShellPlayerController.cpp`, `PolitikaShellGameMode.*` | Local menu | Shell UI | **Changed minimally.** The shell-ready hook drives the development rematch only. The menu is already guarded for dedicated server and non-local controllers (line 26). |
| `Source/Politika/Private/PolitikaTestMapMusicComponent.cpp` | Rendering process | Music | **Moved** from GameMode to HUD (F15). It already returned early on a dedicated server. |
| `Source/Politika/Private/PolitikaLocalProfile.cpp`, `PolitikaUserSettings.cpp` | Local user | Settings and profile | **No change.** Never replicated; not used by gameplay authority. |
| `Source/Politika/Private/PolitikaNavigationAcceptance.cpp`, `PolitikaNavigationFailureAcceptance.cpp`, `Public/PolitikaNavigation*Fixture.h` | Development standalone harness | Acceptance instrumentation | **No change.** `BeginAcceptanceHarness` is skipped in network sessions (F31). |
| `Source/Politika/Private/Tests/*.cpp` (8 existing files) | Editor Automation | Regression | **No change needed.** All 27 pass against the changed coordinator. New file `PolitikaMultiplayerTests.cpp` adds 5 tests. |
| `Source/PolitikaBootstrap/*` | Process start | Logging, version, startup issues | **Changed.** Exposes `network_protocol` from `version.json`. It already skips UI-blocking paths for commandlets and dedicated servers (lines 95, 326). |
| `Source/PolitikaEditor/Private/PolitikaCreateShellCommandlet.cpp` | Editor only | Map and fixture authoring | **Changed.** New `-CreateMultiplayerSkirmishFixture`; it reads the canonical catalog and map without modifying them. |
| `Source/PolitikaEditor/Private/PolitikaImport*Commandlet.cpp`, `Tests/*Navigation*` | Editor only | Asset import and navigation tests | **No change.** Never runs in a game or server process. |
| `Source/Politika.Target.cs`, `PolitikaEditor.Target.cs`, `*.Build.cs` | Build | Targets and modules | `Politika.Build.cs` adds `EngineSettings` (recovery default map). **No Server target** (F32) at the audited commit; closed by MP-3 (`PolitikaServer.Target.cs`). |
| `Politika.uproject` | Build | Plugins | Enables `OnlineSubsystemUtils`, which provides only `IpNetDriver`. No online service is configured (F37). |
| `Config/DefaultEngine.ini` | Every process | Engine config | Adds `LocalPlayerClassName`, IpNetDriver timeouts and `RelevantTimeout=0.25` (F06, F21, F23). `DefaultGame.ini`, `DefaultInput.ini` and `DefaultGameUserSettings.ini` are unchanged (input is local). |
| `Content/Maps/*.umap`, `Content/Politika/{Definitions,Prototypes,Terrain,Audio}` (170 tracked assets) | Cooked into every process | Canonical content | **Unchanged** (git-clean after every fixture run). Unit placements resolve to stable archetype IDs; the replicated `ArchetypeId` lets clients load presentation locally. |
| `Content/Politika/TestFixtures/Multiplayer/*` (new, development only) | Development package | 4-seat + Unaligned fixture | **New.** Cooked only into Development packages; Shipping excludes it (package check). |
| `Tools/Shell/src/**`, `Tools/Infantry/*`, `Tools/InfantryAudio/*`, `Tools/TempestReference/*`, `tools/*` | Developer host | Orchestration and quality | **Added** `rts_multiplayer.rs` and `package-multiplayer.sh`. The existing runners are single-process, offline and unchanged. |

## 3. Path trace and mutation inventory

Path trace (every layout): mouse/keyboard on the local controller → `APolitikaPrototypePlayerController::SubmitIntent`. The host calls `APolitikaPrototypeMatch::SubmitControllerIntent` in-process; a remote client sends `ServerSubmitCommandIntent`, and its `_Implementation` reaches the same function. The intake then runs: connection binding → protocol check → `ClientSequenceGuard` → `DecodeIntent` → per-participant queue bound → entity ID resolution → `SubmitCommand`. At the next 0.05 s step, `AdmitPendingOrders` → `ValidateOrder` → `politika::rules::AdmitCommand`, using team- and fog-aware entities, applies the order. Simulation then runs movement → **team vision** → combat → grenades → damage → cleanup (dead or no-longer-observed targets) → objective → publication. `RouteCommandOutcome` returns each outcome only to the issuing controller. Replicated unit/objective/match state and the owner-only controller assignment drive client presentation through OnRep and multicast cosmetic RPCs.

| State | Authoritative writer | Guard / evidence |
|---|---|---|
| Unit position and navigation route | `APolitikaPrototypeUnit::SimulateMovement` (called only by the coordinator step) | `HasAuthority()` early return. AuthorityOnlySimulation test. Clients use ReplicatedMovement. |
| Health, pending damage, death | `ResolveCombat`, `AdvanceGrenades`, `ApplyPendingDamage`, coordinator cleanup | `HasAuthority()` guards. Packaged damage-consistency check. |
| Order, target, ammo, reload, sprint, grenade | Coordinator admission and unit simulation | Authority-only; replicated owner-only (`COND_OwnerOnly`). |
| Target acquisition | `AutoAcquireForAttackMove` | Authority. Requires `CanAutoEngage` (never the Unaligned) and team observation under fog (FogOfWar test). |
| Ownership, team, faction | `BuildScenario` (spawn), from authored slots and the session layout | Server only; replicated public. Never accepted from a client. |
| Participant binding | `BindController` / `HandleControllerLogout` | Server-only array, not replicated. ParticipantBinding test and packaged MP1-01/02/03. |
| Objective progress and capture team | `APolitikaPrototypeObjective::UpdateCapture` | `HasAuthority()`. Team rules. ResultCommitAndCleanup test. |
| Lifecycle and result | `TransitionLifecycle`, the objective result, `TerminateForParticipantDisconnect` | Server; mirrored by OnRep only. Committed exactly once (MP1-11 tests and packaged run). |
| Team vision | `UpdateTeamVisibility` | Server only. Drives relevancy and admission. |
| Selection, camera, HUD feedback | Local controller and HUD | Never replicated as authority. The camera pawn does not replicate movement. |

## 4. Exposure inventory (replication and RPC)

| Surface | Audience | Notes |
|---|---|---|
| Unit actor: `StableId, OwnerPlayer, TeamId, FactionId, ArchetypeId, UnitName, Health, MaxHealth, MoveSpeed, Weapon*, bAttackable, bBlastDeath`, transform | Only connections whose participant's team currently observes the unit (`IsNetRelevantFor`) | `RelevantTimeout=0.25 s` closes channels promptly. Generic replication is forced, because Iris would bypass this filter. |
| Unit `CurrentOrder, Ammo, ReloadRemaining, SprintRemaining` | The owning connection only | `COND_OwnerOnly`; the unit's net owner is the bound controller. |
| `MulticastShotPresentation`, `MulticastAbilityPresentation` (unreliable) | Connections with the shooter's channel open | **F27 (deferred MP-2):** the shot's target point can reveal an unseen third-party target in 3–4 seat games. |
| Objective `Progress, bContested, bComplete, CapturingPlayerId, CapturingTeamId` | Everyone | Control point state is public match information by design. |
| Match `ReplicatedMapId, MatchGeneration, ReplicatedPhase, result fields, ReplicatedParticipants{slot, player, faction, kind, team}, ReplicatedLayout, bReplicatedFogOfWar, Objective` | Everyone | **The unit list is not replicated**, so hidden enemy counts are not sent. |
| Controller `AssignedRuntimePlayerId, AssignedSlotId, AssignedFactionId, AssignedMatchGeneration` | Owner only | Labels only; the server never reads them back. |
| `ServerSubmitCommandIntent(FPolitikaNetCommandIntent)` | Client → server | No participant, owner or faction field. Bounded fields, versioned (`ProtocolVersion`). |
| `ClientReceiveCommandOutcome(FPolitikaNetCommandOutcome)` | Server → the issuing client | Correlation, sequence, issuer, kind and rejection only. A hidden or unknown target yields the same `TargetNotVisible` code. |

## 5. Findings register

| ID | Source identity | Assumption / defect | Disposition | Remediation (as implemented) | Validation |
|---|---|---|---|---|---|
| F01 | `APolitikaPrototypeMatch::LocalRuntimePlayerId`, `IsLocalControllerBound` | Exactly one local human; authority inferred from `GetFirstPlayerController()` | Change now | A server-only `FParticipantBinding` per connection. Standalone keeps an implicit binding for the first local controller. | ParticipantBinding test (all layouts and standalone); packaged MP1-01/02 |
| F02 | `SubmitLocalControllerCommand` → `SubmitCommand` | Local-only ingress; no transport | Change now | Single `SubmitControllerIntent`; the host calls it in-process, remote clients through a reliable server RPC. | CommandIntake test; packaged host/remote orders |
| F03 | Unit `SimulateMovement`, `ResolveCombat`, `ApplyPendingDamage`, `CleanupDeadTarget`, `AutoAcquire*`, `BeginReload`, `BeginGrenade`; `AdvanceGrenades`; objective `UpdateCapture`; match `Tick` | Would simulate on any copy | Change now | `HasAuthority()` guards at every entry point, not only `Tick`. | AuthorityOnlySimulation test |
| F04 | Units, objective, match | No replication | Change now | Minimal replicated state, OnRep presentation and archetype-ID presentation on clients. | Packaged client observations |
| F05 | `APolitikaPrototypeMatch::Units` | Replicating the list would leak hidden counts | Change now | Not replicated; the client registry follows unit `BeginPlay`/`EndPlay`. | FogOfWar test; packaged MP1-FOG-INITIAL |
| F06 | Unit relevancy | Always relevant (no fog) | Change now | Team vision each step; `IsNetRelevantFor`; `RelevantTimeout=0.25`; Generic replication forced. | FogOfWar test; packaged reveal |
| F07 | `ValidateOrder` / `AdmitOrder` | Clients could target unseen IDs; existence oracle | Change now | `Entity::observed`; `TargetNotVisible` for both hidden and unknown IDs. | Rules tests; FogOfWar test; packaged `hidden_target` probe |
| F08 | `AutoAcquireForAttackMove`, grenade victim, projectile splash | Owner-equality hostility; vision and Unaligned ignored | Change now | `CanAutoEngage`, `IsHostileTo`, team splash exemption, fog checks; under fog an unseen target is lost (`TargetLost`). | FogOfWar test |
| F09 | `APolitikaPrototypeObjective::UpdateCapture` | Only the local player captures | Change now | Per-side presence (team, or owner for team-less units); eligible teams; allies capture together. | ResultAndCleanup test; packaged MP1-11 |
| F10 | Result commit | Winner always the local player | Change now | Winner = capturing participant and team (`WinningTeamId`), replicated. | Same |
| F11 | HUD feedback from `LastCommandFeedback` | Server-wide string leaks other participants' commands | Change now | Per-controller correlated outcomes; the global string is used offline only. | Packaged outcomes per process |
| F12 | HUD subscribed to all gameplay events | Same leak | Change now | `RouteCommandOutcome` → owning controller only. | CommandIntake test (routes consumed) |
| F13 | Infantry `Acknowledge` on every order | Every machine heard every acknowledgement | Change now | `PlayLocalAcknowledgement` on the issuing participant's machine only. | Code review; packaged audio not separately measured (recorded as a limitation) |
| F14 | Infantry shot/ability presentation | Played only on the authority | Change now | Unreliable cosmetic multicast; ignored on the authority. | Packaged MP4 |
| F15 | `UPolitikaTestMapMusicComponent` on GameMode | A server-only object played music; clients were silent | Change now | Moved to the HUD (each rendering player). | Code review |
| F16 | Ground tint and sun created in `BuildScenario` | Only the authority lit the map | Change now | `ApplyMapPresentation` on every rendering process; skipped on a dedicated server. | Packaged client screenshots |
| F17 | `BeginDeathPresentation` | A client would `Destroy()` replicated actors | Change now | Authority destroys; observers hide until replicated destruction. | Packaged deaths |
| F18 | Camera pawn | Replicated movement; shared focus | Change now | No movement replication; networked seats focus on their own start. | Packaged screenshots |
| F19 | `ReturnToMenu` from a remote client | Would mutate the server's match | Change now | Only the authority ends its match; remote leave → `Logout`. | Packaged normal return |
| F20 | `UPolitikaGameInstance` | No network-failure handling | Change now | `HandleNetworkFailure`, kick notes, truthful reasons, MainMenu recovery default. | Packaged host loss / third client |
| F21 | Join URL | No protocol check | Change now | `UPolitikaLocalPlayer` login options plus a `PreLogin` protocol check. | Packaged `MP_PRELOGIN ACCEPTED protocol=1` |
| F22 | Admission capacity | No seat limit | Change now | `PreLogin` (earliest point) plus the `BindController` fallback; capacity = layout seats − host. | Tests (all layouts); packaged third connection |
| F23 | Remote disconnect | Undefined | Change now | `Logout` → unbind → `participant_disconnected`, committed once; the host returns to the menu. | Test; packaged client kill |
| F24 | Intake freshness | No duplicate/rate protection | Change now | `ClientSequenceGuard`, generation check, 16-command queue bound. | Tests; packaged probes |
| F25 | `SelectionByPlayer` | Keyed by player; local only | No change | Selection is never replicated; each process keeps its own map. | — |
| F26 | Replication system | Iris would ignore `IsNetRelevantFor` | Change now | `GetDesiredReplicationSystem` → Generic. | Test asserts `!IsUsingIrisReplication()` |
| F27 | Shot/grenade multicast target point | Can reveal an unseen target (any Unaligned engagement, any layout) | **MP-2 — CLOSED 29 Oct 2026** | Shot multicast removed; per-connection `ClientShotPresentation` with exact / facing-only / no cue by team vision (`ShotCueFor`). Grenade cues carry no position. | `ShotCueRedaction` test; [MP-2 record](../RTS_MULTIPLAYER_MP2_MULTICLIENT_ACCEPTANCE.md) |
| F28 | Grenade debug draws on the listen host (non-Shipping) | Draws hidden positions on the host | Later (development only) | Gate by host team vision when a host-fog playtest needs it. | — |
| F29 | Listen host process memory | The host holds all actors (inherent) | **MP-3** | Hidden enemies are hidden on the host via local component flags; a dedicated server removes the advantage. **MP-3: the dedicated server has no player process (listen remains inherent).** | Packaged host-hidden samples; MP-3 dedicated record |
| F30 | `StableId` | Small sequential integers (guessable) | No change | The no-oracle rejection makes guessing worthless. | Packaged ID sweep probe |
| F31 | Development acceptance harnesses | Standalone assumptions | No change | Skipped in network sessions. | — |
| F32 | Targets | No Server target | **MP-3 — CLOSED 29 Oct 2026** | `PolitikaServer.Target.cs` (TargetType.Server), packaged headless `LinuxServer` boot and 1v1 parity. | [MP-3 record](../RTS_MULTIPLAYER_DEDICATED_SERVER_ACCEPTANCE.md) |
| F33 | Client fog presentation | No fog overlay or last-seen ghosts | **MP-2 — CLOSED 29 Oct 2026** | Viewer-side visible/explored shading and last-known contacts from the viewer's own team vision and received contacts only; networked dead units retire after 1.5 s so a seen death is never remembered as a contact. | Core fog/contact tests; `FogMemoryAndShading` Automation; packaged `rts_multiplayer --fog-memory` and per-layout `FOG-MEMORY-NO-LEAK` ([record](../RTS_MULTIPLAYER_MP2_FOG_MEMORY_ACCEPTANCE.md)) |
| F34 | Unaligned side | Present only as an authority record; no authored units | Change now (schema/runtime) | `UnalignedPlacements`, owner 0, `kUnalignedTeam`, its own vision, never seated, neutral to auto-engagement. The canonical offline map is unchanged and needs an owner content decision. | Rules tests; FogOfWar test; packaged layout check |
| F35 | Mid-match disconnect policy | — | MP-2/MP-4 | MP-1 ends the match; AI takeover, grace and reconnect are deferred. | — |
| F36 | Movement bandwidth/interpolation | Not measured | **MP-2 — CLOSED 29 Oct 2026 (fixture scale)** | Engine per-connection counters and authoritative step cost measured at 2–4 seats, with and without lag/loss/reorder/duplicate emulation: about 1.3–1.8 KB/s down and 0.6–1.3 KB/s up per client, step p99 ≤ 0.48 ms of a 50 ms budget. Unit-scale budgets remain open. | [Network conditions record](../RTS_MULTIPLAYER_MP2_NETWORK_CONDITIONS_ACCEPTANCE.md) |
| F37 | `OnlineSubsystemUtils` | Could imply online services | No change | Only `IpNetDriver` is used; no subsystem is configured; offline boot contacts nothing (Shipping check). | Shipping boot |
| F38 | Recovery default map | Engine `?closed` browse could start a match | Change now | `OnStart` sets the runtime default map to MainMenu **after** the initial route. | Packaged returns |
| F39 | `RecoverToShellAfterAdmissionFailure` | Uses the first controller to travel | No change | On a listen server this is the host; clients follow through disconnect handling. | — |
| F40 | Content validator | Unaligned placements and sight radius unvalidated | Change now | Structural checks (faction, slot 0, bounds) and spatial NavMesh projection; `SightRadiusCm > 0`. | Fixture authoring (spatial PASS); Content tests |
| F41 | Seats and layouts | Slots are authored; no lobby | MP-4 | The server assigns seat order (host = authored human slot) and teams from `TeamForSeat`. Clients cannot choose. | — |
| F42 | 2v1 / 3v1 / 2v2 | Need 3–4 human processes, or AI for empty seats | **MP-2 — CLOSED (dedicated) 29 Oct 2026** | Packaged dedicated runs with 3–4 rendered clients per layout. Listen-hosted 3–4 process runs and AI-filled seats remain open. | [MP-2 record](../RTS_MULTIPLAYER_MP2_MULTICLIENT_ACCEPTANCE.md) |

## 6. Coverage statement

Every file in §2 was opened and its gameplay-relevant functions read; no file in scope was excluded. The three canonical `.umap` levels and the canonical catalog were not modified (verified with `git status` and SHA-256 before and after fixture authoring). Blueprint gameplay logic: none exists (C++-only project, `bHasBlueprintClasses=False` in the AssetManager scan). Re-run this audit when economy, structures, AI, formations or the Unaligned behavior arrive, as the architecture document requires.
