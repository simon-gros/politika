//! Prepare neutral-named infantry FBX and textures for the Unreal import commandlet.
//! Original extraction inputs remain outside the repository; this output is the game-facing import set.
use std::{env, fs, path::Path, process::Command};

fn copy(source: &Path, target: &Path) {
    fs::copy(source, target).unwrap_or_else(|e| panic!("copy {}: {e}", source.display()));
}

fn main() {
    let args: Vec<_> = env::args_os().skip(1).collect();
    assert_eq!(
        args.len(),
        3,
        "prepare-rifle-infantry <converted-fbx-dir> <all-sources-dir> <output-dir>"
    );
    let converted = Path::new(&args[0]);
    let all_sources = Path::new(&args[1]);
    let out = Path::new(&args[2]);
    fs::create_dir_all(out).unwrap();
    let roles = [
        "Mesh",
        "Idle",
        "Move",
        "Fire",
        "Death",
        "Ready",
        "Select",
        "Deselect",
        "DeathAlternate",
        "DeathBlast",
    ];
    for role in roles {
        let name = format!("RifleInfantry_{role}.fbx");
        let source = converted.join(&name);
        assert!(source.is_file(), "missing {}", source.display());
        copy(&source, &out.join(&name));
    }
    let dds = Path::new("/mnt/data/CommandAndConquer/UM/Command and Conquer 3 Tiberium Wars/infantry-import-20261029/textures/CURiflemanA.dds");
    let diffuse = out.join("RifleInfantry_Diffuse.png");
    let status = Command::new("ffmpeg")
        .args(["-hide_banner", "-loglevel", "error", "-y", "-i"])
        .arg(dds)
        .args(["-frames:v", "1"])
        .arg(&diffuse)
        .status()
        .expect("ffmpeg");
    assert!(
        status.success() && diffuse.metadata().unwrap().len() > 100,
        "DDS diffuse conversion failed"
    );
    let mut source_files: Vec<_> = fs::read_dir(all_sources)
        .expect("extracted source directory")
        .map(|entry| entry.unwrap().path())
        .filter(|path| {
            path.file_name()
                .unwrap()
                .to_string_lossy()
                .starts_with("GURFLMAN_")
                && path
                    .extension()
                    .is_some_and(|extension| extension.eq_ignore_ascii_case("w3x"))
        })
        .collect();
    source_files.sort();
    source_files.push(dds.to_path_buf());
    source_files.push(converted.join("RifleInfantry_Mesh.fbx"));
    // Keep useful integrity evidence without copying external product/faction names
    // into the generated game-facing directory.
    let mut hash_lines = String::new();
    for (index, path) in source_files.iter().enumerate() {
        let hash = Command::new("sha256sum").arg(path).output().expect("sha256sum");
        assert!(hash.status.success());
        let digest = String::from_utf8(hash.stdout).unwrap();
        let digest = digest.split_whitespace().next().unwrap();
        hash_lines.push_str(&format!("source_input_{index:02}  {digest}\n"));
    }
    fs::write(out.join("source-hashes.sha256"), hash_lines).unwrap();
    fs::write(out.join("media-provenance.txt"),
        "RifleInfantry_Mesh/animations: locally extracted W3D source, converted with OpenSAGE Blender W3D importer; see conversion-summary.json.\n\
         RifleInfantry_Diffuse.png: locally extracted diffuse texture converted to PNG with ffmpeg for Unreal texture import.\n\
         Reload and grenade animation roles are not included in this import set.\n\
         Audio is not included in this import set; no substitute audio is represented as source audio.\n").unwrap();
    println!(
        "Prepared {} authored FBX roles and diffuse texture; audio intentionally not fabricated.",
        roles.len()
    );
}
