import json,heapq,math
from pathlib import Path
from pyproj import Transformer
import numpy as np
root=Path(__file__).resolve().parents[1];data=json.loads((root/'References/eastside-krog-corridor-osm.json').read_text());meta=json.loads((root/'SourceAssets/Terrain/terrain-georeference.json').read_text());t=Transformer.from_crs(4326,32616,always_xy=True)
ways={e['id']:e for e in data['elements'] if e['type']=='way' and 'Eastside' in e.get('tags',{}).get('name','')};coords={};graph={}
for e in ways.values():
 for n,g in zip(e['nodes'],e['geometry']):coords[n]=t.transform(g['lon'],g['lat'])
 for a,b in zip(e['nodes'],e['nodes'][1:]):
  d=math.dist(coords[a],coords[b]);graph.setdefault(a,[]).append((b,d,e['id']));graph.setdefault(b,[]).append((a,d,e['id']))
start=2396740017;end=6016404357;dist={start:0};prev={};queue=[(0,start)]
while queue:
 cost,a=heapq.heappop(queue)
 if a==end:break
 if cost!=dist[a]:continue
 for b,d,w in graph.get(a,[]):
  if cost+d<dist.get(b,1e99):dist[b]=cost+d;prev[b]=(a,w);heapq.heappush(queue,(cost+d,b))
assert end in dist
chain=[];n=end
while n!=start:
 a,w=prev[n];chain.append((a,n,w));n=a
chain.reverse()
print('length',dist[end],'ways',list(dict.fromkeys(e[2] for e in chain)))
from shapely.geometry import LineString
line=LineString([coords[start]]+[coords[b] for a,b,w in chain]);raw=np.fromfile(root/'SourceAssets/Terrain/atlanta-height.r16',dtype='<u2').reshape(meta['size'][1],meta['size'][0]);ox,oy=meta['origin_utm'];loc=meta['unreal_location_cm'];sc=meta['unreal_scale']
def height(s):
 x,y=line.interpolate(s).coords[0];x=(x-ox)/.03;y=(y-oy)/.03;gx=(x-loc[0])/sc[0];gy=(y-loc[1])/sc[1];ix=int(gx);iy=int(gy);dx=gx-ix;dy=gy-iy
 z00,z10,z01,z11=[(float(raw[yy,xx])-32768)*sc[2]/128 for xx,yy in [(ix,iy),(ix+1,iy),(ix,iy+1),(ix+1,iy+1)]]
 return (z00+(z10-z00)*dx+(z11-z10)*dy if dx>=dy else z00+(z11-z01)*dx+(z01-z00)*dy)+3
station=0;bridges=[]
for a,b,w in chain:
 length=math.dist(coords[a],coords[b])
 if ways[w]['tags'].get('bridge')=='yes':bridges.append({'osm_id':w,'start_m':station,'end_m':station+length,'heights_cm':[[round(s,2),round(height(s),2)] for s in [station-12,station,station+length/2,station+length,station+length+12]]})
 station+=length
print(json.dumps(bridges,indent=2))
(root/'work/eastside-source-chain.json').write_text(json.dumps({'start':start,'end':end,'length_real_m':dist[end],'edges':[{'a':a,'b':b,'way':w} for a,b,w in chain],'bridges':bridges},indent=2))

from route_height_profiles import RouteHeightProfiles
source_line=[[(coords[n][0]-ox)/.03,(coords[n][1]-oy)/.03] for n in [start]+[b for a,b,w in chain]]
profiles=[]
for b in bridges:
 lo=b['start_m']-12;hi=b['end_m']+12
 profiles.append({'osm_id':b['osm_id'],'blend_start_cm':lo/.03,'bridge_start_cm':b['start_m']/.03,'bridge_end_cm':b['end_m']/.03,'blend_end_cm':hi/.03,'start_z_cm':height(lo),'end_z_cm':height(hi)})
profile_path=root/'SourceAssets/Terrain/eastside-height-profiles.json'
profile_path.write_text(json.dumps({'centerline_xy_cm':source_line,'profiles':profiles,'policy':'Authored straight deck grade between bare-earth samples 12 real metres beyond each OSM bridge end, with smooth terrain blends on the approaches; not surveyed bridge heights.'},indent=2)+'\n')
overrides=RouteHeightProfiles(profile_path)
groups=[];station=0
for a,b,w in chain:
 if not groups or groups[-1]['osm_id']!=w:groups.append({'osm_id':w,'nodes':[a],'tags':ways[w]['tags'],'start_station_real_m':station})
 groups[-1]['nodes'].append(b);station+=math.dist(coords[a],coords[b])
for part,g in enumerate(groups):
 local=LineString([coords[n] for n in g['nodes']]);count=max(2,math.ceil(local.length/2)+1);points=[]
 for i in range(count):
  s=g['start_station_real_m']+local.length*i/(count-1);x,y=line.interpolate(s).coords[0];x=(x-ox)/.03;y=(y-oy)/.03
  points.append([x,y,overrides.height(x,y,height(s))])
 g.update(points_cm=points,width_game_cm=320,artifact_eligible=False,length_real_m=local.length,route_part=part)
result={'source':'OpenStreetMap contributors, ODbL','source_coordinate_system':'east/north/up cm; use battle_geography.py for Unreal placement','paths':groups,'length_real_m':line.length,'start_node':start,'end_node':end,'width_policy':'Authored 320 game cm for arcade play; not a surveyed width.','height_profiles':'eastside-height-profiles.json','scope':'Monroe to Irwin, including three authored bridge profiles. Tunnel and Cabbagetown remain unfinished.'}
(root/'SourceAssets/Terrain/eastside-trail-network.json').write_text(json.dumps(result,indent=2)+'\n')
print('Prepared',len(groups),'parts',line.length,'real metres')
