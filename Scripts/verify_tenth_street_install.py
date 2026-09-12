"""Reload the playable map and verify persisted road actors and collision setup."""
import json,pathlib,unreal
root=pathlib.Path(unreal.Paths.project_dir())
assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/PiedmontWorld')
ea=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
actors=[a for a in ea.get_all_level_actors() if 'BattleTenthStreet' in [str(t) for t in a.tags]]
expected={'SM_TenthStreet_'+name for name in ['Road','CycleTrack','Separator','Sidewalk']}
assert {a.get_actor_label() for a in actors}==expected and len(actors)==4
rows=[]
for actor in actors:
 comp=actor.static_mesh_component;mesh=comp.static_mesh
 assert mesh and str(comp.get_collision_profile_name())=='BlockAll'
 assert mesh.get_editor_property('body_setup').get_editor_property('collision_trace_flag')==unreal.CollisionTraceFlag.CTF_USE_COMPLEX_AS_SIMPLE
 assert mesh.get_material(0)
 rows.append({'label':actor.get_actor_label(),'asset':mesh.get_path_name(),'collision':'BlockAll / complex as simple'})
assert not [a for a in ea.get_all_level_actors() if isinstance(a,unreal.SceneCapture2D)]
(root/'Tests/Results/2026-09-11-tenth-street-install.json').write_text(json.dumps({'map_reloaded':True,'actors':rows,'review_camera_absent':True,'desktop_build_updated':False,'presentation_accepted':False,'limitations':'Persistence and collision configuration only. Ride clearance, traffic and crossing gameplay are not verified.'},indent=2)+'\n')
