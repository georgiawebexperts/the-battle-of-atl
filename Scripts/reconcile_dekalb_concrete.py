"""Remove overlapping legacy concrete and feather its exposed road joins."""
import json, math
from pathlib import Path
from shapely.geometry import Point, Polygon
from shapely.ops import unary_union, nearest_points
from shapely import STRtree, constrained_delaunay_triangles
root=Path(__file__).resolve().parents[1];folder=root/'SourceAssets/Terrain/KrogTraffic'
def read(path,sign):
    vertices=[];result=[]
    for row in path.read_text().splitlines():
        f=row.split()
        if f and f[0]=='v':vertices.append((float(f[1]),sign*float(f[2]),float(f[3])))
        elif f and f[0]=='f':
            tri=[vertices[int(v.split('/')[0])-1] for v in f[1:]]
            if Polygon([v[:2] for v in tri]).area>1e-6:result.append(tri)
    return result
def plane(tri,x,y):
    a,b,c=tri;det=(b[0]-a[0])*(c[1]-a[1])-(b[1]-a[1])*(c[0]-a[0])
    u=((x-a[0])*(c[1]-a[1])-(y-a[1])*(c[0]-a[0]))/det
    v=((b[0]-a[0])*(y-a[1])-(b[1]-a[1])*(x-a[0]))/det
    return a[2]+u*(b[2]-a[2])+v*(c[2]-a[2])
roads=read(folder/'Krog_Road.obj',-1);polys=[Polygon([v[:2] for v in t]) for t in roads]
tree=STRtree(polys);footprint=unary_union(polys).buffer(.002)
legacy=read(root/'SourceAssets/Terrain/KrogRoute/KrogRoute_Concrete_7_2.obj',1)
faces=[];removed=0.;seams=[]
for tri in legacy:
    original=Polygon([v[:2] for v in tri]);remaining=original.difference(footprint)
    removed+=original.area-remaining.area
    if remaining.area<.001:continue
    def elevation(x,y):
        old=plane(tri,x,y);p=Point(x,y);d=footprint.distance(p)
        if d>=150:return old
        q=nearest_points(p,footprint)[1]
        j=int(tree.nearest(q));target=plane(roads[j],q.x,q.y)
        correction=target-plane(tri,q.x,q.y)
        weight=max(0,1-d/150);weight=weight*weight*(3-2*weight)
        return old+correction*weight
    def emit(coords,depth=0):
        vs=[(x,y,elevation(x,y)) for x,y in coords]
        mid=[sum(p[k] for p in vs)/3 for k in range(3)]
        if depth<5 and abs(mid[2]-elevation(*mid[:2]))>.25:
            a,b,c=coords;ab,bc,ca=[tuple((p[k]+q[k])/2 for k in range(2)) for p,q in [(a,b),(b,c),(c,a)]]
            for t in [(a,ab,ca),(ab,b,bc),(ca,bc,c),(ab,bc,ca)]:emit(t,depth+1)
        else:faces.append(vs)
    for piece in constrained_delaunay_triangles(remaining).geoms:
        if piece.area>.001:emit(list(piece.exterior.coords)[:3])
for face in faces:
    for x,y,z in face:
        if footprint.distance(Point(x,y))<.01:
            j=int(tree.nearest(Point(x,y)));seams.append(abs(z-plane(roads[j],x,y)))
assert faces and seams and max(seams)<.1,max(seams,default=0)
vs=[v for face in faces for v in face]
rows=['o KrogRoute_Concrete_7_2_Reconciled']
# Legacy placement preserves the original raw ESU coordinates and actor transform.
rows.extend(f'v {x:.6f} {y:.6f} {z:.6f}' for x,y,z in vs)
rows.extend(f'vt {x/200:.6f} {y/200:.6f}' for x,y,z in vs)
for i in range(0,len(vs),3):
    a,b,c=vs[i:i+3];up=(b[0]-a[0])*(c[1]-a[1])-(b[1]-a[1])*(c[0]-a[0])
    indices=[i+1,i+2,i+3] if up>0 else [i+3,i+2,i+1]
    rows.append('f '+' '.join(f'{j}/{j}' for j in indices))
(folder/'KrogRoute_Concrete_7_2_Reconciled.obj').write_text('\n'.join(rows)+'\n')
report={'source_triangles':len(legacy),'triangles':len(faces),'removed_overlap_area_cm2':removed,
        'seam_samples':len(seams),'maximum_source_seam_gap_cm':max(seams),'main_map_changed':False,
        'scope':'Source clipping and seam vertex heights. Native support, tessellation continuity and bike traversal pending.'}
(folder/'reconciled-concrete.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report))
