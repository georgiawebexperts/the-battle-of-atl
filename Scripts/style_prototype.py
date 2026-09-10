import unreal,pathlib,json
colors={'Pavement':(.18,.20,.22),'Grass':(.16,.32,.17),'Water':(.025,.16,.28),'Bike':(.02,.52,.72),'Rubber':(.02,.02,.025),'Scooter':(.9,.27,.04)}
mats={};ats=unreal.AssetToolsHelpers.get_asset_tools()
for name,rgb in colors.items():
    path='/Game/BeltLineGlide/Materials/M_'+name
    mat=unreal.load_asset(path)
    if not mat:
        mat=ats.create_asset('M_'+name,'/Game/BeltLineGlide/Materials',unreal.Material,unreal.MaterialFactoryNew())
        color=unreal.MaterialEditingLibrary.create_material_expression(mat,unreal.MaterialExpressionConstant3Vector,-200,0)
        color.set_editor_property('constant',unreal.LinearColor(r=rgb[0],g=rgb[1],b=rgb[2],a=1))
        unreal.MaterialEditingLibrary.connect_material_property(color,'',unreal.MaterialProperty.MP_BASE_COLOR)
        rough=unreal.MaterialEditingLibrary.create_material_expression(mat,unreal.MaterialExpressionConstant,-200,200);rough.set_editor_property('r',.75)
        unreal.MaterialEditingLibrary.connect_material_property(rough,'',unreal.MaterialProperty.MP_ROUGHNESS)
        unreal.MaterialEditingLibrary.recompile_material(mat);unreal.EditorAssetLibrary.save_loaded_asset(mat)
    mats[name]=mat
counts={}
for a in unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors():
    label=a.get_actor_label();kind=None
    if label.startswith('LakeLoopSeg_') and '_Floor_' in label:kind='Pavement'
    elif 'MappedTrail' in label and '_Floor_' in label:kind='Pavement'
    elif label=='LakeWater_Floor_01':kind='Water'
    elif label=='ParkGround' or 'ParkGround_' in label:kind='Grass'
    elif label.startswith('Glide_Bike'):kind='Rubber' if 'Wheel' in label else 'Bike'
    if kind:
        for c in a.get_components_by_class(unreal.MeshComponent):c.set_material(0,mats[kind]);counts[kind]=counts.get(kind,0)+1
unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).save_current_level()
pathlib.Path(unreal.Paths.project_dir(),'Scripts','style-result.json').write_text(json.dumps(counts,indent=2))
