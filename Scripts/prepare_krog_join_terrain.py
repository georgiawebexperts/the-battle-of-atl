"""Prepare a bounded terrain correction beside the existing tunnel floor."""
from pathlib import Path
import json,array,sys,math,hashlib
r=Path(__file__).resolve().parents[1];base=r/'SourceAssets/Terrain';m=json.loads((base/'terrain-georeference.json').read_text());raw=array.array('H');raw.frombytes((base/m['active_heightmap']).read_bytes())
if sys.byteorder!='little':raw.byteswap()
out=array.array('H',raw);sx,sy,sz=m['unreal_scale'];lx,ly,_=m['unreal_location_cm'];w,h=m['size'];cx,cy=29382,116232;changed=[]
for iy in range(max(0,math.floor((-cy-250-ly)/sy)),min(h,math.ceil((-cy+250-ly)/sy)+1)):
 for ix in range(max(0,math.floor((cx-250-lx)/sx)),min(w,math.ceil((cx+250-lx)/sx)+1)):
  x,y=lx+ix*sx,-(ly+iy*sy);distance=math.hypot(x-cx,y-cy)
  if distance>=250:continue
  weight=max(0,min(1,(250-distance)/100));weight=weight*weight*(3-2*weight);delta=round(12*weight*128/sz);index=iy*w+ix
  if delta:out[index]=raw[index]-delta;changed.append({'grid':[ix,iy],'xy':[x,y],'lowering_cm':delta*sz/128})
assert changed and max(p['lowering_cm'] for p in changed)<=12.1
file=base/'KrogTraffic/atlanta-height-krog-join-candidate.r16';encoded=array.array('H',out)
if sys.byteorder!='little':encoded.byteswap()
file.write_bytes(encoded.tobytes())
report={'author':'2026-09-12 [codex-maclaptop]','base':m['active_heightmap'],'output':str(file.relative_to(base)),'base_sha256':hashlib.sha256((base/m['active_heightmap']).read_bytes()).hexdigest(),'changed_samples':changed,'outside_radius_identical':all(a==b or math.hypot(lx+(i%w)*sx-cx,-(ly+(i//w)*sy)-cy)<250 for i,(a,b) in enumerate(zip(raw,out))),'installed':False}
(base/'KrogTraffic/join-terrain.json').write_text(json.dumps(report,indent=2)+'\n');print({'changed_samples':len(changed),'maximum_lowering_cm':max(p['lowering_cm'] for p in changed),'installed':False})
