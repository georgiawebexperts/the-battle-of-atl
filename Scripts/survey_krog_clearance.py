"""Probe below the authored roof to distinguish roadway floors from shell hits."""
import json,pathlib,unreal
r=pathlib.Path(unreal.Paths.project_dir());d=json.loads((r/'Tests/Results/2026-09-12-krog-crossing-survey.json').read_text())
assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/PiedmontWorld');unreal.PiedmontWorldTools.finish_editor_asset_loading();rows=[]
for sample in d['road_samples']:
 if 'Shell' not in (sample['actor'] or ''):continue
 x,y,z=sample['xyz'];probes=[]
 for height in [40,100,180,240,400]:
  hit=unreal.PiedmontWorldTools.trace_world_surface(unreal.Vector(x,y,z+height),unreal.Vector(x,y,z-250))
  probes.append({'start_above_plan_cm':height,'hit_z':hit[0].z if hit else None,'actor':hit[1].get_actor_label() if hit else None})
 rows.append({'sample':sample,'probes':probes})
(r/'Tests/Results/2026-09-12-krog-clearance-survey.json').write_text(json.dumps({'main_map_changed':False,'sites':rows,'scope':'Layered vertical traces only. Does not establish car-width clearance or traversable grade.'},indent=2)+'\n')
