"""Import road-join repairs and validate car footprints in an isolated native map."""
import unreal,json,pathlib
root=pathlib.Path(unreal.Paths.project_dir());folder=root/'SourceAssets/Terrain/TenthStreetGraded'
assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/PiedmontWorld')
ea=unreal.get_editor_subsystem(unreal.EditorActorSubsystem);world=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
# Replace the owned repair in this transient review world when rebuilding it.
for existing in ea.get_all_level_actors():
 if existing.actor_has_tag('TenthRoadSeams'):ea.destroy_actor(existing)
dest='/Game/BattleForTheA/Environment/TenthStreetGraded';name='SM_TenthStreet_RoadSeams'
opts=unreal.FbxImportUI();opts.import_as_skeletal=False;opts.import_materials=False;opts.import_textures=False;opts.automated_import_should_detect_type=False;opts.mesh_type_to_import=unreal.FBXImportType.FBXIT_STATIC_MESH;opts.static_mesh_import_data.combine_meshes=True;opts.static_mesh_import_data.auto_generate_collision=False
task=unreal.AssetImportTask();task.filename=str(folder/'TenthStreet_RoadSeams.obj');task.destination_path=dest;task.destination_name=name;task.automated=True;task.save=True;task.replace_existing=True;task.options=opts;unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
mesh=unreal.load_asset(dest+'/'+name);assert mesh;mesh.set_material(0,unreal.load_asset('/Game/PiedmontRide/Materials/M_ParkAsphaltWorld'));mesh.get_editor_property('body_setup').set_editor_property('collision_trace_flag',unreal.CollisionTraceFlag.CTF_USE_COMPLEX_AS_SIMPLE);unreal.EditorAssetLibrary.save_loaded_asset(mesh)
a=ea.spawn_actor_from_class(unreal.StaticMeshActor,unreal.Vector());a.set_actor_label('10th Street road join repairs');a.tags=[unreal.Name('TenthRoadSeams'),unreal.Name('RidePath')];a.static_mesh_component.set_static_mesh(mesh);a.static_mesh_component.set_collision_profile_name('BlockAll');a.set_folder_path('Midtown/TenthStreet')
unreal.PiedmontWorldTools.finish_editor_asset_loading()
samples=json.loads((folder/'car-lane-probes.json').read_text())['samples'];failures=[];max_error=0;hits={}
for row in samples:
 x,y,z=row['xyz'];hit=unreal.PiedmontWorldTools.trace_world_surface(unreal.Vector(x,y,z+150),unreal.Vector(x,y,z-150))
 error=None if not hit else abs(hit[0].z-z)
 label=None if not hit else hit[1].get_actor_label();hits[label]=hits.get(label,0)+1
 if error is None or error>3 or isinstance(hit[1],unreal.Landscape):failures.append({'xyz':row['xyz'],'lane':row['lane'],'error_cm':error,'actor':label})
 else:max_error=max(max_error,error)
report={'passed':not failures,'probes':len(samples),'failures':failures,'max_error_cm':max_error,'surface_hits':hits,'main_map_changed':False,'scope':'Vertical footprint collision checks on candidate lanes and repair mesh. Actual driving, crossing controls and packaged behavior pending.'}
if not failures:
 routes=json.loads((folder/'car-lanes.json').read_text())['routes']
 for route in routes:
  car=ea.spawn_actor_from_class(unreal.BattleRoadCar,unreal.Vector(*route['points_cm'][0])+unreal.Vector(0,0,73.3));car.set_actor_label('10th lane review '+route['name']);car.set_editor_property('route',[unreal.Vector(*p) for p in route['points_cm']]);car.tags=list(car.tags)+[unreal.Name('TenthCarLaneReview')]
 target='/Game/PiedmontRide/Maps/PiedmontCarLaneReview';assert unreal.EditorLoadingAndSavingUtils.save_map(world,target);report['review_map']=target
(root/'Tests/Results/2026-09-12-tenth-car-lane-collision.json').write_text(json.dumps(report,indent=2)+'\n')
assert report['passed'],len(failures)
