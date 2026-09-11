"""Original deterministic synthesized ride Foley, no external samples."""
from pathlib import Path
import wave,math,random,struct
root=Path(__file__).resolve().parents[1]/'SourceAssets/Audio';rate=24000
for name,duration in [('Skid',.7),('Splash',1.4),('Bump',.2),('Asphalt',2),('Grass',2),('Motor',2)]:
 rng=random.Random(910);data=[];low=0
 for i in range(int(rate*duration)):
  t=i/rate;n=rng.uniform(-1,1);low=.85*low+.15*n
  if name=='Skid':v=(.17*math.sin(2*math.pi*(640*t+90*t*t))+.22*n)*min(t/.03,1)*max(0,1-t/duration)
  elif name=='Splash':v=(.85*low+.13*n)*min(t/.02,1)*math.exp(-2.5*t)*(1+.3*math.sin(2*math.pi*11*t))
  elif name=='Bump':v=(.45*math.sin(2*math.pi*85*t)+.1*n)*math.exp(-28*t)*min(t/.004,1)
  elif name=='Asphalt':v=.17*low+.025*n
  elif name=='Grass':v=(.28*low+.06*n)*(.7+.3*math.sin(2*math.pi*13*t))
  else:v=.075*math.sin(2*math.pi*120*t)+.025*math.sin(2*math.pi*360*t)
  data.append(struct.pack('<h',int(max(-1,min(1,v))*32767)))
 with wave.open(str(root/('S_'+name+'.wav')),'wb') as f:f.setnchannels(1);f.setsampwidth(2);f.setframerate(rate);f.writeframes(b''.join(data))
