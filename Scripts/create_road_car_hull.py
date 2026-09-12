"""Create a body-shaped collision candidate from the bundled sports car mesh."""
import unreal,json,pathlib
root=pathlib.Path(unreal.Paths.project_dir());dest='/Game/BattleForTheA/Traffic/SM_RoadCarHull'
mesh=unreal.EditorAssetLibrary.duplicate_asset('/Game/Vehicles/SportsCar/SM_SportsCar',dest) if not unreal.EditorAssetLibrary.does_asset_exist(dest) else unreal.load_asset(dest)
assert mesh
s=unreal.get_editor_subsystem(unreal.StaticMeshEditorSubsystem);assert s
assert s.set_convex_decomposition_collisions(mesh,4,24,100000)
unreal.EditorAssetLibrary.save_loaded_asset(mesh)
r={'convex_hulls':s.get_convex_collision_count(mesh),'simple_shapes':s.get_simple_collision_count(mesh),'asset':dest,'source':'Bundled UE sports car body; original asset unchanged','runtime_accepted':False}
(root/'Tests/Results/2026-09-12-road-car-hull.json').write_text(json.dumps(r,indent=2)+'\n')
unreal.SystemLibrary.quit_editor()
