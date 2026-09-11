"""Retained OSM Wylie Street + 3DEP terrain; 98 Estoria fictional bar finish at the referenced Estoria corner."""
import json,math
from pathlib import Path
import numpy as np
from pyproj import Transformer
from shapely.geometry import LineString,Point,box
root=Path(__file__).resolve().parents[1]
meta=json.loads((root/'SourceAssets/Terrain/terrain-georeference.json').read_text())
osm=json.loads((root/'References/eastside-krog-corridor-osm.json').read_text())
way=next(e for e in osm['elements'] if e['type']=='way' and e['id']==9242978)
t=Transformer.from_crs(4326,32616,always_xy=True)
ox,oy=meta['origin_utm'];loc=meta['unreal_location_cm'];scale=meta['unreal_scale']
def xy(g):
 x,y=t.transform(g['lon'],g['lat']);return ((x-ox)/.03,-(y-oy)/.03)
line=LineString([xy(g) for g in way['geometry']])
raw=np.fromfile(root/'SourceAssets/Terrain/atlanta-height-krog.r16',dtype='<u2').reshape(meta['size'][1],meta['size'][0])
def height(x,y):
 gx=(x-loc[0])/scale[0];gy=(-y-loc[1])/scale[1];ix=int(gx);iy=int(gy);dx=gx-ix;dy=gy-iy
 assert 0<=ix<raw.shape[1]-1 and 0<=iy<raw.shape[0]-1
 z00,z10,z01,z11=[(float(raw[yy,xx])-32768)*scale[2]/128 for xx,yy in [(ix,iy),(ix+1,iy),(ix,iy+1),(ix+1,iy+1)]]
 return z00+(z10-z00)*dx+(z11-z10)*dy if dx>=dy else z00+(z11-z01)*dx+(z01-z00)*dy
krog=json.loads((root/'SourceAssets/Terrain/krog-route-network.json').read_text())
last=krog['paths'][-1]['points_cm'][-1];start=[last[0],-last[1],last[2]]
join=line.project(Point(start[:2]));bar_xy=xy({'lat':33.7520664,'lon':-84.3633698});finish_station=line.project(Point(bar_xy))-950
assert join>finish_station
# Road reaches the western retained intersection; quest diverts to a fictional gate earlier.
roadxy=[start[:2],list(line.interpolate(join).coords[0])]+[list(line.interpolate(join*(1-i/100)).coords[0]) for i in range(1,101)]
road=LineString(roadxy)
points=[]
for i in range(math.ceil(road.length/120)+1):
 x,y=road.interpolate(min(i*120,road.length)).coords[0];z=height(x,y)+12
 # Blend the existing tunnel exit, which has an authored floor above the DEM.
 d=min(i*120,road.length);blend=max(0,1-d/600);z=z*(1-blend)+start[2]*blend
 points.append([x,y,z])
a=np.array(line.interpolate(finish_station).coords[0]);tangent=np.array(line.interpolate(finish_station+10).coords[0])-a;tangent/=np.linalg.norm(tangent);south=np.array([-tangent[1],tangent[0]])
gate=a+south*500;home=np.array(bar_xy)+south*200
# Round the tunnel-to-Wylie turn in the riding line, staying within the
# existing 450-cm pavement. The return route must be rideable too.
corner=np.array(line.interpolate(join).coords[0])
radius=400
turn_center=corner-tangent*radius-south*radius
route=[start]
for i in range(25):
 angle=math.pi*.5*i/24
 x,y=turn_center+tangent*radius*math.cos(angle)+south*radius*math.sin(angle)
 z=height(x,y)+12
 d=math.dist((x,y),start[:2]);blend=max(0,1-d/600);z=z*(1-blend)+start[2]*blend
 route.append([float(x),float(y),z])
route.extend(p for p in points if finish_station+350<=line.project(Point(p[:2]))<join-radius)
approach=[]
center=a+tangent*350+south*350
for i in range(17):
 angle=math.pi*.5*i/16;x,y=center-south*350*math.cos(angle)-tangent*350*math.sin(angle);approach.append([float(x),float(y),height(x,y)+12])
for distance in [425,500]:
 x,y=a+south*distance;approach.append([float(x),float(y),height(x,y)+12])
route.extend(approach)
# The reference point is not a surveyed footprint. Reserve a clear bicycle
# corridor past the fictional frontage, including its door and entrance step.
local_route=LineString([(float(np.dot(np.array(p[:2])-home,tangent)),float(np.dot(np.array(p[:2])-home,south))) for p in route])
for name,bounds in [('walls',(-305,-400,305,400)),('door',(-67,-426,67,-402)),('step',(-95,-520,95,-400))]:
 clearance=local_route.distance(box(*bounds))
 assert clearance>=64,(name,clearance)

# Grade a level patio above the retained DEM rather than letting higher
# terrain intersect the slab. Approach rises smoothly before the slab edge.
patio_ground_max=max(height(*(home+tangent*x+south*y)) for x in range(-1140,-359,5) for y in range(-370,731,5))
patio_height=patio_ground_max+16
for p in approach:
 local_y=float(np.dot(np.array(p[:2])-home,south))
 blend=max(0,min(1,(local_y+609.4)/239.4))
 p[2]=p[2]*(1-blend)+patio_height*blend
max_approach_grade=max(abs(b[2]-a[2])/math.dist(a[:2],b[:2]) for a,b in zip(approach,approach[1:]))
assert max_approach_grade<.25,max_approach_grade
gate3=[*map(float,gate),patio_height]
home3=[*map(float,home),height(*home)+12]
profile=json.loads((root/'SourceAssets/Terrain/krog-height-profiles.json').read_text());entryxy=LineString(profile['centerline_xy_cm']).interpolate(profile['profiles'][0]['bridge_start_cm']).coords[0]
allk=[p for part in krog['paths'] for p in part['points_cm']];entry=min(allk,key=lambda p:math.dist(p[:2],entryxy));entry=[entry[0],-entry[1],entry[2]]
result={'source':'OpenStreetMap contributors ODbL; retained way 9242978; retained USGS 3DEP terrain','frame':'Unreal ESU centimeters; north is -Y','home_policy':'Fictional 98 Estoria; real 97 Estoria location from On the Grid map link, 33.7520664,-84.3633698; patio approach adapted for riding','patio_ground_max':patio_ground_max,'patio_height':patio_height,'max_approach_grade':max_approach_grade,'road':points,'route':route,'approach':approach,'gate':gate3,'home':home3,'south_direction':list(map(float,south)),'tunnel_entry':entry,'tunnel_exit':start}
out=root/'SourceAssets/Terrain/Cabbagetown';out.mkdir(exist_ok=True);(out/'route.json').write_text(json.dumps(result,indent=2)+'\n')
header=['#pragma once','// Generated by prepare_cabbagetown.py. OSM + retained terrain; fictional 98 Estoria bar.','namespace BattleHomeData {']
for name,key in [('Road','road'),('Route','route'),('Approach','approach')]:header+=['inline const FVector '+name+'[] = {']+[' FVector('+','.join(f'{v:.5f}' for v in p)+'),' for p in result[key]]+['};']
for name,key in [('Gate','gate'),('Home','home'),('TunnelEntry','tunnel_entry'),('TunnelExit','tunnel_exit')]:header+=['inline const FVector '+name+'('+','.join(f'{v:.5f}' for v in result[key])+');']
header+=['inline const FVector South('+','.join(f'{v:.5f}' for v in [*south,0])+');','}'];(root/'Source/AuraPlayground/BattleHomeData.h').write_text('\n'.join(header)+'\n')
print(json.dumps({'road_points':len(points),'route_points':len(route),'road_length_cm':road.length,'gate':gate3,'home':home3,'tunnel_entry':entry,'tunnel_exit':start}))
