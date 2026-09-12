"""Measure walking-sized capsule clearance along exterior door approaches."""
import json,math
from pathlib import Path
import unreal
root=Path(unreal.Paths.project_dir());assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/PiedmontIrwinSidewalkReview')
unreal.PiedmontWorldTools.finish_editor_asset_loading();world=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
rows=[]
for entry in json.loads((root/'Tests/Results/2026-09-12-irwin-entrance-landings.json').read_text())['entrances']:
 x,y,z=entry['door_xyz'];ox,oy=entry['outward_xy'];samples=[];failures=[];previous=None;min_headroom=1000
 for d in range(40,501,20):
  px,py=x+ox*d,y+oy*d
  floor=unreal.PiedmontWorldTools.trace_world_surface(unreal.Vector(px,py,z+110),unreal.Vector(px,py,z-600))
  if not floor:failures.append({'distance_cm':d,'kind':'missing_floor'});previous=None;continue
  fz=floor[0].z;center=unreal.Vector(px,py,fz+95)
  # A standing capsule with a small floor margin. This does not simulate
  # CharacterMovement step-up, navigation or animation.
  start=previous if previous is not None else center
  blocked=unreal.SystemLibrary.capsule_trace_single(world,start,center+unreal.Vector(0,0,.1),32,90,unreal.TraceTypeQuery.ECC_VISIBILITY,False,[],unreal.DrawDebugTrace.NONE)
  if blocked:failures.append({'distance_cm':d,'kind':'capsule_blocked'})
  for side in [-25,0,25]:
   hx,hy=px-oy*side,py+ox*side
   overhead=unreal.PiedmontWorldTools.trace_world_surface(unreal.Vector(hx,hy,fz+15),unreal.Vector(hx,hy,fz+230))
   if overhead:
    clearance=overhead[0].z-fz;min_headroom=min(min_headroom,clearance)
    if clearance<190:failures.append({'distance_cm':d,'kind':'headroom','height_cm':clearance,'actor':overhead[1].get_actor_label()})
  samples.append({'distance_cm':d,'xyz':[px,py,fz],'floor_actor':floor[1].get_actor_label()});previous=center
 rows.append({'osm_way':entry['osm_way'],'passed':not failures,'samples':samples,'failures':failures,'min_detected_headroom_cm':None if min_headroom==1000 else min_headroom})
report={'passed':all(r['passed'] for r in rows),'routes':rows,'capsule_radius_cm':32,'capsule_half_height_cm':90,'main_map_changed':False,'scope':'Static walking-sized capsule sweeps and sampled headroom only. CharacterMovement traversal, accessibility and continuous surface comfort unverified.'}
(root/'Tests/Results/2026-09-12-irwin-approach-clearance.json').write_text(json.dumps(report,indent=2)+'\n')
