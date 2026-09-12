"""Transient editor pose diagnostic for the low_reaching candidate; not gameplay acceptance."""
import unreal,json,pathlib
root=pathlib.Path(unreal.Paths.project_dir());out=root/'work/low_reach-review';out.mkdir(parents=True,exist_ok=True)
assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/PiedmontWorld')
unreal.PiedmontWorldTools.finish_editor_asset_loading()
world=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world();ea=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
def vector_values(v):return [v.x,v.y,v.z]
base='/Game/CitySampleCrowd/Character/'
cam=ea.spawn_actor_from_class(unreal.SceneCapture2D,unreal.Vector());c=cam.get_component_by_class(unreal.SceneCaptureComponent2D)
t=unreal.RenderingLibrary.create_render_target2d(world,1280,720,unreal.TextureRenderTargetFormat.RTF_RGBA8)
for prop,value in [('texture_target',t),('capture_source',unreal.SceneCaptureSource.SCS_FINAL_COLOR_LDR),('always_persist_rendering_state',True),('capture_every_frame',False),('capture_on_movement',False),('fov_angle',50)]:c.set_editor_property(prop,value)
results=[]
variants=[('Male','m_tal_nrw','crewneck','m_001','Hair_S_AfroFade','/Game/BattleRetarget/Mixamo/LowReachCandidate/MixamoLowReachReference_Anim')]
for index,(sex,prefix,outfit,face,hair,clip) in enumerate(variants):
 x=-17400;y=-5200
 hit=unreal.PiedmontWorldTools.trace_world_surface(unreal.Vector(x,y,7000),unreal.Vector(x,y,-7000),0);assert hit
 origin=hit[0];parts=[]
 paths=[base+sex+'/NormalWeight/Meshes/'+prefix+'_'+part for part in ['body',outfit,'jeans','loafers']]+[base+sex+'/'+face+'/Face/'+face+'_nrw_FaceMesh']
 for path in paths:
  actor=ea.spawn_actor_from_class(unreal.SkeletalMeshActor,origin,unreal.Rotator(yaw=-90));component=actor.skeletal_mesh_component
  component.set_editor_property('disable_post_process_blueprint',True)
  mesh=unreal.load_asset(path);assert mesh
  component.set_skeletal_mesh_asset(mesh)
  component.set_update_animation_in_editor(True)
  component.set_editor_property('visibility_based_anim_tick_option',unreal.VisibilityBasedAnimTickOption.ALWAYS_TICK_POSE_AND_REFRESH_BONES)
  parts.append((actor,component))
 body=parts[0][1]
 animation=unreal.load_asset(clip);assert animation
 body.set_animation_mode(unreal.AnimationMode.ANIMATION_SINGLE_NODE)
 body.set_animation(animation)
 body.play(True)
 for actor,component in parts[1:]:component.set_leader_pose_component(body,True,False)
 hair_actor=ea.spawn_actor_from_class(unreal.StaticMeshActor,origin,unreal.Rotator(yaw=-90))
 hair_actor.static_mesh_component.set_static_mesh(unreal.load_asset(base+sex+'/'+face+'/Hair/Hair/'+hair+'_CardsMesh_Group0_LOD0'))
 hair_actor.static_mesh_component.set_mobility(unreal.ComponentMobility.MOVABLE)
 assert hair_actor.attach_to_component(body,'head',unreal.AttachmentRule.KEEP_WORLD,unreal.AttachmentRule.KEEP_WORLD,unreal.AttachmentRule.KEEP_WORLD,False)
 fill=ea.spawn_actor_from_class(unreal.PointLight,origin+unreal.Vector(220,-180,230))
 fill_component=fill.get_component_by_class(unreal.PointLightComponent)
 fill_component.set_intensity(40)
 fill_component.set_editor_property('attenuation_radius',1000)
 fill_component.set_cast_shadows(False)
 unreal.PiedmontWorldTools.finish_editor_asset_loading()
 pos=origin+unreal.Vector(480,-400,200);target=origin+unreal.Vector(0,0,65)
 cam.set_actor_location(pos,False,False);cam.set_actor_rotation(unreal.MathLibrary.find_look_at_rotation(pos,target),False)
 samples=[]
 for phase,fraction in enumerate([.03,.25,.55,.95]):
  sample_time=animation.get_editor_property('sequence_length')*fraction
  body.set_animation_mode(unreal.AnimationMode.ANIMATION_CUSTOM_MODE)
  body.override_animation_data(animation,True,False,sample_time,1.0)
  for frame in range(20):unreal.PiedmontWorldTools.tick_scene_review();c.capture_scene()
  name=sex.lower()+'-'+clip[-1]+'-'+str(phase)+'.png'
  unreal.RenderingLibrary.export_render_target(world,t,str(out),name)
  assert (out/name).is_file(), "Use -AllowCommandletRendering"
  assert abs(body.get_position()-sample_time)<.01,'Preview did not evaluate requested animation time'
  samples.append({'image':name,'animation_position':body.get_position(),'head':vector_values(body.get_socket_location('head')),'hair_origin':vector_values(hair_actor.get_actor_location()),'pelvis':vector_values(body.get_socket_location('pelvis')),'right_hand':vector_values(body.get_socket_location('hand_r')),'left_hand':vector_values(body.get_socket_location('hand_l')),'left_foot':vector_values(body.get_socket_location('foot_l')),'right_foot':vector_values(body.get_socket_location('foot_r'))})
 results.append({'sex':sex,'clip':clip,'samples':samples})
 for actor,component in parts:ea.destroy_actor(actor)
 ea.destroy_actor(hair_actor)
 ea.destroy_actor(fill)
(out/'manifest.json').write_text(json.dumps({'map_saved':False,'diagnostic_fill_light':{'intensity':40,'radius':1000},'explicit_sampled_pose_evaluation':True,'post_process_disabled_for_diagnostic':True,'hair_attached_to_head':True,'results':results,'scope':'LowReaching candidate editor poses only; native playback, bench alignment and interaction still pending'},indent=2)+'\n')
print('BATTLE_REACH_RENDER_COMPLETE')
