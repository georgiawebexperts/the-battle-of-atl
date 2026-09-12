"""Apply the native Mac-reviewed tree renderer and bounded texture sizes.
Only modifies the migrated game copy; the external Fab source remains untouched.
"""
import json
from pathlib import Path
import unreal

root = Path(unreal.Paths.project_dir())
manifest = json.loads((root / 'SourceAssets/Manifests/epic-tree-migration.json').read_text())
rows = []
for path in manifest['roots']:
    mesh = unreal.load_asset(path)
    assert isinstance(mesh, unreal.StaticMesh), path
    settings = mesh.get_editor_property('nanite_settings')
    before = bool(settings.get_editor_property('enabled'))
    settings.set_editor_property('enabled', False)
    mesh.set_editor_property('nanite_settings', settings)
    assert not mesh.get_editor_property('nanite_settings').get_editor_property('enabled')
    assert unreal.EditorAssetLibrary.save_loaded_asset(mesh), path
    rows.append({'path': path, 'nanite_before': before, 'nanite_enabled': False})
textures = []
for path in manifest['packages']:
    if not path.startswith('/Game/EuropeanHornbeam/Textures/'):
        continue
    texture = unreal.load_asset(path)
    assert isinstance(texture, unreal.Texture2D), path
    # Preserve leaf atlas detail close to the rider; lower-cost bark and distant impostors.
    cap = 4096 if '/TwoSided/' in path else 2048
    before = int(texture.get_editor_property('max_texture_size'))
    texture.set_editor_property('max_texture_size', cap)
    assert unreal.EditorAssetLibrary.save_loaded_asset(texture), path
    textures.append({'path': path, 'previous_cap': before, 'max_texture_size': cap})
unreal.PiedmontWorldTools.finish_editor_asset_loading()
(root / 'work/epic-tree-mac-settings.json').write_text(json.dumps({
    'meshes': rows, 'textures': textures, 'map_saved': False,
    'scope': 'Saved renderer and texture settings only; fresh-process render and gameplay performance pending'
}, indent=2) + '\n')
print('BATTLE_TREE_MAC_SETTINGS_SAVED', len(rows), len(textures))
