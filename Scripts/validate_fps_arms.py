"""Live skinned arm poses, recoil/reload motion and remount cleanup."""
import unreal,pathlib,json,time,traceback
root=pathlib.Path(unreal.Paths.project_dir());level=unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
report={'status':'running','scope':'First-person arm integration; visual grip quality is reviewed separately.','cases':[]};stage=0;mark=0.;started=time.monotonic();rider=None;bike=None;arms=None;right=None;left=None;reload_motion=0.;reload_angle=0.
unreal.get_default_object(unreal.load_class(None,'/Script/UnrealEd.LevelEditorPlaySettings')).set_editor_property('GameGetsMouseControl',True)
def tick(dt):
 global stage,mark,rider,bike,arms,right,left,reload_motion,reload_angle,handle
 try:
  if time.monotonic()-started>100:raise RuntimeError('Arms test timeout')
  worlds=unreal.EditorLevelLibrary.get_pie_worlds(False)
  if not worlds:return
  w=worlds[0];now=unreal.GameplayStatics.get_time_seconds(w);mode=unreal.GameplayStatics.get_game_mode(w)
  if mode.get_editor_property('start_countdown')>0:return
  def check(n,ok,**kw):report['cases'].append({'name':n,'pass':bool(ok),**kw})
  def advance():
   global stage,mark
   stage+=1;mark=now
  def hand(side):return arms.get_bone_location_by_name('Hand_'+side,unreal.BoneSpaces.COMPONENT_SPACE)
  if stage==0:
   bike=unreal.GameplayStatics.get_player_pawn(w,0)
   if not bike.dismount():raise RuntimeError('Cannot enter FPS')
   rider=unreal.GameplayStatics.get_player_pawn(w,0);arms=rider.get_editor_property('first_person_arms');advance()
  elif stage==1 and now-mark>.5:
   check('skinned_arms_and_finger_rig_loaded',arms.get_num_bones()>50 and arms.get_bone_index('Index3_R')>=0 and arms.get_editor_property('skinned_asset') is not None)
   check('arms_visible_only_to_owner',arms.is_visible() and arms.get_editor_property('only_owner_see'))
   right=hand('R');left=hand('L');check('wrists_reach_pistol_grip_targets',(right-rider.get_editor_property('right_grip')).length()<2 and (left-rider.get_editor_property('left_grip')).length()<2)
   rider.fire();advance()
  elif stage==2:
   if (hand('R')-right).length()>.3 or now-mark>.25:
    check('right_hand_moves_with_recoil',(hand('R')-right).length()>.3);advance()
  elif stage==3 and now-mark>.5:
   left=hand('L');rider.reload();advance()
  elif stage==4:
   reload_motion=max(reload_motion,(hand('L')-left).length());reload_angle=max(reload_angle,abs(rider.get_editor_property('weapon').get_editor_property('relative_rotation').pitch))
   if now-mark>1.8:
    check('reload_has_support_hand_motion',reload_motion>10,movement_cm=reload_motion)
    check('reload_tilts_pistol',reload_angle>15,angle=reload_angle)
    check('reload_completes_and_returns_to_grip',rider.get_editor_property('ammo')==12 and (hand('L')-rider.get_editor_property('left_grip')).length()<2)
    check('remount_cleans_up_first_person_arms',rider.mount_bike() and len(unreal.GameplayStatics.get_all_actors_of_class(w,unreal.BattleRider))==0)
    report['status']='passed' if all(c['pass'] for c in report['cases']) else 'failed';unreal.unregister_slate_post_tick_callback(handle)
    if globals().get('WORLD_JOB',{}).get('preview'):bike.dismount()
    else:level.editor_request_end_play()
    stage=5
  report['stage']=stage;(root/'Scripts/fps-arms-validation.json').write_text(json.dumps(report,indent=2))
 except Exception:
  report['status']='error';report['error']=traceback.format_exc();(root/'Scripts/fps-arms-validation.json').write_text(json.dumps(report,indent=2));unreal.unregister_slate_post_tick_callback(handle);level.editor_request_end_play()
handle=unreal.register_slate_post_tick_callback(tick);level.editor_request_begin_play()
