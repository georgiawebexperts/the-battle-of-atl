"""Dedicated knife-attacker palette; leaves the player and shared mesh unchanged."""
import unreal,json,pathlib
folder='/Game/BattleForTheA/Hostiles/Knife'
rows=[];lib=unreal.MaterialEditingLibrary
for name,color in [('M_KnifeTop',(.28,.045,.018)),('M_KnifeShorts',(.018,.022,.028)),('M_KnifeHair',(.025,.016,.012))]:
 m=unreal.load_asset(folder+'/'+name)
 if not m:m=unreal.AssetToolsHelpers.get_asset_tools().create_asset(name,folder,unreal.Material,unreal.MaterialFactoryNew())
 lib.delete_all_material_expressions(m)
 c=lib.create_material_expression(m,unreal.MaterialExpressionConstant3Vector);c.set_editor_property('constant',unreal.LinearColor(*color));lib.connect_material_property(c,'',unreal.MaterialProperty.MP_BASE_COLOR)
 rough=lib.create_material_expression(m,unreal.MaterialExpressionConstant);rough.set_editor_property('r',.82);lib.connect_material_property(rough,'',unreal.MaterialProperty.MP_ROUGHNESS)
 m.set_editor_property('used_with_skeletal_mesh',True);lib.recompile_material(m)
 assert unreal.EditorAssetLibrary.save_loaded_asset(m,only_if_is_dirty=False)
 rows.append({'asset':m.get_path_name(),'color':color})
pathlib.Path(unreal.Paths.project_dir(),'work/knife-palette.json').write_text(json.dumps(rows,indent=2)+'\n')
