# Stage 3 — Data-Driven Content and Map Validation Acceptance Record

**Verdict: ACCEPTED for the scoped Linux UE 5.8.3 Stage 3 gate.** Controlled authored-value and display-label test-copy changes, repeated canonical setup parity, focused shared-C++ structural negatives, normal/fixture Development packages, Shipping fixture isolation, and rendered Development interaction/recovery have current-source evidence below. Shipping rendering and non-Linux platforms remain unqualified. This is independent of the command/simulation milestone record.

## Qualification identity

| Item | Observed identity |
|---|---|
| Source revision | `dc26d5e15ee9cc6341c54f673421b100b4a81473` (`main`, two commits ahead of `origin/main`; worktree was already dirty) |
| Preserved dirty docs | `Docs/DOCUMENTATION_INDEX.md`, `Docs/ENGINE_OUTLINE.md`, `Docs/GAMEPLAY_CONTENT.md`, and this pre-existing untracked ledger were preserved and reconciled. |
| Implementation/evidence changes | `Source/Politika/Private/PolitikaPrototypeGameplay.cpp`, `Source/Politika/Private/Tests/PolitikaPrototypeContentTests.cpp`, `Tools/Shell/src/bin/content_acceptance.rs`, `Tools/Shell/src/bin/shell_recovery.rs` |
| Relevant dirty-source SHA-256 | Gameplay C++ `cd463d478ede15b8e4872ce91c66154b5035a30952c4c34cb1a83f8f43a57113`; content automation `671f0a7e66639bec7d5cbb908b3fb4ce562a6174ee75cbb353e0f23d49cd47fc`; content harness `51f205053e3c578ec6a487a9bda788ec03ccdeb9159ae2351c8e271ee4496bf0`; recovery harness `dbc201c790c34f2b138054a02a60f21e4ef44736dc6f8621526aef3ac96d5b0c`. |
| Engine | UE `5.8.3-0+UE5`, pinned source SHA `396c9f059903aed5fec78ecd3d437a40c6415368` |
| Native toolchain | Clang `20.1.8`, Linux x86_64 |
| Qualification scope | Linux Development content/gameplay; Shipping fixture isolation. No Shipping visual or other-platform qualification. |
| Package identity files | All generated from the final dirty source tree; identity JSON SHA-256 `6c66897a64f58cca777aca1a3361406c3f0aa44523f667d19ee6c22bef60a10a`; `git_revision=dc26d5e15ee9-dirty`, `git_dirty=true`. |
| Fixture identity | `map.test.invalid_schema`, `/Game/Politika/TestFixtures/DA_InvalidMapSchema`, schema 2 where schema 1 is supported; source asset SHA-256 `b9ca1faaec4dae6977727ad37cc6d3753146d16270b66691600105d5dc5d558a`. |
| Canonical content guard | Catalog, unit, weapon, map, and map-level SHA-256s stayed respectively `a434d4ab8ab3fc4339bcf95a0f580f6ac9fbf105fe0223fddffda02394492d77`, `055b97e257a7435eb5177c709b5b3eee155ee04ee1a573fa1b581bfd93c9840c`, `1ba7399c8dd72c21b23d3181536fb6c996a0eefefc2c0d3ee86884af09093009`, `12295e9e1c5239eeb070172221a8f240266966f6a9868e104a5566c48219220c`, `2cb40d47d0974bb5d9db3688130bc0648f2f4855e0a0a180679b2798dfbd88a6`. |

All generated reports and packages are local under ignored `build/` and `Saved/`. No canonical gameplay asset or map was edited. Package identities, manifests and executable hashes:

| Package | Artifact | SHA-256 |
|---|---|---|
| Normal Linux Development `Saved/Packages/Stage3FinalDev/Linux` | Executable `Politika` | `094dd78f8063b991e029a56c1665df6d855d625bd6700e64f36b22d64c7a15c2` |
| Normal Linux Development | `Manifest_UFSFiles_Linux.txt` | `301a347655619a35053b329d7fbc3c6f6065915409de9c017ee125365e4d7c00` |
| Fixture Linux Development `Saved/Packages/Stage3FinalFixtureDev/Linux` | Executable `Politika` | `094dd78f8063b991e029a56c1665df6d855d625bd6700e64f36b22d64c7a15c2` |
| Fixture Linux Development | `Manifest_UFSFiles_Linux.txt` | `909c0b7816e855220852c684ec234dc857bcb8f2f8712a49f63e461a3b59088d` |
| Linux Shipping `Saved/Packages/Stage3FinalShipping/Linux` | Executable `Politika-Linux-Shipping` | `6c453bfa889aeb63d294d193c3ce66e44f82d02bfbf08bef4d2be4f1914d349e` |
| Linux Shipping | `Manifest_UFSFiles_Linux.txt` | `f396d7c5120aaa79bad1562a3605b19e4d109eca2a48a6011393d44a604bc3be` |

The normal Development and fixture Development packages have byte-identical executables; only the explicitly requested fixture cook differs. The normal and Shipping manifests omit the fixture; the fixture-enabled Development manifest includes it.

## Verification evidence

| Evidence | Command / report | Result |
|---|---|---|
| UE pin preflight | `UNREAL_ENGINE_DIR=/mnt/data/unreal/5.8.3 bash tools/unreal.sh doctor` | PASS; installed UE version and source SHA match the pin. |
| Editor build | `UNREAL_ENGINE_DIR=/mnt/data/unreal/5.8.3 bash tools/unreal.sh build` | PASS, exit 0; Linux Development Editor. |
| Content automation | `/mnt/data/unreal/5.8.3/Engine/Binaries/Linux/UnrealEditor-Cmd /mnt/data/politika/Politika.uproject -unattended -nop4 -nullrhi -nosplash '-ExecCmds=Automation RunTests Politika.Content; Quit' -ReportExportPath=/mnt/data/politika/build/diagnostics/stage3-final -abslog=/mnt/data/politika/build/diagnostics/stage3-final/content-automation-final.log` | PASS, exit 0: 7 succeeded, 0 failed, 0 not run; report `build/diagnostics/stage3-final/index.json`. |
| Authored-data authority | `Politika.Content.AuthoredValueChangesAuthoritativeCombatThroughLoader` | PASS: a transient duplicate of `weapon.recon_cannon` had Damage increased by 37, replaced only in the in-memory test catalog, validated/re-resolved through `FPolitikaPrototypeContentLoader`, and passed into the actual unit combat simulation. `ApplyPendingDamage()` applied the changed value. The canonical UObject value was never changed; disk asset hashes remained identical; no C++ recompilation was caused by the authored value edit. |
| Display label vs identity | `Politika.Content.DisplayLabelDoesNotChangeStableIdentityOrResolution` | PASS: a transient duplicate's label changed; `StableId`, `FPrimaryAssetId`, and catalog resolution through that stable ID remained unchanged. Canonical definition was restored. No physical rename/move survival is claimed. |
| Repeated canonical map initialization | `cargo run --locked --manifest-path Tools/Shell/Cargo.toml --bin content_acceptance -- Saved/Packages/Stage3FinalFixtureDev/Linux/Politika/Binaries/Linux/Politika`; evidence `Saved/Packages/Stage3FinalFixtureDev/Linux/content-acceptance-1793129506/` | PASS ST3-07: two fresh packaged initializations produced identical authoritative unit/objective snapshots: count, archetype, runtime owner/slot, position, health, movement speed, authored/resolved weapon IDs and combat values, objective identity/type/location/radius/duration. Exactly 3 unique runtime entity IDs and 1 objective; no duplicates. |
| Structural negative tests | `Politika.Content.StructuralNegativeDiagnosticsAreSpecific` | PASS; each isolated case emitted exactly one diagnostic, with intended code, asset ID and property: malformed ID `CONTENT.ID.INVALID` / `StableId`; non-positive health `CONTENT.VALUE.RANGE` / `MaxHealth`; unsupported unit and weapon schema `CONTENT.SCHEMA.UNSUPPORTED` / `SchemaVersion`; faction-slot mismatch `CONTENT.FACTION.MISMATCH` / `UnitPlacements[0].UnitArchetypeId`; missing required marker `CONTENT.REFERENCE.MISSING_MESH` / `Objective.MarkerMesh`. Existing map schema, reference, bounds, slot and spatial negatives also passed. |
| Canonical validation | `/mnt/data/unreal/5.8.3/Engine/Binaries/Linux/UnrealEditor-Cmd /mnt/data/politika/Politika.uproject -run=PolitikaCreateShell -ValidateContent -unattended -nop4 -nullrhi -nosplash -abslog=/mnt/data/politika/build/diagnostics/stage3-final/canonical-validation.log` | PASS, exit 0; `passed: true`, zero diagnostics, spatial validation passed. Report `Saved/Diagnostics/prototype-content-validation.json`. Validation-only; `-CreateContent` was not used. |
| Blocked route sentinel | Same executable/project/options with `-ValidateBlockedRoute -abslog=/mnt/data/politika/build/diagnostics/stage3-final/blocked-route-validation.log` | Expected negative PASS, exit 0 for the sentinel: `MAP.PATH.UNREACHABLE`. NavMesh reachability does not claim movement-executor path following or dynamic avoidance. |
| Normal valid Development package | `BuildCookRun` Linux Development, no `-CookDir`; then `rts_interaction` on that executable | PASS: valid cooked catalog/map loaded and the rendered regression passed (PAC-10). |
| Fixture Development admission/recovery | `cargo run --locked --manifest-path Tools/Shell/Cargo.toml --bin content_acceptance -- Saved/Packages/Stage3FinalFixtureDev/Linux/Politika/Binaries/Linux/Politika`; `xvfb-run -a -s '-screen 0 1280x720x24 -nolisten tcp' cargo run --locked --manifest-path Tools/Shell/Cargo.toml --bin shell_recovery -- Saved/Packages/Stage3FinalFixtureDev/Linux/Politika/Binaries/Linux/Politika` | PASS: exact log state `units=0 objectives=0 commands=0 simulation_steps=0`; `CONTENT.SCHEMA.UNSUPPORTED`; visible recovery; same-process explicit valid retry. Repeated invalid diagnostic equivalent; unknown fixture rejected without fallback. Reports `content-acceptance-1793129506/` and `content-acceptance-recovery-1793129489/` under the fixture package. Both harness invocations exit 0. |
| Rendered RTS regression | `xvfb-run -a -s '-screen 0 1280x720x24 -nolisten tcp' cargo run --locked --manifest-path Tools/Shell/Cargo.toml --bin rts_interaction -- Saved/Packages/Stage3FinalDev/Linux/Politika/Binaries/Linux/Politika` | PASS PAC-10, exit 0; rendered Linux 1280×720. Evidence `Saved/Packages/Stage3FinalDev/Linux/content-acceptance-interaction-1793129444/`. Selection, camera, commands, movement, rejected-order side effects, combat, damage/death, objective transitions and return to menu passed. |
| Shipping fixture isolation | Shipping `BuildCookRun` without fixture override/`-CookDir`; UFS manifest and executable `strings` inspection | PASS: no fixture path, asset, selector or identifiers in Shipping manifest/executable. Development-only `MATCH_AUTHORITATIVE_` instrumentation is absent too. Shipping rendering is deliberately separate and NOT_RUN; this evidence makes no visible-startup claim. |
| Rust harness compilation | `cargo check --locked --manifest-path Tools/Shell/Cargo.toml --bin content_acceptance` | PASS. The harness only launches the UE runtime and compares structured runtime evidence/manifests; it does not parse Unreal binary assets or simulate gameplay. |
| Documentation audit | `UNREAL_ENGINE_DIR=/mnt/data/unreal/5.8.3 bash tools/unreal.sh audit` | PASS, exit 0; all 45 documentation index links valid; report `build/diagnostics/audit.json`. |

### Package commands

Normal Development:

```sh
/mnt/data/unreal/5.8.3/Engine/Build/BatchFiles/RunUAT.sh BuildCookRun -project=/mnt/data/politika/Politika.uproject -noP4 -platform=Linux -clientconfig=Development -build -cook -stage -pak -iostore -archive -unattended -utf8output -archivedirectory=/mnt/data/politika/Saved/Packages/Stage3FinalDev
```

Fixture-enabled Development used the same command plus `-CookDir=/mnt/data/politika/Content/Politika/TestFixtures` and archive directory `/mnt/data/politika/Saved/Packages/Stage3FinalFixtureDev`. Shipping used `-clientconfig=Shipping`, no `-CookDir`, and archive directory `/mnt/data/politika/Saved/Packages/Stage3FinalShipping`. All three UAT runs reported `BUILD SUCCESSFUL` / exit 0. Each package identity was generated with `cargo run --locked --quiet --manifest-path tools/version/Cargo.toml --bin politika-version -- identity`, and the pinned `tools/unreal-version.env` was archived alongside it.

## ST3-01 through ST3-18

| ID | Verdict | Evidence / scope |
|---|---|---|
| ST3-01 | PASS | Authored unit/weapon values feed runtime `Configure`; no match-critical fallback definitions. |
| ST3-02 | PASS | Stable typed IDs resolve through the catalog; positive content automation and packaged valid match pass. |
| ST3-03 | PASS | Transient authored weapon-value edit flows through runtime loader into authoritative damage application; original content restored and canonical hashes unchanged. |
| ST3-04 | PASS (stable ID only) | Controlled display-label test-copy edit preserves stable ID, primary ID and stable-ID resolution. Physical asset rename/move was not tested. |
| ST3-05 | PASS | Supported schema v1, required references and cooked schema-v2 rejection validated. |
| ST3-06 | PASS | Authored map bounds, slots, placements and objective drive successful packaged scenario construction. |
| ST3-07 | PASS | Two packaged canonical initializations produced identical complete authoritative setup snapshots and unique entities. |
| ST3-08 | PASS (applicable structural classes) | Added focused malformed-ID, invalid health, unit/weapon schema, faction capability/reference mismatch and missing objective-marker failures with exact diagnostic identity and no masking failure. Duplicate objective entries are not representable by the current singular objective schema; no unused field was added. |
| ST3-09 | PASS (spatial validator scope) | Canonical spatial validation and real blocked-route NavMesh failure pass. Movement path following remains a separate behavior claim. |
| ST3-10 | PASS | Validation-only commandlet reports zero diagnostics and leaves canonical content hashes unchanged. |
| ST3-11 | PASS | Normal Development package cooks and runs valid authored content; rendered interaction harness passed. |
| ST3-12 | PASS | Separate fixture package rejects unsupported schema with exact diagnostic and log asserts `units=0 objectives=0 commands=0 simulation_steps=0` before recovery. |
| ST3-13 | PASS | Rendered visible recovery and explicit valid retry in the same process. |
| ST3-14 | PASS (fixture isolation) | Normal Development and Shipping manifests exclude the fixture; separate fixture Development manifest includes it; Shipping executable has no fixture selector/asset strings. Shipping rendering remains NOT_RUN. |
| ST3-15 | PASS (Linux Development scope) | Existing rendered RTS interaction regression passed on the final normal Development package. |
| ST3-16 | PASS (Linux Development scope) | Default MainMenu, visible recovery, same-process retry and return-to-menu passed. |
| ST3-17 | PASS (identified dirty source) | UE pin, dirty revision, relevant source hashes, commands, reports, package identities and artifact hashes are recorded above. All packages are stamped `dc26d5e15ee9-dirty`. |
| ST3-18 | PASS | Roadmap and content documentation distinguish scoped acceptance from Shipping rendering/platform qualification and from the independent command/simulation milestone. |

## Limits and progression

Stage 3 is accepted only for the scoped Linux UE 5.8.3 data/content/map criteria above. Shipping rendering, physical monitor/driver/controller qualification, and Windows/SteamOS/Android remain separate gates. The validated content and command foundations do not imply formation movement: the current unit executor does not promise formation pathing. Do not use this acceptance to promote Stage 4 multi-unit/formation behavior or the independent command/simulation milestone.
