"""Install native-tested Irwin/Lake road traffic, preserving other corridors."""
import json
from pathlib import Path
import unreal
root=Path(unreal.Paths.project_dir())
for suffix in ['native-irwin-population','native-irwin-occupancy','native-irwin-lanes','irwin-lane-support']:
 assert json.loads((root/f'Tests/Results/2026-09-12-{suffix}.json').read_text())['passed'],suffix
review=json.loads((root/'Tests/Results/2026-09-12-irwin-crossing-render.json').read_text())
assert review.get('road_geometry_accepted_for_integration'), 'Rendered road review required'
assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/PiedmontIrwinPopulationReview')
ea=unreal.get_editor_subsystem(unreal.EditorActorSubsystem);roads=[];gates=[];lanes=[]
for a in ea.get_all_level_actors():
 if a.actor_has_tag('IrwinRoad'):roads.append((a.get_actor_label(),a.static_mesh_component.static_mesh.get_path_name(),a.get_actor_transform()))
 if a.actor_has_tag('IrwinCrossingReview'):gates.append((a.get_actor_transform(),a.get_editor_property('CrossingArea').get_unscaled_box_extent()))
 if a.actor_has_tag('IrwinTrafficReview'):
  for lane in a.get_editor_property('Lanes'):
   lanes.append((str(lane.get_editor_property('Name')),list(lane.get_editor_property('Points')),lane.get_editor_property('CruiseSpeed'),lane.get_editor_property('Crossings')[0].get_editor_property('StopDistance')))
assert len(roads)==1 and len(gates)==1 and len(lanes)==2
assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/PiedmontWorld')
actors=ea.get_all_level_actors();assert not any(a.actor_has_tag('IrwinRoadTraffic') or a.actor_has_tag('IrwinRoad') for a in actors),'Already installed'
retained={tag:sum(a.actor_has_tag(tag) for a in actors) for tag in ['TenthRoadTraffic','MonroeRoadTraffic']};assert all(retained.values())
for label,path,transform in roads:
 a=ea.spawn_actor_from_class(unreal.StaticMeshActor,transform.translation);a.set_actor_transform(transform,False,False);a.set_actor_label(label)
 a.tags=[unreal.Name('IrwinRoad'),unreal.Name('RidePath')];a.set_folder_path('Eastside/IrwinLake')
 a.static_mesh_component.set_static_mesh(unreal.load_asset(path));a.static_mesh_component.set_collision_profile_name('BlockAll')
transform,extent=gates[0];gate=ea.spawn_actor_from_class(unreal.BattleRoadCrossing,transform.translation);gate.set_actor_transform(transform,False,False)
gate.set_actor_label('Irwin Lake BeltLine crossing yield');gate.tags=[unreal.Name('IrwinCrossing')];gate.set_folder_path('Eastside/IrwinLake')
gate.get_editor_property('CrossingArea').set_box_extent(extent);gate.set_editor_property('bAutoCycle',False);gate.set_editor_property('bVehicleGreen',True)
newlanes=[]
for name,points,speed,stop in lanes:
 lane=unreal.BattleRoadTrafficLane();lane.set_editor_property('Name',name);lane.set_editor_property('Points',points);lane.set_editor_property('CruiseSpeed',speed)
 binding=unreal.BattleCarCrossing();binding.set_editor_property('Crossing',gate);binding.set_editor_property('StopDistance',stop);lane.set_editor_property('Crossings',[binding]);newlanes.append(lane)
director=ea.spawn_actor_from_class(unreal.BattleRoadTrafficDirector,unreal.Vector());director.set_actor_label('Irwin Lake road traffic')
director.tags=[unreal.Name('IrwinRoadTraffic')];director.set_folder_path('Eastside/IrwinLake');director.set_editor_property('Lanes',newlanes)
unreal.PiedmontWorldTools.finish_editor_asset_loading()
probes=json.loads((root/'SourceAssets/Terrain/IrwinTraffic/car-lane-probes.json').read_text())['samples'];failures=[]
for row in probes:
 x,y,z=row['xyz'];hit=unreal.PiedmontWorldTools.trace_world_surface(unreal.Vector(x,y,z+150),unreal.Vector(x,y,z-150))
 if not hit or isinstance(hit[1],unreal.Landscape) or abs(hit[0].z-z)>.25:failures.append(row)
assert not failures,len(failures)
assert unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).save_current_level()
report={'main_map_changed':True,'support_passed':True,'support_probes':len(probes),'lanes':2,'max_irwin_cars':6,'other_traffic_retained':retained,'desktop_build_updated':False,'remaining':'Lane paint, landmarks, main rider playthrough, combined performance and packaged acceptance.'}
(root/'Tests/Results/2026-09-12-irwin-traffic-install.json').write_text(json.dumps(report,indent=2)+'\n')
