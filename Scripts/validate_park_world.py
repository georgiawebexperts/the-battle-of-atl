import unreal,json,pathlib
if globals().get('WORLD_JOB',{}).get('inspect_actor'):
 name=WORLD_JOB['inspect_actor'];rows=[]
 for a in unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors():
  if a.get_name()==name:rows.append({'name':a.get_name(),'label':a.get_actor_label(),'tags':[str(t) for t in a.tags],'bounds':str(a.get_actor_bounds(False))})
 (pathlib.Path(unreal.Paths.project_dir())/'Scripts/actor-inspection.json').write_text(json.dumps(rows,indent=2))
elif globals().get('WORLD_JOB',{}).get('inspect_bridges'):
 rows=[]
 for a in unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors():
  if not isinstance(a,unreal.PiedmontPathSpline) or not a.get_editor_property('bridge'):continue
  count=a.centerline.get_number_of_spline_points();v=a.centerline.get_location_at_spline_point(count//2,unreal.SplineCoordinateSpace.WORLD)
  hit=unreal.PiedmontWorldTools.trace_world_surface(v+unreal.Vector(0,0,80),v-unreal.Vector(0,0,80));okay=bool(hit and hit[1].actor_has_tag('RideBridge') and abs(hit[0].z-v.z)<2)
  rows.append({'osm_id':a.get_editor_property('osm_way_id'),'pass':okay,'deck':hit[1].get_actor_label() if hit else None,'height_error_cm':abs(hit[0].z-v.z) if hit else None})
 (pathlib.Path(unreal.Paths.project_dir())/'Scripts/installed-bridge-coverage.json').write_text(json.dumps({'status':'passed' if len(rows)==14 and all(r['pass'] for r in rows) else 'failed','scope':'One installed deck sample per sourced bridge way; not full rideability or architectural acceptance','ways':rows},indent=2))
elif globals().get('WORLD_JOB',{}).get('inspect_water'):
 result=[]
 for a in unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors():
  if isinstance(a,(unreal.WaterBodyLake,unreal.WaterZone)):
   r={'actor':a.get_actor_label(),'location':str(a.get_actor_location()),'bounds':str(a.get_actor_bounds(False)),'components':[]}
   for c in a.get_components_by_class(unreal.PrimitiveComponent):
    item={'class':c.get_class().get_name(),'name':c.get_name(),'bounds':str(unreal.SystemLibrary.get_component_bounds(c)),'visible':c.is_visible()}
    if isinstance(c,unreal.StaticMeshComponent):item['mesh']=str(c.static_mesh);item['material']=str(c.get_material(0))
    r['components'].append(item)
   result.append(r)
 (pathlib.Path(unreal.Paths.project_dir())/'Scripts/water-render-inspection.json').write_text(json.dumps(result,indent=2))
elif globals().get('WORLD_JOB',{}).get('inspect_shore'):
 import math
 p=pathlib.Path(unreal.Paths.project_dir());vertices=[];faces=[]
 for line in (p/'SourceAssets/Terrain/LakeShoreCollision.obj').read_text().splitlines():
  if line.startswith('v '):
   x,y,z=map(float,line.split()[1:]);vertices.append((x,-y,z))
  elif line.startswith('f '):faces.append([int(t.split('/')[0])-1 for t in line.split()[1:]])
 rows=[]
 for i in range(0,len(faces),100):
  a,b,c=[vertices[k] for k in faces[i]];dx=b[0]-a[0];dy=b[1]-a[1]
  if math.hypot(dx,dy)<.01:dx=c[0]-a[0];dy=c[1]-a[1]
  length=math.hypot(dx,dy)
  if length<.01:continue
  nx,ny=-dy/length,dx/length;q=[(a[k]+b[k]+c[k])/3 for k in range(3)]
  hit=unreal.PiedmontWorldTools.trace_world_surface(unreal.Vector(q[0]+nx*5,q[1]+ny*5,q[2]),unreal.Vector(q[0]-nx*5,q[1]-ny*5,q[2]))
  okay=bool(hit and hit[1].get_actor_label()=='Lake Clara Meer solid shore')
  rows.append({'face':i,'pass':okay,'actor':hit[1].get_actor_label() if hit else None})
 (p/'Scripts/shore-collision-validation.json').write_text(json.dumps({'status':'passed' if rows and all(r['pass'] for r in rows) else 'failed','scope':'Short horizontal collision rays across sampled shoreline wall faces; bridge openings remain unverified','samples':len(rows),'results':rows},indent=2))
else:
 """Verify installed pavement positions and collision against exported triangles."""
 import unreal,json,pathlib,math
 p=pathlib.Path(unreal.Paths.project_dir());plazas=globals().get('WORLD_JOB',{}).get('collection')=='plazas';base=p/('SourceAssets/Terrain/ParkPlazas' if plazas else 'SourceAssets/Terrain/ParkPavement');manifest=json.loads((base/'manifest.json').read_text());ea=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
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
 (p/('Scripts/park-plazas-validation.json' if plazas else 'Scripts/park-world-validation.json')).write_text(json.dumps(r,indent=2))
 unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).set_level_viewport_camera_info(unreal.Vector(-12000,-25000,45000),unreal.Rotator(pitch=-55,yaw=75,roll=0))
