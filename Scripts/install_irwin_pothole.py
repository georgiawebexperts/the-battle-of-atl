"""Install one reviewed shallow pothole and verify main road/trail support."""
import json
from pathlib import Path
import unreal
root=Path(unreal.Paths.project_dir());folder=root/'SourceAssets/Terrain/IrwinTraffic'
assert json.loads((root/'Tests/Results/2026-09-12-native-pothole-ride.json').read_text())['passed']
assert json.loads((root/'Tests/Results/2026-09-12-pothole-rim-render.json').read_text())['shallow_surface_readable']
assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/PiedmontWorld')
ea=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
assert not any(a.actor_has_tag('IrwinPotholeRoad') for a in ea.get_all_level_actors())
old=[a for a in ea.get_all_level_actors() if a.actor_has_tag('IrwinRoad')];assert len(old)==1
ea.destroy_actor(old[0])
path='/Game/BattleForTheA/Environment/IrwinTraffic'
mesh=unreal.load_asset(path+'/SM_Irwin_PotholeRoad');assert mesh
a=ea.spawn_actor_from_class(unreal.StaticMeshActor,unreal.Vector());a.set_actor_label('Irwin road')
a.tags=[unreal.Name('IrwinRoad'),unreal.Name('RidePath')];a.set_folder_path('Eastside/IrwinLake')
a.static_mesh_component.set_static_mesh(mesh);a.static_mesh_component.set_collision_profile_name('BlockAll')
assert not any(isinstance(actor,unreal.BattlePothole) for actor in ea.get_all_level_actors())
site=json.loads((folder/'pothole-site.json').read_text());x,y,z=site['xyz']
hole=ea.spawn_actor_from_class(unreal.BattlePothole,unreal.Vector(x,y,z));hole.set_actor_label('Irwin shallow pothole');hole.tags=[unreal.Name('AuthoredPothole')];hole.set_folder_path('Eastside/IrwinLake')
hole.get_editor_property('Visual').set_static_mesh(unreal.load_asset(path+'/SM_Pothole_Surface'));hole.set_editor_property('ContactRadius',site['contact_radius_cm']);hole.set_editor_property('bDeep',False)
unreal.PiedmontWorldTools.finish_editor_asset_loading()
failures=[];counts={}
probes=json.loads((folder/'pothole-road-probes.json').read_text())['samples']
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
if passed:assert unreal.EditorLoadingAndSavingUtils.save_map(unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world(),'/Game/PiedmontRide/Maps/PiedmontWorld')
report={'passed':passed,'probes':len(probes),'hits':counts,'failures':failures,'trail_failures':trail_failures,'main_map_changed':passed,'potholes':1,'desktop_build_updated':False,'scope':'Native surface support and original trail preservation only; no traffic or visual acceptance.'}
(root/'Tests/Results/2026-09-12-pothole-main-install.json').write_text(json.dumps(report,indent=2)+'\n')
