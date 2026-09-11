"""Create dedicated instancing-compatible timber and dark metal bench materials."""
import unreal,json,pathlib
root=pathlib.Path(unreal.Paths.project_dir());rows=[]
for name,color,rough,metal in [('M_BenchWood',(.22,.095,.032),.7,0),('M_BenchFrame',(.018,.024,.021),.4,.8)]:
 path='/Game/BattleForTheA/Furniture/'+name
 m=unreal.load_asset(path)
 if not m:m=unreal.AssetToolsHelpers.get_asset_tools().create_asset(name,'/Game/BattleForTheA/Furniture',unreal.Material,unreal.MaterialFactoryNew())
 unreal.MaterialEditingLibrary.delete_all_material_expressions(m)
 c=unreal.MaterialEditingLibrary.create_material_expression(m,unreal.MaterialExpressionConstant3Vector);c.set_editor_property('constant',unreal.LinearColor(*color));unreal.MaterialEditingLibrary.connect_material_property(c,'',unreal.MaterialProperty.MP_BASE_COLOR)
 for prop,value in [(unreal.MaterialProperty.MP_ROUGHNESS,rough),(unreal.MaterialProperty.MP_METALLIC,metal)]:
  e=unreal.MaterialEditingLibrary.create_material_expression(m,unreal.MaterialExpressionConstant);e.set_editor_property('r',value);unreal.MaterialEditingLibrary.connect_material_property(e,'',prop)
 m.set_editor_property('used_with_instanced_static_meshes',True);unreal.MaterialEditingLibrary.recompile_material(m)
 assert unreal.EditorAssetLibrary.save_loaded_asset(m,only_if_is_dirty=False)
 rows.append({'path':m.get_path_name(),'instanced_usage':m.get_editor_property('used_with_instanced_static_meshes')})
(root/'work/bench-materials.json').write_text(json.dumps(rows,indent=2)+'\n')
unreal.SystemLibrary.quit_editor()
