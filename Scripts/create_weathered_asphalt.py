"""Original weathered asphalt material candidate; never changes a saved map."""
import unreal,json,pathlib
root=pathlib.Path(unreal.Paths.project_dir()).resolve();lib=unreal.MaterialEditingLibrary
name='M_ParkAsphaltWeatheredCandidate';dest='/Game/PiedmontRide/Materials';m=unreal.load_asset(dest+'/'+name)
if not m:m=unreal.AssetToolsHelpers.get_asset_tools().create_asset(name,dest,unreal.Material,unreal.MaterialFactoryNew())
lib.delete_all_material_expressions(m)
def node(cls,**props):
 e=lib.create_material_expression(m,cls)
 for k,v in props.items():e.set_editor_property(k,v)
 return e
def wire(a,b,pin):assert lib.connect_material_expressions(a,'',b,pin)
def val(v):return node(unreal.MaterialExpressionConstant,r=v)
def op(cls,a,b):
 e=node(cls);wire(a,e,'A');wire(b,e,'B');return e
def mul(a,b):return op(unreal.MaterialExpressionMultiply,a,b)
def add(a,b):return op(unreal.MaterialExpressionAdd,a,b)
world=node(unreal.MaterialExpressionWorldPosition)
def grain(scale):
 dot=op(unreal.MaterialExpressionDotProduct,mul(world,val(scale)),node(unreal.MaterialExpressionConstant3Vector,constant=unreal.LinearColor(12.9898,78.233,37.719)))
 sine=node(unreal.MaterialExpressionSine);wire(dot,sine,'');fract=node(unreal.MaterialExpressionFrac);wire(mul(sine,val(43758.5453)),fract,'');return fract
fine=grain(.3);coarse=grain(.012);shade=add(val(.80),add(mul(fine,val(.10)),mul(coarse,val(.10))))
color=mul(node(unreal.MaterialExpressionConstant3Vector,constant=unreal.LinearColor(.13,.135,.14)),shade)
assert lib.connect_material_property(color,'',unreal.MaterialProperty.MP_BASE_COLOR)
assert lib.connect_material_property(add(val(.85),mul(fine,val(.08))),'',unreal.MaterialProperty.MP_ROUGHNESS)
assert lib.connect_material_property(val(.08),'',unreal.MaterialProperty.MP_SPECULAR)
m.set_editor_property('used_with_nanite',True);m.set_editor_property('used_with_instanced_static_meshes',True);lib.recompile_material(m)
unreal.PiedmontWorldTools.finish_editor_asset_loading();assert unreal.EditorAssetLibrary.save_loaded_asset(m)
