"""Integrate the traversable first exterior pass; preserve incomplete art status."""
import json
from pathlib import Path
import unreal
root=Path(unreal.Paths.project_dir())
assert json.loads((root/'Tests/Results/2026-09-12-native-irwin-entrance-walk.json').read_text())['passed']
assert json.loads((root/'Tests/Results/2026-09-12-irwin-aprons-render.json').read_text()).get('exterior_accepted_for_integration')
assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/PiedmontIrwinSidewalkReview')
ea=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
rows=[(a.get_actor_label(),a.static_mesh_component.static_mesh.get_path_name(),a.get_actor_transform(),str(a.static_mesh_component.get_collision_profile_name()),a.actor_has_tag('IrwinSidewalk')) for a in ea.get_all_level_actors() if a.actor_has_tag('IrwinBuildingReview') or a.actor_has_tag('IrwinSidewalk')];assert len(rows)==6,len(rows)
assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/PiedmontWorld')
assert not any(a.actor_has_tag('IrwinStreetscape') for a in ea.get_all_level_actors()),'Already installed'
for label,path,transform,profile,is_walk in rows:
 a=ea.spawn_actor_from_class(unreal.StaticMeshActor,transform.translation);a.set_actor_transform(transform,False,False);a.set_actor_label(label);a.tags=[unreal.Name('IrwinStreetscape'),unreal.Name('RidePath' if is_walk or label.endswith('Landing') else 'RideBarrier')];a.set_folder_path('Eastside/IrwinLake')
 a.static_mesh_component.set_static_mesh(unreal.load_asset(path));a.static_mesh_component.set_collision_profile_name(profile)
unreal.PiedmontWorldTools.finish_editor_asset_loading();failures=[];count=0
for filename in ['SourceAssets/Terrain/IrwinTraffic/car-lane-probes.json','SourceAssets/Terrain/IrwinBuildings/trail-probes.json']:
 for row in json.loads((root/filename).read_text())['samples']:
  x,y,z=row['xyz'];hit=unreal.PiedmontWorldTools.trace_world_surface(unreal.Vector(x,y,z+100),unreal.Vector(x,y,z-100));count+=1
  if not hit or abs(hit[0].z-z)>1.25 or hit[1].actor_has_tag('IrwinStreetscape'):failures.append(row)
assert not failures,len(failures)
assert unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).save_current_level()
(root/'Tests/Results/2026-09-12-irwin-streetscape-install.json').write_text(json.dumps({'main_map_changed':True,'actors':len(rows),'road_and_trail_probes':count,'support_passed':True,'desktop_build_updated':False,'scope':'First exterior pass only. Generic facades and estimated building heights; landmark likeness, furnishings, performance and packaged acceptance remain unfinished.'},indent=2)+'\n')
