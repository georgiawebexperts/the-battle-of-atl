"""Exercise natural traffic on the reviewed Monroe geometry and controls."""
import unreal,json,pathlib
root=pathlib.Path(unreal.Paths.project_dir());assert json.loads((root/'Tests/Results/2026-09-12-native-monroe-occupancy.json').read_text())['passed'];assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/PiedmontMonroeExtendedCrossingReview');ea=unreal.get_editor_subsystem(unreal.EditorActorSubsystem);lanes=[]
for a in ea.get_all_level_actors():
 if a.actor_has_tag('TenthRoadTraffic'):ea.destroy_actor(a)
 if a.actor_has_tag('MonroeCrossingReview'):
  a.set_editor_property('bAutoCycle',True);a.set_editor_property('GreenSeconds',18);a.set_editor_property('AmberSeconds',3);a.set_editor_property('RedSeconds',9)
 if a.actor_has_tag('MonroeCarLaneReview'):
  lane=unreal.BattleRoadTrafficLane();lane.set_editor_property('Name',a.get_actor_label());lane.set_editor_property('Points',a.get_editor_property('route'));lane.set_editor_property('CruiseSpeed',500);lane.set_editor_property('Crossings',a.get_editor_property('crossings'));lanes.append(lane);ea.destroy_actor(a)
assert len(lanes)==2
d=ea.spawn_actor_from_class(unreal.BattleRoadTrafficDirector,unreal.Vector());d.set_editor_property('Lanes',lanes);d.set_actor_label('Monroe live traffic review');d.tags=[unreal.Name('MonroeTrafficReview'),unreal.Name('TenthRoadTrafficReview')]
assert unreal.EditorLoadingAndSavingUtils.save_map(unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world(),'/Game/PiedmontRide/Maps/PiedmontMonroePopulationReview')
(root/'Tests/Results/2026-09-12-monroe-population-placement.json').write_text(json.dumps({'lanes':2,'max_cars':6,'interval_seconds':8,'signal_cycle':[18,3,9],'main_map_changed':False,'other_traffic_removed_for_isolated_test':True},indent=2)+'\n')
