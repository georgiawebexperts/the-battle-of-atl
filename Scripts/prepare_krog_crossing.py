"""Prepare mapped Krog/DeKalb approach geometry for native surface review."""
import json,math,array,sys
from pathlib import Path
from pyproj import Transformer
from shapely.geometry import LineString,Point
root=Path(__file__).resolve().parents[1];out=root/'SourceAssets/Terrain/KrogTraffic';out.mkdir(exist_ok=True)
m=json.loads((root/'SourceAssets/Terrain/terrain-georeference.json').read_text());project=Transformer.from_crs(4326,m['crs'],always_xy=True)
data=json.loads((root/'References/eastside-krog-corridor-osm.json').read_text());ways={e['id']:e for e in data['elements'] if e['type']=='way'}
krog=ways[44062160];node=krog['nodes'][0];coordinate=krog['geometry'][0]
assert node in ways[1378256293]['nodes'] and node in ways[1378258205]['nodes']
def xy(p):
 e,n=project.transform(p['lon'],p['lat']);return [(e-m['origin_utm'][0])/.03,-(n-m['origin_utm'][1])/.03]
raw=array.array('H');raw.frombytes((root/'SourceAssets/Terrain'/m['active_heightmap']).read_bytes())
if sys.byteorder!='little':raw.byteswap()
def height(x,y):
 sx,sy,sz=m['unreal_scale'];lx,ly,_=m['unreal_location_cm'];gx=(x-lx)/sx;gy=(-y-ly)/sy;ix=math.floor(gx);iy=math.floor(gy);dx=gx-ix;dy=gy-iy;w,h=m['size'];assert 0<=ix<w-1 and 0<=iy<h-1
 a,b,c,d=[(raw[v*w+u]-32768)*sz/128 for u,v in [(ix,iy),(ix+1,iy),(ix,iy+1),(ix+1,iy+1)]]
 return a+(b-a)*dx+(d-b)*dy if dx>=dy else a+(d-c)*dx+(c-a)*dy
site=xy(coordinate);roads=[]
for road in ways.values():
 if road.get('tags',{}).get('name') not in ['DeKalb Avenue Northeast','Krog Street Northeast','Krog Street Southeast']:continue
 if road.get('tags',{}).get('tunnel')=='yes':continue # Separate floor profile required beneath the railway.
 clip=LineString([xy(p) for p in road['geometry']]).intersection(Point(site).buffer(4500))
 for part in ([clip] if clip.geom_type=='LineString' else getattr(clip,'geoms',[])):
  if part.is_empty or part.length<1:continue
  points=[]
  for distance in list(range(0,math.ceil(part.length),100))+[part.length]:
   x,y=part.interpolate(distance).coords[0];points.append([x,y,height(x,y)+14])
  roads.append({'osm_way':road['id'],'tags':road['tags'],'points_cm':points})
result={'author':'2026-09-12 [codex-maclaptop]','source':'Retained OpenStreetMap contributors ODbL reference; not a fresh street survey','frame':'Unreal east/south/up centimetres, one-third geography','crossing_node':node,'crossing_lon_lat':[coordinate['lon'],coordinate['lat']],'crossing_xyz':site+[height(*site)+14],'roads':roads,'active_heightmap':m['active_heightmap'],'scope':'Surface approach geometry only. Tunnel road excluded pending floor/clearance matching. No main-map changes.'}
(out/'network.json').write_text(json.dumps(result,indent=2)+'\n');print(json.dumps({'junction':result['crossing_xyz'],'segments':len(roads),'samples':sum(len(r['points_cm']) for r in roads)}))
