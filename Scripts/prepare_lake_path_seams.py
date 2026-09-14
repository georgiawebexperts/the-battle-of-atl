"""Fill sub-40cm pavement slits inside grading patches, preserving wider grass areas."""
import json,math
from pathlib import Path
import numpy as np,shapely
from shapely.geometry import Polygon,LineString,Point
from shapely.ops import unary_union,transform
root=Path(__file__).resolve().parents[1];terrain=root/'SourceAssets/Terrain';out=terrain/'LakePathGrading/Seams';out.mkdir(exist_ok=True)
m=json.loads((terrain/'terrain-georeference.json').read_text());nx,ny=m['size'];sx,sy,sz=m['unreal_scale'];lx,ly,_=m['unreal_location_cm'];raw=np.fromfile(terrain/'LakePathGrading/atlanta-height-lake-paths-candidate.r16',dtype='<u2').reshape(ny,nx)
groups={k:[] for k in ['Concrete','Asphalt','Gravel']}
for p in json.loads((terrain/'park-path-network.json').read_text())['paths']:
 if p['tags'].get('bridge')=='yes':continue
 surface=p['tags'].get('surface','asphalt');material='Concrete' if surface in ['concrete','paving_stones'] else 'Gravel' if surface in ['gravel','fine_gravel','compacted','dirt','ground'] else 'Asphalt'
 groups[material].append(LineString([(v[0],-v[1]) for v in p['points_cm']]).buffer(p['width_game_cm']/2,quad_segs=4))
groups={k:unary_union(v) for k,v in groups.items()};paved=unary_union(list(groups.values()));zone=unary_union([Point(-5050,-1925).buffer(1000),Point(-9000,460).buffer(1000)])
lake=json.loads((terrain/'lake-clara-meer.json').read_text());water=Polygon([(v[0],-v[1]) for v in lake['outer_cm']],holes=[[(v[0],-v[1]) for v in lake['island_cm']]])
filler=paved.buffer(20,quad_segs=4).buffer(-20,quad_segs=4).difference(paved).intersection(zone).difference(water.buffer(100))
assert filler.area<10000 and filler.contains(Point(-5100,-2275)), 'Unexpected fill area or missing target seam'
grid=transform(lambda x,y:((np.asarray(x)-lx)/sx,(-np.asarray(y)-ly)/sy),filler);meshes={}
x0,y0,x1,y1=grid.bounds
for iy in range(math.floor(y0),math.ceil(y1)):
 for ix in range(math.floor(x0),math.ceil(x1)):
  if not grid.intersects(shapely.box(ix,iy,ix+1,iy+1)):continue
  for offsets in [[(0,0),(1,0),(1,1)],[(0,0),(1,1),(0,1)]]:
   coords=np.array([(ix+a,iy+b) for a,b in offsets],dtype=float);clip=Polygon(coords).intersection(grid)
   heights=np.array([(int(raw[iy+b,ix+a])-32768)*sz/128+3 for a,b in offsets]);plane=np.linalg.solve(np.column_stack([coords,np.ones(3)]),heights)
   polygons=[clip] if clip.geom_type=='Polygon' else [g for g in getattr(clip,'geoms',[]) if g.geom_type=='Polygon']
   for poly in polygons:
    if poly.area<1e-9:continue
    center=Point(lx+poly.centroid.x*sx,-(ly+poly.centroid.y*sy));material=min(groups,key=lambda k:groups[k].distance(center));mesh=meshes.setdefault(material,{'vertices':[],'faces':[]})
    for face in shapely.get_parts(shapely.constrained_delaunay_triangles(poly)):
     points=list(face.exterior.coords)[:-1]
     if face.area<1e-9:continue
     if not face.exterior.is_ccw:points.reverse()
     start=len(mesh['vertices'])+1
     for x,y in points:mesh['vertices'].append((float(lx+x*sx),float(-(ly+y*sy)),float(plane[0]*x+plane[1]*y+plane[2])))
     mesh['faces'].append([start+2,start+1,start])
rows=[]
for material,mesh in meshes.items():
 name='LakePathSeams_'+material;lines=['# Authored thin-gap closure of OSM path footprints; candidate DEM planes +3cm',f'o {name}']
 lines += ['v %.6f %.6f %.6f'%v for v in mesh['vertices']];lines += ['vt %.6f %.6f'%(v[0]/100,v[1]/100) for v in mesh['vertices']];lines+=['f '+' '.join(f'{i}/{i}' for i in face) for face in mesh['faces']];(out/(name+'.obj')).write_text('\n'.join(lines)+'\n');rows.append({'file':name+'.obj','material':material,'triangles':len(mesh['faces'])})
(out/'manifest.json').write_text(json.dumps({'area_m2':filler.area/10000,'max_closing_width_cm':40,'within_two_patches':True,'water_and_100cm_bank_excluded':True,'meshes':rows},indent=2)+'\n');print(json.dumps({'area_m2':filler.area/10000,'meshes':rows}))
