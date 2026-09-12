"""Locate the user-identified Irwin/Lake crossing in the game's ESU frame."""
import json,math,array,sys
from pathlib import Path
from pyproj import Transformer
from shapely.geometry import LineString,Point
root=Path(__file__).resolve().parents[1];out=root/'SourceAssets/Terrain/IrwinTraffic';out.mkdir(exist_ok=True);m=json.loads((root/'SourceAssets/Terrain/terrain-georeference.json').read_text());project=Transformer.from_crs(4326,m['crs'],always_xy=True);data=json.loads((root/'References/eastside-krog-corridor-osm.json').read_text())
ways={e['id']:e for e in data['elements'] if e['type']=='way'};cross=ways[742982445];index=cross['nodes'].index(69331892);coordinate=cross['geometry'][index]
def xy(p):
 e,n=project.transform(p['lon'],p['lat']);return [(e-m['origin_utm'][0])/.03,-(n-m['origin_utm'][1])/.03]
site=xy(coordinate);raw=array.array('H');raw.frombytes((root/'SourceAssets/Terrain'/m['active_heightmap']).read_bytes())
if sys.byteorder!='little':raw.byteswap()
def height(x,y):
 sx,sy,sz=m['unreal_scale'];lx,ly,_=m['unreal_location_cm'];gx=(x-lx)/sx;gy=(-y-ly)/sy;ix=int(gx);iy=int(gy);dx=gx-ix;dy=gy-iy;w=m['size'][0]
 a,b,c,d=[(raw[v*w+u]-32768)*sz/128 for u,v in [(ix,iy),(ix+1,iy),(ix,iy+1),(ix+1,iy+1)]]
 return a+(b-a)*dx+(d-b)*dy if dx>=dy else a+(d-c)*dx+(c-a)*dy
rows=[]
for road in ways.values():
 if road.get('tags',{}).get('name') not in ['Irwin Street Northeast','Lake Avenue Northeast','Krog Street Northeast']:continue
 line=LineString([xy(p) for p in road['geometry']]);clipped=line.intersection(Point(site).buffer(6500))
 for part in ([clipped] if clipped.geom_type=='LineString' else getattr(clipped,'geoms',[])):
  if part.is_empty or part.length<1:continue
  points=[]
  for d in list(range(0,math.ceil(part.length),50))+[part.length]:
   x,y=part.interpolate(d).coords[0];points.append([x,y,height(x,y)+14])
  rows.append({'osm_way':road['id'],'tags':road['tags'],'points_cm':points})
points=[]
for p in cross['geometry']:
 x,y=xy(p);points.append([x,y,height(x,y)+14])
r={'author':'2026-09-12 [codex-maclaptop]','source':'OpenStreetMap contributors ODbL; References/eastside-krog-corridor-osm.json, matched to user screenshot','frame':'Unreal east/south/up centimetres, one-third geography','crossing_node':69331892,'crossing_lon_lat':[coordinate['lon'],coordinate['lat']],'crossing_xyz':site+[height(*site)+14],'trail_crossing':{'osm_way':cross['id'],'tags':cross['tags'],'points_cm':points},'roads':rows,'active_heightmap':m['active_heightmap'],'main_map_changed':False,'scope':'Mapped topology and terrain only. Existing raised trail support, road surface, controls and traffic pending.'};(out/'network.json').write_text(json.dumps(r,indent=2)+'\n');print({'crossing':r['crossing_lon_lat'],'xyz':r['crossing_xyz'],'road_segments':len(rows)})
