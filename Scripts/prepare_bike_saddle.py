"""Author a rounded saddle with a broad rear and a narrow nose, in centimeters."""
from pathlib import Path
import math,json
root=Path(__file__).resolve().parents[1];out=root/'SourceAssets/Bike';out.mkdir(parents=True,exist_ok=True)
# Longitudinal cross sections: X, half width, half thickness, vertical center.
profile=[(-14,0,0,0),(-13.5,3.3,1.2,0),(-12,6.2,1.9,0),(-9,7.8,2.1,0),(-6,8,2.2,0),(-3,7.5,2.1,0),(0,6,1.9,0),(3,4.4,1.7,-.1),(6,3.2,1.55,-.2),(9,2.7,1.5,-.2),(12,2.4,1.4,-.1),(14,1.65,1.1,0),(15,0,0,0)]
n=32;verts=[(profile[0][0],0,profile[0][3])]
for x,w,h,z in profile[1:-1]:
 for j in range(n):
  t=math.tau*j/n;verts.append((x,w*math.sin(t),z+h*math.cos(t)))
end=len(verts);verts.append((profile[-1][0],0,profile[-1][3]));faces=[]
for j in range(n):faces.append((0,1+(j+1)%n,1+j))
for r in range(len(profile)-3):
 for j in range(n):
  a=1+r*n+j;b=1+r*n+(j+1)%n;c=a+n;d=b+n;faces.extend([(a,b,c),(b,d,c)])
last=1+(len(profile)-3)*n
for j in range(n):faces.append((last+j,last+(j+1)%n,end))
# Choose outward winding and accumulate smooth per-vertex normals.
def cross(a,b):return (a[1]*b[2]-a[2]*b[1],a[2]*b[0]-a[0]*b[2],a[0]*b[1]-a[1]*b[0])
normals=[[0.,0.,0.] for _ in verts];oriented=[]
for a,b,c in faces:
 va,vb,vc=verts[a],verts[b],verts[c];v=cross([vb[k]-va[k] for k in range(3)],[vc[k]-va[k] for k in range(3)])
 center=[(va[k]+vb[k]+vc[k])/3 for k in range(3)]
 if sum(v[k]*center[k] for k in range(3))<0:b,c=c,b;v=tuple(-x for x in v)
 oriented.append((a,b,c))
 for i in [a,b,c]:
  for k in range(3):normals[i][k]+=v[k]
lines=['# Authored saddle, centimeters. OBJ Y = negative Unreal Y.','o BikeSaddle']
lines += [f'v {x:.6f} {-y:.6f} {z:.6f}' for x,y,z in verts]
lines += [f'vt {(x+14)/29:.8f} {(y+8)/16:.8f}' for x,y,z in verts]
for v in normals:
 length=math.sqrt(sum(x*x for x in v));assert length>0
 lines.append(f'vn {v[0]/length:.8f} {-v[1]/length:.8f} {v[2]/length:.8f}')
lines += ['f '+' '.join(f'{i+1}/{i+1}/{i+1}' for i in [a,c,b]) for a,b,c in oriented]
(out/'BikeSaddle.obj').write_text('\n'.join(lines)+'\n')
report={'vertices':len(verts),'triangles':len(faces),'bounds_cm':[[min(v[k] for v in verts) for k in range(3)],[max(v[k] for v in verts) for k in range(3)]],'purpose':'Rounded rear support and tapered nose instead of the old 29x19x5.5 cm box; static decorative mesh, no collision.'}
(out/'BikeSaddle.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report))
