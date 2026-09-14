"""Install candidate gap fillers in the currently loaded review transaction."""
import unreal,json,pathlib,sys
root=pathlib.Path(unreal.Paths.project_dir());sys.path.insert(0,str(root/'Scripts'))
from battle_geography import place_source_geometry
folder=root/'SourceAssets/Terrain/LakePathGrading/Seams';ea=unreal.get_editor_subsystem(unreal.EditorActorSubsystem);dest='/Game/BattleForTheA/Environment/LakePathGrading/Seams';rows=[]
for row in json.loads((folder/'manifest.json').read_text())['meshes']:
 name='SM_'+pathlib.Path(row['file']).stem;opts=unreal.FbxImportUI();opts.import_as_skeletal=False;opts.import_materials=False;opts.import_textures=False;opts.automated_import_should_detect_type=False;opts.mesh_type_to_import=unreal.FBXImportType.FBXIT_STATIC_MESH
 opts.static_mesh_import_data.combine_meshes=True;opts.static_mesh_import_data.auto_generate_collision=False;opts.static_mesh_import_data.remove_degenerates=False
 task=unreal.AssetImportTask();task.filename=str(folder/row['file']);task.destination_path=dest;task.destination_name=name;task.automated=True;task.save=True;task.replace_existing=True;task.options=opts;unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
 mesh=unreal.load_asset(dest+'/'+name);assert mesh and mesh.get_num_triangles(0)==row['triangles']
 material=unreal.load_asset('/Game/PiedmontRide/Materials/'+('M_Gravel' if row['material']=='Gravel' else 'M_Park'+row['material']+'World'));assert material
 mesh.set_material(0,material);mesh.get_editor_property('body_setup').set_editor_property('collision_trace_flag',unreal.CollisionTraceFlag.CTF_USE_COMPLEX_AS_SIMPLE);unreal.EditorAssetLibrary.save_loaded_asset(mesh)
 label='Lake path seam '+row['material'];existing=next((a for a in ea.get_all_level_actors() if a.get_actor_label()==label),None);actor=existing or ea.spawn_actor_from_class(unreal.StaticMeshActor,unreal.Vector());actor.set_actor_label(label);actor.set_folder_path('Piedmont/Pavement');actor.static_mesh_component.set_static_mesh(mesh);actor.static_mesh_component.set_collision_profile_name('BlockAll');actor.tags=[unreal.Name('LakePathSeam'),unreal.Name('RideDirt' if row['material']=='Gravel' else 'RidePath')];place_source_geometry(actor);rows.append({'actor':label,'triangles':mesh.get_num_triangles(0)})
(root/'work/lake-path-seams-import.json').write_text(json.dumps(rows,indent=2)+'\n')
