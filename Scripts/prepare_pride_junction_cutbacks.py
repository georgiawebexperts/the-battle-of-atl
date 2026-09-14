"""Cut only the new Piedmont mouth from retained 10th curb/paint source triangles."""
from pathlib import Path
import json,hashlib
import numpy as np
from shapely.geometry import Polygon,LineString,box
from shapely.ops import unary_union
from shapely import constrained_delaunay_triangles
root=Path(__file__).resolve().parents[1];out=root/'SourceAssets/Terrain/PrideIntersection';source=root/'SourceAssets/Terrain/TenthStreetGraded';data=json.loads((out/'survey.json').read_text())
lines=[LineString([p[:2] for p in w['points_world_cm']]) for w in data['roads'] if w['tags']['name']=='Piedmont Avenue Northeast'];mouth=unary_union(lines).buffer(420,cap_style=2,join_style=2).intersection(box(-26000,11500,-22500,14500));rows=[]
for kind in ['Sidewalk','Separator','WhitePaint','YellowPaint']:
 file=root/'SourceAssets/Terrain/MonroeTraffic/Monroe_SidewalkTrim.obj' if kind=='Sidewalk' else source/('TenthStreet_'+kind+'.obj');digest=hashlib.sha256(file.read_bytes()).hexdigest();verts=[];faces=[];removed=0
 for line in file.read_text().splitlines():
  p=line.split()
  if p and p[0]=='v':verts.append((float(p[1]),-float(p[2]),float(p[3])))
  elif p and p[0]=='f':
   face=[verts[int(v.split('/')[0])-1] for v in p[1:]];assert len(face)==3;poly=Polygon([v[:2] for v in face]);cut=poly.difference(mouth);removed+=poly.area-cut.area
   if not poly.intersects(mouth):faces.append(face);continue
   # Preserve original triangle planes outside the cut; do not regrade distant streets.
   if poly.area<1e-8:continue
   coefficients=np.linalg.solve(np.array([[v[0],v[1],1] for v in face]),np.array([v[2] for v in face]))
   for tri in constrained_delaunay_triangles(cut).geoms:
    if tri.area>1e-8:faces.append([[x,y,float(np.dot(coefficients,[x,y,1]))] for x,y in list(tri.exterior.coords)[:3]])
 v=[p for f in faces for p in f];name='PrideTenth_'+kind;lines=['o '+name]+[f'v {x:.6f} {-y:.6f} {z:.6f}' for x,y,z in v]+[f'vt {x/200:.6f} {y/200:.6f}' for x,y,z in v]
 for k in range(0,len(v),3):
  a,b,c=v[k:k+3];cross=(b[0]-a[0])*(c[1]-a[1])-(b[1]-a[1])*(c[0]-a[0]);order=(k,k+1,k+2) if cross<0 else (k+2,k+1,k);lines.append('f '+' '.join(f'{i+1}/{i+1}' for i in order))
 (out/(name+'.obj')).write_text('\n'.join(lines)+'\n');assert hashlib.sha256(file.read_bytes()).hexdigest()==digest
 rows.append({'name':name,'kind':kind,'source_name':'SM_Monroe_SidewalkTrim' if kind=='Sidewalk' else 'SM_TenthStreet_'+kind,'source_sha256':digest,'triangles':len(faces),'removed_cm2':removed,'bounds_cm':[[min(p[i] for p in v) for i in range(3)],[max(p[i] for p in v) for i in range(3)]]})
(out/'cutbacks.json').write_text(json.dumps({'meshes':rows,'status':'candidate only; original meshes unchanged','mouth_bounds_cm':list(mouth.bounds)},indent=2)+'\n');print(json.dumps(rows,indent=2))
