//! Decode mapped rifle-infantry audio and encode game-ready MP3s from local EA cdata.
//! rustc Tools/InfantryAudio/extract_rifle_audio.rs -o Saved/InfantryImport/extract-rifle-audio
//! extract-rifle-audio <global_common.manifest> <cdata-directory> <output-directory>
use std::{convert::TryInto, env, fs, path::Path, process::Command};

fn u32at(data: &[u8], offset: usize) -> u32 {
    u32::from_le_bytes(data[offset..offset + 4].try_into().unwrap())
}

fn cstr(data: &[u8], offset: usize) -> String {
    let end = data[offset..]
        .iter()
        .position(|b| *b == 0)
        .map(|n| offset + n)
        .unwrap_or(data.len());
    String::from_utf8_lossy(&data[offset..end]).into_owned()
}

fn asset_name(key: &str) -> String {
    let source_name = key.trim_start_matches("AudioFile:GURifle_");
    let (prefix, role) = [
        ("digginL2", "Voice_DigIn"),
        ("foleSele", "Foley_Select"),
        ("gun5Fire", "Weapon_RifleShot"),
        ("moveG1L2", "Foley_Move"),
        ("VoiAttack", "Voice_Attack"),
        ("VoiCreate", "Voice_Create"),
        ("Voidie", "Voice_Death"),
        ("VoiGarris", "Voice_Garrison"),
        ("VoiMovAtt", "Voice_MoveAttack"),
        ("VoiMove", "Voice_Move"),
        ("VoiRetrea", "Voice_Retreat"),
        ("VoiSelBat", "Voice_SelectBattle"),
        ("VoiSelect", "Voice_Select"),
        ("VoiSpeci1", "Voice_Special"),
        ("VoiSuppre", "Voice_Suppressed"),
    ]
    .into_iter()
    .find(|(prefix, _)| source_name.starts_with(prefix))
    .unwrap_or_else(|| panic!("unmapped rifle audio key: {key}"));
    let suffix = &source_name[prefix.len()..];
    format!("PT_RifleInfantry_{role}_{}", suffix.to_ascii_uppercase())
}

fn main() {
    let args: Vec<_> = env::args().skip(1).collect();
    assert!(
        (3..=4).contains(&args.len()),
        "expected manifest, cdata directory, output directory, optional shared cdata directory"
    );
    let manifest_path = Path::new(&args[0]);
    let cdata_dir = Path::new(&args[1]);
    let out = Path::new(&args[2]);
    fs::create_dir_all(out).unwrap();
    let data = fs::read(manifest_path).unwrap();
    let count = u32at(&data, 0x0c) as usize;
    let refs = u32at(&data, 0x20) as usize;
    let streams = u32at(&data, 0x24) as usize;
    let entries = 0x30;
    let names_offset = entries + count * 0x2c + refs + streams;
    let mut ledger = String::from("wav\tmp3\tsource_key\tcdata_id\tsha256_mp3\n");
    let mut decoded = 0usize;
    for i in 0..count {
        let e = entries + i * 0x2c;
        let key = cstr(&data, names_offset + u32at(&data, e + 0x18) as usize);
        if !key.starts_with("AudioFile:GURifle_") {
            continue;
        }
        let id = format!(
            "{:08x}.{:08x}.{:08x}.{:08x}",
            u32at(&data, e),
            u32at(&data, e + 8),
            u32at(&data, e + 4),
            u32at(&data, e + 12)
        );
        let mut source = cdata_dir.join(format!("{id}.cdata"));
        if !source.is_file() && args.len() == 4 {
            source = Path::new(&args[3]).join(format!("{id}.cdata"));
        }
        assert!(
            source.is_file(),
            "missing cdata for {key}: {}",
            source.display()
        );
        let stem = asset_name(&key);
        let output = out.join(format!("{stem}.wav"));
        let status = Command::new("ffmpeg")
            .args([
                "-hide_banner",
                "-loglevel",
                "error",
                "-y",
                "-f",
                "ea_cdata",
                "-i",
            ])
            .arg(&source)
            .args(["-ar", "48000", "-ac", "1", "-c:a", "pcm_s16le"])
            .arg(&output)
            .status()
            .expect("ffmpeg");
        assert!(status.success(), "failed decoding {key}");
        let mp3 = out.join(format!("{stem}.mp3"));
        let status = Command::new("ffmpeg")
            .args(["-hide_banner", "-loglevel", "error", "-y", "-i"])
            .arg(&output)
            .args(["-codec:a", "libmp3lame", "-q:a", "2", "-id3v2_version", "0"])
            .arg(&mp3)
            .status()
            .expect("ffmpeg");
        assert!(status.success(), "failed MP3 encoding {key}");
        let hash = Command::new("sha256sum").arg(&mp3).output().unwrap();
        assert!(hash.status.success());
        let digest = String::from_utf8_lossy(&hash.stdout)
            .split_whitespace()
            .next()
            .unwrap()
            .to_owned();
        ledger.push_str(&format!(
            "{}\t{}\t{}\t{}\t{}\n",
            output.file_name().unwrap().to_string_lossy(),
            mp3.file_name().unwrap().to_string_lossy(),
            key,
            id,
            digest
        ));
        decoded += 1;
    }
    assert!(decoded > 0, "no mapped rifle audio found in manifest");
    fs::write(out.join("source-map.tsv"), ledger).unwrap();
    println!(
        "Decoded {decoded} mapped rifle audio files to {}",
        out.display()
    );
}
