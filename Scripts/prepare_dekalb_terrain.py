"""Create separate terrain cuts for the crowned road; report legacy path conflicts."""
import array, bisect, hashlib, json, math, sys
from pathlib import Path
from shapely.geometry import LineString, Point

root=Path(__file__).resolve().parents[1];base=root/'SourceAssets/Terrain';folder=base/'KrogTraffic'
profile=json.loads((folder/'dekalb-cross-section.json').read_text())
stations=profile['stations'];centres=[s['centre_cm'] for s in stations]
line=LineString([p[:2] for p in centres]);lengths=[0.]
for a,b in zip(centres,centres[1:]):lengths.append(lengths[-1]+math.hypot(b[0]-a[0],b[1]-a[1]))
def section(x,y):
    p=Point(x,y);d=line.project(p);i=min(len(lengths)-2,max(0,bisect.bisect_right(lengths,d)-1))
    t=(d-lengths[i])/(lengths[i+1]-lengths[i]);z=centres[i][2]*(1-t)+centres[i+1][2]*t
    offset=line.distance(p)
    return z-.02*min(offset,450),offset,d

meta=json.loads((base/'terrain-georeference.json').read_text())
source=folder/'atlanta-height-krog-join-candidate.r16'
raw=array.array('H');raw.frombytes(source.read_bytes())
if sys.byteorder!='little':raw.byteswap()
out=array.array('H',raw);sx,sy,sz=meta['unreal_scale'];lx,ly,_=meta['unreal_location_cm'];w,h=meta['size']
x0,y0,x1,y1=line.bounds;changed=[]
for iy in range(max(0,math.floor((-y1-850-ly)/sy)),min(h,math.ceil((-y0+850-ly)/sy)+1)):
    for ix in range(max(0,math.floor((x0-850-lx)/sx)),min(w,math.ceil((x1+850-lx)/sx)+1)):
        x,y=lx+ix*sx,-(ly+iy*sy);z,offset,d=section(x,y)
        if offset>=850:continue
        # Full support clearance through the pavement edge, easing into grass.
        # End cuts taper beyond the authored road ends to avoid a terrain wall.
        weight=min(1,max(0,(850-offset)/350));weight=weight*weight*(3-2*weight)
        index=iy*w+ix;old=(raw[index]-32768)*sz/128
        target=min(old,z-12)
        value=round(32768+(old+(target-old)*weight)*128/sz)
        if value<raw[index]:
            out[index]=value
            changed.append({'grid':[ix,iy],'xy':[x,y],'lowering_cm':(raw[index]-value)*sz/128})
assert changed
destination=folder/'atlanta-height-dekalb-crowned-candidate.r16'
encoded=array.array('H',out)
if sys.byteorder!='little':encoded.byteswap()
destination.write_bytes(encoded.tobytes())

# Never silently leave old path collision protruding through a lowered road.
conflicts=[]
for directory in ['EastsideTrail','KrogRoute']:
    for path in (base/directory).glob('*.obj'):
        if not any(t in path.stem for t in ['Asphalt','Concrete']) and path.stem!='KrogTunnel_Road':continue
        seen=set();bad=[]
        for row in path.read_text().splitlines():
            parts=row.split()
            if not parts or parts[0]!='v':continue
            x,y,z=map(float,parts[1:4])
            if (x,y,z) in seen:continue
            seen.add((x,y,z))
            if x<x0-450 or x>x1+450 or y<y0-450 or y>y1+450:continue
            proposed,offset,d=section(x,y)
            if offset<450 and z>proposed+.25:bad.append({'xyz':[x,y,z],'proposed_z':proposed,'protrusion_cm':z-proposed})
        if bad:conflicts.append({'source':str(path.relative_to(root)),'vertices':len(bad),'worst':sorted(bad,key=lambda p:-p['protrusion_cm'])[:5]})
report={'author':'2026-09-12 [codex-maclaptop]','source':str(source.relative_to(root)),
        'source_sha256':hashlib.sha256(source.read_bytes()).hexdigest(),
        'output':str(destination.relative_to(root)),'changed_samples':len(changed),
        'maximum_lowering_cm':max(p['lowering_cm'] for p in changed),
        'outside_850cm_identical':all(a==b or line.distance(Point(lx+(i%w)*sx,-(ly+(i//w)*sy)))<850 for i,(a,b) in enumerate(zip(raw,out))),
        'legacy_path_vertex_conflicts':conflicts,
        'scope':'Separate terrain candidate only. Legacy overlap check samples vertices, not triangle interiors; native support and joins remain unverified.',
        'main_map_changed':False}
assert report['outside_850cm_identical']
(folder/'dekalb-terrain.json').write_text(json.dumps(report,indent=2)+'\n')
print(json.dumps(report,indent=2))
