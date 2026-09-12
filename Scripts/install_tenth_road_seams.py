"""Install only the collision-validated pavement joins into the playable world."""
import unreal,json,pathlib
root=pathlib.Path(unreal.Paths.project_dir())
for name in ['2026-09-12-tenth-car-lane-collision','2026-09-12-native-road-lanes']:
 assert json.loads((root/'Tests/Results'/f'{name}.json').read_text())['passed'],name
assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/PiedmontWorld')
ea=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
assert not any(a.actor_has_tag('TenthRoadSeams') for a in ea.get_all_level_actors()),'Repair already installed'
mesh=unreal.load_asset('/Game/BattleForTheA/Environment/TenthStreetGraded/SM_TenthStreet_RoadSeams');assert mesh
actor=ea.spawn_actor_from_class(unreal.StaticMeshActor,unreal.Vector());actor.set_actor_label('10th Street road join repairs');actor.tags=[unreal.Name('TenthRoadSeams'),unreal.Name('RidePath')];actor.set_folder_path('Midtown/TenthStreet');actor.static_mesh_component.set_static_mesh(mesh);actor.static_mesh_component.set_collision_profile_name('BlockAll')
unreal.PiedmontWorldTools.finish_editor_asset_loading()
# Verify the discovered seam explicitly in the actual world before saving.
x,y=-21028.25341123144,12835.531643353048
hit=unreal.PiedmontWorldTools.trace_world_surface(unreal.Vector(x,y,1000),unreal.Vector(x,y,-1000));assert hit and hit[1]==actor
assert unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).save_current_level()
(root/'Tests/Results/2026-09-12-road-seam-install.json').write_text(json.dumps({'main_map_changed':True,'seam_probe':[hit[0].x,hit[0].y,hit[0].z],'actor':actor.get_actor_label(),'cars_installed':False,'desktop_build_updated':False},indent=2)+'\n')
