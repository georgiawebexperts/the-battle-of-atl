"""Compare retained path candidates with the installed collision surface."""
import unreal,json
from pathlib import Path
root=Path(unreal.Paths.project_dir())
assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/PiedmontWorld')
unreal.PiedmontWorldTools.finish_editor_asset_loading()
rows=[]
for route in json.loads((root/'work/hill-candidates.json').read_text()):
 samples=[]
 for i in range(13):
  t=i/12;p=[a+(b-a)*t for a,b in zip(route['start'],route['end'])]
  hit=unreal.PiedmontWorldTools.trace_world_surface(unreal.Vector(p[0],p[1],2000),unreal.Vector(p[0],p[1],-2500))
  samples.append(dict(x=p[0],y=p[1],source_z=p[2],z=hit[0].z if hit else None,surface=hit[1].get_actor_label() if hit else None))
 rows.append(dict(route=route,samples=samples))
(root/'Tests/Results/2026-09-13-hill-route-survey.json').write_text(json.dumps(rows,indent=2)+'\n')
