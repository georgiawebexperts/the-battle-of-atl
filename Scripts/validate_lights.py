import unreal,json,pathlib,time,traceback
root=pathlib.Path(unreal.Paths.project_dir());level=unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
report={'status':'running','cases':[]};stage=0;elapsed=0.;last=None;bike=None;zone=None;person=None;started=time.monotonic()
def tick(delta):
 global stage,elapsed,last,bike,zone,person,handle
 try:
  if time.monotonic()-started>100:raise RuntimeError('Lighting integration timeout')
  worlds=unreal.EditorLevelLibrary.get_pie_worlds(False)
  if not worlds:return
  w=worlds[0];mode=unreal.GameplayStatics.get_game_mode(w)
  if mode.get_editor_property('start_countdown')>0:return
  now=unreal.GameplayStatics.get_time_seconds(w)
  if last is None:last=now
  elapsed+=now-last;last=now
  if elapsed<(.5 if stage not in [3,5] else 2):return
  elapsed=0;pawn=unreal.GameplayStatics.get_player_pawn(w,0)
  def record(n,v):report['cases'].append({'name':n,'pass':bool(v)})
  if stage==0:
   bike=pawn;bike.set_actor_location_and_rotation(unreal.Vector(0,4000,100),unreal.Rotator(),False,True);bike.ride.set_editor_property('speed',0);record('daylight_lights_off',not bike.get_editor_property('lights_on'));bike.validation_key('H',True);bike.horn();record('horn_repeat_is_limited',bike.get_editor_property('horn_count')==1)
  elif stage==1:
   bike.validation_key('H',False);record('H_sounds_horn',bike.get_editor_property('horn_count')==1)
   zone=unreal.PiedmontWorldTools.spawn_validation_dark_zone(w,bike.get_actor_location())
   if not zone:raise RuntimeError('Could not create dark-zone fixture')
  elif stage==2:
   record('dark_zone_turns_head_and_tail_lights_on',bike.get_editor_property('lights_on') and bike.get_editor_property('headlight').is_visible() and bike.get_editor_property('tail_light').is_visible());zone.set_actor_location(unreal.Vector(2000,4000,100),False,True)
  elif stage==3:
   record('leaving_darkness_turns_lights_off_after_delay',not bike.get_editor_property('lights_on'));zone.set_actor_location(bike.get_actor_location(),False,True)
  elif stage==4:
   record('reentering_darkness_restores_lights',bike.get_editor_property('lights_on'));record('dismount_for_parked_light_test',bike.dismount());zone.set_actor_location(unreal.Vector(2000,4000,100),False,True)
  elif stage==5:
   record('parked_bike_updates_automatic_lights',bike.get_editor_property('dismounted') and not bike.get_editor_property('lights_on'));report['status']='passed' if all(r['pass'] for r in report['cases']) else 'failed';unreal.unregister_slate_post_tick_callback(handle);level.editor_request_end_play()
  stage+=1;report['stage']=stage;(root/'Scripts/lights-validation.json').write_text(json.dumps(report,indent=2))
 except Exception:
  report['status']='error';report['error']=traceback.format_exc();(root/'Scripts/lights-validation.json').write_text(json.dumps(report,indent=2));unreal.unregister_slate_post_tick_callback(handle);level.editor_request_end_play()
handle=unreal.register_slate_post_tick_callback(tick);level.editor_request_begin_play()
