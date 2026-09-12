"""Reduce overexposure and make the first smoke plume readable."""
import unreal
lib=unreal.MaterialEditingLibrary
m=unreal.load_asset('/Game/BattleForTheA/Effects/BenchFire/M_BenchFire')
root=lib.get_material_property_input_node(m,unreal.MaterialProperty.MP_EMISSIVE_COLOR)
colors=[n for n in lib.get_inputs_for_material_expression(m,root) if isinstance(n,unreal.MaterialExpressionConstant3Vector)];assert len(colors)==1
colors[0].set_editor_property('constant',unreal.LinearColor(2.8,.6,.025));lib.recompile_material(m);assert unreal.EditorAssetLibrary.save_loaded_asset(m)
m=unreal.load_asset('/Game/BattleForTheA/Effects/BenchFire/M_BenchSmoke');fade=lib.get_material_property_input_node(m,unreal.MaterialProperty.MP_OPACITY)
opacity=lib.get_inputs_for_material_expression(m,fade)[0]
scalars=[n for n in lib.get_inputs_for_material_expression(m,opacity) if isinstance(n,unreal.MaterialExpressionConstant)];assert len(scalars)==1
scalars[0].set_editor_property('r',1.0);lib.recompile_material(m);assert unreal.EditorAssetLibrary.save_loaded_asset(m)
