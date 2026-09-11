import unreal,pathlib
root=pathlib.Path(unreal.Paths.project_dir());tools=unreal.AssetToolsHelpers.get_asset_tools();lib=unreal.MaterialEditingLibrary
for name in ['S_DiscLaunch','S_DiscBounce']:
 t=unreal.AssetImportTask();t.filename=str(root/'SourceAssets/Audio'/f'{name}.wav');t.destination_path='/Game/BattleForTheA/Audio';t.destination_name=name;t.automated=True;t.save=True;t.replace_existing=True;tools.import_asset_tasks([t])
m=unreal.load_asset('/Game/BattleForTheA/Materials/M_DiscGlow')
if not m:m=tools.create_asset('M_DiscGlow','/Game/BattleForTheA/Materials',unreal.Material,unreal.MaterialFactoryNew())
lib.delete_all_material_expressions(m)
c=lib.create_material_expression(m,unreal.MaterialExpressionConstant3Vector);c.set_editor_property('constant',unreal.LinearColor(.15,3,.8));lib.connect_material_property(c,'',unreal.MaterialProperty.MP_EMISSIVE_COLOR)
lib.recompile_material(m);unreal.EditorAssetLibrary.save_loaded_asset(m)
