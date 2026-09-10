"""Bake OSM ribbons against the actual R16 Landscape triangle planes.

A draft source asset, not a claim of rideability. Bridges remain in the spline
network but require deck elevations before their separate meshes can be built.
Uses the i00-i11 diagonal from UE LandscapeRender.cpp. Grid clipping makes
pavement follow exactly the same piecewise planar terrain, with a 3 cm lift.
"""
from pathlib import Path
import json, math, collections
import numpy as np
import shapely, mapbox_earcut
from shapely.geometry import LineString, Polygon
from shapely.ops import unary_union
P=Path(__file__).resolve().parents[1]; O=P/'SourceAssets/Terrain'; OUT=O/'ParkPavement';OUT.mkdir(exist_ok=True)
m=json.loads((O/'terrain-georeference.json').read_text()); d=json.loads((O/'park-path-network.json').read_text())
nx,ny=m['size']; raw=np.fromfile(O/'atlanta-height.r16',dtype='<u2').reshape(ny,nx)
loc=np.array(m['unreal_location_cm']); scale=np.array(m['unreal_scale']); groups=collections.defaultdict(list); deferred=[]
for p in d['paths']:
 if p['tags'].get('bridge')=='yes':deferred.append(p['osm_id']);continue
 surface=p['tags'].get('surface','asphalt')
 material='Concrete' if surface in ['concrete','paving_stones'] else 'Gravel' if surface in ['gravel','fine_gravel','compacted','dirt','ground'] else 'Asphalt'
 points=[((v[0]-loc[0])/scale[0],(v[1]-loc[1])/scale[1]) for v in p['points_cm']]
 groups[material].append(LineString(points).buffer(p['width_game_cm']/scale[0]/2,quad_segs=4))
used=Polygon();manifest=[]; area_error=[];allfaces=0
for material in ['Concrete','Asphalt','Gravel']:
 footprint=unary_union(groups[material]).difference(used);used=used.union(footprint)
 xmin,ymin,xmax,ymax=footprint.bounds
 xs=np.arange(max(0,math.floor(xmin)),min(nx-1,math.ceil(xmax)));ys=np.arange(max(0,math.floor(ymin)),min(ny-1,math.ceil(ymax)))
 xx,yy=np.meshgrid(xs,ys);xx=xx.ravel();yy=yy.ravel()
 cells=shapely.box(xx,yy,xx+1,yy+1);mask=shapely.intersects(cells,footprint)
 chunks={}; total_area=0.
 for ix,iy in zip(xx[mask],yy[mask]):
  for offsets in [[(0,0),(1,0),(1,1)],[(0,0),(1,1),(0,1)]]:
   tri=Polygon([(ix+a,iy+b) for a,b in offsets]); clip=tri.intersection(footprint)
   if clip.is_empty or clip.area<1e-10:continue
   # Plane solved from exact quantized Landscape vertices, not bilinear DEM.
   coords=np.array([(ix+a,iy+b) for a,b in offsets],dtype=float)
   z=np.array([(int(raw[iy+b,ix+a])-32768)*scale[2]/128+3 for a,b in offsets])
   plane=np.linalg.solve(np.column_stack([coords,np.ones(3)]),z)
   key=(int(ix)//126,int(iy)//126)
   mesh=chunks.setdefault(key,{'vertices':[],'uv':[],'faces':[],'lookup':{}})
   polygons=[clip] if clip.geom_type=='Polygon' else [g for g in getattr(clip,'geoms',[]) if g.geom_type=='Polygon']
   for poly in polygons:
    rings=[list(poly.exterior.coords)[:-1]]+[list(r.coords)[:-1] for r in poly.interiors]
    vertices=np.array([v for ring in rings for v in ring],dtype=np.float64)
    ends=np.cumsum([len(r) for r in rings],dtype=np.uint32)
    triangles=mapbox_earcut.triangulate_float64(vertices,ends).reshape(-1,3)
    for ids in triangles:
     face=Polygon(vertices[ids])
     if face.area<1e-10:continue
     total_area+=face.area;indices=[]
     v=list(face.exterior.coords)[:3]
     if not face.exterior.is_ccw:v.reverse()
     for x,y in v:
      wz=float(plane[0]*x+plane[1]*y+plane[2]);wx=float(loc[0]+x*scale[0]);wy=float(loc[1]+y*scale[1])
      k=(round(wx,5),round(wy,5),round(wz,5))
      if k not in mesh['lookup']:
       mesh['lookup'][k]=len(mesh['vertices'])+1;mesh['vertices'].append((wx,wy,wz));mesh['uv'].append((wx/200,wy/200))
      indices.append(mesh['lookup'][k])
     mesh['faces'].append(indices)
 error=abs(total_area-footprint.area)/max(footprint.area,1)
 area_error.append({'material':material,'relative_coverage_error':error})
 if error>1e-7:raise RuntimeError(f'{material} polygon triangulation lost coverage: {error}')
 for (cx,cy),mesh in sorted(chunks.items()):
  name=f'Park_{material}_{cx}_{cy}';lines=['# OSM contributors ODbL; elevations USGS 3DEP','# Centimeters, Z up, OBJ Y = negative Unreal Y.',f'o {name}']
  lines += ['v %.6f %.6f %.6f'%(v[0],-v[1],v[2]) for v in mesh['vertices']]
  lines += ['vt %.6f %.6f'%v for v in mesh['uv']]
  lines += ['f '+' '.join(f'{i}/{i}' for i in reversed(f)) for f in mesh['faces']]
  (OUT/(name+'.obj')).write_text('\n'.join(lines)+'\n')
  manifest.append({'file':name+'.obj','material':material,'vertices':len(mesh['vertices']),'triangles':len(mesh['faces']),'bounds_cm':[np.min(mesh['vertices'],axis=0).tolist(),np.max(mesh['vertices'],axis=0).tolist()]});allfaces+=len(mesh['faces'])
result={'status':'source_baked_not_imported','coordinates':'OBJ centimeters, Z up, Y negated from Unreal; bounds remain Unreal world coordinates','source_splines':'../park-path-network.json','landscape_clearance_cm':3,'terrain_diagonal':'i00-i11','chunks':manifest,'triangle_count':allfaces,'deferred_bridge_osm_ids':sorted(set(deferred)),'coverage_checks':area_error,'pending':['Unreal import orientation and collision validation','Bridge deck reconstruction','All path connectivity and ride-through acceptance']}
(OUT/'manifest.json').write_text(json.dumps(result,indent=2))
print(json.dumps({k:v for k,v in result.items() if k!='chunks'},indent=2));print('Chunks:',len(manifest))
