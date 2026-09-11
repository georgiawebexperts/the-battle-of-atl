import unreal,pathlib,json
root=pathlib.Path(unreal.Paths.project_dir());tools=unreal.AssetToolsHelpers.get_asset_tools()
m=unreal.load_asset('/Game/BattleForTheA/Materials/M_Splash')
if not m:m=tools.create_asset('M_Splash','/Game/BattleForTheA/Materials',unreal.Material,unreal.MaterialFactoryNew())
unreal.MaterialEditingLibrary.delete_all_material_expressions(m);m.set_editor_property('blend_mode',unreal.BlendMode.BLEND_TRANSLUCENT)
c=unreal.MaterialEditingLibrary.create_material_expression(m,unreal.MaterialExpressionConstant3Vector);c.set_editor_property('constant',unreal.LinearColor(.15,.65,1));unreal.MaterialEditingLibrary.connect_material_property(c,'',unreal.MaterialProperty.MP_BASE_COLOR);unreal.MaterialEditingLibrary.connect_material_property(c,'',unreal.MaterialProperty.MP_EMISSIVE_COLOR)
o=unreal.MaterialEditingLibrary.create_material_expression(m,unreal.MaterialExpressionConstant);o.set_editor_property('r',.6);unreal.MaterialEditingLibrary.connect_material_property(o,'',unreal.MaterialProperty.MP_OPACITY);unreal.MaterialEditingLibrary.recompile_material(m);unreal.EditorAssetLibrary.save_loaded_asset(m)
loaded=[]
for name in ['Skid','Splash','Bump','Asphalt','Grass','Motor']:
 t=unreal.AssetImportTask();t.filename=str(root/f'SourceAssets/Audio/S_{name}.wav');t.destination_path='/Game/BattleForTheA/Audio';t.destination_name='S_'+name;t.automated=True;t.save=True;t.replace_existing=True;tools.import_asset_tasks([t]);sound=unreal.load_asset('/Game/BattleForTheA/Audio/S_'+name)
 if sound:
  sound.set_editor_property('virtualization_mode',unreal.VirtualizationMode.PLAY_WHEN_SILENT);sound.set_editor_property('looping',name in ['Asphalt','Grass','Motor']);unreal.EditorAssetLibrary.save_loaded_asset(sound);loaded.append(name)
(root/'Scripts/ride-effects-import.json').write_text(json.dumps({'material':bool(m),'sounds':loaded}))
