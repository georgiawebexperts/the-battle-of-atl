"""Find a straight, supported hill on the current world's authoritative splines."""
import unreal,json,math
from pathlib import Path
root=Path(unreal.Paths.project_dir())
assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/PiedmontWorld')
unreal.PiedmontWorldTools.finish_editor_asset_loading()
ea=unreal.get_editor_subsystem(unreal.EditorActorSubsystem);rows=[]
for a in ea.get_all_level_actors():
 if not isinstance(a,unreal.PiedmontPathSpline):continue
 c=a.get_editor_property('centerline');length=c.get_spline_length()
 for start in range(0,int(length)-3000,500):
  pts=[c.get_location_at_distance_along_spline(start+i*250,unreal.SplineCoordinateSpace.WORLD) for i in range(13)]
  p,q=pts[0],pts[-1];dx,dy=q.x-p.x,q.y-p.y;l=math.hypot(dx,dy)
  if l<2950 or max(abs(dx*(v.y-p.y)-dy*(v.x-p.x))/l for v in pts)>65:continue
  samples=[]
  for i in range(13):
   x,y=p.x+dx*i/12,p.y+dy*i/12
   hit=unreal.PiedmontWorldTools.trace_world_surface(unreal.Vector(x,y,2000),unreal.Vector(x,y,-2500))
   if not hit or not hit[1].get_actor_label().startswith('Park pavement'):break
   samples.append([x,y,hit[0].z])
  if len(samples)!=13:continue
  grade=(samples[-1][2]-samples[0][2])/l
  if abs(grade)<.035:continue
  rows.append(dict(actor=a.get_actor_label(),osm=a.get_editor_property('osm_way_id'),grade=grade,length=l,points=samples))
rows.sort(key=lambda x:-abs(x['grade']))
(root/'Tests/Results/2026-09-13-native-hill-candidates.json').write_text(json.dumps(rows[:10],indent=2)+'\n')
