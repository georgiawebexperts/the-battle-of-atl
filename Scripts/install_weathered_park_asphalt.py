"""Apply reviewed weathered asphalt only to park pavement actors."""
import unreal,json,pathlib
root=pathlib.Path(unreal.Paths.project_dir()).resolve();path='/Game/PiedmontRide/Materials/M_ParkAsphaltWeatheredCandidate';m=unreal.load_asset(path);assert m
assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/PiedmontWorld');unreal.PiedmontWorldTools.finish_editor_asset_loading()
ea=unreal.get_editor_subsystem(unreal.EditorActorSubsystem);changed=[]
for a in ea.get_all_level_actors():
 if not a.get_actor_label().startswith('Park pavement SM_Park_Asphalt_'):continue
 c=a.get_component_by_class(unreal.StaticMeshComponent);assert c;c.set_material(0,m);changed.append(a.get_actor_label())
assert changed
assert unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).save_current_level()
assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/PiedmontWorld');unreal.PiedmontWorldTools.finish_editor_asset_loading()
verified=[]
for a in ea.get_all_level_actors():
 if a.get_actor_label() in changed:
  assert a.get_component_by_class(unreal.StaticMeshComponent).get_material(0).get_path_name()==m.get_path_name();verified.append(a.get_actor_label())
assert set(changed)==set(verified)
(root/'Tests/Results/2026-09-12-weathered-park-install.json').write_text(json.dumps({'material':path,'actors':verified,'passed':True,'scope':'Saved and reloaded park pavement material overrides only; native gate static render reviewed, whole park/night review pending; no packaged update'},indent=2))
