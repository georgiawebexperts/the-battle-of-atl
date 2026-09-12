"""Place the first runtime-verified ambient sleeper in the main park."""
import unreal,json,pathlib
root=pathlib.Path(unreal.Paths.project_dir())
report=json.loads((root/'Tests/Results/2026-09-12-native-sleeper-trigger.json').read_text());assert report['passed']
site=json.loads((root/'Tests/Results/2026-09-12-sleeper-sites.json').read_text())['candidates'][0]
assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/PiedmontWorld')
ea=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
assert not any(a.actor_has_tag('AmbientSleeperReview') for a in ea.get_all_level_actors()),'Sleeper already present; inspect before placing another'
p=unreal.Vector(*site['xyz'])+unreal.Vector(0,0,90)
a=ea.spawn_actor_from_class(unreal.PiedmontPedestrian,p,unreal.Rotator(yaw=site['yaw']));assert a
a.set_editor_property('bAmbientSleeper',True);a.set_editor_property('AmbientWakeChance',.18)
a.set_actor_label('Park sleeper — occasional wake and brief chase')
a.tags=list(a.tags)+[unreal.Name('AmbientSleeperReview'),unreal.Name('ParkAmbientSleeper')]
assert unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).save_current_level()
(root/'Tests/Results/2026-09-12-first-ambient-sleeper-install.json').write_text(json.dumps({'map':'/Game/PiedmontRide/Maps/PiedmontWorld','site':site,'wake_chance':.18,'count_added':1,'packaged':False,'limitations':['Initial City crewneck outfit; rough wardrobe and bench sleepers pending.','Continuous animation contact and broader visual polish pending.']},indent=2)+'\n')
