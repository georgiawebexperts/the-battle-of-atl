"""Give the script-authored materials the usage flags they never had.

The last explanation standing for "a material created by these editor scripts is
never drawn in game" is usage flags: a UMaterial that is not marked as used with
static meshes is not compiled for them, and the renderer substitutes its default
material. The flags are not exposed as bUsedWith... properties in this engine's
Python API - reading them by name fails - but the API that owns them is there:
MaterialEditingLibrary.has_material_usage / set_material_usage, with the
MaterialUsage enum.

This reads and sets them on the two materials this project authored by script and
re-saves. The painted card is the test subject, because the card is assigned its
material directly and nothing overrides it at runtime, so the next
`-BattleSpiritCard` render says plainly whether the material draws now.

    UnrealEditor-Cmd <project>.uproject -ExecutePythonScript=<this file>

Not read-only: it re-saves the materials with the flags added.
"""
import json
from pathlib import Path

import unreal

root = Path(unreal.Paths.project_dir()).resolve()
lib = unreal.MaterialEditingLibrary

usage_members = [n for n in dir(unreal.MaterialUsage) if n.startswith("MATUSAGE")]
report = {"material_usage_members": usage_members}

TARGETS = [
    ("spirit_body_authored", "/Game/BattleForTheA/Spirit/M_SpectralBearBody"),
    ("spirit_card_painted", "/Game/BattleForTheA/Spirit/M_SpectralBlackBear"),
    ("skatepark_concrete", "/Game/BattleForTheA/Skatepark/M_SkateConcreteDetailed"),
    ("engine_basicshape", "/Engine/BasicShapes/BasicShapeMaterial"),
]

# The three a static mesh, an instanced static mesh and a shadow/landscape pass
# ask for. Static mesh is the one that matters here.
WANTED = ("MATUSAGE_STATIC_MESH", "MATUSAGE_INSTANCED_STATIC_MESHES", "MATUSAGE_LANDSCAPE")

for label, path in TARGETS:
    entry = {}
    mat = unreal.load_asset(path)
    if not mat:
        report[label] = {"loaded": False}
        continue
    entry["loaded"] = True
    entry["before"] = {}
    entry["after"] = {}
    for name in WANTED:
        if not hasattr(unreal.MaterialUsage, name):
            continue
        usage = getattr(unreal.MaterialUsage, name)
        try:
            entry["before"][name] = bool(lib.has_material_usage(mat, usage))
        except Exception as error:  # noqa: BLE001
            entry["before"][name] = "ERR " + str(error)
            continue
        if path.startswith("/Game/") and not entry["before"][name]:
            try:
                lib.set_material_usage(mat, usage, True)
                entry["after"][name] = bool(lib.has_material_usage(mat, usage))
            except Exception as error:  # noqa: BLE001
                entry["after"][name] = "ERR " + str(error)
    if path.startswith("/Game/"):
        try:
            lib.recompile_material(mat)
            entry["saved"] = bool(unreal.EditorAssetLibrary.save_loaded_asset(mat, False))
        except Exception as error:  # noqa: BLE001
            entry["saved"] = "ERR " + str(error)
    report[label] = entry

(root / "work/material-usage-flags.json").write_text(json.dumps(report, indent=2) + "\n")
print("MATERIALFLAGS " + json.dumps(report))
unreal.SystemLibrary.quit_editor()
