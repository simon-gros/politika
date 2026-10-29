# POLITIKA — Changelog

Record meaningful implemented player-facing and compatibility changes under [version.json](version.json) and the [Versioning Roadmap](Docs/VERSIONING_RELEASE_MANAGEMENT_ROADMAP.md).

## Unreleased

- Multiplayer MP-2 (sixth increment): Development builds can fill each seat with up to 200 units (`-PolitikaMPUnitScale=N`) and measure bandwidth and server step cost per phase at RTS unit counts. Packaged 1v1/2v2 runs from 52 to 802 units found that a client's default 100 KB/s send rate filled at about 400 converged units (F44). The per-connection send rate is now 400 KB/s (`DefaultEngine.ini`), and 802-unit battles peak at about 200 KB/s. Ordering a whole army still costs one long server step (F45). Network protocol and game_version are unchanged. See the [MP-2 unit-scale record](Docs/RTS_MULTIPLAYER_MP2_UNIT_SCALE_ACCEPTANCE.md).

- Prototype controls: **Attack-Move is now Q and Stop is X**. W/A/S/D only pan the camera, so pressing an order no longer also nudges the camera, and holding a pan key with a unit selected no longer issues Attack-Move or Stop. Order keys are `DefaultInput.ini` action mappings, and `quality -- audit` rejects order keys that reuse a pan key. Linux Development package and 21 packaged PAC-10 replays, including 10 under CPU load: see [Test-map controls](Docs/TEST_MAP_TERRAIN.md#pac-10-hotkeypan-key-separation-resolved). Version/counters are unchanged.

- Multiplayer MP-2 (second increment): Development builds now measure each connection's bandwidth and the server's step cost, and the packaged dedicated layouts (1v1 to 2v2) are qualified under engine-emulated latency, packet loss, reordering and duplication. Network protocol and game_version are unchanged. See the [MP-2 network conditions record](Docs/RTS_MULTIPLAYER_MP2_NETWORK_CONDITIONS_ACCEPTANCE.md).

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
