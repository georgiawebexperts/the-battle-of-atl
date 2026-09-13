"""Undo only misplaced approach edits, using the preserved untouched review copy."""
import unreal,json,shutil,datetime
from collections import Counter
from pathlib import Path
root=Path(unreal.Paths.project_dir()).resolve();ea=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/PiedmontKrogApproachReview')
actors=ea.get_all_level_actors();assert not any(a.actor_has_tag('KrogApproachReview') for a in actors)
s=next(a for a in actors if a.actor_has_tag('BattleKrog_2'));points=[s.centerline.get_location_at_spline_point(i,unreal.SplineCoordinateSpace.WORLD) for i in range(s.centerline.get_number_of_spline_points())]
def ids():return {tuple(a.get_editor_property('actor_guid').get_editor_property(k) for k in ('a','b','c','d')) for a in ea.get_all_level_actors()}
original=Counter((a.get_actor_label(),a.get_class().get_name()) for a in ea.get_all_level_actors())
assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/PiedmontWorld')
patches=[a for a in ea.get_all_level_actors() if a.actor_has_tag('KrogApproachReview')];assert len(patches)==1
before=ids();patch_id=tuple(patches[0].get_editor_property('actor_guid').get_editor_property(k) for k in ('a','b','c','d'))
current=Counter((a.get_actor_label(),a.get_class().get_name()) for a in ea.get_all_level_actors());current.subtract([(patches[0].get_actor_label(),patches[0].get_class().get_name())]);assert +current==original
backup=root/'work/map-backups'/datetime.datetime.now().strftime('approach-isolation-repair-%Y%m%d-%H%M%S');backup.mkdir(parents=True);shutil.copy2(root/'Content/PiedmontRide/Maps/PiedmontWorld.umap',backup/'PiedmontWorld.umap')
assert ea.destroy_actor(patches[0]);s=next(a for a in ea.get_all_level_actors() if a.actor_has_tag('BattleKrog_2'));s.set_centerline(points);assert ids()==before-{patch_id}
assert unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).save_current_level()
(root/'Tests/Results/2026-09-13-approach-isolation-repair.json').write_text(json.dumps({'passed':True,'backup':str(backup),'restored_actor_count':len(ids()),'scope':'Removed only misplaced tagged approach actor and restored BattleKrog_2 from the untouched review copy. All original actor GUIDs preserved. Earlier wider-approach ride tested the baseline, not the intended candidate.'},indent=2)+'\n')
