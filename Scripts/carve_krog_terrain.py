"""Carve only the tunnel corridor while preserving the existing lakebed terrain."""
from pathlib import Path
import json,hashlib,math,numpy as np
from shapely.geometry import Point
from shapely.ops import substring
from route_height_profiles import RouteHeightProfiles
root=Path(__file__).resolve().parents[1];base=root/'SourceAssets/Terrain';meta=json.loads((base/'terrain-georeference.json').read_text());h=RouteHeightProfiles(base/'krog-height-profiles.json');p=h.profiles[0]
original=np.fromfile(base/'atlanta-height-lakebed.r16',dtype='<u2').reshape(meta['size'][1],meta['size'][0]);result=original.copy();loc=meta['unreal_location_cm'];scale=meta['unreal_scale'];line=h.line
bounds=substring(line,p['blend_start_cm'],p['blend_end_cm']).buffer(1000).bounds
xmin=max(0,math.floor((bounds[0]-loc[0])/scale[0]));xmax=min(original.shape[1]-1,math.ceil((bounds[2]-loc[0])/scale[0]));ymin=max(0,math.floor((bounds[1]-loc[1])/scale[1]));ymax=min(original.shape[0]-1,math.ceil((bounds[3]-loc[1])/scale[1]))
allowed=np.zeros(original.shape,dtype=bool)
for iy in range(ymin,ymax+1):
 for ix in range(xmin,xmax+1):
  x=loc[0]+ix*scale[0];y=loc[1]+iy*scale[1];q=Point(x,y);s=line.project(q);nearest=np.array(line.interpolate(s).coords[0]);tangent=np.array(line.interpolate(min(line.length,s+1)).coords[0])-np.array(line.interpolate(max(0,s-1)).coords[0]);normal=np.array([-tangent[1],tangent[0]])/np.linalg.norm(tangent);distance=abs(np.dot(np.array([x,y])-nearest,normal)+140)
  if not p['blend_start_cm']<=s<=p['blend_end_cm'] or distance>=780:continue
  weight=max(0,min(1,(780-distance)/200));weight=weight*weight*(3-2*weight)
  z=(int(original[iy,ix])-32768)*scale[2]/128
  floor=h.height(x,y,z+3);target=min(z,floor-32);new=z+(target-z)*weight
  quantized=round(new*128/scale[2]+32768);assert 0<=quantized<=65535
  result[iy,ix]=quantized;allowed[iy,ix]=True
changed=result!=original
assert changed.any() and np.array_equal(result[~allowed],original[~allowed])
assert np.all(result<=original)
output=base/'atlanta-height-krog.r16';result.astype('<u2').tofile(output)
report={'passed':True,'base':'atlanta-height-lakebed.r16','output':output.name,'changed_samples':int(changed.sum()),'allowed_samples':int(allowed.sum()),'outside_corridor_identical':True,'only_lowered':True,'max_lowering_game_cm':float((original.astype(float)-result.astype(float)).max()*scale[2]/128),'source_bounds_cm':bounds,'base_sha256':hashlib.sha256(original.tobytes()).hexdigest(),'output_sha256':hashlib.sha256(result.tobytes()).hexdigest(),'policy':'Landscape is cut 32 cm below the sidewalk profile (20 cm below roadway). Corridor centre shifted -140 cm laterally, full half-width 580 cm, eased shoulders to 780 cm. Authored arcade excavation, not a surveyed subterranean DEM.','installed':False}
(base/'krog-terrain-cut.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report,indent=2))
