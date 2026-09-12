"""Two mapped practice routes plus closed expansion street ends."""
import json,math,heapq,xml.etree.ElementTree as ET
from pathlib import Path
from shapely.geometry import LineString,Point
from shapely.ops import substring
root=Path(__file__).resolve().parents[1]
ns={'__file__':str(root/'Scripts/prepare_cabbagetown.py')};exec((root/'Scripts/prepare_cabbagetown.py').read_text().split('krog=json.loads')[0],ns);xy,height=ns['xy'],ns['height']
r=json.loads((root/'SourceAssets/Terrain/Tutorial/route.json').read_text())
x=ET.parse(root/'References/tutorial-block.osm').getroot();nodes={int(n.get('id')):xy({'lon':float(n.get('lon')),'lat':float(n.get('lat'))}) for n in x.findall('node')};ways=[]
names={'13th Street Northeast','14th Street Northeast','Juniper Street Northeast','Piedmont Avenue Northeast'}
for w in x.findall('way'):
 t={v.get('k'):v.get('v') for v in w.findall('tag')}
 if t.get('name') in names and t.get('highway') in {'primary','secondary','tertiary','residential','unclassified'}:ways.append((t['name'],[int(v.get('ref')) for v in w.findall('nd')]))
def graph(allowed):
 adj={}
 for n,seq in ways:
  if n not in allowed:continue
  for a,b in zip(seq,seq[1:]):
   d=math.dist(nodes[a],nodes[b]);adj.setdefault(a,[]).append((b,d));adj.setdefault(b,[]).append((a,d))
 return adj
adj=graph(names-{'Piedmont Avenue Northeast'});start=min(adj,key=lambda n:math.dist(nodes[n],r['road'][0][:2]));end=min(adj,key=lambda n:math.dist(nodes[n],r['gate'][:2]));q=[(0,start,[])];seen=set()
while q:
 cost,n,path=heapq.heappop(q)
 if n in seen:continue
 seen.add(n);path=path+[n]
 if n==end:break
 for b,d in adj[n]:heapq.heappush(q,(cost+d,b,path))
else:raise RuntimeError('No connected 13th-Juniper-14th route')
pts=[nodes[n] for n in path]+[tuple(r['gate'][:2])]
# OSM node spacing is uneven; vertex-only Chaikin passes leave tiny-radius
# bends at densely mapped intersections. Round by physical distance instead.
pts=list(LineString(pts).simplify(35,preserve_topology=False).coords)
rounded=[pts[0]]
for a,b,c in zip(pts,pts[1:],pts[2:]):
 incoming=math.dist(a,b);outgoing=math.dist(b,c)
 trim=min(650,incoming*.45,outgoing*.45)
 entry=tuple(b[j]+(a[j]-b[j])*trim/incoming for j in range(2))
 leave=tuple(b[j]+(c[j]-b[j])*trim/outgoing for j in range(2))
 rounded.append(entry)
 for step in range(1,25):
  t=step/24
  rounded.append(tuple((1-t)**2*entry[j]+2*(1-t)*t*b[j]+t*t*leave[j] for j in range(2)))
pts=rounded+[pts[-1]]
def sampled(pts):
 line=LineString(pts);out=[]
 for i in range(math.ceil(line.length/100)+1):
  a,b=line.interpolate(min(i*100,line.length)).coords[0];out.append([a,b,height(a,b)+12])
 return out
r['alternate']=sampled(pts)
def corner(one,two):
 a={n for name,s in ways if name==one for n in s};b={n for name,s in ways if name==two for n in s};both=a&b;assert both,(one,two);return nodes[min(both,key=lambda n:math.dist(nodes[n],r['gate'][:2]))]
j13=corner('13th Street Northeast','Juniper Street Northeast');j14=corner('14th Street Northeast','Juniper Street Northeast');p13=corner('13th Street Northeast','Piedmont Avenue Northeast');p14=corner('14th Street Northeast','Piedmont Avenue Northeast')
# Extend the existing south branch along mapped Piedmont Avenue to the market.
market_osm=ET.parse(root/'References/twelfth-market.osm').getroot()
market_nodes={int(n.get('id')):xy({'lon':float(n.get('lon')),'lat':float(n.get('lat'))}) for n in market_osm.findall('node')}
avenue_way=next(w for w in market_osm.findall('way') if w.get('id')=='9272986')
avenue=LineString([market_nodes[int(n.get('ref'))] for n in avenue_way.findall('nd')])
market_gate=market_nodes[316638596];south_stop=Point(market_gate[0],market_gate[1]+700)
market_branch=list(substring(avenue,avenue.project(Point(p13)),avenue.project(south_stop)).coords)
assert math.dist(market_branch[0],p13)<50
market_branch[0]=p13
r['market_approach']=sampled(market_branch)
r['market_approach_source']='OSM way9272986 and gate316638596; authored expansion stop700cm south of gate'
barriers=[];stubs=[]
for p,d in [(j13,(-1,0)),(j13,(0,1)),(j14,(-1,0)),(j14,(0,-1)),(p13,(0,1)),(p14,(0,-1))]:
 if p==p13 and d==(0,1):
  stub=r['market_approach'];dx,dy=stub[-1][0]-stub[-2][0],stub[-1][1]-stub[-2][1];length=math.hypot(dx,dy);d=(dx/length,dy/length)
 else:
  end=(p[0]+d[0]*650,p[1]+d[1]*650);stub=sampled([p,end])
 stubs+=list(zip(stub,stub[1:]));barriers.append({'center':stub[-1],'outward':[d[0],d[1],0]})
r['barriers']=barriers;r['stubs']=stubs;r['alternate_length']=LineString([p[:2] for p in r['alternate']]).length;r['source']+='; alternate from OSM /api/0.6/map bbox -84.383,33.7852,-84.3778,33.7868';r['policy']+='; both street approaches enabled for game practice; street one-way traffic restrictions are not a real-world navigation recommendation'
(root/'SourceAssets/Terrain/Tutorial/block.json').write_text(json.dumps(r,indent=2)+'\n')
h=['#pragma once','namespace BattleTutorialBlock {','inline const FVector Alternate[] = {']+['FVector('+','.join(f'{v:.5f}' for v in p)+'),' for p in r['alternate']]+['};','inline const FVector StubEnds[] = {']+['FVector('+','.join(f'{v:.5f}' for v in p)+'),' for pair in stubs for p in pair]+['};']
for n,k in [('BarrierCenters','center'),('BarrierDirections','outward')]:h+=['inline const FVector '+n+'[] = {']+['FVector('+','.join(f'{v:.5f}' for v in b[k])+'),' for b in barriers]+['};']
h+=['inline const FVector MarketApproach[] = {']+['FVector('+','.join(f'{v:.5f}' for v in p)+'),' for p in r['market_approach'][:-4]]+['};']
h+=['}'];(root/'Source/AuraPlayground/BattleTutorialBlock.h').write_text('\n'.join(h)+'\n');print({'alternate_points':len(r['alternate']),'alternate_length':r['alternate_length'],'barriers':len(barriers),'corners':[j13,j14,p13,p14]})
