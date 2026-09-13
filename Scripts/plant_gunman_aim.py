"""Layer the authored gun upper body over a compatible native standing lower body."""
import unreal,json,pathlib,math
root=pathlib.Path(unreal.Paths.project_dir());folder='/Game/BattleRetarget/Gunman/AimPlanted'
assert not unreal.EditorAssetLibrary.does_directory_exist(folder)
aim=unreal.load_asset('/Game/BattleRetarget/Gunman/AimGrounded/GunmanPointing')
idle=unreal.load_asset('/Game/CitySampleCrowd/Character/Anims/Loco/FTN_Set/FTN_N_Idle_Base')
mesh=unreal.load_asset('/Game/CitySampleCrowd/Character/Female/NormalWeight/Meshes/f_tal_nrw_body')
assert aim and idle and mesh
component=unreal.SkeletalMeshComponent();component.set_skeletal_mesh_asset(mesh)
bones=[component.get_bone_name(i) for i in range(component.get_num_bones())]
def upper(bone):
 while str(bone)!='None':
  if str(bone)=='spine_01':return True
  bone=component.get_parent_bone(bone)
 return False
mask=[upper(bone) for bone in bones];fps=30;duration=idle.get_editor_property('sequence_length');frames=math.ceil(duration*fps)
keys=[]
for frame in range(frames+1):
 t=min(frame/fps,duration);phase=t/duration
 ap=unreal.AnimationLibrary.get_bone_poses_for_time(aim,bones,phase*aim.get_editor_property('sequence_length'),False,mesh)
 ip=unreal.AnimationLibrary.get_bone_poses_for_time(idle,bones,t,False,mesh)
 keys.append([ap[i] if mask[i] else ip[i] for i in range(len(bones))])
factory=unreal.AnimSequenceFactory();factory.set_editor_property('target_skeleton',aim.get_editor_property('skeleton'))
clip=unreal.AssetToolsHelpers.get_asset_tools().create_asset('GunmanStandingAim',folder,unreal.AnimSequence,factory)
c=clip.get_editor_property('controller');c.open_bracket('Standing lower body with authored gun upper body',False);c.set_frame_rate(unreal.FrameRate(fps,1),False);c.set_number_of_frames(unreal.FrameNumber(frames),False)
for i,bone in enumerate(bones):
 track=[frame[i] for frame in keys];c.add_bone_curve(bone,False);assert c.set_bone_track_keys(bone,[t.translation for t in track],[t.rotation for t in track],[t.scale3d for t in track],False)
c.close_bracket(False);assert unreal.EditorAssetLibrary.save_loaded_asset(clip,False)
(root/'Tests/Results/2026-09-13-gunman-planted-export.json').write_text(json.dumps({'clip':clip.get_path_name(),'upper_bones':sum(mask),'lower_bones':len(mask)-sum(mask),'duration':duration,'frames':frames,'visual_acceptance':False},indent=2)+'\n')
