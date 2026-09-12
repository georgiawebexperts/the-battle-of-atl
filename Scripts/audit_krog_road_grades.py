"""Measure actual candidate triangle grades along mapped approach centrelines."""
import json,math
from pathlib import Path
from shapely.geometry import Point,Polygon,LineString
from shapely import STRtree
r=Path(__file__).resolve().parents[1];folder=r/'SourceAssets/Terrain/KrogTraffic'
triangles=[];polygons=[]
paths=[folder/'Krog_Road.obj']+list((r/'SourceAssets/Terrain/KrogRoute').glob('*Asphalt*.obj'))+list((r/'SourceAssets/Terrain/KrogRoute').glob('*Concrete*.obj'))+[r/'SourceAssets/Terrain/KrogRoute/KrogTunnel_Road.obj']
for file in paths:
 vs=[]
 for line in file.read_text().splitlines():
  f=line.split()
  if f and f[0]=='v':vs.append((float(f[1]),-float(f[2]) if file.name=='Krog_Road.obj' else float(f[2]),float(f[3])))
  elif f and f[0]=='f':
   tri=[vs[int(s.split('/')[0])-1] for s in f[1:]];p=Polygon([v[:2] for v in tri])
   if p.area>1e-6:polygons.append(p);triangles.append(tri)
index=STRtree(polygons)
def height(x,y):
 p=Point(x,y);zs=[]
 for j in index.query(p,predicate='intersects'):
  a,b,c=triangles[int(j)];den=(b[1]-c[1])*(a[0]-c[0])+(c[0]-b[0])*(a[1]-c[1]);u=((b[1]-c[1])*(x-c[0])+(c[0]-b[0])*(y-c[1]))/den;v=((c[1]-a[1])*(x-c[0])+(a[0]-c[0])*(y-c[1]))/den;zs.append(u*a[2]+v*b[2]+(1-u-v)*c[2])
 return max(zs) if zs else None
rows=[]
for road in json.loads((folder/'network.json').read_text())['roads']:
 line=LineString([p[:2] for p in road['points_cm']]);samples=[];bad=[];grades=[]
 for distance in range(10,math.floor(line.length)-10,10):
  x,y=line.interpolate(distance).coords[0];z=height(x,y);samples.append({'distance_cm':distance,'xyz':[x,y,z]})
 for a,b in zip(samples,samples[1:]):
  if a['xyz'][2] is None or b['xyz'][2] is None:continue
  grade=(b['xyz'][2]-a['xyz'][2])/(b['distance_cm']-a['distance_cm'])
  grades.append(abs(grade)*100)
  if abs(grade)>.12:bad.append({'from':a,'to':b,'grade_percent':100*grade})
 rows.append({'osm_way':road['osm_way'],'name':road['tags']['name'],'samples':len(samples),'missing':sum(s['xyz'][2] is None for s in samples),'maximum_grade_percent':max(grades,default=0),'over_12_percent':len(bad),'worst':sorted(bad,key=lambda q:-abs(q['grade_percent']))[:10]})
report={'roads':rows,'scope':'10cm centreline source-triangle measurements;12percent is a design review threshold, not a claim about real streets. No main-map changes.'}
(r/'Tests/Results/2026-09-12-krog-road-grades.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps([dict(osm_way=s['osm_way'],samples=s['samples'],missing=s['missing'],over_12_percent=s['over_12_percent'],max_percent=s['maximum_grade_percent']) for s in rows]))
