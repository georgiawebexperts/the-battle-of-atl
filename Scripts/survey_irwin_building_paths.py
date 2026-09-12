"""Retain installed source trail geometry for building setback and native checks."""
from pathlib import Path
import json
from shapely.geometry import Polygon
from shapely.ops import unary_union
root=Path(__file__).resolve().parents[1];folder=root/'SourceAssets/Terrain/IrwinBuildings'
rows=json.loads((folder/'buildings.json').read_text())['buildings'];buildings=unary_union([Polygon(r['original_footprint_xy']) for r in rows]);polys=[];samples=[]
for directory in ['EastsideTrail','KrogRoute']:
 for path in (root/'SourceAssets/Terrain'/directory).glob('*.obj'):
  if not any(s in path.stem for s in ['Asphalt','Concrete']):continue
  vertices=[]
  for line in path.read_text().splitlines():
   p=line.split()
   if p and p[0]=='v':vertices.append(tuple(map(float,p[1:4])))
   elif p and p[0]=='f':
    tri=[vertices[int(v.split('/')[0])-1] for v in p[1:]];poly=Polygon([p[:2] for p in tri])
    if poly.area>.001 and poly.distance(buildings)<300:
     polys.append(poly);samples.append({'xyz':[sum(p[k] for p in tri)/3 for k in range(3)],'source':path.stem})
trail=unary_union(polys);parts=list(trail.geoms) if trail.geom_type=='MultiPolygon' else [trail]
(folder/'trail-footprints.json').write_text(json.dumps({'polygons':[list(p.exterior.coords) for p in parts],'scope':'Conservative protected footprint; enclosed holes intentionally protected too.'})+'\n')
(folder/'trail-probes.json').write_text(json.dumps({'samples':samples})+'\n')
print({'trail_probes':len(samples),'original_overlap_cm2':buildings.intersection(trail).area})
