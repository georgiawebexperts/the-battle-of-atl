"""Original skatepark paint, with existing project concrete shading. No collision edits."""
import unreal,json,pathlib,math
root=pathlib.Path(unreal.Paths.project_dir());base=root/'SourceAssets/Skatepark';dest='/Game/BattleForTheA/Skatepark'
# Read the actual retained mesh vertices so paint matches its triangulated height.
heights={}
for line in (base/'Concrete.obj').read_text().splitlines():
 if line.startswith('v '):
  x,y,z=map(float,line.split()[1:]);heights[round(x),round(-y)]=z

def z(x,y):
 x0=math.floor(x/50)*50;y0=math.floor(y/50)*50;t=(x-x0)/50;u=(y-y0)/50
 a,b,c,d=[heights[p] for p in [(x0,y0),(x0+50,y0),(x0,y0+50),(x0+50,y0+50)]]
 return (a+(b-a)*t+(d-b)*u if t>=u else a+(d-c)*t+(c-a)*u)+1.2
verts=[];faces=[]
def polygon(points):
 start=len(verts)+1;verts.extend([(x,y,z(x,y)) for x,y in points])
 for i in range(1,len(points)-1):faces.append((start,start+i,start+i+1))
# Thin white lines identify the usable bank width, following every mesh row.
for cy in [-750,-250]:
 for x in range(250,1000,50):polygon([(x,cy-4),(x+50,cy-4),(x+50,cy+4),(x,cy+4)])
# Same pair on the west launch bank (axis y = 800), added 2026-09-19 with the bigger park.
for cy in [550,1050]:
 for x in range(-3200,-2200,50):polygon([(x,cy-4),(x+50,cy-4),(x+50,cy+4),(x,cy+4)])
# The quarter pipe's foot, where the roll-in starts, along its whole span.
for x in range(-300,1500,50):polygon([(x,-1854),(x+50,-1854),(x+50,-1846),(x,-1846)])
# The south plaza ledge edge.
for x in range(-200,900,50):polygon([(x,-1454),(x+50,-1454),(x+50,-1446),(x,-1446)])
def arrow(cx,cy,direction=1):
 def convert(points):return [(cx+direction*x,cy+direction*y) for x,y in points]
 polygon(convert([(-75,-12),(20,-12),(20,12),(-75,12)]))
 polygon(convert([(0,-65),(95,0),(0,65)]))
arrow(-120,-500);arrow(120,-500);arrow(2150,170,-1);arrow(-3150,700);arrow(-2150,700,-1);arrow(600,-2050)
lines=['# Original skatepark wayfinding paint; centimetres; 1.2cm surface offset']
lines+=['v %.6f %.6f %.6f'%(x,-y,h) for x,y,h in verts]
lines+=['vt %.6f %.6f'%(x/200,y/200) for x,y,h in verts]
lines+=['f '+' '.join('%d/%d'%(i,i) for i in reversed(f)) for f in faces]
(base/'Markings.obj').write_text('\n'.join(lines)+'\n')
mat=unreal.load_asset(dest+'/M_SkateConcreteDetailed') or unreal.EditorAssetLibrary.duplicate_asset('/Game/PiedmontRide/Materials/M_ParkConcreteWorld',dest+'/M_SkateConcreteDetailed');assert mat
mat.set_editor_property('two_sided',True);unreal.MaterialEditingLibrary.recompile_material(mat);unreal.EditorAssetLibrary.save_loaded_asset(mat)
paint=unreal.load_asset(dest+'/M_SkatePaint')
if not paint:
 paint=unreal.AssetToolsHelpers.get_asset_tools().create_asset('M_SkatePaint',dest,unreal.Material,unreal.MaterialFactoryNew())
 c=unreal.MaterialEditingLibrary.create_material_expression(paint,unreal.MaterialExpressionConstant3Vector);c.set_editor_property('constant',unreal.LinearColor(.68,.66,.57));unreal.MaterialEditingLibrary.connect_material_property(c,'',unreal.MaterialProperty.MP_BASE_COLOR)
 r=unreal.MaterialEditingLibrary.create_material_expression(paint,unreal.MaterialExpressionConstant);r.set_editor_property('r',.86);unreal.MaterialEditingLibrary.connect_material_property(r,'',unreal.MaterialProperty.MP_ROUGHNESS)
 unreal.MaterialEditingLibrary.recompile_material(paint);unreal.EditorAssetLibrary.save_loaded_asset(paint)
options=unreal.FbxImportUI();options.import_as_skeletal=False;options.import_materials=False;options.import_textures=False;options.automated_import_should_detect_type=False;options.mesh_type_to_import=unreal.FBXImportType.FBXIT_STATIC_MESH;options.static_mesh_import_data.combine_meshes=True;options.static_mesh_import_data.auto_generate_collision=False
job=unreal.AssetImportTask();job.filename=str(base/'Markings.obj');job.destination_path=dest;job.destination_name='SM_SkateMarkings';job.automated=True;job.save=True;job.replace_existing=True;job.options=options;unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([job])
mesh=unreal.load_asset(dest+'/SM_SkateMarkings');assert mesh;mesh.set_material(0,paint);unreal.EditorAssetLibrary.save_loaded_asset(mesh)
concrete=unreal.load_asset(dest+'/SM_SkateConcrete');assert concrete;concrete.set_material(0,mat);unreal.EditorAssetLibrary.save_loaded_asset(concrete)
unreal.PiedmontWorldTools.finish_editor_asset_loading()
(root/'work/skate-style-import.json').write_text(json.dumps({'paint_triangles':len(faces),'material':mat.get_path_name(),'geometry':'Noncolliding surface-following paint only; the concrete and berm meshes themselves are baked by prepare_skatepark.py','author':'2026-09-19 [codex-maclaptop]'},indent=2)+'\n')
unreal.SystemLibrary.quit_editor()
