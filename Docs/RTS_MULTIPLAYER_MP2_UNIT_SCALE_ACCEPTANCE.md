# MP-2 — Unit-scale bandwidth and server step budgets: acceptance record

**Date:** 29 October 2026 · **Scope:** the sixth MP-2 increment of [Multiplayer Architecture](RTS_MULTIPLAYER_ARCHITECTURE.md) §9. The [network conditions record](RTS_MULTIPLAYER_MP2_NETWORK_CONDITIONS_ACCEPTANCE.md) closed audit finding F36 at fixture scale only (5–9 units) and left "unit-scale budgets" open. This increment measures per-connection bandwidth and authoritative step cost at RTS unit counts (52 to 802 units) in packaged dedicated sessions. It also supplies the measurement that [Unit Specification Framework](UNIT_SPECIFICATION_FRAMEWORK.md) decision **U-12** (technical per-participant entity cap) waits for.

**Outcome: IMPLEMENTED; packaged measurement PASS for the stated scope, with two new open findings.**
- Every run spawned the requested units, admitted every order, replicated exactly what each team sees, kept shot cues fog-safe, and stayed inside the 50 ms step budget at p99, with no catch-up frames.
- **F44 (open):** per-unit movement replication fills the engine's 100 KB/s per-connection send rate at about 400 units in a converged fight. The headroom gate fails at 1v1 × 200, 2v2 × 100 and 2v2 × 200 per seat. These are the three FAIL rows in §D.
- **F45 (open):** the step that admits a whole-army order spends about 30–70 µs per order on route queries. At 800 orders that one step takes 56 ms, longer than one 50 ms step.
- No gameplay, replicated state or RPC changed. `network_protocol` stays **2**. All new code is Development-only (`!UE_BUILD_SHIPPING`).

Not qualified here:
- **Shipping builds.** All processes are Development. Step cost includes Development event and sample logging, which grows with unit count.
- **Real networks.** Loopback on one host. No emulation profile was applied, so these are best-case links.
- **3v1 and 2v1.** Only 1v1 and 2v2 were run. Bandwidth tracks how many units a client receives, so 2v2 at the same total stands in for them, but that is inferred, not measured.
- **Production-like armies.** Every unit is the same rifle infantry archetype. All units attack-move together, so every client ends up seeing nearly every unit. That is the worst case for bandwidth, not a typical match.
- **Client rendering budgets.** Only seat 1's client renders, under Xvfb software rendering. Its frame times are recorded as facts, not gates.

## A. What changed

- **Unit-scale fixture: `-PolitikaMPUnitScale=N`.** Development builds only. The server accepts it for a networked session on `map.test.multiplayer_skirmish`, with N from 2 to 200 per seat.
  - Each seat keeps its two authored placements and gets N − 2 more of the same archetype.
  - Placement is planned by the portable `politika::unitscale::PlanSeatCandidates` (`Source/PolitikaCore/UnitScale.h`): a 100 cm grid around the seat start, nearest first, inside the playable area (100 cm inset). A cell must be strictly closer to its own start than to any other seat's start, so seats never interleave. It must also be at least 60 cm from authored placements.
  - Each cell is projected onto the navmesh (within 25 cm) at the authored height above ground.
  - Any other value, map or session fails admission closed. Reasons: `MP.UNIT_SCALE.INVALID`, `.UNSUPPORTED`, `.PLACEMENT`, `.NAVIGATION_UNAVAILABLE`. A dedicated server then exits 3 like any admission failure.
  - `MP_UNIT_SCALE_SEAT` and `MP_UNIT_SCALE_FIXTURE` record per-seat counts, the largest radius used and the closest opposing pair at spawn.
- **Load driver.** After `MP_MATCH_ACTIVE` the units hold for 200 steps (10 s, phase `idle`). Then the server submits one attack-move per seated unit through the trusted `SubmitMove` path that offline opponent seats use (phase `engage`).
  - Each unit's destination is the objective plus its offset from its seat start, so seats converge from their own sides.
  - The orders bypass the remote intake queue (16 pending per participant) on purpose: no multi-unit client command exists yet (see §F).
- **Measurements.**
  - `MP_UNIT_SCALE_WINDOW`: once a second, the step count, mean and maximum step cost, units alive and units destroyed.
  - `MP_UNIT_SCALE_STEP_SUMMARY`: once per phase at the result or a disconnect, with nearest-rank p50/p95/p99/max.
  - `MP_UNIT_SCALE_ORDERS` and `MP_UNIT_SCALE_ADMISSION`: issued, accepted and rejected orders, plus the cost of the step that admitted them.
  - `MP_NET_STATS` gains `net_speed` (the connection's configured send rate) and `channels` (open channels, mostly unit actor channels).
  - Every networked client logs `MP_CLIENT_FRAME_WINDOW` once a second: frames, mean and maximum frame time, units present.
- **Bootstrap.** `-PolitikaMPUnitScale=` joins the Development-only valued switches. Strict (Shipping) policy rejects it, and an empty value is rejected under both policies (`Politika.Bootstrap.LaunchOptions`).
- **Harness: `rts_multiplayer --unit-scale SERVER CLIENT [--layouts 1v1,2v2] [--units 25,50,100,200] [--engage-seconds 45]`.**
  - Each layout × count gets a fresh packaged headless server and one client per seat. Seat 1's client renders on Xvfb; the others run `-nullrhi` so host contention stays bounded.
  - The harness takes an idle and an engagement screenshot, holds the engagement until the result or the time limit, then closes the last client so the server reports.
  - Server and client traffic is split at the engagement line using the logs' shared wall clock.
  - A separate server with `-PolitikaMPUnitScale=201` checks the refusal.

### Checks per run

| Check | Pass condition |
|---|---|
| `UNITSCALE-SPAWNED` | `seat_units` = seats × N, and the match holds seats × N + Unaligned units. |
| `UNITSCALE-ORDERS-ADMITTED` | Engagement orders issued = seats × N, all accepted, none rejected. |
| `UNITSCALE-REPLICATION-COMPLETE` | Just before the engagement, every client holds exactly as many units as the server says its team sees. |
| `UNITSCALE-STEP-WITHIN-BUDGET` | Idle and engagement step p99 below 50 ms, and no frame hit the five-step catch-up cap. |
| `UNITSCALE-BANDWIDTH-MEASURED` | Every connection has idle and engagement traffic: down, up and busiest one-second downstream. |
| `UNITSCALE-NET-HEADROOM` | The busiest one-second downstream interval of every connection is below 80 % of its `net_speed`. |
| `UNITSCALE-ENGAGED` | Units were destroyed and shots were decided, so the engagement really happened. |
| `UNITSCALE-SHOT-CUE-CONSISTENT` | `shot_cue_consistency` over every client finds no problem. |
| `UNITSCALE-INVALID-REFUSED` | With 201 per seat: exit code 3, `MP_UNIT_SCALE_INVALID`, `reason=MP.UNIT_SCALE.INVALID`, `MP_DEDICATED_ADMISSION_FAILED`, and no session or units. |

## B. Source changes (on `main`, commit `9212978`)

| Area | Files | Change |
|---|---|---|
| Portable planner | `Source/PolitikaCore/UnitScale.h`, `Tests/Core/RulesTests.cpp` | `ValidUnitsPerSeat`, `PlanSeatCandidates`; ordering, seat split, clearance, bounds and degenerate-input tests. |
| Fixture and driver | `Source/Politika/Private/PolitikaUnitScaleFixture.cpp` (new), `Public/PolitikaPrototypeGameplay.h`, `Private/PolitikaPrototypeGameplay.cpp` | `ResolveUnitScale` runs after spatial validation on a local copy of the authored placements (the map asset is untouched). Also `BeginUnitScaleMeasurement`, `TickUnitScaleDriver`, `ObserveUnitScaleEvent`, `ReportUnitScaleMeasurements`, and the client frame window. |
| Measurements | `Private/PolitikaMultiplayer.cpp` | `net_speed` and `channels` in `MP_NET_STATS`, `MP_CLIENT_FRAME_WINDOW`, and the unit-scale hooks at activation and report time. |
| Launch options | `Source/PolitikaBootstrap/Private/PolitikaBootstrap.cpp`, `Private/Tests/PolitikaLaunchOptionsTests.cpp` | Recognize `-PolitikaMPUnitScale=`; accepted and rejected cases. |
| Harness | `Tools/Shell/src/bin/rts_multiplayer.rs` | `--unit-scale` mode, `log_time`, `timed_net_samples`, `phase_traffic`, `frame_windows`; two unit tests. |

## C. Results

Evidence root: `/mnt/data/politika-qualification/MP2-UnitScale-20261029/final-9212978/unit-scale/`. There is one directory per run (`1v1-n25` … `2v2-n200`) with every process log and the screenshots, plus `unit-scale-summary.tsv` and `unit-scale-evidence.json`.

### Bandwidth per client (engine counters; down = server to client)

| Layout × per seat | Units | Idle down (B/s) | Engage down, mean (B/s) | Engage down, busiest second (B/s) | Share of 100 000 B/s | Headroom gate |
|---|---|---|---|---|---|---|
| 1v1 × 25 | 52 | 1 330–2 364 | 8 436–8 564 | 16 603 | 17 % | PASS |
| 1v1 × 50 | 102 | 1 327–3 195 | 15 896–16 114 | 31 377 | 31 % | PASS |
| 1v1 × 100 | 202 | 1 328–4 859 | 31 566–32 189 | 54 329 | 54 % | PASS |
| 1v1 × 200 | 402 | 1 329–8 182 | 65 356–66 312 | 95 759 | 96 % | **FAIL** |
| 2v2 × 25 | 102 | 1 335–3 210 | 11 839–12 701 | 31 125 | 31 % | PASS |
| 2v2 × 50 | 202 | 1 338–4 871 | 23 996–24 999 | 60 585 | 61 % | PASS |
| 2v2 × 100 | 402 | 1 341–8 155 | 48 672–50 694 | 99 907 | 100 % | **FAIL** |
| 2v2 × 200 | 802 | 1 533–14 729 | 68 786–72 227 | 100 910 | 101 % (capped) | **FAIL** |

- **About 300 B/s per unit in the fight.** Until the cap is reached, the busiest second scales almost linearly with the units in the converged fight: 300–320 B/s per unit at 52–202 units, in both layouts. 80 % of the 100 KB/s cap is therefore reached at about 260 units that one client receives.
- **Saturation looks like this.** At 2v2 × 100 and × 200 every connection sat at the cap (99.9–100.9 KB/s) for its busiest seconds. The engine was dropping or delaying unit updates to stay inside the send rate. This is finding **F44**.
- **Idle traffic is flat.** Stationary units cost nothing beyond the connection's fixed packet rate (about 1.3 KB/s). The higher idle numbers belong to the last client to join: its initial replication of every visible unit (about 400 B per unit) falls inside the idle window because activation follows its bind immediately.
- **Upstream is unaffected** (about 1.2 KB/s), because orders come from the server-side driver.
- Engagements lasted 10.5–13.1 s in 1v1 and 19.8–20.9 s in 2v2 before the objective decided the match.

### Authoritative step cost (budget 50 000 µs)

| Layout × per seat | Units | Idle p99 (µs) | Engage p99 (µs) | Engage max (µs) | Admission step (µs) | Per order (µs) | Catch-up frames | Destroyed |
|---|---|---|---|---|---|---|---|---|
| 1v1 × 25 | 52 | 237 | 811 | 1 402 | 1 402 | 28 | 0 | 28 |
| 1v1 × 50 | 102 | 859 | 2 039 | 2 978 | 2 978 | 30 | 0 | 57 |
| 1v1 × 100 | 202 | 1 583 | 4 924 | 9 518 | 9 518 | 48 | 0 | 94 |
| 1v1 × 200 | 402 | 3 488 | 9 296 | 21 772 | 21 772 | 54 | 0 | 157 |
| 2v2 × 25 | 102 | 409 | 1 858 | 2 920 | 2 920 | 29 | 0 | 80 |
| 2v2 × 50 | 202 | 1 393 | 4 758 | 6 373 | 6 373 | 32 | 0 | 151 |
| 2v2 × 100 | 402 | 5 319 | 12 949 | 18 606 | 18 606 | 47 | 0 | 268 |
| 2v2 × 200 | 802 | 13 105 | 22 173 | **56 143** | **56 143** | 70 | 0 | 516 |

- **The engagement maximum is always the admission step.** The step after the driver's orders resolves a navigation route for every unit at once. Per-order cost grows from about 30 µs to 70 µs with crowding. At 800 orders that single step (56 ms) is longer than one simulation step. The catch-up loop absorbed it without hitting the cap, but it is a real spike: finding **F45**.
- **Idle cost grows faster than linearly.** With every unit stationary, idle p99 goes 0.4 → 1.4 → 5.3 → 13.1 ms for 102 → 802 units in 2v2. That points at all-pairs work per step (team vision and idle target acquisition). It is still inside budget at 802 units, but quadratic growth would reach the 50 ms step at roughly 1 600 units.
- The dedicated server's frame time stayed at its 30 Hz cadence (frame p95 33.5 ms in every run).
- Development logging is inside these numbers: per-event `RTS_GAMEPLAY_EVENT` lines, `MP_SHOT_CUE_SENT` per connection per shot (69 480 decisions in the largest run) and a per-unit `MP_SERVER_SAMPLE` every second. Shipping step cost will be lower by an unmeasured amount.

### Other observations

- **Replication completeness:** just before the engagement, every client in every run held exactly the units its team sees, from 26 at 1v1 × 25 to 408 at 2v2 × 200.
- **Shot cues:** 2 094 to 69 480 decisions per run, all `exact` (every engagement is in mutual sight), every one received and consistent. Facing-only and withheld cues are covered by the [engagement record](RTS_MULTIPLAYER_MP2_UNALIGNED_ENGAGEMENT_ACCEPTANCE.md), not by this fixture.
- **Spawn geometry:** the closest opposing pair at spawn was 2 417 cm at 1v1 × 25 and 1 128 cm at 2v2 × 200. The largest grid radius was 922 cm. Opposing units are at most just inside the 1 200 cm sight at the largest scale, and never inside the 800 cm idle acquisition, so the idle phase never fought.
- **Client frame time** (seat 1, Xvfb software rendering, host-specific): means of 17–22 ms idle and 17–23 ms during engagements at every scale, with 1-second maxima of 26–73 ms. This host's rendered client is not the bottleneck at these counts. These numbers are not a rendering budget.
- **Presentation at scale (observation, not gated):** the development HUD draws a name and health label over every unit. At 200 per seat the labels overlap into an unreadable block (`2v2-n200/client-a-{idle,engage}.png`). Label aggregation or decluttering belongs to the [HUD architecture](RTS_HUD_ARCHITECTURE.md), not to networking.

## D. Verdicts

Vocabulary: PASS / FAIL / BLOCKED / INFRA_ERROR / INCONCLUSIVE / SKIPPED / NOT_RUN. Evidence root: `/mnt/data/politika-qualification/MP2-UnitScale-20261029/`.

| Gate | Result | Evidence |
|---|---|---|
| `tools/core-test.sh` (C++20, `-Werror`), including the new planner tests | PASS | — |
| `PolitikaEditor` Linux Development build | PASS | `editor-build.log` |
| `package-multiplayer.sh development` / `server` from `9212978` | ExitCode 0; `git_dirty=false`, `git_revision 9212978b6399`, `network_protocol` 2 | `package-{dev,server}.log` |
| Harness: `cargo fmt --check`, `cargo test --bin rts_multiplayer` | PASS; 10/10 unit tests | — |
| `--unit-scale` refusal (201 per seat) | PASS | `final-9212978/unit-scale/invalid-201/` |
| `--unit-scale` 1v1 and 2v2 × 25 / 50 / 100 / 200 per seat | 62 PASS, 3 FAIL. All three FAILs are `UNITSCALE-NET-HEADROOM` at 1v1 × 200, 2v2 × 100 and 2v2 × 200 (F44). Every other check passes in all eight runs. The runner exits 1 because of those FAILs. | `final-9212978/unit-scale.out`, `unit-scale/` |
| Automation, full `Politika` suite | REGRESSION_PENDING | `automation-all/` |
| Regression matrix (listen and dedicated layouts, lag/loss, engagement, fog memory, MP-3, MP-1, offline PAC-10 and participant authority) | REGRESSION_PENDING | `final-9212978/summary.txt` |

## E. Identity and reproduction

| Artifact | SHA-256 |
|---|---|
| `server-9212978/LinuxServer/Politika/Binaries/Linux/PolitikaServer` | `7e4ade2781bb46414310d6b17540a9ffcdf0ae9c93be6cc195440151b4436807` |
| `dev-9212978/Linux/Politika/Binaries/Linux/Politika` | `bc1655682a2c76487d14ba207df17eae8401eb16a20d621c231a7062fa4dab23` |

The archives are not the stable player default.

```bash
export UNREAL_ENGINE_DIR=/mnt/data/unreal/5.8.3
Q=/mnt/data/politika-qualification/MP2-UnitScale-20261029
bash Tools/Shell/package-multiplayer.sh development $Q/dev-9212978
bash Tools/Shell/package-multiplayer.sh server      $Q/server-9212978
S=$Q/server-9212978/LinuxServer/Politika/Binaries/Linux/PolitikaServer
C=$Q/dev-9212978/Linux/Politika/Binaries/Linux/Politika
cargo run --locked --release --manifest-path Tools/Shell/Cargo.toml --bin rts_multiplayer -- \
  --unit-scale $S $C --layouts 1v1,2v2 --units 25,50,100,200 --evidence <DIR> --port 19200
```

`$Q/after-package.sh` runs the identity check and the sweep. `$Q/automation-and-matrix.sh` runs Automation and the regression matrix in `$Q/run-matrix.sh`.

## F. Findings, U-12 and next work

- **F44 (open): unit movement replication saturates the per-connection send rate.**
  - Every unit actor replicates its movement to each client whose team sees it, at the engine default update rate, limited only by the server's 30 Hz tick. That costs about 300 B/s per moving unit received.
  - The engine's default `net_speed` of 100 000 B/s is reached at about 330 units received; 80 % at about 260.
  - Candidate remedies, to be chosen against measurements:
    - a lower or adaptive `NetUpdateFrequency` for units;
    - quantized or batched movement state instead of per-actor `ReplicatedMovement`;
    - priority by distance from the viewer's camera;
    - a higher configured client rate for LAN and dedicated play.
  - Rerun this sweep after any change; the busiest-second column is the gate.
- **F45 (open): whole-army order admission is one step.**
  - Admission resolves every route in the step after submission, about 30–70 µs per order. At 800 orders that is 56 ms in one step.
  - A real multi-unit command (see below) would hit the same spike.
  - Candidate remedies: spread route resolution over several steps with an ordered budget per step, or share one route per group with formation offsets.
- **U-12 measurement (owner decision).**
  - With current replication, a match whose armies converge stays under 80 % of the send rate at about 260 units in total. Measured passing points: 1v1 at 100 per seat (202 units, 54 %) and 2v2 at 50 per seat (202 units, 61 %).
  - Proposed provisional technical cap until F44 is fixed: **50 units per participant**. That keeps 2v2 and 3v1 at or below about 200 units total. 3v1 and 2v1 are not run here (inferred from total units).
  - The step budget is not the binding limit at these counts. Bandwidth is.
- **Not covered and next.**
  - Multi-unit selection and group commands: there is no box selection, and the 16-command intake queue would reject a large army order sent one unit at a time.
  - AI-filled seats (no skirmish AI exists).
  - F35 (disconnect policy beyond ending the match).
  - A Shipping server measurement.
  - This sweep under the `lag-loss` profile.
