"""Export existing licensed recovery motions onto Ellison's skeleton for review."""
import unreal,json,pathlib
root=pathlib.Path(unreal.Paths.project_dir())
destination='/Game/BattleRetarget/Ellison/RecoveryCandidate'
assert not unreal.EditorAssetLibrary.does_directory_exist(destination),'Review existing candidate before replacing'
source=unreal.load_asset('/Game/CitySampleCrowd/Character/Male/NormalWeight/Meshes/m_tal_nrw_body')
target=unreal.load_asset('/Game/PiedmontRide/Rider/Casual')
assert source and target
tools=unreal.AssetToolsHelpers.get_asset_tools()
chains=[('Spine','spine_01','spine_05','Abdomen','Chest'),('Neck','neck_01','neck_02','Neck','Neck'),('Head','head','head','Head','Head')]
for side in ['L','R']:
 low=side.lower()
 chains += [('Clavicle'+side,'clavicle_'+low,'clavicle_'+low,'Shoulder_'+side,'Shoulder_'+side),('Arm'+side,'upperarm_'+low,'hand_'+low,'UpperArm_'+side,'Hand_'+side),('Leg'+side,'thigh_'+low,'foot_'+low,'UpperLeg_'+side,'Foot_'+side)]
rigs=[]
for label,mesh,hip,start,end in [('Source',source,'pelvis',1,2),('Target',target,'Hips',3,4)]:
 pose=unreal.PoseableMeshComponent();pose.set_skinned_asset_and_update(mesh)
 bones={str(pose.get_bone_name(i)) for i in range(pose.get_num_bones())}
 rig=tools.create_asset('IK_EllisonRecovery'+label,destination,unreal.IKRigDefinition,unreal.IKRigDefinitionFactory())
 control=unreal.IKRigController.get_controller(rig);assert control.set_skeletal_mesh(mesh);assert control.set_retarget_root(hip)
 for chain in chains:
  assert chain[start] in bones and chain[end] in bones,chain
  assert str(control.add_retarget_chain(chain[0],chain[start],chain[end],'None'))==chain[0]
 rigs.append(rig)
retargeter=tools.create_asset('RTG_CityToEllison',destination,unreal.IKRetargeter,unreal.IKRetargetFactory())
control=unreal.IKRetargeterController.get_controller(retargeter)
control.set_ik_rig(unreal.RetargetSourceOrTarget.SOURCE,rigs[0]);control.set_ik_rig(unreal.RetargetSourceOrTarget.TARGET,rigs[1])
control.add_default_ops();control.auto_map_chains(unreal.AutoMapChainType.EXACT,True);control.auto_align_all_bones(unreal.RetargetSourceOrTarget.TARGET)
for asset in rigs+[retargeter]:assert unreal.EditorAssetLibrary.save_loaded_asset(asset,False)
registry=unreal.AssetRegistryHelpers.get_asset_registry();assets=[]
for side in ['F','B','L','R']:
 path='/Game/BattleRetarget/City/Male/M_ragdoll_getup_stand_'+side
 assets.append(registry.get_asset_by_object_path(path+'.'+path.rsplit('/',1)[1]))
assert all(a.is_valid() for a in assets)
inputs=unreal.IKRetargetBatchOperationInputs()
for key,value in [('assets_to_retarget',assets),('source_mesh',source),('target_mesh',target),('ik_retarget_asset',retargeter),('target_path',destination),('include_referenced_assets',False),('overwrite_existing_files',False)]:inputs.set_editor_property(key,value)
results=unreal.IKRetargetBatchOperation.run_batch_retarget(inputs);assert len(results)==4
rows=[]
for result in results:
 clip=result.get_asset();assert isinstance(clip,unreal.AnimSequence)
 unreal.AnimationLibrary.remove_all_animation_notify_tracks(clip)
 assert clip.get_editor_property('skeleton')==target.get_editor_property('skeleton')
 assert unreal.EditorAssetLibrary.save_loaded_asset(clip,False)
 rows.append({'asset':clip.get_path_name(),'duration':clip.get_editor_property('sequence_length')})
(root/'Tests/Results/2026-09-12-ellison-recovery-retarget.json').write_text(json.dumps({'clips':rows,'rigs':[r.get_path_name() for r in rigs],'visual_acceptance':False,'gameplay_installed':False},indent=2)+'\n')
print('ELLISON_RECOVERY_RETARGET_COMPLETE')
