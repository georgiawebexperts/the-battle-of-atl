"""Authored construction closures at the two unfinished DeKalb road ends."""
from pathlib import Path
from collections import Counter
import json,math
import unreal
root=Path(unreal.Paths.project_dir()).resolve()
assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/PiedmontWorld')
unreal.PiedmontWorldTools.finish_editor_asset_loading();ea=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
roads=[r for r in json.loads((root/'SourceAssets/Terrain/KrogTraffic/network.json').read_text())['roads'] if r['tags'].get('name')=='DeKalb Avenue Northeast']
counts=Counter(tuple(p) for r in roads for p in [r['points_cm'][0],r['points_cm'][-1]])
ends=[]
for r in roads:
 p=r['points_cm']
 for a,b in [(p[0],p[1]),(p[-1],p[-2])]:
  if counts[tuple(a)]==1:ends.append((unreal.Vector(*a),(unreal.Vector(*a)-unreal.Vector(*b)).normal()))
assert len(ends)==2
cube=unreal.load_asset('/Engine/BasicShapes/Cube')
materials={k:unreal.load_asset(path) for k,path in {'dark':'/Game/BattleForTheA/Furniture/M_BenchFrame','red':'/Game/BattleForTheA/Materials/M_ColaRed','white':'/Game/PiedmontRide/Materials/M_Concrete'}.items()}
dest='/Game/BattleForTheA/Environment/KrogConstruction';material=unreal.load_asset(dest+'/M_ConstructionOrange')
if not material:
 material=unreal.AssetToolsHelpers.get_asset_tools().create_asset('M_ConstructionOrange',dest,unreal.Material,unreal.MaterialFactoryNew())
 lib=unreal.MaterialEditingLibrary;color=lib.create_material_expression(material,unreal.MaterialExpressionConstant3Vector)
 color.set_editor_property('constant',unreal.LinearColor(.95,.23,.015));lib.connect_material_property(color,'',unreal.MaterialProperty.MP_BASE_COLOR)
 lib.recompile_material(material);unreal.EditorAssetLibrary.save_loaded_asset(material)
materials['orange']=material
assert cube and all(materials.values())
for actor in ea.get_all_level_actors():
 if actor.actor_has_tag('KrogConstruction'):ea.destroy_actor(actor)
def ground(p):
 hit=unreal.PiedmontWorldTools.trace_world_surface(p+unreal.Vector(0,0,500),p-unreal.Vector(0,0,800))
 assert hit,p
 return hit[0]
def part(label,p,size,yaw,material):
 actor=ea.spawn_actor_from_class(unreal.StaticMeshActor,p,unreal.Rotator(pitch=0,yaw=yaw,roll=0));actor.set_actor_label(label)
 actor.set_actor_scale3d(size/100);actor.tags=[unreal.Name('KrogConstruction'),unreal.Name('RideBarrier')]
 actor.static_mesh_component.set_static_mesh(cube);actor.static_mesh_component.set_material(0,materials[material]);actor.static_mesh_component.set_collision_profile_name('BlockAll')
 return actor
closures=[]
for index,(end,d) in enumerate(ends):
 d.z=0;d=d.normal();side=unreal.Vector(-d.y,d.x,0);center=ground(end+d*200);yaw=math.degrees(math.atan2(d.y,d.x))
 # Dense visible uprights block a human capsule; feet follow the existing slope.
 feet={}
 for offset in range(-1400,1401,40):
  foot=ground(center+side*offset);feet[offset]=[foot.x,foot.y,foot.z]
  part(f'DeKalb closure {index} upright {offset}',foot+unreal.Vector(0,0,180),unreal.Vector(12,10,360),yaw,'dark')
 for z in [70,300]:part(f'DeKalb closure {index} rail {z}',center+unreal.Vector(0,0,z),unreal.Vector(12,2810,10),yaw,'dark')
 part(f'DeKalb closure {index} sign',center-d*20+unreal.Vector(0,0,210),unreal.Vector(25,1000,175),yaw,'orange')
 for offset in range(-450,451,100):
  part(f'DeKalb closure {index} reflective stripe {offset}',center-d*34+side*offset+unreal.Vector(0,0,137),unreal.Vector(3,50,25),yaw,'white')
 for text,z,size in [('UNDER CONSTRUCTION',235,38),('MORE ATL COMING SOON',182,30)]:
  actor=ea.spawn_actor_from_class(unreal.TextRenderActor,center-d*34+unreal.Vector(0,0,z),unreal.Rotator(pitch=0,yaw=yaw+180,roll=0))
  actor.tags=[unreal.Name('KrogConstruction')];actor.set_actor_label(f'DeKalb closure {index} {text}')
  component=actor.get_component_by_class(unreal.TextRenderComponent);component.set_text(text);component.set_world_size(size);component.set_horizontal_alignment(unreal.HorizTextAligment.EHTA_CENTER)
  component.set_text_render_color(unreal.Color(255,245,220,255));component.set_text_material(unreal.load_asset('/Engine/EngineMaterials/UnlitText'))
 closures.append({'center':[center.x,center.y,center.z],'outward':[d.x,d.y,d.z],'feet':feet})
unreal.PiedmontWorldTools.finish_editor_asset_loading()
blocked=0
for row in closures:
 c=unreal.Vector(*row['center']);d=unreal.Vector(*row['outward']);side=unreal.Vector(-d.y,d.x,0)
 for offset in range(-1320,1321,80):
  p=unreal.Vector(*row['feet'][offset])+unreal.Vector(0,0,85)-d*100
  hit=unreal.PiedmontWorldTools.trace_world_surface(p,p+d*200,30)
  assert hit and hit[1].actor_has_tag('KrogConstruction'),(row,offset,[p.x,p.y,p.z],hit)
  blocked+=1
routes=json.loads((root/'SourceAssets/Terrain/KrogTraffic/car-lanes.json').read_text())['routes']
minimum=1e9
for route in routes:
 for p in route['points_cm']:
  for row in closures:
   d=row['outward'];c=row['center'];minimum=min(minimum,abs(sum((p[i]-c[i])*d[i] for i in [0,1])))
assert minimum>300,minimum
assert unreal.EditorLoadingAndSavingUtils.save_map(unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world(),'/Game/PiedmontRide/Maps/PiedmontKrogBoundaryReview')
(root/'Tests/Results/2026-09-12-krog-construction-placement.json').write_text(json.dumps({'closures':closures,'blocked_sweeps':blocked,'minimum_car_centerline_distance_to_fence_cm':minimum,'main_map_changed':False,'scope':'Two local road-end closures; not a complete world perimeter. Native obstruction sweeps and car-route separation checked; rendered sign review and gameplay traversal pending.'},indent=2)+'\n')
