"""Export installed City body/clothing for an arms-only derivative; no asset/map saves."""
import unreal,json
from pathlib import Path
root=Path(unreal.Paths.project_dir()).resolve();out=root/'SourceAssets/Rider/DetailedArms';out.mkdir(parents=True,exist_ok=True)
unreal.PiedmontWorldTools.finish_editor_asset_loading()
rows=[]
for name in ('body','crewneck'):
 path='/Game/CitySampleCrowd/Character/Male/NormalWeight/Meshes/m_tal_nrw_'+name
 mesh=unreal.load_asset(path);assert mesh
 task=unreal.AssetExportTask();task.object=mesh;task.filename=str(out/(name+'.fbx'));task.automated=True;task.prompt=False;task.replace_identical=True
 opts=unreal.FbxExportOption();opts.ascii=False;opts.vertex_color=True;opts.level_of_detail=False;task.options=opts
 assert unreal.Exporter.run_asset_export_task(task),str(task.errors)
 rows.append({'name':name,'mesh':path,'file':task.filename,'materials':[{'slot':str(x.material_slot_name),'asset':x.material_interface.get_path_name() if x.material_interface else None} for x in mesh.get_editor_property('materials')]})
(out/'sources.json').write_text(json.dumps(rows,indent=2)+'\n')
print('DetailedArmsSource: exported body and crewneck')
