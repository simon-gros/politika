#!/usr/bin/env bash
set -euo pipefail
project_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
: "${UNREAL_ENGINE_DIR:?Set UNREAL_ENGINE_DIR to the pinned UE installation}"
cd "$project_root"
archive_dir="${INFANTRY_ARCHIVE_DIR:-$project_root/Saved/Packages}"
bash tools/unreal.sh doctor
bash "$UNREAL_ENGINE_DIR/Engine/Build/BatchFiles/RunUAT.sh" -WaitForUATMutex BuildCookRun \
    "-project=$project_root/Politika.uproject" -noP4 -platform=Linux -clientconfig=Development \
    -build -cook -stage -pak -iostore -archive -unattended -utf8output \
    "-map=/Game/Maps/MainMenu+/Game/Maps/OfflineSandbox+/Game/Maps/RTSVerticalSlice" \
    "-CookDir=$project_root/Content/Politika/Prototypes/Infantry/RifleInfantry" \
    "-archivedirectory=$archive_dir" '-UbtArgs=-WaitMutex -MaxParallelActions=6'
# UAT success alone does not prove that a dynamically loaded catalog was cooked.
manifest="$archive_dir/Linux/Manifest_UFSFiles_Linux.txt"
profile="${INFANTRY_PACKAGE_PROFILE:-LocalRifleInfantry}"
case "$profile" in
    LocalRifleInfantry)
        root="RifleInfantry"
        media=(SK_PT_RifleInfantry SKEL_PT_RifleInfantry T_PT_RifleInfantry_D)
        roles=(Idle Move Fire Death Ready Select Deselect DeathAlternate DeathBlast)
        ;;
    CC0SquadKit)
        root="CC0SquadKit"
        media=(SK_CC0_RifleInfantry SKEL_CC0_RifleInfantry SM_CC0_RifleInfantry_Weapon)
        roles=(Idle Move Fire Death Reload Grenade Ready Select Deselect DeathAlternate DeathBlast)
        ;;
    *) echo "Unknown infantry package profile: $profile" >&2; exit 2 ;;
esac
presentation="DA_${profile/CC0SquadKit/CC0RifleInfantry}"
if [[ "$profile" == LocalRifleInfantry ]]; then presentation=DA_RifleInfantryPresentation; fi
for asset in "$presentation" DA_RifleInfantryBehavior DA_TestRifleInfantry DA_TestRifle DA_TargetDummy; do
    rg -q -F "Politika/Content/Politika/Prototypes/Infantry/$root/$asset.uasset" "$manifest" || {
        echo "Incomplete $profile infantry package: missing $asset in $manifest" >&2
        exit 1
    }
done
for media_asset in "${media[@]}"; do
    rg -q -F "Politika/Content/Politika/Prototypes/Infantry/$root/Media/$media_asset.uasset" "$manifest" || {
        echo "Incomplete $profile infantry package: missing $media_asset in $manifest" >&2
        exit 1
    }
done
if [[ "$profile" == LocalRifleInfantry ]]; then
    rg -q -F "Politika/Content/Politika/Prototypes/Infantry/$root/M_PT_RifleInfantry.uasset" "$manifest"
    audio_root="$project_root/Content/Politika/Prototypes/Infantry/$root/Media/Audio"
    while IFS= read -r asset; do
        rg -q -F "Media/Audio/$(basename "$asset")" "$manifest" || {
            echo "Missing cooked audio asset: $asset" >&2; exit 1;
        }
    done < <(rg --files "$audio_root" -g '*.uasset')
    for cue in Voice_Select Voice_Move Voice_Attack Voice_Death Foley_Move Weapon_RifleShot; do
        rg -q -F "Media/Audio/SC_PT_RifleInfantry_$cue.uasset" "$manifest"
    done
fi
for role in "${roles[@]}"; do
    prefix=CC0_RifleInfantry
    [[ "$profile" == LocalRifleInfantry ]] && prefix=PT_RifleInfantry
    rg -q -F "Media/AS_${prefix}_$role.uasset" "$manifest"
done
cmp "$project_root/Binaries/Linux/Politika" "$archive_dir/Linux/Politika/Binaries/Linux/Politika"
echo "$profile infantry package manifest verified: $archive_dir"
