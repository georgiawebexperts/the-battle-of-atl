import unreal,json,pathlib
p=pathlib.Path(unreal.Paths.project_dir());ea=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
world=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
r={'world':world.get_name(),'landscapes':[],'trace_doc':unreal.SystemLibrary.line_trace_single.__doc__}
for a in ea.get_all_level_actors():
 if isinstance(a,unreal.Landscape):r['landscapes'].append({'bounds':str(a.get_actor_bounds(False)),'scale':str(a.get_actor_scale3d()),'location':str(a.get_actor_location()),'components':len(a.get_components_by_class(unreal.LandscapeComponent))})
unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).set_level_viewport_camera_info(unreal.Vector(-12000,-25000,45000),unreal.Rotator(pitch=-55,yaw=75,roll=0))
(p/'Scripts/measured-landscape-inspection.json').write_text(json.dumps(r,indent=2))
