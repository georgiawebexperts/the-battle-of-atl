"""Measure imported forefoot joints and loafer bounds without saving a map."""
import unreal,json
from pathlib import Path
root=Path(unreal.Paths.project_dir());base='/Game/CitySampleCrowd/Character/Male/NormalWeight/Meshes/m_tal_nrw_'
ea=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
a=ea.spawn_actor_from_class(unreal.SkeletalMeshActor,unreal.Vector())
c=a.skeletal_mesh_component;c.set_skinned_asset_and_update(unreal.load_asset(base+'body'))
r={}
for bone in ['foot_l','ball_l','foot_r','ball_r']:
 p=c.get_socket_location(bone);r[bone]=[p.x,p.y,p.z]
 assert abs(p.x)>1
bounds=unreal.load_asset(base+'loafers').get_imported_bounds()
r['sole_min_z_cm']=bounds.origin.z-bounds.box_extent.z
r['author']='2026-09-13 [codex-maclaptop]'
(root/'Tests/Results/2026-09-13-pedal-shoe-survey.json').write_text(json.dumps(r,indent=2)+'\n')
ea.destroy_actor(a)
