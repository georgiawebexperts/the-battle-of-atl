"""PIE integration: possession, walking, parked-bike persistence and remount guards."""
import unreal,json,pathlib,time,traceback,math
root=pathlib.Path(unreal.Paths.project_dir());level=unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
report={'status':'running','cases':[]};started=time.monotonic();stage=0;elapsed=0.;last=None;bike=None;person=None;origin=None;walk_start=None;blockers=[]

def tick(delta):
 global stage,elapsed,last,bike,person,origin,walk_start,handle
 try:
  if time.monotonic()-started>120:raise RuntimeError('Explorer integration timeout')
  worlds=unreal.EditorLevelLibrary.get_pie_worlds(False)
  if not worlds:return
  w=worlds[0]
  if unreal.GameplayStatics.get_game_mode(w).get_editor_property('start_countdown')>0:return
  pawn=unreal.GameplayStatics.get_player_pawn(w,0);now=unreal.GameplayStatics.get_time_seconds(w)
  if last is None:last=now
  elapsed+=min(now-last,.12);last=now
  if elapsed<.5:return
  elapsed=0
  def vec(v):return [v.x,v.y,v.z]
  def record(name,passed,**extra):report['cases'].append({'name':name,'pass':bool(passed),**extra})
  if stage==0:
   bike=pawn;bike.reset_ride();bike.set_actor_location_and_rotation(unreal.Vector(0,4000,100),unreal.Rotator(),False,True);bike.ride.set_editor_property('speed',0);origin=vec(bike.get_actor_location());bike.validation_key('E',True)
  elif stage==1:
   record('E_transfers_to_walking_rider',isinstance(pawn,unreal.PiedmontExplorer) and bike.get_editor_property('dismounted'))
   if not isinstance(pawn,unreal.PiedmontExplorer):raise RuntimeError('Dismount did not possess explorer')
   person=pawn;person.validation_key('E',False);walk_start=vec(person.get_actor_location());person.validation_key('W',True)
   if globals().get('WORLD_JOB',{}).get('preview'):
    person.validation_key('W',False);report['status']='preview';unreal.unregister_slate_post_tick_callback(handle)
  elif stage==2:
   pass
  elif stage==3:
   person.validation_key('W',False);distance=math.dist(walk_start,vec(person.get_actor_location()));record('walk_and_leave_bike_stationary',distance>150 and math.dist(origin,vec(bike.get_actor_location()))<1,distance_cm=distance)
   record('reject_remote_remount',not person.remount())
   person.set_actor_location_and_rotation(unreal.Vector(0,4105,95),unreal.Rotator(),False,True);person.get_component_by_class(unreal.CharacterMovementComponent).stop_movement_immediately()
  elif stage==4:
   person.validation_key('E',True)
  elif stage==5:
   record('E_remount_restores_bike_possession',pawn==bike and not bike.get_editor_property('dismounted'));bike.validation_key('E',False);bike.validation_key('W',True)
  elif stage==6:
   record('pedal_works_after_remount',bike.ride.get_editor_property('speed')>30,speed= bike.ride.get_editor_property('speed'));bike.validation_key('W',False)
   bike.ride.set_editor_property('speed',600);record('reject_unsafe_fast_dismount',not bike.dismount());bike.ride.set_editor_property('speed',0)
   bike.set_actor_location_and_rotation(unreal.Vector(0,4000,100),unreal.Rotator(),False,True)
   for y in [4105,3895]:
    block=unreal.PiedmontWorldTools.spawn_validation_obstacle(w,unreal.Vector(0,y,100),unreal.Vector(1,1,3))
    if not block:raise RuntimeError('Failed to create isolated PIE blocker')
    blockers.append(block)
  elif stage==7:
   record('reject_blocked_dismount',not bike.dismount())
   for block in blockers:block.destroy_actor()
   for y in [4055,3945]:
    if not unreal.PiedmontWorldTools.spawn_validation_obstacle(w,unreal.Vector(0,y,100),unreal.Vector(1,.1,3)):raise RuntimeError('Failed to create thin barrier')
  elif stage==8:
   record('reject_dismount_through_thin_barrier',not bike.dismount());report['status']='passed' if all(r['pass'] for r in report['cases']) else 'failed';unreal.unregister_slate_post_tick_callback(handle);level.editor_request_end_play()
  stage+=1;report['stage']=stage;(root/'Scripts/explorer-validation.json').write_text(json.dumps(report,indent=2))
 except Exception:
  report['status']='error';report['error']=traceback.format_exc();(root/'Scripts/explorer-validation.json').write_text(json.dumps(report,indent=2));unreal.unregister_slate_post_tick_callback(handle);level.editor_request_end_play()
handle=unreal.register_slate_post_tick_callback(tick);level.editor_request_begin_play()
