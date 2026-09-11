"""Import the creator's CC0 rigged Swat model without modifying existing characters."""
import unreal,json,pathlib
root=pathlib.Path(unreal.Paths.project_dir())
unreal.SystemLibrary.execute_console_command(None,'Interchange.FeatureFlags.Import.FBX 0')
options=unreal.FbxImportUI()
for key,value in {'import_as_skeletal':True,'import_mesh':True,'import_animations':False,'import_materials':True,'import_textures':True,'automated_import_should_detect_type':False}.items():options.set_editor_property(key,value)
options.set_editor_property('mesh_type_to_import',unreal.FBXImportType.FBXIT_SKELETAL_MESH)
task=unreal.AssetImportTask();task.filename=str(root/'SourceAssets/Rider/Swat.fbx');task.destination_path='/Game/BattleForTheA/Police';task.destination_name='Swat';task.automated=True;task.save=True;task.replace_existing=True;task.options=options
unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
unreal.EditorAssetLibrary.save_directory('/Game/BattleForTheA/Police',only_if_is_dirty=False,recursive=True)
rows=[]
for path in task.imported_object_paths:
 asset=unreal.load_asset(path);assert unreal.EditorAssetLibrary.save_loaded_asset(asset,only_if_is_dirty=False)
 row={'path':path,'class':asset.get_class().get_name()}
 if isinstance(asset,unreal.SkeletalMesh):
  row['skeleton']=str(asset.get_editor_property('skeleton'))
  row['materials']=[str(m.get_editor_property('material_interface')) for m in asset.get_editor_property('materials')]
 rows.append(row)
(root/'work/police-import.json').write_text(json.dumps(rows,indent=2)+'\n')
assert any(r['class']=='SkeletalMesh' for r in rows)
unreal.SystemLibrary.quit_editor()
