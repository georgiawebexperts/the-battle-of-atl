"""Terrain-following road/cycle surfaces for native placement review."""
import runpy,json,math
from pathlib import Path
from shapely.geometry import LineString,MultiPoint,Point
from shapely.ops import unary_union,triangulate
root=Path(__file__).resolve().parents[1];ns=runpy.run_path(str(root/'Scripts/prepare_tenth_street.py'));data=ns['result'];height=ns['height'];folder=root/'SourceAssets/Terrain/TenthStreet'
# Road widths are an initial playable interpretation of mapped lane counts in
# this compressed world. Native width/vehicle clearance review remains required.
roads=unary_union([LineString([p[:2] for p in row['points_cm']]).buffer(int(row['tags'].get('lanes','3'))*95,cap_style=2,join_style=2) for row in data['roads']])
cycles=unary_union([LineString([p[:2] for p in row['points_cm']]).buffer(80,cap_style=2,join_style=2) for row in data['cycle_track']])
# Keep the mapped cycle track distinct; motor-vehicle paving never covers it.
roads=roads.difference(cycles.buffer(25));result=[]
for name,geometry in [('Road',roads),('CycleTrack',cycles)]:
 polygons=list(geometry.geoms) if geometry.geom_type=='MultiPolygon' else [geometry];faces=[]
 for poly in polygons:
  sites=[]
  for ring in [poly.exterior,*poly.interiors]:
   line=LineString(ring.coords)
   for i in range(math.ceil(line.length/80)+1):sites.append(line.interpolate(min(i*80,line.length)).coords[0])
   sites+=list(ring.coords)
  x0,y0,x1,y1=poly.bounds
  for x in range(math.floor(x0/120)*120,math.ceil(x1/120)*120+1,120):
   for y in range(math.floor(y0/120)*120,math.ceil(y1/120)*120+1,120):
    if poly.contains(Point(x,y)):sites.append((x,y))
  for tri in triangulate(MultiPoint(sites)):
   if poly.buffer(.01).covers(tri):faces.append([[x,y,height(x,y)+14] for x,y in list(tri.exterior.coords)[:3]])
 verts=[p for f in faces for p in f];lines=['o TenthStreet_'+name]
 for x,y,z in verts:lines.append(f'v {x:.4f} {-y:.4f} {z:.4f}')
 for x,y,z in verts:lines.append(f'vt {x/200:.5f} {y/200:.5f}')
 for i in range(0,len(verts),3):lines.append('f '+' '.join(f'{j}/{j}' for j in [i+3,i+2,i+1]))
 path=folder/('TenthStreet_'+name+'.obj');path.write_text('\n'.join(lines)+'\n');result.append({'file':path.name,'triangles':len(faces),'bounds_cm':[[min(v[i] for v in verts) for i in range(3)],[max(v[i] for v in verts) for i in range(3)]]})
(folder/'surfaces.json').write_text(json.dumps({'surfaces':result,'status':'Generated only; native import, width, continuity, collision and landscape clearance review pending.','road_lane_width_cm':190,'cycle_track_width_cm':160,'sources':'Mapped centerlines. Widths authored for first review in compressed geography.'},indent=2)+'\n');print(json.dumps(result))
