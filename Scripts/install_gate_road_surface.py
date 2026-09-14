"""Promote the tested gate pavement actor while preserving all existing map actors."""
import unreal,pathlib,json,hashlib,shutil
root=pathlib.Path(unreal.Paths.project_dir()).resolve()
main=root/'Content/PiedmontRide/Maps/PiedmontWorld.umap'
review=json.loads((root/'Tests/Results/2026-09-13-gate-road-candidate.json').read_text())
route=json.loads((root/'Tests/Results/2026-09-13-gate-road-route.json').read_text())
assert route['passed'] and route['road_cleanup']==[90,324,1] and route['gate_starts']==1
before=hashlib.sha256(main.read_bytes()).hexdigest();assert before==review['map_sha256'],'Main map changed since candidate review'
backup=root/'work/map-backups'/('PiedmontWorld-before-gate-road-'+before[:12]+'.umap');backup.parent.mkdir(parents=True,exist_ok=True)
if backup.exists():assert hashlib.sha256(backup.read_bytes()).hexdigest()==before
else:shutil.copy2(main,backup)
assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/PiedmontWorld')
ea=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
assert not any('BattleGateRoad' in [str(t) for t in a.tags] for a in ea.get_all_level_actors())
mesh=unreal.load_asset('/Game/BattleForTheA/Environment/GateRoad/SM_GateRoad');assert mesh
actor=ea.spawn_actor_from_class(unreal.StaticMeshActor,unreal.Vector());actor.set_actor_label('SM_GateRoad');actor.tags=[unreal.Name('RidePath'),unreal.Name('BattleGateRoad')];actor.set_folder_path('Midtown/GateRoad');actor.static_mesh_component.set_static_mesh(mesh);actor.static_mesh_component.set_collision_profile_name('BlockAll')
unreal.PiedmontWorldTools.finish_editor_asset_loading()
world=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
assert unreal.EditorLoadingAndSavingUtils.save_map(world,'/Game/PiedmontRide/Maps/PiedmontWorld')
report={'installed':True,'before_sha256':before,'after_sha256':hashlib.sha256(main.read_bytes()).hexdigest(),'backup':str(backup),'added_actor':actor.get_path_name(),'scope':'One terrain-fitted gate pavement actor added; runtime removes its 42 covered legacy blocks. Packaged verification pending.'}
(root/'Tests/Results/2026-09-13-gate-road-install.json').write_text(json.dumps(report,indent=2)+'\n')
