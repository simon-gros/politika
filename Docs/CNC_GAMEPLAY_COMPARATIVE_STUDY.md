# C&C comparative gameplay and content study for POLITIKA

**Purpose:** extract design lessons from the locally installed/extracted Command & Conquer titles and translate them into POLITIKA-owned design guidance. This is an analytical reference, not an implementation specification and not a source of assets to ship.

**Evidence boundary:** UM extraction manifests establish which archives/files are available, not that every file was decoded or validated in-game. The classic Westwood formats expose more human-readable rules, map, and campaign data; later SAGE titles expose useful file taxonomies, map packages, UI manifests, and asset groupings, but much gameplay behavior is in proprietary/cooked formats. Conclusions below distinguish direct structural evidence from design synthesis. Paths are relative to `/mnt/data/CommandAndConquer/UM/` unless stated otherwise.

**Broader comparison:** This C&C-specific, extraction-limited study remains the owner of local C&C source observations. For the wider genre taxonomy (StarCraft, Tempest Rising, large-scale RTS, tactical military games and political/economic hybrids), see [Cross-Genre RTS Comparative Study](RTS_CROSS_GENRE_COMPARATIVE_STUDY.md) and the proposed [Competitive Benchmark Register](RTS_COMPETITIVE_BENCHMARK_REGISTER.md). Those references do not verify local proprietary extraction data or qualify POLITIKA runtime behavior.

## 1. Comparative lenses

| Title family | What the local extraction is especially good for studying | Design pattern to retain (at the level of principle) | POLITIKA application |
|---|---|---|---|
| **Tiberian Dawn / Red Alert** | Classic rule, scenario, map, briefing, unit art/audio and mission data; the Red Alert extraction includes classic MIX content. | A small, legible combat vocabulary can carry strategic depth when production choices, map routes, and timing interact. Missions can teach a rule through a concrete situation. | Keep a comprehensible shared baseline of infantry, armor, indirect fire, reconnaissance, and support. Let faction doctrine, logistics, intelligence, and terrain alter the decisions rather than multiplying near-identical units. Build campaign missions around one or two learnable strategic pressures. |
| **Tiberian Sun / Firestorm; Red Alert 2 / Yuri’s Revenge** | Large campaign/multiplayer map collections and classic data, including the RA2 `rulesmd.ini` and many `.map` files. | Distinct battlefield environments and map geometry make the same roster play differently; faction identity is readable through silhouette, animation, and battlefield role. | Make Balkan geography operational: river crossings, ridgelines, rail/road corridors, industrial sites, and dense settlements should create meaningful routes and exposure. Require faction units to remain distinguishable at gameplay zoom and in the 4K HUD. |
| **Generals / Zero Hour** | Readable INI/map-oriented content, map scripts/configuration, separate faction art/audio groups; Zero Hour adds a variant/commander-style expansion layer. | Unit/structure behavior and map scripting can be data-driven; faction specializations and selectable strategic variants add replayability, but can expand balance/test combinations rapidly. | Keep POLITIKA’s stable faction identity in its five rosters; express doctrines as a small set of bounded command doctrines or operational packages, not an extra full roster layer. Author map scenario rules as data, then validate them against the common match contract. |
| **Tiberium Wars / Kane’s Wrath** | SAGE-era grouped content (`Apt`, streams, terrain, WBData, maps, libraries) and broad campaign/multiplayer map packages. | A strong three-sided faction triangle can be readable, but subfactions and larger upgrade/ability sets raise the cost of learning, UI, and balance. | Preserve the asymmetric strategic roles in POLITIKA’s five factions while enforcing shared counterplay categories and a consistent command grammar. Make every specialist ability communicate cost, range, duration, counter, and strategic consequence. |
| **Red Alert 3 / Uprising** | Rich UI manifests, ability-button assets, campaign/tutorial/multiplayer map packages, and large separate audio/content groups. | Highly differentiated faction presentation and explicit secondary abilities can make unit identity immediate; water/land/air interaction creates multi-axis map choices. | Treat the Adriatic and major rivers as real operational spaces only where the map and scenario justify them. Give units a compact, inspectable ability set; make naval/riverine assets strategic choices rather than a mandatory parallel tech tree on every map. |
| **Tiberian Twilight (C&C4)** | Tactical HUD/shell manifests, map and WorldBuilder data, objective and unit-ability UI taxonomy. Its extraction report lists 20,001 files (~4.9 GiB) in original formats. | Clear objective presentation and contextual command UI can make a complex battlefield legible. The class/loadout and objective-driven direction is worth studying, but its reduced base-building/economy is a deliberate departure from POLITIKA’s intended RTS. | Borrow the *clarity* of objective, command, and selected-unit information—not the stripped economy or class/loadout rules. Retain POLITIKA’s base/economic/strategic layer and ensure objectives are subordinate to the match’s resource, territory, and political pressures. |
| **Renegade** | Partial surface/extraction only: the manifest reports no bulk-extracted MIX1 members. | A first-person perspective offers a useful contrast for scale, hit feedback, and how infantry/vehicles read up close, but it is not a reliable source for this top-down RTS content study. | Use only as an optional presentation reference; do not infer RTS unit balance or content schema from the incomplete extraction. |

## 2. Cross-title patterns translated into POLITIKA rules

### Units, weapons, and abilities

1. **Give each unit one primary battlefield job.** Secondary utility should create a choice or vulnerability, not erase the unit’s counter. POLITIKA roster entries should state role, target class, threat range, mobility, durability, cost/upkeep, counters, and unlock path before detailed stats are tuned.
2. **Share the combat grammar; vary the doctrine.** Across five factions, retain recognizable shared categories (line infantry, anti-armor, reconnaissance, direct-fire armor, indirect fire, air defense, logistics/engineering, command/support). Differentiate through access, deployment, information, sustainment, and tactical tradeoffs—not just larger damage numbers.
3. **Make abilities bounded and previewable.** Every active ability needs an explicit activation condition, target/range, duration, cooldown or charge, resource/ammunition cost, interruption/counter, and UI feedback. Avoid stacking hidden bonuses across faction, doctrine, morale, and terrain.
4. **Keep weapon-counter relationships teachable.** Show why a unit is effective or vulnerable in the HUD/log; test mixed-force engagements rather than isolated damage-per-second tables. Resolve, suppression, visibility, and terrain can modify outcomes, but must not obscure the base weapon role.
5. **Protect roster scale.** Treat proposed roster entries as a design pool, not a launch promise. Prototype a representative slice per faction, then add units only when they introduce a distinct decision and a clear counter.

### Factions and progression

- Maintain five asymmetric strategic identities, including the NPC Unaligned, while preserving cross-faction role equivalence sufficient for map validation and match fairness. The playable factions should not be forced into mirror rosters.
- Use the existing POLITIKA-specific strategic identity—Atlantic Concord expeditionary reach, Red Directorate state-industrial capacity, Iron Ascendancy coercive occupation, Meridian Pact commercial/logistical leverage, and Unaligned local/federal response—as the basis for equipment and mechanics. No C&C faction identity should be transplanted.
- Make progression expose meaningful tradeoffs: a player should choose between capabilities or timing windows, not simply unlock a strictly superior version of every unit. For doctrine variants, track the combinatorial balance and UI burden explicitly.
- Preserve counterplay for information warfare and coercion. Reconnaissance, counterintelligence, sabotage, propaganda, and morale effects should have tells, attribution uncertainty where intended, response options, and recoverable consequences.

### Maps and scenarios

- Evaluate maps by **decision topology**, not visual novelty alone: routes, chokepoints, flank paths, high ground/line of sight, cover, bridges/crossings, resource access, buildable area, reinforcement paths, and objective approach time.
- Ensure asymmetry is intentional and tested. Starting positions need comparable strategic opportunity, not necessarily identical terrain; faction-specific deployment constraints must not silently make a starting slot unusable.
- Separate multiplayer map rules from campaign scripting. Scripted objectives and reinforcements may create authored exceptions, but must be declared in the map profile and tested for a valid completion path.
- Include 1v1, team, and 2–4-player layouts where supported; test resource contest timing, reachability, travel times, and pathing at representative force sizes. Water should be a deliberate strategic axis, not decorative access that only one roster can exploit.

### User interface and feedback

- Keep the shared command loop stable: select → inspect role/state → issue order/ability → receive immediate acknowledgement → see ongoing status and counterplay.
- Make selected-unit information contextual, but put common commands and critical state (health, suppression/resolve, ammunition or cooldown, orders, veterancy/progression where used) in predictable locations.
- Distinguish objective state, unit ability readiness, resource changes, and intelligence confidence with more than color alone. Support readable scaling and input at 4K and at lower resolutions.
- Present the strategic map and tactical HUD as complementary views. A minimap marker must not be the only indication of a critical event; provide accessible text/icon/audio cues and configurable event emphasis.

## 3. Concrete POLITIKA study and prototype agenda

Use the existing design owners rather than duplicating mechanics here:

1. **Roster pass:** annotate each candidate in [POLITIKA_FIVE_FACTION_ROSTERS.md](POLITIKA_FIVE_FACTION_ROSTERS.md) with unique decision, role, counters, and unlock/economic burden. Remove or merge entries that fail the unique-decision test.
2. **Combat slice:** prototype one line infantry, specialist, scout, tank, artillery, air-defense, logistics, and support unit per representative faction pair. Measure whether role and counters are understandable from the HUD; then expand to the five factions.
3. **Map set:** select one compact urban/industrial map, one river-crossing map, one mountainous corridor map, and one larger team map. Check spawn fairness, reachable objectives/resources, route diversity, time-to-contact, build space, and Unaligned intervention rules using the [map validation contract](ENGINE_OUTLINE.md#map-authoring-contract-and-validation-pass).
4. **Ability and UI pass:** take a representative ability from each strategic family (recon, sabotage, morale/information, logistics, direct combat), and verify preview → commit → feedback → counterplay in the [POLITIKA UI overview](POLITIKA_UI_OVERVIEW.md).
5. **Scenario pass:** compare a campaign/tutorial mission against multiplayer rules to ensure the tutorial teaches systems that remain true in the main game; document intentional exceptions.

These are design-study recommendations, not claims that the prototypes or validations have been implemented.

## 4. Source map and limits

The canonical local index is [`UM/INDEX.md`](</mnt/data/CommandAndConquer/UM/INDEX.md>); the generated [file-level reference catalog](</mnt/data/CommandAndConquer/UM/REFERENCE_CATALOG.md>) filters extracted members by title, source archive, extension, and heuristic purpose, with the complete JSONL records alongside it. Per-title extraction manifests enumerate extracted archives and explain limits. Representative local evidence includes:

- Classic rules/maps: `Command & Conquer Red Alert II/extracted/expandmd01/rulesmd.ini`, `Command & Conquer Red Alert II/extracted/MULTI/*.map`, and `Command & Conquer Tiberian Sun/extracted/multi/*.map`.
- Generals data/maps: `Command and Conquer Generals/extracted/INI/Data/INI/Object/*.ini`, `.../INI/Data/INI/GameData.ini`, and `.../maps/maps/*/map.ini`; Zero Hour’s manifest records `INIZH.big` and its extracted data groups.
- SAGE maps/UI: `Command and Conquer 3 Tiberium Wars/extracted/SP/1.8/patch8/data/maps/official/`, `Command and Conquer Red Alert 3/extracted/Data/Maps*/data/maps/official/`, and `Command and Conquer Red Alert 3/extracted/Data/Apt/data/aptui/*ability*`.
- C&C4: `Command and Conquer 4 Tiberian Twilight/extracted/Apt/data/aptui/`, `.../WBData/data/worldbuilder.manifest`, and the per-title extraction manifest.
- `Command & Conquer Renegade/extraction-manifest.md` records the incomplete MIX1 extraction; absence of bulk-extracted files is not evidence that the game lacks those systems.

For detailed mechanics, pair extracted data with official manuals/tutorials or in-game observation. Some proprietary formats are not decoded by UM, and extracted files alone do not establish runtime behavior. See also [C&C extraction report index](</mnt/data/CommandAndConquer/UM/INDEX.md>) and [archive rights and provenance](../Archive/RIGHTS_AND_PROVENANCE.md). Keep extracted assets local; this study records design observations and does not copy game assets into POLITIKA.
