"""Import derivative hand/sleeve meshes with original City materials and skeleton."""
import unreal,json
from pathlib import Path
refresh='-RefreshDetailedArms' in unreal.SystemLibrary.get_command_line()
root=Path(unreal.Paths.project_dir()).resolve();base=root/'SourceAssets/Rider/DetailedArms';rows=[]
for source in json.loads((base/'sources.json').read_text()):
 name=source['name'];asset_name='SK_DetailedHands' if name=='body' else 'SK_DetailedSleeves';dest='/Game/BattleForTheA/Rider/Detailed/'+asset_name
 assert refresh or not unreal.EditorAssetLibrary.does_asset_exist(dest),'Refuse to overwrite existing derivative: '+dest
 original=unreal.load_asset(source['mesh']);assert original
 opts=unreal.FbxImportUI();opts.import_as_skeletal=True;opts.import_mesh=True;opts.import_animations=False;opts.import_materials=False;opts.import_textures=False;opts.create_physics_asset=False;opts.automated_import_should_detect_type=False;opts.mesh_type_to_import=unreal.FBXImportType.FBXIT_SKELETAL_MESH;opts.skeleton=original.get_editor_property('skeleton');opts.skeletal_mesh_import_data.vertex_color_import_option=unreal.VertexColorImportOption.REPLACE
 task=unreal.AssetImportTask();task.filename=str(base/(name+'_arms.fbx'));task.destination_path='/Game/BattleForTheA/Rider/Detailed';task.destination_name=asset_name;task.automated=True;task.save=False;task.replace_existing=refresh;task.options=opts
 unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task]);mesh=unreal.load_asset(dest);assert mesh
 slots=mesh.get_editor_property('materials');assert len(slots)==len(source['materials'])
 for i,mat in enumerate(source['materials']):
  slot=slots[i];slot.material_interface=unreal.load_asset(mat['asset']);assert slot.material_interface;slots[i]=slot
 mesh.set_editor_property('materials',slots);assert unreal.EditorAssetLibrary.save_loaded_asset(mesh)
 rows.append({'asset':dest,'source':task.filename,'materials':source['materials'],'skeleton':mesh.get_editor_property('skeleton').get_path_name()})
(root/'Tests/Results/2026-09-13-detailed-arms-import.json').write_text(json.dumps({'assets':rows,'scope':'Imported derivatives only; pose/render validation pending.'},indent=2)+'\n')
