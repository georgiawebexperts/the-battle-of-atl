"""Compare the same path views in main and candidate worlds; no map saves."""
import unreal,json,pathlib,hashlib
root=pathlib.Path(unreal.Paths.project_dir());out=root/'work/lake-grading-comparison';out.mkdir(exist_ok=True);rows=[]
for label,name in [('before','PiedmontWorld'),('candidate','PiedmontLakePathGradingReview')]:
 assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/'+name)
 world=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world();ea=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
 for actor in ea.get_all_level_actors():
  if isinstance(actor,unreal.DirectionalLight):actor.get_component_by_class(unreal.DirectionalLightComponent).set_intensity(40)
  if isinstance(actor,unreal.WaterBodyLake):assert unreal.PiedmontWorldTools.refresh_water_body(actor)
 unreal.PiedmontWorldTools.finish_editor_asset_loading();unreal.PiedmontWorldTools.rebuild_water_zones()
 for _ in range(60):unreal.PiedmontWorldTools.tick_scene_review()
 cam=ea.spawn_actor_from_class(unreal.SceneCapture2D,unreal.Vector());c=cam.get_component_by_class(unreal.SceneCaptureComponent2D);t=unreal.RenderingLibrary.create_render_target2d(world,1280,720,unreal.TextureRenderTargetFormat.RTF_RGBA8)
 c.set_editor_property('texture_target',t);c.set_editor_property('capture_source',unreal.SceneCaptureSource.SCS_FINAL_COLOR_LDR);c.set_editor_property('always_persist_rendering_state',True);c.set_editor_property('capture_every_frame',False);c.set_editor_property('capture_on_movement',False);c.set_editor_property('fov_angle',75)
 for key,eye,target in [('path61853018',[-4450,-2480,-100],[-5100,-1915,-320]),('path182460439',[-8600,-400,50],[-9020,460,-210])]:
  location=unreal.Vector(*eye);cam.set_actor_location(location,False,False);cam.set_actor_rotation(unreal.MathLibrary.find_look_at_rotation(location,unreal.Vector(*target)),False)
  for _ in range(20):unreal.PiedmontWorldTools.tick_scene_review();c.capture_scene()
  filename=key+'-'+label+'.png';unreal.RenderingLibrary.export_render_target(world,t,str(out),filename);rows.append({'file':filename,'eye':eye,'target':target,'map':name,'sha256':hashlib.sha256((out/filename).read_bytes()).hexdigest()})
(out/'manifest.json').write_text(json.dumps({'map_saved':False,'capture':'Static editor scene, diagnostic light intensity40; refreshed water; full textures; no gameplay acceptance','images':rows},indent=2)+'\n')
