"""Author a crowned DeKalb road profile and quantify required surface changes.

This is a source proposal, not an installed replacement. Its earthwork and joins
must be reconciled with terrain and existing paths before native validation.
"""
import json, math
from pathlib import Path
from shapely.geometry import LineString, Point, Polygon
from shapely.ops import linemerge
from shapely import STRtree

root=Path(__file__).resolve().parents[1]
folder=root/'SourceAssets/Terrain/KrogTraffic'
network=json.loads((folder/'network.json').read_text())
line=linemerge([LineString([p[:2] for p in road['points_cm']])
               for road in network['roads'] if road['tags']['name']=='DeKalb Avenue Northeast'])
assert line.geom_type=='LineString'
if line.coords[0][0]>line.coords[-1][0]:line=LineString(list(line.coords)[::-1])
vertices=[];triangles=[];polygons=[]
for row in (folder/'Krog_Road.obj').read_text().splitlines():
    fields=row.split()
    if fields and fields[0]=='v':vertices.append((float(fields[1]),-float(fields[2]),float(fields[3])))
    elif fields and fields[0]=='f':
        tri=[vertices[int(v.split('/')[0])-1] for v in fields[1:]]
        polygon=Polygon([v[:2] for v in tri])
        if polygon.area>1e-6:triangles.append(tri);polygons.append(polygon)
tree=STRtree(polygons)
def old_height(x,y):
    result=[]
    for index in tree.query(Point(x,y),predicate='intersects'):
        a,b,c=triangles[int(index)]
        det=(b[0]-a[0])*(c[1]-a[1])-(b[1]-a[1])*(c[0]-a[0])
        u=((x-a[0])*(c[1]-a[1])-(y-a[1])*(c[0]-a[0]))/det
        v=((b[0]-a[0])*(y-a[1])-(b[1]-a[1])*(x-a[0]))/det
        result.append(a[2]+u*(b[2]-a[2])+v*(c[2]-a[2]))
    return max(result) if result else None

# Preserve reviewed longitudinal elevations; remove terrain-driven side slopes.
# A 2% crown drops 9 cm from centre to either 450 cm road edge.
stations=[];changes=[];missing=[]
for d in [*range(100,math.floor(line.length)-100,50)]:
    p=line.interpolate(d);a=line.interpolate(d-25);b=line.interpolate(d+25)
    dx,dy=b.x-a.x,b.y-a.y;n=math.hypot(dx,dy);dx/=n;dy/=n
    z=old_height(p.x,p.y)
    assert z is not None, d
    cross=[]
    for offset in range(-450,451,25):
        x,y=p.x-dy*offset,p.y+dx*offset
        desired=z-.02*abs(offset)
        previous=old_height(x,y)
        cross.append([x,y,desired])
        if previous is None:missing.append({'station_cm':d,'offset_cm':offset,'xyz':[x,y,desired]})
        else:changes.append({'station_cm':d,'offset_cm':offset,'xy':[x,y],
                             'old_z_cm':previous,'proposed_z_cm':desired,'delta_cm':desired-previous})
    stations.append({'distance_cm':d,'centre_cm':[p.x,p.y,z],'cross_section_cm':cross})
grades=[abs(b['centre_cm'][2]-a['centre_cm'][2])/(b['distance_cm']-a['distance_cm'])
        for a,b in zip(stations,stations[1:])]
assert max(grades)<.12
profile={'author':'2026-09-12 [codex-maclaptop]','width_cm':900,'crossfall_percent':2,
         'stations':stations,'maximum_longitudinal_grade_percent':100*max(grades),
         'scope':'Proposed full-width road section. Terrain cuts, shoulders, intersecting paths and native car clearance pending; not installed.'}
(folder/'dekalb-cross-section.json').write_text(json.dumps(profile,indent=2)+'\n')
# Separate surface asset lets us inspect the real triangles before replacing
# the current road or sculpting terrain. Two strips meet at the road crown.
mesh_vertices=[];faces=[];minimum_normal_z=1.
for station in stations:
    mesh_vertices.extend(station['cross_section_cm'][i] for i in [0,18,36])
for i in range(len(stations)-1):
    for side in [0,1]:
        a=i*3+side;b=a+1;c=a+3;d=c+1
        for face in [(a,c,b),(b,c,d)]:
            p,q,r=[mesh_vertices[j] for j in face]
            u=[q[k]-p[k] for k in range(3)];v=[r[k]-p[k] for k in range(3)]
            normal=[u[1]*v[2]-u[2]*v[1],u[2]*v[0]-u[0]*v[2],u[0]*v[1]-u[1]*v[0]]
            nz=abs(normal[2])/math.sqrt(sum(t*t for t in normal))
            minimum_normal_z=min(minimum_normal_z,nz)
            faces.append(face if normal[2]>0 else tuple(reversed(face)))
assert minimum_normal_z>.98,minimum_normal_z
obj=['o DeKalb_Crowned_Road']
obj.extend(f'v {x:.5f} {-y:.5f} {z:.5f}' for x,y,z in mesh_vertices)
obj.extend(f'vt {x/200:.5f} {y/200:.5f}' for x,y,z in mesh_vertices)
# Negating OBJ Y reverses orientation relative to world ESU.
obj.extend('f '+' '.join(f'{j+1}/{j+1}' for j in reversed(face)) for face in faces)
(folder/'DeKalb_Crowned_Road.obj').write_text('\n'.join(obj)+'\n')
report={'stations':len(stations),'measured_surface_changes':len(changes),
        'surface_triangles':len(faces),'minimum_triangle_normal_z':minimum_normal_z,
        'maximum_lowering_cm':max(-v['delta_cm'] for v in changes),
        'maximum_raise_cm':max(v['delta_cm'] for v in changes),
        'maximum_longitudinal_grade_percent':100*max(grades),'crossfall_percent':2,
        'missing_old_surface_samples':missing,
        'largest_changes':sorted(changes,key=lambda v:-abs(v['delta_cm']))[:30],
        'main_map_changed':False,'scope':profile['scope']}
(root/'Tests/Results/2026-09-12-dekalb-cross-section.json').write_text(json.dumps(report,indent=2)+'\n')
print(json.dumps({k:v for k,v in report.items() if k not in ['largest_changes','missing_old_surface_samples']},indent=2))
print('Missing old surface samples:',len(missing))
