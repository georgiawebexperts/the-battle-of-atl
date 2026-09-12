"""Prepare mapped Irwin/Lake building footprints and explicit height estimates."""
import json,math,array,sys
from pathlib import Path
from pyproj import Transformer
from shapely.geometry import Polygon,LineString
from shapely.ops import unary_union
root=Path(__file__).resolve().parents[1];folder=root/'SourceAssets/Terrain/IrwinBuildings';folder.mkdir(exist_ok=True)
m=json.loads((root/'SourceAssets/Terrain/terrain-georeference.json').read_text());project=Transformer.from_crs(4326,m['crs'],always_xy=True)
raw=array.array('H');raw.frombytes((root/'SourceAssets/Terrain'/m['active_heightmap']).read_bytes())
if sys.byteorder!='little':raw.byteswap()
def xy(p):
 e,n=project.transform(p['lon'],p['lat']);return [(e-m['origin_utm'][0])/.03,-(n-m['origin_utm'][1])/.03]
def height(x,y):
 sx,sy,sz=m['unreal_scale'];lx,ly,_=m['unreal_location_cm'];gx=(x-lx)/sx;gy=(-y-ly)/sy;ix=math.floor(gx);iy=math.floor(gy);dx=gx-ix;dy=gy-iy;w=m['size'][0]
 a,b,c,d=[(raw[v*w+u]-32768)*sz/128 for u,v in [(ix,iy),(ix+1,iy),(ix,iy+1),(ix+1,iy+1)]]
 return a+(b-a)*dx+(d-b)*dy if dx>=dy else a+(d-c)*dx+(c-a)*dy
road=unary_union([LineString(r['points_cm']).buffer(480,join_style=2) for r in json.loads((root/'SourceAssets/Terrain/IrwinTraffic/network.json').read_text())['roads']])
trail_data=json.loads((folder/'trail-footprints.json').read_text())
trail=unary_union([Polygon(p) for p in trail_data['polygons']]);protected=unary_union([road,trail.buffer(100,join_style=2)])
ids={211061295:1,211061296:1,211061297:2,211061298:2,211061637:1,1139043893:3,1396654871:2};rows=[]
for e in json.loads((root/'References/irwin-lake-buildings-osm.json').read_text())['elements']:
 if e['type']!='way' or e['id'] not in ids:continue
 poly=Polygon([xy(p) for p in e['geometry']]);assert poly.is_valid
 overlap=poly.intersection(road).area
 # Road safety clearance takes precedence where our gameplay road is wider
 # than mapped streets; retain original shape alongside the trimmed candidate.
 clipped=poly.difference(protected);assert clipped.geom_type=='Polygon' and not clipped.interiors
 coords=list(clipped.exterior.coords)[:-1];heights=[height(x,y) for x,y in coords]
 rows.append({'osm_way':e['id'],'tags':e['tags'],'original_footprint_xy':list(poly.exterior.coords),'footprint_xy':coords,'road_trim_area_cm2':overlap,'trail_clearance_trim_area_cm2':poly.difference(road).intersection(trail.buffer(100,join_style=2)).area,'base_z_cm':max(heights)+10,'foundation_z_cm':min(heights)-30,'storeys':ids[e['id']],'storey_height_cm':340,'height_source':'Gameplay estimate, not surveyed height','terrain_range_cm':[min(heights),max(heights)]})
assert len(rows)==len(ids)
r={'author':'2026-09-12 [codex-maclaptop]','source':'OpenStreetMap contributors ODbL; References/irwin-lake-buildings-osm.json','frame':'world ESU centimetres, one-third footprint geography; full gameplay storey heights','buildings':rows,'reserved_roadside_width_cm':180,'trail_setback_cm':100,'main_map_changed':False}
(folder/'buildings.json').write_text(json.dumps(r,indent=2)+'\n');print([(r['osm_way'],r['road_trim_area_cm2']) for r in rows])
