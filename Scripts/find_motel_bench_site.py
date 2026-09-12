"""Survey native collision beside the Motel; do not alter the map."""
import unreal,json,pathlib
root=pathlib.Path(unreal.Paths.project_dir())
assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/PiedmontWorld')
unreal.PiedmontWorldTools.finish_editor_asset_loading()
world=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
rows=[]
def ground(x,y):return unreal.PiedmontWorldTools.trace_world_surface(unreal.Vector(x,y,5000),unreal.Vector(x,y,-5000))
for x in [-17500,-17400,-17300]:
 for y in [11450,11550,11650,11750]:
  hit=ground(x,y)
  if not hit:continue
  p,actor=hit;support=[];clear=True
  # Bench faces east: long seat north/south; back to west. Include standing point.
  for dx,dy in [(-35,-90),(-35,90),(25,-90),(25,90),(55,0)]:
   h=ground(x+dx,y+dy)
   if not h:clear=False;break
   support.append({'xyz':[h[0].x,h[0].y,h[0].z],'surface':h[1].get_actor_label()})
  span=max([s['xyz'][2] for s in support]+[p.z])-min([s['xyz'][2] for s in support]+[p.z])
  retreat=[ground(x+dx,y) for dx in [80,160,240,320]]
  block=unreal.SystemLibrary.capsule_trace_single(world,p+unreal.Vector(55,0,100),p+unreal.Vector(320,0,100),32,85,unreal.TraceTypeQuery.ECC_VISIBILITY,False,[],unreal.DrawDebugTrace.NONE)
  rows.append({'xyz':[p.x,p.y,p.z],'yaw':-90,'surface':actor.get_actor_label(),'support_height_span_cm':span,'support':support,'retreat_surfaces':[h[1].get_actor_label() if h else None for h in retreat],'retreat_blocked':bool(block),'candidate':clear and span<=5 and not block and all(h and abs(h[0].z-p.z)<25 for h in retreat)})
(root/'Tests/Results/2026-09-12-motel-bench-sites.json').write_text(json.dumps({'candidates':rows,'map_saved':False,'scope':'Collision survey only; no authored bench, runtime actors or final clearance acceptance'},indent=2)+'\n')
print('MOTEL_BENCH_CANDIDATES',sum(r['candidate'] for r in rows))
