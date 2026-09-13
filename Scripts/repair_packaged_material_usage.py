"""Persist Nanite usage for assets named in the packaged runtime fallback warnings.

Run in UnrealEditor-Cmd before cooking; this does not change any map or graph.
"""
import json
from datetime import datetime, timezone
from pathlib import Path
import unreal

paths = [
    '/Game/PiedmontRide/Materials/M_ParkConcreteWorld',
    '/Game/PiedmontRide/Materials/M_ParkAsphaltWorld',
    '/Game/BattleForTheA/Weapons/Rifle/Rifle/Materials/Atlas',
]
# Resolve and validate all assets before changing any of them.
materials = [unreal.load_asset(path) for path in paths]
for path, material in zip(paths, materials):
    assert isinstance(material, unreal.Material), f'Expected base material: {path}'

rows = []
for path, material in zip(paths, materials):
    before = bool(material.get_editor_property('used_with_nanite'))
    if not before:
        material.set_editor_property('used_with_nanite', True)
        unreal.MaterialEditingLibrary.recompile_material(material)
        assert unreal.EditorAssetLibrary.save_loaded_asset(material), path
    assert material.get_editor_property('used_with_nanite'), path
    rows.append({'material': path, 'previously_enabled': before, 'nanite_usage': True})

unreal.PiedmontWorldTools.finish_editor_asset_loading()
root = Path(unreal.Paths.project_dir())
stamp = datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%SZ')
report = root / 'Tests/Results' / f'material-usage-repair-{stamp}.json'
report.write_text(json.dumps({
    'materials': rows,
    'scope': 'Persisted material usage only; fresh cook/runtime fallback and visual checks still required.',
    'packaged_verified': False,
}, indent=2) + '\n')
print(report)
