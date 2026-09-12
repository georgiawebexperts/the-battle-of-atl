"""Author an original reusable produce stall; dimensions in game centimetres."""
import math,json
from pathlib import Path
root=Path(__file__).resolve().parents[1];out=root/'SourceAssets/Terrain/TwelfthMarket/Stall';out.mkdir(exist_ok=True)
groups={k:[] for k in ['Canvas','Metal','Wood','Leaf','Tomato','Cloth']}
def face(m,p):
 for i in range(1,len(p)-1):groups[m].append([p[0],p[i],p[i+1]])
def box(m,c,s):
 v=[(c[0]+x*s[0]/2,c[1]+y*s[1]/2,c[2]+z*s[2]/2) for z in [-1,1] for x,y in [(-1,-1),(1,-1),(1,1),(-1,1)]]
 for ids in [(3,2,1,0),(4,5,6,7),(0,1,5,4),(1,2,6,5),(2,3,7,6),(3,0,4,7)]:face(m,[v[i] for i in ids])
def pole(a,b,r=1.5):
 import numpy as np
 a,b=np.array(a),np.array(b);d=(b-a)/np.linalg.norm(b-a);u=np.cross(d,[0,0,1] if abs(d[2])<.9 else [0,1,0]);u/=np.linalg.norm(u);v=np.cross(d,u)
 rings=[[tuple(p+r*(u*math.cos(i*math.tau/8)+v*math.sin(i*math.tau/8))) for i in range(8)] for p in [a,b]]
 for i in range(8):j=(i+1)%8;face('Metal',[rings[0][i],rings[0][j],rings[1][j],rings[1][i]])
 face('Metal',rings[0][::-1]);face('Metal',rings[1])
# Four gently sagging cloth panels with a valance and narrow metal frame.
corners=[(-150,-150),(150,-150),(150,150),(-150,150)]
for k,(x,y) in enumerate(corners):
 pole((x,y,0),(x,y,220),2);pole((x,y,220),(0,0,285));nx,ny=corners[(k+1)%4];pole((x,y,215),(nx,ny,215))
 for i in range(20):
  t0=i/20;t1=(i+1)/20
  a=(x+(nx-x)*t0,y+(ny-y)*t0,220-3*math.sin(math.pi*t0));b=(x+(nx-x)*t1,y+(ny-y)*t1,220-3*math.sin(math.pi*t1))
  face('Canvas',[(0,0,285),a,b]);face('Canvas',[a,(a[0],a[1],201),(b[0],b[1],201),b])
 box('Metal',(x,y,3),(16,16,6))
# Folding vendor table faces the centre aisle (-Y).
box('Wood',(0,-60,78),(220,65,5));box('Cloth',(0,-60,81),(224,69,1));box('Cloth',(0,-95,66),(224,1,30))
for x in [-90,90]:
 for y in [-80,-40]:pole((x,y,0),(x,y,77),1.4)
for cx in [-73,0,73]:
 box('Wood',(cx,-60,87),(63,50,3))
 for y in [-86,-34]:
  for z in [92,99,106]:box('Wood',(cx,y,z),(63,2,5))
 for x in [cx-32,cx+32]:
  for z in [92,99,106]:box('Wood',(x,-60,z),(2,50,5))
 for row in range(3):
  for col in range(4):
   px,py=cx-22+col*14,-76+row*15
   for lat in range(5):
    for lon in range(10):
     def point(i,j):
      t=math.pi*i/5;a=math.tau*j/10;return(px+6*math.sin(t)*math.cos(a),py+6*math.sin(t)*math.sin(a),99+5*math.cos(t))
     face('Leaf' if cx==0 else 'Tomato',[point(lat,lon),point(lat+1,lon),point(lat+1,lon+1),point(lat,lon+1)])
manifest=[]
for name,triangles in groups.items():
 lines=['# Original authored market stall geometry, centimetres']
 for tri in triangles:
  for p in tri:lines.append('v '+' '.join(f'{v:.4f}' for v in p))
 for i in range(len(triangles)):lines.append(f'f {i*3+1} {i*3+2} {i*3+3}')
 file=f'SM_MarketStall_{name}.obj';(out/file).write_text('\n'.join(lines)+'\n');manifest.append({'file':file,'material':name,'triangles':len(triangles)})
(out/'manifest.json').write_text(json.dumps({'status':'source geometry; native material/import/render/collision acceptance pending','dimensions_cm':[300,300,285],'surfaces':manifest},indent=2)+'\n');print(manifest)
