"""Repair Monroe road joins without covering the mapped cycle track."""
import pathlib,json,math,array,sys
from shapely.geometry import Polygon,MultiPoint
from shapely.ops import unary_union
from shapely import constrained_delaunay_triangles
from shapely.geometry.polygon import orient
root=pathlib.Path(__file__).resolve().parents[1];folder=root/'SourceAssets/Terrain/TenthStreetGraded'
def mesh_polygons(name):
 verts=[];faces=[]
 for line in (folder/f'TenthStreet_{name}.obj').read_text().splitlines():
  p=line.split()
  if p and p[0]=='v':verts.append((float(p[1]),-float(p[2])))
  elif p and p[0]=='f':faces.append(Polygon([verts[int(v.split('/')[0])-1] for v in p[1:]]))
 return unary_union(faces)
road=mesh_polygons('Road');protected=unary_union([mesh_polygons('CycleTrack'),mesh_polygons('Separator')])
rows=json.loads((folder/'network.json').read_text())['monroe_roads'];rows.sort(key=lambda r:-max(p[1] for p in r['points_cm']))
patches=[]
for first,second in zip(rows,rows[1:]):
 a=first['points_cm'];b=second['points_cm'];a=a if a[0][1]>a[-1][1] else a[::-1];b=b if b[0][1]>b[-1][1] else b[::-1]
 assert math.dist(a[-1][:2],b[0][:2])<1
 site=a[-1];width=min(int(first['tags'].get('lanes','3')),int(second['tags'].get('lanes','3')))*150;corners=[]
 for p,q in [(a[-2],a[-1]),(b[0],b[1])]:
  dx,dy=q[0]-p[0],q[1]-p[1];l=math.hypot(dx,dy);dx/=l;dy/=l
  for along in [-20,20]:
   for side in [-width,width]:corners.append((site[0]+dx*along-dy*side,site[1]+dy*along+dx*side))
 patches.append(MultiPoint(corners).convex_hull.difference(road).difference(protected))
patch=unary_union(patches);assert patch.area<25000,patch.area
meta=json.loads((root/'SourceAssets/Terrain/terrain-georeference.json').read_text());raw=array.array('H');raw.frombytes((root/'SourceAssets/Terrain/atlanta-height-tenth-graded.r16').read_bytes())
if sys.byteorder!='little':raw.byteswap()
def height(x,y):
 sx,sy,sz=meta['unreal_scale'];lx,ly,_=meta['unreal_location_cm'];gx=(x-lx)/sx;gy=(-y-ly)/sy;ix=int(gx);iy=int(gy);dx=gx-ix;dy=gy-iy;w=meta['size'][0]
 a,b,c,d=[(raw[v*w+u]-32768)*sz/128 for u,v in [(ix,iy),(ix+1,iy),(ix,iy+1),(ix+1,iy+1)]]
 return (a+(b-a)*dx+(d-b)*dy if dx>=dy else a+(d-c)*dx+(c-a)*dy)+14
faces=[]
# Split into small patches so the repair follows the graded terrain.
from shapely.geometry import box
x0,y0,x1,y1=patch.bounds
for x in range(math.floor(x0/80)*80,math.ceil(x1/80)*80,80):
 for y in range(math.floor(y0/80)*80,math.ceil(y1/80)*80,80):
  clipped=patch.intersection(box(x,y,x+80,y+80))
  if clipped.area<.0001:continue
  for tri in constrained_delaunay_triangles(clipped).geoms:
   if tri.area>.0001 and clipped.buffer(.000001).covers(tri):faces.append([(px,py,height(px,py)) for px,py in list(orient(tri,sign=1).exterior.coords)[:3]])
coverage=unary_union([Polygon([p[:2] for p in f]) for f in faces]);assert patch.difference(coverage.buffer(.001)).area<1,(patch.area,patch.difference(coverage.buffer(.001)).area)
verts=[v for f in faces for v in f];lines=['o TenthStreet_RoadSeams']+[f'v {x:.5f} {-y:.5f} {z:.5f}' for x,y,z in verts]
lines += [f'vt {x/200:.5f} {y/200:.5f}' for x,y,z in verts]
for i in range(0,len(verts),3):lines.append('f '+' '.join(f'{j}/{j}' for j in [i+3,i+2,i+1]))
(root/'SourceAssets/Terrain/MonroeTraffic/Monroe_RoadSeams.obj').write_text('\n'.join(lines)+'\n')
(root/'SourceAssets/Terrain/MonroeTraffic/road-seams.json').write_text(json.dumps({'author':'2026-09-12 [codex-maclaptop]','area_cm2':patch.area,'triangles':len(faces),'protected_cycle_overlap_cm2':patch.intersection(protected).area,'installed':False},indent=2)+'\n');print(patch.area,len(faces))
