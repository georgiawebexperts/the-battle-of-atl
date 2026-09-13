import unreal,json,pathlib
rows=[]
for n in ['M_Relaxed_Jump_F_Land_Stand_Light_Lfoot','M_Relaxed_Jump_F_Land_Run_Light_Lfoot']:
 a=unreal.load_asset('/Game/BattleRetarget/Ellison/CityLanding/'+n);samples=[]
 for i in range(int(a.get_editor_property('sequence_length')*10)+1):
  t=i/10;pose=unreal.AnimPoseExtensions.get_anim_pose_at_time(a,t,unreal.AnimPoseEvaluationOptions());r={'t':t}
  for b in ['root','pelvis','foot_l','foot_r']:
   v=unreal.AnimPoseExtensions.get_bone_pose(pose,b,unreal.AnimPoseSpaces.WORLD).translation;r[b]=[v.x,v.y,v.z]
  samples.append(r)
 rows.append({'clip':n,'samples':samples})
pathlib.Path('/Volumes/Adam Assets/Unreal/Projects/AuraPlayground/Tests/Results/2026-09-13-landing-profile.json').write_text(json.dumps(rows,indent=2))
