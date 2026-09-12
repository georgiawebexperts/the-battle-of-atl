"""Create an isolated Mixamo-to-City sleeping candidate; no gameplay assignment."""
import unreal, json, pathlib
root = pathlib.Path(unreal.Paths.project_dir())
destination = '/Game/BattleRetarget/Mixamo/SleepCandidate'
tools = unreal.AssetToolsHelpers.get_asset_tools()
assert not unreal.EditorAssetLibrary.does_directory_exist(destination), 'Candidate exists; review before replacing'
source = unreal.load_asset('/Game/BattleRetarget/Mixamo/SleepingReference/MixamoSleepReference')
target = unreal.load_asset('/Game/CitySampleCrowd/Character/Male/NormalWeight/Meshes/m_tal_nrw_body')
assert source and target
rigs = []
for label, mesh in [('Source', source), ('Target', target)]:
    rig = tools.create_asset('IK_Sleep' + label, destination, unreal.IKRigDefinition, unreal.IKRigDefinitionFactory())
    control = unreal.IKRigController.get_controller(rig)
    assert control.set_skeletal_mesh(mesh)
    assert control.apply_auto_generated_retarget_definition(), label
    rigs.append(rig)
retargeter = tools.create_asset('RTG_SleepMixamoCity', destination, unreal.IKRetargeter, unreal.IKRetargetFactory())
control = unreal.IKRetargeterController.get_controller(retargeter)
control.set_ik_rig(unreal.RetargetSourceOrTarget.SOURCE, rigs[0])
control.set_ik_rig(unreal.RetargetSourceOrTarget.TARGET, rigs[1])
control.add_default_ops()
control.auto_map_chains(unreal.AutoMapChainType.EXACT, True)
control.auto_align_all_bones(unreal.RetargetSourceOrTarget.TARGET)
for asset in rigs + [retargeter]:
    assert unreal.EditorAssetLibrary.save_loaded_asset(asset, False)
path = '/Game/BattleRetarget/Mixamo/SleepingReference/MixamoSleepReference_Anim'
registry = unreal.AssetRegistryHelpers.get_asset_registry()
inputs = unreal.IKRetargetBatchOperationInputs()
for key, value in [('assets_to_retarget', [registry.get_asset_by_object_path(path + '.MixamoSleepReference_Anim')]), ('source_mesh', source), ('target_mesh', target), ('ik_retarget_asset', retargeter), ('target_path', destination), ('include_referenced_assets', False), ('overwrite_existing_files', False)]:
    inputs.set_editor_property(key, value)
results = unreal.IKRetargetBatchOperation.run_batch_retarget(inputs)
assert len(results) == 1, len(results)
clip = results[0].get_asset()
assert isinstance(clip, unreal.AnimSequence)
assert unreal.EditorAssetLibrary.save_loaded_asset(clip, False)
(root / 'Tests/Results/2026-09-11-sleep-retarget.json').write_text(json.dumps({'clip': clip.get_path_name(), 'duration': clip.get_editor_property('sequence_length'), 'rigs': [r.get_path_name() for r in rigs], 'retargeter': retargeter.get_path_name(), 'visual_acceptance': False, 'gameplay_installed': False}, indent=2) + '\n')
print('BATTLE_SLEEP_RETARGET_COMPLETE')
