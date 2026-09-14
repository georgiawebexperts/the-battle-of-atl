"""Load newly migrated outfit meshes and complete compilation before native review."""
import unreal,json
from pathlib import Path
root=Path(unreal.Paths.project_dir());m=json.loads((root/'Tests/Results/2026-09-14-crowd-outfit-migration.json').read_text());rows=[]
for path in m['roots']:
 mesh=unreal.load_asset(path);assert isinstance(mesh,unreal.SkeletalMesh),path
 materials=[x.material_interface for x in mesh.get_editor_property('materials')];assert all(materials)
 rows.append({'mesh':path,'skeleton':mesh.get_editor_property('skeleton').get_path_name(),'materials':[x.get_path_name() for x in materials]})
unreal.PiedmontWorldTools.finish_editor_asset_loading()
(root/'Tests/Results/2026-09-14-crowd-outfit-load.json').write_text(json.dumps({'loaded':rows,'visual_accepted':False},indent=2)+'\n')
