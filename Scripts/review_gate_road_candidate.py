"""Import a continuous gate pavement candidate, verify collision and render without saving the main map."""
import hashlib,json,pathlib,unreal
root=pathlib.Path(unreal.Paths.project_dir()).resolve()
main=root/'Content/PiedmontRide/Maps/PiedmontWorld.umap'
before=hashlib.sha256(main.read_bytes()).hexdigest()
assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/PiedmontWorld')
ea=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
world=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
folder=root/'SourceAssets/Terrain/GateRoad'
data=json.loads((folder/'surface.json').read_text())
opts=unreal.FbxImportUI();opts.import_as_skeletal=False;opts.import_materials=False;opts.import_textures=False;opts.automated_import_should_detect_type=False;opts.mesh_type_to_import=unreal.FBXImportType.FBXIT_STATIC_MESH
opts.static_mesh_import_data.combine_meshes=True;opts.static_mesh_import_data.auto_generate_collision=False;opts.static_mesh_import_data.remove_degenerates=False;opts.static_mesh_import_data.normal_import_method=unreal.FBXNormalImportMethod.FBXNIM_IMPORT_NORMALS
task=unreal.AssetImportTask();task.filename=str(folder/'GateRoad.obj');task.destination_path='/Game/BattleForTheA/Environment/GateRoad';task.destination_name='SM_GateRoad';task.automated=True;task.save=True;task.replace_existing=True;task.options=opts
unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task]);mesh=unreal.load_asset(task.destination_path+'/SM_GateRoad');assert mesh
mesh.set_material(0,unreal.load_asset('/Game/PiedmontRide/Materials/M_Asphalt'))
mesh.get_editor_property('body_setup').set_editor_property('collision_trace_flag',unreal.CollisionTraceFlag.CTF_USE_COMPLEX_AS_SIMPLE)
unreal.EditorAssetLibrary.save_loaded_asset(mesh)
pavement=ea.spawn_actor_from_class(unreal.StaticMeshActor,unreal.Vector());pavement.set_actor_label('SM_GateRoad');pavement.tags=[unreal.Name('RidePath'),unreal.Name('BattleGateRoad')];pavement.static_mesh_component.set_static_mesh(mesh);pavement.static_mesh_component.set_collision_profile_name('BlockAll')
unreal.PiedmontWorldTools.finish_editor_asset_loading()
vertices=[list(map(float,l.split()[1:4])) for l in (folder/'GateRoad.obj').read_text().splitlines() if l.startswith('v ')]
checks=[]
for k in range(0,len(vertices),3):
 f=vertices[k:k+3];x=sum(p[0] for p in f)/3;y=-sum(p[1] for p in f)/3;z=sum(p[2] for p in f)/3
 hit=unreal.PiedmontWorldTools.trace_world_surface(unreal.Vector(x,y,z+1),unreal.Vector(x,y,z-1))
 checks.append(bool(hit and hit[1]==pavement and abs(hit[0].z-z)<.1))
assert all(checks),(len(checks),sum(checks))
assert unreal.EditorLoadingAndSavingUtils.save_map(world,'/Game/PiedmontRide/Maps/PiedmontGateRoadReview')
actor=ea.spawn_actor_from_class(unreal.BattleTutorial,unreal.Vector())
road=next(c for c in actor.get_components_by_class(unreal.InstancedStaticMeshComponent) if c.get_name()=='PracticeStreet')
for a in ea.get_all_level_actors():
 if isinstance(a,unreal.SkyLight):
  s=a.get_component_by_class(unreal.SkyLightComponent);s.set_editor_property('real_time_capture',False);s.recapture_sky()
cam=ea.spawn_actor_from_class(unreal.SceneCapture2D,unreal.Vector(-16600,-5080,500))
cam.set_actor_rotation(unreal.MathLibrary.find_look_at_rotation(cam.get_actor_location(),unreal.Vector(-18400,-4850,150)),False)
cap=cam.get_component_by_class(unreal.SceneCaptureComponent2D)
tex=unreal.RenderingLibrary.create_render_target2d(world,1280,720,unreal.TextureRenderTargetFormat.RTF_RGBA8)
for prop,val in [('texture_target',tex),('capture_source',unreal.SceneCaptureSource.SCS_FINAL_COLOR_LDR),('always_persist_rendering_state',True),('capture_every_frame',False),('capture_on_movement',False),('fov_angle',75)]:cap.set_editor_property(prop,val)
unreal.PiedmontWorldTools.finish_editor_asset_loading()
out=root/'work/gate-road-candidate';out.mkdir(parents=True,exist_ok=True)
assert road.get_instance_count()==414
for index in sorted(data['indices'],reverse=True):assert road.remove_instance(index)
for name in ['candidate']:
 for _ in range(36):unreal.PiedmontWorldTools.tick_scene_review();cap.capture_scene()
 unreal.RenderingLibrary.export_render_target(world,tex,str(out),name+'.png')
assert hashlib.sha256(main.read_bytes()).hexdigest()==before
(root/'Tests/Results/2026-09-13-gate-road-candidate.json').write_text(json.dumps({'map_saved':False,'map_sha256':before,'legacy_instances':road.get_instance_count(),'images':[str(out/'candidate.png')],'collision_probes':len(checks),'removed_indices':data['indices'],'scope':'Isolated candidate with 971 triangle centroid collision probes and a static comparison; runtime riding pending.'},indent=2)+'\n')
