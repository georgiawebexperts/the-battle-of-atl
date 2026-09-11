"""Convert the legacy source-space park into Unreal east/south/up coordinates.
Original meshes/R16 remain in source space. The conversion is applied at the
actor boundary and checked against pre-conversion collision before any save.
"""
import unreal,json,pathlib,math,sys
root=pathlib.Path(unreal.Paths.project_dir())
sys.path.insert(0,str(root/'Scripts'))
from battle_geography import source_vector,WORLD_MARKER
assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/PiedmontWorld')
unreal.PiedmontWorldTools.finish_editor_asset_loading()
world=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
ea=unreal.get_editor_subsystem(unreal.EditorActorSubsystem);actors=ea.get_all_level_actors()
settings=world.get_world_settings();marker=unreal.Name(WORLD_MARKER)
assert marker not in settings.tags,'Map already converted; refusing double conversion'
def mirror(v):return source_vector([v.x,v.y,v.z])
def xyz(v):return [v.x,v.y,v.z]
splines=[a for a in actors if isinstance(a,unreal.PiedmontPathSpline)]
samples=[]
def sample(p,kind):
 hit=unreal.PiedmontWorldTools.trace_world_surface(p+unreal.Vector(0,0,6000),p-unreal.Vector(0,0,6000))
 if hit:
  neighbors=[]
  for dx,dy in [(2,0),(-2,0),(0,2),(0,-2)]:
   q=p+unreal.Vector(dx,dy,0);h=unreal.PiedmontWorldTools.trace_world_surface(q+unreal.Vector(0,0,6000),q-unreal.Vector(0,0,6000))
   if h:neighbors.append({'z':h[0].z,'actor':h[1].get_actor_label()})
  samples.append({'position':xyz(p),'surface':xyz(hit[0]),'actor':hit[1].get_actor_label(),'kind':kind,'neighbors_2cm':neighbors})
for a in splines:
 s=a.centerline;n=max(2,math.ceil(s.get_spline_length()/1000)+1)
 for i in range(n):sample(s.get_location_at_distance_along_spline(s.get_spline_length()*i/(n-1),unreal.SplineCoordinateSpace.WORLD),'path')
meta=json.loads((root/'SourceAssets/Terrain/terrain-georeference.json').read_text());lo=meta['unreal_location_cm'];scale=meta['unreal_scale'];size=meta['size']
for y in range(20):
 for x in range(20):sample(unreal.Vector(lo[0]+(size[0]-1)*scale[0]*(x+.5)/20,lo[1]+(size[1]-1)*scale[1]*(y+.5)/20,0),'terrain')
geometry={'StaticMeshActor','Landscape','PiedmontPathSpline','PiedmontWaterHazard','WaterBodyLake'}
semantic={'DirectionalLight','SkyAtmosphere','SkyLight','PlayerStart','WaterZone','WaterBrushManager'}
ignore={'NavMeshBoundsVolume','RecastNavMesh'}
changes=[]
for a in actors:
 cls=a.get_class().get_name()
 assert not a.get_attach_parent_actor(),a.get_actor_label()
 assert cls in geometry|semantic|ignore,cls
 if cls in ignore:continue
 p=a.get_actor_location();r=a.get_actor_rotation();s=a.get_actor_scale3d()
 a.set_actor_location_and_rotation(mirror(p),unreal.Rotator(pitch=r.pitch,yaw=-r.yaw,roll=-r.roll),False,True)
 if cls in geometry:a.set_actor_scale3d(mirror(s))
 changes.append({'actor':a.get_actor_label(),'class':cls})
 if cls=='Landscape':assert unreal.PiedmontWorldTools.refresh_landscape_collision(a)
 if cls=='WaterBodyLake':assert unreal.PiedmontWorldTools.refresh_water_body(a)
errors=[];seams=[]
for row in samples:
 p=mirror(unreal.Vector(*row['position']));hit=unreal.PiedmontWorldTools.trace_world_surface(p+unreal.Vector(0,0,6000),p-unreal.Vector(0,0,6000))
 error=abs(hit[0].z-row['surface'][2]) if hit else 1e9
 if not hit or hit[1].get_actor_label()!=row['actor'] or error>.1:
  difference={'sample':row,'after':xyz(hit[0]) if hit else None,'actor':hit[1].get_actor_label() if hit else None,'error_cm':error}
  neighbors=row['neighbors_2cm']
  # At an existing deck/pavement seam, float rounding can choose either surface.
  # Accept only a surface already observed within 2 cm before conversion, with
  # its height inside that measured local range; retain all such differences.
  seam=hit and neighbors and hit[1].get_actor_label() in [n['actor'] for n in neighbors] and min(n['z'] for n in neighbors)-.1<=hit[0].z<=max(n['z'] for n in neighbors)+.1 and error<3.1
  (seams if seam else errors).append(difference)
report={'collision_sample_count':len(samples),'errors':errors,'baseline_samples':samples,'measured_seam_differences':seams,'changed_actors':changes,'saved':False}
(root/'work/geography-conversion.json').write_text(json.dumps(report,indent=2))
assert not errors,('Converted collision differs',len(errors),errors[:2])
points=[a.centerline.get_location_at_spline_point(i,unreal.SplineCoordinateSpace.WORLD) for a in splines for i in range(a.centerline.get_number_of_spline_points())]
lo=[min(getattr(p,k) for p in points) for k in ['x','y','z']];hi=[max(getattr(p,k) for p in points) for k in ['x','y','z']]
assert unreal.PiedmontWorldTools.build_park_navigation(unreal.Vector(*[(a+b)/2 for a,b in zip(lo,hi)]),unreal.Vector(*[(b-a)/2+500 for a,b in zip(lo,hi)]))
assert unreal.PiedmontWorldTools.finish_park_navigation_build()
start=next(a for a in actors if isinstance(a,unreal.PlayerStart)).get_actor_location()
end=mirror(unreal.Vector(*json.loads((root/'SourceAssets/Terrain/battle-park-exit.json').read_text())['xyz_cm']))
length=unreal.PiedmontWorldTools.park_route_length(start,end);assert length>0,'Converted start cannot reach Eastside'
settings.tags=list(settings.tags)+[marker]
assert unreal.EditorLoadingAndSavingUtils.save_map(world,'/Game/PiedmontRide/Maps/PiedmontWorld')
report.update(saved=True,passed=True,start_to_eastside_nav_cm=length)
(root/'Tests/Results/2026-09-11-coordinate-conversion.json').write_text(json.dumps(report,indent=2))
