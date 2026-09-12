"""Read-only transient stack survey around the 14th gate path crossing."""
import unreal,json,pathlib
root=pathlib.Path(unreal.Paths.project_dir())
assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/PiedmontWorld')
unreal.PiedmontWorldTools.finish_editor_asset_loading()
rows=[]
for x in range(-18400,-16200,200):
 for y in range(-5600,-4000,200):
  hits=[];top=1500
  for _ in range(8):
   h=unreal.PiedmontWorldTools.trace_world_surface(unreal.Vector(x,y,top),unreal.Vector(x,y,-1500))
   if not h:break
   a=h[1];hits.append({'actor':a.get_actor_label(),'class':a.get_class().get_name(),'z':h[0].z});top=h[0].z-.5
   if a.get_class().get_name()=='Landscape':break
  rows.append({'xy':[x,y],'surfaces':hits})
visuals=[]
for a in unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors():
 for c in a.get_components_by_class(unreal.StaticMeshComponent):
  origin,extent,radius=unreal.SystemLibrary.get_component_bounds(c)
  if origin.x+extent.x < -18400 or origin.x-extent.x > -16200 or origin.y+extent.y < -5600 or origin.y-extent.y > -4000:continue
  m=c.get_editor_property('static_mesh')
  visuals.append({'actor':a.get_actor_label(),'component':c.get_name(),'class':c.get_class().get_name(),'mesh':m.get_path_name() if m else None,'visible':c.get_editor_property('visible'),'hidden_in_game':c.get_editor_property('hidden_in_game'),'collision':str(c.get_collision_enabled()),'origin':[origin.x,origin.y,origin.z],'extent':[extent.x,extent.y,extent.z]})
out=root/'Tests/Results/2026-09-12-market-path-seams.json';out.write_text(json.dumps({'scope':'Transient vertical surface stack; no map saved','samples':rows,'visual_components':visuals},indent=2))
