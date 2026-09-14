"""Match source splines to the graded terrain and rebuild candidate path navigation."""
import unreal,json,pathlib,array,sys
root=pathlib.Path(unreal.Paths.project_dir());terrain=root/'SourceAssets/Terrain';m=json.loads((terrain/'terrain-georeference.json').read_text());nx,ny=m['size'];sx,sy,sz=m['unreal_scale'];lx,ly,_=m['unreal_location_cm'];arrays=[]
for path in [terrain/'atlanta-height-tenth-graded.r16',terrain/'LakePathGrading/atlanta-height-lake-paths-candidate.r16']:
 a=array.array('H');a.frombytes(path.read_bytes())
 if sys.byteorder!='little':a.byteswap()
 arrays.append(a)
def delta(x,y):
 gx=(x-lx)/sx;gy=(-y-ly)/sy;i,j=int(gx),int(gy);u,v=gx-i,gy-j
 if not(0<=i<nx-1 and 0<=j<ny-1):return 0
 a,b,c,d=[(int(arrays[1][(j+dy)*nx+i+dx])-int(arrays[0][(j+dy)*nx+i+dx]))*sz/128 for dx,dy in [(0,0),(1,0),(0,1),(1,1)]]
 return a+(b-a)*u+(d-b)*v if u>=v else a+(d-c)*u+(c-a)*v
world=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world();ea=unreal.get_editor_subsystem(unreal.EditorActorSubsystem);rows=[];allpoints=[];affected=[]
for a in ea.get_all_level_actors():
 if not isinstance(a,unreal.PiedmontPathSpline):continue
 pts=[a.centerline.get_location_at_spline_point(i,unreal.SplineCoordinateSpace.WORLD) for i in range(a.centerline.get_number_of_spline_points())]
 if not a.get_editor_property('bBridge') and 'LakePathGrading_v1' not in [str(t) for t in a.tags]:
  shifts=[delta(p.x,p.y) for p in pts]
  if max(map(abs,shifts),default=0)>.001:
   pts=[unreal.Vector(p.x,p.y,p.z+dz) for p,dz in zip(pts,shifts)];a.set_centerline(pts);a.tags=list(a.tags)+[unreal.Name('LakePathGrading_v1')];rows.append({'label':a.get_actor_label(),'id':str(a.get_editor_property('osm_way_id')),'max_shift_cm':max(map(abs,shifts))});affected.append((a,pts))
 allpoints.extend(pts)
lo=[min(getattr(p,k) for p in allpoints) for k in ['x','y','z']];hi=[max(getattr(p,k) for p in allpoints) for k in ['x','y','z']]
started=unreal.PiedmontWorldTools.build_park_navigation(unreal.Vector(*[(a+b)/2 for a,b in zip(lo,hi)]),unreal.Vector(*[(b-a)/2+500 for a,b in zip(lo,hi)]));finished=started and unreal.PiedmontWorldTools.finish_park_navigation_build();assert finished
checks=[]
for a,pts in affected:
 for center in [unreal.Vector(-5050,-1925,0),unreal.Vector(-9000,460,0)]:
  ix=min(range(len(pts)),key=lambda i:(pts[i]-center).length_squared())
  if (pts[ix]-center).length()>1100:continue
  start,end=pts[max(0,ix-6)],pts[min(len(pts)-1,ix+6)];direct=(end-start).length()
  if direct<50:continue
  length=unreal.PiedmontWorldTools.park_route_length(start,end);checks.append({'label':a.get_actor_label(),'start':str(start),'end':str(end),'direct_cm':direct,'navigation_cm':length,'passed':length>0 and length<direct*1.6})
report={'passed':bool(checks) and all(r['passed'] for r in checks),'changed_routes':rows,'checks':checks,'navigation_build_finished':finished,'installed':False}
(root/'Tests/Results/2026-09-13-lake-grading-navigation.json').write_text(json.dumps(report,indent=2)+'\n');assert report['passed'],str(checks)
