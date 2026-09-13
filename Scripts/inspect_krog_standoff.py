"""Read saved route geometry and collision support around the failed ride; no map edits."""
import unreal,json
from pathlib import Path
root=Path(unreal.Paths.project_dir()).resolve()
assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/PiedmontWorld')
unreal.PiedmontWorldTools.finish_editor_asset_loading()
ea=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
paths=[];lanes=[]
xyz=lambda p:[p.x,p.y,p.z]
for a in ea.get_all_level_actors():
 if isinstance(a,unreal.PiedmontPathSpline) and any(str(t).startswith('BattleKrog_') for t in a.tags):
  paths.append({'tags':[str(t) for t in a.tags],'points':[xyz(a.centerline.get_location_at_spline_point(i,unreal.SplineCoordinateSpace.WORLD)) for i in range(a.centerline.get_number_of_spline_points())]})
 if isinstance(a,unreal.BattleRoadTrafficDirector) and a.actor_has_tag('KrogTrafficReview'):
  for l in a.get_editor_property('Lanes'):
   lanes.append({'points':[xyz(p) for p in l.get_editor_property('Points')]})
samples=[]
for x in range(28200,30601,100):
 for y in range(114800,116501,100):
  hit=unreal.PiedmontWorldTools.trace_world_surface(unreal.Vector(x,y,1400),unreal.Vector(x,y,600))
  if hit:
   p,a=hit;samples.append({'xyz':xyz(p),'paved':a.actor_has_tag('RidePath'),'label':a.get_actor_label()})
(root/'Tests/Results/2026-09-13-krog-standoff-geometry.json').write_text(json.dumps({'paths':paths,'lanes':lanes,'ground_samples':samples,'scope':'Read-only saved main geometry and native surface probes. Incident positions are from the failed main ride report; not a replay.'},indent=2)+'\n')
