"""Connect a measured short grass gap with a bidirectional pedestrian nav link."""
import json
import math
from pathlib import Path
import unreal
root=Path(unreal.Paths.project_dir())
assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/PiedmontKrogWorldReview')
unreal.PiedmontWorldTools.finish_editor_asset_loading()
row=next(p for p in json.loads((root/'Tests/Results/2026-09-12-disconnected-park-native-survey.json').read_text())['paths'] if p['label']=='OSM path 442709517 part 144')
a=unreal.Vector(*row['start']);b=unreal.Vector(*row['end'])
ea=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
assert row['centerline_xy_gap_cm']<400
samples=[]
for i in range(21):
 t=i/20;x=a.x+(b.x-a.x)*t;y=a.y+(b.y-a.y)*t;z=a.z+(b.z-a.z)*t
 hit=unreal.PiedmontWorldTools.trace_world_surface(unreal.Vector(x,y,z+60),unreal.Vector(x,y,z-80))
 assert hit,'Missing ground'
 assert abs(hit[0].z-z)<15,'Unexpected terrain height'
 samples.append(hit[0])
for p,q in zip(samples,samples[1:]):
 assert abs(q.z-p.z)/math.hypot(q.x-p.x,q.y-p.y)<.3,'Steep grass crossing'
 for height in [40,90,145]:
  assert not unreal.PiedmontWorldTools.trace_world_surface(p+unreal.Vector(0,0,height),q+unreal.Vector(0,0,height),35),'Obstructed walking clearance'
pa=unreal.PiedmontWorldTools.project_park_navigation(a);pb=unreal.PiedmontWorldTools.project_park_navigation(b)
assert pa and pb
before=unreal.PiedmontWorldTools.park_route_length(pa,pb)
for actor in ea.get_all_level_actors():
 if actor.actor_has_tag('ParkGrassWalkingLink'):ea.destroy_actor(actor)
link=ea.spawn_actor_from_class(unreal.NavLinkProxy,pa)
link.set_actor_label('Park short grass walking connection')
link.tags=[unreal.Name('ParkGrassWalkingLink')]
point=unreal.NavigationLink()
point.set_editor_property('left',unreal.Vector())
point.set_editor_property('right',pb-pa)
point.set_editor_property('direction',unreal.NavLinkDirection.BOTH_WAYS)
link.set_editor_property('point_links',[point])
# Existing helper rebuilds all path bounds and checks Krog and previous reachable routes.
assert unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).save_current_level()
script=(root/'Scripts/rebuild_krog_world_navigation.py').read_text().replace('2026-09-12-krog-world-navigation.json','2026-09-12-park-grass-link-navigation.json')
exec(compile(script,'rebuild_park_grass_link_navigation','exec'), {'__name__': '__main__'})
pa=unreal.PiedmontWorldTools.project_park_navigation(a);pb=unreal.PiedmontWorldTools.project_park_navigation(b)
after=unreal.PiedmontWorldTools.park_route_length(pa,pb)
reverse=unreal.PiedmontWorldTools.park_route_length(pb,pa)
baseline=json.loads((root/'Tests/Results/2026-09-12-krog-world-navigation.json').read_text())['after']['samples']
current=json.loads((root/'Tests/Results/2026-09-12-park-grass-link-navigation.json').read_text())['after']['samples']
old={(s['label'],s['index']):s['reachable'] for s in baseline}
assert all(s['reachable'] for s in current if old.get((s['label'],s['index']),False))
gained=sum(s['reachable'] and not old.get((s['label'],s['index']),False) for s in current)
result={'newly_reachable_samples_vs_original':gained,'before_cm':before,'after_cm':after,'reverse_cm':reverse,'ground_samples':len(samples),'clearance_sweeps':60,'passed':after>=0 and reverse>=0,'scope':'Ground/clearance and bidirectional native navigation. Actual NPC traversal remains unverified. Main map unchanged.'}
(root/'Tests/Results/2026-09-12-park-grass-link.json').write_text(json.dumps(result,indent=2)+'\n')
assert result['passed'],result
