"""Owned procedural facade materials; no third-party photographs baked into assets."""
import unreal
DEST='/Game/BattleForTheA/Environment/FancyRoachMotel'
lib=unreal.MaterialEditingLibrary
colors={'Brick':(.32,.105,.05),'Stucco':(.7,.71,.68),'Trim':(.07,.085,.10),'Glass':(.075,.15,.20),'WarmGlass':(.35,.25,.12),'Roof':(.075,.08,.085)}
for kind,color in colors.items():
 mat=unreal.load_asset(DEST+'/M_'+kind);assert mat
 lib.delete_all_material_expressions(mat)
 if kind in ['Brick','Stucco','Roof']:
  node=lib.create_material_expression(mat,unreal.MaterialExpressionCustom)
  node.set_editor_property('output_type',unreal.CustomMaterialOutputType.CMOT_FLOAT3)
  inputs=[]
  for name in ['P','N']:
   inp=unreal.CustomInput();inp.set_editor_property('input_name',name);inputs.append(inp)
  node.set_editor_property('inputs',inputs)
  pos=lib.create_material_expression(mat,unreal.MaterialExpressionWorldPosition);normal=lib.create_material_expression(mat,unreal.MaterialExpressionVertexNormalWS)
  lib.connect_material_expressions(pos,'',node,'P');lib.connect_material_expressions(normal,'',node,'N')
  if kind=='Brick':
   code='''float2 uv=float2(abs(N.x)>abs(N.y)?P.y:P.x,P.z)/float2(23.0,7.6);
float row=floor(uv.y); uv.x+=fmod(abs(row),2.0)*0.5;
float2 cell=floor(uv); float2 f=frac(uv);
float2 edge=min(f,1-f); float mortar=1-smoothstep(0.03,0.07,min(edge.x,edge.y));
float variation=frac(sin(dot(cell,float2(12.9898,78.233)))*43758.5453);
float grain=frac(sin(dot(floor(P*1.2),float3(12.13,57.91,19.17)))*15731.743);
float3 brick=lerp(float3(.19,.052,.025),float3(.38,.15,.074),variation)*(0.9+grain*.17);
return lerp(brick,float3(.36,.32,.27),mortar);'''
  else:
   code='float grain=frac(sin(dot(floor(P*1.7),float3(12.13,57.91,19.17)))*15731.743); return float3('+','.join(str(c) for c in color)+')*(.96+.08*grain);'
  node.set_editor_property('code',code);lib.connect_material_property(node,'',unreal.MaterialProperty.MP_BASE_COLOR)
 else:
  node=lib.create_material_expression(mat,unreal.MaterialExpressionConstant3Vector);node.set_editor_property('constant',unreal.LinearColor(*color));lib.connect_material_property(node,'',unreal.MaterialProperty.MP_BASE_COLOR)
 for prop,value in [(unreal.MaterialProperty.MP_ROUGHNESS,.12 if 'Glass' in kind else .82),(unreal.MaterialProperty.MP_METALLIC,.35 if 'Glass' in kind else .5 if kind=='Trim' else 0)]:
  n=lib.create_material_expression(mat,unreal.MaterialExpressionConstant);n.set_editor_property('r',value);lib.connect_material_property(n,'',prop)
 if kind=='WarmGlass':
  n=lib.create_material_expression(mat,unreal.MaterialExpressionConstant3Vector);n.set_editor_property('constant',unreal.LinearColor(.09,.055,.018));lib.connect_material_property(n,'',unreal.MaterialProperty.MP_EMISSIVE_COLOR)
 lib.recompile_material(mat);assert unreal.EditorAssetLibrary.save_loaded_asset(mat,only_if_is_dirty=False)
# Keep Unreal's font atlas masking, but own the lighting and letter color.
textmat=unreal.load_asset(DEST+'/M_LandmarkLettering')
if not textmat:textmat=unreal.EditorAssetLibrary.duplicate_asset('/Engine/EngineMaterials/DefaultTextMaterialOpaque',DEST+'/M_LandmarkLettering')
assert textmat
textmat.set_editor_property('shading_model',unreal.MaterialShadingModel.MSM_UNLIT)
letter=lib.get_material_property_input_node(textmat,unreal.MaterialProperty.MP_EMISSIVE_COLOR)
if not isinstance(letter,unreal.MaterialExpressionConstant3Vector):letter=lib.create_material_expression(textmat,unreal.MaterialExpressionConstant3Vector)
letter.set_editor_property('constant',unreal.LinearColor(19,16.4,11.8));lib.connect_material_property(letter,'',unreal.MaterialProperty.MP_EMISSIVE_COLOR)
lib.recompile_material(textmat);assert unreal.EditorAssetLibrary.save_loaded_asset(textmat,only_if_is_dirty=False)
