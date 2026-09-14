"""Authored rainbow crossings on mapped road approaches; current review world only."""
import unreal,pathlib,json,math
root=pathlib.Path(unreal.Paths.project_dir());folder=root/'SourceAssets/Terrain/PrideIntersection';out=folder/'Crosswalks';out.mkdir(exist_ok=True);data=json.loads((folder/'survey.json').read_text());world=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world();ea=unreal.get_editor_subsystem(unreal.EditorActorSubsystem);dest='/Game/BattleForTheA/Environment/PrideIntersection/Crosswalks'
# Use the mapped approach tangents; road widths are the game's existing human-scale widths.
def at_axis(street,axis,value):
 for row in data['roads']:
  if row['tags']['name']!=street:continue
  for a,b in zip(row['points_world_cm'],row['points_world_cm'][1:]):
   if min(a[axis],b[axis])<=value<=max(a[axis],b[axis]) and abs(b[axis]-a[axis])>1e-6:
    t=(value-a[axis])/(b[axis]-a[axis]);p=[a[k]+t*(b[k]-a[k]) for k in range(2)];v=[b[k]-a[k] for k in range(2)];length=math.hypot(*v);v=[x/length for x in v]
    if v[axis]<0:v=[-x for x in v]
    return p,v
 raise AssertionError((street,axis,value))
site=[]
for label,x in [('West',-25000),('East',-23550)]:
 p,t=at_axis('10th Street Northeast',0,x);n=[-t[1],t[0]];site.append((label,[p[k]+450*n[k] for k in range(2)],t,860))
for label,y in [('North',12000),('South',13850)]:
 p,t=at_axis('Piedmont Avenue Northeast',1,y);site.append((label,p,t,560))
colors=[('Red',(.7,.025,.045)),('Orange',(.95,.22,.015)),('Yellow',(.95,.7,.025)),('Green',(.035,.45,.13)),('Blue',(.025,.18,.75)),('Violet',(.32,.035,.65))];all_checks=[];rows=[]
for index,(name,color) in enumerate(colors):
 faces=[]
 for label,p,t,width in site:
  n=[-t[1],t[0]];lo=-120+index*40;hi=lo+40
  for j in range(math.ceil(width/40)):
   left=-width/2+j*width/math.ceil(width/40);right=-width/2+(j+1)*width/math.ceil(width/40);corners=[]
   for u,v in [(lo,left),(hi,left),(hi,right),(lo,right)]:
    x,y=[p[k]+u*t[k]+v*n[k] for k in range(2)];hit=unreal.PiedmontWorldTools.trace_world_surface(unreal.Vector(x,y,1600),unreal.Vector(x,y,-1200));assert hit,(label,x,y);all_checks.append({'approach':label,'actor':hit[1].get_actor_label(),'xyz':[x,y,hit[0].z]});assert hit[1].actor_has_tag('RidePath'),all_checks[-1];corners.append([x,y,hit[0].z+1.2])
   faces.extend([[corners[i] for i in order] for order in [(0,1,2),(0,2,3)]])
 verts=[v for face in faces for v in face];lines=['o Rainbow'+name]+[f'v {x:.6f} {-y:.6f} {z:.6f}' for x,y,z in verts]+[f'vt {x/100:.6f} {y/100:.6f}' for x,y,z in verts]
 for k in range(0,len(verts),3):
  a,b,c=verts[k:k+3];cross=(b[0]-a[0])*(c[1]-a[1])-(b[1]-a[1])*(c[0]-a[0]);order=(k,k+1,k+2) if cross<0 else (k+2,k+1,k);lines.append('f '+' '.join(f'{i+1}/{i+1}' for i in order))
 file=out/('Rainbow'+name+'.obj');file.write_text('\n'.join(lines)+'\n');opts=unreal.FbxImportUI();opts.import_as_skeletal=False;opts.import_materials=False;opts.import_textures=False;opts.automated_import_should_detect_type=False;opts.mesh_type_to_import=unreal.FBXImportType.FBXIT_STATIC_MESH;opts.static_mesh_import_data.combine_meshes=True;opts.static_mesh_import_data.auto_generate_collision=False
 task=unreal.AssetImportTask();task.filename=str(file);task.destination_path=dest;task.destination_name='SM_Rainbow'+name;task.automated=True;task.save=True;task.replace_existing=True;task.options=opts;unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task]);mesh=unreal.load_asset(dest+'/'+task.destination_name);assert mesh
 mat=unreal.load_asset(dest+'/M_'+name)
 if not mat:mat=unreal.AssetToolsHelpers.get_asset_tools().create_asset('M_'+name,dest,unreal.Material,unreal.MaterialFactoryNew())
 lib=unreal.MaterialEditingLibrary;lib.delete_all_material_expressions(mat);node=lib.create_material_expression(mat,unreal.MaterialExpressionConstant3Vector);node.set_editor_property('constant',unreal.LinearColor(*color));lib.connect_material_property(node,'',unreal.MaterialProperty.MP_BASE_COLOR);rough=lib.create_material_expression(mat,unreal.MaterialExpressionConstant);rough.set_editor_property('r',.95);lib.connect_material_property(rough,'',unreal.MaterialProperty.MP_ROUGHNESS);lib.recompile_material(mat);unreal.EditorAssetLibrary.save_loaded_asset(mat);mesh.set_material(0,mat);unreal.EditorAssetLibrary.save_loaded_asset(mesh)
 a=ea.spawn_actor_from_class(unreal.StaticMeshActor,unreal.Vector());a.set_actor_label('Pride rainbow '+name);a.tags=[unreal.Name('PrideCrosswalk')];a.static_mesh_component.set_static_mesh(mesh);a.static_mesh_component.set_collision_profile_name('NoCollision');a.set_folder_path('Midtown/PrideIntersection');rows.append({'name':name,'triangles':len(faces),'collision':'NoCollision'})
(out/'manifest.json').write_text(json.dumps({'status':'authored game markings, not surveyed paint dimensions','approaches':[{'name':a,'center':b,'tangent':c,'width':d} for a,b,c,d in site],'meshes':rows,'support_probes':len(all_checks),'scope':'Four approach crosswalks, six colors; native visual review pending'},indent=2)+'\n')
