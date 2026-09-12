"""Inspect available simple collision before authoring trunk blockers."""
import unreal,json
from pathlib import Path
root=Path(unreal.Paths.project_dir());paths=sorted(set(r['mesh'] for r in json.loads((root/'SourceAssets/Terrain/park-mixed-canopy.json').read_text())['accepted']));rows=[]
for path in paths:
 mesh=unreal.load_asset(path);body=mesh.get_editor_property('body_setup');agg=body.get_editor_property('agg_geom');row={'mesh':path,'shapes':{}}
 for kind in ['box_elems','sphere_elems','sphyl_elems','convex_elems']:
  shapes=agg.get_editor_property(kind);out=[]
  for shape in shapes:
   values={}
   for prop in ['center','rotation','radius','length','x','y','z']:
    try:
     v=shape.get_editor_property(prop);values[prop]=str(v)
    except Exception:pass
   if kind=='convex_elems':
    try:values['vertices']=len(shape.get_editor_property('vertex_data'))
    except Exception:pass
   out.append(values)
  row['shapes'][kind]=out
 rows.append(row)
(root/'Tests/Results/2026-09-12-tree-collision-inspection.json').write_text(json.dumps(rows,indent=2)+'\n');print(json.dumps(rows))
