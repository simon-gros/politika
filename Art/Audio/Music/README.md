# Dusk — test-map background music

- Creator/source: Simon Gros; original MP3 supplied directly on 2026-10-28, described by Simon as his own production and requested for in-game test-map background music.
- Original: `Dusk.mp3` (stereo, 44,100 Hz, 268.48 seconds).
- Original SHA-256: `06177f451dfcd217edd43c991dbf6648a4bc64d2fb3a5df67c453c3256077258`.
- Runtime asset: `/Game/Politika/Audio/Music/SW_Dusk`.
- Playback: looping, non-spatial, component gain **0.2** (20%, before device/system gain; reduced from 60% on 2026-10-29), asset gain 1.0. Runs for the prototype test-map world's lifetime, including results; stops on map unload/return to menu. No main-menu or sandbox music. Dedicated servers do not play it.
- Scope: user-authorized game integration; no broader third-party licensing claim is made.

The original MP3 is preserved byte-for-byte. Unreal imports a decoded 16-bit PCM WAV, not an MP3 renamed as WAV. The generated WAV stays in ignored `build/music/`:

```sh
mkdir -p build/music
ffmpeg -nostdin -v error -i Art/Audio/Music/Dusk.mp3 -c:a pcm_s16le -y build/music/Dusk.wav
UNREAL_ENGINE_DIR=/mnt/data/unreal/5.8.3 bash tools/unreal.sh build -MaxParallelActions=6
/mnt/data/unreal/5.8.3/Engine/Binaries/Linux/UnrealEditor-Cmd /mnt/data/politika/Politika.uproject -run=PolitikaImportMusic -Source=/mnt/data/politika/build/music/Dusk.wav -unattended -nop4 -nullrhi -nosound
```

The explicit importer reuses an existing sound asset and sets its looping/gain/virtualization properties; it does not overwrite authored maps. `DefaultGame.ini` includes the music directory for cooking. `UPolitikaTestMapMusicComponent`, owned by `APolitikaPrototypeGameMode`, handles playback and teardown without persisting across level travel.

## Volume follow-up — 2026-10-29

Music component gain is now 0.2; combat effects and SoundWave gain are unchanged. UAT build/cook/archive passed (exit 0): `Saved/PlaytestVolume20_20261029-build.log`. Package checksums verified in `Saved/PlaytestVolume20_20261029/Package/Linux/SHA256SUMS`. Promoted to `Saved/Packages/Linux/`, now the default infantry launch/package destination; previous canonical package preserved at `Saved/Packages/LinuxBeforeVolume20_20261029/`.

Rendered packaged startup through the archive and default launcher logged `TEST_MAP_MUSIC_START track=Dusk volume=0.20 looping=1 playing=1`, content ready and music stop on orderly quit. Isolated-user evidence: `Saved/PlaytestVolume20_20261029/runtime.log` and `default-launcher.log`. Content/config/music source hashes match current inputs. Unrelated concurrent gameplay source changes after packaging are not qualified by this volume check. Physical-device loudness was not measured.

## Verified 2026-10-28 (historical 60% setting)

- Pinned UE 5.8.3 preflight, Editor build and import: exit 0. Imported duration 268.48 seconds; looping enabled, asset gain 1.0.
- `UNREAL_ENGINE_DIR=/mnt/data/unreal/5.8.3 cargo run --locked --quiet --manifest-path Tools/Shell/Cargo.toml -- package`: exit 0, UAT `BUILD SUCCESSFUL`; updated launcher `Saved/Packages/Linux/Politika.sh`.
- Rendered Linux package on isolated Xvfb `:112`, fresh `-UserDir`, actual PulseAudio/PipeWire output routed by this process's sink-input ID into a temporary null sink. Seven-second capture contained audio: mean -26.1 dBFS, peak -10.9 dBFS. Runtime reported `TEST_MAP_MUSIC_START track=Dusk volume=0.60 looping=1 playing=1`.
- Escape returned to the main menu; runtime reported `TEST_MAP_MUSIC_STOP track=Dusk playing=0`. Subsequent three-second menu capture was digital silence (FFmpeg's 16-bit volume floor -91.0 dBFS). Map/menu screenshots inspected. Test process, Xvfb and temporary sink were cleaned up.
- Evidence: `Saved/BuildLogs/dusk-editor.log`, `dusk-import.log`, `dusk-package.log`; `build/music/playback-verified/` holds runtime log, WAV captures, levels and screenshots. Earlier `build/music/playback/` captures did not route the game stream and are not audio evidence.
- `git diff --check`: passed. The full 4:28 loop boundary and physical speaker/device loudness were not tested; looping is the imported native SoundWave setting. No version/schema changes.
