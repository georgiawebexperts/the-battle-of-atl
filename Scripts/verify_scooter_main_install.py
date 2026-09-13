"""Read the saved main map in a fresh editor process after scooter/traffic promotion."""
import unreal,json
from pathlib import Path
root=Path(unreal.Paths.project_dir()).resolve()
assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/PiedmontWorld')
unreal.PiedmontWorldTools.finish_editor_asset_loading();ea=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
actors=ea.get_all_level_actors();scenes=[a for a in actors if isinstance(a,unreal.BattleScooterScene)];patches=[a for a in actors if a.actor_has_tag('ScooterNavigation')];directors=[a for a in actors if isinstance(a,unreal.BattleRoadTrafficDirector) and a.actor_has_tag('KrogTrafficReview')]
assert len(scenes)==len(patches)==len(directors)==1
scene=scenes[0];assert abs(scene.get_editor_property('AppearanceChance')-.18)<.001
component=patches[0].get_component_by_class(unreal.HierarchicalInstancedStaticMeshComponent);assert component and component.get_instance_count()==236
assert component.get_collision_enabled()==unreal.CollisionEnabled.NO_COLLISION and not component.is_visible()
lanes=directors[0].get_editor_property('Lanes');assert len(lanes)==1 and lanes[0].get_editor_property('bLoopRoute') and len(lanes[0].get_editor_property('Crossings'))==2
survey=json.loads((root/'Tests/Results/2026-09-13-scooter-navigation-survey.json').read_text());corridor=next(c for c in survey['corridors'] if c['clear_ground_corridor'])
a=unreal.PiedmontWorldTools.project_park_navigation(scene.get_actor_location());b=unreal.PiedmontWorldTools.project_park_navigation(unreal.Vector(*corridor['end']));length=unreal.PiedmontWorldTools.park_route_length(a,b) if a and b else -1
assert length>0,length
report={'passed':True,'scene_count':len(scenes),'appearance_chance':scene.get_editor_property('AppearanceChance'),'navigation_tiles':component.get_instance_count(),'route_length_cm':length,'looping_traffic_lanes':len(lanes),'crossing_bindings':len(lanes[0].get_editor_property('Crossings')),'scope':'Fresh-process saved main scene/settings, hidden noncolliding navigation tiles and native navigation query. Runtime normal spawning, main ride and packaged acceptance remain separate.'}
(root/'Tests/Results/2026-09-13-scooter-main-fresh-read.json').write_text(json.dumps(report,indent=2)+'\n')
