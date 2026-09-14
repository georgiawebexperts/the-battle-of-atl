"""Extend the verified street fixture through the runtime tutorial road to the gate."""
import unreal,pathlib,json,math,hashlib
root=pathlib.Path(unreal.Paths.project_dir());main=root/'Content/PiedmontRide/Maps/PiedmontWorld.umap';before=hashlib.sha256(main.read_bytes()).hexdigest();assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/PiedmontPrideStreetReview');world=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world();ea=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
a=next(a for a in ea.get_all_level_actors() if isinstance(a,unreal.PiedmontPathSpline) and str(a.get_editor_property('osm_way_id'))=='pride-turn-review');s=a.get_editor_property('centerline');pts=[s.get_location_at_spline_point(i,unreal.SplineCoordinateSpace.WORLD) for i in range(s.get_number_of_spline_points())];b=json.loads((root/'SourceAssets/Terrain/Tutorial/block.json').read_text());extra=list(reversed(b['market_approach']));start=min(range(len(b['road'])),key=lambda i:math.dist(b['road'][i][:2],extra[-1][:2]));extra+=b['road'][start:];g=b['gate'];extra.append([g[0]+600,g[1],g[2]])
# Native moving-bike audit must establish support for these runtime-created road pieces.
# Do not substitute editor Landscape hits for proof of the tutorial's actual pavement.
for p in extra:
 end=unreal.Vector(*p);a=pts[-1];distance=math.hypot(end.x-a.x,end.y-a.y);count=max(1,math.ceil(distance/50))
 for i in range(1,count+1):pts.append(a+(end-a)*(i/count))
for actor in ea.get_all_level_actors():
 if isinstance(actor,unreal.PiedmontPathSpline) and str(actor.get_editor_property('osm_way_id'))=='pride-gate-review':ea.destroy_actor(actor)
a=ea.spawn_actor_from_class(unreal.PiedmontPathSpline,unreal.Vector());a.set_actor_label('Pride to 14th gate drive fixture');a.set_editor_property('osm_way_id','pride-gate-review');a.set_editor_property('bArtifactEligible',False);a.set_centerline(pts);a.tags=[unreal.Name('PrideDriveFixture')];assert unreal.EditorLoadingAndSavingUtils.save_map(world,'/Game/PiedmontRide/Maps/PiedmontPrideStreetReview');assert hashlib.sha256(main.read_bytes()).hexdigest()==before
(root/'work/pride-gate-fixture.json').write_text(json.dumps({'points':len(pts),'gate':g,'end':extra[-1],'main_unchanged':True,'runtime_road_support_verified':False,'scope':'Full route fixture includes dynamic tutorial paving, which must be verified in game.'},indent=2)+'\n')
