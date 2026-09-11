"""Import the CC0 Quaternius rifle and report its native asset bounds."""
import unreal,json,pathlib
root=pathlib.Path(unreal.Paths.project_dir())
t=unreal.AssetImportTask();t.filename=str(root/'SourceAssets/Weapons/Rifle.gltf');t.destination_path='/Game/BattleForTheA/Weapons/Rifle';t.destination_name='Rifle';t.automated=True;t.save=True;t.replace_existing=True
unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([t]);rows=[]
for p in unreal.EditorAssetLibrary.list_assets(t.destination_path,recursive=True,include_folder=False):
 a=unreal.load_asset(p)
 if isinstance(a,unreal.StaticMesh):
  b=a.get_bounding_box();rows.append({'path':p,'min':str(b.min),'max':str(b.max)})
unreal.EditorAssetLibrary.save_directory(t.destination_path,only_if_is_dirty=False,recursive=True)
(root/'work/rifle-import.json').write_text(json.dumps(rows,indent=2)+'\n');assert len(rows)==1;unreal.SystemLibrary.quit_editor()
