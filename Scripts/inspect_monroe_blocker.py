import unreal,json,pathlib
root=pathlib.Path(unreal.Paths.project_dir());assert unreal.EditorLoadingAndSavingUtils.load_map('/Game/PiedmontRide/Maps/PiedmontMonroeExtendedReview');ea=unreal.get_editor_subsystem(unreal.EditorActorSubsystem);rows=[]
for a in ea.get_all_level_actors():
 if a.get_name()=='StaticMeshActor_132':
  loc=a.get_actor_location();rows.append({'name':a.get_name(),'label':a.get_actor_label(),'location':[loc.x,loc.y,loc.z],'mesh':a.static_mesh_component.static_mesh.get_path_name()})
(root/'Tests/Results/2026-09-12-monroe-blocker.json').write_text(json.dumps(rows,indent=2)+'\n')
