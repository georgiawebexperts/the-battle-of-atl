import unreal,json,pathlib
actors=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
changes=[]
for a in actors.get_all_level_actors():
 n=a.get_actor_label()
 if n.startswith('Pavement lane stripe'):
  a.static_mesh_component.set_collision_enabled(unreal.CollisionEnabled.NO_COLLISION)
  a.set_actor_enable_collision(False)
 if n=='Uphill ramp' or n=='Downhill ramp':
  changes.append({'name':n,'old':str(a.get_actor_rotation())})
  a.set_actor_rotation(unreal.Rotator(pitch=15 if n=='Uphill ramp' else -15,yaw=0,roll=0),False)
unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).save_current_level()
pathlib.Path(unreal.Paths.project_dir(),'Scripts','v2-course-fixes.json').write_text(json.dumps(changes,indent=2))

pathlib.Path(unreal.Paths.project_dir(),'Scripts','v2-stripe-audit.json').write_text(json.dumps([{'name':a.get_actor_label(),'collision':str(a.static_mesh_component.get_collision_enabled())} for a in actors.get_all_level_actors() if 'stripe' in a.get_actor_label().lower()],indent=2))
