# POLITIKA — Dollar Economy, Commodity Sales, Building Power and Military Food

**Design decision adopted 29 October 2026; implementation NOT RUN.** POLITIKA's primary purchasing resource is **in-game US dollars ($, USD)**, not a separate Operational Budget currency. Acquired gold and eligible fossil fuels can be sold in exchange for dollars. **Operational buildings must meet positive minimum electricity demand; staffed military forces must maintain food reserves, which players can purchase with dollars.** Acquisition, exchange, generation and upkeep values remain unspecified authoring/balance parameters. These rules supplement the [Resource Economy](RESOURCE_ECONOMY.md), [Production](RTS_PRODUCTION_AND_TECHNOLOGY.md), [Engine Outline](ENGINE_OUTLINE.md), [UI](RTS_UI_SETTINGS_AND_KEYMAP.md), and [Visibility](RTS_VISIBILITY_AND_INTELLIGENCE.md). No new code/Unreal assets are implied.

## 1. Resource taxonomy and strategy

| Resource | Model | Acquisition and expenditure |
| --- | --- | --- |
| **US dollars ($)** | One liquid **purchasing currency**, stored as nonnegative integer cents | Initial treasury, baseline and eligible industry/territory revenue, contracts, missions, authorized commodity sales; spent on units, structures, food, repairs, technology and eligible covert/political actions |
| **Gold** | Physical, non-currency saleable inventory | Gold deposits, bank vaults, shipments, negotiated stock or legitimate captured reserves; sell held eligible quantity for USD or reserve for an authored contract |
| **Crude oil / natural gas** | Separately accounted raw fossil commodities | Existing oil/gas sources, delivered supplies and trade; sell directly if sale is allowed, or reserve/refine for strategic capability |
| **Petroleum fuel / kerosene** | Distinct **refined** fossil commodities | Only authorized refinery output/valid external acquisition; may sell for USD or use for transport, production and special abilities |
| **Crude / refined nuclear materials** | Highest-value special strategic resources, not ordinary USD exchange stock | Restricted existing refined-material, faction, intelligence and victory rules; no generic nuclear goods-to-dollar sale route |
| **Electricity** | **Generated capacity versus allocated load**, not spendable money, not stockpiled | Each operational building must have positive minimum load satisfied to function normally; conventional and authorized nuclear generation, imported grids and infrastructure can supply |
| **Food reserves** | Purchased, consumable **military upkeep stock**, not a second general currency | Bought for dollars through an eligible supply channel; periodic upkeep of deployed staffed forces |

"Nuclear materials highest, fossil fuels second" remains the existing **strategic rarity/importance** ordering. USD is the universal price/payment unit, not a competing item in that rarity ranking. Gold is a trade good and does not displace the special fuel/nuclear hierarchy. The display is **USD-denominated fictional game accounting**; it does not assert actual Yugoslav 1980s transactions were conducted in USD or follow real historical commodity/FX rates.

## 2. Primary USD acquisition without farming

A normal player starts with an authorized initial **$ treasury** and can accrue a limited nonzero base income from the initial faction infrastructure. Further inflows derive from controlled eligible industrial facilities, ports/rail junctions, contractual supply payments, mission rewards and the sale of acquired commodities. These are **strategic objectives**, not repeated food/wood/gold worker micromanagement. Author ownership, control, operational status, supply access and contract permissions separately. A unit near a civilian hospital or Unaligned municipal building does not acquire its income or change jurisdiction automatically.

Budget sources use finite integer USD cents per specified authoritative interval or one-time authoritatively logged event. A disconnected/contested/inoperative node may cease generating **future** income, not confiscate prior balances. Include an ordinary-skirmish recovery baseline so one lost power plant or income node does not immediately cause irreversible economic softlock. An idle player does not need to issue a collection order every few seconds.

The first 4B fixture can still demonstrate the **existing source → collector/delivery → balance** boundary by delivering one legitimate finite shipment and selling it for USD. Ordinary subsequent income may be automatic. Changing the exact 4B positive fixture requires coordinated owner-document and test updates, but **the spendable balance is now definitively dollars**.

## 3. Commodity-to-dollar exchange

**Eligible sale goods:** gold, crude oil, natural gas, petroleum fuel and kerosene, subject to participant ownership, available unreserved quantity, sale permissions, valid source/depot, counterparty capacity and an eligible market/contract. Raw and refined oil must never share one undifferentiated stock. A refinery remains obligatory to **produce** finished petroleum/kerosene from crude; selling crude for USD does not transform it into refined fuel. Selling nuclear materials, restricted weapons or missing/stolen-by-claim inventory is forbidden unless an explicitly separate canon rule is approved; this document approves **no nuclear dollar sale**.

**Single authoritative sale transaction:**

1. Verify the actual admitted participant, match generation, commodity stable ID, seller inventory and sale permission. Require a positive quantity and a new transaction ID; do not act on a rumor or a UI quote alone.
2. Author a quote as a nonnegative rational or integer rate **USD cents per commodity unit**, fixed-point precision, nonnegative fee, quote expiry and quantity/counterparty caps. Explicitly specify rounding, not floating-point cash arithmetic.
3. Reserve exact uncommitted inventory; do not permit simultaneous refining, another sale or transfer to reuse these units.
4. At a documented simulation-step boundary, atomically **debit the commodity quantity once and credit net USD cents once**. On rejection, cancellation or timeout, release the reservation with **no value mutation**.
5. Publish accepted/rejected/completed feedback with quantity, price, fee, net yield and owner-authorized visibility. Reject underflow, overflow, duplicate-ID replay, missing counterparty, expired quote and unavailable capacity.
6. Disallow free sale/buy-back or refinery-sale conversion cycles with positive arbitrage caused solely by inconsistent rounding or undocumented fees. Author different values for gold/raw/refined goods as balancing choices; no live real-world market feed.

When the counterparty is an NPC market, commodity stock leaves the player's inventory and the market supplies a USD grant according to an **explicit trade action**. A player-to-player trade, if ever supported, must conserve both parties' funds/goods under distinct authorization. Purchased supplies or acquired commodities cannot bypass Black Market faction restrictions.

## 4. Mandatory electricity upkeep for buildings

**Every deployed operational building has a strictly positive authored minimum power requirement.** A generator has its own declared auxiliary load, and may satisfy it from self-generation during startup; it is not secretly exempt. Maps with a functional base must provide an eligible starting power source (conventional plant, self-powered starting generator, grid agreement or equivalent) sufficient for essential buildings. No impossible "need electricity to start the only electricity producer" cycle is permitted.

At each planned authoritative power update, determine eligible owned/net generation and permitted supply connections, sum positive building loads, allocate deterministically by a declared priority + stable building ID and record whether each facility receives its minimum. A facility below minimum is **Unpowered**: it keeps ownership/health but cannot operate normal powered functions. Production/refining/research queues **pause** (do not finish or re-charge), powered radar/communications/defense functions disable as authored, and the HUD reports the cause and affected buildings. An optional Brownout state can be added after binary on/off is tested. Do not equate zero USD with zero power; purchasing power capacity is separate from satisfying the electrical load.

A generator damaged or cut off by sabotage should create a counterable outage. A legal emergency/auxiliary/self-starting path must allow restoration even when the primary grid is down. The player can still issue basic commands, return to menus and reach an ordinary match result. Only Atlantic Concord and Red Directorate may **construct nuclear power plants**, as already established. Iron Ascendancy and Meridian Pact may use authorized **conventional** or imported power without gaining nuclear construction/refinement privileges. Unaligned grid assets remain under their own ownership/capture rules.

Electricity is **capacity and service**, not another accumulating inventory bar, a currency or a per-frame dollar debit. A fossil-fueled generator can later have an explicitly authored fuel rule, but do not silently consume all crude oil every electricity interval or invent a mandatory refueling chore.

## 5. Mandatory food reserves for staffed forces

A military player's participating force maintains an **owned provisions reserve** (FoodStock). A deployed **staffed** unit or formation has an authored nonnegative food-consumption rate for a fixed upkeep interval, including infantry and crewed vehicles. A deliberately unmanned unit may have zero; every ordinary staffed force therefore generates **positive minimum food upkeep**. Food is *not* an extra general-purpose purchase currency, not civilian welfare, and not a mandatory farm/labor simulation.

At each due authoritative interval, total eligible **alive, staffed, current-owner** unit requirements in stable ID order; debit stocked food units **exactly once** if sufficiently stocked. The initial scenario must supply enough food for a meaningful opening battle, plus an **eligible USD food-purchase path**. Newly produced units enter the upkeep set on the next specified accounting step; destroyed/transferred units stop counting for the old owner; idle, guarding or retreating staffed units still require food.

**Food purchase:** verify participant, $ balance, owned food account, available supply channel, purchase quantity, nonnegative USD-cent price, capacity and unique request ID. Atomically debit USD and credit the purchased food stock once. A future delayed delivery must remain pending/reserved until physically/contractually completed; no instant credit plus duplicated shipment. Optional **Auto-Buy Food** is a *player-controlled* threshold and maximum-budget preference using the **same** authorized transaction path, with a clear failure notification and no free supplies.

**Shortage response:** stock floors at zero; when a due upkeep debit is impossible, set a visible **FoodShortage** status and apply a single limited, specified gameplay penalty (e.g. reduced reinforcement/operational readiness) rather than instantaneous death or irreversible troop removal. The precise penalty is a design/balance variable, not yet adopted code. Food shortage must not block a player's ability to buy food, recover a node or protect their remaining force. On legitimate replenishment, remove the penalty at the next defined upkeep/recovery boundary. This makes logistics consequential without repetitive micromanagement.

## 6. Accounting authority and safe ordering

Maintain **one authoritative C++/Unreal ledger** scoped to participant and match generation. UI, AI and game-rule scripts submit typed requests; they cannot manufacture $ or supplies. Persist integer **USD cents (int64)**, food/commodity nonnegative fixed-point units with declared scale, and nonnegative bounded power capacity/load. Arithmetic, sums, rate multiplication, conversion, reservations and purchases require checked overflow/underflow and a deterministic rounding policy. All purchases/sales have a key **(match generation, participant, transaction ID)**; request retries are idempotent, and no rejected request can modify stock.

Proposed economy step within the current **0.05-second fixed-step coordinator** (not a new Actor Tick/independent timer): admission of queued transactions → settled income and sale/food purchases → power allocation after damage/ownership changes → due food upkeep → eligible production progress → typed event publication. Reconcile precisely with the existing admit→movement→combat→damage→cleanup→objectives→events sequence at implementation time, and decide whether death and completed reinforcements affect this interval or the next. In-progress queues paused for power must resume without double spend. Stop all economic mutation after Terminal/Results/Return/AdmissionRejected; clear income cadence, supply accounts, transactions and reservations on fresh match.

Epic [UPrimaryDataAsset](https://dev.epicgames.com/documentation/unreal-engine/API/Runtime/Engine/UPrimaryDataAsset) supports existing stable-ID asset patterns; [Data Validation](https://dev.epicgames.com/documentation/unreal-engine/data-validation-in-unreal-engine) supports missing-ID, negative-cost, zero-power and invalid-reference authoring checks. [Gameplay Timers](https://dev.epicgames.com/documentation/unreal-engine/gameplay-timers-in-unreal-engine) are real UE facilities but should **not** become independent economic authority when the coordinator owns deterministic step timing.

## 7. UI and AI contract

**Persistent compact HUD:** **$ Available** (plus $ income/interval), **Electricity supplied / required** (surplus/deficit and affected buildings), and **Food reserve / upkeep per interval** (remaining runway/shortage). Gold, crude oil, gas, refined fuel and nuclear stock are expandable commodity details when relevant; do not crowd the combat HUD with every unused inventory type. A trade panel shows source, real stock, USD quote, fee/net, restricted quantity and confirmation. Food panel shows dollar purchase quantity/price and optional replenishment cap, with a clear shortage warning, recovery action and estimated runway. Tooltips distinguish insufficient **$**, insufficient **food**, insufficient **power**, reserved stock, ineligible sale and invalid participant; never imply accepted intent equals settled transaction.

Computer opponents must buy food, trade goods and meet building power requirements under the **same ledger/permissions**; a difficulty setting must not invisibly grant unlimited supplies. Enemy economic truth is visible only under legitimate observation/intelligence permissions, not simply because a HUD can query Actors.

## 8. Implementation stages and acceptance

**Stage 4B first slice (refined design; still unimplemented):** One authored **$** treasury and income/delivery source; one valid source→delivery→conversion or authorized grant; one fixed **powered** base with positive minimum building load; one basic production queue and produced unit; authored starting food for its initial staffed force; one **USD→Food purchase**; one due food-upkeep interval and bounded power/food deficit-and-recovery fixtures. There is no requirement yet to build a full gold mine, multiple oil refiners or a commodity exchange UI. If this is too broad for one increment, split it into explicitly evidenced 4B sub-slices; finish core upkeep before claiming 4D AI / 4F complete skirmish.

**Next economy slices:** actual gold/fossil sale from real inventories; captured node-based USD income, conventional generators and grid sabotage; optional auto-food procurement; specialized refining, regional electricity, diplomacy/espionage trade and Unaligned infrastructure only when their dependencies are ready.

| Gate | Required evidence |
| --- | --- |
| USD-01 | Single $-priced purchase settles once; invalid owner/stale generation/negative cost/overdraft rejected without mutation |
| USD-02 | Actual gold, crude oil, gas and refined-fuel sale returns exactly quoted net $ and consumes the correct stock only once |
| USD-03 | Trade fees, rational rounding, expiry, reserved quantity and overflow cannot duplicate money or sell nonexistent goods |
| USD-04 | Nuclear materials/weapons remain excluded from ordinary dollar exchanges; faction restrictions preserved |
| USD-05 | Authorized income/contract events and same-process rematch cannot double-credit prior USD |
| PWR-01 | Every operational building has positive minimum power and legal source at startup, including a bootstrappable generator |
| PWR-02 | Insufficient supply disables only authorized powered functions/queues and is visibly diagnosed |
| PWR-03 | Damage, capture, repair and reconnection cause deterministic recovery without permanent power softlock |
| FOOD-01 | Staffed initial force has positive upkeep, sufficient authored starting runway, and a real dollar-funded food supply route |
| FOOD-02 | USD→Food purchase atomic, capacity-limited and idempotent; insufficient money/supply leaves accounts unchanged |
| FOOD-03 | One due interval charges once; unit death, capture, rematch, idle status and multi-participant isolation handled |
| FOOD-04 | Shortage gives an observable limited penalty; replenishment restores function without arbitrary unit loss |
| ECO-INT-01 | Different render schedules produce matching qualified $/power/food outcomes at authoritative steps |
| ECO-INT-02 | Rendered cooked Linux package and AI use identical permissions; UI does not leak enemy treasury/shortages |
| ECO-INT-03 | Normal tactical match remains playable without routine gold/food harvesting micromanagement |

**Non-claims:** No Unreal/cooked implementation or new schema exists; the current portable ResourcePool helper is not already a USD/food/power system. The existing Command & Conquer precedent of vulnerable base power is documented in [EA's guide](https://help.ea.com/articles/command-and-conquer/command-and-conquer-remastered/how-to-play/); this does not justify copying a harvester-intensive economy.
