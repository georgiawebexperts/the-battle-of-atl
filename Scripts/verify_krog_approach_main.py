"""Fresh read of promoted main approach and its existing scooter/traffic settings."""
import unreal,json,runpy
from pathlib import Path
root=Path(unreal.Paths.project_dir()).resolve();ea=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
(root/'Tests/Results/2026-09-13-krog-approach-main-fresh-read.json').write_text(json.dumps({'passed':False,'status':'Verification started; incomplete or failed until replaced with a successful report.'})+'\n')
def read(name):
 assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/'+name)
 unreal.PiedmontWorldTools.finish_editor_asset_loading()
 patches=[a for a in ea.get_all_level_actors() if a.actor_has_tag('KrogApproachReview')];assert len(patches)==1
 patch=patches[0];assert patch.static_mesh_component.get_collision_enabled()==unreal.CollisionEnabled.QUERY_AND_PHYSICS
 s=next(a for a in ea.get_all_level_actors() if a.actor_has_tag('BattleKrog_2'))
 points=[s.centerline.get_location_at_spline_point(i,unreal.SplineCoordinateSpace.WORLD) for i in range(s.centerline.get_number_of_spline_points())]
 return points,patch.static_mesh_component.static_mesh.get_path_name()
review,mesh=read('PiedmontKrogApproachReview');main,main_mesh=read('PiedmontWorld');assert mesh==main_mesh and len(main)==len(review)
assert max((a-b).length() for a,b in zip(main,review))<.01
samples=[]
for p in main[-40:]:
 hit=unreal.PiedmontWorldTools.trace_world_surface(p+unreal.Vector(0,0,100),p-unreal.Vector(0,0,100));assert hit and hit[1].actor_has_tag('RidePath') and abs(hit[0].z-p.z)<2
 samples.append(hit[1].get_actor_label())
runpy.run_path(str(root/'Scripts/verify_scooter_main_install.py'))
(root/'Tests/Results/2026-09-13-krog-approach-main-fresh-read.json').write_text(json.dumps({'passed':True,'points':len(main),'surface_samples':len(samples),'new_surface_hits':sum(s=='Krog crossing concrete approach' for s in samples),'mesh':mesh,'scope':'Fresh-process main/review spline agreement, blocking mesh and native ground probes. Existing scooter/loop/navigation verification also passed. Full main ride and packaged acceptance remain separate.'},indent=2)+'\n')
