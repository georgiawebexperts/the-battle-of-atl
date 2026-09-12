"""Generate opposing through lanes from the installed graded-road source geometry."""
import json,math,pathlib,collections
root=pathlib.Path(__file__).resolve().parents[1];folder=root/'SourceAssets/Terrain/TenthStreetGraded'
network=json.loads((folder/'network.json').read_text());segments=[]
for row in network['roads']:
 p=row['points_cm'];p=p if p[0][0]<p[-1][0] else p[::-1];segments.append(p)
segments.sort(key=lambda p:p[0][0]);points=[]
for p in segments:
 if points:
  assert math.dist(points[-1][:2],p[0][:2])<1,(points[-1],p[0])
  points.extend(p[1:])
 else:points.extend(p)
# Offset to lane centres matching the installed paint: northbound side in ESU
# is westbound. Keep the shared turn lane empty through the 4-to-3 transition.
lengths=[0]
for a,b in zip(points,points[1:]):lengths.append(lengths[-1]+math.dist(a[:2],b[:2]))
def sample(d):
 for i in range(1,len(lengths)):
  if d<=lengths[i]:
   t=(d-lengths[i-1])/(lengths[i]-lengths[i-1]);return [a+(b-a)*t for a,b in zip(points[i-1],points[i])]
 return points[-1]
verts=[];tris=[];grid=collections.defaultdict(list)
for line in (folder/'TenthStreet_Road.obj').read_text().splitlines():
 p=line.split()
 if p and p[0]=='v':verts.append([float(p[1]),-float(p[2]),float(p[3])])
 elif p and p[0]=='f':
  tri=[verts[int(v.split('/')[0])-1] for v in p[1:]];idx=len(tris);tris.append(tri)
  for x in range(math.floor(min(v[0] for v in tri)/200),math.floor(max(v[0] for v in tri)/200)+1):
   for y in range(math.floor(min(v[1] for v in tri)/200),math.floor(max(v[1] for v in tri)/200)+1):grid[x,y].append(idx)
verts=[]
for line in (folder/'TenthStreet_RoadSeams.obj').read_text().splitlines():
 p=line.split()
 if p and p[0]=='v':verts.append([float(p[1]),-float(p[2]),float(p[3])])
 elif p and p[0]=='f':
  tri=[verts[int(v.split('/')[0])-1] for v in p[1:]];idx=len(tris);tris.append(tri)
  for x in range(math.floor(min(v[0] for v in tri)/200),math.floor(max(v[0] for v in tri)/200)+1):
   for y in range(math.floor(min(v[1] for v in tri)/200),math.floor(max(v[1] for v in tri)/200)+1):grid[x,y].append(idx)
road_count=len(tris)
verts=[]
for line in (folder/'TenthStreet_CycleTrack.obj').read_text().splitlines():
 p=line.split()
 if p and p[0]=='v':verts.append([float(p[1]),-float(p[2]),float(p[3])])
 elif p and p[0]=='f':
  tri=[verts[int(v.split('/')[0])-1] for v in p[1:]];idx=len(tris);tris.append(tri)
  for x in range(math.floor(min(v[0] for v in tri)/200),math.floor(max(v[0] for v in tri)/200)+1):
   for y in range(math.floor(min(v[1] for v in tri)/200),math.floor(max(v[1] for v in tri)/200)+1):grid[x,y].append(idx)
crossings=[r['points_cm'] for r in network['cycle_track'] if r['tags'].get('cycleway')=='crossing']
def crossing(x,y):
 for points in crossings:
  for a,b in zip(points,points[1:]):
   dx,dy=b[0]-a[0],b[1]-a[1];t=max(0,min(1,((x-a[0])*dx+(y-a[1])*dy)/(dx*dx+dy*dy)))
   if math.hypot(x-a[0]-t*dx,y-a[1]-t*dy)<=90:return True
 return False
def height(x,y):
 for idx in grid[math.floor(x/200),math.floor(y/200)]:
  if idx>=road_count and not crossing(x,y):continue
  a,b,c=tris[idx];det=(b[0]-a[0])*(c[1]-a[1])-(b[1]-a[1])*(c[0]-a[0])
  if abs(det)<1e-8:continue
  u=((x-a[0])*(c[1]-a[1])-(y-a[1])*(c[0]-a[0]))/det;v=((b[0]-a[0])*(y-a[1])-(b[1]-a[1])*(x-a[0]))/det
  if min(u,v,1-u-v)>=-1e-5:return a[2]+u*(b[2]-a[2])+v*(c[2]-a[2])
 raise AssertionError(('Car footprint leaves road',x,y))
routes=[];probes=[]
for name,offset,reverse in [('eastbound',750,False),('westbound',150,True)]:
 lane=[];conflict_indices=[]
 # Stop outside Monroe's conflict area until crossing controls are authored.
 for d in range(600,int(lengths[-1]-1400),100):
  p=sample(d);a=sample(d-30);b=sample(d+30);dx,dy=b[0]-a[0],b[1]-a[1];norm=math.hypot(dx,dy);dx/=norm;dy/=norm
  current_offset=offset if not reverse else 150+300*max(0,min(1,(-674.19-p[0])/1600))
  x,y=p[0]-dy*current_offset,p[1]+dx*current_offset;z=height(x,y);lane.append([x,y,z])
  for forward in [-236,0,236]:
   for side in [-114,0,114]:
    px=x+dx*forward-dy*side;py=y+dy*forward+dx*side;pz=height(px,py);
    if crossing(px,py):conflict_indices.append(len(lane)-1)
    probes.append({'lane':name,'xyz':[px,py,pz]})
 if reverse:lane.reverse()
 routes.append({'name':name,'points_cm':lane,'offset_cm':offset,'lane_selection':('eastbound 750 cm' if not reverse else 'westbound 450 cm in four lanes, tapering to 150 cm in three lanes'),'crossing_point_indices':sorted(set(len(lane)-1-i if reverse else i for i in conflict_indices)),'speed_cm_s':650})
(folder/'car-lanes.json').write_text(json.dumps({'author':'2026-09-12 [codex-maclaptop]','routes':routes,'scope':'10th Street opposing through-lane candidates. Stop short of Monroe conflict zone; signals and crossing traversal pending.'},indent=2)+'\n')
(folder/'car-lane-probes.json').write_text(json.dumps({'samples':probes,'car_half_width_cm':114,'car_half_length_cm':236,'source_coverage_passed':True})+'\n')
print(json.dumps({'lanes':len(routes),'points_per_lane':len(routes[0]['points_cm']),'footprint_probes':len(probes),'source_coverage_passed':True}))
