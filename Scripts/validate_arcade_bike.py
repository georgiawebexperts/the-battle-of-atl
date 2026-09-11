"""Live V3 Character Movement handling checks; does not accept the full FPS milestone."""
import unreal,json,pathlib,time,math,traceback
root=pathlib.Path(unreal.Paths.project_dir());level=unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
report={'status':'running','scope':'V3 terrain/handling foundation only; other gameplay is tested separately and full fun acceptance remains pending.','cases':[]};stage=0;case_index=-1;case_start=0.;started=time.monotonic();bike=None;director=None;victim=None;rows=[];recovery_start=None;gear_step=0
cases=[('hill',( -1600,-2500,100),3.1,[]),('stairs',(-1700,-4500,100),3.2,[]),('roots',(-1500,-6500,100),2.3,[]),('asphalt',(-4000,0,100),2,[]),('grass',(-4000,9000,90),2,[]),('wall',(5000,0,100),2,[]),('sharp_turn',(-4000,4000,100),2,['Right']),('drift',(-4000,4000,100),1,['Right','SpaceBar'])]
def tick(dt):
 global stage,case_index,case_start,bike,director,victim,rows,handle,recovery_start,gear_step
 try:
  if time.monotonic()-started>220:raise RuntimeError('Arcade handling test exceeded runtime budget')
  worlds=unreal.EditorLevelLibrary.get_pie_worlds(False)
  if not worlds:return
  w=worlds[0];mode=unreal.GameplayStatics.get_game_mode(w)
  if mode.get_editor_property('start_countdown')>0:return
  now=unreal.GameplayStatics.get_time_seconds(w)
  def record(n,ok,**details):report['cases'].append({'name':n,'pass':bool(ok),**details})
  def reset(pos,extra=()):
   for key in ['Right','Left','SpaceBar']:
    if key not in extra:bike.validation_key(key,False)
   bike.set_actor_location_and_rotation(unreal.Vector(*pos),unreal.Rotator(),False,True);bike.ride.stop_movement_immediately()
   for k,v in [('gear',5),('speed',1600.),('recovery',0.),('wipeouts',0),('recovery_reason','')]:bike.ride.set_editor_property(k,v)
   bike.validation_key('W',True)
   for key in extra:bike.validation_key(key,True)
  def sample():
   p=bike.get_actor_location();r=bike.get_actor_rotation();return {'x':p.x,'y':p.y,'z':p.z,'pitch':r.pitch,'roll':r.roll,'speed':bike.ride.get_editor_property('speed'),'recovery':bike.ride.get_editor_property('recovery'),'wipeouts':bike.ride.get_editor_property('wipeouts'),'grass':bike.ride.get_editor_property('grass'),'slide':bike.ride.get_editor_property('slide_remaining'),'visual_roll':bike.get_editor_property('visual').get_editor_property('relative_rotation').roll}
  if stage==0:
   bike=unreal.GameplayStatics.get_player_pawn(w,0)
   if not isinstance(bike,unreal.BattleBike):raise RuntimeError('Wrong pawn: V3 BattleBike required')
   director=unreal.GameplayStatics.get_all_actors_of_class(w,unreal.PiedmontTrafficDirector)[0];case_start=now;stage=1
  elif stage==1 and now-case_start>6:
   people=unreal.GameplayStatics.get_all_actors_of_class(w,unreal.PiedmontPedestrian)
   record('fifty_wandering_visitors',len(people)>=50 and sum(v.get_velocity().length()>5 for v in people)>25,count=len(people))
   record('uses_character_movement',isinstance(bike.ride,unreal.CharacterMovementComponent))
   gear_step=0
   director.set_actor_tick_enabled(False)
   for p in people:p.destroy_actor()
   stage=10
  elif stage==10:
   if gear_step<18:bike.validation_key('Up',gear_step%2==0)
   elif gear_step==18:record('five_gear_upper_limit',bike.ride.get_editor_property('gear')==5)
   elif gear_step<37:bike.validation_key('Down',gear_step%2==1)
   else:record('first_gear_lower_limit',bike.ride.get_editor_property('gear')==1);stage=2
   gear_step+=1
  elif stage==2:
   if case_index>=0:
    rows.append(sample());name,pos,duration,keys=cases[case_index]
    if now-case_start<duration:return
    upright=all(abs(r['pitch'])<.2 and abs(r['roll'])<.2 and r['wipeouts']==0 for r in rows);travel=rows[-1]['x']-pos[0];height=max(r['z'] for r in rows)
    if name=='hill':ok=upright and travel>3300 and height>350
    elif name=='stairs':ok=upright and travel>3400 and height>190
    elif name=='roots':ok=upright and travel>2500 and height>110
    elif name=='wall':ok=upright and max(r['x'] for r in rows)<6510 and any(r['speed']<100 for r in rows)
    elif name=='grass':ok=upright and all(r['grass'] for r in rows[-3:]) and 1150<rows[-1]['speed']<=1200.1
    elif name=='asphalt':ok=upright and rows[-1]['speed']>1550
    elif name=='sharp_turn':ok=upright and max(abs(r['visual_roll']) for r in rows)>15
    else:ok=upright and any(r['slide']>0 for r in rows)
    record(name,ok,travel_cm=travel,max_height=height,last=rows[-1])
   case_index+=1;rows=[]
   if case_index<len(cases):
    reset(cases[case_index][1],cases[case_index][3]);case_start=now
   else:
    reset((8500,-9000,100));recovery_start=None;case_start=now;stage=3
  elif stage==3:
   recovery=bike.ride.get_editor_property('recovery')
   if recovery>0 and recovery_start is None:recovery_start=now
   if recovery_start is not None and recovery<=0:
    p=bike.get_actor_location();record('water_returns_to_nearest_path_in_two_seconds',1.7<now-recovery_start<2.35 and abs(p.y+6500)<200 and abs(p.x-8500)<250,elapsed=now-recovery_start,location=[p.x,p.y,p.z]);reset((-4000,4000,100));victim=director.spawn_visitor_for_validation(unreal.Vector(-3000,4000,3),False)
    if not victim:raise RuntimeError('Traffic impact test spawn failed')
    victim.get_controller().stop_movement();victim.get_component_by_class(unreal.CharacterMovementComponent).disable_movement();victim.set_actor_tick_enabled(False);case_start=now;recovery_start=None;stage=4
   elif now-case_start>5:raise RuntimeError('Water recovery did not complete')
  elif stage==4:
   recovery=bike.ride.get_editor_property('recovery')
   if recovery>0 and recovery_start is None:recovery_start=now;victim.destroy_actor()
   if recovery_start is not None and recovery<=0:
    record('direct_traffic_hit_has_two_second_wipeout',1.7<now-recovery_start<2.35 and bike.ride.get_editor_property('wipeouts')==1,elapsed=now-recovery_start)
    for key in ['W','Right','Left','SpaceBar']:bike.validation_key(key,False)
    report['status']='passed' if all(c['pass'] for c in report['cases']) else 'failed';stage=5;unreal.unregister_slate_post_tick_callback(handle);level.editor_request_end_play()
   elif now-case_start>6:raise RuntimeError('Direct hit failed to wipe out/recover')
  report['stage']=stage;report['active']=cases[case_index][0] if 0<=case_index<len(cases) else 'recovery';(root/'Scripts/arcade-bike-validation.json').write_text(json.dumps(report,indent=2))
 except Exception:
  report['status']='error';report['error']=traceback.format_exc();(root/'Scripts/arcade-bike-validation.json').write_text(json.dumps(report,indent=2));unreal.unregister_slate_post_tick_callback(handle);level.editor_request_end_play()
unreal.get_default_object(unreal.load_class(None,'/Script/UnrealEd.LevelEditorPlaySettings')).set_editor_property('GameGetsMouseControl',True)
ew=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
report['editor_nav_actors']=[a.get_name() for a in unreal.GameplayStatics.get_all_actors_of_class(ew,unreal.RecastNavMesh)]
handle=unreal.register_slate_post_tick_callback(tick);level.editor_request_begin_play()
