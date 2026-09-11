import unreal,pathlib
root=pathlib.Path(unreal.Paths.project_dir());lib=unreal.MaterialEditingLibrary;tools=unreal.AssetToolsHelpers.get_asset_tools()
for name,color in [('M_Zombie',(.18,.30,.22)),('M_ZombieEye',(1,12,.15))]:
 m=unreal.load_asset('/Game/BattleForTheA/Materials/'+name)
 if not m:m=tools.create_asset(name,'/Game/BattleForTheA/Materials',unreal.Material,unreal.MaterialFactoryNew())
 lib.delete_all_material_expressions(m)
 c=lib.create_material_expression(m,unreal.MaterialExpressionConstant3Vector);c.set_editor_property('constant',unreal.LinearColor(*color))
 lib.connect_material_property(c,'',unreal.MaterialProperty.MP_EMISSIVE_COLOR if name.endswith('Eye') else unreal.MaterialProperty.MP_BASE_COLOR)
 if name=='M_Zombie':
  m.set_editor_property('used_with_skeletal_mesh',True)
  m.set_editor_property('blend_mode',unreal.BlendMode.BLEND_MASKED)
  dissolve=lib.create_material_expression(m,unreal.MaterialExpressionScalarParameter);dissolve.set_editor_property('parameter_name','Dissolve');dissolve.set_editor_property('default_value',-.1)
  noise=lib.create_material_expression(m,unreal.MaterialExpressionNoise);noise.set_editor_property('scale',.2);noise.set_editor_property('output_min',0);noise.set_editor_property('output_max',1)
  subtract=lib.create_material_expression(m,unreal.MaterialExpressionSubtract);lib.connect_material_expressions(noise,'',subtract,'A');lib.connect_material_expressions(dissolve,'',subtract,'B');lib.connect_material_property(subtract,'',unreal.MaterialProperty.MP_OPACITY_MASK)
  m.set_editor_property('opacity_mask_clip_value',.01)
 lib.recompile_material(m);unreal.EditorAssetLibrary.save_loaded_asset(m)
t=unreal.AssetImportTask();t.filename=str(root/'SourceAssets/Audio/S_ZombieGrowl.wav');t.destination_path='/Game/BattleForTheA/Audio';t.destination_name='S_ZombieGrowl';t.automated=True;t.save=True;t.replace_existing=True;tools.import_asset_tasks([t])
