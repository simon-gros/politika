# C&C3 Riflemen source analysis and POLITIKA import

## Scope and evidence

The source is the locally installed *Command & Conquer 3: Tiberium Wars* content, extracted with C&C Asset Extractor 1.10 into the title-specific Universal Modder workspace at `/mnt/data/CommandAndConquer/UM/Command and Conquer 3 Tiberium Wars/infantry-import-20261029/`. Source archives and extracted W3X/DDS files are not copied into this repository. Authored game data was cross-checked against the local C&C Modding Support XML pack. That XML is useful authored evidence, not proof of shipped runtime values; the W3D adapter warns that animation reconstruction is not guaranteed.

The prior import pipeline and its boundaries are recorded in the archived procedure `CNC4_INFANTRY_IMPORT.md` under `/mnt/data/politika-reference-provenance/20261028/Docs/`. This import reuses its W3D → Blender/OpenSAGE → FBX → Unreal commandlet → cooked-manifest workflow and adds a separate profile/root.

## Source unit model

The authored object is a six-member squad (`HordeContain Slots=6`, initial payload six), not one soldier. Members are `GDIRifleSoldier`; the squad container is `GDIRifleSoldierSquad`.

| Area | Authored support-pack values/behavior | POLITIKA transfer |
|---|---|---|
| Identity/economy | GDI; squad build cost 300, build time 3, command points 100; member cost 50/time 0.5; formation width 2/depth 1 | Kept the existing single gameplay-entity abstraction for the playtest. Squad size/economy/formation are documented, not silently treated as six independently selectable POLITIKA pawns. |
| Durability/armor | Each member has 75 health, normal rifleman armor, composite armor upgrade. Container uses immortal-body semantics. | Existing prototype health remains a balance placeholder; six-by-75 is a future squad-health reference, not a direct scalar override. |
| Mobility/footprint | Human locomotor; squad speed 57, member speed 62; cylinder radius 7/height 13 and squad box 20×25×13 (source game units). | Authored RUNA is the movement loop; Unreal gameplay speed remains the current prototype tuning. Mesh import scale is explicit and still needs visual confirmation in the packaged runtime. |
| Weapon | `GDIRifleSoldierRifle`; attack range 260; clip size 3; clip reload 0.3–0.4 s; 3 damage gun nugget; can fire while moving; aim delta 10°; ground target; suppression radius 5, amount 5, duration 5 s. | Imported ATKA is the firing animation. Existing POLITIKA combat values remain gameplay-owned; no unsupported conversion from C&C units to centimetres is claimed. |
| Awareness/roles | Squad vision 350, shroud 400; stealth detector range 100; selectable/attack/LOS/infantry/horde/capture roles; can pass through infantry; sympathy range 400. | Current target simulation supports selection, movement, attack and death. Detector, capture, garrison, transport and horde mechanics are identified as future gameplay work, not faked by presentation. |
| Tactical behavior | Dig-in setup 5 s/range 15; cover search radius 10; suppression duration 5 s; veteran/heroic fire FX; composite armor upgrade; garrison, capture, retreat and call-for-transport responses. | Recorded for future ability/state design. No current POLITIKA ability is assigned source-only behavior. |
| Effects/audio | Gunfire uses ten `GURifle_gun5Fire[a-j]` variants through `GDI_RifleSoldier_gunFire_v5`; voice events include create, attack, dig-in, garrison, move, move-attack, retreat, select, select-battle and suppressed; there are firing/tracer/impact, digging and foley events. | Source event names and mappings are retained here. CEA audio-source export stalled under Wine on the AudioFile “Processing…” step, initially preventing audio packaging; the native recovery below now imports 108 clips without substituting fixture or other-game sounds. |

### Animation source coverage

The source inventory includes idle (BIDA/B/C), fidgets (BIFA/B/C/D), selection/action transitions (AIDA/AIDB, AIFA/B/C, BATA/B, ABTA/ABTA2, SIDA/SIFA), locomotion (RUNA/B/C), attack while moving (ATRA), fire (ATKA), deaths (DIEA/B/C), cover/guard (KDIA/B/C, KAIA/B, KAKA, KAFA/B/C), stun/fall (FLYA/B, LNDA), crushed (CDTA/B), suppression states/transitions (SDIA, SDVA/B, SMVA), dig-in (DIGA), and celebration (CHRA/B/C). Exact source names are included in the extracted W3X set; not every clip is currently wired into POLITIKA.

The current presentation profile imports the actual skinned mesh, diffuse texture, skeleton, idle BIDA, run RUNA, fire ATKA, death DIEA, ready AIDA, select SIFA, deselect ABTA, alternate death DIEB and blast death DIEC. Reload and grenade clips are deliberately null: none is authored for this rifleman. Fire while moving, suppression, cover, garrison, digging and transitions remain follow-up state-machine work. The source mesh is 348 vertices with a 24-bone skeleton in the current OpenSAGE conversion; the source muzzle attachment is `B_WEAPONA_FX`.

## Import workflow

1. Extract the selected Rifleman W3X sources and `CURiflemanA.dds` with C&C Asset Extractor into the title-specific UM folder. Retain the extracted originals there.
2. Run `blender --background --python Tools/Infantry/blender_export.py -- <all-sources> <converted>`. The adapter uses installed OpenSAGE W3D import, stages copies, and writes `conversion-summary.json` plus FBX and an inspection `.blend`.
3. Set `UNREAL_ENGINE_DIR` and run `Tools/Infantry/import-local-rifle-infantry.sh`. Rust preparation emits neutral-named FBXs/texture, hash-only source integrity entries, and a neutral media note. The commandlet imports under `/Game/Politika/Prototypes/Infantry/RifleInfantry`, promotes `presentation.playtest.rifle_infantry`, and packages the Linux playtest. This file remains the separate provenance record; its source-specific terminology is intentionally not copied into active game assets or their filenames.
4. Confirm cooked-manifest entries and visually inspect the current package before treating the prototype replacement as accepted. A manifest proves asset inclusion, not visual correctness or animation fidelity.

## Current limitations / acceptance boundary

- The initial CEA audio export failure was superseded by the native decode/import below. The English manifest references both language-specific and shared-bank cdata; extracting only the shared manifest omits 67 voice clips.
- The source is a six-member squad while the current POLITIKA implementation remains one actor. Squad formation, per-member casualties, source health/armor, suppression, capture, cover, garrison, dig-in and upgrades are not implied to be implemented.
- The Blender/OpenSAGE importer can report FCurve key insertion warnings for W3D animation channels. The conversion summary records exported role/frame counts; those counts do not prove motion fidelity. Runtime animation inspection is required.
- Imported mesh scale (10× source coordinates) is an initial conversion choice based on the source mesh dimensions and must be confirmed in the running packaged map.
- Reload/grenade gameplay can continue to use the existing POLITIKA behavior only if those actions are enabled; there is no source-specific animation/audio mapping for either role.

## Current local build evidence (2026-10-29)

- Source working identity: repository HEAD `5c9db959360f` plus preserved uncommitted project changes (`dirty`), UE 5.8.3 pinned build.
- Historical pre-neutral-name build evidence is retained locally. The current neutral import script rebuilds and re-cooks the same authored media roles under POLITIKA naming; do not treat the historical package hashes as identity for the current package.

## Audio recovery (2026-10-29)

The interrupted import is continued with `Tools/InfantryAudio/import-local-rifle-audio.sh`. The Rust extractor resolves the English manifest against English and shared cdata, decodes 108 clips to 48 kHz mono PCM intermediates, then encodes neutral `PT_RifleInfantry_*` MP3s using installed FFmpeg 9.0.2 / libmp3lame (`-q:a 2`, no ID3v2 metadata). Intermediates and the exact source-key/hash ledger stay under `Saved/InfantryImport/RifleInfantry/AudioEnglish/`; source archives remain in the title-specific UM tree.

Unreal's Interchange MP3 translator imports 108 `SW_PT_RifleInfantry_*` SoundWaves. Fifteen role-specific `SC_PT_RifleInfantry_*` cues randomize variants without replacement. The presentation uses English select/move/attack/death voices, rifle shots and movement Foley footsteps. Dig-in, garrison, retreat, creation, special, suppression, move-attack and battle-selection variants are imported for future integration, not evidence that those gameplay states exist. Reload, grenade and blast-specific audio remain unset because this bank supplies no such dedicated roles. Cooked playback uses Unreal's platform audio representation; the executable does not decode raw MP3 files at runtime.

Recovery diagnosed an incompatible package: its game executable was timestamped 02:55 while its assets were cooked after 04:53. Earlier checkout synchronization had preserved the infantry runtime in stash `4a2e50d`; dependencies were restored with a three-way merge retaining newer navigation code. A fresh game build and full cook are required; the rejected archive is not a usable playtest. Source recovery snapshots remain preserved in Git stashes and branch `recovery/rifle-audio-interrupted-20261029`.

This remains a local reference-placeholder playtest, not original POLITIKA production media or a release/platform certification.

### Verified acceptance

- Package source: `16644045cc4aac6fffcfb8ac66dd5bdd2fa16847`; engine: `396c9f059903aed5fec78ecd3d437a40c6415368` (UE 5.8.3). Startup log reports `git_revision=16644045cc4a`, matching the archived source/header. Subsequent documentation commits do not change that package identity.
- Portable C++ rules checks pass. Unreal `Politika` automation: **26/26 pass**, exit 0, at the recovered infantry code state; final combined music/infantry Editor build and `Politika.Infantry` recheck: **4/4 pass**, exit 0. Reports/logs: `Saved/RifleAudioRecovery20261029/TestsFinal`, `TestsCombined`, `core-check.log`, `tests-final.log`, `tests-combined.log`.
- Final Linux Development UBT/UAT build/cook/stage/archive: exit 0, `BUILD SUCCESSFUL`, `AutomationTool exiting with ExitCode=0`; `package-final.log`. Cooked manifest contains **108 SoundWave assets, 15 SoundCue assets**, current skeletal media and Dusk music. Executable SHA-256: `5803ee27a2f06c353826a87e59a21b708b2d800b5b799bdc10367d1efec38cca`.
- Rendered Vulkan/X11 playtest at **1920×1080**, isolated `VisualUser/`: three `presentation.playtest.rifle_infantry` actors and grass terrain loaded; real pointer selection, movement, explicit attack, authoritative damage to death, and select/move/attack/fire/footstep/death cue events observed. Dusk logs looping at gain 0.20, stops on Escape/menu travel and restarts on map relaunch. `startup.png`, `combat-after.png`, `menu-ready.png`, `runtime.log` retain evidence.
- PulseAudio/PipeWire output was captured from this test client's explicitly routed sink, not inferred from cue logs alone. `audio-interaction.wav` measured mean **−25.5 dB**, peak **−7.9 dB** (`audio-levels.log`). These checks establish rendered interaction and non-silent mixed output, not physical-device loudness, audio artistic quality, original-game fidelity, other platforms or Shipping acceptance.
- Test-client Exit completed engine teardown and logged shutdown `outcome=ok`; the launch-tool handle reported **143**, not 0. No fatal/runtime audio errors were logged. This record does not treat that wrapper status as a proven zero-exit process gate.
- Archive: `Saved/RifleAudioRecovery20261029/Package/Linux/`; `SOURCE_REVISION`, generated identity header and verified 37-entry `SHA256SUMS` are included. Manifest SHA-256: `00d9a953db2a08c9d2accafe3ffe47906e2a7d8fb5b6ae95f189767865ea510f`.
- Promoted by verified copy to `Saved/Packages/Linux/`; prior default preserved at `Saved/RifleAudioRecovery20261029/previous-default-Linux/`. `Tools/Infantry/run-playtest.sh` launched the promoted package on the physical desktop at 1920×1080; `player.log` confirms the same source revision, all three presentations, content ready and Dusk gain 0.20. `player-window.png` verifies the visible current model. Physical loudness remains user-evaluated. The visible player session subsequently closed via SDL_EVENT_QUIT with launch-tool exit 0; it was not reopened automatically.
