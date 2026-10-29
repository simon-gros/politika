#!/usr/bin/env bash
set -euo pipefail
project_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
: "${UNREAL_ENGINE_DIR:?Set UNREAL_ENGINE_DIR to the pinned UE 5.8.3 installation}"
source_root="${INFANTRY_CONVERTED_DIR:-/mnt/data/CommandAndConquer/UM/Command and Conquer 3 Tiberium Wars/infantry-import-20261029/converted}"
output_dir="${INFANTRY_PREPARED_DIR:-$project_root/Saved/InfantryImport/RifleInfantry}"
rustc Tools/Infantry/prepare_rifle_infantry.rs -o Saved/InfantryImport/prepare-rifle-infantry
all_sources="${INFANTRY_EXTRACTED_DIR:-/mnt/data/CommandAndConquer/UM/Command and Conquer 3 Tiberium Wars/infantry-import-20261029/all-sources}"
Saved/InfantryImport/prepare-rifle-infantry "$source_root" "$all_sources" "$output_dir"
bash tools/unreal.sh build
"$UNREAL_ENGINE_DIR/Engine/Binaries/Linux/UnrealEditor-Cmd" "$project_root/Politika.uproject" \
    -unattended -nop4 -nosplash -run=PolitikaImportInfantry -LocalRifleInfantry -PromoteDefault \
    "-Source=$output_dir" -Log=rifle-infantry-import.log
import_log="$project_root/Saved/Logs/rifle-infantry-import.log"
rg -q -F "INFANTRY_IMPORT_SUCCESS" "$import_log" || {
    echo "Riflemen import did not report success; see $import_log" >&2
    exit 1
}
INFANTRY_PACKAGE_PROFILE=LocalRifleInfantry bash Tools/Infantry/package-playtest.sh
manifest="${INFANTRY_ARCHIVE_DIR:-$project_root/Saved/Packages}/Linux/Manifest_UFSFiles_Linux.txt"
for asset in DA_RifleInfantryPresentation DA_RifleInfantryBehavior DA_TestRifleInfantry DA_TestRifle DA_TargetDummy; do
    rg -q -F "Politika/Content/Politika/Prototypes/Infantry/RifleInfantry/$asset.uasset" "$manifest"
done
for asset in SK_PT_RifleInfantry SKEL_PT_RifleInfantry T_PT_RifleInfantry_D; do
    rg -q -F "Politika/Content/Politika/Prototypes/Infantry/RifleInfantry/Media/$asset.uasset" "$manifest"
done
rg -q -F "Politika/Content/Politika/Prototypes/Infantry/RifleInfantry/M_PT_RifleInfantry.uasset" "$manifest"
for role in Idle Move Fire Death Ready Select Deselect DeathAlternate DeathBlast; do
    rg -q -F "Media/AS_PT_RifleInfantry_$role.uasset" "$manifest"
done
echo "Rifle Infantry package manifest verified: $manifest"
