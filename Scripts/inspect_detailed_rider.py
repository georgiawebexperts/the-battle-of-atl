"""Inspect the already-installed City Sample male rig without changing saved assets."""
import unreal,json
from pathlib import Path
root=Path(unreal.Paths.project_dir()).resolve();assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/PiedmontWorld');unreal.PiedmontWorldTools.finish_editor_asset_loading()
ea=unreal.get_editor_subsystem(unreal.EditorActorSubsystem);actor=ea.spawn_actor_from_class(unreal.SkeletalMeshActor,unreal.Vector());c=actor.skeletal_mesh_component
path='/Game/CitySampleCrowd/Character/Male/NormalWeight/Meshes/m_tal_nrw_body';mesh=unreal.load_asset(path);assert mesh;c.set_skeletal_mesh_asset(mesh)
rows=[]
for i in range(c.get_num_bones()):
 name=c.get_bone_name(i);t=c.get_socket_transform(name,unreal.RelativeTransformSpace.RTS_COMPONENT);p=t.translation;q=t.rotation
 rows.append({'name':str(name),'parent':str(c.get_parent_bone(name)),'position':[p.x,p.y,p.z],'rotation':[q.x,q.y,q.z,q.w]})
r={'mesh':path,'bone_count':len(rows),'bones':rows,'scope':'Live native reference-pose bone transforms; transient actor only, no save.'}
(root/'Tests/Results/2026-09-13-detailed-rider-rig.json').write_text(json.dumps(r,indent=2)+'\n');ea.destroy_actor(actor)
