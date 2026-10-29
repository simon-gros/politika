# MP-3 — Dedicated server parity: acceptance record

**Date:** 29 October 2026 · **Scope:** the first MP-3 increment of [Multiplayer Architecture](RTS_MULTIPLAYER_ARCHITECTURE.md) §9. It adds a real headless Server target, dedicated-server seat binding and the dedicated session lifecycle on top of the [MP-1 foundation](RTS_MULTIPLAYER_FOUNDATION_ACCEPTANCE.md).

**Outcome: MP-3 dedicated server IMPLEMENTED; packaged qualification PASS for the stated scope.** One headless `PolitikaServer` process (Linux Development, `LinuxServer` cook) and independent packaged Linux Development clients ran over 127.0.0.1 in the 1v1 + Unaligned layout.

Not qualified here:
- 2v1, 3v1 and 2v2 on a dedicated server are **Automation-qualified only**; packaged 3–4-client runs are MP-2.
- A Shipping server build, internet/LAN hosting across machines, other platforms, scalability and release multiplayer.

## A. What works

- **Headless server.** `PolitikaServer` is a `TargetType.Server` build of the same `Politika`/`PolitikaBootstrap` modules and rules.
  - It boots straight into the match map. `ServerDefaultMap` is `/Game/Maps/RTSVerticalSlice`.
  - It runs with no display, no audio server and no desktop session variables, from an empty `HOME`/`-UserDir`, without `-nullrhi`.
  - It reports `local_players=0 can_render=0 can_render_audio=0`. The engine logs that the audio device manager is disabled.
  - It creates no player profile or save, emits no shell `ui_ready` event and never loads MainMenu.
- **All seats are remote.** A dedicated server has no local player.
  - Every seat, including seat 0, is bound to a remote connection **in join order**.
  - Participant IDs, slots, factions and teams are identical to a listen session of the same layout.
  - The match activates only when every seat is bound (`politika::rules::SessionSeatsComplete`).
  - A local controller is refused (`MP.BIND.NO_LOCAL_SEAT`).
- **Same rules and intake.** Seat-0 orders now arrive over the network through the same `SubmitControllerIntent` intake and `PolitikaCore` admission as every other remote order. No `remote=0` command exists on a dedicated server.
- **Same fog and authority.**
  - Each client initially receives only its own units.
  - Contact reveals enemy units.
  - All observed damage and deaths are server-originated.
  - The 24 hostile probes are rejected with the same codes as on a listen host.
- **Session lifecycle (dedicated only).**
  - After a result, clients may stay on the result screen. When the last client leaves, the server closes the session within 1 s. If clients linger, it closes after 30 s.
  - If a participant disconnects during play, the match ends exactly once (`participant_disconnected`). After 3 s the server removes the remaining clients with an explicit reason; the client shows `NetworkRemovedByHost`.
  - **Reset:** the server then reloads the same map, port and layout for a fresh match generation. The same process keeps hosting; it never travels to MainMenu and never exits on its own.
- **Fail closed.** If the requested layout or content cannot be hosted, the server logs `MP_DEDICATED_ADMISSION_FAILED` and exits with code 3. It does not linger in a menu route it does not have.
- **Listen and offline unchanged.** A listen host still occupies seat 0. The MP-1 packaged two-process run and the offline PAC-10 and participant-authority replays pass on the same final Development build.

## B. Source changes (branch `codex/mp3-dedicated-server-20261029`)

| Area | Files | Change |
|---|---|---|
| Server target | `Source/PolitikaServer.Target.cs` (new), `Config/DefaultEngine.ini` | `TargetType.Server`, same modules; `ServerDefaultMap`. |
| Portable rules | `Source/PolitikaCore/Rules.h`, `Tests/Core/RulesTests.cpp` | `HostingMode` (Standalone/Listen/Dedicated), `LocalSeatCount`, `RemoteSeatCount`, `SessionSeatsComplete`, `HostingModeName`, and core assertions for every layout. |
| Seats and lifecycle | `PolitikaMultiplayer.cpp`, `PolitikaPrototypeGameplay.{h,cpp}` | Covers: `GetHostingMode`; dedicated seat opening in `ResolveSessionLayout`; `GetMaxRemoteHumans` and activation through the core rule; `MP.BIND.NO_LOCAL_SEAT`. It adds session close scheduling (`results_linger_elapsed`, `results_complete`, `participant_disconnected`), close with kick, and same-URL `ServerTravel` reset. Also `MP_DEDICATED_SERVER_READY` and fail-closed admission exit 3. |
| Process | `PolitikaGameInstance.{h,cpp}` | No MainMenu default-map override on a dedicated server. `RearmDevelopmentTestFixture` lets each reset generation load the Development fixture again. |
| Tests | `Private/Tests/PolitikaMultiplayerTests.cpp` | New `Politika.RTS.Multiplayer.DedicatedHosting`. It covers per-layout listen/dedicated seat parity, all-remote binding, refusal of the local and extra joins, the seat-0 remote intake, seat reopening, disconnect close scheduling, results linger and empty reset, and listen non-interference. |
| Packaging and harness | `Tools/Shell/package-multiplayer.sh`, `Tools/Shell/src/bin/rts_multiplayer.rs` | The `server` packaging mode produces a `LinuxServer` archive. Its manifest must contain the fixture and RTSVerticalSlice and must not contain MainMenu. Archive paths are now made absolute. The `--dedicated` runner mode drives a headless server, rendered clients, the fail-closed check and listen parity. |

## C. MP-0 audit register

- **F32 (no Server target): CLOSED.** It is closed by the target, the package and the evidence below.
- **F29 (the listen host holds every actor):** remains inherent to listen hosting. A dedicated server removes it because there is no player process on the server.
- **Still open:**
  - F27 cosmetic shot-point redaction (MP-2).
  - F33 client fog shading (MP-2).
  - F35 disconnect policy beyond ending the match.
  - F36 bandwidth (MP-2).
  - F42 packaged 3–4-process layouts (MP-2).

## D. Verdicts

Vocabulary: PASS / FAIL / BLOCKED / INFRA_ERROR / INCONCLUSIVE / SKIPPED / NOT_RUN.

| ID | Requirement | Environment | Verdict | Evidence |
|---|---|---|---|---|
| CORE | Hosting rules for every layout (listen 1/2/3/3 remote, dedicated 2/3/4/4 remote, completeness) | `quality -- core`, C++20 `-Werror` | PASS | `build/diagnostics/core.json` |
| AUD | Docs index links | Rust `quality -- audit` | PASS | `build/diagnostics/audit.json` |
| BUILD | `PolitikaEditor` and `PolitikaServer` Linux Development | UBT, UE 5.8.3 | PASS | `logs/editor-build-final.log`, `logs/server-build-3.log` |
| AUTO-MP3 | `Politika.RTS.Multiplayer.DedicatedHosting` | Editor Automation, real listen `IpNetDriver` with the dedicated policy, `-nullrhi` | PASS | `final-automation-all/` |
| AUTO-ALL | Full `Politika` suite (32 existing + 1 new) | Same | PASS 33/33 (Success with environment warnings only) | `final-automation-all/index.json` |
| PKG-SERVER | `LinuxServer` archive; manifest has fixture + RTSVerticalSlice, no MainMenu; executable present | UAT BuildCookRun `-server -noclient` | PASS | `logs/package-final-server.log` |
| MP3-BOOT-HEADLESS | Ready on the port with net driver; `local_players=0 can_render=0 can_render_audio=0`; no display/audio/session env; no `-nullrhi` | Packaged server | PASS | `server.log`, `server_environment` fact |
| MP3-NO-PROFILE-UI-AUDIO | Empty HOME/UserDir boot; no profile/save file; no `profile`/`ui_ready`/MainMenu/audio-mixer markers; audio device manager disabled | Packaged server | PASS | evidence JSON |
| MP3-LAYOUT-1V1-UNALIGNED | `hosting=dedicated remote_seats=2`, 2 seats, fog, 2 Unaligned units | Packaged | PASS | `server.log` |
| MP3-01 / MP3-02 | First joiner → participant 1, team A; second → participant 2, team B | Packaged, 3 processes | PASS | server/client logs |
| MP3-ACTIVE-ALL-REMOTE | `MP_MATCH_ACTIVE hosting=dedicated local=0 remote=2` | Packaged | PASS | `server.log` |
| MP3-FOG-INITIAL / REVEAL | Each client receives only its own units; contact reveals participant 1 to client B | Packaged | PASS | `MP_CLIENT_UNIT_RELEVANT` |
| MP3-POS-CLIENT-B-ORDERS | Real X11 input Sprint/Stop/Guard/Move | Packaged | PASS | client B log + screenshots |
| MP3-POS-CLIENT-A-ORDERS | Seat-0 real input Stop/Guard/Move/Attack-move arrives `remote=1`; no `remote=0` command exists | Packaged | PASS | `server.log` |
| MP3-11 | Exactly one `objective_secured` result for team A; both clients observe it from their own team's view | Packaged | PASS | results screenshots |
| MP3-POS-DAMAGE-CONSISTENCY-A/B | Every client-observed damaged health/death has a server record (57 states, 1 death each) | Packaged | PASS | |
| MP3-NEG-PROBES | 24 hostile probes over the real RPC rejected with the listen-identical codes | Packaged | PASS | client B `MP_PROBE_SENT`/`MP_COMMAND_OUTCOME` |
| MP3-RESET-AFTER-RESULTS | Both clients leave → `results_complete remaining=0` → same-port reset; same server pid; generation 1 → 2 | Packaged | PASS | `server.log` |
| MP3-12-REMATCH | Both clients rejoin generation 2 with fresh bindings and assignments | Packaged | PASS | |
| MP3-03 | Third connection refused at `PreLogin` (`MP.SESSION.IN_PROGRESS`), no seat | Packaged (`-nullrhi` client) | PASS | `third.log` |
| MP3-G6-CLIENT-LOSS | Client A SIGKILL detected in 10.2 s. The match ended once, the remaining client was removed (`kicked=1`) and returned to MainMenu, and the server reset to generation 3 | Packaged | PASS | |
| MP3-SEAT-BY-JOIN-ORDER | In generation 3, the surviving client B binds seat 0 and a new client C binds seat 1 | Packaged | PASS | |
| MP3-G6-SERVER-LOSS | Server SIGKILL: both clients recover with `NetworkConnectionLost` in 10.2 s | Packaged | PASS | `08-server-lost-menu` screenshots |
| MP3-ADMISSION-FAIL-CLOSED | `?PolitikaLayout=5v5` → `MP.LAYOUT.INVALID`, `MP_DEDICATED_ADMISSION_FAILED`, exit 3, no session or units | Packaged server | PASS | `server-invalid-layout.log` |
| MP3-PARITY-SETUP | The listen host (same Development build, fixture, layout) and the dedicated server admit an identical setup. Compared lines: 4 player units with archetype, owner, faction, slot, location and weapon; objective; Unaligned side and positions; content validation; and roster/teams | Packaged | PASS (10/10 lines equal) | `parity-*-signature.txt` |
| MP1 regression | Full MP-1 two-process listen run on the final Development build | Packaged | PASS 17/17 | `final-listen-regression/final-evidence.json` |
| OFFLINE-PAC10 / OFFLINE-AUTHORITY | Rendered offline replays on the final Development build | Packaged Development, Xvfb | PASS | `logs/final-offline-{pac10,authority}.log` |
| 2v1 / 3v1 / 2v2 dedicated packaged | 3–4 clients | — | NOT_RUN (MP-2) | Automation covers binding/teams |
| Shipping server | Shipping `PolitikaServer` | — | NOT_RUN | Development server only |

**Parity scope.** "Dedicated vs listen equivalence" here means three things:
- An identical authoritative setup for the same fixture.
- The same command admission and probe rejection codes.
- The same result rule: objective capture, team A wins, committed once.

A frame-by-frame simulation comparison under identical input timing is **not** claimed. The two runs used human-timed X11 input on different processes.

## E. Package and evidence identity

Both packages were built from commit `644abae` with `git_dirty=false`. The documentation commit that follows does not change either package.

- **Engine:** UE 5.8.3 `396c9f059903aed5fec78ecd3d437a40c6415368`, Linux, loopback 127.0.0.1 (ports 17820/17822/17824; listen regression 17830).
- **Archive:** `Saved/DedicatedServer20261029/Final-644abae/{Server/LinuxServer,Development/Linux}` in the branch worktree. It is immutable and is not the stable player default.
- **Superseded attempts** (not evidence):
  - `Final-01ce1ad`: its relative archive path landed in the engine directory; recoverably trashed.
  - `Final-7cc5741`: wrong manifest name for LinuxServer; trashed.
  - `Final-eff3650`: dry runs only; its first dry run exposed relative log paths in the runner.

| Artifact | SHA-256 |
|---|---|
| `PolitikaServer` | `d49586c8dc9e50a9d4051e1ac088492693543c9c6053c91ed8dd578920af8637` |
| `Politika-LinuxServer.utoc` | `b33412c26f8f60f1557601932328dc8647c7b249d84b8e9a6a83b0f9a8d4c289` |
| `Politika-LinuxServer.ucas` | `d470287185fbc019849d8a2ca097011d7930b627c40dca717432719953e6a59e` |
| Development `Politika` | `da3edfb47a4173f188117ff144462bb6e0f3f81db211f0f190459e0c98406363` |
| Development `Politika-Linux.utoc` | `405745e3ff47097faaa92e03d1802808435d7a52ea303dc69d20af6a5b5f9a30` |
| Development `Politika-Linux.ucas` | `cc5dbc9a4b71f179d9d080c32ef9513bdb98370711ed4317a73bc13b937d865e` |
| `Content/Maps/RTSVerticalSlice.umap` (unchanged from MP-1) | `8e8b8bba057f076cdeb719488e2e846d70626a55fdb60dfb1c5ecc1d41ba8afc` |
| `DA_MultiplayerSkirmish.uasset` (unchanged) | `20e9d37de88f964265d37cd6a20a663c7423410690eb66e2328357a34b1e752f` |
| `DA_MultiplayerCatalog.uasset` (unchanged) | `42f3453197a4ebbf305a6bfd7b5afb1f1dde2b010e9b75fc6e3960daca3d4568` |

- **Dedicated run** (`rts_multiplayer --dedicated`, 22/22 PASS): server pid 966738; clients A 966777, B 966942 and C 967598; refused third client 967484.
  - Evidence is in `build/diagnostics/mp3-dedicated-server/final-dedicated/`: `dedicated-evidence.json`, all logs, `client-a.mp4`/`client-b.mp4`, and screenshots of the active match, orders, contact, results, menu, rematch, client loss and server loss.
  - A durable copy of `build/diagnostics/mp3-dedicated-server/` is at `/mnt/data/politika-qualification/MP3-DedicatedServer-20261029/`.
- **Commands:**

```bash
export UNREAL_ENGINE_DIR=/mnt/data/unreal/5.8.3
bash Tools/Shell/package-multiplayer.sh server      Saved/DedicatedServer20261029/Final-644abae/Server
bash Tools/Shell/package-multiplayer.sh development Saved/DedicatedServer20261029/Final-644abae/Development
A=Saved/DedicatedServer20261029/Final-644abae
cargo run --locked --manifest-path Tools/Shell/Cargo.toml --bin rts_multiplayer -- --dedicated \
  $A/Server/LinuxServer/Politika/Binaries/Linux/PolitikaServer \
  $A/Development/Linux/Politika/Binaries/Linux/Politika --evidence <DIR> --port 17820
# Host the canonical prototype map (no fixture); clients join with: Politika 127.0.0.1:7777
$A/Server/LinuxServer/PolitikaServer.sh "/Game/Maps/RTSVerticalSlice?PolitikaLayout=1v1" -port=7777
```

The canonical-map launch above was checked as a **boot-only smoke**. It reached `MP_MATCH_AWAITING_PARTICIPANTS map=map.prototype.rts_vertical_slice ... hosting=dedicated` and `MP_DEDICATED_SERVER_READY` (`smoke-nofixture/server.log`). Clients were not run against it, so the packaged gameplay verdicts above apply to the multiplayer fixture only.

- **Offline replays** need an X11-only environment (`env -u WAYLAND_DISPLAY SDL_VIDEODRIVER=x11 xvfb-run …`). Otherwise SDL prefers the operator's Wayland session, and the runner reports `no visible window`. The first attempt in this run failed that way and was re-run; the failed attempt is not counted.

## F. Limitations and next work

- **Evidence limits.**
  - Loopback on one Linux host.
  - Xvfb with virtual input.
  - A Development server only.
  - One 1v1 packaged layout.
  - The reset policy timings (1 s / 3 s / 30 s) are prototype values, not tuned UX.
- **Not implemented.**
  - AI-filled seats.
  - Reconnect, late join and spectators.
  - Lobby and session browser.
  - Server administration (console, ban/kick commands, config file).
  - A server-side match-rotation choice other than "reload the same map".
- **Resolved (29 October 2026).** In the run above, Development-only switches with values (`-PolitikaMPRematch=…`) caused an `InvalidLaunchOptions` recovery notice on clients (MP-1 behavior). It did not affect gameplay evidence. Section G covers the fix.
- **Next.**
  - MP-2: packaged 3–4-client runs on the dedicated server for 2v1/3v1/2v2, network emulation, bandwidth and server-step measurements, F27/F33.
  - A Shipping `PolitikaServer` package with a Shipping-safe default fixture policy.

## G. Follow-up: Development-only multiplayer launch switches (29 October 2026)

Branch `codex/mp-dev-launch-switches-20261029`; source commit `4d80a76`.

- **Change.** `FPolitikaLaunchOptions::Parse` (`Source/PolitikaBootstrap/Private/PolitikaBootstrap.cpp`) now takes an `EPolitikaLaunchSwitchPolicy`.
  - **Development policy** (default in non-Shipping builds): recognizes `-PolitikaMPRematch=<url>`, `-PolitikaMPRematchDelay=<s>`, `-PolitikaMPRematchAttempts=<n>`, `-PolitikaMPProbe` and the existing `-PolitikaTestFixture=<id>`. These switches no longer raise `InvalidLaunchOptions`.
  - **Strict policy:** the switch table is compiled out under `UE_BUILD_SHIPPING`, so Shipping is always Strict.
  - **Still rejected under both policies:**
    - unknown switches;
    - flags given a value (`-PolitikaMPProbe=1`);
    - valued switches with no value (`-PolitikaMPRematch`, `-PolitikaMPRematch=`). This includes the previously tolerated empty `-PolitikaTestFixture=`.
    - A rejected switch still cancels `-PolitikaResetUserConfig`.
- **Test.** `Politika.Bootstrap.LaunchOptions` (`Source/PolitikaBootstrap/Private/Tests/PolitikaLaunchOptionsTests.cpp`) checks the following:
  - Each Development switch, and the MP-3 client command line, is accepted under Development and rejected under Strict.
  - The rejection list fails under both policies.
  - Safe mode and reset behavior is unchanged.
  - The build's default policy is correct.
- **Harness.** `rts_multiplayer` adds `MP1-DEV-LAUNCH-OPTIONS` (listen) and `MP3-DEV-LAUNCH-OPTIONS` (dedicated). Each fails if any of these logs contains `"code":"InvalidLaunchOptions"`, or is missing the bootstrap event: listen host/client; dedicated server and clients A/B.

| Gate | Result |
|---|---|
| `UNREAL_ENGINE_DIR=/mnt/data/unreal/5.8.3 bash tools/unreal.sh build` (PolitikaEditor) | Succeeded, exit 0 |
| `bash tools/unreal.sh core-test` | PASS (`core.compile`, `core.command_rules`) |
| `bash tools/unreal.sh test` (Politika Automation) | 34/34 Success, 0 failed (wrapper exit 4 = INCONCLUSIVE by design; report inspected). All warnings are pre-existing log warnings (Wayland/SDL init query, crowd manager, navigation fixtures). A first run failed `Politika.Bootstrap.LaunchOptions` on a typo in the new test (`-politikamprobe`); after the test was corrected and rebuilt, the second run passed. |
| `package-multiplayer.sh development` / `server` / `shipping` | All `ExitCode=0`; manifest checks passed |
| `rts_multiplayer` listen, port 17840 (host 1008361, client 1008465) | 18/18 PASS (MP-1 baseline 17 + `MP1-DEV-LAUNCH-OPTIONS`) |
| `rts_multiplayer --dedicated`, port 17850 (server 1012272, clients A 1012293 / B 1012430 / C 1013576) | 23/23 PASS (MP-3 baseline 22 + `MP3-DEV-LAUNCH-OPTIONS`) |
| `rts_multiplayer --shipping-boot` | 3/3 PASS; `SHIP-NO-DEV-ENTRY-POINTS` found none of the switch names in the Shipping binary |
| Shipping spot-check (manual, Xvfb) with all four MP switches | MainMenu still shows `[InvalidLaunchOptions]` (`shipping-strict/shipping-dev-switches-main-menu.png`) |

- **Visual result.** The new `dedicated/client-b-07-client-a-lost-menu.png` shows only `[NetworkRemovedByHost]`; the MP-3 run's screenshot also showed `[InvalidLaunchOptions]`. No listen or dedicated log contains `InvalidLaunchOptions`.

**Package identity** (`BuildIdentity.json`: `git_revision` `4d80a760134d`, `git_dirty` false, game `0.1.0-dev.0`, network_protocol 1).

| Artifact | SHA-256 |
|---|---|
| Development `Politika` | `effe73ab826c46b75e301ec4aea40121045f0439f273dc52c2c3c37ca1668da0` |
| `PolitikaServer` | `73db148b13abfb896e72c5164802e289084bddfd3ae4e3e59f7a86d47000e79b` |
| `Politika-Linux-Shipping` | `6cd48aed6e22e81496e1970c7a194026e8fd46b6c39a8859ec3f7323d3b0332b` |
| Development `Politika-Linux.utoc` / `Politika-LinuxServer.utoc` | unchanged from section E (content did not change) |

- **Evidence location.** Working copy: `build/diagnostics/mp-dev-launch-switches/` (build, test, package and run logs; `listen/`, `dedicated/`, `shipping/`, `shipping-strict/`). Durable copy: `/mnt/data/politika-qualification/MP-DevLaunchSwitches-20261029/`.
- **Limits.**
  - Same as section F.
  - No Shipping log exists, because Shipping does not log. Shipping strictness rests on three things: the compiled-out table, the Strict-policy Automation cases and the screenshot.
  - No version counters changed.
