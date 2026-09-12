"""Duplicate bundled Epic fire/smoke textures into project-owned assets; plugin is import-only."""
import unreal,json,pathlib
root=pathlib.Path(unreal.Paths.project_dir());dest='/Game/BattleForTheA/Effects/BenchFire'
base='/NetworkPredictionExtras/Art/Effects/Proto/Shared/Textures/'
rows=[]
for kind,name in [('Fire','T_Fire_SubUV'),('Smoke','T_Smoke_SubUV')]:
 source=base+kind+'/'+name;target=dest+'/'+name
 texture=unreal.load_asset(source);assert texture,source
 assert not unreal.EditorAssetLibrary.does_asset_exist(target),'Inspect existing texture before duplicating'
 copy=unreal.EditorAssetLibrary.duplicate_asset(source,target);assert copy
 assert unreal.EditorAssetLibrary.save_loaded_asset(copy)
 task=unreal.AssetExportTask();task.object=copy;task.filename=str(root/'work'/f'{name}.tga');task.automated=True;task.prompt=False;task.replace_identical=True;task.exporter=unreal.TextureExporterTGA()
 exported=unreal.Exporter.run_asset_export_task(task)
 rows.append({'source':source,'target':target,'width':copy.blueprint_get_size_x(),'height':copy.blueprint_get_size_y(),'exported':exported})
(root/'Tests/Results/2026-09-12-bench-fire-textures.json').write_text(json.dumps({'textures':rows,'source':'Bundled Epic UE 5.8 NetworkPredictionExtras sample textures; for this Unreal project.','plugin_runtime_required':False,'fire_scene_implemented':False},indent=2)+'\n')
print(rows)
