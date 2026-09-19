"""Read back what M_SpectralBearBody actually is, and say it out loud.

Writing a material graph in Python and looking at the picture is not enough:
the first pass drew the lines and the bear still rendered as default grey.
"""
import json
import unreal

dest = "/Game/BattleForTheA/Spirit"
mat = unreal.load_asset(dest + "/M_SpectralBearBody")
assert mat, "material missing"
lib = unreal.MaterialEditingLibrary

report = {}
for name in ("shading_model", "blend_mode", "two_sided", "b_use_material_attributes"):
    try:
        report[name] = str(mat.get_editor_property(name))
    except Exception as error:  # noqa: BLE001 - report, do not mask
        report[name] = "ERR " + str(error)
for prop in ("MP_EMISSIVE_COLOR", "MP_BASE_COLOR", "MP_OPACITY", "MP_OPACITY_MASK", "MP_METALLIC", "MP_ROUGHNESS"):
    enum = getattr(unreal.MaterialProperty, prop)
    try:
        report[prop] = [e.get_class().get_name() for e in lib.get_inputs_for_material_property(mat, enum)]
    except Exception as error:  # noqa: BLE001
        report[prop] = "ERR " + str(error)
report["expressions"] = [e.get_class().get_name() for e in lib.get_material_expressions(mat)]
report["scalar_params"] = [str(n) for n in lib.get_scalar_parameter_names(mat)]

mesh = unreal.load_asset(dest + "/SM_SpectralBear")
slots = mesh.get_editor_property("static_materials") if mesh else []
report["mesh_slots"] = [(str(s.material_slot_name), str(s.material_interface.get_name()) if s.material_interface else None)
                        for s in slots]
try:
    report["mesh_sections"] = [str(s.material_index) for s in mesh.get_editor_property("sections")]
except Exception as error:  # noqa: BLE001
    report["mesh_sections"] = "ERR " + str(error)

print("PROBE " + json.dumps(report, indent=1))
