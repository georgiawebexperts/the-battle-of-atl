"""Assign an authored police palette to the imported mesh's named FBX slots."""
import unreal,json,pathlib,re
root=pathlib.Path(unreal.Paths.project_dir())
mesh=unreal.load_asset('/Game/BattleForTheA/Police/Swat');assert mesh
palette={'Swat':(.025,.055,.10),'Swat_Black':(.012,.016,.023),'Visor':(.045,.12,.17)}
# Unreal array iteration yields struct copies; retain edits in a Python list.
slots=list(mesh.get_editor_property('materials'));rows=[]
for slot in slots:
 name=re.sub(r"[0-9]+$","",str(slot.material_slot_name))
 if name=='Skin':material=unreal.load_asset('/Game/PiedmontRide/Rider/Skin')
 else:
  assert name in palette,name
  path='/Game/BattleForTheA/Police/M_'+name
  material=unreal.load_asset(path)
  if not material:
   material=unreal.AssetToolsHelpers.get_asset_tools().create_asset('M_'+name,'/Game/BattleForTheA/Police',unreal.Material,unreal.MaterialFactoryNew())
   color=unreal.MaterialEditingLibrary.create_material_expression(material,unreal.MaterialExpressionConstant3Vector);color.set_editor_property('constant',unreal.LinearColor(*palette[name]));unreal.MaterialEditingLibrary.connect_material_property(color,'',unreal.MaterialProperty.MP_BASE_COLOR)
   rough=unreal.MaterialEditingLibrary.create_material_expression(material,unreal.MaterialExpressionConstant);rough.set_editor_property('r',.18 if name=='Visor' else .72);unreal.MaterialEditingLibrary.connect_material_property(rough,'',unreal.MaterialProperty.MP_ROUGHNESS)
   unreal.MaterialEditingLibrary.recompile_material(material)
 assert material,name
 if name!='Skin':
  material.set_editor_property('used_with_skeletal_mesh',True)
  unreal.MaterialEditingLibrary.recompile_material(material)
 slot.set_editor_property('material_interface',material);rows.append({'slot':name,'material':material.get_path_name()})
mesh.set_editor_property('materials',slots)
assert all(m.material_interface for m in mesh.get_editor_property('materials'))
assert unreal.EditorAssetLibrary.save_loaded_asset(mesh,only_if_is_dirty=False)
for m in mesh.get_editor_property('materials'):
 if m.material_interface.get_path_name().startswith('/Game/BattleForTheA/Police/'):
  assert unreal.EditorAssetLibrary.save_loaded_asset(m.material_interface,only_if_is_dirty=False)
unreal.EditorAssetLibrary.save_directory('/Game/BattleForTheA/Police',only_if_is_dirty=False,recursive=True)
(root/'work/police-materials.json').write_text(json.dumps(rows,indent=2)+'\n')
unreal.SystemLibrary.quit_editor()
