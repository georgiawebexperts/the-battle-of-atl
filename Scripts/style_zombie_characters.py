"""Assign separate weathered clothing, pale undead skin and dark footwear."""
import unreal,json,pathlib
root=pathlib.Path(unreal.Paths.project_dir());rows=[]
palette={'Skin':(.19,.23,.14),'LightBlue':(.055,.085,.09),'Brown':(.12,.065,.023),'Beige':(.23,.20,.12),'Brown2':(.045,.038,.027),'Eyebrows':(.022,.025,.018),'Red':(.13,.022,.016),'Eye':(.8,.52,.025),'White':(.28,.25,.17),'Black':(.027,.032,.025),'Red_Dark':(.075,.024,.018),'Earrings':(.12,.13,.10)}
for name in ['Farmer','Punk']:
 mesh=unreal.load_asset('/Game/BattleForTheA/Zombies/'+name+'/'+name+'/SkeletalMeshes/'+name);assert mesh
 slots=list(mesh.get_editor_property('materials'))
 for slot in slots:
  key=str(slot.material_slot_name);color=palette[key];path='/Game/BattleForTheA/Zombies/Materials/M_UndeadV2_'+key
  m=unreal.load_asset(path)
  if m:
   slot.set_editor_property('material_interface',m);continue
  if not m:
   m=unreal.AssetToolsHelpers.get_asset_tools().create_asset('M_UndeadV2_'+key,'/Game/BattleForTheA/Zombies/Materials',unreal.Material,unreal.MaterialFactoryNew())
  c=unreal.MaterialEditingLibrary.create_material_expression(m,unreal.MaterialExpressionConstant3Vector);c.set_editor_property('constant',unreal.LinearColor(*color))
  dark=unreal.MaterialEditingLibrary.create_material_expression(m,unreal.MaterialExpressionConstant3Vector);dark.set_editor_property('constant',unreal.LinearColor(*(v*(.72 if key=='Skin' else .48) for v in color)))
  uv=unreal.MaterialEditingLibrary.create_material_expression(m,unreal.MaterialExpressionTextureCoordinate);uv.set_editor_property('u_tiling',2);uv.set_editor_property('v_tiling',2)
  noise=unreal.MaterialEditingLibrary.create_material_expression(m,unreal.MaterialExpressionNoise);noise.set_editor_property('quality',2);noise.set_editor_property('levels',1)
  unreal.MaterialEditingLibrary.connect_material_expressions(uv,'',noise,'Position')
  blend=unreal.MaterialEditingLibrary.create_material_expression(m,unreal.MaterialExpressionLinearInterpolate)
  for e,pin in [(dark,'A'),(c,'B'),(noise,'Alpha')]:unreal.MaterialEditingLibrary.connect_material_expressions(e,'',blend,pin)
  unreal.MaterialEditingLibrary.connect_material_property(blend,'',unreal.MaterialProperty.MP_BASE_COLOR)
  rough=unreal.MaterialEditingLibrary.create_material_expression(m,unreal.MaterialExpressionConstant);rough.set_editor_property('r',.9);unreal.MaterialEditingLibrary.connect_material_property(rough,'',unreal.MaterialProperty.MP_ROUGHNESS)
  m.set_editor_property('used_with_skeletal_mesh',True);unreal.MaterialEditingLibrary.recompile_material(m);assert unreal.EditorAssetLibrary.save_loaded_asset(m,only_if_is_dirty=False)
  slot.set_editor_property('material_interface',m)
 mesh.set_editor_property('materials',slots);assert unreal.EditorAssetLibrary.save_loaded_asset(mesh,only_if_is_dirty=False)
 rows.append({'name':name,'path':mesh.get_path_name(),'materials':[str(m.material_interface.get_path_name()) for m in mesh.get_editor_property('materials')]})
(root/'work/zombie-materials.json').write_text(json.dumps(rows,indent=2)+'\n');unreal.SystemLibrary.quit_editor()
