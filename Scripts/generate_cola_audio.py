"""Original can-opening click and fizz; no sampled commercial audio."""
from pathlib import Path
import numpy as np,wave
root=Path(__file__).resolve().parents[1];sr=24000;t=np.arange(int(sr*.5))/sr;rng=np.random.default_rng(811)
noise=rng.normal(size=len(t));noise=np.r_[0,np.diff(noise)]
y=.13*noise*np.exp(-8*t)+.25*np.sin(2*np.pi*1400*t)*np.exp(-180*t)
with wave.open(str(root/'SourceAssets/Audio/S_ColaOpen.wav'),'wb') as f:
 f.setnchannels(1);f.setsampwidth(2);f.setframerate(sr);f.writeframes((np.clip(y,-1,1)*32767).astype('<i2').tobytes())
