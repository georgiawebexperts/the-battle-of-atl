"""Align combined-candidate automatic lighting bounds with the current tunnel shell."""
import json,math
from pathlib import Path
import unreal
root=Path(unreal.Paths.project_dir())
assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/PiedmontKrogWorldReview')
ea=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
manifest=json.loads((root/'SourceAssets/Terrain/KrogContinuousShell/manifest.json').read_text())
def vector(p):return unreal.Vector(p[0],-p[1],p[2])
old=[a for a in ea.get_all_level_actors() if isinstance(a,unreal.PiedmontDarkZone) and a.get_actor_label().startswith('Krog darkness ')]
def contains(actor,p):
 local=unreal.MathLibrary.inverse_transform_location(actor.get_actor_transform(),p)
 extent=actor.get_component_by_class(unreal.BoxComponent).get_unscaled_box_extent()
 return all(abs(getattr(local,k))<=getattr(extent,k) for k in ['x','y','z'])
first=manifest['dark_zones'][0];a=vector(first['a']);b=vector(first['b']);direction=(b-a).normal()
# This point was inside the old bounds but now lies in the uncovered approach.
approach=a-direction*250
before=any(contains(z,approach) for z in old)
for actor in old:ea.destroy_actor(actor)
zones=[]
for i,row in enumerate(manifest['dark_zones']):
 a=vector(row['a']);b=vector(row['b']);d=b-a
 actor=ea.spawn_actor_from_class(unreal.PiedmontDarkZone,(a+b)*.5)
 actor.set_actor_rotation(unreal.Rotator(pitch=0,yaw=math.degrees(math.atan2(d.y,d.x)),roll=0),False)
 actor.set_actor_label('Krog darkness '+str(i));actor.set_folder_path('BattleForTheA/BeltLine')
 actor.get_component_by_class(unreal.BoxComponent).set_box_extent(unreal.Vector(d.length()*.5+2,row['half_width'],row['half_height']))
 zones.append(actor)
inside=[vector(p)+unreal.Vector(0,0,100) for p in manifest['roof_samples']]
assert all(any(contains(z,p) for z in zones) for p in inside),'Roof-covered route lacks light zone'
assert not any(contains(z,approach) for z in zones),'Uncovered approach remains dark'
assert unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).save_current_level()
(root/'Tests/Results/2026-09-12-krog-dark-zone-alignment.json').write_text(json.dumps({
 'passed':True,'old_zone_count':len(old),'new_zone_count':len(zones),'covered_route_samples':len(inside),
 'approach_inside_before':before,'approach_inside_after':False,'portal_setback_cm':manifest['portal_setback_cm'],
 'scope':'Bounds match current shell manifest; native riding light check remains separate. Main map unchanged.'},indent=2)+'\n')
