"""Sample actual authored road triangles along motor-lane centers."""
import json,math,os
from pathlib import Path
from shapely.geometry import Polygon,Point,LineString
from shapely.strtree import STRtree
from shapely.ops import unary_union
root=Path(__file__).resolve().parents[1];candidate=os.environ.get('BATTLE_TENTH_CANDIDATE')=='1';folder=root/'SourceAssets/Terrain'/('TenthStreetGraded' if candidate else 'TenthStreet')
vertices=[];faces=[]
for line in (folder/'TenthStreet_Road.obj').read_text().splitlines():
 p=line.split()
 if p and p[0]=='v':vertices.append((float(p[1]),-float(p[2]),float(p[3])))
 elif p and p[0]=='f':faces.append([vertices[int(v.split('/')[0])-1] for v in p[1:]])
polys=[Polygon([v[:2] for v in f]) for f in faces];tree=STRtree(polys)
def surface(x,y):
 point=Point(x,y)
 for idx in tree.query(point,predicate='intersects'):
  a,b,c=faces[idx];det=(b[0]-a[0])*(c[1]-a[1])-(c[0]-a[0])*(b[1]-a[1])
  if abs(det)<1e-9:continue
  u=((x-a[0])*(c[1]-a[1])-(c[0]-a[0])*(y-a[1]))/det
  v=((b[0]-a[0])*(y-a[1])-(x-a[0])*(b[1]-a[1]))/det
  return a[2]+u*(b[2]-a[2])+v*(c[2]-a[2])
 return None
network=json.loads((folder/'network.json').read_text())
cycle_area=unary_union([LineString([v[:2] for v in r['points_cm']]).buffer(80,cap_style=2,join_style=2) for r in network['cycle_track']])
rows=[]
for road in network['roads']:
 pts=[v[:2] for v in road['points_cm']]
 if pts[0][0]>pts[-1][0]:pts.reverse()
 path=LineString(pts)
 if path.length<400:continue
 lanes=int(road['tags'].get('lanes','3'))
 for lane in range(lanes):
  track=path.offset_curve(150+lane*300,join_style=2);samples=[]
  for d in range(100,int(track.length)-100,50):
   point=track.interpolate(d);z=surface(point.x,point.y);samples.append((d,point.x,point.y,z))
  grades=[(b[3]-a[3])/(b[0]-a[0]) for a,b in zip(samples,samples[1:]) if a[3] is not None and b[3] is not None]
  changes=[abs(b-a) for a,b in zip(grades,grades[1:])]
  rows.append({'osm_way':road['osm_way'],'lane_index':lane,'sample_count':len(samples),'cycle_crossing_samples':sum(v[3] is None and cycle_area.covers(Point(v[1:3])) for v in samples),'missing_pavement_samples':sum(v[3] is None and not cycle_area.covers(Point(v[1:3])) for v in samples),'max_grade_percent':round(max(map(abs,grades),default=0)*100,2),'max_grade_change_per_50cm_percentage_points':round(max(changes,default=0)*100,2)})
result={'sampling_cm':50,'lanes':rows,'scope':'Mesh geometry only, not a native ride test. Grade changes identify profile roughness; thresholds are not road-engineering certification.','native_ride_verified':False}
(root/('Tests/Results/2026-09-11-tenth-street-graded-profile.json' if candidate else 'Tests/Results/2026-09-11-tenth-street-profile.json')).write_text(json.dumps(result,indent=2)+'\n')
print(json.dumps({'lanes':len(rows),'cycle_crossing_samples':sum(r['cycle_crossing_samples'] for r in rows),'missing':sum(r['missing_pavement_samples'] for r in rows),'max_grade_percent':max(r['max_grade_percent'] for r in rows),'max_grade_change_per_50cm_percentage_points':max(r['max_grade_change_per_50cm_percentage_points'] for r in rows)}))
