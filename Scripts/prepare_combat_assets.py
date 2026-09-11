import unreal,pathlib,json,traceback
root=pathlib.Path(unreal.Paths.project_dir());report={}
try:
 for name,color in [('Blood',(.42,.003,.009)),('GunMetal',(.025,.03,.038))]:
  path='/Game/PiedmontRide/Materials/M_'+name;m=unreal.load_asset(path)
  if not m:
   m=unreal.AssetToolsHelpers.get_asset_tools().create_asset('M_'+name,'/Game/PiedmontRide/Materials',unreal.Material,unreal.MaterialFactoryNew());c=unreal.MaterialEditingLibrary.create_material_expression(m,unreal.MaterialExpressionConstant3Vector);c.set_editor_property('constant',unreal.LinearColor(*color));unreal.MaterialEditingLibrary.connect_material_property(c,'',unreal.MaterialProperty.MP_BASE_COLOR);unreal.MaterialEditingLibrary.recompile_material(m);unreal.EditorAssetLibrary.save_loaded_asset(m)
 options=unreal.FbxImportUI();options.import_as_skeletal=False;options.import_materials=True;options.import_textures=False;options.automated_import_should_detect_type=False;options.mesh_type_to_import=unreal.FBXImportType.FBXIT_STATIC_MESH;options.static_mesh_import_data.combine_meshes=True;options.static_mesh_import_data.auto_generate_collision=False
 task=unreal.AssetImportTask();task.filename=str(root/'SourceAssets/Weapons/Pistol_1.fbx');task.destination_path='/Game/PiedmontRide/Bike';task.destination_name='SM_Pistol';task.automated=True;task.save=True;task.replace_existing=True;task.options=options;unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
 mesh=unreal.load_asset('/Game/PiedmontRide/Bike/SM_Pistol');report['pistol']=str(mesh);report['materials']=len(mesh.get_editor_property('static_materials'))
 knife=unreal.AssetImportTask();knife.filename=str(root/'SourceAssets/Weapons/Knife.obj');knife.destination_path='/Game/PiedmontRide/Bike';knife.destination_name='SM_Knife';knife.automated=True;knife.save=True;knife.replace_existing=True;knife.options=options;unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([knife])
 sound=unreal.AssetImportTask();sound.filename=str(root/'SourceAssets/Audio/S_Gunshot.wav');sound.destination_path='/Game/PiedmontRide/Audio';sound.destination_name='S_Gunshot';sound.automated=True;sound.save=True;sound.replace_existing=True;unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([sound])
 horn=unreal.AssetImportTask();horn.filename=str(root/'SourceAssets/Audio/S_Horn.wav');horn.destination_path='/Game/PiedmontRide/Audio';horn.destination_name='S_Horn';horn.automated=True;horn.save=True;horn.replace_existing=True;unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([horn])
 report['status']='installed' if mesh and unreal.load_asset('/Game/PiedmontRide/Audio/S_Gunshot') else 'failed'
except Exception:report={'status':'error','error':traceback.format_exc()}
(root/'Scripts/combat-assets.json').write_text(json.dumps(report,indent=2))
