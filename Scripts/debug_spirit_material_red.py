"""Is M_SpectralBearBody drawn at all, or is the engine substituting a default?

Two renders of the bear on 2026-09-19 could not be explained by the graph's own
values: a body authored near-black came out light grey. The project has twice
concluded that "materials built by these editor scripts come out wrong" and gone
back to an engine material, and twice it has been guessing. This settles it with
one unambiguous number instead of three ambiguous pictures: the material becomes
unlit emissive PURE RED, which no engine default, no sunlight and no tone mapper
can produce by accident.

  UnrealEditor-Cmd <project>.uproject -ExecutePythonScript=<this file>

Run Scripts/import_spectral_bear_mesh.py afterwards to restore the real graph.
"""
import unreal

dest = "/Game/BattleForTheA/Spirit"
name = "M_SpectralBearBody"
existing = unreal.load_asset(dest + "/" + name)
if existing:
    assert unreal.EditorAssetLibrary.delete_asset(dest + "/" + name), "could not replace " + name
mat = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
    name, dest, unreal.Material, unreal.MaterialFactoryNew())
assert mat
mat.set_editor_property("blend_mode", unreal.BlendMode.BLEND_OPAQUE)
mat.set_editor_property("shading_model", unreal.MaterialShadingModel.MSM_UNLIT)
unreal.MaterialEditingLibrary.recompile_material(mat)

red = unreal.MaterialEditingLibrary.create_material_expression(
    mat, unreal.MaterialExpressionConstant3Vector, -400, 0)
red.set_editor_property("constant", unreal.LinearColor(1.0, 0.0, 0.0, 1.0))
unreal.MaterialEditingLibrary.connect_material_property(
    red, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)
unreal.MaterialEditingLibrary.recompile_material(mat)
assert unreal.EditorAssetLibrary.save_loaded_asset(mat, False), "debug material did not save"

mesh = unreal.load_asset(dest + "/SM_SpectralBear")
mesh.set_editor_property("static_materials", [unreal.StaticMaterial(mat, unreal.Name("BearBody"))])
unreal.EditorAssetLibrary.save_loaded_asset(mesh, False)
print("DEBUG material is pure red emissive: " + mat.get_path_name())
unreal.SystemLibrary.quit_editor()
