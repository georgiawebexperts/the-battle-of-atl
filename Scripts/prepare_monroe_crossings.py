"""Retain individual mapped Monroe crossings for shared traffic-control review."""
import json,xml.etree.ElementTree as ET
from pathlib import Path
from pyproj import Transformer
root=Path(__file__).resolve().parents[1];meta=json.loads((root/'SourceAssets/Terrain/terrain-georeference.json').read_text());project=Transformer.from_crs(4326,meta['crs'],always_xy=True)
xml=ET.parse(root/'References/tenth-street-monroe.osm').getroot();nodes={n.get('id'):(float(n.get('lon')),float(n.get('lat'))) for n in xml.findall('node')};wanted={1396654821,1396654823,231270041,1387296277};rows=[]
for w in xml.findall('way'):
 if int(w.get('id')) not in wanted:continue
 points=[]
 for n in w.findall('nd'):
  e,north=project.transform(*nodes[n.get('ref')]);points.append([(e-meta['origin_utm'][0])/.03,-(north-meta['origin_utm'][1])/.03])
 rows.append({'osm_way':int(w.get('id')),'tags':{t.get('k'):t.get('v') for t in w.findall('tag')},'points_cm':points})
assert len(rows)==4
allpoints=[p for row in rows for p in row['points_cm']];lo=[min(p[k] for p in allpoints)-100 for k in range(2)];hi=[max(p[k] for p in allpoints)+100 for k in range(2)]
(root/'SourceAssets/Terrain/MonroeTraffic/crossings.json').write_text(json.dumps({'author':'2026-09-12 [codex-maclaptop]','source':'OpenStreetMap contributors, References/tenth-street-monroe.osm','crossings':rows,'control_bounds_xy':[lo,hi],'scope':'Individual alignments preserved. Shared conservative vehicle conflict area is gameplay control, not a real-world signal timing claim.'},indent=2)+'\n')
