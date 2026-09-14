"""Add an opt-in driving fixture to the isolated street review map only."""
import unreal,pathlib,json,math,hashlib
root=pathlib.Path(unreal.Paths.project_dir());main=root/'Content/PiedmontRide/Maps/PiedmontWorld.umap';before=hashlib.sha256(main.read_bytes()).hexdigest();assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/PiedmontPrideStreetReview');world=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world();ea=unreal.get_editor_subsystem(unreal.EditorActorSubsystem);unreal.PiedmontWorldTools.finish_editor_asset_loading()
d=json.loads((root/'SourceAssets/Terrain/PrideIntersection/survey.json').read_text());junction=d['route_nodes'].index(d['intersection_node']);raw=[(p[0],p[1]+(250 if i<=junction else 0)) for i,p in enumerate(d['route_world_cm'])]
# Keep a normal curved turn within the authored intersection, not an instantaneous 90-degree steering demand.
distances=[0.]
for a,b in zip(raw,raw[1:]):distances.append(distances[-1]+math.dist(a,b))
def along(distance):
 for i in range(len(raw)-1):
  if distances[i+1]>=distance:
   t=(distance-distances[i])/(distances[i+1]-distances[i]);return tuple(raw[i][k]+t*(raw[i+1][k]-raw[i][k]) for k in range(2))
 return raw[-1]
# Trim across multiple short OSM segments: their node spacing is not a bicycle turn radius.
p=along(distances[junction]-500);q=along(distances[junction]+500);b=raw[junction]
coarse=[raw[i] for i in range(len(raw)) if distances[i]<distances[junction]-500]
for j in range(21):
 t=j/20;coarse.append(tuple((1-t)**2*p[k]+2*(1-t)*t*b[k]+t*t*q[k] for k in range(2)))
coarse.extend(raw[i] for i in range(len(raw)) if distances[i]>distances[junction]+500)
points=[];probes=[]
for a,b in zip(coarse,coarse[1:]):
 for j in range(max(1,math.ceil(math.dist(a,b)/50))):
  t=j/max(1,math.ceil(math.dist(a,b)/50));x,y=[a[k]+(b[k]-a[k])*t for k in range(2)];h=unreal.PiedmontWorldTools.trace_world_surface(unreal.Vector(x,y,1600),unreal.Vector(x,y,-1200));assert h,(x,y);probes.append({'xyz':[x,y,h[0].z],'actor':h[1].get_actor_label(),'paved':h[1].actor_has_tag('RidePath')});points.append(h[0])
x,y=coarse[-1];h=unreal.PiedmontWorldTools.trace_world_surface(unreal.Vector(x,y,1600),unreal.Vector(x,y,-1200));assert h;points.append(h[0])
assert all(p['paved'] for p in probes),str([p for p in probes if not p['paved']][:8])
for a in ea.get_all_level_actors():
 if isinstance(a,unreal.PiedmontPathSpline) and str(a.get_editor_property('osm_way_id'))=='pride-turn-review':ea.destroy_actor(a)
a=ea.spawn_actor_from_class(unreal.PiedmontPathSpline,unreal.Vector());a.set_actor_label('Pride junction drive fixture');a.set_editor_property('osm_way_id','pride-turn-review');a.set_editor_property('bArtifactEligible',False);a.set_centerline(points);a.tags=[unreal.Name('PrideDriveFixture')];assert unreal.EditorLoadingAndSavingUtils.save_map(world,'/Game/PiedmontRide/Maps/PiedmontPrideStreetReview');assert hashlib.sha256(main.read_bytes()).hexdigest()==before
(root/'work/pride-drive-fixture.json').write_text(json.dumps({'main_unchanged':True,'points':len(points),'probes':probes,'scope':'All fixture centerline points have native paved support; actual moving bike not tested yet.'},indent=2)+'\n')
