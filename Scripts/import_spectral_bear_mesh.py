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
# The FBX carries no smoothing groups ("No smoothing group information was found
# for this mesh 'Plane'"), so a plain import computes one normal per triangle and
# the bear renders as flat-shaded facets - which is most of why a shape that is
# definitely a bear still reads as a lump of rocks. Computing the normals gets
# averaged, smooth shading instead.
# It lives on the import data, not on the UI object itself (that mistake cost a
# run: "Failed to find property 'normal_import_method' ... on 'FbxImportUI'").
smooth = options.get_editor_property("static_mesh_import_data")
smooth.set_editor_property("normal_import_method", unreal.FBXNormalImportMethod.FBXNIM_COMPUTE_NORMALS)
options.set_editor_property("static_mesh_import_data", smooth)
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

# The body is unlit, so EVERY value here goes straight to the screen - there is
# no scene light to hide behind, and a linear 0.055 emissive is a mid grey-blue
# once tone-mapped, not a black bear. Two renders on 2026-09-19 settled it: the
# "dark" body was grey, and the rim, which is view-dependent, covered the whole
# animal when it was met head-on from the approach and left it a flat lump when
# it was met from the side. A Fresnel alone cannot give a figure a consistent
# read, because from a low angle every normal is grazing.
#
# So the moonlight is baked into the graph as a direction instead: brightest on
# surfaces that face up, black underneath, with the rim added as a thin edge on
# top. That is the same from every angle the rider can meet it at, which is the
# property the last two passes were missing.
body_colour = lib.create_material_expression(mat, unreal.MaterialExpressionConstant3Vector, -1400, 120)
body_colour.set_editor_property("constant", unreal.LinearColor(0.010, 0.014, 0.030, 1.0))
up_vector = lib.create_material_expression(mat, unreal.MaterialExpressionConstant3Vector, -1600, -300)
up_vector.set_editor_property("constant", unreal.LinearColor(0.0, 0.0, 1.0, 1.0))
normal_ws = lib.create_material_expression(mat, unreal.MaterialExpressionVertexNormalWS, -1600, -120)
facing_up = lib.create_material_expression(mat, unreal.MaterialExpressionDotProduct, -1350, -200)
lib.connect_material_expressions(normal_ws, "", facing_up, "a")
lib.connect_material_expressions(up_vector, "", facing_up, "b")
above = lib.create_material_expression(mat, unreal.MaterialExpressionClamp, -1150, -200)
above.set_editor_property("min_default", 0.0)
above.set_editor_property("max_default", 1.0)
lib.connect_material_expressions(facing_up, "", above, "")
top = lib.create_material_expression(mat, unreal.MaterialExpressionMultiply, -950, -200)
lib.connect_material_expressions(above, "", top, "a")
lib.connect_material_expressions(above, "", top, "b")
top_scaled = lib.create_material_expression(mat, unreal.MaterialExpressionMultiply, -750, -200)
top_scaled.set_editor_property("const_b", 0.85)
lib.connect_material_expressions(top, "", top_scaled, "a")
fresnel = lib.create_material_expression(mat, unreal.MaterialExpressionFresnel, -1200, -520)
# A thin edge on top of the top light, not the whole read: exponent 4 with a
# low base fraction is an edge, where exponent 2 with 0.32 was a wash.
fresnel.set_editor_property("exponent", 4.0)
fresnel.set_editor_property("base_reflect_fraction", 0.015)
rim_scaled = lib.create_material_expression(mat, unreal.MaterialExpressionMultiply, -750, -520)
rim_scaled.set_editor_property("const_b", 1.2)
lib.connect_material_expressions(fresnel, "", rim_scaled, "a")
lit = lib.create_material_expression(mat, unreal.MaterialExpressionAdd, -550, -320)
lib.connect_material_expressions(top_scaled, "", lit, "a")
lib.connect_material_expressions(rim_scaled, "", lit, "b")
silver = lib.create_material_expression(mat, unreal.MaterialExpressionConstant3Vector, -550, -520)
silver.set_editor_property("constant", unreal.LinearColor(0.30, 0.56, 1.0, 1.0))
rim = lib.create_material_expression(mat, unreal.MaterialExpressionMultiply, -350, -420)
lib.connect_material_expressions(lit, "", rim, "a")
lib.connect_material_expressions(silver, "", rim, "b")
# SpiritFade is the scalar the encounter drives: 0 absent, 1 fully present. It is
# wired into the emissive so the slow reveal and the gentle dissolve are the
# material brightening in and out. It used to be connected to nothing - the
# reveal only scaled the body - so a "fade" parameter existed while nothing faded.
fade = lib.create_material_expression(mat, unreal.MaterialExpressionScalarParameter, -1200, 380)
fade.set_editor_property("parameter_name", "SpiritFade")
fade.set_editor_property("default_value", 1.0)
# Emissive is the only channel an unlit material draws, so the body colour goes
# there and the moonlight and rim are added on top of it.
surface = lib.create_material_expression(mat, unreal.MaterialExpressionAdd, -120, 0)
lib.connect_material_expressions(body_colour, "", surface, "a")
lib.connect_material_expressions(rim, "", surface, "b")
faded = lib.create_material_expression(mat, unreal.MaterialExpressionMultiply, 120, 0)
lib.connect_material_expressions(surface, "", faded, "a")
lib.connect_material_expressions(fade, "", faded, "b")
lib.connect_material_property(faded, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)
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
# get_inputs_for_material_property does not exist in this engine's Python API
# (checked the hard way), so the graph is reported by its expression list alone.
for key, getter in (("shading_model", "shading_model"), ("blend_mode", "blend_mode")):
    try:
        report[key] = str(mat.get_editor_property(getter))
    except Exception as error:  # noqa: BLE001 - a missing property must not stop the import
        report[key] = "unavailable: " + str(error)
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
