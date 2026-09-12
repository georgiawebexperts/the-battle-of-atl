"""Preserve source candidates and correct their pelvis units for the 100x armature."""
import unreal,json,pathlib
root=pathlib.Path(unreal.Paths.project_dir());folder='/Game/BattleRetarget/Ellison/RecoveryScaled'
assert not unreal.EditorAssetLibrary.does_directory_exist(folder)
mesh=unreal.load_asset('/Game/PiedmontRide/Rider/Casual');component=unreal.SkeletalMeshComponent();component.set_skeletal_mesh_asset(mesh)
bones=[component.get_bone_name(i) for i in range(component.get_num_bones())];fps=30;rows=[]
for side in ['F','B','L','R']:
 source=unreal.load_asset('/Game/BattleRetarget/Ellison/RecoveryCandidate/M_ragdoll_getup_stand_'+side)
 keys=[unreal.AnimationLibrary.get_bone_poses_for_time(source,bones,frame/fps,False,mesh) for frame in range(151)]
 factory=unreal.AnimSequenceFactory();factory.set_editor_property('target_skeleton',source.get_editor_property('skeleton'))
 clip=unreal.AssetToolsHelpers.get_asset_tools().create_asset('GetUp_'+side,folder,unreal.AnimSequence,factory)
 controller=clip.get_editor_property('controller');controller.open_bracket('Correct imported armature units',False)
 controller.set_frame_rate(unreal.FrameRate(fps,1),False);controller.set_number_of_frames(unreal.FrameNumber(150),False)
 for i,bone in enumerate(bones):
  track=[frame[i] for frame in keys]
  positions=[p.translation/100 if str(bone)=='Hips' else p.translation for p in track]
  scales=[unreal.Vector(100,100,100) if str(bone)=='CharacterArmature' else p.scale3d for p in track]
  controller.add_bone_curve(bone,False)
  assert controller.set_bone_track_keys(bone,positions,[p.rotation for p in track],scales,False)
 controller.close_bracket(False);assert unreal.EditorAssetLibrary.save_loaded_asset(clip,False)
 rows.append(clip.get_path_name())
(root/'Tests/Results/2026-09-12-ellison-recovery-scaled.json').write_text(json.dumps({'clips':rows,'pelvis_translation_scale':.01,'armature_scale':100,'runtime_verified':False},indent=2)+'\n')
