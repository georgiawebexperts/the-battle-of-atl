"""Independently check exported OBJ faces against quantized terrain heights."""
from pathlib import Path
import json,sys,numpy as np
P=Path(__file__).resolve().parents[1];O=P/'SourceAssets/Terrain';plazas='--plazas' in sys.argv;B=O/('ParkPlazas' if plazas else 'ParkPavement')
m=json.loads((O/'terrain-georeference.json').read_text());manifest=json.loads((B/'manifest.json').read_text());nx,ny=m['size']
h=(np.fromfile(O/'atlanta-height.r16',dtype='<u2').reshape(ny,nx).astype(float)-32768)*m['unreal_scale'][2]/128
rows=[]
for chunk in manifest['chunks']:
 vertices=[];faces=[]
 for line in (B/chunk['file']).read_text().splitlines():
  if line.startswith('v '):vertices.append([float(x) for x in line.split()[1:]])
  elif line.startswith('f '):faces.append([int(x.split('/')[0])-1 for x in line.split()[1:]])
 v=np.array(vertices);v[:,1]*=-1;f=np.array(faces)[:,::-1];tri=v[f];points=np.concatenate([v,tri.mean(axis=1)])
 gx=(points[:,0]-m['unreal_location_cm'][0])/m['unreal_scale'][0];gy=(points[:,1]-m['unreal_location_cm'][1])/m['unreal_scale'][1]
 ix=np.clip(np.floor(gx).astype(int),0,nx-2);iy=np.clip(np.floor(gy).astype(int),0,ny-2);u=gx-ix;t=gy-iy
 a=h[iy,ix];b=h[iy,ix+1];c=h[iy+1,ix+1];d=h[iy+1,ix]
 expected=np.where(t<=u,a+(b-a)*u+(c-b)*t,a+(c-d)*u+(d-a)*t)
 error=np.max(np.abs(points[:,2]-expected-3))
 cross=np.cross(tri[:,1]-tri[:,0],tri[:,2]-tri[:,0]);up=np.all(cross[:,2]>0)
 rows.append({'file':chunk['file'],'triangles':len(f),'max_clearance_error_cm':float(error),'upward_faces':bool(up),'pass':bool(error<.01 and up and np.isfinite(v).all())})
report={'status':'passed' if rows and all(r['pass'] for r in rows) else 'failed','scope':'Exported source mesh geometry only; Unreal rendering/collision and rideability not yet tested','chunks':len(rows),'triangles':sum(r['triangles'] for r in rows),'worst_clearance_error_cm':max(r['max_clearance_error_cm'] for r in rows),'results':rows}
(P/('Tests/Results/2026-09-10-park-plazas-source.json' if plazas else 'Tests/Results/2026-09-10-park-pavement-source.json')).write_text(json.dumps(report,indent=2));print(json.dumps({k:v for k,v in report.items() if k!='results'},indent=2))
if report['status']!='passed':raise SystemExit(1)
