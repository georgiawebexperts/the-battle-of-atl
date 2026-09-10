import unreal,pathlib,json
actors=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
existing=[a for a in actors.get_all_level_actors() if a.get_actor_label()=='Lake safety trigger']
if not existing:
    hazard=actors.spawn_actor_from_class(unreal.PiedmontWaterHazard,unreal.Vector(13200,-2250,-15))
    hazard.set_actor_label('Lake safety trigger')
    hazard.set_editor_property('polygon',[unreal.Vector(0,0,0),unreal.Vector(5600,0,0),unreal.Vector(5600,4500,0),unreal.Vector(0,4500,0)])
    hazard.set_editor_property('detection_height',190)
unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).save_current_level()
pathlib.Path(unreal.Paths.project_dir(),'Scripts','v2-lab-upgrade.json').write_text(json.dumps({'version':'0.2.1','water_triggers':len([a for a in actors.get_all_level_actors() if isinstance(a,unreal.PiedmontWaterHazard)])}))
