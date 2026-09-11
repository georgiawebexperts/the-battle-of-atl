import unreal,pathlib
root=pathlib.Path(unreal.Paths.project_dir());tools=unreal.AssetToolsHelpers.get_asset_tools()
for name in ['S_LockSwing','S_LockHit']:
 t=unreal.AssetImportTask();t.filename=str(root/'SourceAssets/Audio'/f'{name}.wav');t.destination_path='/Game/BattleForTheA/Audio';t.destination_name=name;t.automated=True;t.save=True;t.replace_existing=True;tools.import_asset_tasks([t])

lib=unreal.MaterialEditingLibrary
m=tools.create_asset('M_LockSteel','/Game/BattleForTheA/Materials',unreal.Material,unreal.MaterialFactoryNew()) if not unreal.load_asset('/Game/BattleForTheA/Materials/M_LockSteel') else unreal.load_asset('/Game/BattleForTheA/Materials/M_LockSteel')
lib.delete_all_material_expressions(m)
c=lib.create_material_expression(m,unreal.MaterialExpressionConstant3Vector);c.set_editor_property('constant',unreal.LinearColor(.16,.19,.22));lib.connect_material_property(c,'',unreal.MaterialProperty.MP_BASE_COLOR)
for prop,value in [(unreal.MaterialProperty.MP_METALLIC,.85),(unreal.MaterialProperty.MP_ROUGHNESS,.27)]:
 v=lib.create_material_expression(m,unreal.MaterialExpressionConstant);v.set_editor_property('r',value);lib.connect_material_property(v,'',prop)
lib.recompile_material(m);unreal.EditorAssetLibrary.save_loaded_asset(m)
