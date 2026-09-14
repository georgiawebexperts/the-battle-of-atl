"""Source-based street plan and terrain comparison; does not edit Unreal assets."""
from pathlib import Path
import json, xml.etree.ElementTree as ET, heapq, math
import numpy as np
from pyproj import Transformer
import shapely
from shapely.geometry import Polygon
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
root=Path(__file__).resolve().parents[1]; terrain=root/'SourceAssets/Terrain'; out=terrain/'PrideIntersection';out.mkdir(exist_ok=True)
m=json.loads((terrain/'terrain-georeference.json').read_text());tr=Transformer.from_crs(4326,m['crs'],always_xy=True)
def xy(lon,lat):
 e,n=tr.transform(lon,lat);return ((e-m['origin_utm'][0])*100*m['scale'],-(n-m['origin_utm'][1])*100*m['scale'])
r=ET.parse(root/'References/piedmont-pride-streets.osm').getroot();tags=lambda e:{t.get('k'):t.get('v') for t in e.findall('tag')}
nodes={n.get('id'):xy(float(n.get('lon')),float(n.get('lat'))) for n in r.findall('node')}
ways=[]
for w in r.findall('way'):
 ids=[n.get('ref') for n in w.findall('nd')]
 if all(n in nodes for n in ids):ways.append({'id':w.get('id'),'tags':tags(w),'nodes':ids})
streetnames=['10th Street Northeast','Piedmont Avenue Northeast','12th Street Northeast','13th Street Northeast','14th Street Northeast']
roads=[w for w in ways if w['tags'].get('name') in streetnames]
tenth={n for w in roads if w['tags']['name']==streetnames[0] for n in w['nodes']};piedmont={n for w in roads if w['tags']['name']==streetnames[1] for n in w['nodes']}
intersection=tenth&piedmont;assert len(intersection)==1,intersection
junction=next(iter(intersection));billy=nodes['1595679215'];gate=nodes['316638596']
motel=json.loads((terrain/'FancyRoachMotel/footprints.json').read_text());fp=np.array([p[:2] for b in motel['buildings'] for p in b['footprint_world_cm']]);center=fp.mean(axis=0)
start=min(tenth,key=lambda n:math.dist(nodes[n],center));end=min(piedmont,key=lambda n:math.dist(nodes[n],billy))
graph={}
for w in roads:
 for a,b in zip(w['nodes'],w['nodes'][1:]):
  d=math.dist(nodes[a],nodes[b]);graph.setdefault(a,[]).append((b,d,w['id']));graph.setdefault(b,[]).append((a,d,w['id']))
queue=[(0,start,[])];seen=set()
while queue:
 d,n,path=heapq.heappop(queue)
 if n in seen:continue
 seen.add(n);path=path+[n]
 if n==end:break
 for b,dist,w in graph.get(n,[]):
  if b not in seen:heapq.heappush(queue,(d+dist,b,path))
else:raise RuntimeError('No source street connection')
assert junction in path
nx,ny=m['size'];sx,sy,sz=m['unreal_scale'];lx,ly,_=m['unreal_location_cm']
base=np.fromfile(terrain/'atlanta-height.r16',dtype='<u2').reshape(ny,nx)
activefile=terrain/'LakePathGrading/atlanta-height-lake-paths-candidate.r16';active=np.fromfile(activefile,dtype='<u2').reshape(ny,nx)
z=(base.astype(float)-32768)*sz/128;za=(active.astype(float)-32768)*sz/128
xs=lx+np.arange(nx)*sx;ys=-(ly+np.arange(ny)*sy)
def height(x,y):
 gx=(x-lx)/sx;gy=(-y-ly)/sy;i,j=int(gx),int(gy);u,v=gx-i,gy-j;a,b,c,d=za[j,i],za[j,i+1],za[j+1,i],za[j+1,i+1]
 return float(a+(b-a)*u+(d-b)*v if u>=v else a+(d-c)*u+(c-a)*v)
route=[[*nodes[n],height(*nodes[n])] for n in path]
# Geographic cross-section bounds are deliberately named, not guessed as the user's bowl.
regions={'southwest_park':(-19000,2500,-8000,10500),'southeast_park':(-7500,3000,9000,14500)}
lake=json.loads((terrain/'lake-clara-meer.json').read_text());water=Polygon([(p[0],-p[1]) for p in lake['outer_cm']],holes=[[(p[0],-p[1]) for p in lake['island_cm']]])
stats={}
for name,(x0,y0,x1,y1) in regions.items():
 a=np.ix_((ys>=y0)&(ys<=y1),(xs>=x0)&(xs<=x1));v=z[a];delta=za[a]-v
 X,Y=np.meshgrid(xs[(xs>=x0)&(xs<=x1)],ys[(ys>=y0)&(ys<=y1)]);land=~shapely.contains_xy(water,X,Y);v=v[land];delta=delta[land]
 stats[name]={'bounds_world_cm':[x0,y0,x1,y1],'excludes_authored_lake_bed':True,'original_elevation_m':[float(v.min()/100/m['scale']+280),float(v.max()/100/m['scale']+280)],'modified_samples':int(np.count_nonzero(delta)),'total_samples':int(delta.size),'largest_change_real_m':float(np.abs(delta).max()/100/m['scale'])}
report={'status':'sourced plan, not installed or collision-tested','source':'OpenStreetMap contributors ODbL, References/piedmont-pride-streets.osm fetched 2026-09-13; USGS source DEM and installed build074 candidate','intersection_node':junction,'intersection_world_cm':nodes[junction],'motel_frontage_node':start,'billys_reference_node':'1595679215','billys_world_cm':billy,'route_nodes':path,'route_world_cm':route,'route_real_length_m':d/100/m['scale'],'direction':'West on 10th, right (north) onto Piedmont, Billys on park side near 12th. Centerline plan, not a traffic-lane or bicycle legality specification.','roads':[dict(w,points_world_cm=[[*nodes[n],height(*nodes[n])] for n in w['nodes']]) for w in roads],'terrain_regions':stats,'vertical_scale_matches_horizontal':math.isclose(sz/100,m['scale'],rel_tol=1e-9),'limitations':['Bowl location needs confirmation before editing hills.','Road dimensions and collision need native review.','USGS bare earth is not bridge or pavement elevation.']}
(out/'survey.json').write_text(json.dumps(report,indent=2)+'\n')
fig,axes=plt.subplots(1,2,figsize=(16,9));fig.patch.set_facecolor('#faf8ef')
for ax in axes:
 ax.set_facecolor('#edf1de')
 for w in ways:
  pts=np.array([nodes[n] for n in w['nodes']]);t=w['tags']
  if len(pts)<2:continue
  if t.get('building'):ax.fill(pts[:,0],pts[:,1],color='#b9b0a1',alpha=.8)
  elif t.get('highway'):ax.plot(pts[:,0],pts[:,1],color='#8d9393' if t['highway'] not in ['footway','path','pedestrian','cycleway'] else '#cdc2a4',lw=2 if t.get('name') in streetnames else .65)
 ax.invert_yaxis();ax.set_aspect('equal');ax.set_xlabel('East / west • game centimetres');ax.set_ylabel('South / north • game centimetres')
ax=axes[0];pts=np.array(route);ax.plot(pts[:,0],pts[:,1],color='#147c9b',lw=4,zorder=6)
for label,p,offset in [('Rainbow intersection\n10th + Piedmont',nodes[junction],(-100,40)),('Billy’s / 12th entrance',billy,(-145,-30)),('The Fancy\nRoach Motel',center,(30,20))]:
 ax.scatter(*p,s=45,color='#a43845',zorder=7);ax.annotate(label,p,xytext=offset,textcoords='offset points',fontsize=10,arrowprops={'arrowstyle':'->'},zorder=8,bbox={'facecolor':'white','alpha':.9,'edgecolor':'none'})
ax.set_xlim(-26000,-13500);ax.set_ylim(15000,-6500);ax.set_title('Street connection to build\nWest on 10th → right onto Piedmont',fontsize=14)
ax=axes[1];selx=(xs>=-21000)&(xs<=13000);sely=(ys>=-6500)&(ys<=16000);zz=z[np.ix_(sely,selx)]/100/m['scale']+280
cont=ax.contour(xs[selx],ys[sely],zz,levels=np.arange(250,300,2),colors='#437953',linewidths=.65);ax.clabel(cont,fontsize=7,fmt='%d m')
ax.set_xlim(-21000,13000);ax.set_ylim(16000,-6500);ax.set_title('Original USGS terrain • 2 m contours\nNo hill edits made from an assumed bowl location',fontsize=14)
fig.suptitle('The Battle of ATL — Piedmont geography review',fontsize=20);fig.text(.5,.025,'Map data © OpenStreetMap contributors (ODbL). Heights: USGS 3DEP. All axes compressed equally to one-third scale.',ha='center',fontsize=9);fig.tight_layout(rect=(0,.045,1,.94));fig.savefig(out/'street-and-terrain-review.png',dpi=140)
print(json.dumps({k:v for k,v in report.items() if k not in ['roads','route_nodes','route_world_cm']},indent=2))
