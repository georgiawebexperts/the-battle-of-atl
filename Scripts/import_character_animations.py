"""Import authored CC0 clips using the full editor (-ExecutePythonScript), which provides Slate.

The commandlet route asserts in the legacy FBX importer before saving.
"""
import unreal,json,pathlib
root=pathlib.Path(unreal.Paths.project_dir())
unreal.SystemLibrary.execute_console_command(None,'Interchange.FeatureFlags.Import.FBX 0')
options=unreal.FbxImportUI()
options.set_editor_property('import_as_skeletal',True)
options.set_editor_property('import_mesh',False)
options.set_editor_property('import_animations',True)
options.set_editor_property('import_materials',False)
options.set_editor_property('import_textures',False)
options.set_editor_property('automated_import_should_detect_type',False)
options.set_editor_property('mesh_type_to_import',unreal.FBXImportType.FBXIT_ANIMATION)
options.set_editor_property('skeleton',unreal.load_asset('/Game/PiedmontRide/Rider/Casual_Skeleton'))
task=unreal.AssetImportTask();task.filename=str(root/'SourceAssets/Rider/Animations.fbx');task.destination_path='/Game/PiedmontRide/Rider/Animations';task.automated=True;task.save=True;task.replace_existing=False;task.options=options
unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
rows=[]
for path in unreal.EditorAssetLibrary.list_assets('/Game/PiedmontRide/Rider/Animations',recursive=True):
 asset=unreal.load_asset(path)
 if isinstance(asset,unreal.AnimSequence):
  assert unreal.EditorAssetLibrary.save_loaded_asset(asset,only_if_is_dirty=False)
  rows.append({'path':path,'seconds':asset.get_play_length()})
(root/'work/character-animation-import.json').write_text(json.dumps(rows,indent=2)+'\n')
assert rows,'No animation sequences imported'

unreal.SystemLibrary.quit_editor()
