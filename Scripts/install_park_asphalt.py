"""Apply visually reviewed asphalt material to the main park; geometry unchanged."""
import unreal,json,pathlib
root=pathlib.Path(unreal.Paths.project_dir())
m=unreal.load_asset('/Game/PiedmontRide/Materials/M_ParkAsphaltWorld');assert m
unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/PiedmontWorld')
ea=unreal.get_editor_subsystem(unreal.EditorActorSubsystem);changed=[]
for a in ea.get_all_level_actors():
 c=a.get_component_by_class(unreal.StaticMeshComponent)
 if c:
  for i,old in enumerate(c.get_materials()):
   if old and old.get_path_name()=='/Game/PiedmontRide/Materials/M_Asphalt.M_Asphalt':c.set_material(i,m);changed.append(a.get_actor_label())
assert changed
assert unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).save_current_level()
(root/'Tests/Results/2026-09-12-asphalt-install.json').write_text(json.dumps({'material':m.get_path_name(),'review_map_only':False,'actors':changed,'visual_accepted':True,'scope':'Procedural asphalt aggregate; no slab joints; no geometry changes'},indent=2)+'\n')
