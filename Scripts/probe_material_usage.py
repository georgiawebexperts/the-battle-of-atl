"""Do the script-created materials have any usage flags, and can they be set?

The one explanation left standing for "materials this project's editor scripts
create are never drawn" is usage flags: a UMaterial that is not marked as used
with static meshes is not compiled for them, and the renderer substitutes the
default material. Engine materials carry those flags because everything uses
them; a material the factory created from nothing and a script filled in has
nobody to set them.

This reports the flags on two materials this project authored by script - the
spirit's body material and the painted card's - against an engine one, then sets
the static-mesh usage on both authored materials and re-saves them, so the next
render says whether that was the whole problem.

    UnrealEditor-Cmd <project>.uproject -ExecutePythonScript=<this file>

Not read-only: it re-saves the two project materials with the flag added.
"""
import json
from pathlib import Path

import unreal

root = Path(unreal.Paths.project_dir()).resolve()
lib = unreal.MaterialEditingLibrary

report = {
    "api_with_usage_in_name": [n for n in dir(lib) if "usage" in n.lower()],
    "api_with_compile_in_name": [n for n in dir(lib) if "compile" in n.lower()],
    "material_usage_enum": [n for n in dir(unreal) if "MaterialUsage" in n or n.startswith("MATUSAGE")],
}

TARGETS = [
    ("spirit_body_authored", "/Game/BattleForTheA/Spirit/M_SpectralBearBody"),
    ("spirit_card_painted", "/Game/BattleForTheA/Spirit/M_SpectralBlackBear"),
    ("engine_basicshape", "/Engine/BasicShapes/BasicShapeMaterial"),
]

for label, path in TARGETS:
    entry = {}
    mat = unreal.load_asset(path)
    if not mat:
        report[label] = {"loaded": False}
        continue
    entry["loaded"] = True
    # Writable flags live on UMaterial as bUsedWith... properties. Not every
    # engine build exposes them to Python, and a missing one is a finding, not
    # an error, so each is read and written defensively.
    for flag in ("b_used_with_static_meshes", "b_used_with_instanced_static_meshes",
                 "b_used_with_skeletal_mesh", "b_used_with_particles",
                 "b_used_with_nanite", "b_used_with_landscape"):
        try:
            before = bool(mat.get_editor_property(flag))
        except Exception as error:  # noqa: BLE001
            entry[flag] = "ERR " + str(error)
            continue
        entry[flag] = before
        if not before and path.startswith("/Game/"):
            try:
                mat.set_editor_property(flag, True)
                entry[flag + "_set"] = True
            except Exception as error:  # noqa: BLE001
                entry[flag + "_set"] = "ERR " + str(error)
    if path.startswith("/Game/"):
        try:
            lib.recompile_material(mat)
            entry["saved"] = bool(unreal.EditorAssetLibrary.save_loaded_asset(mat, False))
        except Exception as error:  # noqa: BLE001
            entry["saved"] = "ERR " + str(error)
    report[label] = entry

(root / "work/material-usage.json").write_text(json.dumps(report, indent=2) + "\n")
print("MATERIALUSAGE " + json.dumps(report))
unreal.SystemLibrary.quit_editor()
