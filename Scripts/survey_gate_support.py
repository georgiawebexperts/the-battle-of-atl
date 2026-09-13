"""Measure terrain beneath the runtime 14th Street gate without changing the map."""
import unreal,json
from pathlib import Path
root=Path(unreal.Paths.project_dir())
assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/PiedmontWorld')
unreal.PiedmontWorldTools.finish_editor_asset_loading()
rows=[]
for side in [-1,1]:
 for i in range(8):
  x,y,z=-17126.01578+100+i*35,-5089.65167+side*350,117.65686
  hit=unreal.PiedmontWorldTools.trace_world_surface(unreal.Vector(x,y,z+1000),unreal.Vector(x,y,z-2000))
  assert hit
  rows.append(dict(side=side,index=i,x=x,y=y,ground_z=hit[0].z,old_base_z=z,gap_cm=z-hit[0].z,surface=hit[1].get_actor_label()))
(root/'Tests/Results/2026-09-13-gate-support-survey.json').write_text(json.dumps(rows,indent=2)+'\n')

# Keep the compiled support heights reproducible from this survey.
header="#pragma once\n// Native pavement survey, 2026-09-13 [codex-maclaptop]. Regenerate after gate terrain edits.\nnamespace BattleGateSupport {\ninline constexpr float BaseZ[2][8] = {\n"
for side in [-1,1]:
 header+=" {"+", ".join(f"{r['ground_z']:.5f}f" for r in rows if r['side']==side)+"},\n"
header+="};\n}\n"
(root/'Source/AuraPlayground/BattleGateSupport.h').write_text(header)
