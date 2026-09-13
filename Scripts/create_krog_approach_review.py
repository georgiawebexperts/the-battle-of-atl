"""Build a ground-fitted 2.4m concrete approach in an isolated copy of current main."""
import unreal,json,math,hashlib
from pathlib import Path
root=Path(unreal.Paths.project_dir()).resolve();ea=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
main_file=root/'Content/PiedmontRide/Maps/PiedmontWorld.umap'
main_hash=hashlib.sha256(main_file.read_bytes()).hexdigest()
assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/PiedmontWorld')
unreal.PiedmontWorldTools.finish_editor_asset_loading()
assert unreal.EditorLoadingAndSavingUtils.save_map(unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world(),'/Game/PiedmontRide/Maps/PiedmontKrogApproachReview')
assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/PiedmontKrogApproachReview')
assert unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world().get_name()=='PiedmontKrogApproachReview'
row=next(r for r in json.loads((root/'Tests/Results/2026-09-13-krog-approach-options.json').read_text())['options'] if r['offset_cm']==220)
assert row['missing']==0
points=[unreal.Vector(*p) for p in row['points']];dist=[0.]
for a,b in zip(points,points[1:]):dist.append(dist[-1]+math.hypot(b.x-a.x,b.y-a.y))
indices=[i for i,d in enumerate(dist) if dist[-1]-d<2700]
vertices=[];levels=[]
for i in indices:
 p=points[i];a=points[max(0,i-1)];b=points[min(len(points)-1,i+1)];n=math.hypot(b.x-a.x,b.y-a.y);normal=unreal.Vector(-(b.y-a.y)/n,(b.x-a.x)/n,0)
 if i==indices[0]:p=p-unreal.Vector((b.x-a.x)/n,(b.y-a.y)/n,0)*10
 if i==indices[-1]:p=p+unreal.Vector((b.x-a.x)/n,(b.y-a.y)/n,0)*10
 edges=[]
 for side in (-120,0,120):
  q=p+normal*side;hit=unreal.PiedmontWorldTools.trace_world_surface(q+unreal.Vector(0,0,400),q-unreal.Vector(0,0,400));assert hit
  edges.append(hit[0])
 # Avoid burying either edge in existing ground. A thin asphalt-level finish,
 # not a raised curb: each station follows the highest ground across its width.
 z=max(p.z for p in edges)+5.0
 levels.append(z);vertices += [(edges[0].x,edges[0].y,z),(edges[2].x,edges[2].y,z)]
 points[i].z=z
faces=[]
for i in range(len(indices)-1):
 a=2*i;faces += [(a,a+1,a+2),(a+1,a+3,a+2)]
folder=root/'SourceAssets/Terrain/KrogApproach';folder.mkdir(exist_ok=True)
lines=['o KrogApproach']+[f'v {x:.5f} {-y:.5f} {z:.5f}' for x,y,z in vertices]+[f'vt {x/200:.5f} {y/200:.5f}' for x,y,z in vertices]
for face in faces:
 # Ensure upward Unreal normals after OBJ handedness conversion.
 a,b,c=[vertices[i] for i in face];cross=(b[0]-a[0])*(c[1]-a[1])-(b[1]-a[1])*(c[0]-a[0]);order=face if cross<0 else tuple(reversed(face));lines.append('f '+' '.join(f'{i+1}/{i+1}' for i in order))
source=folder/'KrogApproach.obj';source.write_text('\n'.join(lines)+'\n')
opts=unreal.FbxImportUI();opts.import_as_skeletal=False;opts.import_materials=False;opts.import_textures=False;opts.automated_import_should_detect_type=False;opts.mesh_type_to_import=unreal.FBXImportType.FBXIT_STATIC_MESH;opts.static_mesh_import_data.combine_meshes=True;opts.static_mesh_import_data.auto_generate_collision=False
asset='/Game/BattleForTheA/Environment/KrogApproach';task=unreal.AssetImportTask();task.filename=str(source);task.destination_path=asset;task.destination_name='SM_KrogApproach';task.automated=True;task.save=True;task.replace_existing=True;task.options=opts
unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task]);mesh=unreal.load_asset(asset+'/SM_KrogApproach');assert mesh
exec(compile((root/'Scripts/style_krog_approach.py').read_text(),'style_krog_approach','exec'),{'__name__':'__main__'})
mesh.get_editor_property('body_setup').set_editor_property('collision_trace_flag',unreal.CollisionTraceFlag.CTF_USE_COMPLEX_AS_SIMPLE);unreal.EditorAssetLibrary.save_loaded_asset(mesh)
a=ea.spawn_actor_from_class(unreal.StaticMeshActor,unreal.Vector());a.set_actor_label('Krog approach concrete review');a.tags=[unreal.Name('RidePath'),unreal.Name('KrogApproachReview')];a.static_mesh_component.set_static_mesh(mesh);a.static_mesh_component.set_collision_profile_name('BlockAll')
spline=next(a for a in ea.get_all_level_actors() if isinstance(a,unreal.PiedmontPathSpline) and a.actor_has_tag('BattleKrog_2'));spline.set_centerline(points)
unreal.PiedmontWorldTools.finish_editor_asset_loading()
failures=[]
for i in indices:
 p=points[i];hit=unreal.PiedmontWorldTools.trace_world_surface(p+unreal.Vector(0,0,100),p-unreal.Vector(0,0,100))
 if not hit or not hit[1].actor_has_tag('RidePath') or abs(hit[0].z-p.z)>2:failures.append({'index':i,'expected_z':p.z,'hit_z':hit[0].z if hit else None,'actor':hit[1].get_actor_label() if hit else None})
assert not failures,failures
assert unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).save_current_level()
assert hashlib.sha256(main_file.read_bytes()).hexdigest()==main_hash,'Main map changed during isolated review'
(root/'Tests/Results/2026-09-13-krog-approach-review.json').write_text(json.dumps({'main_map_changed':False,'active_world':'PiedmontKrogApproachReview','main_sha256_preserved':main_hash,'offset_cm':220,'width_cm':240,'surface_samples':len(indices),'surface_failures':failures,'triangles':len(faces),'scope':'Authored approach fitted to native ground in separate map; no visual, traffic or navigation acceptance yet.'},indent=2)+'\n')
