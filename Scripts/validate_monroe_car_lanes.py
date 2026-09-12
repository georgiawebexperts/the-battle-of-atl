"""Check actual native collision under proposed Monroe traffic footprints."""
import unreal,json,pathlib
root=pathlib.Path(unreal.Paths.project_dir());folder=root/'SourceAssets/Terrain/MonroeTraffic'
assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/PiedmontWorld')
ea=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
for existing in ea.get_all_level_actors():
 if existing.actor_has_tag('MonroeRoadSeams'):ea.destroy_actor(existing)
dest='/Game/BattleForTheA/Environment/MonroeTraffic';name='SM_Monroe_RoadSeams'
opts=unreal.FbxImportUI();opts.import_as_skeletal=False;opts.import_materials=False;opts.import_textures=False;opts.automated_import_should_detect_type=False;opts.mesh_type_to_import=unreal.FBXImportType.FBXIT_STATIC_MESH;opts.static_mesh_import_data.combine_meshes=True;opts.static_mesh_import_data.auto_generate_collision=False
task=unreal.AssetImportTask();task.filename=str(folder/'Monroe_RoadSeams.obj');task.destination_path=dest;task.destination_name=name;task.automated=True;task.save=True;task.replace_existing=True;task.options=opts;unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
mesh=unreal.load_asset(dest+'/'+name);assert mesh;mesh.set_material(0,unreal.load_asset('/Game/PiedmontRide/Materials/M_ParkAsphaltWorld'));mesh.get_editor_property('body_setup').set_editor_property('collision_trace_flag',unreal.CollisionTraceFlag.CTF_USE_COMPLEX_AS_SIMPLE);unreal.EditorAssetLibrary.save_loaded_asset(mesh)
a=ea.spawn_actor_from_class(unreal.StaticMeshActor,unreal.Vector());a.set_actor_label('Monroe road join repairs');a.tags=[unreal.Name('MonroeRoadSeams'),unreal.Name('RidePath')];a.static_mesh_component.set_static_mesh(mesh);a.static_mesh_component.set_collision_profile_name('BlockAll');a.set_folder_path('Midtown/TenthStreet')
unreal.PiedmontWorldTools.finish_editor_asset_loading()
source=json.loads((folder/'car-lanes.json').read_text());samples=json.loads((folder/'car-lane-probes.json').read_text())['samples']
# Include source gaps: native terrain/path coverage must not silently hide a road hole.
for row in source['failures']:samples.append({'lane':row['lane'],'xyz':row['xy']+[-350],'source_surface':'MISSING'})
failures=[];hits={}
for row in samples:
 x,y,z=row['xyz'];hit=unreal.PiedmontWorldTools.trace_world_surface(unreal.Vector(x,y,z+250),unreal.Vector(x,y,z-250));label=None if not hit else hit[1].get_actor_label();hits[label]=hits.get(label,0)+1
 if not hit or isinstance(hit[1],unreal.Landscape) or row['source_surface']=='MISSING' or abs(hit[0].z-z)>3:failures.append({'xyz':row['xyz'],'actor':label,'height':None if not hit else hit[0].z,'source_surface':row['source_surface']})
r={'passed':not failures,'probes':len(samples),'failures':failures,'hits':hits,'main_map_changed':False,'scope':'Native footprint support only; routes, signals and actual car movement unverified.'}
if not failures:
 ea=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
 for row in source['routes']:
  car=ea.spawn_actor_from_class(unreal.BattleRoadCar,unreal.Vector(*row['points_cm'][0])+unreal.Vector(0,0,73.3));car.set_actor_label('Monroe '+row['name']);car.tags=list(car.tags)+[unreal.Name('MonroeCarLaneReview')];car.set_editor_property('route',[unreal.Vector(*p) for p in row['points_cm']]);car.set_editor_property('cruise_speed',row['speed_cm_s'])
 assert unreal.EditorLoadingAndSavingUtils.save_map(unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world(),'/Game/PiedmontRide/Maps/PiedmontMonroeTrafficReview')
(root/'Tests/Results/2026-09-12-monroe-car-support.json').write_text(json.dumps(r,indent=2)+'\n')
