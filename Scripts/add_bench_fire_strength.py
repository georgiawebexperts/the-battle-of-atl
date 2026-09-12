"""Add runtime fade strength to existing bench fire and smoke materials."""
import unreal
lib=unreal.MaterialEditingLibrary
for kind,prop in [('Fire',unreal.MaterialProperty.MP_EMISSIVE_COLOR),('Smoke',unreal.MaterialProperty.MP_OPACITY)]:
 m=unreal.load_asset('/Game/BattleForTheA/Effects/BenchFire/M_Bench'+kind);assert m
 assert 'Strength' not in [str(n) for n in lib.get_scalar_parameter_names(m)],'Strength already exists'
 source=lib.get_material_property_input_node(m,prop);output=lib.get_material_property_input_node_output_name(m,prop)
 strength=lib.create_material_expression(m,unreal.MaterialExpressionScalarParameter);strength.set_editor_property('parameter_name','Strength');strength.set_editor_property('default_value',1)
 product=lib.create_material_expression(m,unreal.MaterialExpressionMultiply)
 assert lib.connect_material_expressions(source,output,product,'A');assert lib.connect_material_expressions(strength,'',product,'B');assert lib.connect_material_property(product,'',prop)
 lib.recompile_material(m);assert unreal.EditorAssetLibrary.save_loaded_asset(m)
