import unreal,pathlib,json,traceback
root=pathlib.Path(unreal.Paths.project_dir())
try:
 opts=unreal.FbxImportUI();opts.import_as_skeletal=True;opts.import_mesh=True;opts.import_animations=False;opts.import_materials=True;opts.import_textures=False;opts.automated_import_should_detect_type=False;opts.mesh_type_to_import=unreal.FBXImportType.FBXIT_SKELETAL_MESH
 t=unreal.AssetImportTask();t.filename=str(root/'SourceAssets/Rider/FPSArms.fbx');t.destination_path='/Game/BattleForTheA/Rider';t.destination_name='SK_FPSArms';t.automated=True;t.save=True;t.replace_existing=True;t.options=opts;unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([t]);mesh=unreal.load_asset('/Game/BattleForTheA/Rider/SK_FPSArms');report={'status':'imported' if mesh else 'failed','paths':list(t.imported_object_paths)}
 if mesh:
  slots=mesh.get_editor_property('materials');names=[]
  for slot in slots:
   name=str(slot.material_slot_name);source=unreal.load_asset('/Game/PiedmontRide/Rider/'+name)
   if source:slot.material_interface=source
   names.append({'slot':name,'reused':bool(source)})
  mesh.set_editor_property('materials',slots);unreal.EditorAssetLibrary.save_loaded_asset(mesh);report['materials']=names
except Exception:report={'status':'error','error':traceback.format_exc()}
(root/'Scripts/fps-arms-import.json').write_text(json.dumps(report,indent=2))
