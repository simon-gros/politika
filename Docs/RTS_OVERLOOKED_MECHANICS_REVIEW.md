# POLITIKA — Overlooked RTS Mechanics, Tactical Friction and Agency Review

**Status: RESEARCH/DESIGN, 29 October 2026; no runtime features added.** The [USD/food/electricity design decision](RTS_OPERATIONAL_ECONOMY_AND_POWER.md) makes money ($) the ordinary purchasing currency; gold/fossil sales, electricity for every operational building and dollar-purchased food for staffed forces are now intended core economy constraints, not merely optional ideas. Preserve a recoverable shortage response without turning these into food farms or automatic force death. Read with [RTS systems matrix](RTS_SYSTEMS_COMPLETENESS_MATRIX.md), [UI settings/keymap](RTS_UI_SETTINGS_AND_KEYMAP.md), [Operational economy/power](RTS_OPERATIONAL_ECONOMY_AND_POWER.md) and [Engine Outline](ENGINE_OUTLINE.md). Existing genre ontology, faction asymmetry, civilian legitimacy, espionage, propaganda, reconnaissance and nuclear victory designs are already extensive. The gaps here concern **playability contracts**, not absence of lore.

## 1. Priorities for a fighting/diplomacy/spying RTS

| Concern | Underplayed failure mode | Recommended minimal policy | Stage/gate |
| --- | --- | --- | --- |
| Discoverability/onboarding | First player can issue orders but cannot understand outcome/valid targets | Contextual first-three-minute tutorial, optional command glossary, in-game hotkey menu generated from actual binding registry | UI-A/C/D; actual novice test |
| Tactical retreat/stance | Five orders lack explicit Hold Position, Retreat, stance/engagement behavior | Distinguish Stop vs Guard vs future Hold/Retreat; choose explicit auto-acquire/chase policy; preserve existing five variants until tested | After 4F or narrow needed slice |
| Alert history | Audio-only attack warning disappears before player acts | Event ticker with optional Space-to-focus, bounded recent alerts, source/time/priority and safe camera focus without new enemy reveal | 4C baseline then later |
| Camera/minimap | Large map can be hard to traverse, hidden enemies leak via minimap | Select-to-focus, safe view bounds, objective/bookmarked sites, player-knowledge minimap, sector overlays; no omniscient global query | UI-D / VIS |
| Campaign political consequence | Dialogue choice looks cosmetic or forces hidden unwinnable state | Every meaningful diplomatic/spying decision has stated cost, permission, expected uncertainty, cooldown, legal/Unaligned consequence and observable outcome | Stage 5 |
| Legitimacy/civilians | Civilian destruction is mechanically optimal or historically insensitive | Model collateral damage and service outages with authored campaign/Unaligned responses; no universal civilian ownership transfer by conquest | Later scenario-specific |
| Infrastructure sabotage | "Power failure" only toggles a cosmetic icon | Producer/radar/radio consequence, timed outage, repair/reconnection, partial information and a recoverable counterplay | Power stage |
| Supply lines/encirclement | Territory capture instantly grants infinite generic cash | Connected node income and finite contest/cut-off; no per-vehicle consumable chore; clear split between nav reachability and procurement | Economy future |
| Production stall | Budget spent and unit stuck forever on blocked spawn | Queue states, blocked-spawn notification, bounded retry/refund and accessible alternate rally | 4B |
| Construction/territorial geometry | Production structures block only path or only artillery LOS | Separate footprint, path accessibility, objective/civilian exclusion and nav rebuild test | Post-4F placement |
| Diplomacy/influence and black market | Extra resources turn into daily clicking | Finite, mutually exclusive commitments with diplomatic costs, visible expiry and counterintelligence exposure; no background claim that spying is free | Future |
| Units lost and resilience | Snowball after base is destroyed becomes automatic with no choices | Guaranteed minimal operational recovery option, surrender/win rule with transparent terminal condition; AI should also obey losses | 4E/4F |
| Information/counterintelligence | Spy report reveals truth in tooltip or enemy health through API | Participant-scoped claim provenance, misinformation discovery, verified vs unverified sources | VIS + Stage 5 |
| Accessibility/scale | 4K or handheld makes commands unreadable; signal depends on color/audio | Independent UI/text scale, icon and status labels, subtitles/log, keyboard/controller/touch profiles | UI |
| Difficulty/AI honesty | "Hard" opponent silently sees all units or gains fabricated money | Disclose permitted AI economy modifiers, maintain participant observation and deterministic logged decision reasons | 4D/Stage 5 |
| Match pacing | Endless passive income makes games stalemate or pushes forced rush | Contested nodes/strategic objectives, explicit win/loss, bounded budgets, attrition and neutral pressure; performance/pacing telemetry | 4E–4F |
| Save/pause/resume | Settings preview commits while running, UI pause allows attack | Keep settings transactional, define local pause freeze and valid save boundaries independent from profile | UI-B + SAV deferred |
| Play modes and testing | A "mode" appears in menu without runnable match | Clear prototype labels, one scenario-complete skirmish, no fake campaign/multiplayer/replay buttons | 4F |

## 2. Three strategic systems worth developing later

**A. Communications and command cohesion.** A damaged relay/radio station can increase warning delay, prevent an air-support request or interrupt reconnaissance reports without disabling all player orders. Distinguish command *authorization* from command *delivery*; no event may silently vanish after accepted admission. UI shows delay/interference reasons and the countermeasure. This gives radio towers, propaganda and espionage a common material object without inventing a "micro every squad" signal meter.

**B. Rules of engagement and civilian consequence.** An order to bombard a civilian zone should have a clear target-policy distinction between legal/forbidden/restricted attack behavior under the fictional match rules, plus consequences in public support, Unaligned attention or mission failure. Target restrictions must be validated at authoritative execution, not just hidden behind a disabled button. This is a potential later gameplay system, not a claim about any real government's conduct.

**C. Infrastructure and legitimacy as competing objectives.** Sometimes taking a power plant intact is strategically more useful than destroying it; a negotiated municipal administration may yield a slower but more durable budget benefit; covert sabotage may preserve deniability at the cost of exposure risk. These mechanics create meaningful alternatives to straightforward combat and make Yugoslav industrial/transport geography active gameplay rather than visual decoration. Avoid morally simplistic universal "capture civilian to gain income" incentives.

## 3. Evidence-guided minimal tests

- First-play test: a new player can use default controls to select a unit, move, attack, understand Stop/Guard and recover from one failed order without reading external documentation. Record their actual observed failures, not a designer's estimate.
- Tactical clarity: one valid opponent contact, one unverified report and one hidden enemy produce distinct visible information and no privileged minimap leak.
- Economic choice: a player can gain a reinforcement without assigning multiple idle economic worker cycles, and a decisive facility fight changes their options at a measurable authoritative step.
- Power choice: disable one generator and verify exact affected buildings; ensure a countermeasure remains possible with no generator.
- Political choice: a sabotage/diplomacy action has distinct immediate tactical and delayed legitimacy outcomes, both authored and testable.
- Audio accessibility: a muted radio/voice event remains discoverable through captions and alert history.
- Losing game: a losing player has a transparent comeback or normal surrender/defeat mechanism, not an endless inactive match.
- Real packaging: feature-specific automation plus a rendered, cooked Linux Development test; desktop success does not prove Deck or Android accessibility.

## 4. Status boundary

None of the tactical, infrastructure, political or UI ideas in this review is newly implemented. Treat core gameplay commands, objective, runtime NavMesh and shell as existing scoped work; prioritize finishing PA-08/10/11 and remaining navigation acceptance rather than implementing this entire list in parallel. For more advanced mechanics, choose one measurable friction and an existing subsystem owner before adding any new game state, economy variable, Unreal plugin or asset schema.
