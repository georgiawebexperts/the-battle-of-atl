"""Render the local Epic car candidate on 10th Street without saving actors."""
import unreal,pathlib,json
root=pathlib.Path(unreal.Paths.project_dir());out=root/'work/template-car-review';out.mkdir(parents=True,exist_ok=True)
assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/PiedmontWorld')
unreal.PiedmontWorldTools.finish_editor_asset_loading()
ea=unreal.get_editor_subsystem(unreal.EditorActorSubsystem);world=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
def ground(x,y):
 h=unreal.PiedmontWorldTools.trace_world_surface(unreal.Vector(x,y,5000),unreal.Vector(x,y,-5000));assert h;return h
p,surface=ground(-18600,13075)
mesh=unreal.load_asset('/Game/Vehicles/SportsCar/SKM_SportsCar');assert mesh
car=ea.spawn_actor_from_class(unreal.SkeletalMeshActor,unreal.Vector());body=car.skeletal_mesh_component;body.set_skeletal_mesh_asset(mesh)
body.set_editor_property('disable_post_process_blueprint',True)
body.set_update_animation_in_editor(True)
body.set_editor_property('visibility_based_anim_tick_option',unreal.VisibilityBasedAnimTickOption.ALWAYS_TICK_POSE_AND_REFRESH_BONES)
unreal.PiedmontWorldTools.finish_editor_asset_loading()
for _ in range(12):unreal.PiedmontWorldTools.tick_scene_review()
origin,extent=car.get_actor_bounds(False);initial=car.get_actor_location();bottom_offset=origin.z-extent.z-initial.z
car.set_actor_location(p-unreal.Vector(0,0,bottom_offset),False,False)
front,_=ground(p.x+200,p.y);back,_=ground(p.x-200,p.y);car.set_actor_rotation(unreal.MathLibrary.find_look_at_rotation(back,front),False)
cam=ea.spawn_actor_from_class(unreal.SceneCapture2D,unreal.Vector());c=cam.get_component_by_class(unreal.SceneCaptureComponent2D)
t=unreal.RenderingLibrary.create_render_target2d(world,1280,720,unreal.TextureRenderTargetFormat.RTF_RGBA8)
for key,value in [('texture_target',t),('capture_source',unreal.SceneCaptureSource.SCS_FINAL_COLOR_LDR),('capture_every_frame',False),('capture_on_movement',False),('always_persist_rendering_state',True),('fov_angle',45)]:c.set_editor_property(key,value)
for name,offset in [('front',unreal.Vector(650,-550,280)),('rear',unreal.Vector(-650,550,280))]:
 look=p+unreal.Vector(0,0,65);pos=look+offset;cam.set_actor_location(pos,False,False);cam.set_actor_rotation(unreal.MathLibrary.find_look_at_rotation(pos,look),False)
 for _ in range(24):unreal.PiedmontWorldTools.tick_scene_review();c.capture_scene()
 unreal.RenderingLibrary.export_render_target(world,t,str(out),name+'.png');assert (out/(name+'.png')).exists()
(out/'manifest.json').write_text(json.dumps({'mesh':mesh.get_path_name(),'surface':surface.get_actor_label(),'dimensions_cm':[extent.x*2,extent.y*2,extent.z*2],'ground':[p.x,p.y,p.z],'actor_location':[car.get_actor_location().x,car.get_actor_location().y,car.get_actor_location().z],'bottom_offset':bottom_offset,'initial_location':[initial.x,initial.y,initial.z],'map_saved':False,'scope':'Editor appearance candidate; traffic, suspension and wheel contact unverified'},indent=2)+'\n')
