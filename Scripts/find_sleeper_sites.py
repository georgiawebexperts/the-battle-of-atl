"""Find clear level ground beside mapped park paths; do not place or save actors."""
import json,math,pathlib,unreal
root=pathlib.Path(unreal.Paths.project_dir())
assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/PiedmontWorld')
ea=unreal.get_editor_subsystem(unreal.EditorActorSubsystem);world=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
paths=[a for a in ea.get_all_level_actors() if isinstance(a,unreal.PiedmontPathSpline) and not a.get_editor_property('bBridge')]
def ground(x,y):return unreal.PiedmontWorldTools.trace_world_surface(unreal.Vector(x,y,7000),unreal.Vector(x,y,-7000))
water=[a for a in ea.get_all_level_actors() if isinstance(a,unreal.PiedmontWaterHazard)]
def wet(point):
 for a in water:
  p=unreal.MathLibrary.inverse_transform_location(a.get_actor_transform(),point)
  if p.z>a.detection_height:continue
  def inside(ring):
   result=False
   for i in range(len(ring)):
    x,y=ring[i],ring[i-1]
    if (x.y>p.y)!=(y.y>p.y) and p.x<(y.x-x.x)*(p.y-x.y)/(y.y-x.y)+x.x:result=not result
   return result
  if inside(a.polygon) and not inside(a.island_polygon):return True
 return False
rows=[];tested=0;labels=sorted(a.get_actor_label() for a in paths)
for path in paths:
 spline=path.centerline;length=spline.get_spline_length();width=path.get_editor_property('WidthCm')
 for distance in range(500,int(length),500):
  p=spline.get_location_at_distance_along_spline(distance,unreal.SplineCoordinateSpace.WORLD)
  if not(-22000<p.y<9500 and -25000<p.x<12000):continue
  d=spline.get_direction_at_distance_along_spline(distance,unreal.SplineCoordinateSpace.WORLD);mag=math.hypot(d.x,d.y)
  if mag<.01:continue
  for side in [-1,1]:
   tested+=1;offset=width*.5+180
   hit=ground(p.x-side*d.y/mag*offset,p.y+side*d.x/mag*offset)
   if not hit:continue
   point,actor=hit
   if wet(point+unreal.Vector(0,0,90)):continue
   if not(isinstance(actor,unreal.Landscape) or actor.actor_has_tag('RidePath')):continue
   if any((point-r['vector']).length()<2200 for r in rows):continue
   clear=True
   for other in paths:
    near=other.centerline.find_location_closest_to_world_location(point,unreal.SplineCoordinateSpace.WORLD)
    if math.hypot(near.x-point.x,near.y-point.y)<other.get_editor_property('WidthCm')*.5+110:clear=False;break
   if not clear:continue
   nav=unreal.PiedmontWorldTools.project_park_navigation(point)
   if not nav or (nav-point).length()>150:continue
   for yaw in [0,90,180,270]:
    angle=math.radians(yaw-90);x=-261.387456;y=39.753575
    landing=point+unreal.Vector(x*math.cos(angle)-y*math.sin(angle),x*math.sin(angle)+y*math.cos(angle),0)
    if unreal.SystemLibrary.capsule_trace_single(world,point+unreal.Vector(0,0,105),landing+unreal.Vector(0,0,105),100,100,unreal.TraceTypeQuery.ECC_VISIBILITY,False,[],unreal.DrawDebugTrace.NONE):continue
    probes=[]
    for i in range(7):
     q=point+(landing-point)*(i/6);g=ground(q.x,q.y)
     if not g or abs(g[0].z-point.z)>5 or wet(g[0]+unreal.Vector(0,0,90)):break
     blocked_path=False
     for other in paths:
      near=other.centerline.find_location_closest_to_world_location(g[0],unreal.SplineCoordinateSpace.WORLD)
      if math.hypot(near.x-g[0].x,near.y-g[0].y)<other.get_editor_property('WidthCm')*.5+100:blocked_path=True;break
     if blocked_path:break
     probes.append([g[0].x,g[0].y,g[0].z])
    if len(probes)!=7:continue
    rows.append({'vector':point,'path':path.get_actor_label(),'xyz':[point.x,point.y,point.z],'yaw':yaw,'landing_xyz':[landing.x,landing.y,landing.z],'ground_samples':probes,'surface':actor.get_actor_label()});break
for row in rows:del row['vector']
report={'candidates':rows,'tested_path_sides':tested,'path_labels':labels,'map_saved':False,'scope':'Editor trace/nav candidate scan; runtime benches, crowd, dynamic obstacles and final gameplay placement not validated.'}
(root/'Tests/Results/2026-09-12-sleeper-sites.json').write_text(json.dumps(report,indent=2)+'\n')
print('SLEEPER_SITE_CANDIDATES',len(rows))
