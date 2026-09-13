"""Create a soft, dark-red unlit spray card material without external textures."""
import unreal
lib=unreal.MaterialEditingLibrary
path='/Game/PiedmontRide/Materials';name='M_BloodSpray'
m=unreal.load_asset(path+'/'+name) or unreal.AssetToolsHelpers.get_asset_tools().create_asset(name,path,unreal.Material,unreal.MaterialFactoryNew())
lib.delete_all_material_expressions(m)
m.set_editor_property('blend_mode',unreal.BlendMode.BLEND_TRANSLUCENT)
m.set_editor_property('shading_model',unreal.MaterialShadingModel.MSM_UNLIT)
m.set_editor_property('two_sided',True)
def node(cls):return lib.create_material_expression(m,cls)
uv=node(unreal.MaterialExpressionTextureCoordinate)
center=node(unreal.MaterialExpressionConstant2Vector);center.r=.5;center.g=.5
mask=node(unreal.MaterialExpressionSphereMask);mask.set_editor_property('attenuation_radius',.5);mask.set_editor_property('hardness_percent',45)
lib.connect_material_expressions(uv,'',mask,'A');lib.connect_material_expressions(center,'',mask,'B')
fade=node(unreal.MaterialExpressionScalarParameter);fade.set_editor_property('parameter_name','Opacity');fade.set_editor_property('default_value',1)
alpha=node(unreal.MaterialExpressionMultiply);lib.connect_material_expressions(mask,'',alpha,'A');lib.connect_material_expressions(fade,'',alpha,'B');lib.connect_material_property(alpha,'',unreal.MaterialProperty.MP_OPACITY)
color=node(unreal.MaterialExpressionConstant3Vector);color.constant=unreal.LinearColor(.04,.0001,.0001)
lib.connect_material_property(color,'',unreal.MaterialProperty.MP_EMISSIVE_COLOR)
lib.recompile_material(m);unreal.EditorAssetLibrary.save_loaded_asset(m)
unreal.PiedmontWorldTools.finish_editor_asset_loading();unreal.SystemLibrary.quit_editor()
