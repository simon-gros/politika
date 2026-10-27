# POLITIKA — Versioning, Build Identity & Release Management Roadmap

**Status:** canonical policy and standalone tooling introduced; Unreal runtime integration and playable releases not yet implemented.
**Initial baseline (27 October 2026):** 0.1.0-dev.0, foundational design/documentation stage.
**Authoritative source:** [version.json](../version.json).
**Reference:** the Marburgum versioning/release management model, adapted for a multi-platform Unreal RTS.

## 1. Semantic version policy

POLITIKA uses Semantic Versioning 2.0.0: MAJOR.MINOR.PATCH[-PRERELEASE][+BUILD].

- Before 1.0, MINOR denotes a significant verified development milestone, PATCH denotes compatible corrections and a prerelease suffix denotes unfinished work toward a milestone. Initial 0.1.0-dev.0 establishes an identity and does **not** imply a playable 0.1.0.
- Use development stages dev.N → alpha.N → beta.N → rc.N → stable. Move stages only when demonstrated project maturity justifies the change.
- From 1.0 onward, MAJOR signals a deliberately broken supported compatibility contract; MINOR a backward-compatible feature/content release; PATCH a backward-compatible fix.
- Never bump the version for every commit, historical archive research, minor documentation edit or unimplemented gameplay concept.
- Semantic build metadata (for example +ci.27) improves traceability but **does not affect ordering**. CI build number and source Git SHA are separate.
- The game version is shared across Linux, Windows, SteamOS/Steam Deck and Android; any future consoles or Apple ports use the same identity. Platform-specific build/store numbers may differ, and different platforms can ship at different times.

## 2. Canonical manifest and separate compatibility identities

One hand-maintained root [version.json](../version.json) contains:

| Field | Initial value | Change criterion |
| --- | --- | --- |
| game_version | 0.1.0-dev.0 | Verified milestone or release warrants a new SemVer |
| network_protocol | 1 | Incompatible on-wire command, replication, or session contract |
| save_schema | 1 | Incompatible serialization of local campaign/skirmish/match saves |
| persistence_schema | 1 | Incompatible durable profile/progression or later database storage |
| world_schema | 1 | Incompatible map, mission, generated world, or gameplay content-data format |

The initial integer values reserve version-one contracts, **not implementations** of networking, saving, databases or map serialization. Bump counters only when corresponding implemented contracts change incompatibly, never to follow game_version; never reuse or decrease published counters. POLITIKA is offline-first: networking is optional future architecture, not a login requirement.

Content *format* and content *revision* are different. Future build tooling should derive a gameplay content hash from a canonical manifest of packaged maps, missions, unit/faction definitions, rules and resources, not manually edit another version. Replays may eventually need a separate replay_schema if their serialized contract changes independently.

## 3. Build tooling and identity

The Python standard-library-only [tools/version.py](../tools/version.py) implements:

- python3 tools/version.py print — show the canonical SemVer.
- python3 tools/version.py check — validate SemVer, compatibility counters and any matching Git tag; print build identity.
- python3 tools/version.py identity — emit JSON with game/protocol/schema identity plus Git revision, build number and dirty flag.
- python3 tools/version.py generate — produce build/generated/PolitikaVersion.generated.h for future Unreal C++ integration (custom --output accepted).
- python3 tools/version.py check --release — require an exact vVERSION tag, clean source and the matching checked-out commit.

Generated C++ constants live under politika::version and cover GAME_VERSION, NETWORK_PROTOCOL, SAVE_SCHEMA, PERSISTENCE_SCHEMA, WORLD_SCHEMA, BUILD_NUMBER and GIT_REVISION. The version is **never** hand-copied into another Unreal config, module, data asset or HUD.

Build number derives from POLITIKA_BUILD_NUMBER or GITHUB_RUN_NUMBER (otherwise local); Git revision derives from GITHUB_SHA or the local checkout, with -dirty when appropriate. Neither belongs in version.json. A release package should later carry a generated build manifest with exact Unreal source/toolchain pin, platform, target, configuration, content/cook identity and SHA-256 checksums. Do not copy Marburgum-specific Godot/GDExtension tooling or world-protocol assumptions.

## 4. Coding agent/version advancement authority

Agents and contributors may advance game_version or dev/alpha/beta/rc counters when their *implemented and validated* changes justify it, without seeking a separate approval for each bump. Inspect the existing version and changelog, choose the smallest defensible forward increment, update the canonical manifest plus CHANGELOG.md/release notes, and document the milestone evidence.

Do not bump for routine commits, exploratory design, archive curation, planned platform targets or graphics references. Keep protocol/save/persistence/world counters independent and add migration/compatibility tests when changing an actual contract. Never move a published tag or reuse an already distributed version.

## 5. Runtime and Unreal build integration plan

After executable game source exists:

1. Generate the C++ header before compiling client/standalone and any later server; expose it through a small platform-neutral runtime service.
2. Show short SemVer in menu/settings/About and full build, Git and compatibility identities in developer diagnostics, startup/crash logs and bug-report bundles.
3. Embed save_schema and content identifiers in local saves, and validate them before changing existing progression. Support verified older schemas with ordered migrations and rollback/recovery copies.
4. If multiplayer is added, compare network_protocol during the handshake and reject incompatible clients with a clear reason before sending gameplay state. SemVer equality alone is not a network compatibility rule.
5. Stamp replays/recorded command streams with version, content and rules identity; test equivalent outcomes on targeted platforms rather than assuming cross-platform determinism.
6. Keep packaged platform metadata derived from the canonical version; Android versionCode and storefront-specific package counters are not new game versions.

## 6. Tagging and release channels

Use immutable **annotated** Git tags of the form v0.1.0-alpha.1, v0.1.0-beta.1, v0.1.0-rc.1, v0.1.0, etc. The tag text must equal v + game_version. Ordinary development commits do not get version tags. Do not tag the initial unverified design baseline as a packaged release.

| Channel | Meaning |
| --- | --- |
| dev | Recent development artifacts, potentially incomplete |
| playtest | Deliberately promoted tester builds |
| stable | Public release after acceptance gates |

The distribution channel does not replace the SemVer. GitHub Releases should document shipped packages, known issues and schema migrations, with checksums and build manifests. Label alpha/beta/RC GitHub Releases as pre-releases; never imply a target platform is supported before a real package has passed validation.

## 7. Save, persistence, map and replay compatibility

- Store explicit schema identity at serialization boundaries, and distinguish data structure changes from altered faction balance or missions.
- Chain supported migrations (1 → 2 → 3), test current-read/current-write and older-read/current-migrate, and preserve recoverable backups on failure.
- Preserve campaign progress, mission outcomes, resources, faction/unit ownership and player choices unless a documented migration intentionally changes them.
- Use stable IDs and upfront validation for maps, factions, units and mission/scenario data. Avoid silently consuming outdated or incompatible world content.
- Future network matches and replay files must check their own compatibility identities, not just a matching menu version.

## 8. Automated validation and promotion

The lightweight [.github/workflows/versioning.yml](../.github/workflows/versioning.yml) workflow validates the manifest, executes unit tests and checks deterministic header generation on PRs, main and version tags. Passing this gate certifies **only versioning infrastructure**, not a playable release, a packaged Unreal build or Steam Deck/Android compatibility.

When playable builds exist, candidate promotion additionally needs real Unreal compile/cook/package evidence, verified platform-specific input/UI and hardware performance, RTS/gameplay acceptance, save migration checks and checksum/reproducibility evidence. Run qualified target tests on Linux, Windows, SteamOS/Steam Deck and Android separately before promoting their supported status.

## 9. Delivery status

| Phase | Status |
| --- | --- |
| V0: Canonical manifest, policy, changelog and contributor rules | Introduced |
| V1: Version validator, build identity header generator and tests | Implemented as standalone tooling |
| V2: GitHub versioning CI | Workflow added; execution must be verified |
| V3: Actual Unreal compile/runtime integration and menu display | Pending executable source |
| V4: Save/map/protocol/replay compatibility fixtures | Pending runtime systems |
| V5: Packaged cross-platform playtest releases and manifests | Pending validated builds |
| V6: Stable releases, migration windows and tested rollback | Pending release maturity |

Related documents: [Engine Outline](ENGINE_OUTLINE.md), [Proposed Engine Layers](PROPOSED_ENGINE_LAYERS.md), [Platform Support](PLATFORM_SUPPORT.md), [Main README](../README.md), [Changelog](../CHANGELOG.md).
