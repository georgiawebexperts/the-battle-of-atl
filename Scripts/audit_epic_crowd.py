import unreal,json,pathlib
root=pathlib.Path(unreal.Paths.project_dir())
m=json.loads((root/'SourceAssets/Manifests/epic-crowd-selection.json').read_text())
rows=[]
for path in m['roots']:
 a=unreal.load_asset(path);assert a,path
 row={'path':path,'class':a.get_class().get_name()}
 if isinstance(a,(unreal.SkeletalMesh,unreal.AnimSequence)):
  row['skeleton']=a.get_editor_property('skeleton').get_path_name()
 if isinstance(a,unreal.SkeletalMesh):
  row['materials']=[str(x.material_interface.get_path_name()) if x.material_interface else None for x in a.get_editor_property('materials')]
  assert all(row['materials']),path
 if isinstance(a,unreal.AnimSequence):row['length']=a.get_editor_property('sequence_length')
 rows.append(row)
(root/'work/epic-crowd-load.json').write_text(json.dumps({'loaded':rows},indent=2)+'\n')
print('BATTLE_CROWD_LOAD_COMPLETE',len(rows))
