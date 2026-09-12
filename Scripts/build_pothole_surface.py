"""Build a locally positioned worn surface on the physically depressed road."""
import json,math
from pathlib import Path
from shapely.geometry import Polygon
from shapely.geometry.polygon import orient
from shapely import constrained_delaunay_triangles
root=Path(__file__).resolve().parents[1];folder=root/'SourceAssets/Terrain/IrwinTraffic';site=json.loads((folder/'pothole-site.json').read_text());cx,cy,cz=site['xyz']
ring=[]
for i in range(80):
 a=i*math.tau/80;r=site['radius_cm']*.9*(1+.08*math.sin(5*a)+.05*math.cos(7*a));ring.append((cx+r*math.cos(a),cy+r*math.sin(a)))
shape=Polygon(ring);vertices=[];faces=[]
for line in (folder/'Irwin_PotholeRoad.obj').read_text().splitlines():
 p=line.split()
 if p and p[0]=='v':vertices.append((float(p[1]),-float(p[2]),float(p[3])))
 elif p and p[0]=='f':
  a,b,c=[vertices[int(s.split('/')[0])-1] for s in p[1:]];patch=Polygon([a[:2],b[:2],c[:2]]).intersection(shape)
  if patch.area<1e-5:continue
  det=(b[0]-a[0])*(c[1]-a[1])-(b[1]-a[1])*(c[0]-a[0])
  def z(x,y):
   u=((x-a[0])*(c[1]-a[1])-(y-a[1])*(c[0]-a[0]))/det;v=((b[0]-a[0])*(y-a[1])-(b[1]-a[1])*(x-a[0]))/det
   return a[2]+u*(b[2]-a[2])+v*(c[2]-a[2])+.15
  for t in constrained_delaunay_triangles(patch).geoms:
   if t.area>1e-5:faces.append([(x-cx,y-cy,z(x,y)-cz) for x,y in list(orient(t,sign=1).exterior.coords)[:3]])
vs=[p for f in faces for p in f];lines=['o Pothole_Surface']+[f'v {x:.6f} {-y:.6f} {z:.6f}' for x,y,z in vs]+[f'vt {x/200+.5:.6f} {y/200+.5:.6f}' for x,y,z in vs]
lines.append('vn 0 0 1')
for i in range(0,len(vs),3):lines.append('f '+' '.join(f'{j}/{j}/1' for j in [i+3,i+2,i+1]))
(folder/'Pothole_Surface.obj').write_text('\n'.join(lines)+'\n');print('surface triangles',len(faces))
