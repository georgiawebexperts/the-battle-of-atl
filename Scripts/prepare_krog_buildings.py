"""Mapped building envelopes near Krog, with explicitly estimated heights."""
import json,math,array,sys,re
from pathlib import Path
from pyproj import Transformer
from shapely.geometry import Polygon,LineString,Point
from shapely.ops import unary_union
root=Path(__file__).resolve().parents[1];folder=root/'SourceAssets/Terrain/KrogBuildings';folder.mkdir(exist_ok=True)
m=json.loads((root/'SourceAssets/Terrain/terrain-georeference.json').read_text());project=Transformer.from_crs(4326,m['crs'],always_xy=True)
raw=array.array('H');raw.frombytes((root/'SourceAssets/Terrain/KrogTraffic/atlanta-height-dekalb-crowned-candidate.r16').read_bytes())
if sys.byteorder!='little':raw.byteswap()
def xy(p):
 e,n=project.transform(p['lon'],p['lat']);return [(e-m['origin_utm'][0])/.03,-(n-m['origin_utm'][1])/.03]
def height(x,y):
 sx,sy,sz=m['unreal_scale'];lx,ly,_=m['unreal_location_cm'];gx=(x-lx)/sx;gy=(-y-ly)/sy;ix=math.floor(gx);iy=math.floor(gy);dx=gx-ix;dy=gy-iy;w=m['size'][0]
 a,b,c,d=[(raw[v*w+u]-32768)*sz/128 for u,v in [(ix,iy),(ix+1,iy),(ix,iy+1),(ix+1,iy+1)]]
 return a+(b-a)*dx+(d-b)*dy if dx>=dy else a+(d-c)*dx+(c-a)*dy

manifest=json.loads((root/'SourceAssets/Terrain/KrogContinuousShell/manifest.json').read_text())
tunnel=LineString([(p[0],-p[1]) for p in manifest['roof_samples']]);area=tunnel.buffer(2200)
roads=json.loads((root/'SourceAssets/Terrain/KrogTraffic/network.json').read_text())['roads']
road=unary_union([LineString([p[:2] for p in r['points_cm']]).buffer((450 if r['tags']['name']=='DeKalb Avenue Northeast' else 300)+150) for r in roads])
paths=json.loads((root/'SourceAssets/Terrain/krog-route-network.json').read_text())['paths']
trail=unary_union([LineString([(p[0],-p[1]) for p in r['points_cm']]).buffer(260) for r in paths])
home_source=(root/'Source/AuraPlayground/BattleHomeData.h').read_text()
def vectors(text):return [tuple(map(float,m)) for m in re.findall(r'FVector\(([\d.-]+),([\d.-]+),([\d.-]+)\)',text)]
finish_clearance=[]
for name,width in [('Road',375),('Approach',250)]:
 section=home_source.split('inline const FVector '+name+'[] = {',1)[1].split('};',1)[0]
 finish_clearance.append(LineString([p[:2] for p in vectors(section)]).buffer(width))
for name,radius in [('Gate',800),('Home',650)]:
 p=vectors(home_source.split('inline const FVector '+name,1)[1].split(';',1)[0].replace('(', 'FVector(',1))[0]
 finish_clearance.append(Point(p[:2]).buffer(radius))
protected=unary_union([road,trail,tunnel.buffer(700),*finish_clearance])
rows=[];excluded=[];reserved=[]
for e in json.loads((root/'References/krog-buildings-osm.json').read_text())['elements']:
 if e.get('type')!='way' or len(e.get('geometry',[]))<4:continue
 poly=Polygon([xy(p) for p in e['geometry']])
 if not poly.is_valid or not poly.intersects(area):continue
 if e.get('tags',{}).get('name')=='97 Estoria':
  reserved.append({'osm_way':e['id'],'footprint_xy':list(poly.exterior.coords),'reason':'Existing BattleHome 98 Estoria venue and celebration retained; use footprint for later dedicated art refinement.'});continue
 if poly.area<2000:continue
 clipped=poly.difference(protected)
 if clipped.geom_type!='Polygon' or clipped.interiors or clipped.area<poly.area*.6:
  excluded.append({'osm_way':e['id'],'reason':'Road/trail clearance or multipart footprint requires individual design','original_area_cm2':poly.area});continue
 clipped=clipped.simplify(3,preserve_topology=True)
 assert clipped.intersection(protected.buffer(-3.01)).area<.01
 tags=e.get('tags',{});levels=tags.get('building:levels')
 try:storeys=max(1,min(8,int(float(levels))));source='OSM building:levels; full gameplay storey height'
 except (ValueError,TypeError):storeys=1 if tags.get('building') in ['house','detached','garage','garages','shed','industrial','warehouse'] else 2;source='Estimated storeys, not surveyed; full gameplay storey height'
 coords=list(clipped.exterior.coords)[:-1];heights=[height(x,y) for x,y in coords]
 rows.append({'osm_way':e['id'],'tags':tags,'original_footprint_xy':list(poly.exterior.coords),'footprint_xy':coords,
              'clearance_trim_area_cm2':poly.area-clipped.area,'base_z_cm':max(heights)+10,'foundation_z_cm':min(heights)-30,
              'storeys':storeys,'storey_height_cm':340,'height_source':source,'terrain_range_cm':[min(heights),max(heights)]})
assert rows
(folder/'buildings.json').write_text(json.dumps({'author':'2026-09-12 [codex-maclaptop]','source':'OpenStreetMap contributors ODbL; retained References/krog-buildings-osm.json','buildings':rows,'excluded':excluded,'reserved_landmarks':reserved,'finish_clearance_source':'Source/AuraPlayground/BattleHomeData.h','main_map_changed':False,'scope':'Mapped footprints, gameplay clearances and explicit height estimates. Facades, entrances and native ground/clearance unverified.'},indent=2)+'\n')
print(json.dumps({'buildings':len(rows),'excluded':len(excluded),'height_tags':sum('OSM' in r['height_source'] for r in rows)}))
