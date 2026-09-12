"""Export Epic recovery clips to the two City bodies in the external staging project."""
import unreal,json,pathlib
out=pathlib.Path('/Volumes/Adam Assets/Unreal/Projects/AuraPlayground/work/city-recovery-retarget.json')
source=unreal.load_asset('/Game/Characters/UEFN_Mannequin/Meshes/SKM_UEFN_Mannequin');assert source
retargeter=unreal.load_asset('/Game/MetaHumans/Common/Common/Rigs/RTG_UEFN_to_Metahuman_nrw');assert retargeter
registry=unreal.AssetRegistryHelpers.get_asset_registry()
clips=['/Game/Characters/UEFN_Mannequin/Animations/Ragdoll/M_ragdoll_getup_stand_'+side for side in ['F','B','L','R']]
assets=[registry.get_asset_by_object_path(p+'.'+p.rsplit('/',1)[1]) for p in clips]
assert all(a.is_valid() for a in assets)
rows=[]
for sex,prefix in [('Male','m_tal_nrw'),('Female','f_tal_nrw')]:
 target=unreal.load_asset('/Game/CitySampleCrowd/Character/'+sex+'/NormalWeight/Meshes/'+prefix+'_body');assert target
 destination='/Game/BattleRetarget/City/'+sex
 assert not unreal.EditorAssetLibrary.does_directory_exist(destination),'Review existing exports before rerunning'
 inputs=unreal.IKRetargetBatchOperationInputs()
 for name,value in [('assets_to_retarget',assets),('source_mesh',source),('target_mesh',target),('ik_retarget_asset',retargeter),('target_path',destination),('include_referenced_assets',False),('overwrite_existing_files',False)]:inputs.set_editor_property(name,value)
 results=unreal.IKRetargetBatchOperation.run_batch_retarget(inputs)
 assert len(results)==4,(sex,len(results))
 for item in results:
  clip=item.get_asset();assert isinstance(clip,unreal.AnimSequence)
  unreal.AnimationLibrary.remove_all_animation_notify_tracks(clip)
  assert unreal.EditorAssetLibrary.save_loaded_asset(clip,False)
  rows.append({'sex':sex,'path':clip.get_path_name(),'skeleton':clip.get_editor_property('skeleton').get_path_name(),'length':clip.get_editor_property('sequence_length')})
out.write_text(json.dumps({'source':source.get_path_name(),'retargeter':retargeter.get_path_name(),'exports':rows,'status':'exported; visual and runtime acceptance pending'},indent=2)+'\n')
print('BATTLE_RECOVERY_RETARGET_COMPLETE',len(rows))
