"""Reduce the high-density shotgun candidate while retaining original source data."""
import unreal,pathlib,json
root=pathlib.Path(unreal.Paths.project_dir());row=json.loads((root/'work/remington870-import.json').read_text())[0];mesh=unreal.load_asset(row['path']);unreal.PiedmontWorldTools.finish_editor_asset_loading()
sub=unreal.get_editor_subsystem(unreal.StaticMeshEditorSubsystem) or unreal.new_object(unreal.StaticMeshEditorSubsystem);nanite=sub.get_nanite_settings(mesh);nanite.enabled=False;sub.set_nanite_settings(mesh,nanite,True);unreal.PiedmontWorldTools.finish_editor_asset_loading();settings=sub.get_lod_reduction_settings(mesh,0);settings.percent_triangles=1.;sub.set_lod_reduction_settings(mesh,0,settings);unreal.PiedmontWorldTools.finish_editor_asset_loading();before=mesh.get_num_triangles(0);settings.percent_triangles=.15
sub.set_lod_reduction_settings(mesh,0,settings);unreal.PiedmontWorldTools.finish_editor_asset_loading();after=mesh.get_num_triangles(0);print('REDUCTION',before,after);assert 0<after<before
unreal.EditorAssetLibrary.save_loaded_asset(mesh)
(root/'work/remington870-optimization.json').write_text(json.dumps({'before_triangles':before,'after_triangles':after,'source_preserved':True,'percent_requested':.15},indent=2)+'\n');unreal.SystemLibrary.quit_editor()
