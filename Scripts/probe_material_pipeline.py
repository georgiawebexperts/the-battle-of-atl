"""Why do materials created by these editor scripts render as the default?

Measured on 2026-09-19: a material asset created by these scripts is not drawn
in game at all. The experiment that proved it (Scripts/debug_spirit_material_red.py)
rebuilt the spirit's material as unlit emissive pure red and the mesh still
rendered grey, while the same mesh, component and slot drew a dark body from an
engine material.

This narrows it down. It reads back, from the saved asset:

  1. what is connected to each material property the renderer reads, so "the
     graph is drawn in the editor but the connection never survives the save"
     can be told apart from "the connection is there and the renderer is
     substituting anyway";
  2. the material's usage flags and the two API calls that would explain a
     missing shader map;
  3. the same for the project's other script-created material, the skatepark
     concrete, because if that one is also falling back then the default grey
     the park has been wearing is not a colour choice.

    UnrealEditor-Cmd <project>.uproject -ExecutePythonScript=<this file>

Read-only. Writes work/material-pipeline.json and prints it.
"""
import json
from pathlib import Path

import unreal

root = Path(unreal.Paths.project_dir()).resolve()
lib = unreal.MaterialEditingLibrary

TARGETS = [
    ("spirit_body_authored", "/Game/BattleForTheA/Spirit/M_SpectralBearBody"),
    ("skatepark_concrete", "/Game/BattleForTheA/Skatepark/M_SkateConcreteDetailed"),
    ("engine_basicshape", "/Engine/BasicShapes/BasicShapeMaterial"),
]
PROPERTIES = (
    "MP_EMISSIVE_COLOR",
    "MP_BASE_COLOR",
    "MP_OPACITY",
    "MP_OPACITY_MASK",
    "MP_METALLIC",
    "MP_ROUGHNESS",
    "MP_NORMAL",
)

report = {}
for label, path in TARGETS:
    entry = {"path": path}
    mat = unreal.load_asset(path)
    if not mat:
        entry["loaded"] = False
        report[label] = entry
        continue
    entry["loaded"] = True
    entry["class"] = mat.get_class().get_name()
    for prop in ("shading_model", "blend_mode", "two_sided", "is_material_instance",
                 "b_use_material_attributes", "material_domain", "blendable_location"):
        try:
            entry[prop] = str(mat.get_editor_property(prop))
        except Exception as error:  # noqa: BLE001
            entry[prop] = "ERR " + str(error)
    entry["connected"] = {}
    for prop in PROPERTIES:
        try:
            inputs = lib.get_inputs_for_material_property(mat, getattr(unreal.MaterialProperty, prop))
            entry["connected"][prop] = [e.get_class().get_name() for e in inputs]
        except Exception as error:  # noqa: BLE001
            entry["connected"][prop] = "ERR " + str(error)
    entry["expressions"] = [e.get_class().get_name() for e in lib.get_material_expressions(mat)]
    # The two calls that would explain a material with a graph and no shader.
    try:
        entry["recompile_ok"] = bool(lib.recompile_material(mat))
    except Exception as error:  # noqa: BLE001
        entry["recompile_ok"] = "ERR " + str(error)
    for flag in ("b_used_with_static_meshes", "b_used_with_instanced_static_meshes",
                 "b_used_with_niagara_sprites", "b_used_with_niagara_mesh_particles",
                 "b_used_with_skeletal_mesh", "b_used_with_landscape", "b_used_with_nanite"):
        try:
            entry[flag] = bool(mat.get_editor_property(flag))
        except Exception:  # noqa: BLE001 - older engine builds do not expose all of these
            pass
    report[label] = entry

(root / "work/material-pipeline.json").write_text(json.dumps(report, indent=2) + "\n")
print("MATERIALPIPELINE " + json.dumps(report))
unreal.SystemLibrary.quit_editor()
