"""Import the CC0 Quaternius SMG and report its native asset bounds.

Elliott: "cant you find assets like you did last time?" - the same route the
rifle came in on. Quaternius Zombie Apocalypse Kit (March 2024), CC0, public
mirror agentkaerf/FreeModels. Slot 2 used to borrow the rifle's prop because
there was no authored machine gun mesh; this is that mesh.
"""
import json
import pathlib

import unreal

root = pathlib.Path(unreal.Paths.project_dir())
t = unreal.AssetImportTask()
t.filename = str(root / "SourceAssets/Weapons/SMG.gltf")
t.destination_path = "/Game/BattleForTheA/Weapons/SMG"
t.destination_name = "SMG"
t.automated = True
t.save = True
t.replace_existing = True
unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([t])
rows = []
for p in unreal.EditorAssetLibrary.list_assets(t.destination_path, recursive=True, include_folder=False):
    a = unreal.load_asset(p)
    if isinstance(a, unreal.StaticMesh):
        b = a.get_bounding_box()
        rows.append({"path": p, "min": str(b.min), "max": str(b.max)})
unreal.EditorAssetLibrary.save_directory(t.destination_path, only_if_is_dirty=False, recursive=True)
(root / "work/smg-import.json").write_text(json.dumps(rows, indent=2) + "\n")
assert len(rows) == 1, rows
unreal.SystemLibrary.quit_editor()
