import json,heapq,math
from pathlib import Path
from pyproj import Transformer
root=Path(__file__).resolve().parents[1];d=json.loads((root/'References/piedmont-osm.json').read_text());meta=json.loads((root/'SourceAssets/Terrain/terrain-georeference.json').read_text());t=Transformer.from_crs(4326,32616,always_xy=True)
coords={};graph={};ways={}
for e in d['elements']:
 tags=e.get('tags',{})
 if e['type']!='way' or tags.get('highway') not in ['footway','path','cycleway','steps','pedestrian'] or tags.get('area')=='yes' or tags.get('access') in ['private','no'] or tags.get('bicycle')=='no':continue
 ways[e['id']]=e
 for n,p in zip(e['nodes'],e['geometry']):coords[n]=t.transform(p['lon'],p['lat'])
 for a,b in zip(e['nodes'],e['nodes'][1:]):
  length=math.dist(coords[a],coords[b]);graph.setdefault(a,[]).append((b,length,e['id']));graph.setdefault(b,[]).append((a,length,e['id']))
anchor=[11322.241115950359,-8938.93870107519];anchor=[meta['origin_utm'][i]+anchor[i]*.03 for i in [0,1]]
from shapely.geometry import LineString,Point
installed=json.loads((root/'SourceAssets/Terrain/park-path-network.json').read_text())['paths']
installed_lines=[LineString([v[:2] for v in path['points_cm']]) for path in installed]
def overlap(n):
 p=Point(*[(coords[n][i]-meta['origin_utm'][i])*100*meta['scale'] for i in [0,1]])
 return min(line.distance(p) for line in installed_lines)
eligible=[n for n in ways[182302109]['nodes'] if overlap(n)<1]
assert eligible
start=min(eligible,key=lambda n:math.dist(coords[n],anchor));end=2396740017
print('start',start,'distance_m',math.dist(coords[start],anchor),'end',end)
heap=[(0,start)];dist={start:0};prev={}
while heap:
 cost,a=heapq.heappop(heap)
 if a==end:break
 if cost!=dist[a]:continue
 for b,l,w in graph.get(a,[]):
  if cost+l<dist.get(b,1e99):dist[b]=cost+l;prev[b]=(a,w);heapq.heappush(heap,(cost+l,b))
if end not in dist:print('NO ROUTE; start component nodes',len(dist));raise SystemExit(1)
chain=[];n=end
while n!=start:
 a,w=prev[n];chain.append({'a':a,'b':n,'way':w});n=a
chain.reverse();print('route_m',dist[end],'edges',len(chain),'ways',list(dict.fromkeys(c['way'] for c in chain)))
for wid in dict.fromkeys(c['way'] for c in chain):print(wid,ways[wid]['tags'])
(root/'work/connector-source-chain.json').write_text(json.dumps({'start':start,'end':end,'length_real_m':dist[end],'edges':chain},indent=2))

import numpy as np
raw=np.fromfile(root/'SourceAssets/Terrain/atlanta-height.r16',dtype='<u2').reshape(meta['size'][1],meta['size'][0])
loc=meta['unreal_location_cm'];scale=meta['unreal_scale']
def world_point(utm):
 x=(utm[0]-meta['origin_utm'][0])*100*meta['scale'];y=(utm[1]-meta['origin_utm'][1])*100*meta['scale']
 gx=(x-loc[0])/scale[0];gy=(y-loc[1])/scale[1];ix=int(gx);iy=int(gy);dx=gx-ix;dy=gy-iy
 assert 0<=ix<raw.shape[1]-1 and 0<=iy<raw.shape[0]-1
 z00=(int(raw[iy,ix])-32768)*scale[2]/128
 z10=(int(raw[iy,ix+1])-32768)*scale[2]/128
 z01=(int(raw[iy+1,ix])-32768)*scale[2]/128
 z11=(int(raw[iy+1,ix+1])-32768)*scale[2]/128
 z=z00+(z10-z00)*dx+(z11-z10)*dy if dx>=dy else z00+(z11-z01)*dx+(z01-z00)*dy
 return [x,y,z+3]
from shapely.geometry import LineString,Point
installed=json.loads((root/'SourceAssets/Terrain/park-path-network.json').read_text())['paths']
start_world=world_point(coords[start])
distance=min(LineString([p[:2] for p in path['points_cm']]).distance(Point(start_world[:2])) for path in installed)
assert distance<1, f'Connector start does not overlap installed centerline: {distance}'
groups=[]
for edge in chain:
 if not groups or groups[-1]['osm_id']!=edge['way']:
  groups.append({'osm_id':edge['way'],'nodes':[edge['a']],'tags':dict(ways[edge['way']]['tags'])})
 groups[-1]['nodes'].append(edge['b'])
for group in groups:
 line=LineString([coords[n] for n in group['nodes']]);count=max(2,int(math.ceil(line.length/2))+1)
 group['points_cm']=[world_point(line.interpolate(line.length*i/(count-1)).coords[0]) for i in range(count)]
 group['length_real_m']=line.length;group['width_game_cm']=320;group['artifact_eligible']=False
 group['name']=group['tags'].get('name','Monroe trail crossing')
result={'source':'OpenStreetMap contributors, ODbL','paths':groups,'length_real_m':dist[end],
 'start_overlap_error_game_cm':distance,'width_policy':'Authored 320 game cm to match the existing arcade trail width; not a surveyed width.',
 'scope':'Park trail to northern Eastside endpoint only; full route remains unfinished'}
(root/'SourceAssets/Terrain/beltline-connector-network.json').write_text(json.dumps(result,indent=2)+'\n')
print('Connector prepared:',result['length_real_m'],'real metres; start overlap',distance)
