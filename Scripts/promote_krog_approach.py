"""Transfer reviewed approach onto backed-up current main, then verify sampled navigation."""
import unreal,json,hashlib,shutil,datetime
from pathlib import Path
root=Path(unreal.Paths.project_dir()).resolve();ea=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
for name,key in [('2026-09-13-krog-verified-approach-ride.json','passed'),('2026-09-13-krog-approach-render.json','visual_accepted'),('2026-09-13-krog-approach-navigation-regression.json','passed')]:
 assert json.loads((root/'Tests/Results'/name).read_text())[key],name
assert json.loads((root/'Tests/Results/2026-09-13-krog-verified-approach-ride.json').read_text())['candidate_actor_verified']
assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/PiedmontKrogApproachReview')
patches=[a for a in ea.get_all_level_actors() if a.actor_has_tag('KrogApproachReview')];assert len(patches)==1
mesh_path=patches[0].static_mesh_component.static_mesh.get_path_name()
s=next(a for a in ea.get_all_level_actors() if a.actor_has_tag('BattleKrog_2'));points=[]
for i in range(s.centerline.get_number_of_spline_points()):
 p=s.centerline.get_location_at_spline_point(i,unreal.SplineCoordinateSpace.WORLD);points.append((p.x,p.y,p.z))
assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/PiedmontWorld')
unreal.PiedmontWorldTools.finish_editor_asset_loading()
assert not any(a.actor_has_tag('KrogApproachReview') for a in ea.get_all_level_actors()),'Already present; inspect instead of duplicating'
start=unreal.Vector(*json.loads((root/'SourceAssets/Terrain/lake-test-points.json').read_text())['safe_start'])
def survey():
 anchor=unreal.PiedmontWorldTools.project_park_navigation(start);assert anchor
 rows={}
 for a in ea.get_all_level_actors():
  if not isinstance(a,unreal.PiedmontPathSpline):continue
  count=a.centerline.get_number_of_spline_points();dense=any(str(t).startswith('BattleKrog_') for t in a.tags)
  for i in sorted(set([0,count//2,count-1]+(list(range(0,count,5)) if dense else []))):
   p=unreal.PiedmontWorldTools.project_park_navigation(a.centerline.get_location_at_spline_point(i,unreal.SplineCoordinateSpace.WORLD));length=unreal.PiedmontWorldTools.park_route_length(anchor,p) if p else -1
   rows[a.get_actor_label()+':'+str(i)]={'reachable':length>=0,'krog':dense}
 return rows
before_nav=survey()
def ids():return {tuple(a.get_editor_property('actor_guid').get_editor_property(k) for k in ('a','b','c','d')) for a in ea.get_all_level_actors() if not isinstance(a,unreal.RecastNavMesh)}
before_ids=ids();main=root/'Content/PiedmontRide/Maps/PiedmontWorld.umap'
backup=root/'work/map-backups'/datetime.datetime.now().strftime('pre-krog-approach-%Y%m%d-%H%M%S');backup.mkdir(parents=True);shutil.copy2(main,backup/main.name)
patch=ea.spawn_actor_from_class(unreal.StaticMeshActor,unreal.Vector());patch.set_actor_label('Krog crossing concrete approach');patch.tags=[unreal.Name('RidePath'),unreal.Name('KrogApproachReview')];patch.static_mesh_component.set_static_mesh(unreal.load_asset(mesh_path));patch.static_mesh_component.set_collision_profile_name('BlockAll')
s=next(a for a in ea.get_all_level_actors() if a.actor_has_tag('BattleKrog_2'));s.set_centerline([unreal.Vector(*p) for p in points])
all_points=[]
for a in ea.get_all_level_actors():
 if isinstance(a,unreal.PiedmontPathSpline):all_points.extend(a.centerline.get_location_at_spline_point(i,unreal.SplineCoordinateSpace.WORLD) for i in range(a.centerline.get_number_of_spline_points()))
lo=[min(getattr(p,k) for p in all_points) for k in ('x','y','z')];hi=[max(getattr(p,k) for p in all_points) for k in ('x','y','z')]
assert unreal.PiedmontWorldTools.build_park_navigation(unreal.Vector(*[(a+b)/2 for a,b in zip(lo,hi)]),unreal.Vector(*[(b-a)/2+500 for a,b in zip(lo,hi)]))
assert unreal.PiedmontWorldTools.finish_park_navigation_build()
after_nav=survey();regressions=[k for k,v in before_nav.items() if v['reachable'] and not after_nav.get(k,{}).get('reachable')];krog_failures=[k for k,v in after_nav.items() if v['krog'] and not v['reachable']]
assert not regressions and not krog_failures,(regressions,krog_failures)
assert before_ids<ids() and len(ids()-before_ids)==1,'Existing main actors changed'
assert unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).save_current_level()
(root/'Tests/Results/2026-09-13-krog-approach-main-promotion.json').write_text(json.dumps({'passed':True,'backup':str(backup/main.name),'backup_sha256':hashlib.sha256((backup/main.name).read_bytes()).hexdigest(),'main_sha256':hashlib.sha256(main.read_bytes()).hexdigest(),'samples':len(after_nav),'reachable':sum(v['reachable'] for v in after_nav.values()),'regressions':regressions,'krog_failures':krog_failures,'scope':'One concrete actor and reviewed BattleKrog_2 points transferred to current main, preserving other actor GUIDs. Navigation rebuilt and sampled. Fresh main runtime/cooked acceptance remains pending.'},indent=2)+'\n')
