"""Inspect Ellison's riding skeleton and candidate animation compatibility."""
import unreal,json
from pathlib import Path
root=Path(unreal.Paths.project_dir());mesh=unreal.load_asset('/Game/PiedmontRide/Rider/Casual');assert mesh
component=unreal.PoseableMeshComponent();component.set_skinned_asset_and_update(mesh)
physics=mesh.get_editor_property('physics_asset');skeleton=mesh.get_editor_property('skeleton');bounds=mesh.get_bounds()
clips=[]
for path in unreal.EditorAssetLibrary.list_assets('/Game/PiedmontRide/Rider/Animations',recursive=True,include_folder=False):
 clip=unreal.load_asset(path)
 if isinstance(clip,unreal.AnimSequence):clips.append({'path':path,'seconds':clip.get_editor_property('sequence_length'),'same_skeleton':clip.get_editor_property('skeleton')==skeleton})
r={'riding_mesh':mesh.get_path_name(),'skeleton':skeleton.get_path_name(),'physics_asset':physics.get_path_name() if physics else None,'bones':[str(component.get_bone_name(i)) for i in range(component.get_num_bones())],'bounds_extent':[bounds.box_extent.x,bounds.box_extent.y,bounds.box_extent.z],'clips':clips,'scope':'Asset compatibility only. No player crash state or physics integration changed.'}
(root/'Tests/Results/2026-09-12-player-crash-assets.json').write_text(json.dumps(r,indent=2)+'\n');print(json.dumps(r))
