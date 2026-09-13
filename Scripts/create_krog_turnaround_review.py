"""Place continuous DeKalb traffic in an isolated copy of the scooter review map."""
import unreal,json,math
from pathlib import Path
root=Path(unreal.Paths.project_dir()).resolve()
assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/PiedmontScooterReview')
unreal.PiedmontWorldTools.finish_editor_asset_loading();ea=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
data=json.loads((root/'SourceAssets/Terrain/KrogTraffic/turnaround-routes.json').read_text());assert data['passed'] and data['closed']
failures=[]
for p in data['footprint_samples']:
    x,y,z=p['xyz'];hit=unreal.PiedmontWorldTools.trace_world_surface(unreal.Vector(x,y,z+180),unreal.Vector(x,y,z-180))
    if not hit or abs(hit[0].z-z)>.25:failures.append({'xyz':p['xyz'],'hit_z':hit[0].z if hit else None})
assert not failures,failures[:5]
directors=[a for a in ea.get_all_level_actors() if isinstance(a,unreal.BattleRoadTrafficDirector) and a.actor_has_tag('KrogTrafficReview')]
assert len(directors)==1,len(directors)
d=directors[0];gate=d.get_editor_property('Lanes')[0].get_editor_property('Crossings')[0].get_editor_property('Crossing');assert gate
center=gate.get_actor_location();extent=gate.get_editor_property('CrossingArea').get_unscaled_box_extent()
points=data['points_cm'];distance=0;was_inside=False;stops=[]
for i,p in enumerate(points[:-1]):
    if i:distance+=math.dist(p[:2],points[i-1][:2])
    q=points[i+1];dx,dy=q[0]-p[0],q[1]-p[1];n=math.hypot(dx,dy);dx/=n;dy/=n
    inside=abs(p[0]-center.x)<=extent.x+abs(dx)*236+abs(dy)*114 and abs(p[1]-center.y)<=extent.y+abs(dy)*236+abs(dx)*114
    if inside and not was_inside:stops.append(distance-100)
    was_inside=inside
assert len(stops)==2 and min(stops)>0,stops
lane=unreal.BattleRoadTrafficLane();lane.set_editor_property('Name','DeKalb construction turnarounds');lane.set_editor_property('Points',[unreal.Vector(*p) for p in points]);lane.set_editor_property('CruiseSpeed',data['speed_cm_s']);lane.set_editor_property('bLoopRoute',True)
bindings=[]
for stop in stops:
    b=unreal.BattleCarCrossing();b.set_editor_property('Crossing',gate);b.set_editor_property('StopDistance',stop);bindings.append(b)
lane.set_editor_property('Crossings',bindings);d.set_editor_property('Lanes',[lane]);d.set_editor_property('MaxCars',6);d.set_editor_property('MaxCarsPerLane',6)
d.set_actor_label('DeKalb traffic with construction turnarounds')
assert unreal.EditorLoadingAndSavingUtils.save_map(unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world(),'/Game/PiedmontRide/Maps/PiedmontKrogTurnaroundReview')
(root/'Tests/Results/2026-09-13-krog-turnaround-review.json').write_text(json.dumps({'passed':True,'native_support_samples':len(data['footprint_samples']),'crossing_stops_cm':stops,'loop_points':len(points),'main_map_changed':False,'scope':'Native surface agreement for turn body footprints and two crossing bindings. Runtime laps, collision clearance and visuals remain unverified.'},indent=2)+'\n')
