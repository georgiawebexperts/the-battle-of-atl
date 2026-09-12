"""Enable existing narrow trunk capsules and probe every saved tree before saving main."""
import json,unreal
from pathlib import Path
root=Path(unreal.Paths.project_dir());assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/PiedmontWorld')
ea=unreal.get_editor_subsystem(unreal.EditorActorSubsystem);actors=[a for a in ea.get_all_level_actors() if a.actor_has_tag('BattleMixedCanopy')];assert len(actors)==3
meshes={}
for actor in actors:
 for component in actor.get_components_by_class(unreal.InstancedStaticMeshComponent):
  mesh=component.static_mesh;meshes[mesh.get_path_name()]=mesh
assert len(meshes)==3
for path,mesh in meshes.items():
 body=mesh.get_editor_property('body_setup');agg=body.get_editor_property('agg_geom');assert len(agg.get_editor_property('sphyl_elems'))==1 and not agg.get_editor_property('box_elems') and not agg.get_editor_property('sphere_elems') and not agg.get_editor_property('convex_elems')
 body.set_editor_property('collision_trace_flag',unreal.CollisionTraceFlag.CTF_USE_SIMPLE_AS_COMPLEX);assert unreal.EditorAssetLibrary.save_loaded_asset(mesh)
for actor in actors:assert unreal.PiedmontWorldTools.enable_park_trunk_collision(actor)
unreal.PiedmontWorldTools.finish_editor_asset_loading()
failures=[];count=0
for actor in actors:
 for component in actor.get_components_by_class(unreal.InstancedStaticMeshComponent):
  shape=component.static_mesh.get_editor_property('body_setup').get_editor_property('agg_geom').get_editor_property('sphyl_elems')[0]
  for i in range(component.get_instance_count()):
   transform=component.get_instance_transform(i,world_space=True);centre=unreal.MathLibrary.transform_location(transform,shape.get_editor_property('center'));radius=shape.get_editor_property('radius')*max(transform.scale3d.x,transform.scale3d.y,transform.scale3d.z);span=radius*2+40
   hit=unreal.PiedmontWorldTools.trace_world_surface(centre-unreal.Vector(span,0,0),centre+unreal.Vector(span,0,0),0)
   if not hit or hit[1]!=actor:failures.append({'actor':actor.get_actor_label(),'instance':i,'reason':'trunk_not_hit'})
   side=centre+unreal.Vector(0,radius+30,0);miss=unreal.PiedmontWorldTools.trace_world_surface(side-unreal.Vector(span,0,0),side+unreal.Vector(span,0,0),0)
   if miss and miss[1].actor_has_tag('RideTree'):failures.append({'actor':actor.get_actor_label(),'instance':i,'reason':'unexpected_side_blocker'})
   count+=1
assert count==587
report={'instances':count,'failures':failures,'passed':not failures,'main_map_changed':False,'scope':'Native horizontal trunk hit and adjacent miss probes for all instances. No physical bike/walking traversal, slope/path or collision animation acceptance.'}
if not failures:
 for name in ['Mature','Field','Forest']:assert unreal.EditorAssetLibrary.save_asset('/Game/BattleForTheA/Environment/Park/Mixed/'+name+'/PCG_ParkCanopy')
 assert unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).save_current_level();report['main_map_changed']=True
(root/'Tests/Results/2026-09-12-tree-trunk-collision.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps({'passed':report['passed'],'instances':count,'failures':len(failures)}));assert not failures
