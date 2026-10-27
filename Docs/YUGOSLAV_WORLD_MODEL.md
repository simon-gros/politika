# POLITIKA — Yugoslav World and Campaign Systems Model

**Status:** Historical-world design synthesis and candidate gameplay mapping, 27 October 2026. This is **not** a catalogue of implemented features, a new canonical event timeline, a gameplay content schema, or historical certification.  
**Period:** Historical reference layer 1980–1989; POLITIKA alternate-history campaign 1984–1989.  
**Purpose:** Translate the [RTS research model](RTS_GAME_ONTOLOGY.md) and [minimal engine gameplay contract](ENGINE_OUTLINE.md#minimal-rts-gameplay-contract) into a place-specific world that represents Yugoslavia as a complex federation rather than a generic military arena.

**Authority and provenance:** [Story Outline](STORY_OUTLINE.md) determines fixed fictional events and pacing; [Concepts Outline](CONCEPTS_OUTLINE.md), [Match Structure](MATCH_STRUCTURE.md), and specialist systems own gameplay rules; [Historical Chronology](../Archive/HISTORICAL_CHRONOLOGY_1980_1989.md), [Political History](../Archive/POLITICAL_HISTORY_1980_1989.md), [Political Events Catalogue](../Archive/POLITICAL_EVENTS.csv), and [Map Archive](../Archive/MAPS_1980_1989.md) provide historical research rather than automatic game canon; [Production Standards](PRODUCTION_STANDARDS.md) and [Archive Rights Policy](../Archive/RIGHTS_AND_PROVENANCE.md) govern verification and reuse. All examples below are candidate designs unless an owning canon document explicitly establishes them.

## 1. Three different histories must remain distinguishable

| Layer | What belongs here | What it does **not** establish |
|---|---|---|
| **Historical record (real 1980s)** | Constitutional arrangements, six socialist republics, Kosovo and Vojvodina as autonomous provinces within Serbia, self-management, federal institutions, JNA, Territorial Defence, period geography, actual economic stress and documented events | That real movements or institutions committed fictional crimes; that actual events follow the same sequence once history diverges |
| **POLITIKA alternate-history world** | Tito survives his **actual death on 4 May 1980**, is assassinated in **1984** by the fictional Iron Ascendancy; a small Unaligned inner circle installs a separate body and voice double and hides the murder; corroborated exposure and war follow in **1989** | A real account of Tito, Yugoslav officials, the 1980s protests, nuclear espionage, or the end of Yugoslavia; actual dissolution and wars began later |
| **Match and campaign simulation** | Scenario-specific locations, unit spawns, institutional orders, economic pressures, objective triggers, discovered evidence, local alternative outcomes | Permission to change the fixed story truth or to assign all historical actors a predefined faction alignment |

The **historical record** and the **fictional world's objective truth** are separately authoritative in different contexts. A genuine scanned 1985 newspaper is historical evidence; an invented in-game 1985 newspaper claiming Tito addressed a crowd may be an *intentional lie within the game's alternate timeline*. Neither should be confused with the other.

**Continuity check:** The real post-1980 collective rotating presidency is historical reference, **not automatically POLITIKA's governing institution in 1984**: the living-Tito divergence means institutional arrangements and officeholders must be explicitly resolved in the [historical decisions ledger](../Data/historical_decisions.json) before being represented as unchanged. Do not invent a constitutional replacement just to simplify a mission.

## 2. From abstract RTS concepts to this particular country

| Shared engine concept | POLITIKA's Yugoslav-world counterpart | Why the separation matters |
|---|---|---|
| **World / time** | 1984–1989 federal environment, campaign year and phase, operational map, evolving crisis | One territory can be politically different across acts without rewriting terrain geometry |
| **Regions / access** | Socialist republics, autonomous provinces, municipalities, border districts, restricted zones, railway corridors, ports | Geographic containment differs from legal jurisdiction and military occupation |
| **Entity / site** | Civilian, JNA unit, Territorial Defence formation, police patrol, public factory, road bridge, transmitter, hospital, archive, diplomatic mission | A site can be strategically valuable without being a conventional capturable base |
| **Identity / affiliation** | Federal, republican, provincial or municipal institution; workforce/cooperative or enterprise; fictional foreign faction | Legal origin is not equivalent to controller, allegiance or knowledge of conspiracy |
| **Agent / controller** | Playable faction agent, local civil authority, federal security, military unit commander, inner continuity cell | The Unaligned remains one NPC-only faction, but its internal services need not share intentions or knowledge |
| **Relations / rules** | Jurisdiction, inspection powers, passage rights, public order, military access, trade licences, secret directives | Neutrality is not lack of ownership, and physical capability is not legal permission |
| **Resources / production** | Enterprise output, power, fuel, transport capacity, scarce materials, credits, working time, service continuity | Historical self-management suggests varied institutional/enterprise interests; it does not require a simulated national accounting system |
| **Actions / processes** | Patrol, escort, inspect, harvest, construct, defend, repair, infiltrate, censor, transport, broadcast, sabotage | The same order/admission/event grammar can resolve conventional combat and noncombat mission objectives |
| **Information / evidence** | Radio broadcast, newspaper, archive document, hospital record, witness, forged intelligence report, intercepted dispatch | What occurred, what one institution believes, and what the public hears are different things |
| **Objectives / consequences** | Hold crossing, keep hospital operating, protect witnesses, obtain corroboration, prevent broadcast tampering, evacuate civilians | Victory can mean preservation of evidence or institutional functioning rather than annihilation |

This mapping is a *design vocabulary*, not a mandate to create ten separate systems now.

## 3. Political and administrative geography as a layered world

Historically, the SFRY contained **six socialist republics**—Slovenia, Croatia, Bosnia and Herzegovina, Serbia, Montenegro, and Macedonia—and **two autonomous provinces inside Serbia**, Vojvodina and Kosovo. The 1974 constitutional order gave federal units substantial representation and distinct administrative powers. Local municipalities and socially owned enterprises mattered alongside federal authorities, the League of Communists, and the military. This was neither a unitary state in which Belgrade issued every local order nor a collection of independent nation-states. See the [historical chronology](../Archive/HISTORICAL_CHRONOLOGY_1980_1989.md), the comparative political history [1–3], and the contemporary 1985 country profile [4].

The world should be assembled from **overlapping functional layers**, not a single regional owner flag:

| Layer | Examples of in-world authority or activity | Candidate RTS consequence |
|---|---|---|
| Federal institutional | Common state institutions, diplomacy, federal policy, border/central security | Federal alerts, orders, permission overrides, crisis response |
| Republican and provincial | Administrative bodies, party organizations, regional institutions | Different willingness to obey emergency instructions, distinct local access/response |
| Municipal / urban | Road administration, public transit, civilian services, police and local archives | Local restrictions, service outages, mission objectives involving actual communities |
| Military and defence | Yugoslav People's Army (JNA), Territorial Defence, military installations | Military jurisdiction, garrisons, escorts, defensive engagement |
| Enterprise and self-management | Industrial enterprise, transport operator, workforce/management bodies, distribution | Working production, delivery dependencies, strikes or interruptions where supported |
| Media and communications | Local newspapers, broadcasters, studios, transmitters, relay stations | Distribution and verification of information, emergency broadcasts, censorship |
| Human and civil society | Civilians, workers, students, journalists, doctors, archivists, witnesses | Noncombatant activity, witness credibility, public reaction; never a monolithic ethnicity-controlled faction |
| Hidden continuity apparatus (fiction only) | Provisional Continuity Directorate and cleared operatives | Secrecy, exceptional directives and record suppression, with knowledge limited to the inner circle |

**Important game boundary:** none of these real administrative layers is an additional **playable** faction. The four playable foreign factions and one Unaligned NPC faction remain the match roster specified by [Match Structure](MATCH_STRUCTURE.md). The table describes possible **institutional roles and decision scopes within the Unaligned world**, not eight extra armies or independent multiplayer slots.

A municipality may refuse a suspicious document seizure even while its police patrol maintains ordinary traffic rules. A JNA commander can follow a conventional order without knowing its concealed narrative purpose. A provincial editor can question a broadcast without automatically discovering the assassination. This permits believable disagreement without granting every NPC full political agency or requiring a separate strategic AI for each organization.

### 3.1 Period-city and strategic site vocabulary

Candidates for *representative* environments, subject to dated map/asset review, include:

- **Federal and international administration:** Belgrade, with government corridors, transport exchanges, broadcasters, diplomatic traffic and institutional records.
- **Republican urban/industrial environments:** Ljubljana and Maribor; Zagreb and Rijeka; Sarajevo; Skopje; Titograd (the period name of today's Podgorica).
- **Autonomous provincial environments:** Novi Sad (Vojvodina) and Priština/Prishtina (Kosovo), with locally specific institutions rather than generic "Serbian city" treatment.
- **Transport and strategic terrain:** Slovenian Alpine approaches, Pannonian plains, the Sava and Danube corridors, Dinaric passes, Adriatic ports and coastal routes, tunnels, bridges and airfields.

These are **geographic design candidates**, not assigned locations for Tito's assassination, doubles, fabricated evidence, nuclear research, or the campaign's final transmitter. A cartographic place-name or junction must be verified against the *date depicted* and publication/revision year. [Map Archive](../Archive/MAPS_1980_1989.md) supplies the period and provenance policy.

### 3.2 Historical events as pressures, not scripts

The archive records the **1981 Kosovo demonstrations**, **1984 Sarajevo Winter Olympics**, changing press and youth culture, **1987 controversies surrounding Day of Youth imagery**, **1988 Slovenian JBTZ proceedings and other protests**, **1989 Trepča mining actions**, the **1989 Belgrade Non-Aligned summit**, and severe economic stress. Such material can suggest architectural settings, civilian traffic, headlines, collective debate, police restrictions, industrial stoppages and diplomatic delegations. These are *historically attested reference themes*, **not automatic 1984–1989 alternate-timeline mission events**.

In particular:

- The real 1984 Sarajevo Olympics is a strong reference for venue networks, urban transit, security logistics and graphic aesthetics; its precise role relative to the undated **fictional** 1984 assassination must be decided by campaign writers.
- Press debates, university activism, workers' mobilizations and regional dissent must not be retroactively explained as secret operations of the fictional Iron Ascendancy, Meridian Pact, or other invented powers.
- Real 1989 Yugoslavia did **not** collapse into the fictional war described in POLITIKA's 1989 ending; separate source chronology from the canonical alternate-history result.
- Sensitively disputed events require several sources and a reviewed ruling; no ethnic, religious, regional or civilian population gets automatic combatant, loyalty or hostility attributes. Institutions and individual actions determine the game's operational responses.

## 4. The terrain is also political infrastructure

The map should layer **physical traversability**, **institutional access**, **functional connectivity**, and **strategic influence**:

| Place / site type | Physical significance | Additional world-specific constraint or affordance |
|---|---|---|
| Bridge / railway interchange | Chokepoint or long-route connector | Military closure, engineering repair, traffic congestion, supply and civilian continuity |
| Border crossing / customs depot | Access to a route | Permits, inspections, illicit transport, customs enforcement, diplomatic status |
| Industrial complex | Physical site with production capacity | Workforce, energy, transport links, administrative jurisdiction, sabotage exposure |
| Refinery / energy facility | Input/output conversion or service | Fuel and power availability, emergency shutdown, protected infrastructure |
| Hospital / medical registry | Civilian service and location | Confidential records, protected personnel, records custody, emergency access |
| Broadcast studio / relay | Transmits messages across an area | Editorial/technical control, coverage, jamming/blackout, authenticity of recordings |
| Municipal / republican archive | Stores documents with provenance | Custody and accessibility, competing authorities, duplication risk |
| Police barracks / JNA installation | Deploys a security/military response | Jurisdiction, threat assessment, access rights, defensive orders |
| Market / depot / port | Movement and transfer of goods | Merchant contracts, delivery, local enforcement, control without military conquest |
| Neighborhood / residential block | Dense terrain with civilian presence | Evacuation routes, damage to services and noncombatants, movement restrictions |

**Semantic map rule:** crossing a passable bridge does not mean the crossing is authorized; holding a building does not mean one controls its communications network; occupying an archive does not imply that its records remain intact; destroying a transmitter does not equal successfully releasing evidence.

Maps should preserve the practical distinction between *geometry*, *movement profile*, *civil jurisdiction*, *operational capability*, and *knowledge access*. Use map profiles and scenario-specific authored exceptions rather than assuming every fictional map simulates all legal/economic layers.

## 5. Narrative truth, institutional knowledge and public belief

The game's conspiracy works only if four different states do not collapse into one:

1. **Physical/narrative truth:** Tito survives 1980, is killed in 1984, and remains dead thereafter. The separate doubles, original records, and perpetrators exist independently of who knows them.
2. **Institutional access and custody:** a medical clerk may know a document exists; a broadcaster may hold an edited master; an inner-circle officer may know both their significance and the hidden order. Access to a record is not equivalent to comprehending the entire plot.
3. **Reported/public picture:** the officially living leader is presented through statements, staged appearances and recorded speech. Public reports may be true, incomplete, stale, censored or fabricated.
4. **Agent-specific assessment and credibility:** different faction leaders, journalists, witnesses, provincial authorities and civilians may reach different conclusions based on records, testimony, experience, corroboration, and propaganda.

The factual truth of Tito's murder in the story is fixed and **cannot be player-voted into or out of existence**. Conversely, a simulated false intelligence report about a nuclear facility must never spawn that facility or satisfy an actual infiltration requirement.

### 5.1 A witness, an item of evidence, and a claim are different objects

| Story object | What it is | Meaningful gameplay relationship |
|---|---|---|
| Medical record | Physical or copied document, with creation and custody history | Can be recovered, authenticated, destroyed, forged, or matched to other records |
| Broadcast master | Audio/documentary artifact distinct from public transmission | Can be compared against a broadcast; control of studio ≠ control of content |
| Body double | Person with custody, agency and observations | Protection, extraction, testimony; not merely a capturable victory token |
| Voice double | Different person with different handlers and evidence | Separate testimony; cannot be merged with the body double |
| Witness testimony | Proposition attributed to a person and time | Requires context; may contradict another report, need not be conclusive |
| Intelligence report | Claim with purported and actual origins | Can be planted, intercepted, verified, disputed or discredited |
| Official communiqué | Published assertion attributed to an authority | May alter confidence and public narrative without changing objective truth |

**Historical archive provenance** (how developers verified a real source) and **fictional in-world provenance** (who authored, altered or possessed a dossier) are separate data domains. Neither should be substituted for the other.

### 5.2 Corroboration rather than collectible count

The [Story Outline](STORY_OUTLINE.md) requires **multiple independent lines of evidence** before the public revelation. The conceptual campaign rule is not simply "collect three documents": it is *obtain and preserve evidence from sufficiently independent sources, verify or confront contradictions, and disseminate it through functioning channels*.

A future objective system could distinguish *possession*, *authenticity*, *source independence*, *knowledge*, *public disclosure* and *institutional response*. The exact number of sources, credibility thresholds, puzzle logic, and mission scripts are **not yet specified**.

A possible causal chain is:

- Real evidence exists and is held by distinct custodians.
- A faction obtains a subset; competing actors may steal or suppress it.
- Testimony, archival dates, and original broadcast recordings independently corroborate the death, double arrangement and concealment.
- A public communications channel can disseminate verified findings while opponents can contest or interrupt transmission.
- Corroborated release in **1989** triggers the canonically fixed crisis; battlefield outcomes control who and what survives, but do not erase the revelation.

A false report can change an actor's next order; only verified story evidence can establish a qualifying fact where the scenario requires it.

## 6. Institutional crisis as a sequence of state changes

Political breakdown is not one global *Unaligned hostility* switch. It is a collection of **relationships, permissions, public confidence, institutional mandates and operational capacities**, changing at different rates and places.

| Phase | Fictional narrative context | What might change in a playable local map |
|---|---|---|
| **Ordinary administration** | Public Tito still considered alive by most, public life continues | Traffic, inspection, workplaces, transit, neutral patrols |
| **Secrecy and exceptional directives** | Inner circle protects records and doubles | Restricted sites, higher surveillance, unexplained closures |
| **Disputed legitimacy** | Contradictory evidence and regional dissent accumulate | Conflicting access decisions, inspection/refusal, local protests, protection of witnesses |
| **Loss of authority** | Credible cover-up evidence reaches competing institutions | Some orders rejected, assets secured or transferred, agencies disagree |
| **Fragmentation and open conflict** | 1989 public revelation and wider crisis trigger armed clashes in fiction | Routes blocked, local command divided, evacuations and defensive deployments |

This table describes **candidate scenario states**, not a fully simulated legitimacy statistic, guaranteed event order, or extra engine module. Different institutions may enter different states, and ordinary civilian services can continue operating amid security tension. The fiction should not conflate "public mistrust" with universal ethnic hostility.

## 7. Five-act campaign crosswalk

The act names and fixed discoveries below come from [Story Outline](STORY_OUTLINE.md). Proposed environmental fixtures are examples, not new mission canon.

| Campaign stage | Known story state | Historically informed environment | Candidate RTS interactions | Information / objective contract |
|---|---|---|---|---|
| **Prologue — The Empty Chair (1984)** | Assassination concealed; doubles installed | Federal security, sealed routes, border and hospital/transport services | Escort, quarantine, inspect, intercept without indiscriminate combat | Contradictory times and unusual security; *no early full revelation* |
| **Act I — One Voice, One Country (1984–1985)** | Official appearances/broadcasts continue; witnesses fear exposure | Radio relays, ports, public transport, federal communications, period press | Defend relay, escort specialist, recover shipment | Establish inconsistent statements or fabricated appearances; murder not yet proven |
| **Act II — The Man Behind the Curtain (1986)** | Two distinct doubles become supportable as a finding | Urban archives, railway yards, studios, hospitals | Capture archive intact, keep a witness alive, inspect recording master | Distinguish evidence custody from verification; doubles uncovered, death unresolved for most |
| **Act III — A Federation of Secrets (1987)** | The 1984 death becomes credible to a few; responsibility unclear | Industrial regions, republic/municipal bodies, print culture, disputed orders | Defend archivist route, secure repository, coordinate with dissenting officials | Multiple records indicate assassination, but killer identity remains partial |
| **Act IV — The Republics Begin to Break (1988)** | Ascendancy role and official cover-up become provable | Local/federal command friction, demonstrations, disrupted services | Protect evacuation, maintain road/rail access, secure testimony | Evidence demonstrable but public release still obstructed |
| **Act V — The Last Broadcast (1989)** | Multiple sources reach public; institutions lose legitimacy; war erupts in fiction | Broadcaster and relay networks, ministries, ports, transport/civil protection | Protect witnesses, transmit corroborated account, rescue civilians, defend services | **Public revelation and ensuing war are fixed canon**, while local losses and gains vary |

The *separate* nuclear contest is governed by [Nuclear Weapons System](NUCLEAR_WEAPONS_SYSTEM.md). The Atlantic Concord and Red Directorate begin nuclear powers, the Ascendancy seeks a first **own-produced** bomb via eligible intelligence theft and resource/production gates, and the Meridian Pact has no dedicated nuclear campaign missions. **Producing**, **stealing**, **possessing**, and **deploying** a nuclear weapon are different events; a fabricated report cannot unlock real weapons research. The Ascendancy's qualifying own-production is an active-match instant-win rule where enabled, not a substitute for rewriting the main storyline's canonical 1989 revelation.

## 8. Four concrete scenario sketches for environment validation

These are *proposed fixtures for later design review*, not approved maps, missions, historical event reconstructions, or instructions to implement now.

| Fixture | Example setting and tension | What it tests |
|---|---|---|
| **Crossing and customs** | A Sava-region road/rail crossing serving federal patrols, local commerce and rival armed traffic | Geographic passability versus permission; inspection without automatic hostility; civilian-route continuity |
| **Industrial town and workplace** | A regionally administered factory and housing district amid economic pressure | Site ownership vs operability, power/fuel/transport dependencies, distinct workers/municipal/security controllers |
| **Broadcast and archive complex** | A Yugoslav urban radio network linked to an archive and local police jurisdiction | Distinct physical custody, technical control, permitted observation, forged claims, evidence preservation |
| **Provincial administrative district** | A Kosovo or Vojvodina-inspired administration with military and local authorities pursuing different lawful mandates | Provincial jurisdiction and conflicting orders; *no preassigned civilian ethnicity-to-combatant mapping* |

No proposed fixture may borrow a particular documented real protest or historical person's identity to stand for a fictional extremist operation. Likewise, a fictional nuclear research facility does not become a historical Yugoslav installation through a map label.

## 9. Concrete engine and content boundaries

Only these **general-purpose hooks** need to be preserved from the historical mapping while the data-driven prototype is being developed:

| Existing/forthcoming foundational contract | Minimal requirement now | Deferred use when justified |
|---|---|---|
| Map and region markers | Stable-ID map profile, playable bounds, slots, starts and objective positions, movement classes and reachability | Jurisdiction zones, transit networks, protected institutions and changing access |
| Typed actor definitions | Existing prototype unit/weapon assets and distinct runtime identities | Civilians, police, territorial defence, factories, records and broadcast equipment |
| Controllers/participants | Authoritative player/AI command admission and correct separate Unaligned controller | Local/federal institutional mandates, contracts and divided loyalty |
| State, events and objectives | Tested combat, capture/progress events and scenario victory | Evidence custody, verified disclosure, institutional refusal and conditional rights |
| Visibility/interface boundary | Presentation reads authoritative match state without changing it | Per-agent observations, last-known state, forged reports and public confidence |
| Definition validation | Reject invalid gameplay assets and illegal map configurations before a match | Period-location references, dated institutional content, profile-specific narrative prerequisites |

**Explicitly deferred:** simulation of an entire federal legal system; autonomous every-citizen AI; complete enterprise/self-management economic accounting; a universal belief engine; a reconstructed chronology of every 1980s political event; historical-person crime attribution; full evidence-chain quest machinery; and production-quality countrywide historical maps. None is a prerequisite for the next data-driven content/map validation milestone.

### 9.1 Initial authored content implications

Use **typed role/capability markers** in prose and future schemas only when a supported use case requires them. An archival records office, radio relay, refinery, JNA barracks and road crossing are not all the same generic "building" in terms of interactions. They may share placement/collision/identity conventions but have different optional capabilities.

The first map fixture may remain fictional and minimal. When a historical campaign map is eventually proposed, carry a separate *date/setting review* referencing the archive, approved divergence decision, source provenance and actual gameplay constraints. Never embed a real historical event as a guaranteed mission script because a reference row exists.

## 10. Historical research and presentation safeguards

- **Time and legal status:** Record at least scenario year, political-region identifier, source depicted date, source publication/revision date and approved alternate-history departure for period-specific authored maps/sets when those assets are commissioned.
- **Constitutional and institutional accuracy:** Do not mistake municipal police for the federal military, a party organization for an elected republic institution, or a geographically nested province for a non-autonomous district. The alternate Tito continuity must be reviewed separately.
- **Infrastructure and terrain:** Verify bridges, rail yards, city plans, period names and industrial facilities against dated sources before claiming a historically faithful reconstruction. Modern basemaps are location aids, not 1980s proof.
- **Culture and civilians:** Preserve linguistic, regional, architectural, religious and workplace diversity without simplifying ethnicity into a combat team or claiming all civilians supported a single ideology.
- **Political history:** Avoid portraying real 1980s protest movements, trade unions, named journalists, victims or real public figures as participants in invented assassinations, fake flag incidents or military conspiracies.
- **Rights and authenticity:** Archive sources grant evidence, not redistribution rights; use original POLITIKA art, text, maps, newspapers, sound and interfaces unless a particular asset is cleared for reuse.
- **Status honesty:** Narrative/environment sketches are proposed until promoted by the story and historical review owners; engine behavior needs executable tests before being marked implemented.

## 11. Research basis and owning documents

**Project sources:** [Story](STORY_OUTLINE.md), [Concepts](CONCEPTS_OUTLINE.md), [Engine](ENGINE_OUTLINE.md), [RTS research](RTS_GAME_ONTOLOGY.md), [Counterintelligence](UNALIGNED_COUNTERINTELLIGENCE.md), [Match Structure](MATCH_STRUCTURE.md), [Historical Chronology](../Archive/HISTORICAL_CHRONOLOGY_1980_1989.md), [Political History](../Archive/POLITICAL_HISTORY_1980_1989.md), [Map Archive](../Archive/MAPS_1980_1989.md), [Production Standards](PRODUCTION_STANDARDS.md).

**Additional historical context** (used for high-level institutional/economic understanding, not to approve a game asset):

1. Sabrina P. Ramet, *Nationalism and Federalism in Yugoslavia, 1962–1991*, [Indiana University Press, internal balance of power](https://publish.iupress.indiana.edu/read/nationalism-and-federalism-in-yugoslavia-1962-1991/section/9bfa5625-cfc5-4a13-82f2-941cd567d041) — federal-republican-provincial relations.
2. "[Rokkan Rules? Communist Elites and the Choice of Electoral Systems in the Yugoslav Republics, 1989–1990](https://pmc.ncbi.nlm.nih.gov/articles/PMC11073039/)" — representation, League of Communists organizations and competing regional responses to economic crisis.
3. "[Kosovo in the 1980s — Yugoslav Perspectives and Interpretations](https://www.degruyterbrill.com/document/doi/10.1515/soeu-2021-0059/html)" — historiographical cautions and cross-republic perspectives.
4. [1985 CIA World Factbook Yugoslavia government profile](https://geographic.org/wfb1985/yugoslavia/yugoslavia_government.html) — period snapshot of federal structure; historical rather than alternate-world leadership.
5. [World Bank, Yugoslav economic instability and self-management](https://documents1.worldbank.org/curated/en/533291468766518413/pdf/multi-page.pdf) — contextual research lead, not a claim that every figure has been visually checked here.

The [historical decisions ledger](../Data/historical_decisions.json) and [rights register](../Archive/RIGHTS_AND_PROVENANCE.md) govern subsequent source-by-source promotion. No new historical asset, political ruling, source-catalogue entry, or gameplay definition is approved by this document alone.
