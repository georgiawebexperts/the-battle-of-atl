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
 hand=world[bones.index('hand_r')];q=hand.rotation
 row={'time':frame/30,'hand_rotation_xyzw':[q.x,q.y,q.z,q.w]}
 for axis in ['x','y','z']:
  v={'x':unreal.Vector(1,0,0),'y':unreal.Vector(0,1,0),'z':unreal.Vector(0,0,1)}[axis];v=unreal.MathLibrary.transform_direction(hand,v);row['axis_'+axis]=[v.x,v.y,v.z]
 for name in ['hand_r','index_01_r','middle_01_r','thumb_01_r','thumb_03_r']:
  p=world[bones.index(name)].translation;row[name]=[p.x,p.y,p.z]
 rows.append(row)
report={'samples':[min(rows,key=lambda x:abs(x['time']-t)) for t in [0,1.5,2.2,3.5]],'clip':clip.get_path_name()}
(root/'Tests/Results/2026-09-12-bench-hand-grip.json').write_text(json.dumps(report,indent=2)+'\n')
