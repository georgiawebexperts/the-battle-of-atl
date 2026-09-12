"""Build Krog/DeKalb road surfaces with joins to the installed BeltLine meshes."""
import array, json, math, sys, argparse
from functools import lru_cache
from pathlib import Path
from shapely.geometry import Point, Polygon, LineString, box
from shapely.geometry.polygon import orient
from shapely.ops import unary_union, nearest_points
from shapely import STRtree, constrained_delaunay_triangles, set_precision

root = Path(__file__).resolve().parents[1]
folder = root / 'SourceAssets/Terrain/KrogTraffic'
data = json.loads((folder / 'network.json').read_text())
meta = json.loads((root / 'SourceAssets/Terrain/terrain-georeference.json').read_text())
assert meta['active_heightmap'] == data['active_heightmap']
parser=argparse.ArgumentParser();parser.add_argument('--heightmap',type=Path);parser.add_argument('--junction-table',action='store_true');args=parser.parse_args()
heightmap=(args.heightmap or root/'SourceAssets/Terrain'/meta['active_heightmap']).resolve()
raw = array.array('H')
raw.frombytes(heightmap.read_bytes())
if sys.byteorder != 'little': raw.byteswap()

def terrain(x, y):
    sx, sy, sz = meta['unreal_scale']; lx, ly, _ = meta['unreal_location_cm']
    gx, gy = (x-lx)/sx, (-y-ly)/sy
    ix, iy = math.floor(gx), math.floor(gy); dx, dy = gx-ix, gy-iy
    w, h = meta['size']; assert 0 <= ix < w-1 and 0 <= iy < h-1
    a,b,c,d = [(raw[v*w+u]-32768)*sz/128 for u,v in [(ix,iy),(ix+1,iy),(ix,iy+1),(ix+1,iy+1)]]
    return a+(b-a)*dx+(d-b)*dy if dx >= dy else a+(d-c)*dx+(c-a)*dy

# Round caps fill source-way junction wedges; terminal extents are clipped below.
site = Point(data['crossing_xyz'][:2]); limit = site.buffer(4500)
road = unary_union([LineString(r['points_cm']).buffer(450 if r['tags']['name']=='DeKalb Avenue Northeast' else 300, join_style=2)
                    for r in data['roads']]).intersection(limit)
polys, triangles, surface_names = [], [], []
for directory in ['EastsideTrail', 'KrogRoute']:
    for path in (root/'SourceAssets/Terrain'/directory).glob('*.obj'):
        if not any(s in path.stem for s in ['Asphalt','Concrete']) and path.stem!='KrogTunnel_Road': continue
        vertices = []
        for line in path.read_text().splitlines():
            fields = line.split()
            # These older source meshes receive source-to-world placement after
            # Unreal OBJ import; raw OBJ coordinates already match world ESU.
            if fields and fields[0] == 'v': vertices.append(tuple(map(float, fields[1:4])))
            elif fields and fields[0] == 'f':
                face = [vertices[int(s.split('/')[0])-1] for s in fields[1:]]
                poly = Polygon([p[:2] for p in face])
                if path.stem=='KrogTunnel_Road':
                    a,b,c=face
                    if (b[0]-a[0])*(c[1]-a[1])-(b[1]-a[1])*(c[0]-a[0])<=0:continue
                if poly.area > 1e-5 and (poly.distance(road) < 350 or (args.junction_table and poly.distance(site)<1500)):
                    assert len(face) == 3
                    polys.append(poly); triangles.append(face); surface_names.append(path.stem)
assert polys
index = STRtree(polys); protected = unary_union(polys)
# Coalesce sub-millimetre gaps between independently exported pavement chunks.
# Otherwise their 12cm curb-height difference creates near-vertical sliver faces.
protected = protected.buffer(.1,join_style=2).buffer(-.1,join_style=2)
geometry = road.difference(protected)
if args.junction_table:geometry=geometry.union(protected.intersection(site.buffer(1500)))

def plane(i,x,y):
    a,b,c = triangles[i]
    den = (b[1]-c[1])*(a[0]-c[0])+(c[0]-b[0])*(a[1]-c[1])
    u = ((b[1]-c[1])*(x-c[0])+(c[0]-b[0])*(y-c[1]))/den
    v = ((c[1]-a[1])*(x-c[0])+(a[0]-c[0])*(y-c[1]))/den
    return u*a[2]+v*b[2]+(1-u-v)*c[2]

@lru_cache(maxsize=500000)
def approach_height(x,y):
    p = Point(x,y); i = int(index.nearest(p)); q = nearest_points(p,polys[i])[1]
    distance = p.distance(q); blend = max(0, 1-distance/300)
    if blend==0:return terrain(x,y)+14
    if args.junction_table and distance<.02:
        # Exported shared edges differ by micrometres; include touching top faces.
        covering=[int(j) for j in index.query(p.buffer(.02)) if polys[int(j)].distance(p)<.02]
        if covering:return max(terrain(x,y)+3,max(plane(j,x,y) for j in covering))
    # Blend the nearest point of each distinct surface, not triangle density.
    # A hard nearest-triangle choice jumps by the curb height along its bisector.
    nearest={}
    for candidate in index.query(p.buffer(distance+80)):
        j=int(candidate);d=polys[j].distance(p);name=surface_names[j]
        if d<distance+80 and (name not in nearest or d<nearest[name][0]):nearest[name]=(d,j)
    total=correction=0.
    for d,j in nearest.values():
        point=nearest_points(p,polys[j])[1]
        weight=(max(0,1-(d-distance)/80)**2)/max(d,.00001)**2
        correction+=weight*(plane(j,point.x,point.y)-terrain(point.x,point.y)-14);total+=weight
    return max(terrain(x,y)+3,terrain(x,y)+14+blend*correction/total)

@lru_cache(maxsize=500000)
def height(x,y):
    z=approach_height(x,y)
    if not args.junction_table:return z
    distance=math.hypot(x-site.x,y-site.y)
    if distance>=1500:return z
    # Raised crossing joins the trail and roadway; ease over a 7m approach.
    weight=max(0,min(1,(1500-distance)/700));weight=weight*weight*(3-2*weight)
    return max(terrain(x,y)+3,z+(1000-z)*weight)

# Confirm source frame against actual installed hits before generating pavement.
survey = json.loads((root/'Tests/Results/2026-09-12-krog-crossing-survey.json').read_text())
errors = []
for sample in survey['samples']:
    if not sample['actor'] or not sample['actor'].startswith(('Eastside trail','Krog route')) or 'Shell' in sample['actor']: continue
    if Point(sample['xy']).distance(road)>300:continue
    x,y = sample['xy']; p = Point(x,y)
    candidates = index.query(p, predicate='intersects')
    assert len(candidates), sample
    errors.append(abs(max(plane(int(i),x,y) for i in candidates)-sample['height']))
assert errors and max(errors)<.25, max(errors)
surface_regions=[set_precision(unary_union([poly for poly,name in zip(polys,surface_names) if name==key]),.01) for key in sorted(set(surface_names))]
geometry=set_precision(geometry,.01)
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
        pieces=[patch]
        # Honor existing curb outlines instead of triangulating across a height step.
        if args.junction_table:
            for region in surface_regions:
                partition=[]
                for piece in pieces:
                    if not piece.intersects(region):partition.append(piece);continue
                    partition.extend(part for part in [piece.intersection(region),piece.difference(region)] if part.area>1e-5)
                pieces=partition
        for piece in pieces:
            for tri in constrained_delaunay_triangles(piece).geoms:
                if tri.area>1e-5:emit(list(orient(tri,sign=1).exterior.coords)[:3])
# Adaptive subdivision must share boundary vertices with adjacent triangles.
# Otherwise a newly sampled edge midpoint has a different height from its
# unsplit neighbour, leaving a vertical crack despite complete XY coverage.
xy_vertices=sorted({(round(p[0],5),round(p[1],5)) for f in faces for p in f})
point_tree=STRtree([Point(p) for p in xy_vertices])
conforming=[];split_edges=0;max_previous_gap=0
for face in faces:
    corners=[(round(p[0],5),round(p[1],5)) for p in face];ring=[]
    for a,b in zip(corners,corners[1:]+corners[:1]):
        line=LineString([a,b]);length=line.length
        if length<1e-7:continue
        candidates=[]
        for i in point_tree.query(line.buffer(.00003)):
            p=xy_vertices[int(i)];d=line.project(Point(p))
            if d>1e-6 and d<length-1e-6 and line.distance(Point(p))<.00003:
                candidates.append((d,p))
                max_previous_gap=max(max_previous_gap,abs(height(*p)-((1-d/length)*height(*a)+d/length*height(*b))))
        if candidates:split_edges+=1
        ring.append(a);ring.extend(p for _,p in sorted(candidates))
    polygon=Polygon(ring)
    if polygon.area<1e-8:continue
    assert polygon.is_valid
    for tri in constrained_delaunay_triangles(polygon).geoms:
        if tri.area>1e-8:
            conforming.append([(x,y,height(x,y)) for x,y in list(orient(tri,sign=1).exterior.coords)[:3]])
degenerate_faces=[f for f in conforming if Polygon([p[:2] for p in f]).area<.001]
faces=[f for f in conforming if Polygon([p[:2] for p in f]).area>=.001]
degenerate_area=sum(Polygon([p[:2] for p in f]).area for f in degenerate_faces)
assert degenerate_area<1,degenerate_area
degenerate_region=unary_union([Polygon([p[:2] for p in f]) for f in degenerate_faces])
from collections import Counter
edges=Counter()
for face in faces:
    pts=[(round(p[0],5),round(p[1],5)) for p in face]
    for a,b in zip(pts,pts[1:]+pts[:1]):edges[tuple(sorted((a,b)))]+=1
interior_open_edges=[];numerical_open_edges=[]
for (a,b),count in edges.items():
    assert count<=2,(a,b,count)
    if count==1 and geometry.boundary.distance(LineString([a,b]).interpolate(.5,normalized=True))>.02:
        if degenerate_region.buffer(.00003).covers(LineString([a,b])):numerical_open_edges.append([a,b])
        else:interior_open_edges.append([a,b])
assert not interior_open_edges,interior_open_edges[:5]
coverage=unary_union([Polygon([p[:2] for p in f]) for f in faces])
missing=geometry.difference(coverage.buffer(.001)).area
assert missing<1,missing
vertices=[p for f in faces for p in f]
lines=['o Krog_Road']+[f'v {x:.5f} {-y:.5f} {z:.5f}' for x,y,z in vertices]
lines += [f'vt {x/200:.5f} {y/200:.5f}' for x,y,z in vertices]
for x,y,z in vertices:
    nx=-(height(x+1,y)-height(x-1,y))/2;ny=(height(x,y+1)-height(x,y-1))/2
    length=math.sqrt(nx*nx+ny*ny+1);lines.append(f'vn {nx/length:.8f} {ny/length:.8f} {1/length:.8f}')
for i in range(0,len(vertices),3):lines.append('f '+' '.join(f'{j}/{j}/{j}' for j in [i+3,i+2,i+1]))
(folder/'Krog_Road.obj').write_text('\n'.join(lines)+'\n')
# Test actual native support later at triangle centroids, not only source bounds.
probes=[{'xyz':[sum(p[k] for p in f)/3 for k in range(3)]} for f in faces]
(folder/'road-probes.json').write_text(json.dumps({'samples':probes},indent=2)+'\n')
seam_gaps=[];seam_locations=[]
for x,y,z in vertices:
    point=Point(x,y)
    if protected.boundary.distance(point)<.001:
        i=int(index.nearest(point));q=nearest_points(point,polys[i])[1]
        gap=abs(z-plane(i,q.x,q.y));seam_gaps.append(gap)
        if gap>.25:seam_locations.append({"xyz":[x,y,z],"existing_z":plane(i,q.x,q.y),"terrain_z":terrain(x,y),"gap_cm":gap})
(folder/'join-conflicts.json').write_text(json.dumps(sorted(seam_locations,key=lambda r:-r['gap_cm']),indent=2)+'\n')
report={'removed_degenerate_faces':len(degenerate_faces),'removed_area_cm2':degenerate_area,'degenerate_boundary_edges':len(numerical_open_edges),'junction_table':args.junction_table,'junction_table_height_cm':1000 if args.junction_table else None,'heightmap':str(heightmap.relative_to(root)),'maximum_join_step_cm':max(seam_gaps,default=0),'join_samples':len(seam_gaps),'conformed_edges':split_edges,'max_previous_edge_height_gap_cm':max_previous_gap,'interior_open_edges':len(interior_open_edges),'triangles':len(faces),'missing_area_cm2':missing,'protected_overlap_cm2':coverage.intersection(protected).area,
        'source_to_native_max_error_cm':max(errors),'native_reference_samples':len(errors),
        'main_map_changed':False,'scope':'Source candidate only; native collision, driving and appearance pending.'}
(folder/'road-surfaces.json').write_text(json.dumps(report,indent=2)+'\n');print(report)
