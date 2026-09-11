"""Original synthesized chain swing and impact, without sampled recordings."""
from pathlib import Path
import numpy as np,wave
root=Path(__file__).resolve().parents[1];sr=24000;rng=np.random.default_rng(822)
for name,duration in [('S_LockSwing',.45),('S_LockHit',.3)]:
 t=np.arange(int(sr*duration))/sr;noise=rng.normal(size=len(t))
 if name.endswith('Swing'):
  env=np.sin(np.pi*t/duration)**2
  y=.13*noise*env+.09*np.sin(2*np.pi*2100*t)*np.exp(-18*t)
 else:
  y=.3*np.sin(2*np.pi*(130*t-90*t*t))*np.exp(-22*t)+.16*noise*np.exp(-35*t)+.12*np.sin(2*np.pi*1800*t)*np.exp(-25*t)
 with wave.open(str(root/'SourceAssets/Audio'/f'{name}.wav'),'wb') as f:
  f.setnchannels(1);f.setsampwidth(2);f.setframerate(sr);f.writeframes((np.clip(y,-1,1)*32767).astype('<i2').tobytes())
