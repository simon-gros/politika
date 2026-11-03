# ECON-E6 to E26 — Building damage, capture, demolition and repair: acceptance record

**Date:** 3 November 2026 · **Scope:** qualification of 21 small slices (commits `2e62886` to `32511a5`) that were pushed with focused in-engine tests only and no packaged run or record, plus the fixes that qualification needed. Owner decision of 3 November 2026: pause new HUD slices until this work is qualified.

**Outcome: IMPLEMENTED and ACCEPTED for the skirmish test map, after fixes. As first pushed, attacking, capturing, demolishing and repairing a building did not work from a remote client at all. New packaged session 18/18 in two runs with identical traces, real input on two rendered remote clients; Automation 58/58 with no failures; regression matrix on a quiet host matches the accepted [ECON-E5](RTS_ECONOMY_E5_ACCEPTANCE.md) matrix with 0 regressions. The balance figures are still prototype values, and [repair is free and instant](#d-decisions-for-the-owner).**

## A. What the slices added

- **Health** (E6, E13). A producer has 200 and a generator 100. One point of damage takes a generator offline until it is repaired to full. A destroyed building draws no power.
- **Attack** (E7). A right-click on a hostile building is an ordinary attack order. The selected units walk into weapon range and keep firing.
- **Fog** (E8). A hostile building reaches a client only while that team can see it.
- **Capture** (E9, E10, E11, E16, E18). **C** walks the nearest selected armed unit to within 2 m, then holds for 3 s. A hit on the unit or leaving cancels it. A captured building takes the new owner's colour. A producer is refused while the capturer has a living one, and capturing it refunds its old queue once.
- **Demolition** (E12). **Z** destroys your own building at once. A second press, an enemy building and a resource are refused.
- **What a destroyed building leaves** (E17, E20). A producer leaves the map. A generator husk stays and can be repaired.
- **Harvesters** (E20, E21, E22). A harvester delivers only to a living producer. Without one it keeps its load and goes to the next one you get. A failed trip remembers that producer.
- **Reading it** (E14, E15, E19, E23 to E26). The cursor names the building under it, and cargo reads on the way, holding, unloading or store full.

`network_protocol` went from 14 to 21 across these slices. E12 added a replicated `Health` and a server call while keeping the protocol at 16. Later bumps cover that, so it needs no fix unless a build from E12 to E15 was kept.

## B. What qualification found

| Finding | Cause | Fix |
|---|---|---|
| **On a remote client, Z, C and right-click attack did nothing, and the cursor named no building** | `Configure` enabled building collision on the authority only. Collision does not replicate, so a client's cursor went straight through every building | A building enables cursor hits on a client too (`25076ba`) |
| **On a remote client, repair could not work** | The click called the authority-only repair function directly, and no server call existed | `ServerRepairBuilding`, which checks ownership (`25076ba`) |
| **A client would have read every plant as offline** | The cursor line read the ledger, which exists only on the authority | A generator replicates whether it is generating (`25076ba`) |
| **The first fix broke two accepted sessions** (32 checks) | Once a client's cursor could meet a building, a right-click over your own building was refused instead of moving, and a drag ending over one took its roof as the ground, so the plan placed nobody | A click on your own building, other than a damaged generator, is a move to the ground under it. Drags and attack-moves skip buildings when finding the ground (`dfbd4d0`) |
| **`H-A09` failed in Automation** | Test setup: the hauler was moved before its first order had been admitted and failed. That order then ended in Stop, which left it waiting instead of loading | The check waits for that order to fail, and takes the ration issue window after loading (`32511a5`) |
| **The ground probe misled a harness** | `MP_CLIENT_GROUND_PROBE` traced to a roof once clients had collision, which threw a facing drag about 290 cm off. This was a diagnostic fault, not a game fault | The probe skips buildings (`40116e6`) |

Also changed: **C** and **Z** moved from hardcoded keys to `DefaultInput.ini`, the help line names them, and the repair hint now says what it takes, a selected unit and a right-click. `network_protocol` 21 to 22. New diagnostics are `RTS_BUILDING_CURSOR` and `RTS_STRUCTURE_WITHDRAWN`.

**Negative control.** `--buildings` on the packages built from `654bcaf` plus the then-uncommitted E26 work (`neg/run/`): `BLD-P02`, `P03`, `P06` and `P07` fail, because Z, C and the attack right-click reach no server code, and the session stops at the attack. `BLD-P04` fails too, but Z had not damaged the generator there, so that run does not test repair by itself; the missing server call is visible in the code. The same run passes the walk into the enemy base and fog relevance, so the session itself was working.

## C. Verification

**Identity.** Packages from clean commit `40116e6` (`git_dirty=false`, `network_protocol` 22, engine 5.8.3). Client `dev-40116e6/…/Politika` `a2a58aa0bee7a63927ae3d5ee081940197ed68c67d3cb0b33da439e17d1b8181`; server `server-40116e6/…/PolitikaServer` `4bf976b62eec83b6fee9cf0bb53f94342a574eadfd1f0391e89ae0b7f59345ec`. Evidence directory: `/mnt/data/politika-qualification/QUAL-E6-E26-20261103/` (local to the development machine). The first matrix, with its regressions, is kept as `final-25076ba/`. The CPU and GPU mining services were off for the whole matrix.

| Check | Result |
|---|---|
| Portable C++20 suite (`tools/core-test.sh`) | PASS |
| `cargo fmt --check` (Tools/Shell); Tools/Shell tests (26: 14 rts_multiplayer, 12 rts_interaction); `quality -- core`; `quality -- audit` | PASS |
| Automation, all Politika tests (`auto-final/`) | 58/58, 0 failures, 0 not run. All 58 report the same engine log warnings as ECON-E5's 53 |
| `--buildings` ×2 (`final-40116e6/`) | 18/18 twice, identical `buildings-trace.txt` |
| `--harvest` ×2, `--gold` ×2, `--food` ×2, `--power` ×2, `--economy` ×2 | 14/14, 14/14, 13/13, 12/12, 16/16 twice, each trace identical between runs and to the ECON-E5 baseline |
| `--economy-limits` ×2, `--pointer-stall` ×2 | 5/5 and 25/25 twice |
| `--group-move-facing` ×2, `--group-move-shape` ×2 | 54/54 and 47/47 twice |
| `--unit-scale` | 83/83: worst engage-phase step p99 1.5 ms against the 50 ms budget, longest step 2.7 ms, no catch-up frames |
| Launch options | 0 of 294 logs flag `InvalidLaunchOptions` |
| Whole matrix against ECON-E5 (`compare-matrix.py`, `compare-40116e6.txt`) | 0 regressions, including the three expected `NOT_RUN` checks |

`--fog-memory` with the free harvester on (`POLITIKA_HARNESS_DEFAULT_FORCES=1`) stays 7/8, as ECON-E5 recorded, and is not a gate. It passed 8/8 on the first matrix, so it still depends on timing.

**The `--buildings` session, by check.** A dedicated 1v1 from the normal start, free harvesters on. Team B holds its fire by a real **H**, and team A's soldier walks a corridor clear of the objective and both militia, so nothing else ends the match or damages it.

| Check | What it shows |
|---|---|
| `BLD-P01-EACH-CLIENT-HAS-ITS-OWN-BUILDINGS` | Under fog each client starts with its own two |
| `BLD-P02-THE-CURSOR-NAMES-A-BUILDING` | The cursor reads `Your generator  100 / 100  ·  online` on a remote client |
| `BLD-P03-Z-DEMOLISHES-YOUR-GENERATOR` | Z destroys it, and training is refused as `producer_unpowered` |
| `BLD-P04-A-RIGHT-CLICK-REPAIRS-YOUR-GENERATOR` | A right-click restores it to 100, and training is accepted again |
| `BLD-P17-A-RIGHT-CLICK-ON-YOUR-BUILDING-IS-A-MOVE` | On a healthy building of yours, the unit moves |
| `BLD-P05-A-SEEN-ENEMY-BUILDING-ARRIVES` | The enemy buildings reach the client when its soldier sees them |
| `BLD-P06-CAPTURE-RULES-HOLD-ON-THE-SERVER` | Z on an enemy building and C on a producer with your own alive do nothing; C walks in, and a move order during the hold cancels it |
| `BLD-P07-A-RIGHT-CLICK-ATTACKS-A-BUILDING` | The server admits an attack order on the building |
| `BLD-P08-THE-GENERATOR-GOES-DOWN-FOR-ITS-OWNER` | The first hit takes it offline, it falls to 0, and its owner's client reads that and is refused training |
| `BLD-P09-ITS-OWNER-REPAIRS-IT` | The other seat repairs its husk and queues two units |
| `BLD-P10-C-CAPTURES-THE-GENERATOR` | After the hold it changes hands. The new owner's cursor reads `Your generator`, and the old owner loses power |
| `BLD-P11-Z-TAKES-YOUR-PRODUCER-OFF-THE-MAP` | It refunds its queue, leaves the map on the client, a second press does nothing, and training is refused as `no_producer` |
| `BLD-P12-THE-CAPTURED-PRODUCER-WORKS` | Capturing refunds the old owner's queue, and a unit trains there |
| `BLD-P16-THE-HARVESTER-DELIVERS-TO-THE-CAPTURED-PRODUCER` | The free harvester goes on delivering after its own producer is gone |
| `BLD-P13-FIRE-DESTROYS-A-PRODUCER-WITH-ONE-REFUND` | Another seat's fire destroys it with a queue on it: one refund, and gone on both clients |
| `BLD-P14-AMMUNITION-STAYS-IN-RANGE` | Firing at a building never takes ammunition below zero |
| `BLD-P15-LEDGERS-WHOLE-AND-THE-NEXT-MATCH-FRESH` | Both ledgers reconcile at the end, and the next generation starts powered with no building damage |
| `BLD-P00-LAUNCH-OPTIONS` | Every switch the session passes is recognized |

Screenshots in each run's directory show the husk with its hint and the refusal on its owner's client, the captured generator, and the captured producer.

## D. Decisions for the owner

- **Repair is free, instant and works from anywhere.** One right-click undoes any damage while the generator's owner has a unit selected, so an attack on a generator stops training only until its owner notices. Real play can now cause an outage, but it is a short one. A cost, a time or a repairing unit would each make it matter more.
- **One soldier destroys a building quickly.** At 8 damage every 0.2 s, a generator lasts about 2.5 s and a producer about 5 s. These are prototype figures, not balance decisions.
- **You cannot capture a producer while you have one.** That rule came with E9. In the session, team A had to demolish its own producer first.
- **Z has no confirmation.** It destroys your building on the first press.

## E. Map to the acceptance gates

| ID | Status in this increment |
|---|---|
| PWR-03 | Partial: damage, capture and repair recover deterministically by real input. Reconnection during an outage is not exercised, and neither is a seat that has lost every building and unit |
| PRD-08 | Partial: producer death and capture refund once, and cancellation was already covered. A bad spawn and a blocked route are unchanged by this work |
| PRD-12 | Partial: each link runs in a rendered package, and combat now reaches buildings, but no one session plays the whole chain |
| ECO-FX-04 | Partial for a real outage: training is refused and accepted again by real damage and repair. Resuming the same queue entry is shown by `--power` with the fixture outage, and by Automation |
| ECO-INT-02 | Partial: the HUD's building line on two clients. No opponent AI exists yet |

## F. Not covered

- A capture cancelled by a hit on the unit, a harvester's failed trip, and a full food store reading `store full` have Automation only. No packaged session plays them.
- A listen host playing these features. Automation runs on that path and the listen regression sessions pass, but they never press C or Z.
- The same features under packet loss or latency, or with more than two seats.
- C, Z and repair outside the Active phase.
- Selecting a unit standing inside a building. Buildings do not block movement, and a click meets whichever comes first.
- The six navigation harnesses, which were not rerun, as in ECON-E5.
- Multiple-machine play, Shipping builds, a real desktop display and platforms other than Linux.
