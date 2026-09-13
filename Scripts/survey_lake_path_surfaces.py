"""Read-only vertical surface stacks beside Lake Clara Meer; never saves map."""
import unreal,json,pathlib
root=pathlib.Path(unreal.Paths.project_dir())
assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/PiedmontWorld')
unreal.PiedmontWorldTools.finish_editor_asset_loading()
rows=[]
for x in range(-8000,1001,250):
 for y in range(-6500,1001,250):
  hits=[];top=1800
  for _ in range(8):
   h=unreal.PiedmontWorldTools.trace_world_surface(unreal.Vector(x,y,top),unreal.Vector(x,y,-1600))
   if not h:break
   a=h[1];hits.append({'actor':a.get_actor_label(),'class':a.get_class().get_name(),'z':h[0].z});top=h[0].z-.5
   if isinstance(a,unreal.Landscape):break
  rows.append({'xy':[x,y],'surfaces':hits})
(root/'work/lake-path-surface-survey.json').write_text(json.dumps({'scope':'Read-only actual surface stacks around southeast lake paths','samples':rows},indent=2))
