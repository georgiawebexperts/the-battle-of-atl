"""Swap only affected path meshes in the transient graded review world."""
import unreal,json,pathlib
root=pathlib.Path(unreal.Paths.project_dir());folder=root/'SourceAssets/Terrain/TenthStreetGraded/Connections';ea=unreal.get_editor_subsystem(unreal.EditorActorSubsystem);dest='/Game/BattleForTheA/Environment/TenthStreetGraded/Connections';rows=[]
for row in json.loads((folder/'manifest.json').read_text())['chunks']:
 actors=[a for a in ea.get_all_level_actors() if isinstance(a,unreal.StaticMeshActor) and a.static_mesh_component.static_mesh and a.static_mesh_component.static_mesh.get_name()==row['original_mesh']]
 if not actors:rows.append({'source':row['original_mesh'],'matched_actors':0});continue
 old=actors[0].static_mesh_component.static_mesh
 opts=unreal.FbxImportUI();opts.import_as_skeletal=False;opts.import_materials=False;opts.import_textures=False;opts.automated_import_should_detect_type=False;opts.mesh_type_to_import=unreal.FBXImportType.FBXIT_STATIC_MESH;opts.static_mesh_import_data.combine_meshes=True;opts.static_mesh_import_data.auto_generate_collision=False
 task=unreal.AssetImportTask();task.filename=str(folder/row['file']);task.destination_path=dest;task.destination_name=row['original_mesh'];task.automated=True;task.save=True;task.replace_existing=True;task.options=opts;unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
 mesh=unreal.load_asset(dest+'/'+row['original_mesh']);assert mesh
 b=mesh.get_bounding_box();bounds=[[b.min.x,b.min.y,b.min.z],[b.max.x,b.max.y,b.max.z]];assert max(abs(bounds[i][j]-row['bounds_cm'][i][j]) for i in range(2) for j in range(3))<.1
 mesh.set_material(0,old.get_material(0));mesh.get_editor_property('body_setup').set_editor_property('collision_trace_flag',unreal.CollisionTraceFlag.CTF_USE_COMPLEX_AS_SIMPLE);unreal.EditorAssetLibrary.save_loaded_asset(mesh)
 for actor in actors:actor.static_mesh_component.set_static_mesh(mesh)
 rows.append({'source':row['original_mesh'],'matched_actors':len(actors)})
near=[]
for a in ea.get_all_level_actors():
 if not isinstance(a,unreal.StaticMeshActor) or not a.static_mesh_component.static_mesh:continue
 c,e=a.get_actor_bounds(False)
 if c.x+e.x>9000 and c.x-e.x<15000 and c.y+e.y>9000 and c.y-e.y<16000:
  near.append({'label':a.get_actor_label(),'mesh':a.static_mesh_component.static_mesh.get_path_name()})
(root/'work/graded-path-connections.json').write_text(json.dumps({'changes':rows,'near_monroe':near},indent=2)+'\n')
