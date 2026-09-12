"""Check car envelope support and create isolated Krog/DeKalb drive review."""
import json
from pathlib import Path
import unreal
root=Path(unreal.Paths.project_dir());folder=root/'SourceAssets/Terrain/KrogTraffic'
assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/PiedmontKrogRoadReview')
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
 for row in source['routes']:
  car=ea.spawn_actor_from_class(unreal.BattleRoadCar,unreal.Vector(*row['points_cm'][0])+unreal.Vector(0,0,73.3))
  car.set_actor_label('Krog DeKalb '+row['name']);car.tags=list(car.tags)+[unreal.Name('KrogCarLaneReview'),unreal.Name('MonroeCarLaneReview')]
  # Monroe tag reuses the existing short-route audit without changing runtime.
  car.set_editor_property('route',[unreal.Vector(*p) for p in row['points_cm']]);car.set_editor_property('cruise_speed',row['speed_cm_s'])
 assert unreal.EditorLoadingAndSavingUtils.save_map(unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world(),'/Game/PiedmontRide/Maps/PiedmontKrogLaneReview')
report={'passed':not failures,'probes':len(probes),'hits':counts,'failures':failures,'main_map_changed':False,'scope':'Car footprint and wheel sample support only; actual movement tested separately.'}
(root/'Tests/Results/2026-09-12-krog-lane-support.json').write_text(json.dumps(report,indent=2)+'\n')
