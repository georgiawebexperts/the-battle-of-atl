"""Drape Monroe lane lines, stop bars and mapped crossing stripes onto pavement."""
import json,math
from pathlib import Path
from shapely.geometry import LineString,Polygon,Point
from shapely.ops import substring,unary_union,linemerge
from shapely import constrained_delaunay_triangles
from shapely.geometry.polygon import orient
root=Path(__file__).resolve().parents[1];folder=root/'SourceAssets/Terrain/MonroeTraffic';old=root/'SourceAssets/Terrain/TenthStreetGraded'
network=json.loads((folder/'approaches-network.json').read_text());crossings=json.loads((folder/'crossings.json').read_text())['crossings'];white=[];yellow=[];crosspaint=[]
paths=[]
merged=linemerge([LineString(r['points_cm']) for r in crossings])
for source_path in ([merged] if merged.geom_type=='LineString' else merged.geoms):
 pts=list(source_path.coords);a,b=pts[0],pts[-1];dx,dy=b[0]-a[0],b[1]-a[1];length=math.hypot(dx,dy);dx/=length;dy/=length
 path=LineString([(a[0]-dx*650,a[1]-dy*650),*pts,(b[0]+dx*650,b[1]+dy*650)]);paths.append(path)
 for d in range(0,int(path.length),65):
  p=path.interpolate(d);q=path.interpolate(min(d+1,path.length));vx,vy=q.x-p.x,q.y-p.y;n=math.hypot(vx,vy)
  if n<.001:continue
  vx/=n;vy/=n;crosspaint.append(LineString([(p.x-vy*100,p.y+vx*100),(p.x+vy*100,p.y-vx*100)]).buffer(15,cap_style=2))
clear=unary_union([p.buffer(140) for p in paths]+[Point(12112.828,11635.199).buffer(650)])
for row in network['roads']:
 pts=row['points_cm'];pts=pts if pts[0][1]>pts[-1][1] else pts[::-1];path=LineString(pts);width=int(row['tags'].get('lanes','3'))*150
 for offset in [-10,10]:yellow.append(path.offset_curve(offset,join_style=2).buffer(5,cap_style=2))
 for offset in [-width+20,width-20]:white.append(path.offset_curve(offset,join_style=2).buffer(5,cap_style=2))
 if width==600:
  for offset in [-300,300]:
   line=path.offset_curve(offset,join_style=2)
   if line.geom_type!='LineString':continue
   for d in range(20,int(line.length)-20,600):white.append(substring(line,d,min(d+250,line.length-20)).buffer(5,cap_style=2))
stopbars=[]
for row in json.loads((root/'Tests/Results/2026-09-12-monroe-signal-support.json').read_text())['signals']:
 x,y,z=row['stop_xyz'];dx,dy=row['heading_xy'];x+=dx*270;y+=dy*270
 stopbars.append(LineString([(x-dy*150,y+dx*150),(x+dy*150,y-dx*150)]).buffer(15,cap_style=2))
paints={'WhitePaint':unary_union([unary_union(white).difference(clear),*stopbars,*crosspaint]),'YellowPaint':unary_union(yellow).difference(clear)}
triangles=[]
for path in [folder/'Monroe_Approaches.obj',folder/'Monroe_RoadSeams.obj']+[old/f'TenthStreet_{n}.obj' for n in ['Road','RoadSeams','CycleTrack']]:
 verts=[]
 for line in path.read_text().splitlines():
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
 lines=['o Monroe_'+name]+[f'v {x:.4f} {-y:.4f} {z:.4f}' for x,y,z in vertices]+[f'vt {x/200:.5f} {y/200:.5f}' for x,y,z in vertices]
 for i in range(0,len(vertices),3):lines.append('f '+' '.join(f'{j}/{j}' for j in [i+3,i+2,i+1]))
 (folder/f'Monroe_{name}.obj').write_text('\n'.join(lines)+'\n');results.append({'file':f'Monroe_{name}.obj','triangles':len(faces)})
(folder/'markings.json').write_text(json.dumps({'surfaces':results,'scope':'Gameplay candidate. Centre/edge lines, four-lane dashes, mapped crossing stripes, stop bars. Three-lane turn-lane detailing and visual acceptance pending.','clearance_cm':1},indent=2)+'\n');print(results)
