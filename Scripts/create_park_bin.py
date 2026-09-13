"""Original slatted park trash bin in centimetres; no third-party geometry."""
import math,json,struct
from pathlib import Path
root=Path(__file__).resolve().parents[1];out=root/'SourceAssets/Environment/ParkBin';out.mkdir(parents=True,exist_ok=True)
groups=[[],[],[]]
def face(p,hint,mat):
 a,b,c=p[:3];u=[b[i]-a[i] for i in range(3)];v=[c[i]-a[i] for i in range(3)];n=[u[1]*v[2]-u[2]*v[1],u[2]*v[0]-u[0]*v[2],u[0]*v[1]-u[1]*v[0]]
 if sum(x*x for x in n)<1e-12:return
 if sum(n[i]*hint[i] for i in range(3))<0:p=list(reversed(p))
 for i in range(1,len(p)-1):groups[mat].extend([p[0],p[i],p[i+1]])
def ring(ro,ri,z0,z1,mat):
 for i in range(48):
  a,b=i*math.tau/48,(i+1)*math.tau/48
  def p(r,t,z):return (r*math.cos(t),r*math.sin(t),z)
  n=(math.cos((a+b)/2),math.sin((a+b)/2),0)
  face([p(ro,a,z0),p(ro,b,z0),p(ro,b,z1),p(ro,a,z1)],n,mat)
  if ri:face([p(ri,a,z0),p(ri,b,z0),p(ri,b,z1),p(ri,a,z1)],tuple(-x for x in n),mat)
  for z,s in [(z0,-1),(z1,1)]:
   if ri:face([p(ro,a,z),p(ro,b,z),p(ri,b,z),p(ri,a,z)],(0,0,s),mat)
   else:face([p(ro,a,z),p(ro,b,z),(0,0,z)],(0,0,s),mat)
def rib(angle):
 v=[]
 for z in [6,79]:
  for r,t in [(25.8,-2.2),(29,-2.2),(29,2.2),(25.8,2.2)]:v.append((r*math.cos(angle)-t*math.sin(angle),r*math.sin(angle)+t*math.cos(angle),z))
 center=tuple(sum(p[k] for p in v)/8 for k in range(3))
 for ids in [[0,1,2,3],[4,5,6,7],[0,1,5,4],[1,2,6,5],[2,3,7,6],[3,0,4,7]]:
  p=[v[j] for j in ids];face(p,tuple(sum(x[k] for x in p)/4-center[k] for k in range(3)),0)
ring(29.5,0,0,6,0)
ring(24.5,24,6,80,1)
ring(24,0,10,11,1)
for i in range(24):rib(i*math.tau/24)
ring(30,24,76,79,2)
ring(31,24,79,83,2)
ring(30,25,83,85,2)
blob=bytearray();views=[];accessors=[];primitives=[]
for material,points in enumerate(groups):
 positions=[(x/100,z/100,-y/100) for x,y,z in points];normals=[]
 for j in range(0,len(positions),3):
  a,b,c=positions[j:j+3];u=[b[i]-a[i] for i in range(3)];v=[c[i]-a[i] for i in range(3)];n=[u[1]*v[2]-u[2]*v[1],u[2]*v[0]-u[0]*v[2],u[0]*v[1]-u[1]*v[0]];length=math.sqrt(sum(x*x for x in n));normals.extend([tuple(x/length for x in n)]*3)
 attrs={}
 for name,data in [('POSITION',positions),('NORMAL',normals),('TEXCOORD_0',[(0.,0.),(1.,0.),(0.,1.)]*(len(positions)//3))]:
  offset=len(blob)
  for row in data:blob.extend(struct.pack('<'+'f'*len(row),*row))
  views.append({'buffer':0,'byteOffset':offset,'byteLength':len(blob)-offset,'target':34962})
  acc={'bufferView':len(views)-1,'componentType':5126,'count':len(data),'type':'VEC2' if name=='TEXCOORD_0' else 'VEC3'}
  if name=='POSITION':acc.update(min=[min(p[i] for p in data) for i in range(3)],max=[max(p[i] for p in data) for i in range(3)])
  accessors.append(acc);attrs[name]=len(accessors)-1
 primitives.append({'attributes':attrs,'material':material,'mode':4})
mats=[{'name':n,'pbrMetallicRoughness':{'baseColorFactor':c,'metallicFactor':metal,'roughnessFactor':rough}} for n,c,metal,rough in [('ParkGreen',[.035,.085,.055,1],.45,.52),('Liner',[.009,.012,.011,1],0,.85),('Rim',[.055,.12,.08,1],.55,.4)]]
doc={'asset':{'version':'2.0','generator':'The Battle of ATL original prop'},'scene':0,'scenes':[{'nodes':[0]}],'nodes':[{'mesh':0,'name':'SM_ParkBin'}],'meshes':[{'name':'SM_ParkBin','primitives':primitives}],'materials':mats,'buffers':[{'uri':'ParkBin.bin','byteLength':len(blob)}],'bufferViews':views,'accessors':accessors}
(out/'ParkBin.bin').write_bytes(blob);(out/'ParkBin.gltf').write_text(json.dumps(doc,indent=2)+'\n');print({'triangles':sum(len(g) for g in groups)//3,'bytes':len(blob)})
