"""Native transverse floor survey below the tunnel ceiling."""
import json,math
from pathlib import Path
import unreal
root=Path(unreal.Paths.project_dir())
assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/PiedmontKrogRoadReview')
unreal.PiedmontWorldTools.finish_editor_asset_loading()
points=json.loads((root/'SourceAssets/Terrain/KrogPortalCandidate/manifest.json').read_text())['roof_samples']
rows=[];actors={}
for i in range(1,len(points)-1,2):
 p=points[i];a=points[i-1];b=points[i+1]
 dx,dy=b[0]-a[0],-(b[1]-a[1]);length=math.hypot(dx,dy);dx/=length;dy/=length
 samples=[]
 for offset in range(-400,701,25):
  x,y=p[0]-dy*offset,-p[1]+dx*offset
  hit=unreal.PiedmontWorldTools.trace_world_surface(unreal.Vector(x,y,p[2]+100),unreal.Vector(x,y,p[2]-150))
  label=hit[1].get_actor_label() if hit else None
  actors[label]=actors.get(label,0)+1
  samples.append({'offset_cm':offset,'xyz':[x,y,hit[0].z if hit else None],'actor':label,'above_profile_cm':hit[0].z-p[2] if hit else None})
 rows.append({'station_index':i,'profile_z':p[2],'samples':samples})
level=json.loads((root/'SourceAssets/Terrain/KrogTraffic/road-surfaces.json').read_text()).get('level_tunnel_floor',False)
deep=[s for row in rows if row['station_index']>=21 for s in row['samples'] if -300<=s['offset_cm']<=600]
flat=all(s['above_profile_cm'] is not None and abs(s['above_profile_cm'])<.25 for s in deep)
name='2026-09-12-krog-level-merged-width.json' if level else '2026-09-12-krog-floor-width.json'
(root/'Tests/Results'/name).write_text(json.dumps({'actors':actors,'stations':rows,'deep_floor_samples':len(deep),'deep_floor_matches_profile':flat,'scope':'Native floor traces below ceiling, not a full body clearance test.','main_map_changed':False},indent=2)+'\n')
if level:assert flat,'Deep tunnel floor differs from authored level profile'
