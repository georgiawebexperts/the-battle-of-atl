import unreal,json
from pathlib import Path
root=Path(unreal.Paths.project_dir());rows=[]
for name in ['/Game/PiedmontRide/Materials/M_Asphalt','/Game/PiedmontRide/Materials/M_Concrete','/Game/BattleForTheA/Materials/M_ColaRed']:
 m=unreal.load_asset(name);assert m,name
 while isinstance(m,unreal.MaterialInstance):m=m.get_editor_property('parent')
 before=m.get_editor_property('used_with_instanced_static_meshes');m.set_editor_property('used_with_instanced_static_meshes',True);unreal.MaterialEditingLibrary.recompile_material(m);assert unreal.EditorAssetLibrary.save_loaded_asset(m)
 rows.append({'asset':m.get_path_name(),'before':before,'after':m.get_editor_property('used_with_instanced_static_meshes')})
(root/'work/home-material-usage.json').write_text(json.dumps(rows,indent=2)+'\n')
unreal.SystemLibrary.quit_editor()
