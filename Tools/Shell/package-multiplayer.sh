#!/usr/bin/env bash
# Package the networked-foundation build into an immutable archive (never the stable
# player default). Development cooks the multiplayer test fixture; Shipping must exclude it.
# Usage: UNREAL_ENGINE_DIR=... bash Tools/Shell/package-multiplayer.sh development|shipping ARCHIVE_DIR
set -euo pipefail
project_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
: "${UNREAL_ENGINE_DIR:?Set UNREAL_ENGINE_DIR to the pinned UE installation}"
config="${1:?usage: package-multiplayer.sh development|shipping ARCHIVE_DIR}"
archive_dir="${2:?usage: package-multiplayer.sh development|shipping ARCHIVE_DIR}"
cd "$project_root"
case "$config" in
    development) client_config=Development ;;
    shipping) client_config=Shipping ;;
    *) echo "unknown configuration: $config" >&2; exit 2 ;;
esac
if [[ -e "$archive_dir/Linux" ]]; then
    echo "Refusing to overwrite an existing archive: $archive_dir/Linux" >&2
    exit 3
fi
mkdir -p "$archive_dir"
bash tools/unreal.sh doctor
# BuildCookRun reads one -CookDir value and splits it on '+'.
cook_dirs="$project_root/Content/Politika/Prototypes/Infantry/RifleInfantry"
if [[ "$config" == development ]]; then
    # Dynamically loaded by map ID; UAT only cooks it from an explicit physical directory.
    cook_dirs+="+$project_root/Content/Politika/TestFixtures/Multiplayer"
fi
bash "$UNREAL_ENGINE_DIR/Engine/Build/BatchFiles/RunUAT.sh" -WaitForUATMutex BuildCookRun \
    "-project=$project_root/Politika.uproject" -noP4 -platform=Linux "-clientconfig=$client_config" \
    -build -cook -stage -pak -iostore -archive -unattended -utf8output \
    "-map=/Game/Maps/MainMenu+/Game/Maps/OfflineSandbox+/Game/Maps/RTSVerticalSlice" \
    "-CookDir=$cook_dirs" "-archivedirectory=$archive_dir" '-UbtArgs=-WaitMutex -MaxParallelActions=12'

manifest="$archive_dir/Linux/Manifest_UFSFiles_Linux.txt"
[[ -f "$manifest" ]] || { echo "UAT produced no UFS manifest: $manifest" >&2; exit 1; }
require() { rg -q -F "$1" "$manifest" || { echo "Package manifest is missing $1" >&2; exit 1; }; }
for asset in DA_RifleInfantryPresentation DA_RifleInfantryBehavior DA_TestRifleInfantry DA_TestRifle; do
    require "Politika/Content/Politika/Prototypes/Infantry/RifleInfantry/$asset.uasset"
done
fixture="Politika/Content/Politika/TestFixtures/Multiplayer"
if [[ "$config" == development ]]; then
    for asset in DA_MultiplayerCatalog DA_MultiplayerSkirmish DA_MultiplayerOppositionRifleInfantry \
        DA_MultiplayerThirdRifleInfantry DA_MultiplayerFourthRifleInfantry DA_MultiplayerUnalignedMilitia; do
        require "$fixture/$asset.uasset"
    done
    binary=Politika
else
    if rg -q -F "$fixture/" "$manifest"; then
        echo "Shipping package unexpectedly contains the multiplayer test fixture" >&2
        exit 1
    fi
    binary=Politika-Linux-Shipping
fi
[[ -x "$archive_dir/Linux/Politika/Binaries/Linux/$binary" ]] || {
    echo "Archived executable missing: $binary" >&2; exit 1; }
cargo run --locked --quiet --manifest-path tools/version/Cargo.toml -- identity > "$archive_dir/Linux/BuildIdentity.json"
cp tools/unreal-version.env "$archive_dir/Linux/unreal-version.env"
sha256sum "$archive_dir/Linux/Politika/Binaries/Linux/$binary" > "$archive_dir/Linux/executable.sha256"
echo "Multiplayer $config package verified: $archive_dir/Linux"
