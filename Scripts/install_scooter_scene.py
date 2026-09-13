"""Add the reviewed rare scooter encounter to current main, without replacing the map."""
import json,shutil,datetime,hashlib
from pathlib import Path
import unreal
root=Path(unreal.Paths.project_dir()).resolve()
for name in ['2026-09-13-scooter-navigation-regression.json','2026-09-13-native-scooter-visit-nav-review.json','2026-09-13-krog-turnaround-mixed-clearance.json']:
    assert json.loads((root/'Tests/Results'/name).read_text())['passed'],name
assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/PiedmontWorld')
unreal.PiedmontWorldTools.finish_editor_asset_loading()
ea=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
assert any(isinstance(a,unreal.BattleRoadTrafficDirector) and a.actor_has_tag('KrogTrafficReview') and any(l.get_editor_property('bLoopRoute') for l in a.get_editor_property('Lanes')) for a in ea.get_all_level_actors()),'Promote verified turnaround traffic before installing scene'
assert not any(isinstance(a,unreal.BattleScooterScene) for a in ea.get_all_level_actors()),'Scene already installed; inspect instead of duplicating'
patches=[a for a in ea.get_all_level_actors() if a.actor_has_tag('ScooterNavigation')]
assert len(patches)<=1,'Duplicate navigation patches'
if patches:
    nav=json.loads((root/'Tests/Results/2026-09-13-scooter-navigation-main.json').read_text())
    assert nav['passed'] and nav['saved'] and nav['tiles']==236,'Cannot resume unverified navigation'
source=root/'Content/PiedmontRide/Maps/PiedmontWorld.umap'
backup=root/'work/map-backups'/datetime.datetime.now().strftime('pre-scooter-%Y%m%d-%H%M%S')
backup.mkdir(parents=True,exist_ok=False);backup=backup/source.name;shutil.copy2(source,backup)
before={str(tuple(a.get_editor_property('actor_guid').get_editor_property(k) for k in ('a','b','c','d'))) for a in ea.get_all_level_actors() if not isinstance(a,unreal.RecastNavMesh)}
# Reuse the ground sampling and nav build, but edit current main directly.
script=(root/'Scripts/build_scooter_navigation_review.py').read_text()
script=script.replace("dest='/Game/PiedmontRide/Maps/PiedmontScooterReview'","dest='/Game/PiedmontRide/Maps/PiedmontWorld'")
script=script.replace('assert unreal.EditorLoadingAndSavingUtils.save_map(world,dest)','')
script=script.replace('2026-09-13-scooter-navigation-review.json','2026-09-13-scooter-navigation-main.json').replace("'main_map_changed':False","'main_map_changed':True")
# Save once, after the scene and preservation check both succeed.
script=script.replace("report['saved']=bool(unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).save_current_level())","report['saved']=False")
if not patches:
    exec(compile(script,'scooter_main_navigation','exec'),{'__name__':'__main__'})
a=ea.spawn_actor_from_class(unreal.BattleScooterScene,unreal.Vector(30349.800013,114057.877225,1110.508188))
a.set_actor_label('Occasional Krog scooter crash and helpers');a.tags=[unreal.Name('KrogScooterEncounter')];a.set_editor_property('AppearanceChance',.18)
assert before <= {str(tuple(a.get_editor_property('actor_guid').get_editor_property(k) for k in ('a','b','c','d'))) for a in ea.get_all_level_actors() if not isinstance(a,unreal.RecastNavMesh)},'Existing main actors removed'
assert unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).save_current_level()
nav_report=root/'Tests/Results/2026-09-13-scooter-navigation-main.json'
nav=json.loads(nav_report.read_text());nav['saved']=True
nav_report.write_text(json.dumps(nav,indent=2)+'\n')
(root/'Tests/Results/2026-09-13-scooter-main-install.json').write_text(json.dumps({'passed':True,'main_map_changed':True,'backup':str(backup),'backup_sha256':hashlib.sha256(backup.read_bytes()).hexdigest(),'appearance_chance':.18,'scope':'Added native scene and reviewed navigation tiles to current main; existing actor GUIDs preserved. Fresh-process verification and packaged acceptance pending.'},indent=2)+'\n')
