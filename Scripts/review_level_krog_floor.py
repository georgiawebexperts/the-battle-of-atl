"""Test a full-width tunnel floor level with the existing concrete ride strip."""
import json
from pathlib import Path
import unreal
root=Path(unreal.Paths.project_dir())
assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/PiedmontKrogRoadReview')
ea=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
import sys
sys.path.insert(0,str(root/'Scripts'))
from krog_floor_tools import apply_level_floor
apply_level_floor(root,ea)
unreal.PiedmontWorldTools.finish_editor_asset_loading()
assert unreal.EditorLoadingAndSavingUtils.save_map(unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world(),'/Game/PiedmontRide/Maps/PiedmontKrogFloorReview')
(root/'Tests/Results/2026-09-12-krog-level-floor.json').write_text(json.dumps({'main_map_changed':False,'floor_lift_cm':12,'scope':'Isolated full-width floor lift with overlapping concrete clipped out. Side traversal, seam verification and visual review pending.'},indent=2)+'\n')
