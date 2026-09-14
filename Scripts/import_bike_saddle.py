"""Import the authored saddle without changing any map."""
import unreal,json,pathlib
root=pathlib.Path(unreal.Paths.project_dir()).resolve();folder=root/'SourceAssets/Bike'
opts=unreal.FbxImportUI();opts.import_as_skeletal=False;opts.import_materials=False;opts.import_textures=False;opts.automated_import_should_detect_type=False;opts.mesh_type_to_import=unreal.FBXImportType.FBXIT_STATIC_MESH
opts.static_mesh_import_data.combine_meshes=True;opts.static_mesh_import_data.auto_generate_collision=False;opts.static_mesh_import_data.normal_import_method=unreal.FBXNormalImportMethod.FBXNIM_IMPORT_NORMALS
job=unreal.AssetImportTask();job.filename=str(folder/'BikeSaddle.obj');job.destination_path='/Game/PiedmontRide/Bike';job.destination_name='SM_BikeSaddle';job.automated=True;job.replace_existing=True;job.save=True;job.options=opts
unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([job]);mesh=unreal.load_asset(job.destination_path+'/SM_BikeSaddle');assert mesh
mesh.set_material(0,unreal.load_asset('/Game/BeltLineGlide/Materials/M_Rubber'));unreal.EditorAssetLibrary.save_loaded_asset(mesh)
b=mesh.get_bounding_box();actual=[[b.min.x,b.min.y,b.min.z],[b.max.x,b.max.y,b.max.z]];expected=json.loads((folder/'BikeSaddle.json').read_text())['bounds_cm'];error=max(abs(actual[i][j]-expected[i][j]) for i in range(2) for j in range(3));assert error<.01
(root/'Tests/Results/2026-09-13-saddle-import.json').write_text(json.dumps({'passed':True,'bounds_cm':actual,'bounds_error_cm':error,'triangles':mesh.get_num_triangles(0),'map_saved':False},indent=2)+'\n')
