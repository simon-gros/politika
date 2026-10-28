//! Strict, self-contained textured static GLB reader/writer (no external URIs).
use crate::geometry::Mesh;
use anyhow::{Context, Result, bail, ensure};
use glam::{Mat4, Vec3};
use gltf::{buffer::Source, image::Source as ImageSource, mesh::Mode};
use serde_json::{Value, json};
use std::collections::HashSet;

fn align(bin: &mut Vec<u8>) {
    while !bin.len().is_multiple_of(4) {
        bin.push(0);
    }
}
fn view(bin: &mut Vec<u8>, views: &mut Vec<Value>, bytes: &[u8], target: Option<u32>) -> usize {
    align(bin);
    let offset = bin.len();
    bin.extend_from_slice(bytes);
    let mut v = json!({"buffer":0,"byteOffset":offset,"byteLength":bytes.len()});
    if let Some(t) = target {
        v["target"] = json!(t);
    }
    views.push(v);
    views.len() - 1
}
fn floats(values: impl Iterator<Item = f32>) -> Vec<u8> {
    values.flat_map(f32::to_le_bytes).collect()
}
pub fn encode(meshes: &[Mesh], png: &[u8]) -> Result<Vec<u8>> {
    ensure!(!meshes.is_empty(), "empty scene");
    let mut bin = Vec::new();
    let mut views = Vec::new();
    let mut accessors = Vec::new();
    let mut objects = Vec::new();
    let mut nodes = Vec::new();
    for m in meshes {
        validate_mesh(m)?;
        let (lo, hi) = m.bounds();
        let base = accessors.len();
        let p = view(
            &mut bin,
            &mut views,
            &floats(m.positions.iter().flat_map(|p| p.to_array())),
            Some(34962),
        );
        accessors.push(json!({"bufferView":p,"componentType":5126,"count":m.positions.len(),"type":"VEC3","min":lo.to_array(),"max":hi.to_array()}));
        let n = view(
            &mut bin,
            &mut views,
            &floats(m.normals.iter().flat_map(|p| p.to_array())),
            Some(34962),
        );
        accessors.push(
            json!({"bufferView":n,"componentType":5126,"count":m.normals.len(),"type":"VEC3"}),
        );
        let u = view(
            &mut bin,
            &mut views,
            &floats(m.uv.iter().flat_map(|u| *u)),
            Some(34962),
        );
        accessors
            .push(json!({"bufferView":u,"componentType":5126,"count":m.uv.len(),"type":"VEC2"}));
        let i = view(
            &mut bin,
            &mut views,
            &m.indices
                .iter()
                .flat_map(|i| i.to_le_bytes())
                .collect::<Vec<_>>(),
            Some(34963),
        );
        accessors.push(
            json!({"bufferView":i,"componentType":5125,"count":m.indices.len(),"type":"SCALAR"}),
        );
        nodes.push(json!({"name":m.name,"mesh":objects.len()}));
        objects.push(json!({"name":m.name,"primitives":[{"attributes":{"POSITION":base,"NORMAL":base+1,"TEXCOORD_0":base+2},"indices":base+3,"material":0,"mode":4}]}));
    }
    let im = view(&mut bin, &mut views, png, None);
    align(&mut bin);
    let root = json!({"asset":{"version":"2.0","generator":"POLITIKA Rust infantry authoring"},"scene":0,"scenes":[{"nodes":(0..nodes.len()).collect::<Vec<_>>()}],"nodes":nodes,"meshes":objects,"buffers":[{"byteLength":bin.len()}],"bufferViews":views,"accessors":accessors,"images":[{"bufferView":im,"mimeType":"image/png"}],"samplers":[{"wrapS":33071,"wrapT":33071}],"textures":[{"source":0,"sampler":0}],"materials":[{"name":"AC_INF_01_original_color_atlas","doubleSided":true,"pbrMetallicRoughness":{"baseColorTexture":{"index":0},"metallicFactor":0.0,"roughnessFactor":0.92}}]});
    let mut js = serde_json::to_vec(&root)?;
    while !js.len().is_multiple_of(4) {
        js.push(b' ');
    }
    let size = u32::try_from(28 + js.len() + bin.len()).context("GLB exceeds 4 GiB")?;
    let mut out = Vec::new();
    out.extend_from_slice(b"glTF");
    out.extend_from_slice(&2u32.to_le_bytes());
    out.extend_from_slice(&size.to_le_bytes());
    out.extend_from_slice(&(js.len() as u32).to_le_bytes());
    out.extend_from_slice(b"JSON");
    out.extend(js);
    out.extend_from_slice(&(bin.len() as u32).to_le_bytes());
    out.extend_from_slice(b"BIN\0");
    out.extend(bin);
    Ok(out)
}
pub fn validate_mesh(m: &Mesh) -> Result<()> {
    ensure!(
        !m.positions.is_empty() && !m.indices.is_empty() && m.indices.len().is_multiple_of(3),
        "empty/nontriangular mesh {}",
        m.name
    );
    ensure!(
        m.uv.len() == m.positions.len() && m.normals.len() == m.positions.len(),
        "missing UV/normals: {}",
        m.name
    );
    ensure!(
        m.positions.iter().all(|p| p.is_finite())
            && m.normals
                .iter()
                .all(|p| p.is_finite() && (p.length() - 1.).abs() < 0.01),
        "invalid vertex/normal: {}",
        m.name
    );
    ensure!(
        m.uv.iter()
            .flatten()
            .all(|v| v.is_finite() && (0.0..=1.0).contains(v)),
        "invalid UV: {}",
        m.name
    );
    ensure!(
        m.indices.iter().all(|&i| (i as usize) < m.positions.len()),
        "index out of bounds: {}",
        m.name
    );
    Ok(())
}
pub fn decode(bytes: &[u8]) -> Result<(Vec<Mesh>, Vec<u8>)> {
    let doc = gltf::Gltf::from_slice(bytes).context("parse/validate glTF")?;
    let bin = doc.blob.as_deref().context("requires binary GLB buffer")?;
    ensure!(
        doc.buffers().count() == 1 && doc.buffers().all(|b| matches!(b.source(), Source::Bin)),
        "external/multiple buffers unsupported"
    );
    ensure!(
        doc.animations().count() == 0 && doc.skins().count() == 0,
        "only static meshes supported"
    );
    let mut meshes = Vec::new();
    let mut png: Option<Vec<u8>> = None;
    fn walk(
        node: gltf::Node<'_>,
        parent: Mat4,
        bin: &[u8],
        out: &mut Vec<Mesh>,
        png: &mut Option<Vec<u8>>,
        path: &mut HashSet<usize>,
        depth: usize,
    ) -> Result<()> {
        ensure!(
            depth < 128 && path.insert(node.index()),
            "cyclic/overdeep scene graph"
        );
        let world = parent * Mat4::from_cols_array_2d(&node.transform().matrix());
        ensure!(
            world.is_finite() && world.determinant().abs() > 1e-12,
            "singular/nonfinite transform"
        );
        if let Some(mesh) = node.mesh() {
            for primitive in mesh.primitives() {
                ensure!(
                    primitive.mode() == Mode::Triangles,
                    "only triangles supported"
                );
                ensure!(
                    primitive.morph_targets().next().is_none(),
                    "morph targets unsupported"
                );
                let material = primitive.material();
                let pbr = material.pbr_metallic_roughness();
                ensure!(
                    pbr.base_color_factor() == [1.0; 4]
                        && pbr.metallic_factor() == 0.0
                        && (pbr.roughness_factor() - 0.92).abs() < 1e-5
                        && material.double_sided()
                        && material.alpha_mode() == gltf::material::AlphaMode::Opaque
                        && material.normal_texture().is_none()
                        && material.occlusion_texture().is_none()
                        && material.emissive_texture().is_none()
                        && material.emissive_factor() == [0.0; 3]
                        && pbr.metallic_roughness_texture().is_none(),
                    "unsupported material: requires original opaque, double-sided, nonmetallic atlas profile"
                );
                let tex = primitive
                    .material()
                    .pbr_metallic_roughness()
                    .base_color_texture()
                    .context("refusing mesh without base-color texture")?;
                ensure!(tex.tex_coord() == 0, "requires TEXCOORD_0");
                let texture = match tex.texture().source().source() {
                    ImageSource::View { view, mime_type } => {
                        ensure!(mime_type == "image/png", "requires PNG atlas");
                        let end = view
                            .offset()
                            .checked_add(view.length())
                            .context("image overflow")?;
                        bin.get(view.offset()..end)
                            .context("image view out of bounds")?
                            .to_vec()
                    }
                    _ => bail!("external images unsupported"),
                };
                if let Some(existing) = png.as_ref() {
                    ensure!(*existing == texture, "multiple atlases unsupported");
                } else {
                    *png = Some(texture);
                }
                let reader = primitive.reader(|_| Some(bin));
                let positions = reader
                    .read_positions()
                    .context("missing POSITION")?
                    .map(Vec3::from)
                    .collect();
                let uv = reader
                    .read_tex_coords(0)
                    .context("missing UV")?
                    .into_f32()
                    .collect();
                let normals = reader
                    .read_normals()
                    .map(|n| n.map(Vec3::from).collect())
                    .unwrap_or_default();
                let indices = reader
                    .read_indices()
                    .context("missing indices")?
                    .into_u32()
                    .collect();
                let mut m = Mesh {
                    name: mesh.name().unwrap_or("unnamed").to_string(),
                    positions,
                    indices,
                    uv,
                    normals,
                };
                ensure!(
                    m.indices.iter().all(|&i| (i as usize) < m.positions.len()),
                    "index out of bounds"
                );
                if m.normals.is_empty() {
                    m.compute_normals();
                }
                validate_mesh(&m)?;
                m.transform(world);
                validate_mesh(&m)?;
                out.push(m);
            }
        }
        for child in node.children() {
            walk(child, world, bin, out, png, path, depth + 1)?;
        }
        path.remove(&node.index());
        Ok(())
    }
    let scene = doc
        .default_scene()
        .or_else(|| doc.scenes().next())
        .context("missing scene")?;
    for node in scene.nodes() {
        walk(
            node,
            Mat4::IDENTITY,
            bin,
            &mut meshes,
            &mut png,
            &mut HashSet::new(),
            0,
        )?;
    }
    ensure!(!meshes.is_empty(), "no meshes");
    let png = png.context("missing atlas")?;
    let image = image::load_from_memory_with_format(&png, image::ImageFormat::Png)
        .context("invalid PNG")?;
    ensure!(
        image.width() == 1280 && image.height() == 1024,
        "unexpected atlas dimensions"
    );
    Ok((meshes, png))
}
pub fn merge(meshes: &[Mesh]) -> Result<Mesh> {
    let mut merged = Mesh::new(vec![], vec![]);
    merged.name = "SM_AC_INF_01_Playtest".to_string();
    for m in meshes {
        validate_mesh(m)?;
        let base = u32::try_from(merged.positions.len())?;
        merged.positions.extend(&m.positions);
        merged.normals.extend(&m.normals);
        merged.uv.extend(&m.uv);
        for &i in &m.indices {
            merged
                .indices
                .push(base.checked_add(i).context("index overflow")?);
        }
    }
    validate_mesh(&merged)?;
    Ok(merged)
}
