"""Actual driving exercises rendered feedback and audio state, without accepting final mix/art."""
import unreal,pathlib,json,time,traceback
root=pathlib.Path(unreal.Paths.project_dir());level=unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
report={'status':'running','scope':'Ride feedback integration; visual/audio review separate.','cases':[]};stage=0;mark=0.;started=time.monotonic();bike=None;fx=None;fixture=None;peak=0.;first=None;before=0;last_water=0
unreal.get_default_object(unreal.load_class(None,'/Script/UnrealEd.LevelEditorPlaySettings')).set_editor_property('GameGetsMouseControl',True)
def tick(dt):
 global stage,mark,bike,fx,fixture,peak,first,before,last_water,handle
 try:
  if time.monotonic()-started>180:raise RuntimeError('Feedback test timeout')
  worlds=unreal.EditorLevelLibrary.get_pie_worlds(False)
  if not worlds:return
  w=worlds[0];now=unreal.GameplayStatics.get_time_seconds(w);mode=unreal.GameplayStatics.get_game_mode(w)
  if mode.get_editor_property('start_countdown')>0:return
  def check(name,ok,**extra):report['cases'].append(dict(name=name,**{'pass':bool(ok)},**extra))
  def advance():
   global stage,mark
   stage+=1;mark=now
  def place(p,speed):
   bike.set_actor_location_and_rotation(p,unreal.Rotator(),False,True);bike.ride.stop_movement_immediately();bike.ride.set_editor_property('speed',speed);bike.ride.set_editor_property('gear',5)
  def audio(name):return bike.get_editor_property(name+'_audio')
  if stage==0:
   bike=unreal.GameplayStatics.get_player_pawn(w,0);fx=bike.get_editor_property('ride_effects')
   for d in unreal.GameplayStatics.get_all_actors_of_class(w,unreal.PiedmontTrafficDirector):d.set_actor_tick_enabled(False)
   for p in unreal.GameplayStatics.get_all_actors_of_class(w,unreal.PiedmontPedestrian):p.destroy_actor()
   place(unreal.Vector(-4500,4000,100),1400);bike.validation_key('W',True);advance()
  elif stage==1 and now-mark>.4:
   check('asphalt_motor_audio_play_during_ride',audio('asphalt').is_playing() and audio('motor').is_playing() and audio('asphalt').get_editor_property('volume_multiplier')>.4 and audio('grass').get_editor_property('volume_multiplier')==0)
   bike.validation_key('D',True);advance()
  elif stage==2 and now-mark>.45:
   check('fast_turn_without_brake_leaves_tracks_and_skid_sound',fx.get_editor_property('track_count')>=3 and fx.get_editor_property('tracks').get_instance_count()>=3 and bike.get_editor_property('skid_sounds')==1,tracks=fx.get_editor_property('track_count'))
   check('drift_feedback_never_tips_capsule',abs(bike.get_actor_rotation().roll)<.1 and bike.ride.get_editor_property('wipeouts')==0)
   bike.validation_key('D',False);bike.validation_key('SpaceBar',False);place(unreal.Vector(-4000,9000,100),1000);advance()
  elif stage==3 and now-mark>.7:
   check('grass_switches_tire_audio_and_has_bob',bike.ride.get_editor_property('grass') and audio('grass').get_editor_property('volume_multiplier')>.3 and audio('asphalt').get_editor_property('volume_multiplier')==0 and abs(bike.get_editor_property('handlebar').get_editor_property('relative_location').z-151)>.01)
   place(unreal.Vector(-4000,4000,100),1400);fixture=unreal.PiedmontWorldTools.spawn_validation_obstacle(w,unreal.Vector(-3000,4000,12.5),unreal.Vector(2,12,.25));before=bike.get_editor_property('terrain_bumps');peak=0;advance()
  elif stage==4:
   peak=max(peak,bike.get_editor_property('feedback_strength'))
   if now-mark>1.6:
    check('curb_bobs_and_shakes_without_wipeout',bike.get_actor_location().x>-2500 and bike.get_editor_property('terrain_bumps')>before and peak>.1 and bike.ride.get_editor_property('wipeouts')==0,peak=peak,bumps=bike.get_editor_property('terrain_bumps')-before)
    fixture.destroy_actor();bike.validation_key('W',False);place(unreal.Vector(8500,-9000,110),900);advance()
  elif stage==5 and fx.get_editor_property('splash_count')>0 and bike.get_editor_property('chase').get_editor_property('relative_location').length()>.05:
   check('water_entry_spawns_visible_splash_and_camera_impulse',fx.get_editor_property('active_drops')>=30 and fx.get_editor_property('spray').get_instance_count()==48 and fx.get_editor_property('spray').get_material(0) is not None and bike.get_editor_property('feedback_strength')>1 and bike.ride.get_editor_property('recovery')>1.5)
   last_water=now;first=bike.get_actor_location();advance()
  elif stage==6 and bike.ride.get_editor_property('recovery')<=0:
   check('water_feedback_preserves_two_second_path_return',1.8<now-last_water<2.2 and (bike.get_actor_location()-first).length()>500,seconds=now-last_water)
   check('splash_drops_expire',fx.get_editor_property('active_drops')==0 and fx.get_editor_property('spray').get_instance_count()==0)
   place(unreal.Vector(-4000,4000,100),0);advance()
  elif stage==7 and now-mark>.4:
   check('dismount_for_audio_stop',bike.dismount());advance()
  elif stage==8 and now-mark>.4:
   check('parked_bike_mutes_ride_audio',all(audio(n).get_editor_property('volume_multiplier')==0 for n in ['asphalt','grass','motor']))
   rider=unreal.GameplayStatics.get_player_pawn(w,0);rider.mount_bike();advance()
  elif stage==9 and now-mark>10.5:
   tracks=fx.get_editor_property('tracks');faded=True
   for i in range(tracks.get_instance_count()):
    t=tracks.get_instance_transform(i,True);faded=faded and abs(t.scale3d.y)<.001
   check('old_skids_expire_and_pool_stays_bounded',faded and tracks.get_instance_count()<=160)
   report['status']='passed' if all(c['pass'] for c in report['cases']) else 'failed';unreal.unregister_slate_post_tick_callback(handle)
   if globals().get('WORLD_JOB',{}).get('preview')=='water':
    unreal.GameplayStatics.set_global_time_dilation(w,.06);place(unreal.Vector(8500,-9000,110),900)
   elif globals().get('WORLD_JOB',{}).get('preview')=='skid':
    bike.get_editor_property('arm').set_editor_property('enable_camera_lag',False);unreal.GameplayStatics.set_global_time_dilation(w,.08);place(unreal.Vector(-4000,4000,100),1400);bike.validation_key('W',True);bike.validation_key('D',True)
   else:level.editor_request_end_play()
   stage=10
  report['stage']=stage;(root/'Scripts/ride-feedback-validation.json').write_text(json.dumps(report,indent=2))
 except Exception:
  report['status']='error';report['error']=traceback.format_exc();(root/'Scripts/ride-feedback-validation.json').write_text(json.dumps(report,indent=2));unreal.unregister_slate_post_tick_callback(handle);level.editor_request_end_play()
if globals().get('WORLD_JOB',{}).get('action')=='play':
 level.editor_request_begin_play()
elif globals().get('WORLD_JOB',{}).get('action')=='inspect_skid':
 w=unreal.EditorLevelLibrary.get_pie_worlds(False)[0];bike=unreal.GameplayStatics.get_player_pawn(w,0);tracks=bike.get_editor_property('ride_effects').get_editor_property('tracks');t=tracks.get_instance_transform(tracks.get_instance_count()-4,True);bike.validation_key('W',False);bike.validation_key('D',False);bike.ride.stop_movement_immediately();bike.ride.set_editor_property('speed',0);bike.set_actor_location_and_rotation(t.translation+unreal.Vector(-160,120,98),unreal.Rotator(),False,True);unreal.GameplayStatics.set_global_time_dilation(w,.001)
elif globals().get('WORLD_JOB',{}).get('action')=='preview':
 w=unreal.EditorLevelLibrary.get_pie_worlds(False)[0];bike=unreal.GameplayStatics.get_player_pawn(w,0);bike.get_editor_property('arm').set_editor_property('enable_camera_lag',False);unreal.GameplayStatics.set_global_time_dilation(w,.04);bike.ride.set_editor_property('recovery',0);bike.ride.stop_movement_immediately();bike.set_actor_location_and_rotation(unreal.Vector(8500,-9000,110),unreal.Rotator(),False,True)
else:
 handle=unreal.register_slate_post_tick_callback(tick);level.editor_request_begin_play()
