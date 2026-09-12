"""Original world-space gravel, weathered concrete, timber and steel surfaces."""
import unreal,json
from pathlib import Path
root=Path(unreal.Paths.project_dir());lib=unreal.MaterialEditingLibrary;dest='/Game/BattleForTheA/Environment/KrogRail'
common='''float fine=frac(sin(dot(floor(P*1.6),float3(12.13,57.91,19.17)))*15731.743);
float coarse=frac(sin(dot(floor(P/35.0),float3(37.1,19.3,73.7)))*4317.13);
'''
codes={
'Ballast':'''float2 uv=P.xy/4.5;float2 cell=floor(uv);float2 f=frac(uv);float nearest=9;float shade=0;
for(int x=-1;x<=1;x++){for(int y=-1;y<=1;y++){
float2 g=float2(x,y);float2 id=cell+g;
float2 jitter=frac(sin(float2(dot(id,float2(127.1,311.7)),dot(id,float2(269.5,183.3))))*43758.5453);
float2 delta=g+jitter-f;float d=dot(delta,delta);
if(d<nearest){nearest=d;shade=jitter.x;}}}
float3 stone=lerp(float3(.075,.072,.064),float3(.24,.22,.185),shade);
return stone*(.68+.32*saturate(1-nearest))*(.9+.16*fine);''',
'Retaining':'''float u=abs(N.x)>abs(N.y)?P.y:P.x;
float2 panel=frac(float2(u/300.0,P.z/110.0));float2 edge=min(panel,1-panel);
float joint=1-smoothstep(.003,.009,min(edge.x,edge.y));
float column=frac(sin(floor(u/9.0)*12.9898)*43758.5453);
float streak=smoothstep(.6,.95,column)*(.4+.6*saturate((1300-P.z)/400));
float mottling=.94+.1*sin(u*.031)*sin(P.z*.044)+.06*fine;
float3 concrete=float3(.22,.215,.20)*mottling;
concrete=lerp(concrete,float3(.08,.075,.061),streak*.45);
return lerp(concrete,float3(.08,.079,.073),joint*.7);''',
'Sleepers':'''float grain=.5+.5*sin(P.x*1.9+sin(P.y*.08)*2.0);
float split=pow(saturate(grain),18.0);
return lerp(float3(.095,.056,.028),float3(.038,.023,.013),split*.65)*(.78+.3*fine+.12*coarse);''',
'Steel':'''float wear=.5+.5*sin(P.x*.047+sin(P.y*.039)*2.0)*sin(P.y*.023+P.z*.07);
float rust=smoothstep(.58,.9,wear)*(.12+.88*(1-abs(N.z)));
return lerp(float3(.25,.27,.28),float3(.16,.065,.025),rust)*(.93+.1*fine);'''
}
for kind,code in codes.items():
 mat=unreal.load_asset(dest+'/M_KrogRail_'+kind);assert mat
 lib.delete_all_material_expressions(mat)
 node=lib.create_material_expression(mat,unreal.MaterialExpressionCustom);node.set_editor_property('output_type',unreal.CustomMaterialOutputType.CMOT_FLOAT3)
 inputs=[]
 for name in ['P','N']:
  i=unreal.CustomInput();i.set_editor_property('input_name',name);inputs.append(i)
 node.set_editor_property('inputs',inputs);node.set_editor_property('code',common+code)
 p=lib.create_material_expression(mat,unreal.MaterialExpressionWorldPosition);n=lib.create_material_expression(mat,unreal.MaterialExpressionVertexNormalWS)
 lib.connect_material_expressions(p,'',node,'P');lib.connect_material_expressions(n,'',node,'N');lib.connect_material_property(node,'',unreal.MaterialProperty.MP_BASE_COLOR)
 for prop,value in [(unreal.MaterialProperty.MP_ROUGHNESS,.5 if kind=='Steel' else .95),(unreal.MaterialProperty.MP_METALLIC,.7 if kind=='Steel' else 0)]:
  value_node=lib.create_material_expression(mat,unreal.MaterialExpressionConstant);value_node.r=value;lib.connect_material_property(value_node,'',prop)
 lib.recompile_material(mat);assert unreal.EditorAssetLibrary.save_loaded_asset(mat,only_if_is_dirty=False)
(root/'Tests/Results/2026-09-12-krog-rail-materials.json').write_text(json.dumps({'materials':list(codes),'author':'2026-09-12 [codex-maclaptop]','scope':'Original procedural color/roughness/metallic surfaces; no displacement or collision changes. Visual acceptance pending.'},indent=2)+'\n')
