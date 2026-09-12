"""Measure door-to-pavement height and support in the current exterior study."""
import json,math
from pathlib import Path
import unreal
root=Path(unreal.Paths.project_dir());folder=root/'SourceAssets/Terrain/IrwinBuildings'
assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/PiedmontIrwinSidewalkReview')
unreal.PiedmontWorldTools.finish_editor_asset_loading()
rows=[];buildings={b['osm_way']:b for b in json.loads((folder/'buildings.json').read_text())['buildings']}
for entry in json.loads((folder/'manifest.json').read_text())['entrances']:
 x,y,z=entry['xyz'];pts=buildings[entry['osm_way']]['footprint_xy'];nearest=None
 for a,b in zip(pts,pts[1:]+pts[:1]):
  dx,dy=b[0]-a[0],b[1]-a[1];l2=dx*dx+dy*dy
  if l2<.01:continue
  t=max(0,min(1,((x-a[0])*dx+(y-a[1])*dy)/l2));q=[a[0]+t*dx,a[1]+t*dy];d=math.hypot(x-q[0],y-q[1])
  if nearest is None or d<nearest[0]:nearest=(d,q)
 distance,q=nearest;assert distance>1
 ox,oy=(x-q[0])/distance,(y-q[1])/distance;samples=[]
 for d in [40,100,180,300,500]:
  px,py=x+ox*d,y+oy*d
  hit=unreal.PiedmontWorldTools.trace_world_surface(unreal.Vector(px,py,z+100),unreal.Vector(px,py,z-600))
  samples.append({'distance_cm':d,'xy':[px,py],'surface_z':hit[0].z if hit else None,'actor':hit[1].get_actor_label() if hit else None,'door_above_surface_cm':z-hit[0].z if hit else None})
 rows.append({'osm_way':entry['osm_way'],'door_xyz':entry['xyz'],'outward_xy':[ox,oy],'samples':samples})
assert all(abs(r['samples'][0]['door_above_surface_cm'])<.25 and r['samples'][0]['actor']=='Irwin buildings Landing' for r in rows),rows
(root/'Tests/Results/2026-09-12-irwin-entrance-landings.json').write_text(json.dumps({'passed':True,'entrances':rows,'main_map_changed':False,'scope':'Door threshold matches landing top at seven sampled centres. Full approach and walking traversal remain unverified.'},indent=2)+'\n')
