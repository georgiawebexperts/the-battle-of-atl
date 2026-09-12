"""Bake the evaluated sleeping pose through the same path as fall/wake transitions."""
import unreal,json,pathlib
root=pathlib.Path(unreal.Paths.project_dir());folder='/Game/BattleRetarget/Mixamo/SleepCandidate'
assert not unreal.EditorAssetLibrary.does_asset_exist(folder+'/SleepingBaked')
fall=unreal.load_asset(folder+'/MixamoSleepReference_Anim');assert fall
mesh=unreal.load_asset('/Game/CitySampleCrowd/Character/Male/NormalWeight/Meshes/m_tal_nrw_body')
component=unreal.SkeletalMeshComponent();component.set_skeletal_mesh_asset(mesh)
bones=[component.get_bone_name(i) for i in range(component.get_num_bones())];fps=30
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
count=round(fall.get_editor_property('sequence_length')*fps)
keys=[unreal.AnimationLibrary.get_bone_poses_for_time(fall,bones,frame/fps,False,mesh) for frame in range(count+1)]
path=save('SleepingBaked',keys)
(root/'Tests/Results/2026-09-12-sleep-baked.json').write_text(json.dumps({'path':path,'source':fall.get_path_name(),'frames':len(keys),'runtime_seam_verified':False},indent=2)+'\n')
