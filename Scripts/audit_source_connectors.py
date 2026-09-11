"""Find genuine OSM-node chains for old centerline components; never invent edges."""
from pathlib import Path
import json,heapq,math,collections
from pyproj import Transformer
P=Path(__file__).resolve().parents[1];O=P/'SourceAssets/Terrain';m=json.load(open(O/'terrain-georeference.json'));t=Transformer.from_crs('EPSG:4326',m['crs'],always_xy=True)
elements=json.load(open(P/'References/piedmont-osm.json'))['elements'];used={p['osm_id'] for p in json.load(open(O/'park-path-network.json'))['paths']};components=json.load(open(O/'park-graph-components.json'));nodes={};edges=collections.defaultdict(list);ways={}
for e in elements:
 tags=e.get('tags',{})
 if tags.get('highway') not in ['footway','cycleway','path','pedestrian','service'] or tags.get('access') in ['private','no'] or tags.get('private')=='yes':continue
 ways[e['id']]=tags
 for n,v in zip(e.get('nodes',[]),e.get('geometry',[])):nodes[n]=t.transform(v['lon'],v['lat'])
 for a,b in zip(e.get('nodes',[]),e.get('nodes',[])[1:]):
  length=math.dist(nodes[a],nodes[b]);edges[a].append((b,length,e['id']));edges[b].append((a,length,e['id']))
coord_to_nodes=collections.defaultdict(set)
for n,xy in nodes.items():coord_to_nodes[tuple(round(v,2) for v in xy)].add(n)
def source_nodes(c):return set().union(*(coord_to_nodes[tuple(xy)] for xy in c))
goals=source_nodes(components[0]);rows=[]
for i,c in enumerate(components[1:],1):
 starts=source_nodes(c);queue=[(0,n) for n in starts];heapq.heapify(queue);distance={n:0 for n in starts};parent={};found=None
 while queue:
  dist,n=heapq.heappop(queue)
  if dist!=distance[n]:continue
  if n in goals:found=n;break
  for nxt,cost,way in edges[n]:
   if dist+cost<distance.get(nxt,float('inf')):distance[nxt]=dist+cost;parent[nxt]=(n,way);heapq.heappush(queue,(dist+cost,nxt))
 chain=[];ids=[];end=found
 if found is not None:
  while found not in starts:
   prev,way=parent[found];chain.append(found);ids.append(way);found=prev
  chain.append(found);chain.reverse();ids.reverse()
 missing=list(dict.fromkeys(w for w in ids if w not in used))
 rows.append({'component':i,'node_count':len(c),'mapped_source_nodes':len(starts),'source_route_found':end is not None,'route_real_m':distance[end] if end is not None else None,'missing_way_ids':missing,'missing_ways':[{'id':w,'tags':ways[w]} for w in missing],'node_chain':chain,'edge_way_ids':ids,'points_utm':[nodes[n] for n in chain]})
result={'scope':'Exact shared-OSM-node connectivity. Routes may extend outside the clipped park; candidates require geographic and installed-surface review. No connecting geometry was fabricated.','components':rows};(O/'source-connector-audit.json').write_text(json.dumps(result,indent=2))
for r in rows:print(r['component'],r['source_route_found'],None if r['route_real_m'] is None else round(r['route_real_m']),r['missing_way_ids'])
