"""Photo-informed exterior meshes on retained MAA footprint; no copied imagery."""
import json,math
from pathlib import Path
import numpy as np
from shapely.geometry import Polygon
from shapely.geometry.polygon import orient
from shapely.ops import triangulate
root=Path(__file__).resolve().parents[1];folder=root/'SourceAssets/Terrain/FancyRoachMotel';source=json.loads((folder/'footprints.json').read_text())
groups={k:[] for k in ['Brick','Stucco','Trim','Glass','Roof','WarmGlass']}
def face(mat,pts):groups[mat].append([list(p) for p in pts])
def box(mat,p,size,yaw=0):
 c,s=math.cos(yaw),math.sin(yaw);v=[]
 for z in [-1,1]:
  for x,y in [(-1,-1),(1,-1),(1,1),(-1,1)]:
   a=x*size[0]/2;b=y*size[1]/2;v.append([p[0]+c*a-s*b,p[1]+s*a+c*b,p[2]+z*size[2]/2])
 for ids in [(3,2,1,0),(4,5,6,7),(0,1,5,4),(1,2,6,5),(2,3,7,6),(3,0,4,7)]:face(mat,[v[i] for i in ids])
windows=0;fronts=[]
for building in source['buildings']:
 poly=orient(Polygon([p[:2] for p in building['footprint_world_cm']]),sign=1);pts=list(poly.exterior.coords);z=max(p[2] for p in building['footprint_world_cm'])-18
 # Five visible window tiers in official corner photograph. Height is authored,
 # not a surveyed measurement; footprint retains game's geographic compression.
 height=1450;floor=280
 for a,b in zip(pts,pts[1:]):
  a=np.array(a);b=np.array(b);delta=b-a;length=float(np.linalg.norm(delta));u=delta/length;out=np.array([u[1],-u[0]]);mid=(a+b)/2;yaw=math.atan2(u[1],u[0])
  def part(mat,d,h,w,depth,tall,offset=0):
   p=a+u*d+out*offset;box(mat,[*p,z+h],[w,depth,tall],yaw)
  part('Brick',length/2,140,length+2,28,280)
  part('Stucco',length/2,865,length+2,28,1170)
  for h in [285,565,845,1125,1450]:part('Stucco',length/2,h,length+10,38,16,5)
  part('Roof',length/2,1470,length+24,100,22,18)
  count=int(length//185)
  for i in range(count):
   d=(i+.5)*length/count
   for level in range(5):
    h=level*floor+155;w=min(106,length/count-45);tall=175 if level else 180
    part('Trim',d,h,w+18,12,tall+18,19)
    part('WarmGlass' if (i+level+building['osm_way'])%11==0 else 'Glass',d,h,w,9,tall,27)
    for side in [-1,1]:part('Stucco',d+side*w/2,h,4,8,tall,34)
    part('Stucco',d,h,4,8,tall,34);part('Stucco',d,h,w,8,4,34)
    part('Stucco',d,h-tall/2,w+20,20,7,31);windows+=1
   if i%2==0:part('Trim',d,1360,5,35,150,48)
  if out[1]>.9 and length>600:fronts.append({'a':a.tolist(),'b':b.tolist(),'out':out.tolist(),'z':z,'length':length})
 for tri in triangulate(poly):
  if poly.covers(tri):face('Roof',[[x,y,z+1450] for x,y in list(tri.exterior.coords)[:-1]])
front=max(fronts,key=lambda f:f['length']);mid=(np.array(front['a'])+front['b'])/2;out=np.array(front['out']);sign=mid+out*55;yaw=math.atan2(*(np.array(front['b'])-front['a'])[::-1]);box('Trim',[*sign,front['z']+340],[650,25,85],yaw)
manifest={'meshes':[],'windows':windows,'sign':{'position_world_cm':[*list(sign+out*15),front['z']+340],'yaw':math.degrees(math.atan2(out[1],out[0])),'text':'THE FANCY ROACH MOTEL'},'scope':'Photo-informed authored facade, actual footprint; height and window spacing approximated. Native render pending.'}
for mat,faces in groups.items():
 path=folder/('FancyRoach_'+mat+'.obj');verts=[v for f in faces for v in f];lines=['o FancyRoach_'+mat]+['v '+' '.join(f'{c:.4f}' for c in (v[0],-v[1],v[2])) for v in verts]+['vt 0 0','vt 1 0','vt 1 1','vt 0 1'];n=1
 for f in faces:lines.append('f '+' '.join(f'{i}/{i-n+1}' for i in reversed(range(n,n+len(f)))));n+=len(f)
 path.write_text('\n'.join(lines)+'\n');a=np.array(verts);manifest['meshes'].append({'file':path.name,'material':mat,'bounds':[a.min(axis=0).tolist(),a.max(axis=0).tolist()],'faces':len(faces)})
(folder/'geometry.json').write_text(json.dumps(manifest,indent=2)+'\n');print(json.dumps({'windows':windows,'meshes':len(groups),'sign':manifest['sign']}))
