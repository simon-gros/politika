# MP-2 — Viewer fog shading and last-known contacts: acceptance record

**Date:** 29 October 2026 · **Scope:** the third MP-2 increment of [Multiplayer Architecture](RTS_MULTIPLAYER_ARCHITECTURE.md) §9, on top of the [multi-client layouts](RTS_MULTIPLAYER_MP2_MULTICLIENT_ACCEPTANCE.md) and [network conditions](RTS_MULTIPLAYER_MP2_NETWORK_CONDITIONS_ACCEPTANCE.md) increments. It closes MP-0 audit finding **F33** (clients had no fog overlay and no last-seen contacts; hidden units simply vanished). [Visibility and Intelligence](RTS_VISIBILITY_AND_INTELLIGENCE.md) §2 owns the rules followed here: currently visible cells are separate from explored terrain, a unit that leaves sight becomes a last-known contact, and a death outside sight is not revealed.

**Outcome: IMPLEMENTED; packaged qualification PASS for the stated scope.**
- Every rendering process of a networked session shades unexplored, explored and visible ground from its own team's vision, and draws a "LAST SEEN #id  Ns" marker where a non-friendly unit was when it left sight.
- Both come only from what that process already legitimately holds. The server still decides alone what each connection receives; nothing new is replicated and `network_protocol` stays **2**.
- One gameplay-visible server change: in a networked session a dead unit stays replicated for 1.5 s (hidden, no collision) before the actor is destroyed, so every viewer that saw the death receives it.

Not qualified here:
- **Fog for offline play.** Standalone matches keep full vision, as before.
- **Shipping builds.** All packaged runs are Development (the evidence lines are Development-only logs). The presentation code itself is not Development-only.
- **Listen host over sockets.** The listen-host path (local fog transitions) is covered by Automation in a real listen-mode world, not by a packaged listen run that checks contacts. The packaged listen regression below only confirms nothing else changed.
- **Uncertainty, reports, deception, terrain line of sight, structures' remembered silhouettes, a minimap.** The [visibility contract](RTS_VISIBILITY_AND_INTELLIGENCE.md) keeps these proposed.
- **Visual design.** The overlay is a placeholder HUD treatment (flat dark shading, a hollow diamond), not final art.

## A. What changed

- **Portable rules — `Source/PolitikaCore/Fog.h`.**
  - `FogMap`: a fixed grid with `Unexplored` / `Explored` / `Visible` cells. `Update(team, sources)` marks cells whose centre is inside one of the viewer team's own sight circles (the same inclusive rule as server unit vision), and turns previously visible cells into explored. Other teams' sources are ignored; an unseated viewer sees nothing.
  - `ContactMemory`: `Withdrawn(contact, last_known_alive)` records or updates a contact only if it was last known alive; if the viewer saw it die, any memory is forgotten. `Reobserved(entity)` drops it when the live unit is received again. `ClearRevealed(team, sources)` drops contacts whose remembered position is clearly inside the viewer's own sight, using the sight radius minus `kContactRevealMarginCm` = 150 cm. The margin absorbs replication delay: the server withdraws a contact when its *current* position leaves sight, so its last replicated position can still be just inside the radius.
- **Where the memory is fed (`APolitikaPrototypeMatch`, never replicated).**
  - **Client.** A unit actor's `EndPlay` (reason `Destroyed`, which covers both relevancy loss and replicated destruction) calls `NoteLocalContactWithdrawn` with the last replicated position and "alive" = `IsAlive() && !bDeathPresented`. `BeginPlay` calls `NoteLocalContactObserved`.
  - **Listen host.** `ApplyLocalFogPresentation` now uses `APolitikaPrototypeUnit::ApplyLocalFogVisibility`, which reports a change only after the unit's first application, so the fogged start is not a sighting. A unit that becomes hidden is remembered at `GetLastLocalFogVisibleLocation()` (the last position the host saw), never at its hidden position.
  - Only non-friendly contacts are remembered, and only while the match is `Active`. Leaving `Active` clears contacts (results teardown withdraws every unit; that is not a sighting); a new match generation also resets the explored map.
  - `UpdateLocalFogKnowledge` runs from `Tick` at 10 Hz on clients and the listen host. It is inactive offline, on a dedicated server and for an unseated viewer (`IsLocalFogPresentationActive`).
- **Grid.** 125 cm cells over the map's playable bounds plus 3500 cm (88 × 88 cells for the fixture).
- **HUD (`APolitikaPrototypeHUD`).** `DrawLocalFog` draws translucent canvas triangles with per-vertex darkness (explored 0.45, unexplored 0.78, averaged over adjacent cells so edges are soft) and a radial skirt from the grid edge outward so terrain beyond the grid reads as unexplored. `DrawLastKnownContacts` draws the marker and age. Both draw before labels and HUD text. Markers are not actors, so clicks and right-clicks cannot select or target them; orders still require a currently observed entity (`TargetNotVisible`).
- **Networked death retirement.** `APolitikaPrototypeUnit::BeginDeathPresentation`: a networked authority no longer destroys a non-animated dead unit in the same frame. It hides it, disables collision and sets a 1.5 s life span (`NetworkDeathRetireSeconds`). Without this, a client could receive the destruction before `Health = 0`, and a death in plain sight would look like a withdrawal. Infantry with death animations already lived on as corpses. Standalone play is unchanged.
- **Client sight radius.** `ApplyObservedArchetypePresentation` now copies the archetype's `SightRadiusCm` to the client's unit copies, so own-team shading uses the authored radius.
- **Development evidence lines (`!UE_BUILD_SHIPPING`).** `MP_LOCAL_FOG` (once a second: visible/explored/unexplored counts and contacts), `MP_LOCAL_CONTACT_LAST_KNOWN`, `MP_LOCAL_CONTACT_CLEARED reason=reobserved|revealed|died|reset`. `MP_CLIENT_UNIT_WITHDRAWN` now also logs the position.

## B. Source changes (on `main`: implementation `bc6ebc8`, harness fix `50c929f`)

| Area | Files | Change |
|---|---|---|
| Portable rules | `Source/PolitikaCore/Fog.h`, `Tests/Core/RulesTests.cpp` | `FogMap`, `ContactMemory`, `ContactClearReason`; grid, explored persistence, margin, dead/alive withdrawal tests. |
| Runtime | `Source/Politika/Private/PolitikaFogPresentation.cpp` (new), `PolitikaMultiplayer.cpp`, `PolitikaPrototypeGameplay.{h,cpp}` | Local knowledge update, client/host hooks, HUD drawing, networked death retirement, client sight radius. |
| Automation | `Private/Tests/PolitikaMultiplayerTests.cpp` | `Politika.RTS.Multiplayer.FogMemoryAndShading`. |
| Harness | `Tools/Shell/src/bin/rts_multiplayer.rs` | `--fog-memory` session, `contact_memory_audit` (+4 unit tests), per-layout `FOG-MEMORY-NO-LEAK` check in `--dedicated-layouts`. `50c929f`: `wait_line` matches only complete log lines. |

## C. How it is checked

**`Politika.RTS.Multiplayer.FogMemoryAndShading`** (real listen-mode `IpNetDriver` world, 1v1): the fogged start records nothing; own ground is visible and unseen ground unexplored; an enemy seen at the edge of sight and then moved away is remembered at its last *visible* x, not its hidden position; former sight becomes explored; the reobserved enemy clears the memory; a contact whose spot is clearly in sight is recorded and then cleared as revealed; an enemy that dies while hidden stays remembered; a never-seen Unaligned unit is never remembered; leaving `Active` clears contacts but keeps explored terrain; a new generation resets the map; standalone presents nothing.

**`rts_multiplayer --fog-memory SERVER CLIENT`** (packaged headless server, rendered client A, null-RHI client B, 1v1 + Unaligned): client A, with real input, moves a unit toward the objective until the southern Unaligned militia (stationary, neutral to auto-engagement, outside team A's starting sight) is received; moves it back home; then forward again.

| Check | Pass condition |
|---|---|
| `MP2-FOG-SHADING-ACTIVE` | The militia was not received at the start; the first `MP_LOCAL_FOG` has visible and unexplored cells. |
| `MP2-FOG-CONTACT-SEEN` | The Move is admitted and the militia becomes relevant to client A. |
| `MP2-FOG-LAST-KNOWN` | The militia is withdrawn alive; the client records it (role client, viewer 1, owner 0) at exactly the withdrawn position, which matches the server's position for this stationary unit within 5 cm. |
| `MP2-FOG-EXPLORED-PERSISTS` | No explored cells at the start; after the retreat, explored > 0, unexplored decreased and one contact is held. |
| `MP2-FOG-REOBSERVED` | Advancing again clears the contact with `reason=reobserved`. |
| `MP2-FOG-MEMORY-NO-LEAK` | `contact_memory_audit`: every recorded contact follows a live withdrawal of the same entity at the same position after it was received, never names the viewer's side, and never follows a death the client saw. Client B, which never saw the militia, recorded none. |
| `MP2-FOG-SERVER-PRESENTS-NOTHING` | The dedicated server logged no viewer fog or contacts. |

**`--dedicated-layouts`** runs the same audit on every client of every layout as `FOG-MEMORY-NO-LEAK`. These matches include combat deaths in sight, so they check the death-retirement change under real replication.

## D. Verdicts

Vocabulary: PASS / FAIL / BLOCKED / INFRA_ERROR / INCONCLUSIVE / SKIPPED / NOT_RUN. Evidence root: `/mnt/data/politika-qualification/MP2-FogMemory-20261029/` (`final-bc6ebc8/` for packaged runs).

Package identity: `package-multiplayer.sh development` and `server` from `bc6ebc8`, UAT exit 0, `git_dirty=false`, `network_protocol` 2. Executable SHA-256: client `c3dd986b1a8ec54fa3a9b9c34e88afeea96472a0c2935b4ab5526480f9215bf6`, server `a7c8ac2e37286a3ae0fab474921c5424eacf66516e95eefca12d76f5f36740f5`. Harness: `rts_multiplayer` built from `bc6ebc8`, plus the `50c929f` rerun noted below.

| Gate | Result | Evidence |
|---|---|---|
| `quality -- core` (C++20, `-Werror`; gcc, and clang syntax check) | PASS | `logs/quality-core.log` |
| `PolitikaEditor` Linux Development build | PASS | — |
| Automation, full `Politika` suite | PASS 36/36, including the new `FogMemoryAndShading`. Warnings are the pre-existing environment and navigation-fixture warnings. | `automation-all/` |
| `fog-memory-run1`, `fog-memory-run2` | PASS 7/7 each. Client A recorded the militia (entity 6) at `x=0.0 y=-1050.0`, its exact withdrawn and server position; explored cells rose from 0 to 107 after the retreat; reobserve cleared it; client B and the server recorded nothing. | `final-bc6ebc8/fog-memory-run{1,2}/` |
| Layouts 2v1 / 3v1 / 2v2, profile `none` | PASS 74/74; `PROTOCOL-LEGACY-REFUSED` NOT_RUN (no RPC change). `FOG-MEMORY-NO-LEAK` PASS on all 11 clients: each saw a death in sight and none recorded it; 7 contacts recorded in normal play, all from live withdrawals. | `final-bc6ebc8/matrix-none/` |
| Layout 1v1, profile `none` | First attempt: 19 PASS, **1 FAIL** `MP2-1V1-SEATS-BY-JOIN-ORDER`. The harness read `MP_CLIENT_ASSIGNMENT participant=2 slot=2` while the line was still being written; the log holds the full line with the expected faction and generation. Harness fixed in `50c929f` (waits match only complete lines). Rerun on the same packages: PASS 20/20, `FOG-ALLIED-SHARED` and `PROTOCOL-LEGACY-REFUSED` NOT_RUN. | `final-bc6ebc8/matrix-none-1v1{,-rerun}/` |
| 1v1 + 2v2, profile `lag-loss` | PASS 47/47; the same two NOT_RUN. `FOG-MEMORY-NO-LEAK` PASS on all 6 clients under ~120 ms lag and 3 % loss: seen deaths still arrived before the 1.5 s retirement. | `final-bc6ebc8/profile-lag-loss/` |
| MP-3 dedicated 1v1 regression (`--dedicated`) | PASS 23/23 | `final-bc6ebc8/regression-dedicated-1v1/` |
| MP-1 listen 1v1 regression | PASS 18/18 | `final-bc6ebc8/regression-listen-1v1/` |
| Offline PA-AUTHORITY / PAC-10 (rendered, packaged Development) | PASS / PASS (first attempt each) | `final-bc6ebc8/offline-{authority,pac10}.out` |
| `quality -- audit`, version tests, `cargo fmt --check`, Shell tests | PASS (88 index links; harness unit tests 4/4 new + 10 existing) | `logs/quality-audit.log`, `logs/version-tests.log` |

An earlier package of the first implementation (`1ed3fa7`) passed `--fog-memory` 7/7 too. Its screenshots showed the terrain outside the 48 × 48 fog grid fully lit, so the grid was widened and the outer skirt added before the final commit; that run is kept as `fog-memory-1ed3fa7-run1/` and is not counted.


## E. Reproduction

```bash
export UNREAL_ENGINE_DIR=/mnt/data/unreal/5.8.3
Q=/mnt/data/politika-qualification/MP2-FogMemory-20261029
bash Tools/Shell/package-multiplayer.sh development $Q/dev-bc6ebc8
bash Tools/Shell/package-multiplayer.sh server      $Q/server-bc6ebc8
S=$Q/server-bc6ebc8/LinuxServer/Politika/Binaries/Linux/PolitikaServer
C=$Q/dev-bc6ebc8/Linux/Politika/Binaries/Linux/Politika
cargo run --locked --release --manifest-path Tools/Shell/Cargo.toml --bin rts_multiplayer -- \
  --fog-memory $S $C --evidence <DIR> --port 18510
```

`$Q/run-matrix.sh` runs the whole packaged matrix in §D sequentially with the harness binaries copied next to it.

## F. Limitations and next work

- **Evidence limits.** Loopback on one Linux host; Xvfb with virtual input; Development server and clients; one stationary contact in the packaged fog-memory session. The "revealed" (spot seen empty) and "death while hidden" paths are covered by core tests and Automation, not by a packaged run.
- **Position precision.** A client's last-known position is its last replicated position, up to one relevancy timeout (0.25 s) of movement behind where the unit actually left sight. That is information the client already had; nothing newer is revealed.
- **Still open for MP-2:** an Unaligned engagement fixture (facing-only and withheld shot cues over sockets), listen-hosted 3–4 process runs, AI-filled seats, unit-scale bandwidth/step budgets, F35 (disconnect policy beyond ending the match).
