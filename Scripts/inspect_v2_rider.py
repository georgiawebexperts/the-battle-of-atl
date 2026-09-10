import unreal,json,pathlib
out=[];ea=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
for a in ea.get_all_level_actors():
 if isinstance(a,unreal.SkeletalMeshActor):
  c=a.skeletal_mesh_component
  bones=[{'name':str(c.get_bone_name(i)),'position':str(c.get_socket_location(c.get_bone_name(i))-a.get_actor_location())} for i in range(c.get_num_bones())]
  out.append({'mesh':str(c.get_skeletal_mesh_asset()),'bones':bones})
  ea.destroy_actor(a)
pathlib.Path(unreal.Paths.project_dir(),'Scripts','v2-rider-import.json').write_text(json.dumps(out,indent=2))
