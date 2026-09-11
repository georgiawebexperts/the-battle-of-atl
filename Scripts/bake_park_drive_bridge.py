"""Continuous Park Drive crossing across three overlapping OSM way ribbons.
Deck is authored from bank elevations; it is not a bridge survey.
"""
from pathlib import Path
import json,math,numpy as np
from shapely.geometry import LineString,Polygon
from shapely.ops import unary_union
import mapbox_earcut
P=Path(__file__).resolve().parents[1];O=P/'SourceAssets/Terrain/ParkDriveBridge';O.mkdir(exist_ok=True)
paths=json.load(open(P/'SourceAssets/Terrain/park-path-network.json'))['paths'];ids={61490793,1389725669,1389725671};selected=[p for p in paths if p['osm_id'] in ids];main=next(p for p in selected if p['osm_id']==61490793)
lines=[LineString(np.array(p['points_cm'])[:,:2]) for p in selected];poly=unary_union([l.buffer(140,cap_style=2,join_style=2) for l in lines]);axis=np.array(main['points_cm'][-1][:2])-main['points_cm'][0][:2];axis/=np.linalg.norm(axis);origin=np.array(main['points_cm'][0][:2]);longs=np.array(poly.exterior.coords)@axis;lo,hi=longs.min(),longs.max()
m=json.load(open(P/'SourceAssets/Terrain/terrain-georeference.json'));nx,ny=m['size'];raw=np.fromfile(P/'SourceAssets/Terrain/atlanta-height.r16',dtype='<u2').reshape(ny,nx);loc=m['unreal_location_cm'];scale=m['unreal_scale']
def ground(x,y):
 gx=(x-loc[0])/scale[0];gy=(y-loc[1])/scale[1];ix=min(nx-2,max(0,int(math.floor(gx))));iy=min(ny-2,max(0,int(math.floor(gy))));u=gx-ix;v=gy-iy;a,b,c,d=[(int(raw[j,i])-32768)*scale[2]/128 for i,j in [(ix,iy),(ix+1,iy),(ix+1,iy+1),(ix,iy+1)]];return (a+(b-a)*u+(c-b)*v if v<=u else a+(c-d)*u+(d-a)*v)+3
from shapely.geometry import Point
def profile(x,y):
 point=Point(x,y);values=[];weights=[]
 for p,line in zip(selected,lines):
  t=line.project(point)/line.length;values.append(p['points_cm'][0][2]*(1-t)+p['points_cm'][-1][2]*t+8*math.sin(math.pi*t));weights.append(1/(line.distance(point)**2+2500))
 return float(np.average(values,weights=weights))
from scipy.interpolate import RBFInterpolator
anchors=np.array([p['points_cm'][i][:2] for p in selected for i in [0,-1]])
correction=RBFInterpolator(anchors/1000,np.array([ground(x,y)-profile(x,y) for x,y in anchors]),kernel='thin_plate_spline',degree=1)
def top(x,y):
 # Bare earth falls away beneath the span. Keep the deck bank-anchored instead
 # of blending it down into the streambed at either approach.
 return max(ground(x,y),profile(x,y)+float(correction(np.array([[x,y]])/1000)[0]))
mesh={'v':[],'f':[]};rails={'v':[],'f':[]}
def quad(m,v):
 n=len(m['v']);m['v'].extend([list(p) for p in v]);m['f'].extend([[n,n+1,n+2],[n,n+2,n+3]])
def beam(m,a,b,width,depth):
 a=np.array(a);b=np.array(b);d=b-a;s=np.array([-d[1],d[0],0.]);s=s/np.linalg.norm(s)*width/2;v=[a-s,a+s,b+s,b-s];low=[p-[0,0,depth] for p in v];quad(m,list(reversed(v)));quad(m,low)
 for i in range(4):j=(i+1)%4;quad(m,[v[i],low[i],low[j],v[j]])
# Shared 25 cm grid vertices; clipped triangle boundaries use the same plane.
x0,y0,x1,y1=poly.bounds;size=25
for x in np.arange(math.floor(x0/size)*size,x1,size):
 for y in np.arange(math.floor(y0/size)*size,y1,size):
  for off in [[(0,0),(1,0),(1,1)],[(0,0),(1,1),(0,1)]]:
   xy=np.array([(x+i*size,y+j*size) for i,j in off]);clip=Polygon(xy).intersection(poly)
   if clip.is_empty or clip.area<1e-7:continue
   plane=np.linalg.solve(np.c_[xy,np.ones(3)],[top(a,b) for a,b in xy]);pieces=[clip] if clip.geom_type=='Polygon' else [g for g in clip.geoms if g.geom_type=='Polygon']
   for part in pieces:
    vertices=np.array(part.exterior.coords[:-1]);faces=mapbox_earcut.triangulate_float64(vertices,np.array([len(vertices)],dtype=np.uint32)).reshape(-1,3)
    for face in faces:
     v=vertices[face];cross=np.cross(np.r_[v[1]-v[0],0],np.r_[v[2]-v[0],0])[2]
     if cross<0:v=v[::-1]
     n=len(mesh['v']);mesh['v'].extend([[a,b,float(plane[0]*a+plane[1]*b+plane[2])] for a,b in v]);mesh['f'].append([n,n+1,n+2])
# Close the slab so the crossing is also solid from underneath.
top_v=list(mesh['v']);top_f=list(mesh['f']);n=len(top_v);mesh['v'].extend([[x,y,z-18] for x,y,z in top_v]);mesh['f'].extend([[n+i for i in reversed(f)] for f in top_f])
def deck_height(x,y):
 ix=math.floor(x/size)*size;iy=math.floor(y/size)*size;u=(x-ix)/size;v=(y-iy)/size
 a,b,c,d=[top(px,py) for px,py in [(ix,iy),(ix+size,iy),(ix+size,iy+size),(ix,iy+size)]]
 return a+(b-a)*u+(c-b)*v if v<=u else a+(c-d)*u+(d-a)*v
for a,b in zip(list(poly.exterior.coords),list(poly.exterior.coords)[1:]):
 a=np.array(a);b=np.array(b);delta=b-a;cuts={0.,1.}
 # Break at each grid edge and triangle diagonal. Long unsplit cap faces can
 # protrude above the curved riding surface and become invisible barriers.
 for va,vb in [(a[0],b[0]),(a[1],b[1]),(a[0]-a[1],b[0]-b[1])]:
  if abs(vb-va)<1e-8:continue
  for k in range(math.floor(min(va,vb)/size),math.ceil(max(va,vb)/size)+1):
   t=(k*size-va)/(vb-va)
   if 0<t<1:cuts.add(t)
 ts=sorted(cuts)
 for ta,tb in zip(ts,ts[1:]):
  pa=a+delta*ta;pb=a+delta*tb;va=np.r_[pa,deck_height(*pa)];vb=np.r_[pb,deck_height(*pb)];quad(mesh,[va,vb,vb-[0,0,18],va-[0,0,18]])
# Railings follow the outer boundary only; road/sidewalk joins have no barriers.
ring=list(poly.exterior.coords)
for a,b in zip(ring,ring[1:]):
 a=np.array(a);b=np.array(b);length=np.linalg.norm(b-a);n=max(1,math.ceil(length/80))
 for i in range(n):
  x=a+(b-a)*i/n;y=a+(b-a)*(i+1)/n;mid=(x+y)/2;s=np.dot(mid,axis)
  if s-lo<210 or hi-s<210:continue
  va=np.r_[x,top(*x)];vb=np.r_[y,top(*y)]
  for h in [45,100]:beam(rails,va+[0,0,h],vb+[0,0,h],8,8)
  beam(rails,va+[-5,0,106],va+[5,0,106],10,106)
chunks=[]
for kind,obj in [('Deck',mesh),('Rails',rails)]:
 name='ParkDrive_'+kind;v=np.array(obj['v']);text=[f'o {name}']+['v %.6f %.6f %.6f'%(x,-y,z) for x,y,z in v]+['vt %.6f %.6f'%(x/200,y/200) for x,y,z in v]+['f '+' '.join(f'{i+1}/{i+1}' for i in reversed(f)) for f in obj['f']];(O/(name+'.obj')).write_text('\n'.join(text)+'\n');chunks.append({'file':name+'.obj','kind':kind,'label':'Park Drive unified '+kind,'bounds_cm':[v.min(axis=0).tolist(),v.max(axis=0).tolist()]})
bridges=[]
for p in selected:
 points=[[x,y,top(x,y)] for x,y,z in p['points_cm']];bridges.append({'osm_id':p['osm_id'],'centerline_cm':points,'length_cm':LineString(np.array(points)[:,:2]).length,'width_cm':280})
mid=np.array(main['points_cm'][len(main['points_cm'])//2][:2]);side=np.array([-axis[1],axis[0]]);cross_route=[]
for offset in np.linspace(-180,180,9):
 xy=mid+side*offset;cross_route.append([float(xy[0]),float(xy[1]),top(*xy)])
(O/'manifest.json').write_text(json.dumps({'status':'source_baked','elevation_policy':'Authored smooth profile constrained to six bank endpoints, clamped above terrain; not surveyed bridge height','chunks':chunks,'bridges':bridges,'test_routes':{'cross_deck':cross_route},'pending':['Collision and bidirectional riding for all three mapped ways','Final architectural detail']},indent=2));print('Park Drive:',len(mesh['f']),'deck triangles;',len(bridges),'mapped ways')
