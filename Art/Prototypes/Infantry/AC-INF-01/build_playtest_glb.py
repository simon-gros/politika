#!/usr/bin/env python3
"""Offline-only authoring conversion: split-object review GLB -> one textured static playtest mesh.
Requires trimesh (no runtime Unreal/Python dependency).
"""
from pathlib import Path
import argparse
import hashlib
import trimesh

HERE = Path(__file__).resolve().parent
parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument("--input", type=Path, default=HERE / "Generated" / "AC-INF-01_Professional_Rifle_Section_v2.glb")
parser.add_argument("--output", type=Path, default=HERE / "Generated" / "SM_AC_INF_01_Playtest.glb")
args = parser.parse_args()
scene = trimesh.load(str(args.input), force="scene")
mesh = scene.to_geometry()   # apply transforms and combine authored components
if not mesh.visual.kind == "texture":
    raise SystemExit("Refusing export without original color UV/material")
if len(mesh.faces) != 22544:
    raise SystemExit(f"Unexpected triangle count {len(mesh.faces)}; investigate source changes")
out_scene = trimesh.Scene()
out_scene.add_geometry(mesh, geom_name="SM_AC_INF_01_Playtest", node_name="SM_AC_INF_01_Playtest")
args.output.parent.mkdir(parents=True, exist_ok=True)
args.output.write_bytes(trimesh.exchange.gltf.export_glb(out_scene))
check = trimesh.load(args.output, force="scene")
assert len(check.geometry) == 1
assert sum(len(g.faces) for g in check.geometry.values()) == 22544
assert next(iter(check.geometry.values())).visual.kind == "texture"
print("Wrote:", args.output, "bytes:", args.output.stat().st_size)
print("SHA-256:", hashlib.sha256(args.output.read_bytes()).hexdigest())
