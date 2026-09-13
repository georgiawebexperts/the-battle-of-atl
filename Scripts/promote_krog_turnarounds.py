"""Transfer only verified traffic-lane settings into the current main map."""
import unreal,json,shutil,datetime,hashlib
from pathlib import Path
root=Path(unreal.Paths.project_dir()).resolve()
assert json.loads((root/'Tests/Results/2026-09-13-krog-turnaround-mixed-clearance.json').read_text())['passed']
assert json.loads((root/'Tests/Results/2026-09-13-krog-turnaround-render.json').read_text())['visual_accepted']
ea=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
def director():
    found=[a for a in ea.get_all_level_actors() if isinstance(a,unreal.BattleRoadTrafficDirector) and a.actor_has_tag('KrogTrafficReview')]
    assert len(found)==1,len(found)
    return found[0]
assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/PiedmontKrogTurnaroundReview')
d=director();lanes=[]
for l in d.get_editor_property('Lanes'):
    bindings=[(b.get_editor_property('StopDistance'),b.get_editor_property('Crossing').get_actor_location()) for b in l.get_editor_property('Crossings')]
    lanes.append((l.get_editor_property('Name'),list(l.get_editor_property('Points')),l.get_editor_property('CruiseSpeed'),l.get_editor_property('bLoopRoute'),bindings))
assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/PiedmontWorld')
d=director();gate=d.get_editor_property('Lanes')[0].get_editor_property('Crossings')[0].get_editor_property('Crossing');assert gate
source=root/'Content/PiedmontRide/Maps/PiedmontWorld.umap';backup=root/'work/map-backups'/datetime.datetime.now().strftime('pre-turnarounds-%Y%m%d-%H%M%S');backup.mkdir(parents=True,exist_ok=False);backup=backup/source.name;shutil.copy2(source,backup)
before={str(tuple(a.get_editor_property('actor_guid').get_editor_property(k) for k in ('a','b','c','d'))) for a in ea.get_all_level_actors()};updated=[]
for name,points,speed,loop,bindings in lanes:
    lane=unreal.BattleRoadTrafficLane();lane.set_editor_property('Name',name);lane.set_editor_property('Points',points);lane.set_editor_property('CruiseSpeed',speed);lane.set_editor_property('bLoopRoute',loop);new_bindings=[]
    for stop,location in bindings:
        assert (location-gate.get_actor_location()).length()<.1,'Crossing moved since review'
        b=unreal.BattleCarCrossing();b.set_editor_property('Crossing',gate);b.set_editor_property('StopDistance',stop);new_bindings.append(b)
    lane.set_editor_property('Crossings',new_bindings);updated.append(lane)
d.set_editor_property('Lanes',updated);d.set_editor_property('MaxCarsPerLane',6);d.set_actor_label('DeKalb traffic with construction turnarounds')
assert before=={str(tuple(a.get_editor_property('actor_guid').get_editor_property(k) for k in ('a','b','c','d'))) for a in ea.get_all_level_actors()}
assert unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).save_current_level()
(root/'Tests/Results/2026-09-13-krog-turnaround-promotion.json').write_text(json.dumps({'passed':True,'main_map_changed':True,'backup':str(backup),'backup_sha256':hashlib.sha256(backup.read_bytes()).hexdigest(),'actor_guids_preserved':True,'scope':'Traffic lane settings transferred onto existing main director and crossing only. Fresh main/cooked acceptance pending.'},indent=2)+'\n')
