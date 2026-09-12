"""Rebuild combined-world navigation and compare sampled path connectivity."""
import unreal,json,time
from pathlib import Path
root=Path(unreal.Paths.project_dir());report_path=root/'Tests/Results/2026-09-12-krog-world-navigation.json'
assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/PiedmontKrogWorldReview')
unreal.PiedmontWorldTools.finish_editor_asset_loading();ea=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
paths=[a for a in ea.get_all_level_actors() if isinstance(a,unreal.PiedmontPathSpline)]
all_points=[];samples=[]
for a in paths:
 count=a.centerline.get_number_of_spline_points();points=[a.centerline.get_location_at_spline_point(i,unreal.SplineCoordinateSpace.WORLD) for i in range(count)];all_points+=points
 dense=any(str(t).startswith('BattleKrog_') for t in a.tags)
 indices=sorted(set([0,count//2,count-1]+(list(range(0,count,5)) if dense else [])))
 for i in indices:samples.append({'label':a.get_actor_label(),'index':i,'point':points[i],'krog':dense})
start=json.loads((root/'SourceAssets/Terrain/lake-test-points.json').read_text())['safe_start']
def survey():
 anchor=unreal.PiedmontWorldTools.project_park_navigation(unreal.Vector(*start));rows=[]
 for s in samples:
  p=unreal.PiedmontWorldTools.project_park_navigation(s['point']);distance=unreal.PiedmontWorldTools.park_route_length(anchor,p) if anchor and p else -1
  rows.append({'label':s['label'],'index':s['index'],'krog':s['krog'],'projected':bool(p),'reachable':distance>=0,'route_length_cm':distance,'xyz':[s['point'].x,s['point'].y,s['point'].z]})
 return {'anchor_projected':bool(anchor),'samples':rows}
before=survey();report_path.write_text(json.dumps({'status':'building','before':before,'main_map_changed':False},indent=2)+'\n')
lo=[min(getattr(v,k) for v in all_points) for k in ['x','y','z']];hi=[max(getattr(v,k) for v in all_points) for k in ['x','y','z']]
center=unreal.Vector(*[(a+b)/2 for a,b in zip(lo,hi)]);extent=unreal.Vector(*[(b-a)/2+500 for a,b in zip(lo,hi)])
assert unreal.PiedmontWorldTools.build_park_navigation(center,extent)
assert unreal.PiedmontWorldTools.finish_park_navigation_build()
after=survey();regressions=[{'before':a,'after':b} for a,b in zip(before['samples'],after['samples']) if a['reachable'] and not b['reachable']]
krog_failures=[s for s in after['samples'] if s['krog'] and not s['reachable']]
passed=after['anchor_projected'] and not regressions and not krog_failures
report={'status':'checked','passed':passed,'before':before,'after':after,'regressions':regressions,'krog_failures':krog_failures,'main_map_changed':False,'scope':'Rebuilt Recast navigation; path endpoints/midpoints and dense Krog samples reachable from park anchor. Not actual NPC pursuit, full polygon coverage or runtime crowd performance.'}
report['saved']=bool(unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).save_current_level()) if passed else False
report_path.write_text(json.dumps(report,indent=2)+'\n');assert passed,(len(regressions),len(krog_failures))
