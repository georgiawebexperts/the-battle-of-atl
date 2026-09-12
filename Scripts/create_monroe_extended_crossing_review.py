"""Bind both Monroe lanes to a shared crossing area in an isolated test map."""
import unreal,json,pathlib,math
root=pathlib.Path(unreal.Paths.project_dir());folder=root/'SourceAssets/Terrain/MonroeTraffic'
assert json.loads((root/'Tests/Results/2026-09-12-native-monroe-extended-lanes.json').read_text())['passed']
assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/PiedmontMonroeExtendedReview')
unreal.PiedmontWorldTools.finish_editor_asset_loading();ea=unreal.get_editor_subsystem(unreal.EditorActorSubsystem);world=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
lo,hi=json.loads((folder/'crossings.json').read_text())['control_bounds_xy'];center=[(a+b)/2 for a,b in zip(lo,hi)];ground=unreal.PiedmontWorldTools.trace_world_surface(unreal.Vector(*center,1000),unreal.Vector(*center,-1000));assert ground
Gate=ea.spawn_actor_from_class(unreal.BattleRoadCrossing,unreal.Vector(*center,ground[0].z+90));Gate.set_actor_label('Monroe shared BeltLine crossing control');Gate.tags=[unreal.Name('MonroeCrossingReview')];Gate.get_editor_property('CrossingArea').set_box_extent(unreal.Vector((hi[0]-lo[0])/2,(hi[1]-lo[1])/2,200));Gate.set_editor_property('bAutoCycle',True);Gate.set_editor_property('GreenSeconds',2);Gate.set_editor_property('AmberSeconds',1);Gate.set_editor_property('RedSeconds',6)
bindings=[]
for car in ea.get_all_level_actors():
 if not car.actor_has_tag('MonroeCarLaneReview'):continue
 points=list(car.get_editor_property('route'));distance=0;stop=None
 for i,p in enumerate(points):
  if i:distance+=math.hypot(p.x-points[i-1].x,p.y-points[i-1].y)
  q=points[min(i+1,len(points)-1)] if i<len(points)-1 else points[i-1];dx,dy=q.x-p.x,q.y-p.y;length=math.hypot(dx,dy);dx/=length;dy/=length
  hx,hy=abs(dx)*236+abs(dy)*114,abs(dy)*236+abs(dx)*114
  if p.x+hx>=lo[0] and p.x-hx<=hi[0] and p.y+hy>=lo[1] and p.y-hy<=hi[1]:stop=distance-100;break
 assert stop is not None and stop>0,(car.get_actor_label(),stop)
 binding=unreal.BattleCarCrossing();binding.set_editor_property('crossing',Gate);binding.set_editor_property('stop_distance',stop);car.set_editor_property('crossings',[binding]);bindings.append({'car':car.get_actor_label(),'stop_distance_cm':stop})
assert len(bindings)==2
assert unreal.EditorLoadingAndSavingUtils.save_map(world,'/Game/PiedmontRide/Maps/PiedmontMonroeExtendedCrossingReview')
(root/'Tests/Results/2026-09-12-monroe-extended-crossing-placement.json').write_text(json.dumps({'bounds_xy':[lo,hi],'bindings':bindings,'main_map_changed':False,'signals_visible':False,'scope':'Accelerated 2/1/6 test cycle, shared exclusive reservation. Live traffic timing and visible poles pending.'},indent=2)+'\n')
