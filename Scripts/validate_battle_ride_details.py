"""New bike horn, automatic lights, curb and collision/exit edge cases."""
import unreal,json,pathlib,time,traceback
root=pathlib.Path(unreal.Paths.project_dir());level=unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
report={'status':'running','cases':[]};stage=0;mark=0.;started=time.monotonic();bike=None;walker=None;jogger=None;zone=None;origin=None;first=None;fixture=None;rows=[];director=None;walls=[];lit_seen=False
unreal.get_default_object(unreal.load_class(None,'/Script/UnrealEd.LevelEditorPlaySettings')).set_editor_property('GameGetsMouseControl',True)
def tick(dt):
 global stage,mark,bike,walker,jogger,zone,origin,first,fixture,rows,director,walls,lit_seen,handle
 try:
  if time.monotonic()-started>180:raise RuntimeError('Ride details test timeout')
  worlds=unreal.EditorLevelLibrary.get_pie_worlds(False)
  if not worlds:return
  w=worlds[0];now=unreal.GameplayStatics.get_time_seconds(w);mode=unreal.GameplayStatics.get_game_mode(w)
  if mode.get_editor_property('start_countdown')>0:return
  def check(name,ok,**kw):report['cases'].append({'name':name,'pass':bool(ok),**kw})
  def advance():
   global stage,mark
   stage+=1;mark=now
  def reset(speed=0):
   bike.set_actor_location_and_rotation(origin,unreal.Rotator(),False,True);bike.ride.stop_movement_immediately();bike.ride.set_editor_property('speed',speed);bike.ride.set_editor_property('gear',5)
  if stage==0:
   bike=unreal.GameplayStatics.get_player_pawn(w,0);origin=unreal.Vector(-4000,4000,100);reset();director=unreal.GameplayStatics.get_all_actors_of_class(w,unreal.PiedmontTrafficDirector)[0];director.set_actor_tick_enabled(False)
   for p in unreal.GameplayStatics.get_all_actors_of_class(w,unreal.PiedmontPedestrian):p.destroy_actor()
   walker=director.spawn_visitor_for_validation(origin+unreal.Vector(500,0,-97),False);jogger=director.spawn_visitor_for_validation(origin+unreal.Vector(950,0,-97),True)
   if not walker or not jogger:raise RuntimeError('Horn visitors did not spawn')
   walker.set_editor_property('pause_remaining',5);jogger.set_editor_property('pause_remaining',5);first=walker.get_actor_location();bike.validation_key('H',True);advance()
  elif stage==1 and now-mark>.25:
   bike.validation_key('H',False);bike.horn();check('H_horn_and_repeat_limit',bike.get_editor_property('horn_count')==1);advance()
  elif stage==2 and now-mark>1.1:
   check('walker_moves_aside_for_new_bike_horn',walker.get_editor_property('horn_reactions')==1 and (walker.get_actor_location()-first).length()>25)
   check('jogger_keeps_course_despite_horn',jogger.get_editor_property('horn_reactions')==0);walker.destroy_actor();jogger.destroy_actor();check('daylight_lights_off',not bike.get_editor_property('lights_on'));zone=unreal.PiedmontWorldTools.spawn_validation_dark_zone(w,origin);advance()
  elif stage==3 and now-mark>.5:
   check('dark_zone_lights_head_and_tail',bike.get_editor_property('lights_on') and bike.get_editor_property('headlight').is_visible() and bike.get_editor_property('tail_light').is_visible());zone.set_actor_location(origin+unreal.Vector(3000,0,0),False,True);advance()
  elif stage==4 and now-mark>2:
   check('lights_switch_off_after_leaving_darkness',not bike.get_editor_property('lights_on'));zone.set_actor_location(origin,False,True);check('dismount_for_parked_light_test',bike.dismount());advance()
  elif stage==5 and now-mark>.5:
   check('parked_bike_still_turns_lights_on',bike.get_editor_property('parked') and bike.get_editor_property('lights_on'));zone.set_actor_location(origin+unreal.Vector(3000,0,0),False,True);advance()
  elif stage==6 and now-mark>2:
   check('parked_bike_still_turns_lights_off',not bike.get_editor_property('lights_on'));rider=unreal.GameplayStatics.get_player_pawn(w,0);check('remount_after_light_test',rider.mount_bike());zone.destroy_actor();reset()
   for d,scale in [(unreal.Vector(145,0,0),unreal.Vector(.3,4,3)),(unreal.Vector(-145,0,0),unreal.Vector(.3,4,3)),(unreal.Vector(0,145,0),unreal.Vector(4,.3,3)),(unreal.Vector(0,-145,0),unreal.Vector(4,.3,3))]:walls.append(unreal.PiedmontWorldTools.spawn_validation_obstacle(w,origin+d,scale))
   check('blocked_exits_keep_player_safely_on_bike',not bike.dismount() and unreal.GameplayStatics.get_player_pawn(w,0)==bike and not bike.get_editor_property('parked'))
   for wall in walls:wall.destroy_actor()
   fixture=unreal.PiedmontWorldTools.spawn_validation_obstacle(w,unreal.Vector(-3000,4000,12.5),unreal.Vector(2,12,.25));reset(1600);bike.validation_key('W',True);rows=[];advance()
  elif stage==7:
   p=bike.get_actor_location();r=bike.get_actor_rotation();rows.append((p.x,p.z,r.roll,r.pitch,bike.ride.get_editor_property('wipeouts')))
   if now-mark>1.7:
    check('full_speed_25cm_curb_stays_upright_and_crosses',p.x>-2400 and max(v[1] for v in rows)>120 and all(abs(v[2])<.2 and abs(v[3])<.2 and v[4]==0 for v in rows),height=max(v[1] for v in rows),end_x=p.x)
    fixture.destroy_actor();reset(900);walker=director.spawn_visitor_for_validation(origin+unreal.Vector(800,50,-97),False)
    if not walker:raise RuntimeError('Glancing-contact target did not spawn')
    walker.get_controller().stop_movement();walker.get_component_by_class(unreal.CharacterMovementComponent).disable_movement();walker.set_actor_tick_enabled(False);advance()
  elif stage==8 and now-mark>1.5:
   check('glancing_contact_nudges_without_wipeout',walker.get_editor_property('bike_contacts')>=1 and bike.ride.get_editor_property('wipeouts')==0 and bike.get_actor_location().x>-3000,contacts=walker.get_editor_property('bike_contacts'),wipeouts=bike.ride.get_editor_property('wipeouts'),end_x=bike.get_actor_location().x)
   walker.destroy_actor();bike.set_actor_location_and_rotation(unreal.Vector(3000,-6500,100),unreal.Rotator(),False,True);bike.ride.set_editor_property('speed',1600);advance()
  elif stage==9:
   lit_seen=lit_seen or bike.get_editor_property('lights_on')
   if now-mark<3.8:return
   check('ride_through_saved_shelter_switches_lights_on_then_off',lit_seen and not bike.get_editor_property('lights_on') and bike.get_actor_location().x>5000 and bike.ride.get_editor_property('wipeouts')==0)
   bike.validation_key('W',False);report['status']='passed' if all(c['pass'] for c in report['cases']) else 'failed';unreal.unregister_slate_post_tick_callback(handle);level.editor_request_end_play();stage=10
  report['stage']=stage;(root/'Scripts/battle-ride-details-validation.json').write_text(json.dumps(report,indent=2))
 except Exception:
  report['status']='error';report['error']=traceback.format_exc();(root/'Scripts/battle-ride-details-validation.json').write_text(json.dumps(report,indent=2));unreal.unregister_slate_post_tick_callback(handle);level.editor_request_end_play()
handle=unreal.register_slate_post_tick_callback(tick);level.editor_request_begin_play()
