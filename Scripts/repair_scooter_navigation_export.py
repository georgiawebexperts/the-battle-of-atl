"""Rebuild current main navigation after verifying navigation-only export in review."""
import unreal,json,hashlib,datetime,shutil,runpy
from pathlib import Path
root=Path(unreal.Paths.project_dir()).resolve();ea=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
review=json.loads((root/'Tests/Results/2026-09-13-krog-approach-navigation-regression.json').read_text());assert review['passed'] and review['scooter_route_length_cm']>0
assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/PiedmontWorld')
assert sum(a.actor_has_tag('KrogApproachReview') for a in ea.get_all_level_actors())==1
def ids():return {tuple(a.get_editor_property('actor_guid').get_editor_property(k) for k in ('a','b','c','d')) for a in ea.get_all_level_actors() if not isinstance(a,unreal.RecastNavMesh)}
before=ids();main=root/'Content/PiedmontRide/Maps/PiedmontWorld.umap';backup=root/'work/map-backups'/datetime.datetime.now().strftime('pre-nav-export-repair-%Y%m%d-%H%M%S');backup.mkdir(parents=True);shutil.copy2(main,backup/main.name)
source=(root/'Scripts/rebuild_krog_world_navigation.py').read_text().replace('PiedmontKrogWorldReview','PiedmontWorld').replace('2026-09-12-krog-world-navigation.json','2026-09-13-main-nav-export-repair.json').replace("'main_map_changed':False","'main_map_changed':True")
old="report['saved']=bool(unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).save_current_level()) if passed else False";assert old in source
source=source.replace(old,"report['saved']=False")
exec(compile(source,'main_nav_export_repair','exec'),{'__name__':'__main__'})
scene=next(a for a in ea.get_all_level_actors() if isinstance(a,unreal.BattleScooterScene));corridor=next(c for c in json.loads((root/'Tests/Results/2026-09-13-scooter-navigation-survey.json').read_text())['corridors'] if c['clear_ground_corridor'])
a=unreal.PiedmontWorldTools.project_park_navigation(scene.get_actor_location());b=unreal.PiedmontWorldTools.project_park_navigation(unreal.Vector(*corridor['end']));length=unreal.PiedmontWorldTools.park_route_length(a,b) if a and b else -1
p=root/'Tests/Results/2026-09-13-main-nav-export-repair.json';r=json.loads(p.read_text());r.update({'passed':bool(r['passed'] and length>0 and ids()==before),'saved':False,'scooter_route_length_cm':length,'backup':str(backup/main.name),'actor_guids_preserved':ids()==before});p.write_text(json.dumps(r,indent=2)+'\n')
assert r['passed'],r
assert ids()==before
assert unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).save_current_level()
p=root/'Tests/Results/2026-09-13-main-nav-export-repair.json';r=json.loads(p.read_text());r.update({'saved':True,'scooter_route_length_cm':length,'backup':str(backup/main.name),'actor_guids_preserved':True});p.write_text(json.dumps(r,indent=2)+'\n')
