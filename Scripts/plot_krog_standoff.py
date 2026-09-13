import json,math
from pathlib import Path
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
from matplotlib.patches import Polygon,Circle
root=Path(__file__).resolve().parents[1];j=json.loads((root/'Tests/Results/2026-09-13-krog-standoff-geometry.json').read_text())
fig,ax=plt.subplots(figsize=(12,9));origin=(29000,115600)
def xy(p):return((p[0]-origin[0])/100,(p[1]-origin[1])/100)
for paved,color,label in [(False,'#aac39c','Other native ground'),(True,'#cccccc','Paved native ground')]:
 pts=[xy(s['xyz']) for s in j['ground_samples'] if s['paved']==paved]
 ax.scatter(*zip(*pts),marker='s',s=90,c=color,alpha=.6,label=label)
for i,p in enumerate(j['paths']):
 pts=[xy(p) for p in p['points']];ax.plot(*zip(*pts),color='#186fb4',lw=3,label='Mapped rider centreline' if i==0 else None)
for l in j['lanes']:
 pts=[xy(p) for p in l['points']];ax.plot(*zip(*pts),color='#e5892b',lw=2,label='Car centreline')
car=(29278.283,115643.170);rider=(28896.426,115622.833)
p=j['lanes'][0]['points'];i=min(range(len(p)),key=lambda i:math.dist(p[i][:2],car));a=p[max(0,i-1)];b=p[min(len(p)-1,i+1)];h=math.atan2(b[1]-a[1],b[0]-a[0]);f=(math.cos(h),math.sin(h));right=(-f[1],f[0]);corners=[xy((car[0]+x*f[0]+y*right[0],car[1]+x*f[1]+y*right[1])) for x,y in [(-236,-114),(236,-114),(236,114),(-236,114)]]
ax.add_patch(Polygon(corners,color='#ad3227',alpha=.75,label='Stopped car collision footprint'))
ax.add_patch(Circle(xy(rider),.62,color='#25223e',label='Rider at failure'))
ax.annotate('Rider stopped here',xy(rider),xytext=(-7,5),arrowprops={'arrowstyle':'->'})
ax.set(xlim=(-8,16),ylim=(-8,9),xlabel='World X offset (metres)',ylabel='World Y offset (metres)',title='Krog standoff: saved map geometry and failed-run positions')
ax.set_aspect('equal');ax.grid(alpha=.2);ax.legend(loc='lower left',fontsize=8)
fig.tight_layout();out=root/'work/krog-standoff-layout.png';fig.savefig(out,dpi=140);print(out)
print('Car heading from saved lane:',math.degrees(h))
