"""Add collision-free navigation tiles fitted to real ground in an isolated map."""
import unreal,json,math
from pathlib import Path
root=Path(unreal.Paths.project_dir()).resolve();dest='/Game/PiedmontRide/Maps/PiedmontScooterReview'
assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/PiedmontWorld')
unreal.PiedmontWorldTools.finish_editor_asset_loading();ea=unreal.get_editor_subsystem(unreal.EditorActorSubsystem);world=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
assert unreal.EditorLoadingAndSavingUtils.save_map(world,dest)
assert unreal.EditorLoadingAndSavingUtils.load_map(dest)
unreal.PiedmontWorldTools.finish_editor_asset_loading()
r=json.loads((root/'Tests/Results/2026-09-13-scooter-navigation-survey.json').read_text());corridor=next(c for c in r['corridors'] if c['clear_ground_corridor']);site=unreal.Vector(*r['site']);end=unreal.Vector(*corridor['end']);distance=corridor['distance_cm'];direction=unreal.Vector((end.x-site.x)/distance,(end.y-site.y)/distance,0);normal=unreal.Vector(-direction.y,direction.x,0);yaw=math.degrees(math.atan2(direction.y,direction.x))
# A staging pad plus a 2m-wide connector. Tile tops match sampled terrain;
# they export navigation only and cannot collide with players or NPCs.
indices={(a,s) for a in range(-225,226,50) for s in range(-225,226,50)}
indices|={(a,s) for a in range(225,math.ceil(distance+150),50) for s in (-75,-25,25,75)}
tiles=[];samples=[]
for along,side in sorted(indices):
 at=site+direction*along+normal*side;hit=unreal.PiedmontWorldTools.trace_world_surface(at+unreal.Vector(0,0,300),at-unreal.Vector(0,0,300));assert hit,(along,side)
 p,actor=hit
 assert not unreal.PiedmontWorldTools.trace_world_surface(p+unreal.Vector(0,0,5000),p+unreal.Vector(0,0,190)),('roof',along,side)
 assert not unreal.PiedmontWorldTools.trace_world_surface(p+unreal.Vector(0,0,185),p+unreal.Vector(0,0,40),30),('clearance',along,side)
 samples.append({'xyz':[p.x,p.y,p.z],'actor':actor.get_actor_label()})
 tiles.append(unreal.Transform(location=p-unreal.Vector(0,0,1),rotation=unreal.Rotator(yaw=yaw),scale=unreal.Vector(.51,.51,.02)))
patch=unreal.PiedmontWorldTools.create_ground_navigation_tiles(tiles);assert patch
for s in samples:
 p=unreal.Vector(*s['xyz']);hit=unreal.PiedmontWorldTools.trace_world_surface(p+unreal.Vector(0,0,100),p-unreal.Vector(0,0,100));assert hit and hit[1]!=patch and abs(hit[0].z-p.z)<.1
points=[]
for a in ea.get_all_level_actors():
 if isinstance(a,unreal.PiedmontPathSpline):points += [a.centerline.get_location_at_spline_point(i,unreal.SplineCoordinateSpace.WORLD) for i in range(a.centerline.get_number_of_spline_points())]
points += [unreal.Vector(*s['xyz']) for s in samples]
lo=[min(getattr(p,k) for p in points) for k in ('x','y','z')];hi=[max(getattr(p,k) for p in points) for k in ('x','y','z')]
assert unreal.PiedmontWorldTools.build_park_navigation(unreal.Vector(*[(a+b)/2 for a,b in zip(lo,hi)]),unreal.Vector(*[(b-a)/2+500 for a,b in zip(lo,hi)]))
assert unreal.PiedmontWorldTools.finish_park_navigation_build()
start=unreal.PiedmontWorldTools.project_park_navigation(site);goal=unreal.PiedmontWorldTools.project_park_navigation(end);length=unreal.PiedmontWorldTools.park_route_length(start,goal) if start and goal else -1
report={'tiles':len(tiles),'samples':samples,'site_projected':bool(start),'target_projected':bool(goal),'route_length_cm':length,'passed':bool(start and goal and length>0),'main_map_changed':False,'scope':'Local navigation-only tile export, native projection and route query; no physical collision change. Runtime visit and full navigation regression checks still required.'}
report['saved']=bool(unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).save_current_level())
(root/'Tests/Results/2026-09-13-scooter-navigation-review.json').write_text(json.dumps(report,indent=2)+'\n');assert report['passed'],report
