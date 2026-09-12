"""Authored playable tunnel cross-section on sourced road-portal alignment.
Dimensions accommodate the unscaled player at 1:3 world scale; not a survey.
"""
from pathlib import Path
import json,math,argparse,numpy as np
from route_height_profiles import RouteHeightProfiles
root=Path(__file__).resolve().parents[1]
parser=argparse.ArgumentParser();parser.add_argument('--output',type=Path);parser.add_argument('--portal-setback-cm',type=float,default=0);parser.add_argument('--continuous-shell',action='store_true');parser.add_argument('--outward-sides',action='store_true');args=parser.parse_args()
out=args.output or root/'SourceAssets/Terrain/KrogRoute'
assert 0<=args.portal_setback_cm<=2500
out.mkdir(parents=True,exist_ok=True)
h=RouteHeightProfiles(root/'SourceAssets/Terrain/krog-height-profiles.json');p=h.profiles[0];line=h.line

def quad(m,vs):
 n=len(m['v']);m['v'].extend([list(v) for v in vs]);m['f'].extend([[n,n+1,n+2],[n,n+2,n+3]])
def beam(m,a,b,width,depth):
 a=np.array(a);b=np.array(b);d=b-a;side=np.array([-d[1],d[0],0.]);side=side/np.linalg.norm(side)*width/2
 v=[a-side,a+side,b+side,b-side];low=[x-np.array([0,0,depth]) for x in v]
 quad(m,list(reversed(v)));quad(m,low)
 for i in range(4):j=(i+1)%4;quad(m,[v[i],v[j],low[j],low[i]] if args.outward_sides else [v[i],low[i],low[j],v[j]])
def at(s,lateral=0,zoffset=0):
 q=np.array(line.interpolate(s).coords[0]);d=np.array(line.interpolate(s+1).coords[0])-np.array(line.interpolate(s-1).coords[0]);normal=np.array([-d[1],d[0]])/np.linalg.norm(d);q+=normal*lateral
 z=p['start_z_cm']+(p['end_z_cm']-p['start_z_cm'])*(s-p['blend_start_cm'])/(p['blend_end_cm']-p['blend_start_cm'])
 return np.array([q[0],q[1],z+zoffset])
def ribbon(m,a,b,left,right,top,bottom,cap_start=True,cap_end=True):
 v=[at(a,left,top),at(a,right,top),at(b,right,top),at(b,left,top)]
 low=[at(a,left,bottom),at(a,right,bottom),at(b,right,bottom),at(b,left,bottom)]
 quad(m,list(reversed(v)));quad(m,low)
 for i in range(4):
  if (i==0 and not cap_start) or (i==2 and not cap_end):continue
  j=(i+1)%4;quad(m,[v[i],v[j],low[j],low[i]] if args.outward_sides else [v[i],low[i],low[j],v[j]])
lo=p['bridge_start_cm'];hi=p['bridge_end_cm'];stations=np.linspace(lo,hi,math.ceil((hi-lo)/120)+1)
meshes={k:{'v':[],'f':[]} for k in ['Road','Shell','Columns']}
for a,b in zip(stations,stations[1:]):
 ribbon(meshes['Road'],a,b,-620,340,-12,-32)
stations=np.linspace(lo+args.portal_setback_cm,hi,math.ceil((hi-lo-args.portal_setback_cm)/120)+1)
for index,(a,b) in enumerate(zip(stations,stations[1:])):
 caps={'cap_start':not args.continuous_shell or index==0,'cap_end':not args.continuous_shell or index==len(stations)-2}
 ribbon(meshes['Shell'],a,b,-660,380,310,270,**caps)
 for side in [-640,360]:ribbon(meshes['Shell'],a,b,side-20,side+20,270,-32,**caps)
for s in np.linspace(lo+args.portal_setback_cm+80,hi-80,math.ceil((hi-lo-args.portal_setback_cm-160)/280)+1):
 q=at(s,-300,270);beam(meshes['Columns'],q+[-18,0,0],q+[18,0,0],36,282)
manifest=json.loads((root/'SourceAssets/Terrain/KrogRoute/manifest.json').read_text());manifest['portal_setback_cm']=args.portal_setback_cm;manifest['chunks']=[c for c in manifest['chunks'] if not c.get('structure')]
for kind,m in meshes.items():
 name='KrogTunnel_'+kind;v=np.array(m['v']);lines=['# Authored arcade structure on OSM alignment; ODbL',f'o {name}']
 lines+=['v %.6f %.6f %.6f'%(x,-y,z) for x,y,z in v];lines+=['vt %.6f %.6f'%(x/200,z/200) for x,y,z in v]
 lines+=['f '+' '.join(f'{i+1}/{i+1}' for i in reversed(face)) for face in m['f']]
 (out/(name+'.obj')).write_text('\n'.join(lines)+'\n')
 manifest['chunks'].append({'file':name+'.obj','material':'Asphalt' if kind=='Road' else 'Concrete','structure':kind,'triangles':len(m['f']),'vertices':len(m['v']),'bounds_cm':[v.min(axis=0).tolist(),v.max(axis=0).tolist()]})
manifest['continuous_shell']=args.continuous_shell
manifest['outward_sides']=args.outward_sides
manifest['triangle_count']=sum(c['triangles'] for c in manifest['chunks']);manifest['architecture_policy']='Authored 270 cm player headroom, 960 cm interior, lowered road and column line; mapped portals and floor grade, not measured or photo-matched dimensions.'
manifest['dark_zones']=[{'a':at(a,-140,130).tolist(),'b':at(b,-140,130).tolist(),'half_width':480,'half_height':140} for a,b in zip(stations,stations[1:])]
manifest['roof_samples']=[at(s).tolist() for s in np.linspace(lo+args.portal_setback_cm+10,hi-10,50)]
(out/'manifest.json').write_text(json.dumps(manifest,indent=2)+'\n');print('Baked',len(manifest['chunks']),'meshes;',manifest['triangle_count'],'triangles')
