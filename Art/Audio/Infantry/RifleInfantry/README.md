# Rifle-infantry local playtest audio

108 neutral-named MP3 variants, encoded with installed FFmpeg 9.0.2 / libmp3lame quality 2 from 48 kHz mono PCM intermediates. The retained WAV intermediates are not shipped. SHA256SUMS records these encoded sources.

These are externally sourced reference placeholders, not original POLITIKA production media. Exact title attribution, bank resolution and acceptance boundaries are in [the separate provenance record](../../../../Docs/CNC3_RIFLEMEN_IMPORT.md). The local source-key mapping remains in `Saved/InfantryImport/RifleInfantry/AudioEnglish/source-map.tsv`.

`PolitikaImportRifleInfantryAudio` imports these MP3s into native SoundWaves and creates role-specific randomized cues. Unreal cooks its own platform audio representation; packaged playback does not require raw MP3/WAV files. Re-import directly from this folder with `-run=PolitikaImportRifleInfantryAudio -Source=<absolute folder>` after building the Editor.
