"""Extend retained Monroe road topology around the BeltLine crossing."""
import json,xml.etree.ElementTree as ET,math
from pathlib import Path
from pyproj import Transformer
from shapely.geometry import LineString,box
root=Path(__file__).resolve().parents[1];folder=root/'SourceAssets/Terrain/MonroeTraffic';m=json.loads((root/'SourceAssets/Terrain/terrain-georeference.json').read_text());project=Transformer.from_crs(4326,m['crs'],always_xy=True)
xml=ET.parse(root/'References/tenth-street-monroe.osm').getroot();nodes={n.get('id'):(float(n.get('lon')),float(n.get('lat'))) for n in xml.findall('node')};rows=[]
for w in xml.findall('way'):
 tags={t.get('k'):t.get('v') for t in w.findall('tag')}
 if tags.get('name')!='Monroe Drive Northeast':continue
 pts=[]
 for n in w.findall('nd'):
  e,north=project.transform(*nodes[n.get('ref')]);pts.append([(e-m['origin_utm'][0])/.03,-(north-m['origin_utm'][1])/.03])
 clipped=LineString(pts).intersection(box(2000,3000,22000,20000))
 for line in ([clipped] if clipped.geom_type=='LineString' else getattr(clipped,'geoms',[])):
  if line.is_empty or line.length<1:continue
  points=[list(line.interpolate(d).coords[0]) for d in range(0,math.ceil(line.length),50)]+[list(line.coords[-1])]
  rows.append({'osm_way':int(w.get('id')),'tags':tags,'points_cm':points})
assert rows
(folder/'approaches-network.json').write_text(json.dumps({'author':'2026-09-12 [codex-maclaptop]','roads':rows,'source':'OpenStreetMap contributors; References/tenth-street-monroe.osm','main_map_changed':False},indent=2)+'\n');print(len(rows),sum(LineString(r['points_cm']).length for r in rows))
