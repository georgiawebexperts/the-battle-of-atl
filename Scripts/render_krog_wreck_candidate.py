"""Transient scooter-help staging review; no event or map placement is committed."""
import unreal,json,uuid,math
from pathlib import Path
root=Path(unreal.Paths.project_dir()).resolve();out=root/'work/krog-wreck-review'/uuid.uuid4().hex;out.mkdir(parents=True)
assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/PiedmontWorld')
unreal.PiedmontWorldTools.finish_editor_asset_loading()
ea=unreal.get_editor_subsystem(unreal.EditorActorSubsystem);world=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
r=json.loads((root/'Tests/Results/2026-09-12-krog-wreck-site-survey.json').read_text());site=next(c for c in r['candidates'] if c['back_from_portal_cm']==r['preferred_candidate']['back_from_portal_cm'] and c['lateral_offset_cm']==r['preferred_candidate']['lateral_offset_cm']);x,y=site['center_xy'];z=site['samples'][0]['z'];origin=unreal.Vector(x,y,z)
base='/Game/CitySampleCrowd/Character/Male/';actors=[];bodies=[]
roles=[('rider',(0,0),0,'/Game/BattleRetarget/City/Male/M_ragdoll_getup_stand_B',.08),('helper',(-40,-100),0,'/Game/BattleRetarget/City/Male/M_ragdoll_getup_stand_B',.30),('bystander',(145,90),150,'/Game/BattleRetarget/Mixamo/LowReachCandidate/MixamoLowReachReference_Anim',.03)]
for role,(ox,oy),yaw,clip,fraction in roles:
 hit=unreal.PiedmontWorldTools.trace_world_surface(unreal.Vector(x+ox,y+oy,z+300),unreal.Vector(x+ox,y+oy,z-300));assert hit
 pos=hit[0];parts=[]
 for path in [base+'NormalWeight/Meshes/m_tal_nrw_'+n for n in ('body','crewneck','jeans','loafers')]+[base+'m_001/Face/m_001_nrw_FaceMesh']:
  actor=ea.spawn_actor_from_class(unreal.SkeletalMeshActor,pos,unreal.Rotator(yaw=yaw));actor.set_actor_label('Wreck review '+role);c=actor.skeletal_mesh_component;c.set_editor_property('disable_post_process_blueprint',True);c.set_skeletal_mesh_asset(unreal.load_asset(path));c.set_update_animation_in_editor(True);c.set_editor_property('visibility_based_anim_tick_option',unreal.VisibilityBasedAnimTickOption.ALWAYS_TICK_POSE_AND_REFRESH_BONES);c.set_collision_enabled(unreal.CollisionEnabled.NO_COLLISION);parts.append(c);actors.append(actor)
 body=parts[0];animation=unreal.load_asset(clip);assert animation
 body.set_animation_mode(unreal.AnimationMode.ANIMATION_CUSTOM_MODE);body.override_animation_data(animation,True,False,animation.get_editor_property('sequence_length')*fraction,1.0)
 for c in parts[1:]:c.set_leader_pose_component(body,True,False)
 bodies.append((role,body))
# Fallen scooter: grounded deck, two rubber wheels, steering stem and bar.
frame=unreal.Transform(location=origin+unreal.Vector(-110,100,14),rotation=unreal.Rotator(roll=82,yaw=25))
for name,local,size,mesh,rotation in [('deck',(0,0,0),(100,20,7),'Cube',unreal.Rotator()),('stem',(43,0,50),(5,5,100),'Cube',unreal.Rotator()),('handle',(43,0,100),(5,44,5),'Cube',unreal.Rotator()),('front',(43,0,0),(20,20,6),'Cylinder',unreal.Rotator(roll=90)),('rear',(-43,0,0),(20,20,6),'Cylinder',unreal.Rotator(roll=90))]:
 a=ea.spawn_actor_from_class(unreal.StaticMeshActor,unreal.Vector());a.set_actor_label('Wreck scooter '+name);c=a.static_mesh_component;c.set_static_mesh(unreal.load_asset('/Engine/BasicShapes/'+mesh));c.set_material(0,unreal.load_asset('/Game/BattleForTheA/Furniture/M_BenchFrame'));c.set_collision_enabled(unreal.CollisionEnabled.NO_COLLISION)
 local_t=unreal.Transform(location=unreal.Vector(*local),rotation=rotation,scale=unreal.Vector(*(v/100 for v in size)));t=unreal.MathLibrary.compose_transforms(local_t,frame);a.set_actor_transform(t,False,True);actors.append(a)
cam=ea.spawn_actor_from_class(unreal.SceneCapture2D,unreal.Vector());capture=cam.get_component_by_class(unreal.SceneCaptureComponent2D);target=unreal.RenderingLibrary.create_render_target2d(world,1280,720,unreal.TextureRenderTargetFormat.RTF_RGBA8)
for prop,val in [('texture_target',target),('capture_source',unreal.SceneCaptureSource.SCS_FINAL_COLOR_LDR),('always_persist_rendering_state',True),('capture_every_frame',False),('capture_on_movement',False),('fov_angle',55)]:capture.set_editor_property(prop,val)
for sky in ea.get_all_level_actors():
 if isinstance(sky,unreal.SkyLight):
  sky.get_component_by_class(unreal.SkyLightComponent).set_editor_property('real_time_capture',False);sky.get_component_by_class(unreal.SkyLightComponent).recapture_sky()
unreal.PiedmontWorldTools.finish_editor_asset_loading()
fill=ea.spawn_actor_from_class(unreal.PointLight,origin+unreal.Vector(250,-250,350));light=fill.get_component_by_class(unreal.PointLightComponent);light.set_intensity(100);light.set_editor_property('attenuation_radius',1800);light.set_cast_shadows(False)
images=[]
offsets=[];sightlines=[]
for angle in range(0,360,30):
 offset=unreal.Vector(550*math.cos(math.radians(angle)),550*math.sin(math.radians(angle)),300);eye=origin+offset;top=unreal.PiedmontWorldTools.trace_world_surface(eye+unreal.Vector(0,0,5000),eye)
 obstruction=unreal.PiedmontWorldTools.trace_world_surface(eye,origin+unreal.Vector(0,0,60))
 sightlines.append({'angle':angle,'overhead':top[1].get_actor_label() if top else None,'obstruction':obstruction[1].get_actor_label() if obstruction else None})
 if not top and not obstruction:offsets.append(offset)
(root/'work/krog-wreck-sightlines.json').write_text(json.dumps(sightlines,indent=2))
assert len(offsets)>=2,'No clear camera sightlines'
for i,offset in enumerate((offsets[0],offsets[len(offsets)//2])):
 eye=origin+offset;cam.set_actor_location(eye,False,False);cam.set_actor_rotation(unreal.MathLibrary.find_look_at_rotation(eye,origin+unreal.Vector(0,0,60)),False)
 for _ in range(24):unreal.PiedmontWorldTools.tick_scene_review();capture.capture_scene()
 name=f'scene-{i}.png';unreal.RenderingLibrary.export_render_target(world,target,str(out),name);images.append(str(out/name))
report={'images':images,'map_saved':False,'diagnostic_fill_light':True,'hair_omitted_for_pose_review':True,'visual_review':'pending','scope':'Frozen pose/primitive-scooter composition only. No collision, animation transitions, activation, rarity or gameplay acceptance.'}
(root/'Tests/Results/2026-09-12-krog-wreck-visual-candidate.json').write_text(json.dumps(report,indent=2)+'\n')
