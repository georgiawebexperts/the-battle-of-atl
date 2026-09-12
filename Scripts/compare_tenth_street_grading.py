"""Compare source and candidate terrain along the same 10th lane tracks."""
import json
from pathlib import Path
import numpy as np
from shapely.geometry import LineString
root=Path(__file__).resolve().parents[1];folder=root/'SourceAssets/Terrain';meta=json.loads((folder/'terrain-georeference.json').read_text());sx,sy,sz=meta['unreal_scale'];lx,ly,_=meta['unreal_location_cm']
tracks=[]
for row in json.loads((folder/'TenthStreet/network.json').read_text())['roads']:
 pts=[p[:2] for p in row['points_cm']]
 if pts[0][0]>pts[-1][0]:pts.reverse()
 path=LineString(pts)
 if path.length<400:continue
 for lane in range(int(row['tags'].get('lanes','3'))):tracks.append(path.offset_curve(150+300*lane,join_style=2))
rows=[]
for filename in ['atlanta-height-krog.r16','atlanta-height-tenth-graded.r16']:
 raw=np.fromfile(folder/filename,dtype='<u2').reshape(meta['size'][1],meta['size'][0])
 def height(x,y):
  gx=(x-lx)/sx;gy=(-y-ly)/sy;ix=int(gx);iy=int(gy);dx=gx-ix;dy=gy-iy
  a,b,c,d=[(float(raw[v,u])-32768)*sz/128 for u,v in [(ix,iy),(ix+1,iy),(ix,iy+1),(ix+1,iy+1)]]
  return a+(b-a)*dx+(d-b)*dy if dx>=dy else a+(d-c)*dx+(c-a)*dy
 grades=[];changes=[]
 for track in tracks:
  h=[height(*track.interpolate(d).coords[0]) for d in range(100,int(track.length)-100,50)]
  g=np.diff(h)/50;grades.extend(abs(g));changes.extend(abs(np.diff(g)))
 rows.append({'source':filename,'max_grade_percent':float(max(grades)*100),'max_grade_change_per_50cm_percentage_points':float(max(changes)*100),'p95_grade_change_per_50cm_percentage_points':float(np.percentile(changes,95)*100)})
report={'comparison':rows,'native_verified':False,'accepted':False,'scope':'Same lane tracks sampled against source/candidate terrain. Final road mesh and terrain transitions not yet reviewed.'}
(root/'Tests/Results/2026-09-11-tenth-street-grading-candidate.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report))
