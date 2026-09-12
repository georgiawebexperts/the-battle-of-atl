"""Park-side return from the market fence toward the 14th Street entry opening."""
from pathlib import Path
import json,math
from shapely.geometry import Polygon,Point,LineString
from shapely.ops import substring,unary_union
root=Path(__file__).resolve().parents[1]
park=Polygon(json.loads((root/'SourceAssets/Terrain/park-region.json').read_text())['outline']);boundary=park.exterior
gate=json.loads((root/'SourceAssets/Terrain/Tutorial/route.json').read_text())['gate']
start=(-19107.993785,3187.443844-800)
crossing=boundary.intersection(LineString([(-30000,gate[1]+400),(0,gate[1]+400)]))
finish=crossing if crossing.geom_type=='Point' else min(crossing.geoms,key=lambda p:p.x)
near=boundary.interpolate(boundary.project(Point(start)))
section=substring(boundary,boundary.project(near),boundary.project(finish))
assert section.length<10000
line=LineString([start]+list(section.coords));pts=[list(line.interpolate(min(i*40,line.length)).coords[0]) for i in range(math.ceil(line.length/40)+1)]
# The playable road is widened/rounded relative to the mapped centreline.
# Keep the temporary fence on its park side, outside its225cm half-width.
tutorial=json.loads((root/'SourceAssets/Terrain/Tutorial/block.json').read_text())
roads=unary_union([LineString([v[:2] for v in tutorial[key]]) for key in ['road','alternate','market_approach']])
offsets=[]
for p in pts:
 original=p[0]
 while roads.distance(Point(p))<310:p[0]+=10
 offsets.append(p[0]-original)
# Join the south stone pier rather than leaving an open walk-around end.
# Its centre is360cm south of the gate, outside the270cm timer opening.
pier=(gate[0],gate[1]+360)
connection=LineString([pts[-1],pier])
assert connection.distance(roads)>300
for i in range(1,math.ceil(connection.length/40)+1):
 pts.append(list(connection.interpolate(min(i*40,connection.length)).coords[0]))
assert math.dist(pts[-1],pier)<.001
assert max(offsets)<1500
assert LineString(pts).distance(roads)>300
assert len(pts)>100
r={'source':'Retained OSM park polygon; authored fence tracing nearest boundary, joins14th gate south stone pier via road-cleared return','points_xy':pts,'length_cm':LineString(pts).length,'maximum_parkward_adjustment_cm':max(offsets),'road_clearance_cm':LineString(pts).distance(roads),'status':'Candidate closure return; native ground and route tests required','start':start,'finish':list(pier),'connection_length_cm':connection.length}
(root/'SourceAssets/Terrain/TwelfthMarket/return-fence.json').write_text(json.dumps(r,indent=2)+'\n')
h=['#pragma once','namespace BattleMarketReturn {','inline const FVector2D Points[]={']+[f'FVector2D({x:.6f},{y:.6f}),' for x,y in pts]+['};','}'];(root/'Source/AuraPlayground/BattleMarketReturn.h').write_text('\n'.join(h)+'\n');print({'length_cm':line.length,'posts':len(pts),'finish':r['finish']})
