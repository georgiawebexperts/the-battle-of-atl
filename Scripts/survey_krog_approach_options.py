"""Survey ground under offset crossing approaches, without changing the map."""
import unreal,json,math
from pathlib import Path
root=Path(unreal.Paths.project_dir()).resolve()
assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/PiedmontWorld')
unreal.PiedmontWorldTools.finish_editor_asset_loading()
ea=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
a=next(a for a in ea.get_all_level_actors() if isinstance(a,unreal.PiedmontPathSpline) and a.actor_has_tag('BattleKrog_2'))
points=[a.centerline.get_location_at_spline_point(i,unreal.SplineCoordinateSpace.WORLD) for i in range(a.centerline.get_number_of_spline_points())]
length=[0.]
for p,q in zip(points,points[1:]):length.append(length[-1]+math.hypot(q.x-p.x,q.y-p.y))
normal=unreal.Vector(-.31636,-.94864,0) # Outer/north side of the westbound DeKalb lane.
rows=[]
for offset in (100,150,180,220):
 samples=[];adjusted=[]
 for p,d in zip(points,length):
  remaining=length[-1]-d
  def smooth(t):t=max(0,min(1,t));return t*t*(3-2*t)
  weight=smooth((2400-remaining)/700)*smooth(remaining/700)
  q=p+normal*(offset*weight)
  hit=unreal.PiedmontWorldTools.trace_world_surface(q+unreal.Vector(0,0,400),q-unreal.Vector(0,0,400))
  adjusted.append([q.x,q.y,hit[0].z if hit else q.z])
  if weight>0:
   for side in (-62,0,62):
    at=q+normal*side;h=unreal.PiedmontWorldTools.trace_world_surface(at+unreal.Vector(0,0,400),at-unreal.Vector(0,0,400))
    samples.append({'xyz':[at.x,at.y,h[0].z if h else at.z],'ground':bool(h),'paved':bool(h and h[1].actor_has_tag('RidePath')),'actor':h[1].get_actor_label() if h else None})
 rows.append({'offset_cm':offset,'points':adjusted,'samples':samples,'missing':sum(not s['ground'] for s in samples),'unpaved':sum(not s['paved'] for s in samples)})
(root/'Tests/Results/2026-09-13-krog-approach-options.json').write_text(json.dumps({'options':rows,'main_map_changed':False,'scope':'Native centre/edge surface survey of authored route offset candidates; no runtime clearance, visual acceptance or map change.'},indent=2)+'\n')
