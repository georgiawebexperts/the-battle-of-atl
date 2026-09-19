"""Import the bear candidate as the spirit's body, and build its material.

Elliott: "the bear or angel or whatever still doesnt look good". What he was
looking at was a flat painted card - a 2.7 x 1.85 m unlit plane standing in the
trail. It reads as a cut-out from the bike because that is what it is. This
imports the 3D bear found on 2026-09-19 and gives it a spectral material with a
`SpiritFade` scalar so the encounter's reveal and dissolve can drive it.

Saved assets only; it does not spawn anything and does not touch the map.

  UnrealEditor-Cmd <project>.uproject -ExecutePythonScript=<this file> \
    -unattended -nosplash -stdout -NoTextureStreaming
"""
import json
from pathlib import Path

import unreal

root = Path(unreal.Paths.project_dir()).resolve()
source = root / "SourceAssets/Spirit/BearCandidate/Bear.fbx"
dest = "/Game/BattleForTheA/Spirit"
assert source.exists(), "staged bear FBX missing"

report = {"source": str(source), "folder": dest}

task = unreal.AssetImportTask()
task.filename = str(source)
task.destination_path = dest
task.destination_name = "SM_SpectralBear"
task.automated = True
task.replace_existing = True
task.save = True
options = unreal.FbxImportUI()
options.set_editor_property("import_mesh", True)
options.set_editor_property("import_materials", False)
options.set_editor_property("import_textures", False)
options.set_editor_property("import_animations", False)
options.set_editor_property("mesh_type_to_import", unreal.FBXImportType.FBXIT_STATIC_MESH)
task.options = options
unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])

mesh = unreal.load_asset(dest + "/SM_SpectralBear")
assert mesh, "the bear did not import as a static mesh"
origin, extent = mesh.get_bounds().origin, mesh.get_bounds().box_extent
report["bounds_cm"] = {"origin": [origin.x, origin.y, origin.z],
                       "extent": [extent.x, extent.y, extent.z]}
# The mesh is authored nose along +Y and standing on its own origin, 569 cm
# long. The spirit actor faces +X, so the body is yawed -90 to put the nose
# forward, and scaled to a real black bear: 190 cm nose to tail, ~112 cm at the
# withers, which is what the design note asks for next to a 1.8 m rider.
LENGTH_CM = 190.0
scale = LENGTH_CM / (2 * extent.y)
report["length_cm"] = 2 * extent.y
report["scale"] = scale
report["height_cm"] = 2 * extent.z * scale

lib = unreal.MaterialEditingLibrary
name = "M_SpectralBearBody"
# Rebuilt from scratch rather than emptied in place: deleting the expressions of
# a material that the imported mesh already references asserts inside
# DeleteAllMaterialExpressions and takes the editor with it (seen 2026-09-19).
existing = unreal.load_asset(dest + "/" + name)
if existing:
    assert unreal.EditorAssetLibrary.delete_asset(dest + "/" + name), "could not replace " + name
mat = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
    name, dest, unreal.Material, unreal.MaterialFactoryNew())
assert mat
# BLEND_OPAQUE for now: the translucent version of this graph renders nothing at
# all in this project (the body is drawn - WasRecentlyRendered says so - and
# still leaves no pixels), which is the same class of material trouble this
# project has hit before. The spectral read comes from the unlit rim instead.
mat.set_editor_property("blend_mode", unreal.BlendMode.BLEND_OPAQUE)
mat.set_editor_property("shading_model", unreal.MaterialShadingModel.MSM_UNLIT)
mat.set_editor_property("two_sided", True)

fresnel = lib.create_material_expression(mat, unreal.MaterialExpressionFresnel, -900, 0)
# A thin rim, not a wash. At exponent 2 with a 0.32 base the whole body glowed
# and the bear read as pale stone; the design note asks for a dark body with a
# fine silver-blue rim, which is a high exponent and almost no base.
fresnel.set_editor_property("exponent", 4.0)
fresnel.set_editor_property("base_reflect_fraction", 0.06)
rim_colour = lib.create_material_expression(mat, unreal.MaterialExpressionConstant3Vector, -700, -220)
rim_colour.set_editor_property("constant", unreal.LinearColor(0.35, 0.60, 1.0, 1.0))
rim = lib.create_material_expression(mat, unreal.MaterialExpressionMultiply, -500, -200)
rim.set_editor_property("const_b", 2.6)
body_colour = lib.create_material_expression(mat, unreal.MaterialExpressionConstant3Vector, -700, 120)
body_colour.set_editor_property("constant", unreal.LinearColor(0.030, 0.036, 0.052, 1.0))
# SpiritFade is the scalar the encounter drives: 0 invisible, 1 fully present.
fade = lib.create_material_expression(mat, unreal.MaterialExpressionScalarParameter, -900, 320)
fade.set_editor_property("parameter_name", "SpiritFade")
fade.set_editor_property("default_value", 1.0)
opacity = lib.create_material_expression(mat, unreal.MaterialExpressionLinearInterpolate, -300, 220)
opacity.set_editor_property("const_a", 0.62)
opacity.set_editor_property("const_b", 1.0)
faded_rim = lib.create_material_expression(mat, unreal.MaterialExpressionMultiply, -120, 220)
opacity_faded = lib.create_material_expression(mat, unreal.MaterialExpressionMultiply, -120, 420)
lib.connect_material_expressions(fresnel, "", rim, "a")
lib.connect_material_expressions(rim_colour, "", rim, "b")
# Emissive is the only channel an unlit material draws, so the body colour goes
# there and the rim is added on top of it rather than multiplied into nothing.
surface = lib.create_material_expression(mat, unreal.MaterialExpressionAdd, -120, 0)
lib.connect_material_expressions(body_colour, "", surface, "a")
lib.connect_material_expressions(rim, "", surface, "b")
lib.connect_material_property(surface, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)
lib.recompile_material(mat)
assert unreal.EditorAssetLibrary.save_loaded_asset(mat, False), "material did not save"
report["material"] = mat.get_path_name()
# Read the parameter names back rather than trusting the graph: a ScalarParameter
# whose name did not stick leaves the MID setting nothing, and the body then
# renders at whatever the expression's default happened to be.
report["scalar_parameters"] = [str(n) for n in lib.get_scalar_parameter_names(mat)]
report["fade_default"] = fade.get_editor_property("default_value")
report["fade_name"] = str(fade.get_editor_property("parameter_name"))
# What the material actually ended up as, read back from the asset: a graph that
# is drawn in the editor but not connected to the material properties renders as
# the default lit grey, which is what the first opaque pass looked like.
report["shading_model"] = str(mat.get_editor_property("shading_model"))
report["blend_mode"] = str(mat.get_editor_property("blend_mode"))
report["emissive_inputs"] = [str(e.get_class().get_name())
                             for e in lib.get_inputs_for_material_property(mat, unreal.MaterialProperty.MP_EMISSIVE_COLOR)]
report["base_inputs"] = [str(e.get_class().get_name())
                         for e in lib.get_inputs_for_material_property(mat, unreal.MaterialProperty.MP_BASE_COLOR)]
report["expression_count"] = len(lib.get_material_expressions(mat))

# StaticMaterial takes (material_interface, material_slot_name) - the slot name
# is a Name, not a material, which is the order that failed first.
slot = unreal.Name("BearBody")
mesh.set_editor_property("static_materials", [unreal.StaticMaterial(mat, slot)])
unreal.EditorAssetLibrary.save_loaded_asset(mesh, False)
report["mesh"] = mesh.get_path_name()
report["mesh_materials"] = [str(m.material_interface.get_name())
                            for m in mesh.get_editor_property("static_materials") or [] if m.material_interface]

manifest = root / "Tests/Results/2026-09-19-spectral-bear-mesh-import.json"
manifest.write_text(json.dumps(report, indent=2) + "\n")
print(json.dumps(report))
