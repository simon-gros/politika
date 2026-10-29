# MP-2 — Packaged 3–4 client layouts and fog-safe shot cues: acceptance record

**Date:** 29 October 2026 · **Scope:** the first MP-2 increment of [Multiplayer Architecture](RTS_MULTIPLAYER_ARCHITECTURE.md) §9. It closes MP-0 audit findings **F42** (packaged 2v1/3v1/2v2) and **F27** (shot cue target points), on top of the [MP-1 foundation](RTS_MULTIPLAYER_FOUNDATION_ACCEPTANCE.md) and the [MP-3 dedicated server](RTS_MULTIPLAYER_DEDICATED_SERVER_ACCEPTANCE.md).

**Outcome: IMPLEMENTED; packaged qualification PASS for the stated scope.** One headless `PolitikaServer` (Linux Development, `LinuxServer` cook) hosted 2v1, 3v1 and 2v2 + Unaligned matches for three or four independent rendered packaged Linux Development clients over 127.0.0.1. Every packaged check passed: 57/57 in the layout matrix, plus the protocol-refusal check.

Not qualified here:
- Network emulation (latency, loss, reordering), bandwidth and server-step measurements (F36). These are the next MP-2 increment.
- **The packaged runs exercised only exact shot cues.** In player-versus-player combat the target's team always sees the shooter, because weapon range (650 cm) is below sight (1200 cm). Facing-only and withheld cues only arise around the Unaligned side, and the fixture choreography does not fight it. Those two paths are **Automation-qualified only**.
- Listen-server 3–4 process layouts, client fog shading/last-seen contacts (F33), AI-filled seats, reconnect, a Shipping server, other machines/platforms.

## A. What changed

- **Shot cues follow team vision (F27).** The unreliable `MulticastShotPresentation(Target)` is gone. A multicast reaches every connection that has the shooter's channel open, so it carried an unseen target's exact position to the opposing team. For example, team B saw where team A was shooting an Unaligned unit that only team A could see.
  - The server now decides per connection (`APolitikaPrototypeMatch::DistributeShotPresentation`, core rule `politika::rules::ShotCueFor`):
    - **Exact.** The viewer's team observes the target: it receives the target point.
    - **Facing only.** The viewer's team sees the shooter but not the target. It receives a point 600 cm along the shooter's facing. Facing is already public through the replicated rotation, and the fixed distance carries nothing about the target.
    - **None.** The viewer's team cannot see the shooter: no cue is sent.
  - Delivery is `APolitikaPrototypePlayerController::ClientShotPresentation`: unreliable, owner-connection only, sent once per bound participant.
  - The listen host's own presentation uses the same rule for its seat. It no longer draws or *hears* shots from shooters its team cannot see.
  - A dedicated server keeps the exact point for its own log only.
  - Offline play is unchanged: full presentation, nothing sent.
  - Ability cues (reload, grenade wind-up and launch) carry no position and stay multicast.
- **Allied hostile probes.** The Development hostile probe now separates hostile units from an ally's units. When an ally exists, it also sends `allied_issuer` (commanding an ally's unit: `NotOwned`) and `allied_target` (attacking an ally's unit: `FriendlyTarget`).
- **Network protocol 1 → 2.** The RPC set changed incompatibly, so `version.json` `network_protocol` is 2. The existing `PreLogin` check refuses old clients before any controller exists. The refusal was verified against the immutable MP-3 protocol-1 client (§D).
- **Harness.** `rts_multiplayer --dedicated-layouts` drives one headless server and one rendered client per seat. Each client has its own Xvfb display, user directory, log and MP4. The existing `--dedicated` and listen modes now read the protocol from `version.json`. Their probe attribution check also accepts any bound remote participant (in MP-3, participant 1 is remote too).

## B. Source changes (branch `codex/mp2-multiclient-shot-redaction-20261029`, commit `c14a6c4`)

| Area | Files | Change |
|---|---|---|
| Portable rules | `Source/PolitikaCore/Rules.h`, `Tests/Core/RulesTests.cpp` | `ShotCue`, `ShotCueFor`, `ShotCueName` and static assertions for every combination. |
| Shot cues | `PolitikaPrototypeGameplay.{h,cpp}`, `PolitikaMultiplayer.cpp` | Covers: `ResolveShotCue`, `DistributeShotPresentation`, `ClientShotPresentation`, `PresentObservedShot`; the multicast is removed. Development logs `MP_SHOT_CUE_SENT` (server, per participant) and `MP_CLIENT_SHOT_CUE` (client). |
| Probes | `PolitikaMultiplayer.cpp` | Hostile/ally separation, `allied_issuer`, `allied_target`, `MP_PROBE_BEGIN ally=`. |
| Tests | `Private/Tests/PolitikaMultiplayerTests.cpp` | New `Politika.RTS.Multiplayer.ShotCueRedaction`. It covers a 2v2 listen world (exact / facing-only / none per seat, the host view, independence of the hidden target's distance, the Unaligned unit as shooter and target), a dedicated 1v1 world (server record exact, seats fog-filtered) and offline. |
| Compatibility | `version.json` | `network_protocol` 2. |
| Harness | `Tools/Shell/src/bin/rts_multiplayer.rs` | `--dedicated-layouts`, shot-cue consistency, per-client damage records, the legacy-protocol refusal session, `network_protocol()`. |

## C. MP-0 audit register

- **F42 (packaged 2v1 / 3v1 / 2v2): CLOSED for dedicated hosting.** Listen-hosted 3–4 process runs were not run; Automation still covers listen binding for every layout.
- **F27 (shot target point): CLOSED.** Code and Automation cover every cue kind. Packaged runs prove per-connection delivery and exact-cue integrity. See the limitation above.
- **F28** (grenade debug draws on a listen host, non-Shipping): unchanged, later.
- **Still open for MP-2:** F33 (client fog shading), F36 (bandwidth / server step), F35 (disconnect policy beyond ending the match).

## D. Verdicts

Vocabulary: PASS / FAIL / BLOCKED / INFRA_ERROR / INCONCLUSIVE / SKIPPED / NOT_RUN. `<L>` is `2V1`, `3V1` or `2V2`; every row below passed for **all three** layouts unless stated.

| ID | Requirement | Environment | Verdict | Evidence |
|---|---|---|---|---|
| CORE | `ShotCueFor` and the existing rules | `quality -- core`, C++20 `-Werror` | PASS | `build/diagnostics/core.json` |
| BUILD | `PolitikaEditor` Linux Development (`PolitikaServer` built by UAT) | UBT, UE 5.8.3 | PASS | `logs/editor-build-2.log` (first attempt failed: private member access, fixed) |
| AUTO-SHOTCUE | `Politika.RTS.Multiplayer.ShotCueRedaction` | Editor Automation, real listen `IpNetDriver`, `-nullrhi` | PASS | `automation-all/` |
| AUTO-ALL | Full `Politika` suite (33 existing + 1 new) | Same | PASS 34/34 (each with the one environment warning `Querying IsUsingWayland before SDL is initialized`) | `automation-all/index.json` |
| PKG | `LinuxServer` + Development archives; manifests checked by the package script | UAT BuildCookRun | PASS | `logs/package-{server,development}.log` |
| MP2-`<L>`-LAYOUT | `hosting=dedicated`, seats = remote seats = 3/4/4, fog, headless `local_players=0 can_render=0`, protocol 2 | Packaged server | PASS ×3 | `server.log` |
| MP2-`<L>`-SEATS-BY-JOIN-ORDER | Clients joining one at a time bind participants 1..N with teams `[A,B,A]`, `[A,B,A,A]`, `[A,B,A,B]` | 4–5 processes | PASS ×3 | server/client logs |
| MP2-`<L>`-ACTIVE-ALL-REMOTE | `MP_MATCH_ACTIVE local=0 remote=N`; N + 1 distinct processes | Packaged | PASS ×3 | |
| MP2-`<L>`-FOG-INITIAL | Each client receives exactly the units whose server activation sample marks its team as observing | Packaged | PASS ×3 | `MP_SERVER_SAMPLE reason=activated` vs `MP_CLIENT_UNIT_RELEVANT` |
| MP2-`<L>`-FOG-ALLIED-SHARED | Every client receives all allied participants' units | Packaged | PASS ×3 | |
| MP2-`<L>`-ORDERS-EVERY-CLIENT | Every client's real X11 Stop + Guard admitted as `remote=1` for its participant; no `remote=0` command | Packaged | PASS ×3 (6 / 8 / 8 orders) | |
| MP2-`<L>`-ADVANCE-ORDERS | Real input: participant 1 Move onto the objective, other team-A members Attack-move, team B Move part-way | Packaged | PASS ×3 | |
| MP2-`<L>`-FOG-REVEAL | Every team-B client receives a team-A unit it did not see at start | Packaged | PASS ×3 | |
| MP2-`<L>`-RESULT-ONCE-ALL-VIEWERS | One `objective_secured` commit; every client observes the same winner (team A each time) from its own `viewer_team` | Packaged | PASS ×3 | `04-results` screenshots (VICTORY / DEFEAT per team) |
| MP2-`<L>`-DAMAGE-CLIENT-n | Every damaged health and death a client saw has a server record | Packaged, each client | PASS (2v1 3×57 states / 1 death; 3v1 4×59 / 2; 2v2 4×57 / 1) | |
| MP2-`<L>`-PROBES-CLIENT-2/3 | Hostile probes from a team-B and a team-A client over the real RPC, attributed only to bound participants | Packaged | PASS ×6 (24/24, 23/21, 24/24 probes) | |
| MP2-`<L>`-ALLIED-PROBES-CLIENT-2/3 | `allied_issuer` → `NotOwned`, `allied_target` → `FriendlyTarget`, sent exactly when the team has another member | Packaged | PASS ×6 | `MP_PROBE_BEGIN ally=` |
| MP2-`<L>`-SHOT-CUES-FOG-SAFE | Every client cue matches a server decision for that participant (same kind and point within NetQuantize rounding); exact cues carry the target; nothing withheld was delivered; no multicast | Packaged | PASS ×3 (exact 171 / 236 / 228; facing 0; none 0) | `MP_SHOT_CUE_SENT` / `MP_CLIENT_SHOT_CUE` |
| MP2-`<L>`-RESET-AFTER-RESULTS | All clients leave → `results_complete remaining=0` → same server process resets to generation 2 with the same layout | Packaged | PASS ×3 | |
| MP2-PROTOCOL-LEGACY-REFUSED | MP-3 protocol-1 client → `MP_PRELOGIN REJECTED protocol=1 reason=MP.PROTOCOL.MISMATCH`; client shows the mismatch and returns to MainMenu; no seat bound | Packaged server + immutable MP-3 client `da3edfb4…` | PASS | `legacy-protocol/` |
| Facing-only / withheld cues over sockets | Packaged | — | NOT_RUN | Needs an Unaligned engagement choreography; Automation covers both |
| MP3 regression | `rts_multiplayer --dedicated` 1v1 on this build | Packaged | PASS 22/22 | `regression-dedicated-1v1/` |
| MP1 regression | Two-process listen 1v1 on this build | Packaged | PASS 17/17 (first attempt BLOCKED: no free Xvfb display, see below) | `regression-listen-1v1/` |
| OFFLINE-AUTHORITY | Rendered participant-authority replay | Packaged Development, Xvfb | PASS | `logs/offline-authority.log` |
| OFFLINE-PAC10 | Rendered offline interaction replay | Same | PASS on re-run; **first attempt FAIL** (`camera input reached the controller but its clamped position did not change … axis=right x=-0.0 y=0.0`) | `logs/offline-pac10{,-attempt1-fail}.log` |
| Listen 3–4 process | Listen host + 2–3 clients | — | NOT_RUN | Dedicated only in this increment |

**Harness defect found and fixed — Xvfb display exhaustion.** The first listen regression was BLOCKED because every display 90–159 had a stale `/tmp/.X<n>-lock` while no Xvfb was running. `Display::drop` SIGKILLed Xvfb, so Xvfb never removed its lock; MP-1, MP-3 and this matrix had accumulated 71 of them. The 71 stale locks (all owned by dead PIDs) were removed. The runner now sends SIGTERM, waits up to 5 s, and removes the lock and socket itself if it must SIGKILL. After the re-run only the operator's two real displays held locks. This fix is in the harness commit after `c14a6c4`; the matrix and dedicated regression ran with the earlier teardown, which only affects cleanup.

**Flaky offline check, not caused by this slice.** PAC-10's first attempt failed its camera-pan assertion, and the immediate re-run on the same binary passed. This slice changes no camera, input or content code, and the cooked content containers hash identical to MP-3. The intermittent camera assertion is recorded for the camera/acceptance owner.

**Finding — the 3v1 starts are not fully fogged.** The fixture comment says every side starts fogged, but in 3v1 seat 2 (team B, first unit at 1350, 600) and seat 4 (team A, first unit at 1350, −600) are exactly 1200 cm apart. That equals sight radius, and the vision test is inclusive. So entities 3 and 7 are mutually observed from activation. The runner derives the expected initial view from the server's own vision masks, so the packaged check is exact either way. The fixture is unchanged here, since moving it would change MP-1/MP-3 setup parity baselines. It is recorded for the fixture owner.

## E. Package and evidence identity

Both packages were built from commit `c14a6c4` with `git_dirty=false` and `network_protocol=2` (`BuildIdentity.json`). Documentation commits that follow do not change either package.

- **Engine:** UE 5.8.3 `396c9f059903aed5fec78ecd3d437a40c6415368`, Linux, loopback 127.0.0.1 (ports 17860/17862/17864, legacy check 17866; regressions 17880/17890).
- **Archive:** `Saved/MP2Multiclient20261029/Final-c14a6c4/{Server/LinuxServer,Development/Linux}` in the branch worktree. It is immutable and is not the stable player default.

| Artifact | SHA-256 |
|---|---|
| `PolitikaServer` | `251d87fd69d3cf6838c7dc32acc5d939225f875159487aac45ba54829d14386e` |
| `Politika-LinuxServer.utoc` (unchanged from MP-3) | `b33412c26f8f60f1557601932328dc8647c7b249d84b8e9a6a83b0f9a8d4c289` |
| `Politika-LinuxServer.ucas` (unchanged from MP-3) | `d470287185fbc019849d8a2ca097011d7930b627c40dca717432719953e6a59e` |
| Development `Politika` | `a39ea2cd0bd75728d378a8d327784b0cb170fbae78d40ae24fd059d0e52209ed` |
| Development `Politika-Linux.utoc` (unchanged from MP-3) | `405745e3ff47097faaa92e03d1802808435d7a52ea303dc69d20af6a5b5f9a30` |
| Development `Politika-Linux.ucas` (unchanged from MP-3) | `cc5dbc9a4b71f179d9d080c32ef9513bdb98370711ed4317a73bc13b937d865e` |
| `RTSVerticalSlice.umap`, `DA_MultiplayerSkirmish`, `DA_MultiplayerCatalog` | unchanged (`8e8b8bba…`, `20e9d37d…`, `42f34531…`) |

- **Matrix run** (`run-1/`, 57/57 PASS):
  - 2v1: server 1032065; clients 1032112, 1032222, 1032331.
  - 3v1: server 1033151; four clients 1033167–1033733.
  - 2v2: server 1034868; four clients 1034884–1035219.
  - Per layout: `layout-<L>/` holds server/client logs, five screenshots per client (active, orders, contact, results, menu) and one MP4 per client. `multiclient-evidence.json` holds all verdicts, commands and hashes.
- Evidence root: `build/diagnostics/mp2-multiclient/` in the branch worktree. A durable copy is at `/mnt/data/politika-qualification/MP2-Multiclient-20261029/`.
- **Commands:**

```bash
export UNREAL_ENGINE_DIR=/mnt/data/unreal/5.8.3
A=Saved/MP2Multiclient20261029/Final-c14a6c4
bash Tools/Shell/package-multiplayer.sh server      $A/Server
bash Tools/Shell/package-multiplayer.sh development $A/Development
cargo run --locked --manifest-path Tools/Shell/Cargo.toml --bin rts_multiplayer -- --dedicated-layouts \
  $A/Server/LinuxServer/Politika/Binaries/Linux/PolitikaServer \
  $A/Development/Linux/Politika/Binaries/Linux/Politika \
  --legacy-client <MP-3 Final-644abae Development Politika> --evidence <DIR> --port 17860
```

## F. Limitations and next work

- **Evidence limits.**
  - Loopback on one Linux host.
  - Xvfb with virtual input.
  - A Development server only.
  - Human-timed input, so no frame-exact comparison between layouts.
  - Team A won every run: the choreography favors it, and a team-B result in a 3–4 client layout was not exercised.
- **Next MP-2 increments.**
  - Network emulation with latency, loss and reordering (`PktLag`/`PktLoss`/`PktOrder`), with duplicate and reorder intake checks under real conditions.
  - Per-client bandwidth and server step-time measurements across 2–4 seats (F36).
  - An Unaligned engagement fixture to observe facing/withheld cues over sockets.
  - Client fog shading and last-seen contacts (F33).
  - Listen-hosted 3–4 process runs.
