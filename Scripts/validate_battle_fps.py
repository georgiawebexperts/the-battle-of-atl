"""V3 live possession, FPS controls, pistol and parking integration."""
import unreal,json,pathlib,time,traceback
root=pathlib.Path(unreal.Paths.project_dir());level=unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
report={'status':'running','scope':'FPS/pistol handoff foundation, not full combat or milestone fun acceptance.','cases':[]};stage=0;started=time.monotonic();mark=0.;bike=None;rider=None;home=None;target=None;shots=0;jump_z=0
unreal.get_default_object(unreal.load_class(None,'/Script/UnrealEd.LevelEditorPlaySettings')).set_editor_property('GameGetsMouseControl',True)
def tick(dt):
 global stage,mark,bike,rider,home,target,shots,jump_z,handle
 try:
  if time.monotonic()-started>160:raise RuntimeError('FPS integration timeout')
  worlds=unreal.EditorLevelLibrary.get_pie_worlds(False)
  if not worlds:return
  w=worlds[0];mode=unreal.GameplayStatics.get_game_mode(w);now=unreal.GameplayStatics.get_time_seconds(w);pc=unreal.GameplayStatics.get_player_controller(w,0)
  if mode.get_editor_property('start_countdown')>0:return
  def check(name,ok,**kw):report['cases'].append({'name':name,'pass':bool(ok),**kw})
  def next_stage():
   global stage,mark
   stage+=1;mark=now
  if stage==0:
   bike=unreal.GameplayStatics.get_player_pawn(w,0)
   for d in unreal.GameplayStatics.get_all_actors_of_class(w,unreal.PiedmontTrafficDirector):d.set_actor_tick_enabled(False)
   for v in unreal.GameplayStatics.get_all_actors_of_class(w,unreal.PiedmontPedestrian):v.destroy_actor()
   bike.ride.set_editor_property('speed',900);home=bike.get_actor_location();bike.validation_key('E',True);next_stage()
  elif stage==1 and now-mark>.4:
   rider=unreal.GameplayStatics.get_player_pawn(w,0);check('E_dismount_possesses_first_person_rider',isinstance(rider,unreal.BattleRider))
   if not isinstance(rider,unreal.BattleRider):raise RuntimeError('E did not dismount')
   rider.validation_key('E',False);check('bike_stops_and_parks',bike.get_editor_property('parked') and bike.get_velocity().length()<1 and bike.ride.get_editor_property('speed')==0)
   home=bike.get_actor_location();camera=rider.get_editor_property('camera');eye=camera.get_world_location();check('camera_at_rider_eye',abs(eye.z-rider.get_actor_location().z-64)<2 and (eye-rider.get_actor_location()).length()<80)
   rider.validation_key('W',True);rider.validation_key('LeftShift',True);next_stage()
  elif stage==2 and now-mark>1:
   check('sprint_moves_fps_rider',rider.get_velocity().length()>700,speed=rider.get_velocity().length());check('parked_bike_stays_put',(bike.get_actor_location()-home).length()<1)
   check('cannot_remount_from_far_away',not rider.mount_bike());rider.validation_key('W',False);rider.validation_key('LeftShift',False);jump_z=rider.get_actor_location().z;rider.validation_key('SpaceBar',True);shots=rider.get_editor_property('shots_fired');rider.validation_key('LeftMouseButton',True);next_stage()
  elif stage==3 and now-mark>.3:
   check('space_jumps',rider.get_actor_location().z>jump_z+30,height=rider.get_actor_location().z-jump_z);check('pistol_fires_while_airborne',rider.get_editor_property('shots_fired')>shots);rider.validation_key('SpaceBar',False);rider.validation_key('LeftMouseButton',False);next_stage()
  elif stage==4 and now-mark>1:
   rider.set_actor_location_and_rotation(home+unreal.Vector(0,160,0),unreal.Rotator(),False,True);rider.get_component_by_class(unreal.CharacterMovementComponent).stop_movement_immediately();rider.validation_key('RightMouseButton',True);next_stage()
  elif stage==5 and now-mark>.5:
   check('right_mouse_aims',rider.get_editor_property('aiming') and rider.get_editor_property('camera').get_editor_property('field_of_view')<70)
   rider.validation_key('RightMouseButton',False);rider.set_editor_property('ammo',0);rider.validation_key('R',True);next_stage()
  elif stage==6 and now-mark>.2:rider.validation_key('R',False);next_stage()
  elif stage==7 and now-mark>1.7:
   check('reload_refills_without_reserve_limit',rider.get_editor_property('ammo')==12)
   ds=unreal.GameplayStatics.get_all_actors_of_class(w,unreal.PiedmontTrafficDirector);target=ds[0].spawn_visitor_for_validation(home+unreal.Vector(700,160,-90),False)
   if not target:raise RuntimeError('Could not spawn pistol collision target')
   target.get_controller().stop_movement();target.get_component_by_class(unreal.CharacterMovementComponent).disable_movement();target.set_actor_tick_enabled(False);rider.aim_at_for_validation(target);next_stage()
  elif stage==8 and now-mark>.3:
   check('pistol_hits_target',rider.fire() and target.get_editor_property('dead'));target.destroy_actor();rider.set_editor_property('health',63.0);rider.validation_key('E',True);next_stage()
  elif stage==9 and now-mark>.3:
   check('E_remount_restores_bike_possession',unreal.GameplayStatics.get_player_pawn(w,0)==bike and not bike.get_editor_property('parked'));bike.validation_key('E',False)
   check('remount_preserves_health_and_magazine',63<=bike.get_editor_property('rider_health')<70 and bike.get_editor_property('pistol_ammo')==11)
   check('one_player_pawn_after_remount',len(unreal.GameplayStatics.get_all_actors_of_class(w,unreal.BattleRider))==0)
   report['status']='passed' if all(c['pass'] for c in report['cases']) else 'failed';unreal.unregister_slate_post_tick_callback(handle)
   if globals().get('WORLD_JOB',{}).get('preview'):bike.dismount()
   else:level.editor_request_end_play()
   stage=10
  report['stage']=stage;(root/'Scripts/battle-fps-validation.json').write_text(json.dumps(report,indent=2))
 except Exception:
  report['status']='error';report['error']=traceback.format_exc();(root/'Scripts/battle-fps-validation.json').write_text(json.dumps(report,indent=2));unreal.unregister_slate_post_tick_callback(handle);level.editor_request_end_play()
handle=unreal.register_slate_post_tick_callback(tick);level.editor_request_begin_play()
