"""PIE checks for timer, coarse compass, on-foot fire permissions, cover and fatal restart."""
import unreal,json,pathlib,time,traceback,math
root=pathlib.Path(unreal.Paths.project_dir());level=unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
report={'status':'running','cases':[]};stage=0;elapsed=0.;last=None;bike=None;person=None;timer=0.;old_item=None;run=0;started=time.monotonic()
def xyz(v):return [v.x,v.y,v.z]
def tick(delta):
 global stage,elapsed,last,bike,person,timer,old_item,run,handle
 try:
  if time.monotonic()-started>180:raise RuntimeError('Combat rules test timed out')
  worlds=unreal.EditorLevelLibrary.get_pie_worlds(False)
  if not worlds:return
  w=worlds[0];mode=unreal.GameplayStatics.get_game_mode(w);pc=unreal.GameplayStatics.get_player_controller(w,0);pawn=unreal.GameplayStatics.get_player_pawn(w,0)
  if mode.get_editor_property('start_countdown')>0:return
  now=unreal.GameplayStatics.get_time_seconds(w)
  if last is None:last=now
  elapsed+=min(now-last,.12);last=now
  if elapsed<(.5 if stage!=3 else 2):return
  elapsed=0
  def record(name,okay,**kw):report['cases'].append({'name':name,'pass':bool(okay),**kw})
  if stage==0:
   bike=pawn;bike.set_actor_location_and_rotation(unreal.Vector(0,4000,100),unreal.Rotator(),False,True);bike.ride.set_editor_property('speed',0);timer=mode.get_editor_property('time_remaining');bike.validation_key('E',True)
  elif stage==1:
   if not isinstance(pawn,unreal.PiedmontExplorer):raise RuntimeError('No walking rider')
   person=pawn;person.validation_key('E',False);record('timer_continues_through_dismount',mode.get_editor_property('time_remaining')<timer);timer=mode.get_editor_property('time_remaining');person.validation_key('One',True)
  elif stage==2:
   person.validation_key('One',False);record('draw_on_foot',person.get_editor_property('weapon_drawn'))
   if globals().get('WORLD_JOB',{}).get('preview'):
    report['status']='preview';unreal.unregister_slate_post_tick_callback(handle);(root/'Scripts/combat-rules-validation.json').write_text(json.dumps(report,indent=2));return
   ammo=person.get_editor_property('ammo');record('shot_consumes_one_round',person.fire() and person.get_editor_property('ammo')==ammo-1);record('cooldown_rejects_immediate_second_shot',not person.fire());person.reload()
  elif stage==3:
   record('reload_completes_while_clock_runs',person.get_editor_property('ammo')==12 and mode.get_editor_property('time_remaining')<timer-1)
   loc=person.get_actor_location();wall=unreal.PiedmontWorldTools.spawn_validation_obstacle(w,loc+unreal.Vector(45,0,0),unreal.Vector(.1,4,4));pc.set_control_rotation(unreal.Rotator())
   fired=person.fire();end=person.get_editor_property('last_shot_end');record('muzzle_cannot_fire_through_close_cover',fired and end.x<=loc.x+51,hit=xyz(end));wall.destroy_actor()
   person.set_editor_property('swimming',True);record('swimming_rejects_fire',not person.fire());person.set_editor_property('swimming',False)
   person.set_editor_property('dead',True);record('dead_rider_rejects_fire',not person.fire());person.set_editor_property('dead',False)
   record('remount_after_weapon_use',person.remount())
  elif stage==4:
   record('mounted_rider_has_no_weapon_pawn',pawn==bike)
   unreal.GameplayStatics.apply_damage(bike,1,pc,None,unreal.PiedmontKnifeDamage)
  elif stage==5:
   person=unreal.GameplayStatics.get_player_pawn(w,0);record('first_stab_forces_dismount_without_death',isinstance(person,unreal.PiedmontExplorer) and person.get_editor_property('knife_wounded') and not mode.get_editor_property('run_ended'))
   if not isinstance(person,unreal.PiedmontExplorer):raise RuntimeError('Knife dismount failed')
   mode.set_editor_property('item_collected',True);old_item=xyz(mode.get_editor_property('item_location'));run=mode.get_editor_property('run_number');timer=mode.get_editor_property('time_remaining');unreal.GameplayStatics.apply_damage(person,1,pc,None,unreal.PiedmontKnifeDamage)
  elif stage==6:
   record('second_stab_ends_run_and_discards_item',mode.get_editor_property('run_ended') and not mode.get_editor_property('item_collected'));timer=mode.get_editor_property('time_remaining')
  elif stage==7:
   record('ended_run_stops_clock',mode.get_editor_property('time_remaining')==timer);mode.restart_run()
  elif stage==8:
   record('restart_returns_bike_and_resets_search',pawn==bike and not mode.get_editor_property('item_collected') and mode.get_editor_property('time_remaining')>590 and mode.get_editor_property('run_number')==run+1 and xyz(mode.get_editor_property('item_location'))!=old_item)
   original=mode.get_editor_property('item_location');here=bike.get_actor_location();directions=[]
   for label,d in [('NORTH',(0,5000,0)),('EAST',(5000,0,0)),('SOUTH',(0,-5000,0)),('WEST',(-5000,0,0))]:
    mode.set_editor_property('item_location',here+unreal.Vector(*d));directions.append(mode.item_direction()=='SEARCH '+label)
   record('compass_shows_only_four_cardinal_hints',all(directions));mode.set_editor_property('item_location',original)
   bike.set_actor_location_and_rotation(original+unreal.Vector(0,0,55),unreal.Rotator(),False,True)
  elif stage==9:
   record('physical_item_can_be_collected',mode.get_editor_property('item_collected'));unreal.GameplayStatics.apply_damage(bike,1,pc,None,unreal.PiedmontBulletDamage)
  elif stage==10:
   record('single_bullet_ends_run_and_clears_collected_item',mode.get_editor_property('run_ended') and not mode.get_editor_property('item_collected'));report['status']='passed' if all(r['pass'] for r in report['cases']) else 'failed';unreal.unregister_slate_post_tick_callback(handle);level.editor_request_end_play()
  stage+=1;report['stage']=stage;(root/'Scripts/combat-rules-validation.json').write_text(json.dumps(report,indent=2))
 except Exception:
  report['status']='error';report['error']=traceback.format_exc();(root/'Scripts/combat-rules-validation.json').write_text(json.dumps(report,indent=2));unreal.unregister_slate_post_tick_callback(handle);level.editor_request_end_play()
handle=unreal.register_slate_post_tick_callback(tick);level.editor_request_begin_play()
