# POLITIKA — implementation status and evidence index

**Snapshot:** 3 November 2026, through commit [`1424b1e`](https://github.com/simon-gros/politika/commit/1424b1e29a26ce60f14c7ab2608a9c60187f81a5). **Scope:** repository documentation and recorded acceptance; this page does not claim a fresh local build or independently inspected binaries. Update this snapshot only after checking newer commits and acceptance evidence.

This is the **single concise current-status entry point** for the README, [Engine Outline](ENGINE_OUTLINE.md), [Multiplayer Architecture](RTS_MULTIPLAYER_ARCHITECTURE.md), [Match Structure](MATCH_STRUCTURE.md), and [UI Overview](POLITIKA_UI_OVERVIEW.md). Those documents own their respective **design contracts**, not a competing live implementation ledger. The detailed acceptance documents are historical evidence and must not be silently rewritten.

## Status vocabulary and promotion

- **Designed:** requirements/proposals only; no implementation claim.
- **Implemented:** source code or assets exist, but applicable integration/packaged qualification may be absent.
- **Accepted (scoped):** a specific test configuration passed; name its test, topology, build, and exclusions.
- **Platform-qualified:** actual shipping-relevant package/device/network conditions passed their own gate. Do not infer this from Linux Development tests.
- **Release-ready:** separate product/security/performance/accessibility/installation/support gates; not achieved.

Every promotion records an exact revision, protocol/schema compatibility, commands, results, test topology, failures/waivers and follow-up defects. A green Automation suite cannot substitute for interactive packaged-client behavior.

## Implemented and recorded acceptance

| Area | Evidence as of snapshot | Boundary |
|---|---|---|
| Native shell, RTS slice, typed command/lifecycle | [Stage 4A](STAGE4A_MATCH_LIFECYCLE_ACCEPTANCE.md), [vertical slice](RTS_VERTICAL_SLICE_MILESTONE.md), [command contract](RTS_COMMAND_SIMULATION_CONTRACT.md) | Bounded prototype; not a rules-complete match |
| Server-authoritative listen multiplayer, dedicated `PolitikaServer`, connection-bound participants | [MP-1](RTS_MULTIPLAYER_FOUNDATION_ACCEPTANCE.md), [MP-3](RTS_MULTIPLAYER_DEDICATED_SERVER_ACCEPTANCE.md), [MP-2](RTS_MULTIPLAYER_MP2_MULTICLIENT_ACCEPTANCE.md) | Linux Development/loopback and recorded fixtures; not public hosting |
| 1v1, 2v1, 3v1, 2v2 layouts, team fog/last-known contacts | [MP-2 multiclient](RTS_MULTIPLAYER_MP2_MULTICLIENT_ACCEPTANCE.md), [fog memory](RTS_MULTIPLAYER_MP2_FOG_MEMORY_ACCEPTANCE.md), [listen layouts](RTS_MULTIPLAYER_MP2_LISTEN_LAYOUTS_ACCEPTANCE.md) | Test identities are not fully realized canonical factions |
| Lag/loss/duplication emulation, measured unit scale | [network conditions](RTS_MULTIPLAYER_MP2_NETWORK_CONDITIONS_ACCEPTANCE.md), [unit scale](RTS_MULTIPLAYER_MP2_UNIT_SCALE_ACCEPTANCE.md) | Whole-army admission spike (F45) remains recorded; does not prove Internet-scale readiness |
| Economic increment: timed unit-based building repair, neutral works with recurring placeholder income, finite crude field/harvester/sale | [E27](RTS_ECONOMY_E27_ACCEPTANCE.md), [E28](RTS_ECONOMY_E28_ACCEPTANCE.md), [E29/E30](RTS_ECONOMY_E29_E30_ACCEPTANCE.md) | Skirmish fixture only; balancing numbers provisional; broader economy incomplete |
| E29/E30 regression | `--oil` 10/10 twice; `--works` 12/12 twice; `--buildings` 20/20 twice, identical remote-client traces; Automation 62/62 | The record notes a timing-dependent non-gate fog-memory flag and a pause/resume; do not flatten those into a blanket perfect-regression claim |

**Protocol at this snapshot:** `network_protocol` **25**, as recorded in ECON-E30. Consult `version.json` at the revision actually being built for the latest authoritative value.

## Known limitations and open gates

1. **First Playable Skirmish** remains unaccepted: full human-versus-server-AI cycle, legitimate AI orders, integrated production and terminal results. The first opponent (OPP-1) is a proposed next slice, **not** recorded as implemented in this snapshot.
2. **ECON-E31/E32**: no acceptance document at `Docs/RTS_ECONOMY_E31_E32_ACCEPTANCE.md` in the reviewed snapshot. Do not presume landing or run builds over another active qualification process. Confirm fresh `main`/`origin/main`, cleanliness and qualification completion before work.
3. **Interaction defect:** an idle harvester parked at the producing building's centre can be unclickable by direct click; test harness box-selection workaround is **not a gameplay fix**. [E29/E30 §D](RTS_ECONOMY_E29_E30_ACCEPTANCE.md).
4. **Targeting qualification limit:** E29's look-through was packaged-tested on a unit's visible body; its earlier proposed Automation case was removed because the fixture lacked a physics scene. Do not claim arbitrary zoom/aim-point proof.
5. **Faction/Unaligned content:** networked participants and a system authority record exist; full canonical faction rosters, autonomous Yugoslav-world AI, diplomacy, intelligence deception, nuclear gameplay and campaign remain design/integration work.
6. **Platform/release:** Linux Development is the dominant evidence baseline. Windows, SteamOS/Deck, Android, public dedicated hosting, remapping/accessibility and release security require their own qualification.

## UI-first playtest readiness (decision 3 November 2026)

**Priority change, not an acceptance claim:** The current prototype has many dense, concurrent text/index readouts. Before **extensive skirmish playtesting, prolonged balance exercises or user-facing demos**, implement and scope-qualify **UI-F0–UI-F4**: on-screen data audit; reusable design system and visual HUD; authorized economy, selection, queue and order binding; robust picking and focus; and tested human-readable interaction. Keep raw diagnostic IDs in an optional developer overlay rather than the default HUD. See [UI Overview §22](POLITIKA_UI_OVERVIEW.md#22-ui-first-visual-consolidation-and-skirmish-playtest-entry-gate--3-november-2026), [HUD Architecture](RTS_HUD_ARCHITECTURE.md) and [Engine Outline §7.4B](ENGINE_OUTLINE.md#74b-ui-first-integration-gate--required-before-extensive-skirmish-playtesting-decision-3-november-2026).

**Allowed concurrently:** focused economy/network/command/AI Automation, packaged two-client regression and short interaction smoke tests. UI-F4 is a **product usability gate**, not a blocker to writing or narrow-testing OPP-1. This decision does not claim UI-F0–F4 implemented, and does not retroactively disqualify already accepted sub-slices.

## Near-term order

1. Confirm dependency and qualification state for ECON-E31/E32 before editing or building any checkout with an active qualification job; preserve accepted existing economy/network evidence.
2. **Begin UI-F0 immediately:** inventory textual HUD data, screenshot overlap/legibility at 720p, 1080p and 4K, map each field to permitted observer data and sketch the new information hierarchy.
3. **Build UI-F1 and UI-F2:** a visually structured, original HUD with grouped resource/power/food indicators, selection and producer inspectors, queue/progress and contextual action card, with real server-authorized values and typed command parity.
4. **Complete UI-F3 and UI-F4:** fix parked harvester and world/UI click priority, clarify denied actions, implement focus/help cues and qualify readable tasks on real rendered clients. Only then begin **extended human-versus-AI skirmish and balance playtesting**.
5. Continue narrow server-owned OPP-1 development and economy/network regression alongside UI work; then integrate into a complete repeatable skirmish with normal victory/defeat, results and rematch.
6. Measure large selections, widget invalidation, mass command cost and replication; integrate canonical faction content incrementally. Qualify each platform independently.

## Documentation change policy

The README links here rather than maintaining detailed milestone tables. Architecture, economy, visibility, faction, story and UI documents own **normative behavior**; acceptance files own **what was observed**. When a rule or implementation changes, update the owning spec, this current snapshot, impacted UI contract and acceptance index together. Distinguish `proposed`, `implemented`, `accepted`, `platform-qualified` in every new status claim. Never imply that a conceptual feature exists merely because a HUD placeholder or test data structure exists.
