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
  actor=ea.spawn_actor_from_class(unreal.SkeletalMeshActor,pos,unreal.Rotator(yaw=yaw));actor.set_actor_label('Wreck review '+role);c=actor.skeletal_mesh_component;c.set_editor_property('disable_post_process_blueprint',True);c.set_skeletal_mesh_asset(unreal.load_asset(path));c.set_update_animation_in_editor(True);c.set_forced_lod(1);c.set_editor_property('visibility_based_anim_tick_option',unreal.VisibilityBasedAnimTickOption.ALWAYS_TICK_POSE_AND_REFRESH_BONES);c.set_collision_enabled(unreal.CollisionEnabled.NO_COLLISION);parts.append(c);actors.append(actor)
 body=parts[0];animation=unreal.load_asset(clip);assert animation
 body.set_animation_mode(unreal.AnimationMode.ANIMATION_SINGLE_NODE);body.set_animation(animation);body.set_position(animation.get_editor_property('sequence_length')*fraction,False);body.stop()
 for c in parts[1:]:c.set_leader_pose_component(body,True,False)
 bodies.append((role,body,parts))
# Evaluate each frozen pose before measuring actual clothing/face vertices.
unreal.PiedmontWorldTools.finish_editor_asset_loading()
for _ in range(24):unreal.PiedmontWorldTools.tick_scene_review()
pose_ground=[]
for role,body,parts in bodies:
 samples=[unreal.PiedmontWorldTools.review_skin_ground_clearance(c) for c in parts[1:4]]
 assert all(v is not None for v in samples),role+' has missing ground traces'
 adjustment=1-min(v[0] for v in samples)
 for c in parts:
  a=c.get_owner();a.set_actor_location(a.get_actor_location()+unreal.Vector(0,0,adjustment),False,False)
 verified=[unreal.PiedmontWorldTools.review_skin_ground_clearance(c) for c in parts[1:4]]
 assert all(v is not None for v in verified)
 minimum=min(v[0] for v in verified);assert abs(minimum-1)<.1,(role,minimum)
 pose_ground.append({'role':role,'adjustment_cm':adjustment,'minimum_clearance_cm':minimum,'vertices_checked':sum(v[1] for v in verified),'part_clearances_before_cm':[v[0] for v in samples]})
# Original scooter finishes are saved assets; the scene actors remain transient.
materials={}
for name,color,rough,metal in [('Metal',(.38,.42,.46),.28,.85),('Trim',(.16,.5,.025),.5,0)]:
 dest='/Game/BattleForTheA/Environment/KrogIncident';mat=unreal.load_asset(dest+'/M_Scooter'+name)
 if not mat:
  mat=unreal.AssetToolsHelpers.get_asset_tools().create_asset('M_Scooter'+name,dest,unreal.Material,unreal.MaterialFactoryNew());lib=unreal.MaterialEditingLibrary
  c=lib.create_material_expression(mat,unreal.MaterialExpressionConstant3Vector,-250,0);c.set_editor_property('constant',unreal.LinearColor(*color,1));lib.connect_material_property(c,'',unreal.MaterialProperty.MP_BASE_COLOR)
  for prop,value,yoff in [(unreal.MaterialProperty.MP_ROUGHNESS,rough,120),(unreal.MaterialProperty.MP_METALLIC,metal,220)]:
   n=lib.create_material_expression(mat,unreal.MaterialExpressionConstant,-250,yoff);n.set_editor_property('r',value);lib.connect_material_property(n,'',prop)
  mat.set_editor_property('used_with_instanced_static_meshes',True);lib.recompile_material(mat);unreal.EditorAssetLibrary.save_loaded_asset(mat)
 materials[name]=mat
materials['Rubber']=unreal.load_asset('/Game/BeltLineGlide/Materials/M_Rubber')
frame=unreal.Transform(location=origin+unreal.Vector(-110,100,14),rotation=unreal.Rotator(roll=82,yaw=25))
parts_data=[('deck',(0,0,0),(110,20,7),'Cube',unreal.Rotator(),'Trim'),('grip tape',(0,0,4),(92,16,1),'Cube',unreal.Rotator(),'Rubber'),('stem',(48,0,50),(5,5,100),'Cylinder',unreal.Rotator(),'Metal'),('stem band',(48,0,82),(5.5,5.5,8),'Cylinder',unreal.Rotator(),'Trim'),('handle',(48,0,100),(4,4,48),'Cylinder',unreal.Rotator(roll=90),'Metal')]
for side in (-1,1):parts_data.append(('grip'+str(side),(48,side*18,100),(5,5,12),'Cylinder',unreal.Rotator(roll=90),'Rubber'))
for wheel,xoff in [('front',53),('rear',-53)]:
 parts_data.append((wheel,(xoff,0,-6),(24,24,6),'Cylinder',unreal.Rotator(roll=90),'Rubber'));parts_data.append((wheel+' hub',(xoff,0,-6),(8,8,7),'Cylinder',unreal.Rotator(roll=90),'Metal'))
scooter=[];raise_by=-10000
for name,local,size,mesh,rotation,finish in parts_data:
 a=ea.spawn_actor_from_class(unreal.StaticMeshActor,unreal.Vector());a.set_actor_label('Wreck scooter '+name);c=a.static_mesh_component;c.set_static_mesh(unreal.load_asset('/Engine/BasicShapes/'+mesh));c.set_material(0,materials[finish]);c.set_collision_enabled(unreal.CollisionEnabled.NO_COLLISION)
 local_t=unreal.Transform(location=unreal.Vector(*local),rotation=rotation,scale=unreal.Vector(*(v/100 for v in size)));t=unreal.MathLibrary.compose_transforms(local_t,frame);a.set_actor_transform(t,False,True);actors.append(a);scooter.append(a)
 for sx in (-50,50):
  for sy in (-50,50):
   for sz in (-50,50):
    point=t.transform_location(unreal.Vector(sx,sy,sz));hit=unreal.PiedmontWorldTools.trace_world_surface(unreal.Vector(point.x,point.y,z+300),unreal.Vector(point.x,point.y,z-300));assert hit
    raise_by=max(raise_by,hit[0].z-point.z+1)
for a in scooter:a.set_actor_location(a.get_actor_location()+unreal.Vector(0,0,raise_by),False,False)
cam=ea.spawn_actor_from_class(unreal.SceneCapture2D,unreal.Vector());capture=cam.get_component_by_class(unreal.SceneCaptureComponent2D);target=unreal.RenderingLibrary.create_render_target2d(world,1280,720,unreal.TextureRenderTargetFormat.RTF_RGBA8)
for prop,val in [('texture_target',target),('capture_source',unreal.SceneCaptureSource.SCS_FINAL_COLOR_LDR),('always_persist_rendering_state',True),('capture_every_frame',False),('capture_on_movement',False),('fov_angle',55)]:capture.set_editor_property(prop,val)
for sky in ea.get_all_level_actors():
 if isinstance(sky,unreal.SkyLight):
  sky.get_component_by_class(unreal.SkyLightComponent).set_editor_property('real_time_capture',False);sky.get_component_by_class(unreal.SkyLightComponent).recapture_sky()
unreal.PiedmontWorldTools.finish_editor_asset_loading()
fill=ea.spawn_actor_from_class(unreal.PointLight,origin+unreal.Vector(250,-250,350));light=fill.get_component_by_class(unreal.PointLightComponent);light.set_intensity(100);light.set_editor_property('attenuation_radius',1800);light.set_cast_shadows(True)
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
report={'images':images,'map_saved':False,'diagnostic_fill_light':True,'hair_omitted_for_pose_review':True,'forced_character_lod':0,'pose_ground_scope':'Clothing and shoes LOD0; excludes face asset root geometry and does not prove physical support.','pose_ground':pose_ground,'scooter_parts':len(scooter),'scooter_ground_adjustment_cm':raise_by,'visual_review':'pending','scope':'Frozen pose/primitive-scooter composition only. No collision, animation transitions, activation, rarity or gameplay acceptance.'}
(root/'Tests/Results/2026-09-12-krog-wreck-visual-candidate.json').write_text(json.dumps(report,indent=2)+'\n')
