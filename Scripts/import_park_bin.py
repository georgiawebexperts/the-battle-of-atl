import unreal,json,pathlib
root=pathlib.Path(unreal.Paths.project_dir());dest='/Game/BattleForTheA/Environment/ParkBin'
t=unreal.AssetImportTask();t.filename=str(root/'SourceAssets/Environment/ParkBin/ParkBin.gltf');t.destination_path=dest;t.automated=True;t.save=True;t.replace_existing=True
unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([t]);unreal.PiedmontWorldTools.finish_editor_asset_loading()
rows=[];sub=unreal.get_editor_subsystem(unreal.StaticMeshEditorSubsystem) or unreal.new_object(unreal.StaticMeshEditorSubsystem)
for path in unreal.EditorAssetLibrary.list_assets(dest,recursive=True,include_folder=False):
 m=unreal.load_asset(path)
 if isinstance(m,unreal.StaticMesh):
  settings=sub.get_nanite_settings(m);settings.enabled=False;sub.set_nanite_settings(m,settings,True);unreal.EditorAssetLibrary.save_loaded_asset(m)
  b=m.get_bounding_box();rows.append({'path':path,'triangles':m.get_num_triangles(0),'min':str(b.min),'max':str(b.max)})
assert len(rows)==1,rows
(root/'work/park-bin-import.json').write_text(json.dumps(rows,indent=2)+'\n');unreal.SystemLibrary.quit_editor()
