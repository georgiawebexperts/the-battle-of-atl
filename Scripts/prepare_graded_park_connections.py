"""Preserve baked path topology/clearance while matching the graded DEM."""
import json
from pathlib import Path
import numpy as np
root=Path(__file__).resolve().parents[1];terrain=root/'SourceAssets/Terrain';meta=json.loads((terrain/'terrain-georeference.json').read_text());sx,sy,sz=meta['unreal_scale'];lx,ly,_=meta['unreal_location_cm']
source=np.fromfile(terrain/'atlanta-height-krog.r16',dtype='<u2').reshape(meta['size'][1],meta['size'][0]);graded=np.fromfile(terrain/'atlanta-height-tenth-graded.r16',dtype='<u2').reshape(source.shape);delta=(graded.astype(float)-source)*sz/128
out=terrain/'TenthStreetGraded/Connections';out.mkdir(parents=True,exist_ok=True);rows=[]
def shift(x,y):
 gx=(x-lx)/sx;gy=(-y-ly)/sy;ix=int(gx);iy=int(gy);dx=gx-ix;dy=gy-iy
 if not (0<=ix<delta.shape[1]-1 and 0<=iy<delta.shape[0]-1):return 0
 a,b,c,d=delta[iy,ix],delta[iy,ix+1],delta[iy+1,ix],delta[iy+1,ix+1]
 return a+(b-a)*dx+(d-b)*dy if dx>=dy else a+(d-c)*dx+(c-a)*dy
for collection in ['ParkPavement','ParkPlazas','EastsideTrail','BeltlineConnector']:
 for path in sorted((terrain/collection).glob('*.obj')):
  if 'Bridge' in path.stem:continue
  lines=[];changes=[];vertices=[]
  for line in path.read_text().splitlines():
   parts=line.split()
   if parts and parts[0]=='v':
    x,y,z=map(float,parts[1:4]);dz=shift(x,y);z+=dz;changes.append(abs(dz));vertices.append((x,-y,z));line=f'v {x:.6f} {y:.6f} {z:.6f}'
   lines.append(line)
  if max(changes,default=0)<.001:continue
  (out/path.name).write_text('\n'.join(lines)+'\n');rows.append({'file':path.name,'source_collection':collection,'original_mesh':'SM_'+path.stem,'changed_vertices':int(sum(d>.001 for d in changes)),'max_shift_cm':max(changes),'bounds_cm':[[min(v[i] for v in vertices) for i in range(3)],[max(v[i] for v in vertices) for i in range(3)]]})
(out/'manifest.json').write_text(json.dumps({'chunks':rows,'scope':'Matching existing path topology to terrain delta; native contact and navigation not yet verified.'},indent=2)+'\n');print(json.dumps(rows))
