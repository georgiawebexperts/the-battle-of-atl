"""Install the reviewed crossing and visible signal, without test vehicles."""
import unreal,json,pathlib
root=pathlib.Path(unreal.Paths.project_dir())
assert json.loads((root/'Tests/Results/2026-09-12-native-signal-route.json').read_text())['passed']
assert json.loads((root/'Tests/Results/2026-09-12-native-signal-approach.json').read_text())['visual_accepted']
assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/PiedmontSignalCrossingReview')
ea=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
gate=next(a for a in ea.get_all_level_actors() if isinstance(a,unreal.BattleRoadCrossing));signal=next(a for a in ea.get_all_level_actors() if isinstance(a,unreal.BattleTrafficSignal))
gate_position=gate.get_actor_location();extent=gate.get_editor_property('CrossingArea').get_unscaled_box_extent();signal_position=signal.get_actor_location();signal_rotation=signal.get_actor_rotation()
assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/PiedmontWorld')
assert not any(a.actor_has_tag('TenthPiedmontBikeCrossing') for a in ea.get_all_level_actors()),'Crossing already installed'
unreal.PiedmontWorldTools.finish_editor_asset_loading()
hit=unreal.PiedmontWorldTools.trace_world_surface(signal_position+unreal.Vector(0,0,100),signal_position-unreal.Vector(0,0,100));assert hit and isinstance(hit[1],unreal.Landscape)
gate=ea.spawn_actor_from_class(unreal.BattleRoadCrossing,gate_position);gate.get_editor_property('CrossingArea').set_box_extent(extent);gate.set_editor_property('bAutoCycle',True);gate.set_actor_label('10th Street Piedmont bike crossing');gate.tags=[unreal.Name('TenthPiedmontBikeCrossing')]
signal=ea.spawn_actor_from_class(unreal.BattleTrafficSignal,signal_position,signal_rotation);signal.set_editor_property('Crossing',gate);signal.set_actor_label('10th Street westbound traffic signal');signal.tags=list(signal.tags)+[unreal.Name('TenthPiedmontBikeSignal')]
for a in [gate,signal]:a.set_folder_path('Midtown/Traffic')
assert unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).save_current_level()
(root/'Tests/Results/2026-09-12-signal-crossing-install.json').write_text(json.dumps({'main_map_changed':True,'crossing_tag':'TenthPiedmontBikeCrossing','signal_tag':'TenthPiedmontBikeSignal','cars_installed':False,'desktop_build_updated':False},indent=2)+'\n')
