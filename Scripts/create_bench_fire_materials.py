"""Build lightweight interpolated fire/smoke flipbook materials from bundled textures."""
import unreal,json,pathlib
root=pathlib.Path(unreal.Paths.project_dir());lib=unreal.MaterialEditingLibrary;dest='/Game/BattleForTheA/Effects/BenchFire';results=[]
for kind,grid in [('Fire',6),('Smoke',8)]:
 name='M_Bench'+kind;assert not unreal.EditorAssetLibrary.does_asset_exist(dest+'/'+name)
 m=unreal.AssetToolsHelpers.get_asset_tools().create_asset(name,dest,unreal.Material,unreal.MaterialFactoryNew())
 m.set_editor_property('blend_mode',unreal.BlendMode.BLEND_ADDITIVE if kind=='Fire' else unreal.BlendMode.BLEND_TRANSLUCENT)
 m.set_editor_property('shading_model',unreal.MaterialShadingModel.MSM_UNLIT);m.set_editor_property('two_sided',True)
 def n(cls,**props):
  e=lib.create_material_expression(m,cls)
  for k,v in props.items():e.set_editor_property(k,v)
  return e
 def wire(a,b,pin,out=''):assert lib.connect_material_expressions(a,out,b,pin)
 def c(v):return n(unreal.MaterialExpressionConstant,r=v)
 def op(cls,a,b):
  e=n(cls);wire(a,e,'A');wire(b,e,'B');return e
 def unary(cls,a):
  e=n(cls);wire(a,e,'');return e
 def mul(a,b):return op(unreal.MaterialExpressionMultiply,a,b)
 def add(a,b):return op(unreal.MaterialExpressionAdd,a,b)
 frame=n(unreal.MaterialExpressionScalarParameter,parameter_name='Frame',default_value=0)
 alpha=unary(unreal.MaterialExpressionFrac,frame);base=unary(unreal.MaterialExpressionFloor,frame)
 uv=n(unreal.MaterialExpressionTextureCoordinate)
 tex=unreal.load_asset(dest+'/T_'+kind+'_SubUV');assert tex
 samples=[]
 for i in [0,1]:
  index=add(base,c(i));col=unary(unreal.MaterialExpressionFrac,mul(index,c(1/grid)))
  row=unary(unreal.MaterialExpressionFrac,mul(unary(unreal.MaterialExpressionFloor,mul(index,c(1/grid))),c(1/grid)))
  offset=n(unreal.MaterialExpressionAppendVector);wire(col,offset,'A');wire(row,offset,'B')
  coords=add(mul(uv,c(1/grid)),offset)
  sample=n(unreal.MaterialExpressionTextureSample,texture=tex,sampler_type=unreal.MaterialSamplerType.SAMPLERTYPE_COLOR);wire(coords,sample,'UVs');samples.append(sample)
 blend=n(unreal.MaterialExpressionLinearInterpolate);wire(samples[0],blend,'A','RGB' if kind=='Fire' else 'A');wire(samples[1],blend,'B','RGB' if kind=='Fire' else 'A');wire(alpha,blend,'Alpha')
 tint=n(unreal.MaterialExpressionConstant3Vector,constant=unreal.LinearColor(2.8,.6,.025) if kind=='Fire' else unreal.LinearColor(.065,.06,.055))
 strength=n(unreal.MaterialExpressionScalarParameter,parameter_name='Strength',default_value=1)
 color=mul(mul(blend,tint),strength) if kind=='Fire' else tint
 assert lib.connect_material_property(color,'',unreal.MaterialProperty.MP_EMISSIVE_COLOR)
 opacity=c(1) if kind=='Fire' else mul(blend,c(1.0))
 fade=n(unreal.MaterialExpressionDepthFade,fade_distance_default=12);wire(opacity,fade,'Opacity')
 assert lib.connect_material_property(fade if kind=='Fire' else mul(fade,strength),'',unreal.MaterialProperty.MP_OPACITY)
 lib.recompile_material(m);assert unreal.EditorAssetLibrary.save_loaded_asset(m)
 results.append({'material':m.get_path_name(),'grid':grid,'interpolated':True})
(root/'Tests/Results/2026-09-12-bench-fire-materials.json').write_text(json.dumps({'materials':results,'visual_verified':False},indent=2)+'\n')
