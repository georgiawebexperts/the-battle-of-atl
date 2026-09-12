"""Promote verified combined scenery, retaining an exact pre-promotion map backup."""
from pathlib import Path
from collections import Counter
import hashlib,json,shutil,datetime
import unreal
root=Path(unreal.Paths.project_dir()).resolve();maps=root/'Content/PiedmontRide/Maps'
main='/Game/PiedmontRide/Maps/PiedmontWorld';candidate='/Game/PiedmontRide/Maps/PiedmontKrogWorldReview'
ea=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
def inventory():return Counter((a.get_actor_label(),a.get_class().get_name()) for a in ea.get_all_level_actors())
def traffic():
 result={}
 for actor in ea.get_all_level_actors():
  if not isinstance(actor,unreal.BattleRoadTrafficDirector):continue
  lanes=[]
  for lane in actor.get_editor_property('Lanes'):
   lanes.append({'name':str(lane.get_editor_property('Name')),'points':[[p.x,p.y,p.z] for p in lane.get_editor_property('Points')],
    'speed':lane.get_editor_property('CruiseSpeed'),'crossings':[(b.get_editor_property('Crossing').get_actor_label(),b.get_editor_property('StopDistance')) for b in lane.get_editor_property('Crossings')]})
  result[actor.get_actor_label()]={'lanes':lanes,**{p:actor.get_editor_property(p) for p in ['MaxCars','MaxCarsPerLane','SpawnInterval']}}
 return result
for filename in ['2026-09-12-krog-world-yielding-crowds.json','2026-09-12-krog-shell-winding.json','2026-09-12-krog-outward-navigation.json']:
 assert json.loads((root/'Tests/Results'/filename).read_text())['passed'],filename
assert unreal.EditorLoadingAndSavingUtils.load_map(main)
original=inventory();original_traffic=traffic()
backup=root/'work/map-backups'/datetime.datetime.now().strftime('pre-krog-%Y%m%d-%H%M%S');backup.mkdir(parents=True,exist_ok=False)
map_file=maps/'PiedmontWorld.umap';backup_file=backup/map_file.name
shutil.copy2(map_file,backup_file)
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
assert sha(map_file)==sha(backup_file)
assert unreal.EditorLoadingAndSavingUtils.load_map(candidate)
current=inventory();missing=original-current
assert all(label=='Krog route SM_KrogRoute_Concrete_7_1' or label.startswith('Krog darkness ') for label,cls in missing),list(missing)
current_traffic=traffic()
assert all(current_traffic.get(label)==value for label,value in original_traffic.items()),'Original traffic changed'
assert len(current_traffic)==4
actors=ea.get_all_level_actors()
assert sum(a.actor_has_tag('ParkGrassWalkingLink') for a in actors)==1
for kind in ['Shell','Columns']:
 actor=next(a for a in actors if a.get_actor_label()=='Krog route SM_KrogTunnel_'+kind)
 assert actor.static_mesh_component.static_mesh.get_name()=='SM_KrogOutward_'+kind
assert sum(isinstance(a,unreal.PiedmontDarkZone) and a.get_actor_label().startswith('Krog darkness ') for a in actors)==32
lights=[a.get_component_by_class(unreal.PointLightComponent) for a in actors if isinstance(a,unreal.PointLight) and a.actor_has_tag('KrogUtilityLightReview')]
assert len(lights)==6 and all(l.get_editor_property('intensity')==2 for l in lights)
world=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
assert unreal.EditorLoadingAndSavingUtils.save_map(world,main)
assert unreal.EditorLoadingAndSavingUtils.load_map(main)
assert inventory()==current,'Saved main inventory differs'
assert traffic()==current_traffic
# Verify persisted navigation after changing the map package, without rebuilding it.
unreal.PiedmontWorldTools.finish_editor_asset_loading()
nav=json.loads((root/'Tests/Results/2026-09-12-krog-outward-navigation.json').read_text())['after']
anchor=unreal.PiedmontWorldTools.project_park_navigation(unreal.Vector(*json.loads((root/'SourceAssets/Terrain/lake-test-points.json').read_text())['safe_start']))
assert anchor
checked=0
for row in nav['samples']:
 if not row['reachable']:continue
 point=unreal.PiedmontWorldTools.project_park_navigation(unreal.Vector(*row['xyz']))
 assert point and unreal.PiedmontWorldTools.park_route_length(anchor,point)>=0,row['label']
 checked+=1
result={'passed':True,'main_map_changed':True,'backup':str(backup_file),'backup_sha256':sha(backup_file),'main_sha256':sha(map_file),'preserved_original_traffic_directors':len(original_traffic),'traffic_directors':len(current_traffic),'reachable_samples_verified':checked,'main_actor_count':sum(current.values()),'scope':'Candidate promoted and reloaded; inventory, original traffic lanes/settings, corrected assets and persisted navigation checked. Installed app not replaced.'}
(root/'Tests/Results/2026-09-12-krog-main-promotion.json').write_text(json.dumps(result,indent=2)+'\n')
