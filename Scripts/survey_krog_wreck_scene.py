"""Read-only ground and clearance survey for the requested rare scooter-help scene."""
import unreal,json,math
from pathlib import Path
root=Path(unreal.Paths.project_dir()).resolve()
assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/PiedmontWorld')
unreal.PiedmontWorldTools.finish_editor_asset_loading()
samples=json.loads((root/'SourceAssets/Terrain/KrogOutwardShell/manifest.json').read_text())['roof_samples'];a,b=samples[:2]
a=[a[0],-a[1],a[2]];b=[b[0],-b[1],b[2]];dx,dy=b[0]-a[0],b[1]-a[1];length=math.hypot(dx,dy);dx/=length;dy/=length
rows=[]
for back in (1100,1500,1900,2300):
 for side in (-1100,-850,-600,600,850,1100):
  x,y=a[0]-dx*back-dy*side,a[1]-dy*back+dx*side
  points=[]
  for ox,oy in ((0,0),(-180,-140),(-180,140),(180,-140),(180,140)):
   h=unreal.PiedmontWorldTools.trace_world_surface(unreal.Vector(x+ox,y+oy,a[2]+5000),unreal.Vector(x+ox,y+oy,a[2]-600))
   points.append({'xy':[x+ox,y+oy],'z':h[0].z if h else None,'actor':h[1].get_actor_label() if h else None})
  heights=[p['z'] for p in points if p['z'] is not None]
  rows.append({'back_from_portal_cm':back,'lateral_offset_cm':side,'center_xy':[x,y],'samples':points,'height_spread_cm':max(heights)-min(heights) if len(heights)==5 else None})
roads=json.loads((root/'SourceAssets/Terrain/KrogTraffic/network.json').read_text())['roads']
def distance(p,a,b):
 dx,dy=b[0]-a[0],b[1]-a[1];den=dx*dx+dy*dy;t=max(0,min(1,((p[0]-a[0])*dx+(p[1]-a[1])*dy)/den)) if den else 0
 return math.hypot(p[0]-a[0]-dx*t,p[1]-a[1]-dy*t)
for row in rows:
 clearances=[]
 for road in roads:
  width=900 if 'DeKalb' in road['tags'].get('name','') else 600
  clearances.append(min(distance(row['center_xy'],a,b) for a,b in zip(road['points_cm'],road['points_cm'][1:]))-width/2-math.hypot(180,140))
 row['conservative_road_edge_clearance_cm']=round(min(clearances),2)
eligible=[row for row in rows if row['height_spread_cm'] is not None and row['height_spread_cm']<35 and row['conservative_road_edge_clearance_cm']>100 and all('USGS' in (p['actor'] or '') for p in row['samples'])]
preferred=min(eligible,key=lambda row:row['height_spread_cm']) if eligible else None
r={'source':'Current main-world north Krog portal and native traces','candidates':rows,'preferred_candidate':({'back_from_portal_cm':preferred['back_from_portal_cm'],'lateral_offset_cm':preferred['lateral_offset_cm']} if preferred else None),'scope':'Ground survey only. Source-road-width bounding-circle clearance included; native lane/body clearance, pedestrian nav, pose footprint and rarity behavior not yet verified. No actors or map changes.'}
(root/'Tests/Results/2026-09-12-krog-wreck-site-survey.json').write_text(json.dumps(r,indent=2)+'\n')
