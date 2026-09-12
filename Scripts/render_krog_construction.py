"""Inspect local road closure signs; transient fixed editor cameras, no map save."""
import json
from pathlib import Path
import unreal
root=Path(unreal.Paths.project_dir()).resolve();out=root/'work/krog-construction-render';out.mkdir(exist_ok=True)
assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/PiedmontKrogBoundaryReview')
unreal.PiedmontWorldTools.finish_editor_asset_loading();ea=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
world=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
for actor in ea.get_all_level_actors():
 if isinstance(actor,unreal.SkyLight):
  component=actor.get_component_by_class(unreal.SkyLightComponent);component.set_editor_property('real_time_capture',False);component.recapture_sky()
cam=ea.spawn_actor_from_class(unreal.SceneCapture2D,unreal.Vector());capture=cam.get_component_by_class(unreal.SceneCaptureComponent2D)
texture=unreal.RenderingLibrary.create_render_target2d(world,1280,720,unreal.TextureRenderTargetFormat.RTF_RGBA8)
capture.set_editor_property('texture_target',texture);capture.set_editor_property('capture_source',unreal.SceneCaptureSource.SCS_FINAL_COLOR_LDR)
capture.set_editor_property('always_persist_rendering_state',True);capture.set_editor_property('capture_every_frame',False);capture.set_editor_property('capture_on_movement',False);capture.set_editor_property('fov_angle',85)
rows=json.loads((root/'Tests/Results/2026-09-12-krog-construction-placement.json').read_text())['closures'];images=[]
for i,row in enumerate(rows):
 c=unreal.Vector(*row['center']);d=unreal.Vector(*row['outward']);p=c-d*1200+unreal.Vector(0,0,240)
 cam.set_actor_location(p,False,False);cam.set_actor_rotation(unreal.MathLibrary.find_look_at_rotation(p,c+unreal.Vector(0,0,180)),False)
 for _ in range(24):unreal.PiedmontWorldTools.tick_scene_review();capture.capture_scene()
 name=f'closure-{i}.png';unreal.RenderingLibrary.export_render_target(world,texture,str(out),name);images.append(str(out/name))
(root/'Tests/Results/2026-09-12-krog-construction-render.json').write_text(json.dumps({'images':images,'visual_review':'pending','main_map_changed':False,'scope':'Fixed editor views; no live traffic or gameplay traversal.'},indent=2)+'\n')
