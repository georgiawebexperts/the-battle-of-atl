from pathlib import Path
import json,math,numpy as np,sys
sys.path.insert(0,str(Path(__file__).resolve().parent))
from shapely.geometry import LineString
from route_height_profiles import RouteHeightProfiles
from arcade_trail import rail_offset_cm
root=Path(__file__).resolve().parents[1];out=root/'SourceAssets/Terrain/EastsideTrail'
h=RouteHeightProfiles(root/'SourceAssets/Terrain/eastside-height-profiles.json');line=h.line
def quad(m,vs):
 n=len(m['v']);m['v'].extend([list(v) for v in vs]);m['f'].extend([[n,n+1,n+2],[n,n+2,n+3]])
def box(m,a,b,width,depth):
 # Beam top follows a->b, with horizontal transverse width and vertical depth.
 a=np.array(a);b=np.array(b);d=b-a;side=np.array([-d[1],d[0],0.]);side=side/np.linalg.norm(side)*width/2
 v=[a-side,a+side,b+side,b-side];low=[x-np.array([0,0,depth]) for x in v]
 quad(m,list(reversed(v)))
 quad(m,low)
 for i in range(4):j=(i+1)%4;quad(m,[v[i],low[i],low[j],v[j]])

manifest=json.loads((out/'manifest.json').read_text())
manifest['chunks']=[c for c in manifest['chunks'] if c.get('material')!='BridgeRails']
for profile in h.profiles:
 m={'v':[],'f':[]};lo=profile['bridge_start_cm'];hi=profile['bridge_end_cm'];stations=np.linspace(lo,hi,max(2,math.ceil((hi-lo)/180)+1))
 for sign in [-1,1]:
  edge=[]
  for s in stations:
   p=np.array(line.interpolate(s).coords[0]);a=np.array(line.interpolate(s-1).coords[0]);b=np.array(line.interpolate(s+1).coords[0]);d=b-a;normal=np.array([-d[1],d[0]])/np.linalg.norm(d);q=p+normal*sign*rail_offset_cm()
   z=profile['start_z_cm']+(profile['end_z_cm']-profile['start_z_cm'])*(s-profile['blend_start_cm'])/(profile['blend_end_cm']-profile['blend_start_cm'])
   edge.append(np.array([q[0],q[1],z]))
  for a,b in zip(edge,edge[1:]):
   for height in [48,100]:box(m,a+[0,0,height],b+[0,0,height],8,8)
  for p in edge:box(m,p+[-5,0,104],p+[5,0,104],10,104)
 name='EastsideBridge_'+str(profile['osm_id'])+'_Rails';v=np.array(m['v']);lines=['# Authored arcade rail dimensions; OSM alignment, ODbL',f'o {name}']
 lines+=['v %.6f %.6f %.6f'%(x,-y,z) for x,y,z in v]
 lines+=['vt %.6f %.6f'%(x/200,y/200) for x,y,z in v]
 lines+=['f '+' '.join(f'{i+1}/{i+1}' for i in reversed(face)) for face in m['f']]
 (out/(name+'.obj')).write_text('\n'.join(lines)+'\n')
 manifest['chunks'].append({'file':name+'.obj','material':'BridgeRails','triangles':len(m['f']),'vertices':len(m['v']),'bounds_cm':[v.min(axis=0).tolist(),v.max(axis=0).tolist()]})
manifest['triangle_count']=sum(c['triangles'] for c in manifest['chunks']);manifest['rail_policy']='Authored 104 cm guardrail height, two horizontal rails, posts spaced at most 180 cm; reference architecture remains pending.'
(out/'manifest.json').write_text(json.dumps(manifest,indent=2)+'\n')
print('Baked',len(manifest['chunks']),'trail/deck/rail meshes,',manifest['triangle_count'],'triangles')
