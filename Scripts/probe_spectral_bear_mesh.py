"""What is actually in SM_SpectralBear: how many parts, and which material each draws.

The bear renders grey whatever material it is given, including a material
authored as pure red emissive, and the FBX import warns about a mesh named
'Plane'. A static mesh whose sections do not all match the material slot the
import script assigned falls back to the engine default material - grey - for
the sections that do not match, which would look exactly like this.

    UnrealEditor-Cmd <project>.uproject -ExecutePythonScript=<this file>

Read-only. Writes work/spectral-bear-mesh.json.
"""
import json
from pathlib import Path

import unreal

root = Path(unreal.Paths.project_dir()).resolve()
mesh = unreal.load_asset("/Game/BattleForTheA/Spirit/SM_SpectralBear")
assert mesh, "mesh missing"

report = {"mesh": mesh.get_path_name()}
slots = mesh.get_editor_property("static_materials") or []
report["material_slots"] = [
    {"slot_name": str(s.material_slot_name),
     "material": str(s.material_interface.get_name()) if s.material_interface else None}
    for s in slots
]
try:
    report["sections"] = [
        {"material_index": int(s.material_index),
         "num_triangles": int(s.num_triangles),
         "cast_shadow": bool(s.cast_shadow)}
        for s in mesh.get_editor_property("sections")
    ]
except Exception as error:  # noqa: BLE001
    report["sections"] = "ERR " + str(error)

bounds = mesh.get_bounds()
report["asset_bounds"] = {
    "origin": [bounds.origin.x, bounds.origin.y, bounds.origin.z],
    "box_extent": [bounds.box_extent.x, bounds.box_extent.y, bounds.box_extent.z],
}
report["triangles"] = sum(s.get("num_triangles", 0) for s in report["sections"]) \
    if isinstance(report["sections"], list) else None

(root / "work/spectral-bear-mesh.json").write_text(json.dumps(report, indent=2) + "\n")
print("SPECTRALMESH " + json.dumps(report))
unreal.SystemLibrary.quit_editor()
