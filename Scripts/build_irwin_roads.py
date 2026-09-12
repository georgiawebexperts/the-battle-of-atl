"""Build Irwin/Lake road surfaces with joins to the installed BeltLine meshes."""
import array, json, math, sys
from pathlib import Path
from shapely.geometry import Point, Polygon, LineString, box
from shapely.geometry.polygon import orient
from shapely.ops import unary_union, nearest_points
from shapely import STRtree, constrained_delaunay_triangles

root = Path(__file__).resolve().parents[1]
folder = root / 'SourceAssets/Terrain/IrwinTraffic'
data = json.loads((folder / 'network.json').read_text())
meta = json.loads((root / 'SourceAssets/Terrain/terrain-georeference.json').read_text())
assert meta['active_heightmap'] == data['active_heightmap']
raw = array.array('H')
raw.frombytes((root / 'SourceAssets/Terrain' / meta['active_heightmap']).read_bytes())
if sys.byteorder != 'little': raw.byteswap()

def terrain(x, y):
    sx, sy, sz = meta['unreal_scale']; lx, ly, _ = meta['unreal_location_cm']
    gx, gy = (x-lx)/sx, (-y-ly)/sy
    ix, iy = math.floor(gx), math.floor(gy); dx, dy = gx-ix, gy-iy
    w, h = meta['size']; assert 0 <= ix < w-1 and 0 <= iy < h-1
    a,b,c,d = [(raw[v*w+u]-32768)*sz/128 for u,v in [(ix,iy),(ix+1,iy),(ix,iy+1),(ix+1,iy+1)]]
    return a+(b-a)*dx+(d-b)*dy if dx >= dy else a+(d-c)*dx+(c-a)*dy

# Round caps fill source-way junction wedges; terminal extents are clipped below.
site = Point(data['crossing_xyz'][:2]); limit = site.buffer(6500)
road = unary_union([LineString(r['points_cm']).buffer(300, join_style=2)
                    for r in data['roads']]).intersection(limit)
polys, triangles = [], []
for directory in ['EastsideTrail', 'KrogRoute']:
    for path in (root/'SourceAssets/Terrain'/directory).glob('*.obj'):
        if not any(s in path.stem for s in ['Asphalt','Concrete']): continue
        vertices = []
        for line in path.read_text().splitlines():
            fields = line.split()
            # These older source meshes receive source-to-world placement after
            # Unreal OBJ import; raw OBJ coordinates already match world ESU.
            if fields and fields[0] == 'v': vertices.append(tuple(map(float, fields[1:4])))
            elif fields and fields[0] == 'f':
                face = [vertices[int(s.split('/')[0])-1] for s in fields[1:]]
                poly = Polygon([p[:2] for p in face])
                if poly.area > 1e-5 and poly.distance(road) < 350:
                    assert len(face) == 3
                    polys.append(poly); triangles.append(face)
assert polys
index = STRtree(polys); protected = unary_union(polys)
geometry = road.difference(protected)

def plane(i,x,y):
    a,b,c = triangles[i]
    den = (b[1]-c[1])*(a[0]-c[0])+(c[0]-b[0])*(a[1]-c[1])
    u = ((b[1]-c[1])*(x-c[0])+(c[0]-b[0])*(y-c[1]))/den
    v = ((c[1]-a[1])*(x-c[0])+(a[0]-c[0])*(y-c[1]))/den
    return u*a[2]+v*b[2]+(1-u-v)*c[2]

def height(x,y):
    p = Point(x,y); i = int(index.nearest(p)); q = nearest_points(p,polys[i])[1]
    distance = p.distance(q); blend = max(0, 1-distance/300)
    # Match existing triangle exactly at the seam; fade elevation correction
    # over three game metres without changing the active landscape.
    return terrain(x,y)+14 + blend*(plane(i,q.x,q.y)-terrain(q.x,q.y)-14)

# Confirm source frame against actual installed hits before generating pavement.
survey = json.loads((root/'Tests/Results/2026-09-12-irwin-crossing-survey.json').read_text())
errors = []
for sample in survey['samples']:
    if not sample['actor'] or not sample['actor'].startswith(('Eastside trail','Krog route')): continue
    x,y = sample['xy']; p = Point(x,y)
    candidates = index.query(p, predicate='intersects')
    assert len(candidates), sample
    errors.append(abs(max(plane(int(i),x,y) for i in candidates)-sample['height']))
assert errors and max(errors)<.25, max(errors)
faces=[]
def emit(coords, depth=0):
    points = [(x,y,height(x,y)) for x,y in coords]
    samples = [(.5,.5,0),(.5,0,.5),(0,.5,.5),(1/3,1/3,1/3)]
    deviation=0; clearance=999
    for weights in samples:
        x,y,z=[sum(w*p[k] for w,p in zip(weights,points)) for k in range(3)]
        deviation=max(deviation,abs(z-height(x,y)));clearance=min(clearance,z-terrain(x,y))
    if (deviation>1 or clearance<.5) and depth<5:
        a,b,c=coords
        ab,bc,ca=[tuple((p[k]+q[k])/2 for k in range(2)) for p,q in [(a,b),(b,c),(c,a)]]
        for tri in [(a,ab,ca),(ab,b,bc),(ca,bc,c),(ab,bc,ca)]:emit(tri,depth+1)
    else:
        assert clearance>0, (clearance,coords)
        faces.append(points)
x0,y0,x1,y1=geometry.bounds
for x in range(math.floor(x0/100)*100,math.ceil(x1/100)*100,100):
    for y in range(math.floor(y0/100)*100,math.ceil(y1/100)*100,100):
        patch=geometry.intersection(box(x,y,x+100,y+100))
        if patch.area<1e-5:continue
        for tri in constrained_delaunay_triangles(patch).geoms:
            if tri.area>1e-5:emit(list(orient(tri,sign=1).exterior.coords)[:3])
coverage=unary_union([Polygon([p[:2] for p in f]) for f in faces])
missing=geometry.difference(coverage.buffer(.001)).area
assert missing<1,missing
vertices=[p for f in faces for p in f]
lines=['o Irwin_Road']+[f'v {x:.5f} {-y:.5f} {z:.5f}' for x,y,z in vertices]
lines += [f'vt {x/200:.5f} {y/200:.5f}' for x,y,z in vertices]
for i in range(0,len(vertices),3):lines.append('f '+' '.join(f'{j}/{j}' for j in [i+3,i+2,i+1]))
(folder/'Irwin_Road.obj').write_text('\n'.join(lines)+'\n')
# Test actual native support later at triangle centroids, not only source bounds.
probes=[{'xyz':[sum(p[k] for p in f)/3 for k in range(3)]} for f in faces]
(folder/'road-probes.json').write_text(json.dumps({'samples':probes},indent=2)+'\n')
report={'triangles':len(faces),'missing_area_cm2':missing,'protected_overlap_cm2':coverage.intersection(protected).area,
        'source_to_native_max_error_cm':max(errors),'native_reference_samples':len(errors),
        'main_map_changed':False,'scope':'Source candidate only; native collision, driving and appearance pending.'}
(folder/'road-surfaces.json').write_text(json.dumps(report,indent=2)+'\n');print(report)
