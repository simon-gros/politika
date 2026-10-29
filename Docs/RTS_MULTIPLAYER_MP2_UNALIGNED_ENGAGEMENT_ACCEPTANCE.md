# MP-2 — Unaligned engagement and shot-cue redaction over sockets: acceptance record

**Date:** 29 October 2026 · **Scope:** the fourth MP-2 increment of [Multiplayer Architecture](RTS_MULTIPLAYER_ARCHITECTURE.md) §9, after the [multi-client layouts](RTS_MULTIPLAYER_MP2_MULTICLIENT_ACCEPTANCE.md), [network conditions](RTS_MULTIPLAYER_MP2_NETWORK_CONDITIONS_ACCEPTANCE.md) and [fog memory](RTS_MULTIPLAYER_MP2_FOG_MEMORY_ACCEPTANCE.md) increments. The first increment closed audit finding **F27**: each connection gets an exact, facing-only or no shot cue, decided from its team's vision. Its packaged runs only ever produced exact cues, because A-versus-B combat always happens in mutual sight. The facing-only and withheld cases were covered by Automation alone. This increment adds a packaged fixture that produces both over real sockets. It also exercises explicit attacks on the Unaligned side, which [visibility](RTS_VISIBILITY_AND_INTELLIGENCE.md) and the F34 rules allow and which no packaged run had used.

**Outcome: IMPLEMENTED; packaged qualification PASS for the stated scope.** No gameplay or network behaviour changed. `network_protocol` stays **2**. What was added:
- a Development-only screen probe for fixture map points;
- a new harness session;
- a new open finding, **F43**: the facing-only cue's endpoint can land near the hidden target. See §F.

Not qualified here:
- **Listen-hosted engagement.** The listen host's own view follows the same rule (`DistributeShotPresentation` resolves it through `ResolveShotCue`). It is covered by `ShotCueRedaction` Automation, not by a packaged listen run.
- **3–4 seat engagement and network emulation.** The fixture is 1v1 + Unaligned, profile `none`. The existing per-layout `SHOT-CUE` check still runs in every layout and under `lag-loss`.
- **Shipping builds.** All packaged runs are Development; the evidence lines are Development-only logs.
- **Grenade cues.** They carry no position (F27) and are not exercised here.

## A. What changed

- **Ground probes (`APolitikaPrototypeMatch::SampleGroundProbes`, `!UE_BUILD_SHIPPING`).** `-PolitikaMPGroundProbe=X:Y/X:Y` makes a rendered client log `MP_CLIENT_GROUND_PROBE index x y z ground_hit on_screen screen_x screen_y viewer` once a second, with the observed-state sample. Each point is traced to the ground on the visibility channel that the controller uses for clicks; units are ignored, and the trace falls back to the objective's height. A packaged run can then right-click an exact fixture position with real input. Before this, it could only click units, the objective, or midpoints between them. Malformed points log `MP_CLIENT_GROUND_PROBE_INVALID`.
- **Bootstrap.** `-PolitikaMPGroundProbe=` joins the Development-only valued switches. Strict (Shipping) policy still rejects it, and an empty value is rejected under both policies (`Politika.Bootstrap.LaunchOptions`).
- **Harness — `rts_multiplayer --engagement SERVER CLIENT`.** `--fog-memory` and `--engagement` now share one session runner (`run_dedicated_session`). `shot_cue_consistency` takes client references.

## B. Source changes (on `main`)

| Area | Files | Change |
|---|---|---|
| Runtime instrumentation | `Source/Politika/Private/PolitikaMultiplayer.cpp`, `Public/PolitikaPrototypeGameplay.h` | `SampleGroundProbes` and its parsed point list (Development only). |
| Launch options | `Source/PolitikaBootstrap/Private/PolitikaBootstrap.cpp`, `Private/Tests/PolitikaLaunchOptionsTests.cpp` | Recognize `-PolitikaMPGroundProbe=`; accepted/rejected cases. |
| Harness | `Tools/Shell/src/bin/rts_multiplayer.rs` | `--engagement` session, ground-probe/arrival/attack helpers, shared session runner, two unit tests (probe parsing; the fixture vision geometry behind the two posts). |

## C. How it is checked

**`rts_multiplayer --engagement SERVER CLIENT`**: a packaged headless server, with team A (client A) and team B (client B) both rendered on their own Xvfb displays. The layout is 1v1 + Unaligned on `map.test.multiplayer_skirmish`. Client A drives everything with real X11 input:

1. It moves its first unit to the **facing post** (300, 1300). The post is 304 cm from the northern militia, inside the 650 cm weapon range. It is about 1057 cm from team B's nearer start unit: inside that unit's 1200 cm sight, but outside its 800 cm idle acquisition. That team-B unit stays 1307 cm from the militia. Client A then right-clicks the militia, an explicit Attack, and the unit fires until the militia dies. Team B sees the shooter but never the target.
2. It moves its second unit to the **withheld post** (−700, −950), where the southern militia comes into sight 707 cm away, and attacks it. No team-B unit is anywhere near, so team B sees neither shooter nor target.

Positions and observation come from the server's own samples (`MP_SERVER_SAMPLE` positions and `ObservedByTeams` masks). Cue decisions come from `MP_SHOT_CUE_SENT` and receipts from `MP_CLIENT_SHOT_CUE`.

| Check | Pass condition |
|---|---|
| `MP2-ENGAGE-FACING-GEOMETRY` | Before the attack, the shooter is within 625 cm of the militia. The server marks the shooter observed by team B and the militia not. Team B's nearest unit is inside sight of the shooter and outside sight of the militia. |
| `MP2-ENGAGE-EXPLICIT-ATTACK-ON-UNALIGNED` | Both explicit Attack orders on Unaligned units are admitted and both militia are destroyed (`RTS_UNIT_DESTROYED`). |
| `MP2-ENGAGE-UNALIGNED-NEVER-RETALIATES` | No shot decision has an Unaligned shooter. Auto-engagement never crosses the Unaligned boundary. |
| `MP2-SHOT-CUE-OWNER-EXACT` | Every team-A decision for both engagements is `exact`, and client A received exact cues for both. |
| `MP2-SHOT-CUE-FACING-OVER-SOCKETS` | Every team-B decision for the northern engagement is `facing`. Client B received such cues, all with `exact=0`, and the northern militia was never replicated to client B. |
| `MP2-SHOT-CUE-WITHHELD-OVER-SOCKETS` | Every team-B decision for the southern engagement is `none`. Client B received none of those sequences, and neither the shooter nor the southern militia was ever replicated to client B. |
| `MP2-SHOT-CUE-CONSISTENT` | `shot_cue_consistency` over both clients finds no problem: exact points within 1 cm of the target; facing points away from the target and not received as exact; no withheld cue received; received points within 1.5 cm of the server's. The run also contains facing and withheld decisions. |
| `MP2-ENGAGE-FOG-MEMORY` | Client B, which never saw a militia, recorded no contact. Client A, which saw both deaths, did not remember either militia. |
| `MP2-ENGAGE-MATCH-UNDISTURBED` | No match result was committed and objective progress stayed 0, so the fixture never touched the control point. |

## D. Verdicts

Vocabulary: PASS / FAIL / BLOCKED / INFRA_ERROR / INCONCLUSIVE / SKIPPED / NOT_RUN. Evidence root: `/mnt/data/politika-qualification/MP2-Engagement-20261029/` (`final-239b8db/` for the packaged matrix).

Package identity: `package-multiplayer.sh development` and `server` from `239b8db`, UAT exit 0, `git_dirty=false`, `network_protocol` 2. Executable SHA-256: client `b3f10d0212e7572db961ca7fca4a4a04664c9f33bfea57ef4ff78478e421ac09`, server `3c033fc108d8c5c1adc7ed34ab11994cd9bfcdde69409f91fa6989a9b1573873`. Harness: `rts_multiplayer` / `rts_interaction` built from the same source (the later harness commit adds unit tests only).

| Gate | Result | Evidence |
|---|---|---|
| `PolitikaEditor` Linux Development build | PASS | — |
| `engagement-run1`, `engagement-run2` (plus the exploratory `try1`) | PASS 9/9 each. Shooter 301 cm from the militia. Team B's nearest unit was 1055 cm from the shooter and 1307 cm from the militia. Observed masks: shooter 14 (teams A, B, Unaligned), militia 10 (A, Unaligned). Per run, the server decided 58 exact, 29 facing and 29 none; clients received 87: all 58 exact, plus all 29 facing with `exact=0`. Facing endpoints were 299–310 cm from the hidden militia. Zero contacts remembered; objective untouched. | `final-239b8db/engagement-run{1,2}/`, `try1/` |
| `fog-memory` (shared runner regression) | PASS 7/7 | `final-239b8db/fog-memory/` |
| Dedicated layouts 2v1 / 3v1 / 2v2, profile `none` | PASS 74/74; one expected NOT_RUN (`MP2-PROTOCOL-LEGACY-REFUSED`, no `--legacy-client`) | `final-239b8db/matrix-none/` |
| Dedicated layout 1v1, profile `none` | PASS 20/20; expected NOT_RUNs: allied fog (1v1 has no allies), legacy protocol | `final-239b8db/matrix-none-1v1/` |
| Dedicated layouts 1v1 + 2v2, profile `lag-loss` | PASS 47/47; expected NOT_RUNs: 1v1 allied fog, legacy protocol | `final-239b8db/profile-lag-loss/` |
| MP-3 dedicated 1v1 regression | PASS 23/23 | `final-239b8db/regression-dedicated-1v1/` |
| MP-1 listen 1v1 regression | PASS 18/18 | `final-239b8db/regression-listen-1v1/` |
| Offline PAC-10 and participant authority (`rts_interaction`) | PASS, first attempt each | `final-239b8db/offline-{pac10,authority}.out` |
| Automation, full `Politika` suite (editor built from `239b8db`) | PASS 36/36, exit 0, including the updated `Politika.Bootstrap.LaunchOptions` | `automation-all/` |
| Harness: `cargo fmt --check`, `cargo test --bin rts_multiplayer` | PASS; 6/6 unit tests, including the two new ones | — |

## E. Reproduction

```bash
export UNREAL_ENGINE_DIR=/mnt/data/unreal/5.8.3
Q=/mnt/data/politika-qualification/MP2-Engagement-20261029
bash Tools/Shell/package-multiplayer.sh development $Q/dev-239b8db
bash Tools/Shell/package-multiplayer.sh server      $Q/server-239b8db
S=$Q/server-239b8db/LinuxServer/Politika/Binaries/Linux/PolitikaServer
C=$Q/dev-239b8db/Linux/Politika/Binaries/Linux/Politika
cargo run --locked --release --manifest-path Tools/Shell/Cargo.toml --bin rts_multiplayer -- \
  --engagement $S $C --evidence <DIR> --port 18910
```

`$Q/run-matrix.sh` runs the packaged matrix in §D sequentially, using the harness binaries copied next to it.

## F. Limitations, findings and next work

- **F43 (new, open): the facing-only endpoint can land near the hidden target.** A facing-only cue is drawn to a fixed point 600 cm along the shooter's facing (`ShotFacingCueDistanceCm`). The rifle's range is 650 cm, and an Attack approach stops 100 cm inside range, so the most common engagement distance is 550 cm. At that distance the endpoint lies about 50 cm past the hidden target, plus at most about 120 cm sideways from the 11.5° aim tolerance. The opponent then learns roughly where the target is, not just its bearing. The fixture deliberately fires from 301 cm, so its facing endpoints sit 299–310 cm from the target and the harness's 100 cm leak check is meaningful. At 550 cm that check would fail. A likely fix is an endpoint that can never coincide with a legal target, such as a short muzzle stub or a point beyond the maximum weapon range. That changes the opponent's presentation and needs an owner decision; tracked in the [audit](Reports/RTS_MULTIPLAYER_CODE_AUDIT_20261029.md).
- **Evidence limits.** Loopback on one Linux host; Xvfb with virtual input; Development builds; one geometry per cue type; stationary, non-retaliating targets.
- **Still open for MP-2:** F43, listen-hosted 3–4 process runs, AI-filled seats, unit-scale bandwidth/step budgets, F35 (disconnect policy beyond ending the match).
