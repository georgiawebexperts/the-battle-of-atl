"""Import and inspect the new street in isolation. Never save the main map."""
import unreal,json,pathlib,hashlib,math
root=pathlib.Path(unreal.Paths.project_dir());main=root/'Content/PiedmontRide/Maps/PiedmontWorld.umap';before=hashlib.sha256(main.read_bytes()).hexdigest();folder=root/'SourceAssets/Terrain/PrideIntersection';out=root/'work/pride-street-review';out.mkdir(exist_ok=True)
assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/PiedmontWorld')
world=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world();ea=unreal.get_editor_subsystem(unreal.EditorActorSubsystem);rows=[]
for row in json.loads((folder/'cutbacks.json').read_text())['meshes']+json.loads((folder/'surfaces.json').read_text())['meshes']:
 name='SM_'+row['name'];opts=unreal.FbxImportUI();opts.import_as_skeletal=False;opts.import_materials=False;opts.import_textures=False;opts.automated_import_should_detect_type=False;opts.mesh_type_to_import=unreal.FBXImportType.FBXIT_STATIC_MESH;opts.static_mesh_import_data.combine_meshes=True;opts.static_mesh_import_data.auto_generate_collision=False;opts.static_mesh_import_data.remove_degenerates=False
 if 'source_name' not in row:opts.static_mesh_import_data.normal_import_method=unreal.FBXNormalImportMethod.FBXNIM_IMPORT_NORMALS
 task=unreal.AssetImportTask();task.filename=str(folder/(row['name']+'.obj'));task.destination_path='/Game/BattleForTheA/Environment/PrideIntersection';task.destination_name=name;task.automated=True;task.save=True;task.replace_existing=True;task.options=opts;unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task]);mesh=unreal.load_asset(task.destination_path+'/'+name);assert mesh
 b=mesh.get_bounding_box();bounds=[[b.min.x,b.min.y,b.min.z],[b.max.x,b.max.y,b.max.z]];error=max(abs(bounds[i][j]-row['bounds_cm'][i][j]) for i in range(2) for j in range(3));assert error<.1,(name,error)
 replacement=None
 if 'source_name' in row:
  targets=[a for a in ea.get_all_level_actors() if isinstance(a,unreal.StaticMeshActor) and a.static_mesh_component.static_mesh and a.static_mesh_component.static_mesh.get_name()==row['source_name']];assert len(targets)==1,(row['source_name'],len(targets));replacement=targets[0];material=replacement.static_mesh_component.get_material(0)
 else:material=unreal.load_asset('/Game/PiedmontRide/Materials/'+('M_ParkConcreteWorld' if 'Sidewalk' in name else 'M_ParkAsphaltWorld'))
 mesh.set_material(0,material);mesh.get_editor_property('body_setup').set_editor_property('collision_trace_flag',unreal.CollisionTraceFlag.CTF_USE_COMPLEX_AS_SIMPLE);unreal.EditorAssetLibrary.save_loaded_asset(mesh)
 actor=ea.spawn_actor_from_class(unreal.StaticMeshActor,unreal.Vector());actor.set_actor_label(name);actor.tags=[unreal.Name('BattlePrideStreet'),unreal.Name('RidePath')];actor.set_folder_path('Midtown/PrideIntersection');actor.static_mesh_component.set_static_mesh(mesh);actor.static_mesh_component.set_collision_profile_name('NoCollision' if 'Paint' in name else 'BlockAll');
 if replacement:ea.destroy_actor(replacement)
 rows.append({'name':name,'bounds_error_cm':error,'triangles':mesh.get_num_triangles(0)})
unreal.PiedmontWorldTools.finish_editor_asset_loading()
# Source vertex probes establish native collision placement without assuming generic ground hits are road.
checks=[]
for row in json.loads((folder/'surfaces.json').read_text())['meshes']:
 vertices=[list(map(float,l.split()[1:4])) for l in (folder/(row['name']+'.obj')).read_text().splitlines() if l.startswith('v ')]
 for k in range(0,len(vertices)-2,183):
  f=vertices[k:k+3];x=sum(p[0] for p in f)/3;y=-sum(p[1] for p in f)/3;z=sum(p[2] for p in f)/3;hit=unreal.PiedmontWorldTools.trace_world_surface(unreal.Vector(x,y,z+2),unreal.Vector(x,y,z-2));error=None if not hit else abs(hit[0].z-z);checks.append({'mesh':row['name'],'xyz':[x,y,z],'error':error,'actor':None if not hit else hit[1].get_actor_label()})
assert checks and all(c['error'] is not None and c['error']<.1 and c['actor']=='SM_'+c['mesh'] for c in checks),str([c for c in checks if c['error'] is None or c['error']>=.1][:3])
exec(compile((root/'Scripts/add_pride_crosswalks_review.py').read_text(),'add_pride_crosswalks_review.py','exec'), {'__name__':'__main__'})
exec(compile((root/'Scripts/add_pride_street_signs_review.py').read_text(),'add_pride_street_signs_review.py','exec'), {'__name__':'__main__'})
# Save only an isolated review map; tutorial boundary remains intentionally unchanged until integrated.
assert unreal.EditorLoadingAndSavingUtils.save_map(world,'/Game/PiedmontRide/Maps/PiedmontPrideStreetReview')
cam=ea.spawn_actor_from_class(unreal.SceneCapture2D,unreal.Vector());cap=cam.get_component_by_class(unreal.SceneCaptureComponent2D);tex=unreal.RenderingLibrary.create_render_target2d(world,1280,720,unreal.TextureRenderTargetFormat.RTF_RGBA8);cap.texture_target=tex;cap.capture_source=unreal.SceneCaptureSource.SCS_FINAL_COLOR_LDR;cap.capture_every_frame=False;cap.capture_on_movement=False;cap.always_persist_rendering_state=True;cap.fov_angle=75
for a in ea.get_all_level_actors():
 if isinstance(a,unreal.DirectionalLight):a.get_component_by_class(unreal.DirectionalLightComponent).set_intensity(40)
for name,pos,target in [('pride-corner',[-26300,15100,2900],[-23800,12100,150]),('billys-approach',[-22400,8000,2600],[-19600,4200,0]),('road-shading',[-23800,10800,650],[-23300,8500,50])]:
 cam.set_actor_location(unreal.Vector(*pos),False,False);cam.set_actor_rotation(unreal.MathLibrary.find_look_at_rotation(unreal.Vector(*pos),unreal.Vector(*target)),False)
 for _ in range(20):unreal.PiedmontWorldTools.tick_scene_review();cap.capture_scene()
 unreal.RenderingLibrary.export_render_target(world,tex,str(out),name+'.png')
assert hashlib.sha256(main.read_bytes()).hexdigest()==before
report={'passed':True,'main_unchanged_sha256':before,'review_map':'/Game/PiedmontRide/Maps/PiedmontPrideStreetReview','meshes':rows,'collision_probes':len(checks),'maximum_error_cm':max(c['error'] for c in checks),'scope':'Import bounds and sampled native mesh collision only. Static views use diagnostic lighting. Boundary, road seams and moving rider need verification; not installed.'};(out/'result.json').write_text(json.dumps(report,indent=2)+'\n')
