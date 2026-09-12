"""Read-only native terrain samples at the fictional bungalow foundation and porch."""
import unreal,re,json
from pathlib import Path
root=Path(unreal.Paths.project_dir()).resolve()
h=[float(v) for v in re.search(r'Home\(([^)]+)',(root/'Source/AuraPlayground/BattleTutorialData.h').read_text()).group(1).split(',')]
assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/PiedmontWorld')
rows=[]
for y in (-750,-700,-650,-600,-550,-500,-425,0,425):
 for x in (-325,-160,0,160,325):
  hit=unreal.PiedmontWorldTools.trace_world_surface(unreal.Vector(h[0]+x,h[1]+y,h[2]+1000),unreal.Vector(h[0]+x,h[1]+y,h[2]-1000))
  rows.append({'offset':[x,y],'height_relative_home':hit[0].z-h[2] if hit else None,'actor':hit[1].get_actor_label() if hit else None})
r={'home':h,'samples':rows,'scope':'Ground survey; no map mutation.'}
(root/'Tests/Results/2026-09-12-tutorial-house-ground.json').write_text(json.dumps(r,indent=2)+'\n')
