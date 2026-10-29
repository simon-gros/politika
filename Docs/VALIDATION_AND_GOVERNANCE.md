# POLITIKA — Development Validation and Documentation Governance

**Status:** Mandatory lightweight working policy from 27 October 2026, updated 29 October 2026. The Unreal project and bounded gameplay prototype exist, with scoped historical Linux Development acceptance evidence; this policy never infers a current build or hosted workflow from repository source. The Rust quality utility performs offline metadata and native core checks.

## 1. Single-source documentation hierarchy

1. *Implemented and tested source code* establishes observed behavior; a green tooling result never proves a nonexistent Unreal feature.
2. [Concepts Outline](CONCEPTS_OUTLINE.md), [Story Outline](STORY_OUTLINE.md), faction descriptions and specialized RTS gameplay documents own game rules and fiction. Resolve inconsistencies in their owning documents.
3. [Engine Outline](ENGINE_OUTLINE.md), [Proposed Engine Layers](PROPOSED_ENGINE_LAYERS.md) and [Platform Plan](PLATFORM_SUPPORT.md) own technical architecture and product targets.
4. [Historical Archive](../Archive/README.md) and its catalogs/rights policy own real-world evidence. [Production Standards](PRODUCTION_STANDARDS.md) owns research-to-asset approval methods.
5. [Versioning Roadmap](VERSIONING_RELEASE_MANAGEMENT_ROADMAP.md) owns version/schema changes. [Performance and Release Gates](PERFORMANCE_AND_RELEASE_GATES.md) owns validation thresholds and acceptance proof.
6. This policy and [Development Foundations](DEVELOPMENT_FOUNDATIONS.md) own validation sequencing. [Documentation Index](DOCUMENTATION_INDEX.md) provides navigation, not alternative authority. Root [README](../README.md) is an accessible overview, not the complete game-design or test record.

Use four distinct status labels in authored documentation: **design/proposed**, **source implemented**, **validated locally**, and **verified in packaged gameplay**. Add *platform-qualified* separately and only after real-device evidence. Never call a design entry an implemented game feature.

When editing a feature, update its owning document, associated regression tests, and status together. Avoid copying full concepts into the README, introducing duplicate systems, and preserving a stale “future” claim after actual code/tests are present.

## 2. Proportional development validation matrix

| Change scope | Required immediate checks | What remains unproven |
| --- | --- | --- |
| Narrative, descriptions, documentation | References/link hygiene, canon consistency, review of historical assertions; existing Rust docs/metadata audit where relevant | Unreal launch, gameplay balance, campaign outcomes |
| Historical archives and political/map source catalogues | Source ID and rights/date/category consistency; historical decision traceability; research-only versus approved asset status | Cartographic accuracy of uninspected items or copyright permission |
| Rust tools/manifests | Cargo formatting check, `cargo test --locked --all-targets`, `quality audit`, targeted malformed-data regression tests | Unreal builds and imports |
| Portable C++ RTS command/rules kernel | Compile/tests through `quality core` or standalone `bash tools/core-test.sh` when Cargo is unavailable, including missing/invalid/edge cases | Standalone test covers only the C++ assertions and produces no Rust report; neither mode proves movement/combat or Unreal match integration |
| Unreal C++ changes (once the project exists) | Pinned toolchain preflight, affected C++ tests and Unreal Automation IDs, Editor/Client compilation | Packaged-world user acceptance unless explicitly run |
| Maps/navigation/fog (once implemented) | One map fixture, spawn/collision, group paths, LOS/visibility and faction/NPC occupation assertions | Every campaign map and every target platform |
| Art, materials, audio imports (once implemented) | Provenance/rights review; file/metadata import validation; one representative in-engine playback/render check | Complete art polish, memory/performance qualification |
| Release or deliberate milestone | Full release evidence matrix and target-device packages | Unchecked devices or platform storefront certification |

**No automatic heavy validation on every text change.** Local checks are triggered deliberately. Restore hosted automation only by an explicit later decision; there is no `.github/workflows` directory in this policy.

## 3. Diagnostic evidence and handoff

Run the [Rust quality tool](../tools/version/src/bin/quality.rs) in appropriate lanes and retain its generated `build/diagnostics/*.json` reports locally. Verdicts have distinct meanings: `PASS` proven assertions, `FAIL` violated checks, `BLOCKED` missing prerequisites, `INFRA_ERROR` tool failure, `INCONCLUSIVE` insufficient evidence, `SKIPPED` intentional non-execution and `NOT_RUN` not executed. Never turn a captured image, exit code zero or compiler success into a gameplay PASS.

For significant code handoffs record exact Git SHA, files changed, commands executed, which checks actually passed/failed, unresolved blockers, and next responsible action. Generated data should be reproducible where practicable and must not be committed merely as evidence of an unsuccessful local run. Pin exact tool versions when reproducibility depends on them.

**Decision on speed:** A documentation-only or archival commit does not require an Unreal build. A rules change gets a focused C++ test. A real map importer change requires its affected map fixtures and in-engine proof. A full suite is reserved for a release candidate or intentional milestone; do not freeze unrelated development because an unimplemented release gate is blocked.

## 4. Documentation maintenance rules

- [Documentation Index](DOCUMENTATION_INDEX.md) is the entry point for maintainers; group documents by gameplay, technical architecture, production and research.
- Existing canonical titles/paths should remain stable unless an actual rename and link update are justified.
- No file should make unsourced declarations of package compatibility or historical certainty.
- Separate future targets from current support, and optional concepts from committed scope.
- Place tactical mechanics in concept/specialist documents, platform constraints in the platform plan, technical implementation contracts in engineering documents, and objective quality gates in the release document.
- Each new roadmap needs a distinct owner/problem; prefer a section in an existing document.
- Use the existing root `version.json` as the sole maintained game-version identity and treat production metadata as different, subordinate contracts.

## 5. Tool boundaries and current baseline

Rust 2024/Cargo is the preferred external validation/manifest/diagnostic toolchain. Native C++ is the authoritative gameplay rules language; Unreal Engine 5.8.3 provides simulation integration, UI, rendering, asset import and build packaging when those systems exist. C# is permitted for a narrowly justified external tool. Do not introduce a scripting-language dependency or a duplicate runtime rule engine.

Immediate documented source files: [faction registry](../Data/content_manifest.json), [historical decisions](../Data/historical_decisions.json), [map candidates](../Data/map_production.json), [audio events](../Data/audio_events.json), [performance fixtures](../Data/performance_cases.json) and [release matrix](../Data/release_matrix.json). Valid registry schemas are groundwork; no playable or packaged acceptance is asserted.
