"""Measure installed market leg/support contact without saving assets."""
import unreal,json,pathlib,math
root=pathlib.Path(unreal.Paths.project_dir())
assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/PiedmontWorld')
unreal.PiedmontWorldTools.finish_editor_asset_loading();ea=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
market=[a for a in ea.get_all_level_actors() if unreal.Name('TwelfthStreetMarket') in a.tags]
for a in market:a.set_actor_enable_collision(False)
metal=[a for a in market if a.static_mesh_component.static_mesh.get_name()=='SM_MarketStall_Metal']
wood=next(a for a in market if a.static_mesh_component.static_mesh.get_name()=='SM_MarketStall_Wood').static_mesh_component.static_mesh.get_bounding_box();sign=1 if wood.min.y+wood.max.y>0 else -1
supports=[a for a in market if 'adjustable foot' in a.get_actor_label()];rows=[]
for a in metal:
 for i,(x,y) in enumerate([(x,y) for x in [-150,150] for y in [-150,150]]+[(x,y*sign) for x in [-90,90] for y in [40,80]]):
  p=unreal.MathLibrary.transform_location(a.get_actor_transform(),unreal.Vector(x,y,0))
  nearby=[b for b in supports if math.hypot(b.get_actor_location().x-p.x,b.get_actor_location().y-p.y)<.25]
  bottom=p.z
  if nearby:
   assert len(nearby)==1
   origin,extent=nearby[0].get_actor_bounds(False);bottom=origin.z-extent.z
  hit=unreal.PiedmontWorldTools.trace_world_surface(unreal.Vector(p.x,p.y,1500),unreal.Vector(p.x,p.y,-1500));assert hit
  rows.append({'stall':a.get_actor_label(),'foot':i,'xyz':[p.x,p.y,p.z],'support':nearby[0].get_actor_label() if nearby else None,'bottom_gap_cm':bottom-hit[0].z,'ground':hit[1].get_actor_label()})
r={'passed':len(rows)==80 and all(abs(r['bottom_gap_cm'])<1 for r in rows),'count':len(rows),'map_saved':False,'max_absolute_gap_cm':max(abs(r['bottom_gap_cm']) for r in rows),'feet':rows}
(root/'Tests/Results/2026-09-14-market-feet.json').write_text(json.dumps(r,indent=2)+'\n');print('MARKET_FEET '+str({k:v for k,v in r.items() if k!='feet'}))
