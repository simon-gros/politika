//! Small procedural primitives; all source coordinates are metres, +Z up.
use glam::{Mat4, Quat, Vec3};
use std::collections::HashMap;

#[derive(Clone, Debug)]
pub struct Mesh {
    pub name: String,
    pub positions: Vec<Vec3>,
    pub indices: Vec<u32>,
    pub uv: Vec<[f32; 2]>,
    pub normals: Vec<Vec3>,
}
impl Mesh {
    pub fn new(vertices: Vec<Vec3>, faces: Vec<u32>) -> Self {
        Self {
            name: String::new(),
            positions: vertices,
            indices: faces,
            uv: vec![],
            normals: vec![],
        }
    }
    pub fn transform(&mut self, matrix: Mat4) {
        for p in &mut self.positions {
            *p = matrix.transform_point3(*p);
        }
        let normal = matrix.inverse().transpose();
        for n in &mut self.normals {
            *n = normal.transform_vector3(*n).normalize_or_zero();
        }
        if matrix.determinant() < 0.0 {
            for f in self.indices.as_chunks_mut::<3>().0 {
                f.swap(1, 2);
            }
        }
    }
    pub fn bounds(&self) -> (Vec3, Vec3) {
        self.positions.iter().fold(
            (Vec3::splat(f32::INFINITY), Vec3::splat(f32::NEG_INFINITY)),
            |(a, b), &p| (a.min(p), b.max(p)),
        )
    }
    pub fn compute_normals(&mut self) {
        self.normals = vec![Vec3::ZERO; self.positions.len()];
        for f in self.indices.as_chunks::<3>().0 {
            let n = (self.positions[f[1] as usize] - self.positions[f[0] as usize])
                .cross(self.positions[f[2] as usize] - self.positions[f[0] as usize]);
            for &i in f {
                self.normals[i as usize] += n;
            }
        }
        for n in &mut self.normals {
            *n = n.normalize_or_zero();
        }
    }
}
pub fn box_mesh(c: [f32; 3], d: [f32; 3]) -> Mesh {
    let c = Vec3::from(c);
    let d = Vec3::from(d) * 0.5;
    let v = [
        (-1., -1., -1.),
        (-1., -1., 1.),
        (-1., 1., -1.),
        (-1., 1., 1.),
        (1., -1., -1.),
        (1., -1., 1.),
        (1., 1., -1.),
        (1., 1., 1.),
    ]
    .map(|(x, y, z)| c + Vec3::new(x, y, z) * d)
    .to_vec();
    Mesh::new(
        v,
        vec![
            1, 3, 0, 4, 1, 0, 0, 3, 2, 2, 4, 0, 1, 7, 3, 5, 1, 4, 5, 7, 1, 3, 7, 2, 6, 4, 2, 2, 7,
            6, 6, 5, 4, 7, 5, 6,
        ],
    )
}
pub fn ellipsoid(c: [f32; 3], scale: [f32; 3], subdivisions: usize) -> Mesh {
    let t = (1. + 5_f32.sqrt()) / 2.;
    let mut v = vec![
        Vec3::new(-1., t, 0.),
        Vec3::new(1., t, 0.),
        Vec3::new(-1., -t, 0.),
        Vec3::new(1., -t, 0.),
        Vec3::new(0., -1., t),
        Vec3::new(0., 1., t),
        Vec3::new(0., -1., -t),
        Vec3::new(0., 1., -t),
        Vec3::new(t, 0., -1.),
        Vec3::new(t, 0., 1.),
        Vec3::new(-t, 0., -1.),
        Vec3::new(-t, 0., 1.),
    ];
    for p in &mut v {
        *p = p.normalize();
    }
    let mut faces = vec![
        [0, 11, 5],
        [0, 5, 1],
        [0, 1, 7],
        [0, 7, 10],
        [0, 10, 11],
        [1, 5, 9],
        [5, 11, 4],
        [11, 10, 2],
        [10, 7, 6],
        [7, 1, 8],
        [3, 9, 4],
        [3, 4, 2],
        [3, 2, 6],
        [3, 6, 8],
        [3, 8, 9],
        [4, 9, 5],
        [2, 4, 11],
        [6, 2, 10],
        [8, 6, 7],
        [9, 8, 1],
    ];
    for _ in 0..subdivisions {
        let mut cache = HashMap::new();
        let mut next = Vec::new();
        for [a, b, c] in faces {
            let mut mid = |a: u32, b: u32| {
                *cache.entry((a.min(b), a.max(b))).or_insert_with(|| {
                    let i = v.len() as u32;
                    v.push((v[a as usize] + v[b as usize]).normalize());
                    i
                })
            };
            let ab = mid(a, b);
            let bc = mid(b, c);
            let ca = mid(c, a);
            next.extend([[a, ab, ca], [b, bc, ab], [c, ca, bc], [ab, bc, ca]]);
        }
        faces = next;
    }
    for p in &mut v {
        *p = *p * Vec3::from(scale) + Vec3::from(c);
    }
    Mesh::new(v, faces.into_iter().flatten().collect())
}
pub fn tube(a: [f32; 3], b: [f32; 3], r: f32, end: f32, n: usize) -> Mesh {
    let a = Vec3::from(a);
    let b = Vec3::from(b);
    let delta = b - a;
    let h = delta.length();
    assert!(h > 1e-5);
    let mut v = Vec::new();
    let mut f = Vec::new();
    for (z, radius) in [(-h / 2., r), (h / 2., end)] {
        for k in 0..n {
            let t = std::f32::consts::TAU * k as f32 / n as f32;
            v.push(Vec3::new(radius * t.cos(), radius * t.sin(), z));
        }
    }
    for k in 0..n {
        let j = (k + 1) % n;
        f.extend([
            k as u32,
            j as u32,
            (n + k) as u32,
            j as u32,
            (n + j) as u32,
            (n + k) as u32,
        ]);
    }
    if (r - end).abs() < 1e-5 {
        let lo = v.len() as u32;
        v.push(Vec3::new(0., 0., -h / 2.));
        let hi = v.len() as u32;
        v.push(Vec3::new(0., 0., h / 2.));
        for k in 0..n {
            let j = (k + 1) % n;
            f.extend([lo, j as u32, k as u32, hi, (n + k) as u32, (n + j) as u32]);
        }
    } else {
        for k in 1..n - 1 {
            f.extend([
                0,
                (k + 1) as u32,
                k as u32,
                n as u32,
                (n + k) as u32,
                (n + k + 1) as u32,
            ]);
        }
    }
    let mut m = Mesh::new(v, f);
    m.transform(Mat4::from_rotation_translation(
        Quat::from_rotation_arc(Vec3::Z, delta / h),
        (a + b) / 2.,
    ));
    m
}
pub fn beam(a: [f32; 3], b: [f32; 3], width: f32, depth: f32) -> Mesh {
    let a = Vec3::from(a);
    let b = Vec3::from(b);
    let d = b - a;
    let mut m = box_mesh([0.; 3], [width, depth, d.length()]);
    m.transform(Mat4::from_rotation_translation(
        Quat::from_rotation_arc(Vec3::Z, d.normalize()),
        (a + b) / 2.,
    ));
    m
}
pub fn block(levels: &[[f32; 5]], n: usize) -> Mesh {
    let mut v = Vec::new();
    let mut f = Vec::new();
    for &[z, x, y, rx, ry] in levels {
        for k in 0..n {
            let t = std::f32::consts::TAU * k as f32 / n as f32;
            v.push(Vec3::new(x + rx * t.cos(), y + ry * t.sin(), z));
        }
    }
    for i in 0..levels.len() - 1 {
        for k in 0..n {
            let j = (k + 1) % n;
            f.extend([
                (i * n + k) as u32,
                (i * n + j) as u32,
                ((i + 1) * n + j) as u32,
                (i * n + k) as u32,
                ((i + 1) * n + j) as u32,
                ((i + 1) * n + k) as u32,
            ]);
        }
    }
    let top = (levels.len() - 1) * n;
    // Outward-facing caps (legacy source had these reversed).
    for k in 1..n - 1 {
        f.extend([
            0,
            (k + 1) as u32,
            k as u32,
            top as u32,
            (top + k) as u32,
            (top + k + 1) as u32,
        ]);
    }
    Mesh::new(v, f)
}
pub fn prism(outline: &[[f32; 2]], y: f32, thickness: f32) -> Mesh {
    let n = outline.len();
    let mut v = Vec::new();
    let mut f = Vec::new();
    for yy in [y - thickness / 2., y + thickness / 2.] {
        for &[x, z] in outline {
            v.push(Vec3::new(x, yy, z));
        }
    }
    for i in 1..n - 1 {
        f.extend([
            0,
            (i + 1) as u32,
            i as u32,
            n as u32,
            (n + i) as u32,
            (n + i + 1) as u32,
        ]);
    }
    for j in 0..n {
        let k = (j + 1) % n;
        f.extend([
            j as u32,
            k as u32,
            (n + k) as u32,
            j as u32,
            (n + k) as u32,
            (n + j) as u32,
        ]);
    }
    Mesh::new(v, f)
}
