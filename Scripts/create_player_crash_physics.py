"""Create a separate crash physics candidate; do not assign it to the live rider mesh."""
import unreal,json
from pathlib import Path
root=Path(unreal.Paths.project_dir());mesh=unreal.load_asset('/Game/PiedmontRide/Rider/Casual');before=mesh.get_editor_property('physics_asset')
asset=unreal.PiedmontWorldTools.create_player_crash_physics();assert asset
assert mesh.get_editor_property('physics_asset')==before
assert unreal.EditorAssetLibrary.save_loaded_asset(asset)
(root/'Tests/Results/2026-09-12-player-crash-physics-candidate-v3.json').write_text(json.dumps({'asset':asset.get_path_name(),'live_mesh_assignment_changed':False,'main_map_changed':False,'scope':'Anatomical capsule radii fitted in scaled bone space; auto-generated joint constraints retained. Physical stability, body clearance, mounted pose transfer, recovery and gameplay integration unverified.'},indent=2)+'\n')
