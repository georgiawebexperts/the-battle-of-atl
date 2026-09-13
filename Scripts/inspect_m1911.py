"""Inspect externally staged free M1911 geometry and dependency closure; no saves."""
import unreal,json
from pathlib import Path
r=unreal.AssetRegistryHelpers.get_asset_registry();r.search_all_assets(synchronous_search=True)
seeds=['/Game/M1911/Meshes/NonRigged_M1911','/Game/M1911/Meshes/NonRigged_M1911_Magazine','/Game/M1911/Meshes/SkeletalMesh/Rigged_M1911']
seen=set();pending=list(seeds)
while pending:
 p=pending.pop()
 if p in seen:continue
 seen.add(p)
 pending += [str(x) for x in r.get_dependencies(p,unreal.AssetRegistryDependencyOptions()) if str(x).startswith('/Game/')]
world=unreal.EditorLoadingAndSavingUtils.new_blank_map(False);ea=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
rows=[]
for p in seeds:
 a=unreal.load_asset(p);assert a
 row={'path':p,'class':a.get_class().get_name()}
 if isinstance(a,unreal.StaticMesh):
  b=a.get_bounding_box();row['min']=[b.min.x,b.min.y,b.min.z];row['max']=[b.max.x,b.max.y,b.max.z];row['materials']=[str(x.material_interface.get_path_name()) for x in a.static_materials]
 else:
  row['materials']=[{'slot':str(x.material_slot_name),'material':x.material_interface.get_path_name() if x.material_interface else None} for x in a.materials]
  actor=ea.spawn_actor_from_class(unreal.SkeletalMeshActor,unreal.Vector());c=actor.skeletal_mesh_component;c.set_skeletal_mesh_asset(a);bones=[]
  for i in range(c.get_num_bones()):
   n=c.get_bone_name(i);t=c.get_socket_transform(n,unreal.RelativeTransformSpace.RTS_COMPONENT);v=t.translation;q=t.rotation;bones.append({'name':str(n),'parent':str(c.get_parent_bone(n)),'position':[v.x,v.y,v.z],'rotation':[q.x,q.y,q.z,q.w]})
  row['bones']=bones;ea.destroy_actor(actor)
 rows.append(row)
Path('/Volumes/Adam Assets/Unreal/Projects/AuraPlayground/Tests/Results/2026-09-13-m1911-inspect.json').write_text(json.dumps({'assets':rows,'dependencies':sorted(seen)},indent=2)+'\n')
