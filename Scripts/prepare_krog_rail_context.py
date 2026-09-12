"""Project mapped railway lines and locate their crossings over the tunnel."""
import json
from pathlib import Path
from pyproj import Transformer
from shapely.geometry import LineString,Point
root=Path(__file__).resolve().parents[1];base=root/'SourceAssets/Terrain'
meta=json.loads((base/'terrain-georeference.json').read_text())
project=Transformer.from_crs(4326,meta['crs'],always_xy=True)
manifest=json.loads((base/'KrogContinuousShell/manifest.json').read_text())
tunnel=LineString([(p[0],-p[1]) for p in manifest['roof_samples']])
area=tunnel.buffer(2500,cap_style=2)
source=json.loads((root/'References/krog-railways-osm.json').read_text())
rails=[]
for way in source['elements']:
 if way.get('type')!='way' or len(way.get('geometry',[]))<2:continue
 points=[]
 for p in way['geometry']:
  e,n=project.transform(p['lon'],p['lat']);points.append([(e-meta['origin_utm'][0])/.03,-(n-meta['origin_utm'][1])/.03])
 original=LineString(points);clip=original.intersection(area)
 pieces=[clip] if clip.geom_type=='LineString' else getattr(clip,'geoms',[])
 for part in pieces:
  if part.is_empty or part.geom_type!='LineString' or part.length<1:continue
  crossing=part.intersection(tunnel)
  crossings=[crossing] if crossing.geom_type=='Point' else [p for p in getattr(crossing,'geoms',[]) if p.geom_type=='Point']
  rails.append({'osm_way':way['id'],'tags':way.get('tags',{}),'points_xy_cm':[list(p) for p in part.coords],
                'tunnel_crossings':[{'xy_cm':[p.x,p.y],'distance_from_portal_cm':tunnel.project(p)} for p in crossings]})
out=base/'KrogRailContext';out.mkdir(exist_ok=True)
report={'author':'2026-09-12 [codex-maclaptop]','source':'OpenStreetMap contributors, ODbL; retained References/krog-railways-osm.json',
        'frame':'World east/south centimetres, one-third geographic scale','railways':rails,
        'scope':'Plan alignment only. Rail elevation, deck support, terrain joins and present-day operational status are not verified. No main-map changes.'}
(out/'network.json').write_text(json.dumps(report,indent=2)+'\n')
print(json.dumps({'rail_segments':len(rails),'crossing_segments':sum(bool(r['tunnel_crossings']) for r in rails),
                  'crossings':[{'way':r['osm_way'],'railway':r['tags'].get('railway'),'at':r['tunnel_crossings']} for r in rails if r['tunnel_crossings']]}))
