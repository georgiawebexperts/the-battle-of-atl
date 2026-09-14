import unreal,pathlib
root=pathlib.Path(unreal.Paths.project_dir());assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/PiedmontPrideStreetReview');world=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world();ea=unreal.get_editor_subsystem(unreal.EditorActorSubsystem);unreal.PiedmontWorldTools.finish_editor_asset_loading()
for a in ea.get_all_level_actors():
 if isinstance(a,unreal.DirectionalLight):a.get_component_by_class(unreal.DirectionalLightComponent).set_intensity(40)
a=ea.spawn_actor_from_class(unreal.SceneCapture2D,unreal.Vector(-23100,12600,640));a.set_actor_rotation(unreal.MathLibrary.find_look_at_rotation(a.get_actor_location(),unreal.Vector(-23800,12100,550)),False);c=a.get_component_by_class(unreal.SceneCaptureComponent2D);tex=unreal.RenderingLibrary.create_render_target2d(world,1280,720,unreal.TextureRenderTargetFormat.RTF_RGBA8);c.texture_target=tex;c.capture_source=unreal.SceneCaptureSource.SCS_FINAL_COLOR_LDR;c.capture_every_frame=False;c.capture_on_movement=False;c.always_persist_rendering_state=True;c.fov_angle=55
for _ in range(20):unreal.PiedmontWorldTools.tick_scene_review();c.capture_scene()
unreal.RenderingLibrary.export_render_target(world,tex,str(root/'work/pride-street-review'),'street-signs.png')
