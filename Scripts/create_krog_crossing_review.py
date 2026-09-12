"""Check car envelope support and create isolated Krog/DeKalb drive review."""
import json,math
from pathlib import Path
import unreal
root=Path(unreal.Paths.project_dir());folder=root/'SourceAssets/Terrain/KrogTraffic'
assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/PiedmontKrogRailReview')
ea=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
# Isolate this car test from unrelated distant traffic simulation.
for actor in ea.get_all_level_actors():
 if isinstance(actor,unreal.BattleRoadTrafficDirector):ea.destroy_actor(actor)
unreal.PiedmontWorldTools.finish_editor_asset_loading()
source=json.loads((folder/'car-lanes.json').read_text());assert source['source_coverage_passed']
probes=json.loads((folder/'car-lane-probes.json').read_text())['samples'];failures=[];counts={}
for row in probes:
 x,y,z=row['xyz'];hit=unreal.PiedmontWorldTools.trace_world_surface(unreal.Vector(x,y,z+250),unreal.Vector(x,y,z-250))
 label=hit[1].get_actor_label() if hit else None;counts[label]=counts.get(label,0)+1
 if not hit or isinstance(hit[1],unreal.Landscape) or abs(hit[0].z-z)>.25:failures.append({'xyz':row['xyz'],'actor':label,'z':hit[0].z if hit else None})
if not failures:
 center=json.loads((folder/'network.json').read_text())['crossing_xyz']
 # Conservative authored yield area around the Krog/DeKalb crossing.
 half=500
 gate=ea.spawn_actor_from_class(unreal.BattleRoadCrossing,unreal.Vector(*center)+unreal.Vector(0,0,90))
 gate.set_actor_label('Krog DeKalb crossing control');gate.tags=[unreal.Name('KrogCrossingReview'),unreal.Name('MonroeCrossingReview')]
 gate.get_editor_property('CrossingArea').set_box_extent(unreal.Vector(half,half,200))
 gate.set_editor_property('bAutoCycle',False);gate.set_editor_property('bVehicleGreen',True)
 bindings=[]
 for row in source['routes']:
  car=ea.spawn_actor_from_class(unreal.BattleRoadCar,unreal.Vector(*row['points_cm'][0])+unreal.Vector(0,0,73.3))
  car.set_actor_label('Krog DeKalb '+row['name']);car.tags=list(car.tags)+[unreal.Name('KrogCarLaneReview'),unreal.Name('MonroeCarLaneReview')]
  # Monroe tag reuses the existing short-route audit without changing runtime.
  car.set_editor_property('route',[unreal.Vector(*p) for p in row['points_cm']]);car.set_editor_property('cruise_speed',row['speed_cm_s'])
  points=row['points_cm'];distance=0;stop=None
  for i,p in enumerate(points[:-1]):
   if i:distance+=math.dist(p[:2],points[i-1][:2])
   q=points[i+1];dx,dy=q[0]-p[0],q[1]-p[1];length=math.hypot(dx,dy);dx/=length;dy/=length
   hx,hy=abs(dx)*236+abs(dy)*114,abs(dy)*236+abs(dx)*114
   if abs(p[0]-center[0])<=half+hx and abs(p[1]-center[1])<=half+hy:stop=distance-100;break
  assert stop is not None and stop>0,row['name']
  binding=unreal.BattleCarCrossing();binding.set_editor_property('crossing',gate);binding.set_editor_property('stop_distance',stop)
  car.set_editor_property('crossings',[binding]);bindings.append({'lane':row['name'],'stop_distance_cm':stop})
 assert len(bindings)==2
 assert unreal.EditorLoadingAndSavingUtils.save_map(unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world(),'/Game/PiedmontRide/Maps/PiedmontKrogCrossingReview')
report={'passed':not failures,'probes':len(probes),'hits':counts,'failures':failures,'main_map_changed':False,'bindings':bindings if not failures else [],'scope':'Two opposing DeKalb cars bound to one authored occupancy-yield region. No real-world signal timing claimed; runtime verification pending.'}
(root/'Tests/Results/2026-09-12-krog-crossing-placement.json').write_text(json.dumps(report,indent=2)+'\n')
