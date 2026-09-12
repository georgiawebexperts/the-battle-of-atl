"""Read current ground/roof heights along mapped railway lines."""
import json,math
from pathlib import Path
import unreal
root=Path(unreal.Paths.project_dir())
assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/PiedmontKrogShellReview')
unreal.PiedmontWorldTools.finish_editor_asset_loading()
network=json.loads((root/'SourceAssets/Terrain/KrogRailContext/network.json').read_text())
rows=[]
for rail in network['railways']:
 samples=[];distance=0.
 for a,b in zip(rail['points_xy_cm'],rail['points_xy_cm'][1:]):
  length=math.dist(a,b);count=max(1,math.ceil(length/100))
  for i in range(count):
   t=i/count;x=a[0]+t*(b[0]-a[0]);y=a[1]+t*(b[1]-a[1])
   hit=unreal.PiedmontWorldTools.trace_world_surface(unreal.Vector(x,y,3000),unreal.Vector(x,y,-1000))
   samples.append({'distance_cm':distance+t*length,'xy_cm':[x,y],'surface_z_cm':hit[0].z if hit else None,'actor':hit[1].get_actor_label() if hit else None})
  distance+=length
 rows.append({'osm_way':rail['osm_way'],'samples':samples})
report={'railways':rows,'main_map_changed':False,'scope':'Current native surface heights, not surveyed rail elevations. High traces may hit tunnel roof or terrain. Rail profile and deck/earthwork not yet authored.'}
(root/'Tests/Results/2026-09-12-krog-rail-support-survey.json').write_text(json.dumps(report,indent=2)+'\n')
