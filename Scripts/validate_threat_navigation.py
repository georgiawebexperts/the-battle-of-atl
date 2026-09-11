"""Actual park pursuit around water, followed by the first-stab/escape rule."""
import unreal,json,pathlib,time,traceback,math
root=pathlib.Path(unreal.Paths.project_dir());level=unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
world=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
if world.get_name()!='PiedmontWorld':raise RuntimeError('Park map required')
candidates=json.loads((root/'SourceAssets/Terrain/threat-navigation-candidates.json').read_text());chosen=None
for row in candidates:
 a=unreal.PiedmontWorldTools.project_park_navigation(unreal.Vector(*row['start']));b=unreal.PiedmontWorldTools.project_park_navigation(unreal.Vector(*row['target']))
 if not a or not b:continue
 length=unreal.PiedmontWorldTools.park_route_length(a,b)
 if row['straight_cm']*1.5<length<12000:chosen=(a,b,length,row['straight_cm']);break
if not chosen:raise RuntimeError('No complete around-lake test route found')
report={'status':'running','scope':'Live knife pursuit around Lake Clara Meer using saved park navigation, then first-stab escape.','route_cm':chosen[2],'straight_cm':chosen[3],'cases':[]};stage=0;started=time.monotonic();threat=None;bike=None;last=None;distance=0.;swam=False;max_points=0;timer=0

def tick(dt):
 global stage,threat,bike,last,distance,swam,max_points,handle,timer
 try:
  if time.monotonic()-started>160:raise RuntimeError('Park navigation pursuit timeout')
  worlds=unreal.EditorLevelLibrary.get_pie_worlds(False)
  if not worlds:return
  w=worlds[0];mode=unreal.GameplayStatics.get_game_mode(w)
  if mode.get_editor_property('start_countdown')>0:return
  pawn=unreal.GameplayStatics.get_player_pawn(w,0)
  def record(name,ok,**details):report['cases'].append({'name':name,'pass':bool(ok),**details})
  if stage==0:
   bike=pawn;bike.set_actor_location_and_rotation(chosen[1]+unreal.Vector(0,0,98),unreal.Rotator(),False,True);bike.ride.set_editor_property('speed',0);bike.set_actor_tick_enabled(False)
   threat=mode.spawn_threat_for_validation(False,chosen[0]+unreal.Vector(0,0,90))
   if not threat:raise RuntimeError('Could not spawn navigation test attacker')
   timer=mode.get_editor_property('time_remaining');last=threat.get_actor_location();stage=1
  elif stage==1:
   here=threat.get_actor_location();distance+=math.dist([here.x,here.y],[last.x,last.y]);last=here
   swam=swam or threat.get_editor_property('swimming');max_points=max(max_points,len(threat.get_editor_property('nav_route')))
   report.update({'travel_cm':distance,'nav_points_max':max_points,'swam':swam,'route_requests':threat.get_editor_property('route_requests'),'navigation_failure':threat.get_editor_property('navigation_failure'),'remaining_cm':math.dist([here.x,here.y],[chosen[1].x,chosen[1].y])})
   if isinstance(pawn,unreal.PiedmontExplorer):
    record('uses_multi_waypoint_park_navigation',threat.get_editor_property('using_navigation') and max_points>2 and threat.get_editor_property('route_requests')>1)
    record('takes_land_route_around_lake',not swam and distance>chosen[3]*1.3,travel_cm=distance,straight_cm=chosen[3])
    record('reaches_player_and_first_stab_dismounts',pawn.get_editor_property('knife_wounded') and threat.get_editor_property('attacks')==1 and not mode.get_editor_property('run_ended'))
    record('clock_keeps_ticking_during_pursuit',mode.get_editor_property('time_remaining')<timer-3)
    record('can_escape_by_remounting',pawn.remount());threat.destroy_actor();bike.set_actor_tick_enabled(True)
    stage=2
  elif stage==2:
   safe=json.loads((root/'SourceAssets/Terrain/lake-test-points.json').read_text())['safe_start'];bike.set_actor_location_and_rotation(unreal.Vector(*safe),unreal.Rotator(),False,True);bike.ride.set_editor_property('speed',0);bike.set_actor_tick_enabled(False);stage=3
  elif stage==3:
   before=mode.get_editor_property('encounter_countdown');spawned=mode.try_encounter_for_validation()
   record('rare_encounter_selects_reachable_spawn',spawned is not None)
   if spawned:
    pos=spawned.get_actor_location();target=bike.get_actor_location();gap=math.dist([pos.x,pos.y],[target.x,target.y]);record('encounter_stays_twenty_to_thirty_five_meters_away',2000<=gap<=3500,distance_cm=gap)
    record('only_one_active_encounter_after_repeated_opportunity',mode.try_encounter_for_validation()==spawned and len(unreal.GameplayStatics.get_all_actors_of_class(w,unreal.PiedmontThreat))==1);spawned.destroy_actor()
   record('validation_does_not_change_rare_encounter_clock',abs(mode.get_editor_property('encounter_countdown')-before)<.01)
   bike.set_actor_tick_enabled(True);report['status']='passed' if all(r['pass'] for r in report['cases']) else 'failed';stage=4;unreal.unregister_slate_post_tick_callback(handle);level.editor_request_end_play()
  (root/'Scripts/threat-navigation-validation.json').write_text(json.dumps(report,indent=2))
 except Exception:
  report['status']='error';report['error']=traceback.format_exc();(root/'Scripts/threat-navigation-validation.json').write_text(json.dumps(report,indent=2));unreal.unregister_slate_post_tick_callback(handle);level.editor_request_end_play()
handle=unreal.register_slate_post_tick_callback(tick);level.editor_request_begin_play()
