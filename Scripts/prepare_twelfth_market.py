"""Prepare a sourced 12th Street market layout for review, not installation."""
from pathlib import Path
import json,math,xml.etree.ElementTree as ET
from pyproj import Transformer
from shapely.geometry import Point,Polygon,LineString
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
root=Path(__file__).resolve().parents[1];out=root/'SourceAssets/Terrain/TwelfthMarket';out.mkdir(exist_ok=True)
meta=json.loads((root/'SourceAssets/Terrain/terrain-georeference.json').read_text());tr=Transformer.from_crs(4326,meta['crs'],always_xy=True)
r=ET.parse(root/'References/twelfth-market.osm').getroot();nodes={};tags=lambda e:{t.get('k'):t.get('v') for t in e.findall('tag')}
for n in r.findall('node'):
 x,y=tr.transform(float(n.get('lon')),float(n.get('lat')));nodes[n.get('id')]=((x-meta['origin_utm'][0])*100/3,-(y-meta['origin_utm'][1])*100/3)
ways={w.get('id'):w for w in r.findall('way')};points=lambda w:[nodes[n.get('ref')] for n in w.findall('nd') if n.get('ref') in nodes]
gate=nodes['316638596'];billy=nodes['1595679215'];ids=[n.get('ref') for n in ways['28798697'].findall('nd')];index=ids.index('316638596');approach=LineString([nodes[n] for n in ids[index::-1]])
park=Polygon(json.loads((root/'SourceAssets/Terrain/park-region.json').read_text())['outline'])
buildings=[]
for w in ways.values():
 if tags(w).get('building') and len(points(w))>=4:
  p=Polygon(points(w));
  if p.is_valid:buildings.append((w.get('id'),p))
stalls=[];rejected=[]
for distance in [400,800,1200,1600,2000]:
 p=approach.interpolate(distance);a=approach.interpolate(distance-10);b=approach.interpolate(distance+10);yaw=math.atan2(b.y-a.y,b.x-a.x);c,s=math.cos(yaw),math.sin(yaw)
 for side in [-1,1]:
  x,y=p.x-s*side*340,p.y+c*side*340
  corners=[(x+c*u-s*v,y+s*u+c*v) for u,v in [(-150,-150),(150,-150),(150,150),(-150,150)]];poly=Polygon(corners)
  reason='outside park' if not park.covers(poly) else 'building clearance' if any(poly.distance(g)<50 for _,g in buildings) else None
  row={'center_xy':[x,y],'yaw_degrees':math.degrees(yaw),'footprint_xy':corners,'distance_along_path_cm':distance,'side':side,'roof_size_cm':[300,300],'planned_eave_height_cm':220,'planned_peak_height_cm':285}
  if reason:rejected.append(dict(row,reason=reason))
  else:stalls.append(row)
assert stalls
plan={'source':'OpenStreetMap contributors ODbL, gate node 316638596, pedestrian way 28798697, restaurant node 1595679215','status':'source layout candidate; ground survey, meshes, collision and installation pending','frame':'Unreal ESU centimetres at existing terrain scale','gate_xy':gate,'billys_reference_xy':billy,'entrance_path_xy':list(approach.coords),'stalls':stalls,'rejected':rejected,'tutorial_closure':'Author market setup barriers across 12th Street gate; preserve Piedmont Avenue sidewalk and both routes to 14th Street. Collision height and bypass validation pending.'}
(out/'layout.json').write_text(json.dumps(plan,indent=2)+'\n')
fig,ax=plt.subplots(figsize=(11,9));fig.patch.set_facecolor('#faf8f0');ax.set_facecolor('#edf3df')
for w in ways.values():
 t=tags(w);p=points(w)
 if t.get('highway') and len(p)>1:
  x,y=zip(*p);ax.plot(x,y,color='#777f85' if t['highway'] not in ['footway','pedestrian','path'] else '#c4b59c',lw=4 if t['highway']=='secondary' else 1,zorder=1)
for _,poly in buildings:
 x,y=poly.exterior.xy;ax.fill(x,y,color='#bbb2a3',alpha=.8)
for i,stall in enumerate(stalls):
 x,y=zip(*(stall['footprint_xy']+[stall['footprint_xy'][0]]));ax.fill(x,y,color='#f6f3e9',edgecolor='#638568',linewidth=2);ax.text(*stall['center_xy'],str(i+1),ha='center',va='center',fontsize=9)
ax.scatter(*gate,c='#c54d32',s=70,zorder=4);ax.annotate('12th Street gate\nclosure to be designed',gate,xytext=(-110,40),textcoords='offset points',arrowprops={'arrowstyle':'->'},fontsize=10)
ax.scatter(*billy,c='#345272',s=60);ax.annotate("Billy’s reference building",billy,xytext=(-110,-35),textcoords='offset points',arrowprops={'arrowstyle':'->'},fontsize=10)
ax.set_xlim(gate[0]-2300,gate[0]+3100);ax.set_ylim(gate[1]+2600,gate[1]-2300);ax.set_aspect('equal');ax.set_title('12th Street Green Market — candidate layout\nMapped gate and buildings; authored stall positions',fontsize=15);ax.set_xlabel('World east (cm)');ax.set_ylabel('World south (cm)');ax.grid(alpha=.15);fig.tight_layout();fig.savefig(out/'layout-review.png',dpi=150)
print({'stalls':len(stalls),'rejected':len(rejected),'gate_xy':gate,'billys_xy':billy})
