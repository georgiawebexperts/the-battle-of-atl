"""Move practice cutoff via a map-owned marker; retain the main map unchanged."""
import unreal,pathlib,json,math,hashlib
root=pathlib.Path(unreal.Paths.project_dir());main=root/'Content/PiedmontRide/Maps/PiedmontWorld.umap';before=hashlib.sha256(main.read_bytes()).hexdigest();assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/PiedmontPrideStreetReview');world=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world();ea=unreal.get_editor_subsystem(unreal.EditorActorSubsystem);unreal.PiedmontWorldTools.finish_editor_asset_loading();data=json.loads((root/'SourceAssets/Terrain/PrideIntersection/survey.json').read_text());position=None
for row in data['roads']:
 if row['tags']['name']!='Piedmont Avenue Northeast':continue
 for a,b in zip(row['points_world_cm'],row['points_world_cm'][1:]):
  if min(a[1],b[1])<=14600<=max(a[1],b[1]) and a[1]!=b[1]:
   t=(14600-a[1])/(b[1]-a[1]);x=a[0]+t*(b[0]-a[0]);dx,dy=b[0]-a[0],b[1]-a[1]
   if dy<0:dx,dy=-dx,-dy
   position=(x,14600);yaw=math.degrees(math.atan2(dy,dx));break
assert position
hit=unreal.PiedmontWorldTools.trace_world_surface(unreal.Vector(*position,1600),unreal.Vector(*position,-1200));assert hit and hit[1].actor_has_tag('RidePath')
for actor in ea.get_all_level_actors():
 if actor.actor_has_tag('PrideStreetBoundary'):ea.destroy_actor(actor)
a=ea.spawn_actor_from_class(unreal.TargetPoint,hit[0],unreal.Rotator(yaw=yaw));a.set_actor_label('Piedmont south construction boundary');a.tags=[unreal.Name('PrideStreetBoundary')];a.set_folder_path('Midtown/PrideIntersection');assert unreal.EditorLoadingAndSavingUtils.save_map(world,'/Game/PiedmontRide/Maps/PiedmontPrideStreetReview');assert hashlib.sha256(main.read_bytes()).hexdigest()==before
(root/'work/pride-boundary-placement.json').write_text(json.dumps({'position':[hit[0].x,hit[0].y,hit[0].z],'yaw':yaw,'main_unchanged_sha256':before,'scope':'Review marker placed on mapped street; runtime fence and notice tests pending'},indent=2)+'\n')
