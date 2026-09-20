"""Import Ellison's three hurt takes for in-world playback.

Elliott, 2026-09-19: "when you are getting attacked Ellison needs to at least
scream or say stop". The bike loads S_RiderHurt1/2/3 by path in BeginPlay and
rotates them, so a missing take is silence rather than a broken build - but
silence is the exact complaint, which is why this asserts the duration of each
one the way the police-line import does.
"""
import json
import unreal
from pathlib import Path

root = Path(unreal.Paths.project_dir())
results = {}
for take in ("S_RiderHurt1", "S_RiderHurt2", "S_RiderHurt3"):
    task = unreal.AssetImportTask()
    task.filename = str(root / f"SourceAssets/Audio/{take}.wav")
    task.destination_path = "/Game/BattleForTheA/Audio"
    task.destination_name = take
    task.automated = True
    task.replace_existing = True
    task.save = True
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    sound = unreal.load_asset(f"/Game/BattleForTheA/Audio/{take}")
    assert sound, f"{take} did not import"
    duration = sound.get_editor_property("duration")
    assert 0.6 < duration < 2.0, f"{take} is {duration:.2f}s, outside the bark window"
    results[take] = {"asset": sound.get_path_name(), "duration": duration, "passed": True}

(root / "work/rider-voice-import.json").write_text(json.dumps(results, indent=2))
print(json.dumps(results, indent=2))
