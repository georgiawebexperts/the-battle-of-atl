"""Original generated street art, projected onto review tunnel vertical surfaces."""
import json
from pathlib import Path
import unreal
root=Path(unreal.Paths.project_dir());dest='/Game/BattleForTheA/Environment/KrogArt'
task=unreal.AssetImportTask();task.filename=str(root/'SourceAssets/Textures/Krog/krog-original-street-art-v1.png')
task.destination_path=dest;task.destination_name='T_KrogOriginalStreetArt_v1';task.automated=True;task.save=True;task.replace_existing=True
unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task]);texture=unreal.load_asset(dest+'/T_KrogOriginalStreetArt_v1');assert texture
name='M_KrogOriginalStreetArt_v1';m=unreal.load_asset(dest+'/'+name)
if not m:m=unreal.AssetToolsHelpers.get_asset_tools().create_asset(name,dest,unreal.Material,unreal.MaterialFactoryNew())
lib=unreal.MaterialEditingLibrary;lib.delete_all_material_expressions(m)
def node(cls,**props):
 e=lib.create_material_expression(m,cls)
 for k,v in props.items():e.set_editor_property(k,v)
 return e
def wire(a,b,pin,out=''):assert lib.connect_material_expressions(a,out,b,pin)
def val(x):return node(unreal.MaterialExpressionConstant,r=x)
def binary(cls,a,b):
 e=node(cls);wire(a,e,'A');wire(b,e,'B');return e
world=node(unreal.MaterialExpressionWorldPosition)
u=binary(unreal.MaterialExpressionDotProduct,world,node(unreal.MaterialExpressionConstant3Vector,constant=unreal.LinearColor(0,1/900,0)))
x=binary(unreal.MaterialExpressionDotProduct,world,node(unreal.MaterialExpressionConstant3Vector,constant=unreal.LinearColor(1/900,0,0)))
v=binary(unreal.MaterialExpressionDotProduct,world,node(unreal.MaterialExpressionConstant3Vector,constant=unreal.LinearColor(0,0,-1/300)))
uv=binary(unreal.MaterialExpressionAppendVector,u,v)
sample=node(unreal.MaterialExpressionTextureSample,texture=texture);wire(uv,sample,'UVs')
normal=node(unreal.MaterialExpressionPixelNormalWS)
uv_x=binary(unreal.MaterialExpressionAppendVector,x,v)
sample_x=node(unreal.MaterialExpressionTextureSample,texture=texture);wire(uv_x,sample_x,'UVs')
def normal_axis(axis):
 e=node(unreal.MaterialExpressionComponentMask,r=axis==0,g=axis==1,b=False,a=False);wire(normal,e,'')
 a=node(unreal.MaterialExpressionAbs);wire(e,a,'');return a
nx=normal_axis(0);ny=normal_axis(1)
denominator=binary(unreal.MaterialExpressionAdd,binary(unreal.MaterialExpressionAdd,nx,ny),val(.0001))
side_weight=binary(unreal.MaterialExpressionDivide,nx,denominator)
wall_color=node(unreal.MaterialExpressionLinearInterpolate);wire(sample_x,wall_color,'A','RGB');wire(sample,wall_color,'B','RGB');wire(side_weight,wall_color,'Alpha')
vertical=node(unreal.MaterialExpressionComponentMask,r=False,g=False,b=True,a=False);wire(normal,vertical,'')
absolute=node(unreal.MaterialExpressionAbs);wire(vertical,absolute,'')
blend=node(unreal.MaterialExpressionLinearInterpolate)
wire(wall_color,blend,'A');wire(node(unreal.MaterialExpressionConstant3Vector,constant=unreal.LinearColor(.19,.18,.16)),blend,'B');wire(absolute,blend,'Alpha')
assert lib.connect_material_property(blend,'',unreal.MaterialProperty.MP_BASE_COLOR)
assert lib.connect_material_property(val(.92),'',unreal.MaterialProperty.MP_ROUGHNESS)
assert lib.connect_material_property(val(.1),'',unreal.MaterialProperty.MP_SPECULAR)
m.set_editor_property('used_with_nanite',True);lib.recompile_material(m);assert unreal.EditorAssetLibrary.save_loaded_asset(m)
assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/PiedmontKrogRoadReview')
ea=unreal.get_editor_subsystem(unreal.EditorActorSubsystem);changed=[]
for actor in ea.get_all_level_actors():
 if actor.get_actor_label() in ['Krog route SM_KrogTunnel_Shell','Krog route SM_KrogTunnel_Columns']:
  actor.static_mesh_component.set_material(0,m);changed.append(actor.get_actor_label())
assert len(changed)==2
unreal.PiedmontWorldTools.finish_editor_asset_loading();assert unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).save_current_level()
(root/'Tests/Results/2026-09-12-krog-street-art.json').write_text(json.dumps({'material':m.get_path_name(),'actors':changed,'main_map_changed':False,'visual_review':'pending','scope':'Original generated texture and review-only material overrides, no collision changes.'},indent=2)+'\n')
