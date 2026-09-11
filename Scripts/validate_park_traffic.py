"""Live park population, walking, horn response, and swept bike contacts."""
import unreal,json,pathlib,time,math,traceback
root=pathlib.Path(unreal.Paths.project_dir());level=unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
report={'status':'running','scope':'Initial local walker/jogger population and controlled real-park interactions; not full-world density or visual acceptance.','cases':[]};stage=0;elapsed=0.;last_time=None;started=time.monotonic();bike=None;director=None;first={};visitor=None;jogger=None;horn_start=None
lane=json.loads((root/'SourceAssets/Terrain/traffic-test-lane.json').read_text());origin=unreal.Vector(*lane['start']);direction=unreal.Vector(*lane['direction']);yaw=math.degrees(math.atan2(direction.y,direction.x))
def tick(dt):
 global stage,elapsed,last_time,bike,director,first,visitor,jogger,horn_start,handle
 try:
  if time.monotonic()-started>150:raise RuntimeError('Traffic integration timeout')
  worlds=unreal.EditorLevelLibrary.get_pie_worlds(False)
  if not worlds:return
  w=worlds[0];mode=unreal.GameplayStatics.get_game_mode(w)
  if mode.get_editor_property('start_countdown')>0:return
  now=unreal.GameplayStatics.get_time_seconds(w)
  if last_time is None:last_time=now
  elapsed+=min(now-last_time,.12);last_time=now
  def visitors():return unreal.GameplayStatics.get_all_actors_of_class(w,unreal.PiedmontPedestrian)
  def record(name,ok,**kw):report['cases'].append({'name':name,'pass':bool(ok),**kw})
  def advance():
   global stage,elapsed
   stage+=1;elapsed=0
  def reset_bike(speed=0):
   bike.set_actor_tick_enabled(True);bike.set_actor_location_and_rotation(origin+unreal.Vector(0,0,98),unreal.Rotator(yaw=yaw),False,True)
   for prop,value in [('speed',speed),('recovery',0),('crashes',0),('last_crash','')]:bike.ride.set_editor_property(prop,value)
  def obstacle(distance):
   v=director.spawn_visitor_for_validation(origin+direction*distance,False)
   if not v:raise RuntimeError('Cannot spawn controlled visitor')
   v.get_controller().stop_movement();v.get_component_by_class(unreal.CharacterMovementComponent).disable_movement();v.set_actor_tick_enabled(False);return v
  if stage==0:
   bike=unreal.GameplayStatics.get_player_pawn(w,0);bike.ride.set_editor_property('speed',0);bike.set_actor_tick_enabled(False)
   ds=unreal.GameplayStatics.get_all_actors_of_class(w,unreal.PiedmontTrafficDirector)
   if not ds:raise RuntimeError('Traffic director was not created')
   director=ds[0];first={v.get_name():v.get_actor_location() for v in visitors()};advance()
  elif stage==1 and elapsed>7:
   people=visitors();walkers=[v for v in people if 'WALKER' in str(v.get_editor_property('kind'))];joggers=[v for v in people if 'JOGGER' in str(v.get_editor_property('kind'))]
   moved=sum(v.get_name() in first and (v.get_actor_location()-first[v.get_name()]).length()>200 for v in people)
   record('local_population_contains_walkers_and_joggers',len(people)>=16 and len(walkers)>0 and len(joggers)>0,count=len(people),walkers=len(walkers),joggers=len(joggers))
   record('walkers_form_pairs',any(v.get_editor_property('group_leader') for v in walkers))
   record('visitors_use_ai_controllers_and_move',all(isinstance(v.get_controller(),unreal.AIController) for v in people) and moved>=6,moved=moved)
   record('population_stays_out_of_water',all(not v.get_editor_property('swimming') for v in people))
   director.set_actor_tick_enabled(False)
   for v in people:v.destroy_actor()
   reset_bike();bike.set_actor_tick_enabled(False);advance()
  elif stage==2 and elapsed>.3:
   visitor=director.spawn_visitor_for_validation(origin+direction*500,False);jogger=director.spawn_visitor_for_validation(origin+direction*950,True)
   if not visitor or not jogger:raise RuntimeError('Horn test visitor spawn failed')
   visitor.set_editor_property('pause_remaining',5.0);horn_start=visitor.get_actor_location();bike.horn();advance()
  elif stage==3 and elapsed>1.3:
   moved=(visitor.get_actor_location()-horn_start).length();record('walker_yields_to_actual_bike_horn',visitor.get_editor_property('horn_reactions')==1 and moved>25,moved_cm=moved)
   record('jogger_does_not_yield_to_horn',jogger.get_editor_property('horn_reactions')==0)
   visitor.destroy_actor();jogger.destroy_actor();reset_bike(200);visitor=obstacle(250);advance()
  elif stage==4:
   if visitor.get_editor_property('bike_contacts')>0 or elapsed>4:
    record('low_speed_contact_stumbles_and_slows',visitor.get_editor_property('bike_contacts')==1 and visitor.get_editor_property('stumble_remaining')>0 and bike.ride.get_editor_property('speed')<120 and bike.ride.get_editor_property('crashes')==0,speed=bike.ride.get_editor_property('speed'),contacts=visitor.get_editor_property('bike_contacts'))
    visitor.destroy_actor();reset_bike(650);visitor=obstacle(500);advance()
  elif stage==5:
   if bike.ride.get_editor_property('crashes')>0 or elapsed>4:
    record('high_speed_contact_crashes_once',bike.ride.get_editor_property('crashes')==1 and visitor.get_editor_property('bike_contacts')==1,reason=bike.ride.get_editor_property('last_crash'))
    record('traffic_crash_has_four_second_recovery',3.7<=bike.ride.get_editor_property('recovery')<=4.0 and bike.ride.get_editor_property('speed')==0,recovery=bike.ride.get_editor_property('recovery'))
    visitor.destroy_actor();report['status']='passed' if all(r['pass'] for r in report['cases']) else 'failed';advance();unreal.unregister_slate_post_tick_callback(handle);level.editor_request_end_play()
  report['stage']=stage;(root/'Scripts/park-traffic-validation.json').write_text(json.dumps(report,indent=2))
 except Exception:
  report['status']='error';report['error']=traceback.format_exc();(root/'Scripts/park-traffic-validation.json').write_text(json.dumps(report,indent=2));unreal.unregister_slate_post_tick_callback(handle);level.editor_request_end_play()
handle=unreal.register_slate_post_tick_callback(tick);level.editor_request_begin_play()
