"""Run in external GameAnimationSample; create dedicated City Ellison motion clips."""
import unreal,json,pathlib,hashlib,shutil
root=pathlib.Path('/Volumes/Adam Assets/Unreal/Projects/AuraPlayground')
source=unreal.load_asset('/Game/Characters/UEFN_Mannequin/Meshes/SKM_UEFN_Mannequin');target=unreal.load_asset('/Game/CitySampleCrowd/Character/Male/NormalWeight/Meshes/m_tal_nrw_body');retargeter=unreal.load_asset('/Game/MetaHumans/Common/Common/Rigs/RTG_UEFN_to_Metahuman_nrw');assert source and target and retargeter
names=['Jump/M_Relaxed_Jump_F_Land_Stand_Light_Lfoot','Jump/M_Relaxed_Jump_F_Land_Run_Light_Lfoot']
r=unreal.AssetRegistryHelpers.get_asset_registry();r.search_all_assets(synchronous_search=True)
paths=['/Game/Characters/UEFN_Mannequin/Animations/'+n for n in names]
assets=[r.get_asset_by_object_path(p+'.'+p.rsplit('/',1)[1]) for p in paths];assert all(a.is_valid() for a in assets)
dest='/Game/BattleRetarget/Ellison/CityLanding';assert not unreal.EditorAssetLibrary.does_directory_exist(dest),'Do not overwrite previous exports'
i=unreal.IKRetargetBatchOperationInputs()
for k,v in [('assets_to_retarget',assets),('source_mesh',source),('target_mesh',target),('ik_retarget_asset',retargeter),('target_path',dest),('include_referenced_assets',False),('overwrite_existing_files',False)]:i.set_editor_property(k,v)
exports=unreal.IKRetargetBatchOperation.run_batch_retarget(i);assert len(exports)==len(names)
rows=[]
for item in exports:
 clip=item.get_asset();assert isinstance(clip,unreal.AnimSequence);unreal.AnimationLibrary.remove_all_animation_notify_tracks(clip);assert unreal.EditorAssetLibrary.save_loaded_asset(clip,False)
 rows.append({'path':clip.get_path_name(),'skeleton':clip.get_editor_property('skeleton').get_path_name(),'length':clip.get_editor_property('sequence_length')})
(root/'Tests/Results/2026-09-13-detailed-landing-retarget.json').write_text(json.dumps({'source':source.get_path_name(),'target':target.get_path_name(),'retargeter':retargeter.get_path_name(),'clips':rows,'scope':'Retargeted copies only; not integrated or visually accepted.'},indent=2)+'\n')
print('DetailedLocomotion: exported',len(rows))
