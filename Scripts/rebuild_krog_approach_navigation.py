"""Rebuild only the isolated approach review, retaining main-map bytes."""
import unreal,hashlib,json
from pathlib import Path
root=Path(unreal.Paths.project_dir()).resolve();main=root/'Content/PiedmontRide/Maps/PiedmontWorld.umap';before=hashlib.sha256(main.read_bytes()).hexdigest()
(root/'Tests/Results/2026-09-13-krog-approach-navigation-regression.json').write_text(json.dumps({'passed':False,'status':'Rebuild started; fresh scooter connection verification pending.'})+'\n')
assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/PiedmontKrogApproachReview')
ea=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
assert sum(a.actor_has_tag('KrogApproachReview') for a in ea.get_all_level_actors())==1
source=(root/'Scripts/rebuild_krog_world_navigation.py').read_text()
assert 'PiedmontKrogWorldReview' in source and '2026-09-12-krog-world-navigation.json' in source
source=source.replace('PiedmontKrogWorldReview','PiedmontKrogApproachReview').replace('2026-09-12-krog-world-navigation.json','2026-09-13-krog-approach-navigation.json')
exec(compile(source,'approach_navigation','exec'),{'__name__':'__main__'})
assert hashlib.sha256(main.read_bytes()).hexdigest()==before,'Main changed during review navigation rebuild'
report=root/'Tests/Results/2026-09-13-krog-approach-navigation.json';data=json.loads(report.read_text());data['main_sha256_preserved']=before;report.write_text(json.dumps(data,indent=2)+'\n')
source=(root/'Scripts/compare_scooter_navigation.py').read_text().replace("after=survey('PiedmontScooterReview')","after=survey('PiedmontKrogApproachReview')").replace('2026-09-13-scooter-navigation-regression.json','2026-09-13-krog-approach-navigation-regression.json')
exec(compile(source,'approach_navigation_comparison','exec'),{'__name__':'__main__'})
assert hashlib.sha256(main.read_bytes()).hexdigest()==before

assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/PiedmontKrogApproachReview')
unreal.PiedmontWorldTools.finish_editor_asset_loading()
scene=next(a for a in ea.get_all_level_actors() if isinstance(a,unreal.BattleScooterScene))
corridor=next(c for c in json.loads((root/'Tests/Results/2026-09-13-scooter-navigation-survey.json').read_text())['corridors'] if c['clear_ground_corridor'])
a=unreal.PiedmontWorldTools.project_park_navigation(scene.get_actor_location());b=unreal.PiedmontWorldTools.project_park_navigation(unreal.Vector(*corridor['end']));length=unreal.PiedmontWorldTools.park_route_length(a,b) if a and b else -1
report=root/'Tests/Results/2026-09-13-krog-approach-navigation-regression.json';data=json.loads(report.read_text());data['scooter_route_length_cm']=length;data['passed']=bool(data['passed'] and length>0);report.write_text(json.dumps(data,indent=2)+'\n');assert data['passed'],data
