"""Verify actual constructor geometry against the saved native pavement survey."""
import unreal,json
from pathlib import Path
root=Path(unreal.Paths.project_dir())
assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/PiedmontWorld')
unreal.PiedmontWorldTools.finish_editor_asset_loading()
ea=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
a=ea.spawn_actor_from_class(unreal.BattleTutorial,unreal.Vector())
c=next(c for c in a.get_components_by_class(unreal.InstancedStaticMeshComponent) if c.get_name()=='IronAndRoof')
rows=[]
for r in json.loads((root/'Tests/Results/2026-09-13-gate-support-survey.json').read_text()):
 hits=[]
 for i in range(c.get_instance_count()):
  t=c.get_instance_transform(i,world_space=True);p=t.translation;s=t.scale3d
  if abs(p.x-r['x'])<.05 and abs(p.y-r['y'])<.05 and s.z>2:
   hits.append(p.z-s.z*50)
 assert hits,r
 actual=unreal.PiedmontWorldTools.trace_world_surface(unreal.Vector(r['x']+12,r['y']+12,500),unreal.Vector(r['x']+12,r['y']+12,-500))
 assert actual
 gap=max(hits)-actual[0].z
 rows.append(dict(side=r['side'],index=r['index'],base_z=max(hits),adjacent_ground_z=actual[0].z,gap_cm=gap))
 assert -8<gap<0,rows[-1]
(root/'Tests/Results/2026-09-13-gate-support-native.json').write_text(json.dumps({'passed':True,'posts':rows,'scope':'16 gate uprights touch surveyed pavement; all-world geometry not covered'},indent=2)+'\n')
ea.destroy_actor(a)
