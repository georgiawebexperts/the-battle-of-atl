from pathlib import Path
import math
root=Path(__file__).resolve().parents[1]/'SourceAssets/Bike';root.mkdir(parents=True,exist_ok=True)
lines=['mtllib BikeWheel.mtl'];count=0

def torus(R,r,material,N=64,M=10):
 global count
 base=count;lines.append('usemtl '+material)
 for i in range(N):
  a=2*math.pi*i/N
  for j in range(M):
   b=2*math.pi*j/M
   lines.append('v %.6f %.6f %.6f'%((R+r*math.cos(b))*math.cos(a),(R+r*math.cos(b))*math.sin(a),r*math.sin(b)));count+=1
 for i in range(N):
  for j in range(M):
   q=[base+i*M+j+1,base+((i+1)%N)*M+j+1,base+((i+1)%N)*M+(j+1)%M+1,base+i*M+(j+1)%M+1]
   lines.append('f %d %d %d %d'%tuple(q))
def cylinder(a,b,r,material,n=8):
 global count
 v=[b[i]-a[i] for i in range(3)];l=math.sqrt(sum(x*x for x in v));v=[x/l for x in v]
 ref=[0,0,1] if abs(v[2])<.9 else [1,0,0]
 u=[v[1]*ref[2]-v[2]*ref[1],v[2]*ref[0]-v[0]*ref[2],v[0]*ref[1]-v[1]*ref[0]];l=math.sqrt(sum(x*x for x in u));u=[x/l for x in u]
 w=[v[1]*u[2]-v[2]*u[1],v[2]*u[0]-v[0]*u[2],v[0]*u[1]-v[1]*u[0]];base=count;lines.append('usemtl '+material)
 for c in [a,b]:
  for i in range(n):
   t=2*math.pi*i/n;lines.append('v %.6f %.6f %.6f'%tuple(c[k]+r*(u[k]*math.cos(t)+w[k]*math.sin(t)) for k in range(3)));count+=1
 for i in range(n):lines.append('f %d %d %d %d'%(base+i+1,base+(i+1)%n+1,base+n+(i+1)%n+1,base+n+i+1))
torus(32.5,2.5,'Rubber');torus(29.5,.7,'Alloy')
for i in range(28):
 t=2*math.pi*i/28;cylinder((0,0,2 if i%2 else -2),(29.5*math.cos(t),29.5*math.sin(t),0),.14,'Alloy')
cylinder((0,0,-4),(0,0,4),3.6,'Alloy',24)
(root/'BikeWheel.obj').write_text('\n'.join(lines))
(root/'BikeWheel.mtl').write_text('newmtl Rubber\nKd 0.018 0.022 0.025\nNs 20\nnewmtl Alloy\nKd 0.55 0.58 0.6\nNs 120\n')
print('Authored wheel:',count,'vertices')
