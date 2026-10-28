# Prototype gameplay content authoring

This document covers the implemented UE 5.8.3 content slice. It is deliberately narrower than the complete RTS content roadmap: it supports the three existing prototype units/weapons, one control-point scenario, and the default ground Recast navigation profile. The architecture direction and deferred systems remain in [ENGINE_OUTLINE.md](ENGINE_OUTLINE.md).

## Asset types and ownership

Authored assets are under `/Game/Politika/Definitions/`:

| Type | UE class | Responsibility |
|---|---|---|
| Weapon | `UPolitikaWeaponDefinition` (`UPrimaryDataAsset`) | Stable ID, schema version, display label, range, damage, and cooldown. |
| Unit archetype | `UPolitikaUnitArchetypeDefinition` (`UPrimaryDataAsset`) | Stable ID, schema version, faction, health, movement speed, weapon ID, mesh/scale/tint, and attackability. |
| Map definition | `UPolitikaPrototypeMapDefinition` (`UPrimaryDataAsset`) | Versioned profile, referenced level, XY-centimeter bounds, movement profile, player slots/starts, unit placements, control point, camera bounds, and fixture lighting. |
| Catalog | `UPolitikaPrototypeContentCatalog` (`UPrimaryDataAsset`) | Runtime root for weapon, unit, and map assets; reference resolution is by stable ID within type namespaces. |

The initial catalog is `DA_PrototypeContentCatalog`; it references `W_Recon`, `W_Fireteam`, `W_Patrol`, three unit archetypes, and `DA_RTSVerticalSlice`. Runtime discovery uses the Asset Manager scan rooted at `/Game/Politika/Definitions`; the catalog is configured to cook with its referenced content. A fixed catalog object path is used only as a bootstrap fallback if primary-asset discovery has not registered the configured catalog ID.

Stable IDs are lowercase ASCII letters/digits/underscores, optionally separated by single dots (for example `unit.recon_vehicle`, `weapon.recon_cannon`, `map.prototype.rts_vertical_slice`). IDs are independent from display names and asset paths. Duplicate IDs are rejected within each type namespace; references from unit/map content resolve through the catalog. Do not change an ID just to rename or move an asset.

## Schema and map contract

Weapon, unit, catalog, and map assets each carry an explicit schema version. The current supported version is `1`; runtime and editor validation reject other versions. Additive optional fields can remain in v1 only when old assets receive safe, explicit values. A changed meaning, required field, or incompatible serialized shape requires a new version and a deliberate migration before that version is accepted. No unused migration framework is provided.

The current map schema describes the existing fixture only:

- Authored: profile, stable map ID/version, level reference, `world_xy_centimeters`, XY bounds, supported `movement.prototype_ground` profile, tagged ground actor, player-slot and faction assignments, starts, unit placements, one control-point objective and marker, camera pan bounds, and fixture lighting.
- Supplied by the level: static ground mesh/collision, tagged NavMesh bounds volume, and Unreal navigation data generated from the level's collision/navigation geometry.
- Derived during validation: whether starts, objective, and unit placements project to the default Recast NavMesh, and whether each start has a UE navigation path to the required objective.

`Data/map_production.json` remains a historical source/provenance ledger; it is not a gameplay map definition and is not consumed by the match runtime. The prototype validator does not claim coverage for future vehicle/infantry traversal profiles, occupancy rules, resource nodes, or all match profiles.

## Editing content

Open the assets in Unreal Editor and edit typed properties directly. Keep stable IDs unique and references consistent. When creating a unit, add its weapon definition and unit archetype to the catalog, then add a placement associated with a declared slot in the map asset. A unit's faction must match the selected slot's faction. When creating a weapon, add it to the catalog and update each unit's `WeaponId` as needed. For this fixture, retain exactly one human-controlled slot and unique positive runtime player/slot IDs.

The editor commandlet has two distinct modes:

```sh
cd /mnt/data/politika

# Read-only validation of existing assets and the saved level; writes only the JSON report.
/mnt/data/unreal/5.8.3/Engine/Binaries/Linux/UnrealEditor-Cmd /mnt/data/politika/Politika.uproject \
  -run=PolitikaCreateShell -ValidateContent -unattended -nop4 -nullrhi -nosplash

# Explicit bootstrap only: recreates the initial prototype definition values and prepares the fixture.
/mnt/data/unreal/5.8.3/Engine/Binaries/Linux/UnrealEditor-Cmd /mnt/data/politika/Politika.uproject \
  -run=PolitikaCreateShell -CreateContent -unattended -nop4 -nullrhi -nosplash

# Negative spatial sentinel: adds a transient wall and verifies MAP.PATH.UNREACHABLE.
/mnt/data/unreal/5.8.3/Engine/Binaries/Linux/UnrealEditor-Cmd /mnt/data/politika/Politika.uproject \
  -run=PolitikaCreateShell -ValidateBlockedRoute -unattended -nop4 -nullrhi -nosplash
```

Do **not** run `-CreateContent` after authoring changes: it reapplies the initial fixture values. `-ValidateContent` does not invoke that bootstrap or rebuild/save the level. The resulting machine-readable report is `Saved/Diagnostics/prototype-content-validation.json`; errors produce a nonzero commandlet result. Editor asset edits remain the source of truth.

## Validation and runtime behavior

The shared runtime C++ validator checks stable IDs and duplicates, explicit schema versions, required/nonfinite/range-sensitive values, weapon/unit/map references, faction-to-slot matching, supported enum/profile values, map bounds, camera bounds, unique slots/player IDs/factions, starts and placements, objective ID/type/marker/radius/duration, and finite fixture lighting values. The spatial pass checks matching loaded level, ground mesh/collision/coverage, navigation-volume coverage, a completed default NavMesh, projections for starts/objective/units, and path reachability from each valid start to the required objective. It uses `UNavigationSystemV1::ProjectPointToNavigation` and `GetPathLength`; geometric distance is not accepted as proof of reachability.

After shared validation succeeds, the coordinator derives a match-local participant roster directly from authored slots. Slot ID, runtime participant ID, faction ID, and controller kind remain distinct; `bHumanControlled` binds the prototype's local-human identity, while a non-human slot is only an assigned opponent/test role and does not imply functioning AI. Spawned units retain both participant ownership and authored faction affiliation. The Unaligned is excluded from playable slots and represented once as separate NPC/system authority, with no spawned units or autonomous simulation in this slice. Standard two-to-four canonical-faction setup remains unimplemented.

Diagnostics are structured (code, severity, asset ID, property path, message, expected/actual value, and suggested action). Common codes include `CONTENT.ID.INVALID`, `CONTENT.ID.DUPLICATE`, `CONTENT.SCHEMA.UNSUPPORTED`, `CONTENT.REFERENCE.UNRESOLVED`, `MAP.BOUNDS.INVALID`, `MAP.SLOT.DUPLICATE`, `MAP.START.OUT_OF_BOUNDS`, `MAP.START.NOT_TRAVERSABLE`, `MAP.OBJECTIVE.OUT_OF_BOUNDS`, `MAP.OBJECTIVE.NOT_TRAVERSABLE`, `MAP.PATH.UNREACHABLE`, `MAP.NAV.DATA_MISSING`, and `MAP.NAV.NOT_READY`.

Every standalone/runtime match loads and validates the catalog and map before spawning units or objectives. A structural or spatial error leaves the match content uninitialized, logs `CONTENT_VALIDATION` diagnostics, and prevents the acceptance harness from running. Validated authored combat values are copied without runtime clamping, and attackability is authored on each unit archetype; there are no fallback unit/weapon/objective definitions. The editor and runtime call the same validator; only the commandlet/report writer is editor-only.

### Packaged negative-content fixture

The Development-only selector `-PolitikaTestFixture=InvalidMapSchema` requests stable map ID `map.test.invalid_schema`, backed by the authored `DA_InvalidMapSchema` asset under `/Game/Politika/TestFixtures`. It differs from the valid prototype map only in its serialized schema version (2; supported version is 1). It is resolved as a cooked Unreal asset and passed to `ValidateMapStructure`; it is not inserted in the production catalog. Unknown fixture IDs and a missing cooked asset report `MATCH_ADMISSION_SELECTION_FAILED` and `TEST_FIXTURE.ASSET.MISSING` respectively, with no fallback. Selection and the test-asset loader are compiled out under `UE_BUILD_SHIPPING`.

The normal package action does not add the test directory to cooking. To build a separate negative-test package, first run `UNREAL_ENGINE_DIR=/mnt/data/unreal/5.8.3 cargo run --locked --quiet --manifest-path Tools/Shell/Cargo.toml -- maps` (which authors the isolated fixture), then run Linux Development `BuildCookRun` with `-CookDir=/mnt/data/politika/Content/Politika/TestFixtures` and a separate archive directory such as `Saved/Packages/InvalidContentDevelopment`. Do not add this directory to `DirectoriesToAlwaysCook`, Asset Manager production scan paths, or the valid catalog. The negative archive is test-only and must not be promoted as a normal release package.

At runtime, structural validation returns before spatial setup or any unit/objective spawn. A rejection emits the shared structured validator diagnostic followed by `MATCH_ADMISSION_REJECTED ... units=0 objective=0 simulation_started=0`; `bContentReady` remains false, so the authoritative simulation tick exits immediately. The match records a recoverable `MatchAdmissionRejected` issue in the existing `UGameInstance` issue list and schedules the existing `ReturnToMenu` transition on the next world tick. The MainMenu recovery panel exposes the issue without replacing the detailed engine diagnostic. The standard startup map remains MainMenu unless a separate explicit map route is supplied.

Run the focused Unreal tests:

```sh
/mnt/data/unreal/5.8.3/Engine/Binaries/Linux/UnrealEditor-Cmd Politika.uproject \
  -unattended -nop4 -nullrhi -nosplash \
  '-ExecCmds=Automation RunTests Politika.Content; Quit' \
  -ReportExportPath=build/diagnostics/content-automation
```

Unreal Automation Tests cover typed-ID/reference success, authored map structure, duplicate and malformed IDs, unresolved references, unsupported map/unit/weapon schemas, invalid required health, faction-slot mismatch, invalid bounds and objective marker, missing/duplicate starts and player-slot associations, missing map/world diagnostics, and rejection of invalid content before a validated snapshot can be returned. Focused negatives assert the exact diagnostic code, asset ID and property, and isolate unrelated errors. They also assert baseline health, movement speed, weapon values, objective rules, player slots, and placements. A transient cloned weapon value is validated and loaded through `FPolitikaPrototypeContentLoader`, then shown to change authoritative combat damage; canonical asset bytes are unchanged. The positive commandlet spatial run validates the canonical level against Unreal navigation data. `-ValidateBlockedRoute` adds a transient wall and proves the real NavMesh query emits `MAP.PATH.UNREACHABLE`; it does not save the changed level or map asset.

## Verification record (2026-10-27)

See the [Stage 3 acceptance record](STAGE3_DATA_CONTENT_MAP_ACCEPTANCE.md) for the complete ST3-01–ST3-18 verdicts, exact dirty-source and UE identities, command results, package hashes, and retained limits. The scoped Stage 3 gate is **accepted**. The current Linux Development package passed the rendered RTS interaction regression; the separate fixture package explicitly reports zero units, objectives, accepted commands, and simulation steps on invalid admission, followed by visible recovery and valid retry. Shipping rendering remains unqualified.

- UE pin preflight and Linux Development Editor build passed on source `dc26d5e15ee9-dirty`; `Politika.Content` automation passed **7/7** (`build/diagnostics/stage3-final/index.json`).
- `-ValidateContent` returned `passed: true`, zero diagnostics, and a positive spatial result. `-ValidateBlockedRoute` emitted the expected `MAP.PATH.UNREACHABLE`. Validation-only runs left canonical content hashes unchanged.
- The normal Development, separately fixture-enabled Development, and Shipping Linux packages all built successfully from the identified dirty source. Normal and Shipping manifests exclude the invalid fixture; the fixture-enabled manifest contains it. Shipping executable inspection confirms Development fixture selectors and acceptance instrumentation are absent.
- Two fresh canonical packaged map initializations produced identical authoritative unit and objective snapshots. Controlled transient unit/weapon test-copy edits demonstrated stable identity and authored-value authority without changing saved content or gameplay balance.

## Deferred work and limitations

- **Packaged rejection and recovery follow-up (27 October 2026):** rebuilt the Linux Development test package from source revision `27b9ad529834` plus uncommitted changes with UE `5.8.3-0+UE5`. The Rust harness at `Tools/Shell/src/bin/content_acceptance.rs` verified the cooked fixture in the UFS manifest, the exact `CONTENT.SCHEMA.UNSUPPORTED` diagnostic (`map.test.invalid_schema`, `SchemaVersion`, expected 1/actual 2), rejection, zero unit/objective/simulation-step state, recoverable MainMenu transition, deterministic repeat diagnostic, unknown-fixture rejection without fallback, normal default launch, and a valid packaged match with three units/objective. It also passed the packaged capture-pause and attacker-death lifecycle fixtures. The negative path was separately rendered under Xvfb: the rejection notice appeared on the MainMenu, and a subsequent user click in the same process entered the canonical valid match (`MATCH_CONTENT_READY`); the development selector is consumed once, so it does not poison retry. The parser now recognizes the Dev-only selector without producing a spurious `InvalidLaunchOptions` issue.
- **Rendered packaged acceptance (27 October 2026):** Development-only C++ lifecycle events now record camera input, order admission/rejection, first authoritative position movement, health changes, unit death, and objective state transitions; all instrumentation is compiled out of Shipping. `rts_interaction.rs` drives the actual Linux Development package with Xvfb/xdotool, checks the rendered 1280×720 window geometry, asserts the runtime events, and captures screenshots. It passed WASD camera pan (including a rendered pixel-difference assertion), selection, movement/position update, rejected friendly-target command with no attack/health side effect, stop, guard, attack-move, attack damage and death, contested then uncontested objective state/progress, and Escape to MainMenu. Screenshot/log/report evidence is in the generated `content-acceptance-interaction-*` directory in the test package. Wheel zoom is not claimed by this replay; it remains present in the HUD controls and was covered by the earlier accepted clean-source interaction record.
- `shell_recovery.rs` separately drives an invalid fixture launch in a rendered package, verifies the schema diagnostic and zero-state rejection, captures the visible recovery notice, clicks the prototype action, verifies the same process admits the canonical valid map after the one-shot override is consumed, and returns to the menu. This passed PAC-07 and same-process PAC-09. `content_acceptance.rs` passed PAC-01 through PAC-06 and PAC-08 plus fresh-process PAC-09 and the capture-pause/attacker-death lifecycle fixtures; its own PAC-07 is INCONCLUSIVE and PAC-10 is NOT_RUN because it is a null-renderer harness, not because the separate rendered tools failed.
- **Historical Shipping rendering note:** an earlier Shipping startup probe displayed a black frame and was inconclusive. The current Stage 3 acceptance rebuilt and inspected Shipping only for fixture isolation; it did not requalify Shipping rendering. No visible-startup claim is made.
- Linux Development package construction used `RunUAT.sh BuildCookRun -project=/mnt/data/politika/Politika.uproject -noP4 -platform=Linux -clientconfig=Development -build -cook -stage -pak -iostore -archive -unattended -utf8output -CookDir=/mnt/data/politika/Content/Politika/TestFixtures -archivedirectory=/mnt/data/politika/Saved/Packages/InvalidContentDevelopment`. Run content checks with `cargo run --locked --manifest-path Tools/Shell/Cargo.toml --bin content_acceptance -- Saved/Packages/InvalidContentDevelopment/Linux/POLITIKA/Binaries/Linux/Politika`; rendered interaction with `xvfb-run -a -s '-screen 0 1280x720x24 -nolisten tcp' cargo run --locked --manifest-path Tools/Shell/Cargo.toml --bin rts_interaction -- Saved/Packages/InvalidContentDevelopment/Linux/POLITIKA/Binaries/Linux/Politika`; rendered recovery with the same `xvfb-run` prefix and `--bin shell_recovery`. Shipping construction used the same `BuildCookRun` options with `-clientconfig=Shipping`, no `-CookDir`, and archive directory `Saved/Packages/ShippingAudit`. Package output/evidence are ignored; this is not a clean-source delivery certificate.

- Only schema v1 and the current prototype ground movement profile are implemented; no migration from later historical schemas exists.
- The map validator is specific to the existing two-slot/control-point scenario and default Recast NavMesh. It is not a general terrain, buildability, spawn-overlap, economy, or faction-roster validator.
- Automation coverage is focused, not the full negative matrix in the engine roadmap. One deterministic transient blocked-route fixture exercises actual NavMesh path failure; it does not cover multiple terrain topologies or movement classes.
- Runtime startup and authored entity initialization are verified in the current Linux Development cooked packages, and the migrated package passed the rendered RTS interaction and recovery harnesses. Other target platforms, physical display/input devices, and Shipping rendering remain separate qualification items.
- Art, production factions, additional unit/movement types, editor UI, multiplayer, and general content migration tooling remain out of scope.
