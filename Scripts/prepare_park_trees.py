"""Deterministic ESU tree stations with explicit OSM provenance and exclusions.

Mapped tree positions are retained where safe. Additional canopy is authored in
mapped woods, a shore belt and boundary belt; it is not a surveyed tree census.
"""
import json,random,math,pathlib
from pyproj import Transformer
from shapely.geometry import Point,Polygon,LineString
from shapely.ops import unary_union
root=pathlib.Path(__file__).resolve().parents[1]
read=lambda p:json.loads((root/p).read_text())
meta=read('SourceAssets/Terrain/terrain-georeference.json');origin=meta['origin_utm'];scale=100/3
tr=Transformer.from_crs('EPSG:4326',meta['crs'],always_xy=True)
def xy(g):
 x,y=tr.transform(g['lon'],g['lat']);return ((x-origin[0])*scale,-(y-origin[1])*scale)
def polygon(e):return Polygon([xy(g) for g in e['geometry']]).buffer(0)
park=polygon(read('References/piedmont-boundary-osm.json')['elements'][0]).buffer(-160)
elements=read('References/piedmont-trees-and-exclusions-osm.json')['elements']
lake=read('SourceAssets/Terrain/lake-clara-meer.json')
water=Polygon([(p[0],-p[1]) for p in lake['outer_cm']], [[(p[0],-p[1]) for p in lake['island_cm']]])
paths=read('SourceAssets/Terrain/park-path-network.json')['paths']
exclusions=[water.buffer(120)]
for p in paths:exclusions.append(LineString([(v[0],-v[1]) for v in p['points_cm']]).buffer(p['width_game_cm']/2+180))
woods=[];mapped=[]
for e in elements:
 t=e.get('tags',{})
 if t.get('natural')=='tree' and 'lat' in e:mapped.append((xy(e),e['id']))
 if 'geometry' not in e or len(e['geometry'])<4:continue
 if t.get('natural')=='wood' or t.get('landuse')=='forest':woods.append(polygon(e))
 if 'building' in t or t.get('leisure') in ('pitch','swimming_pool','playground','garden','dog_park') or t.get('landuse')=='allotments':exclusions.append(polygon(e).buffer(180))
# Current authored frisbee clearings. Stations are world ESU, not source ENU.
for x,y in [(4500,5000),(-6500,8500),(6200,8000),(-10500,7600),(4000,1800)]:exclusions.append(Point(x,y).buffer(1900))
blocked=unary_union(exclusions)
legal=park.difference(blocked)
regions=[('mapped_woodland',unary_union(woods).intersection(legal)),('authored_lakeshore',water.buffer(1600).difference(water).intersection(legal)),('authored_boundary',park.difference(park.buffer(-1100)).intersection(legal))]
rng=random.Random(2701);rows=[];grid={};spacing=330

def accept(pt,kind,osm=None):
 p=Point(pt)
 if not legal.covers(p):return False
 key=(math.floor(pt[0]/spacing),math.floor(pt[1]/spacing))
 if any(math.dist(pt,q)<spacing for i in range(key[0]-1,key[0]+2) for j in range(key[1]-1,key[1]+2) for q in grid.get((i,j),[])):return False
 grid.setdefault(key,[]).append(pt);rows.append({'xy_cm':list(pt),'yaw':rng.uniform(-180,180),'height_game_cm':rng.uniform(850,1450),'placement':kind,'osm_id':osm});return True
for pt,osm in mapped:accept(pt,'mapped_tree',osm)
for name,region in regions:
 if region.is_empty:continue
 a,b,c,d=region.bounds
 for _ in range(18000):
  pt=(rng.uniform(a,c),rng.uniform(b,d))
  if region.covers(Point(pt)):accept(pt,name)
report={'coordinate_system':'ESU: +X east, +Y south, cm at 1:3','seed':2701,'asset_status':'Interim Epic ArchVis HillTree_02; not the required Megascans species','spacing_cm':spacing,'path_clearance_cm':180,'water_clearance_cm':120,'trees':rows,'count':len(rows),'by_region':{k:sum(r['placement']==k for r in rows) for k in ['mapped_tree']+[x[0] for x in regions]},'all_outside_exclusions':all(not blocked.intersects(Point(r['xy_cm'])) for r in rows),'all_inside_park':all(park.covers(Point(r['xy_cm'])) for r in rows)}
assert report['all_outside_exclusions'] and report['all_inside_park'] and len(rows)>300
out=root/'SourceAssets/Terrain/park-tree-stations.json';out.write_text(json.dumps(report,indent=2)+'\n')
print(json.dumps({k:v for k,v in report.items() if k!='trees'},indent=2))
