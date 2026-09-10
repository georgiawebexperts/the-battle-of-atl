from pathlib import Path
import json,math,numpy as np
from shapely.geometry import Polygon,LineString,Point
P=Path(__file__).resolve().parents[1];O=P/'SourceAssets/Terrain';l=json.loads((O/'lake-clara-meer.json').read_text());m=json.loads((O/'terrain-georeference.json').read_text());d=json.loads((O/'park-path-network.json').read_text());raw=np.fromfile(O/'atlanta-height-lakebed.r16',dtype='<u2').reshape(m['size'][1],m['size'][0])
poly=Polygon([v[:2] for v in l['outer_cm']],[[v[:2] for v in l['island_cm']]]);island=Polygon([v[:2] for v in l['island_cm']]);water=poly.representative_point();ip=island.representative_point()
def z(x,y):
 gx=(x-m['unreal_location_cm'][0])/m['unreal_scale'][0];gy=(y-m['unreal_location_cm'][1])/m['unreal_scale'][1];ix=int(gx);iy=int(gy);u=gx-ix;v=gy-iy;a=float(raw[iy,ix]);b=float(raw[iy,ix+1]);c=float(raw[iy+1,ix+1]);dd=float(raw[iy+1,ix]);h=a+(b-a)*u+(c-b)*v if v<=u else a+(c-dd)*u+(dd-a)*v;return (h-32768)*m['unreal_scale'][2]/128
edges=[]
for path in d['paths']:
 if path['tags'].get('bridge')=='yes':continue
 for a,b in zip(path['original_vertices_utm'],path['original_vertices_utm'][1:]):
  pa=[(a[i]-m['origin_utm'][i])*100/3 for i in range(2)];pb=[(b[i]-m['origin_utm'][i])*100/3 for i in range(2)];mid=[(pa[i]+pb[i])/2 for i in range(2)]
  if poly.distance(Point(mid))>1000:edges.append((math.dist(pa,pb),pa,pb,mid))
_,a,b,mid=max(edges,key=lambda e:e[0]);safe=[*mid,z(*mid)+100];yaw=math.degrees(math.atan2(b[1]-a[1],b[0]-a[0]))
ring=l['outer_cm'];candidates=[]
for a,b in zip(ring,ring[1:]):
 v=np.array(b[:2])-a[:2];length=np.linalg.norm(v)
 if length<120:continue
 n=np.array([-v[1],v[0]])/length;mid=(np.array(a[:2])+b[:2])/2
 if not poly.contains(Point(mid+n*10)):n=-n
 start=mid-n*220
 if any(LineString([v[:2] for v in path['points_cm']]).distance(Point(mid))<path['width_game_cm']/2+100 for path in d['paths'] if path['tags'].get('bridge')=='yes'):continue
 candidates.append((length,start,n))
_,start,n=max(candidates,key=lambda e:e[0]);out={'safe_start':safe,'safe_yaw':yaw,'shore_start':[float(start[0]),float(start[1]),z(*start)+100],'shore_yaw':math.degrees(math.atan2(n[1],n[0])),'water_drop':[water.x,water.y,l['water_z_cm']+600],'island_stand':[ip.x,ip.y,z(ip.x,ip.y)+100]};(O/'lake-test-points.json').write_text(json.dumps(out,indent=2));print(out)
