"""Author a supported static rail-corridor study; no surveyed elevations implied."""
import json,math,array,sys
from pathlib import Path
from shapely.geometry import LineString,Polygon
from shapely.ops import unary_union
from shapely.geometry.polygon import orient
from shapely import constrained_delaunay_triangles
root=Path(__file__).resolve().parents[1];folder=root/'SourceAssets/Terrain/KrogRailContext'
network=json.loads((folder/'network.json').read_text())
survey=json.loads((root/'Tests/Results/2026-09-12-krog-rail-support-survey.json').read_text())
roof_hits=[s['surface_z_cm'] for r in survey['railways'] for s in r['samples'] if 'Shell' in (s['actor'] or '') or 'shell' in (s['actor'] or '')]
assert roof_hits
height=max(roof_hits)+30
meta=json.loads((root/'SourceAssets/Terrain/terrain-georeference.json').read_text())
raw=array.array('H');raw.frombytes((root/'SourceAssets/Terrain/KrogTraffic/atlanta-height-dekalb-crowned-candidate.r16').read_bytes())
if sys.byteorder!='little':raw.byteswap()
def ground(x,y):
 sx,sy,sz=meta['unreal_scale'];lx,ly,_=meta['unreal_location_cm'];gx=(x-lx)/sx;gy=(-y-ly)/sy;ix=math.floor(gx);iy=math.floor(gy);dx=gx-ix;dy=gy-iy;w=meta['size'][0]
 a,b,c,d=[(raw[v*w+u]-32768)*sz/128 for u,v in [(ix,iy),(ix+1,iy),(ix,iy+1),(ix+1,iy+1)]]
 return a+(b-a)*dx+(d-b)*dy if dx>=dy else a+(d-c)*dx+(c-a)*dy
rails=[r for r in network['railways'] if r['tunnel_crossings']]
lines=[LineString(r['points_xy_cm']) for r in rails]
track_beds=unary_union([line.buffer(95,cap_style=2,join_style=2) for line in lines])
# A continuous authored yard envelope avoids isolated strips above the underpass.
# Its limits follow the outer mapped tracks, not surveyed property boundaries.
deck=track_beds.convex_hull
buildings=json.loads((root/'SourceAssets/Terrain/KrogBuildings/buildings.json').read_text())['buildings']
for building in buildings:
 assert deck.intersection(Polygon(building['footprint_xy']).buffer(30)).area<.01,building['osm_way']
roof=json.loads((root/'SourceAssets/Terrain/KrogContinuousShell/manifest.json').read_text())
tunnel=LineString([(p[0],-p[1]) for p in roof['roof_samples']])
roads=json.loads((root/'SourceAssets/Terrain/KrogTraffic/network.json').read_text())['roads']
reserve=unary_union([tunnel.buffer(700),*[LineString([p[:2] for p in r['points_cm']]).buffer((450 if r['tags']['name']=='DeKalb Avenue Northeast' else 300)+150) for r in roads]])
# The raised slab spans the tunnel/road reserves; retaining fill never enters them.
fill=deck.difference(reserve)
groups={m:[] for m in ['Ballast','Retaining','Sleepers','Steel']}
def face(mat,points):
 for i in range(1,len(points)-1):groups[mat].append([points[0],points[i],points[i+1]])
def parts(geom):return [geom] if geom.geom_type=='Polygon' else list(geom.geoms)
def cap(poly,mat,z,reverse=False):
 for t in constrained_delaunay_triangles(poly).geoms:
  p=[(x,y,z) for x,y in list(orient(t,sign=1).exterior.coords)[:3]]
  face(mat,list(reversed(p)) if reverse else p)
def wall(poly,mat,top,bottom):
 for ring in [orient(poly,sign=1).exterior,*orient(poly,sign=1).interiors]:
  for a,b in zip(ring.coords,list(ring.coords)[1:]):
   length=math.dist(a,b);n=max(1,math.ceil(length/100))
   for i in range(n):
    c=[a[j]+(b[j]-a[j])*i/n for j in range(2)];d=[a[j]+(b[j]-a[j])*(i+1)/n for j in range(2)]
    face(mat,[(c[0],c[1],bottom(*c)),(d[0],d[1],bottom(*d)),(d[0],d[1],top),(c[0],c[1],top)])
for p in parts(deck):
 cap(p,'Ballast',height);cap(p,'Retaining',height-24,True);wall(p,'Retaining',height,lambda x,y:height-24)
for p in parts(fill):wall(p,'Retaining',height-24,lambda x,y:min(height-25,ground(x,y)-20))
def beam(mat,a,b,width,low,high):
 dx,dy=b[0]-a[0],b[1]-a[1];length=math.hypot(dx,dy);ox,oy=dy/length*width/2,-dx/length*width/2
 p=Polygon([(a[0]+ox,a[1]+oy),(b[0]+ox,b[1]+oy),(b[0]-ox,b[1]-oy),(a[0]-ox,a[1]-oy)])
 cap(p,mat,high);cap(p,mat,low,True);wall(p,mat,high,lambda x,y:low)
tie_count=0
for line in lines:
 for distance in range(12,int(line.length)-12,24):
  p=line.interpolate(distance);a=line.interpolate(max(0,distance-1));b=line.interpolate(min(line.length,distance+1));yaw=math.atan2(b.y-a.y,b.x-a.x);nx,ny=-math.sin(yaw),math.cos(yaw)
  beam('Sleepers',(p.x-nx*45,p.y-ny*45),(p.x+nx*45,p.y+ny*45),9,height,height+5);tie_count+=1
 # Gauge scaled with the map's one-third geography. Steel extends continuously.
 for side in [-1,1]:
  offset=line.offset_curve(side*143.5/6,join_style=2)
  assert offset.geom_type=='LineString'
  for a,b in zip(offset.coords,list(offset.coords)[1:]):beam('Steel',a,b,3,height+5,height+11)
rows=[]
for mat,triangles in groups.items():
 out=['o KrogRail_'+mat];points=[p for tri in triangles for p in tri]
 for x,y,z in points:out.append(f'v {x:.5f} {-y:.5f} {z:.5f}')
 for x,y,z in points:out.append(f'vt {x/100:.5f} {y/100:.5f}')
 for i in range(0,len(points),3):out.append('f '+' '.join(f'{j}/{j}' for j in [i+3,i+2,i+1]))
 name='KrogRail_'+mat+'.obj';(folder/name).write_text('\n'.join(out)+'\n');rows.append({'material':mat,'file':name,'triangles':len(triangles)})
report={'author':'2026-09-12 [codex-maclaptop]','surfaces':rows,'yard_envelope_area_cm2':deck.area,'track_bed_area_cm2':track_beds.area,'rail_segments':len(rails),'sleepers':tie_count,'deck_height_cm':height,'deck_underside_cm':height-24,'retaining_fill_reserve_area_cm2':deck.intersection(reserve).area,'main_map_changed':False,'scope':'Static mapped rail corridor study. Authored continuous yard envelope, level deck/elevation and retaining fill; not surveyed engineering. Tunnel and approach road reserves excluded from retaining walls. Ends and adjoining railway terrain need refinement. MARTA elevated line omitted pending separate support design. Native clearance and visuals pending.'}
(folder/'deck-manifest.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report))
