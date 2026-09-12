"""Bake a review-only sleep-to-get-up transition, preserving source animations."""
import json, pathlib, unreal
root=pathlib.Path(unreal.Paths.project_dir())
folder='/Game/BattleRetarget/Mixamo/SleepCandidate'
path=folder+'/SleepToStand_R'
assert not unreal.EditorAssetLibrary.does_asset_exist(path), 'Review existing candidate before rebuilding'
sleep=unreal.load_asset(folder+'/MixamoSleepReference_Anim')
wake=unreal.load_asset('/Game/BattleRetarget/City/Male/M_ragdoll_getup_stand_R')
mesh=unreal.load_asset('/Game/CitySampleCrowd/Character/Male/NormalWeight/Meshes/m_tal_nrw_body')
assert sleep and wake and mesh
component=unreal.SkeletalMeshComponent();component.set_skeletal_mesh_asset(mesh)
bones=[component.get_bone_name(i) for i in range(component.get_num_bones())]
def poses(clip,time):
    return unreal.AnimationLibrary.get_bone_poses_for_time(clip,bones,time,False,mesh)
start=poses(sleep,0);end=poses(wake,0)
fps=30;blend_frames=15;wake_frames=round(wake.get_editor_property('sequence_length')*fps)
frames=[]
for frame in range(blend_frames):
    alpha=frame/blend_frames
    alpha=alpha*alpha*(3-2*alpha)
    frames.append([unreal.MathLibrary.t_lerp(a,b,alpha) for a,b in zip(start,end)])
for frame in range(wake_frames+1):frames.append(poses(wake,frame/fps))
factory=unreal.AnimSequenceFactory();factory.set_editor_property('target_skeleton',wake.get_editor_property('skeleton'))
clip=unreal.AssetToolsHelpers.get_asset_tools().create_asset('SleepToStand_R',folder,unreal.AnimSequence,factory)
controller=clip.get_editor_property('controller');controller.open_bracket('Build sleeping wake transition',False);controller.set_frame_rate(unreal.FrameRate(fps,1),False)
controller.set_number_of_frames(unreal.FrameNumber(len(frames)-1),False)
for index,bone in enumerate(bones):
    controller.add_bone_curve(bone,False)
    track=[frame[index] for frame in frames]
    assert controller.set_bone_track_keys(bone,[p.translation for p in track],[p.rotation for p in track],[p.scale3d for p in track],False),str(bone)
controller.close_bracket(False)
assert unreal.EditorAssetLibrary.save_loaded_asset(clip,False)
(root/'Tests/Results/2026-09-11-sleep-wake-bake.json').write_text(json.dumps({'clip':clip.get_path_name(),'sleep_source':sleep.get_path_name(),'wake_source':wake.get_path_name(),'blend_seconds':blend_frames/fps,'frames':len(frames),'bones':len(bones),'duration':clip.get_editor_property('sequence_length'),'visual_acceptance':False,'gameplay_installed':False},indent=2)+'\n')
print('BATTLE_SLEEP_WAKE_BAKED')
