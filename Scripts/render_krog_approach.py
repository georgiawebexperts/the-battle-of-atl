"""Render the isolated wider approach; never save camera/lighting changes."""
import unreal,json
from pathlib import Path
root=Path(unreal.Paths.project_dir()).resolve();out=root/'work/krog-approach';out.mkdir(exist_ok=True)
assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/PiedmontKrogApproachReview')
unreal.PiedmontWorldTools.finish_editor_asset_loading();ea=unreal.get_editor_subsystem(unreal.EditorActorSubsystem);world=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
assert sum(a.actor_has_tag('KrogApproachReview') for a in ea.get_all_level_actors())==1
for a in ea.get_all_level_actors():
 if isinstance(a,unreal.SkyLight):
  s=a.get_component_by_class(unreal.SkyLightComponent);s.set_editor_property('real_time_capture',False);s.recapture_sky()
cam=ea.spawn_actor_from_class(unreal.SceneCapture2D,unreal.Vector());cap=cam.capture_component2d;tex=unreal.RenderingLibrary.create_render_target2d(world,1280,720,unreal.TextureRenderTargetFormat.RTF_RGBA8)
for k,v in [('texture_target',tex),('capture_source',unreal.SceneCaptureSource.SCS_FINAL_COLOR_LDR),('always_persist_rendering_state',True),('capture_every_frame',False),('capture_on_movement',False),('fov_angle',70)]:cap.set_editor_property(k,v)
images=[]
for name,eye,target in [('wide',[28300,115950,6500],[28300,115951,1000]),('rider',[28750,115520,1210],[29400,115560,1040])]:
 p=unreal.Vector(*eye);cam.set_actor_location(p,False,False);cam.set_actor_rotation(unreal.MathLibrary.find_look_at_rotation(p,unreal.Vector(*target)),False)
 for _ in range(24):unreal.PiedmontWorldTools.tick_scene_review();cap.capture_scene()
 unreal.RenderingLibrary.export_render_target(world,tex,str(out),name+'.png');images.append(str(out/(name+'.png')))
(root/'Tests/Results/2026-09-13-krog-approach-render.json').write_text(json.dumps({'images':images,'main_map_changed':False,'visual_accepted':False},indent=2)+'\n')
