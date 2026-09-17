import unreal, pathlib, json
root=pathlib.Path(unreal.Paths.project_dir());dest='/Game/BattleForTheA/Spirit';name='M_SpectralBlackBear'
tools=unreal.AssetToolsHelpers.get_asset_tools();lib=unreal.MaterialEditingLibrary
mat=unreal.load_asset(dest+'/'+name)
if not mat: mat=tools.create_asset(name,dest,unreal.Material,unreal.MaterialFactoryNew())
assert mat
lib.delete_all_material_expressions(mat)
mat.set_editor_property('blend_mode',unreal.BlendMode.BLEND_MASKED)
mat.set_editor_property('shading_model',unreal.MaterialShadingModel.MSM_UNLIT)
mat.set_editor_property('two_sided',True)
mat.set_editor_property('opacity_mask_clip_value',0.04)
tex=unreal.load_asset(dest+'/T_SpectralBlackBear');assert tex
sample=lib.create_material_expression(mat,unreal.MaterialExpressionTextureSample,-350,0)
sample.set_editor_property('texture',tex);sample.set_editor_property('sampler_type',unreal.MaterialSamplerType.SAMPLERTYPE_COLOR)
lib.connect_material_property(sample,'RGB',unreal.MaterialProperty.MP_EMISSIVE_COLOR)
lib.connect_material_property(sample,'RGB',unreal.MaterialProperty.MP_BASE_COLOR)
lib.connect_material_property(sample,'A',unreal.MaterialProperty.MP_OPACITY_MASK)
lib.recompile_material(mat);assert unreal.EditorAssetLibrary.save_loaded_asset(mat,False)
(root/'Scripts/spirit-bear-material.json').write_text(json.dumps({'material':mat.get_path_name(),'texture':tex.get_path_name(),'blend':'masked','opacity_mask_clip':0.04,'unlit':True,'two_sided':True},indent=2))
