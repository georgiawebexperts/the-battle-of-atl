"""Replace the flat grass shader with a sourced ground surface; preserve geometry/physics."""
import unreal,json
from pathlib import Path
root=Path(unreal.Paths.project_dir());dest='/Game/PiedmontRide/Materials/Ground';textures={};lib=unreal.MaterialEditingLibrary
for key in ['diff','nor_gl','rough']:
 t=unreal.AssetImportTask();t.filename=str(root/'SourceAssets/Environment/LeafyGrass'/f'leafy_grass_{key}_2k.jpg');t.destination_path=dest;t.destination_name='T_LeafyGrass_'+key;t.automated=True;t.replace_existing=True;t.save=True
 unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([t]);tex=unreal.load_asset(dest+'/'+t.destination_name);assert tex
 tex.set_editor_property('srgb',key=='diff');tex.set_editor_property('lod_group',unreal.TextureGroup.TEXTUREGROUP_WORLD_NORMAL_MAP if key=='nor_gl' else unreal.TextureGroup.TEXTUREGROUP_WORLD)
 if key=='nor_gl':tex.set_editor_property('compression_settings',unreal.TextureCompressionSettings.TC_NORMALMAP);tex.set_editor_property('flip_green_channel',True)
 if key=='rough':tex.set_editor_property('compression_settings',unreal.TextureCompressionSettings.TC_MASKS)
 unreal.EditorAssetLibrary.save_loaded_asset(tex,only_if_is_dirty=False);textures[key]=tex
m=unreal.load_asset('/Game/PiedmontRide/Materials/M_Grass');assert m
before=lib.get_num_material_expressions(m)
# Keep old nodes disconnected: deleting the loaded graph asserts in UE 5.8 commandlet mode.
def node(cls,**props):
 e=lib.create_material_expression(m,cls)
 for k,v in props.items():e.set_editor_property(k,v)
 return e
def wire(a,b,pin,out=''):assert lib.connect_material_expressions(a,out,b,pin),(a,b,pin,out)
def scalar(v):return node(unreal.MaterialExpressionConstant,r=v)
def op(cls,a,b):
 e=node(cls);wire(a,e,'A');wire(b,e,'B');return e
def mul(a,b):return op(unreal.MaterialExpressionMultiply,a,b)
def add(a,b):return op(unreal.MaterialExpressionAdd,a,b)
def sample(tex,uv,kind):
 e=node(unreal.MaterialExpressionTextureSample,texture=tex,sampler_type=kind);wire(uv,e,'UVs');return e
world=node(unreal.MaterialExpressionWorldPosition)
xy=node(unreal.MaterialExpressionComponentMask,r=True,g=True,b=False,a=False);wire(world,xy,'')
uv=mul(xy,scalar(1/200))
base=sample(textures['diff'],uv,unreal.MaterialSamplerType.SAMPLERTYPE_COLOR)
macro_uv=add(mul(xy,scalar(1/4700)),node(unreal.MaterialExpressionConstant2Vector,r=.371,g=.683))
macro=sample(textures['diff'],macro_uv,unreal.MaterialSamplerType.SAMPLERTYPE_COLOR)
macro_r=node(unreal.MaterialExpressionComponentMask,r=True,g=False,b=False,a=False);wire(macro,macro_r,'','RGB')
variation=add(scalar(.80),mul(macro_r,scalar(1.2)))
tint=node(unreal.MaterialExpressionConstant3Vector,constant=unreal.LinearColor(.50,.82,.45))
color=mul(mul(base,tint),variation);assert lib.connect_material_property(color,'',unreal.MaterialProperty.MP_BASE_COLOR)
normal=sample(textures['nor_gl'],uv,unreal.MaterialSamplerType.SAMPLERTYPE_NORMAL)
flat=node(unreal.MaterialExpressionConstant3Vector,constant=unreal.LinearColor(0,0,1))
blend=node(unreal.MaterialExpressionLinearInterpolate,const_alpha=.55);wire(flat,blend,'A');wire(normal,blend,'B','RGB')
normalize=node(unreal.MaterialExpressionNormalize);wire(blend,normalize,'');assert lib.connect_material_property(normalize,'',unreal.MaterialProperty.MP_NORMAL)
rough=sample(textures['rough'],uv,unreal.MaterialSamplerType.SAMPLERTYPE_MASKS)
rough_r=node(unreal.MaterialExpressionComponentMask,r=True,g=False,b=False,a=False);wire(rough,rough_r,'','RGB')
roughness=add(scalar(.78),mul(rough_r,scalar(.2)));assert lib.connect_material_property(roughness,'',unreal.MaterialProperty.MP_ROUGHNESS)
assert lib.connect_material_property(scalar(.15),'',unreal.MaterialProperty.MP_SPECULAR)
m.set_editor_property('used_with_instanced_static_meshes',True);lib.layout_material_expressions(m);lib.recompile_material(m)
assert unreal.EditorAssetLibrary.save_loaded_asset(m,only_if_is_dirty=False)
(root/'work/ground-material-import.json').write_text(json.dumps({'material':m.get_path_name(),'expressions_before':before,'expressions_after':lib.get_num_material_expressions(m),'textures':{k:v.get_path_name() for k,v in textures.items()},'world_tile_cm':200,'macro_tile_cm':4700,'normal_strength':.55,'geometry_modified':False},indent=2)+'\n')
