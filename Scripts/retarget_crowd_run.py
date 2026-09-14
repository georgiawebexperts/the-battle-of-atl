"""Run in external GameAnimationSample; export dedicated male/female crowd runs."""
import unreal,json,pathlib,shutil,hashlib
root=pathlib.Path('/Volumes/Adam Assets/Unreal/Projects/AuraPlayground')
source=unreal.load_asset('/Game/Characters/UEFN_Mannequin/Meshes/SKM_UEFN_Mannequin')
retargeter=unreal.load_asset('/Game/MetaHumans/Common/Common/Rigs/RTG_UEFN_to_Metahuman_nrw');assert source and retargeter
r=unreal.AssetRegistryHelpers.get_asset_registry();r.search_all_assets(synchronous_search=True)
path='/Game/Characters/UEFN_Mannequin/Animations/Run/M_Neutral_Run_Loop_F'
asset=r.get_asset_by_object_path(path+'.M_Neutral_Run_Loop_F');assert asset.is_valid()
rows=[]
for gender,prefix in [('Male','m_tal_nrw'),('Female','f_tal_nrw')]:
 target=unreal.load_asset('/Game/CitySampleCrowd/Character/'+gender+'/NormalWeight/Meshes/'+prefix+'_body');assert target
 dest='/Game/BattleRetarget/CrowdRun/'+gender
 assert not unreal.EditorAssetLibrary.does_directory_exist(dest),'Do not overwrite existing exports'
 i=unreal.IKRetargetBatchOperationInputs()
 for k,v in [('assets_to_retarget',[asset]),('source_mesh',source),('target_mesh',target),('ik_retarget_asset',retargeter),('target_path',dest),('include_referenced_assets',False),('overwrite_existing_files',False)]:i.set_editor_property(k,v)
 exports=unreal.IKRetargetBatchOperation.run_batch_retarget(i);assert len(exports)==1
 clip=exports[0].get_asset();assert isinstance(clip,unreal.AnimSequence)
 unreal.AnimationLibrary.remove_all_animation_notify_tracks(clip);assert unreal.EditorAssetLibrary.save_loaded_asset(clip,False)
 package=clip.get_path_name().split('.')[0];rel=package.removeprefix('/Game/')+'.uasset'
 src=pathlib.Path(unreal.Paths.project_content_dir())/rel;dst=root/'Content'/rel
 assert src.is_file() and not dst.exists();dst.parent.mkdir(parents=True,exist_ok=True);shutil.copy2(src,dst)
 rows.append({'gender':gender,'path':clip.get_path_name(),'target':target.get_path_name(),'skeleton':clip.get_editor_property('skeleton').get_path_name(),'file':str(dst),'sha256':hashlib.sha256(dst.read_bytes()).hexdigest()})
(root/'Tests/Results/2026-09-14-crowd-run-retarget.json').write_text(json.dumps({'exports':rows,'scope':'Retargeted animations; native visual acceptance pending'},indent=2)+'\n')
print('CrowdRun: exported',len(rows))
