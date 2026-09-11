"""Original synthesized shotgun and SMG reports; no sampled weapon recordings."""
from pathlib import Path
import numpy as np,wave
root=Path(__file__).resolve().parents[1];sr=24000;rng=np.random.default_rng(823)
for name,duration in [('S_Shotgun',.7),('S_SMG',.15)]:
 t=np.arange(int(sr*duration))/sr;n=rng.normal(size=len(t));decay=9 if name=='S_Shotgun' else 45
 y=.4*n*np.exp(-decay*t)+.3*np.sin(2*np.pi*80*t)*np.exp(-decay*t)
 with wave.open(str(root/'SourceAssets/Audio'/f'{name}.wav'),'wb') as f:
  f.setnchannels(1);f.setsampwidth(2);f.setframerate(sr);f.writeframes((np.clip(y,-1,1)*32767).astype('<i2').tobytes())
