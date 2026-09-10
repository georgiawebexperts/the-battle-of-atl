import unreal,json,pathlib,array,traceback
p=pathlib.Path(unreal.Paths.project_dir());m=json.loads((p/'SourceAssets/Terrain/terrain-georeference.json').read_text());raw=array.array('H');raw.frombytes((p/'SourceAssets/Terrain/atlanta-height.r16').read_bytes())
w=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world();nx,ny=m['size'];loc=m['unreal_location_cm'];scale=m['unreal_scale'];rows=[]
try:
 for mode,dx,dy,radius in [('surface',.25,.35,0.),('corner_contact',0.,0.,2.)]:
  for ix in [12,nx//4,nx//2,3*nx//4,nx-13]:
   for iy in [12,ny//4,ny//2,3*ny//4,ny-13]:
    x=loc[0]+(ix+dx)*scale[0];y=loc[1]+(iy+dy)*scale[1]
    a=raw[iy*nx+ix];b=raw[iy*nx+ix+1];c=raw[(iy+1)*nx+ix+1];d=raw[(iy+1)*nx+ix]
    value=a+(b-a)*dx+(c-b)*dy if dy<=dx else a+(c-d)*dx+(d-a)*dy
    expected=(value-32768)*scale[2]/128
    hit=unreal.PiedmontWorldTools.trace_world_surface(unreal.Vector(x,y,expected+300),unreal.Vector(x,y,expected-300),radius)
    if hit:
     point,actor=hit;error=abs(point.z-expected)
     rows.append({'mode':mode,'sample':[ix,iy],'expected_z':expected,'collision_z':point.z,'actor':actor.get_actor_label() if actor else None,'error_cm':error,'pass':bool(actor and isinstance(actor,unreal.Landscape) and error<2)})
    else:rows.append({'mode':mode,'sample':[ix,iy],'pass':False,'error':'No terrain contact'})
 r={'status':'passed' if rows and all(x['pass'] for x in rows) else 'failed','map':w.get_name(),'scope':'25 interior surface rays and 25 finite-radius grid-corner contacts; full riding acceptance remains separate','samples':rows}
except Exception:r={'status':'error','error':traceback.format_exc(),'samples':rows}
(p/'Scripts/measured-landscape-validation.json').write_text(json.dumps(r,indent=2))
