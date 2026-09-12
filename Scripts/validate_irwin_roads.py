"""Import the Irwin road into an isolated review map and check native support."""
import json
from pathlib import Path
import unreal
root=Path(unreal.Paths.project_dir());folder=root/'SourceAssets/Terrain/IrwinTraffic'
assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/PiedmontWorld')
ea=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
assert not any(a.actor_has_tag('IrwinRoad') for a in ea.get_all_level_actors())
opts=unreal.FbxImportUI();opts.import_as_skeletal=False;opts.import_materials=False;opts.import_textures=False
opts.automated_import_should_detect_type=False;opts.mesh_type_to_import=unreal.FBXImportType.FBXIT_STATIC_MESH
opts.static_mesh_import_data.combine_meshes=True;opts.static_mesh_import_data.auto_generate_collision=False
opts.static_mesh_import_data.remove_degenerates=False
path='/Game/BattleForTheA/Environment/IrwinTraffic';name='SM_Irwin_Road'
task=unreal.AssetImportTask();task.filename=str(folder/'Irwin_Road.obj');task.destination_path=path;task.destination_name=name
task.automated=True;task.save=True;task.replace_existing=True;task.options=opts
unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
mesh=unreal.load_asset(path+'/'+name);assert mesh
mesh.set_material(0,unreal.load_asset('/Game/PiedmontRide/Materials/M_ParkAsphaltWorld'))
mesh.get_editor_property('body_setup').set_editor_property('collision_trace_flag',unreal.CollisionTraceFlag.CTF_USE_COMPLEX_AS_SIMPLE)
unreal.EditorAssetLibrary.save_loaded_asset(mesh)
a=ea.spawn_actor_from_class(unreal.StaticMeshActor,unreal.Vector());a.set_actor_label('Irwin Lake road approaches')
a.tags=[unreal.Name('IrwinRoad'),unreal.Name('RidePath')];a.set_folder_path('Eastside/IrwinLake')
a.static_mesh_component.set_static_mesh(mesh);a.static_mesh_component.set_collision_profile_name('BlockAll')
unreal.PiedmontWorldTools.finish_editor_asset_loading()
failures=[];counts={}
probes=json.loads((folder/'road-probes.json').read_text())['samples']
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
passed=not failures and not trail_failures
if passed:assert unreal.EditorLoadingAndSavingUtils.save_map(unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world(),'/Game/PiedmontRide/Maps/PiedmontIrwinRoadReview')
report={'passed':passed,'probes':len(probes),'hits':counts,'failures':failures,'trail_failures':trail_failures,'main_map_changed':False,'scope':'Native surface support and original trail preservation only; no traffic or visual acceptance.'}
(root/'Tests/Results/2026-09-12-irwin-road-support.json').write_text(json.dumps(report,indent=2)+'\n')
