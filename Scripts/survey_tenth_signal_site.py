"""Survey support and path clearance for a crossing signal pole."""
import unreal,pathlib,json
root=pathlib.Path(unreal.Paths.project_dir());unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/PiedmontWorld');unreal.PiedmontWorldTools.finish_editor_asset_loading();rows=[]
for x in [-20400,-20600,-20800,-21000]:
 for y in [12000,12050,12100,12150,12200,13650,13700,13750]:
  samples=[]
  for dx,dy in [(0,0),(-20,-20),(20,-20),(-20,20),(20,20)]:
   hit=unreal.PiedmontWorldTools.trace_world_surface(unreal.Vector(x+dx,y+dy,3000),unreal.Vector(x+dx,y+dy,-3000));samples.append(None if not hit else {'z':hit[0].z,'actor':hit[1].get_actor_label()})
  rows.append({'xy':[x,y],'samples':samples})
(root/'Tests/Results/2026-09-12-signal-site-survey.json').write_text(json.dumps(rows,indent=2)+'\n')
