import unreal,json,pathlib,array,traceback
p=pathlib.Path(unreal.Paths.project_dir());m=json.loads((p/'SourceAssets/Terrain/terrain-georeference.json').read_text());raw=array.array('H');raw.frombytes((p/'SourceAssets/Terrain/atlanta-height.r16').read_bytes())
w=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world();nx,ny=m['size'];loc=m['unreal_location_cm'];scale=m['unreal_scale'];rows=[]
try:
 for ix in [12,nx//4,nx//2,3*nx//4,nx-13]:
  for iy in [12,ny//4,ny//2,3*ny//4,ny-13]:
   x=loc[0]+ix*scale[0];y=loc[1]+iy*scale[1];expected=(raw[iy*nx+ix]-32768)*scale[2]/128
   hit=unreal.SystemLibrary.line_trace_single(w,unreal.Vector(x,y,10000),unreal.Vector(x,y,-10000),unreal.TraceTypeQuery.TRACE_TYPE_QUERY1,False,[],unreal.DrawDebugTrace.NONE)
   if hit:
    point=hit.get_editor_property('impact_point')
    rows.append({'sample':[ix,iy],'expected_z':expected,'collision_z':point.z,'error_cm':abs(point.z-expected),'pass':abs(point.z-expected)<2})
   else:rows.append({'sample':[ix,iy],'pass':False,'error':'No terrain collision'})
 r={'status':'passed' if all(x['pass'] for x in rows) else 'failed','map':w.get_name(),'samples':rows}
except Exception:r={'status':'error','error':traceback.format_exc(),'samples':rows}
(p/'Scripts/measured-landscape-validation.json').write_text(json.dumps(r,indent=2))
