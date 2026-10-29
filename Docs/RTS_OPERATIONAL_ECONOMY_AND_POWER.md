# POLITIKA — Non-Farming Operational Economy, Electricity and Infrastructure

**Status: PROPOSED DESIGN DECISION RECORD, 29 October 2026. No newly authored resources, UE classes or gameplay values are implemented.** This adds a *primary purchasing/accounting concept* and *power capacity contract* to the [Resource Economy](RESOURCE_ECONOMY.md), which owns existing fuel/nuclear hierarchy and faction rules. [Production](RTS_PRODUCTION_AND_TECHNOLOGY.md) owns spend/queue/producer authority; [Engine Outline](ENGINE_OUTLINE.md) owns Stage 4B sequencing; [Concepts](CONCEPTS_OUTLINE.md) owns faction and narrative themes; [Visibility](RTS_VISIBILITY_AND_INTELLIGENCE.md) owns hidden information. This document does not alter established nuclear victory conditions, oil/gas refinery rules, faction permissions or historical claims.

## 1. Central design choice

POLITIKA is a **combat, diplomacy and intelligence** RTS in an alternate-history 1980s Yugoslavia. To avoid turning it into economic-farming simulation, recommend **Operational Budget** (working name; also called operations funding or command funds) as the **one universal, directly spendable purchasing balance** for ordinary troops, construction, repairs, reconnaissance and selected covert/diplomatic actions. It accrues at bounded authoritative intervals from admissible institutions, infrastructure/industry, territory and negotiated contracts. It is **not gold in a mine**, a peasant food/water mechanic or a separate convoy of endless worker trips.

The existing statements "*nuclear materials highest strategic value; fossil fuels second*" remain intact: that is a **strategic scarcity/value hierarchy**, whereas Operational Budget is the **ordinary unit of account** used to pay for common actions. Exceptional units/abilities may require both budget **and** eligible fuel/refined material, intelligence clearance, time and infrastructure. Never reinterpret nuclear material as a purchasable generic currency.

| Layer | Gameplay character | Suggested basic cost/constraint |
| --- | --- | --- |
| Operational Budget (new candidate) | Primary liquid purchasing currency; start allowance + controlled recurring flows | Units, buildings, repairs, conventional research, mission actions, diplomatic/intelligence expenses |
| Fuel (existing fossil resource) | Scarce strategic feedstock; oil/gas production and refining according to existing canonical rules | Vehicles, aircraft, advanced production/refining, logistics operations, air sorties; not routine per-step vehicle "fuel hunger" by default |
| Nuclear raw/refined material (existing) | Exceptional, finite, high-value late-game input | Nuclear capabilities under strict faction/narrative permissions; not exchanged freely for baseline units |
| Electricity (new operational model) | Continuous **capacity vs load**, not stored farmable currency | Production throughput, sensors, communication, advanced research, powered defenses, refining; shortage degrades services |
| Optional logistics access (later) | Connectivity/route capacity, not a new wallet | Territory-generated income and reinforcements, supply vulnerability and transport |
| Political legitimacy/influence (existing concepts) | Scenario/policy variables; **not** automatic second basic wallet | Diplomacy permissions, Unaligned reactions, counterintelligence consequences, civilian attitudes |

**Avoid:** four mandatory resource bars, dedicated "farm food" workers, incessant collectors, invisible passive mana bonuses, multiple simultaneous currencies for every ordinary command, perpetual negative upkeep that prevents any recovery, or universal power requirements on every foot soldier.

## 2. Comparative reasoning

**Age of Empires IV** visibly emphasizes worker-gathering of food, wood, gold and stone and repeated villager assignment. That creates a different strategic genre rhythm. [Official Age IV guide](https://www.ageofempires.com/news/age-of-empires-iv-tips-to-help-you-get-started/).

**Company of Heroes** centers manpower/fuel/munition flows on captured/connected sectors, making battlefield position economically meaningful. Some versions also have headquarters income; this reduces worker micromanagement while preserving denial and expansion. [Company of Heroes guide](https://www.coh2.org/guides/1987/introduction-to-coh-franchise).

**Command & Conquer Remastered** uses harvested economic material and a separate base power system; EA describes low power and attacks on power plants as tactical disruption. [EA gameplay guide](https://help.ea.com/articles/command-and-conquer/command-and-conquer-remastered/how-to-play/).

**POLITIKA synthesis:** primary budget from **political/economic control and logistics**, persistent baseline so losers can recover, energy as a readily understood facility constraint, and the established scarce fuel/nuclear chains for faction asymmetry. Do not imitate opponent asset graphics or force one game's exact resource numbers onto the Yugoslav world.

## 3. Budget generation — strategic locations without farming

Candidate sources, subject to the map's historically reviewed fiction and participants' knowledge:

1. **Baseline allocation:** each admitted playable participant has an authored initial budget and a small guaranteed periodic allocation from its initial political/military network. Even cut off from every contested sector, it can attempt a modest comeback rather than entering an unrecoverable zero-income loop.
2. **Industrial, municipal and transport nodes:** controlling a factory complex, rail interchange, municipal supply depot, distribution/communications center, customs crossing or port contributes automatic budget flow while operational and properly connected. No manual worker unit is required. This location is also a combat objective and target for spying/sabotage.
3. **Diplomatic contracts:** an authorized, finite-term agreement or negotiated supply grant increases a budget inflow or gives a one-shot payment in return for political obligations/exposure. Do not create an unconstrained passive diplomatic clicker.
4. **Limited mission decisions:** escorting a freight convoy, defending administration, stealing documents, supplying local allies or maintaining a public service can produce a bounded one-off reward or temporary change to flow. These are **discrete missions**, not a busywork collecting loop.
5. **Trade/black market (later):** when the existing special system becomes implemented, allow authored budget conversions or purchases subject to faction, rights, intelligence and market rules. Never permit forbidden nuclear-material sale simply because Operational Budget exists.

A controlled node may be **owned, occupied, influenced, administratively controlled, active or connected** as separate flags. Do not assume a soldier standing on a village immediately transfers its entire economy. A temporary contested capture point may suspend only incremental income, not erase reserves. Administrative/Unaligned structures are a distinct NPC authority; damage to civilian infrastructure can reduce productivity and political legitimacy rather than automatically convert it to a spendable factory.

### 3.1 Candidate authoritative budget formula

At each fixed budget accounting boundary (e.g. every **20 simulation steps = 1.0 second** for the existing 0.05 s coordinator; this interval is **provisional**, not a committed gameplay balance):

\`\`\`text
Income[player, tick] =
    BaseAllocation[player]
  + Σ EnabledConnectedNodeYield[player, node]
  + Σ AuthorizedContractYield[player, contract]
  + EventGrant[player, tick]
  - ExplicitDocumentedPeriodicCosts[player, tick]

NewBalance = clamp_nonnegative_and_check_overflow(OldBalance + Income - ApprovedSpends)
\`\`\`

Use integer/fixed-point amounts, deterministic sorted node/transaction IDs and separate *pending*, *reserved* and *available* funds. Spending must never execute twice after retry, corrupt a neighboring participant's account or drop to a negative balance. Caps, rounding, reservation and affordability remain scenario-authored. Do not disguise a forced fee or unit upkeep as a "resource sink" without clear UI notice. If there is a supply cut, a formerly connected node's future income can stop after a documented grace period; previously collected funds remain owned unless a separately authorized theft/seizure transaction occurs.

In later versions, link regions to a home/base or eligible allied supply through a simple **typed adjacency graph** of authored connected road/rail/port hubs—not expensive continuous per-unit transport simulation. An enemy cutting a bridge or rail chokepoint changes graph connectivity at a defined authoritative step; recompute affected regions only when control/edge state changes. Preserve distinct traversability versus income connectivity; NavMesh path existence does not itself prove an institution has authorized supply access.

## 4. Electricity — capacity/shortage rather than harvesting

Recommend one **per-participant aggregate electric capacity/load** in the first *powered infrastructure* increment, then optional **regional sectors** when sabotage and local outages are being developed. A full transmission-line load-flow solver would be disproportionate for the first skirmish.

**Candidate fields:**

| Power state | Rules |
| --- | --- |
| Production | Authoritative \`power_available\` from eligible operational generators, plus independently authored imported/external grid capacity if map rules allow |
| Consumption | Sum of enabled buildings' \`power_required\`; ordinary infantry doesn't draw a per-unit power tax |
| Surplus/deficit | \`available - required\`; shortage classification from deterministic integer comparison |
| Priority and emergency | Small essential shell/HQ/control functions remain operational in shortage; prioritization is authored, never a free invisible economic bonus |
| Outages | Generator destruction, sabotage, deliberate cut, damaged substations, civilian grid failure or fuel shortage may decrease availability according to explicit feature-dependent rules |
| Recovery | Repair, reconnection, generator restoration, alternate feed or policy action; deterministic transition, explicit UI feedback |

Keep **power capacity** and **energy storage/fuel** distinct. No battery inventory or grid electricity stockpile is necessary in first implementation. Avoid automatically consuming crude oil/gas for every watt unless the particular authored generator has an explicit fuel-recipe contract; otherwise it would silently multiply farming loops.

**Faction asymmetry already in canon:** Atlantic Concord and Red Directorate can construct nuclear power plants; Iron Ascendancy and Meridian Pact cannot. That restriction does **not** bar the latter factions from using conventional/captured grid capacity as allowed by scenario rules. A nuclear generator requires its own canonical permissions and strategic investment; do not equate a power station with the separate raw-to-refined nuclear weapons-resource chain. The Unaligned may administer existing civilian grid assets without becoming an ordinary playable power-plant builder.

### 4.1 Shortage effects with recovery, not hard locks

Proposed fixed order of budget/power resolution:

1. Apply validated generator damage/repair and any authorized supply/contract updates.
2. Calculate participant/region supply and requested demand.
3. Allocate capacity in a **stable authored priority order** to operational facilities.
4. Mark per-building effective status: \`Powered\`, \`Brownout\`, \`Unpowered\`, \`Disabled/Destroyed\`.
5. Affect only explicit services next step: production speed or queue pause, radar/sensor availability, defensive system rate, refinery throughput, communications/espionage operations.
6. Publish typed power-status and queue/ability-eligibility changes to authorized observers; no global hidden-enemy power reveal.

**Essential functions must remain recoverable**: e.g., a basic repair unit, command-post interface and at least a minimal budget source should not require the very electricity that the player has lost, unless a separately accepted survival scenario intentionally uses that failure condition. Avoid a cascade where loss of one plant makes all rebuilding impossible. Reserve **partial brownout** for graded degradation if tests/UX justify it; simpler initial implementation can be binary power-on/off per nonessential structure.

### 4.2 Prototype UI

Top bar: \`Budget  860 (+12/interval) | Power  140 / 110 (surplus 30)\` as **illustrative values**, not actual game balance. On shortage: \`POWER SHORTAGE −25 — Barracks queue paused\`, with clickable list of affected owned structures and repair/alternate supply options. Separate "no power" from "no budget", "no refined fuel", and "producer damaged". Use icons/words alongside any red/yellow/green; only reveal opponent power statistics where intelligence explicitly authorizes it.

## 5. Engagement over farming: incentive tuning

**Strategic tension should come from competition over sites and relations**, not optimal micromanagement of workers. Suggested (uncommitted) design tests:

- A player can issue the first combat order and understand a credible tactical objective within the first ~30 seconds of a normal 4F scenario; exact onboarding timing must be user-tested.
- One typical battle group should require at most one or two *deliberate* economic choices (e.g., capture refinery, protect rail hub, negotiate a supply contract) during an ordinary short engagement.
- Economic investment must be legible and counterable: budget rate, power deficiency, damaged link and contract expiration produce specific effects on possible orders, not hidden aggregate numerical multipliers.
- Winning control should grant advantage **without snowballing irreversibly**: baseline allocation, capped sector yield, temporary contested suspension and alternate recovery routes are prototype levers.
- Military, diplomatic and intelligence approaches must create meaningful tradeoffs: raid a fuel convoy vs seize a rail junction, sabotage a power station vs expose civilian harm, steal a budget transfer vs negotiate legitimacy.
- Avoid a forced per-vehicle refueling/refilling loop during ordinary movement; include supply disruption only where it adds spatial decisions or authored mission objectives.
- Never make civilian labor, food, water or homelessness generic exploitable "resource mines"; civilian welfare may affect legitimacy/mission failure and the Unaligned response under historically respectful authoring.

## 6. Stage boundaries and consistency with the existing roadmap

The existing **4B** gate stays narrower: one spendable resource, one source and collector/delivery, one balance, fixed base, one production queue and one reinforcement. To minimize *collecting* micromanagement while keeping the currently specified 4B positive path, the first source may be an **automated operational-budget delivery/procurement event with a bounded supply-run actor**; make the collection and deposit a single optional/automated demonstration rather than every unit requiring workers. **This is an alternative design proposal, not a silent rewrite of 4B's already committed gather→deliver acceptance**. If 4B is intentionally revised to eliminate the collector, update its owning roadmap, gameplay/content schema and positive/negative tests together *before implementation*. Keep current 4B contract authoritative until that decision is adopted.

**Later economy/power increment** (after 4F unless a selected scenario requires it): validate faction/node control, periodic budget income, one simple generator, two facilities with nonzero demand, shortage effects, repair/reconnection and explicit power UI. Advanced fuel refining/nuclear economy, detailed regional supply, Unaligned institutions, covert manipulation and alliance-linked economic flows follow only when core mechanics are measured and gameplay justified.

## 7. Engineering and acceptance matrix (proposal)

| Gate | Positive and fail-closed checks |
| --- | --- |
| ECO-01 | Starting budget comes from versioned authored scenario, not HUD; exact participant isolation |
| ECO-02 | One tick of baseline/node/contract income credited exactly once at a specified fixed-step boundary |
| ECO-03 | Duplicate grant, lost node, disconnected supply, expired contract do not double-credit or leak funds |
| ECO-04 | No negative/overflow/zero-cost exploit, mis-owned purchase, unauthorized transfer or repeated spend |
| ECO-05 | Balance/status/production snapshots return to initial state on rejected match and same-process rematch |
| ECO-06 | Owned node income stable across render schedules and different object iteration order |
| PWR-01 | Generator ownership, operational status, capacity and load are validated from authored IDs |
| PWR-02 | One generator powers eligible consumer; loss produces an observable, reproducible status transition |
| PWR-03 | Outage degrades correct services, excludes unrelated units and **cannot create irrevocable softlock** in ordinary skirmish |
| PWR-04 | Restored capacity/repaired link revives eligible production without illicit duplicate queue debit |
| PWR-05 | Non-nuclear playable factions cannot construct a nuclear plant; allowed conventional grid remains usable |
| PWR-06 | Per-participant observation prevents hidden enemy budget and power leakage |
| PWR-07 | Typed diagnostics and one in-package rendered UI exposure match actual authoritative power state |
| ECO-07 | Can win or lose a real match through combat/diplomacy-intelligence choices without repeatedly assigning a dedicated gatherer |

**No arbitrary rates, unit prices or plant capacities are locked here.** Use scenario-authored tuning informed by gameplay measurements, not historical-economic claims. Integrate the selected version in [Resource Economy](RESOURCE_ECONOMY.md), [Production](RTS_PRODUCTION_AND_TECHNOLOGY.md), and [Engine Outline](ENGINE_OUTLINE.md) before implementing.
