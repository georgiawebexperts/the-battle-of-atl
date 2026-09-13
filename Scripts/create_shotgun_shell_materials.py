import unreal
base='/Game/BattleForTheA/Weapons/ShotgunParts';lib=unreal.MaterialEditingLibrary;tools=unreal.AssetToolsHelpers.get_asset_tools()
for name,color,metal,rough in [('M_ShellRed',(.45,.012,.007),0,.42),('M_ShellBrass',(.55,.3,.045),.8,.3)]:
 mat=unreal.load_asset(base+'/'+name) or tools.create_asset(name,base,unreal.Material,unreal.MaterialFactoryNew());lib.delete_all_material_expressions(mat)
 n=lib.create_material_expression(mat,unreal.MaterialExpressionConstant3Vector);n.constant=unreal.LinearColor(*color);lib.connect_material_property(n,'',unreal.MaterialProperty.MP_BASE_COLOR)
 for value,prop in [(metal,unreal.MaterialProperty.MP_METALLIC),(rough,unreal.MaterialProperty.MP_ROUGHNESS)]:
  n=lib.create_material_expression(mat,unreal.MaterialExpressionConstant);n.r=value;lib.connect_material_property(n,'',prop)
 lib.recompile_material(mat);unreal.EditorAssetLibrary.save_loaded_asset(mat)
unreal.PiedmontWorldTools.finish_editor_asset_loading();unreal.SystemLibrary.quit_editor()
