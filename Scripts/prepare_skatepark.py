"""Author bike-scale concrete bowls/street ramps beside the mapped skatepark.
Original geometry; OSM determines position, official aerial guides arrangement.
Retain underlying DEM unchanged; grade a surrounding berm down to existing ground.
"""
import json, math
from pathlib import Path
import numpy as np
from shapely.geometry import MultiLineString,Point
root=Path(__file__).resolve().parents[1];out=root/'SourceAssets/Skatepark';out.mkdir(exist_ok=True)
meta=json.loads((root/'SourceAssets/Terrain/terrain-georeference.json').read_text());raw=np.fromfile(root/'SourceAssets/Terrain/atlanta-height-krog.r16',dtype='<u2').reshape(meta['size'][1],meta['size'][0]);loc=meta['unreal_location_cm'];sc=meta['unreal_scale'];cx,cy=39000.,74000.
def ground(x,y):
 gx=(x+cx-loc[0])/sc[0];gy=(-y-cy-loc[1])/sc[1];ix=int(gx);iy=int(gy);dx=gx-ix;dy=gy-iy
 a,b,c,d=[(float(raw[j,i])-32768)*sc[2]/128 for i,j in [(ix,iy),(ix+1,iy),(ix,iy+1),(ix+1,iy+1)]]
 return a+(b-a)*dx+(d-b)*dy if dx>=dy else a+(d-c)*dx+(c-a)*dy
# Two joined rounded bowls to the west, open street section east.
def feature(x,y):
 bowl=0
 for bx,by,rx,ry,depth in [(-950,-350,650,600,180),(-950,400,600,550,140)]:
  r=math.hypot((x-bx)/rx,(y-by)/ry)
  if r<1: bowl=min(bowl,-depth*(.5+.5*math.cos(math.pi*min(1,max(0,(r-.25)/.75)))))
 # Bank-to-lip jump line, rising east; open bypasses on either side.
 jump=0
 if abs(y+500)<380 and 250<x<1350:
  side=min(1,(380-abs(y+500))/100)
  if x<1000: h=300*((x-250)/750)**2
  elif x<=1200:h=300
  else:h=300*(1350-x)/150
  jump=h*max(0,side)
 # Low manual pad with broad roll-on banks in the other street lane.
 pad=0
 if 300<x<1300 and abs(y-550)<280:
  pad=70*min(1,(x-300)/180,(1300-x)/180,(280-abs(y-550))/100)
 return bowl+jump+max(0,pad)
step=50;xs=np.arange(-2400,3200+step,step);ys=np.arange(-1800,1800+step,step)
inside=lambda x,y:abs(x)<=1800 and abs(y)<=1200
deck=max(ground(x,y)-feature(x,y)+18 for x in xs for y in ys if inside(x,y));deck=math.ceil(deck/10)*10
# East connection joins the existing Eastside trail at verified source-network elevation.
trail=(41703.21309731,74167.72982629,475.11337638)
network=json.loads((root/'SourceAssets/Terrain/eastside-trail-network.json').read_text())
trail_lines=MultiLineString([[(p[0],-p[1]) for p in path['points_cm']] for path in network['paths']])
def surface(x,y):
 if inside(x,y):return deck+feature(x,y)
 dx=max(0,abs(x)-1800)/600;dy=max(0,abs(y)-1200)/600
 blend=max(0,1-max(dx,dy));z=ground(x,y)+2+(deck-ground(x,y)-2)*blend
 if x>=1800 and abs(y-(trail[1]-cy))<250:
  t=min(1,(x-1800)/(2500-1800));z=deck+(trail[2]-deck)*t
 return z
verts=[(float(x),float(y),surface(x,y)) for y in ys for x in xs];groups={'Concrete':[],'Berm':[]};nx=len(xs)
for j in range(len(ys)-1):
 for i in range(nx-1):
  x=(xs[i]+xs[i+1])/2;y=(ys[j]+ys[j+1])/2
  concrete=inside(x,y) or (x>=1800 and x<=2550 and abs(y-(trail[1]-cy))<250)
  if not concrete and trail_lines.distance(Point(cx+x,cy+y))<230:continue
  a=j*nx+i+1;b=a+1;c=a+nx;d=c+1
  groups['Concrete' if concrete else 'Berm'].extend([(a,b,d),(a,d,c)])
# Encode OBJ handedness explicitly: importer reverses Y; reverse winding too.
for name,faces in groups.items():
 lines=['# Original Battle for the ATL skatepark '+name]
 lines+=['v %.6f %.6f %.6f'%(v[0],-v[1],v[2]) for v in verts]
 lines+=['vt %.6f %.6f'%(v[0]/200,v[1]/200) for v in verts]
 lines+=['f '+' '.join(f'{i}/{i}' for i in reversed(f)) for f in faces]
 (out/(name+'.obj')).write_text('\n'.join(lines)+'\n')
manifest={'origin_world':[cx,cy,0],'deck_z':deck,'trail_connection':list(trail),'grid_spacing_cm':step,'vertex_count':len(verts),'triangles':{k:len(v) for k,v in groups.items()},'bounds':[[min(v[k] for v in verts) for k in range(3)],[max(v[k] for v in verts) for k in range(3)]],'bonuses':[[cx-950,cy-350,surface(-950,-350)+40],[cx-950,cy+400,surface(-950,400)+40],[cx+1100,cy-500,surface(1100,-500)+40]],'ramp_start':[cx+100,cy-500,deck+98],'bowl_start':[cx-950,cy-350,surface(-950,-350)+98],'policy':'Original playable adaptation of mapped Fourth Ward skatepark. Two connected bowls, street launch bank and manual pad. Berm/entrance meet retained terrain. Landscape untouched.'}
(out/'manifest.json').write_text(json.dumps(manifest,indent=2)+'\n');print(json.dumps(manifest))
