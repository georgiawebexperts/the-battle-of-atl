"""Plan a lake circuit, alternate-bank exit and shore walkback from saved geometry."""
from pathlib import Path
import json,math
from shapely.geometry import Polygon,LineString,Point
r=Path(__file__).resolve().parents[1]
inventory=json.loads((r/'Tests/Results/2026-09-13-swim-lake-inventory.json').read_text())
outer=next(x['polygon'] for x in inventory if x['class']=='PiedmontWaterHazard')
poly=Polygon([(x,y) for x,y,z in outer]);inside=poly.buffer(-400);outside=poly.buffer(250)
assert inside.geom_type=='Polygon' and outside.geom_type=='Polygon'
entry=(-12611.248,5129.596);exit_bank=(-4650.458,939.074)
def samples(shape):
 line=LineString(shape.exterior.coords);n=math.ceil(line.length/200)
 return [(line.interpolate(i*line.length/n).x,line.interpolate(i*line.length/n).y) for i in range(n)]
def nearest(points,p):return min(range(len(points)),key=lambda i:math.dist(points[i],p))
def between(points,a,b):
 forward=[points[(a+i)%len(points)] for i in range((b-a)%len(points)+1)]
 backward=[points[(a-i)%len(points)] for i in range((a-b)%len(points)+1)]
 return min([forward,backward],key=lambda ps:sum(math.dist(a,b) for a,b in zip(ps,ps[1:])))
water=samples(inside);start=nearest(water,entry);end=nearest(water,exit_bank)
circuit=[water[(start+i)%len(water)] for i in range(len(water)+1)]
swim=circuit+between(water,start,end)[1:]
land=samples(outside);walk=[exit_bank]+between(land,nearest(land,exit_bank),nearest(land,entry))+[entry]
# The walking driver must steer around the retained mature tree at(-13960,2898).
# These west-bank detour points remain outside the lake; native clearance is verified by the run.
insert=nearest(walk,(-14063.332,3042.112))
walk[insert:insert]=[(-14300,2850),(-14300,3170)]
points=[{'x':x,'y':y,'swim':True} for x,y in swim]+[{'x':x,'y':y,'swim':False} for x,y in walk]
result={'description':'Full inner shoreline circuit, then alternate-bank exit and shortest outer shoreline walkback; native collision validation required.','points':points,'swim_points':len(swim),'walk_points':len(walk),'planned_cm':sum(math.dist((a['x'],a['y']),(b['x'],b['y'])) for a,b in zip(points,points[1:]))}
out=r/'Tests/Fixtures/swim-exploration.json';out.parent.mkdir(exist_ok=True);out.write_text(json.dumps(result,indent=2)+'\n');print({k:v for k,v in result.items() if k!='points'})
