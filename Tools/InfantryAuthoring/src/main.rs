mod design;
mod geometry;
mod glb;
use anyhow::{Context, Result, bail, ensure};
use glam::Vec3;
use image::{ImageEncoder, Rgb, RgbImage, codecs::png::PngEncoder};
use serde_json::{Value, json};
use sha2::{Digest, Sha256};
use std::{
    fs,
    io::{Cursor, Write},
    path::{Path, PathBuf},
};
use zip::{ZipWriter, write::SimpleFileOptions};
const REVIEW: &str = "AC-INF-01_Professional_Rifle_Section_v2.glb";
const PLAYTEST: &str = "SM_AC_INF_01_Playtest.glb";
const ATLAS: &str = "AC-INF-01_color_atlas.png";
const MANIFEST: &str = "AC-INF-01_manifest_v2.json";
const README: &str = "AC-INF-01_README_v2.md";
fn atlas() -> Result<Vec<u8>> {
    // Fixed xorshift seed and Box-Muller noise; reproducible without numpy/RNG-version coupling.
    let mut state = 1310u64;
    let mut uniform = || {
        state ^= state << 13;
        state ^= state >> 7;
        state ^= state << 17;
        ((state >> 40) as f32 + 0.5) / 16777216.
    };
    let mut normal = || (-2. * uniform().ln()).sqrt() * (std::f32::consts::TAU * uniform()).cos();
    let mut img = RgbImage::new(1280, 1024);
    for (i, color) in design::COLORS.iter().enumerate() {
        for y in 0..256 {
            let row = normal() * 2.;
            for x in 0..256 {
                let mut noise = normal() * 4.2 + row;
                let mut waves =
                    (x as f32 * 0.16 + y as f32 * 0.07).sin() * 1.8 + (y as f32 * 0.27).sin() * 1.1;
                if [12, 13, 17].contains(&i) {
                    noise *= 0.42;
                    waves *= 0.6;
                }
                if [15, 16].contains(&i) {
                    noise *= 0.55;
                }
                img.put_pixel(
                    (i as u32 % 5) * 256 + x,
                    (i as u32 / 5) * 256 + y,
                    Rgb(color.map(|c| (c as f32 + noise + waves).clamp(0., 255.) as u8)),
                );
            }
        }
    }
    let mut png = Vec::new();
    PngEncoder::new(&mut png).write_image(
        img.as_raw(),
        1280,
        1024,
        image::ExtendedColorType::Rgb8,
    )?;
    Ok(png)
}
fn summary(meshes: &[geometry::Mesh]) -> Result<Value> {
    let triangles: usize = meshes.iter().map(|m| m.indices.len() / 3).sum();
    ensure!(
        triangles == 22544,
        "unexpected triangle count {triangles} (expected 22544)"
    );
    let (lo, hi) = meshes.iter().map(|m| m.bounds()).fold(
        (Vec3::splat(f32::INFINITY), Vec3::splat(f32::NEG_INFINITY)),
        |(a, b), (c, d)| (a.min(c), b.max(d)),
    );
    let ext = hi - lo;
    ensure!((1.83..2.05).contains(&ext.z), "unexpected standing height");
    Ok(
        json!({"mesh_objects":meshes.len(),"triangles":triangles,"bounding_box_m":[lo.to_array(),hi.to_array()],"dimensions_m":ext.to_array()}),
    )
}
fn write_new(path: &Path, bytes: &[u8]) -> Result<()> {
    // Never overwrite an existing review or approved asset. Validate all data before writing.
    let mut file = fs::OpenOptions::new()
        .write(true)
        .create_new(true)
        .open(path)
        .with_context(|| format!("refusing overwrite or cannot create {}", path.display()))?;
    file.write_all(bytes)?;
    Ok(())
}
fn digest(bytes: &[u8]) -> String {
    format!("{:x}", Sha256::digest(bytes))
}
fn generate(out: &Path) -> Result<()> {
    let meshes = design::build();
    let mut names = std::collections::HashSet::new();
    ensure!(
        meshes.iter().all(|m| names.insert(&m.name)),
        "duplicate component name"
    );
    let stats = summary(&meshes)?;
    ensure!(meshes.len() > 100, "missing components");
    let png = atlas()?;
    let bytes = glb::encode(&meshes, &png)?;
    let (check, check_png) = glb::decode(&bytes)?;
    ensure!(
        check_png == png && summary(&check)? == stats,
        "GLB readback mismatch"
    );
    let mut manifest = stats;
    manifest["asset_id"] = json!("AC-INF-01");
    manifest["revision"] = json!("v2-rust-review");
    manifest["status"] =
        json!("Original procedural static review candidate; not approved/imported/packaged");
    manifest["reference"] = json!("Factions/TheAtlanticConcord/Units/AC-INF-01.png");
    manifest["units"] = json!("meters");
    manifest["orientation"] = json!("+Z up; forward -Y; verify axes at import");
    manifest["sha256"] = json!(digest(&bytes));
    manifest["components"]=json!(meshes.iter().map(|m|json!({"name":m.name,"vertices":m.positions.len(),"triangles":m.indices.len()/3})).collect::<Vec<_>>());
    manifest["limitations"] = json!([
        "Static posed mesh; no rig, weights, animations, sockets, LODs or collision",
        "Intersecting procedural clothing/gear; not production retopology",
        "Rust texture noise and corrected cap winding differ from legacy export",
        "Blender/Unreal import and cooked playtest not run; historical styling not certified"
    ]);
    let manifest = serde_json::to_vec_pretty(&manifest)?;
    let readme=b"# AC-INF-01 Rust-generated review candidate\n\nImport the GLB into Blender and use Material Preview. Named components, UVs and a 1280x1024 PNG atlas are embedded. Original procedural design derives from Factions/TheAtlanticConcord/Units/AC-INF-01.png. This static posed model is unrigged, unapproved and not packaged. Metres, +Z up, forward -Y: verify unit and axis conversion in Unreal. Review silhouette, cap winding and texture before adopting. No uasset is modified. See the manifest for counts, dimensions, SHA-256 and limitations.\n";
    let files = [
        (REVIEW, bytes.as_slice()),
        (ATLAS, png.as_slice()),
        (MANIFEST, manifest.as_slice()),
        (README, readme.as_slice()),
    ];
    let mut zip = ZipWriter::new(Cursor::new(Vec::new()));
    for (name, data) in files {
        zip.start_file(
            name,
            SimpleFileOptions::default().compression_method(zip::CompressionMethod::Deflated),
        )?;
        zip.write_all(data)?;
    }
    let archive = zip.finish()?.into_inner();
    let archive_name = "AC-INF-01_v2_Blender_Review_Package.zip";
    for name in [REVIEW, ATLAS, MANIFEST, README, archive_name] {
        ensure!(
            !out.join(name).exists(),
            "refusing overwrite: {}",
            out.join(name).display()
        );
    }
    fs::create_dir_all(out)?;
    for (name, data) in files {
        write_new(&out.join(name), data)?;
    }
    write_new(&out.join(archive_name), &archive)?;
    println!(
        "{}",
        serde_json::to_string_pretty(
            &json!({"output":out,"sha256":digest(&bytes),"geometry":summary(&check)?})
        )?
    );
    Ok(())
}
fn merge(input: &Path, output: &Path) -> Result<()> {
    ensure!(input != output, "input and output must differ");
    let (meshes, png) = glb::decode(&fs::read(input)?)?;
    summary(&meshes)?;
    let mesh = glb::merge(&meshes)?;
    let bytes = glb::encode(&[mesh], &png)?;
    let (check, check_png) = glb::decode(&bytes)?;
    ensure!(
        check.len() == 1 && png == check_png,
        "merged readback mismatch"
    );
    summary(&check)?;
    if let Some(parent) = output.parent().filter(|p| !p.as_os_str().is_empty()) {
        fs::create_dir_all(parent)?;
    }
    write_new(output, &bytes)?;
    println!(
        "{}",
        json!({"output":output,"sha256":digest(&bytes),"geometry":summary(&check)?})
    );
    Ok(())
}
fn main() -> Result<()> {
    let mut args = std::env::args().skip(1);
    let command = args.next().unwrap_or_else(|| "help".into());
    let root = PathBuf::from(env!("CARGO_MANIFEST_DIR"))
        .join("../../Art/Prototypes/Infantry/AC-INF-01/Generated");
    match command.as_str() {
        "generate" => {
            let out = option(&mut args, "--output-dir", root)?;
            generate(&out)
        }
        "merge" => {
            let mut input = root.join(REVIEW);
            let mut output = root.join(PLAYTEST);
            while let Some(flag) = args.next() {
                match flag.as_str() {
                    "--input" => {
                        input = PathBuf::from(args.next().context("missing --input path")?)
                    }
                    "--output" => {
                        output = PathBuf::from(args.next().context("missing --output path")?)
                    }
                    _ => bail!("unknown option {flag}"),
                }
            }
            merge(&input, &output)
        }
        "inspect" => {
            let path = PathBuf::from(args.next().context("inspect requires GLB path")?);
            ensure!(args.next().is_none(), "unexpected arguments");
            let bytes = fs::read(path)?;
            let (meshes, _) = glb::decode(&bytes)?;
            println!(
                "{}",
                json!({"sha256":digest(&bytes),"geometry":summary(&meshes)?})
            );
            Ok(())
        }
        "help" | "--help" | "-h" => {
            println!(
                "generate [--output-dir DIR]\nmerge [--input REVIEW.glb] [--output PLAYTEST.glb]\ninspect FILE.glb\nNo existing file is overwritten. Defaults are in Art/Prototypes/Infantry/AC-INF-01/Generated."
            );
            Ok(())
        }
        _ => bail!("unknown command {command}; use --help"),
    }
}
fn option(
    args: &mut impl Iterator<Item = String>,
    flag: &str,
    default: PathBuf,
) -> Result<PathBuf> {
    match args.next() {
        None => Ok(default),
        Some(arg) => {
            ensure!(arg == flag, "unknown option {arg}");
            let value = PathBuf::from(args.next().context("missing option value")?);
            ensure!(args.next().is_none(), "unexpected arguments");
            Ok(value)
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use std::sync::atomic::{AtomicUsize, Ordering};
    static NEXT: AtomicUsize = AtomicUsize::new(0);
    struct Temp(PathBuf);
    impl Temp {
        fn new() -> Self {
            let p = std::env::temp_dir().join(format!(
                "politika-infantry-{}-{}",
                std::process::id(),
                NEXT.fetch_add(1, Ordering::Relaxed)
            ));
            fs::create_dir(&p).unwrap();
            Self(p)
        }
    }
    impl Drop for Temp {
        fn drop(&mut self) {
            let _ = fs::remove_dir_all(&self.0);
        }
    }
    fn fixture() -> (Vec<geometry::Mesh>, Vec<u8>) {
        (design::build(), atlas().unwrap())
    }
    // Edit only the JSON chunk, preserving the binary data and GLB alignment.
    fn edit(bytes: &[u8], f: impl FnOnce(&mut Value)) -> Vec<u8> {
        let len = u32::from_le_bytes(bytes[12..16].try_into().unwrap()) as usize;
        let mut root: Value = serde_json::from_slice(&bytes[20..20 + len]).unwrap();
        f(&mut root);
        let mut js = serde_json::to_vec(&root).unwrap();
        while !js.len().is_multiple_of(4) {
            js.push(b' ');
        }
        let mut out = bytes[..12].to_vec();
        out.extend_from_slice(&(js.len() as u32).to_le_bytes());
        out.extend_from_slice(b"JSON");
        out.extend(js);
        out.extend_from_slice(&bytes[20 + len..]);
        let size = out.len() as u32;
        out[8..12].copy_from_slice(&size.to_le_bytes());
        out
    }
    #[test]
    fn recipe_and_roundtrip() {
        let (meshes, png) = fixture();
        assert_eq!(meshes.len(), 299);
        let stats = summary(&meshes).unwrap();
        let bytes = glb::encode(&meshes, &png).unwrap();
        let (read, texture) = glb::decode(&bytes).unwrap();
        assert_eq!(summary(&read).unwrap(), stats);
        assert_eq!(texture, png);
        for (a, b) in meshes.iter().zip(read) {
            assert_eq!(a.name, b.name);
            assert_eq!(a.positions, b.positions);
            assert_eq!(a.indices, b.indices);
            assert_eq!(a.uv, b.uv);
        }
    }
    #[test]
    fn parent_transforms_and_reflection_are_baked() {
        let (meshes, png) = fixture();
        let m = glb::merge(&meshes).unwrap();
        let bytes = glb::encode(std::slice::from_ref(&m), &png).unwrap();
        let bytes = edit(&bytes, |r| {
            r["nodes"][0]["scale"] = json!([-2., 3., 0.5]);
            r["nodes"]
                .as_array_mut()
                .unwrap()
                .push(json!({"children":[0],"translation":[4.,5.,6.]}));
            r["scenes"][0]["nodes"] = json!([1]);
        });
        let (read, _) = glb::decode(&bytes).unwrap();
        let r = &read[0];
        for (&p, &actual) in m.positions.iter().zip(&r.positions) {
            let expected = p * Vec3::new(-2., 3., 0.5) + Vec3::new(4., 5., 6.);
            assert!(expected.abs_diff_eq(actual, 1e-6));
        }
        assert_eq!(&r.indices[..3], &[m.indices[0], m.indices[2], m.indices[1]]);
        for (&n, &actual) in m.normals.iter().zip(&r.normals) {
            let expected = (n / Vec3::new(-2., 3., 0.5)).normalize();
            assert!(expected.abs_diff_eq(actual, 1e-6));
        }
    }
    #[test]
    fn reject_untextured_external_nontriangular_and_bad_uv() {
        let (meshes, png) = fixture();
        let bytes = glb::encode(&[glb::merge(&meshes).unwrap()], &png).unwrap();
        for edit_fn in [
            (|r: &mut Value| {
                r["meshes"][0]["primitives"][0]["attributes"]
                    .as_object_mut()
                    .unwrap()
                    .remove("TEXCOORD_0");
            }) as fn(&mut Value),
            |r| {
                r["materials"][0]["pbrMetallicRoughness"]
                    .as_object_mut()
                    .unwrap()
                    .remove("baseColorTexture");
            },
            |r| {
                r["buffers"][0]["uri"] = json!("untrusted.bin");
            },
            |r| {
                r["materials"][0]["pbrMetallicRoughness"]["baseColorFactor"] =
                    json!([0.5, 1.0, 1.0, 1.0]);
            },
            |r| {
                r["images"][0] = json!({"uri":"untrusted.png"});
            },
            |r| {
                r["meshes"][0]["primitives"][0]["mode"] = json!(1);
            },
            |r| {
                r["nodes"][0]["scale"] = json!([0., 1., 1.]);
            },
        ] {
            assert!(glb::decode(&edit(&bytes, edit_fn)).is_err());
        }
        assert!(glb::decode(b"interrupted GLB").is_err());
        let mut bad = meshes[0].clone();
        bad.uv[0][0] = f32::NAN;
        assert!(glb::encode(&[bad], &png).is_err());
    }
    #[test]
    fn legacy_without_normals_is_supported() {
        let (meshes, png) = fixture();
        let bytes = glb::encode(&[glb::merge(&meshes).unwrap()], &png).unwrap();
        let bytes = edit(&bytes, |r| {
            r["meshes"][0]["primitives"][0]["attributes"]
                .as_object_mut()
                .unwrap()
                .remove("NORMAL");
        });
        assert!(glb::decode(&bytes).is_ok());
    }
    #[test]
    fn generation_merge_determinism_zip_and_overwrite_guard() {
        let a = Temp::new();
        let b = Temp::new();
        generate(&a.0).unwrap();
        generate(&b.0).unwrap();
        for name in [
            REVIEW,
            ATLAS,
            MANIFEST,
            README,
            "AC-INF-01_v2_Blender_Review_Package.zip",
        ] {
            assert_eq!(
                fs::read(a.0.join(name)).unwrap(),
                fs::read(b.0.join(name)).unwrap()
            );
        }
        let original = fs::read(a.0.join(REVIEW)).unwrap();
        assert!(generate(&a.0).is_err());
        assert_eq!(fs::read(a.0.join(REVIEW)).unwrap(), original);
        let mut zip = zip::ZipArchive::new(Cursor::new(
            fs::read(a.0.join("AC-INF-01_v2_Blender_Review_Package.zip")).unwrap(),
        ))
        .unwrap();
        assert_eq!(zip.len(), 4);
        for name in [REVIEW, ATLAS, MANIFEST, README] {
            let mut entry = zip.by_name(name).unwrap();
            let mut bytes = vec![];
            std::io::Read::read_to_end(&mut entry, &mut bytes).unwrap();
            assert_eq!(bytes, fs::read(a.0.join(name)).unwrap());
        }
        let output = a.0.join(PLAYTEST);
        merge(&a.0.join(REVIEW), &output).unwrap();
        let bytes = fs::read(&output).unwrap();
        let (read, png) = glb::decode(&bytes).unwrap();
        assert_eq!(read.len(), 1);
        assert_eq!(summary(&read).unwrap()["triangles"], 22544);
        assert_eq!(png, fs::read(a.0.join(ATLAS)).unwrap());
        assert!(merge(&a.0.join(REVIEW), &output).is_err());
        assert_eq!(fs::read(&output).unwrap(), bytes);
    }
    #[test]
    fn block_caps_face_outward() {
        let mut m = geometry::block(&[[0., 0., 0., 1., 1.], [1., 0., 0., 1., 1.]], 16);
        m.compute_normals();
        for face in m.indices.as_chunks::<3>().0 {
            let a = m.positions[face[0] as usize];
            let b = m.positions[face[1] as usize];
            let c = m.positions[face[2] as usize];
            let n = (b - a).cross(c - a);
            if a.z == b.z && b.z == c.z {
                assert!(if a.z == 0. { n.z < 0. } else { n.z > 0. });
            }
        }
    }
}
