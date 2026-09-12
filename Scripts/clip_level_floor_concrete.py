"""Clip covered concrete out of the level tunnel floor candidate."""
import json
from pathlib import Path
from shapely.geometry import Polygon
from shapely.ops import unary_union
from shapely import constrained_delaunay_triangles
root=Path(__file__).resolve().parents[1];folder=root/'SourceAssets/Terrain/KrogTraffic'
def read(path,sign):
    vertices=[];result=[]
    for row in path.read_text().splitlines():
        f=row.split()
        if f and f[0]=='v':vertices.append((float(f[1]),sign*float(f[2]),float(f[3])))
        elif f and f[0]=='f':
            tri=[vertices[int(v.split('/')[0])-1] for v in f[1:]]
            if Polygon([v[:2] for v in tri]).area>1e-6:result.append(tri)
    return result
def plane(tri,x,y):
    a,b,c=tri;det=(b[0]-a[0])*(c[1]-a[1])-(b[1]-a[1])*(c[0]-a[0])
    u=((x-a[0])*(c[1]-a[1])-(y-a[1])*(c[0]-a[0]))/det
    v=((b[0]-a[0])*(y-a[1])-(b[1]-a[1])*(x-a[0]))/det
    return a[2]+u*(b[2]-a[2])+v*(c[2]-a[2])

road=read(root/'SourceAssets/Terrain/KrogRoute/KrogTunnel_Road.obj',1)
footprint=unary_union([Polygon([v[:2] for v in t]) for t in road]).buffer(.002)
reports=[]
for name in ['KrogRoute_Concrete_7_1','KrogRoute_Concrete_8_1']:
 original=read(root/'SourceAssets/Terrain/KrogRoute'/(name+'.obj'),1);faces=[];removed=0
 for tri in original:
  polygon=Polygon([v[:2] for v in tri]);remainder=polygon.difference(footprint);removed+=polygon.area-remainder.area
  for patch in constrained_delaunay_triangles(remainder).geoms:
   if patch.area>.001:faces.append([(x,y,plane(tri,x,y)) for x,y in list(patch.exterior.coords)[:3]])
 vertices=[v for face in faces for v in face]
 if not vertices:
  reports.append({'source':name,'source_triangles':len(original),'remaining_triangles':0,'removed_overlap_cm2':removed});continue
 rows=['o '+name+'_LevelFloor']+[f'v {x:.6f} {y:.6f} {z:.6f}' for x,y,z in vertices]+[f'vt {x/200:.6f} {y/200:.6f}' for x,y,z in vertices]
 for i in range(0,len(vertices),3):
  a,b,c=vertices[i:i+3];up=(b[0]-a[0])*(c[1]-a[1])-(b[1]-a[1])*(c[0]-a[0])
  indices=[i+1,i+2,i+3] if up>0 else [i+3,i+2,i+1]
  rows.append('f '+' '.join(f'{j}/{j}' for j in indices))
 (folder/(name+'_LevelFloor.obj')).write_text('\n'.join(rows)+'\n')
 reports.append({'source':name,'source_triangles':len(original),'remaining_triangles':len(faces),'removed_overlap_cm2':removed})
(folder/'level-floor-concrete.json').write_text(json.dumps({'surfaces':reports,'scope':'Source clipping only; native import, seam and traversal validation pending.'},indent=2)+'\n')
print(json.dumps(reports))
