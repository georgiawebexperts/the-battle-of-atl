"""Place a mapped crossing, signal and opposing cars in an isolated review map."""
import unreal,json,pathlib,math
root=pathlib.Path(unreal.Paths.project_dir());folder=root/'SourceAssets/Terrain/TenthStreetGraded'
assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/PiedmontWorld')
unreal.PiedmontWorldTools.finish_editor_asset_loading();ea=unreal.get_editor_subsystem(unreal.EditorActorSubsystem);world=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
# Rebuild only our crossing in this transient review world after installation.
for existing in ea.get_all_level_actors():
 if existing.actor_has_tag('TenthPiedmontBikeCrossing') or existing.actor_has_tag('TenthPiedmontBikeSignal'):ea.destroy_actor(existing)
# Keep the pole completely off motor/cycle paving and seat the foot into terrain.
x,y=-20800,12050;ground=[]
for dx,dy in [(0,0),(-20,-20),(20,-20),(-20,20),(20,20)]:
 hit=unreal.PiedmontWorldTools.trace_world_surface(unreal.Vector(x+dx,y+dy,3000),unreal.Vector(x+dx,y+dy,-3000));assert hit and isinstance(hit[1],unreal.Landscape);ground.append(hit[0].z)
assert max(ground)-min(ground)<5
crossing=next(r for r in json.loads((folder/'network.json').read_text())['cycle_track'] if r['tags'].get('cycleway')=='crossing' and min(math.dist(p[:2],[-21672.022,12403.509]) for p in r['points_cm'])<1)
points=crossing['points_cm'];lo=[min(p[k] for p in points) for k in range(3)];hi=[max(p[k] for p in points) for k in range(3)];center=[(a+b)/2 for a,b in zip(lo,hi)];center[2]+=100
Gate=ea.spawn_actor_from_class(unreal.BattleRoadCrossing,unreal.Vector(*center));Gate.set_actor_label('10th Street mapped bike crossing');Gate.get_editor_property('CrossingArea').set_box_extent(unreal.Vector((hi[0]-lo[0])/2+90,(hi[1]-lo[1])/2+90,160));Gate.set_editor_property('bAutoCycle',True);Gate.tags=[unreal.Name('TenthSignalCrossingReview')]
Signal=ea.spawn_actor_from_class(unreal.BattleTrafficSignal,unreal.Vector(x,y,min(ground)),unreal.Rotator(yaw=180));Signal.set_actor_label('10th Street westbound crossing signal');Signal.set_editor_property('crossing',Gate)
rows=[]
for route in json.loads((folder/'car-lanes.json').read_text())['routes']:
 car=ea.spawn_actor_from_class(unreal.BattleRoadCar,unreal.Vector(*route['points_cm'][0])+unreal.Vector(0,0,73.3));car.set_actor_label('10th signal review '+route['name']);car.set_editor_property('route',[unreal.Vector(*p) for p in route['points_cm']]);car.tags=list(car.tags)+[unreal.Name('TenthCarLaneReview')]
 if route['crossing_point_indices']:
  first=min(route['crossing_point_indices']);stop=sum(math.dist(a[:2],b[:2]) for a,b in zip(route['points_cm'][:first],route['points_cm'][1:first+1]))-100
  binding=unreal.BattleCarCrossing();binding.set_editor_property('crossing',Gate);binding.set_editor_property('stop_distance',stop);car.set_editor_property('crossings',[binding]);rows.append({'lane':route['name'],'stop_distance_cm':stop})
for a in [Gate,Signal]:a.set_folder_path('Midtown/TrafficReview')
assert unreal.EditorLoadingAndSavingUtils.save_map(world,'/Game/PiedmontRide/Maps/PiedmontSignalCrossingReview')
(root/'Tests/Results/2026-09-12-signal-crossing-placement.json').write_text(json.dumps({'signal_xyz':[x,y,min(ground)],'support_z':ground,'crossing_center':center,'bindings':rows,'main_map_changed':False,'approach_visibility_validated':False},indent=2)+'\n')
