"""Static editor-scene review. Run via render_park_review.sh; never saves the map."""
import unreal,pathlib,json,datetime,hashlib,struct
root=pathlib.Path(unreal.Paths.project_dir());out=root/'work/scene-review';out.mkdir(parents=True,exist_ok=True)
assert unreal.SystemLibrary.get_console_variable_int_value('r.TextureStreaming')==0,'Use -NoTextureStreaming to load landscape heightmaps completely.'
assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/PiedmontWorld')
world=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world();ea=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
unreal.PiedmontWorldTools.finish_editor_asset_loading()
# Bright diagnostic lighting is transient. It is not a claim about game exposure.
lights=[]
for actor in ea.get_all_level_actors():
 if isinstance(actor,unreal.DirectionalLight):
  light=actor.get_component_by_class(unreal.DirectionalLightComponent);lights.append({'actor':actor.get_actor_label(),'saved_intensity':light.get_editor_property('intensity'),'review_intensity':40});light.set_intensity(40)
cam=ea.spawn_actor_from_class(unreal.SceneCapture2D,unreal.Vector());capture=cam.get_component_by_class(unreal.SceneCaptureComponent2D)
texture=unreal.RenderingLibrary.create_render_target2d(world,1280,720,unreal.TextureRenderTargetFormat.RTF_RGBA8)
capture.set_editor_property('texture_target',texture);capture.set_editor_property('capture_source',unreal.SceneCaptureSource.SCS_FINAL_COLOR_LDR)
capture.set_editor_property('always_persist_rendering_state',True);capture.set_editor_property('capture_every_frame',False);capture.set_editor_property('capture_on_movement',False);capture.set_editor_property('fov_angle',85)
views=[('gate-oblique',[-17400,-5200,600],[-14000,-5200,100]),('park-aerial',[-16000,16000,18000],[-5000,-2000,-400])]
images=[]
for name,position,target in views:
 location=unreal.Vector(*position);cam.set_actor_location(location,False,False);cam.set_actor_rotation(unreal.MathLibrary.find_look_at_rotation(location,unreal.Vector(*target)),False)
 for _ in range(12):
  unreal.PiedmontWorldTools.tick_scene_review()
  capture.capture_scene()
 unreal.RenderingLibrary.export_render_target(world,texture,str(out),name+'.png')
 file=out/(name+'.png');data=file.read_bytes();assert data[:8]==b'\x89PNG\r\n\x1a\n';size=struct.unpack('>II',data[16:24]);assert size==(1280,720)
 images.append({'file':file.name,'camera':position,'target':target,'size':list(size),'sha256':hashlib.sha256(data).hexdigest()})
result={'created_utc':datetime.datetime.now(datetime.timezone.utc).isoformat(),'map':'/Game/PiedmontRide/Maps/PiedmontWorld','map_saved':False,'capture':'static editor SceneCapture2D, diagnostic light override','texture_streaming':0,'warmup_frames_per_view':12,'persistent_capture_state':True,'force_lod':unreal.SystemLibrary.get_console_variable_int_value('r.ForceLOD'),'lights':lights,'images':images,'scope':'Static scene only. No game-mode spawning, HUD, input, audio or FPS acceptance.'}
(out/'manifest.json').write_text(json.dumps(result,indent=2)+'\n')
