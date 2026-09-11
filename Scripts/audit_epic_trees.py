import unreal,json,pathlib
root=pathlib.Path(unreal.Paths.project_dir())
manifest=json.loads((root/'work/epic-tree-migration.json').read_text())
rows=[]
for path in manifest['roots']:
 mesh=unreal.load_asset(path);assert isinstance(mesh,unreal.StaticMesh),path
 materials=mesh.get_editor_property('static_materials')
 assert materials and all(m.material_interface for m in materials),path
 bounds=mesh.get_bounds();assert bounds.box_extent.z>10,path
 rows.append({'path':path,'height_cm':bounds.box_extent.z*2,'material_slots':[str(m.material_interface.get_path_name()) for m in materials]})
(root/'work/epic-tree-load-audit.json').write_text(json.dumps({'loaded':rows,'status':'loaded in game project; visual and performance review still required'},indent=2)+'\n')
print('BATTLE_TREE_LOAD_AUDIT_COMPLETE')
