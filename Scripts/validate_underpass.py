"""A thin low bridge must not capture ground probes from the path below."""
import unreal,pathlib,json,time,traceback
p=pathlib.Path(unreal.Paths.project_dir());level=unreal.get_editor_subsystem(unreal.LevelEditorSubsystem);report={'status':'running','cases':[]};stage=0;elapsed=0.;last=None;bike=None;deck=None;floor=0.;started=time.monotonic()
def tick(delta):
 global stage,elapsed,last,bike,deck,floor,handle
 try:
  if time.monotonic()-started>100:raise RuntimeError('Underpass timeout')
  worlds=unreal.EditorLevelLibrary.get_pie_worlds(False)
  if not worlds:return
  w=worlds[0];mode=unreal.GameplayStatics.get_game_mode(w)
  if mode.get_editor_property('start_countdown')>0:return
  now=unreal.GameplayStatics.get_time_seconds(w)
  if last is None:last=now
  elapsed+=min(now-last,.12);last=now
  def record(n,ok,**kw):report['cases'].append({'name':n,'pass':bool(ok),**kw})
  if stage==0:
   bike=unreal.GameplayStatics.get_player_pawn(w,0);hit=unreal.PiedmontWorldTools.trace_world_surface(unreal.Vector(0,4000,300),unreal.Vector(0,4000,-300));floor=hit[0].z
   bike.set_actor_location_and_rotation(unreal.Vector(0,4000,floor+96),unreal.Rotator(),False,True);bike.ride.set_editor_property('speed',260);bike.ride.set_editor_property('gear',1)
   deck=unreal.PiedmontWorldTools.spawn_validation_obstacle(w,unreal.Vector(600,4000,floor+194.5),unreal.Vector(6,5,.01));deck.tags=[unreal.Name('RideBridge')];bike.validation_key('W',True);stage=1;elapsed=0
  elif stage==1:
   loc=bike.get_actor_location()
   if loc.x>1050 or elapsed>7:
    bike.validation_key('W',False);record('ride_under_deck_without_climbing_or_crashing',loc.x>1050 and abs(loc.z-floor-96)<5 and bike.ride.get_editor_property('crashes')==0,x=loc.x,z=loc.z,floor=floor,reason=str(bike.ride.get_editor_property('last_crash')));bike.ride.set_editor_property('speed',0);bike.set_actor_location_and_rotation(unreal.Vector(600,4000,floor+195+96),unreal.Rotator(),False,True);stage=2;elapsed=0
  elif stage==2 and elapsed>1:
   loc=bike.get_actor_location();record('same_deck_supports_rider_from_above',abs(loc.z-floor-195-96)<5 and bike.ride.get_editor_property('grounded'),z=loc.z);report['status']='passed' if all(r['pass'] for r in report['cases']) else 'failed';unreal.unregister_slate_post_tick_callback(handle);level.editor_request_end_play();stage=3
  (p/'Scripts/underpass-validation.json').write_text(json.dumps(report,indent=2))
 except Exception:
  report['status']='error';report['error']=traceback.format_exc();(p/'Scripts/underpass-validation.json').write_text(json.dumps(report,indent=2));unreal.unregister_slate_post_tick_callback(handle);level.editor_request_end_play()
handle=unreal.register_slate_post_tick_callback(tick);level.editor_request_begin_play()
