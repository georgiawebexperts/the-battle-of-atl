"""Actual lake: ride into water, swim away, return to bank and remount the unmoved bike."""
import unreal,json,pathlib,time,traceback,math
root=pathlib.Path(unreal.Paths.project_dir());level=unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
points=json.loads((root/'SourceAssets/Terrain/lake-test-points.json').read_text())
report={'status':'running','cases':[]};stage=0;elapsed=0.;last=None;bike=None;person=None;parked=None;swim_start=None;started=time.monotonic();stroke=[]
def v(a):
 p=a.get_actor_location();return [p.x,p.y,p.z]
def tick(delta):
 global stage,elapsed,last,bike,person,parked,swim_start,handle
 try:
  if time.monotonic()-started>160:raise RuntimeError('Swimming test timed out')
  worlds=unreal.EditorLevelLibrary.get_pie_worlds(False)
  if not worlds:return
  w=worlds[0];pawn=unreal.GameplayStatics.get_player_pawn(w,0);now=unreal.GameplayStatics.get_time_seconds(w)
  if last is None:last=now
  elapsed+=min(now-last,.12);last=now
  def record(name,okay,**kw):report['cases'].append({'name':name,'pass':bool(okay),**kw})
  def finish():
   report['status']='passed' if all(r['pass'] for r in report['cases']) else 'failed';unreal.unregister_slate_post_tick_callback(handle);level.editor_request_end_play()
  if stage==2 and person:
   hand=person.get_editor_property('body').get_bone_location_by_name('Hand_L',unreal.BoneSpaces.COMPONENT_SPACE);stroke.append(hand.z)
  if stage==0:
   if elapsed<2:return
   bike=pawn;bike.reset_ride();bike.set_actor_location_and_rotation(unreal.Vector(*points['shore_start']),unreal.Rotator(yaw=points['shore_yaw']),False,True);bike.ride.set_editor_property('speed',300);bike.validation_key('W',True);stage=1;elapsed=0
  elif stage==1 and ((isinstance(pawn,unreal.PiedmontExplorer) and elapsed>.5) or elapsed>6):
   record('water_crash_separates_rider',isinstance(pawn,unreal.PiedmontExplorer) and bike.get_editor_property('dismounted'),bike=v(bike),reason=str(bike.ride.get_editor_property('last_crash')),recovery=bike.ride.get_editor_property('recovery'),speed=bike.ride.get_editor_property('speed'))
   if not isinstance(pawn,unreal.PiedmontExplorer):raise RuntimeError('Lake entry did not dismount rider')
   person=pawn;parked=v(bike);swim_start=v(person);person.validation_key('W',True);stage=2;elapsed=0
  elif stage==2 and elapsed>2:
   record('swim_away_with_bike_parked',person.get_editor_property('swimming') and math.dist(swim_start,v(person))>180 and math.dist(parked,v(bike))<1,swimmer=v(person),bike=v(bike))
   record('swim_stroke_moves_arms',bool(stroke) and max(stroke)-min(stroke)>15,hand_travel_cm=max(stroke)-min(stroke) if stroke else 0)
   record('cannot_remount_while_swimming',not person.remount())
   person.validation_key('W',False);stage=3;elapsed=0
   if globals().get('WORLD_JOB',{}).get('preview'):
    report['status']='preview';unreal.unregister_slate_post_tick_callback(handle)
  elif stage==3:
   b=bike.get_actor_location();a=person.get_actor_location()
   if person.get_editor_property('swimming'):
    angle=math.radians(points['shore_yaw']);b=unreal.Vector(b.x-math.cos(angle)*220-math.sin(angle)*180,b.y-math.sin(angle)*220+math.cos(angle)*180,b.z)
   direction=unreal.Rotator(yaw=math.degrees(math.atan2(b.y-a.y,b.x-a.x)))
   unreal.GameplayStatics.get_player_controller(w,0).set_control_rotation(direction);person.validation_key('W',True)
   if math.dist(v(person)[:2],parked[:2])<145 and not person.get_editor_property('swimming'):
    person.validation_key('W',False);record('swim_to_bank_and_walk_back',True,position=v(person));record('bike_never_teleported',math.dist(parked,v(bike))<1);record('remount_after_swimming',person.remount());finish();stage=4
   elif elapsed>12:
    person.validation_key('W',False);record('swim_to_bank_and_walk_back',False,position=v(person),swimming=person.get_editor_property('swimming'),bike=parked);finish();stage=4
  report['stage']=stage;(root/'Scripts/swimming-validation.json').write_text(json.dumps(report,indent=2))
 except Exception:
  report['status']='error';report['error']=traceback.format_exc();(root/'Scripts/swimming-validation.json').write_text(json.dumps(report,indent=2));unreal.unregister_slate_post_tick_callback(handle);level.editor_request_end_play()
handle=unreal.register_slate_post_tick_callback(tick);level.editor_request_begin_play()
