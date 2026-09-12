"""Native collision probes and route-height correction in isolated review world."""
import unreal,json,pathlib,array,sys
root=pathlib.Path(unreal.Paths.project_dir());terrain=root/'SourceAssets/Terrain';ea=unreal.get_editor_subsystem(unreal.EditorActorSubsystem);world=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
meta=json.loads((terrain/'terrain-georeference.json').read_text());width,height=meta['size'];sx,sy,sz=meta['unreal_scale'];lx,ly,_=meta['unreal_location_cm']
arrays=[]
for name in ['atlanta-height-krog.r16','atlanta-height-tenth-graded.r16']:
 a=array.array('H');a.frombytes((terrain/name).read_bytes())
 if sys.byteorder!='little':a.byteswap()
 arrays.append(a)
def delta(x,y):
 gx=(x-lx)/sx;gy=(-y-ly)/sy;ix=int(gx);iy=int(gy);dx=gx-ix;dy=gy-iy
 if not(0<=ix<width-1 and 0<=iy<height-1):return 0
 a,b,c,d=[(int(arrays[1][v*width+u])-int(arrays[0][v*width+u]))*sz/128 for u,v in [(ix,iy),(ix+1,iy),(ix,iy+1),(ix+1,iy+1)]]
 return a+(b-a)*dx+(d-b)*dy if dx>=dy else a+(d-c)*dx+(c-a)*dy
routes=[]
for actor in ea.get_all_level_actors():
 if not isinstance(actor,unreal.PiedmontPathSpline) or actor.get_editor_property("bBridge") or 'TenthStreetGraded_v1' in [str(t) for t in actor.tags]:continue
 points=[];changes=[]
 for i in range(actor.centerline.get_number_of_spline_points()):
  p=actor.centerline.get_location_at_spline_point(i,unreal.SplineCoordinateSpace.WORLD);dz=delta(p.x,p.y);points.append(unreal.Vector(p.x,p.y,p.z+dz));changes.append(abs(dz))
 if max(changes,default=0)>.001:actor.set_centerline(points);actor.tags=list(actor.tags)+[unreal.Name('TenthStreetGraded_v1')];routes.append({'label':actor.get_actor_label(),'changed_points':sum(d>.001 for d in changes),'max_shift_cm':max(changes)})
results=[]
for probe in json.loads((terrain/'TenthStreetGraded/collision-samples.json').read_text())['samples']:
 x,y,z=probe['xyz'];top=unreal.Vector(x,y,z+150);bottom=unreal.Vector(x,y,z-150)
 hit=unreal.PiedmontWorldTools.trace_world_surface(top,bottom)
 cap=unreal.SystemLibrary.capsule_trace_single(world,top,bottom,32,96,unreal.TraceTypeQuery.ECC_VISIBILITY,False,[],unreal.DrawDebugTrace.NONE)
 results.append({'mesh':probe['mesh'],'xyz':probe['xyz'],'height_error_cm':None if not hit else hit[0].z-z,'hit_actor':None if not hit else hit[1].get_actor_label(),'capsule_hit':cap is not None})
failures=[r for r in results if r['height_error_cm'] is None or abs(r['height_error_cm'])>8 or not r['capsule_hit']]
report={'route_updates':routes,'probe_count':len(results),'failures':failures,'passed':not failures,'scope':'Vertical native line/capsule probes and transient route-point adjustment. Moving bike and rebuilt navmesh not verified.','map_saved':False}
(root/'Tests/Results/2026-09-11-graded-native-collision.json').write_text(json.dumps(report,indent=2)+'\n')
# Rebuild actual path navigation after geometry and source routes agree.
route_points=[a.centerline.get_location_at_spline_point(i,unreal.SplineCoordinateSpace.WORLD) for a in ea.get_all_level_actors() if isinstance(a,unreal.PiedmontPathSpline) for i in range(a.centerline.get_number_of_spline_points())]
lo=[min(getattr(p,k) for p in route_points) for k in ['x','y','z']];hi=[max(getattr(p,k) for p in route_points) for k in ['x','y','z']]
nav_started=unreal.PiedmontWorldTools.build_park_navigation(unreal.Vector(*[(a+b)/2 for a,b in zip(lo,hi)]),unreal.Vector(*[(b-a)/2+500 for a,b in zip(lo,hi)]))
nav_finished=nav_started and unreal.PiedmontWorldTools.finish_park_navigation_build()
nav_rows=[]
for actor in ea.get_all_level_actors():
 if not isinstance(actor,unreal.PiedmontPathSpline) or actor.get_editor_property('bBridge'):continue
 points=[actor.centerline.get_location_at_spline_point(i,unreal.SplineCoordinateSpace.WORLD) for i in range(actor.centerline.get_number_of_spline_points())]
 affected=[p for p in points if abs(delta(p.x,p.y))>.01]
 if len(affected)<2:continue
 start=affected[0];end=affected[-1]
 if (start-end).length()<100:continue
 start_nav=unreal.PiedmontWorldTools.project_park_navigation(start);end_nav=unreal.PiedmontWorldTools.project_park_navigation(end)
 length=unreal.PiedmontWorldTools.park_route_length(start,end) if start_nav and end_nav else -1
 nav_rows.append({'label':actor.get_actor_label(),'start':[start.x,start.y,start.z],'end':[end.x,end.y,end.z],'start_projected':bool(start_nav),'end_projected':bool(end_nav),'route_length_cm':length})
report['navigation']={'build_started':nav_started,'build_finished':nav_finished,'routes':nav_rows,'failed_routes':sum(r['route_length_cm']<0 for r in nav_rows)}
report['passed']=not failures and nav_finished and bool(nav_rows) and all(r['route_length_cm']>=0 for r in nav_rows)
report['scope']='Vertical native collision probes, route-height updates and rebuilt navigation. Moving bike transitions remain unverified.'
(root/'Tests/Results/2026-09-11-graded-native-collision.json').write_text(json.dumps(report,indent=2)+'\n')
