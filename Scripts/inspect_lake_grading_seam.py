"""Read-only ray and mesh inventory for the visible candidate pavement seam."""
import unreal,json,pathlib,math
root=pathlib.Path(unreal.Paths.project_dir());assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/PiedmontLakePathGradingReview');unreal.PiedmontWorldTools.finish_editor_asset_loading();ea=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
meshes=[];routes=[]
for a in ea.get_all_level_actors():
 if isinstance(a,unreal.StaticMeshActor):
  m=a.static_mesh_component.static_mesh
  if m and '/LakePathGrading/' in m.get_path_name():meshes.append({'actor':a.get_actor_label(),'mesh':m.get_path_name(),'triangles':m.get_num_triangles(0),'scale':str(a.get_actor_scale3d())})
 if isinstance(a,unreal.PiedmontPathSpline) and str(a.get_editor_property('osm_way_id')) in ['61853018','182460439']:
  routes.append({'label':a.get_actor_label(),'id':str(a.get_editor_property('osm_way_id')),'points':a.centerline.get_number_of_spline_points(),'length':a.centerline.get_spline_length()})
eye=unreal.Vector(-4450,-2480,-100);target=unreal.Vector(-5100,-1915,-320);rot=unreal.MathLibrary.find_look_at_rotation(eye,target);f=unreal.MathLibrary.get_forward_vector(rot);r=unreal.MathLibrary.get_right_vector(rot);u=unreal.MathLibrary.get_up_vector(rot);hits=[]
for x,y in [(980,451),(980,456),(980,460),(1100,480),(1100,488)]:
 d=f+r*((2*x/1280-1)*math.tan(math.radians(37.5)))+u*((1-2*y/720)*math.tan(math.radians(37.5))*720/1280)
 h=unreal.PiedmontWorldTools.trace_world_surface(eye,eye+d*10000)
 row={'pixel':[x,y]}
 if h:
  p=h[0];row.update(ray_actor=h[1].get_actor_label(),point=[p.x,p.y,p.z]);top=1800;stack=[]
  for _ in range(8):
   v=unreal.PiedmontWorldTools.trace_world_surface(unreal.Vector(p.x,p.y,top),unreal.Vector(p.x,p.y,-1600))
   if not v:break
   stack.append({'actor':v[1].get_actor_label(),'z':v[0].z});top=v[0].z-.5
   if isinstance(v[1],unreal.Landscape):break
  row['stack']=stack
 hits.append(row)
(root/'work/lake-grading-seam.json').write_text(json.dumps({'meshes':meshes,'routes':routes,'rays':hits},indent=2)+'\n')
