"""Build opposing Krog/DeKalb routes using source road and installed trail triangles."""
import json,math,collections
from pathlib import Path
from shapely.geometry import LineString
from shapely.ops import linemerge
root=Path(__file__).resolve().parents[1];out=root/'SourceAssets/Terrain/KrogTraffic'
network=json.loads((out/'network.json').read_text())
line=linemerge([LineString([p[:2] for p in row['points_cm']]) for row in network['roads'] if row['tags']['name'] in ['DeKalb Avenue Northeast']]);assert line.geom_type=='LineString'
if line.coords[0][0]>line.coords[-1][0]:line=LineString(list(line.coords)[::-1])
triangles=[];grid=collections.defaultdict(list)
meshes=[(out/'Krog_Road.obj',-1)]
for directory in ['EastsideTrail','KrogRoute']:
 meshes += [(p,1) for p in (root/'SourceAssets/Terrain'/directory).glob('*.obj') if any(t in p.stem for t in ['Asphalt','Concrete'])]
for mesh_path,sign in meshes:
 vertices=[];name=mesh_path.stem
 for row in mesh_path.read_text().splitlines():
  values=row.split()
  if values and values[0]=='v':vertices.append([float(values[1]),sign*float(values[2]),float(values[3])])
  elif values and values[0]=='f':
   tri=[vertices[int(v.split('/')[0])-1] for v in values[1:]]
   if max(p[0] for p in tri)<line.bounds[0]-500 or min(p[0] for p in tri)>line.bounds[2]+500 or max(p[1] for p in tri)<line.bounds[1]-500 or min(p[1] for p in tri)>line.bounds[3]+500:continue
   index=len(triangles);triangles.append((name,tri))
   for x in range(math.floor(min(p[0] for p in tri)/200),math.floor(max(p[0] for p in tri)/200)+1):
    for y in range(math.floor(min(p[1] for p in tri)/200),math.floor(max(p[1] for p in tri)/200)+1):grid[x,y].append(index)
def height(x,y):
 found=[]
 for index in grid[math.floor(x/200),math.floor(y/200)]:
  name,(a,b,c)=triangles[index];det=(b[0]-a[0])*(c[1]-a[1])-(b[1]-a[1])*(c[0]-a[0])
  if abs(det)<1e-8:continue
  u=((x-a[0])*(c[1]-a[1])-(y-a[1])*(c[0]-a[0]))/det;v=((b[0]-a[0])*(y-a[1])-(b[1]-a[1])*(x-a[0]))/det
  if min(u,v,1-u-v)>=-1e-5:found.append((a[2]+u*(b[2]-a[2])+v*(c[2]-a[2]),name))
 return max(found) if found else None
routes=[];probes=[];failures=[]
for name,offset,reverse in [('eastbound',200,False),('westbound',-200,True)]:
 points=[]
 for d in range(400,int(line.length-400),50):
  x,y=line.interpolate(d).coords[0];a=line.interpolate(d-25);b=line.interpolate(d+25);dx,dy=b.x-a.x,b.y-a.y;n=math.hypot(dx,dy);dx/=n;dy/=n;x-=dy*offset;y+=dx*offset
  h=height(x,y)
  if not h:failures.append({'lane':name,'xy':[x,y],'kind':'centre'});continue
  points.append([x,y,h[0]])
  for along,across in [(a,b) for a in [-236,0,236] for b in [-114,0,114]]+[(a,b) for a in [123,-141.2] for b in [-90,90]]:
   px,py=x+dx*along-dy*across,y+dy*along+dx*across;hit=height(px,py)
   if not hit:failures.append({'lane':name,'xy':[px,py],'kind':'footprint'})
   else:probes.append({'lane':name,'xyz':[px,py,hit[0]],'source_surface':hit[1]})
 if reverse:points.reverse()
 routes.append({'name':name,'points_cm':points,'speed_cm_s':500,'scope':'Krog/DeKalb candidate; native support, crossing controls and endpoint visibility pending.'})
report={'author':'2026-09-12 [codex-maclaptop]','source':'OpenStreetMap contributors; retained Krog/DeKalb network and native-reviewed surfaces','road_length_cm':line.length,'source_coverage_passed':not failures,'failures':failures,'routes':routes,'main_map_changed':False}
(out/'car-lanes.json').write_text(json.dumps(report,indent=2)+'\n');(out/'car-lane-probes.json').write_text(json.dumps({'samples':probes})+'\n');print(json.dumps({'road_length_cm':line.length,'routes':len(routes),'probes':len(probes),'failures':len(failures)}))
