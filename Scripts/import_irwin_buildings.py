"""Import exterior study into a separate Irwin crossing review map."""
import json
from pathlib import Path
import unreal
root=Path(unreal.Paths.project_dir());folder=root/'SourceAssets/Terrain/IrwinBuildings';dest='/Game/BattleForTheA/Environment/IrwinBuildings'
assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/PiedmontIrwinCrossingReview')
ea=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
for a in ea.get_all_level_actors():
 if a.actor_has_tag('IrwinBuildingReview'):ea.destroy_actor(a)
assets=[]
for row in json.loads((folder/'manifest.json').read_text())['surfaces']:
 opts=unreal.FbxImportUI();opts.import_as_skeletal=False;opts.import_materials=False;opts.import_textures=False;opts.automated_import_should_detect_type=False;opts.mesh_type_to_import=unreal.FBXImportType.FBXIT_STATIC_MESH;opts.static_mesh_import_data.combine_meshes=True;opts.static_mesh_import_data.auto_generate_collision=False
 task=unreal.AssetImportTask();task.filename=str(folder/row['file']);task.destination_path=dest;task.destination_name='SM_IrwinBuildings_'+row['material'];task.automated=True;task.save=True;task.replace_existing=True;task.options=opts
 unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task]);mesh=unreal.load_asset(dest+'/'+task.destination_name);assert mesh
 mesh.set_material(0,unreal.load_asset('/Game/BattleForTheA/Environment/FancyRoachMotel/M_'+row['material']))
 mesh.get_editor_property('body_setup').set_editor_property('collision_trace_flag',unreal.CollisionTraceFlag.CTF_USE_COMPLEX_AS_SIMPLE);unreal.EditorAssetLibrary.save_loaded_asset(mesh)
 a=ea.spawn_actor_from_class(unreal.StaticMeshActor,unreal.Vector());a.set_actor_label('Irwin buildings '+row['material']);a.tags=[unreal.Name('IrwinBuildingReview'),unreal.Name('RideBarrier')];a.static_mesh_component.set_static_mesh(mesh);a.static_mesh_component.set_collision_profile_name('BlockAll' if row['material'] in ['Brick','Roof'] else 'NoCollision');assets.append(mesh.get_path_name())
cars=[a for a in ea.get_all_level_actors() if isinstance(a,unreal.BattleRoadCar)]
for car in cars:car.set_actor_enable_collision(False)
unreal.PiedmontWorldTools.finish_editor_asset_loading();failures=[]
for row in json.loads((root/'SourceAssets/Terrain/IrwinTraffic/car-lane-probes.json').read_text())['samples']:
 x,y,z=row['xyz'];hit=unreal.PiedmontWorldTools.trace_world_surface(unreal.Vector(x,y,z+150),unreal.Vector(x,y,z-150))
 if not hit or hit[1].actor_has_tag('IrwinBuildingReview') or abs(hit[0].z-z)>.25:failures.append({'sample':row,'actor':hit[1].get_actor_label() if hit else None,'height':hit[0].z if hit else None})
(root/'Tests/Results/2026-09-12-irwin-building-clearance.json').write_text(json.dumps({'failures':failures,'passed':not failures},indent=2)+'\n')
for car in cars:car.set_actor_enable_collision(True)
assert not failures,len(failures)
trail_failures=[]
for row in json.loads((folder/'trail-probes.json').read_text())['samples']:
 x,y,z=row['xyz'];hit=unreal.PiedmontWorldTools.trace_world_surface(unreal.Vector(x,y,z+180),unreal.Vector(x,y,z-100))
 if not hit or hit[1].actor_has_tag('IrwinBuildingReview') or abs(hit[0].z-z)>1.25:trail_failures.append({'sample':row,'actor':hit[1].get_actor_label() if hit else None,'height':hit[0].z if hit else None})
(root/'Tests/Results/2026-09-12-irwin-building-trail-clearance.json').write_text(json.dumps({'passed':not trail_failures,'probes':len(json.loads((folder/'trail-probes.json').read_text())['samples']),'failures':trail_failures},indent=2)+'\n')
assert not trail_failures,len(trail_failures)
assert unreal.EditorLoadingAndSavingUtils.save_map(unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world(),'/Game/PiedmontRide/Maps/PiedmontIrwinBuildingsReview')
(root/'Tests/Results/2026-09-12-irwin-building-import.json').write_text(json.dumps({'assets':assets,'road_probes':6474,'road_clear':not failures,'main_map_changed':False,'visual_accepted':False},indent=2)+'\n')
