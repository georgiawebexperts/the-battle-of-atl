"""Preserve an aim trial and restore its stationary pelvis height from measured bind pose."""
import unreal,json,pathlib,math
root=pathlib.Path(unreal.Paths.project_dir());folder='/Game/BattleRetarget/Gunman/AimGrounded'
assert not unreal.EditorAssetLibrary.does_directory_exist(folder)
source=unreal.load_asset('/Game/BattleRetarget/Gunman/AimCandidate/Animations_CharacterArmature_Idle_Gun_Pointing')
mesh=unreal.load_asset('/Game/CitySampleCrowd/Character/Female/NormalWeight/Meshes/f_tal_nrw_body')
component=unreal.SkeletalMeshComponent();component.set_skeletal_mesh_asset(mesh)
bones=[component.get_bone_name(i) for i in range(component.get_num_bones())];fps=30;frames=math.ceil(source.get_editor_property('sequence_length')*fps)
keys=[unreal.AnimationLibrary.get_bone_poses_for_time(source,bones,min(frame/fps,source.get_editor_property('sequence_length')),False,mesh) for frame in range(frames+1)]
pelvis=bones.index('pelvis') if 'pelvis' in bones else next(i for i,n in enumerate(bones) if str(n)=='pelvis')
# Native GunmanPose diagnostic measured target mesh pelvis local bind translation.
bind=unreal.Vector(0,2.217581,91.978760);offset=bind-keys[0][pelvis].translation
factory=unreal.AnimSequenceFactory();factory.set_editor_property('target_skeleton',source.get_editor_property('skeleton'))
clip=unreal.AssetToolsHelpers.get_asset_tools().create_asset('GunmanPointing',folder,unreal.AnimSequence,factory)
c=clip.get_editor_property('controller');c.open_bracket('Restore stationary aim pelvis height',False);c.set_frame_rate(unreal.FrameRate(fps,1),False);c.set_number_of_frames(unreal.FrameNumber(frames),False)
for i,bone in enumerate(bones):
 track=[frame[i] for frame in keys];positions=[t.translation+offset if i==pelvis else t.translation for t in track]
 c.add_bone_curve(bone,False);assert c.set_bone_track_keys(bone,positions,[t.rotation for t in track],[t.scale3d for t in track],False)
c.close_bracket(False);assert unreal.EditorAssetLibrary.save_loaded_asset(clip,False)
(root/'Tests/Results/2026-09-13-gunman-aim-grounded-export.json').write_text(json.dumps({'clip':clip.get_path_name(),'frames':frames,'pelvis_offset':[offset.x,offset.y,offset.z],'visual_acceptance':False},indent=2)+'\n')
