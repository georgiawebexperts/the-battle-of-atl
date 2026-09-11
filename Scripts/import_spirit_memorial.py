"""Import original flower geometry. Does not edit or save the world map."""
import unreal,json
from pathlib import Path
root=Path(unreal.Paths.project_dir());dest='/Game/BattleForTheA/Spirit';rows=[]
colors={'Stems':(.045,.12,.035),'Petals':(.84,.81,.73),'Lavender':(.38,.25,.49),'Centers':(.49,.30,.045),'Stone':(.16,.17,.16),'Ribbon':(.22,.26,.27)}
for name,color in colors.items():
 mat=unreal.load_asset(dest+'/M_Memorial'+name)
 if not mat:mat=unreal.AssetToolsHelpers.get_asset_tools().create_asset('M_Memorial'+name,dest,unreal.Material,unreal.MaterialFactoryNew())
 unreal.MaterialEditingLibrary.delete_all_material_expressions(mat)
 c=unreal.MaterialEditingLibrary.create_material_expression(mat,unreal.MaterialExpressionConstant3Vector);c.set_editor_property('constant',unreal.LinearColor(*color));unreal.MaterialEditingLibrary.connect_material_property(c,'',unreal.MaterialProperty.MP_BASE_COLOR)
 r=unreal.MaterialEditingLibrary.create_material_expression(mat,unreal.MaterialExpressionConstant);r.set_editor_property('r',.83);unreal.MaterialEditingLibrary.connect_material_property(r,'',unreal.MaterialProperty.MP_ROUGHNESS)
 mat.set_editor_property('two_sided',True);unreal.MaterialEditingLibrary.recompile_material(mat);unreal.EditorAssetLibrary.save_loaded_asset(mat,only_if_is_dirty=False)
 o=unreal.FbxImportUI();o.import_as_skeletal=False;o.import_materials=False;o.import_textures=False;o.automated_import_should_detect_type=False;o.mesh_type_to_import=unreal.FBXImportType.FBXIT_STATIC_MESH;o.static_mesh_import_data.combine_meshes=True;o.static_mesh_import_data.auto_generate_collision=False
 t=unreal.AssetImportTask();t.filename=str(root/'SourceAssets/Spirit'/(name+'.obj'));t.destination_path=dest;t.destination_name='SM_Memorial'+name;t.automated=True;t.save=True;t.replace_existing=True;t.options=o
 unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([t]);mesh=unreal.load_asset(dest+'/SM_Memorial'+name);assert mesh
 mesh.set_material(0,mat);unreal.EditorAssetLibrary.save_loaded_asset(mesh,only_if_is_dirty=False)
 b=mesh.get_bounding_box();rows.append({'asset':mesh.get_path_name(),'min':str(b.min),'max':str(b.max)})
(root/'work/spirit-memorial-import.json').write_text(json.dumps(rows,indent=2)+'\n');unreal.SystemLibrary.quit_editor()
