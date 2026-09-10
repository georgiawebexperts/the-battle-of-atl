"""Locate disconnected OSM components without inventing connecting paths."""
from pathlib import Path
import json
from shapely.geometry import LineString, Point, Polygon
from shapely.ops import unary_union, nearest_points
P=Path(__file__).resolve().parents[1]
O=P/'SourceAssets/Terrain'
data=json.loads((O/'park-path-network.json').read_text())
components=json.loads((O/'park-graph-components.json').read_text())
paths=data['paths']
lines=[LineString(p['original_vertices_utm']) for p in paths]
keys=[set((round(x,2),round(y,2)) for x,y in p['original_vertices_utm']) for p in paths]
mainkeys=set(map(tuple,components[0]))
main=unary_union([line for line,k in zip(lines,keys) if k & mainkeys])
rows=[]
for component in components[1:]:
 nodes=set(map(tuple,component))
 indices=[i for i,k in enumerate(keys) if k & nodes]
 geometry=unary_union([lines[i] for i in indices])
 a,b=nearest_points(geometry,main)
 rows.append({'node_count':len(component),'osm_ids':sorted(set(paths[i]['osm_id'] for i in indices)),
 'tags':[paths[i]['tags'] for i in indices], 'gap_to_main_real_m':a.distance(b),
 'nearest_pair_utm':[list(a.coords[0]),list(b.coords[0])],
 'requires_review':True,
 'reason':'Bridge elevation/connection' if any(paths[i]['tags'].get('bridge')=='yes' for i in indices) else 'Separated source centerline; check connector or boundary clipping'})
report={'status':'needs_review','main_component_nodes':len(mainkeys),'disconnected_components':rows,
 'policy':'No nearest-neighbor shortcut edges added. A small gap does not prove a safe crossing.',
 'bridge_osm_ids':sorted(set(p['osm_id'] for p in paths if p['tags'].get('bridge')=='yes'))}
(O/'park-connectivity-audit.json').write_text(json.dumps(report,indent=2))
for r in rows:print(r['osm_ids'],round(r['gap_to_main_real_m'],2),'m',r['reason'])
