"""Project the mapped park-side Eastside Trail connector onto currently installed paving."""
import json
from pathlib import Path
from pyproj import Transformer
from shapely.geometry import Point,LineString
root=Path(__file__).resolve().parents[1]
meta=json.loads((root/'SourceAssets/Terrain/terrain-georeference.json').read_text())
source=json.loads((root/'References/piedmont-osm.json').read_text())
way=next(e for e in source['elements'] if e['id']==741964055)
index=way['nodes'].index(5674504871);node=way['geometry'][index]
east,north=Transformer.from_crs('EPSG:4326',meta['crs'],always_xy=True).transform(node['lon'],node['lat'])
p=Point((east-meta['origin_utm'][0])*100*meta['scale'],(north-meta['origin_utm'][1])*100*meta['scale'])
paths=json.loads((root/'SourceAssets/Terrain/park-path-network.json').read_text())['paths']
path=min(paths,key=lambda x:LineString([v[:2] for v in x['points_cm']]).distance(p))
line=LineString([v[:2] for v in path['points_cm']]);target=line.interpolate(line.project(p))
data={'source_node':5674504871,'source_way':741964055,'source_lat_lon':[node['lat'],node['lon']],
 'description':'First route-home waypoint on nearest installed park path; not the full connector or home finish',
 'projected_to_installed_path':path['osm_id'],'xy_cm':[target.x,target.y],
 'projection_distance_game_cm':line.distance(p),'georeference':'terrain-georeference.json',
 'limitation':'The mapped connector is about 66 real metres from the nearest installed centerline. The remaining connector and full route are still required.',
 'author':'2026-09-11 [codex-maclaptop]'}
(root/'SourceAssets/Terrain/battle-park-exit.json').write_text(json.dumps(data,indent=2)+'\n')
print(data)
