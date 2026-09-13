"""Flatten the credited source glTF into one Unreal-ready mesh, preserving materials/UVs."""
import copy,json,pathlib,numpy as np
root=pathlib.Path(__file__).resolve().parents[1];p=root/'SourceAssets/Weapons/Remington870'
g=json.loads((p/'scene.gltf').read_text());data=bytearray((p/'scene.bin').read_bytes());parts=[]
def read(index):
 a=g['accessors'][index];v=g['bufferViews'][a['bufferView']]
 return np.ndarray((a['count'],3),dtype='<f4',buffer=data,offset=v.get('byteOffset',0)+a.get('byteOffset',0),strides=(v.get('byteStride',12),4)).copy()
def walk(i,parent):
 n=g['nodes'][i];m=parent@np.array(n.get('matrix',np.eye(4).T.flatten().tolist())).reshape(4,4).T
 if 'mesh' in n:
  for pr in g['meshes'][n['mesh']]['primitives']:
   parts.append((copy.deepcopy(pr),m,read(pr['attributes']['POSITION'])))
 for c in n.get('children',[]):walk(c,m)
for i in g['scenes'][g.get('scene',0)]['nodes']:walk(i,np.eye(4))
world=[v@m[:3,:3].T+m[:3,3] for _,m,v in parts]
lo=np.vstack(world).min(0);hi=np.vstack(world).max(0);center=(lo+hi)/2;scale=1.0/(hi[0]-lo[0])
def append(values):
 while len(data)%4:data.append(0)
 arr=np.asarray(values,dtype='<f4');offset=len(data);data.extend(arr.tobytes());vi=len(g['bufferViews']);g['bufferViews'].append({'buffer':0,'byteOffset':offset,'byteLength':arr.nbytes,'target':34962})
 ai=len(g['accessors']);g['accessors'].append({'bufferView':vi,'componentType':5126,'count':len(arr),'type':'VEC3','min':arr.min(0).tolist(),'max':arr.max(0).tolist()});return ai
primitives=[]
for (pr,m,v),w in zip(parts,world):
 normals=read(pr['attributes']['NORMAL'])@np.linalg.inv(m[:3,:3]);normals/=np.linalg.norm(normals,axis=1,keepdims=True)
 pr['attributes']['POSITION']=append((w-center)*scale);pr['attributes']['NORMAL']=append(normals)
 # Imported normals remain valid; let Unreal calculate tangents from UVs after baking transforms.
 pr['attributes'].pop('TANGENT',None);primitives.append(pr)
g['meshes']=[{'name':'Remington870','primitives':primitives}];g['nodes']=[{'name':'Remington870','mesh':0}];g['scenes']=[{'nodes':[0]}];g['scene']=0;g['buffers']=[{'uri':'Remington870.bin','byteLength':len(data)}]
(p/'Remington870.bin').write_bytes(data);(p/'Remington870.gltf').write_text(json.dumps(g))
(root/'work/remington870-preparation.json').write_text(json.dumps({'source_bounds':[lo.tolist(),hi.tolist()],'length_m':1,'primitives':len(parts),'license':'CC-BY-4.0 per archive','author':'britdawgmasterfunk'},indent=2)+'\n')
print('Prepared',len(parts),'primitives, one-metre length')
