"""Original growl effect and dialogue header; not spoken dialogue recordings."""
from pathlib import Path
import json,wave,numpy as np
root=Path(__file__).resolve().parents[1]
lines=(root/'SourceAssets/Data/ZombieLines.txt').read_text().splitlines()
assert len(lines)>=30 and len(set(lines))==len(lines)
(root/'Source/AuraPlayground/BattleZombieLines.h').write_text('#pragma once\n// Original game dialogue. Spoken recordings remain a separate audio deliverable.\nnamespace BattleZombieLines { inline const TCHAR* Lines[] = {\n'+''.join(' TEXT('+json.dumps(v)+'),\n' for v in lines)+'}; }\n')
sr=24000;t=np.arange(int(sr*.8))/sr;rng=np.random.default_rng(420);phase=2*np.pi*(72*t+10*np.sin(t*7)*t)
y=(np.sin(phase)+.4*np.sin(phase*2.1)+.25*rng.normal(size=t.size))*.25*np.sin(np.pi*t/.8)**1.3
with wave.open(str(root/'SourceAssets/Audio/S_ZombieGrowl.wav'),'wb') as f:
 f.setnchannels(1);f.setsampwidth(2);f.setframerate(sr);f.writeframes((np.clip(y,-1,1)*32767).astype('<i2').tobytes())
