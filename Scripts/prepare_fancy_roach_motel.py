"""Retain the identified apartment footprints in the game's terrain frame."""
import json, xml.etree.ElementTree as ET
from pathlib import Path
from pyproj import Transformer
from shapely.geometry import Polygon
import numpy as np
root=Path(__file__).resolve().parents[1]
meta=json.loads((root/'SourceAssets/Terrain/terrain-georeference.json').read_text())
project=Transformer.from_crs(4326,meta['crs'],always_xy=True)
xml=ET.parse(root/'References/maa-piedmont-park.osm').getroot()
nodes={n.get('id'):(float(n.get('lon')),float(n.get('lat'))) for n in xml.findall('node')}
ways={int(w.get('id')):w for w in xml.findall('way')}
def tags(w):return {t.get('k'):t.get('v') for t in w.findall('tag')}
def ring(w):return [nodes[n.get('ref')] for n in w.findall('nd')]
site=Polygon(ring(ways[1436985590]))
assert tags(ways[1436985590])['name']=='MAA Piedmont Park'
raw=np.fromfile(root/'SourceAssets/Terrain/atlanta-height-krog.r16',dtype='<u2').reshape(meta['size'][1],meta['size'][0])
def world(p):
 e,n=project.transform(*p);x=(e-meta['origin_utm'][0])/.03;y=-(n-meta['origin_utm'][1])/.03
 sx,sy,sz=meta['unreal_scale'];lx,ly,_=meta['unreal_location_cm'];gx=(x-lx)/sx;gy=(-y-ly)/sy;ix=int(gx);iy=int(gy);dx=gx-ix;dy=gy-iy
 assert 0<=ix<raw.shape[1]-1 and 0<=iy<raw.shape[0]-1
 a,b,c,d=[(float(raw[v,u])-32768)*sz/128 for u,v in [(ix,iy),(ix+1,iy),(ix,iy+1),(ix+1,iy+1)]]
 z=a+(b-a)*dx+(d-b)*dy if dx>=dy else a+(d-c)*dx+(c-a)*dy
 return [round(x,3),round(y,3),round(z,3)]
buildings=[]
for ident in (218548431,886024366):
 w=ways[ident];polygon=Polygon(ring(w));assert polygon.is_valid and site.intersection(polygon).area/polygon.area>.95
 points=[world(p) for p in ring(w)]
 buildings.append({'osm_way':ident,'tags':tags(w),'footprint_world_cm':points,'base_height_cm':min(p[2] for p in points),'levels':int(tags(w)['building:levels']),'height_note':'Floor count sourced; facade and roof heights must be authored from exterior reference.'})
report={'name':'The Fancy Roach Motel','reference_name':'MAA Piedmont Park','address':'250 10th Street NE','official_reference':'https://www.maac.com/georgia/atlanta/maa-piedmont-park/','source':'OpenStreetMap contributors, ODbL; retained maa-piedmont-park.osm','site_way':1436985590,'world_frame':'Unreal east/south/up centimetres, same one-third geographic scale as existing terrain','buildings':buildings,'status':'Footprint/terrain preparation only. Building, road access, signage and nearby bench scene not yet installed.'}
out=root/'SourceAssets/Terrain/FancyRoachMotel';out.mkdir(parents=True,exist_ok=True)
(out/'footprints.json').write_text(json.dumps(report,indent=2)+'\n')
print(json.dumps({'buildings':len(buildings),'vertices':[len(b['footprint_world_cm']) for b in buildings],'base_heights':[b['base_height_cm'] for b in buildings]}))
