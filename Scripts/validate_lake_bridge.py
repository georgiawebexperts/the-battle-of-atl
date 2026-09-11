"""Ride the installed lake deck both directions and dismount above water."""
import unreal,json,pathlib,time,traceback,math
p=pathlib.Path(unreal.Paths.project_dir());job=globals().get('WORLD_JOB',{});collection=job.get('collection','lake');wet=collection!='lake';data=json.loads((p/('SourceAssets/Terrain/'+{'wetlands':'WetlandBridges','parkdrive':'ParkDriveBridge'}.get(collection,'LakeBridge')+'/manifest.json')).read_text());points=data['spur_cm'] if job.get('spur') else data['test_routes'][job['route_name']] if job.get('route_name') else next(b['centerline_cm'] for b in data['bridges'] if b['osm_id']==job['osm_id']) if wet else data['centerline_cm'];report_file=p/(('Scripts/bridge-'+str(job.get('route_name',job.get('osm_id')))+'-validation.json') if wet else ('Scripts/lake-spur-validation.json' if job.get('spur') else 'Scripts/lake-bridge-validation.json'));level=unreal.get_editor_subsystem(unreal.LevelEditorSubsystem);report={'status':'running','cases':[]};stage=0;elapsed=0.;last=None;bike=None;start=None;origin=None;started=time.monotonic()
# Include the bank approaches so reaching a deck end is not mistaken for a full crossing.
for front in ([] if job.get('route_name') or job.get('spur') else [True,False]):
 a,b=(points[0],points[1]) if front else (points[-1],points[-2]);dx=a[0]-b[0];dy=a[1]-b[1];length=math.hypot(dx,dy);v=[a[0]+dx/length*150,a[1]+dy/length*150,a[2]]
 if front:points.insert(0,v)
 else:points.append(v)
def tick(delta):
 global stage,elapsed,last,bike,start,origin,handle
 try:
  if time.monotonic()-started>180:raise RuntimeError('Bridge validation timeout')
  worlds=unreal.EditorLevelLibrary.get_pie_worlds(False)
  if not worlds:return
  w=worlds[0];mode=unreal.GameplayStatics.get_game_mode(w)
  if mode.get_editor_property('start_countdown')>0:return
  now=unreal.GameplayStatics.get_time_seconds(w)
  if last is None:last=now
  elapsed+=min(now-last,.12);last=now
  pawn=unreal.GameplayStatics.get_player_pawn(w,0)
  def record(name,ok,**extra):report['cases'].append({'name':name,'pass':bool(ok),**extra})
  def begin(reverse):
   global start,origin,elapsed
   bike.reset_ride();bike.ride.set_editor_property('recovery',0);bike.ride.set_editor_property('crashes',0);bike.ride.set_editor_property('last_crash','');bike.validation_key('Left',False);bike.validation_key('Right',False)
   a=points[-1] if reverse else points[0];b=points[-2] if reverse else points[1];yaw=math.degrees(math.atan2(b[1]-a[1],b[0]-a[0]));origin=unreal.Vector(*a);bike.set_actor_location_and_rotation(origin+unreal.Vector(0,0,98),unreal.Rotator(yaw=yaw),False,True);bike.ride.set_editor_property('speed',260);bike.ride.set_editor_property('gear',1);bike.validation_key('W',True);start=now;elapsed=0
  if stage==0:
   bike=pawn
   if globals().get('WORLD_JOB',{}).get('preview'):stage=4
   else:begin(False);stage=1
  elif stage in [1,3]:
   if pawn!=bike:raise RuntimeError('Bridge wrongly triggered water entry')
   # Follow the original curved centerline with actual steering input.
   loc=bike.get_actor_location();nearest=min(range(len(points)),key=lambda i:(points[i][0]-loc.x)**2+(points[i][1]-loc.y)**2);target=points[max(0,nearest-4) if stage==3 else min(len(points)-1,nearest+4)];wanted=math.degrees(math.atan2(target[1]-loc.y,target[0]-loc.x));error=(wanted-bike.get_actor_rotation().yaw+180)%360-180;bike.validation_key('Right',error>3);bike.validation_key('Left',error<-3)
   end=points[0] if stage==3 else points[-1];distance=math.hypot(loc.x-end[0],loc.y-end[1])
   if distance<job.get('end_tolerance_cm',100) or elapsed>12 or bike.ride.get_editor_property('crashes')>0:
    bike.validation_key('W',False);record('ride_reverse' if stage==3 else 'ride_forward',distance<job.get('end_tolerance_cm',100) and bike.ride.get_editor_property('crashes')==0 and not bike.get_editor_property('dismounted'),remaining_cm=distance,position=[loc.x,loc.y,loc.z],reason=str(bike.ride.get_editor_property('last_crash')));stage+=1;elapsed=0
  elif stage==2:begin(True);stage=3
  elif stage==4:
   bike.reset_ride();bike.ride.set_editor_property('recovery',0);bike.ride.set_editor_property('crashes',0);bike.ride.set_editor_property('last_crash','');middle=points[len(points)//2];bike.ride.set_editor_property('speed',0);bike.validation_key('Right',False);bike.validation_key('Left',False);bike.set_actor_location_and_rotation(unreal.Vector(*middle)+unreal.Vector(0,0,98),unreal.Rotator(yaw=math.degrees(math.atan2(points[-1][1]-points[0][1],points[-1][0]-points[0][0]))),False,True);stage=5;elapsed=0
  elif stage==5 and elapsed>.7:
   record('dismount_on_deck_above_water',bike.dismount());stage=6;elapsed=0
  elif stage==6 and elapsed>.5:
   record('walking_on_bridge_is_not_swimming',isinstance(pawn,unreal.PiedmontExplorer) and not pawn.get_editor_property('swimming'))
   record('remount_on_bridge',isinstance(pawn,unreal.PiedmontExplorer) and pawn.remount());report['status']='passed' if all(x['pass'] for x in report['cases']) else 'failed';unreal.unregister_slate_post_tick_callback(handle)
   if not globals().get('WORLD_JOB',{}).get('preview'):level.editor_request_end_play()
   else:report['status']='preview'
   stage=7
  report['stage']=stage;report_file.write_text(json.dumps(report,indent=2))
 except Exception:
  report['status']='error';report['error']=traceback.format_exc();report_file.write_text(json.dumps(report,indent=2));unreal.unregister_slate_post_tick_callback(handle);level.editor_request_end_play()
handle=unreal.register_slate_post_tick_callback(tick);level.editor_request_begin_play()
