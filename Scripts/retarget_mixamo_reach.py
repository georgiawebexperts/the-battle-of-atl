"""Create an isolated Mixamo-to-City reach candidate; no gameplay assignment."""
import unreal, json, pathlib
root = pathlib.Path(unreal.Paths.project_dir())
destination = '/Game/BattleRetarget/Mixamo/ReachCandidate'
tools = unreal.AssetToolsHelpers.get_asset_tools()
assert not unreal.EditorAssetLibrary.does_directory_exist(destination), 'Candidate exists; review before replacing'
source = unreal.load_asset('/Game/BattleRetarget/Mixamo/ReachReference/MixamoReachReference')
target = unreal.load_asset('/Game/CitySampleCrowd/Character/Male/NormalWeight/Meshes/m_tal_nrw_body')
assert source and target
retargeter = unreal.load_asset('/Game/BattleRetarget/Mixamo/SleepCandidate/RTG_SleepMixamoCity')
assert retargeter
rigs=[]
path = '/Game/BattleRetarget/Mixamo/ReachReference/MixamoReachReference_Anim'
registry = unreal.AssetRegistryHelpers.get_asset_registry()
inputs = unreal.IKRetargetBatchOperationInputs()
for key, value in [('assets_to_retarget', [registry.get_asset_by_object_path(path + '.MixamoReachReference_Anim')]), ('source_mesh', source), ('target_mesh', target), ('ik_retarget_asset', retargeter), ('target_path', destination), ('include_referenced_assets', False), ('overwrite_existing_files', False)]:
    inputs.set_editor_property(key, value)
results = unreal.IKRetargetBatchOperation.run_batch_retarget(inputs)
assert len(results) == 1, len(results)
clip = results[0].get_asset()
assert isinstance(clip, unreal.AnimSequence)
assert unreal.EditorAssetLibrary.save_loaded_asset(clip, False)
(root / 'Tests/Results/2026-09-12-reach-retarget.json').write_text(json.dumps({'clip': clip.get_path_name(), 'duration': clip.get_editor_property('sequence_length'), 'rigs': [r.get_path_name() for r in rigs], 'retargeter': retargeter.get_path_name(), 'visual_acceptance': False, 'gameplay_installed': False}, indent=2) + '\n')
print('BATTLE_REACH_RETARGET_COMPLETE')
