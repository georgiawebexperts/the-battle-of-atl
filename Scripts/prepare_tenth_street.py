"""Retain 10th Street, cycle track and Monroe crossing in the game terrain frame."""
import json,math,xml.etree.ElementTree as ET
from pathlib import Path
from pyproj import Transformer
import numpy as np
from shapely.geometry import LineString,box,Point
from shapely.ops import unary_union
root=Path(__file__).resolve().parents[1];meta=json.loads((root/'SourceAssets/Terrain/terrain-georeference.json').read_text())
project=Transformer.from_crs(4326,meta['crs'],always_xy=True);xml=ET.parse(root/'References/tenth-street-monroe.osm').getroot()
nodes={n.get('id'):(float(n.get('lon')),float(n.get('lat'))) for n in xml.findall('node')}
raw=np.fromfile(root/'SourceAssets/Terrain/atlanta-height-krog.r16',dtype='<u2').reshape(meta['size'][1],meta['size'][0])
def xy(p):
 e,n=project.transform(*p);return ((e-meta['origin_utm'][0])/.03,-(n-meta['origin_utm'][1])/.03)
def height(x,y):
 sx,sy,sz=meta['unreal_scale'];lx,ly,_=meta['unreal_location_cm'];gx=(x-lx)/sx;gy=(-y-ly)/sy;ix=int(gx);iy=int(gy);dx=gx-ix;dy=gy-iy
 assert 0<=ix<raw.shape[1]-1 and 0<=iy<raw.shape[0]-1
 a,b,c,d=[(float(raw[v,u])-32768)*sz/128 for u,v in [(ix,iy),(ix+1,iy),(ix,iy+1),(ix+1,iy+1)]]
 return a+(b-a)*dx+(d-b)*dy if dx>=dy else a+(d-c)*dx+(c-a)*dy
ways=[]
for w in xml.findall('way'):
 tags={v.get('k'):v.get('v') for v in w.findall('tag')};refs=[n.get('ref') for n in w.findall('nd')]
 if len(refs)>1:ways.append({'id':int(w.get('id')),'tags':tags,'nodes':refs,'line':LineString([xy(nodes[n]) for n in refs])})
roads=[w for w in ways if w['tags'].get('name')=='10th Street Northeast'];assert roads
monroe=[w for w in ways if w['tags'].get('name')=='Monroe Drive Northeast']
roadnodes={n for w in roads for n in w['nodes']};crossnodes=roadnodes&{n for w in monroe for n in w['nodes']};assert crossnodes
cross=min(crossnodes,key=lambda n:abs(nodes[n][1]-33.782));crossxy=xy(nodes[cross])
# Preserve mapped road topology; keep the requested frontage from Juniper to Monroe.
west=xy((-84.3825,33.782))[0];east=crossxy[0]+50;clip=box(west,-100000,east,100000)
result={'source':'OpenStreetMap contributors ODbL; tenth-street-monroe.osm','frame':'Unreal ESU cm at current one-third geographic scale','roads':[],'cycle_track':[],'monroe_roads':[],'monroe_crossing':{'node':cross,'lon_lat':nodes[cross],'world_cm':[*crossxy,height(*crossxy)]},'status':'Mapped geometry only; road surfaces, traffic, markings, crossing controls and race scenery not yet installed.'}
near_monroe=[w for w in monroe if w['line'].distance(Point(crossxy))<2500]
for w in roads+near_monroe+[w for w in ways if w['tags'].get('highway')=='cycleway' and any(w['line'].distance(r['line'])<350 for r in roads)]:
 geom=w['line'].intersection(Point(crossxy).buffer(2500) if w in near_monroe else clip)
 if w not in roads and w not in near_monroe:geom=geom.intersection(unary_union([r['line'].buffer(650) for r in roads]))
 if geom.is_empty:continue
 lines=[geom] if geom.geom_type=='LineString' else [g for g in geom.geoms if g.geom_type=='LineString']
 for line in lines:
  if line.length<10:continue
  points=[]
  for d in np.linspace(0,line.length,max(2,math.ceil(line.length/100)+1)):
   x,y=line.interpolate(float(d)).coords[0];points.append([round(x,3),round(y,3),round(height(x,y)+12,3)])
  item={'osm_way':w['id'],'tags':w['tags'],'points_cm':points,'length_cm':round(line.length,3)}
  result['roads' if w in roads else 'monroe_roads' if w in near_monroe else 'cycle_track'].append(item)
folder=root/'SourceAssets/Terrain/TenthStreet';folder.mkdir(parents=True,exist_ok=True);(folder/'network.json').write_text(json.dumps(result,indent=2)+'\n')
print(json.dumps({'roads':len(result['roads']),'cycle_segments':len(result['cycle_track']),'monroe_crossing':result['monroe_crossing']}))
