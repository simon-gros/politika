//! Original AC-INF-01 procedural recipe, ported to Rust. Coordinates are metres, Z-up.
use crate::geometry::*;
use glam::{Mat4, Quat, Vec3};
pub const COLORS: [[u8; 3]; 20] = [
    [111, 115, 99],
    [82, 88, 77],
    [128, 129, 111],
    [102, 108, 94],
    [65, 70, 63],
    [101, 102, 83],
    [136, 132, 107],
    [92, 98, 79],
    [75, 83, 69],
    [66, 64, 55],
    [52, 54, 49],
    [56, 59, 52],
    [46, 49, 47],
    [88, 90, 86],
    [99, 88, 70],
    [174, 152, 131],
    [136, 113, 100],
    [75, 89, 89],
    [74, 83, 71],
    [203, 204, 188],
];
const CLOTH: usize = 0;
const SHADOW: usize = 1;
const LIGHT: usize = 2;
const HELMET: usize = 3;
const EDGE: usize = 4;
const WEB: usize = 5;
const WEBLIGHT: usize = 6;
const PACK: usize = 7;
const PACKSHADOW: usize = 8;
const LEATHER: usize = 9;
const BOOT: usize = 10;
const GLOVE: usize = 11;
const METAL: usize = 12;
const STEEL: usize = 13;
const STOCK: usize = 14;
const SKIN: usize = 15;
const SKINSHADOW: usize = 16;
const GLASS: usize = 17;
const PATCH: usize = 18;
const PATCHLINE: usize = 19;
pub fn build() -> Vec<Mesh> {
    let mut meshes = Vec::new();
    let mut frame = Mat4::IDENTITY;
    macro_rules! add {
        ($name:expr,$m:expr,$mat:expr) => {{
            let mut m = $m;
            m.name = $name.to_string();
            m.transform(frame);
            let (lo, hi) = m.bounds();
            let span = hi - lo;
            let col = ($mat % 5) as f32;
            let row = ($mat / 5) as f32;
            m.uv = m
                .positions
                .iter()
                .map(|p| {
                    let u =
                        ((p.x - lo.x) / if span.x > 0.001 { span.x } else { 0.1 }).clamp(0., 1.);
                    let v =
                        ((p.z - lo.z) / if span.z > 0.001 { span.z } else { 0.1 }).clamp(0., 1.);
                    [
                        (col + 0.065 + 0.87 * u) / 5.,
                        1. - (row + 0.065 + 0.87 * v) / 4.,
                    ]
                })
                .collect();
            m.compute_normals();
            meshes.push(m);
        }};
    }
    macro_rules! bx {
        ($n:expr,$c:expr,$d:expr,$m:expr) => {
            add!($n, box_mesh($c, $d), $m)
        };
    }
    macro_rules! el {
        ($n:expr,$c:expr,$d:expr,$m:expr) => {
            el!($n, $c, $d, $m, 2)
        };
        ($n:expr,$c:expr,$d:expr,$m:expr,$s:expr) => {
            add!($n, ellipsoid($c, $d, $s), $m)
        };
    }
    macro_rules! tu {
        ($n:expr,$a:expr,$b:expr,$r:expr,$m:expr) => {
            tu!($n, $a, $b, $r, $m, 12)
        };
        ($n:expr,$a:expr,$b:expr,$r:expr,$m:expr,$s:expr) => {
            tu!($n, $a, $b, $r, $m, $s, $r)
        };
        ($n:expr,$a:expr,$b:expr,$r:expr,$m:expr,$s:expr,$end:expr) => {
            add!($n, tube($a, $b, $r, $end, $s), $m)
        };
    }
    macro_rules! bl {
        ($n:expr,$l:expr,$m:expr) => {
            bl!($n, $l, $m, 16)
        };
        ($n:expr,$l:expr,$m:expr,$s:expr) => {
            add!($n, block($l, $s), $m)
        };
    }
    macro_rules! pr {
        ($n:expr,$l:expr,$y:expr,$t:expr,$m:expr) => {
            add!($n, prism($l, $y, $t), $m)
        };
    }
    macro_rules! be {
        ($n:expr,$a:expr,$b:expr,$w:expr,$d:expr,$m:expr) => {
            add!($n, beam($a, $b, $w, $d), $m)
        };
    }
    macro_rules! di {
        ($n:expr,$c:expr,$normal:expr,$r:expr,$m:expr) => {
            di!($n, $c, $normal, $r, $m, 12)
        };
        ($n:expr,$c:expr,$normal:expr,$r:expr,$m:expr,$s:expr) => {{
            let c = Vec3::from($c);
            let d = Vec3::from($normal).normalize() * 0.0025;
            tu!($n, (c - d).to_array(), (c + d).to_array(), $r, $m, $s);
        }};
    }
    for (side, x0, y0, kx, ky, hx, hy, yfoot, xx) in [
        (
            "L", -0.235, -0.16, -0.19, -0.095, -0.145, 0.045, -0.22, -0.275,
        ),
        ("R", 0.225, 0.135, 0.225, 0.08, 0.155, 0.06, 0.10, 0.285),
    ] {
        bl!(
            format!("{side}_trouser_thigh"),
            &[
                [0.52, kx, ky, 0.101, 0.104],
                [0.60, kx * 0.98, ky, 0.119, 0.105],
                [0.71, (kx + hx) * 0.5, (ky + hy) * 0.5, 0.119, 0.114],
                [0.82, hx, hy, 0.112, 0.113],
                [0.98, hx, hy, 0.108, 0.101]
            ],
            CLOTH
        );
        el!(
            format!("{side}_knee_cloth"),
            [kx, ky, 0.52],
            [0.10, 0.10, 0.087],
            CLOTH
        );
        bl!(
            format!("{side}_trouser_shin"),
            &[
                [0.16, x0, y0, 0.081, 0.086],
                [0.23, x0, y0, 0.09, 0.094],
                [0.37, (x0 + kx) / 2., (y0 + ky) / 2., 0.102, 0.103],
                [0.47, kx, ky, 0.098, 0.105],
                [0.51, kx, ky, 0.106, 0.105]
            ],
            CLOTH
        );
        for (i, z) in [0.275, 0.355, 0.46, 0.57, 0.68, 0.82]
            .into_iter()
            .enumerate()
        {
            let xc = if z < 0.5 {
                (x0 + kx) / 2.
            } else {
                (kx + hx) / 2.
            };
            let yc = if z < 0.5 {
                (y0 + ky) / 2.
            } else {
                (ky + hy) / 2.
            };
            el!(
                format!("{side}_trouser_fold_{i}"),
                [xc - 0.005, yc - 0.058, z],
                [0.068, 0.019, 0.008],
                if i % 2 == 1 { LIGHT } else { SHADOW },
                1
            );
        }
        bx!(
            format!("{side}_cargo_bellows"),
            [xx, (ky + hy) / 2., 0.735],
            [0.035, 0.143, 0.175],
            SHADOW
        );
        bx!(
            format!("{side}_cargo_flap"),
            [xx * 1.055, (ky + hy) / 2., 0.826],
            [0.027, 0.150, 0.031],
            CLOTH
        );
        di!(
            format!("{side}_cargo_snap"),
            [xx * 1.14, (ky + hy) / 2., 0.836],
            [1., 0., 0.],
            0.012,
            METAL
        );
        tu!(
            format!("{side}_boot_cuff"),
            [x0, y0, 0.105],
            [x0, y0, 0.29],
            0.079,
            BOOT,
            14
        );
        el!(
            format!("{side}_boot_toe"),
            [x0, yfoot - 0.08, 0.073],
            [0.080, 0.140, 0.061],
            BOOT
        );
        bx!(
            format!("{side}_sole"),
            [x0, yfoot - 0.092, 0.033],
            [0.162, 0.284, 0.033],
            LEATHER
        );
        bx!(
            format!("{side}_boot_tongue"),
            [x0, yfoot - 0.064, 0.15],
            [0.075, 0.021, 0.18],
            LEATHER
        );
        for l in 0..6 {
            let z = 0.115 + l as f32 * 0.028;
            for sg in [-1., 1.] {
                tu!(
                    format!("{side}_lace_{l}_{sg}"),
                    [x0 + sg * 0.038, yfoot - 0.098, z],
                    [x0 - sg * 0.012, yfoot - 0.102, z + 0.020],
                    0.0027,
                    WEBLIGHT,
                    6
                );
            }
        }
        for n in 0..4 {
            bx!(
                format!("{side}_sole_tread_{n}"),
                [x0, yfoot - 0.215 + n as f32 * 0.075, 0.013],
                [0.167, 0.018, 0.016],
                BOOT
            );
        }
    }
    bl!(
        "jacket_tail",
        &[
            [0.94, 0., 0.046, 0.24, 0.152],
            [1.00, 0., 0.035, 0.274, 0.158],
            [1.08, 0., 0.01, 0.288, 0.169],
            [1.15, 0., 0.02, 0.278, 0.175]
        ],
        SHADOW,
        24
    );
    bl!(
        "jacket_body",
        &[
            [1.03, 0., 0.024, 0.267, 0.174],
            [1.16, 0., 0.023, 0.285, 0.189],
            [1.31, 0., 0.025, 0.300, 0.207],
            [1.46, 0., 0.028, 0.302, 0.191],
            [1.565, 0., 0.036, 0.245, 0.150]
        ],
        CLOTH,
        28
    );
    for (side, s) in [("L", -1.), ("R", 1.)] {
        pr!(
            format!("{side}_jacket_lapel"),
            &[
                [s * 0.045, 1.565],
                [s * 0.19, 1.555],
                [s * 0.075, 1.43],
                [s * 0.015, 1.472]
            ],
            -0.158,
            0.026,
            LIGHT
        );
        bx!(
            format!("{side}_shirt_chest_pocket"),
            [s * 0.162, -0.202, 1.40],
            [0.15, 0.025, 0.132],
            CLOTH
        );
        bx!(
            format!("{side}_chest_pocket_flap"),
            [s * 0.162, -0.225, 1.469],
            [0.16, 0.027, 0.035],
            SHADOW
        );
        di!(
            format!("{side}_pocket_button"),
            [s * 0.16, -0.241, 1.456],
            [0., -1., 0.],
            0.009,
            STEEL
        );
        tu!(
            format!("{side}_jacket_vertical_seam"),
            [s * 0.026, -0.186, 1.16],
            [s * 0.026, -0.168, 1.42],
            0.004,
            SHADOW,
            6
        );
    }
    bx!(
        "button_placket",
        [-0.005, -0.204, 1.285],
        [0.022, 0.016, 0.31],
        SHADOW
    );
    for (k, z) in [1.18, 1.245, 1.315, 1.38, 1.45].into_iter().enumerate() {
        di!(
            format!("jacket_button_{k}"),
            [0.005, -0.220, z],
            [0., -1., 0.],
            0.009,
            STEEL,
            9
        );
    }
    for (side, s) in [("L", -1.), ("R", 1.)] {
        let shoulder = [s * 0.29, 0., 1.505];
        let elbow = if s < 0. {
            [s * 0.39, -0.17, 1.286]
        } else {
            [s * 0.38, -0.15, 1.32]
        };
        let hand = if s < 0. {
            [s * 0.16, -0.475, 1.24]
        } else {
            [s * 0.21, -0.45, 1.235]
        };
        el!(
            format!("{side}_shoulder_fold"),
            shoulder,
            [0.095, 0.109, 0.100],
            CLOTH
        );
        tu!(
            format!("{side}_upper_sleeve"),
            shoulder,
            elbow,
            0.094,
            CLOTH,
            16,
            0.082
        );
        el!(
            format!("{side}_elbow_fatigue"),
            elbow,
            [0.079, 0.080, 0.071],
            SHADOW
        );
        tu!(
            format!("{side}_lower_sleeve"),
            elbow,
            hand,
            0.072,
            CLOTH,
            16,
            0.060
        );
        for (i, t) in [0.25, 0.5, 0.7].into_iter().enumerate() {
            let p = Vec3::from(elbow).lerp(Vec3::from(hand), t).to_array();
            el!(
                format!("{side}_sleeve_fold_{i}"),
                p,
                [0.052, 0.039, 0.016],
                if i % 2 == 1 { LIGHT } else { SHADOW },
                1
            );
        }
        tu!(
            format!("{side}_wrist_cuff"),
            [hand[0], hand[1] + 0.036, hand[2]],
            [hand[0], hand[1] - 0.028, hand[2]],
            0.067,
            SHADOW,
            12
        );
        el!(
            format!("{side}_gloved_hand"),
            hand,
            [0.074, 0.069, 0.045],
            GLOVE
        );
        for j in 0..4 {
            let x = hand[0] + (j as f32 - 1.5) * 0.023;
            tu!(
                format!("{side}_glove_finger_{j}"),
                [x, hand[1] - 0.006, hand[2] - 0.013],
                [x, hand[1] - 0.072, hand[2] - 0.027],
                0.011,
                GLOVE,
                8
            );
        }
        let x = s * 0.395;
        pr!(
            format!("{side}_insignia_shield"),
            &[
                [x - s * 0.043, 1.46],
                [x + s * 0.043, 1.46],
                [x + s * 0.038, 1.392],
                [x, 1.365],
                [x - s * 0.038, 1.392]
            ],
            -0.108,
            0.011,
            PATCH
        );
        di!(
            format!("{side}_insignia_ring"),
            [x, -0.119, 1.419],
            [0., -1., 0.],
            0.022,
            PATCHLINE,
            12
        );
        for q in [-1., 1.] {
            tu!(
                format!("{side}_insignia_spoke_{q}"),
                [x + q * 0.017, -0.124, 1.398],
                [x - q * 0.017, -0.124, 1.437],
                0.0027,
                PATCHLINE,
                6
            );
        }
    }
    bx!("web_belt", [0., -0.086, 1.07], [0.525, 0.178, 0.07], WEB);
    bx!(
        "steel_belt_buckle",
        [0., -0.190, 1.07],
        [0.064, 0.018, 0.05],
        STEEL
    );
    bx!(
        "belt_center",
        [0., -0.201, 1.07],
        [0.035, 0.014, 0.026],
        METAL
    );
    for (side, s) in [("L", -1.), ("R", 1.)] {
        be!(
            format!("{side}_field_harness"),
            [s * 0.205, -0.14, 1.57],
            [s * 0.096, -0.218, 1.11],
            0.052,
            0.034,
            WEB
        );
        bx!(
            format!("{side}_harness_clasp"),
            [s * 0.148, -0.216, 1.34],
            [0.055, 0.02, 0.04],
            STEEL
        );
        for (j, z) in [1.105, 1.185].into_iter().enumerate() {
            let x = s * (0.22 + 0.05 * j as f32);
            bx!(
                format!("{side}_belt_utility_{j}"),
                [x, -0.185, z],
                [0.103, 0.098, 0.116],
                WEBLIGHT
            );
            bx!(
                format!("{side}_belt_flap_{j}"),
                [x, -0.238, z + 0.053],
                [0.108, 0.014, 0.024],
                WEB
            );
        }
    }
    for (i, x) in [-0.173, -0.058, 0.058, 0.173].into_iter().enumerate() {
        bx!(
            format!("magazine_case_{i}"),
            [x, -0.244, 1.205],
            [0.102, 0.066, 0.147],
            WEBLIGHT
        );
        bx!(
            format!("magazine_case_flap_{i}"),
            [x, -0.283, 1.269],
            [0.103, 0.021, 0.036],
            WEB
        );
        bx!(
            format!("magazine_case_tab_{i}"),
            [x, -0.299, 1.243],
            [0.023, 0.010, 0.040],
            LEATHER
        );
        di!(
            format!("magazine_case_rivet_{i}"),
            [x, -0.310, 1.223],
            [0., -1., 0.],
            0.006,
            METAL,
            8
        );
    }
    el!(
        "neck",
        [0., -0.008, 1.600],
        [0.074, 0.079, 0.111],
        SKINSHADOW
    );
    el!(
        "face_main",
        [0., -0.029, 1.716],
        [0.111, 0.097, 0.142],
        SKIN,
        3
    );
    el!("jaw", [0., -0.059, 1.661], [0.089, 0.075, 0.068], SKIN);
    el!(
        "chin",
        [0., -0.091, 1.624],
        [0.062, 0.042, 0.026],
        SKINSHADOW
    );
    for s in [-1., 1.] {
        el!(
            format!("ear_{s}"),
            [s * 0.109, -0.020, 1.717],
            [0.022, 0.036, 0.047],
            SKIN
        );
        el!(
            format!("cheekbone_{s}"),
            [s * 0.07, -0.101, 1.704],
            [0.035, 0.008, 0.016],
            SKIN
        );
        el!(
            format!("brow_{s}"),
            [s * 0.054, -0.120, 1.766],
            [0.045, 0.008, 0.008],
            SKINSHADOW
        );
        el!(
            format!("eye_{s}"),
            [s * 0.052, -0.119, 1.744],
            [0.010, 0.004, 0.006],
            METAL
        );
        el!(
            format!("ear_canal_{s}"),
            [s * 0.123, -0.044, 1.725],
            [0.007, 0.006, 0.015],
            SKINSHADOW,
            1
        );
        el!(
            format!("nose_wing_{s}"),
            [s * 0.018, -0.116, 1.692],
            [0.011, 0.007, 0.009],
            SKIN,
            1
        );
    }
    el!(
        "nose_bridge",
        [0., -0.112, 1.717],
        [0.018, 0.020, 0.047],
        SKIN
    );
    el!(
        "lip",
        [0., -0.111, 1.653],
        [0.026, 0.004, 0.003],
        SKINSHADOW
    );
    bl!(
        "helmet_shell",
        &[
            [1.775, 0., -0.006, 0.146, 0.136],
            [1.793, 0., 0.004, 0.161, 0.152],
            [1.827, 0., 0.008, 0.159, 0.151],
            [1.864, 0., 0.012, 0.144, 0.135],
            [1.900, 0., 0.011, 0.102, 0.102],
            [1.919, 0., 0.010, 0.036, 0.036]
        ],
        HELMET,
        32
    );
    bl!(
        "helmet_flared_skirt",
        &[
            [1.765, 0., 0.013, 0.176, 0.165],
            [1.779, 0., 0.008, 0.173, 0.162],
            [1.800, 0., 0.012, 0.159, 0.147]
        ],
        EDGE,
        32
    );
    for j in 0..12 {
        let t = std::f32::consts::TAU * j as f32 / 12.;
        tu!(
            format!("helmet_stitched_seam_{j}"),
            [0.158 * t.cos(), 0.151 * t.sin() + 0.012, 1.819],
            [0.104 * t.cos(), 0.101 * t.sin() + 0.012, 1.895],
            0.0029,
            SHADOW,
            7
        );
    }
    for j in 0..20 {
        let t = std::f32::consts::TAU * j as f32 / 20.;
        let u = std::f32::consts::TAU * (j + 1) as f32 / 20.;
        tu!(
            format!("helmet_olive_band_{j}"),
            [0.155 * t.cos(), 0.149 * t.sin() + 0.006, 1.816],
            [0.155 * u.cos(), 0.149 * u.sin() + 0.006, 1.816],
            0.010,
            WEB,
            8
        );
    }
    for s in [-1., 1.] {
        let x = s * 0.079;
        el!(
            format!("goggle_frame_{s}"),
            [x, -0.148, 1.864],
            [0.069, 0.026, 0.028],
            LEATHER
        );
        el!(
            format!("goggle_lens_{s}"),
            [x, -0.169, 1.867],
            [0.058, 0.011, 0.021],
            GLASS,
            3
        );
        el!(
            format!("goggle_glint_{s}"),
            [x - 0.019, -0.177, 1.878],
            [0.021, 0.002, 0.002],
            STEEL,
            1
        );
        for j in 0..3 {
            let f = j as f32;
            be!(
                format!("goggle_strap_{s}_{j}"),
                [s * (0.145 + f * 0.002), -0.127 + f * 0.045, 1.863],
                [
                    s * (0.146 + (f + 1.) * 0.002),
                    -0.127 + (f + 1.) * 0.045,
                    1.863
                ],
                0.015,
                0.023,
                LEATHER
            );
        }
        tu!(
            format!("helmet_chinstrap_{s}"),
            [s * 0.145, -0.014, 1.788],
            [s * 0.028, -0.112, 1.630],
            0.008,
            LEATHER,
            8
        );
    }
    be!(
        "goggle_center_bridge",
        [-0.022, -0.181, 1.873],
        [0.022, -0.181, 1.873],
        0.013,
        0.024,
        LEATHER
    );
    bx!(
        "chinstrap_buckle",
        [0.047, -0.109, 1.666],
        [0.025, 0.009, 0.020],
        STEEL
    );
    bl!(
        "field_pack",
        &[
            [1.21, 0., 0.235, 0.194, 0.143],
            [1.29, 0., 0.252, 0.214, 0.147],
            [1.45, 0., 0.252, 0.218, 0.144],
            [1.61, 0., 0.25, 0.207, 0.135]
        ],
        PACK,
        22
    );
    bx!(
        "pack_large_outer_pocket",
        [0., 0.411, 1.324],
        [0.236, 0.070, 0.183],
        PACKSHADOW
    );
    bx!(
        "pack_upper_flap",
        [0., 0.395, 1.528],
        [0.326, 0.082, 0.079],
        PACKSHADOW
    );
    for s in [-1., 1.] {
        be!(
            format!("pack_front_shoulder_webbing_{s}"),
            [s * 0.187, 0.08, 1.54],
            [s * 0.166, -0.169, 1.09],
            0.060,
            0.027,
            WEBLIGHT
        );
        be!(
            format!("pack_lashing_{s}"),
            [s * 0.115, 0.411, 1.565],
            [s * 0.115, 0.448, 1.27],
            0.031,
            0.023,
            WEBLIGHT
        );
        bx!(
            format!("pack_side_pocket_{s}"),
            [s * 0.229, 0.275, 1.3],
            [0.076, 0.145, 0.182],
            PACKSHADOW
        );
        bx!(
            format!("pack_side_flap_{s}"),
            [s * 0.240, 0.283, 1.4],
            [0.08, 0.144, 0.042],
            WEB
        );
        bx!(
            format!("pack_clasp_{s}"),
            [s * 0.115, 0.451, 1.427],
            [0.036, 0.017, 0.024],
            STEEL
        );
        tu!(
            format!("bedroll_belt_{s}"),
            [s * 0.13, 0.320, 1.54],
            [s * 0.13, 0.322, 1.691],
            0.011,
            WEBLIGHT,
            10
        );
    }
    tu!(
        "pack_bedroll",
        [-0.19, 0.319, 1.616],
        [0.19, 0.319, 1.616],
        0.064,
        PACKSHADOW,
        16
    );
    el!(
        "canteen",
        [0.269, 0.055, 1.102],
        [0.056, 0.074, 0.096],
        PACK
    );
    bx!(
        "canteen_cap",
        [0.270, 0.056, 1.193],
        [0.038, 0.037, 0.027],
        METAL
    );
    bx!(
        "radio_satchel",
        [-0.283, 0.143, 1.22],
        [0.105, 0.093, 0.173],
        WEB
    );
    tu!(
        "radio_antenna",
        [-0.305, 0.173, 1.29],
        [-0.338, 0.188, 1.67],
        0.0045,
        METAL,
        8
    );
    bx!(
        "grenade_spoon",
        [0.276, -0.151, 1.179],
        [0.016, 0.038, 0.043],
        METAL
    );
    el!(
        "grenade_body",
        [0.271, -0.138, 1.153],
        [0.031, 0.030, 0.046],
        METAL
    );
    tu!(
        "grenade_ring",
        [0.287, -0.133, 1.204],
        [0.304, -0.123, 1.21],
        0.008,
        STEEL,
        8
    );
    // Static art prop only, not a mechanism or engineering model.
    frame = Mat4::from_rotation_translation(
        Quat::from_rotation_y(0.085),
        Vec3::new(0.015, -0.452, 1.304),
    );
    pr!(
        "rifle_fixed_stock",
        &[
            [-0.59, 0.013],
            [-0.51, 0.070],
            [-0.275, 0.045],
            [-0.274, -0.061],
            [-0.39, -0.054],
            [-0.57, -0.116],
            [-0.60, -0.109]
        ],
        0.,
        0.055,
        STOCK
    );
    for (name, c, d, mat) in [
        (
            "rifle_buttplate",
            [-0.59, 0., -0.049],
            [0.021, 0.067, 0.148],
            METAL,
        ),
        (
            "rifle_receiver",
            [-0.169, 0., 0.],
            [0.245, 0.091, 0.095],
            METAL,
        ),
        (
            "receiver_upper",
            [-0.184, 0., 0.070],
            [0.234, 0.083, 0.033],
            STEEL,
        ),
        (
            "dust_cover",
            [-0.098, -0.050, 0.009],
            [0.104, 0.011, 0.034],
            STEEL,
        ),
        (
            "receiver_pin",
            [-0.246, -0.050, -0.009],
            [0.018, 0.010, 0.020],
            STEEL,
        ),
        (
            "charging_handle",
            [-0.212, 0.049, 0.082],
            [0.055, 0.033, 0.012],
            METAL,
        ),
        (
            "magazine_well",
            [-0.047, 0., -0.061],
            [0.081, 0.080, 0.054],
            METAL,
        ),
    ] {
        bx!(name, c, d, mat);
    }
    for (n, (x, z)) in [
        (-0.057, -0.10),
        (-0.050, -0.14),
        (-0.043, -0.18),
        (-0.023, -0.22),
    ]
    .into_iter()
    .enumerate()
    {
        bx!(
            format!("rifle_curved_magazine_segment_{n}"),
            [x, 0., z],
            [0.073, 0.066, 0.052],
            if n == 0 { STEEL } else { METAL }
        );
        for s in [-1., 1.] {
            bx!(
                format!("rifle_magazine_rib_{n}_{s}"),
                [x + s * 0.026, -0.035, z],
                [0.005, 0.006, 0.041],
                STEEL
            );
        }
    }
    be!(
        "rifle_pistol_grip",
        [-0.226, 0., -0.05],
        [-0.292, 0., -0.182],
        0.051,
        0.061,
        LEATHER
    );
    for n in 0..6 {
        let n = n as f32;
        tu!(
            format!("grip_checkering_{n}"),
            [-0.260, -0.034, -0.094 - n * 0.011],
            [-0.286, -0.034, -0.100 - n * 0.010],
            0.0016,
            STEEL,
            6
        );
    }
    for (i, (a, b)) in [
        ([-0.218, -0.051, -0.047], [-0.214, -0.051, -0.100]),
        ([-0.214, -0.051, -0.100], [-0.126, -0.051, -0.100]),
        ([-0.126, -0.051, -0.100], [-0.105, -0.051, -0.051]),
    ]
    .into_iter()
    .enumerate()
    {
        tu!(format!("trigger_guard_{i}"), a, b, 0.006, METAL, 8);
    }
    pr!(
        "rifle_fore_end",
        &[
            [-0.04, 0.042],
            [0.413, 0.046],
            [0.425, -0.041],
            [-0.04, -0.043]
        ],
        0.,
        0.090,
        STOCK
    );
    bx!(
        "rifle_upper_handguard",
        [0.20, 0., 0.048],
        [0.456, 0.094, 0.019],
        METAL
    );
    for s in [-1., 1.] {
        for i in 0..8 {
            el!(
                format!("handguard_slot_{s}_{i}"),
                [0.013 + i as f32 * 0.054, s * 0.048, 0.006],
                [0.016, 0.004, 0.009],
                METAL,
                1
            );
        }
    }
    for i in 0..4 {
        let x = 0.01 + i as f32 * 0.118;
        be!(
            format!("handguard_lower_reinforcement_{i}"),
            [x, -0.050, -0.035],
            [x, 0.050, -0.035],
            0.010,
            0.010,
            STEEL
        );
    }
    for (name, a, b, r, mat) in [
        (
            "rifle_barrel",
            [0.42, 0., 0.007],
            [0.76, 0., 0.007],
            0.014,
            METAL,
        ),
        (
            "rifle_gas_tube",
            [0.34, 0., 0.046],
            [0.59, 0., 0.046],
            0.011,
            METAL,
        ),
        (
            "rifle_front_barrel_ring",
            [0.41, 0., 0.007],
            [0.446, 0., 0.007],
            0.023,
            STEEL,
        ),
        (
            "rifle_muzzle_brake",
            [0.751, 0., 0.007],
            [0.815, 0., 0.007],
            0.019,
            METAL,
        ),
    ] {
        tu!(name, a, b, r, mat, 14);
    }
    bx!(
        "rifle_front_sight_base",
        [0.589, 0., 0.053],
        [0.028, 0.036, 0.068],
        METAL
    );
    bx!(
        "rifle_front_sight_tip",
        [0.589, 0., 0.096],
        [0.008, 0.009, 0.022],
        STEEL
    );
    bx!(
        "rifle_rear_sight",
        [-0.219, 0., 0.104],
        [0.027, 0.036, 0.024],
        METAL
    );
    for s in [-1., 1.] {
        bx!(
            format!("muzzle_ports_{s}"),
            [0.789, s * 0.018, 0.007],
            [0.012, 0.005, 0.008],
            STEEL
        );
    }
    meshes
}
