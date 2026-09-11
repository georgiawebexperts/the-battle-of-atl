"""Build navigation from installed path/deck surfaces and inspect sourced path access."""
import unreal,pathlib,json,time,traceback
job=globals().get("WORLD_JOB",{});dense_ids={str(x) for x in job.get("dense_ids",[])}
p=pathlib.Path(unreal.Paths.project_dir());world=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world();ea=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
paths=[a for a in ea.get_all_level_actors() if isinstance(a,unreal.PiedmontPathSpline)];points=[]
for a in paths:
 for i in range(a.centerline.get_number_of_spline_points()):points.append(a.centerline.get_location_at_spline_point(i,unreal.SplineCoordinateSpace.WORLD))
lo=[min(getattr(v,k) for v in points) for k in ['x','y','z']];hi=[max(getattr(v,k) for v in points) for k in ['x','y','z']];center=unreal.Vector(*[(a+b)/2 for a,b in zip(lo,hi)]);extent=unreal.Vector(*[(b-a)/2+500 for a,b in zip(lo,hi)])
if not job.get('audit_only') and not unreal.PiedmontWorldTools.build_park_navigation(center,extent):raise RuntimeError('Navigation setup failed')
bounds=next(a for a in ea.get_all_level_actors() if a.actor_has_tag('PiedmontNavBounds'));actual=bounds.get_actor_location()
if max(abs(getattr(actual,k)-getattr(center,k)) for k in ['x','y','z'])>1:raise RuntimeError('Navigation bounds did not retain their geographic center')
started=time.monotonic();report={'status':'building','path_count':len(paths)};(p/('Scripts/park-navigation-dense.json' if dense_ids else 'Scripts/park-navigation.json')).write_text(json.dumps(report,indent=2))
def tick(delta):
 global handle
 try:
  if time.monotonic()-started<3:return
  if unreal.PiedmontWorldTools.is_park_navigation_building():
   if time.monotonic()-started>240:raise RuntimeError('Navigation build still running after four minutes')
   return
  unreal.unregister_slate_post_tick_callback(handle)
  starts=json.loads((p/'SourceAssets/Terrain/lake-test-points.json').read_text());anchor=unreal.PiedmontWorldTools.project_park_navigation(unreal.Vector(*starts['safe_start']));rows=[]
  for a in paths:
   count=a.centerline.get_number_of_spline_points();samples=[]
   for i in (range(count) if str(a.get_editor_property('osm_way_id')) in dense_ids else sorted(set([0,count//2,count-1]))):
    v=a.centerline.get_location_at_spline_point(i,unreal.SplineCoordinateSpace.WORLD);projected=unreal.PiedmontWorldTools.project_park_navigation(v);length=unreal.PiedmontWorldTools.park_route_length(anchor,projected) if anchor and projected else -1
    samples.append({'point':i,'location_cm':[v.x,v.y,v.z],'projected':bool(projected),'reachable_from_start':length>=0,'search_limit_reached':length==-2,'route_length_cm':length})
   rows.append({'osm_id':a.get_editor_property('osm_way_id'),'label':a.get_actor_label(),'samples':samples,'fully_reachable':all(s['reachable_from_start'] for s in samples)})
  report={'status':'audited' if anchor else 'failed','scope':'Recast navigation over paved/gravel paths, decks and barriers; excludes open grass/water. Sampled route queries, not full bike ride acceptance.','bounds':[{'location':str(a.get_actor_location()),'extent':str(a.get_actor_bounds(False))} for a in ea.get_all_level_actors() if a.actor_has_tag('PiedmontNavBounds')],'job':job,'search_limit_samples':sum(sum(x['search_limit_reached'] for x in r['samples']) for r in rows),'anchor_projected':bool(anchor),'path_count':len(rows),'reachable_path_samples':sum(sum(x['reachable_from_start'] for x in r['samples']) for r in rows),'sample_count':sum(len(r['samples']) for r in rows),'fully_reachable_paths':sum(r['fully_reachable'] for r in rows),'paths':rows};report['level_saved']=None if job.get('audit_only') else bool(unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).save_current_level());(p/('Scripts/park-navigation-dense.json' if dense_ids else 'Scripts/park-navigation.json')).write_text(json.dumps(report,indent=2))
 except Exception:
  (p/('Scripts/park-navigation-dense.json' if dense_ids else 'Scripts/park-navigation.json')).write_text(json.dumps({'status':'error','error':traceback.format_exc()},indent=2))
handle=unreal.register_slate_post_tick_callback(tick)
