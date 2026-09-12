"""Author lane paint and drape it exactly onto the generated pavement triangles."""
import json,math,os
from pathlib import Path
from shapely.geometry import LineString,Polygon
from shapely.ops import substring,unary_union
from shapely import constrained_delaunay_triangles
from shapely.geometry.polygon import orient
root=Path(__file__).resolve().parents[1];folder=root/'SourceAssets/Terrain'/('TenthStreetGraded' if os.environ.get('BATTLE_TENTH_CANDIDATE')=='1' else 'TenthStreet')
network=json.loads((folder/'network.json').read_text());white=[];yellow=[]
# Authored road layout: four lanes have a divided center and dashed lane lines;
# three lanes have a shared center turn lane, bounded by solid/dashed yellow.
for row in network['roads']:
 pts=[p[:2] for p in row['points_cm']]
 if pts[0][0]>pts[-1][0]:pts.reverse()
 path=LineString(pts);lanes=int(row['tags'].get('lanes','3'));width=lanes*300
 def stripe(offset,dashed,target):
  line=path.offset_curve(offset,join_style=2)
  if line.geom_type!='LineString' or line.length<60:return
  # Stop short of segment ends; crossing clipping below provides clear zones.
  if dashed:
   for d in range(40,int(line.length)-40,600):
    target.append(substring(line,d,min(d+250,line.length-40)).buffer(5,cap_style=2))
  else:target.append(substring(line,20,line.length-20).buffer(5,cap_style=2))
 for edge in [20,width-20]:stripe(edge,False,white)
 if lanes==4:
  for offset in [300,900]:stripe(offset,True,white)
  for offset in [590,610]:stripe(offset,False,yellow)
 else:
  for offset in [300,600]:stripe(offset,False,yellow)
  for offset in [320,580]:stripe(offset,True,yellow)
# Markings must not cross the protected cycle crossings or Monroe conflict zone.
from shapely.geometry import Point
clear=unary_union([Point(network['monroe_crossing']['world_cm'][:2]).buffer(1000)]+[LineString([p[:2] for p in r['points_cm']]).buffer(170) for r in network['cycle_track'] if r['tags'].get('cycleway')=='crossing'])
paints={'WhitePaint':unary_union(white).difference(clear),'YellowPaint':unary_union(yellow).difference(clear)}
verts=[];triangles=[]
for line in (folder/'TenthStreet_Road.obj').read_text().splitlines():
 parts=line.split()
 if parts and parts[0]=='v':verts.append((float(parts[1]),-float(parts[2]),float(parts[3])))
 elif parts and parts[0]=='f':triangles.append([verts[int(p.split('/')[0])-1] for p in parts[1:]])
results=[]
for name,paint in paints.items():
 faces=[]
 for points in triangles:
  patch=Polygon([p[:2] for p in points]).intersection(paint)
  if patch.area<1e-6:continue
  a,b,c=points;det=(b[0]-a[0])*(c[1]-a[1])-(c[0]-a[0])*(b[1]-a[1])
  def z(x,y):
   u=((x-a[0])*(c[1]-a[1])-(c[0]-a[0])*(y-a[1]))/det
   v=((b[0]-a[0])*(y-a[1])-(x-a[0])*(b[1]-a[1]))/det
   return a[2]+u*(b[2]-a[2])+v*(c[2]-a[2])+1
  for t in constrained_delaunay_triangles(patch).geoms:
   if t.area>1e-6:faces.append([(x,y,z(x,y)) for x,y in list(orient(t,sign=1).exterior.coords)[:3]])
 vertices=[v for f in faces for v in f];assert vertices
 lines=['o TenthStreet_'+name]+[f'v {x:.4f} {-y:.4f} {z:.4f}' for x,y,z in vertices]
 lines += [f'vt {x/200:.5f} {y/200:.5f}' for x,y,z in vertices]
 for i in range(0,len(vertices),3):lines.append('f '+' '.join(f'{j}/{j}' for j in [i+3,i+2,i+1]))
 path=folder/('TenthStreet_'+name+'.obj');path.write_text('\n'.join(lines)+'\n')
 results.append({'file':path.name,'triangles':len(faces),'bounds_cm':[[min(v[i] for v in vertices) for i in range(3)],[max(v[i] for v in vertices) for i in range(3)]]})
(folder/'markings.json').write_text(json.dumps({'surfaces':results,'paint_clearance_cm':1,'scope':'10th motor-lane markings only. Monroe signals, stop lines and cycle symbols pending.'},indent=2)+'\n')
print(json.dumps(results))
