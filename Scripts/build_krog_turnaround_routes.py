"""Join both DeKalb lanes with slow, continuous road-end turnarounds."""
import json,math
from pathlib import Path
root=Path(__file__).resolve().parents[1]
# Reuse read-only source triangle sampling without executing the lane writer.
namespace={"__file__":str(root/'Scripts/build_krog_car_lanes.py')}
exec((root/'Scripts/build_krog_car_lanes.py').read_text().split('routes=[];probes=[];failures=[]')[0],namespace)
height=namespace['height'];folder=root/'SourceAssets/Terrain/KrogTraffic'
old=json.loads((folder/'car-lanes.json').read_text())['routes']
east=next(r['points_cm'] for r in old if r['name']=='eastbound');west=next(r['points_cm'] for r in old if r['name']=='westbound')
failures=[];probes=[]
def arc(incoming,outgoing,name):
    a,b=incoming[-1],outgoing[0];c=[(a[k]+b[k])/2 for k in range(2)];rad=[a[k]-c[k] for k in range(2)];radius=math.hypot(*rad)
    d=[a[k]-incoming[-2][k] for k in range(2)];norm=math.hypot(*d);d=[v/norm for v in d]
    assert abs(sum(rad[k]*d[k] for k in range(2)))<1
    result=[]
    for i in range(33):
        t=math.pi*i/32;x=c[0]+rad[0]*math.cos(t)+d[0]*radius*math.sin(t);y=c[1]+rad[1]*math.cos(t)+d[1]*radius*math.sin(t)
        h=height(x,y)
        if not h:failures.append({'turn':name,'sample':i,'kind':'centre'});continue
        result.append([x,y,h[0]])
        dx=-rad[0]*math.sin(t)+d[0]*radius*math.cos(t);dy=-rad[1]*math.sin(t)+d[1]*radius*math.cos(t);n=math.hypot(dx,dy);dx/=n;dy/=n
        for along in [-236,0,236]:
            for side in [-114,0,114]:
                px=x+dx*along-dy*side;py=y+dy*along+dx*side;floor=height(px,py)
                if not floor:failures.append({'turn':name,'sample':i,'kind':'body_corner','xy':[px,py]})
                else:probes.append({'xyz':[px,py,floor[0]],'surface':floor[1]})
    if len(result)==33:result[0]=a;result[-1]=b
    return result
turn_e=arc(east,west,'east');turn_w=arc(west,east,'west')
loop=east+turn_e[1:]+west[1:]+turn_w[1:]
report={'passed':not failures,'closed':loop[0]==loop[-1],'points_cm':loop,'speed_cm_s':250,'footprint_samples':probes,'failures':failures,'scope':'Source-triangle coverage of slow DeKalb end turnarounds. Native sweep, loop continuity, multi-car crossing behavior and visuals unverified. No saved map changed.'}
(folder/'turnaround-routes.json').write_text(json.dumps(report,indent=2)+'\n')
print(json.dumps({'passed':report['passed'],'closed':report['closed'],'points':len(loop),'samples':len(probes),'failures':failures[:8]}))
raise SystemExit(0 if report['passed'] and report['closed'] else 1)
