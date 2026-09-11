import unreal,pathlib
root=pathlib.Path(unreal.Paths.project_dir());tools=unreal.AssetToolsHelpers.get_asset_tools();lib=unreal.MaterialEditingLibrary
m=unreal.load_asset('/Game/BattleForTheA/Materials/M_ColaRed')
if not m:m=tools.create_asset('M_ColaRed','/Game/BattleForTheA/Materials',unreal.Material,unreal.MaterialFactoryNew())
lib.delete_all_material_expressions(m)
c=lib.create_material_expression(m,unreal.MaterialExpressionConstant3Vector);c.set_editor_property('constant',unreal.LinearColor(.8,.012,.006));lib.connect_material_property(c,'',unreal.MaterialProperty.MP_BASE_COLOR)
for prop,value in [(unreal.MaterialProperty.MP_METALLIC,.45),(unreal.MaterialProperty.MP_ROUGHNESS,.28)]:
 v=lib.create_material_expression(m,unreal.MaterialExpressionConstant);v.set_editor_property('r',value);lib.connect_material_property(v,'',prop)
lib.recompile_material(m);unreal.EditorAssetLibrary.save_loaded_asset(m)
t=unreal.AssetImportTask();t.filename=str(root/'SourceAssets/Audio/S_ColaOpen.wav');t.destination_path='/Game/BattleForTheA/Audio';t.destination_name='S_ColaOpen';t.automated=True;t.save=True;t.replace_existing=True;tools.import_asset_tasks([t])
