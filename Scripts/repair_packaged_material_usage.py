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
    assert isinstance(material, (unreal.Material, unreal.MaterialInstanceConstant)), f'Unsupported material: {path}'

lib = unreal.MaterialEditingLibrary
usage = unreal.MaterialUsage.MATUSAGE_NANITE
rows = []
for path, material in zip(paths, materials):
    before = lib.has_material_usage(material, usage)
    if not before:
        if isinstance(material, unreal.MaterialInstanceConstant):
            lib.set_material_usage_override(material, usage, True, True)
        else:
            lib.set_base_material_usage(material, usage, True)
        assert unreal.EditorAssetLibrary.save_loaded_asset(material), path
    assert lib.has_material_usage(material, usage), path
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
