import unreal,pathlib,json
p=pathlib.Path(unreal.Paths.project_dir())
o=unreal.FbxImportUI();o.import_as_skeletal=False;o.import_materials=True;o.import_textures=False;o.automated_import_should_detect_type=False;o.mesh_type_to_import=unreal.FBXImportType.FBXIT_STATIC_MESH;o.static_mesh_import_data.combine_meshes=True
job=unreal.AssetImportTask();job.filename=str(p/'SourceAssets/Bike/BikeWheel.obj');job.destination_path='/Game/PiedmontRide/Bike';job.destination_name='SM_BikeWheel';job.automated=True;job.save=True;job.replace_existing=True;job.options=o
unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([job])
(p/'Scripts/v2-wheel-import.json').write_text(json.dumps(list(job.imported_object_paths),indent=2))
