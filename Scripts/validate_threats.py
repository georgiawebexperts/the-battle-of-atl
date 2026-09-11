"""Live attacker pursuit, escape/remount, defensive shooting and two-hit death."""
import unreal,json,pathlib,time,traceback,math
root=pathlib.Path(unreal.Paths.project_dir());level=unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
report={'status':'running','cases':[]};stage=0;elapsed=0.;last=None;bike=None;person=None;threat=None;timer=0.;started=time.monotonic()
def tick(delta):
 global stage,elapsed,last,bike,person,threat,timer,handle
 try:
  if time.monotonic()-started>240:raise RuntimeError('Threat integration timeout')
  worlds=unreal.EditorLevelLibrary.get_pie_worlds(False)
  if not worlds:return
  w=worlds[0];mode=unreal.GameplayStatics.get_game_mode(w);pc=unreal.GameplayStatics.get_player_controller(w,0);pawn=unreal.GameplayStatics.get_player_pawn(w,0)
  if mode.get_editor_property('start_countdown')>0:return
  now=unreal.GameplayStatics.get_time_seconds(w)
  if last is None:last=now
  elapsed+=min(now-last,.12);last=now
  def record(name,okay,**kw):report['cases'].append({'name':name,'pass':bool(okay),**kw})
  def next_stage():
   global stage,elapsed
   stage+=1;elapsed=0
  if stage==0:
   bike=pawn;bike.set_actor_location_and_rotation(unreal.Vector(0,4000,100),unreal.Rotator(),False,True);bike.ride.set_editor_property('speed',0);timer=mode.get_editor_property('time_remaining')
   record('protected_start_has_long_encounter_cooldown',mode.get_editor_property('encounter_countdown')>290)
   threat=mode.spawn_threat_for_validation(False,unreal.Vector(-450,4000,100))
   if not threat:raise RuntimeError('Failed to spawn knife attacker')
   record('only_one_active_attacker',not mode.spawn_threat_for_validation(True,unreal.Vector(1000,4000,100)));next_stage()
  elif stage==1:
   if isinstance(pawn,unreal.PiedmontExplorer):
    person=pawn;record('knife_attacker_chases_and_dismounts_player',person.get_editor_property('knife_wounded') and threat.get_editor_property('attacks')==1 and not mode.get_editor_property('run_ended'))
    record('escape_by_remounting_before_second_stab',person.remount());bike.set_actor_location_and_rotation(unreal.Vector(1000,4000,100),unreal.Rotator(),False,True);next_stage()
   elif elapsed>10:raise RuntimeError('Knife attacker did not reach and dismount player')
  elif stage==2 and elapsed>1:
   record('remount_survives_first_stab_and_clock_continues',not mode.get_editor_property('run_ended') and mode.get_editor_property('time_remaining')<timer)
   threat.destroy_actor();bike.set_actor_location_and_rotation(unreal.Vector(0,4000,100),unreal.Rotator(),False,True);bike.ride.set_editor_property('speed',0)
   if not bike.dismount():raise RuntimeError('Cannot dismount for defense test')
   next_stage()
  elif stage==3 and elapsed>.5:
   person=pawn;person.toggle_weapon();pc.set_control_rotation(unreal.Rotator());loc=person.get_actor_location();threat=mode.spawn_threat_for_validation(True,loc+unreal.Vector(700,0,0))
   if not threat:raise RuntimeError('Cannot spawn gunman')
   next_stage()
  elif stage==4:
   person.aim_at_for_validation(threat)
   if elapsed<.7:return
   record('player_fires_at_attacker',person.fire(),shot=str(person.get_editor_property('last_shot_end')),target=str(threat.get_actor_location()),health=threat.get_editor_property('health'));record('hit_creates_blood_effect',len(unreal.GameplayStatics.get_all_actors_of_class(w,unreal.PiedmontBlood))>0);next_stage()
  elif stage==5 and elapsed>.5:
   record('player_shot_defeats_attacker',threat.get_editor_property('dead'));mode.restart_run();next_stage()
  elif stage==6:
   bike=pawn;bike.set_actor_location_and_rotation(unreal.Vector(0,4000,100),unreal.Rotator(),False,True);bike.ride.set_editor_property('speed',0);threat=mode.spawn_threat_for_validation(False,unreal.Vector(-450,4000,100));next_stage()
  elif stage==7:
   if mode.get_editor_property('run_ended'):
    record('unescaped_chase_causes_second_stab_death',threat.get_editor_property('attacks')==2 and 'Second stab' in mode.get_editor_property('failure_reason'));report['status']='passed' if all(r['pass'] for r in report['cases']) else 'failed';unreal.unregister_slate_post_tick_callback(handle);level.editor_request_end_play();next_stage()
   elif elapsed>15:raise RuntimeError('Second-stab chase did not finish')
  report['stage']=stage;(root/'Scripts/threat-validation.json').write_text(json.dumps(report,indent=2))
 except Exception:
  report['status']='error';report['error']=traceback.format_exc();(root/'Scripts/threat-validation.json').write_text(json.dumps(report,indent=2));unreal.unregister_slate_post_tick_callback(handle);level.editor_request_end_play()
handle=unreal.register_slate_post_tick_callback(tick);level.editor_request_begin_play()
