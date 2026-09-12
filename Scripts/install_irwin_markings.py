"""Copy visually reviewed Irwin lane paint into the main world."""
import json
from pathlib import Path
import unreal
root=Path(unreal.Paths.project_dir())
assert json.loads((root/'Tests/Results/2026-09-12-irwin-paint-render.json').read_text()).get('paint_accepted_for_integration')
assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/PiedmontIrwinCrossingReview')
ea=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
rows=[(a.get_actor_label(),a.static_mesh_component.static_mesh.get_path_name(),a.get_actor_transform()) for a in ea.get_all_level_actors() if a.actor_has_tag('IrwinPaintReview')];assert len(rows)==2
assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/PiedmontWorld')
assert any(a.actor_has_tag('IrwinRoadTraffic') for a in ea.get_all_level_actors())
for a in ea.get_all_level_actors():
 if a.actor_has_tag('IrwinPaint'):ea.destroy_actor(a)
for label,path,transform in rows:
 a=ea.spawn_actor_from_class(unreal.StaticMeshActor,transform.translation);a.set_actor_transform(transform,False,False);a.set_actor_label(label);a.tags=[unreal.Name('IrwinPaint')];a.set_folder_path('Eastside/IrwinLake')
 a.static_mesh_component.set_static_mesh(unreal.load_asset(path));a.static_mesh_component.set_collision_profile_name('NoCollision')
 assert str(a.static_mesh_component.get_collision_profile_name())=='NoCollision'
assert unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).save_current_level()
(root/'Tests/Results/2026-09-12-irwin-paint-install.json').write_text(json.dumps({'main_map_changed':True,'paint_actors':2,'collision':'NoCollision','desktop_build_updated':False,'scope':'Road centre and edge lines; buildings, signage, potholes and full streetscape unfinished.'},indent=2)+'\n')
