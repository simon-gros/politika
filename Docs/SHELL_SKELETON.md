# Shell skeleton — implementation and build guide

Target: Unreal Engine **5.8.3**, C++ runtime, C# Unreal build rules, Rust developer helper. This milestone is the application shell, **not a playable RTS**. Linux is the initial build host; Windows, SteamOS/Deck and Android qualification remains separate work in [Platform Support](PLATFORM_SUPPORT.md).

## Ownership

| Component | Responsibility |
|---|---|
| `PolitikaBootstrap` (`PostConfigInit`, Core + Json only) | Single typed launch-options parser, pre-renderer settings recovery/reset, correlation ID, structured startup events and native fatal reporting |
| `Politika` (runtime) | GameInstance boot phases, engine user-settings subclass, local SaveGame profile, Slate menu/recovery UI, offline world travel |
| `PolitikaEditor` (editor only) | `PolitikaCreateShell` commandlet generates the two real `.umap` assets; preserves existing maps |
| `*.Target.cs`, `*.Build.cs` | Standard UBT game/editor targets and dependencies; no C# runtime or external launcher |
| `Tools/Shell` | Rust process wrapper around UBT, commandlet and UAT plus isolated runtime smoke assertions; reuses repository engine preflight and version tooling, does not own runtime policy |

The early module is intentionally separate: UE preloads resolution and scalability **before** GameInstance initialization. Resetting only in `UGameInstance::Init()` would be too late. Startup follows `EngineReady → ParseLaunchOptions → LoadLocalSettings → ResolveLocalProfile → SelectStartupRoute → Ready`. `Ready` means local services are available; the player controller emits `ui_ready` separately after a world and viewport exist. GameInstance is per game/PIE instance, not a universal process singleton.

## Build and launch on Linux

```sh
cd /mnt/data/politika
export UNREAL_ENGINE_DIR=/mnt/data/unreal/5.8.3
cargo run --manifest-path Tools/Shell/Cargo.toml -- prepare
cargo run --manifest-path Tools/Shell/Cargo.toml -- run -- -NoSplash -log
```

`prepare` verifies the exact engine source pin, builds `PolitikaEditor`, then invokes the map-generation commandlet. `UE_ROOT` remains accepted as a compatibility alias. Direct UBT builds also invoke the Rust canonical-version generator before compiling the bootstrap module.

Build parallelism defaults to six compiler jobs; set `POLITIKA_BUILD_JOBS` (1–64) to suit host memory/CPU availability. This does not change engine or project configuration.

Build an archive with:

```sh
cargo run --manifest-path Tools/Shell/Cargo.toml -- package
```

Output: `Saved/Packages/Linux/` (UAT's platform archive layout), including `Politika.sh`, `BuildIdentity.json` and the engine source pin. Engine build products, `Saved`, `Intermediate`, `DerivedDataCache` and Rust `target` are local artifacts, not source. The root `.gitignore` includes these; the small generated map assets under `Content/Maps` are source-controlled content.

Direct engine commands are also supported; Rust is not required to run the game:

```sh
"$UNREAL_ENGINE_DIR/Engine/Build/BatchFiles/Linux/Build.sh" PolitikaEditor Linux Development "$PWD/Politika.uproject" -WaitMutex -MaxParallelActions=6
"$UNREAL_ENGINE_DIR/Engine/Binaries/Linux/UnrealEditor-Cmd" "$PWD/Politika.uproject" -run=PolitikaCreateShell -unattended -nullrhi -nosound
"$UNREAL_ENGINE_DIR/Engine/Binaries/Linux/UnrealEditor" "$PWD/Politika.uproject" -game -NoSplash
```

If a desktop launcher cannot resolve the project's `5.8` association to a locally registered source build, select the 5.8.3 installation or use these explicit executable paths. Do not silently upgrade the project.

## Maps and UI

- `/Game/Maps/MainMenu`: lightweight default/editor map with `APolitikaShellGameMode`.
- `/Game/Maps/OfflineSandbox`: empty offline world with `APolitikaOfflineGameMode`; shell travel/return destination only, no fabricated campaign/skirmish gameplay.
- Both maps are explicitly included in cooking. No Level Blueprint startup logic, downloaded content, online SDK, marketplace UI plugin or external font is required.
- Native Slate uses core engine fonts, localized `LOCTEXT`, engine-owned shortest-side DPI scaling (no second scaler), safe zones, scrollable content, focusable buttons and an initial keyboard focus. Translation catalogs and hardware-specific navigation qualification are future milestones.
- Application errors remain visible in a non-blocking panel with codes, retry-profile, safe-settings and confirmed backup/reset controls. The primary action continues offline even with a temporary profile. Diagnostics expose current/last-completed phase, versions, run ID and the actual log filename (including an engine `-ABSLOG` override).
- Engine command-line map URLs remain supported; the project does not rewrite them. Other authored maps must deliberately select their own gameplay mode/controller when those systems arrive.

## Switch contract

| Switch | Behavior |
|---|---|
| `-PolitikaOffline` | Explicit offline intent; offline is already the default and no provider is contacted |
| `-PolitikaSafeMode` | Before display preload, redirects GameUserSettings to `Saved/ShellSessions/<run>/GameUserSettings.ini`; applies conservative, unsaved session display settings |
| `-PolitikaResetUserConfig` | Before display preload, backs up and resets **only** the normal generated `GameUserSettings.ini`; never touches profiles, campaign slots or input/language preferences |
| Unknown `-Politika…` or flags with `=value` | Recovery notice; unrecognized text/values are not echoed into project logs; any reset request is cancelled |
| Safe mode + reset | Recovery notice; safe mode wins and reset is cancelled |
| `-NoSplash`, `-log`, `-ABSLOG=…`, `-LogCmds=…`, `-nullrhi` | Passed to Unreal; not reimplemented |

An explicit UE `-GameUserSettingsINI=…` override is a development-only external destination: the shell does not reset it. Combining it with startup safe mode/reset produces an early actionable failure (exit 70) without changing the original. Runtime safe/reset controls are disabled for that destination. No `-headless` flag is invented.

`-nullrhi` is a **client diagnostic mode**, not a server: it can initialize a local profile/player but constructs no shell widgets. Dedicated-server guards are present, but there is **no Server target** and no server support claim; build step 5 is conditional on a future multiplayer/server scope decision.

## Storage and recovery boundaries

| Store | Implementation / policy |
|---|---|
| Device/display settings | `UPolitikaUserSettings`, shipped `DefaultGameUserSettings.ini`, generated per-platform `GameUserSettings.ini` |
| Reset backup | Same directory, `.backup-<UTC>-<unique ID>` suffix; copy must succeed and byte comparison must match before deleting the single source file |
| Safe/invalid-settings overlay | Separate run directory; original preferences are not copied or written, and the settings subclass suppresses normal saves |
| Local identity | `UPolitikaLocalProfile`, schema 1, random stable ID and creation timestamp in UE SaveGame slot `PolitikaLocalProfile`, local user 0; native SaveGame payload wrapped in `PKPROF01` plus SHA-1 corruption checksum |
| Future campaign/match slots | Reserved profile-derived prefix from `SaveSlotPrefix()`; **no campaign persistence implementation in this milestone** |

Missing profile creates a local one through UE SaveGame facilities. A corrupt/unsupported profile is **not overwritten**: retry after external repair or use a temporary offline profile. Write failure offers the same fallback. PIE does not create a persistent player profile. Reset has no code path to enumerate or delete SaveGame slots. It currently means **display/user-config reset only**, not a blanket reset of every future preference store.

The integrity envelope is checked before UE UObject deserialization, which can assert on arbitrary damaged SaveGame headers. It detects accidental corruption; it is not authentication. Raw unwrapped profiles from the unfinished pre-delivery shell are unsupported, preserved and routed to recovery rather than silently migrated or discarded. No released save format was changed. Checks exercise random corruption, altered payload bytes and truncation.

UE owns INI parsing; startup validates the shell's known persisted display fields/version and falls back on invalid values. This is not a full settings migration framework or a validator for every possible engine CVar. Safe mode cannot repair missing engine libraries, unavailable GPU drivers, invalid packages or failures before project modules load. Explicit engine graphics flags still take engine-defined precedence.

Menu safe mode applies to the current, already-running renderer; it cannot retroactively recover a renderer that failed to start. Menu reset redirects future writes first, backs up/removes the normal settings file and keeps temporary defaults for this session. Restart normally to persist new settings. Session directories are disposable diagnostics; no automatic deletion of user files is performed.

## Diagnostics

`LogPolitikaStartup` emits one compact JSON object per event inside UE's normal log prefix. Fields include `event`, `run`, `phase`, `elapsed_ms`, `outcome`, `code`, `route`, game/engine version and sanitized option booleans. Settings and offline-provider messages use `LogPolitikaSettings` and `LogPolitikaPlatform`. Project events omit local profile IDs and raw command lines; UE's own diagnostics may still print engine arguments and paths. Logs use Unreal's normal output location/`-ABSLOG` override.

Handled pre-UI failures log/flush before requesting exit 70, and show a native message only for interactive runs. Engine failures before the bootstrap module remain the responsibility of Unreal's own logs/crash handling. Recoverable map/profile/settings failures do not intentionally crash the process.

## Reproducible process checks

```sh
# Actual packaged runtime; isolated fixtures are created under Saved/ShellChecks/smoke-<time>.
cargo run --locked --manifest-path Tools/Shell/Cargo.toml --bin smoke -- \
  "$PWD/Saved/Packages/Linux/Politika/Binaries/Linux/Politika"
# The same checks can run on the Editor's standalone game path, but do not certify packaging.
cargo run --locked --manifest-path Tools/Shell/Cargo.toml --bin smoke -- \
  "$UNREAL_ENGINE_DIR/Engine/Binaries/Linux/UnrealEditor-Cmd" "$PWD/Politika.uproject"
```

The 14 checks cover first-run/restart identity, explicit-map routing, byte-preserving safe mode, exact reset backup and save-sentinel preservation, invalid settings, three corrupt-profile cases, profile-write failure, invalid/conflicting launch switches, no-network startup, fatal override handling (exit 70), and backup-write failure. Each run records a TSV verdict report and per-process logs, enforces a 90-second timeout, and verifies startup evidence beyond the process exit code. Network isolation uses Linux user/network namespaces with the current UID (not root). These null-RHI checks do **not** certify rendering, input or gameplay. No campaign format exists yet; a preservation sentinel verifies reset's storage boundary, not campaign loading.

## Delivery and acceptance status

Implementation is being built with the locally installed UE 5.8.3 toolchain. See the final delivery record below for actual build/artifact outcomes. Do not infer runtime acceptance from compilation.

Build and runtime results are recorded below after final verification. Gamepad/touch navigation, translations, physical display/driver coverage and non-Linux targets need separate qualification. Server acceptance is deferred with the Server target. This milestone has no RTS simulation or campaign/skirmish implementation.
