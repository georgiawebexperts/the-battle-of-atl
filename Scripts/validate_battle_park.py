"""Real park V3 mode, movement, water return and all authored bridge routes."""
import unreal,pathlib,json,time,math,traceback
root=pathlib.Path(unreal.Paths.project_dir());level=unreal.get_editor_subsystem(unreal.LevelEditorSubsystem);job=globals().get('WORLD_JOB',{});points=json.loads((root/'SourceAssets/Terrain/lake-test-points.json').read_text());routes=[]
lake=json.loads((root/'SourceAssets/Terrain/LakeBridge/manifest.json').read_text());routes.append(('lake_crossing',lake['centerline_cm'],True));routes.append(('lake_wood_spur',lake['spur_cm'],False))
for folder in ['WetlandBridges','ParkDriveBridge']:
 d=json.loads((root/f'SourceAssets/Terrain/{folder}/manifest.json').read_text())
 for b in d['bridges']:routes.append((str(b['osm_id']),b['centerline_cm'],True))
 if 'test_routes' in d:
  for n,pts in d['test_routes'].items():routes.append((n,pts,False))
if job.get('route'):routes=[r for r in routes if r[0]==job['route']]
report={'status':'running','scope':'V3 controls on existing park and authored bridge routes; not full park/game/route acceptance.','cases':[]};stage=0;mark=0;started=time.monotonic();bike=None;first=None;water_start=None;route_index=0;reverse=False;active=[];water_seen=False;route_pedaling=False
def open_water_point():
 data=json.loads((root/'SourceAssets/Terrain/lake-clara-meer.json').read_text());outer=data['outer_cm'];island=data['island_cm'];z=data['water_z_cm']
 def inside(x,y,ring):
  yes=False
  for a,b in zip(ring,ring[1:]+ring[:1]):
   if (a[1]>y)!=(b[1]>y) and x<(b[0]-a[0])*(y-a[1])/(b[1]-a[1])+a[0]:yes=not yes
  return yes
 candidates=[]
 for dx in range(-3600,3601,600):
  for dy in range(-3600,3601,600):
   x=points['water_drop'][0]+dx;y=points['water_drop'][1]+dy
   if dx*dx+dy*dy<600**2 or not inside(x,y,outer) or inside(x,y,island):continue
   hit=unreal.PiedmontWorldTools.trace_world_surface(unreal.Vector(x,y,z+1000),unreal.Vector(x,y,z-2000),2)
   if hit and hit[0].z<z-40:candidates.append((dx*dx+dy*dy,unreal.Vector(x,y,z+600)))
 if not candidates:raise RuntimeError('No open water point clear of bridge decks')
 return min(candidates,key=lambda x:x[0])[1]

unreal.get_default_object(unreal.load_class(None,'/Script/UnrealEd.LevelEditorPlaySettings')).set_editor_property('GameGetsMouseControl',True)
def tick(dt):
 global stage,mark,bike,first,water_start,route_index,reverse,active,water_seen,route_pedaling,handle
 try:
  if time.monotonic()-started>900:raise RuntimeError('Park validation timeout')
  worlds=unreal.EditorLevelLibrary.get_pie_worlds(False)
  if not worlds:return
  w=worlds[0];now=unreal.GameplayStatics.get_time_seconds(w);mode=unreal.GameplayStatics.get_game_mode(w)
  if mode.get_editor_property('start_countdown')>0:return
  def check(name,ok,**extra):report['cases'].append({'name':name,'pass':bool(ok),**extra})
  def advance():
   global stage,mark
   stage+=1;mark=now
  def keys_off():
   for key in ['W','Left','Right']:bike.validation_key(key,False)
  def place(v,yaw=0):
   keys_off();bike.ride.stop_movement_immediately();bike.ride.set_editor_property('speed',0);bike.ride.set_editor_property('recovery',0);bike.set_actor_location_and_rotation(v,unreal.Rotator(yaw=yaw),False,True)
  def route_start():
   global active,mark,water_seen,route_pedaling
   name,pts,extend=routes[route_index];active=[list(p) for p in (reversed(pts) if reverse else pts)]
   if extend:
    for front in [True,False]:
     a,b=(active[0],active[1]) if front else (active[-1],active[-2]);dx=a[0]-b[0];dy=a[1]-b[1];length=math.hypot(dx,dy);v=[a[0]+dx/length*150,a[1]+dy/length*150,a[2]]
     if front:active.insert(0,v)
     else:active.append(v)
   a,b=active[:2];place(unreal.Vector(*a)+unreal.Vector(0,0,102),math.degrees(math.atan2(b[1]-a[1],b[0]-a[0])));bike.ride.set_editor_property('gear',1);mark=now;water_seen=False;route_pedaling=False
  if stage==0:
   bike=unreal.GameplayStatics.get_player_pawn(w,0);check('park_uses_V3_mode_and_arcade_bike',isinstance(mode,unreal.BattleParkMode) and isinstance(bike,unreal.BattleBike));first=bike.get_actor_location()
   entry=json.loads((root/'SourceAssets/Terrain/battle-start.json').read_text());distance=math.hypot(first.x-entry['start_xy_cm'][0],first.y-entry['start_xy_cm'][1]);angle=abs((bike.get_actor_rotation().yaw-entry['heading_yaw']+180)%360-180);check('spawns_inside_14th_street_gate_facing_inward',distance<10 and angle<1,distance_cm=distance,heading_error_degrees=angle)
   if job.get('startup_only'):
    report['status']='passed' if all(c['pass'] for c in report['cases']) else 'failed';(root/'Scripts/battle-park-start-validation.json').write_text(json.dumps(report,indent=2));unreal.unregister_slate_post_tick_callback(handle);level.editor_request_end_play();return
   advance()
  elif stage==1 and now-mark>8:
   visitors=unreal.GameplayStatics.get_all_actors_of_class(w,unreal.PiedmontPedestrian);check('park_streams_wandering_visitors',len(visitors)>=24,count=len(visitors))
   for d in unreal.GameplayStatics.get_all_actors_of_class(w,unreal.PiedmontTrafficDirector):d.set_actor_tick_enabled(False)
   for v in visitors:v.destroy_actor()
   bike.validation_key('W',True);advance()
  elif stage==2 and now-mark>2.5:
   check('actual_park_path_can_be_ridden',bike.ride.get_editor_property('wipeouts')==0 and (bike.get_actor_location()-first).length()>500 and abs(bike.get_actor_rotation().roll)<.1,distance_cm=(bike.get_actor_location()-first).length());drop=open_water_point();report['water_entry_cm']=[drop.x,drop.y,drop.z];place(drop);advance()
  elif stage==3 and now-mark>8:raise RuntimeError('Open-water entry did not start recovery')
  elif stage==3 and bike.ride.get_editor_property('recovery')>0:
   check('real_lake_entry_uses_V3_splash_recovery',bike.ride.get_editor_property('recovery_reason')=='Splash' and bike.get_editor_property('ride_effects').get_editor_property('splash_count')==1);water_start=now;advance()
  elif stage==4 and bike.ride.get_editor_property('recovery')<=0:
   pos=bike.get_actor_location();hit=unreal.PiedmontWorldTools.trace_world_surface(pos,pos-unreal.Vector(0,0,200),2);check('real_lake_returns_to_clear_path_in_two_seconds',1.8<now-water_start<2.2 and bool(hit and any(hit[1].actor_has_tag(t) for t in ['RidePath','RideDirt','RideBridge'])),seconds=now-water_start);route_start();advance()
  elif stage==5:
   if not route_pedaling:
    if now-mark<.15:return
    bike.validation_key('W',True);route_pedaling=True
   loc=bike.get_actor_location();water_seen=water_seen or bike.ride.get_editor_property('recovery')>0
   nearest=min(range(len(active)),key=lambda i:(active[i][0]-loc.x)**2+(active[i][1]-loc.y)**2);target=active[min(len(active)-1,nearest+3)];wanted=math.degrees(math.atan2(target[1]-loc.y,target[0]-loc.x));error=(wanted-bike.get_actor_rotation().yaw+180)%360-180;bike.validation_key('Right',error>3);bike.validation_key('Left',error<-3)
   distance=math.hypot(loc.x-active[-1][0],loc.y-active[-1][1]);length=sum(math.dist(a,b) for a,b in zip(active,active[1:]));timeout=max(12,length/250+8)
   if distance<105 or now-mark>timeout or water_seen:
    check('bridge_'+routes[route_index][0]+('_reverse' if reverse else '_forward'),distance<105 and not water_seen and abs(bike.get_actor_rotation().roll)<.1,remaining_cm=distance,recovery=water_seen,position=[loc.x,loc.y,loc.z]);keys_off()
    if not reverse:reverse=True;route_start()
    else:
     reverse=False;route_index+=1
     if route_index<len(routes):route_start()
     else:
      pts=lake['centerline_cm'];mid=pts[len(pts)//2];place(unreal.Vector(*mid)+unreal.Vector(0,0,102),math.degrees(math.atan2(pts[-1][1]-pts[0][1],pts[-1][0]-pts[0][0])));advance()
  elif stage==6 and now-mark>.7:
   check('V3_dismount_on_lake_bridge',bike.dismount());advance()
  elif stage==7 and now-mark>.5:
   rider=unreal.GameplayStatics.get_player_pawn(w,0);check('bridge_FPS_is_grounded_not_swimming',isinstance(rider,unreal.BattleRider) and not rider.get_editor_property('swimming') and rider.get_component_by_class(unreal.CharacterMovementComponent).is_moving_on_ground());check('bridge_pistol_fires',isinstance(rider,unreal.BattleRider) and rider.fire());check('bridge_remount_restores_arcade_bike',isinstance(rider,unreal.BattleRider) and rider.mount_bike());report['status']='passed' if all(c['pass'] for c in report['cases']) else 'failed';unreal.unregister_slate_post_tick_callback(handle)
   if not job.get('preview'):level.editor_request_end_play()
   stage=8
  report['stage']=stage;report['route']=routes[route_index][0] if route_index<len(routes) else 'bridge FPS';report['reverse']=reverse;(root/'Scripts/battle-park-validation.json').write_text(json.dumps(report,indent=2))
 except Exception:
  report['status']='error';report['error']=traceback.format_exc();(root/'Scripts/battle-park-validation.json').write_text(json.dumps(report,indent=2));unreal.unregister_slate_post_tick_callback(handle);level.editor_request_end_play()
if job.get('action')=='stop':
 import gc,types
 # Cancel only this script's live validator, including pre-registry runs.
 for f in gc.get_objects():
  if isinstance(f,types.FunctionType) and f.__name__=='tick' and f is not tick:
   g=f.__globals__
   if g.get('report',{}).get('scope')==report['scope'] and g.get('report',{}).get('status')=='running' and 'handle' in g:
    unreal.unregister_slate_post_tick_callback(g['handle']);g['report']['status']='stopped';(root/'Scripts/battle-park-validation.json').write_text(json.dumps(g['report'],indent=2))
 level.editor_request_end_play()
else:
 handle=unreal.register_slate_post_tick_callback(tick);level.editor_request_begin_play()
