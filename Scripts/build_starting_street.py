"""Build a terrain-fitted frontage surface; import assets without saving the world."""
import unreal,json,math,hashlib
from pathlib import Path
root=Path(unreal.Paths.project_dir()).resolve();main=root/'Content/PiedmontRide/Maps/PiedmontWorld.umap';before=hashlib.sha256(main.read_bytes()).hexdigest()
assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/PiedmontWorld');unreal.PiedmontWorldTools.finish_editor_asset_loading()
r=json.loads((root/'SourceAssets/Terrain/Tutorial/block.json').read_text());raw=list(reversed(r['alternate'][:21]))+r['road'][1:31]
pts=[]
for a,b in zip(raw,raw[1:]):
 for t in (0,.5):pts.append([a[k]+t*(b[k]-a[k]) for k in range(3)])
pts.append(raw[-1]);distance=[0]
for a,b in zip(pts,pts[1:]):distance.append(distance[-1]+math.dist(a[:2],b[:2]))
rows=[]
for i,p in enumerate(pts):
 a=pts[max(0,i-1)];b=pts[min(len(pts)-1,i+1)];d=math.hypot(b[0]-a[0],b[1]-a[1]);n=[-(b[1]-a[1])/d,(b[0]-a[0])/d];ground={}
 for side in (-325,-225,0,225,325):
  q=unreal.Vector(p[0]+n[0]*side,p[1]+n[1]*side,p[2]);hit=unreal.PiedmontWorldTools.trace_world_surface(q+unreal.Vector(0,0,2000),q-unreal.Vector(0,0,2000));assert hit
  ground[side]=hit[0].z
 wanted=max(p[2]+16,ground[-225]+10,ground[225]+10,ground[-325]-6,ground[325]-6)
 rows.append({'point':p,'normal':n,'ground':ground,'wanted':wanted})
# An upper envelope limits abrupt height variation while clearing the ground.
for i,row in enumerate(rows):
 t=min(1,distance[i]/500,(distance[-1]-distance[i])/500);f=t*t*(3-2*t)
 envelope=max(other['wanted']-.08*abs(distance[i]-distance[j]) for j,other in enumerate(rows))
 row.update(fade=f,z=row['point'][2]+.8+f*(envelope-row['point'][2]))
folder=root/'SourceAssets/Terrain/StartingStreet';folder.mkdir(parents=True,exist_ok=True)
def xyz(row,side,height):
 p=row['point'];n=row['normal'];return [p[0]+side*n[0],p[1]+side*n[1],height]
def ribbon(name,columns):
 vertices=[];faces=[];count=len(columns(rows[0]))
 for row in rows:vertices.extend(columns(row))
 for i in range(len(rows)-1):
  for j in range(count-1):
   a=i*count+j;b=a+1;c=a+count;faces.extend([(a,b,c),(b,c+1,c)])
 lines=['o '+name]+[f'v {x:.5f} {-y:.5f} {z:.5f}' for x,y,z in vertices]+[f'vt {x/200:.5f} {y/200:.5f}' for x,y,z in vertices]
 for face in faces:
  # Columns run across the path: preserve consistent upward winding, with vertical curb faces.
  a,b,c=[vertices[k] for k in face];cross=(b[0]-a[0])*(c[1]-a[1])-(b[1]-a[1])*(c[0]-a[0]);order=face if cross<=0 else tuple(reversed(face))
  lines.append('f '+' '.join(f'{k+1}/{k+1}' for k in order))
 source=folder/(name+'.obj');source.write_text('\n'.join(lines)+'\n')
 opts=unreal.FbxImportUI();opts.import_as_skeletal=False;opts.import_materials=False;opts.import_textures=False;opts.automated_import_should_detect_type=False;opts.mesh_type_to_import=unreal.FBXImportType.FBXIT_STATIC_MESH;opts.static_mesh_import_data.combine_meshes=True;opts.static_mesh_import_data.auto_generate_collision=False
 task=unreal.AssetImportTask();task.filename=str(source);task.destination_path='/Game/BattleForTheA/Environment/StartingStreet';task.destination_name='SM_'+name;task.automated=True;task.save=True;task.replace_existing=True;task.options=opts
 unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task]);mesh=unreal.load_asset(task.destination_path+'/SM_'+name);assert mesh
 mesh.set_material(0,unreal.load_asset('/Game/PiedmontRide/Materials/M_ParkAsphaltWorld' if name=='FrontageRoad' else '/Game/PiedmontRide/Materials/M_ParkConcreteWorld'))
 mesh.get_editor_property('body_setup').set_editor_property('collision_trace_flag',unreal.CollisionTraceFlag.CTF_USE_COMPLEX_AS_SIMPLE);unreal.EditorAssetLibrary.save_loaded_asset(mesh)
 return {'asset':mesh.get_path_name(),'vertices':len(vertices),'triangles':len(faces)}
assets=[ribbon('FrontageRoad',lambda row:[xyz(row,s,row['z']-abs(s)*.02*row['fade']) for s in (-225,0,225)])]
for sign,name in [(-1,'FrontageSouthWalk'),(1,'FrontageNorthWalk')]:
 def columns(row,sign=sign):
  f=row['fade'];edge=row['z']-4.5*f;top=edge+18*f
  pairs=[(225,edge),(225,top),(245,top),(325,top-1.6*f),(325,row['ground'][sign*325]-10)]
  if sign<0:pairs=list(reversed(pairs))
  return [xyz(row,sign*s,z) for s,z in pairs]
 assets.append(ribbon(name,columns))
assert hashlib.sha256(main.read_bytes()).hexdigest()==before
report={'assets':assets,'rows':rows,'length_cm':distance[-1],'main_sha256_preserved':before,'scope':'50m frontage asset generation from native terrain; world unchanged. Runtime import/ground clearance, start height, both tutorial routes and visual review pending.'}
(folder/'profile.json').write_text(json.dumps(report,indent=2)+'\n');(root/'Tests/Results/2026-09-13-starting-street-build.json').write_text(json.dumps({k:v for k,v in report.items() if k!='rows'},indent=2)+'\n')
