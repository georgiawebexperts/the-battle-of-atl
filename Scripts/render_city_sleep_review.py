"""Transient flat-floor review of sleeping contact and recovery candidate poses."""
import unreal,json,pathlib,os
root=pathlib.Path(unreal.Paths.project_dir())
settle=os.environ.get('BATTLE_SLEEP_SETTLE')=='1'
stumble=os.environ.get('BATTLE_SLEEP_STUMBLE')=='1' or settle
baked=os.environ.get('BATTLE_SLEEP_WAKE')=='1'
recovery=os.environ.get('BATTLE_SLEEP_RECOVERY')=='1'
reference=os.environ.get('BATTLE_SLEEP_REFERENCE')=='1'
offset=float(os.environ.get('BATTLE_SLEEP_REVIEW_OFFSET','0'))
out=root/('work/'+('settle-' if settle else '')+'sleep-'+('stumble' if stumble else ('wake' if baked else ('recovery' if recovery else ('source' if reference else 'city'))))+'-contact-'+str(offset));out.mkdir(parents=True,exist_ok=True)
assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/PiedmontWorld')
world=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world();ea=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
def vector_values(v):return [v.x,v.y,v.z]
base='/Game/CitySampleCrowd/Character/'
cam=ea.spawn_actor_from_class(unreal.SceneCapture2D,unreal.Vector());c=cam.get_component_by_class(unreal.SceneCaptureComponent2D)
t=unreal.RenderingLibrary.create_render_target2d(world,1280,720,unreal.TextureRenderTargetFormat.RTF_RGBA8)
for prop,value in [('texture_target',t),('capture_source',unreal.SceneCaptureSource.SCS_FINAL_COLOR_LDR),('always_persist_rendering_state',True),('capture_every_frame',False),('capture_on_movement',False),('fov_angle',50)]:c.set_editor_property(prop,value)
floor=ea.spawn_actor_from_class(unreal.StaticMeshActor,unreal.Vector(0,0,10000))
floor.static_mesh_component.set_static_mesh(unreal.load_asset('/Engine/BasicShapes/Cube'))
floor.set_actor_scale3d(unreal.Vector(100,100,.1))
key=ea.spawn_actor_from_class(unreal.DirectionalLight,unreal.Vector(0,0,10500),unreal.Rotator(pitch=-45,yaw=-35))
key.get_component_by_class(unreal.DirectionalLightComponent).set_intensity(12)
results=[]
variants=[]
variants=[('Male','m_tal_nrw','crewneck','m_001','Hair_S_AfroFade','/Game/BattleRetarget/Mixamo/SleepCandidate/MixamoSleepReference_Anim')]
if stumble:
 variants=[('Male','m_tal_nrw','crewneck','m_001','Hair_S_AfroFade','/Game/BattleRetarget/Mixamo/StumbleCandidate/MixamoStumbleReference_Anim')]
if settle:
 variants=[('Male','m_tal_nrw','crewneck','m_001','Hair_S_AfroFade','/Game/BattleRetarget/Mixamo/StumbleCandidate/'+name) for name in ['StumbleToSleep','SleepingAtLanding']]
if baked:
 variants=[('Male','m_tal_nrw','crewneck','m_001','Hair_S_AfroFade','/Game/BattleRetarget/Mixamo/SleepCandidate/SleepToStand_R')]
if recovery:
 variants=[('Male','m_tal_nrw','crewneck','m_001','Hair_S_AfroFade','/Game/BattleRetarget/City/Male/M_ragdoll_getup_stand_'+side) for side in ['F','B','L','R']]
for index,(sex,prefix,outfit,face,hair,clip) in enumerate(variants):
 origin=unreal.Vector(0,0,10005+offset);parts=[]
 paths=[base+sex+'/NormalWeight/Meshes/'+prefix+'_'+part for part in ['body',outfit,'jeans','loafers']]+[base+sex+'/'+face+'/Face/'+face+'_nrw_FaceMesh']
 if reference:
  paths=['/Game/BattleRetarget/Mixamo/SleepingReference/MixamoSleepReference']
  clip='/Game/BattleRetarget/Mixamo/SleepingReference/MixamoSleepReference_Anim'
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
 for actor,component in parts[1:]:component.set_leader_pose_component(body,True,False)
 hair_actor=None
 if not reference:
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
 pos=origin+(unreal.Vector(400,-340,180) if (recovery or baked) else unreal.Vector(320,-280,90));target=origin+unreal.Vector(0,0,65 if (recovery or baked) else 30)
 cam.set_actor_location(pos,False,False);cam.set_actor_rotation(unreal.MathLibrary.find_look_at_rotation(pos,target),False)
 samples=[]
 times=[0,.125,.25,.375,.5,1,2,animation.get_editor_property('sequence_length')-.001] if baked else [animation.get_editor_property('sequence_length')*fraction for fraction in [0,.25,.55,.999]]
 if settle and clip.endswith('StumbleToSleep'):times=[0,4.9,5.3,animation.get_editor_property('sequence_length')-.001]
 for phase,sample_time in enumerate(times):
  body.set_animation_mode(unreal.AnimationMode.ANIMATION_CUSTOM_MODE)
  body.override_animation_data(animation,True,False,sample_time,1.0)
  for frame in range(20):unreal.PiedmontWorldTools.tick_scene_review();c.capture_scene()
  if stumble:
   target=body.get_socket_location('pelvis')
   pos=target+unreal.Vector(400,-340,180)
   cam.set_actor_location(pos,False,False);cam.set_actor_rotation(unreal.MathLibrary.find_look_at_rotation(pos,target),False)
   for frame in range(10):unreal.PiedmontWorldTools.tick_scene_review();c.capture_scene()
  name=sex.lower()+'-'+clip[-1]+'-'+str(phase)+'.png'
  unreal.RenderingLibrary.export_render_target(world,t,str(out),name)
  assert (out/name).is_file(), "Use -AllowCommandletRendering"
  assert abs(body.get_position()-sample_time)<.01,'Preview did not evaluate requested animation time'
  samples.append({'image':name,'animation_position':body.get_position(),'head':vector_values(body.get_socket_location('Head' if reference else 'head')),'hair_origin':vector_values(hair_actor.get_actor_location()) if hair_actor else None,'pelvis':vector_values(body.get_socket_location('Hips' if reference else 'pelvis')),'left_foot':vector_values(body.get_socket_location('LeftFoot' if reference else 'foot_l')),'right_foot':vector_values(body.get_socket_location('RightFoot' if reference else 'foot_r'))})
 results.append({'sex':sex,'clip':clip,'samples':samples})
 for actor,component in parts:ea.destroy_actor(actor)
 if hair_actor:ea.destroy_actor(hair_actor)
 ea.destroy_actor(fill)
(out/'manifest.json').write_text(json.dumps({'map_saved':False,'flat_floor_z':10005,'candidate_mesh_offset_z':offset,'diagnostic_fill_light':{'intensity':40,'radius':1000},'explicit_sampled_pose_evaluation':True,'post_process_disabled_for_diagnostic':True,'hair_attached_to_head':True,'results':results,'scope':'Sleeping candidate poses only; no gameplay or transition acceptance'},indent=2)+'\n')
print('BATTLE_SLEEP_RENDER_COMPLETE')
