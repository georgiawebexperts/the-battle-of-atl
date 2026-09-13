"""Retarget the existing authored pointing stance to the detailed gunman; isolated copy."""
import unreal,json,pathlib
root=pathlib.Path(unreal.Paths.project_dir())
dest='/Game/BattleRetarget/Gunman/AimCandidate'
assert not unreal.EditorAssetLibrary.does_directory_exist(dest)
tools=unreal.AssetToolsHelpers.get_asset_tools()
source=unreal.load_asset('/Game/PiedmontRide/Rider/Casual')
target=unreal.load_asset('/Game/CitySampleCrowd/Character/Female/NormalWeight/Meshes/f_tal_nrw_body')
assert source and target
chains=[('Spine','Abdomen','Chest','spine_01','spine_05'),('Head','Neck','Head','neck_01','head')]
for side in ['L','R']:
 s=side.lower()
 chains += [(side+'Arm','UpperArm_'+side,'Hand_'+side,'upperarm_'+s,'hand_'+s),(side+'Leg','UpperLeg_'+side,'Foot_'+side,'thigh_'+s,'foot_'+s)]
rigs=[]
for idx,(name,mesh) in enumerate([('Source',source),('Target',target)]):
 rig=tools.create_asset('IK_Gunman'+name,dest,unreal.IKRigDefinition,unreal.IKRigDefinitionFactory());c=unreal.IKRigController.get_controller(rig);assert c.set_skeletal_mesh(mesh)
 assert c.set_retarget_root('Hips' if idx==0 else 'pelvis')
 for row in chains:
  result=c.add_retarget_chain(row[0],row[1+idx*2],row[2+idx*2],'None');assert str(result)!='None',row
 rigs.append(rig)
rt=tools.create_asset('RTG_GunmanAim',dest,unreal.IKRetargeter,unreal.IKRetargetFactory());c=unreal.IKRetargeterController.get_controller(rt)
c.set_ik_rig(unreal.RetargetSourceOrTarget.SOURCE,rigs[0]);c.set_ik_rig(unreal.RetargetSourceOrTarget.TARGET,rigs[1]);c.add_default_ops();c.auto_map_chains(unreal.AutoMapChainType.EXACT,True);c.auto_align_all_bones(unreal.RetargetSourceOrTarget.TARGET)
for a in rigs+[rt]:assert unreal.EditorAssetLibrary.save_loaded_asset(a,False)
path='/Game/PiedmontRide/Rider/Animations/Animations_CharacterArmature_Idle_Gun_Pointing'
r=unreal.AssetRegistryHelpers.get_asset_registry();r.search_all_assets(synchronous_search=True)
i=unreal.IKRetargetBatchOperationInputs()
for k,v in [('assets_to_retarget',[r.get_asset_by_object_path(path+'.'+path.rsplit('/',1)[1])]),('source_mesh',source),('target_mesh',target),('ik_retarget_asset',rt),('target_path',dest),('include_referenced_assets',False),('overwrite_existing_files',False)]:i.set_editor_property(k,v)
exports=unreal.IKRetargetBatchOperation.run_batch_retarget(i);assert len(exports)==1
clip=exports[0].get_asset();assert unreal.EditorAssetLibrary.save_loaded_asset(clip,False)
(root/'Tests/Results/2026-09-13-gunman-aim-retarget.json').write_text(json.dumps({'clip':clip.get_path_name(),'duration':clip.get_editor_property('sequence_length'),'visual_acceptance':False},indent=2)+'\n')
print('GUNMAN_AIM_RETARGET_COMPLETE')
