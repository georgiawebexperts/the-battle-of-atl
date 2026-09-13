"""Read-only native survey from incident grass to existing walkable network."""
import unreal,json,math
from pathlib import Path
root=Path(unreal.Paths.project_dir()).resolve()
assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/PiedmontWorld')
unreal.PiedmontWorldTools.finish_editor_asset_loading()
site=unreal.Vector(30349.800013,114057.877225,1110.508188);options=[];seen=set()
ea=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
for actor in ea.get_all_level_actors():
 if not isinstance(actor,unreal.PiedmontPathSpline) or not any(str(t).startswith('BattleKrog_') for t in actor.tags):continue
 spline=actor.centerline
 for i in range(math.ceil(spline.get_spline_length()/50)+1):
  raw=spline.get_location_at_distance_along_spline(min(i*50,spline.get_spline_length()),unreal.SplineCoordinateSpace.WORLD)
  if math.hypot(raw.x-site.x,raw.y-site.y)>3500:continue
  p=unreal.PiedmontWorldTools.project_park_navigation(raw)
  if not p:continue
  key=(round(p.x/10),round(p.y/10),round(p.z/10))
  if key in seen:continue
  seen.add(key);distance=math.hypot(p.x-site.x,p.y-site.y)
  if distance>=300:options.append((distance,p))
end_surfaces=[];filtered=[]
for distance,p in options:
 hit=unreal.PiedmontWorldTools.trace_world_surface(p+unreal.Vector(0,0,100),p-unreal.Vector(0,0,200))
 label=hit[1].get_actor_label() if hit else 'missing'
 end_surfaces.append({'distance_cm':distance,'end':[p.x,p.y,p.z],'actor':label})
 if hit and abs(hit[0].z-p.z)<20:filtered.append((distance,p))
options=sorted(filtered,key=lambda v:v[0]);rows=[]
for distance,end in options[:30]:
 dx=end.x-site.x;dy=end.y-site.y;normal=unreal.Vector(-dy/distance,dx/distance,0);samples=[];good=True
 for i in range(math.ceil(distance/50)+1):
  f=min(1,i*50/distance);center=site+unreal.Vector(dx*f,dy*f,0)
  for side in (-110,0,110):
   at=center+normal*side;hit=unreal.PiedmontWorldTools.trace_world_surface(at+unreal.Vector(0,0,300),at-unreal.Vector(0,0,300))
   if not hit:good=False;continue
   point,actor=hit;label=actor.get_actor_label();clear=not unreal.PiedmontWorldTools.trace_world_surface(point+unreal.Vector(0,0,185),point+unreal.Vector(0,0,40),30)
   roof=unreal.PiedmontWorldTools.trace_world_surface(point+unreal.Vector(0,0,5000),point+unreal.Vector(0,0,190));clear=clear and not roof
   terrain='USGS' in label
   samples.append({'xyz':[point.x,point.y,point.z],'actor':label,'clear':clear});good=good and clear and (terrain or actor.actor_has_tag('RidePath') or actor.actor_has_tag('RideDirt'))
 rows.append({'distance_cm':distance,'end':[end.x,end.y,end.z],'clear_ground_corridor':good,'samples':samples})
report={'site':[site.x,site.y,site.z],'candidate_count':len(options),'end_surfaces':end_surfaces,'corridors':rows,'scope':'Native 50cm-spaced, 220cm-wide ground and vertical radius30 clearance probes. Does not establish crossing safety or navigation connectivity.'}
(root/'Tests/Results/2026-09-13-scooter-navigation-survey.json').write_text(json.dumps(report,indent=2)+'\n')
