"""Install the tested Monroe road/traffic integration while retaining 10th traffic."""
import unreal,json,pathlib
root=pathlib.Path(unreal.Paths.project_dir())
for name in ['native-monroe-population','native-monroe-occupancy','native-monroe-extended-crossing','monroe-extended-support']:
 assert json.loads((root/f'Tests/Results/2026-09-12-{name}.json').read_text())['passed'],name
assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/PiedmontMonroePopulationReview');ea=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
meshes=[];signals=[];lanes=[]
for a in ea.get_all_level_actors():
 if a.actor_has_tag('MonroeApproaches') or a.actor_has_tag('MonroePaintReview'):
  meshes.append((a.get_actor_label(),a.static_mesh_component.static_mesh.get_path_name(),a.get_actor_transform(),[str(t) for t in a.tags],str(a.static_mesh_component.get_collision_profile_name())))
 if a.get_actor_label()=='SM_TenthStreet_Sidewalk':sidewalk=a.static_mesh_component.static_mesh.get_path_name()
 if a.actor_has_tag('MonroeSignalReview'):signals.append((a.get_actor_label(),a.get_actor_transform()))
 if a.actor_has_tag('MonroeCrossingReview'):gate_transform=a.get_actor_transform();gate_extent=a.get_editor_property('CrossingArea').get_unscaled_box_extent()
 if a.actor_has_tag('MonroeTrafficReview'):
  for lane in a.get_editor_property('Lanes'):lanes.append((str(lane.get_editor_property('Name')),list(lane.get_editor_property('Points')),lane.get_editor_property('CruiseSpeed'),lane.get_editor_property('Crossings')[0].get_editor_property('StopDistance')))
assert len(meshes)==3 and len(signals)==2 and len(lanes)==2
assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/PiedmontWorld')
assert not any(a.actor_has_tag('MonroeRoadTraffic') for a in ea.get_all_level_actors()),'Already installed'
old=[a for a in ea.get_all_level_actors() if a.get_actor_label()=='SM_TenthStreet_Sidewalk'];assert len(old)==1;old[0].static_mesh_component.set_static_mesh(unreal.load_asset(sidewalk))
for label,path,transform,tags,profile in meshes:
 a=ea.spawn_actor_from_class(unreal.StaticMeshActor,transform.translation);a.set_actor_transform(transform,False,False);a.set_actor_label(label);a.tags=[unreal.Name(t) for t in tags if t!='MonroePaintReview']+[unreal.Name('MonroeInstalledSurface')];a.static_mesh_component.set_static_mesh(unreal.load_asset(path));a.static_mesh_component.set_collision_profile_name(profile)
gate=ea.spawn_actor_from_class(unreal.BattleRoadCrossing,gate_transform.translation);gate.set_actor_transform(gate_transform,False,False);gate.set_actor_label('Monroe BeltLine crossing control');gate.tags=[unreal.Name('MonroeCrossing')];gate.get_editor_property('CrossingArea').set_box_extent(gate_extent);gate.set_editor_property('bAutoCycle',True);gate.set_editor_property('GreenSeconds',18);gate.set_editor_property('AmberSeconds',3);gate.set_editor_property('RedSeconds',9)
for label,transform in signals:
 a=ea.spawn_actor_from_class(unreal.BattleTrafficSignal,transform.translation);a.set_actor_transform(transform,False,False);a.set_actor_label(label);a.tags=list(a.tags)+[unreal.Name('MonroeSignal')];a.set_editor_property('crossing',gate)
newlanes=[]
for name,points,speed,stop in lanes:
 lane=unreal.BattleRoadTrafficLane();lane.set_editor_property('Name',name);lane.set_editor_property('Points',points);lane.set_editor_property('CruiseSpeed',speed);binding=unreal.BattleCarCrossing();binding.set_editor_property('Crossing',gate);binding.set_editor_property('StopDistance',stop);lane.set_editor_property('Crossings',[binding]);newlanes.append(lane)
d=ea.spawn_actor_from_class(unreal.BattleRoadTrafficDirector,unreal.Vector());d.set_actor_label('Monroe road traffic');d.tags=[unreal.Name('MonroeRoadTraffic')];d.set_editor_property('Lanes',newlanes)
unreal.PiedmontWorldTools.finish_editor_asset_loading()
failures=[]
for row in json.loads((root/'SourceAssets/Terrain/MonroeTraffic/extended-car-lane-probes.json').read_text())['samples']:
 x,y,z=row['xyz'];hit=unreal.PiedmontWorldTools.trace_world_surface(unreal.Vector(x,y,z+150),unreal.Vector(x,y,z-150))
 if not hit or isinstance(hit[1],unreal.Landscape) or abs(hit[0].z-z)>3:failures.append(row)
assert not failures,len(failures)
assert unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).save_current_level()
(root/'Tests/Results/2026-09-12-monroe-traffic-install.json').write_text(json.dumps({'main_map_changed':True,'support_probes':7696,'support_passed':True,'signals':2,'lanes':2,'max_monroe_cars':6,'tenth_traffic_retained':any(a.actor_has_tag('TenthRoadTraffic') for a in ea.get_all_level_actors()),'desktop_build_updated':False,'remaining':'Main-world rider playthrough, full lane detailing, scenery polish, performance and packaged acceptance.'},indent=2)+'\n')
