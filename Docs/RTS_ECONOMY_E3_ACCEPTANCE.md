# ECON-E3 — Military food: acceptance record

**Date:** 1 November 2026 · **Scope:** the third sub-slice of Stage 4B in the [Engine Outline](ENGINE_OUTLINE.md): a food reserve for each participant, upkeep for the living force, a dollar purchase and a recoverable shortage, as designed in §5 of [Operational Economy and Power](RTS_OPERATIONAL_ECONOMY_AND_POWER.md). It builds on [ECON-E1](RTS_ECONOMY_E1_ACCEPTANCE.md) and [ECON-E2](RTS_ECONOMY_E2_ACCEPTANCE.md).

**Outcome: IMPLEMENTED and ACCEPTED for the skirmish test map. Packaged food check 12/12 in two runs with identical normalized traces, by real input on a dedicated 1v1; Automation 51/51 with no failures; regression matrix on a quiet host matches the accepted ECON-E2 matrix with no differences. Stage 4B is not complete: a source/delivery route and the commodity sale stay open.**

## A. What changed (commit `5c526ba`)

- **Reserve** (`Source/PolitikaCore/Economy.h`). Each account holds integer rations with its own reconciliation: starting + bought = held + issued, never negative. The reserve is separate from money and is not a spending currency.
- **Upkeep.** Every 200 steps the account owes one ration per living unit it owns. The debit happens once per due step and stops at zero. Unaligned units belong to no account and owe nothing.
- **Purchase.** **N** buys 50 rations for $50.00. It debits the dollars and credits the rations in one step, or changes nothing: too little money, a full store (capacity 1,000), a malformed request and a repeated request all leave both balances alone. The request carries no price or quantity. It goes through the same intake, sequence check and step as training, as a new `BuyFood` operation.
- **Shortage.** While a due debit could not be paid in full, new training is refused as `food_shortage` and the queue holds with no charge. Cancel still refunds, the purchase stays open, and units are not harmed.
- **Recovery.** The next due debit paid in full ends the shortage, and production resumes in that same step on the same entry.
- **Fixture figures** (prototype data, not balance decisions). 240 rations at the start, about 20 minutes for the 2-unit opening force and 2 minutes at the 20-unit limit.
- **View and HUD.** The owner-only view gains reserve, upkeep and shortage. A new HUD line reads `FOOD 240  -2 / 10 s  ·  about 20 min  ·  N buy 50 for $50.00` and turns red during a shortage, and the training text says `PAUSED: food shortage`.
- **Network and diagnostics.** `network_protocol` 11 to 12. `RTS_FOOD_START`, `RTS_FOOD_TX`, new fields on `RTS_ECONOMY_VIEW`, and a `food` kind on `RTS_ECONOMY_TX`.
- **Fixture.** `-PolitikaMPEconomyStartFood=<participant>:<rations>` (non-Shipping).
- **Tests.** Portable cases in `Tests/Core/RulesTests.cpp`, Automation `Politika.RTS.Multiplayer.Food`, and the `--food` packaged session.

## B. Decisions taken

The design left the penalty and the boundary open. These are easy to change, and the first is a gameplay choice worth reviewing in play.

- **The penalty stops reinforcement.** The design asks for one limited penalty, and names reduced reinforcement or readiness as examples. This slice refuses new training and holds the queue. Combat, movement and orders are unaffected, so a short force can still fight, take the objective and reach a result.
- **The recovery boundary is the next due debit paid in full.** Buying food refills the reserve at once, but the shortage ends only at the next interval, up to 10 seconds later, so buying and spending in one instant does not skip the consequence.
- **An unpaid debit takes what is there and leaves no debt.** The stock floors at zero and the remainder is not carried forward.
- **Step order.** Requests, then income, power, food upkeep and production, inside the existing economy phase, after casualties and before the objective. The force is counted at that point: this step's casualties have stopped eating, and a unit that appears in this step first eats at the next interval. A purchase in a due step is available to that step's debit.
- **Every unit eats the same.** All current units are staffed infantry, so the rate is one constant. An authored per-archetype rate, including zero for an unmanned unit, waits until such a unit exists.
- **A full force can feed itself.** At $1.00 a ration and $20.00 of income per interval, income alone can feed the 20-unit limit, so a shortage is never a softlock. A portable static assertion holds this. A first figure of $2.00 a ration failed that test, as [section C](#c-verification) records.

## C. Verification

**Identity.** Packages from clean commit `5c526ba` (`git_dirty=false`, `network_protocol` 12, engine 5.8.3). Client `dev-5c526ba/…/Politika` `8d3d3b423a5ed1ddfbd46a22ada52d1e65c2619ddcec137617df5ecd915201ce`; server `server-5c526ba/…/PolitikaServer` `cbc237cf19443cd2392d8d7e390b949b7890d00999ddbb963292079ded45f0ef`. Evidence directory: `/mnt/data/politika-qualification/ECON-E3-20261101/` (local to the development machine). The CPU and GPU mining services were stopped for the whole matrix.

| Check | Result |
|---|---|
| Portable C++20 suite (`tools/core-test.sh`) | PASS |
| `cargo fmt --check` (Tools/Shell); Tools/Shell tests (24: 13 rts_multiplayer, 11 rts_interaction); `quality -- core`; `quality -- audit` | PASS |
| Automation, all Politika tests (`auto-all-final/`) | 51/51, 0 failures, 0 not run: 50 at ECON-E2 plus `Food`. Warnings match ECON-E2 apart from `Food`'s own `IsUsingWayland` notice |
| `--food` ×2 (`final-5c526ba/`) | 12/12 twice, identical `food-trace.txt`; a gate run on the same packages also 12/12 |
| `--power` ×2 | 11/11 twice, identical traces |
| `--economy` ×2 and `--economy-limits` ×2 | 15/15 and 5/5 twice, identical economy traces |
| `--pointer-stall` ×2 and `--group-move-facing` ×2 | 24/24 and 53/53 twice, normalized plans byte-identical |
| `--unit-scale` | 73/73: worst engage-phase step p99 1.8 ms against the 50 ms budget, longest step 2.4 ms, no catch-up frames |
| Whole matrix against ECON-E2 (`compare-matrix.py`) | 0 differences, including the three expected `NOT_RUN` checks and the protocol-11 client refused in `matrix-none` |

**The `--food` session, by check.** Seat 1 starts with 2 rations through the fixture and seat 2 with the normal 240.

| Check | What it shows |
|---|---|
| `FOOD-P01-RESERVES-AT-THE-START` | 2 rations for seat 1 and 240 for seat 2 |
| `FOOD-P02-DUE-UPKEEP-CHARGED-ONCE` | Step 200 owes 2, pays 2 and empties the reserve with no shortage |
| `FOOD-P03-SHORTAGE-STOPS-AT-ZERO` | Step 400 owes 3, because the trained unit has joined, pays 0 and sets the shortage with the stock at zero |
| `FOOD-P04-SHORTAGE-REFUSES-TRAINING` | A real B press is refused as `food_shortage` and charges nothing |
| `FOOD-P05-PURCHASE-OPEN-IN-A-SHORTAGE` | A real N press debits $50.00 and credits 50 rations, and the shortage stays until the next issue |
| `FOOD-P06-NEXT-FULL-ISSUE-ENDS-IT` | Step 600 owes and pays 3, leaving 47, and the same entry resumes with the same remaining time |
| `FOOD-P07-HELD-UNIT-LATE-BY-THE-SHORTAGE` | The second unit appears 600 steps after the first purchase: 400 of training and 200 held |
| `FOOD-P08-TRAINING-WORKS-AFTER-RECOVERY` | A later purchase is admitted and the ledger reconciles |
| `FOOD-P09-NO-HARM-AND-OWNER-ONLY` | No unit is destroyed, seat 2 is never short, and neither client receives the other's view |
| `FOOD-P10-RESERVES-REPLAY` | Both seats' reserves follow exactly from their upkeep and purchase lines, on an exact 200-step cadence |
| `FOOD-P11-MONEY-REPLAYS` | The money ledger replays with exactly one food purchase |
| `FOOD-P12-NEXT-GENERATION-FRESH` | After a match end the next generation starts from its own reserves with no food lines |

Screenshots `client-a-02-shortage.png`, `-03-refused.png`, `-04-bought.png` and `-05-recovered.png` show the red food line, the paused training text, the refusal, the purchase and the recovered runway. Automation also covers what the packaged session does not: a casualty stops eating, a malformed purchase changes nothing, and each seat's view is its own. The portable suite covers too little money and a full store.

**Failed attempts.** Automation `auto-1` failed two `Economy` assertions: that test held 19 units for 20 intervals to collect money, which ran the force out of rations and correctly refused its training. That exposed a real problem with the first price, $2.00 a ration: a 19-unit force ate $38.00 per interval on $20.00 of income, so income alone could never end a shortage. The price became $1.00 a ration, the portable assertion was added, and the test now collects its money before adding the units. All of this came before the feature commit.

## D. Map to the acceptance gates

| ID | Status in this increment |
|---|---|
| FOOD-01 | Met for fixture data: positive upkeep, an opening runway and a real dollar-funded route |
| FOOD-02 | Met: atomic, capacity-limited and refused on a repeated request. Insufficient money and a full store are shown in the portable suite, not over the network |
| FOOD-03 | Met for one charge per interval, idle units, casualties (Automation), rematch and seat isolation. Capture does not exist |
| FOOD-04 | Met: an observable limited penalty, and replenishment restores function with no unit loss |
| PRD-14 | Met for the fixture figures |
| ECO-FX-05 | Met |
| ECO-INT-01 | Partial: two packaged runs give identical normalized traces on one machine. Different render schedules were not compared directly |

## E. Not covered

- Auto-buy, delayed delivery and a food procurement panel. N buys one fixed pack at once.
- Per-archetype or unmanned rates, and any balance review of the prototype figures.
- A source/delivery route and the commodity sale, which remain open for Stage 4B.
- The opponent AI's use of food, since no opponent AI exists yet.
- Multiple-machine play, Shipping builds, a real desktop display and platforms other than Linux.
