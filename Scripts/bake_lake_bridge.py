"""OSM lake crossing: bank-anchored authored deck, not surveyed bridge elevation."""
from pathlib import Path
import json,math,numpy as np
from shapely.geometry import LineString,Point
P=Path(__file__).resolve().parents[1];O=P/'SourceAssets/Terrain/LakeBridge';O.mkdir(exist_ok=True)
paths=json.loads((P/'SourceAssets/Terrain/park-path-network.json').read_text())['paths']
a=next(p for p in paths if p['osm_id']==102679938);b=next(p for p in paths if p['osm_id']==146304988)
pts=np.array(a['points_cm']);ds=np.r_[0,np.cumsum(np.linalg.norm(np.diff(pts[:,:2],axis=0),axis=1))];pts[:,2]=np.interp(ds,[0,ds[-1]],[pts[0,2],pts[-1,2]])+12*np.sin(np.pi*ds/ds[-1])
line=LineString(pts[:,:2]);branch=np.array(b['points_cm']);join=line.project(Point(branch[0,:2]));branch[:,2]=np.interp(join,ds,pts[:,2])
meshes={k:{'v':[],'f':[]} for k in ['Deck','Wood','Rails']}
def quad(m,vs):
 n=len(m['v']);m['v'].extend([list(v) for v in vs]);m['f'].extend([[n,n+1,n+2],[n,n+2,n+3]])
def box(m,a,b,width,depth):
 # Beam top follows a->b, with horizontal transverse width and vertical depth.
 a=np.array(a);b=np.array(b);d=b-a;side=np.array([-d[1],d[0],0.]);side=side/np.linalg.norm(side)*width/2
 v=[a-side,a+side,b+side,b-side];low=[x-np.array([0,0,depth]) for x in v]
 quad(m,list(reversed(v)))
 quad(m,low)
 for i in range(4):j=(i+1)%4;quad(m,[v[i],low[i],low[j],v[j]])
def ribbon(points,width,m):
 # Shared miter vertices avoid pinholes at OSM bends.
 left=[];right=[]
 for i,p in enumerate(points):
  d=points[min(i+1,len(points)-1)]-points[max(0,i-1)];s=np.array([-d[1],d[0],0.]);s=s/np.linalg.norm(s)*width/2;left.append(p+s);right.append(p-s)
 for i in range(len(points)-1):
  v=[right[i],left[i],left[i+1],right[i+1]];quad(m,list(reversed(v)));low=[x-np.array([0,0,20]) for x in v];quad(m,low)
  quad(m,[right[i],right[i+1],right[i+1]-[0,0,20],right[i]-[0,0,20]])
  quad(m,[left[i+1],left[i],left[i]-[0,0,20],left[i+1]-[0,0,20]])
 return left,right
left,right=ribbon(pts,280,meshes['Deck']);ribbon(branch,280,meshes['Wood'])
# Open a junction in the rail toward the mapped wood spur.
near=np.array(line.interpolate(join).coords[0]);tangent=pts[min(len(pts)-1,int(np.searchsorted(ds,join))+1),:2]-pts[max(0,int(np.searchsorted(ds,join))-1),:2]
branch_side=np.sign(np.cross(tangent,branch[-1,:2]-near))
for sign,edge in [(1,left),(-1,right)]:
 for i,(x,y) in enumerate(zip(edge,edge[1:])):
  if sign==branch_side and abs((ds[i]+ds[i+1])/2-join)<190:continue
  for h in [48,95]:box(meshes['Rails'],x+[0,0,h],y+[0,0,h],6,6)
  if i%3==0:box(meshes['Rails'],x+[-4,0,98],x+[4,0,98],8,98)
manifest=[]
for k,m in meshes.items():
 name='LakeBridge_'+k;v=np.array(m['v']);lines=['# OSM contributors ODbL; authored deck elevation',f'o {name}']
 lines+=['v %.6f %.6f %.6f'%(x,-y,z) for x,y,z in v]
 lines+=['vt %.6f %.6f'%(x/200,y/200) for x,y,z in v]
 lines+=['f '+' '.join(f'{i+1}/{i+1}' for i in reversed(f)) for f in m['f']]
 (O/(name+'.obj')).write_text('\n'.join(lines)+'\n');manifest.append({'file':name+'.obj','kind':k,'bounds_cm':[v.min(axis=0).tolist(),v.max(axis=0).tolist()]})
result={'status':'baked','source_osm_ids':[a['osm_id'],b['osm_id']],'elevation_policy':'Authored linear bank-to-bank deck with 12 cm crown; wood spur anchored to crossing. Not measured bridge survey.','chunks':manifest,'centerline_cm':pts.tolist(),'spur_cm':branch.tolist(),'width_cm':280,'length_cm':float(ds[-1]),'pending':['Installed collision and ride-through','Final reference-matched architecture']}
(O/'manifest.json').write_text(json.dumps(result,indent=2));print('Baked',len(manifest),'meshes;',round(ds[-1]/100,2),'scaled metres')
