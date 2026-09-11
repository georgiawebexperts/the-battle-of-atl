import json,heapq,math
from pathlib import Path
from pyproj import Transformer
import numpy as np
root=Path(__file__).resolve().parents[1];data=json.loads((root/'References/eastside-krog-corridor-osm.json').read_text());meta=json.loads((root/'SourceAssets/Terrain/terrain-georeference.json').read_text());t=Transformer.from_crs(4326,32616,always_xy=True)
ways={e['id']:e for e in data['elements'] if e['type']=='way' and any(n in e.get('tags',{}).get('name','') for n in ['Eastside','Southeast'])};coords={};graph={}
for e in ways.values():
 for n,g in zip(e['nodes'],e['geometry']):coords[n]=t.transform(g['lon'],g['lat'])
 for a,b in zip(e['nodes'],e['nodes'][1:]):
  d=math.dist(coords[a],coords[b]);graph.setdefault(a,[]).append((b,d,e['id']));graph.setdefault(b,[]).append((a,d,e['id']))
start=6016404357;end=5718299534;dist={start:0};prev={};queue=[(0,start)]
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

from shapely.geometry import Point
road=next(e for e in data['elements'] if e['id']==44062162)
portals=[t.transform(p['lon'],p['lat']) for p in [road['geometry'][0],road['geometry'][-1]]]
stations=[line.project(Point(p)) for p in portals]
print('portal stations',stations,'route length',line.length)
for s in [stations[0]-15,stations[0]-5,stations[0],(stations[0]+stations[1])/2,stations[1],min(line.length,stations[1]+5),line.length]:print(round(s,3),round(height(s),3))
(root/'work/krog-source-chain.json').write_text(json.dumps({'start':start,'end':end,'length_real_m':line.length,'edges':[{'a':a,'b':b,'way':w} for a,b,w in chain],'portal_stations_real_m':stations},indent=2))
from route_height_profiles import RouteHeightProfiles
source_line=[[(coords[n][0]-ox)/.03,(coords[n][1]-oy)/.03] for n in [start]+[b for a,b,w in chain]]
lo=stations[0]-15;hi=line.length
profile={'bounds_padding_cm':1000,'kind':'tunnel','source_way':722838795,'portal_reference_way':44062162,'blend_start_cm':lo/.03,'bridge_start_cm':stations[0]/.03,'bridge_end_cm':stations[1]/.03,'blend_end_cm':hi/.03,'start_z_cm':height(lo),'end_z_cm':height(hi)}
profile_path=root/'SourceAssets/Terrain/krog-height-profiles.json'
profile_path.write_text(json.dumps({'centerline_xy_cm':source_line,'profiles':[profile],'policy':'Authored tunnel floor grade between bare-earth samples beyond both roadway portals. Full-height section extends beyond the single tunnel-tagged cycleway into its name-changing continuation. Bridge-named profile fields designate the fully overridden section for compatibility.'},indent=2)+'\n')
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
(root/'SourceAssets/Terrain/krog-route-network.json').write_text(json.dumps({'source':'OpenStreetMap contributors, ODbL','source_coordinate_system':'east/north/up cm; convert once at placement','paths':groups,'length_real_m':line.length,'start_node':start,'end_node':end,'height_profiles':'krog-height-profiles.json','width_policy':'Authored 320 game cm for arcade play','scope':'Irwin through the complete Krog tunnel sidewalk to its southern exit; Cabbagetown home still pending'},indent=2)+'\n')
print('Prepared Krog network:',len(groups),'parts')
