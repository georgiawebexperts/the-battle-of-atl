"""Preserve the fall trajectory and align a sleeping loop at its landing point."""
import json, math, pathlib, unreal
root=pathlib.Path(unreal.Paths.project_dir())
folder='/Game/BattleRetarget/Mixamo/StumbleCandidate'
assert not unreal.EditorAssetLibrary.does_asset_exist(folder+'/StumbleToSleep'), 'Review existing candidate before rebuilding'
fall=unreal.load_asset(folder+'/MixamoStumbleReference_Anim')
sleep=unreal.load_asset('/Game/BattleRetarget/Mixamo/SleepCandidate/MixamoSleepReference_Anim')
mesh=unreal.load_asset('/Game/CitySampleCrowd/Character/Male/NormalWeight/Meshes/m_tal_nrw_body')
assert fall and sleep and mesh
component=unreal.SkeletalMeshComponent();component.set_skeletal_mesh_asset(mesh)
bones=[component.get_bone_name(i) for i in range(component.get_num_bones())]
parents=[bones.index(component.get_parent_bone(b)) if component.get_parent_bone(b) in bones else -1 for b in bones]
def poses(clip,time):return unreal.AnimationLibrary.get_bone_poses_for_time(clip,bones,time,False,mesh)
def globals_for(local):
    values=[]
    for i,pose in enumerate(local):values.append(unreal.MathLibrary.compose_transforms(pose,values[parents[i]]) if parents[i]>=0 else pose)
    return values
fps=30;fall_frames=round(fall.get_editor_property('sequence_length')*fps);blend_frames=24
frames=[poses(fall,f/fps) for f in range(fall_frames+1)]
end=frames[-1];sleep_start=poses(sleep,0)
a=globals_for(end);b=globals_for(sleep_start);hip=bones.index('pelvis');head=bones.index('head')
ad=a[head].translation-a[hip].translation;bd=b[head].translation-b[hip].translation
yaw=math.degrees(math.atan2(ad.y,ad.x)-math.atan2(bd.y,bd.x))
rotation=unreal.Rotator(yaw=yaw)
rotated=unreal.MathLibrary.rotate_angle_axis(b[hip].translation,yaw,unreal.Vector(0,0,1))
translation=unreal.Vector(a[hip].translation.x-rotated.x,a[hip].translation.y-rotated.y,0)
anchor=unreal.Transform(location=translation,rotation=rotation,scale=unreal.Vector(1,1,1))
def aligned(local):
    result=list(local)
    for i,parent in enumerate(parents):
        if parent<0:result[i]=unreal.MathLibrary.compose_transforms(result[i],anchor)
    return result
aligned_start=aligned(sleep_start)
for f in range(1,blend_frames+1):
    alpha=f/blend_frames;alpha=alpha*alpha*(3-2*alpha)
    frames.append([unreal.MathLibrary.t_lerp(x,y,alpha) for x,y in zip(end,aligned_start)])
def save(name,keys):
    factory=unreal.AnimSequenceFactory();factory.set_editor_property('target_skeleton',fall.get_editor_property('skeleton'))
    clip=unreal.AssetToolsHelpers.get_asset_tools().create_asset(name,folder,unreal.AnimSequence,factory)
    controller=clip.get_editor_property('controller');controller.open_bracket('Bake authored landing transition',False)
    controller.set_frame_rate(unreal.FrameRate(fps,1),False);controller.set_number_of_frames(unreal.FrameNumber(len(keys)-1),False)
    for i,bone in enumerate(bones):
        controller.add_bone_curve(bone,False);track=[frame[i] for frame in keys]
        assert controller.set_bone_track_keys(bone,[p.translation for p in track],[p.rotation for p in track],[p.scale3d for p in track],False)
    controller.close_bracket(False);assert unreal.EditorAssetLibrary.save_loaded_asset(clip,False)
    return clip.get_path_name()
fall_path=save('StumbleToSleep',frames)
loop_frames=round(sleep.get_editor_property('sequence_length')*fps)
loop_path=save('SleepingAtLanding',[aligned(poses(sleep,f/fps)) for f in range(loop_frames+1)])
report={'transition':fall_path,'aligned_loop':loop_path,'duration':(len(frames)-1)/fps,'blend_seconds':blend_frames/fps,'anchor_local_translation':[translation.x,translation.y,translation.z],'anchor_local_yaw_degrees':yaw,'visual_acceptance':False,'gameplay_installed':False,'note':'Trajectory remains in mesh root; runtime must reserve fall clearance and reconcile capsule with landing anchor before placement.'}
(root/'Tests/Results/2026-09-12-stumble-sleep-bake.json').write_text(json.dumps(report,indent=2)+'\n')
print('BATTLE_STUMBLE_SLEEP_BAKED')
