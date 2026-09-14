"""Transient pavement/landscape rendering comparison. Does not save assets."""
import unreal,json,pathlib
root=pathlib.Path(unreal.Paths.project_dir());out=root/'work/krog-route-stall';out.mkdir(exist_ok=True)
assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/PiedmontWorld')
unreal.PiedmontWorldTools.finish_editor_asset_loading()
ea=unreal.get_editor_subsystem(unreal.EditorActorSubsystem);world=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
actors=ea.get_all_level_actors();land=[a for a in actors if isinstance(a,unreal.Landscape)]
for a in actors:
 if a.get_editor_property('hidden'):
  for c in a.get_components_by_class(unreal.PrimitiveComponent):c.set_visibility(False,True)
 if isinstance(a,unreal.SkyLight):
  c=a.get_component_by_class(unreal.SkyLightComponent);c.set_editor_property('real_time_capture',False);c.recapture_sky()
cam=ea.spawn_actor_from_class(unreal.SceneCapture2D,unreal.Vector(25700,100400,1200));cam.set_actor_rotation(unreal.MathLibrary.find_look_at_rotation(cam.get_actor_location(),unreal.Vector(26050,100850,890)),False)
cap=cam.get_component_by_class(unreal.SceneCaptureComponent2D);tex=unreal.RenderingLibrary.create_render_target2d(world,1280,720,unreal.TextureRenderTargetFormat.RTF_RGBA8)
for k,v in [('texture_target',tex),('capture_source',unreal.SceneCaptureSource.SCS_FINAL_COLOR_LDR),('always_persist_rendering_state',True),('capture_every_frame',False),('capture_on_movement',False),('fov_angle',75)]:cap.set_editor_property(k,v)

rows=[]
for dx,dy in [(0,600),(0,-600),(600,0),(-600,0),(400,400),(-400,400)]:
 start=unreal.Vector(26050,100850,894);end=start+unreal.Vector(dx,dy,0)
 h=unreal.PiedmontWorldTools.trace_world_surface(start,end,32)
 rows.append({'delta':[dx,dy],'hit':h[1].get_actor_label() if h else None,'xyz':[h[0].x,h[0].y,h[0].z] if h else None})
for y in range(100650,101101,25):
 h=unreal.PiedmontWorldTools.trace_world_surface(unreal.Vector(26050,y,1200),unreal.Vector(26050,y,400))
 rows.append({'floor_y':y,'hit':h[1].get_actor_label() if h else None,'z':h[0].z if h else None})
for _ in range(32):unreal.PiedmontWorldTools.tick_scene_review();cap.capture_scene()
unreal.RenderingLibrary.export_render_target(world,tex,str(out),'stalled-location.png')
(root/'Tests/Results/2026-09-14-krog-route-stall-scene.json').write_text(json.dumps({'map_saved':False,'probes':rows,'image':str(out/'stalled-location.png')},indent=2)+'\n')
