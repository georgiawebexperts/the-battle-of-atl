"""Verify winding repair retains each original triangle's physical vertices."""
from pathlib import Path
from collections import Counter
import json
root=Path(__file__).resolve().parents[1]
def read(path):
 vertices=[];faces=[]
 for line in path.read_text().splitlines():
  if line.startswith('v '):vertices.append(tuple(map(float,line.split()[1:])))
  elif line.startswith('f '):faces.append(tuple(vertices[int(v.split('/')[0])-1] for v in line.split()[1:]))
 return faces
rows=[]
for kind in ['Shell','Columns']:
 before=read(root/f'SourceAssets/Terrain/KrogContinuousShell/KrogTunnel_{kind}.obj')
 after=read(root/f'SourceAssets/Terrain/KrogOutwardShell/KrogTunnel_{kind}.obj')
 assert Counter(tuple(sorted(f)) for f in before)==Counter(tuple(sorted(f)) for f in after)
 rows.append({'mesh':kind,'triangles':len(after),'physical_triangles_preserved':True})
# In the first roof strip, each top/bottom/side triangle must point away from its box centre.
f=read(root/'SourceAssets/Terrain/KrogOutwardShell/KrogTunnel_Shell.obj')
vertices=set(p for tri in f[:4] for p in tri);center=[sum(p[k] for p in vertices)/len(vertices) for k in range(3)]
dots=[]
for tri in f[:10]:
 a,b,c=tri;u=[b[k]-a[k] for k in range(3)];v=[c[k]-a[k] for k in range(3)]
 normal=[u[1]*v[2]-u[2]*v[1],u[2]*v[0]-u[0]*v[2],u[0]*v[1]-u[1]*v[0]]
 dots.append(sum(normal[k]*((a[k]+b[k]+c[k])/3-center[k]) for k in range(3)))
assert all(d>0 for d in dots)
r={'passed':True,'meshes':rows,'first_roof_strip_outward_dots':dots}
(root/'Tests/Results/2026-09-12-krog-shell-winding.json').write_text(json.dumps(r,indent=2)+'\n');print(json.dumps(r))
