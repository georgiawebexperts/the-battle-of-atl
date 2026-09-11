"""Sourced 13th Street / Piedmont Avenue approach; fictional bungalow setback."""
from pathlib import Path
root=Path(__file__).resolve().parents[1]
ns={'__file__':str(root/'Scripts/prepare_cabbagetown.py')};exec((root/'Scripts/prepare_cabbagetown.py').read_text().split('krog=json.loads')[0],ns)
import json,math
from shapely.geometry import LineString,Point
xy,height=ns['xy'],ns['height'];osm=json.loads((root/'References/tutorial-streets-osm.json').read_text());ways={e['id']:e for e in osm['elements']}
pts=[xy(g) for g in ways[673535177]['geometry']]
avenue=[xy(g) for g in ways[9272986]['geometry']];j=min(range(len(avenue)),key=lambda i:math.dist(avenue[i],pts[-1]));pts+=avenue[j+1:]
for id in [431571139,1035224026]:pts += [xy(g) for g in ways[id]['geometry'][1:]]
gate_json=json.loads((root/'SourceAssets/Terrain/battle-start.json').read_text());gate=[gate_json['gate_xy_cm'][0],-gate_json['gate_xy_cm'][1]]
# A rideable right turn into the gate, with a 450-cm radius rather than a sharp corner.
center=(gate[0]-200,gate[1]+450)
arc_start=(center[0]-450,center[1])
pts=[p for p in pts if p[1]>arc_start[1]+180]+[arc_start]
for i in range(1,25):
 angle=math.pi*.5*i/24
 pts.append((center[0]-450*math.cos(angle),center[1]-450*math.sin(angle)))
pts.append(tuple(gate))
line=LineString(pts);road=[]
for i in range(math.ceil(line.length/100)+1):
 x,y=line.interpolate(min(i*100,line.length)).coords[0];road.append([x,y,height(x,y)+12])
home=[road[0][0],road[0][1]+1000];home+=[height(*home)+12];gate3=[*gate,height(*gate)+12]
r={'source':'OpenStreetMap contributors ODbL; 13th Street way 673535177 and Piedmont Avenue 9272986,431571139,1035224026; gate node 5674178517; retained USGS terrain','policy':'Fictional bungalow, no private address or owner depicted; free practice approach to 14th Street','road':road,'home':home,'gate':gate3,'length':line.length}
out=root/'SourceAssets/Terrain/Tutorial';out.mkdir(exist_ok=True);(out/'route.json').write_text(json.dumps(r,indent=2)+'\n')
h=['#pragma once','namespace BattleTutorialData {','inline const FVector Road[] = {']+['FVector('+','.join(f'{v:.5f}' for v in p)+'),' for p in road]+['};']
for n,k in [('Home','home'),('Gate','gate')]:h+=['inline const FVector '+n+'('+','.join(f'{v:.5f}' for v in r[k])+');']
h+=['}'];(root/'Source/AuraPlayground/BattleTutorialData.h').write_text('\n'.join(h)+'\n');print({'points':len(road),'length':line.length,'start':road[0],'home':home,'gate':gate3})
