# POLITIKA — Historical Cartographic Acquisition and Map-Production Guide

**Research updated:** 29 October 2026. [MAP_CATALOG.csv](MAP_CATALOG.csv) contains **94 source records**, including **29 period-original, institutional series or period-revision references**, **44 selected topographic sheet leads**, and additional older or later materials. [Map-only index](media/reference-only/maps/INDEX.md) · [period overview](MAPS_1980_1989.md) · [rights](RIGHTS_AND_PROVENANCE.md).

## What has actually been acquired?

This archive has *verified metadata and clickable original-source locations*. It has not downloaded, copied, relicensed or converted 94 third-party map images. Three categories of evidence must remain visibly different:

- **Digitized original period map:** a period-printed map visible through the source site, such as the 1988 railway map on Wikimedia Commons and on Spomenik Database, or the 1986 monuments tourist map. Check the original publication line and scan-reproduction rights separately.
- **Period original, physical-copy identified:** a museum or catalog confirms an item was printed in 1981–1989, but a usable download or its redistribution rights have not been established, such as the 1988 Geokarta Belgrade plan and 1988 Yugoslav road map held by the University of Maribor Library.
- **Historical topographic series / prospective acquisition:** a library/institution describes a series or holds sheet indexes, but a particular sheet edition, rights, download and geo-accuracy are unverified. Croatian TK100/TK200 production-series records are not 55/18 individually catalogued map binaries.

## High-priority originals and source-specific next steps

| ID | Source | Original date | Access status | Action |
| --- | --- | --- | --- | --- |
| `POL-MAP-1981-CIA-POL` | [Library of Congress 1981 CIA political map](https://www.loc.gov/item/2011587053/) | 1981 | Scanned online | Check item-specific LOC rights and fetch authorized original image, retain LCCN/base number |
| `POL-MAP-1981-CIA-RELIEF` | [LOC 1981 CIA relief map](https://www.loc.gov/item/2011587054/) | 1981 | Scanned online | Pair with political map; compare hydrography, terrain and spatial extent |
| `POL-MAP-1981-82-SISAK-VGI` | [Croatian History Museum VGI catalogue](https://kartevgi.hismus.hr/en/e-catalog/belgrade/) | 1981 and 1982 | Museum item HPM-102650 | Locate/restudy the two original sheets and verify museum image permission |
| `POL-MAP-1988-JZ-RAIL` | [Wikimedia original 1988 JŽ–Geokarta railway map](https://commons.wikimedia.org/wiki/File:Yugoslavia_Railway-Lines_1988.jpg) | 1988 | High-resolution source image | Check the Commons uploader's conflicting 1940 description in history and the Yugoslav official-publication public-domain rationale independently |
| `POL-MAP-1988-RAIL-SPOMENIK` | [Spomenik Database zoomable image](https://www.spomenikdatabase.org/1988-map) | 1988 | Interactive image | **Same underlying railway map**; use multilingual symbols and route references without counting as another map edition |
| `POL-MAP-1986-MEMORIALS` | [Spomenik Database memorial tourist map](https://www.spomenikdatabase.org/1986-map) | 1986 | Interactive image | Check original legend/creator and page copyright; can ground 1980s commemorative roadside signage |
| `POL-MAP-1988-89-BELGRADE-GEOKARTA` | [CiNii 1988 Belgrade city map](https://ci.nii.ac.jp/ncid/BA75523445) | 1988 | Physical library item | Seek permitted high-resolution scan; 1:20k map, municipal wards, eight insets, transit keys |
| `POL-MAP-1988-YU-AUTO-UKM` | [COBISS/UKM 1988 road map](https://plus-legacy.cobiss.net/cobiss/si/en/bib/ukm/35263233) | 1988 | Held by University of Maribor Library | Request reading-room reproduction access and licence; 4th ed., 1:1,550,000 |
| `POL-MAP-1982-YU-AUSTRIA` | [Austrian State Archives 1982 tourist map](https://www.archivinformationssystem.at/detail.aspx?ID=2139619) | 1982 | Archive catalog item | Contains eight illustrated routes; archive consultation permission does **not** automatically authorize scan republication |
| `POL-MAP-1989-MICHELIN-991` | [CiNii Michelin map 991](https://ci.nii.ac.jp/ncid/BC14808956) | 1989 (©1988) | Physical museum library item | Rights remain commercial; use for route/street-symbol research without copying artwork |
| `POL-MAP-1980-84-CRO-TK100` | [Croatian DGU series registration](https://registri.nipp.hr/izvori/11) | 1980–1984 | Official historical coverage series / WMS | Request vintage sheet images and permission; **55 sheets** at 1:100k, not independently ingested |
| `POL-MAP-1982-86-CRO-TK200` | [Croatian DGU series registration](https://registri.nipp.hr/izvori/11) | 1982–1986 | Official historical coverage series / WMS | Request vintage original publication dates; **18 sheets** at 1:200k, not individually ingested |
| `POL-MAP-1985-86-SLO-VGI25` | [Slovenian geodetic survey history](https://dlib.si/details/URN:NBN:SI:DOC-N9BMLCDV) | Map state 1985–1986 | Technical description of retained source material | Identify source plates and historical TK25 reproductions; do not mistake Slovenia's 1995–96 DTK25 for the 1980s |

## Additional institutions and catalogue leads

- **[Serbian Military Geographical Institute](https://www.vgi.mod.gov.rs/lat/221/karte-bivse-jugoslavije-221)** lists former-Yugoslavia maps at scales 1:25k, 1:50k, 1:100k, 1:200k, 1:300k, 1:500k, 1:750k and 1:1m. Request dated sheet indexes and legal access, rather than assuming each scale is available online.
- **[University Library Svetozar Marković map portal](https://maps.unilib.rs/)** indexes historic Yugoslavia, Serbia, the Balkans, Adriatic and other thematic groups; identify 1980s item dates and rights separately.
- **[COBISS Slovenia historical cartographic search](https://plus-legacy.cobiss.net/cobiss/si/sl/bib/search?c=ar%3D7518819&r1=0)** returns 1981, 1982, 1984, 1986, 1987 and 1989 *Socialistična federativna republika Jugoslavija* map items, including a reissued map still crediting Ivan Selan after his 1981 death. Determine whether multiple entries are editions, reproductions or copies of one design.
- **[COBISS Serbian-language catalogue](https://plus-legacy.cobiss.net/cobiss/sr/sr_latn/bib/204791815)** documents 1980, 1982, 1985 and 1986 foreign-language tourist map editions; some may share a base design and should be linked as related editions in future normalization.
- **[Croatian History Museum cartographic editions](https://kartevgi.hismus.hr/en/editions/)** documents historical domestic topographic mapping and the Sisak 1981/82 assembly as its most recent map in that collection.

## A consistent source-ingestion lifecycle

1. **Discover:** record stable identifier, precise map title, language, holding library or museum, persistent URL and exact item designation. Prefer an *item page* to a general Google search or gallery.
2. **Authenticate:** separately enter original publication year, source survey year, revision year, depicted map state and later online scan/upload year. If only a range is known, preserve it as a range.
3. **License:** record rights *of both cartographic original and digital reproduction* (and its attribution). Distinguish consultation permission, public access, public-domain claim, reuse licence and permission to ship commercial/open-source game media.
4. **Preserve:** for any cleared downloaded original, record filename, SHA-256, byte size, download timestamp, source URL, scale, projection, edition, scan DPI and source institution; keep an unmodified archival original alongside any derivative.
5. **Extract:** prepare geospatial registration with grounded control points (road/rail crossings, valleys, coastal reference) and document RMS error. Never pretend unlocated historical poster maps have reliable GIS coordinates.
6. **Compare:** check dated later/earlier editions against period roads, settlements, street names, rail lines and borders; annotate temporal uncertainty, particularly around Kosovo/Vojvodina constitutional status and fictional events from 1984 onward.
7. **Produce:** use the source as evidence for an *original* RTS map or UX graphic, not an automatic playable map. Define walkable/vehicle/passable classes, bridge edges, depot locations, control-point markers and neutral traffic with human review and scenario balancing.
8. **Validate:** game-map acceptance includes adequate traversability, distinct civilian versus military zones, historical toponymy, legal source-to-asset provenance, and no anachronistic post-1991 features.

## Geographic production queues

| Area | Research priority | Concrete RTS map cues |
| --- | --- | --- |
| Slovenia | Ljubljana, Maribor, Jesenice and Ptuj dated urban plans; 1985/86 VGI TK25 | Alpine rail/road corridor, Drava, industrial towns, border posts |
| Croatia | Sisak 1981/82, Zagreb 1981, rail and road crossings, Rijeka, Split, Istria and Slavonia | Sava/Kupa industrial river junctions, ports, highway corridors |
| Bosnia and Herzegovina | Sarajevo dated pre-war city plans, Zenica mines/steelworks, Mostar, Banja Luka, Tuzla | Mountain passes, rail tunnels, valley settlements, industrial facilities |
| Serbia | 1988 Belgrade map, Niš crossroads and Danube basin | Federal administration, bridges, rail marshaling and road junctions |
| Vojvodina | Novi Sad, Subotica, Zrenjanin and agricultural regional plans | Flatland river logistics, grain distribution, power grid |
| Kosovo | Priština, Trepča, Peć, Prizren and border-highland plans | Mining transport, urban protest/assembly locations, mountain chokepoints |
| Macedonia | Skopje 1980s plans, Veles, Tetovo, Ohrid, Bitola | Vardar transport/rail corridor, highland borders, industrial areas |
| Montenegro | Titograd, Nikšić, Bar, Kotor and coastal routes | Port supply, steep valleys, Adriatic military/civilian traffic |

## Source ethics and historical separation

The real 1980–1989 Yugoslavia is **historical evidence**; the fictional Iron Ascendancy, Red Directorate, Atlantic Concord, Meridian Pact and The Unaligned are **game content**. Administrative units are not ethnic combat groups. Respect place-name variants and the distinction between contemporary politics and present-day successor-state borders.

**Repository state:** metadata only for this expansion; no newly imported third-party map images, PDFs or copyrighted art. The public-domain claim for the 1988 railway file remains *unverified for republication*, so it is not bundled despite its online availability.
