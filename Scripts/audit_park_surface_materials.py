"""Identify visible and collidable park surfaces around the sleeper without changing assets."""
import unreal,json,pathlib
root=pathlib.Path(unreal.Paths.project_dir());assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/PiedmontWorld')
unreal.PiedmontWorldTools.finish_editor_asset_loading()
site=json.loads((root/'Tests/Results/2026-09-12-sleeper-sites.json').read_text())['candidates'][0]['xyz'];rows={}
for dx in range(-1200,1201,200):
 for dy in range(-1200,1201,200):
  p=unreal.Vector(site[0]+dx,site[1]+dy,site[2]);hit=unreal.PiedmontWorldTools.trace_world_surface(p+unreal.Vector(0,0,2000),p-unreal.Vector(0,0,2000))
  if not hit:continue
  point,a=hit;key=a.get_name()
  if key not in rows:
   c=a.get_component_by_class(unreal.StaticMeshComponent)
   rows[key]={'name':key,'label':a.get_actor_label(),'class':a.get_class().get_name(),'mesh':c.static_mesh.get_path_name() if c and c.static_mesh else None,'materials':[m.get_path_name() if m else None for m in c.get_materials()] if c else [],'samples':[]}
  rows[key]['samples'].append([point.x,point.y,point.z])
(root/'Tests/Results/2026-09-12-park-surface-materials.json').write_text(json.dumps({'site':site,'actors':list(rows.values()),'map_saved':False},indent=2)+'\n')
print([(r['label'],r['materials'],len(r['samples'])) for r in rows.values()])
