# POLITIKA — Regional cartographic atlas for alternate-history Yugoslavia, 1984–1989

**Research date:** 29 October 2026. This is a source-driven *production-facing index* to real regional geography, political administration and transportation, **not a playable level roster or a declaration that the game already contains these maps**. The [map catalogue](MAP_CATALOG.csv) contains **147 distinct source references**, including **80 named M709 topographic sheet leads**, period maps, older nautical and road cartography, and modern reference maps. [Reference index](media/reference-only/maps/INDEX.md) · [Acquisition and provenance](MAP_ACQUISITION_GUIDE.md) · [POLITIKA story](../Docs/STORY_OUTLINE.md).

## Provenance baseline

The 1981 [CIA political map](https://www.loc.gov/item/2011587053/), [shaded-relief map](https://www.loc.gov/item/2011587054/) and the 1982 [CIA World Factbook Yugoslavia locator](https://commons.wikimedia.org/wiki/File:World_Factbook_(1982)_Yugoslavia.jpg) are contemporary period evidence. The 1988 [Yugoslav railway map](https://commons.wikimedia.org/wiki/File:Yugoslavia_Railway-Lines_1988.jpg) and earlier [VGI road maps in Croatian History Museum](https://kartevgi.hismus.hr/en/e-catalog/belgrade/) provide thematic and historic infrastructure references. Different source maps can contradict each other because the cartographic date, compiler and scale differ; record the conflict, don't silently mix incompatible features.

For a starting geographical UI, [NordNordWest's modern equirectangular SVG of Yugoslavia](https://commons.wikimedia.org/wiki/File:Yugoslavia_(1946-1990)_location_map.svg) includes its reference extent (**13.2°E–23.3°E, 40.7°N–47.0°N**) and uses **CC BY-SA 3.0 DE**. This is a **2011 reconstruction**, not a map printed in the 1980s. Its geometry can inform non-authoritative pin placement in a prototype if licence requirements are met. [The modern 1968–1990 republic/province map](https://commons.wikimedia.org/wiki/File:Socialist_Republics_and_Autonomous_Provinces_of_the_Socialist_Federal_Republic_of_Yugoslavia_(1968-1990).png) was created in **2024 and revised in 2025**, under **CC BY-SA 3.0**; it documents a modern drawing of period administrative divisions, not a contemporaneous legal document.

### Interpretation ladder

| Research tier | Example | What it can establish | What it cannot establish |
| --- | --- | --- | --- |
| Dated 1980s map | 1981 CIA maps, 1988 JŽ map | Maps/transport as understood at original date and scale | Precise building footprints or game battlefield traversal without more source detail |
| Period paper catalogue record | 1988–1989 Geokarta Belgrade, 1988 Yugoslav tourist road map | Publication date, existence, extent, scale, institutional usage | Its detailed contents without seeing a scan |
| Older original / historical chart | 1954 tourist poster, 1962 VGI roads, Admiralty coastal charts | Earlier coastlines, map design, some persistent topography | 1985 bridge, harbour or checkpoint condition |
| Later topographic sheet | DMA M709 with mixed edition years | Persistent hills and drainage, candidate tile reference | 1980s street naming, factory boundaries, borders, military facilities |
| Modern licensed historical reconstruction | Commons CC maps created 2011/2024 | Preliminary labels, vector geography, UI reference under exact licence | Independent proof that its depicted borders are correct or that it was made in 1984 |

## Geographic and mission-research regions

### Federal Yugoslavia and neutral institutions

**Administrative layer:** six socialist republics (Slovenia, Croatia, Bosnia and Herzegovina, Serbia, Macedonia, Montenegro), with Kosovo and Vojvodina as socialist autonomous provinces inside the Republic of Serbia. Use [1981 period political reference](https://www.loc.gov/item/2011587053/) and the [modern province drawing](https://commons.wikimedia.org/wiki/File:Socialist_Republics_and_Autonomous_Provinces_of_the_Socialist_Federal_Republic_of_Yugoslavia_(1968-1990).png) only in their proper evidentiary roles. Federal, republican, municipal, provincial, military and police jurisdictions should have separate map layers; they must not be flattened into a single fictional actor.

**Suggested UI layers:** political entity, constitutional jurisdiction, civilian transport, security jurisdiction, road and rail infrastructure, industry/resources, terrain movement class, active fictional faction control, uncertainty/fog-of-war and verified/false intelligence. All gameplay layers are design proposals and must not overwrite the historical base.

### Slovenia — Alpine approaches and the Drava/Sava corridors

**Political geography:** SR Slovenia; capital Ljubljana. **Landscape:** eastern Alps, karst and forested hills, Soča, Sava, Drava and Mura river systems. **Relevant sheet references:** `POL-MAP-M709-AJDOVSCINA`, `POL-MAP-M709-IDRIJA`, `POL-MAP-M709-ILIRSKA-BISTRICA`, `POL-MAP-M709-TOLMIN`, plus the existing Ptuj, Jesenice, Nova Gorica, Kranj, Postojna and Murska Sobota sheets.

**Historical cartography priorities:** 1985–1986 VGI 1:25,000 reproductions from Slovenian geodetic history (`POL-MAP-1985-86-SLO-VGI25`); period tourist and municipal maps, particularly Ljubljana/Maribor urban plans. Older VGI road maps give earlier route baselines. Original period urban plans for Maribor and Ljubljana remain *research gaps*: don't imply the M709 index covers these cities if it does not.

**RTS environmental hypothesis:** interlinked industrial towns, narrow Alpine road passes, steep valley transit, tunnels and bridges; civilian border posts and railway crossings as distinct neutral sites. At player-level scale, strategic movement restrictions should be authored from playable map validation rather than invented from national-scale print-map glyphs.

### Croatia — Adriatic ports, Slavonia and the Sava/Kupa basins

**Political geography:** SR Croatia; capital Zagreb. **Contemporary regional evidence:** [1981 U.S. State Department Zagreb city map](https://maps.lib.utexas.edu/maps/world_cities.html), [Sisak topographic map printed in 1981–82](https://kartevgi.hismus.hr/en/e-catalog/belgrade/) and the [1980–1984 Croatian TK100 and 1982–1986 TK200 series](https://registri.nipp.hr/izvori/11). These series imply **55 TK100** and **18 TK200** indexed original sheets respectively, NOT 73 downloaded files.

**Coastal reference:** [Admiralty 1955 chart 1561](https://commons.wikimedia.org/wiki/File:Admiralty_Chart_No_1561_Ports_Anchorages_and_Channels_in_Yugoslavia,_Published_1955.jpg) and [1963 chart 3906](https://commons.wikimedia.org/wiki/File:Admiralty_Chart_No_3906_Krilo_and_Igrane_to_Otok_Lastovo,_Published_1963.jpg) establish older waterway and harbor representation. They are *not* evidence for exact 1980s port modifications.

**Strategic sheet leads:** `POL-MAP-M709-PLOCE`, `POL-MAP-M709-ARZANO`, `POL-MAP-M709-APATIN`, `POL-MAP-M709-BACKA-PALANKA`; earlier indexed Rijeka, Pula, Dubrovnik, Split, Vukovar, Knin and Rovinj. Study which roads/rail junctions and port facilities actually existed in 1984–89 before adding deployable supply depots.

### Bosnia and Herzegovina — mountains, valleys and industrial nodes

**Political geography:** SR Bosnia and Herzegovina; capital Sarajevo. **Terrain:** Dinaric mountain corridors, Neretva/Drina/Una/Sana valleys and metallurgical/mining regions.

**Named quadrangles:** `POL-MAP-M709-BANOVICI`, `POL-MAP-M709-BIHAC`, `POL-MAP-M709-BUGOJNO`, `POL-MAP-M709-GORAZDE`, `POL-MAP-M709-PRIJEDOR`, `POL-MAP-M709-ROGATICA`, `POL-MAP-M709-RUDO`, `POL-MAP-M709-TRNOVO`, plus existing Sarajevo, Banja Luka, Tuzla, Zenica, Srebrenica and Mostar references. **Temporal issue:** many available DMA sheets reflect later territory names and infrastructure and must not be misrepresented as authentic Sarajevo 1984 winter-Olympics city plans.

**Production hypothesis:** rail/mining supply chain, highland ambush/detour classes, civilian industrial safety, urban–valley chokepoints and Yugoslav administrative buildings. No geographical feature alone establishes ethnic ownership or combat allegiances.

### Serbia — federal capital, Morava network and eastern border

**Political geography:** SR Serbia; Belgrade is also the federal capital. **Important paper original:** [Geokarta city plan issued 1988–1989](https://ci.nii.ac.jp/ncid/BA75523445), 1:20,000 with street index and insets. Until a scan is acquired this bibliographic item confirms the map's existence and specifications but *not* each street coordinate.

**M709 sheet leads:** `POL-MAP-M709-ALEKSINAC`, `POL-MAP-M709-NOVI-PAZAR`, `POL-MAP-M709-PIROT`, `POL-MAP-M709-PROKUPLJE`, `POL-MAP-M709-SMEDEROVO`, `POL-MAP-M709-TITOVO-UZICE`, plus Belgrade, Niš East/West, Bor and Kragujevac.

**Production hypothesis:** Danube bridge and river-port logistics, southern Morava–Vardar movement, eastern border customs, suburban federal institutions and military transport corridors. In the game, Unaligned administrative rules and the four fictional factions' activities should occupy separate simulation layers.

### Vojvodina — Danube/Tisa plains and agricultural logistics

**Political geography:** SAP Vojvodina, an autonomous province of SR Serbia (its powers underwent significant constitutional dispute in the late 1980s). **M709 references:** `POL-MAP-M709-ADA`, `POL-MAP-M709-APATIN`, `POL-MAP-M709-BACKA-PALANKA`, earlier Novi Sad North/South, Sombor, Subotica and Zrenjanin East.

**Production hypothesis:** open agricultural terrain, field boundaries, roads, rivers and rail stations; neutral farm settlements and crossings. Use period laws and primary political sources to drive any change in province status instead of depicting today's borders as a fixed 1984 treaty.

### Kosovo — mining, highland roads and provincial institutions

**Political geography:** SAP Kosovo of SR Serbia, with significant late-1980s constitutional disputes. **Original sources needed:** contemporaneous provincial maps, mining works diagrams for the Trepča/Trepça complex, rail and public-transit diagrams; these are not replaced by post-1991 topographic maps.

**Existing reference sheets:** Priština, Peć, Kosovska Mitrovica, Podujevo, Leposavić and border highlands from the UT M709 catalogue. Their printed map editions and place-name practices must be checked independently.

**Production hypothesis:** industrial mineral transport, valley towns, highland chokepoints, public communications and civil jurisdiction; avoid retroactively equating political protests with fictional sabotage or applying collective characterizations to a real population.

### Macedonia — Vardar, Ohrid and the southern international frontier

**Political geography:** SR Macedonia; capital Skopje. **New sheet leads:** `POL-MAP-M709-STRUGA`, `POL-MAP-M709-RESEN`, `POL-MAP-M709-SVETI-NIKOLE`, plus the earlier Skopje North, Ohrid, Debar and Veles references. Macedonia's 1989 political transition requires locally sourced legal and newspaper evidence, not exclusively northwestern European cartography.

**Production hypothesis:** rail crossings along Morava–Vardar transport routes, mountain/hill road access, agricultural plains and lake tourism; do not treat today's North Macedonia place names or borders as 1980s edition proofs.

### Montenegro — Titograd, mountain basins and naval ports

**Political geography:** SR Montenegro; in the 1980s the republican capital was **Titograd** (now Podgorica). **New sheet leads:** `POL-MAP-M709-ANDRIJEVICA`, `POL-MAP-M709-PLJEVLJA`, `POL-MAP-M709-HERCEG-NOVI`, plus earlier Podgorica, Bar, Cetinje, Kotor, Žabljak and Ulcinj references. A modern M709 file may say Podgorica while a 1984 street sign would read Titograd.

**Production hypothesis:** Bar seaport, Bay of Kotor maritime access, steep inland pass classes, industrial rail supply and border-country diplomacy. Separate civilian maritime activity from military surveillance/defense installations.

## Original sources available for international/cosmopolitan UI

[1982 CIA political world map](https://www.loc.gov/item/82690356/) and [1983 CIA political world map](https://www.loc.gov/item/2011589550/) establish a period-appropriate global geography backdrop for international diplomacy and Cold War news. The [1982 CIA World Factbook Yugoslavia diagram](https://commons.wikimedia.org/wiki/File:World_Factbook_(1982)_Yugoslavia.jpg) offers a smaller period-produced geographic locator. The game's Atlantic Concord, Red Directorate, Iron Ascendancy and Meridian Pact are fictional, not transcriptions of actual states or movements.

## Original reusable cartography versus period research

The multi-language general overview maps on Commons (English, Croatian, Serbo-Croatian and Slovenian) share a **2011 original graphic by Gap**, published with **CC BY-SA 3.0**; language variants are metadata entries for localization and should not be treated as four independent historical evidence items. The [2008 user-made map titled '1988Yugoslavia map detail'](https://commons.wikimedia.org/wiki/File:1988Yugoslavia_map_detail.gif) was *created 24 February 2008* despite its misleading filename. It is a useful catalogue regression test: `depicted_year` is not `publication_year`.

### Implementation acceptance checklist

- Source is valid; title and map ID unique; original date not confused with scan upload date.
- Geographic scope matches historical administrative status for a selected scenario's year; country and city names are date-qualified.
- Every map texture or visual has an original/derived distinction and item-specific licence/credit.
- Terrain layer is neither a factual assertion about political control nor a fabricated route-travel-time guarantee.
- Routes and passability are verified through Unreal gameplay-map validation, not inferred from a scanned schematic.
- The factual Yugoslav historical base and alternative game-timeline overlays remain separately selectable throughout a session.
