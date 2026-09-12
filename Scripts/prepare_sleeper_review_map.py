"""Create an isolated native review map with the first scanned ambient sleeper."""
import json,pathlib,unreal
root=pathlib.Path(unreal.Paths.project_dir());destination='/Game/PiedmontRide/Maps/PiedmontSleeperReview'
assert not unreal.EditorAssetLibrary.does_asset_exist(destination), 'Review map exists; inspect before replacing'
report=json.loads((root/'Tests/Results/2026-09-12-sleeper-sites.json').read_text());assert report['candidates']
site=report['candidates'][0]
assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/PiedmontWorld')
ea=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
p=unreal.Vector(*site['xyz'])+unreal.Vector(0,0,90)
actor=ea.spawn_actor_from_class(unreal.PiedmontPedestrian,p,unreal.Rotator(yaw=site['yaw']))
assert actor
actor.set_editor_property('bAmbientSleeper',True)
actor.set_actor_label('Ambient sleeper review — ground candidate 1')
actor.tags=list(actor.tags)+[unreal.Name('AmbientSleeperReview')]
world=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
assert unreal.EditorLoadingAndSavingUtils.save_map(world,destination)
(root/'Tests/Results/2026-09-12-sleeper-review-map.json').write_text(json.dumps({'map':destination,'site':site,'ambient_sleeper':True,'main_map_changed':False,'runtime_verified':False},indent=2)+'\n')
