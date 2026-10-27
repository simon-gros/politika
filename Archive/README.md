# POLITIKA — Historical Reference Archive

POLITIKA is an open-source real-time strategy game inspired by the original *Command & Conquer* series, set in an alternate-history **1980s Yugoslavia**. This directory is its historical research and reference archive: a structured record of the places, institutions, political ideas, technologies, environments, cultures and everyday realities that can inform the game's world.

**Primary period:** 1980–1989. **Primary setting:** the Socialist Federal Republic of Yugoslavia (SFRY), with relevant neighbouring countries, international relationships and Cold War context.

The archive is an **evidence layer, not a game-asset repository or a statement of game canon**. Its purpose is to make historical inspiration traceable while giving designers the freedom to create an original alternate history. Archival photographs, publications and recordings may inform original maps, buildings, vehicles, uniforms, interfaces, missions and faction designs without being licensed for direct inclusion in the game.

## Research priorities

The first priority is evidence from Yugoslavia itself during **1980–1989**. Research should cover the federation as a whole while respecting substantial differences between republics, autonomous provinces, regions, cities and communities.

Use the following source hierarchy:

1. **Contemporary, dated primary evidence (1980–1989):** photographs, television and radio, newspapers, period maps, statistical publications, official records, industrial catalogues, manuals, advertisements, posters and other surviving material.
2. **Contemporary material with uncertain dating:** potentially useful evidence from the decade, clearly marked as approximate or unverified.
3. **Adjacent-period evidence:** material from the 1970s or early 1990s where continuity into the 1980s can be established; do not silently treat it as 1980s evidence.
4. **Retrospective scholarship and testimony:** histories, academic research, memoirs, interviews and oral histories, with the distinction between later interpretation and contemporary observation preserved.
5. **Modern reference material:** useful for long-lived geography or physical structures when corroborated, but not proof of their historical appearance, use or political meaning.

Official publications, partisan reporting and propaganda are valuable evidence of how institutions or movements represented themselves, **not automatically independent confirmation of their claims**. Record the source's perspective, geographical scope and uncertainties.

## Subject coverage

Prioritize research that can answer concrete game-design questions:

- **Politics and institutions:** the federal government, republics and provinces, League of Communists, Titoism, workers' self-management, nonalignment, political debate, state administration and international diplomacy.
- **Economy and society:** factories, industrial production, energy, transport networks, agriculture, trade, inflation, household consumption, labour, education and regional development.
- **Military and security:** the Yugoslav People's Army, Territorial Defence, civil defence, policing, equipment, vehicles, logistics, facilities, organization and historically accurate visual identification.
- **Geography and built environment:** cities, rural settlements, roads, railways, ports, airfields, rivers, terrain, industrial complexes, monuments, architecture and changing infrastructure.
- **Everyday life and culture:** clothing, signage, consumer technology, cars, broadcasting, newspapers, television, cinema, music, sport, language, slang, advertising and material culture.
- **Cold War context:** NATO, the Warsaw Pact, the Non-Aligned Movement, neighbouring states, global trade and international tensions insofar as they illuminate the Yugoslav setting.

Give particular attention to **RTS production uses**: terrain and map design; architectural and industrial tilesets; faction and unit silhouettes; equipment and vehicle references; user-interface styling; environmental storytelling; dialogue; campaign chronology; mission objectives; and the depiction of civilian institutions.

## Archive organization

At present, this directory contains this overview. Following Marburgum's historical-reference and media-library approach, the archive can gradually acquire the following components **as research is collected**:

| Proposed path | Purpose |
| --- | --- |
| `CATALOG.csv` | Searchable master index of individual records and collection-level research leads. |
| `SOURCES.md` | Curated guide to archives, libraries, museums, broadcasters, publications and discovery portals. |
| `RIGHTS_AND_PROVENANCE.md` | Detailed rules for attribution, copyright, storage, reuse and archival integrity. |
| `media/README.md` | Instructions for locally stored reference material and any redistribution-cleared media. |
| `media/licensed/` | Material permitted to be distributed, with explicit source and licence records. |
| `media/reference-only/` | Link/index information for research-only or rights-uncertain sources, not unauthorized copies. |

These are **planned conventions**, not claims that the files or subdirectories already exist. The Git repository should primarily version metadata, citations, source links and permitted material. Large files and material without clear redistribution permission should remain outside the public repository.

## Cataloguing and evidence standards

Assign each entry a stable identifier (for example, `POL-1984-SARAJEVO-001`) and record, where available:

- Title, description, creator, original language and medium.
- Date of creation **and** the date or period depicted; indicate exact, approximate and disputed dates.
- Place, republic/province, collection or institution, archival signature and source URL.
- Subject tags, suggested production uses and what the source actually establishes.
- Rights/licence and any restrictions; whether a local copy exists.
- Confidence, missing context, interpretive limitations and the date the record was checked.

Keep original-language names and scripts where useful, including Latin- and Cyrillic-script forms, with searchable transliterations. Do not generalize from one city, republic, social group or year to all Yugoslavia.

For significant historical claims, distinguish **direct evidence**, **well-supported inference**, **contextual analogy** and **speculative reconstruction**. Important decisions should ideally be supported by more than one independent source.

## Research-to-production workflow

1. **Discover:** register a source or collection lead even if its contents are not yet available.
2. **Verify:** check the date, place, provenance, identity and historical scope; preserve unresolved questions.
3. **Classify:** identify which facts, designs or period details it can support.
4. **Check rights:** determine whether it is research-only or may also be distributed or incorporated into an asset.
5. **Apply:** use evidence to guide original gameplay, visual and narrative design; cite archive identifiers in relevant asset or mission notes.
6. **Review:** check that the final result is plausible for the intended year and place, and distinguish deliberate alternate-history choices from accidental anachronisms.

Do not upload protected scans, press photographs, music, broadcasts, film clips or third-party graphics simply because they are publicly viewable. Historical reference **does not equal permission to reproduce**. Prefer links and properly attributed metadata; check licences and permissions separately for redistribution, adaptation, promotional use and inclusion in a released game.

### Cold War nuclear-context references

Because POLITIKA's **secondary fictional campaign arc** concerns preventing the Iron Ascendancy from obtaining a particular additional nuclear weapon, historical research may include **Cold War nuclear deterrence, arms-control diplomacy, strategic competition, civil-defense communications, and international security debates** as period background. Such research informs atmosphere, terminology, political stakes, and plausible briefing material—not playable nuclear-weapons production or technical construction instructions.

The three principal playable powers (Atlantic Concord, Red Directorate, Iron Ascendancy) are defined as nuclear-armed **within the fictional game world**; the Meridian Pact is a **non-nuclear playable faction excluded from this nuclear subplot**, and Yugoslavia's Unaligned authorities are non-nuclear NPCs. These are **design-canon statements**, not claims about real historical factions. Do not misclassify fictional nuclear acquisition events as archival evidence.

## Historical evidence versus fictional worldbuilding

POLITIKA's campaign, events and factions are fictionalized. The archive documents **historical Yugoslavia and its international context**, not the fictional history of the game. In particular, do not present the fictional organizations documented in [`Factions/`](../Factions/) as real historical actors. Conversely, historical institutions should not be retroactively assigned fictional activities without clearly labelling the departure from fact.

Where the game portrays Yugoslav federal authorities through **The Unaligned**, maintain a clear distinction between documented Yugoslav administrative, political and military structures and their adaptation into an NPC-only game faction. The same historical-versus-fictional boundary applies to the game's other factions, technologies and conflicts.

For design and implementation decisions, consult [`Docs/`](../Docs/); for the public project summary, see the [main README](../README.md). Those directories describe **what POLITIKA is building**. This archive records **the evidence informing how its 1980s world is represented**.
