"""Author non-colliding lane paint on the reviewed Krog/DeKalb pavement."""
import json,math
from pathlib import Path
from shapely.geometry import LineString,Polygon,Point
from shapely.ops import unary_union,linemerge
from shapely import constrained_delaunay_triangles
from shapely.geometry.polygon import orient
root=Path(__file__).resolve().parents[1];folder=root/'SourceAssets/Terrain/KrogTraffic'
network=json.loads((folder/'network.json').read_text());white=[];yellow=[]
clear=Point(network['crossing_xyz'][:2]).buffer(600)
for name in ['DeKalb Avenue Northeast','Krog Street Northeast']:
 lines=[LineString([p[:2] for p in row['points_cm']]) for row in network['roads'] if row['tags']['name']==name and row['tags'].get('highway')!='service']
 merged=linemerge(lines)
 for path in ([merged] if merged.geom_type=='LineString' else merged.geoms):
  for offset in [-10,10]:yellow.append(path.offset_curve(offset,join_style=2).buffer(5,cap_style=2))
  for offset in ([-400,400] if name=='DeKalb Avenue Northeast' else [-250,250]):white.append(path.offset_curve(offset,join_style=2).buffer(5,cap_style=2))
paints={'WhitePaint':unary_union(white).difference(clear),'YellowPaint':unary_union(yellow).difference(clear)}
triangles=[];verts=[]
for line in (folder/'Krog_Road.obj').read_text().splitlines():
 p=line.split()
 if p and p[0]=='v':verts.append((float(p[1]),-float(p[2]),float(p[3])))
 elif p and p[0]=='f':triangles.append([verts[int(v.split('/')[0])-1] for v in p[1:]])
results=[]
for name,paint in paints.items():
 faces=[]
 for points in triangles:
  patch=Polygon([p[:2] for p in points]).intersection(paint)
  if patch.area<1e-6:continue
  a,b,c=points;det=(b[0]-a[0])*(c[1]-a[1])-(c[0]-a[0])*(b[1]-a[1])
  if abs(det)<1e-8:continue
  def z(x,y):
   u=((x-a[0])*(c[1]-a[1])-(c[0]-a[0])*(y-a[1]))/det;v=((b[0]-a[0])*(y-a[1])-(x-a[0])*(b[1]-a[1]))/det
   return a[2]+u*(b[2]-a[2])+v*(c[2]-a[2])+1
  for t in constrained_delaunay_triangles(patch).geoms:
   if t.area>1e-6:faces.append([(x,y,z(x,y)) for x,y in list(orient(t,sign=1).exterior.coords)[:3]])
 vertices=[v for f in faces for v in f];assert vertices
 lines=['o Krog_'+name]+[f'v {x:.4f} {-y:.4f} {z:.4f}' for x,y,z in vertices]+[f'vt {x/200:.5f} {y/200:.5f}' for x,y,z in vertices]
 for i in range(0,len(vertices),3):lines.append('f '+' '.join(f'{j}/{j}' for j in [i+3,i+2,i+1]))
 (folder/f'Krog_{name}.obj').write_text('\n'.join(lines)+'\n');results.append({'file':f'Krog_{name}.obj','triangles':len(faces)})
(folder/'markings.json').write_text(json.dumps({'surfaces':results,'scope':'Game-authored double yellow centre lines and white edges. Leave a600cm clear junction. Follows retained OSM roadway geometry; not a claim about current real-world paint. Visual review pending.','clearance_cm':1},indent=2)+'\n');print(results)
