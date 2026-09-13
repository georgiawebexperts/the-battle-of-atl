"""Import and reduce the articulated shotgun; bind existing credited Mac materials."""
import unreal,json,pathlib
root=pathlib.Path(unreal.Paths.project_dir());dest='/Game/BattleForTheA/Weapons/ShotgunParts';t=unreal.AssetImportTask();t.filename=str(root/'SourceAssets/Weapons/Remington870/ShotgunParts.gltf');t.destination_path=dest;t.automated=True;t.save=True;t.replace_existing=True
unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([t]);unreal.PiedmontWorldTools.finish_editor_asset_loading();sub=unreal.get_editor_subsystem(unreal.StaticMeshEditorSubsystem) or unreal.new_object(unreal.StaticMeshEditorSubsystem);rows=[]
for path in unreal.EditorAssetLibrary.list_assets(dest,recursive=True,include_folder=False):
 mesh=unreal.load_asset(path)
 if not isinstance(mesh,unreal.StaticMesh):continue
 nanite=sub.get_nanite_settings(mesh);nanite.enabled=False;sub.set_nanite_settings(mesh,nanite,True);unreal.PiedmontWorldTools.finish_editor_asset_loading();before=mesh.get_num_triangles(0);settings=sub.get_lod_reduction_settings(mesh,0);settings.percent_triangles=.15;sub.set_lod_reduction_settings(mesh,0,settings)
 for i,slot in enumerate(mesh.static_materials):
  name='M_Remington_Polymer' if 'plastic' in str(slot.material_slot_name).lower() else 'M_Remington_Steel';mat=unreal.load_asset('/Game/BattleForTheA/Weapons/Remington870/'+name);assert mat;mesh.set_material(i,mat)
 unreal.PiedmontWorldTools.finish_editor_asset_loading();unreal.EditorAssetLibrary.save_loaded_asset(mesh);b=mesh.get_bounding_box();rows.append({'path':path,'triangles_before':before,'triangles_after':mesh.get_num_triangles(0),'min':str(b.min),'max':str(b.max)})
assert len(rows)==2
(root/'work/shotgun-parts-import.json').write_text(json.dumps(rows,indent=2)+'\n');unreal.SystemLibrary.quit_editor()
