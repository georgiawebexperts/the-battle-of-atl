"""Draft local path grading, preserving the lake and all terrain outside three small patches.

No Unreal assets or maps are changed. Source DEM identity is checked against
native collision measurements before preparing the candidate and its pavement.
"""
from pathlib import Path
import json,hashlib
import numpy as np
import shapely
from shapely.geometry import Polygon
root=Path(__file__).resolve().parents[1];terrain=root/'SourceAssets/Terrain'
out=terrain/'LakePathGrading';out.mkdir(exist_ok=True)
m=json.loads((terrain/'terrain-georeference.json').read_text());sx,sy,sz=m['unreal_scale'];lx,ly,_=m['unreal_location_cm'];nx,ny=m['size']
basefile=terrain/'atlanta-height-tenth-graded.r16';raw=np.fromfile(basefile,dtype='<u2').reshape(ny,nx);z=(raw.astype(float)-32768)*sz/128
survey=json.loads((root/'work/lake-path-grades.json').read_text())
def sample(field,x,y):
 gx=(x-lx)/sx;gy=(-y-ly)/sy;i,j=int(gx),int(gy);u,v=gx-i,gy-j
 if not (0<=i<nx-1 and 0<=j<ny-1):return 0.
 a,b,c,d=field[j,i],field[j,i+1],field[j+1,i],field[j+1,i+1]
 return a+(b-a)*u+(d-b)*v if u>=v else a+(d-c)*u+(c-a)*v
errors=[abs(r['xyz'][2]-sample(z,*r['xyz'][:2])-3) for r in survey['samples'] if r['actor'].startswith('Park pavement')]
assert errors and max(errors)<.01, 'Source DEM does not match current native pavement survey'
segments=[s for s in survey['segments'] if s['tags'].get('bridge')!='yes' and all(a.startswith('Park pavement') for a in s['actors'])]
centers=[(-5050.,-1925.),(-9000.,460.),(-5000.,-3200.)];radius=1000.;inner=400.;sigma=4.
# Nine-meter real-world smoothing kernel, only within three 10m game-space circles.
k=np.arange(-16,17);kernel=np.exp(-.5*(k/sigma)**2);kernel/=kernel.sum()
smooth=np.apply_along_axis(lambda a:np.convolve(np.pad(a,(16,16),mode='edge'),kernel,mode='valid'),0,z)
smooth=np.apply_along_axis(lambda a:np.convolve(np.pad(a,(16,16),mode='edge'),kernel,mode='valid'),1,smooth)
xx=lx+np.arange(nx)*sx;yy=-(ly+np.arange(ny)*sy);X,Y=np.meshgrid(xx,yy);weight=np.zeros(z.shape)
for x,y in centers:
 t=np.clip((radius-np.hypot(X-x,Y-y))/(radius-inner),0,1);weight=np.maximum(weight,t*t*(3-2*t))
lake=json.loads((terrain/'lake-clara-meer.json').read_text());water=Polygon([(v[0],-v[1]) for v in lake['outer_cm']],holes=[[(v[0],-v[1]) for v in lake['island_cm']]])
indices=np.where(weight>0);dist=shapely.distance(shapely.points(X[indices],Y[indices]),water)
# Preserve water and a 100cm bank margin, taper grading from 100 to 300cm away.
t=np.clip((dist-100)/200,0,1);weight[indices]*=t*t*(3-2*t)
new=np.rint((z+(smooth-z)*weight)*128/sz+32768).clip(0,65535).astype('<u2');changed=new!=raw
assert not np.any(changed[weight==0]);new.tofile(out/'atlanta-height-lake-paths-candidate.r16');newz=(new.astype(float)-32768)*sz/128;delta=newz-z
rows=[]
for s in segments:
 if s['osm_id'] not in [61853018,182460439]:continue
 a,b=s['a'],s['b'];grade=(sample(newz,*b[:2])-sample(newz,*a[:2]))/s['distance_cm'];rows.append({'osm_id':s['osm_id'],'a':a,'b':b,'grade_before':s['grade'],'grade_candidate':grade})
connections=out/'Connections';connections.mkdir(exist_ok=True);meshes=[]
for collection in ['ParkPavement','ParkPlazas','EastsideTrail','BeltlineConnector']:
 for file in sorted((terrain/collection).glob('*.obj')):
  if 'Bridge' in file.stem:continue
  lines=[];vertices=[];changes=[]
  for line in file.read_text().splitlines():
   parts=line.split()
   if parts and parts[0]=='v':
    x,y,h=map(float,parts[1:4]);dz=sample(delta,x,y);changes.append(abs(dz));h+=dz;vertices.append((x,-y,h));line=f'v {x:.6f} {y:.6f} {h:.6f}'
   lines.append(line)
  if max(changes,default=0)<.001:continue
  (connections/file.name).write_text('\n'.join(lines)+'\n');meshes.append({'file':file.name,'source_collection':collection,'original_mesh':'SM_'+file.stem,'changed_vertices':int(sum(d>.001 for d in changes)),'max_shift_cm':max(changes),'bounds_cm':[[min(v[i] for v in vertices) for i in range(3)],[max(v[i] for v in vertices) for i in range(3)]]})
(connections/'manifest.json').write_text(json.dumps({'chunks':meshes},indent=2)+'\n')
report={'status':'source_candidate_only','source':basefile.name,'source_sha256':hashlib.sha256(basefile.read_bytes()).hexdigest(),'candidate_sha256':hashlib.sha256(new.tobytes()).hexdigest(),'native_source_max_error_cm':max(errors),'centers_world_cm':centers,'radius_cm':radius,'gaussian_sigma_cm':sigma*sx,'lake_and_100cm_bank_unchanged':True,'outside_patches_identical':True,'changed_samples':int(changed.sum()),'maximum_shift_cm':float(np.abs(delta).max()),'target_max_grade_before':max(abs(r['grade_before']) for r in rows),'target_max_grade_candidate':max(abs(r['grade_candidate']) for r in rows),'segments':rows,'mesh_chunks':meshes,'pending':['Native review-map import and collision comparison','Visual review with actual water and camera','Navigation and moving bike checks before main-map installation']}
(out/'manifest.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps({k:v for k,v in report.items() if k not in ['segments','mesh_chunks']},indent=2))
