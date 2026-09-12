"""Regenerate derived water geometry after saving the canopy under a new map name."""
import json,unreal
from pathlib import Path
root=Path(unreal.Paths.project_dir())
assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/PiedmontDetailedCanopyReview')
ea=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
zones=[a for a in ea.get_all_level_actors() if isinstance(a,unreal.WaterZone)];lakes=[a for a in ea.get_all_level_actors() if isinstance(a,unreal.WaterBodyLake)];assert len(zones)==1 and len(lakes)==1
lake=lakes[0];component=lake.get_component_by_class(unreal.WaterBodyLakeComponent);component.set_water_zone_override(zones[0]);assert unreal.PiedmontWorldTools.refresh_water_body(lake)
unreal.PiedmontWorldTools.rebuild_water_zones();unreal.PiedmontWorldTools.finish_editor_asset_loading()
assert unreal.EditorLoadingAndSavingUtils.save_map(unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world(),'/Game/PiedmontRide/Maps/PiedmontDetailedCanopyReview')
(root/'Tests/Results/2026-09-12-canopy-review-water-refresh.json').write_text(json.dumps({'main_map_changed':False,'lakes_refreshed':1,'zones_rebound':1,'runtime_water_visible':False,'scope':'Derived geometry regeneration after SaveAs; runtime appearance must be checked.'},indent=2)+'\n')
