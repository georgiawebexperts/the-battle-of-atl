"""Independent native collision probes from candidate mesh triangle interiors."""
import json,random
from pathlib import Path
from shapely.geometry import Polygon,Point
from shapely.strtree import STRtree
root=Path(__file__).resolve().parents[1];folder=root/'SourceAssets/Terrain/TenthStreetGraded';rng=random.Random(143);rows=[];all_triangles=[]
files=[(folder/('TenthStreet_'+k+'.obj'),False) for k in ['Road','CycleTrack','Sidewalk','Separator']]+[(p,True) for p in (folder/'Connections').glob('*.obj')]
for path,legacy in files:
 verts=[];points=[]
 for line in path.read_text().splitlines():
  parts=line.split()
  if parts and parts[0]=='v':
   x,y,z=map(float,parts[1:4]);verts.append((x,y if legacy else -y,z))
  elif parts and parts[0]=='f':
   a,b,c=[verts[int(v.split('/')[0])-1] for v in parts[1:]]
   all_triangles.append((a,b,c))
   if abs((b[0]-a[0])*(c[1]-a[1])-(c[0]-a[0])*(b[1]-a[1]))<200:continue
   center=[sum(p[k] for p in [a,b,c])/3 for k in range(3)]
   if legacy and not (-31000<center[0]<14000 and 8500<center[1]<14500):continue
   points.append(center)
 rows.extend({'mesh':path.stem,'xyz':p} for p in rng.sample(points,min(60,len(points))))
tree=STRtree([Polygon([v[:2] for v in tri]) for tri in all_triangles])
for row in rows:
 x,y,z=row['xyz'];tops=[]
 for idx in tree.query(Point(x,y),predicate='intersects'):
  a,b,c=all_triangles[idx];det=(b[0]-a[0])*(c[1]-a[1])-(c[0]-a[0])*(b[1]-a[1])
  if abs(det)<1e-9:continue
  u=((x-a[0])*(c[1]-a[1])-(c[0]-a[0])*(y-a[1]))/det;v=((b[0]-a[0])*(y-a[1])-(x-a[0])*(b[1]-a[1]))/det
  tops.append(a[2]+u*(b[2]-a[2])+v*(c[2]-a[2]))
 row['source_surface_z']=z;row['xyz'][2]=max(tops,default=z)
(folder/'collision-samples.json').write_text(json.dumps({'samples':rows},indent=2)+'\n');print(len(rows))
