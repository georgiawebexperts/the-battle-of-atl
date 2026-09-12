"""Restore main-world traffic to the Krog candidate without replacing the main map."""
import unreal,json
from pathlib import Path
root=Path(unreal.Paths.project_dir());ea=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/PiedmontWorld')
main_labels={a.get_actor_label():a.get_class().get_name() for a in ea.get_all_level_actors()}
directors=[]
for a in ea.get_all_level_actors():
 if not isinstance(a,unreal.BattleRoadTrafficDirector):continue
 lanes=[]
 for lane in a.get_editor_property('Lanes'):
  bindings=[]
  for b in lane.get_editor_property('Crossings'):
   gate=b.get_editor_property('Crossing');assert gate
   bindings.append({'label':gate.get_actor_label(),'stop':b.get_editor_property('StopDistance'),'location':gate.get_actor_location(),'settings':{p:gate.get_editor_property(p) for p in ['bAutoCycle','GreenSeconds','AmberSeconds','RedSeconds']}})
  lanes.append({'name':lane.get_editor_property('Name'),'points':list(lane.get_editor_property('Points')),'speed':lane.get_editor_property('CruiseSpeed'),'bindings':bindings})
 directors.append({'label':a.get_actor_label(),'tags':list(a.tags),'transform':a.get_actor_transform(),'lanes':lanes,'settings':{p:a.get_editor_property(p) for p in ['MaxCars','MaxCarsPerLane','SpawnInterval']}})
assert len(directors)>=3
assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/PiedmontKrogPopulationReview')
actors=ea.get_all_level_actors();by_label={a.get_actor_label():a for a in actors};restored=[]
for row in directors:
 assert row['label'] not in by_label,row['label']
 lanes=[]
 for saved in row['lanes']:
  lane=unreal.BattleRoadTrafficLane();lane.set_editor_property('Name',saved['name']);lane.set_editor_property('Points',saved['points']);lane.set_editor_property('CruiseSpeed',saved['speed']);bindings=[]
  for b in saved['bindings']:
   gate=by_label[b['label']];assert isinstance(gate,unreal.BattleRoadCrossing)
   here=gate.get_actor_location();assert max(abs(getattr(here,k)-getattr(b['location'],k)) for k in ['x','y','z'])<.01
   for p,value in b['settings'].items():assert gate.get_editor_property(p)==value,(b['label'],p)
   bind=unreal.BattleCarCrossing();bind.set_editor_property('Crossing',gate);bind.set_editor_property('StopDistance',b['stop']);bindings.append(bind)
  lane.set_editor_property('Crossings',bindings);lanes.append(lane)
 d=ea.spawn_actor_from_class(unreal.BattleRoadTrafficDirector,row['transform'].translation);d.set_actor_transform(row['transform'],False,False);d.set_actor_label(row['label']);d.tags=row['tags'];d.set_editor_property('Lanes',lanes)
 for p,v in row['settings'].items():d.set_editor_property(p,v)
 restored.append({'label':row['label'],'lanes':len(lanes),**row['settings']})
current={a.get_actor_label():a.get_class().get_name() for a in ea.get_all_level_actors()}
missing=[{'label':label,'class':cls} for label,cls in main_labels.items() if label not in current]
changed=[{'label':label,'old':cls,'new':current[label]} for label,cls in main_labels.items() if label in current and cls!=current[label]]
report={'restored_traffic':restored,'directors_total':sum(isinstance(a,unreal.BattleRoadTrafficDirector) for a in ea.get_all_level_actors()),'main_labels_missing_in_candidate':missing,'main_label_class_changes':changed,'main_map_changed':False,'scope':'Combined candidate with original traffic settings and gate references restored. Label inventory is a diagnostic, not full property equivalence. Runtime and performance pending.'}
(root/'Tests/Results/2026-09-12-krog-world-review.json').write_text(json.dumps(report,indent=2)+'\n')
assert not changed,changed
assert all(r['label']=='Krog route SM_KrogRoute_Concrete_7_1' and r['class']=='StaticMeshActor' for r in missing),missing
assert unreal.EditorLoadingAndSavingUtils.save_map(unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world(),'/Game/PiedmontRide/Maps/PiedmontKrogWorldReview')
