"""Connect the actual lanes and installed crossing to a live traffic director."""
import unreal,json,pathlib,math
root=pathlib.Path(unreal.Paths.project_dir());install='-BattleInstallRoadTraffic' in unreal.SystemLibrary.get_command_line()
if install:
 for name in ['2026-09-12-native-road-traffic-director','2026-09-12-native-traffic-population']:
  assert json.loads((root/'Tests/Results'/f'{name}.json').read_text())['passed'],name
assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/PiedmontWorld');ea=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
for a in ea.get_all_level_actors():
 if a.actor_has_tag('TenthRoadTraffic'):
  assert not install,'Traffic already installed'
  ea.destroy_actor(a)
gate=next(a for a in ea.get_all_level_actors() if a.actor_has_tag('TenthPiedmontBikeCrossing'));assert isinstance(gate,unreal.BattleRoadCrossing)
lanes=[]
for row in json.loads((root/'SourceAssets/Terrain/TenthStreetGraded/car-lanes.json').read_text())['routes']:
 lane=unreal.BattleRoadTrafficLane();lane.set_editor_property('Name',row['name']);lane.set_editor_property('Points',[unreal.Vector(*p) for p in row['points_cm']]);lane.set_editor_property('CruiseSpeed',row['speed_cm_s'])
 if row['crossing_point_indices']:
  first=min(row['crossing_point_indices']);stop=sum(math.dist(a[:2],b[:2]) for a,b in zip(row['points_cm'][:first],row['points_cm'][1:first+1]))-100
  binding=unreal.BattleCarCrossing();binding.set_editor_property('Crossing',gate);binding.set_editor_property('StopDistance',stop);lane.set_editor_property('Crossings',[binding])
 lanes.append(lane)
director=ea.spawn_actor_from_class(unreal.BattleRoadTrafficDirector,unreal.Vector());director.set_editor_property('Lanes',lanes);director.set_actor_label('10th Street live road traffic');director.tags=[unreal.Name('TenthRoadTraffic' if install else 'TenthRoadTrafficReview')];director.set_folder_path('Midtown/Traffic')
world=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world();assert unreal.EditorLoadingAndSavingUtils.save_map(world,'/Game/PiedmontRide/Maps/PiedmontWorld' if install else '/Game/PiedmontRide/Maps/PiedmontTrafficPopulationReview')
(root/('Tests/Results/2026-09-12-traffic-population-install.json' if install else 'Tests/Results/2026-09-12-traffic-population-review-map.json')).write_text(json.dumps({'lanes':len(lanes),'max_cars':6,'max_per_lane':3,'spawn_interval_seconds':8,'main_map_changed':install,'review_population_validated':install,'desktop_build_updated':False},indent=2)+'\n')
