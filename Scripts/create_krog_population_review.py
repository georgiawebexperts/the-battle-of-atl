"""Exercise natural traffic on the reviewed Krog geometry and controls."""
import unreal,json,pathlib
root=pathlib.Path(unreal.Paths.project_dir());assert json.loads((root/'Tests/Results/2026-09-12-native-krog-occupancy.json').read_text())['passed'];assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/PiedmontKrogCrossingReview');ea=unreal.get_editor_subsystem(unreal.EditorActorSubsystem);lanes=[]
for a in ea.get_all_level_actors():
 if a.actor_has_tag('TenthRoadTraffic'):ea.destroy_actor(a)
 if a.actor_has_tag('KrogCrossingReview'):
  a.set_editor_property('bAutoCycle',False);a.set_editor_property('bVehicleGreen',True)
 if a.actor_has_tag('KrogCarLaneReview'):
  lane=unreal.BattleRoadTrafficLane();lane.set_editor_property('Name',a.get_actor_label());lane.set_editor_property('Points',a.get_editor_property('route'));lane.set_editor_property('CruiseSpeed',500);lane.set_editor_property('Crossings',a.get_editor_property('crossings'));lanes.append(lane);ea.destroy_actor(a)
assert len(lanes)==2
# IrwinTrafficReview selects the existing audit's west-facing offscreen camera.
# It also leaves occupied-crossing proof to the separate occupancy test.
d=ea.spawn_actor_from_class(unreal.BattleRoadTrafficDirector,unreal.Vector());d.set_editor_property('Lanes',lanes);d.set_actor_label('Krog live traffic review');d.tags=[unreal.Name('KrogTrafficReview'),unreal.Name('TenthRoadTrafficReview'),unreal.Name('IrwinTrafficReview')]
assert unreal.EditorLoadingAndSavingUtils.save_map(unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world(),'/Game/PiedmontRide/Maps/PiedmontKrogPopulationReview')
(root/'Tests/Results/2026-09-12-krog-population-placement.json').write_text(json.dumps({'lanes':2,'max_cars':6,'interval_seconds':8,'audit_camera':'Existing Irwin fixture at (13000,100850,2500), facing west; Krog is behind the view','control':'Yield to occupied crossing; no timed signal','main_map_changed':False,'other_traffic_removed_for_isolated_test':True},indent=2)+'\n')
