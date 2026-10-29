# MP-2 — Network emulation, bandwidth and server step cost: acceptance record

**Date:** 29 October 2026 · **Scope:** the second MP-2 increment of [Multiplayer Architecture](RTS_MULTIPLAYER_ARCHITECTURE.md) §9, on top of the [first MP-2 increment](RTS_MULTIPLAYER_MP2_MULTICLIENT_ACCEPTANCE.md). It closes MP-0 audit finding **F36** (movement bandwidth and server step cost were never measured) and qualifies the dedicated layouts under engine-emulated latency, loss, reordering and duplication.

**Outcome: IMPLEMENTED; packaged qualification PASS for the stated scope.**
- One headless `PolitikaServer` hosted 1v1, 2v1, 3v1 and 2v2 + Unaligned matches for two to four rendered packaged clients over 127.0.0.1.
- Every layout passed with no emulation. 1v1 and 2v2 also passed under three engine emulation profiles applied to every process: lag + loss, reorder + loss, and duplicate + loss.
- Every client command (real input and hostile probes, 31–60 per run) reached the server exactly once, in send order, under every profile. No honest command was rejected as a duplicate or for rate.
- Bandwidth per client was about 1.3–1.8 KB/s down and 0.6–1.3 KB/s up, and it did not grow with seat count from two to four seats.
- Authoritative step cost: p99 at most 0.48 ms and maximum at most 1.93 ms, against a 50 ms step budget. No catch-up cap was hit.
- No gameplay or RPC change: `network_protocol` stays **2**. The new code is Development-only instrumentation.

Not qualified here:
- **Shipping-build bandwidth.** The measurements are from Development builds, which also carry Development-only RPC traffic (hostile probes) and log sampling. Unit replication itself is identical in Shipping, but no Shipping multiplayer process was measured.
- **Real networks.** Loopback on one host with engine emulation is not a WAN, Wi-Fi or NAT path. UDP/IP headers are outside the engine's byte counters.
- **Scale.** The fixture has 5–9 units. Bandwidth and step cost at RTS unit counts (hundreds of units) are not measured, so these numbers do not set a budget for them.
- Listen-hosted 3–4 process runs, client fog shading (F33), the Unaligned engagement fixture (facing-only and withheld shot cues over sockets), a Shipping server.

## A. What changed

- **Measurements (Development builds only, `!UE_BUILD_SHIPPING`).**
  - `MP_NET_STATS`: the engine's own per-connection counters (`UNetConnection` total bytes, packets, lost packets, out-of-order recovered/lost/duplicate, average RTT, jitter) plus the number of units the participant's team can see.
    - Server: one line per bound remote connection at match activation (the baseline), then every second, then at the result.
    - Client: its server connection, every second.
  - `MP_SERVER_STEP_SUMMARY`: written once when the result commits (or on disconnect/end of play). It gives nearest-rank p50/p95/p99/max of the authoritative step cost (`AdmitPendingOrders` + `RunSimulationStep`) and of the server frame time, the number of frames that hit the five-step catch-up cap, and the budget (one 50 ms simulation step).
  - `MP_NET_EMULATION`: the packet simulation settings each process's net driver actually applied (`FPacketSimulationSettings`), so a run proves which conditions it was under.
  - Only the active match is measured; finished matches add no samples.
- **Portable helper.** `Source/PolitikaCore/Metrics.h`: `politika::metrics::NearestRank` and `RatePerSecond`, with compile-time checks in `Tests/Core/RulesTests.cpp`.
- **Harness.** `rts_multiplayer --dedicated-layouts` gained:
  - `1v1` as a layout. Participant 2 probes; the allied-vision check is NOT_RUN because 1v1 has no allies. Team A's second unit attack-moves in support of the capture (§D finding).
  - `--net-profile none|lag-loss|reorder|duplicate`. The profile's engine switches go to the server and every client, so both directions of each connection are impaired.
  - Five checks per layout: `NET-EMULATION-APPLIED`, `INTAKE-EXACTLY-ONCE-ORDERED`, `NET-BANDWIDTH-MEASURED`, `NET-IMPAIRMENT-OBSERVED` (emulated profiles only) and `SERVER-STEP-WITHIN-BUDGET`.

### Emulation profiles

UE 5.8.3 reads these switches in non-Shipping builds (`UNetDriver::InitPacketSimulationSettings`). The engine gives `PktOrder` precedence over `PktLag` and `PktDup` and refuses `PktDup` together with lag, so each impairment is its own profile.

| Profile | Switches (every process) | Effect on a round trip |
|---|---|---|
| `none` | — | Loopback only |
| `lag-loss` | `-PktLag=60 -PktLagVariance=20 -PktLoss=3` | About 120 ms added, variable, plus 3 % loss each way |
| `reorder` | `-PktOrder=1 -PktLoss=3` | Outgoing packets shuffled, plus 3 % loss each way |
| `duplicate` | `-PktDup=10 -PktLoss=3` | 10 % of packets sent twice, plus 3 % loss each way |

## B. Source changes (branch `codex/mp2-network-conditions-20261029`; implementation `9c97b65`, harness fix `495345d`)

| Area | Files | Change |
|---|---|---|
| Portable helper | `Source/PolitikaCore/Metrics.h`, `Tests/Core/RulesTests.cpp` | `NearestRank`, `RatePerSecond` and their static assertions. |
| Instrumentation | `PolitikaMultiplayer.cpp`, `PolitikaPrototypeGameplay.{h,cpp}` | `LogNetworkEmulation`, `SampleNetworkStats`, `SampleClientNetworkStats`, `ReportNetworkMeasurements`; per-step and per-frame timing in `APolitikaPrototypeMatch::Tick`. |
| Tests | `Private/Tests/PolitikaMultiplayerTests.cpp` | `ResultCommitAndCleanup` now asserts that every active step and frame is timed, the report is written once at the result, and a finished match adds no samples. |
| Harness | `Tools/Shell/src/bin/rts_multiplayer.rs` | `NetProfile`, `network_checks`, `net_samples`, 1v1 layout support and its supporting attack-move. |

## C. MP-0 audit register

- **F36 (bandwidth / server step): CLOSED for the fixture scale.** Measured per connection and per step across 2, 3 and 4 seats, with and without emulation. Unit-scale budgets remain open until a fixture with RTS unit counts exists.
- F24 (intake freshness) gains packaged evidence under real impairment: the reliable RPC channel delivers commands once and in order through loss, reordering and duplication. The `ClientSequenceGuard` rejected only the probes built to be rejected.
- Still open for MP-2: F33 (client fog shading), F35 (disconnect policy beyond ending the match). *Update 29 Oct 2026: F33 closed by the [fog memory record](RTS_MULTIPLAYER_MP2_FOG_MEMORY_ACCEPTANCE.md).*

## D. Verdicts

Vocabulary: PASS / FAIL / BLOCKED / INFRA_ERROR / INCONCLUSIVE / SKIPPED / NOT_RUN.

| Gate | Result | Evidence (`build/diagnostics/mp2-network/`) |
|---|---|---|
| `quality -- core` (C++20, `-Werror`) | PASS | `../core.json`, `logs/core.log` |
| `PolitikaEditor` Linux Development build | PASS | `logs/editor-build{,-2}.log` |
| Automation, full `Politika` suite | PASS 35/35. All 76 warnings are the same pre-existing environment and navigation-fixture warnings as the accepted MP-2 run; none are new. | `automation-all/index.json`, `logs/automation.log` |
| `package-multiplayer.sh server` / `development` from `9c97b65` | ExitCode 0, manifests verified, `git_dirty=false`, protocol 2 | `logs/package-{server,development}.log` |
| Layouts 2v1 / 3v1 / 2v2, profile `none` | PASS: every check, including the five new ones, in all three layouts. The protocol-1 refusal was not rerun (NOT_RUN, no RPC change). | `matrix-none/`, `matrix-none.out` |
| Layout 1v1, profile `none` | PASS 19/19; `FOG-ALLIED-SHARED` NOT_RUN (no allies) | `matrix-none-1v1/` |
| 1v1 + 2v2, profile `lag-loss` | PASS 45/45; `FOG-ALLIED-SHARED` NOT_RUN for 1v1 | `profile-lag-loss/` |
| 1v1 + 2v2, profile `reorder` | PASS 45/45; same NOT_RUN | `profile-reorder/` |
| 1v1 + 2v2, profile `duplicate` | PASS 45/45; same NOT_RUN | `profile-duplicate/` |
| MP-3 dedicated 1v1 regression (`--dedicated`) | PASS 23/23 | `regression-dedicated-1v1/` |
| MP-1 listen 1v1 regression | PASS 18/18 | `regression-listen-1v1/` |
| Offline PAC-10 / PA-AUTHORITY | PASS / PASS (first attempt each) | `logs/offline-{pac10,authority}.log` |
| `quality -- audit` (documentation index links), version tests, `cargo fmt --check` | PASS (87 links; 20 version tests) | `logs/audit.log`, `logs/version-tests.log` |

Every emulated layout ran the complete MP-2 choreography and passed all its existing checks: seat binding by join order, all-remote activation, initial fog from the server's activation samples, real-input orders from every client, contact reveal, one result seen by every viewer, server-originated damage, hostile and allied probes, fog-safe shot cues, the session reset and the launch-option check.

**Finding (harness, fixed):** the first 1v1 baseline run (in `matrix-none/layout-1v1/`) timed out waiting for a result. Team A's lone unit captured the point to 869 ‰ and was then destroyed by team B's unit, which was left at 1 health. Nobody remained to capture, so the match could not end. Larger layouts always had allied support. The runner now sends team A's second unit in support (`495345d`); the rerun is `matrix-none-1v1/`. This was not a gameplay defect: the stalled capture is the designed rule (progress holds without a capturer).

### Bandwidth (engine counters, activation to result, per client connection)

Down = server to client; up = client to server. Bytes are the engine's connection-level counts; UDP/IP headers (about 28 B per packet) are not included.

| Run | Seats | Down per client (B/s) | Up per client (B/s) | Packets/s down / up | RTT (ms) |
|---|---|---|---|---|---|
| 1v1 `none` | 2 | 1581–1627 | 1029–1076 | 28 / 25 | 38–39 |
| 2v1 `none` | 3 | 1491–1732 | 993–1043 | 27 / 24 | 37–44 |
| 3v1 `none` | 4 | 1488–1766 | 943–1000 | 26–27 / 23–24 | 39–46 |
| 2v2 `none` | 4 | 1529–1592 | 965–1034 | 27 / 24 | 39–41 |
| 1v1 `lag-loss` | 2 | 1385–1413 | 704–730 | 22–24 / 17 | 191–201 |
| 2v2 `lag-loss` | 4 | 1293–1423 | 639–696 | 21–23 / 16–17 | 160–192 |
| 1v1 `reorder` | 2 | 1490–1525 | 818–828 | 24–25 / 19–20 | 50–59 |
| 2v2 `reorder` | 4 | 1368–1640 | 731–796 | 23–24 / 18–19 | 53–75 |
| 1v1 `duplicate` | 2 | 1610–1621 | 1173–1178 | 28 / 28–29 | 32–36 |
| 2v2 `duplicate` | 4 | 1598–1718 | 1212–1268 | 29 / 30 | 33–43 |

- Loopback RTT is about 40 ms because the engine's RTT includes frame time at both ends; the dedicated server ticks at about 30 Hz (frame p50 33.4 ms in every run).
- Per-client traffic does not grow from two to four seats. With 5–9 units in the fixture, it is dominated by the connection's fixed packet rate, not by unit count.
- Under lag each client sent fewer packets of the same average size (about 17 instead of 24 per second), so upstream bytes fell.
- Duplication raised upstream bytes by about 10–25 % (the duplicates themselves).

### Impairment the engine counted (server and client counters combined, per run)

| Run | Packets lost | Out of order | Duplicate |
|---|---|---|---|
| All four `none` layouts | 0 | 0 | 0 |
| 1v1 / 2v2 `lag-loss` | 103 / 210 | 28 / 65 | 0 / 0 |
| 1v1 / 2v2 `reorder` | 115 / 251 | 48 / 94 | 0 / 0 |
| 1v1 / 2v2 `duplicate` | 155 / 263 | 393 / 865 | 10 / 15 |

Shot cues are unreliable cosmetic RPCs. Under emulation, clients received 80/86 and 222/228 (lag-loss), 83/86 and 218/228 (reorder), and 87/88 and 220/228 (duplicate) of the server's decisions. Every received cue matched its server decision and point, and no cue was received twice. All decisions were exact cues (see the [first increment's limitation](RTS_MULTIPLAYER_MP2_MULTICLIENT_ACCEPTANCE.md)).

### Authoritative step cost (`MP_SERVER_STEP_SUMMARY`, budget 50 000 µs)

| Run | Steps | Mean µs | p50 | p95 | p99 | Max | Catch-up cap frames |
|---|---|---|---|---|---|---|---|
| 1v1 `none` | 497 | 58.7 | 45.7 | 160.3 | 202.3 | 307.1 | 0 |
| 2v1 `none` | 531 | 72.4 | 67.0 | 240.8 | 280.8 | 966.0 | 0 |
| 3v1 `none` | 565 | 78.1 | 62.7 | 242.5 | 337.9 | 1776.4 | 0 |
| 2v2 `none` | 565 | 73.8 | 67.8 | 244.7 | 303.2 | 537.9 | 0 |
| 1v1 / 2v2 `lag-loss` | 515 / 584 | 56.4 / 75.1 | 43.7 / 31.8 | 157.8 / 257.6 | 196.7 / 363.8 | 223.4 / 1230.8 | 0 / 0 |
| 1v1 / 2v2 `reorder` | 499 / 572 | 59.0 / 73.2 | 48.7 / 54.5 | 161.5 / 244.6 | 238.2 / 478.1 | 722.4 / 890.0 | 0 / 0 |
| 1v1 / 2v2 `duplicate` | 496 / 559 | 60.1 / 72.5 | 42.1 / 63.1 | 186.0 / 241.1 | 291.5 / 288.0 | 352.8 / 1926.7 | 0 / 0 |

The step includes Development log sampling, so the Shipping cost will be lower. The host ran four rendered clients and their recorders at the same time, so the maxima are probably host contention rather than simulation cost.

## E. Identity and commands

| Artifact (`Saved/MP2Network20261029/Final-9c97b65/`) | SHA-256 |
|---|---|
| `Server/LinuxServer/Politika/Binaries/Linux/PolitikaServer` | `fb4a6ef5250e2c82dd067755a8814f59d272e1ad46fb564140f19d4d3ca0e0db` |
| `Development/Linux/Politika/Binaries/Linux/Politika` | `81bae423775d3efc7d3d6bb6f835d8f6dabdf37f06b09f30a5e263ffc0889677` |

`BuildIdentity.json`: `git_revision 9c97b6569332`, `git_dirty false`, `network_protocol 2`. The harness fix `495345d` changes only the Rust runner, so the packaged binaries are the same for every run. The archive is not the stable player default.

```bash
export UNREAL_ENGINE_DIR=/mnt/data/unreal/5.8.3
A=Saved/MP2Network20261029/Final-9c97b65
bash Tools/Shell/package-multiplayer.sh server      $A/Server
bash Tools/Shell/package-multiplayer.sh development $A/Development
S=$A/Server/LinuxServer/Politika/Binaries/Linux/PolitikaServer
C=$A/Development/Linux/Politika/Binaries/Linux/Politika
run() { cargo run --locked --manifest-path Tools/Shell/Cargo.toml --bin rts_multiplayer -- \
  --dedicated-layouts $S $C --layouts "$1" --net-profile "$2" --evidence "$3" --port "$4"; }
run 2v1,3v1,2v2 none      <DIR>/matrix-none       18100
run 1v1         none      <DIR>/matrix-none-1v1   18150
run 1v1,2v2     lag-loss  <DIR>/profile-lag-loss  18200
run 1v1,2v2     reorder   <DIR>/profile-reorder   18300
run 1v1,2v2     duplicate <DIR>/profile-duplicate 18400
```

The runner exits non-zero whenever any verdict is not PASS, including the expected NOT_RUN rows above. Read `multiclient-evidence.json` for the verdicts.

- Evidence root: `build/diagnostics/mp2-network/` in the branch worktree. A durable copy is at `/mnt/data/politika-qualification/MP2-NetworkConditions-20261029/`.
- Server processes: matrix 2v1 1142109, 3v1 1143483, 2v2 1145504 (the stalled first 1v1 attempt was 1139378); 1v1 rerun 1147337; lag-loss 1148345 / 1149162; reorder 1150704 / 1151757; duplicate 1153678 / 1154613.
- Xvfb: this runner left no lock. A stale `:99` from `xvfb-run` (offline replays) was removed after its process had exited; a live `:101` belonged to another session and was left alone.

## F. Limitations and next work

- **Evidence limits.** Loopback on one Linux host; engine emulation, not a real network; Xvfb with virtual input; a Development server and clients only; 5–9 units; team A won every run.
- **Next MP-2 increments.**
  - An Unaligned engagement fixture, so facing-only and withheld shot cues are observed over sockets.
  - Client fog shading and last-seen contacts (F33).
  - Listen-hosted 3–4 process runs.
  - A unit-scale fixture to set bandwidth and step budgets for real RTS unit counts.
