"""Import the downloaded Mixamo reference rig and authored one-handed low_reach clip."""
import unreal,json,pathlib
root=pathlib.Path(unreal.Paths.project_dir());source=root/'SourceAssets/Mixamo/PickingUpObjectLow_WithSkin.fbx';dest='/Game/BattleRetarget/Mixamo/LowReachReference';assert source.is_file()
opts=unreal.FbxImportUI();opts.import_as_skeletal=True;opts.import_mesh=True;opts.import_animations=True;opts.import_materials=True;opts.import_textures=True;opts.automated_import_should_detect_type=False;opts.mesh_type_to_import=unreal.FBXImportType.FBXIT_SKELETAL_MESH;opts.create_physics_asset=False
job=unreal.AssetImportTask();job.filename=str(source);job.destination_path=dest;job.destination_name='MixamoLowReachReference';job.automated=True;job.save=True;job.replace_existing=True;job.options=opts
unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([job]);rows=[]
for path in job.imported_object_paths:
 asset=unreal.load_asset(path)
 if asset:rows.append({'path':path,'type':asset.get_class().get_name()})
assert any(r['type']=='SkeletalMesh' for r in rows) and any(r['type']=='AnimSequence' for r in rows)
unreal.PiedmontWorldTools.finish_editor_asset_loading()
(root/'Tests/Results/2026-09-12-mixamo-low_reach-import.json').write_text(json.dumps({'imported':rows,'retargeted':False,'gameplay_installed':False},indent=2)+'\n')
