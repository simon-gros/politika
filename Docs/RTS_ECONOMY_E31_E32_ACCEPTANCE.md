# ECON-E31 and ECON-E32 — A click prefers the unit, and a plant a seat can buy: acceptance record

**Date:** 3 November 2026 · **Scope:** two slices qualified together on one package: `e5275fb` (ECON-E31) and `ad68c0e` (ECON-E32), packaged from `0f58a9c`, which adds a harness correction and changes no package input.

**Outcome: IMPLEMENTED and ACCEPTED for the skirmish test map. The packaged `--plant` session passes 9/9 and `--oil` 12/12, each in two runs with identical traces, on real input with two rendered remote clients. Automation 63/63 with no failures. The regression matrix has one regression against ECON-E30: `MP2-FOG-EXPLORED-PERSISTS` in `default-fog-memory` went from PASS to FAIL. It failed with the same counts at ECON-E28, passed on these same sources earlier the same evening, and earlier records call it timing-dependent and not a gate (section B). A first full matrix on `ad68c0e` did not count, because the package had no build identity and two offline checks never ran; the owner chose a clean rerun over patching it. The plant's price and build time are placeholders.**

## A. What the slices added

**ECON-E31, a click prefers the unit.** A left-click that lands on a building now selects a unit standing in it or behind it.

- This fixes the idle harvester that could not be clicked where it parks at the producer's centre, recorded in [section D of the ECON-E29 and E30 record](RTS_ECONOMY_E29_E30_ACCEPTANCE.md).
- Only the left-click changes. C, Z and right-clicks still find the building.
- A click on a building with no unit in the way selects nothing, as before. The look-through logs `RTS_SELECTION_THROUGH_BUILDING`.

**ECON-E32, one conventional plant a seat can buy.**

- **P charges $400.00 once,** and the plant stands 20 s (400 steps) later on a fixed site across the producer from the first plant. No selection is needed.
- **It adds 100 kW of supply and its own 10 kW load,** so a base goes from 100 / 50 kW to 200 / 60 kW.
- **It is a generator like any other** for damage, repair and capture, on its own grid id.
- **Starting it takes a living producer and no power,** so a seat can buy its way out of an outage.
- **One a match, with no cancel and no refund.**
- **Its account view goes to the owner alone,** and the other seat does not receive the building while it is unseen. The HUD shows the price, then the time left, and then the kW figures say the rest.

`network_protocol` 25 to 26.

**Crude oil figures.** The owner confirmed the ECON-E30 figures on 3 November 2026: $1.50 a unit, a 3% fee, 400 units in a field and a load of 20. They are an owner decision now, not placeholders.

## B. What qualification found

| Finding | Cause | What was done |
|---|---|---|
| **`trial-1` (`ad68c0e`): `--plant` stopped at seat 2's site** | The harness's site was computed from rounded logs, and the check on the client's log compares one decimal | `0f58a9c` uses the observed (1906.2, 628.9). Harness only. `trial-2` passes 9/9 |
| **First full matrix (`final-ad68c0e/`): `offline-pac10` and `offline-authority` exit 1 with no checks** | The package had no `BuildIdentity.json`, so `rts_interaction` stopped before launching the game. The qualification script called UAT directly and skipped `Tools/Shell/package-multiplayer.sh`, which writes that file, `executable.sha256` and `unreal-version.env`. `rts_multiplayer` skips a missing identity silently, so its reports had none either | **That run does not count, and those two checks had neither passed nor failed.** The owner chose a clean rerun. Packages were rebuilt from clean `0f58a9c` with `package-multiplayer.sh`, and the qualification script now refuses to start unless both packages hold the three files with the right revision, `git_dirty=false` and protocol 26. The earlier run is kept as evidence |
| **The first matrix ended at 22:04 and nothing acted on it until the owner asked at 22:33** | The run's completion did not lead to any follow-up | Not a test failure. The rerun had a scheduled fallback check |
| **`default-fog-memory` exit 1: `MP2-FOG-EXPLORED-PERSISTS` FAIL, the comparer's only regression** | After the retreat the counts were (422, 200, 7122). ECON-E28 failed with the same counts, and ECON-E30 passed with (408, 214, 7122). `final-ad68c0e` passed with (404, 218, 7122), on the same `Source`, `Config` and `Content` as this package | Not a gate, as in the [ECON-E28 record](RTS_ECONOMY_E28_ACCEPTANCE.md). It was not rerun. The cause is inferred from these runs, not isolated, and **the check remains a real failure in this matrix** |
| **`listen-none` and `profile-lag-loss` exit 1** | The three expected `NOT_RUN` checks: `MP2-PROTOCOL-LEGACY-REFUSED` twice, and `MP2-1V1-LAG-LOSS-FOG-ALLIED-SHARED` | None. `matrix-none` covers the legacy refusal |
| **`origin/main` gained documentation commits during the rerun** | Seventeen commits, documentation only, from 23:02 to 23:44 | Merged, not rebased, so the tested `0f58a9c` stays in history. They change no package input |

## C. Verification

**Identity.** Packages from clean commit `0f58a9c` (`git_dirty=false`, `network_protocol` 26, engine 5.8.3). Client `dev-0f58a9c/…/Politika` `8e101023307672f6f3919e8db2e999a4d9b3fd8c53eb56957c35f7c8f4956bb6`; server `server-0f58a9c/…/PolitikaServer` `db228e135dad8f7f14401592ac9e09c5c81e04c46ac3d8687baf6e26fe7e8517`. Each session's evidence records that identity. Evidence is in `/mnt/data/politika-qualification/ECON-E32-20261103/`: `final-0f58a9c/`, `auto-final-0f58a9c/`, `compare-0f58a9c.txt`.

| Check | Result |
|---|---|
| Portable C++20 suite (`tools/core-test.sh`), at `0f58a9c` | PASS |
| `cargo fmt --check` (Tools/Shell); Tools/Shell tests (26: 14 rts_multiplayer, 12 rts_interaction); `quality -- core`; `quality -- audit` | PASS |
| `tools/version`: `check`, `cargo test --all-targets` (7 and 14), `cargo fmt --check` | PASS; reports protocol 26 |
| Automation, all Politika tests (`auto-final-0f58a9c/`) | 63/63, 0 failures, 0 not run, with the new `Plant`. All 63 finish with warnings, as ECON-E30's 62 did. The editor library was built at `ad68c0e`; `0f58a9c` changes no source |
| `--plant` ×2 | 9/9 twice, identical `plant-trace.txt` |
| `--oil` ×2 | 12/12 twice, identical `oil-trace.txt` |
| `--works` ×2, `--buildings` ×2 | 12/12 and 20/20 twice, each with identical traces |
| `--harvest` ×2, `--gold` ×2, `--food` ×2, `--power` ×2, `--economy` ×2 | 14/14, 14/14, 13/13, 12/12, 16/16 twice, each with identical traces |
| `--economy-limits` ×2, `--pointer-stall` ×2 | 5/5 and 25/25 twice |
| `--group-move-facing` ×2, `--group-move-shape` ×2 | 54/54 and 47/47 twice |
| Offline `PAC-10` and `PA-AUTHORITY` (`rts_interaction`) | PASS, on this package |
| `--unit-scale` | 83/83, including every step-budget check: at 3v1 with 20 a seat, engage p99 1.4 ms and longest step 2.1 ms against the 50 ms budget, no catch-up frames |
| Launch options | 0 sessions flag `InvalidLaunchOptions` |
| Whole matrix against ECON-E30 (`compare-matrix.py`, `compare-0f58a9c.txt`) | 1 regression, `default-fog-memory`, as in section B. `--oil` gained `OIL-P10` and `OIL-P11`, and `--plant` is new. The three expected `NOT_RUN` checks are unchanged |

**The `--plant` session.**

| Check | What it shows |
|---|---|
| `PLT-P00-LAUNCH-OPTIONS` | Valid launch options on the server and both clients |
| `PLT-P01-A-BASE-STARTS-ON-ONE-PLANT` | A seat starts on its first plant alone |
| `PLT-P02-P-CHARGES-ONCE-AND-STARTS-THE-BUILD` | P debits 40000 cents once, by real input |
| `PLT-P03-IT-STANDS-AFTER-ITS-BUILD-AND-THE-SUPPLY-DOUBLES` | It stands 400 steps later at (1906.2, 628.9), at 200 / 60 kW |
| `PLT-P04-ONE-A-MATCH` | A second P is refused with `plant_already_bought`, and there is 1 charge |
| `PLT-P05-OWNER-ONLY` | The other client receives no view of it and no enemy structure |
| `PLT-P06-THE-FIRST-PLANT-DIES-AND-THE-PRODUCER-KEEPS-RUNNING` | With the first plant destroyed, the bought plant carries the base |
| `PLT-P07-WITH-BOTH-DOWN-THE-PRODUCER-STOPS` | With both down, supply is 0 kW and the producer is unpowered |
| `PLT-P08-LEDGERS-WHOLE-AND-THE-NEXT-MATCH-HAS-NO-PLANT` | The ledgers reconcile, and the next generation starts without a plant |

**The click.** `OIL-P11` selects the parked harvester with one click and logs `RTS_SELECTION_THROUGH_BUILDING`. `OIL-P10` is its control: a click on the building alone selects nothing. `--oil` still needs no box select.

**Tempest check.** The AGENTS rule was applied to the plant. The reference registry holds only names, such as `TedStructureDefinitionAsset` power plants and `ActorTag.SP_GDF_M09.DestroyPowerPlants`, with no values. It supports the concept of a destructible power structure and nothing else, so no value, name or asset was taken from it. Confidence is low, and the gap is every POLITIKA number, which stay placeholders.

## D. Decisions for the owner

- **$400.00 and 20 s for the plant are placeholders.** So are its 100 kW and 10 kW, which match the first plant's figures.
- **The site is fixed, and a player does not place the plant.** Placement is separate work.
- **A parked harvester is still hidden.** It stands inside the producer's 3 m cube, so only its label shows. The click fixes selection, not that. Parking it elsewhere remains open.
- **The oil field's position is still a fixture-map result,** and crude still has no use except sale.

## E. Map to the acceptance gates

| ID | Status in this increment |
|---|---|
| USD-01 | Met for the plant: it settles once, and a refused second purchase does not charge. Insufficient funds is covered by the rules suite only, not by Automation or `--plant` |
| PWR-02 | Partial: losing one of two plants leaves the producer running, and losing both stops it |
| PWR-03 | Partial, as before: the bought plant's damage is exercised by real input, and its capture and repair by the rules suite and Automation only. Reconnection during an outage is not exercised |

## F. Not covered

- **ECON-E31 has no Automation,** because the fixture world has no physics scene. It is proven by `OIL-P10` and `OIL-P11` only: other idle harvesters, other zoom levels and other aim points were not checked.
- A packaged purchase with too little money, and a packaged capture or repair of the bought plant.
- **A purchase during an outage is not tested anywhere.** `BuildPlant` has no power check in the code, and that is the only basis for saying a seat can buy its way out.
- The same play under packet loss or latency.
- The six navigation harnesses, which were not rerun, as in ECON-E5 to E30.
- Multiple-machine play, Shipping builds, a real desktop display and platforms other than Linux.
