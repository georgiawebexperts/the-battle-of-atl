import unreal,pathlib,json,traceback
p=pathlib.Path(unreal.Paths.project_dir())
try:
    options=unreal.FbxImportUI()
    options.set_editor_property('import_as_skeletal',True)
    options.set_editor_property('import_mesh',True)
    options.set_editor_property('import_animations',False)
    options.set_editor_property('import_materials',True)
    options.set_editor_property('import_textures',True)
    options.set_editor_property('automated_import_should_detect_type',False)
    options.set_editor_property('mesh_type_to_import',unreal.FBXImportType.FBXIT_SKELETAL_MESH)
    task=unreal.AssetImportTask();task.filename=str(p/'SourceAssets/Rider/Casual.fbx');task.destination_path='/Game/PiedmontRide/Rider';task.destination_name='Casual';task.automated=True;task.save=True;task.replace_existing=True;task.options=options
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    result={'imported':list(task.imported_object_paths),'meshes':[]}
    ea=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    for path in task.imported_object_paths:
        mesh=unreal.load_asset(path)
        if isinstance(mesh,unreal.SkeletalMesh):
            a=ea.spawn_actor_from_class(unreal.SkeletalMeshActor,unreal.Vector(0,0,500))
            c=a.skeletal_mesh_component;c.set_skeletal_mesh_asset(mesh)
            bones=[{'name':str(c.get_bone_name(i)),'position':str(c.get_bone_location(c.get_bone_name(i),unreal.BoneSpaces.COMPONENT_SPACE))} for i in range(c.get_num_bones())]
            result['meshes'].append({'path':path,'bones':bones,'bounds':str(c.get_local_bounds())})
            ea.destroy_actor(a)
    (p/'Scripts/v2-rider-import.json').write_text(json.dumps(result,indent=2))
except Exception:
    (p/'Scripts/v2-rider-import-error.txt').write_text(traceback.format_exc())
    raise
