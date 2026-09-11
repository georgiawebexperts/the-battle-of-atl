"""Authored bank-anchored decks for seven mapped creek and Northwoods crossings."""
from pathlib import Path
import json,numpy as np
P=Path(__file__).resolve().parents[1];O=P/'SourceAssets/Terrain/WetlandBridges';O.mkdir(exist_ok=True)
paths=json.loads((P/'SourceAssets/Terrain/park-path-network.json').read_text())['paths']
ids={146304984,182302113,226119763,226324512,226324514,1032063077,1050793219}
chunks=[];bridges=[]
def quad(m,v):
 n=len(m['v']);m['v'].extend([list(x) for x in v]);m['f'].extend([[n,n+1,n+2],[n,n+2,n+3]])
def beam(m,a,b,width,depth):
 a=np.array(a);b=np.array(b);d=b-a;s=np.array([-d[1],d[0],0.]);s=s/np.linalg.norm(s)*width/2
 v=[a-s,a+s,b+s,b-s];lo=[x-[0,0,depth] for x in v];quad(m,list(reversed(v)));quad(m,lo)
 for i in range(4):j=(i+1)%4;quad(m,[v[i],lo[i],lo[j],v[j]])
for path in paths:
 if path['osm_id'] not in ids:continue
 pts=np.array(path['points_cm']);distance=np.r_[0,np.cumsum(np.linalg.norm(np.diff(pts[:,:2],axis=0),axis=1))];ground=pts[:,2].copy();pts[:,2]=np.maximum(ground,np.interp(distance,[0,distance[-1]],[ground[0],ground[-1]])+8*np.sin(np.pi*distance/distance[-1]))
 decks={'v':[],'f':[]};rails={'v':[],'f':[]};left=[];right=[]
 for i,v in enumerate(pts):
  d=pts[min(i+1,len(pts)-1)]-pts[max(0,i-1)];s=np.array([-d[1],d[0],0]);s=s/np.linalg.norm(s)*140;left.append(v+s);right.append(v-s)
 for i in range(len(pts)-1):
  v=[right[i],left[i],left[i+1],right[i+1]];lo=[x-[0,0,18] for x in v];quad(decks,list(reversed(v)));quad(decks,lo)
  for edge in [left,right]:
   a=edge[i];b=edge[i+1];quad(decks,[a,b,b-[0,0,18],a-[0,0,18]])
   for height in [35,66,102]:beam(rails,a+[0,0,height],b+[0,0,height],7,7)
 for edge in [left,right]:
  prev=-10000
  for i,v in enumerate(edge):
   if distance[i]-prev>=120 or i==len(edge)-1:
    beam(rails,v+[-5,0,108],v+[5,0,108],10,108);prev=distance[i]
 # End caps and modest footings are geometry, not arbitrary new path connectors.
 for i in [0,len(pts)-1]:quad(decks,[left[i],right[i],right[i]-[0,0,18],left[i]-[0,0,18]])
 wood=path['tags'].get('surface')=='wood';deckkind='Wood' if wood else 'Deck'
 for kind,m in [(deckkind,decks),('Rails',rails)]:
  name='ParkBridge_'+str(path['osm_id'])+'_'+kind;v=np.array(m['v']);text=['# OSM contributors ODbL; authored bridge elevation',f'o {name}']+['v %.6f %.6f %.6f'%(x,-y,z) for x,y,z in v]+['vt %.6f %.6f'%(x/200,y/200) for x,y,z in v]+['f '+' '.join(f'{i+1}/{i+1}' for i in reversed(f)) for f in m['f']]
  (O/(name+'.obj')).write_text('\n'.join(text)+'\n');chunks.append({'file':name+'.obj','kind':kind,'label':f'Park bridge {path["osm_id"]} {kind}','bounds_cm':[v.min(axis=0).tolist(),v.max(axis=0).tolist()]})
 bridges.append({'osm_id':path['osm_id'],'centerline_cm':pts.tolist(),'length_cm':float(distance[-1]),'max_grade':float(np.max(np.abs(np.diff(pts[:,2]))/np.diff(distance))),'width_cm':280})
result={'status':'source_baked','elevation_policy':'Authored bank-to-bank deck with 8 cm crown, never below sampled terrain. Not surveyed bridge heights.','chunks':chunks,'bridges':bridges,'pending':['Installed collision and bidirectional bank approach tests','Reference-matched appearance','Full park connectivity']};(O/'manifest.json').write_text(json.dumps(result,indent=2));print([(b['osm_id'],round(b['length_cm']/100,1),round(b['max_grade'],3)) for b in bridges])
