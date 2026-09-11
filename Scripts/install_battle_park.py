"""Enable V3 riding on the sourced park. Does not accept the final gate/landmarks/route."""
import unreal,pathlib,json
root=pathlib.Path(unreal.Paths.project_dir());ea=unreal.get_editor_subsystem(unreal.EditorActorSubsystem);level=unreal.get_editor_subsystem(unreal.LevelEditorSubsystem);w=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
if w.get_name()!='PiedmontWorld':raise RuntimeError('Park integration requires PiedmontWorld')
w.get_world_settings().set_editor_property('default_game_mode',unreal.BattleParkMode)
points=json.loads((root/'SourceAssets/Terrain/battle-start.json').read_text());pos=unreal.Vector(*points['start_xy_cm'],0);hit=unreal.PiedmontWorldTools.trace_world_surface(pos+unreal.Vector(0,0,2000),pos-unreal.Vector(0,0,2000),2)
if not hit or not hit[1].actor_has_tag('RidePath'):raise RuntimeError('Development start has no verified path surface')
starts=[a for a in ea.get_all_level_actors() if isinstance(a,unreal.PlayerStart)];start=starts[0] if starts else ea.spawn_actor_from_class(unreal.PlayerStart,unreal.Vector())
start.set_actor_label('14th Street gate — V3 park start');start.set_actor_location_and_rotation(hit[0]+unreal.Vector(0,0,102),unreal.Rotator(yaw=points['heading_yaw']),False,True)
report={'map':w.get_name(),'mode':'BattleParkMode','saved':level.save_current_level(),'start':str(start.get_actor_location()),'scope':'Existing sourced park with V3 controls; 14th Street placement sourced; gate architecture and full world/game incomplete.'}
(root/'Scripts/battle-park-install.json').write_text(json.dumps(report,indent=2))
