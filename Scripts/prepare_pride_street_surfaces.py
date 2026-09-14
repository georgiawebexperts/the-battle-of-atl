"""Build terrain-conforming Piedmont extension candidates without editing the map."""
from pathlib import Path
import json, math
import numpy as np
from shapely.geometry import Polygon,LineString,box,Point
from shapely.ops import unary_union
from shapely import constrained_delaunay_triangles
root=Path(__file__).resolve().parents[1];folder=root/'SourceAssets/Terrain/PrideIntersection';data=json.loads((folder/'survey.json').read_text());m=json.loads((root/'SourceAssets/Terrain/terrain-georeference.json').read_text());sx,sy,sz=m['unreal_scale'];lx,ly,_=m['unreal_location_cm'];nx,ny=m['size'];raw=np.fromfile(root/'SourceAssets/Terrain/LakePathGrading/atlanta-height-lake-paths-candidate.r16',dtype='<u2').reshape(ny,nx);z=(raw.astype(float)-32768)*sz/128
# Extend north through the current cutoff to the existing 13th Street road.
clip=box(-26200,-1000,-17400,14800)
lines=[LineString([p[:2] for p in w['points_world_cm']]) for w in data['roads'] if w['tags']['name']=='Piedmont Avenue Northeast']
road=unary_union(lines).buffer(300,cap_style=2,join_style=2).intersection(clip)
# Preserve existing 10th paving. Its installed wider motor lanes sit south of the mapped centerline.
def footprint(file):
 v=[];f=[]
 for line in file.read_text().splitlines():
  p=line.split()
  if p and p[0]=='v':v.append((float(p[1]),-float(p[2])))
  elif p and p[0]=='f':f.append(Polygon([v[int(s.split('/')[0])-1] for s in p[1:]]))
 return unary_union(f)
existing=unary_union([footprint(folder/('PrideTenth_'+name+'.obj') if name in ['Sidewalk','Separator'] else root/'SourceAssets/Terrain/TenthStreetGraded'/('TenthStreet_'+name+'.obj')) for name in ['Road','CycleTrack','Sidewalk','Separator','RoadSeams']])
# Buildings constrain the authored human-scale road and sidewalk widths.
import xml.etree.ElementTree as ET
from pyproj import Transformer
tr=Transformer.from_crs(4326,m['crs'],always_xy=True);xml=ET.parse(root/'References/piedmont-pride-streets.osm').getroot();nodes={}
for n in xml.findall('node'):
 e,north=tr.transform(float(n.get('lon')),float(n.get('lat')));nodes[n.get('id')]=((e-m['origin_utm'][0])*100*m['scale'],-(north-m['origin_utm'][1])*100*m['scale'])
buildings=[]
for w in xml.findall('way'):
 if not any(t.get('k')=='building' for t in w.findall('tag')):continue
 refs=[n.get('ref') for n in w.findall('nd')]
 if len(refs)>3 and all(n in nodes for n in refs):
  p=Polygon([nodes[n] for n in refs])
  if p.is_valid and p.intersects(clip):buildings.append(p)
blocked=unary_union(buildings).buffer(20)
walk=road.buffer(120,join_style=2).difference(road).intersection(clip).difference(blocked).difference(existing)
road=road.difference(blocked).difference(existing)
def height(x,y):
 gx=(x-lx)/sx;gy=(-y-ly)/sy;i,j=int(gx),int(gy);u,v=gx-i,gy-j;a,b,c,d=z[j,i],z[j,i+1],z[j+1,i],z[j+1,i+1]
 return float(a+(b-a)*u+(d-b)*v if u>=v else a+(d-c)*u+(c-a)*v)
meshes=[]
for name,poly,lift in [('PiedmontRoad',road,14),('PiedmontSidewalk',walk,18)]:
 faces=[];x0,y0,x1,y1=poly.bounds
 for j in range(max(0,int((-y1-ly)/sy)),min(ny-1,math.ceil((-y0-ly)/sy))):
  for i in range(max(0,int((x0-lx)/sx)),min(nx-1,math.ceil((x1-lx)/sx))):
   a=(lx+i*sx,-(ly+j*sy));b=(a[0]+sx,a[1]);c=(a[0],a[1]-sy);d=(b[0],c[1])
   for tri in [Polygon([a,b,d]),Polygon([a,d,c])]:
    if not poly.intersects(tri):continue
    cut=poly.intersection(tri)
    for t in constrained_delaunay_triangles(cut).geoms:
     if t.area>.001:faces.append([[x,y,height(x,y)+lift] for x,y in list(t.exterior.coords)[:3]])
 coverage=unary_union([Polygon([p[:2] for p in f]) for f in faces]);missing=poly.difference(coverage.buffer(.001)).area;assert missing<1,(name,missing)
 verts=[p for f in faces for p in f];lines=['o '+name]+[f'v {x:.5f} {-y:.5f} {h:.5f}' for x,y,h in verts]+[f'vt {x/200:.5f} {y/200:.5f}' for x,y,h in verts]
 for k in range(0,len(verts),3):
  a,b,c=verts[k:k+3];cross=(b[0]-a[0])*(c[1]-a[1])-(b[1]-a[1])*(c[0]-a[0]);order=(k,k+1,k+2) if cross<0 else (k+2,k+1,k)
  lines.append('f '+' '.join(f'{v+1}/{v+1}' for v in order))
 (folder/(name+'.obj')).write_text('\n'.join(lines)+'\n');meshes.append({'name':name,'triangles':len(faces),'bounds_cm':[[min(v[k] for v in verts) for k in range(3)],[max(v[k] for v in verts) for k in range(3)]],'uncovered_cm2':missing,'building_overlap_cm2':poly.intersection(blocked).area})
# Sidewalk and street tile share elevation at boundaries except the intentional 4cm lip.
report={'status':'candidate meshes only; native review pending','road_width_cm':600,'sidewalk_width_cm':120,'terrain':'build074 lake-graded DEM','terrain_unchanged':True,'existing_tenth_road_preserved':True,'junction_curb_cutbacks':'cutbacks.json','meshes':meshes,'pending':['Native import and collision checks','Corner/sidewalk visual inspection','Relocate tutorial cutoff with full boundary coverage','Rainbow markings and street signs','Both-direction bike traversal and release']}
(folder/'surfaces.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report,indent=2))
