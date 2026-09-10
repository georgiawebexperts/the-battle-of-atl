"""Verify installed pavement positions and collision against exported triangles."""
import unreal,json,pathlib,math
p=pathlib.Path(unreal.Paths.project_dir());base=p/'SourceAssets/Terrain/ParkPavement';manifest=json.loads((base/'manifest.json').read_text());ea=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
actors=ea.get_all_level_actors();rows=[]
for chunk in manifest['chunks']:
 vertices=[];faces=[]
 for line in (base/chunk['file']).read_text().splitlines():
  if line.startswith('v '):
   x,y,z=map(float,line.split()[1:]);vertices.append((x,-y,z))
  elif line.startswith('f '):faces.append([int(i.split('/')[0])-1 for i in line.split()[1:]])
 step=max(1,len(faces)//12)
 for index in range(0,len(faces),step):
  triangle=[vertices[i] for i in faces[index]];v=[sum(t[k] for t in triangle)/3 for k in range(3)]
  hit=unreal.PiedmontWorldTools.trace_world_surface(unreal.Vector(v[0],v[1],v[2]+100),unreal.Vector(v[0],v[1],v[2]-100))
  point,actor=hit if hit else (None,None)
  label='Park pavement SM_'+pathlib.Path(chunk['file']).stem
  okay=bool(actor and actor.get_actor_label()==label and abs(point.z-v[2])<1)
  rows.append({'mesh':chunk['file'],'triangle':index,'expected_z':v[2],'actual_z':point.z if point else None,'actor':actor.get_actor_label() if actor else None,'pass':okay})
r={'status':'passed' if all(x['pass'] for x in rows) else 'failed','scope':'Static installed pavement collision samples; full ride-through and bridges remain unverified','samples':len(rows),'passed':sum(x['pass'] for x in rows),'spline_count':sum(isinstance(a,unreal.PiedmontPathSpline) for a in actors),'results':rows}
(p/'Scripts/park-world-validation.json').write_text(json.dumps(r,indent=2))
unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).set_level_viewport_camera_info(unreal.Vector(-12000,-25000,45000),unreal.Rotator(pitch=-55,yaw=75,roll=0))
