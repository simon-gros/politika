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

Diagnostics are structured (code, severity, asset ID, property path, message, expected/actual value, and suggested action). Common codes include `CONTENT.ID.INVALID`, `CONTENT.ID.DUPLICATE`, `CONTENT.SCHEMA.UNSUPPORTED`, `CONTENT.REFERENCE.UNRESOLVED`, `MAP.BOUNDS.INVALID`, `MAP.SLOT.DUPLICATE`, `MAP.START.OUT_OF_BOUNDS`, `MAP.START.NOT_TRAVERSABLE`, `MAP.OBJECTIVE.OUT_OF_BOUNDS`, `MAP.OBJECTIVE.NOT_TRAVERSABLE`, `MAP.PATH.UNREACHABLE`, `MAP.NAV.DATA_MISSING`, and `MAP.NAV.NOT_READY`.

Every standalone/runtime match loads and validates the catalog and map before spawning units or objectives. A structural or spatial error leaves the match content uninitialized, logs `CONTENT_VALIDATION` diagnostics, and prevents the acceptance harness from running. Validated authored combat values are copied without runtime clamping, and attackability is authored on each unit archetype; there are no fallback unit/weapon/objective definitions. The editor and runtime call the same validator; only the commandlet/report writer is editor-only.

Run the focused Unreal tests:

```sh
/mnt/data/unreal/5.8.3/Engine/Binaries/Linux/UnrealEditor-Cmd Politika.uproject \
  -unattended -nop4 -nullrhi -nosplash \
  '-ExecCmds=Automation RunTests Politika.Content; Quit' \
  -ReportExportPath=build/diagnostics/content-automation
```

Unreal Automation Tests cover typed-ID/reference success, the authored map's structural success, duplicate IDs, unresolved weapon references, unsupported map version, invalid bounds, out-of-bounds objective marker, missing/duplicate starts and player-slot associations, missing map/world diagnostics, and rejection of an invalid map before a validated runtime snapshot can be returned. They also assert the original prototype health, movement speeds, weapon range/damage/cooldown, objective radius/duration, player-slot count, and unit-placement count. The positive commandlet spatial run validates the canonical level against actual Unreal navigation data. `-ValidateBlockedRoute` adds a transient wall, moves the objective in memory, and proves the real NavMesh path query emits `MAP.PATH.UNREACHABLE`; it does not save the changed level or map asset. Runtime match startup and the cooked package are separately exercised with valid content.

## Verification record (2026-10-27)

- `UNREAL_ENGINE_DIR=/mnt/data/unreal/5.8.3 bash tools/unreal.sh build` — passed for UE 5.8.3 Linux Development Editor after the final runtime integration edits.
- `Automation RunTests Politika.Content` — **4/4 passed**; JSON/HTML reports are under `build/diagnostics/content-automation/`, with captured logs under `build/diagnostics/content-milestone/`.
- `PolitikaCreateShell -ValidateContent` — exit status 0; report contains `passed: true` and zero diagnostics. SHA-256 snapshots confirmed the validation-only invocation did not modify the `.umap` or `.uasset` inputs.
- `PolitikaCreateShell -ValidateBlockedRoute` — exit status 0 for the negative-test harness; report contains `MAP.PATH.UNREACHABLE` for the start/objective pair across the transient wall. The `passed: false` payload is the expected negative fixture result, not a content-validation pass.
- Linux Development `BuildCookRun` (build/cook/stage/package/archive) — succeeded after the final runtime integration edits to `build/packages/content-milestone/Linux/`. The captured UFS manifest at `build/diagnostics/content-milestone/cooked-ufs-manifest.txt` lists the catalog, map, unit, and weapon definition packages.
- The final packaged executable launched `/Game/Maps/RTSVerticalSlice` and logged `CONTENT_VALIDATION_PASSED` followed by `MATCH_CONTENT_READY` (`units=3`, objective `objective.central_control`); the captured output is `build/diagnostics/content-milestone/final-packaged-runtime.log`. This verifies cooked asset resolution and match-content initialization; it is not a full interaction replay.

## Deferred work and limitations

- Only schema v1 and the current prototype ground movement profile are implemented; no migration from later historical schemas exists.
- The map validator is specific to the existing two-slot/control-point scenario and default Recast NavMesh. It is not a general terrain, buildability, spawn-overlap, economy, or faction-roster validator.
- Automation coverage is focused, not the full negative matrix in the engine roadmap. One deterministic transient blocked-route fixture exercises actual NavMesh path failure; it does not cover multiple terrain topologies or movement classes.
- Runtime startup and authored entity initialization have been observed both in the standalone Linux Development editor runtime and the Linux Development cooked package. Other target platforms and a full gameplay interaction replay after migration remain separate acceptance items.
- Art, production factions, additional unit/movement types, editor UI, multiplayer, and general content migration tooling remain out of scope.
