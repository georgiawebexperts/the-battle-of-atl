import unreal,json
from pathlib import Path
root=Path(unreal.Paths.project_dir()).resolve();ea=unreal.get_editor_subsystem(unreal.EditorActorSubsystem);reports=[]
for name in ('PiedmontWorld','PiedmontKrogApproachReview'):
 assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/'+name)
 unreal.PiedmontWorldTools.finish_editor_asset_loading()
 assert sum(a.actor_has_tag('KrogApproachReview') for a in ea.get_all_level_actors())==(0 if name=='PiedmontWorld' else 1)
 a=next(a for a in ea.get_all_level_actors() if isinstance(a,unreal.PiedmontPathSpline) and a.actor_has_tag('BattleKrog_2'))
 points=[a.centerline.get_location_at_spline_point(i,unreal.SplineCoordinateSpace.WORLD) for i in range(a.centerline.get_number_of_spline_points())];failures=[]
 for i,(p,q) in enumerate(zip(points,points[1:])):
  # Use actual ground, since legacy mapped Z is not the final crowned road.
  pp=unreal.PiedmontWorldTools.trace_world_surface(p+unreal.Vector(0,0,400),p-unreal.Vector(0,0,400))[0]
  qq=unreal.PiedmontWorldTools.trace_world_surface(q+unreal.Vector(0,0,400),q-unreal.Vector(0,0,400))[0]
  for z in (95,155):
   hit=unreal.PiedmontWorldTools.trace_world_surface(pp+unreal.Vector(0,0,z),qq+unreal.Vector(0,0,z),62)
   if hit:failures.append({'segment':i,'height':z,'actor':hit[1].get_actor_label(),'xyz':[hit[0].x,hit[0].y,hit[0].z]})
 reports.append({'map':name,'failures':failures,'passed':not failures})
(root/'Tests/Results/2026-09-13-krog-approach-clearance.json').write_text(json.dumps({'maps':reports,'scope':'Native 62cm sphere sweeps at two rider heights along entire approach, versus static visibility collision. No runtime car clearance or full capsule proof.'},indent=2)+'\n')
