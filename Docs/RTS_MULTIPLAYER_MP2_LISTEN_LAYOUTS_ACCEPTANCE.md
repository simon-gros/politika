# MP-2 — Listen-hosted 2v1 / 3v1 / 2v2 with 3–4 processes: acceptance record

**Date:** 29 October 2026 · **Scope:** the fifth MP-2 increment of [Multiplayer Architecture](RTS_MULTIPLAYER_ARCHITECTURE.md) §9. It follows the [multi-client layouts](RTS_MULTIPLAYER_MP2_MULTICLIENT_ACCEPTANCE.md), [network conditions](RTS_MULTIPLAYER_MP2_NETWORK_CONDITIONS_ACCEPTANCE.md), [fog memory](RTS_MULTIPLAYER_MP2_FOG_MEMORY_ACCEPTANCE.md) and [Unaligned engagement](RTS_MULTIPLAYER_MP2_UNALIGNED_ENGAGEMENT_ACCEPTANCE.md) increments.

The first increment closed **F42** (packaged 2v1/3v1/2v2) for dedicated hosting only. Listen hosting of the larger layouts was covered by Automation, and the only packaged listen runs were the two-process MP-1 1v1. This increment runs 2v1, 3v1 and 2v2 + Unaligned with a rendered packaged listen host in seat 1 and two or three rendered packaged clients, over real sockets. It applies every check of the dedicated layout runs and adds three checks for the listen host's own view:
- fog presentation;
- last-known contacts;
- shot cues.

**Outcome: IMPLEMENTED; packaged qualification PASS for the stated scope.** No gameplay or network behaviour changed. `network_protocol` stays **2**. What was added: Development-only host instrumentation and a `--listen-layouts` harness mode. F42 is now closed for listen hosting as well.

Not qualified here:
- **Legacy-protocol refusal against a listen host.** `PreLogin` refuses an old protocol before hosting mode matters. The dedicated layout runs qualify that refusal (`--legacy-client`); the listen runs mark it NOT_RUN.
- **Listen 3v1 under emulation.** The `lag-loss` listen profile ran 2v1 and 2v2.
- **Facing-only and withheld cues on the listen host.** Player-versus-player combat always happens in mutual sight, so every packaged host cue was exact. That each host cue matched its allies' decisions shows the host shares its team's rule; the facing-only and none cases for the host view are covered by `ShotCueRedaction` Automation. The packaged facing-only and withheld cases were run on a dedicated server in the [engagement record](RTS_MULTIPLAYER_MP2_UNALIGNED_ENGAGEMENT_ACCEPTANCE.md).
- **Mid-match disconnects in 3–4 seat listen sessions.** F35 is unchanged: a departed participant ends the match. MP-1 qualifies this for 1v1 only.
- **Shipping builds.** All packaged runs are Development; the evidence lines are Development-only logs.

## A. What changed

- **Host fog transitions (`APolitikaPrototypeUnit::ApplyLocalFogVisibility`, `!UE_BUILD_SHIPPING`).** The listen host holds every actor and hides those its team does not observe (`ApplyLocalFogPresentation`). It now logs `MP_HOST_UNIT_FOG entity owner team hidden first alive x y` the first time each unit is presented and on every change. A hidden unit reports where the host last saw it, which is exactly where a last-known contact must sit. This is the host's counterpart of the client's `MP_CLIENT_UNIT_RELEVANT`/`WITHDRAWN`.
- **Host shot cues (`DistributeShotPresentation`, `!UE_BUILD_SHIPPING`).** The listen host's own cue for each shot was already resolved through `ResolveShotCue` for its seat, but it was never logged. It now logs `MP_HOST_SHOT_CUE sequence participant shooter target cue target_x target_y point_x point_y generation`.
- **Harness: `rts_multiplayer --listen-layouts CLIENT [--layouts 2v1,3v1,2v2] [--net-profile …]`.** The dedicated layout runner is now `layout_session`, shared by both hosting modes:
  - Seat 1 is the rendered listen host, driven by real X11 input through the server-side samples (`MP_SERVER_SAMPLE`, `MP_SERVER_OBJECTIVE`). Seats 2–4 are remote clients, as before.
  - `network_checks` takes the first remote participant, so intake and bandwidth are measured only for remote connections.
  - Normal completion differs by mode: the host returns to MainMenu, and every client explains the ended match. A dedicated server instead resets for a new generation.

## B. Source changes (on `main`)

| Area | Files | Change |
|---|---|---|
| Runtime instrumentation | `Source/Politika/Private/PolitikaMultiplayer.cpp` | `MP_HOST_UNIT_FOG`, `MP_HOST_SHOT_CUE` (Development only). |
| Harness | `Tools/Shell/src/bin/rts_multiplayer.rs` | `--listen-layouts`; `layout_session` with `seat_game`/`seat_markers`/`rendered_games`; `host_hidden_at_activation`, `host_contact_audit`, `host_shot_cue_audit`; `network_checks(first)`; two unit tests. |

## C. How it is checked

`rts_multiplayer --listen-layouts CLIENT` starts one packaged host (`/Game/Maps/RTSVerticalSlice?listen?PolitikaLayout=<L>`) and then one client per remaining seat, each on its own Xvfb display with its own user directory, log and MP4. Clients join one at a time so seats bind in a known order. Every check of the dedicated layout runs applies, with these listen-specific conditions:

| Check (`MP2-LISTEN-<L>-…`) | Pass condition |
|---|---|
| `LAYOUT` | `hosting=listen`, seats 3/4/4, `remote_seats` = seats − 1, fog on; the host is listening on the requested port with a net driver. |
| `SEATS-BY-JOIN-ORDER` | Participant 1 is the host (`remote=0`, team A). Participants 2.. are bound `remote=1` in join order with teams `[A,B,A]`, `[A,B,A,A]` or `[A,B,A,B]`. |
| `ACTIVE-HOST-AND-REMOTE` | `MP_MATCH_ACTIVE hosting=listen local=1 remote=seats−1`, and as many distinct processes as seats. |
| `FOG-INITIAL`, `FOG-ALLIED-SHARED` | At activation, the units the host leaves unhidden (`host_hidden=0`) are exactly those team A observes, including its allies' units. Each client receives exactly its team's observed units. |
| `ORDERS-EVERY-CLIENT` | Real-input Stop and Guard from every seat are admitted for that seat's participant. Every local command (`remote=0`) is the host's participant 1; every client's command is remote. |
| `HOST-SHOT-CUES` | Every host cue for a shot equals the server's decision for each allied remote participant on the same shot sequence, since team vision is shared. Exact cues sit on the target, and facing cues stay more than 100 cm from it. |
| `FOG-MEMORY-NO-LEAK` | Every host contact (`role=host`) follows a live `MP_HOST_UNIT_FOG hidden=1` of the same entity at the same position, after the host had seen it, and never names team A. Client contacts pass the existing audit. |
| `NET-*`, `INTAKE-EXACTLY-ONCE-ORDERED`, `SERVER-STEP-WITHIN-BUDGET` | As for dedicated runs, over the remote connections; the step summary reports `hosting=listen` and all seats. |
| `RETURN-AFTER-RESULTS` | After the single committed result, the host leaves for MainMenu. Every client reports `MP_CLIENT_NETWORK_FAILURE result_observed=1` with the observed result and returns to MainMenu. |

## D. Verdicts

Vocabulary: PASS / FAIL / BLOCKED / INFRA_ERROR / INCONCLUSIVE / SKIPPED / NOT_RUN. Evidence root: `/mnt/data/politika-qualification/MP2-ListenLayouts-20261029/` (`final-65ae853/` for the packaged matrix).

Package identity: `package-multiplayer.sh development` and `server` from `65ae853`, UAT exit 0, `git_dirty=false`, `network_protocol` 2. Executable SHA-256: client `c6fd1d31d53c43743fd30f04d62fb7100f655b339e910caf1a4fecb14a838e17`, server `27521431bc32230a25d1ac85fa3efd2918073930cadf69ca16f6eb311a662d4a`. Harness: `rts_multiplayer` / `rts_interaction` built from the same commit.

| Gate | Result | Evidence |
|---|---|---|
| `PolitikaEditor` Linux Development build | PASS | — |
| Exploratory listen 2v1 (`try1`) | PASS 24/24; the host remembered the Unaligned unit #8 from its own fog withdrawal, and its screenshot shows team A's shared vision and the `LAST SEEN #8` marker | `try1/listen-2v1/` |
| **Listen 2v1 / 3v1 / 2v2, profile `none`, run 1** | **PASS 74/74**; one expected NOT_RUN (legacy protocol). Host cues: 57 per layout, all exact, matching every allied decision (57, 114 with two allies in 3v1, 57). Host contacts: one per layout, each from a host fog withdrawal (5–6 transitions per run). Per layout: 3/4/4 processes, `local=1 remote=2/3/3`. | `final-65ae853/listen-none-run1/` |
| **Listen 2v1 / 3v1 / 2v2, profile `none`, run 2** | **PASS 74/74**; same expected NOT_RUN | `final-65ae853/listen-none-run2/` |
| **Listen 2v1 + 2v2, profile `lag-loss`** | **PASS 51/51**; same expected NOT_RUN. The engine counted 137 and 189 lost packets and 46 and 66 out-of-order packets. Client RTT was 164–180 ms, and downstream was about 1.5–1.8 KB/s per connection. | `final-65ae853/listen-lag-loss/` |
| Dedicated layouts 2v1 / 3v1 / 2v2, profile `none` (shared runner regression) | PASS 74/74; expected NOT_RUN: legacy protocol (no `--legacy-client`) | `final-65ae853/matrix-none/` |
| Dedicated layout 1v1, profile `none` | PASS 20/20; expected NOT_RUNs: allied fog (1v1), legacy protocol | `final-65ae853/matrix-none-1v1/` |
| Dedicated layouts 1v1 + 2v2, profile `lag-loss` | PASS 47/47; expected NOT_RUNs: 1v1 allied fog, legacy protocol | `final-65ae853/profile-lag-loss/` |
| Unaligned engagement (`--engagement`) | PASS 9/9 | `final-65ae853/engagement/` |
| Fog memory (`--fog-memory`) | PASS 7/7 | `final-65ae853/fog-memory/` |
| MP-3 dedicated 1v1 regression | PASS 23/23 | `final-65ae853/regression-dedicated-1v1/` |
| MP-1 listen 1v1 regression | PASS 18/18 | `final-65ae853/regression-listen-1v1/` |
| Offline PAC-10 and participant authority (`rts_interaction`) | PASS, first attempt each | `final-65ae853/offline-{pac10,authority}.out` |
| Automation, full `Politika` suite (editor built from `65ae853`) | PASS 36/36, exit 0 | `automation-all/` |
| Harness: `cargo fmt --check`, `cargo test --bin rts_multiplayer` | PASS; 8/8 unit tests, including the two new host audits | — |

## E. Reproduction

```bash
export UNREAL_ENGINE_DIR=/mnt/data/unreal/5.8.3
Q=/mnt/data/politika-qualification/MP2-ListenLayouts-20261029
bash Tools/Shell/package-multiplayer.sh development $Q/dev-65ae853
C=$Q/dev-65ae853/Linux/Politika/Binaries/Linux/Politika
cargo run --locked --release --manifest-path Tools/Shell/Cargo.toml --bin rts_multiplayer -- \
  --listen-layouts $C --layouts 2v1,3v1,2v2 --evidence <DIR> --port 19000
```

`$Q/run-matrix.sh` runs the packaged matrix in §D sequentially, using the harness binaries copied next to it. The dedicated regressions also need `package-multiplayer.sh server $Q/server-65ae853`.

## F. Limitations and next work

- **Evidence limits.** Loopback on one Linux host; Xvfb with virtual input; Development builds; one scripted choreography per layout, in which team A always takes the objective.
- **Audit register.** F42 is closed for both hosting modes. No new finding.
- **Still open for MP-2:** ~~F43~~ *(closed in `f6c354b`, see the [engagement record](RTS_MULTIPLAYER_MP2_UNALIGNED_ENGAGEMENT_ACCEPTANCE.md) §F)*, AI-filled seats (no skirmish AI exists yet), unit-scale bandwidth/step budgets, and F35 (disconnect policy beyond ending the match).
