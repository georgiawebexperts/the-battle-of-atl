"""Original deterministic boost whoosh; no sampled or external recordings."""
from pathlib import Path
import wave,math,random,struct
root=Path(__file__).resolve().parents[1];rng=random.Random(709);rate=24000;data=[];low=0
for i in range(rate*3):
 t=i/rate;env=min(t/.1,1)*min((3-t)/.35,1);low=.82*low+.18*rng.uniform(-1,1);v=env*(.38*low+.10*math.sin(2*math.pi*(95*t+60*t*t)))*(1-.2*t/3);data.append(struct.pack('<h',int(max(-1,min(1,v))*32767)))
with wave.open(str(root/'SourceAssets/Audio/S_Boost.wav'),'wb') as f:
 f.setnchannels(1);f.setsampwidth(2);f.setframerate(rate);f.writeframes(b''.join(data))
