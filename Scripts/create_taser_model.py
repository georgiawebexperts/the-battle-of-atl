"""Author a lightweight fictional yellow/black taser prop in centimetres."""
import math,json,struct
from pathlib import Path
root=Path(__file__).resolve().parents[1];out=root/'SourceAssets/Weapons/Taser';out.mkdir(parents=True,exist_ok=True)
groups=[[],[],[]]
def solid(profile,width,material):
 centre=(sum(p[0] for p in profile)/len(profile),0,sum(p[1] for p in profile)/len(profile));n=len(profile)
 v=[(x,y,z) for y in [-width/2,width/2] for x,z in profile]
 faces=[]
 for side in [0,n]:
  for i in range(1,n-1):faces.append([side,side+i,side+i+1])
 for i in range(n):
  j=(i+1)%n;faces.extend([[i,j,n+j],[i,n+j,n+i]])
 for face in faces:
  pts=[v[i] for i in face];a,b,c=pts;u=[b[i]-a[i] for i in range(3)];w=[c[i]-a[i] for i in range(3)];normal=(u[1]*w[2]-u[2]*w[1],u[2]*w[0]-u[0]*w[2],u[0]*w[1]-u[1]*w[0])
  if sum(normal[i]*((a[i]+b[i]+c[i])/3-centre[i]) for i in range(3))<0:pts.reverse()
  groups[material].extend(pts)
def box(x0,x1,z0,z1,width,mat,bevel=.3):
 c=min(bevel,(x1-x0)/3,(z1-z0)/3)
 solid([(x0+c,z0),(x1-c,z0),(x1,z0+c),(x1,z1-c),(x1-c,z1),(x0+c,z1),(x0,z1-c),(x0,z0+c)],width,mat)
box(-7,10,0,7,4.8,0,.8) # yellow housing
box(8,17,1,6.7,5.2,1,.5) # dark cartridge
solid([(-5,1),(1,1),(-.3,-12),(-5.4,-12),(-7,-10)],3.8,1)
box(-4.5,0,-10,-2,4.05,0,.4)
for z in [-8,-6,-4]:box(-4.6,.1,z,z+.55,4.25,1,.1)
box(1,7,-5,-3.8,1.1,1,.2);box(6,7,-4,1,1.1,1,.2) # open trigger guard
box(2.2,3,-2.7,.3,1,1,.1)
box(-5,-2,7,7.7,1.8,1,.2)
box(15.5,17.5,2,2.7,4.1,2,.1);box(15.5,17.5,5,5.7,4.1,2,.1)
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
mats=[{'name':n,'pbrMetallicRoughness':{'baseColorFactor':c,'metallicFactor':metal,'roughnessFactor':rough}} for n,c,metal,rough in [('SafetyYellow',[.9,.55,.025,1],.08,.38),('Polymer',[.012,.016,.022,1],.05,.48),('Contacts',[.35,.4,.44,1],.85,.22)]]
doc={'asset':{'version':'2.0','generator':'The Battle of ATL original prop'},'scene':0,'scenes':[{'nodes':[0]}],'nodes':[{'mesh':0,'name':'SM_Taser'}],'meshes':[{'name':'SM_Taser','primitives':primitives}],'materials':mats,'buffers':[{'uri':'Taser.bin','byteLength':len(blob)}],'bufferViews':views,'accessors':accessors}
(out/'Taser.bin').write_bytes(blob);(out/'Taser.gltf').write_text(json.dumps(doc,indent=2)+'\n');print({'triangles':sum(len(g) for g in groups)//3,'bytes':len(blob)})
