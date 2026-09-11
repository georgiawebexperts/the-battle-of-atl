"""Original synthesized disc launcher and ricochet effects."""
from pathlib import Path
import numpy as np,wave
root=Path(__file__).resolve().parents[1];sr=24000;rng=np.random.default_rng(824)
for name,duration in [('S_DiscLaunch',.35),('S_DiscBounce',.2)]:
 t=np.arange(int(sr*duration))/sr
 if name.endswith('Launch'): y=.2*np.sin(2*np.pi*(700*t-650*t*t))*np.exp(-9*t)+.08*rng.normal(size=len(t))*np.exp(-12*t)
 else:y=.28*np.sin(2*np.pi*(1200*t-1800*t*t))*np.exp(-22*t)
 with wave.open(str(root/'SourceAssets/Audio'/f'{name}.wav'),'wb') as f:
  f.setnchannels(1);f.setsampwidth(2);f.setframerate(sr);f.writeframes((np.clip(y,-1,1)*32767).astype('<i2').tobytes())
