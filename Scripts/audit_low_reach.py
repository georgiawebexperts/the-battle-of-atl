"""Measure hand path for bench contact planning, without assigning gameplay."""
import unreal,json,pathlib
root=pathlib.Path(unreal.Paths.project_dir())
mesh=unreal.load_asset('/Game/CitySampleCrowd/Character/Male/NormalWeight/Meshes/m_tal_nrw_body')
clip=unreal.load_asset('/Game/BattleRetarget/Mixamo/LowReachCandidate/MixamoLowReachReference_Anim')
assert mesh and clip
component=unreal.SkeletalMeshComponent();component.set_skeletal_mesh_asset(mesh)
bones=[component.get_bone_name(i) for i in range(component.get_num_bones())]
parents=[bones.index(component.get_parent_bone(b)) if component.get_parent_bone(b) in bones else -1 for b in bones]
rows=[]
for frame in range(round(clip.get_editor_property('sequence_length')*30)+1):
 poses=unreal.AnimationLibrary.get_bone_poses_for_time(clip,bones,frame/30,False,mesh);world=[]
 for i,pose in enumerate(poses):world.append(unreal.MathLibrary.compose_transforms(pose,world[parents[i]]) if parents[i]>=0 else pose)
 row={'time':frame/30}
 for name in ['hand_r','hand_l','foot_r','foot_l','pelvis']:
  p=world[bones.index(name)].translation;row[name]=[p.x,p.y,p.z]
 rows.append(row)
report={'clip':clip.get_path_name(),'space':'Skeletal mesh component coordinates, centimeters; no actor/ground transform applied','lowest_right_hand':min(rows,key=lambda x:x['hand_r'][2]),'lowest_left_hand':min(rows,key=lambda x:x['hand_l'][2]),'samples':rows,'gameplay_installed':False,'native_contact_verified':False}
(root/'Tests/Results/2026-09-12-low-reach-trajectory.json').write_text(json.dumps(report,indent=2)+'\n')
