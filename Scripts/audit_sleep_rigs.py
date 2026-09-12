"""Inspect authored bone hierarchies before choosing a sleeping retarget path."""
import json
import pathlib
import unreal

root = pathlib.Path(unreal.Paths.project_dir())
paths = {
    'mixamo': '/Game/BattleRetarget/Mixamo/SleepingReference/MixamoSleepReference',
    'punk': '/Game/BattleForTheA/Zombies/Punk/Punk/SkeletalMeshes/Punk',
    'farmer': '/Game/BattleForTheA/Zombies/Farmer/Farmer/SkeletalMeshes/Farmer',
    'city_male': '/Game/CitySampleCrowd/Character/Male/NormalWeight/Meshes/m_tal_nrw_body',
}
rows = {}
for label, path in paths.items():
    mesh = unreal.load_asset(path)
    assert mesh, path
    component = unreal.SkeletalMeshComponent()
    component.set_skeletal_mesh_asset(mesh)
    bones = []
    for index in range(component.get_num_bones()):
        name = component.get_bone_name(index)
        bones.append({'name': str(name), 'parent': str(component.get_parent_bone(name))})
    assert bones, label
    rows[label] = {'mesh': path, 'bones': bones}
report = root / 'Tests/Results/2026-09-11-sleep-rig-audit.json'
report.write_text(json.dumps({'rigs': rows, 'retarget_acceptance': False}, indent=2) + '\n')
print('BATTLE_SLEEP_RIG_AUDIT_COMPLETE', {key: len(row['bones']) for key, row in rows.items()})
