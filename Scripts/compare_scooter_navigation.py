"""Compare retained route samples in main and scooter navigation review maps."""
import unreal,json
from pathlib import Path
root=Path(unreal.Paths.project_dir()).resolve();ea=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
start=json.loads((root/'SourceAssets/Terrain/lake-test-points.json').read_text())['safe_start']
def survey(name):
 assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/'+name)
 unreal.PiedmontWorldTools.finish_editor_asset_loading();anchor=unreal.PiedmontWorldTools.project_park_navigation(unreal.Vector(*start));assert anchor
 rows={}
 for a in ea.get_all_level_actors():
  if not isinstance(a,unreal.PiedmontPathSpline):continue
  count=a.centerline.get_number_of_spline_points();dense=any(str(t).startswith('BattleKrog_') for t in a.tags)
  for i in sorted(set([0,count//2,count-1]+(list(range(0,count,5)) if dense else []))):
   raw=a.centerline.get_location_at_spline_point(i,unreal.SplineCoordinateSpace.WORLD);p=unreal.PiedmontWorldTools.project_park_navigation(raw);length=unreal.PiedmontWorldTools.park_route_length(anchor,p) if p else -1
   rows[a.get_actor_label()+':'+str(i)]={'reachable':length>=0,'length_cm':length,'krog':dense}
 return rows
before=survey('PiedmontWorld');after=survey('PiedmontScooterReview');missing=sorted(set(before)-set(after));regressions=[k for k,v in before.items() if v['reachable'] and (k not in after or not after[k]['reachable'])]
r={'before_samples':len(before),'after_samples':len(after),'before_reachable':sum(v['reachable'] for v in before.values()),'after_reachable':sum(v['reachable'] for v in after.values()),'missing':missing,'regressions':regressions,'passed':not missing and not regressions,'scope':'Retained route endpoints/midpoints plus dense Krog samples; no full polygon, traffic or runtime route acceptance. Read-only; main map unchanged.'}
(root/'Tests/Results/2026-09-13-scooter-navigation-regression.json').write_text(json.dumps(r,indent=2)+'\n');assert r['passed'],r
