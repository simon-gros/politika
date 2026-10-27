# POLITIKA — Archive rights, provenance and evidence policy

The archive provides evidence for historical grounding, not a library of automatically reusable game art. Borrow the proven physical and historical *properties* of sources and create original game assets; copying source pixels, trademarks, posters, audio and text requires its own legal permission.

## Rights classes in `CATALOG.csv`

- `CC-BY-SA-4.0`: source page explicitly identifies Creative Commons Attribution–ShareAlike 4.0. The four Milan Suvajac 1982 tram photographs use this class on Commons. Reuse is possible only in compliance with full licence obligations (credit, link, indication of changes and share-alike as applicable). A share-alike image embedded in a game is **not** a blanket permission to incorporate incompatible derivative artwork without review.
- `copyrighted-reference-only`: study, metadata, period reading, design inspiration only; do not mirror/reproduce in the repo or distribute within the game by default.
- `rights-vary-by-item`: archive/institutional collection index; check *each* artefact's rights and permission before import. This is not a media licence.
- `unknown`: treat as copyrighted and reference-only until permission is established.
- If `public-domain`, `CC-BY`, `CC-BY-NC` or a specific institutional licence arises later, record exact version, holder, URL, scope and obligations. NC licences do not automatically permit game distribution or promotion.

The Olympic Museum explicitly limits its website content to private, educational, editorial and non-commercial purposes. Its original Olympic poster designs, wordmarks, mascot, and the IOC Olympic rings must **not** be copied into released game art simply because a web image is available. Museum images, actual federal coats of arms, trade logos and printed period newspaper pages can have separate rights concerns even if the subject is historically relevant.

## Cartographic rights, map dates and original game assets

The [map catalogue](MAP_CATALOG.csv) records map publication or known revision date, geographic coverage, scale, source institution, edition/sheet identifier, period status and rights. A map printed in 1996 can be useful for persistent geography without providing reliable 1985 roads, jurisdiction or buildings.

Before reusing an image, verify item-level rights: a Library of Congress digital record or US government agency attribution is not by itself a blanket licence for every hosted reproduction or attached content. Croatian History Museum map images are research-only absent permission. Do not mirror whole protected GeoPDF series or convert scanned maps into distributable game textures without authorization.

Create original game political maps, terrain textures and UI cartography informed by references; preserve source ID, period, license, game-asset derivation and historical-vs-fictional distinctions.

## Evidence vs. assets

For each `ref_id`, distinguish **source evidence** (what can be learned) from **incorporated material** (what is actually placed into an asset). A photo that establishes a tram's roof geometry can be an evidence source without being a texture. Retrospective writing establishes a reported or argued historic claim, but is not a substitute for contemporary evidence of clothing colours, exact street furnishings or historical appearance.

The archive is also **not** a claim of historicity for the Story Outline. Tito historically died in 1980. In the game he survives and is killed in 1984, which is a deliberate fantasy. No actual individual, movement, ethnic population or Yugoslav institution should be falsely credited with the fictional assassination, double deception, fake intelligence, or faction-specific sabotage.

## Metadata rules

Every row should provide `ref_id`, title, creator/source institution, stable URL or shelfmark, historical date (or an explicit unknown range), location, medium, subject tags, proposed gameplay/art use, `time_relation`, `evidence_confidence`, `canon_relation`, current `rights_status`, `repository_status`, checked date and a limitations note. POLITIKA's catalogue extends its source records with game-specific provenance and licensing metadata.

- **A — direct:** original period artifact with reliable dating/provenance or primary document *metadata* for the relevant year; it does not guarantee the complete contents were inspected.
- **B — strong:** museum research, competent secondary academic/official history or reliably contextualized period artefact.
- **C — contextual:** useful but incomplete, brief, ambiguous or differently framed evidence.
- **D — discovery lead:** linked collection may contain items for 1980–1989, but none have been confirmed on the cited page. Never promote to direct reference without verifying an item.
- `date_precision` distinguishes exact, year, range, academic-year, not-before, approximate end and **research-target-range**. Publication year is not photographic event year.
- `time_relation` distinguishes contemporary, retrospective, near-period secondary, institutional metadata and collection lead; language and subjective standpoint matter.

## Digital hygiene and public Git policy

- Version **catalogue, index, source notes, permissions, original study sketches and small cleared media** in Git.
- Keep downloaded archives, third-party scans, unlicensed videos, protected posters, music and source photos outside the public repo. If an archival item is merely linkable, it belongs in `media/reference-only/INDEX.md`, *not* as a committed binary.
- Put separately cleared binaries only under `media/licensed/`, and accompany each with a manifest entry: `ref_id,filename,creator,source_url,licence,licence_url,credit,modifications,approval_date,checksum`. This initial research pass commits **no media binaries**.
- Copyright and personality/privacy rights can differ; especially avoid turning real people in a politically sensitive archive into fictional enemy combatants or covert criminals.
- Attribution must survive future asset export, packaging and promotional use. Do not erase source signatures, licences or authors from catalogues.
- If a source page becomes unavailable, preserve stable bibliographic metadata, discovery date and archival identifier, not an unauthorized mirror copy.

## Production review

1. Confirm item identity, city and precise date; verify period build/state rather than rely on a modern photo.
2. Read item-specific licence text and attribution, not the hosting website's general copyright footer.
3. Record what part of the final model is *observed* and what is *invented* for RTS readability.
4. Check original-world cultural accuracy separately from POLITIKA's intentionally fictional timeline.
5. For protected period documents, invent convincing but original-looking text and layout, rather than copying scanned original pages.
6. Require reference IDs in PRs for major faction-neutral environments, period vehicles, historical civic interfaces, mission props and original archival-inspired posters.

**Historical and licence review:** 2026-10-27. Policies should be rechecked before publishing assets.
