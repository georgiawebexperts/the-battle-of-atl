"""Create exterior building shells and storefront bays on mapped footprints."""
import json,math
from pathlib import Path
from shapely.geometry import Polygon,LineString,Point
from shapely.geometry.polygon import orient
from shapely import constrained_delaunay_triangles
root=Path(__file__).resolve().parents[1];folder=root/'SourceAssets/Terrain/KrogBuildings';data=json.loads((folder/'buildings.json').read_text())
groups={k:[] for k in ['Brick','Roof','Glass','Stucco','Landing']}
def face(mat,points):
 for i in range(1,len(points)-1):groups[mat].append([points[0],points[i],points[i+1]])
def box(mat,center,size,yaw):
 c,s=math.cos(yaw),math.sin(yaw);v=[]
 for z in [-1,1]:
  for x,y in [(-1,-1),(1,-1),(1,1),(-1,1)]:
   a,b=x*size[0]/2,y*size[1]/2;v.append([center[0]+c*a-s*b,center[1]+s*a+c*b,center[2]+z*size[2]/2])
 for ids in [(3,2,1,0),(4,5,6,7),(0,1,5,4),(1,2,6,5),(2,3,7,6),(3,0,4,7)]:face(mat,[v[i] for i in ids])
windows=0;entrances=[]
survey_path=root/'Tests/Results/2026-09-12-krog-entrance-survey.json'
thresholds={r['osm_way']:r['samples'][0]['surface_z']+4 for r in json.loads(survey_path.read_text())['entrances']} if survey_path.exists() else {}
for row in data['buildings']:
 poly=orient(Polygon(row['footprint_xy']),sign=1);ring=list(poly.exterior.coords);base=row['base_z_cm'];roof=base+row['storeys']*row['storey_height_cm'];bottom=row['foundation_z_cm']
 # Choose a substantial facade facing the crossing for a closed shop entrance.
 candidates=[(LineString([a,b]).distance(Point(29619.957,115727.832)),i) for i,(a,b) in enumerate(zip(ring,ring[1:])) if math.dist(a,b)>240]
 entry_edge=min(candidates)[1] if candidates else -1
 for edge_index,(a,b) in enumerate(zip(ring,ring[1:])):
  dx,dy=b[0]-a[0],b[1]-a[1];length=math.hypot(dx,dy)
  if length<1:continue
  ux,uy=dx/length,dy/length;ox,oy=uy,-ux;yaw=math.atan2(dy,dx)
  face('Brick',[(a[0],a[1],bottom),(b[0],b[1],bottom),(b[0],b[1],roof),(a[0],a[1],roof)])
  def part(mat,d,z,width,depth,tall,offset):box(mat,[a[0]+ux*d+ox*offset,a[1]+uy*d+oy*offset,z],[width,depth,tall],yaw)
  part('Roof',length/2,roof+12,length,24,24,0)
  for level in range(row['storeys']):
   part('Stucco',length/2,base+level*340+325,length,12,10,4)
   count=int(length//220)
   for i in range(count):
    width=min(145,length/count-55);d=(i+.5)*length/count;z=base+level*340+165;tall=220 if level==0 else 170
    is_entry=level==0 and edge_index==entry_edge and i==count//2
    if is_entry:
     entry_base=thresholds.get(row['osm_way'],base)
     z=entry_base+115;tall=230;width=min(140,width)
     part('Roof',d,entry_base+246,width+50,90,14,35)
     part('Landing',d,entry_base-4,width+40,70,8,60)
     entrances.append({'osm_way':row['osm_way'],'xyz':[a[0]+ux*d+ox*25,a[1]+uy*d+oy*25,entry_base],'scope':'Closed exterior door at estimated foundation height; native entrance approach unverified.'})
    part('Roof',d,z,width+16,12,tall+16,7);part('Glass',d,z,width,8,tall,15)
    part('Stucco',d,z,5,8,tall,20)
    if not is_entry:part('Stucco',d,z,width,8,5,20)
    else:
     for side in [-1,1]:part('Stucco',d+side*12,entry_base+105,4,8,26,25)
    windows+=1
 for tri in constrained_delaunay_triangles(poly).geoms:face('Roof',[(x,y,roof) for x,y in list(orient(tri,sign=1).exterior.coords)[:3]])
aprons=[]
results=[]
for mat,faces in groups.items():
 lines=['o KrogBuildings_'+mat];vertices=[p for f in faces for p in f]
 for x,y,z in vertices:lines.append(f'v {x:.5f} {-y:.5f} {z:.5f}')
 for x,y,z in vertices:lines.append(f'vt {x/200:.5f} {z/200:.5f}')
 for i in range(0,len(vertices),3):lines.append('f '+' '.join(f'{j}/{j}' for j in [i+3,i+2,i+1]))
 name='KrogBuildings_'+mat+'.obj';(folder/name).write_text('\n'.join(lines)+'\n');results.append({'material':mat,'file':name,'triangles':len(faces)})
(folder/'manifest.json').write_text(json.dumps({'buildings':len(data['buildings']),'window_bays':windows,'entrances':entrances,'aprons':aprons,'surfaces':results,'scope':'Mapped footprint exterior study; estimated heights and generic facade treatment, not photo-matched or main-installed.'},indent=2)+'\n');print(results,windows)
