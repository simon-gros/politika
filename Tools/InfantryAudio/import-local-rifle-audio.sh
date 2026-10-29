#!/usr/bin/env bash
set -euo pipefail
project_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
cd "$project_root"
: "${UNREAL_ENGINE_DIR:?Set UNREAL_ENGINE_DIR to the pinned UE installation}"
source_root="${RIFLE_SOURCE_ROOT:-/mnt/data/CommandAndConquer/UM/Command and Conquer 3 Tiberium Wars/extracted}"
english="$source_root/EnglishAudio/1.0/EnglishAudio/data"
common="$source_root/Core/1.0/GlobalStream/data"
audio="${RIFLE_AUDIO_DIR:-$project_root/Saved/InfantryImport/RifleInfantry/AudioEnglish}"
mkdir -p "$project_root/Saved/InfantryImport"
rustc --edition=2021 Tools/InfantryAudio/extract_rifle_audio.rs -o Saved/InfantryImport/extract-rifle-audio
Saved/InfantryImport/extract-rifle-audio "$english/global_english.manifest" \
    "$english/global_english/cdata" "$audio" "$common/global_common/cdata"
bash tools/unreal.sh build -MaxParallelActions=6
"$UNREAL_ENGINE_DIR/Engine/Binaries/Linux/UnrealEditor-Cmd" "$project_root/Politika.uproject" \
    -unattended -nop4 -nosplash -nullrhi -run=PolitikaImportRifleInfantryAudio \
    "-Source=$audio" "-ABSLOG=$project_root/Saved/InfantryImport/rifle-audio-import.log"
rg -q 'RIFLE_AUDIO_IMPORTED mp3_sources=108 soundwaves=108' Saved/InfantryImport/rifle-audio-import.log
INFANTRY_PACKAGE_PROFILE=LocalRifleInfantry bash Tools/Infantry/package-playtest.sh
