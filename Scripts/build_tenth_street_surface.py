"""Terrain-following road/cycle surfaces for native placement review."""
import runpy,json,math
from pathlib import Path
from shapely.geometry import LineString,MultiPoint,Point,Polygon
from shapely.ops import unary_union,triangulate
from shapely import constrained_delaunay_triangles
from shapely.geometry.polygon import orient
root=Path(__file__).resolve().parents[1];ns=runpy.run_path(str(root/'Scripts/prepare_tenth_street.py'));data=ns['result'];height=ns['height'];folder=ns['folder']
# Geography is compressed, but rider/vehicle bodies use gameplay scale. Keep
# mapped cycle alignment and expand motor lanes south, away from the park and
# apartment footprints. Lane counts are retained at 300 cm per motor lane.
road_pieces=[]
for row in data['roads']:
 points=[p[:2] for p in row['points_cm']]
 if points[0][0]>points[-1][0]:points.reverse()
 road_pieces.append(LineString(points).buffer(int(row['tags'].get('lanes','3'))*300,single_sided=True,join_style=2))
for row in data['monroe_roads']:
 road_pieces.append(LineString([p[:2] for p in row['points_cm']]).buffer(int(row['tags'].get('lanes','3'))*150,cap_style=2,join_style=2))
roads=unary_union(road_pieces)
cycles=unary_union([LineString([p[:2] for p in row['points_cm']]).buffer(80,cap_style=2,join_style=2) for row in data['cycle_track']])
# Keep the mapped cycle track distinct; motor-vehicle paving never covers it.
junction=Point(data['monroe_crossing']['world_cm'][:2]).buffer(500)
crossings=unary_union([LineString([p[:2] for p in row['points_cm']]).buffer(120) for row in data['cycle_track'] if row['tags'].get('cycleway')=='crossing']+[junction])
separator=cycles.buffer(25).difference(cycles).intersection(roads.buffer(70)).difference(crossings)
road_union=roads
roads=roads.difference(cycles).difference(separator)
sidewalk=road_union.buffer(160).difference(road_union.buffer(25)).difference(cycles.buffer(30)).difference(crossings)
# Existing apartment walls are not sidewalk space.
footprints=json.loads((root/'SourceAssets/Terrain/FancyRoachMotel/footprints.json').read_text())
no_walk=unary_union([Polygon([p[:2] for p in b['footprint_world_cm']]).buffer(20) for b in footprints['buildings']])
sidewalk=sidewalk.difference(no_walk)
result=[]
for name,geometry in [('Road',roads),('CycleTrack',cycles),('Separator',separator),('Sidewalk',sidewalk)]:
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
  def terrain_face(coords,depth=0):
   lift=20 if name=='Separator' else 18 if name=='Sidewalk' else 14
   points=[[x,y,height(x,y)+lift] for x,y in coords]
   samples=[(.5,.5,0),(.5,0,.5),(0,.5,.5),(1/3,1/3,1/3)]
   clearance=min(sum(w*p[2] for w,p in zip(weights,points))-height(*[sum(w*p[k] for w,p in zip(weights,points)) for k in range(2)]) for weights in samples)
   if clearance<5 and depth<5:
    a,b,c=coords;ab=tuple((a[k]+b[k])/2 for k in range(2));bc=tuple((b[k]+c[k])/2 for k in range(2));ca=tuple((c[k]+a[k])/2 for k in range(2))
    for sub in [(a,ab,ca),(ab,b,bc),(ca,bc,c),(ab,bc,ca)]:terrain_face(sub,depth+1)
   else:
    assert clearance>0,(name,clearance,coords)
    faces.append(points)
  for tri in triangulate(MultiPoint(sites)):
   # Clipping must retain boundary-crossing triangle portions. Dropping the
   # entire triangle leaves holes at curved/nonconvex road edges.
   clipped=tri.intersection(poly)
   if clipped.area>1e-6:
    for part in constrained_delaunay_triangles(clipped).geoms:
     if part.area>1e-6:terrain_face(list(orient(part,sign=1).exterior.coords)[:3])
 coverage=unary_union([Polygon([p[:2] for p in f]) for f in faces]);missing=geometry.difference(coverage.buffer(.001)).area
 assert missing<1,(name,missing)
 verts=[p for f in faces for p in f];lines=['o TenthStreet_'+name]
 for x,y,z in verts:lines.append(f'v {x:.4f} {-y:.4f} {z:.4f}')
 for x,y,z in verts:lines.append(f'vt {x/200:.5f} {y/200:.5f}')
 for x,y,z in verts:
  nx=-(height(x+60,y)-height(x-60,y))/120;ny=(height(x,y+60)-height(x,y-60))/120;length=math.sqrt(nx*nx+ny*ny+1);lines.append(f'vn {nx/length:.6f} {ny/length:.6f} {1/length:.6f}')
 for i in range(0,len(verts),3):lines.append('f '+' '.join(f'{j}/{j}/{j}' for j in [i+3,i+2,i+1]))
 path=folder/('TenthStreet_'+name+'.obj');path.write_text('\n'.join(lines)+'\n');result.append({'file':path.name,'triangles':len(faces),'missing_area_cm2':missing,'bounds_cm':[[min(v[i] for v in verts) for i in range(3)],[max(v[i] for v in verts) for i in range(3)]]})
(folder/'surfaces.json').write_text(json.dumps({'surfaces':result,'status':'Generated only; native import, width, continuity, collision and landscape clearance review pending.','road_lane_width_cm':300,'cycle_track_width_cm':160,'sources':'Mapped cycle alignment and road topology; motor lanes expanded south to gameplay scale. Native vehicle and intersection review pending.'},indent=2)+'\n');print(json.dumps(result))
