# POLITIKA — Historical Reference Archive

POLITIKA is an independent open-source real-time strategy game set in an alternate-history 1980s Yugoslavia. This directory is its historical research and reference archive: a structured record of the places, institutions, political ideas, technologies, environments, cultures and everyday realities that can inform the game's world.

Primary period: 1980–1989. Primary setting: the Socialist Federal Republic of Yugoslavia (SFRY), with relevant neighbouring countries, international relationships and Cold War context.

The archive is an evidence layer, not a game-asset repository or a statement of game canon. Its purpose is to make historical inspiration traceable while giving designers the freedom to create an original alternate history. Archival photographs, publications and recordings may inform original maps, buildings, vehicles, uniforms, interfaces, missions and faction designs without being licensed for direct inclusion in the game.

## Research priorities

The first priority is evidence from Yugoslavia itself during 1980–1989. Research should cover the federation as a whole while respecting substantial differences between republics, autonomous provinces, regions, cities and communities.

Use the following source hierarchy:

1. Contemporary, dated primary evidence (1980–1989): photographs, television and radio, newspapers, period maps, statistical publications, official records, industrial catalogues, manuals, advertisements, posters and other surviving material.
2. Contemporary material with uncertain dating: potentially useful evidence from the decade, clearly marked as approximate or unverified.
3. Adjacent-period evidence: material from the 1970s or early 1990s where continuity into the 1980s can be established; do not silently treat it as 1980s evidence.
4. Retrospective scholarship and testimony: histories, academic research, memoirs, interviews and oral histories, with the distinction between later interpretation and contemporary observation preserved.
5. Modern reference material: useful for long-lived geography or physical structures when corroborated, but not proof of their historical appearance, use or political meaning.

Official publications, partisan reporting and propaganda are valuable evidence of how institutions or movements represented themselves, not automatically independent confirmation of their claims. Record the source's perspective, geographical scope and uncertainties.

## Subject coverage

Prioritize research that can answer concrete game-design questions:

- Politics and institutions: the federal government, republics and provinces, League of Communists, Titoism, workers' self-management, nonalignment, political debate, state administration and international diplomacy.
- Economy and society: factories, industrial production, energy, transport networks, agriculture, trade, inflation, household consumption, labour, education and regional development.
- Military and security: the Yugoslav People's Army, Territorial Defence, civil defence, policing, equipment, vehicles, logistics, facilities, organization and historically accurate visual identification.
- Geography and built environment: cities, rural settlements, roads, railways, ports, airfields, rivers, terrain, industrial complexes, monuments, architecture and changing infrastructure.
- Everyday life and culture: clothing, signage, consumer technology, cars, broadcasting, newspapers, television, cinema, music, sport, language, slang, advertising and material culture.
- Cold War context: NATO, the Warsaw Pact, the Non-Aligned Movement, neighbouring states, global trade and international tensions insofar as they illuminate the Yugoslav setting.

Give particular attention to RTS production uses: terrain and map design; architectural and industrial tilesets; faction and unit silhouettes; equipment and vehicle references; user-interface styling; environmental storytelling; dialogue; campaign chronology; mission objectives; and the depiction of civilian institutions.

## Populated archive — first research seed

**Research pass: 27 October 2026.** The archive now contains **166 documented historical and cartographic references** and live item/collection links covering real Yugoslav history from 1980–1989. The records explicitly distinguish primary photographs and publications, historical secondary analysis, curated museum objects and research-only leads, with date reliability, creator, region, rights, evidence grade and application to the game.

Start with the [Master Catalogue](CATALOG.csv), then use the [Reference-only Index](media/reference-only/INDEX.md) for human browsing. The most concrete period-visual seed currently includes four individually dated Sarajevo tram/urban photographs from 1982 under file-specific CC BY-SA 4.0 licences; museum and periodical records of Sarajevo's 1984 Olympics, its visual identity, the 1987 Day of Youth material, and 1984–1988 press; an early-1980 Belgrade street negative; and a collection of independently sourced economic, political and diplomatic contextual studies.

The October 2026 political-history expansion adds **69 source-linked political developments** across 1980–1989. It covers federal succession and executive governance, the economy and debt, party and constitutional reform, Kosovo and Vojvodina, specific developments in all six republics, the non-aligned diplomatic network, the European Communities, neighboring Albania/Bulgaria/Romania/Hungary and the wider Cold War. Year-by-year narratives explain their historical significance and suggested game-system relevance; political changes after 1984 require an explicit alternate-history review.

**New cartographic research:** 66 indexed map and map-series records, including authentic CIA Yugoslav maps from 1981, a 1981 Zagreb city map, older Military Geographical Institute road maps, and 44 selected named topographic sheets. The records distinguish verified period originals from older baselines, post-1989 maps, and undated collection leads. Map scans are not mirrored.

**New documents and their roles:**

| Path | Active content |
| --- | --- |
| [`CATALOG.csv`](CATALOG.csv) | Master index with **166** linked rows, stable `POL-` IDs, creator/date/area, research confidence and rights. |
| [`MAPS_1980_1989.md`](MAPS_1980_1989.md) | Source-based political, physical, topographic, city and road mapping guide with chronology and asset-use rules. |
| [`MAP_CATALOG.csv`](MAP_CATALOG.csv) | 66 individually identified maps, sheets and collections; 44 selected named topographic-sheet references. |
| [`media/reference-only/maps/INDEX.md`](media/reference-only/maps/INDEX.md) | Browsable map-source links with no unauthorized copies. |
| [`POLITICAL_HISTORY_1980_1989.md`](POLITICAL_HISTORY_1980_1989.md) | 69 sourced political developments: comprehensive year-by-year register, implications, uncertainty, political context and proposed RTS applications. |
| [`POLITICAL_EVENTS.csv`](POLITICAL_EVENTS.csv) | Structured index of the 68 political developments for searching, validation and future engine/tool ingestion. |
| [`SOURCES.md`](SOURCES.md) | Curated source institutions, verified artefacts, search terms, geographical gaps and acquisition methods. |
| [`HISTORICAL_CHRONOLOGY_1980_1989.md`](HISTORICAL_CHRONOLOGY_1980_1989.md) | Decade chronology and historical-vs-alternate-history divergence ledger. |
| [`DESIGN_REFERENCE_ATLAS.md`](DESIGN_REFERENCE_ATLAS.md) | Visual grammar, cultural atmosphere, environmental assets, proposed mission hooks and traceability to the existing story and systems. |
| [`RIGHTS_AND_PROVENANCE.md`](RIGHTS_AND_PROVENANCE.md) | Rights classes, historical confidence, ethical use and source-to-asset rules. |
| [`media/README.md`](media/README.md) | Media storage and reference-only policy. |
| [`media/reference-only/INDEX.md`](media/reference-only/INDEX.md) | Browsable outbound links to sources, with no unauthorized scans or copied graphics. |
| [`media/licensed/README.md`](media/licensed/README.md) | Reserved staging area and manifest requirements for explicitly redistribution-cleared media; no binary files committed yet. |

The initial visual/media research is **not** a balanced survey of all regions. It is strongest for Sarajevo, Slovenian publications, 1980 Belgrade and late-1980s Kosovo and federal political history; specific period evidence for Croatia, Macedonia, Montenegro and Vojvodina remains a priority. A collection-level landing page does not count as a verified individual 1980s photograph.

The repository principally versions source metadata, item links and original documentation. Primary and third-party media should stay at the source until redistribution terms and provenance have been checked. A design reference is *not* an asset licence.

## Cataloguing and evidence standards

Assign each entry a stable identifier (for example, `POL-1984-SARAJEVO-001`) and record, where available:

- Title, description, creator, original language and medium.
- Date of creation and the date or period depicted; indicate exact, approximate and disputed dates.
- Place, republic/province, collection or institution, archival signature and source URL.
- Subject tags, suggested production uses and what the source actually establishes.
- Rights/licence and any restrictions; whether a local copy exists.
- Confidence, missing context, interpretive limitations and the date the record was checked.

Keep original-language names and scripts where useful, including Latin- and Cyrillic-script forms, with searchable transliterations. Do not generalize from one city, republic, social group or year to all Yugoslavia.

For significant historical claims, distinguish direct evidence, well-supported inference, contextual analogy and speculative reconstruction. Important decisions should ideally be supported by more than one independent source.

## Research-to-production workflow

1. Discover: register a source or collection lead even if its contents are not yet available.
2. Verify: check the date, place, provenance, identity and historical scope; preserve unresolved questions.
3. Classify: identify which facts, designs or period details it can support.
4. Check rights: determine whether it is research-only or may also be distributed or incorporated into an asset.
5. Apply: use evidence to guide original gameplay, visual and narrative design; cite archive identifiers in relevant asset or mission notes.
6. Review: check that the final result is plausible for the intended year and place, and distinguish deliberate alternate-history choices from accidental anachronisms.

Do not upload protected scans, press photographs, music, broadcasts, film clips or third-party graphics simply because they are publicly viewable. Historical reference does not equal permission to reproduce. Prefer links and properly attributed metadata; check licences and permissions separately for redistribution, adaptation, promotional use and inclusion in a released game.

### Period media and public-morale references

Radio broadcasting, daily and regional newspapers, television programming, print distribution, broadcasting infrastructure and official public announcements are useful historical references for POLITIKA's fictional propaganda and morale systems. In game design, all five factions have population morale, affecting production output and army discipline, while only Atlantic Concord, Red Directorate, Iron Ascendancy and NPC Unaligned possess distinct advanced propaganda abilities. The Meridian Pact has none. These are fictional gameplay mechanics, not historical claims about real organizations. The main Tito assassination and media-double conspiracy likewise remains alternate history. See [Propaganda and Morale](../Docs/PROPAGANDA_AND_MORALE.md).

### Cold War research, nuclear deterrence and espionage references

The archive can support POLITIKA's fictional two nuclear powers (Atlantic Concord and Red Directorate), the initially non-nuclear Iron Ascendancy's first-bomb espionage race, and the primary Tito conspiracy with historical background: Cold War deterrence, security organizations, intelligence, counterintelligence, nuclear-power infrastructure, electricity, period industrial capacity, oil/gas extraction, refining facilities and regional energy supply.

Fictional design premises, not historical facts: Atlantic Concord has a Psy-Ops espionage subdivision that can make an eligible unit self-destruct upon fully broken Discipline/Morale or briefly attack its allies in a rampage; Red Directorate steals rival intelligence slightly faster and cheaper through basic infiltration; Ascendancy spying is initially more limited and must be fully developed before qualifying infiltration of a Concord or Directorate nuclear research site; Intimidation is an exclusive Ascendancy RTS ability forcing isolated eligible hostile units to change allegiance. The nuclear-material economy uses abstract processing; Concord/Directorate can build power plants and bombs, while Ascendancy can refine only for its first bomb after stolen research and Meridian cannot refine nuclear materials. Crude oil is refined into petroleum fuel or kerosene and Concord has cheaper refining. None of these fictional faction-specific acts, special abilities or victory outcomes should be attributed to real historical governments or persons.

The archive does not document technical procedures for building or processing nuclear weapons, nor does it treat game mechanics, assassination fiction or spy actions as historical evidence. It provides verifiable research context, visuals and terminology for original fictional worldbuilding. See [Espionage and Intimidation](../Docs/ESPIONAGE_AND_INTIMIDATION.md), [Resource Economy](../Docs/RESOURCE_ECONOMY.md), and [Story Outline](../Docs/STORY_OUTLINE.md) for canon.

### Fictional early scouting and sabotage are gameplay rules

POLITIKA's different early reconnaissance platforms—fast, cheap, lightly armed and armored Meridian scout-combat trucks; inexpensive unarmed Concord/Ascendancy light scout planes; and basic-rifle-equipped Directorate motorcycles—are game-unit concepts, not historical claims about actual Yugoslav or Cold War military forces.

Meridian's special early, inexpensive conventional explosive sabotage of eligible vehicles/buildings through designated non-spy units is likewise a fictional RTS mechanic. Atlantic Concord, Red Directorate and Iron Ascendancy receive comparable actions later, at greater cost and restricted to spies. Archive references may inform authentic-looking period vehicles, roads, aircraft and visual design, but should not be mistaken for real explosive-handling instructions. See [Reconnaissance and Scouting](../Docs/RECONNAISSANCE_AND_SCOUTING.md) and [Explosive Sabotage](../Docs/SABOTAGE_SYSTEM.md).

### Fictional Unaligned counterintelligence and false-flag balance

In POLITIKA's invented game world, the Unaligned possesses the strongest counterintelligence and uses intercepted intelligence, fabricated reports and false-flag attribution to preserve Yugoslav stability and sometimes divert stronger military factions away from a weak opponent nearing defeat. A misleading report of a nuclear facility is an authored RTS information mechanic and is not a claim about a real historical Yugoslav nuclear program or an actual Cold War operation.

Historical intelligence and propaganda sources in the archive may support period terminology and presentation, but must not be cited as proof of this fictional system-AI behavior. See [Unaligned Counterintelligence](../Docs/UNALIGNED_COUNTERINTELLIGENCE.md).

## Historical evidence versus fictional worldbuilding

POLITIKA's campaign, events and factions are fictionalized. The archive documents historical Yugoslavia and its international context, not the fictional history of the game. In particular, do not present the fictional organizations documented in [`Factions/`](../Factions/) as real historical actors. Conversely, historical institutions should not be retroactively assigned fictional activities without clearly labelling the departure from fact.

Where the game portrays Yugoslav federal authorities through The Unaligned, maintain a clear distinction between documented Yugoslav administrative, political and military structures and their adaptation into an NPC-only game faction. The same historical-versus-fictional boundary applies to the game's other factions, technologies and conflicts.

For design and implementation decisions, consult [`Docs/`](../Docs/); for the public project summary, see the [main README](../README.md). Those directories describe what POLITIKA is building. This archive records the evidence informing how its 1980s world is represented.
