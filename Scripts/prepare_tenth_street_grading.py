"""Prepare a localized terrain-grading candidate; never replace the source DEM.
Only pavement and its transition strip change. Native installation is separate.
"""
import json,math,hashlib
from pathlib import Path
import numpy as np
from shapely import points,distance
from shapely.geometry import Polygon
from shapely.ops import unary_union
root=Path(__file__).resolve().parents[1];folder=root/'SourceAssets/Terrain';meta=json.loads((folder/'terrain-georeference.json').read_text())
source=folder/'atlanta-height-krog.r16';raw=np.fromfile(source,dtype='<u2').reshape(meta['size'][1],meta['size'][0]);original_hash=hashlib.sha256(source.read_bytes()).hexdigest()
sx,sy,sz=meta['unreal_scale'];lx,ly,_=meta['unreal_location_cm']
polygons=[]
for kind in ['Road','CycleTrack','Separator','Sidewalk']:
 vertices=[]
 for line in (folder/'TenthStreet'/('TenthStreet_'+kind+'.obj')).read_text().splitlines():
  p=line.split()
  if p and p[0]=='v':vertices.append((float(p[1]),-float(p[2])))
  elif p and p[0]=='f':polygons.append(Polygon([vertices[int(v.split('/')[0])-1] for v in p[1:]]))
corridor=unary_union(polygons)
# A 6 m blend preserves unmodified terrain beyond the street frontage.
transition=600.;sigma=12.;radius=math.ceil(sigma*3);padding=transition+radius*max(sx,sy)
x0,y0,x1,y1=corridor.bounds
ix0=max(0,math.floor((x0-padding-lx)/sx));ix1=min(raw.shape[1],math.ceil((x1+padding-lx)/sx)+1)
iy0=max(0,math.floor((-y1-padding-ly)/sy));iy1=min(raw.shape[0],math.ceil((-y0+padding-ly)/sy)+1)
patch=raw[iy0:iy1,ix0:ix1].astype(float)
k=np.arange(-radius,radius+1);kernel=np.exp(-.5*(k/sigma)**2);kernel/=kernel.sum()
smoothed=np.apply_along_axis(lambda a:np.convolve(np.pad(a,radius,mode='edge'),kernel,mode='valid'),0,patch)
smoothed=np.apply_along_axis(lambda a:np.convolve(np.pad(a,radius,mode='edge'),kernel,mode='valid'),1,smoothed)
xx,yy=np.meshgrid(lx+np.arange(ix0,ix1)*sx,-(ly+np.arange(iy0,iy1)*sy));samples=points(xx.ravel(),yy.ravel())
dist=distance(samples,corridor).reshape(xx.shape);t=np.clip(1-dist/transition,0,1);weight=t*t*(3-2*t)
# Keep existing building foundations unchanged and feather their protection.
footprints=json.loads((folder/'FancyRoachMotel/footprints.json').read_text())
buildings=unary_union([Polygon([p[:2] for p in row['footprint_world_cm']]) for row in footprints['buildings']])
bd=distance(samples,buildings).reshape(xx.shape);bt=np.clip((bd-80)/220,0,1);weight*=bt*bt*(3-2*bt)
updated=np.rint(patch+(smoothed-patch)*weight).clip(0,65535).astype('<u2');result=raw.copy();result[iy0:iy1,ix0:ix1]=updated
candidate=folder/'atlanta-height-tenth-graded.r16';result.tofile(candidate)
changed=updated!=patch;delta=(updated.astype(float)-patch)*sz/128
assert np.all(updated[weight==0]==patch[weight==0]);assert hashlib.sha256(source.read_bytes()).hexdigest()==original_hash
report={'candidate':candidate.name,'source_sha256':original_hash,'source_unchanged':True,'changed_samples':int(changed.sum()),'max_cut_cm':float(-delta.min()),'max_fill_cm':float(delta.max()),'gaussian_sigma_cm':sigma*sx,'terrain_blend_width_cm':transition,'building_protection_cm':80,'native_installed':False,'accepted':False,'limitations':'Candidate only. Must regenerate matching road meshes, preview terrain transitions, and test native collision and riding before installation.'}
(folder/'TenthStreet/grading.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report))
