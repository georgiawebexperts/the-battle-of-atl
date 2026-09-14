"""Transient pavement/landscape rendering comparison. Does not save assets."""
import unreal,json,pathlib
root=pathlib.Path(unreal.Paths.project_dir());out=root/'work/park-ground-diagnostic';out.mkdir(exist_ok=True)
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
for label in ['baseline','lod0','landscape-hidden']:
 if label=='lod0':unreal.SystemLibrary.execute_console_command(world,'r.ForceLOD 0')
 if label=='landscape-hidden':
  for a in land:
   for c in a.get_components_by_class(unreal.PrimitiveComponent):c.set_visibility(False,True)
 for _ in range(32):unreal.PiedmontWorldTools.tick_scene_review();cap.capture_scene()
 unreal.RenderingLibrary.export_render_target(world,tex,str(out),label+'.png');rows.append(str(out/(label+'.png')))
(root/'Tests/Results/2026-09-14-park-ground-diagnostic.json').write_text(json.dumps({'map_saved':False,'images':rows,'force_lod':unreal.SystemLibrary.get_console_variable_int_value('r.ForceLOD'),'scope':'Transient editor capture; no runtime acceptance'},indent=2)+'\n')
