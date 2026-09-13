"""Import the credited Remington art candidate; no main-map or gameplay changes."""
import unreal,json,pathlib
root=pathlib.Path(unreal.Paths.project_dir());dest='/Game/BattleForTheA/Weapons/Remington870'
t=unreal.AssetImportTask();t.filename=str(root/'SourceAssets/Weapons/Remington870/Remington870.gltf');t.destination_path=dest;t.automated=True;t.save=True;t.replace_existing=True
unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([t]);rows=[]
for p in unreal.EditorAssetLibrary.list_assets(dest,recursive=True,include_folder=False):
 a=unreal.load_asset(p)
 if isinstance(a,unreal.StaticMesh):
  b=a.get_bounding_box();rows.append({'path':p,'min':str(b.min),'max':str(b.max),'lods':a.get_num_lods()})
unreal.EditorAssetLibrary.save_directory(dest,only_if_is_dirty=False,recursive=True)
(root/'work/remington870-import.json').write_text(json.dumps(rows,indent=2)+'\n');assert len(rows)==1
unreal.SystemLibrary.quit_editor()
