"""Persist reviewed mixed tree stations in new editable PCG groups in the main world."""
import json,unreal
from pathlib import Path
root=Path(unreal.Paths.project_dir())
assert json.loads((root/'Tests/Results/2026-09-12-canopy-runtime-main-mixed.json').read_text())['mixed_canopy_visual_candidate_accepted']
manifest=json.loads((root/'SourceAssets/Terrain/park-mixed-canopy.json').read_text());stations=json.loads((root/'SourceAssets/Terrain/park-tree-stations.json').read_text())['trees']
assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/PiedmontWorld')
ea=unreal.get_editor_subsystem(unreal.EditorActorSubsystem);world=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
assert not any(a.actor_has_tag('BattleMixedCanopy') for a in ea.get_all_level_actors())
old=[a for a in ea.get_all_level_actors() if a.actor_has_tag('BattleParkCanopy')];assert old
old_count=sum(c.get_instance_count() for a in old for c in a.get_components_by_class(unreal.InstancedStaticMeshComponent));assert old_count==587,old_count
groups={}
for row in manifest['accepted']:
 station=stations[row['index']];mesh=unreal.load_asset(row['mesh']);assert mesh;x,y=station['xy_cm']
 hit=unreal.PiedmontWorldTools.trace_world_surface(unreal.Vector(x,y,7000),unreal.Vector(x,y,-7000),0);assert hit and isinstance(hit[1],unreal.Landscape),row['index']
 b=mesh.get_bounds();scale=station['height_game_cm']/(b.box_extent.z*2)
 transform=unreal.Transform(location=unreal.Vector(x,y,hit[0].z-(b.origin.z-b.box_extent.z)*scale),rotation=unreal.Rotator(yaw=station['yaw']),scale=unreal.Vector(scale,scale,scale))
 groups.setdefault(row['mesh'],[]).append(transform)
created=[];rows=[]
for path,transforms in sorted(groups.items()):
 name='Mature' if 'HillTree' in path else 'Forest' if 'Forest' in path else 'Field';folder='/Game/BattleForTheA/Environment/Park/Mixed/'+name
 assert not unreal.EditorAssetLibrary.does_asset_exist(folder+'/PCG_ParkCanopy'),folder
 actor=unreal.PiedmontWorldTools.create_park_foliage_at_path(transforms,unreal.load_asset(path),folder);assert actor
 actor.set_actor_label('Piedmont mixed canopy '+name);actor.tags=list(actor.tags)+[unreal.Name('BattleMixedCanopy')];actor.set_folder_path('Piedmont/Canopy');created.append(actor)
 for _ in range(900):
  unreal.PiedmontWorldTools.tick_scene_review()
  count=sum(c.get_instance_count() for c in actor.get_components_by_class(unreal.InstancedStaticMeshComponent))
  if count==len(transforms):break
 assert count==len(transforms),(name,count,len(transforms))
 assert unreal.EditorAssetLibrary.save_asset(folder+'/DA_TreeStations');assert unreal.EditorAssetLibrary.save_asset(folder+'/PCG_ParkCanopy')
 rows.append({'group':name,'instances':count,'mesh':path,'graph':folder+'/PCG_ParkCanopy'})
assert sum(r['instances'] for r in rows)==587
for actor in old:ea.destroy_actor(actor)
unreal.PiedmontWorldTools.finish_editor_asset_loading()
assert unreal.EditorLoadingAndSavingUtils.save_map(world,'/Game/PiedmontRide/Maps/PiedmontWorld')
(root/'Tests/Results/2026-09-12-mixed-canopy-install.json').write_text(json.dumps({'main_map_changed':True,'previous_instances':old_count,'instances':587,'groups':rows,'original_pcg_assets_preserved':True,'desktop_build_updated':False,'scope':'Editable PCG generation counts and same-world save verified. Native saved-world visual/water and traversal/performance checks pending; no trunk collision.'},indent=2)+'\n')
