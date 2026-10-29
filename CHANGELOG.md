# POLITIKA — Changelog

Record meaningful implemented player-facing and compatibility changes under [version.json](version.json) and the [Versioning Roadmap](Docs/VERSIONING_RELEASE_MANAGEMENT_ROADMAP.md).

## Unreleased

- Multiplayer MP-2 (first increment): the dedicated server now hosts packaged 2v1, 3v1 and 2v2 matches for three or four clients, and shot effects are sent per connection by team vision so they never reveal a target the viewer cannot see. **Network protocol 1 → 2**: clients built before this change are refused when joining, with a protocol-mismatch message. Development builds only; see the [MP-2 record](Docs/RTS_MULTIPLAYER_MP2_MULTICLIENT_ACCEPTANCE.md). game_version is unchanged.

- Implemented the UE 5.8.3 offline application shell, two cooked maps, persistent local profile with corruption detection, safe/reset recovery controls, canonical Rust-generated build identity, and Linux packaging. Recovered the interrupted build and fixed link exports, corrupt-profile deserialization, duplicate 4K DPI scaling, and fatal-shutdown settings writes. See [shell verification](Docs/SHELL_SKELETON.md) for compiler, package, 14-case process-check and UI evidence. This is not RTS gameplay or a public release; version/counters are unchanged.

- Integrated the newer production validators, faction roster and unit-art references with the shell. Corrected the Rust production-module path and formatting, and completed the interrupted validation record.

- Defined RTS-native historical authenticity, map-change review, 3D and visual standards, audio events, proportional validation, documentation authority, measurable performance fixtures and release acceptance. Added five source-controlled metadata ledgers and Rust schema/reference tests; performance and release claims remain blocked until real Unreal evidence exists.

- Added Rust/Cargo design-metadata regression tests, five-faction content validation, Unreal 5.8.3 source/build preflight, structured JSON diagnostic verdicts, and a staged RTS-native adoption guide; no playable engine build or hosted workflow was added.

- Consolidated version validation, build identity generation and regression tests into the Rust/Cargo toolchain; no hosted CI workflow is currently configured.

No playable release is currently available. Documentation and historical archive entries are not shipped game features.

## 0.1.0-dev.0 — 27 October 2026 (initial development identity, not a release)

### Project foundation

- Established POLITIKA's alternate-history RTS direction, core factions, story, design concepts, engine architecture and multi-platform targets.
- Added canonical game version and separately reserved network, save, persistence and world schema counters.
- Added Rust/Cargo validation, build identity generation and Rust regression tests.

**Status:** project foundation only; not tagged, packaged, playtested or publicly released.
