"""Inspect saved park lighting and compare exposure without saving assets."""
import unreal,json,pathlib
root=pathlib.Path(unreal.Paths.project_dir());out=root/'work/park-exposure-review';out.mkdir(exist_ok=True)
assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/PiedmontWorld')
unreal.PiedmontWorldTools.finish_editor_asset_loading()
ea=unreal.get_editor_subsystem(unreal.EditorActorSubsystem);world=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
actors=ea.get_all_level_actors();land=[a for a in actors if isinstance(a,unreal.Landscape)]
for a in actors:
 if a.get_editor_property('hidden'):
  for c in a.get_components_by_class(unreal.PrimitiveComponent):c.set_visibility(False,True)
 if isinstance(a,unreal.SkyLight):
  c=a.get_component_by_class(unreal.SkyLightComponent);c.set_editor_property('real_time_capture',False);c.recapture_sky()
cam=ea.spawn_actor_from_class(unreal.SceneCapture2D,unreal.Vector(-16495,2606,100));cam.set_actor_rotation(unreal.MathLibrary.find_look_at_rotation(cam.get_actor_location(),unreal.Vector(-16735,2936,-150)),False)
cap=cam.get_component_by_class(unreal.SceneCaptureComponent2D);tex=unreal.RenderingLibrary.create_render_target2d(world,1280,720,unreal.TextureRenderTargetFormat.RTF_RGBA8)
for k,v in [('texture_target',tex),('capture_source',unreal.SceneCaptureSource.SCS_FINAL_COLOR_LDR),('always_persist_rendering_state',True),('capture_every_frame',False),('capture_on_movement',False),('fov_angle',75)]:cap.set_editor_property(k,v)

rows=[]
for a in actors:
 if isinstance(a,(unreal.DirectionalLight,unreal.SkyLight)):
  c=a.get_component_by_class(unreal.LightComponent)
  rows.append({'actor':a.get_actor_label(),'class':a.get_class().get_name(),'intensity':c.get_editor_property('intensity') if c else None})
 if isinstance(a,unreal.PostProcessVolume):
  p=a.get_editor_property('settings');rows.append({'actor':a.get_actor_label(),'exposure_bias':p.get_editor_property('auto_exposure_bias'),'override_bias':p.get_editor_property('override_auto_exposure_bias')})
baseline_bias=cap.get_editor_property('post_process_settings').get_editor_property('auto_exposure_bias')
rows.append({'capture_default_exposure_bias':baseline_bias})
for name,bias in [('baseline',None),('relative_minus035',baseline_bias-.35)]:
 if bias is not None:
  settings=cap.get_editor_property('post_process_settings');settings.set_editor_property('override_auto_exposure_bias',True);settings.set_editor_property('auto_exposure_bias',bias);cap.set_editor_property('post_process_settings',settings);cap.set_editor_property('post_process_blend_weight',1)
 for _ in range(48):unreal.PiedmontWorldTools.tick_scene_review();cap.capture_scene()
 unreal.RenderingLibrary.export_render_target(world,tex,str(out),name+'.png')
(out/'manifest.json').write_text(json.dumps({'lights':rows,'map_saved':False,'scope':'Scene capture comparison only; native gameplay exposure may differ'},indent=2)+'\n')
