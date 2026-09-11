"""Import geometry and rigs from the creator's mirrored CC0 glTF characters."""
import unreal,json,pathlib
root=pathlib.Path(unreal.Paths.project_dir());rows=[]
for name in ['Farmer','Punk']:
 data=json.loads((root/'SourceAssets/Rider'/(name+'.gltf')).read_text());data['animations']=[a for a in data.get('animations',[]) if a.get('name') in ['Idle','Walk','Run']]
 nodes=[n for n in data['nodes'] if 'mesh' in n]
 assert all(n.get('skin')==0 and not any(k in n for k in ['matrix','translation','rotation','scale']) for n in nodes)
 primitives=[p for n in nodes for p in data['meshes'][n['mesh']]['primitives']]
 data['meshes']=[{'name':name,'primitives':primitives}];nodes[0]['mesh']=0;nodes[0]['name']=name
 for n in nodes[1:]:n.pop('mesh');n.pop('skin')
 (root/'work'/(name+'.gltf')).write_text(json.dumps(data))
 task=unreal.AssetImportTask();task.filename=str(root/'work'/(name+'.gltf'));task.destination_path='/Game/BattleForTheA/Zombies/'+name;task.destination_name=name;task.automated=True;task.save=True;task.replace_existing=True
 unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
 paths=unreal.EditorAssetLibrary.list_assets(task.destination_path,recursive=True,include_folder=False)
 meshes=[unreal.load_asset(p) for p in paths];meshes=[m for m in meshes if isinstance(m,unreal.SkeletalMesh)];assert len(meshes)==1,(name,paths)
 mesh=meshes[0]
 rows.append({'name':name,'path':mesh.get_path_name(),'materials':[str(m.material_slot_name) for m in mesh.get_editor_property('materials')],'skeleton':str(mesh.get_editor_property('skeleton'))})
 assert unreal.EditorAssetLibrary.save_loaded_asset(mesh,only_if_is_dirty=False)
 unreal.EditorAssetLibrary.save_directory(task.destination_path,only_if_is_dirty=False,recursive=True)
(root/'work/zombie-import.json').write_text(json.dumps(rows,indent=2)+'\n')
exec(compile((root/'Scripts/style_zombie_characters.py').read_text(),'style_zombie_characters.py','exec'))
