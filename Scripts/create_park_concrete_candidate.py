"""Create a world-mapped concrete candidate and apply only to the sleeper review map."""
import unreal,json,pathlib
root=pathlib.Path(unreal.Paths.project_dir());lib=unreal.MaterialEditingLibrary
path='/Game/PiedmontRide/Materials/M_ParkConcreteWorld'
assert not unreal.EditorAssetLibrary.does_asset_exist(path),'Inspect existing candidate before rebuilding'
m=unreal.AssetToolsHelpers.get_asset_tools().create_asset('M_ParkConcreteWorld','/Game/PiedmontRide/Materials',unreal.Material,unreal.MaterialFactoryNew())
def node(cls,**props):
 e=lib.create_material_expression(m,cls)
 for k,v in props.items():e.set_editor_property(k,v)
 return e
def wire(a,b,pin,out=''):assert lib.connect_material_expressions(a,out,b,pin)
def val(x):return node(unreal.MaterialExpressionConstant,r=x)
def op(cls,a,b):
 e=node(cls);wire(a,e,'A');wire(b,e,'B');return e
def mul(a,b):return op(unreal.MaterialExpressionMultiply,a,b)
def add(a,b):return op(unreal.MaterialExpressionAdd,a,b)
def unary(cls,a):
 e=node(cls);wire(a,e,'');return e
world=node(unreal.MaterialExpressionWorldPosition)
# Two inexpensive arithmetic hash scales give aggregate variation without texture downloads.
def grain(scale):
 dot=node(unreal.MaterialExpressionDotProduct);wire(mul(world,val(scale)),dot,'A');wire(node(unreal.MaterialExpressionConstant3Vector,constant=unreal.LinearColor(12.9898,78.233,37.719)),dot,'B')
 return unary(unreal.MaterialExpressionFrac,mul(unary(unreal.MaterialExpressionSine,dot),val(43758.5453)))
fine=grain(.3);coarse=grain(.012)
shade=add(val(.76),add(mul(fine,val(.12)),mul(coarse,val(.12))))
# Narrow joints every 180cm in world XY; widths remain stable on irregular UV layouts.
joints=[]
for axis in ['r','g']:
 mask=node(unreal.MaterialExpressionComponentMask,r=axis=='r',g=axis=='g',b=False,a=False);wire(world,mask,'')
 f=unary(unreal.MaterialExpressionFrac,mul(mask,val(1/180)))
 edge=op(unreal.MaterialExpressionMin,f,op(unreal.MaterialExpressionSubtract,val(1),f))
 joints.append(unary(unreal.MaterialExpressionSaturate,mul(edge,val(180/0.7))))
joint=op(unreal.MaterialExpressionMin,*joints)
shade=mul(shade,add(val(.58),mul(joint,val(.42))))
color=mul(node(unreal.MaterialExpressionConstant3Vector,constant=unreal.LinearColor(.27,.255,.23)),shade)
assert lib.connect_material_property(color,'',unreal.MaterialProperty.MP_BASE_COLOR)
assert lib.connect_material_property(add(val(.82),mul(fine,val(.12))),'',unreal.MaterialProperty.MP_ROUGHNESS)
assert lib.connect_material_property(val(.18),'',unreal.MaterialProperty.MP_SPECULAR)
m.set_editor_property('used_with_instanced_static_meshes',True);m.set_editor_property('used_with_nanite',True);lib.recompile_material(m)
assert unreal.EditorAssetLibrary.save_loaded_asset(m)
unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/PiedmontSleeperReview')
ea=unreal.get_editor_subsystem(unreal.EditorActorSubsystem);changed=[]
for a in ea.get_all_level_actors():
 c=a.get_component_by_class(unreal.StaticMeshComponent)
 if c:
  for i,old in enumerate(c.get_materials()):
   if old and old.get_path_name()=='/Game/PiedmontRide/Materials/M_Concrete.M_Concrete':c.set_material(i,m);changed.append(a.get_actor_label())
assert changed
assert unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).save_current_level()
(root/'Tests/Results/2026-09-12-concrete-candidate.json').write_text(json.dumps({'material':path,'review_map_only':True,'actors':changed,'visual_accepted':False,'scope':'Procedural aggregate and 180cm joints; no geometry changes'},indent=2)+'\n')
