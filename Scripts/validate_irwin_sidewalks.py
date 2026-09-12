"""Import the Irwin sidewalk into an isolated review map and check native support."""
import json
from pathlib import Path
import unreal
root=Path(unreal.Paths.project_dir());folder=root/'SourceAssets/Terrain/IrwinTraffic'
assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/PiedmontIrwinBuildingsReview')
ea=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
assert not any(a.actor_has_tag('IrwinSidewalk') for a in ea.get_all_level_actors())
opts=unreal.FbxImportUI();opts.import_as_skeletal=False;opts.import_materials=False;opts.import_textures=False
opts.automated_import_should_detect_type=False;opts.mesh_type_to_import=unreal.FBXImportType.FBXIT_STATIC_MESH
opts.static_mesh_import_data.combine_meshes=True;opts.static_mesh_import_data.auto_generate_collision=False
opts.static_mesh_import_data.remove_degenerates=False
path='/Game/BattleForTheA/Environment/IrwinTraffic';name='SM_Irwin_Sidewalk'
task=unreal.AssetImportTask();task.filename=str(folder/'Irwin_Sidewalk.obj');task.destination_path=path;task.destination_name=name
task.automated=True;task.save=True;task.replace_existing=True;task.options=opts
unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
mesh=unreal.load_asset(path+'/'+name);assert mesh
mesh.set_material(0,unreal.load_asset('/Game/PiedmontRide/Materials/M_ParkConcreteWorld'))
mesh.get_editor_property('body_setup').set_editor_property('collision_trace_flag',unreal.CollisionTraceFlag.CTF_USE_COMPLEX_AS_SIMPLE)
nanite=mesh.get_editor_property('nanite_settings');nanite.enabled=True;nanite.position_precision=8;nanite.generate_fallback=unreal.NaniteGenerateFallback.ENABLED
nanite.fallback_target=unreal.NaniteFallbackTarget.PERCENT_TRIANGLES;nanite.fallback_relative_error=0;nanite.fallback_percent_triangles=1
mesh.set_editor_property('nanite_settings',nanite)
unreal.EditorAssetLibrary.save_loaded_asset(mesh)
a=ea.spawn_actor_from_class(unreal.StaticMeshActor,unreal.Vector());a.set_actor_label('Irwin Lake roadside pavement')
a.tags=[unreal.Name('IrwinSidewalk'),unreal.Name('RidePath')];a.set_folder_path('Eastside/IrwinLake')
a.static_mesh_component.set_static_mesh(mesh);a.static_mesh_component.set_collision_profile_name('BlockAll')
unreal.PiedmontWorldTools.finish_editor_asset_loading()
cars=[car for car in ea.get_all_level_actors() if isinstance(car,unreal.BattleRoadCar)]
for car in cars:car.set_actor_enable_collision(False)
failures=[];counts={}
probes=json.loads((folder/'sidewalk-probes.json').read_text())['samples']
for row in probes:
 x,y,z=row['xyz'];hit=unreal.PiedmontWorldTools.trace_world_surface(unreal.Vector(x,y,z+250),unreal.Vector(x,y,z-250))
 label=hit[1].get_actor_label() if hit else None;counts[label]=counts.get(label,0)+1
 if not hit or hit[1]!=a or abs(hit[0].z-z)>.25:failures.append({'xyz':row['xyz'],'actor':label,'z':hit[0].z if hit else None})
# Original trail probes must stay exactly on the same installed surfaces.
trail_failures=[]
survey=json.loads((root/'Tests/Results/2026-09-12-irwin-crossing-survey.json').read_text())
for row in survey['samples']:
 if not row['actor'] or not row['actor'].startswith(('Eastside trail','Krog route')):continue
 x,y=row['xy'];z=row['height'];hit=unreal.PiedmontWorldTools.trace_world_surface(unreal.Vector(x,y,z+250),unreal.Vector(x,y,z-250))
 if not hit or hit[1].get_actor_label()!=row['actor'] or abs(hit[0].z-z)>.25:trail_failures.append(row)
for car in cars:car.set_actor_enable_collision(True)
passed=not failures and not trail_failures
if passed:assert unreal.EditorLoadingAndSavingUtils.save_map(unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world(),'/Game/PiedmontRide/Maps/PiedmontIrwinSidewalkReview')
report={'passed':passed,'probes':len(probes),'hits':counts,'failures':failures,'trail_failures':trail_failures,'main_map_changed':False,'scope':'Native surface support and original trail preservation only; no traffic or visual acceptance.'}
(root/'Tests/Results/2026-09-12-irwin-sidewalk-support.json').write_text(json.dumps(report,indent=2)+'\n')
