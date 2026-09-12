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
for v,z in baseline:
 hit=unreal.PiedmontWorldTools.trace_world_surface(v+unreal.Vector(0,0,150),v-unreal.Vector(0,0,150))
 if hit and hit[1]==a and table and ((v.x-junction[0])**2+(v.y-junction[1])**2)**.5<=1500:
  changed_crossing_samples.append({'xyz':[v.x,v.y,z],'new_z':hit[0].z});continue
 if not hit or abs(hit[0].z-z)>.5:trail_failures.append({'xyz':[v.x,v.y,z],'new_z':hit[0].z if hit else None})
passed=not failures and not trail_failures
if passed:assert unreal.EditorLoadingAndSavingUtils.save_map(unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world(),'/Game/PiedmontRide/Maps/PiedmontKrogRoadReview')
report={'passed':passed,'probes':len(probes),'hits':counts,'failures':failures,'trail_failures':trail_failures,'main_map_changed':False,'preserved_path_probes':len(baseline)-len(changed_crossing_samples),'raised_crossing_samples':changed_crossing_samples,'scope':'Native surface support and original trail preservation only; no traffic or visual acceptance.'}
(root/'Tests/Results/2026-09-12-krog-road-support.json').write_text(json.dumps(report,indent=2)+'\n')
