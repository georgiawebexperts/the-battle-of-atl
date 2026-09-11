import unreal,pathlib,json
root=pathlib.Path(unreal.Paths.project_dir());tools=unreal.AssetToolsHelpers.get_asset_tools()
m=unreal.load_asset('/Game/BattleForTheA/Materials/M_ShotGlow')
if not m:
 m=tools.create_asset('M_ShotGlow','/Game/BattleForTheA/Materials',unreal.Material,unreal.MaterialFactoryNew());c=unreal.MaterialEditingLibrary.create_material_expression(m,unreal.MaterialExpressionConstant3Vector);c.set_editor_property('constant',unreal.LinearColor(14,5,.25));unreal.MaterialEditingLibrary.connect_material_property(c,'',unreal.MaterialProperty.MP_EMISSIVE_COLOR);unreal.MaterialEditingLibrary.recompile_material(m);unreal.EditorAssetLibrary.save_loaded_asset(m)
t=unreal.AssetImportTask();t.filename=str(root/'SourceAssets/Audio/S_Boost.wav');t.destination_path='/Game/BattleForTheA/Audio';t.destination_name='S_Boost';t.automated=True;t.save=True;t.replace_existing=True;tools.import_asset_tasks([t])
(root/'Scripts/battle-effects.json').write_text(json.dumps({'glow':bool(m),'boost':bool(unreal.load_asset('/Game/BattleForTheA/Audio/S_Boost'))}))
