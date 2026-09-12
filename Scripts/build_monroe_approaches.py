"""Build extended Monroe pavement while preserving installed crossing surfaces."""
import pathlib,json,math,array,sys
from shapely.geometry import Polygon,LineString,Point,MultiPoint
from shapely.ops import unary_union,triangulate
from shapely import constrained_delaunay_triangles
from shapely.geometry.polygon import orient
root=pathlib.Path(__file__).resolve().parents[1];folder=root/'SourceAssets/Terrain/MonroeTraffic';existing=root/'SourceAssets/Terrain/TenthStreetGraded'
data=json.loads((folder/'approaches-network.json').read_text());pieces=[]
for row in data['roads']:
 pieces.append(LineString(row['points_cm']).buffer(int(row['tags'].get('lanes','3'))*150,cap_style=2,join_style=2))
# Buffering separate road ways with flat caps leaves wedges at bends.
# Join adjacent centreline segments at the shared minimum carriageway width.
ordered=sorted(data['roads'],key=lambda r:-max(p[1] for p in r['points_cm']))
for first,second in zip(ordered,ordered[1:]):
 a=first['points_cm'];b=second['points_cm'];a=a if a[0][1]>a[-1][1] else a[::-1];b=b if b[0][1]>b[-1][1] else b[::-1]
 if math.dist(a[-1],b[0])>.01:continue
 width=min(int(first['tags'].get('lanes','3')),int(second['tags'].get('lanes','3')))*150
 pieces.append(LineString([a[-2],a[-1],b[1]]).buffer(width,cap_style=2,join_style=2))
covered=[]
for path in [existing/f'TenthStreet_{name}.obj' for name in ['Road','RoadSeams','CycleTrack','Separator']]+[folder/'Monroe_RoadSeams.obj']:
 verts=[]
 for line in path.read_text().splitlines():
  p=line.split()
  if p and p[0]=='v':verts.append((float(p[1]),-float(p[2])))
  elif p and p[0]=='f':covered.append(Polygon([verts[int(v.split('/')[0])-1] for v in p[1:]]))
roads=unary_union(pieces).difference(unary_union(covered))
meta=json.loads((root/'SourceAssets/Terrain/terrain-georeference.json').read_text());raw=array.array('H');raw.frombytes((root/'SourceAssets/Terrain/atlanta-height-tenth-graded.r16').read_bytes())
if sys.byteorder!='little':raw.byteswap()
def height(x,y):
 sx,sy,sz=meta['unreal_scale'];lx,ly,_=meta['unreal_location_cm'];gx=(x-lx)/sx;gy=(-y-ly)/sy;ix=int(gx);iy=int(gy);dx=gx-ix;dy=gy-iy;w=meta['size'][0]
 a,b,c,d=[(raw[v*w+u]-32768)*sz/128 for u,v in [(ix,iy),(ix+1,iy),(ix,iy+1),(ix+1,iy+1)]]
 return (a+(b-a)*dx+(d-b)*dy if dx>=dy else a+(d-c)*dx+(c-a)*dy)
verts=[];sidefaces=[]
for row in (existing/'TenthStreet_Sidewalk.obj').read_text().splitlines():
 p=row.split()
 if p and p[0]=='v':verts.append((float(p[1]),-float(p[2])))
 elif p and p[0]=='f':sidefaces.append(Polygon([verts[int(v.split('/')[0])-1] for v in p[1:]]))
sidewalk=unary_union(sidefaces).difference(unary_union(pieces))
result=[]
for name,geometry in [('Approaches',roads),('SidewalkTrim',sidewalk)]:
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
   lift=20 if name=='Separator' else 18 if name=='SidewalkTrim' else 14
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
 verts=[p for f in faces for p in f];lines=['o Monroe_'+name]
 for x,y,z in verts:lines.append(f'v {x:.4f} {-y:.4f} {z:.4f}')
 for x,y,z in verts:lines.append(f'vt {x/200:.5f} {y/200:.5f}')
 for x,y,z in verts:
  nx=-(height(x+60,y)-height(x-60,y))/120;ny=(height(x,y+60)-height(x,y-60))/120;length=math.sqrt(nx*nx+ny*ny+1);lines.append(f'vn {nx/length:.6f} {ny/length:.6f} {1/length:.6f}')
 for i in range(0,len(verts),3):lines.append('f '+' '.join(f'{j}/{j}/{j}' for j in [i+3,i+2,i+1]))
 path=folder/('Monroe_'+name+'.obj');path.write_text('\n'.join(lines)+'\n');result.append({'file':path.name,'triangles':len(faces),'missing_area_cm2':missing,'bounds_cm':[[min(v[i] for v in verts) for i in range(3)],[max(v[i] for v in verts) for i in range(3)]]})
(folder/'approach-surfaces.json').write_text(json.dumps({'surfaces':result,'main_map_changed':False,'scope':'Extended Monroe pavement candidate; native collision and visual review pending.'},indent=2)+'\n');print(json.dumps(result))
