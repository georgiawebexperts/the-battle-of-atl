"""Measure authored pavement against a provisional two-way car envelope.
This geometry audit does not substitute for native collision or actual vehicle bounds.
"""
import json,math
from pathlib import Path
from shapely.geometry import LineString,Polygon
from shapely.ops import unary_union
root=Path(__file__).resolve().parents[1]
folder=root/'SourceAssets/Terrain/TenthStreet'
vertices=[];polygons=[]
for line in (folder/'TenthStreet_Road.obj').read_text().splitlines():
 fields=line.split()
 if fields and fields[0]=='v':vertices.append((float(fields[1]),-float(fields[2])))
 elif fields and fields[0]=='f':polygons.append(Polygon([vertices[int(v.split('/')[0])-1] for v in fields[1:]]))
pavement=unary_union(polygons)
network=json.loads((folder/'network.json').read_text())
rows=[]
for road in network['roads']:
 path=LineString([p[:2] for p in road['points_cm']])
 if path.length<400:continue
 for i in range(1,max(2,math.ceil(path.length/1000))):
  d=path.length*i/max(2,math.ceil(path.length/1000));p=path.interpolate(d);a=path.interpolate(d-20);b=path.interpolate(d+20)
  dx=b.x-a.x;dy=b.y-a.y;n=math.hypot(dx,dy);nx=-dy/n;ny=dx/n
  cut=LineString([(p.x-nx*1500,p.y-ny*1500),(p.x+nx*1500,p.y+ny*1500)]).intersection(pavement)
  lengths=[g.length for g in cut.geoms] if hasattr(cut,'geoms') else [cut.length]
  width=max(lengths,default=0)
  rows.append({'osm_way':road['osm_way'],'xy_cm':[round(p.x,2),round(p.y,2)],'widest_contiguous_pavement_cm':round(width,2),'fits_two_300cm_lanes':width>=600})
footprints=json.loads((root/'SourceAssets/Terrain/FancyRoachMotel/footprints.json').read_text())
blocking=unary_union([Polygon([p[:2] for p in b['footprint_world_cm']]) for b in footprints['buildings']])
result={'vehicle_width_assumption_cm':200,'clearance_each_side_cm':50,'two_way_required_width_cm':600,'samples':rows,'insufficient_width_samples':sum(not r['fits_two_300cm_lanes'] for r in rows),'apartment_footprint_road_overlap_m2':round(pavement.intersection(blocking).area/10000,3),'accepted':False,'scope':'Planar authored geometry only. Actual vehicle dimensions and native collision still required.'}
(root/'Tests/Results/2026-09-11-tenth-street-clearance.json').write_text(json.dumps(result,indent=2)+'\n')
print(json.dumps({k:v for k,v in result.items() if k!='samples'}))
