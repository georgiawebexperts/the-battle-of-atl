"""Derive the 14th Street start from OSM gate and the installed path centerline."""
import json,math
from battle_geography import source_to_world, source_yaw_to_world
from pathlib import Path
from pyproj import Transformer
from shapely.geometry import LineString,Point
root=Path(__file__).resolve().parents[1];meta=json.loads((root/'SourceAssets/Terrain/terrain-georeference.json').read_text());raw=json.loads((root/'References/fourteenth-gate-osm.json').read_text());gate=next(e for e in raw['elements'] if e['id']==5674178517);way=next(e for e in raw['elements'] if gate['id'] in e.get('nodes',[]));transform=Transformer.from_crs(4326,32616,always_xy=True);x,y=transform.transform(gate['lon'],gate['lat']);g=[(x-meta['origin_utm'][0])*100*meta['scale'],(y-meta['origin_utm'][1])*100*meta['scale']]
path=next(p for p in json.loads((root/'SourceAssets/Terrain/park-path-network.json').read_text())['paths'] if p['osm_id']==way['id']);line=LineString([v[:2] for v in path['points_cm']]);along=line.project(Point(g));start=line.interpolate(along-200);target=line.interpolate(along-700)
result={'name':'14th Street gate start','source':'OpenStreetMap contributors, ODbL','gate_node':gate['id'],'path_way':way['id'],'gate_lon_lat':[gate['lon'],gate['lat']],'gate_xy_cm':g,'start_xy_cm':[start.x,start.y],'heading_yaw':math.degrees(math.atan2(target.y-start.y,target.x-start.x)),'gate_distance_to_installed_path_cm':line.distance(Point(g)),'placement_policy':'Two game meters inside the mapped gate, facing inward along path 61491566. Z resolved against installed collision. Gate identity inferred from the OSM gate at the 14th Street terminus and Conservancy map; architecture still pending.','official_map':'https://piedmontpark.org/maps/?location=19','official_printable_map':'https://piedmontpark.org/wp-content/uploads/2026/06/2026_PPCPrintableMap.pdf','author':'2026-09-11 [codex-maclaptop]'}
result.update(source_coordinate_system='east/north/up cm',world_start_xy_cm=source_to_world(result['start_xy_cm']),world_heading_yaw=source_yaw_to_world(result['heading_yaw']))
(root/'SourceAssets/Terrain/battle-start.json').write_text(json.dumps(result,indent=2)+'\n');print(result)
