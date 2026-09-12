"""Measure native wheel-height discontinuities around stopped review cars."""
import json, math
from pathlib import Path
import unreal

root = Path(unreal.Paths.project_dir())
assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/PiedmontKrogLaneReview')
unreal.PiedmontWorldTools.finish_editor_asset_loading()
ea = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
obstacles = []
for actor in ea.get_all_level_actors():
    if actor.get_name() == 'StaticMeshActor_294':
        obstacles.append({'name': actor.get_name(), 'label': actor.get_actor_label(),
                          'location': str(actor.get_actor_location())})

def trace(x, y, z):
    hit = unreal.PiedmontWorldTools.trace_world_surface(unreal.Vector(x,y,z+250), unreal.Vector(x,y,z-250))
    return {'z': hit[0].z, 'actor': hit[1].get_actor_label()} if hit else None

routes = json.loads((root/'SourceAssets/Terrain/KrogTraffic/car-lanes.json').read_text())['routes']
results = []
for route, stopped in zip(routes, [2557.589, 2492.315]):
    points = route['points_cm']
    lengths = [0]
    for a,b in zip(points, points[1:]):
        lengths.append(lengths[-1]+math.hypot(b[0]-a[0],b[1]-a[1]))
    def sample(d):
        for i in range(1,len(points)):
            if d <= lengths[i]:
                t=(d-lengths[i-1])/(lengths[i]-lengths[i-1])
                return [a+(b-a)*t for a,b in zip(points[i-1],points[i])]
        return points[-1]
    for step in range(-20,41):
        d=stopped+step
        p=sample(d); ahead=sample(d+150)
        dx,dy=ahead[0]-p[0],ahead[1]-p[1]
        n=math.hypot(dx,dy);dx/=n;dy/=n
        for wheel in range(4):
            along=123 if wheel<2 else -141.2
            across=90 if wheel%2 else -90
            x,y=p[0]+dx*along-dy*across,p[1]+dy*along+dx*across
            hits=[trace(x+ox,y+oy,p[2]) for ox,oy in [(0,0),(-1,0),(1,0),(0,-1),(0,1)]]
            nz=None
            if all(hits):
                gx=(hits[2]['z']-hits[1]['z'])/2
                gy=(hits[4]['z']-hits[3]['z'])/2
                nz=1/math.sqrt(1+gx*gx+gy*gy)
            results.append({'lane':route['name'],'distance_cm':d,'wheel':wheel,'xy':[x,y], 'normal_z_estimate':nz,'hits':hits})
report={'obstacles':obstacles,'scope':'Native heights and finite-difference slopes, not exact triangle normals.',
        'worst':sorted(results,key=lambda r:r['normal_z_estimate'] or 0)[:30],
        'samples':len(results)}
(root/'Tests/Results/2026-09-12-krog-car-diagnosis.json').write_text(json.dumps(report,indent=2)+'\n')
