"""Live near-miss boost and shared bike/FPS shot feedback checks."""
import unreal,json,pathlib,time,traceback,math
root=pathlib.Path(unreal.Paths.project_dir());level=unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
report={'status':'running','scope':'Near-miss nitro and bike pistol/visible shot effects; not full enemy roster or combat feel acceptance.','cases':[]};stage=0;mark=0.;started=time.monotonic();bike=None;visitor=None;director=None;boost_start=None;peak=0.;fx_seen=False
unreal.get_default_object(unreal.load_class(None,'/Script/UnrealEd.LevelEditorPlaySettings')).set_editor_property('GameGetsMouseControl',True)
def tick(dt):
 global stage,mark,bike,visitor,director,boost_start,peak,fx_seen,handle
 try:
  if time.monotonic()-started>160:raise RuntimeError('Action validation timeout')
  worlds=unreal.EditorLevelLibrary.get_pie_worlds(False)
  if not worlds:return
  w=worlds[0];now=unreal.GameplayStatics.get_time_seconds(w);mode=unreal.GameplayStatics.get_game_mode(w)
  if mode.get_editor_property('start_countdown')>0:return
  def check(name,ok,**kw):report['cases'].append({'name':name,'pass':bool(ok),**kw})
  def advance():
   global stage,mark
   stage+=1;mark=now
  def reset():
   bike.set_actor_location_and_rotation(unreal.Vector(-4000,4000,100),unreal.Rotator(),False,True);bike.ride.stop_movement_immediately();bike.ride.set_editor_property('speed',1600);bike.ride.set_editor_property('gear',5);bike.validation_key('W',True)
  if stage==0:
   bike=unreal.GameplayStatics.get_player_pawn(w,0);director=unreal.GameplayStatics.get_all_actors_of_class(w,unreal.PiedmontTrafficDirector)[0];director.set_actor_tick_enabled(False)
   for p in unreal.GameplayStatics.get_all_actors_of_class(w,unreal.PiedmontPedestrian):p.destroy_actor()
   reset();visitor=director.spawn_visitor_for_validation(unreal.Vector(-3300,4145,3),False)
   if not visitor:raise RuntimeError('Near-miss target failed to spawn')
   visitor.get_controller().stop_movement();visitor.get_component_by_class(unreal.CharacterMovementComponent).disable_movement();visitor.set_actor_tick_enabled(False);advance()
  elif stage==1 and now-mark>1:
   check('close_pass_rewards_nitro_without_contact',bike.get_editor_property('near_misses')==1 and bike.get_editor_property('nitro')==20 and visitor.get_editor_property('bike_contacts')==0)
   check('partial_meter_cannot_boost',not bike.boost());reset();advance()
  elif stage==2 and now-mark>1:
   check('same_visitor_cannot_be_farmed_immediately',bike.get_editor_property('near_misses')==1);visitor.destroy_actor();bike.set_editor_property('nitro',100);bike.validation_key('LeftShift',True);advance()
  elif stage==3:
   if bike.ride.get_editor_property('boost_remaining')>0:
    if boost_start is None:boost_start=now
    peak=max(peak,bike.ride.get_editor_property('speed'));bike.validation_key('LeftShift',False)
   elif boost_start is not None:
    check('shift_consumes_meter_and_boosts_for_three_seconds',2.6<now-boost_start<3.35 and bike.get_editor_property('nitro')==0 and peak>1750,elapsed=now-boost_start,peak_speed=peak)
    bike.validation_key('W',False);bike.ride.set_editor_property('speed',0);bike.ride.stop_movement_immediately();bike.set_actor_location_and_rotation(unreal.Vector(-4000,4000,100),unreal.Rotator(),False,True);advance()
   elif now-mark>1:raise RuntimeError('Shift failed to activate boost')
  elif stage==4 and now-mark>.5:
   camera=bike.get_editor_property('chase');eye=camera.get_world_location();forward=camera.get_forward_vector();distance=(110-eye.z)/forward.z;point=eye+forward*distance;visitor=mode.spawn_threat_for_validation(True,unreal.Vector(point.x,point.y,90))
   if not visitor:raise RuntimeError('Bike shot target failed to spawn')
   if visitor.get_controller():visitor.get_controller().stop_movement()
   visitor.get_component_by_class(unreal.CharacterMovementComponent).disable_movement();visitor.set_actor_tick_enabled(False);bike.validation_key('LeftMouseButton',True);advance()
  elif stage==5:
   fx_seen=fx_seen or len(unreal.GameplayStatics.get_all_actors_of_class(w,unreal.BattleShotFX))>0
   if visitor.get_editor_property('dead') or now-mark>2:
    check('left_click_fires_pistol_from_bike',bike.get_editor_property('shots_fired')>0)
    check('bike_camera_aim_hits_visible_target',visitor.get_editor_property('dead'),end=str(bike.get_editor_property('last_shot_end')))
    check('shot_spawns_renderable_muzzle_and_tracer',fx_seen)
    check('damage_shows_hit_feedback',bike.get_editor_property('hit_feedback')>0)
    check('enemy_kill_refills_nitro',bike.get_editor_property('enemy_kills')==1 and bike.get_editor_property('nitro')==25)
    bike.validation_key('LeftMouseButton',False);visitor.destroy_actor();advance()
  elif stage==6 and now-mark>.4:
   bike.ride.set_editor_property('speed',1600);bike.fire_pistol();check('riding_at_speed_increases_pistol_spread',bike.get_editor_property('pistol_spread')>=2.9)
   check('ground_hit_does_not_show_damage_marker',bike.get_editor_property('hit_feedback')<=0)
   bike.ride.set_editor_property('speed',0);report['status']='passed' if all(c['pass'] for c in report['cases']) else 'failed';unreal.unregister_slate_post_tick_callback(handle)
   if globals().get('WORLD_JOB',{}).get('preview'):bike.set_editor_property('nitro',60);bike.validation_key('LeftMouseButton',True)
   else:level.editor_request_end_play()
   stage=7
  report['stage']=stage;(root/'Scripts/battle-action-validation.json').write_text(json.dumps(report,indent=2))
 except Exception:
  report['status']='error';report['error']=traceback.format_exc();(root/'Scripts/battle-action-validation.json').write_text(json.dumps(report,indent=2));unreal.unregister_slate_post_tick_callback(handle);level.editor_request_end_play()
handle=unreal.register_slate_post_tick_callback(tick);level.editor_request_begin_play()
