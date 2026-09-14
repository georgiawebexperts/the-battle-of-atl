"""Compare gate paving with and without legacy tutorial cubes; never save the map."""
import hashlib,json,pathlib,unreal
root=pathlib.Path(unreal.Paths.project_dir()).resolve()
main=root/'Content/PiedmontRide/Maps/PiedmontWorld.umap'
before=hashlib.sha256(main.read_bytes()).hexdigest()
assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/PiedmontWorld')
ea=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
world=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
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
out=root/'work/gate-road-overlap';out.mkdir(parents=True,exist_ok=True)
for name,visible in [('before',True),('without-cubes',False)]:
 road.set_visibility(visible)
 for _ in range(36):unreal.PiedmontWorldTools.tick_scene_review();cap.capture_scene()
 unreal.RenderingLibrary.export_render_target(world,tex,str(out),name+'.png')
assert hashlib.sha256(main.read_bytes()).hexdigest()==before
(root/'Tests/Results/2026-09-13-gate-road-overlap.json').write_text(json.dumps({'map_saved':False,'map_sha256':before,'legacy_instances':road.get_instance_count(),'images':[str(out/(n+'.png')) for n in ['before','without-cubes']],'scope':'Transient diagnostic hiding all tutorial pavement, not an approved collision change.'},indent=2)+'\n')
