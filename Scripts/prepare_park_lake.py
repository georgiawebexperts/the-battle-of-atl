"""Lake Clara Meer footprint, island and DEM-derived still-water elevation."""
from pathlib import Path
import json,numpy as np,shapely
from shapely.geometry import Polygon
from pyproj import Transformer
P=Path(__file__).resolve().parents[1];O=P/'SourceAssets/Terrain';m=json.loads((O/'terrain-georeference.json').read_text());r=json.loads((P/'References/lake-osm.json').read_text())['elements'][0]
t=Transformer.from_crs('EPSG:4326',m['crs'],always_xy=True);rings={member['role']:[t.transform(v['lon'],v['lat']) for v in member['geometry']] for member in r['members']}
poly=Polygon(rings['outer'],[rings['inner']]);west,south,east,north=m['bounds_utm'];spacing=m['sample_spacing_real_m'];dem=np.load(O/'atlanta-elevation-north-up.npy')
# Read submerged interior cells only, avoiding the bank and island.
water=poly.buffer(-5);x0,y0,x1,y1=water.bounds
ix=np.arange(int((x0-west)/spacing),int((x1-west)/spacing)+1);iy=np.arange(int((north-y1)/spacing),int((north-y0)/spacing)+1);xx,yy=np.meshgrid(ix,iy);mask=shapely.contains_xy(water,west+xx*spacing,north-yy*spacing);samples=dem[yy[mask],xx[mask]];height=float(np.median(samples));ox,oy=m['origin_utm'];z=(height-m['base_elevation_m'])*100/3
out={'name':'Lake Clara Meer','osm_relation':r['id'],'source':'OpenStreetMap contributors / USGS 3DEP','water_elevation_real_m':height,'water_z_cm':z,'dem_interior_sample_count':len(samples),'dem_percentiles_real_m':np.percentile(samples,[5,50,95]).tolist(),'outer_cm':[[(x-ox)*100/3,(y-oy)*100/3,z] for x,y in rings['outer']],'island_cm':[[(x-ox)*100/3,(y-oy)*100/3,z] for x,y in rings['inner']]}
(O/'lake-clara-meer.json').write_text(json.dumps(out,indent=2));print({k:v for k,v in out.items() if not k.endswith('_cm')})
# 3DEP measures the water surface, not the bed. Add an explicitly authored
# underwater basin, preserving every sample outside the lake and its island.
raw=np.fromfile(O/'atlanta-height.r16',dtype='<u2').reshape(m['size'][1],m['size'][0]);carved=raw.copy()
x0,y0,x1,y1=poly.bounds
ix=np.arange(int((x0-west)/spacing),int((x1-west)/spacing)+1);iy=np.arange(int((north-y1)/spacing),int((north-y0)/spacing)+1);xx,yy=np.meshgrid(ix,iy)
fullmask=shapely.contains_xy(poly,west+xx*spacing,north-yy*spacing)
px=west+xx[fullmask]*spacing;py=north-yy[fullmask]*spacing
bank_distance=shapely.distance(shapely.points(px,py),poly.boundary)
depth=np.minimum(bank_distance*.3,4.5)
values=np.round(32768+(height-depth-m['base_elevation_m'])*128).astype('<u2')
carved[(m['size'][1]-1)-yy[fullmask],xx[fullmask]]=values
carved.tofile(O/'atlanta-height-lakebed.r16')
# Solid submerged shoreline walls. Leave bridge crossings for their deck actors;
# the water hazard still protects these gaps until decks are built.
network=json.loads((O/'park-path-network.json').read_text())
from shapely.geometry import LineString,Point
from shapely.ops import unary_union
bridges=unary_union([LineString([(v[0],v[1]) for v in path['points_cm']]).buffer(path['width_game_cm']/2+35) for path in network['paths'] if path['tags'].get('bridge')=='yes'])
verts=[];faces=[];gaps=0
for name in ['outer_cm','island_cm']:
 ring=out[name]
 for a,b in zip(ring,ring[1:]):
  length=np.linalg.norm(np.array(b[:2])-a[:2]);n=max(1,int(np.ceil(length/70)))
  for i in range(n):
   start=np.array(a[:2])+(np.array(b[:2])-a[:2])*i/n;end=np.array(a[:2])+(np.array(b[:2])-a[:2])*(i+1)/n
   if bridges.intersects(LineString([start,end])):gaps+=1;continue
   k=len(verts)+1
   verts.extend([(start[0],-start[1],z-300),(end[0],-end[1],z-300),(end[0],-end[1],z+120),(start[0],-start[1],z+120)])
   # Both windings ensure contact from either side, including island shore.
   faces.extend([(k,k+1,k+2),(k,k+2,k+3),(k+2,k+1,k),(k+3,k+2,k)])
lines=['# Solid shoreline: OpenStreetMap geometry, OBJ Y = -Unreal Y']+['v %.6f %.6f %.6f'%v for v in verts]+['f %d %d %d'%f for f in faces]
(O/'LakeShoreCollision.obj').write_text('\n'.join(lines)+'\n')
out['authored_bathymetry']={'max_depth_real_m':4.5,'source':'Authored underwater depth; no measured bathymetry available','modified_samples':int(np.count_nonzero(raw!=carved)),'shore_wall_triangles':len(faces),'bridge_gap_segments':gaps}
(O/'lake-clara-meer.json').write_text(json.dumps(out,indent=2));print(out['authored_bathymetry'])
