"""Create reusable native traffic signal finishes and a controllable lens material."""
import unreal
lib=unreal.MaterialEditingLibrary;dest='/Game/BattleForTheA/Traffic'
for name,color,rough,metal in [('M_SignalHousing',(.022,.025,.028),.72,.1),('M_SignalPole',(.22,.24,.25),.48,.65),('M_SignalLens',(.12,.003,.001),.24,0)]:
 existing=unreal.load_asset(dest+'/'+name)
 m=existing or unreal.AssetToolsHelpers.get_asset_tools().create_asset(name,dest,unreal.Material,unreal.MaterialFactoryNew());assert m
 lib.delete_all_material_expressions(m)
 node=lib.create_material_expression(m,unreal.MaterialExpressionVectorParameter);node.set_editor_property('parameter_name','Color');node.set_editor_property('default_value',unreal.LinearColor(*color));lib.connect_material_property(node,'RGB',unreal.MaterialProperty.MP_BASE_COLOR)
 for prop,value in [(unreal.MaterialProperty.MP_ROUGHNESS,rough),(unreal.MaterialProperty.MP_METALLIC,metal)]:
  c=lib.create_material_expression(m,unreal.MaterialExpressionConstant);c.set_editor_property('r',value);lib.connect_material_property(c,'',prop)
 if name=='M_SignalLens':
  strength=lib.create_material_expression(m,unreal.MaterialExpressionScalarParameter);strength.set_editor_property('parameter_name','Strength');strength.set_editor_property('default_value',0)
  glow=lib.create_material_expression(m,unreal.MaterialExpressionMultiply);lib.connect_material_expressions(node,'RGB',glow,'A');lib.connect_material_expressions(strength,'',glow,'B');lib.connect_material_property(glow,'',unreal.MaterialProperty.MP_EMISSIVE_COLOR)
 lib.recompile_material(m);assert unreal.EditorAssetLibrary.save_loaded_asset(m)
unreal.PiedmontWorldTools.finish_editor_asset_loading()
