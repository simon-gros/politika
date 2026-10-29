#!/usr/bin/env bash
# Package the networked-foundation build into an immutable archive (never the stable
# player default). Development cooks the multiplayer test fixture; Shipping must exclude it.
# Server packages the headless PolitikaServer target (LinuxServer, Development) with the
# fixture so it can host the same Development matches as a listen server.
# Usage: UNREAL_ENGINE_DIR=... bash Tools/Shell/package-multiplayer.sh development|shipping|server ARCHIVE_DIR
set -euo pipefail
project_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
: "${UNREAL_ENGINE_DIR:?Set UNREAL_ENGINE_DIR to the pinned UE installation}"
config="${1:?usage: package-multiplayer.sh development|shipping|server ARCHIVE_DIR}"
archive_dir="${2:?usage: package-multiplayer.sh development|shipping|server ARCHIVE_DIR}"
# UAT runs from the engine directory; a relative archive path would land there.
archive_dir="$(realpath -m -- "$archive_dir")"
cd "$project_root"
maps="/Game/Maps/MainMenu+/Game/Maps/OfflineSandbox+/Game/Maps/RTSVerticalSlice"
platform_dir=Linux
case "$config" in
    development) target_args=(-clientconfig=Development) ;;
    shipping) target_args=(-clientconfig=Shipping) ;;
    server)
        # The server never shows the shell; it boots straight into the match map.
        target_args=(-server -noclient -serverplatform=Linux -serverconfig=Development)
        maps="/Game/Maps/RTSVerticalSlice"
        platform_dir=LinuxServer
        ;;
    *) echo "unknown configuration: $config" >&2; exit 2 ;;
esac
if [[ -e "$archive_dir/$platform_dir" ]]; then
    echo "Refusing to overwrite an existing archive: $archive_dir/$platform_dir" >&2
    exit 3
fi
mkdir -p "$archive_dir"
bash tools/unreal.sh doctor
# BuildCookRun reads one -CookDir value and splits it on '+'.
cook_dirs="$project_root/Content/Politika/Prototypes/Infantry/RifleInfantry"
if [[ "$config" != shipping ]]; then
    # Dynamically loaded by map ID; UAT only cooks it from an explicit physical directory.
    cook_dirs+="+$project_root/Content/Politika/TestFixtures/Multiplayer"
fi
bash "$UNREAL_ENGINE_DIR/Engine/Build/BatchFiles/RunUAT.sh" -WaitForUATMutex BuildCookRun \
    "-project=$project_root/Politika.uproject" -noP4 -platform=Linux "${target_args[@]}" \
    -build -cook -stage -pak -iostore -archive -unattended -utf8output \
    "-map=$maps" "-CookDir=$cook_dirs" "-archivedirectory=$archive_dir" '-UbtArgs=-WaitMutex -MaxParallelActions=12'

out="$archive_dir/$platform_dir"
manifest="$out/Manifest_UFSFiles_$platform_dir.txt"
[[ -f "$manifest" ]] || { echo "UAT produced no UFS manifest: $manifest" >&2; exit 1; }
require() { rg -q -F "$1" "$manifest" || { echo "Package manifest is missing $1" >&2; exit 1; }; }
for asset in DA_RifleInfantryPresentation DA_RifleInfantryBehavior DA_TestRifleInfantry DA_TestRifle; do
    require "Politika/Content/Politika/Prototypes/Infantry/RifleInfantry/$asset.uasset"
done
fixture="Politika/Content/Politika/TestFixtures/Multiplayer"
case "$config" in
    shipping)
        if rg -q -F "$fixture/" "$manifest"; then
            echo "Shipping package unexpectedly contains the multiplayer test fixture" >&2
            exit 1
        fi
        binary=Politika-Linux-Shipping
        ;;
    *)
        for asset in DA_MultiplayerCatalog DA_MultiplayerSkirmish DA_MultiplayerOppositionRifleInfantry \
            DA_MultiplayerThirdRifleInfantry DA_MultiplayerFourthRifleInfantry DA_MultiplayerUnalignedMilitia; do
            require "$fixture/$asset.uasset"
        done
        binary=Politika
        [[ "$config" == server ]] && binary=PolitikaServer
        ;;
esac
if [[ "$config" == server ]]; then
    require "Politika/Content/Maps/RTSVerticalSlice.umap"
    # A headless server must not stage the shell menu maps.
    if rg -q -F "Politika/Content/Maps/MainMenu.umap" "$manifest"; then
        echo "Server package unexpectedly contains the MainMenu shell map" >&2
        exit 1
    fi
fi
[[ -x "$out/Politika/Binaries/Linux/$binary" ]] || {
    echo "Archived executable missing: $binary" >&2; exit 1; }
cargo run --locked --quiet --manifest-path tools/version/Cargo.toml -- identity > "$out/BuildIdentity.json"
cp tools/unreal-version.env "$out/unreal-version.env"
sha256sum "$out/Politika/Binaries/Linux/$binary" > "$out/executable.sha256"
echo "Multiplayer $config package verified: $out"
