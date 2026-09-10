"""Prepare OSM path centerlines and graph diagnostics in the measured terrain CRS."""
from pathlib import Path
import json,math,collections,numpy as np
from pyproj import Transformer
from shapely.geometry import Polygon,LineString
from scipy.ndimage import map_coordinates
P=Path(__file__).resolve().parents[1];O=P/'SourceAssets/Terrain'
m=json.loads((O/'terrain-georeference.json').read_text());dem=np.load(O/'atlanta-elevation-north-up.npy')
t=Transformer.from_crs('EPSG:4326',m['crs'],always_xy=True)
ox,oy=m['origin_utm'];west,south,east,north=m['bounds_utm'];spacing=m['sample_spacing_real_m']
boundary=json.loads((P/'References/piedmont-boundary-osm.json').read_text())['elements'][0]
park=Polygon([t.transform(v['lon'],v['lat']) for v in boundary['geometry']])
elements=json.loads((P/'References/piedmont-osm.json').read_text())['elements']
def height(x,y):return float(map_coordinates(dem,[[(north-y)/spacing],[(x-west)/spacing]],order=1,mode='nearest')[0])
paths=[];adj=collections.defaultdict(set);excluded=collections.Counter()
for e in elements:
 tags=e.get('tags',{});kind=tags.get('highway','')
 if kind not in ['footway','cycleway','path','pedestrian','service']:continue
 if tags.get('area')=='yes' or tags.get('access') in ['private','no']:excluded['area_or_private']+=1;continue
 points=[t.transform(v['lon'],v['lat']) for v in e.get('geometry',[])]
 if len(points)<2:continue
 line=LineString(points)
 clipped=line.intersection(park.buffer(8))
 if clipped.is_empty:continue
 pieces=[clipped] if clipped.geom_type=='LineString' else [g for g in getattr(clipped,'geoms',[]) if g.geom_type=='LineString']
 for part in pieces:
  if part.length<2:continue
  # Keep every surveyed bend. Subdivide edges to <=2 real metres without changing centerline shape.
  raw=list(part.coords);samples=[]
  for a,b in zip(raw,raw[1:]):
   length=math.dist(a,b);n=max(1,math.ceil(length/2))
   for j in range(n):samples.append((a[0]+(b[0]-a[0])*j/n,a[1]+(b[1]-a[1])*j/n))
  samples.append(raw[-1])
  xyz=[[(x-ox)*100/3,(y-oy)*100/3,(height(x,y)-m['base_elevation_m'])*100/3+3] for x,y in samples]
  keys=[(round(x,2),round(y,2)) for x,y in raw]
  for a,b in zip(keys,keys[1:]):adj[a].add(b);adj[b].add(a)
  paths.append({'osm_id':e['id'],'name':tags.get('name','Park path'),'tags':tags,'length_real_m':part.length,'width_game_cm':320 if kind=='cycleway' else 280,'points_cm':xyz,'original_vertices_utm':raw})
seen=set();components=[]
for node in adj:
 if node in seen:continue
 todo=[node];seen.add(node);nodes=[]
 while todo:
  a=todo.pop();nodes.append(a)
  for b in adj[a]:
   if b not in seen:seen.add(b);todo.append(b)
 components.append(nodes)
components.sort(key=len,reverse=True)
result={'source':'OpenStreetMap contributors (ODbL)','source_file':'References/piedmont-osm.json','scale':1/3,'paths':paths,'park_boundary_cm':[[(x-ox)*100/3,(y-oy)*100/3] for x,y in park.exterior.coords],'diagnostics':{'path_count':len(paths),'length_real_m':sum(p['length_real_m'] for p in paths),'graph_nodes':len(adj),'component_sizes':[len(c) for c in components],'excluded':dict(excluded)},'notes':['Road widths remain rideable at human scale while distances and terrain scale 1:3.','Disconnected components require inspection; no artificial shortcut edges have been added.','Bridge elevation and safe water crossings require separate validation.']}
(O/'park-path-network.json').write_text(json.dumps(result,indent=2))
(O/'park-graph-components.json').write_text(json.dumps(components))
print(json.dumps(result['diagnostics']))
