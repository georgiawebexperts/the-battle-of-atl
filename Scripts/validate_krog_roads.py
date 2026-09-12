"""Import the Krog road into an isolated review map and check native support."""
import json,sys
from pathlib import Path
import unreal
root=Path(unreal.Paths.project_dir());folder=root/'SourceAssets/Terrain/KrogTraffic'
assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/PiedmontWorld')
ea=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
sys.path.insert(0,str(root/'Scripts'))
from battle_geography import import_source_landscape,source_vector
unreal.PiedmontWorldTools.finish_editor_asset_loading()
meta=json.loads((root/'SourceAssets/Terrain/terrain-georeference.json').read_text())
network=json.loads((folder/'network.json').read_text());junction=network['crossing_xyz'];surface_meta=json.loads((folder/'road-surfaces.json').read_text());table=surface_meta.get('junction_table',False)
baseline=[]
for filename in ['park-path-network.json','eastside-trail-network.json','krog-route-network.json']:
 for path in json.loads((root/'SourceAssets/Terrain'/filename).read_text())['paths']:
  points=path.get('points_cm',[])
  for p in points[::max(1,len(points)//12)]:
   v=source_vector(p)
   if ((v.x-29382)**2+(v.y-116232)**2)**.5<400:continue
   hit=unreal.PiedmontWorldTools.trace_world_surface(v+unreal.Vector(0,0,150),v-unreal.Vector(0,0,150))
   if hit:baseline.append((v,hit[0].z))
old=[a for a in ea.get_all_level_actors() if isinstance(a,unreal.Landscape)];assert len(old)==1
land=import_source_landscape(root/surface_meta['heightmap'],meta);assert land
land.set_editor_property('landscape_material',old[0].get_editor_property('landscape_material'));land.tags=list(old[0].tags)
label=old[0].get_actor_label();assert ea.destroy_actor(old[0]);land.set_actor_label(label)
assert unreal.PiedmontWorldTools.refresh_landscape_collision(land)
for kind in ['Shell','Columns']:
 actors=[a for a in ea.get_all_level_actors() if a.get_actor_label()=='Krog route SM_KrogTunnel_'+kind];assert len(actors)==1
 actors[0].static_mesh_component.set_static_mesh(unreal.load_asset('/Game/BattleForTheA/Environment/KrogPortalCandidate/SM_KrogPortal_'+kind))

assert not any(a.actor_has_tag('KrogRoad') for a in ea.get_all_level_actors())
opts=unreal.FbxImportUI();opts.import_as_skeletal=False;opts.import_materials=False;opts.import_textures=False
opts.automated_import_should_detect_type=False;opts.mesh_type_to_import=unreal.FBXImportType.FBXIT_STATIC_MESH
opts.static_mesh_import_data.combine_meshes=True;opts.static_mesh_import_data.auto_generate_collision=False
opts.static_mesh_import_data.remove_degenerates=False
path='/Game/BattleForTheA/Environment/KrogTraffic';name='SM_Krog_Road'
task=unreal.AssetImportTask();task.filename=str(folder/'Krog_Road.obj');task.destination_path=path;task.destination_name=name
task.automated=True;task.save=True;task.replace_existing=True;task.options=opts
unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
mesh=unreal.load_asset(path+'/'+name);assert mesh
mesh.set_material(0,unreal.load_asset('/Game/PiedmontRide/Materials/M_ParkAsphaltWorld'))
mesh.get_editor_property('body_setup').set_editor_property('collision_trace_flag',unreal.CollisionTraceFlag.CTF_USE_COMPLEX_AS_SIMPLE)
nanite=mesh.get_editor_property('nanite_settings');nanite.enabled=True;nanite.position_precision=8;nanite.generate_fallback=unreal.NaniteGenerateFallback.ENABLED
nanite.fallback_target=unreal.NaniteFallbackTarget.PERCENT_TRIANGLES;nanite.fallback_relative_error=0;nanite.fallback_percent_triangles=1
mesh.set_editor_property('nanite_settings',nanite)
unreal.EditorAssetLibrary.save_loaded_asset(mesh)
if surface_meta.get('crowned_dekalb'):
 concrete_name='SM_Krog_Concrete_Reconciled'
 concrete_task=unreal.AssetImportTask();concrete_task.filename=str(folder/'KrogRoute_Concrete_7_2_Reconciled.obj')
 concrete_task.destination_path=path;concrete_task.destination_name=concrete_name
 concrete_task.automated=True;concrete_task.save=True;concrete_task.replace_existing=True;concrete_task.options=opts
 unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([concrete_task])
 concrete=unreal.load_asset(path+'/'+concrete_name);assert concrete
 concrete.set_material(0,unreal.load_asset('/Game/PiedmontRide/Materials/M_ParkConcreteWorld'))
 concrete.get_editor_property('body_setup').set_editor_property('collision_trace_flag',unreal.CollisionTraceFlag.CTF_USE_COMPLEX_AS_SIMPLE)
 unreal.EditorAssetLibrary.save_loaded_asset(concrete)
 targets=[a for a in ea.get_all_level_actors() if a.get_actor_label()=='Krog route SM_KrogRoute_Concrete_7_2'];assert len(targets)==1
 targets[0].static_mesh_component.set_static_mesh(concrete)
a=ea.spawn_actor_from_class(unreal.StaticMeshActor,unreal.Vector());a.set_actor_label('Krog DeKalb road approaches')
a.tags=[unreal.Name('KrogRoad'),unreal.Name('RidePath')];a.set_folder_path('Eastside/KrogLake')
a.static_mesh_component.set_static_mesh(mesh);a.static_mesh_component.set_collision_profile_name('BlockAll')
unreal.PiedmontWorldTools.finish_editor_asset_loading()
failures=[];counts={}
probes=json.loads((folder/'road-probes.json').read_text())['samples']
for row in probes:
 x,y,z=row['xyz'];hit=unreal.PiedmontWorldTools.trace_world_surface(unreal.Vector(x,y,z+250),unreal.Vector(x,y,z-250))
 label=hit[1].get_actor_label() if hit else None;counts[label]=counts.get(label,0)+1
 if not hit or (hit[1]!=a and not (table and hit[1].get_actor_label().startswith('Krog route SM_KrogRoute_'))) or abs(hit[0].z-z)>.25:failures.append({'xyz':row['xyz'],'actor':label,'z':hit[0].z if hit else None})
# Preserve distant installed walking/riding surfaces.
trail_failures=[];changed_crossing_samples=[]
crown_points=[s['centre_cm'] for s in json.loads((folder/'dekalb-cross-section.json').read_text())['stations']] if surface_meta.get('crowned_dekalb') else []
concrete_triangles=[]
if surface_meta.get('crowned_dekalb'):
 vertices=[]
 for row in (folder/'KrogRoute_Concrete_7_2_Reconciled.obj').read_text().splitlines():
  fields=row.split()
  if fields and fields[0]=='v':vertices.append(tuple(map(float,fields[1:4])))
  elif fields and fields[0]=='f':concrete_triangles.append([vertices[int(value.split('/')[0])-1] for value in fields[1:]])
def matches_authored_concrete(v,new_z):
 heights=[]
 for p,q,r in concrete_triangles:
  if not min(p[0],q[0],r[0])-.001<=v.x<=max(p[0],q[0],r[0])+.001 or not min(p[1],q[1],r[1])-.001<=v.y<=max(p[1],q[1],r[1])+.001:continue
  dx,dy=q[0]-p[0],q[1]-p[1];ex,ey=r[0]-p[0],r[1]-p[1];det=dx*ey-dy*ex
  if abs(det)<1e-8:continue
  u=((v.x-p[0])*ey-(v.y-p[1])*ex)/det;t=(dx*(v.y-p[1])-dy*(v.x-p[0]))/det
  if min(u,t,1-u-t)>=-.00001:heights.append(p[2]+u*(q[2]-p[2])+t*(r[2]-p[2]))
 return bool(heights) and abs(new_z-max(heights))<=.25
def matches_authored_crown(v,new_z):
 nearest=None
 for p,q in zip(crown_points,crown_points[1:]):
  dx,dy=q[0]-p[0],q[1]-p[1];t=max(0,min(1,((v.x-p[0])*dx+(v.y-p[1])*dy)/(dx*dx+dy*dy)))
  distance=((v.x-p[0]-t*dx)**2+(v.y-p[1]-t*dy)**2)**.5
  if nearest is None or distance<nearest[0]:nearest=(distance,p[2]+t*(q[2]-p[2])-.02*distance)
 return nearest is not None and nearest[0]<=450 and abs(new_z-nearest[1])<=.5
for v,z in baseline:
 hit=unreal.PiedmontWorldTools.trace_world_surface(v+unreal.Vector(0,0,150),v-unreal.Vector(0,0,150))
 if hit and hit[1]==a and matches_authored_crown(v,hit[0].z):
  changed_crossing_samples.append({'xyz':[v.x,v.y,z],'new_z':hit[0].z,'reason':'Matches authored crowned DeKalb cross section within 0.5cm'});continue
 if hit and hit[1]==a and table and ((v.x-junction[0])**2+(v.y-junction[1])**2)**.5<=1500:
  changed_crossing_samples.append({'xyz':[v.x,v.y,z],'new_z':hit[0].z});continue
 if hit and abs(hit[0].z-z)>.5 and hit[1].get_actor_label()=='Krog route SM_KrogRoute_Concrete_7_2' and matches_authored_concrete(v,hit[0].z):
  changed_crossing_samples.append({'xyz':[v.x,v.y,z],'new_z':hit[0].z,'reason':'Matches authored concrete transition triangle within 0.25cm'});continue
 if not hit or abs(hit[0].z-z)>.5:trail_failures.append({'xyz':[v.x,v.y,z],'new_z':hit[0].z if hit else None})
passed=not failures and not trail_failures
if passed:assert unreal.EditorLoadingAndSavingUtils.save_map(unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world(),'/Game/PiedmontRide/Maps/PiedmontKrogRoadReview')
report={'passed':passed,'probes':len(probes),'hits':counts,'failures':failures,'trail_failures':trail_failures,'main_map_changed':False,'preserved_path_probes':len(baseline)-len(changed_crossing_samples),'raised_crossing_samples':changed_crossing_samples,'scope':'Native surface support and original trail preservation only; no traffic or visual acceptance.'}
(root/'Tests/Results/2026-09-12-krog-road-support.json').write_text(json.dumps(report,indent=2)+'\n')
