"""Promote only the verified road-end closure additions and refresh main navigation."""
import json,shutil,datetime,hashlib
from pathlib import Path
from collections import Counter
import unreal
root=Path(unreal.Paths.project_dir()).resolve();ea=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
assert json.loads((root/'Tests/Results/2026-09-12-native-krog-construction.json').read_text())['passed']
def inventory():return Counter((a.get_actor_label(),a.get_class().get_name()) for a in ea.get_all_level_actors())
assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/PiedmontWorld')
before=inventory();source=root/'Content/PiedmontRide/Maps/PiedmontWorld.umap'
backup=root/'work/map-backups'/datetime.datetime.now().strftime('pre-closures-%Y%m%d-%H%M%S');backup.mkdir(parents=True,exist_ok=False);backup=backup/source.name;shutil.copy2(source,backup)
assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/PiedmontKrogBoundaryReview')
after=inventory();assert not before-after,'Review dropped main actors'
added=after-before;assert added and all(label.startswith('DeKalb closure ') for label,cls in added),'Unexpected additions'
assert sum(a.actor_has_tag('KrogConstruction') for a in ea.get_all_level_actors())==sum(added.values())
assert unreal.EditorLoadingAndSavingUtils.save_map(unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world(),'/Game/PiedmontRide/Maps/PiedmontWorld')
script=(root/'Scripts/rebuild_krog_world_navigation.py').read_text().replace('PiedmontKrogWorldReview','PiedmontWorld').replace('2026-09-12-krog-world-navigation.json','2026-09-12-krog-construction-main-navigation.json').replace("'main_map_changed':False","'main_map_changed':True")
exec(compile(script,'main_construction_navigation','exec'),{'__name__':'__main__'})
assert inventory()==after
(root/'Tests/Results/2026-09-12-krog-construction-promotion.json').write_text(json.dumps({'main_map_changed':True,'backup':str(backup),'backup_sha256':hashlib.sha256(backup.read_bytes()).hexdigest(),'added_actors':sum(added.values()),'passed':True,'scope':'Only closure actors added; navigation rebuilt without sampled regressions. Installed055 unchanged.'},indent=2)+'\n')
