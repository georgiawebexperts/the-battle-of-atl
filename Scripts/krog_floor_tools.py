"""Apply the authored level-floor candidate to an already loaded review world."""
import json,unreal

def apply_level_floor(root,ea):
 actors=[a for a in ea.get_all_level_actors() if a.get_actor_label()=='Krog route SM_KrogTunnel_Road'];assert len(actors)==1
 road=actors[0]
 if road.actor_has_tag('KrogLevelFloorApplied'):return
 road.tags=list(road.tags)+[unreal.Name('KrogLevelFloorApplied')]
 old=road.get_actor_location();road.set_actor_location(old+unreal.Vector(0,0,12),False,False)
 road.static_mesh_component.set_material(0,unreal.load_asset('/Game/PiedmontRide/Materials/M_ParkAsphaltWeatheredCandidate'))
 folder=root/'SourceAssets/Terrain/KrogTraffic'
 for row in json.loads((folder/'level-floor-concrete.json').read_text())['surfaces']:
  targets=[a for a in ea.get_all_level_actors() if a.get_actor_label()=='Krog route SM_'+row['source']];assert len(targets)==1
  if row['remaining_triangles']==0:assert ea.destroy_actor(targets[0]);continue
  opts=unreal.FbxImportUI();opts.import_as_skeletal=False;opts.import_materials=False;opts.import_textures=False
  opts.automated_import_should_detect_type=False;opts.mesh_type_to_import=unreal.FBXImportType.FBXIT_STATIC_MESH
  opts.static_mesh_import_data.combine_meshes=True;opts.static_mesh_import_data.auto_generate_collision=False
  name='SM_'+row['source']+'_LevelFloor';dest='/Game/BattleForTheA/Environment/KrogTraffic'
  task=unreal.AssetImportTask();task.filename=str(folder/(row['source']+'_LevelFloor.obj'));task.destination_path=dest;task.destination_name=name
  task.automated=True;task.save=True;task.replace_existing=True;task.options=opts
  unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task]);mesh=unreal.load_asset(dest+'/'+name);assert mesh
  mesh.set_material(0,targets[0].static_mesh_component.get_material(0))
  mesh.get_editor_property('body_setup').set_editor_property('collision_trace_flag',unreal.CollisionTraceFlag.CTF_USE_COMPLEX_AS_SIMPLE)
  unreal.EditorAssetLibrary.save_loaded_asset(mesh);targets[0].static_mesh_component.set_static_mesh(mesh)
